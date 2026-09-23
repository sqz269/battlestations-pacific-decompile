"""Symbolic SSA walker over a tools/x87trace.py listing.

Consumes the text output of `python tools/x87trace.py trace <start> <end> <--call ...>`
(for 009BFEE0 the committed argv is tools/callee_effects_009bfee0.json's
`x87trace_argv`), saved to a file. Each trace line gives the address, x87trace's
depth, the frame base, the bytes, the Capstone instruction and, for ESP-relative
operands, the canonical `base [ESP+NNh]` slot. Capstone's operand forms are used,
so `DC C9` (`fmul st(1), st(0)`) writes ST1, not ST0 as Ghidra's text suggests.

Usage:
  python tools/x87_sym_walker.py <start> <stop,stop,...> [--trace FILE]
         [--path ADDR:t,ADDR:f,...] [--range LO HI] [--init s0,s1,...]
         [--max-visits N] [--limit K]

It walks paths depth-first from <start> until an address in <stops>, carrying the
x87 stack and the frame slots as named SSA values (`slot := vN = expr`), and prints
every slot store and every branch with its condition, then the x87 stack at the stop.
  --path     forces conditional jumps (t = taken, f = fall through); unforced ones fork
  --range    ends a path when it leaves [LO, HI], which keeps the fork count bounded
  --init     names the incoming x87 stack, top first, for walks starting mid-body
Constants are read from the image at the load's own width. Calls use a small table of
known x87 effects (asin_clamped, sqrt, atan2 with x87 arguments, ...); anything else is
logged. Unmodelled x87 opcodes are counted and printed on the first line.

CAVEAT, 009BFEE0: x87trace reports depth 3 at 009C0251 because of its join conflict at
009C00C0, but every real path from 009C0026 reaches Phase A with an EMPTY x87 stack.
This walker warns (`!! depth mismatch`) where its per-path depth differs from x87trace's
and does not resync; its own depth is the per-path truth. docs/PLANE_FOLLOW_PHASE_A.md.
Packet cc9_follow_phase_a; committed in cc9_pitch_callers.
"""
import json, re, struct, sys

args = sys.argv[1:]
start = int(args[0], 16)
stops = {int(x, 16) for x in args[1].split(",")}
maxv = 2
forced = {}
limit = 40
SYNC = False
TRACE = "local/trall.txt"
RANGE = (0, 0xFFFFFFFF)
INIT = []
i = 2
while i < len(args):
    if args[i] == "--max-visits":
        maxv = int(args[i + 1]); i += 2
    elif args[i] == "--path":
        for kv in args[i + 1].split(","):
            a, t = kv.split(":"); forced[int(a, 16)] = (t == "t")
        i += 2
    elif args[i] == "--trace":
        TRACE = args[i + 1]; i += 2
    elif args[i] == "--range":
        RANGE = (int(args[i + 1], 16), int(args[i + 2], 16)); i += 3
    elif args[i] == "--init":
        INIT = args[i + 1].split(","); i += 2
    elif args[i] == "--limit":
        limit = int(args[i + 1]); i += 2
    else:
        raise SystemExit("bad arg " + args[i])

cfg = json.load(open("config/target.json")); IMG = open(cfg["binary"], "rb").read()
pe = struct.unpack_from("<I", IMG, 0x3C)[0]; ns = struct.unpack_from("<H", IMG, pe + 6)[0]
opt = struct.unpack_from("<H", IMG, pe + 20)[0]; BASE = struct.unpack_from("<I", IMG, pe + 52)[0]
SECS = [struct.unpack_from("<IIII", IMG, pe + 24 + opt + k * 40 + 8) for k in range(ns)]
def rd(a, n):
    for vs, va, rs, rp in SECS:
        if BASE + va <= a < BASE + va + max(vs, rs):
            o = a - BASE - va
            return IMG[rp + o:rp + o + n] if o + n <= rs else None
def const(addr, width):
    if width == "qword":
        b = rd(addr, 8); return "%.9g" % struct.unpack("<d", b)[0] if b else "BSS%X" % addr
    b = rd(addr, 4); return "%.9g" % struct.unpack("<f", b)[0] if b else "BSS%X" % addr

INS = {}
DEPTH = {}
ORDER = []
for line in open(TRACE):
    m = re.match(r"\s+([0-9a-f]{8})\s+d(-?\d+)\s+fb=0x([0-9a-f]+)\s+[0-9a-f]+\s+(.*?)\s*(?:;\s*base \[ESP\+(-?[0-9A-F]+)h\])?\s*$", line)
    if not m:
        continue
    a = int(m.group(1), 16)
    INS[a] = (m.group(4).strip(), m.group(5))
    DEPTH[a] = int(m.group(2))
    ORDER.append(a)
ORDER.sort()
NEXT = {ORDER[k]: ORDER[k + 1] for k in range(len(ORDER) - 1)}

def slotname(lbl):
    v = int(lbl, 16)
    return "B%+03Xh" % v if v >= 0 else "B-%02Xh" % (-v)

def memop(op, lbl):
    op = op.strip()
    m = re.match(r"(dword|qword) ptr \[(.*)\]", op)
    if not m:
        return None, None
    w, inner = m.group(1), m.group(2)
    if lbl is not None and inner.startswith("esp"):
        return ("slot", slotname(lbl)), w
    mc = re.match(r"0x([0-9a-f]+)$", inner)
    if mc:
        return ("const", const(int(mc.group(1), 16), w)), w
    return ("mem", "[" + inner + "]"), w

UNK = {}
class St:
    def __init__(s):
        s.fpu = []; s.slots = {}; s.xmm = {}; s.cmp = None; s.log = []; s.n = 0; s.regs = {}
    def copy(s):
        t = St(); t.fpu = list(s.fpu); t.slots = dict(s.slots); t.xmm = dict(s.xmm)
        t.cmp = s.cmp; t.log = list(s.log); t.n = s.n; t.regs = dict(s.regs); return t
    def new(s, expr):
        s.n += 1; name = "v%d" % s.n; s.log.append("      %s = %s" % (name, expr)); return name
    def st(s, k):
        return s.fpu[-1 - k] if k < len(s.fpu) else "ST?%d" % k
    def setst(s, k, v):
        while k >= len(s.fpu): s.fpu.insert(0, "ST?")
        s.fpu[-1 - k] = v

def val(s, kind_name):
    kind, name = kind_name
    if kind == "slot":
        return s.slots.get(name, name)
    return name

def stidx(t):
    m = re.match(r"st\((\d)\)", t.strip()); return int(m.group(1)) if m else None

ARITH = {"fadd": "+", "fsub": "-", "fmul": "*", "fdiv": "/"}
def step(s, a):
    txt, lbl = INS[a]
    parts = txt.split(None, 1); op = parts[0]; rest = parts[1] if len(parts) > 1 else ""
    ops = [x.strip() for x in re.split(r",(?![^\[]*\])", rest)] if rest else []
    if op in ("fld",):
        k = stidx(rest)
        if k is not None: s.fpu.append(s.st(k))
        else:
            mo, w = memop(rest, lbl); s.fpu.append(val(s, mo))
    elif op == "fldz": s.fpu.append("0")
    elif op == "fld1": s.fpu.append("1")
    elif op in ("fstp", "fst"):
        k = stidx(rest)
        top = s.st(0)
        if k is not None:
            s.setst(k, top)
        else:
            mo, w = memop(rest, lbl)
            if mo and mo[0] == "slot":
                nm = s.new(top); s.slots[mo[1]] = nm
                s.log[-1] = "  %s: %s := %s = %s" % ("%08X" % a, mo[1], nm, top)
                if op == "fstp": (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a))
                s.fpu_fix = True
                return
            s.log.append("  %08X: store %s <- %s" % (a, rest, top))
        if op == "fstp": (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a))
    elif op == "fxch":
        k = stidx(rest) if rest else 1
        t0, tk = s.st(0), s.st(k); s.setst(0, tk); s.setst(k, t0)
    elif op in ("fchs", "fabs", "fsin", "fcos", "fsqrt"):
        f = {"fchs": "-(%s)", "fabs": "|%s|", "fsin": "sin(%s)", "fcos": "cos(%s)", "fsqrt": "sqrt(%s)"}[op]
        s.setst(0, f % s.st(0))
    elif re.match(r"f(add|sub|subr|mul|div|divr)p?$", op):
        base = op.rstrip("p") if op.endswith("p") and op not in ("fsubp",) else op
        popping = op.endswith("p")
        core = op[:-1] if popping else op
        rev = core.endswith("r")
        o = ARITH[core[:-1] if rev else core]
        def comb(x, y):  # x op y
            return "(%s %s %s)" % (x, o, y)
        if popping:
            k = stidx(ops[0]) if ops else 1
            # ST(k) = ST(k) op ST0 (or ST0 op ST(k) for the r form), pop
            r = comb(s.st(0), s.st(k)) if rev else comb(s.st(k), s.st(0))
            s.setst(k, r); (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a))
        elif len(ops) == 2 and stidx(ops[0]) is not None:
            d, src = stidx(ops[0]), stidx(ops[1])
            r = comb(s.st(src), s.st(d)) if rev else comb(s.st(d), s.st(src))
            s.setst(d, r)
        elif len(ops) == 1 and stidx(ops[0]) is not None:
            k = stidx(ops[0])
            r = comb(s.st(k), s.st(0)) if rev else comb(s.st(0), s.st(k))
            s.setst(0, r)
        else:
            mo, w = memop(rest, lbl); m = val(s, mo)
            r = comb(m, s.st(0)) if rev else comb(s.st(0), m)
            s.setst(0, r)
    elif op in ("fcomi", "fcomip", "fcompi", "fucomip", "fcomp", "fcom"):
        k = stidx(rest) if rest else 1
        s.cmp = (s.st(0), s.st(k))
        if op in ("fcomip", "fcompi", "fucomip", "fcomp"): (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a))
    elif op.startswith("f") and op not in ("fnstsw", "fwait"):
        UNK[op] = UNK.get(op, 0) + 1
    elif op in ("movss",):
        dst, src = ops
        if dst.startswith("xmm"):
            if src.startswith("xmm"): s.xmm[dst] = s.xmm.get(src, src)
            else:
                mo, w = memop(src, lbl); s.xmm[dst] = val(s, mo)
        else:
            mo, w = memop(dst, lbl)
            v = s.xmm.get(src, src)
            if mo and mo[0] == "slot":
                nm = s.new(v); s.slots[mo[1]] = nm
                s.log[-1] = "  %08X: %s := %s = %s" % (a, mo[1], nm, v)
            else:
                s.log.append("  %08X: store %s <- %s" % (a, dst, v))
    elif op == "movaps":
        s.xmm[ops[0]] = s.xmm.get(ops[1], ops[1])
    elif op in ("subss", "addss", "mulss", "divss"):
        o = {"subss": "-", "addss": "+", "mulss": "*", "divss": "/"}[op]
        src = ops[1]
        v = s.xmm.get(src) if src.startswith("xmm") else val(s, memop(src, lbl)[0])
        s.xmm[ops[0]] = "(%s %s %s)" % (s.xmm.get(ops[0], ops[0]), o, v)
    elif op == "xorps" and ops[0] == ops[1]:
        s.xmm[ops[0]] = "0"
    elif op == "comiss":
        src = ops[1]
        v = s.xmm.get(src) if src.startswith("xmm") else val(s, memop(src, lbl)[0])
        s.cmp = (s.xmm.get(ops[0], ops[0]), v)
    elif op == "test" and ops[0] == ops[1]:
        s.cmp = ("%s" % s.regs.get(ops[0], ops[0]), "0")
    elif op == "cmp":
        s.cmp = (s.regs.get(ops[0], ops[0]), ops[1])
    elif op == "mov" and ops[0] in ("bl", "al", "cl"):
        src = ops[1]
        mo, w = (memop(src.replace("byte", "dword"), lbl) if "ptr" in src else (None, None))
        s.regs[ops[0]] = (mo[1] if mo else src)
        s.log.append("  %08X: %s = %s" % (a, ops[0], s.regs[ops[0]]))
    elif op == "call":
        s.log.append("  %08X: call %s  (fpu top in: %s)" % (a, rest, s.st(0)))
        eff = {"0x42cf10": (1, "asin_clamped"), "0x42be90": (1, "abs"), "0xbf7030": (1, "sqrt"),
               "0x438b10": (1, "wrapsub"), "0x415510": (1, "min"), "0x414c60": (1, "len2"),
               "0x419010": (1, "interp"), "0x7d7da0": (1, "turnrate"), "0xbf701a": (1, "atan2x87"),
               "0x42b2f0": (1, "len3")}.get(rest)
        if rest in ("eax", "edx", "ecx"):
            ind = {0x9c00c8: "leader_heading_vt50()", 0x9c01b6: "own_heading_vt50()"}.get(a)
            if ind: s.fpu.append(ind)
        if eff:
            if rest == "0xbf701a":
                y, x = s.st(1), s.st(0); (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a)); (s.fpu.pop() if s.fpu else s.log.append('  !! x87 underflow at %08X' % a)); s.fpu.append("atan2(%s, %s)" % (y, x))
            elif rest == "0xbf7030":
                s.fpu[-1] = "sqrt(%s)" % s.st(0)
            else:
                s.fpu.append("%s(...)" % eff[1])
    return

def cond(op, c):
    a, b = c if c else ("?", "?")
    return {"ja": "%s > %s", "jae": "%s >= %s", "jb": "%s < %s", "jbe": "%s <= %s",
            "je": "%s == %s", "jne": "%s != %s", "jg": "%s > %s", "jl": "%s < %s",
            "jle": "%s <= %s", "jge": "%s >= %s", "jp": "unordered(%s,%s)", "jnp": "ordered(%s,%s)"}.get(op, op + " %s %s") % (a, b)

paths = []
def dfs(a, s, visits):
    while True:
        if len(paths) >= limit:
            return
        if a in stops:
            s.log.append("  STOP %08X  fpu(top first) = %s" % (a, list(reversed(s.fpu))))
            paths.append(s.log); return
        if not (RANGE[0] <= a <= RANGE[1]):
            return
        if a not in INS:
            s.log.append("  LEFT TRACE at %08X" % a); return
        visits = dict(visits); visits[a] = visits.get(a, 0) + 1
        if visits[a] > maxv:
            return
        txt = INS[a][0]; op = txt.split()[0]
        if op == "jmp":
            a = int(txt.split()[1], 16); continue
        if op.startswith("j"):
            tgt = int(txt.split()[1], 16); c = cond(op, s.cmp)
            for take in ([forced[a]] if a in forced else [True, False]):
                t = s.copy()
                t.log.append("  %08X: %s  -> %s" % (a, ("IF " if take else "NOT ") + c, "%08X" % (tgt if take else NEXT[a])))
                dfs(tgt if take else NEXT[a], t, visits)
            return
        if op == "ret":
            return
        if len(s.fpu) != DEPTH[a]:
            s.log.append("  !! depth mismatch at %08X: walker %d, x87trace %d (%s)" % (a, len(s.fpu), DEPTH[a], txt))
            if SYNC:
                while len(s.fpu) < DEPTH[a]: s.fpu.insert(0, "?")
                while len(s.fpu) > DEPTH[a]: s.fpu.pop(0)
        step(s, a)
        a = NEXT[a]

_s0 = St(); _s0.fpu = list(reversed(INIT)); dfs(start, _s0, {})
print("paths:", len(paths), "unmodelled x87:", UNK)
for k, p in enumerate(paths):
    print("=== path", k)
    print("\n".join(p))
