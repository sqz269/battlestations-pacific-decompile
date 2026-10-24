# Mission render preparation jobs

`GameNativeMissionFrameJobs` supplies the missing application lifetime binding
for the existing reconstructed native render-command job branch. It is Source
composition over the established raw owners and Win32 scheduler, not a new
reconstruction of those bodies or an Original binary ABI replacement.

## Evidence and unchanged execution

The concrete `execute_native_render_command_00b1d950` implementation selects
jobs only when synchronization mode `0108D6DC` is zero and **both signed batch
counts exceed 50**. For each batch in index order it writes batch `+08`, obtains
the frame owner through `004C1130`, reloads the batch, captures the secondary
pool, obtains the preparation descriptor through `00B0FFB0`, and enqueues via
`00BE3020`. It then dispatches that same pool via `00BE3150` with workers enabled
(`1`). Serial preparation remains the original other branch. This component
does not choose a mode, change a threshold, override processor count, call a
serial fallback, or submit replacement jobs.

The established native providers used here are:

| Original entry / profile | Role / ABI evidence |
| --- | --- |
| `004C1130` / `004BFA40` | cdecl singleton getter; ECX primary owner constructor, `138A8h` allocation, secondary pool at `+04` |
| `00BE3040` / `00BE2C70` / `00BE4800` | ECX pool constructor with processor argument `-1`; original environment processor getter and suspended Win32 workers |
| `00D5E160:00B1BF70` | preparation job current virtual `+00`; ECX owner ignored, raw batch DWORD argument; forwards to current batch `+0C` `00B51DF0` |
| `00D68650` / `00CE7554` | secondary pool profiles: `+04=BE3020`, `+08=BE3150`, `+0C=BE2EA0`, `+10=4BFAB0`, `+14=4BFAA0`; scalar slots differ |
| `00D6821C` | event profile: scalar `BD19B0`, signal `BD1910`, wait `BD17C0`, reset `BD1960` |
| `00BE2BA0` | Win32 stdcall entry, actual secondary pool argument, invokes the current worker body; returns zero, native `RET4` |
| `00CE7550:004BFB30` | primary scalar deletion, invokes stop/join and clears `0109CF08` |
| `00D5E15C:00B0F1C0` | registered preparation secondary scalar deletion, adjusts to primary and clears `00F8D444` |

These contracts come from the existing source in
`native_render_command_execution.cpp`, `native_render_batch_preparation_actual.cpp`,
`native_frame_job_execution.cpp`, `native_frame_job_lifetime.cpp`,
`native_render_job_publication.cpp`, and `native_singleton_destruction.cpp`.
The new component reads the same current profile data through
`GameNativeReadOnlyData`; it does not execute numeric Original addresses.

## Identity and integration contract

Create one component inside the opt-in mission renderer application, borrowing
its existing `FrameGraph::queue_getter`, the same raw manager/deletion map, and
the **already initialized startup `RandomThreads` instance**. There was no
instantiation of `GameNativeFrameJobApplication` or an existing application
render-job publication context in the inspected startup source. Consequently
this component owns the authoritative Source cells representing `0109CF08`,
`00F8D444`, and atomic scope `0109DBE4`; they are not copies of another provider.
Its constructor refuses an existing `deletion.render_jobs` binding or another
live component worker-entry binding. Do not create an additional frame-job
application or a separate random registry alongside it.

Construction installs metadata only. Native owners/events/arrays/threads remain
lazy, constructed by the original getters already reached by `B1D950`.
`NativeRenderPreparationJobDispatch` is the **direct** dispatcher of the one
`NativeFrameJobExecution`. Its context is the exact borrowed queue getter.
This satisfies the command executor's pointer-identity checks. The unused
`GameNativeFrameJobApplication` has observing/fixed-step dispatch decorators and
cannot be substituted unchanged: passing preparation as its remaining dispatcher
would fail the direct-root identity requirement. Unknown job profiles fail;
adding other native job families requires an explicitly established binding.

The public API is:

```cpp
GameNativeMissionFrameJobs(GameNativeReadOnlyData&,
    NativeRenderBatchPreparationContext& same_queue_getter,
    NativeSingletonDeletionBindings& same_deletion_map,
    RandomThreads& existing_startup_random);
NativeRenderJobPublicationContext& publication();
NativeRenderPreparationJobDispatch& preparation_dispatch();
void observe_interrupted() noexcept;
bool requires_process_retention() const noexcept;
void after_native_drain();
```

Set command execution `jobs=&frame_jobs.publication()` and
`preparation_dispatch=&frame_jobs.preparation_dispatch()`. Leave `preparation`
pointing at the same queue getter. The command adapter can remove its old
serial-only admission guard after these providers are attached; its persistent
system/material frames and native batch lifetimes remain unchanged.

Calls that can enter native job construction/execution must catch escaping
exceptions, call `observe_interrupted()`, and rethrow. This includes exceptions
before a constructor publishes `CF08`: the original constructor does not invent
rollback for partial arrays/events/suspended threads. `B1D950` calls the raw
factories directly, so a direct dispatcher cannot observe every such failure by
itself. The existing application failed phase must retain the complete renderer
graph as well.

## Lifetime and limits

Serialize producers, frame execution, loading, and shutdown as required by the
existing scheduler. Retain this component, queue context, mapped profiles, raw
manager/deletion map, startup random registry, batches, and all their providers
through native dispatch and worker shutdown. There is no private job queue,
native owner copy, pose state, replay, or automatic native cleanup here.

`requires_process_retention()` rejects interrupted/retargeted bindings, nonzero
shared scope, queued work, or active dispatch. Idle live workers are eligible for
normal native shutdown. The raw singleton manager owns descriptor retirement
and `004BFB30` pool stop/join. Call `after_native_drain()` only after that same
manager's shutdown returns: it requires `AA0`, `CF08`, and `F8D444` null, scope
zero, and all entered worker calls returned. Only then does it remove the Source
Win32 entry and deletion-map bindings. Destruction before that condition
terminates rather than freeing live dependencies. An interrupted graph must be
retained until OS exit.

All original allocation sizes, processor selection, events, thread priorities,
LIFO claims, random register/unregister, native dispatch count stores, and stop
ordering remain in their existing reconstructed providers. The entry wrapper
adds only Source failure/entered-worker observations around `BE2BA0`; it passes
the exact actual pool pointer and uses the same execution instance on caller and
workers. Source C++ exceptions and binding metadata are not native FH3/SEH,
private-stack, fault, register, or vtable ABI parity claims. No new original
arithmetic body is introduced. This packet is source-reviewed only; build,
concurrency, actual scheduler execution, and gameplay validation belong to the
primary integration and are not established by its diff check.
