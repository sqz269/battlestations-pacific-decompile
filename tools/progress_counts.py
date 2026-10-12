"""Print reconstruction progress counts from the local index (local/bsp_index.sqlite).

Definitions follow the cc13 band docs:
  raw         snapshot name starts with FUN_, no tag row (tag_category empty) and no ledger name
  bookmarked  snapshot name FUN_ whose only tag row is an stl_probable bookmark, no ledger name
  ledger      a ledger name (config/names) is recorded for the address
  tagged      a tag row (config/tags) exists for the address
  named       ledger name, or a snapshot name that is not a Ghidra default (FUN_/thunk_FUN_)
  recon       a reconstruction record exists (config/reconstruction)

The index holds one tag row per function (functions.tag_*), so a function counts as tagged
once. Counts reflect the index as built; run `python tools/bsp.py index --if-stale` first.

Usage:
  python tools/progress_counts.py                      whole image
  python tools/progress_counts.py --range 00670000-0073FFFF
  python tools/progress_counts.py --by-band [--range lo-hi]   one row per 64 KB band
"""
import argparse
import os
import sqlite3
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_DB = os.path.join(ROOT, 'local', 'bsp_index.sqlite')

COLUMNS = ('total', 'raw', 'bookmarked', 'fun_named', 'named', 'ledger', 'tagged', 'recon', 'thunk')

QUERY = """
SELECT {group}
  COUNT(*),
  SUM(fun AND NOT tagged AND NOT ledger),
  SUM(fun AND NOT ledger AND tag_category = 'stl_probable'),
  SUM(fun),
  SUM(ledger OR NOT fun),
  SUM(ledger),
  SUM(tagged),
  SUM(recon),
  SUM(thunk)
FROM (
  SELECT address,
         (name LIKE 'FUN\\_%' ESCAPE '\\' OR name LIKE 'thunk\\_FUN\\_%' ESCAPE '\\') AS fun,
         (COALESCE(tag_category, '') <> '') AS tagged,
         (COALESCE(ledger_name, '') <> '') AS ledger,
         (COALESCE(recon_kind, '') <> '' OR COALESCE(recon_name, '') <> '') AS recon,
         COALESCE(thunk, 0) <> 0 AS thunk,
         tag_category
  FROM functions
  WHERE address BETWEEN ? AND ?
) {tail}
"""


def parse_range(text):
    lo, _, hi = text.partition('-')
    return int(lo, 16), int(hi or lo, 16)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--db', default=DEFAULT_DB)
    ap.add_argument('--range', default='00000000-FFFFFFFF', help='hex lo-hi, inclusive')
    ap.add_argument('--by-band', action='store_true', help='group by 64 KB band')
    args = ap.parse_args()
    if not os.path.exists(args.db):
        sys.exit('no index at %s: run python tools/bsp.py index' % args.db)
    lo, hi = parse_range(args.range)
    con = sqlite3.connect(args.db)
    built = dict(con.execute('SELECT key, value FROM meta').fetchall())
    if args.by_band:
        sql = QUERY.format(group='address >> 16,', tail='GROUP BY address >> 16 ORDER BY 1')
    else:
        sql = QUERY.format(group='', tail='')
    rows = con.execute(sql, (lo, hi)).fetchall()
    stamp = built.get('snapshot') or built.get('built') or ''
    print('index %s  range %08X-%08X%s' % (stamp, lo, hi, '' if not built else ''))
    head = ('band     ' if args.by_band else '') + ' '.join('%10s' % c for c in COLUMNS)
    print(head)
    for row in rows:
        lead = ''
        if args.by_band:
            lead = '%04X     ' % row[0]
            row = row[1:]
        print(lead + ' '.join('%10d' % (v or 0) for v in row))


if __name__ == '__main__':
    main()
