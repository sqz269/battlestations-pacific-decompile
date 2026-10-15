# Scene-record map constructor variants

Complete ordinary Source for 00468900 (41 bytes, 14 instructions) and
0046F130 (43 bytes, 15 instructions). Existing STL library names are retained;
the scene-record family description is inferred from the 1Ch node layout and
surrounding users, not recovered symbols. Both are ECX tree, RET, with no stack
arguments. Incoming EDX is unused. There are no live caller references.

Both allocate through the actual 00468260 provider before touching tree fields.
They publish the allocation at tree+4, set allocated head.nil19=1, then reload
CURRENT tree+4 separately for parent, left, and right self-links, finally writing
count+8=0. Tree+0 is untouched; there is no prior-head cleanup or local rollback.
The accepted allocator retains its complete current-CRT allocation behavior.

00468900 leaves EAX holding the head captured for the right self-link, which can
be different from a later reread where valid raw storage aliases. Source exposes
that raw pointer result without claiming an inferred semantic return type.
0046F130 explicitly copies the original tree into EAX after the count store.
Its memory schedule is instruction-identical to accepted 00468E70; the only
other difference is native RET versus RET8 with two unread stack words. Source
reuses that complete existing construction implementation, whose ordinary API
already omits the unread words. The native transfers are recorded below.

Native transfers are `00468903 CALL 00468260` and `0046F133 CALL 00468260`.
No callbacks, placeholder allocations, ownership guesses, or extra reloads are
introduced. Valid reached raw storage and current-CRT allocation remain required.
Build and existing regression proof do not establish original binary/CRT/heap
ABI, new-body native differential behavior, application adoption, or gameplay.
