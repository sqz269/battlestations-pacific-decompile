"""Print each named function's listing on one line (up to its first RET), for side-by-side reading."""
import json, sys, pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
root = r"J:\PROG\battlestations-pacific-decompile-cc13-seg59"
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase; img = pe.get_memory_mapped_image()
md = Cs(CS_ARCH_X86, CS_MODE_32)
for s in sys.argv[1:]:
    a = int(s, 16)
    parts = []
    for addr, sz, mn, ops in md.disasm_lite(img[a - base:a - base + 400], a):
        parts.append(f"{mn} {ops}".replace("dword ptr ", "").replace("byte ptr ", "b").strip())
        if mn == "ret" or len(parts) > 60:
            break
    print(s, "|", "; ".join(parts))
