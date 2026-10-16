# Actual InputDevice tree rotations

This packet reconstructs two complete nil99 tree leaves in
[native_input_device_tree_rotations.cpp](../src/native_input_device_tree_rotations.cpp),
with [explicit interfaces](../include/bsp/native_input_device_tree_rotations.hpp)
and an [evidence report](../reports/oct15_input_device_tree_rotations.json).
The existing STL names are retained as hypotheses, not recovered symbols.

| Entry | Coverage | Operation |
|---|---|---|
|00552340..00552394|85B / 30 instructions|right rotation|
|00553F00..00553F50|81B / 30 instructions|left rotation|

Both native entries receive the actual tree in ECX and one stacked node word;
incoming EDX is unused and immediately overwritten by that node. Each normal
exit uses RET4 and leaves EAX containing the pivot. The caller windows consume
no semantic result, so the new Source APIs return void and explicitly qualify
that machine residue. The unused EDX Source parameter retains the physical
input role; it is not a new semantic tree value.

Right rotation captures node.left as pivot, reads pivot.right and stores it to
node.left, then **rereads pivot.right after that store** before the nil99 test.
Only a nonnil child receives parent=node. It then reads CURRENT node.parent,
stores pivot.parent, captures tree.head once, and compares node with that
captured head.root. A root replacement uses that captured head. Otherwise it
reloads node.parent, compares node with parent.right first, and updates right
or left. Final writes are pivot.right=node, then node.parent=pivot.

Left rotation separately captures node.right, reads/stores/rereads pivot.left,
and applies the same reached nil99/parent/head schedule. Its nonroot comparison
tests parent.left first; final writes are pivot.left=node, then node.parent=pivot.
The two bodies are not implemented through a generic direction helper. Inline
assembly keeps each raw DWORD/byte read and write, including alias-sensitive
rereads and the once-captured head. Source joins the final two stores within
each function; private compiler epilogues add no actual-storage access there.

The tree head word is at+4; node links are0/4/8. Bounded producer windows show
the actual9Ch allocation, link publications, pair receiver node+C, color98 and
nil99=0. Six bounded erase-one fixup windows push the node, pass tree in ECX,
and either reload their sibling after the rotation or continue without using
EAX. These windows establish layout/input/use evidence only; neither their
whole constructor nor erase-one Source is promoted by this packet.

The leaves have no calls, tail-delivered child service, local EH, allocation,
pool/CRT context, callback or rollback. No null/nil-start/ownership guard is
added. Reached pivot/child/sentinel/parent/head storage must be valid and
writable. Old actual AV algorithms and previously accepted Source stay intact.
Ordinary RawCtx erase-one, its message action and the range entry remain outside
this packet.

Fresh complete typed native bodies, PE/live bytes and instruction starts agree:
166 owned bytes / 60 starts; 384 inspected bytes / 115 starts including only
the bounded support windows. Full owned listings and per-span hashes are
retained in the report/ignored audit. Every live batch verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

| Native transfer | Scope |
|---|---|
|006A7392 CALL 00553F00|read-only fixup caller|
|006A73DB CALL 00552340|read-only fixup caller|
|006A7401 CALL 00553F00|read-only fixup caller|
|006A7424 CALL 00552340|read-only fixup caller|
|006A747D CALL 00553F00|read-only fixup caller|
|006A74A1 CALL 00552340|read-only fixup caller|
|0055BB66 CALL 00BF681B|read-only layout producer|
|0055BB9D CALL 0055BA00|read-only layout producer|

MSVC Win32 build and existing three suites passed, 3/3 in 8.35 seconds; no new
tests were added. Full emitted COFF retained the two rereads, once-captured
head, side-specific comparisons, ordered stores and RET4; no calls or
relocations. The Source private frame differs from the native leaf frame. Source/compiler private frames, register saves,
flags/residue and hardware fault/SEH behavior remain distinct from original
native ABI or game validation. This worker performs no Ghidra mutation or push.
