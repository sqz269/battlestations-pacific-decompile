# Dyn engine and profile construction

Addresses: `00C55F50`, `00C55EA0`, `00C50310`, `00C44000`.
The descriptive Dyn names are hypotheses, not recovered C++ symbols.

The engine now owns the actual `14h` storage, `9Ch` profile owner and its `48h`
Root node, and the separately reconstructed `358h` task manager. The engine
context borrows the existing allocator pair and actual publication cells
`0109E9FC`/`0109E9F8`. World and scene contexts can read that same engine cell.
The task manager is constructed by `dyn_task_manager_construct_00c37740`; no
replacement manager, worker count object, method table or second allocator is
introduced. This packet does not connect the owner to the game startup host.

| Routine | Native ABI and inclusive body end | Coverage |
| --- | --- | --- |
| `00C55F50` | ECX descriptor pointer, EAX engine, plain RET at `00C55FBE` | Complete normal construction/reuse sequence and explicit null-return branches; native EH cleanup excluded |
| `00C55EA0` | Stack engine pointer and descriptor pointer, EAX engine, RET8 at `00C55F34` or `00C55F4C`; end `00C55F4E` | Complete normal engine storage construction and explicit null-return branches; native EH cleanup excluded |
| `00C50310` | ESI profile pointer, EAX profile, plain RET at `00C50382` | Complete normal profile construction/publication and explicit null-root branch; native EH cleanup excluded |
| `00C44000` | EDX NUL-terminated name, stack node pointer and word2C, EAX node, RET8 at `00C4407F`; end `00C44081` | Complete normal node construction through existing legacy string assignment; native EH cleanup excluded |

## Order and publication

`00C55F50` first reads `0109E9FC`. A nonnull engine is returned without reading
the descriptor or allocating anything. Otherwise it allocates `14h`, calls the
engine constructor, and publishes its result only after that call returns.

The engine constructor zeros its vector words at `+00/+04/+08`, allocates `9Ch`
and constructs the profile, then stores that pointer at `+0C`. It allocates
`358h` for the task manager. Only when that allocation succeeds does it load the
descriptor's first DWORD, snapshot it, and call the actual manager constructor.
The returned manager is stored at engine `+10`. This snapshot matters: the
native passes a value on the stack, not a live descriptor reference.

`00C50310` allocates a `48h` node. The actual name at `00D79DC8` is `Root\0`;
the context supplies its current bytes. After construction, profile `+00/+04`
both point to that node and `+08` is zero. The routine publishes the profile at
`0109E9F8` **before** zeroing `+0C..+9B`. Its ESI input is established by
`00C55EE4..00C55EEC`: save ESI, set it to the allocation, call, restore ESI.

`00C44000` zeros node `+04/+08/+0C`, initializes the legacy string at `+10`, and
uses the existing `native_legacy_sbo_string_assign_counted_00408720`. It then
copies the second stack word into `+2C` and clears `+38/+3C/+40/+30/+34`, in
that order. Node `+00`, the string's preserved word at `+10`, node `+44`, and
unused inline-string bytes remain untouched. `NativeLegacySboStringStorage`
already defines the real `1Ch` representation; it is not the pooled NativeString.
The other node-constructor caller, `00C50390` at `00C503CB`, supplies a caller
name and word2C, so the public node API does not restrict them to Root/zero.
The meaning or ownership of word2C is not inferred from the root call.

## Inputs, lifetime and failure boundaries

The game call at `004DE129` receives ECX from `LEA [ESP+14h]`; the DWORD there
is the result of `00BE4800` at `004DE11C`. Those game/processor routines are
external dependencies, not extra engine fields or a new count policy.

The borrowed allocator and global cells must remain valid for their actual uses.
Constructed profile, node and task-manager storage must outlive dependent world
and scene operations. There is no concurrent singleton-startup guarantee.
Native engine/profile destruction, profile timing, world-list insertion and game
startup integration remain separate. Task-manager ownership/execution and its
native handle-lifetime limits belong to `docs/DYN_TASK_MANAGER.md`.

The native `operator_new` at `00BF681B` retries malloc through the new handler
and throws on exhaustion. Its callers nevertheless contain null checks; this
source retains those branches if an explicitly supplied allocator returns null.
Such a profile/node/manager null can produce an engine unsuitable for world
construction. It is preserved as a native branch, not treated as usable runtime
state. Native SEH cleanup and exception/global-publication rollback are not
reproduced. The C++ entry points are not binary ABI replacements.

## Evidence and verification

All four owned bodies have complete Ghidra function definitions and no call
fallthrough gaps. The report lists every ten direct call sites across them.
`00C50390` has an existing three-byte free continuation gap at
`00C50434..00C50436`; it is an unowned caller-evidence dependency and is neither
reconstructed nor repaired here. No Ghidra mutations were performed by this worker.

Seven complete code spans (the four constructors, both task-manager construction
entries and the existing string assignment) match the installed executable and
live Ghidra bytes. Ignored `local/prepare_engine_fixture.py` verifies the captured
bytes and preserves the previous scene fixture's relocated native image.

Win32 Release and both existing tests passed. All12 call rows passed the live
report verifier. The focused probe passed one normal construction and four
forced-null allocation paths, four singleton-reuse checks, and12 owned buffer
pairs; all612 fixture handles were closed and no tracked allocations remained.
Exact combined-source hashes are recorded in `reports/dyn_engine_runtime.json`.
The focused fixture executes the original
engine/profile/node/task-manager constructor bytes with zero requested workers.
It uses real Win32 critical sections, semaphore and 101 events; no worker thread
execution is claimed by this engine fixture. Forced allocator-null cases are
explicit branch probes, not original allocator-exhaustion or native EH tests.
Only known pointer fields are canonicalized. Actual OS handles are checked for
validity and initial nonsignaled state before normalization by role; the sole
excluded word is manager `+18`, `CRITICAL_SECTION.DebugInfo`.

Zero-worker construction leaves task-manager `+00` untouched. Fixture cleanup
therefore closes all 102 handles, deletes the critical section, and releases raw
buffers directly. It does not initialize that word or claim native destructor
validation. Complete engine/profile bytes, allocation sequence, publication at
subsequent allocation boundaries, singleton reuse and untouched bytes are compared.
Native task execution, nonzero worker behavior, asynchronous profiler observation,
and game/world integration are outside this fixture's evidence.
