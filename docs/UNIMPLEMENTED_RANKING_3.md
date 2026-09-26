# Unimplemented host records, ranking 3

cc9-platform2, 2026-09-26, packet `cc9_unimplemented_ranking_3`. It replaces
`docs/UNIMPLEMENTED_RANKING_2.md`. The input is three runs of one fresh build of **main 8c6cf2c49**
(`local\bin\rk3`), with an idle player, `BSP_GUNNERY_RNG_STREAMS=1`, and a 1600x900 window. All
three logs show the resolution fit line and the final COM release.
- **E2**: `local\rk3_e2.log`, USN04 9200/9000.
- **USN04**: `local\rk3_usn04.log`, USN04 4700/4500.
- **USN02**: `local\rk3_usn02.log`, USN02 9200/9000.

The script is `local\rank3.py`. It reads UNIMPLEMENTED rows (name + address) and compares them with
ranking 2's E2 log `local\ab_on_e2.log`.

| | E2 9000 | USN04 4500 | USN02 9000 |
| --- | ---: | ---: | ---: |
| unimplemented calls | 4,078,603 | 2,249,389 | 4,302,303 |
| rows | 550 | 541 | 507 |
| ranking 2 | 4,538,628 | 2,399,229 | not taken |

The classes are ranking 2's:
- **mirror**: bound elsewhere, render-side or unread;
- **gap**: unread behaviour that may write state;
- **partial**: some of the behaviour is bound;
- **contract**: a library.

The owning file is where the record's literal sits. `src/game_hosts_units.cpp` (the planner) and
`src/game_hosts_ship_ai.cpp` (the gunnery worker) are leased for hours. Section 3 lists the gaps
outside them.

## 1. Top 30 rows by calls

The rate is calls per 1000 mission frames (E2 / USN04 / USN02). Ranked by rate, the order is the
same except for three things:
- USN02 lifts the ring-scan probes (rows 9 to 11) to 18,440 per 1000 frames, against 2,247 in E2.
  Surface fights run the ring scan far more often.
- The aircraft rows (1, 13, 16, 18, 22 to 24, 26, 27) are zero in USN02, which has no aircraft.
  Rows 14 (`navigation_arm_tail`) and 15 (the avoid-zone sample) are zero there too.
- A few rows do not scale with mission length, so their USN04 rate is higher. The clearest is
  `AvoidZoneLayer::sample_0041bc20`: 101,505 calls in both USN04 runs, so it is front-loaded.
  `PilotBot::plan_controls` (14,979 against 19,264) and the move-to rows are next; aircraft are
  busier in the first half of USN04.

| # | row | address | E2 | USN04 | USN02 | rate | owning file | class |
| ---: | --- | --- | ---: | ---: | ---: | --- | --- | --- |
| 1 | `Plane::device_busy_1fc` | 007ceb00 | 361508 | 187201 | 0 | 40168 / 41600 / 0 | game_hosts_units.cpp | gap: 007CEB00's device walk, the plane tick (planner) |
| 2 | `ShipMotion::unit_post_motion` | 00826b84 | 163434 | 81000 | 285540 | 18159 / 18000 / 31727 | game_hosts_units.cpp | gap: 00825F20's tail after the controller step |
| 3 | `ShipMotion::rigid_body_substep_schedule` | 00c5bb30 | 163434 | 81000 | 285540 | 18159 / 18000 / 31727 | game_hosts_units.cpp | contract: Dyn_World_Substep, physics library |
| 4 | `ShipAi::unit_weapon_director` | 00cfc3d0+vtable114 | 162000 | 81000 | 252000 | 18000 / 18000 / 28000 | game_hosts_ship_ai.cpp | mirror: vtable dispatch, value bound elsewhere |
| 5 | `ShipAi::tail_009da8d0` | 009da8d0 | 162000 | 81000 | 252000 | 18000 / 18000 / 28000 | game_hosts_ship_ai.cpp | gap (small): the torpedo-warning timer; gunnery packet cc9_ship_ai_tails_2 |
| 6 | `ShipAiOrder::slot_to_order_ring` | 00825f7c | 162000 | 81000 | 252000 | 18000 / 18000 / 28000 | game_hosts_ship_ai.cpp | mirror: slot+40h..+48h, not read by steering |
| 7 | `UnitInstance::smooth_intensity` | 008227e0 | 189000 | 94500 | 252000 | 21000 / 21000 / 28000 | game_hosts_units.cpp | mirror: visual intensity smoother |
| 8 | `ShipAi::drive_heading_vtable50` | 00cfc3d0+vtable50 | 162000 | 81000 | 252000 | 18000 / 18000 / 28000 | game_hosts_ship_ai.cpp | mirror: vtable dispatch, heading bound |
| 9 | `ShipAiRingScan::probe_space_vtable_0218` | 009e66eb | 20220 | 20220 | 165960 | 2247 / 4493 / 18440 | game_hosts_ship_ai.cpp | partial: the ring scan's avoid-zone probe; no zone space |
| 10 | `ShipAiRingScan::probe_hit_0041b4e0` | 0041b4e0 | 20220 | 20220 | 165960 | 2247 / 4493 / 18440 | game_hosts_ship_ai.cpp | partial: the same probe's hit test |
| 11 | `ShipAiRingScan::probe_origin_00417b10` | 00417b10 | 20220 | 20220 | 165960 | 2247 / 4493 / 18440 | game_hosts_ship_ai.cpp | partial: the same probe's origin push |
| 12 | `ShipAiArmTail::after_arm` | 009de5b0 | 109814 | 59760 | 145972 | 12202 / 13280 / 16219 | game_hosts_ship_ai.cpp | gap: 009DE5B0; gunnery packet cc9_ship_ai_tails_2 |
| 13 | `PilotBot::plan_controls` | 0099d300 | 134813 | 86688 | 0 | 14979 / 19264 / 0 | game_hosts_units.cpp | gap: planner-owned |
| 14 | `ShipAi::navigation_arm_tail` | 009eeaab | 110906 | 60814 | 0 | 12323 / 13514 / 0 | game_hosts_ship_ai.cpp | gap: 009EEAAB; gunnery packet cc9_ship_ai_tails_2 |
| 15 | `AvoidZoneLayer::sample_0041bc20` | 0041bc20 | 101505 | 101505 | 0 | 11278 / 22557 / 0 | game_hosts_units.cpp | gap: the avoid-zone layer sample; the sea stands in |
| 16 | `BotTaskGun::tick` | 009fc7c0 | 45576 | 18576 | 0 | 5064 / 4128 / 0 | game_hosts_units.cpp | gap: planner/dogfight-owned |
| 17 | `MissionCamera::collision_ray` | 0098b370 | 44995 | 22495 | 44995 | 4999 / 4999 / 4999 | game_hosts_hud.cpp | partial: the camera's collision probe; no unit collision shapes |
| 18 | `BotApproachMoveTo::command_block_range_6c` | 009c359f | 42065 | 23254 | 0 | 4674 / 5168 / 0 | game_hosts_units.cpp | gap: new with the move-to chain (planner) |
| 19 | `ShipAiArmTail::neighbour_list_count` | 009eeb8b | 14862 | 9768 | 41037 | 1651 / 2171 / 4560 | game_hosts_ship_ai.cpp | partial: the same world list |
| 20 | `ShipAiArmTail::entity_hull_radius` | 009eebe0 | 14862 | 9768 | 41037 | 1651 / 2171 / 4560 | game_hosts_ship_ai.cpp | partial: world entity list absent (004DE610 is a load record) |
| 21 | `HudShipScreen::gauge_digit_uv` | 0043aba0 | 36316 | 18316 | 36316 | 4035 / 4070 / 4035 | game_hosts_hud.cpp | mirror: GUI vertex UVs, render-side |
| 22 | `BotStateFollow::steer_to_point` | 009f9e40 | 34479 | 18811 | 0 | 3831 / 4180 / 0 | game_hosts_units.cpp | gap: planner-owned |
| 23 | `BotStateFollow::command_step` | 009bee30 | 34479 | 18811 | 0 | 3831 / 4180 / 0 | game_hosts_units.cpp | gap: planner-owned |
| 24 | `BotStateFollow::steer_point` | 009bfee0 | 34479 | 18811 | 0 | 3831 / 4180 / 0 | game_hosts_units.cpp | gap: planner-owned |
| 25 | `ShipAiFollow::push_out_of_zones` | 00417b10 | 29070 | 14670 | 12594 | 3230 / 3260 / 1399 | game_hosts_ship_ai.cpp | partial: avoid zones absent |
| 26 | `BotApproachMoveTo::arrived_vtable8` | 009c3647 | 27818 | 9246 | 0 | 3091 / 2055 / 0 | game_hosts_units.cpp | gap: new with the move-to chain (planner) |
| 27 | `BotTaskDogfight::follow` | 009c1fd0 | 27462 | 9462 | 0 | 3051 / 2103 / 0 | game_hosts_units.cpp | gap: planner/dogfight-owned |
| 28-30 | `ShipAiApproach::*` (13 rows at one count) | 009F1DBF..009F339A | 3008 | 3008 | 24773 | 334 / 668 / 2753 | game_hosts_ship_ai.cpp | partial: the approach sub-state's unbound terms, one call each per approach frame |

## 2. What moved since ranking 2

The E2 columns are compared, ranking 2's `local\ab_on_e2.log` against `local\rk3_e2.log`.

**Closed (concrete or gone now).**

| ranking 2 row | E2 then | now | closed by |
| --- | ---: | --- | --- |
| `ShipAiOrder::tail_009ef910` | 162,000 | concrete | afff8eb6a, packet cc9_ship_ai_turn_clearance |
| `ShipAiPath::refresh_plan_head` 009ED3E0 | 136,509 | concrete | the same packet |
| `ShipAi::hold` 009DA0D0 | 129,193 | concrete | 1ff05f399, kShipAiSnapshotBound |
| `ShipAi::replan_finish` 009DDBC0 | 32,807 | concrete | the same |
| `Gunnery::recon_slot_contacts_008053c0` | 28,486 | concrete | cc9_recon_team_lists (a6a77c147) |
| `UnitPickScreen::segment_query` 009043A0 | 18,160 | concrete | 5edd9e5b5, kHudPickSegmentQueryBound |
| `UnitPickScreen::team_unit_list`, `kind35_list`, `HudMinimap::team_unit_list` (004C3CB0 sites) | 18,160 each | gone | 710efc679, kReconUnitListSourcesBound |
| `HudMinimap::camera_heading_virtual`, `HudMinimap::camera_unit` 004B4B00 | 18,160 each | gone; concrete | 6e73d4bb6, the minimap wedge's camera unit |

**Moved but still open.**
- `ShipAi::navigation_arm_tail` (137,641 to 110,906) and `ShipAiArmTail::after_arm` (136,509 to
  109,814) fell. The snapshot moved E2's steps from the path-pick arm to the station-keeping arm
  (docs/SHIP_AI_TAILS.md section 5: path picks 144,062 to 115,202).
- `Plane::device_busy_1fc` fell (391,703 to 361,508). `PilotBot::plan_controls` rose (89,441 to
  134,813), and so did the `BotStateFollow` and `BotTaskGun` rows. These are the planner's
  landings on the aircraft paths (the dive approach and the move-to glide, 8cbf76762 and after).
  This packet did not attribute them row by row.
- The ring-scan probes (16,800 to 20,220) and the `ShipAiApproach` family (2,496 to 3,008) rose
  with the changed fights. No switch newly exposed them.

**Newly exposed (unimplemented now, absent before), 5,000 calls or more in E2.**

| row | E2 | owning file | exposed by |
| --- | ---: | --- | --- |
| `BotApproachMoveTo::command_block_range_6c` 009C359F | 42,065 | units.cpp | the planner's move-to chain |
| `BotApproachMoveTo::arrived_vtable8` 009C3647 | 27,818 | units.cpp | the same |
| `BotStateFollow::trail_arm_85` 009C207C | 20,259 | units.cpp | the same |
| `BotStateMoveToCircle::steer` 009FBB20 | 13,929 | units.cpp | the same |
| `InGameInterfaceUpdate::query_006529c0` | 9,000 | game_hosts_hud.cpp | a record since 895ab0c89, first reached now; 006529C0 is `MOV AL,[ECX+66h]; RET` |
| `BotApproach::refresh_path_leg_009fd050` | 8,449 | units.cpp | the move-to chain |
| `BotStateMoveTo::target_speed_override_009c23b0`, `direction_009fabe0` | 7,856 each | units.cpp | the same |

## 3. The top gaps outside the planner's and gunnery's leased files

Most of the largest rows outside the two files are once-per-frame HUD records (18,160 calls, two
HUD passes per frame). Many are mirrors of state this process does not have: session bytes such
as `game_19c4`, input records, GUI resources. The real gaps, meaning unread behaviour that can
write state, by owning file:

| # | row | address | E2 | USN02 | owning file | what it is |
| ---: | --- | --- | ---: | ---: | --- | --- |
| 1 | `MissionEvents::pre_pass` | 00982540 | 9,000 | 9,000 | game_hosts_mission_frame.cpp | `BSP_WarningManager_PumpInputChannel`, 1,100 bytes |
| 2 | `MissionEvents::poll_zones` | 0096D540 | 9,000 | 9,000 | game_hosts_mission_frame.cpp | `BSP_WarningManager_PollPrompt` |
| 3 | `MissionEvents::poll_triggers` | 00968550 | 9,000 | 9,000 | game_hosts_mission_frame.cpp | `BSP_WarningManager_UpdateDeadlines` |
| 4 | `PlayerGunSeat::segment_query` | 00957DA0 (in 00957BD0) | 18,158 | 0 | game_hosts_gunnery.cpp | the player seat's segment query; `query_segment_units` (09298b10e) can serve it |
| 5 | `MissionCamera::collision_ray` | 0098B370 | 44,995 | 44,995 | game_hosts_hud.cpp | the camera's collision probe; it moves only the camera |
| 6 | `PowerUps::post_pass` | 00613760 | 9,000 | 9,000 | game_hosts_mission_frame.cpp | the power-up pass, not read for this ranking |
| 7 | `AiGroups::seed_collection` | 00A2E835 | 9,000 | 9,000 | game_hosts_ai.cpp | the AI group seed walk, not read for this ranking |

**Rows 1 to 3 are misnamed.** The records say `MissionEvents`, but all three addresses are the
warning manager at [00F8A0C4]. That is the HUD's warning system, the same object 009DA8D0's
torpedo-warning timer calls through 00977690/00977820 (docs/SHIP_AI_TAILS.md section 2). They run
once per world tick, and each is unimplemented on every frame. The fourth warning-manager record
in the same binding, `MissionEvents::periodic` (00977990, `BSP_WarningManager_ScanProximity`),
was not reached in any of the three runs.

## 4. Is the ray pick's downward camera the image's?

Yes. The zero own-formation hits in `docs/HUD_PICK_SEGMENT_QUERY.md` section 6 follow from the
image's camera, not from a host choice.
- `docs/MISSION_CAMERA.md` section 3: 0064DA40 seeds the ShipCaptain mover's pitch +388h from
  00CECA08. The bytes are `C3 B8 32 BE`, -0.1745329 as a float: -10 degrees. The yaw comes from
  the unit's pose.
- Its update 00432ED0 (section 4) pivots at (unit.x, class CameraMinHeight, unit.z), applies
  `RotX(-pitch)` and offsets by the class camera distances. The pair logged the Lexington's as
  Front 250, Side 150, Vertical 50, CameraMinHeight 45, Length 250 (section 12, and again in
  `local\rk3_usn04.log`).
- That gives the probe's camera: 53.7 up, about 250 behind, forward pitched 10 degrees down.
- With an idle player nothing changes the orbit.
- `kMissionCameraBound` is ON. Its labelled substitutions (section 5) do not touch the pitch or
  the offset: the shake phases are not drawn, and the unit-collision ray never hits.

So the image, too, casts 29h's pick segment into the sea about 300 ahead with an idle player. Hits
need a unit within that distance.

## 5. Recommended next HUD packet

**The warning manager's per-frame tick** (section 3, rows 1 to 3): 00982540, 0096D540 and 00968550 on
[00F8A0C4], with 00977990 behind them, currently misnamed `MissionEvents::*` in
`src/game_hosts_mission_frame.cpp`.
- They are routines of one object, called once per world tick.
- The first step is a read of what they write: the warning queue, prompts and deadlines, and
  whether anything outside the HUD reads them (the proximity scan may feed sounds or AI).
- A binding follows under one switch.
- It needs a lease on `src/game_hosts_mission_frame.cpp`'s world-tick binding, which is outside my
  current files.
- 009DA8D0's timer (now the gunnery worker's) feeds the same object. The two packets should agree
  on the object's contract.

The seat's segment query (row 4) is a small follow-on for the gunnery worker, now that
`query_segment_units` exists.
