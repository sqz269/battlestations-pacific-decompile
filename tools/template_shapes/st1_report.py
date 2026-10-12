import json,collections,sqlite3
rows=json.load(open('local/st1_entries.json'))
before={20:546,17:220,23:224,26:372,27:373,29:329,6:337,15:298,12:193,19:216,2:247,9:269,78:243,81:252,84:215,85:159,86:24,88:6,89:4,90:68,91:46,92:11,93:108,94:108,96:3,43:310,45:200,46:127,61:295,64:304,65:126,76:5,77:5,79:125,80:5,82:108,83:68,25:120,22:149,16:161,28:157,4:145,10:199,5:130}
db=sqlite3.connect('local/bsp_index.sqlite')
after={s:db.execute("select count(*) from functions where segment=? and name like 'FUN_%' and thunk=0",(s,)).fetchone()[0] for s in before}
seg={r['address']:db.execute('select segment from functions where address=?',(int(r['address'],16),)).fetchone()[0] for r in rows}
shapes=collections.defaultdict(list)
for r in rows:
  shapes[(r['category'],r['name'].rsplit('_',1)[0] if r['name'] else '(bookmark)',r['evidence_id'])].append(r['address'])
ev=json.load(open('config/tags/evidence.json'))
rep={'packet':'cc13-stl1-template-shapes','doc':'docs/CC13_TEMPLATE_COPY_SWEEP.md',
 'image_untagged_fun_before':17966,'image_untagged_fun_after':14559,
 'segments_fun_before':sum(before.values()),'segments_fun_after':sum(after.values()),
 'renamed':sum(1 for r in rows if r['action']=='rename'),'bookmarked':sum(1 for r in rows if r['action']=='bookmark'),
 'skipped_leased':42,
 'per_segment':{str(s):{'before':before[s],'after':after[s]} for s in before},
 'shapes':[{'category':k[0],'prefix':k[1],'evidence':ev[k[2]],'count':len(v),'addresses':v} for k,v in sorted(shapes.items(),key=lambda kv:-len(kv[1]))]}
open('reports/cc13_template_copy_sweep.json','w',newline='\n').write(json.dumps(rep,indent=1)+'\n')
for s in rep['shapes']: print(s['count'],s['category'],s['prefix'])
