# Allocation-statistics startup ownership handover readiness

The existing Source allocation and free providers are sufficient for the actual
12-byte receiver. Startup handover is held: Source still lacks the allocation
statistics process publication cell and complete lifecycle/deletion binding
needed to keep that receiver valid through the canonical singleton drain.
The standalone constructor is available at worker commit
`08748971b356b38ed2bbecd38d36ead738452476`; it does not supply ownership.

This is a Source-only readiness audit. Exactly this document and its report are
changed. No implementation, CMake registration, startup edit, build, test,
Native query/window, installed-image read or Ghidra mutation was performed.
The separately frozen accepted-main context is
`f95dc28b7371449a488bdf88ee8ffb5599113bc5` (Source747). Root's constructor
integration is independent and was pending when this audit began. The earlier
Source746/750-pin compilation context remains immutable; neither it nor its
build results is relabelled as a new Source747 build.

During completion Root reported constructor acceptance/push at
`e9396b4883232059bdad28753bc95403536403bf`, with its separate Source749 frozen
context and existing checks passing. Root requested that this audit retain its
original baselines. That integration status supplies no stats lifecycle or
startup handover credit and no new build claim for this readiness packet.

## Existing Source contracts

| Contract | Evidence and readiness |
| --- | --- |
| Receiver allocation | `singleton_lifetime_allocate` in `src/singleton_lifetime.cpp` exposes a raw `SingletonAllocationRequest`. The appropriate existing request is `{SingletonAllocationKind::object, 12, 12}` on the required MSVC Win32 target. Its ordinary malloc/new-handler retry and `bad_alloc` behavior already model `00BF681B`; no new allocator or callback is needed. |
| Receiver free | `singleton_lifetime_free` uses the matching CRT free service. This is the available Source release boundary, not an assertion of the original executable's heap identity or `_free` machine ABI. |
| Actual constructor | `NativeAllocationStatsConstructorContext` borrows the actual manager and stats publication cells. The standalone base/derived routines write actual receiver words and use genuine getter/registration/guard/base-cleanup providers. They allocate neither the receiver nor an owning stats cell and add no receiver free. |
| Canonical manager | `GameNativeStringProcess::manager_01090aa0()` owns the process manager cell. `GameSingletonHost` borrows it and exposes it to consumers. Its shutdown drains the same raw manager, then frees that manager and clears its publication after a successful drain. A private manager is unnecessary and would change the domain. |
| Canonical deletion | `NativeSingletonDeletionBindings` is a stable borrowed finite dispatch map. The raw drain pops the slot, reads the popped object's current profile, and dispatches flags one. Unknown profiles or missing bindings throw a Source contract error. Neither `D685E0` nor `D685F4` has a case in either frozen context. |
| Actual stats owner/cell | Not established in the inspected Source domain. The new constructor and `NativeResourceLoadCacheContext` borrow `void* volatile& actual_allocation_stats_0109cefc`; neither owns a permanent cell or the receiver. Entity-think references are abstract/held consumers, not a storage owner. |
| Existing startup | `GameStartupHost::run_initialize_phases` still creates local `AllocationStatsState allocation_stats{}` and calls the semantic constructor. That routine writes a null profile, budget `40000000`, and zero startup count. It does not perform raw publication/registration. |

The cache's genuine `BE2700` metric is already available; replacing it with a
constant callback, constructing a fake callable vtable, or deriving a new
allocation policy is not part of the missing contract. Original numeric
profile words identify admitted Native profiles; they are not rebuilt C++
function tables. The existing private/public binding pattern requires bindings
before the constructor can publish/register, with the same cell and contexts
alive through the complete drain.

These findings are bounded by complete sorted searches across tracked
`src/` and `include/` at both frozen commits, plus full snapshots of the named
providers and startup/deletion contexts. They do not assert absence in every
repository artifact, untracked file, binary, or external runtime. All 25 common
audited files have identical Git bytes between worker087 and accepted mainf95;
the two constructor files are absent at mainf95. The six retained queries
contain 30/17 stats-related lines and zero lifecycle/action implementation
matches in the respective Source domains.

## Retained startup allocation and failure schedule

The already accepted startup prefix gives entry ESP `S`, quiescent ESP
`Q=S-148h`, allocation spill `Q+18h=S-130h`, and state sentinel
`Q+144h=S-4`. The explicit `XOR EDI,EDI` at `0073D433` supplies the zero
comparison only under the intervening callees' ordinary nonvolatile-EDI
contract. This audit adds no new callee or hardware-fault evidence.

The retained 35-byte window `[0073D45D,0073D480)` requests 12 bytes from
`00BF681B`, stores returned EAX in the allocation spill, compares against EDI,
and stores EDI into the state sentinel before the conditional `00BE2900`
call at `0073D478`. The allocator's ordinary Source behavior returns storage
or throws; that fact does not license deleting the retained Native null branch
from the documented caller schedule. A throw before allocation returns creates
no returned receiver to free.

After the constructor/skip path, `OR EBX,-1` is followed by the retained
`CMP [01090AB0],EDI` and only then the sentinel store of EBX (`-1`). The
global comparison therefore precedes retirement of state zero. No global data
was read here; a hardware fault at that load remains outside the ordinary
Source C++ contract. Moving the state retirement earlier would erase this
Native ordering distinction.

The retained descriptor has 49 states, but only its state-zero record was
opened: transition to `-1`, action `00C86A30`. The other 48 records remain
unread. The action's complete **saved** extent is twelve bytes ending after
`MOV EAX,[EBP-130h]; PUSH EAX; CALL 00BF65AC`. Separately admitted physical
bytes at `00C86A3C` establish `POP ECX; RET` after an ordinary returning
cdecl free. The argument slot is discarded, the original action return
`00C07B37` is consumed, and helper execution resumes with ESP `T-1Ch` for
helper-entry ESP `T`. The pop need not recover the original free-argument value.

That action reads the **current spill word**, not current `0109CEFC` and not
an independently recovered allocation identity. Calling it a free of the
original allocation additionally assumes that the spill retains the returned
pointer. The two physical tail instructions add no publication clear,
unregistration, or additional cleanup call. Free's internal effects, successful
release and fault paths remain outside this bounded evidence.

The accepted internal helper/prologue composition gives action EBP equal to
forwarded raw argument two plus `0Ch`. Substituting the startup parent's
registration node for that raw argument still requires the unresolved OS-entry
association. The prologue's own FS node is distinct. A future Source C++ guard
can implement a deliberately stated conditional cleanup contract; it cannot
establish this OS association or Native FH3/SEH delivery. Saved `CALL_RETURN`
at `00C86A37` remains preserved, no listing extent was repaired, and the six
probe bytes after the interpreted `POP; RET` remain uninterpreted.

## Publication, destructor and scalar-delete constraints

The standalone constructor already preserves the retained two-getter schedule:
capture the first manager's section, enter/increment, publish the receiver,
resolve the second manager, reload **current** stats publication and register
that value, then release the first captured section. It must not substitute
the original receiver for the reloaded registration subject or assume the two
manager results coincide. On an ordinary Source C++ constructor escape its
armed guard cleanup precedes generic-base reset. Completed publication and
registration effects remain. There is no constructor rollback, unregister,
publication clear or receiver free.

Consequently an admitted outer startup free must not silently erase publication
or registrations. It frees the allocation-spill subject, after applicable
constructor cleanup; it is not the allocation-statistics destructor. If an
escape occurs after publication or registration, stale externally visible
state can remain. No claim is made that retrying startup or continuing the
canonical drain after such failure is safe. Adding rollback to manufacture that
guarantee would change the retained schedule.

The retained complete ordinary `00BE27F0` body captures the incoming receiver,
stamps `D685E0` before state zero, captures/enters/increments the first manager's
section, arms state one, resolves the second manager, reloads current stats
and unregisters it through `00BCFCA0`. Only after normal unregistration does
it clear stats publication, then decrement/leave the first section and finally
stamp `CE3818` on the original receiver. It does not read the budget/startup
words. Its referenced handler `00CC6AB0` and failure actions remain unopened.
The constructor's accepted cleanup actions do not prove this destructor's
different failure cleanup.

Both retained scalar schedules (`00BE2890` for `D685E0`, `00BE2930` for
`D685F4`) capture the incoming receiver, call `BE27F0` first, test flags bit zero
only after that call returns, conditionally free the original captured receiver,
return that receiver and use `RET 4`. On a destructor escape, ordinary control
does not reach the flag test/free. Freeing unconditionally in a catch/finally
would add behavior. The accepted physical scalar bodies include their separate
three-byte `ADD ESP,4` gaps; their saved split extents/flow overrides remain
unchanged. These are retained physical/ordinary contracts, not a new ABI test.

## Smallest remaining handover contract

1. Close the actual `BE27F0` failure/cleanup contract, beginning with its exact
   referenced `CC6AB0` handler metadata and only the descriptor/actions thereby
   justified. This is a future evidence gate requiring its own admitted scope;
   no new Native address window was opened by this audit. Ordinary destructor
   and both slot-zero identities are already retained and need no speculative
   adjacent-family replacement.
2. Implement the complete raw destructor and both scalar-delete schedules with
   genuine current-manager/unregister/captured-section/free providers. Specify
   ordinary Source C++ escape behavior and keep original ABI, hardware-fault
   and OS-delivery limits explicit. No new generic callback is necessary.
3. Provide one durable stats publication-cell domain and lifecycle context tied
   to the existing canonical manager cell. Install both actual profile deletion
   bindings before constructor publication/registration and retain those exact
   bindings through drain. Construction, cache readers and deletion must borrow
   the same stats cell. Receiver storage must be aligned, valid for three DWORD
   accesses, and separate from contexts/cells for their full required lifetimes.
4. Then replace the semantic startup local with the existing 12-byte allocation
   provider plus the accepted constructor, preserving null/state ordering and
   stating the conditional Source cleanup boundary. Successful construction must
   leave deletion to the admitted manager/domain contract, not a scope-exit
   local deleter. The outer failure action must retain spill identity and
   publication persistence; it cannot claim Native exception routing.

The missing unit is a coherent stats lifecycle/publication/delete domain, not
an allocation wrapper. Steps 1-3 precede startup wiring. Nothing in this audit
adds C++ implementation credit, fresh compilation/linking, Native execution,
ABI compatibility, hardware/SEH correctness, startup success or gameplay proof.

## Reproducible evidence

The ignored packet directory contains 52 full Git Source snapshots (plus 27
worker working-byte snapshots), two complete tracked-source path lists, six
complete sorted query outputs, 18 full prior documents/reports and all 32
retained lifecycle artifact receipts. Both prior immutable constructor/tail
archives and manifests are copied whole; all 2,903 named payloads are checked.
The constructor archive continues to carry the frozen Source746 evidence.

`prepared_inputs.json` pins every input. `verify_evidence.py` replays complete
Git blobs/search outputs, Source boundaries, retained pins and nested ZIP
payload hashes without Ghidra or installed-image access. `freeze_evidence.py`
packages full inputs, scripts, this document and the report. Its manifest and
archive pins are recorded out of band in `final_verification.json` to avoid a
self-referential report. Initial collection attempts only located lifecycle
receipts under their existing earlier frozen packet; the successful preparation
uses those preserved bytes and changes none of them.
