import json,sys
from datetime import datetime,timezone
sys.path.insert(0,'tools')
from ghidra_tag import make_post, apply
from ghidra_export import Client, ROOT
from coordination import ghidra_lock
from ledger import load_tags
mine={r['address'] for r in json.load(open('local/st1_entries.json'))}
entries=[e for e in load_tags() if e['address'] in mine]
lo,hi=int(sys.argv[1]),int(sys.argv[2])
entries=entries[lo:hi]
client=Client(json.loads((ROOT/'config/target.json').read_text())); client.verify()
post=make_post(client)
stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
log=ROOT/'local'/f'st1-ghidra-tags-{lo}-{stamp}.json'
with ghidra_lock(ttl_seconds=7200,purpose='cc13-stl1 shape tags'):
  ch=apply(client,post,entries,log,True)
s={}
for r in ch: s[r['status']]=s.get(r['status'],0)+1
print(len(entries),s,log)
