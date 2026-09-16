# Actual renderer control worker

The new module reconstructs the actual `24h` `B33DA0` worker, its destructor
and scalar deletion, the `B33C20` thread schedule, and the actual raw-clock
sample `BEE080`. It uses the existing real `BD1970` event owner and Win32
thread APIs. Its normal active render path still requires an actual EndFrame
provider for `B2F4A0 -> B2D8E0`; no complete provider for that path was found.

Bind one `NativeRendererControlWorkerContext` before creating any worker. Its
address is immutable for the process: rebinding to a different context throws.
The binding contains stable references to the application's current clock
`01090AB0`, renderer `F8D394`, and time bits `0108D6E4`, plus the corresponding
original-token profile tables. It must outlive every joined worker and all
calls through the module. There is no owner map, repurposed native field,
second clock, automatic teardown, or unbind operation.

Current profiles are checked before virtual dispatch. `D6821C` selects the
existing real event signal, wait and scalar-delete bodies. `D68D50+20` selects
the new raw `BEE080` sample. `D5F0A8+C` selects the existing substantive
`begin_native_renderer_frame_00b2b200` with the supplied actual begin context.
`D5F0A8+14` requires the supplied real EndFrame provider. Numeric original EXE
addresses are identity data and are never executed as function pointers.
Missing reached renderer providers or unsupported profiles throw a source
contract error. A missing provider does not produce a successful no-op.

The constructor writes `D5F1F0`, clears run/shutdown, thread handle/id and the
shared time, creates two auto-reset events, then calls `CreateThread` with the
raw owner and `CREATE_SUSPENDED`, sets priority `-2`, and resumes the current
handle. Bytes `06..07`, rate `18`, callback `1C` and callback context `20` retain
preimage. Event/thread API failures are retained as native fields; no new
rollback or success policy is added. A constructor allocation exception can
leave prior event acquisitions live, as the original body has no local cleanup.

The destructor captures wake `+C` before writing profile/shutdown, signals that
event, waits indefinitely on the current thread handle, reloads and closes the
handle, and zeros it. It then deletes the current nonnull wake and idle events
with flags1 and clears each slot. Scalar deletion conditionally frees the
captured worker on low flags bit0 and returns its original address bits.

The thread first waits on current wake. While run is set it reloads the current
clock, samples actual storage, and executes the original x87/SSE instruction
order. Accepted samples publish time before an optional native-style ECX
callback using current context `+20`. The lifecycle-busy loop reloads the
current renderer and sleeps10; the separate final busy check remains present.
The current renderer is reloaded separately for BeginFrame and EndFrame.
Rejected samples call `SwitchToThread`. Idle signals the current idle event,
waits on current wake, and only then rechecks shutdown. Normal exit calls
`ExitThread(0)`. The inner run/busy loops do not check shutdown, so a destructor
can wait forever if the original stop protocol has not cleared run.

The timing fragment first rounds signed64 ticks/frequency to float, multiplies
by the exact `1000.0` double while retaining that double on the x87 stack, and
rounds the sample to float again. It subtracts previous float time and rounds
delta to float. Nonpositive or unordered delta takes the original SSE
`negative_zero - delta` arm: `D7A208` is `80000000`, not a wrap period. Rate is
read once for its sign test and again by `FILD`; negative signed interpretation
is corrected with the exact float `2^32` at `CE3978`. The final x87 comparison
against retained `1000/rate` uses the original unordered/JBE behavior. Incoming
x87 control/status and MXCSR rules are retained rather than replaced by C++
`abs`, double arithmetic, or a new clock.

`BEE080` is a raw Win32 ECX/RET4 assembly provider. Its fixed branch preserves
the ordered four-DWORD read/write schedule from clock `+20`. Otherwise it
calls actual `QueryPerformanceCounter` with an uninitialized native stack
buffer, ignores its BOOL result, copies that buffer, and reloads current clock
frequency `+60/+64` in the original order. The QPC failure path has no fabricated
zero or previous-time fallback. The broader clock construction/publication
lifetime is a separate dependency.

The report records exact installed-PE and saved-live hashes for all five full
bodies, the timing constants, and observed tables. No Ghidra mutation was made.
The focused fixture compares the original114-byte timing fragment with source
under two rounding modes, checks fixed and real-QPC clock samples against the
original93-byte body, then uses real worker events/thread creation, rate0 clock
sampling/yield, stop/idle acknowledgement, join and flags1 deletion. Renderer
providers remain absent in that fixture and no accepted Begin/End path is
claimed. QPC/CreateThread failure, arbitrary concurrent mutation, private
FH3/SEH, full original thread ABI identity and gameplay are not established.

## Integrated validation at 6641c05f

The integrated source at `6641c05f524e29f9352f66ddbd017c64d2f99cee` passed the strict MSVC Win32 build and both CTests. The existing focused fixture was relinked against that exact `bsp_core.lib` with `/MD`, `/W4 /WX`, `/sourceDependencies` and an embedded manifest. 16 original/source x87 timing comparisons across two rounding modes; raw clock fixed/QPC paths; real owner create/sample/yield/stop/join/scalar-delete and binding rejection. Active frame path not exercised.

The packet passed 7 numeric call-site checks. Its 5 reviewed function names and evidence comments were applied, saved and read back in Ghidra, preserving previous comments. The combined 15-body checkpoint retains 3310 immutable source, build, compiler, fixture and measured runtime artifacts at `local/checkpoints/6641c05f/native-renderer-components-default/validation.json` (SHA-256 `a2eee5eeedc6e943e5118ec4ceb183866a9019881530fbec771b3063d0f77c2e`). Worker manifests remain historical inputs; their output-file labels are not treated as proof of loaded modules. Full renderer adoption, native exception ABI and gameplay remain unvalidated.

## Main reuse R33

The component is compiled on main with its process context unbound. An actual
EndFrame provider remains required before an accepted worker sample can render.
No production caller binds the context in this packet.

Fresh inspection corrects the clock body extent to **93 bytes**,
`BEE080..BEE0DC` inclusive. The preserved historical report and original worker
fixture header captured only 91 bytes, stopping at the final `C2` opcode and
omitting the `04 00` stack-pop immediate. They are historical receipts, not
complete clock-body validation. R33 regenerates the ignored reference header
from the complete current PE/live span before freshly linking the existing
fixture. The source already emitted the correct `RET 4`; its prior `10h`
destination correction is retained. See
`reports/native_renderer_control_worker_main_r33.json` for current validation.

Source context errors and C++ exceptions escaping the Win32 thread entry remain
outside the returning-worker domain. Native API failures remain observable in
the actual fields: no event rollback, substitute handle, or success path is
added. An invalid thread handle cannot establish a successful join. Even with
valid handles, destruction can wait forever while run remains set, a callback
does not return, or the lifecycle-busy loop does not finish. Stop/idle is a
separate protocol; shutdown is checked after wake, not inside those loops.
