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
    parser.add_argument('--indirect-pops', type=int, default=None,
                        help='assume every indirect CALL pops this many bytes instead of '
                             'marking the rest of the walk unknown. There is no safe '
                             'default: use the pushed-since-last-call figure the walker '
                             'reports at each indirect call, and only when you have '
                             'established the callee convention.')
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
    # Bytes pushed since the last CALL of any kind. At an indirect call this is
    # the argument block a __stdcall or __thiscall callee would pop, which is the
    # number the analyst needs in order to correct the walk.
    pushed_since_call = 0
    indirect_pops = args.indirect_pops
    crossed_return = False
    warned_return = False
    last_ret = ''
    for address, text_ in lines:
        note = ''
        upper = text_.upper()
        before = depth
        if crossed_return and not warned_return:
            warned_return = True
            print('*** the walk has crossed a RET at %s. This is a LINEAR walk: the '
                  'epilogue it just stepped through popped a depth that belongs to a '
                  'different path, so every depth and frame below here is offset by '
                  'that epilogue. Add back the POPs and ADD ESP of every epilogue '
                  'crossed (4 bytes per POP) to recover the true frame. Relative '
                  'comparisons within one basic block are still sound.' % last_ret)

        for operand in ESP_OPERAND.finditer(text_):
            if unknown:
                note += '  frame=?'
                break
            note += '  frame=%d(0x%x)' % (imm(operand.group(1)) - depth,
                                          (imm(operand.group(1)) - depth) & 0xffffffff)

        # A RET ends a path, and the walk is LINEAR - the next instruction belongs
        # to some other basic block that a jump reaches from above the epilogue.
        # So every epilogue the walk crosses subtracts its own POPs and ADD ESP
        # from a depth that was never theirs. 0099D0A0's two early returns put
        # its whole main body 64 bytes out, which is enough to make the wrong
        # stack slot look like the right one. The walk cannot fix this without a
        # control-flow graph, so it says so, loudly, once per crossing.
        if upper.startswith('PUSH'):
            depth += 4
            pushed_since_call += 4
        elif upper.startswith('POP'):
            depth -= 4
        elif upper.startswith('SUB ESP,'):
            depth += imm(text_.split(',')[1].strip())
            pushed_since_call += imm(text_.split(',')[1].strip())
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
                pushed_since_call = 0
            elif 'CALL' in upper:
                # An indirect call's callee cannot be resolved, so its cleanup is
                # unknowable and everything after it is marked frame=?. That is
                # safe but unhelpful, and two packets have had to correct the
                # walk by hand here - so report the pushes standing since the
                # last call, which is what the cleanup will be if the callee is
                # __stdcall or __thiscall. The analyst supplies the answer with
                # --indirect-pops; the walker never guesses it.
                if indirect_pops is None:
                    unknown = True
                    note += ('  [INDIRECT CALL - %d bytes pushed since the last call; '
                             'cleanup unknown, frame unknown after this. Re-run with '
                             '--indirect-pops N to assume a cleanup]' % pushed_since_call)
                else:
                    depth -= indirect_pops
                    note += ('  [INDIRECT CALL - assuming it pops %d (--indirect-pops); '
                             '%d bytes had been pushed since the last call]'
                             % (indirect_pops, pushed_since_call))
                pushed_since_call = 0

        if upper.startswith('RET'):
            crossed_return = True
            last_ret = address

        if args.filter and args.filter.upper() not in upper:
            continue
        print('%s: %-42s depth=%-5d%s' % (address, text_, before, note))
    return 0


if __name__ == '__main__':
    sys.exit(main())
