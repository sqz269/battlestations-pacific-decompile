"""Wiring readiness: which reconstructed game functions are reachable from the bsp_game host, and which
unwired ones are ready to wire (bottom-up), from the Ghidra call graph plus a byte scan of the original exe.

Read-only. Inputs: local/bsp_index.sqlite, build/win32/bsp_game.map, src/, include/, the original exe from
config/target.json, and config/wire_host_shaped.json (optional, see below).
Output: local/output/wire_readiness.json and a short summary on stdout.

    python tools/wire_readiness.py                 # summary
    python tools/wire_readiness.py --list raw_view # ready rows of one ABI class, with their stand-in hosts
    python tools/wire_readiness.py --standins      # ready rows a host file already records, by host file

Approximations (say so when quoting numbers):
- "Wired" is source-level and file-granular: a function counts as wired when its C++ identifier is used in a
  source file reachable from the host roots (game_hosts*.cpp and the bsp_game entry files) through
  identifier references, other than as its own definition. Behind-a-switch uses count as wired.
- A recon record without an _xxxxxxxx-suffixed name gets its identifier from its source file (the name, a
  heading address comment, a snake-case form of a BSP_<Class>_<Method> ledger name), from the map, or from a
  body suffixed with its address anywhere, or a verified config/wire_identifiers*.json entry; rows'
  "ident_via" says which. Rows still without one (table-driven
  shapes, bodies folded into another function, records whose source file no longer has the address) stay
  unknown_no_identifier.
- Callers/callees come from the Ghidra call table plus rel32 CALL/JMP to exact function starts; functions whose
  address is taken (absolute dword in .text or data) are flagged, since vtable/table dispatch has no caller edge.
- A "ready" row is split by the parameter list of its C++ definition (regex, first match in its source file):
    library          the name or evidence reads as STL/CRT: bind it, do not wire it
    typed            plain values or typed structs: the host can call it directly
    callback         takes a *Host& interface the host has to implement
    raw_host_shaped  takes a Raw / void* / byte pointer, and config/wire_host_shaped.json records that the
                     host already keeps that object image-shaped, so Raw over the host's storage works
    raw_view         takes a Raw / void* / byte pointer and needs a typed view overload (Raw kept as a wrapper)
    raw_callback     raw_view that also takes a *Host& interface, which has to be implemented as well
    nodef            no definition matched; check by hand
- "stand_in_hosts" lists the host files whose text mentions the address: usually a record/unimplemented row
  the body would replace, sometimes only a comment.
"""
import argparse, bisect, collections, json, re, sqlite3, struct, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import progress_board as pb  # reuse the game/non-game split and the segment categories

ap = argparse.ArgumentParser(description='wiring readiness of reconstructed game functions')
ap.add_argument('--list', metavar='CLASS', help='print the ready rows of one ABI class')
ap.add_argument('--standins', action='store_true', help='print ready rows that a host file mentions, by host file')
ap.add_argument('--frontier', metavar='LOG', help='rank unwired host stand-ins reached in this run log (native '
                'table calls>0) by the callees-done rows their wiring would unlock')
ap.add_argument('--limit', type=int, default=60)
ap.add_argument('--byte-scan', action='store_true', help='use the old unaligned E8/E9 and dword byte scans instead of the '
                'index edges table (for comparison)')
args = ap.parse_args()

db = sqlite3.connect(f'file:{(ROOT / "local/bsp_index.sqlite").as_posix()}?mode=ro', uri=True)
funcs = {}
lib_like = set()  # callees satisfied by binding: untagged or game-tagged STL/CRT bodies named as such by review
lib_named = set()
LIB_TEXT = re.compile(r'(?i)\bstd::|\bSTL_|red-black tree|std ?(map|list|vector|set|deque|string)\b|\b(CRT|libcpmt|Dinkumware)\b')
for a, name, seg, tag, ln, ev, thunk in db.execute('select address, name, segment, tag_category, ledger_name, ledger_evidence, thunk from functions'):
    funcs[a] = (name, seg, tag)
    # Ghidra thunks are import stubs (xlive.dll, d3d9, fmod...): satisfied by the DLL, never reconstructed.
    # Dyn physics (block_dyn) is in-house Mit/Dyn code per the library inventory and stays a real blocker.
    if thunk or LIB_TEXT.search(' '.join(x for x in (name, ln, ev) if x)):
        lib_like.add(a)
    # The ready rows' own library class reads names only: game bodies often cite std:: in their evidence.
    if thunk or LIB_TEXT.search(' '.join(x for x in (name, ln) if x)):
        lib_named.add(a)
starts = sorted(funcs)
recon = {}
recon_candidates = []
excluded_recon = collections.Counter()
out_of_scope = set()
for a, kind, name, src, status in db.execute('select address, kind, name, source, status from recon'):
    if status in pb.OUT_OF_SCOPE_STATUSES:
        out_of_scope.add(a)
    if kind == 'function' and pb.reconstruction_has_body(status):
        recon.setdefault(a, (name, src))
        recon_candidates.append((a, name, src))
    elif kind == 'function':
        excluded_recon[status] += 1
game = {a: (not v[0].startswith(pb.FUNCLET + pb.LIBRARY_NAME_PREFIXES))
           and pb.is_game(a, v[2]) and a not in out_of_scope for a, v in funcs.items()}

seg_rows = db.execute('select id, start, keywords from segments').fetchall()
seg_cat = {sid: pb.category_for(st, [k.strip() for k in (kw or '').split(',') if k.strip()][:8]) for sid, st, kw in seg_rows}
label = dict(pb.CATEGORIES)

# ---- call graph: Ghidra edges + rel32 scan + address-taken scan of the original image
callers, callees = collections.defaultdict(set), collections.defaultdict(set)
for c, e in db.execute('select caller, callee from calls'):
    if c in funcs and e in funcs:
        callers[e].add(c); callees[c].add(e)
ghidra_edges = sum(len(v) for v in callees.values())

cfg = json.load(open(ROOT / 'config/target.json'))
img = Path(cfg['binary']).read_bytes()
base = int(cfg['image_base'], 16)
pe = struct.unpack_from('<I', img, 0x3c)[0]
nsec = struct.unpack_from('<H', img, pe + 6)[0]
so = pe + 24 + struct.unpack_from('<H', img, pe + 20)[0]
sections = []
for i in range(nsec):
    name = img[so + 40 * i: so + 40 * i + 8].rstrip(b'\0').decode('latin1')
    vsize, va, rsize, raw = struct.unpack_from('<4I', img, so + 40 * i + 8)
    chars = struct.unpack_from('<I', img, so + 40 * i + 36)[0]
    sections.append((name, base + va, img[raw: raw + min(rsize, vsize or rsize)], bool(chars & 0x20000000)))
fset = set(funcs)


def containing(va):
    i = bisect.bisect_right(starts, va) - 1
    return starts[i] if i >= 0 else None


scan_edges = taken_text = 0
address_taken = collections.defaultdict(set)  # func -> taker functions (None = data)
use_index_edges = not args.byte_scan and db.execute("select 1 from sqlite_master where name='edges'").fetchone() \
    and db.execute('select 1 from edges limit 1').fetchone()
if use_index_edges:
    # Per-site edges from tools/callgraph_sweep.py (and callgraph_indirect.py when it has run): instruction-decoded
    # calls/jumps, address-taken immediates, data pointers and code Ghidra never made a function, in place of the
    # unaligned byte scans below (an E8/E9 or dword at any byte offset over-counts). vcall? candidates are not edges.
    for c, e, k in db.execute("select caller, callee, kind from edges where kind in "
                              "('call','tailjmp','addr_taken','data_ptr','vslot','lua')"):
        if e not in fset:
            continue
        if k in ('call', 'tailjmp'):
            if c in fset and c != e and e not in callees[c]:
                callees[c].add(e); callers[e].add(c); scan_edges += 1
            elif c not in fset:
                address_taken[e].add(None)  # reached from undefined code: referenced, no known caller
        else:
            taker = c if k == 'addr_taken' and c in fset else None
            address_taken[e].add(taker)
            taken_text += taker is not None
for name, va0, data, is_code in ([] if use_index_edges else sections):
    n = len(data)
    if is_code:
        for m in re.finditer(rb'[\xe8\xe9]', data):
            i = m.start()
            if i + 5 > n:
                continue
            tgt = (va0 + i + 5 + struct.unpack_from('<i', data, i + 1)[0]) & 0xffffffff
            if tgt in fset:
                src = containing(va0 + i)
                if src is not None and src != tgt and tgt not in callees[src]:
                    callees[src].add(tgt); callers[tgt].add(src); scan_edges += 1
    for i in range(0, n - 3):
        v = struct.unpack_from('<I', data, i)[0]
        if v in fset and v >= base:
            taker = containing(va0 + i) if is_code else None
            address_taken[v].add(taker)
            taken_text += is_code

# ---- source side: identifiers, host-reachable files
mp = (ROOT / 'build/win32/bsp_game.map').read_text(errors='replace')
ident_by_addr = collections.defaultdict(set)
for m in re.finditer(r'\?{1,2}([A-Za-z_][A-Za-z0-9_]*?_([0-9a-f]{8}))@', mp):
    ident_by_addr[int(m.group(2), 16)].add(m.group(1))
srcs = {p: p.read_text(errors='replace') for p in (ROOT / 'src').rglob('*.cpp')}
srcs.update({p: p.read_text(errors='replace') for p in (ROOT / 'include').rglob('*.hpp')})
ident_re = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*_([0-9a-f]{8}))\b')
defined_in = collections.defaultdict(set)  # identifier -> files mentioning it
for p, t in srcs.items():
    for m in ident_re.finditer(t):
        defined_in[m.group(1)].add(p)
for a, (nm, src) in recon.items():  # identifiers named in source records that the map regex did not catch
    if nm and re.search(r'_[0-9a-f]{8}$', nm):
        ident_by_addr[a].add(nm)

# ---- identifiers for recon rows whose name has no _xxxxxxxx suffix (ledger-style or plain C++ names).
# Tried in order: (1) the record's name, or a ' / '-separated part of it, defined in the record's source file;
# (2) an address comment heading the comment block directly above a definition ("// 006E7DE0, ..." or
# "Address: 0x006E7DE0"), first in the record's file, else anywhere if one definition claims it;
# (3) the record's name among the map's symbols, defined in exactly one source file.
# A suffixed definition with another address never counts. A plain identifier defined in more than one file
# (or overloaded in a namespace we cannot tell apart) only counts uses inside its own file, since a mention
# elsewhere could be any of them; uses are counted in code with comments stripped.
KEYWORDS = {'if', 'for', 'while', 'switch', 'catch', 'return', 'sizeof', 'else', 'do', 'case', 'static_assert',
            'decltype', 'alignof', 'defined', 'noexcept', 'operator', 'throw', 'new', 'delete'}
DEF_RE = re.compile(r'^[ \t]*(?:template\s*<[^>{;]*>\s*)?(?:__declspec\([^\n)]*\)\s*)?'
                    r'(?:[\w:<>,\*&~ \t]+?[ \t\*&])?'
                    r'((?:(?:\w+(?:<[^;{}\n]*>)?)::)*(~?\w+))[ \t]*'
                    r'\(([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*)\)[ \t]*(?:const[ \t]*)?(?:noexcept[ \t]*)?'
                    r'(?:override[ \t]*)?(?:->[^{;]*)?(?::[^;{]+)?\s*\{', re.M)
ADDR_HEAD = re.compile(r'^\s*(?://+|/?\*+)\s*(?:Address:?\s*)?(?:0x)?([0-9A-Fa-f]{8})\b|Address:\s*(?:0x)?([0-9A-Fa-f]{8})\b')
COMMENT_LINE = re.compile(r'^\s*(//|/\*|\*)')
strip_comments = lambda t: re.sub(r'//[^\n]*|/\*.*?\*/', ' ', t, flags=re.S)
code_text = {p: strip_comments(t) for p, t in srcs.items()}
code_words = {p: collections.Counter(re.findall(r'\b[A-Za-z_]\w*\b', t)) for p, t in code_text.items()}
defs_in = collections.defaultdict(list)     # file -> [(simple, qualified)]
def_files = collections.defaultdict(set)    # simple name -> files with a body
def_count = collections.defaultdict(collections.Counter)  # file -> simple -> bodies
addr_defs = collections.defaultdict(set)    # address -> {(file, simple)} from the comment block above a body
for p, t in srcs.items():
    lines = None
    for m in DEF_RE.finditer(t):
        simple = m.group(2)
        if simple in KEYWORDS:
            continue
        defs_in[p].append((simple, m.group(1))); def_files[simple].add(p); def_count[p][simple] += 1
        if lines is None:
            lines = t.split('\n'); line_at = [0]
            for ln_ in lines:
                line_at.append(line_at[-1] + len(ln_) + 1)
        i = bisect.bisect_right(line_at, m.start(1)) - 1
        block = [lines[i]]
        j = i - 1
        while j >= 0 and i - j <= 40 and COMMENT_LINE.match(lines[j]):
            block.append(lines[j]); j -= 1
        for ln_ in block:
            h = ADDR_HEAD.search(ln_)
            if h:
                addr_defs[int(h.group(1) or h.group(2), 16)].add((p, simple))
# The same heading rule over header prototypes: many bodies carry their address only at the declaration.
PROTO_RE = re.compile(r'^[ \t]*(?:[\w:<>,\*&~ \t]+?[ \t\*&])((?:\w+::)*(\w+))[ \t]*\([^;{}]*\)[ \t]*'
                      r'(?:const[ \t]*)?(?:noexcept[ \t]*)?;', re.M)
addr_decls = collections.defaultdict(set)  # address -> {(header, simple)}
for p, t in srcs.items():
    if p.suffix != '.hpp':
        continue
    lines = t.split('\n'); line_at = [0]
    for ln_ in lines:
        line_at.append(line_at[-1] + len(ln_) + 1)
    for m in PROTO_RE.finditer(t):
        simple = m.group(2)
        if simple in KEYWORDS:
            continue
        i = bisect.bisect_right(line_at, m.start(1)) - 1
        j = i - 1
        block = [lines[i]]
        while j >= 0 and i - j <= 40 and COMMENT_LINE.match(lines[j]):
            block.append(lines[j]); j -= 1
        for ln_ in block:
            h = ADDR_HEAD.search(ln_)
            if h:
                addr_decls[int(h.group(1) or h.group(2), 16)].add((p, simple))
map_names = set(re.findall(r'\?{1,2}([A-Za-z_]\w*)@', mp))
SUFFIX = re.compile(r'_([0-9a-f]{8})$')
suffixed_defs = collections.defaultdict(set)
for s in def_files:
    if SUFFIX.search(s):
        suffixed_defs[int(SUFFIX.search(s).group(1), 16)].add(s)
ident_via = {}
plain_idents = set()
for a, nm, src in recon_candidates:
    if ident_by_addr.get(a):
        continue
    sp = ROOT / src if src else None
    parts = [x.strip().split('::')[-1].split('(')[0].strip() for x in re.split(r'\s*/\s*|,\s*', nm or '') if x.strip()]
    parts = [x for x in parts if re.fullmatch(r'~?[A-Za-z_]\w*', x)]
    ok = lambda s: not (SUFFIX.search(s) and int(SUFFIX.search(s).group(1), 16) != a)
    hit = [s for s in parts if sp in def_files.get(s, ()) and ok(s)]
    via = 'name'
    if not hit:
        cands = {(f, s) for f, s in addr_defs.get(a, ()) if ok(s)}
        # Two bodies in one file headed by the same address are slices of it (0057cb60's mode arms), not it.
        local = sorted({s for f, s in cands if f == sp})
        decl = sorted({s for f, s in addr_decls.get(a, ()) if ok(s) and s in def_files})
        names = sorted({s for f, s in cands} | set(decl))
        hit = local if len(local) == 1 else decl if len(decl) == 1 else (names if not local and len(names) == 1 else [])
        via = 'address_comment'
    if not hit:
        hit = [s for s in parts if s in map_names and len(def_files.get(s, ())) == 1]
        via = 'map'
    if not hit and sp in defs_in:  # ledger name BSP_<Class>_<Method>: a body in the file named for it in snake case
        ln_ = re.match(r'BSP_([A-Za-z0-9]+)_(\w+)$', nm or '')
        if ln_:
            snake = lambda x: re.sub(r'(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])', '_', x).lower()
            whole, meth = snake(ln_.group(1) + ln_.group(2)), snake(ln_.group(2))
            simple = {s for s, _ in defs_in[sp] if ok(s)}
            bare = lambda s: SUFFIX.sub('', s)
            for want in [whole] + ([meth] if '_' in meth else []):
                m_ = {s for s in simple if bare(s) == want or bare(s).endswith('_' + want)}
                if len(m_) == 1:
                    hit = sorted(m_); break
        via = 'ledger_snake'
    if not hit:  # a body named for this address anywhere, whatever the record calls it
        hit = sorted(suffixed_defs.get(a, ()))
        via = 'suffix_body'
    if hit:
        ident_by_addr[a].update(hit); ident_via[a] = via
        recon[a] = (nm, src)
        plain_idents.update(s for s in hit if not SUFFIX.search(s))
# Explicit mappings carry reviewable address-to-body evidence without modifying busy source files.
# A metadata row, prototype or table accessor is insufficient: each named implementation must have a body.
ident_sources, explicit_mappings = {}, {}
for path in sorted((ROOT / 'config').glob('wire_identifiers*.json')):
    config = json.loads(path.read_text(encoding='utf-8'))
    if config.get('schema_version') != 1:
        raise ValueError(f'{path}: unsupported identifier mapping schema')
    for record in config['identifiers']:
        a = int(record['address'], 16)
        if a in explicit_mappings:
            raise ValueError(f'{path}: duplicate mapping for {a:08x}')
        sp = ROOT / record['source']
        names = record['identifiers']
        if not names or not record.get('evidence'):
            raise ValueError(f'{path}: mapping without body/evidence for {a:08x}')
        known = {simple for simple, qualified in defs_in.get(sp, ())}
        if not set(names) <= known:
            raise ValueError(f'{path}: missing C++ body for {a:08x}: {set(names) - known}')
        if any(SUFFIX.search(s) and int(SUFFIX.search(s).group(1), 16) != a for s in names) and not record.get('shared_implementation'):
            raise ValueError(f'{path}: another address suffix for {a:08x}')
        explicit_mappings[a] = record
        if a not in recon or ident_by_addr.get(a):
            continue
        ident_by_addr[a].update(names)
        ident_via[a] = 'explicit_map'
        ident_sources[a] = record['source']
        plain_idents.update(s for s in names if not SUFFIX.search(s))

DECL_RE = lambda s: re.compile(r'^[ \t]*(?!return\b|else\b|case\b)[\w:<>,\*& \t]*[\w>\*&][ \t]+[\*&]?' + re.escape(s) +
                               r'[ \t]*\([^;{]*\)[ \t]*(?:const[ \t]*)?(?:noexcept[ \t]*)?;', re.M)
own_uses = {}  # (file, plain ident) -> code mentions that are neither its body nor a prototype


def uses_in(f, s):
    k = (f, s)
    if k not in own_uses:
        own_uses[k] = code_words[f][s] - def_count[f][s] - len(DECL_RE(s).findall(code_text[f]))
    return own_uses[k]


for s in plain_idents:
    files = def_files.get(s, set())
    if len(files) == 1:  # a use is a call, an address-of or a qualified name, not a same-named local or field
        use = re.compile(r'(?<!\w)' + re.escape(s) + r'\s*\(|&\s*(?:\w+::)*' + re.escape(s) + r'\b|::' + re.escape(s) + r'\b')
        defined_in[s] = {p for p, c in code_words.items() if c.get(s) and (p in files or use.search(code_text[p]))}
    else:
        defined_in[s] = set(files)

roots = [p for p in srcs if p.suffix == '.cpp' and (p.name.startswith('game_host') or p.name in ('game_main.cpp', 'game_startup.cpp', 'game_app.cpp'))]
file_idents = {p: {m.group(1) for m in ident_re.finditer(t)} for p, t in srcs.items()}
owner_files = collections.defaultdict(set)  # identifier -> files that define/declare it with a body or prototype
for p, t in srcs.items():
    for m in re.finditer(r'^[^\n;]*?\b([A-Za-z_][A-Za-z0-9_]*_[0-9a-f]{8})\s*\([^;{]*\)\s*(?:noexcept\s*)?(?:const\s*)?\{', t, re.M):
        owner_files[m.group(1)].add(p)
for s in plain_idents:  # plain identifiers carry reachability only when one file defines them
    owner_files[s] = set(def_files.get(s, ()))
    if len(owner_files[s]) == 1:
        for p in defined_in[s]:
            file_idents[p].add(s)
reach, todo = set(roots), list(roots)
while todo:
    f = todo.pop()
    for ident in file_idents.get(f, ()):
        for g in owner_files.get(ident, ()):
            if g not in reach:
                reach.add(g); todo.append(g)

host_files = set(roots)


def wired(a):
    for ident in ident_by_addr.get(a, ()):
        users = [f for f in defined_in.get(ident, ()) if f in reach and f.suffix == '.cpp']
        defs = owner_files.get(ident, set())
        if ident in plain_idents and len(def_files.get(ident, ())) > 1:
            # Common method names (dispatch, initialize, release...) must not borrow a
            # use from an unrelated class/file. Ambiguous plain names only count locally.
            implementation = ROOT / ident_sources.get(a, recon[a][1]) if recon[a][1] else None
            users = [f for f in users if f == implementation]
        for f in users:
            if f not in defs:
                return True
            if ident in plain_idents:
                if uses_in(f, ident) > 0:
                    return True
            elif srcs[f].count(ident) > 1:  # used again inside its own file
                return True
    return False


def host_own(a):
    # the host defines its own function at this address (a stand-in or an independent host version)
    for ident in ident_by_addr.get(a, ()):
        files = owner_files.get(ident, ())
        if ident in plain_idents and len(def_files.get(ident, ())) > 1:
            implementation = ROOT / ident_sources.get(a, recon[a][1]) if recon[a][1] else None
            files = [f for f in files if f == implementation]
        if any(f in host_files for f in files):
            return True
    return False


# ---- ABI class of a ready row, from its definition's parameter list
# char* is a C string or a text buffer, not image memory, so it does not make a row raw.
# Object (std::uint8_t*, ten cc13 files) and Pointer (void*) are the image-pointer aliases bodies take.
RAW_PARAM = re.compile(r'\bRaw\b|\bObject\b|\bPointer\b|std::uintptr_t|\bvoid\s*\*|std::uint8_t\s*\*')
HOST_PARAM = re.compile(r'\w*Host\s*&')
shaped_path = ROOT / 'config/wire_host_shaped.json'
host_shaped = {k.lower(): v for k, v in json.load(open(shaped_path)).items()} if shaped_path.exists() else {}
host_text = {p.name: srcs[p].lower() for p in roots}


def abi_class(a, ident, source):
    if a in lib_named:
        return 'library', None
    text = srcs.get(ROOT / source, '') if source else ''
    m = re.search(r'[\w:<>,\s\*&]+?\b' + re.escape(ident) + r'\s*\(([^)]*)\)\s*(const\s*)?(noexcept\s*)?\{', text)
    if m is None:
        return 'nodef', None
    params = m.group(1)
    if RAW_PARAM.search(params):
        if f'{a:08x}' in host_shaped:
            return 'raw_host_shaped', params
        # A raw body that also takes a cc13 *Host& needs that interface implemented first; the
        # view overload alone does not make it callable.
        return ('raw_callback' if HOST_PARAM.search(params) else 'raw_view'), params
    if HOST_PARAM.search(params):
        return 'callback', params
    return 'typed', params


rows = []
counts = collections.Counter()
abi_counts = collections.Counter()
for a, (name, seg, tag) in funcs.items():
    if not game[a] or a not in recon:
        continue
    w = wired(a)
    gc = [c for c in callees[a] if game.get(c)]
    missing = [c for c in gc if c not in recon and c not in lib_like]
    bound = [c for c in gc if c not in recon and c in lib_like]
    cr = [c for c in callers[a] if c is not None]
    wired_callers = [c for c in cr if c in recon and wired(c)]
    taken = a in address_taken
    if w:
        state = 'wired'
    elif not ident_by_addr.get(a):
        state = 'unknown_no_identifier'
    elif not missing and (wired_callers or host_own(a)):
        state = 'ready'
    elif not missing:
        state = 'callees_done_no_wired_caller'
    else:
        state = 'blocked_on_callees'
    counts[state] += 1
    row = dict(address=f'{a:08x}', name=recon[a][0] or name, source=recon[a][1], category=seg_cat.get(seg, 'other'),
               state=state, callers=len(cr), wired_callers=len(wired_callers), callees=len(gc),
               missing_callees=[f'{c:08x}' for c in missing][:12], library_bound_callees=[f'{c:08x}' for c in bound][:12],
               address_taken=taken, host_has_own=host_own(a))
    if a in ident_sources:
        row['implementation_source'] = ident_sources[a]
        row['identifier_evidence'] = explicit_mappings[a]['evidence']
    if a in ident_via:
        row['ident'], row['ident_via'] = sorted(ident_by_addr[a]), ident_via[a]
    if state == 'ready':
        ident = sorted(ident_by_addr[a])[0] if a in ident_via else row['name'].split('::')[-1]
        row['abi'], params = abi_class(a, ident, ident_sources.get(a, row['source']))
        row['params'] = (params or '')[:120].replace('\n', ' ')
        row['stand_in_hosts'] = sorted(n for n, t in host_text.items() if row['address'] in t)
        abi_counts[row['abi']] += 1
    rows.append(row)

by_cat = collections.defaultdict(collections.Counter)
for r in rows:
    by_cat[r['category']][r['state']] += 1
out = ROOT / 'local/output/wire_readiness.json'
out.parent.mkdir(parents=True, exist_ok=True)
json.dump(dict(summary=dict(counts), excluded_recon=dict(excluded_recon),
               ready_abi=dict(abi_counts), by_category={k: dict(v) for k, v in by_cat.items()},
               edges=dict(ghidra=ghidra_edges, rel32_added=scan_edges, address_taken_funcs=len(address_taken)),
               reachable_files=len(reach), roots=len(roots), rows=sorted(rows, key=lambda r: (r['state'], r['category'], r['address']))),
          open(out, 'w'), indent=1)

ready = [r for r in rows if r['state'] == 'ready']
if args.frontier:
    # A root is an unwired recon row whose own address has a native-table row with calls>0 in the log, i.e. a
    # host stand-in the run executes. Its unlock count is the callees_done_no_wired_caller rows reachable from it
    # through unwired recon rows (top-down wiring of that subtree); blocked rows and non-recon callees stop the
    # walk, and a row reachable from several roots counts for each.
    calls = collections.Counter()
    for m in re.finditer(r'^\s+\S.*?\s([0-9a-f]{8})\s+(?:concrete|UNIMPLEMENTED|\w+)\s+calls=(\d+)',
                         Path(args.frontier).read_text(errors='replace'), re.M):
        calls[int(m.group(1), 16)] += int(m.group(2))
    st = {int(r['address'], 16): r for r in rows}
    roots_ = [a for a, r in st.items() if r['state'] != 'wired' and calls[a] > 0]
    ranked, unlocked = [], set()
    takes = collections.defaultdict(set)  # code that takes a function's address dispatches it (tables, callbacks)
    for v, takers in address_taken.items():
        for t_ in takers:
            if t_ is not None and t_ != v:
                takes[t_].add(v)
    for a in roots_:
        seen, todo, done = {a}, [a], []
        while todo:
            x = todo.pop()
            for c in callees[x] | takes.get(x, set()):
                r = st.get(c)
                if c in seen or r is None or r['state'] in ('wired', 'blocked_on_callees'):
                    continue
                seen.add(c); todo.append(c)
                if r['state'] == 'callees_done_no_wired_caller':
                    done.append(c)
        hosts = sorted(n for n, t in host_text.items() if f'{a:08x}' in t)
        ranked.append((len(done), a, st[a], calls[a], hosts))
        unlocked.update(done)
    ranked.sort(key=lambda x: (-x[0], x[1]))
    print(f'{len(roots_)} reached unwired stand-ins unlock {len(unlocked)} distinct of '
          f'{counts["callees_done_no_wired_caller"]} callees-done rows; columns: unlock calls state address name source hosts')
    for n, a, r, k, h in ranked[:args.limit]:
        print(f'{n:5} {k:6} {r["state"][:12]:12} {a:08x} {r["name"][:44]:44} {r["source"]} {",".join(h) or "-"}')
    sys.exit(0)
if args.list:
    sel = [r for r in ready if r['abi'] == args.list]
    print(f'{args.list}: {len(sel)} ready rows')
    for r in sel[:args.limit]:
        print(r['address'], r['name'][:60], r['source'], ','.join(r['stand_in_hosts']) or '-')
    if len(sel) > args.limit:
        print(f'... {len(sel) - args.limit} more in', out)
    sys.exit(0)
if args.standins:
    by_host = collections.defaultdict(list)
    for r in ready:
        for h in r['stand_in_hosts']:
            by_host[h].append(r)
    shown = 0
    for h, v in sorted(by_host.items(), key=lambda kv: -len(kv[1])):
        print(f'== {h} {len(v)}', dict(collections.Counter(r["abi"] for r in v)))
        for r in v:
            if shown >= args.limit:
                break
            print('  ', r['abi'], r['address'], r['name'][:60]); shown += 1
    print('wrote', out)
    sys.exit(0)
print('edges', ghidra_edges, '+rel32', scan_edges, 'address-taken functions', len(address_taken))
print('roots', len(roots), 'reachable source files', len(reach), 'of', sum(1 for p in srcs if p.suffix == '.cpp'))
print('recon game functions', len(rows), dict(counts))
print('ready by ABI class', dict(abi_counts), '| with a host stand-in', sum(1 for r in ready if r['stand_in_hosts']))
no_ident = sum(1 for r in rows if not ident_by_addr.get(int(r['address'], 16)))
print('without any identifier (cannot be judged wired):', no_ident)
print('identifiers resolved without an address suffix, by method:', dict(collections.Counter(ident_via[int(r['address'], 16)]
      for r in rows if int(r['address'], 16) in ident_via)))
for k, v in sorted(by_cat.items(), key=lambda kv: -sum(kv[1].values())):
    print(f'{label.get(k, k):32}', dict(v))
print('wrote', out)
