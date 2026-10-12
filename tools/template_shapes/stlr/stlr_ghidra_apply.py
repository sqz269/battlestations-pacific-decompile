"""cc13-stlreview1 Ghidra step (under the write lock): rename/bookmark upgraded rows through ghidra_tag.apply,
then delete the 'Inventory: stl_probable' bookmark on every reviewed/upgraded/dropped row. Logs to local/."""
import json, sys
from datetime import datetime, timezone
sys.path.insert(0, 'tools')
from ghidra_tag import make_post, apply
from ghidra_export import Client, ROOT
from coordination import ghidra_lock
from ledger import load_tags

rows = json.load(open('local/stlr_ghidra.json'))
mine = {g['address'] for g in rows if g['decision'] in ('upgrade_shape', 'upgrade_named')}
entries = [e for e in load_tags() if e['address'] in mine]
client = Client(json.loads((ROOT / 'config/target.json').read_text()))
client.verify()
post = make_post(client)
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
log = ROOT / 'local' / f'stlr-ghidra-tags-{stamp}.json'
dlog = ROOT / 'local' / f'stlr-ghidra-unbookmark-{stamp}.json'
old_ev = json.loads((ROOT / 'config/tags/evidence.json').read_text(encoding='utf-8'))
done = []
with ghidra_lock(ttl_seconds=7200, purpose='cc13-stlreview stl_probable review'):
    ch = apply(client, post, entries, log, False)
    for i, g in enumerate(rows):
        rec = {'address': g['address'], 'decision': g['decision'], 'deleted_category': 'Inventory: stl_probable'}
        try:
            post('delete_bookmark', address=g['address'], category='Inventory: stl_probable')
            rec['status'] = 'deleted'
        except Exception as exc:  # noqa: BLE001
            rec['status'] = 'error'
            rec['error'] = str(exc)[:300]
        done.append(rec)
        if i % 200 == 0:
            print(i, g['address'], rec['status'], flush=True)
    post('save_program')
dlog.write_text(json.dumps({'note': 'previous bookmark comments were "[medium] " + evidence e0029 or st1_scl97 text', 'rows': done}, indent=0))
s, t = {}, {}
for r in ch:
    s[r['status']] = s.get(r['status'], 0) + 1
for r in done:
    t[r['status']] = t.get(r['status'], 0) + 1
print('tags', len(entries), s, 'unbookmark', t, log, dlog)
