"""Audit progress-board rows outside the partition and its unreferenced set.

This is an evidence report, not a reconstruction pass. It uses the board's
actual game/reference predicates and source-history matcher, and preserves
metadata-only rows in the target so a status correction cannot hide a gap.
"""
import argparse
import collections
import json
from pathlib import Path
import sqlite3

import progress_board as board


METADATA = 'metadata_only_no_function_body_verified_oct24'


def source_history(starts, cache):
    tree = board.git('rev-parse', board.MAIN + ':src').strip()
    if cache.exists():
        saved = json.loads(cache.read_text(encoding='utf-8'))
        if saved.get('src_tree') == tree:
            return {int(a, 16) for a in saved['addresses']} & starts
    found = set(board.first_source_bodies(starts, board.Passes()))
    cache.parent.mkdir(parents=True, exist_ok=True)
    cache.write_text(json.dumps({'src_tree': tree, 'addresses': [f'{a:08x}' for a in sorted(found)]},
                               indent=2) + '\n', encoding='utf-8')
    return found


def disposition(row):
    status = row['recon_status'] or ''
    if status in (METADATA, 'unreferenced_dead_chain_recorded'):
        return 'recorded_only_no_body'
    if status.startswith('partial_'):
        return 'partial_body'
    if status.startswith('compiler_boundary_'):
        return 'compiler_boundary_projection'
    return 'existing_implementation_record'


def audit(index, cache):
    db = sqlite3.connect(f'file:{index.resolve().as_posix()}?mode=ro', uri=True)
    db.row_factory = sqlite3.Row
    functions = {r['address']: dict(r) for r in db.execute('select * from functions')
                 if not r['name'].startswith(board.FUNCLET)}
    starts = set(functions)
    source = source_history(starts, cache)
    records = list(db.execute("select address,status from recon where kind='function'"))
    ledger = {a for a, status in records if board.reconstruction_has_body(status)}
    audited_no_body = {a for a, status in records if not board.reconstruction_has_body(status)} - ledger
    union = (source | ledger) - audited_no_body
    referenced = board.referenced_functions(db, starts)
    game = {a for a, r in functions.items()
            if board.is_game(a, r['tag_category'])
            and not r['name'].startswith(board.LIBRARY_NAME_PREFIXES)
            and r['recon_status'] not in board.OUT_OF_SCOPE_STATUSES}
    target = game & (referenced | union)
    edge_counts = collections.defaultdict(collections.Counter)
    for r in db.execute('select callee, kind, count(*) from edges group by callee, kind'):
        edge_counts[r[0]][r[1]] = r[2]

    def describe(address):
        r = functions[address]
        return dict(address=r['hex'], name=r['name'], segment=r['segment'],
                    tag=r['tag_category'], status=r['recon_status'], source=r['recon_source'],
                    kind=r['recon_kind'], disposition=disposition(r),
                    historical_source_credit=address in source,
                    effective_source_credit=address in source and address not in audited_no_body,
                    function_ledger_credit=address in ledger,
                    board_referenced=address in referenced,
                    raw_edge_counts=dict(edge_counts[address]))

    outside = [describe(a) for a in sorted(target) if functions[a]['segment'] is None]
    unreferenced = [describe(a) for a in sorted(game - target)]
    outside_excluded = [describe(a) for a in sorted(game - target)
                        if functions[a]['segment'] is None]
    stats = dict(target=len(target), game_outside=len(outside), unreferenced=len(unreferenced),
                 outside_by_disposition=dict(collections.Counter(r['disposition'] for r in outside)),
                 outside_by_tag=dict(collections.Counter(r['tag'] or 'untagged' for r in outside)),
                 unreferenced_by_tag=dict(collections.Counter(r['tag'] or 'untagged' for r in unreferenced)),
                 unreferenced_by_segment=dict(collections.Counter(str(r['segment']) for r in unreferenced)),
                 unreferenced_by_edge_shape=dict(collections.Counter(
                     ','.join(sorted(r['raw_edge_counts'])) or 'none' for r in unreferenced)))
    return dict(head=board.git('rev-parse', board.MAIN).strip(),
                index_meta=dict(db.execute('select key, value from meta')), stats=stats,
                outside=outside, outside_excluded=outside_excluded, unreferenced=unreferenced)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--index', type=Path, default=board.INDEX)
    parser.add_argument('--source-cache', type=Path,
                        default=board.ROOT / 'local/output/oct24_source_history.json')
    parser.add_argument('--out', type=Path, default=board.ROOT / 'local/output/oct24_outside_audit.json')
    parser.add_argument('--board', type=Path, help='assert the baseline target/outside/unreferenced counts')
    args = parser.parse_args()
    result = audit(args.index, args.source_cache)
    if args.board:
        expected = json.loads(args.board.read_text(encoding='utf-8'))['kpi']
        for key in ('target', 'game_outside', 'unreferenced'):
            if result['stats'][key] != expected[key]:
                raise SystemExit(f"{key}: audit {result['stats'][key]} != board {expected[key]}")
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result['stats'], indent=2))
    print(args.out)


if __name__ == '__main__':
    main()
