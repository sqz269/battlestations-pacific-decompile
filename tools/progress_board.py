"""Build the progress board: one self-contained HTML page with the address-band map, growth over
time by category and by harness, commit activity, and per-category / per-segment tables.

    python tools/progress_board.py                 # -> local/progress_board.html
    python tools/progress_board.py --out path.html --json data.json
    python tools/progress_board.py --no-index      # skip the `bsp.py index --if-stale` refresh

Data sources, all local: the lookup index (segments, functions, tags, docs), git history of
config/reconstruction, config/names and src/ (when each address first got a ledger record, an
address-named C++ body, or a reviewed name), and the merge commits of the executable milestones.
Harness follows commit trailers: a Claude trailer means Claude, everything else is Codex.

A function counts as reconstructed when its address has a function record in the reconstruction
ledger or a column-0 C++ definition named after it under src/, dated by whichever came first.
"""
import argparse
import collections
import datetime as dt
import json
import re
import sqlite3
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TEMPLATE = ROOT / 'tools/progress_board_template.html'
INDEX = ROOT / 'local/bsp_index.sqlite'
TARGET = 13987  # inventory point estimate of hand-reconstruction work (reports/library_inventory/SUMMARY.md)
STEP = 2 * 3600

# Category per partition segment, keyed by the segment's start address so a re-partition cannot
# silently relabel a band. Segments whose start is not listed fall back to keyword matching.
CATEGORIES = [
    ('core', 'Core, startup & platform'), ('files', 'Files & VFS'), ('lua', 'Lua & script API'),
    ('scene', 'Scene, terrain & weather'), ('units', 'Units, weapons & damage'), ('ai', 'AI & orders'),
    ('mission', 'Mission, scoring & campaign'), ('gui', 'GUI, menus & HUD'), ('render', 'Rendering, camera & effects'),
    ('sound', 'Sound'), ('net', 'Network & online'),
]
SEGMENT_CATEGORY = {
    'core': [0x00401010, 0x0042a920, 0x0043e8f0, 0x004d0920, 0x0098c630, 0x00bd9220, 0x009668c0, 0x00bd2f10],
    'files': [0x00be4460],
    'lua': [0x004ba290, 0x0088b410, 0x00a62660, 0x00b65ba0],
    'scene': [0x00413d10, 0x00465610, 0x0047c6a0, 0x00491170, 0x004e7dd0, 0x009222e0, 0x00968e80, 0x0097ab70, 0x00ad5590, 0x00ada420, 0x00b999f0, 0x00bb3db0, 0x00bbdec0],
    'units': [0x006beb70, 0x006d4370, 0x006deca0, 0x006fa540, 0x0070bc30, 0x00714060, 0x00728a90, 0x00735f30, 0x00758f90, 0x007a49a0, 0x007b2dd0, 0x007d1e50, 0x0080da00, 0x0081aa60, 0x008288d0, 0x00851e10, 0x008742a0, 0x00878530, 0x008e3de0, 0x0092e0b0, 0x00951f40, 0x009f6090],
    'ai': [0x0051e7e0, 0x0071ba20, 0x008ee670, 0x00996120, 0x009fe130, 0x00a16050],
    'mission': [0x0051a0e0, 0x0061d5c0, 0x00696470, 0x007f8400, 0x008dcda0],
    'gui': [0x004486c0, 0x004f9d80, 0x00527cb0, 0x00544e60, 0x00554680, 0x00568cb0, 0x0056d9a0, 0x005805a0, 0x005c57d0, 0x005cd070, 0x005e09a0, 0x00604bc0, 0x0060cb60, 0x00654a90, 0x00680db0, 0x006d8b00, 0x00701b70, 0x00a9ac70, 0x00aa0f70, 0x00aa6750, 0x00aacf10, 0x00ac97f0],
    'render': [0x00453cc0, 0x00723030, 0x0078d880, 0x00af37d0, 0x00b1bf40, 0x00b25dc0, 0x00b402e0, 0x00b4b490, 0x00b6d3c0, 0x00b80fd0, 0x00b86e40, 0x00b8a2b0],
    'sound': [0x00a7a440],
    'net': [0x0076aa00, 0x007870d0, 0x00a371c0, 0x00a46b80],
}
KEYWORD_FALLBACK = [
    ('net', ('online', 'server', 'client', 'peer', 'matchmaking', 'network', 'send_', 'recv_')),
    ('sound', ('fmod', 'sound', 'stream')), ('files', ('cfilestore', 'directory', 'removefile')),
    ('ai', ('moveto', 'attackmove', 'bot', 'coordinator', 'command', 'follow')),
    ('gui', ('_icon', '_text', 'menu', 'framebox', 'gui', 'scrollbar', 'listbox', 'navigate')),
    ('render', ('mshd', 'mvfm', 'shader', 'emitter', 'particle', 'clight', 'cmesh', 'camera', 'sprite')),
    ('units', ('torpedo', 'damage', 'bullet', 'weapon', 'gun', 'plane', 'ship', 'runway', 'hangar')),
    ('mission', ('scoring', 'objective', 'unlock', 'mission', 'savedata')),
    ('scene', ('scene', 'terrain', 'entity', 'spawn', 'cloud', 'ocean', 'foliage')),
    ('lua', ('lua', 'dofile', 'userdata', 'collectgarbage', 'chunk')),
]
START_CATEGORY = {start: key for key, starts in SEGMENT_CATEGORY.items() for start in starts}
FUNCLET = ('Unwind@', 'Catch_All@', 'thunk_')
ADDR_FIELD = re.compile(r'"address"\s*:\s*"([0-9a-fA-F]{8})"')
LEGACY_KEY = re.compile(r'^\+\s*"([0-9a-fA-F]{8})"\s*:\s*\{')
SIG = re.compile(r"^\+(?P<line>[A-Za-z_][\w:<>,*&\s\[\]]*?\s[*&]*(?P<name>~?[A-Za-z_][\w:]*(?:<[^()]*>)?)\s*\((?P<args>[^;{}]*)"
                 r"(?:\)\s*(?:const)?\s*(?:noexcept(?:\([^)]*\))?)?\s*(?:override|final)?\s*(?:\{.*)?|,)\s*)$")
SUFFIX = re.compile(r'_([0-9a-fA-F]{8})$')
KEYWORDS = {'return', 'if', 'for', 'while', 'switch', 'namespace', 'using', 'template', 'struct', 'class', 'enum', 'typedef',
            'static_assert', 'extern', 'else', 'case', 'default', 'do', 'try', 'catch', 'throw', 'delete', 'new', 'sizeof'}
SEP = '@@COMMIT@@'
FMT = SEP + '%H%x1f%ct%x1f%(trailers)%x1e'


def git(*args, text=True):
    return subprocess.run(['git', *args], cwd=ROOT, capture_output=True).stdout.decode('utf-8', 'replace') if text else None


def harness_of(trailers):
    return 'claude' if re.search(r'Claude-Session:|co-authored-by:.*claude', trailers, re.I) else 'codex'


def commits_with_patches(*paths):
    """Yield (ts, harness, patch_lines) for every non-merge commit on main touching paths, oldest first."""
    raw = git('log', 'main', '--reverse', '--no-merges', '-p', '--no-color', '--format=' + FMT, '--', *paths)
    for chunk in raw.split(SEP)[1:]:
        head, _, body = chunk.partition('\x1e')
        parts = head.split('\x1f')
        if len(parts) < 3:
            continue
        yield int(parts[1]), harness_of(parts[2]), body.splitlines()


def first_ledger_records(kind_now):
    """address -> (ts, harness) for functions and fragments, first time each address entered the ledger."""
    funcs, frags = {}, {}
    for ts, harness, lines in commits_with_patches('config/reconstruction', 'config/reconstruction.json'):
        cur = None
        for line in lines:
            if line.startswith('+++ b/'):
                cur = line[6:]
                continue
            if not cur or not line.startswith('+') or line.startswith('+++'):
                continue
            addr = kind = None
            if cur.endswith('.jsonl'):
                try:
                    rec = json.loads(line[1:])
                except ValueError:
                    continue
                addr, kind = str(rec.get('address', '')).lower(), rec.get('kind')
            elif cur == 'config/reconstruction.json':
                m = LEGACY_KEY.match(line) or ADDR_FIELD.search(line)
                if m:
                    addr, kind = m.group(1).lower(), kind_now.get(m.group(1).lower())
            if not addr:
                continue
            pool = frags if kind == 'fragment' else funcs
            if addr not in funcs and addr not in frags:
                pool[addr] = (ts, harness)
    return funcs, frags


def first_source_bodies(entries):
    """address -> (ts, harness): first column-0 C++ definition named _<address> for a function entry."""
    first = {}
    for ts, harness, lines in commits_with_patches('src'):
        cur = None
        for line in lines:
            if line.startswith('+++ b/'):
                cur = line[6:]
                continue
            if not cur or not cur.endswith('.cpp') or not line.startswith('+') or line.startswith('+++'):
                continue
            body = line[1:]
            if not body or body[0] in ' \t#/}{' or body.rstrip().endswith(';'):
                continue
            tok = re.match(r'[A-Za-z_]\w*', body)
            if not tok or tok.group(0) in KEYWORDS or body.startswith(('BSP_', 'TEST_', 'CATCH_')):
                continue
            m = SIG.match(line)
            if not m:
                continue
            ms = SUFFIX.search(m.group('name').rsplit('::', 1)[-1])
            if ms:
                addr = int(ms.group(1), 16)
                if addr in entries and addr not in first:
                    first[addr] = (ts, harness)
    return first


def first_names():
    first = {}
    for ts, harness, lines in commits_with_patches('config/names', 'config/ghidra_names.json'):
        for line in lines:
            if line.startswith('+') and not line.startswith('+++'):
                m = ADDR_FIELD.search(line)
                if m:
                    addr = int(m.group(1), 16)
                    if addr not in first:
                        first[addr] = (ts, harness)
    return first


def category_for(start, keywords):
    if start in START_CATEGORY:
        return START_CATEGORY[start]
    kw = ' '.join(keywords).lower()
    for key, needles in KEYWORD_FALLBACK:
        if any(n in kw for n in needles):
            return key
    return 'core'


def cumulative(first_map, addr_cat, grid):
    ev = sorted((ts, addr_cat.get(a) or 'other', h) for a, (ts, h) in first_map.items())
    keys = [k for k, _ in CATEGORIES] + ['other']
    out_cat = {k: [] for k in keys}
    out_h = {'codex': [], 'claude': []}
    total, i, n = [], 0, 0
    cc, ch = collections.Counter(), collections.Counter()
    for t in grid:
        while i < len(ev) and ev[i][0] <= t:
            cc[ev[i][1]] += 1
            ch[ev[i][2]] += 1
            n += 1
            i += 1
        for k in keys:
            out_cat[k].append(cc[k])
        for k in out_h:
            out_h[k].append(ch[k])
        total.append(n)
    return {'cat': out_cat, 'harness': out_h, 'total': total}


def build(now=None):
    now = now or int(time.time())
    db = sqlite3.connect(f'file:{INDEX.as_posix()}?mode=ro', uri=True)
    funcs = {}
    for a, name, seg, tag, rk, ln in db.execute('select address, name, segment, tag_category, recon_kind, ledger_name from functions'):
        if not name.startswith(FUNCLET):
            funcs[a] = (seg, tag, rk, ln)
    kind_now = {}
    for addr, kind in db.execute('select printf("%08x", address), kind from recon'):
        if kind == 'function' or addr not in kind_now:
            kind_now[addr] = kind  # a function record wins over a fragment at the same address
    ledger_funcs, ledger_frags = first_ledger_records(kind_now)
    ledger_first = {int(a, 16): v for a, v in ledger_funcs.items()}
    frag_first = {int(a, 16): v[0] for a, v in ledger_frags.items()}
    src_first = first_source_bodies(set(funcs))
    union = dict(ledger_first)
    for a, v in src_first.items():
        if a not in union or v[0] < union[a][0]:
            union[a] = v
    names_first = first_names()

    seg_rows = db.execute('select id, start, end, keywords from segments order by id').fetchall()
    seg_cat, segments, fallback = {}, [], []
    by_seg = collections.defaultdict(list)
    for a, v in funcs.items():
        by_seg[v[0]].append((a, v))
    for sid, st, en, kw in seg_rows:
        keywords = [k.strip() for k in (kw or '').split(',') if k.strip()][:8]
        cat = category_for(st, keywords)
        if st not in START_CATEGORY:
            fallback.append((sid, f'{st:08x}', cat))
        seg_cat[sid] = cat
        rows = by_seg.get(sid, [])
        total = len(rows)
        tagged = sum(1 for a, v in rows if v[1])
        done = sum(1 for a, v in rows if a in union)
        done_pool = sum(1 for a, v in rows if a in union and not v[1])
        pool = total - tagged
        by_h = collections.Counter(union[a][1] for a, v in rows if a in union)
        segments.append(dict(
            id=sid, start=st, end=en, cat=cat, keywords=keywords, total=total, tagged=tagged, pool=pool, done=done,
            remaining=max(pool - done_pool, 0), frag=sum(1 for a, v in rows if v[2] == 'fragment' and a not in union),
            named=sum(1 for a, v in rows if v[3]),
            new24=sum(1 for a, v in rows if a in union and union[a][0] >= now - 86400),
            new72=sum(1 for a, v in rows if a in union and union[a][0] >= now - 3 * 86400),
            codex=by_h['codex'], claude=by_h['claude']))
    if fallback:
        print(f'note: {len(fallback)} segments not in SEGMENT_CATEGORY, categorised by keywords: {fallback[:6]}', file=sys.stderr)

    addr_cat = {a: seg_cat.get(v[0]) for a, v in funcs.items()}
    t0 = min(min(ts for ts, h in union.values()), min(ts for ts, h in names_first.values())) // STEP * STEP
    grid = list(range(t0, now, STEP)) + [now]
    funcs_series = cumulative(union, addr_cat, grid)
    names_series = cumulative(names_first, addr_cat, grid)
    ledger_total = cumulative(ledger_first, addr_cat, grid)['total']
    fe, frag_total, i, n = sorted(frag_first.values()), [], 0, 0
    for t in grid:
        while i < len(fe) and fe[i] <= t:
            n += 1
            i += 1
        frag_total.append(n)

    hourly = collections.Counter(int(l) // 3600 * 3600 for l in git('log', 'main', '--format=%ct').split())
    milestones, seen = [], set()
    for line in git('log', 'main', '--reverse', '--format=%ct %s').splitlines():
        m = re.match(r'(\d+) Merge agent/cc-exe-(2[a-z]\d?)$', line)
        mid = m.group(2) if m else ('1' if line.endswith(' Merge agent/cc-game-exe') else ('2a' if line.endswith(' Merge agent/cc-game-vfs') else None))
        if mid and mid not in seen:
            seen.add(mid)
            milestones.append({'ts': int(line.split()[0]), 'id': mid})

    tags = collections.Counter(v[1] for v in funcs.values() if v[1])
    snapshot = (db.execute("select value from meta where key='built_utc'").fetchone() or ['?'])[0][:16].replace('T', ' ') + ' UTC'
    return dict(
        generated=dt.datetime.fromtimestamp(now, dt.timezone.utc).strftime('%Y-%m-%d %H:%M UTC'), snapshot=snapshot,
        head=git('rev-parse', '--short', 'main').strip(),
        categories=[dict(key=k, label=l, segments=[s['id'] for s in segments if s['cat'] == k]) for k, l in CATEGORIES],
        segments=segments, grid=grid, funcs=funcs_series, names=names_series, ledger_total=ledger_total, frag_total=frag_total,
        commits=sorted(hourly.items()), milestones=milestones, tags=dict(tags.most_common()),
        kpi=dict(functions=len(union), ledger_functions=len(ledger_first), fragments=len(frag_first), names=len(names_first),
                 tags=sum(tags.values()), target=TARGET, pool_remaining=sum(s['remaining'] for s in segments),
                 rate24=sum(1 for ts, h in union.values() if ts >= now - 86400),
                 names24=sum(1 for ts, h in names_first.values() if ts >= now - 86400),
                 codex=sum(1 for ts, h in union.values() if h == 'codex'), claude=sum(1 for ts, h in union.values() if h == 'claude'),
                 internal=db.execute('select count(*) from functions').fetchone()[0]))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--out', default=str(ROOT / 'local/progress_board.html'))
    ap.add_argument('--json', help='also write the data as JSON')
    ap.add_argument('--no-index', action='store_true', help='do not run bsp.py index --if-stale first')
    args = ap.parse_args()
    if not args.no_index:
        subprocess.run([sys.executable, str(ROOT / 'tools/bsp.py'), 'index', '--if-stale'], cwd=ROOT, check=False)
    if not INDEX.exists():
        sys.exit('no index; run: python tools/bsp.py index')
    data = build()
    payload = json.dumps(data, separators=(',', ':')).replace('</', '<\\/')
    html = TEMPLATE.read_text(encoding='utf-8')
    assert '__DATA__' in html
    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(html.replace('__DATA__', payload), encoding='utf-8')
    if args.json:
        Path(args.json).write_text(json.dumps(data, indent=1), encoding='utf-8')
    k = data['kpi']
    print(f"wrote {out} ({out.stat().st_size // 1024} KB): {k['functions']} reconstructed (+{k['rate24']} in 24h), "
          f"{k['names']} named, {k['fragments']} fragments, {len(data['segments'])} segments, {len(data['grid'])} samples, "
          f"milestones through {data['milestones'][-1]['id'] if data['milestones'] else 'none'}")


if __name__ == '__main__':
    main()
