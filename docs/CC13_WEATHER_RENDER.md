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
Call-site claims are in `reports/cc13_weather1_calls.json` (40 rows, `tools/verify_report_calls.py`
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
parameter `cLifetime` bound to the global `010909B4`; seven bolt meshes
`Effects\Lightning\villam_00{1..7}.MMOD` ("villam" is Hungarian for lightning; loop bound
`CMP ESI,7` at `00BC10D5`, and `00BC04C0` releases seven) loaded into `+08..+20` (model) and
`+24..+3C` (scene node), placed at `(0, 20, 0)` and hidden. The bolt pick is `% 6`, so
`villam_007` is loaded but never shown. Then
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
`00BBE640` once per instance for lights 0 and 1 (reconstructed):

1. Sort along `-dir` (`00BC3430`): eye = normalize(-dir) * bounding radius, key = |eye - puff|
   (`+34h`), CRT `qsort` with `00BC2FE0` = `_ftol2(key(b) - key(a))`, so the farthest puff from
   the eye comes first and keys within 1.0 compare equal (their order is whatever the CRT qsort
   leaves).
2. View = look-at from `1000 * dir` towards the origin; up is +Y, or +Z when `dir.y > 0.9`.
3. For each puff in sorted order, `light[L]` = `00BC3730`: the product over the puffs before it
   of `clamp(screen distance / (size_j + size_i) + density, 0, 1)`, distances taken in the x/y of
   `(p, 1) * view` (`00B62D10`). `density` is therefore the minimum transmittance per occluder.
   The mean of the terms goes to `*mean`; each puff also gets `alpha = trunc(Uniform(stream 0,
   0.6, 0.75) * 255)`.
4. `shade[L]` = the mean of `light[L]` over the puffs whose centre is within `size_i * blur`; the
   puff itself counts twice (seed plus the distance-0 match).
5. Lower-half darkening, on every call: `g = clamp(max(mid - y, 0) / half, 0, 1)` over the
   bounds' y range, `shade[0] *= 1 - g`, `shade[1] = g`. Because it runs for both lights, the
   final `shade[0]` carries `(1 - g)^2` and the smoothed `shade[1]` is overwritten by `g`.
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

## Atmosphere and sky box

`00BBDFF0` builds the 40h atmosphere object that `BSP_Game_ConstructWorld` keeps at
`game+19E8h` (`docs/GAME_WORLD_CONSTRUCT.md` calls it the ocean owner): vtable `00D64518` over
`BSP_WeakOwner`, `+0Ch` source object, `+10h` fog receiver (`00BBDF20`), `+18h` sky box
(`00BBF8A0`), `+1Ch` 40000, `+20h` time of day (0.5), `+24h` 1.0, `+28h` second blend
coordinate (0.1), `+2Ch` wind direction normalize(1, 0, 0.4), `+38h` wind speed 0.6, `+3Ch` cloud
manager (`00BBDA50`, A8h). Accessors `00BBDD30` (+20h), `00BBDD40` (+28h), `00BBDD50` (+24h),
`00BBDD60` (&+2Ch), `00BBDD70` (+38h), `00BBDD80` (direction * speed). The cloud wind drift in
`00BBE8B0` reads this direction and speed.

The setters `00BBDE10` (+20h) and `00BBDE50` (+28h) recompute the fog colour with
`00BBF590` on the sky box and pass it to the fog receiver (`00B84C40`); `00BBDE90` sets the wind
from a vector. These and `00BBDE00` lie in `00BBDDF0..00BBDEC0`, between segment 93 and 94, where
Ghidra has only the 3-byte `TRIV_body_00bbddf0`; they are documented here, not defined or renamed
(outside this lease).

Sky box (`00BBF8A0`, 130h): `sky\skybox.mmod`; three layers of 54h bytes from `+34h`, each with
four texel taps `cOffset0..3` (0, 1/w, 1/h, both) and `cTextureSize` from the layer texture
`<name>_A/_B/_C.tga` (`+84h`, `+D8h`, `+12Ch`); mesh elements whose texture name contains
`skyboxdummy_a/b/c` bind layer 0/1/2 and `cLuminance`. `ConstructWorld` sets the rotation
(`00BBF1A0`, degrees from scene record `+C5Ch`) and `+30h` (`00BBF200`, record `+C60h`);
`world_construct.cpp` calls these two `ocean_set_scalar`.

Fog colour (`00BBF590`, reconstructed): two dword colour strips at sky box `+18h` and `+24h`
(B,G,R,A bytes). `00BBF440` samples a strip at `u = count*t - 0.5`, takes the truncated texel
unclamped and the next texel wrapped (signed `%`) or clamped, and lerps (`00BBF280`). The colour is
`lerp(c1, c2, c1.a)` with c1 from the first strip at the time of day (wrapping) and c2 from the
second at `+28h` (clamped), then `004FB850` packs ARGB. No writer of the two strips was found in
this segment (`00BBF600`, the only other user of their growth helper `00BBF390`, has no
reference).

## Application model factories and the lightning strand model

The application vtable `00CFEAB0` has four model factories at `+1Ch..+28h` (`+2Ch` `00BC04B0`
returns 0): cloud render model (`00BC02E0` -> pool `010909F4`, `00BC54F0`, `cloud.mvfm`, vtable
`00D64878` with the cCloudInstance type slots), cloud plane (`00BC0350` -> pool `01090A2C`,
`00BC6060` over `00BC7DB0` "cloudplane"), an unnamed refcounted object (`00BC03C0` -> `00BC6430`,
vtable `00D64940`) and the lightning strand model (`00BC0440` -> pool `01090A68`, `00BC7460`).
The three pool thunks `00BC54E0`, `00BC6050`, `00BC7450` were tagged `CG_static_dtor_stub`; they
are `MOV ECX,pool; JMP allocate`.

`00BC7460` builds a GeneratedModel "Lightning" (`00BC93E0`) with a 200-vertex, 50-quad strip and two
draw sections (`lightning.mshd`, `thunder.mshd`); the definition passed in has atlas items at
`+24h` (14h records, names resolved through `00AEFB20`). `00BC84A0` (from `00BC9390`, vtable
`00D64A50` +5Ch) removes expired strands, and when its timer `+1B8h` runs out spawns
Uniform(0, def+14h) bursts of up to 50 segments with random directions, widths and atlas cells,
then re-arms with Uniform(0, def+18h); all of it on stream 0. It is named, not reconstructed.

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
| `00BC84A0` | 0 | 1 per expiry of the burst timer, plus several per segment | lightning strand model tick |

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
| `00BBF280` | `sky_lerp_00bbf280` | whole |
| `00BBF440` | `sky_gradient_sample_00bbf440` | whole |
| `00BBF590` | `sky_fog_colour_00bbf590` | float colour before `004FB850` |
| `00BC2FE0` | `cloud_puff_compare_00bc2fe0` | whole |
| `00BC3430` | `cloud_sort_puffs_00bc3430` | whole; qsort via `CloudLightView` |
| `00BC3730` | `cloud_puff_transmittance_00bc3730` | whole; transform via `CloudLightView` |
| `00BC3930` | `cloud_compute_lighting_00bc3930` | whole; look-at via `CloudLightView` |

Original ABI per function is in the ledger records. Uncertainty shared by all: x87 intermediates
are evaluated in double and rounded at the native float spills; `fsin`/`fcos` are `std::sin`/
`std::cos` on the float argument; the vector kernels (`00419440`, `00419510`, `00419260`,
`0042CF10`, `00BF7030`) and the generator are host calls (`WeatherMath`), so the order of calls is
the contract. NaN orderings follow the native FCOMIP/COMISS branches where they were checked
(drop life, stick, gravity weight, bounds, recycle).

## Ghidra functions defined by this packet

`reports/cc13_weather1_defined.json`: `00BBE2B0`, `00BBE2C0`, `00BBE2E0` (cCloudInstance type
slots, vtables `00D64528` and `00D64878`), `00BC1230` (waterdrops vtable +58h) and `00BC2340`
(waterdrops vtable +60h, 00BC2340..00BC27B0), `00BC7170` (lightning model vtable +5Ch) and
`00BC4D20` (cloud render model vtable +5Ch, 00BC4D20..00BC4FA8). Code without any reference was left undefined:
`00BBE5B0` (returns 2.0f), `00BBE5D0` (component multiply), `00BC1250..00BC1290`, `00BC1850`,
`00BC4A10` (name-string copy of the cloud class).

## Segment 9 triage (`004BA290-004D0860`)

Segment 9 is the tail of the `GGame` translation unit (strings `GGame::OnInitTitle`,
`GGame::Render`): 395 candidates, 133 already reconstructed and 171 reviewed names before this
packet, almost all documented elsewhere (`docs/GAME_FRAME_CONTROL.md`, `GAME_RENDER_FRAME.md`,
`FRONTEND_MANAGERS.md`, `APP_FRAME_GAME_STATE.md`, `CONTROLLED_UNIT.md`, `GAME_EXECUTABLE.md`,
`ENTITY_LIFECYCLE_TAILS.md`, `CC13_SCENE_BROWSER.md`). The 269 remaining `FUN_` entries were
triaged with their callers, callees and string references:

| Cluster | Addresses | Disposition |
| --- | --- | --- |
| Xbox LIVE lobby, friends, leaderboards | `004BB3E0..004BC7F0` (`XUserReadStats`, `XEnumerate`, `XFriendsCreateEnumerator`, `XCancelOverlapped`, `XUserSetContext`) | multiplayer/LIVE session, skipped |
| Online stats | `004C04C0` (`XSessionWriteStats`), `004C5F10`, `004C9780`, `004CAA90` ("- - ONLINE - - WriteStats userleft %d") | multiplayer, skipped |
| Network bit writer use | `004C60E0` (`BSP_BitCursor_Write*`), `004CA120` (`ingame.multi_highlatency`) | multiplayer, skipped |
| Session/player/unit glue | `004C3A80`, `004C5070..004C5140` (input contexts), `004BFED0`, `004C4900` | gameplay, covered by cc9/cc12 docs or callers already named; skipped |
| Border-zone and segment geometry | `004BBDD0`, `004BC120`, `004C7AA0`, `004CA760..004CA930` | gameplay world queries; `004BC120`, `004C7AA0`, `004CA930` already cited by other docs; skipped |
| Containers, observers, EH helpers | `004BA290..004BA570`, `004BD350..004BF8A0`, `004C1630..004C2AD0`, `004C8070..004C9A60`, `004CC390..004CCAB0` | MSVC container/observer instantiations and unwind helpers, no anchors; left raw |
| STL instantiations | `004CDEC0..004D0570` | `xlen` throw strings; left as instantiations |

Named here: `004BFFA0` `BSP_Game_GetMissionCampaignKind` (reconstructed in
`src/game_mission_kind.cpp`: campaign kind from the mission path at `[[game+5FCh]+910h]`, IJN 1,
USN 2, TrainingGrounds 3, Multi 4, tested in the native order with case-sensitive `strstr`; sole
call `004C0343` in the LIVE presence refresh), `004CACA0` `BSP_NativeString_AppendInt`,
`004CD450` (picks the first unused `c:\screenshot_<n>.png`, counter `game+1940h`, then renderer
vtable+110h; unreferenced debug code), and the two `"white.dds"` default constructors
`004C62A0`/`004C63E0` called by `BSP_SceneRecord_Construct` (500, 1.0, white.dds, four 1.0s,
zeros; field meanings unknown).

The weather link in this segment, `004BA870` `BSP_SceneRecord_ScatterClouds` (`Cloud`,
`CloudSmall`), was already reconstructed (`docs/GAME_EXECUTABLE.md`).

## Open

- Render-model bodies not reconstructed: `00BC4D20` cloud puff sort/cull, `00BC5300`,
  `00BC66B0`/`00BC6FA0` vertex fills, `00BC84A0`/`00BC9160` strand update and fill, `00BC2340`
  and `00BC2D50` water-drop geometry. The per-family vector helpers (`00BC4C50..00BC5300`,
  `00BC5A50..00BC5E70`, `00BC6C20..00BC7270`) are unnamed.
- `00BC3580` (cloud sphere coverage query, sqrt against `radius * 00CEC160`) has no reference; not
  analysed further.
- `00BBE380` also calls `00BC33B0` (the class `+1A8h` model upload); both are unnamed.
