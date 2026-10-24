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

### 50.8 The sequencer's runway hold, read (read-only)

`006C7960` gives mode 4 when the corridor test `006C3B10` holds, the deck is usable (`006BED60`) and `rec+8 > 0`.
`006C3F80` computes `rec+8`:
- From the other airborne landers' path gaps (`F = 1.2 FollowDistTime`).
- With none (k = 0), from the site clock `site+40h`. It is restamped at each touchdown (`007CB5F0` ->
  `006CE230`): `+8h = 00419010(0.75 FDT, 0.01, FDT, 1.0, site + tp)`, zeroed while `tp < 0.25 FDT` and
  `site < 0.4 FDT`.
So the image spaces landers by time since the last touchdown. Nothing reads the runway's occupants. A wingman can
start its final while the previous plane still taxis, in the image as in the host, and the vehicle arm then avoids
the taxiing plane. 50.7 may therefore be image behaviour.

Still to compare against the image:
- the host's taxi time: II and III need 20-30 s to clear the strip at about 7 m/s, through park's path speed
  `RunwayYawTurnSpdLimit/1`;
- the touchdown point.
Neither has a known host difference.

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

### 51.4 Measured, and the verdict: ON

Both sides are builds of 169839289. OFF is `local\p14_off_rt` (the tree build). ON is `local\p14_rt`
(`--flip kRotateAboutAxisImageTailBound=true`). Reference environment. Logs are `local\p14rt{off,on}_<row>.log`.

| row | `pair_diff` | death rows | plane deaths OFF / ON | NaN |
| --- | --- | --- | --- | --- |
| USN01 3000 | 3 | 33 = 33, 2 changed | 5 / 5 | none |
| JM05 9000 | 3 | 44 = 44, 21 changed; Shokaku (OFF) and Zuikaku (ON) swap | 26 / 26 | none |
| USN04 4500 | 3 | 51 = 51, 27 changed | 51 / 51 | none |
| BSM01 3000 (control) | 1 | 0 | - | none |

1. **Held:** every row with planes moves (3), at the timing level.
2. **Held:** no regime change. The plane death counts are equal on every row, and no NaN appears. The JM05 carrier
   swap is a moved attack timing on the shared stream.
3. **Held:** the control returned 1.

**Verdict: `kRotateAboutAxisImageTailBound = true`.** It is the image's own tail, and every row keeps its regime.

## 52. 007CBA50, the plane water step: what is left (cc9-physics14, 2026-10-17; lead item 3, read-only)

- **Powerlost (`007CBE02`).** `kPlaneWaterPowerLostBound` (ON) routes it as a death. The C3Ah hook skips
  `007CA8A0`'s draw when `plane_water_powerlost_raised` is set (`game_hosts_units.cpp`, "A water powerlost calls
  007BBFA0 directly"). The image's chain is `007BBFA0` -> message 66h -> `007D0B80`:
  - stop the engines and set C39h;
  - the tail sets C3Ah and calls vt[70h](1), Destroy, so +60h;
  - no Kill and no draw.
  The host's `kill_unit_00926d90(self, 1)` differs only in the Kill bookkeeping (+5Fh and the kill cause). The plane
  is dead on both, and no shared-RNG draw is taken on either. Nothing gameplay-visible is left to bind.
- **The stay test (`007CBC6A`-`007CBD6D`, listing read).**
  - It runs only when the height above the water exceeds class+508h + 1.5 and `unit+770h` is set.
  - Each 12-byte point of the class vector `+5BCh`/`+5C0h` is put through the world matrix `+CCh` (`004142E0`), and
    `0078CF20` samples the sea at it. A point below the surface jumps to `007CBE6C`: the plane stays on the water.
  - With none below, it calls `(+72Ch)->vt[24h](0)` (`007CBD6B`) and lifts off.
  - The host carries neither the point vector (it is loaded with the model, like the class+50h box) nor a producer of
    `+770h`. So its lift-off is the 1.5 m test alone.
  - Binding this needs the model's points. It is left with the other mesh-bound items (48.1's deck crash). Not bound.
- **The splash and the +911h latch.** They are presentation only. Not bound.

## 53. State after cc9-physics14's first pass (2026-10-17, at about 55% context)

**Commits on agent/cc9-physics1.** The full `scripts/build.ps1` with tests passes on each.
- 201a1cbf1 / 8acd39e3f: `kStockReturnDestroyBytesBound`, ON (50).
- 169839289 / ae5f2e9a5: `kRotateAboutAxisImageTailBound`, ON (51).
- 18c41f797, 1b52714e4: 50.8 and 52 (read-only), and the section order.

**Open, in order.**
1. **50.7: landers avoiding taxiing planes.** USNOS #4.4|.-IV and #4.5|.-III/-IV still touch down yawed and leave the
   runway. 50.8 reads the sequencer as image-faithful, so compare the taxi time and the touchdown point against the
   image next.
2. **The free-attack tick 009F8470 (48.3).** It needs the squadron director's override layer: 0071D980 ->
   0071E7F0, the director host's 00836D67 arm, and `director+188h`. It touches `game_hosts_commands.cpp`,
   `command_execution.cpp` and the units host's squadron command path. Size it as a fresh worker's packet.
3. **007DC830's extra forces (CC13_FRONT3 5).** The stall block needs `r = forward / StallSpd < StallRangeMax` (1.6);
   a landing flare at 36-39 m/s against 20.56 stays above it. ctl+94h is the wreck/water gate. Neither moves a
   live landing.
4. **The bag > 0 rack with no loadout entry (46).** It needs the model's BSPGun platform slots against each
   Equipments entry. Start with a census.
5. **The catapult adapter (`Seg33aHost`, 37).**

**Tools in `local\`.**
- `p14_slot_call_scan.py <exe> <disp> <slot> [--ptr]`: virtual calls through a member at `+disp`.
- `p14_track.py <csv> <t0> <t1>`: the track heading from a plane CSV.
- `p14_chain.ps1 -Exe -Prefix -Rows 'M:frames,...'`: sequential rows in the reference environment.
- The diagnostic edits (`p14_diag_*.py`, applied to a scratch build only) print the vehicle-avoidance neighbour,
  the band sources and the planner yaw inputs in a land final.

## 54. A positive bag keeps a rack's constructor round: `kRackPositiveBagRoundBound` (cc9-physics14, 2026-10-17; lead item 5, 46's open item)

### 54.1 The image (`006E3350`-`006E3402`, disk listing)

- With an owner answering IsKindOf(0Fh) and `+C54h` (the bag) > 0, it walks `009552E0(bag)`'s entries. An entry whose
  ammo `[+8h]+8h` > 0 and whose key `[+8h]+4h` equals the rack's key (`[+3F4h]+6Ch`, `006E33AA`-`006E33B6`) gets SetAmmo
  (`vt[1BCh]`) and the reload time (`vt[1C0h]`).
- Every path then reaches `006E33E3`. Unless `[+C0h]+4 == 3` (unread, taken as not met), it tail-jumps `vt[224h]`, which
  attaches the next round.
- A rack with no entry in the bag therefore keeps the ammo its constructor `006E3C00` stored, 1, and attaches one
  round. 46 bound this for bag 0 in the 007B9140 answer only.

### 54.2 Reach (census build, not committed: `local\p14bag_<row>.log`)

- **ESMP08 40000 frames, `esmp08_p12_e8_rb`:** 31 planes. All are B6N Jill (class 163) with bag 1.
  - Equipments[1] loads slot 50 with platform 122. Slot 51 is only in Equipments[2] (platform 87).
  - So the slot-51 rack keeps the default device (bullet class 2Bh) and, in the image, one round. The host gives it none.
- **USN04 4500:** none.

### 54.3 The binding

`kRackPositiveBagRoundBound` (`src/game_hosts_units.cpp`), committed **OFF**. With it ON, such a single rack gets
authored ammo 1:
- in `ordnance_racks` (the 007B9140 answer and 007EDAD0's kinds);
- in `plane_rack_census` (`rack_authored_per_rack`, which seeds the release counts).
Summary line: `summary rack positive bag round bound= rounds=`.

### 54.4 Predictions (ON against OFF; written before the runs)

1. **ESMP08 (rb):** `rounds` > 0.
   - Each Jill's slot-51 rack holds a 2Bh round, so its torpedo release issues a second round. The squadron is not
     spent after its first drop: 009F8160/007ED790 still finds a round.
   - Torpedo releases rise, and US ship damage moves. `pair_diff` 3.
2. **USN04 4500 and USN01 3000:** `rounds` 0, exit 0 or 1.
3. **Mechanism failure:** a counted round on a rack whose loadout has an entry, or on a bag-0 plane (46's path).

### 54.5 Measured, and the verdict: ON

Both sides are builds of cd85205bb. OFF is `local\p14_off_br` and ON is `local\p14_br` (the flip). Reference
environment. Logs are `local\p14br{off,on}_<row>.log`.

| row | `pair_diff` | `rounds` ON | outcome OFF / ON |
| --- | --- | --- | --- |
| ESMP08 140000, `esmp08_p12_e8_rb` | 3 | 877 | completed 5444.78 s / completed 3959.94 s (both log the sus_prog.exe refusal) |
| USN04 4500 | 1 | 0 | - |
| USN01 3000 | 3 | 50 | death rows identical (33); shots 2040 -> 2051, hits 1073 -> 1063 |

1. **ESMP08: held.** `rounds` > 0. After one drop each Jill's second rack still holds its round. Zuiho_sqn03's rack
   line reads `drops=1 ammo=1` ON against `ammo=0` OFF. OFF latches `spent ordnance` at 396.93 s; ON has no latch.
   - Each run still releases one torpedo per rack run (`drops=1`). The "second round issued" part of the prediction
     did not show in these lines.
   - The row still completes, earlier.
2. **USN04: held** (0, exit 1). **USN01: failed as a premise.** The H6K Mavis squadrons Mav1-Mav5 carry a positive bag
   with a rack that has no entry (50 rounds counted). The row moves only in shots and hits; the death rows are
   identical.
3. **No mechanism failure.** `rack_positive_bag_round` acts only when the bag is > 0 and the loadout has no entry for
   the rack (`ammo < 0`), so a rack with an entry and the bag-0 path are untouched by construction.

**Verdict: `kRackPositiveBagRoundBound = true`.** It is 006E3350's walk as read. ESMP08 keeps its completion and USN01
keeps its death table.


### 54.6 Follow-up: JM05 9000 under 54's flip

The same OFF and ON builds of cd85205bb (`local\p14br{off,on}_jm05.log`).
- `rounds` = 44 ON: JM05 has positive-bag planes with racks that have no loadout entry.
- `pair_diff` 3. Death rows 44 -> 58: 15 only ON (two Fubukis, USS Perkins, a Clemson, two troop transports,
  SecondaryAirfieldEntity 01_sqn16 x3, Yorktown/Lexington wingmen) and 1 only OFF (Zuikaku).
- So 54 moves JM05 9000 strongly, as 55.4 inferred. The verdict stands (mechanism held), and the row needs re-timing.

## 55. 007DC830's stall block: `kFreeFlightStallForceBound` (cc9-physics14, 2026-10-17; lead item 5, 007DC830 wiring)

### 55.1 The image (cc13's `src/cc13_front3.cpp` reading, spot-checked at `007DC998`-`007DC9B1`)

After the core law (`007DC86E`):
- r = `007D99C0` / StallSpd.
- When StallRangeMax > r, or the plane is dead (+5Dh):
  - m = `00419010(StallRangeMin, 1, StallRangeMax, 0, r)`. A dead plane takes the larger of m and t, where t is the
    DeadMeat StallMul ramp (or the SpinStallMul ramp when spinning) on +C3Ch.
  - g = `00419010(OnPitch, 1, OffPitch, 0, pitch)`. The spot-check confirms the argument order: x1 = `00F87358`
    (SpinStallMulOffPitch) with y1 = 0.
  - d = clamp(+908h - 1, 0, 1).
  - When f = g m d > 0: `007DAFD0(ctl; (0,0,1), (0,-1,0), f x StallRotAccel (desc+17Ch), step, 1.5)`, a nose-down kick.
- The host ran the core law alone.

### 55.2 The binding

`kFreeFlightStallForceBound` (`src/game_hosts_units.cpp`), committed **OFF**.
- ON, `free_flight_stall_force_007dc830` runs after the core law in the free-flight arm (`007CC2F0`) and in the
  ground-roll arm's free branch (`007DCCF0`).
- The kick is the host's `water_angular_kick_007dafd0` on the world angular velocity. That velocity comes from the
  body rates through the pose rows and goes back the same way, as `water_law_tail` does.
- LABELLED:
  - the dying spin `ctl+F0h..F8h` (its constructor draw is not hosted) and `ctl+94h` are not carried;
  - spinning is C36h or C37h (the host has no C38h);
  - StallRotAccel is read per class (`read_vehicle_class_number`, fallback 0, the descriptor's zero).
- Summary line: `summary free flight stall force bound= steps= kicks=`.

### 55.3 Predictions (ON against OFF; written before the runs)

1. **The kicks happen** where planes fly below 1.6 x StallSpd nose-up: climb-outs after lift-off, slow dogfight turns, and
   dying planes (the DeadMeat ramp). `kicks` > 0 on every row with planes. The landing flare stays above 1.6 x StallSpd
   (36-39 m/s against 20.56 for the Liz), so landings barely move.
2. **Moves:** USN01 3000, JM05 9000 and USN04 4500 move (`pair_diff` 3). Dying planes pitch down sooner, which moves the
   wreck impact points. The plane death counts stay equal or close.
3. **Mechanism failure:** a live plane above StallRangeMax x StallSpd getting a kick, or a NaN pose.

### 55.4 Measured, and the verdict: ON

Both sides are builds of b8f7a3c42. OFF is `local\p14_off_sf` and ON is `local\p14_sf` (the flip). Reference environment.
Logs are `local\p14sf{off,on}_<row>.log`.

| row | `pair_diff` | kicks / steps ON | death rows OFF -> ON | plane deaths OFF -> ON | NaN |
| --- | --- | --- | --- | --- | --- |
| USN01 3000 | 1 | 366 / 15236 | 33 = 33, identical | 5 = 5 | none |
| JM05 9000 | 3 | 2246 / 165650 | 58 -> 44 (15 only OFF, 1 only ON: Shokaku) | 34 -> 26 | none |
| USN04 4500 | 3 | 6952 / 130081 | 51 = 51, all changed | 51 = 51 | none |

1. **Held:** kicks on every row with planes, and only below StallRangeMax x StallSpd or for a dead plane (the gate is
   the listing's).
2. **Held, with a large JM05 move.**
   - On JM05 the OFF side sinks two Fubukis, USS Perkins, a Clemson and two troop transports. It also loses three
     SecondaryAirfieldEntity 01_sqn16 planes and two Yorktown/Lexington wingmen. ON keeps them and sinks Shokaku.
   - The OFF base already differs from the 51.4 runs (44 rows then, 58 now). 8b02bc7ac (54) is the only flip between
     them, and its pair did not run JM05. So **54's JM05 effect is unmeasured**, and ON here lands back on 44 rows.
   - USN04 moves at the timing level with equal counts.
3. **No mechanism failure:** no NaN, and no kick outside the gate.

**Verdict: `kFreeFlightStallForceBound = true`.** It is 007DC830's block as read. JM05 9000 moves at exit 3; reference
rows with planes need re-timing on the new base.

## 56. Handoff (cc9-physics14, 2026-10-17, at about 65% context)

**Commits on agent/cc9-physics1 since 53** (the full `scripts/build.ps1` with tests passes on each):
- cd85205bb / 8b02bc7ac: `kRackPositiveBagRoundBound`, ON (54; 54.6 records its JM05 move);
- b8f7a3c42 / 724fb3a6c: `kFreeFlightStallForceBound`, ON (55);
- this commit: 54.6 and this section.
Four switches are flipped in this pass: 50 (stock return bytes), 51 (0085E4D0's tail), 54 and 55. Rows with planes move
on all four. JM05 9000 moves strongly under 54, and under 55 it lands back on 44 death rows. ESMP08 rb completes at
3959.94 s with 54 ON (5444.78 s OFF).

**Open, in order** (53's items 3 and 4 are done):
1. **The free-attack tick 009F8470 (48.3).** It needs the director override layer. Make it a fresh worker's whole packet.
2. **50.7: landers avoiding taxiing wingmen.** It is probably the image's own behaviour (50.8). Compare the taxi time
   and the touchdown point if USNOS needs it.
3. **007DC830's remaining terms:**
   - the dying spin `ctl+F0h..F8h`, which needs `007D7EA0`'s construction draw `U(DeadMeat RotationMin, RotationMax)`
     on the shared stream (CC13_GPR_48 162, RNG census `007D7F90`);
   - `ctl+94h` (the wreck/water gate, 007CE0DB-007CE1D9);
   - spin-left C38h.
4. **The catapult adapter (`Seg33aHost`, 37).**
5. The mesh-bound items: the deck crash's class+50h box (48.1) and 007CBA50's hull points (52).

## 57. The squadron free-attack tick 009F8470: `kSquadronFreeAttackTickBound` (cc9-physics15, 2026-10-17; lead item 1)

### 57.1 What runs, and where

- **The tick is cc13's** (`src/cc13_seg68.cpp`, `squadron_auto_target_tick_009f8470`, CC13_SEG68 1), run unchanged.
  `src/squadron_free_attack.cpp` fills a small byte arena per squadron and per step with the fields it reads, at their
  native offsets: the +348h block (`+30h` mode, `+3Ch`/`+3Dh`, `+54h` slot 0, `+58h` its descriptor), the squadron
  (`+3D0h`, `+54h`, `+370h`), the leader (`+1B0h`, `+184h`, `+C24h`), each contact (`+C8h`, `+FCh`..`+104h`), the enemy
  chain `[slot+DE8h]` (node `+4h`/`+8h`, item `+4h`), the AttackLists block (`004D6CE0`, each vector's begin/end at
  `+4h`/`+8h`) and the tuning record (`+594h` GuardDist). The tick's calls are recorded as a decision.
- **Position.** `0071F290` arm 7 runs `[+38h]->vtable[4]` (009F8470) and then `vtable[7Ch]` (0084E010). The host already
  runs 009F8470's first two calls (009F8160, 009F7C90) and 0084E010's B5 per squadron in `run_spent_ordnance_rtb_0084e010`;
  the selection runs between them.
- **The candidates.** The side's enemy triple (`recon_triple_units(party, 1)`) with a flag per entry for the kind-18h
  group record 00805490 leaves (the host's unit for it is the fused leader). A plane's own class-bucket record is
  IsKindOf(0Fh) and refused by 009F7480; the group record gets its own arena object, so IsKindOf(18h) holds and `+3D0h`
  is the leader, as 009F7BA0 reads it.
- **The score** asks `00804350` (sensors, mask FFh) from the own leader, now `GameGunneryHost::sensors_detect_unit_00804350`
  over the recon pass's own rows (`i*8+j`, `i, j < 7`: `008045E8`/`008045F8` CMP 7; row `i == 4` divided by the sonar
  multiplier squared), and 007EEC50 through the script-order host's chooser on the leader.
- **AttackLists** are this installation's `scripts/datatables/squadronfreeattacktargets.lua` (mtime 2024-07-13, untouched
  bulk), transcribed (LABELLED), names mapped through 00E0CD80 read from the disk image.
- **The +3Ch/+3Dh bytes** are the script-order host's (`UnitSetFireStance`, else 0084D810's defaults:
  Behaviour `+364h` 0 frees both; otherwise a fighter class gets fire only, anything else move only;
  `0084D862`-`0084D8A2`, read here).
- **0084D960**, the block's `vtable[50h]` the tick asks at 009F8584: 0 when `+3Dh` and `+3Ch` are both set, else 1.
- **0071E4C0** (filter C's path point): slot 0 `moveonpath` gives the path's final point (`007B1350`), otherwise slot 0's
  descriptor point (`006F7DD0`), otherwise none.

LABELLED substitutions: 00927F10 for a party slot other than 8 answers player-held; leader `+C24h` (PilotFires) has no
reader and is clear, so `007EDAC0` is false; 007ED7E0 / 007ED8D0 / 007EDA10 / 007ED880 read the members' ordnance masks;
007EE8B0's `+361h` is the retreat exit latch the units host keeps by squadron name.

## 58. 007DC830's dying spin and wreck lift: `kFreeFlightDyingSpinBound`, `kFreeFlightWreckLiftBound` (cc9-physics15, 2026-10-17; lead item 2)

### 58.1 The image (cc13's `src/cc13_front3.cpp` reading of 007DC830; 007D7EA0 per CC13_GPR_48 161)

- **The dying spin.** Inside the stall block's `m > 0` arm, after the stall kick, a dead plane (`+5Dh` re-read at
  `007DCAF2`) adds `step * ctl+F0h..F8h` to `ctl+24h..2Ch` (`007DCAF9`-`007DCB40`; each product stored as a float first).
  `ctl+24h` is the world angular velocity (CC13_GPR_48: the parent-frame rotation carries it with `ctl+18h`).
  `ctl+F0h..F8h` is set once by the controller constructor 007D7EA0: `m = U(+250h, +254h)` (`007D7F90`). Both
  fields hold Dynamics/DeadMeat/RotationMin, DEG(10): the loader pushes the RotationMin key twice and RotationMax is never
  read (GAME_TUNING_SINGLETON "Keys the loader reads"), so the magnitude is 10 degrees/s and only the direction is drawn. It
  multiplies `00439950`'s direction (`z = U(-1, 1)`, `phi = U(0, 2 pi)`), three stream-1 draws.
- **The wreck lift** (`007DCB4B`-`007DCCCB`), outside the stall block, with `ctl+94h` set: `v` = the world velocity with
  `y = 0`; `h = 00419010(0, 4, 69.444443, 20, |v|)`; `u = max(0, 1 - |+C64h|)`; `q = min(1, |+C68h| * 0.8 + 0.6)`; the
  `007DAFD0` kick at `(0,0,1)` toward `(0,-1,0)` of `q u h`, limit `1000.0f` (`00CE3804`).

### 58.2 The binding (both committed OFF, `src/game_hosts_units.cpp`)

- `kFreeFlightDyingSpinBound`: the stall block no longer returns on `f <= 0` for a dead plane; the kick still needs `f > 0`;
  the spin is added to the world angular velocity the kick uses and goes back to the body rates once.
  LABELLED: the three draws come from the plane's keyed stream (`name#ds_m`, `#ds_z`, `#ds_p`) at its first dying step,
  not from stream 1 at construction (the shared stream would shift every later stream-1 consumer by three draws per plane).
- `kFreeFlightWreckLiftBound`: `free_flight_wreck_lift_007dcb4b` after the stall block when `ctl+94h` is set. The host
  sets `ctl+94h` only in the water arm (`007CBC02`); 007CE040's hull-point arm (`007CE0DB`-`007CE1D9`: a dead plane below
  the altitude at `00CF3F20` with any class `+5B8h` hull point below `y = -1`) needs the mesh points and is not carried.
- Summary line: `summary free flight dying spin bound= steps= seeds= wreck_lift_bound= wreck_lift_kicks=`.

### 58.3 Predictions (ON against OFF; written before the runs)

- OFF: identical by construction (the restructured stall block runs the same kick and write-back).
- Dying spin ON: every row with planes shot down in free flight moves from the first dying step: the wreck's attitude
  and track, so where and when it reaches the water (the plane's removal time in the death table). Ships, the scored
  deaths and AA kills before the first plane death stay identical. `seeds` = the number of planes that died in the
  stall block's arm; `steps` grows with each dying plane's fall.
- Wreck lift ON: `wreck_lift_kicks` 0 or near 0 (ctl+94h is set only after a dead plane's water contact, and such a
  plane stays in the water arm), so gameplay identical or nearly so.

### 58.4 Measured, and the verdict: both ON

Both sides are builds of d97c232cf: OFF `local\p14_sf` (no flip), ON `local\p14_br` (both switches). Reference
environment, `--stop-after-end 40`. Logs `local\p15sp{off,on}_<row>.log` with manifests. No NaN on any ON log.

| row | `pair_diff` | seeds / steps ON | wreck lift kicks | death rows OFF -> ON | first moved row |
| --- | --- | --- | --- | --- | --- |
| USN01 3000 | 1 | 5 / 638 | 0 | identical | none (gameplay identical) |
| USN04 4500 | 3 | 40 / 5465 | 0 | 51 = 51, 20 changed | B5N Kate #4.1/.-IV, the first plane death (killer_range 954 -> 953) |
| JM05 9000 (fails at 300.05 s both sides) | 3 | 24 / 2255 | 0 | 44 = 44, 18 changed | USS Yorktown_sqn06/.-II, the first plane death |

1. **Held:** seeds only for planes that died in the stall block's arm, and nothing moves before the first plane death
   on either moved row; the later rows move through the AA that keeps firing at the falling wrecks (killer changes).
2. **Held:** no wreck-lift kick anywhere.

**Verdict: both ON.** `kFreeFlightDyingSpinBound` is 007DC830's `007DCAF9` arm with 007D7EA0's draw (keyed stream,
labelled); `kFreeFlightWreckLiftBound` is `007DCB4B` as read and has no reach in these rows. USN04 and JM05 move at
the timing level with equal death counts; reference rows with plane deaths need re-timing on the new base.

## 59. 50.7 closed: landers avoiding taxiing wingmen is the image's behaviour (cc9-physics15, 2026-10-17; lead item 4, read-only)

- **Spacing.** 50.8: `006C7960`'s mode 4 needs `rec+8 > 0`, which `006C3F80` derives from the other landers' path gaps
  or, with none, from the time since the last touchdown (`site+40h`, restamped by `007CB5F0` -> `006CE230`). Nothing in
  the sequencer reads the runway's occupants, so a wingman starts its final while the previous plane still taxis.
- **The taxi itself** is the land task's park phase as read in SQUADRON_LAND_TASK (`009B24F3`-`009B270D`): the speed
  request `00419010(5 base, base, 10 base, max(base, 0.6 StallRangeMax StallSpd), dz)` with `base` = AirField/MoveSpd
  (Pilot/Landing/ParkVelocity on a carrier), `0.85 v` beyond 100 m, and below `1.5 base` the turn distance
  `qd = (1.5 / rate) RunwayYawTurnSpdLimit/1` with the speed set to RunwayYawTurnSpdLimit/1 inside `qd + 30`. The host's
  arm (`src/game_hosts_units.cpp`, the land task's park step) carries each term with its address; no host
  difference was found in the taxi speed or the path join.
- **The avoidance** a lander then gets from a taxiing plane is the vehicle arm's (`007DF96C`-`007DF9A9`, every listed
  neighbour, mask FFh), as 50.7 says.

So a lander touching down yawed while a wingman taxis is what the image's own sequencer, taxi law and vehicle arm
produce together. The touchdown point was not compared against the image (no image run is possible here); nothing read
in this pass points at it. **Closed**, no binding. If USNOS #4.4/#4.5 needs a different outcome, that is a re-plan of the row.

### 57.2 The binding (`kSquadronFreeAttackOverrideBound`, `include/bsp/game_hosts_commands.hpp`, committed OFF)

- **The override layer** is ships56's (219b1d3c0, `send_override_command_0071d980`, `raise_override_stage_0071d9e0`,
  0071EB60's override arm, 0071C130(0)). It now runs for either sender: `kDirectorOverrideLayerBound =
  kAutoTargetOverrideBound || kSquadronFreeAttackOverrideBound`; the ship tick's step 14 and 00836D67 stay on
  `kAutoTargetOverrideBound` alone.
- **0071E7F0's repeat test** (cc13-verify1): the early return needs `0071E200(new, director+18Ch)`, the same resolved
  object AND a squared distance `<= 1.0` between the two points (the `00F87574` zero vector when byte `+1` is clear;
  x87 at `0071E27B`). The host compared the resolved objects only, so it refused a moved entity's and every positional
  override. Only the override path reads it.
- **Per squadron and step** (`run_squadron_free_attack_009f8470`, called from `run_spent_ordnance_rtb_0084e010` after
  009F8160 / 009F7C90 and before B5):
  1. The live override's target removed (0071DDB0's override arm -> 0071EDD0): a squadron target moves to its next
     live flight leader (the override is re-sent on it), anything else ends it (0071D9E0(2)). LABELLED: tested per step
     on the gunnery death, not at the destroy flush.
  2. The override cleared (the leader's mode is no longer 2): the other members' overrides are cleared too (the host's
     members each hold one; the image has one block), the override rows are retired, and 0099A170 builds the head
     command's task again (the moveto task restarts).
  3. The tick (57.1). Its 0071D9E0(2) goes to every live member; its 0071D980 goes to every live member's director,
     and each member that took it gets the class's task (0099A170), its command target and a retired moveto task.
- LABELLED, besides 57.1: 009F85EB..009F8615 (a `stop` head is cleared and replaced by a moveto to the squadron's own
  position before the send) is counted (`moveto_self`) and not issued, so after the override the head is still `stop`;
  0071F290 arm 5 (0084DDA0 -> 0071F600, the override's begin) is not run, the send having just passed 0071D6D0;
  0084E010's `+50h == 1 -> 0071D9E0(2)` is not carried (no host path ends an override at stage 1).
- `BSP_FREE_ATTACK_CENSUS=1` runs the tick and logs each squadron's stop reason when it changes, with nothing sent.

### 57.3 Reach (census, a local build of the edits, `local\p15fc_<row>.log`)

| row | squadrons | first-stop counts (mode / no_target / same_target / override) | first would-be override |
| --- | --- | --- | --- |
| USN04 4500 | 21 | 21 / 29 / 4 / 40 | 63.0 s: Zero #1.2 (moveto head) dogfights Lexington sqn01; the US fighters (stop head) dogfight the Zeros from 70.7 s |
| JM05 9000 (300 s) | 19 | 19 / 22 / 0 / 11 | 88.8 s: MainAirfieldEntity 01_sqn01 strafes Japan Troop Transport 03 |
| ESMP08 6000 | 12 | 12 / 12 / 0 / 0 | none within 300 s |
| BSM01 6000 | 2 | 2 / 2 / 0 / 0 | none within 300 s |

### 57.4 Predictions (ON against OFF; written before the runs)

- USN04 and JM05 move from the first override (about 63 s and 89 s): fighters on a moveto or stop head turn on enemy
  squadrons in sensor range (dogfight 00E08F58), the strike squadron strafes. Death rows before then are identical.
- ESMP08 and BSM01: identical for the census window; later moves only if a squadron with a moveto/stop/moveonpath head
  and ordnance or a fighter class meets a detected enemy.
- Counters: `sends` = overrides installed; `cleared` follows each dead target or ended attack; no member may hold
  mode 2 after its leader's override cleared.

### 57.5 Measured, and the verdict: ON

Both sides are builds of dd056450d: OFF `local\p14_sf`, ON `local\p14_br` (the flip). Reference environment,
`--stop-after-end 40`. Logs `local\p15fa{off,on}_<row>.log` with manifests. No NaN on any ON log.

| row | `pair_diff` | sends / cleared ON | deaths OFF -> ON | first moved |
| --- | --- | --- | --- | --- |
| USN04 4500 | 3 | 37 / 29 | 51 -> 56 (5 US fighters only ON, Zero #8.2/.-II only OFF) | first hit 97.05 -> 90.50 s, after the first override at 63.05 s |
| JM05 9000 (fails at 300.05 s both sides) | 3 | 2 / 0 | 44 -> 46 (Lexington sqn03 only ON) | MainAirfieldEntity 01_sqn01's strafe override (88.8 s); its deaths move from 109 s |
| BSM01 30000 | 1 | 0 / 0 | identical | none |
| ESMP08 30000 | 0 | 0 / 0 | identical (184) | none (every tick ends at no_target: no head is moveto/stop/moveonpath with ordnance or a fighter, or no detected contact) |

1. **Held:** the moved rows move from the first override and not before; the rows with no override are identical.
2. **Held:** on USN04 the US fighters (stop head, fire only, filter C) dogfight the escorting Zeros and the Zeros
   (moveto head) dogfight the US squadrons; removed targets re-point or end (29 clears, 61 head tasks rebuilt).

**Verdict: `kSquadronFreeAttackOverrideBound = true`.** It is 009F8470 as cc13 read it, on ships56's override layer with
cc13-verify1's 0071E7F0 fix. USN04 and JM05 move strongly; reference rows with squadrons on moveto/stop heads need
re-timing on the new base.

## 60. Handoff (cc9-physics15, 2026-10-17, at about 72% context)

**Commits on agent/cc9-physics1** (the full `scripts/build.ps1` with tests passes on each):
- 79f8074c2: script-order accessors (permissions, 007EEC50, 0099A170 install);
- d97c232cf / 99923ee2f: `kFreeFlightDyingSpinBound`, `kFreeFlightWreckLiftBound`, both ON (58);
- d142cb67c: 58.1 wording, 59 (50.7 closed as image behaviour);
- c8d1f9463: the override-layer gate `kDirectorOverrideLayerBound` and 0071E7F0's 0071E200 repeat test (cc13-verify1);
- dd056450d / 7bc50f281: the free-attack tick 009F8470, `kSquadronFreeAttackOverrideBound`, ON (57).

**Open, in order.**
1. **The catapult adapter (`Seg33aHost`, 37).** Not started. cc13's seg33a catapult routines take raw object pointers
   (206 host virtuals in `Seg33aHost`), so the arena approach of `src/squadron_free_attack.cpp` (fill the fields a
   routine reads at their native offsets, record its calls) is the cheapest faithful adapter. 37's four gaps stand
   (State 3 hold, no catapult devices, GetNumCatapulted / SetCatapultStock, shared-file hooks). Pair on JM02/JM03/JM05.
2. **Free-attack leftovers (57.2 LABELLED):** the `stop` head's clear-and-moveto-self (009F85EB..009F8615) is counted,
   not issued; 0071F290 arm 5 (0084DDA0) and 0084E010's `+50h == 1` raise are not carried; 00927F10 for slots other
   than 8; leader `+C24h` (PilotFires) has no reader.
3. **007CE040's hull-point arm** sets ctl+94h (007CE0DB-007CE1D9); it needs the class `+5B8h` hull points (mesh-bound,
   with 48.1 and 52).

**Tools in `local\`.** `p15_kind_names.py` (00E0CD80's names from the disk image), `p15_vt.py <va> <n>` (a vtable's
slots from disk), `p15_cl_check.ps1` (one-file compile check), `p15_census_sum.py <logs>` (the free-attack census),
`p15_edit_*.py` (the edits as applied).

## 61. GetProperty "TorpedoStock" and the squadron "state" (cc9-physics16, 2026-10-17; lead item 1)

**Image.** `00815870` (vtable+138h of the eight ship vtables, and `00758340`'s first call for the
mother ship) runs `00779BB0`, then `_stricmp(key, "TorpedoStock")` at `00815891` (string `00D09484`),
and pushes `00815850`'s value through `00B664B0`. `00815850-00815867`: `s = unit+104Ch`;
`TEST ESI,ESI / JL 00815864` pushes `s` when negative, else `00810E90() + s` (the loaded torpedo
tubes). `007EF1C0` (PlaneSquadronGen `00D087C0` +138h): after `ammoType`, `00425850(key, "state")`
at `007EF20E`; over the `+3CCh` planes at `+3D0h` it counts A = `(plane+72Ch)->vtable[38h]()` true
(the free-flight control mode, `0074E210`: `+1D4h == 7`, kept by this host as `+900h`) and B = the
rest; pushes 2 when B == 0 (an empty squadron too), else 1 when A > 0, else 0.

**Host.** `kGetPropertyTorpedoStockBound` (`include/bsp/game_hosts_lua.hpp`) answers for every
`IsKindOf(6)` unit through `GameGunneryHost::torpedo_stock_total_00815850`. Pass C (`008201B8`) sets
every ship's spare at load; this host sets it at the first shot, so an unset ship answers what pass C
leaves (MaxTorpedoStock, or the tube count when the unload loop is off). **LABELLED:** a
ShipSetTorpedoStock queued this frame answers `max(stock, 0)`, which is spare + loaded after
`0081F8B0` on both arms. `kGetPropertySquadronStateBound` answers `state` for a squadron slot.
`WingCount` has no GetProperty reader: `00CF8840` is read only as a bag key (`007F4735`, `006C50EE`,
`0084521A`, `006EC979`, `006FED65`, `0094BCD8`, `0094C0D1`), and the two scripts that name it do so
in a comment.

**Predictions (OFF -> ON).** JM08: `luaJM8Checktorp` stops raising on `torp < 12` (nil), so the US PT
loop reaches `NavigatorAttackMove` and `luaSetScriptTarget` for each idle PT; the PTs attack, and
`ReloadTorpedoes` (`008C3CE0`, not bound in the host) is called and recorded. Missions using
commandhelpers' airstrike manager (`luaAirstrikeManager_think`) issue attack orders when every member
of the squadron is in free flight. No other key moves.

### 61.1 Pairs and flip (2026-10-17)

`local\p16_ts_off` / `local\p16_ts_on` (commit ffa8b9684, both switches flipped), 12000 mission frames,
reference env. Manifests `local\p16ts{off,on}_{jm08,jm06,jm05}.log.manifest.json`.

| row | asked | served | not ship | verdict |
| --- | --- | --- | --- | --- |
| JM08 | 0 | 0 | 0 | gameplay identical (exit 1) |
| JM06 | 11 | 6 | 5 | gameplay identical (exit 1) |
| JM05 | 27 | 27 | 0 | gameplay identical (exit 1) |

JM08's US PT loop (`luaJM8CheckUSNPts`) needs an enemy within 3000 m of the first PT, and no run reached
it: neither this 600 s pair nor the 140000-frame wins run `local\p8w2_j8f9.log`, whose log has no
`TorpedoStock` ask at all. JM06's five refusals are `luaJM6AddTorp` asking `Mission.PlayerUnit` while that
is not an `IsKindOf(6)` unit, where the image's reader is not reached either. The squadron `state` key was
asked by nothing: its only reader, commandhelpers' `luaAirstrikeManager_think`, is never attached by any
script or scene in this installation. Both switches are flipped ON by mechanism; the JM08 PT attack stays
unmeasured until a run brings the IJN fleet within 3 km of the PTs.

## 62. The planner's control laws: verify12 R5, R6, R7 (cc9-physics16, 2026-10-17; lead item 2)

Four OFF switches in `GameUnitsHost::Impl` (`src/game_hosts_units.cpp`), with the pure parts in
`src/plane_ai_control.cpp`.

- **`kPilotModeOneRollTuningBound` (R5).** 0099E26E-0099E39D is one arm for both roll modes. Mode 1 now
  takes it whole: 0099E27B-0099E2B5 clamp plan+2C4h in place (`00415690`) to +-plan+2C8h when that is
  below pi (the double `00CE3D28`); the band, gain and rate are `[EBX+40h/44h/48h]` with EBX =
  singleton+538h (0099D47F-0099D487): SoftRollCtrl, SoftRollMul and +580h; the servo limit
  0099E344-0099E367 is class+1A8h x float(class+1A8h / class+1BCh / WaggleLimit)
  (`pilot_roll_servo_limit_0099e344`). Before: band 0, gain 1, rate 0, limit 1, no clamp, so every
  mode-1 bank hold (follow, dogfight, strafe, level-bomb holds, dive glide and turndown, land and
  takeoff halves) ran a unit-gain servo saturating at 1 rad of error instead of the image's
  0.05-rad soft band with gain 0.7 and an offset beyond it.
- **`kPilotSpeedCorrDivisorBound` (R6).** plan+2B8h gets its producer. 0099D756-0099D79A (every pass
  past the centred gate): above 1.0 it falls by TrgSpeedCorrMulDecay (+5F0h, EBX+B8h) x `[ESP+6Ch]`
  (dt / dtScale), floored at 1.0. 0099D924-0099D970 (head of the demand arm, reached only through
  0099D8CD, while the byte plan+2B0h is clear): `00415550` max(+2B8h, `00419010`(-MinPitch, SpeedMul,
  0, 1, unit+C64h)). 0099D99E-0099D9A3 divide the measured speed by it. **LABELLED:** the start value is
  1.0. A census of `[reg+2B8h]` stores over 00990000-00A00000 (`local\p16_disp_writers.py`) finds only
  009A4ECD, 009A50F6 (a task constructor family whose `+2ACh = this` store does not fit the plan block)
  and 009B9CB2; the pilot bot (`0099A880`, 0BCh bytes) is not the planner object.
- **`kPilotThrottleForwardSpeedBound` (R6, the measured speed).** 0099D992-0099D99E take 007D99C0
  (ECX = unit+AB0h), the body forward speed, not |v|. The host takes v . forward row, as it already
  does for 007D9A70's argument. **LABELLED:** the carrier-deck parent term of 007D99C0 is dropped.
- **`kPlaneBombLoadAuthorityBound` (R7).** 007D9A70 returns 007C0F40(unit) x t^2. 007C0F40: 1.0, or
  for the first slot of +974h whose vtable[210h](2Ah, 0) answers (007C0F72), `00419010`(0, 1, 0.8
  (00CE74F8), class+15Ch BombControlLimit, slot vtable[214h]); x class+608h when unit+BC8h (turbo).
  **LABELLED:** the slot test is `rack_holds_round`; a single rack's fraction (006E4130) is the host's
  rounds left over the authored Ammo; a multi rack (006E3700) answers 1.0; the turbo arm is not taken.

**Predictions (OFF -> ON).** R5: mode-1 bank holds settle softer (|roll command| below 1 near the
target) and overshoot less; dogfight and strafe geometry move. R6: in a dive (pitch -30 deg) the speed
hold sees the plane 1.69 x slower and opens the throttle, so dive bombers and torpedo glides dive
faster and later level-offs move; the forward-speed term lowers the measured speed in sideslip. R7:
loaded torpedo and bomb planes rotate at 70-90% of today's authority until release, widening their
turns and approach geometry. Every AI plane mission moves; deaths and kill tables may move.

### 62.1 Pairs and flip (2026-10-17)

`local\p16_x_off` / `local\p16_x_law` (commit 7b012d81a, the four switches flipped together), 12000
mission frames, reference env; manifests `local\p16x{off,law}_{usn04,jm05,esmp08,usn01}.log.manifest.json`.
ON counters: USN04 mode-1 tuned 50468 thinks, divisor raised 56003, bomb-load scaled 78783; JM05 40612 /
17061 / 110355; ESMP08 5 / 36451 / 514773. No NaN or inf in any log.

| row | deaths OFF -> ON | dive-bomb releases | verdict |
| --- | --- | --- | --- |
| USN04 | 59 -> 53 | 0 of 20 -> 3 of 27 | moved (3) |
| JM05 | 33 -> 34 | 5 of 10 -> 7 of 20; failed 300.05 s -> 285.05 s (carrier lost) | moved (3) |
| ESMP08 | 36 -> 36 | 0 of 24 -> 0 of 24 | moved (3) |
| USN01 | 52 -> 52 | 4 of 2 -> 2 of 1 | moved (3) |

The movement is where section 62 put it: Kate and Val attack runs end later and lower, Corsair strafes
shift, dive bombers release where they did not. The mechanism matches the image on every arm, so all four
switches are flipped ON as faithful fixes; any WINS row they move is to be re-planned.

## 63. MoveTo cruise altitude by ordnance and the leg manager: verify12 R1, R4 (cc9-physics16, 2026-10-17; lead item 3)

- **`kMoveToCruiseByOrdnanceBound` (R1).** 009C3650 (the kind-7 task's +54h profile) picks ctl+394h by
  ordnance exactly as 009BD6E0 does: torpedo (007B93F0, MoveOn +43Ch) +430h, depth charge (007B94F0,
  +4B4h) +4A4h, general bomb (007B9320) LevelBomb (0047B850, +454h) +444h or DiveBomb (+4D4h) +4C0h, each
  plus 0.6 x TravelAltRandom (009C36DD-009C36F2); otherwise 009C38D0 0047B880 then 009C38FD Small/Large
  PlaneTravelAlt with no random term. The host calls `moveonpath_cruise_profile_009bd6e0` with the
  channel open and discards its approach+30h draw. **LABELLED:** every member computes it at the 0.5 s
  refresh from its own ordnance; the image's leader writes the squadron block at activation.
  This installation: torpedo 530, dive 1330, depth charge 730, fighter 800, level bomber 1400 (MoveOn
  false), against 830 / 830 / 830 / 830 / 1430 before, ahead of the +2Ch offset both subtract.
- **`kMoveToLegAltitudeBound` (R4, with `bsp::kLegAltitudeManagerBound`).** 009BEC85 009FD050(+64h,
  point) registers inside 3000 m and drops outside; while +28h > 0, 009BECA9 009BD400 and 009BECBA
  009FC260 write the separated height to the steer point's +4Ch. Slot +58h 009C32A0 (009C32B8
  009FD050(-1)) drops the record when the task goes. **LABELLED:** the drop is taken on the first think
  after the host clears `moveto_task_installed`.

**Predictions.** Scripted MoveTo strikes fly at their ordnance altitude: torpedo squadrons 300 m lower,
dive bombers 500 m higher, fighters 30 m lower; time to target and the first-attack geometry move.
Planes converging on one MoveTo point within 3 km stack in 30 m bands above 120 m.

## 64. Carrier landing: the velocity lead and the touchdown brake, verify12 R2, R3 (cc9-physics16, 2026-10-17; lead item 4)

- **`kCarrierLandingVelocityLeadBound` (R2).** 009AFAF0's on-ground arm: e = |00605070(A4 - unit+C6Ch)|
  (fmod by 2pi `00CE3828`, then into [-pi, pi]); A8 += min(class+268h x 0.5 x e, A8 x 0.18 (`00D05AA0`))
  and is stored (009AFD1A); t = min(A8 / +B1Ch, 10.0 (`00CE38B8`)); 006BCA80(out, t) adds the owner's
  velocity x t (006BCBF8-006BCC51). **LABELLED:** holder+8Ch is 0, so the turn about the holder is the
  identity; +B1Ch is taken as |velocity|.
- **`kCarrierTouchdownBrakeBound` (R3).** 009B201B-009B2074: air brake 1.0 (`00D7A24C`) when the land
  target answers vtable[5Ch](9), else 0.2 (`00CE54A0`).

**Predictions.** Carrier landings under way aim up to 10 s of ship travel ahead (a 15 m/s carrier: up
to 150 m), so final approaches line up on where the deck will be; fewer wave-offs and go-arounds. On a
carrier deck the roll-out brakes at 1.0, so planes stop shorter; airfield landings are unchanged.

## 65. Lua natives from cc13 verify5 (cc9-physics16, 2026-10-17; lead item 5)

- **R6, `kLuaUntouchableClearBound`.** RemoveUntouchableUnit 008AC2B0: 00888AA0 (008AC3AF),
  vtable[140h] (008AC3D0), `MOV byte [EAX+1D4h],0` (008AC3D6). IsUnitUntouchable 008AC420 pushes
  vtable[140h]() != null && +1D4h != 0. jm05:1242 hands a captured PT boat to the player and releases it;
  usn_06:1023 gates its release on IsUnitUntouchable (nil before).
- **R5, `kGenerateObjectEveryCallBound`.** 0046D930 has no already-created arm; every call instantiates.
  usn_03:1621-1636 (PRCP01) makes Max Kate/Val pairs per pass instead of moving one pair.
- **R3, `kLuaTrulyDeadOnKilledBound`** (with `kLuaTrulyDeadBound`). The two units-host sites that apply
  the kill list's drain and record 00928C80 (the KillDepth drain and the row-15 removal) now call
  `GameMissionLuaHost::on_entity_killed_00928c80`, which sets the self table's `Ptr` to lightuserdata(0)
  as 00928D04 does. **LABELLED:** the last-position table (00928D49..00928E7E) is not written.
- **Census `:3689`, `kSpawnPlacementMapBoundsBound`.** 00941D30's 0071C4F0 test reads this host's own
  Map bounds (`border_bounds_`), so a SpawnNew member outside the map takes the retry / requeue arms.
- **R1, R2, R4** are the existing `kLuaIsInBorderZoneBound`, `kLuaSquadronSetReleaseAltBound` and
  `kLuaSquadronLandAndKillBound`; they are paired here with R5 and R6.
- **R7 is not bound, and verify5's reading is corrected.** 007BC0AA-007BC10C take the engine index from
  Lua argument 2 through 00B66290 (lua_tonumber, then 00BF7420 ftol); only an explicit -1, or a call with
  no argument frame (007CA8A0's death chooser), draws `NextU32` on stream 1 (007BC0FC). LOMP08:902 and
  LOMP09:1157 pass two arguments, so the index is 0 and nothing is drawn. The effect is message 65h ->
  007EB0B0 destroying that engine (CC13_GPR_47 1.3 step 6); this host has no per-engine model, so the arm
  stays a record.

**Predictions.** USN02 / JM05: a released unit becomes targetable again (fire at it resumes). PRCP01:
more Kate/Val strikers per wave; JM06: the Lexington-exit failure check becomes reachable; USN01: Val 1/2
dive altitude capped at 700 m for the rest of the mission; usn_03 squadrons land and are removed.
TrulyDead: JM06's Yamato completion test and commandhelpers' loser-unit choice see sunk hulls removed.

## 66. Pairs for sections 63-65, R7's turbo arm, ReloadTorpedoes (cc9-physics17, 2026-10-17)

**Air switches (63, 64).** `local\p16_x_off` / `local\p16_x_air` (commit 7b012d81a, the four flipped together),
reference env; manifests `local\p16x{off,air}_{usn04,jm05,usn01}.log.manifest.json` (12000 mission frames) and
`local\p17x{off,air}_usn01w.log.manifest.json` (30000 frames, `--helm-orders` `config\win_orders\usn01_p8_u1_i.txt`).

| row | counters ON | verdict |
| --- | --- | --- |
| USN04 | by_ordnance 1489, leads 0 | moved (3): deaths 59 -> 53, dive-bomb releases 0 of 20 -> 2 of 27 |
| JM05 | all 0 | gameplay identical (1) |
| USN01 | leads 583 | moved (3): KatSBD V ditches 118 m short of the led deck point (574.78 s) instead of landing; Enterprise then sinks at 596.38 s (Nell6; it had 179 hp left OFF) |
| USN01 win orders | leads 4761 | moved (3): all 11 Enterprise landings finish 2-7 s earlier; the mission still completes at 1230.07 s on both sides; deaths 56 both |

- `kMoveToCruiseByOrdnanceBound`: the cruise arms fire where 63 put them (USN04's scripted MoveTo strikes). **ON.**
- `kMoveToLegAltitudeBound`: no row registered a leg (separations 0 everywhere). Flipped as the image's structure,
  **ON, unexercised.**
- `kCarrierLandingVelocityLeadBound`: the lead is 009AFC58-009AFD52 as read (disasm checked: e from vtable[50h],
  vtable[38h] = 007B8E60 `+B1Ch`, the 0.18 cap `00D05AA0`, the 10 s cap `00CE38B8`, 00414C60 over x and z). Landings
  line up earlier; the one ditch is recorded, not explained. **ON.**
- `kCarrierTouchdownBrakeBound`: **unreached.** 009B1EF2's touched byte never sets in this host (`summary landing
  plane ... touched=0` on all 13 USN01 landings, which go to the park through the ground-roll law), so the
  land-final ground half that holds the brake does not run. **ON** (identity); the touched byte is the open item.

**Lua natives (65).** `local\p16_x_lua` against `p16_x_off`, 12000 frames. PRCP01 (`local\p17x{off,lua}_prcp01`):
moved (3), units 23 -> 95, Kate1/Val1 instantiated per pass (#2..#4), first hit -1 -> 365.23 s, failed at 456.11 s
(Enterprise sunk) where OFF never ends. That is `kGenerateObjectEveryCallBound`: **ON** (lease on
`game_hosts_lua.hpp` queued). USN02, JM05 and USN01: gameplay identical; no RemoveUntouchableUnit, IsInBorderZone,
SquadronSetReleaseAlt or SquadronLandAndKill call in 600 s. This installation's callers: IsInBorderZone jm02,
jm03:2078, jm04, jm05:4211, jm06:996, jm13; RemoveUntouchableUnit jm05:1242, jm10, jm13, bsm_09:807;
SquadronSetReleaseAlt bsm_09:321. JM02 / JM03 pairs follow.

**R7's turbo arm.** 007C0FCF-007C0FE4 multiply the bomb-load factor by class+608h TurboControlLimit while
plane+BC8h is set. `bomb_load_factor` now takes it with cc13-fix89's latch, under `kKamikazeBoostTurboBound`
(49575c7db).

**ReloadTorpedoes 008C3CE0** (`kReloadTorpedoesBound`, OFF, 057145750). 008C3E00-008C3E47: for each device
(`[unit+48h]`, next `+44h`) answering vtable[5Ch](20h) with `[[+3F4h]+80h] == 7`, 0072CF00(gun, b, 0.0f, 0) for
b in 0..`[+448h)`: `[+414h][b] = 0` (0072CF88), so every tube, fired ones included, is loaded at once; the stock is
not drawn. LABELLED: a torpedo gun is a host gun row of category 7; the call is queued and applied at the next
gunnery fixed step; 0072CF00's torpedo-model tail is presentation. This installation's callers: commandhelpers.lua
3531, 3595, 14801; jm03:1486 (`luaJM3ReloadTorp`); jm07:1918; jm08:774; jm11:1716 (a second argument is ignored).
The script-orders binding is pending its lease.

## 67. Lua flips, cc13's plane switches, the catapult adapter (cc9-physics17, 2026-10-17)

**Lua (65), at 49575c7db / 7b012d81a** (`local\p17hoff_*`, `local\p17htd_*`, `local\p17x{off,lua}_jm06w`):
- `kLuaTrulyDeadBound` + `kLuaTrulyDeadOnKilledBound`: JM06 win orders (40000 frames) and USN04, gameplay identical;
  the kill hook nulled `Ptr` 16 times on JM06; no TrulyDead call in either. **ON** (334be0e5f).
- `kSpawnPlacementMapBoundsBound`: JM06's SpawnNew placements ran the bounds test, identical. **ON.**
- `kLuaIsInBorderZoneBound`: 82 calls on JM06 (jm06:996), identical. **ON.**
- `kLuaUntouchableClearBound`, `kLuaSquadronSetReleaseAltBound`, `kLuaSquadronLandAndKillBound`: no call in USN01,
  USN02, JM02, JM03, JM05, JM06, PRCP01 (600 s, JM06 2000 s). **OFF, unexercised.**

**cc13's plane switches, at 49575c7db** (`local\p17_h_{off,plane,band,gun}`, 12000 frames):

| row | plane (all four) | band only | gun only |
| --- | --- | --- | --- |
| USN04 | moved: deaths 53 -> 52, dive releases 3 of 27 -> 2 of 34 | moved: hull hits 398 -> 638, dive 3 -> 0 | moved: deaths 53 -> 52, dive 3 -> 1 |
| USN01 | moved: completed 554.99 s -> no end; deaths 52 -> 17 | the same numbers as the plane set | - |
| JM08 | moved slightly (hits 7113 -> 7129) | - | - |
| USNOS win orders, 200000 frames | failed 2326.14 s (OFF) -> failed 964.89 s | - | - |

- `kFollowBandSafeAltFloorBound`: 009C16E5-009C175A checked in the listing: L = max(min(leaderY + block+4,
  state+88h), leaderY - 120.0 (`00D1F3F8`)), state+88h = SafeAlt 60 (planeglobals.lua:498; CC13_FIX5). USN01's
  12000-frame completion is lost with it alone. **ON** as the image's law; USN01 and USNOS are WINS work.
- `kPilotFighterGunfireScaleBound`: 0099EFA5-0099F029 as cc13-fix2 read it; USN04 moves. **ON.**
- `kFollowSightSearchBound`: `follow sight searches=0` on USN04, USN01 and JM08. **OFF, unexercised.**
- `kKamikazeBoostTurboBound`: no kamikaze plane on any row (`kamikaze task ordered_planes=0`, no `_kz` unit even on
  the 200000-frame USNOS run, which fails before the Ohka phase). **OFF, unexercised**; R7's turbo arm rides on it.

**ReloadTorpedoes** (`local\p17_r_{off,torp}`, 3250f9d2f): JM08 12000 frames and JM08 `jm08_l65_j8_p2` 72000 frames,
gameplay identical, `reload torpedoes calls=0` (luaJM8Checktorp never sees `torp < 12`; the win-orders run fails at
1157.05 s on both sides, on this base). JM03 600 s: no call. **OFF, unexercised.**

**The catapult adapter** (c246320d6, `kShipCatapultLaunchBound` OFF; `src/ship_catapult_host.cpp`, header for the
labelled substitutions). `local\p17_c_{off,on}`: JM02 (its recon check, jm02:1268-1290, is inside a `--[[` comment in
this installation), JM03 (luaJM3SendPlanes, only once the AI planes are gone) and USN16 3000 frames (the carriers'
launches sit in a later init) make no ShipUseCatapult call; JM03 identical. JM06's `ShipUseCatapult(IJNSubsGrp1[1])`
(jm06:2235, the recon intro) was not reached in 2000 s. **OFF, unexercised**; it needs a row that reaches one of those.

**Open.** 64's touched byte (009B1EF2, land final never sets `+20h`, so the touchdown brake is unreached); the
three unexercised Lua natives; `kFollowSightSearchBound` and `kKamikazeBoostTurboBound` (need a firing follow leader
and a released Ohka); ReloadTorpedoes and the catapult need rows that reach their scripts.

## 68. Handoff (cc9-physics17, 2026-10-17, paused by the user at about 50% context)

**Commits on agent/cc9-physics1** (each passed the full `scripts/build.ps1` with tests): 49575c7db (R7's turbo arm),
057145750 + 3250f9d2f (ReloadTorpedoes: gunnery side and script-orders binding, OFF; GenerateObjectEveryCall ON),
e3da85f78 (four air switches ON, section 66), 334be0e5f (TrulyDead x2, SpawnPlacementMapBounds, IsInBorderZone ON),
c246320d6 (catapult adapter, OFF), 16404484a (band floor and fighter gunfire scale ON, section 67). No run is in flight.

**Queue remaining, in order** (lead routing, 2026-10-17):
1. **cc13 fix12** (main 2f2cdac74, `game_hosts_lua.hpp`): `kLuaIsUnitSelectableBound` (008AB580) and
   `kLuaGetEntitySpeedBound` (008C0820). Flip them TOGETHER: IsUnitSelectable alone makes usn_11's `and` chain hit
   `nil < 30`. Merge origin/main first, because fix18 (ecc9982d9) moved the plane arm onto `plane_world_velocity`.
   Pair USN11 (PRCP11 if cheap). Reached by usn_11/prcp_11 CheckMontR*, commandhelpers :14963 (PT boats), jm12 :702 and
   jm05 :1236. **Caveat (cc13, unreviewed):** usn_11 :1296/:1533 and prcp_11 :1302/:1539 raise on
   `pairs(GetSquadTorpedoes(sq,true))`, because 008BEF80 returns nil. The rest of that function never runs
   (SetDeviceReloadEnabled, SquadronSetTravelAlt, PilotMoveTo/SetShipSpeed, SetRoleAvailable, SetForcedReconLevel, the
   Phase4Movie delay). Judge fix12 by mechanism, not by mission outcome. cc13 MINOR: the image's +B1Ch is
   |plane_world_velocity|, and it differs from the host while a plane is deck-parented.
2. **cc13 fix10** (main 8cf922c82): `kLuaProximityTriggersBound` (AddProximityTrigger 008C9730, RemoveTrigger 008C5FD0,
   the per-step walk 008759B0/00750000). Pair ONLY USN04 and JM13. Prediction for USN04 ON: the convoy passes the split
   navpoint, North03 sets CarrierEvacEnable, Kill runs at North04, the West04 PTs head for the border, and the two
   natives leave the UNIMPLEMENTED census. Do not judge it on usn_03, prcp_06, jm02 or usn_09: the target-destroyed
   observer 0074FFE0 is not modelled, so record any move there as provisional.
3. **cc13 fix14 with the catapult adapter.** `kLuaGetNumCatapultedBound` (00892AA0) answers 0 until
   `kShipCatapultLaunchBound` fills slots. c246320d6 already routes GetNumCatapulted through the adapter's record when
   the adapter switch is ON. Pair and flip them together. Live callers: jm03 :1614 (luaJM3SendPlanes, random >= 80, the
   image stops at 4), jm04 :1011/:1047, jm14 :670, LOMP01 :703. None was reached on JM02, JM03, JM06 or USN16 (67).
4. **cc13 fix16** (main dd4cc6fb6, `game_hosts_script_orders.hpp`). Pair USN12 and JM05 (JM13 if cheap), then flip the
   five as one batch: `kLuaPlaneForceReleaseBound`, `kLuaSquadronForceReleaseBound`, `kLuaPlaneBombFallingTimeBound`,
   `kLuaPlaneReloadBombPlatformsBound` and `kLuaGetPlaneIsReloadingBound`. Send USN12 WINS moves to lua66. Do NOT flip
   `kLuaPilotBombBound`: it asks the wing-0 plane where the image asks the squadron (008A505C); cc13 fix21 fixes it.
   **Release MINORs in our code**, to fix behind a switch if cheap: (a) `release_ordnance_007bbba0` (units.cpp ~3593)
   turns channel C on when it is disabled, where the image skips the raise at +60h == 0 and only does C20h += 1
   (007BBBB0); (b) the attach test has no drop-to-attach gap (006E3FE0 tests an attached round only). Also correct
   docs/ATTACK_COMMANDS.md :89-91 and :307: the squadron 007EFB00 never answers IsKindOf(10h).
5. Unexercised and OFF (67): UntouchableClear, SquadronSetReleaseAlt, SquadronLandAndKill, FollowSightSearch,
   KamikazeBoostTurbo (with R7's turbo arm), ReloadTorpedoes, ShipCatapultLaunch. Each needs a row that reaches it.

**Open item, TouchdownBrake (64, 66).** 009B1EF2-009B1F10 sets the land-final touched byte +20h when
`(plane+72Ch)->vtable[38h]()` is false. The host stands in `plane_control_mode_900 != 7` for that call. The state rule
009B3D0B moves a plane whose +900h is 4 or 5 to land/park first (`land_enter_park_009b21a0`), so land final's on-ground
half, which holds the brake, never runs (`touched=0` on all 13 USN01 landings). Next step: identify the plane's
+72Ch sub-object and its vtable[38h]. If it answers false before +900h leaves 7 (on wheel contact, for example), the
host must take that signal in place of the mode test.

**Tools in `local\`:** `p12_launch.ps1` (one detached run with the reference env), `p11_wait.ps1`, `p17_exports*.ps1`
(pair_export batches), and the `p17_*` export trees and logs that sections 66-67 cite.

## 69. Menu rows and their scripts; pairs for fix10, fix12, fix14, fix16 (cc9-physics18, 2026-10-18)

**The menu ids do not run the files the fix docs cite.** A row's script is the scene's `StageScript`
(`missiontree.lua` sceneFile -> the `.scn` header -> `scripts/missions/<StageScript>.lua`; the run log's
`mission script name:` line shows the same). `local\p18_reach.py <natives...>` maps every id this way and counts
live (uncommented) call sites. In this installation USN04 runs `usn/usn_19_coralus.lua`, USN11 runs
`COTP-USN/PRCPUS/usn_06_guadalcanal_2nd_battle.lua`, USN12 runs `usn_12_augusta`, JM13 runs `COTP-IJN/jm16.lua`
and JM16 runs `PRCPIJN/JM14.lua`. `usn_04_defend_guadalcanal.lua`, `usn_11_protecting_iowa.lua` and `jm13.lua` are
never loaded, so USN04 and JM13 have no `AddProximityTrigger` call (no `008c9730` row in either OFF census) and USN12
has none of fix16's natives. Live callers by row:

| native | rows (call sites) |
| --- | --- |
| AddProximityTrigger | USN11 (8), PRCP05 (3), PRCP08 (3), USN10 (2), PRCP01 (1), JM02 (1), PRCP07 (1) |
| IsUnitSelectable / GetEntitySpeed | PRCP07 (11 / 11), JM05 (4 / 1), PRCP05 (3 / 0), JM01, CHG05 (the harness does not load JM01 or CHG05) |
| GetSquadTorpedoes | PRCP07 (2, `prcp_11_patrol.lua` :1302 / :1539) |
| GetNumCatapulted / ShipUseCatapult | LOMP01 (1 / 0), JM16 (1 / 0), JM03 (1 / 1), USN20 (0 / 9), JM02, JM06 (0 / 1) |
| fix16's five | JM05 (PlaneForceRelease, PlaneReloadBombPlatforms), PRCP08 (SquadronForceRelease), JM01 / JM16 (GetPlaneIsReloading), PRCP07 (PlaneReloadBombPlatforms) |

Pairs are same-commit exports: `local\p18_a_{off,f12,f12t,prox,cat,rel}` at fcb6c6fa5, `local\p18_b_{prox,proxf12}` at
b62233d78 (fix19b inside the proximity switch). Logs are `local\p18<set>_<row>.log` with their `.manifest.json`.

**fix14 + the catapult adapter: ON** (`kShipCatapultLaunchBound`; `kLuaGetNumCatapultedBound` is already implied
by it at `game_hosts_lua.cpp:610/879` and only adds the summary line, so it flips when `game_hosts_lua.hpp` is free).
- LOMP01, 24000 frames: OFF, every `GetNumCatapulted(Mission.Repulse)` returns nothing and `luaCheckReconplanes`
  raises `attempt to compare number with nil` (`01_saving_force_z.lua:704`) inside `luaTimetable`
  (`timetable.lua:32`), 362 times. ON it answers 0 (356 calls), the timetable entry runs on, and gameplay MOVES:
  deaths 47 -> 53, hit records 1546 -> 1715, units 126 -> 132, three more movie camera builds, a `Countdown`, formation
  joins 5 -> 7. That is the script running as written.
- JM16, 24000 frames: OFF 18 calls (each raising), ON 2088; gameplay identical.
- USN20, 24000 frames: the nine `ShipUseCatapult` carriers are refused (stock 0, no catapult class), as the image's
  `008141E0` (`+638h > 0`) refuses them; gameplay identical.
- LABELLED as in the header: the answer is 0 because no ship AI or script launches a catapult plane before the call.

**fix10, `kLuaProximityTriggersBound`: mechanism matches; still OFF** (`game_hosts_lua.hpp` is leased by cc13-fix19c).
- USN11 (36000 frames, fails at 285.60 s on both sides): 3 adds (`ConvoyReachedPathPoint1` on three transports),
  14048 checks, 0 fires, `unresolved=0`; gameplay identical.
- USN10 (fails at 1000.23 s on both sides): 1 add (`ProxiamityFlagshipEscape`, 4000 m), 12000 checks, 0 fires;
  identical.
- PRCP05: the trigger FIRES. `"AirField 01" "ProximityAirfield" -> luaUS09AirfieldDetected()` near mission frame
  4241 completes primary 1 and sets `MissionPhase = 3`; OFF never leaves phase 1 and fails at 1397.74 s. ON then
  reaches `luaUS09EM_Airfield_1` (`usn_09_Leyte.lua:1985`), whose `GenerateObject(Mission.PTTemplate.Name, "Gyoraitei
  PT")` aborts the run near 1062 s: `unit observer creator projection is unavailable: unit=Gyoraitei PT #1.1
  creator=00000000 (no vehicle-class descriptor; a StationaryClass row carries no Type)`. That is a spawn-host gap the
  flip exposes, not a trigger fault. Same on the b62233d78 pair.
- PRCP08 (fails at 378.08 s): no `AddProximityTrigger` call.

**fix12 (`kLuaIsUnitSelectableBound` + `kLuaGetEntitySpeedBound`) and the GetSquadTorpedoes stopgap: OFF,
unexercised.** No reachable row calls them: PRCP07 (36000 frames, phase 4 needs the player), JM05 (fails at
321.05 s), PRCP05 with the proximity switch ON (to the 1062 s abort), and the old rows USN11, JM04, USN09.

**fix16's five + `kLuaPilotBombBound`:** JM05 (36000 frames) and USN12 with `usn12_s38_u12_p2` (34000 frames): gameplay
identical, none of the six natives called. **ON, unexercised**, as the lead routed (fix16 rated them safe; cc13 fix21 fixed PilotBomb's squadron class).

**USN12 WINS note:** on fcb6c6fa5 OFF, `usn12_s38_u12_p2` does not complete in 34000 frames (MissionPhase 3,
Distance 10.12 km); the table's last completion was 1539.83 s. Sent to the WINS keeper.

**fix16's release MINORs: recorded, not fixed (not cheap).**
- (a) `007BBBA0` (disasm-raw): `007BBBA6 CMP byte [EDX+60h],0` / `007BBBB0 JE 007BBC00`, so a disabled channel C
  skips the bay writes and still reaches `007BBC00 ADD [ECX+C20h],EBX` (EBX = 1). The host's two callers
  (`release_ordnance_007bbba0` and `release_bomb_007bbba0`, `game_hosts_units.cpp`) instead turn channel C on first.
  The host also gates the torpedo spawn on the bay's acceptance, where the image's spawn hangs off `+C20h` on
  every path (bay already open included). Removing the force-enable alone would stop releases on any plane whose
  channel C is disabled, so the faithful fix is to key the spawn on the `+C20h` raise for all callers
  (`release_ordnance_007bbba0` has eleven call sites in the shared units file). Left for the plane lane.
- (b) The attach test has no drop-to-attach gap: `006E3FE0` tests an attached round only. Not touched.
- `docs/ATTACK_COMMANDS.md` corrected: `PilotBomb` asks `vtable[5Ch](10h)` of its argument (`008A505C`), not of
  the `+3D0h` object, so a squadron argument never gets `levelbomb`.

**TouchdownBrake (68's open item): the stand-in is the image's call.** `009B1EF2`-`009B1F07`: ECX = the plane
(`[[task+4]+4]`), `ADD ECX,72Ch`, `CALL [[ECX]+38h]`. The plane class stores vtable `00D06130` at `+72Ch`
(`007D03FD MOV [EDI+72Ch],00D06130` in its destructor; the unit base stores `00CFC384` at `006FE4A9`). Slot `38h`
(`00D06168`) is `0074E210` `BSP_PlaneControlMode_IsFreeFlight`: `CMP [ECX+1D4h],7 / SETE AL`, and `+72Ch + 1D4h` is
the plane's `+900h`. So the call answers `+900h == 7`, which is exactly the host's `plane_control_mode_900 != 7`
test for setting the touched byte. The brake stays unreached for the reason 68 gives (`009B3D0B` moves a plane
whose `+900h` is 4 or 5 to land/park first); whether the image ever has `+900h` at another value (6, say; `0074E230`
groups 7, 6, 4 and 5) during land final is the remaining question, and it is a `+900h` writer census, not this call.
  Immediate `+900h` writers (scan-bytes `C7 86/81 00 09 00 00`): 7 at `007C6481` / `007C7183` / `007D6600` (spawn,
  begin flying, property bag), 6 at `007C63F4` (spawn) and `007CB9E0` (session-remote only), 4 at `007C1697`
  (FiveToFour; callers `009B22C0` approach-base, `009CD540` taxi, `009CE2C0` takeoff) and `007C7488` (remote), 5 at
  `007C171E`, 2 at `007CC7E2` and 1 at `007CC857` (both from `007CCFA0` BSP_Plane_HandleMessage). So in single
  player only a message arm (1 or 2) could clear the free-flight test during land final; which message, if any,
  the landing sends is the next step.

**`kLuaSquadronSetReleaseAltBound` (67's unexercised list): exercised, flip pending the `game_hosts_lua.hpp` lease.**
`local\p18_reach.py` finds it live in BSM09 (`bsm_09_battle_of_the_coral_sea.lua:321`, `SquadronSetReleaseAlt(squad,
220)`) and PRCP08 (3 sites). `local\p18_a_relalt` (fcb6c6fa5) against `p18_a_off`:
- BSM09, 24000 frames (fails at 183.06 s on both sides): 5 calls, each `alt=220.0 force=0 stored=1`. The Vals'
  `+39Ch` ceiling now holds 220 instead of 9999, so `009C7A94` caps the dive altitude and the first release comes at
  113.2 m (OFF 229.9 m); dive-bomb-task releases 0 of 7 -> 6 of 13. Deaths 25 both. MOVED, by the mechanism.
- PRCP08 (fails at 378.08 s on both): 1 call (`"Corsair" alt=150`), gameplay identical.
The other two unexercised natives still have no reachable caller: SquadronLandAndKill only in PRCP01
(`usn_03`, failed at 465.71 s with no call), UntouchableClear in no mission script. ReloadTorpedoes is live only in
JM03's script (`PRCPIJN/JM03.lua`), not JM08's.
- ReloadTorpedoes on JM03 for 36000 frames (`local\p18r{off,torp}_jm03`, the 3250f9d2f exports): both fail at
  1176.02 s, no call, identical. Still OFF, unexercised.

**Tool:** `python tools/mission_script_reach.py <Native> [<Native> ...]` lists every menu id whose real script (the
scene's `StageScript`) calls one of the natives, with call-site counts; with no arguments it prints each id's script.

## 70. Proximity and ordnance triggers ON; the SpawnNew missing-class raise; the kill-funnel feed (cc9-physics18, 2026-10-18)

Exports at e62312da7 (main with cc13 fix19c): `local\p18_c_{off,prox,sod}`; logs `local\p18c<set>_<row>.log`.

**`kLuaProximityTriggersBound` ON** (lead routing, after 69's mechanism). The squadron-target rows:
- PRCP01 with `prcp01_l55_p01_c` (30000 frames): both complete at 986.78 s, gameplay identical. The one trigger
  (`"Hornet"`, `FakeDiveBomberReconlistenerID...` -> `luaUS03HornetStrikeIncoming`, 1500 m, target entity 142, a
  squadron) never fires; its target squadron goes and the trigger is retired (fix19c's poll), 57 notices.
- JM02 (36000 frames): the trigger (`"Buffalo 06"` -> `ThisLooksBad`, 1500 m) FIRES once; gameplay identical.

**`kLuaSquadOrdnanceDroppedTriggerBound` ON** (by mechanism).
- USNRM01 (36000 frames): `AddSquadOrdnanceDroppedTrigger(Mission.OklahomaKillers, nil, "luaOklahomaKillersRetreat")`
  (`usn_1_pearl.lua:1507`) fires once: the squadron's fire stance goes to 2 and it moves off toward `JapRetreat`.
  MOVED: the mission fails at 784.93 s (OFF 569.98 s), deaths 343 -> 514.
- BSM04 (fails at 84.05 s on both sides): one add (`"B-17"` -> `luaFortressDroppedBombs`), no fire; identical.

**`kSpawnNewMissingClassRaisesBound` (new, OFF): PRCP05's abort.** The abort's unit is the shipyard PT of
`usn_09_Leyte.lua` (`SpawnNew` with `["Type"] = 240`, `"Gyoraitei PT"`). This installation's
`scripts/datatables/autoload/vehicleclasses.lua` (modified 2026-05-09) has no `VehicleClass[240]` (it jumps from
237 to 241), so the class factory has no row. The image: `00949750` (SpawnNew's body) builds each member through
`00945C30` -> `00944210` -> `00964790`, which takes `VehicleClass[classIndex]` (`00B67720`, a nil object) and then
its `Type` through `00B67800` -> `00A67C20` `lua_gettable` -> `luaV_gettable` on nil, which raises `attempt to index
a nil value`. The error unwinds the native into the script's protected call: the calling function stops and
nothing is spawned (the members are read inside the native, before `009484xx` creates any). The host instead created
the member with no class and aborted at the observer projection. The switch raises the same Lua error at the member
read. Uncertain: the exact message text (no variable name is available to `luaG_typeerror` from a C call), and that
`00964790` makes no earlier call that would fault first; the listing shows none.

**`kKillFunnelTargetGoneBound` (new, OFF, `game_hosts_gunnery.cpp`).** `kill_unit_00926d90` (the host's 00926D90
Kill entry) now feeds `GameMissionLuaHost::target_gone_00925c40` for a unit gunnery does not yet count dead
(00926DED). It covers the Kill callers fix19 lists as unfed (plane depth kill 007CE3A7, landed remainder 00749B7B,
SquadronLandAndKill's planes, the other 00926D90 callers). A gunfire death takes destroy (00926C80) and is not fed,
because the image's destroy notice is the no-op slot `+8h` `0042B120`; its trigger retires when the wreck is Killed.

## 71. Pair results for the SpawnNew raise and the kill-funnel feed (cc9-physics18, 2026-10-18)

Base c925b2319 (prox and SquadOrdnanceDropped ON), PRCP05 (`usn_09_Leyte`), 36000 frames each.

**`kSpawnNewMissingClassRaisesBound` FLIPPED ON.** OFF (`local\p18_d_off`) stops at mission clock 1061.96 s: the
shipyard SpawnNew (`0094c480`, callback `luaUS09JapPTSpawned`) queued "Gyoraitei PT #1.1" with no class, and the
units host failed startup at the observer projection (creator 00000000), exit 1. ON (`local\p18_d_spawn`): the same
call raises once (`spawn missing class raises=1`), `US09Think` fails at script line 1093 with `attempt to index a
nil value`, and the run continues to 36000 frames with exit 0; the mission does not end (`Mission.EndMission never
true`) and has no further script failures. The flip is by mechanism (the image's `luaV_gettable` on the nil row);
PRCP05 still cannot complete on this installation because its PTs never spawn.

**`kKillFunnelTargetGoneBound` FLIPPED ON.** Pairs at c925b2319 (`local\p18_d_off` vs `local\p18_d_feed`), 24000
frames, no helm orders. PRCP01 (`usn_03`): 8 feeds, both fail at 465.71 s (`usn03.obj_p2_fail`), pair_diff exit 1
(gameplay identical). JM02: 0 feeds, no mission end on either, exit 1. The fed units had no live proximity trigger
on these rows, so nothing moved; `target_gone_00925c40` notices each unit once, so a later Lua Kill or pass-K notice
for the same unit adds nothing. Flipped by mechanism (00926D90's kill list notifies 0074FFE0 at pass K).

## 72. Handoff (cc9-physics18, 2026-10-18, at about 70% context)

Landed on agent/cc9-physics1 after c925b2319: 8bdea81c1 (SpawnNew missing-class raise ON), e687bff42 (kill-funnel
target-gone feed ON). All of the lead's six-item packet is done: prox ON, PT spawn gap ON, PRCP01/JM02 prox pairs,
SquadOrdnanceDropped ON, squadron_unit comment, gunnery-funnel feed ON. The PRCP01 prox result went to cc9-lua67
(identical, completes 986.78 s with prcp01_l55_p01_c). Leases: only p18_catapult_flip (OCEAN_WAVE_FIELD.md,
ship_catapult_host.hpp) is still held; release it when you take over.

Open, in the lead's order (all low priority):
1. Pair cc15's `kActuatorFlagCc13BodiesBound` (OFF on main 106f741a0, `game_hosts_units.cpp`) on
   jm08_s45_j8_m10 for 72000 frames before the flip. cc15's manifests are in
   `J:\PROG\battlestations-pacific-decompile-cc15-wire1\local\cc15_act1_*.manifest.json`. A 72000-frame run outlasts
   the 600 s foreground cap: launch, then fill the wait with read-only work.
2. cc15 held item (a), 007D99C0 forward speed. The host inlines dot(v_world, fwd) at units.cpp ~11834/11915. The image
   returns ctl+44h + parent_local_z * (0.1 if z >= 0 else 0.8) when the plane is parented (007D9A31..007D9A5F). Put
   it behind an OFF switch if the listing confirms it, and pair it on a carrier row. Lease units.cpp after cc15
   releases it.
3. Item (b): model 007D80C0's rate at 24-bit precision per op. Item (c): 007D9CE0/007D9C10 are not examined.
4. The release MINORs from section 69. 007BBBA0 (the spawn should key on the +C20h raise for all 11 callers) is not
   cheap.
5. fix12 and the GetSquadTorpedoes stopgap stay OFF as unexercised (section 69).

Run tools: `local\p12_launch.ps1 -Exe <exe> -Name <n> -Mission <id> -Frames <f> [-Orders <abs path>]`;
`local\p18_wait.ps1 -Names a,b -Max 580`; export builds `local\p18_exports*.ps1`. Stagger launches by 60 s, run at
most two, and run one build at a time. `python tools/mission_script_reach.py <Native>` maps menu ids to scripts.
PRCP05 does not complete on this installation: VehicleClass has no row 240, so its PTs never spawn.

## 73. 006BCA80's owner-velocity lead is already bound; per-op rounding (cc9-physics19, 2026-10-18)

cc13's CC13_SEG29U.md item 1 reports 006BCA80 as a live mismatch ("velocity = 0, +8Ch ignored"). It read
`land_path_point_006bca80`'s comment, which described only the t = 0 point. The t != 0 call (009AFD52) adds the
owner's velocity x t in `land_geometry_009afaf0` under `kCarrierLandingVelocityLeadBound`, ON since section 66;
`local\p17xair_usn01w.log` has `summary carrier landing leads=4761 bound=1`. The comment now says so.
- Callers (rel32 + absolute scan of the PE, `local\p19_rel32.py`): 007C7329 (t 0, only `.w`), 009AFB46 (t 0),
  009AFD52. The one absolute hit, 004AAEFC, is inside `CALL 00B67980` at 004AAEFB.
- holder+8Ch: 006BEE40 (the carrier refresh) writes +88h only. With +8Ch = 0, a = 0 x t = 0, UCOMISS sets ZF and
  `TEST AH,44h` leaves PF clear, so the JNP at 006BCAD9 skips the rotation. Dead arm, not modelled.
- TouchdownBrake is not affected: 009B1EF2's touched byte keys on +900h == 7 (section 70), not on the path point.

**`kCarrierLandingLeadFloatOpsBound` (OFF).** 006BCC0A-006BCC51 stores each v[k] x t as a float, then the float
sum p + T[k]; the host rounds a double v x t + T once. Prediction for a USN01 pair: the same landings in the same
order; touchdown times and approach traces move by float noise only, and the chaotic flight afterwards may
move counters. A mechanism check, not a behaviour change.

## 74. 007D99C0's deck term (cc9-physics19, 2026-10-18)

007D99C0 (007D99C0-007D9A69, `__thiscall(ctl)`, RET, result in ST0) returns ctl+44h, plus, when the plane has a
scene parent (`[ctl+8]+3Ch`), a deck term. The listing, with ESP tracked through the two calls:
- 007D99DD-007D9A04: with unit+10Ch clear, 00414DB0 refreshes the pose and 00B63D50 builds the inverse world
  (unit+CCh) into unit+110h; +10Ch = 1.
- 007D9A09-007D9A1E: the parent's vtable[34h](out = base+10h), RET 4: its velocity. The PUSH 0 and PUSH unit+110h
  before it are 0042D0D0's arguments (matrix, normalise 0). 007D9A22-007D9A26: 0042D0D0(ECX = base+8h, EDX = the
  velocity): out[k] = sum_i v[i] x M[4i+k], so out[2] (base+10h) = dot(v_parent, the plane's forward row).
- 007D9A31-007D9A5F: c = 0.1 (00D7A2F0) when z >= 0.0 (00D7A218), else 0.8 (00CE74F8; COMISS/JC, so unordered
  takes 0.8); FLD z, FMUL c, FADD ctl+44h, FSTP.
The host's planes keep deck-relative velocity while parented (007D9CE0's subtraction in
`carrier_deck_reparent_007c71e0`), as ctl+18h does, so ctl+44h is already the relative forward speed.

**`kPlaneForwardSpeedDeckTermBound` (OFF)**, `plane_forward_speed_007d99c0`, at the three host sites that model a
007D99C0 call: the free-flight state's forward speed (007DC878; it feeds 007DA710's floor, the water law and the
pose advance), 0099D99E's measured speed (`kPilotThrottleForwardSpeedBound`) and 007DC878-007DC886's stall ratio.
LABELLED: the parent's vtable[34h] is its `motion.linear_velocity`, as in 007D9CE0's host. Not wired: the callers
007C38FC (bow wave), 007C3CC7, 007D9AB4, 007DB75B/007DBD07 (the core law; whether the host's core law reads the
state speed was not checked), 0099F3EF, 009A176D, 009B0C6D. 007DAABD's floor takes the state speed, so it follows. Prediction: only parented planes move (deck taxi, the take-off
roll, landed planes); off a deck the helper returns ctl+44h. On a carrier moving forward at v, a plane facing the
bow reads +0.1v more forward speed, so the stall ratio and the throttle error shift slightly during the roll.
`summary forward speed deck terms=` counts the parented calls.

**73, result: `kCarrierLandingLeadFloatOpsBound` ON.** USN01 with `usn01_p8_u1_i.txt`, 30000 frames, at 888960641:
`local\p19a_off_usn01.log` against `local\p19a_ops_usn01.log` (manifests beside them). pair_diff 3 (moved), as
predicted: the same 19 touchdowns, the first four (417.87, 502.05, 521.45, 541.44 s) identical, the rest drifting
by 0.15-12 s (KatSBD 706.25 -> 718.10); leads 7287 -> 7571, Enterprise inserts 12 / releases 5 on both; deaths 36
on both, 15 death times shifted; neither run ends the mission in 30000 frames. Mechanism matches; flipped.

**74, result: `kPlaneForwardSpeedDeckTermBound` stays OFF.** USN01 `usn01_p8_u1_i.txt`, 30000 frames:
`local\p19a_off_usn01.log` (888960641) against `local\p19b_fwd_usn01.log` (88698c836 + the flip; both have the lead
rounding OFF). pair_diff 3: `deck terms=16206`; carrier stows 19 -> 0, ground retakeoffs 0 -> 37, refused land/abort
on-ground entries 0 -> 13, plane water contacts 8 -> 16, deaths 36 -> 30. The chain, ConTBD1: touchdown 553.29 s,
land/park taxis toward z 82.6 but with the deck term the throttle's measured speed (0099D99E) reads about 1.7 m/s
high (Enterprise at about 16.7 m/s, x 0.1), the plane creeps to a stop at z 78.2 (v 0.79, commanded 2.23) short of
the lift, the park timer (+28h) runs out, 009B21D0 sets done, 009B37A7 enters land/abort, whose on-ground arm is
unbound (refused), so 0099B107 pushes a re-takeoff and the plane lifts off and ditches (|v| 152.76). OFF, the same
plane reaches z 80.6 and the lift takes it (574.18 s).
- The mechanism itself is confirmed: 00923810(1) is `[unit+3Ch]` (the scene parent; 007C7253-007C7266 compares it
  with the carrier's `[[holder+4]+7Ch]` before re-parenting through vtable[ACh]), so the term does apply on deck.
- Why it stays OFF: the failure passes through an unbound arm (land/abort on the ground), and the image's carriers
  stow their planes. Open before a flip: whether 009B22C0's park drive reaches the throttle through 0099D99E for a
  parented plane in the image (or brakes/drives the wheels directly), and the abort on-ground arm.

**74, rewired (cc9-physics19).** The first wiring fed the deck term into the whole free-flight state speed, so
ctl+7Ch (007D80C0 latches ctl+44h, no term), the pose advance (reads ctl+7Ch) and the water law (ctl+44h) took it
too. Now the state speed is ctl+44h again and the term reaches only 007D9AB4 (the authority, `control_authority`)
and 007DAABD (the floor) inside `control_step_007da710`, plus 0099D99E and 007DC878 as before. The USN01 result
above was measured with the first wiring; the stow failure ran through 0099D99E's throttle, which the rewire keeps.

## 75. cc15's actuator bodies ON; 007D9CE0 / 007D9C10 frames (cc9-physics19, 2026-10-18)

**`kActuatorFlagCc13BodiesBound` ON.** JM08 `jm08_s45_j8_m10.txt`, 72000 frames, at af8206868:
`local\p19c_off_jm08.log` against `local\p19c_act_jm08.log` (manifests beside them). pair_diff 1, gameplay identical:
126 deaths, 70763 shots, the same end (failed at 1107.00 s, "The HQ has been destroyed!", as on main since the
wing-fold flip, WINS notes); only the known ship-AI noise counters and one ship-ai summary line move. Exercised:
`wing fold ... prep_holds=360` on both sides (009CDEED's 007B8D10 read through the cc13 bodies); not exercised:
extends / retracts / elevator waits are 0 on both, so the extend and retract arms have no pair yet.

**007D9CE0 / 007D9C10 (cc15 held item c), read, no switch.** 007D9CE0 (re-parent): with an old parent
(`[unit+3Ch]`), ctl+18h (velocity) and ctl+24h (angular) go through the old parent's world matrix (+CCh, 0042D0D0)
and the old parent's velocity (vtable[34h]) is added, each component FADD/FSTP; with a new parent its velocity is
subtracted and both vectors go through the new parent's inverse world (+110h, built by 00B63D50 when +10Ch is
clear). So a parented plane's ctl+18h/+24h are in the PARENT'S LOCAL frame. 007D9C10 then builds ctl+B0h as the
inverse (0085DEA0) of unit+74h, the plane's local matrix, and ctl+3Ch..44h / +48h..50h are those vectors in the
body frame, then tail-jumps to 007D8020. The host keeps the relative velocity in world axes and forms body
components from the world pose rows: the same value while the carrier holds its heading, but the image's vector
turns with the deck and the host's does not. LABELLED difference; it matters only for a parented plane on a
turning or rolling carrier. Carrying a parent-local velocity would touch every parented reader; not started.

## 76. Section 69's MINOR (a): the bay request no longer force-enables channel C (cc9-physics19, 2026-10-18)

With `kReleaseIssueStageBound` (and its Vals twin) ON, the spawn already keys on unit+C20h: line 3671's raise is
unconditional, and the rack stage 007CE9FD spends it. So the "spawn hangs off the bay acceptance" half of MINOR (a)
is only the OFF stand-in. What is left is the force-enable: both `release_ordnance_007bbba0` and
`release_bomb_007bbba0` turned a disabled channel C on (rate 1.0) before the request, where 007BBBA6-007BBBB0 skips
the bay writes. **`kBayRequestNoForceEnableBound` (OFF)** keeps the channel as it is;
`summary bay request disabled channel C=` counts the requests that met a disabled channel. Prediction: when that
count is 0 the pair is identical; otherwise only the bay animation and whatever reads channel C (its value) move,
and the releases do not, because the stage spends +C20h either way.
- MINOR (b) (the attach test has no drop-to-attach gap) is untouched.
- **Not paired, stays OFF.** The host has no writer that enables channel C: the force-enable in these two
  functions is the only `channel_c.enabled = true` in `src/`, and nothing in `game_hosts_units.cpp` reads channel C.
  In the image the class's init enables it for a plane with a bay-door part (as 007EABC0 enables channel A from
  PartAnims.Wings). So ON, every bay would stay shut, and a pair would be identical because nothing reads the
  channel. Next step before a flip: find channel C's enabling writer in the class setup (the bay-door part's
  PartAnims entry next to 007EABC0) and bind it; then this switch removes only the force for classes without one.
- **Enabler bound under the same switch.** 007EABC0's arm after the wings one (decompile, `0x5f4`): with
  class+5F4h set it writes +60h = 1 (enabled), +61h = 0 (target), +64h = 0 (value), +6Ch = 1/class+5F0h (rate),
  +70h/+74h = class+5ECh/+5E8h. `wing_fold_class_007eabc0` now does that from `PartAnims.BayDoor` (007D3B06 reads it
  into class+5E8h) when the switch is ON, with the wings arm's substitution (the live VehicleClass table at the
  first block step; every plane taken as animated). So ON: a bay-door class opens its bay at the request and a class
  without one keeps channel C shut while +C20h still rises. `bay_door_classes=` in the summary counts the enables.
- **Pair, USN01 `usn01_p8_u1_i.txt`, 14000 frames, at d0f0d7813:** `local\p19e_off_usn01.log` against
  `local\p19e_bay_usn01.log`. pair_diff 1, gameplay identical. Disabled-channel requests 27 -> 106 and
  `Plane::release_spawn_deferred_to_rack` 70 -> 57 (the bay no longer accepts, so the accepted-request record is
  rarer), while the releases themselves do not move (the stage spends +C20h). `bay_door_classes=0`: this
  installation's VehicleClass has `PartAnims.BayDoor` on Avenger (both), Helldiver, Judy, B-17, B-25, B-29 (both),
  D4Y (both), Gekko, Betty (three), Liz, B7A, AD-2 and a few gun rows, none of which flies in USN01 (TBD / SBD).
  So the no-force arm matches and the enabler arm is unexercised: stays OFF until a row with one of those classes
  releases (a late USN row with Avengers or a JM row with Bettys).
- **ESMP08 (TBF Avengers), 12000 frames:** `local\p19e_off_esmp08.log` against `local\p19e_bay_esmp08.log`. pair_diff
  1, identical; `bay_door_classes` 0 -> 72 (the enabler arm runs), no bay request in the window on either side.
  Both arms now matched with identical gameplay: **flipped ON.**

## 77. Handoff (cc9-physics19, 2026-10-18, at about 65% context)

Landed on agent/cc9-physics1 after 4341b23c2: 888960641 / af8206868 (lead per-op rounding, ON), d559bd895
(mission_script_reach long comments), 88698c836 / cec37f14e / 8155ccf58 (007D99C0 deck term, OFF), e6da39cac
(007D80C0 rate model), 44165fce4 (cc15 actuator bodies, ON), 232c7c9cf / 5fc97b0f4 / d0f0d7813 / c2c20c59e /
0b6d6ff54 (bay channel C, ON). No leases held. The lead's queue (items 1-5) is done; what stays open:
1. `kPlaneForwardSpeedDeckTermBound` (section 74), OFF: with it every USN01 carrier stow fails. Read 009B22C0's
   carrier park drive and 0099D300's speed hold for a +900h = 4 plane on a moving deck: does the image's measured
   speed carry the 0.1 x deck term there, and what keeps the plane rolling to the lift? Also land/abort's on-ground
   arm (refused) is where the failure escapes into a re-takeoff.
2. Section 75: a parented plane's velocity is parent-local in the image, world-axis relative in the host.
3. cc15 actuator bodies: the extend / retract arms had no pair (JM08 extends=0).
4. Section 69 MINOR (b) (the attach test's drop-to-attach gap) is untouched; fix12 and the GetSquadTorpedoes
   stopgap stay OFF as unexercised.
5. cc13's CC13_SEG29U.md item 1 (006BCA80 "mismatch") is answered by section 73; tell cc13 through the lead if its
   faithful body lands, so it is not wired twice.
Run tools as section 72 lists; `local\p19_rel32.py <hex>` scans the PE for rel32 calls/jumps and absolute dwords to
an address. Export trees `local\p19_*` are kept (no reparse points; drive J has over 1 TB free).

## 78. The stuck release request; 009C1FD0's release arm; final -> park (cc9-physics20, 2026-10-18)

**The stuck release (USNOS `l65_os_c1`) is faithful.** The divebomb stage line now splits `guard_blocked` by the
first 007CE9FD guard that holds (`guard_by=` app 007CEA02 / +9E0h 007CEA0F / +C3Ah 007CEA1C / +5Dh 007CEA29 /
+900h 007CEA33-4B) and prints the last blocked +900h. Diagnostic run `local\p20_os_diag.log` (30000 fr, reference
env, orders `cc9-lua52\local\l65_os_c1.txt`, working-tree build of this section's first commit): the two rows left
with C20h = 1, C25h = 1 are `Essex_sqn05|.-II` (`guard_by=0/0/97/0/43`, died 425.27 s) and `Intrepid_sqn22|.-II`
(`0/0/109/0/43`, shot down by Cargo7 at 954.64 s, alt 117 m, right after its first bomb). The 43 are the deck ticks
(+900h = 2, before launch); everything after is +C3Ah, which the death-message tail 007D1314 raises
(`kPlaneDeathModesBound`). A dead plane's request is never spent in the image either; no host change.

**009C1FD0's release arm, `kFollowReleaseArmBound` (OFF).** `disasm-raw 009C1FD0`: after 009BFD70 (false ->
009C234E), `CMP byte [unit+C25h],0` (009C1FFD) / `CMP [unit+C20h],0; JLE` (009C2006) take 009C200F-009C2063:
cmd+2D0h = 2, +2BCh = 0.0, +2C4h = 0.0, +2CCh = 1, +2B4h = `[[bot+4]+8]+190h` (the class's TravelSpeed x
NewTravelSpeedMul slot, 007D2406), +2B0h = 0, +2D8h = 1, then `RET 4` before 009BFEE0 / 009BEE30. The host's
plain follow tick ran the law unconditionally (it gated only the trail arm and sight search) and the moveto follow
returned without the stores. True: both seams take the arm. LABELLED: bot+8 is read as the plane's own class.
`summary follow release arm pending= applied=` counts it in both builds.
Prediction: wingmen holding a queued or pending release (dive / level bombers between the request and the issue)
fly level at travel speed instead of station-keeping; the number of such ticks is small (a release spends C20h
within one interval), except for planes whose request is never spent.

**009B3DD9's final -> park edge, `kLandFinalParkEdgeBound` (OFF).** `disasm-raw 009B3DB2`: final (+5F8h) with
mode +448h = 4 calls 009B3370 `BSP_BotStateLandFinal_ParkRule` (`final+20h != 0 && !(plane+72Ch)->vt[38h]()`) and
on true switches to +620h land/park through 009B3680 (009B3DE6-009B3DEF). The host refused it while park was ON.
True: the edge calls `land_enter_park_009b21a0`. LABELLED as before: vt[38h] is the host's +900h == 7. Prediction:
only a touched-down plane whose +900h is neither 4/5 (the earlier park arm) nor 7 reaches it, so few planes.

**007E2090's candidate noise, `kFinderNoiseDrawBound` (OFF).** `disasm-raw 007E2160`: after 007DEEC0's score
(007E2175), `score > 0` (007E2184 FCOMI against FLDZ) and `noise > [00D7A218]` (007E218E COMISS) push the pair
(0.0, noise) and call 00BD2F10 with ECX = 1 (007E21A2-007E21AA), adding the draw to the score (007E21AF). The
host added the midpoint 0.025 as a constant (the only noisy caller is 009AAA80 with 00CE7638 = 0.05). True: a
keyed stream-1 draw (`unit name + "#fnoise"`, the host's 00BD2F10 convention) per positive candidate. It couples
the stream: judge it on per-entity tables, never aggregates.

## 79. The squadron helm moveto (cc9-physics20, 2026-10-18)

BSM09 (`cc9-lua52\local\l67r_b9d1_rt_ref.log`, orders `l67_b9_d1.txt` line 2 `10 moveto Donald -2400 3200
repeat 30`): every helm moveto on Donald logs `player command issued ... current=0 latched=0` and Donald keeps its
scene moveto until Shokaku_sqn04 kills it at 181.71 s. The harness sent the moveto down the ship path
(`issue_player_command`, a command row on the plane's slot), and no plane bot reads that row: the host's plane
tasks are installed at delivery by the binding that issues the order (the fan-out substitution, as for
`PilotMoveTo` 008A4150 and the air-ops order 006CCDA0's kMoveToSelf).

In the image the player's moveto 005F9B20 orders [00E188D8] through 0077D600 with 00E08F68, which is
`kPilotOrderClassMoveTo`; on a squadron the command lands on the squadron's director, and each member's bot sees
the change (0099A4C0) and 0099A170 builds the kind-7 moveto task on the position descriptor (0099A23A arm).

`kHelmPlaneMovetoBound` (OFF, `game_hosts_mission_frame.cpp`): a helm moveto on a squadron member goes to
`GameScriptOrdersHost::player_plane_moveto`, which issues 00E08F68 with a position descriptor (kind 0, +1h set,
the point at the leader's height) on every live member and runs 0099A170 at delivery with the air-ops path's
stores (class, range 0, no target, the point). LABELLED: no 00645060 select and no 0064B870 / 00780214 role
transfer run; the line stands for a player who selected the squadron. Prediction (BSM09 d1): Donald turns NW
towards (-2400, 3200) from 0.5 s, so the fighters that picked it at 118 / 134.6 s meet it later or not at all.

**78, results (USN01 `usn01_p8_u1_i.txt`, 30000 frames, reference env, exports of 2f9d20fdb).** OFF
`local\p20a_off_usn01.log`; manifests beside each log.
- `kFollowReleaseArmBound` (`local\p20a_rel_usn01.log`): pair_diff 3 (moved). `pending=25 applied=0` OFF,
  `pending=28 applied=28` ON. Deaths 36 / 36 with 25 rows moved; the first is ScoutDauntless|.-II (159.76 ->
  164.01 s); stows 19 -> 17: ConTBD2|.-II and ConTBD3|.-II (torpedo wingmen, the torpedo follow seam) are shot down
  (402.23 s, alt 566, Convoy2) instead of stowing. The mechanism is the bytes' arm and it is exercised: **ON**.
- `kLandFinalParkEdgeBound` (`local\p20a_fp_usn01.log`): pair_diff 1, gameplay identical, the edge is never
  reached (no final plane is touched with +900h outside 4, 5 and 7). Stays OFF, unexercised.
- `kLandAbortGroundArmBound` (`local\p20a_ag_usn01.log`): pair_diff 1, identical; with the deck term OFF no plane
  enters land/abort on the ground. Paired again with the deck term below.

**79, result: `kHelmPlaneMovetoBound` ON.** BSM09, orders `cc9-lua52\local\l67_b9_d1.txt`, 36000 frames, reference
env. OFF `local\p20a_off_bsm09d1.log` (export of 2f9d20fdb, which predates the switch and runs the same ship path),
ON `local\p20b_helm_bsm09d1.log` (export of 57a15d07b with the flip). OFF: Game Over at 183.06 s (Donald killed),
as the reference. ON: every `moveto Donald` reaches the squadron (`to 1 squadron plane(s)`), Donald is not in the
death rows, and the mission completes (`luaMissionComplete` 411.97 s, `Scoring_SetMissionCompleted` completed=1;
EndMission 408.02 s). pair_diff 3: deaths 29 -> 66, units 66 -> 90 (the later waves spawn). The run stops 40
frames after EndMission, before `luaMissionCompletedNew`, so the exec guard's refusal is not in this log.

**78, the deck term on top (item 3).** Export of 2f9d20fdb with `kPlaneForwardSpeedDeckTermBound`,
`kLandAbortGroundArmBound` and `kLandFinalParkEdgeBound` all true (`local\p20c_dtagfp_usn01.log`, USN01 as above)
against `local\p20a_off_usn01.log`: pair_diff 3; stows 19 -> 1, ground re-takeoffs 0 -> 26 (physics19's deck term
alone: 24), land/park entries 12, deaths 36 -> 27. The abort on-ground arm now runs (`on_ground_arm` 009B0E74
calls=26), but each run is the abort entry tick itself: ScoutDauntless parks at 417.92 s, its park timer expires
short of the lift, 009B21D0 sets done, 009B37A7 enters land/abort at 436.02 s, and 0099B0BE pushes the re-takeoff on
that same tick (`head=land state=4 landed=1`); the plane is airborne (height 21.6) when the arm's +21h sends it back
to park 9.7 s later. The stows do not hold, so **`kPlaneForwardSpeedDeckTermBound` stays OFF**. The open question
is still physics19's: what keeps the image's plane rolling to the lift with the 0.1 x deck term in 0099D99E's
measured speed (009B22C0's tail writes +2B4h, +2B0h = 1, +2D8h = 1, so the park drive does go through the speed
hold). **`kLandAbortGroundArmBound` ON**: it is the bytes (009B0E74-009B0F93, as the switch comment lists), the
USN01 pair with the deck term OFF is identical, and the arm is reached only on a failed park.
`kLandFinalParkEdgeBound` stays OFF, unexercised.

**78, result: `kFinderNoiseDrawBound` ON.** Export of 57a15d07b with the flip against the 2f9d20fdb OFF export (the
same code path with the switch OFF). BSM09 `l67_b9_d1.txt` 36000 fr (`local\p20b_fnoise_bsm09d1.log` vs
`local\p20a_off_bsm09d1.log`): pair_diff 1, gameplay identical, per-entity tables identical (the only summary
change is the known ship-avoidance refill noise). USN04, no orders, 12000 fr (`local\p20b_fnoise_usn04.log` vs
`local\p20a_off_usn04.log`; 26 fighters log early edges 009AAA80): pair_diff 0, identical. The draw is keyed per
unit, so it couples no other stream; no pick changed on either row. Flipped as the bytes' draw; UNCERTAINTY: no
counter shows how often the draw ran.

**78, a further 0099D300 read for the deck question.** `disasm-raw 0099D980`: after 0099D99E's 007D99C0 and the
FDIV by the speed scale (0099D9A3), a plane in +900h 4 or 5 on a holder (+BF8h, +BF4h) whose 006049F0 owner is a
ship (`vt[5Ch](6)`, 0099DA05) and whose ship speed 0092D730 ([ship+1018h]) is NEGATIVE (FLDZ / FCOMI / JBE at
0099DA29-0099DA2F) gets `measured -= ship speed` (0099DA31-0099DA48). The host does not model this arm (a carrier
going astern only). It also shows the image keeps the forward deck term in the measured speed on purpose, so the
answer to the stow question lies elsewhere (the parented ctl+44h, section 75's frame question, or 009B21D0's
timer), not in 0099D300.
Eliminated as well: 009B260C-009B2631's clock (+28h runs down while vt[38h] < 1.0 and dz < 10) reads vt[38h] =
|ctl+18h| (007B8E60), the parent-local relative speed in the image too (section 75), so a world-speed reading does
not explain the image's stows. With the deck term the throttle holds measured = ParkVelocity, i.e. a relative speed
of ParkVelocity - 0.1 x deck speed (about 0.56 m/s at 16.7 m/s), under the clock's 1.0, so the clock drains. Next
to check: the plane's heading on the deck during the park (z < 0 takes 0.8 with the opposite sign) and the actual
value of ParkVelocity (Pilot/Landing, tuning +4E8h) the host reads against this installation's Lua.
ParkVelocity is `KTS(40)` in this installation's `scripts\datatables\planeglobals.lua:632` (about 20.6 m/s; its
Hungarian comment: the plane rolls to its place at this speed after landing, and if it is too large the plane
cannot stop on the carrier's lift). physics19's commanded 2.23 m/s is therefore 009B2C1E's +2B4h = interp x speed
with the heading-error interp near its low end (00D7A2F0 = 0.1): the creeping plane is still turning onto the
lift line. The heading error at the stop is the next thing to log.

## 80. Dogfight avoid timer, early-edge clear, land minimum speed (cc9-physics20, 2026-10-18)

The lead's queue from cc13 seg67 (UNREVIEWED notes, checked here against the bytes).

**009A7A50 avoid timer, `kDogfightAvoidTimerDrawBound` (OFF).** `disasm-raw 009A7D40`: on the latched arm
(009A7D2E, +24h set), `MOVSS [esi+18h]; COMISS [00CE3958] (2.0); JBE` (009A7D52-009A7D5E), then the pair
(1.0 FLD1, 2.0) and `CALL 00BD2F10` (009A7D72), `FSTP [esi+18h]`. ECX is 1 (the byte the latch stores from CL at
009A7D0F). The host wrote the midpoint 1.5. True: a keyed stream-1 draw (`unit name + "#dfat"`, the host's
00BD2F10 convention). `summary dogfight avoid timer redraws=` counts the arm in both builds.

**009AAA80 early-edge clear, `kDogfightEarlyEdgeClearBound` (OFF).** Both gate failures (009AAA9C `+4D4h > 0`
JBE, 009AAABB `1.0 > +4D8h` JBE; NaN fails both) go to 009AAC5D: `MOV ECX,[task+3FCh]; CALL 007B9740`, then
`XOR AL,AL`. 007B9740: `ECX = [ECX+C50h]`, null returns, else tail-JMP 007E2070, which takes `esi = nb+9Ch`,
and when `[esi+14h]` (nb+B0h, the target cache 007E2090 returns while +84h <= 2.0) is set calls 006952A0 (the
observer unregister) and zeroes it. The two later false returns (009AAB1F, 009AAC31) go to 009AAC68 and skip the
clear, as the host's do. True: `nb_found_plus_one = 0` on the gate failure. LABELLED: +C50h is taken to exist.

**009B1300 land minimum speed, `kLandMinSpeedOwnerVelocityBound` (OFF).** `disasm-raw 009B1300`: with
approach+30h set (009B1309), the plane's vt[34h] velocity is normalised by 0042B2F0's length (each component
FDIV'd and stored as a float, 009B1331-009B1359), approach+30h's vt[34h] (009B1362) gives w, and
`(w.y ny + w.x nx) + w.z nz` (009B1364-009B137C) is added to 007C47F0 (009B138E) before the max with 007C4810
(009B13A9-009B13C0). The host had no such term anywhere (`kCarrierLandingVelocityLeadBound`'s sites are
009AFC73-009AFD1A, the lead point, not this). True: the land target (`land_site_plus_one`) linear velocity enters.
An airfield's is zero, so only carrier recoveries move. Prediction (USN01): the final approach's commanded speed
rises by about the carrier's speed (16-17 m/s) for planes heading with the carrier, touchdowns shift.

**80, results.** Variants built in this tree at 236355ef9 / dea03afaf / 967fa63ea (`local\p20_variant.ps1`: the
switch flipped, `scripts/build.ps1`, exe copied to `local\p20d_<tag>`, `variant.txt` beside it; the OFF
code paths of the later commits are the 236355ef9 ones). OFF exe `local\p20d_off`. Manifests beside each log.
- **`kDogfightAvoidTimerDrawBound` ON.** USN04 12000 fr (`local\p20d_avt_usn04.log` vs `p20d_off_usn04.log`) and
  JM02 24000 fr (`p20d_avt_jm02.log` vs `p20d_off_jm02.log`): gameplay identical, `redraws=0` on both rows
  (3 / 5 fighters latch, never with the timer above 2.0; AvoidTime is {2, 3} in planeglobals.lua:418). The bytes'
  draw on a keyed stream; flipped, unexercised.
- **`kDogfightEarlyEdgeClearBound` ON.** USN04 (`p20d_ec_usn04.log`): pair_diff 3, `early edge clears=210`. Deaths
  54 -> 53, 44 death rows moved from 90.80 s (A6M Zero #1.2|.-II), shots 13554 -> 9252, units 120 -> 81 (fewer
  later spawns in 12000 frames). The fighters re-search instead of re-using a cached target after a failed gate.
  Mechanism exercised; flipped.
- **`kLandMinSpeedOwnerVelocityBound` ON.** USN01 p8_u1_i 30000 fr (`p20d_lms_usn01.log` vs `p20d_off_usn01.log`):
  pair_diff 3, `owner terms=2332`; stows 17 -> 16, deaths 36 / 36 with 17 rows moved. ConTBD1 now flies land/final at
  spd_cmd 51.3 m/s (the carrier's speed added; OFF about 35) and descends at about 11 m/s at pitch -0.22, and dies
  at 609.82 s, alt 23, no damage, in land/final, where OFF it stowed (620.67 s). The term is the bytes and is
  exercised; flipped. FLAG for the stow question: the final descent law takes world pitch while the closure on the
  deck is speed - carrier speed, so a faster air speed steepens the path relative to the deck; check what in
  009B1ED0 / 009B1F2C accounts for the moving deck before blaming this term.
- **`kGunfireExemptWandererMulBound` ON.** JM02 24000 fr (`p20d_gx_jm02.log` vs `p20d_off_jm02.log`): pair_diff
  MOVED; the exempt arm runs about 2000 times (exempt 2321 OFF / 1993 ON), and each now writes WandererMul
  (+840h) = PilotAvoidance.GunfireBomberVsGunfire as the bytes do. Deaths 44 -> 39, shots 64515 -> 58299,
  units 48 -> 54, PlayerBetty survives (dies OFF); the ON run reaches the scripted "did not find Force Z" fail at
  740.19 s (OFF: EndMission never true in 24000 fr). Flipped by mechanism (faithful store, exercised). JM02 is not
  a WINS row.
- **`kFollowExitClearsBound` ON.** JM02 (`p20d_fx_jm02.log`): gameplay identical, `follow exit 009BDE40 calls=0`;
  USN01 identical with `exits=0` earlier. Only LB prepare's exit (009B6660 JMP 009BDE40) and the moveto follow are
  wired, as in vtable 00D20AB8. Flipped, unexercised.
- **`kStrafeHorizontalFloatOpsBound` (OFF, added).** 009CAD00 rounds dx/dz to float (009CAD30 / 009CAD3E) and stores
  d2 = (dx dx + 0 0) + dz dz as a float (009CAD58) before the 00CE3820 test and sqrt 00BF7030; the host took dx/dz
  as doubles. Not paired yet.

**80, item 7: the deck-term stow question, narrowed (read-only).** 007D99C0 re-read: the parent's vt[34h](out)
(RET 4; the PUSH 0 / PUSH plane+110h it leaves are 0042D0D0's stack arguments), then 0042D0D0 (ECX = local, EDX =
the velocity, matrix +110h, flag 0) is v times the matrix's column; the COMISS / FLD read the output's z
([ESP+18h] with ESI/EDI pushed = [ESP+10h] after the pops). +110h is 00B63D50's inverse of +CCh, and 00414DB0 builds
+CCh = local +74h times the parent's +CCh, the WORLD matrix. So the term is the parent's world velocity along the
plane's world forward, times 0.1, as the host has it; the frame is not the answer. Section 79's heading-interp
guess is wrong too: `p20c_dtagfp_usn01.log`'s park traces for ScoutDauntless show err 0.013 at the stop. What
stops it is 009B2923-009B298D's cap c = max(0.5, dz x 0.5) on the commanded speed: the hold drives measured =
relative + 0.1 x 16.7 = relative + 1.67 to c, so the relative speed reaches zero at dz = 2 x 1.67 = 3.3 m
(trace: dz 3.9, v 0.45, spd 1.94 at 435.92 s), while 006CFF70's intake wants the nose within 3.0 - v of the lift
(about 2.5 m). With an exact hold the bytes stow only for a deck slower than about 12.5 m/s (2.5 / 2 / 0.1). So
the open question is the deck speed during recovery (the host's carrier runs 16.7 m/s on USN01, a ships-lane
input) or a ground speed hold that does not settle exactly (idle thrust / wheel brake under +2B0h = 1,
+2D8h = 1). The switch stays OFF.
- **`kStrafeHorizontalFloatOpsBound` ON.** JM08 `jm08_s45_j8_m10.txt` 24000 fr (`local\p20e_sh_jm08.log` vs
  `local\p20e_off_jm08.log`, exes built at de14db729): gameplay identical, all tables identical; the strafe task is
  exercised (42 planes, aim 1840 ticks, 47 aims, 729 gun-point fires), so no decision crossed a float/double
  rounding edge. Flipped as the bytes.
- **Item 8 remainder.** 0099B940: the image skips a pair on an unordered compare (FCOMI / JBE at 0099B977,
  FCOMIP / JA at 0099B97F); the host's `v <= lo || hi <= v` took a NaN pair as a hit. Rewritten as
  `!(v > lo) || !(hi > v)` without a switch: identical for every ordered value; no run was checked for a NaN band (unpaired). 009A17D0
  zeroes the whole +26Ch dword (009A17E9); the host carries only bit 4 (`ga_flag_26c_4`), and no other bit is
  modelled or read, so nothing to bind.

**80, the early-edge USN04 move per entity (for lua68's re-time).** `units` in pair_diff is `summary mission
world units=`, the units-host slot count at the end, so it counts every unit ever spawned. ON (`p20d_ec_usn04.log`)
lacks 39 rows, all of phase 2: Zuiho-class01 (Shoho), Takao-class01, Mogami-class01, moviefisher, and their and
the US strike squadrons (Zuiho_sqn05/08/10/12/14, Yorktown_sqn06/07/09/11/13). Deaths are the same set except
Yorktown-class01_sqn04|.-II, which dies at 194.61 s OFF and survives ON. Mechanism, in usn_19_coralus.lua:569-586:
when the IJN bomber list is empty, the per-tick check calls `luaObj_Completed("primary",1)` and
`Blackout(true, "luaMoveToPh2", 3)` on every pass while the condition holds. ON, the faster fighters clear the
bombers by 195.01 s, so the check re-arms Blackout every 2.5 s: 135 arms, 141 Blackout calls against 35 OFF. Each
arm resets remaining to 3.0 (005B9BA0, `src/mission_blackout.cpp`), so luaMoveToPh2 never runs. OFF clears them at
344.99 s; the next 80 s bomber wave refills the list, the re-arms stop, and GenerateObject("Zuiho-class01") runs at
about 425 s. Planes that stopped firing are the phase-2 squadrons that never spawned (shots 13554 -> 9252). In
phase 1 the US CAP fires less and the ships fire more: Lexington_sqn01|.-II 1153 -> 250, Yorktown_sqn04|.-II
828 -> 0 (it never engages), Yorktown_sqn04|.-III 1045 -> 220, the A6M escorts about 30-40 % fewer; Northampton-class03
310 -> 804, Lexington 693 -> 865. The stall is a script knife edge on the faithful Blackout re-arm.

## 81. Bot-task constructor seeds; carrier speed in recovery (cc9-physics20, 2026-10-18)

**`kBotTaskCtorSeedsBound` ON.** The task base constructors 0099C6F0 / 0099C8B0 (every bot-task constructor
calls one) seed task+308h = -U(0, 1) on stream 1 (0099C765, FCHS, FSTP 0099C76C), +3E8h = U(-1, 0) on stream 0
(0099C7BA, FSTP 0099C7BF), and +3ECh = -1.0 (0099C7D3). The follow state 009C2980 seeds +74h = -U(0, 0.6), the
dogfight maneuver 009A84E0 seeds +2Ch = -U(0, 1) (009A853A), and the torpedo attackrun 009D06D0 seeds
+1Ch = -U(0, 1). Before this switch the host kept per-unit values across tasks: the gunfire timers 0 / 0, the
follow +74h 0, and the maneuver and torpedo countdowns at the midpoint -0.5. True: each host task install draws
them from keyed streams (`#tb308`, `#tb3e8`, `#tb74`, `#tbmv2c`, `#tbt1c`). Land and retreat reuse their existing
`#l74` / `#rf` follow draws. Without BSP_GUNNERY_RNG_STREAMS the draws share the host's one stream. That is not the
image's stream 0 / 1 order, which would need the image's global draw sequence. Not hosted, recorded:
- task+308h is seeded but not read. 009998A0's accumulator (00999912-009999BB) runs 0099B740 only when
  dt >= +308h, then adds +304h - dt. The host runs 0099B740 on every think.
- The depth-charge attackrun 009A36A0 +1Ch has no host countdown field.
- The dogfight attackrun 009A70E0 +1Ch is not reached, because the mode stays 1.

Pairs, exes built at c137d931e (`local\p20f_off`, `local\p20f_seed`):
- JM02, 24000 fr (`p20f_seed_jm02.log` vs `p20f_off_jm02.log`): pair_diff MOVED with 48 seeds. Deaths 39 / 39,
  with 3 rows swapped. Hits 2842 -> 3251, shots 56796 -> 60458. The scripted Force Z fail moves 741.19 -> 829.17 s.
- USN01 `usn01_p8_u1_i.txt`, 30000 fr (`p20f_seed_usn01.log` vs `p20f_off_usn01.log`): MOVED with 77 seeds. Deaths
  36 / 36 with 22 rows shifted by about 0.1 s. The hangar / stow log lines go from 18 to 21.

Flipped by mechanism: these are the bytes' values, and they are exercised on both rows.

**Carrier speed during recovery (read-only).** USN01's Enterprise runs at the scene's reference speed: the ship
pre-step input logs `reference_speed=16.7194405` (log line 1935). The ship AI is in `movetopos / navigate` at
throttle 1.0 through the park (ship ai step 8610-8630; d32c falls 8.31 m per 0.5 s). usn_1_marshall.lua sets no
speed for the CV group (lines 304-320). In the image, the bounded search found nothing that ties air ops to a ship's
speed: no tuning string (the "wind" / "Recover" / "Landing" strings are landing-craft and weather keys), and the
callers of 006CE610 / 006C0840 / 006C0750 are all plane or air-ops code. So no image cap under 12.5 m/s was found,
and the deck term stays OFF. Still open: whether the image's ground speed hold under +2B0h = 1 / +2D8h = 1 settles
exactly (the throttle and wheel-brake law of a parented plane), and the ConTBD1 final at 51 m/s.

## 82. 0099B740 cadence; HANDOFF (cc9-physics20, 2026-10-18)

**`kTaskAttackModeCadenceBound`** (c38b38b13 added it OFF; the flip commit follows this section). 009998A0's
slow path runs 0099B740 only when dt >= task+308h (00999912-00999926 FCOMI / JC). On that pass
+308h = (+304h - dt) + +308h, using 0099992C FSUBR and 00999934 FADDP (+304h = 1.0, from 0099C75D). On every other
pass +308h -= dt (009999B5-009999BB). The task arm 00999950-0099995A runs either way. True: all seven host
0099B740 sites take the gate: dive `run_dive_bomb_attack_mode_tick_0099b740`, level bomb `lb_cruise_profile_009b8c90`,
strafe, depth charge, kamikaze / drop kamikaze `kz_attack_mode_0099b740`, and torpedo `run_attack_mode_tick_0099b740`.
On a skipped pass the squadron +370h keeps its value. The gate reads the `task_phase_308` that
`kBotTaskCtorSeedsBound` seeds (section 81). Pairs, exes built at c38b38b13 (`local\p20g_off`, `local\p20g_cad`):
- JM02, 24000 fr: identical, fires 0 / skips 0. JM02's tasks are all dogfights, and no dogfight arm calls 0099B740.
- USN01 `usn01_p8_u1_i.txt`, 30000 fr (`p20g_cad_usn01.log` vs `p20g_off_usn01.log`): gameplay identical, with
  fires 2028 and skips 17647. 0099B740 is an assignment of 1 for a flight leader, so a slower cadence only changes
  how long a Lua-raised mode 2 survives, and no USN01 order raises it.
Flipped by mechanism: exercised, and it is the bytes.

**The parked-plane speed hold (the stow question, unfinished).** The host's park speed law is in
`run_land_park_tick_009b22c0`, with traces `land park trace`. The throttle law (game_hosts_units.cpp ~36740)
integrates the demand from desired - measured, using 007D99C0's measured speed. With the deck term,
measured = relative + 1.67, so the integrator drives the throttle to zero once 009B2923's cap
c = max(0.5, dz / 2) falls below 1.67 (dz < 3.3 m). The plane then coasts on ground friction. In
`p20c_dtagfp_usn01.log`, ScoutDauntless decelerates at about 0.13 m/s^2 from dz 5.5 m / 0.98 m/s, covers 1.6 m in 4 s,
and stops at dz 3.9 m. 006CFF70 wants the nose within 3.0 - v of the lift (about 2.5 m). Coasting the rest would need
only about 0.88 m/s at dz 5.5, so the answer turns on the ground deceleration: wheel brake (classDesc+1E0h WheelBrake,
read at 007D22A6, `cc9_plane_ground_roll`) and rolling friction, under +2D8h = 1. Next steps:
- Read 007D22A6's brake condition. Does it brake when measured > desired with +2D8h = 1, or only on +2D8h = 0 / an
  explicit brake?
- Compare it with the host's `last_brake` / `last_friction` in the ground summary line (~41262).
- If the image does not brake there, a host brake under +2D8h = 1 is the stow blocker. Put that fix behind an OFF
  switch and pair it with `kPlaneForwardSpeedDeckTermBound` on USN01.
- The ConTBD1 crash short of the deck at 51 m/s (section 80, `kLandMinSpeedOwnerVelocityBound`) is still unread.
  Check 009B1ED0 / 009B1F2C's descent law against the moving deck.

**Open items for the successor.**
1. The stow question above.
2. ConTBD1's final at 51 m/s.
3. The USN04 Blackout knife edge is with lua68 (does the Lua native refuse a re-arm while active?).
4. 0099C8B0's ConstructFromStream callers are not hosted (save / stream path).

Leases: `p20_doc2` (this doc) is released with this handoff. The flip commit takes units.cpp for one commit.


## 83. The stow with the deck term, ConTBD1's final, the final -> park edge, stream order (cc9-physics21, 2026-10-18)

**(1) The stow with `kPlaneForwardSpeedDeckTermBound` ON: the image brakes as the host does.** 007DBEB3-007DBF0C
(read again from the disk bytes): brake = classDesc+1E0h WheelBrake (80) x max(latched air brake [F+3Ch]+10h,
0.6 (00CEFF98) - 26.0 (00D06880) x latched throttle [F+24h], the 007DB755 copy of unit+BBCh). The air brake is
0099D300's negative demand (0099DC01). `ground_band_007dbeb3` and `pilot_plan_throttle_0099d300` already carry
both, so there is no host brake fix to make. The 0.13 m/s^2 of section 82 is an artifact of trace lines 2 s
apart. The diagnostic `kParkStowTrace` (a2690b07a, 2c5fa3ef8, OFF) logs every second park tick within 8 m of
the target. The run `local\p21b_deck_usn01.log` (deck term ON, USN01 `usn01_p8_u1_i.txt`, 30000 fr; 1 stow, as
in section 78) shows a roll that alternates between thrust and the brake. Each time the quantized throttle (007BB6E0,
steps of 1/127) reaches 2/127 or less, the brake (15.2 at 2/127, 48 at 0) stops the plane within one step.
The blocker is 0099D300's dead band, 0099DB1F-0099DB56 (00D09450 = 0.8333). The host matches it, and it uses the
measured speed after 0099D99E's 007D99C0. A stopped parented plane measures the deck term alone, 0.1 x 16.7 = 1.67.
That is at least 1.0. For dz < 5 m the desired speed max(0.5, dz / 2) (009B2923-009B298D, 00D7A280 = 0.5) lies
within 0.83 of it, and the ratio stays inside [0.5, 1.5]. So the increment is skipped and the throttle freezes. Two
examples:
- ConTBD3, 512.15 s: f18 3.23, throttle 0.0157, brake 15.24, meas 1.672, spd 1.615. 009B260C's 3 s clock then
  ends the park (land/abort at 512.45 s, re-takeoff).
- Even without the freeze, the converged relative speed is dz / 2 - 0.1 x V_deck. That reaches zero at
  dz = 0.2 x V_deck (3.34 m at 16.7 m/s), short of 006CFF70's 3.0 - |v|.
So with these laws, which are image-exact as far as I have read them, a carrier at 16.7 m/s cannot stow by convergence. A stow
needs V_deck under about 12.5 m/s, or an arm not yet read. The open question is the carrier's speed during recovery
(section 81), not the brake. The deck term stays OFF.
Side finding, recorded and not changed: a stopped plane keeps body vy = -0.736. That is one step of the 1.5 g
gravity, 14.715 x 0.05. 007D87F6-007D8838 zeroes vy only with dyn+A4h set or dyn+94h > 0, and both are clear at
rest. |v| then cuts 006CFF70's margin by 0.74 m.

**(2) ConTBD1's final at 51 m/s.** The final speed law 009B1F2C-009B1FDF (read from the bytes) is
+2B4h = r + a+4Ch x (s - r), with s = 00419010(a+24h x ApproachDist (+4ECh), r, a+24h x PosBehind (+4F0h) x 0.4
(00CE65D0), (r + a+5Ch) x 0.5, A8). `run_land_final_tick_009b1ed0` matches it. Nothing in 009B1ED0 or 009B1420
refers to the deck's motion except r itself (009B1300's owner term). The glide geometry Y/Z is deck-relative. The
pitch arm's `far_out = Z > 3 x |v|` uses vt[38h], the plane's world speed. In `local\p20d_lms_usn01.log` ConTBD1
enters the fixed approach pitch (+0.1047, mode 1) at Z 111-150 m, against 90-105 m OFF. It crosses the runway
start at local y 3.58 (OFF 1.22). It floats at 36 m/s with aoa 0.22: touchdown summary contact_steps=18,
min_height 0.05, low_steps 0. WheelHeight is 0.00, because this class authors none (vehicleclasses.lua, mtime
2026-05-09). The crash sweep 007C0910 -> 0084BF00 then takes it on Enterprise at 609.82 s. It is the bytes' law
given the faster r. The 604.43 s event (throttle 0.008, air brake 1.0 while spd_cmd 51.45 > spd 49.4, as the
direction hold starts) is unexplained. A first candidate is 0099DAC8's low-pass correction. Unchecked.

**(3) `kLandFinalParkEdgeBound`: no row reaches 009B3DD9.** The edge needs land/final, mode 4, +20h touched, and
+900h not 7 and not 4/5 (the earlier park arm), so +900h must be 1, 2 or 6. That rules out three paths:
- State 6 from water contact (007CB92C) skips the kill only when MinWaterSpd is 0 and up_y > 0.8. A plane with
  MinWaterSpd 0 cannot touch down on a deck (007C71E6 needs classDesc+198h != 0).
- Powerlost deaths gate the bot think (0099ACE1).
- 1 and 2 are the hangar and placement states.
A scan of the summary tails of 1315 cc9 logs from the last two days found 7222 `summary landing plane` records (the
pattern matches with 0) and no `park_refused` above 0 (`local\p21_scan_park_refused.py`). It stays OFF, unexercised.

**(4) Stream 0/1 order for `kBotTaskCtorSeedsBound` with the streams env off: not cheap, not implemented.** The
host has no image 00BD2F10 generator with streams. `release_altitude_draw_00bd2f10` is a host LCG (one shared state,
or one per key with BSP_GUNNERY_RNG_STREAMS), and each subsystem draws from its own helper. The image order would
need every 00BD2F10 caller in the host routed through one image-faithful generator, called in the image's global
order.

Leases: `p21_stow` (src/game_hosts_units.cpp) and `p21_doc` (this doc) are released with this section.

## 84. The carrier's speed in the image (read-only); ConTBD1's 604.43 s throttle cut (cc9-physics21, 2026-10-18)

**(1b) Enterprise's commanded speed on USN01.** No image cap was found under 16.72 m/s:
- MaxSpeed is 16.71944 for VehicleClass[2] "Yorktown 1944" in this installation's vehicleclasses.lua, which is
  32.5 kn in m/s. 0080FC30 = unit+9C0h x the category-4 modifier, so the reference speed is the class maximum.
- The leader cap 009F4DA0 is min(0070D140, 0070D0F0) / 0080FC30. Each member's 0070D100 publish (009DF6AA) is its
  own reference x 1.25 / max(blend, 0.25), so the cap is 1.0 unless a turn blend goes above 1.25. CVGroup is
  Enterprise + Ralph, McCall and Blue (usn_1_marshall.lua 304-334, NavigatorMoveToRange to NTRNav).
- 009E1170's first arm stores -1.0 into *(unit+73Ch)+28h for an AI-held ship, so it has no Cruise speed.
- MotherShip_Update 00758240 -> 006BA670 -> the launch site's vtable+08 (vtable 00CF8A58) is 006CFE40
  `BSP_AirOpsElevatorSite_PlatformTick`, which only steps the lift (006FC480).
- brain+AF0h is written only by the land / kamikaze / engage / lead-pursuit / tangent states.
- The host matches: throttle 1.000 on movetopos / navigate, d32c -8.32 m per 0.5 s.
Open: the image's terminal speed at throttle 1.0, and the host's Enterprise holding heading 0.0000 against target
-2.5885 with rudder 0.004 throughout (ships lane).

**(2b) ConTBD1's throttle cut at 604.43 s (kLandMinSpeedOwnerVelocityBound ON, dea03afaf + the env-gated trace
`BSP_THROTTLE_TRACE`, `local\p21d2_lms_usn01.log`).** 0099D300 is not the cause. Its demand holds 0.4252 through
604.23 s (dead band skipped, corr -0.37). The cut comes from 007DFDD3-007DFE7F, vehicle avoidance's aircraft arm,
which sets pilot+25Ch = interp(3, 1, 15, -1, s) = -1.0 from 604.23 s on. 0099C130 then stores throttle 0.01
(0.0079 after 007BB6E0) and air brake 1.0. The threat is ConSBD1|.-III. It touched down at 598.63 s and is taxiing
up the deck in land/park at 20.6 m/s relative. Its vt[34h] in the host is plane_world_velocity
(-9.80, -0.52, -16.49), the deck-relative velocity rotated into world axes. Against ConTBD1's 51 m/s, s = 32.4 < 35
and t = 1.95 s < 2, so the arm fires. In the bytes, 007BBB70 copies unit+AC8h, and 007D9CE0 keeps that vector in
the parent's local frame (section 75). So the image's threat velocity is the carrier-frame vector, and its
closure differs with the carrier's heading. On current main (`local\p21c_thr_usn01.log`) the geometry differs: no
cut, and ConTBD1 touches down at 604.63 s.

**`kAvoidParentLocalVelocityBound` (OFF)**, in `avoid_velocity` (all six vehicle / terrain avoidance reads of
vt[34h]): a parented plane answers `deck_local_vel`. Prediction: only rows with a plane on a carrier deck while
another plane flies close by move. Avoidance against parked or taxiing deck planes changes by the carrier's heading.
**84, result: `kAvoidParentLocalVelocityBound` ON.** USN01 `usn01_p8_u1_i.txt`, 30000 fr, exes at 4bd013e4a
(`local\p21e_off_usn01.log` against `local\p21e_on_usn01.log`, manifests beside them). pair_diff MOVED, as
predicted. Deaths 36 / 36 with 12 death times shifted, every one a US carrier plane (0.1-2.4 s). Stows are 19 / 19 and
re-takeoffs 0 / 0. The aircraft-arm throttle sets on US planes go 447 -> 448. Flipped by mechanism: it is
007BBB70's unit+AC8h in 007D9CE0's parent frame, and it is exercised. UNCERTAINTY: the other vt[34h] readers of a
parented plane (the deck term's own parent read is the carrier's, unaffected) still take world axes. Section 75's
whole-frame item stays open.
## 85. Torpedo state-enter stores 009D2530 / 009D15D0 (cc9-physics21, 2026-10-18)

Source: cc13 CC13_SEG67B 3.51. Bytes read on disk (this installation's exe, read only).

- **009D2530**, the done/prepare enter (vtable 00D21320 +4h), stores state+98h = -1.0 at 009D253B (00D7A260) on every entry. Done is task+310h and prepare is task+740h, and each has its own +98h. 009D49A0 arms only prepare's. The host had one `torpedo_drop_timer`, reset only at install. A done tick could therefore count down a timer that prepare armed earlier, and a re-entered prepare kept its old arm.
- **009D15D0**, the aim enter (vtable 00D212FC +4h), does four things in order: it calls 009D02A0 (the host already redraws there under kAimErrorDrawBound), stores approach+A8h = 1 at 009D15DE, state+24h = 0 at 009D15E7, and state+2Ch = 0 at 009D15EA.
  - state+24h is the release timer 009FA3A0's +0Ch enable, the host's `torpedo_aim_timer_enabled_0c`.
  - state+2Ch is the aim-complete byte that 009D236E sets.
- The aim exit **009D1350** (vtable +8h) stores approach+A8h = 0 at 009D1353.
- The census of +A8h byte writers in 009D0000-009D6000 found five:
  - 009D0511, in the constructor;
  - 009D12F0 and 009D1300, a set and a clear accessor with no rel32 or absolute references;
  - 009D1353, the aim exit;
  - 009D15DE, the aim enter.
- So +A8h is 1 only while the plane is in aim. It gates the replan's over-land writers at 009D3688. The host never stored it, so `over_land_a9` was never written there.

Switches, both OFF in this commit:
- **kTorpedoStateEnterTimersBound** splits the done timer out (`torpedo_done_timer_98`) and resets the entered state's timer to -1 on each enter.
- **kTorpedoAimEnterStoresBound** makes the aim enter clear +24h and +2Ch and set +A8h = 1, and makes the aim exit clear +A8h. The latch diagnostics (`torpedo_aim_complete_tick` and the rest) are not cleared; they re-latch on the next completion.

Counters, which run in both builds, print one line per plane: "aim enters=N (2Ch already set=M) done-state drops=D stale-timer drops=S".
- A stale-timer drop is a release from the done/prepare tick that read a positive timer with no 009D49A0 arm since the last enter.

Predictions:
- OFF: on reattack rows, M > 0 (a reattack enters aim with +2Ch set, so the 009D31B0 hold fires at once), and S > 0 wherever done follows an armed prepare.
- ON: S = 0 and no releases in the done state from a timer.

**85, result: both switches ON.** The exes were built at b3c467f82 (`local\p21e_off`, and `local\p21e_on` with both switches flipped). Env: BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1. Manifests sit beside each log.

| row | logs | aim enters | 2Ch already set | stale-timer / done-state drops | pair_diff |
|---|---|---|---|---|---|
| USN21 `s46_u21_f`, 80000 fr | `local\p21f_{off,on}_u21f.log` | 73 over 73 planes | 0 | 0 / 0 | 1, gameplay identical (completed 1325.17 s both) |
| USN04, 40000 fr | `local\p21f_{off,on}_u04.log` | 16 over 16 planes | 0 | 0 / 0 | 1, gameplay identical |

- **Exercised:** the +A8h gate. With ON, the replan's over-land writers run during aim:
  - 0041BC20 samples rise by 1493 on USN21 and by 414 on USN04;
  - `BotApproach::target_reachable 007df360` now shows as UNIMPLEMENTED with the same 1493 and 414 calls.
  - Its host answer leaves `over_land_a9` alone, and no terrain sample exceeded 00CE38B8. That gap is 007DF360 (unit+C50h); it is not modelled, so the decision the image makes there is still missing.
- **Not exercised:** no plane re-entered aim (one aim enter per plane on both rows), and no row reaches prepare (as SQUADRON_LAND_TASK 5hr found). So the timer split and the +2Ch clear did not change anything here.
- Both are faithful stores read from the bytes, with gameplay identical on both rows, so both are flipped ON.
- WINS: nothing moved.

## 86. HANDOFF (cc9-physics21, 2026-10-18)

The lane is PHYSICS / PLANE FLIGHT, worked in tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 on branch agent/cc9-physics1.

Commits this session:
- 22e58011f (landed): kParkStowTrace, OFF.
- aa13241d2: the BSP_THROTTLE_TRACE env trace.
- 4bd013e4a / 5caaef9d8: kAvoidParentLocalVelocityBound, ON (section 84).
- b3c467f82 / 85951f409: kTorpedoStateEnterTimersBound and kTorpedoAimEnterStoresBound, ON (section 85).

Closed:
- The stow: the brake law matches the image. The deck term stays OFF and the blocker is the carrier's speed (section 83).
- ConTBD1's 604.43 s cut: it was the avoidance frame, not 0099DAC8.
- Item (a), the air-ops site tick: it only steps the lift.
- Item (b), terminal speed: no cap. It is MaxSpeed x throttle, less one frame of 00937440 drag.

Open, in priority order:
1. **007DF360 target_reachable is UNIMPLEMENTED** (unit+C50h). It now runs from the torpedo replan during aim. The image may set over_land_a9 there and abort or replan the run. Read 007DF360 and the unit+C50h writer.
2. **The timer split and the +2Ch clear are unexercised.** Find a row with a torpedo reattack (aim re-entered) or a prepare state, and re-check the counters on the "aim enters=" summary line.
3. **The host's Enterprise holds heading 0.0000 against a target of -2.5885** with rudder 0.004 (USN01, section 84). This belongs to the ships lane (ships59).
4. **Section 75's whole-frame item:** the other vt[34h] readers of a parented plane still take world axes.
5. **kLandFinalParkEdgeBound** stays OFF and unreached. Stream order (item 4) is not cheap.

## 87. 007DF360, the replan's line-of-sight writer of +A9h (cc9-physics22, 2026-10-18)

Bytes read on disk (this installation's exe, read only). 007DF360 sits in cc15-r48's lease (007D1E50-007F8390), so its ledger name goes through the lead.

- **The call site.** 009D36C3-009D36F3: when approach+4 (the unit) is set and [unit+C50h] != 0, 009D36E4 calls approach->vtable[0](&local) for the aim point, then 009D36EE `007DF360(ECX = [unit+C50h], [approach+CCh], point)` and 009D36F3 `MOV [ESI+A9h],AL`. That is an overwrite, not an OR: a 0 clears the terrain arm's 1 from 009D36BC. The host had `a9 = r ? 1 : a9`.
- **The C50h object.** 007D61C0-007D621F (plane construction): [unit+C50h] = 0, then the 007E1E20 object is built when [game+1FE4h] is 0 or 1 and unit+54h != [[game+5FCh]+908h] (the scene's DummyAIEnabled, 3 when absent). That is the same predicate as kAvoidanceDummyAiGateBound. Its +30h list (IsKindOf 6 or 0Fh within 1080, refreshed every 3 s by 007E11D0) is the host's `nb_near_30`.
- **007DF360**, `char __thiscall(neighbours, target, const float* point)`, RET 8 (body 007DF360-007DF4E9, exclusive; INT3 follows):
  - 007DF36A: o = [[ECX+4]+28h]; null returns 0. Its +FCh/+104h are taken as the plane's own x/z (LABELLED; the refresh uses the same owner).
  - 007DF3A8-007DF3D0: d = (o.x - p.x, 0, o.z - p.z), len = 0042B2F0(d): float((x*x + y*y) + z*z), sqrt when above 1e-10 ([00CE3820]), else 0.
  - 007DF3E4-007DF4D7, per +30h member n: skip null, skip n == target, skip !n->vtable[5Ch](6). a = n.x - p.x, b = n.z - p.z (float stores); dot = float((a*dx + 0*0) + b*dz); JBE skips dot <= 0 or NaN; t = float(dot / len); q = float((0 + a*a) + b*b); perp = float(q - t*t); return 1 when Length^2 * 0.22 > perp (FCOMIP/JBE; [n+538h]+A0h, double [00D06BA0] = 0x3FCC28F5C0000000).
  - Return 0 after the walk.
- **Meaning.** A ship (kind 6), other than the target, lies ahead of the aim point toward the plane and within 0.469 x its Length of that line. The aim tick then floors at 30 m instead of 5 m (009D1613-009D163F), which moves the release threshold (009D1D5A) and the dive command (009D1E8E).

Switch **kTorpedoLineOfSightBound**, committed OFF. The host interface now returns -1 (no C50h: skip), 0 or 1, and the core overwrites on 0/1. OFF returns -1, which keeps the old answer exactly. ON models the walk over `nb_near_30` with float ops (24-bit per op); class+A0h is `motion_class.hull_length`. The per-plane line "los 007DF360 calls= screened= a9 clears= zero-length=" prints on ON.

Predictions:
- OFF vs ON on USN21 `s46_u21_f` and USN04: calls equal the 1493 / 414 of section 85 if the rows are unchanged. `screened > 0` wherever an escort sits between the torpedo plane and its target (USN04's escorted carriers). Each screened aim floors at 30 m and holds the drop longer, so drop times and torpedo hits move on those planes.
- a9 clears = 0 (no terrain sample exceeds 10.0 on these rows).
- zero-length > 0 would mean a screening ship has no Length key, which disables its screen.

**87, result: kTorpedoLineOfSightBound ON.** Exes built at f72a22d73 (`local\p22a_off`, `local\p22a_on` with the switch flipped). Env BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1; manifests beside each log.

| row | logs | 007DF360 calls | screened | pair_diff |
|---|---|---|---|---|
| USN04, 40000 fr | `local\p22a_{off,on}_u04.log` | 414 over 16 Kates | 0 | 1, gameplay identical |
| USN21 `s46_u21_f`, 80000 fr | `local\p22a_{off,on}_u21f.log` | 72 planes | 53 on 10 planes (Lexington_sqn22: 14 of 15) | 3, moved |

- The mechanism is exercised where the US torpedo squadrons attack the escorted IJN groups on USN21: a screened aim floors at 30 m. Three planes show "a9 clears=1", a screen answer replaced by a clear one on a later replan, which the old OR could never undo. Zero-length = 0: every screening ship has a Length.
- USN21 moved: torpedo drops 12 -> 8, torpedo-task releases 3 -> 1, dive-bomb-task releases 2 -> 10, deaths 299 -> 301, and the mission still completes (1325.17 s -> 1355.20 s). USN04's Kates are never screened.
- Faithful by the bytes, so it is flipped ON. The drop loss is the image's 30 m floor, not a host defect.

## 88. The 009D4A70 cruise pass (cc9-physics22, 2026-10-18)

Bytes read on disk (009D4A70-009D4C0D, `disasm-raw`). The body in order:
- 009D4A77 0099B660, then 009D4A82 007B8AD0 (flight leader). Only the leader runs 009D4A91-009D4B5D: the cruise altitude [+40Ch] (replaced by [00CE3850] when below double [00D7A370]) into the squadron control block +37Ch's three slots, with 009D4B3E MOVSS [00CE4C04] = 9999.0 into ctl+39Ch (+37Ch+20h) when its byte +12h is clear, 0 > [+8] and byte +2Fh is clear; then 009D4B57 task+46Ch = ctl+398h.
- 009D4B63-009D4BA7, the clamp: task+484h = max(task+484h, float([tuning+434h] * task+41Ch)) (FCOMI/JBE; a NaN takes the product).
- 009D4BAF-009D4BC8, the flip: approach = task+3F8h; when approach+12Ch > 0 it becomes [00D7A208] (-0.0) minus itself.
- 009D4BD0-009D4BE6, the park: approach+B0h and +ACh = [00CF87D0] (999999.0).
- 009D4BEE-009D4BF9: approach+60h = 0.0 (MOVSS from a zeroed XMM1), +6Ch, +68h, +64h = 0.
- 009D4BD8/009D4BDB/009D4BFC: FLDZ, FSTP to the pushed slot, CALL 009D3420 with ECX = approach. Then 009D4C08 tail-JMP 0099B740.

The old cites 009D4AC4, 009D4AE7, 009D4AF5 and 009D4AA9 are not instruction starts in this body; they are fixed with no switch, and `torpedo_engage_range_009d4ac4` is renamed `torpedo_engage_range_009d4b63`.

**The dt = 0 pass.** 009D3420 tests dt < +12Ch (009D3640-009D3682). The reset leaves +12Ch = -U(0, 1) and the flip keeps it at or below 0, so the pass replans: +12Ch += +128h (1.0) - 0. The plan position is parked, so the plan is rebuilt there and then. 009D34BB calls 009FADA0, whose 009FAEB0 tests sub+41h and runs the re-pick 009FA260 when it is set (the constructor 009FB200, ResetAimOffset 009FA350, SetAimError 009FA380 through 009D02A0, and OnTargetDestroyed 009FAF90 set it). The timed re-pick 009FAE26-009FAE44 needs sub+60h + dt > 2.0 and cannot fire at dt = 0. In the host the first arm tick does all of this one tick later. After that tick the replan timer agrees in both (1 - U - dt).

The host also cleared approach+5Ch (the scan's turn offset) where the image clears +60h. +5Ch is read by the aim heading (bearing + offset) and is only rewritten by a scan, so on a re-install without a scan the image keeps the old offset.

Switch **kTorpedoCruisePassBound**, committed OFF: the +60h clear in place of +5Ch, the flip, and the pass at the end of the install (after the reset's draws, as in the image), through the same TorpedoArmBinding the arm tick uses. The per-plane line "cruise pass 009D4BFC=N replans=M" prints on ON.

Predictions:
- N = 1 per install; replans +1 per plane (the pass), and the first arm tick no longer replans.
- The host's re-pick takes its unit draws from a per-shooter seed (`approach_target_ref_unit_draws_substitute`), not from stream 1, so moving it one call earlier moves no draw. The plan is rebuilt from the same positions (the install and the first arm tick run in the same host call). Expected: gameplay identical on USN21 and USN04 (pair_diff 0 or 1). Any movement would come from the +5Ch carry on a retarget re-install.
- 009D4C10 break-off: a NaN range breaks off in the image (FCOMI/JA) and not in the host's >=. A NaN range needs a NaN position, so it is left as is.

**88, result: kTorpedoCruisePassBound ON.** Exes built at fe6e06f9f (`local\p22b_off`, `local\p22b_on`). USN04 (`local\p22b_{off,on}_u04.log`) and USN21 `s46_u21_f` (`local\p22b_{off,on}_u21f.log`) are both gameplay identical (pair_diff 1): death rows, plane death modes and the unit table are identical. USN21 took 108 install passes (009D3420 and 009FADA0 calls 151237 -> 151345); only the per-tick diagnostic lines moved. As predicted, the pass moves no draw. Flipped ON by mechanism.

## 89. The C25h arm of the moveto ticks (cc9-physics22, 2026-10-18)

Bytes (`disasm-raw`, reviewer-confirmed in the brief):
- 009C2430, the moveto state tick: after the speed stores (+2B4h, 009C2503 +2B0h = 0, 009C250A +2D8h = 1), 009C2519 `CMP byte [unit+C25h],0` / JE 009C255C. When set: 009C252A cmd+2BCh = 0.0, 009C2532 +2D0h = 2, 009C2543 +2C4h = 0.0, 009C254B +2CCh = 1, RET 4 at 009C2559.
- 009C26D0, the circle tick: the same test at 009C275A after its speed stores (009C273B-009C274F), then 009C2769 / 009C2771 / 009C2782 / 009C278A the same four stores (+2CCh from ECX = 1, loaded at 009C274A), RET 4 at 009C2794.
- These are the four stores of the follow release arm 009C200F (section 78) without its speed stores, which the moveto ticks have already made.

The host's two ticks only recorded the arm and returned, so the previous tick's pitch, bank and heading commands stood. Switch **kMoveToC25ArmBound**, committed OFF: when set, it makes the four stores (and drops the heading-written latch, as follow_release_arm_009c200f does). The native table shows `BotStateMoveTo::c25_arm` / `BotStateMoveToCircle::c25_arm` as concrete on ON.

Predictions: unreached on USN04 and USN21 (no moveto tick with C25h set). The reach candidate is USNOS `l65_os_c1` (`Intrepid_sqn22|.-II`, C20h = C25h = 1, section 78). Where it is reached, the plane levels (pitch target 0, mode 2) and holds heading mode 1 while the request is pending. The dive bombers' drop timing moves only if a moveto tick runs with the request pending.

**89, result: kMoveToC25ArmBound ON (faithful, unreached).** Exes built at 2cacdab2f (`local\p22c_off`, `local\p22c_on`). USNOS `l65_os_c1` 30000 fr (`local\p22c_{off,on}_os.log`) is identical apart from noise (pair_diff 0). It runs 2890 moveto state ticks, but no moveto or circle tick runs with C25h set; the c25_arm record never appears. USN21 `s46_u21_f` (`local\p22c_{off,on}_u21f.log`) is gameplay identical and has no moveto tick at all. The four stores are read from the bytes, so the switch is flipped ON, unreached. A reach needs a moveto-command task on a plane with a pending release.

## 90. The moveto tick head, the level-bomb goaway order, the +2B0h clears (cc9-physics22, 2026-10-18)

- **kMoveToTickHeadBound** (OFF). 009C2440 `MOV byte [[state+4]+18h]+26Ch, 1` comes first in the moveto state tick. The host only ever stored 2 (009C1FE2's follow-tick stamp). `plan_mode_26c` has no host reader (its image reader is 0099D300's gate A, section 6c of BOMBER_AFTER_TASK). The distance: 009C246A-009C2496 compute dx, dz and d2 = dx*dx + dz*dz, each stored or rounded to float, then 009C249A-009C24A8 compare d2 against double 1e-10 ([00CE3820]) with FCOMI/JBE, taking 0 at or below it and float(sqrt) above it. The host used a double d2 and 1e-6. Prediction: gameplay identical, since the distance feeds 009C1850's speed blend only through `dist`, and the two differ only in the last float bit or below 1e-3 m.
- **kLevelBombGoAwayOrderBound** (OFF). 009B5680: 009B5724 FLD approach+24h / 009B572A FSTP to a local, 009B5746 the draw, then 009B574B FLD the ratio, FMUL the clamped base (FMUL [ESP+4]), FMULP the draw: +18h = draw * (ratio * base), per op at 24 bits. The host computed (draw * base) * ratio. Prediction: identical wherever the ratio is 1.0 (the floor). The level bombers here have MaxSpd below the reference speed, so they sit on the floor.
- **The +2B0h clears, no switch.** 009C1850 stores +2B0h = 0 at 009C18A0 after its +2B4h store. Three host moveto sites called its speed but kept +2B0h: the dive moveto, the torpedo moveto and the land moveto. Each now stores 0 there, as st_speed already did for the others. Behaviour identical expected: +2B0h = 1 comes only from the land standby and final states, and those leave through states that clear it.
- **attack_commands.cpp kRows.** The strafe test 009CC850 compares +44Ch, falling back to +468h when +44Ch is 0 (009CC898-009CC8AA). Both 0 answers valid. The row's single offset field is documentation only (no reader); a comment now says so.

**90, result.** Exes built at c87cfe51e (`local\p22d_off`, and `local\p22d_on` with both kMoveToTickHeadBound and kLevelBombGoAwayOrderBound flipped). USN04, USN21 `s46_u21_f`, USNOS `l65_os_c1` and USN01 `usn01_p8_u1_i` (`local\p22d_{off,on}_{u04,u21f,os,u1i}.log`) all came out gameplay identical, with pair_diff 0 or 1. USNOS installs 24 level-bomb tasks, every one with +24h = 1.000 (the ratio floor), and none reaches goaway, so the order switch is unreached and could not move anything on these rows. Both are faithful to the bytes. **Flip pending:** src/game_hosts_units.cpp is leased to cc13-convoy until 04:59Z, and p22_flip2_units is queued behind it.
- The unswitched +2B0h clears: c87cfe51e's OFF build against 2cacdab2f's (`local\p22c_on_{u21f,os}.log` vs `local\p22d_off_{u21f,os}.log`, and `local\p22b_on_u04.log` vs `local\p22d_off_u04.log`) is gameplay identical on all three rows, as predicted.

**Item: a torpedo plane that re-enters aim (reach for kTorpedoStateEnterTimersBound / kTorpedoAimEnterStoresBound).** USN01 with gunnery52's `config\win_orders\usn01_p8_u1_i.txt` (30000 fr): `ConTBD3|.-III aim enters=3 (2Ch already set=0)`, on both builds. The aim enter and exit therefore run three times, and the +A8h set and clear and the LOS writer run in each window. The +2Ch clear is still a no-op there: the plane never completed aim (+2Ch was never set) before it re-entered. The done enter 009D2530 is reached on USN21 (`Enterprise_sqn23|.-II`, `Cabot_sqn56|.-III`, one each) with stale-timer drops 0, so the timer split runs but never decides a release. No row seen reaches prepare. gunnery52's experimental `g52_bd_on` build shows `ConTBD1|.-II aim enters=291 (2Ch already set=288)` on USN01; that is a flicker in that build, not on main.

**Item: the census of host readers of a parented plane's velocity.** `local\p22_census.py` and `local\p22_census2.py` list every `plane_world_velocity` reader in game_hosts_units.cpp (about 70 sites) and flag those within reach of a `deck_parent_plus_one` test. The host's `plane_world_velocity` for a parented plane is the world-axes rotation of the same relative vector (section 84), so a reader that takes only its length (vt[38h], 007B8E60 / ctl+6Ch, 007D8020) gets the same value either way. The deck-reached readers:
- `avoid_velocity` (vt[34h], 007DF4F0 and the terrain arms 0099F1C0 / 009A1420): already parent-local under kAvoidParentLocalVelocityBound.
- `carrier_deck_capture` / `carrier_deck_carry_in` / `carrier_deck_reparent_007c71e0` (007D9CE0, 007C7277): these convert between frames explicitly.
- `carrier_deck_carry_in`'s (3.0 - vt[38h]) test at 15075, `wanderer_fixed_step_007be060`, Lua's `plane_cached_speed_007b8e60` and the takeoff / land speeds: lengths only, so the frame does not matter.
- `plane_touchdown_007cc440`'s y > -6.0 test (vt[34h] y, 007CC440): runs before the plane is parented to that owner.
- The core law's parent term at 007DC0A2-007DC131 reads the PARENT's vt[34h] (the carrier's world velocity), not the plane's.
No further direction-dependent reader of a parented plane's own vt[34h] was found, so no new switch. The core law's own use of ctl+18h in the body frame while parented is still section 75's whole-frame item.

**90, flips.** kMoveToTickHeadBound and kLevelBombGoAwayOrderBound are ON, on the pairs above (identical, faithful).

## 91. Strafe draw counts (cc9-physics22, 2026-10-18)

- 009CB0F0, the strafe aim enter: +1Dh = +1Ch = 0 (009CB110 / 009CB113), then 009CB116 00BD2F10(ECX = 1, 0.2 [00CE54A0], 0.8 [00CE74F8]) FSTP +18h (009CB11B), RET at 009CB11F. No strafe tick reads +18h. The host took no draw.
- The strafe goaway's tightturn (009BC030) and flikflak (009BC0A0) pushes each construct a bot task through 0099C6F0. Its seeds draw +308h = -U(0, 1) on stream 1 and +3E8h = U(-1, 0) on stream 0 (section 81). The host's push builds no task, so it took neither draw.

Switch **kStrafeDrawCountBound**, committed OFF. It takes these draws under the keys the host already uses (`<name>#sta18`, `<name>#tb308`, `<name>#tb3e8`) and drops the values; the pushed task's own +308h / +3E8h are not hosted (LABELLED). A summary line "strafe draw-count takes=" prints when nonzero.

Prediction: with BSP_GUNNERY_RNG_STREAMS=1 (the reference env) each purpose has its own keyed stream. The aim draw is in a new key and moves nothing. The push draws advance `#tb308` / `#tb3e8`, so the next task install on that plane takes a later value of its think phase and ga timer, and only strafing planes that tightturn or flikflak can move. USNOS `l65_os_c1` strafes (planes #1.x on TroopTrans); USN04 and USN21 have no strafe task installs.
- The remaining low item: the land standby 009B0FE0 / final 009B1ED0 +2B4h blend is per-op 24-bit in the image (009B11E9-1205, 009B1FC6-1FDF) and round-once double in the host. The dive goaway 009C4A40 ceiling NaN is not reachable without a NaN altitude. Both are left open.

**91, result: kStrafeDrawCountBound ON.** Exes built at 188de95ef (`local\p22e_off`, `local\p22e_on`). USNOS `l65_os_c1` (`local\p22e_{off,on}_os.log`) took 38 draws and is gameplay identical, with the death rows, death modes and unit table all identical. USN01 `usn01_p8_u1_i` (`local\p22e_{off,on}_u1i.log`) is identical. Flipped ON: these are the image's draw counts.

## 92. The land blend at 24 bits (cc9-physics22, 2026-10-18)

009B11C4-009B1205 (the standby 009B0FE0) and 009B1FC2-009B1FDF (the final 009B1ED0) both compute +2B4h = a + [approach+4Ch] * (b - a). The sequence is FLD b, FLD a, FLD ST0, FSUBP ST2 (b - a), FLD +4Ch, FMULP ST2, FADDP, FSTP; on the main thread every op rounds to float. In the standby, a = 007C47F0 (LevelFlight x StallSpd) and b = class+190h; in the final, a is the local at [ESP+20h] and b is the 00419010 result.

The host rounded the sum once: in the standby (b - a) was taken in float, in the final in double, and the product and sum in double. Switch **kLandBlendFloatOpsBound**, committed OFF, takes each op in float (`land_blend_2b4`). OFF keeps both old expressions exactly. Prediction: the commanded speeds differ by at most one float ulp, so landings move at the last bit at most; expected gameplay identical, or a sub-tick shift on carrier recoveries (USN01).

**92, result.** Exes built at 25ef73b11 (`local\p22f_off`, `local\p22f_on`). On USN01 `usn01_p8_u1_i` (`local\p22f_{off,on}_u1i.log`) the standby is reached (ScoutDauntless, spacing 1.000) and gameplay is identical. USN21 `s46_u21_f` (`local\p22f_{off,on}_u21f.log`, which completes at 2861.67 s in both) is identical apart from noise (pair_diff 0). The mechanism is faithful to the bytes, so the switch is to be flipped ON. The flip waits for src/game_hosts_units.cpp (cc13-stophead holds it; p22_flip4_units is queued).

**92, flip (cc9-physics23).** kLandBlendFloatOpsBound is ON, on the `local\p22f_{off,on}_{u1i,u21f}` pairs above (identical, faithful). The same commit corrects the comment at the 007D87F6-007D8838 vy-zero arm: the rule is (dyn+A4h != 0, the 007D87FD JNE, OR dyn+94h > 0) AND |dyn+6Ch| < 1e-5 -> dyn+68h = 0, and a NaN vz keeps vy. The code already did this.

## 93. HANDOFF (cc9-physics22, 2026-10-18)

The lane is PHYSICS / PLANE FLIGHT, worked in tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1). The pair launcher is `local\p22_pair.ps1 <exportdir> <row>`, with rows u21f, u04, os, u1i (the orders paths are inside it).

Switches this session (sections 87-92):
- ON: kTorpedoLineOfSightBound (moves USN21: 53 screened answers, torpedo drops 12 -> 8), kTorpedoCruisePassBound, kMoveToC25ArmBound (unreached), kMoveToTickHeadBound, kLevelBombGoAwayOrderBound (unreached: ratio 1.0, no goaway), kStrafeDrawCountBound.
- Not switched: the moveto +2B0h clears, the stale-cite fixes on 009D4A70 (`torpedo_engage_range_009d4b63`), and the kRows comment.
- **OFF, flip ready: kLandBlendFloatOpsBound.** Its pairs are identical (section 92). Flipping it is a one-line edit in src/game_hosts_units.cpp; the lease is queued as p22_flip4_units behind cc13-stophead. Build, add a line to section 92, commit.

Open:
1. The two items the lead routed after the resume, "the ConTBD3 stow trace (Q1 only)" and "the ~20472 comment", reached this session only by name (lost in compaction). Get the brief from the lead.
2. A reach for the moveto C25h arm: a moveto-command task on a plane with a pending release.
3. A reach for prepare. USN01 `usn01_p8_u1_i` re-enters aim (ConTBD3|.-III, 3 enters), but the +2Ch clear is a no-op there.
4. The dive goaway 009C4A40 ceiling and the 009D4C10 break-off differ from the host only on NaN, which no row reaches.
5. Section 75's whole-frame item (the core law's ctl+18h while parented). Section 90's census found no other direction-dependent reader.
6. The deck term stays OFF (section 83). The ships lane has the Enterprise turn question.

## 94. The ConTBD3 stow trace (cc9-physics23, 2026-10-19)

The question (routed from cc13's static reading): ConTBD3 on physics21's `p21b` (USN01 `usn01_p8_u1_i`, built at 2c5fa3ef8) stopped on deck at 1285.18 s when the 0099D300 dead band released (skip=0) and the throttle went to 0 with the brake at 48. Do the host's think phase and lowpass order match the image?

**The image's order (static, disk bytes).** In the core law 007DB680: 007DB697 calls 007D9C10, which sets ctl+B0h from the unit orientation, ctl+3Ch..44h = the body-frame ctl+18h, and (through 007D8020) ctl+6Ch = |ctl+18h|. 007DC6E6 calls the integrate 007D8470(dyn, dt, unit+74h, ctl+B0h), which holds the 007D87F6 vy-zero arm. Then 007DC6EE-007DC708 copy dyn+64h (the body velocity) to ctl+3Ch (EBX = ctl+3Ch, set at 007DB6AE), 007DC70E-007DC71F copy dyn+4Ch (the world velocity) to ctl+18h, and 007DC743-007DC80B low-pass ctl+30h toward that ctl+18h (or copy it when dt is not below [00CE3868]). 007DC80E calls 007D9C80, which rebuilds ctl+18h by rotating ctl+3Ch with the new orientation and ends in 007D8020: ctl+6Ch (unit+B1Ch) = the length of the post-integrate velocity. So both inputs of 0099DA97-0099DAC8 are snapshots of the same post-integrate state, and the low-pass takes that step's v (updated after it, not before). 0099DABE multiplies by double 20.0 (00CE3D88), then by [ESP+6Ch].
**The host.** The low-pass (kPlaneVelocityLowpassBound) runs after the velocity integrate with the same step's v, and the think recomputes |v| from `plane_world_velocity` at think time. That is the same order. The think runs every 0.1 s (kPilotThinkInterval, `pend=0.1000` in the trace), so it sees every other step.

**The run.** `local\p23a_trace_u1` (exe at 4b74dd6b4, USN01 with the p21b orders, 26200 frames, BSP_THROTTLE_TRACE / BSP_PLANE_GROUND_TRACE = ConTBD3). On current main ConTBD3 touches down at 472.76 s, parks, is taken by the elevator at 486.85 s and stows at 489.00 s. All 16 touchdowns in the run stow (ScoutDauntless 423.17 to ConSBD1|.-III 839.67). The 1285 s deck stop does not occur, so there is no release think to report. Of the 141 deck thinks from 472.8 to 486.85 s, 27 skip. Taking |v| = lp + corr / (20 * pend), 62 thinks land on the vy = 0 phase and 74 on the vy = -0.736 phase (5 match neither, within 0.01 m/s against the forward speed). The think does not stay locked to one phase.

**Verdict.** The order matches the image, and the failure no longer reproduces on main. kPlaneForwardSpeedDeckTermBound stays OFF. Still open: whether the think's parity against the deck vy alternation matches the image. That would need the 009998A0 accumulator's start phase, and no row now needs it. The raw log was deleted after this was written.

## 95. cc13's squadron switches paired (cc9-physics23, 2026-10-19)

The exes are pair_export builds of 5b9d1b7d2, which contains cc13's 880318465: `local\p23c_off`, `local\p23c_a` (kSquadronStopHeadMoveToSelfBound), `local\p23c_ab` (plus kSquadronDogfightLeashBound) and `local\p23c_abc` (plus kSquadronGeneralStepTailBound). Logs are `local\p23c_<build>_<row>.log`, launched by `local\p23_pair.ps1` with the reference env.
- **(a) Stop head, off vs a.** BSM09 d1 (`bsm09_l68_b9_d1`): identical, moveto_self 0 (unreached), still EndMission 327.04 s. USN04: MOVED. 6 free-attack moveto-self sends (18 member `009F8615` orders), free-attack sends 27 -> 34, same-target refusals 615 -> 0 (the head is now the moveto, not `stop`), deaths 56 -> 59. The death rows are RNG-coupled plane rows. Reached as the header describes.
- **(b) Leash, a vs ab.** Identical on BSM09 d1, USN04, USNOS `l65_os_c1` and JM06 (no leash line). Reached on BSM09 with no orders (`p23c_ab_b9n`): "Zuikaku_sqn01 leash at 167.21 s: 00E08F58 on Donald outside 3750 m of the head point, override ended". That row fails either way, 183.06 -> 180.06 s (Donald).
- **(c) General step tail, ab vs abc.** BSM09 d1: gameplay identical, 19 "squadron step tail:" lines (empty-queue squadrons get 0084E595 moveto-self at 0.00 s), still EndMission 327.04 s. USN04: MOVED, 22 tail lines, RNG-coupled. JM06 `jm06_s45_j6_k`: MOVED, 7 tail lines (b4=1 moveto-self loiter for the PBY and Lexington squadrons, including two completed stage-2 00E08F38 heads at 629.02 s). It still completes, at 1457.30 s instead of 730.95 s, guard logged.
- **USN21 `s46_u21_f` could not be paired.** OFF, ab and abc all exit 0xC0000409 at mission frame ~6018 (301 s) on 5b9d1b7d2. physics22's 25ef73b11 completed this row, so the crash comes from main between those commits, not from these switches. Reported to the lead.
- **USN21, paired later.** The crash was kVehicleClassPreloadCc15Bound (off again on main f403f1bf4). Exports of a7ad66226 have every other switch at main: `local\p23f_on` (main, step tail ON) and `local\p23f_off` (kSquadronGeneralStepTailBound=false). On `s46_u21_f` (`local\p23f_{off,on}_u21f.log`) there are 172 "squadron step tail:" lines ON and 0 OFF. Gameplay MOVED (deaths 578 -> 576, shots 98755 -> 91544, dive-bomb releases 13 of 219 on both, torpedo drops 8 on both); the moved rows are F6F and Kamikaze dogfight deaths after 2720 s. The mission completes in both, 2911.72 s OFF and 2916.72 s ON, guard logged. The switch stays ON.

**Flips.** All three are ON, by mechanism: each is reached where its header predicts and is faithful to cc13's static reading, and the moved rows are RNG-coupled. The lead confirmed cc13's gate commit is in before (c). Moved WINS row: JM06 `s45_j6_k` (730.95 -> 1457.30 s, still a win). BSM09 d1 is unchanged.

## 96. The dive cruise profile's travel-altitude gate and clock re-seed (cc9-physics23, 2026-10-19)

From cc13's seg67d review. 009C8920 (task slot +54h; with kTaskProfileActivationOnlyBound it runs at the head change) does three things the dive host lacked.
- **Block 1, 009C894F-009C8971.** This is inside the 009C8938 leader branch, on [task+404h]+37Ch. Unless +38Dh is set: when +380h < 0 and +3A9h is clear, +394h = tuning+4C0h and +3ADh = 1; then +3A9h = 0 either way. The dive host read +394h through `strike_ctl_cruising_alt_394`, which keeps a script altitude while `sq_alt_set`, so an expired SquadronSetTravelAlt was never replaced and +3A9h was never cleared. The moveto (009C3650) and level-bomb (009B8C90) hosts already have this gate. The dive copy mirrors the moveto one: clearing `sq_alt_set` stands for the tuning write.
- **The re-seed, 009C8A66-009C8A74.** task+4BCh (approach+C4h, the hit clock) = 3600.0 (00CFDEB0, the bytes 00 00 61 45) for every member, past the 009C8A1B join. Then 009C8A7C calls 009C7A80 with dt 0.0. The host carried only the 009C3FD2 install seed. The reader between calls is the dive goaway's 009C4AA4 (1.0 > C4h forces the goaway re-roll); only the hit notice 009C7900 zeroes the clock. A head change after a hit (a pushed task popping back) therefore gives the image a fresh 3600 and the host a small value. Host: on the profile call the clock leaves the tick at 3600 + dt (LABELLED: the function's own advance stands for the tick's 009C7A80).
- Both are behind **kDiveCruiseProfileGateBound**, committed OFF. The summary line "dive cruise profile gate" counts the resets of a set script altitude and the re-seeds of a clock below 3600.
- Not changed: the +4B0h max (009C8A42-8A50 is cur > prod ? cur : prod, so a NaN cur gives prod, while the host keeps a NaN). No row has a NaN attack distance. The unused `dive_bomb_cruise_profile_009c8920` in src/dive_bomb_task.cpp now carries a comment listing its defects. Text only: src/torpedo_task_arm.cpp's 00CE3D48 is 1.6f widened (3FF99999A0000000), not a double 1.6.

Reach: BSM09 (bsm09_l68_b9_d1) gives D3A Val 1-5 SquadronSetTravelAlt 1200 / 1500, unforced, and they fly dive-bomb tasks. Prediction: a reset needs a profile call after the 0.5 s timer and after an earlier call has cleared +3A9h, so it comes only at a dive leader's second head change. Expect few or no resets, and a re-seed on every head change after the first. Moves are possible on any dive row where a hit notice precedes a head change.

**96, result: kDiveCruiseProfileGateBound ON, unexercised.** Exes built at 749351fae (`local\p23e_off`, `local\p23e_on`). BSM09 d1, BSM09 with no orders, USN04 and USNOS `l65_os_c1` (`local\p23e_{off,on}_{b9,b9n,u04,os}.log`) are all gameplay identical. No summary line printed on any of them, so no row reset a set travel altitude or re-seeded a clock below 3600. BSM09's D3A Val travel altitudes stay set: no dive leader reaches the second head change, as predicted. Flipped by mechanism; it is faithful to the bytes and unexercised on these rows.

## 97. cc13's device walk, per rack (cc9-physics23, 2026-10-19)

kPlaneDeviceWalkPerRackBound (include/bsp/cc13_devwalk.hpp, docs/CC13_DEVWALK.md; 007C0D90 per rack in list order). The exes are pair_export builds of 7f9207dc6, which contains 9df6eecb9: `local\p23h_off` and `local\p23h_on` (PerRack only; kPlaneDeviceWalkSharedDrawBound stays OFF until cc13's follow-up). Logs are `local\p23h_{off,on}_<row>.log`.
- BSM09 d1, USN04, USN01 `usn01_p8_u1_i` and USNOS `l65_os_c1`: gameplay identical (BSM09 still EndMission 327.04 s). On these rows every rack on a plane is one class, as cc13 predicted. 007C0D90 is reached: Plane::droppable_device_walk goes from unimplemented to concrete.
- USN21 `s46_u21_f`: MOVED, deaths 576 -> 574, dive-bomb releases 13 -> 18, torpedo drops 8 -> 6. The first divergence is JakeTmpl, a kind-10h floatplane on a depth-charge task with 4 single racks. Its squadron spends its ordnance and goes RTB at 116.60 s instead of 117.50 s (`spent ordnance ... latched`). The walk now fires each plain rack in list order with the level delay in one pass, where the plane-wide arm fired one per issue. That is cc13's "kind-10h planes with mixed racks" case, and everything after it is RNG-coupled. The mission completes at 2916.72 s in both, guard logged.
- No rocket (33h) carrier appears on any of these rows (no opportunistic-rocket decision with rocket_platforms > 0), so the rocket arm is unexercised.

Verdict: faithful by mechanism, to be flipped ON. include/bsp/cc13_devwalk.hpp is held by cc13-devwalk2 (the shared-draw follow-up); p23_devwalk_flip is queued behind it.

**97, flip.** kPlaneDeviceWalkPerRackBound is ON (cc13-devwalk2's lease freed and p23_devwalk_flip took the header). kPlaneDeviceWalkSharedDrawBound stays OFF until its pair.

## 98. cc13's follow-release seams and the goaway float stores (cc9-physics23, 2026-10-19)

The exes are pair_export builds of 2c395b950, which contains 7f9207dc6 and 4943076ca: `local\p23i_off`, `local\p23i_seam` (kFollowReleaseSeamsBound) and `local\p23i_gfs` (kDiveBombGoawayFloatStoresBound). Logs are `local\p23i_<build>_<row>.log`.
- **kFollowReleaseSeamsBound** (include/bsp/follow_release_seams_cc13.hpp; the 009C1FFD/009C2006 C25h||C20h arm in the other host copies of 009C1FD0). Reached on every row: the "follow release arm" applied count goes 29 -> 199 on USN01 `usn01_p8_u1_i`, 63 -> 157 on BSM09 d1 and 27 -> 203 on USN21 `s46_u21_f`. Gameplay MOVED on all three, RNG-coupled.
  - USN01: torpedo-task releases 4/9 -> 3/8, dive-bomb releases 5 -> 7, no end in both.
  - BSM09 d1: deaths 49 -> 50, dive releases 9 -> 6, torpedo drops 2 -> 3. It still wins, EndMission 327.04 -> 351.04 s.
  - USN21: deaths 576 -> 574, dive releases 13 -> 15. It completes at 2916.72 s in both.
  - No draw (the arm takes no 00BD2F10). Faithful by mechanism: **ON**. cc13's drop-kamikaze gating follow-up is still to come; the header states the host's 009AD1F0 path already calls the base only on the image's arm.
- **kDiveBombGoawayFloatStoresBound** (include/bsp/cc13_tier2_switches.hpp; 009C7F00's travel*0.9f, the reload product and ceiling-100 stored as float). Identical on USN01 u1_i, BSM09 d1 and USN21 (pair_diff exit 0), as expected away from the float boundary. Faithful by mechanism: **ON**.
- The 006082D0 comment fix routed with these (007BC7A0's rocket pass: COMISS 0,[desc+E0h] / JAE 007BC86C skips the attitude gate when E0h <= 0) waits for the next units.cpp commit. Other lanes are queued on that file.
- WINS: BSM09 d1 re-times 327.04 -> 351.04 s under the seams (sent to cc9-lua70).

## 99. The device walk's shared draw; HANDOFF (cc9-physics23, 2026-10-19)

**kPlaneDeviceWalkSharedDrawBound ON.** With the switch on, the next-rack delay draw takes 00BD2F10 on shared stream 1 in the image's call order. The exes are pair_export builds of 5eab32b9e, which contains 18eaf4acc, so both sides have PerRack ON and the restored issue counters: `local\p23j_off` and `local\p23j_on`. Pairs ran in the reference env and in the default env without BSP_GUNNERY_RNG_STREAMS (`local\p23j_{off,on}_<row>[_def].log`). Every row moves, as a change in draw order must; the per-entity rows are RNG-coupled.
- BSM09 d1: ref still wins, 351.04 s in both. Default env wins in both, 330.04 -> 516.00 s.
- USN01 `usn01_p8_u1_i`: no end in either env (ref deaths 37 -> 36; default 30 -> 33, torpedo drops 10 -> 11).
- USN21 `s46_u21_f`: ref completes 2911.72 -> 2921.72 s, default 2961.76 -> 2956.76 s.
Faithful by mechanism (the image's draw is the shared generator), so it is flipped. WINS: BSM09 d1 and USN21 re-time slightly (sent to cc9-lua70).

**Merge note.** cff560b45/18eaf4acc set PerRack ON in cc13's header, and my 917c0b57c set it too. The merge at 5eab32b9e kept main's line.

### HANDOFF

The lane is PHYSICS / PLANE FLIGHT, in tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1). Launcher: `local\p23_pair.ps1 -Tag <export dir> -Row <b9|b9n|u04|u21f|os|u1g|u1i|j6> [-EnvMode def]`. ON this session: kLandBlendFloatOpsBound, the three cc13 squadron switches (95), kDiveCruiseProfileGateBound (96, unexercised), PerRack (97), kFollowReleaseSeamsBound and kDiveBombGoawayFloatStoresBound (98), SharedDraw (99). The ConTBD3 stow works on main (94). The USN01 Convoy2 loss is kFollowReleaseArmBound (lua69's bisect); cc13 is auditing it.

Open, from the lead's queue file (scratchpad cc9lead_physics_queue.txt):
1. 4a kPlaneFlightStateNotifyCc13Bound (land chain, main b47417a6a, include/bsp/plane_land_chain_cc13.hpp). Pair on carrier rows with landings: USN21, USN04, BSM09.
2. 4b kPlaneRetireOnDestroyedDrawCc13Bound + kPlaneCombatDeathRetireDrawCc13Bound. Wait for kPlaneFunnelKillDestroyOnlyCc13Bound, or pair in the ref env only.
3. 4c kPlaneGroundStateDeadTailCc13Bound (007CA50F). Check reach first.
4. 4d kDogfightFireEnvelopeBound + kDogfightFireHoldArmBound (main c8f654aeb, include/bsp/cc13_fire_envelope.hpp). Dogfight rows USN21, BSM09, JM06, in both envs.
5. Coming: kTaskGunBlockReconstructBound, kMoveToReleaseFollowExitBound, kPlaneFunnelKillDestroyOnlyCc13Bound, the paratroopers on JM08, the 006D2510 airfield gate, and cc13-prop's 009C7F00 goaway-rounding switch.
6. Text only, in the next units.cpp commit: player_bomb_release_006082d0's comment says the rocket pass applies with E0h > 0. The bytes are 007BC7CD COMISS 0,[desc+E0h] / 007BC7D4 JAE 007BC86C, so the attitude gate is SKIPPED when E0h <= 0. units.cpp had three cc13 waiters when this was written.
7. Low, unchanged: the 009C8A42 +4B0h NaN max, the 009C4A40 ceiling NaN arm, and section 75's whole-frame item.

## 100. cc13's dogfight fire envelope and the per-task gun block (cc9-physics24, 2026-10-19)

All exports are pair_export builds of 19dcd5aba (contains d6c0c5ccd): `local\p24_off`, `local\p24d_on` (kDogfightFireEnvelopeBound), `local\p24h_on` (that plus kDogfightFireHoldArmBound), `local\p24e_on` (kTaskGunBlockReconstructBound), `local\p24f_on` (that plus kTaskGunBlockBurstTimerResetBound, which static_asserts the first). Launcher `local\p24_pair.ps1` (p23's rows plus `b1` = BSM01 bsm01_l50_b1_a, `j8` = JM08 jm08_l65_j8_p2). Logs `local\<tag>_<row>[_def].log`; `_def` is the default env without BSP_GUNNERY_RNG_STREAMS.

**kDogfightFireEnvelopeBound ON, kDogfightFireHoldArmBound ON.** No direct draws; the burst edges reuse "#dfgb", so both envs were paired.
- BSM09 d1: ref still ends, 351.04 -> 348.04 s (hits 1454 -> 1384, dive releases 6 -> 3); default still ends, 516.00 -> 462.01 s (deaths 75 -> 71).
- USN21 s46_u21_f: completes at the same time in both envs (2906.71 / 2891.70 s); shots 89620 -> 84812 (ref). Per entity the player's leader F6F_1 dies instead of F6F_2 (ref), RNG-coupled.
- BSM01 l50_b1_a and JM08 l65_j8_p2: neither side ends within 1200/1500 s in the ref env (the BSM01 row did not reproduce its 767.94 s win at 19dcd5aba OFF either). JM08 default env: OFF no end, ON fails at 1088.99 s ("The HQ has been destroyed!"), default-env RNG coupling on a row that is not a win.
- JM06 s45_j6_k: identical in both envs (no bursts there, as CC13_FIRE_ENVELOPE section 8 says).
- Hold arm (p24d_on vs p24h_on): identical on BSM09, BSM01, JM08 and USN21 in both envs; hold_ahead = 0 everywhere, so D/E is unexercised. JM08 shows re-aims (C) of 6 to 16 per fighter.
Faithful by mechanism (the listing's envelope and gates); flipped. No WINS row is lost.

**kTaskGunBlockReconstructBound ON, kTaskGunBlockBurstTimerResetBound ON.**
- Reconstruct (p24_off vs p24e_on): BSM09 ref moves (deaths 50 -> 49, shots 14149 -> 13846, still ends 351.04 s); default still ends, 516.00 -> 459.01 s. USN01 u1i (77 resets) and USN21 (1089 resets) are gameplay identical in the ref env.
- Burst timer reset (p24e_on vs p24f_on): gameplay identical on USN01 u1i, BSM09 and USN21 in both envs.
Faithful by mechanism; flipped.

The two flips were paired separately against the same OFF; the combined state is checked by the post-flip confirmation runs below.
## 101. cc13's torpedo release timer (cc9-physics24, 2026-10-19)

Exports of main c01bcb8fb: `local\p24n_off`, `local\p24h1` (kReleaseTimerReseedDrawBound), `local\p24h2` (that plus kReleaseTimerPredicate007BB110Bound). Rows BSM01 bsm01_l69_b1_d, USN01 u1i, USN21 s46_u21_f, BSM09 d1, both envs.

**kReleaseTimerReseedDrawBound ON.** One stream-1 draw per timed fire, so every row moves. BSM09 still ends (ref 351.04 s both sides; default 444.02 -> 441.02 s); USN21 still completes (default 2911.72 -> 2926.73 s); BSM01 default drops 35 -> 49 (no end on either side, see below); USN01 u1i default LOSES its completion (OFF completed 490.00 s, ON no end, deaths 53 -> 30), RNG-coupled: the ref env has no end on either side. Faithful by mechanism; flipped.

**kReleaseTimerPredicate007BB110Bound stays OFF (mechanism failure as built at c01bcb8fb).** The predicate refuses while the bay channel C is enabled and below 1.0 with a round in the bay, and in that build only 007BBBA0 itself opened the bay, so a bay-door torpedo plane never released: BSM09 ref timer_blocked_007BB110 = 378, timer_fires = 0, torpedo drops 3 -> 0 and the mission is LOST (Game Over at 1188.68 s); USN21 default blocked 169, drops 5 -> 0. BSM01 and USN01 are identical (no bay-door rounds). cc13 has since bound the image's bay opener (kReleaseBayOpenCc13Bound, main 14deb8770, OFF, static_asserted by the predicate); the predicate is to be re-paired over it.

**BSM01.** bsm01_l69_b1_d has no end within 1200 s at 19dcd5aba and c01bcb8fb OFF, in both envs (MiniSub never dies); the WINS row won at ec0df436c. Sent to cc9-lua70.
## 102. Three older OFF switches found live by cc13's stub census (cc9-physics24, 2026-10-19)

Exports of main 677ee345f: `local\p24b_off`, `local\p24g1` (kFollowSightSearchBound), `local\p24g2` (kDiveHitClockBound), `local\p24g3` (kDiveBombMoveToOriginBound), each alone. Rows USN01 u1i (ref), BSM09 d1 and USN21 s46_u21_f (both envs). All three are in src/game_hosts_units.cpp and are flipped ON by mechanism; every win holds.
- kFollowSightSearchBound (009C211D-009C232A): reached on BSM09 only (98 searches, 96 found, 283 held ticks); BSM09 ends earlier (ref 348.04 -> 327.04 s, def 462.01 -> 408.02 s). USN01, USN21 and BSM01 have 0 searches and are identical.
- kDiveHitClockBound (009C4AA4 reads the carried approach+C4h): USN01 dive releases 7 -> 5, USN21 ref 11 -> 15 (same completion), BSM09 and USN21 default identical.
- kDiveBombMoveToOriginBound (009C18C0 origin separation, 009F9E40 heading): USN01 7 -> 5, BSM09 ref ends 348.04 -> 327.04 s (dive releases 3 -> 6), default still ends 462.01 s; USN21 completes in both envs (default 2891.70 -> 2856.66 s).
## 103. cc13's bay opener and the real 007BB110 (cc9-physics24, 2026-10-19)

Exports of 3df710613 (reseed ON, main d4258a897): `local\p24q0`, `local\p24q1` (kReleaseBayOpenCc13Bound), `local\p24q2` (that plus kReleaseTimerPredicate007BB110Bound). Rows BSM09 d1, USN01 u1i, USN21 s46_u21_f, both envs. At this base BSM09 ends at 282.05 s (ref) / 405.02 s (default) and USN01 u1i completes at 1150.00 s / 1004.88 s.
- Bay opener alone: gameplay identical on every row in both envs.
- Predicate over it: identical on BSM09 and USN01 (timer_blocked_007BB110 = 0); USN21 ref identical, USN21 default moves (2 blocks, drops 6 -> 3, completes 2911.72 -> 2921.72 s). The section-101 deadlock is gone.
Both flipped ON by mechanism. kReleaseTimerNotify0DhBound (log only) is left OFF.
## 104. cc13's land chain and the funnel kill (cc9-physics24, 2026-10-19)

**First pairs** (exports of 19dcd5aba / c01bcb8fb, ref env unless noted): kPlaneFlightStateNotifyCc13Bound alone was gameplay identical on BSM09, USN04 and USN21 (162 / 45 / 1440 notifies). kPlaneGroundStateDeadTailCc13Bound is unreached (skips = 0 on BSM09, USN04, USN21). kPlaneFunnelKillDestroyOnlyCc13Bound (`local\p24k1` vs `local\p24n_off`) moves BSM09, USN04 and USN21 slightly through cause-2 removals (12 D3A at 42.75 s on BSM09, movieval on USN04, JakeTmpl on USN21); BSM09 still ends (ref 351.04 s, default 444.02 -> 408.02 s), USN21 completes in both envs. Over FunnelKill, kPlaneRetireOnDestroyedDrawCc13Bound (`p24k2`) is identical everywhere (destroy_draws = 0, unreached) and kPlaneCombatDeathRetireDrawCc13Bound (`p24k3`, 48 / 58 / 560 draws) is identical in the ref env and moves the default env slightly with every win kept.

**Re-pair after cc13's lift-off fold (7f45a702a)**, exports of 59c0ae06b: `local\p24r0` (OFF), `local\p24r1` (the notify alone), `local\p24r2` (all five). The notify alone stays gameplay identical on BSM09 (194 / 263 notifies), USN04 (60) and USN21 (1920) in both envs. All five together: BSM09 ref identical end (285.05 s); USN21 completes in both envs (ref 2906.71 -> 2901.71 s, default 2916.72 -> 2921.72 s); USN04 moves (no end either side). **BSM09 default env is LOST**: Game Over at 141.05 s, because the player's Donald collides with a D3A Val in a crash sweep at 140.95 s (no damage; the stream-0 draw order moved the dogfight). RNG-coupled in the default env; the reference env keeps the win.

All five are flipped ON by mechanism (cc13's listing reconstruction; the lead's rule that a faithful fix flips even if a win is lost).

**4f paratroopers stay OFF (unreached).** Exports of 677ee345f, `local\p24s1` (kLevelBombParatrooperDropBound), `p24s2` (plus its draw), `p24s3` (plus kParatrooperRoundBound). The C47 squadrons spawn only at about 1318 s, and every JM08 plan we have loses the HQ before any C47 releases: l65_j8_p2 OFF fails at 1676.56 s in a 3000 s run, and option (1) ON brings the failure to 1476.36 s (the C47s now fly the attack and draw AA off the B-25/B-17). drops = 0 on l65_j8_p2, s45_j8_m10, p6_j8_f, l64_j8_e and stock JM08 (stock spawns no C47). S2 and S3 are identical over S1. JM13 has no paratrooper units. CORRECTION: these exports (677ee345f) predate cc13's parafix (main f10dce645, the 007BCCF0 release point from pose forward x y/2.8 instead of velocity), so drops = 0 is that binding bug, not reach. Object 154 is Headquarter 01 and target_valid is only position_valid. The HQ loss is US ship shelling on a knife edge (OFF keeps 62.4 of about 5950 HP), not the paratroopers. Re-pair JM08 l65_j8_p2 on main f10dce645 or later, both envs, expecting drops and landed > 0 and min_miss under 1 km.
## 105. HANDOFF (cc9-physics24, 2026-10-19)

Lane PHYSICS / PLANE FLIGHT, tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1). Launcher `local\p24_pair.ps1 -Tag <export dir> -Row <b9|b9n|u04|u21f|os|u1g|u1i|j6|b1|j8|j8L|j8n|j8m|j8f|j8e|j13> [-EnvMode def]`; `b1` is now bsm01_l69_b1_d. Commits this session: fbdcdc7fc (006082D0 comment), fb31ab760 (4d envelope + hold arm, 4e gun block + burst reset; section 100), 3df710613 (release timer reseed; 101), 96ed1be14 (4g FollowSightSearch, DiveHitClock, DiveBombMoveToOrigin; 102), e0e95ccdd (bay opener + 007BB110 predicate; 103), 282567f82 (land chain 4a/4b/4c + funnel kill; 104).

Open (from the lead's queue file, cc9lead_physics_queue.txt):
1. 4f paratroopers: re-pair JM08 on main f10dce645 or later (my runs predate the parafix; see 104). Pair option (1) first, then the round and its three draw sub-switches, both envs; send capture evidence to the ships lane.
2. kReleaseTimerNotify0DhBound (log only) left OFF; kPlaneFunnelKillPathTagCc13Bound (OFF; part of 4b, pair over FunnelKill, JM02 PlayerBetty is a direct check) not paired. Also open: 4i torpedo goaway draws, 4j steer gate, 4k takeoff notify, 4L goaway target term (IJN08 level term first), 5a kTorpedoMoveToRangesBound (one USN21 ON run, glide census), and the USNOS landed=0 / slot_tail_unapplied check with 4a ON.
3. Coming per the queue: the +68h commanded-direction steer gate (009FCCA4; USNOS/BSM09/BSM01), the torpedo-goaway entry and tick (4 stream-1 draws, both envs), 007C17D0 NotifyTakeoffStart and SlowTakeoff +AA0h, the level-bomb CanFire 00729A80 (only if live), kAirOpsLaunchFlagAirborneBound (CHG06 only, low), the 006D2510 airfield gate.
4. WINS: BSM01 bsm01_l69_b1_d has no end within 1200 s from 19dcd5aba on (sent to cc9-lua70). BSM09 default env is lost at 141.05 s after 282567f82 (RNG-coupled collision); USN01 u1i default lost its completion after 3df710613.
5. Low, unchanged: the 009C8A42 +4B0h NaN max, the 009C4A40 ceiling NaN arm, and section 75's whole-frame item.
Disk: the 3000 s JM08 logs (*_j8L.log, over 1 GB each) were deleted after reading; do not keep long JM08 logs.## 106. Goaway target terms (dive, level), torpedo moveto ranges, moveto speed shaping; paratroopers blocked (cc9-physics25, 2026-10-19)

Exports of origin/main cb3192296, reference env: `local\p25_off`, `local\p25_dive` (kDiveGoawayTargetExtentCc13Bound), `local\p25_lvl` (kLevelGoawayTargetExtentCc13Bound), `local\p25_tmr` (kTorpedoMoveToRangesBound), `local\p25_mss` (kMovetoSpeedShapingCc13Bound), `local\p25_para` (paratrooper drop + drop draw + round). Launcher `local\p25_pair.ps1`; term totals from `local\p25_goaway_sum.py`.
- **Dive target term, flipped ON.** USN01 u1i moves (8 dive enters, the term moves 5; dive releases 3 -> 6; completes 1195.04 -> 1069.92 s). JM08 l65_j8_p2: gameplay identical (10 enters, all moved by the term). BSM01 bsm01_l69_b1_d: identical, no dive enters (ends by EndMission, nil status, at 812.93 s). USN21 s46_u21_f: identical (completes at 2941.74 s).
- **Level target term, flipped ON.** IJN08 p10_i08_z (3000 s) moves: level enters 328 -> 93, deaths 256 -> 206. The row was already lost OFF and still is (Game Over 2611.42 -> 2511.32 s).
- **kTorpedoMoveToRangesBound, flipped ON (identity).** USN21 ON: all 1409 "glide census" lines read commanded=500.0; gameplay identical.
- **kMovetoSpeedShapingCc13Bound, flipped ON (unexercised).** USN21 ON: "summary torpedo moveto sep" n=68352, mean_dsep 134.78, max_dsep 243.71, speed_changed=0, max_dspeed=0; gameplay identical.
- **Paratroopers (4f): blocked.** On cb3192296 the JM08 HQ falls before the C47 wave spawns (about 1318 s): l65_j8_p2 fails at 1031.93 s and lua70's l70_j8_j at 981.93 s, OFF and ON alike; drops/landed/soldiers/killed = 0. It needs a JM08 plan that holds the HQ past 1500 s (main drift, on the lead's drift list).
## 107. Torpedo goaway target term and the dive goaway draws (cc9-physics25, 2026-10-19)

Exports of 5481a7ca5 (section 106's flips over origin/main cb3192296): `local\p25b_off`, `local\p25b_torp` (kTorpGoawayTargetExtentCc13Bound), `local\p25b_dd` (kDiveGoawayDrawsCc13Bound with the standoff, window and countdown draws). Rows USN01 u1i, BSM01 bsm01_l69_b1_d, JM08 l65_j8_p2, USN21 s46_u21_f. The torp term ran in the reference env, the draws in both envs.
- **Torpedo target term, flipped ON.** BSM01 moves a lot: OFF, Jap #3.1 re-enters goaway 39 times (151 torpedo enters in all), against 53 enters ON, because the standoff now clears the target's 1500-2000 m AA. OFF ends with EndMission (nil status) at 812.93 s after MiniSub and Donald die; ON has no end within 1200 s (torpedo releases 9 -> 32). USN01 still completes (1069.92 -> 1049.90 s). JM08 is still lost (1002.93 -> 1010.93 s). USN21 is identical.
- **Dive goaway draws, flipped ON.** These are stream-1 draws, so every row that reaches them moves. USN01 completes later (ref 1069.92 -> 1280.12 s, default 1054.90 -> 1305.15 s). JM08 ref is still lost (1002.93 -> 1008.93 s); JM08 default goes from no end within 1500 s to failed at 1088.99 s, which is RNG-coupled and on an already-open row. USN21 ref completes at 2941.74 s on both sides with small death-row moves, and USN21 default is identical (2946.75 s). BSM01 is identical in both envs (no dive goaway).
- Why BSM01 has no end with the torp term ON: MiniSub is never killed. OFF (`p25b_off_b1`), HenryPT kills it at 685.81 s (killer_gun 0, range 57 m, hull water to 0 health at 685.86 s), and that is the 812.93 s EndMission win. ON (`p25b_torp_b1`), the run has no MiniSub death row in 1200 s. The torpedo releases rise from 9 to 32 and the fight moves (the HenryPT kill does not happen); the logs do not say which change keeps HenryPT off MiniSub. The win is lost because MiniSub survives, not because someone else kills MiniSub or Donald.
- On this base the JM08 l65_j8_p2 HQ falls at 1002.93 s, against 1031.93 s on cb3192296; the level term from section 106 moved it.
## 108. Pilot-bot config load and the torpedo goaway draws (cc9-physics25, 2026-10-19)

Exports of 39ac16569: `local\p25c_off`, `local\p25c_pbc` (kPilotBotConfigLoadBound), `local\p25c_ent` (that plus kTorpGoawayEnterDrawsBound and its distance, climb and window-delay draws), `local\p25c_tick` (that plus the two tick draws). Rows USN01 u1i and JM08 l65_j8_p2, both envs (pbc in the reference env only).
- **kPilotBotConfigLoadBound, flipped ON.** Gameplay identical on both rows. The only change is 4 more MissionLua::open_script calls. The log reads "loaded=1 script reads=4 misses=0; TorpFlikFlakTime Stun 20,30 SPNormal 12,22 SPVeteran 5,8 ...".
- **Enter draws, flipped ON.** USN01 still completes (ref 1195.04 -> 1270.11 s, default 1139.99 -> 1300.14 s). JM08 ref is still lost (1025.93 -> 1033.93 s); JM08 default has no end on either side.
- **Tick draws, flipped ON.** USN01 ref ends at 1270.11 s on both enter and tick, with a small difference in shot counts; USN01 default completes at 1280.12 s. JM08 ref tick is the same as enter (the tick draws are unreached there); JM08 default moves and has no end on either side.
- Every move is RNG-coupled; no row changes its outcome.
## 109. Flak second-ammo range (cc9-physics25, 2026-10-19)

Exports of 3ca24854e: `local\p25d_off`, `local\p25d_flak` (kFlakSecondAmmoRangeCc13Bound, src/game_hosts_gunnery.cpp, on loan for this one commit). Reference env. As predicted it moves every row that has category-6 flak, since AA reach grows from 1500-1600 m to 2000 m:
- USN01 u1i completes earlier (1270.11 -> 1155.00 s).
- BSM09 d1 ends later (EndMission 285.05 -> 405.02 s).
- USN21 completes slightly earlier (2886.69 -> 2876.68 s).
Flipped ON by mechanism. BSM01 is left out: from 39ac16569 on, the row spawns only 284 of its 663 units and has no hits (an origin/main regression in 5481a7ca5..39ac16569, reported to the lead).
## 110. Takeoff notify, the ShipYardLaunch +AA0h and the slow push; USNOS carrier landings (cc9-physics25, 2026-10-19)

Exports of 3ca24854e: `local\p25d_off`, `local\p25d_tk1` (kPlaneTakeoffStartNotifyCc13Bound), `local\p25d_tk2` (that plus kPlaneShipyardLaunchAa0Cc13Bound), `local\p25d_tk3` (that plus kPlaneShipyardLaunchSlowPushCc13Bound). Rows USNOS l65_os_c1 (1500 s) and USN21 s46_u21_f. tk1 and tk2 ran in the reference env, tk2 and tk3 in both envs.
- **Notify, flipped ON.** 007C17D0 goes from unimplemented to concrete with 480 calls on USN21, and USN21 gameplay is identical (completes 2886.69 s). USNOS moves (deaths 282 -> 289, units 617 -> 632) and has no end on either side.
- **+AA0h, flipped ON (unexercised).** It is identical to the notify alone on both rows.
- **Slow push, flipped ON (unreached).** It is identical over +AA0h on both rows in both envs: USNOS has no end; USN21 default completes at 2881.69 s.
- **USNOS carriers still never land (queue item 7).** With the land chain ON, the 3ca24854e OFF run has Essex landed=0 slot_tail_unapplied=15 and Intrepid landed=0 slot_tail_unapplied=16. Enterprise has landed=0 slot_tail_unapplied=9 with queue=3 records=6 inserts=2: sqn01, sqn02 and sqn36 queue at 1400-1401 s but never touch down. The airfields do land (Airfield1 74, Airfield3 508). Open; it is not caused by the takeoff switches.
## 111. The funnel kill path tag; JM02 crashes; USN21 regressed (cc9-physics25, 2026-10-19)

Exports of 8ca49d230: `local\p25e_off`, `local\p25e_tag` (kPlaneFunnelKillPathTagCc13Bound over the FunnelKill destroy-only test). Reference env.
- **Path tag, flipped ON by mechanism.** USN04 is identical. BSM09 d1 has the same end (405.02 s) and the same death rows; one plane death mode drops out (Yorktown_sqn16|.-II, an explosion_delayed at 381.08 s OFF). USN21 moves slightly (Fujinami, Hayashimo and Akishimo survive ON) and has no end on either side.
- **JM02 (stock, no orders) crashes with 0xC0000374 (heap corruption)** at the start, right after GenerateObject "Buffalo 01" (id 31). It does so OFF and ON, on every base back to cb3192296 (p25_off, p25b_off, p25c_off, p25d_off, p25d_tk1, p25e_off). So the PlayerBetty direct check could not run. Reported to the lead.
- **USN21 s46_u21_f has no end at 8ca49d230 OFF** (it completed at 2886.69 s on 3ca24854e). The cause is in 3ca24854e..8ca49d230, which includes the ships61 astern skip / arc-centre test, kSceneHiddenSpawnCarryCc13Bound, the cc15 loading queue and app preload, and this lane's flak and takeoff flips. Those flips were paired on 3ca24854e and kept USN21's completion there. Reported to the lead. The lead attributes this to kSceneHiddenSpawnCarryCc13Bound ON (872adeff3, SHIP_AI 313): Intrepid and Enterprise cruise east into the Center Force, as authored. gunnery56 is re-planning USN21, so no bisect was needed. The path-tag pair's Fujinami, Hayashimo and Akishimo survivors are on this already-lost base.
## 112. The attack-target switch is unreached; USNOS confirm; the steer gate (cc9-physics25, 2026-10-23)

All exports are of main 802e1d8b9 at difficulty 1 (the run_game.ps1 pin), reference env. Logs without "present failed" only.
- **kUnitGetAttackTargetDirectorKindCc13Bound stays OFF (unreached as built).** `local\p25i_off` vs `local\p25i_atk`: USN13 (Game Over 290.05 s, 0 calls), USNRM01 (704.95 s), IJN20 (no end) and JM05 (569.98 s) are identical, with the same attack-target summary on both sides (IJN20: 2974 calls, fire 2895, command 79, pushed 0). The ON test `unit_is_kind_of(index, 0x18)` never matches because the host resolves a squadron's Lua entity to its leader plane's slot (e.g. "Airfield1_sqn06" is unit 655, plane type 38), which is not kind-of 0x18. IJN20's airfield level bombers reinstall 28-89 times each with 0 releases, which is the host gap. Routed to cc13 through the lead.
- **USNOS 1800 s confirm (item 7) did not reach the window.** `local\p25i_off_osL` (l65_os_c1) fails with Game Over at 1099.95 s (HQ1). The carriers land nothing (landed=0; slot_tail_unapplied 0/7/25 on Enterprise/Essex/Intrepid) while Airfield3 lands 362. Intrepid and Essex squadrons queue at 380-523 s, but none touches down. This needs a plan that holds HQ1 past about 1600 s.
- **Steer gate, flipped ON** (kBotCommandedDirection68Bound plus kSteerGateCommandedDirectionBound; `local\p25j_pred`, `local\p25j_gate`). Prediction mode (store only) is gameplay identical. BSM09 predicts to_pass 42 and to_block 799, and USN01 predicts 0 / 0. With the gate ON, USN01 is identical (completes 1155.00 s; 109 gates, no flips). BSM09 still ends, later (EndMission 405.02 -> 429.02 s; to_pass 33, to_block 247; dive releases 9 -> 3). USNOS moves (to_pass 15, to_block 57) and still fails at 1099.95 s. `tools/off_switch_on_build.py` built both ON cleanly.
## 113. HANDOFF (cc9-physics25, 2026-10-23)

Lane PHYSICS / PLANE FLIGHT, tree J:\PROG\battlestations-pacific-decompile-cc9-physics1 (branch agent/cc9-physics1). The launcher is `local\p25_pair.ps1 -Tag <export dir> -Row <row> [-EnvMode ref|def|j203]`. `j203` is the reference env plus BSP_FRAME_JITTER=20,3. Rows include `u13`, `rm1`, `i20`, `j5`, `j2`, `c6` (CHG06, does not load), `osL` (USNOS 1800 s) and `j8j` (lua70's JM08 j). Helpers: `local\p25_d.ps1 <off> <on>` (short pair_diff), `local\p25_goaway_sum.py <logs>` (goaway target-term totals), `local\p25_atk.ps1 <row>` (attack-target summary, level-bomb installs). run_game.ps1 pins --difficulty 1. Launch with pwsh, not Windows PowerShell 5: under PS5, Get-FileHash fails and no manifest is written.

Commits this session (sections 106-112): 5481a7ca5 (dive/level goaway terms, torpedo moveto ranges, moveto speed shaping), 39ac16569 (torp goaway term, dive goaway draws), 3ca24854e (pilot-bot config load, torp goaway enter/tick draws), 052fd3356 (flak second-ammo range; loan of game_hosts_gunnery.cpp), 8ca49d230 (takeoff notify, +AA0h, slow push), 7207dcf49 (funnel path tag), 6d1051d37 (doc notes), 4b308a58d (steer gate on the commanded direction).

Open:
1. Paratroopers (4f): blocked. On current main the JM08 HQ falls before the C47 wave (about 1318 s) on l65_j8_p2 and l70_j8_j. This needs a JM08 plan that holds the HQ past 1500 s. Switches: kLevelBombParatrooperDropBound, its draw, kParatrooperRoundBound plus three draw subs.
2. kUnitGetAttackTargetDirectorKindCc13Bound: unreached as built (section 112). The ON test `unit_is_kind_of(index, 0x18)` fails because the squadron's Lua entity resolves to the leader plane's slot. Routed to cc13; re-pair on IJN20 (airfield bombers reinstall 28-89 times, 0 releases), USN13, USNRM01 and JM05 once it is fixed. IJN08 can't test it until bisect2 explains the phase-1 sub wipe-out (subs dead by 638.82 s on current main; 978-1216 s on cb3192296).
3. USNOS carrier landings (queue item 7): unconfirmed. l65_os_c1 now fails at 1099.95 s (HQ1) on 802e1d8b9; carriers have landed=0, and Intrepid/Essex squadrons queue at 380-523 s but never touch down. This needs a plan that holds HQ1 past about 1600 s to see Enterprise's approach.
4. kAirOpsLaunchFlagAirborneBound and kAirOpsLaunchFlagAltitudeDrawBound: OFF. `--menu-select CHG06` never loads the mission (sideMission; "no mission-detail page was built for published group 0"). This needs harness support for challenge missions.
5. Level-bomb CanFire 00729A80 (CC13_STUB_CENSUS row 7), started. 006E5800-006E581A calls rack->vtable[1D0h](1), i.e. CanFire with checkReload, on rack = ESI-310h; a refusal jumps to 006E58CF. The host (game_hosts_units.cpp, run_level_bomber_rack_tick_006e56f0) records it and tests ammo only. Next steps:
   - Confirm that the rack class's vtable slot 1D0h is 00729A80.
   - Find which writers set rack+414h (per-barrel timers), +450h, +478h, +3B8h and +358h in the rack fire path 006E4D50. If the drop never sets them, the gate is ammo-only, which is the host's substitution.
   - Then bind any live gate behind an OFF switch.
6. Still OFF in this lane: kReleaseTimerNotify0DhBound (log only). Low priority: the 006D2510 airfield gate, the 009C8A42 +4B0h NaN max, the 009C4A40 ceiling NaN arm, and section 75's whole-frame item.
7. kMovetoSpeedShapingCc13Bound went ON unexercised (USN21: speed_changed=0). If a later row exercises it, log its "summary torpedo moveto sep" line here.
8. NOT RUN (lead's order, next in this sequence):
   (a) The atktgt re-pair on cc13's fix, main 9961fbc7d or later. ON now also takes the command arm for a squadron stand-in (luawire_squadron_record). Rows: IJN20, JM05, USNRM01, USN13. Expect squadron_standin=N > 0 at the end of the attack-target summary; on IJN20, Airfield2_sqn34 arm=command, pushed > 0, and the reinstall loop gone. Flip by the rules if it holds. Run tools/off_switch_on_build.py and a full build before the flip commit.
   (b) The USNOS queued-squadron trace, from `local\p25i_off_osL.log` (802e1d8b9; Game Over at 1099.95 s). For Intrepid_sqn04 (queued 380.23 s), Intrepid_sqn07 (451.01 s), Intrepid_sqn09 (523.30 s) and Essex_sqn08 (494.90 s), record:
      - the fate after queueing (death time and killer, or alive at the end);
      - the mode/state and the distance to the carrier at death or at the end;
      - the slot or queue position;
      - whether the carrier was turning, moving or stopped.
      A squadron that is alive, unhurt and still queued at 1100 s is a landing-gate candidate for cc13.
   CanFire (item 5) comes after (a) and (b).
## 114. The attack-target switch goes ON; USNOS queued squadrons; the level-bomb rack CanFire (cc9-physics26, 2026-10-23)

Pairs are same-commit exports of 31e82488d (`local\p26_off`, `local\p26_atk` with
kUnitGetAttackTargetDirectorKindCc13Bound=true), reference env, difficulty 1, launched by `local\p26_pair.ps1`.
No log has "present failed"; the session is the console. physics25's "unreached" runs (section 112) were on
802e1d8b9, which predates cc13's fix 9961fbc7d (not an ancestor), so they did not test the stand-in arm.

**kUnitGetAttackTargetDirectorKindCc13Bound ON (reached, mechanism matches).** The attack-target summary
(`calls / fire_arm / command_arm / pushed / squadron_standin`), OFF -> ON:

| Row | OFF | ON | pair_diff | End OFF -> ON |
| --- | --- | --- | --- | --- |
| IJN20 idle | 2974 / 2895 / 79 / 0 / 2974 | 3817 / 0 / 3817 / 3676 / 3817 | 3 | no end both |
| JM05 idle | 1039 / 1016 / 23 / 57 / 980 | 924 / 53 / 871 / 900 / 871 | 3 | failed 530.99 -> 471.01 s (Troop Transport 02) |
| USNRM01 idle | 2722 / 2719 / 3 / 0 / 2722 | 4590 / 0 / 4590 / 3923 / 4590 | 3 | Game Over 849.92 -> 884.91 s |
| USN13 idle | 0 calls | 0 calls | 0 | Game Over 460.01 s both |

- IJN20: the airfield level bombers no longer reinstall. OFF, every Airfield1/2 bomber reinstalls 28-89 times with
  0 releases (Airfield2_sqn34 i89/i84/i85, 0 drops). ON, each installs once (i1), and Airfield2_sqn07's wingmen
  release (II: r1 d12, III: r1 d5). Airfield2_sqn34 is not spawned in the ON run (the airfield's pick order moves),
  so the "sqn34 arm=command" check is replaced by the whole-row counts above.
- WINS rows (row orders from cc9-lua52 `local\l72_rows.txt`):
  - PRCP01 `p9_p01_k`: **lost**. completed 1024.52 s -> failed 1038.39 s (`usn03.obj_p2_fail`, Enterprise sunk); standin 485.
  - BSM01 `s62_b1_h`: EndMission 815.92 -> 821.92 s (moved, still ends); standin 1831.
  - BSM09 `l68_b9_d1`: EndMission 429.02 -> 414.02 s (moved, still ends); standin 443.
  - JM06 `l70_j6_n`: gameplay identical (exit 1), completed 1922.75 s; standin 461.
  - USN01 `l69_u1_a` (completed 370.03 s) and LOMP06 `g56_l6_v2` (completed 330.84 s): identical, 0 calls.
  - USN02, LOMP10, USN12, ESMP08, IJN08, USN21, USN15 make 0 calls in lua72's latest logs, and IJN18 logs
    squadron_standin=0, so the flip cannot move them.
- The flip is faithful (the image's squadron director always takes the command arm; the record test is the
  labelled stand-in for IsKindOf(18h)), so it goes ON with PRCP01 lost; PRCP01 needs a re-plan.
  `tools/off_switch_on_build.py --names kUnitGetAttackTargetDirectorKindCc13Bound` exit 0; full build passes.

**USNOS queued squadrons (static, `local\p25i_off_osL.log`, 802e1d8b9).** Script `local\p26_osq.py`.
Every carrier squadron that queued died within seconds over the target area, 14-17 km from its carrier. None was
alive at 1099.95 s, so **there is no landing-gate candidate**.

| Squadron | Queued (n, deck) | Fate | Carrier distance at RTB |
| --- | --- | --- | --- |
| Intrepid_sqn04 | 380.23 s (n=0) | II 381.53 s (Heavy AA platform 09), III 385.73 s (HQ2), leader 391.23 s (Shimotsuke) | 17426 m |
| Intrepid_sqn07 | 451.01 s (n=0) | leader 460.76 s, II/III 462.91 s (all Shimotsuke) | 17134 m |
| Essex_sqn08 | 494.90 s (n=0) | III 484.06 s and II 490.75 s before the RTB (Zao1); leader 496.65 s (CB2) | 17058 m |
| Intrepid_sqn09 | 523.30 s (n=0) | II/III 532.29 s, leader 532.44 s (all Shimotsuke) | 16789 m |

- Each was alone in the queue (n=0). `squadron kill` erased its one queue record (00926D90 -> 006C4B30).
- The leaders were in land-task state 4C4 (cmd_alt 1450) and the wingmen were in follow (500). No touchdown
  probe got below 1000 m.
- Intrepid is moving, not turning. Its site x goes 10203.7 (380 s) -> 9496.0 (451 s) -> 8774.9 (523 s) at
  z about -9990, roughly 10 m/s westward.
- The later queuers die the same way: Intrepid_sqn11/13/22/30/33 and Essex_sqn31/34. Essex_sqn34 queued at
  1097.40 s and died at 1102.35 s (HQ1).
- The carriers' landed=0 is a consequence of flak over Shimotsuke and the HQs, not a landing gate.

**Level-bomb rack CanFire (CC13_STUB_CENSUS row 7): no gate can bite. The host's "has ammo" is exact for a
level bomber, and no switch is needed.**
- **The vtable slot.** 006E5800 loads `[rack]+1D0h` with rack = ESI-310h. The two MBombPlatform vtables are
  00CF96A8 and 00CF9918. Their slot 1D0h (00CF9878, 00CF9AE8) holds 006E3460, which is
  `Gun_CanFire(1) && this->vtable[210h](2Ah, 0)`. The slot 210h entries are 006E3FE0 (00CF98B8) and 006E45E0
  (00CF9B28), which ask whether an attached child round answers kind 2Ah. The loadout arm is skipped for arg 0.
- **The 00729A80 gates for a BOMBPLATFORM rack** (Function 0Ah):
  - Tests 5-7 are exempt. The artillery set is {2,3,4,6}, torpedo is 7, AA is {1,5,6}.
  - +358h has no non-zero writer. The constructor zeroes it at 0072E61A.
  - +3B8h is written only by BSP_Gun_ApplySetDestroyedFailure / ClearDestroyedFailure (0072B5BC, 007280C0) and
    by GUN_CREATE (0072A1BF).
  - +450h starts at 0 (0072E8D2 XORPS, stored 0072E8DB). Its only positive writer is BSP_Gun_Fire's tail at
    00730A08, and a rack's slot 1D8h is 006E4D50, not Gun_Fire.
  - +478h starts at 0 (Construct 0072E66C). Its only positive writer is SetTriggerHeld 0072D3B8 (slot 1E8h), and
    the rack overrides slot 1E8h with the empty body 006E3CB0 (00CF9890, 00CF9B00).
  - The +414h barrel timers start at 0 (0072EE99 FLDZ -> 0072CF00). Only RearmBarrel (from Gun_Fire and
    SetTorpedoStock) writes a reload. SelectFireMode writes FLT_MAX, and only on a mode change with no round.
  - The drop 006E4D50 (to RET 0Ch at 006E56ED) writes none of +3B8h/+358h/+414h/+418h/+450h/+478h. It reads
    +484h at 006E55E7.
- **Verdict.** CanFire reduces to "the rack has an attached kind-2Ah round". The drop re-attaches the next round
  through slot 224h (006E55FA), so this is the host's ammo test.
- **Uncertainty.** A destroyed-failure message aimed at a plane's rack would set +3B8h. No plane-side sender was
  checked.


## 115. Airborne launch ON (CHG06); JM08 plan h holds the HQ but spawns no C47 (cc9-physics26, 2026-10-24)

The exports are of 997fe5044 (main after 04cc91447, where cc15 turns on kChallengeGroupPageCc15Bound): `local\p26b_off`,
`local\p26b_air` (kAirOpsLaunchFlagAirborneBound + kAirOpsLaunchFlagAltitudeDrawBound), `local\p26b_s1`
(kLevelBombParatrooperDropBound) and `local\p26b_s6` (all six paratrooper switches). The env is ref and no log has
"present failed".

**kAirOpsLaunchFlagAirborneBound and kAirOpsLaunchFlagAltitudeDrawBound are ON (reached).** `--menu-select CHG06` now
loads chg_6_forcez.lua, whose forcedDifficultyLevel 2 overrides the pin.
- ON logs four "airborne launch for Hermes (006C5050 flag arm, State 7)" lines. They come from
  LaunchAirBaseSlot(AlliedCV, 1..4, true), with y = Ceiling + draw = 1631.91, 1602.52, 1723.02, and so on. OFF logs
  none.
- Both runs fail at 986.88 s ("You have failed to destroy the battleships!", Prince of Wales). Each logs one
  commandhelpers.lua:330 line, the same on both sides.
- pair_diff gives 3 (moved). Deaths go 56 -> 51, shots 13164 -> 130 and hull hits 692 -> 9.
- The cause: OFF, the Hermes squadrons start on the deck and fly over the airfield, so AirField's Heavy AA 01-03 fire
  (14-16 shots each) and the airfield buildings die. ON, they start airborne at the carrier's x/z and never come
  within 5 km of that AA (nearest 101 -> 5295 m). That is the mechanism the image's flag arm describes, so both
  switches flip.
- `tools/off_switch_on_build.py` gives exit 0, and the full build passes.
- WINS: LaunchAirBaseSlot is called 0 times in every WINS log checked, so no row can move:
  - lua72's l72r_*_{m,main}_ref (b1, b9, e8, i08, i18, j6, l10, l6, p01, u01, u02, u12, u21);
  - this session's p26_off_w*;
  - USN15 l71r_u15i.

**JM08 plan h (ships62 `s62_j8_h`, copy `local\p26_s62_j8_h.txt`; 72000 frames) on 997fe5044: the HQ holds, but
the paratrooper path is still unreached.**
- OFF (`local\p26b_off_j8h`) and S1 (`local\p26b_s1_j8h`) have no end through 3600 s; the HQ never falls. They are
  gameplay identical (exit 1).
- No C47 spawns, and neither does any US bomber. The only plane types created are:
  - Japanese: MainAirFieldEntity 01 (types 152, 154, 166 and 167), Hosho (162), Gekko, Ki-43 and the H6Ks;
  - three Wildcats (133).
- `summary mission capture paratroopers adds=0` and `units_with ... paratrooper=0` on both sides.
- The plan's own "C47 .." targets are refused with "no unit". The run therefore does not contain the ~1318 s US air
  wave that physics24's runs saw (section 104).
- The S6 run was cancelled because S1 was identical.
- Next: find what spawns JM08's US air wave (the mission script; LST 01 dies at 1275.27 s to the HQ's guns, which may change
  the phase), then re-pair S1 and S6.


## 116. CHG06 re-pair with the party switch ON; PRCP01 re-plan (cc9-physics26, 2026-10-24)

**CHG06 with kLaunchedSquadronPartyCc13Bound ON on both sides.** This is a temporary build because cc9-party1's
flip is not on main yet; the lead asked for it. Exports of 50b1127e3: `local\p26c_poff` (party ON, airborne switches
OFF) and `local\p26c_pair` (party ON, airborne switches ON).
- The airborne arm is still reached: 4 "airborne launch for Hermes" lines ON, 0 OFF.
- Both sides fail at 986.88 s and log one commandhelpers.lua: line.
- pair_diff gives 3, but only through distances. Deaths (51), hits (1132), shots (130) and damage match, and the
  death rows are identical. The unit table moves only nearest distances (about 15 m). The natives show
  place_members_007f2920 and initial_command_007f4e9e (4 calls each) taking the place of begin_flying_007c7110
  (12 calls), as the flag arm predicts.
- So the section 115 shot drop (13164 -> 130) came from the party switch's effect on the OFF baseline, not from the
  airborne arm. The flip stands.

**PRCP01 re-plan `p26_p01_a`** (`local\p26_p01_a.txt`, built by `local\p26_mkp01.py`).
- The plan is p9_p01_k with every Enterprise strike on Junyo at frame >= 5700 retargeted to Zuiho.
- Why: with the attack-target flip, Junyo dies at about 283 s, so those launches were refused. Zuiho's squadrons,
  now on the command arm, sank Enterprise at 1038.39 s.
- Results on the tree build of 50b1127e3:
  - ref: completed 833.02 s (`main_p01a`).
  - 20,3: completed 726.70 s (`main_p01a_j203`).
  - Both log the exec guard, and neither has "present failed".
- On the same build, p9_p01_k still fails at 1038.39 s. Adding Wildcat CAP launches gives a result identical to
  `a` (b); CAP alone fails (c).
- The plan was sent to cc9-lua72 (WINS keeper; it holds config/win_orders) to commit as the row.


## 117. Why JM08 plan h never spawns the C47s (cc9-physics26, 2026-10-24)

The script is `scripts/missions/COTP-IJN/PRCPIJN/prcpjm08.lua` (this installation, mtime 2024-08-26). The scene
prcpijn_08_defend_guadalcanal.scn names it, through the record+928h StageScript slot.

**Trigger chain (Lua only, no doubtful native).**
1. luaIntroMovieEnd starts CheckCompletion (line 635).
2. CheckCompletion (790) polls `luaObj_GetSuccess("primary",2) and luaObj_GetSuccess("primary",1)` every second.
   - Primary 2 is set by CheckPrim2 (747) when Missouri is dead.
   - Primary 1 is set by CheckPrim1 (727) when `Mission.LandShips` has no live member. LandShips is eleven ships
     (519-529): USTroopTransport 01-06, LST 01-03 and LSM 01-02.
3. When both are set, it moves the invasion force to RetreatPoint and starts the "Run" dialog (332), whose last step
   calls FadeIn (999).
4. FadeIn runs PreparePhase2 (1005) after 5 s, which starts luaBomberWave1 (B-17s, WingCount 3-5).
5. Each wave calls the next with luaDelay(..., 180): luaBomberWave2 (B-25s, 1080), then luaBomberWave3 (C-47s,
   1183, `Mission.C47Count`).

So the C-47s (the paratroopers) arrive about 365 s after the last LandShip and Missouri are both dead. Phase 2 has
no timer or HQ path.

**Plan h on main 4ada4aa61 (rebuilt tree, ref env, 72000 frames, `local\main_j8h`):** no end, and no present
failures. The guard line is absent because there is no completion.
- Missouri dies at 414.12 s. LandShips deaths:
  - USTroopTransport 06 at 499.65 s, 05 at 630.27, 04 at 699.55, 01 at 760.94 and 02 at 805.58;
  - LSM 01 at 666.96 s and LST 03 at 687.11;
  - LST 02 at 1129.63 s (AA truck 05), LST 01 at 1275.27 (Headquarter 01) and LSM 02 at 1391.78 (AA truck 06).
- **USTroopTransport 03 never dies.** From about step 22320 it sits stopped at (1636.6, -646.8) in attackmove,
  firing at Headquarter 01. It took 157 hits; its health is 1722.4 at the last hit (3977.4 at the first).
- Primary 1 is therefore never set, CheckCompletion never fires, and no B-17, B-25 or C-47 is ever spawned.
- Why nothing finishes it:
  - Every Japanese surface ship is dead by 876.31 s (the Gyoraitei, Fubuki 714.20 s, Isokaze 759.59 s).
  - Hosho sinks at 471.06 s.
  - MainAirFieldEntity 01 runs out of bombers: class 166/167 refuse with "no stock of the class" from frame 12902
    (645 s).
  - After that, the 43 applied orders on USTroopTransport 03 are fighter launches and order-3 retargets (classes
    152/154), which do not sink it.
- Fix (plan side): sink USTroopTransport 03 while bomber stock remains. For example, keep one class 166/167
  squadron in reserve for it, or put it first in the generator's target list.
  - Only then can phase 2 and the C-47s, and therefore the paratrooper switches, be reached.
  - ships62's generator is `local\s62_j8mk.py` (cc9-ships44).


## 118. USNOS: the loss is Enterprise torpedoed by the Takao group; plan `p26_os_a` holds to 1675 s; carrier final approaches float at ~80 m (cc9-physics26, 2026-10-24)

The script is `scripts/missions/COTP-USN/us_osumi.lua` (this installation, mtime 2024-10-29).

**Lose condition.**
- luaCheckObjectives (955) calls luaMissionFailed while primary 1 is active and either all six TroopTrans1-6 are
  dead or `Mission.BigE` (Enterprise) is dead.
- luaMissionFailedNew(Mission.JapHQs[1], "Game Over") only uses HQ1 as the camera entity, so `entity="HQ1"` in our
  summaries does not mean HQ1 fell.
- At the start of phase 2, luaMoveToPh2 (1619) sends `NavigatorAttackMove(Mission.InitTakao[1], Mission.BigE)`. That
  is the Takao1 group: Takao1 plus Shima4-7, in formation on Takao1.
- Primary 1 is completed only in luaReinTime (1735). luaReinTime runs either at the end of the 900 s blockade
  countdown (luaBlockadeMovieEnd) or once the blockade is cleared (luaReinFadeOut).
- The win needs all of:
  - all three JapHQs captured (troops within 1500 m of a neutral HQ);
  - the reinforcement's FinalTrgs killed (primary 5, ReinDead);
  - then the nuke sequence.

**The 1099.95 s loss** (`p25i_off_osL` on 802e1d8b9, and again `local\main_osL` on main 14ae38b93, l65_os_c1):
- Enterprise dies at 1095.79 s, first damage at 1080.88 s. The killer is Shima4, by torpedo (cat 7) from 2.29 km.
- l65_os_c1 parks Enterprise at (12500, -4000) from frame 300, and the Takao group closes on it from the
  north-west at about 14 m/s.

Variants (`local\p26_mkos.py`, all on the tree build of 14ae38b93, ref, 36000 frames):

| Plan | Change | End |
| --- | --- | --- |
| `p26_os_a` | Enterprise moveto (13500,-9000) at frame 15000 and (15000,-13000) at 19000 | failed 1675.51 s (Enterprise torpedoed by Shima7 at 1672.51 s) |
| `p26_os_b` | Enterprise 331/339 strikes on Takao1/Shima4/Shima5 every 600 frames from 14000 | failed 1099.95 s (Takao1 dies 1094.04 s; Shima4 still torpedoes at 1080.88 s) |
| `p26_os_c` | a + b | failed 1585.42 s (Shima7 at 1581.07 s) |

`p26_os_a` meets goal 1: HQ1 is held, and the run goes past 1600 s.

**The carriers still land nothing, and the cause is now visible.**
- In `main_osa`, Enterprise_sqn01, 02 and 37 queue at Enterprise at 1400.19-1401.19 s.
- Enterprise is stopped at (14995, -12993) by about step 32000.
- All three squadrons are alive and still queued at 1672.56 s. When Enterprise dies they re-queue on Reprisal and
  Lexington.
- Deck summary for Enterprise: queue=3, records=3, releases=0, landed=0, modes 3=2210.
- The heads do fly finals: Enterprise_sqn02 enters 5D8 at 1528.32 s and 1619.90 s, and Enterprise_sqn01 at
  1554.94 s.
- Every one of those finals ends in the abort state (64C -> 5B8 at 1543.33 s, 1571.16 s and 1635.52 s).
- The pattern (Enterprise_sqn01, `land final trace` / `land final flight`, 1555-1566 s):
  - throttle 0, brake 0.197, and speed falls from 98 to 33 m/s (q 3.1 -> 1.04);
  - the commanded pitch steepens from -0.22 to -0.55, but the live pitch rises from -0.24 to +0.03;
  - vy goes to about 0 at Y about 80 m;
  - the first abort trace is at local (x -0.4, y 77.1, z -204.2): 77 m above the deck, 204 m behind it.
- Enterprise_sqn02's aborts look the same (y 77.9 at z -206.5).
- The heads stop descending once they are slow, about 80 m above the deck. This is the airfield "float" of
  SQUADRON_LAND_TASK.md 5n/5o, but on a deck. Airfield3 does land planes in the same run (inserts=2, found=492).
- Next: find why the live pitch fails to follow `cmd` below q of about 1.5 on the carrier final. Candidates are the
  5o direction-hold steer with a deck target, a stall or altitude protection, or the deck-height reference. Read
  game_hosts_units.cpp around 36540-36640 and the 009B1F1D begin steer. Pair any fix on `p26_os_a`.

## 119. Carrier-final float on USNOS: comparison with an airfield final, and handoff (cc9-physics26, 2026-10-24)

Static, from two logs: `local\main_osa.log` (USNOS, plan `p26_os_a`, Enterprise_sqn01 final 1555-1566 s, then
abort 64C->5B8) and cc9-party1's `local\pt1_off_na_b06.log` (Airfield_sqn06 final 975-985 s, 5D8->5F8->620
success). Airfield3 in USNOS is not a usable reference: it lands 359 planes but logs no `land final trace` line, so
it does not fly the 5D8/5F8 final.

| | Airfield_sqn06 (lands) | Enterprise_sqn01 (floats) |
|---|---|---|
| 5B8 circle speed before the final | 97.8 m/s, mode 3 | 133.3 m/s, mode 3 |
| `land begin trace` at A8 ~830 | spd 69.0, `fast=0`, Y 181.9, H 79.7 | spd 100.6, `fast=1`, Y 168.8, H 80.3 |
| first `land final flight` | spd ~46 vs spd_cmd ~40, q 1.31 | spd 98.0 vs spd_cmd 46.5, q 3.11 |
| throttle / air brake on the airborne final | thr 0.00-0.28, brk 0.000 throughout | thr 0.000, brk 0.197 from 1556 s to 1563 s, then 0.118, 0.047, 0.039 |
| live pitch vs `cmd` | tracks within 0.003 (-0.229 vs -0.227) at q 1.02-1.31 | tracks to 1557 s (-0.253 vs -0.245); then -0.190/-0.236 (q 1.68), -0.101/-0.251, -0.043/-0.288, -0.023/-0.344, -0.014/-0.398, +0.030/-0.553 (q 1.04) |
| vy | about -8 steady | -17.5 at 1557 s, then -10.4, -5.3, -2.1, -1.0, -0.5 |
| outcome | Y 116 -> 35 over A8 555 -> 199, success | Y levels at about 80 (79.4 at A8 211); the abort trace puts it 77 m above the deck |

Findings:
1. **The pitch-rate limit and low-q elevator authority are not the term by themselves.** The airfield final holds
   pitch -0.23 to within 0.003 at q 1.02-1.08, lower than the carrier's q when it diverges (1.68). The divergence
   starts while the carrier plane is still above spd_cmd and decelerating hard.
2. **What differs is the entry energy and the air brake.** The carrier squadron flies its 5B8 standby circle at
   133 m/s (vs 98 m/s), enters the final with `fast=1` at about 100 m/s, and the airborne final (009B1F2C-009B1FDF,
   `plane_air_brake_mode_2d8 = 1`, game_hosts_units.cpp:36586) answers the 52 m/s overspeed with throttle 0 and the
   air brake at 0.197 for seven seconds. The airfield plane never uses the brake. The live pitch leaves `cmd` exactly
   in the braking window and climbs to level while aoa stays near 0, so the nose follows the flight path rather than
   the command; once the plane is slow, the hold (A8 < 260) asks for -0.55 and gets +0.03.
3. **The deck reference is a candidate, not shown.** H is 80.3 at the carrier's begin trace (the airfield's is 79.7
   at the same A8 and falls to 63.7 by A8 677), and the plane levels at Y of about 80, which is far above a
   carrier deck. I did not establish whether H, the 5o glide pitch P = -atan2(Y-H, m), or something else sets the
   80 m level. The begin trace stops printing once the final starts, so H during the float is not logged.
4. Item (3) of the lead's brief: this hinges on image terms that the host transcribes with labelled substitutions.
   The question for cc13 is below. I did not read the image.

**Question for cc13 (via the lead):**
(a) In the airborne land-final half (009B1F2C-009B1FDF and the throttle/brake law 0099D300 with +2D8h = 1), does the
image drive the air brake when the plane is over +2B4h, and does the air-brake channel (or the deceleration) feed a
pitching moment or a pitch-command change in 007D8470 or the 007D88CB direction hold? The host shows the brake at
0.197 and the nose rising to level with aoa about 0 while `cmd` goes from -0.24 to -0.55.
(b) What height does 009B1420's H use for a deck target (approach+30h answering vtable[5Ch](9))? Is it the deck
height of the moving ship, or a fixed or airfield-style reference? The host logs H = 80.3 for Enterprise at A8 833.
(c) Is the 5B8 standby circle speed for a carrier holder (133 m/s here) image-faithful, or is the circle flown at
the carrier's own speed or a fixed loiter speed?

**Next step for the successor:**
- Log H, P and the brake/pitch every tick from 1550-1570 s on `p26_os_a` (main_osa reproduces it at 1555 s;
  Enterprise is stopped at (14995, -12993) from 1400 s). Add an `H=` field to `land final trace`.
- Once cc13 answers, put the indicated term behind an OFF switch and pair it on `p26_os_a` (ref env). Success
  means a `land final trace` with Y below 30 near the deck and a 620 state for Enterprise_sqn01/02/37, and
  `releases`/`landed` above 0 on Enterprise's deck line.
- A cheap discriminator before cc13 answers: a temporary build that holds the brake at 0 on the airborne final.
  If the float goes away, the brake/pitch coupling is the term; if Y still levels near 80, it is the H reference.
  This is a diagnostic only, not a flip.

**Lane handoff (cc9-physics26).** My commits: a12180f58 (atktgt flip, section 114), 691b19de2 (CHG06 airborne and
altitude-draw flips, 115), 8d85ea786 and 29f1ab4f8 (sections 116-117), 5632272b4 (section 118), and this section.
Open items:
- JM08 phase 2 needs USTroopTransport 03 dead. It is with cc9-ships63 (section 117).
- USNOS has no win yet. `p26_os_a` holds the HQ to 1675.51 s. The carrier finals above stop the squadrons landing.
- The JM08 paratroopers are unreached.
- `kReleaseTimerNotify0DhBound` is still OFF.
Plans and logs are in this tree's local\: `p26_os_a/b/c.txt`, `p26_p01_a.txt`, `main_osa.log`, `main_osL.log`.

Lease `p26-doc119` (this doc) is released with this section.

## 120. `kCarrierSiteHasFreeRunwayCc13Bound` (006D0530) is unreached on six carrier rows (cc9-physics27, 2026-10-24)

The switch (cc13 44b94b4b9, OFF) makes a carrier deck's mode-4 landing spacing wait for a free runway: 006D0530
answers false while an occupant of site+34h lies less than Pilot/Landing/MinFreeRunwayLength along the deck
(006BCC90 z). `planeglobals.lua:631` (this installation) sets that length to 12 m. The answer is evaluated in both
modes, and ` site_busy=N` counts the passes where it would zero the spacing: tp < 0.25 FDT, the site not late
(now - site+40h >= 0.4 FDT) and the deck not free.

Pairs on 44b94b4b9: OFF is the tree build, ON is `local\p27_fr` (`pair_export --flip
kCarrierSiteHasFreeRunwayCc13Bound=true`). Ref env (`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`), launcher
`local\p27_pair.ps1`.

| Row (plan) | Carrier decks with mode-4 passes (m4 / site passes / landed) | site_busy | pair_diff |
| --- | --- | --- | --- |
| USN01 (`usn01_l69_u1_a`) | Enterprise 77 / 77 / 0 | 0 | gameplay identical; completed 370.03 s, guard logged |
| BSM01 (`bsm01_s62_b1_h`) | Akagi 806/291/275, Kaga 1664/711/470, Hiryu 1778/776/548, Soryu 1090/402/343, Zuikaku 1147/446/394, Shokaku 1619/636/491 | 0 | gameplay identical; EndMission 821.92 s, guard logged |
| BSM04 (`l72w_bsm04_h1_f`) | Lexington 673 / 370 / 195 | 0 | gameplay identical; completed 941.89 s |
| USNOS (`p26_os_a`) | Enterprise 152 / 142 / 0 | 0 | gameplay identical; failed 1675.51 s (as section 118) |
| ESMP08 (`esmp08_p12_e8_rb`, OFF only) | Zuiho 116/80/36, Chitose 313/128/147 | 0 | - ; completed 1284.28 s |
| IJN18 (`l71_i18_s`, OFF only) | Bogue1-4: no mode-4 passes (l71r's Bogue3 landings did not recur) | 0 | - ; EndMission 1791.62 s |

Verdict: **unreached**. Over about 3,700 carrier site passes, with 2,900 planes landed on carrier decks, no pass
found a deck occupant inside the 12 m free-runway margin while the site was not late. The switch stays OFF, recorded
as unreached; flipping it changes nothing on these rows. A plausible reason (not shown): a plane that touches down is
inside the margin only during the first 0.4 FDT after its touchdown, when the site is late anyway and the spacing is
zeroed by the other term.

Note: `local\main_osa.log` is now this pair's OFF run. physics26's log of the same name (sections 118-119) was
copied first to `local\p26_main_osa.log`.
