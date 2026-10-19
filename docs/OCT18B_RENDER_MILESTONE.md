# oct18b mission ship milestone A-C

The visible milestone is reached in USN04: an installed textured Fletcher is
submitted through B1F4D0 -> D5E5E0/B1D950 -> B24010. The opt-in switch is
`BSP_NATIVE_MISSION_SHIP=1`; its default is OFF.

Core integration commits include `501149fdc`, `9c00d9262`, `d09b30db3`,
`661926708`, `026cfe7b6` and `a3da8ace9`. Component evidence is in
[resources](OCT18B_RENDER_A1.md), [command](OCT18B_RENDER_A2.md),
[scene](OCT18B_RENDER_SCENE.md), [model](OCT18B_RENDER_MODEL.md),
[preload](OCT18B_RENDER_PRELOAD.md), [lighting](OCT18B_RENDER_LIGHTING.md) and
[runtime texture ownership](OCT18B_RENDER_RUNTIME_OWNERS.md).

## OFF proof

A clean control was built from main `8d81131049eef754908c335410f7a14c2a87e633`
in the isolated `oct18b-render-off-ref` worktree. Its executable SHA-256 is
`90d8342d01d0b2277dcc1d91e66f945301ef928a43bdd0c869dcc3cfbe2ab4f2`.
Both control and candidate use USN04, 600 application frames, press-start 30,
181 mission frames at 0.05 seconds, 1280x720 and vsync. Both manifests have an
empty BSP environment.

`tools/pair_diff.py local/oct18b_render_current_main_reference_usn04.log
local/oct18b_render_final_off_8d_usn04.log --json
local/oct18b_render_final_off_8d_pair_diff.json --limit 1` exits **0**.
Gameplay and all 21 unit rows match; there are no native status/count changes,
all 664 summaries match, and other unmasked lines match. The sole native-row
presence difference is the tool's recognized OS pretranslation noise.
The earlier `checkpoint_off` comparison also exits0 with 1349 identical rows.

The first integration independently passed against frozen main `e3d55e84c`
(`local/oct18b_render_final_off_pair_diff.json`), with 1349 identical native
rows and 665 identical summaries. Its paired PNG bytes also matched.

## ON proof

`local/oct18b_render_on_attempt12.log`, its manifest and frozen
`local/oct18b_render_on_attempt12.exe` retain the live evidence. The run
completes 600 application frames and 599 successful presents. It preloads
179 materials through genuine 73BF80, initializes B14A10/B107F0 resources,
and loads 26 native Fletcher models with 445 mesh sections. B1F4D0 submits
one ordinary command with 134 and 5 entries; each view executes 139 B24010
indexed draws and records all 139 returns, with command/context/batch owners
retired and the native queue empty.

The same run retains these unmodified back-buffer captures:

- `local/oct18b_render_on_final_N001.png` at mission1 / 0.05 seconds.
- `local/oct18b_render_on_final_N060.png` at mission60 / 2.999998 seconds.
- `local/oct18b_render_on_final_N180.png` at mission180 / 9.000016 seconds.

Visual inspection of frame180 shows a recognizable camouflaged Fletcher with
hull, deck and turret texture detail. Immediately before its first B24010,
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

ON12 reaches native drain after retiring preload Application credits,
scene/model ownership, lighting, camera and foliage. Generated textures now
join the existing canonical texture-owner service, which closes the former
luminance-quad companion failure. A later particle-clock cleanup still faults
in B1B680 -> 004DDB40 on a null sink's reference count (KERNEL32 write4).
`local/oct18b_render_on_attempt12.dmp` retains this boundary. Normal shutdown
is being completed separately from the established draw/screenshot proof.

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
