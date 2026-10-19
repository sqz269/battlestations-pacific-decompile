# CC13_VERIFY8: labelled substitutions in the plane-side task hosts

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify8 on 2026-10-17 against
base 44242d5c7. It covers the plane task files in `src/` (`plane_*.cpp`, `dive_bomb_*.cpp`,
`torpedo_*.cpp`, `air_operations.cpp`, `attack_commands.cpp`, `bot_tasks.cpp`) and, because those files
are pure laws, the arms in `src/game_hosts_units.cpp` that feed them: the level-bomb arm (009B8B50,
lines 23990-24710) and the dive-bomb arm (009C8790, lines 25719-27740). Earlier passes left these two arms
out (CC13_VERIFY1 covered dogfight, strafe, kamikaze and torpedo). No code was changed and no ledger
records were added. Line numbers are against the base. "GHU" is `src/game_hosts_units.cpp`. "Image"
means battlestationspacific.exe, read with `bsp.py disasm-raw` and `ghidra bytes`. Script references are
to this installation's `scripts/` tree, which is modded.

Excluded, as briefed: `src/squadron_free_attack.cpp` (cc9-physics15's packet) and
`rotate_about_axis_0085e4d0` in `plane_advance_pose.cpp` (already fixed). This pass does not repeat items
already in CC13_VERIFY1 / VERIFY4 / MOVETO1 (the issue-interval draw, the dogfight reselect draws, the
009C31B0 keep test, the chooser's `unit_is_kind_of(0Fh)`). Where an item here belongs to one of those
classes, it is named under "same class".

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width with no effect, presentation, or RNG
  draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Census

A keyword sweep over the listed files matched
`substitut|approximat|stub|simplif|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|fixed value|unresolved|unimplemented|not modelled|unread|assumed|provisional`
on 21 lines in 12 files. Most of them are notes on how a body was read, not substitutions. The same
sweep over the two GHU arms (with `STAND-IN|PARTIAL|pins|low end|LABELLED|record\(` added) gave 11 lines
in the level-bomb arm and 33 in the dive-bomb arm. The OFF switches in GHU and
`include/bsp/game_hosts_units.hpp` were listed, and the plane-related ones were read:
`kDiveBombMoveToOriginBound`, `kDiveHitClockBound`, `kDiveReleaseAltActivationOnlyBound`,
`kMoveToActivationLuaAltOrderBound`, `kPlaneFormationPlacementEnabled`, `kLandAbortGroundArmBound`,
`kRackUnequippedEmptyBound`, plus `kTorpedoArmFallthroughReleaseBound` in
`include/bsp/torpedo_task_arm.hpp`. The last five carry pair verdicts or refutations in their comments
and were not re-judged.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | GHU `:26928`, `:26774-26775` dive-bomb goaway draws fixed at the low end (URGENT, goaway) | 009C49FC U(1.0, 1.25) x standoff; 009C4D13 U(3.0, 6.0) bank window; 009C4D33 U(12.0 [00CEB4B8], 15.0 [00CE5380]) countdown | every dive-bomb goaway (USN01 and USN04 Vals) | image 3 draws on stream 1; host none |
| R2 | GHU `:24019`, `:26933` goaway turn side from the arm's tick parity (URGENT, goaway) | 009B5680 and 009C4950: `[00F876B0] AND 80000001h`, odd gives -1.0 | every level-bomb and dive-bomb goaway | none |
| R3 | GHU `:27345-27349` dive flyabove heading has no avoidance increment | 009C6B7B-009C6CB6 build `[ESP+20h]` from the 007F0280 probe; 009C6D54 adds it to the heading | dive-bomb flyabove with wingmen in the probe box | none |

### R1 Dive-bomb goaway draws fixed at the low end

`run_dive_bomb_goaway_enter_009c4950` sets `in.standoff_jitter = kStandoffJitterLo` (`:26928`, comment
"the low end"). The goaway tick's re-roll sets `rin.window_draw = kBankWindowLo` and
`rin.countdown_draw = kCountdownLo` (`:26774-26775`, comment "as this host pins every draw").

Image:
- 009C49E5-009C49FC: `FLD [00CF29A8]` (1.25) into [ESP+4], `FLD1` into [ESP], `ECX = 1`,
  `CALL 00BD2F10`, then `FMUL [ESI+20h]`.
  The standoff is U(1.0, 1.25) x max(2 x approach+B4h, target extent).
- 009C4CFB-009C4D18: `00BD2F10(3.0 [00CE3854], 6.0 [00CE6630])` with `ECX = EBX`, stored to +2Ch (the bank
  window).
- 009C4D1E-009C4D3E: `00BD2F10(12.0 [00CEB4B8], 15.0 [00CE5380])` plus +2Ch, stored to +24h (the countdown).

Host: the standoff is always the minimum, and the bank window is always 3 s. The countdown is always the
low end of its range. The level-bomb enter at `:24025` does draw (`lb_draw`, a keyed stream), so the two
goaway arms differ.

- **MISMATCH REAL.** Dive-bomber goaways are always the shortest, about 11% short on average.
- **LIVE:** every dive-bomb release.
- **RNG:** three draws on 00BD2F10 stream 1 per enter and re-roll in the image; none in the host.

### R2 Goaway side from the arm's tick parity

Level bomb `:24016-24019` and dive bomb `:26929-26933` (both labelled STAND-IN) take the side from
`lb_arm_ticks & 1` / `dive_bomb_arm_ticks & 1`. Their premise is that the host has no mission step
counter.

Image 009B5680 (`BSP_BotStateLevelBombGoAway_Enter`) and 009C4950 (`BSP_BotStateDiveBombGoAway_Enter`)
both open with `MOV EAX,[00F876B0]` / `AND EAX,80000001h` / `JNS` / `DEC-OR-INC`. A non-zero remainder
loads -1.0 (00D7A260) and zero loads +1.0 (00D7A24C) into the side.

- The arm counter starts at the task install, so its parity differs from the step counter's whenever
  the install fell on an odd step. That is about half of all goaways. The image also gives every plane
  that enters goaway on the same step the same side. The host does not.
- The premise is doubtful. `host.summary.motion_steps` (`GHU :18237`) is incremented once per fixed step.
  Whether its origin matches [00F876B0] was not checked.
- **MISMATCH REAL.** The plane breaks away to the wrong side about half the time.
- **LIVE:** every level-bomb and dive-bomb goaway. **RNG:** none.

### R3 Dive flyabove heading has no avoidance increment

The label at `:27345-27349` says: "STILL UNBOUND ... the avoidance increment 009C6D59 adds to A, which
comes out of the unbound 007F0280". 007F0280 is now bound: `kNearFieldProbeBound` is ON, and the flyabove
probe runs at `:27297` to feed the speed slot (009C6B75).

In the image, 009C6B7B-009C6CB6 build `[ESP+20h]` from that probe's out_a / out_b, with two 00419010
interpolations, under the same `|out_a.x| > 0.05` gate (009C6B75). Then 009C6D42-009C6D54 compute
`00438AA0([ESP+4Ch], [ESP+20h])`, and that result goes on to the slew at 009C6D6F.
`include/bsp/dive_bomb_task.hpp:997` records the same omission.

- **MISMATCH REAL.** A dive bomber in flyabove never sidesteps a neighbour inside the probe box.
- **LIVE:** dive-bomb flyabove with wingmen close by. **RNG:** none.

## Other findings

| # | Host | Verdict | Note |
|---|---|---|---|
| C1 | GHU `:6019` `kDiveHitClockBound = false` | UNRESOLVED (OFF, body exists; goaway) | 009C7900 (a dive-task vtable slot, no direct callers) zeroes task+4BCh = approach+C4h, which arms the forced re-roll at 009C4AA4 for one second. The host keeps C4h at the 3600 floor, so the forced re-roll never fires. It is live only if that slot's caller (a hit notice, per the comment) runs. Sent to team-lead. |
| C2 | GHU `:5809` `kDiveBombMoveToOriginBound = false` (body at `:26291`, `:26395`) | MINOR (OFF, body exists) | 009C18EC-009C1913 measure from the +2Ch entity's pose origin (+FCh/+104h after 00414DB0) and steer at it. The host uses the 009FADA0 hull aim point (`:4456-4472`), which is at most about half a hull length away. |
| C3 | `include/bsp/torpedo_task_arm.hpp:239` `kTorpedoArmFallthroughReleaseBound = false` | already reported (CC13_VER_6 finding 3a) | 0099B6A0 re-read: it tests `[ESI+2FCh]`, calls unit+72Ch vt[38h], calls 007BBBA0 when that is true, and returns AL = 1. The host's OFF arm keeps the order unspent and makes no release (torpedo). |
| L1 | GHU `:24683-24684` "prepare's tick 009B6670 is unread, so +98h keeps -1.0" | STALE LABEL | `kLevelBombPrepareTickBound` is ON (`:6281`), and `lb_prepare_tick_009b6670` (`:24397`) writes `lb_prepare_98`. |
| L2 | GHU `:24579` `record("BotStateMoveTo::direction_009fabe0")` in the level goaway tick | MINOR, same class as VERIFY1 D8 | At 009B5993 the image calls 009FABE0. That function writes gun-block +68h..+70h from the heading and 0099B630's pitch. The host does not model gun+68h. The dive goaway's `:26825-26832` note is the same. |
| L3 | GHU `:24429-24432` leader bay byte | MINOR / UNRESOLVED | The image byte leader+9C1h[idx*8] is written by 007CDC70 (007CDCA3) as `device(+DECh)+60h != 0 && +64h != 1.0`. The host uses the leader approach's `close_cc`, which is a different predicate. It only sets the wingman's bay flag +CCh in prepare. |
| L4 | GHU `:24020-24024` level goaway has no target extent | **CORRECTED 2026-10-19 (cc13-goaway): MISMATCH, now switched** | Was "MATCH in effect: 007B5BE0 wins only above SafeDist 1000, more than any ship's half-extent". The fields are not extents. `unit+434h/+444h/+448h` are `00956D63`'s longest ammunition ranges of categories 1/5/6 (AAMACHINEGUN, FLAK, LIGHTARTILLERYFLAK; `unit+430h + cat*4`). For the level bombers' actual targets (IJN08 Maru1/Maru2, USNOS TroopTrans2-6 and JM08 Missouri and transports; docs/CC13_TORP_GOAWAY.md C.5) the term is 1600 in the host and 2000 in the image. The 1600 is the category 1 AA machine-gun ammunition; the 2000 is category 6 flak's second ammunition. So the term exceeds SafeDist 1000 (planeglobals.lua:535) up to the 3500 cap (00D046A0). The level enter now has `kLevelGoawayTargetExtentCc13Bound` (OFF) on `lb_target_plus_one`; docs/CC13_TORP_GOAWAY.md part C. |
| L5 | GHU `:24329-24332`, `:24689` `force_release_3ae = false` | MATCH | No script in this installation calls `SquadronSetForceRelease` (008A28B0). |
| L6 | GHU `:24136` paratrooper branch (007B9500 31h, 009B7D7D-009B7E1F) | MINOR | Matters only for planes carrying paratroopers. |
| L7 | GHU `:24613` a new task per order; `:24661` target +5Dh read as liveness | same class as VERIFY1 S8 / S9 | |
| L8 | GHU `:25889-25902` "approach+50h and +5Ch/+60h have no producer read. Zero" | STALE LABEL | +5Ch/+60h are set from the robots row at `:25812-25813`. +50h is written every tick at `:4464` (`kFlyoverSpeedBound` ON, `:2464`). |
| L9 | GHU `:27038-27041` aimglide "PARTIAL: bank, altitude and direct yaw left unbound" | STALE LABEL | `kAimGlideYawBound`, `kAimGlidePitchBound` and `kAimGlideThrottleBound` are ON (`:2448-2458`). |
| L10 | GHU `:27060-27062` aimglide heading "the bearing to the aim point in place of the frame slot 009C5435 reads" | STALE LABEL, MATCH | `[ESP+6Ch]` is atan2 of (approach vt[0] aim point - unit position) (009C51C7-009C51F7, 009C5357-009C5386). That is the bearing to the aim point. |
| L11 | GHU `:25764-25767` 009C8A74's +C4h re-seed not carried | MATCH in effect | C4h starts at 3600 and only increases, so the re-seed changes nothing (unless C1 is bound). |
| L12 | GHU `:4279`, `:4297` MultiBombPlatform keeps the 2-round substitute | MINOR, labelled | `kDiveBombCarriedRoundsBound` is ON for single racks only. |
| L13 | GHU `:4443` 009C8A5E floor AttackDist x task+41Ch, with 41Ch taken as 1.0 | UNRESOLVED, minor | It matters only when 1.6..1.8 x the turn radius is below 1100. |
| L14 | GHU `:26462-26464` "Left at `true` here" | STALE comment | The call passes `!kPlaneFollowLawBound`, which is false. |
| L15 | GHU `:26975-26983` the dive done-enter's follow-state writes (+6Ch, +84h/+85h, +88h-+94h, the observer) not carried | same class as VERIFY1 S2 / D13 | |
| P1 | `src/plane_flight.cpp:642` the 007D98F0 predicate is analytic | already reported (CC13_GPR_48 item 2) | 007D9360 is a full trial integration (007D7C00, 007D8470, 007D9140, 007D92B0). It is reconstructed as `trial_integrate_at_climb_angle_007d9360` in `src/gameplay_seg48_cc13.cpp` but not wired. Its result feeds desc+1E4h/+1ECh for every plane. Also MINOR: the `:624` guard (max_spd or accel <= 0 returns 0) is not in the image (007D98F0 has none). |
| P2 | `include/bsp/plane_follow_law.hpp` "007D7DA0 ... its body is unread" | STALE LABEL | The host binds `leader_turn_rate_007d7da0` (GHU `:8404`, `:8983`). |
| P3 | `src/air_operations.cpp:963-964` order 2 and slot states 2-4 "not modelled" | label consistent | The player's held-slot orders run through `GameScriptOrdersHost::player_air_ops_order` (`src/game_hosts_script_orders.cpp:1900`). The `:554-558` class id in place of class+70h is a stated contract. |
| P4 | `src/torpedo_task_arm.cpp:136` "unread, taken as 0" | same as C3 | |
| P5 | `src/plane_squadron.cpp:53` malloc-backed native string | MATCH | It is only a representation of the string. |

## Unlabelled arithmetic spot checks

All of these were read against `disasm-raw` bytes.

| Function | Host | Verdict |
|---|---|---|
| 009B7C90 latch and projection time (009B7F58-009B802F) | `plane_level_bomb_task.cpp:73-90` | MATCH, NaN arms included. The 1.1 x +B8h compare (009B7F8E `FMUL qword`) is taken in double. That is at most a 1-ulp difference if the step runs at 24 bits. |
| 009B7F13-009B7F56 inline bearing | `heading_command_009f9e40` | MATCH (the same atan2 / pi/2 / 2 pi sequence as 009F9E7E-009F9EAB) |
| 009B639A-009B645B aim release, abort and step | `:266-277` | MATCH |
| 009B8C90 cruise profile | GHU `:24059-24112` | MATCH. +38Dh/+38Ch freeze and the +380h/+37Ch timers are modelled as expiry times. The +4B0h max agrees. |
| 009B5840-009B58D8 level goaway heading | `:495-509` | MATCH. A NaN turn pushes in the host and not in the image (009B589F JB). |
| 009B59F0 release timer (009B5ABD-009B5AE9) | GHU `:24518-24522` | MATCH |
| 009B88F0 release and goaway cases (009B8A6E-009B8ACB) | `:143-147` | MATCH |
| 009FD570 blend gate, side latch and constants | `plane_fly_to_solver.cpp` | MATCH. The -200 gate at 00D21CE8, 0.1 at 00D7A3A0 and -0.1 at 00CE3928 are both the float 0.1 widened to double. |
| 009D0F2F-009D101D torpedo goaway tick head | `torpedo_goaway_tick.cpp:86-121` | MATCH (the 100 m slack at 00D7A220, the 6.0 guard at 00CE6628, the draw order) |
| 009C4311-009C43CA dive attackrun | `dive_bomb_task.cpp:530-553` | MINOR. A NaN margin keeps NaN in the image (009C4379 JBE) but becomes 50 in the host. The host's `clamped_distance != 0` guard is not in the image, which divides at 009C4397; this matters only when the plane is directly over the target. |
| 007DE1E0 actuator channel | `torpedo_release_spawn.cpp:20-44` | MINOR. A NaN value stays NaN in the image (007DE25D/007DE267 JBE) but goes to 0 in the host. |
| 007F2576-007F25B8 formation shape 1 | `plane_formation.cpp:139-141` | MATCH at 24 bits. Shapes 2-4 round blended double expressions once, while at 24-bit PC the image rounds every operation (rule 4c, 1 ulp). |
| 009C9FB0 retreat state rule | `plane_retreat_task.cpp:144-159` | MATCH. The cadence is in GHU `:32987`. A NaN timer differs only on the moveto arm. |
| 009B7AB0 goaway clear | `:164-169` | MATCH |

## Not covered

- These files were checked only for labels (none matched the census) and for their host wiring, with no
  arithmetic spot check: `plane_ai_control.cpp`, `plane_angular_velocity.cpp`,
  `plane_attitude_angles.cpp`, `plane_bow_wave.cpp`, `plane_class_fields.cpp`, `plane_control_rate.cpp`,
  `plane_death_modes.cpp`, `plane_ground_ops.cpp`, `plane_pose_commit.cpp`, `plane_release_ver_cc13.cpp`,
  `plane_squadron_entity.cpp`, `plane_wanderer.cpp`, `torpedo_aim_tick.cpp`,
  `torpedo_approach_update.cpp`, `torpedo_first_release.cpp`, `torpedo_release_orders.cpp`,
  `dive_bomb_aimdive_tail.cpp` and `bot_tasks.cpp`. The torpedo host arm was covered by CC13_VERIFY1 A.5.
- `plane_water_law.cpp`: only the min/max/clamp helpers and their NaN arms were checked (MATCH).
- The dive-bomb transition rule 009C7A80 and its feed (GHU `:25917-26280`) were not re-read.
