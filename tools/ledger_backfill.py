"""Backfill reconstruction-ledger records for C++ bodies that have none.

A column-0 definition under src/*.cpp whose name ends in _<8 hex digits> that is a function entry in
the snapshot, and whose address has no ledger record of any kind, gets a record pointing at the
source line and the documents that mention the address. The record says in its status and evidence
that it was backfilled and re-verified nothing; the document remains the evidence of record.

    python tools/ledger_backfill.py            # dry run: counts and the first candidates
    python tools/ledger_backfill.py --apply    # claim a lease on the addresses and write the records
    python tools/ledger_backfill.py --fragment 00721a40 ...   # record these addresses as fragments

Writes go through ledger.upsert with the reconstruction key and coordination.check_writable, the
same path as `bsp.py ledger add-function`; addresses leased to another owner are skipped.
"""
import argparse
import datetime as dt
import glob
import json
import re
import sqlite3
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import ledger  # noqa: E402
import coordination  # noqa: E402

SIG = re.compile(r"^(?P<line>[A-Za-z_][\w:<>,*&\s\[\]]*?\s[*&]*(?P<name>~?[A-Za-z_][\w:]*(?:<[^()]*>)?)\s*\((?P<args>[^;{}]*)"
                 r"(?:\)\s*(?:const)?\s*(?:noexcept(?:\([^)]*\))?)?\s*(?:override|final)?\s*(?:\{.*)?|,)\s*)$")
SUFFIX = re.compile(r"_([0-9a-fA-F]{8})$")
KEYWORDS = {'return', 'if', 'for', 'while', 'switch', 'namespace', 'using', 'template', 'struct', 'class', 'enum',
            'typedef', 'static_assert', 'extern', 'else', 'case', 'default', 'do', 'try', 'catch', 'throw', 'delete',
            'new', 'sizeof'}


def load_index():
    path = ROOT / 'local/bsp_index.sqlite'
    if not path.exists():
        sys.exit('no index; run: python tools/bsp.py index')
    db = sqlite3.connect(f"file:{path.as_posix()}?mode=ro", uri=True)
    entries = {a: n for a, n in db.execute(
        "select address, name from functions where name not like 'Unwind@%' and name not like 'Catch_All@%' and name not like 'thunk_%'")}
    docs = {}
    for p, a in db.execute('select path, address from doc_addresses'):
        docs.setdefault(a, []).append(p.replace('\\', '/'))
    return entries, docs


def scan(entries):
    """address -> list of (file, line, name, comment) for column-0 definitions named after a function entry."""
    found = {}
    for path in sorted((ROOT / 'src').glob('*.cpp')):
        rel = path.relative_to(ROOT).as_posix()
        lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
        for i, line in enumerate(lines):
            if not line or line[0] in ' \t#/}{' or line.rstrip().endswith(';'):
                continue
            tok = re.match(r'[A-Za-z_]\w*', line)
            if not tok or tok.group(0) in KEYWORDS or line.startswith(('BSP_', 'TEST_', 'CATCH_')):
                continue
            m = SIG.match(line)
            if not m:
                continue
            name = m.group('name')
            ms = SUFFIX.search(name.rsplit('::', 1)[-1])
            if not ms:
                continue
            address = int(ms.group(1), 16)
            if address not in entries:
                continue
            block, j = [], i - 1
            while j >= 0 and (lines[j].startswith('//') or not lines[j].strip()) and len(block) < 12:
                if lines[j].startswith('//'):
                    block.insert(0, lines[j][2:].strip())
                j -= 1
            found.setdefault(address, []).append((rel, i + 1, name, ' '.join(block)))
    return found


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--apply', action='store_true')
    ap.add_argument('--fragment', nargs='*', default=[], help='addresses to record as fragments (partial bodies)')
    ap.add_argument('--ttl', type=float, default=3.0)
    ap.add_argument('--limit', type=int, default=15)
    args = ap.parse_args()
    fragments = {ledger.norm(a) for a in args.fragment}

    entries, docs = load_index()
    recon = ledger.load_reconstruction()
    have = {r['address'] for r in recon['functions']} | {r['address'] for r in recon['fragments']}
    owner = coordination.owner_name()
    cmake_text = '\n'.join(p.read_text(encoding='utf-8', errors='replace') for p in [ROOT / 'CMakeLists.txt'] + list((ROOT / 'cmake').glob('*.cmake')))

    todo, held = [], []
    for address, defs in sorted(scan(entries).items()):
        hex_addr = f'{address:08x}'
        if hex_addr in have:
            continue
        others = [l for l in coordination.holders(address) if l['owner'] != owner]
        if others:
            held.append((hex_addr, others[0]['id']))
            continue
        todo.append((hex_addr, defs))
    print(f'owner {owner}: {len(todo)} bodies without a ledger record, {len(held)} skipped as leased to others'
          + (': ' + ', '.join(f'{a} ({l})' for a, l in held[:5]) if held else ''))
    for hex_addr, defs in todo[:args.limit]:
        rel, line, name, _ = defs[0]
        print(f'  {hex_addr} {rel}:{line} {name}')
    if len(todo) > args.limit:
        print(f'  ... {len(todo) - args.limit} more')
    if not args.apply or not todo:
        return

    coordination.claim('ledger_backfill', addresses=[a for a, _ in todo], ttl_hours=args.ttl, owner=owner,
                       note='backfill ledger records for address-named C++ bodies without one')
    today = dt.date.today().isoformat()
    written = failed = 0
    for hex_addr, defs in todo:
        rel, line, name, comment = defs[0]
        kind = 'fragment' if hex_addr in fragments else 'function'
        doc_list = [d for d in docs.get(int(hex_addr, 16), []) if d != 'docs/GAME_EXECUTABLE.md'][:4]
        evidence = (f'C++ body at {rel}:{line} ({name}), a definition named after the address. '
                    f"Documents: {', '.join(doc_list) if doc_list else 'no document lists this address'}. "
                    f'Ledger record backfilled {today} from the source tree because the reconstructing commit recorded none; '
                    f'the document holds the ABI, coverage and validation evidence. The backfill re-verified nothing. '
                    f"Source file is {'listed in' if rel in cmake_text else 'not listed in'} the CMake build.")
        if comment.strip():
            evidence += ' Source comment: ' + re.sub(r'\s+', ' ', comment)[:240]
        record = {'address': hex_addr, 'name': name, 'source': rel,
                  'status': ('partial_projection' if kind == 'fragment' else 'reconstructed') + '_ledger_backfilled_from_source',
                  'evidence': evidence, 'cpp_symbol': name, 'abi_compatible': False, 'game_validated': False, 'kind': kind}
        if len(defs) > 1:
            record['notes'] = f'{len(defs)} column-0 definitions carry this address suffix in src/; this record points at the first.'
        try:
            coordination.check_writable([hex_addr], owner=owner)
            ledger.upsert(ledger.RECON_DIR, record, key=ledger.RECON_KEY)
            written += 1
        except Exception as exc:  # LeaseConflict or I/O
            failed += 1
            print(f'  failed {hex_addr}: {str(exc)[:120]}')
    print(f'written {written}, failed {failed}; lease {owner}:ledger_backfill (release it after committing)')


if __name__ == '__main__':
    main()
