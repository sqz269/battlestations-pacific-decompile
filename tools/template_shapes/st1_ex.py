import json,sys
d=json.load(open('local/st1_shapes.json'));T=d['targets']
w=int(sys.argv[1])
for a in sys.argv[2:]:
  print(a, T[a]['fine'].replace('\n',' ; ').replace('dword ptr ','')[:w]); print()
