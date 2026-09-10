# Command and batch execution

`execute_render_command_00b1d950_fragment` reconstructs the command's control
flow around concrete batch storage. `00B20240` first checks inhibit/lost state.
The command then captures context camera `+08`, binds target `+14`, invokes
camera preparation `00B71360`, and invokes system-prefix gather `00B46A70` with
command scene `+04` and the captured camera.

The optional diagnostic singleton receives the command's length/string at
`+14/+18`. Concrete renderer virtual `+114` is `00B20210`, whose entire body is
RET4. Renderer virtual `+2C` is `00B1FE20`, a nonzero test of renderer `+1998`.
Preparation runs only when that frame gate passes.

Both batches prepare serially unless renderer synchronization is disabled and
both counts exceed50. That branch writes each batch's preparation mode `+08`,
submits both actual jobs, then waits with argument1. The job singleton's primary
vtable `00D5E160` resolves to missing leaf `00B1BF70`: load the batch argument,
replace it with batch `+08`, and tail-dispatch virtual `+0C`. The typed job
adapter preserves this mode read. Its required job service must complete work
and establish visibility before returning; no missing-service serial fallback
is substituted. Job-service allocation/singleton lifetime remains unported.

The retained `InstanceRenderQueue` overload of `prepare_render_batch_00b51df0`
sorts its actual entry pointer array. It uses the original batch index for
configuration and comparator selection. See `RENDER_BATCH_PREPARATION.md` for
the recovered unsigned64 key and complete native permutation behavior.

Batch execution `00B55550` checks frame activity again, clears the cached effect
owner identity at global `0108FBF4`, then dispatches material/effect virtual `+14`. The count and
list are reloaded after callbacks. Its two stack arguments are unused; batch
category does not select a material pass. The typed interface requires an
actual material dispatcher. `MaterialEntryDispatcher` now reconstructs
`00B45360`/`00B44750`/`00B43410` around the required constant builder and other
actual owner operations; see `MATERIAL_ENTRY_DISPATCH.md`. The complete builder
`00B42350` remains open.

On scope exit `00B13510` reloads the optional singleton and assigns the literal
**"X"**, whose original bytes are at `00CE9A38`. It does not clear the string to
empty. The new diagnostic storage projects string values while preserving this
reload boundary; it does not claim the native temporary-string/pool/SEH ABI.

Native command ABI: ECX command, no stack arguments, RET. Batch ABI: ECX batch,
two ignored stack words, RET8. Readiness and active-frame predicates return
their native boolean result without mutation. The new error interface stops
after malformed inputs or an explicitly failed required adapter, retaining
earlier effects. Surface-binding failures do not suppress later native stages;
they are reported after execution. Exceptions unwind diagnostic state.

This is an orchestration fragment. Required camera, system-prefix and material
operations have no default no-op implementation. Compiling it does not prove a
complete frame, native job scheduling, binary ABI compatibility or gameplay.
Queue/context/group lifetime is reconstructed separately in
`RENDER_COMMAND_QUEUE.md`; installed mesh GPU validation remains scoped to its
controlled scene until the complete actual dispatch dependencies are supplied.
