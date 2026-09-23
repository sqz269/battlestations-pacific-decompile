# Mode B's inputs, checked by provenance

Addresses: 009C62B0, 009C62CF, 009C62E8, 009C6320-009C63FE, 009C65C1-009C65FD, 009C6615-009C66E7,
009C6674, 009C6A37, 009C6F97, 009C3F16, 009C3F86-009C3F97, 009FA2E0, 007D297F.

Packet `cc9_mode_b_inputs`. Every name is a hypothesis, not a recovered symbol. `docs/DIVE_MODES.md`
section 1 found that flights #3.1 and #7.1 leave the fly-over (009C66E3) before they roll in,
because the lead-bearing error against a moving target outgrows the 20° tolerance inside the 30°
dead band. This packet checks that each input to that race is the image's, by source and value.

The host values are from `local\T3_9000.log` (`fa_trace`), cross-checked in `local\R1fr_9000.log`,
which was built after main took ce615ac7d.

## 1. The input table

| term | host value | image value | source | match |
| --- | --- | --- | --- | --- |
| class+268h TurnCircleRadius (Val) | 1300 | 1300 | `vehicleclasses.lua` VehicleClass[158] line 54263; plain Number read at 007D297F | yes |
| approach+B4h, the attack distance | **780** (0.6 × 1300, the draw's low end) | **uniform(0.6, 0.8) × 1300 = 780-1040**, drawn once per task | 009C3F86-009C3F97; 00CE3D30 0.6 and 00CE74F8 0.8, floats; 00BD2F10 | **no** (pinned draw), but see section 2 |
| approach+B8h | 2080 | uniform(1.6, 1.8) × 1300 = 2080-2340 | 009C3FE8/009C3FF5 | pinned draw; not a Mode B input |
| S = 0.7 × max(H, 100) + 200 | 943 at #3.1's leave (H 1062) | same | 0.7 [00CEFFA0 qword], 200 [00CE4D70 qword], floor 100 | yes |
| leave far endpoint scale | 0.8 | 0.8 | [00CE3D40] qword `0x3FE99999A0000000` | yes |
| leave tolerance floor | 0.3491 (20°) | 0.3491 | [00CE398C] float `0x3EB2B8C3` | yes |
| leave tolerance top | π | π | [00D7A264] | yes |
| dead band at span 0 | 0.5236 (30°) | 0.5236 | [00CEC724] float `0x3F060A92` | yes |
| dead band span | 200 | 200 | [00CE386C] float `0x43480000` | yes |
| lead time | 3.0 s | 3.0 s | [00D7A2B0] qword `0x4008000000000000`, 009C6320/009C6328 | yes |
| own velocity in the lead | unit world velocity | unit vtable+34h, `007BBB70`, which returns unit+AC8h | 009C62CF | yes |
| target velocity in the lead | ship rigid-body linear velocity (the Yorktown: (-16.64, -0.90) and (-12.25, 11.30) in T3; the same in R1) | 009FA2E0 BSP_BotApproachTargetRef_GetTargetVelocity, then the target's vtable+34h | 009C62E8 | magnitude yes, 16.67 against MaxSpeed 16.72; **the ship's vtable+34h body is unread** |
| roll-in bearing arm | 1.6 rad | 1.6 | [00CE3D48] qword | yes |
| fly-over speed | 65.97 m/s | 0.95 × class+188h MaxSpd (69.444443) = 65.97, plus the near-field term | 009C3F16 (0.95 [00CEFFB0] qword); `vehicleclasses.lua` VehicleClass[158] MaxSpd | yes |
| the moving target | Yorktown-class01, 16.67 m/s | the same ship under its script order (moveonpath, throttle 0.90-0.97), MaxSpeed 16.71944 | `vehicleclasses.lua` VehicleClass[2] line 968 | yes, within 0.3% |

## 2. The one mismatch does not reach the race

The host pins B4h at the low end of its draw, as it pinned every draw (`docs/AIMGLIDE_PITCH.md` 2).
The leave tolerance's far endpoint is `0.8 × B4h - S`.
* At #3.1's leave, S is 943. The image's whole draw range gives 0.8 × B4h of 624-832, so the far
  endpoint is **-319 to -111**. It is negative for every possible draw.
* The tolerance map 00419010(0, 20°, far, π, span) with a negative far endpoint clamps to its 20°
  floor at every span ≥ 0.
* So the tolerance is 20° under any draw, as it is in the host. The pinned draw changes nothing in
  Mode B.
* For it to matter, S would have to fall below 0.8 × 1040 = 832, which means H < 903 m. The
  fly-over leaves at H of 1042-1062 for #3.1 and 966-981 for #7.1 (T3). #7.1's 966 m is within
  reach only if its draw lands above 0.8 × B4h = S = 876, that is B4h > 1095, which exceeds the
  draw's top of 1040. So it is not reachable either.

## 3. The moving target

* **Which ship.** Flights #3.1 and #7.1 both attack **Yorktown-class01** (`command target` rows),
  which runs its script path at 16.64-16.67 m/s against an authored MaxSpeed of 16.71944.
* **Speed changes.** ce615ac7d (ship formation speed) is in the R1 tree, and the Yorktown's velocity
  there is the same (-16.64, -0.83). The station-keeping packet in flight could change the ship's
  speed only if it moves the Yorktown's commanded throttle. It is not in this tree, and its effect
  is not measured here.
* **Sensitivity.** The lead error's drift comes from the target's cross-track motion over the 3 s
  lead while the Val holds its heading inside the dead band. The drift is proportional to the
  target speed: a ship at 12 m/s instead of 16.7 would take about 1.4 times as long to push the error past
  20°, giving the Val 1.4 times as long to close the span. Nothing in the image's data points to a slower Yorktown.

## 4. Decision

* **Every input to Mode B is the image's**, with one pinned draw that cannot change the outcome.
* **Mode B is closed as image law** at the image's speeds, subject to one unread body: the ship's
  own velocity getter, vtable+34h, which the host stands in with the rigid body's velocity. Its
  magnitude agrees with the authored top speed.
* No switch and no run.
* R0 is still queued behind the renderer-init crash (`docs/DIVE_FLIGHT_RESPONSE.md` 5). This packet's
  one probe (`localprobeN.log`) died at renderer init again.
