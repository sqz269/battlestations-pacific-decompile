"""Part B check (cc13-mistag): the open library-tagged `MOV [ECX], imm32; RET` rows.

For every vt-reset row of tools/cc13_mistag_candidates.json (the frozen output of tools/cc13_mistag_vtclass.py, taken before any row was recorded) this re-reads the disk bytes and requires
C7 01 <imm32> C3 with imm32 equal to the vtable the scan recorded. It classes the vtable
game / lib / unknown from its other slots (the rule of tools/cc13_mistag_vtclass.py, plus MANUAL), looks up the
vtable's slot 0 and records whether that slot-0 routine reaches the row (E8/E9 rel32 to it within
its first 64 bytes), and lists the row's callers from the index. It refuses rows in the struck
ranges and 006C5000-006C5FFF (recorded rows are kept, so the
check can be re-run after the records exist). Output: local/mistag_partB.json, and a summary on stdout.
"""
import json, re, sqlite3, struct, sys, pefile

root = str(__import__("pathlib").Path(__file__).resolve().parents[1])
c = sqlite3.connect(root + r"\local\bsp_index.sqlite")
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
info = {a: (n, cat, ln, rs) for a, n, cat, ln, rs in
        c.execute("select address, name, tag_category, ledger_name, recon_status from functions")}
LIB = re.compile(r"^(STL_|CG_|LIBCRT|Catch_All|Unwind@|_|operator|std::|`|\?\?|CRT_|EH_|__|thunk_|lua|"
                 r"inflate|deflate|zlib|XLive|XNet|xlive|TRIV_|FID_|iostdnet|TELEMETRY|FUN_)", re.I)
STRUCK = [(0x007DF000, 0x007DFFFF), (0x007EE000, 0x007F2FFF), (0x00836000, 0x00837FFF),
          (0x0084E000, 0x0084EFFF), (0x006C5000, 0x006C5FFF)]


def u32(va):
    return struct.unpack_from("<I", img, va - base)[0]


# Hand classifications that the slot-name rule gets wrong (rev4): 00D05840 has no RTTI locator
# ([00D0583C] = 007B45E0 is code) and its slots are __purecall, 007B5920, __purecall, __purecall;
# 007B5920 is a game deleting destructor (segment 47) that stores 00D05840 itself.
MANUAL = {0x00D05840: "game"}


def vt_class(vt):
    klass, game, lib = vt_class_slots(vt)
    if klass != "game" and vt not in MANUAL and game_storers(vt):
        klass = "base"                             # a base vtable that game classes stamp
    return MANUAL.get(vt, klass), game, lib


def game_storers(vt):
    """Game-named functions (BSP_, other than the BSP_Vt*_Destruct rows themselves) whose code
    stores vt: a base vtable whose slots are only its deleting destructor and __purecall (rev2
    nit: 00CF88F8 = 006CDD40, __purecall) takes its class from the classes that stamp it."""
    out = []
    for f in STORERS.get(vt, ()):
        nm = (info.get(f, (None,) * 4)[2] or info.get(f, (None,) * 4)[0] or "")
        if nm.startswith("BSP_") and not nm.startswith("BSP_Vt"):
            out.append(f)
    return out


STARTS = sorted(info)
STOPS = dict(c.execute("select func, stop from ranges"))
STORERS = {}


def stored_vtables():
    """Every imm32 that code stores with `MOV r/m32, imm32` (C7 /0) and that points into the
    vtable region, plus the index's vtables table: the walk stops at the next one (rev2 nit:
    00CF88F8 has two slots; 00CF8900 is a separate vtable stored at 006CDD8F). Also fills
    STORERS: vtable -> the functions whose code holds the store."""
    import bisect
    text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    lo, hi = text.VirtualAddress, text.VirtualAddress + text.Misc_VirtualSize
    code, found = img[lo:hi], {a for (a,) in c.execute("select address from vtables")}
    for i in range(len(code) - 6):
        if code[i] != 0xC7 or (code[i + 1] >> 3) & 7:
            continue
        mod, rm, n = code[i + 1] >> 6, code[i + 1] & 7, 2
        if mod == 3:
            continue
        if rm == 4:
            n += 1
            if mod == 0 and code[i + 2] & 7 == 5:
                n += 4
        n += {0: 4 if rm == 5 else 0, 1: 1, 2: 4}[mod]
        if i + n + 4 <= len(code):
            v = struct.unpack_from("<I", code, i + n)[0]
            if 0xCE2000 <= v < 0xE00000:
                found.add(v)
                va = base + lo + i
                f = STARTS[bisect.bisect_right(STARTS, va) - 1]
                if va < STOPS.get(f, f):                   # inside the body, not an undefined chunk after it
                    STORERS.setdefault(v, set()).add(f)
    # rev4 B1: the register form, `MOV r32, imm32` (B8+r) then at once `MOV r/m32, r32` (89 /r)
    # with the same register and a memory operand (0084A0B5 MOV EDI,00D0B6A0; MOV [ESP+3Ch],EDI).
    # A store whose register is written further away is still missed.
    for i in range(len(code) - 7):
        r = code[i] - 0xB8
        if not 0 <= r < 8 or code[i + 5] != 0x89:
            continue
        modrm = code[i + 6]
        if modrm >> 6 == 3 or (modrm >> 3) & 7 != r:
            continue
        v = struct.unpack_from("<I", code, i + 1)[0]
        if 0xCE2000 <= v < 0xE00000 and v in found:     # only vtables the C7 form or the index know
            va = base + lo + i
            f = STARTS[bisect.bisect_right(STARTS, va) - 1]
            if va < STOPS.get(f, f):
                STORERS.setdefault(v, set()).add(f)
    return found


STORED = stored_vtables()


def vt_class_slots(vt):
    game = lib = 0
    for k in range(40):
        s = u32(vt + 4 * k)
        if not (0x401000 <= s < 0xCE2000) or (k and vt + 4 * k in STORED):
            break
        rec = info.get(s, (None,) * 4)
        nm = rec[2] or rec[0] or ""
        if nm.startswith("BSP_") or (rec[3] and not LIB.match(nm)):
            game += 1
        elif nm and LIB.match(nm) and not nm.startswith(("CG_", "TRIV_", "FUN_")):
            lib += 1
    return ("game" if game else "lib" if lib else "unknown"), game, lib


def reaches(src, dst):
    code = img[src - base:src - base + 64]
    for i in range(len(code) - 4):
        if code[i] in (0xE8, 0xE9) and src + i + 5 + struct.unpack_from("<i", code, i + 1)[0] == dst:
            return True
    return False


rows = json.load(open(root + r"\tools\cc13_mistag_candidates.json"))
# Row 120 (rev4): outside the 16 scanned segments (segment 34); the out-of-line copy of the
# 00CF88F8 store that 006CDD40 inlines.
rows.append([34, "006fbc10", "TRIV_body_006fbc10", "trivial_body", "00cf88f8", 0, 0, "vtreset"])
out, bad = [], []
for sid, a, n, cat, v, *_ in rows:
    A, vt = int(a, 16), int(v, 16)
    b = img[A - base:A - base + 7]
    shape_ok = b[0] == 0xC7 and b[1] == 0x01 and b[6] == 0xC3
    if not shape_ok and _[-1] != "vtreset":
        continue                                   # a wrapper or irregular row: not part B
    if not shape_ok or struct.unpack_from("<I", b, 2)[0] != vt:
        bad.append((a, "bytes", b.hex()))
        continue
    if any(lo <= A <= hi for lo, hi in STRUCK):
        bad.append((a, "struck", ""))
        continue
    klass, game, lib = vt_class(vt)
    slot0 = u32(vt)
    callers = [f"{x:08x}" for (x,) in c.execute(
        "select distinct caller from calls where callee = ? order by caller", (A,))]
    out.append({"segment": sid, "address": a, "name": n, "tag": cat, "vtable": f"{vt:08x}",
                "class": klass, "status": info.get(A, (0, 0, 0, None))[3], "game_slots": game, "lib_slots": lib, "slot0": f"{slot0:08x}",
                "slot0_reaches": reaches(slot0, A), "callers": callers})
json.dump({"rows": out, "refused": bad}, open(root + r"\local\mistag_partB.json", "w"), indent=0)
by = {}
for r in out:
    by.setdefault(r["class"], []).append(r)
print({k: len(v) for k, v in by.items()}, "refused", len(bad))
print("game rows whose slot 0 does not reach them:",
      sum(1 for r in by.get("game", []) if not r["slot0_reaches"]))
for k in ("lib", "unknown"):
    for r in by.get(k, []):
        print(k, r["address"], r["name"], r["vtable"], r["game_slots"], r["lib_slots"])
for x in bad:
    print("refused", *x)
