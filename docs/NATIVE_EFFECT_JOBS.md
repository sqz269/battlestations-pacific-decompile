# Native effect phase-two jobs

Addresses: `008663B0`, `008663D0`, `00866400`, `008667F0`, `00866C60`.

The complete `00866C60` phase-two method now queues the manager's raw event
references through the existing frame singleton and its real worker pool. The
job callback `008663B0` calls each event's current virtual `+28` with the shared
arguments read when that job executes. The callback owner has an independent
eight-byte singleton lifetime; it has no intrusive reference count.

Evidence was checked against saved `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed executable with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Names are descriptive hypotheses, not recovered symbols. The compiler scalar
classification/name `CG_scalar_deleting_dtor_00866400` remains correct.

| Entry | Complete inclusive range | Native interface |
| --- | --- | --- |
| Phase two | `00866C60..00866CC9`, 106 bytes | ECX manager, stack float delta/reference, RET 8 |
| Job singleton | `008667F0..008668BD`, 206 bytes | No inputs, EAX current F87658, RET |
| Job callback | `008663B0..008663CE`, 31 bytes | ECX job ignored, stack raw event, RET 4 |
| Secondary scalar adjustor | `008663D0..008663D7`, 8 bytes | ECX secondary, flags forwarded, tail jump |
| Primary scalar | `00866400..00866433`, 52 bytes | ECX primary, stack flags, EAX original primary, RET 4 |

## Dispatch and event resolution

`00866C60` reads the count at manager `+20`, publishes the reference to F8760C,
reads the backing pointer at `+1C` twice for captured end and begin, and publishes
delta to F87608 with MOVSS. It preserves native DWORD arithmetic. It never
retains, filters, clears, or replaces these non-owning cells.

For each captured cell it first gets the actual frame owner through `004C1130`,
then reads that cell and captures frame secondary `+04`, then gets the effect
job singleton through `008667F0`. Only afterward does it read the captured
pool's current table and enqueue slot `+04`. `NativeFrameJobExecution` now
validates that current slot against the established CE7554/D68650 profiles;
the actual `00BE3020` queue operation remains unchanged. A final `004C1130`
and current virtual `+08` dispatch with argument 1 always run, including when
the captured span was empty. The final frame may differ from earlier captures.

`008663B0` loads F87608 through x87 FLD, reads F8760C, captures the raw event's
current virtual `+28`, rounds delta through FSTP, and invokes that virtual. It
does not inspect event activity/type, complete it, cancel it, or retain it.
Later jobs therefore observe any shared-argument changes made by earlier jobs.
The actual point owner table D0D3EC contains only two entries and is not an
event `+28` dispatch table.

The C++ job executor borrows `PointEffectChildEvents` and requires a pure
raw-event-to-existing-companion lookup. `NativeGamepadForceEvents::find_actual`
searches the same existing canonical list without binding, allocating, changing
counts, or introducing another owner domain. The current rumble callback route
continues through the already implemented `00872150`. Other event owners and
job classes require their actual existing implementations. `NativeEffectJobDispatch`
checks the job's current primary identity and current D0D3E8 word zero on every
execution, and delegates other job identities to the required application
executor.

The new C++ ABI captures delta with an x87 FLD/FSTP helper and reads the current
reference between them. Pure host event resolution follows that capture. Native
table loads occur before FSTP; the C++ interface does not claim native table-fault
ordering, original exception-dispatch identity, or arbitrary asynchronous table
mutation parity. Actual event, span, job, and table lifetimes remain caller
preconditions.

## Eight-byte singleton lifetime

F87658 points to primary word D0D3E8, whose word zero is `008663B0`, and
secondary word D0D3E4 at allocation `+04`, whose word zero is `008663D0`.
The next table at D0D3EC is a distinct point owner table. No count occupies the
job allocation's second word.

On the fast path `008667F0` returns the current publication without a manager
call or lock. Otherwise it gets the actual shared lifetime manager through
`00415350`, captures manager `+10`, enters its actual Win32 critical section,
and increments the actual recursion counter only after OS entry. Under that
captured lock it rechecks F87658, allocates eight bytes if absent, and writes
secondary D0D3C8, primary D0D3E8, then secondary D0D3E4 before publication.
The null allocation branch explicitly publishes null.

The getter reloads publication and captures its secondary address or null
**before** the second lifetime-manager getter. It registers that captured
pointer with `00BD0C30`, decrements the captured counter, leaves the captured
OS section, then reloads the current publication for its return. Registration
failure preserves the published owner; there is no raw-allocation cleanup guard.
Every adapter shares the actual application's `SingletonLifetimeDomain`.

The EH handler at C94D48 selects FuncInfo DC6B7C. Its sole state-zero map row
at DC6B74 is `(-1, C94D40)`; that funclet forms the captured guard at EBP-14
and jumps to `00411EE0`. Allocation/registration exceptions only release the
captured lock. The original EH dispatcher was not executed in the fixture.

`00866400` unconditionally clears the current F87658, restores CE3818 to the
captured allocation's secondary word, frees the primary allocation only if
flags bit zero is set, and returns that primary address. It preserves primary
word zero and does not unregister. `008663D0` subtracts four from the registered
secondary receiver and tail-calls that scalar. Shared-domain shutdown has
already popped the registration; application lifetime routing must check the
registered interface's current table and invoke this adjustor. A valid nonnull
owner is required: the original's apparent null branch subsequently writes
address zero and is not successful null-object handling.

## Validation and limits

`local/extract_effect_jobs_ah.py` verified every byte of all five complete bodies,
all RET immediate bytes, and the handler/map data against both live Ghidra and
the installed PE. `reports/native_effect_jobs.json` records their hashes and
all direct/indirect call sites.

The focused ignored `local/effect_jobs_probe_ah.cpp` compares original relocated
206-byte getter, 52-byte scalar, 8-byte adjustor and 106-byte empty dispatcher
against C++. It checks normal/null allocation, the fast path, actual secondary
registration in the real shared lifetime manager, actual lock/counter state,
unconditional global clearing with a different current publication, flags
0/2/1, primary return identity, ESP balance, and real frame construction,
one actual Win32 worker, empty dispatch, and teardown. Current enqueue-table
mutation is rejected. C++ allocation exceptions leave the captured lock released
and no registration/publication. A throwing actual registration-validation
callback preserves the published job, performs no raw free, and releases the
captured lock; the fixture restores its malformed pointer header and explicitly
retires that unregistered owner after checking these results. Native getter manager/registration calls use
an ABI view of the same actual manager/OS section, and allocation/free bridges
invoke the same allocator with explicit null/failure injection. Native empty
dispatch bridges to the actual frame getter and actual pool dispatcher; no fake
job executor succeeds. All executable probes embed a manifest.

Strict direct Win32 compilation with `/W4 /WX /fp:strict` passed for changed
translation units. The primary integrator performs the CMake build, CTests,
nonempty application integration, Ghidra updates, and shared-ledger recording.
The standalone worker fixture did not execute the original 31-byte event
callback or nonempty phase-two loop. None of these checks establishes gameplay,
physical device output, native exception-dispatch execution, or complete event
class coverage.

## Primary integration validation

The combined strict Win32 build and both existing CTests passed. The separate `local/effect-job-callback-probe-ah.log` executes all31 original callback bytes, checking execution-time shared arguments, current virtual28, RET4/ESP, masked signaling-NaN quieting/invalid status, signed zero, unchanged actual20h event/count/request state, and complete lifetime teardown. Its temporary native event-table ABI bridge restores the real table identity before the canonical no-op method. The manager fixture additionally runs nonempty C++ phase-two dispatch through the actual Win32 pool and effect-job singleton, nine job updates and complete secondary teardown. Borrowed event-list population is explicit fixture input. Original106-byte phase-two execution remains limited to the earlier empty-span fixture; the original231-byte manager uses actual C++ callee bridges. See [LIVE_EFFECT_UPDATE.md](LIVE_EFFECT_UPDATE.md).

Recovered signatures, names and evidence comments were saved in Ghidra, checked by readback with prior comments preserved, and re-exported. Native exception ABI, physical device output and gameplay remain unvalidated.
