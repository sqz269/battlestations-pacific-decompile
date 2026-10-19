# oct18b mission ship milestone A-C

The visible milestone is reached in USN04: an installed textured Fletcher is
submitted through B1F4D0 -> D5E5E0/B1D950 -> B24010. The opt-in switch is
`BSP_NATIVE_MISSION_SHIP=1`; its default is OFF.

Core integration commits include `501149fdc`, `9c00d9262`, `d09b30db3`,
`661926708`, `026cfe7b6` and `a3da8ace9`. Component evidence is in
[resources](OCT18B_RENDER_A1.md), [command](OCT18B_RENDER_A2.md),
[scene](OCT18B_RENDER_SCENE.md), [model](OCT18B_RENDER_MODEL.md),
[preload](OCT18B_RENDER_PRELOAD.md), [lighting](OCT18B_RENDER_LIGHTING.md) and
[runtime texture ownership](OCT18B_RENDER_RUNTIME_OWNERS.md). Later integration
adds `708640549`/`743d7bc3f` genuine procedural-factory setup and `380482b47`
idempotent completed retirement. [Factory frames](OCT18B_RENDER_FACTORY_FRAMES.md)
document their bounded Source contract and zero new Original body credit.

## OFF proof

A clean final control was built from main `fa5bbe68c28da0edda76b31079c608c4e3bda643`
in the isolated `oct18b-render-off-ref` worktree. Its executable SHA-256 is
`8d2ef8c34e56c85451205cc289c90e7beba8ba29b7442a0156300eb33e83eb25`.
Both control and candidate use USN04, 600 application frames, press-start 30,
181 mission frames at 0.05 seconds, 1280x720 and vsync. Both manifests have an
empty BSP environment.

Final control: `local/oct18b_render_fa5_reference_usn04.log`; final candidate:
`local/oct18b_render_final_main_off_usn04.log`. The final comparison is recorded
in `local/oct18b_render_final_main_off_pair_diff.json`: **pair_diff exit 0**.
Both processes exit 0. All 1349 native rows, all 665 summaries and all 21 unit
rows match, with no added/removed/status/count differences and no other unmasked
differences. The manifests confirm OFF environments `{}` and ON environment
`{"BSP_NATIVE_MISSION_SHIP":"1"}`.

Earlier full OFF comparisons against main `8d8113104` and `ab2062aec` both
exit **0**. The latter retains all 665 identical summaries and all 21 unit rows,
with no native status/count changes or other unmasked differences; the only
native-row presence difference is recognized OS pretranslation noise.
See `local/oct18b_render_final_off_trace_pair_diff.json`.

The first integration independently passed against frozen main `e3d55e84c`
(`local/oct18b_render_final_off_pair_diff.json`), with 1349 identical native
rows and 665 identical summaries. Its paired PNG bytes also matched.

## ON proof

`local/oct18b_render_final_main_on_usn04.log`, its manifest and frozen
`local/oct18b_render_final_main_on.exe` retain the final live evidence. The run
completes 600 application frames and 599 successful presents. It preloads
179 materials through genuine 73BF80, initializes B14A10/B107F0 resources,
and loads 26 native Fletcher models with 445 mesh sections. B1F4D0 submits
one ordinary command with 134 and 5 entries; each view executes 139 B24010
indexed draws and records all 139 returns, with command/context/batch owners
retired and the native queue empty.

The same run retains these unmodified back-buffer captures:

- `local/oct18b_render_final_main_N001.png` at mission1 / 0.05 seconds.
- `local/oct18b_render_final_main_N060.png` at mission60 / 2.999998 seconds.
- `local/oct18b_render_final_main_N180.png` at mission180 / 9.000016 seconds.

Visual inspection of frame180 shows a recognizable camouflaged Fletcher with
hull, deck and turret texture detail. Earlier landmarks retain opaque native UI
panels that obscure parts of the ship. Immediately before its first B24010,
actual device GetTexture pointers match the native material owners: slot0 is
`models/textures/fletcher_1943_hull_camo.dds` (2048x2048 DXT1); slots1/2 are
`blank.dds` (4x4 DXT1); slots3/4 are `spec.dds` and `bump.dds` (64x64 DXT5).
All GetTexture/description calls succeed. The shader's authored grey Blank
map and dark albedo remain intact.

The installed asset is `models/ships/us/fletcher.mmod`, SHA-256
`a6d4d1bac3d7e13f01f3ab474d5c2f445f42dfb04cb95facc8d8f7257470d211`.
Its authored hierarchy supplies the static pose. The fixed camera is fitted
to native model spheres. Clear-air fog uses genuine B84D00/B84D40 density
setters; real VS/PS readback confirms `fog=200,7000,0` and
`height=200,0,-120`. Explicit native HDR illumination inputs compensate for
this direct linear view's absence of the later exposure/tone-mapping pass.
Camera, fog and light values are diagnostic choices, with no original
mission-lighting or postprocess-parity claim.

Genuine BBCB40/BBC900 factory setup now precedes 73BF80, matching Original
73D8CC before 73DD12. The four factories supply real procedural texture sources;
BBC5D0 retires them after consumer quiescence. The earlier null clock sink and
generated-texture companion failures are resolved. The final run completes the
native singleton/clock drain and closes the mission graph.

**Remaining blocker:** late C++ destruction reaches the unchanged nonempty-list
guard in `NativeTextureLoadOwners::~NativeTextureLoadOwners`. The exact ON16
trace shows B316C0 entering with two texture rows, releasing the noise owner
`effects/noise.dds` from 1 to 0, and its B32250/B31DC0 callback shrinking the
row count to one. B316C0 then rereads the current count and destroys the remaining
error row storage without releasing its texture credit. B32370 releases its
fallback slot from 2 to 1, leaving one canonical `models/textures/error.dds`
record. Original assembly matches the inspected loop, callback, vtable,
publication and registration order; an Original-game leak or missing wider caller
contract remains unproved. No guessed release or guard relaxation was added.
The actual ON process exits `C0000409` through the harness terminate handler,
despite the earlier normal-loop summary. The final dump is
`local/oct18b_render_final_main_on_usn04.dmp`.

Both required checks pass in the final MSVC Win32 build (11.48 seconds).
`local/oct18b_render_final_main_on_evidence.json` summarizes the 561 completed
ordinary command frames, all with 139 returned B24010 calls and empty queues.

## Contracts and next milestone

Native construction, material/texture/geometry loaders, queue execution and
ownership remain in the existing canonical provider domains. New host paths
are gated OFF by default. Source C++ interfaces add explicit contexts and
lifetime observations; original register/stack replacement ABI and retail
parity remain unproved. Address, evidence and uncertainty accompany the
component reconstructions and ledger fragments; Source-only admission work
claims zero additional native body bytes.

D onward must associate rendered models with mission unit identities/poses,
compose the main scene/multipass B156C0/D5E13C path and exposure/tone mapping,
and then submit terrain, ocean, sky and particles. The fixed diagnostic ship
is the A-C milestone rather than a reconstructed whole mission renderer.
