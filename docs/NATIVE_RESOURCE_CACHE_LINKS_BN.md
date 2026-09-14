# Actual resource-cache links and predecessor BN

This packet reconstructs three complete ordinary bodies over the actual
resource-cache tree/node/iterator storage. They are independent source bodies;
similar registry or VFS helper addresses are not treated as aliases.

| Routine | Native ABI | Coverage |
|---|---|---|
| B7CBD0..B7CC21, 82 bytes | ECX tree, stack node, EAX captured pivot, RET4 | Complete right rotation |
| B7D5F0..B7D63D, 78 bytes | ECX tree, stack node, EAX captured pivot, RET4 | Complete left rotation |
| B7CDF0..B7CE78, 137 bytes | ECX iterator, no stacked arguments, RET or tail JMP BF6713 | Complete predecessor; no uniform EAX result |

The APIs are `rotate_native_resource_cache_right_00b7cbd0`,
`rotate_native_resource_cache_left_00b7d5f0`, and
`decrement_native_resource_cache_iterator_00b7cdf0`. The last receives existing
`SingletonLifetimeCallbacks`. No generic container, allocation, manager owner,
insertion, reference count, recoloring or resource dispatch is introduced.

## Actual layout and read/store schedule

Tree+4 is the head; head+4 is the root. Actual1Ch nodes have left/parent/right
at +0/+4/+8, key at +C/+10, borrowed resource at +14, color at +18 and nil at
+19. The actual iterator is two DWORDs `{owner,node}`. The producer `B7F220`
and [ownership audit](RESOURCE_CACHE_OWNERSHIP.md) establish these offsets;
the concurrent node worker independently confirmed them. Payload, color,
count and padding are untouched by these helpers.

Right rotation captures node.left as pivot in EAX. It reads pivot.right and
writes node.left, then **reloads** pivot.right for its nil check and optional
parent repair. It reads current node.parent and publishes pivot.parent before
capturing the current tree head. At the root it changes that captured head's
root; otherwise it reloads current node.parent and checks parent.right before
choosing parent.right or parent.left. Finally it writes pivot.right=node and
node.parent=pivot. EAX retains pivot on all three RET4 paths.

Left rotation mirrors the links while preserving its specific order:
capture node.right, write/reload pivot.left, repair the nonnil moved child,
publish pivot.parent, capture current head, then repair root or parent link.
Its nonroot branch checks parent.left first. The final stores are
pivot.left=node and node.parent=pivot; EAX again retains the captured pivot.

Predecessor checks owner-null through CALL `B7CDF8 -> BF6713`, then reloads
the current node so a returning handler's repair is observed. A nil node
selects its captured rightmost pointer, publishes it to iterator+4, and checks
that captured result. A nil result tail-jumps from B7CE13 to BF6713; source
returns after a returning handler with no additional stores.

A nonnil left subtree is followed to its rightmost nonnil node. Otherwise
predecessor climbs current parents while the current iterator node equals
the ancestor's left link. Each climbed ancestor is published to iterator+4
**before** reading that ancestor's current parent. The final nil test rereads
the current iterator node. A nil value tail-jumps from B7CE6F to BF6713 and
returns if the handler returns; otherwise publish the captured ancestor.
No added begin check, owner-equality check or sentinel normalization appears.

`BF6713` calls its existing CRT boundary with five zero arguments, cleans
14h bytes and returns. Source callbacks may return and repair storage; they
are not annotated as noreturn. No original FH3/SEH equivalence is claimed.

## Callers and body membership

Current direct xrefs were inspected together with their argument setup.
Rotations receive the current tree in ECX and one pushed node; the callee
cleans that DWORD. Erase `B7FA60` calls right rotation at B7FC35/B7FC66/B7FCC9
and left rotation at B7FC02/B7FC51/B7FCAF. Insert-at `B7FF80` calls right at
B800B3/B800E1 and left at B80095. Its other left rotation is inlined and is
not attributed to this packet.

Insert-unique calls predecessor at B8045F with an actual local iterator in
ECX and reloads its node afterward. Wrappers B7D8C0 and B7DEE0 call it at
B7D8C3 and B7DEE3, then return the captured iterator pointer. The wrappers'
EAX result is not a uniform return value of B7CDF0 itself.

Live `bsp.py ghidra` batches verified project `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 language and image base400000. The complete
listings contain 30/30/48 instructions with zero gaps. There are no
`no_ghidra_function` intervals inside the three owned bodies. No Ghidra writes
were made. The report records the one internal CALL, both internal tail JMPs
with `kind: tail_jump`, and all twelve current external call rows.

## Validation and integration boundary

Strict MSVC Win32 C++17 `/EHsc /W4 /WX /O2 /fp:strict /MD` TU and fixture
compilation pass. Ten source/header inputs are frozen. Library inputs were
copied from the preceding lookup fixture, preserving its original primary
checkout provenance; pre-copy/post-copy/frozen-copy hashes agree. The core
hash is `134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`.
No shared build artifact was read during this packet. These helpers require
no external reconstructed body beyond the explicit callback, so merely
passing that library to the linker adds no dependency-behavior proof.

The ignored probe links with `/MANIFEST:EMBED`. It exercises root and both
parent-side branches of each rotation, nonnil/nil moved children, the pivot
return and unchanged payload/color/count. Predecessor checks cover left-subtree
rightmost selection, ancestor ascent, end/max, owner repair and both returning
tail handlers.

The final tail is reached through a controlled **actual-storage** alias:
the iterator occupies node B+14, overlapping B's color/nil bytes with its node
word. A is at a 64KB-aligned `VirtualAlloc` base and B is base+100h. Initially
iterator.node=A keeps B.nil=0. Publishing B while climbing changes B.nil to1;
the current-node nil test reaches B7CE6F. The handler repairs the iterator to
A, and the source returns without replacing that repair with the ancestor.
The probe releases its page afterward. It neither executes original code nor
uses a fake iterator implementation to force the branch.

The ten-rule checklist's callee, boundary, caller, producer, complete-coverage,
argument-cleanup, register-provenance, lease and report requirements are
covered by this evidence. Frame-runtime validation is not applicable here:
no production route or manager was integrated and no frame behavior is claimed.
No permanent tests, CMake/shared metadata edits, full integration build, ABI
replacement or gameplay result is claimed. Primary owns those next actions.
Arbitrary original stack/register-spill aliases, concurrency and native
exception identity remain outside the source proof.
