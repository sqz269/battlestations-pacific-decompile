"""Probe for the 21 irregular rows: which vtable slots hold each row, which recorded functions
store each vtable (C7 xx <vt>), and whether the five 00D03720 rows are byte-identical (rel32 masked)."""
import json, sqlite3, struct, pefile
root = str(__import__("pathlib").Path(__file__).resolve().parents[1])
c = sqlite3.connect(root + r"\local\bsp_index.sqlite")
pe = pefile.PE(json.load(open(root + r"\config\target.json"))['binary'], fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase; img = pe.get_memory_mapped_image()
info = {a: (n, ln, rs) for a, n, ln, rs in c.execute("select address, name, ledger_name, recon_status from functions")}
ranges = sorted(c.execute("select func, stop from ranges"))
import bisect
starts = [r[0] for r in ranges]


def containing(va):
    i = bisect.bisect_right(starts, va) - 1
    if i >= 0 and ranges[i][0] <= va < ranges[i][1]:
        return ranges[i][0]
    return None


def refs(value):
    pat, out, i = struct.pack("<I", value), [], img.find(struct.pack("<I", value))
    while i != -1 and len(out) < 40:
        out.append(base + i)
        i = img.find(pat, i + 1)
    return out


rows = {"00729500": 0xcfdbfc, "0072ac70": 0xcfdc0c, "007d7ff0": 0xd06848, "008444e0": 0xd0b6a0,
        "00851f20": 0xd0beac, "009be0e0": 0xd209c4, "009d91d0": 0xd214f4, "009daaa0": 0xd056d0,
        "009dac30": 0xd056d0, "009db000": 0xd056d0, "009dbef0": 0xd2152c, "009e8420": 0xd056d0,
        "00743e50": 0xcff358, "0074ec00": 0xd00514, "007669f0": 0xd03720, "00767500": 0xd03754,
        "00767970": 0xd03720, "00767b90": 0xd03720, "00767cc0": 0xd03720, "00767dd0": 0xd03720,
        "00950380": 0xd19b74}
for a, vt in rows.items():
    A = int(a, 16)
    slots = [f"{r:08x}" for r in refs(A) if 0xCE0000 <= r - 0 < 0xD80000 + base and r >= 0xCE2000]
    stores = []
    for r in refs(vt):
        if img[r - base - 2] == 0xC7 or img[r - base - 3] == 0xC7:
            f = containing(r)
            if f is not None and f != A:
                stores.append(f"{f:08x}:{(info.get(f, ('?', None, None))[1] or info.get(f, ('?',))[0])}:{info.get(f, (0, 0, None))[2]}")
    print(a, "rec=", info.get(A, (None, None, None))[2], "slots=", slots[:6])
    print("    stores of", f"{vt:08x}", ":", sorted(set(stores))[:8], "n=", len(set(stores)))


def masked(a, n):
    b = bytearray(img[a - base:a - base + n])
    for i in range(n - 4):
        if b[i] == 0xE8:
            b[i + 1:i + 5] = b"\0\0\0\0"
    return bytes(b)


ref = masked(0x007669F0, 0x5B)
for a in (0x00767970, 0x00767B90, 0x00767CC0, 0x00767DD0):
    print(f"{a:08x} identical to 007669F0:", masked(a, 0x5B) == ref)
