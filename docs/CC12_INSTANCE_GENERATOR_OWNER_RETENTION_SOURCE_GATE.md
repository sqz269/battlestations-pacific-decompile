# CC12 mutable instance-generator owner admission: Source gate

**Mutable `NativeInstanceGeneratorOwners&` exposure is not ready.** A latch added only to `requires_process_retention()` would cover some exits but would leave current frame, worker, reset, presentation and direct-drain entries open. The smallest useful next prerequisite is a concrete retained-acquisition admission contract plus a control-worker admission/acknowledgement contract. Both can use existing real owners and services; neither interface currently exists. Keep mutable exposure absent while those contracts are established.

This is a read-only continuation from Source candidate `4427aecec9a59a9519791a5f6490bc34f76531ec`. Its metadata-only borrow was pending Root acceptance at this packet's start; this audit does not imply acceptance or authorize activation. The earlier Source749 baseline and all context-borrow artifact pins are preserved. Root's later Source819/raw-ID receipt is separate context, not a resynchronization or a claim that every current Root source file matches this worker.

## The concrete admission boundary

The future application must retain its entire actual graphics/layout/string/VFS/mapped-data graph and the owner of every operation record. It must close lifecycle admission and account for already admitted work, validate the ready state and exact domain, then set a **monotonic obligation before a mutable reference escapes**. That obligation must not clear on successful return, `Phase::complete`, an empty canonical registry or an empty companion list. Its presence must be observed before every transition listed below.

The current generator owner learns about an operation only at completed-creator registration. Name allocation, declaration acquisition, stream-descriptor work and layout creation can already have happened. `register_owner` adds an Entry before binding it, so a failed bind can also leave an unbound companion. Neither the registry nor the owner's entries is a record of all pre-entry acquisitions. A finalizer's null early-out does not prove that some other admitted operation has no obligation.

There is a genuine existing record owner to reuse: `NativeMeshResourceCalls::parse_item` stores a `unique_ptr<NativeMeshResourceAcquired>` before entering any of its three parsers. That record owns its mesh-field/subset acquisition tree, and each subset embeds its generator record. The application must retain this real service before its parser can enter, and admission must validate that an operation belongs to the retained tree. A saved pointer to a caller's stack record is insufficient. Direct generator constructors, attachment and finalizer APIs still accept arbitrary records, while `NativeInstanceGeneratorOwners::context()` exposes mutable service references. Those paths require an explicit admission/membership contract before their first phase, name or raw-object write; admitting only at registration is too late. Standalone records require genuine retained owner storage, not a permissive callback around the current signatures.

Const on the already implemented metadata borrow does not provide that boundary: the context is copyable and its references are shallow. Its no-activation contract remains unchanged.

## Finite lifecycle cuts

All six current retry families converge on the actual `NativeRendererDeviceRecreationContext` and `recreate_native_renderer_device_00b29670`:

| Existing caller | Concrete path to the common recreation entry |
| --- | --- |
| Logical vertex and logical index constructors | Existing `ActualRecreation::call_00b29670` |
| 2D, cube and volume file loading | Existing `TextureLoadingGraph::retry` |
| Surface factory | Its borrowed `c.recreation` |
| Runtime 2D texture and runtime volume texture factories | Their borrowed `c.recreation` |

A shared, nonpermissive admission reference in that genuine context can cover these destructive retries without replacing callbacks or changing the leaf retry algorithms. The same-context bindings must be rechecked. A refusal must not fabricate a successful retry or discard the already retained acquisition frame. In particular, the 2D file-loader retry deliberately reuses its captured device.

Recreation is not the whole cut. These entries have earlier independent effects:

- `process_native_renderer_device_reset_00b2abd0` can release resources, perform in-place `Reset`, and restore them without reaching `B29670` at all. Its entry gate must precede optional guards and pending/lost processing.
- `change_native_renderer_presentation_mode_00b29e60` writes presentation parameters before it either recreates or publishes a pending reset. Check before locks and writes, including the no-recreation branch. No current Source caller was found, but its public entry remains part of an all-entry contract.
- `initialize_native_renderer_device_00b2aeb0` immediately clears and writes presentation storage. The application wrapper's once-only phase prevents ordinary post-ready reuse; a direct raw-context entry still needs the shared guard.
- `begin_native_renderer_frame_00b2b200` writes the frame-active/inhibit state before reset. `GameNativeRendererApplication::begin_frame` also writes its phase before calling it. Both must refuse before those writes.
- `end_native_renderer_frame_00b2d8e0` performs queue execution, binding changes, buffer rewinds, rendering and Present. The control worker reaches it independently through `FrameGraph::worker_finish`. Normal and worker frame contexts must carry the same admission identity. Preserve the existing inactive-renderer branch that returns before dereferencing a possibly null context; the default/save wrappers then inherit the central active-entry guard.
- `stop_native_renderer_worker_mode_00b28a90` and `clear_native_renderer_stop_pipeline_00b26920` wait or mutate real synchronization/binding/cache/device state. The current queue constructor can reach this path. A stop-pipeline gate belongs before the wait or first mutation.
- `initialize_window_render_entry_cache` is a separate ready-state application mutator. Either finish it before admission as an explicit prerequisite or gate its entry; the current metadata-ready check alone does not imply it already ran.

`GameDeviceHost::clear_and_present` reaches application begin before its overlay and capture callbacks. The new application cut would therefore dominate those existing callbacks; an extra replacement drawing wrapper is unnecessary.

## Worker admission is a specific missing interface

The actual control-worker loop samples the lifecycle busy depth and then calls its begin/end providers. That sampled depth is not a held admission lock. The application `ready` phase does not acknowledge worker callbacks or frames already admitted, and an atomic/volatile latch alone would not close a check-then-enter race.

The real `request_native_renderer_worker_stop_00b33bf0` is not an unconditional solution. It clears `run` and waits on the auto-reset idle event; it does not signal wake. The constructor creates both events unsignaled, starts with `run=0`, and the thread waits on wake before it can ever signal idle. Calling the stop helper for a never-started worker can therefore wait indefinitely, as its existing header explicitly states. The destructor's shutdown/wake/join path is a destructive lifetime operation, not an admission handshake.

The missing interface must distinguish never-started, running, already idle, callback/frame in flight, failed and teardown states. It must prevent new entries after admission closes and acknowledge earlier work without consuming a stale idle signal as proof. The complete current Source census is retained, but the absence of a named start setter is not proof that a runtime worker is quiescent; computed raw writes and unmodeled callers remain outside that lexical result. No Native or live-worker evidence was collected in this packet.

## Direct drain and process exit ordering

`GameSingletonHost::shutdown` bypasses the application's retention predicate. Its manager-null branch still retires VFS, so a shared obligation check belongs before even that branch. Its fallback destructor calls the same method. The existing `deletion_bindings.renderer_owner` already points to the genuine renderer-destructor context, which can carry the same concrete obligation; no second registry or arbitrary callback wrapper is needed.

The raw manager entry also needs a check **before** `destroy_native_singleton_manager_00bd0400` enters its `try`. That catch calls `clear_native_singleton_storage_00bd0220`; throwing a refusal from inside the try would itself destroy retained bookkeeping. `destroy_native_renderer_00b32920` likewise must refuse before profile stores, lifecycle-lock release, control-worker deletion or its native unwind region. Both scalar-deleting renderer wrappers converge on this entry before freeing storage.

The four existing `GameStartupHost` checks already precede application shutdown, manager drain, host/provider destruction and `std::exit`/CRT callbacks. They can observe a correctly bound new monotonic predicate without separate replacement exit callbacks. Normal and exceptional `run_win_main` completion reach `host.reset()` before the final normal return. The VFS qualification returns use separate graphs.

`std::_Exit` is a terminal containment path. It does not demonstrate safe continued operation or normal return, and it cannot retroactively retain an operation record already destroyed by caller stack unwinding. Record ownership must be established before native entry. Normal cleanup after activation remains a separate release/retirement proof, even if all lists later look empty.

## Exact future file and function ownership

The following is a finite, conservative change set for a closed Source admission contract, not authorization to implement it. The [machine report](../reports/cc12_instance_generator_owner_retention_source_gate.json) enumerates all functions and 25 unique paths, with 57 current full-file anchors.

| Coordinated owner | Files that need a concrete hook/interface change | Function boundary |
| --- | --- | --- |
| Primary application integrator | `game_native_renderer_application.hpp/.cpp` | Future owner admission; real state lifetime; retention; frame/cache/drain entries and Impl destruction |
| Generator/mesh lifetime worker | `native_instance_generator_owner.hpp/.cpp`, `native_mesh_subset_loading.hpp/.cpp` | Retained record admission before direct constructors/attach/finalize; real parser record tree; owner context/registration contract |
| Device/lifetime worker | `native_renderer_device_recreation_actual.hpp/.cpp`, `game_native_renderer_device.inc`, `game_native_renderer_lifetime.inc`, `native_renderer_reset_process.cpp`, `native_renderer_presentation_mode.cpp`, `native_renderer_device_startup_actual.cpp` | Same concrete obligation through real contexts; pre-side-effect transition cuts |
| Frame worker | `native_renderer_begin_frame.cpp`, `native_renderer_end_frame.hpp/.cpp`, `native_renderer_stop_pipeline.hpp/.cpp`, `game_native_renderer_frame.inc` | Host and raw frame/worker-end/stop-pipeline admission |
| Control-worker specialist | `native_renderer_control_worker.hpp/.cpp` | Concrete close/acknowledgement protocol and all worker callback/frame admissions |
| Primary singleton integrator | `native_renderer_destructor.hpp/.cpp`, `game_hosts_singletons.cpp`, `native_singleton_destruction.cpp` | Shared real obligation and pre-try/pre-retirement direct-drain cuts |

Headers above are under `include/bsp/`; implementations and composition includes are under `src/`. Shared application/context interfaces require integrator coordination before independent work. The leaf retry files, existing host exit checks, normal main exit order and provider destructors are verification dependencies; they do not each require independent guard wrappers once the listed common cuts and genuine lifetime retention are established. Changing raw API scope could alter this set and requires another caller audit.

No bounded mutable-exposure implementation is ready until the retained-record and worker protocols are concrete and reviewable. The next packet should close those interface/evidence gates while keeping owner exposure absent, then implement the listed common cuts as one consistent contract. Do not promote the metadata context into activation or rely on a terminal exit as successful admission.

## Frozen evidence and replay

This packet freezes 4,209 complete current Source/configuration files and matching Git blobs. Its 56 roots produce a 733-file quoted-include closure, 737 files with configuration, and 1,528 resolved include edges. All 4,205 tracked Source files form the complete five-query search corpus. All 753 Source749 pins are copied intact. All 3,862 context-borrow bundle payloads and its ZIP were checked unchanged; their manifest/receipts are retained separately rather than claiming that entire earlier bundle is duplicated here.

Run the standalone own-bundle replay, adding `--verify-prior` when the original context-borrow artifact directory is available:

```powershell
python local/cc12_instance_generator_owner_retention_source_gate/review_evidence.py --verify-prior
```

It verifies whole files, Git blob identities, include closure, complete searches, all 57 anchors, the finite hook matrix and Source749 copies. It performs no compilation, tests, child-process calls, Native execution or Ghidra work. Proposed guard names and interfaces are Source contracts, not recovered symbols or original ABI guarantees. Only this document and its machine report are changed.
