"""Part C check (cc13-mistag): the open library-tagged scalar deleting destructor wrappers.

For every scalar_trivial row of local/mistag_vtclass.json this re-reads the disk bytes and
requires the whole wrapper shape (31 bytes):
    F6 44 24 04 01        TEST byte [ESP+4], 1
    56                    PUSH ESI
    8B F1                 MOV ESI, ECX
    C7 06 <vt>            MOV [ESI], vt          (the inlined vt-reset destructor)
    74 09                 JZ  +9
    56                    PUSH ESI
    E8 <rel32>            CALL 00BF65AC _free
    83 C4 04              ADD ESP, 4
    8B C6 5E C2 04 00     MOV EAX, ESI; POP ESI; RET 4
with vt equal to the scanned vtable. It classes the vtable as part B does, and finds the
out-of-line copies: every recorded function whose bytes are C7 01 <vt> C3. Refuses the struck
ranges, 006C5000-006C5FFF and rows that already have a record. Output: local/mistag_partC.json.
"""
import json, re, sqlite3, struct, pefile

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
FREE = 0x00BF65AC


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


def copies(vt):
    pat = b"\xC7\x01" + struct.pack("<I", vt) + b"\xC3"
    found, i = [], img.find(pat)
    while i != -1:
        a = base + i
        if a in info and info[a][3] is not None:
            found.append((f"{a:08x}", info[a][2]))
        i = img.find(pat, i + 1)
    return found


rows = json.load(open(root + r"\local\mistag_vtclass.json"))
out, bad = [], []
for sid, a, n, cat, v, *rest in rows:
    if rest[-1] != "scalar_trivial":
        continue
    A, vt = int(a, 16), int(v, 16)
    b = img[A - base:A - base + 27]
    ok = (b[:8] == bytes.fromhex("F6442404 01 56 8BF1".replace(" ", "")) and b[8:10] == b"\xC7\x06"
          and struct.unpack_from("<I", b, 10)[0] == vt and b[14:17] == bytes.fromhex("740956")
          and b[17] == 0xE8 and A + 22 + struct.unpack_from("<i", b, 18)[0] == FREE
          and b[22:] == bytes.fromhex("83C4048BC6") and img[A - base + 27:A - base + 31] == bytes.fromhex("5EC20400"))
    if not ok:
        bad.append((a, "bytes", img[A - base:A - base + 31].hex()))
        continue
    if any(lo <= A <= hi for lo, hi in STRUCK):
        bad.append((a, "struck", ""))
        continue
    if info.get(A, (0, 0, 0, None))[3] is not None:
        bad.append((a, "recorded", info[A][3]))
        continue
    klass, game, lib = vt_class(vt)
    out.append({"segment": sid, "address": a, "name": n, "vtable": f"{vt:08x}", "class": klass,
                "game_slots": game, "lib_slots": lib, "copies": copies(vt)})
json.dump({"rows": out, "refused": bad}, open(root + r"\local\mistag_partC.json", "w"), indent=0)
by = {}
for r in out:
    by.setdefault(r["class"], []).append(r)
print({k: len(v) for k, v in by.items()}, "refused", len(bad))
g = by.get("game", [])
print("game rows citing an out-of-line copy:", sum(1 for r in g if r["copies"]),
      "| citing _free only:", sum(1 for r in g if not r["copies"]))
for k in ("lib", "unknown"):
    for r in by.get(k, []):
        print(k, r["address"], r["name"], r["vtable"], r["game_slots"], r["lib_slots"])
for x in bad:
    print("refused", *x)
