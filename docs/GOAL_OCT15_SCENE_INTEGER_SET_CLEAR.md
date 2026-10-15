# Scene integer-set subtree destruction and clear

Complete ordinary Source for 00468300 (53 bytes, 23 instructions) and 00468980
(43 bytes, 16 instructions), verified against the existing BSP program and
installed executable. Existing STL library names are retained; the scene
integer-set family description is inferred from the 14h key-only node layout
and surrounding callers, not recovered debug symbols.

00468300 takes ECX tree and one stacked node, RET4. EDX is unused. The tree
pointer is forwarded unchanged to recursive calls but never dereferenced.
The initial byte at node+11h is tested. A nonnil node recurses into its current
right child, then captures its current left child before the real 00BF65AC
free provider. It tests captured left+11h after free and repeats. Initial nil
nodes, including the sentinel, are never freed. The scalar key has no destructor.
There is no initial-node shortcut that would suppress the native nil read.

00468980 takes ECX tree, RET, no semantic result. It captures tree+4 then
head+4, calls the subtree provider, reloads tree+4 and stores head.parent=head.
It reloads tree+4 BEFORE storing count 0, writes captured head.left=head, then
reloads tree+4 again before head.right=head. These captures and volatile raw
accesses preserve observable ordering where caller-owned storage aliases.
Tree+0 and the sentinel allocation remain untouched. Reusing the record-map
provider would read nil19h and destroy pooled string keys, so the actual nil11h
key-only implementation is required.

Native transfers are `00468317: CALL 00468300`, `0046831F: CALL 00BF65AC`,
`0046898C: CALL 00468300`, and `0046B1BA: CALL 00468300`.
These transfer claims refer to native instructions; the Source free service is
singleton_lifetime_free. Complete source uses ordinary explicit raw pointers
and current CRT, without callbacks, stubs, or invented mapped ownership.

Reachable pointers and allocations must belong to the documented native raw
layout and current-CRT allocation domain. Source compilation and existing
regression checks do not prove original register/CRT/heap identity, private
stack alias compatibility, new-body native differential behavior, application
adoption, or game validation. No new tests were added for this direct cleanup.
