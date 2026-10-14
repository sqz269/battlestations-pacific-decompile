# Main-scene execution prefix and actual camera history

This packet supplies the genuine B156C0 entry through B1586C, its complete
actual-storage B0DB00 history calculation, the separate B1586D..B15BD1
texture/far-corner continuation, the B15BD2..B15E77 bloom/target stage, and
B15E78..B15EC3 camera-frame/mode/system-gather continuation, and the
B15EC4..B15F62 DRSH label/ownership clause, and B15F63..B16067 batch
selection, preparation and execution. The current final boundary is immediately
before B16068, which tests the optional excluded-node pass.
Reaching any boundary does not mean a scene command executed successfully. The
existing queue must continue refusing D5E13C: B17240 preparation and the
remaining B156C0 render passes are not installed or claimed complete.

The implementation uses actual service/command/camera/clock storage and existing
raw camera, matrix and CRT kernels. It creates no renderer, scene, camera,
history, owner or callable table. The older callback-based scene-renderer
projection remains separate and is not used as a replacement execution body.

## Executed prefix

`execute_native_main_scene_prefix_00b156c0` implements B156C0..B1586C, a 429-byte
native prefix. Its original function has ECX service and one command stack
argument, RET4; this new source stage takes explicit borrowed context and a
persistent external continuation record.

1. Capture the actual F8D394 renderer publication and current profile+2C. The
   established D5F0A8 slot is B1FE20, the frame-active getter. An inactive frame
   returns without reading the command or service. This is distinct from the
   B20240 gate in the outer B17240 wrapper; the prefix does not add that gate.
2. Repeatedly obtain the current command-context camera through full raw B1BF40.
   Capture its +1D4 near-plane bits, perform the original x87 CE3800 argument
   store and set the same actual camera through raw B6FBF0. Capture current +190
   through full raw B6FE60, then replace the saved high byte with FF. The
   external record retains these exact words for the later original restores.
3. Obtain projected Y and X axes through full raw B70EA0/B70FE0 using the existing
   canonical axes/CRT domain. Normalize actual service+19C velocity through the
   existing 00419510 kernel. Preserve the original x87 dot products, spill
   boundaries and CEC9E0 double multiplication into service+1A8/+1AC.
4. Reload the actual 01090AB0 clock, load its accumulated float at +04 and retain
   its two original float32 stores around the next current-camera getter. Run
   full raw B0DB00 on the actual service+314 history.
5. Copy the history result at service+55C to service+5A8 through raw 004134F0.
   Obtain the next current camera's real B70490 view-projection cache, invert
   it through full raw B632D0 into the original temporary shape, and copy that
   inverse to service+5E8 through 004134F0. Return at B1586D.

Every original camera getter reloads the current command context. The prefix
does not snapshot a single camera or replace its world/cache/matrix storage.
The axes/history contexts must borrow the same actual CRT and constant cells;
their identity checks are source-domain admission, not native owner operations.

## Complete history body and rounded lerp

`update_native_camera_history_00b0db00` supplies all B0DB00..B0E924: 3,621 native
bytes and 817 instructions. Original ECX is the actual history, with time float
bits and camera on the stack, RET8. EDX adds the borrowed source context. The
294h storage is delay+00, count+04, eight 48h samples at +08 (world matrix, FOV,
time), output view-projection matrix+248 and prior position+288. Native-valid
counts 0..8 are required; the implementation adds no clamp or initialization.

The complete schedule preserves distance-based restart, forward x87 sliding of
the seven retained samples when count is eight, current world/FOV/time capture,
count publication, the original time-search comparisons, fraction clamp and
unordered handling. It then performs position and axis interpolation, all four
rounded square-root/normalization stages, both cross-product stages, interpolated
FOV projection, scaled-affine inverse and matrix product. It stores the full
result before the final world refresh and previous-position publication.

The original zero-duration and unordered fraction paths remain intact. There is
no fresh-history shortcut, alternate sample search, repaired axis or substituted
perspective calculation. Seven calls resolve to established raw/CRT domains:
B6DB70, 00419440, BF7030, B642F0, B63B30 and 00413920, plus the newly concrete
005803E0 lerp.

`lerp_native_float3_005803e0` supplies all 177 native bytes, with original ECX
output, EDX old vector, target/fraction stack arguments, RET8 and EAX output.
It retains the separate x87 binary32 difference/product/add stores, current
source reads and overwrite of its captured first public argument slot. The
older value-returning `main_menu_map_lerp_005803e0` interface is unchanged.

## Register and lifetime evidence

The listing's abbreviated `FMUL ST1` at B157EA is ambiguous without its bytes.
The installed instruction is **DC C9: `FMUL ST1,ST0`**, so it multiplies ST1 and
leaves the scale in ST0. A first bounded comparison exposed the wrong direction;
the final source uses the verified opcode direction at every one-register x87
arithmetic instruction. This preserves the negative half-scale in both projected
velocity constants.

The history helper saves an extra EBP for its context. Only the two public
arguments move by four bytes; all native scratch offsets remain unchanged. The
native search already saves/restores EBP around its endpoint pointer. Constant
translation preserves live EAX and adds no floating operation or spill. The
existing raw projection provider receives the original writable argument slots;
its new plain-RET ABI is balanced by an explicit 10h stack adjustment.

The B156C0 prefix has not armed a native cleanup state before its boundary.
Changing the camera near plane is not rolled back here. The fresh external
record, actual service/camera/history and borrowed domains must survive for a
future genuine continuation. The raw arithmetic/CRT callees must return normally;
C++ exceptions across the naked helpers, native FH3/SEH, floating traps and
arbitrary original private-stack aliases are not established.

## Texture constants and complete far corners

`continue_native_main_scene_texture_constants_00b1586d` implements the complete
869-byte B1586D..B15BD1 clause. It requires the same actual service/command and
the persistent state returned by the earlier prefix. It leaves the saved near
word, saved color, and earlier continuation flags intact, then publishes
`reached_bloom_stage` only after the complete clause returns.

The original first B4CB10 getter captures a texture for the later height query;
the second getter obtains the current texture for width. This schedule repeats
for the full-texel pair. Each getter reloads service+4C and uses the existing raw
holder+08 body. Current captured D61948 slots+3C/+40 resolve to the established
raw B3CE50/B3CE60 getters over the actual texture+28/+2C descriptor words. No
nominal dimensions, retained owner, COM query or replacement texture is added.
The native owner-profile capture is kept, and the selected borrowed table slot
is checked at the reached dispatch; the owner profile is not reread.

Signed FILD followed by the original 2^32 correction preserves unsigned DWORD
conversion, including the high-bit branch. Half-texels go to service+04/+08;
full texels go to +184/+188. The SSE constant initialization and all eighteen
separate x87 multiply/store operations produce nine XY sample offsets at
service+F4..+180. Zero dimensions retain native infinities and NaNs.

The final current-camera getter feeds complete raw B0EB10..B0F008, 1,273 bytes
and 277 listing instructions. The helper retains the near-plane FCOMIP/JBE
decision, including unordered behavior; writable raw projection argument
slots; full matrix inverse and vector4 transform; homogeneous divide and its
float32 stores; rounded far/near scaling; eight original world-rotation passes;
and all four final XYZW stores. The world matrix is refreshed at each reached
original bit2 test. The helper writes output only after the calculation, so
output/camera overlap retains the native store order.

The new far-corner context is saved above the original local frame. Only the
two public camera/output offsets move by four bytes; all original scratch
offsets remain unchanged. Real B642F0/B632D0/B62D10/B6DB70 callees are used.
The context borrows the same native one, negative-one and half cells as the
texture stage. All one-register x87 directions were derived from installed
opcodes, not the abbreviated listing operand.

The texture continuation has not armed a native cleanup state either. Its
arithmetic/domain callees must return normally. No rollback is added on failure;
unsupported profiles are rejected rather than dispatched as numeric addresses,
but C++ exceptions crossing the naked helper are outside the valid contract.

## Bloom and actual frame targets

`continue_native_main_scene_bloom_targets_00b15bd2` supplies all 678 native bytes
of B15BD2..B15E77. Its 120-instruction arithmetic helper runs the original
service+21A branch. When enabled, it repeatedly reads the current service+2C
bloom and actual B54CD0 output holder+20, then gets width/height through the
same concrete texture domain. It preserves unsigned conversion, half-texel
stores at +18C/+190, current +258/+25C/+260 adaptation values divided by the
actual double20000 cell CE3CB8, and both native lower/upper/unordered clamp
schedules from +264/+268 into +274/+27C. Disabled bloom leaves those fields
untouched and joins the shared screen-constant matrix at +1D8..+214. The shared
half and negative-half cells remain native inputs, not capability defaults.

The ownership suffix uses normal C++ exception frames. It gets the current
camera, captures its full +188 clear-flags word in the external state, then
gets the current camera again for flags7 and again for white clear color. The
prior near/color restore words remain unchanged. The original white word comes
from four byte stores of BL=FF, established at B1573D and preserved across prior
callees; its private stack-store order is outside this new source interface.

For slots0/1/2, it independently reads service+38/+3C/+40, calls the existing
full B4CB20 primary-surface getter, then reloads current service+1C8 and invokes
full B1FAB0. The same actual surface domain publishes and retains incoming
before releasing the captured outgoing owner at zero. No extra owner credit,
null replacement, automatic cleanup or post-failure rollback is added.

Finally it reloads the actual F8D394 publication through that same surface
domain, captures the renderer profile, reloads current service+1C8, and resolves
current D5F0A8 slot98 to full B24E70. The genuine binding provider keeps its
original optional guard, frame retain/release, current F8D398 sRGB setting,
color unbind/bind sequence, depth binding and current device methods. The
setting is not inferred from device capabilities. Material/effect execution
has not yet been reached; the actual bloom output owner is only read here.

The persistent state records whether this stage started, each reached call
site, the saved clear-flags word and completed clear changes. A failure retains
earlier mutations and the active call site. A started stage cannot be replayed.
`reached_camera_frame_stage` is published only after target binding returns.
The B156C0 caller still has no cleanup state armed at this boundary; the actual
child owner/renderer providers retain their own cleanup obligations. Exceptions
across the naked arithmetic helper, native caller EH/SEH and arbitrary private
stack aliases remain outside the source contract.

## Camera frame, mode and system gather

`continue_native_main_scene_camera_gather_00b15e78` supplies the complete
21-instruction, 76-byte clause B15E78..B15EC3. It invokes existing complete
actual-storage B71360 using the first current B1BF40 camera. That provider
retains enabled17C gating, three separate renderer/profile reads, genuine
B285A0 preparation, B26770 viewport and B21430 clear dispatch, including the
original FLD/FSTP outgoing depth argument. No provider is replaced by a semantic
camera view or a callback body.

After B71360 returns, a second current camera supplies the saved full mode word
at +198. A third current camera receives mode6 through the new full raw
`set_native_camera_render_mode_00b6fdf0`. Its seven instructions and 28 emitted
bytes match the installed function: ECX actual camera, stacked DWORD mode,
EAX same camera, RET4; full +198 mode store precedes the +19C masked-shift store.
The added unused EDX formal keeps the original mode argument stacked. This raw
body does not project a `CameraFrameState` or update an owner companion.

A fourth current camera supplies scene+A4; a fifth supplies the camera argument
to full existing B46A70 system gather. The captured scene is retained across
the fifth getter. The service receiver is unused in this clause: B46A70 retains
its own current service-publication reads. Source boundary checks require the
same actual renderer publication/profile, synchronization, axes/CRT, timer and
texture domain as the preceding contexts. They do not capture renderer values
early or replace the callee's current-slot checks.

The caller provides one fresh persistent `NativeSystemConstantGatherFrame`
with its initialized 1,232-byte native prefix preimage. Unwritten words are
preserved; no blank prefix is synthesized. The original warm/cold sampler
operation belongs to that frame, which must survive failure. Earlier near,
color and clear-flags saves remain unchanged. New state records frame completion,
saved mode, mode mutation, gather start and the reached native site. No cleanup,
mode restoration, rollback or replay is added. Existing provider limitations
on native exception identity, naked-frame propagation and private stack aliases
remain; this stage has not armed the later B156C0 string-cleanup state.

## DRSH label and exact cleanup ownership

`continue_native_main_scene_drsh_label_00b15ec4` supplies all 51 instructions
and 159 bytes through B15F62. The original caller zeros both local8B header
words, resizes to4 through the actual raw pool, captures the resulting data in
EBX, and copies current length+1 from D5E500. The source borrows those actual
verified bytes; it does not introduce a C++ DRSH literal or a string projection.

Only then does B15EF7 capture current F8D39C. B15EFE arms state0 even when that
service is null. Nonnull service uses the actual +684 header, preserving the
header-identity skip, requested source length, post-resize current source-length
test, current destination length/data and the earlier captured source pointer.
The source follows the existing raw-string policy of omitting zero-byte host
memmove calls. No terminator copy is added to the service assignment.

The installed/saved handler CBC499 loads FuncInfo DF4698 and jumps BF6B43.
Its nine-entry map at DF46BC gives state0 -> (-1,CBC460), state1 -> (0,CBC46B),
and state2 -> (-1,CBC46B). CBC460 addresses the current temporary at EBP-88
and tail-calls 41DD20. CBC46B passes an ignored ECX value to B13510, which
resets the currently published service label. State1 is not reached by this
clause. B15F42 changes state0 to state2 BEFORE normal temporary release:
the later 419CC0 failure path resets the label and must not retry the temporary.
Normal return uses captured EBX data and current temporary length+1, evaluated
before the returning pool getter. The header is not cleared.

The persistent `NativeMainSceneDrshLabelFrame` owns the external8B header,
captures, reached site and cleanup state. State2 remains armed at B15F63;
future batch/material code must retain that frame and same raw pool/service
domain until the original reset boundary. No frame destructor performs cleanup.
The explicit unwind adapter consumes only reached state0 or2 and disarms before
the real action. Source exceptions in this clause invoke the proven active
action, retain failed-frame diagnostics and prior mutations, then rethrow.
An exception during cleanup terminates. This reconstructs source ownership
behavior without claiming the original FH3/register/stack/fault ABI.

## Validation

- Installed executable and saved Ghidra body bytes matched for the complete
  3,621-byte history, 177-byte lerp and 429-byte execution prefix. The verified
  project was `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
  Original constant bytes confirm near0.5, restart20, one1 and direction-0.5.
- Both translation units passed strict MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`.
  A manifested source-entry link probe linked the real callee closure against
  the fresh `bsp_core.lib` and launched successfully.
- A static machine-listing audit matched all 648 unchanged tagged x87
  instructions in the history/lerp source and 41 in the prefix: 689 total.
  The context-bound constant loads and shifted public time slot are separately
  reviewed adaptations, not included in that unchanged-encoding count.
- One local fixture compared rebased original B156F6..B1586C plus complete
  original B0DB00/005803E0 bodies against the source prefix/history. All 23
  service/camera byte comparisons passed across ordinary rolling eight-sample
  history, zero-delay unordered values, reset-distance transitions and actual
  world-cache refresh. The fixture uses the same established raw camera/math/CRT
  callees, with argument/context bridges for their new source interfaces. It
  separately checks the source inactive-frame exit and continuation fields.
  The original outer gate, full renderer, native EH and CRT callback/exception
  branches are not differential-tested by this fixture. Floating controls use
  the ordinary masked environment; FP status/stack identity is not compared.
- `scripts/build.ps1` passed the current Release Win32 build and all three checks:
  reconstructed_math, native_math_differential and tool_tests. This worker leaves
  registration to the primary owner, so the new units are verified separately by
  strict compilation/linking. No persistent test suite was added.

The follow-up texture packet independently matched saved Ghidra and installed
bytes for the complete 1,273-byte far-corner body and 869-byte continuation.
Both changed/new units passed the same strict Win32 flags, then a manifested
probe linked the fresh real core/dependency libraries and launched. The ordinary
build again passed all three existing tests; permanent registration remains
primary-owned. The new camera file needs registration alongside the two prior
units.

Its one bounded original/source fixture passed eight complete texture-stage
service/camera comparisons and eight complete far-corner camera/output alias
comparisons. Cases cover ordinary, one, zero and high-bit dimensions; near below,
equal, above and unordered; raw world refresh and a singular projection. The
original virtual dimension lookup is translated to the same concrete raw getter
in this fixture; current-profile dispatch is source-reviewed, not separately
native-compared. Masked ordinary floating controls are used; FP status/stack,
floating traps, native EH, concurrent mutation and private stack aliases remain
unproved. A machine-listing audit matched 164 unchanged x87 instructions in
the full far-corner body and 74 in the new clause, plus all 41 prior prefix
instructions. The checked total is 279; constant-pointer/public-argument
adaptations are reviewed separately.

The bloom/target follow-up matched all 678 installed/saved Ghidra caller bytes.
Its manifested fixture passed five complete original/source stage comparisons
of service, camera, renderer and both frame-group byte states, plus incoming and
outgoing surface counts. Cases cover inactive/active bloom, zero/high-bit
dimensions, lower/upper/equal clamp branches and unordered weights. Every case
changes the bound frame and invokes the real B1FAB0/B24E70 providers. A genuine
D3D9 HAL device and three real render targets are used; device queries confirm
all three current target identities and enabled sRGB state. This is bounded
backend-state evidence, not visible world output.

The fixture translates only the known native dimension/renderer virtual slots
to the same concrete source bodies. It does not prove original table dispatch,
terminal-zero deletion, enabled synchronization, child exceptions, concurrent
publication changes, FP status/traps or original exception ABI. Controlled owner
preimages keep counts positive and retain the real lifecycle context; no
deletion callback is substituted. Separate source checks confirm rejected
renderer dispatch preserves completed mutations/call-site state and disallows
replay. The current encoding audit matches 46 new x87 instructions plus all
279 preceding ones, 325 total. Strict Win32 compilation, fresh real-core link,
fixture launch and all three ordinary build tests passed again. No new source
unit or persistent test suite was added in this follow-up.

The camera/gather follow-up matched all 76 caller and 28 mode bytes against both
installed PE and saved/live Ghidra data. Three original/source caller comparisons
use the same complete current B71360 and B46A70 providers with an actual D3D9
HAL device. They cover disabled camera execution, enabled viewport/clear, and a
real Clear forwarding observer that changes the command's camera publication
before B71360 returns. The later mode save/write and system upload must then
use the new current camera. Both camera byte arrays, renderer and service bytes,
the saved mode and all 77 registers from each real VS/PS bank matched. Device
queries also confirm the reached viewport. This fixture uses a warm sampler,
supplied prefix preimage, no lighting owner, synchronization mode0 and masked
ordinary FP; it does not independently revalidate every original descendant.

Five direct original/source mode calls cover 0, 6, 31, 32 and FFFFFFFF, comparing
all camera bytes and returned identity. The emitted COFF leaf is byte-identical
to the installed 28-byte body. Caller listing review verifies all five camera
getter calls and the original eight-call sequence, including the scene+A4 load
before the last getter. No new x87 arithmetic is added; all prior 325 tagged
x87 encodings still match. A source-only late domain rejection occurs after the
real camera frame and mode6/mask40 writes; these mutations and saved mode remain,
gather has not started, and a repaired context cannot replay the started state.
Strict compilation, manifested fresh-core fixture link/launch and ordinary
Release Win32 build with all three tests pass. No new persistent tests or source
unit were added.

The DRSH packet matched its159 caller bytes, both cleanup actions, handler,
FuncInfo, all nine unwind-map entries and borrowed D5E500 bytes against installed
PE and saved/live Ghidra. Three rebased original/source caller comparisons use
the same complete actual 41DD40/419CC0/BD1120/BD1510 providers and a genuinely
constructed raw string pool. All 8AD4A0 pool bytes and service headers match for
null service, changed label length and same label length with small returns
disabled. Three isolated original CBC46B comparisons change the publication
after the normal stage and confirm cleanup resets the new current service.
One isolated CBC460 comparison uses a current temporary buffer different from
the captured EBX buffer and confirms only the current header drives cleanup.

The tests also verify retained prior near/color/flags/mode words, stage2 ownership
at the boundary, unchanged temporary header after return, replay rejection and
no cleanup for a source-domain failure before state0. Emitted code review checks
the publication capture before state0, original alias/copy reads, captured source
pointer, state2 before the returning getter, argument capture before that getter,
and cleanup disarming before the actual action. All325 inherited x87 encodings
still match; this clause adds no floating arithmetic. Strict compilation, fresh
manifested real-core probe and the ordinary three tests pass.

Original caller calls and cleanup tail jumps are relocated to the same genuine
current source providers. The test proves the isolated cleanup actions, not full
native FH3 exception dispatch, child-throw propagation or private stack aliases.
It uses a warm pool and does not prove cold singleton construction, manager
failure, returning-getter failure, concurrent publication or complete renderer
behavior. No callback allocator or projected pool replaces an actual operation.

`reports/native_main_scene_execution_prefix_cc12.json` records artifact hashes,
validation limits and annotation receipts. Outputs are under
`local/main_scene_execution/`, `local/main_scene_texture/` and
`local/main_scene_bloom/`, `local/main_scene_camera/` and
`local/main_scene_label/`. All fifteen
preceding main-scene artifacts, including its exact old core library, were
preserved before the prefix build under
`local/main_scene_receipt_3ccc58cbf/receipt.json`. All 34 artifacts from immutable
prefix commit `450a751a0`, including its core library and prior receipt, were
verified and preserved before the follow-up build under
`local/main_scene_execution_receipt_450a751a0/receipt.json`. Before the
bloom/target build, all 36 artifacts from immutable `ed43121f9` were verified
and copied to `local/main_scene_texture_receipt_ed43121f9/receipt.json`. Before
the camera/gather build, all 35 artifacts from immutable `ca9c9c4c0` were preserved
under `local/main_scene_bloom_receipt_ca9c9c4c0/receipt.json`; those copies and
all preceding receipts were verified again. Before the DRSH build, all41
artifacts from immutable7452af434 were copied and verified under
`local/main_scene_camera_receipt_7452af434/receipt.json`.

## Actual batch selection and execution

`continue_native_main_scene_batches_00b15f63` covers all 90 instructions and
261 bytes at B15F63..B16067. Its persistent batch frame carries the caller's
material-pass frames and progress markers. It retains the preceding DRSH frame
and the same publication/context domain; it creates no replacement renderer,
queue, job dispatcher or batch owner.

The current raw byte at 0108D6DC is tested first. Only zero permits the two
short-circuited B51B20 reads, both interpreted as **signed greater than 50**.
The serial path performs prepare0, execute0, prepare1, execute1. It reloads each
command batch after preparation, captures that batch's profile before B1BF40,
and reads the captured profile's execution slot only after the current-camera
getter returns. The native numeric profiles select complete raw B51DF0/B55550
providers; no host callback table stands in for an unknown target. D62064's
base +08 purecall remains unsupported at execution; concrete D5E5AC is required.

The parallel path writes batch0/1 mode +08 before the corresponding actual
4C1130 frame getter. Each current command batch and returned pool+04 are
captured before the B0FFB0 preparation-job getter. The current pool profile
then selects real BE3020 enqueue. A third frame getter precedes real BE3150
dispatch with use-workers=1, followed by execute0 and execute1. These providers
share the actual manager, queue/renderer publication and preparation dispatcher
with their existing raw job lifetime domain. No queued job, scope counter or
native mutation is rolled back on failure.

State2 remains armed at the successful B16068 boundary. A source C++ failure
sets the persistent failure markers and invokes the same explicit CBC46B action
adapter, which disarms before resetting the **current** global service label.
Replay is rejected without consuming live state2. The later normal disarm at
B16097 and B160A2 reset are still outside this clause.

Installed PE and saved/live Ghidra match all 380 bytes in the caller, both
leaves and borrowed profiles/literal used by the focused comparison. Five
original261B/source pairs cover mode 128, first count 50, second count 50,
signed first count -1, and both counts 51. Actual nonempty pointer-slot sorting
and B55550/B45360 material traversal run in both lanes; material descriptors,
visibility 0.5 and camera mode6 take the genuine no-selected-pass exit. Whole
batch/entry/material/texture graphs, sort queue, string pool and frame-job pool
bytes match. The original caller's direct calls and native virtual targets are
bridged to the same complete source providers; original count/camera leaf bytes
execute unchanged. This checks caller composition, not every descendant anew.

The parallel case creates one genuine Windows worker through BE3040, uses
BE3150's wake/resume/drain/wait behavior, and shuts the worker down through
BE30C0. It does not attribute either preparation job specifically to the worker
instead of the calling thread's drain. Warm singleton publications are used;
cold manager/queue/frame/job creation and registration are not exercised here.
One additional source-domain failure after real preparation verifies retained
mutations and state2 cleanup. Original FH3 dispatch and native child-throw
propagation are not tested. There are no selected material passes, device draws,
application route, visual or gameplay claims.

The manifested Win32 fixture reserves a private PE section before process
startup, avoiding ASLR allocation conflicts at the original numeric table/code
addresses. It verifies MEM_IMAGE ownership before writing fixture bytes; all
existing PE section bytes, including the embedded manifest, are preserved.
Strict /MD /W4 /WX /fp:strict compilation, the standard build and all three
existing checks pass. Emitted code review confirms the signed predicates,
reload/capture/slot order, serial interleaving and retained cleanup state; all
325 inherited address-tagged x87 encodings still match. No persistent test
suite was added. New artifacts are under `local/main_scene_batch/`; all 46
previous artifacts were copied and hash-verified before edits/build under
`local/main_scene_label_receipt_9ca98fd55/receipt.json`.

## Next executed clause

B16068 tests service+22D; its enabled path invokes B10500 at B1607B. B10500
currently has only the older projected implementation and is the next concrete
dependency to reconstruct. It must not be silently skipped. The following
camera render-mode restore, state2 disarm/current-label reset, later scene and
post-effect/cockpit/distortion/statistics paths also remain required. D5E13C
continues to be unadmitted pending the complete raw body and application graph.
