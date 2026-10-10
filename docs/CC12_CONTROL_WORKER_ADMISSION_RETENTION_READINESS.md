# Control-worker admission and retained acquisition readiness

**Mutable instance-generator Owners exposure remains unavailable.** The smallest next prerequisite is a genuine retained-record admission interface coupled to a cooperative worker close/acknowledgement interface. Existing Source supplies useful owners, records, context bindings and event primitives, but no bounded interface currently composes them into that guarantee. This is a read-only specification, not permission to activate construction or loading.

Baseline: worker commit `4799d10b2edebbb25dd9ee4badfd2caca4aec1eb`. At this packet's start Root was finishing candidate `4427aecec` acceptance/build, and gate `4799d10b2` remained pending primary review. Neither acceptance is inferred here. The Source corpus is unchanged from that gate. Its complete transition/drain prerequisites remain in [the prior gate](CC12_INSTANCE_GENERATOR_OWNER_RETENTION_SOURCE_GATE.md).

## Concrete reusable Source

`NativeMeshResourceCalls::parse_item` in `src/native_mesh_subset_loading.cpp:403` already allocates and inserts a `unique_ptr<NativeMeshResourceAcquired>` before calling any of the three mesh parsers. Its heap pointee remains stable if the vector grows. The root contains `loading.fields`; that owner appends a heap subset record before `read_native_mesh_subset_00b941d0`, and the subset embeds its generator record and owns texture-child records. This is actual retained bookkeeping, including failed operations. A future composition should retain this service and its complete dependency graph, preserving this insertion-before-entry order.

This service is **not instantiated by the current application**. The full 4,205-file Source census finds `NativeMeshResourceCalls` and the mesh loading/subset context types only in their declaration/implementation pair. `GameNativeMeshFieldServices` supplies buffer, metadata, texture and lighting prerequisites. `EffectOwnersGraph` supplies effect construction/lifetime/owner services. Neither supplies `NativeMaterialEffectCacheContext` with its genuine effect-loading, date, platform, synchronization, profile and allocation dependencies. Full mesh activation is therefore a separate composition prerequisite; a permissive callback or dummy cache would not close it.

The application already owns the stable graphics/layout/string/VFS/mapped-data domain needed by `NativeInstanceGeneratorContext`. Its current shallow-const metadata borrow explicitly forbids activation. `NativeInstanceGeneratorOwners` knows completed creator registrations, not all earlier acquired records. Direct declarations/generic/building constructors take `NativeInstanceGeneratorContext&` and a caller-owned acquired record; attachment and finalization also accept records directly. Names, raw storage, declaration lookup and layout work can precede registration. Merely adding a vector of borrowed record pointers would not retain a caller's stack storage.

The concrete worker domain is also available: `GameNativeRendererApplication::Impl::control`, bound once through `bind_native_renderer_control_worker_process_context`, borrows the real clock, current renderer publication and actual frame providers. `bind_platform_services` supplies `FrameGraph::begin`, `FrameGraph::worker_finish` and the real `worker_end` context. These are the correct composition points. `DuplicateHandle` and `after_native_drain` provide post-destruction thread-termination observation; they do not establish admission closure before acquisition.

## Why the current stop is insufficient

`construct_native_renderer_control_worker_00b33da0` sets `run=0`, creates wake and idle as initially unsignalled auto-reset events, then creates/resumes the thread. The thread's first action is `wait(wake)`. It cannot signal idle until it has passed that wait and left the inner run loop. Here, never started means no work cycle has been enabled; the OS thread already exists.

`request_native_renderer_worker_stop_00b33bf0` clears raw byte `+4` and waits on idle. It does not signal wake. An otherwise healthy never-started worker can therefore wait indefinitely; the existing stop header explicitly states this limitation. `stop_native_renderer_worker_mode_00b28a90` ignores the wait result and then changes synchronization mode, clears pipeline bindings and sleeps. It is a destructive renderer transition, not a worker-only admission primitive.

The worker checks `run` at the loop head, then samples the clock, updates time, calls the native callback, waits for lifecycle busy depth, and calls begin/end. Clearing `run` after that check does not retract the admitted iteration. There is no recheck between callback completion and frame dispatch. The final busy-depth check is a sample, not a held entry lock.

| Worker state | Existing behavior | Required evidence before Owners escape |
| --- | --- | --- |
| Never started | Blocks on first wake; idle has never been signalled. | Admission initialized before thread publication, exact worker identity, and a cooperative initial parked acknowledgement or a reviewed exclusive-start proof that also prevents future entry. |
| Running between iterations | Run clear can race after the loop-head check. | The same synchronization must decide entry and closure; already admitted iterations remain accounted for. |
| Callback in flight | Callback can still be followed by busy wait and begin/end. | Keep callback/provider dependencies alive through completion; a callback requesting closure must not wait for itself. |
| Busy wait in flight | Wait does not test run/shutdown; lifecycle depth may remain positive. | Close before taking a lifecycle/provider lock needed by admitted work, or establish a reviewed cooperative escape. Waiting while holding that lock can deadlock. |
| Frame in flight | Raw begin/end bypass application phase methods. | Account for the complete iteration and actual provider calls, using the same admission identity as normal frames. |
| Previously idle | Idle may be stale, unconsumed, or consumed by another waiter. | Acknowledgement must belong to the current closed request and mean zero in-flight work plus no future entry. Resetting an event alone does not establish this. |
| Failure | Event/thread calls and wrappers do not uniformly check results; provider/callback exceptions have no close/ack catch in the thread body. | Null/invalid handles, wait failure, missed signal or dead thread must not become parked success. Keep admission closed/unknown and retain the graph. |
| Shutdown | Destructor sets shutdown, wakes, joins and deletes; inner run loop ignores shutdown until run clears. | Preserve existing worker/context/event/provider lifetimes through joins. Destruction cannot substitute for live retained-owner admission. |

A one-shot permanent close can avoid implementing restart semantics, but it still needs a unique, current acknowledgement and a proof that no raw start/wake path can bypass it. The complete lexical census finds no named `run_04=1` setter; it also captures the known inline-assembly raw clear in B33BF0. This is not evidence that computed writes or unmodeled callers are impossible.

The native event APIs may carry notification after their actual result and lifetime contracts are established. They carry no request identity themselves. Do not steal the native callback/context fields to implement a synthetic acknowledgement callback, change the actual 0x24-byte storage layout, or substitute a different process context. The process binding is immutable and has no unbind.

## Required retention and ordering contract

1. Keep mutable Owners unavailable. Retain the genuine application graph and stable admission state before worker creation/publication. Validate the exact worker, events, thread and eventual frame-provider identities; application `ready` is insufficient.
2. Close new worker callback/frame admissions using the mechanism that controls those entries. Wait for the current closure's acknowledgement without holding locks needed by admitted work. Handle never-started, stale-idle, in-flight, failed and reentrant cases explicitly. A wake with `run=0` is useful only after exclusive admission and a current acknowledgement protocol have been established.
3. While entry remains closed, revalidate application readiness and the original domain, then establish real owning roots for all admitted generator records. Membership must identify storage that the retained service actually owns, or a nested record in an already retained root. Direct context/constructor/attachment/finalizer paths must enforce the same rule before their first effects.
4. Set the application's **monotonic process-retention obligation before any mutable Owners reference or activation capability escapes**. Preserve it through allocation or bind failures, successful returns, empty registries, finalizer early-outs and `Phase::complete`. Canonical registration is too late, and insertion-before-bind can leave an unbound companion.
5. Admit each new acquisition root before provider work and each child before child work. Retain full failed roots and their strings, mapped data, VFS readers, geometry, layouts, caches and worker/provider graph. Owning only the serial cell or the generator companion list is insufficient.
6. Close every transition/drain entry identified by the prior 25-path gate. Worker acknowledgement alone does not guard raw frame entry, in-place Reset, recreation, presentation, startup, direct singleton shutdown or process exits. Do not reopen admission or authorize drain merely because one operation completed.

The ordinary native worker destructor and the terminal process-retention exit are not successful versions of this admission sequence. A callback can block indefinitely; this audit claims no bounded completion time or general deadlock freedom. Existing queue construction conditionally reaches B28A90 when its preserved `control_20` preimage is 2, so any future close interface must have an explicit self-call policy. No occurrence of that runtime path is claimed here.

## Exact future ownership

These are proposed responsibilities for a later coordinated packet, not C++ leases or an approved implementation. The report enumerates exact existing functions for every path.

| Owner | Existing paths | Responsibility |
| --- | --- | --- |
| Primary application integrator | `include/bsp/game_native_renderer_application.hpp`, `src/game_native_renderer_application.cpp` | Own the genuine admission/record lifetime, bind before thread publication, latch before escape, retain through failed work and joins. |
| Retained-acquisition specialist | `include/bsp/native_instance_generator_owner.hpp`, `src/native_instance_generator_owner.cpp` | Define owning-root membership and enforce pre-entry admission for direct constructors, attachment, finalizer and mutable context access. |
| Control-worker specialist | `include/bsp/native_renderer_control_worker.hpp`, `src/native_renderer_control_worker.cpp` | Establish closed-entry and current acknowledgement semantics on the actual process/worker identity, including initial/failure/reentrant states. |
| Control-worker specialist with integrator | `include/bsp/native_renderer_stop_pipeline.hpp`, `src/native_renderer_stop_pipeline.cpp` | Preserve the recovered stop contract and distinguish it from Source admission; refuse destructive stop before its run/wait/mode/cache effects when retention requires it. |
| Primary frame integrator | `src/game_native_renderer_frame.inc` | Bind the same admission identity to the actual normal and worker frame providers. Coordinate with the full transition gate. |
| Deferred mesh-loading owner specialist | `include/bsp/native_mesh_subset_loading.hpp`, `src/native_mesh_subset_loading.cpp` | Conditional full mesh extension: retain the real parser service/root tree, cover direct parser entries, and require complete effect-cache/loading composition first. |

The first nine paths are a finite core responsibility set, not a claim that a nine-file patch is sufficient for mutable exposure. The last two paths are conditional on full mesh activation. Existing event, synchronization, renderer constructor/destructor, queue construction and material-effect provider files are verification dependencies, enumerated in the report. The prior 25-path gate remains a mandatory coordinated dependency.

## Reproducible read-only evidence

The ignored artifact directory is `local/cc12_control_worker_admission_retention_readiness/`. It retains all 4,209 complete Source/config files and their Git blobs, the complete 4,205-file searchable Source census, 56 roots, 733 quoted-include closure files plus four configuration files, 1,528 resolved include edges, eight new complete query results, 34 line-addressed full-file anchors, eight worker state contracts and eleven proposed future paths. No quoted include is unresolved.

All 753 Root Source749 pins are copied unchanged. The prior gate's 9,202 immutable payloads and ZIP are checked in place before preparation and during the final optional prior verification; its complete report, document, query results, audit, manifests and receipts are retained. The earlier context-borrow bundle remains unchanged and separately verifiable. Root's newer Source819 notice/receipt stays separately qualified; no equality with its entire 823-artifact closure is asserted.

Run `python local/cc12_control_worker_admission_retention_readiness/review_evidence.py` for pure offline file/hash/Git-blob/include/query/audit replay; `--verify-prior` additionally verifies the preserved prior local bundle directories. The replay body uses no child process, compiler, test executable, Native or Ghidra call. Detached receipts record packaging, replay and the final exact two-file commit. This packet changes no C++ and performs no compilation, unit test, concurrency probe, startup, ABI or game validation.
