# The ocean wave field behind the water height

Packet cc9_ocean_waves, 2026-09-23. Base: main e1d1c0494. Reconstruction:
include/bsp/ocean_wave_field.hpp and src/ocean_wave_field.cpp. Host seam: the water-height
binding in src/game_hosts_units.cpp, behind `kOceanWaveFieldBound`, which is on. Status:
reconstructed, build-tested and run-compared (section 5). Not ABI-compatible and not
game-validated. Names are hypotheses.

Addresses: 0078C890 BSP_OceanWaveField_SampleHeight, 00B9CF50 BSP_OceanWaveField_CoverageMask,
00B960C0, 00BA6FB0, 00BA6C40, 00BA1400, 0078DAA0, 0078C9B0, 00B9A4C0, 00B9A450, 00B96C10,
00B96C50, 00B96B90, 00B95750, 00B95D30, 00B95AA0, 00B96C80, 00BA0C00, 00BA1370, 004CB420.

## 1. The field

**Construction.** The world constructor 0078DAA0 calls the factory 00BA6FB0 (ECX = its first
argument, DL = its second byte). The factory allocates 6BCh bytes (00BA6FC8) and constructs them
with 00BA6C40, which first runs the base 00BA1400. The result is stored at world+A8h (0078DB03).

The DL byte becomes field+F9h, the flat-sea byte that 0078C890 tests. The game constructor
passes [[game+5FCh]+0C20h], a byte of the mission's scene record, or 0 when there is none
(004DF7E1 / 004DF7FE).

**Sampling.** 0078C890 (docs/OCEAN_HEIGHT.md) computes:
- u = frac(float(k·x)) and v = frac(float(k·z)), with k = field+B4h and the fractional parts
  taken against the 00BF85B0 floor in double
- h = 00B960C0([field+BCh], u, v)
- the result float(h · [field+24h]), with the multiply at 0078C92D

**The grid, 00B960C0.** `float __thiscall(spectrum, float u, float v)`, RET 8, body
00B960C0..00B962D4:
- u and v are scaled by the double 63.0 at 00D099A0, floored, and clamped to cells 0..3Fh
  (00B96140)
- each +1 neighbour wraps with `AND 8000003Fh`
- the four cells are blended bilinearly in lerp form
- a cell is 8 bytes of the vector at spectrum+20h/+24h, and its first float is negated (a SUBSS
  from the −0.0 at 00D7A208)

**Where the heights come from.** The grid is simulated at runtime. No installation file holds it.

| Routine | What it does |
|---|---|
| 00B95750 | Seeds a local Mersenne state (BSP_RandomState_Seed(1105h) on its roughly 9D8h-byte stack frame), so it does not couple to the game's random streams. Builds the initial spectrum from the wind parameters. |
| 00B96C10, 00B96B90 | Set the wind direction (cos and sin at +6Ch/+70h), the speed at +74h and +80h..+8Ch, then rerun 00B95750. |
| 00B96C50 | Sets +80h and +88h, then reruns 00B95750. |
| 00B95D30 | Gated by the byte at 00E130B4, statically 1 and never written. Advances the complex spectrum for the 32×65 half-plane (loop to 820h in steps of 41h). |
| 00B95AA0 | Runs the inverse FFT over the vectors at +1Ch and +2Ch. |
| 00B96C80 | The vtable slot at 00D63900 that drives 00B95D30 each step. |

The parameters come from the scene record's +990h environment block, applied by 0078C9B0:
- +4Ch (scaled by the double at 00CE3D28) and +50h go to 00B9A450, then 00B96C10
- +54h and +58h go to 00B96C50
- +5Ch goes to field+B8h
- +2Ch, +30h and +34h go to 00B9A4C0, which stores +18h, +1Ch, +20h, +B0h = the tile, and +B4h
  = 1/tile (the FDIVRP at 00B9A4D2)

The constructor 004CB420 defaults these to +2Ch = 0.6, +30h = 100.0, +34h = 1.0, +4Ch = 0.5,
+50h = 64.0, +54h = 80.0, +58h = 0.04 and +5Ch = 1.0.

**Corrected 2026-10-11 (section 8): the conclusion of the next three paragraphs is wrong.**
00BA0090 stores field+24h = field+B8h (Waves.HeightScale) on every in-mission render frame
(00BA0B90 `FLD [ESI+2Ch]`, 00BA0B93 `FSTP [ESI+24h]`), so the image's CPU sea is not flat. The
census below missed a store that lies inside its scanned band. They are kept for history.

**The amplitude.** field+24h is stored once, as 0.0, by the base constructor (00BA19C6 `XORPS`,
00BA19C9 `MOVSS [ESI+24h]`). I searched for every other store to it. Every encoding below was
scanned over 00B98000..00BA7FFF. The FSTP and MOVSS forms were also scanned over 00BA8000..00BBFFFF
and in 0078xxxx, 004Dxxxx and 004Exxxx. The encodings:
- FSTP and FST [reg+24h] for EAX, ECX, EDX, EBX, EBP, ESI and EDI
- MOVSS [reg+24h] from xmm0..xmm7
- MOV dword [reg+24h] from any register, and with an immediate
- ESP-SIB stores, and SIB stores at +18h and +24h
- LEA reg,[reg+24h]

None writes the field. The hits were vertex fills and copies to other objects (EAX, EDX, EDI
receivers), the region builder 00BA0C00, and the 0078CBF8 environment copy (EDI = the
environment). Therefore **0078C890 returns h · 0.0 in the image**, a signed zero for any finite
grid sample. The CPU-side sea that ship motion samples is flat. The rendered waves come from the
same spectrum through the renderer's own path.

This negative is bounded by the encodings listed. A block copy into +18h..+2Ch or a store
through a containing object's larger displacement would not be found by this census.

## 2. The coverage mask, 00B9CF50

The rule is traced through the x87 stack:
- **Region test.** The first region with x0 ≤ x, z0 ≤ z, x ≤ x1 and z ≤ z1 (00B9CF73..00B9CF90;
  +10h, +14h, +18h, +1Ch).
- **Indices.**
  - col = unsigned-min(trunc(float((x − x0)·sx·W)), trunc(W − 1))
  - row = unsigned-min(trunc(float((1 − (z − z0)·sz)·H)), trunc(H − 1))
  - sx and sz are +20h and +24h, W and H are +28h and +2Ch; the truncation is FISTP under
    RC = truncate
- **Value.** The byte at [region+4h] + row·trunc(W) + col, divided by the double 255.0 at
  00CE4B48, stored to float, and clamped to [0, 1].
- **No region.** FLD1 at 00B9CFA0.

The region array at field+620h/+624h is built by 00BA0C00, which appends 30h-byte records and is
called from 00BA1370, from coverage bitmaps. The host does not load those bitmaps. The mask
multiplies the wave height in 0078CF20, so with a zero amplitude it cannot change the result.

## 3. Host against image

| Term | Image | Host |
|---|---|---|
| +F9h | [[game+5FCh]+0C20h] | clear; the host builds no scene record (LABELLED) |
| +B4h | 1/authored +30h | 1/100, the 004CB420 default (LABELLED) |
| +24h | 0.0 (00BA19C9) | 0.0 |
| grid | the FFT spectrum at +BCh | a labelled zero grid |
| regions | 00BA0C00 from the bitmaps | none |
| 0078C890 and 00B9CF50 | the rules above | the same rules, now `concrete` |

Since the amplitude is 0.0 in both, the host's water height equals the image's: ±0.0
everywhere. **Superseded by section 8:** the image's +24h is HeightScale after the first
in-mission render frame, and its grid is live; the host row is still 0.0 and a zero grid.

## 4. Predictions (local/ow_predictions.txt, written before any run)

- The height is ±0.0 at every ship and every step.
- Pitch, roll, heave, the keel point and the throttle gate are unchanged.
- Gunnery is unchanged, with no random-number coupling.
- Only the method table moves, by about 2.0M calls.

## 5. Measurement

The first attempt, at 10:55 on 2026-09-23, crashed at the renderer init request on every binary.
That was an environment fault, and the pair was run again after 11:05. Runs used
`BSP_GUNNERY_RNG_STREAMS=1` on both sides:
- the control, build/win32/owC, is the same source with `kOceanWaveFieldBound` false
- the treatment is build/win32/owT
- USN04 ran at 4700/4500 (local/ow_ctl_usn04.log, local/ow_trt_usn04.log)
- USN01 ran at 3200/3000 (local/ow_ctl_usn01.log, local/ow_trt_usn01.log)

With heap addresses, thread ids and the module path masked, the only other difference in either
pair is the ship-avoidance refills counter (58 against 57 on USN01), which is ignored. Every
ship-AI step, impact, death and motion line is identical. So the prediction held: the height is
±0.0 everywhere and nothing moves.

| | USN04 control | USN04 treatment |
|---|---|---|
| queued hits / damage / deaths | 238 / 18929.4 / 24 | 238 / 18929.4 / 24 |
| ocean_wave_field, ocean_coverage_mask | 1,001,274 calls each, unimplemented | 1,001,274 calls each, now implemented |
| unimplemented calls in total | 5,947,155 | 3,944,607 |

USN01's two ocean sites, 408,105 calls each, are also now implemented. The USN04 unimplemented
total falls by 2,002,548.

Decision: landed. The binding is behaviour-neutral by construction and by measurement.

## 6. Open

- A runtime check that field+24h stays 0.0. Only an attached debugger on the retail game could
  confirm it, and this packet does not run the retail game.
- The scene record's +0C20h byte and the +990h block per mission. The host does not parse them.
  They matter only if the amplitude is ever non-zero.
- The FFT itself (00B95750, 00B95D30, 00B95AA0, 00B952D0). Reproducing the rendered sea would
  need it; ship motion does not.

## 7. Authored ocean data and the field's other readers (appended 2026-09-23, read-only)

### (a) Where the flat-sea byte and the wave parameters come from

The scene file's header properties reach the record through 00469BF0
BSP_SceneFile_ReadHeaderBlock, then 004F1D70 BSP_SceneRecord_ApplyHeaderProperties.

- **The ocean branch.** 004F2205 calls 004EB9B0, which reads named keys out of the header's
  property bag (BSP_ScenePropertyBag_Find):

  | Key | Written to | Then read by |
  |---|---|---|
  | `0_SimpleOcean` (type 3, a boolean) | record+0C20h | 004DF7E1, as field+F9h |
  | `Waves.WindDirection` | +9DCh, block +4Ch | 0078C9EC, 00B9A450 |
  | `Waves.WindSpeed` | +9E0h, block +50h | 00B9A450 |
  | `Waves.WaveHeight` | +9E4h, block +54h | 00B96C50, spectrum +80h |
  | `Waves.ChoppyWavesFactor` | +9E8h, block +58h | spectrum +88h |
  | `Waves.HeightScale` | +9ECh, block +5Ch | 0078CA0E, field+B8h |
  | `Waves.Falloff` | +9BCh, block +2Ch | field+18h |
  | `Waves.LayerScale` | +9C0h, block +30h | field+1Ch and +B0h, with 1/LayerScale at +B4h |
  | `Waves.TimeScale` | +9C4h, block +34h | field+20h |

  "Block" is the +990h environment block, so block +4Ch is record+9DCh. The remaining
  `1WaveDampening*` keys and the NormalMaps and foam keys feed the renderer.
- **The fallback.** 004F2222 looks up `g_Weather` (00CEA414). When it is present, 004ECA30 reads
  the fog, cloud and rain keys into the same block. When it is absent, 004CB420 builds a default
  block that 004F224C copies over record+990h.

**The format.** The installation's scenes are text property files: `Key = F 0.4000 ;` for a float
and `Key = B false ;` for a boolean. Paths are under universe/scenes/missions/.

**Survey of this installation.** A read-only parse of all 259 .scn files (local/scn_survey.py)
finds:
- every file authors the Waves block
- `0_SimpleOcean` is false in all 259, so field+F9h is clear everywhere
- there are 32 distinct combinations
- WindDirection 0.4 and WindSpeed 1.0 appear in every file
- WaveHeight ranges 50..250, ChoppyWavesFactor 1.3..3.5, HeightScale 0.5..3.0, Falloff 0.2..0.7,
  LayerScale 51..125, TimeScale 0.6..1.0

The two measured missions:

| Mission | WaveHeight | ChoppyWavesFactor | HeightScale | Falloff | LayerScale | TimeScale |
|---|---|---|---|---|---|---|
| usn_04_defend_guadalcanal.scn | 100.0 | 1.5 | 2.0 | 0.3 | 80.0 | 0.6 |
| usn_01_battle_of_eastern_solomons.scn | 100.0 | 3.5 | 2.0 | 0.3 | 125.0 | 0.6 |

The host's stand-in tile of 100.0 (the 004CB420 default) therefore differs from both, which author
80.0 and 125.0. At the image's amplitude of 0.0 the tile cannot change the sampled height. It
would matter only if +24h were ever non-zero. Binding it needs a .scn text reader for these keys.
The host does not read these keys today.

### (b) Whether any other CPU path reads the grid

A rel32 census of the executable on disk (local/rel32_scan.py) finds:
- **00B960C0**, the grid sample: one call, 0078C928. Nothing else samples the spectrum grid
  through it.
- **00B9A3D0**, the field+BCh spectrum getter: one call, 0078C9D0, the parameter set in
  0078C9B0.
- **0078C890**, the wave field: seven calls.
  - 0078CF3E, in 0078CF20
  - 0078D2A8, in 0078D1B0 BSP_GameWorld_BisectWaterCrossing
  - 00B9F163, in 00B9F0A0
  - 00BA365F and 00BA3691, in 00BA2FF0
  - 00BAB90C, in 00BAB3F0
  - 00BAB9E9, in 00BAB930
- **0078CF20**, the water height: 49 calls.

Every one of these paths ends in 0078C890's multiply by field+24h, so every CPU consumer sees the
same amplitude (HeightScale, not zero: section 8). The last four callers of 0078C890 sit in the water-effects module
(00B9..00BB, beside the water tracers). The rendered waves therefore cannot come through 0078C890.
They come from the spectrum vectors the renderer uploads, which is the renderer's contract and is
not bound here.

## 8. Correction: the CPU sea is live (packet cc9_physics1_sea, 2026-10-11, read-only inventory)

Base: main fa36636cd. Found by cc13 (docs/CC13_WATER_RENDER.md section 1); re-read here from the
disk bytes (`bsp.py disasm-raw`), because Ghidra bodies after `_free` calls are being restored and
live decompiles there are not trusted. Names are hypotheses.

### (a) The writer, every in-mission render frame

| Site | Instruction / call | Meaning |
|---|---|---|
| 004CA440 BSP_Game_Render | interface level 1 enqueues job B (docs/GAME_RENDER_FRAME.md) | only the 3D view runs it; levels 2/3 (map views) never do |
| 004B4820 | `mov ecx,[00E198C4]; call 0068A0D0` | job B |
| 0068A0D5..0068A0EC | `fld [00F876A4]`, push; ECX = `[[00E188A8]+19F0h]`; `call 0078CFF0` | the world, with the fixed-step mission clock |
| 0078CFF0..0078D1A6 | no branch skips the tail; `RET 8` | |
| 0078D180..0078D19D | `fld [esp+18h]` (the clock), `fmul qword [00D7A3A0]` (= (double)0.1f), ECX = `[EDI+0A8h]`, push camera, `call 00BA0090` | t = float(clock * 0.1) |
| 00BA00A8..00BA00BA | `[01090494] = float(t * 10.0)` (00CE3DC0) | the ocean clock global |
| 00BA00C0..00BA00DA | ECX = field+BCh (the spectrum); `call [vtbl+8]` with float(t * field+20h) | step; +20h is Waves.TimeScale |
| 00BA00DC..00BA00E7 | `call [vtbl+18h]` | 00B96E80, a bare `RET` |
| 00BA0B32..00BA0B6D | +2Ch = +B8h (MOVSS); +28h = +30h = float(+B8h * [spectrum+88h]) | cWaveScale |
| 00BA0B8B | `call 00B9F0A0` | reflection pass; its 0078C890 call (00B9F163) still sees the old +24h |
| **00BA0B90 / 00BA0B93** | `FLD [ESI+2Ch]` / `FSTP [ESI+24h]` (D9 46 2C / D9 5E 24) | **amplitude = HeightScale** |
| 00BA0BF1 | `RET 8` | two stack arguments (camera, t) |

The listing 00BA0090..00BA0C30 has no jump or return between the entry and 00BA0B93 that leaves the
function (every Jcc lands below 00BA0B1C), so the store is unconditional per call.

**Order against the simulation.** GGame::OnMove (004E4A40) runs the simulation, including the
fixed-step driver 00875BB0, and only then BSP_Game_Render (004E54BC, docs/GAME_RENDER_TAIL.md).
So the fixed steps of frame N read the grid and amplitude that frame N-1's render produced, and
the clock the render passes is 00875BB0's end-of-frame value (00875F26 adds the leftover
accumulator back, so it is not a multiple of the step). Before the first level-1 render frame the
amplitude is the constructor's 0.0.

### (b) The spectrum object (field+BCh)

- Allocated by the world constructor, not the field: 0078DB09 `new(90h)`, 0078DB20 `call 00B97100`
  (BSP_OceanSpectrum_Construct, vtable 00D638F8), then 0078DB3B `call 00B9DD90`, which stores
  field+BCh = spectrum and spectrum+8 = field (00B9DDD1).
- The constructor sizes six vectors (one of 1081h and three of 1000h eight-byte cells, two of
  1000h floats), sets +84h = 0.5 (00CE3800), and ends with 00B96B90(1105h, 1.5707960 (00D638CC),
  32.0, 60.0, 2.0, 0.02, 0.00005) (00D638CC..00D638E0, `RET 1Ch`): +6Ch/+70h = cos/sin of the
  direction (FCOS/FSIN), +74h = 32.0, +80h = 60.0, +84h = 2.0, +88h = 0.02, +8Ch = 0.00005, then
  00B95750. The first argument is not stored.
- Vtable 00D638F8: +8h = 00B96C80 (the step), +0Ch = 00B96E50 (`RET 4`), +10h = 00B96E60, +14h =
  00B96E70, +18h = 00B96E80 (all bare `RET`).
- **The step, 00B96C80(float t), `RET 4`.** vtbl+14h (no-op); 00B95D30(t); `[spectrum+8]->vtbl[0]`,
  which is the field's slot 0, 00B9D7B0 RefreshGridTiles (render tiles); vtbl+10h (no-op).
- **The build, 00B95750** (`__fastcall`, no stack argument): +78h = float(+74h² / [00CF9058]),
  +7Ch = float(+78h * +8Ch); a Mersenne state seeded with 1105h by 00BF0CF0 **on its own stack**
  (ECX = `lea [esp+28h]`), so it never touches the game's shared stream 00BD2F10; two draws of
  00BF0DF0(state, 0.0, 1.0) per cell, 00B94CA0 (the spectrum term), a square root (00BF7030) and
  the scale [0109048C]; stores the 41h×41h half-plane into the vector at +10h, then two 64×64 float
  tables (+60h and +50h). Body to be read in full in packet (2).
- 0078C9B0 applies the scene's +990h block: 00B96C50(WaveHeight → +80h, ChoppyWavesFactor → +88h)
  and 00B9A450 → 00B96C10(direction, speed), each rebuilding through 00B95750, and +B8h =
  HeightScale (section 7(a)).

### (c) The readers

The rel32 census of the disk image (local/p1_rel32.py) finds 49 calls to 0078CF20, 10 to 0078D1B0
BisectWaterCrossing and 7 to 0078C890 (section 7(b)). Every one now sees grid(u,v) · HeightScale
(times the coverage mask, for 0078CF20). Attribution by `ghidra proto --brief`, which may
under-report bodies Ghidra truncated after `_free`; five 0078CF20 sites (00435060, 0068A0A4,
008159AA, 0084B08D/0084B0E3/0084B1A3) and three 0078D1B0 sites (0084B14C, 0084B20C) report no
function.

| Image reader (site) | Host equivalent | What the host's reader gets today |
|---|---|---|
| 00826985 ship motion wave gate (00825F20) | game_hosts_units.cpp `ocean_height` → OceanFieldBinding | 0078CF20 run whole over amplitude 0.0, zero grid: 0.0 |
| 00932D6B hydro forces (009329C0) | game_hosts_units.cpp `water_height_0078cf20` → OceanFieldBinding | 0.0 |
| 00825972/0082599F unit update, 008348D3/00834BFF/00834D62 bow wave, spray, stern wave | game_hosts_units.cpp `sample_ocean_height` → OceanFieldBinding | 0.0 |
| 007DCF05..007DD3E7 plane water-surface law, 007CBD2B, 007CC523 | game_hosts_units.cpp 17401 → OceanFieldBinding | 0.0 |
| 0041BC20-side avoid surface (substitution) | game_hosts_units.cpp `avoid_surface_height` → OceanFieldBinding | 0.0 |
| 009D390E segment occluders, OFF arm | game_hosts_units.cpp 17970/17977 → OceanFieldBinding | 0.0 |
| 0042F38B/0042F3B1 camera keep-above-water | game_hosts_hud.cpp CameraOceanField (its own copy) | 0.0 |
| 0068CBDE in-game interface update | game_hosts_hud.cpp `water_height`, a record | 0.0, unimplemented |
| 00446340 dynamics buoyancy step (004462D0) | game_hosts_fixed_step.cpp, a record | 0.0, unimplemented |
| 006FD08C depth charge in water, 006FD7C1, 008569B3/00856A85 torpedo enters water, 00856FC6 torpedo steer, 0084B3F8..0084B5B5 projectile water traces, 006E8680 projectile create | game_hosts_gunnery.cpp (cc9-gunnery35's lane): the plane y = 0 written inline ("open sea is height zero", lines ~8689 and ~8775); projectile_helpers.cpp and bomb_torpedo_tick.cpp take a sampler interface no game host implements | 0.0 |
| 00854BBF/00855198 submarine update | not traced in this inventory | |
| 008451FB shipyard launched unit | game_hosts_gunnery.cpp 11398 comment; no sample | 0.0 |
| 0043BA49, 00487C3C, 00491BC0, 00498CBD, 0049BADE, 006E21D7, 006E2E5C, 0078D3D0/0078D6C0/0078D785 (seven sites), 007BCDAE, 007C384F, 007C6442, 0085ED6D, 0085F850 | none found by address in src/include | |

So every host reader of the sea answers 0.0; none runs the spectrum, none reads the amplitude.

### (d) What the host has and lacks

| Piece | Image | Host |
|---|---|---|
| 0078C890 sample rule, 00B960C0 bilinear grid, 00B9CF50 mask, 0078CF20 product | section 1-2 | reconstructed (src/ocean_wave_field.cpp, src/ocean_height.cpp) |
| amplitude +24h | HeightScale after the first level-1 render (00BA0B93) | 0.0 constant (game_hosts_units.cpp, game_hosts_hud.cpp) |
| HeightScale +B8h, TimeScale +20h, tile +B0h/+B4h | the scene's Waves keys via 0078C9B0 | not read; tile fixed at 100.0 |
| the spectrum (00B97100, 00B96B90/C10/C50, 00B95750, 00B95D30, 00B95AA0 and its stages 00B94E90/00B950B0/00B952D0, the Phillips term 00B94CA0, the Mersenne pair 00BF0CF0/00BF0DF0) | runs | absent; a labelled zero grid |
| the per-frame step 00BA00C0..00DD and the 00F876A4 clock at the render point | runs | absent; the clock exists as `FixedStepClock::simulation_clock` (in_mission_subsystem_tick.cpp, 00875F26) |
| coverage regions | 00883090..008830AA: each landscape load (BSP_Landscape_LoadTerrain) adds one through 00BA1370 → 00BA0C00 from a landscape texture | none; the mask is 1.0 everywhere, so the host would over-state waves near islands |
| 00BA2B50 horizontal displacement | one rel32 caller, 00BA36CB in the beam system 00BA2FF0; no absolute reference | none; not a gameplay reader (item 4 of the queue) |

Open for packet (2): the full bodies of 00B95750, 00B94CA0, 00BF0CF0/00BF0DF0, 00B95D30,
00B95AA0 and the FFT stages, the writer of [0109048C], and where the host's mission frame can run
the step after the fixed-step driver.

## 9. Binding the live sea (packet cc9_physics1_sea, item 2)

Reconstruction: include/bsp/ocean_spectrum.hpp, src/ocean_spectrum.cpp. Switch:
`bsp::kWaveFieldLiveAmplitudeBound` in that header, committed OFF. Read from disk listings
(`disasm-raw`), never from a live decompile. Status: reconstructed and build-tested; the x87
products are modelled in double and rounded at each native float store, so the grid is close to
the image's but not bit-exact. Names are hypotheses.

### (a) Routines

| Routine | ABI | Rule | Coverage |
|---|---|---|---|
| 00BF0CF0 | `__thiscall(state, seed)`, RET 4 | the existing `random_seed_00bf0cf0` (seed \| 1, ×69069, index 624) | complete |
| 00BF0DF0 | `float __thiscall(state, mean, sigma)`, RET 8 | polar Box-Muller: u = float(2·float(u32·2^-32) − 1) twice until s = float(u1²+u2²) < 1 (no zero test); float(float(sqrt(float(float(ln s)·−2/s)))·u1)·sigma + mean; the second deviate is discarded | complete; the ln/exp helpers 00BFA910/00BFEEEC are taken as the CRT's |
| 00B94CA0 | `float __thiscall(spectrum, kx, ky)`, RET 8 | k = max(float(sqrt(float(ky²)+float(kx²))), 1e-8); A·exp(−1/(kL)² − (kl)²)·\|k̂·w\|^p / k⁴, with \|·\|^p = 0 when the dot is exactly 0 | complete |
| 00B95750 | `__fastcall(spectrum)` | +78h = float(V²/g), +7Ch = float(+78h·+8Ch); a stack Mersenne state seeded 1105h; 41h×41h h0 = (s·g2, s·g1), s = float(1/√2 · float(sqrt(P(n·dk, m·dk)))), two normal draws per cell before the term; 64×64 1/k (k clamped) and ω = float(sqrt(float(k·g))) | complete (vtable+10h is a bare RET) |
| 00B96B90 / 00B96C10 / 00B96C50 | RET 1Ch / 8 / 8 | the setters, each rebuilding through 00B95750 | complete |
| 00B97100 | constructor | +84h = 0.5, then 00B96B90(1105h, 1.570796, 32, 60, 2.0, 0.02, 0.00005) | height path only |
| 00B952D0 | `__thiscall(spectrum, dir, re*, im*)`, RET 0Ch | 64-point radix-2 FFT in place: bit reversal, six passes, twiddles c2 = float(float(sqrt(float(1−c1)))·1/√2)·−dir, c1 = float(float(sqrt(float(c1+1)))·1/√2); dir 1 divides by 64 | complete |
| 00B95AA0 | `__thiscall(spectrum, dir, vector*)`, RET 8 | rows, then columns, through 00B952D0 | complete |
| 00B95D30 | `__thiscall(spectrum, t)`, RET 4 | gate byte 00E130B4 (1 on disk, no writer); h(k,t) = h0(k)e^{iωt} + conj(h0(−k))e^{−iωt} into +20h at row m&3Fh, column n&3Fh; 00B95AA0(−1, +1Ch) | partial: the choppy vectors 00B95EBF..00B9604C and the second transform 00B960A7 are skipped (their only reader is 00BA2B50) |
| 00BA0090 | `__thiscall(field, camera, t)`, RET 8 | [01090494] = float(t·10); step(float(t·+20h)); +2Ch = +B8h; +28h = +30h = float(+B8h·+88h); +24h = +2Ch | height path only; the render body is cc13's |
| 0078C9B0 (ocean part) | | 00B96C50(WaveHeight, Choppy); 00B9A450 → 00B96C10(float(WindDirection·π), WindSpeed); +B8h = HeightScale; 00B9A4C0(Falloff, LayerScale, TimeScale) | ocean keys only |

Constants: dk = 6.283184051513672 (00D638E8), g = 9.8100004196167 (00CF9058), k floor 1e-8
(00CF7FE8), 1/√2 from the static initializer 00CD8870 into [0109048C], 0.1 (00D7A3A0, 0078D184),
10.0 (00CE3DC0), π (00CE3D28).

Self-check (local/p1_probe.cpp, not built by CMake): the transformed grid's imaginary part stays
below 1% of its real part, as the conjugate-symmetric construction requires.

### (b) Host wiring

- **Build.** `ocean_live_field_publish_scene` parses `g_Ocean` (`0_SimpleOcean`) and
  `g_Ocean/Waves` from the scene header (the 004CB420 default for a missing key) and builds the
  object as 0078DAA0 + 0078C9B0 do. It is called from the mission host's header pass
  (src/game_hosts_mission.cpp, routed edit).
- **Step.** `ocean_live_field_render_job_0068a0d0(fixed_clock.simulation_clock)` from the mission
  frame's `render_004ca440` (src/game_hosts_mission_frame.cpp, routed edit): after the frame's
  fixed steps, as 004E4A40 orders them. LABELLED: every in-mission frame is taken as interface
  level 1; the host runs no BSP_Game_Render view branch.
- **Readers.** OceanFieldBinding (src/game_hosts_units.cpp: ship motion 00826985, hydro forces
  00932D6B, the unit's ocean height, the plane water-surface law, the avoid-surface
  substitution, the segment-occluder fallback) and CameraOceanField (src/game_hosts_hud.cpp,
  0042F38B/0042F3B1) sample the live grid times the live amplitude when the switch is on.
- **Not bound here:** the coverage regions (bound since in section 10), gunnery's inline y = 0 plane (torpedo, depth
  charge, shell water crossings in src/game_hosts_gunnery.cpp), the dynamics buoyancy record
  (src/game_hosts_fixed_step.cpp) and the interface-update record (src/game_hosts_hud.cpp 614).

### (c) Predictions (written before any run)

The sea the readers see, from the probe over this installation's scene keys (WindSpeed 1.0 and
WindDirection 0.4 everywhere, so L = 0.102 and the shape is common; amplitude scales with
sqrt(WaveHeight)·HeightScale):

| Row | Scene | WaveHeight / Choppy / HeightScale / LayerScale | rms height | max | max slope per metre |
|---|---|---|---|---|---|
| USN01 | usn_1_marshall.scn | 200 / 2.5 / 2.0 / 100 | 1.04 m | 3.2 m | 0.25..0.33 |
| USN04 | usn_19_coralus.scn (mtime 2024-08-09) | 250 / 1.3 / 3.0 / 125 | 1.75 m | 5.3 m | 0.40..0.44 |
| JM06 | ijn_06_prelude_to_midway.scn | 150 / 1.5 / 2.0 / 100 | 0.90 m | 2.8 m | 0.22..0.29 |
| USN12 (calm) | usn_12_augusta.scn | 100 / 1.5 / 0.5 / 70 | 0.18 m | 0.56 m | 0.05..0.08 |

1. **Mechanism.** Each ON log has one `ocean waves:` line with these keys, then an
   `ocean live field:` line whose amplitude is the row's HeightScale, and render steps equal to
   the in-mission frames so far (one per frame). OFF logs carry neither line.
2. **Hulls.** Ship heave, pitch and roll move on every row from the first in-mission frames; the
   size grows with the table's rms (USN04 most, USN12 least). Hull positions drift through the
   hydro forces, so ranges, gunnery and every RNG-coupled number after the first shot move
   (pair_diff exit 3 on all four rows; USN12's moves are expected to be the smallest).
3. **Planes.** The water-surface law and the avoid-surface substitution see the wave height, so
   low-flying planes (torpedo runs) can move on USN01/USN04/JM06.
4. **No crash, no lost win in the control row.** A win lost on any row is a plan casualty, listed
   in item 3, not a regression.
5. **Uncertain.** Whether a hull's motion is large enough to change a scripted win's timing; the
   ship-motion wave gate (00826985) could also change throttle or keel behaviour, which this
   inventory did not trace.

## 10. The coverage regions (packet cc9_physics1_sea, queue item 2)

Reconstruction: include/bsp/ocean_coverage_regions.hpp, src/ocean_coverage_regions.cpp, driven by
`ocean_live_field_add_landscape_008830aa` (src/ocean_spectrum.cpp) under the same switch
`kWaveFieldLiveAmplitudeBound`. Read from the live decompile (both bodies are clear of the _free
repair) and the 00BA0C00 listing. Status: reconstructed, build-tested, probe-checked over every
terrain model of this installation. Names are hypotheses.

### (a) The chain

| Address | ABI | Rule |
|---|---|---|
| 008830AA | in 00882AC0 | `call 00BA1370` with ECX = [world+A8h] and the terrain render node +3CCh, after 00ADE820 built it (00883058) and 00B6D890 registered it (0088308C); only when [[00E188A8]+19F0h] is set |
| 00BA1370 | `__thiscall(ocean, node)`, RET 4 | 00BA0C00(node), then append node to ocean+5F4h (count +5F8h) |
| 00BA0C00 | `__stdcall(node)`, ocean in EDI | for a geometry node: every draw section whose effect name after its last `/` (00CE7898) holds `oceanwavedamp` is collected; then per section: box over all vertices of its stream (+3Ch) through 004768D0, start x0 = z0 = 1e10 (00CE4970), x1 = z1 = −1e10 (00CE4ADC), each FCOMI/JBE taking the vertex on below/equal/unordered; x0, x1 += [node+120h], z0, z1 += [node+128h] (node world translation, no rotation); sx = float(1/(x1−x0)), sz = float(1/(z1−z0)); the mask from the material's first texture (+10h when the short +34h ≥ 1, else a null and no region); a region record is appended to +620h only when 00B9E170 succeeds; the section moves to the ocean's own draw list (render only); then the children (+34h, siblings +3Ch) |
| 00B9E170 | `bool __thiscall(texture, region*)`, RET 8 | size = texture vt+3Ch, halved while > 80h counting the level; size 0 → log 00D63960, false; level ≥ level count (00B33E50) → log 00D6392C, false; a 1-level A8R8G8B8 texture (renderer vt+88h, pool 2) gets the level through 00B3CD70 = D3DXLoadSurfaceFromSurface(filter NONE); the red byte of each pixel fills a w·h byte vector; +28h = float(w), +2Ch = float(h) |

The region record matches 00B9CF50's reads (section 2): +4h bitmap, +10h..+1Ch box, +20h/+24h
scales, +28h/+2Ch width and height.

### (b) Host

- **Model.** The landscape's `models/terrain/<FilePath>.mmod` through the recovered structured
  reader (00B7F430 with the Mesh/Note/GroupParams parsers). Each Hierarchy item is a node; its
  world frame is its Matrix times its parent's (row vectors), the top items' parent being the
  landscape's world frame. LABELLED: the terrain render node is taken to sit at that frame, and
  items are walked in file order (only overlapping regions depend on order; every terrain model
  here has at most one damp section).
- **Positions.** 004768D0's two paths: float3 at the element offset with no compressed data,
  else the typed lanes (FLOAT3 `pf43`, FLOAT4 `pf44`, FLOAT16 `pf24`) times record 0's scale plus
  its bias, every step in float. LABELLED: the half conversion treats exponent 31 as a number.
- **Texture.** Read through the scene host's VFS reader (the 00B31C20 → 00BDF4C0 route, so a
  `.tga` name finds the `.dds`); an unresolved name falls back to `error.tga` as 00B31C20 does.
  The loader 00B3FCAF keeps the file's levels when renderer+1D84h (= 2 − TextureDetail) is 0;
  LABELLED: 0 is assumed (this installation's options.txt has TextureDetail 2). A file without
  DDSD_MIPMAPCOUNT has one level (D3DXGetImageInfo).
- **Red channel.** DXT1/DXT5 colour blocks decoded as d3dx9_40 does, FITTED, not recovered: red =
  r5·float(1/31); the third and fourth colours a + (b−a)·float(1/3) and b + (a−b)·float(1/3);
  DXT1's three-colour midpoint a + (b−a)·0.5 and fourth colour 0; byte = int(float(v·255) + 0.5).
  Checked against the actual DLL (local p1_dxt_probe: D3DXCreateTextureFromFileInMemoryEx with
  00B3FCAF's arguments, then D3DXLoadSurfaceFromSurface filter NONE): all 54 damp textures and
  error.dds, 2422 distinct (mode, r0, r1, index) keys, 0 mismatches.
- **Hook.** The scene-contents host calls `ocean_live_field_add_landscape_008830aa` once per
  Landscape at the pass-A point, after the terrain load (routed edit in
  src/game_hosts_scene_contents.cpp), and logs `ocean coverage:` per landscape. The unit host's
  OceanFieldBinding and the camera's CameraOceanField read `ocean_live_field_coverage_mask`, which
  is 1.0 while the switch is off.

### (c) Census over this installation (local p1_probe, identity frame)

107 terrain models: 87 damp sections, 84 regions. Refused as the image refuses: dlc_pu_01 and
ijn09c (no texture on the section), usn09_leyte (Leyte_oceanwavedamp.dds has one level, the mask
needs level 2: 00D6392C). dlc_pu_02 and dlc_pu_07 name `waterdamp_DLC3_PU2.tga`, absent from the
loose files, so they take error.dds's red channel. truk01.mmod is refused by the host's Subset
reader (`Could not read Subset child header`); it has no damp section. Every mask is 128×128.

### (d) Predictions for the rows (written before any run)

| Row | Landscapes | Regions | Expected |
|---|---|---|---|
| USN01 | m07_a, 3 × DLC_L_03_S | 4 | waves damped near the atoll; the convoy and carrier near islands move less than in a regions-less ON run |
| USN04 | none (usn_19_coralus.scn) | 0 | identical to the regions-less ON run |
| JM06 | ijn06_cure_atoll | 1 (30 km box) | damping inside the box, which covers the whole play area |
| USN12 | invasion_of_fiji_islands, s07_s | 2 | calm sea, small effect |

1. Each ON log has one `ocean coverage:` line per Landscape with the counts above; OFF logs have
   none.
2. OFF is unchanged (the mask reader returns 1.0 as before).

## 11. The pairs and the flip (packet cc9_physics1_sea, queue item 3)

Same-tree pairs from e103dabd0: OFF is the tree's build, ON `pair_export --flip
kWaveFieldLiveAmplitudeBound=true` (local\p1_on, bsp_game.exe SHA-256 B29453BB75C8). Reference
launch form: BSP_GUNNERY_RNG_STREAMS=1, BSP_DEATH_TABLE=1, `--stop-after-end 40`. Logs
local\p1b_{off,on}_<row>.log; trajectories local\p1t_{off,on}.*.csv.

### (a) Mechanism (predictions 9(c)1 and 10(d)1)

| Row | `ocean waves:` HeightScale | amplitude +24h at 1000 render steps | `ocean coverage:` regions | verdict |
|---|---|---|---|---|
| USN12 (calm) | 0.5 | 0.500 | 2 (invasion_of_fiji_islands, s07_s) | as predicted |
| USN04 | 3.0 | 3.000 | 0 | as predicted |
| JM06 | 2.0 | 2.000 | 1 (ijn06_cure_atoll) | as predicted |
| USN01 (l50_u1_u2) | 2.0 | 2.000 | 4 (m07_a, 3 × DLC_L_03_S) | as predicted |
| USN02 (s36_u2_p7, control) | 1.0 | 1.000 | 0 | as predicted |

Render steps equal the in-mission frames (1000 at clock 50.000 s). The first fixed steps see
amplitude 0.0 (render steps 0), as the image's 00BA19C9 initial value before the first render.
OFF logs carry none of the three lines.

Hulls (USN04, 1200 frames, trajectory CSV, t > 5 s): OFF the destroyers sit still (y standard
deviation 0.001..0.013 m); ON they heave, standard deviation 0.50..0.80 m, ranges about 2.5..3.8 m,
against the predicted sea rms of 1.75 m (a hull averages the wave over its length).

### (b) Outcomes (pair_diff, all exit 3)

| Row | OFF | ON |
|---|---|---|
| USN12 | 3 deaths, 56 hits | 3 deaths, 56 hits; damage 1526.0 -> 1530.3, shots 132 -> 126 (smallest move, as predicted) |
| USN04 | 27 deaths | 27 deaths; shots 7396 -> 8116, torpedo drops 3 -> 2 |
| JM06 | 2 deaths | 2 deaths; damage 3587.2 -> 3565.0 |
| USN01 l50_u1_u2 | **completed 702.85 s** (sus_prog.exe refused) | **no completion** in 22000 frames; deaths 95 -> 57, units 123 -> 93 (the later phases never spawn) |
| USN02 s36_u2_p7 | **completed 782.03 s** (sus_prog.exe refused) | **no completion** in 16000 frames; deaths 18 -> 11 |

No ON ship dies without a weapon killer; the killer-less rows (USN04's movie planes, JM06's Gato at
t = 0, USN01's aircraft) appear OFF too. The two OFF wins reproduce
the AG references exactly.

### (c) Verdict

The mechanism matches every prediction, so `kWaveFieldLiveAmplitudeBound` is ON. The two lost
wins are plan casualties of an image-faithful sea: their order files were tuned on a flat one.
Still flat after the flip: gunnery's inline y = 0 crossing (src/game_hosts_gunnery.cpp, a
separate proposed switch), the dynamics buoyancy record and the interface-update record
(section 9(b)).

## 12. The horizontal-displacement sampler 00BA2B50 has no gameplay reader (item 4)

A scan of the PE on disk (local p1_xscan.py: every E8/E9 rel32 in .text and every absolute
dword in every section; control: 0078CF20 returns 49 references) finds one reference to 00BA2B50:
the call at 00BA36CB in 00BA2FF0 (BSP_OceanBeams_Update). 00BA2FF0 itself has one reference, the
call at 0078D02C inside 0078CFF0, the ocean's render job (0068A0D0 -> 0078CFF0, section 8(a)), and
no absolute dword. So the displacement reaches only the light-beam render effect; no simulation
path reads it, and the skipped choppy vectors of 00B95D30 (section 9(a)) change no gameplay
value. Recorded and closed.

## 13. The six wins with the live sea on (queue item 3, after section 11)

Runs from local\p1_on (e103dabd0 with the flip, the same code as 126f79f83), reference launch form;
logs local\p1w_on_<key>.log and, for the two pair rows, local\p1b_on_{u1w,u2w}.log.

| Win | AG / OFF | ON |
|---|---|---|
| USN02 s36_u2_p7 | 782.03 s | **lost** (no completion in 16000 frames) |
| LOMP10 s37_l10_p2 | 957.54 s | completes 1333.08 s, guard line logged |
| USN12 s38_u12_p2 | 1529.92 s | **lost** (no completion in 36000 frames) |
| ESMP08 l46_e8_orders2 | 3204.10 s | completes 2307.28 s, guard line logged |
| USN01 l50_u1_u2 | 702.85 s | **lost** (no completion in 22000 frames) |
| LOMP06 l50_l6_q6 | 486.80 s | completes 520.05 s, guard line logged |

Three wins lost to the live sea (plan casualties), three hold with moved times.

## 14. Correction: the damp boxes take the landscape frame only (cc13 audit8)

docs/CC13_AUDIT_CC9_RECENT_2.md: 00ADE820 deep-clones a parentless 'Terrain Visibility Group' node whose local
matrix is the landscape's +74h frame (00B6F8F0 / 00B6F150), and 00BA0C00 recurses over every geometry node adding
that root's [+120h] / [+128h]. Item translations are never applied. Section 10(b)'s composed item frames were wrong;
the host now adds landscape_world[12] / [14] for every damp section. Census of the damp items' own composed
translations (local p1 script over the 107 terrain models): 86 of 87 are (0, 0), so their boxes did not move. Only
dlc_pu_04's `DLC_PU_04:waterdamp` was misplaced, by (-3.639, -22.063). None of the pair rows' landscapes is affected.

## 15. The combined pair: live sea + live water crossing (2026-10-12)

Exports of 327ae7e86: ON flips `kWaveFieldLiveAmplitudeBound` and `kGunneryLiveWaterSurfaceBound` (main ec19522f3)
both true (local\p1_on, SHA-256 36BC07976E68), OFF both false (local\p1_off, 7C9A6AD100D3); reference form;
logs local\p1c_{off,on}_<key>.log (local\p1_combined.ps1, summary local\p1_csum.py). Every row pair_diff exit 3.
Mechanism: each ON log has `ocean waves:` with the scene's HeightScale, the same +24h amplitude at 1000 render
steps, the predicted coverage regions, and gunnery's `live water crossings=N bound=1`. OFF has none of these.

| Row | OFF | ON |
|---|---|---|
| USN04 3000 | impacts 101, water 9 | impacts 94, water 9 |
| JM06 3000 | 2 deaths; impacts 141, water 4 | **10 deaths**: the player subs' torpedoes sink eight US ships; shots 235 -> 512, every one a live crossing. A torpedo launched below y = 0 never satisfied the old `from > 0` test, so it never started its swim; 0084B380 needs only end.y <= surface |
| LOMP10 3000 | impacts 118 | impacts 142 |
| USN01 l50_u1_u2 | completed 475.21 s | **no completion** |
| USN02 s36_u2_p7 | completed 782.03 s | completed **203.01 s** |
| LOMP10 s37_l10_p2 | completed 1127.67 s | **no completion**; only-OFF deaths include Oyodo (Coastal gun) |
| USN12 s38_u12_p2 | completed 1529.92 s | **no completion**; only-OFF deaths: Charles, Claxton (Japanese torpedoes) |
| ESMP08 l46_e8_orders2 | completed 3456.80 s | completed 2807.71 s |
| LOMP06 l50_l6_q6 | completed 486.80 s | completed 397.53 s |
| BSM01 l50_b1_a | EndMission 767.94 s | EndMission 767.94 s |

The OFF wins moved from AG on current main (USN01 702.85 -> 475.21, LOMP10 957.54 -> 1127.67, ESMP08 3204.10 ->
3456.80); these are other lanes' commits, so compare within the pair only. Wins with both ON: USN02, ESMP08, LOMP06
and BSM01 hold; USN01, LOMP10 and USN12 are lost. USN02 came back: it was lost with the sea alone and completes with
the crossing too. The LOMP10 and USN12 losses are not explained per entity yet.

## 16. The LOMP10 and USN12 losses, per entity (cc9-physics2, 2026-10-12)

Logs from section 15's pair (local\p1c_*); per-entity tools are local\p1_deaths.py and local\p2_health.py.

**USN12 (s38_u12_p2): a knife-edge on a one-ship plan.** Phase 3 completes when `luaRemoveDeadsFromTable(MontGrp)[1]`
comes within 1000 of ClEscape (usn_12_augusta.lua, this installation, 2024-07-13). The orders steer only
Montpelier. Both runs have Agano launch the same 8-torpedo salvo at Charles (OFF 818.87..822.67 s, ON
812.73..816.52 s). In OFF it sinks Charles at 830.67 s; in ON it sinks Montpelier at 824.07 s. Haguro's next salvo
likewise sinks Claxton in OFF and Columbia in ON. With Montpelier dead, the escape test runs on Cleveland, which
nothing steers. Systematic damage is not what changed: deaths are 20 / 20, and only the victims of one salvo
differ.

**LOMP10 (s37_l10_p2): a stalemate after a chaotic divergence.** The win is MonitorSanJoseForce's
`getn(alive) == 3` over 8 ships (10_san_jose.lua), polled each second. OFF loses 5 (Kasumi 472.96 s, Asashimo
938.14, Sugi 968.69, Oyodo 1044.49, Kaya 1111.71). ON loses 4 (Kasumi 442.52, Kaya 474.31, Asashimo 954.79, Sugi
1044.84). The killers and times line up: the same CB4 squadrons, CB4 itself, and Coastal Gun 02 at about 1044.5 s.
The gun takes Oyodo in OFF and Sugi in ON, because in ON Sugi survived the CB4 strike that sank it at 968 s. An ON
run extended to 40000 frames (local\p2s_on_l10x.log) has no fifth death and no completion. The four survivors
(Ashigara, Oyodo at hull-water health 0.073, Kiyoshimo, Kashi) take no damage after the 1500 s cap, so this is not a
frame-cap artefact. The first moved entity is Kasumi, 30 s earlier, to the same squadron. Verdict: chance, not a
directional effect of the sea; the plan has no margin for one kill fewer.

## 17. The flip: live sea and live water crossing ON (cc9-physics3, 2026-10-12)

`kWaveFieldLiveAmplitudeBound` (include/bsp/ocean_spectrum.hpp) and `kGunneryLiveWaterSurfaceBound`
(src/game_hosts_gunnery.cpp) are set true together, by the lead's decision. Basis: the mechanism matched every
prediction in sections 11 and 15 (the scene's HeightScale as the +24h amplitude, one render step per frame, the
predicted coverage regions, every round's water crossing at 0084B380 -> 0078D1B0 on the live surface), and each win
the combined pair lost (USN01, LOMP10, USN12) was explained per entity as a knife-edge or chance, not a directional
effect (section 16). The two switches go together because the crossing samples the live sea: with the sea alone a
round still crosses at y = 0 while hulls ride the waves. The wins pair on main with and without this commit follows
in its own section.

## 18. The seven wins, main against main + the flip (cc9-physics3, 2026-10-12)

Exports: OFF = origin/main db5ab3c4e (local\p3_off, SHA-256 8151D0015447), ON = fe01ecf32, section 17's commit
(local\p3_on, B22D402CE8ED). Reference form with `--stop-after-end 40`, BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1;
logs local\p3w_{off,on}_<key>.log (local\p3_runs.ps1, summary local\p3_sum.py, one-sided deaths local\p3_deathdiff.py).
Every row is pair_diff exit 3; every ON log has `live water crossings=N bound=1`, OFF has 0. Every completion logs
the exec guard's sus_prog.exe refusal.

| Row | OFF (main) | ON (main + flip) | Deaths OFF / ON |
|---|---|---|---|
| USN01 l50_u1_u2 | completed 425.72 s | **no completion** (22000 frames) | 91 / 61 |
| USN02 s36_u2_p7 | completed 198.06 s | completed 203.01 s | 14 / 12 |
| LOMP10 s37_l10_p2 | completed 1184.73 s | **no completion** (30000 frames) | 56 / 71 |
| USN12 s38_u12_p2 | completed 1589.38 s | completed 1554.69 s | 109 / 109 |
| ESMP08 l46_e8_orders2 | **no completion** (80000 frames) | completed 3714.45 s | 738 / 711 |
| LOMP06 l50_l6_q6 | completed 369.98 s | completed 535.24 s | 7 / 4 |
| BSM01 l50_b1_a | EndMission 767.94 s | EndMission 767.94 s | 194 / 188 |

Per entity:
- USN01: the only OFF-only ship death is Convoy6 (194.96 s, ConTBD3, cat 1), the same single roll as section 13.
  The ON-only deaths are shore targets after 619 s. A plan casualty, as before.
- LOMP10: OFF-only Kasumi (466.46 s, CB4 squadron) and Kiyoshimo (1167.96 s, Coastal gun 02); ON-only Oyodo
  (950.69 s, CB4). The plan needs every ship, and one fewer falls. The same kind as section 16.
- USN12 is back: it completes ON with the same 109 deaths.
- ESMP08 is back: Cassin dies at 438.32 s in ON (Zuikaku_sqn02, cat 10), which is the strike main's Dyn precision
  flip lost (GUNNERY 183). On main alone it is still lost (Cassin survives; Zuikaku dies at 1673.16 s).

Net: main holds six of seven (all but ESMP08); main + the flip holds five (USN02, USN12, ESMP08, LOMP06, BSM01).
Lost by the flip: USN01 and LOMP10. Gained: ESMP08.

## 19. classDesc+74h over the bullet classes, and 006E8680's (x, y) (cc9-physics3, 2026-10-12, read-only)

**+74h.** `0084C11C..0084C14C` runs the exit trace `0084B4B0` only when the shot's vtable[2Ch] (its medium) is
non-zero and `0084C133 cmp byte [edi+74h], 0` finds zero; otherwise the entry trace `0084B380`. The vtable[30h]
accessor `006E2580` reads the same byte (`mov eax,[ecx+4]; mov al,[eax+74h]; ret`). The byte is the weapon class's
`ExplWaterHit` (include/bsp/projectile_kinds.hpp `kWeaponClassOffExplWaterHit`, read with default false at
projectile_kinds.cpp 191). Census of this installation's bulletclasses.lua (local\p3_explwater.py; realistic
105 entries, mtime 2025-06-02; arcade 119 entries, mtime 2026-05-09, locally modified like vehicleclasses.lua):

| Type | realistic | arcade |
|---|---|---|
| Artillery, Bomb, Bullet, Flak, Rocket, Kamikaze, DummyKamikazePlane, DummySubmarine, DummyTarget | all `true` (83) | all `true` (96) |
| Torpedo, Depthcharge, WaterMine, Paratrooper | all absent, so false (22) | all absent (23) |

So the exit trace can run only for a torpedo, depth charge, mine or paratrooper already in the water; every
other round ends at the water whichever medium it is in. That is what the host does (GUNNERY 182).

**006E8680.** `006E8673` stores `[esi+100h]` as the second argument and `006E867D` `[esi+0FCh]` as the first, then
`006E8680` calls `0078CF20` (which passes both straight to `0078C890` and `00B9CF50`) and `006E8685` compares
`[esi+100h]` with the result: y <= h selects vtable[28h], else [24h]. The spawn-time medium test therefore samples
(x, y), the same axis defect as `0084B3D8` and `0078D29D` (docs/PROJECTILE_HELPERS.md section 1). The host's
`create_projectile_006e8430` (projectile_kinds.cpp 339) samples (x, z); it has no caller in src, so nothing moves.
The one-line correction was routed to the lead (not this lane's file).
## 20. Re-planned USN01 and LOMP10 for main + the flip (cc9-physics3, 2026-10-12)

All runs on local\p3_on (fe01ecf32), reference form, `--stop-after-end 40`; order files and logs in
J:\PROG\battlestations-pacific-decompile-cc9-physics1\local (p3_u1_*.txt, p3_l10_*.txt, p3r_on_*.log). Each
completion logs the exec guard's sus_prog.exe refusal.

**LOMP10.** The win is `getn(alive SanJoseForce) == 3` of eight (this installation's usn\LOMP\10_san_jose.lua 557,
mtime 2024-07-13): five of Ashigara, Oyodo, Kiyoshimo, Asashimo, Sugi, Kashi, Kaya, Kasumi must sink. Section 18's ON
run sank four. s37_l10_p2's launches end at frame 11000 and most of its retries are refused ("slot not in state 1 or
5"). The re-plan keeps s37_l10_p2 and adds a CB4_AF launch every 200 frames from 11400 to 26000, slots 1..4 with the
same classes (118, 104, 135, 104). **p3_l10_b.txt** (targets alternate Kasumi, Kiyoshimo): completed 978.43 s, the
fifth death Kiyoshimo at 863.86 s (CB4_AF_sqn17). p3_l10_a.txt (Kasumi, Kiyoshimo, Kashi): completed 1034.48 s.

**USN01.** Phase 2 completes on `getn(alive Mission.Convoy) == 0` (usn\usn_1_marshall.lua 535, mtime 2024-07-13), and
only the Con* squadrons are player units then: `select` of a cruiser is refused (00645060) and Enterprise's slots are
empty for a Convoy6 launch (006C0F00). After the strike the squadrons are spent: retargets at 200-400 s apply but
Convoy6 keeps 1348/2400 with damage control on, and the squadrons ditch at 23 m after 525 s (p3_u1_a..f). Retargets
before the strike move which ship survives (p3_u1_g/h/i/k each leave one or two). What works is one retarget in the
middle of the strike: ConTBD3, whose second plane sinks Convoy5 at 185.16 s, is pointed at Convoy6 for its remaining
torpedo. **p3_u1_o.txt** (l50_u1_u2 + `3690 select ConTBD3`, `3691 target ConTBD3 Convoy6`): completed 618.72 s, Convoy6
sunk at 200.36 s. p3_u1_m.txt (the same at 3704): completed 618.72 s. At 3720 (p3_u1_n) it is too late.
## 21. The plane water laws next to the ocean object (cc9-physics3, 2026-10-12, read-only)

cc13's band sweep (docs/CC13_BAND_0074_0078.md) left `0078D3D0`, `0078D6C0` and `0078D785` to this lane. Callers are
from a rel32/absolute-dword scan of the image on disk (local\p3_xref.py; it finds the known calls, so a missing
caller is a real absence of literal references, not a vacuous scan).

| Address | What it is | Callers |
|---|---|---|
| `0078D3D0`..`0078D53E` (RET 0Ch, thiscall world) | the water-surface normal at (x, z): four `0078CF20` samples at x +- 1.0 and z +- 1.0 (the double `00D7A210` = 1.0), a cross product `004F9B30` of the two differences (with the float `00D7A24C` = 1.0 as the step component) and a normalize through `00419440` | `007DD034` and `007DD39D`, both in `007DCDD0`, the plane's state-6 water law |
| `0078D6C0`..`0078D81E` (RET 1Ch) | a segment's water crossing: samples `0078CF20` at the start, then bisects with `0078D1B0` towards the entry (start above) or, from `0078D785`, the exit (start below) | one, `0053FC0A` in `0053F2D0`, the menu screen burst update (include/bsp/menu_screens_2.hpp 994): not gameplay |
| `0078D785` | not a function: the start-below branch target of `0078D6C0` (`0078D6EB JBE`) | none |

**What the host does against the live sea.**
- The free-flight water line (`007CC523`..`007CC562`, game_hosts_units.cpp 17550) samples `0078CF20` through
  `OceanFieldBinding`, which returns the live field when the sea is on. Logged contacts now show non-zero water
  heights (USN01 p3r_on_u1o: Mav4 water -0.58, Mav3 1.07). Live; no gap.
- The depth kill (`007CE313`..`007CE3AC`) compares the absolute altitude with -30.0, as the image does. No gap.
- A live aircraft with a non-zero MinWaterSpd (every torpedo and dive bomber) ignores a contact (`007CB7F0`'s early
  return) and stays in free flight, so a low torpedo run sees the live surface only through the line above. No gap.
- **The gap is the state-6 arm `007CBA50` -> `007DCDD0`** (docs/WATER_SURFACE_LAW.md 2.3): the buoyancy law with the
  `0078D3D0` normal, the `007DB680` water arm (partly unread at `007DC426`..`007DC64A`), the righting term, the
  speed damping and the 1.5 m lift-off back to state 7. The host freezes a state-6 aircraft where it touched.

**Reach of the gap.** Only aircraft in state 6: dead ones (a wreck that floats instead of freezing) and live
classes with MinWaterSpd zero. In this installation's vehicleclasses.lua (mtime 2026-05-09) those are the nine
floatplanes and flying boats: M6A Seiran, OS2U Kingfisher, PBY Catalina, F1M Pete, E13A Jake, H6K Mavis, H8K Emily,
A6M-N Rufe, Black Cat (local\p3_minwater.py). In the logs on disk the only live state-6 entry is JM08's "Movie
Mavis" (alive, MinWaterSpd 0, cc9-gunnery35 g35_*_jm08 logs), a scripted alighting. No win row and no torpedo row
has one. Not bound: the binding would be the whole `007DCDD0` law with an unread arm, and nothing scored reaches it.
## 22. Section 21's gap, closed (cc9-physics4, 2026-10-12)

The state-6 arm `007CBA50` -> `007DCDD0` is read whole and bound: `kPlaneWaterSurfaceLawBound`, ON at
`25d13aa63` (docs/WATER_SURFACE_LAW.md sections 10-12). `007DC426`-`007DC64A` is read. USN01 and ESMP08
pairs are gameplay identical; only the wrecks move. `0078D6C0` (the menu burst's water crossing) stays
unbound: not gameplay.

## 23. cc9-physics4 handoff (sea/physics lane + WINS KEEPER), 2026-10-13 01:18 UTC

(GUNNERY_OPEN_ITEMS.md was leased by gunnery38, so the handoff lives here.) Tree
J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1), merged with origin/main. Run launcher
local\p4_runs.ps1 (sets ga, arc, wr, lt, u1z4; -Seq runs one pair at a time; orders from config/win_orders). Edit
scripts local\p4_*.py.

Done and flipped ON: kPlaneWaterSurfaceLawBound (25d13aa63), kPlaneWreckDepthRemovalBound (0ea291d35),
kGunAimDegreePrecisionBound (f435022e8), kGunArcFloatPrecisionBound (67e99782c). Docs: WATER_SURFACE_LAW 10-13,
X87_CONTROL_WORD (the host precision policy and the spots), DYN_LCP_IMPULSE_MATH / DYN_CONTACT_SOLVER corrections,
PLANE_INPUT_CHECKS_CC9.md. Win order files in config/win_orders (3bde171d1).

Open, in order:
1. **USN01 re-plan** on main once the lead confirms n1 and lua53's kamikaze+sweep flip have landed (gunnery38's O1,
   avoid-pick and gun-burst are on main at bc4f4022a). First divergences in GUNNERY 198-199: Convoy5 survives (n1),
   Convoy1/6 (gun-burst/O1), phase 3 escort (avoid-pick). Method: WINS_TABLE keeper notes (retarget mid-strike).
2. **kPlaneLeaderTurnRollBound** (8b0e7bfce, OFF): faithful, loses USN01 p3_u1_z4 (deaths 90 -> 68); the lead decides
   flip + re-plan. Its USN12 half was invalid (no order file): re-pair USN12 + LOMP10 from config/win_orders.
3. **Channel C (the wing fold) at the elevator:** read whole (PLANE_INPUT_CHECKS_CC9.md (a)); the binding needs a Lua
   accessor for VehicleClass[i].PartAnims.Wings[1..3] (lua lane), then a per-plane value stepped in the fixed step and
   the elevator test value == 0.0 for the ten Wings classes.
4. Census row 1 skipped by the lead's ruling; the sea bands' raw-ownership leftovers are teardown helpers only.
5. One launch died at renderer init with the session on the console (18:05 local): check query session first.

## 24. cc9-physics5 handoff (sea/physics lane + WINS KEEPER), 2026-10-13

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1). Launcher `local\p5_runs.ps1 -Rows
"<log>|<exe>|<MISSION>|<mission frames>|<orders or empty>|<jitter or empty>"` (one Start-Process per row; logs
`local\<log>.log`), wait `local\p5_wait.ps1 -Names a,b -Need n` (a log is done when it is no longer locked), USN01 one-screen
summary `local\p5_u1sum.ps1`, plan variants `local\p5_mkplan.py <name> "<frame> <order>" ...` (inserts into z4),
rel32 census `local\p5_rel32_scan.py <targets>`.

Done: kPlaneLeaderTurnRollBound ON (53c3f2814); `read_vehicle_class_part_anim` (7066f75f4); the wing fold
`kPlaneWingFoldElevatorBound` / `kPlaneWingSpreadTakeoffBound` (24086a71d, OFF; docs/PLANE_INPUT_CHECKS_CC9.md);
`kActuatorBlockStepOrderBound` ON (1f83f2ffa, c01d9516e, e97058a57); USN01 plan e (1db9b4dd7, won on 3eac809d6).

Open, in order:
1. **USN01 re-plan again** on current main: plan e loses on 64675c644 (WINS_TABLE). Plan d (= e + the Wildcat
   launches) loses too. Start from `p5so_on_u1e.log`: Convoy2 and Convoy6 survive. Check whether the dead-target
   retest ends ConTBD1's/ConTBD3's command after their first kill (`squadron retest:` lines) and re-target those
   squadrons with `select`/`target` lines just after each kill, as plan d did for ConTBD1 -> Convoy3.
2. **Wing fold pair: not decided.** On top of the step order (builds `local\p5_wf` elevator-only, `local\p5_wt`
   both, at c01d9516e) every pair so far is gameplay-identical: USN01 d/e (no Wings plane reaches phase 3 on the
   current main), LOMP10 (no Wings class), ESMP08 without orders (510 Wings planes, no carrier deck ops). The only
   row with carrier launches of Wings classes is ESMP08 WITH orders (Avengers/Helldivers/Kates/Jills from carriers):
   pair both switches there, combined with item 3. Expect `prep_holds` > 0 and launches later by up to the class
   duration (2.5-4.0 s); `retracts`/`elevator_waits` only if a Wings plane lands on a carrier.
3. **ESMP08 re-plan** (lead's queue): base orders cc9-gunnery36\local\g38_ord_e8.txt (r1/r2), generator
   cc9-gunnery36\local\g38_e8_retarget.py (re-target slots whose target sinks), allocation to sink Fanning, Grayson,
   Woodworth and Dunlap before ~1750 s while the big ships draw the flak (GUNNERY 196-199). Commit the winner to
   config/win_orders/.
4. WINS_TABLE rows pending ships45's re-check: USNOS `usnos_s45_os_k`, JM08 `jm08_s45_j8_m10`.
5. `local\p4_runs.ps1`'s USN12 rows quote `'$root\config\...'` in single quotes, so the path is literal: that is why
   cc9-physics4's USN12 runs had no orders. Use p5_runs.ps1.


## 25. cc9-physics6 handoff (sea/physics lane + WINS KEEPER), 2026-10-13

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1). Launcher `local\p5_runs.ps1 -Rows
"<log>|<exe>|<MISSION>|<mission frames>|<orders or empty>|<jitter or empty>"`, wait `local\p5_wait.ps1`. Pass every
order file as an ABSOLUTE path. Run slots are 6 since f585ddfd3. New helpers in local\:
- `p6_prepend.py`: first targets for a carrier's launch groups in a frame window.
- `p6_e8_alloc.py`: ESMP08 slot target re-ordering.
- `p6_e8_extend.py`: repeats the ESMP08 600-frame launch cycle.
- `p6_e8_retarget.py`: copy of g38_e8_retarget.py.
- `p6_e8sum.py` / `p6_e8time.py`: ESMP08 deaths and launches per 300 s.
- `p6_j8gen.py` / `p6_splice.py`: ships45's JM08 generator; the splice at 24000 reproduces m10.
- `p6_j8sum.py`: JM08 landing-ship timeline.
- `p6_u1_ships.py`: ship attack orders. The USN01 cruisers cannot be selected.
- `p6_ctrl.py`: samples the controlled-unit trace.
- `p6_firstdiff.py`: first differing filtered line between two logs.
- `p6_v1r1_build.ps1`: export + patch + build for a scratch pair.

Done:
- Flips:
  - Wing fold ON (ae34e5462).
  - kSceneShortNameModeSuffixBound ON (398a3056b, a0b684a37).
  - kMissionWarningReportWalkAllBound ON (8c0dc221a; no host caller yet).
  - V1 kPlaneSpeedFloorBound and R1 kPlaneRateLawCrossingFloorBound ON (57fce131f, CC13_GPR_48 9).
- Wins re-planned:
  - JM08 `jm08_p6_j8_f` (c7358a3d2).
  - BSM01 `bsm01_p6_b1_a` (8c0dc221a).
  - USN01 `usn01_p6_u1_g` (ac09d074d), then `usn01_p6_u1_k6` for R1.
  - ESMP08 `esmp08_p6_e8_br3l` (0993442ed; it needed kAirOpsSquadronCeilingRaisedBound).

Open, in order:
1. **USN04 climb-out crash with V1.** cc13-ver14/ver17 report Shoho/Zuiho/Yorktown script launches flying into the
   sea 16-26 s after lift-off, and ver17 traces it to V1 (planes under continuous band repair bleed to 17-19 m/s).
   - My pair here does not reproduce the setting: USN04 with no order file, 12000 frames. OFF is
     `local\p6_rfv` EC73A09F1208 (V1 OFF), ON is the tree at 57fce131f (`local\p6v_{off,on}_u4.log`).
   - That run launches only four US squadrons (Lexington/Yorktown-class01) and no Japanese script launches. 19
     plane water contacts on both sides, all scene squadrons. `speed_floor_arms` 1980, raises 299, gameplay moved.
   - Next: get ver14's exact run (frames, order file, build) from the lead, then pair it.
2. **USN01 `k6` is a knife-edge.** It fails under jitter 20,3 and 20,7. The torpedo budget has no margin (GUNNERY
   / WINS_TABLE notes); ConSBD3 never releases, because every aimglide is blocked at the ceiling. Before
   re-planning further, ask whether lua56's dive work fixes that, since it would add six bombs.
3. **The rest of the plane-flight packet**, each behind OFF on src/game_hosts_units.cpp (shared: claim, apply,
   build, commit, release at once):
   - D2: bank yaw 007DA080-007DA117, `bank_yaw_angle_007da0ae`.
   - D4: wave 1 pose before wave 3 physics.
   - D5: thrust scale unit+0CC8h x 008E6430(6).
   - R2: yaw gain BC4h placement.
   - C1: wreck forward drag 007DBC77, model `dead_meat_forward_drag_007dbc77`.
   - R4: crippled roll/spin.
   - The deck/runway clamp 007DA2B1-007DA338 and the boost arm.
   - P1 is DROPPED (cc13-ver15: the host equals the image).
   - Pair each on USN12, LOMP10 and a carrier win (USN01 k6, JM08 f, ESMP08 br3l).
4. **WINS_TABLE**: re-run USN02, LOMP06, BSM01, USN15, IJN18, JM06 and JM08 f on the next main (the navigator group,
   V1/R1 and the ship flips since moved times).
## 26. cc9-physics7 handoff (sea/physics lane + WINS KEEPER), 2026-10-13, goal PAUSED

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1). Nothing in src/ is committed, and no
lease is held. The units.cpp patches are scripts in local\:
- `p7_d2_apply.py` (kPlaneBankYawBound), `p7_r2_apply.py` (kPlaneYawGainRawBound), `p7_d4_apply.py`
  (kPlanePoseFirstBound) and `p7_d1_apply.py` (kPlaneDeckClampBound). D4 and D1 need D2 applied first.
- `p7_chain.py <units.cpp> d2:on,r2:on,d4:on,d1:on` applies all four OFF, then flips the named switches.
- `p7_scratch_build.ps1 -Out <dir> -Apply p7_chain.py -ApplyArg ...` exports HEAD, patches it and builds.
- Runs: `p7_wins.ps1 -Tag <t> -Exe <exe> [-Only keys]` (every win, ABSOLUTE order paths) and `p7_keep.py <tag>`.

Done (docs/CC13_GPR_48.md 10 has the evidence, predictions and pairs; WINS_TABLE has the times):
- Wins re-time on main ad68156b1. Lost: PRCP01 (lua57's think-delay fix, now on main, restores it at 865.06 s) and
  ESMP08 br3l (Cummings survives; the kill rate halves).
- Pairs, OFF = main, rows USN12 / LOMP10 / USN01 k6 / JM08 f:
  - D2 bank yaw: faithful. Its sign follows the bank. It loses USN01 k6 and JM08 f alone.
  - R2: no loss.
  - D4: loses USN01 k6.
  - D1+V4: no loss; lifts 0 on USN12.
  - All four ON: every win holds except USN01 (re-planned: `usn01_p7_u1_a`, 366.33 s, jitter unchecked) and
    ESMP08 (stopped before it ended).
- D5: no writer of unit+CC8h by disp32 anywhere (the only hit in plane code is the 007DB7BB read). 008E6430 is empty
  in the host. Nothing to bind; the host's 1.0 stands.

Plan for the resume, APPROVED by the lead:
1. Claim src/game_hosts_units.cpp, apply `p7_chain.py ... d2:on,r2:on,d4:on,d1:on` to the tree, build, commit one
   commit with the four switches ON, and release.
2. Re-run the win set on that commit. Check `usn01_p7_u1_a` under jitter 20,3 and 20,7. Re-plan ESMP08 (the 14
   kills took until 6475 s; try a longer cycle with `p6_e8_extend.py`, or re-targets to Cummings). Re-check JM08 f
   (1603.74 s on the scratch).
3. C1 and R4 (the lead allows edits to src/plane_flight.cpp and src/plane_control_rate.cpp/.hpp):
   - R4 decoded: when unit+5Dh is set, `roll *= Interp(0,1,RollMulTime,RollMul,C3Ch)`. Then, if C36h,
     `roll +/- min(4*C3Ch,1)*SpinRollSpd*max(0.35,RollSpd)`. The sign comes from C37h, set at 007D0DD7 from spin
     message byte +24h: 1 for rightspinning (007BA200 arg), and only when (unit+360h)+160h is non-null. The host
     has plane_death_c36 and death modes SpinLeft/Right; C37h needs a field.
   - C1: the in-air ctl+94h producer is 007CE0DB-007CE1D9: unit+5Dh, altitude unit+100h < 15.0 (00CF3F20), and any
     point of the (unit+538h)+5B8h list below world y -1.0 -> 007D8180(0). That also turns on the extra gravity
     (state.extra_gravity) in the air. The point list is not in the host.
4. USN04 V1 confirmation (queue 2): write a minimal USN04 order file that reaches phase 2 (read
   usn_19_coralus.lua's phase-1 exit). Pair V1 OFF/ON with BSP_FRAME_JITTER=20,3, --frames 45200 --mission-frames
   45000 --mission-frame-seconds 0.0222 and the reference env.

**Update, same day:** plan step 1 is DONE. The four switches are ON in the commit that adds this note (lead's go
under the pause). Still pending for the resume:
- ESMP08 br3l on this build. The all-ON run was stopped before it ended; on main it was already lost, so expect a
  re-plan with `p6_e8_extend.py` or re-targets to Cummings.
- The USN01 `usn01_p7_u1_a` jitter checks.
- Steps 3-4 above.

## 27. C1 and R4 bound OFF (cc9-physics8, 2026-10-14)

Packet cc9_plane_wreck_roll. Two switches in `src/game_hosts_units.cpp`, both committed OFF; the terms live in
`plane_control_targets_007da710` (src/plane_control_rate.cpp) and `accumulate_free_flight_007db680`
(src/plane_flight.cpp), which leave the old result when the host does not set their new inputs.

- **R4, `kPlaneDeadMeatRollBound`, `007DA7E4`-`007DA8D5`** (listing read whole; constants read from the image):
  - unit+5Dh set: `roll = 00419010(0, 1, RollMulTime [00F8733C], RollMul [00F87340], C3Ch) * roll`
    (6 s and 2.0 in this installation's PlaneGlobals).
  - and C36h set: `ramp = float(C3Ch * 4.0)` (double 00D7A328) capped at 1.0f (00D7A24C, JBE so the value wins
    ties); `term = ramp * SpinRollSpd [00F87338] * 00415550(0.35f [00CF6560], RollSpd desc+1A8h)`, the larger.
    C37h set: `roll + term` (007DA890 FADD); clear: `roll - term` (007DA8D1 FSUBR on the roll kept as a double).
  - Then 007DA8D9 still zeroes the roll target in controller mode 1.
  - C37h: 007D0DD7 sets it in the kind-67h arm when the message byte +24h is set (rightspinning) and the model
    object is non-null; that arm also looks up the "rwing" (00CE444C) and "aileron_r" (00D06178) nodes. Leftspinning
    takes 007D1043, which sets C38h ("lwing", "aileron_l") and leaves C37h clear. Host: `plane_spin_right_c37` set
    at the death latch for `SpinRight`. SUBSTITUTION: the model object is taken as present.
  - unit+5Dh is the host's `plane_death_c3a`, as in the water step.
- **C1, `kPlaneWreckForwardDragBound`, `007DBC77`-`007DBD34`**, the cc13 model `dead_meat_forward_drag_007dbc77`
  (CC13_GPR_48 8.3) placed after the roll-drag extra, inside the speed > 0.001 path. dyn+0Ch is still zero there,
  and the ceiling adds to it later, so the order is exact.
  - The gate is ctl+94h. The image sets it in two places: the water step (007CBBE7, the host carries it) and the
    free-flight arm `007CE0DB`-`007CE1D9` (listing read): unit+5Dh, `unit+100h < 15.0` (double 00CF3F20), ctl+94h
    still clear (`007D7A80` returns it), then each 12-byte point of the class list (unit+538h)+5BCh..5C0h through the
    pose (`004142E0`); the first with world y below -1.0f (00D7A260) calls `007D8180(0)`, which sets ctl+94h in
    single player (`[00E188A8]+1FE4h` 0 or 1). The point list is not in the host, so the in-air producer is not
    carried (SUBSTITUTION, labelled): an airborne wreck gets C1 only after its first water step.
- Counters: `summary plane dead meat roll steps=.. spin=.. bound=.. wreck drag steps=.. bound=..`.

**Predictions (written before any run).**
- OFF: every counter is 0 with bound=0, and the build is gameplay-identical to main (pair_diff 0 or 1).
- R4 ON: `steps` > 0 wherever a plane dies in the air, `spin` > 0 where a death draws spinning
  (`death_mode_chances_spinning`). Dying planes roll up to twice as fast over 6 s and spinners corkscrew
  (5 * max(0.35, RollSpd) rad/s more). Expected verdict 3 on USN12/LOMP10/the carrier win: crash points and
  timings move, so crash-sweep contacts and the shared RNG stream re-roll later kills. Mechanism check: the
  per-plane roll of a spinning wreck in the ON log grows in the C37h sign.
- C1 ON: `wreck drag steps` > 0 only where a dying plane reaches the water with C3Ch > 1 s; wrecks in the water
  lose forward speed with a 1 s time constant (10 s^-1 below 8 m). Expected verdict 1 (presentation only) or a
  small 3 through wreck positions, with no win lost.

**Pairs (cc9-physics8, 2026-10-14).** OFF `local\p8_off` (dc07a0286, exe 2C6EFA46E476), R4 ON `local\p8_r4`, C1 ON
`local\p8_c1` (exe D38D11A0C35A); reference env, `--stop-after-end 40`; logs `local\p8p_{off,r4,c1}_<row>.log`,
diffs `local\p8pd_*`. Rows USN01 `usn01_p8_u1_c`, USN12 `s38_u12_p2`, LOMP10 `p3_l10_b`, PRCP01 `l55_p01_c`.

| Row | OFF end | R4 | C1 |
|---|---|---|---|
| USN01 | completed 1094.94 s | 1 identical; roll steps 7945, spin 0 | 1 identical; wreck drag steps 564 |
| USN12 | completed 1510.35 s | 3 moved: deaths, units and death modes identical, one killer_range 554 -> 553; steps 7199, spin 0 | 1 identical; 860 |
| LOMP10 | completed 1176.57 s | 1 identical; 10999, spin 0 | 1 identical; 742 |
| PRCP01 | completed 670.81 s | 1 identical; 2487, spin 0 | 1 identical; 244 |

- C1 as predicted: wrecks reach the -30 m removal on new paths (USN12 MovieBetty|.-3 at 1021.77 s instead of
  1021.67 s), nothing else moves.
- R4: the ramp half runs on every dying plane's control step. **The spin half has no reach in the host**: no
  death draws spinning, because `plane_death_mode_007ca8a0` needs unit+800h (the spin side) at 0 or 1, and the host
  passes -1 (game_hosts_units.cpp, `din.spin_side_800 = -1`): 007CFDB4 sets the side only when a hull segment of
  the plane is hit, and the host has no per-segment plane hits. Open for the gunnery/plane-damage lane.
- **Both flipped ON** by verdict (faithful, no win lost) in the commit that adds this note.

**USN04 V1 pair (section 26 step 4, CC13_GPR_48 9).** No order file is needed: with no orders, 45000 frames at
0.0222 s and `BSP_FRAME_JITTER 20,3` (reference env) the mission reaches phase 2 ("blackout callback luaMoveToPh2
ran"). At 0.05 s frames it does not: the phase-1 exit re-calls `Blackout(true, "luaMoveToPh2", 3)` on every Think,
60 frames (3.0 s) apart, and 005B9BA0 re-arms +C8h to 3.0001 s on each call in the image as in the host
(docs/MISSION_BLACKOUT.md), so the fade never completes. V1 OFF `local\p8_v1off` (dc07a0286 with
`kPlaneSpeedFloorBound` false, exe 2D59608AE80E, `local\p8u4_v1off.log`) against V1 ON `local\p8_off`
(`local\p8u4_n.log`): carrier-squadron deaths with no killer (`local\p8_crash.py`) **46 OFF (Zuiho 40, Yorktown 6,
at 3-20 m from 277 s on) and 0 ON**; plane water contacts 61 -> 36; shots 15319 -> 25374. V1 is confirmed as the
fix for the climb-out crashes cc13-ver17 found. Neither side ends the mission in 1000 s.

## 28. C2, the ctl+30h velocity low-pass, bound OFF (cc9-physics8, 2026-10-14)

Packet cc9_plane_velocity_lowpass, switch `kPlaneVelocityLowpassBound` in `src/game_hosts_units.cpp`, committed OFF.

- **Producer, 007DC743-007DC80B** (listing re-read; the cc13 model `velocity_lowpass_007dc762`): with `step < 0.25f`
  (00CE3868, JB), `ctl+30h = ((0.25 - step) * ctl+30h + step * ctl+18h) * 4.0` (doubles 00D7A348, 00D7A328, every
  product and sum stored as a float), else `ctl+18h`. ctl+18h is the relative velocity, as the host's
  `plane_world_velocity` is. Other writers (disp-0x30 store census over 007D7000-007DE000): 007DB1F0 (from
  `BSP_Plane_ApplyControlStateMessage`) and 007DB2C0 (spawn: `ChooseSpawnFlightState`, `PlaceOnLaunchSpotLocked`,
  `ReadPropertyBag`) copy ctl+18h into it; 007D9CE0 copies it at a re-parent; 007D8430 zeroes it.
- **Readers** (unit+AE0h = ctl+30h, disp32 census `E0 0A 00 00`):
  - 007C18B0 (the attitude refresh): when `|ctl+18h| > 0.1` (double 00D7A3A0), unit+C7Ch = atan2(v.y, |v.xz|) and
    **unit+C84h = atan2(lp.y, |lp.xz|)** (each horizontal length 0 unless its square exceeds 1e-10, 00CE3820); then
    C80h and C88h are their rates. Pitch mode 2 holds C84h (0099DD0A); the host used the live velocity's angle.
  - 0099DA97-0099DAC8 (throttle): `error -= (unit+B1Ch - |unit+AE0h|) * 20.0 (00CE3D88) * pending`. unit+B1Ch is
    ctl+6Ch, which 007D8020 (`RefreshBodyFrame`/`BodyToWorldVelocity`) sets to `|ctl+18h|` (007D807C). The host took
    the term as 0 because neither field has a literal-displacement writer; both are written through the ctl base.
  - 0099D51A-0099D593: `|B1Ch - |AE0h||` into [ESP+48h] of the planner. Not carried (its consumer is unread).
- Host: `plane_velocity_lp_30` filtered after the integrate, copied at both re-parent sites; C84h in
  `refresh_attitude_007c1900`; the two readers. SUBSTITUTION: the filter seeds from the velocity at a slot's first
  core step instead of at spawn and at 007DB1F0.

**Predictions (before any run).** OFF: counters 0, gameplay identical to main. ON: `steps` on every core step;
`c84_reads` wherever a plane holds pitch mode 2 (torpedo runs, USN01/PRCP01 more than USN12); corrections on every
throttle tick, small in steady flight (|v| - |lp| is about 4*dt*accel, so 20x that times pending stays well under the
0.83 dead band) and larger in dives and pull-outs. Expected verdict 3 on plane missions (throttle and torpedo-run
pitch move, RNG re-rolls), with torpedo-run timings shifting by a few frames.

**Pairs (cc9-physics8, 2026-10-14).** OFF `local\p8_off` (a103f270a, exe DD0CD04DFDFF), ON `local\p8_c1` (the same
with the switch, exe 8A53591B1821); reference env; logs `local\p8c2_{off,on}_<row>.log`, diffs `local\p8pd_c2_*`.

| Row | OFF | ON | verdict |
|---|---|---|---|
| USN01 `usn01_p8_u1_c` | completed 1094.94 s | **no end**: Convoy2 survives (5 convoys sink by 208 s); torpedo-task releases 2/7 -> 6/14 | 3 |
| USN12 | completed 1510.35 s | completed 1510.35 s; deaths identical, gunnery and ship ranges move | 3 |
| LOMP10 | completed 1396.79 s | completed 1631.02 s; deaths 111 -> 126 | 3 |
| PRCP01 | **failed** 1491.83 s ("usn03.obj_p2_fail", Enterprise) | no end | 3 |

- Mechanism as predicted: `c84_reads` 54693-130412 per mission, corrections on every throttle tick with mean
  |correction| 0.25 m/s (USN12, PRCP01) to 0.80 m/s (USN01). The error it corrects is cmd+2B4h minus the measured
  speed (0099DA52-0099DA85), in m/s.
- PRCP01 is lost on main before C2: a103f270a with R4 and C1 also OFF (`local\p8_r4` re-exported, exe F53C93043F09,
  `local\p8rc_off_p01.log`) is gameplay-identical to the OFF row. First divergence against the win on f83e91891
  (`local\p8w1_p01.log`): Kate1's strike dies 30 s later (147.20 -> 177.46 s). Commits in that range include
  a7a20bc52 (kAirOpsOwnerByInstanceBound, kBaseLaunchBrakeAttachGateBound) and 4cbd5b2cb (kDogfightAimLeadBound).
- USN01 re-plans for C2 ON: `local\p8_u1_f.txt` (c with ConTBD1's last retarget to Convoy2) completes at 385.03 s
  and `local\p8_u1_g.txt` (ConTBD2 to Convoy2 at 3413) at 360.04 s, both by the strike; on OFF both complete
  (1094.94 s, 1155.00 s). Both fail under jitter 20,3 on ON (Convoy5, or Convoy3 and Convoy5, survive). Re-plan
  continues.
- **Flipped ON** (faithful; the USN01 loss is re-planned by the keeper), in the commit that adds this note.
- The third reader, 0099D51A-0099D593 (`|B1Ch - |AE0h||` into the planner's frame+48h), has no consumer: a sweep of
  0099D300-0099EA00 with the ESP adjustments tracked (`local/output/disasm-raw-0099d300-length-0x1700-233702.txt`)
  finds only writes to frame+48h after 0099D593 (0099DBEE and 0099E51F address other slots under a push and a
  `SUB ESP,8`), and 0099E180's `[ESP+5Ch]` is the double stored at 0099E136 under `SUB ESP,14h`. So the value is
  dead and nothing is lost by not carrying it (uncertainty: by-eye ESP tracking over a 6 KB body).

## 29. cc9-physics8 handoff (sea/physics lane + WINS KEEPER), 2026-10-14

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1), everything committed; no lease held.
Scripts in local\: `p8_wins.ps1 -Tag <t> [-Exe <exe>] [-Only keys] [-Jitter 20,3] [-Max 3]` (every win, absolute
order paths, at most -Max runs of mine at once, longest first), `p8_runs.ps1` (rows), `p8_keep.py <tag>` (summary),
`p8_deaths.py <a> <b>` (death rows side by side, first divergence), `p8_crash.py` (carrier-squadron crashes),
`p8_c1r4_apply.py` / `p8_c2_apply.py` (the units.cpp patches as applied). `local\p7_m` is the export of the last
re-time (a2ad905f1).

Done this session:
- Re-time on f83e91891 and again on a2ad905f1 (WINS_TABLE). USN01 re-planned twice: `usn01_p8_u1_c` (flak/AA flips)
  and `usn01_p8_u1_i` (C2); i fails under jitter 20,7.
- R4 (`kPlaneDeadMeatRollBound`) and C1 (`kPlaneWreckForwardDragBound`) bound, paired, ON (section 27). R4's spin
  half has no reach: the host never draws a spinning death (unit+800h, 007CFDB4, not carried; gunnery/plane-damage).
- USN04 V1 confirmed (section 27): no orders at 0.0222 s reach phase 2; V1 removes 46 climb-out crashes.
- C2 (`kPlaneVelocityLowpassBound`) bound, paired, ON (section 28): the ctl+30h low-pass, the C84h path angle that
  pitch mode 2 holds, and the throttle error correction at 0099DAC8. The third reader is a dead store.
- P1 (007D9CE0's rotation into the parent frame) needs nothing: `carrier_deck_capture` / `carrier_deck_carry_in`
  already keep the relative velocity in carrier axes.

Open, in order:
1. **JM08 lost to C2** (WINS_TABLE): f, m10 and f at 140000 frames all leave "landingships" and "bombing"
   incomplete. Needs a re-plan from the C2-ON logs (`local\p8w2_j8*.log`).
2. **PRCP01 c lost on main a103f270a**, not by the plane-flight switches (identical with R4/C1/C2 OFF). Kate1's
   strike dies 30 s later. The lead was asked to route it to the owning lane (suspects a7a20bc52, 4cbd5b2cb,
   827fc0327); re-plan it if the change is kept.
3. **JM06 n1** (lost on f83e91891, sub plan) and **USNOS k**: proposed for cc9-ships48; check the lead's answer.
4. **ESMP08**: Iowa kills Zuikaku by gunfire from 467 s and the launch rate falls with the carriers. The Iowa-first
   allocations (`local\p8_e8_iowa.txt`, `p8_e8_iowac.txt`, identical results) sink 13 of 15 by 4475 s; Cummings,
   Woodworth, Fanning and Grayson survive. Keeping the carriers alive (orders to move them away from Iowa) is the
   next idea.
5. USN01 i's 20,7 failure (Convoy2 and Convoy4 survive): a variant with a second weapon on Convoy2/4 would add
   margin.

**Addendum to 29 (lead's routing, 2026-10-14).**
- Open keeper items: **PRCP01** `l55_p01_c` was lost to cc9-lua58's a7a20bc52 (Junyo owner-by-instance and the brake
  attach gate, faithful, staying ON; SQUADRON_LAND_TASK 5gy). Junyo and Zuiho now launch 7 squadrons, and Enterprise
  sqn01's brake releases at 59.80 s instead of 30.90 s, so plan lines 4-5 are refused by the 0043F080 gate. This is
  the successor's TOP item: re-plan it on current main. The f83e91891 rows are the pre-flip baseline. **ESMP08** is
  parked until the plane-AI flips settle. **JM06 / USNOS** went to cc9-ships49. **JM08** is with cc9-lua59.
  **USN01**: 81397688d's dive flips refuse the opening "release ScoutDauntless ... within 20" line, so expect
  `usn01_p8_u1_i` to be lost; re-plan after PRCP01, and record the best knife-edge plan rather than chase margin.
- Not chased: the consumer of 0099D558 (recorded above as a dead store, by-eye ESP tracking). Still open if anyone
  needs certainty.
- Frame budgets used (0.05 s frames, `--frames` = budget + 200): USN02 16000, LOMP06 16000, USN01 26000, PRCP01 30000,
  USN12 36000, LOMP10 36000 (it needs at least 34000: it completes at 1581.97 s), BSM01 40000, IJN18 64000,
  USN15 90000, JM08 110000, JM06 120000, ESMP08 140000, USNOS 200000. USN04 V1 pair: 45000 at 0.0222 s.
- Keeper notes not yet re-timed: kTorpedoBoatRudderTorqueBound (BSM01 833.92 s, JM08 1907.44 s by ships48) and
  kLandMasterDamageBound 1f3bf3d62 (check rows with land targets).

## 30. The third C2 reader has a consumer (cc9-physics9, 2026-10-14; corrects section 28)

Section 28 recorded 0099D51A-0099D593 (`|vt[38h]() - |ctl+30h||`, the C2 low-pass velocity) as a dead store. It is
not. Evidence (Ghidra listing of 0099D300, `bsp.py ghidra disasm 0099d300`; decompile of the same):
- Stack depth. 0099D300 `SUB ESP,58h; PUSH ESI; PUSH EDI`; the early return pops them at 0099D3BD-0099D3BF; the main
  path pushes EBX and EBP at 0099D3DA/0099D3DB and pops them at 0099EA66/0099EA67, before `POP EDI` (0099EB94),
  `POP ESI` (0099EBA5) and `ADD ESP,58h` (0099EBA6). So `[ESP+48h]` at 0099D54B..0099D593 (EBX/EBP pushed) and
  `[ESP+40h]` at 0099EB54 (popped) are the same slot, entry-20h. Ghidra's decompiler merges them as `fStack_20`:
  written at 0099D54B (vt[38h]), 0099D564/0099D593 (the absolute difference) and read once, at 0099EB54.
- The consumer, 0099EB28-0099EB81, at the end of every think: when the two earlier small-value tests pass
  (plan+280h/284h by plan+288h, plan+298h/29Ch by plan+2A0h, each `< [00D1F3C8]`), 0099B260(plan+28Ch) through
  0042BE90 is `< [00D1F3C8]` and `fStack_20 < [00CE3958]` (2.0), it writes
  `unit+520h = ([[00E188A8]+5FCh]+908h == unit+54h)` (0099EB5B-0099EB73) and `plan+2ECh = [00E0E2EC]` (0.24,
  0099EB79/0099EB81).
- Reach. 0099D431 clears unit+520h at the head of the same think, so for any plane that is not the player record's
  selected unit the tail writes the 0 again: no change for AI planes. For the selected plane, a settled flight
  (speed within 2 m/s of the filtered velocity, the other terms small) publishes unit+520h = 1, which the host
  reads as "not an AI plane" (game_hosts_units.cpp: 0099C270's fast path, 00953D84, 009A17F8). plan+2ECh is
  rewritten at the next think's head (0099D446, or [00E0E2F4] at 0099D466 when unit+900h is 4 or 5) before this
  function reads it; a reader outside 0099D300 between thinks was not searched.
- Host: not carried (no host site writes unit+520h for a plane, nothing reads the C2 difference). Since the harness
  gives no player input, carrying the selected-plane hand-over would leave a `select`ed plane on neutral controls;
  no committed win selects a plane for long (USN01's `release ScoutDauntless` line is refused). Left unbound;
  candidate packet for the plane lane: bind the tail behind a switch, OFF, with the selected-plane prediction above.

## 31. `kPlaneWandererFloatTrigBound` pair (cc9-physics9, 2026-10-14)

cc13's a0ae654d7 puts 00439950's float rounding of FCOS/FSIN behind `kPlaneWandererFloatTrigBound` (OFF). Listing
check (`bsp.py disasm-raw 004399A0`): `FLD [esp+4]; FCOS; FSTP dword [esp+8]` (004399BD/004399BF), `FSIN; FSTP dword
[esp+0Ch]` (004399C7/004399C9), then `FLD [esp+8]; FLD [esp+14h]` (r, stored as a float at 004399B5), `FMULP`
(004399E2) into [esi], `FMUL dword [esp+0Ch]` (004399EA) into [esi+4]; z goes to [esi+8] by MOVSS (004399DD). The ON
path matches; the OFF path multiplies r by the unrounded double cos/sin. ABI: __thiscall-like, ESI = out, returns
EAX = out.
Prediction (written before the runs): only the last ulp of the wander direction's x/y changes, so the summary line
`plane wanderer` keeps planes/steps/timer_draws until the first chaotic divergence; offset/speed/accel/displacement
move in late digits. No mechanism counter can fail; the verdict is by mechanism unless the wins move.
Pair: OFF = `local\p9_m` (exe 073805D36E49, the re-time logs `local\p9w1_<row>.log`), ON = `local\p9_wt1` (main
25b98b1fa with the flip), same launch lines.
Results (logs `local\p9w1_<row>.log` OFF, `local\p9wt_<row>.log` ON, reference env):

| Row | OFF | ON | pair_diff |
|---|---|---|---|
| USN02 `s36_u2_p7` | completed 200.06 s | completed 200.06 s | identical |
| LOMP06 `l50_l6_q6` | completed 376.83 s | completed 376.83 s | identical |
| USN01 `p8_u1_i` | completed 360.04 s | completed 360.04 s | moved |
| PRCP01 `p9_p01_k` | completed 536.54 s | completed 536.59 s | moved |
| LOMP10 `p3_l10_b` | completed 1187.58 s | completed 1144.54 s | moved (deaths 94 -> 90) |
| BSM01 `p6_b1_a` | EndMission 833.92 s | EndMission 833.92 s | moved |

Mechanism as predicted: `plane wanderer` keeps planes=52 (LOMP10) / 277 (BSM01), offset_max, speed_max and
accel_max; steps, active, timer_draws and displacement move by under 1% with the run length. LOMP10's first moved
row is a plane death at 97.70 -> 97.75 s (the killer changes), the chaotic divergence of a shared stream, not a
mechanism failure. **Flipped ON** (`include/bsp/plane_wanderer.hpp`); every win holds, guard logged on all twelve.

## 32. cc9-physics9 handoff (sea/physics lane + WINS KEEPER), 2026-10-14

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1), everything committed; no lease held.
Export `local\p9_m` = main 25b98b1fa (exe 073805D36E49); `local\p9_wt1` = the same with the wanderer flip.
Scripts in local\: `p9_wins.ps1 -Tag <t> [-Exe] [-Only keys] [-Jitter] [-Max]` (rows now point at
`prcp01_p9_p01_k` and `ijn08_p9_i08_w` with 100000 frames), `p9_runs.ps1`, `p9_keep.py <tag>`, `p9_story.py`
(PRCP01 orders/deaths), `p9_i08plan.py` (IJN08 generator), `p9_e8gen.py` / `p9_e8gen_f.py` / `p9_e8sum.py` /
`p9_e8launch.py` (ESMP08), `p9_traj.py` (trajectory CSV sampler: `--trajectory-csv <path>` on the game).

Done this session (commits 42388ec7c, 010357cbc, 64381d75b, 1a6bc9f97, ad8ef14ce and this one):
- PRCP01 re-planned: `prcp01_p9_p01_k` (moveto-self then attack Zuiho on Enterprise slot 3), 536.54 s, robust to
  jitter 20,3/20,5.
- Re-time on 25b98b1fa (WINS_TABLE): all hold except IJN08; USN01 i holds (360.04 s).
- IJN08 lost to 91d6a9d22 (phase 1) and the plane flips (phase 2); re-planned `ijn08_p9_i08_w`, 4551.09 s, knife-edge
  (fails 20,5). Attack lines on dead targets are APPLIED (correction committed).
- Section 30: the third C2 reader has a consumer (settled tail, 0099EB54); not carried.
- Section 31: kPlaneWandererFloatTrigBound flipped ON by mechanism.

Open, in order:
1. ESMP08 (WINS_TABLE, last entry): best is f, 13/15. Ideas: resolve f's per-cycle targets from f's own log (it
   diverges from b after frame 44100) and iterate; start the surface strike earlier while the carriers' Zeros still
   cover it; keep two carriers alive by holding them farther north-east once the stock is gone.
2. IJN08 margin: `w` fails under 20,5 (Marus sunk by 1365 s). Marus are the only target of the airfield manager
   (ijn_8_north_sol.lua:543-565); kill US_Airfield_03/02 earlier or keep the fleet AA tighter around them.
3. The settled-tail packet of section 30 for the plane lane (OFF switch, selected-plane prediction).
4. Keeper notes still not re-timed: kTorpedoBoatRudderTorqueBound and kLandMasterDamageBound (check land rows).

## 33. Packet: the settled tail of 0099D300 (cc9-physics10, 2026-10-14; for the plane lane)

Self-contained packet for section 30's open item. Suggested id `cc9_plane_settled_tail`, switch
`kPlaneSettledTailBound`, committed OFF. Nothing below is bound yet.

**Image site.** `0099D300 BSP_PilotBot_PlanControls` (__thiscall, ECX = bot/plan, one float dt, `RET 4` at
0099EBA9; body 0099D300-0099EBAC, INT3 padding from 0099EBAC). Inside the body ESI = plan, EDI = unit. Listing
(`bsp.py disasm-raw 0099EA84 --length 300`, disk bytes):
- Head clear, 0099D431: `MOV byte [EDX+520h], 0` (unit+520h = 0 on every think that passes gate A at 0099D309).
- Tail gates, 0099EA84-0099EB59, every one a fall-through to 0099EB8C on failure (the shared exit that stores 0.12
  to plan+260h at 0099EB95 and XMM2 to plan+264h at 0099EB9D):
  1. `plan+26Ch` (byte) non-zero (0099EA84).
  2. `unit+908h` (float, spilled to [ESP+64h]) is below 3600.0 (`[00CE3D70]`, double) or above 3602.0
     (`[00D1F3D0]`, double) (0099EA91-0099EAB5); inside [3600, 3602] the tail is skipped.
  3. `|yaw slot|` < 0.035 (`[00D1F3C8]`, float): the slot value is plan+284h when the byte plan+288h is set, else
     plan+280h; the absolute value is `x > 0 ? x : -0.0 - x` (XMM2 = 0 from 0099D5AC/0099D5FF/0099D70E, XMM4 =
     `[00D7A208]` = -0.0) (0099EABF-0099EAF6).
  4. `|pitch slot|` < 0.035: plan+29Ch when plan+2A0h is set, else plan+298h (0099EAFC-0099EB26).
  5. `|0099B260(plan+28Ch)|` < 0.035, the roll slot's value through 0042BE90 (0099EB28-0099EB4A).
  6. `[ESP+40h]` < 2.0 (`[00CE3958]`) (0099EB4C-0099EB59): the C2 difference `| |vt[38h]()| - |ctl+30h| |`
     written at 0099D54B/0099D564/0099D593 (section 30 has the stack-depth proof that [ESP+48h] there is this slot).
- Writes when all six pass: `unit+520h = ([[00E188A8]+5FCh]+908h == unit+54h)` (0099EB5B-0099EB73: the local
  player record's party/id field against the unit's), and `plan+2ECh = [00E0E2EC]` = 0.24 (0099EB79-0099EB81).
- Section 30 omitted gates 1 and 2; they are listed here from the listing and are part of the packet.

**Host.** Not carried. `generic_suppress_520` (game_hosts_units.cpp:384) is the host's unit+520h; its readers are
the 009A17F8 avoidance gate (:7753), 007CE6CA (:14639), the bot fast path (:33629) and the ship exit (:35478). No
host site clears it at 0099D431 or writes it from this tail. The C2 low-pass is carried (`kPlaneVelocityLowpassBound`,
ON), but the difference at 0099D564/0099D593 is not kept. Insertion point: the end of the host's 0099D300 order,
just before the `kBandShaperPlan264Bound` block (game_hosts_units.cpp ~33652, the 0099EB9D zero), which keeps the
image's order (tail, then 0099EB95/0099EB9D). Plan slots: `bsp::kPilotSlotYaw`/`Roll`/`Pitch`
(include/bsp/pilot_plan_slots.hpp); the plane-lane owner must map plan+280h..2A0h and +2ECh to the host's slot
fields (the yaw/pitch `desired` fields are written at :8246/:8251) and confirm what 0099B260 returns.

**Correction to section 30 (reach).** The compare is a PARTY test, not a unit identity: unit+54h is the unit's
party (AIR_OPERATIONS.md: `Party` = owner+54h; AI_PLANNER_TAILS.md teams 0-2), and `[[00E188A8]+5FCh]+908h` is the
same record the 009A17F8 gate reads as the local player's party (game_hosts_units.cpp:7749). So the tail publishes
unit+520h = 1 for EVERY plane of the local player's party whose mode byte plan+26Ch is non-zero and whose flight is
settled, not only for a selected plane. unit+520h is also the formation byte of BOMBER_AFTER_TASK.md section 6c:
it is published per step to `unit[9C2h + idx*8]` (007CDCD0), the follow state 009BEE30 sets a member's +520h at
009BF0B8 when its leader's published byte is set (009BEF09), and 0099D300's gate A (0099D33D/0099D345) reads it.
The writer census there (`00699B14`, `007B959B`, `007BBCF8`, `00959454`, `0099D431`, `009BF0B8`, `00698716`,
`0095CDD7`, `0099EB73`) is the starting point.

**Prediction (to write into the lane doc before the pair).** Enemy-party planes: the compare gives 0, which the
head clear already wrote, so nothing moves for them. Player-party planes with plan+26Ch non-zero and a settled flight
(all six gates): `generic_suppress_520` becomes true until the next think's head (0099D431) clears it, so for the
ticks in between 009A17F8's avoidance, 007CE6CA and 009998A0's fast-path test (0099C270) see a set byte, and the
follow states of that plane's members may set their own +520h (009BF0B8, if the host carries it). plan+2ECh = 0.24
is rewritten at the next think's head (0099D446, or `[00E0E2F4]` at 0099D466 when unit+900h is 4 or 5) before
0099D300 reads it; a reader outside 0099D300 between thinks was not searched (search it first). Expected movers:
missions with player-party planes in long settled flight (USN01, PRCP01, LOMP10, BSM01, ESMP08's Zeros); counters to
watch: the avoidance gate's entries and the fast-path count. A moved row must trace to a player-party plane with
plan+26Ch non-zero; a moved enemy plane is a mechanism failure (OFF, recorded). Pair rows: USN02 and LOMP06
(identical expected), USN01, PRCP01, LOMP10, BSM01.

**Uncertainty.** The meaning of unit+908h (gate 2) is unknown (listing only). `[[00E188A8]+5FCh]+908h` as the
local player's party is BOMBER_AFTER_TASK's provisional reading; the host's stand-in must be named before binding
(the 009A17F8 comment at :7749 uses the controlled unit's party, labelled). plan+26Ch is task+270h, the mode byte
gate A tests against 2 (BOMBER_AFTER_TASK.md: `009998CD LEA EDI,[ESI+4]`); which states leave it non-zero decides
the reach and must be listed by the packet owner.

## 34. Carrier runway/hangar failure: the launch gate is the carrier's own block (cc9-physics10, 2026-10-14; read-only)

Lead item (C), from CC13_SEG30 adoption note 1. Read from disk bytes (`bsp.py disasm-raw`, `scan-bytes`); no code
changed. **Correction to CC13_SEG30 note 1:** on a carrier the air-ops block is not at `+72Ch`. The mothership
constructor `00758550` builds it at `+1188h` (`00758578 LEA EDI,[ESI+1188h]`, then `00758589 CALL 006CAC00`, the
block constructor that zeroes block `+1Ch` / `+1Dh` at `006CAC37` / `006CAC3A`). So the bytes `006BF620` reads for a
carrier are `unit+11A4h` (runway) and `unit+11A5h` (hangar), and those **are** set:

- **Set.** `007583A0 BSP_Mothership_SetFailureFromMessage` (MMothership override of slot `+220h`): message kind 72h
  writes `unit+11A4h = 1` (`007583B7`), kind 74h writes `unit+11A5h = 1` (`007583D2`), else `008198A0`.
- **Clear.** `007583F0 BSP_Mothership_ClearFailureFromMessage`: kind 73h clears `+11A4h` (`00758407`), 75h clears
  `+11A5h` (`00758422`).
- **Senders.** Three mothership vtable methods (vtable base `00D01630`):
  - `+194h` = `00758750` (dword at `00D017C4`), the Lua `SetFailure` virtual (`0088F14C CALL [vtable+194h]`).
    `"RunwayFailure"` (`_strcmp` against `00CF8E24`) builds message 72h (`0075B430`, class `00CF8B40`) and routes it
    with `0077C2A0(msg, 7, 0)` (`007587D5`); `"HangarFailure"` (`00425850` against `00CF8E14`) builds 74h (class
    `00CF8B68`). **The duration argument is not read on either arm**; any other name goes to the ship base
    `008132C0` with it (`00758860`-`00758868`).
  - `+19Ch` = `00758A50` (`00D017CC`): the same two names build 73h / 75h (`00758A91`, `00758B00`); other names go to
    `00813660`.
  - `+21Ch` = `00758890` (`00D0184C`): the same 72h / 74h arms; other names go to `00827B90` (the damage-section
    failure effect).
- **Readers of `+11A4h` / `+11A5h`** (`scan-bytes '?? ?? a4 11 00 00'` / `'?? ?? a5 11 00 00'`, the landing-ship hits
  in `0074xxxx` are a float field of another class): the setters and clearers above, the constructor `00758370`,
  the GetFailure-by-name getter `007589D0` (`007589F6` / `00758A19`), and, through the block, `006BF620`
  (IsReadyToSendPlanes, block `+1Ch` / `+1Dh`).
- So in the image `SetFailure(carrier, "RunwayFailure")` closes that carrier's launch gate until something calls
  `+19Ch` with the same name. No script in this installation calls `ClearFailure`/`RepairFailure` with these names
  (only "EngineJam": `02_MI.lua:1464`). **Not traced:** who calls `+19Ch` / `+21Ch` on the damage-section path
  (`shipglobals.lua` runway/hangar sections, `FailureDuration` 20); the repair task `0093BED0` / `0093C520` is the
  candidate.
- The airfield is different (CC13_SEG30 note 2): its block is at `+72Ch` and its own handlers only clear.

**Reach.** `bsm_09_battle_of_the_coral_sea.lua:1203` `SetFailure(Mission.Lexington, "RunwayFailure", 180)` (inside
the scripted sinking, with `OverrideHP(0.01)`), so Lexington stops launching for good; `ijn/ESMP/03_south_pacific.lua:1246`
`SetFailure(Mission.Zuiho, "RunwayFailure")`, so Zuiho stops launching for good. In the host both are counted as
"other" by `run_set_failure_0088eff0` (game_hosts_lua.cpp:7517) because `script_set_failure_008132c0` knows only
the four ship arms. Damage-section failures on carriers reach the host's `apply_failure_effect_00827b90`
(game_hosts_gunnery.cpp:12344), which ignores the two names.

**Packet (not started).** Proposed switch `kCarrierRunwayFailureBound`, OFF. In `run_set_failure_0088eff0`, for a
unit with an air-ops deck (a carrier, not an airfield), map "RunwayFailure" / "HangarFailure" (exact-case strcmp
for runway, `00425850` for hangar: check its case rule) to `deck.runway_failure` / `deck.hangar_failure` = true,
ignoring the duration; the same in `apply_failure_effect_00827b90` for the section path once its clear is traced.
Files: game_hosts_lua.cpp (lua lane), game_hosts_gunnery.cpp (lease queue), air_operations deck fields (already
read by `air_ops_is_ready_to_send_planes_00895d20`). Predictions: BSM09 and ESMP03 move from the SetFailure line on
(the carrier's later launch lines refused at the readiness gate; AI launches through 006CD810 stop too);
ESMP08 and every row without a carrier runway hit stay identical until the section path is bound. Pair rows: BSM09,
ESMP03, ESMP08 (control), USN04 (control).

## 35. cc9-physics10 handoff (sea/physics lane + WINS KEEPER), 2026-10-14

Tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (agent/cc9-physics1), everything committed; the lane-doc
lease `p10_lane_docs` is released with this commit. Export `local\p10_m` = main 610dc9bfd (exe 349115AD7B28).
Scripts in local\ (all `p10_`): `p10_e8gen.py` (ESMP08 launch re-preference), `p10_stock.py <log>` (per-carrier stock
ledger: AddAirBaseStock, launched, returned, first no-stock refusal), `p10_sqalive.py <log>` (carrier squadrons with a
live member at the end), `p10_latchfate.py <log>` (spent-ordnance latches without a B5 and the members' deaths),
`p10_rtb.py`, `p10_kinds.py <log> <word>` (line shapes), `p10_prog.py`. Run launcher: physics9's `p9_runs.ps1`.

Done (commits 3f09792a4, 7d613369d and this one):
- Section 33: the settled-tail packet (0099EA84-0099EB81) for the plane lane; the reach is party-wide (correction).
- ESMP08 handed to lua61 with r2's results (5018.40 s; 4500.43 s at 20,5; no end at 20,3).
- Slot state 3 (lead item 1): no host gap. Held slots are live Zero squadrons (009F8160 excludes fighters, image);
  Zuiho/Chiyoda are out of stock. The faithful release is the player's recall, `order <carrier> <slot> 2`
  (untested). Torpedo squadrons that latch without a B5 are all shot down within 60 s; 009D4C10 reads nothing the
  009D2720 disarmed arm writes (kTorpedoDoneDisarmedArmBound is ON).
- IJN08: `ijn08_p10_i08_z` committed (plain 3006.81 s, 20,3 3947.73 s, 20,5 no end, 20,11 fail); w passes 20,5 and
  20,7 but fails 20,3 and 20,11 on main 610dc9bfd. Neither is robust: the Marus fall to airfield 02/03 squadrons at
  1000-1600 s. Untried: holding the Marus inside the battleship screen until 03 dies.
- Item A (catapult spectate): no change possible until the catapult launch chain (008141E0 .. 00957450) is hosted.
- Item B (recon listener sync): already ON on main (kForcedReconSyncNotifyBound, kReconListenerResetCycleBound).
- Item C: read in section 34; the packet is written there, not started.

Open, in order (the lead's queue of 2026-10-14; IJN08 margin work is closed, z is on main cbd691a6a):
1. (B) recon listener sync: nothing left; both switches are ON on main (above).
2. (C) section 34's packet (`kCarrierRunwayFailureBound`), and the damage-section caller of vtable +19Ch/+21Ch.
3. (D) the ship catapult launch packet: no ship ever catapults a plane in the host. Chain 008141E0 (reconstructed,
   cc13_seg30_seg50.cpp:458) -> 006EB680 queue -> 006ED1C0 / 006ED261 timeline (gameplay_seg33_cc13.cpp:386-462)
   -> 00957450 (unbuilt: the slot +550h + 18h*i and +630h write, the stock decrement), plus SetCatapultStock
   009539A0. Then GetLastCatapulted (game_hosts_lua.cpp:6937) answers from +630h, and the HUD hook
   `turn_timer_expired_009539e0` (game_hosts_hud.cpp:2361) should be renamed to the spectate-last-catapulted
   009539E0 and wired (cc13_seg62.cpp:171). Grep before reconstructing. OFF switch; pair JM02 / JM03 / JM05 (Pete
   floatplanes: jm02.lua:2444, jm03.lua:1587/1615, jm05.lua:5479/5498); watch the RNG and keyed streams; flip by
   mechanism.
4. Recall-the-Zeros order lines for ESMP08: lua61 is running them (l61_e8_n / l61_e8_o).

## 36. `kCarrierRunwayFailureBound` (cc9-physics11, 2026-10-15; section 34's packet)

Switch in `include/bsp/mothership_failure.hpp`, committed OFF. Three arms, all gated on `IsKindOf(09h)`
(MMothership) and the case-blind names `RunwayFailure` (`_stricmp` 00BF7FBF) / `HangarFailure` (00425850):

- **Lua.** `run_set_failure_0088eff0` (game_hosts_lua.cpp) takes the two names before the ship base, as
  vtable `+194h` 00758750 does, and sets `deck.runway_failure` / `deck.hangar_failure` (block `+1Ch` / `+1Dh`,
  unit `+11A4h` / `+11A5h` through 007583A0). The duration is ignored. Log arm 10 / 11.
- **Section set.** The traced damage-section caller of `+21Ch` is `0093BED0` (`0093C0E5 MOV EDX,[EDX+21Ch]`; also
  `0093C300` at `0093C4DA`, not hosted). On a carrier `+21Ch` = 00758890 takes the two names before 00827B90, so a
  `runway` / `hangar` section failure (shipglobals.lua rows 3/4, FailureDuration 20, this installation 2024-07-13)
  sets the flag. The host's FailureBinding `unit_on_failure` now routes them there.
- **Section clear.** `0093C520` (`0093C622 MOV EDX,[EAX+19Ch]`) on expiry: `+19Ch` = 00758A50 sends 73h / 75h and
  007583F0 clears the byte. FailureBinding `unit_named_state_dispatch` routes them. The other `+19Ch` caller,
  `00958D2C` in 00958A30, passes `"InferiorFailure"` (00CF0B74), not these names. A scripted SetFailure keeps no
  record, so only a section failure of the same name ever clears it (and its expiry does clear it).
- The flatten now names shipglobals rows `RunwayFailure` / `HangarFailure` (effect 5 / 6) instead of `Failure#N`,
  in both switch states (log text only; the record name was already unique per row).
- Not bound: GetFailure on a mothership (007589D0) still answers false for the two names (no script asks).

Predictions. BSM09 and ESMP03 move from the SetFailure line on (Lexington at bsm_09:1203 inside the scripted
sinking, Zuiho at 03_south_pacific:1246/1400): the carrier's later launches are refused at the readiness gate
(006BF620 / 006CD810 paths). Any row in which a carrier takes a runway/hangar section hit moves too: a 20 s launch
blackout per section failure. ESMP08 and USN04 stay gameplay-identical unless their logs show `component failure
RunwayFailure|HangarFailure` lines on a carrier.

**Pair (commit 9a51e05d8; exports `local\p11_off` / `local\p11_crf`; reference env, 0.05 s, no orders unless named;
logs `local\p11off_<row>.log` / `local\p11on_<row>.log` with manifests).**

| Row | Frames | Verdict | Note |
|---|---|---|---|
| ESMP03 | 60000 | 1, gameplay identical | `SetFailure "RunwayFailure" on "Zuiho" arm=0` -> `mothership 00758750 72h runway on deck=yes`, arm=10. Zuiho never launches in a no-order run (`base launch site Zuiho: sets=0`), so nothing moves. |
| USN04 | 45000 | 1, gameplay identical | control |
| ESMP08 `l61_e8_f` | 140000 | 0, identical | control; no runway/hangar section failure |
| BSM09 | 60000 | not run | the host cannot start BSM09: `startup failed: unit observer creator projection is unavailable: unit=NEW_Cloud 01` (StationaryClass row with no Type). Not this packet's gap. |

The section path is never reached in these rows: carriers do take component failures (ESMP08: Zuikaku 4, Zuiho 3,
Chitose, Chiyoda in `cc9-lua52\local\l61rt_e8_j3.log`), but none on a `runway`/`hangar` section (27 such sections in
this installation's vehicleclasses.lua). Whether hits ever resolve to kind 11 (runway) is open.
**Flipped ON by mechanism.**

## 37. Ship catapult launch: scoping for the packet (cc9-physics11, 2026-10-15; read-only, lead item 4)

What the host has (all built, none wired): `ship_launch_from_catapult_008141e0` (src/cc13_seg30_seg50.cpp:458, the
7Bh arm: ready = unit `+638h` > 0 and every catapult's `+1D0h(1)`; several catapults pick by 006ED000 heading),
`catapult_can_fire_006ebd30` / `catapult_spawn_properties_006ec8e0` / `catapult_after_launch_006ec070`
(src/air_operations.cpp), `catapult_fire_head_006ed1c0` / `catapult_launch_timeline_006ed261`
(src/gameplay_seg33_cc13.cpp:389/418), `unit_add_launched_child_00957450` (src/gameplay_seg61_62_cc13.cpp:411),
`ship_catapult_stock_clamp_009539a0`. The squadron creator to reuse is the kamikaze one,
`GameScriptOrdersHost::create_dummy_kamikaze_squadron_006fec00` (game_hosts_script_orders.cpp:965): MCatapult::Fire
006EC8E0 calls the same 004F0AD0 with the bag {Type = owner `+530h` or class `+C0h` LaunchedClass, WingCount 1, Skill
(vtable 12Ch), Race `+58h`, Party `+54h`, OwnerPlayer `+188h`, **State 3**, PlaneParentID `+174h`, Equipment `+534h`
or class `+C4h`, Behaviour 1, ResourceUsage launched class `+124h`} at the catapult node's world matrix (`+CCh`).

Chain: ShipUseCatapult 00891D50 (message 7Bh) -> 008141E0 -> 006EB680 (`+488h` startLaunch = 1, `+48Ch` = heading)
-> tick head 006ED1C0 (can_fire -> message ADh -> 006EC8E0 Fire: spawn, observer pair `+4A8h`, 006EC070 sets
`launchTick`) -> 006ED261 timeline (VEHICLE_ADD_LAUNCHED at t > 1 s -> ship 00957450: slot `+550h + 18h*i`,
`+630h` = slot, `+638h` decremented; takeoff at turn_time + 0.5 s with (0, 0, +4D4h) in the plane frame, about
45 m/s; PLANE_TAKEOFF(1)).

Data (this installation): ship class `Catapult = {Equipment, LaunchedClass, LaunchStock, MaxLaunchedPlanes}`
(e.g. LaunchedClass 121, stock 10, max 1); the catapult device class (deviceclasses.lua, `Type = "Catapult"`,
`Function = "CATAPULT"`) carries HorzRotSpeed / VertRotSpeed / HorzRotAngle / ReloadTime / PlaneStartPosY/Z; the
mapping of these keys onto class `+88h` / `+8Ch` / `+DCh` / `+E4h` / `+E0h` is NOT established (read the device
class reader before binding the timeline).

Gaps that make this bigger than one packet:
1. **The plane is spawned with State 3** (held on the catapult until the takeoff impulse). The host's plane control
   has no catapult-held mode; the nearest substitution is to create the squadron at the takeoff instant as State 7
   at the ship's pose plus (0, PlaneStartPosY, PlaneStartPosZ), at the takeoff speed. Label it.
2. **No catapult device instances** exist in the host (008141E0 walks the ship's device list `+48h`/`+44h`,
   IsKindOf 28h). A one-catapult-per-class substitution is needed, or the device list from the class Weapons.
3. **Natives.** GetNumCatapulted 00892AA0 and SetCatapultStock 00892C30 are unbound (`num < 4` in jm02.lua:2445
   then compares nil); GetLastCatapulted / GetCatapultStock answer constants. They must read one per-ship record
   (`+550h` slots cleared when the child dies, `+630h`, `+638h`) that the launch writes.
4. Files: game_hosts_lua.cpp (natives) and game_hosts_script_orders.cpp (spawn + per-step tick, next to
   `run_air_ops_update_006cdc70`) are both shared; put the record and tick in a new file and keep the shared-file
   hooks to a few lines. Rename `turn_timer_expired_009539e0` (game_hosts_hud.cpp:2361) to spectate_last_catapulted
   in the same commit.

Reach: jm02.lua:1490/2448 (Repulse and the recon check), jm03.lua:1626, jm05.lua:5493/5512 (the player's Pete);
ShipUseCatapult appears 19 times in this installation's scripts.

## 38. JM06 under jitter: the bisect and the re-plan `jm06_p12_j6_a4` (cc9-physics12, 2026-10-16; lead item 1)

**Bisect.** WINS_TABLE's suspect range fd6d92617..9228d13fe for the `jm06_s49_j6_s` loss under `BSP_FRAME_JITTER 20,3`.
Control exports (`tools/pair_export.py`, no flips) in `local\p11_bA`, `local\p11_bB`, `local\p12_bC`, and one flip export
`local\p12_bCtt`. Reference env plus 20,3, 120000 frames, `--stop-after-end 40`, logs `local\p12b<X>_j6j3.log` with
manifests:

| Build | Result |
| --- | --- |
| 76536f249 (bA) | completed 2901.71 s (the fd6d92617 win, exact) |
| ef0a50051 (bC) | no end: PlayerSub 01 sunk 1408.55 s by Fletcher-class 05 |
| 8612ef146 (bB) | no end: PlayerSub 01 sunk 1148.80 s by Fletcher-class 05 |
| ef0a50051 with only `kTorpedoBotThinkTimerBound = false` (bCtt) | completed 2901.71 s (exact) |

So the loss is `kTorpedoBotThinkTimerBound` (0cc305ee0, GUNNERY 245, flipped ON by its pair). The first moved death is
US Cargo Transport 02 at 171.96 -> 172.01 s, a one-frame shift of an AI boat's torpedo launch, and the run drifts from
there. The flip is faithful, so the plan is re-planned, not the switch.

**On current main (this tree at 25fbdcba0)** `s` completes the reference at 2144.97 s but has no end under 20,3, and
the failure is now earlier than the brief's: `MissionPhase` stays 1. `luaJM6CheckObjectives` (jm06.lua 839-876, this
installation 2024-07-13) needs every `Mission.CargoTargets` ship dead, and USTroopTransport 01 (a target; the reference
sinks it at 388.08 s) is only crippled (hull segments 7..3 from 1705 s) and stops. PlayerSub 01 launches nothing
between ~650 s and the Lexington phase, because the `torpedo ... at` lines need the target in reach.

**Variants (all 20,3; logs `local\p12j6_<v>_j3.log`).** u2 (more `torpedo at` rotations over the survivors 13300-20800)
and a2 (an `attack` rotation over five survivors): no end, Troop 01 survives. a1 (`attack` Troop 01 at 13010/16010/
19010): the sub closes and hits it with the deck gun only; the 21000 moveto then takes it away; no end. a3 (`attack`
Troop 01 every 1500 frames to 29010): no end. **a4** = a3 plus `torpedo at` Troop 01 every 600 frames 14100-29000: the
sub closes under the attack and the torpedo lines then reach it (Troop 01 sunk 862.91 s under 20,3). A line at a sunk
target is refused (0f70e9dee), so in the reference, where Troop 01 dies at 388 s, the extra attack lines do nothing.

| a4 (`config/win_orders/jm06_p12_j6_a4.txt`) | Result | Guard |
| --- | --- | --- |
| reference | completed 2226.05 s | yes |
| 20,3 | completed 2343.16 s | yes |
| 20,5 | completed 2175.00 s | yes |
| 20,7 | completed 2033.86 s | yes |
| 20,11 | completed 3180.98 s | yes |

Generators: `local\p12_j6attack.py` (attack rotations) then `local\p12_j6gen.py` (a copy of cc9-ships49's
`s49_j6gen.py`, the torpedo rotation).

## 39. ESMP08 recall: the cascade is hosted; dead planes' landing records are not compacted (cc9-physics12, 2026-10-16; lead item 2)

**The cascade.** In the image the order-2 recall (006CCDA0's state-3 arm) issues `land` 00E08FA0 through 0077D600 on
the squadron (ECX = slot+28h). 0077D600 routes a ` Mv` message to the squadron's `vtable[160h]` 007F1940, which issues
the land on the squadron's own director (+348h; cleared first when the message's +21h is set, else after its
`vtable[34h]` accept). The members are not addressed: each member's attack task retires when its predicate sees the
squadron command change (0099A4C0) and 0099A170's arm 0099A3DD installs `land` (SQUADRON_LAND_TASK 1). The host's
recall arm calls `land_at_site_0099a3dd` per member, and the leader's call fans out to every member: in
`cc9-lua52\local\l61_n_e8.log` every applied recall with live members logs `land command: ... 3 plane(s), 3 task(s)`
(the exceptions are planes not airborne, `land task REFUSED ... not airborne`). So the land does reach the wingmen;
nothing is bound for the cascade.

**Why the slots stay in state 4 anyway** (`local\p12e8_ra.log`, plan `local\p11_e8_ra.txt`, this tree 25fbdcba0, no
end at 140000 frames). The end-of-run `summary land task plane` lines show the recalled wingmen either dead in
land/final (5F8) or alive in land/standby (5B8) for thousands of seconds. Chitose_sqn14|.-3 is the clear case: from
881 s to the end it cycles standby -> begin -> final -> abort (263 state changes), and each final ends with the head's
sequencer mode flapping 4 -> 3 -> 4 at 120-300 m with spacing 0.000/0.010, then abort at about 16 m. Its squadron mate
Chitose_sqn14|.-2 died in final at 872.36 s (below), and `summary landing record Chitose_sqn14|.-2: mode=4 path=33.7`
is still in Chitose's record list at the end. Inferred, not traced: 006C3F80's k loop counts that record (mode 4, +4h > 0, `landing_airborne`
true for the wreck, its squadron link gone so the head skip does not apply), 70 m ahead of any survivor, so the
survivor's spacing is 0 near the deck, its mode drops to 3 and final aborts, for ever.

The image removes such records: **006C77E0** (`BSP_AirOpsBlock_CompactQueueRecords`, 006C77E0-006C7951, RET), the third
sub-update of 006CDC70, erases under the block's lock every +A8h record whose plane is null or has +5Ch clear or +5Dh,
+60h or +5Eh set (006C788D-006C78B6), shifting the tail down. CC13_VER_15 2 lists it as not hosted, and
game_hosts_script_orders.cpp says so at the queue call.

**The binding (`kAirOpsQueueCompactBound`, game_hosts_units.cpp, committed OFF).** At the head of the queue tick in
`run_landing_queue_006cd240`, every deck's `assignments` drop the records whose plane is gone, fails
`unit_alive_and_visible` (0043F080's four bytes) or is dead by the host's +5Dh stand-in (plane_death_c3a / removed /
gunnery dead, as the leg manager uses). LABELLED: the position (the image runs it before the site virtual, 006C64B0 and
006C6540, none of which reads +A8h). Each erasure logs `landing sequencer: record of <plane> erased`.

**The deck crash (not bound, recorded).** Chitose_sqn14|.-2's death at 872.36 s (`BSP_PLANE_RATE_TRACE`, `local\p12e8_ra_tr.log`):
on final at A8 about 190 m and 31 m altitude its pitch-rate target jumps from 0.01 to 0.35 rad/s with the stick at
full pull and the roll stick flipping between -1 and +1 every few steps; the nose rises to 1.15 rad, speed climbs from
33 to 73 m/s, and the crash sweep finds Chitose. At the time Chitose_sqn34 was on the deck in its takeoff run (raised
861.56 s, airborne 873.51 s), and the plane's `vehicle/terrain avoid` summary has plane=46 ship=6 ticks. The image's
landing gate 006C3F80 asks the carrier site's vtable[30h] = 006D0530 `HasFreeRunway` (vtable 00CF8A58, slot
00CF8A88; the host hardcodes the airfield's 006CF3F0 `true`), which fails only for an occupant within
MinFreeRunwayLength (12 m in this installation's planeglobals.lua, 2024-10-29) of T; a plane on the launch spot is
about 100 m past T, so 006D0530 would not have stopped this landing. Whether the image blocks a launch while a plane
is on final was not found; open.

**Predictions (before any ON run).** OFF: gameplay identical (nothing runs). ON: on rows where a plane in a landing
queue dies (ESMP08 with the recall plans; any carrier row with deck crashes), its record is erased at the death, and
survivors that cycled final -> abort land on a later pass: more `state 5F8 -> 620`, more `stowed plane stock return`
sends and `slot_returns`, fewer 5B8 survivors at the end, and launches that continue later into ESMP08. Rows with no
queue deaths: no erasures, gameplay identical (USN04 45000 control).

### 39.1 The pair and the verdict: ON

Exports of 6cfd0ff18: OFF `local\p12_off6c` (this tree's build), ON `local\p12_qc` (exe 7ADB78F84D1D). Reference env,
`--stop-after-end 40`, no jitter; logs `local\p12qc_<off|on>_<e8|u4>.log` with manifests; diffs `local\p12qc_diff_<row>.txt`.

| Row | pair_diff | Note |
| --- | --- | --- |
| USN04 45000 (no plan) | 1, gameplay identical | no erasures (no landing queue deaths) |
| ESMP08 60000 (`local\p11_e8_ra.txt`) | 3, moved | 17 erasures; first moved death Chitose_sqn14\|.-3 at 957.99 s (ON only) |

ESMP08 mechanism. The erasures are the stowed planes (mode 4, path -1.0, once the elevator has taken them) and the
planes that die in the queue (Zuiho_sqn15\|.-2 at 861.96 s path 34.8, Chitose_sqn14\|.-2 at 872.36 s path 33.7,
Chiyoda_sqn13\|.-2/.-3 at 1390/1402 s). With its mate's record gone, Chitose_sqn14\|.-3 no longer aborts: its first
pass after the erase (final at 939.14 s) reaches the deck and dies there at 957.99 s in the same deck crash as its mate
(crash sweep victim Chitose; Chitose_sqn37 was raised at 949.09 s and in its takeoff run at 955.74 s). So the predicted
"lands on a later pass" became "reaches the deck on the next pass": the cycle is gone, and the death is the separate
deck-crash gap of section 39. At 3000 s OFF has one landing plane alive (in abort), ON none; air-ops releases 67 -> 72, squadrons created 74 -> 78, US ship deaths the same eleven (Mobile, Grayson, Dunlap and
Santa Fe 2-37 s earlier). The mechanism is the image's, so the switch is **flipped ON**.

### 39.2 The deck crash is vehicle avoidance (diagnostic, nothing bound)

A diagnostic export of 6cfd0ff18 with `kAirOpsQueueCompactBound` ON and `kPilotVehicleAvoidanceBound` OFF
(`local\p12_qcva`, log `local\p12qcva_e8.log`, ESMP08 `p11_e8_ra` to 20000 frames; not a candidate, avoidance is the
image's) lands all three of Chitose_sqn14 (and all of Zuikaku_sqn16) with no crash sweep on any carrier, where the ON
build loses both Chitose wingmen on the deck. Each lost wingman's summary shows plane-arm avoidance ticks
(`vehicle/terrain avoid ... plane=46` and `plane=78`) where the leader, which landed on an empty deck, has `plane=0`;
both crashes happen while a new squadron is in its takeoff run on the same deck. So the landing plane is steered off
its glide by the plane-arm bands against the departing aircraft, near the deck and at about 31 m/s, while the direction
hold keeps forcing the velocity onto the glide path. Open: whether the image's 007DF4F0 lists a plane in a ground state
on a carrier deck (the host's neighbour refresh takes every in-world kind 0Fh), and whether the image holds launches
while a plane is on final. Until then the plans avoid launching from a carrier that is recovering planes only by luck.

## 40. ESMP08 re-planned: `esmp08_p12_e8_rb` (cc9-physics12, 2026-10-16; lead item 2)

cc9-physics11's three recall variants (`local\p11_e8_ra/rb/rc.txt`, made by `local\p11_e8recall.py` from
`esmp08_l61_e8_f`: slot-4 Zero launches thinned to every k-th 600-frame cycle, and `order <carrier> 4 2` d frames after
each kept launch; ra k=20 d=9000, rb k=24 d=10000, rc k=16 d=7000) on this tree. Reference env, 140000 frames,
`--stop-after-end 40`.

| Plan | Exe | reference | 20,3 | 20,5 | 20,7 |
| --- | --- | --- | --- | --- | --- |
| f (lua62, `cc9-lua52\local\l62rt_e8_*.log`) | 25fbdcba0-equivalent | completed 6532.47 s | no end (13/15) | - | - |
| ra | 25fbdcba0 (`local\p12_m`) | no end (13/15: Washington, Cummings) | - | - | - |
| rc | same | no end | - | - | - |
| rb | same | completed 4684.71 s | completed 3594.58 s | completed 4624.95 s | - |
| **rb** | **135f8ae26** (`local\p12_m2`, compaction ON) | **completed 4684.71 s** | **completed 3594.58 s** | **completed 3251.20 s** | no end (12/15: Washington, Fanning, Woodworth) |

Logs `local\p12e8_<plan>[_j<n>].log` (first exe) and `local\p12e8b_rb_<ref|j3|j5|j7>.log` (135f8ae26), with manifests;
every completion logs the exec guard's sus_prog.exe refusal. On 135f8ae26 the rb reference erases 6 landing records and
moves in small ways (`pair_diff` 3 against the first exe) without moving the reference or 20,3 end time (20,5 moves 4624.95 -> 3251.20 s). Committed as
`config/win_orders/esmp08_p12_e8_rb.txt` (rb unchanged, one header line added): it holds the reference, 20,3 and 20,5, where f holds only the reference; 20,7 is lost.

## 41. 009F8160's land arm: `kSquadronLandTargetGoneBound` (cc9-physics12, 2026-10-16; lead item 3, seg-68 note)

**Liveness of the seg-68 notes (CC13_SEG68 7.3-7.5), checked on this tree before binding.**
- *Fighter out of ordnance sets +3Dh (009F81F6-009F81FC).* LIVE on ESMP08: with `BSP_SPENT_RTB_CENSUS=1`
  (`local\p12e8_f_cen.log`) every Zero squadron (class 150, `equipment=0`, no rack round) is logged `spent, not latched
  ... command 00E08F20 ... fighter=1` from about 113 s. In the image the +3Dh then ends that divebomb through 009F7C90.
  Not bound yet: the host launches these Zeros with divebomb because 007EEC50 accepts divebomb through
  `BSP_Unit_AttackCommandAppliesWithLoadout`'s loadout out-parameter, and whether the image's launch then arms the
  squadron with that loadout (so the image's Zeros carry bombs) must be read first; binding the +3Dh alone would end
  every such strike at launch.
- *The 00E08FA0 arm (009F8295-009F82D5).* LIVE on ESMP08: Zuikaku sinks at 1412.80 s (`local\p12e8_ra.log`) while
  Zuikaku_sqn35, sent home by the spent-ordnance B5 at 1166.51 s ("land at home Zuikaku"), is in its standby circle;
  it circles the wreck until all three die at 1439.78 s. Bound below.
- *Dogfight end for targets off the map (009F8319-009F83A8), the guard arm (009F83AD-009F8407), +3Eh forcing +3Dh.*
  Reachable only with a retreating target off the map, a director guard distance, and an unread +3Eh writer; not
  observed in the logs read here; not bound.
- *The free-attack tick 009F8470.* Its re-target (009F84A5-) runs for party slot 8 or an AI-held party
  (00927F10), or with director +3Ch/+3Dh and unit+370h == 2: every AI squadron reaches it. Not hosted, large
  (AttackLists ranking, override command, stage-2 raise); left for its own packet.
- *near_edge (009FA510).* Live only for aircraft inside the map-edge margin; not checked.

**The land arm.** 009F8160 runs from 009F8470 every tick with the squadron's command block. With the current command
`land` 00E08FA0 it sets +3Ch unless the resolved target exists with +5Eh and +5Dh clear and either 00927C90(0) is false
or the target's side matches; 009F7C90 then ends the command once 007EE6B0 finds every member's task willing (the land
task's +1Ch is the base 0099C230, true unless the in-mission interface watches the plane), and 0084E010's B5 issues
`returntobase`, whose 007F16D0 resolution skips a sunk home and takes the nearest live site (game_hosts_units.cpp
`record_return_to_base_007f16d0`).

**The binding (game_hosts_units.cpp, committed OFF).** In `run_spent_ordnance_rtb_0084e010`'s squadron loop, before
the B5 gate: when the leader has a land task installed (target: the task's site owner), a target that is gone (no unit, `state->simulate`, scene
`destroyed_005e`, or dead in the gunnery host) latches +3Ch once per lost site, starting a fresh latch cycle; the
existing break-off, B5 and 007F16D0 code then send the squadron to the nearest live site. Labelled: the installed land task standing for the current command `land` (the host keeps the class the order came from;
a first version keyed on the class missed every returntobase-sent squadron, so no ON run of it counts), the +5Dh/+5Eh stand-ins, 00927C90 false, and the fresh cycle
(the image's +3Ch reset is unread). Each latch logs `land target gone`.

**Predictions (before any ON run).** OFF: identical. ON: ESMP08 latches for every squadron whose landing carrier sinks
while it is inbound (Zuikaku about 1400 s, Zuiho later); their B5 re-resolves to a live carrier (`returntobase 007F16D0
... -> land at site Chitose|Chiyoda|Zuiho`), so those planes land there instead of dying in the wreck's circle;
ESMP08 gameplay moves from the first sinking of a carrier with inbound planes. USN04 45000: no carrier sinks with
planes inbound, gameplay identical.

### 41.1 The pair and the verdict: ON

ON: an export of 943359208 with the switch flipped (`local\p12_lg`, exe of 2026-10-16 17:53); OFF: that commit's
build (`local\p12_offb2`). Reference env, no jitter; logs `local\p12lg2_<on|off>_e8.log` (ESMP08 `p11_e8_rb.txt`,
60000 frames) and `local\p12lg2_on_u4.log` against `local\p12lg_off_u4.log` (USN04 45000, b1c9e2fdd's OFF build, the
same gameplay code); diff `local\p12lg2_diff_e8.txt`.

| Row | pair_diff | Note |
| --- | --- | --- |
| USN04 45000 | 1, gameplay identical | no latch |
| ESMP08 60000 (rb) | 3, moved | one latch: `land target gone: squadron "Zuikaku_sqn36" ... site Zuikaku latched +3Ch at 1396.94 s` |

Mechanism, as predicted: Zuikaku dies at 1396.94 s with Zuikaku_sqn36 (sent home by the B5 at 1143.24 s) in its
landing pass. ON latches at that instant; the break-off and the B5 run in the same tick, and 007F16D0 resolves `land at
site Zuiho` (3 of 4 sites pass, the sunk home excluded); the land task is installed toward Zuiho and the squadron joins
Zuiho's queue. OFF flies on into Zuikaku's final and dies over the wreck at 1434.37 s; ON lives to 1627.96 s. The
rest of the run moves from there (5 more deaths, 6 more units, dive releases 127 -> 128). **Flipped ON.**

## 42. The fly-to solver's world-edge pull: `kFlyToWorldEdgeBound` (cc9-physics12, 2026-10-16; lead item 3, seg-68 note 7.5)

009FD570 (the fly-to solver, src/plane_fly_to_solver.cpp) adds a pull toward the map centre when its lead point is
within 500 m (00CE397C) of the world rectangle (009FDD58-009FDE8B, listing read): 00681F40 (00681F40-00681FB7, read
whole) answers "near", 009FA510 (BSP_GGame_WorldEdgeOverrun) the largest overrun, divided by 60.0 (00CE3D68), and the
centre is 0.5 x (GGame+7128h + 711Ch, 7130h + 7124h), each sum stored as float before the double 0.5 (009FDD8F-
009FDDC5). The host's four solver sites (the torpedo goaway, the drop-kamikaze, the dive goaway and the moveto task)
forced `near_edge = false` with "the world bounds are unmodelled"; they are modelled now: the Lua host's
`world_border_bounds()` (NW.x / NW.z / SE.x / SE.z = +711Ch / +7124h / +7128h / +7130h, the mapping
game_hosts_ship_ai.cpp and plane_retreat_task.cpp already use).

**The binding (game_hosts_units.cpp, committed OFF).** `Impl::fly_to_world_edge_009fdd62(fin)` replaces the four
forced stores: with bounds, the lead point (`unit_position + lead_vector x kLeadSeconds`, as the solver forms it) is
tested as 00681F40 does; when near, `penetration` is cc13's `ggame_world_edge_overrun_009fa510` (src/cc13_seg68.cpp)
with margin 500 and the centre as above. A summary line counts calls and near answers.

**Predictions (before any ON run).** OFF: identical (the store is the same false). ON: `summary mission fly-to world edge`
shows calls > 0 on every row with solver users; near > 0 only where a plane steering through the solver comes within
500 m of the map edge. Rows where that never happens are gameplay identical (USN04 45000 expected identical: its
strikes fly in open ocean). Where it happens, those planes' headings bend toward the map centre near the edge.

### 42.1 The pair: gameplay identical, the near arm not reached; ON

Export of 5f839f749 with the switch flipped (`local\p12_we`, exe 5B737877E477) against that commit's build
(`local\p12_offwe`). Reference env; logs `local\p12we_<on|off>_<e8|u4>.log`.

| Row | Result |
| --- | --- |
| ESMP08 60000 (`esmp08_p12_e8_rb`) | `pair_diff` 1, gameplay identical; ON `calls=4076 near=0` |
| USN04 45000 | ON `calls=0 near=0` (no solver user ever runs), so identical by construction; no OFF run |

The gate is evaluated on every ESMP08 solver call and never answers "near": no solver user flies within 500 m of
that map's edge. The binding computes what the image computes there, so it is **flipped ON**; the pull itself is
unexercised in these rows (a row with solver users near an edge would show `near > 0` and moved headings).

## 43. 009F8160's fighter arm: `kFighterSpentEndBound` (cc9-physics12, 2026-10-16; lead item 3, seg-68 note 7.3)

**The question section 41 left open, answered.** This installation's Zero (vehicleclasses.lua class 150, autoload,
mtime 2026-05-09: `DefaultEquipment = 0`; `Equipments[1]` gives rack slots 50/51 one round each, platform 89) is
launched by the ESMP08 plans with `equipment=0`, so its racks hold no round. 007EEC50 still answers `divebomb` for a ship
target: 007EE8F0's divebomb arm asks 007ED7E0, which reads each rack slot's descriptor for the loadout
(`slot->vtable[220h](loadout)`, ATTACK_COMMANDS "The six ordnance queries"), a type question, not a round count. So the
image gives these Zeros divebomb exactly as the host does, and 009F8160 then finds 007ED790 (any member can drop) false:
for a non-fighter it latches +3Ch (the host's spent-ordnance RTB), for a fighter (`[sq+35Ch]->vtable[18h](13h)`) it
sets +3Dh (009F81F6-009F81FC, then 009F8412), and 009F7C90's +3Dh arm (009F7D01-009F7D45) ends the command with
0071E430(current, 1) once 007EE720 agrees (the dive task's vtable[20h] is 007B40F0, `MOV AL,1`). The host did nothing
for fighters, so these Zeros kept diving on ships with guns only.

**The binding (game_hosts_units.cpp, committed OFF).** In `run_spent_ordnance_rtb_0084e010`'s squadron loop, after the
ordnance arm: a fighter-kind leader with one of the seven ordnance classes, the reload gate open and no live member
holding a rack round has its command ended through `end_command_0071e430(leader, command, true)`, each live member's
rows retired and resolved target cleared, and `ended_stage2_command` set so the existing 0084E4B1/0084E595 moveto-self
step treats it as the image's stage 2. Labelled as the dead-target retest models the same arm: 007EE720 taken true, the
kind-1Ch moveto re-issue not modelled; one end per command class. Logs `fighter spent: ... +3Dh`.

**Predictions (before any ON run).** OFF: identical. ON: ESMP08 ends every bombless Zero squadron's divebomb within a
tick of its first think (the census saw them from about 113 s), then each gets the moveto-self hold at its own position;
those Zeros stop diving on US ships, so US ship damage and the deaths of the Zeros to ship AA fall, and Zero kills of
attacking US planes may rise (they now hold near the fleet). ESMP08 gameplay moves from about 113 s. USN04 (no IJN
fighter with an ordnance class in the logs read) is expected identical.

### 43.1 The pair, and the verdict: ON (a first reading of it as a mechanism failure was wrong)

Export of a1e22d8b4 with the switch flipped (`local\p12_f3`, exe C266B7A303BC) against that commit's build
(`local\p12_offf3`); reference env; logs `local\p12f3_<on|off>_<e8|u4>.log`, diff `local\p12f3_diff_e8.txt`.

| Row | pair_diff | Note |
| --- | --- | --- |
| USN04 45000 | 1, gameplay identical | no fighter end |
| ESMP08 60000 (`esmp08_p12_e8_rb`) | 3, moved hard | 45 ends from 113.30 s; dive-bomb-task releases 128 -> 5, US ships sunk by 3000 s 14 -> 5 |

The OFF run has these Zeros dropping bombs (`bomb from Chiyoda_sqn13|.-2 impact ...`), which first read as the rack-round
reader being wrong. It is the other way round. The image gives an equipment index n the loadout `009552E0(class, n)`:
entry n-1 for 0 < n <= the class's Equipments count, **null for n = 0** (SQUADRON_LAND_TASK 5df, "`[unit+C54h]`").
Carrier launches copy class `+134h` DefaultEquipment (0 for the Zero) into the slot (006C0F00 -> 006C7490), so an
air-ops Zero has no loadout and no bomb ("Airfield fighters carry no bombs", SHIP_AI_OPEN_ITEMS for the P-38/P-40).
The host's `plane_holds_rack_round_007b9140` with the carried index answers that correctly; the host's bombs come from
the release path's own default (`876b192cf`: "The release path keeps its own default", labelled "a carrier-launched Zero
would also count as loaded"), a host substitution the image does not make. So in the image these squadrons get
divebomb (007ED7E0 reads the rack's descriptor type), 009F8160 sets +3Dh, 009F7C90 ends it, and the squadron takes the
stage-2 moveto-self hold: exactly the ON run. **Flipped ON by mechanism.** It costs ESMP08 most of its IJN dive
strength (the Zeros' phantom bombs); the row is re-planned against it. The release path's default still arms an
equipment-0 Zero on any path this switch does not end (a host gap for the plane lane: the release seed should follow
009552E0's null loadout).

Why the image still answers divebomb for an empty rack: the Zero's `Platforms` table (vehicleclasses.lua, the class-150
row) mounts platform 89 at slots 50 and 51 whatever the loadout, and `Equipments[1]` only supplies their Ammo; so the rack
devices exist with no round, and 007ED7E0's descriptor query (a type, not a count) sees a general bomb. Uncertainty: the
descriptor a mounted rack with no loadout entry returns from `vtable[220h](0)` was not read; if it were null the image
would choose a gun class (strafe) instead and neither OFF nor ON would match.

## 44. Handoff (cc9-physics12, 2026-10-16, at about 62% context)

**Commits on agent/cc9-physics1** (on top of the origin/main merge at 25fbdcba0; full `scripts/build.ps1` with tests
passes on each): 8652858c8 (JM06 re-plan, 38), 6cfd0ff18 / 135f8ae26 (006C77E0 landing-record compaction, ON, 39),
2f1480151 (ESMP08 `esmp08_p12_e8_rb`, 40), b1c9e2fdd / 943359208 / b46d56a35 (009F8160 land arm, ON, 41),
5f839f749 / 8aa30c198 (fly-to world-edge pull, ON, 42), a1e22d8b4 / 98e97e3b6 / 45ffa86bf (009F8160 fighter +3Dh,
ON, 43; 98e97e3b6's "mechanism failure" text is superseded by 45ffa86bf's 43.1).

**Rows.** JM06 `jm06_p12_j6_a4` holds reference, 20,3, 20,5, 20,7 and 20,11 (38). ESMP08 `esmp08_p12_e8_rb` held
reference / 20,3 / 20,5 up to b46d56a35 (3334.33 / 3594.58 / 3251.20 s) and is **lost on 45ffa86bf**: the reference
sinks 5 of 15 (`local\p12e8d_rb_ref.log`), and f sinks 4 (`local\p12e8d_f_ref.log`). Both were sent to cc9-lua62 up
to b46d56a35; the loss is not yet reported to lua62.

**Open, in order.**
1. **ESMP08 re-plan after 43.1.** The Zeros launched by the plans carry no bomb in the image (equipment 0 is a null
   loadout), so their divebomb ends at launch and the plans lose most of their ship kills. Options: (a) a harness
   loadout option for `launch` (the image's slot+10h writers 006C1170 / 006C80C0 have no references; the Lua path is
   `PlaneChangeAmmoType` 0089F1F0 -> 007ED690 -> unit+C54h), routed to the harness owner; (b) a plan from the strike
   classes 159 / 162 / 163 only, slot 4's Zeros held back as cover. Check first whether the release path still arms an
   equipment-0 Zero anywhere (the plane lane's `876b192cf` default; 43.1).
2. **The deck crash (39.2).** Recalled wingmen die on the deck when another squadron is in its takeoff run: plane-arm
   vehicle avoidance against the departing aircraft. Read whether the image's 007DF4F0 neighbour list includes a plane
   in a deck ground state, and whether air ops hold a launch while a plane is on final.
3. **009F8470's re-target** (41): every AI-held squadron's per-tick AttackLists re-selection is unhosted. Large.
4. **Item 4 (catapult)** is not started beyond section 37. cc13's seg-33 reconstructions are on main in
   `src/cc13_seg33a.cpp` (`mcatapult_queue_launch_006eb680` and the rest, behind a `Seg33aHost` interface): hosting
   means an adapter, not a call.

**Tools in this tree's `local\`.** `p12_launch.ps1` (one detached run with the reference env, optional jitter and extra
env), `p11_wait.ps1`, `p12_j6attack.py`, `p12_j6gen.py`; exes kept for pairs: `p12_m*` (tree builds),
`p12_off*` / `p12_qc` / `p12_lg` / `p12_we` / `p12_f3` (pair exports). A tree exe used by a running game blocks
`scripts/build.ps1` at the link: run from a copy.

## 45. JM06 re-planned again: `jm06_p12_j6_c1`; BSM01 and USNOS checks (cc9-physics12, 2026-10-16)

On main with gunnery48's 6c900f1e0 (`kTorpedoAngleErrPerEvaluationBound` ON; this tree merged at fedbf3c92, exe
`local\p12_m5`), `jm06_p12_j6_a4` completes 20,3 at 2000.83 s but not the reference: PlayerSub 01 is sunk at 1750.83 s
by Lexington-class 01_sqn02|.-2 (damaged from 723.85 s, on its way to the 13000-frame moveto (800, -1500)); the
Lexington dies only at 3243.79 s and phase 3 never ends. Extra Lexington torpedo or attack lines (b1, b2) cannot help
once the sub is gone. **c1** drops that one moveto (the sub stays near (1300, -1500)); c2 sends it to (2200, -2500)
instead.

| Plan | reference | 20,3 | 20,5 | 20,7 |
| --- | --- | --- | --- | --- |
| a4 | no end | completed 2000.83 s | - | - |
| c2 | completed 1970.80 s | completed 2006.83 s | no end | completed 2000.83 s |
| **c1** (`config/win_orders/jm06_p12_j6_c1.txt`) | **completed 1967.79 s** | **completed 2259.08 s** | **completed 1901.73 s** | **completed 2102.93 s** |

Logs `local\p12j6<plan>_<ref|j3|j5|j7>.log` with manifests; the guard's sus_prog.exe refusal is logged in all four c1
runs.

**BSM01.** The keeper lua62 re-timed at 833.92 s is `bsm01_p6_b1_a`, not `l50_b1_a`. On this main p6 reaches EndMission at
779.93 s (reference) and 1515.40 s (20,3); `l50_b1_a` reaches it at 1518.36 s here and not within 2000 s on 25fbdcba0's
exe, so there is no regression to bisect (`local\p12b1*.log`).

**USNOS, the world-edge pull (42).** With ships51's `s51_os_a7` orders, wave 2's plane #4.4|.-2 still leaves the map
(z = -20 km at 600 s, -97 km at 1990 s; the map's SE z is -17500). The level-bomb goaway does steer through 009FD570,
but `fly-to world edge calls=6534 near=0`: the off-map leg is not flown by the solver (`local\p12os_a7.log`, `.csv`).

**ESMP08 after 43.1 (still open).** Without the slot-4 lines (`local\p12_e8_rb_no4.txt`) the reference still sinks only
5 of 15: class 150 is also allocated to slots 2 and 3 against cruisers and destroyers (the `l61_e8_alloc` ranks), so 23
Zero squadrons still launch with divebomb and hold. A re-plan must move those slots' targets to classes 159/162/163.

## 46. An equipment-0 Zero carries its racks' constructor rounds: `kRackConstructorRoundBound` (cc9-physics13, 2026-10-17; lead items 1-2)

**43.1's premise corrected.** 43.1 read "equipment 0 = null loadout = no bomb". The loadout is null, but the racks
are still loaded. The Zero's racks are default devices: the Platforms table mounts platform 89 at slots 50 and 51,
and `0095A880`'s flag pass builds them (SQUADRON_LAND_TASK 5ey). Each rack's owner bind is `006E3350` (vtable 9Ch,
`BSP_MBombPlatform_BindOwnerLoadout`; `disasm-raw` 006E3350-006E3402):
- `006E3378`-`006E3386`: it reads the owner's `+C54h` and takes `JLE 006E33E3` when it is <= 0. That skips the
  `009552E0` loadout walk and its `vtable[1BCh]` SetAmmo.
- `006E33E3`-`006E33FE`: unless `[+C0h]+4 == 3` (unread), it tail-jumps `vtable[224h]` = `006E3E70` AttachNextRound.
- The constructor `006E3C00` stored ammo `+484h` = orgAmmo `+488h` = 1 (`006E3C20 MOV EAX,1`, `006E3C37`, `006E3C3D`).
- So the rack attaches one round, and `007B9140`'s `vtable[210h](2Ah)` (a round attached) answers true.

5ey reached the same result for the release path, which is why `kRackUnequippedEmptyBound` stays refuted. The host's
`plane_holds_rack_round_007b9140` disagrees: with `kPlaneSceneEquipmentBound` and bag 0, `authored_ammo` is -1 and the
rack holds nothing until the release seed fills it. So `009F8160` (`007ED790`) finds a fresh carrier-launched Zero
squadron spent at its first think, and 43's +3Dh end fires at launch. That is the "45 ends from 113.30 s" of 43.1.
**43's switch stays right**: once these Zeros have dropped, the +3Dh end is the image's. Only the 007B9140 answer
before the first drop is a host gap. Item 2's "follow 009552E0's null loadout" (no rounds) would make the gap wider,
so it is not bound.

**The binding (game_hosts_units.cpp, committed OFF).** In `plane_holds_rack_round_007b9140`, an unseeded single rack
of a plane whose carried bag is 0 answers one round. LABELLED:
- only bag 0. A positive bag whose loadout has no entry for a rack also keeps the constructor round in the image, but
  the host's release seed gives that rack none. That inconsistency stays open.
- the `[+C0h]+4 == 3` skip is taken as not met.

**The harness loadout option (97b1f18b5).** `<frame> launch <base> <slot> <class> <count> <target...> loadout <n>`
is the screen's own step. After `006C0F00`, `00673A10` calls `006BA4B0(slot, screen+334h)` at `0067485D`, which stores
the loadout ring's choice at slot+10h. screen+334h defaults to the class's +134h (`0065C9A0`), which is what the host
already stored, so a line without the option is unchanged. For the Zero, `loadout 1` gives the same two rounds as the
constructor default.

**Predictions (before any ON run).**
- OFF: identical.
- ON, ESMP08 with `esmp08_p12_e8_rb`: no `fighter spent` end before a Zero squadron has released. The Zeros' dives and
  releases come back (dive-bomb-task releases rise from 43.1's 5 toward the OFF 128), followed by +3Dh ends after the
  drops. US ship damage rises from 113 s. pair_diff 3.
- ON, USN04 45000: identical or gameplay-identical (no bag-0 bomb rack is asked at 009F8160).
- Mechanism failure: a `fighter spent` end for a squadron whose members still hold an unseeded rack round, or a
  positive-bag plane whose answer moves.

### 46.1 The pair, and the verdict: ON

Export of 0e9c7ad07 with the switch flipped (`local\p13_rc`) against that commit's build (`local\p13_off_rc`); reference
env; logs `local\p13rc_<on|off>_<e8|u4>.log` with manifests, diffs `local\p13rc_diff_<e8|u4>.txt`.

| Row | pair_diff | Note |
| --- | --- | --- |
| USN04 45000 | 1, gameplay identical | as predicted |
| ESMP08 60000 (`esmp08_p12_e8_rb`) | 3, moved | `fighter spent` ends 46 -> 21, the first at 113.30 s -> 465.86 s; dive-bomb-task releases 1 of 162 -> 100 of 224; US ships sunk by 3000 s 4 -> 10 |

Mechanism: the first ON end (Zuiho_sqn15 at 465.86 s) follows its members' four drops (`bomb from Zuiho_sqn15|.-II/.-III`,
two rounds each); no end precedes a squadron's drops. **Flipped ON by mechanism.** ESMP08 regains the Zeros' bombs
that 43.1 removed, so `esmp08_p12_e8_rb` is re-timed on the ON build rather than re-planned around fighter-only Zeros.

## 47. The neighbour object's three clock draws: `kNeighbourClockDrawsBound` (cc9-physics13, 2026-10-17; lead item 4, cc13 seg48)

The unit+C50h constructor `007E1E20` ends (`007E1F94`-`007E1FF2`, `disasm-raw`) with three `00BD2F10` calls,
ECX = 1 (stream 1), bounds (0, P). Each result has the period added by `FADD` and is stored by `FSTP`:
- `+7Ch` = U(0, 3) + `+88h` 3.0;
- `+84h` = U(0, 2) + `+8Ch` 2.0;
- `+80h` = U(0, 1) + `+90h` 1.0.

`007E2010` adds dt to all three and refreshes when `+7Ch >= +88h`. So each plane's first refresh comes at once, and its
3 s refresh, 2 s finder and 1 s friendly re-check phases are random. The host starts them at the midpoints (4.5 / 3.0 /
1.5) for every plane, so all planes share a phase, and it makes three fewer stream-1 draws per construction.

**The binding (game_hosts_units.cpp, committed OFF).** `nb_seed_clocks` runs at a plane's first neighbour use
(avoidance, the friendly re-check, the finder). It draws the three values from the plane's keyed stream
(`name#nb7c`, `#nb84`, `#nb80`; the shared stream only when BSP_GUNNERY_RNG_STREAMS is off) and gives every host copy of
`+7Ch` the one value. LABELLED: seeded at first use, not at `007D621F`'s construction; the sum is taken in float.
`summary mission neighbour clock draws seeded=N` counts the seeds.

**Predictions.**
- OFF: identical.
- ON: every plane that reaches 0099B670 / 007B96D0 / 007E2090 seeds once; the refresh/finder/re-check instants move
  by up to 1.5 s, the finder's targets and the avoidance bands move with them, and plane-heavy rows (ESMP08, USN04 with
  its strikes) move from the first fighter contact; pair_diff 3. No other keyed stream moves, because the draws are on
  their own keys.
- Mechanism failure: a seed count that differs from the number of distinct planes using the object, or a moved draw
  on any other key.

### 47.1 The pair, and the verdict: ON

Export of 58304aea0 flipped (`local\p13_nb`) against that commit's build (`local\p13_off_nb`); reference env; logs
`local\p13nb_<on|off>_<e8|u4>.log` with manifests, diffs `local\p13nb_diff_<e8|u4>.txt`.

| Row | pair_diff | Note |
| --- | --- | --- |
| ESMP08 60000 (`esmp08_p12_e8_rb`) | 3 | seeded=700; first moved deaths at 442-466 s (the first fighter contacts over the fleet); deaths 632 -> 633, dive-bomb-task releases 100 -> 96, torpedo drops 38 -> 29 |
| USN04 45000 | 3 | seeded=63; first hit 96.65 -> 98.85 s; deaths 51 both |

Both rows move only from the first air contact, as predicted. The draws are on their own keys, so no other stream
moves. **Flipped ON by mechanism.**

## 48. Read-only results: the deck crash, 007DAFD0, and the scope of 009F8470 (cc9-physics13, 2026-10-17; lead items 3, 4b, 5)

### 48.1 The deck crash (39.2): the image steers the same way

- **The list.** `007E11D0`'s add loop walks the world list `[[00E188A8]+19CCh]+58h`. It keeps any entity answering
  IsKindOf(6) or (0Fh), other than the owner, that is not already listed and is within R. It applies no ground-state,
  `+900h` or `+904h` test. So a plane in its takeoff run on a deck is listed, as in the host.
- **The landing plane runs the arm.** 009A17D0 needs `+72Ch` vtable[38h] (mode 7) or mode 6, and a plane on final is
  airborne (mode 7). The arm also needs pilot+2E4h bit 2. The land task's per-tick `009B3EB0` sets its scratch
  `+4ACh` = FFh (`009B3EC2`) and copies it to `+2E4h`. A displacement scan of 009A0000-009D8000 (`local\p13_disp_scan.py`)
  finds no other write to `+4ACh`, so the mask stays FFh on final.
- **No hold.** `006BED60` (the launch queue's readiness) tests only the runway and hangar failure bytes and the owner.
  `006C7960`'s final mode 4 needs `006BED60`, the corridor and `rec+8 > 0`. Neither looks at a plane on the deck or
  on final.
- **cc13's four differences** (backlog 2026-10-14):
  - pilot+26Ch |= 2: its only reader is the describe string `009A1A90` (`AV_VEH`), so it has no gameplay effect;
  - unit+9B4h: the ship branch's altitude filter only;
  - the PitchSpd fallback: the ESMP08 carrier classes author PitchSpd (vehicleclasses.lua: 150, 162 and 163 0.436332,
    159 0.610865), so the fallback is not taken;
  - the class+50h model box: it feeds R, the sum of the two +500h extents. This is the one difference on the crash
    path, and binding it needs the mesh bounds the host does not load (PLANE_SUBSTITUTION_SWEEP).
- So nothing host-only gates the deck crash at this level. Nothing was bound.

### 48.2 007DAFD0's pose refresh (cc13 seg48 note)

The image calls `00414DB0(unit)` first when byte unit+C8h is 0, which rebuilds the world pose from a dirty body. The
host passes the current motion pose, which is never stale at that call. No behaviour difference; not bound.

### 48.3 The free-attack tick 009F8470: scope (not started)

cc13 reconstructed it in `src/cc13_seg68.cpp` (`squadron_auto_target_tick_009f8470`, the selector `009F7810`, the
three filters, `009F7BA0` scoring, `009F9770` AttackLists rank). It sits behind `SquadronAutoTargetHost`, a raw-memory
interface (`u8` / `u32` / `f32` at offsets), so hosting it means an adapter or a value-level port. The tick needs these
from the host:
- **The gate.** Director `+30h` == 1 (the queue head runs, `CommandMode::kQueueHead`), and either the leader's party
  slot is 8 or AI-held (`00927F10`) with leader `+184h` clear, or director `+3Ch` and `+3Dh` are set with squadron
  `+370h` == 2. In mode 2 with `+3Ch` clear it raises the override stage to 2 (`0071D9E0`).
- **The candidates.** The party's recon slot list `[008053C0(+54h)]+DE8h` (the host's
  `GameGunneryHost::recon_triple_units(side, 1)`), filtered by `009F7480`.
- **The score.** `0.5 / max(1, d) + rank`, where rank is the AttackLists entry of the class 007EEC50 would choose
  (`009F9770`; the host has `choose_attack_class_007eec50` and the `ai_target_weights` FreeAttack fields), gated by
  sensor detection `00804350`. A new best must beat the current target's score + 0.05.
- **The issue.** When the best differs from the current command's resolved target: `009F6D60` picks the command
  class, nothing happens if both the current and the new class are ordnance classes, `stop` is replaced by a moveto to
  the squadron's position, and `0071D980` sends an **override** command.
- **The missing host layer.** The override pieces are reconstructed but not called:
  - `set_override_command_0071e7f0` and `clear_override_command_0071e610` are in `src/command_execution.cpp`;
  - the director host `game_hosts_commands.cpp` notes that its override arm 00836D67 "cannot fire here because
    nothing writes director+188h";
  - the send `0071D980` needs `[director+40h] < 0` (the auto-target hold, DIRECTOR_TARGET_GATE.md), then routes
    MT_GAMEUNIT_SETCMD with flag 0. Its receiver `00721A40` takes the override arm at `00721B7C`.
  Wiring those for the squadron's fused director is the largest part, and every AI squadron on every row reaches it (41).
- Recommendation: a fresh worker packet with its own switch: the queue/override layer first (OFF, identical when
  unused), then the tick.

### 47.2 ESMP08 `esmp08_p12_e8_rb` re-timed on 57ecc76a4 (both switches ON)

Exe `local\p13_m3` (tree build of 57ecc76a4); reference env, 140000 frames, `--stop-after-end 40`; logs
`local\p13e8c_rb_<ref|j3|j5|j7>.log` with manifests; every completion logs the exec guard's sus_prog.exe refusal.

| Plan | reference | 20,3 | 20,5 | 20,7 |
| --- | --- | --- | --- | --- |
| rb on af23c565f (`local\p13_m2`, before 47) | completed 6132.09 s | no end (Iowa, Cummings) | - | - |
| **rb on 57ecc76a4** | **completed 3984.96 s** | **completed 4707.67 s** | **completed 3966.89 s** | **completed 3261.21 s** |
| dd (`local\p13_e8_dd.txt`, destroyers first in each Zero group) on 57ecc76a4 | - | no end | - | - |

So the row needs no re-plan: with the Zeros' rounds back (46), `esmp08_p12_e8_rb` holds the reference, 20,3, 20,5 and 20,7.

## 49. Handoff (cc9-physics13, 2026-10-17, at about 55% context)

**Commits on agent/cc9-physics1** (on top of the origin/main merge at f4d7658e6; the full `scripts/build.ps1` with tests
passes on each):
- 97b1f18b5: the harness `launch ... loadout <n>` (46);
- 0e9c7ad07 / af23c565f: `kRackConstructorRoundBound`, ON (46, 46.1);
- 58304aea0 / 57ecc76a4: `kNeighbourClockDrawsBound`, ON (47, 47.1);
- 2e9ce0a2e / 8a3e67fd8: the ESMP08 re-time (47.2) and the read-only results (48).

**Rows.** `esmp08_p12_e8_rb` on 57ecc76a4 holds the reference (3984.96 s), 20,3 (4707.67 s), 20,5 (3966.89 s) and 20,7
(3261.21 s). Sent to cc9-lua64.

**Open, in order.**
1. **The free-attack tick 009F8470** (48.3): wire the override send `0071D980` -> `0071E7F0` for the squadron's fused
   director first, behind an OFF switch that is identical while nothing sends. Then host cc13's
   `squadron_auto_target_tick_009f8470`: filters, selector, `009F9770` rank, and the recon list via
   `recon_triple_units`. Large, x87-light, RNG-free.
2. **The catapult chain** (37; cc13 `src/cc13_seg33a.cpp` behind `Seg33aHost`): an adapter.
3. **007DC830, the free-flight step** (cc13 seg48 note): every airborne plane runs it (007CC2F0 -> 007DC830 ->
   007DB680). The host only counts it. This is the core flight law, x87-heavy; scope it as several packets.
4. **The deck crash** stays open on the class+50h model box (48.1). It needs mesh bounds.
5. **A positive bag with no loadout entry for a rack** (46): the image keeps the constructor's round, but the host's
   release seed gives that rack none. Small; check the BSPGun platform list against the class's default devices first.

**Tools in this tree's `local\`.** `p13_disp_scan.py` (each displacement access in a VA range, with a write/read guess),
`p13_vc_equip.py` (a class's DefaultEquipment and Equipments from this installation's vehicleclasses.lua),
`p13_e8_loadout.py`, `p13_e8_dd_first.py`; exes `p13_m3` (57ecc76a4), `p13_off_rc` / `p13_rc` / `p13_off_nb` / `p13_nb`
(the pairs).

## 50. USNOS plane #4.4|.-IV: a retired plane is still a neighbour: `kStockReturnDestroyBytesBound` (cc9-physics14, 2026-10-17; lead item 1)

### 50.1 The chain, measured (diagnostic builds of cd47de824, not committed)

USNOS 14000-15200 mission frames with `cc9-ships44\local\s55_os_r0.txt`; logs `local\p14_diag{,2,4,5,6}_usnos.log`.

1. **The runway exit starts in the flare.** Each #4.x level bomber touches down with its nose 0.27-0.41 rad left of its
   track. The track stays on the runway axis (-1.11 against holder+88h -1.0995). At touchdown the wheel friction turns
   the velocity onto the nose, and the plane rolls off at about 20 degrees. Park is fast (`v` > 1.5 x MoveSpd) and steers
   along the axis (009B273A JE 009B28BF: `sx` = 0), so the yaw saturates and still loses. #4.4|.-IV crosses
   x = -17.5 (half of the 35 m holder) at 11 m/s and the land task aborts. #1.4|.-II lands with an error of 0.005 and
   taxis into its hangar. On cd47de824 both #4.4|.-III and -IV leave the runway.
2. **The pilot is overridden.** In the flare the planner asks pitch -1 and yaw +1. Each think, 0099BF30 (`gunfire_repair`)
   replaces the command block with pitch +1.1 and yaw -1.1 (by the bank's sign), because 0099B940 finds a band
   covering [-1, 1] in both sets. Pitch runs to 0.46 (command 0.1047) and the heading by -0.3 rad in 3 s.
3. **The bands.**
   - Terrain avoidance 0099F1C0 puts pitch bands [-1.1, 0.3-0.56] on every lander, #1.4|.-II included. Its search never
     fails.
   - The #4.x planes also get vehicle avoidance 007DF4F0 bands in both sets. They widen to [-2.0, 2.0] by the threshold.
   - The neighbour is #1.4|.-II on every #4.4 line: closing, d = 154 -> 30 m, at (-8005.9, 5.2, -8923.8), the
     touchdown zone. Its `+900h` is 1, `+904h` set and `+C00h` clear.
4. **#1.4|.-II was retired at 396.18 s.** It taxied into the hangar, and the stock return
   (007CE6CA C7h -> 007CC8B0 -> 007F1CA0) set flight state 1 and called Kill(plane, 5). The host kept its `+5Fh` and
   `+60h` bytes clear. The vehicle arm's neighbour refresh (007E11D0 as the host carries it) keeps any slot whose
   state is active and whose `+5Eh`/`+60h` are clear. So the retired plane stayed listed, and every later lander
   avoided it.

### 50.2 The image: Kill sets the bytes at once

- `00926D90` Kill (disk listing): `00926DF3` +5Fh = 1. With +60h clear (`00926DEF`), vtable[70h](1) at `00926E05`.
- The plane's vtable[70h] is `0077D1A0` (the dword at `00D066A8` and at `00D19D98`). It tail-calls `00926C80` Destroy at
  `0077D243`, which stores +60h = 1 at `00926CD5` before anything is queued.
- The host's C3Ah death path already sets +60h (and the flush's +5Dh) at once (`kPlaneDeathFlagsBound`). The
  ship kill-depth path sets +5Fh and +60h. The stock return set neither.
- No avoidance or neighbour test in the image keys on `+C00h` or visibility. `007DF96C`-`007DF9A9` skips only
  `pilot+260h` and non-planes. So the image removes the retired plane through these bytes, not through the hide.

### 50.3 The binding

`kStockReturnDestroyBytesBound` (`src/game_hosts_units.cpp`), committed **OFF**. With it ON, the stock return sets
+5Fh = 1 and, when +60h is clear, +60h = 1 and the flush's +5Dh (`state->simulate`), after its Kill. The retired
plane then fails the neighbour refresh's in-world test and leaves the list at the next 3 s refresh. Summary line:
`summary stock return destroy bytes bound= destroys=`.

### 50.4 Predictions (ON against OFF, both from the commit that carries this section; written before the runs)

1. **USNOS** (s55_os_r0.txt, 150000 frames):
   - `destroys` equals the stock returns of the row.
   - No #4.x vehicle-avoidance band against a retired plane. The #4.x flares keep the planner's pitch and yaw, and the
     touchdown heading errors fall from 0.27-0.41 to under 0.05.
   - No `plane ground contact lost` and no ground re-takeoff for a #4.x bomber. #4.4|.-IV parks and is retired, so
     PhOneAttackers no longer keeps a live member below the sea.
   - `pair_diff` 3. Every death row that moves involves a plane that landed after the first stock return, or the
     phase it closes.
2. **JM05 9000** (landing control, airfield and carrier stock returns): `destroys` > 0, `pair_diff` 1 or 3. Any moved
   landing is one whose approach crossed a retired plane's spot.
3. **USN01 3000** (control): `destroys` 0 unless a plane is stowed before 3000 frames; `pair_diff` 0 or 1.
4. **Mechanism failure:** a retired plane still listed after the next refresh. Also a failure: any change on a row
   with no stock return, or a lander that still gets bands against a plane with +60h set.

### 50.5 Answers to the lead's three questions

- **(a) Why park lets it leave the runway.** Park's band clamp (009B276A-009B2851) applies only when slow. A fast plane
  steers straight down the axis, as the image does. The plane leaves because it touched down yawed 0.41 rad by the
  avoidance override (50.1).
- **(b) Abort's on-ground arm** (009B0E74-009B0F93) is transcribed behind `kLandAbortGroundArmBound` (SQUADRON_LAND_TASK
  5v). With the ground re-takeoff ON (5br) it runs one tick per grounded abort. It does not close this row.
- **(c) Why the plane under the sea does not die.**
  - A state-4 plane has no water or terrain test in the image. `007CBFA0` calls no water sampler. `007CB7F0` is reached
    only from `007CC2F0` (state 7) and from the control-mode slot 28h (`007CC2D0`).
  - A `.text` scan for slot-28h calls through `+72Ch` finds none. The same scan for slot 24h finds `007CBD6B`, so the
    scan works. Tool: `local\p14_slot_call_scan.py`.
  - The takeoff run's arm D (`h = 0 > y`) returns every step once the plane is below 0, which leaves it level near
    -105 m.
  - That is the image's law as read. The fix belongs upstream (50.3).

## 51. 0085E4D0's own tail: the up pick and 0085D3D0: `kRotateAboutAxisImageTailBound` (cc9-physics14, 2026-10-17; lead item 2)

### 51.1 The image (disk listing)

- **Up reference** (`0085E58F`-`0085E6C7`). `|c|` is `c` when `c > 0`, else `-0.0f - c`.
  - When `|x| > |y|` and `|x| > |z|`, `XMM6` (y) takes 1.0 (`0085E62F`): up = (0,1,0).
  - Else when `|y| > |x|` and `|y| > |z|`: up = (0,0,1) (`0085E6BD`).
  - Else up = (1,0,0) (`0085E6C7`).
  - The host picked (1,0,0) or (0,0,1) by `|z| > |x|` and never (0,1,0). In exact arithmetic the frame choice cancels in
    `L x RotZ x transpose(L)`, so only the rounding moves.
- **0085D3D0 at `0085E85F`**, run on the result after row 3 is restored. Its listing:
  - normalise row 0 (`00419440`, `inv = L > 0 ? 1/L : 0`);
  - `d = (a.y b.y + a.x b.x) + a.z b.z` (`0085D44E`-`0085D472`);
  - `b -= a d` (`0085D494`-`0085D4AD`), then normalise row 1;
  - row 2 = row0 x row1 (`004F9B30`), then normalise row 2.
  The host stopped before it, so the pose was never re-orthonormalised by this step.
- **Precision.** 0085E4D0's callers 007C18B0, 007D9F60 and 00901C20 run on the main thread (24-bit) or on a 53-bit
  worker (docs/CC13_FPU_PRECISION.md). The host takes the 24-bit form here, the same as cc13's standalone
  `src/cc13_front4.cpp`: the length's squares and sums, and every 0085D3D0 product and sum, rounded to float.

### 51.2 The binding

`kRotateAboutAxisImageTailBound` (`include/bsp/plane_advance_pose.hpp`), committed **OFF**. ON:
- `rotation_axis_angle_0085e4d0` takes the per-op float length and the image's up pick;
- `rotate_about_axis_0085e4d0` ends with a local `0085D3D0` (`src/plane_advance_pose.cpp`, bsp_core; cc13's copy in
  `gameplay_seg53_cc13.cpp` is bsp_game only).
Both callers move: the plane pose step (`game_hosts_units.cpp` `rotate_pose_0085e4d0`) and the GunBot intercept
(`game_hosts_gunnery.cpp` 3215, 00901E7A). Neither draws from 00BD2F10.

### 51.3 Predictions (ON against OFF, both from the commit carrying this section; written before the runs)

1. **Every row with planes moves** (`pair_diff` 3). Each plane step's pose changes at the float-rounding level, and
   the re-orthonormalisation removes the drift that accumulates without it. Positions diverge slowly. Shot and kill
   timings shift, and the AA kills are RNG-coupled (shared stream).
2. **No regime change.** The landings, releases and plane deaths per row stay within a few of OFF. A changed regime
   would be a mechanism failure: a plane losing control, NaN poses, or every plane of a squadron missing.
3. **Rows:** USN01 3000, JM05 9000, USN04 4500. BSM01 3000 (no planes before 3000 frames) is the control: exit 0 or 1.

### 50.6 Measured, and the verdict: ON

Both sides are builds of 201a1cbf1. OFF is `local\p14_off_sd` (the tree build). ON is `local\p14_sd`
(`--flip kStockReturnDestroyBytesBound=true`, SHA-256 3132DB9463CF). The rows ran in the reference environment, and a
300-frame USN01 smoke of OFF exited 0.

| row | `pair_diff` | stock returns OFF / ON | `destroys` ON | runway exits (#4.x) OFF / ON |
| --- | --- | --- | --- | --- |
| USNOS 150000, s55_os_r0.txt (`local\p14{off,on}_os.log`) | 3 | 3 / 10 | 10 | 4.4\|.-III, 4.4\|.-IV / 4.4\|.-IV, 4.5\|.-III, 4.5\|.-IV |
| JM05 9000 (`local\p14{off,on}_jm05.log`) | 1 | 0 / 0 | 0 | - |
| USN01 3000 (`local\p14{off,on}_u01.log`) | 1 | 0 / 0 | 0 | - |

**The predictions:**
1. **USNOS.**
   - `destroys` = the stock returns (10 = 10): **held**.
   - #4.4|.-III now parks and is retired at 747.44 s (OFF: it left the runway at 725.00 s).
   - #4.4 and #4.4|.-II are retired as before, and #4.5|.-IV is retired at 1432.32 s. Ten stock returns against three.
   - **Missed:** #4.4|.-IV still touches down 0.404 rad off the axis and leaves the runway at 735.49 s. #4.5|.-III and
     -IV leave it at 973 and 985 s. They relaunch (re-takeoff) and land again until they are lost.
   - In IV's flare the planes ahead are live: #4.4|.-II taxis until 742.19 s and #4.4|.-III until 747.44 s. So the
     remaining yaw comes from avoiding live taxiing planes, not retired ones. That is a different mechanism (50.7).
   - On this base #4.4|.-IV dies at 958.79 s (OFF) and 959.59 s (ON). It no longer flies at -105 m to the end on either
     side.
   - Neither side ends the mission with these orders. Deaths: 325 -> 322 rows, 68 changed.
2. **JM05 9000:** failed as a premise. This base has no stock return on the row (sends 0 on both sides), so it is a
   no-reach control. It returned 1, as predicted for a row with no stock return.
3. **USN01:** held (1).
4. **No mechanism failure.** Every stock return set the bytes, and none of the retired planes is avoided again
   (USNOS 4.4|.-III parks).

**Verdict: `kStockReturnDestroyBytesBound = true`.** The mechanism matches the image as read (50.2). The touchdown
miss is recorded and belongs to 50.7.

### 50.7 Open: a lander still avoids planes taxiing on the runway

- In the image, the vehicle arm walks every listed neighbour (`007DF96C`-`007DF9A9`), and the land task's mask is FFh.
  So a lander whose flare closes on a taxiing plane gets the same bands in the image, unless the landing sequencer
  keeps the runway clear first.
- The host lets a wingman start its final about 10 s behind the previous lander, which still taxis for 20-30 s.
- Next: read the sequencer's hold against the runway's occupants (`006C7960` mode 3 -> 4, `006BED60`, the occupancy
  vector `+34h`). 48.1 found no hold in `006BED60`, so read the corridor test and `rec+8` next.
