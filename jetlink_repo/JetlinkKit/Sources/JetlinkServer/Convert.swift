#if canImport(Accelerate)
  import Accelerate
#endif

/// The frame path's bulk conversions, and its finite check.
///
/// A frame converts about 800,000 camera bytes to float16 and 35,000 floats
/// each way: 0.2 ms as element loops in an optimized build, over 40 ms in a
/// Debug one (65 ms a frame on an iPhone 17 Pro run from Xcode). So on Apple
/// platforms vImage does them, whatever the build; elsewhere they are loops a
/// release build vectorizes (`NativeHalf`, `IntegerHalf`). ConvertTests holds
/// each to the Float16 element loop bit for bit, NaN, overflow and subnormals
/// included, so the staged tensors keep matching numpy's round to nearest
/// even. Swift's SIMD types convert lane by lane, 2 to 100 times slower than
/// what the vectorizer makes of the loops.
enum Convert {
  /// uint8 to float16: 0...255 are all exact in float16.
  static func u8ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    #if canImport(Accelerate)
      VImageHalf.u8ToF16(source, destination, count: count)
    #else
      IntegerHalf.u8ToF16(source, destination, count: count)
    #endif
  }

  /// Round to nearest even, as numpy's float32 to float16 cast. The source
  /// need not be aligned.
  static func f32ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    #if canImport(Accelerate)
      VImageHalf.f32ToF16(source, destination, count: count)
    #elseif arch(arm64)
      NativeHalf.f32ToF16(source, destination, count: count)
    #else
      IntegerHalf.f32ToF16(source, destination, count: count)
    #endif
  }

  static func f16ToF32(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    #if canImport(Accelerate)
      VImageHalf.f16ToF32(source, destination, count: count)
    #elseif arch(arm64)
      NativeHalf.f16ToF32(source, destination, count: count)
    #else
      IntegerHalf.f16ToF32(source, destination, count: count)
    #endif
  }

  /// float16 to uint8 for an image queue's rows, which came from uint8
  /// frames: each is a whole number in 0...255, which this returns exactly
  /// from the bits alone, for a graph that reads its images as uint8 (as
  /// LiteRT's do). The implicit leading one, shifted down by the exponent;
  /// zero shifts it out entirely. Other values are not images and come out
  /// wrong.
  static func f16ToU8(_ source: UnsafePointer<UInt16>, _ destination: UnsafeMutableRawPointer, count: Int) {
    let out = destination.assumingMemoryBound(to: UInt8.self)
    for i in 0..<count {
      let half = Int32(source[i])
      let shift = max(0, 25 &- ((half &>> 10) & 0x1F))
      out[i] = UInt8(truncatingIfNeeded: (0x400 | (half & 0x3FF)) &>> shift)
    }
  }

  /// No NaN and no infinity among `count` float32s. A float is not finite
  /// when its exponent bits are all set, and one more in the exponent then
  /// carries into the sign bit: or-ing those sums keeps the loop free of
  /// branches, so it vectorizes, where `isFinite` with an early exit does not.
  static func allFinite(_ source: UnsafeRawPointer, count: Int) -> Bool {
    var carries: UInt32 = 0
    for i in 0..<count {
      carries |= (source.loadUnaligned(fromByteOffset: i &* 4, as: UInt32.self) & 0x7F80_0000) &+ 0x0080_0000
    }
    return carries & 0x8000_0000 == 0
  }
}

#if canImport(Accelerate)
  enum VImageHalf {
    private static let u8ToF16Bits: [UInt16] = (0..<256).map { Float16(Float($0)).bitPattern }

    private static func buffer(_ pointer: UnsafeRawPointer, _ count: Int, _ size: Int) -> vImage_Buffer {
      vImage_Buffer(data: UnsafeMutableRawPointer(mutating: pointer), height: 1, width: vImagePixelCount(count), rowBytes: count * size)
    }

    static func u8ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      guard count > 0 else { return }
      var src = buffer(source, count, 1)
      var dst = buffer(destination, count, 2)
      u8ToF16Bits.withUnsafeBufferPointer { table in
        _ = vImageLookupTable_Planar8toPlanar16(&src, &dst, table.baseAddress!, vImage_Flags(kvImageDoNotTile))
      }
    }

    static func f32ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      guard count > 0 else { return }
      var src = buffer(source, count, 4)
      var dst = buffer(destination, count, 2)
      _ = vImageConvert_PlanarFtoPlanar16F(&src, &dst, vImage_Flags(kvImageDoNotTile))
    }

    static func f16ToF32(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      guard count > 0 else { return }
      var src = buffer(source, count, 2)
      var dst = buffer(destination, count, 4)
      _ = vImageConvert_Planar16FtoPlanarF(&src, &dst, vImage_Flags(kvImageDoNotTile))
    }
  }
#endif

/// Float16's own conversions as element loops, which a release build turns
/// into arm64's vector conversions (FCVTN, FCVTL): the element loop's bits,
/// several lanes at a time. On x86_64 each element would be a call.
enum NativeHalf {
  static func f32ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    let out = destination.assumingMemoryBound(to: UInt16.self)
    for i in 0..<count { out[i] = Float16(source.loadUnaligned(fromByteOffset: i &* 4, as: Float.self)).bitPattern }
  }

  static func f16ToF32(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    let out = destination.assumingMemoryBound(to: Float.self)
    for i in 0..<count { out[i] = Float(Float16(bitPattern: source.loadUnaligned(fromByteOffset: i &* 2, as: UInt16.self))) }
  }
}

/// The conversions in integer arithmetic, which vectorize on any CPU. Bytes
/// go this way everywhere, needing no half-precision instruction at all.
/// Floats go this way on x86_64, whose baseline has no F16C, so Float16
/// there converts through a library call per element. Every case is computed
/// and the result selected without a branch, so the loops vectorize. A NaN
/// keeps its sign and the top of its payload and comes out quiet, as arm64,
/// F16C and vImage convert it.
enum IntegerHalf {
  static func u8ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    let src = source.assumingMemoryBound(to: UInt8.self)
    let out = destination.assumingMemoryBound(to: UInt16.self)
    for i in 0..<count {
      // Exact in float32, and scaling by 2^-112 moves float32's exponent
      // bias to float16's, so the top bits are the float16. Zero stays zero.
      out[i] = UInt16(truncatingIfNeeded: (Float(src[i]) * 0x1p-112).bitPattern &>> 13)
    }
  }

  static func f32ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    let out = destination.assumingMemoryBound(to: UInt16.self)
    for i in 0..<count { out[i] = half(source.loadUnaligned(fromByteOffset: i &* 4, as: UInt32.self)) }
  }

  static func f16ToF32(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
    let out = destination.assumingMemoryBound(to: UInt32.self)
    for i in 0..<count { out[i] = single(source.loadUnaligned(fromByteOffset: i &* 2, as: UInt16.self)) }
  }

  /// float32 bits to float16 bits, round to nearest even.
  @inline(__always)
  static func half(_ x: UInt32) -> UInt16 {
    let a = x & 0x7FFF_FFFF
    // Normal: rebias the exponent by -112 and round off the 13 dropped bits,
    // ties to even; a carry out of the fraction moves to the next exponent,
    // and past 65504 to infinity.
    let normal = (a &+ 0xC800_0FFF &+ ((a &>> 13) & 1)) &>> 13
    // Below 2^-14, a subnormal: adding 0.5, whose ulp is float16's least
    // subnormal, has the FPU round the fraction into place. A float32
    // subnormal rounds to zero here even where the FPU flushes it.
    let subnormal = (Float(bitPattern: a) + 0.5).bitPattern &- 0x3F00_0000
    let nan = 0x7E00 | ((a &>> 13) & 0x3FF)
    var h = a < 0x3880_0000 ? subnormal : normal
    h = a >= 0x4780_0000 ? 0x7C00 : h
    h = a > 0x7F80_0000 ? nan : h
    return UInt16(truncatingIfNeeded: h | ((x &>> 16) & 0x8000))
  }

  /// float16 bits to float32 bits, exact.
  @inline(__always)
  static func single(_ h: UInt16) -> UInt32 {
    let a = UInt32(h & 0x7FFF)
    let normal = (a &<< 13) &+ 0x3800_0000
    // A subnormal is its fraction times 2^-24: through an integer, so no
    // float32 subnormal is ever an operand.
    let subnormal = (Float(a) * 0x1p-24).bitPattern
    let special = 0x7F80_0000 | ((a & 0x3FF) &<< 13) | (a > 0x7C00 ? 0x40_0000 : 0)
    var x = a < 0x400 ? subnormal : normal
    x = a >= 0x7C00 ? special : x
    return x | (UInt32(h & 0x8000) &<< 16)
  }
}
