"""cc13-stl1: turn the reviewed shape groups into tag-ledger rows (config/tags shards + evidence.json).

Sources: local/st1_decisions.txt (reviewed coarse groups), st1_auto.json (coarse twins of STL_/CG_/TRIV_
reference functions), st1_rules.json (the repo's own mechanical rules), st1_family.json (family regexes).
Skips addresses leased to another owner and addresses already tagged. Writes local/st1_entries.json.
"""
import collections
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import coordination  # noqa: E402
from ledger import TAGS_DIR, TAG_EVIDENCE, band, _read_dir, _write_shard  # noqa: E402

SRC = 'docs/CC13_TEMPLATE_COPY_SWEEP.md'
EV = {
    'iterwrap': 'cc13-stl1 shape: push esi; mov esi,ecx; call <STL-tagged checked-iterator helper>; mov eax,esi; ret (checked iterator operator++/-- wrapper returning *this)',
    'dealloc': 'cc13-stl1 shape: mov eax,[esp+4]; push eax; call _free; pop ecx; ret 8 (std::allocator<T>::deallocate)',
    'treedtor': 'cc13-stl1 shape: erase(begin(),end()) through a stack iterator, _free(head), zero head/size (std::_Tree destructor / _Tidy)',
    'vectidy': 'cc13-stl1 shape: if first: _free(first); first=last=end=0 (std::vector<POD>::_Tidy)',
    'vectidydestroy': 'cc13-stl1 shape: destroy [first,last) then _free(first); first=last=end=0 (std::vector<T>::_Tidy)',
    'rotate': 'cc13-stl1 shape: red-black left/right rotation over node links with _Isnil byte checks (std::_Tree::_Lrotate/_Rrotate)',
    'treemin': 'cc13-stl1 shape: follow [node+0] until child _Isnil byte set (std::_Tree::_Min)',
    'treemax': 'cc13-stl1 shape: follow [node+8] until child _Isnil byte set (std::_Tree::_Max)',
    'treeminmax': 'cc13-stl1 shape: follow left/right link until child _Isnil byte set (std::_Tree::_Min/_Max)',
    'dispatch': 'cc13-stl1 shape: builds zero-byte iterator-category tag arguments on the stack and forwards to a _Uninit_*/_Copy_* worker (STL algorithm tag dispatch)',
    'trivial': 'cc13-stl1 shape: trivial body (ret / constant return / getter / setter), repo trivial_pattern',
    'treector': 'cc13-stl1 shape: head=_Buynode(); head->_Isnil=1; head->left=parent=right=head; size=0 (std::_Tree constructor/_Init)',
    'alloc': 'cc13-stl1 shape: max_size check, operator new(n*sizeof(T)), std::bad_alloc throw (std::allocator<T>::allocate/_Allocate)',
    'treehead': 'cc13-stl1 shape: operator new(node); links=0; _Color=black; _Isnil=0 (std::_Tree::_Buynode for the head)',
    'refptr': 'cc13-stl1 shape: if p: if InterlockedDecrement(&p->refs)==0: p->vslot0(); p=0 (template ref-counted pointer reset/release)',
    'treeclear': 'cc13-stl1 shape: _Erase(root) then head->left=parent=right=head, size=0 (std::_Tree::clear fast path)',
    'adjustor': 'cc13-stl1 shape: sub ecx,N; jmp target (compiler this-adjustor thunk)',
    'fill': 'cc13-stl1 shape: dword store loop over [first,last) or n copies of *val (std::fill / fill_n for 4-byte elements)',
    'listclear': 'cc13-stl1 shape: unlink head, size=0, walk nodes freeing each (std::list::clear)',
    'listhead': 'cc13-stl1 shape: operator new(node); next=prev=self (std::list::_Buynode for the head)',
    'vecsize': 'cc13-stl1 shape: first? (last-first)/sizeof(T) : 0 (std::vector::size)',
    'listctor': 'cc13-stl1 shape: head=list _Buynode(); size=0; return this (std::list constructor)',
    'sdd': 'cc13-stl1 shape: inlined destructor body, test [esp+N]&1 -> _free(this), ret 4 (scalar deleting destructor)',
    'ptrarray': 'cc13-stl1 shape: pointer-array capacity management: operator new(cap*4), copy elements, _free old (template pointer array reserve/grow/push_back; not std::vector)',
    'construct': 'cc13-stl1 shape: EH frame; if p: placement copy-construct (std::allocator::construct / _Construct)',
    'copypod': 'cc13-stl1 shape: (last-first) bytes moved with _memmove_s, returns dest end/begin (std::copy / copy_backward _Copy_opt for scalars)',
    'insertn': 'cc13-stl1 shape: vector _Insert_n(where,count,val) with capacity growth and checked-iterator hook (std::vector::_Insert_n)',
    'listtidy': 'cc13-stl1 shape: clear all nodes then _free(head), head=0 (std::list::_Tidy / destructor)',
    'ownedptr': 'cc13-stl1 shape: if p: p->vslot(1) (deleting destructor); p=0 (template owning pointer reset)',
    'treeerase': 'cc13-stl1 shape: recursive post-order _Erase(right) / _free(node) loop guarded by node _Isnil byte (std::_Tree::_Erase)',
    'treecopy': 'cc13-stl1 shape: _Copy(root,head) then recompute leftmost/rightmost, size copy; EH rethrow (std::_Tree::_Copy / copy constructor / operator=)',
    'uninit': 'cc13-stl1 shape: EH-guarded loop constructing each element of [first,last) at dest, catch destroys and rethrows (std::_Uninit_copy)',
    'uninitpod': 'cc13-stl1 shape: loop copying 4/8-byte elements to non-null dest slots (std::_Uninit_copy / _Uninit_fill_n for scalars)',
    'freeptr': 'cc13-stl1 shape: if *this: _free(*this) [; *this=0] (template owning raw-pointer free)',
    'treeinsert': 'cc13-stl1 shape: walk from root comparing keys, _Isnil checks, then _Insert / iterator decrement (std::_Tree::insert(value) unique)',
    'treeinserthint': 'cc13-stl1 shape: size==0 -> _Insert(true,head,val); otherwise checked-iterator hint compare (std::_Tree::insert(hint,value))',
    'treenode': 'cc13-stl1 shape: operator new(node); store left/parent/right, value, color, _Isnil=0 (std::_Tree::_Buynode)',
    'listnode': 'cc13-stl1 shape: operator new(node); next, prev, value stored (std::list::_Buynode)',
    'fillobj': 'cc13-stl1 shape: loop over [first,last) calling element assignment with one value (std::fill for class elements)',
    'lowerbound': 'cc13-stl1 shape: root walk with key compare and _Isnil byte, returns candidate node (std::_Tree::_Lbound)',
    'destroyrange': 'cc13-stl1 shape: loop over [first,last) running the element destructor (std::_Destroy_range)',
    'stringtidy': 'cc13-stl1 shape: if cap>=16: _free(buf); cap=15; size=0; buf[0]=0 (std::basic_string::_Tidy)',
    'twin': 'cc13-stl1: masked body identical to an existing STL_/CG_/TRIV_ reference function',
    'scl96': 'calls _invalid_parameter_noinfo 00bf6713 (_SECURE_SCL hook) and body <= 96 bytes (precision ~0.95)',
    'scl97': 'calls _invalid_parameter_noinfo 00bf6713 with body > 96 bytes; bookmark only',
}

d = json.loads((ROOT / 'local/st1_shapes.json').read_text())
T = d['targets']
groups = collections.defaultdict(list)
for a, t in T.items():
    if t['tag'] or t['ledger'] or t['recon']:
        continue
    groups[t['coarse']].append(a)

picks = {}  # address -> (category, prefix, evkey, action, confidence)


def put(a, cat, prefix, key, action='rename', conf='high'):
    picks.setdefault(a, (cat, prefix, key, action, conf))


for line in (ROOT / 'tools/template_shapes/st1_decisions.txt').read_text().splitlines():
    if not line or line[0] == '#':
        continue
    ex, cat, prefix, key = line.split('|')
    for a in groups[T[ex]['coarse']]:
        put(a, cat, prefix, key)
for v, names in json.loads((ROOT / 'local/st1_auto.json').read_text()):
    n = names[0]
    if n.startswith('TRIV_body'):
        cat, prefix = 'trivial_body', 'TRIV_body'
    elif n.startswith('CG_adjustor'):
        cat, prefix = 'cg_adjustor_thunk', 'CG_adjustor_thunk'
    elif n.startswith('CG_'):
        continue
    else:
        cat, prefix = 'stl_shape', 'STL_inst'
    for a in v:
        put(a, cat, prefix, 'twin')
rules = json.loads((ROOT / 'local/st1_rules.json').read_text())
for a in rules.get('cg_adjustor_thunk', []):
    put(a, 'cg_adjustor_thunk', 'CG_adjustor_thunk', 'adjustor')
for a in rules.get('stl_instantiation', []):
    put(a, 'stl_instantiation', 'STL_inst', 'scl96')
for a in rules.get('cg_scalar_deleting_dtor?', []):
    put(a, 'cg_scalar_deleting_dtor', 'CG_scalar_deleting_dtor', 'sdd')
for a in rules.get('trivial_body?', []):
    f = T[a]['fine']
    if re.fullmatch(r'(xor al, al|mov al, 1|xor eax, eax|mov eax, 1)\nret( \S+)?', f):
        put(a, 'trivial_body', 'TRIV_body', 'trivial')
for a in rules.get('scl_97plus', []):
    put(a, 'stl_probable', '', 'scl97', action='bookmark', conf='medium')
for k, v in json.loads((ROOT / 'local/st1_family.json').read_text()).items():
    prefix, key = k.split('|')
    cat = 'trivial_body' if prefix == 'TRIV_body' else 'stl_shape'
    for a in v:
        put(a, cat, prefix, key)

leases = [l for l in coordination.active_leases() if l['owner'] != coordination.owner_name()]
existing = {r['address'] for r in _read_dir(TAGS_DIR)}
rows, skipped = [], collections.Counter()
for a, (cat, prefix, key, action, conf) in sorted(picks.items()):
    if a in existing:
        skipped['already_tagged'] += 1
        continue
    if any(coordination.covers(l, int(a, 16)) for l in leases):
        skipped['leased'] += 1
        continue
    rows.append({'address': a, 'name': f'{prefix}_{a}' if prefix else '', 'category': cat, 'confidence': conf,
                 'action': action, 'create': False, 'source': SRC, 'evidence_id': f'st1_{key}'})
print(len(rows), 'rows;', dict(skipped), dict(collections.Counter(r['category'] for r in rows)))
(ROOT / 'local/st1_entries.json').write_text(json.dumps(rows))
if '--write' in sys.argv:
    by_band = collections.defaultdict(list)
    for r in rows:
        by_band[band(r['address'])].append(r)
    for b, new in by_band.items():
        path = TAGS_DIR / f'{b}.jsonl'
        old = []
        if path.exists():
            old = [json.loads(l) for l in path.read_text(encoding='utf-8').splitlines() if l.strip()]
        _write_shard(path, old + new)
    ev = json.loads(TAG_EVIDENCE.read_text(encoding='utf-8'))
    for key in {r['evidence_id'] for r in rows}:
        ev[key] = EV[key[4:]]
    TAG_EVIDENCE.write_text(json.dumps(ev, indent=1, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')
    print('written', len(by_band), 'shards')
