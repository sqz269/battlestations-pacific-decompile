"""Reproduce the Oct24 identifier evidence inventory; candidates are not accepted mappings.

Usage: python tools/oct24_tail_identifiers.py --baseline local/output/oct24_wire_before.json
The small line-oriented scanner is deliberately conservative. Explicit maps in config/ are
reviewed separately and wire_readiness validates each mapped body with its own definition parser.
"""
import argparse
import collections
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ap = argparse.ArgumentParser()
ap.add_argument('--baseline', type=Path, required=True)
ap.add_argument('--out', type=Path, default=ROOT / 'local/output/oct24_identifier_candidates.json')
args = ap.parse_args()
rows = [r for r in json.loads(args.baseline.read_text())['rows'] if r['state'] == 'unknown_no_identifier']
address = re.compile(r'(?i)(?:0x|\b|_)(00[4-9a-f][0-9a-f]{5})(?:[uUlL]*\b)')
signature = re.compile(r'^\s*(?:template\s*<[^>{;]*>\s*)?(?:[\w:<>,*&~ \t]+?[ \t*&])?'
                       r'((?:(?:\w+(?:<[^;{}\n]*>)?)::)*(~?\w+))\s*\(')
keywords = {'if', 'for', 'while', 'switch', 'catch', 'return', 'static_assert', 'sizeof'}
sources = {}
for source in sorted({r['source'] for r in rows}):
    p = ROOT / source
    if not p.is_file():
        sources[source] = ([], [])
        continue
    lines = p.read_text(errors='replace').splitlines()
    defs = []
    if p.suffix in ('.cpp', '.hpp'):
        for i, line in enumerate(lines):
            m = signature.match(line)
            if not m or m[2] in keywords or line.lstrip().startswith(('//', '*', '#')):
                continue
            head = '\n'.join(lines[i:i + 8])
            brace, semi = head.find('{'), head.find(';')
            if brace >= 0 and (semi < 0 or brace < semi) and '=' not in head[:brace]:
                defs.append((i + 1, m[2], m[1]))
    sources[source] = (lines, defs)
inventory = []
for row in rows:
    lines, defs = sources[row['source']]
    occurrences = []
    for i, line in enumerate(lines, 1):
        if any(int(m[1], 16) == int(row['address'], 16) for m in address.finditer(line)):
            prior = [d for d in defs if d[0] <= i]
            occurrences.append(dict(line=i, text=line.strip()[:350], preceding_definition=prior[-1] if prior else None))
    inventory.append(dict(row, occurrences=occurrences, definitions=defs))
args.out.parent.mkdir(parents=True, exist_ok=True)
args.out.write_text(json.dumps(inventory, indent=1), encoding='utf-8')
print('unknown rows:', len(inventory), 'with source address:', sum(bool(r['occurrences']) for r in inventory))
print('by source:', collections.Counter(r['source'] for r in inventory).most_common(12))
print('wrote', args.out)
