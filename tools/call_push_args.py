"""For each call to 00B6A020, print the last PUSH before it (the library mask). Read-only."""
import struct
import collections
import capstone

EXE = r"I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe"
data = open(EXE, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
opt = struct.unpack_from("<H", data, pe + 20)[0]
base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
for i in range(nsec):
    o = pe + 24 + opt + i * 40
    if data[o:o + 5] == b".text":
        vsz, va, rsz, rptr = struct.unpack_from("<IIII", data, o + 8)
        text = data[rptr:rptr + rsz]
        tva = base + va
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
import sys
target = int(sys.argv[1], 16)
sites = []
for i in range(1, len(text) - 4):
    if text[i - 1] == 0xE8:
        t = (tva + i + 4 + struct.unpack_from("<i", text, i)[0]) & 0xFFFFFFFF
        if t == target:
            sites.append(tva + i - 1)
hist = collections.Counter()
for s in sites:
    best = None
    for back in range(6, 40):
        start = s - back
        ins = list(md.disasm(text[start - tva:s - tva + 5], start))
        if ins and ins[-1].address == s:
            pushes = [x for x in ins if x.mnemonic == "push"]
            if pushes:
                best = " / ".join(x.op_str for x in pushes[-2:])
                break
    hist[best] += 1
    print(f"{s:08x} mask {best}")
print(dict(hist))
