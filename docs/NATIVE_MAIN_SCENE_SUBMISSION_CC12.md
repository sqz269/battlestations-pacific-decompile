# Native main-scene submission: B101C0

This packet reconstructs the complete ordinary B101C0..B104F2 caller over the
existing actual service, camera, command, queue and owner domains. It does not
activate the D5E13C command execution profile or supply its B17240/B156C0 renderer.
The older `SceneRendererHostC` projection in `scene_renderer_cc13.cpp` remains
separate. This source entry adds no replacement scene, camera or callable table.

The original ABI is `__thiscall`: ECX service, scene and camera on the stack,
RET8. The new source ABI borrows the existing `NativeCameraOwner` and explicit
domain context, with a persistent external operation record and command return
for observation. It is not a drop-in binary replacement. The descriptive Ghidra
name remains a hypothesis, not a recovered symbol.

## Recovered schedule

1. Read the actual tracked node at service+1C0 and its +44 hidden byte; publish
   service+22C. When tracking is active, read the current 01090AB0 clock and its
   current profile+1C. Admit the established D68D50:BEE070 body only. The existing
   raw getter returns the real signed tick/frequency pair. Preserve FILD/FILD,
   FDIVP and the float32 interval spill before node world-cache refresh.
2. Preserve the original x87 position copies, float32 delta/division spills,
   accumulation of +244/+248/+24C, velocity stores at +19C and accumulator clears.
   Use the actual 00419440 vector kernel with the existing CRT access; spill speed
   to +1B0, reload the current tracked node, refresh if needed and copy its world
   position to +1B4/+1B8/+1BC.
3. Preserve FCOMIP/JBE and the double D5E1C8 hysteresis expression, including
   unordered behavior and the existing +219/+22D enable/history bytes. Publish
   +22D, then call raw B6DA70 on the current tracked node with factor zero for
   blur or one for active tracking without blur, recurse zero. No tracking skips
   the visibility call.
4. Allocate the actual 44h command before reading the current service+1D4 target.
   Invoke the actual B1F170 constructor with the same command associations and
   publish D5E13C at its actual allocation. The FH3 record DF3EE4 points to the
   single state0 unwind entry at DF3EDC, whose CBBC60 funclet frees the saved outer
   allocation through BF65AC. Constructor failure therefore frees that allocation
   only. Existing constructor partial context/batch acquisition rules still apply.
5. Refresh the existing camera's raw world cache when +5C bit2 is clear. Preserve
   MOVSS/COMISS/JBE: a world Y below current CFBC84 sets the plane `{0,D7A260,0,
   CE3958}` through B70410; equality or unordered clears through B70470. The
   `CameraClipPlaneState` borrows this owner's real +2E0 plane and existing frame.
6. Call the raw 004C11F0 queue getter, then concrete B1EB80 with the actual command
   and flags1, including its real B72190/B1E990 traversal/upload. Build metadata
   `{0,24h,5Ch}`, call the raw queue getter a second time, and update that returned
   queue's current last command through B1CB50. A changed publication is live.
7. If service+B0 is nonzero and the previous service+10 camera differs, publish
   the new actual camera before calling the current CE221C increment import.
   Decrement the captured previous identity through the current CE2220 import;
   a zero result dispatches its current actual owner through the same registry,
   without another decrement.

Later failure does not destroy the command or roll back earlier native changes.
The operation record distinguishes allocation, command construction, the first
and second observed queue, append entry, append return and full completion.
`append_started` is not proof that a pointer was appended; `append_returned`
also means the reached traversal/upload returned.

## Evidence and validation

The installed executable's complete 819-byte B101C0 body matched saved Ghidra
bytes from `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, byte for byte:
SHA-256 `96ef2c8bceda7115291c544d97e7fedf20fd2b9d90798acb2682602c40243590`.
The listing has 215 instructions. FH3 state and cleanup were separately read
from saved analysis. Ghidra naming/comment annotation preserves previous values
in the normal local annotation receipt and refreshes the affected export.

- Strict MSVC Win32 `/MD /W4 /WX /O2 /fp:strict` compilation passed. A manifested
  Win32 link probe forced the new complete entry and real callee closure against
  the freshly rebuilt `bsp_core.lib`; it linked and launched successfully.
- `scripts/build.ps1` passed Release Win32 and all three currently enabled checks:
  `reconstructed_math`, `native_math_differential`, and `tool_tests`. This worker
  does not own CMake registration; the separate strict compile/link proves the
  new translation unit, while the standard build checks its existing closure.
- One local focused fixture compared the motion kernel with rebased original
  B101DD..B103D3 bytes using the same recovered raw timer, world, visibility and
  vector callees. Six scenarios covered normal blur, hysteresis retention and
  rejection, disabled blur, unordered speed, and hidden tracked node. Service and
  node output bytes matched. The native timer's indirect call was resolved to
  the same BEE070 body; this is a bounded prefix comparison, not a full caller
  or native command/renderer differential. The finite scenarios kept world bit2
  set; the refresh branch is source-composed but not exercised by this fixture.
- Focused source clip checks passed below-threshold, equal-threshold and
  unordered-Y decisions and plane words. These checks are not native clip-body
  differential execution. No persistent test suite was added.

Artifact hashes, paths and precise evidence boundaries are in
`reports/native_main_scene_submission_cc12.json`. Receipts are under
`local/main_scene/`; this build did not overwrite the preserved prior packet
receipt under `local/world_draw_receipt_b8c8ff0e/`.

## Remaining boundary

The service and camera must be genuine existing live storage; this entry does
not construct the application scene-renderer service. Canonical clock profiles,
valid native node/array spans, stable prepared associations and a returning CRT
handler are prerequisites. Exceptions cannot cross the naked arithmetic or
existing raw traversal/upload frames; general native FH3/SEH, floating-point
traps and arbitrary original stack aliases are not established.

No actual application invocation, full retained-owner failure differential,
visible scene, native D5E13C execution or gameplay fidelity is claimed. The real
main path continues through D5E13C:B17240 to B156C0. Queue execution must continue
rejecting that profile until those bodies and their real domains are present.
The 867-line/53-callee B156C0 body requires a later specialist decomposition;
a callback-only renderer would not close it.
