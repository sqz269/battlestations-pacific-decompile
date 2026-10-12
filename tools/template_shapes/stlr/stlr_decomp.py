"""Read-only: decompile the raw_pure / raw_mixed rows into local/stlr_decomp/<addr>.c (skips existing)."""
import json, os, sys, time
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))), 'tools'))
from ghidra_export import Client, ROOT
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
out = os.path.join(root, 'local', 'stlr_decomp')
os.makedirs(out, exist_ok=True)
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))
want = [x['a'] for x in d if x['b'] in sys.argv[1].split(',')]
client = Client(json.loads((ROOT / 'config/target.json').read_text()))
client.verify()
t0 = time.time()
n = 0
for a in want:
    p = os.path.join(out, a + '.c')
    if os.path.exists(p):
        continue
    try:
        txt = str(client.get('decompile_function', address=a, timeout=60))
    except Exception as e:  # noqa: BLE001
        txt = 'ERROR ' + str(e)[:200]
    open(p, 'w', encoding='utf-8').write(txt)
    n += 1
    if time.time() - t0 > float(sys.argv[2]):
        break
print(n, 'decompiled in', round(time.time() - t0), 's;', sum(1 for a in want if os.path.exists(os.path.join(out, a + '.c'))), 'of', len(want))
