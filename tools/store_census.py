"""Promoted from a worker local/ script (packet cc8_torpedo_attack_mode_lowering, 2026-09-18).
Run from a worktree root with a built local/bsp_index.sqlite; needs the pefile module.

Exhaustive census of every store form to a given structure offset.

Usage: python tools/store_census.py 0x424 [0x...]
Covers disp8 and disp32 encodings so a small offset is not missed, which is the
trap that makes a disp32-only scan vacuous.
"""
import sqlite3
import struct
import sys

import pefile

EXE = r"I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe"

FORMS = [
    ("MOV dword imm", b"\xc7", 0x38, 0x00),
    ("MOV dword reg", b"\x89", 0x00, 0x00),
    ("MOV byte imm", b"\xc6", 0x38, 0x00),
    ("MOV byte reg", b"\x88", 0x00, 0x00),
    ("MOVSS", b"\xf3\x0f\x11", 0x00, 0x00),
    ("FSTP", b"\xd9", 0x38, 0x18),
    ("FST", b"\xd9", 0x38, 0x10),
    ("FISTP", b"\xdb", 0x38, 0x18),
    ("INC dword", b"\xff", 0x38, 0x00),
    ("DEC dword", b"\xff", 0x38, 0x08),
    ("ADD dword imm", b"\x83", 0x38, 0x00),
    ("SUB dword imm", b"\x83", 0x38, 0x28),
]


def main():
    offs = [int(a, 0) for a in sys.argv[1:]]
    pe = pefile.PE(EXE, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    for s in pe.sections:
        if s.Name.rstrip(b"\0") == b".text":
            va0, data = base + s.VirtualAddress, s.get_data()
    db = sqlite3.connect("local/bsp_index.sqlite")

    def owner(a):
        r = db.execute("SELECT address, name FROM functions WHERE address <= ? "
                       "ORDER BY address DESC LIMIT 1", (a,)).fetchone()
        return ("%08x %s" % (r[0], r[1])) if r else "?"

    for off in offs:
        print("=== offset %#x ===" % off)
        d8 = off if -128 <= off <= 127 else None
        d32 = struct.pack("<i", off)
        hits = []
        for name, op, mask, match in FORMS:
            n = len(op)
            i = 0
            while True:
                i = data.find(op, i)
                if i < 0:
                    break
                j = i + n
                if j < len(data):
                    m = data[j]
                    if (m & mask) == match and (m & 0x07) != 0x04:
                        if (m & 0xC0) == 0x80 and data[j + 1:j + 5] == d32:
                            hits.append((va0 + i, name + " disp32"))
                        elif (m & 0xC0) == 0x40 and d8 is not None \
                                and data[j + 1] == (d8 & 0xFF):
                            hits.append((va0 + i, name + " disp8"))
                i += 1
        hits.sort()
        for a, name in hits:
            print("  %08x  %-18s in %s" % (a, name, owner(a)))
        print("  total %d" % len(hits))


main()
