"""Check every image-address constant in a header against both widths.

A constant declared beside an image address can be read at four bytes or at
eight, and the two give unrelated values. `00D21318` is the case that cost two
days: its float is -1.3962634 = -DEG(80) and its double is 0.05625, a header
carried the double, and both loads are four-byte - so the aim tick turned every
descent into a climb. `00CE3D48` is the same hazard the other way: float
-1.084202e-19, double 1.6.

This reads eight bytes at each declared address out of the image and prints the
float and the double there beside the declared value, so a mismatch is visible
without reading the load site. It cannot tell you which width the CODE uses -
only the listing does that - so a mismatch means "go and read the load", not
"the constant is wrong": a declaration may legitimately differ from this check
if the address is shared by two constants of different widths.

Usage:
    python tools/const_width_sweep.py include/bsp/foo.hpp [more.hpp ...]
    python tools/const_width_sweep.py --all          # every header under include/bsp
    python tools/const_width_sweep.py --all --full   # one line per constant, not just misses

Exit code is 1 when any constant mismatches its declared width.
"""
import glob
import json
import os
import re
import struct
import sys

import pefile

PATTERN = re.compile(
    r"inline\s+constexpr\s+(float|double)\s+(\w+)\s*=\s*"
    r"([-+]?[0-9.]+(?:[eE][-+]?[0-9]+)?)f?\s*;\s*//\s*([0-9A-Fa-f]{8})\b")


def load_image():
    with open("config/target.json", encoding="utf-8") as fh:
        exe = json.load(fh)["binary"]
    pe = pefile.PE(exe, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    return [(base + s.VirtualAddress, s.get_data()) for s in pe.sections]


def read8(sections, va):
    for va0, data in sections:
        off = va - va0
        if 0 <= off <= len(data) - 8:
            return data[off:off + 8]
    return None


def main(argv):
    full = "--full" in argv
    args = [a for a in argv if not a.startswith("--")]
    if "--all" in argv:
        args = sorted(glob.glob(os.path.join("include", "bsp", "*.hpp")))
    if not args:
        print(__doc__)
        return 2

    sections = load_image()
    checked = 0
    misses = []
    for path in args:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line_no, line in enumerate(fh, 1):
                m = PATTERN.search(line)
                if not m:
                    continue
                width, name, value, addr = m.groups()
                raw = read8(sections, int(addr, 16))
                if raw is None:
                    continue
                f = struct.unpack("<f", raw[:4])[0]
                d = struct.unpack("<d", raw)[0]
                declared = float(value)
                got = f if width == "float" else d
                checked += 1
                tol = max(1e-9, abs(declared) * 1e-6)
                ok = abs(got - declared) <= tol
                if full:
                    print("%-46s %5d %-30s %-6s declared=%-16g float=%-16g double=%-16g %s"
                          % (path, line_no, name, width, declared, f, d,
                             "ok" if ok else "MISMATCH"))
                if not ok:
                    misses.append((path, line_no, name, width, declared, f, d))

    for path, line_no, name, width, declared, f, d in misses:
        print("%s:%d  %s declared %s=%g  but float=%g double=%g"
              % (path, line_no, name, width, declared, f, d))
    print("checked %d constants in %d headers, %d mismatched"
          % (checked, len(args), len(misses)))
    return 1 if misses else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
