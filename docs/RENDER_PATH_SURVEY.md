# Mission world render-path survey

Packet `oct18-render-survey`, 2026-10-18. Source baseline:
`3a4e910974253c92d5bd380f15598c1c253c4856` (`origin/main` when inspected).
Read-only analysis; this packet changes this document only. No host code, ledger,
Ghidra annotation, game launch or new visual-validation claim accompanies it.
The configured analysis is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; `bsp.py brief` reported that program live and
`bsp.py ghidra count` confirmed 76,774 functions, matching the snapshot.
Names below are descriptive hypotheses unless they identify library APIs.

## Finding

The device/presentation path exists. The mission does **not** currently enter the
reconstructed world submission/execution path. Its `004CA440` host method records
the call and advances the separately modelled ocean field. Constructing more
low-level draw functions alone will not make mission geometry appear: the missing
work is composing live scene/model/camera/material owners and their execution
contexts, then connecting mission submission to those owners.

Two corrections to the packet's starting assumptions matter:

* `native renderer after raw drain: owner=null ...` is a **teardown** observation.
  `GameStartupHost::destroy_singleton_lifetime_manager` and exceptional/destructor
  cleanup call `drain_singletons`; `after_native_drain` emits the line after the
  worker has joined. It is not evidence of a null renderer during the mission.
  `GameDeviceHost::clear_and_present` uses that application's renderer every frame.
* The renderer constructor already constructs shader-state definitions
  (`0108FE90`), the system-constant registry (`0108FE94`) and the renderer Lua owner.
  The missing render-resource service (`00F8D39C`) is a different object. Normal
  startup still logs `Phase 4 renderer_resources` as unimplemented at `00B14A10`,
  although an explicit `construct_render_resources()` API and its raw constructor
  exist. Null values after shutdown cannot distinguish these cases.

Evidence: [mission host](../src/game_hosts_mission_frame.cpp),
[application/device/startup hosts](../src/game_hosts.cpp),
[renderer application](../src/game_native_renderer_application.cpp),
[renderer constructor](../src/native_renderer_constructor.cpp),
[application frame graph](../src/game_native_renderer_frame.inc).

## Call tree and current reachability

In the trees, `R` means an ordinary raw/image-storage implementation exists;
`T` means a reconstructed typed or host-interface body exists; `P` means a raw
fragment/continuation rather than a complete admitted entry. These describe source
coverage, not original binary ABI, native EH equivalence or runtime proof.
`W` means reached by normal `bsp_game` startup/frame execution, `H` means the live
host currently records or projects the operation, and `N` means the world route is
not connected. A routine can have both typed and raw implementations.

```text
Application frame / GameFrameHost::game_on_move                         T/W
  -> menu/mission dispatch -> GameMissionFrameHost::run_mission_frame_004e4a40
     -> run_mission_frame (mission_state_frame.cpp)                    T/W
        -> 004CA440 GGame::Render, site 004E54D1                        T/H
           live binding: record + ocean_live_field_render_job_0068a0d0
        -> 004CA1F0 finish, site 004E54EB                               T/H
  -> GameLoopCallbacks::after_application_frame                        W
     -> GameDeviceHost::clear_and_present                              W
        -> 00B2B200 BeginFrame -> D3D9 BeginScene                       R/W
        -> 00B21430 clear; frontend/HUD sprite overlay                  R/W + bridge/W
        -> diagnostic capture callback (if requested)                  W
        -> 00B2F4A0 -> 00B2D8E0 EndFrame/Present                       R/W
```

The intended `004CA440` branch is below. Its world-related nodes are `N` in the
mission application; the first tree's device bracket is the bracket actually used.

```text
004CA440 run_game_render                                               T/N
  -> HUD virtual +0C/+04                                               interface/N
  -> shadow view 00A8F3B0 (when light, shadow owner and camera exist)    T/N
  -> 00BE3020 enqueue twice -> 00BE3150 dispatch                        R/T/N
     -> 004B4820 camera/render job [LIFO, runs first]                   T/N
        -> 0068A0D0 submit main scene                                  T/N
           -> 0078CFF0 environment -> camera/fog/ocean update            T + raw fragment/N
           -> 00B101C0 service(scene=game+19EC, camera=game+19FC)         R/N
              -> 00B1F170 construct command; install D5E13C profile     R/N
              -> 00B1EB80 append/gather/upload                         R/N
                 -> 00B72190 scene traversal                          R/N
                    -> node slot +20: 00B6D990 / camera 00B6FB80        R/N
                    -> model 00B748E0 -> mesh entries 00B72F80         R/N
                       -> 00B1DFF0 instance collection                 R/N
                 -> 00B1E990 instance-group upload                     R/N
              -> 00B1CB50 metadata; retain/update camera               R/N
           -> 004C7E60 after-submit helper                             T/N
     -> 004BBD00 sound listener/frame job                              T/N (not geometry)
  -> alternative ortho 0059D7B0 / 00535430; else 00735B50 sound inline   T/N
  -> GUI/preview/debug producers                                       reconstructed domains/N
  -> 00B1EBE0 execute queue                                            R + T/N
     -> command virtual 0, D5E13C -> 00B17240                           T/N; raw route unadmitted
        -> system-prefix gather 00B46A70                               R/N
        -> 00B156C0 main-scene multi-pass body                          T + P/N
           -> target/camera/gather; DRSH and SCNE batches               R continuations/N
           -> 00B51DF0 prepare each batch                              R/N
           -> 00B55550 execute batch entries                           R/N
              -> effect D61A00 slot +14 -> 00B45360                    R/N
                 -> 00B44750 geometry binding -> 00B43410 draw pass    R/N
                    -> 00B42350 material constants                     R/N
                    -> 00B21D10 / 00B21C20 shader bindings              R/N
                    -> 00B21820 / 00B218C0 constant uploads             R/N
                    -> indexed: 00B24010 -> D3D9 DrawIndexedPrimitive   R/N
                    -> other:   00B21B40 -> D3D9 DrawPrimitive          R/N
           -> depth/post/restore stages                               T + P/N
  -> scene post-render, resources/listener flush, world labels          T/R dependencies/N
004CA1F0 -> end frame / optional screenshot -> Present                  T/N at mission binding
```

`kWorldView004bbd00` in `game_render_frame.hpp` is a misleading historical enum
label: the export of `004BBD00` is `BSP_SoundFrame_RunJob`, and it builds a listener
matrix and dispatches sound. `00735B50` is likewise the inline sound service.
Geometry submission is in the other job, `004B4820 -> 0068A0D0`. Preserve the
native ordering rather than treating both jobs as world-draw calls.

The ordinary command path provides a smaller initial target:

```text
00B1F4D0 submit scene / 00B1F5E0 submit drawable                         R/N
  -> 00B1F170 + ordinary D5E5E0 profile -> traversal/00B1E990 upload      R/N
  -> 00B1EBE0 -> D5E5E0 virtual0=00B1D950                              R/N
     -> target/camera 00B71360 / system gather 00B46A70                  R/N
     -> 00B51DF0 -> 00B55550 -> same material and D3D draw leaves above R/N
```

This is suitable for an explicitly selected diagnostic model milestone. It does
not reproduce the original mission's main-scene multipass behavior.

### Exact boundaries that prevent connecting these arrows today

| Node/domain | Reconstruction and wiring | Current host object or missing contract |
| --- | --- | --- |
| `004CA440`, `004C6C30`, `004CA1F0` | Typed complete control bodies; mission bindings do not call them | `FrameBinding` records render/finish; frontend bindings also record. Separate device loop owns Begin/Present. A real adapter must own one phase sequence and avoid beginning/presenting twice. |
| `004B4820`, `0068A0D0`, `0078CFF0` | Typed bodies exist in `cc13_left8_seg8.cpp`, `frontend_seg16_24_28_cc13.cpp`, `cc13_front7.cpp`; raw fog interval is narrower | Mission substitutes `ocean_spectrum.cpp`'s live-field simulation hook. No render camera/environment/service is supplied here. |
| scene submission/traversal/upload | Raw bodies exist | No native mission scene or live unit-model/geometry association. Four canonical node/camera/model/Traceline profiles are supported by traversal; arbitrary terrain/ocean/particle profiles require their genuine dispatch. |
| ordinary `00B1EBE0` queue execution | Complete raw ordinary-command route | `FrameGraph::end` supplies null execution providers while native counts are empty. A nonempty queue requires persistent command frames, owners, camera, system, batch and material contexts. |
| main command `D5E13C/00B17240` | Typed `scene_renderer_cc13.cpp` body exists; raw executor does **not** admit it | `native_render_queue_execution.cpp` explicitly requires `D5E5E0/00B1D950`. Submitting a main command alone will fail admission, not draw it. Destructor and immediate/last-command execution domains need matching profile support too. |
| `00B156C0` | Typed broad orchestration exists; raw prefix and continuations are partial | `native_main_scene_execution_prefix.hpp/.cpp` currently reach `00B1645B` on the POST true arm or `00B164B8` on its false arm. Later post draw/gather/restore work and whole-entry composition/admission remain. Do not mark the whole body complete because DRSH/SCNE batches exist. |
| `00B55550 -> 00B45360 -> 00B44750/00B43410` | Raw complete operations exist | Require actual effect/pass/material/mesh-section storage, current program states, stream/index buffers and live parameter/light owners. The menu sprite bridge supplies none of this world graph. |
| `00B42350` and final draw leaves | Raw material builder and both draw routes exist | Older `RENDER_COMMAND_EXECUTION.md` describes the builder as open; current `native_material_constant_build.cpp` supersedes that statement. Availability is not application wiring. |
| D3D terminals | Raw renderer methods call device `+144h` / `+148h` | Existing `IDirect3DDevice9`; final world draws are unconnected. Frontend `DrawPrimitiveUP` in `game_hosts_frontend.cpp` proves the separate UI bridge, not this route. |

Implementation references:
[game render body](../src/game_render_frame.cpp),
[mission frame](../src/mission_state_frame.cpp),
[main submission](../include/bsp/native_main_scene_submission.hpp),
[scene submission](../include/bsp/native_render_scene_submission.hpp),
[scene traversal](../include/bsp/native_scene_render_traversal.hpp),
[raw queue](../src/native_render_queue_execution.cpp),
[ordinary execution](../src/native_render_command_execution.cpp),
[main-scene frontier](../include/bsp/native_main_scene_execution_prefix.hpp),
[typed scene renderer](../src/scene_renderer_cc13.cpp),
[material entry](../src/native_material_entry_dispatch.cpp),
[material pass](../src/native_material_pass_execution.cpp),
[indexed terminal](../src/native_renderer_indexed_draw.cpp),
[primitive terminal](../src/native_renderer_draw_primitive.cpp).

## Native object inventory and synchronization risks

This inventory covers the ownership domains needed by the traced path, including
objects already constructed. It distinguishes a constructor's existence from a
complete asset-loading or mission-lifetime adapter. A typed host record is not an
image-shaped owner even when it holds the same position, name or clock.

| Native object / publication | Constructor or producer; reconstruction | Mission construction/wiring and duplicated host state |
| --- | --- | --- |
| renderer `00F8D394`, `1D94h` | `00B32410`, raw complete | **Already constructed** by `GameNativeRendererApplication::construct`; device creation and presentation run. Reuse its device, pools and publications; do not introduce a second renderer beside `GameDeviceHost`. |
| definitions `0108FE90`, system constants `0108FE94`, renderer Lua `00F8D434` | `00B585A0`, `00B5BF70`, `00B1BB90`, raw constructors called by `00B32410` | **Already constructed.** Actual per-frame scene/camera/fog/material inputs still need binding. Recreating registries would duplicate the application's definitions, Lua and canonical owner domains. |
| render-resource/service `00F8D39C`, `6ACh` | `00B14A10`, raw complete; base publication `00B0F020`, child textures `00B52550` | Normal startup records it; explicit application constructor/provider APIs exist. Needs shared surfaces, post effects, listener and proper destruction. Its history/targets must follow the same device generation as `GameDeviceHost`. |
| queue, command/context, batches, render-entry cache | Queue acquisition `004C11F0`, constructor `00B1F280`; command `00B1F170`/context `00B1EDC0`; cache startup fragment `00BED1E8`, reconstructed raw APIs | Frame graph already provides queue lifetime/getters and initializes the cache. Missing are **nonempty** execution/destruction contexts and mission command producers. Do not replace these with a second host vector or reuse per-command continuation frames. |
| scene root `game+19EC`, `24h`; base node | `00B724E0`, raw complete in `native_gui_scene_storage.cpp`; node `00B6F5A0`, raw complete | Mission load does not materialize this root. Despite the source's GUI-oriented name, `GAME_WORLD_CONSTRUCT.md` identifies this constructor for `World`. Host scene-contents/entity registries need a stable entity-to-native-node association, including attach/detach/destruction. |
| Operator camera `game+19FC`, native `458h` payload plus pool identity; viewport/fog | `00B71A80`, complete owner constructor; viewport domain already available; fog `00B84E50`, complete initializer | Application exposes the canonical camera environment but does not construct the mission Operator. Host navigation/controlled-unit/scene poses are separate. Publish one camera's pose, FOV, aspect, clip planes and viewport at the render boundary; do not run a second camera simulation. Preserve original allocation versus pool-slot sizes. |
| directional light `game+19F8`, shadow owner | Game setup `004C9EC0`; light `00B7C6B0` over base `00B7C4C0`, reconstructed actual-owner bodies | Mission lighting/light-to-scene attachment remains absent. Startup's native shadow target is already constructed (`GameNativeShadowTargetApplication`), but that does not create the mission light/view. Sync authored environment/light parameters, not only a host sun color. |
| materials, effects, shader programs and textures | Material `00B18900`; effect base `00B18D60`, effect `00B407A0`; program compiler `00B3C3A0`; descriptor `00B43B00`, reconstructed native APIs | Application has compiler/effect/cache provider domains and some UI/font consumers. Unit/terrain/ocean material instances, their parameter owners and geometry bindings are not created by the mission. Reuse VFS/string/texture/owner registries; preserve per-instance damage/remap/visibility and texture lifetime. A shared effect identity is not a whole model material. |
| unit models and geometry | Model `00B75030`, raw owner constructor; mesh/resource loading, section selection and instance upload are separate existing domains | `GameUnitsHost` and scene factories create typed simulation records, not native draw models. Map each stable entity ID to its native model; publish transforms/LOD/visibility and damage state after simulation/interpolation. Asset names/HP tables alone do not load `.mmod` geometry or GPU streams. |
| atmosphere/sky `game+19E8`, `40h` | `00BBDFF0`, typed `water_weather_cc13.cpp`; sky-box child `00BBF8A0` and cloud manager `00BBDA50` are dependencies | Not materialized by mission `construct_world`. Typed source reconstructs atmosphere fields and delegates children; this is not proof of a ready raw cloud graph. Share authored sky/time-of-day/environment state with Lua and camera fog. |
| ocean environment `game+19F0`, `B8h`; wave field and spectrum | `0078DAA0`, typed `cc13_sweepB.cpp`; field producer `00BA6FB0`, spectrum `00B97100`, setter `00B9DD90` | Not materialized as native owners. Host already advances a wave field through `ocean_live_field_render_job_0068a0d0`. Bind the same clock/amplitude/sea-level source to native rendering; avoid two independently advancing oceans that disagree with buoyancy. Field geometry, material, reflections and shadow-camera ownership remain integration work. |
| terrain/landscape, quadtree, render objects and foliage | Height field `00ADD290` (typed `cc13_sweepC2.cpp`), base/weak link `00AE9870`, quadtree `00AEA8C0`, tile `00ADF6B0`, render objects `00AD9E60/00AD9F90` (typed `terrain_particles_cc13.cpp`) | Host terrain/collision/contact or scene records do not constitute a rendered landscape. Need the actual landscape manager/load path, tile/chunk/GPU buffers and current camera culling. Constructor bodies exist, but a complete mission-level raw producer chain was not established by this survey. Use the same terrain origin, scale and heights as simulation. |
| particles, emitters and particle models | Emitter definition `00AFA280` plus cone/sphere/smart-area constructors; type base `00B01150`; emitter `00AFF5F0`, model `00AF74A0`, arrays `00AFD2E0`, reconstructed native APIs | Global pool/clock support exists; mission effects do not create a complete native effect graph. Host gunnery/wake/death/effect events need exactly-once spawn/retire mapping, shared random/clock policy and native owner lifetimes. A simulation event or pool allocation is not a visible particle. |
| native world `game+19CC`, `4BCh`, and scene-load aggregate | `004CB030` and `009037F0`; aggregate `004DE610` reconstructed through typed host contracts | Mission `construct_world` mainly records construction and projects the active byte; units already own many actual typed lists. Do not replay initialization into a second simulation world to obtain graphics. Bind render associations to existing entities; raw world adoption is a separate ownership decision. |

The historical construction document calls `19E8` the ocean owner and `19F0` sky.
Current constructor bodies give a more useful distinction: `00BBDFF0` constructs
atmosphere/sky children; `0078DAA0` constructs ocean environment/field/spectrum.
The slots and addresses are firmer evidence than those older labels.

Additional references: [world construction](GAME_WORLD_CONSTRUCT.md),
[world/host list distinctions](CONSTRUCT_WORLD.md),
[renderer resource construction](../include/bsp/native_render_resources_construction.hpp),
[model owner](../include/bsp/native_model_owner.hpp),
[camera owner](../include/bsp/native_camera_owner.hpp),
[scene-root storage](../include/bsp/native_gui_scene_storage.hpp),
[material owner](../include/bsp/native_material_owner.hpp),
[particle model construction](../include/bsp/native_particle_model_construction.hpp).

## Proposed bring-up and packet estimate

Recommend **one installed ship model with a fixed camera**, on a diagnostic scene
using the ordinary command path, as the first visible milestone. It exercises
real asset loading, transforms, material/shader constants, GPU buffers and draw
submission without requiring main-scene post processing, ocean reflection or
terrain. Keep its diagnostic scope explicit; it is not mission-render parity.
`d3d9_mesh_probe.cpp` is useful prior art for installed mesh/material/GPU checks,
but its controlled fixture does not prove the `bsp_game` mission path.

The estimates below are integration packets with explicit owner/producer and
validation contracts, not counts of unreconstructed functions. They assume the
existing typed/raw routines can be composed; a newly discovered unsupported
profile or asset loader adds a bounded dependency packet.

| Step | Packets | Work and files | Exit evidence |
| --- | --- | --- | --- |
| A. Establish resource, lifetime and frame ownership | 2 | `game_native_renderer_application.cpp`, `game_native_renderer_resources.inc`, `game_native_renderer_frame.inc`, `game_hosts.cpp`, relevant headers: compose the existing `00B14A10` resource/service graph and one retained nonempty ordinary-command execution domain, continuation frames, cleanup and device-generation handling | Live renderer/definitions/service/queue/cache recorded **before** mission render; one Begin/End/Present per frame; empty and nonempty teardown complete |
| B. Fixed camera and scene | 1 | Renderer application/camera host; native scene/camera owner APIs; diagnostic selection in `game_main.cpp`/options | Construct actual root/Operator/viewport/fog, attach one node, verify matrices/clip/viewport and balanced owner retirement |
| C. Installed model, material and ordinary draw | 2 | Renderer/model loading host, canonical geometry/material/cache providers; reuse existing native loaders and `native_render_*` APIs | First milestone: recognizable textured ship, native queue count/entries and `00B24010` or `00B21B40` reached; fixed-frame screenshots retained |
| D. Attach render associations to mission entities | 1 | `game_hosts_mission_frame.cpp`, `game_hosts_scene_contents.cpp`, `game_hosts_units.cpp/.hpp`, a small dedicated renderer adapter | Existing simulation owns state; entity-ID associations survive spawn/despawn; camera/transform projection once after interpolation; ON/OFF simulation comparison |
| E. Genuine main-scene command | 2 | `game_native_renderer_application.cpp`, main-scene continuation/queue/lifetime files, render frame adapter in renderer hosts | Reuse A's service; finish raw `00B156C0` tail/composition and `00B17240`; admit `D5E13C` only with execution and cleanup; DRSH/SCNE/POST captures and correct restoration |
| F. Atmosphere, sky and ocean | 2 | Dedicated environment/ocean render adapter, existing `water_weather_cc13`/`cc13_sweepB` and ocean domains | Shared simulation clock/sea level, fixed-camera horizon/sky/ocean, reflection/clip and water interaction captures |
| G. Terrain and foliage | 3 | Landscape/terrain asset and render adapters, `terrain_particles_cc13`, culling/upload providers | One known tile then island, collision/render height agreement, camera movement without missing/repeated tiles |
| H. Dynamic models and particles | 2 | Units/model association adapter, native particle application/providers, effects/event hosts | Moving ship and damage/remap; wakes/smoke/muzzle effects appear once and retire; transparent ordering and owner counts checked |
| I. Original capture alignment and visual regression | 2 | Harness/capture plumbing in `game_main.cpp`, `game_hosts.cpp`, tools; separate original-game observation/capture adapter | Original/reconstruction pairs at matching mission frames, settings and camera; persistent manifests and visual review |

Planning total: **17 packets**, with a reasonable discovery range of **14–22**.
First visible diagnostic model: **5 packets** (A–C). A model driven by existing
mission entities adds D. Start the original-capture packet alongside A so reference
alignment is available when C finishes. These estimates are not elapsed-time or
game-completeness guarantees. Ocean-plus-sky is a later milestone because its
environment, water geometry and target dependencies make it larger than one model.

## Validation and deterministic screenshots

For each step run the required Win32 `./scripts/build.ps1` and relevant existing
checks. Add a test only for a concrete uncovered behavioral risk. Record source
commit, executable/content hashes, device generation, asset and shader names,
queue/batch/entry counts, draw-call counts, owner references and failure boundary.
Construction/build/fixture success is separate from live draw, visual and ABI proof.

Use `pair_export.py`/`pair_diff.py` for an ON/OFF same-tree simulation comparison.
`pair_diff` compares logs, gameplay rows and summaries; it does **not** compare
pixels or establish similarity to the original game. Rendering must not perturb
simulation clocks, RNG streams, entity identities or input scheduling. Classify
expected new native-call/summary rows separately from moved gameplay rows.

Proposed capture contract:

1. Pin a mission, installed ship/skin, resolution, graphics settings, camera pose,
   projection, input/order schedule and time policy. Initially use fixed camera,
   `--mission-frame-seconds 0.05` and no jitter. Capture mission-frame ordinals
   **1, 60 and 180**, recording the actual simulation clock beside each image.
   Freeze weather/time-of-day only through an explicitly recorded diagnostic
   configuration shared by the two references, not through fabricated renderer data.
2. The reconstruction already supports `--screenshot` and
   `--screenshot-mission-frame N`. Run one capture per process, since the callback
   is one-shot. Example after bring-up, from the relevant checkout:

   ```powershell
   ./tools/run_game.ps1 -Log local/render-N060.log -- --frames 600 --press-start-frame 30 --menu-select USN02 --mission-frames 181 --mission-frame-seconds 0.05 --screenshot local/render-N060.png --screenshot-mission-frame 60
   ```

   Retain the launcher's manifest/input copies and the screenshot. Verify the log
   actually reaches the requested mission frame; a load screen or a terminated
   mission is not a frame match. Match the original's asset and view for the model
   milestone rather than comparing a diagnostic ship against an unrelated view.
3. **Original-game fixed-frame capture is additional work.** `run_game.ps1` is a
   `bsp_game` launcher, not an original-game deterministic driver. Arrange an
   external observer/capture adapter at the original mission-frame boundary
   `004E4A40`/render `004CA440` and the corresponding presented frame. Replay the
   same input/timing/camera contract and label captures by observed mission ordinal
   and clock. Keep installed files read-only. If original fixed stepping cannot be
   established, report timing uncertainty and use matched pose/time observations;
   do not claim equal frame numbers from a wall-clock delay.
4. Current reconstruction capture occurs after the sprite overlay and **before**
   native EndFrame. EndFrame/debug/XLive work can still alter presented pixels.
   Either capture both games at that same pre-EndFrame stage or add a capture
   point after final draw work and before Present for both. Record whether HUD is
   included; retain a world-only crop plus the full frame so HUD pixels cannot
   conceal a missing world.
5. Compare silhouette, projection/horizon, depth/occlusion, materials/textures,
   light/fog, water/terrain placement and transparent effects. Retain a side-by-side
   and difference image; use pixel metrics with stated tolerances for water,
   particles and temporal effects. A non-clear image or a draw-call count alone
   is insufficient. Repeat the fixed-camera capture, then move the camera and
   check clipping/LOD, resize/reset and return-to-menu cleanup as those features
   are enabled.

No game launches are needed for this survey. Future validation follows the
machine rules: check `~/.bsp/DRAIN` before every launch; no detached runners that
bypass it, no `--hardware-probe-commit`, and hold launches after exclusive-fullscreen
`0x88760868` failures. These are invalid starts, not failed world-render evidence.

## Ownership, unresolved work and coordinator decisions

Lease snapshot: the survey owns only `docs/RENDER_PATH_SURVEY.md`. The checked
leases include cc9's `game_hosts_units.hpp` and `game_hosts_script_orders.cpp`,
cc13's `00996120–009F6060` band and separate scene/weather fixes, cc15's telemetry
and segment-48 work, and cc15's `cmake/startup.cmake` lease/queue. A pending cc9
queue also names `game_hosts_units.cpp`. Renderer application/frame files and
`game_main.cpp` were not leased in this snapshot. Recheck before implementation;
request shared host files through `lease queue`, and keep cc9 responsible for
simulation-side state contracts. Do not treat an unleased file as permanently free.

No reconstruction addresses were skipped for contention: this packet reads only.
Unresolved implementation boundaries are specifically `004CA440`'s mission
adapter, `00B14A10` startup composition, `D5E13C/00B17240` queue admission and
`00B156C0`'s raw continuations after `00B1645B`/`00B164B8`, plus live native
mission objects and landscape/ocean/particle producer chains described above.
The survey does not certify every virtual target reachable from arbitrary assets.

The coordinator/user must choose whether to approve A–C as an isolated fixed-camera
model milestone, then D–I, and assign ownership of the shared mission/units boundary
and original capture adapter. **Do not start bring-up code before approval.**
No CRT/STL/zlib/Lua/D3D9/D3DX body is proposed for reconstruction: use their existing
bindings. Compiler EH handlers (for example `004CA440`'s `00C6557E`) are not credited
as new game functions, and native exception/ABI equivalence remains separate.
