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


def load_site_index():
    """addr -> [(va, mnemonic, operand bytes)] for absolute [disp32] operands.

    Linear disassembly of .text with Capstone. It desyncs on inline data, so an
    address with no entry is UNREFERENCED BY THIS SCAN, which is not the same as
    unreferenced: report it as its own class rather than as safe. The operand
    size is what discriminates `FLD m32` from `FLD m64` and `MOVSS` from `MOVSD`,
    and it is the field the x87 access-flag quirk in our notes does not touch.
    """
    import capstone

    with open("config/target.json", encoding="utf-8") as fh:
        exe = json.load(fh)["binary"]
    pe = pefile.PE(exe, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    text = None
    for s in pe.sections:
        if s.Name.rstrip(b"\0") == b".text":
            text = (base + s.VirtualAddress, s.get_data())
    if text is None:
        return {}

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    index = {}
    va0, data = text

    # Sweeping linearly from the section start desyncs on inline data and finds
    # none of the sites this stream has read by hand. Disassemble from each
    # KNOWN FUNCTION START instead, up to the next one, which is the technique
    # the project's own notes prescribe for exactly this reason.
    import sqlite3

    starts = []
    try:
        db = sqlite3.connect("local/bsp_index.sqlite")
        starts = [r[0] for r in db.execute(
            "SELECT address FROM functions ORDER BY address")]
    except sqlite3.Error:
        starts = []
    if not starts:
        raise SystemExit("local/bsp_index.sqlite has no function table; run "
                         "`python tools/bsp.py index` first")

    end = va0 + len(data)
    for i, start in enumerate(starts):
        if start < va0 or start >= end:
            continue
        stop = starts[i + 1] if i + 1 < len(starts) else end
        stop = min(stop, end)
        off = start - va0
        for insn in md.disasm(data[off:off + (stop - start)], start):
            for op in insn.operands:
                if op.type != capstone.x86.X86_OP_MEM:
                    continue
                mem = op.mem
                # An absolute [disp32]: no base, no index.
                if mem.base == 0 and mem.index == 0 and mem.disp:
                    index.setdefault(mem.disp & 0xFFFFFFFF, []).append(
                        (insn.address, insn.mnemonic, op.size))
    return index


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

    if "--load-sites" not in argv:
        for path, line_no, name, width, declared, f, d in misses:
            print("%s:%d  %s declared %s=%g  but float=%g double=%g"
                  % (path, line_no, name, width, declared, f, d))
        print("checked %d constants in %d headers, %d mismatched"
              % (checked, len(args), len(misses)))
        return 1 if misses else 0

    # The discriminator. "Matches the other width" covers two opposite cases:
    # every load is 8 bytes, so the image value IS the declared one and only the
    # C++ type is narrow (harmless); or some load is 4 bytes, so the declared
    # value was read at the wrong width and is simply wrong (the kPitchClampLo
    # case, which prints as Class A). Only the load sites tell them apart.
    index = load_site_index()
    buckets = {"A-harmless": [], "A-WRONG": [], "A-unreferenced": [],
               "B": [], "B-unreferenced": []}
    for path, line_no, name, width, declared, f, d in misses:
        addr = None
        with open(path, encoding="utf-8", errors="replace") as fh:
            for i, line in enumerate(fh, 1):
                if i == line_no:
                    m = PATTERN.search(line)
                    if m:
                        addr = int(m.group(4), 16)
                    break
        sites = index.get(addr, []) if addr is not None else []
        sizes = sorted({size for _, _, size in sites})
        other = d if width == "float" else f
        class_a = abs(other - declared) <= max(1e-9, abs(declared) * 1e-6)
        row = (path, line_no, name, width, declared, f, d, sites, sizes)
        if not class_a:
            buckets["B" if sites else "B-unreferenced"].append(row)
        elif not sites:
            buckets["A-unreferenced"].append(row)
        elif 4 in sizes and width == "float":
            buckets["A-WRONG"].append(row)
        elif 8 in sizes and width == "double":
            buckets["A-WRONG"].append(row)
        else:
            buckets["A-harmless"].append(row)

    out = os.path.join("local", "output", "const_load_widths.txt")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8") as fh:
        for bucket in ("A-WRONG", "B", "A-unreferenced", "B-unreferenced",
                       "A-harmless"):
            fh.write("=== %s (%d) ===\n" % (bucket, len(buckets[bucket])))
            for path, line_no, name, width, declared, f, d, sites, sizes in buckets[bucket]:
                shown = ", ".join("%08x %s m%d" % (va, mn, sz * 8)
                                  for va, mn, sz in sites[:4])
                fh.write("%s:%d  %s  declared %s=%g  float=%g double=%g  "
                         "sizes=%s  %s\n"
                         % (path, line_no, name, width, declared, f, d,
                            sizes or "none", shown))

    for bucket in ("A-WRONG", "B"):
        for path, line_no, name, width, declared, f, d, sites, sizes in buckets[bucket]:
            # The value the CODE reads: whatever width its loads use. Mixed
            # widths mean the address is shared and both are named.
            if sizes == [4]:
                reads = "m32 -> %g" % f
            elif sizes == [8]:
                reads = "m64 -> %g" % d
            else:
                reads = "mixed %s -> m32 %g / m64 %g" % (sizes, f, d)
            print("%-10s %s:%d %s declared %s=%g; loads read %s"
                  % (bucket, path, line_no, name, width, declared, reads))
    print("checked %d, mismatched %d: A-WRONG %d, B %d, A-unreferenced %d, "
          "B-unreferenced %d, A-harmless %d"
          % (checked, len(misses), len(buckets["A-WRONG"]), len(buckets["B"]),
             len(buckets["A-unreferenced"]), len(buckets["B-unreferenced"]),
             len(buckets["A-harmless"])))
    print("full table: %s" % out)
    return 1 if buckets["A-WRONG"] or buckets["B"] else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
