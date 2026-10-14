# Native fixed-step frame-job binding

Packet `cc12_fixed_pool_binding` adds the missing compiled raw group/descriptor
bridge in `src/native_fixed_step_job_waves.cpp`. `GameFixedStepHost` accepts a
borrowed `NativeFixedStepJobWaves` and uses it for the three fixed waves, the
interpolation wave and the pending-registration flush. The attached path queues
actual descriptor owners and actual element pointers through the reconstructed
native frame singleton and pool. It executes the descriptor bodies inside pool
dispatch, with no second simulated body walk.

The application does **not** currently supply this attachment. Without it the
host reports missing actual group/owner bindings. It no longer asserts that
typed Source unit construction proves the raw groups or pending list empty,
and row 11 remains an explicit record instead of claiming a native splice.

## Reconstructed behavior and evidence

The shared exports were inspected with `bsp.py show`; assembly was used for
the driver admission/dispatch sites, four job bodies and the pending splice.
Existing reviewed names are retained; descriptive C++ names remain hypotheses.

| Original address | New compiled behavior |
| --- | --- |
| `00874C90` | Unlink each actual pending node, append before its group tail, reload current last at the native sites, then reset the four pending header fields. |
| `008755A0`, `00875750`, `008754D0`, `00875670` | Four double-checked descriptor factories sharing the actual lifetime domain. Allocate 8/8/8/12 bytes, write original primary/secondary profiles, publish after construction and register the actual secondary pointer. Interpolation+08 remains uninitialized until the driver writes it. |
| `00875010`, `00875100`, `008751B0`, `00875240` and their secondary thunks | Resolve the current secondary profile, subtract four from the actual registered pointer, clear the corresponding publication, install `00CE3818`, and free only for flags&1. |
| `008750A0` | Capture fixed-step FLD/FSTP argument, call current element virtual04 and current virtual0C, refresh current payload, reload payload after that callback, and conditionally refresh/set inverse flag/build the inverse. |
| `00875B90` | Capture fixed-step FLD/FSTP argument and call the existing actual-element subnode service. |
| `00874FE0` | Capture fixed-step FLD/FSTP argument, call current virtual08 and refresh current payload. |
| `00875160` | Capture descriptor+08 with FLD/FSTP and call current element virtual04. |
| `00875CAA..00875DF9`, `00875F43..00875FC0` | Walk actual 68h groups until each fixed +34 tail. Capture payload once before admission, preserve the phase-specific byte-test order, get current pool before each descriptor, enqueue the raw node DWORD, and dispatch each nonempty group once. The actual `00E0B6CE` byte is read anew after the current dispatch slot check. |
| `004C1130`, `00BE3020`, `00BE3150`, `00BE2FA0` | Invoke the existing real singleton/lifetime, queue and dispatch/drain implementations. This packet introduces no alternate pool, worker, event, scope counter or queue. |

The native dispatcher keeps atomic LIFO claiming, low-byte worker selection,
current event/array access and worker waits. This bridge adds no exception
rollback. A thrown descriptor/tick callback leaves the pool active flag, shared
scope and captured slot in the state left by the original body.

## Required application attachment

The parent must supply all of the following from its **one actual owner domain**:

1. Actual stable five-group backing for `00F876C0`, pending head/tail backing
   corresponding to `00E0B6D0`/`00E0B704`, and the same current run-pass byte at
   `00E0B6CE` that the fixed-step driver updates. The bridge initializes none of
   these. Producers must run actual parent receiver construction and
   `construct_native_tick_registration_00875890`; typed unit vectors are not a
   substitute for those embedded raw nodes/payloads and current virtual tables.
2. The existing `01090AA0` singleton lifetime domain, actual four descriptor
   publication cells, frame publication `0109CF08`, and its existing lifetime
   bindings using the application's same random registry, native table metadata,
   event owners, `0109DBE4` scope and Win32 worker entry. The entry must receive
   the real pool address and invoke the reconstructed native thread entry.
3. `NativeFixedStepTickBindings` that resolves the **actual receiver's current
   table** and calls its established virtual04/08/0C, pose refresh, inverse and
   subnode bodies. Pure table resolution cannot execute callbacks or mutate
   receivers. Missing class-specific bindings must fail, never report success.
4. `NativeFixedStepJobDispatch` inserted into the same existing dispatcher used
   by callers and workers, with the actual four original primary-table spans.
   Other job owners continue through that existing dispatcher. The actual
   lifetime domain's destructor callback must call
   `delete_native_fixed_step_descriptor_current_00` for these secondary owners
   and route unrelated current profiles through its existing resolver.
5. A borrowed `NativeFixedStepJobWaves` with those references, attached using
   `GameFixedStepHost::attach_native_jobs`. All receivers, payloads, sentinel
   backing and execution/lifetime services must survive dispatch and worker
   shutdown. Pending flushing remains at the existing world-gated row 11.

The bridge's complete dependency interfaces provide no replacement receivers,
success defaults, synthetic registrations or fabricated callable tables. The
ordinary application's raw receiver production/current virtual bindings remain
the limiting parent dependency. This packet does not declare gameplay wiring
active merely because an attachment API now exists.

## Validation boundary

Ignored `local/fixed_step_job_probe.cpp` was built as MSVC Win32 with an embedded
manifest. It links the new service with the existing core and checks actual
`00875890` registration into fixture-backed pending sentinels, the native splice,
the four descriptor factories/secondary lifetime cleanup, serial raw LIFO order,
admission of inactive receivers only during interpolation, one execution per
queued body, unrelated-owner routing and callback failure without pool rollback.
It also checks that interpolation creates/updates its descriptor even when all
groups are empty. The probe uses the real pool constructor's one-processor
zero-worker arm and instrumented downstream tick/pose/subnode services.

This is focused Source fixture evidence, **not** a paired original-body test,
original Win32 calling-convention/EH compatibility, worker participation,
ordinary game startup, graphics or gameplay parity.
The service uses a new explicit C++ interface; raw profile identity is metadata,
not execution of original integer vtables. Valid spans and coordinated owner
lifetime are caller preconditions. Exact native hardware-fault behavior and the
full floating-point state/exception ABI are not established.

`./scripts/build.ps1` passed the full Release Win32 build and both existing
CTests (2/2); its output is in ignored `local/fixed_pool_binding_build.log`.
No permanent test suite was added. Reviewed evidence was appended to the nine
existing Ghidra names/comments through the serialized annotation tool, preserving
old comments, and the program was saved. Prior values are in ignored
`local/ghidra-annotations-20261014T163816Z.json`; all nine affected exports were
refreshed. Live batches verify the existing `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe` before querying or mutating it.
