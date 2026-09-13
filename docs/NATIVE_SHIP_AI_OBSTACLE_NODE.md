# Native neighbour-node construction and lifetime

Packet `orch6_native_neighbour_node_q`, base `522cabd1`, implements the raw
90h obstacle node constructor and its normal observer lifetime chain. The
existing semantic `ShipAiObstacleNode` and partial
`ship_ai_obstacle_owner_initialize_projection_009e52e0` are unchanged. They
are not aliases to this storage and do not gain native ownership implicitly.

`NativeShipAiObstacleNodeStorage` embeds the existing actual 10h
`NativeObserverOwnerStorage` at offset zero. It has no field defaults, owned
sidecar, duplicate observer manager, geometry initialization or implicit
destructor. The caller supplies the existing allocation, actual observed
endpoint and controller, raw lifetime word and shared `NativeObserverLifetime`.
For malloc-backed storage, start the trivial C++ object lifetime with
`::new (allocation) NativeShipAiObstacleNodeStorage` without parentheses or
braces; this default initialization leaves the allocation bytes untouched.
It must keep those owners alive and call the deleting destructor exactly once
when relinquishing an allocated node. Stack storage requires flags with bit0
clear and separate storage lifetime management.

## Coverage and native calls

| Routine | Verified native span (inclusive) | Coverage / native ABI |
|---|---|---|
| 009E52E0 | 009E52E0..009E53A5, 55 instructions | Complete normal constructor; ECX=node, stack owner/controller/lifetime word, EAX=node, RET0Ch at53A3 |
| 0064A610 | 0064A610..0064A667, 24 instructions | Complete normal common base destructor, exposed only for this raw node; ECX=base, RET |
| 0064B5F0 | 0064B5F0..0064B60D, 11 instructions | Complete scalar deleting wrapper; ECX=node, stack flags, EAX=original address, RET4 atB60B |
| 0064B5C0 | 0064B5C0..0064B5E6, 14 instructions | Complete owner callback through borrowed actual event slot+4; ECX=node, stack event, RET4 atB5E4 |

The functions expose new C++ interfaces, not drop-in binary ABI replacements.
Descriptive names are hypotheses. The existing constructor name and
`CG_scalar_deleting_dtor_0064b5f0` library identity are retained. The common
destructor is also called by other native classes; this packet does not
reconstruct their constructors or impose a 90h size on those receivers.

| Native site | Actual call / inputs |
|---|---|
| 009E532D | 00694A60: ECX=captured owner, EDX=actual node callback base; no stack arguments |
| 0064A644 | 006952A0: ECX=node+14 captured after base-table store, EDX=node |
| 0064A653 | 00695870: ECX=node callback base |
| 0064B5F3 | 0064A610: original ECX saved in ESI for later return/free |
| 0064B600 | 00BF65AC `_free`: push saved ESI; ADD ESP,4 atB605 |
| 0064B5CC | Actual event vtable+4: ECX=stack event, no extra stack arguments; EAX=first-endpoint identity |
| 0064B5DE | 006952A0: ECX=event getter result, EDX=node, after clearing node+14 |

All six direct CALL instructions are reported for the call gate. The indirect
call is documented separately rather than attributed to an invented callee.
The event producer and concrete slot+4 target remain external; the borrowed
provider must perform the actual call once, including its effects. No dummy
event, complete vtable, callback queue or observer dispatch owner is created.

The sole constructor call is 009F0E53 in 009F0D20. The caller allocates exactly
90h through 00BF681B at009F0E2F, cleans its one size argument at0E34, checks
the returned pointer, then pushes the produced lifetime, ESI controller and
EBP candidate owner. ECX is that allocation. It appends the constructor result
to controller+608 using the live count+604, then increments the count. The
constructor itself does not allocate its own backing. The existing allocator
boundary `singleton_lifetime_allocate({object,0x90,0x90})` supplies that raw
storage; allocation failure/new-handler history and caller allocation unwind
remain the caller's responsibility.

## Exact constructor writes and preserved bytes

The constructor writes 61 distinct bytes and preserves 83 bytes. Registration
separately changes actual edge arrays and their allocations.

| Native offset | Store / ordering |
|---|---|
| 04,08,0C | Three edge-array words zeroed at52FC/52FF/5302 |
| 14 | Zero at530A; nonnull captured owner assigned at532A before registration |
| 10 | Byte1 at530D; no broader enabled-state behavior inferred |
| 00 | Table identity00CF5C94 at5311 |
| 18 | Dword zero at5321 |
| 1C | Actual controller argument at5338, after registration returns |
| 78 | Input lifetime bits for captured nonnull owner, otherwise BF800000 (-1), at5359 |
| 69,88,75,8C,74 | Zero at5361/5364/536A/536D/5373, in this order |
| 68 | Byte1 at5377 |
| 7C | Positive-zero word at537B |
| 84,80 | C47A0000 (-1000) from00D7A240 at5380/5388 |
| 70 | Positive-zero word at5390 |

Unwritten intervals are `11..13`, `20..67`, `6A..6F`, `76..77`. Both boxes,
axes, headings and unused geometry words retain the allocator's preimage.
The API uses raw words for the lifetime/cache stores because the native
constructor uses MOVSS/XORPS without arithmetic or x87 evaluation. Signaling
NaN and negative-zero input bits therefore remain unchanged for a nonnull
owner. No default zero/NaN geometry, clamp or finiteness guard is added.

ESI retains the node, EDI retains the original owner and EBX remains zero
across registration. The nonnull lifetime selection uses captured EDI, not a
reload of node+14 after registration. The latter distinction is retained in
the C++ implementation.

## Destruction, callback and failure boundaries

CF5C94 slot0 is0064B5F0. It invokes0064A610 first; only normal completion
reaches flags&1 and `_free`. Even flags retain the allocation but still run
all observer cleanup. The returned original address is only an identity once
freed. No geometry or controller destructor is called.

0064A610 writes base tableCF5C20 before reading owner+14. A nonnull owner
is unregistered with the actual node callback base; owner+14 is not cleared
by this destructor. It then invokes existing00695870, which writesCE3CD4,
performs separately nested shared-lock lookups/count query, detaches any
remaining edges, releases its captured lock and frees the node edge array.
Existing pointer/count/capacity bytes are left as the native destructor leaves
them; they must not be reused as a live constructed owner afterward.

CF5C94 slot+4 is0064B5C0. It dispatches the supplied event's slot+4 first,
then compares the result with a fresh node+14 load. Inequality returns without
cleanup. Equality clears node+14 before calling006952A0 on the returned
endpoint. There is no added null guard. Inputs where equality yields a null
first endpoint violate the downstream valid-storage contract; this packet
does not invent recovery for that native invalid input.

The existing registration/unregistration routines capture their actual shared
recursive locks. Duplicate registration increments the shared edge reference
word. Unregistration first clears matching live dispatch entries, decrements
references and removes/deletes the edge only when it reaches zero. Callback
base destruction detaches all remaining edges irrespective of their references.
The raw API reuses those implementations and the shared runtime owners.

Constructor FH3 metadataCB0828 -> DE5A18 has state0 cleanupCB0820. It loads
the saved node and jumps through0064A8D0 to0064A610. Common destructor
metadataC7A938 -> DA7498 has state0 cleanupC7A930 ->00695870. Source C++
exceptions follow these cleanup calls, then propagate. Original FH3 frame
registration, native exception dispatch, asynchronous faults, second failures
during cleanup and allocation-failure behavior are not fixture-validated.
The normal-byte fixture executes the original prologues/epilogues and verifies
that FS:[0] is restored, but it never invokes the retained native EH metadata.

Root repaired the three false-free bytes0064B605..607 using the formal flow
tool, saved and refreshed the export. The instruction is correctly decoded
but these three bytes still lack Ghidra function-body membership: a live
query atB605/B607 finds no containing function. The server disables the
supported body-union mutation; root leaves that configuration unchanged.
Thus the zero instruction-flow-gap result does not establish a complete
Ghidra body set. The full30-byte PE span and source wrapper remain verified
separately. Q made no Ghidra changes.

Root also defined and saved the five adjacent RET stubs0042B110/120/130/140
and006935C0. Their exact ranges and root report are recorded; no missing
function entries remain. This does not supply a complete callable node or
event vtable. The incomplete deleting-wrapper body membership remains a
separate metadata limitation.

## Verification and limits

The ignored fixture relocates all 355 bytes of the four routines, six direct
calls and two literal references. The direct0064B5F0 ->0064A610 edge stays
inside the original-byte image. Register/unregister/callback-base destruction
share the existing reconstructed observer library on both sides, including
actual allocated edges, arrays, singleton lock construction and Win32 sections.
The original free call uses the same CRT free boundary. This establishes the
new callers' ordering/storage behavior, not independent fidelity of those
shared helpers or CRT allocation history.

The fixture supplies explicit empty endpoint prefixes and a borrowed dispatch
vector with two copies of each actual edge. It checks invalidation during
normal callback/destruction. Those fixtures are not original unit/event or
dispatch-singleton producers. The callable event slot is a provider that
returns a specified endpoint and can mutate node+14 before the comparison.

The paired cases cover null/non-null owners; ordinary, negative-zero, signaling
NaN, quiet-NaN and infinity lifetime words; mismatched callback; duplicate
registration; matching callback; mutation before callback comparison; remaining
edge detachment; even flags and separate odd-flag deletion. Whole node bytes,
endpoint fields, live edge identity/refcounts, dispatch slots and lock depth/
RecursionCount are compared. Only typed pointers are canonicalized to the
known node/endpoint/controller/array roles; all 83 untouched bytes remain
compared. The fixture asserts the actual controller, allowed owner identities,
edge endpoints and unchanged captured array pointers before normalization.
Array capacity tails, freed bytes, OS-private lock bytes, allocation
addresses/history and the singleton manager's private bytes are not compared.
Normal final lock counts, singleton shutdown, constructor x87/MXCSR status and
FS restoration are checked. Freed odd-flag storage is never inspected.

Exact result counts, source/library hashes and commands are in the report and
ignored `local/native_node_artifact_manifest.json`. The required Win32 build
and both existing CTests passed. The 20 paired sequences matched 17,300 record
bytes per side with zero mismatches; 15 original registration preimage checks,
20 original free calls and 60 event getter calls executed. No tracked tests are
added. No game run,
runtime consumer binding, observer event production or whole-game neighbour
lifetime claim is made.
