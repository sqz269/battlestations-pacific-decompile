"""Feature table for every stl_probable bookmark row: ledger name, datarefs, callee/caller classes."""
import json, sqlite3, os, sys, collections
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
db = sqlite3.connect(os.path.join(root, 'local', 'bsp_index.sqlite'))
F = {}
for r in db.execute('select address,name,tag_category,tag_name,ledger_name,recon_name from functions'):
    F[r[0]] = r
callees = collections.defaultdict(set)
callers = collections.defaultdict(set)
for a, b in db.execute('select caller,callee from calls'):
    callees[a].add(b)
    callers[b].add(a)
drefs = collections.defaultdict(list)
for f, ref, t in db.execute('select func,ref,text from datarefs'):
    drefs[f].append((ref, t))

import re
ELEM = re.compile(r'NativeString|SizedStoragePool|CharacterString|LegacySbo|StringRange|RefPtr|_CopyConstruct|_Destroy|_Destruct|_Assign|_Release|_AddRef|_Construct|DestroyActualStorage|DestroyStorage|_Copy(_|$)|_Less|_Equals|_Compare|_Hash')
LIB_CATS = ('stl_', 'tmpl_', 'cg_', 'crt', 'trivial', 'static_init', 'lua511')


def cls(a):
    r = F.get(a)
    if not r:
        return 'unk'
    nm = r[4] or r[1] or ''
    cat = r[2] or ''
    if cat == 'stl_probable':
        return 'probable'
    if cat.startswith(LIB_CATS):
        return 'lib'
    if nm.startswith(('std::', '_', 'operator', '__', 'Concurrency', 'STL_', 'TRIV_', 'CG_', 'TMPL_', 'Tmpl_', 'stl_', 'CRT_', 'lua')) or 'Dinkum' in nm:
        return 'lib'
    if nm.startswith('FUN_') or not nm:
        return 'raw'
    return 'game'


def name(a):
    r = F.get(a)
    if not r:
        return '?'
    return r[4] or r[1] or ''


rows = []
import glob
for f in sorted(glob.glob(os.path.join(root, 'config', 'tags', '*.jsonl'))):
    for ln in open(f, encoding='utf-8'):
        if ln.strip():
            t = json.loads(ln)
            if t['category'] == 'stl_probable':
                rows.append(t)
import pefile, struct
_pe = pefile.PE(json.load(open(r'J:\PROG\battlestations-pacific-decompile\exports\bsp\disk_binary.json'))['path'], fast_load=True)
_img = _pe.get_memory_mapped_image()


def rkind(ref):
    if ref >= 0xe08000:
        return 'data'
    if 0xce2000 <= ref < 0xe08000:
        v = struct.unpack_from('<I', _img, ref - 0x400000)[0]
        return 'vptr' if 0x401000 <= v < 0xce2000 else 'const'
    return 'other'


out = []
for t in rows:
    a = int(t['address'], 16)
    ce = collections.Counter(cls(x) for x in callees[a])
    cr = collections.Counter(cls(x) for x in callers[a])
    ds = [x for x in drefs[a] if x[0] != 0xce3a0c]
    strs = [x[1] for x in ds if x[1]]
    out.append({
        'a': t['address'], 'ev': t['evidence_id'], 'ledger': (F.get(a) or [None] * 6)[4] or '',
        'ghidra': (F.get(a) or [None] * 6)[1] or '', 'recon': (F.get(a) or [None] * 6)[5] or '',
        'ce': dict(ce), 'cr': dict(cr), 'nd': len(ds), 'rk': dict(collections.Counter(rkind(x[0]) for x in ds if not x[1])), 'strs': strs[:4],
        'drefs': [hex(x[0]) for x in ds][:8],
        'game_callees': [name(x) for x in callees[a] if cls(x) == 'game'][:6], 'ngc': sum(1 for x in callees[a] if cls(x) == 'game'), 'ngs': sum(1 for x in callees[a] if cls(x) == 'game' and not ELEM.search(name(x))), 'strong': [name(x) for x in callees[a] if cls(x) == 'game' and not ELEM.search(name(x))][:5], 'nstr': len(strs),
        'game_callers': [name(x) for x in callers[a] if cls(x) == 'game'][:4],
    })
json.dump(out, open(os.path.join(root, 'local', 'stlr_feat.json'), 'w'), indent=0)
print(len(out))
