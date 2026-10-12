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
