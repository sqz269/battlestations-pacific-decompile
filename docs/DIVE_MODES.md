# The dive's two lost-release modes, and the dogfight leader's climb

Addresses: 009C62B0, 009C63BF-009C63FE, 009C65FD, 009C6615-009C66E7, 009C6674, 009C6A37-009C6A9F,
009C6D42-009C6DEF, 009C6F97, 009C3F16, 009C58D0, 009C5AA3, 009C5AF1, 009C5D0E-009C5DB2, 007DA710,
007DA732, 007CE47F, 009C18C0, 009C2AC0, 009C2CA0, 009A955B, 009AB1C0, 009BDE80, 009FBA50.

Packet `cc9_dive_modes`. Every name is a hypothesis, not a recovered symbol. Background:
`docs/AIMGLIDE_PITCH.md` section 4, runs U3 and M0.

Diagnostic logs:
* `local\T3_9000.log` is U3 plus the `fa_trace` fly-over line added this packet.
* `local\M0T_9000.log` is main (M0) with the same trace.
* Both use `BSP_GUNNERY_RNG_STREAMS=1` at E2.

## 1. Mode B: flights #3.1 and #7.1 roll in 380 m early

**Which arm fires.** Neither roll-in arm fires on the first pass. The **leave** arm (009C66E3)
fires first, and the roll-in 380 m out is the *second* approach. In T3:

| aircraft | leave: t, lead error, span, speed | second pass: roll-in by the 1.6 rad bearing arm |
| --- | --- | --- |
| #3.1 | t=1081, 0.3508, span 32.9, 60.0 m/s | t=1260, error 2.40, range 1413 |
| #3.1\|.-2 | t=1098, 0.3526, span 12.8, 52.1 m/s | t=1280, error 2.79, range 1415 |
| #7.1 | t=1220, 0.3513, span 157.8, 60.4 m/s | t=1377, error 2.92, range 1417 |
| #7.1\|.-2 | t=1235, 0.3509, span 182.8, 50.1 m/s | t=1404, error 2.50, range 1415 |
| #1.1 (reference) | none | t=1282, span arm, span 0.0, error 0.090, range 1046 |

**Why leave fires.**
* The leave tolerance is 00419010(0, 20° [00CE398C], B4h × 0.8 [00CE3D40] - S, π, span). With the
  attack distance B4h, the far endpoint is below zero, so the tolerance sits at its 20° floor
  (0.349 rad).
* The heading arm's dead band is T = 00419010(0, 30° [00CEC724], 200 [00CE386C], 0, span)
  (009C6674, applied at 009C6A37). It widens to 30° as the span closes. A lead-bearing error under T
  is never corrected: the heading stays at -2.9132 for #7.1 across the last ticks.
* Flights #3.1 and #7.1 attack a **moving** target, with velocity (-16.64, -0.89) m/s in
  `fa_trace`. #1.1's target is still. So their 3-second lead bearing (009C63BF-009C63FE) drifts
  while the aircraft holds heading inside the dead band. The error grows from about 0.04 to 0.35
  rad and crosses 20° before the span closes.

**The speed decides the race, and the speed is the image's.**
* M0T, with the frozen throttle, runs the same law. #3.1 and #3.1|.-2 close at 98-99 m/s and reach
  span 0 at errors 0.281 and 0.233, so they roll in normally at 1276-1290 m.
* In T3 they close at 50-60 m/s and lose the race.
* #7.1's own leader loses it even in M0T: leave at t=1146, error 0.3504, span 32.5, 100.9 m/s, then
  the 1.6 rad roll-in at 1416 m (t=1314). That is the same sequence at the old speed.
* The fly-over's speed is the image's own law. It is 009C6F97: approach+A4h (= classDesc+188h MaxSpd
  × 0.95 [00CEFFB0 qword], 009C3F16), plus the near-field term. For the Val that is
  0.95 × 69.44 = 65.97 m/s, and the host commands exactly that.

**Host against image, term by term:**
* the lead point (009C62D1-009C63E6);
* the span (009C65FD);
* the dead band (009C6674 / 009C6A37);
* the slew (009C6D6F);
* the leave (009C66E3);
* the roll-in (009C67B0);
* the fly-over speed (009C6F97).

All are already bound in the host from the same reads, and nothing on this path is a substitution
for the two flights.

**Conclusion.** Mode B is the image's own behaviour at the image's speed: the dead band outgrows the
20° leave tolerance against a moving target. **Nothing is bound.** One host gap remains on this path
and may matter: `+19h` is not carried across ticks (009C688F/009C68E1 clear it, and 009C6A30 sets it
from `+18h` when cos(e) × R < [00CF180C]). The host recomputes it from 009C67B0 alone. It is
documented at `src/game_hosts_units.cpp` ("READ and NOT applied"). It does not decide the first
pass, where neither arm is set.

## 2. Mode A: wingmen bank past 1 rad late in the aimdive

**The law matches the image.** At 009C5D0E-009C5DB2 the roll stick at cmd+290h (active +294h,
cmd+2CCh = EBX) is 00419010(-band, 1, band, -1, x).
* It takes the wide band 0.5 [00CE3800] on [ESP+18h], the impact-point bearing error from 009C5AF1,
  while [ESP+5Ch] > 0 and the folded |bank| [ESP+20h] is under 60° [00D05AAC].
* Otherwise it takes the tight band 0.4 [00CE7804] on [ESP+24h], the aircraft-bearing error from
  009C5AA3.
* The host's inputs are those same slots, from `db_impact_bearing_18`, `db_bearing_c0` and
  `db_aim_heading_last`.
* There is no bank limit and no other roll write in the tick.

**What happens (T3, D3A Val #1.1|.-3):**
* At pitch -0.42 to -0.52, shallower than the 30° steep gate, [ESP+5Ch] carries the planar distance,
  which is positive. So the wide band runs on the impact bearing.
* With the CCIP distance at 15-30 m, that bearing sweeps 3.85, 3.42, 3.01, 1.80, 1.24, 0.50, 6.00,
  5.28 rad over 30 ticks. The stick saturates, and the bank goes from 0.12 to -1.17 rad in 9 ticks.
* Past 60° the tight band takes over. Its error, aim heading minus the aircraft bearing, is small,
  and the bank holds at -1.05 to -1.62 until the abort.

**Downstream of the law, the control-rate roll arm is:**

    roll = RollSpd × modeFactor × (unit+838h + unit+BB8h)

It is read at 007DA732-007DA7DA, with EDI = ctl+8h = the unit.
* The host has unit+838h at 0.0 (`roll_base_838`, never written).
* The disp32 census finds no writer of unit+838h in the plane code. The five 007CE040 hits
  (007CE47F-007CE555) are `[ESI+838h]` with ESI = unit+310h (007CE479 `LEA EBP,[ESI-310h]`), so they
  write unit+B48h, a loadout-dependent value (1.0, or tuning+344h/+33Ch/... by weapon kind), not
  unit+838h.
* An indirect writer through another base is not excluded.

**The roll stick, measured (V1, `local\V1_9000.log`, D3A Val #1.1|.-3).** `live_roll` equals
the aimdive's stick throughout, and the stick is a pure copy down to the physics: the 0099BC00 slew,
the 007BB6E0 quantisation, then unit+BB8h into 007DA732.
* A positive stick drives the bank negative. At t=1417-1427 the stick is +1.0 and the bank goes
  from 0.12 to -0.97.
* A negative bank turns the heading down. At t=1437-1459 the bank is -1.05 to -1.62 and the
  heading falls from 3.13 to 2.67.
* In the tight band the error `aimhdg - brg_c0` is -0.08 to -0.31. The map gives a positive
  stick, +0.07 to +0.76, which deepens the bank and turns the nose further from the aim bearing.
  That is positive feedback.

**The sign is the image's, not a host inversion.**
* **The subtraction.** 00438B10 is wrap(arg0 - arg1) (00438B10 FLD [ESP+4], FSUB [ESP+8]). At
  009C5A95-009C5AA3 arg0 is base+10h, the 009C4F80 result stored at 009C593A, and arg1 is the
  bearing. So the tight arm's x is aim heading minus bearing.
* **The tight map.** It is 00419010(-0.4 [00D1F400], +1, 0.4 [00CE7804], -1 [00D7A260], x), from
  the pushes at 009C5D60-009C5D8B.
* **The planner's servo in the host has the same physics convention.** In V1's aimglide ticks,
  under the wings-level servo, D3A Val #5.1|.-2 at bank +1.47 gets live stick +1 and the bank
  falls. movieval at -1.31 gets -1 and the bank rises. That servo is the image's 0099E2CE-0099E390,
  falling on bank target minus bank.
* So the image's aimdive map is the opposite sense to its own planner on the heading error.
  009C4F80 explains why:
  * At pitch at or below -40° ([00CE7D1C] = -0.698, `kAimHeadingSteepPitch`) the aim heading is the bearing
    of the body up axis (009C5010-009C507C), which rolling moves directly. In that regime the same
    map can be stabilizing.
  * Above -40° it is the raw heading (009C516C), or the raw heading plus π when inverted, and the
    map is destabilizing.
* The host's slow dives sit at pitch -0.42 to -0.68 (-24° to -39°), which is inside the raw-heading
  arm.

**Conclusion.** Mode A is the image's own law run outside its design regime: a dive shallower than
-40°. Nothing is bound. The term that decides it is **the dive's steepness**. The dive enters at
pitch -1.07 at 54 m/s. The entry aim error swings from -294 m to +252 m in the first 25 ticks,
and the pitch law (the 30° steep gate, then error × AimPrecPull) pitches the aircraft up to -0.42
while the error settles. That is the swing already recorded in `docs/AIMDIVE_RESPONSE.md` 4(b) and
`docs/DIVE_THROTTLE.md` 4. Its first 25 ticks are the next read: the aim error's producer
009C59BA-009C5C9B at aimdive entry, where the CCIP distance is 540-595 m at 54 m/s.

## 3. The dogfight leader's climb

**The image's dogfight moveto is the generic tick.** 009C2CA0 builds it with vtable 00D20B24: +Ch
is 009C18C0 and +1Ch is 009C1BC0. It is called once, at 009A955B in the dogfight approach
constructor, with ranges passed as constants:
* +30h = 500.0 [00CE397C];
* +34h = 100.0 [00CE3D08];
* +38h = 1000.0 [00CE3804].

The store order is 009C2AC0's 009C2B17/2B22/2B41. Of the rel32 callers of 009BDE80
BSP_BotStateMoveTo_SetRanges (007B7C7F, 009A2B47, 009A671A, 009AE8F7, 009B8BA7, 009C8825, 009CD1AF,
009D48CF), none is the dogfight arm 009AB1C0, so the ranges stay fixed.

**Bound** as `kDogfightMovetoGenericBound`. The dogfight moveto runs `move_to_glide_009c18c0` with
(500, 100, 1000), then 009FBA50 and 009FB800, as the dive-bomb tick does. The heading stays 009F9E40
toward the target, and the speed slot 009C1BC0 is already bound.

**Prediction.** 009FBA50 caps the command at the ceiling less 50 (1450 m), because
base = max(100 + target y, 500). So the leader holds at or below 1450 m, outside the ceiling force
of `docs/AIMGLIDE_PITCH.md` section 6, and accelerates toward its 83.3 m/s slot instead of hanging
at 34 m/s. Measured in section 5.

## 4. Predictions for V1/V2/V3 (written before the runs)

* **V1** is the U3 set.
  * Mode B has nothing to bind, so flights #3.1 and #7.1 keep their 0 releases.
  * Mode A has nothing bound, so the result stays at U3's: 16, and 8 at 4500.
* **V2** adds the generic moveto.
  * The Yorktown leader stays at or below 1450 m and above 60 m/s.
  * The dive-bomb rows do not move.
  * The AA rows are RNG-coupled.
* **V3** adds the hull offset. It releases no more than V2, as in U2 and U4.

## 5. Runs

All runs use `BSP_GUNNERY_RNG_STREAMS=1`. "9000" is `--frames 9200 --mission-frames 9000` and "4500"
is `--frames 4700 --mission-frames 4500`, with the other E2 parameters. Logs are in this tree's
`local\`.

| run | binary | switches beyond main | 9000: drops / kills / water / end | 4500: drops / kills / water / end |
| --- | --- | --- | --- | --- |
| control | `dmC` | none (main at 3e0d96213) | 32 / 34 / 8 / failed 228.06 s (`DC_9000.log`) | 20 / 24 / 0 / none (`DC_4500.log`) |
| V1 | `dmT4` | U3 set (fix, tail, glide pitch, draw, yaw), traces | 16 / 37 / 7 / failed 297.05 s (`V1_9000.log`) | not run |
| V2 | `dmV2` | V1 + generic moveto | 16 / 37 / 7 / failed 297.05 s (`V2_9000.log`) | 8 / 16 / 0 / none (`V2_4500.log`) |
| V3 | `dmV3` | V2 + hull offset | 18 / 36 / 7 / failed 297.05 s (`V3_9000.log`) | not run |
| G | `dmG` | main + generic moveto only | 33 / 33 / 9 / failed 228.06 s (`G_9000.log`) | not run |

* The control moved from AIMGLIDE_PITCH's M0 (29 and 17) to 32 and 20, because main took other
  packets since.
* The first diagnostic runs (T4 and T4b) crashed at D3D device creation, along with every probe from
  about 10:54 to 11:40, including `binC0`. That was environmental, and it cleared on its own. The
  probes are `local\probeA.log` to `probeH.log`.

**Predictions against the results:**
* **V1 as predicted.** It gives 16, with U3's per-aircraft losses: Mode B for flights #3.1 and #7.1,
  and Mode A for the wingmen.
* **V2 against V1.** The dive-bomb rows are identical (`release census`, `db aim exit` and
  `bomb drop` lines diff empty). The Yorktown leader now settles at **1475 m and 82.2 m/s** from
  about think 1600 to the end, against V1's 1547 m and 34 m/s. It does not reach the water. That is
  25 m above the predicted 1450: the command is capped at 1450, and the pitch law settles it a little
  high. It is still under the 1500 m ceiling, so no ceiling force applies.
* **V3.** The hull offset adds 2 rounds at 9000, one extra salvo.
* **G, on main's own configuration.** The generic moveto is not row-neutral.
  * 69 dogfight rows move.
  * 46 dive-bomb rows (`release census`, `db aim exit`, `bomb drop`) move, and drops go from 32 to
    33.
  * One extra Val water contact appears, D3A Val #3.1|.-4 at 69.5 m/s.
  * The dive-bomb moves can only come through the fighters' changed engagements, since nothing in
    the Val's own path changed. They are not explained row by row.

## 6. Decisions

* **The flip does not land.** V1 to V3 release 16-18 against the control's 32, and 8 against 20 at
  4500. `kPilotThrottleSlotBound`, `kAimDiveTailBound`, `kAimGlidePitchBound`, `kAimGlideYawBound`
  and `kReleaseAltitudeDrawBound` stay OFF.
* **Mode B and Mode A bind nothing.** Both are the image's own laws running in the regime the
  image's own speeds put the Val in.
  * **Mode B** is the dead band outgrowing the 20° leave tolerance against a moving target at
    65.97 m/s.
  * **Mode A** is the aimdive roll map, which is stabilizing only in a dive at or steeper than -40°,
    applied to a -24° to -39° dive.
* **`kDogfightMovetoGenericBound` lands OFF.** The read is the image's: the dogfight moveto is
  the generic 009C18C0 with ranges (500, 100, 1000). With the throttle fix it cures the leader: it
  settles at 1475 m and 82.2 m/s instead of 34 m/s at 1547 m. On main alone (G) it moves dive-bomb
  rows through the fighters, which is not yet explained.
* **The diagnostic traces are off with `kHullAimTrace`:** `fa_trace`, plus `roll`, `aimhdg` and
  `live_roll` in `hull_trace`.
* **The next term** is the aimdive entry swing: the aim error at 009C59BA-009C5C9B over the first
  25 ticks of the dive, from -294 m to +252 m at 54 m/s with a CCIP distance of 540-595 m. It is what
  pitches the dive up from -1.07 to -0.42. A steeper dive would clear the 009C5B43 abort, which needs
  a dive shallower than -60°, reach the release altitude, and put 009C4F80 on its body-axis arm,
  where the roll map is stabilizing. It is the one term both modes share downstream of the speed.
