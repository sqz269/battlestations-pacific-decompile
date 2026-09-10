# Camera position and look-at dispatch

`src/camera_pose.cpp` reconstructs the complete position setter at `00B6DAE0`,
camera override at `00B71400`, and look-at wrapper at `00B700E0`. All writes go
through the same borrowed `CameraState` fields used by the existing camera
matrix/cache functions. The new C++ signatures are not binary replacements.

| Entry | Full bytes, end exclusive | Original ABI | Proposed descriptive name |
| --- | --- | --- | --- |
| `00B6DAE0` | 47, `00B6DB0F` | ECX object, stack position pointer; tail JMP virtual+34 with world pointer | `BSP_Transform_SetWorldPosition` |
| `00B71400` | 34, `00B71422` | ECX camera, stack position pointer, RET4 | `BSP_Camera_SetWorldPosition` |
| `00B700E0` | 277, `00B701F5` | ECX camera, stack eye pointer then target pointer, RET8 | `BSP_Camera_SetLookAt` |

`00B71400` had no saved Ghidra function when examined. Its entire 34 bytes match
the installed executable and live saved-program memory; capped Capstone decoding
recovers its two calls and RET4. The saved `00B700E0` pseudocode incorrectly has
only one argument and gives unreliable register-call types. Its full assembly
loads the target from the second stack argument and ends in RET8.

## Actual dispatch binding

`CameraPoseAccess` requires the actual owner context, separate table and slot
readers, and selected-entry invocation adapters for virtual+30 and virtual+34.
The owner must represent the supplied `CameraState`. Table/slot values are actual
identities; the adapter must invoke the captured entry rather than resolving a
new override or routing to a fixed implementation. There is no synthetic owner,
default callback, cached override, or shadow camera state in this packet.

Table and slot readers perform only their indicated raw integer load. They must
not use floating point, invoke callbacks, mutate state, retain inputs, throw, or
change the FP environment. This includes preserving a live x87 stack value.
Invocation adapters pass the original references through and may produce the
actual override's effects. Required callback and CRT pointers are checked before
input loads or state changes; rejection is a property of this new adapter API.

The primary integrator owns `NativeCameraOwner` binding, CMake registration,
Ghidra function creation/annotation/export refresh, and ledger edits.

## Recovered order

`00B6DAE0` loads and stores X, then Y, into actual `world[12]` and `world[13]`
(native+120/+124). It loads input Z into ST0, reads the current vtable and its
+34 entry, then stores Z into actual `world[14]` (+128). It dispatches the selected
entry with the actual world matrix itself. Overlapping source/destination words
therefore observe the preceding stores. The host bridge keeps ST0 live across
the two pure integer reads; it adds no FSTP80/FLD80 spill. MSVC object disassembly
confirms an FLD before the bridge call, FSTP afterward, and no FP instructions in
the bridge. The native tail JMP becomes a typed call and return.

`00B71400` clears camera flags+2F0 with `FFFFFE4B`, calls that base setter, then
refreshes direction and target through `refresh_camera_direction_00b70660`.

`00B700E0` selects the current virtual+30 entry before clearing those camera
flags. After invoking it with the live eye reference, it rereads the target
argument, copies three words by x87 into actual target+1A0, and computes three
float32-spilled `target-eye` components. The shared `00419440` length kernel uses
the supplied actual CRT mode/exception boundary. Length is spilled; `FCOMI` and
`JBE` select positive reciprocal versus zero, with unordered taking zero. The
original x87 multiplication and spill schedule writes direction+1AC.

It then calls the complete `00B63F10` builder with live eye and actual target,
using raw up `(0,1,0)`. Changes to inputs or actual target by the position override
or CRT handler therefore affect the later native reads. After the builder it
captures the current vtable, calls `00B63B30` into a distinct temporary world
matrix, loads +34 from that captured table, and invokes the selected world
override. The existing builder, inverse, length, camera setter, and cache methods
are reused; no guessed translation, alternate normalization, or alias snapshot
is added.

## Verification and limits

`reports/camera_pose_dispatch_audit.json` records exact spans, hashes, original
ABIs, assembly sites, and ignored local evidence paths. The preparation script
uses the guarded repository Ghidra CLI, verifying project `bsp` and program
`/battlestationspacific.exe` before each byte batch. All 26 code/boundary spans
and eight data spans match installed bytes or the PE zero-fill definition.

One focused original-versus-host sequence runs three phases across four input
variants and four x87 rounding modes: 48 phases, 36,288 complete raw-field bytes,
80 selected virtual dispatches, and 19 controlled CRT dispatches. It checks
overlapping world-position input, a position backed by actual target, look-at
eye/target backed by actual target/direction, live input edits and vtable A-to-B
switching by virtual+30, signaling NaNs, coincident inputs, and a CRT handler
editing actual target and live dispatch mode. Complete 756-byte owner-field
images, callback input and field snapshots, selected-entry order, actual input
alias identity, x87 status low15 bits (including TOP/condition), control word,
and full MXCSR match. Missing adapter rejection leaves the backing unchanged.

The sparse runner maps only recorded spans, leaves other pages inaccessible and
unused committed bytes as INT3, and rebases 19 verified absolute operands.
Its only patched original entry is the actual external CRT `__87except`
boundary `00C27489`; controlled virtual entries execute the actual native
`00B71400`/`00B71460` chains beneath their observation/mutation adapters.
It does not load the full PE, run its entry point, resolve imports, or replace
unresolved native functions with stubs. The fixture uses a root camera with no
attachment or descendants; it does not construct a native camera or exercise
unmasked FP traps, concurrent mutation, arbitrary hierarchy/virtual overrides,
CRT errno/matherr/SEH behavior, or gameplay/rendering.

The strict MSVC Win32 focused executable compiles the new source plus its actual
dependencies with `/fp:strict /O2 /W4 /WX` and passes. All eight seed spans match;
`scripts/build.ps1` passes the repository build and both existing CTest checks.
The latter build precedes primary CMake registration of this new source; the
focused executable supplies its direct compilation and behavioral evidence.
