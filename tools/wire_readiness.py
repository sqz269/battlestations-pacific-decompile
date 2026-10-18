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
- Callers/callees come from the Ghidra call table plus rel32 CALL/JMP to exact function starts; functions whose
  address is taken (absolute dword in .text or data) are flagged, since vtable/table dispatch has no caller edge.
- A "ready" row is split by the parameter list of its C++ definition (regex, first match in its source file):
    library          the name or evidence reads as STL/CRT: bind it, do not wire it
    typed            plain values or typed structs: the host can call it directly
    callback         takes a *Host& interface the host has to implement
    raw_host_shaped  takes a Raw / void* / byte pointer, and config/wire_host_shaped.json records that the
                     host already keeps that object image-shaped, so Raw over the host's storage works
    raw_view         takes a Raw / void* / byte pointer and needs a typed view overload (Raw kept as a wrapper)
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
ap.add_argument('--limit', type=int, default=60)
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
for a, kind, name, src in db.execute('select address, kind, name, source from recon'):
    if kind == 'function':
        recon.setdefault(a, (name, src))
game = {a: (not v[0].startswith(pb.FUNCLET)) and pb.is_game(a, v[2]) for a, v in funcs.items()}

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
for name, va0, data, is_code in sections:
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

roots = [p for p in srcs if p.suffix == '.cpp' and (p.name.startswith('game_host') or p.name in ('game_main.cpp', 'game_startup.cpp', 'game_app.cpp'))]
file_idents = {p: {m.group(1) for m in ident_re.finditer(t)} for p, t in srcs.items()}
owner_files = collections.defaultdict(set)  # identifier -> files that define/declare it with a body or prototype
for p, t in srcs.items():
    for m in re.finditer(r'^[^\n;]*?\b([A-Za-z_][A-Za-z0-9_]*_[0-9a-f]{8})\s*\([^;{]*\)\s*(?:noexcept\s*)?(?:const\s*)?\{', t, re.M):
        owner_files[m.group(1)].add(p)
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
        for f in users:
            if f not in defs:
                return True
            if srcs[f].count(ident) > 1:  # used again inside its own file
                return True
    return False


def host_own(a):
    # the host defines its own function at this address (a stand-in or an independent host version)
    return any(f in host_files for ident in ident_by_addr.get(a, ()) for f in owner_files.get(ident, ()))


# ---- ABI class of a ready row, from its definition's parameter list
RAW_PARAM = re.compile(r'\bRaw\b|std::uintptr_t|\bvoid\s*\*|std::uint8_t\s*\*|\bchar\s*\*')
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
        return ('raw_host_shaped' if f'{a:08x}' in host_shaped else 'raw_view'), params
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
    if state == 'ready':
        ident = row['name'].split('::')[-1]
        row['abi'], params = abi_class(a, ident, row['source'])
        row['params'] = (params or '')[:120].replace('\n', ' ')
        row['stand_in_hosts'] = sorted(n for n, t in host_text.items() if row['address'] in t)
        abi_counts[row['abi']] += 1
    rows.append(row)

by_cat = collections.defaultdict(collections.Counter)
for r in rows:
    by_cat[r['category']][r['state']] += 1
out = ROOT / 'local/output/wire_readiness.json'
out.parent.mkdir(parents=True, exist_ok=True)
json.dump(dict(summary=dict(counts), ready_abi=dict(abi_counts), by_category={k: dict(v) for k, v in by_cat.items()},
               edges=dict(ghidra=ghidra_edges, rel32_added=scan_edges, address_taken_funcs=len(address_taken)),
               reachable_files=len(reach), roots=len(roots), rows=sorted(rows, key=lambda r: (r['state'], r['category'], r['address']))),
          open(out, 'w'), indent=1)

ready = [r for r in rows if r['state'] == 'ready']
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
print('without an address-named identifier (cannot be judged wired):', no_ident)
for k, v in sorted(by_cat.items(), key=lambda kv: -sum(kv[1].values())):
    print(f'{label.get(k, k):32}', dict(v))
print('wrote', out)
