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
