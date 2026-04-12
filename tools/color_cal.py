#!/usr/bin/env python3
import argparse
import binascii
import os
import struct
from pathlib import Path


IGNORED_TAIL = b"\x00\x00\x00\x00\x00"
VALUES_COUNT = 13
VALUES_FMT = f"<{VALUES_COUNT}e"
VALUES_SIZE = struct.calcsize(VALUES_FMT)


def read_color_cal(path: str) -> tuple[float, list[float], list[float], bytes]:
  with open(path, "rb") as f:
    data = f.read()

  if len(data) < VALUES_SIZE:
    raise ValueError(f"{path} is too short: expected at least {VALUES_SIZE} bytes, got {len(data)}")

  values = struct.unpack(VALUES_FMT, data[:VALUES_SIZE])
  gamma = float(values[0])
  ccm = [float(v) for v in values[1:10]]
  wb = [float(v) for v in values[10:13]]
  return gamma, ccm, wb, data[VALUES_SIZE:]


def write_color_cal(path: str, gamma: float, ccm: list[float], wb: list[float], tail: bytes = IGNORED_TAIL) -> None:
  if len(ccm) != 9:
    raise ValueError("CCM must contain exactly 9 values")
  if len(wb) != 3:
    raise ValueError("WB gains must contain exactly 3 values")

  payload = struct.pack(VALUES_FMT, gamma, *ccm, *wb)
  data = payload + tail
  out = Path(path)
  out.parent.mkdir(parents=True, exist_ok=True)
  out.write_bytes(data)


def cmd_analyze(args: argparse.Namespace) -> int:
  gamma, ccm, wb, tail = read_color_cal(args.path)
  data = Path(args.path).read_bytes()
  print(f"path: {args.path}")
  print(f"size: {len(data)}")
  print(f"hex: {binascii.hexlify(data).decode()}")
  print(f"gamma: {gamma:.4f}")
  print("ccm:")
  print(f"  [{ccm[0]:.4f}, {ccm[1]:.4f}, {ccm[2]:.4f}]")
  print(f"  [{ccm[3]:.4f}, {ccm[4]:.4f}, {ccm[5]:.4f}]")
  print(f"  [{ccm[6]:.4f}, {ccm[7]:.4f}, {ccm[8]:.4f}]")
  print(f"wb: [R={wb[0]:.4f}, G={wb[1]:.4f}, B={wb[2]:.4f}]")
  print(f"ignored_tail_hex: {tail.hex()}")
  return 0


def cmd_generate(args: argparse.Namespace) -> int:
  tail = binascii.unhexlify(args.tail_hex) if args.tail_hex else IGNORED_TAIL
  write_color_cal(args.output, args.gamma, args.ccm, args.wb, tail=tail)
  print(f"wrote {args.output}")
  return 0


def cmd_from_hex(args: argparse.Namespace) -> int:
  data = binascii.unhexlify(args.hex_string)
  out = Path(args.output)
  out.parent.mkdir(parents=True, exist_ok=True)
  out.write_bytes(data)
  print(f"wrote {args.output}")
  return 0


def build_parser() -> argparse.ArgumentParser:
  parser = argparse.ArgumentParser(description="Inspect and generate comma/Weston color_cal files.")
  subparsers = parser.add_subparsers(dest="command", required=True)

  analyze = subparsers.add_parser("analyze", help="Decode an existing color_cal file")
  analyze.add_argument("path")
  analyze.set_defaults(func=cmd_analyze)

  generate = subparsers.add_parser("generate", help="Generate a color_cal file from gamma/CCM/WB values")
  generate.add_argument("output")
  generate.add_argument("--gamma", type=float, required=True)
  generate.add_argument("--ccm", type=float, nargs=9, required=True)
  generate.add_argument("--wb", type=float, nargs=3, required=True)
  generate.add_argument("--tail-hex", default="", help="Optional trailing bytes as hex; Weston ignores these")
  generate.set_defaults(func=cmd_generate)

  from_hex = subparsers.add_parser("from-hex", help="Write a raw color_cal blob from a hex string")
  from_hex.add_argument("output")
  from_hex.add_argument("hex_string")
  from_hex.set_defaults(func=cmd_from_hex)

  return parser


def main() -> int:
  parser = build_parser()
  args = parser.parse_args()
  return args.func(args)


if __name__ == "__main__":
  raise SystemExit(main())
