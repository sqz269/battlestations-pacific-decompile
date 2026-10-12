import json,collections,re
d=json.load(open('local/st1_shapes.json'));T=d['targets']
g=collections.defaultdict(list)
for a,t in T.items():
  if t['tag'] or t['ledger'] or t['recon']: continue
  g[t['coarse']].append(a)
dec={l.split('|')[0] for l in open('tools/template_shapes/st1_decisions.txt') if l[0]!='#'}
decsig={T[e]['coarse'] for e in dec if e in T}
auto={a for v,_ in json.load(open('local/st1_auto.json')) for a in v}
rem=[a for s,v in g.items() if s not in decsig for a in v if a not in auto]
def rule(t):
  L=t['fine'].split('\n'); n=len(L); f=t['fine']
  if n<=3 and re.fullmatch(r'(sub|add) ecx, \S+\njmp \S+',f): return 'cg_adjustor_thunk'
  if 'call LIBCRT_unmatched' in f and t['size']<=96: return 'stl_instantiation'
  if re.search(r'test byte ptr \[esp \+ (4|8|0x[0-9a-f]+)\], 1',f) and re.search(r'call (_free|operator_delete)',f) and L[-1]=='ret 4' and n<=40: return 'cg_scalar_deleting_dtor?'
  if t['size']<=32 and n<=3 and re.fullmatch(r'ret( \S+)?',L[-1]):
    body=L[:-1]
    ok=all(re.match(r'(mov|xor|lea|fld|fldz|fld1|movzx|movsx|or|and) ',x) or x in('fld1','fldz') for x in body)
    if ok: return 'trivial_body?'
  if 'call LIBCRT_unmatched' in f: return 'scl_97plus'
  return None
c=collections.Counter(); ex=collections.defaultdict(list)
for a in rem:
  r=rule(T[a]); c[r]+=1; ex[r].append(a)
print(len(rem),c)
json.dump({k:v for k,v in ex.items() if k},open('local/st1_rules.json','w'))
for k in ex:
  if k: print(k, ex[k][:6])
