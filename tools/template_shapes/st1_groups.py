"""Summarise st1_shapes.json: coarse groups, their sizes, and known-name matches."""
import collections
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
d = json.loads((ROOT / 'local/st1_shapes.json').read_text())
T = d['targets']
mode = sys.argv[1] if len(sys.argv) > 1 else 'summary'
key = 'coarse'
groups = collections.defaultdict(list)
for a, t in T.items():
    if t['tag'] or t['ledger'] or t['recon']:
        continue
    groups[t[key]].append(a)
kc = d['known_coarse']


def kinds(names):
    c = collections.Counter(n.split(':', 1)[1] for n in names)
    return ', '.join(f'{k}x{v}' for k, v in c.most_common(4))


ordered = sorted(groups.items(), key=lambda kv: -len(kv[1]))
if mode == 'summary':
    tot = sum(len(v) for v in groups.values())
    multi = sum(len(v) for v in groups.values() if len(v) > 1)
    matched = sum(len(v) for s, v in groups.items() if s in kc)
    print(f'untagged {tot}; in groups>1: {multi}; coarse-match known: {matched}; groups {len(groups)}')
    lo = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    hi = int(sys.argv[3]) if len(sys.argv) > 3 else 40
    for i, (s, v) in enumerate(ordered[lo:hi], lo):
        n = s.count('\n') + 1
        k = kinds(kc.get(s, []))
        print(f'#{i} n={len(v)} ins={n} ex={v[0]} known=[{k[:90]}]')
elif mode == 'show':
    i = int(sys.argv[2])
    s, v = ordered[i]
    print(f'#{i} n={len(v)} members: {" ".join(v[:30])}')
    print('known:', kinds(kc.get(s, []))[:300])
    print(T[v[0]]['fine'][:1500])
elif mode == 'dump':
    out = [{'i': i, 'members': v, 'known': kc.get(s, [])[:20], 'fine0': T[v[0]]['fine']} for i, (s, v) in enumerate(ordered)]
    (ROOT / 'local/st1_groups.json').write_text(json.dumps(out, indent=0))
