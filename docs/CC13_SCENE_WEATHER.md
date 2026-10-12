# Scene-header weather, fog, lights, sky and music readers (cc13-weather1)

Addresses: 004ECA30 004E5C00 004EA870 004EE450 004ED690 004ED2D0 004ED530 004B6800 004B6980
004CB420 004F1D70 00865900 004DE610 0078C9B0 004EB9B0 004EB180 004EB5A0 004E5D20 004EE9E0 004ED910
004E8510 00469BF0

The second packet (`cc13_scene_headers2`, sections from "004EB9B0 g_Ocean" on) adds the ocean,
terrain, terrain-texture and post-process readers in the same files. Its call-site report is
`reports/cc13_scene_environment_calls.json` (16 rows, 0 failed).

Packet `cc13_scene_weather`. Code: `include/bsp/scene_weather_cc13.hpp`,
`src/scene_weather_cc13.cpp` (namespace `bsp::scene_weather_cc13`, target `bsp_core`). Call-site
report: `reports/cc13_scene_weather_calls.json` (`tools/verify_report_calls.py`: 20 rows, 0 failed).

Status, in the project's terms:

- **Reconstructed, build-tested.** 004ECA30, 004E5C00, 004EA870, 004EE450 (with 004B6800's
  defaults), 004ED690 with its helpers 004ED530 and 004ED2D0.
- **Probe-checked, not fixture-tested.** An ad hoc probe (not committed; it lives in the worker's
  `local/`) parsed all 259 `.scn` files of this installation with the host's own
  `parse_scene_document`, adapted the header through `scene_bag_from_block`, and ran the readers
  with linearization on. Results: 258 g_Weather blocks read without error, plus 1 scene with no
  g_Weather (`COTP-IJN/ijn_02_force_z.scn`, which also has no g_Environment). All 258
  g_Environment blocks and all 259 Sound blocks read without error. One sample was checked by hand
  against its file (`bsm_01_stationed_at_pearl.scn`).
- **Not ABI-compatible, not game-validated, not wired into the host.**

Names are hypotheses. The scene keys are the image's own strings (pushed immediates, read from the PE).

## Where the readers sit

`BSP_SceneRecord_ApplyHeaderProperties` 004F1D70 runs over the header `properties` bag and hands
sub-bags to the readers (record = EBP, the scene record that becomes `[game+5FCh]`):

| site | key | callee |
|---|---|---|
| 004F2205 | `g_Ocean` (type 6) | 004EB9B0 water (not in this packet) |
| 004F2237 | `g_Weather` (type 6) | **004ECA30** |
| 004F2242..004F2271 | no `g_Weather` | 004CB420 builds a default 288h block, 004E8BF0 copies it over record+990h, then 004C6520 destroys the temporary |
| 004F22C0 | `Sound` (type 6), only when the stack flag byte `[ESP+2ACh]` is set | **004ED690** |
| 004F230B | `g_Environment` (type 6) | **004EE450**, which calls 004EA870 with its `Lights` |
| 004F232A | `g_Terrain` | 004EE9E0 (not in this packet) |
| 004F2349 | `g_PostProcess` | 004E8510 (not in this packet) |
| 004F2368 | `Cloud` | 004E7220 cloud areas (`include/bsp/gui_screens_3.hpp`) |

One quirk follows from this order. When `g_Weather` is absent, the default block copy also resets
the `g_Ocean` fields (block +0h..+5Ch, +104h.., +144h..) that 004EB9B0 has just written. Only
`ijn_02_force_z.scn` reaches this path, and it has no g_Ocean either.

Every reader is `__fastcall(ECX = bag, EDX = scene record)` with a plain `RET`. The two helpers are
the exception: 004ED530 and 004ED2D0 also take `(index, record)` on the stack and end with `RET 8`.
The bag lookups are `BSP_ScenePropertyBag_Find` 008F2260 and its two C-string wrappers 004772C0
and 004773A0. The 004773A0 wrapper finds the key, then finds it again and returns the second
result, which is the same record. The sweep's warning about "register-hidden sub-bag switching"
comes down to this: each sub-bag pointer is the found record's `+0Ch`, loaded into ESI, EDI, EBX
or EBP. That `+0Ch` becomes the next ECX, and the pseudocode drops ECX. Every bag below was
traced from the assembly, not from the pseudocode.

## Value reads (shared by all readers)

- **Number.** The reader checks `CMP [rec+4],0`. Type 0 (I) goes through `CVTSI2SS`. Any other
  type has its `+0Ch` dword reinterpreted as a float.
- **Byte.** `MOV r8,[rec+0Ch]`: the low byte of the value. `RainColor` is authored as `I`, so it
  takes the int's low byte.
- **Raw dword.** `RainDropNum` is copied as an int32 (`MOV EDX,[rec+0Ch]`).
- **Sub-bag.** The child is `[rec+0Ch]`. A null child is tested only where the table says so.
- **Required keys and host errors.** In the tables, "required" means the image dereferences the
  008F2260 result with no null test. In the host, a missing required key returns `false` with an
  error, and every field written before it keeps its value.

## 004ECA30 `BSP_SceneRecord_ReadWeatherFogAndCloudLights`

The writes go into the record. "Block" is record+990h, as in `include/bsp/water_render.hpp`.

| order | key path (in g_Weather) | gate | record (block) | treatment |
|---|---|---|---|---|
| 1 | `CloudsAffectedByWind` | required; stored only if type 3 | BECh (+25Ch) | byte |
| 2 | `Rain` | optional; a null child skips to `Fog` | | |
| 3 | `Rain.IsRain` | required | 9F0h (+60h) | byte |
| 4 | `Rain.RainDropNum` | required | 9F4h (+64h) | raw dword |
| 5 | `Rain.RainSpeed.X/Y/Z` | `RainSpeed` required; a null child skips the three | 9F8h/9FCh/A00h (+68h..) | number |
| 6 | `Rain.RainColor.R/G/B/A` | presence test (004772C0); child not null-tested | A06h/A05h/A04h/A07h (+74h) | bytes, so memory order B,G,R,A (a D3DCOLOR) |
| 7 | `Rain.RainDropLengthMin`, `Max` | each optional | A08h, A0Ch (+78h, +7Ch) | number |
| 8 | `Fog` | optional; a null child skips to `CloudLights` | | |
| 9 | `LinFogDistDensity`, `LinFogHeightDensity`, `LinFogDistStart`, `LinFogDistEnd`, `LinFogHeightEnd`, `LinFogUWHeightEnd` | required | A60h, A6Ch, A64h, A68h, A70h, A74h (+D0h, +DCh, +D4h, +D8h, +E0h, +E4h) | number |
| 10 | `ObjectRenderClip`, `ReflectRenderClip`, `ShadowRenderClip`, `RefractRenderClip`, `xFoliageRenderClip`, `xFoliageLowResDist` | each optional | A78h, A7Ch, A84h, A80h, A88h, A8Ch (+E8h..+FCh) | number |
| 11 | `FogColor.R/G/B/L` | required; a null child skips to `DirFogColor` | A10h..A1Ch (+80h) | rgb = rgb*L, a = L, then linearize rgb |
| 12 | `DirFogColor.Right/Left/Front/Back`, each `.R/G/B/L` | required; a null DirFogColor child ends `Fog`; the face children are not null-tested | A20h + i*10h (+90h..+CFh) | rgb = rgb*L, a = 1.0, never linearized |
| 13 | `CloudLights.Ambient/Sun/Bottom.R/G/B` | `CloudLights` optional; each colour key required; a null child skips that colour | 1014h, 1024h, 1034h | rgb, then linearize. Alpha is untouched |

The x87 detail at item 11 (004ECF1B..004ECF63) comes from the raw bytes. `D9 C0 / DE CA` is
FMULP ST(2),ST and `D8 C9` is FMUL ST,ST(1), so blue is scaled like red and green. The sequence
also multiplies the old `+A1Ch` by L (`D8 4B 0C`); 004ECF63 then overwrites that product with L.
Each product of two floats is exact in the 80-bit register, so the host's float multiply is
bit-identical. Item 12 uses only memory-operand FMULs. Its alpha is set to 1.0 (00D7A24C) after
the multiply.

The g_Weather path never writes block +100h. That byte is `BushShadowAllowed`, written by
004EA870 below.

### Linearization 004E5C00 `BSP_Color_LinearizeRgbGamma22`

`__fastcall(float* rgb)`, RET. It runs only when the settings byte `00F88A0C` is non-zero. That
byte is GameSettings +8Ch (`shader_flag_8c` in `include/bsp/game_settings.hpp`, default 1, not
persisted). For each of the three channels:

1. 0 stays 0. The UCOMISS/LAHF/`TEST AH,44h` idiom sends NaN down the compute path.
2. A negative value becomes `-0.0f - c` (00D7A208).
3. The result is `2^(2.2 * log2 c)` via FYL2X/FRNDINT/F2XM1/FSCALE (00CE89D8 = 2.2f), that is
   `c^2.2`, stored as a float.

The host computes in double; the image keeps 80-bit intermediates, so the last bit can differ.
Its callers: 004ECA30 (three calls and the tail jump at 004ED2C0), 004EA870 (7), 004EB9B0
water (9), 004EB180 (1) and `BSP_TacticalLibraryPreview_SetupEnvironment` (5).

### Defaults

Defaults come from `BSP_SceneEnvironmentBlock_ConstructDefault` 004CB420, run by the record
constructor `BSP_SceneRecord_Construct` 004DA2A0 at 004DA342 and as the no-g_Weather temporary.
The cloud lights come from `BSP_CloudLightColours_ConstructDefault` 004B6980, run by the record
constructor at 004DA419.

| field | default |
|---|---|
| IsRain | 0 |
| RainDropNum | 1400 |
| RainSpeed | (6, -12, 3) |
| RainColor | dword 80D0D0FFh |
| RainDropLength min/max | 0.15 / 1.05 |
| FogColor | (0.644, 0.64, 0.667, 0) |
| DirFogColor | (0.4, 0.4, 0.4, 0) x4 |
| LinFog density / start / end / height density / height end / UW height end | 0.7, 100, 7500, 0.7, 200, -120 |
| all six clips | 30000 |
| block +100h (BushShadowAllowed) | 1 |
| CloudsAffectedByWind | 1 |
| CloudLights | Ambient (0.2, 0.2, 0.2, 0), Sun (0.8, 0.8, 0.8, 0), Bottom (0.1, 0.1, 0.3, 0) |

`SceneWeatherFields` carries these as member initializers.

## 004EA870 `BSP_SceneRecord_ReadSunLightProperties` (g_Environment.Lights)

| key (in Lights) | gate | record | treatment |
|---|---|---|---|
| `WaterTracerColor.R/G/B/A` | presence test; child not null-tested | C44h..C50h | rgba, then linearize rgb |
| `ParticleMultiplier` | required | 1054h | number |
| `BushShadowAllowed` | required | A90h (block +100h) | byte |
| `AmbientLumScale`, `AmbientLumOffset` | required | locals | number |
| `BushDiffuseFactor` | required | foliage 00AD7140 | number |
| `BushTextureName` | required, string | foliage 00AD7FE0 (by value) | |
| `BushNormalAllowed` | required | foliage 00AD5730 (stores foliage+11h) | the image pushes the whole stack dword, of which only the low byte is set |
| `Ambient.R/G/B/L` | required | F48h | a = L*scale + offset, then linearize |
| `UWAmbient.R/G/B/L` | required | F58h | rgb and the old alpha times L, then a = 1.0, then linearize |
| `AmbientCube.HDR.Left/Right/Top/Bottom/Front/Back` | all required, looked up before any face is read | F68h + i*10h | rgb, a = L*scale + offset, linearize each |
| `Diffuse.R/G/B` | required | FC8h, a = 1.0 | linearize; FFCh = 1.0, or `Diffuse.Luminance` when present and linearizing |
| `UWDiffuse.R/G/B/L` | required | FD8h | as UWAmbient |
| `Specular.R/G/B` | presence test | FE8h | linearize; when absent: 0.6 grey (00CE3D30), not linearized. FF4h = 1.0; 1000h = 1.0, or `Specular.Luminance` when present and linearizing |
| `Angles.HorizontalAngle`, `VerticalAngle` | required | 1004h, 1008h | `deg * pi / 180` (double constants 00CE3D28, 00CE3D20) to float; 100Ch and 1010h get the same two values |

F48h..100Bh is the lighting configuration that `include/bsp/lighting_configuration_apply.hpp`
(`LightingConfigurationFields`, consumer 004BACD0) already models. In that header,
`second_angle_1004` and `first_angle_1008` are the horizontal and vertical angles.

**Native hazard (004EB031).** When `Specular` is absent, the `Luminance` lookup takes
`ESI = [ESP+20h]`. That stack dword holds the BushNormalAllowed byte at 004EAA3A, with stale upper
bytes, so the image would treat a byte as a bag pointer. Every Lights block in this installation
authors `Specular` (survey below), so the path is unreached. The host refuses it with an error
rather than invent a bag.

When the g_Environment bag has no `Lights` sub-bag, 004EE4AA copies a fresh 004B6800 object over
F48h. `BSP_LightingConfiguration_ConstructDefault` sets these fields:

| offset | default |
|---|---|
| +0h, +10h | (0.5, 0.5, 0.5, 1) |
| +80h, +90h, +A0h | (1, 1, 1, 1) |
| +B4h | 1.0 |
| +BCh, +C0h | 0.6108652 (35 degrees, 00CE74F4) |

It does not write the cube (+20h..+7Fh), +B0h or +B8h, so the REP MOVSD copies stack garbage into
them. The host zeroes them.

## 004EE450 `BSP_SceneRecord_ReadSkyAndLights` (g_Environment)

The reader runs these steps in order:

1. 004E7220 reads the cloud areas from the same bag.
2. `Lights` (type 6) goes to 004EA870. Otherwise the 004B6800 reset above runs.
3. `Skybox` (presence test, then type 2) is assigned to C54h. When linearizing, C54h becomes
   `"HDR_" + C54h`, whether or not Skybox was present. The record constructor's default is
   `sky_001` (00CE7AB0).
4. `SkyboxRotate` and `SkyboxLuminance` are stored to C5Ch and C60h, each only for type 1 (F).
5. If `ReflCubemap` is present, B98h = the name. BA0h = `substr(name, 0, size-4) + "_ocean" +
   ".dds"`. The count is unsigned; 00469840 clamps it.
6. When linearizing, B98h and BA0h both get the `"HDR_"` prefix, whether or not ReflCubemap was
   present.
7. `Sun` (type 2) is assigned to 1044h and `ThunderStorm` (type 2) to 104Ch. These are effect
   names: 004DE9BD/004DEAF6 build effects from them when they are non-empty (00871BA0, game+19D4h
   and game+19DCh).

## 004ED690 `BSP_SceneRecord_ReadMusicAndAmbience` (Sound)

- **Environment sounds.** `Ambient` (type 6): for Ocean, Air and Underwater (index 0..2), 004ED530
  looks up the key. The entry must be type 6 with a non-empty string `FileName`; otherwise the
  slot is left alone, distances included. When it is, record D60h + i*14h becomes
  `"sound/environment/" + FileName`. `Distance` goes to D68h + i*14h and `Distance2` to
  D6Ch + i*14h, each only for type 1 (F).
- **Music.** `Music` (type 6) holds `Party0` and `Party1` (each optional, type 6). For each of the
  nine states `Default, Tension, Action, Victory, Defeat, Calm, Custom1, Custom2, Custom3`,
  004ED2D0 runs with slot = party*9 + state:
  1. It clears the path at D98h + slot*18h and the channel at DA0h + slot*18h.
  2. If the state entry is a type-6 bag: a non-empty `FileName` sets the path to
     `"sound/music/" + FileName`. A non-empty `Channel` sets the channel. A non-empty
     `SoundEffect` is acquired from the sample cache `[00F8BBE8]` (00A83FD0) and assigned into
     DA8h + slot*18h (004E7BB0).
  3. It always calls 00A79480 on `[00F8BBCC]` with (&path, party, state). That call stores into
     the manager's slot table `+1C0h + (party*9+state)*4` (body read up to the slot store; the
     stream construction is left unread).

## Consumers of the fields

The census scanned the disp32 displacements of every record offset above across `.text`, with a
capstone decode at each byte hit (`local/wx1_disp.py`). It kept the functions that also load
`+5FCh` or pass record+990h. It then followed the block pointer that 004DE610 hands out
(004DF878, 004DF88F).

| field(s) | consumer | use |
|---|---|---|
| A10h FogColor, A20h DirFogColor, A60h..A70h | `BSP_WorldAtmosphere_ApplyPreset` 00865900 (block pointer, from 004DF890) | SystemFogOwner setters 00B84D00..00B84D40, 00B84C40 and 00B84FA0. 00B84D00 gets +D0h, 00B84D40 +DCh, 00B84D10 +D4h, 00B84D20 +D8h, 00B84D30 +E0h. The camera clear colour comes from FogColor (004FB850, then 00B6FE50). |
| A20h DirFogColor | `BSP_Game_ConstructWorld` 004DE610 at 004DF781..004DF7A3 | 00B84FA0 per face |
| A10h..A74h | 0078C9B0 (block pointer, from 004DF87F) | the authored-fog copy, `docs/AUTHORED_FOG_SOURCE.md` (block 80h..117h) |
| 9F0h..A0Ch, BECh | 00865900 | when IsRain is set: rain descriptor +20h..+28h speed, +0Ch drop count as float, +18h min length, +1Ch max - min, +34h colour, and the emitter 008023B0 at +38h. BECh goes to the cloud manager's wind switch (00BBCE60). |
| A78h ObjectRenderClip | 004DE952 | `[game+19FCh]+178h` (a camera's clip distance) |
| A7Ch ReflectRenderClip, A80h RefractRenderClip | 0078C9B0 at 0078CEF1/0078CF03 | `[ocean+C8h]+178h` and `[ocean+D0h]+178h`; ocean = `[world+A8h]`, EBX at 0078CCE3 |
| A84h ShadowRenderClip | 004DF3FD | `BSP_Light_GetShadowMapOwner([game+19F8h])+8` |
| A88h, A8Ch foliage clips | 004DE96E, 004DE989 | `BSP_FoliageSystem_SetScalar08` / `SetScalar0C` on `[00F8C210]` |
| A90h BushShadowAllowed | 004DE9A4 | foliage system (byte) |
| 1014h/1024h/1034h CloudLights | 004DF495/004DF4D1/004DF50D | cloud manager colours: Ambient to 00BBCD90 (+70h), Sun to 00BBCDE0 (+80h), Bottom to 00BBCD50. All four dwords are passed, so alpha stays at the constructor's 0. |
| C54h, C5Ch, C60h | 004DF457, 004DF549, 004DF56B | sky creation 00BBDFF0 and the sky rotate/luminance setters (00BBF1A0 takes degrees) |
| 1044h, 104Ch | 004DE9BD, 004DEAF6 | effect handles at game+19D4h / +19DCh |
| F48h..100Bh | 004BACD0 `BSP_Lighting_ApplyConfiguration` | `include/bsp/lighting_configuration_apply.hpp` |

The fog scalar getters 00B84CA0/00B84CB0/00B84CC0 and 00B84C90 are read only by the shader
constant upload 00B46A70 and the render-target routine 00B83F30 (no callers).

**Gameplay consumers: none found** in this scope. The scope was the direct record displacement
scan, the two block-pointer consumers, and the fog getters' callers. Nothing in ship, plane, AI,
recon or visibility code reads these offsets or the fog getters. Not covered: readers of the
SystemFogOwner fields themselves (+68h..+90h) outside the getters, and readers of the rain
descriptor singleton 00F875C0 other than 00865AB0.

**For cc9 (RNG, not a consumer).** In the image, the rain these fields enable draws from the
shared generator. When IsRain is set, 00865900 creates the rain emitter at descriptor+38h.
`BSP_RainDescriptor_Update` 00865AB0 then runs from `BSP_Game_OnMove` every frame, and
`BSP_RainEmitter_SpawnStep` 00801400 draws 00BD2F10 (stream 0) at least once per call (already
listed in `docs/CC13_RNG_STREAM_CENSUS.md`). On rainy missions the image's stream-0 sequence
therefore contains rain draws that a host without rain lacks. This installation has 18 scenes
with `IsRain = B true` (the survey below), among them ijn_04_coral_sea,
ijn_08_defend_guadalcanal, ijn_10_sydney, usn_02_battle_of_cape_esperance, ijn_16_aleutians,
usn_19_coralus, usn_8_cape and 07_Komandorski. Per the RNG rule, if the host ever wires rain, it
must use a presentation stream and record that the image shares stream 0.

## Corrections to existing provisional names

Neither file below belongs to this packet; this section only records the corrections.

- `include/bsp/world_construct.hpp` names several record offsets from the consumer side. The
  reader settles them:
  - `kSceneRecordFogEntryOffset` A20h is DirFogColor (Right, Left, Front, Back).
  - `kSceneRecordCameraNearOffset` A78h is ObjectRenderClip (camera +178h), not a near plane.
  - `kSceneRecordLightScalarOffset` A84h is ShadowRenderClip.
  - `kSceneRecordFoliageDensityOffset`/`RangeOffset` A88h/A8Ch are xFoliageRenderClip and
    xFoliageLowResDist.
  - `kSceneRecordFoliageEnableOffset` A90h is BushShadowAllowed.
  - `kSceneRecordOceanDescOffset` C54h is the Skybox name, and `kSceneRecordOceanScalar0/1` C5Ch
    and C60h are SkyboxRotate and SkyboxLuminance.
  - `kSceneRecordOceanVector0/1/2` 1014h/1024h/1034h are CloudLights Ambient/Sun/Bottom.
  - `kSceneRecordTerrainHandleOffset` 1044h is the `Sun` effect name, and
    `kSceneRecordShoreHandleOffset` 104Ch is the `ThunderStorm` effect name.
  - `kSceneRecordTerrainVectorOffset` 100Ch is the horizontal-angle copy; 1010h is the vertical
    copy.
- `docs/CC13_FOLIAGE_TERRAIN_SETUP.md` says "no store to +A88h was found". 004ECA30 stores both: 004ECE34 writes A88h from
  xFoliageRenderClip and 004ECE6A writes A8Ch from xFoliageLowResDist.

## Host plug-in (description only; no host edit)

The host reads the scene header in `SetPendingSceneHost::load_scene_header_pass`
(`src/game_hosts_mission.cpp`, 0046DF00). After `parse_scene_document` it calls
`read_scene_record_slot_table_004f1d70` on `document.header.properties`. That file is cc9's
mission-load lane, so no diff is offered. A wiring would follow these steps:

1. Build `scene_bag_from_block(document.header.properties)` once.
2. Find `g_Weather` and call `read_weather_fog_and_cloud_lights_004eca30`. If g_Weather is absent,
   keep the default `SceneWeatherFields`.
3. Find `g_Environment` and call `read_sky_and_lights_004ee450`.
4. Find `Sound` and call `read_music_and_ambience_004ed690`, only on the path where 004F1D70's
   flag byte is set.
5. Pass `GameSettings::shader_flag_8c` as `linearize`.

The consumers in the table above, which are presentation-side in `BSP_Game_ConstructWorld`, would
then read the structs. The rain emitter must stay off unless it gets its own RNG stream.

## Survey of this installation (read-only)

`local/wx1_survey.py` walks the 259 `.scn` files under `universe/scenes`:

- **g_Weather.** 258 files have it, and every one of them authors every key in the 004ECA30
  table. `CloudsAffectedByWind` is true in 211; `IsRain` is true in 18.
- **Lights.** All 258 Lights blocks author Specular, Specular.Luminance, Diffuse.Luminance,
  AmbientCube.HDR (six faces) and Angles.
- **Unread keys.** One file carries extra `*_PC` / `*_PS2` / `*_XBOX` variants and
  `ForceDisableDynShadow` (258 files) is present. 004EA870 reads none of these.

## Open questions

- 004ED690 runs only when the byte at `[ESP+2ACh]` in 004F1D70's frame is set. Which argument
  that is, and which callers set it, was not read.
- 00A79480's stream construction past the slot store is unread.
- The meaning of camera +178h is inferred ("clip distance") from the three keys that feed it.
- What writes record+90Ch, the string the terrain LUT name comes from (004DFB70 also takes its
  address), was not read.
- 004EB5A0 is a separate routine that Ghidra folds into 004EB9B0's body (body 004EB5A0-004ECA23,
  entry 004EB9B0). It was left undefined; defining it would split that body.

## 004EB9B0 `BSP_SceneRecord_ReadWaterProperties` (g_Ocean)

`__fastcall(ECX = g_Ocean bag, EDX = record)`, caller 004F2205. The record offsets equal
`water_render::block` + 990h for every key; `include/bsp/water_render.hpp` already names them, and
`SceneOceanFields` in the header lists them with their record offsets.

| section | gate | keys and treatment |
|---|---|---|
| `0_SimpleOcean` | required; stored only for type 3 | C20h byte |
| `Optics` | required; skipped unless type 6 | the tints `1LocalReflectionTint` AE4h, `1ReflectionTint` AF4h, `1RefractionTint` B04h, `1UWRefractionTint` B14h, `RefrSpecularColor` B74h (rgb, child unchecked, linearized when 00F88A0C) and `1SprayTintColor` B30h, which is linearized **always**: 004EBB21 calls 004E5C00 with no flag test. `7SkyRefl*` AD4h..AE0h, the `2*`, `9ShadowStrength`, `3*`, `4Fresnel*`, `5ShoreFade*`, `UnderWaterRefrIndex`, `6ReflectionMultiplier` and `6FinalMultiplier` are required numbers. `8UWBaseGlow/SkyGlow/SunSpotStrength` (B24h..B2Ch) are optional. |
| `Waves` | required; type 6 | WindDirection 9DCh, WindSpeed 9E0h, WaveHeight 9E4h, ChoppyWavesFactor 9E8h, HeightScale 9ECh, Falloff 9BCh, LayerScale 9C0h, TimeScale 9C4h, `1WaveDampeningDepth/Power/Scale` 9C8h..9D0h, `1WaveDampModel` string 9D4h; all required |
| `NormalMaps` | required; type 6 | `layer0..2_Scale` BF0h.., `layer0..2_ScrollSpeed` C00h.., `Texture` C10h; required |
| `Foam` | required; type 6 | UseFoam 99Ch byte, ScrollSpeed 990h, NoiseScrollSpeed 994h, Height 9A0h, FadeRange 998h, Scale 9A4h, NoiseScale 9A8h, Texture 9ACh, NoiseTexture 9B4h; required |
| `Caustics` | optional; child unchecked | `CausticsColor` (presence) ABCh rgb, linearized; `CausticsScale` AB8h and `CausticsType` (string ACCh) optional |
| `Fog` | optional; a null child skips to WaterParticle | `UWExpDistMaxdist` A94h, `UWExpDistMinvalue` A98h x0.01, `UWExpHeightCutdepth` A9Ch, `UWExpHeightMaxdepth` AA0h, `UWExpHeightMaxvalue` AA4h x0.01, each optional (the x0.01 is `FMUL qword 0.01` (00CE6638) on the x87). `UWFogColor` (presence, null child skips): rgb, then 004E5D20 multiplies all four floats (including the old AB4h) by L, then `MaxDarken` overwrites AB4h, then linearize. |
| `WaterParticle` | optional; type 6 | 004EB180 into the object at [record+C18h] |
| `Beam` | optional; type 6 | tail jump at 004ECA1B to 004EB5A0, into the object at [record+C1Ch] |

The two particle-set readers share one layout (`SceneParticleSetFields`):

| offset | content |
|---|---|
| +0 | count (`NumParticles` or `NumBeams`, raw dword) |
| +4 | `Speed` |
| +8 | `TextureFile` string |
| +10h | `Color` RGBA, rgb linearized when the flag is set |
| +20h..+5Ch | `Tex00`..`Tex15`, read as numbers |

In 004EB180 only, Tex04 is halved (004EB3C0 `FMUL qword 0.5`). `BSP_Color_ScaleRgbaByScalar`
004E5D20 is `__thiscall(float* rgba, float s)`, RET 4. Its register forms are DE CA and D8 C9 per
the bytes, so it is four plain products.

## 004EE9E0 `BSP_SceneRecord_ReadStaticShadowAndShaderTextures` (g_Terrain)

`__fastcall(ECX = g_Terrain bag, EDX = record)`, caller 004F232A. The scalars do not go into the
record. They go to terrain shader globals at 00E18808..00E18988, which are read by
`BSP_Landscape_BindTerrainMaterialParams`, `BSP_MapScene_SetupOceanMaterials`,
`BSP_MapScene_BuildQuad` and 0078C9B0 (00E1891Ch). `BSP_SceneEnvironmentBlock_ConstructDefault`
004CB420 writes their defaults.

- **Optional scalars.** There are 26 (presence test, then a number). `kTerrainScalars` in the
  source maps each key to its global; examples: `a_UW_Coral_Sand_Slope` 00E188F8,
  `d_Shore_Height` 00E18928, `f_NmPower_Rock` 00E18918.
- **`h_UW_Color0..2`.** Each lookup is required; a null child skips that colour. The rgb goes to
  00E18960 / 00E18970 / 00E18980. It is not linearized.
- **Subtype groups.** Each group has five required numbers (`x0_width, x0x1_delta, x1_width,
  x1x2_delta, x2_width`), multiplied by 0.01 after all five are stored (`FMUL ST,ST(1)`, D8 C9).
  - `i_UW_subtypes`: `coral_subtypes` to 00E18894 and `rock_subtypes` to 00E1886C.
  - `i_GROUND_subtypes`: `grass_subtypes` 00E18830, `dirt_subtypes` 00E1881C and `rock_subtypes`
    00E18808.
  - Each `i_*` child is null-tested.
- **Quirk at 004EF2D3.** `i_SHORE_subtypes` is only tested for a non-null child. The values for its
  slot (00E18844) are then read from `rock_subtypes` of the **i_UW_subtypes** bag that is still in
  EDI. All 258 terrain blocks in this installation author both. Their shore rock values differ
  from the UW ones (r0r1_delta is 25 in 180 files against 40 in 118 for UW), so the authored shore
  values are never used. If UW is absent and SHORE is present, the image looks the key up in a
  null bag; the host refuses that case.
- **Textures.** `g_DiffAtlas` BC4h, `g_NormAtlas` BCCh, `g_MaskTexture` BD4h, `g_NoiseTexture`
  BDCh and `g_StaticShadowTexture` BE4h are optional. When present, each must be a string; the
  image takes strlen with no null test.
- **Static shadow shot.** `ga_StaticShadowShotSize/OffsetX/OffsetZ` (optional) go to
  00E1891C/20/24.
- **LUT name** (004EFB2E..004EFEB6). The source is `[[00E188A8]+5FCh]+90Ch`: the game's current
  record, not EDX. If it is empty, the global string 00E18990 is left alone. Otherwise:
  1. Cut through the last `/` (00CE7898), or else the last `\`, together with the 4-character
     extension: `substr(i+1, size-i-5)`.
  2. Cap the result at 14h characters.
  3. Store `"LUT_" + name + ".tga"` in 00E18990.

  004ED910 publishes the result as `TerrainLUT`.

## 004ED910 `BSP_SceneFile_ApplyTerrainTextureHeader`

`__fastcall(ECX = record)`, RET. Caller `BSP_SceneFile_ReadHeaderBlock` 00469BF0 at 00469D53,
right after 004F1D70 and only when a stack flag byte of 00469BF0 is set. It is a publish step, not
a bag reader:

1. **World parameter strings** (00B1B830 on [00F8D434]), in this order:
   - `ReflectionCubeMap` (B98h) and `CloudShadowMap` (BA8h)
   - `DiffAtlas_A` (BC4h) and `DiffAtlas_B`, which also gets **BC4h**: 004EDAE6 pushes the same
     address
   - `NormAtlas` (BCCh), `MaskTexture` (BD4h) and `NoiseTexture` (BDCh)
   - `TerrainLUT` (global 00E18990) and `StaticShadowTexture` (BE4h)
2. `[00F8D394]` vtable+120h, contract unread.
3. **Environment sounds.** For each slot at record D5Ch + i*14h: when the sample handle at +0 is
   null and the path at +4 is non-empty, acquire the sample (00A83FD0) into the slot, then call
   00A7F2F0 on [00F8BBD8] (slot, i).
4. **Aliases.** `BSP_VFS_ClearAliases`; then, when `FileSystemLua` (record+918h, written by
   004F1D91) is non-empty, run that Lua file and call `BSP_VFS_AddAlias(name, newname)` for each
   `Files[i]` (i = 1 while non-nil).
5. `BSP_FoliageSystem_RefreshBushTextures`.

## 004E8510 `BSP_SceneRecord_ApplyPostEffectProperties` (g_PostProcess)

`__fastcall(ECX = g_PostProcess bag, EDX = record)`, caller 004F2349. `HDR`, `MotionBlur`,
`UnderWaterWave` and `Colorize` are required keys, each gated on type 6, with every inner key
required.

| block | setter | contents |
|---|---|---|
| HDR | 00B0E9B0 | BrightPassTreshold, BrightPassOffset, MiddleGray, `1 - AdaptationPercent/100` (x87 FDIV qword / FLD1 / FSUBRP), BloomScale, Min/MaxLuminance, NewHDRParam0, NewHDRParam1 (read 1 before 0), AntiBandingON byte, ParticleLuminanceMul |
| MotionBlur | 00B0EA20 | MaxVelocity, VelocityScale, MinSpeed, Divider (+0Ch), MaxBlur (+10h), three zero floats |
| UnderWaterWave | 00B0EA70 | Horiz/Vert Speed, Scale and Phase, and the two amplitudes divided by 1000 |
| Colorize | record | TextureName C24h, TextureNameUW C2Ch, TextureNameOldFilm C34h, TextureNameOldFilmUW C3Ch |

All three setters take ECX = [00F8D39C]. The stack defaults that the image builds first are the
member initializers. A successful read overwrites every one of them.

## Second-packet probe and consumers

The probe (`local/wx1_probe2.cpp`, not committed) ran over the 259 files: g_Ocean, g_Terrain and
g_PostProcess each read without error in 258 files. `ijn_02_force_z.scn` lacks all three. The
SHORE-subtype quirk is hit in all 258 files. Every file has both a WaterParticle and a Beam set.
One sample was checked by hand: wind 0.4/1, WaveHeight 150, NormalMaps texture
`oceannormalmap08.tga`, 20000 particles with Tex04 0.05, and HDR adaptation 0.95.

The consumer census used the same displacement scan as the first packet:

- **Gameplay (cc9).** `0_SimpleOcean` (C20h) and the `Waves` keys feed the ocean wave field and
  `BSP_OceanWaveField_SampleHeight` 0078C890, the water height used by ship motion.
  - `0_SimpleOcean`: 004DF7E1 passes it to the world constructor 0078DAA0 as the flat-sea byte
    field+F9h.
  - `Waves`: 0078C9B0 hands them to the field (WindDirection/WindSpeed 00B9A450, WaveHeight
    00B96C50, LayerScale k = field+B4h, HeightScale field+B8h, Falloff/TimeScale +18h/+20h).
  - `docs/OCEAN_WAVE_FIELD.md` (cc9) already reconstructs the field and notes that the host does
    not parse these keys. `read_water_properties_004eb9b0` now provides that parse. No cc9 file
    was edited.
- **Presentation.** Everything else:
  - `1WaveDampening*` goes to the ocean shader parameters through 00B9A0D0 (ocean +5E0h..).
  - The optics go through the 00B99E60.. setters, and the caustics scale through 004DEF1F.
  - The particle and beam objects are reached through 004DF8A2/004DF8C1.
  - The terrain globals feed the landscape/map material binders.
  - The post-process blocks go to the renderer service.
  - The Colorize strings have no direct displacement reader.
- **RNG.** None of these readers draws. Ocean motion consumes no RNG in these routines.

## Handoff

Both packets are done. Remaining leads: the 004EB5A0 function definition (left to the
integrator), the writer of record+90Ch, and the `[00F8D394]` vtable+120h call in 004ED910. Leases
of both packets are released at the end.
