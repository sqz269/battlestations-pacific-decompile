# Milestone D: host scene registry run on the actual lighting resource

Worker cc16-mdcrash, 2026-10-28. Diagnosis only; no code change accompanies this note.
Scope is the milestone-D native render environment
(`BSP_NATIVE_MISSION_SHIP=1 BSP_NATIVE_MISSION_FRAME=1 BSP_NATIVE_MISSION_UNITS=1`).

## Symptom

USN04 in that environment faults or hangs right after late-spawned Japanese planes load
(units 27-32, B5N Kate / A6M Zero, devices `japan_plane_mg.mmod`). The log shows two
`native mission diagnostic camera fit` lines, then one of:

- `harness crash: exception c0000005 at 10406637 reading b4c2b7cf ... mission_frame 515`
  (cc15 artifact `kept_mdcrash\gui4_usn04_a1base_a1msd2.*`), or
- a main-thread spin in a corrupted `std::unordered_map` walk (`_Find_last`). The log
  stops and the process burns CPU indefinitely. Both of my runs ended this way.

## Faulting function and chain

The Release build of 9bc29d0ac matches the crashing executable byte for byte over a
64-byte window at RVA 0x406637. The linker map (`build/win32/bsp_game.map`) names
it `bsp::add_scene_node_if_type_00b83d50` +0x97, with `SceneNodeRegistry::insert_00b83700`
inlined. The faulting instruction is the bucket walk at `src/scene_attachment.cpp:157-158`
(`position = position->previous; ... position->key`).

Main-thread stack, from live `cdbX86 -pv` samples of both runs, symbolized with the map:

```
_Hash::_Find_last / emplace          SceneNodeRegistry::bindings_ (scene_attachment.cpp:168)
add_scene_node_if_type_00b83d50      scene_attachment.cpp:376-381
set_node_scene_00b6ed80              scene_attachment.cpp:388-401
upload_native_instance_groups_00b1e990  native_instance_group_upload.cpp:137-141
submit_native_scene_00b1f4d0         native_render_scene_submission.cpp:198
MissionCommandGraph::submit_scene / MissionShipGraph::submit_scene
FrameHost::dispatch_frame_jobs -> NativeMissionFrameCoordinator::run
GameNativeRendererApplication::run_mission_render_frame -> GameDeviceHost::clear_and_present
```

## Evidence: the "SceneResource" is the actual native lighting resource

Minidump registers at the fault are `ebx=1f0169f0` (the object whose `+4` list owner and
`+10` bucket array the code reads), `eax=1f81dfd8` (loaded from `[ebx+10]`), and
`edi=b4c2b7c7` (garbage `previous` link).

The same log contains:

```
native mission lighting ready: outer=373C2BD0 resource=1F0169F0 ambient=1F81DFD8 sun=37930008 ...
native mission ship ready: ... scene=373C2BD0 ...
```

- `1F0169F0` is the 0x3C raw allocation that `game_native_mission_lighting.inc:166-196`
  builds with `construct_native_scene_resource_00b83c50` and publishes at outer `+0x1C`
  through `set_native_gui_scene_resource_00b723f0`. It is actual native storage.
- `upload_native_instance_groups_00b1e990` reads that slot with
  `native_scene_lighting_owner_00b72110` and casts it to a host `SceneResource*`
  (`native_instance_group_upload.cpp:116-118,137`). The host `SceneResource`
  (`include/bsp/scene_attachment.hpp:74-82`) is explicitly "new C++ storage, not a
  native object overlay": `references` at +0, `SceneNodeRegistry` at +4, buckets at +0x10.
- The host code therefore takes the actual resource's `+0x10` (the **ambient** pointer,
  `1F81DFD8`) as its bucket array, and follows ambient-object words as list links.
- `set_node_scene_00b6ed80` also does `requested->references.fetch_add(1)`
  (`scene_attachment.cpp:395`) on host `+0`, which on the actual resource is its native
  table word, not its `+4` count.

Milestone D already guards against exactly this on the root path.
`game_native_mission_scene.inc:24-38` admits only `B6ED80` with `requested==0` and
model `+0x170==nullptr`, with the comment "never ... interpret a raw3Ch resource as a
logical SceneResource". The instance-group upload has no equivalent guard.

The native code at B1E9CB..B1E9E5 is `model->vtbl[0x50](scene->[+1C], 0)`, and
`00B72110` is `MOV EAX,[ECX+1C]; RET`. In the original image, therefore, the actual
`B6ED80` runs on the actual model node. The reconstruction instead routes that call into
the host companion runtime (`a.scenes`, wired in `game_native_mission_command.inc:216`).

## Why models pass the light gate (B83D50 bytes)

Native `B83D50`:

```
00b83d5a MOV EDI,[ESI]          ; node table
00b83d5e CALL 00b7aa70          ; 00b7aa70: MOV EAX,[0109018c]; RET  (light type id)
00b83d63 PUSH EAX
00b83d64 MOV EAX,[EDI+0c]
00b83d69 CALL EAX               ; node->is_type(light id)
00b83d6d JZ  00b83d85           ; not a light: no registry insert
00b83d79 LEA ECX,[EBX+14]
00b83d80 CALL 00b83700          ; insert into resource+14 registry
```

The model predicates compare that token against the model's own type cells:
`006EF860` scans 01090034..01090040, and `00B6F570` scans 0108FF90..0108FF98.
Given the real light id, a model node fails the test and the native inserts nothing.

The host replacement passes `get_scene_registry_type_00b7aa70(runtime)` =
`runtime.registry_type_token` (`scene_attachment.cpp:368-370`). The mission graph's
`ActualSceneRuntime` is built with `SceneAttachmentRuntime(0,{0,0,0})`
(`game_native_renderer_lifetime.inc:11`), so the token is **0**, not `[0109018C]`.
Both runs reached `insert_00b83700`, so the model predicate accepted token 0.
**Inferred, not proven:** one of the model/node type cells it compares
(`ModelTypeBootstrap` own/node/root, or `LightTypeBootstrap` node own/root) holds 0
at that point. Reading those cells in a live run would confirm it.
Even with a correct token, the `references.fetch_add` above still writes the actual
resource's table word. Fixing the token alone is therefore not sufficient.

## When it is reached, and why milestone D missed it

`submit_native_scene_00b1f4d0` calls the upload for every command. The upload returns at
once when the command has no instance groups (`native_instance_group_upload.cpp:128`).
The faulting insert needs an instance-group model whose host `binding.scene` differs from
the published lighting resource. Mission roots are attached with a null scene
(`game_native_mission_scene.inc:38`; lighting is bound after roots, and later roots
are attached inside `begin_root_attachment/end_root_attachment`). In USN04 the first such
upload follows the late plane and device loads near mission frame 515.
**Inferred:** I did not log the upload's first non-empty call.

The milestone-D validation runs (`docs/OCT24_RENDER_MILESTONE_D.md`) used 181 mission
frames and screenshots at frames 1/60/180. That ends before the late spawns, so the path
was never executed.

## Commits

No bisect was needed. `git diff c3f1e2132 9bc29d0ac` is empty for every file involved
(`scene_attachment.*`, `native_instance_group_upload.*`, `game_native_mission_*`,
`light_scene_retention.cpp`). A rebuilt c3f1e2132 run hangs in the identical chain
at the same spawn point. The bug is a collision of three earlier commits:

- de6fdea8c (2026-10-18) "Compose retained ordinary mission command execution": wires the
  upload to the host runtime `g.scenes`.
- 6c646a77b (2026-10-18) "Compose native ambient and directional lighting for the installed
  ship": publishes an actual 3Ch lighting resource at outer `+0x1C`.
- f85a98af7 (2026-10-24, milestone D) "Compose all mission unit models ...": adds late
  unit/device model loading (`refresh_units`, `load_new_unit_models`), which first
  makes an instance-group upload meet that resource.

8eded9e50 (`kApplicationCc15Bound` flip) is not an ancestor of c3f1e2132. It is not the
cause of this fault. The 500-frame preload-retirement failure at 737F30 is a separate issue.

Runs: two, both USN04 with 700 mission frames in the milestone-D environment.
Logs are in this worktree's `local/cc16md_repro_9bc.log` and `local/cc16md_c3f.log`.

## Fix designs

Default-env reachability, for all designs: the only constructor of the upload access is
the mission command graph (`game_native_mission_command.inc:216`). That graph exists only
on the `BSP_NATIVE_MISSION_SHIP=1` branch. The native GUI render application also refuses
to start without `BSP_NATIVE_MISSION_SHIP=1 BSP_NATIVE_MISSION_FRAME=1`
(`game_native_gui_render_application.cpp:733-735`). A default run (no native env vars)
does not execute this path. Design (a) still edits shared `bsp_core` files, so it
needs a default-env pair_diff. Design (b) can be argued unreachable by construction.

Leases are as of 2026-10-28 (`bsp.py lease list`). `agent/cc15-gui4` (packet `cc15-gui4-a1`,
until 2026-10-29T08:54) holds `game_native_mission_lighting.inc`,
`game_native_mission_scene.inc`, `game_native_mission_ship.inc`,
`game_native_mission_resources.inc`, `game_native_renderer_application.{cpp,hpp}` and
`game_native_renderer_gui_scene.inc`. No other active lease covers the files below.

### (a) Faithful: actual-storage B6ED80 for instance-group models

Make the upload call the actual-storage `set_native_node_scene_storage_00b6ed80`
(`native_node_scene_attachment.hpp`) on `attached_model` with the actual lighting resource
and recursion 0. That writes node+170, takes the actual `+4` credit, and runs the actual
`B83D50` gate with the real light id. In addition:

- the model and matrix-indexed model companions must treat node+170 / the actual resource
  as the authoritative scene. Today their `binding.scene` holds host `SceneResource*`
  values, and their remove and destruction paths call host `remove_native_node_scene_00b6ee10`
  and `remove_scene_node_if_type_00b83ec0`. Those paths must learn to release an actual
  resource (`00B6EE10` actual storage), or the credit leaks and shutdown retirement fails.
- the upload access must carry a `NativeNodeSceneAttachmentContext` (gates, the canonical
  `NativeRenderActualOwnerRegistry`, `00CE221C/00CE2220` increments, child dispatch)
  with persistent frames.
- lighting retirement (`retire_before_native_drain`, which today expects resource count 2)
  must account for one more credit per instanced model, or release those credits first.
- the header warning in `native_node_scene_attachment.hpp` ("Do NOT install these
  identities into legacy logical node/light companions") has to be resolved deliberately.

Files: `src/native_instance_group_upload.cpp`, `include/bsp/native_instance_group_upload.hpp`,
`src/game_native_mission_command.inc`, `src/native_model_owner.cpp`,
`src/native_matrix_indexed_model_owner.cpp` (319/325/482), `src/generated_model_lifetime.cpp`
(245), possibly `src/scene_attachment.cpp`, plus `src/game_native_mission_lighting.inc`
(retirement credit count; **leased by cc15-gui4**).
Cost: large. This is a cross-cutting ownership change in shared `bsp_core`, comparable to a
milestone. Risk: high. It touches model destruction and shutdown residue for every native
model, and companions used by the GUI path. It needs a default-env pair_diff and
milestone-D runs past the spawns with shutdown. Benefit: models get their real lighting
association (node+170), as in the original.

### (b) Minimal containment behind a divergence switch

Add an explicit OFF-by-default-for-fidelity divergence switch, for example
`kMissionUploadKeepsHostNodesOffActualLighting`, ON for the milestone-D graph.
`NativeInstanceGroupUploadAccess` gains a borrowed "actual lighting identity" pointer.
The mission command graph supplies the published resource, read through the lighting
owner or the outer `+0x1C` at graph build. When the switch is ON and `scene` equals that
identity, the upload skips the host `set_node_scene_00b6ed80` call. It still requires the
`0x00B6ED80` table check and the `resolve_key` binding, so a missing binding still fails.
All later steps (sort, lock, write instances) are unchanged.

What it skips: the per-frame `B6ED80(model, lighting, 0)` for instance-group models. Their
node+170 stays null. Every other milestone-D model already has node+170 null by design
(roots go in with lighting detached; see the `retire_before_native_drain` comment
"model+170 remained null throughout this diagnostic"). The visible effect is therefore
the existing diagnostic lighting state for the new planes and devices, not a new
regression against the current milestone-D look. Against retail, instanced models miss
whatever node+170 lighting association the original uses; that effect is not measured
here. No light registry entry is added in either case, because the native gate rejects
models.

Files: `src/native_instance_group_upload.cpp`, `include/bsp/native_instance_group_upload.hpp`,
`src/game_native_mission_command.inc` (to supply the identity). If the identity must come
from the lighting graph object, that also touches `src/game_native_mission_lighting.inc`,
leased by cc15-gui4. Reading outer `+0x1C` in the command graph avoids that lease.
Cost: small (tens of lines). Risk: low. The switch is reachable only in the
milestone-D graph, and default env cannot reach it. A build with the switch flipped
must compile in both settings. Verification: USN04 in the milestone-D env past frame 1500
with a screenshot, plus a default-env pair_diff for form.

### (c) Middle option: host companion keyed by the actual identity

Register a host `SceneResource` companion for the actual lighting resource, the way
`NativeRenderActualOwnerRegistry` maps actual identities to companions. The upload then
resolves `+0x1C` through that registry and terminates if no companion exists. The
mission runtime's token must also be set to the real light id (`[0109018C]` after
`initialize_light_00cd80a0`), so that models fail the gate as in the native. This
removes the type confusion everywhere the host runtime meets an actual resource.
node+170 still stays null, so the visual result equals (b).
Files: (b)'s set plus `src/game_native_renderer_lifetime.inc` (token) and the companion
owner, likely `src/game_native_mission_lighting.inc` (leased by cc15-gui4).
Cost: medium. Risk: medium. The token change affects every host `add/remove_scene_node_if_type`
in the mission graph, including the sun's own registration path.

Recommendation: (b) now, to unblock long milestone-D runs; (a) as a planned packet if
retail lighting parity for instanced models becomes a goal.
