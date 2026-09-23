# 009DE5B0's other sections, and the torpedo response's image terms

Packet `cc9_heading_target_sections`, 2026-09-23. Names are hypotheses. Offsets are relative to
blk = brain+8h. The section numbers follow docs/SHIP_AI_ARM_FINAL_STEP.md, whose reconstruction
`ship_ai_arm_final_step_009de5b0` (`src/ship_ai_arm_final_step.cpp`) already covers every section.

## 1. The sections the host still records

009DE5B0 runs at the end of every arm of 009ED6B0: 009EF206..009EF213, and the station arm's
009EE57B jump. Its order is 1, 2, 3, 4, 5, 6, 7, so section 5's torpedo override sees section 4's
escape turn, and sections 6 and 7 then work from section 5's result.

| section | listing | what it does to blk+324h | its producer in this host |
| --- | --- | --- | --- |
| 1, early out | 009DE5B6..009DE5E5 | nothing when \|0092D730\| <= 1.0 | bound (the torpedo packet) |
| 3, direction disagreement | 009DE67D..009DE6D7 | none directly. It sets `reverse` (the latch against the speed's sign) and the frame heading H, plus pi astern, for sections 4 and 6 | pure |
| 4, avoid-zone escape blend | 009DE6DB..009DE8ED | adds a turn of at most 30 degrees times the ramp product toward the escape direction blk+150h. Ramps: Interp(pi/4, 1, 80 deg, 0) of the heading error (00CEB5A8, 00CF8858); Interp(3, 0, 5, 1) of path left over max(100, length) (00CE3854, 00CE3850); Interp(0, 0.15, blk+3C8h, 1) of blk+14Ch (00CE7818). It raises blk+354h to 3.0 and unit+102Ch to 1.5 x product, and clears `allow_query` (009DE6F5) | **blk+160h, +14Ch, +150h come from 009ECA20, which the host records** (`ShipAi::step_009eca20`). blk+160h stays 0, so the section never enters |
| 5, avoidance override | 009DE8F1..009DE96C | replaces blk+324h with the avoidance vector's heading | bound (`kShipTorpedoResponseBound`) |
| 6, traffic separation | 009DE96C..009DEE07 | 1/7 of the folded bearing of the neighbours' summed push, clamped to +-1.8 x blk+3D0h (00CED5D8, 00D049A8) and bounded by the path turn side blk+304h | ~~needs blk+604h > 0. The neighbour list's only appender 009F0D20 is recorded (`ShipAi::neighbour_list_add_009f0d20`), so the count is 0~~ **Correction 2026-09-23:** bound. Packet cc9_ship_neighbour_list fills the list (009F1856, 009F0D20, 009F0EA0) and runs this section over it (`traffic_separation_009de96c`, the walk split out as `ship_ai_arm_final_separation_vector_009de96c`); docs/SHIP_NEIGHBOUR_AVOIDANCE.md section 3 |
| 7, free bearing query | 009DEE0B..009DF10F | blk+324h = query+1Ch when 009DC2E0 accepts | **009DC2E0 (009DC2E0-009DCEA2) is unread.** The host answers a recorded false (`ShipAiSectorScan::free_bearing_009dc2e0`) |

**Consequence.** In this host sections 4, 6 and 7 cannot change blk+324h on either mission: 4
because its gate byte has no producer, 6 because its list is empty, 7 because its callee is
unread. Binding the reconstruction now would add records and change no row, so the section-5-only
call stays and the sections stay records. Each is unblocked by one read:

- 009ECA20 (blk+160h, +14Ch, +150h, +30Ch). The scene does carry avoid zones: 3 TerrainGridLayers
  from the `.nav`.
- 009F0D20 plus its candidate walk, which needs the world object list at [[00E188A8]+19CCh].
- 009DC2E0.

**None of the sections feeds the torpedo response.** Section 5 reads only 009DA1D0, blk+354h and
the vector. Section 4 is the only other writer of blk+354h, and it is inert here.

## 2. The torpedo response's image terms (`kShipTorpedoResponseImageTerms`)

| substitution (docs/SHIP_TORPEDO_RESPONSE.md) | the image | now |
| --- | --- | --- |
| a live round is a candidate | slot38 008561F0 returns the record's active byte +458h, set once released (006E1331). slot2C 00855F00 returns 2 when the height record+100h is below 2 x swim depth record+47Ch, else the underwater flag record+354h (docs/TORPEDO_TICK.md). Together: released and in the water | `swimming` required |
| heading +46Ch from the velocity | 0085748A..008574A0 seeds +46Ch from the hull's own yaw on the first swim step (sentinel 10000.0f). 00856BB0 then steers toward it | **an equivalence, not a substitution, once only swimming rounds count.** A host round keeps the direction it entered the water with, and the host has no separate torpedo pose, so atan2(vx, vz) is the entry yaw |
| two timer draws on the first re-plan | 009F1160 makes seven stream-1 draws at construction: B3C, B40, B48, B50, B58, B44, B4C | the seven draws in that order when the draw source binds, before any gameplay draw. The host builds its controllers one call before the gunnery host binds (`game_hosts_mission_frame.cpp` 1468/1469) |
| 009DE5B0 sections 3, 4, 6, 7 as records | section 1 above | unchanged: inert in this host |

## 3. Predictions, written before the pair

USN04 4700/4500. Control `build/win32/hsC` (image terms off), treatment `build/win32/hsT` (on).
Both have the torpedo response on, come from one tree, and run with `BSP_GUNNERY_RNG_STREAMS=1`.

1. **Admissions fall.** A round is admitted only once it is in the water, not during its fall
   of about 1-2 s. Tracks form later by up to one scan period (1.5-2 s).
2. **The Yorktown's overrides stay near 179, give or take tens.** Consumption waits for
   observation (3.5-5 s of run time) either way, so the later tracks lose little. The
   different B44/B48 draws shift each ship's scan phase by up to 2 s. The largest turn stays
   about 1.4 rad, and the first override stays near 124.65 s, within a scan period.
3. **Ship rows are flat:** damage taken by entity, the Lexington (gate shut), and no ship death
   appearing or vanishing. Aircraft rows may move through the Yorktown's AA, as in the torpedo
   pair.
4. **The followers' station latches are flat.** Neither term reaches the station arm.
5. **Unimplemented calls are flat**, apart from the walk's call counts.

No USN04 9000 pair: no section touches the E2 torpedo picture.

## 4. Results (2026-09-23)

`local\hs_ctl_usn04.log` against `local\hs_trt_usn04.log`: the tree is main `c57c0d5fc` plus this
packet.

**The torpedo picture itself has changed since the torpedo-response pair.** Main now makes 4
torpedo releases in USN04 4500 instead of 13 (`torpedo task: releases=4`, with
`blocked_engaged=21678`), from merges this packet did not bisect. Kate #4's four rounds pass the
Yorktown at 157-184 m after runs of 28-60 s, and no other ship is aimed at. So in **both** runs
the Yorktown builds 4 tracks and consumes them behind an open gate (661 and 635 times) but never
forms an avoidance vector, and no ship overrides its heading.

| row | control | treatment |
| --- | --- | --- |
| admissions, all ships / tracks built | 32 / 17 | 16 / 4 |
| Yorktown admissions / tracks / overrides | 19 / 4 / 0 | 16 / 4 / 0 |
| Yorktown scans | 119 | 137 |
| brain constructor draws | - | 18 controllers x 7 |
| deaths, ship rows, station rows | - | identical |
| unimplemented calls | 3377720 | 3377720 |

Against the predictions:

1. **Admissions fall: met.** Every non-Yorktown admission in the control was a round still in
   the air.
2. **The Yorktown's overrides near 179: not testable.** The control has 0, because of the
   release count above, not this packet.
3. **Ship rows flat: met.** Nothing appears or vanishes, and even the aircraft rows are identical.
4. **Station latches flat: met.**
5. **Unimplemented calls flat: met.**

**Decision.** `kShipTorpedoResponseImageTerms` lands ON. It removes two labelled substitutions,
and the pair shows no row it moves except the admissions it is meant to move.
