# Canonical application frame-job graph

Packet `cc12_native_frame_job_application` supplies a concrete, stable Source
execution/worker graph in `GameNativeFrameJobApplication`. It borrows the existing
actual `NativeRenderJobPublicationContext`, raw `01090AA0` manager, frame
publication `0109CF08`, four descriptor publication cells, startup `RandomThreads`
registry and shared `0109DBE4` atomic counter. Construction installs bindings;
it does not create a pool or choose a processor count.

`frame_owner()` invokes the existing complete raw getter
`get_native_frame_jobs_actual_004c1130`. The original `004BFA40`/`00BE3040`/
`00BE2C70` construction reads the original `NUMBER_OF_PROCESSORS` environment
getter, creates the original pool/event arrays and real suspended Win32 workers.
There is no replacement pool, private semantic lifetime manager, fallback worker
count, shadow queue, fake job or replacement random registry.

## Worker and execution composition

The application reads the actual `D6821C`, `D68650`, `CE7554`, `D0DEB4`,
`D0DEC4`, `D0DEAC`, `D0DEBC` table spans and `D0DE84` literal from the existing
`GameNativeReadOnlyData`. That provider verifies the supported original executable
and supplies non-executable original data. Missing spans or unexpected current
table entries fail; the component contains no fabricated table arrays.

Both caller and worker execution use the same `NativeFrameJobExecution` and
`NativeFixedStepJobDispatch`. Other actual job owners continue through the
application's required existing dispatcher. Class-specific current tick virtuals,
pose and subnode services remain required bindings, with no success default.

The stdcall worker entry receives the **actual secondary pool address** supplied
by `CreateThread`. An explicit Source process entry binding resolves the stable
application service and checks that pool against its current canonical frame
publication. This binding is C++ execution context, not an invented native game
global. It is installed before suspended-worker construction and retained until
the original scalar destruction has stopped/joined every worker. The entry calls
`native_frame_job_thread_entry_00be2ba0`; the original reconstructed worker body
alone registers/unregisters random state and owns wait/reset/claim/signal and stop
behavior. The adapter adds no done signal, slot clear or successful failure return.

Descriptor factories now have an explicit `NativeFixedStepJobDescriptorsActual`
path through `SoundLifetimeAccess` over the same raw manager cell. The preceding
semantic fixture API remains available. Actual raw waves invoke the existing raw
frame getter instead of requiring a semantic manager. The shared singleton
deletion map routes `CE7550` through its existing frame scalar destructor and
`D0DEB0`/`D0DEC0`/`D0DEA8`/`D0DEB8` through the new actual descriptor resolver.
The same bindings and publications survive the full canonical raw manager drain.

## Failure and teardown

Native queue/dispatch bodies still control LIFO claims, low-byte worker selection,
current events, caller draining, worker waits and shared scope. The Source graph
records an interrupted callback, owner acquisition or actual wave failure and
rethrows without rolling back the original pool/slot/scope state. An interrupted
graph requires process retention. The parent must inspect
`requires_process_retention()` **before** its normal raw singleton drain and keep
the graph, mapped data, random registry and downstream services alive until OS
exit. Freeing descriptors ahead of a failed pool with queued jobs is unsafe.

On a normal run, the application performs its existing raw manager shutdown.
That drain deletes the registered descriptor secondaries and original frame
primary; the original frame scalar owns worker stop/join, event deletion, array
free and publication clear. Only after the same manager publication and all five
job publications are null may the parent call `retire_after_manager_drain()`.
It removes Source bindings without draining owners or changing native order.
The same pre-drain check also refuses a pool with remaining/active jobs or
retargeted execution bindings, because descriptors pop before the frame owner.
Destroying a still-live/interrupted graph terminates instead of releasing live
worker dependencies. There is no automatic partial-state cleanup or replay.

## Required parent hooks

1. Retain this component in `GameStartupHost` after its existing random registry
   and mapped data exist. Supply the canonical render-job publication context,
   that `GameSingletonHost`'s deletion map/raw manager cell, same shared scope,
   actual descriptor cells and complete current tick/remaining-job bindings.
   An already-owned frame execution graph cannot be replaced or adopted.
2. Use this component's owner/queue/dispatch services for the application's frame
   jobs. Existing renderer `begin_frame`/`end_frame` and `GameFrameProfiler` are
   separate services; their buffers, frame worker and chosen profiler slot count
   do not supply the canonical `0109DBE4` pool scope or raw tick registrations.
3. After genuine raw parent receiver/tick construction establishes groups,
   pending sentinels and the actual driver byte, call
   `bind_actual_fixed_step_groups` and attach the returned service to
   `GameFixedStepHost`. Repeated binding must preserve those exact identities.
   This creates no receiver or node and initializes no fake empty group.
4. Check retention before normal/fallback singleton shutdown. Keep all graph
   dependencies through the canonical raw drain, detach fixed-step borrowers,
   then retire the graph before destroying the random registry or mapped data.

These hooks remain explicit. The compiled canonical owner service is available;
the ordinary game host is not declared active merely because that service exists.
Raw receiver production, genuine remaining render-job bodies and actual caller
attachment must be established by their parent owners.

## Evidence boundary

This packet composes already reconstructed bodies; it recovers no new x87,
register-ABI or control-flow body. Original addresses are `004C1130`, `004BFA40`,
`00BE3040`, `00BE2C70`, `00BE2BA0`, `00BE2EA0`, `00BE3020`, `00BE3150`,
`00BE2FA0`, `004BFB30`, `004BFAC0` and the four descriptor families described in
`NATIVE_FIXED_STEP_JOB_WAVES.md`. C++ names remain hypotheses and all component
interfaces are explicit Source C++, not native ABI replacements.

The focused ignored application probe uses verified mapped original metadata,
an actual raw manager, original pool construction and a real worker. Its isolated
child receives `NUMBER_OF_PROCESSORS=2` to exercise the original one-worker arm;
production code never overrides that input. Tick and remaining-job callbacks are
fixture instrumentation, not reconstructed game receiver/render bodies. A
separate failure child deliberately retains the graph until process exit after
checking original active/scope/slot state and refused retirement. Native ABI/EH,
paired original-body equivalence, ordinary game startup and gameplay remain open.

MSVC Win32 direct compilation and both application children passed against the
exact matching 200-byte core in this worktree. The normal child also confirms
Source graph retirement after the canonical drain. `./scripts/build.ps1` passed
the Release repository build and both configured CTests (2/2). The application TU
was directly compiled/probed while its shared CMake registration was queued;
primary registration and the combined Main build remain integration checks.
Readonly compiler/object/executable, build and probe receipts are retained under
ignored `local/frame_job_application_receipts_20261014`; their hashes and source
dependency provenance are in `reports/game_native_frame_job_application.json`.
The probe uses the existing suspended-child metadata handoff instead of relying
on unreserved fixed bands after CRT startup. Linking a stale Main-root archive
with the appended Source deletion binding is invalid; this proof uses the matching
worktree core and header/dispatcher, with all checked old offsets preserved.
