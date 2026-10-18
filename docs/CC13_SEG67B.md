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

The host's `run_follow_tick_009c1fd0` (game_hosts_units.cpp) ran the law
unconditionally and wrote none of the seven stores. None of its callers gates on
C20h/C25h. The moveto-follow arm (`run_moveto_follow_tick_009c1fd0`) returns at the
right point but drops the stores. Adoption note sent to cc9. cc9 has since added the
arm behind `kFollowReleaseArmBound` (packet p20_follow_release_arm, OFF as of
2026-10-18); with the switch OFF the host behaves as described above.

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

- **009B09C0, the land/abort on-ground arm 009B0E74-009B0F93.** The host's arm (in
  `run_land_abort_tick_009b09c0`, behind `kLandAbortGroundArmBound`, OFF) matches the bytes, including float(pi/2)
  00CE3830, the COMISS/JA sign test, and the 00419010 yaw with ±1.1. The only
  host-only behaviour is the null-holder refusal.
- **0099B0BE-0099B113, the re-takeoff.** Its condition is:
  !(plane+72Ch)->vt[38h], task->vt[38h], (0042A7E0 IsOnGround || +900h == 6), and
  task->vt[30h]. When all hold, the tick pushes 009CFF40(DL=0) through 00999F50. The
  host's gate (in `pilot_think_and_commit`, before `push_ground_retakeoff_0099b107`) agrees, taking task vt[38h] as "a head task exists".
- **009B3CF0, the land state rule.** 009B3D0B parks a plane whose +900h is 4/5 unless
  it is in park or abort, and the host agrees. One gap: the final->park edge
  (009B3DD9, 009B3370) is refused in `land_state_rule_009b3cf0` even with
  `kLandParkStateBound` ON. cc9 has since added the edge behind
  `kLandFinalParkEdgeBound` (packet p20_land_final_park, OFF as of 2026-10-18).
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

### 3.7 Ship-AI destructors, the move-to refresh and the path-leg manager singleton

| Address | Function | Notes |
|---|---|---|
| 009E5110 / 009E5130 / 009E5460 / 009E8C10 | ship-AI destructor bodies | 009E48A0 is the std::list destructor (library), bound through the host |
| 009BEBA0 | `bot_approach_refresh_target_009beba0` | the whole body |
| 009BD400 | `bot_path_leg_manager_get_body_009bd400` | a lazy singleton taken under the lifetime manager's critical section |

009BEBA0 and the cc9 host's `moveto_refresh_009beba0` (game_hosts_units.cpp):
- The host is a partial model that carries its own labelled substitutions.
- The +2Ch zeroing has the same polarity in both: `sq_lua_alt_3ad` is true exactly when byte +3ADh is 0.
- The host does not apply the +28h leg-altitude override (009FC260). It keeps that as a record, as its comment states.

### 3.8 The depth-charge task's archive, factory and load bodies

| Address | Function | Notes |
|---|---|---|
| 009A35D0 | approach construct-from-stream | reads the "_targetLock" section into the target ref at +7Ch |
| 009A5E60 / 009A5ED0 | approach / task saves | writes "_targetLock" and "_state_moveTo"; the state index goes through 00411450 |
| 009A67C0 | task destructor body | |
| 009A6820 / 009A69E0 | task construct-from-stream and load factory | the state index goes through 00411170 |
| 009A6970 | `bot_task_make_depth_charge_009a6970` | allocates a 7BCh-byte task |
| 009A19B0 | heading difference from the live plane | |
| 0099B670 | vehicle avoidance dispatch | calls 007DF4F0 when unit+C50h is set |

Ledger-only: 009A3FA0 (`depth_charge_in_release_window`) matches, including its NaN arm.

### 3.9 Dogfight, drop-kamikaze and kamikaze destructor bodies, stream constructors and factories

| Address | Function | Notes |
|---|---|---|
| 009AB370 / 009AEB80 / 009AF610 | task destructor bodies | each runs the approach block's destructor at +3F8h, then 007B4030 |
| 009AB3D0 / 009AEAE0 | stream constructors (kinds 2 and 5) | 009F9980 runs before 00411170 here; in 009A6820 the order is reversed |
| 009AB630 / 009AEC50 | load factories | 758h- and 74Ch-byte tasks |
| 009AEBE0 / 009AF720 | make factories | the constructors are 009ADBF0 and 009AEEF0, called through the host |
| 009A98F0 | dogfight aim "bored" test | +18h > +1Ch |
| 009B1E30 | land target kind-9 test | |

Ledger-only: 009AF230 is a byte-identical copy of 009A56C0 (`set_state`).

### 3.10 Land, level-bomb, move-on-path, stop, immelmann and gain-altitude: teardown, stream and factories

| Address | Function | Notes |
|---|---|---|
| 009B3F50 | land task destructor body | for a flight leader, retires the landing squadron (006C8800) before 009B2C80 |
| 009B4120 / 009B4230 | land stream constructor and load factory | 009B3040 receives [bot+50h] rather than the bot |
| 009B8E80 / 009B8EE0 / 009B90A0 / 009B9030 | level-bomb destructor body, stream constructor, load and make factories | |
| 009BD9D0 / 009BDA30 / 009BDBB0 / 009BDC10 | move-on-path destroy, stream constructor and factories | the stream constructor draws -U(0, 1) into +51Ch from the shared RNG |
| 009BADB0 | stop-task factory | a bare 3F8h-byte task of kind 0Fh |
| 009BA960 / 009BAAB0 | immelmann and gain-altitude stream constructors | both read into the archive argument's own stack slot |
| 009BD300 / 009BC7E0 | move-on-path approach save and stream constructor | keys "altOffset" and "travelAltOffset" |

### 3.11 Level-bomb predicates and move-to state constructors

| Address | Function | Notes |
|---|---|---|
| 009B7A80 / 009B7B10 / 009B7B40 | prepare-waiting, aim-release and aim-abort predicates | the release and abort tests read the squadron byte ctl+3AEh |
| 009B6C10 / 009B7480 | level-bomb move-to state constructors (direct and from stream) | each makes one draw from the shared RNG; the stream form copies the reader handle and increments its count |

Ledger-only: 009BC780 is spot-checked against the move-on-path model, which inlines it.

### 3.12 Dive-bomb and move-to task and state bodies, the leader-velocity copy and the move-to target

| Address | Function | Notes |
|---|---|---|
| 009C8B60 / 009C8C70 | dive-bomb task destructor body and make factory | 7E0h-byte task |
| 009C2CA0 / 009C2D00 | dogfight move-to state constructors (direct and from stream) | vtables 00D20B24 and 00D20B0C |
| 009C3990 / 009C3CA0 | move-to task stream constructor and load factory | draws -U(0, 1) into +54Ch from the shared RNG |
| 009C4140 | dive-bomb attack-run state | a timed state |
| 009C7850 | the aim-glide out-of-bombs test | !007B9320(controller, 0); this is the input behind the host's `aimglide_out_of_bombs` |
| 009BE050 | leader-velocity copy | then 007D9C10 on unit+AB0h |
| 009BE0F0 | move-to target from an entity | |

Ledger-only: 009C9F70 is a byte-identical copy of `set_state`.

### 3.13 Level-bomb and dive-bomb task constructors

| Address | Function | Notes |
|---|---|---|
| 009B7990 / 009C7710 | level-bomb (kind 4) and dive-bomb (kind 8) task constructors | the base, the approach block at +3F8h, three vtables, then a flight leader starts in +4E8h / +4F0h and a member in +534h / +52Ch |

### 3.14 Land minimum speed and park edge, the takeoff factory, and the ship-AI signed-node walks

| Address | Function | Notes |
|---|---|---|
| 009B1300 | land approach minimum speed | max(007C4810, 007C47F0 + the target's velocity along the plane's unit direction); the cc9 host drops the projection because its airfield target is static |
| 009B3770 | land state rule, the park edge | from +5B8h / +598h a set done byte re-picks the cruise state (009AFA50, an inline set_state); any other state but +4C4h / +500h goes to +64Ch |
| 009CFF40 | takeoff task factory | 4D4h-byte task, 009CF8E0 |
| 009D5AE0 / 009D5B90 | plan walks for a node with a positive / negative side byte | reuse the 009D5930 direction rule; on NaN costs the image takes +1 and the shared rule takes -1 |

Ledger-only (existing implementations, spot-checked against the bytes): 009B4400, 009C3DA0 and 009D02A0 (aim-error redraws); 009C23B0 (move-to target speed); 009CB650 and 009CB780 (strafe go-away re-plans); 009A86F0 (dogfight maneuver expiry); 009A9970 (avoid pick); 009B21D0 (park done test); 009B3C00 (land/begin final rule). The NaN-only differences in 009CB780 and 009B21D0 are recorded in their ledger rows.

### 3.15 Task destructor bodies, the gun-bot destructor body and the move-to approach block

| Address | Function | Notes |
|---|---|---|
| 009CA1A0 / 009CD1F0 / 009CFEE0 / 009D4DB0 | retreat, strafe, takeoff and torpedo task destructor bodies | the approach block's destructor on task+3F8h, then 007B4030 |
| 009F3E80 | ship-AI gun-bot destructor body | +2260h sub-vtable 00D05704, the owned object at +2264h, 009E8C10 on +58h, then 007293D0 |
| 009C2EF0 | move-to approach block from a stream | registers the moveto, follow and circle states in the registry at +70h |

### 3.16 Ship-AI path node walks, the navigation goal with a penalty and two debug colours

| Address | Function | Notes |
|---|---|---|
| 009D5A80 | path node subtree test | recursive; true for a node with no children, or when any child flagged at +31h / +32h has a leg longer than 1.0 or is itself complete |
| 009D9440 | run heading | follows the decided direction past legs of 1.0 or less and returns the heading of the first longer leg (00414EB0); 0.0 with no direction |
| 009DE280 | navigation goal with a penalty | 009DE050(goal, 1, 0), then mode +1C4h = 3, the wrapped heading at +1D8h and the penalty at +250h and +2B8h |
| 009D5E10 / 009D5FD0 | debug colour records | function-local statics {0.5, 0.5, 0.5, 1.0} and {0.0, 1.0, 1.0, 1.0} |

### 3.17 Ship-AI approach accessors and the move-to approach stream pair

| Address | Function | Notes |
|---|---|---|
| 009E5E00 / 009E5E30 | the approach's engagement target (kind 1Ch) and its kind-6 test | both read [brain+B20h] and ask vtable[5Ch] |
| 009E6120 | the approach goal point | brain+B2Ch..+B34h |
| 009C1DC0 / 009C33D0 | move-to approach construct-from-stream and save | keys altOffset, closingComplete, _updateTrgPosTimer, closingTime, targetpos and targetentity; the stream constructor draws -U(0, 1) into +58h from the shared RNG and reads targetentity into its own argument's stack slot |

009C2EF0 now calls the 009C1DC0 body directly. 009E7F60 is a std::list node allocation (library, bind-only).

### 3.18 Move-on-path and level-bomb approach blocks from a stream

| Address | Function | Notes |
|---|---|---|
| 009BCD50 | move-on-path approach block from a stream | registers path, follow and circle (path) at +40h |
| 009B77C0 | level-bomb approach block from a stream | builds the seven level-bomb states and registers them through 009B42D0; go-away +2F8h is twice the class turn radius |

The move-on-path and level-bomb task stream constructors now call these bodies directly rather than through host hooks.

### 3.19 Land and depth-charge approach blocks from a stream

| Address | Function | Notes |
|---|---|---|
| 009B3040 | land approach block from a stream | six embedded land states, registered by the existing 009AF9A0 body through the host |
| 009A5000 | depth-charge approach block from a stream | DepthCharge/done (+1E4h) and /prepare (+2ECh) share vtable 00D1F6FC; leave (+38Ch) has +3A4h = 5.0 |

The land and depth-charge task stream constructors now call these bodies directly.

### 3.20 Dogfight approach block from a stream and the level-bomb approach block

| Address | Function | Notes |
|---|---|---|
| 009A9670 | dogfight approach block from a stream | eight states registered through 009A6A60 |
| 009B75E0 | level-bomb approach block | the move-to state gets near = +B0h - 100.0, far = +B0h and a third float +B4h; the rest is the tail it shares with 009B77C0 |

The trailing argument of 009C2AC0, 009B6C10 and 009C2CA0 is a float (009C2AC0 reads it with MOVSS at 009C2B27); the earlier `mode` typing is corrected in the header, the source and the two ledger records.

### 3.21 Dogfight and dive-bomb approach blocks

| Address | Function | Notes |
|---|---|---|
| 009A94E0 | dogfight approach block | the move-to state takes 500.0, 100.0 and 1000.0; the rest is the tail it shares with 009A9670 |
| 009C73A0 | dive-bomb approach block | ten states; DiveBomb/done (+26Ch) and /prepare (+1CCh) share vtable 00D20D28; goaway +32Ch is twice approach+B4h |

The dogfight and dive-bomb task constructors now call these bodies directly.

### 3.22 Drop-kamikaze approach blocks

| Address | Function | Notes |
|---|---|---|
| 009AD720 / 009AD920 | drop-kamikaze approach block, direct and from a stream | seven states registered through 009AB740; the two follow-built states take vtable 00D1FC08 |

The drop-kamikaze task stream constructor now calls 009AD920 directly.

### 3.23 Direct move-on-path, depth-charge and land approach blocks

| Address | Function | Notes |
|---|---|---|
| 009BCC60 | move-on-path approach block | 009BC780, then the tail it shares with 009BCD50 |
| 009A4DC0 | depth-charge approach block | the move-to state takes d = +3Ch + +34h as both distances and +40h as the third float |
| 009B2E50 | land approach block | the move-to state targets [block+30h] with 100.0, 100.0 and 800.0 |

Each shares its tail with the stream constructor of the same block; the tails are now helpers (`move_on_path_block_tail`, `depth_charge_block_tail`, `land_block_tail`, and likewise `level_bomb_block_tail`, `dogfight_block_tail`, `drop_kamikaze_block_tail`).

### 3.24 Strafe, torpedo and takeoff task constructors

| Address | Function | Notes |
|---|---|---|
| 009CC230 | strafe task (kind 0Ah) | a flight leader starts in +4E0h, a member in +51Ch |
| 009D3050 | torpedo task (kind 0Eh) | +544h / +580h |
| 009CF8E0 | takeoff task (kind 0Dh) | +4B8h when the plane's +904h byte is set and +900h is 5; else +448h when the flag byte is set; else +490h when unit+AA0h > 0.0, otherwise +470h |

The takeoff factory 009CFF40 now calls 009CF8E0 directly.

### 3.25 Strafe, torpedo and takeoff approach blocks

| Address | Function | Notes |
|---|---|---|
| 009CC020 | strafe approach block | base 009CA4A0; move-to +E8h (near = far = +40h, third = +38h); follow +124h/+1BCh; gotowards +254h (800.0 twice); aim +278h (0.5); goaway 009CB500 at +298h and attackrun 009CAC80 at +2E0h (both draw from the RNG 00BD2F10 in their own bodies); seven names |
| 009D2DA0 | torpedo approach block | base 009D0380; move-to +14Ch with near = far = +78h + +74h and third = +7Ch when 15.0 <= +134h (or NaN), else +80h; follow +188h; done +220h and prepare +348h (00D21320, -1.0); attackrun 009D06D0 at +2BCh; goaway +2E0h (120.0); aim +318h; seven names |
| 009CF710 | takeoff approach block | base 009CD420; four embedded states prepare +50h, Takeoff +78h, SlowTakeoff +98h, parking +C0h |

The task constructors 009CC230, 009D3050 and 009CF8E0 now call these bodies directly.

### 3.26 Strafe and torpedo state constructors, takeoff approach base

| Address | Function | Notes |
|---|---|---|
| 009CB500 | strafe goaway state | vtable 00D2103C; four draws from the RNG 00BD2F10 on stream 1: +30h and +3Ch from U(0, 5.0), +44h = -U(0, 1), +18h from U(0.3, 0.9) |
| 009CAC80 / 009D06D0 | strafe and torpedo attackrun states | the 009A36A0 shape (+18h = 1.0, +1Ch = -U(0, 1), +20h = 0.0) with vtables 00D21004 / 00D212C4 |
| 009CD420 | takeoff approach base | 009F9CE0 with 9999.0; +34h = the plane's +100h when the plane has a holder (+BF4h); +30h = 15.0, or 25.0 when the plane passes the 10h kind test; +2Ch = +30h + +34h |

### 3.27 Strafe approach base

009CA4A0 builds the strafe approach on 009F9CE0 (tuning +65Ch) with vtable 00D20FC8. It makes four draws from the RNG 00BD2F10 on stream 1:
- +34h = min(U(0.9, 1.05) x u+DCh, tuning+658h x 0.8) x +24h; the first value is kept when the compare is unordered.
- +38h = U(0.9, 1.1) x u+230h x +24h.
- +3Ch = U(0.9, 1.1) x u+D8h x +24h.
- +D0h = -U(0, 1).
- The glide seed 009CA3B0, called near the end, draws on its own.

It also sets +44h = 3600.0, takes +4Ch from 007C2610, and keeps the target at +54h only when the target passes the 41h kind test. Kinds 10h and 16h scale +34h and +3Ch by 1.4.

### 3.28 Torpedo approach base

009D0380 is the reset the torpedo approach block runs at construction. The whole body now sits in cc13_seg67b.cpp; its steps match the fragment helpers in src/torpedo_approach_update.cpp (packets cc8/cc9) with two differences:
- **Precision.** The body models 24-bit precision; the helpers model 53-bit.
- **NaN on the attack-speed cap.** At 009D03FD the JBE keeps 0.75 x 007BCE20 when the compare is unordered. torpedo_reset_attack_speed_009d03d9 takes desc+18Ch in that case.

It draws six times from the RNG 00BD2F10 on stream 1, and 009D02A0 draws again after them.

### 3.29 Dive-bomb approach base

009C3EA0 builds the dive-bomb approach on 009F9CE0 (tuning +4D8h) with vtable 00D20C48. It makes three draws from the RNG 00BD2F10 on stream 1:
- +A8h from the record's +38h..+3Ch range;
- +B4h = U(0.6, 0.8) x TurnCircleRadius;
- +B8h and +BCh share U(1.6, 1.8) x TurnCircleRadius.

+D4h is the larger of +A8h + 250.0 and the mean of +A8h and +ACh. The mean is kept on a tie or an unordered compare, as dive_bomb_dive_entry_height_009c4045 also does; that helper is bsp_game-only, so this body has its own copy of the step. The body then copies the plane's position into +D8h..+E0h and calls the aim-error redraw 009C3DA0.

The uncertainty notes for 009CA4A0 and 009D0380 were corrected: float-exact double constants give exact products in double, so those steps round once.

### 3.30 Level-bomb approach base

009B44F0 builds the level-bomb approach on 009F9CE0 (tuning +458h) with vtable 00D200A8. It makes two draws from the RNG 00BD2F10 on stream 1:
- +A8h = U(0.3, 0.65) x 007C1FB0 (the longest rack stick time);
- +B4h = U(1.6, 1.8) x TurnCircleRadius, with +B8h = +BCh = 1.4 times that.

It then copies the plane's position into +D0h..+D8h and calls the aim-error redraw 009B4400.

### 3.31 Dogfight approach base

009A6C10 builds the dogfight approach on 009F9CE0 (tuning +650h, with the plane at [bot+50h]) with vtable 00D1F844. The target reference at +2Ch takes [arg+3D0h]. The observer member at +B8h is initialized on arg (007F0F80) and then given vtable 00CF6514. The body seeds +D4h/+D8h = 9999.0 and +E8h = 99.0, and resets the aim offset with 009FA350. It makes no draws from the RNG.

### 3.32 Drop-kamikaze approach base (existing implementation)

009ABAB0 stays a host hook. It already has an implementation, drop_kamikaze_approach_seed_009abab0 in src/kamikaze_task_cc13.cpp, recorded by cc13-backfill as an existing implementation with a minor deviation.

### 3.33 Drop-kamikaze approach base from a stream

009ABBE0 is the stream twin of 009ABAB0. It opens the target reference from {0,"targetLock"}, sets +A4h = TravelSpeed and +A8h = 1200.0, then reads six fields: startDist +ACh, attackDist +B0h, targetDist +B4h, insideAttackRange (byte) +C0h, targetHdg +B8h and currentAvoidFlag (dword) +BCh. 009AD920 now calls it directly.

### 3.34 Land approach base; the depth-charge base is an existing implementation

009AFE70 builds the land approach on 009F9CE0 (tuning +52Ch) with vtable 00D1FDB8. It works as follows:
- It keeps the air-ops object at +2Ch and its landing unit [air ops+7Ch] at +30h.
- It queues the plane with 006C0B50 and stores the result at +34h.
- It makes three draws from the RNG 00BD2F10 on stream 1:
  - +54h = U(0.9, 1.1) x tuning+4E4h;
  - +58h = U(0.9, 1.1) x tuning+4E0h;
  - +B0h = -U(0, 0.5).
- It copies the landing unit's position into +38h..+40h, then overwrites +3Ch with the plane's own height.
- It finishes with the geometry pass 009AFAF0(approach, 0).

009B2E50 now calls it directly.

009A3390, the depth-charge approach base, was not rewritten. depth_charge_task_construct_009a5240 in src/depth_charge_task_cc13.cpp already models it inline. Its step order matches the bytes, but it computes +40h in double and rounds once. It was recorded ledger-only as an existing implementation, spot-checked.

### 3.35 Land approach base from a stream

009AFFF0 is the stream twin of 009AFE70. Its +ACh seed is 1.0, where the direct path seeds 0.5. It draws only +B0h = -U(0, 1) from the RNG.

The landing unit comes from the archive (airBaseEntity), and the air-ops block comes from 006BCD20 on that unit. The timer pair at +ACh is read from updateLandQueueTimer, then approachAngle, approachPitch and standbyheightadd are read into +54h, +58h and +60h. The tail matches 009AFE70: the two position refreshes, then 009AFAF0. 009B3040 now calls it directly.

### 3.36 Part-3 review fixes

These rows from earlier sections are now recorded ledger-only, because cc9 already hosts them. Their cc13_seg67b bodies and the hooks only they used were removed. Each is now an existing implementation, spot-checked.

| Address | Host | Note |
|---|---|---|
| 009B1300 | land_min_speed_009b1300 (game_hosts_units.cpp) | **Mismatch.** The host drops the w·v carrier term (009B1309-009B1362) and adds 0. On a carrier landing, approach+30h is the mother ship, so that term is the carrier's velocity along the plane direction. |
| 009B3770 | inline in land_state_rule_009b3cf0 (game_hosts_units.cpp) | Arms match. |
| 009E5E00 | engagement_target_009e5e00 (game_hosts_ship_ai.cpp) | The host skips the kind-1Ch test. This is benign: the only caller sits under standoff mode 4, which is latched only for kind-1Ch targets. |
| 009E6120 | probe_point_009e6120 (game_hosts_ship_ai.cpp) | Matches. |

Other corrections:
- **00999AE0:** the walk is now the image's stale cursor. The cursor is taken once from +58h and compared by != against an end that is re-read after each entry (00999B07-00999B15).
- **009C1DC0:** at 009C1E64 and 009C1F5C, `MOV [ESP+2Ch],ESP` overwrites the arg slot before the targetentity read. In the image, a missing key therefore leaves a stack address. The body starts e as arg, and this is now labelled as an uncertainty.
- **009D9440:** labelled as inheriting 009D5930's NaN arm. The image's JC takes +1 on unordered costs.
- **009CAD00:** marked as a last-bit mismatch. The image rounds dx and dz to float (009CAD30, 009CAD3E); the host keeps them in double.
- **009B9030 and 009C8C70:** these now call the local constructors 009B7990 and 009C7710 directly.
- **009CA4A0, 009CB500, 009CD420 and 009B44F0:** these stay as new reconstructions. Each body now cites the inline cc9 host site that models it with per-name RNG stand-ins.

### 3.37 Review minors (packet cc13-seg67C)

- **009D0380, 009C3EA0 and 009AFE70** stay as new reconstructions and now cite cc9's inline models by host function and switch: `run_torpedo_task_arm_009d4850` under `kTorpedoResetDrawsBound` and `kTorpedoResetEngageDrawsBound`; `run_dive_bomb_task_arm_009c8790` under `kReleaseAltitudeDrawBound` (the +A8h draw); `GameUnitsHost::Impl::install_land_task_0099a3dd` (the land approach draws).
- Host cites by line number in this file and in `src/cc13_seg67b.cpp` were replaced by host function, block or switch names.
- **009C1DC0:** the ledger uncertainty now says that a missing targetentity key leaves a stack address in the image, not arg.
- **00999AE0:** the ledger evidence now says that the cursor is taken once and the end is re-read after each entry.

### 3.38 Stream constructors and the move-on-path path entity (packet cc13-seg67C)

009A6D40 builds the dogfight approach from a stream. It follows 009A6C10 but builds both observer members inline, and it reads the target lock from {0,"_targetLock"}. Further details:
- The +B8h observer is swapped to the entity read from "squadName".
- The +A0h observer is cleared, then swapped to the target ref's entity [+44h] when that entity's vtable[5Ch](0Fh) holds.
- +D4h is the 3-D distance from the target to the plane, (dy² + dx²) + dz². +D8h is the horizontal distance, dz² + (dx² + 0), where the zero is the FLDZ the image squares. Each takes the 1e-10 cutoff with the JBE NaN arm.
- The image overwrites the `bot` argument slot with stack addresses before the squadName read. A missing key would therefore register the observer on a stack address. The body seeds the slot with its own field record's address, and this is labelled.

009A9670 now calls it directly.

009B4690 is the stream twin of 009B44F0. It makes no RNG draws and does not call 009B4400. It reads ten fields, from releaseTime to calcHitPos. 009B77C0 now calls it directly.

009BD0B0 answers the path's entity for the move-on-path still-valid test 009BD280. The pseudocode shows a "varargs" vtable[114h], but that comes from the hoisted `PUSH 0`, which is 0071BFF0's own stack argument. vtable[114h] takes no arguments.

### 3.39 Ship-AI debug draws (packet cc13-seg67C)

009D7260, 009D98C0, 009DD9C0 and 009DDCC0 draw through 0085FB30, 0085F990, 00860BB0, 00860BE0 and 004166E0 (all host hooks).

The function-local static colours (red, green, blue, cyan and yellow) live in the image's block at 00F8A68C, which the hook now covers up to 00F8A703. They are written at the image's guard tests. The cyan record is 009D5FD0's.

009D7260 needed every stack slot traced through the four colour blocks. Its corners are (c ± v) ± u, with each product and sum rounded to float.

### 3.40 Saves, the retire list and the kamikaze approach (packet cc13-seg67C)

The approach saves 009AA410 (dogfight), 009AE2D0 (drop kamikaze) and 009B37F0 (land) write the same field lists that the stream constructors 009A6D40, 009ABBE0 and 009AFFF0 read, through archive slot +0Ch. Their task saves 009AA5C0, 009AE4C0 and 009B3B90 run 0099D060, the approach save, the `_state_moveTo` block (009BEAA0) and 00411450.

Each family also has an approach-level twin that writes the approach save plus `_state_moveTo`: 009AA570, 009AE470 and 009B3B40. None of the three has a rel32 or absolute-dword reference in the image, so they are not reconstructed.

0099A660, reached through the JMP thunk 0099A870, moves a task from the bot's active array (+58h/+5Ch) to its retired array (+64h/+68h/+6Ch). When the retired array is full, its capacity grows to 2n + 2. 0099B690 now calls the body directly, and the 0099A870 hook is removed.

009AED20 builds the kamikaze approach: base 009AB920, the moveto, follow and prepare states, and the inline gotowards and aim states, then registers the five names. The kamikaze model in `src/kamikaze_task_cc13.cpp` covers this construction inline with per-name RNG stand-ins, so the whole body is allowed. The two follow constructors (009C2980) each draw U(0, 0.6) from the shared stream: follow at +12Ch first, prepare at +1C4h second. The model does not take these draws.

The byte fields in these saves (inattackrange, insideAttackRange and bayDoorState) store only the low byte of the record's payload. The image leaves the upper three bytes as stack garbage; the bodies write zero there.

### 3.41 Level-bomb save, moveto approach, retreat and strafe factories (packet cc13-seg67C)

- **009B84E0 and 009B87B0**: the level-bomb saves. They write the ten fields that 009B4690 reads. The approach-level twin 009B8760 has no reference in the image.
- **009C1C30**: the moveto approach constructor. It draws twice on stream 1: +2Ch = U(0, MaxAltOffset), then +58h = -U(0, 0.5). For the flight leader it seeds the arrival dwell +60h as min(planar distance, row+268h) / row+18Ch. cc9 models this inline in `run_moveto_task_install_009c3000` with per-name stand-ins, so the whole body is allowed. cc13_moveto1's `approach_moveto_construct_009c1c30` hook can route to it.
- **009CA2B0 and 009CD300**: the retreat and strafe factories. Each calls operator new (564h and 6FCh bytes) and then the constructor. 009CD300 calls the local 009CC230.
- **009CA780**: re-arms the strafe target reference from the strafe row, runs the 009FA260 pick, and hands the point to the opportunistic rocket (007B7870). cc9 models it inline without the 007B7870 tail.
- **009CC5B0**: the strafe task's state set, ledger-only. 009CC5B0-009CC5DE is the 009A56C0 instruction sequence with [ECX+310h], so it is the generic `set_state` in src/bot_task_states.cpp. The body that a129dc8a9 added was a second copy and has been removed.
- **009D4E30**: the torpedo factory. It calls operator new (7DCh bytes) and then the local 009D3050.
- **009DDBA0**: Ghidra matched this to the library's `~SubAllocator`, but it is game code. It runs `eh vector destructor iterator` over the ship AI's two path plans at +60h (68h bytes each, destructor 009DBDB0), and only the unwind funclets 00CB0630 and 00CB06C0 reach it.

### 3.42 Ship-brain state constructors and their draws (packet cc13-seg67C)

009E4F90 builds the sub_attack parent and its "approach" and "fire" children. 009E8450 builds the attackmove state, its approach sub-state 009E5CA0, and four further sub-states. Each constructor makes one stream-1 draw: 009E4F90 sets +CCh = -U(0, 1) at 009E5061, and 009E8450 sets +1500h = -U(0, 1) at 009E84F8.

Both are reached for every ship: 00810DD0 -> 009F3F20 -> 009F3BA0 -> 009F39C0, which calls 009E8450 at 009F3AB2 and 009E4F90 at 009F3AC3 with no kind gate. In the image, every brain therefore makes 69 stream-1 draws, in this order:
1. 009F1160's seven.
2. 009E5530's sixty: U(0, 2.0 [00CE3958]) into each ring slot's +48h at 009E55A1, in a 60-pass loop reached through 009E8450 -> 009E5CA0.
3. 009E8450's one.
4. 009E4F90's one.

cc9's `seed_brain_draws_009f1160` takes the seven draws. It takes the 009E4F90 draw for submarines only, with the comment "no other brain reaches sub_attack", and it never takes the 009E8450 draw. cc9's `ensure_approach_ring` builds the ring lazily and takes none of the sixty (`ship_ai_attackmove_ring_slot_009e5530`, ship_ai_attackmove_substates.cpp ~125, sets jitter_48 = 0). So the shared stream is 62 draws short for each surface ship and 61 for each submarine at spawn. An URGENT-class adoption note was sent (submarine, shared RNG), and the lead has since sent cc9 this corrected count.

Three STL instantiations are tagged library-only after their callees were read:
- 009E48A0, the std::list destructor;
- 009E7F60, the list's _Buynode;
- 009E8EF0, the vector's _Insert_n.

### 3.43 The ship-AI allocation chain (packet cc13-seg67C)

The chain 009F3F20 -> 009F3BA0 -> 009F39C0 -> 009F1160 is now four bodies:
- 009F3F20 allocates 2268h bytes (009F3F3E) and runs 009F3BA0 on them (009F3F59); a failed allocation returns 0.
- 009F3BA0 runs the gun-bot base 0072BBD0, builds the brain at ai+58h through 009F39C0 (009F3BE5), installs the ship-AI vtables, makes the brain's `cruise` state current (ai+2264h = ai+0BC8h, its vtable+4h at 009F3C27), and attaches through 008FBC80 to ship+310h (009F3C43).
- 009F39C0 runs 009F1160, builds the state registry at brain+0B5Ch, the seven inline state objects, attackmove (009E8450) and sub_attack (009E4F90), then registers nine names in this order: "cruise", "stop", "follow", "land", "movetopos", "moveonpath", "attackmove", "sub_attack", "kamikaze_attack".
- 009F1160 builds the navigation block (009E4330, hooked), the unit references, the submarine slot +0AB4h (`ship->vtable[+5Ch](8) ? ship : 0`) and its 4-byte holder at +0AC4h, and the defaults through +0AF8h (009F1160-009F126A). 009F126B..009F1401 (the goal defaults, the seven stream-1 draws, the critical section) is the existing `ship_ai_brain_produce_tail_009f126b` (src/ship_ai_brain_producers.cpp), reached through a hook rather than restated.

Whole bodies are allowed because cc9 models these constructors only inline (`seed_brain_draws_009f1160` with keyed stand-ins, the follow and land defaults, the initial cruise state). Its seven draws match the image's order and ranges; the missing ring and state-constructor draws are the finding of 3.42.

### 3.44 Pilot-bot table slots and helpers, 00997340-0099C683 (packet cc13-seg67C)

These rows have no direct caller; most are vtable slots. Each is a body in src/cc13_seg67b.cpp:
- **Task-list queries:** 00999A10 (does the head task block a switch), 00999B30 (any task's +28h slot) and 00999C70 (find a task by its +3Ch slot). 00999B30 has no explicit false: with no match it returns the count's low byte, which the body reproduces.
- **Park predicate:** 00999A60, 00999A80 (scalar delete), 0099A940 and 0099A980 (the "planeID" read and write).
- **Pointer arrays {data, count, capacity}:** 00999700 (insert at), 00999810 (grow, 2c+2), 00999CC0 (push back) and 00999D40. The ledger calls 00999D40 PushBackOdd, but it inserts at index 0. Both inserts loop without bound when count is 0 with spare capacity; the bodies store once and label it.
- **Loading:** 0099AA20 loads the "_bot" base, then the numbered tasks, each by its "commandName" through the factory list (0099C690). Each task is pushed onto bot+58h.
- **Seeds and constructors:** 0099A630 (observer node), 0099B2A0 (the bare five-slot seed, twin of 0099B450), 0099BDB0 (the 0C8h plan table element), 009995A0 (attach) and 0099B3D0 / 0099B3E0 (byte setters).
- **0099C2C0** is the base command-still-valid slot. It returns 2 with no unit or command, and 1 when the director still runs the command (or runs 00E08F78 with a kind 1/2 command) and the aim point is invalid or within 100.0 of the active target; otherwise 0. The push at 0099C34E is 006F7DD0's scratch argument; 0071EB60 is a plain RET.
- **0099C3C0** is the two-pass lead point. Pass 1 predicts with t = |T - P| / speed; pass 2 recomputes t from the predicted point and writes it at +10h. +0Ch receives the pass-2 *distance*, not the time (XMM0 is loaded before the FDIV). The byte +8h only decides whether the target's vtable[+38h] is called (and dropped) first.
- **00997340** and **0099B1B0** are unreferenced.

Ledger-only: 00999610 and 0099B710 (see the table).

### 3.45 One-line table slots (packet cc13-seg67C)

These are the open zero-caller rows of six instructions or fewer. Each listing was read whole, and a host that already models a row takes precedence over a new body.

There are 36 bodies:
- **Constant returns:** static bot-task factory records (00F8A370, 00F8A3E8, 00F8A438, 00F8A534), task names, retreat's kind 3, and pull-up's 10.0.
- **Bare RETs:** 009CE290 and 009D0C00.
- **Field slots:** 009A9C80 and 009A9D30.
- **Leave-alone flush:** 009B9480.
- **Adjustor thunks:** three that reach the scalar deleting destructors.
- **Follow-or-base wait values:** 009A5D50 tests two follow states (+544h and +6E4h); 009BD2E0 and 009CA0A0 test one each.
- **The plane+844h byte:** cleared on dive-bomb turn-down enter, set by land/final's exit 009B1E90. The cc9 land host notes that +844h is not modelled.
- **Command-record accessors.**
- **The order-slot address:** 009D4F70.
- **Navigation setters:** five unreferenced ones, 009DABB0-009DABF0.
- **Auto-target base constructor:** 009F5260.

There are 35 ledger-only rows (section 4 records only the group):
- Command-class and name constants that a host header already carries.
- Wait values that the dogfight, land and strafe hosts model.
- The land standby, line and park enter and exit slots.
- The torpedo, strafe and drop-kamikaze attack-run enters.
- The ship-AI rows 009DAB10, 009DAE20, 009E8540 and 009E6480.
- Byte twins of existing bodies, found by a masked byte scan of every small body:
  - 009A71C0 and 009C4200 are twins of 007B4F40 (rocket_attackrun_enter_007b4f40) and 009A3750.
  - 009C44E0, 009C58A0 and 009C6210 are twins of 009B1E90.
  - 009BAC60, 009C3170 and 009C9E70 are twins of the entity_orders.cpp rows 006F8C00, 006F8920 and 006F8C90.
  - 009C3150 is a twin of 009DAE20 (kMoveToCommandClass).

### 3.46 Medium rows, batch 1 (packet cc13-seg67C)

Rows of 7 to 40 instructions with no host mention, depth charge and dogfight first. Before writing, each row had three checks: a rel32-masked byte-twin scan against rows that already have reconstructions, a `git grep` of every address form, and a read of the hosts that model the same task inline.

There are 21 bodies:
- **The nine Describe slots (+18h):** 009A68C0, 009AF670, 009AF890, 009B8F80, 009BDB00, 009C3A60, 009C8BC0, 009CA200 and 009CD250.
  - All share one helper. Each calls 009A1A90 for the base name and 00411430 for the state name, using the task's own registry offset, then 007B7E70 to join them. The temporary string is released inline.
- **009A79F0, the enter of the dogfight avoid state at vtable 00D1F8F8:**
  - +18h = U(0.75, 1.5) x AvoidTime. This is one draw on stream 1, with 00CEE07C at [ESP] and 00CE380C at [ESP+4].
  - It clears +24h, +1Ch and +20h.
  - cc9's `dogfight_avoid_timer_009a7de0` computes the same timer for 009A7DE0 and 009A8020, but that function is bsp_game only. This state is not hosted.
- **Debug draws:**
  - 009B35E0, the land task's. Its first stack argument (+440h) is the float radius of 008608B0; the colour is 008608B0's own static rgba 00F875A8, passed to 00860710 in EDX (the names-ledger row is corrected).
  - 009CFA10, the takeoff task's.
- **Maneuver slots:** 009B93D0 (conditioned tick), 009B93F0 (tight-turn slot +68h) and 009B9420 (leave-alone cruise profile).
  - 009B93F0 computes PitchSpd x pi + 1. It is modelled at 24-bit precision; at 53-bit the product would be rounded once less.
- **Saves:** 009B9FD0 (tight turn: pull-up plus "left"), 009BD380 (move on path) and 009C3510 (move to).
- **Small accessors:** 009CA0C0 (retreat target position) and 009CA480 (the kind-41h cast).
- **009AAF30, the dogfight cruise profile (slot +54h):** 0099B660, then the altitude block of `bot_task_update_cruise_profile` (src/bot_tasks.cpp ~158) with the cruise offset 640h behind the 007B8AD0 gate, then 009AAC70(task+3F8h, 0.0) through a hook and the tail 0099B740. The cc9 dogfight host has not bound it (include/bsp/dogfight_task.hpp ~59).

There are 33 ledger-only rows (section 4):
- **Byte twins:**
  - 009A6BF0 is a twin of 004BEAD0.
  - 009AE0D0, 009B8180 and 009C7FF0 are twins of depth_charge_flush_009a5710.
  - The array helpers 009B4AF0, 009B4B60, 009B4C10 and 009B4C90 are twins of 007EDD70, 007EDDE0, 006F3410 and 00B0A440.
  - 009BD3C0 and 009C88E0 are twins of 009A2B90.
  - 009C32D0 is a twin of set_state 009A56C0.
  - 009CA730 is a twin of 007B4890.
- **Inlined set_state:** 009AF2B0 is set_state with next = +654h.
- **Hosted rows:**
  - The still-valid tests 009A5DB0, 009A9CC0, 009AE140 and 009AF560.
  - 009AE210, 009A35A0, 009A7F70 and 009AB1C0.
  - The follow-state selectors 009A3160, 009A6B10, 009AB6D0, 009AB7F0 and 009B4380. Each matches the initial-state selector of its host.
  - The target-point getters 009A6B70, 009B4670 and 009B4A30.
  - 009B7A50, which is inlined at the tail of 009B8100.
  - 009A4900, the drop arm of the done tick, without the 0.3 re-arm.
  - 009B94B0, inlined in the flikflak tick.
  - 009B8450, `bot_task_step_result` with the level-bomb attack states.

### 3.47 Medium rows, batch 2 (packet cc13-seg67C)

Each row had the same three checks as batch 1, run before writing. The twin scan now decodes instructions with capstone and masks only the rel32 of CALL, JMP and Jcc.

There are 21 bodies:
- **Split-S roll step (009B94E0):** +294h = (-v / 2r) x (-pi/2 - pitch)^2.
  - v is the bank, or 00438B10(pi, bank) when the argument byte is set.
  - The ratio r = d+1A8h / d+1BCh is taken from the task descriptor at +2F8h.
  - It is modelled at 24-bit precision. Caller: the split s tick 009B9CA0 (no Ghidra function; vtable 00D207C0 +64h) at 009B9D05, with wrap = ([task+3F8h] == 1); cc9 does not model the split s task.
- **Stop tick (009B95D0)** writes the plan slots.
- **Factories:** 009BAE10 (gain altitude), 009BAE80 (immelmann) and 009BAEF0 (stop). Stop uses 0099C8B0 with kind 0Fh and vtable 00D20500.
- **009BC6B0** registers "path", "follow" and "circle (path)".
- **Plain state constructors:** 009BC870, 009BC8A0 and 009C6180.
- **Circle debug draws:** 009BC9F0 and 009C2670 are the same 23 instructions and share one helper.
- **009BCF70:** the command path cursor; it is the first half of 009BD0B0.
- **009BD1D0:** the move-on-path debug draw. The image's two lazily initialised colours are passed as equal local constants.
- **009BDED0:** the move-to observer setter.
- **009C5870:** the aimdive enter. cc9 keeps no aimdive enter of its own.
- **009C6220:** clamps a to [-b, b] and keeps a NaN a.
- **Ship-AI path nodes:**
  - 009D58A0, the destructor body.
  - 009D59C0, the run end point. Its tail recursion is written as a loop.
  - 009D5CF0, over the existing 009D5AE0 and 009D5B90 bodies.
- **009D5D60:** the target-ref reset, with one stream-1 U(0, 2) draw. It is unreferenced; the same reset is inlined 60 times in 009E5530's first pass (the sixty draws of §3.42).
- **009D6530:** the ref holder.

There are 6 ledger-only rows (section 4):
- 009D3E80 is a byte twin of 009A5710.
- The four selectors are each hosted where their task picks its first state:
  - 009BC6F0 in `moveonpath_rule_state_009bd17f`;
  - 009BDDC0 in `moveto_task_construct_009c3000`;
  - 009CA380, the strafe selector, in `st_install_009cc230`;
  - 009D0100, the torpedo selector, in `torpedo_next_state_009d4030`.
- 009D5990 is 009D5930 followed by `ship_ai_path_node_next_009d9ee6`.

### 3.48 Medium rows, batch 3 (packet cc13-seg67C)

Same checks as batch 2: the capstone twin scan, a grep of every address form, `bsp lookup`, and the hosts that model the routine inline. A scan of the image for absolute dwords and rel32 CALL/JMP targets backs each "unreferenced" note.

There are 9 bodies:
- **009DA950:** +0h = 0, then the 009D5D60 reset (one stream-1 U(0, 2) draw into +48h). Unreferenced.
- **009DBD60:** the "NavigatorBot" named object constructor (vtables 00D17DC0, then 00D217DC; the name duplicated by 00438E40 into +8h). Unreferenced.
- **009DE1B0:** zeroes +360h, +368h and +1C4h when +1C4h is set, resets the path plan (009DA4E0), and stores the throttle clamped to [-1, 1] at +1D4h. A NaN throttle is stored unchanged.
- **009DFB90:** the observer member setter with a point copy (+18h..+20h). Unreferenced.
- **009E2C40:** the ship-AI bot debug draw: 009DDCC0 on the ship AI at +60h, then the sub-state name through 004C7070 and the state's own text draw at (x, y + 15). Its only caller is the unreferenced adapter at 009E2CB0.
- **009E2F10:** slot 1 of the brain record member vtable 00D21898 (brain+AF8h). It first turns the stored offset into a world point through the observed entity's matrix ([member+14h]+CCh) and sets +24h. Only then does it test released->vtable[4]() against [member+14h] and unlink on a match. It has no reach on the goal vector (docs/SHIP_AI_OPEN_ITEMS.md).
- **009E69D0 / 009E6A30:** heading-error ramps. Each stores 00419010(0, peak, span, 0, |wrapped error|) at +34h (span = pi) or +38h (span from the stack). Both are unreferenced, and the owning class is unidentified.
- **009F3CC0:** the gun bot's sub-state name (vtable 00D21AE8 slot 5), or the empty string 00CE3A0C when there is no sub-state.

There are 16 ledger-only rows (section 4):
- 13 byte twins. They twin 008FC570, 0042D870 (x2), 00465060, 006F3490, 0042EC20 (x4) and 0042E700 (x4).
- 009DFA60, 009DFA90 and 009E5220 were first recorded here as twins of 0046A090 and 0041E1B0. They are not twins: see section 3.50.
- 009DAE30 and 009DB7F0 copy the goal vector brain+B2Ch..B34h. The hosts read that field directly.
- 009E9C40 is the host override `tubes_min_water_speed_0aec`.

### 3.49 Medium rows, batch 4 (packet cc13-seg67C)

Same checks as batch 3. A scan for absolute dwords and rel32 targets backs each "unreferenced" note and each vtable slot below.

There are 11 bodies:
- **Describes with a tail:** each appends 004B8490 text through 007B7CC0 after the base describe.
  - 009AB220, dogfight: "_follow" or "_goaway" in the go-away state (task+6A4h), then "_autoaim".
  - 009B3FF0 (land) and 009CFDD0 (takeoff): " spd:%f" with the unit's vtable[38h] speed.
  - 009D4C90, torpedo: " dr:%f  dir:%d lr:%d-%d" with 009D1500 and three task dwords.
- **009AC7C0 / 009ACD90:** the kamikaze go-towards and aim debug draws, 41 identical instructions sharing one helper.
- **009BB4A0 / 009BB530:** the two unreferenced tight-turn wrappers over the existing 009BB380 body.
  - 009BB4A0 takes atan2(dx, dz) and reads the heading first.
  - 009BB530 takes atan2(dz, dx), maps it to pi/2 - a in [0, 2pi), and fires only above the threshold.
  - std::atan2 in double stands for _CIatan2, as in 00996270.
- **009D9390:** the path node's turn point through 00423520. Its only caller is unreferenced code at 009D9F90.
- **009E7FA0:** the unreferenced brain wrapper of 009DE280.
- **009EAAD0:** the sub-attack debug text. The tube counts come from 009E9910 and 009E9A50.

There are 4 ledger-only rows (section 4):
- 009C3D80 and 009C8D20 are the dive-bomb and retreat follow-state selectors, hosted where each task picks its first state.
- 009C91A0 is 009C8D40 then 009C8F40, the host's `retreat_refresh_approach`.
- 009D32A0 is the byte twin of 006F8290 ("torpedo").

### 3.50 Three false twins corrected (packet cc13-seg67C)

Three rows of section 3.48 matched a recorded body's bytes outside the rel32 operands but call a different target. The twin scan now also requires every rel32 CALL, JMP and Jcc target to be equal; a target inside the body is compared relative to the body start. The rerun over all of this packet's twin rows and over the batch 3 and 4 bodies found no other false twin.

Each is now a body. An image scan finds no absolute or rel32 reference to any of them.
- **009DFA60:** frees the owned path node through 009DA880 and 00BF65AC, then clears the slot. 0046A090 calls 00469010 instead.
- **009DFA90:** the same shape over 0064A610, the observed-owner base destroy.
- **009E5220:** +4h = 009DF8F0() (a 0Ch self-linked sentinel), +8h = 0. 0041E1B0 calls 0041BF50, which allocates a 10h node.

### 3.51 Medium leftovers and host comparisons, batch 5 (packet cc13-seg67C)

Same checks as batch 4. The twin scan is the target-equal one. Each host-named row was read against the named host function and the switch above it.

There are 5 bodies:
- **009B8260 / 009C80D0:** the level-bomb and dive-bomb debug draws. Each draws a red sphere at its task marker (+4C8h / +4D0h) and a white sphere at the approach target; the two share one helper.
- **009D7780 / 009D7930:** oriented-rectangle distances. Both are unreferenced.
  - Each projects the delta on two axes, clamps it to the half-extents, rebuilds the closest point, and takes sqrt of its squared distance, with 0 at or under 1e-10.
  - 009D7930 takes the delta from +44h/+48h but rebuilds the point from +20h/+24h. Byte +68h short-cuts it to the delta's own length.
  - Every x87 step is modelled as a float operation.
- **009D6E50:** the 65-byte profile sampler. It is unreferenced.
  - It clamps the angle, indexes the profile, and walks left and right for the nearest lower byte (or the first higher one).
  - It writes back pos / 16 - 2, clamped through 00415690, and returns the chosen byte.

There are 20 ledger-only rows (section 4). 18 match their hosts, with these notes:
- The level-bomb moveto and attackrun enters draw on stream 0. The host takes that draw through its keyed stand-in `lb_draw`.
- Land/final's +24h copy and the plane+844h clear are the host's own labelled gap. The fly-over enter's plane+844h clear is not modelled either.
- Retreat's still-valid test does not distinguish the null-squadron answer 2.
- 009BE2C0 is also a target-equal byte twin of 009A6B70.

The other two are partial and went to the lead as candidate torpedo mismatches:
- **009D2530**, the done/prepare enter, resets +98h to -1.0 on every entry. The host resets `torpedo_drop_timer` only at install, and done and prepare share that one timer.
- **009D15D0**, the aim enter, clears the aim-complete byte +2Ch. The host's `torpedo_aim_complete_2c` is only ever set, so it stays true once set.

## 4. Ledger-only records (packet cc13-seg67C)

| Address | Host body | Result |
|---|---|---|
| 0099B790 | `ga_insert_0099b790` | matches; the array order after a multi-merge differs, but the set of pairs is the same |
| 0099BF30 | `gunfire_repair_0099bf30` | matches; differences only on NaN fallbacks |
| 0099EC40 | `gunfire_detect_0099ec40` | **mismatch**: the exempt arm's +840h WandererMul store (0099F197) is missing; adoption note sent |
| 009A7A50 | `df_avoid_common_009a7a50` | **mismatch**: the U(1, 2) redraw at 009A7D72 is a keyed stand-in under kDogfightAvoidTimerDrawBound (ON); the shared stream-1 draw itself is still not taken. Approach+100h bit 2 is not modelled; adoption note sent |
| 009AFAF0 | `land_geometry_009afaf0` | matches; the speed divisor (vtable[38h]) is the host's \|v\| |
| 009B1420 | `land_begin_steer_009b1420` | first third (009B1420-009B15BC) matches; coverage partial, 009B15C0 onward not re-read |
| 009C7A80 | `update_dive_bomb_approach` | matches, with the host's labelled substitutions |
| 0099B940 | `ga_band_search_0099b940` | matches; a NaN value takes the first pair in the host, while the image skips every pair |
| 009A17D0 | `gunfire_avoidance_009a17d0` | matches; the image zeroes all of +26Ch (009A17E9), the host only its modelled bit 4 |
| 009A85B0 | `df_threat_009a85b0` | matches apart from cc9's labelled gap: the T+9C0h / 007B9140 clause is skipped |
| 009AAA80 | `df_early_edge_009aaa80` | **gap**: the failed gate's 007B9740 clear (009AAC63) is missing; the finder noise is at its midpoint; adoption note sent |
| 009998A0 | inline task arm in `game_hosts_units.cpp` | partial: only the vtable[64h] arm is modelled; the 0099C270 fast path, the +308h pacing of 0099B740 and the C24h gate on 009FC7C0 are not |
| 009A57D0 | `depth_charge_entry_state` | matches |
| 009B3900 | inline in `run_land_task_tick_009b3eb0` | matches; dt * 60.0 is rounded at 24 bits in the image and at 53 in the host; the NaN dt arms differ |
| 009BFD70 | the +85h latch in `run_follow_law_009bfee0_009bee30` | matches; the host's distance and dot are in double, the image's at 24 bits |
| 009C47D0 | `goaway_turn_heading_009c47d0` | matches every literal and offset the host lists (goaway class, no note needed) |
| 009C3310 | inline moveto state rule | matches, including the NaN arm |
| 009C3570 | inline moveto approach update | matches; the NaN arms differ, and +6Ch is the host's labelled substitution |
| 009CA310 | inline strafe range update | matches |
| 009CFA80 | `takeoff_altitude_floor_009cfa80` | matches, with the host's labelled layer substitution |
| 009CFB60 | `takeoff_slow_leave_009cfb60` | matches; the host takes the first arm (+AA0h carried as 0) |
| 009CFC70 | `takeoff_rule_009cfc70` | matches; the image returns with RET 4 |
| 009D31B0 | `aim_hold_009d31b0` input | matches |
| 009D57E0 | `static_zone_blocks_009d57e0` | matches |
| 009D66B0 | the pass-side queue entry in `post_pass_side_009d8c60` | matches the message's content (observed id, side) |
| 009D8C60 | `clear` and `post_pass_side_009d8c60` in `ship_ai_neighbour_clips.cpp` | matches |
| 009DA0D0 / 009DDBC0 | `hold_009da0d0` / `replan_finish_009ddbc0` | matches the block copies; the station block's field mapping was not re-derived |
| 009DA690 | the find lambda in `deliver_pass_side_messages` | matches |
| 009DA8D0 | `tail_009da8d0` | matches; the NaN dt arms differ |
| 009DBBC0 | `list_element_009dbbc0` | matches |
| 009E6170 | `traffic_record_active_009e6170` | matches; the image rounds d2 to a float, the host compares in double |
| 009E6240 | `advance_traffic_record_009e6240` | matches; the U(2, 3) draw is the host's labelled keyed stand-in |
| 009E8360 | `TrafficRecord{}` in `insert_traffic_record` | matches the read fields (timer -1.0, weight 1.0) |
| 009E99A0 / 009E9AE0 | `tubes_min_reload` | matches; the host's operational test is the same constant for every tube |
| 009E9BC0 | `torpedo_bot_range_00901ba0`'s list pick | matches |
| 009EAA10 | `ship_ai_sub_attack_tick_009eaa90` (src/ship_ai_sub_attack.cpp) | matches; NaN arm differs (FCOMI JB takes cd - dt), unreachable with finite timers, given a finite dt |
| 009F30F0 | `ship_ai_approach_enter_009f3220` | matches; sixty keyed draws stand in for the stream-1 U(0, 2) at 009F314E |
| 009F3D00 | `select_state_for_command_009f3d00` | matches |
| 009F59F0 | `auto_target_owner_can_engage_009f59f0` (src/bot_fire_target.cpp) | matches |
| 009F5D30 | `auto_target_scan_009f5d30` (src/bot_fire_target.cpp) | matches; the +1D4h skip is carried by the gunnery host's candidate build |
| 00999610 | `kBulletThrowMul` PilotBot row (src/game_hosts_gunnery.cpp) | matches the robots row +250h; the host reads the authored values |
| 0099B710 | inline constant true (src/game_hosts_units.cpp) | matches (`MOV AL,1 / RET`) |
| 009A54D0, 009A99E0, 009ADDC0, 009AF060, 009B7BB0, 009BCFC0, 009BCFD0, 009BCFE0, 009C9E20, 009CC3D0, 009D3280 | command-class / name / slot constants in depth_charge_task_cc13.hpp, attack_commands.cpp, kamikaze_task_cc13.hpp, moveonpath_task_cc13.hpp, plane_retreat_task.hpp | match |
| 009A9C60, 009B3750, 009CC8D0 | the follow-or-base wait values in game_hosts_units.cpp | match |
| 009B0230, 009B0240, 009B02E0, 009B02F0, 009B21C0 | the land state machine in game_hosts_units.cpp | match |
| 009ACFB0, 009CAD90, 009D0790 | the attack-run enter resets (kamikaze_task_cc13.cpp, strafe_task.hpp, game_hosts_units.cpp) | match |
| 009DAB10, 009DAE20, 009E6480, 009E8540 | game_hosts_ship_ai.cpp, ship_ai_states.hpp, ship_ai_state_steps.cpp | match |
| 009A6BF0, 009AE0D0, 009B8180, 009C7FF0, 009B4AF0, 009B4B60, 009B4C10, 009B4C90, 009BD3C0, 009C88E0, 009C32D0, 009CA730 | byte twins (rel32-masked) of 004BEAD0, 009A5710, 007EDD70, 007EDDE0, 006F3410, 00B0A440, 009A2B90, 009A56C0, 007B4890 | twins |
| 009AF2B0 | `set_state` (src/bot_task_states.cpp ~231) with the constant next = task+654h (kamikaze gotowards); no stack argument (RET) | matches |
| 009A5DB0, 009A9CC0, 009AE140, 009AF560 | `attack_task_still_valid` (attack_commands.cpp ~359) family; kamikaze_task_cc13.hpp ~291 | match |
| 009AE210, 009A35A0, 009A7F70, 009AB1C0 | `drop_kamikaze_manual_release_009ae210`; `depth_charge_approach_update_009a5f50` (~233); `df_avoid_roll_weight_009a7f70`; `df_arm_body_009ab1c0` | match |
| 009A3160, 009A6B10, 009AB6D0, 009AB7F0, 009B4380 | the initial-state selectors (`depth_charge_task_construct_009a5240` ~380, `dogfight_unengaged_state_009aafa0`, `kamikaze_initial_state_009aeef0` (~405), `drop_kamikaze_initial_state_009adbf0`, `level_bomb_next_state_009b88f0` ~121) | match |
| 009A6B70, 009B4670, 009B4A30, 009B7A50 | dogfight_task.hpp ~225; plane_level_bomb_task.hpp ~176; `level_bomb_attackrun_009b5320` (~303) and `level_bomb_prepare_arm_009b8100` (~390) | match |
| 009A4900 | the drop arm of `depth_charge_follow_tick_009a4940` (009A4A89-009A4ABA) | matches; 009A4900 has no 0.3 re-arm |
| 009B94B0 | the flikflak arm of `st_manoeuvre_tick` (game_hosts_units.cpp ~24728) | matches (one stream-1 draw) |
| 009B8450 | `bot_task_step_result` (src/bot_tasks.cpp ~121) | matches |
| 009D3E80 | byte twin (decoded rel32 masked) of 009A5710 | twin |
| 009BC6F0, 009BDDC0, 009CA380, 009D0100 | `moveonpath_rule_state_009bd17f` (~124), `moveto_task_construct_009c3000` (~81), `st_install_009cc230` (~24579), `torpedo_next_state_009d4030` (~76) | match |
| 009D5990 | `ship_ai_path_node_decide_direction_009d5930` then `ship_ai_path_node_next_009d9ee6` (~147) | matches |
| 009DBD80 | byte twin (decoded rel32 masked) of 008FC570 `BSP_RobotConfig_DestructBase` (`delete_robot_descriptor`, src/robot_config.cpp) | twin |
| 009DF980, 009E4E50 | byte twins of 0042D870 `observer_ref_set_0042d870` (src/profile_seg2_cc13.cpp) | twin |
| 009DFA40 | byte twin of 00465060 `BSP_Unit_CastToSubmarine` (src/actions_seg5_cc13.cpp) | twin |
| 009DFAC0 | byte twin of 006F3490 `BSP_Entity_AsKind0C` (src/cc13_seg33b_command_building.cpp) | twin |
| 009E2CE0, 009E2D10, 009E2D90, 009E2DC0 | byte twins of 0042EC20 `observer_ref_on_observed_released_0042ec20` (src/profile_seg2_cc13.cpp) | twin |
| 009E2E10, 009E4F50, 009E51B0, 009E5240 | byte twins of 0042E700 `observer_ref_set_0042e700` (src/profile_seg2_cc13.cpp) | twin |
| 009DAE30, 009DB7F0 | the goal field `ShipAiApproachRetargetInputs::goal` (ship_ai_approach_update.hpp ~1086) and `brain_goal_x` (game_hosts_ship_ai.hpp ~274) | match (unreferenced getters) |
| 009E9C40 | the host override `tubes_min_water_speed_0aec` (game_hosts_ship_ai.cpp ~9887) | matches (009E97B0 then +AECh) |
| 009C3D80 | `dive_bomb_entry_state_009c8310` (src/dive_bomb_task.cpp ~95; DiveBombState kMoveTo 4F0h, kFollow 52Ch) | matches |
| 009C8D20 | `install_retreat_task_009c9d00` (game_hosts_units.cpp ~9485; RetreatState kMoveTo / kFollow) | matches |
| 009C91A0 | `retreat_refresh_approach` (game_hosts_units.cpp ~9412) over `retreat_refresh_zone_009c8d40` and `retreat_refresh_target_009c8f40` | matches |
| 009D32A0 | byte twin (decoded rel32 masked) of 006F8290, the torpedo row of `kEntityOrderCommandClasses` (src/entity_orders.cpp ~37) | twin |
| 009B0980, 009B09A0, 009B13E0 | `land_enter_abort_009b0980` (~33170), `land_exit_abort_009b09a0` (~33187), `land_enter_begin_009b13e0` (~32684), game_hosts_units.cpp | match |
| 009B1E60 | `land_enter_final_009b1e60` (game_hosts_units.cpp ~32998) | matches apart from the host's labelled gap (+24h, plane+844h) |
| 009B5C60, 009B6CE0, 009B4BD0 | the kAim / kMoveTo / kAttackRun arms of `lb_enter` (game_hosts_units.cpp ~25378) | match; the stream-0 U(0.4, 0.7) draws go through the keyed `lb_draw` |
| 009CE270 | the SlowTakeoff -> Takeoff switch (game_hosts_units.cpp ~34291) | matches |
| 009D24E0 | the torpedo install (game_hosts_units.cpp ~29576) | matches (+98h = -1) |
| 009C8260, 009D4970, 009B8420 | `member_follows_007bcc20` (game_hosts_units.cpp ~8826) | match |
| 009BE2C0 | target-equal byte twin of 009A6B70 (src/dogfight_task.cpp); moveto reads +48h..+50h (~35690) | twin |
| 009BC760, 009CA680 | MoveOnPath `ref_34` (moveonpath_task_cc13.hpp ~58), `st_aim_point` (game_hosts_units.cpp ~603) | match |
| 009C9E30 | `run_retreat_task_tick_009ca130` (game_hosts_units.cpp ~34893) | matches on the command class; the null-squadron 2 is not distinguished |
| 009C6270 | the kFlyAbove enter (game_hosts_units.cpp ~27795) | matches; plane+844h not modelled |
| 009CA820 | `st_gotowards_enter_009ca820` (game_hosts_units.cpp ~24608) | matches with the labelled substitutions |
| 009D2530 | the done/prepare arm of `set_state` (game_hosts_units.cpp ~21909) | **partial**: no +98h reset on enter; done and prepare share one host timer; sent to the lead |
| 009D15D0 | the aim enter redraw (game_hosts_units.cpp ~29996) | **partial**: +2Ch is never cleared in the host (sticky `torpedo_aim_complete_2c`); sent to the lead |
