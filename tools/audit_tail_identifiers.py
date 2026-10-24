"""Account for every member of the original Oct24 1,744-row identifier cohort.

First run: --baseline local/output/oct24_wire_before.json. Later runs can reuse
the baseline fields embedded in the committed JSONL audit. Run bsp.py index and
wire_readiness.py first. This script does not mutate source, ledgers or Ghidra.
"""
import argparse
import collections
import json
import re
import sqlite3
from pathlib import Path

import ledger
import progress_board as board

ROOT = Path(__file__).resolve().parents[1]
RESERVED = ((0x00728a90, 0x00735ec0), (0x008e3de0, 0x008ee5f0), (0x00996120, 0x009f6060))


def reserved(address):
    return any(lo <= address <= hi for lo, hi in RESERVED)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', type=Path)
    ap.add_argument('--wire', type=Path, default=ROOT / 'local/output/wire_readiness.json')
    ap.add_argument('--out', type=Path, default=ROOT / 'docs/OCT24_TAIL_IDENTIFIER_AUDIT.jsonl')
    args = ap.parse_args()
    if args.baseline:
        cohort = [dict(address=r['address'], baseline_name=r['name'], baseline_source=r['source'],
                       category=r['category']) for r in json.loads(args.baseline.read_text())['rows']
                  if r['state'] == 'unknown_no_identifier']
    else:
        cohort = [{k: r[k] for k in ('address', 'baseline_name', 'baseline_source', 'category')}
                  for r in (json.loads(line) for line in args.out.read_text().splitlines())]
    if len(cohort) != 1744 or len({r['address'] for r in cohort}) != 1744:
        raise ValueError('expected the exact unique 1744-row baseline cohort')
    wire = json.loads(args.wire.read_text())
    current = {r['address']: r for r in wire['rows']}
    db = sqlite3.connect(f'file:{board.INDEX.as_posix()}?mode=ro', uri=True)
    functions = {a: (name, tag, status) for a, name, tag, status in
                 db.execute('select address,name,tag_category,recon_status from functions')}
    records = collections.defaultdict(list)
    for r in ledger.load_reconstruction()['functions']:
        records[r['address']].append(r)
    mappings = {}
    for p in sorted((ROOT / 'config').glob('wire_identifiers*.json')):
        for r in json.loads(p.read_text())['identifiers']:
            a = int(r['address'], 16)
            if reserved(a):
                raise ValueError(f'mapping mutates a reserved address: {a:08x}')
            if a in mappings:
                raise ValueError(f'duplicate mapping {a:08x}')
            mappings[a] = r
    suffixes = collections.defaultdict(set)
    for m in re.finditer(r'\?{1,2}([A-Za-z_]\w*?_([0-9a-f]{8}))@',
                         (ROOT / 'build/win32/bsp_game.map').read_text(errors='replace')):
        suffixes[int(m[2], 16)].add(m[1])
    audit = []
    for old in sorted(cohort, key=lambda r: r['address']):
        r = dict(old)
        a = int(r['address'], 16)
        active = current.get(r['address'])
        recs = records[r['address']]
        r['ledger_statuses'] = sorted({x['status'] for x in recs})
        r['evidence'] = list(dict.fromkeys(x['oct24_body_audit'] for x in recs if x.get('oct24_body_audit')))
        if active and active['state'] != 'unknown_no_identifier':
            r['disposition'] = 'resolved_body'
            r['state'] = active['state']
            r['identifiers'] = active.get('ident') or sorted(suffixes.get(a, ()))
            r['ident_via'] = active.get('ident_via', 'address_suffix_or_link_map')
            r['implementation_source'] = active.get('implementation_source', active['source'])
            if a in mappings:
                r['mapping_evidence'] = mappings[a]['evidence']
                r['shared_implementation'] = mappings[a].get('shared_implementation', False)
        elif active:
            r['state'] = active['state']
            if reserved(a):
                r['disposition'] = 'reserved_cc13'
                r['evidence'].append('User ownership reservation: seg40/58 per docs/CC13_SEG40B.md, or seg67. No source, function ledger or Ghidra mutation by this lane.')
            elif a == 0x00957740:
                r['disposition'] = 'reserved_cc9'
                r['evidence'].append('Existing skipped_cc9_lane record:335-instruction x87 aim body; gunnery lane ownership retained. Host seat substitution does not establish this function body.')
            else:
                r['disposition'] = 'unresolved'
        elif recs and all(not board.reconstruction_has_body(x['status']) for x in recs):
            r['disposition'] = ('incomplete_or_unverified_credit_withdrawn'
                                if any(x['status'] == 'incomplete_or_unverified_body_credit_withdrawn_oct24' for x in recs)
                                else 'no_body_credit_withdrawn')
        else:
            name, tag, status = functions[a]
            if status in board.OUT_OF_SCOPE_STATUSES:
                r['disposition'] = 'classified_out_of_scope'
            elif name.startswith(board.FUNCLET + board.LIBRARY_NAME_PREFIXES) or not board.is_game(a, tag):
                r['disposition'] = 'existing_board_exclusion'
            else:
                r['disposition'] = 'unresolved'
            r['scope_evidence'] = dict(analysis_name=name, tag=tag, status=status)
        audit.append(r)
    summary = dict(cohort=len(audit), dispositions=dict(collections.Counter(r['disposition'] for r in audit)),
                   current_states=wire['summary'],
                   inputs=wire.get('inputs', {}),
                   by_category={cat: dict(collections.Counter(r['disposition'] for r in audit if r['category'] == cat))
                                for cat in sorted({r['category'] for r in audit})},
                   mapping_entries=len(mappings))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(''.join(json.dumps(r, ensure_ascii=False) + '\n' for r in audit), encoding='utf-8')
    out = ROOT / 'local/output/oct24_identifier_audit_summary.json'
    out.write_text(json.dumps(summary, indent=2) + '\n', encoding='utf-8')
    args.out.with_suffix('.meta.json').write_text(json.dumps(summary, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({k: v for k, v in summary.items() if k not in ('by_category', 'inputs')}, indent=2))
    print(args.out)
    if summary['dispositions'].get('unresolved'):
        raise SystemExit('unreserved rows still need review; see audit JSONL')


if __name__ == '__main__':
    main()
