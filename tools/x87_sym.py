"""Single-path symbolic executor for x87/SSE code, read-only over the PE on disk.

Usage:
  python tools/x87_sym.py <start> [--take A,B..] [--stop A,..] [--steps N] [--pop A=n,..]
         [--fret A,..] [--sqrt A,..] [--out CALLEE=w[:len],..] [--init REG=sym,..]
         [--esp N] [--binary PATH] [--trace] [--loads]

Follows ONE path from <start>: a conditional jump falls through unless the jump's own
address or its target is in --take. Prints every memory store (values named v<addr>),
every call with its first six stack arguments, ECX/EDX and the x87 stack, every branch
with the last compare, and the x87 stack at RET.

x87 register arithmetic is decoded from the opcode bytes per the Intel SDM, not from
mnemonics, because Ghidra's short forms (`FMUL ST1` = DC C9 writes ST1, not ST0) are
ambiguous. Constants are read from the image at the load's own width. Entry ESP = 0
(return address at [0], first stack argument at [4]); ECX starts as `this`.

  --take   jump addresses (or targets) to take; all others fall through
  --stop   addresses that end the walk
  --pop    callee stack cleanup in bytes (hex) per CALL site, ADDR=n; otherwise a
           callee's own RET imm16 is read from the image
  --fret   call sites or callees that return a value in ST0 (pushes ret_<site>)
  --sqrt   callees that replace ST0 with sqrt(ST0) (default 00BF7030, the CRT _CIsqrt)
  --out    callees that fill an out-buffer: w = 'c' (ECX) or the stack-argument index;
           len (hex, default 40h) bytes there become r<site>_<off>
  --init   initial register values, e.g. ESI=obj or EAX=0x10
  --esp    initial ESP (default 0)
  --binary the PE to read (default: config/target.json "binary")
  --trace  print every instruction; --loads also prints memory loads

Origin: cc13-water2's local/wa2_sym.py (particle and water x87 verification), made
standalone. Different from tools/x87_sym_walker.py, which forks depth-first over a
tools/x87trace.py listing and tracks frame slots; this one runs directly from the image,
follows a single forced path, and also models general registers and scalar SSE.

Regression (cc13-shadow1): `x + 0.0` terms are printed, never folded. Before the fix,
`python tools/x87_sym.py 00A8C636 --stop 00A8C67D --fpu 0.0,S` lost (the 00A8C679 line now ends `+ 0.0`) the `+ 0.0` that turns -0 into +0.
"""
import json
import sys
import struct
import re
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parent.parent


def _binary_path(argv):
    if '--binary' in argv:
        return argv[argv.index('--binary') + 1]
    return json.loads((ROOT / 'config' / 'target.json').read_text())['binary']


_pe = pefile.PE(_binary_path(sys.argv), fast_load=True)
BASE = _pe.OPTIONAL_HEADER.ImageBase
_img = _pe.get_memory_mapped_image()


def read(va, n):
    o = va - BASE
    return _img[o:o + n]


import capstone
from capstone import x86

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True

SECS = [(s.Name.rstrip(b'\0').decode(), BASE + s.VirtualAddress, BASE + s.VirtualAddress + s.Misc_VirtualSize) for s in _pe.sections]


def section(va):
    for n, lo, hi in SECS:
        if lo <= va < hi:
            return n
    return '?'


args = sys.argv[1:]
start = int(args[0], 16)
opt = {}
i = 1
while i < len(args):
    a = args[i]
    if a in ('--trace', '--loads'):
        opt[a] = True; i += 1
    else:
        opt[a] = args[i + 1]; i += 2
lst = lambda k: [x for x in opt.get(k, '').split(',') if x]
take = {int(x, 16) for x in lst('--take')}
stops = {int(x, 16) for x in lst('--stop')}
pops = {int(k, 16): int(v, 16) for k, v in (x.split('=') for x in lst('--pop'))}
fret = {int(x, 16) for x in lst('--fret')}
outs = {int(k, 16): v for k, v in (x.split('=') for x in lst('--out'))}
sqrts = {int(x, 16) for x in lst('--sqrt')} or {0x00BF7030}
steps = int(opt.get('--steps', '3000'))
TRACE = '--trace' in opt


def fmt(v):
    if isinstance(v, float):
        if v == int(v) and abs(v) < 1e7:
            return repr(v)
        return '%.9g' % v
    return v


def atom(s):
    s = fmt(s)
    return s if re.fullmatch(r'[\w\.\[\]\-#@+]*', s) and not s.startswith('-') else '(' + s + ')'


def binop(op, a, b):
    if isinstance(a, float) and isinstance(b, float):
        return {'+': a + b, '-': a - b, '*': a * b, '/': a / b if b else float('nan')}[op]
    if op == '*' and (a == 1.0 or b == 1.0):
        return b if a == 1.0 else a
    # No `x + 0.0` folding: it maps -0 to +0, and native code adds stack zeros on purpose
    # (00A8C636 FADD ST(7)). Only `-0.0 - x` is folded, because that is an exact negation.
    if op == '-' and isinstance(a, float) and a == 0.0 and str(a) == '-0.0':
        return 'neg(' + fmt(b) + ')'
    return '%s %s %s' % (atom(a), op, atom(b))


regs = {r: r.lower() + '0' for r in ('EAX', 'EBX', 'ECX', 'EDX', 'ESI', 'EDI', 'EBP')}
regs['ECX'] = 'this'
for x in lst('--init'):
    k, v = x.split('=')
    regs[k.upper()] = int(v, 16) if re.fullmatch(r'0x[0-9a-fA-F]+', v) else v
esp = int(opt.get('--esp', '0'), 0)
mem = {}
xmm = {'XMM%d' % k: ['xmm%d_0' % k, 'xmm%d_1' % k, 'xmm%d_2' % k, 'xmm%d_3' % k] for k in range(8)}
fpu = [float(x) if re.fullmatch(r'-?[\d.]+', x) else x for x in lst('--fpu')]  # top is fpu[-1]
lastcmp = ['?']
cur = [0]


ecount = [0]


def nm(v, ip):
    if isinstance(v, str) and len(v) > 90:
        ecount[0] += 1
        name = 'e%x_%d' % (ip, ecount[0])
        print('%08X   %s = %s' % (ip, name, v))
        return name
    return v


def st(k):
    return fpu[-1 - k]


def setst(k, v):
    fpu[-1 - k] = nm(v, cur[0])


def regval(name):
    name = name.upper()
    if name == 'ESP':
        return ('S', esp)
    return regs.get(name, name.lower())


def addr_key(ins, op):
    m = op.mem
    base = ins.reg_name(m.base).upper() if m.base else None
    idx = ins.reg_name(m.index).upper() if m.index else None
    disp = m.disp
    if base is None and idx is None:
        return ('G', disp & 0xffffffff)
    bv = regval(base) if base else None
    if idx:
        iv = regval(idx)
        if isinstance(iv, int) and isinstance(bv, tuple):
            return ('S', bv[1] + iv * m.scale + disp)
        if isinstance(iv, int) and isinstance(bv, int):
            return ('G', (bv + iv * m.scale + disp) & 0xffffffff)
        return ('X', '%s[%s*%d%+#x]' % (fmt(bv) if not isinstance(bv, tuple) else 'S%+#x' % bv[1], fmt(iv), m.scale, disp))
    if isinstance(bv, tuple):
        return ('S', bv[1] + disp)
    if isinstance(bv, int):
        return ('G', (bv + disp) & 0xffffffff)
    return ('B', bv, disp)


def keyname(k):
    if k[0] == 'S':
        return 'S%+#x' % k[1]
    if k[0] == 'G':
        return '[%08X]' % k[1]
    if k[0] == 'X':
        return k[1]
    return '%s[%#x]' % (atom(k[1]), k[2])


def load(ins, op, kind):
    k = addr_key(ins, op)
    if k in mem:
        return mem[k][1]
    if k[0] == 'G':
        sec = section(k[1])
        if sec == '.rdata':
            if kind == 'f':
                return struct.unpack('<f', read(k[1], 4))[0]
            if kind == 'd':
                return struct.unpack('<d', read(k[1], 8))[0]
            if kind == 'i':
                return struct.unpack('<i', read(k[1], 4))[0]
            return struct.unpack('<I', read(k[1], 4))[0]
    v = keyname(k)
    if k[0] == 'S' and k[1] > 0:
        v = 'arg%+#x' % k[1]
    if '--loads' in opt:
        print('   load %s' % v)
    return v


named = {}


def store(ins, op, val, kind, note=''):
    k = addr_key(ins, op)
    simple = isinstance(val, (float, int, tuple)) or re.fullmatch(r'[\w\.\[\]#@+\-]*', str(val))
    if not simple:
        nm = 'v%x' % ins.address
        print('%08X %s := %s%s   (%s)' % (ins.address, nm, fmt(val), note, keyname(k)))
        val = nm
    elif k[0] != 'S':
        print('%08X STORE %s = %s%s' % (ins.address, keyname(k), fmt(val), note))
    mem[k] = (kind, val)


def callee_pop(t):
    if t in pops:
        return pops[t]
    off = 0
    for ins in md.disasm(read(t, 0x4000), t):
        if ins.mnemonic == 'ret':
            return int(ins.op_str, 16) if ins.op_str else 0
        if ins.mnemonic == 'int3':
            break
    return 0


ip = start
n = 0
while n < steps:
    n += 1
    if ip in stops:
        print('STOP at %08X' % ip)
        break
    ins = next(md.disasm(read(ip, 16), ip))
    cur[0] = ip
    nxt = ip + ins.size
    mn = ins.mnemonic
    ops = ins.operands
    b = ins.bytes
    if TRACE:
        print('   %08X %-28s fpu=%s' % (ip, mn + ' ' + ins.op_str, [fmt(x) for x in fpu[::-1]]))
    try:
        if mn == 'push':
            o = ops[0]
            v = o.imm if o.type == x86.X86_OP_IMM else (regval(ins.reg_name(o.reg)) if o.type == x86.X86_OP_REG else load(ins, o, 'u'))
            esp -= 4
            mem[('S', esp)] = ('u', v)
        elif mn == 'pop':
            v = mem.get(('S', esp), ('u', '?'))[1]
            regs[ins.reg_name(ops[0].reg).upper()] = v
            esp += 4
        elif mn in ('mov', 'lea', 'movzx', 'movsx'):
            d, s = ops
            if mn == 'lea':
                k = addr_key(ins, s)
                v = k if k[0] == 'S' else (k[1] if k[0] == 'G' else '&' + keyname(k))
                if k[0] == 'S':
                    v = ('S', k[1])
            elif s.type == x86.X86_OP_IMM:
                v = s.imm
            elif s.type == x86.X86_OP_REG:
                v = regval(ins.reg_name(s.reg))
            else:
                v = load(ins, s, 'u')
            if d.type == x86.X86_OP_REG:
                r = ins.reg_name(d.reg).upper()
                if r == 'ESP':
                    esp = v[1] if isinstance(v, tuple) else esp
                else:
                    regs[r] = v
            else:
                store(ins, d, v, 'u')
        elif mn in ('add', 'sub', 'inc', 'dec') and ops[0].type == x86.X86_OP_MEM:
            k = addr_key(ins, ops[0])
            a = mem.get(k, ('u', keyname(k)))[1]
            bv = ops[1].imm if len(ops) > 1 else 1
            v = (a + bv if mn in ('add', 'inc') else a - bv) if isinstance(a, int) else '(%s %s %s)' % (fmt(a), '+' if mn in ('add', 'inc') else '-', bv)
            mem[k] = ('u', v)
            lastcmp[0] = '%s %s -> %s' % (mn, keyname(k), fmt(v))
        elif mn in ('add', 'sub') and ops[0].type == x86.X86_OP_REG and ins.reg_name(ops[0].reg) == 'esp':
            esp += ops[1].imm if mn == 'add' else -ops[1].imm
        elif mn in ('add', 'sub', 'imul', 'shl', 'sar', 'shr', 'and', 'xor', 'or', 'inc', 'dec') and ops[0].type == x86.X86_OP_REG:
            r = ins.reg_name(ops[0].reg).upper()
            a = regs.get(r)
            if mn == 'xor' and len(ops) == 2 and ops[1].type == x86.X86_OP_REG and ops[1].reg == ops[0].reg:
                regs[r] = 0
            else:
                if len(ops) == 1:
                    bv = 1
                elif ops[-1].type == x86.X86_OP_IMM:
                    bv = ops[-1].imm
                elif ops[-1].type == x86.X86_OP_REG:
                    bv = regval(ins.reg_name(ops[-1].reg))
                else:
                    bv = load(ins, ops[-1], 'u')
                if mn == 'imul' and len(ops) == 3:
                    a = regval(ins.reg_name(ops[1].reg)) if ops[1].type == x86.X86_OP_REG else load(ins, ops[1], 'u')
                opc = {'add': '+', 'sub': '-', 'imul': '*', 'shl': '<<', 'sar': '>>', 'shr': '>>>', 'and': '&', 'xor': '^', 'or': '|', 'inc': '+', 'dec': '-'}[mn]
                if isinstance(a, int) and isinstance(bv, int):
                    regs[r] = int(eval('%d %s %d' % (a, opc.replace('>>>', '>>'), bv))) & 0xffffffff
                elif isinstance(a, tuple) and isinstance(bv, int) and mn in ('add', 'sub'):
                    regs[r] = ('S', a[1] + (bv if mn == 'add' else -bv))
                else:
                    regs[r] = '(%s %s %s)' % (fmt(a) if not isinstance(a, tuple) else 'S%+#x' % a[1], opc, fmt(bv))
            lastcmp[0] = '%s %s' % (mn, r)
        elif mn in ('cmp', 'test'):
            vs = []
            for o in ops:
                vs.append(o.imm if o.type == x86.X86_OP_IMM else (regval(ins.reg_name(o.reg)) if o.type == x86.X86_OP_REG else load(ins, o, 'u')))
            lastcmp[0] = '%s %s, %s' % (mn, fmt(vs[0]) if not isinstance(vs[0], tuple) else vs[0], fmt(vs[1]) if not isinstance(vs[1], tuple) else vs[1])
        elif mn == 'call':
            t = ops[0].imm if ops[0].type == x86.X86_OP_IMM else None
            tname = '%08X' % t if t else (fmt(regval(ins.reg_name(ops[0].reg))) if ops[0].type == x86.X86_OP_REG else keyname(addr_key(ins, ops[0])))
            a = [fmt(mem.get(('S', esp + 4 * j), ('u', '?'))[1]) if not isinstance(mem.get(('S', esp + 4 * j), ('u', '?'))[1], tuple) else 'S%+#x' % mem[('S', esp + 4 * j)][1][1] for j in range(6)]
            ecx = regs['ECX']
            edx = regs['EDX']
            print('%08X CALL %s edx=%s ecx=%s args=[%s] fpu=%s' % (ip, tname, fmt(edx) if not isinstance(edx, tuple) else 'S%+#x' % edx[1], fmt(ecx) if not isinstance(ecx, tuple) else 'S%+#x' % ecx[1], ', '.join(map(str, a)), [fmt(x) for x in fpu[::-1]]))
            esp += pops.get(ip, callee_pop(t) if t else 0)
            regs['EAX'] = 'ret_%x' % ip
            if t in outs:
                w = outs[t]
                ln = 0x40
                if ':' in w:
                    w, ln = w.split(':'); ln = int(ln, 16)
                p = ecx if w == 'c' else mem.get(('S', esp - pops.get(ip, callee_pop(t)) + 4 * int(w)), ('u', None))[1]
                if isinstance(p, tuple):
                    for k in range(0, ln, 4):
                        mem[('S', p[1] + k)] = ('f', 'r%x_%x' % (ip, k))
                    regs['EAX'] = p
            regs['ECX'] = 'ecx?'
            regs['EDX'] = 'edx?'
            if t in sqrts:
                fpu[-1] = 'sqrt(%s)' % fmt(fpu[-1])
            elif ip in fret or (t in fret if t else False):
                fpu.append('ret_%x' % ip)
        elif mn == 'ret':
            print('%08X RET esp=%#x fpu=%s' % (ip, esp, [fmt(x) for x in fpu[::-1]]))
            break
        elif mn == 'jmp':
            if ops[0].type == x86.X86_OP_IMM:
                nxt = ops[0].imm
            else:
                print('%08X indirect jmp' % ip); break
        elif mn.startswith('j'):
            t = ops[0].imm
            tk = t in take or ip in take
            print('%08X %s %08X  [%s]  -> %s' % (ip, mn, t, lastcmp[0], 'TAKEN' if tk else 'fall'))
            if tk:
                nxt = t
        # ---------------- SSE
        elif mn in ('movss', 'movaps', 'movups', 'movd'):
            d, s = ops
            if s.type == x86.X86_OP_REG:
                v = list(xmm[ins.reg_name(s.reg).upper()])
            else:
                v = [load(ins, s, 'f'), 0.0, 0.0, 0.0] if mn == 'movss' else [load(ins, s, 'f')] + ['?'] * 3
            if d.type == x86.X86_OP_REG:
                r = ins.reg_name(d.reg).upper()
                if mn == 'movss' and s.type == x86.X86_OP_REG:
                    xmm[r][0] = v[0]
                else:
                    xmm[r] = v
            else:
                store(ins, d, v[0], 'f')
        elif mn == 'xorps':
            r = ins.reg_name(ops[0].reg).upper()
            if ops[1].type == x86.X86_OP_REG and ops[1].reg == ops[0].reg:
                xmm[r] = [0.0] * 4
            else:
                m = load(ins, ops[1], 'u')
                xmm[r] = ['xor(%s,%s)' % (fmt(xmm[r][0]), fmt(m))] + xmm[r][1:]
        elif mn in ('addss', 'subss', 'mulss', 'divss', 'sqrtss', 'maxss', 'minss'):
            r = ins.reg_name(ops[0].reg).upper()
            s = ops[1]
            v = xmm[ins.reg_name(s.reg).upper()][0] if s.type == x86.X86_OP_REG else load(ins, s, 'f')
            a = xmm[r][0]
            if mn == 'sqrtss':
                xmm[r][0] = 'sqrt(%s)' % fmt(v)
            elif mn in ('maxss', 'minss'):
                xmm[r][0] = '%s(%s, %s)' % (mn[:3], fmt(a), fmt(v))
            else:
                xmm[r][0] = nm(binop({'addss': '+', 'subss': '-', 'mulss': '*', 'divss': '/'}[mn], a, v), ip)
        elif mn in ('comiss', 'ucomiss'):
            s = ops[1]
            v = xmm[ins.reg_name(s.reg).upper()][0] if s.type == x86.X86_OP_REG else load(ins, s, 'f')
            lastcmp[0] = 'comiss %s ? %s' % (fmt(xmm[ins.reg_name(ops[0].reg).upper()][0]), fmt(v))
        elif mn in ('cvttss2si', 'cvtss2si'):
            s = ops[1]
            v = xmm[ins.reg_name(s.reg).upper()][0] if s.type == x86.X86_OP_REG else load(ins, s, 'f')
            regs[ins.reg_name(ops[0].reg).upper()] = '%s(%s)' % (mn, fmt(v))
        elif mn == 'cvtsi2ss':
            s = ops[1]
            v = regval(ins.reg_name(s.reg)) if s.type == x86.X86_OP_REG else load(ins, s, 'i')
            xmm[ins.reg_name(ops[0].reg).upper()][0] = 'float(%s)' % fmt(v)
        # ---------------- x87
        elif mn in ('fld', 'fild'):
            o = ops[0]
            if o.type == x86.X86_OP_REG:
                fpu.append(st(int(re.sub(r'\D', '', ins.reg_name(o.reg)) or 0)))
            else:
                kind = 'd' if o.size == 8 else 'f'
                if mn == 'fild':
                    v = load(ins, o, 'i')
                    v = float(v) if isinstance(v, int) else 'float(%s)' % fmt(v)
                else:
                    v = load(ins, o, kind)
                fpu.append(v)
        elif mn == 'fld1':
            fpu.append(1.0)
        elif mn == 'fldz':
            fpu.append(0.0)
        elif mn in ('fst', 'fstp'):
            o = ops[0]
            if o.type == x86.X86_OP_REG:
                k = int(re.sub(r'\D', '', ins.reg_name(o.reg)) or 0)
                setst(k, st(0))
            else:
                store(ins, o, st(0), 'f', '' if o.size == 4 else ' [%d-byte]' % o.size)
            if mn == 'fstp':
                fpu.pop()
        elif mn == 'fxch':
            k = int(re.sub(r'\D', '', ins.reg_name(ops[-1].reg)) or 1) if ops else 1
            a, c = st(0), st(k)
            setst(0, c); setst(k, a)
        elif mn == 'fchs':
            setst(0, 'neg(%s)' % fmt(st(0)) if not isinstance(st(0), float) else -st(0))
        elif mn == 'fabs':
            setst(0, 'abs(%s)' % fmt(st(0)))
        elif mn in ('fsqrt', 'fsin', 'fcos'):
            setst(0, '%s(%s)' % (mn[1:], fmt(st(0))))
        elif mn in ('fcompi', 'fucompi', 'fcomip', 'fucomip', 'fcomi', 'fucomi', 'fcomp', 'fcom', 'fcompp', 'fucomp'):
            if ops and ops[-1].type == x86.X86_OP_REG:
                k = int(re.sub(r'\D', '', ins.reg_name(ops[-1].reg)) or 1)
                other = st(k)
            elif ops:
                other = load(ins, ops[0], 'd' if ops[0].size == 8 else 'f')
            else:
                other = st(1)
            lastcmp[0] = 'fcmp ST0=%s ? %s' % (fmt(st(0)), fmt(other))
            if mn in ('fcompi', 'fucompi', 'fcomip', 'fucomip', 'fcomp', 'fucomp', 'fcompp', 'fucompp'):
                fpu.pop()
            if mn == 'fcompp':
                fpu.pop()
        elif mn in ('fnstsw',):
            pass
        elif mn[:4] in ('fadd', 'fsub', 'fmul', 'fdiv') or mn[:5] in ('fsubr', 'fdivr'):
            if len(b) == 2 and b[1] >= 0xC0 and b[0] in (0xD8, 0xDC, 0xDE):
                op8, i8 = b[0], b[1] & 7
                kind = (b[1] >> 3) & 7  # 0 add 1 mul 4 E0 5 E8 6 F0 7 F8
                if op8 == 0xD8:
                    a, c = st(0), st(i8)
                    r = {0: ('+', a, c), 1: ('*', a, c), 4: ('-', a, c), 5: ('-', c, a), 6: ('/', a, c), 7: ('/', c, a)}[kind]
                    setst(0, binop(*r))
                else:
                    a, c = st(0), st(i8)  # target is ST(i)
                    r = {0: ('+', c, a), 1: ('*', c, a), 4: ('-', a, c), 5: ('-', c, a), 6: ('/', a, c), 7: ('/', c, a)}[kind]
                    setst(i8, binop(*r))
                    if op8 == 0xDE:
                        fpu.pop()
            else:
                o = ops[0]
                v = load(ins, o, 'd' if o.size == 8 else 'f')
                if mn.startswith('fi'):
                    v = 'float(%s)' % fmt(v)
                base = mn
                a = st(0)
                r = {'fadd': ('+', a, v), 'fmul': ('*', a, v), 'fsub': ('-', a, v), 'fsubr': ('-', v, a), 'fdiv': ('/', a, v), 'fdivr': ('/', v, a)}[base]
                setst(0, binop(*r))
        else:
            print('%08X ?? %s %s' % (ip, mn, ins.op_str))
    except Exception as e:
        print('%08X !! %s %s : %r fpu=%s' % (ip, mn, ins.op_str, e, fpu))
        break
    ip = nxt
else:
    print('step cap at %08X' % ip)
