# Native alias insertion and resource-record assignment

This is read-only discovery for `004D2660`, complete `004D26A0` including catch
`004D2746`, and `00B30510`. Their bodies and dependencies are established well
enough for bounded implementation. They are not reconstructed by this packet.
The remaining count-growth/exception service and actual-owner name
composition must be present before accepting the whole assignment path.

The checkout snapshot is `c9f8b9d`: concrete alias-node construction, checked
iterators/erase, the actual 2Ch-byte resource record, and the six legacy SBO
operations are present. A later integration refresh verified root commit
`52012c4` removing the SBO convenience initializers and `f46f236` adding
`clear_native_render_resource_aliases_004d05e0(void*, SizedStoragePool&)`.
These dependencies are now available in main; the audit pins their committed
source separately from this discovery worktree's base. The primary owns the
`004CE780` follow-up, and the exception worker owns the legacy 28h-byte exception
constructors/copies/destructors. That count/exception work remains separate.

## Actual storage and ABI

The list owner is the address of the actual 12-byte embedded list at record
`+8`, not the record address or a copied header. Its preserved word, sentinel,
and unsigned count are at owner `+0/+4/+8`. An alias node is 16 bytes:
next/previous at `+0/+4`, pooled string length/data at `+8/+C`. Its strings are
the eight-byte pooled representation, not the 1Ch-byte legacy SBO string used
to build length exceptions. `NativeRenderAliasIterator` is the existing
eight-byte `{actual owner, node}` pair.

| Entry and complete span | Original calling contract |
| --- | --- |
| `004D2660..004D2691` | ECX actual destination list; one stack pointer to an eight-byte source string header; RET 4. No semantic return is established; ordinary success leaves the count helper's value in EAX. |
| `004D26A0..004D27B3` | ECX actual destination list; seven stack DWORDs: insertion-position owner/node, current-source owner/node, source-end owner/node, and one unread DWORD; RET 1C. No semantic return is established. |
| `004D2746..004D27A0` | Parent-bound catch inside the preceding span. Requires its parent's EBP, locals and mutable stack arguments; not an independent callable helper. |
| `00B30510..00B305B1` | ECX actual destination 2Ch-byte record; one stack source-record pointer; EAX same destination; RET 4. |

`00B30510` pushes the captured source sentinel for the unused seventh range
argument. A template tag is plausible, but its source-level type is unproven.
Preserve the observed seven-DWORD native stack contract when making a native
test bridge; a new C++ interface need not invent a semantic use for that word.
Proposed names are descriptive: alias-list append, alias-list insert range,
and native render resource-record assignment. Saved names/prototypes and
comments are retained in the audit; no Ghidra annotations were changed.

## Single insertion: `004D2660`

The routine captures the actual destination sentinel once, then captures that
sentinel's previous node. It calls complete `004CE6F0` with next=sentinel,
previous=captured previous, and the source string header. That helper allocates
and constructs an **unlinked** node. Only after it returns does `004D2660` call
`004CE780(actual destination, 1)`.

On count-growth success, write captured-sentinel.previous=new node; then reload
new-node.previous and write that node's next=new node. Do not reload a different
destination sentinel after allocation, publish either link before count growth,
or discard the second current-previous read.

This routine has no local EH frame or node cleanup. If allocation/copy itself
fails, `004CE6F0` owns its documented raw-node cleanup. If the later count-growth
operation throws, this caller performs no release of the already allocated,
unlinked node or its string. A guard that frees that successful node on every
insertion failure would change the native behavior.

## Range insertion and rollback: `004D26A0`

At entry the function captures the actual destination owner, insertion node,
source owner, and current source node. It copies the initial source iterator
into locals `EBP-1C/-18`; the working source iterator lives in mutable argument
words `EBP+10/+14`. The insertion iterator lives at `EBP+8/+C`. The source-end
pair is at `EBP+18/+1C`.

Each normal iteration checks the captured current-source owner against null
and the current end-owner argument. A failed check calls returning-capable
`00BF6713`; it then compares current source/end node words. Equality terminates
even after a returning owner-validation call. If work remains, it performs a
second null-owner check and checks current source node against that owner's
current sentinel before dereferencing the node's string at `+8`.

It allocates a copied node before the fixed insertion node, calls count growth
with one, and publishes the two links in the same order as single insertion.
It then reloads the source-owner argument, checks the captured source node
against its current sentinel, calls the handler if needed, and loads that
captured node's **current next pointer after the handler**. Only then does it
write the working source-node argument and continue.

The normal loop does not compare insertion-position.owner with the separately
supplied actual destination owner. The insertion node is used directly; its
position-owner word becomes relevant in the rollback helper calls.

These normal-loop checks are inlined. Do not replace them indiscriminately
with the checked-iterator helper calls: the inlined loop keeps its owner/node
register captures at sites where a standalone iterator helper would reload an
iterator word. In particular, repairing a caller's by-value iterator does not
replace the loop's already captured owner. A returning handler is not an
authorization to skip the subsequent native access or add a second check.

The handler at `00C65EB0` selects FuncInfo `00D8EB30`. Its two unwind states
at `00D8EB0C` both transition to -1 with no cleanup action. The single try
record at `00D8EB1C` covers state 0, has catch-high 1, and points to the catch-all
record `00D8EAFC`, whose handler is `004D2746`. The normal loop publishes state
0 before validation. No node-specific unwind action appears in this map.

The catch compares the saved initial source iterator with the working source
iterator through complete `004BE820` (this helper returns **not equal**). If
they compare equal, it immediately rethrows. Otherwise it captures the current
insertion owner/node and actual destination owner, then repeats:

1. Restore the fixed insertion iterator into its argument words and call
   `004BECC0` on those actual words to move to its previous node.
2. Read the returned iterator's node and owner, pass both by value to
   `004D0990` on the actual destination list, and use the original source-end
   argument words as the output iterator storage. These end words are overwritten.
3. Advance the saved initial **source** iterator through `004B9FF0`, then use
   `004BE820` to compare it with the working source iterator. Repeat while unequal.

Finally it calls native `__CxxThrowException(0,0)` to rethrow. This is explicit
catch-body rollback, not a no-throw destructor guard. Returning validation can
affect the iterator words that the existing helpers reread; an exception from
rollback itself can leave its partial work and supersede the original throw.
Do not infer an active nested catch solely from a raw parent state word.

Rollback is governed by **source cursor progress**, not a stored inserted-node
count. With an independent stable source and all helpers returning normally,
each completed source advance causes one removal from immediately before the
fixed insertion node. Important failure distinctions follow directly:

| Failure point | Local native consequence |
| --- | --- |
| Allocation or embedded-string construction | The node helper provides its own cleanup; range rollback handles only earlier source progress. |
| Count growth after a successful node construction | The current unlinked node is not part of rollback and is not released here; earlier completed progress may be rolled back. |
| A throwing post-link sentinel-validation handler, before source advance | There is one more linked insertion than recorded source advances. With otherwise stable independent iterators, rollback removes only the number of completed advances from the end, leaving the earliest inserted node; the first-iteration case removes nothing. |
| Validation or erasure during rollback | Preserve the helper's current partial effects; there is no stronger all-or-nothing guarantee. |

There is no source/destination-owner disjointness check. Shared-list ranges,
overlapping owners, or handler-induced graph mutations can change traversal and
rollback, including making a saved source cursor refer to freed storage. Do
not introduce a source snapshot, assume rollback is a simple reverse count,
or advertise self-range safety beyond the actual native accesses.

## Resource-record assignment: `00B30510`

The name prefix occupies record `+0/+4`. Unless the two record addresses are
equal, resize the actual destination prefix through `0041DD40` using the
source's length captured before the call and preserve=true. After it returns,
read source length again as the nonzero guard; copy the **current destination
length** bytes from the current source data to current destination data using
`memcpy`. The zero/equal-length quirks and allocator-driven current-field
reloads belong to the existing resize contract. This is live assignment; using
the fresh copy constructor would clear and abandon the old name first.

The raw-prefix composition must preserve equal-length resize as a no-op even
with null data. A changed zero length releases old data with old length plus
one and clears both fields. A nonzero request allocates request plus one before
release, preserves the minimum of requested and current old length, rereads
current old data/length for release, and publishes the new fields/terminator.
It must act on the actual record fields throughout.

For distinct embedded list addresses, capture source sentinel and its first
node **before clearing the destination**. Call `004D05E0` on the actual
destination list, which retains its sentinel. Afterwards reload destination
sentinel and its current next node; that current next node is the insertion
position. Call range insertion with the actual destination owner, captured
source first/sentinel pair, and the extra captured-sentinel DWORD.

The clear operation captures the first node before resetting sentinel.next;
it reloads the sentinel before resetting previous, compares initial emptiness
against the current sentinel, then publishes count zero.
Each loop captures the node's current data and next, returns nonnull data with
node length plus one, frees the captured node, and compares captured next with
the **current sentinel after those calls**. It keeps the sentinel allocation,
owner's leading word, name and tail. The whole record destructor is unsuitable.

The source endpoint is not refreshed after destination clear. Likewise,
destination insertion position is not assumed to equal the sentinel after
allocator activity. The routine does not preserve a copy of the old name or
aliases if this later insertion throws, and has not yet reached its tail stores.

After the list work succeeds, copy six current source DWORDs to destination
offsets `14,18,1C,20,24,28` in that order. The final word is the existing
resource pointer. There is no AddRef, release, virtual call, resource lookup,
or destruction of the overwritten pointer in this routine. Exact self-assignment
skips the name and list work but still executes all six tail stores; do not
replace the whole function with an early identity return or a bulk record copy.

## Dependency closure and implementation readiness

| Dependency | Current evidence and remaining integration |
| --- | --- |
| `004CE6F0`, catch `004CE75C`, `0044BCB0` | Concrete actual-node and embedded pooled-string construction are present in `native_render_resource_alias_nodes.cpp`; reuse the same actual shared string pool and ordinary allocation service. No later insertion cleanup belongs to this helper. |
| `004BE820`, `004BECC0`, `004B9FF0`, `004D0990` | Concrete checked-iterator and actual-node erasure operations are present in `native_render_alias_checked_ops.cpp`; use the real list address, callback domain and shared pool. Their returning-handler continuations are part of rollback. |
| `00BF6713` | Bind the existing required `SingletonLifetimeCallbacks.invalid_parameter`; preserve both return and throw. The native encoded handler/default termination ABI remains a runtime boundary. |
| `004CE780` | Named `STL_xlen_throw_004ce780`, but no implementation is present at this snapshot. It increments count on success and constructs/throws a length exception on failure; its name alone is not readiness. Primary-owned follow-up. |
| Six legacy SBO operations | Present after the string packet, including retry/unwind paths. These are for the exception message, not record names or alias strings. Root commit `52012c4` removes convenience default member initializers so raw placement initialization preserves unwritten leading words. |
| `00411700`, `00411780`, `004117C0`, `004118D0`, `00411940` | Named or discovered legacy 28h-byte exception-owner/dispatch/copy dependencies, in a separate worker packet. They must be distinguished from host `std::length_error` construction and the original CRT throw/copy ABI. |
| `00BF6885`, native ThrowInfo/type/copy records | Recognized CRT `__CxxThrowException@8`; preserve the library identity. A real semantic runtime adapter must account for construction, copy, cleanup and rethrow boundaries. Do not pass a new C++ wrapper as an original exception object or replace all failure construction with an unqualified host throw. |
| `0041DD40` | Complete pooled resize behavior exists, but `NativeString` owns private fields; it cannot borrow the resource record's raw name members. Add an actual-prefix view/composition of that behavior. Do not copy to a temporary header or overlay a separate C++ object's lifetime. |
| `004D05E0` | Root commit `f46f236` now exposes complete `clear_native_render_resource_aliases_004d05e0` on the actual list owner and uses it from record destruction. Reuse that API; do not call the whole destructor, which also frees the sentinel/name. The discovery base predates this integration. |
| `00419CC0`, `00BD1120`, `00BD1510`, `00BF681B`, `00BF65AC`, `00BF7680` | Existing concrete shared pool/allocation/free/copy service boundaries. Bind the actual pool rather than creating another pool domain or a success-only allocator. |

The exact count check is unsigned `(1FFFFFFFh - current_count) < increment`.
On success it stores the wrapped sum. A corrupt preexisting count above the
limit can pass through subtraction wrap; do not strengthen the comparison.
Both insertion routines pass increment one. On failure, `004CE780` constructs
the 16-byte message `"list<T> too long"` from `00CE38F8` in a legacy SBO
temporary, constructs the legacy logic-error owner through `00411700`, installs
length-error vtable `00D69260`, and throws with `00D83F98`. This message exceeds
the 15-byte inline capacity, so its two message constructions can allocate.
FuncInfo `00D8E3D0` arms state 0 only after the first temporary's assignment
returns; unwind `00C65A80` destroys that temporary through `004072D0`. The
exception-owner and original CRT-copy boundaries require their own completion.

A useful next implementation split is single/range insertion once count growth
is concrete, then record assignment after range insertion and actual-prefix
name assignment, using the now available actual-list clear. The address
bands are not independent proof of readiness. Keep ownership of common source
files and the exception/count dependencies explicit.

The smallest independent pooled-name follow-up is the complete 164-byte
`0041DD40` operation, with a borrowed actual-header API such as
`resize_native_string_header_0041dd40(void* actual_header, NativeStringStorage&,
uint32_t length, bool preserve)`. Reuse `PooledStringStorage` for the shared
pool; let the existing `NativeString::resize_0041dd40` delegate to this
same body rather than duplicating it. No constructor or extra owner identity
belongs in the borrowed header interface.

The later name-assignment wrapper can expose the exact
`00B3051D..00B30545` exclusive fragment with actual destination/source header
addresses and the same storage service. It comprises identity guard, resize,
current source-length guard, and current-field copy. This is a fragment inside
pending full `00B30510`, not another completed record-assignment function.
Prefer completing the independent resize entry first if count/throw transport
still blocks full assignment. Suggested ownership is `native_string.hpp/.cpp`
plus one focused doc/audit, disjoint from the count and exception-owner files.

## Evidence and limits

The audit records complete primary spans, overlapping catch evidence, complete
dependency spans, EH maps, old annotations, source hashes, and exact readiness
states. All captured bytes were freshly compared between the verified existing
Ghidra project/program and the installed PE. Assembly supplied the missing
register inputs, seven-word stack contract, catch behavior, field reloads, and
the `004D0625..004D062F` clear-loop continuation hidden by `_free` analysis.

No C++ reconstruction, fixture, build claim, gameplay claim, Ghidra mutation,
shared metadata edit, or CMake change is made by this discovery packet. The
failure table is assembly/EH-map reasoning, not a new fault-injection result.
