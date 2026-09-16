# Raw render-command execution (R59)

`native_render_command_execution` reconstructs the full 369-byte `00B1D950`
command executor against actual command fields and the recovered providers. The
older typed fragment remains historical evidence; it is not called by this entry.

## Execution contract

After the readiness check, the function captures the command context and camera,
binds targets, executes the camera command, gathers system constants, and copies
the optional diagnostic label. It then arms the native reset obligation before
the concrete metadata no-op and checks the current active-frame state.

Synchronization disabled plus both signed batch counts above 50 selects the job
path. Each batch mode is stored before the frame-pool getter; the batch is reloaded
after that getter, and the returned pool is retained across the preparation-job
getter. Current enqueue and wait slots call genuine scheduler implementations.
The serial path calls the same actual batch preparation directly. Both paths then
execute the two current batches through the full R57 dispatcher.

The native exception guard resets the current service label only after metadata
arming. Target, camera, system, and initial label failures precede that guard.
Normal reset occurs after disarming, so its failure does not trigger a second reset.
No child-frame cleanup, rollback, owner substitution, or callback-only dispatcher
is added. System and pass frames must be prepared by the caller and persist through
failure. Serial and worker paths must share the same concrete preparation binding.

## Raw diagnostic strings

The full 57-byte `00B13030` and 165-byte `00B13510` receive new raw string-pool
overloads. They use the existing actual `01090AA8/01090AA4/01090AA0` domain.
This preserves exceptions from the getter during normal buffer return, which the
older `NativeStringStorage::release noexcept` interface could not propagate.

Reset constructs its native temporary from `X` before arming cleanup, captures
normal data/length, cleans the current temporary header on unwind, and disarms
before releasing the captured normal buffer. Copies preserve native overlap and
header read order. Existing interfaces remain available to their current callers.

## Evidence and validation

Fresh installed-PE/live-Ghidra comparisons cover complete bodies, current profile
slots, the reset literal, and both EH maps. `00B1DA6D` is unreachable three-byte
alignment with no live references; it remains unchanged.

- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three CTests passed.
- Four copied-full-native/source command readiness exits preserve the complete
  renderer fixture and skip inaccessible command/service/manager storage.
- Copied full native label bodies and raw source exercise alias/null exits,
  growing, overlapping, and empty copies, and reset to `X`, through the genuine
  raw pool and manager. Labels and the pool are drained through real providers.
- Linked machine inspection confirms parent provider order, selection of raw-pool
  overloads, serial preparation, and the genuine job get/enqueue/wait chain.
- Exact source/build/probe/native receipts and physical I386 runtime files are
  retained in hashed local archives referenced by the report.

These native fixtures adapt global addresses and call genuine source helper
providers. They do not execute the original helper binaries or active command
path. No application owner graph, jobs, draw calls, native exception/fault ABI,
visual, or gameplay equivalence is claimed. Raw queue execution remains next.

Evidence: `reports/native_render_command_execution_r59.json`.
