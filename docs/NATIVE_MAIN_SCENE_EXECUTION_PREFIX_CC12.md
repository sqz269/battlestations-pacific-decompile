# Main-scene execution prefix and actual camera history

This packet supplies the genuine B156C0 entry through B1586C, its complete
actual-storage B0DB00 history calculation, the separate B1586D..B15BD1
texture/far-corner continuation, and the B15BD2..B15E77 bloom/target stage.
The current final boundary is immediately before B15E78 camera-frame execution.
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

`reports/native_main_scene_execution_prefix_cc12.json` records artifact hashes,
validation limits and annotation receipts. Outputs are under
`local/main_scene_execution/`, `local/main_scene_texture/` and
`local/main_scene_bloom/`. All fifteen
preceding main-scene artifacts, including its exact old core library, were
preserved before the prefix build under
`local/main_scene_receipt_3ccc58cbf/receipt.json`. All 34 artifacts from immutable
prefix commit `450a751a0`, including its core library and prior receipt, were
verified and preserved before the follow-up build under
`local/main_scene_execution_receipt_450a751a0/receipt.json`. Before the
bloom/target build, all 36 artifacts from immutable `ed43121f9` were verified
and copied to `local/main_scene_texture_receipt_ed43121f9/receipt.json`.

## Next executed clause

B15E78 next obtains the current camera and invokes B71360 frame execution.
Later work changes camera mode, submits system constants, prepares and executes
batch/material passes, and performs the concrete
post-effect/cockpit/distortion/stats paths. These remain required. No hidden-node
bypass, callback-only renderer, D5E13C admission, application invocation, visible
world pixels or gameplay equivalence is supplied by these stages.
