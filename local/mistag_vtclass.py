import json, re, sqlite3, struct, collections, pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
root = r"J:\PROG\battlestations-pacific-decompile-cc13-seg59"
c = sqlite3.connect(root + r"\local\bsp_index.sqlite")
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase; img = pe.get_memory_mapped_image()
md = Cs(CS_ARCH_X86, CS_MODE_32)
info = {a: (n, cat, ln, rs) for a, n, cat, ln, rs in c.execute("select address, name, tag_category, ledger_name, recon_status from functions")}
LIB = re.compile(r"^(STL_|CG_|LIBCRT|Catch_All|Unwind@|_|operator|std::|`|\?\?|CRT_|EH_|__|thunk_|lua|inflate|deflate|zlib|XLive|XNet|xlive|TRIV_|FID_|iostdnet|TELEMETRY|FUN_)", re.I)
d = json.load(open(root + r"\local\mistag_vtscan2.json"))
rows = []
for sid, a, n, cat, v in d["vtstore"]:
    A = int(a, 16)
    if info.get(A, (0, 0, 0, None))[3] is not None: continue      # already recorded
    vt = int(v, 16)
    game = lib = 0
    for k in range(40):
        s = struct.unpack_from("<I", img, vt - base + 4 * k)[0]
        if not (0x401000 <= s < 0xCE2000): break
        nm = (info.get(s, (None,) * 4)[2] or info.get(s, (None,) * 4)[0] or "")
        if nm.startswith("BSP_") or (info.get(s, (0, 0, 0, None))[3] and not LIB.match(nm)): game += 1
        elif nm and LIB.match(nm) and not nm.startswith(("CG_", "TRIV_", "FUN_")): lib += 1
    ins = []
    for i in md.disasm_lite(img[A - base:A - base + 64], A):
        ins.append(i[2])
        if i[2] in ("ret", "jmp"): break
    shape = "vtreset" if ins[:2] == ["mov", "ret"] else ("scalar_trivial" if len(ins) <= 12 and ins.count("call") == 1 else "other")
    rows.append((sid, a, n, cat, v, game, lib, shape))
cnt = collections.Counter((r[7], "game" if r[5] else ("lib" if r[6] else "unknown")) for r in rows)
for k, v in sorted(cnt.items()): print(k, v)
json.dump(rows, open(root + r"\local\mistag_vtclass.json", "w"), indent=0)
for r in rows:
    if r[7] == "other": print("  other", *r[:5], r[5], r[6])
