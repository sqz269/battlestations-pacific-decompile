"""Bounded lookups and live queries for the reconstruction workspace.

Everything here prints a short, capped result so a model never has to read a ledger, an
export, or a directory whole. `index` rebuilds local/bsp_index.sqlite (ignored, disposable)
from the snapshot, sharded ledgers, tags, call graph, partition, PE strings and docs.

  python tools/bsp.py state                      one-screen orientation card (start a turn here)
  python tools/bsp.py lookup 00ab9fd0            everything known about one address
  python tools/bsp.py show 00ab9fd0 [--asm] [--lines 80] [--start 0]   capped export excerpt
  python tools/bsp.py range 00ab0000 00ac0000 --only FUN_
  python tools/bsp.py callers|callees|docs-for 00ab9fd0 / segment 12 / find GuiManager
  python tools/bsp.py ghidra count|proto|flow|instruction-context|xrefs|callers|callees|bytes|comments|decompile|disasm|export ...
  python tools/bsp.py ghidra ensure [--status]     start Ghidra when nothing answers (autostart --install: at logon)
  python tools/bsp.py snapshot [--force]         snapshot + index only if Ghidra's function count changed
  python tools/bsp.py index [--if-stale]
  python tools/bsp.py ledger add-name|add-function|add-fragment|migrate ...
"""
import argparse
import collections
import contextlib
import hashlib
import io
import json
import os
import re
import sqlite3
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import branch_sync  # noqa: E402
import coordination  # noqa: E402
import ledger  # noqa: E402
import workspace  # noqa: E402
import worktree_gc  # noqa: E402

ROOT = ledger.ROOT
DB = ROOT / 'local/bsp_index.sqlite'
EXPORTS = workspace.exports_dir()  # main checkout's exports/bsp, shared by every worktree
ADDR = re.compile(r'\b(00[4-9a-c][0-9a-f]{5})\b', re.IGNORECASE)  # docs spell addresses in either case

DEFAULT_OUTPUT_TOKENS = 2000  # measured: the median read, carried ~45x, is what costs money, not the outliers


@contextlib.contextmanager
def output_budget(argv=(), full=False):
    """Cap what one invocation prints. Anything over the budget is written to local/output/ whole and
    replaced by its head plus a marker naming the exact command that reads the rest.

    Reasoning from a silently truncated body is the expensive failure, so the marker is never omitted.
    Escape hatches: `--full`, or BSP_OUTPUT_BUDGET=<tokens> (0 disables the cap).
    """
    try:
        budget = int(os.environ.get('BSP_OUTPUT_BUDGET', DEFAULT_OUTPUT_TOKENS))
    except ValueError:
        budget = DEFAULT_OUTPUT_TOKENS
    if full or budget <= 0:
        yield
        return
    limit = budget * 4  # ~4 bytes per token, measured at 3.99 on this corpus
    buf = io.StringIO()
    real = sys.stdout
    sys.stdout = buf
    try:
        yield
    finally:
        sys.stdout = real
        text = buf.getvalue()
        if len(text) <= limit:
            real.write(text)
        else:
            slug = re.sub(r'[^a-z0-9]+', '-', ' '.join(str(a) for a in argv).lower()).strip('-')[:60] or 'output'
            path = ROOT / 'local/output' / f"{slug}-{datetime.now().strftime('%H%M%S')}.txt"
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding='utf-8')
            head, shown = [], 0
            for line in text.splitlines(True):
                if shown + len(line) > limit:
                    break
                head.append(line)
                shown += len(line)
            rest = len(text.splitlines()) - len(head)
            real.write(''.join(head))
            if not head or not head[-1].endswith('\n'):
                real.write('\n')
            rel = path.relative_to(ROOT).as_posix()
            real.write(f"[bsp: TRUNCATED at {budget} tokens. {rest} more lines, {len(text) - shown} more bytes.\n"
                       f" Whole output: {rel}  (ignored by git)\n"
                       f" Read the rest with:  rg -n '<term>' {rel}   or   sed -n '{len(head) + 1},{len(head) + 80}p' {rel}\n"
                       f" Re-run uncapped with --full, or BSP_OUTPUT_BUDGET=0. Do not conclude from the head alone.]\n")


SCHEMA = """
CREATE TABLE meta(key TEXT PRIMARY KEY, value TEXT);
CREATE TABLE functions(address INTEGER PRIMARY KEY, hex TEXT, name TEXT, thunk INTEGER, exported INTEGER,
  segment INTEGER, tag_category TEXT, tag_name TEXT, tag_confidence TEXT, tag_action TEXT,
  ledger_name TEXT, ledger_evidence TEXT, recon_kind TEXT, recon_name TEXT, recon_status TEXT, recon_source TEXT);
CREATE INDEX functions_name ON functions(name);
CREATE INDEX functions_segment ON functions(segment);
CREATE TABLE recon(address INTEGER, kind TEXT, name TEXT, status TEXT, source TEXT);
CREATE INDEX recon_address ON recon(address);
CREATE TABLE calls(caller INTEGER, callee INTEGER);
CREATE INDEX calls_caller ON calls(caller);
CREATE INDEX calls_callee ON calls(callee);
CREATE TABLE edges(caller INTEGER, callee INTEGER, site INTEGER, kind TEXT, detail TEXT);
CREATE INDEX edges_caller ON edges(caller);
CREATE INDEX edges_callee ON edges(callee);
CREATE TABLE funclet_parent(funclet INTEGER PRIMARY KEY, parent INTEGER);
CREATE TABLE ranges(func INTEGER PRIMARY KEY, stop INTEGER);
CREATE TABLE undefined(address INTEGER PRIMARY KEY, stop INTEGER);
CREATE TABLE vtables(address INTEGER PRIMARY KEY, class TEXT, slots INTEGER);
CREATE TABLE datarefs(func INTEGER, ref INTEGER, text TEXT);
CREATE INDEX datarefs_func ON datarefs(func);
CREATE TABLE segments(id INTEGER PRIMARY KEY, start INTEGER, end INTEGER, candidates INTEGER, purity REAL,
  wave INTEGER, lua_bindings INTEGER, vtables INTEGER, keywords TEXT);
CREATE TABLE docs(path TEXT PRIMARY KEY, title TEXT);
CREATE TABLE doc_addresses(path TEXT, address INTEGER);
CREATE INDEX doc_addresses_address ON doc_addresses(address);
"""


def h(address):
    return f'{address:08x}'


def norm(text):
    return f'{int(text, 16):08x}'


def rel(path):
    """Path for display: relative to this checkout, else to the main checkout (shared exports), else absolute."""
    for base in (ROOT, workspace.main_root()):
        try:
            return Path(path).relative_to(base).as_posix()
        except ValueError:
            continue
    return Path(path).as_posix()


def cap(text, lines, start=0, label='lines'):
    """Print at most `lines` lines starting at `start`; say how much was left out."""
    rows = text.splitlines()
    for row in rows[start:start + lines]:
        print(row)
    left = len(rows) - (start + lines)
    if left > 0:
        print(f'... {left} more {label} of {len(rows)} (use --start {start + lines} or --lines)')


# ---------------------------------------------------------------- index build

def load_strings(binary):
    try:
        import pefile
    except ImportError:
        return {}
    if not Path(binary).exists():
        return {}
    pe = pefile.PE(str(binary), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    strings = {}
    for section in pe.sections:
        if section.Name.rstrip(b'\0') in (b'.rdata', b'.data'):
            data = section.get_data()
            va = base + section.VirtualAddress
            for match in re.finditer(rb'[\x20-\x7e]{3,}\x00', data):
                strings[va + match.start()] = match.group()[:-1].decode('ascii', errors='ignore')
    return strings


def inputs_digest():
    """Digest of everything the index is built from, to detect staleness cheaply."""
    digest = hashlib.sha256()
    for path in [EXPORTS / 'functions.json', EXPORTS / 'callgraph.json', EXPORTS / 'datarefs.json',
                 EXPORTS / 'callgraph_edges.json', EXPORTS / 'callgraph_indirect.json',
                 ROOT / 'reports/library_inventory/candidate_partition.json']:
        if path.exists():
            digest.update(path.name.encode())
            digest.update(path.read_bytes())
    for directory in (ledger.NAMES_DIR, ledger.RECON_DIR, ledger.TAGS_DIR):
        for path in sorted(directory.glob('*.json*')) if directory.exists() else []:
            digest.update(path.name.encode())
            digest.update(path.read_bytes())
    for legacy in (ledger.LEGACY_NAMES, ledger.LEGACY_RECON, ledger.LEGACY_TAGS):
        if legacy.exists():
            digest.update(legacy.read_bytes())
    for doc in sorted((ROOT / 'docs').glob('*.md')):
        stat = doc.stat()
        digest.update(f'{doc.name}:{stat.st_size}:{int(stat.st_mtime)}'.encode())
    return digest.hexdigest()


def index_meta():
    if not DB.exists():
        return {}
    db = None
    try:
        db = sqlite3.connect(DB)
        return dict(db.execute('SELECT key, value FROM meta').fetchall())
    except sqlite3.Error:
        return {}
    finally:
        if db is not None:
            db.close()  # an open handle would block the rebuild's unlink on Windows


def build_index(args):
    if getattr(args, 'if_stale', False) and index_meta().get('inputs_sha256') == inputs_digest():
        print('index is fresh')
        return
    DB.parent.mkdir(parents=True, exist_ok=True)
    functions_path = EXPORTS / 'functions.json'
    if not functions_path.exists():
        # keep the old index: an empty one makes every later lookup and re-export see nothing
        sys.exit(f'No {functions_path}; the shared exports look wiped. Run: python tools/bsp.py exports restore')
    previous_exported = index_exported_count()
    building = DB.with_name(f'{DB.stem}.{os.getpid()}.building')
    building.unlink(missing_ok=True)
    db = sqlite3.connect(building)
    db.executescript(SCHEMA)
    functions = json.loads(functions_path.read_text(encoding='utf-8'))
    exported = {p.name for p in (EXPORTS / 'functions').glob('*') if (p / 'decompiled.c').exists()}
    if previous_exported and len(exported) < previous_exported // 2:
        print(f'WARNING: only {len(exported)} exported functions on disk, the last index saw {previous_exported}; '
              'the shared exports may have been emptied. Run: python tools/bsp.py exports check')
    rows = {}
    for f in functions:
        if f.get('isExternal'):
            continue
        a = int(f['address'], 16)
        rows[a] = [a, h(a), f['name'], int(bool(f.get('isThunk'))), int(h(a) in exported),
                   None, None, None, None, None, None, None, None, None, None, None]
    for t in ledger.load_tags():
        a = int(t['address'], 16)
        if a in rows:
            rows[a][6:10] = [t['category'], t.get('name') or None, t.get('confidence'), t.get('action')]
    for n in ledger.load_names():
        a = int(n['address'], 16)
        if a in rows:
            rows[a][10:12] = [n['name'], n.get('evidence', '')]
    recon = ledger.load_reconstruction()
    for kind in ('function', 'fragment'):
        for r in recon['fragments' if kind == 'fragment' else 'functions']:
            a = int(r['address'], 16)
            db.execute('INSERT INTO recon VALUES (?,?,?,?,?)', (a, kind, r.get('name'), r.get('status'), r.get('source')))
            if a in rows and (rows[a][12] is None or kind == 'function'):
                rows[a][12:16] = [kind, r.get('name'), r.get('status'), r.get('source')]
    partition = ROOT / 'reports/library_inventory/candidate_partition.json'
    if partition.exists():
        for s in json.loads(partition.read_text(encoding='utf-8'))['segments']:
            start, end = int(s['start'], 16), int(s['end'], 16)
            db.execute('INSERT INTO segments VALUES (?,?,?,?,?,?,?,?,?)',
                       (s['segment'], start, end, s['candidates'], s.get('purity'), s.get('wave'),
                        s.get('lua_bindings'), s.get('vtables'), ', '.join(s.get('keywords', [])[:8])))
            for a in rows:
                if start <= a <= end:
                    rows[a][5] = s['segment']
    db.executemany('INSERT INTO functions VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)', rows.values())
    callgraph = EXPORTS / 'callgraph.json'
    edges_path = EXPORTS / 'callgraph_edges.json'
    if (not callgraph.exists() or callgraph.stat().st_mtime < functions_path.stat().st_mtime
            or not edges_path.exists() or edges_path.stat().st_mtime < functions_path.stat().st_mtime):
        # Without the sweep, lookup/callers/callees are silently empty; it takes about two seconds.
        print('call graph missing or older than the snapshot: running tools/callgraph_sweep.py')
        subprocess.run([sys.executable, str(ROOT / 'tools/callgraph_sweep.py'), '--output', str(EXPORTS)],
                       cwd=ROOT, capture_output=True, text=True)
    if callgraph.exists():
        edges = [(int(k, 16), int(v, 16)) for k, vs in json.loads(callgraph.read_text()).items() for v in vs]
        db.executemany('INSERT INTO calls VALUES (?,?)', edges)
    # Per-site edges with kinds (tools/callgraph_sweep.py), plus vtable/Lua edges when
    # tools/callgraph_indirect.py has run. `calls` above stays the unique direct-edge table older readers expect.
    if edges_path.exists():
        swept = json.loads(edges_path.read_text())
        db.executemany('INSERT INTO edges VALUES (?,?,?,?,NULL)', swept['edges'])
        db.executemany('INSERT INTO funclet_parent VALUES (?,?)', [(int(k), v) for k, v in swept['funclet_parent'].items()])
        db.executemany('INSERT INTO ranges VALUES (?,?)', [(int(k), v) for k, v in swept.get('ranges', {}).items()])
        db.executemany('INSERT INTO undefined VALUES (?,?)', [(int(k), v) for k, v in swept.get('undefined', {}).items()])
    indirect_path = EXPORTS / 'callgraph_indirect.json'
    if indirect_path.exists():
        extra = json.loads(indirect_path.read_text())
        num = lambda v: int(v, 16) if isinstance(v, str) else v
        db.executemany('INSERT INTO edges VALUES (?,?,?,?,?)',
                       [(num(e.get('caller')), num(e['callee']), num(e.get('site')), e['kind'], e.get('detail'))
                        for e in extra.get('edges', [])])
        db.executemany('INSERT OR REPLACE INTO vtables VALUES (?,?,?)',
                       [(num(v['address']), v.get('class'), len(v.get('slots', []))) for v in extra.get('vtables', [])])
    datarefs = EXPORTS / 'datarefs.json'
    if datarefs.exists():
        config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
        strings = load_strings(config['binary'])
        refs = [(int(k, 16), int(v, 16), strings.get(int(v, 16)))
                for k, vs in json.loads(datarefs.read_text()).items() for v in vs]
        db.executemany('INSERT INTO datarefs VALUES (?,?,?)', refs)
    for doc in sorted((ROOT / 'docs').glob('*.md')):
        text = doc.read_text(encoding='utf-8', errors='ignore')
        title = next((line.lstrip('# ').strip() for line in text.splitlines() if line.startswith('#')), doc.stem)
        rel = doc.relative_to(ROOT).as_posix()
        db.execute('INSERT INTO docs VALUES (?,?)', (rel, title))
        db.executemany('INSERT INTO doc_addresses VALUES (?,?)',
                       [(rel, int(a, 16)) for a in sorted({m.lower() for m in ADDR.findall(text)})])
    db.execute('INSERT INTO meta VALUES (?,?)', ('built_utc', datetime.now(timezone.utc).isoformat()))
    db.execute('INSERT INTO meta VALUES (?,?)', ('functions_sha256', hashlib.sha256(functions_path.read_bytes()).hexdigest()))
    db.execute('INSERT INTO meta VALUES (?,?)', ('inputs_sha256', inputs_digest()))
    db.commit()
    counts = {t: db.execute(f'SELECT COUNT(*) FROM {t}').fetchone()[0]
              for t in ('functions', 'calls', 'edges', 'datarefs', 'segments', 'docs', 'doc_addresses')}
    db.close()
    # publish atomically; a reader holding the old file open (WinError 32) only delays the swap
    for attempt in range(20):
        try:
            os.replace(building, DB)
            break
        except PermissionError:
            if attempt == 19:
                sys.exit(f'index built at {building} but {DB} is held open by another process; '
                         'close it and rerun (the old index is untouched)')
            import time
            time.sleep(0.5)
    print(f'built {DB.relative_to(ROOT).as_posix()}: {counts}')


def index_exported_count():
    """Exported-function count recorded by the current index, or 0 when there is none."""
    if not DB.exists():
        return 0
    db = None
    try:
        db = sqlite3.connect(f'file:{DB.as_posix()}?mode=ro', uri=True)
        return db.execute('SELECT COUNT(*) FROM functions WHERE exported=1').fetchone()[0]
    except sqlite3.Error:
        return 0
    finally:
        if db is not None:
            db.close()


# ---------------------------------------------------------------- index queries

def connect(required=True):
    # An empty file is what a plain sqlite3.connect() on a missing path leaves behind (fresh worktrees);
    # treat it, and a file without the functions table, as no index at all.
    usable = DB.exists() and DB.stat().st_size > 0
    if usable:
        db = sqlite3.connect(DB)
        if db.execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name='functions'").fetchone():
            db.row_factory = sqlite3.Row
            return db
        db.close()
    if required:
        sys.exit(f'No usable index at {DB}; run: python tools/bsp.py index')
    return None


def fn_label(db, address):
    row = db.execute('SELECT name FROM functions WHERE address=?', (address,)).fetchone() if db else None
    if row is None and db and db.execute("SELECT 1 FROM sqlite_master WHERE name='undefined'").fetchone() and \
            db.execute('SELECT 1 FROM undefined WHERE address=?', (address,)).fetchone():
        return f'{h(address)} (undefined code)'
    return f"{h(address)} {row['name'] if row else '?'}"


def annotate(db, text):
    """Append the indexed function name after each known address unless the line already shows it."""
    if db is None:
        return text
    out = []
    for line in text.splitlines():
        def repl(match):
            row = db.execute('SELECT name FROM functions WHERE address=?', (int(match.group(1), 16),)).fetchone()
            if row is None or row['name'] in line:
                return match.group(1)
            return f"{match.group(1)} {row['name']}"
        out.append(ADDR.sub(repl, line))
    return '\n'.join(out)


def header_lines(db, a, width=300):
    """Compact header for one function; shared by lookup and show."""
    f = db.execute('SELECT * FROM functions WHERE address=?', (a,)).fetchone()
    if f is None:
        near = db.execute('SELECT address, name FROM functions WHERE address<=? ORDER BY address DESC LIMIT 1', (a,)).fetchone()
        return None, [f'{h(a)}: no function starts here' + (f"; enclosing candidate {h(near['address'])} {near['name']}" if near else '')]
    lines = [f"{f['hex']}  {f['name']}  thunk={'yes' if f['thunk'] else 'no'}  "
             f"export={'exports/bsp/functions/' + f['hex'] if f['exported'] else 'none'}"]
    if f['segment'] is not None:
        s = db.execute('SELECT * FROM segments WHERE id=?', (f['segment'],)).fetchone()
        lines.append(f"segment {s['id']} [{h(s['start'])}-{h(s['end'])}] {s['candidates']} candidates wave {s['wave']}  keywords: {s['keywords']}")
    if f['tag_category']:
        lines.append(f"tag: {f['tag_category']} ({f['tag_confidence']}, {f['tag_action']}) {f['tag_name'] or ''}")
    if f['ledger_name']:
        lines.append(f"ledger name: {f['ledger_name']} - {f['ledger_evidence'][:width]}")
    for r in db.execute('SELECT * FROM recon WHERE address=? ORDER BY kind, name', (a,)):
        lines.append(f"reconstruction: {r['kind']} {r['name']} status={r['status']} source={r['source']}")
    return f, lines


def lookup(args):
    db = connect()
    a = int(args.address, 16)
    f, lines = header_lines(db, a, args.width)
    if f is None and (inside := containing(db, a)) is not None:
        chunk = db.execute('SELECT stop FROM undefined WHERE address=?', (inside,)).fetchone()
        if chunk:
            print(f'{h(a)} is in undefined code {h(inside)}-{h(chunk[0])} ({chunk[0] - inside} bytes): '
                  f'not a Ghidra function; the call sweep found it between snapshot functions ({lines[0].split("; ")[-1]})')
            for direction in ('callers', 'callees'):
                rows = group_edges(neighbours(db, inside, direction, EDGE_KINDS, fold=False), False)
                print(f'{direction} ({len(rows)}): ' + ', '.join(
                    (fn_label(db, o) if o is not None else 'data') + f' {k}' for o, k, s, d in rows[:args.limit]))
            return
        print(f'{h(a)} is inside {fn_label(db, inside)} (+{a - inside:#x})')
        a = inside
        f, lines = header_lines(db, a, args.width)
    print('\n'.join(lines))
    if f is None:
        return
    callers = sorted({r[0] for r in neighbours(db, a, 'callers', ('call', 'tailjmp')) if r[0] is not None})
    callees = sorted({r[0] for r in neighbours(db, a, 'callees', ('call', 'tailjmp'))})
    print(f"callers ({len(callers)}): " + ', '.join(fn_label(db, c) for c in callers[:args.limit]) + (' ...' if len(callers) > args.limit else ''))
    print(f"callees ({len(callees)}): " + ', '.join(fn_label(db, c) for c in callees[:args.limit]) + (' ...' if len(callees) > args.limit else ''))
    if has_edges(db):
        other = collections.Counter(r[1] for r in neighbours(db, a, 'callers', EDGE_KINDS[2:]))
        if other:
            print('also referenced: ' + ', '.join(f'{k} {n}' for k, n in other.items()) + f' (bsp.py callers {h(a)} lists them)')
        p = db.execute('SELECT parent FROM funclet_parent WHERE funclet=?', (a,)).fetchone()
        if p:
            print(f'EH funclet of {fn_label(db, p[0])}')
    strings = [r['text'] for r in db.execute('SELECT text FROM datarefs WHERE func=? AND text IS NOT NULL ORDER BY ref', (a,))]
    if strings:
        print(f"string refs ({len(strings)}, immediates in the body, not necessarily call arguments): "
              + ' | '.join(s[:60] for s in strings[:args.limit]) + (' ...' if len(strings) > args.limit else ''))
    docs = db.execute('SELECT d.path, d.title FROM doc_addresses da JOIN docs d ON d.path=da.path WHERE da.address=? ORDER BY d.path', (a,)).fetchall()
    if docs:
        print(f"docs ({len(docs)}): " + '; '.join(f"{d['path']} ({d['title'][:50]})" for d in docs[:args.limit]))


def mark_asm_gaps(text):
    """Insert a marker where consecutive listing addresses skip bytes Ghidra never disassembled."""
    rows = text.splitlines()
    listed = [(i, int(m.group(1), 16)) for i, line in enumerate(rows) if (m := re.match(r'^([0-9a-f]{8}):', line))]
    if len(listed) < 2:
        return text
    try:
        import pefile
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        return text
    config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    if not Path(config['binary']).exists():
        return text
    pe = pefile.PE(config['binary'], fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    section = next((s for s in pe.sections if s.Name.rstrip(b'\0') == b'.text'), None)
    if section is None:
        return text
    text_lo, code = base + section.VirtualAddress, section.get_data()
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    inserts = []
    for (_, a), (j, b) in zip(listed, listed[1:]):
        chunk = code[a - text_lo:a - text_lo + 16]
        ins = next(md.disasm_lite(chunk, a), None)
        end = a + (ins[1] if ins else 1)
        if b > end:
            inserts.append((j, f'; [gap {end:08x}..{b - 1:08x}: {b - end} bytes not disassembled by Ghidra; check with bsp.py ghidra bytes {end:08x}]'))
    for j, marker in reversed(inserts):
        rows.insert(j, marker)
    return '\n'.join(rows)


def resolve_start(value, asm_text):
    """--start is a line count, or (with --asm) a hex instruction address at or above the image base."""
    v = str(value).strip().lower()
    looks_hex = v.startswith('0x') or (len(v) == 8 and all(c in '0123456789abcdef' for c in v))
    n = int(v, 16) if looks_hex else int(v, 0)
    if looks_hex and n >= 0x400000:
        if asm_text is None:
            raise SystemExit(f'--start {value}: an address start needs --asm; without it --start counts lines')
        key = f'{n:08x}'
        for i, line in enumerate(asm_text.splitlines()):
            if line.strip().lower().startswith(key):
                return i
        raise SystemExit(f'--start {value}: no listing line starts at that address')
    return n


def show(args):
    """Capped excerpt of the exported pseudocode (default) or assembly for one function."""
    a = int(args.address, 16)
    db = connect(required=False)
    if db:
        _, lines = header_lines(db, a)
        print('\n'.join(lines))
    name = 'assembly.txt' if args.asm else 'decompiled.c'
    path = EXPORTS / 'functions' / h(a) / name
    if path.exists():
        text = path.read_text(encoding='utf-8', errors='ignore')
        print(f'--- {rel(path)} ---')
    elif args.live:
        text = str(client().get('disassemble_function' if args.asm else 'decompile_function', address=h(a)))
        print(f'--- live {"disassembly" if args.asm else "decompile"} (not exported) ---')
    else:
        print(f'not exported; run: python tools/bsp.py ghidra export {h(a)}   (or --live for an unsaved view)')
        return
    if args.asm:
        text = mark_asm_gaps(text)
    start = resolve_start(args.start, text if args.asm else None)
    if not args.asm and start == 0:
        # A long evidence comment at the top of an export would otherwise eat the whole line budget:
        # print it capped separately so --lines always yields code.
        m = re.match(r'\s*(/\*.*?\*/\s*)+', text, re.S)
        if m and m.end() < len(text):
            comment, text = text[:m.end()], text[m.end():]
            rows = comment.strip().splitlines()
            print('\n'.join(rows[:10]) + (f'\n   ... {len(rows) - 10} more comment lines (see the export file)' if len(rows) > 10 else ''))
    cap(text, args.lines, start)
    if db and not args.asm and start == 0:
        callees = [r[0] for r in db.execute('SELECT callee FROM calls WHERE caller=? ORDER BY callee', (a,))]
        if callees:
            print(f"callees ({len(callees)}): " + ', '.join(fn_label(db, c) for c in callees[:12]) + (' ...' if len(callees) > 12 else ''))


def reconstruction_summary(row):
    if row['recon_kind']:
        return f"{row['recon_kind']}:{row['recon_status'] or 'status_unspecified'}"
    return f"tag:{row['tag_category']}" if row['tag_category'] else ''


def range_query(args):
    db = connect()
    lo, hi = int(args.start, 16), int(args.end, 16)
    query = 'SELECT * FROM functions WHERE address>=? AND address<? '
    params = [lo, hi]
    if args.only:
        query += 'AND name LIKE ? '
        params.append(args.only + '%')
    total = db.execute(query.replace('SELECT *', 'SELECT COUNT(*)'), params).fetchone()[0]
    for f in db.execute(query + 'ORDER BY address LIMIT ?', params + [args.limit]):
        extra = reconstruction_summary(f)
        print(f"{f['hex']} {f['name']:<44} seg={f['segment'] if f['segment'] is not None else '-':<4} {extra}")
    if total > args.limit:
        print(f'... {total - args.limit} more (raise --limit)')


EDGE_KINDS = ('call', 'tailjmp', 'addr_taken', 'data_ptr', 'eh', 'vslot', 'vcall?', 'lua')


def has_edges(db):
    return db.execute("SELECT 1 FROM sqlite_master WHERE name='edges'").fetchone() is not None and \
        db.execute('SELECT 1 FROM edges LIMIT 1').fetchone() is not None


def containing(db, a):
    """The function whose swept body holds `a` (its start when `a` is one), else None."""
    if db.execute('SELECT 1 FROM functions WHERE address=?', (a,)).fetchone():
        return a
    near = db.execute('SELECT address FROM functions WHERE address<=? ORDER BY address DESC LIMIT 1', (a,)).fetchone()
    if not near:
        return None
    if not db.execute("SELECT 1 FROM sqlite_master WHERE name='undefined'").fetchone():
        return near[0]  # an index built before the per-site sweep
    stop = db.execute("SELECT stop FROM ranges WHERE func=?", (near[0],)).fetchone()
    if stop is None or a < stop[0]:
        return near[0]
    # code between snapshot functions that Ghidra never defined (tools/callgraph_sweep.py lists the chunks)
    chunk = db.execute('SELECT address, stop FROM undefined WHERE address<=? ORDER BY address DESC LIMIT 1', (a,)).fetchone()
    return chunk[0] if chunk and a < chunk[1] else None


def resolve_address(db, text):
    """Hex address -> function start, saying so when a mid-function address was moved to its container."""
    a = int(text, 16)
    f = containing(db, a)
    if f is None:
        sys.exit(f'{h(a)} is not inside any swept function body (padding, data, or past the int3 pad); try lookup')
    if f != a:
        label = fn_label(db, f) if db.execute('SELECT 1 FROM functions WHERE address=?', (f,)).fetchone() else \
            f'{h(f)} (undefined code, not a Ghidra function)'
        print(f'{h(a)} is inside {label} (+{a - f:#x}); using that')
    return f


def undefined_query(args):
    """Code between snapshot functions that Ghidra never defined, which something references."""
    db = connect()
    rows = db.execute("""SELECT u.address, u.stop, COUNT(e.callee) AS refs,
                                SUM(e.kind IN ('call','tailjmp')) AS direct, SUM(e.kind='data_ptr') AS data
                         FROM undefined u JOIN edges e ON e.callee=u.address
                         GROUP BY u.address ORDER BY u.address""").fetchall()
    total = db.execute('SELECT COUNT(*) FROM undefined').fetchone()[0]
    rows = [r for r in rows if not args.direct or r['direct']]
    print(f'{len(rows)} referenced undefined-code chunks ({total} chunks in all; the rest nothing references):')
    for r in rows[:args.limit]:
        near = db.execute('SELECT address, name, segment FROM functions WHERE address<? ORDER BY address DESC LIMIT 1', (r['address'],)).fetchone()
        callers = [fn_label(db, c[0]) for c in db.execute("SELECT DISTINCT caller FROM edges WHERE callee=? AND kind IN ('call','tailjmp') AND caller IS NOT NULL LIMIT 2", (r['address'],))]
        print(f"  {h(r['address'])} {r['stop'] - r['address']:>5} bytes seg={near['segment'] if near and near['segment'] is not None else '-'} "
              f"refs={r['refs']} (direct {r['direct']}, data {r['data']}) after {near['name'] if near else '?'}"
              + (f"; called by {', '.join(callers)}" if callers else ''))
    if len(rows) > args.limit:
        print(f'  ... {len(rows) - args.limit} more (raise --limit)')


def wired_set():
    """Function starts tools/wire_readiness.py last classified as wired (empty when it has not run here)."""
    path = ROOT / 'local/output/wire_readiness.json'
    if not path.exists():
        return None
    return {int(r['address'], 16) for r in json.loads(path.read_text(encoding='utf-8')).get('rows', [])
            if r.get('state') == 'wired'}


def neighbours(db, a, direction, kinds, fold=True):
    """[(other, kind, site, detail)] edges of one function. With fold, EH funclets count as their parent:
    a funclet caller is reported as the parent function, and a function's callees include its funclets'."""
    if not has_edges(db):
        col, other = ('callee', 'caller') if direction == 'callers' else ('caller', 'callee')
        return [(r[0], 'call', None, None) for r in db.execute(f'SELECT {other} FROM calls WHERE {col}=?', (a,))]
    marks = ','.join('?' * len(kinds))
    out = []
    if direction == 'callers':
        for c, k, s, d in db.execute(f'SELECT caller, kind, site, detail FROM edges WHERE callee=? AND kind IN ({marks})', (a, *kinds)):
            if fold and c is not None:
                p = db.execute('SELECT parent FROM funclet_parent WHERE funclet=?', (c,)).fetchone()
                if p and k != 'eh':
                    d, c = f'via funclet {h(c)}', p[0]
            out.append((c, k, s, d))
    else:
        sources = [a]
        if fold:
            sources += [r[0] for r in db.execute('SELECT funclet FROM funclet_parent WHERE parent=?', (a,))]
        for src in sources:
            for e, k, s, d in db.execute(f'SELECT callee, kind, site, detail FROM edges WHERE caller=? AND kind IN ({marks})', (src, *kinds)):
                if src != a:
                    d = f'via funclet {h(src)}'
                if fold and k == 'eh':
                    continue  # the funclets themselves are folded in, not listed
                out.append((e, k, s, d))
    return out


def group_edges(rows, by_site):
    """Collapse per-site rows to one per (other, kind): [(other, kind, [sites], [details])]. Data pointers are
    one row in total, since each is an unattributed table slot (vslot/lua rows name the tables they know)."""
    groups = {}
    for other, kind, site, detail in rows:
        key = (other, kind, site) if by_site else ((None, kind) if other is None else (other, kind))
        g = groups.setdefault(key, (other, kind, [], []))
        if site is not None:
            g[2].append(site)
        if detail and detail not in g[3]:
            g[3].append(detail)
    return list(groups.values())


def edge_row(db, other, kind, sites, details, wired):
    where = (' @' + ','.join(h(s) for s in sorted(sites)[:4]) + (f' +{len(sites) - 4}' if len(sites) > 4 else '')) if sites else ''
    extra = f" [{'; '.join(details[:3])}{' ...' if len(details) > 3 else ''}]" if details else ''
    if other is None:
        return f"  {len(sites)} data slot(s){'':<30} {kind}{where}"
    f = db.execute('SELECT * FROM functions WHERE address=?', (other,)).fetchone()
    if f is None:
        v = db.execute('SELECT class FROM vtables WHERE address=?', (other,)).fetchone()
        u = db.execute('SELECT stop FROM undefined WHERE address=?', (other,)).fetchone()
        label = (f"vtable {v[0] or '(no RTTI)'}" if v else
                 f'undefined code, not a Ghidra function ({u[0] - other} bytes)' if u else '?')
        return f"  {h(other)} {label:<44} seg=- {kind}{where}{extra}"
    status = reconstruction_summary(f)
    if wired is not None:
        status += ' wired' if other in wired else ''
    count = f' x{len(sites)}' if len(sites) > 1 else ''
    return (f"  {h(other)} {f['name']:<44} seg={f['segment'] if f['segment'] is not None else '-'} "
            f"{kind}{count}{where}{extra} {status}")


def calls_query(args, direction):
    db = connect()
    a = resolve_address(db, args.address)
    kinds = tuple(args.kind.split(',')) if args.kind else EDGE_KINDS
    bad = [k for k in kinds if k not in EDGE_KINDS]
    if bad:
        sys.exit(f'unknown --kind {bad}; kinds: {", ".join(EDGE_KINDS)}')
    wired = wired_set() if (args.unwired or args.status) else None
    if args.unwired and wired is None:
        sys.exit('--unwired needs local/output/wire_readiness.json: run python tools/wire_readiness.py first')

    def keep(other):
        if other is None:
            return not (args.unbuilt or args.unwired)
        f = db.execute('SELECT recon_kind FROM functions WHERE address=?', (other,)).fetchone()
        if args.unbuilt and f and f[0]:
            return False
        return not (args.unwired and other in wired)

    if args.path_from_host:
        return host_path(db, a, kinds, args.limit)
    printed, seen, frontier = 0, {a}, [a]
    for depth in range(1, max(1, args.depth) + 1):
        level = []
        for node in frontier:
            rows = neighbours(db, node, direction, kinds)
            shown = sorted(group_edges([r for r in rows if keep(r[0])], args.sites),
                           key=lambda g: (EDGE_KINDS.index(g[1]), g[0] or 0, min(g[2]) if g[2] else 0))
            head = f'{direction} of {fn_label(db, node)}'
            if depth == 1:
                counts = collections.Counter(r[1] for r in rows)
                print(f"{head} ({len(rows)} sites: {', '.join(f'{k} {n}' for k, n in sorted(counts.items(), key=lambda kv: EDGE_KINDS.index(kv[0])))};"
                      f" {len(shown)} rows{' after filters' if args.unbuilt or args.unwired else ''}):")
            else:
                print(f'{"  " * (depth - 1)}{head} (depth {depth}, {len(shown)}):')
            for r in shown:
                if printed >= args.limit:
                    print(f'  ... more (raise --limit)')
                    return
                print('  ' * (depth - 1) + edge_row(db, *r, wired))
                printed += 1
                if r[0] is not None and r[0] not in seen and r[1] in ('call', 'tailjmp', 'vcall?', 'lua', 'addr_taken'):
                    seen.add(r[0]); level.append(r[0])
        frontier = level
        if not frontier:
            break


def host_path(db, target, kinds, limit):
    """Shortest caller chains from a wired function (tools/wire_readiness.py) to `target`, walking callers."""
    wired = wired_set()
    if wired is None:
        sys.exit('--path-from-host needs local/output/wire_readiness.json: run python tools/wire_readiness.py first')
    if target in wired:
        print(f'{fn_label(db, target)} is itself wired')
        return
    walk = tuple(k for k in kinds if k in ('call', 'tailjmp', 'vcall?', 'lua', 'addr_taken'))
    parent, frontier, found = {target: None}, [target], []
    for _ in range(12):
        nxt = []
        for node in frontier:
            for c, k, s, d in neighbours(db, node, 'callers', walk):
                if c is None or c in parent:
                    continue
                parent[c] = (node, k)
                if c in wired:
                    found.append(c)
                else:
                    nxt.append(c)
        if found or not nxt:
            break
        frontier = nxt
    if not found:
        print(f'no caller chain within 12 levels reaches a wired function from {fn_label(db, target)} '
              f'(kinds {",".join(walk)}; vtable/Lua edges need tools/callgraph_indirect.py)')
        return
    print(f'{len(found)} shortest chain(s) from wired code to {fn_label(db, target)}:')
    for start in found[:limit]:
        chain, node = [], start
        while parent[node] is not None:
            nxt_node, k = parent[node]
            chain.append(f'{fn_label(db, node)} -{k}->')
            node = nxt_node
        print('  ' + ' '.join(chain) + f' {fn_label(db, target)}')


def docs_for(args):
    db = connect()
    a = int(args.address, 16)
    docs = db.execute('SELECT d.path, d.title FROM doc_addresses da JOIN docs d ON d.path=da.path WHERE da.address=? ORDER BY d.path', (a,)).fetchall()
    print(f'docs mentioning {h(a)} ({len(docs)}):')
    for d in docs[:args.limit]:
        print(f"  {d['path']}  {d['title']}")


def segment(args):
    db = connect()
    if re.fullmatch(r'(0x)?[0-9a-fA-F]{5,8}', str(args.id)) and not str(args.id).isdigit() or len(str(args.id)) >= 5:
        a = int(str(args.id), 16)  # an address: find the segment that contains it
        s = db.execute('SELECT * FROM segments WHERE start <= ? AND ? < end ORDER BY start DESC LIMIT 1', (a, a)).fetchone()
        if s is None:
            sys.exit(f'{h(a)} is in no partition segment')
    else:
        s = db.execute('SELECT * FROM segments WHERE id=?', (int(args.id),)).fetchone()
    if s is None:
        sys.exit(f'no segment {args.id}')
    print(f"segment {s['id']} [{h(s['start'])}-{h(s['end'])}] candidates={s['candidates']} purity={s['purity']} wave={s['wave']} "
          f"lua_bindings={s['lua_bindings']} vtables={s['vtables']}\nkeywords: {s['keywords']}")
    done = db.execute('SELECT COUNT(*) FROM functions WHERE segment=? AND recon_kind IS NOT NULL', (s['id'],)).fetchone()[0]
    named = db.execute("SELECT COUNT(*) FROM functions WHERE segment=? AND ledger_name IS NOT NULL", (s['id'],)).fetchone()[0]
    print(f'reconstructed here: {done}; reviewed names here: {named}')
    for f in db.execute("SELECT * FROM functions WHERE segment=? AND name LIKE 'FUN_%' ORDER BY address LIMIT ?", (s['id'], args.limit)):
        print(f"  {f['hex']} {f['name']}")


def disasm_raw(args):
    """Local Capstone disassembly of the disk PE bytes; works for addresses Ghidra never defined."""
    try:
        import pefile
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        sys.exit('pefile and capstone are required')
    config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    pe = pefile.PE(config['binary'], fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    a = int(args.address, 16)
    section = next((s for s in pe.sections if base + s.VirtualAddress <= a < base + s.VirtualAddress + s.Misc_VirtualSize), None)
    if section is None:
        sys.exit(f'{h(a)} is not inside a section')
    data = section.get_data()
    offset = a - (base + section.VirtualAddress)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    db = connect(required=False)
    count = 0
    for address, size, mnemonic, operands in md.disasm_lite(data[offset:offset + args.length], a):
        line = f'{address:08x}: {mnemonic} {operands}'
        if db:
            line = annotate(db, line)
        print(line)
        count += 1
        if args.lines and count >= args.lines:
            print(f'... capped at {args.lines} instructions (--lines); the byte window is --length {args.length}')
            break
    print(f'(disk bytes, {section.Name.rstrip(b"\x00").decode()} section; Ghidra state not consulted)')


def scan_bytes(args):
    """Search the disk PE image for a byte pattern with ?? wildcards; no Ghidra state involved."""
    try:
        import pefile
    except ImportError:
        sys.exit('pefile is required')
    tokens = args.pattern.replace(',', ' ').split()
    if not tokens or any(not (t == '??' or re.fullmatch(r'[0-9a-fA-F]{2}', t)) for t in tokens):
        sys.exit('pattern: space-separated hex bytes, ?? for a wildcard, e.g. "55 8b ec ?? ?? e8"')
    regex = re.compile(b''.join(b'.' if t == '??' else re.escape(bytes.fromhex(t)) for t in tokens), re.S)
    config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    pe = pefile.PE(config['binary'], fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    db = connect(required=False)
    shown = 0
    total = 0
    lo = int(args.range.split('-')[0], 16) if args.range else 0
    hi = int(args.range.split('-')[1], 16) if args.range else 1 << 32
    for section in pe.sections:
        name = section.Name.rstrip(b'\x00').decode(errors='replace')
        if args.section and name != args.section:
            continue
        data = section.get_data()
        start = base + section.VirtualAddress
        for m in regex.finditer(data):
            a = start + m.start()
            if not (lo <= a < hi):
                continue
            total += 1
            if shown < args.limit:
                where = ''
                if db:
                    row = db.execute('SELECT address, name FROM functions WHERE address <= ? ORDER BY address DESC LIMIT 1', (a,)).fetchone()
                    if row:
                        where = f"  in {row['address']:08x} {row['name']}" if isinstance(row['address'], int) else f"  in {row['address']} {row['name']}"
                print(f'{a:08x}  {name:<8} {m.group(0).hex(" ")}{where}')
                shown += 1
    if total > shown:
        print(f'... {total - shown} more matches (raise --limit or narrow with --section)')
    if not total:
        print('no matches')


def strings_query(args):
    """Functions whose bodies reference a string containing the text (from the Capstone data-reference sweep)."""
    db = connect()
    rows = db.execute('SELECT DISTINCT d.func, d.text FROM datarefs d WHERE d.text LIKE ? ORDER BY d.func LIMIT ?',
                      (f'%{args.text}%', args.limit + 1)).fetchall()
    for r in rows[:args.limit]:
        print(f"{fn_label(db, r['func']):<52} {r['text'][:80]}")
    if len(rows) > args.limit:
        print('... more (raise --limit)')
    if not rows:
        print('no referenced string contains that text (strings only referenced from data tables are not indexed)')


def find(args):
    db = connect()
    like = f'%{args.text}%'
    rows = db.execute('SELECT * FROM functions WHERE name LIKE ? OR ledger_name LIKE ? OR recon_name LIKE ? ORDER BY address LIMIT ?',
                      (like, like, like, args.limit + 1)).fetchall()
    for f in rows[:args.limit]:
        print(f"{f['hex']} {f['name']:<44} {f['ledger_name'] or ''} {f['recon_status'] or ''}")
    if len(rows) > args.limit:
        print('... more (raise --limit)')


# ---------------------------------------------------------------- state card

def git(*argv):
    return subprocess.run(['git', *argv], cwd=ROOT, capture_output=True, text=True).stdout.strip()


def state(args):
    """One-screen orientation: snapshot, ledgers, index freshness, git, packets. Replaces re-reading docs."""
    snap = json.loads((EXPORTS / 'snapshot.json').read_text()) if (EXPORTS / 'snapshot.json').exists() else None
    if snap:
        print(f"snapshot {snap['utc'][:19]}Z  internal={snap['internal_function_count']}  total={snap['total_function_count']}")
    else:
        print('snapshot: none (python tools/bsp.py snapshot)')
    recon = ledger.load_reconstruction()
    print(f"ledger: {len(recon['functions'])} functions, {len(recon['fragments'])} fragments, {len(ledger.load_names())} reviewed names, "
          f"{len(ledger.load_tags())} tags")
    legacy = [p.name for p in (ledger.LEGACY_NAMES, ledger.LEGACY_RECON, ledger.LEGACY_TAGS) if p.exists()]
    if legacy:
        print(f"legacy ledger files present {legacy}: run python tools/bsp.py ledger migrate before committing")
    owner = coordination.owner_name()
    leases = coordination.active_leases()
    mine = [l for l in leases if l['owner'] == owner]
    lock = coordination.lock_status()
    print(f"agent: owner={owner}  worktree={ROOT}  registry={coordination.coordination_dir()}"
          + (f"  ghidra-lock={lock['owner']}{' (expired)' if lock.get('expired') else ''}" if lock else ''))
    print(f"leases: {len(leases)} active" + ('' if mine else f"  (none for {owner}: python tools/bsp.py lease claim --packet <id> --from-packet)"))
    for l in leases[:args.limit]:
        print(f"  {l['owner']:<18} {l['packet']:<28} {len(l.get('addresses', [])):3d} addrs {len(l.get('ranges', [])):2d} ranges "
              f"{len(l.get('files', [])):2d} files  until {l['expires'][:16]}")
    db = connect(required=False)
    if db:
        fun = db.execute("SELECT COUNT(*) FROM functions WHERE name LIKE 'FUN_%' AND thunk=0").fetchone()[0]
        funclets = db.execute("SELECT COUNT(*) FROM functions WHERE name LIKE 'Unwind@%' OR name LIKE 'Catch_All@%' OR name LIKE 'EHHandler@%'").fetchone()[0]
        exported = db.execute('SELECT COUNT(*) FROM functions WHERE exported=1').fetchone()[0]
        print(f"functions: {fun} untagged FUN_, {funclets} EH funclets, {exported} exported")
        meta = index_meta()
        fresh = meta.get('inputs_sha256') == inputs_digest()
        print(f"index: built {meta.get('built_utc', '?')[:19]}  {'fresh' if fresh else 'STALE -> python tools/bsp.py index'}")
    else:
        print('index: none -> python tools/bsp.py index')
    try:
        import ghidra_launch
        config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
        live, detail = ghidra_launch.probe(config, timeout=1.0)
        print(f"ghidra: up  {detail}" if live == 'up' else
              f"ghidra: {live.upper()} ({detail}) -> python tools/bsp.py ghidra ensure")
    except Exception as error:  # orientation must never fail on a probe
        print(f'ghidra: unprobed ({type(error).__name__})')
    status = git('status', '--short').splitlines()
    modified = sum(1 for l in status if not l.startswith('??'))
    untracked = sum(1 for l in status if l.startswith('??'))
    print(f"git: {git('rev-parse', '--abbrev-ref', 'HEAD')} @ {git('log', '-1', '--format=%h %ci %s')[:90]}  ({modified} modified, {untracked} untracked)")
    packets_path = ROOT / 'config/parallel_work.json'
    if packets_path.exists():
        try:
            work = json.loads(packets_path.read_text(encoding='utf-8'))
            packets = work.get('packets', [])
            print(f"packets ({len(packets)}):")
            for p in packets[:args.limit]:
                ident = p.get('id') or p.get('name') or '?'
                status_text = p.get('status') or p.get('state') or p.get('phase') or ''
                owner = p.get('owner') or p.get('worker') or ''
                print(f"  {ident:<28} {status_text[:40]:<40} {owner}")
            if len(packets) > args.limit:
                print(f'  ... {len(packets) - args.limit} more in config/parallel_work.json')
            ready = work.get('next_dispatch', [])
            if ready:
                print(f'next packets ({len(ready)}):')
                for p in ready[:args.limit]:
                    addresses = p.get('function_addresses', [])
                    anchors = ','.join(addresses[:5]) + (' ...' if len(addresses) > 5 else '')
                    print(f"  {p.get('packet', '?'):<28} {p.get('worker', ''):<18} {anchors}")
        except (ValueError, AttributeError):
            print('packets: config/parallel_work.json unreadable')


# ---------------------------------------------------------------- live Ghidra

def client():
    from ghidra_export import Client
    config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    c = Client(config)
    try:
        c.verify()
    except (OSError, RuntimeError) as error:
        import ghidra_launch
        # Only a dead server is ours to fix; a wrong project or a broken endpoint is not.
        if os.environ.get('BSP_GHIDRA_AUTOSTART', '1') == '0' or ghidra_launch.probe(config)[0] != 'down':
            raise
        note = lambda line: print(line, file=sys.stderr)  # noqa: E731  keep progress out of the capped stdout
        note(f'ghidra: {config["ghidra_url"]} not answering ({type(error).__name__}); starting it')
        if not ghidra_launch.ensure(config, report=note):
            raise
        c.verify()
    return c


def as_text(result):
    return result if isinstance(result, str) else json.dumps(result, indent=1)


def first_int(text):
    m = re.search(r'\d+', str(text))
    return int(m.group()) if m else None


def instruction_address(text):
    value = str(text).strip()
    if not re.fullmatch(r'(?:0[xX])?[0-9a-fA-F]{1,8}', value):
        raise argparse.ArgumentTypeError('address must be a 32-bit hexadecimal value')
    return f'{int(value, 16):08x}'


def instruction_context_count(text):
    try:
        value = int(text, 10)
    except ValueError as exc:
        raise argparse.ArgumentTypeError('context must be an integer from 0 through 64') from exc
    if not 0 <= value <= 64:
        raise argparse.ArgumentTypeError('context must be an integer from 0 through 64')
    return value


def ghidra_instruction_context(c, addresses, context):
    """Read exact listing starts through the bridge's typed POST endpoint."""
    import time
    from urllib.parse import urlencode
    from urllib.request import Request, urlopen

    endpoint = 'get_assembly_context'
    body = {
        'xref_sources': addresses,
        'context_instructions': context,
        'include_patterns': [],
    }
    url = c.config['ghidra_url'].rstrip('/') + '/' + endpoint + '?' + urlencode({'program': c.config['program']})
    request = Request(url, data=json.dumps(body).encode('utf-8'),
                      headers={'Content-Type': 'application/json'}, method='POST')
    for attempt in range(3):
        try:
            with urlopen(request, timeout=90) as response:
                raw = response.read()
            break
        except (TimeoutError, OSError):
            if attempt == 2:
                raise
            time.sleep(1 + attempt)
    text = raw.decode('utf-8')
    try:
        result = json.loads(text)
    except json.JSONDecodeError:
        result = text
    if isinstance(result, dict) and (result.get('error') or result.get('success') is False):
        raise RuntimeError(f'{endpoint}: {result}')
    if isinstance(result, str) and result.lstrip().lower().startswith(('error', 'failed', 'no program')):
        raise RuntimeError(f'{endpoint}: {result}')
    return result, raw


def ghidra_cmd(args):
    sub = args.ghidra_command
    if sub in ('ensure', 'autostart'):
        import ghidra_launch
        live = lambda line: print(line, file=sys.stderr, flush=True)  # noqa: E731  a launch takes minutes; stream it
        if sub == 'autostart':
            ghidra_launch.autostart(install=not args.remove, delay=args.delay, wait=args.wait, report=live)
            return
        ok = ghidra_launch.ensure(wait=args.wait, restart=args.restart, force_kill=args.force_kill,
                                  status_only=args.status, report=live)
        raise SystemExit(0 if ok else 1)
    if sub == 'typed-flow':
        import ghidra_typed_flow
        typed_args = [*args.addresses, '--output', args.output, '--max-ranges', str(args.max_ranges)]
        if args.config:
            typed_args.extend(['--config', args.config])
        raise SystemExit(ghidra_typed_flow.main(typed_args))
    c = client()
    db = connect(required=False)
    if sub == 'count':
        live = first_int(c.get('get_function_count'))
        snap = json.loads((EXPORTS / 'snapshot.json').read_text())['total_function_count'] if (EXPORTS / 'snapshot.json').exists() else None
        print(f'live function count {live}; snapshot {snap}; {"changed" if live != snap else "same"}')
    elif sub == 'proto':
        for value in args.addresses:
            a = norm(value)
            info = as_text(c.get('get_function_by_address', address=a))
            if args.brief:
                # one line per address: name, signature and body span
                fields = {k.strip(): v.strip() for k, v in (line.split(':', 1) for line in info.splitlines() if ':' in line)}
                print(f"{a} {fields.get('Function', '?').split(' at ')[0]}  {fields.get('Signature', '?')}  body {fields.get('Body', '?')}")
                continue
            cap(info, 6)
            try:
                cap(as_text(c.get('get_function_signature', address=a)), args.lines)
            except RuntimeError as exc:
                print(f'(signature unavailable: {str(exc)[:120]})')
    elif sub == 'flow':
        # Read-only: the bridge's clear_instruction_flow_override dry run actually clears, so
        # the gap report comes from comparing Ghidra's listing with the disk bytes instead.
        cmd = [sys.executable, str(ROOT / 'tools/ghidra_flow_repair.py'), *[norm(v) for v in args.addresses]]
        run = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
        cap((run.stdout + run.stderr).strip(), args.lines)
    elif sub == 'flow-properties':
        cmd = [sys.executable, str(ROOT / 'tools/ghidra_flow_properties.py'),
               *[norm(v) for v in args.addresses], '--output', args.output]
        run = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
        cap((run.stdout + run.stderr).strip(), args.lines)
        if run.returncode:
            raise SystemExit(run.returncode)
    elif sub == 'instruction-context':
        result, raw = ghidra_instruction_context(c, args.addresses, args.context)
        if args.output:
            path = (ROOT / args.output).resolve()
            if not path.is_relative_to((ROOT / 'local').resolve()) or path.suffix != '.json':
                sys.exit('Instruction-context output must be a JSON file under local/.')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(raw)
            print(f'Saved the full instruction-context response for {len(args.addresses)} addresses to {path.relative_to(ROOT).as_posix()}')
        else:
            cap(as_text(result), args.lines)
    elif sub in ('comments', 'documentation'):
        # Annotation readback often spans a batch. Persist complete records in
        # ignored local storage while keeping the interactive view bounded.
        path = None
        if args.output:
            path = (ROOT / args.output).resolve()
            if not path.is_relative_to((ROOT / 'local').resolve()) or path.suffix != '.json':
                sys.exit('Annotation output must be a JSON file under local/.')
        if sub == 'documentation':
            rows = [{'address': norm(a), 'documentation':
                     c.get('get_function_documentation', address=norm(a))}
                    for a in args.addresses]
        else:
            rows = [{'address': norm(a), 'annotation': c.get('get_plate_comment', address=norm(a))}
                    for a in args.addresses]
        if path:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps(rows, indent=2) + '\n', encoding='utf-8', newline='\n')
            print(f'Saved {len(rows)} {sub} records to {path.relative_to(ROOT).as_posix()}')
        else:
            cap(as_text(rows), args.lines, args.start)
    elif sub in ('xrefs', 'callers', 'callees'):
        a = norm(args.address)
        endpoint = {'xrefs': 'get_xrefs_to', 'callers': 'get_function_callers', 'callees': 'get_function_callees'}[sub]
        result = c.get(endpoint, address=a, limit=args.limit)
        cap(annotate(db, as_text(result)), args.lines)
    elif sub == 'bytes':
        a = norm(args.address)
        result = c.get('read_memory', address=a, length=args.length)
        hexstr = result.get('hex') if isinstance(result, dict) else re.sub(r'[^0-9a-fA-F]', '', str(result))
        raw = bytes.fromhex(hexstr)
        for off in range(0, len(raw), 16):
            chunk = raw[off:off + 16]
            print(f'{int(a, 16) + off:08x}  {chunk.hex(" ")}  {"".join(chr(b) if 32 <= b < 127 else "." for b in chunk)}')
    elif sub in ('decompile', 'disasm'):
        a = norm(args.address)
        endpoint = ('force_decompile' if args.force else 'decompile_function') if sub == 'decompile' else 'disassemble_function'
        try:
            result = c.get(endpoint, address=a)
        except RuntimeError as exc:
            _, lines = header_lines(db, int(a, 16)) if db else (None, [])
            sys.exit(f"{sub}: Ghidra has no function starting at {a} ({str(exc)[:80]}). " + (lines[0] if lines else ''))
        text = as_text(result)
        start = args.start
        if sub == 'disasm':
            # disassemble_function lists the whole enclosing function; when the address is
            # mid-function, start the excerpt at that instruction instead of the head.
            # --start also accepts a hex address (an instruction start) instead of a line count.
            lines = text.splitlines()
            if isinstance(start, str) and re.fullmatch(r'(0x)?[0-9a-fA-F]{5,8}', start):
                want = norm(start) + ':'
                pos = [i for i, line in enumerate(lines) if line.lower().startswith(want)]
                if not pos:
                    sys.exit(f'{norm(start)} is not an instruction start inside the stored body of {a}; for a truncated body use disasm-raw {norm(start)}')
                text = chr(10).join(lines[pos[0]:])
                start = 0
            hits = [i for i, line in enumerate(lines) if line.lower().startswith(a + ':')]
            if hits and hits[0]:
                text = chr(10).join(lines[hits[0]:])
            elif not hits and lines and not lines[0].lower().startswith(a + ':'):
                print(f'({a} is not an instruction start in the stored listing; showing the enclosing function from its head)')
        cap(text, args.lines, int(start))
    elif sub == 'export':
        addresses = [norm(x) for x in args.addresses]
        cmd = [sys.executable, str(ROOT / 'tools/ghidra_export.py'), 'decompile', '--addresses', *addresses]
        if args.force:
            cmd.append('--force')
        run = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
        tail = (run.stdout + run.stderr).strip().splitlines()[-3:]
        print('\n'.join(tail))
        for a in addresses:
            path = EXPORTS / 'functions' / a / 'decompiled.c'
            print(f"{a}: {'exported ' + rel(path) if path.exists() else 'FAILED'}  -> python tools/bsp.py show {a}")
        if run.returncode:
            sys.exit(run.returncode)


def snapshot(args):
    c = client()
    live = first_int(c.get('get_function_count'))
    snap_path = EXPORTS / 'snapshot.json'
    previous = json.loads(snap_path.read_text())['total_function_count'] if snap_path.exists() else None
    if live == previous and not args.force:
        print(f'snapshot skipped: Ghidra still reports {live} functions (use --force to refresh anyway)')
    else:
        print(f'function count {previous} -> {live}; taking snapshot')
        run = subprocess.run([sys.executable, str(ROOT / 'tools/ghidra_export.py'), 'snapshot'], cwd=ROOT, capture_output=True, text=True)
        print((run.stdout + run.stderr).strip().splitlines()[-1] if (run.stdout + run.stderr).strip() else '')
        if run.returncode:
            sys.exit(run.returncode)
    args.if_stale = True
    build_index(args)


# ---------------------------------------------------------------- ledger

def ledger_cmd(args):
    try:
        ledger_write(args)
    except coordination.LeaseConflict as exc:
        sys.exit(f'lease conflict: {exc}')


def ledger_write(args):
    if args.ledger_command == 'migrate':
        print(ledger.migrate(prune=not args.keep_legacy))
    elif args.ledger_command == 'add-name':
        address = ledger.norm(args.address)
        coordination.check_writable([address], force=args.force)
        existing = next((r for r in ledger.load_names() if r['address'] == address), None)
        evidence = args.evidence
        if existing and args.append_evidence:
            evidence = existing['evidence'].rstrip() + ' | ' + args.evidence.strip()
        elif existing and not args.replace:
            sys.exit(f"{address} already has a reviewed name; pass --replace to supersede it or --append-evidence to extend it. Existing record:\n{json.dumps(existing)}")
        previous = ledger.upsert(ledger.NAMES_DIR, {'address': address, 'name': args.name, 'evidence': evidence})
        print(f"{address} -> {args.name}")
        if previous and args.append_evidence:
            print(f"appended evidence (record keeps its earlier text; previous record: {json.dumps(previous)})")
        elif previous:
            print(f"replaced (previous record stays in git history): {json.dumps(previous)}")
    elif args.ledger_command in ('add-function', 'add-fragment'):
        data = json.loads(Path(args.json_file).read_text(encoding='utf-8')) if getattr(args, 'json_file', None) else json.loads(args.json)
        records = data if isinstance(data, list) else [data]
        for record in records:
            for key in ('address', 'name', 'source', 'status'):
                if key not in record:
                    sys.exit(f'missing field {key}')
            record['address'] = ledger.norm(record['address'])
            record['kind'] = 'function' if args.ledger_command == 'add-function' else 'fragment'
        keys = [(r['address'], r['kind'], r['name']) for r in records]
        if len(keys) != len(set(keys)):
            sys.exit('duplicate reconstruction key in batch')
        # Validate the entire batch and its leases before the first ledger mutation.
        coordination.check_writable([r['address'] for r in records], force=args.force)
        recon = ledger.load_reconstruction()
        for record in records:
            pool = recon['fragments'] if record['kind'] == 'fragment' else recon['functions']
            existing = next((r for r in pool if r['address'] == record['address'] and r.get('name') == record['name']), None)
            if existing and not args.replace:
                sys.exit(f"{record['address']} {record['kind']} {record['name']} already recorded; pass --replace to supersede it. Existing record:\n{json.dumps(existing)}")
        for record in records:
            previous = ledger.upsert(ledger.RECON_DIR, record, key=ledger.RECON_KEY)
            if len(records) == 1:
                print(f"{record['address']} {record['kind']} {record['name']}")
                if previous:
                    print(f"replaced (previous record stays in git history): {json.dumps(previous)}")
        if len(records) != 1:
            print(f"recorded {len(records)} {args.ledger_command} records; previous values stay in git history")


# ---------------------------------------------------------------- coordination

def load_packets():
    path = ROOT / 'config/parallel_work.json'
    return (json.loads(path.read_text(encoding='utf-8')) if path.exists() else {}), path


def packet_done(packet):
    text = str(packet.get('state') or packet.get('status') or '')
    return bool(packet.get('done')) or 'integrated' in text or text in ('done', 'complete', 'completed')


def packet_dependencies(packet):
    return sorted(set(packet.get('depends_on', [])) | set(packet.get('prerequisite_packets', [])))


def packet_source_blocked(packet):
    scope = packet.get('scope')
    return packet.get('source_ready') is False or (isinstance(scope, dict) and scope.get('ready') is False)


def lease_cmd(args):
    sub = args.lease_command
    if sub == 'claim':
        addresses, ranges, files = list(args.addresses or []), list(args.ranges or []), list(args.files or [])
        if args.from_packet:
            work, _ = load_packets()
            packet = next((p for p in work.get('packets', []) if p.get('id') == args.packet), None)
            if packet is None:
                sys.exit(f'no packet {args.packet} in config/parallel_work.json')
            addresses += [a for a in packet.get('function_addresses', [])]
            ranges += [r for r in packet.get('code_ranges', []) if isinstance(r, (str, dict))]
            files += [f for f in packet.get('output_files', []) if isinstance(f, str)]
        if not (addresses or ranges or files):
            sys.exit('a lease needs --addresses, --ranges lo-hi, --files, or --from-packet')
        try:
            lease = coordination.claim(args.packet, addresses, ranges, files, ttl_hours=args.ttl, owner=args.owner,
                                       note=args.note or '', jump=args.jump or '')
        except coordination.LeaseConflict as exc:
            sys.exit(f'lease refused: {exc}')
        print(f"leased {lease['id']}: {len(lease['addresses'])} addresses, {len(lease['ranges'])} ranges, "
              f"{len(lease['files'])} files until {lease['expires']}")
        if args.jump and lease.get('jumped'):
            print(f"jumped {lease['jumped'][-1]['over']} (reason recorded and mailed)")
    elif sub == 'queue':
        if not (args.addresses or args.ranges or args.files):
            sys.exit('a waiter needs --files, --addresses or --ranges lo-hi')
        row = coordination.queue(args.packet, args.addresses or [], args.ranges or [], args.files or [],
                                 ttl_hours=args.ttl, owner=args.owner, note=args.note or '')
        print(f"queued {row['id']} since {row['queued'][:16]} until {row['expires'][:16]}")
        active, waiters = coordination.active_leases(), coordination.live_waiters()
        for f in row['files']:
            holders = [l['id'] for l in coordination.path_holders(f, active) if l['owner'] != row['owner']]
            line = [w['id'] for w in coordination.path_waiters(f, waiters)]
            print(f"  {f}: held by {', '.join(holders) or 'nobody'}; place {line.index(row['id']) + 1} of {len(line)}"
                  + ('' if holders else ' -> free now: lease claim'))
        if row['addresses'] or row['ranges']:
            part = {'addresses': row['addresses'], 'ranges': row['ranges'], 'files': []}
            holders = [l['id'] for l in active if l['owner'] != row['owner'] and coordination.overlaps(part, l)]
            ahead = [w['id'] for w in coordination.waiters_ahead(part, row['owner'], waiters)]
            print(f"  addresses/ranges: held by {', '.join(holders[:4]) or 'nobody'}; ahead: {', '.join(ahead[:4]) or 'nobody'}")
    elif sub == 'unqueue':
        gone = coordination.unqueue(args.packet, owner=args.owner)
        print(f"left the queue: {[w['id'] for w in gone]}" if gone else 'no waiter row to remove')
    elif sub == 'release':
        released = coordination.release(args.id, packet=args.packet, owner=args.owner)
        print(f"released {[r['id'] for r in released]}" if released else 'nothing active to release')
        active, waiters = coordination.active_leases(), coordination.live_waiters()
        for r in released:
            for f in r.get('files', [])[:args.limit]:
                holders = [l['id'] for l in coordination.path_holders(f, active)]
                line = [w['id'] for w in coordination.path_waiters(f, waiters) if w['owner'] != r['owner']]
                if line or holders:
                    print(f"  {f}: next {line[0] if line else 'nobody queued'}"
                          + (f" (still held by {', '.join(holders)})" if holders else ''))
            part = {'addresses': r.get('addresses', []), 'ranges': r.get('ranges', []), 'files': []}
            line = [w['id'] for w in coordination.resource_waiters(dict(part, owner=r['owner']), waiters)]
            if line:
                print(f"  addresses/ranges: next {line[0]}" + (f" then {', '.join(line[1:3])}" if len(line) > 1 else ''))
    elif sub == 'list':
        rows = coordination.active_leases() if not args.all else coordination.load_leases()
        waiters = coordination.live_waiters()
        shown = set()
        for l in rows:
            print(f"{l['id']:<40} {l.get('status', ''):<9} until {l['expires'][:16]}  {len(l.get('addresses', []))} addrs "
                  f"{l.get('ranges', [])} files={l.get('files', [])} worktree={l.get('worktree', '')}")
            if args.verbose:
                addrs = l.get('addresses', [])
                print(f"    addresses: {' '.join(addrs[:args.limit])}" + (f' ... {len(addrs) - args.limit} more' if len(addrs) > args.limit else ''))
            if l.get('status') == 'active':
                for f in l.get('files', []):
                    line = [w for w in coordination.path_waiters(f, waiters) if w['owner'] != l['owner']]
                    shown.update(w['id'] for w in line)
                    if line:
                        print(f"    queue {f}: " + ', '.join(w['id'] for w in line))
                part = {'addresses': l.get('addresses', []), 'ranges': l.get('ranges', []), 'files': [], 'owner': l['owner']}
                line = coordination.resource_waiters(part, waiters) if part['addresses'] or part['ranges'] else []
                shown.update(w['id'] for w in line)
                if line:
                    print(f"    queue addresses: " + ', '.join(w['id'] for w in line))
        if not rows:
            print('no leases')
        rest = [w for w in waiters if w['id'] not in shown]
        if rest:
            print('waiting on unleased resources: ' + '; '.join(
                f"{w['id']} ({' '.join(w.get('files', [])[:3]) or str(len(w.get('addresses', []))) + ' addrs'})" for w in rest))
    elif sub == 'check':
        active = coordination.active_leases()
        waiters = coordination.live_waiters()
        for target in args.targets:
            if re.fullmatch(r'(0x)?[0-9a-fA-F]{5,8}', target):
                a = int(target, 16)
                holders = coordination.holders(a)
                line = [w['id'] for w in waiters if coordination.covers(w, a)]
                print(f"{h(a)}: " + ('; '.join(f"{l['id']} until {l['expires'][:16]}" for l in holders) if holders else 'unleased')
                      + (f"  queue: {', '.join(line)}" if line else ''))
            else:
                path = target.replace(chr(92), '/')
                holders = coordination.path_holders(path, active)  # directory-aware, as a claim refusal
                line = [w['id'] for w in coordination.path_waiters(path, waiters)]
                print(f"{path}: " + ('; '.join(f"{l['id']} until {l['expires'][:16]}" for l in holders) if holders else 'unleased')
                      + (f"  queue: {', '.join(line)}" if line else ''))
    elif sub == 'lock-status':
        lock = coordination.lock_status()
        print(json.dumps(lock) if lock else 'ghidra lock free')


def message_header(m):
    first = m['text'].splitlines()[0] if m['text'] else ''
    return f"#{m['id']} {m['sent'][5:16].replace('T', ' ')} from {m['from']} to {m['to']}: {first[:90]}"


def msg_cmd(args):
    """Codex sessions cannot receive SendMessage; this registry mailbox reaches every harness through brief."""
    sub = args.msg_command
    if sub == 'send':
        text = Path(args.file).read_text(encoding='utf-8') if args.file else ' '.join(args.text)
        try:
            row = coordination.send_message(args.to, text, sender=args.sender)
        except ValueError as exc:
            sys.exit(f'not sent: {exc}')
        print(f"sent #{row['id']} to {row['to']}")
    elif sub == 'inbox':
        owner = args.owner or coordination.owner_name()
        rows = coordination.mailbox(owner, include_read=args.all)
        unread = [m for m in rows if owner not in m.get('read_by', [])]
        shown = rows[-args.limit:] if args.all else unread[:args.limit]
        print(f"inbox for {owner}: {len(unread)} unread" + (f", {len(rows)} total" if args.all else '')
              + (f"; showing {len(shown)} (--limit)" if len(shown) < (len(rows) if args.all else len(unread)) else ''))
        for m in shown:
            print(message_header(m) + ('' if owner in m.get('read_by', []) else '  [new]'))
            rest = m['text'].splitlines()[1:]
            for line in rest:
                print(f"    {line}")
        marked = coordination.mark_read([m['id'] for m in shown if owner not in m.get('read_by', [])], owner)
        if marked:
            print(f"marked read: {' '.join(f'#{i}' for i in marked)}")
    elif sub == 'ack':
        owner = args.owner or coordination.owner_name()
        ids = [m['id'] for m in coordination.mailbox(owner)] if args.ids == ['all'] else args.ids
        marked = coordination.mark_read(ids, owner)
        print(f"marked read: {' '.join(f'#{i}' for i in marked)}" if marked else 'nothing unread to mark')


def packets_cmd(args):
    work, path = load_packets()
    packets = work.get('packets', [])
    by_id = {p.get('id'): p for p in packets}
    leases = coordination.active_leases()
    leased = {l['packet']: l['owner'] for l in leases}
    if args.packets_command in ('list', 'ready'):
        for p in packets:
            pid = p.get('id')
            deps = packet_dependencies(p)
            missing = [d for d in deps if not (d in by_id and packet_done(by_id[d]))]
            done = packet_done(p)
            source_blocked = packet_source_blocked(p)
            ready = not done and not missing and not source_blocked and pid not in leased
            if args.packets_command == 'ready' and not ready:
                continue
            flag = 'done' if done else ('leased:' + leased[pid] if pid in leased else
                    ('blocked:' + ','.join(missing) if missing else ('blocked:source_contract' if source_blocked else 'ready')))
            print(f"{pid:<28} {flag:<34} {len(p.get('function_addresses', [])):3d} addrs  {str(p.get('state') or '')[:50]}")
        # Packets the integrator has proposed but not yet promoted into `packets`.
        for nd in work.get('next_dispatch', []) or []:
            pid = nd.get('packet') or nd.get('id') or '?'
            if pid in by_id:
                continue
            flag = 'leased:' + leased[pid] if pid in leased else 'proposed'
            if args.packets_command == 'ready' and pid in leased:
                continue
            print(f"{pid:<28} {flag:<34} {len(nd.get('function_addresses', [])):3d} addrs  worker={nd.get('worker', '')}  {str(nd.get('work') or '')[:40]}")
    elif args.packets_command == 'done':
        p = by_id.get(args.id) or sys.exit(f'no packet {args.id}')
        p['done'] = True
        path.write_text(json.dumps(work, indent=1) + '\n', encoding='utf-8', newline='\n')
        print(f'{args.id} marked done')
    elif args.packets_command == 'depend':
        p = by_id.get(args.id) or sys.exit(f'no packet {args.id}')
        p['depends_on'] = sorted(set(p.get('depends_on', [])) | set(args.on))
        path.write_text(json.dumps(work, indent=1) + '\n', encoding='utf-8', newline='\n')
        print(f"{args.id} depends_on {p['depends_on']}")


def worktree_cmd(args):
    if args.worktree_command == 'list':
        print(git('worktree', 'list'))
        return
    main = workspace.main_root()
    if args.worktree_command == 'gc':
        sys.exit(worktree_gc.gc(args))
    if args.worktree_command == 'remove':
        # Never `git worktree remove` / `git clean` / Remove-Item: they follow junctions into the
        # shared exports. worktree_gc deletes with a walker that detaches reparse points instead.
        path = (main.parent / f'{main.name}-{args.name}').resolve()
        tree = next((t for t in worktree_gc.worktrees()
                     if os.path.normcase(str(t['path'].resolve())) == os.path.normcase(str(path))), None)
        if tree is None and not path.exists():
            worktree_gc.git('worktree', 'prune')
            print(f'{path} is not a worktree and does not exist; pruned stale entries')
            return
        if tree is None:
            sys.exit(f'{path} exists but is not a linked worktree of this repository; not touching it')
        dirty = worktree_gc.dirty_summary(path)
        if dirty and not args.force:
            sys.exit(f'{path} has uncommitted work ({dirty}); commit it, or pass --force to discard it')
        try:
            worktree_gc.remove_tree(path, tree['branch'], args.delete_branch, archive=True)
        except worktree_gc.ExportsLoss as error:
            sys.exit(f'STOP: {error}')
        return
    name = args.name
    branch = f'agent/{name}'
    path = (main.parent / f'{main.name}-{name}').resolve()
    if path.exists():
        sys.exit(f'{path} already exists')
    if args.base == 'origin/main':
        # the shared checkout's local main is often behind (or carries another lead's unpushed work)
        subprocess.run(['git', 'fetch', '-q', 'origin', 'main'], cwd=main, capture_output=True, text=True)
    run = subprocess.run(['git', 'worktree', 'add', str(path), '-b', branch, args.base], cwd=main, capture_output=True, text=True)
    if run.returncode:
        sys.exit((run.stdout + run.stderr).strip())
    print(f"worktree {path} on branch {branch}")
    print(f"exports resolve to {main / 'exports' / 'bsp'} through git (no junction; never create one inside a worktree)")
    print(f"next: cd {path}; python tools/bsp.py index; python tools/bsp.py lease claim --packet <id> --from-packet"
          + (f" (suggested packet: {args.packet})" if args.packet else ''))


CHEATSHEET = """bsp.py in one screen. Every read is capped at 2000 tokens; over that it spills to
local/output/ and prints the path. --full or BSP_OUTPUT_BUDGET=0 lifts the cap.

  brief                          start every turn here: state + dirty files + ready packets, one call
  sync                           fetch; fast-forward this branch to origin/main, or list the unlanded commits and refuse
  state [--limit N]              snapshot, ledgers, leases, index freshness
  lookup <addr>                  everything known about one address (name, recon, callers, docs)
  show <addr> [--asm] [--lines N] [--start N]   capped export excerpt; --start pages, --asm for listing
  range <lo> <hi> [--only FUN_]  functions in an address range
  callers|callees <addr>         any address inside a function resolves; one row per function and edge kind
      --kind call,tailjmp,addr_taken,data_ptr,eh,vslot,vcall?,lua   --sites (one row per site)
      --depth N   --unbuilt   --unwired   --status   --path-from-host (chains from wired code; reads wire_readiness.json)
  undefined [--direct]           referenced code Ghidra never made a function (from the call sweep)
  docs-for <addr>
  segment <id|addr>              partition band and its keywords
  find <text>                    name search        strings <text>   functions referencing a string
  disasm-raw <addr> [--length N] Capstone over disk bytes, works where Ghidra has no function
  scan-bytes '<pat ?? pat>'      byte pattern search, with the enclosing function

  ghidra count|proto|flow|instruction-context|xrefs|callers|callees|bytes|comments|decompile|disasm|documentation|export
  ghidra ensure [--status] [--restart] [--wait 300]   start Ghidra when nothing answers on ghidra_url
  ghidra autostart [--remove]                        do that at every logon (Startup folder, no elevation)
      proto/flow/comments take several addresses; decompile/disasm take one plus --lines
      flow-properties <addresses> --output local/response.json reads exact flow flags; needs bridge script capability
      typed-flow <addresses> --output local/response.json reads typed body/flow metadata; no script fallback
      instruction-context <addresses> [--context 0..64] [--output local/response.json] reads exact listing starts
      bytes <addr> --length N (NOT --limit)

  snapshot [--force]             re-export only if Ghidra's function count moved
  index [--if-stale]             rebuild local/bsp_index.sqlite after snapshots or ledger edits

  lease claim --packet <id> --from-packet | --addresses A B --files P [--ttl H]
  lease release [--packet <id>] | lease list [-v] | lease check <addr|path> | lease lock-status
  lease queue --packet <id> --files P [--addresses A] [--ttl 24] | lease unqueue --packet <id> | claim --jump 'reason'
  msg send <owner|prefix-|all> <text>|--file F | msg inbox [--all] | msg ack <id..|all>
  packets list|ready|done <id>|depend <id> --on <ids>
  worktree add <name> [--packet P] [--base origin/main] | worktree remove <name> [--force] [--delete-branch]
  worktree gc [--apply] [--idle-hours 12] [--merged-only] [--match TEXT] [--delete-merged-branches]
      the ONLY sanctioned way to delete trees: never git worktree remove / git clean / Remove-Item -Recurse
  worktree list   |   exports check | exports restore (hours: snapshot + index + re-decompile, resumable)
  ledger add-name <addr> <name> --evidence T [--replace|--append-evidence]
  ledger add-function|add-fragment --json '{...}' [--replace]   |   ledger migrate

Rules that cost money when broken: one representation at a time (pseudocode or assembly, not both);
batch independent reads into one shell call; never re-read a path already in this context.
"""


def exports_cmd(args):
    import exports_restore
    sys.exit(exports_restore.check() if args.exports_command == 'check' else exports_restore.restore())


def cheatsheet(args):
    print(CHEATSHEET, end='')


def sync_cmd(args):
    """A refused --ff-only means this branch has a commit main lacks, not that main moved."""
    sys.exit(branch_sync.sync(ROOT, fetch=not args.no_fetch))


def brief(args):
    """Replaces the state + git status + index + lease ritual that opened most sessions."""
    state(args)
    dirty = subprocess.run(['git', 'status', '--short'], cwd=ROOT, capture_output=True, text=True).stdout.splitlines()
    if dirty:
        print(f"dirty ({len(dirty)}): " + '; '.join(d.strip() for d in dirty[:args.limit])
              + (f" ... {len(dirty) - args.limit} more" if len(dirty) > args.limit else ''))
    else:
        print('dirty: clean')
    branch = subprocess.run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    if branch and branch != 'main':
        cherry = subprocess.run(['git', 'cherry', 'main', branch], cwd=ROOT, capture_output=True, text=True).stdout
        pending = sum(1 for l in cherry.splitlines() if l.startswith('+'))
        if pending:
            oldest = next((l.split()[1] for l in cherry.splitlines() if l.startswith('+')), None)
            when = subprocess.run(['git', 'log', '-1', '--format=%ad', '--date=format:%m-%d %H:%M', oldest],
                                  cwd=ROOT, capture_output=True, text=True).stdout.strip() if oldest else '?'
            flag = '  INTEGRATE NOW' if pending > 20 else ''
            print(f"unmerged: {pending} patches on {branch} not in main, oldest {when}{flag}")
        else:
            print(f"unmerged: none ({branch} fully in main)")
    print(branch_sync.tip_line(ROOT))
    work, _ = load_packets()
    packets = {p.get('id'): p for p in work.get('packets', [])}
    leased = {l['packet'] for l in coordination.active_leases()}
    ready = [pid for pid, p in packets.items()
             if not packet_done(p) and not packet_source_blocked(p) and pid not in leased
             and all(d in packets and packet_done(packets[d]) for d in packet_dependencies(p))]
    print(f"ready packets ({len(ready)}): " + ' '.join(sorted(ready)[:args.limit]))
    owner = coordination.owner_name()
    for w, target in coordination.your_turn(owner)[:args.limit]:
        print(f"your turn: {target} (queued for {w['packet']}) -> python tools/bsp.py lease claim --packet {w['packet']} ...")
    unread = coordination.mailbox(owner)
    if unread:
        print(f"mail: {len(unread)} unread -> python tools/bsp.py msg inbox")
        for m in unread[:3]:
            print(f"  {message_header(m)}")
    print('next: python tools/bsp.py cheatsheet   for the full command list')


class CompactParser(argparse.ArgumentParser):
    """Argument errors printed the whole usage banner 795 times in one month. Print the fix instead."""

    def error(self, message):
        prog = self.prog
        sys.stderr.write(f"{prog}: {message}\n")
        opts = [a for action in self._actions for a in action.option_strings if a.startswith('--')]
        if opts:
            sys.stderr.write(f"accepted here: {' '.join(sorted(set(opts)))}\n")
        sys.stderr.write("one-screen reference: python tools/bsp.py cheatsheet\n")
        sys.exit(2)


def main():
    parser = CompactParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--full', action='store_true', help='do not cap this command\'s output')
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('index'); p.add_argument('--if-stale', action='store_true'); p.set_defaults(func=build_index)
    p = sub.add_parser('state'); p.add_argument('--limit', type=int, default=8); p.set_defaults(func=state)
    p = sub.add_parser('brief', help='state + dirty files + ready packets in one call; start a turn here')
    p.add_argument('--limit', type=int, default=8); p.set_defaults(func=brief)
    sub.add_parser('cheatsheet', help='every subcommand on one screen').set_defaults(func=cheatsheet)
    p = sub.add_parser('sync', help='fetch, then fast-forward to origin/main or list the unlanded commits and refuse')
    p.add_argument('--no-fetch', action='store_true'); p.set_defaults(func=sync_cmd)
    p = sub.add_parser('lookup'); p.add_argument('address'); p.add_argument('--limit', type=int, default=12); p.add_argument('--width', type=int, default=300); p.set_defaults(func=lookup)
    p = sub.add_parser('show'); p.add_argument('address'); p.add_argument('--asm', action='store_true'); p.add_argument('--live', action='store_true')
    p.add_argument('--lines', '--limit', dest='lines', type=int, default=80); p.add_argument('--start', default='0', help='lines to skip, or with --asm a hex instruction address (8 digits or 0x...) to start at'); p.set_defaults(func=show)
    p = sub.add_parser('range'); p.add_argument('start'); p.add_argument('end'); p.add_argument('--only', help='name prefix filter, e.g. FUN_'); p.add_argument('--limit', type=int, default=40); p.set_defaults(func=range_query)
    p = sub.add_parser('undefined', help='referenced code that Ghidra never made a function (from the call sweep)')
    p.add_argument('--direct', action='store_true', help='only chunks something calls or jumps to')
    p.add_argument('--limit', type=int, default=25); p.set_defaults(func=undefined_query)
    for name in ('callers', 'callees'):
        p = sub.add_parser(name, help=f'{name} of the function containing ADDRESS (mid-function addresses resolve), one row per site and edge kind')
        p.add_argument('address'); p.add_argument('--limit', type=int, default=25)
        p.add_argument('--kind', help=f'comma list of edge kinds ({",".join(EDGE_KINDS)}); default all')
        p.add_argument('--depth', type=int, default=1, help='walk this many levels (call/tailjmp/vcall?/lua/addr_taken edges)')
        p.add_argument('--unbuilt', action='store_true', help='only functions with no reconstruction record')
        p.add_argument('--unwired', action='store_true', help='hide functions wire_readiness.py classified as wired')
        p.add_argument('--status', action='store_true', help='mark wired rows (reads local/output/wire_readiness.json)')
        p.add_argument('--path-from-host', action='store_true', help='shortest caller chains from wired code to ADDRESS')
        p.add_argument('--sites', action='store_true', help='one row per call site instead of one per function and kind')
        p.set_defaults(func=lambda a, d=name: calls_query(a, d))
    p = sub.add_parser('docs-for'); p.add_argument('address'); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=docs_for)
    p = sub.add_parser('segment', help='partition segment by id, or the one containing an address'); p.add_argument('id', metavar='ID|ADDRESS'); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=segment)
    p = sub.add_parser('find'); p.add_argument('text'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=find)
    p = sub.add_parser('strings', help='functions referencing a string containing the text'); p.add_argument('text'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=strings_query)
    p = sub.add_parser('disasm-raw', help='Capstone disassembly of disk bytes from an address (must be an instruction start), even where Ghidra has no function'); p.add_argument('address')
    p.add_argument('--length', '--limit', dest='length', type=lambda v: int(v, 0), default=96, help='bytes to decode, decimal or 0x hex (bounds the output)'); p.add_argument('--lines', type=int, default=0, help='optional instruction cap'); p.set_defaults(func=disasm_raw)
    p = sub.add_parser('scan-bytes', help='find a byte pattern (?? wildcards) in the disk image, with the enclosing function'); p.add_argument('pattern')
    p.add_argument('--limit', type=int, default=20); p.add_argument('--section', help='restrict to one section, e.g. .text'); p.add_argument('--range', help='hex lo-hi address window, e.g. 00680000-006a0000'); p.set_defaults(func=scan_bytes)
    p = sub.add_parser('snapshot'); p.add_argument('--force', action='store_true'); p.set_defaults(func=snapshot)
    p = sub.add_parser('ghidra', help='live, capped Ghidra queries through the loopback client'); gs = p.add_subparsers(dest='ghidra_command', required=True)
    gs.add_parser('count')
    q = gs.add_parser('proto'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=20); q.add_argument('--brief', action='store_true', help='one line per address: name, signature, body span')
    q = gs.add_parser('flow'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=40)
    q = gs.add_parser('flow-properties', help='read exact current flow/no-return/thunk properties without analysis changes'); q.add_argument('addresses', nargs='+'); q.add_argument('--output', required=True); q.add_argument('--lines', '--limit', dest='lines', type=int, default=40)
    q = gs.add_parser('typed-flow', help='capture and validate exact typed body/flow metadata without scripts or mutation'); q.add_argument('addresses', nargs='+'); q.add_argument('--output', required=True); q.add_argument('--max-ranges', type=int, default=4096); q.add_argument('--config')
    q = gs.add_parser('instruction-context', help='read instructions at exact addresses plus bounded listing neighbors; does not infer function flow')
    q.add_argument('addresses', nargs='+', type=instruction_address); q.add_argument('--context', type=instruction_context_count, default=0)
    q.add_argument('--output'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=80)
    q = gs.add_parser('comments'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=40); q.add_argument('--start', type=int, default=0); q.add_argument('--output')
    q = gs.add_parser('documentation', help='archive full function documentation before body repairs'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=40); q.add_argument('--start', type=int, default=0); q.add_argument('--output')
    for name in ('xrefs', 'callers', 'callees'):
        q = gs.add_parser(name); q.add_argument('address'); q.add_argument('--limit', type=int, default=25); q.add_argument('--lines', type=int, default=40)
    q = gs.add_parser('bytes'); q.add_argument('address'); q.add_argument('--length', '--limit', dest='length', type=int, default=64)
    for name in ('decompile', 'disasm'):
        q = gs.add_parser(name, help='decompile: whole function; disasm: listing from the given address to the function end'); q.add_argument('address'); q.add_argument('--lines', '--limit', dest='lines', type=int, default=80)
        q.add_argument('--start', default='0', help='lines to skip, or for disasm a hex instruction address to start at')
        if name == 'decompile':
            q.add_argument('--force', action='store_true', help='flush Ghidra decompiler cache before reading')
    q = gs.add_parser('export'); q.add_argument('addresses', nargs='+'); q.add_argument('--force', action='store_true')
    q = gs.add_parser('ensure', help='start Ghidra with the project and program restored when nothing answers')
    q.add_argument('--status', action='store_true', help='probe only, never launch')
    q.add_argument('--restart', action='store_true', help='also relaunch a front end that is running without a server')
    q.add_argument('--force-kill', action='store_true', help='with --restart, kill a front end that will not close (discards unsaved state)')
    q.add_argument('--wait', type=int, default=300, help='seconds to wait for the server')
    q = gs.add_parser('autostart', help='run that same check at every logon (Startup folder entry, no elevation)')
    q.add_argument('--remove', action='store_true'); q.add_argument('--delay', type=int, default=60); q.add_argument('--wait', type=int, default=420)
    p.set_defaults(func=ghidra_cmd)
    p = sub.add_parser('lease', help='address/file leases shared across worktrees and harnesses'); lz = p.add_subparsers(dest='lease_command', required=True)
    q = lz.add_parser('claim'); q.add_argument('--packet', required=True); q.add_argument('--owner'); q.add_argument('--addresses', nargs='*')
    q.add_argument('--ranges', nargs='*', help='lo-hi hex pairs'); q.add_argument('--files', nargs='*'); q.add_argument('--from-packet', action='store_true')
    q.add_argument('--ttl', type=float, default=8.0, help='hours'); q.add_argument('--note')
    q.add_argument('--jump', metavar='REASON', help='claim ahead of earlier waiters; the reason is recorded and mailed to them')
    q = lz.add_parser('queue', help='wait in line for leased files/addresses; claim refuses others ahead of you')
    q.add_argument('--packet', required=True); q.add_argument('--owner'); q.add_argument('--addresses', nargs='*')
    q.add_argument('--ranges', nargs='*', help='lo-hi hex pairs'); q.add_argument('--files', nargs='*')
    q.add_argument('--ttl', type=float, default=24.0, help='hours before the waiter row expires'); q.add_argument('--note')
    q = lz.add_parser('unqueue'); q.add_argument('--packet', required=True); q.add_argument('--owner')
    q = lz.add_parser('release'); q.add_argument('id', nargs='?'); q.add_argument('--packet'); q.add_argument('--owner')
    q.add_argument('--limit', type=int, default=20, help='released files reported per lease')
    q = lz.add_parser('list'); q.add_argument('--all', action='store_true'); q.add_argument('--verbose', '-v', action='store_true', help='also list each lease\'s addresses')
    q.add_argument('--limit', type=int, default=40, help='addresses shown per lease with --verbose')
    q = lz.add_parser('check', help='who holds each address or file path'); q.add_argument('targets', nargs='+', metavar='ADDRESS|FILE')
    lz.add_parser('lock-status')
    p.set_defaults(func=lease_cmd)
    p = sub.add_parser('msg', help='mailbox in the coordination registry, readable by every harness'); ms = p.add_subparsers(dest='msg_command', required=True)
    q = ms.add_parser('send'); q.add_argument('to', help='owner (agent/cc9-ships47), prefix ending in - / or * (agent/cc9-), or all')
    q.add_argument('text', nargs='*'); q.add_argument('--file', help='read the message text from a file'); q.add_argument('--from', dest='sender')
    q = ms.add_parser('inbox', help='unread mail for this owner; shown messages are marked read'); q.add_argument('--all', action='store_true', help='include read mail')
    q.add_argument('--limit', type=int, default=10); q.add_argument('--owner')
    q = ms.add_parser('ack'); q.add_argument('ids', nargs='+', metavar='ID|all'); q.add_argument('--owner')
    p.set_defaults(func=msg_cmd)
    p = sub.add_parser('packets', help='work packets from config/parallel_work.json with dependencies and leases'); pk = p.add_subparsers(dest='packets_command', required=True)
    pk.add_parser('list'); pk.add_parser('ready')
    q = pk.add_parser('done'); q.add_argument('id')
    q = pk.add_parser('depend'); q.add_argument('id'); q.add_argument('--on', nargs='+', required=True)
    p.set_defaults(func=packets_cmd)
    p = sub.add_parser('worktree', help='one git worktree per harness agent, sharing exports'); wt = p.add_subparsers(dest='worktree_command', required=True)
    q = wt.add_parser('add'); q.add_argument('name'); q.add_argument('--base', default='origin/main'); q.add_argument('--packet')
    q = wt.add_parser('remove', help='safe removal: archives local inputs, detaches reparse points, checks the shared exports')
    q.add_argument('name'); q.add_argument('--delete-branch', action='store_true', help='only when merged into origin/main')
    q.add_argument('--force', action='store_true', help='discard uncommitted work')
    q = wt.add_parser('gc', help='bulk-remove idle, clean, unleased worktrees (dry run unless --apply)')
    q.add_argument('--apply', action='store_true'); q.add_argument('--idle-hours', type=float, default=12.0)
    q.add_argument('--merged-only', action='store_true', help='also require the branch to be merged into origin/main')
    q.add_argument('--delete-merged-branches', action='store_true'); q.add_argument('--no-archive', action='store_true')
    q.add_argument('--match', nargs='*', help='only trees whose name contains one of these')
    q.add_argument('--limit', type=int, default=400)
    wt.add_parser('list')
    p.set_defaults(func=worktree_cmd)
    p = sub.add_parser('exports', help='check or rebuild the shared exports after a loss')
    p.add_argument('exports_command', choices=['check', 'restore'])
    p.set_defaults(func=exports_cmd)
    p = sub.add_parser('ledger'); ls = p.add_subparsers(dest='ledger_command', required=True)
    q = ls.add_parser('migrate'); q.add_argument('--keep-legacy', action='store_true')
    q = ls.add_parser('add-name'); q.add_argument('address'); q.add_argument('name'); q.add_argument('--evidence', required=True)
    q.add_argument('--force', action='store_true', help='ignore another owner\'s lease'); q.add_argument('--replace', action='store_true', help='supersede an existing record')
    q.add_argument('--append-evidence', action='store_true', help='flag: keep the existing record and append the --evidence text to its evidence (the name is updated if it differs)')
    q = ls.add_parser('add-function'); g = q.add_mutually_exclusive_group(required=True); g.add_argument('--json'); g.add_argument('--json-file'); q.add_argument('--force', action='store_true'); q.add_argument('--replace', action='store_true')
    q = ls.add_parser('add-fragment'); g = q.add_mutually_exclusive_group(required=True); g.add_argument('--json'); g.add_argument('--json-file'); q.add_argument('--force', action='store_true'); q.add_argument('--replace', action='store_true')
    p.set_defaults(func=ledger_cmd)
    args = parser.parse_args()
    with output_budget(argv=sys.argv[1:], full=getattr(args, 'full', False)):
        args.func(args)


if __name__ == '__main__':
    main()
