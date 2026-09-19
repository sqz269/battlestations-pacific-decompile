"""Read float/double/dword constants from the target PE at virtual addresses.

Usage: python tools/pe_const_read.py f:00ce3820 d:00d7a2b0 x:00e188a8
  f = 4-byte float, d = 8-byte double, x = 4-byte hex dword, s = NUL string.
Reads the on-disk image through config/target.json's `binary`; .bss addresses
past the section's raw size report `zero-fill`.
"""
from __future__ import annotations

import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def load_sections():
    cfg = json.loads((ROOT / "config" / "target.json").read_text())
    data = Path(cfg["binary"]).read_bytes()
    base = int(cfg["image_base"], 16)
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    tbl = pe + 24 + opt
    secs = []
    for i in range(nsec):
        o = tbl + 40 * i
        name = data[o:o + 8].rstrip(b"\0").decode("latin1")
        vsize, vaddr, rawsize, rawptr = struct.unpack_from("<IIII", data, o + 8)
        secs.append((name, base + vaddr, vsize, rawptr, rawsize))
    return data, secs


def offset_of(secs, va):
    for name, vbase, vsize, rawptr, rawsize in secs:
        if vbase <= va < vbase + vsize:
            rel = va - vbase
            if rel >= rawsize:
                return None, name
            return rawptr + rel, name
    return None, "?"


def main(argv):
    data, secs = load_sections()
    for spec in argv:
        kind, _, addr = spec.partition(":")
        va = int(addr, 16)
        off, sec = offset_of(secs, va)
        if off is None:
            print(f"{spec:>16} {sec:<10} zero-fill (past raw size)")
            continue
        if kind == "f":
            (v,) = struct.unpack_from("<f", data, off)
            print(f"{spec:>16} {sec:<10} {v!r}")
        elif kind == "d":
            (v,) = struct.unpack_from("<d", data, off)
            print(f"{spec:>16} {sec:<10} {v!r}")
        elif kind == "x":
            (v,) = struct.unpack_from("<I", data, off)
            print(f"{spec:>16} {sec:<10} 0x{v:08x}")
        elif kind == "s":
            end = data.index(b"\0", off)
            print(f"{spec:>16} {sec:<10} {data[off:end].decode('latin1')!r}")
        else:
            print(f"{spec:>16} unknown kind {kind!r}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
