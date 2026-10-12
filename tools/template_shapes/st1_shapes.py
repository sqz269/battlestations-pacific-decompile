"""cc13-stl1: shape signatures for untagged FUN_ in the cc13 segments.

Disassembles each function from its start to the next function start (any function, incl.
EH funclets), strips trailing INT3/NOP padding, normalises call/jump targets and constants,
and groups identical bodies. Also signs every already tagged / named function in the image
so a group can be matched to a known reference shape.
Output: local/st1_shapes.json
"""
import json
import re
import sqlite3
import sys
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parents[2]
SEGS = [20, 17, 23, 26, 27, 29, 6, 15, 12, 19, 2, 9, 78, 81, 84, 85, 86, 88, 89, 90, 91, 92, 93, 94, 96,
        43, 45, 46, 61, 64, 65, 76, 77, 79, 80, 82, 83, 25, 22, 16, 28, 4, 10, 5]
EXE = json.loads((ROOT.parent / 'battlestations-pacific-decompile' / 'exports/bsp/disk_binary.json').read_text())['path']

db = sqlite3.connect(ROOT / 'local/bsp_index.sqlite')
rows = db.execute('SELECT address, name, thunk, segment, tag_category, tag_name, ledger_name, recon_kind FROM functions').fetchall()
starts = sorted(r[0] for r in rows)
info = {r[0]: r for r in rows}
import bisect

pe = pefile.PE(EXE, fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
text = next(s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text')
tlo = base + text.VirtualAddress
tdata = text.get_data()
thi = tlo + len(tdata)
md = Cs(CS_ARCH_X86, CS_MODE_32)
md.detail = False

NUM = re.compile(r'0x[0-9a-f]+|\b\d+\b')


def label(t):
    r = info.get(t)
    if r is None:
        return 'X'
    name = r[1]
    if not name.startswith('FUN_'):
        # library / CRT names are informative; tagged inventory names collapse to their prefix
        m = re.match(r'(STL_inst|CG_[a-z_]+?|TRIV_body|STL_xlen_throw|STL_probable)_[0-9a-f]{8}$', name)
        return m.group(1) if m else name
    if r[4]:
        return 'T:' + r[4]
    return 'F'


def body(start):
    i = bisect.bisect_right(starts, start)
    end = starts[i] if i < len(starts) else start + 4096
    end = min(end, start + 4096, thi)
    code = tdata[start - tlo:end - tlo]
    out = []
    for ins in md.disasm(code, start):
        if ins.mnemonic == 'int3':
            break
        out.append(ins)
    # strip trailing padding
    while out and out[-1].mnemonic in ('int3', 'nop'):
        out.pop()
    return out, end - start


def sig(insns, start, end):
    fine, coarse = [], []
    for ins in insns:
        m, op = ins.mnemonic, ins.op_str
        if (m == 'call' or m.startswith('j')) and re.fullmatch(r'0x[0-9a-f]+', op):
            t = int(op, 16)
            if start <= t < end:
                op = f'L{t - start:x}'
            else:
                op = label(t)
            fine.append(f'{m} {op}')
            coarse.append(f'{m} {op if not op.startswith("L") else "L"}')
            continue

        def mask_abs(mo):
            v = int(mo.group(0), 0)
            return 'A' if v >= 0x400000 else mo.group(0)
        f = NUM.sub(mask_abs, op)
        c = NUM.sub(lambda mo: 'A' if int(mo.group(0), 0) >= 0x400000 else 'N', op)
        fine.append(f'{m} {f}')
        coarse.append(f'{m} {c}')
    return '\n'.join(fine), '\n'.join(coarse)


def main():
    targets = [r for r in rows if r[3] in SEGS and r[1].startswith('FUN_') and not r[2]]
    known = [r for r in rows if not r[1].startswith('FUN_') and not r[1].startswith(('Unwind@', 'Catch'))]
    out = {'targets': {}, 'known_fine': {}, 'known_coarse': {}}
    for r in targets:
        a = r[0]
        if not (tlo <= a < thi):
            continue
        ins, span = body(a)
        f, c = sig(ins, a, a + span)
        out['targets'][f'{a:08x}'] = {'seg': r[3], 'tag': r[4], 'ledger': r[6], 'recon': r[7],
                                       'n': len(ins), 'span': span, 'size': (ins[-1].address + ins[-1].size - a) if ins else 0, 'fine': f, 'coarse': c}
    for r in known:
        a = r[0]
        if not (tlo <= a < thi):
            continue
        ins, span = body(a)
        if len(ins) > 200:
            continue
        f, c = sig(ins, a, a + span)
        out['known_fine'].setdefault(f, []).append(f'{a:08x}:{r[1]}')
        out['known_coarse'].setdefault(c, []).append(f'{a:08x}:{r[1]}')
    (ROOT / 'local/st1_shapes.json').write_text(json.dumps(out))
    print(len(out['targets']), 'targets', len(known), 'known')


if __name__ == '__main__':
    main()
