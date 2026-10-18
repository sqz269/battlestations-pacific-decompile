# CC13_SEG67B: segment 67 plane AI states, second pass (packet cc13-seg67A)

Packet cc13-seg67A, 2026-10-18, on cc9's 2026-10-18 addendum terms (segment 67,
00996120-009F6060). The file names carry `seg67b` because `cc13_seg67a.*` and
`docs/CC13_SEG67A.md` already belong to the earlier cc13-seg67a lineage, and on
this case-insensitive file system `seg67A` would collide with them.

Status words follow AGENTS.md. The cc9 host files are read-only. A row that cc9
or another worker already implements is recorded ledger-only as
`existing_implementation_spot_checked`, and any mismatch goes to cc9 as an
adoption note.

## 1. Priority findings (cc9's three items)

### 1.1 The follow tick's release return, 009C1FFD (priority 1)

`009C1FD0 BSP_BotStateFollow_Tick`, `__thiscall(state, float dt)`, `RET 4`.
Read from disk bytes:

- 009C1FE2 sets cmd(task+18h)+26Ch to 2.
- 009C1FEA calls 009BFD70, the station; AL=0 jumps to 009C234E.
- 009C1FFD tests unit+C25h != 0, and 009C2006 tests unit+C20h > 0. Either one takes
  the release arm, and it does so **before** the follow law (009C2068 CALL 009BFEE0,
  009C2077 CALL 009BEE30).
- The arm writes cmd+2D0h=2, +2BCh=0.0f, +2C4h=0.0f, +2CCh=1,
  +2B4h=[task+8]+190h (TravelSpeed), byte +2B0h=0 and +2D8h=1, then returns. So
  while a release request is queued, the plane holds level flight at TravelSpeed
  instead of keeping station.

The host's `run_follow_tick_009c1fd0` (game_hosts_units.cpp:9330) runs the law
unconditionally and writes none of the seven stores. None of its callers gates on
C20h/C25h. The moveto-follow arm at :35102 returns at the right point but drops the
stores. Adoption note sent to cc9.

The issue stage 007CE9FD lies outside this segment. Its five guards (bytes
007CEA02-007CEA4B) are:

1. the session mode [E188A8]+1FE4h == 2;
2. unit+9E0h;
3. unit+C3Ah;
4. unit+5Dh;
5. unit+900h, which must be 4/5/6/7. 0042A7E0 shows that 4/5 means on the ground.

The host's single `guard_blocked` counter lumps all five together, so the 1224
blocked ticks on USNOS have not been attributed to a guard.

### 1.2 Land / park / land-abort (priority 2)

- **009B09C0, the land/abort on-ground arm 009B0E74-009B0F93.** The host's arm (:33246,
  behind `kLandAbortGroundArmBound`, OFF) matches the bytes, including float(pi/2)
  00CE3830, the COMISS/JA sign test, and the 00419010 yaw with ±1.1. The only
  host-only behaviour is the null-holder refusal.
- **0099B0BE-0099B113, the re-takeoff.** Its condition is:
  !(plane+72Ch)->vt[38h], task->vt[38h], (0042A7E0 IsOnGround || +900h == 6), and
  task->vt[30h]. When all hold, the tick pushes 009CFF40(DL=0) through 00999F50. The
  host's gate (:35681) agrees, taking task vt[38h] as "a head task exists".
- **009B3CF0, the land state rule.** 009B3D0B parks a plane whose +900h is 4/5 unless
  it is in park or abort, and the host agrees. One gap: the final->park edge
  (009B3DD9, 009B3370) is refused at :33497 even with `kLandParkStateBound` ON.
- **009B3370.** The host's predicate (`land_touched_20 && +900h != 7`) matches.

### 1.3 The torpedo goaway's done condition (priority 3)

The host's `torpedo_next_state_009d4030` and `torpedo_goaway_complete_009d3150`
match the bytes.

When squadron+369h (ReloadEnabled, default 1 at 007F2D09) and [00E17BF2] are both
set, 009D3150 is false whenever approach+132h is 0. 009D3420 rewrites +132h every
tick (009D34C5 007B93F0, kind 2Bh, at 009D486F, before the rule at 009D48DE). So
in the binary, a plane that has dropped its torpedo stays in goaway until a reload
restores the 2Bh bit or 009D3210 reports it disengaged. This is binary behaviour,
not a host defect.

## 2. Ledger-only records (existing implementations, spot-checked)

| Address | Name | Host body | Result |
|---|---|---|---|
| 009C1FD0 | BSP_BotStateFollow_Tick | `run_follow_tick_009c1fd0` | mismatch: release arm order and stores (1.1) |
| 009B09C0 | BSP_BotStateLandAbort_Tick | `run_land_abort_tick_009b09c0` | on-ground arm matches |
| 009B3CF0 | BSP_BotTaskLand_StateRule | `land_state_rule_009b3cf0` | matches except final->park |
| 009B3370 | BSP_BotStateLandFinal_ParkRule | inline predicate | matches |

## 3. Reconstructed bodies (`src/cc13_seg67b.cpp`)

These are new C++ interfaces over raw native records. Every native call goes through
`Cc13Seg67bHost` at the image's point, with the image's arguments. Nothing is wired
in. Status: build-tested only. Per-function evidence and uncertainty are in the
code comments and the ledger records.

### 3.1 The bot-task base and the factory list

| Address | Function | Notes |
|---|---|---|
| 0099EBB0 | `bot_task_factory_register_0099ebb0` | pushes the factory onto 00F8A314 |
| 0099EBE0 | `bot_task_factory_unregister_0099ebe0` | unlinks it, then returns the name block through 00419CC0 / 00BD1510 |
| 0099B630 | `pilot_command_pitch_target_or_live_0099b630` | plan+2BCh when +2D0h is set, else unit+C64h |
| 0099B650 | `bot_task_set_owner_25c_0099b650` | +25Ch store |
| 0099B690 | `bot_task_retire_0099b690` | 0099A870(bot, task) |
| 009A1A20 | `bot_task_commanded_heading_009a1a20` | 00438AA0(heading, 0099D0A0 or 0) |
| 0099C6F0 | `bot_task_construct_base_0099c6f0` | two draws: stream 1 U(0,1), stream 0 U(-1,0) |
| 0099C8B0 | `bot_task_construct_base_from_stream_0099c8b0` | the same, plus a read of "avoidFlags" |
| 0099D060 | `bot_task_save_base_0099d060` | writes "avoidFlags" |

### 3.2 Bot conditions (00996120-009972ED)

| Address | Function |
|---|---|
| 00996120 | `bot_condition_timed_destruct_00996120` |
| 00996270 | `bot_condition_turn_to_test_00996270` |
| 00996460 | `bot_condition_timed_save_00996460` |
| 009965D0 | `bot_target_condition_save_target_ref_009965d0` |
| 00996670, 00996770 | `bot_target_condition_destruct_00996670` / `_00996770` |
| 00996720 | `bot_condition_turn_to_construct_00996720` |
| 009968D0 | `bot_condition_create_from_archive_009968d0` |
| 00996BB0 | `bot_condition_load_from_reader_00996bb0` |
| 00996C00 | `bot_condition_timed_load_00996c00` |
| 00996CA0 | `bot_target_condition_save_00996ca0` |
| 00996D00, 00996D70, 00996E10, 00996EB0 | the turnAway, turnToHdg, turnTo and farAway saves |
| 00996F50 | `bot_target_condition_load_target_00996f50` |
| 00996FB0, 00997090, 00997110, 009971B0, 00997250 | the target, turnAway, turnToHdg, turnTo and farAway loads |

Two loads are seeded from the stack: the read slots in 00996C00 and 00996F50 are
the entry `PUSH ECX` slots. Before the archive writes them they hold the pointer
bits, and the code keeps that. 009968D0 calls `o->vtable[8]` even when no object
was made; the image faults on that path, so the host receives a null object.

### 3.3 The follow state, and the teardown bodies

| Address | Function | Notes |
|---|---|---|
| 009C2980 | `bot_state_follow_construct_009c2980` | +74h = -U(0, 0.6), drawn from stream 1 (shared) at 009C2A0A; the float argument is unread |
| 009BDE40 | `bot_state_follow_exit_009bde40` | 007B8A90 on the unit, then (approach+1Ch)+4Ch = 0 |
| 009BDE80 | `bot_state_moveto_set_ranges_009bde80` | three MOVSS copies to +30h..+38h |
| 009A3930, 009AD390, 009B55D0, 009C40F0, 009D0B20 | follow-style state destructor bodies | 006CDD70(s+18h), then 007B45F0(s) |
| 009C2880, 009B6480, 009C70A0, 009A4680, 009ADAE0, 009A93E0, 009C9A80, 009D2390, 009CBE90, 009ACE50, 009CF850, 009BCB70 | approach-block destructors | the image's member-destructor order (see code) |
| 009A6B90 | `bot_approach_dogfight_destruct_body_009a6b90` | 00653390, 006CDD70, 00695870 |
| 009BAF60, 009C2920 | conditioned / moveto task destructor bodies | |
| 009D60F0, 009D5EB0, 009DB380, 009DB3D0, 009DB420 | ship-AI observer member destructors | |

The teardown bodies share one sequence interpreter (`teardown`). It keeps each
image's call order exactly, including the null-adjusted registry cast
(`ECX = b ? b+off : 0`) in front of 004116D0. EH state bookkeeping is not modelled.

### 3.4 The moveto-style state's archive pair and the task-base predicates

| Address | Function | Notes |
|---|---|---|
| 009BEAA0 | `bot_approach_save_altitude_leg_009beaa0` | "target" goes through the writer handle itself; minAlt, diffAlt and refDist go through handle+8h; then 004107B0 |
| 009C2B80 | `bot_state_construct_base_009c2b80` | the read twin, ending with 00694A60 and 00410630; the unwritten target slot is 0 here (an ESP value in the image) |
| 0099C210 | `bot_task_pilot_robots_row_0099c210` | the robots-row address |
| 0099C230 | `bot_task_break_off_allowed_for_unit_0099c230` | 0 when the [00E198C4] holder's unit is the task's unit or squadron |
| 0099C270 | `bot_task_tick_gate_passes_0099c270` | the side byte at unit+9C2h |

Ledger-only, existing ship-AI host bodies, spot-checked: 009E97B0 (`build_tubes_009e97b0`),
and 009E9910 / 009E9A50 (`tubes_ready`). Both match, under the host's labelled
substitutions.

### 3.5 Dogfight construction, conditioned-task archives, registrars and timed states

| Address | Function | Notes |
|---|---|---|
| 009A7650 | `bot_approach_dogfight_set_target_009a7650` | observer swap on +A0h/+B4h, then 009FB5E0, 009FA350, 007B7870 |
| 009A9810 | `bot_task_dogfight_construct_009a9810` | the initial state is +510h for a flight leader (007B8AD0), else +54Ch |
| 009AB490 / 009AB500 / 009AB570 | dogfight factories | 758h-byte task; kinds 0Fh (squadron) and 18h |
| 009B9EA0 / 009BA1E0 | conditioned save, leave-alone (JMP) | does not call the base save 0099D060 |
| 009B9F20 / 009BA1F0 / 009BA270 | pull-up, barrel-roll, flik-flak saves | barrel-roll writes "started" twice |
| 009BA830 | conditioned construct-from-stream | 0Fh kind, vtable 00D20388 |
| 009A3090 / 009A6A60 / 009AB740 / 009B42D0 | state-name registrars | registry at approach+FCh / +104h / +C4h / +DCh |
| 009A36A0 / 009A70E0 / 009A84E0 / 009B49B0 | timed state constructors | each draws one value from the shared RNG 00BD2F10, stream 1 |
| 009BC960 | move-on-path circle state | byte +18h = (0.5 <= U(0, 1)), using the shared RNG |
| 009B9680 | `bot_task_maneuver_roll_completed_009b9680` | the x87 order is read from the bytes DC E9 / DE E9 |
| 009BB380 | `pilot_bot_prepend_tight_turn_to_point_009bb380` | 00999F50 runs even when the allocation failed |

Ledger-only spot checks:
- 009A5420 (`depth_charge_is_attack_state`) matches.
- 009A56C0 (`set_state`) matches, and covers its byte-identical copies 009AE290, 009B84A0 and 009B3680. 009A9D50 is the same body with a redundant leading test.
- The cc9 host's 009CA3B0 (glide seed) matches, with its labelled ctl+398h substitution.
- The cc9 host's 009CAD00 (horizontal distance) matches, with one exception: the host keeps dx/dz in double, while the image rounds each to float before squaring, so the result can differ in the last bit.

### 3.6 Plan endpoints, the land cruise pick, angle normalisation and ship-AI teardown

| Address | Function | Notes |
|---|---|---|
| 009C25D0 | move-to circle state | the 009BC960 body with vtable 00D20A9C, using the shared RNG |
| 009AFA50 | `bot_task_land_pick_cruise_state_009afa50` | +620h when (unit+72Ch)->vtable[38h] is false; otherwise +4C4h for a leader and +500h for the rest |
| 0099B260 / 0099A9B0 / 0099A9E0 | plan slot, endpoint observer and copy | the plan registers itself as the observer |
| 0099BA10 | `math_normalize_angle_to_pi_by_ref_0099ba10` | fmod by 2pi, then one fold; NaN passes through |
| 0099BB00 | yaw-rate multiplier latch | max with the NaN arm taking the argument |
| 00999AE0 | first-task wait value | -1.0 when every task passes vtable[34h] |
| 009DA880 / 009DB470 / 009DF8F0 | ship-AI node destroy, brain members, list head | the list head writes address 4 on a null allocation, as in the image |

Ledger-only: 009C82D0 and 009D3E40 are byte-identical copies of 009A56C0 (`set_state`).
