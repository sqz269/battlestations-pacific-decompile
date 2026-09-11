# Native resource registry erase

This packet reconstructs complete `B19F90[716]` and `B1A2F0[201]` using actual
caller-owned registry tree storage, the completed six resource tree leaves,
the existing actual owning string pool, and the existing owning native
out-of-range exception transport. It does not establish registry population,
factory lifetime, enclosing registry destruction, or the cache consumer.

| Entry | Native ABI | New source |
| --- | --- | --- |
| B19F90..B1A25C exclusive | ECX tree; stack output pointer, input owner/node by value; RET0C; EAX output | `erase_native_resource_registry_iterator_00b19f90` |
| B1A2F0..B1A3B9 exclusive | ECX tree; stack output pointer, first owner/node, last owner/node by value; RET14; EAX output | `erase_native_resource_registry_range_00b1a2f0` |

These are new MSVC Win32 C++ interfaces. Their extra pool and returning
invalid-parameter arguments bind existing concrete services. They are not
native ABI replacements. No Ghidra, shared ledger, CMake, or installed-game
changes belong to this worker packet.

## Actual storage and ownership

The tree is the actual registry subobject at registry+4. Its own head is +4,
count +8, and preserved first word +0. Each 1Ch node has left/parent/right
DWORDs +0/+4/+8, pooled key length/data +0C/+10, opaque factory value +14,
and color/nil bytes +18/+19. The head stores minimum/root/maximum in its
three links. Checked iterators are the existing two actual owner/node words.
Current storage is read directly; no projections, reconstructed population,
hardware-layout node substitution, or extra validation is introduced.

Single erase returns a present key through `ActualNativeStringPoolStorage`
before freeing the original node with the existing CRT free boundary. The
key's data and length+1 are captured before the unconditional actual getter
and return composition. Factory values are neither inspected nor released.
The pool uses its existing actual publication, gate, raw storage and canonical
lifetime binding. Its `release` API is already `noexcept`: lazy recreation
failure and native SEH remain outside the returning-getter source domain.

## Single erase schedule

The original input node is captured and its nil byte tested before owner
validation. Non-nil input advances its by-value iterator through full
`B19890`; the original node remains the disposal target. It selects the
right child when left is nil, the left child when only right is nil, or the
advanced successor when both are present. The direct case captures parent
after the replacement nil test, repairs the current parent/root link, and
updates current head minimum/maximum through full `B196F0`/`B196D0`.

The two-child case retains live `B1A09C..B1A0F3`, formerly omitted from the
throw-only pseudocode: successor transplant, conditional successor-parent
unlink, original-left and current-right parent rewrites, current root or
parent-side replacement, parent publication, and the byte color swap.
Neither the key nor factory value is copied into another node.

Both sides of black-node fixup are complete. They preserve red sibling
rotation, nil sibling continuation, both-black child recoloring, near-child
rotation, far-child recoloring/final rotation, current-head/root reload on
ascent, and final replacement blackening. Existing complete resource-tree
rotations are selected directly. Current near-child links are reloaded for
the conditional color writes, and accesses retain native DWORD/BYTE widths.

The complete returning-free tail `B1A222..B1A25C` reads the current tree count
after the actual free. It decrements only a nonzero unsigned value, captures
advanced owner, output pointer and advanced node, then stores output owner
before output node. It returns that output pointer. Single erase does not
require input owner to equal the destination tree, and it does not reset
head/count or acquire ownership of the returned iterator.

## Full and partial range schedule

Range erase captures first owner and current head minimum before the first
returning validation; it reads first node afterward. When that node equals
the captured minimum, it captures last owner and current head before the
second validation, then reads last node for comparison with the captured
head. A matching full range destroys the current root through `B1A260`.

After subtree disposal it reloads head to publish root=head, reloads head
before count=0 and minimum=head, reloads head again for maximum=head, then
captures current minimum and publishes output owner/node. Empty full ranges
execute the same route. Header/output aliasing does not justify moving the
result-node read after the first output store.

The partial path validates cached first owner against the current by-value
last owner on every iteration and compares cached first node with last node
after the handler returns. Before erase it advances its own first iterator;
then it calls complete `B19F90` with the old cached owner/node, which advances
its separate by-value iterator again. The erase output is ignored. It
reloads first node and then first owner before repeating. Those two increments
cannot be replaced by assignment of the erase result. A throwing handler or
failure leaves already completed mutations in place; there is no new rollback.

## Native owning exception and source transport

Nil input initializes only the observed SBO temporary fields: capacity 15,
length zero and first NUL. Full `408720` assigns the 27 bytes at CE44E0,
including the terminator in the counted length. State 0 is armed only after
assignment succeeds. `411700` constructs the owning logic-error storage;
then the raw profile becomes D6926C and native `_CxxThrowException` receives
ThrowInfo D863A8.

Fresh FH3 evidence is CBC628 -> DF48F0, unwind map DF48E8, state 0 -> CBC620.
CBC620 uses the temporary at EBP-50 and jumps to complete `4072D0`. A failed
initial assignment therefore does not receive newly invented outer cleanup.
The source's completed-temporary guard begins at the same successful
assignment boundary and uses that complete destructor.

The existing `NativeHardwareLayoutInvalidIterator` supplies only the owning
host exception transport: actual 28h storage, `411700` then D6926C, complete
`441760` copy, and complete `4412B0` destructor. No hardware tree operation
or layout is called. Native ThrowInfo's copy/destructor identities agree;
the host carrier still has new RTTI, catch type and exception ABI. The source
does not reproduce native BF6885/RaiseException/SEH dispatch. Existing SBO,
logic-error, base exception and CRT helpers retain their documented native
contracts and host runtime boundaries.

## Verification and limits

Every fresh live byte query verifies the existing `bsp.gpr` and
`/battlestationspacific.exe`. The 31 finite spans, 2,333 bytes total, include
both full bodies, all six tree leaves, direct exception/pool/CRT providers,
FH3 actions and data, ThrowInfo/catchable profile/message, and neighboring
padding. Each span matches the unchanged installed PE. B19F90's saved full
body already includes the repaired returning-free tail; no worker repair
is requested. The report pins exact source, provider, build and artifact
inputs rather than inferring closure from familiar helper names.

The isolated base contains the six tree-leaf source files before their CMake
registration reaches a commit. An ignored hook registers those existing
leaves and this new source for the strict Win32 build. It leaves the shared
CMake file unchanged. Strict Win32 `/fp:strict /W4 /WX` build, both existing
CTests and all eight native seeds passed. The focused comparison passed with
231 trace words. Nine exact selected archive members contain 198 nonempty
COFF code sections, 14,256 bytes and 461 relocations. The proof records 82
linked sections, 8,238 bytes. Linked bytes outside relocation operands match
the selected COFF sections; relocation records and full linked bytes remain
available for review. It does not prove every resolved relocation target.

One ignored focused fixture compares five top-level original/source
operations: mirrored near-child rebalancing, a three-node partial range
including deep successor transplant, full clear, and an empty full range
whose returning handlers replace current heads and whose output aliases the
header. It uses actual node malloc/free and pool providers. Returning-free
observation changes count to 13 or zero; getter allocation changes a captured
key header. The partial route's 11 actual invalid-handler returns distinguish
the two independent increments. These mutations are fixture observations,
not production allocator or provider callbacks. Fixture node construction
and leftover-node cleanup do not establish a production population route.

Both owned original bodies, subtree disposal, iterator increment and both
rotations execute with four fixed existing pool/CRT provider bridges. The
minimum/maximum leaves are installed and statically checked; this sequence
does not reach them. Original pool/CRT helpers and original
exception runtime are not re-executed. A separate source-only nil-input check
observes the exact native owning exception payload, independent owning copy,
destruction and the absence of owner validation/output publication. Static
FH3 and linked code complement this check; no native EH equivalence is claimed.
Whole-object provenance is broader than executed paths. The immutable bundle
keeps the reviewed fixture and trace for primary relinking.
