# Native scene and drawable command submission (CC12)

`src/native_render_scene_submission.cpp` closes the raw command-production
bodies `00B1F4D0` and `00B1F5E0`. These now call the existing actual command
constructor, raw command-pointer append, scene traversal, and instance upload.
They do not create a second scene, renderer, command queue, or render entry.
Descriptive names remain hypotheses, not recovered symbols.

## The implemented call chain

| Native stage | Source route |
| --- | --- |
| `BF681B(44h)` and `B1F170(scene,camera,target)` | shared allocation boundary and `construct_native_render_command_00b1f170` with `NativeRenderCommandActualEnvironment` |
| Native queue `+14/+18/+1C` reserve/append | existing `append_native_render_command_pointer_00b1cc20`; raw identities, equality-only growth, current count increment |
| `B72190(scene,command+28)` | `traverse_native_scene_00b72190`, same canonical four-profile mapping and actual render services |
| `B1E990(command)` | `upload_native_instance_groups_00b1e990`, same scene/instance/mapping/render domains |
| Scene label resize/copy/return | concrete raw `41DD40`, `419CC0`, `BD1510`; no `std::string` conversion |

The `NativeRenderSceneSubmissionContext` borrows established domains only.
The caller prepares command associations under their existing constructor
contract; the submission code does not register fabricated owners. A fresh
external `NativeRenderSceneSubmissionOperation` records the physical command
allocation, completed command, append point, and normal return. It supplies no
native field or lifetime behavior. After failure the caller retains the reached
operation and the original associations, rather than replaying construction.

`B1F4D0` receives an owned native eight-byte string by value in the original
ABI. The source API receives the address of that argument storage and consumes
its buffer. The header is left unchanged and must not be destroyed again.
`B1F521` captures the data pointer before resizing the command label. The
copy and normal return use that captured pointer even if a call changes the
argument's current data pointer. Length is reloaded at the original points.
Normal state `-1` is published before the final pool getter. Exception cleanup
uses the current argument header, as `CBCC70 -> 41DD20` does. Constructor failure
first frees the outer `44h` allocation (`CBCC78`), without pretending to undo
the initializer's already-acquired children. Failures after a completed
constructor do not destroy or remove the command.

`B1F5E0` appends before testing `camera+19C & drawable+48`. A mask miss still
queues the constructed command. On a hit, the private source assembly helper
preserves `FLD1`, current profile/context/slot reads, `FSTP32`, `FLDZ`, and the
second `FSTP32` before calling current slot20. Known node/camera/model/Traceline
targets reach the existing concrete bodies. Changed targets in those canonical
tables use the existing nonthrowing application dispatcher; unknown numeric
profiles fail rather than acquiring a synthetic table. Upload follows dispatch.

## Native evidence and admitted boundary

Verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Original disk
and saved Ghidra bytes match for the complete `B1F4D0..B1F5D6` (263 bytes,
92 instructions) and `B1F5E0..B1F6AE` (207 bytes, 71 instructions) bodies.
The original ABIs are ECX queue / five stack words / `RET14`, and ECX queue /
four stack words / `RET10`, respectively. Source APIs add explicit context,
operation, and returned command observation. They are not binary replacements.

The `B1F4D0` FH3 descriptor `DF5188` points to `DF5178`: state0 -> `CBCC70`
(argument string cleanup), state1 -> `CBCC78` (free outer allocation), then
state0. These tables were inspected rather than inferred from pseudocode.
Original SEH/hardware-fault handling, throwing cleanup/derived dispatch,
private-stack aliases, and arbitrary concurrent mutation are not established.
The admitted domain retains existing nonnegative array/span, valid raw owner,
returning cleanup, canonical profile, and nonthrowing traversal requirements.

`local/world_draw/caller_probe.exe` compares the original bodies with the new
source caller. It rebases each original body and redirects its direct callee
calls to explicit recorders. It checks normal scene submission, a changed label
data pointer during resize plus a changed length during traversal/upload, and
a mask-miss drawable. The traces and queue/command/context identities match.
A source-only constructor-failure case agrees with the inspected FH3 cleanup
map; the original exception runtime is not executed. The visible drawable
dispatch helper is assembly-reviewed, not covered by this fixture. No real
mesh, material, D3D draw, main scene renderer, or complete native lifetime is
exercised by those intercepted-callee cases. Exact hashes and build results are
in `reports/world_draw_frontier_cc12.json`.

The standalone TU compiles with MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`.
Both entrypoints also link against the freshly built real `bsp_core` library
and its existing dependencies, without fixture replacements. That link probe
only prints entry addresses; it does not enter a fabricated scene. The new TU
is not registered in this worker's shared CMake file; the primary integrator
owns that registration. Its standalone compile and real dependency link are
separate from the worker's full `scripts/build.ps1` result.

## Exact caller integration and remaining visible-world boundary

These concrete entrypoints replace the formerly callback-only bodies at
`B1F4D0` / `B1F5E0`. Their existing original callers are GUI scene submission,
directional shadow views, excluded-motion-blur drawing, and cockpit drawing.
An actual caller passes its current `NativeRenderCommandQueueStorage`, actual
scene/camera/targets/drawable, and the same prepared command/instance domains.
For `B1F4D0`, it additionally hands over its actual owned label argument.
There is no automatic active-app invocation in this packet: the currently
bound `GameDeviceHost::clear_and_present` still performs native begin, sprite
overlay, and native end. Its complete canonical scene/camera/command-association
graph must be integrated before using these routines in that frame.

The inspected `004E538E..004E54F0` tail updates particle time, foliage visible
sets, interface/sound work, then calls `004CA440` and `004CA1F0`. Assembly also
retains the pending-interface loop that the decompiler marked unreachable.
`004CA440` queues the two jobs and dispatches them, but the old "world view"
label is misleading: `004BBD00` services sound. The main scene route is
`004B4820 -> 0068A0D0 -> B101C0 -> B17240 -> B156C0`.

`B101C0` constructs a distinct `D5E13C` command and calls already-concrete
`B1EB80(queue,command,1)` to traverse/upload. This packet does not substitute
the ordinary `D5E5E0` command for that main-scene profile. The existing raw
`B1EBE0` executor currently admits `D5E5E0/B1D950`; admitting the main-scene
profile additionally requires its real `B17240/B156C0` execution domain.
Closing this command-production gap is Source progress, not evidence that the
dark USN01 frame or gameplay fidelity is fixed.
