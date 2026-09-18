"""Promoted from a worker local/ script (packet cc8_torpedo_attack_mode_lowering, 2026-09-18).
Run from a worktree root with a built local/bsp_index.sqlite; needs the pefile module.

Exhaustive rel32 CALL/JMP census for a set of targets, plus absolute dwords.

bsp.py ghidra callers under-reports (it returned 25 of 62 real callers of
0077D600 in an earlier packet), so any claim that a census is complete has to
come from the bytes.  This scans .text for E8/E9 rel32 landing on each target
and .text/.rdata/.data for the literal address, and resolves the enclosing
function through local/bsp_index.sqlite.
"""
import sqlite3
import struct
import sys

import pefile

EXE = r"I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe"
TARGETS = [int(a, 16) for a in sys.argv[1:]] or [0x007ed3f0, 0x007ed430]


def main():
    pe = pefile.PE(EXE, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    secs = {}
    for s in pe.sections:
        secs[s.Name.rstrip(b"\0").decode()] = (base + s.VirtualAddress, s.get_data())
    db = sqlite3.connect("local/bsp_index.sqlite")

    def owner(addr):
        row = db.execute(
            "SELECT address, name FROM functions WHERE address <= ? "
            "ORDER BY address DESC LIMIT 1", (addr,)).fetchone()
        return ("%08x %s" % (row[0], row[1])) if row else "?"

    va0, data = secs[".text"]
    for t in TARGETS:
        print("=== %08x ===" % t)
        n = 0
        for i in range(len(data) - 5):
            op = data[i]
            if op not in (0xE8, 0xE9):
                continue
            rel = struct.unpack_from("<i", data, i + 1)[0]
            if va0 + i + 5 + rel == t:
                kind = "CALL" if op == 0xE8 else "JMP "
                print("  %s %08x  in %s" % (kind, va0 + i, owner(va0 + i)))
                n += 1
        lit = struct.pack("<I", t)
        for name in (".text", ".rdata", ".data"):
            if name not in secs:
                continue
            v0, d = secs[name]
            j = 0
            while True:
                j = d.find(lit, j)
                if j < 0:
                    break
                if name != ".text" or d[j - 1] not in (0xE8, 0xE9):
                    print("  ADDR %08x  (%s)%s" % (v0 + j, name,
                          "  in " + owner(v0 + j) if name == ".text" else ""))
                    n += 1
                j += 1
        print("  total %d" % n)


main()
