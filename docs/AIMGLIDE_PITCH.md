# The aimglide pitch target and the release-altitude draw

Addresses: 009C5180, 009C5353, 009C5484-009C55DF, 009C5493, 009C54E7, 00BF8490, 009C555E,
009C55AB, 009C3EA0, 009C3F09-009C3F2E, 00BD2F10, 009C73A0, 009C5B43, 0099D300.

Packet `cc9_aimglide_pitch`. Every name is a hypothesis, not a recovered symbol. Background:
`docs/DIVE_THROTTLE.md` sections 4-5.

## 1. The pitch target, 009C5484-009C55DF

In the aimglide tick `BSP_BotStateDiveBombAimGlide_Tick` 009C5180, the frame base is 68h
(`SUB ESP,58h` plus four pushes). Every 00419010 call pushes 14h and `RET 14h` pops it, so the
`[ESP+N]` below are base offsets.

| step | site | value |
| --- | --- | --- |
| C | 009C5484-009C5493 | `(approach+14h)->+40h * approach+A8h`, NewReleaseMul times the release altitude, FSTP dword into `[ESP+1Ch]`. This is the ceiling the release gate also reads |
| D | 009C5497-009C54CD | `abs([ESP+10h] - [ESP+14h])` (AND 7FFFFFFFh on the dword), then `max(., 1.0)` (FLD1, FCOMIP, JBE). This is the planar distance to the aim point less the planar distance to the predicted impact point |
| a | 009C54CD-009C54F7 | `atan((C - H) / D)`, where H = `[ESP+20h]` is the height above the fed aim point (009C5265-009C5281). 00BF8490 is `_CIatan`. Each step is stored as a dword |
| a' | 009C54FB-009C5522 | `min(a, classDesc+1ECh)`, the class climb angle |
| P1 | 009C5522-009C5563 | `00419010(0.75 [00CEE07C], -1.0 [00D7A260], 1.15 [00D20CE4], a', R)`, where R = `[ESP+24h]` = `[ESP+10h] / [ESP+14h]` (009C534B) |
| P2 | 009C5567-009C55B0 | `00419010(C * 0.7 [00CEFFA0 qword], 0.0 (FLDZ), C * 1.1 [00CE3DF0 qword], -1.0, H)`. Both products are stored as dwords |
| target | 009C55B4-009C55DF | `max(P1, P2)` into cmd+2BCh, and cmd+2D0h = EBP = 2 (009C53DD) |

* **P1.** When the bomb would carry past the aim point (R below 0.75), P1 commands -1 rad. When
  it would fall short (R above 1.15), P1 commands the glide slope that reaches the ceiling C over
  the remaining miss distance D, capped at the climb angle.
* **P2.** Below 0.7C, P2 is 0 (level). Above 1.1C it is -1 rad.
* **The larger of the two wins.** So P2 pulls the glide level near and below the ceiling, and P1
  can lift it.

It is bound as `dive_bomb_aimglide_pitch_009c5484` (`src/dive_bomb_aimdive_tail.cpp`) behind
`kAimGlidePitchBound`. The host feeds it:
* `db_planar_bc` as `[ESP+10h]`;
* `db_impact_throw_14` as `[ESP+14h]`;
* `unit.y - db_aim_point_height_50` as H, where the fed aim point is the one 009FADA0 stores;
* `new_release_mul_04c * db_dive_alt_a8` as C, the same product as the host's release-gate ceiling;
* `plane_climb_angle_1ec` as the climb angle.

## 2. The release-altitude draw

* **The draw.** In the approach constructor 009C3EA0, `approach+A8h =
  00BD2F10(row+38h, row+3Ch)`. The pushes are at 009C3F1C and 009C3F23, `ECX = 1` is set at
  009C3F09, and the store is at 009C3F2E.
* **When it runs.** 009C3EA0's only caller is the dive-bomb task constructor 009C73A0, so the
  draw is **per task**, not per dive. The per-tick 009C7A94 then only lowers it.
* **The row values.** On the SPNormal row, row+38h and row+3Ch are DiveBombReleaseAlt { 350, 450 }.
  The host pinned the draw at 350.
* **The substitution.** `kReleaseAltitudeDrawBound` draws it instead, and this is a labelled
  substitution. The image's generator is the one process-wide stream the gunnery host models,
  but that generator is private to `src/game_hosts_gunnery.cpp`, which this packet may not edit.
  So the units host keeps a copy of the same algorithm and seed (LCG 1664525 / 1013904223,
  24-bit), one sequence across these draws. With `BSP_GUNNERY_RNG_STREAMS=1` it is keyed per
  unit name, the same way the gunnery option keys its draws.

## 3. Predictions, written before runs M0/U1/U2

* M0 is main as merged: the fix and tail are off, the goaway and glide throttle are on, and the
  new switches are off.
* U1 is the fix, the tail, the glide pitch and the draw, with the hull and throttle traces on.
* U2 is U1 plus the hull offset.

All are at E2 (9000) and at the 4700/4500 reference, with `BSP_GUNNERY_RNG_STREAMS=1`.

1. **M0** reproduces C0 at both lengths: 29 and 17 drops, and 8 and 0 water contacts.
2. **U1, dive entry.** The entry speed is unchanged from T1 (about 54 m/s from about 850 m).
   The same entry swing appears, settling within about 40 ticks.
3. **U1, the aimdive release altitude.** With a8 drawn in 350-450, a Val whose draw exceeds its
   abort height (about 420 m, as in T1) releases in aimdive. Otherwise the 009C5B43 abort still
   fires first, since the dive is still shallower than -60 degrees.
4. **U1, the glide.** After an abort at about 420 m the glide has C = 0.6 × a8, which is 210-270.
   * H is about 410 and at least 1.1C, so P2 = -1 and the glide pitches steeply down.
   * It levels as H falls through 1.1C to 0.7C, which is under the 260 m ceiling gate (C + 50).
   * The glide gates then decide: bearing, lateral 120 m, and the lead window -25 to -5 m.
   * I expect most glides to reach a release, and releases to return to at least C0's (29 at
     9000, 17 at 4500), with lower release altitudes and speeds than C0's 300 m and 133 m/s.
5. **Water contacts** stay near 0 because goaway is powered. The fighters survive, as in T1.
6. **Risk.** The lead window. In C0 the glides that reached the gates failed on lateral and lead.
   If that repeats, releases fall short and the next term is the glide's heading (the labelled
   substitution at 009C5435).

### 3a. Added after U1, before runs U3/U4

U1 (section 4) released 8 of the control's 29. The dominant glide block is the 30-degree bearing
gate, and a second term was found:
* **The yaw arm** 009C53E7-009C542A. The glide steers by direct yaw (8.0 × the signed bearing
  error, wings level) whenever the planar miss |aim - impact| is under 140 m (009C53D8). The host
  only ever ran the heading arm.
* It is bound as `dive_bomb_aimglide_steer_009c53d0` behind `kAimGlideYawBound`.
* U3 is U1 plus the yaw arm, and U4 is U3 plus the hull offset.

**Prediction for U3.** Bearing blocks fall sharply, because the yaw arm swings the nose onto the
aim point in the last 140 m of miss. Releases rise well above U1's 8, to about 20-35 at 9000 frames.

## 4. Results

The runs use `BSP_GUNNERY_RNG_STREAMS=1`. "9000" is `--frames 9200 --mission-frames 9000` and
"4500" is `--frames 4700 --mission-frames 4500`, with the other E2 parameters. Logs are in this
tree's `local\`.

| run | switches beyond main | 9000: drops / kills / water / end | 4500: drops / kills / water / end |
| --- | --- | --- | --- |
| M0 | none (main as merged) | 29 / 35 / 8 / failed 222.06 s (`M0_9000.log`) | 17 / 26 / 0 / failed 222.06 s (`M0_4500.log`) |
| U1 | fix, tail, glide pitch, draw | 8 / 36 / 2 / failed 342.04 s (`U1_9000.log`) | 6 / 16 / 0 / none (`U1_4500.log`) |
| U2 | U1 + hull offset | 8 / 36 / 3 / failed 342.04 s (`U2_9000.log`) | 4 / 16 / 2 / none (`U2_4500.log`) |
| U3 | U1 + glide yaw arm | 16 / 37 / 7 / failed 264.06 s (`U3_9000.log`) | 8 / 16 / 0 / none (`U3_4500.log`) |
| U4 | U3 + hull offset | 16 / 37 / 7 / failed 261.06 s (`U4_9000.log`) | not run |

M0 matches C0 of `docs/DIVE_THROTTLE.md` on drops and water contacts. Its mission ends at 222 s
instead of 186 s because main has since taken other packets.

**Per aircraft, 9000 frames**, from `release census` and `db aim exit`:

| aircraft | M0 | U3 |
| --- | --- | --- |
| #1.1 and #5.1 leaders, #1.1\|.-2, #5.1\|.-2..-4, movieval, movieval\|.-2 | 2 rounds each in aimdive, at 288-350 m and 132-135 m/s | a 2-round salvo in aimglide, at 231-290 m and 78-91 m/s |
| #1.1\|.-3, #1.1\|.-4, movieval\|.-3 | 2 rounds each | **0**: mode A |
| #3.1, #3.1\|.-2, #3.1\|.-4 | 1 round each | **0**: mode B, and #3.1\|.-4 mode A |
| #7.1\|.-2, #7.1\|.-4 | 2 rounds each | **0**: mode B |

* **No aimdive releases in any U run.** Every Val that aborts does so at 418-436 m, with range
  497-516 m. With a8 drawn in 350-450, none was both under a8 and on aim before the 009C5B43 abort.
  Prediction 3 failed.
* **The glide works where it is entered cleanly.** For the four leaders and their clean wingmen,
  P2 = -1 carries the glide down through the 1.1C band. It releases at 231-290 m, under the C + 50
  ceiling. Prediction 4 held for them.
* **Mode A: glide entry with a large bearing error.** Four wingmen, one per flight, bank to about
  -1.0 to -1.77 rad late in aimdive. That is the aimdive roll command chasing the impact-point
  bearing (`brg_18` swings 3.4, then 1.2, 0.3, 5.3 as `ccip_d` shrinks). They enter the glide
  beyond the 30-degree bearing tolerance, and the pull-out latch (009C57FF) sends them to goaway
  after 2 calls. The yaw arm (U3) cannot help, because they never stay in the glide.
* **Mode B: turndown begins too far out.** All of flights #3.1 and #7.1 are affected.
  * They start turndown at 1420 m range, against 1040 m for #1.1 and #5.1. Their range then grows
    to 1694-1727 m during the turndown.
  * They enter aimdive at 916-949 m height with a +440 to +569 m error, and every pass ends at the
    pull-out floor (170-219 m), still 350-1135 m out.
  * The flyabove's readiness (009C67B0) fires on either the 1.6 rad lead-bearing arm or `span <= 0`.
    Its lead range is a 3-second lead, which is shorter at the new 63-66 m/s flyabove speed than
    at the frozen throttle's 109 m/s. **Which arm fires for these two flights is not traced.**
* **Water.** Every U3 water contact is at 68.3 m/s after `done`, the same family as M0's 8.
  Neither the fighters nor any Val reach the water in a dive.
* **Hull offset (U2, U4).** It adds nothing at 9000 and loses 2 releases at 4500.

**Prediction scorecard:**
* 1 held.
* 2 held: entry at 51-54 m/s, and the same swing.
* 3 failed.
* 4 held only for clean entries.
* 5 held.
* 6 was partly right: the next blockers are upstream of the glide gates.
* 3a failed: 16, not 20-35.

## 5. Decision

* Releases do not return: 16 of 29 at 9000 and 8 of 17 at 4500. So **the flip does not land**.
  `kPilotThrottleSlotBound` and `kAimDiveTailBound` stay OFF.
* `kAimGlidePitchBound`, `kAimGlideYawBound` and `kReleaseAltitudeDrawBound` land **OFF**, as the
  brief says for a short flip. Main is therefore unchanged: the final build equals M0.
* The hull offset stays OFF.
* **The next missing terms**, in order of the releases they cost:
  1. **Mode B, 5 aircraft and 7 rounds at 9000.** Flights #3.1 and #7.1 roll into turndown at
     1420 m and open to about 1700 m during it. The first read is 009C67B0's two arms per tick for
     those flights, with the flyabove's lead range at 63-66 m/s, and then the turndown's heading
     (009C44F0) while the range opens.
  2. **Mode A, 3-4 aircraft and 6-8 rounds.** Wingmen bank past 1 rad late in aimdive, consistent with the
     roll command following the impact-point bearing (009C5AF1) as the CCIP distance goes to zero.
     The first read is how the image's aimdive roll arm behaves as `ccip_d` goes to zero, at a dive
     of -0.55 to -0.7 rad.

## 6. Step 3: the Yorktown leader at 1547 m and 34 m/s

**It is the flight model's ceiling, not the throttle scale.**
* **The throttle scale is not starving it.** In `T1_9000.log` the leader's `throttle_trace` shows
  pending = 0.100 (the dt) and output 1.000 on every think. The throttle is saturated. The image's
  scale is `dt / max(unit+340h × 0.4, 1)`, with unit+340h the CheatTurbo multiplier. That leaves dt
  unchanged unless CheatTurbo exceeds 2.5, and the host passes dt as is.
* **The ceiling force is the limit.**
  * `planeglobals` Ceiling = 1500 m, "above this height every plane stalls", and CeilingForce = 0.1
    m/s² per metre.
  * 007DBE0E-007DBEA8 (`src/plane_flight.cpp`) adds `-(altitude - 1500) × 0.1` to the vertical,
    and the same times a forward share of 00419010(1.5, 1, 0.5, 0, v / StallSpd) along the body.
  * The Wildcat (class 101) has StallSpd 17.5, so at 34 m/s the ratio is 1.94 and the share is 1.
  * At 1547 m the aircraft therefore carries -4.7 m/s² both down and along its nose, at full
    throttle. It climbed through 1500 m at 78 m/s in moveto (thinks 1100-1350), topped out at
    1552 m, and decayed to a 34 m/s equilibrium.
* **Open:** why the moveto holds it above the ceiling. That is the dogfight moveto's altitude
  command, not the throttle.
