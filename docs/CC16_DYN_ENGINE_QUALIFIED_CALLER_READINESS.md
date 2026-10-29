# Dyn engine qualified caller readiness

The actual matched-heap supplier and qualified Root interface are available, but **complete nested cleanup is not ready in the current engine call chain**. C37740 still exposes only its normal constructor and omits two native constructor-unwind actions. The smallest coherent path is a bounded qualified task-construction prerequisite, followed by additive C55EA0/C55F50 overloads. No production change, build or execution is made by this readiness packet.

The source baseline is the exact clean integration HEAD resolved at task start: `64cd1530ec223a889d8fb88fa32a0c880ab72c6c`. This is an explicit integration snapshot, not an assumed published-main revision. It contains Root90e4 and reviewcb53, the actual1e69 default supplier and unactivated raw helper source. This packet owns only C55EA0/C55F50 read-only address leases and its two evidence files. No frozen runtime tree was synchronized, merged or changed.

## Complete engine allocation scopes

Original C55EA0 is175 bytes/57 instructions, stack `(engine*,descriptor*)`, EAX=engine, RET8. C55F50 is111 bytes/34 instructions, ECX=descriptor, EAX=existing/new/null engine, plainRET. Live bytes, original PE bytes and saved instruction-address lists agree. These are original private ABIs; new C++ argument/reference/catch transport remains separate.

C55EA0 first zeros only engine `+0/+4/+8`. It leaves `+C/+10` untouched until their native stores. Handler CC8403 selects FuncInfo E042CC: magic19930522, maxState3, map E042B4:

| State | Previous/action | Exact acquired extent and identity |
| --- | --- | --- |
| 0 | -1 / CC83E5 | Armed at C55ECB **before** the9Ch allocator C55ECF. Reload original engine argument `[EBP+4]`, tail-call409B10; capture its **current** data word and conditionally BF6989-free it. No field clear. Covers both allocations and later constructor work until return, except initial three zeros. |
| 1 | 0 / CC83ED | Capture returned9Ch raw profile at C55ED7, arm1 at C55EDD, conditionally call Root at C55EE7. Free captured `[EBP-10h]` through BF65AC, unconditionally as an action. Demote to0 at C55EF1 **before** engine+C store C55EFA. |
| 2 | 0 / CC83F8 | Allocate358h at C55EFD under state0; overwrite the capture at C55F05, arm2 at C55F0B. Includes descriptor dereference C55F16, C37740 call C55F1A, engine+10 store and both return paths. No state demotion before FS restoration. Free the captured raw task address, then unwind state0. |

The two raw profiles/tasks use the same original E-10h captured slot at different acquired states (entry ESP=E). Neither free reloads engine+C/+10 or a callee's returned pointer. State0 instead reloads the original incoming engine argument, then the current vector data. Calling it a full engine destructor would be incorrect.

Descriptor loading must remain after the358h allocator callback returns nonnull and after state2 is armed. Native C55F12 loads the descriptor pointer; C55F16 snapshots its first DWORD before C37740. Use a single explicit volatile DWORD snapshot at that epoch in the new qualified path and verify emitted placement. The null task branch skips both descriptor access and child construction, writes engine+10=null and returns. The profile pointer has already been stored at engine+C before task allocation; callbacks may observe that state and alter the later descriptor value.

C55F50 handler CC8418 selects E042F8, maxState1, map E042F0 `{-1,CC840D}`. It reads engine publication0109E9FC once at C55F66. A nonnull capture returns immediately without descriptor dereference or allocation. A null capture allocates14h at C55F74 under state-1, captures its returned address at C55F7C and arms0 at C55F82. State0 covers conditional C55EA0, publication at C55F93 or the null-allocation publication at C55FAA, and return. Action CC840D unconditionally BF65AC-frees captured `[EBP-10h]`; it performs no engine destructor or publication clear. No post-callback fast-path recheck is present.

## What failures leave behind

| Failure epoch | Native cleanup after the failing child has unwound |
| --- | --- |
| 14h engine allocation | No acquired ensure cleanup. Callback effects remain. |
| 9Ch profile allocation | Current engine-vector free; then enclosing captured14h free. No returned profile allocation has been acquired. |
| Inside Root, including its own48h allocation | Root performs only its acquired actions, if any; then captured9Ch free, current engine-vector free, captured14h free. No profile publication clear. |
| 358h task allocation | Current engine-vector free and captured14h free. The completed9Ch profile/48h Root remain allocated and profile-published. |
| Descriptor access / task construction / engine+10 tail | Only acquired task-constructor actions, then captured358h free, current engine-vector free, captured14h free. Completed profile/Root are not destroyed. Descriptor faults are original native scope evidence, not C++ fault transport proof. |
| Ensure publication after engine construction returns | Captured14h free only; completed child allocations and publications are not rolled back. Constructor scopes have already returned. |

A later failure can leave finished allocations orphaned; a Root failure after profile publication can leave that publication pointing at the enclosing9Ch storage subsequently freed. If ensure publication has been reached before a covered failure, no action clears it. Do not substitute current-publication frees, child destructors, unregister operations, retries, equality repairs or rollback for the recorded actions. Captured raw frees are unconditional actions; vector frees alone test the captured current data for null.

## Required task-constructor prerequisite

Current `dyn_task_manager_construct_00c37740` is explicitly normal-only. Its174-byte/58-instruction original has handler CC806F, FuncInfo E03DF8, maxState2, map E03DE8:

| State | Previous/action | Required operation |
| --- | --- | --- |
| 0 | -1 / CC8059 | After queue+C/+10/+14 zeroing, arm0 at C37771 **before** InitializeCriticalSectionAndSpinCount. Reload original task argument, addC and invoke408E80: capture current queue data once, BF6989-release iff nonnull, leave all fields alone. |
| 1 | 0 / CC8064 | Armed at C37783 after InitializeCriticalSectionAndSpinCount returns, regardless of its BOOL result. Reload original task argument, add18 and invoke real C30930→DeleteCriticalSection; then state0's queue release. Retained through the normal return. |

There is no native constructor-failure semaphore/event/worker-handle closure, worker join or complete C40FF0 destructor in this map. Calling that destructor would add actions and access partially constructed fields. Count0's existing untouched+0/+8 behavior and normal lifetime preconditions also stay unchanged.

All needed genuine services are available: actual `context.memory.release` for the queue vector and the real Win32 DeleteCriticalSection already used by the task module. C30930's existing cc13 wrapper requires an unrelated pure call interface; no fake instance is needed because its actual eight-byte operation is precisely the same Win32 call. The source repair should compose that real operation directly with the actual task+18 backing. Helpers409B10,408E80 and existingA900 are each14 bytes/seven instructions and have identical normalized current-data/nonnull/BF6989 behavior; the current private A900 operation can serve the engine's base-vector action without changing old helper code or inventing a new global.

## Concrete additive API proposal

1. First claim C37740 and `include/bsp/dyn_task_manager.hpp`, `src/dyn_task_manager.cpp`, plus new `docs/CC16_DYN_TASK_CONSTRUCTION_CLEANUP.md` / `reports/cc16_dyn_task_construction_cleanup.json`. Add `dyn_task_manager_construct_qualified_00c37740(storage,count,memory)` with the same three actual inputs. Preserve the existing normal API. Use an outer queue-cleanup catch starting before CS initialization and an inner CS-cleanup catch starting after it returns. Return normally without cleanup; on C++ failure delete CS before current queue release/rethrow. Preserve existing entry count capture, OS/field order, ignored BOOL behavior and current worker code. No dummy provider/tag, protocol, new context layout or fallback is required.
2. After that dependency is reviewed/admitted, claim C55EA0/C55F50 and `include/bsp/dyn_engine_runtime.hpp`, `src/dyn_engine_runtime.cpp`, plus `docs/CC16_DYN_ENGINE_QUALIFIED_CALLERS.md` / `reports/cc16_dyn_engine_qualified_callers.json`. Add four-argument `dyn_engine_construct_00c55ea0(engine,descriptor,context,delete_partial)` and three-argument `dyn_engine_ensure_00c55f50(descriptor,context,delete_partial)`. Reuse existing `DynProfileRootPartialNodeDelete`; it is type-compatible with the actual supplier. Keep both old APIs normal-only.

The new constructor should validate its required nonnull raw deleter before the initial three stores, then use exact nested catches: base-vector scope around both allocations; captured9Ch scope only around qualified3argRoot; captured358h scope after that allocation through descriptor/qualifiedtask/store/return. Call qualified Root and qualified task directly, with no legacy fallback. Preserve Root's existing `_memset`/native-feature/fault qualifications.

The new ensure should keep the first nonnull-publication return. Recommended null-provider validation is on the **cold path before allocation**, so that the untouched fast path accesses no descriptor/allocator/provider and does not pretend to requalify an existing engine. Allocate14h outside the captured catch; retain construction, current-cell publication (including null) and return inside it. The catch frees only that original raw allocation. This is an additive API validation boundary, not an original invalid-input ABI claim. No missing provider may select legacy construction.

## Actual supplier and root boundary

Use the **same existing** `GameNativeDynProcess`: `owner.dynamics().engine` supplies its actual memory/publication/name context; `owner.profile_partial_node_delete_00bf65ac()` checks the exact default context/allocate/release tuple after completed startup and returns real unsized scalar `::operator delete`. That same tuple allocates all raw48h/9Ch/358h/14h blocks through scalar `::operator new`. No second owner, rebind, global thunk, generic `memory.release` cast or default raw free is needed. Custom/unmatched tuples remain unqualified unless they supply their own genuine matching flat deleter.

Raw BF65AC custody remains distinct from vector BF6989 `memory.release`. The profile name's real SBO allocation instead uses existing singleton malloc/new-handler/free services. Native BF6989's original alias or a common CRT implementation does not license cross-domain replacement. These source-domain conclusions reuse the admitted supplier/Root proof; no new link, fixture or runtime proof is claimed.

The current actual chain remains `NativeGameConstructionCalls::call_00c55f50`→old2argensure→old3argengine→old2argRoot. Game constructor004DDB90 calls it at004DE129; the other indexed incoming edge is the separate undefined5-byte C5B190 tail wrapper. New Game call-service/context propagation, world/scene/table ownership and whole root/lifetime/failure adoption remain a later packet. An added engine API alone is not activation.

Static evidence comprises11 live/PE spans,14 export pins and18 exact source/evidence pins; Root source equals90e4 and the default supplier source equals1e69 at this snapshot. No source/CMake/shared-ledger/Ghidra edit, build, test, probe, fixture, game, DLL call or peer runtime-tree change occurred. Original FH3/SEH/hardware-fault/secondary-cleanup and private ABI transport remain unproven; no original unwind frames should be registered merely to link. Future source packets require strict Win32 object/emitted-scope/old-API checks, independent review and the primary's common build; actual caller activation still needs separately frozen relevant runtime admission. Native/vendor-body and activation credit are zero.

[Machine-readable readiness](../reports/cc16_dyn_engine_qualified_caller_readiness.json).
