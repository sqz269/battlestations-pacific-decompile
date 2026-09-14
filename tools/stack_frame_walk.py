"""Normalise `[ESP+N]` operands in one function to entry-relative frame offsets.

A literal `[ESP+1Ch]` does not name the same storage at two points in a function:
every `PUSH`, `SUB ESP`, `ADD ESP`, `POP` and callee-cleaned `CALL` moves the
base. This project has had that produce a wrong reading twice -
`docs/PILOT_BOT_PLAN_CONTROLS.md` found `[ESP+1Ch]` was really frame slot
`[ESP+14h]` because a `SUB ESP,8` was live at the read, and
`docs/PLANE_CONTROL_RATE_LAW.md` stopped short of a law for the same reason.

This walks the listing once, tracks ESP against its value on entry (so `frame=0`
is the return address and `frame=4` the first stack argument), and rewrites every
`[ESP+N]` as `frame=<entry-relative>`. Two reads of different literals that print
the same `frame=` are the same slot; the same literal printing two `frame=`
values is the trap.

Callee-cleaned calls are handled by looking up each `CALL <abs>` target's `RET
imm16` through `bsp.py ghidra proto` and subtracting it. A call whose target
cannot be resolved is reported and the walk marks everything after it
`frame=?` rather than guessing - an unknown cleanup makes every later offset
unknowable, and silently carrying on is exactly how the trap bites.

Usage:
    python tools/stack_frame_walk.py <function-address> [--limit N] [--filter ESP]

Read-only: it shells out to `bsp.py ghidra disasm` and `ghidra proto` and writes
nothing.
"""
import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
LINE = re.compile(r'^([0-9a-f]{8}):\s+(.*?)\s*$')
ESP_OPERAND = re.compile(r'\[ESP(?:\s*\+\s*(0x[0-9a-f]+|\d+))?\]', re.I)
CALL_ABS = re.compile(r'^CALL\s+0x([0-9a-f]{8})$', re.I)
RET_IMM = re.compile(r'\bRET\s+(0x[0-9a-f]+|\d+)', re.I)


def run(args, timeout=600):
    try:
        out = subprocess.run([sys.executable, str(ROOT / 'tools' / 'bsp.py')] + args,
                             capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return ''
    return out.stdout


def imm(text):
    if text is None:
        return 0
    return int(text, 16) if text.lower().startswith('0x') else int(text)


def callee_cleanup(address, cache):
    """Bytes the callee pops, from its own `RET imm16`. None when unresolved."""
    if address in cache:
        return cache[address]
    text = run(['ghidra', 'disasm', address, '--limit', '400'])
    found = None
    for line in text.splitlines():
        match = RET_IMM.search(line)
        if match:
            found = imm(match.group(1))
            break
        if re.search(r'\bRET\b', line):
            found = 0
            break
    cache[address] = found
    return found


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('address')
    parser.add_argument('--limit', type=int, default=700)
    parser.add_argument('--filter', default='', help='only print lines containing this')
    args = parser.parse_args()

    text = run(['ghidra', 'disasm', args.address, '--limit', str(args.limit)])
    lines = []
    for raw in text.splitlines():
        match = LINE.match(raw.strip())
        if match:
            lines.append((match.group(1), match.group(2)))
    # bsp.py caps its own output at ~2000 tokens and writes the whole result to
    # local/output/. The capped stdout parses fine, so a "did we get anything"
    # check silently walks a truncated function - take the spill whenever it is
    # longer.
    spills = sorted((ROOT / 'local' / 'output').glob(
        'ghidra-disasm-%s-*' % args.address), key=lambda p: p.stat().st_mtime)
    if spills:
        spilled = []
        for raw in spills[-1].read_text(encoding='utf-8',
                                        errors='replace').splitlines():
            match = LINE.match(raw.strip())
            if match:
                spilled.append((match.group(1), match.group(2)))
        if len(spilled) > len(lines):
            lines = spilled
    if not lines:
        print('no disassembly for %s' % args.address)
        return 1

    # depth counts bytes ESP has moved BELOW its value on entry, so an operand
    # [ESP+N] names entry-relative offset N - depth... with depth positive down.
    depth = 0
    unknown = False
    cache = {}
    for address, text_ in lines:
        note = ''
        upper = text_.upper()
        before = depth

        for operand in ESP_OPERAND.finditer(text_):
            if unknown:
                note += '  frame=?'
                break
            note += '  frame=%d(0x%x)' % (imm(operand.group(1)) - depth,
                                          (imm(operand.group(1)) - depth) & 0xffffffff)

        if upper.startswith('PUSH'):
            depth += 4
        elif upper.startswith('POP'):
            depth -= 4
        elif upper.startswith('SUB ESP,'):
            depth += imm(text_.split(',')[1].strip())
        elif upper.startswith('ADD ESP,'):
            depth -= imm(text_.split(',')[1].strip())
        else:
            call = CALL_ABS.match(text_)
            if call:
                cleanup = callee_cleanup(call.group(1), cache)
                if cleanup is None:
                    unknown = True
                    note += '  [UNRESOLVED CALLEE - frame unknown after this]'
                else:
                    depth -= cleanup
                    if cleanup:
                        note += '  [callee pops %d]' % cleanup
            elif 'CALL' in upper:
                unknown = True
                note += '  [INDIRECT CALL - cleanup unknown, frame unknown after this]'

        if args.filter and args.filter.upper() not in upper:
            continue
        print('%s: %-42s depth=%-5d%s' % (address, text_, before, note))
    return 0


if __name__ == '__main__':
    sys.exit(main())
