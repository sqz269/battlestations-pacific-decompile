"""Correct the five rows mislabelled STL_Tree_Assign (really map<int, map>::operator[]); tags + Ghidra under the lock."""
import json, sys
from datetime import datetime, timezone
sys.path.insert(0, 'tools')
from pathlib import Path
ADDR = ['00575e80', '00575fd0', '0062bb60', '0062c170', '0097efa0']
ROOT = Path('.').resolve()
for band in sorted({a[:4] + '0000' for a in ADDR}):
    p = ROOT / 'config/tags' / f'{band}.jsonl'
    out = []
    for line in p.read_text(encoding='utf-8').splitlines():
        r = json.loads(line)
        if r['address'] in ADDR and r['name'].startswith('STL_Tree_Assign_'):
            r['name'] = r['name'].replace('STL_Tree_Assign_', 'STL_Map_Subscript_')
            r['evidence_id'] = 'st1_mapsubscript'
        out.append(json.dumps(r))
    p.write_text('\n'.join(out) + '\n', encoding='utf-8', newline='\n')
if '--ghidra' in sys.argv:
    from ghidra_tag import make_post, apply
    from ghidra_export import Client
    from coordination import ghidra_lock
    from ledger import load_tags
    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    post = make_post(client)
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    log = []
    with ghidra_lock(ttl_seconds=1800, purpose='cc13-stlreview1 Tree_Assign -> Map_Subscript fix'):
        for a in ('00575e80', '00575fd0', '0097efa0'):
            before = client.get('get_function_by_address', address=a)
            post('rename_function_by_address', function_address=a, new_name=f'STL_Map_Subscript_{a}')
            log.append({'address': a, 'before': str(before)[:200], 'after': f'STL_Map_Subscript_{a}'})
        ent = [e for e in load_tags() if e['address'] in ADDR]
        ch = apply(client, post, ent, ROOT / 'local' / f'stlr-fix-assign-tags-{stamp}.json', False)
        post('save_program')
    (ROOT / 'local' / f'stlr-fix-assign-{stamp}.json').write_text(json.dumps(log, indent=1))
    print([(r['address'], r['status']) for r in ch])
