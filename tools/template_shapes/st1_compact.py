import json,collections,sys
d=json.load(open('local/st1_shapes.json'))
T=d['targets'];g=collections.defaultdict(list)
for a,t in T.items():
  if t['tag'] or t['ledger'] or t['recon']: continue
  g[t['coarse']].append(a)
o=sorted(g.items(),key=lambda kv:-len(kv[1]))
kc=d['known_coarse']
lo,hi=int(sys.argv[1]),int(sys.argv[2]); w=int(sys.argv[3]) if len(sys.argv)>3 else 500
for i in range(lo,hi):
  s,v=o[i]
  k=collections.Counter(n.split(':',1)[1] for n in kc.get(s,[]))
  fines=collections.Counter(T[a]['fine'] for a in v)
  print(f"#{i} n={len(v)} fine={len(fines)} ex={v[0]} known={list(k)[:3]}")
  print('   ', T[v[0]]['fine'].replace('\n',' ; ').replace('dword ptr ','')[:w])
