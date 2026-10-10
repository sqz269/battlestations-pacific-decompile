# CC12 allocation-stats process cell and binding readiness

The smallest next Source change is **one appended, zero-initialized stats publication cell and reference accessor in `GameNativeStringProcess`**. Its existing retained allocation already owns the canonical manager cell and outlives application teardown and late CRT callbacks. No new manager or process owner is needed. Deletion bindings and startup allocation ownership remain separate changes; this readiness packet activates neither.

This read-only audit initially froze worker `4f09095097b4d8378413b4f20d4e66f420385b1a` and accepted main `7e33f2025db2ab6e0f496d29204ba9b3e5084627` while lifecycle acceptance was pending. **Root then accepted and pushed the lifecycle at `3431065a5e62032df97d006b7f205d50873e682a`, before this readiness packet's final freeze.** The final proposal uses that accepted lifecycle. The initial pending-status snapshots remain historical. All current relevant process/host/dispatch/cache/shutdown Source files still match; current CMake now registers the lifecycle. The sole difference among shared initial audited paths is an unrelated raw effect-context addition in `gameplay_effect_definition.hpp`.

The full accepted Source822/826-pin context is retained alongside Source819/823 pins, Source749/753 pins, and the immutable worker lifecycle archive. The latest [lifecycle primary review](CC12_ALLOCATION_STATS_RAW_LIFECYCLE_SOURCE_PRIMARY_REVIEW.md) records six fresh Root/worker object matches and three passing normal checks. This readiness packet retains that evidence without repeating or claiming those builds/tests as new work. Cell, finite binding and startup changes remain proposals pending their own review.

## Canonical cell placement

`GameNativeStringProcess` currently retains, in order: manager, string-pool publication, small-return-disable word, raw string context, actual string storage adapter, two distinct empty-byte authorities, and pending-registry publication. The raw/string adapters reference the original manager/pool/disable fields. `game_native_string_process()` returns an intentionally retained heap object through a trivial static pointer. Its existing first access may allocate the process object; its cells have no exit destructor that can invalidate later CRT users.

The proposed exact two-Source-file boundary is:

| File | Proposed change |
| --- | --- |
| `include/bsp/game_native_string_process.hpp` | Append `void* volatile allocation_stats_0109cefc_{};` after `pending_registry_00f878cc_`. Declare `void* volatile& allocation_stats_0109cefc() noexcept;`. |
| `src/game_native_string_process.cpp` | Define that accessor to return exactly the new field. |

Do not reorder fields, reuse an existing cell, change the process allocation policy, replace the existing string bindings, or add cleanup. The accessor must not create a stats receiver, call a manager getter, publish/register anything or install dispatch. This is durable publication storage, not ownership of the 12-byte receiver. A later implementation should check the existing Win32 build and emitted member layout to confirm the old prefix/cell identities remain; no compiler/layout proof is claimed here. These files are already registered in the build, so this boundary needs no new translation-unit registration.

## Separate finite deletion binding

The raw lifecycle is now accepted. After the process-cell accessor and this binding proposal receive their separate review, a second Source packet can change exactly `include/bsp/game_hosts_singletons.hpp`, `src/game_hosts_singletons.cpp`, `include/bsp/native_singleton_destruction.hpp`, and `src/native_singleton_destruction.cpp`.

Append a `NativeAllocationStatsConstructorContext` to `GameSingletonHost`, binding its existing canonical `manager_publication_01090aa0_` reference and the new retained stats cell. Expose this same context for future explicit construction and cache-cell borrowing. Append a context pointer to `NativeSingletonDeletionBindings` and install it in the host constructor before any stats publication/registration. This context and table stay alive through both normal shutdown and the host destructor's fallback shutdown. Never bind a temporary context. The frozen table is 188 bytes, ending with its pending-registry pointer at 184; an appended x86 pointer is expected at 188 with total size 192, subject to actual next-packet compilation and preservation of every existing offset.

| Current popped profile | Genuine Source scalar to admit |
| --- | --- |
| `D685E0` | `delete_native_allocation_stats_base_00be2890` |
| `D685F4` | `delete_native_allocation_stats_00be2930` |

Pass the actual popped receiver and provided flags unchanged with the retained context. Do not require receiver/publication equality or choose a replacement receiver from the publication. Both accepted Native scalar schedules call BE27F0 directly before testing bit0/freeing the original receiver. Missing bindings must retain the existing contract error, with no fallback metric, callback or arbitrary virtual call. Keep existing CE3818 dispatch unchanged. No separate BE28E0 body is implied.

## Shared drain and lifetime contract

`GameSingletonHost` already borrows the canonical process manager cell. Its raw BD0400 path shrinks/pops the current last vector slot before reading that owner's current profile and dispatching flags1. It skips null entries and rechecks the live count after each scalar, allowing established dynamic registration effects. Only after the vector is empty does it release the actual manager +10h section, free the current vector and zero +4/+8/+C. The host then retires VFS services, records observer state, frees the captured manager and clears its publication.

The new stats context must remain live through that whole sequence. BE27F0's first captured section remains usable while the manager drains; its second manager observation must borrow the same canonical cell. It unregisters the current stats publication, which need not be the popped receiver, then clears that actual cell only after unregister returns. Normal release uses the first captured section. Neither the new cell nor a binding changes those observation points.

The host is constructed before explicit observer/settings/pending CRT initialization; stats initialization currently appears later in phase0. Therefore stats is not necessarily the first registration or last deletion. Actual registration history and live recounting decide order. Future cache borrowers must be retired or remain valid according to the real owner order, not an invented last-stats rule.

Both explicit application teardown and destructor fallback call the renderer's guarded drain or `GameSingletonHost::shutdown`. Renderer draining retains Lua activation/services and marks itself failed on an exception. Input/sound/clock/VFS and host cells remain through the shared drain; the host's own destructor calls shutdown before destroying its members. Existing renderer/Lua/frame-clock/VFS process-retention guards do not establish a stats-specific failure policy.

An exception during finite dispatch occurs after the slot was popped. The existing manager catch invokes genuine BD0220, freeing/zeroing the remaining vector, and rethrows. The subsequent normal section release and host manager free/clear do not complete. This is not a safely retryable or recoverable drain contract. Binding the two stats profiles must not add repairs, retry, forced clear or receiver free after BE27F0 failure.

## Current stats consumers

| Consumer | Current behavior and remaining boundary |
| --- | --- |
| Raw constructors | Borrow the stats reference, publish the receiver, then observe the second manager before reloading current stats for registration. They create no permanent publication cell. |
| Accepted raw lifecycle | Reuses that context, unregisters current publication, clears only after success, and supplies both direct-base scalar routes. Root primary acceptance3431065a5 closes the earlier candidate status. |
| `NativeResourceLoadCacheContext` / B80720 | Borrows a stats cell. A cache hit returns before stats reads. A miss reloads current stats/profile/slot4 twice and writes wrapping starting-minus-ending to current resource+40; no stats null gate or owner synthesis exists. |
| Known metric adapter | Genuine BE2700 Source already returns its established 40000000 result. Unknown target values remain explicit other-provider calls. Do not replace this with a new constant callback. |
| Profile table data | Cache code dereferences numeric profile+4 before dispatching the identity. The existing verified read-only data service and declared `D60000` application band cover the derived table. Retain that service through its consumers; this does not create callable Native vtables. No mapping or PE read was performed here. |
| Particle/model paths | Raw particle resources and damageable class-model loading borrow an externally supplied cache context. `GameNativeParticleRuntime` retains that input and checks VFS and string/manager identities, but does not establish stats-cell identity or create an actual stats/cache owner. |
| Phase0 semantic state | `run_initialize_phases` still uses a local `AllocationStatsState`. Its helper writes a null profile, budget and count; it does not perform raw publication/registration. |
| Entity-think +C | Current adapters return false or record unimplemented through the abstract predicate. They do not read actual stats and are not recovered BE2740 behavior. |
| Allocation report/+8 | Only descriptive BE3FD0/comment and semantic field initialization are present in the searched Source domain. No raw reporting implementation is established. |

Future concrete cache composition must borrow the same canonical stats cell and the genuine existing metric/VFS/resource providers. Numeric base-profile slot4 behavior beyond admitted evidence, +8 reporting and +C predicate behavior remain distinct gates. The current false entity-think adapter must not be treated as permission to invent that predicate when an actual stats owner appears.

## Startup remains a separate handover

After both deletion cases exist, a later packet can replace the scoped phase0 call with genuine `singleton_lifetime_allocate({SingletonAllocationKind::object,12,12})` and raw BE2900 using the same host context. It must preserve the retained allocation spill, conditional call at 0073D478, state0-before-call schedule and postcall comparison of 01090AB0 before state-1 retirement. The current semantic API should remain available until other users are deliberately migrated.

The retained conditional startup action frees the current saved allocation spill, not the publication. Constructor failure can preserve publication/registration while that allocation is freed. Consequently cell and binding existence cannot justify automatic rollback, clearing, unregistering, retry or fallback drain after such failure. A concrete failure/retention boundary must be settled before startup activation. Ordinary Source C++ cleanup also does not establish the Native parent OS frame, FH3 policy, private spill identity, register ABI or hardware-fault delivery.

## Immutable audit evidence

The [structured report](../reports/cc12_allocation_stats_process_cell_binding_readiness.json) retains complete searches, both Git/source contexts, exact next-file contracts and preserved artifact pins. The audit follows 21 worker roots into 750 repository-local files and 20 accepted-main roots into 748; the two external quoted Lua headers remain labeled compiler include boundaries, not newly audited providers. Three additional shutdown implementation witnesses are frozen whole. The sole initial closure attempt stopped at the external `lua.h` include; its helper is preserved and the final closure labels that boundary explicitly.

The local `cc12_allocation_stats_process_cell_binding_readiness` evidence directory contains an immutable manifest/archive and read-only full-pin/Git/query/nested-archive replay. Source749 remains intact inside the preserved candidate archive; Source819 and latest Source822 are copied in full from their separate frozen contexts. An additional 28 current accepted Git files and five full searches verify the lifecycle acceptance overlay, without replacing the initial snapshots. There are exactly two repository outputs and zero C++ edits, compiler/build/test invocations, fresh Native queries/windows/PE semantic reads, Ghidra mutations, startup activations or runtime checks. This establishes a ready cell-accessor boundary and a concrete subsequent binding contract, not a completed raw stats owner.
