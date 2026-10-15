import Foundation

#if canImport(Metal)
  import Metal
#endif

/// The machine's chip, as a backend names what its artifacts are valid for.
public enum HostChip {
  /// The SoC's name on Apple platforms, which is the GPU: "Apple M1 Pro",
  /// "Apple A17 Pro". On a Mac the CPU brand string, as the Python's gpu_name
  /// reads it, so the two agree on a cache key. "cpu" elsewhere, where the
  /// app passes the SoC's model.
  public static func name() -> String {
    #if os(macOS)
      var size = 0
      if sysctlbyname("machdep.cpu.brand_string", nil, &size, nil, 0) == 0, size > 1 {
        var bytes = [CChar](repeating: 0, count: size)
        if sysctlbyname("machdep.cpu.brand_string", &bytes, &size, nil, 0) == 0 {
          return String(decoding: bytes.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, as: UTF8.self)
        }
      }
    #endif
    #if canImport(Metal)
      let name = MTLCreateSystemDefaultDevice()?.name ?? "unknown"
      return name.hasSuffix(" GPU") ? String(name.dropLast(4)) : name
    #else
      return "cpu"
    #endif
  }

  /// The chip a backend's artifacts are for: `chip` as the app names the
  /// SoC, or this machine's name when it names none; "unknown" for an empty
  /// name.
  public static func resolve(_ chip: String?) -> String {
    let name = chip ?? name()
    return name.isEmpty ? "unknown" : name
  }

  /// A CPU backend's pool when it runs the whole model: half the cores,
  /// leaving the rest to the link and the app.
  public static var cpuThreads: Int { max(1, ProcessInfo.processInfo.activeProcessorCount / 2) }
}
