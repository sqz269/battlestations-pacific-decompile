"""Cluster decompiled raw rows by normalised pseudocode text."""
import json, os, re, sys, collections, hashlib
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
d = json.load(open(os.path.join(root, 'local', 'stlr_feat.json')))


def norm(t):
    t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
    t = re.sub(r'\b(FUN|LAB|DAT|PTR|EHHandler|Unwind|STL|BSP|TMPL|TRIV|CG|LIBCRT|LIBSTL|SoldierRegistry|MSVC)_\w+', 'F', t)
    t = re.sub(r'\b0x[0-9a-fA-F]+\b', 'N', t)
    t = re.sub(r'\b\d+\b', 'N', t)
    t = re.sub(r'\b(param|local|iVar|uVar|piVar|puVar|pcVar|cVar|bVar|pvVar|ppiVar|ppuVar|fVar|sVar|lVar|auStack|pcStack|pvStack|uStack|iStack|in_\w+|unaff_\w+|extraout_\w+)_?\w*', 'v', t)
    t = re.sub(r'\b(undefined\d?|int|uint|char|short|ushort|byte|bool|float|double|void|code|dword|longlong|ulonglong)\b', 'T', t)
    t = re.sub(r'\s+', ' ', t)
    return t


groups = collections.defaultdict(list)
for x in d:
    p = os.path.join(root, 'local', 'stlr_decomp', x['a'] + '.c')
    if not os.path.exists(p):
        continue
    t = open(p, encoding='utf-8').read()
    x['vcall'] = bool(re.search(r'\(\*\(code \*\*?\)', t)) and not re.search(r'\)\)\(1\);', t)
    k = hashlib.md5(norm(t).encode()).hexdigest()[:10]
    x['nk'] = k
    groups[k].append(x['a'])
json.dump(d, open(os.path.join(root, 'local', 'stlr_feat.json'), 'w'), indent=0)
sz = collections.Counter(len(v) for v in groups.values())
print('groups', len(groups), 'size hist', sorted(sz.items())[:12])
for k, v in sorted(groups.items(), key=lambda kv: -len(kv[1]))[:int(sys.argv[1]) if len(sys.argv) > 1 else 20]:
    print(len(v), k, v[:4])
