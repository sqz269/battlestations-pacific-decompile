# cc15-gui4 A1: the startup raw scene domain

Switch: `kNativeRendererStartupCc15Bound` (`include/bsp/game_native_renderer_application.hpp`), ON candidate.
Status: source-reviewed and baseline-gated at `634258460`; publication remains pending the
latest-common-code USN21 serial gate. This activates the raw domain, without native GUI page drawing.

## What A1 builds

At the host's 0073DD12 position, after the shadow target at 0073DCCB, ON opens one
`StartupSceneDomain` (`src/game_native_renderer_gui_scene.inc`) inside the renderer Impl. The domain
contains:

- the registry and gate recipe that used to be private to the mission lighting graph;
- the `NativeGuiSceneEnvironment`;
- the raw node, camera, viewport, look-at, light, directional and fog contexts;
- the dispatch providers that AC59A0's `require_context` admits:
  - pose: B71400, B6DAE0, B71460, B6E870;
  - world: B6DBE0, B8E6B0, B6DBC0;
  - tree +18: B6F310;
  - tree +0: BD30E0, only for a zero-count, scene-null owner;
  - scene +50: B6ED80 for raw cameras and B7C020 for directionals, plus one delegate;
  - renderer +30: B1FF60.

  Any other target throws with its address.

The domain also runs the image's CRT initializers for these types:

- CD80A0 and CD80F0, the light and directional type descriptors;
- CD8080, the directional pool 01090154, plus its atexit destroy CE0EB0.

One startup line reports the domain's reservation size (107376 bytes, 256 frame slots per
provider). The opt-in mission ship graph (`BSP_NATIVE_MISSION_SHIP`) borrows the domain when the
switch is ON. Its lighting graph uses the startup registry recipe and skips the three initializers;
its resources graph borrows the startup scene environment. The lighting graph's original binding
wrappers still construct but remain unused; those constructors do not register native objects.
The command, resources and preload graphs stay mission-only.

`startup_scene_domain()` and `bind_startup_scene_child_delegate()` are the accessors for the GUI
page work (P3a2).

## Singletons, ON vs OFF

| Object | OFF | ON | Teardown |
|---|---|---|---|
| Light type descriptor 0109018C (CD80A0) | initialized in mission lighting `create_and_bind`, after model roots, msd env only | initialized first in the startup domain's registry initializer at 0073DD12 | static descriptor, no native destructor; the same 0109010D guard prevents a second type id |
| Directional type descriptor 0109019C (CD80F0) | immediately after CD80A0 in mission lighting | immediately after CD80A0 at startup | static descriptor, no native destructor; the same 0109010E guard prevents a second type id |
| Directional pool 01090154 (CD8080) | after both type initializers in mission lighting | during startup directional-context construction, after registry, scene, node and camera bindings | `initialize_directional_light_once_00cd8080` guards initialization and the CE0EB0 atexit registration; CE0EB0 destroys the pool once at process exit, after the host's native drain |
| Registry allocation, mutation, registration and gate bindings | mission lighting constructor | startup registry constructor, before scene/node/camera bindings; mission-local wrappers remain unused | wrappers hold references and callbacks, do not register anything at construction, and perform no native retirement; mission wrappers die before the startup wrappers |
| Ambient/resource storage and identity bindings, scene-field bindings | same mission lighting constructor, after gate bindings | same startup registry constructor, after gate bindings; mission lighting selects the borrowed bindings | native resource/sun owners retain their existing retirement and drain; metadata does not release them again |
| `NativeGuiSceneEnvironment` | mission resources constructor; this patch stores the metadata in `owned_scenes` | startup constructor, after the registry; `owned_scenes` stays null and mission resources borrow it | pure references, no native registration/destructor; mission resources close and destruct before the startup environment |
| Raw node/tree/scene-child, viewport, look-at, camera, light, directional and fog contexts/providers | no startup instance; existing mission camera/light contexts remain mission-owned | startup bindings after the registry/environment, all sharing the existing owners and string domain | metadata drops with the startup domain, after `mission_ship`; it performs no second native cleanup |
| Actual-owner registry, raw strings and weak-owner domain | existing renderer/host providers | same providers, borrowed by the startup domain and mission | their lifetimes are unchanged; `Impl::startup_domain` follows its providers and precedes `mission_ship`, preserving reverse destruction order |
| Shader cache 0108D6EC, GUI Text resources | existing mission-only preparation | unchanged by A1 | existing owners; the separate shutdown ordering repair below closes the late-model bracket before Application rows |

This is a table of host binding construction, not an assertion of image CRT initializer order.
The concrete guards are in `src/light_type_bootstrap.cpp:98` and
`src/game_native_resource_pools.cpp:207`; the declaration order is in
`src/game_native_renderer_application.cpp` (`startup_domain` before `mission_ship`).

## Decision: no startup shader preload in A1

The image runs 0073BF80 at 0073DD12, inside 0073D410:

1. B3A600 opens the binary shader cache.
2. `shaderfx/shaderpreload.lua` is loaded, and each effect is appended to Application+08.
3. B3B140 closes the cache.

Fonts and GUI (0073E13C) then find warm effect-registry hits with the cache closed
(`docs/NATIVE_POST_CACHE_ORDER_CC12.md`).

The host version of 0073BF80 (`MissionShipShaderPreloadGraph`, `src/game_native_mission_preload.inc`)
needs these inputs:

- the shader cache context;
- the effect registry. It is reached through a GUI sampler and effect facade
  (`GameNativeGuiSamplerApplication` plus `GameNativeGuiEffectApplication`). The sampler facade
  borrows the `GameInputRuntime`, which the host only builds at 0073DD8E, after 0073DD12.
- the Lua service binding, the string storage, the owner registry, the VFS and the startup
  Application at E1AE90;
- the 00BBCB40 texture-source factories, registered in F8D41C:
  - `CausticsTextureSource`;
  - `ShoreWaveTextureSource0`, `ShoreWaveTextureSource1` and `ShoreWaveTextureSource2`.

  The image runs them at 0073D8CC. The host only has them in `GameNativeMissionFactories`, which
  `MissionShipGraph::prepare` creates just before its preload.

It does not need the command, resources or mission preload graphs, or the D21D00 band.

### Trial run without the factories

The trial is kept outside the tree as `local/gui4_startup_preload_attempt.inc`. It ran the
preload after 0073DD8E, with the factories absent.

The preload itself succeeded: 179 entries, cache closed, and the msd mission borrowed the result.
However, the sampler loader F8D420 was left with 49 records whose +28 resource is null:

- 43 records for `causticstexturesource`;
- 2 each for `shorewavetexturesource0`, `1` and `2`.

At exit the default env then crashed with c0000005. The path was `drain_singletons`, then
`GameSingletonHost::shutdown`, then 00B1B680, then 004DDA40, then 004DDB40. 004DDB40 decrements
`[sink+4]` with no null check (004DDB41..004DDB49).

So the real preload in A1 would also need 00BBCB40 hoisted to 0073D8CC. That is a fourth graph,
and minimal option (b) keeps it mission-only.

### What A1 does instead

A1 opens no shader-cache bracket at all:

- An empty B3A600/B3B140 pair would only add cache I/O, and it would need a second create in
  `ShaderGraph`. The image's startup has no such sequence.
- GUI Text preparation stays on the mission prepare callback, as on main. P3b decides where it
  moves.
- There is no layout-adoption exception and no retention change.
- `BSP_NATIVE_GUI_EFFECT_PROBE` is unaffected.

A later packet could run the real preload by hoisting the factories (0073D8CC) together with it.
Its sampler facade's dependency on the input runtime is a host-order divergence that packet would
have to resolve.

## Provisional: GUI Text type ids

With A1 ON, the line `native GUI Text lineage ready: text=19 element=18` reads `text=21 element=20`.
This is because CD80A0 and CD80F0 take their two type ids at 0073DD12, before GUI Text's late
bootstrap.

In the image the numbering comes from CRT initializer order, which neither build reproduces, so
both numberings are PROVISIONAL. Only type checks read these ids today. Recheck this if anything
ever serializes or compares an id.

## Known limits

- The harness has no two-mission reload. It only has `--menu-select`.
- OFF replaces the embedded scene-environment metadata with one heap allocation in mission
  resources. The default environment does not construct that mission graph; the msd gate must
  cover this changed allocation path.
- The immutable `634258460` matrix baseline used advancing-only tree and scene-child frame
  counters. Common snapshot `6216ea1af` reuses outer-completed frame prefixes within the 256
  reserved slots of each kind; entered, failed and not-entered frames remain reserved. This
  prepared change provides no live GUI, reload or allocator runtime proof.
- The prior msd frame-515 crash was contained on main by 171CEF8C7; this does not prove that the
  late planes draw. See `docs/MILESTONE_D_SCENE_RESOURCE_CONFUSION.md`.

## Application shutdown ordering repair

With `kApplicationCc15Bound` ON, 00737F30 reaches
`GameNativeRendererApplication::release_application_preload_rows()` before the renderer drain.
The msd mission keeps its separate late-model shader-cache bracket open until that drain.
`MissionShipShaderPreloadGraph::retire_before_native_drain()` requires the cache to be closed,
so the earlier row release failed with "mission preload retirement requires the completed
quiescent native Application".

The repair closes that late-model bracket immediately before `release_borrowed_rows()`. It uses
one idempotent helper shared with the later mission retirement. Before an open bracket is
released, the helper checks its completed operation, current cache identity, unreleased state,
and the mission's ready/retiring phase. The earlier entry also retains the renderer's existing
partial-state rejection. The open flag clears only after the native release returns; failures
remain retained. The later drain observes the cleared flag and cannot release the cache twice.

No new build or game run was performed for this repair by the reviewer. Required integration
gates are the current 13-row OFF/base identity, ordered dialog/message/Lua and start-frame/first-RNG
checks, plus a USN04 run with `BSP_NATIVE_MISSION_SHIP=1`, `BSP_NATIVE_MISSION_FRAME=1` and
`BSP_NATIVE_MISSION_UNITS=1` for at least 1600 mission frames that exits 0. Verify the preload rows,
mission graph and renderer all close, and that no partial state is suppressed. A1 stays OFF;
these gates do not accept an ON flip.

## Integrator acceptance, 2026-10-28

The reviewed A1 OFF implementation and shutdown repair are integrated in source
candidate `381792703`. The Win32 Release build and both existing suites passed.
All 13 current OFF/base pairs passed gameplay, ordered dialog/message/Lua,
teardown, start-frame and first-RNG checks. Milestone-D USN04 ran 1600 mission
frames and exited 0, with the preload/Application/mission/renderer closing once
and no unexpected retained state. The startup-domain switch remains OFF.

Exact executable hashes, row receipts and remaining ON gates are recorded in
`docs/CODEX_CC16_INTEGRATION_2026-10-28.md`. These receipts validate the current
domain-only fallback and shutdown correction; they do not accept the ON flip,
startup shader preload, repeated GUI traversal, reload or late-plane drawing.

## A1 activation candidate, 2026-10-28

The switch-only activation candidate `634258460` sets
`kNativeRendererStartupCc15Bound` ON over accepted OFF source `5ca4639f0b`.
No other switch changes. Its lifecycle and all 13 baseline row contracts pass;
raw pair diffs retain exit 1 for the exact startup-domain diagnostic and the
provisional GUI Text type-id records. The serial alternating USN21 4+4 gate
and no-aid pair remain pending a matching latest-common-code OFF/ON build.
Exact evidence and limits are in `docs/CODEX_GUI4_A1_ACTIVATION.md`.
