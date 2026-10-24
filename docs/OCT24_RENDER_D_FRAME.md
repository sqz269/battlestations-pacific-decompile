# Native mission frame adapter, 2026-10-24

Owned packet: `oct24-renderD-frame`. Source baseline `515837c10`. Files:
`game_native_mission_frame.hpp/.cpp`. Integration, CMake, device ownership and
scene callbacks belong to the primary `oct24-renderD` lane.

The component calls the existing reconstructed `try_begin_render_frame_004c6c30`,
`run_game_render` and `finish_render_frame_004ca1f0`. It does not implement another
phase machine or copy the caller's `GameRenderFrameState`. This is a bounded
adapter for the real ordinary-command scene path, not a recovered whole mission
renderer. It neither runs nor replaces game simulation.

## Evidence and ABI

The verified local configuration is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; this worktree's opening `bsp.py brief` reported that
program live. No new Ghidra export/analysis batch or mutation was performed.

The primary evidence is [GAME_RENDER_FRAME](GAME_RENDER_FRAME.md), the existing
[header](../include/bsp/game_render_frame.hpp) and
[body](../src/game_render_frame.cpp), [GAME_RENDER_TAIL](GAME_RENDER_TAIL.md), and
[RENDER_PATH_SURVEY](RENDER_PATH_SURVEY.md). Descriptive names are hypotheses.

| Original entry | ABI and observed behavior | Adapter binding |
| --- | --- | --- |
| `004C6C30` | `__thiscall(game)`, no stack args, plain `RET`, EAX result; phase 0 to 1 after renderer begin, retained queue blocks begin; a nonzero phase returns success without another begin | `begin_and_clear` calls the primary device's existing `00B2B200` begin and `00B21430` clear once |
| `004CA440` | `__thiscall(game)`, no stack args, plain `RET`; requires phase 1 and stores phase 2 before callbacks | Existing body selects level-1 jobs and invokes the actual scene callbacks below |
| `004CA1F0` | `__thiscall(game)`, no stack args, plain `RET`; phase 2 plus nonretained queue calls `00B2F4A0/00B2D8E0` then stores phase 0 | `end_and_present` reports the actual native present observation |

Original SEH handlers `00C6557E`/`00C65530` are not modeled by the existing typed
bodies. The new API uses ordinary C++ callbacks; it is not binary ABI compatible.
If a callback throws, the exception propagates and the primary must preserve its
failed native graph/owner lifetimes. No synthetic cleanup or successful present
is reported. A false begin result deliberately resets phase to idle and stops
before scene work; native `00B2B200` itself returns success even when inhibited or
lost, so this device-bridge failure handling is an explicit adaptation.

## Integration contract

`NativeMissionFrameCoordinator::run(state, switches, hooks)` accepts the one
actual render state. Initialize its nonphase inputs to the genuine selected
mission branch: game state `0x0D`, interface manager present, interface level 1,
no mission-end suspension, and a gate that selects `kJobPoolScene`. The coordinator
reads these inputs and never rewrites them. Native screenshot capture must be
off; the existing diagnostic capture remains a hook.

Required hooks, in device/frame order:

1. `queue_retained` supplies the actual `00B1BF90`-equivalent predicate to both
   original phase gates; `begin_and_clear` supplies one begin and clear. Do not
   query the nonrendering `presentation()` API from this callback.
2. The existing body enqueues job A then job B. The adapter drains LIFO as
   `00BE2FA0` does. Camera job B (`004B4820 -> 0068A0D0`) invokes `update_camera`,
   then `gather_sort_submit`. The latter must call the existing native scene
   path (`00B1F4D0 -> 00B1F170 -> 00B1EB80` traversal/grouping/upload/queue
   insertion); it must retain the existing real camera, scene, materials,
   geometry and command execution frame identities.
3. `execute_commands` binds `MissionCommandGraph::execute_before_overlay`, which
   executes `00B1EBE0` using the existing ordinary `D5E5E0/00B1D950` route. It
   starts no additional BeginScene/EndScene/Present.
4. `overlay_and_capture` occupies the adapter's diagnostic-flush boundary after
   queue execution. It runs the existing bridge overlay/capture, not native GUI.
5. `end_and_present` invokes the primary's native end-frame once and returns its
   actual success. The native end context remains responsible for its internal
   queue drain/rewind/debug/Present work and native device-loss behavior.

The parent splits the already existing mission ship `draw` into camera,
submission and execution phases. No replacement device or render service is
created here. The adapter refuses missing required hooks and recursive entry.
It returns `kDisabled`, `kBlocked`, `kQueueRetained`, `kBeginFailed`, `kPresented`
or `kPresentFailed`; fallback rendering is appropriate only for `kDisabled`.
Retained queues keep the original state at phase 2: a subsequent call finishes
that state without beginning, submitting or drawing the overlay again.

## Explicit unavailable shell callbacks

One file-log message identifies the bounded scope when a frame first proceeds.
These `GameRenderFrameHost` methods have no native provider in this adapter:

* HUD `hud_render`/`hud_post_render`; profiler-zone registration, finish-slot
  instrumentation and profiler overlay.
* Shadow-owner selection/update/end; both orthographic interface views;
  frontend preview; native GUI and native debug overlay; input debug actions.
* Job A `004BBD00 -> 0068A670` and inline `00735B50`. Despite the historical
  `kWorldView004bbd00` enum label, these submit sound/listener data. Geometry
  belongs to camera job B. No audio work is claimed here.
* Native screenshot directory/capture path. Those unreachable host methods
  throw if reached instead of inventing a successful file operation.
* World scene-post-render, resource flush `00B0D190`, and camera-finalize
  `00B1EF60`. The existing ordinary command route still owns command retirement;
  this adapter does not claim the main-scene/world cleanup graph is integrated.

The diagnostic-flush callback is repurposed as the explicit host bridge slot.
All other unavailable shell callbacks above are no-ops or return absent. No
console output is added.

## Default OFF visual-pass switches and blockers

`native_mission_frame_switches_from_environment(ship_enabled)` enables the
adapter only with the existing `BSP_NATIVE_MISSION_SHIP` master active and
`BSP_NATIVE_MISSION_FRAME=1`. Every switch defaults false and requires exactly
the value `1`. With the frame disabled the adapter calls no hooks and changes
no state. The five subordinate flags are read only under that master.

Each pass below is an explicit no-op. With the frame master enabled, all five
emit their state and stated provider blocker once per coordinator through the
supplied file logger: `=0 disabled: blocker=...` or `=1 no-op: blocker=...`.
Reporting precedes the begin gate, so a retained queue or failed begin does not
hide missing frame components. Terrain, ocean
and weather slots follow camera setup; post and tonemap slots follow queue
execution. These are adapter boundaries for unavailable work, not evidence of
the original multipass order.

| Switch | Required providers before a real pass can be enabled |
| --- | --- |
| `BSP_NATIVE_MISSION_TERRAIN` | Retained landscape/visibility nodes, authored 33x33 patch vertex/index streams and materials, current-camera culling and native section-pool bindings (`00ADD3F0/00ADE0E0/00ADC930/00AE1A10/00AE47E0`). Collision height fields alone are insufficient. |
| `BSP_NATIVE_MISSION_OCEAN` | Retained world+A8 ocean and shared simulation spectrum/clock, projector/grid, float16 height textures, reflection/refraction targets and ocean effect execution (`00B9AE00/00B9D7B0/00BA5030/00B9F0A0/00BA51C0`). Do not advance a second independent wave field. |
| `BSP_NATIVE_MISSION_WEATHER` | Retained atmosphere/weather with sky/cloud geometry and materials, particle emitters, and main-scene execution bindings (`0078CFF0/00BBCF60-00BBDEC0`; atmosphere producer `00BBDFF0`, sky/cloud children `00BBF8A0/00BBDA50`). |
| `BSP_NATIVE_MISSION_POST` | Main-scene target ownership, post-effect instances/materials, and complete `00B156C0` continuation execution/admission. Ordinary-command drawing alone supplies none of those. |
| `BSP_NATIVE_MISSION_TONEMAP` | Retained HDR scene/luminance targets, exposure state and tone-map effect/material bindings in that main-scene pipeline. A dedicated original tonemap entry/order was not established in this packet. |

[CC13_TERRAIN_RENDER](CC13_TERRAIN_RENDER.md) supplies foliage culling/bake/load
kernels and the heightmap patch kernel. It does not establish a mission-level
native terrain producer/resource graph. Its builder/vtable choices remain
open. [CC13_WATER_RENDER](CC13_WATER_RENDER.md) section 10 supersedes the older
claim that projector/grid/beam routines were unreconstructed: `00B9AE00`,
`00B9D7B0`, `00B9A990` and `00BA2FF0` were reconstructed and build-tested. They
remain unwired, with upload/capture/resource ownership still needed. Neither
document therefore establishes an independently ready visual pass to invoke
from this isolated packet. No optional real pass is added.

Current-source revalidation also found `native_main_scene_execution_prefix.hpp`
still explicitly leaves `D5E13C` unadmitted and stops at `00B1645B` before its
post draw or `00B164B8` before gather. The multipass blocker is therefore present
in this checkout, rather than inferred only from the older survey.

## Sequential GUI command handoff

The follow-up adds two methods to the existing private `MissionCommandGraph`:

```cpp
void submit_scene_with_label(void* scene, void* camera, void* targets,
    void* actual8b_label);
void retire_executed_command_before_overlay();
```

The label-aware method borrows the address of the original eight-byte
`NativeString` argument header. The header's data belongs to the same canonical
string pool as `submission.strings`. It passes that exact address to
`submit_native_scene_00b1f4d0`; the adapter does not copy, relabel, clear or destroy
it. The native body still performs its own evidenced command-name copy and
consumes the argument's buffer on normal and exceptional exit, leaving the
header unchanged. The caller must not destroy it after native entry. If a
pre-entry validation or command-acquisition check fails, consumption has not
started and the caller retains its original cleanup responsibility. Existing
`submit_scene` keeps its internally constructed `"Mission ship"` label.

Both methods use the same `pending` record, acquisition observation, submission
frame and already-bound `queue_frame`. There is one retained command companion
at a time, with multiple successful sequential commands permitted inside one
actual BeginScene/Present. The explicit retirement method requires a nonnull
pending command and delegates to `complete_frame`: ready/executed, no failed
state, empty native queue/context, and retired context/batch owners are all
required before resetting the companion. It performs no EndScene or Present.
The existing completion counter is a command ordinal when two commands are
issued in a device frame, despite its historical `frame=` log field.

The primary lane enforces `overlay_window_open` and at most one GUI submission
per actual begin epoch. After successful ship execution it retires that
companion, submits the GUI's real scene/camera/targets and original label,
executes the GUI command, then lets normal native end-frame complete the GUI
companion. The bridge overlay/capture follows both native commands. Failures
retain the same graph and propagate; a second graph must not be constructed,
because it would replace the shared frame's execution-context/frame bindings.

The existing pure pass-context getter is
`bsp::cc13_front4::render_service_word_1d4_00b0d0d0(Raw service)` from
`cc13_front4.hpp/.cpp`, where `Raw` aliases `bsp::cc13_sweepA::Raw`. It reads one
Win32 dword at `service+0x1D4`, with no host calls or mutation. Original ABI:
`__thiscall(ECX=service)`, no stack args, plain `RET`, EAX result. The GUI adapter
must read the current actual `00F8D39C` publication and convert that returned
word through `uintptr_t` to the targets/pass-context pointer. `+0x1CC` belongs
to the different `00B0D1C0` getter and is not interchangeable. This is a source
API, not a newly recovered binary-compatible call interface.
The service pointer must be live and nonnull; the getter supplies no guard.
Forward its returned value unchanged, including a null targets pointer.

## D2 MatrixIndexedModel render and palette contract

Live bytes from the configured `bsp.gpr` program establish that actual profile
`00D63590` uses `00B748E0` at vtable `+0x20`, `00B6E8C0` at `+0x48` and
`006EF890` at `+0x58`. It is not a distinct matrix-only render body. The existing
`render_native_generated_model_00b748e0` implements the original complete normal
body and already recognizes the sphere/visibility leaves. Original render ABI:
ECX actual model, stack context/LOD/visibility/flags, `RET 10h`; the source adds
EDX access while retaining the original stack-word positions and x87/SSE body.
The source interface is not a drop-in original ABI or native FH3 proof.

The D2 extension binds a genuine borrowed `00D63590` table in
`NativeRender20Profiles`, using the matrix owner's
`NativeMatrixIndexedModelEnvironment`. It must share the exact existing base
`NativeModelEnvironment`; no static-model table is substituted. Captured numeric
profile `00D63590` translates to that canonical current table, after which the
existing target selector reaches `00B748E0`. The model facade's render services
must return that same table for actual matrix models. Null optional bindings
leave the profile unsupported. The canonical owner/pool/constructor and bone
materialization are separate units/model packets.

The existing material constant builder has two different palette contracts:

* Shader-definition byte `+0x30 != 0xFF`: `00B8FF00` reads the signed count at
  model `+0x188`; `00B90620` reads the node from `[[model+0x184]+index*4]`.
  `00B426A8..00B426E4` refreshes that genuine node's world matrix and transposes
  the first three rows from node `+0xF0` into successive 48-byte register data.
  This is the ordered pointer-array contract needed by MatrixIndexedModel.
* Shader-definition byte `+0x32 != 0xFF`: the separate skin path expects either
  optimized animator/palette storage at `+0x190`, or 0x60-byte records containing
  node pointers and inverse-bind matrices. It is not interchangeable with the
  matrix-indexed pointer array.

The Val resource's authored WeightMapNames must therefore resolve, in their
original order, to actual hierarchy nodes with their authored/world transforms.
The model packet supplies those nodes and the original `+0x184/+0x188/+0x18C`
header. Runtime material observations must still establish the selected shader
register contract; identity palettes or inferred bone names are not substitutes.
No new render body, palette implementation, test or live-validation claim is
added by this translation packet.

Both `NativeRender20Profiles` constructors accept the trailing optional
`const NativeMatrixIndexedModelEnvironment*` after the optional group pointer.
For the ordinary mission route, construct with
`(camera_environment, model_environment, group_or_null, matrix_environment)`.
The matrix environment's `base` must be that exact model environment, its
node runtime must be the camera's same runtime, and its actual `00D63590`
table must be nonnull. The appended table is at source-record offset `0x18`;
the group's table/environment offsets and every render-access offset stay
unchanged. Only integer captured-profile translation is extended; original
render-body FP instructions and outgoing stack words are untouched. The packet
depends on the separate matrix-owner header from the units worker and is not
independently build-tested.

## Validation boundary

Source review and `git diff --check` cover this packet. No new tests, standalone
probe, game launch or Ghidra write was performed. The primary lane owns CMake
registration, strict Win32 build, default-OFF identity, ON execution and visual
capture verification. Until those checks are recorded, this adapter is source
implementation only; it is not build-tested, OFF-identical or game-validated.
