# The dive's flight response: class Accel is scaled in the image, raw in the host

Addresses: 007DB680, 007DB875, 007DB990, 007D92B0, 007D8470, 007DA710, 007D9050, 007D9140,
007D20C6-007D2144, 007C4984-007C499C, 007C47F0.

Packet `cc9_dive_flight_response`. Every name is a hypothesis, not a recovered symbol. Background:
`docs/TURNDOWN_EXIT.md` section 3 named the flight model at 35-55 m/s as unread. Run V1 is
`local\V1_9000.log`.

## 1. What integrates the speed, and what the host binds

These are per fixed step, in `src/plane_flight.cpp` and the free-flight block of
`src/game_hosts_units.cpp`, with the reads in `docs/PLANE_FLIGHT_CORE_LAW.md`:

| term | image | host |
| --- | --- | --- |
| thrust, body forward | 007D9050 via 007DB744-007DB80A: class+164h Accel × throttle, gated on throttle > 0.01, × the fall cheat (FallMul 2.6 over pitch 10°-60°), clamped to [0, 100] | bound. The unit+0CC8h and 008E6430 multipliers are taken as 1.0 (labelled) |
| drag, along velocity | 007D9140 via 007DBA32-007DBC76: -v² × k × class+50Ch × (1 + AirBrakeDrag × brake), with k = (1 - throttle)² × GlideRate + 1 + DragPitchRatio × \|elevator\| and a pitch ramp | bound, with class+50Ch taken as Accel / MaxSpd² (007C4984-007C499C) |
| lift, body up | 007DB875, (1 + AoA) × min(q², 1) × AccelCheatMul × 9.81 | bound |
| gravity, world down | 007DB990, AccelCheatMul (1.5) × 9.81 | bound |
| body damping | 007D92B0 × XDrag/YDrag on the lateral and vertical body components | bound; it does not act along the path |
| control rates | 007DA710 | bound (`docs/DIVE_MODES.md` 2) |

**The divergence is class+164h.**
* The image's class reader loads Accel at 007D20D8/007D20DD. Then at 007D20F8-007D2127: if
  tuning+31Ch AccelCheatMul > 1.0 (007D2100 COMISS against 1.0 [00D7A24C], `JBE` to the clamp),
  **class+164h = tuning+320h AccelCheatMulMul × tuning+31Ch × Accel**, stored back in place.
  Otherwise +31Ch is set to 1.0 and Accel stays raw.
* 007C4850 then derives the drag coefficient class+50Ch from the scaled +164h (007C4984 FLD +164h,
  007C4990/007C4992 two FDIVPs by MaxSpd, 007C499C FST +50Ch).
* This installation: AccelCheatMul 1.5 and AccelCheatMulMul 1.15 (`planeglobals.lua` lines
  265-266), a factor of **1.725**. The D3A Val's Accel of 6 becomes **10.35**.
* The host sets `plane_accel` to the raw row value (6) and uses it for thrust, for the drag
  coefficient, and for the climb angle 007D98F0.
* `plane_class_accel_007d20f3` already reconstructed the scaling but was never called.

## 2. The model, checked against the host

`local\dive_model.py` integrates the along-path law with the host's own terms, at the trace's
attitude, throttle and brake per think (dt 0.1 s), from the V1 aimdive entry of D3A Val #1.1.

| t | pitch | throttle, brake | host trace | model, Accel 6 | model, Accel 10.35 (image) | h |
| --- | --- | --- | --- | --- | --- | --- |
| 1339 | -1.076 | 0.197, 0.504 | 53.6 | 53.6 | 53.6 | 844 |
| 1354 | -0.996 | 0.197, 0.504 | 65.0 | 65.7 | 61.8 | 763 |
| 1369 | -0.667 | 0.197, 0.504 | 69.6 | 70.8 | 62.8 | 681 |
| 1399 | -0.465 | 0.197, 0.504 | 64.7 | 65.9 | 54.6 | 585 |
| 1424 | -0.593 | 0.354, 0.118 | 65.2 | 65.8 | 54.4 | 504 |
| 1444 | -0.689 | 0.197, 0.354 | 70.0 | 70.6 | 59.4 | 420 |

* The Accel-6 model tracks the host within 1.5 m/s over the whole dive. So the host integrates its
  own terms as written, and **the one divergent input is Accel**.
* With the image's 10.35 the drag coefficient rises 1.725 times while gravity does not. The same
  dive runs **8-11 m/s slower**, not faster. The divergence does not explain the lost releases:
  the image's own law makes the dive slower still.

## 3. A Val entering at 34.5 m/s

This is a straight dive from 844 m under the aimdive tail's entry command (power 0.197, brake 0.504),
run to h = 350 with the same model.

| attitude | host (Accel 6): v at 350 m, terminal | image (Accel 10.35): v at 350 m, terminal |
| --- | --- | --- |
| -1.08 rad (62°) | 80.2, 82.5 m/s | **66.8, 67.1 m/s** |
| -0.70 rad | 71.2, 71.9 | 59.0, 59.0 |
| -0.50 rad | 62.6, 62.8 | 51.8, 51.8 |

* The dive reaches its terminal speed well before the release altitude, so the entry speed (34.5
  or 54 m/s) does not matter.
* In the image a Val reaches the release altitude at **about 67 m/s at 62°**, and never the 134
  m/s of the frozen-throttle host.
* The release is reachable only if the dive stays steeper than -60°. The 009C5B43 abort cannot
  fire then (it needs pitch > -60° [00D20338]), and the aimdive release needs only altitude < +A8h
  and \|error\| < 25 m.
* Speed is not the blocker. The attitude is: the aim law pitches the host's dive up to -0.4 to
  -0.7 (`docs/AIMDIVE_ENTRY.md` 2), where the abort takes it.

## 4. Predictions, written before the pairs

The binding is `kPlaneAccelCheatScaleBound`. It changes every aircraft's class Accel, so it acts
with the throttle fix off as well: full-throttle acceleration and the climb angle 007D98F0 both
move. Both pairs are therefore run: **R0/R1** in the V1 configuration (the lead's pair) and
**S0/S1** in main's configuration. All four are E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, built in
this tree, and each pair differs only by the switch.

1. **R1 against R0.** The dives are 8-11 m/s slower, at 50-60 m/s. Dive releases stay near R0's
   (16 ± 5), with no systematic recovery. Water contacts are about the same.
2. **S1 against S0.**
   * Planes accelerate 1.725 times harder at full throttle, and the climb angle 007D98F0 grows.
   * The frozen-throttle dives meet more drag. The terminal speed at full throttle and -62°
     (thrust × FallMul 2.6, no brake) goes from 152 to 136 m/s. The observed 132-135 m/s releases
     are short of terminal, so they drop only slightly.
   * Every aircraft path moves. Releases move by ±6 from 32.
   * This is a broad, faithful change that I cannot explain row by row, so on main it would land
     OFF unless S1 is at least S0's 32 with no new water contacts.
3. **The fighters** reach their slot speeds faster, so the dogfight rows move.

## 5. Results

All runs are E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, built in this tree from main 327366749.
Logs are in `local\`.

| run | binary | configuration | drops | kill credits | water | mission end |
| --- | --- | --- | --- | --- | --- | --- |
| S0 | `frS0` | main | 32 | 31 | 8 | failed 228.06 s (`S0fr_9000.log`) |
| S1 | `frS1` | main + Accel scale | 24 | 34 | 6 | failed 231.06 s (`S1fr_9000.log`) |
| R0 | `frR0` | V1 set | **not run**: the renderer-init crash (0xC0000005 after `online_manager_initialize`, probes `probeJ`/`probeK`) returned at 12:27 | | | |
| R1 | `frR1` | V1 set + Accel scale | **26** | 35 | 8 | failed 258.06 s (`R1fr_9000.log`) |

For R0 the nearest reference is `local\Q0_9000.log`, the same V1 set built from main 7d1ff2166
(`docs/AIMDIVE_ENTRY.md` 4): 16 drops, 37 kills, 7 water. The V1 set gave 16 in three trees
before that (U3, V1, Q0). **That is a reference, not a same-tree pair.**

**R1, the V1 set with the image's Accel.**
* 26 releases, 13 aircraft with a 2-round salvo each, at 250-319 m and 72-93 m/s. All come from
  the glide. #3.1|.-2 and #7.1 release on a later pass, after first dives that ended at the floor.
* **Mode A is gone.** #1.1|.-3, #1.1|.-4 and movieval|.-3, the three wingmen that pulled out on
  the glide's bearing gate in U3/V1, now glide cleanly (29-38 calls, blocked on the ceiling only)
  and release.
* **Mode B remains.** #3.1, #3.1|.-3, #3.1|.-4 and all of #7.1 except its leader still dive from
  1000-1800 m and end at the floor.
* **Prediction 1 failed on the count.** I expected no systematic recovery, and got 26 against the
  16 reference. The release speeds of 72-93 m/s are higher than the straight-dive model's 55-67,
  because they are glide releases under the glide's MaxPower 0.7, not dive releases.
* Why the wingmen's late bank disappears is not traced. The slower entry (the image's drag) changes
  the aimdive's CCIP geometry at the moment the wide roll band acts. **It needs the R0 pair and a
  roll trace to state.**

**S1, main with the image's Accel.**
* Prediction 2 held on speed: the frozen-throttle releases fall from 126-135 to 116-125 m/s.
* It lost 8 releases: #3.1|.-2 (1), #3.1|.-3 (2), #7.1|.-2 (2), and one each for the three
  moviewals.
* The water contacts are 6 against 8. Two of them are Vals at 26.4 m/s after `done` (#1.1|.-2 and
  #5.1|.-2), which S0 does not have.
* Not explained row by row.

## 6. Decision

* **The divergence is real and bounded.** class+164h is Accel × AccelCheatMulMul × AccelCheatMul
  (007D20F3-007D2127). The host used the raw row value. It is bound as
  `kPlaneAccelCheatScaleBound`, which calls the existing `plane_class_accel_007d20f3`.
* **It lands OFF.**
  * On main it costs 8 of 32 releases, not explained row by row.
  * Its V1-configuration pair lacks the same-tree control R0, because the runs are blocked
    environmentally.
  * The R1 result (26 against a 16 reference) is the strongest single step toward recovery in the
    throttle-fix regime so far. It should be taken with R0 before the flip set is judged again.
* **The 34.5 m/s entry does not block the release in the image's law.** The dive reaches about
  67 m/s at 62° by the release altitude. What decides it is staying steeper than -60° so the
  009C5B43 abort cannot fire.

## 7. R0 taken, 2026-09-23 (packet cc9_fighter_gunfire_rate)

A 120-frame probe passed at about 12:30, so R0 ran with the binary `frR0` from the same tree as R1:
`local\R0fr_9000.log`, E2 9000 with `BSP_GUNNERY_RNG_STREAMS=1`.

| run | configuration | drops | kill credits | water | mission end |
| --- | --- | --- | --- | --- | --- |
| R0 | V1 set | **16** | 34 | 7 | failed 297.05 s |
| R1 | V1 set + Accel scale | **26** | 35 | 8 | failed 258.06 s |

* The pair is same-tree and differs only by `kPlaneAccelCheatScaleBound`.
* R0 releases from the same 8 aircraft as U3 and V1: the #1.1 and #5.1 flights' clean members and
  movieval / movieval|.-2.
* R1 adds 10 rounds:
  * the three Mode A wingmen, #1.1|.-3, #1.1|.-4 and movieval|.-3, 6 rounds;
  * #3.1|.-2 and #7.1 on later passes, 4 rounds.
* **Under the throttle fix, the image's class Accel is worth 10 releases.** It stays OFF only because
  of its main-configuration pair (S0 32 against S1 24). The two belong together: with the frozen
  throttle, the extra drag slows dives that the host only won by being too fast.
