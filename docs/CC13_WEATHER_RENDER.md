# CC13 weather render (partition segment 94)

Packet `cc13_weather1_seg94`, worker cc13-weather1. Segment 94 is `00BBDEC0-00BD2E60`; its
weather and sky part is `00BBDEC0-00BC94D0`, followed by zlib 1.2.1 (`00BC94D0-00BCF720`,
already tagged) and the pointer-vector, singleton, allocator and event helpers that earlier
packets reconstructed (`00BCF910-00BD2E20`). Before this packet the weather part had one
reconstructed function (`00BBDF20`, `src/system_fog_owner.cpp`).

Analysis target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Names are hypotheses
backed by string anchors, vtables and caller context, not recovered symbols. Status words:
*named* (ledger + Ghidra), *reconstructed* (C++ in `src/weather_render.cpp`), *build-tested*
(`scripts/build.ps1` passes). Nothing here is fixture-tested, ABI-compatible or game-validated.
Call-site claims are in `reports/cc13_weather1_calls.json` (23 rows, `tools/verify_report_calls.py`
clean).

## Correction: the "ocean" at `[game+19E8h]+3Ch` is the cloud manager

`docs/GAME_WORLD_OCEAN.md` and `src/world_ocean.cpp` treat the object at `[game+19E8h]+3Ch`
(`00BBDDD0` BSP_Ocean_Update, `00BBD310` BSP_Ocean_UpdateFrameAndLayers) as the ocean and its
`+18h` layers as shore-wave layers updated by `00BBE8B0`. The evidence says these are clouds:

- `00BBE8B0` registers `cCloudAmbientColor`, `cCloudDiffuseColor`, `cCloudSpecularColor`,
  `cLightDir01`, `cColor01`, `cColor02` and `cScale` (datarefs `00D645A8..00D64608`).
- Its objects are built by `00BBE7D0`, a GeneratedModel named `"CloudInstance"` (vtable
  `00D64528`, type info `"cCloudInstance"` at `00BBE770`). The constructor zeroes `+1B8h..+1C0h`,
  exactly the "scroll" vector the ocean document found, and sets `+1E8h` (wind flag) to 1.
- `00BBDDF0` reads the same `[game+19E8h]+3Ch`; it is the object the clouds option and the cloud
  scene-property pass use (`include/bsp/options_menu_screens.hpp:400`,
  `include/bsp/cloud_scene_properties.hpp:42`). The `+98h` "enabled" flag is the clouds switch.
- `00BBD6C0`, called by `BSP_CloudClass_ReadLua` (`004774A0`), creates cloud classes through
  `00BC4A60`.

The frame arithmetic the ocean document reconstructed is still the arithmetic of `00BBD310`;
only the object identity is wrong. `00BBE8B0` is renamed `BSP_CloudInstance_Update` (old name
recorded by `ghidra_annotate`). `00BBD310`/`00BBDDD0` and the two files were not touched: they
sit outside this packet's lease (segment 93). The lead was told.

## Lightning storm (`00BC0510`, `00BC0D40`)

Owner: the gameplay-effect code in segment 53 builds the storm with `00873EB0`
(`Effects\Lightning\lightning.ats`) and ticks it from `00873C80`.

Construction (`00BC0D40`): material `3dLightning.mshd` with texture `lightning_002.dds`, float
parameter `cLifetime` bound to the global `010909B4`; six bolt meshes
`Effects\Lightning\villam_00{1..6}.MMOD` ("villam" is Hungarian for lightning) loaded into
`+08..+1C` (model) and `+24..+38` (scene node), placed at `(0, 20, 0)` and hidden. Then
`+00` flash duration 0.1 (`00D7A2F0`), `+04` max strike interval 14.1 (`00D64740`), `+40` timer
= Uniform(stream 0, 0, 14.1), `+44` age -1000, `+48` light null, `+4C` flashes left 0.

Tick (`00BC0510(camera, delta, vec3* thunder_out)`, RET 0Ch, AL = a strike sequence started):

1. timer -= delta. While the age is non-negative, age += delta and
   `010909B4 = 1 - (6 - flashes_left) * 0.14` (flashes read unsigned).
2. When timer <= 0 and no flashes are left, a strike starts: bolt = `00BD2FC0(stream 1) % 6`;
   bearing Uniform(stream 0, -80, 80) degrees off the camera forward in the horizontal plane
   (forward from `00B6FC60`, right from `00B70FE0`, both flattened and normalised); range
   Uniform(stream 0, 500, 2000); the bolt node moves to camera + dir * range and becomes visible;
   `*thunder_out` = camera + dir * ((range - 500) * 0.8667 + 200), i.e. the thunder source is
   placed between 200 and 1500 along the same bearing. A point light
   `"dynamic_light_3Dlightning"` (radius 20000, diffuse (1,1,1,1), scale 4) goes 500 above the
   bolt base; flashes left = trunc(Uniform(stream 0, 1, 5)) (FISTP qword under a chop control
   word, so 1..4; Ghidra's `ROUND` is wrong). Timer is parked at 1000.
3. When timer <= 0 with flashes left, the same bolt flashes again with light scale
   `4 - (6 - flashes_left) * 0.5`, then flashes_left -= 1.
4. When age > duration the bolt is hidden, the light released, age = -1000, `010909B4` = 0, and
   timer = Uniform(stream 0, 0, flashes_left ? 0.07 : 14.1).

The native writes the new light's fields without checking the allocation result of `00B7BD30`.

## Camera water drops (`00BC12F0`, `00BC1A20`, `00BC1990`, `00BC2030`)

Driven by the gameplay-effect Waterdrops component (`00873100`, definition fields in
`docs/GAMEPLAY_EFFECT_SCALAR_COMPONENTS.md`). On a trigger the component scales WaterdropCount by
a distance falloff, creates a new system (`00BC2D00`: pool `00BC2BB0`, 1DCh objects, then
`00BC2030`), copies the definition through the setters below and spawns; every tick it calls
`00BC12F0` with `[00E188DC]` world position (zero when absent).

| Offset | Field | Writer |
| --- | --- | --- |
| `184h/188h/18Ch` | drop vector (34h bytes per drop) | `00BC1860` |
| `190h` | requested count | `00BC11A0` |
| `194h/198h` | MinSize / MaxSize (0.01 / 0.025) | `00BC1160` |
| `19Ch/1A0h` | MinLifeTime / MaxLifeTime (0.5 / 4) | `00BC1180` |
| `1A4h` | StickyPercent | `00BC11D0` |
| `1A8h` | live drops last tick | `00BC12F0` |
| `1ACh` / `1B8h` | last camera / old - new camera | `00BC1A20`, `00BC12F0` |
| `1C4h` | Gravity (0.95) | `00BC11B0` |
| `1C8h/1CCh/1D0h` | texture, cells = vt+3Ch / vt+40h, 1/cells | `00BC1990` |
| `1D4h..1D7h` | ApplyAtBottomOnly (default 1), StartTopOffScreen, ApplyAtTopOnly, NoCameraMove | `00BC11F0..00BC1220` |

Drop record: x, y (screen -1..1), size, height (size * camera `+1C8h` aspect), slide
(Uniform(0.5,1) * sqrt(size^2 + height^2)), unit direction from (0.01, 0.01), life total, life
left, `+24h` (0, rewritten 0 each tick), `+28h` (0, rewritten -1 each tick), stick (1, or 1000 for
the first requested*StickyPercent/100 drops, whose life doubles), texture cell
(`_ftol2(Uniform(0,1000)) & (cells-1)`).

Placement: ApplyAtBottomOnly takes x = a*1.4142, y = 1.4142*(1-b^2) rotated by the camera roll
(`-asin(m01)`, mirrored through 3.14 when `m11 < 0`, matrix at camera `+F0h`); StartTopOffScreen
puts y in [1, 1.1]; ApplyAtTopOnly in [0.5, 0.8]; otherwise y in [-1, 1].

Global budget `010909B8`: spawn caps requested so the total stays at most 3000 (unsigned
compare); the destructor body `00BC1F30` subtracts requested, floored at 0.

Tick: the camera step and the world down axis go to view space (`00B6FCB0` + `0042D0D0`, no
normalise); visibility = (live last tick != 0); each live drop loses delta of life; drops with
stick <= 1 and NoCameraMove clear get a jitter of Uniform(0.001, 0.005) * |step| * stick in a
random unit direction plus a streak of (dir + view step) * |step| * slide * delta; stick decays
by 0.2/s; then gravity rotated by Uniform(-pi/5, pi/5) moves the drop by
`g * delta * Gravity * slide * clamp(1 - stick, 0, 1)` (so sticky drops never slide).

## Particle cloud class (`00BC4A60` family)

`00BC4A60` (vtable `00D64860`, from `00BBD6C0`) stores name `+0Ch`, texture name `+60h`, density
`+08h`, then `00BC47D0` loads: material `cloud.mshd` with the class texture, bounds reset to
+-1e10, resource read (`00BC4530` -> `00BC4480` "Resource" -> `00BC43D0` "CloudSystem" ->
`00BC42E0`), puffs, bounds.

- `00BC42E0`: a CloudSystem element is a count and that many (x, y, z, radius) spheres; `+3Ch`
  keeps the largest radius.
- `00BC40F0`: for each sphere, `density * 1000` attempts, each a cube-uniform direction (three
  Uniform(-100, 100), normalised by hand, so not sphere-uniform), radius Uniform(0, sphere
  radius) and spin Uniform(0, 2pi); puff size is `max radius / 2.7`.
- `00BC3EF0`: an attempt is dropped when it lies within `existing.size * 0.45 + size * 0.5` of
  any existing puff (`00BC32F0`: per-axis box test, then length^2 <= r^2); otherwise the factory
  at `[010909B0]` vtable+0Ch makes the puff and the bounding radius (`+2Ch`, max |p| + size) and
  box (`+44h`, `+50h`) grow.
- `00BC47D0` tail: centre `+84h`, extent `+78h`, inverse extent `+6Ch`.

Puff accessors (`00BC7A70..00BC7B50`): position `+0Ch`, size `+1Ch`, two light terms `+20h[2]`,
two shade terms `+28h[2]`, alpha byte `+30h` (`trunc(f*255)`), sort key `+34h`.

`00BC3930` (light index, direction, density `01090908`, blur `01090904`, float* mean) is called by
`00BBE640` once per instance for lights 0 and 1. It sorts the puffs along the light (`00BC3430`,
qsort with `00BC2FE0`), builds a look-at from 1000 * direction, computes each puff's light term
(`00BC3730`), draws one Uniform per puff for the alpha byte, smooths the terms over neighbours
within `size * blur`, and writes a height gradient into the shade terms. Its body uses ECX puff
pointers Ghidra lost; it is named, not reconstructed.

## Cloud instance (`00BBE7D0`, `00BBE8B0`)

Setters (called from the cloud scene-property pass `00477710`): scale `+1C8h` (`00BBE470`),
alpha `+1D8h` (`00BBE490`), wind flag `+1E8h` (`00BBE5C0`), and five globals shared by every
instance: ambient `01090970`, diffuse `01090980`, specular `01090990`, inside `010909A0`, density
`01090908`, blur radius `01090904`.

`00BBE8B0` per tick: lazy lighting (`00BBE640`); visibility 1; fade
`t = clamp((max(|camera - pos| - class radius, 0) - 2000) / 600, 0, 1)` and visibility
`+1C4h * (1 - t)`. Fully faded: nothing more when the drift is zero; otherwise, when the home
position (pos - drift) is also out of range (`1 - (reach - 2000) * 0.002 <= 0`, reach >= 2500),
the instance snaps home and the drift clears, and the call returns either way before the material
update. Visible: when the manager's wind flag (`00BBCE70`) and `+1E8h` are set, drift and position
advance by `direction * delta * speed * 30` (`00BBDD60`, `00BBDD70`, `00CE7630`); then vtable+5Ch
and the material parameters.

## Random draws (process generator `00BD2F10` / `00BD2FC0`)

All draws use stream 0 (`XOR ECX,ECX`) except the bolt choice. These are the same per-thread
states gameplay uses (see `include/bsp/random_threads.hpp`), so weather presentation shifts later
gameplay draws whenever it runs on the same thread.

| Site | Stream | Draws | When |
| --- | --- | --- | --- |
| `00BC1102` | 0 | 1 | storm construction |
| `00BC05A9` | 1 | 1 (u32 % 6) | strike start |
| `00BC05DD`, `00BC072B` | 0 | 2 | strike start |
| `00BC0A93` | 0 | 1 | strike start (flash count) |
| `00BC0CF4` | 0 | 1 | end of every flash |
| `00BC1A20` loop | 0 | 6 per drop | waterdrops spawn |
| `00BC14C8..00BC152F` | 0 | 3 per non-sticky live drop per tick (unless NoCameraMove) | waterdrops tick |
| `00BC16BF` | 0 | 1 per live drop per tick | waterdrops tick |
| `00BC40F0` loop | 0 | 6 per puff attempt | cloud class load |
| `00BC3930` loop | 0 | 1 per puff per light (2 lights) | first update of each cloud instance |

The water-drop tick is the notable one: it draws once or four times per live drop per frame, so
its consumption is frame-rate dependent while drops are on screen.

## Reconstructed (`src/weather_render.cpp`, build-tested)

| Native | C++ | Coverage |
| --- | --- | --- |
| `00BC0D40` | `lightning_storm_init_00bc0d40` | timing tail only |
| `00BC0510` | `lightning_storm_update_00bc0510` | whole state machine; nodes/lights via `LightningHost` |
| `00BC1990` | `waterdrops_set_texture_cells_00bc1990` | cell arithmetic |
| `00BC1A20` | `waterdrops_spawn_00bc1a20` | whole; attach/hide via host |
| `00BC12F0` | `waterdrops_update_00bc12f0` | whole |
| `00BC1F30` | `waterdrops_release_budget_00bc1f30` | budget release only |
| `00BC42E0` | `cloud_read_spheres_00bc42e0` | whole (reader interface) |
| `00BC32F0` | `cloud_offset_within_00bc32f0` | whole |
| `00BC3EF0` | `cloud_add_puff_00bc3ef0` | whole; puff object via factory |
| `00BC40F0` | `cloud_scatter_puffs_00bc40f0` | whole |
| `00BC47D0` | `cloud_build_geometry_00bc47d0` | scatter loop and bounds tail |
| `00BBE8B0` | `cloud_instance_step_00bbe8b0` (+ fade / recycle helpers) | fade, recycle, wind drift |

Original ABI per function is in the ledger records. Uncertainty shared by all: x87 intermediates
are evaluated in double and rounded at the native float spills; `fsin`/`fcos` are `std::sin`/
`std::cos` on the float argument; the vector kernels (`00419440`, `00419510`, `00419260`,
`0042CF10`, `00BF7030`) and the generator are host calls (`WeatherMath`), so the order of calls is
the contract. NaN orderings follow the native FCOMIP/COMISS branches where they were checked
(drop life, stick, gravity weight, bounds, recycle).

## Ghidra functions defined by this packet

`reports/cc13_weather1_defined.json`: `00BBE2B0`, `00BBE2C0`, `00BBE2E0` (cCloudInstance type
slots, vtables `00D64528` and `00D64878`), `00BC1230` (waterdrops vtable +58h) and `00BC2340`
(waterdrops vtable +60h, 00BC2340..00BC27B0). Code without any reference was left undefined:
`00BBE5B0` (returns 2.0f), `00BBE5D0` (component multiply), `00BC1250..00BC1290`, `00BC1850`,
`00BC4A10` (name-string copy of the cloud class).

## Open

- Render families not yet read: `00BC4C50..00BC54F0` (vtable `00D64878`, cloud render model,
  vt+5Ch `00BC4D20` has no Ghidra function), `00BC5A50..00BC6060` (`cloud.mvfm`),
  `00BC6C20..00BC7460` (`thunder.mshd`, `lightning.mvfm`, `lightning.mshd`), `00BC7DB0`
  (`cloudplane`), `00BC84A0` (410 lines), `00BBF6D0`/`00BBF8A0` (sky box `sky\skybox.mmod`,
  `skyboxdummy_a..c`, `cLuminance`, `cOffset0..3`, `cTextureSize`).
- `00BC3930`/`00BC3730`/`00BC3430`/`00BC3580` cloud lighting: needs an assembly pass.
- `00BBE380` also calls `00BC33B0` (the class `+1A8h` model upload); both are unnamed.
