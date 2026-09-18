"""Group every [ESP+n] access in a function by the stack slot it really means.

tools/stack_frame_walk.py prints a depth per instruction but stops accounting
at an indirect CALL and at a callee it cannot resolve, so from the first such
site onward its depths drift upward by whatever those callees pop.  Two
accesses to one slot then look like two different slots, and a backward trace
from a read lands on the wrong write.

This tool replays that walk and applies a cleanup per call site, so a read and
a write can be matched.  Supply one --pop per site the walker flags:

    python tools/frame_slot_census.py 009d15f0 \\
        --pop 009d1679=0 --pop 009d1714=4 --pop 009d1a94=24 ...

A __thiscall virtual is callee-clean and pops exactly what the walker reports
as "pushed since the last call", so that figure is the right value for an
indirect site.  For a named callee, read its RET imm16.  Run with no --pop
first: the walker's own flags name every site that needs one.

    --slot K   restrict the output to one corrected slot key

Slot keys are `depth - n` after correction; they are comparable within one run
and have no meaning across functions.  A slot with several writers is normal,
because the compiler reuses stack space: use tools/frame_slot_dominance.py to
find out which of them actually reaches a given read.
"""
import argparse
import re
import subprocess
import sys

ROW = re.compile(r"^([0-9a-f]{8}): (.*?)\s+depth=(-?\d+)")
MEM = re.compile(r"\[ESP(?:\s*\+\s*(0x[0-9a-f]+|\d+))?\]")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("address")
    ap.add_argument("--pop", action="append", default=[],
                    metavar="ADDR=BYTES",
                    help="bytes a callee pops that the walker did not subtract")
    ap.add_argument("--slot", type=int, default=None)
    ap.add_argument("--limit", type=int, default=4000)
    args = ap.parse_args()

    fixups = {}
    for item in args.pop:
        addr, _, count = item.partition("=")
        fixups[addr.lower().lstrip("0x").rjust(8, "0")] = int(count)

    raw = subprocess.run(
        [sys.executable, "tools/stack_frame_walk.py", args.address,
         "--limit", str(args.limit)],
        capture_output=True, text=True).stdout
    if not raw.strip():
        print("stack_frame_walk.py produced nothing", file=sys.stderr)
        return 1

    correction = 0
    by_slot = {}
    flagged = []
    for line in raw.splitlines():
        m = ROW.match(line)
        if not m:
            continue
        addr, insn = m.group(1), m.group(2).strip()
        depth = int(m.group(3))
        if "INDIRECT CALL" in line or "UNRESOLVED CALLEE" in line:
            if addr not in fixups:
                flagged.append((addr, insn, line.split("[", 1)[-1].rstrip("]")))
        mm = MEM.search(insn)
        if mm:
            off = mm.group(1)
            n = 0 if off is None else int(off, 16) if off.startswith("0x") else int(off)
            by_slot.setdefault((depth - correction) - n, []).append((addr, insn))
        if addr in fixups:
            correction += fixups[addr]

    if flagged:
        print("sites the walker could not account for, and no --pop was given:")
        for addr, insn, why in flagged:
            print(f"  {addr}: {insn}   {why}")
        print("depths after the first of these are NOT comparable; supply --pop.\n")

    for key in sorted(by_slot):
        if args.slot is not None and key != args.slot:
            continue
        print(f"--- frame slot K={key} ({len(by_slot[key])} accesses)")
        for addr, insn in by_slot[key]:
            print(f"  {addr}: {insn}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
