"""Part B check (cc13-mistag): the open library-tagged `MOV [ECX], imm32; RET` rows.

For every vt-reset row of local/mistag_vtclass.json this re-reads the disk bytes and requires
C7 01 <imm32> C3 with imm32 equal to the vtable the scan recorded. It classes the vtable
game / lib / unknown from its other slots (the rule of local/mistag_vtclass.py), looks up the
vtable's slot 0 and records whether that slot-0 routine reaches the row (E8/E9 rel32 to it within
its first 64 bytes), and lists the row's callers from the index. It refuses rows in the struck
ranges and 006C5000-006C5FFF. Output: local/mistag_partB.json, and a summary on stdout.
"""
import json, re, sqlite3, struct, sys, pefile

root = r"J:\PROG\battlestations-pacific-decompile-cc13-seg59"
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


def vt_class(vt):
    game = lib = 0
    for k in range(40):
        s = u32(vt + 4 * k)
        if not (0x401000 <= s < 0xCE2000):
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


rows = json.load(open(root + r"\local\mistag_vtclass.json"))
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
    if info.get(A, (0, 0, 0, None))[3] is not None:
        bad.append((a, "recorded", info[A][3]))
        continue
    klass, game, lib = vt_class(vt)
    slot0 = u32(vt)
    callers = [f"{x:08x}" for (x,) in c.execute(
        "select distinct caller from calls where callee = ? order by caller", (A,))]
    out.append({"segment": sid, "address": a, "name": n, "tag": cat, "vtable": f"{vt:08x}",
                "class": klass, "game_slots": game, "lib_slots": lib, "slot0": f"{slot0:08x}",
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
