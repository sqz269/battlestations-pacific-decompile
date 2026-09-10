"""Bounded lookups and live queries for the reconstruction workspace.

Everything here prints a short, capped result so a model never has to read a ledger, an
export, or a directory whole. `index` rebuilds local/bsp_index.sqlite (ignored, disposable)
from the snapshot, sharded ledgers, tags, call graph, partition, PE strings and docs.

  python tools/bsp.py state                      one-screen orientation card (start a turn here)
  python tools/bsp.py lookup 00ab9fd0            everything known about one address
  python tools/bsp.py show 00ab9fd0 [--asm] [--lines 80] [--start 0]   capped export excerpt
  python tools/bsp.py range 00ab0000 00ac0000 --only FUN_
  python tools/bsp.py callers|callees|docs-for 00ab9fd0 / segment 12 / find GuiManager
  python tools/bsp.py ghidra count|proto|flow|xrefs|callers|callees|bytes|comments|decompile|disasm|export ...
  python tools/bsp.py snapshot [--force]         snapshot + index only if Ghidra's function count changed
  python tools/bsp.py index [--if-stale]
  python tools/bsp.py ledger add-name|add-function|add-fragment|migrate ...
"""
import argparse
import hashlib
import json
import re
import sqlite3
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import coordination  # noqa: E402
import ledger  # noqa: E402
import workspace  # noqa: E402

ROOT = ledger.ROOT
DB = ROOT / 'local/bsp_index.sqlite'
EXPORTS = workspace.exports_dir()  # main checkout's exports/bsp, shared by every worktree
ADDR = re.compile(r'\b(00[4-9a-c][0-9a-f]{5})\b')

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
            for match in re.finditer(rb'[\x20-\x7e]{5,}\x00', data):
                strings[va + match.start()] = match.group()[:-1].decode('ascii', errors='ignore')
    return strings


def inputs_digest():
    """Digest of everything the index is built from, to detect staleness cheaply."""
    digest = hashlib.sha256()
    for path in [EXPORTS / 'functions.json', EXPORTS / 'callgraph.json', EXPORTS / 'datarefs.json',
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
    if DB.exists():
        DB.unlink()
    db = sqlite3.connect(DB)
    db.executescript(SCHEMA)
    functions_path = EXPORTS / 'functions.json'
    if not functions_path.exists():
        sys.exit('No exports/bsp/functions.json; run tools/ghidra_export.py snapshot first')
    functions = json.loads(functions_path.read_text(encoding='utf-8'))
    exported = {p.name for p in (EXPORTS / 'functions').glob('*') if (p / 'decompiled.c').exists()}
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
    if callgraph.exists():
        edges = [(int(k, 16), int(v, 16)) for k, vs in json.loads(callgraph.read_text()).items() for v in vs]
        db.executemany('INSERT INTO calls VALUES (?,?)', edges)
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
                       [(rel, int(a, 16)) for a in sorted(set(ADDR.findall(text)))])
    db.execute('INSERT INTO meta VALUES (?,?)', ('built_utc', datetime.now(timezone.utc).isoformat()))
    db.execute('INSERT INTO meta VALUES (?,?)', ('functions_sha256', hashlib.sha256(functions_path.read_bytes()).hexdigest()))
    db.execute('INSERT INTO meta VALUES (?,?)', ('inputs_sha256', inputs_digest()))
    db.commit()
    counts = {t: db.execute(f'SELECT COUNT(*) FROM {t}').fetchone()[0]
              for t in ('functions', 'calls', 'datarefs', 'segments', 'docs', 'doc_addresses')}
    print(f'built {DB.relative_to(ROOT).as_posix()}: {counts}')


# ---------------------------------------------------------------- index queries

def connect(required=True):
    if not DB.exists():
        if required:
            sys.exit('No index; run: python tools/bsp.py index')
        return None
    db = sqlite3.connect(DB)
    db.row_factory = sqlite3.Row
    return db


def fn_label(db, address):
    row = db.execute('SELECT name FROM functions WHERE address=?', (address,)).fetchone() if db else None
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
    print('\n'.join(lines))
    if f is None:
        return
    callers = [r[0] for r in db.execute('SELECT caller FROM calls WHERE callee=? ORDER BY caller', (a,))]
    callees = [r[0] for r in db.execute('SELECT callee FROM calls WHERE caller=? ORDER BY callee', (a,))]
    print(f"callers ({len(callers)}): " + ', '.join(fn_label(db, c) for c in callers[:args.limit]) + (' ...' if len(callers) > args.limit else ''))
    print(f"callees ({len(callees)}): " + ', '.join(fn_label(db, c) for c in callees[:args.limit]) + (' ...' if len(callees) > args.limit else ''))
    strings = [r['text'] for r in db.execute('SELECT text FROM datarefs WHERE func=? AND text IS NOT NULL ORDER BY ref', (a,))]
    if strings:
        print(f"strings ({len(strings)}): " + ' | '.join(s[:60] for s in strings[:args.limit]) + (' ...' if len(strings) > args.limit else ''))
    docs = db.execute('SELECT d.path, d.title FROM doc_addresses da JOIN docs d ON d.path=da.path WHERE da.address=? ORDER BY d.path', (a,)).fetchall()
    if docs:
        print(f"docs ({len(docs)}): " + '; '.join(f"{d['path']} ({d['title'][:50]})" for d in docs[:args.limit]))


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
        print(f'--- {path.relative_to(ROOT).as_posix()} ---')
    elif args.live:
        text = str(client().get('disassemble_function' if args.asm else 'decompile_function', address=h(a)))
        print(f'--- live {"disassembly" if args.asm else "decompile"} (not exported) ---')
    else:
        print(f'not exported; run: python tools/bsp.py ghidra export {h(a)}   (or --live for an unsaved view)')
        return
    cap(text, args.lines, args.start)
    if db and not args.asm and args.start == 0:
        callees = [r[0] for r in db.execute('SELECT callee FROM calls WHERE caller=? ORDER BY callee', (a,))]
        if callees:
            print(f"callees ({len(callees)}): " + ', '.join(fn_label(db, c) for c in callees[:12]) + (' ...' if len(callees) > 12 else ''))


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
        extra = f['tag_category'] or f['recon_status'] or ''
        print(f"{f['hex']} {f['name']:<44} seg={f['segment'] if f['segment'] is not None else '-':<4} {extra}")
    if total > args.limit:
        print(f'... {total - args.limit} more (raise --limit)')


def calls_query(args, direction):
    db = connect()
    a = int(args.address, 16)
    col, other = ('callee', 'caller') if direction == 'callers' else ('caller', 'callee')
    rows = [r[0] for r in db.execute(f'SELECT {other} FROM calls WHERE {col}=? ORDER BY {other}', (a,))]
    print(f'{direction} of {fn_label(db, a)} ({len(rows)}):')
    for r in rows[:args.limit]:
        f = db.execute('SELECT * FROM functions WHERE address=?', (r,)).fetchone()
        print(f"  {h(r)} {f['name'] if f else '?':<44} seg={f['segment'] if f and f['segment'] is not None else '-'} {(f['tag_category'] or f['recon_status'] or '') if f else ''}")
    if len(rows) > args.limit:
        print(f'  ... {len(rows) - args.limit} more')


def docs_for(args):
    db = connect()
    a = int(args.address, 16)
    docs = db.execute('SELECT d.path, d.title FROM doc_addresses da JOIN docs d ON d.path=da.path WHERE da.address=? ORDER BY d.path', (a,)).fetchall()
    print(f'docs mentioning {h(a)} ({len(docs)}):')
    for d in docs[:args.limit]:
        print(f"  {d['path']}  {d['title']}")


def segment(args):
    db = connect()
    s = db.execute('SELECT * FROM segments WHERE id=?', (args.id,)).fetchone()
    if s is None:
        sys.exit(f'no segment {args.id}')
    print(f"segment {s['id']} [{h(s['start'])}-{h(s['end'])}] candidates={s['candidates']} purity={s['purity']} wave={s['wave']} "
          f"lua_bindings={s['lua_bindings']} vtables={s['vtables']}\nkeywords: {s['keywords']}")
    done = db.execute('SELECT COUNT(*) FROM functions WHERE segment=? AND recon_kind IS NOT NULL', (s['id'],)).fetchone()[0]
    named = db.execute("SELECT COUNT(*) FROM functions WHERE segment=? AND ledger_name IS NOT NULL", (s['id'],)).fetchone()[0]
    print(f'reconstructed here: {done}; reviewed names here: {named}')
    for f in db.execute("SELECT * FROM functions WHERE segment=? AND name LIKE 'FUN_%' ORDER BY address LIMIT ?", (s['id'], args.limit)):
        print(f"  {f['hex']} {f['name']}")


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
        funclets = db.execute("SELECT COUNT(*) FROM functions WHERE name LIKE 'Unwind@%' OR name LIKE 'Catch_All@%'").fetchone()[0]
        exported = db.execute('SELECT COUNT(*) FROM functions WHERE exported=1').fetchone()[0]
        print(f"functions: {fun} untagged FUN_, {funclets} EH funclets, {exported} exported")
        meta = index_meta()
        fresh = meta.get('inputs_sha256') == inputs_digest()
        print(f"index: built {meta.get('built_utc', '?')[:19]}  {'fresh' if fresh else 'STALE -> python tools/bsp.py index'}")
    else:
        print('index: none -> python tools/bsp.py index')
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
    c = Client(json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8')))
    c.verify()
    return c


def as_text(result):
    return result if isinstance(result, str) else json.dumps(result, indent=1)


def first_int(text):
    m = re.search(r'\d+', str(text))
    return int(m.group()) if m else None


def ghidra_cmd(args):
    c = client()
    sub = args.ghidra_command
    db = connect(required=False)
    if sub == 'count':
        live = first_int(c.get('get_function_count'))
        snap = json.loads((EXPORTS / 'snapshot.json').read_text())['total_function_count'] if (EXPORTS / 'snapshot.json').exists() else None
        print(f'live function count {live}; snapshot {snap}; {"changed" if live != snap else "same"}')
    elif sub == 'proto':
        a = norm(args.address)
        cap(as_text(c.get('get_function_by_address', address=a)), 6)
        try:
            cap(as_text(c.get('get_function_signature', address=a)), args.lines)
        except RuntimeError as exc:
            print(f'(signature unavailable: {str(exc)[:120]})')
    elif sub == 'flow':
        # The installed bridge exposes the previous override in its dry-run
        # result. dry_run MUST be a query parameter (never a JSON body field).
        # This command cannot apply a repair.
        from urllib.parse import urlencode
        from urllib.request import Request, urlopen
        rows = []
        for value in args.addresses:
            address = norm(value)
            request = Request(c.config['ghidra_url'] + '/clear_instruction_flow_override?' +
                urlencode({'program': c.config['program'], 'dry_run': 'true'}),
                data=json.dumps({'address': address}).encode(),
                headers={'Content-Type': 'application/json'}, method='POST')
            with urlopen(request, timeout=90) as response:
                result = json.loads(response.read())
            if result.get('error') or result.get('dry_run') is not True:
                sys.exit(f'Flow inspection did not return a confirmed dry run: {result}')
            rows.append({'address': address, 'result': result})
        cap(as_text(rows), args.lines)
    elif sub == 'comments':
        # Annotation readback often spans a batch. Persist complete records in
        # ignored local storage while keeping the interactive view bounded.
        path = None
        if args.output:
            path = (ROOT / args.output).resolve()
            if not path.is_relative_to((ROOT / 'local').resolve()) or path.suffix != '.json':
                sys.exit('Comment output must be a JSON file under local/.')
        rows = [{'address': norm(a), 'annotation': c.get('get_plate_comment', address=norm(a))}
                for a in args.addresses]
        if path:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps(rows, indent=2) + '\n', encoding='utf-8', newline='\n')
            print(f'Saved {len(rows)} comment records to {path.relative_to(ROOT).as_posix()}')
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
        result = c.get(endpoint, address=a)
        cap(as_text(result), args.lines, args.start)
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
            print(f"{a}: {'exported ' + str(path.relative_to(ROOT).as_posix()) if path.exists() else 'FAILED'}  -> python tools/bsp.py show {a}")
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
        coordination.check_writable([ledger.norm(args.address)], force=args.force)
        previous = ledger.upsert(ledger.NAMES_DIR, {'address': args.address, 'name': args.name, 'evidence': args.evidence})
        print(f"{ledger.norm(args.address)} -> {args.name}" + (f" (replaced {previous['name']})" if previous else ''))
    elif args.ledger_command in ('add-function', 'add-fragment'):
        record = json.loads(args.json)
        for key in ('address', 'name', 'source', 'status'):
            if key not in record:
                sys.exit(f'missing field {key}')
        coordination.check_writable([ledger.norm(record['address'])], force=args.force)
        record['kind'] = 'function' if args.ledger_command == 'add-function' else 'fragment'
        previous = ledger.upsert(ledger.RECON_DIR, record, key=ledger.RECON_KEY)
        print(f"{ledger.norm(record['address'])} {record['kind']} {record['name']}" + (' (replaced)' if previous else ''))


# ---------------------------------------------------------------- coordination

def load_packets():
    path = ROOT / 'config/parallel_work.json'
    return (json.loads(path.read_text(encoding='utf-8')) if path.exists() else {}), path


def packet_done(packet):
    text = str(packet.get('state') or packet.get('status') or '')
    return bool(packet.get('done')) or 'integrated' in text or text in ('done', 'complete', 'completed')


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
            lease = coordination.claim(args.packet, addresses, ranges, files, ttl_hours=args.ttl, owner=args.owner, note=args.note or '')
        except coordination.LeaseConflict as exc:
            sys.exit(f'lease refused: {exc}')
        print(f"leased {lease['id']}: {len(lease['addresses'])} addresses, {len(lease['ranges'])} ranges, "
              f"{len(lease['files'])} files until {lease['expires']}")
    elif sub == 'release':
        released = coordination.release(args.id, packet=args.packet, owner=args.owner)
        print(f"released {[r['id'] for r in released]}" if released else 'nothing active to release')
    elif sub == 'list':
        rows = coordination.active_leases() if not args.all else coordination.load_leases()
        for l in rows:
            print(f"{l['id']:<40} {l.get('status', ''):<9} until {l['expires'][:16]}  {len(l.get('addresses', []))} addrs "
                  f"{l.get('ranges', [])} files={l.get('files', [])} worktree={l.get('worktree', '')}")
        if not rows:
            print('no leases')
    elif sub == 'check':
        a = int(args.address, 16)
        holders = coordination.holders(a)
        print(f"{h(a)}: " + ('; '.join(f"{l['id']} until {l['expires'][:16]}" for l in holders) if holders else 'unleased'))
    elif sub == 'lock-status':
        lock = coordination.lock_status()
        print(json.dumps(lock) if lock else 'ghidra lock free')


def packets_cmd(args):
    work, path = load_packets()
    packets = work.get('packets', [])
    by_id = {p.get('id'): p for p in packets}
    leases = coordination.active_leases()
    leased = {l['packet']: l['owner'] for l in leases}
    if args.packets_command in ('list', 'ready'):
        for p in packets:
            pid = p.get('id')
            deps = p.get('depends_on', [])
            missing = [d for d in deps if not (d in by_id and packet_done(by_id[d]))]
            done = packet_done(p)
            ready = not done and not missing and pid not in leased
            if args.packets_command == 'ready' and not ready:
                continue
            flag = 'done' if done else ('leased:' + leased[pid] if pid in leased else ('blocked:' + ','.join(missing) if missing else 'ready'))
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
    if args.worktree_command == 'remove':
        path = (main.parent / f'{main.name}-{args.name}').resolve()
        link = path / 'exports'
        # Older worktrees carried a junction to the shared exports; git would traverse it and
        # empty the target, so detach the reparse point first (rmdir removes only the link).
        if link.exists() and link.is_dir():
            probe = subprocess.run(['cmd', '/c', 'fsutil', 'reparsepoint', 'query', str(link)], capture_output=True, text=True)
            if probe.returncode == 0:
                subprocess.run(['cmd', '/c', 'rmdir', str(link)], capture_output=True, text=True)
                print(f'detached exports junction in {path}')
        run = subprocess.run(['git', 'worktree', 'remove', '--force', str(path)], cwd=main, capture_output=True, text=True)
        print((run.stdout + run.stderr).strip() or f'removed {path}')
        if args.delete_branch and run.returncode == 0:
            print(git('branch', '-D', f'agent/{args.name}'))
        return
    name = args.name
    branch = f'agent/{name}'
    path = (main.parent / f'{main.name}-{name}').resolve()
    if path.exists():
        sys.exit(f'{path} already exists')
    run = subprocess.run(['git', 'worktree', 'add', str(path), '-b', branch, args.base], cwd=main, capture_output=True, text=True)
    if run.returncode:
        sys.exit((run.stdout + run.stderr).strip())
    print(f"worktree {path} on branch {branch}")
    print(f"exports resolve to {main / 'exports' / 'bsp'} through git (no junction; never create one inside a worktree)")
    print(f"next: cd {path}; python tools/bsp.py index; python tools/bsp.py lease claim --packet <id> --from-packet"
          + (f" (suggested packet: {args.packet})" if args.packet else ''))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('index'); p.add_argument('--if-stale', action='store_true'); p.set_defaults(func=build_index)
    p = sub.add_parser('state'); p.add_argument('--limit', type=int, default=8); p.set_defaults(func=state)
    p = sub.add_parser('lookup'); p.add_argument('address'); p.add_argument('--limit', type=int, default=12); p.add_argument('--width', type=int, default=300); p.set_defaults(func=lookup)
    p = sub.add_parser('show'); p.add_argument('address'); p.add_argument('--asm', action='store_true'); p.add_argument('--live', action='store_true')
    p.add_argument('--lines', type=int, default=80); p.add_argument('--start', type=int, default=0); p.set_defaults(func=show)
    p = sub.add_parser('range'); p.add_argument('start'); p.add_argument('end'); p.add_argument('--only', help='name prefix filter, e.g. FUN_'); p.add_argument('--limit', type=int, default=40); p.set_defaults(func=range_query)
    p = sub.add_parser('callers'); p.add_argument('address'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=lambda a: calls_query(a, 'callers'))
    p = sub.add_parser('callees'); p.add_argument('address'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=lambda a: calls_query(a, 'callees'))
    p = sub.add_parser('docs-for'); p.add_argument('address'); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=docs_for)
    p = sub.add_parser('segment'); p.add_argument('id', type=int); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=segment)
    p = sub.add_parser('find'); p.add_argument('text'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=find)
    p = sub.add_parser('snapshot'); p.add_argument('--force', action='store_true'); p.set_defaults(func=snapshot)
    p = sub.add_parser('ghidra', help='live, capped Ghidra queries through the loopback client'); gs = p.add_subparsers(dest='ghidra_command', required=True)
    gs.add_parser('count')
    q = gs.add_parser('proto'); q.add_argument('address'); q.add_argument('--lines', type=int, default=20)
    q = gs.add_parser('flow'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', type=int, default=40)
    q = gs.add_parser('comments'); q.add_argument('addresses', nargs='+'); q.add_argument('--lines', type=int, default=40); q.add_argument('--start', type=int, default=0); q.add_argument('--output')
    for name in ('xrefs', 'callers', 'callees'):
        q = gs.add_parser(name); q.add_argument('address'); q.add_argument('--limit', type=int, default=25); q.add_argument('--lines', type=int, default=40)
    q = gs.add_parser('bytes'); q.add_argument('address'); q.add_argument('--length', type=int, default=64)
    for name in ('decompile', 'disasm'):
        q = gs.add_parser(name); q.add_argument('address'); q.add_argument('--lines', type=int, default=80); q.add_argument('--start', type=int, default=0)
        if name == 'decompile':
            q.add_argument('--force', action='store_true', help='flush Ghidra decompiler cache before reading')
    q = gs.add_parser('export'); q.add_argument('addresses', nargs='+'); q.add_argument('--force', action='store_true')
    p.set_defaults(func=ghidra_cmd)
    p = sub.add_parser('lease', help='address/file leases shared across worktrees and harnesses'); lz = p.add_subparsers(dest='lease_command', required=True)
    q = lz.add_parser('claim'); q.add_argument('--packet', required=True); q.add_argument('--owner'); q.add_argument('--addresses', nargs='*')
    q.add_argument('--ranges', nargs='*', help='lo-hi hex pairs'); q.add_argument('--files', nargs='*'); q.add_argument('--from-packet', action='store_true')
    q.add_argument('--ttl', type=float, default=8.0, help='hours'); q.add_argument('--note')
    q = lz.add_parser('release'); q.add_argument('id', nargs='?'); q.add_argument('--packet'); q.add_argument('--owner')
    q = lz.add_parser('list'); q.add_argument('--all', action='store_true')
    q = lz.add_parser('check'); q.add_argument('address')
    lz.add_parser('lock-status')
    p.set_defaults(func=lease_cmd)
    p = sub.add_parser('packets', help='work packets from config/parallel_work.json with dependencies and leases'); pk = p.add_subparsers(dest='packets_command', required=True)
    pk.add_parser('list'); pk.add_parser('ready')
    q = pk.add_parser('done'); q.add_argument('id')
    q = pk.add_parser('depend'); q.add_argument('id'); q.add_argument('--on', nargs='+', required=True)
    p.set_defaults(func=packets_cmd)
    p = sub.add_parser('worktree', help='one git worktree per harness agent, sharing exports'); wt = p.add_subparsers(dest='worktree_command', required=True)
    q = wt.add_parser('add'); q.add_argument('name'); q.add_argument('--base', default='main'); q.add_argument('--packet')
    q = wt.add_parser('remove', help='safe removal: detaches any exports junction before git worktree remove'); q.add_argument('name'); q.add_argument('--delete-branch', action='store_true')
    wt.add_parser('list')
    p.set_defaults(func=worktree_cmd)
    p = sub.add_parser('ledger'); ls = p.add_subparsers(dest='ledger_command', required=True)
    q = ls.add_parser('migrate'); q.add_argument('--keep-legacy', action='store_true')
    q = ls.add_parser('add-name'); q.add_argument('address'); q.add_argument('name'); q.add_argument('--evidence', required=True); q.add_argument('--force', action='store_true')
    q = ls.add_parser('add-function'); q.add_argument('--json', required=True); q.add_argument('--force', action='store_true')
    q = ls.add_parser('add-fragment'); q.add_argument('--json', required=True); q.add_argument('--force', action='store_true')
    p.set_defaults(func=ledger_cmd)
    args = parser.parse_args()
    args.func(args)


if __name__ == '__main__':
    main()
