"""Absolute-dword references (outside vtables) to every Part B row: PUSH imm32 and the like."""
import json, struct, pefile
root = str(__import__("pathlib").Path(__file__).resolve().parents[1])
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase; img = pe.get_memory_mapped_image()
rows = [r for r in json.load(open(root + r"\local\mistag_partB.json"))["rows"] if r["class"] == "game"]
for r in rows:
    a = int(r["address"], 16)
    pat, i, hits = struct.pack("<I", a), None, []
    i = img.find(pat)
    while i != -1:
        hits.append(f"{base + i - 1:08x}:{img[i - 1]:02x}")
        i = img.find(pat, i + 1)
    if hits:
        print(r["address"], hits)
