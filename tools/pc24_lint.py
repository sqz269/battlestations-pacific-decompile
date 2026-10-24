"""PC24 lint: flag new un-narrowed multi-op double chains in reconstructed x87 bodies.

The main thread runs the image's x87 at 24-bit precision control, so every FADD/FSUB/FMUL/FDIV
rounds to float. C++ that evaluates two or more of those operations in double and rounds once
(or compares the unrounded double) can differ from the image. This lint finds such chains.

  python tools/pc24_lint.py --diff origin/main...HEAD    statements HEAD added or changed since the
                                                         merge-base (BASE..TIP means the same)
  python tools/pc24_lint.py --staged                     the index against HEAD
  python tools/pc24_lint.py src/foo.cpp [...]            every statement in the files

A finding needs all of:
  - a body whose image address (a _XXXXXXXX name suffix, else the first address in its comments)
    has at least two x87 arithmetic instructions in the exports listing (tools/workspace.py);
  - a double op whose operand is itself an un-narrowed double op result, directly or through a
    double local. Single double ops on float operands are exact-then-round and pass; so do
    literal (x) literal, power-of-two scaling and +0.0.
Excluded: the Dyn physics range 00C30000-00C5FFFF, bodies with __asm, the OFF arm of a
`k...Bound` switch (the else of `if [constexpr] (kX)`, the if of `(!kX)`, the third operand of
`kX ? on : off`; --include-off keeps them), and in diff mode a statement whose text already
existed in the base version of the file (moved or re-indented code).
Justify a real case with `// pc24-lint: allow <reason>` on the statement or the line above.

Exit status: 0 clean, 1 findings, 2 usage error. Output is capped like bsp.py (spill to
local/output/). The typed expression parser is rev7's census detector (cc13, 2026-10-24).
"""
import argparse
import bisect
import collections
import math
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import workspace  # noqa: E402

ROOT = workspace.ROOT
FUNCS = workspace.exports_dir() / 'functions'
DYN = (0x00C30000, 0x00C60000)
SOURCE_EXT = ('.cpp', '.cc', '.hpp', '.h', '.inl')
ADDR_RE = re.compile(r'(?<![0-9A-Fa-fx])(00[4-9a-cA-C][0-9A-Fa-f]{5})(?![0-9A-Fa-f])')
X87_ARITH = re.compile(r'^\s*[0-9a-f]{8}:\s+F(I?ADD|I?SUBR?|I?MUL|I?DIVR?)P?\b', re.I)
CONTAINER = re.compile(r'^\s*(template\s*<[^>]*>\s*)?(namespace\b|extern\s+"[^"]*"|(struct|class|union|enum)\b(?![^(]*\())')
CONTROL = re.compile(r'^\s*(if|for|while|switch|catch|do|else|try|return)\b')
SWITCH = re.compile(r'^!?(?:\w+::)*k\w*Bound$')
ALLOW = re.compile(r'pc24-lint:\s*allow\b\s*(\S.*)?', re.I)

# ---------------------------------------------------------------------------
# Typed double-chain detector (rev7_dchain).
# ---------------------------------------------------------------------------
TOK = re.compile(r"""\s*(
    (?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?[fFlLuU]*|0[xX][0-9a-fA-F]+[uUlL]*|
    ->|::|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||[+\-*/%&|^]=|
    [A-Za-z_]\w*|
    [-+*/%<>=!&|^~?:,.;()\[\]{}]
)""", re.X)
MATH = {'sqrt', 'sin', 'cos', 'tan', 'atan', 'atan2', 'asin', 'acos', 'exp', 'log', 'pow', 'hypot', 'fmod', 'log10'}
EXACT = {'fabs', 'abs', 'floor', 'ceil', 'trunc', 'round', 'copysign', 'min', 'max'}
SKIP_KW = {'return', 'else', 'const', 'constexpr', 'static', 'auto', 'case', 'do', 'new', 'delete', 'throw',
           'if', 'while', 'for', 'switch', 'volatile', 'inline', 'typename', 'unsigned', 'signed', 'long',
           'short', 'int', 'bool', 'char', 'void', 'default', 'break', 'continue', 'goto', 'using', 'static_assert'}
TYPE_WORDS = {'float': 'f', 'double': 'd'}
IDENT = re.compile(r'[A-Za-z_]\w*$')


class V:
    __slots__ = ('t', 'dop', 'lit', 'name')

    def __init__(s, t='u', dop=False, lit=False, name=None):
        s.t, s.dop, s.lit, s.name = t, dop, lit, name


class P:
    LEVELS = [('||',), ('&&',), ('|',), ('^',), ('&',), ('==', '!='), ('<', '>', '<=', '>='),
              ('<<', '>>'), ('+', '-'), ('*', '/', '%')]

    def __init__(s, toks, env, include_off):
        s.k, s.i, s.env, s.include_off = toks, 0, env, include_off
        s.ops = s.chains = s.off = 0

    def peek(s, o=0):
        j = s.i + o
        return s.k[j] if j < len(s.k) else None

    def take(s):
        t = s.k[s.i]
        s.i += 1
        return t

    def expect(s, t):
        if s.peek() != t:
            raise SyntaxError(t)
        s.i += 1

    def expr(s):
        v = s.assign()
        while s.peek() == ',':
            s.take()
            v = s.assign()
        return v

    def assign(s):
        lhs = s.cond()
        op = s.peek()
        if op == '=':
            s.take()
            rhs = s.assign()
            if lhs.name and s.env['types'].get(lhs.name) == 'd':
                s.env['dop'][lhs.name] = rhs.dop
            return V(lhs.t if lhs.t != 'u' else rhs.t, rhs.dop if lhs.t == 'd' else False)
        if op in ('+=', '-=', '*=', '/='):
            s.take()
            rhs = s.assign()
            if lhs.t == 'd' or rhs.t == 'd':
                s.ops += 1
                if rhs.dop or lhs.dop:
                    s.chains += 1
                if lhs.name and s.env['types'].get(lhs.name) == 'd':
                    s.env['dop'][lhs.name] = True
            return V(lhs.t)
        if op in ('%=', '&=', '|=', '^=', '<<=', '>>='):
            s.take()
            s.assign()
            return V('i')
        return lhs

    def cond(s):
        start = s.i
        c = s.binary(0)
        if s.peek() != '?':
            return c
        sw = ''.join(t for t in s.k[start:s.i] if t not in ('(', ')'))
        s.take()
        c0 = s.chains
        a = s.assign()
        c1 = s.chains
        s.expect(':')
        b = s.assign()
        c2 = s.chains
        if SWITCH.match(sw) and not s.include_off:
            dropped = (c1 - c0) if sw.startswith('!') else (c2 - c1)   # the OFF operand
            s.chains -= dropped
            s.off += dropped
        t = 'd' if 'd' in (a.t, b.t) else a.t
        return V(t, a.dop or b.dop)

    def binary(s, lvl):
        if lvl == len(s.LEVELS):
            return s.unary()
        a = s.binary(lvl + 1)
        while s.peek() in s.LEVELS[lvl]:
            op = s.take()
            b = s.binary(lvl + 1)
            if op in ('+', '-', '*', '/'):
                if a.lit and b.lit:
                    a = V('d' if 'd' in (a.t, b.t) else a.t, False, True)
                elif (op in '*/' and b.lit and b.name and b.name.startswith('#exact')) or \
                     (op == '*' and a.lit and a.name and a.name.startswith('#exact')) or \
                     (op in '+-' and b.lit and b.name == '#exact0'):
                    keep = a if not (a.lit and a.name and a.name.startswith('#exact')) else b
                    a = V('d' if 'd' in (a.t, b.t) else keep.t, keep.dop)
                elif op == '+' and a.lit and a.name == '#exact0':
                    a = V('d' if 'd' in (a.t, b.t) else b.t, b.dop)
                elif 'd' in (a.t, b.t):
                    s.ops += 1
                    if a.dop or b.dop:
                        s.chains += 1
                    a = V('d', True)
                else:
                    a = V('f' if 'f' in (a.t, b.t) else a.t)
            else:
                a = V('i')
        return a

    def unary(s):
        t = s.peek()
        if t in ('-', '+', '!', '~', '*', '&', '++', '--'):
            s.take()
            v = s.unary()
            return v if t in ('-', '+', '*') else V('i' if t in ('!', '~') else v.t)
        if t == 'sizeof':
            s.take()
            s.unary()
            return V('i', lit=True)
        if t == '(':
            j, words = s.i + 1, []
            while j < len(s.k) and s.k[j] != ')' and (IDENT.match(s.k[j]) or s.k[j] in ('::', '*', '&')):
                words.append(s.k[j])
                j += 1
            closes = j < len(s.k) and s.k[j] == ')'
            if closes and words and words[-1] in ('double', 'float', 'int', 'unsigned', 'long', 'char', 'short') or \
               (closes and words and words[0] in ('std', 'uint32_t', 'int32_t', 'bool') and j + 1 < len(s.k)
                    and s.k[j + 1] not in (')', ';', ',', '+', '-', '*', '/', '?', ':', '<', '>', '=', '==',
                                           '&&', '||', '.', '->', '[')):
                s.i = j + 1
                v = s.unary()
                if words[-1] == 'double':
                    return V('d', v.dop, v.lit)
                if words[-1] == 'float':
                    return V('f', False, v.lit)
                return V('i', False, v.lit)
        return s.postfix()

    def template_args(s):
        depth, j, words = 0, s.i, []
        while j < len(s.k):
            t = s.k[j]
            if t == '<':
                depth += 1
            elif t == '>':
                depth -= 1
                if depth == 0:
                    break
            elif t == '>>':
                depth -= 2
                if depth <= 0:
                    break
            elif not (IDENT.match(t) or t in ('::', '*', '&', ',', 'const') or re.match(r'\d', t)):
                return None
            else:
                words.append(t)
            j += 1
        if j >= len(s.k) or s.k[j + 1:j + 2] and s.k[j + 1] not in ('(', '::', '{'):
            return None
        s.i = j + 1
        return words

    def postfix(s):
        v = s.primary()
        while True:
            t = s.peek()
            if t == '(':
                s.take()
                args = []
                if s.peek() != ')':
                    args.append(s.assign())
                    while s.peek() == ',':
                        s.take()
                        args.append(s.assign())
                s.expect(')')
                v = s.call(v, args)
            elif t == '[':
                s.take()
                s.expr()
                s.expect(']')
                v = V('u')
            elif t in ('.', '->'):
                s.take()
                s.take()
                if s.peek() == '<':
                    ta = s.template_args()
                    v = V(TYPE_WORDS.get(ta[0], 'u') if ta else 'u', name='#tmpl')
                else:
                    v = V('u')
            elif t in ('++', '--'):
                s.take()
            else:
                return v

    def call(s, f, args):
        fn = f.name or ''
        base = fn.split('::')[-1]
        if fn.startswith('#cast:'):
            ty = fn[6:]
            a = args[0] if args else V()
            if ty == 'double':
                return V('d', a.dop, a.lit)
            if ty == 'float':
                return V('f', False, a.lit)
            return V('i')
        if fn == '#tmpl':
            return V(f.t)
        if base in MATH:
            if any(a.t == 'd' for a in args):
                s.ops += 1
                if any(a.dop for a in args):
                    s.chains += 1
                return V('d', True)
            return V('f')
        if base in EXACT:
            return V('d' if any(a.t == 'd' for a in args) else 'u', any(a.dop for a in args))
        if base in s.env['dfuncs']:
            return V('d')
        return V('u')

    def primary(s):
        t = s.peek()
        if t is None:
            raise SyntaxError('eof')
        if t == '(':
            s.take()
            v = s.expr()
            s.expect(')')
            return v
        if t == '{':
            s.take()
            depth = 1
            while depth and s.peek() is not None:
                x = s.take()
                depth += (x == '{') - (x == '}')
            return V('u')
        if re.match(r'(\d|\.\d)', t):
            s.take()
            if re.match(r'0[xX]', t) or not re.search(r'[.eE]', t):
                return V('i', lit=True)
            try:
                val = float(t.rstrip('fFlL'))
            except ValueError:
                val = None
            tag = None
            if val == 0.0:
                tag = '#exact0'
            elif val is not None and math.frexp(abs(val))[0] == 0.5:
                tag = '#exact2'
            return V('f' if t[-1] in 'fF' else 'd', lit=True, name=tag)
        if IDENT.match(t):
            name = s.take()
            while s.peek() == '::' and re.match(r'[A-Za-z_~]\w*$', s.peek(1) or ''):
                s.take()
                name += '::' + s.take()
            if name in ('static_cast', 'reinterpret_cast', 'const_cast', 'bit_cast', 'std::bit_cast') and s.peek() == '<':
                ta = s.template_args() or ['?']
                ty = ta[-1] if ta[-1] in ('double', 'float') else 'other'
                if name.endswith('bit_cast') or name == 'reinterpret_cast':
                    ty = 'other'
                return V('u', name='#cast:' + ty)
            if s.peek() == '<':
                save = s.i
                ta = s.template_args()
                if ta is not None:
                    return V(TYPE_WORDS.get(ta[0], 'u'), name='#tmpl')
                s.i = save
            if name in ('true', 'false', 'nullptr'):
                return V('i', lit=True)
            tag = s.env['exact'].get(name.split('::')[-1])
            if tag and name.split('::')[-1] not in s.env['dop']:
                return V(tag[0], lit=True, name=tag[1])
            ty = s.env['types'].get(name.split('::')[-1])
            if ty == 'd':
                key = name.split('::')[-1]
                return V('d', s.env['dop'].get(key, False), key in s.env['consts'], key)
            if ty == 'f':
                return V('f', name=name)
            return V('u', name=name)
        raise SyntaxError(t)


DECL = re.compile(r'\b(double|float)\s*&?\s+(\w+)\s*(?=[=;,)({\[])')
DFUNC = re.compile(r'\bdouble\s+(\w+)\s*\(')
CONSTD = re.compile(r'\b(?:constexpr|const)\s+(?:static\s+)?double\s+(\w+)')


CONST_VALUE = re.compile(r'\bconstexpr\s+(double|float)\s+(\w+)\s*=\s*([-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?)[fF]?\s*;')
_header_consts = None


def exact_consts(code):
    """Named constants equal to 0 or a power of two ('#exact0' / '#exact2'), from include/ and this
    file. A name defined with differing values anywhere is left out."""
    global _header_consts

    def collect(text, into):
        for ty, name, value in CONST_VALUE.findall(text):
            val = float(value)
            tag = '#exact0' if val == 0.0 else ('#exact2' if math.frexp(abs(val))[0] == 0.5 else None)
            into[name].add((('d' if ty == 'double' else 'f'), tag))

    if _header_consts is None:
        _header_consts = collections.defaultdict(set)
        for p in (ROOT / 'include').rglob('*.hpp'):
            collect(strip(p.read_text(encoding='utf-8', errors='replace'))[0], _header_consts)
    merged = collections.defaultdict(set, {k: set(v) for k, v in _header_consts.items()})
    collect(code, merged)
    return {k: next(iter(v)) for k, v in merged.items() if len(v) == 1 and next(iter(v))[1]}


def file_scope(code):
    consts = set(CONSTD.findall(code))
    return {'types': {n: 'd' for n in consts}, 'consts': consts, 'dfuncs': set(DFUNC.findall(code)),
            'exact': exact_consts(code)}


# ---------------------------------------------------------------------------
# Source structure: comments/strings blanked, function bodies, OFF arms.
# ---------------------------------------------------------------------------
def strip(text):
    """Blank comments and string literals (offsets kept); return (code, comments by line)."""
    out, i, n, line = [], 0, len(text), 1
    comments = collections.defaultdict(str)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            comments[line] += text[i:j]
            out.append(' ' * (j - i))
            i = j
            continue
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            seg = text[i:j]
            for k, piece in enumerate(seg.split('\n')):
                comments[line + k] += piece
            out.append(re.sub(r'[^\n]', ' ', seg))
            line += seg.count('\n')
            i = j
            continue
        if c == 'R' and text.startswith('R"', i):
            m = re.match(r'R"([^(]*)\(', text[i:])
            if m:
                end = text.find(')' + m.group(1) + '"', i)
                j = n if end < 0 else end + len(m.group(1)) + 2
                seg = text[i:j]
                out.append(re.sub(r'[^\n]', ' ', seg))
                line += seg.count('\n')
                i = j
                continue
        if c in '"\'':
            if c == "'" and i > 0 and text[i - 1].isalnum():   # digit separator 1'000
                out.append(c)
                i += 1
                continue
            j = i + 1
            while j < n and text[j] != c and text[j] != '\n':
                j += 2 if text[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(c + ' ' * (j - i - 2) + c if j - i >= 2 else ' ' * (j - i))
            i = j
            continue
        if c == '\n':
            line += 1
        out.append(c)
        i += 1
    return ''.join(out), comments


def match_close(code, i, opening, closing):
    depth = 0
    for j in range(i, len(code)):
        if code[j] == opening:
            depth += 1
        elif code[j] == closing:
            depth -= 1
            if depth == 0:
                return j
    return len(code) - 1


def statement_end(code, i):
    """End index (inclusive) of the statement starting at or after i."""
    while i < len(code) and code[i].isspace():
        i += 1
    if i >= len(code):
        return len(code) - 1
    if code[i] == '{':
        return match_close(code, i, '{', '}')
    m = re.compile(r'(if|while|for|switch)\s*(constexpr\s*)?\(').match(code, i)
    if m:
        close = match_close(code, m.end() - 1, '(', ')')
        end = statement_end(code, close + 1)
        if m.group(1) == 'if':
            e = re.compile(r'\s*else\b').match(code, end + 1)
            if e:
                end = statement_end(code, e.end())
        return end
    depth = 0
    for j in range(i, len(code)):
        ch = code[j]
        if ch in '({[':
            depth += 1
        elif ch in ')}]':
            depth -= 1
        elif ch == ';' and depth <= 0:
            return j
    return len(code) - 1


IF_SWITCH = re.compile(r'\bif\s*(constexpr\s*)?\(')


def off_ranges(code):
    """Character ranges holding the OFF arm of an `if [constexpr] ([!]k...Bound)` switch."""
    ranges = []
    for m in IF_SWITCH.finditer(code):
        close = match_close(code, m.end() - 1, '(', ')')
        cond = ''.join(code[m.end():close].split())
        if not SWITCH.match(cond):
            continue
        then_end = statement_end(code, close + 1)
        e = re.compile(r'\s*else\b').match(code, then_end + 1)
        if cond.startswith('!'):
            ranges.append((close + 1, then_end))
        elif e:
            ranges.append((e.end(), statement_end(code, e.end())))
    return ranges


def functions(code):
    """Function bodies: dict(name, header, start, end (lines), body, body_pos)."""
    line_starts = [0] + [m.end() for m in re.finditer('\n', code)]

    def lineno(pos):
        return bisect.bisect_right(line_starts, pos)

    res, i, n, header_start = [], 0, len(code), 0
    while i < n:
        c = code[i]
        if c in ';}':
            header_start = i + 1
            i += 1
            continue
        if c != '{':
            i += 1
            continue
        header = '\n'.join(l for l in code[header_start:i].split('\n') if not l.lstrip().startswith('#'))
        header = re.sub(r'^\s*(public|private|protected)\s*:', '', header.strip())
        h = ' '.join(header.split())
        if CONTAINER.match(h) or not h:
            header_start = i + 1
            i += 1
            continue
        j = match_close(code, i, '{', '}')
        if '(' in h and ')' in h and not CONTROL.match(h) and '=' not in h.split('(')[0] \
                and not h.rstrip().endswith(('=', ',')):
            m = re.search(r'([\w:~]+)\s*\([^()]*(\([^()]*\)[^()]*)*\)\s*(const|noexcept|override|final|\s)*'
                          r'(->[^{]*)?(:[^{]*)?$', h)
            words = h.split('(')[0].split()
            name = m.group(1) if m else (words[-1] if words else '?')
            first = header_start + (len(code[header_start:i]) - len(code[header_start:i].lstrip()))
            res.append(dict(name=name, header=h, start=lineno(first), end=lineno(j), body=code[i:j + 1],
                            body_pos=i, bline=lineno(i)))
        header_start = j + 1
        i = j + 1
    return res, lineno


def doc_address(fn, comments):
    """The body's image address: a _XXXXXXXX name suffix, else the first address in its comments."""
    m = re.search(r'_([0-9a-fA-F]{8})\b', fn['name'])
    if m and ADDR_RE.fullmatch(m.group(1)):
        return m.group(1).upper()
    above, k = [], fn['start'] - 1
    while k > 0 and k in comments and len(above) < 40:
        above.append(comments[k])
        k -= 1
    text = ' '.join(reversed(above)) + ' ' + ' '.join(comments.get(x, '') for x in range(fn['start'], fn['end'] + 1))
    mm = ADDR_RE.search(text)
    return mm.group(1).upper() if mm else None


# ---------------------------------------------------------------------------
# Image side: x87 arithmetic count of the function holding an address.
# ---------------------------------------------------------------------------
_x87, _starts = {}, None


def image_function(addr):
    """(start, x87 count) of the exported function holding addr, or (None, None)."""
    global _starts
    if _starts is None:
        try:
            _starts = sorted(int(d.name, 16) for d in os.scandir(FUNCS)
                             if d.is_dir() and re.fullmatch(r'[0-9A-Fa-f]{8}', d.name))
        except OSError:
            _starts = []
    a = int(addr, 16)
    k = bisect.bisect_right(_starts, a) - 1
    if k < 0:
        return None, None
    start = '%08X' % _starts[k]
    if start not in _x87:
        try:
            lines = (FUNCS / start / 'assembly.txt').read_text(encoding='utf-8', errors='replace').splitlines()
        except OSError:
            lines = []
        last = 0
        for ln in lines:
            mm = re.match(r'\s*([0-9a-fA-F]{8}):', ln)
            if mm:
                last = max(last, int(mm.group(1), 16))
        _x87[start] = (last, sum(1 for ln in lines if X87_ARITH.match(ln)))
    last, count = _x87[start]
    if a != _starts[k] and a > last:
        return None, None
    return start, count


# ---------------------------------------------------------------------------
# Per-file analysis.
# ---------------------------------------------------------------------------
INT_DECL = re.compile(r'(?:^|[(,])\s*(?:const\s+|constexpr\s+|static\s+)*'
                      r'((?:std::)?(?:u?int(?:8|16|32|64)_t|size_t|ptrdiff_t|uintptr_t)|unsigned|int|long|short|'
                      r'char|bool)\s+(?:const\s+)?(\w+)\s*(?==[^=]|[;,{(]|$)')


def retype(st, env):
    """A declaration re-types its name from here on (a body may reuse `width` as float, then int)."""
    for ty, nm in DECL.findall(st):
        env['types'][nm] = 'd' if ty == 'double' else 'f'
        env['dop'].pop(nm, None)
    for _, nm in INT_DECL.findall(st):
        env['types'][nm] = 'i'
        env['dop'].pop(nm, None)


def statements(fn, code, lineno, env, offs, include_off):
    """Yield (first line, last line, chains, text) for statements of fn with a double chain."""
    body, base = fn['body'], fn['body_pos']
    pos = 0
    for part in re.split(r'([;{}])', body):
        here = base + pos
        pos += len(part)
        st = part.strip()
        if not st or st in ';{}':
            continue
        first = here + (len(part) - len(part.lstrip()))
        last = here + len(part.rstrip()) - 1
        retype(st, env)
        if not include_off and any(lo <= first <= hi for lo, hi in offs):
            continue
        toks = [m.group(1) for m in TOK.finditer(st)]
        p = P(toks, env, include_off)
        while p.i < len(toks):
            t = toks[p.i]
            if t in SKIP_KW or t in (')', ':', ']', ','):
                p.i += 1
                continue
            if IDENT.match(t) and p.i + 1 < len(toks) and IDENT.match(toks[p.i + 1]) and toks[p.i + 1] not in SKIP_KW:
                p.i += 1
                continue
            start = p.i
            try:
                p.expr()
            except (SyntaxError, IndexError, RecursionError, AttributeError, TypeError):
                p.i = start + 1
            if p.i == start:
                p.i += 1
        if p.chains:
            yield lineno(first), lineno(last), p.chains, ' '.join(st.split())


def analyze_text(text, include_off, added=None):
    """Chain statements of a file (only those touching the added lines when given), plus counters."""
    code, comments = strip(text)
    fscope = file_scope(code)
    offs = [] if include_off else off_ranges(code)
    fns, lineno = functions(code)
    found, stats = [], collections.Counter()
    for fn in fns:
        env = {'types': dict(fscope['types']), 'dop': {}, 'dfuncs': fscope['dfuncs'], 'consts': set(fscope['consts']),
               'exact': fscope['exact']}
        for ty, nm in DECL.findall(fn['header'] + ' ' + fn['body']):
            env['types'][nm] = 'd' if ty == 'double' else 'f'
        env['consts'] |= set(CONSTD.findall(fn['body']))
        hits = [h for h in statements(fn, code, lineno, env, offs, include_off)
                if added is None or any(ln in added for ln in range(h[0], h[1] + 1))]
        if not hits:
            continue
        if re.search(r'\b__asm\b|\basm\s*\(', fn['body']):
            stats['asm'] += 1
            continue
        addr = doc_address(fn, comments)
        if not addr:
            stats['no_address'] += len(hits)
            continue
        if DYN[0] <= int(addr, 16) < DYN[1]:
            stats['dyn'] += len(hits)
            continue
        image, x87 = image_function(addr)
        if image is None:
            stats['no_export'] += len(hits)
            continue
        if x87 < 2:
            stats['image_below_2_x87'] += len(hits)
            continue
        for first, last, chains, st in hits:
            allow = None
            for ln in range(first - 1, last + 1):
                am = ALLOW.search(comments.get(ln, ''))
                if am:
                    allow = (am.group(1) or '').strip() or '(no reason given)'
                    break
            found.append(dict(first=first, last=last, chains=chains, text=st, fn=fn['name'], addr=addr,
                              image=image, x87=x87, allow=allow))
    return found, stats


def statement_texts(text):
    """Whitespace-normalised statement texts of every body (for moved-code detection)."""
    code, _ = strip(text)
    out = set()
    for fn in functions(code)[0]:
        for part in re.split(r'[;{}]', fn['body']):
            st = ' '.join(part.split())
            if st:
                out.add(st)
    return out


# ---------------------------------------------------------------------------
# Git plumbing.
# ---------------------------------------------------------------------------
def git(*args, check=True):
    r = subprocess.run(['git', *args], cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
    if check and r.returncode != 0:
        raise RuntimeError(r.stderr.strip() or 'git ' + ' '.join(args))
    return r.stdout if r.returncode == 0 else None


def added_lines(diff_args, path):
    lines = set()
    out = git('diff', '-U0', '--no-color', '--no-ext-diff', *diff_args, '--', path)
    for m in re.finditer(r'^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@', out, re.M):
        start, count = int(m.group(1)), int(m.group(2) or '1')
        lines.update(range(start, start + count))
    return lines


def main(argv):
    ap = argparse.ArgumentParser(description='Flag new un-narrowed multi-op double chains in x87 bodies (PC24).')
    ap.add_argument('files', nargs='*', help='source files: every statement is checked')
    ap.add_argument('--diff', metavar='BASE...TIP',
                    help='statements TIP added or changed since merge-base(BASE, TIP); BASE..TIP means the same, '
                         'BASE alone means BASE...HEAD')
    ap.add_argument('--no-merge-base', action='store_true',
                    help='diff BASE itself against TIP (a literal two-dot range)')
    ap.add_argument('--staged', action='store_true', help='statements the index adds against HEAD')
    ap.add_argument('--include-off', action='store_true', help='also check the OFF arms of k...Bound switches')
    ap.add_argument('--show-allowed', action='store_true', help='list statements carrying a pc24-lint allow marker')
    ap.add_argument('--full', action='store_true', help='no output cap')
    args = ap.parse_args(argv)
    if sum(bool(x) for x in (args.files, args.diff, args.staged)) != 1:
        ap.print_usage(sys.stderr)
        sys.stderr.write('pc24_lint.py: give exactly one of FILES, --diff or --staged\n')
        return 2

    targets = []   # (path, text, added line set or None, base text or None)
    if args.files:
        for f in args.files:
            p = Path(f) if Path(f).is_absolute() else ROOT / f
            if not p.is_file():
                sys.stderr.write(f'pc24_lint.py: no such file {f}\n')
                return 2
            rel = os.path.relpath(p, ROOT).replace('\\', '/')
            targets.append((rel, p.read_text(encoding='utf-8', errors='replace'), None, None))
    else:
        try:
            if args.staged:
                base_rev, diff_args, tip_spec = 'HEAD', ['--cached'], ':'
            else:
                # BASE...TIP and BASE..TIP both diff from merge-base(BASE, TIP): on a branch behind
                # BASE the literal two-dot range would lint BASE's newer changes, reversed.
                spec = args.diff
                base_rev, _, tip = spec.partition('...') if '...' in spec else spec.partition('..')
                tip = tip or 'HEAD'
                if not args.no_merge_base:
                    base_rev = git('merge-base', base_rev, tip).strip()
                diff_args, tip_spec = [base_rev, tip], tip + ':'
            names = git('diff', '--name-only', '--diff-filter=AM', '--no-color', *diff_args).split()
        except RuntimeError as e:
            sys.stderr.write(f'pc24_lint.py: {e}\n')
            return 2
        for rel in names:
            if not rel.endswith(SOURCE_EXT):
                continue
            text = git('show', tip_spec + rel)
            base_text = git('show', f'{base_rev}:{rel}', check=False) or ''
            targets.append((rel, text, added_lines(diff_args, rel), base_text))

    findings, allowed, stats = [], [], collections.Counter()
    for rel, text, added, base_text in targets:
        found, st = analyze_text(text, args.include_off, added)
        stats.update(st)
        base_set = statement_texts(base_text) if added is not None and base_text else set()
        for f in found:
            if added is not None:
                if f['text'] in base_set:
                    stats['moved_from_base'] += 1
                    continue
            f['file'] = rel
            (allowed if f['allow'] else findings).append(f)

    # The summary comes first so a capped output still carries it.
    skipped = ', '.join(f'{k} {v}' for k, v in sorted(stats.items()))
    print(f'pc24_lint: {len(findings)} finding(s), {len(allowed)} allowed, {len(targets)} file(s)'
          + (f'; not checked: {skipped}' if skipped else ''))
    if findings:
        print('each finding is a double op fed by an un-narrowed double result, so the chain rounds once '
              'where the PC24 image rounds every FADD/FSUB/FMUL/FDIV to float: narrow per op, or justify '
              'it with `// pc24-lint: allow <reason>`')

    def show(f, reason):
        text = f['text'] if len(f['text']) <= 160 else f['text'][:157] + '...'
        print(f"{f['file']}:{f['first']}  {f['addr']} {f['fn']} (image {f['image']}, {f['x87']} x87 ops): {reason}")
        print(f'    {text}')

    for f in findings:
        show(f, f"{f['chains']} chained double op(s)")
    if args.show_allowed:
        for f in allowed:
            show(f, f"allowed: {f['allow']}")
    return 1 if findings else 0


if __name__ == '__main__':
    argv = sys.argv[1:]
    full = '--full' in argv
    import bsp   # noqa: E402  (the shared output cap)
    code = 0
    with bsp.output_budget(argv=['pc24_lint', *argv], full=full):
        code = main(argv)
    sys.exit(code)
