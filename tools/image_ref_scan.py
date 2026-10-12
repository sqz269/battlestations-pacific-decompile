"""Find E8/E9 rel32 and absolute-dword references to target addresses in the game exe. Read-only."""
import struct
import sys

EXE = r"I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe"
data = open(EXE, "rb").read()
pe = struct.unpack_from("<I", data, 0x3C)[0]
nsec = struct.unpack_from("<H", data, pe + 6)[0]
opt = struct.unpack_from("<H", data, pe + 20)[0]
base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
secs = []
for i in range(nsec):
    o = pe + 24 + opt + i * 40
    name = data[o:o + 8].rstrip(b"\0").decode()
    vsz, va, rsz, rptr = struct.unpack_from("<IIII", data, o + 8)
    secs.append((name, va, vsz, rptr, rsz))

targets = [int(a, 16) for a in sys.argv[1:]]
tset = set(targets)
for name, va, vsz, rptr, rsz in secs:
    blob = data[rptr:rptr + rsz]
    for i in range(len(blob) - 4):
        d = struct.unpack_from("<I", blob, i)[0]
        if d in tset:
            print(f"abs {base + va + i:08x} {name} -> {d:08x}")
        if name == ".text" and i > 0 and blob[i - 1] in (0xE8, 0xE9):
            t = (base + va + i + 4 + struct.unpack_from("<i", blob, i)[0]) & 0xFFFFFFFF
            if t in tset:
                print(f"rel {base + va + i - 1:08x} {'call' if blob[i-1]==0xE8 else 'jmp'} -> {t:08x}")
