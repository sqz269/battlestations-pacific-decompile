# Screen provider readiness58 and bounded directional proposal60

The next missing Source method is **raw-name construction/admission/retirement
of the existing typed directional-light owner graph**, not a new light body.
The normal renderer already supplies the canonical raw-name node/camera domain;
the current `GuiScreenSceneRuntime` provider contract cannot yet use that domain
because its directional factory/destruction and its own temporary strings
require semantic names. Close only the typed directional lifetime in packet60.
Leave Screen strings, stores, manager/page producers and activation separate.

Exact Root `f9dd0d899d2fd6082adfee779f5a843420574847` was merged successfully
before analysis. This packet owns only this document and its JSON report. BSP
project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was
verified for the read-only Original analysis batches; all Original code reads
used `bsp.py show`, with assembly for x87/width/cleanup/callback questions.

## Method-level findings

| `GuiScreenSceneNativeCalls` method | Existing faithful Source behavior and exact remaining gap |
|---|---|
| `allocate_directional_light_00b7c6b0` | Raw B7C4C0/B7C6B0 chain is complete in `native_directional_light_construction.cpp:45`. Typed `DirectionalLightOwner` and canonical `DirectionalLightReference` also exist, but `directional_light_reference.cpp:130` selects semantic names; `directional_light_owner.cpp:49` finishes its node through semantic-only destruction. A fresh raw light cannot honestly be published through this graph yet. |
| `root_radius_00ac5f00` | `gui_screen_root_radius_00ac5f00` in `gui_group_bounds.cpp:66` already preserves actual D7A308 double -> BF7030 x87 sqrt -> float32 spill/reload -> BF7420 -> signed EAX/CVTSI2SS. This is an AC59A0 subexpression, not a separate Original function. Needs genuine `GuiGroupBoundsCrtAccess`, including the current DWORD conversion selector. Do not replace it with sqrt2, a constant radius, a cast or a bool pointer alias. |
| `set_root_bounds_00b8e6c0` | Both live `NativeGroupOwner` and actual raw Group storage overloads already exist (`gui_group_bounds.cpp:54,63`). Original clears flags138 mask30 and byte175 before four forward FLD/FSTP pairs into08..17. Resolve the SAME canonical group; a model tail or `SystemLightingWords4` memcpy substitute is invalid. No new body needed. |
| `register_page_00aa52a0` | Complete raw manager/page registration exists in `native_gui_page_registry.cpp`: remove the first same pointer, then insert before first greater signed +FC priority; preserve equal-priority arrival order. It needs actual88h AA5D70 manager and actual124h page. `GuiWidgetOwner`/`GuiScreenLayerState` are not those allocations. Normal frontend `ManagerState` and `GuiPageRegistry` are Source projections, not raw handles. Raw AC6600 page construction still exposes a partial prefix/continuation; do not cast or publish that as a completed page. |
| optional F8BF4C visibility | Original AC4450 reads F8BF4C at AC44A0 and again at AC44BE; only a changed visibility with installed hook calls current virtual7C, then passes its returned EAX in ECX and visible byte in DL. Xrefs show real WarningManager writes at009874F6 and00989C0C. No normal live hook-cell/dispatch binding was found. Optional does not establish null or permit a fabricated absent-hook value. |

Radius assembly at AC5F00/1B/20/24/28/34 proves the exact mixed x87/SSE
sequence; the actual root+4C is reloaded after the CRT calls at AC5F2D. Bounds
body is `[00B8E6C0,00B8E6EC)`, ECX group, stack sphere, RET4. Registration is
ECX actual manager, stack actual page, RET4. The existing Source statuses are
build/fixture qualifications under supplied domains, not normal native Screen,
native Text drawing or gameplay proof.

## Exact common-domain availability

`renderer.camera_environment()` returns `CameraGraph::environment`
(`game_native_renderer_application.cpp:563`), whose `nodes` is the SAME
`DestructionGraph::nodes`. That graph binds the application's raw VFS strings
(`game_native_renderer_lifetime.inc:65`) and the SAME scenes/attachments. Its
`LightTypeBootstrap` borrows `vfs.native_owners().types()` and
`vfs.native_types().light_types()`; camera types use the same counter/backing
(`game_native_renderer_camera.inc:36`). It already borrows the canonical
`game_native_resource_pool_process().camera_pool_0108ffb0()`, viewport registry,
renderer publication, actual profiles and `application_camera_axes_crt()`.
`renderer.node_constants()` exposes the actual one/positive/negative cells.
Do not make another node/scene runtime, camera pool, type counter or raw pool.

That is identity availability, not complete Screen lighting readiness.
`ActualSceneRuntime` starts `registry_type_token` at0 (`lifetime.inc:11`) and
only later fills object tokens as `{0,node,root}` (`:96`). No normal game-owner
directional/light bootstrap or publication of the actual cLight registry token
was found. Zero must not become an invented cLight ID. The existing common
backing can be initialized/published deliberately in a later host packet.

Scoped game-owner searches found no normal `GuiCameraStoreMap`,
`NativeGuiSceneEnvironment`/`NativeGuiSceneWeakBase`, directional pool/static
CD8080 owner or concrete `GuiScreenSceneNativeCalls`. A real weak pool process
already exists (`game_native_weak_pool.hpp`: CD8A60, pool0109CE94); that is a
reusable prerequisite, not an existing outer-scene weak-base adapter. Existing
`GuiCameraStoreMap` owner APIs and actual raw manager/store APIs also exist;
they are distinct representations and must not become two authoritative maps.
Normal frontend's `gui_manager_004c12b0` returns its small `ManagerState`
(`game_hosts_frontend.cpp:475`), not the raw88h manager accepted by AA52A0.

`GuiScreenSceneRuntime::Shared` explicitly rejects raw names at
`gui_screen_scene_runtime.cpp:47`; `ScreenString` and scene/camera/GuiLights/
directional temporary paths repeat semantic admission (`:16,100..134`). Scene
name ownership already has raw support, and camera owner raw construction/
destruction exists, but `allocate_native_gui_camera` still chooses its semantic
overload (`gui_camera_store_owner.cpp:145`). These are later caller/frame
bridges, not permission to install a parallel semantic name domain. Packet60
must not expand into them.

## One proposed Source packet60

Own `include/bsp/directional_light_owner.hpp`, `src/directional_light_owner.cpp`,
`include/bsp/directional_light_reference.hpp`, `src/directional_light_reference.cpp`,
plus unique `CC12_NATIVE_GUI_RAW_DIRECTIONAL_LIFETIME_60` doc/report/ignored
evidence. Claim/check the existing related addresses before edits:

| Address range | Original ABI / reuse |
|---|---|
| B7C4C0..B7C576 |182B base constructor, ECX actual1F0 slot, stack raw8h header, EAX same, RET4. Existing raw kernel; no duplicate body. |
| B7C6B0..B7C6C9 |25B directional constructor, same ABI; call base atB7C6B8 then stampD62FB0. |
| B7C5B0..B7C6AA |250B light destructor, ECX actual owner, RET; signed scene drain, shadow release, array cleanup, then full node destructor atB7C693 and RETB7C6A9. |
| B7C820..B7C846 |38B deleting destructor, ECX owner, stack flags, EAX original address, RET4; stamp profile, destroy, then bit0 returns same slot through B7B2F0. |

Pool B7BD40/B7BAC0, B7B610/B7B2F0, CD8080/CE0EB0, B6F5A0 and raw node
destruction are read-only reuse; no pool/process/CMake/Screen/renderer/VFS
owner edits in60. The pool and all lifetime/provider environments remain
required real caller inputs. A normal process owner/adoption is a later packet.

The Source work is narrow:

1. Add an explicit raw-header construction/admission route using the SAME
   `nodes.require_raw_name_pool()`, `renderer.node_constants()` references and
   caller-borrowed actual `CE7820` cell. Forward the header unchanged. B7C4C0
   reads current one only after B6F5A0 and CE7820 only after three1C4 zero
   stores; do not substitute copied constants or defaults. Preserve
   constructor-unwritten194..1A3/1B4..1C3/1DC..1EB and pool1EC DWORD. Materialize
   the existing typed tail by preserving/restoring its actual bytes, not by
   reconstructing a second light. The node prefix/atomic already come from the
   existing raw node constructor.
2. Select the existing raw node destructor in `finish_node_base` for a raw
   runtime; preserve the complete current typed light body and semantic path.
   Use direct throwing raw cleanup, not a noexcept name adapter. Existing
   terminal canonical callbacks remain their explicit noexcept boundary.
3. Provide a caller-retained Source operation frame before pool/getter entry.
   Preflight domain/profiles/constants/predicates first. Track the acquired
   actual slot, constructor entry/return, typed-tail/owner admission, scene
   binding and canonical-reference publication persistently. Prefix failure
   is already cleaned once by B6F5A0; do not repeat it. The existing semantic
   factory's local rollback is not evidence for a raw getter/metadata failure.
   Do not retry, return a live/partial slot, or lose the only owner in a local
   destructor. Preserve interrupted frames and expose metadata-only retention
   status for the future host to terminate before any manager/CRT drain.
4. Publish exactly one `DirectionalLightReference` only after the SAME scene
   and attachment bindings exist; borrow actual+4 without a new retain/count.
   After successful handoff use the existing terminal companion disposal and
   pool identity. Snapshot/clear ownership before any potentially retiring
   callback and do not read a retired record. The caller's actual name/header
   cleanup remains its enclosing persistent frame's obligation.

There is already a separate complete raw-storage light lifetime and
`NativeDirectionalLightStorageReference` in `native_light_lifetime.hpp/cpp`.
Its actual raw-scene retirement architecture explicitly does not convert to
`DirectionalLightOwner`/`SceneAttachmentRuntime`. Do not rewrite that body,
create another registry/tree, or install both companions for one light. The
proposed packet closes only the existing typed graph required by the current
`GuiScreenDirectionalAllocation` interface, matching normal renderer nodes.

Validation proposed for60: existing canonical Win32 build/checks, unchanged
kernel/source evidence, and one focused actual raw string-pool/directional
pool/type/runtime probe for same count/bindings/name, delayed retained release,
final slot return and normal interruption=false. Review entered failures and
all whole changed functions; no broad suite, runtime activation, Original
FH3/private ABI or full Screen graph claim. Approval and leases precede Source
edits. This58 receipt itself adds no build/probe or Ghidra mutation.
