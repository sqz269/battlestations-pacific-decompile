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

## Validation boundary

Source review and `git diff --check` cover this packet. No new tests, standalone
probe, game launch or Ghidra write was performed. The primary lane owns CMake
registration, strict Win32 build, default-OFF identity, ON execution and visual
capture verification. Until those checks are recorded, this adapter is source
implementation only; it is not build-tested, OFF-identical or game-validated.
