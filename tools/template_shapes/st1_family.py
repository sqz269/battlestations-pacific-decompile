"""Family regexes over the fine signature for the remaining untagged FUN_ (not covered by decisions/auto/rules)."""
import collections
import json
import re
import sys

d = json.load(open('local/st1_shapes.json'))
T = d['targets']
g = collections.defaultdict(list)
for a, t in T.items():
    if t['tag'] or t['ledger'] or t['recon']:
        continue
    g[t['coarse']].append(a)
dec = {l.split('|')[0] for l in open('tools/template_shapes/st1_decisions.txt') if l[0] != '#'}
decsig = {T[e]['coarse'] for e in dec if e in T}
auto = {a for v, _ in json.load(open('local/st1_auto.json')) for a in v}
rules = {a for v in json.load(open('local/st1_rules.json')).values() for a in v}
rem = [a for s, v in g.items() if s not in decsig for a in v if a not in auto and a not in rules]

R = r'(?:e[abcd]x|e[sd]i|ebp)'
N = r'(?:0x[0-9a-f]+|\d+)'
FAM = [
    # map/set destructor: erase(begin(), end()) through a stack iterator, free the head, zero head/size
    ('STL_Tree_Destroy', 'treedtor', rf'^sub esp, 8 ; push esi ; (?:push edi ; )?mov e[sd]i, ecx ; (?:mov eax, dword ptr \[e[sd]i \+ {N}\] ; lea esi, \[e[sd]i \+ {N}\] ; mov ecx, dword ptr \[eax\]|mov eax, dword ptr \[esi \+ 4\] ; mov ecx, dword ptr \[eax\]) ; push eax ; push esi ; push ecx ; push esi ; lea eax, \[esp \+ {N}\] ; push eax ; mov ecx, esi ; call \S+ ; mov ecx, dword ptr \[esi \+ 4\] ; push ecx ; call _free'),
    # _Tree::_Erase: recursive post-order free, guarded by the node's _Isnil byte
    ('STL_Tree_Erase', 'treeerase', rf'^push ebx ; push esi ; push edi ; mov edi, dword ptr \[esp \+ 0x10\] ; cmp byte ptr \[edi \+ {N}\], 0 ; mov ebx, ecx ; mov esi, edi ; jne L\w+ ; mov eax, dword ptr \[esi \+ 8\] ; push eax ; mov ecx, ebx ; call L0 ; .*call _free'),
    # _Tree::_Min / _Max: follow left/right links until the child's _Isnil byte is set
    ('STL_Tree_MinMax', 'treeminmax', rf'^mov eax, ecx ; mov edx, dword ptr \[eax(?: \+ 8)?\] ; cmp byte ptr \[edx \+ {N}\], 0 ; jne L\w+ ; (?:jmp L\w+ ; lea ecx, \[ecx\] ; |lea ebx, \[ebx\] ; )?mov eax, edx ; mov edx, dword ptr \[eax(?: \+ 8)?\] ; cmp byte ptr \[edx \+ {N}\], 0 ; je L\w+ ; ret $'),
    # allocator::allocate: max_size check, operator new, std::bad_alloc throw
    ('STL_allocator_allocate', 'alloc', r'^sub esp, 0x10 ; test ecx, ecx ; ja L\w+ ; xor ecx, ecx ; .*call operator_new ; add esp, 4 ; add esp, 0x10 ; ret  ; or eax, A ; xor edx, edx ; div ecx ; .*call exception ; .*call __CxxThrowException@8$'),
    # allocator::deallocate
    ('STL_allocator_deallocate', 'dealloc', r'^mov eax, dword ptr \[esp \+ 4\] ; push eax ; call _free ; pop ecx ; ret 8$'),
    # vector::size for non-power-of-two element sizes (magic-number divide)
    ('STL_Vector_Size', 'vecsize', rf'^mov eax, dword ptr \[ecx \+ 4\] ; test eax, eax ; jne L\w+ ; ret  ; mov ecx, dword ptr \[ecx \+ 8\] ; sub ecx, eax ; mov eax, (?:A|{N}) ; imul ecx ; (?:add edx, ecx ; )?sar edx, {N} ; mov eax, edx ; shr eax, 0x1f ; add eax, edx ; ret $'),
    # trivial constant / float constant returns
    ('TRIV_body', 'trivial', r'^(?:fldz|fld1|fld dword ptr \[A\]|xor al, al|mov al, 1|xor eax, eax|mov eax, 1) ; ret(?: 4| 8| 0xc)?$'),
    # cc13-sweep6D: red-black rotation (the decisions only matched the reference coarse signatures)
    ('STL_Tree_Rotate', 'rotate', rf'^mov edx, dword ptr \[esp \+ 4\] ; mov eax, dword ptr \[edx(?: \+ 8)?\] ; push esi ; mov esi, dword ptr \[eax(?: \+ 8)?\] ; mov dword ptr \[edx(?: \+ 8)?\], esi ; mov esi, dword ptr \[eax(?: \+ 8)?\] ; cmp byte ptr \[esi \+ {N}\], 0 ; jne L\w+ ; mov dword ptr \[esi \+ 4\], edx ; mov esi, dword ptr \[edx \+ 4\] ; mov dword ptr \[eax \+ 4\], esi'),
    # cc13-sweep6D: checked-iterator ++/-- wrapper returning *this around an STL-tagged step helper
    ('STL_IterWrap', 'iterwrap', r'^push esi ; mov esi, ecx ; call T:stl_\w+ ; mov eax, esi ; pop esi ; ret $'),
    # cc13-sweep6D: head-node _Buynode: operator new(node), three null links, _Color=1, _Isnil=0
    ('STL_Tree_BuyHeadNode', 'treehead', rf'^push {N} ; call operator_new ; add esp, 4 ; test eax, eax ; je L\w+ ; mov dword ptr \[eax\], 0 ; lea ecx, \[eax \+ 4\] ; test ecx, ecx ; je L\w+ ; mov dword ptr \[ecx\], 0 ; lea ecx, \[eax \+ 8\] ; test ecx, ecx ; je L\w+ ; mov dword ptr \[ecx\], 0 ; mov byte ptr \[eax \+ {N}\], 1 ; mov byte ptr \[eax \+ {N}\], 0 ; ret $'),
]

out = collections.defaultdict(list)
for a in rem:
    f = T[a]['fine'].replace('\n', ' ; ')
    for name, key, pat in FAM:
        if re.search(pat, f):
            out[(name, key)].append(a)
            break
print(len(rem), {k[0]: len(v) for k, v in out.items()})
for k, v in out.items():
    print(k[0], v[:8])
json.dump({f'{k[0]}|{k[1]}': v for k, v in out.items()}, open('local/st1_family.json', 'w'))
