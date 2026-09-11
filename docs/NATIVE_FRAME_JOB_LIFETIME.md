# Native frame-job pool lifetime

Packet `orch3_native_frame_job_lifetime_x` completes the actual `138A8h`
frame-job owner and its `138A4h` secondary pool, using the worker execution in
`NATIVE_FRAME_JOB_EXECUTION.md`. These are reconstructed C++ interfaces, not
drop-in native vtables or exception handlers. The game has not been run with them.

Evidence: existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; installed executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`reports/native_frame_job_lifetime.json` records full live/disk byte spans,
original ABI, saved annotations, repairs and validation boundaries. Names describe
observed behavior; they are hypotheses rather than recovered symbols. The correct
existing `CG_scalar_deleting_dtor_004bfb30` name is retained.

| Native function | Complete behavior |
|---|---|
| `004B7A00` | Clear `0109CF08` regardless of its previous pointer; primary identity `CE3818`. |
| `00BE2C30` | Secondary identity `D68638` only. |
| `00BE4800` | `_dupenv_s("NUMBER_OF_PROCESSORS")`; error returns 1, success captures `atol`, frees text and returns the captured value. |
| `00BE2C70` | Initialize counts/jobs; allocate arrays and create actual suspended workers. |
| `00BE3040` | Secondary constructor; `-1` selects the environment helper. |
| `00BE2DA0` | Wait, request stop, dispatch, join, close events/threads, free arrays. |
| `00BE30C0` | Secondary destructor with base-identity unwind. |
| `00BE31E0` | Secondary scalar destructor; bit 0 controls final allocation free. |
| `004BFA40` | Construct secondary at primary `+04` with `-1`; publish final identities. |
| `004BFAD0` | Destroy secondary, clear singleton publication, restore primary base identity. |
| `004BFB30` | Primary scalar destructor; bit 0 controls final allocation free. |
| `004BFAC0` | Subtract 4 from incoming secondary pointer, tail-jump to primary scalar. |
| `004C1130` | Shared-domain lazy allocation, construction, publication and registration. |

All member functions take their native owner in ECX. Constructor `BE3040` and
creator `BE2C70` consume one stack DWORD and use `RET4`. Scalar entries consume
one flags DWORD, use `RET4` and return the original allocation address in EAX.
The adjustor therefore returns the primary allocation, not its incoming secondary
pointer. Other member functions use plain RET; constructors return their owner.
The processor helper and singleton getter are cdecl/plain RET with EAX results.

## Construction and ownership

Primary `+00` starts as `CE7520`. The secondary is constructed at `+04`, starts
as `D68650`, and contains the already established actual handles, thread IDs,
start/done events, signed counts and 10,000 raw two-word job slots. Successful
primary construction writes secondary `CE7554`, then primary `CE7550`. It does
not publish the singleton itself.

`BE3040` zeroes secondary pointers `+04`, `+08`, `+10`, and all job slots. It
does **not** initialize pointer `+0C`. `BE2C70` writes active/stop/remaining zero,
sets worker count to the DWORD-wrapped `processors - 1`, and zeroes the slots
again. For a positive signed worker count it allocates handles, IDs, start and
done arrays in that order. Each allocation reloads current worker count and
uses unsigned multiplication by four with overflow requesting `FFFFFFFFh`.
There is no processor clamp, worker-limit clamp or OS processor-query substitute.
With a nonpositive count, `+0C` retains its incoming preimage. Calling destruction
on arbitrary uninitialized preimages is not made safe by this reconstruction.

Each worker receives two concrete manual-reset events from `BD1970(CL=1)`.
The done event is signaled before `CreateThread(nullptr, 0, entry, actual_pool,
CREATE_SUSPENDED, &actual_ids[index])`. The result goes into the current handle
array, then `SetThreadPriority(current_handle, 2)` is called. Win32 failures are
retained/ignored as native. Arrays/count are reloaded in the observed order.
No partial-allocation rollback or fabricated valid handle is added.

`NativeFrameJobLifetimeBindings` borrows the established execution object, the
actual shared scope DWORD and a **required application Win32 entry binding** for
`BE2BA0`. The entry receives the actual secondary pool address and must route it
to `native_frame_job_thread_entry_00be2ba0` with its existing execution binding.
No implicit global dispatch registry, substitute context argument, extra pool or
random-thread owner is created. This application route and concrete job virtual
execution remain explicit integration requirements.

## Shutdown and exceptional paths

`BE2DA0` first waits on each current done event. It captures current pool virtual
`+08`, sets stop `+1C = 1`, and dispatches with low-byte `use_workers = 1`.
This wakes/resumes even workers which have never run; the caller still drains
queued jobs while stopping workers exit. It then calls
`WaitForMultipleObjects(current_count, current_handles, TRUE, INFINITE)`, even
for a nonpositive count after DWORD conversion, and ignores the API result.

Per worker, it closes the current thread handle, scalar-deletes each nonnull
start and done event with flags 1, and clears the **captured array cell after**
the call. It frees and clears current arrays in the order start, done, handles,
IDs. It leaves worker count and flags in their resulting state. The helper has
no local EH, retry or successful-error substitution.

Seven false no-return gaps were repaired and saved: `BE2E55`, `BE2E68`,
`BE2E7B`, `BE2E8E` (six bytes each, stack fixup and pointer clear),
`BE483C` (ten bytes, captured processor result/return), `4BFB45` and `BE3223`
(three-byte scalar stack fixups). All four affected stored bodies contain their
verified terminal returns after repair. No function was deleted/recreated.

| EH handler | FuncInfo / map | State cleanup |
|---|---|---|
| `CC6B48` | `E01328` / `E01320` | 0 to -1: `CC6B40 -> BE2C30` |
| `CC6B68` | `E01354` / `E0134C` | 0 to -1: `CC6B60 -> BE2C30` |
| `CC6B88` | `E01380` / `E01378` | 0 to -1: `CC6B80 -> BE2C30` |
| `C64DA8` | `D8D330` / `D8D328` | 0 to -1: `C64DA0 -> 4B7A00` |
| `C64DC8` | `D8D35C` / `D8D354` | 0 to -1: `C64DC0 -> 4B7A00` |
| `C64EF3` | `D8D4F0` / `D8D4E0` | 1 to 0: saved raw allocation free; 0 to -1: captured lock release |

Secondary construction/destruction failure restores only its base identity.
Primary construction/destruction failure clears the publication and restores
its base identity after secondary unwind. No member-array cleanup is invented.
A shutdown job failure retains the native active/scope/stop values and failed
slot; scalar owners are not freed after failed destruction.

The getter captures the existing shared singleton domain's critical section,
enters/increments its actual depth, rechecks `0109CF08`, allocates `138A8h`,
constructs and publishes, gets the same manager again, registers the **current**
publication, unlocks the captured section and reloads the result. Constructor
failure frees raw owner after member unwind and before unlock. Registration
failure has no publication rollback. The domain's required scalar callback must
dispatch the exact registered owner through its current primary scalar binding.

## Validation and remaining integration

Strict MSVC Win32 build and both existing CTests passed. One ignored focused
probe (`local/frame_job_probe_x.cpp`) reused the preceding worker fixture and
executed all 13 full original bodies, plus the original worker/execution chain,
against the reconstructed operations. Original function/data bytes were matched
to both the installed image and the verified Ghidra program.

The probe verified the environment helper, nonpositive counts/preimages, real
suspended worker creation and priority, manual-reset event state, caller-only
and parallel jobs, queued work during shutdown, joins/exit zero and array free
order. It covered primary/secondary destructors, flags 2/1, the adjustor's primary
return, and actual shared-domain getter fast path/registration/shutdown. Original
normal paths use real Win32 APIs and existing random/event/domain implementations;
only addresses, current vtable entry bridges and required host bindings are
relocated. The child probe process alone uses `NUMBER_OF_PROCESSORS=2`.

Module-local allocation interception checked C++ partial allocation, overflow
request, member/base cleanup, raw free at lock depth 1 before unlock, and shutdown
job failure. No permanent tests or production test hooks were added. These are
one-worker composition checks, not general concurrency or performance proof.
Original EH dispatch, unavailable-API failure branches, binary ABI replacement,
concrete game-job execution and gameplay are unvalidated. Point-effect virtual
zero can now compose with the recovered frame-pool lifetime once its actual
entry/scalar/job application bindings are supplied.
