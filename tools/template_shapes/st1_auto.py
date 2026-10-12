import json,collections,re
d=json.load(open('local/st1_shapes.json'));T=d['targets'];kc=d['known_coarse']
g=collections.defaultdict(list)
for a,t in T.items():
  if t['tag'] or t['ledger'] or t['recon']: continue
  g[t['coarse']].append(a)
dec={l.split('|')[0] for l in open('tools/template_shapes/st1_decisions.txt') if l[0]!='#'}
decsig={T[e]['coarse'] for e in dec if e in T}
out=[]
for s,v in g.items():
  if s in decsig or s not in kc: continue
  names=[n.split(':',1)[1] for n in kc[s]]
  if all(re.match(r'(STL_|CG_|TRIV_body|LIBCRT|STL_inst)',n) for n in names):
    out.append((v,names))
c=collections.Counter()
for v,names in out:
  c[re.sub(r'_[0-9A-Fa-f]{8}.*$','',names[0])]+=len(v)
print(sum(len(v) for v,_ in out), c.most_common(40))
json.dump(out,open('local/st1_auto.json','w'))
