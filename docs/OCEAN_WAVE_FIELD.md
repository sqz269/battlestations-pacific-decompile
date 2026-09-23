# The ocean wave field behind the water height

Packet cc9_ocean_waves, 2026-09-23. Base: main e1d1c0494. Reconstruction:
include/bsp/ocean_wave_field.hpp and src/ocean_wave_field.cpp. Host seam: the water-height
binding in src/game_hosts_units.cpp, behind `kOceanWaveFieldBound`, which is on. Status:
reconstructed and build-tested; measurement in section 5. Not ABI-compatible and not
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
everywhere.

## 4. Predictions (local/ow_predictions.txt, written before any run)

- The height is ±0.0 at every ship and every step.
- Pitch, roll, heave, the keel point and the throttle gate are unchanged.
- Gunnery is unchanged, with no random-number coupling.
- Only the method table moves, by about 2.0M calls.

## 5. Measurement

See the report (reports/ocean_wave_field.json, `validation`) for the run status on this machine.

## 6. Open

- A runtime check that field+24h stays 0.0. Only an attached debugger on the retail game could
  confirm it, and this packet does not run the retail game.
- The scene record's +0C20h byte and the +990h block per mission. The host does not parse them.
  They matter only if the amplitude is ever non-zero.
- The FFT itself (00B95750, 00B95D30, 00B95AA0, 00B952D0). Reproducing the rendered sea would
  need it; ship motion does not.
