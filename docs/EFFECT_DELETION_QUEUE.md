# Effect deferred-deletion queue and lock lifetime

Packet `orch3_effect_deletion_queue_u` reconstructs eight complete functions in
`src/effect_deletion_queue.cpp`, using the actual 28h live manager and its existing
0Ch list header/node layouts from `native_live_effect_manager.hpp`. Descriptive
names are hypotheses. The correct compiler scalar-destructor name is retained.

| Span (exclusive end) | Original ABI | Behavior |
| --- | --- | --- |
| `00866040..00866051` | ECX lock owner; RET | Clear F87654, restore CE3818 |
| `008662B0..008662F5` | ECX owner; EAX same owner; RET | Construct D0D3D0 lock owner |
| `00866500..008665BD` | No inputs; EAX published owner; RET | Lazy deletion-lock getter |
| `00866790..008667C7` | ECX owner, stack flags; EAX original; RET 4 | Destroy lock and optionally free owner |
| `008665F0..00866623` | Stack next, previous, source cell; EAX node; RET 0C | Allocate raw payload node |
| `008675E0..00867673` | ECX list, stack increment; RET 4 | Check and increment count |
| `00868010..008680A7` | ECX live manager, stack raw effect; RET 4 | Enqueue deferred deletion |
| `008671A0..0086720D` | ECX live manager; RET | Flush current pending list |

## Separate lock owner

The deletion lock is an 8h owner published at `00F87654`, with native identity
`00D0D3D0` and owned section at +04. It is separate from insertion's F87650 lock
and the 28h live-instance manager at F8765C. The C++ interface reuses the existing
two-word `EffectManager` layout and `EffectManagerLifetimeAccess`; each publication
cell names its own actual owner, section and tracked +18 depth. The concrete
access uses the application's same `SingletonLifetimeDomain`, not a new domain.
Its actual registered-owner callback must route this owner to 866790.

Construction writes D0D3D0 before canonical BD1860 section creation, then publishes
the returned section. Creation failure runs 866040: clear F87654 unconditionally,
restore CE3818, preserve +04. Getter 866500 first returns a captured nonnull global.
On the slow path it captures lifetime-manager +10 section, enters/increments it,
rechecks the global, allocates 8h, constructs a nonnull result, and publishes it.
It obtains the lifetime manager again, reloads the global for BD0C30 registration,
then unlocks the captured section and reloads the global for its return. A failed
construction frees the captured raw owner before unlocking. Registration failure
does not roll back publication. A null allocation result is published/registered
as null, following the existing access contract.

Scalar deletion resets D0D3D0, destroys/clears the actual owned section through
41CC80, clears F87654 and restores CE3818. Flags bit zero additionally frees the
owner; other bits do not request a free. EAX is the original address even after
free. The lifetime domain already removes its entry before dispatch; this scalar
routine does not unregister itself.

## Queue behavior and callback order

8665F0 allocates a 0Ch node and independently tests the computed addresses for
next, previous and payload writes. It reads the source pointer cell only after
allocation and link publication. It does not retain the raw payload. ECX is not
consumed. The normal allocation domain supplies storage or throws; malformed
null-address memory faults are not a supported C++ interface.

8675E0 captures count and compares unsigned `3FFFFFFF - count` against increment.
The subtraction and final addition wrap as DWORDs, including for corrupt input
counts; this is not replaced by a saturating or generalized overflow check. Its
normal path writes captured count plus increment. The old heuristic name
`STL_xlen_throw_008675e0` missed this normal-path mutation and is replaced with
`BSP_EffectDeletionList_GrowCount` while preserving the prior annotation history.

The error branch initializes the observed legacy SBO fields, assigns the counted
16-character `list<T> too long` message through 408720, and arms temporary cleanup
only after assignment succeeds. It constructs the existing actual 28h legacy
logic-error owner and changes its identity to D69260. The reconstructed throw
reuses `NativeAliasListLengthError`, the existing owning transport for this same
length-error payload/copy/destructor. This has host RTTI and exception ABI; native
throw-info D83F98 and original exception dispatch are not executed.

868010 obtains the separate deletion lock even for a null payload. It captures
the current sentinel and its previous node, allocates the node using the incoming
argument cell, then grows count by one. Only afterward does it write captured
head->previous and new-node->current-previous->next. It holds the captured real
section across allocation, count growth and splice. Its sole unwind state unlocks
the section. Count-overflow failure leaves the already allocated node unlinked;
there is no node rollback or payload release in the native handler.

8671A0 has no internal lock. While current count is nonzero it captures current
head->next, calls BF6713 if that is the sentinel, then reads the captured payload.
A nonnull payload receives its current virtual +04 scalar destructor with flags
one; afterward the captured payload cell is cleared. It then reloads head/front,
checks the sentinel again, and compares the captured front against the current
head after a returning validation callback. A nonsentinel node is unlinked using
the observed current neighbor loads, freed, and current count is decremented.
The next loop reloads count. Callback changes can therefore select a different
node for unlinking than the one whose payload was destroyed.

`EffectDeletionDispatch` requires actual current virtual +04 dispatch for the
exact raw owner and a real BF6713 binding that may return. It introduces no
reference decrement, virtual-zero substitute, owner map or success fallback.
Applications must supply their established concrete effect scalar bindings.
Captured nodes must survive through the native final clear/access.

## Saved analysis and validation

Analysis/export batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All eight full body spans and four handlers matched
the installed executable. Hashes, prior annotations and validation are in
`reports/effect_deletion_queue.json`. The three-byte scalar free continuation at
8667BE was restored under the write lock; the complete stored body includes RET4.
`reports/effect_deletion_queue_flow_u.json` records that repair.

Four ten-byte compiler handlers were defined and saved:

| Handler | FuncInfo / map | Unwind |
| --- | --- | --- |
| C94CE8 | DC6AE8 / DC6AE0 | State 0 -> -1: C94CE0 loads saved owner, jumps 866040 |
| C94D33 | DC6B50 / DC6B40 | State 1 -> 0: C94D28 frees saved raw owner; state 0 -> -1: C94D20 / 411EE0 unlock |
| C94EA8 | DC6D28 / DC6D20 | State 0 -> -1: C94EA0 / 4072D0 destroys completed message |
| C94FB8 | DC6E70 / DC6E68 | State 0 -> -1: C94FB0 / 411EE0 unlock only |

Strict Win32 build and both existing CTests passed. No permanent tests were
added. Ignored `local/effect_queue_probe_u.cpp` compares all eight copied original
bodies with the reconstruction. It checks real lazy lifetime registration and
shutdown, getter fast path, scalar flags 2 and 1, base field preservation, nested
Win32 lock depth, raw/null enqueue, normal flush order, captured-slot clearing
after a callback changes the front/count, a returning invalid-parameter handler,
and unsigned count boundary/wrap cases.

Original allocator/free/import operands are rebound to canonical allocation/free
and Win32 Enter/Leave. Getter lifetime/section bridges expose the actual native
section addresses of the same concrete domain and owner; registration uses the
actual published native owner. Constructor, getter, enqueue and node/grow calls
use the copied native bodies. F87654 operands point to the fixture's publication
cell. Instrumented scalar callbacks verify current virtual +04 and flags, leave
the actual +04 count unchanged, release their owned payload, and retain dead
fixture wrapper storage for observation. This proves queue dispatch/order, not
the still-separate game effect scalar implementation.

The ignored build rebinds allocation/free symbols only in this production module.
Successful operations delegate to the canonical allocator. Focused C++ failures
verify node-allocation unlock, overflow's unlinked allocation and owned legacy
error payload, and constructor base cleanup/raw free before lifetime unlock.
The probe frees the intentionally unlinked node after checking it; production
does not add that rollback. Native handler immediates remain unchanged and native
exception dispatch is not executed. New C++ ABI and gameplay remain unvalidated.

Whole live-manager getter/destructor/scalar composition and point-effect virtual
zero's frame-job-policy decision remain next dependencies.
