"""Bounded lookups over a derived SQLite index of the reconstruction workspace.

`index` rebuilds local/bsp_index.sqlite (ignored, disposable) from the export snapshot,
the sharded ledgers, the inventory tags, the Capstone call graph and data references,
the candidate partition, the docs that mention addresses, and the disk PE strings.
Every query prints a short, capped result so a model never has to read a ledger or
directory whole. `ledger` subcommands append records to the right shard.

  python tools/bsp.py index
  python tools/bsp.py lookup 00ab9fd0
  python tools/bsp.py range 00ab0000 00ac0000 --only FUN_
  python tools/bsp.py callers 00ab9fd0 / callees 00ab9fd0 / docs-for 00ab9fd0 / segment 12 / find GuiManager
  python tools/bsp.py ledger add-name 00ab9fd0 BSP_Font_LayoutLine --evidence "..."
  python tools/bsp.py ledger add-function --json '{"address":"00ab9fd0","name":"...","source":"src/x.cpp","status":"..."}'
  python tools/bsp.py ledger migrate
"""
import argparse
import hashlib
import json
import re
import sqlite3
import sys
from datetime import datetime, timezone
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ledger  # noqa: E402

ROOT = ledger.ROOT
DB = ROOT / 'local/bsp_index.sqlite'
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


def build_index(args):
    DB.parent.mkdir(parents=True, exist_ok=True)
    if DB.exists():
        DB.unlink()
    db = sqlite3.connect(DB)
    db.executescript(SCHEMA)
    exports = ROOT / 'exports/bsp'
    functions_path = exports / 'functions.json'
    if not functions_path.exists():
        sys.exit('No exports/bsp/functions.json; run tools/ghidra_export.py snapshot first')
    functions = json.loads(functions_path.read_text(encoding='utf-8'))
    exported = {p.name for p in (exports / 'functions').glob('*') if (p / 'decompiled.c').exists()}
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
    callgraph = exports / 'callgraph.json'
    if callgraph.exists():
        edges = [(int(k, 16), int(v, 16)) for k, vs in json.loads(callgraph.read_text()).items() for v in vs]
        db.executemany('INSERT INTO calls VALUES (?,?)', edges)
    datarefs = exports / 'datarefs.json'
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
    db.commit()
    counts = {t: db.execute(f'SELECT COUNT(*) FROM {t}').fetchone()[0]
              for t in ('functions', 'calls', 'datarefs', 'segments', 'docs', 'doc_addresses')}
    print(f'built {DB.relative_to(ROOT).as_posix()}: {counts}')


def connect():
    if not DB.exists():
        sys.exit('No index; run: python tools/bsp.py index')
    db = sqlite3.connect(DB)
    db.row_factory = sqlite3.Row
    return db


def fn_label(db, address):
    row = db.execute('SELECT name FROM functions WHERE address=?', (address,)).fetchone()
    return f"{h(address)} {row['name'] if row else '?'}"


def lookup(args):
    db = connect()
    a = int(args.address, 16)
    f = db.execute('SELECT * FROM functions WHERE address=?', (a,)).fetchone()
    if f is None:
        near = db.execute('SELECT address, name FROM functions WHERE address<=? ORDER BY address DESC LIMIT 1', (a,)).fetchone()
        print(f'{h(a)}: no function starts here' + (f"; enclosing candidate {h(near['address'])} {near['name']}" if near else ''))
        return
    print(f"{f['hex']}  {f['name']}  thunk={'yes' if f['thunk'] else 'no'}  "
          f"export={'exports/bsp/functions/' + f['hex'] if f['exported'] else 'none'}")
    if f['segment'] is not None:
        s = db.execute('SELECT * FROM segments WHERE id=?', (f['segment'],)).fetchone()
        print(f"segment {s['id']} [{h(s['start'])}-{h(s['end'])}] {s['candidates']} candidates wave {s['wave']}  keywords: {s['keywords']}")
    if f['tag_category']:
        print(f"tag: {f['tag_category']} ({f['tag_confidence']}, {f['tag_action']}) {f['tag_name'] or ''}")
    if f['ledger_name']:
        print(f"ledger name: {f['ledger_name']} - {f['ledger_evidence'][:args.width]}")
    for r in db.execute('SELECT * FROM recon WHERE address=? ORDER BY kind, name', (a,)):
        print(f"reconstruction: {r['kind']} {r['name']} status={r['status']} source={r['source']}")
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


def ledger_cmd(args):
    if args.ledger_command == 'migrate':
        print(ledger.migrate(prune=not args.keep_legacy))
    elif args.ledger_command == 'add-name':
        previous = ledger.upsert(ledger.NAMES_DIR, {'address': args.address, 'name': args.name, 'evidence': args.evidence})
        print(f"{ledger.norm(args.address)} -> {args.name}" + (f" (replaced {previous['name']})" if previous else ''))
    elif args.ledger_command in ('add-function', 'add-fragment'):
        record = json.loads(args.json)
        for key in ('address', 'name', 'source', 'status'):
            if key not in record:
                sys.exit(f'missing field {key}')
        record['kind'] = 'function' if args.ledger_command == 'add-function' else 'fragment'
        previous = ledger.upsert(ledger.RECON_DIR, record, key=ledger.RECON_KEY)
        print(f"{ledger.norm(record['address'])} {record['kind']} {record['name']}" + (' (replaced)' if previous else ''))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('index').set_defaults(func=build_index)
    p = sub.add_parser('lookup'); p.add_argument('address'); p.add_argument('--limit', type=int, default=12); p.add_argument('--width', type=int, default=300); p.set_defaults(func=lookup)
    p = sub.add_parser('range'); p.add_argument('start'); p.add_argument('end'); p.add_argument('--only', help='name prefix filter, e.g. FUN_'); p.add_argument('--limit', type=int, default=40); p.set_defaults(func=range_query)
    p = sub.add_parser('callers'); p.add_argument('address'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=lambda a: calls_query(a, 'callers'))
    p = sub.add_parser('callees'); p.add_argument('address'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=lambda a: calls_query(a, 'callees'))
    p = sub.add_parser('docs-for'); p.add_argument('address'); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=docs_for)
    p = sub.add_parser('segment'); p.add_argument('id', type=int); p.add_argument('--limit', type=int, default=20); p.set_defaults(func=segment)
    p = sub.add_parser('find'); p.add_argument('text'); p.add_argument('--limit', type=int, default=25); p.set_defaults(func=find)
    p = sub.add_parser('ledger'); ls = p.add_subparsers(dest='ledger_command', required=True)
    q = ls.add_parser('migrate'); q.add_argument('--keep-legacy', action='store_true')
    q = ls.add_parser('add-name'); q.add_argument('address'); q.add_argument('name'); q.add_argument('--evidence', required=True)
    q = ls.add_parser('add-function'); q.add_argument('--json', required=True)
    q = ls.add_parser('add-fragment'); q.add_argument('--json', required=True)
    p.set_defaults(func=ledger_cmd)
    args = parser.parse_args()
    args.func(args)


if __name__ == '__main__':
    main()
