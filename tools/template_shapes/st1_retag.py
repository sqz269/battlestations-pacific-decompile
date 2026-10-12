"""Upgrade existing stl_probable bookmark rows to named shape tags (cc13-sweep6D).
Input file lines: addr|category|prefix|evidence_key. Replaces the row for each address in
config/tags/<band>.jsonl, adds new evidence keys, and writes local/st1_entries.json for st1_apply.py.
"""
import json, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
EV_NEW = {
    'st1_treeinserthint': None,  # existing
    'st1_iterinc': 'cc13-sweep6D shape: checked _Tree iterator: null-container and _Isnil hooks (_invalid_parameter_noinfo), then right-subtree _Min or parent climb (std::_Tree::const_iterator::_Inc / operator++)',
    'st1_iterdec': 'cc13-sweep6D shape: checked _Tree iterator: null-container hook, _Isnil -> rightmost, else left-subtree _Max or parent climb (std::_Tree::const_iterator::_Dec / operator--)',
    'st1_treeeraserange': 'cc13-sweep6D shape: erase(first,last): begin()/end() pair takes the _Erase(root)+reset fast path, otherwise loops erase(where) with checked-iterator hooks (std::_Tree::erase(first,last))',
    'st1_mapsubscript': 'cc13-sweep6D shape: _Lbound walk; on miss builds a default value (inner map head _Buynode) and calls insert(hint,pair); returns &node->value.second (std::map::operator[])',
}
# Usage: st1_retag.py <input> [--source DOC]; DOC defaults to the 006D doc that introduced these shapes.
args = sys.argv[1:]
source = 'docs/CC13_BAND_006D_006F.md'
if '--source' in args:
    i = args.index('--source')
    source = args[i + 1]
    del args[i:i + 2]
rows = [l.strip().split('|') for l in open(args[0]) if l.strip() and not l.startswith('#')]
want = {a: (cat, prefix, key) for a, cat, prefix, key in rows}
tagdir = ROOT / 'config/tags'
entries = []
for band in sorted({a[:4] + '0000' for a in want}):
    path = tagdir / f'{band}.jsonl'
    out = []
    for line in path.read_text(encoding='utf-8').splitlines():
        if not line.strip():
            continue
        r = json.loads(line)
        if r['address'] in want:
            cat, prefix, key = want.pop(r['address'])
            r = {'address': r['address'], 'name': f"{prefix}_{r['address']}", 'category': cat, 'confidence': 'high',
                 'action': 'rename', 'create': False, 'source': source, 'evidence_id': f'st1_{key}'}
            entries.append(r)
        out.append(json.dumps(r))
    path.write_text('\n'.join(out) + '\n', encoding='utf-8', newline='\n')
if want:
    print('no existing row for', sorted(want))
evp = tagdir / 'evidence.json'
ev = json.loads(evp.read_text(encoding='utf-8'))
for r in entries:
    k = r['evidence_id']
    if k not in ev:
        ev[k] = EV_NEW[k]
evp.write_text(json.dumps(ev, indent=1, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')
(ROOT / 'local/st1_entries.json').write_text(json.dumps(entries))
print(len(entries), 'rows upgraded')
