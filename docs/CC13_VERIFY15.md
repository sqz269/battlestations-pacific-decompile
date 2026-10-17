# CC13_VERIFY15: labelled substitutions in game_hosts_units.cpp lines 1-20000, continued

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify15 on 2026-10-17 against
base b655d94de. It continues CC13_VERIFY11 over the line ranges that pass listed under "Not covered"
in `src/game_hosts_units.cpp` lines 1-20000 (cc13-verify12 takes 20001 to the end). The file is
unchanged since VERIFY11's base 56f4b923a, so the line numbers agree with that doc. No code was
changed and no ledger records were added. "GHU" is `src/game_hosts_units.cpp`. "Image" means
battlestationspacific.exe, read with `bsp.py disasm-raw` and `bsp.py ghidra bytes`. Script
references are to this installation's `scripts/` tree, which is modded.

Excluded, as briefed: `squadron_free_attack` wiring and the dying-spin / stall / rack switches of
cc9-physics14/15 (the plane death-mode block at `:18756-18790` and the water-contact / depth-kill block
at `:19600-19910` are left alone for that reason and because 007CB7F0 is frozen). Items already in
CC13_VERIFY1..13 and CC13_VER_* are not repeated. Items already recorded in other docs (the RNG census,
PLANE_SUBSTITUTION_SWEEP, PLANE_FOLLOW_ENTER) are noted only where this pass resolves or reclassifies them.

Rubric (cc13 common brief), as in VERIFY11:
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width with no effect, presentation, or RNG
  draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Census

VERIFY11's keyword sweep, restricted to its uncovered ranges (`:1-160`, `:500-2200`, `:4400-5390`,
`:5560-6000`, `:7700-7960`, `:8000-8700`, `:8830-9440`, `:12180-13550`, `:13600-13880`, `:14360-14700`,
`:14850-16000`, `:16480-17160`, `:18100-20000`), matched 120 lines. A second sweep over the same ranges for
`is unread|are unread|not carried|taken as ...|LABELLED` matched 124 lines, many overlapping. About 70
hits were read against their code, and the per-tick and label-premise ones against the image or the
installed scripts. The rest are notes on how a body was read, log text, or records placed after the
write they name.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | GHU `:1973` label; the feed is `tin.speed_scale = 1.0f` at `:35175-35178` (verify12's range, not in any VERIFY doc) | 0099D300: 0099D75C-0099D79A decays plan+2B8h, 0099D924-0099D970 raises it on nose-down pitch, 0099D99E-0099D9A3 divides the measured speed by it | every mode-1 speed hold with plan+2B0h clear while nose-down: dive-bomb approach, moveto, follow (all rows; the USN01/USN04 Val dives most) | none |
| R2 | GHU `:8108-8110`: the kind-13h gunfire-avoidance scaling "not modelled (no USN04 attacker is kind 13h)" | 0099EFA5-0099F029: kind 13h is MPlaneFighter; timer x f, rest / f, f = max(1 - level x 0.3, 0.4) | every fighter held in a firing plane's gun cone (dogfights) | none extra |
| R3 | GHU `:9023-9028` `gin.state_88 = 1.0e30f` | 009BED80 latches state+88h = Pilot/Follow/SafeAlt (60); band 009C16E5-009C175A | every follow-state wingman | none |

### R1 The speed-hold divisor plan+2B8h is pinned at 1.0

The label at `:1966-1975` says "The host's throttle rule substitutes 1.0 for plan+2B8h". The feed at
`:35175-35178` says the producer "0099D756-0099D79A and 0099D970, is unread". It has now been read:

- 0099D75C-0099D79A: when +2B8h > 1.0f (00D7A24C), +2B8h -= tuning+B8h (TrgSpeedCorrMulDecay) x
  [ESP+6Ch] (the step); 0099D794-0099D79A floor it at 1.0f.
- 0099D79F-0099D7AF: +2D8h != 0 goes to 0099D8C1. Mode 1 below the one-shot threshold
  (0099D8C6 `COMISS xmm0,[ESI+2B4h]`, 0099D8CD `JBE 0099D924`) reaches 0099D924. Modes 0 and 2+ go to
  0099D8F5 and never reach it.
- 0099D924-0099D970: with byte +2B0h clear, +2B8h = 00415550 max(+2B8h, 00419010(x0 = -0.0 (00D7A208)
  - tuning+B0h, y0 = tuning+B4h, x1 = 0, y1 = 1.0, x = [ESP+10h])). 00419010 is
  BSP_Math_InterpolateClamped(x0, y0, x1, y1, x). [ESP+10h] is the angle 0099D50A takes the cosine
  of, which the host's own label (`:1971`) reads as the pitch.
- 0099D99E-0099D9A3: measured speed 007D99C0(unit+AB0h), `FDIV [EBP]` with EBP = &+2B8h.

This installation's planeglobals.lua `:405-407` authors TrgSpeedCorrMinPitch = DEG(70),
TrgSpeedCorrSpeedMul = 2.6 and TrgSpeedCorrMulDecay = 0.8. At 70 degrees nose-down the image divides
the measured speed by 2.6. After the pull-out the divisor decays at 0.8 per second, so it lasts about
2 s. The host never divides.

- **MISMATCH REAL.** In a dive under a mode-1 speed hold, the image sees itself slower than it is
  and keeps or adds throttle. The host sees its true speed and cuts throttle.
- **LIVE:** kPilotThrottleSlotBound is ON (`:7190`). The writers that set +2B0h = 0 with +2D8h = 1
  include the dive-bomb approach 009C6FEA (`:27678`), moveto 009C1C0D (`:22977`), follow 009BFD15
  (`:9161`), 009C2743 (`:34081`) and 009C2503 (`:34179`). Aim (+2B0h = 1, `:31467`, `:31573`,
  `:32021`) is unaffected, as docs/FIGHTER_ROLL_TRACE.md section 3 found.
- **RNG:** none.
- A second difference was seen but not settled: the host's measured speed is |world velocity|
  (`:35170-35173`), and the image's is 007D99C0(unit+AB0h). Whether that is the forward speed was not
  checked.

### R2 Fighters' gunfire-avoidance timers are not scaled

The label at `:8108-8110` says the kind-13h arm is "not modelled (no USN04 attacker is kind 13h in this
host's class table)". docs/PLANE_SUBSTITUTION_SWEEP.md:20 left it Open. Kind 13h is MPlaneFighter
(`src/entity_class_ids.cpp:31`). The evader in 0099EC40 is any plane within 800 m in a firing plane's
gun cone (`:8067-8079`), so the arm is reached in every dogfight.

Image 0099EFA5-0099F029, right after the two 00BD2F10 draws into +3E8h and +3ECh:
- 0099EFB8-0099EFBE: unit vtable[5Ch](13h), skipped when false.
- 0099EFC8-0099EFEF: f = 1.0f, or with +2F4h and its +DF4h present, f = float(1.0 - fild([+DF4h]+34h)
  x 0.30000001192 (double 00CE3DC8)). [+DF4h]+34h is the robot level index (009F9D0E uses it to pick
  the 248h-stride row).
- 0099EFF3-0099F00B: if 0.4 (double 00CE65D0) > f, then f = 0.4f (00CE7804).
- 0099F015-0099F029: +3E8h *= f; +3ECh = +3ECh / f.

So a fighter at SPNormal (level 1) holds its evasion 0.7 x as long and rests 1.43 x longer. At level 2
or above it is 0.4 x and 2.5 x. At Stun (level 0) nothing changes.

- **MISMATCH REAL.**
- **LIVE:** kPilotGunfireAvoidanceBound is ON (`:6164`). Every fighter that a firing plane holds in its
  gun cone, on the rows with fighter combat. USN04 flags only Vals (ATTACKER_EVASION.md 5.4). The host
  has the level as `db_skill_row_14` (kSkillLevelBound).
- **RNG:** no extra draws. Only the stored values change.

### R3 The follow band floor's state+88h is SafeAlt, not 1e30

`:9023-9028` passes `gin.state_88 = 1.0e30f` because "this host has no field for it".
docs/PLANE_FOLLOW_ENTER.md section 8 found the only writer, 009BEDBB, but not the value.

Image:
- 009BED84-009BED92: 009BE150 BSP_BotFollowParameters_Copy (7 floats, 009BE156-009BE17B) copies
  singleton+380h into the block at [state+6Ch].
- 009BED9A-009BEDBB: state+88h = [[state+6Ch]+8] = singleton+388h. That is Pilot/Follow/SafeAlt
  (`include/bsp/game_tuning_singleton.hpp:270`), authored 60 in this installation's planeglobals.lua
  `:498`. Its neighbours are FollowedPointDist 250 (`:496`) and LeaderFollowAlt 10 (`:497`).
- 009C16E5-009C1725: L = min(leaderY + block+4 (10), state+88h).
- 009C171B-009C175A: L = max(L, leaderY - 120.0 (double 00D1F3F8)). There is no branch around this
  step.
- 009C17C6-009C183B: the station Y (state+34h) and the steer Y (state+48h) are clamped into [L, cap]
  (docs/PLANE_FOLLOW_LAW.md 5.9).

So the image's L is max(min(leaderY + 10, 60), leaderY - 120). For a leader above 50 m that is
max(60, leaderY - 120). The host's L is leaderY + 10. The host also applies the -120 bound only when
`in.run_phase_a` (`src/plane_follow_law.cpp:659`). The catch-up arm (`:8401-8406`) floors at
leaderY + LeaderFollowAlt in the same way.

- **MISMATCH REAL.** The host lifts every station or steer Y below leaderY + 10 to leaderY + 10, and
  the image passes it. The spent-member done floor is max(60, leaderY - 120) in the image, not
  leaderY + 10. The label's "can only RAISE the floor" is accurate, but it applies on every follow
  tick where the station or steer point is at or below the leader.
- **LIVE:** kPlaneFollowLawBound is ON (`:5732`). Every follow-state wingman, including the torpedo and
  dive-bomb follow ticks (009D2731 and 009C7278 -> 009C1FD0), on all rows with squadrons. The size
  depends on the formation's station Y offsets, which were not measured.
- **RNG:** none.

## Other findings

| # | Host | Class | Evidence |
|---|---|---|---|
| S1 | GHU `:4515-4520`, `:4562-4566`, `:4586-4590` "the aim point this host carries is the commanded target's position" | STALE LABEL | `tp` is `tp_hull`, filled by `aim_point_009fada0` (`:4476-4484`): the 009FADA0 hull point plus lead. The break-off range, the spent-member leader test and the impact point already use it. |
| S2 | GHU `:4613-4617` "this host draws none of 009C3DA0 ... the draw's mean, 0" | STALE LABEL | `kAimErrorDrawBound` is ON (`:2342`). `redraw_aim_error` (`:9258-9275`) makes the three draws, stored at `:26231` and `:26464`, and `:4620` feeds `db_aim_time_error_c8`. |
| S3 | GHU `:4703-4850`, the "NOT WIRED" and pinned-mode narrative before `in.engaged.control_mode_370 = 2` | STALE LABEL | `kDiveAttackModeFedBound` is ON (`:7223`). `:4852-4854` feeds `db_attack_mode_370`, so the pin is dead on the live build. |
| S4 | GHU `:4879` `b.speed_ratio_41c = 1.0f`; `:27732-27737` bank-limit arm takes approach+24h as 1.0 | MATCH for this installation | The image's approach+24h (task+41Ch) is 009F9D37-009F9D61 max(1.0, class+188h MaxSpd / reference), with reference = tuning+4D8h (009C3EBF-009C3ED5, DiveBomb ReferenceSpeed KMH(280) = 77.78). The dive bombers' MaxSpd in this installation's vehicleclasses.lua are SB2C 72.22 (`:21066`), SBD 69.44 (`:46454`), D3A 69.44 (`:54101`) and D4Y 72.22 (`:54394`), all below 77.78, so the ratio is 1.0. It would differ for a class with MaxSpd above 77.78 flying the dive-bomb task (a fighter-bomber, if any does). That was not established. |
| S5 | GHU `:5148-5152` "that remains a labelled SUBSTITUTION" (the bearing from the latched D8h/E0h point) | STALE LABEL | `:5169-5172` uses `db_impact_bearing_18` and `db_impact_planar_5c`, the impact-point pair 009C5950 / 009C5960. |
| S6 | GHU `:8198` "007D7A40 after a changed pitch or yaw: unread, not modelled" | STALE LABEL | `finish()` in `gunfire_repair_0099bf30` (`:8148-8158`) arms 007D7A40's 1.2 s floor timer under `kPlaneSpeedFloorBound` (ON, `:5852`). |
| S7 | GHU `:8999-9003` (007D7DA0 turn rate "unread"), `:9089-9095` (007C47F0 and the classDesc+188h floor "not carried"), `:9127-9132` (009F9ED0 "unread") | STALE LABEL | Each is followed by its bound arm: `kFollowLeaderTurnRateBound` (`:6122`), `kPlaneFollowCatchupBound` (`:5766`, used at `:9097-9101`) and `kPlaneFollowFlyToPitch` (`:6454`, `:9149`), all ON. |
| S8 | GHU `:14537` "planeDesc+158h is 0.0 (kPlaneDesc158)" | STALE LABEL | `kPlaneDesc158Bound` is ON (`include/bsp/game_hosts_units.hpp:109`). The launch-spot slide calls `plane_desc_158(p)` (`:14564`). The constant `kPlaneDesc158` (`:14126`) has no reader. |
| S9 | GHU `:15194-15195` "Unbound, the channel stays disabled and this is true" | STALE LABEL | `kPlaneWingFoldElevatorBound` is ON (`:5951`). The wing test `wing_at_007b8d10` follows it at `:15196-15200`. |
| S10 | GHU `:19951-19961` "on a freshly constructed plan [0099BF30] writes NOTHING ... the pass-through here is well-founded" | STALE LABEL | The gunfire avoidance inserts bands (`ga_insert_0099b790`, `:8130-8138`), and `gunfire_repair_0099bf30` runs at `:34621`. The pass-through premise no longer holds, but the repair is hosted. |
| S11 | GHU `:19021-19028` "TurboMultiplier when ctl+4h and desc+604h when ctl+5h (neither modelled)" | STALE LABEL for the turbo; UNRESOLVED for ctl+5h | The turbo is applied under `kFollowTurboBound` (`:19029-19039`). ctl+5h x desc+604h is not applied, and unit+CC8h / 008E6430(6) are taken as 1.0 (known: docs/OCEAN_WAVE_FIELD.md D5, no disp32 writer of unit+CC8h). See U2. |
| S12 | GHU `:18329-18332` "With the flat-sea stand-in that is a test of the authored hull height against zero" | STALE LABEL (log comment only) | `src/ship_motion.cpp:302` takes `host.ocean_height` at the keel point, which is the live field. |
| S13 | GHU `:504-510` "Nothing writes them yet ... no plane turns" (plane_body_angular) | STALE LABEL | The rate law and core step write the body rates every step (for example the follow-hold attitude reads them at `:8527-8529`). |
| S14 | GHU `:5913-5917` "the third reader, 0099D558's \|B1Ch - \|AE0h\|\| into [ESP+48h], is not carried" | MATCH (dead store) | 0099D549-0099D593 store \|vt[38h]() - \|AE0h\|\| to [ESP+48h]. A scan of 0099D300-0099DF00 finds no later read of [ESP+48h] at that ESP (0099D931 `SUB ESP,14h` is restored by 00419010's RET 14h before 0099D961). |
| S15 | GHU `:8515-8517` unit+C70h taken as 0 | MATCH (installation values) | Its only consumer is rf_hdgV, authored 0, as the label says. |
| S16 | GHU `:8523-8529` ctl+A0h..A8h replaced by the body angular velocity | MISMATCH MINOR, known | docs/PLANE_FOLLOW_LAW.md:693 records it. Only pf_pitchV has a non-zero gain. Whether ctl+A4h equals ctl+48h was not checked. |
| S17 | GHU `:8225-8235` 009C23B0's target speed from `motion.linear_velocity` | MATCH | For a plane target that field is synced from `plane_world_velocity` every core step (`:19576-19579`), so it equals 007B8E60's \|v\|. |

## UNRESOLVED

| # | Host | What is missing |
|---|---|---|
| U1 | GHU `:8383-8385` `in.sight_active_84 = false` | 009C211D-009C22F5 is a periodic search (timer +74h against the +70h period, latch +85h, 007B8EE0 on sq+3D0h, a loop over 008053C0) that sets +84h / +78h / +7Ch / +80h. It was not read far enough to say what the follow law does with an active sight. |
| U2 | GHU `:666-668` `plan_boost_2e4` "Recorded only: its reader 007BB7B9 is not hosted", written by the kamikaze arm (`:25468-25471`) | The request runs plan+2E4h -> cmd+14h -> unit+A10h -> unit+9F8h..9FAh (docs/PILOT_PLAN_SLOT_PIPELINE.md:397). The thrust arm's ctl+5h x desc+604h (S11) may be its effect. Only kamikaze rows are concerned, and none of the five test rows is one. |
| U3 | GHU `:8034-8037` the gunFire byte +BC9h stands in for lastGunState +C35h in the 00F87278 shooter list | Whether +C35h (007C74A0 / 007C75A0 from 007CC345) and +BC9h differ (for example on an empty or reloading gun) was not read. |
| U4 | GHU `:9023-9028` follows R3: the host's -120 bound applies only in phase A | 009C171B-009C175A has no branch around it. Whether the host's non-phase-A caller corresponds to an image path that skips it was not checked. |
| U5 | GHU `:14537-14540` airfield launch spot: the height is path point 0's y, and the airfield's axes are taken as world | The image takes y from [airfield+7ACh]+9Ch or the ground. Only airfield launches are concerned. |
| U6 | GHU `:13637-13642` 006D40F0: of message 7Dh only +720h is modelled, and 006D2980 is not run | 006D4180-006D41F3 (an 00BF7680 format, vtable[194h], 00419CC0 / 00BD1510 post) was not read. |

Known and not repeated: the 009C6114 aimdive re-arm draw pinned at 0.5 (`:5114-5119`,
docs/CC13_RNG_STREAM_CENSUS.md:523), and OnDestroyed's +C14h stream-0 draw for a stowed plane
(`:14934-14935`, CC13_RNG_STREAM_CENSUS.md:153).

## Not covered

- Read only for labels, with no image check: `:1-160` (no hits), `:1000-1100` (dogfight fields; their
  feeds are in verify12's range), `:4400-4500`, `:5560-5680`, `:5700-5830`, `:7700-7960`
  (`kPlaneFormationPlacementEnabled`, judged in VERIFY8), `:12180-12550` (land convoy roster: the
  00922DE0 property-bag copy is not modelled), `:12880-13550`, `:13600-13880`, `:14360-14500`,
  `:14600-14700`, `:15200-16000` (landing sequencer, base launch timing `:15788-15814`), `:16480-16700`,
  `:16750-17160`, `:18100-18320`, `:18340-18700` (ship terrain contact; exit-zone role 0081BEB0 is in the
  struck range).
- `:18756-18790` and `:19600-19910` were skipped as excluded (death modes, water contact / depth kill).
