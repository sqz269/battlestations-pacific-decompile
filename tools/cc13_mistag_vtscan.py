"""Widened form of rev-convoy's mt_dropped_vt.py (copied as mistag_dropped_vt.py).

Scans every open library-tagged row of the 16 segments (not only the filter-dropped ones) for
game-class vtable forms read from the disk bytes:
  VT+JMP out-of-line dtors: C7 01 imm32, then any of C7 81 d32 imm32 / C7 41 d8 imm32 /
  83 C1 xx / 8D 49 xx, then E9 rel32 (or 8B 49 d8 / 8B 89 d32 before the E9 is not accepted);
  early vtable stores: C7 06|C7 01|C7 46 d8|C7 41 d8 imm32 with imm in .rdata (00CE2000-00E07FFF)
  within the first 32 bytes.
Prints rows not already reconstructed or bound in this tree's index."""
import json, re, sqlite3, struct
root = str(__import__("pathlib").Path(__file__).resolve().parents[1])
c = sqlite3.connect(root + r"\local\bsp_index.sqlite")
import pefile
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase; img = pe.get_memory_mapped_image()
SEGS = [40, 47, 48, 52, 53, 67, 59, 51, 58, 30, 41, 42, 50, 61, 54, 46]
STRUCK = [(0x007DF000, 0x007E0000), (0x007EE000, 0x007F3000), (0x00836000, 0x00838000),
          (0x0084E000, 0x0084F000), (0x006C5000, 0x006C6000)]
LIB = re.compile(r"^(STL_|CG_|LIBCRT|Catch_All|Unwind@|_|operator|std::|`|\?\?|CRT_|EH_|__|thunk_STL|thunk_CG|lua|inflate|deflate|zlib|XLive|XNet|xlive|TRIV_|FID_|iostdnet|TELEMETRY)", re.I)
funclets = {r[0] for r in c.execute("select funclet from funclet_parent")}
def isvt(v): return 0x00CE2000 <= v < 0x00E08000
out = {"vtjmp": [], "vtstore": []}
for sid in SEGS:
    lo, hi = c.execute("select start, end from segments where id = ?", (sid,)).fetchone()
    for a, n, cat, ln in c.execute("select address, name, tag_category, ledger_name from functions where address >= ? and address < ? and recon_status is null", (lo, hi)).fetchall():
        nm = ln or n
        if a in funclets or any(s <= a < e for s, e in STRUCK): continue
        if not (cat and cat != "game_code") and not LIB.match(nm or ""): continue
        x = img[a - base:a - base + 48]
        if x[:2] == b"\xC7\x01" and isvt(struct.unpack_from("<I", x, 2)[0]):
            j = 6; ok = True
            while True:
                if x[j:j + 2] == b"\xC7\x81": j += 10
                elif x[j:j + 2] == b"\xC7\x41": j += 7
                elif x[j:j + 2] in (b"\x83\xC1", b"\x8D\x49"): j += 3
                else: break
            if x[j] == 0xE9:
                t = a - base + j + 5 + struct.unpack_from("<i", x, j + 1)[0] + base
                out["vtjmp"].append((sid, f"{a:08x}", nm, f"{struct.unpack_from('<I', x, 2)[0]:08x}", f"{t:08x}"))
                continue
        for m in re.finditer(rb"\xC7(?:\x06|\x01)(....)|\xC7(?:\x46|\x41)(.)(....)", x[:32], re.S):
            v = struct.unpack("<I", m.group(1) or m.group(3))[0]
            if isvt(v):
                out["vtstore"].append((sid, f"{a:08x}", nm, cat, f"{v:08x}")); break
print("VT+JMP:", len(out["vtjmp"]))
for r in out["vtjmp"]: print("  ", *r)
print("early vtable stores:", len(out["vtstore"]))
json.dump(out, open(root + r"\local\mistag_vtscan2.json", "w"), indent=0)
import collections
print(collections.Counter(r[3] for r in out["vtstore"]))
