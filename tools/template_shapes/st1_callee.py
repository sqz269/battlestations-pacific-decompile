# for groups given by example address: list callee addresses (non-local calls to F) and their coarse-group ex + label
import json,sys,collections,re,bisect,sqlite3
sys.argv=sys.argv
d=json.load(open('local/st1_shapes.json'));T=d['targets']
g=collections.defaultdict(list)
for a,t in T.items():
  if t['tag'] or t['ledger'] or t['recon']: continue
  g[t['coarse']].append(a)
ex_of={a:v[0] for s,v in g.items() for a in v}
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
import pefile
pe=pefile.PE(json.load(open('../battlestations-pacific-decompile/exports/bsp/disk_binary.json'))['path'],fast_load=True)
tx=next(s for s in pe.sections if s.Name.startswith(b'.text'));lo=pe.OPTIONAL_HEADER.ImageBase+tx.VirtualAddress;data=tx.get_data()
db=sqlite3.connect('local/bsp_index.sqlite');names=dict(db.execute('select address,name from functions'))
md=Cs(CS_ARCH_X86,CS_MODE_32)
def calls(a):
  out=[]
  for i in md.disasm(data[a-lo:a-lo+600],a):
    if i.mnemonic=='int3':break
    if i.mnemonic=='call' and i.op_str.startswith('0x'):out.append(int(i.op_str,16))
  return out
for ex in sys.argv[1:]:
  s=T[ex]['coarse'];mem=g[s];c=collections.Counter()
  for a in mem:
    for t in calls(int(a,16)):
      h=f'{t:08x}';nm=names.get(t,'?')
      if nm.startswith('FUN_'): c['grp:'+ex_of.get(h,'notarget:'+h if h not in T else 'tagged/named:'+h)]+=1
      else: c[re.sub(r'_[0-9a-f]{8}$','',nm)]+=1
  print(ex,len(mem),dict(c.most_common(6)))
