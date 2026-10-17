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
    python tools/const_width_sweep.py --all --exact  # bit-exact against each load site

Exit code is 1 when any constant mismatches its declared width.

--exact has no tolerance. For every constant whose address has load sites, it
takes the bits the compiler stores for the declared type (a float literal is
rounded once, from the decimal, to float) and the bits the image holds at the
width each load reads, widens both the way the load widens (FLD m32 -> the
float's exact value), and reports any difference with its ulp distance. The
right-hand side may be a decimal or hex literal, a static_cast of one, or
literal arithmetic; anything naming another identifier is counted as unparsed.
A double declared as the decimal of a float the image widens (0.85 where the
image's FLD m32 gives 0.8500000238418579) is flagged as such. Exit code 1 on any
mismatch; the whole list goes to local/output/const_exact.txt.
"""
import fractions
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


EXACT_PATTERN = re.compile(
    r"\bconstexpr\s+(float|double)\s+(\w+)\s*=\s*([^;]+);\s*//\s*([0-9A-Fa-f]{8})\b")
TOKEN = re.compile(
    r"\s*(?:(0[xX][0-9A-Fa-f]*\.?[0-9A-Fa-f]*[pP][-+]?[0-9]+)([fF]?)"
    r"|([0-9]+\.?[0-9]*(?:[eE][-+]?[0-9]+)?|\.[0-9]+(?:[eE][-+]?[0-9]+)?)([fF]?)"
    r"|(static_cast\s*<\s*(?:float|double)\s*>)|([-+*/()]))")


def round_f32(value):
    """Round an exact Fraction (or double) to the nearest float, ties to even.

    Going through a double first can round twice; this rounds the exact value
    once, which is what the compiler does for a float literal.
    """
    if isinstance(value, float) and value == 0:
        return value  # a Fraction would drop the sign of -0.0
    exact = fractions.Fraction(value)
    near = struct.unpack("<f", struct.pack("<f", float(exact)))[0]
    bits = struct.unpack("<I", struct.pack("<f", near))[0]
    best = None
    for b in (bits - 1, bits, bits + 1):
        b &= 0xFFFFFFFF
        cand = struct.unpack("<f", struct.pack("<I", b))[0]
        if cand != cand or cand in (float("inf"), float("-inf")):
            continue
        err = abs(fractions.Fraction(cand) - exact)
        key = (err, b & 1)
        if best is None or key < best[0]:
            best = (key, cand)
    return best[1]


def eval_literal_expr(text):
    """(value, type) of literal arithmetic as C++ types it, or None.

    float op float stays float (rounded per operation); anything with a double
    is double. An identifier, call or unknown token gives None.
    """
    toks = []
    pos = 0
    text = text.strip()
    while pos < len(text):
        m = TOKEN.match(text, pos)
        if not m or m.end() == pos:
            return None
        hexlit, hexsuf, declit, decsuf, cast, op = m.groups()
        if hexlit:
            v = fractions.Fraction(float.fromhex(hexlit))
            toks.append(("num", (round_f32(v), "float") if hexsuf
                         else (float(v), "double")))
        elif declit:
            v = fractions.Fraction(declit)
            toks.append(("num", (round_f32(v), "float") if decsuf
                         else (float(v), "double")))
        elif cast:
            toks.append(("cast", "float" if "float" in cast else "double"))
        else:
            toks.append(("op", op))
        pos = m.end()

    def arith(a, op, b):
        ty = "float" if a[1] == b[1] == "float" else "double"
        x, y = fractions.Fraction(a[0]), fractions.Fraction(b[0])
        r = {"+": x + y, "-": x - y, "*": x * y, "/": x / y}[op]
        return (round_f32(r), ty) if ty == "float" else (float(r), ty)

    idx = [0]

    def peek():
        return toks[idx[0]] if idx[0] < len(toks) else (None, None)

    def take():
        idx[0] += 1
        return toks[idx[0] - 1]

    def unary():
        kind, val = peek()
        if kind == "op" and val in "+-":
            take()
            v = unary()
            return v if val == "+" else (-v[0], v[1])
        if kind == "num":
            take()
            return val
        if kind == "cast":
            take()
            if take() != ("op", "("):
                raise ValueError
            v = expr()
            if take() != ("op", ")"):
                raise ValueError
            return (round_f32(v[0]), "float") if val == "float" else (float(v[0]), "double")
        if (kind, val) == ("op", "("):
            take()
            v = expr()
            if take() != ("op", ")"):
                raise ValueError
            return v
        raise ValueError

    def term():
        v = unary()
        while peek()[0] == "op" and peek()[1] in "*/":
            v = arith(v, take()[1], unary())
        return v

    def expr():
        v = term()
        while peek()[0] == "op" and peek()[1] in "+-":
            v = arith(v, take()[1], term())
        return v

    try:
        v = expr()
    except (ValueError, IndexError, ZeroDivisionError):
        return None
    return v if idx[0] == len(toks) else None


def ordered(bits, width):
    """Map float bits onto a monotonic integer line for ulp distances."""
    sign = 1 << (width * 8 - 1)
    return bits + sign if bits & sign == 0 else (sign << 1) - 1 - bits


def ulps(a, b, width):
    fmt, ifmt = ("<f", "<I") if width == 4 else ("<d", "<Q")
    ba = struct.unpack(ifmt, struct.pack(fmt, a))[0]
    bb = struct.unpack(ifmt, struct.pack(fmt, b))[0]
    return abs(ordered(ba, width) - ordered(bb, width))


def exact_main(argv):
    args = [a for a in argv if not a.startswith("--")]
    if "--all" in argv:
        args = sorted(glob.glob(os.path.join("include", "bsp", "*.hpp")))
    if not args:
        print(__doc__)
        return 2

    sections = load_image()
    index = load_site_index()
    parsed = unparsed = unreferenced = checked = 0
    rows = []
    for path in args:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line_no, line in enumerate(fh, 1):
                m = EXACT_PATTERN.search(line)
                if not m:
                    continue
                decl, name, rhs, addr_text = m.groups()
                value = eval_literal_expr(rhs)
                if value is None:
                    unparsed += 1
                    continue
                parsed += 1
                # What the compiler stores: the literal converted to the
                # declared type (a double literal into a float rounds once).
                host = round_f32(value[0]) if decl == "float" else float(value[0])
                addr = int(addr_text, 16)
                raw = read8(sections, addr)
                sites = index.get(addr, [])
                if raw is None or not sites:
                    unreferenced += 1
                    continue
                by_width = {}
                for va, mn, size in sites:
                    if size in (4, 8):
                        by_width.setdefault(size, []).append((va, mn))
                for size, wsites in sorted(by_width.items()):
                    checked += 1
                    if size == 4:
                        img = struct.unpack("<f", raw[:4])[0]
                        img_bits = "%08X" % struct.unpack("<I", raw[:4])[0]
                    else:
                        img = struct.unpack("<d", raw)[0]
                        img_bits = "%016X" % struct.unpack("<Q", raw)[0]
                    if img == host and (img != 0 or
                                        struct.pack("<d", img) == struct.pack("<d", host)):
                        continue
                    if decl == "float":
                        host_bits = "%08X" % struct.unpack("<I", struct.pack("<f", host))[0]
                    else:
                        host_bits = "%016X" % struct.unpack("<Q", struct.pack("<d", host))[0]
                    # Distance in the image's width when the host value fits
                    # it exactly, else in doubles (both widen exactly).
                    if size == 4 and decl == "float":
                        dist = ulps(host, img, 4)
                        unit = "float"
                    else:
                        dist = ulps(host, img, 8)
                        unit = "double"
                    note = ""
                    if (decl == "double" and round_f32(img) == img
                            and round_f32(host) == img):
                        # m32 widened by the load, or an m64 the compiler
                        # stored from a float: either way the image's value is
                        # a float and the header wrote its short decimal.
                        note = ("double written as the decimal of a widened "
                                "float: image %r" % img)
                    elif img == 0 and host == 0:
                        note = "signed zero"
                    elif decl == "float" and size == 8 and round_f32(img) == img:
                        note = ("declared float, loads read a double holding a "
                                "float, %d float ulp" % ulps(host, img, 4))
                    elif decl == "float" and size == 8:
                        note = "declared float, loads read a double"
                    elif decl == "double" and size == 4:
                        note = "declared double, loads read a float"
                    rows.append((path, line_no, name, decl, host, host_bits,
                                 addr, size, img, img_bits, dist, unit,
                                 wsites, note))

    out = os.path.join("local", "output", "const_exact.txt")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", encoding="utf-8") as fh:
        for (path, line_no, name, decl, host, host_bits, addr, size, img,
             img_bits, dist, unit, wsites, note) in rows:
            shown = ", ".join("%08X %s" % (va, mn) for va, mn in wsites[:6])
            if len(wsites) > 6:
                shown += ", +%d more" % (len(wsites) - 6)
            fh.write("%s:%d  %s  host %s %s (%r)  image %08X m%d %s (%r)  "
                     "%d %s ulp  loads: %s%s\n"
                     % (path.replace("\\", "/"), line_no, name, decl, host_bits,
                        host, addr, size * 8, img_bits, img, dist, unit, shown,
                        ("  [" + note + "]") if note else ""))
    for row in rows[:40]:
        path, line_no, name, decl, host, host_bits, addr, size, img, img_bits, dist, unit = row[:12]
        print("%s:%d %s %s=%s image %08X m%d=%s %d %s ulp"
              % (path.replace("\\", "/"), line_no, name, decl, host_bits, addr,
                 size * 8, img_bits, dist, unit))
    if len(rows) > 40:
        print("... %d more" % (len(rows) - 40))
    print("parsed %d constants (%d unparsed expressions), %d without load sites, "
          "%d width checks, %d exact mismatches"
          % (parsed, unparsed, unreferenced, checked, len(rows)))
    print("full table: %s" % out)
    return 1 if rows else 0


def main(argv):
    if "--exact" in argv:
        return exact_main(argv)
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
