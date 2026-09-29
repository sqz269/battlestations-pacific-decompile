# Ship AI and AI command: open items, ranked

Addresses: 00852860 009E873B 009E26C0 009F3670 00417B10 00811940 009DF41A 009DF432 009DF4C5 009DF607 009DC2E0 00A15970 0070E450 00605070 00A179E0 00A1443D 00827F95 009F1BC0 009FFEB0 00778890 00A0F970 0071C1E0 009E1170 00835C70 00A0C650 00A0C3C0 00A0C330 00A04560 00A04240 00A07E40 009F3220 009F30F0 009E86C0 009E86E0 009E2B60 009DF2D0 009F6A20 007788B0 0077C980 00827FB0 00963C70 00416270 009D7050 009EF910 00A2B8F0

This file ranks what is still open in the ship-AI and AI-command lane, as
docs/GUNNERY_OPEN_ITEMS.md section 31 does for gunnery and docs/LUA_BINDING_MISSION.md does for the
Lua natives. Each later section is one packet taken from the ranking.

## 1. The first ranking (packet `cc9_ship_ai_open_ranking_1`, cc9-ships7, 2026-09-28)

**Superseded by section 16**, which ranks the lane again on main `941fd197b`.

**Source.** The nine reference i logs `local\rb9_{bsm01,e2,jm06,jm08,lomp06,usn01,usn02,usn04,usn13}.log`
in worktree cc9-gunnery7. They were built at main `d466d4250` (docs/GAME_EXECUTABLE.md "2026-09-28 i").
- `local\ships7_census.py` (this worktree) sums every non-concrete host row with calls over the nine
  logs. `rows <regex>` prints each row's calls per mission.
- `local\ships7_sites.py` prints the comment block and the answer at each row's record site in
  `src/`.

**Reference i is older than main.** 114 commits separate `d466d4250` from this branch's base
`79b055574`. Five lane switches were flipped ON after i:
- `kShipAiSubAttackSelectBound` and `kShipAiSubAttackStatesBound` (docs/SHIP_AI_SUB_ATTACK.md 9). The
  submarines no longer run `attackmove`, so the `ShipAiState::attack_subject_00779aa0` row is gone.
- `kCautiousRouteBound` and `kCautiousWedgeBound` (docs/AI_CAUTIOUS_ROUTE.md 9, 14, 17).
- `kAiGroupSeedPerEntityBound` (docs/AI_CAUTIOUS_ROUTE.md 19). Every mission's AI grouping moved, so
  the group-shaped rows below (ranks 5, 6, 8 and 10) need re-counting on reference j.

Where a flip could move a count, the table cites a newer log as well. The two sub-attack ON logs are
`local\s5_on_{jm06,lomp06}.log` in worktree cc9-ships5 (commit `1f39b05db`).

**Reach**, as in docs/GUNNERY_OPEN_ITEMS.md section 16:
- 4 decides who is shot at, or a death.
- 3 decides an order or a ship's movement.
- 2 is a count or a score.
- 1 is presentation.

"Differs" says whether the host's answer differs from the image's on the measured rows.

| rank | item | image | image read | host file and label | calls (i) | differs | reach, in one line |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | **a surface ship attacking a submarine** | the altitude gate `00852860` at `009E873B`; the sub-states it opens are lead pursuit `009E26C0` (`state+14CCh`) and tangent `009F3670` (`state+14E0h`) | gate complete (`00852860-008528AC`, `ship_ai_attackmove_altitude_gate_00852860`); both steps projected (`src/ship_ai_attackmove_substates.cpp`) | `game_hosts_ship_ai.cpp`: `ShipAiAttack::call_00852860` answers false. The two steps are the records `ShipAiAttack::lead_pursuit_step` and `tangent_step` | 1014 (JM06 994, LOMP06 20); 852 / 20 with sub attack ON | yes, whenever the target submarine is below a third of `[+1200h]+[+1204h]` | 3, and 4 if the pursuit is what puts escorts over a submerged boat. The label reads "no producer for +1200h / +1204h, unreachable, no kind-8 target". Both halves are false: JM06 reaches it 852 times, and those words are the dive bands the host holds since SUBMARINE_MODEL 12 (`GameUnitsHost::submarine_band_y`, `73f4f884c`) |
| 2 | **the follower's station point** (bound ON, both halves, section 10) | `009DF2D0`: the zone set `vtable[218h]` at `009DF41A`, the push `00417B10` at `009DF432` / `009DF4C5`, the leader yaw rate `00811940` at `009DF607` | `00417B10` complete (`avoid_zone_group_offset_00417b10`, bound for the ring probe as `GameAvoidZoneRuntime::offset`); `00811940` reconstructed (`GameUnitsHost::unit_current_yaw_rate_00811940`); `vtable[218h]` = `006DFD90`, which the ring probe binds as `zones.group_for_layer` | `game_hosts_ship_ai.cpp` `FollowFormationPointBinding`: `zone_set_218` answers 0, `push_out_of_zones` returns the point, `leader_yaw_rate` answers 0 | push 105916, yaw rate 52958, zone set 52958, on all nine rows | yaw rate: yes, whenever a leader turns. Push: only near a zone, and JM06's ring probe moved no start (`moved_starts=0`) | 3: the speed blend `009DF5E4..009DF65B` takes the leader's current yaw rate, so a follower of a turning leader keeps too much speed. The push label ("the body is unread") is stale |
| 3 | the free-bearing query (**bound ON, section 14**) | `009DC2E0` (`009DC2E0-009DCEA2`), at `009DF0FA` (the arm final step) and `009EC0C1` (the sector scan) | early outs only (docs/SHIP_NEIGHBOUR_AVOIDANCE.md 6). About 300 pseudocode lines are unread | `game_hosts_ship_ai.cpp`: `ShipAiArmFinal::free_bearing_009dc2e0` and `ShipAiSectorScan::free_bearing_009dc2e0` answer false | 450328. Zone rows: JM06 42457, LOMP06 18410, USN01 10438, USN13 6172, BSM01 6069, JM08 2886. No-zone rows (exact by the early out): E2 161339, USN02 122021, USN04 80536 | only for a ship with avoid-zone segments inside its query box; how often that happens is not counted | 3: it replaces `blk+324h`, the heading target, near land. A binding needs the runtime's segment search (`refresh_search`, `search_segment`, `search_arc`). It is the largest read here |
| 4 | the AI command's avoid-zone point | `00417B10` from the command tick (`ai_command_tick.cpp`) | complete, as rank 2 | `game_hosts_ai.cpp`: `AiCommand::avoid_zone_offset_point` returns the requested point. Label "contract: unread", stale | 1193 (USN13 517, E2 255, JM08 160, BSM01 104, USN04 78, USN01 74) | only for a point inside a zone | 3: the destination the AI command orders. It fits in rank 2's packet as the same routine and runtime |
| 5 | the party brain's replan flag | `00A15970`, from `00A182C0`: outside modes 4 to 7 it returns the OR of `brain+0h..+0Ch` `vtable[30h]()` (`00A159E8..00A15A6A`). For those four planners that is `00A18480`, which reads and clears the replan byte `planner+2Ch` (docs/AI_PLANNERS.md). The claim sets that byte | complete (listing read here) | `game_hosts_ai.cpp`: `AiGroups::brain_wants_immediate_think` answers false. Its comment reads only the mode 4 to 7 arms and says no planner sets a replan request; `planner_claim_group` sets no flag | 38491 (E2 8999, USN02 8999, USN04 4499, the others 2999 or 999) | yes, once after each planner claim: 1 to 3 claims per row (`ai parties claims=`) | 3: in the image a claim makes the party think again on the next call instead of 3 to 5 s later, so the first orders come earlier. Cheap: a flag set at the claim and cleared by the query. Where the claim sets `+2Ch` must be quoted from `00A22750` first |
| 6 | the group's area key | `0070E450` (`0070E450-0070E4B2`) at `009DE5B0` and `009ECA20` | unread (98 bytes) | `game_hosts_ship_ai.cpp`: `ShipAiArmFinal::group_area_key_0070e450` answers the leader's own travel layer, so the "moved" searcher is never chosen | 31576 (E2 8938, USN02 6858, USN01 4470, USN04 4438, JM06 3000, USN13 2886, LOMP06 986) | only for a group whose members sit on different travel layers. Per-entity seeding changes every group, so recount | 3: the path search's layer |
| 7 | the heading wrap after a heading store | `00605070` with ECX = `&blk+1D8h` at `009DFF81`, `009E00FA`, and the setter `009DFFB0`; `&brain+1E0h` at `009F3360` | complete (`BSP_Math_WrapAngleInPlace_Provisional`, `ship_ai_firepower_wrap_angle_00605070`) | `game_hosts_ship_ai.cpp`: `ShipAiControls::after_heading_stored` and `ShipAiApproach::wrap_brain_heading` record and do not wrap. `include/bsp/ship_ai_state_steps.hpp` still says "Body unread" | 154026 (USN13 93957, JM08 27233, BSM01 16734, USN02 7720, USN01 5700); 34126 for the approach | unknown: yes only if a heading outside (-pi, pi] is stored and a reader compares it unwrapped | 0 to 3. The binding is a one-line change; its value depends on a census of `blk+1D8h`'s readers, which was not done |
| 8 | the party brain's engagement pass | `00A179E0` (`00A179E0-00A18195`), from `BSP_AiPartyBrain_Think` | unread | `game_hosts_ai.cpp`: `AiParties::party_brain_plan_tail` records | 491, on all nine rows | unknown | unknown, possibly 3. It builds a vector from `brain+20h` (`008EA0C0`) and snapshots the group lists. Whether it issues orders is not established |
| 9 | the close attack's busy member | `[member+538h]->vtable[2Ch]` at `00A1443D`, in `00A13B60` | unread | `game_hosts_ai.cpp`: `AiCommand::close_controller_busy` answers false | 5991 (BSM01 1500, USN02 1315, JM08 782, JM06 672) | unknown | 3: a busy member is not served by the close-attack pass. The same slot decides the Cargo capture weight (3.0 or 0 in `capture_weight_00a03510`) |
| 10 | the planner candidate's base weight | `00A0F970` | **bound ON (section 7)** | `game_hosts_ai.cpp`: `AiPlanners::candidate_base_weight` weighs by member count | 451 (E2 139, USN02 111, USN04 107, JM06 72, LOMP06 22) | likely | 3: which group the planner picks. Recount after per-entity seeding |
| 11 | BigLandingShip | `class+808h` at `00827F95` (the neighbour admission `00827F70`) and in `00A03510` | the byte's readers are read | `game_hosts_ship_ai.cpp`: `ShipAiNeighbour::big_landing_ship_808` answers 0. `game_hosts_ai.cpp` takes the capture weight's 0.1 arm | 6331 (BSM01 4289, JM08 2042) | yes for four classes: this installation's `vehicleclasses.lua` (mtime 2026-05-09, locally modified) sets `BigLandingShip` true on the LSM (class 12), the US LST (41), the IJN LST (91) and the strafeable US LST (345) | 3 when an enemy submarine is near one of those hulls (the admission skips it), and 3 in capture scoring (1.0 against 0.1) |
| 12 | the approach frame state's unread spans | `009F1BC0`: `009F1DBF-009F1E16`, `009F2124-009F2216`, `009F221C-009F237B`, `009F2395-009F26EC`, `009F270A-009F3083`, with the approach-point stores at `009F2216`, `009F237D`, `009F23B5` and `009F26F0` | partial (docs/SHIP_AI_APPROACH_UPDATE.md, routine table) | `game_hosts_ship_ai.cpp`: `ShipAiApproach::frame_state_unread_spans` | 34126 (USN02 29287, JM06 4603, LOMP06 236) | not on USN02's mode 0 (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md 1 finds the host's standoff equal to the image's). Possibly for modes 1, 3 and 4 | 3 for a submarine target (mode 1) or a landing ship at a command building (modes 3 and 4). It follows rank 1 |
| 13 | the carrier arm of the squadron exclusion | `009FFEB0` | the `00E17BF2` test only; the carrier arm is unread | `game_hosts_ai.cpp`: `AiCommand::squadron_excluded_009ffeb0` answers `007EDA90`'s false | 13039, on all nine rows | unknown | 3 for a carrier's squadrons in an AI group. It borders the plane lane |
| 14 | the clearance's path fade | `00778890` and the identity test against `00E08F80` (`moveonpath`), at `009F0000` / `009F0019` | `00778890` complete (`ship_ai_unit_group_leads_00778890`) | `game_hosts_ship_ai.cpp`: `ShipAiClearance::path_fade_00778890` answers false. Its label, "neither has a producer here", no longer holds for `moveonpath`, which the host now runs (`kPilotMoveOnPathBound`, the cautious route) | 24310, on all nine rows | only for a group leader on `moveonpath` | 3: the clearance distance |
| 15 | the navigator's avoidance receivers | `00835640` over `0071C1E0` (`00721A93`'s 5Ah arm); the setters `009DABB0` (torpedo) and `009DABD0` (land) | the route is read; the setters have no caller | `game_hosts_script_orders.cpp`: `Navigator::avoidance_receiver_torpedo` and `_land` | 142 / 130 (USN13 52, USN02 28) | yes: the host never stores the Lua request in `blk+3ECh` | 3, on few calls |
| 16 | a director `stop` and the command begin | `009E1170` (`stop` reaches the cruise arm and latches nothing), `00835C70` at `vtable[78h]` | the `stop` state family is unreconstructed; `00835C70` is run at its own site | `game_hosts_commands.cpp` `CruiseCommand::stop_state_step`; `game_hosts_ship_ai.cpp` `CommandController::begin_command` | 1117 / 1427 | unknown | 3: what a `stop` asks of the ship |
| 17 | the follow request's OwnerPlayer arm | `00779DB4`, `entity+188h` | read | `game_hosts_ai.cpp`: `owner_player_known = false`, so the arm is skipped | follow requests: 36 to 2550 per row, joins 1 to 23 | can only admit a follow the image refuses, between differently owned ships | 3, and rare: docs/AI_CAUTIOUS_ROUTE.md 15 finds USN12's refusals are the image's own |

**Exact or closed, kept off the ranking:**

| row or item | calls (i) | why |
| --- | --- | --- |
| `ShipAiEngageGate::armament_readiness` `009E85CD` | 8548 | exact for every class on these rows except the Kaiten and the Kamikaze Boat (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md 4 and 5.3) |
| `ShipAiApproach::zone_allows_target_00864680` | 26649 | the gunnery host's line-of-sight substitution, kept identical here; it belongs to gunnery |
| `ShipAiTorpedoStandoff::torpedo_bot_accuracy_008fb530` | 17454 | answers the robots.lua table (docs/GUNNERY_OPEN_ITEMS.md 31) |
| `ShipAiOrder::slot_to_order_ring`, `ShipAi::unit_weapon_director`, `ShipAi::drive_heading_vtable50` | 1014678 each | structure. The published triple has twelve readers and none steers (docs/UNIT_AI_ORDER_SLOT_READER.md) |
| `EntityOrder::route_message`, `Session::dispatch_*` | 7545 / 8553 | the local loopback of the session route. The order is applied |
| `ShipAiMoveOnPath::brain_leg_scale_0308`, `ShipAiApproach::set_brain_throttle_0258`, `ShipAiGoal::observer_*` | 5752 / 34126 / 327 | stores with no reader found in the recovered chain (the host comments at each site); not proved absent |
| `ShipAiNavBlock::seed_steering_unprojected` | 273 | the site's comment: only `blk+3C4h..+3E4h` of `009DFCB0` is projected, and the constructor overwrites all of it except `+3DCh` and `+3E0h`. Not re-checked here |
| `EntityCommandArm::path_interface` `007AC9D0` | 104 | answers "no path interface", which is the ship case |
| `AiParties::group_has_member_in_world_set` `00A2C450` | 13 | exact while `Objectives_Add` is unimplemented (the Lua lane) |
| the capture range stand-in (500) | | closed: `kCaptureAccessorsBound` reads `+7A0h` (docs/PLANNER_TASK_CHOICE.md 14). The comment at `game_hosts_ai.cpp` `kCaptureRangeStandIn` still reads as if open |
| the submarine's `attackmove` (`00779AA0`, brain+AB4h) | 44 | closed, ON (docs/SHIP_AI_SUB_ATTACK.md 9) |
| the player's ships in Montpelier's group | | closed, ON (docs/AI_CAUTIOUS_ROUTE.md 19) |
| the cautious route, the wedge, RETREAT, USN12's follow refusals | | closed (docs/AI_CAUTIOUS_ROUTE.md 9, 14 to 17) |
| the order ring's replace, the generated ship and squadron brains, the probe length | | closed, ON (docs/ORDER_RING_REPLACE.md 5, docs/GENERATED_SHIP_AI.md 5 to 7) |
| the attack-move standoff range | | closed: the host holds the image's standoff (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md 1) |
| the torpedo evasion | | closed: the host evades as the image does (docs/TORPEDO_EVASION.md, USN02 2026-09-27) |
| the ship turn rate | | closed within 5%. Its residual is the category-5 gameplay modifiers `008E6430`, which no single-player registration is known to set (docs/SHIP_TURN_RATE.md) |
| the player's scripted helm | | a tool, not an image item (docs/SCRIPTED_HELM.md 9). USN02's primary 2 fails on the phase-2 enemy's gunfire |

**Not ranked, and why:**
- `ShipMotion::*` and `UnitMotion::*` rows are in the units host, which cc9-lua6 holds.
- `Unit::device_requests_release`, `can_release` and `slot_byte_9c0` are the plane lane's.
- `UnitInstance::smooth_intensity` and `wake_setting` are presentation.
- The planner quick-spawn `00A2B400` is a units-host creation, like every host quick-spawn.

**Labels that no longer hold:**
- Rank 1: "no producer" and "unreachable" for `00852860`.
- Rank 2: "the body is unread" for `00417B10`.
- Rank 4: "contract: unread" for the same routine.
- Rank 5: "no planner in this process sets a replan request", on `00A15970`.
- Rank 7: "Body unread" for `00605070`, in `include/bsp/ship_ai_state_steps.hpp`.
- Rank 14: "neither has a producer here".
- The capture range comment in the closed table.

**Next packet: rank 1, `cc9_submarine_target_substates`.**
- **Read** `009E873B..009E876C` again with the gate's inputs. The gate's inputs are the target's
  world Y (`+100h`) and the bands `[+1200h]`, `[+1204h]`. Confirm that these are bands 0 and 1 of
  `submarine_band_y`, and find the attackers on JM06 with their targets' depth levels.
- **Read** the host calls of `009E26C0` and `009F3670`, and what `brain+0B28h` holds when the gate
  opens.
- **Bind** the gate and the two steps OFF, with predictions on JM06 3200/3000 and LOMP06 1200/1000.
  Expect identity on USN01, USN02, USN04 and USN13, which have no submarine.

## 2. A surface ship attacking a submarine (packet `cc9_submarine_target_substates`, rank 1, `kShipAiSubTargetSubStatesBound`)

Worker cc9-ships7, 2026-09-28. The switch was bound OFF first and is now ON. The pairs and the verdict follow the
predictions.

### The image

- **The selector's kind-8 arm** (`009E86F0`, docs/SHIP_AI_STATE_STEPS.md): for a submarine target
  `009E873B` asks `00852860`. When it answers true, the member is lead pursuit (`state+14CCh`) with
  `brain+0B28h` set, or the tangent circle (`state+14E0h`) with it clear. When it answers false,
  the member is the approach (`state+8h`) unless the engage member holds.
- **The gate `00852860`** (`00852860-008528AC`, complete, `ship_ai_attackmove_altitude_gate_00852860`)
  answers `target+100h <= ([+1204h] + [+1200h]) / 3.0`. `+1200h` and `+1204h` are the target's dive
  bands 0 and 1. The units host holds them per boat (`submarine_band_y`, default 0 and -20 from
  `00E0B578`). A boat at periscope depth or deeper opens the gate, and a surfaced one does not.
- **The machine switch `007B6EE0`** calls the old member's `vtable[8]` and the new one's
  `vtable[4]`. The five members' slots, read from the PE:

  | member | vtable | enter `+4h` | exit `+8h` |
  | --- | --- | --- | --- |
  | approach `state+8h` | `00D21994` | `009F3220` | `009E6480` |
  | engage `state+14C0h` | `00D2174C` | `009DB5E0` | `007B3DC0` |
  | lead pursuit `state+14CCh` | `00D2177C` | `009DB670` | `007B3DC0` |
  | tangent `state+14E0h` | `00D217AC` | `009E2BB0` | `009DB7D0` |
  | initial `state+14F4h` | `00D2171C` | `009DB590` | `007B3DC0` |

  - `007B3DC0` is a bare `RET` (`C3`, INT3 from `007B3DC1`).
  - `009DB670-009DB689`: `sub+10h = 2.5f` (`00CF87C8`), `sub+0Ch = 1`, `sub+8h = 0`.
  - `009DB7D0-009DB7D8`: `sub+10h = 0`.
  - `009E2BB0-009E2C3A` (RET at `009E2C3A`, INT3 from `009E2C3B`). Ghidra has no function here and
    shows the bytes inside `FUN_009E2B60`.
    - `sub+8h = 009DB820(sub)`.
    - `009E2B60(sub)` when `200.0 > sub+8h` (`009E2BC4..009E2BCE`).
    - `sub+0Ch = 00BD2F10(1, 30, 40) * 00419010(500, 1, 1000, 0, sub+8h)`, with the interpolation
      first (`009E2C00`, `009E2C24`).
    - `sub+10h = 0`.
  - `009E6480`: `00863780(1)` on `[unit+6DCh]`. `00863780` stores its argument in the gunnery pass
    byte `+7Dh`, and walks the guns' `BSP_Gun_ClearBotFireTarget` only for 0. So the exit sets the
    byte to 1. The gunnery host answers that byte as its constructor's constant 1
    (`torpedo_may_take_fire_target`), so the exit is exact in effect.
  - `009F3220`: `009F30F0` on the nested object, then `sub+14B4h = 1.0f`. `009F30F0` re-seeds the
    sixty ring records (one stream-1 draw each at `009F314E`), frees the list at `+14A4h` and runs
    `009F1BC0(0)`. **The host has never run it**, not even on an attackmove's first selection
    (initial to approach). That is a separate open item, listed below; it stays a record here.
- **The steps** `009E26C0` and `009F3670` are complete and projected
  (docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md). Three of their callees were open:
  - `009DB6C0-009DB778`, read here: `__thiscall(ignored)(float point[2], float inset)`, `RET 8`.
    - It clamps x into `[g+711Ch + inset, g+7128h - inset]` and z into `[g+7130h + inset,
      g+7124h - inset]`, with `g = [00E188A8]`.
    - Each bound is stored as a float, and the low bound is tested first.
    - Those words are the NW and SE map bounds that 004D5EDE selects. The avoid-zone runtime keeps
      its copy (`GameAvoidZoneRuntime::world_bounds`, new).
  - `009E2B60-009E2BA3`, read here. For each child on `[unit+48h]` / `+44h` that answers
    `vtable[5Ch](24h)` (a depth-charge launcher: class 27h `MDepthChargeLauncher` is a kind 24h,
    and its `vtable[1F0h]` is `006FDC50` = `vtable[1E8h](1)`, the latch) and whose `[+3F4h]+80h`
    is 8, it calls `vtable[1F0h]()` (docs/GUN_SHOT_CADENCE.md 10; bound in section 9). Function 8 is in `009542B0`'s depth-charge group 5
    (docs/SHIP_SCREEN_UPDATE.md 31). So the tangent fires the depth charges near the circle.
  - `settings+4D4h` is `SubAttack.SubmarineLostTime` (`gameplay_settings.hpp`), not a release
    delay.
    - This installation's `shipglobals.lua` (line 473, mtime 2024-07-13) authors 30.
    - Its comment says the hunters chase a submarine lost from sight for that long and then give up.
    - So after 35 s in the tangent, with the director stage not 2, `0071E430(director, attackmove, 1)`
      ends the command.
- **`brain+0AF0h`**: 009F144F resets it to 1.0 on every pass, and `009F4DD1` in `009F4DA0` is its
  only reader (docs/SHIP_FORMATION_SPEED.md 2). Both steps write it:
  - the lead pursuit, an interpolation of the heading error or 1.0;
  - the tangent, 1.0 on the goal arm and 0.5 on the heading arm.

### The binding (`kShipAiSubTargetSubStatesBound`, `src/game_hosts_ship_ai.cpp`)

- The gate answers from the target's position and bands. Its image answer is counted on both
  sides, one `summary mission ship ai sub target` line per attacker.
- The selector's switch runs the exits and enters above. The approach enter `009F3220`, the engage
  enter `009DB5E0` and the initial enter `009DB590` stay records.
- The two steps run through `SubTargetLeadBinding` and `SubTargetTangentBinding`.
- `brain+0AF0h` is a controller field. The pre-pass resets it, and `009F4DA0` reads it. It stays
  1.0 whenever the switch is off.
- New reconstructions in `src/ship_ai_attackmove_substates.cpp`: the two enters, the tangent exit
  and the world-box clamp.
- Header corrections in `include/bsp/ship_ai_attackmove_substates.hpp`:
  - `settings_weapon_release_delay_04d4` is SubmarineLostTime.
  - `unit_armament_speed_00a0` is the class Length.

**Substitutions, labelled in the code:**
- The pursuit's `0082ECB0(class, [unit+984h], vtable[38h](), 1.0)` answers
  `GameUnitsHost::unit_current_yaw_rate_00811940`.
  - That routine passes the body-axis speed and the unit's own turn efficiency.
  - It is the only door the units host has to `0082ECB0`.
  - The value feeds only the budget-mode turn count.
- `SubmarineLostTime` is the constant 30. This host's settings object holds only the ShipAvoidance
  block. A reader for the SubAttack block would sit in the Lua host, which cc9-lua7 holds.
- `009E2B60`'s immediate fire is a record with a count. The fire belongs to the gunnery host.
- The target's velocity is `neighbour_world_velocity` (the hull heading times `0092D730`), as for
  the sub attack.

### The OFF census (`local\ships7_sh2_{jm06,lomp06}.log`, this tree's build)

| mission | attacker | target | gate calls | image opens | with `brain+0B28h` | first open |
| --- | --- | --- | --- | --- | --- | --- |
| JM06 | USTroopTransport 01..04 | PlayerSub 03 (min y -39.4) | 145 each | 145 | 129 | 5.75 s |
| JM06 | Fletcher-class 08 | PlayerSub 01 (min y -13.2) | 145 | 145 | 107 | 5.75 s |
| JM06 | Fletcher-class 09 | PlayerSub 01 | 95 | 95 | 95 | 55.70 s |
| LOMP06 | Yugiri | Narwhal (min y -10.1) | 20 | 20 | 2 | 30.95 s |

Every gate call opens, because every target boat is at periscope depth or deeper. The two OFF
runs of this build are gameplay-identical (pair_diff exit 1).

### Predictions, written before any ON run

The pairs are `pair_export --commit <this commit> --flip kShipAiSubTargetSubStatesBound=true`
against this tree's OFF build, with reference i's launch lines.

**JM06 3200/3000: exit 3.**
- Each of the six attackers enters lead pursuit at its first open: 5.75 s, and 55.70 s for
  Fletcher 09.
- The troop transports and Fletcher 08 enter the tangent while their target is not visible. That
  covers about 16 and 38 of their calls. Fletcher 09 never enters it.
- No attacker returns to the approach, because every call opens.
- `lost_ends` is 0, because no invisible stretch reaches 35 s.
- The attackers close on the lead points, so their distances and positions move. The lead is the
  target plus 3.9 s of its velocity for PlayerSub 03 at -39 m, and 3 s for PlayerSub 01.
- `009E2B60` notices are counted only on a tangent entry within 200 m of the destination, or in
  the tangent's tail.
- Hits on PlayerSub 01 or 03 may change through the gunnery side's own depth-charge bot. That is
  not predicted.

**LOMP06 1200/1000: exit 3.**
- Yugiri enters lead pursuit at 30.95 s, and the tangent on most of its 18 later calls.
- `lost_ends` is 0, because the run ends 19 s after the first open.
- Yugiri's distance moves.

**USN01 3200/3000, USN02 9200/9000, USN04 4700/4500 and USN13 3200/3000: exit 0 or 1.**
- They have no submarine target, so there is no gate call.
- The new records, the approach and initial enters, appear only on the ON side.

**Verdict rule.**
- The mechanism is checked by three things:
  - the lead and tangent enters match the visible and invisible opens;
  - the steps run;
  - the four rows without a submarine stay identical.
- A mechanism failure keeps the switch OFF.

### Found on the way (added to the ranking)

- **The approach enter `009F3220` never runs in the host.**
  - In the image it runs on every attackmove's first selection and on every return to the
    approach.
  - It re-seeds the sixty ring records with sixty stream-1 draws, and runs `009F1BC0(0)`.
  - Its reach is 3 on every attackmove row (USN02 has 21 attackmove ships), and it shifts the shared
    stream-1 draws.
- **`ShipAiApproach::unit_depth_reference` (`unit+494h`) has a producer.**
  - `00956C20` writes `unit+494h`, the any-weapon max range (`kUnitOffAnyWeaponMaxRange`).
  - The host already holds it as `GameGunneryUnitRow::any_weapon_max_range`, yet the label says
    "no producer".
  - Its only use is the approach throttle at `brain+258h`, which has no reader in the recovered
    chain, so its reach is low.

### The pairs

- **OFF** is this tree's build: `local\ships7_off2_{jm06,lomp06}.log` at `46accdd3d`, and
  `local\ships7_off_{usn01,usn02,usn04,usn13}.log` at `24e2b2194`. The fix between the two touches
  only the ON path.
- **ON** is `pair_export --commit 46accdd3d --flip kShipAiSubTargetSubStatesBound=true` into
  `local\ships7_on2` (bsp_game.exe SHA-256 prefix `610D9346F379`). Its logs are
  `local\ships7_on2_<row>.log`.
- **A first ON export, at `24e2b2194`, failed its mechanism check.** The selector asks `007B6EE0`
  for its member on every kind-8 open, and the binding skipped `007B6EE0`'s early return when the
  machine already holds the member. So every open re-ran the exit and the enter: 676 lead enters on
  JM06 for six attackers. `46accdd3d` adds the early return. Only the pairs below count.

| row | pair_diff | reading |
| --- | --- | --- |
| JM06 3200/3000 | exit 3 | deaths 2 -> 1, hit records 284 -> 314, damage 4730.3 -> 4405.8, shots 352 -> 381, first hit 68.60 -> 62.40 s |
| LOMP06 1200/1000 | exit 3 | shots 6 -> 9; Yugiri's row moves |
| USN01 3200/3000 | exit 1 | gameplay identical |
| USN02 9200/9000 | exit 1 | gameplay identical |
| USN04 4700/4500 | exit 1 | gameplay identical |
| USN13 3200/3000 | exit 1 | gameplay identical |

The ON census:

| attacker | gate / open / visible | lead enters / steps | tangent enters / steps | notices | lost_ends |
| --- | --- | --- | --- | --- | --- |
| USTroopTransport 01..04 | 145 / 145 / 127 | 2 / 506 | 1 / 72 | 0 | 0 |
| Fletcher-class 08 | 128 / 128 / 76 | 3 / 304 | 2 / 210 | 0 | 6 |
| Fletcher-class 09 | 95 / 95 / 75 | 2 / 298 | 1 / 80 | 0 | 0 |
| Yugiri (LOMP06) | 20 / 20 / 2 | 1 / 8 | 1 / 69 | 1 | 0 |

**JM06's unit moves:**
- **US Cargo Transport 02 survives with 2382 health.** OFF has it sunk at 140.15 s, killed by
  Fletcher-class 09. In ON, Fletcher 09 fires 0 shots against 38, because it pursues PlayerSub 01
  instead of holding the approach's standoff.
- **USTroopTransport 02 takes 3599 damage against 1233** (health 402). USTroopTransport 01 closes
  to 169 m against 731.
- **Fletcher 08 takes no damage** (151 OFF), and its nearest approach opens from 48 to 107 m.
- **The Hospital Ship's row** reads health 1583 -> 421 and `sunk_at` 445 -> 1662. That column holds
  a value beside a non-zero health, so its meaning is not settled here; the numbers are quoted raw.

**Predictions:**
- **Held:**
  - one lead enter at each first open (5.75 s, and 55.70 s for Fletcher 09);
  - tangent enters only during invisible stretches, and none while the target is visible;
  - no return to the approach, because every call opened;
  - LOMP06's Yugiri in the tangent for most of its calls;
  - identity on USN01, USN02, USN04 and USN13.
- **Failed on spread:**
  - "Fletcher 09 never enters the tangent": it enters it once.
  - "`lost_ends` 0": Fletcher 08 gives up its command. Its target was visible on 76 of 128 calls
    ON, against 107 of 145 OFF, and one invisible stretch passed SubmarineLostTime + 5 s = 35 s.
    That is the image's give-up rule firing, not a wrong mechanism. Its six `0071E430` calls
    are one per tangent step while the director's `+30h` was not yet 2. Its gate calls then stop
    at 128.
  - The notices: Yugiri has one, which is not a mechanism fault.

**Verdict: `kShipAiSubTargetSubStatesBound` ON.** Every mechanism check held after the
`007B6EE0` fix. The misses are on spread, and the switch is the image's selector, enters, exits
and steps. By the brief's rule it flips, with the misses recorded.
- JM06's moves are this switch's own.
- The depth-charge fire in `009E2B60` is still a record. The attackers therefore reach their
  targets, but only the gunnery side's own bots fire at them.

**Open after this packet:**
- `009E2B60`'s immediate fire on the Function-8 guns needs a gunnery-host entry.
  - Proposed declaration: `bool GameGunneryHost::fire_function_guns_now_009e2b60(std::size_t
    unit_index, int function)`. It would call `vtable[1F0h]` for every depth-charge launcher
    (kind 24h) child whose `[+3F4h]+80h` equals `function`, and return whether any fired. Landed as
    main `f708c1eb2` and wired in section 9.
  - It is routed through the lead, because `src/game_hosts_gunnery.cpp` is cc9-gunnery7's.
- SubmarineLostTime is read from a constant. A Lua-host reader for the SubAttack block belongs to
  cc9-lua7's lease.
- The lead pursuit's `0082ECB0` inputs need a units-host door. Proposed declaration:
  `float GameUnitsHost::unit_class_yaw_rate_0082ecb0(std::size_t index, float rudder, float speed,
  float efficiency)`.
- The approach enter `009F3220` (see above) is next in this lane.

### Re-paired on a tree synced with main (`29e95bad6`)

The lead asked for the pairs to be run on a tree synced with main, citing that tree's own OFF logs.
Reference j is still being built.
- **The tree:** main merged into `agent/cc9-ships7` at `29e95bad6`.
- **OFF:** `pair_export --commit 29e95bad6 --flip kShipAiSubTargetSubStatesBound=false` into
  `local\ships7_offj` (SHA-256 prefix `661050E5FE85`), logs `local\ships7_offj_<row>.log`.
- **ON:** the tree's own build, logs `local\ships7_onj_<row>.log`.

The results are the same as the pairs above:
- JM06 and LOMP06 move (exit 3).
- JM06's figures and death row are unchanged from those pairs: deaths 2 -> 1, hit records
  284 -> 314, damage 4730.3 -> 4405.8, shots 352 -> 381, and US Cargo Transport 02 survives.
- USN01, USN02, USN04 and USN13 are gameplay-identical (exit 1).
- The per-attacker census is identical to the table above.

## 3. The party brain's replan flag (packet `cc9_party_replan_flag`, rank 5, `kAiPartyReplanFlagBound`)

Worker cc9-ships7, 2026-09-28. The switch was bound OFF first and is now ON. The pairs and the verdict
follow the predictions.

### The image

- **`00A15970`** (`BSP_AiPartyBrain_WantsImmediateThink`), called from `00A182C0` before each party's
  think timer is tested.
  - In effective game modes 4 to 7 it asks the one mode planner at `brain+10h..+1Ch` through
    `vtable[+30h]` (`00A15984..00A159E6`).
  - Every other mode calls `vtable[+30h]` on all four of `brain+0h..+0Ch` in order, with no short
    circuit (`00A159E8`, `00A15A0E`, `00A15A2A`, `00A15A40`). A null slot answers 0. It returns
    their OR (`00A15A46..00A15A6A`).
- **`00A18480`** (`BSP_AiPlanner_TakeReplanFlag`) is the `+30h` slot of those four planners
  (docs/AI_PLANNERS.md): `MOV AL,[ECX+2Ch]; MOV byte [ECX+2Ch],0; RET`.
- **The claim `00A22750`** (`__thiscall(planner)(group)`, `RET 4`):
  - It calls `00A2C600(group)` and discards the answer.
  - It then asks `00A1C8B0` (`BSP_AiPlanner_OwnsGroup`) whether the planner's own list at `+20h`
    already holds the group, and returns if it does.
  - Otherwise it pushes the group, stores the planner at `group+5654h`, registers the observer
    (`00694A60`), and sets `+2Ch = 1` at `00A227A9`.
  - It does not test `group+5654h` before the push.
- **So a claim makes the party think again on the next call to `00A182C0`,** instead of waiting for
  its 3 to 5 s timer. The immediate think also draws a new interval and reschedules.

### The host before this packet

- `AiGroups::brain_wants_immediate_think` answered false. Its comment read only the mode 4 to 7 arms
  and said no planner sets a replan request.
- `planner_claim_group` refuses a group that any planner holds. The image only skips a group the
  claiming planner already holds.

### The binding (`kAiPartyReplanFlagBound`, `src/game_hosts_ai.cpp`)

- `Planner::replan_002c` is set on the claim when the planner's own list lacks the group, which is
  `00A1C8B0`'s test.
- `brain_wants_immediate_think` reads and clears the flag on planners 0 to 3 and answers their OR,
  outside modes 4 to 7. The campaign runs in mode 0.
- The census is printed on both sides as `summary mission ai replan flag`: `sets` counts the
  claims that set the byte under the image's rule, `immediate` counts the true answers, and
  `foreign_claims` counts the claims of a group another planner held.

**Substitution, labelled:** the host still refuses a group another planner holds, and does not model
the image's transfer. `foreign_claims` is 0 on all six OFF rows below, so that case does not arise
on them.

### The OFF census (this tree at `88e8c713a` plus this packet, `local\ships7_rpoff_<row>.log`)

| row | party thinks | claims = sets | foreign claims |
| --- | --- | --- | --- |
| USN01 3200/3000 | 39 | 4 | 0 |
| USN02 9200/9000 | 112 | 7 | 0 |
| USN04 4700/4500 | 57 | 10 | 0 |
| USN13 3200/3000 | 37 | 20 | 0 |
| JM08 3200/3000 | 38 | 8 | 0 |
| JM06 3200/3000 | 38 | 4 | 0 |

### Predictions, written before any ON run

**Every row: exit 3.**
- `immediate` is between 1 and `sets` on each row. Several claims made in one think set flags that
  one query clears together.
- `thought` rises by about `immediate`.
- Each immediate think draws one more think interval from the AI stream and reschedules the party.
  So every later think time moves, and with it the planner orders and the combat.
- The first commands do not move: the first think is not preceded by a claim.

**Mechanism checks:**
- `immediate` > 0 on every row;
- `immediate` <= `sets`;
- `sets` and `foreign_claims` stay at the OFF values until the first immediate think, and
  `foreign_claims` stays 0.

A mechanism failure keeps the switch OFF.

### The pairs

- **OFF:** this tree's build at `1f4c80280`, logs `local\ships7_rpoff_<row>.log`.
- **ON:** `pair_export --commit 1f4c80280 --flip kAiPartyReplanFlagBound=true` into
  `local\ships7_rpon` (SHA-256 prefix `652BE91D967D`), logs `local\ships7_rpon_<row>.log`.

| row | pair_diff | thought OFF -> ON | sets | immediate | foreign | moves |
| --- | --- | --- | --- | --- | --- | --- |
| USN01 3200/3000 | exit 3 | 39 -> 40 | 4 | 2 | 0 | hit records 516 -> 466, shots 1534 -> 1425 |
| USN02 9200/9000 | exit 3 | 112 -> 115 | 7 | 1 | 0 | deaths 11 -> 12, hit records 1693 -> 3410, damage 54395.4 -> 57117.1, shots 2047 -> 2397 |
| USN04 4700/4500 | exit 3 | 57 -> 57 | 10 | 2 | 0 | hit records 749 -> 692, damage 13347.8 -> 11673.2; four aircraft deaths only OFF |
| USN13 3200/3000 | exit 1 | 37 -> 38 | 20 | 1 | 0 | gameplay identical |
| JM08 3200/3000 | exit 1 | 38 -> 41 | 8 | 2 | 0 | gameplay identical |
| JM06 3200/3000 | exit 3 | 38 -> 38 | 4 | 1 | 0 | hit records 314 -> 299, damage 4405.8 -> 4330.6 |

**USN02's death rows:**
- **Houston sinks at 313.30 s against 20.95 s.** Her killer changes from Yamakaze at 2432 m to
  Tokitsukaze at 4861 m.
- Yamakaze, Minegumo and Encounter die only in ON. John3 and Asagumo die only in OFF.
- The mission's end state is the same on both sides: `MissionPhase=1`, `EndMission=true`, and
  `MissionFailedRan=nil`.
- The OFF numbers are this tree's, not reference i's. The tree includes the submarine switch and
  main's later commits.

**Predictions:**
- **Held:**
  - `immediate` > 0 on every row and never above `sets` (1 or 2 against 4..20);
  - `foreign_claims` 0 on every row;
  - the moves on USN01, USN02, USN04 and JM06.
- **Failed on spread:**
  - USN13 and JM08 are gameplay-identical, where exit 3 was predicted. Their extra thinks
    (38 against 37, 41 against 38) issue nothing that changes the fight.
  - `thought` does not rise on USN04 or JM06. The earlier thinks move the later timer draws, and
    the count inside the window comes out equal.

**Verdict: `kAiPartyReplanFlagBound` ON.**
- The mechanism matched on all six rows. The misses are on spread.
- The switch is the image's query and the image's flag.
- **Flagged for the lead: USN02 moves a long way.** Houston lives about 290 s longer. This comes
  through the party's think timing and the shared AI stream it draws from. The next reference
  rebaseline will carry it.

## 4. The approach enter `009F3220` (packet `cc9_approach_enter_reseed`, read; binding waits on the file lease)

Worker cc9-ships7, 2026-09-28. The read is complete. The binding needs `src/game_hosts_ship_ai.cpp`,
which cc9-gunnery8 holds until 2026-09-29T00:40Z for `cc9_periscope_out`, so nothing is bound yet.

### The image

- **`009F3220`**, the approach member's `vtable[4]` (`00D21994+4h`), is `__thiscall(sub)`, `RET`.
  It calls `009F30F0` with `ECX = sub+8h`, the nested object, and then sets `sub+14B4h = 1.0f`
  (`00D7A24C`, `009F322B`).
- **`009F30F0-009F3212`** (`__thiscall(nested)`, `RET` at `009F3212`, INT3 from `009F3213`):
  - **The ring loop** (`009F30F6..009F3169`) runs sixty times, with `ESI` stepping `4Ch` from
    `nested+2Ch`, which is record `+28h`. For each record it:
    - zeroes `+18h..+3Ch`, which is the host's `ShipAiApproachSlotScore` except its byte;
    - clears the byte `+40h` (`blocked_40`);
    - calls `00BD2F10(ECX = 1, 0.0, [00CE3958] = 2.0f)` at `009F314E` and stores the draw in
      `+48h`, the ring probe's re-probe timer (`jitter_48`);
    - stores `[00CE3804] = 1000.0f` in `+44h`, the probe's clear distance (`reset_44`).

    This is `009E5530`'s first pass again (docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md), with the draw
    before the 1000 store.
  - **The traffic list.** `009F316B..009F31A3` frees every node of the list at `nested+14A4h` and
    zeroes its count at `+14A8h`. That is the host's `Controller::traffic`, the list at
    `nested+14A0h`.
  - **`009F31A7..009F31B5`:** `nested+11F4h = [00D7A260] = -1.0f`, the avoidance refresh timer. So
    the next pass refreshes the avoidance at once.
  - **`009F31BD`:** `009F1BC0(nested, 0.0f)`, the frame state with zero seconds. It writes
    `nested+11ECh`, the unit's heading, among its other outputs.
  - **`009F31C2..009F31ED`:**
    - `nested+11D8h = 0`, the retarget timer;
    - the byte `+11D6h` cleared;
    - `nested+11F8h` and `nested+120Ch` = `nested+11ECh`, so the selected bearing and the
      commanded heading restart from the current heading;
    - the byte `+1208h` cleared.
  - **`009F31F3..009F3207`:** `+1208h = unit->vtable[22Ch]()`. In both ship vtables read
    (`00CFC3D0`, the destroyer, and `00D09678`, the ship base), the slot `+22Ch` is `006DFDB0`:
    `XOR AL,AL; RET`. So the byte stays 0, which the host's `flag_1208` already holds.
- **When it runs.** Every switch to the approach member through `007B6EE0` runs it, and so does the
  attackmove state's own enter.
  - `009E86C0` (`00D219D0+4h`): `state+1500h = 0`, then the current member's `vtable[4]`, a
    tail jump.
  - `009E86E0` (`00D219D0+8h`): the current member's `vtable[8]`.
  - So each entry into the attackmove state, with the approach as the current member, re-seeds
    the ring, and zeroes the selector countdown so the selector runs on the first step.
- **The host today.**
  - The approach enter is a record (`ShipAiAttack::approach_enter_009f3220`, since
    section 2's binding).
  - The attackmove state's enter and exit are the generic records `ShipAiState::enter_vtable04` and
    `exit_vtable08`.
  - The construction pass's own sixty draws (`009E55A1`) are not modelled either:
    `ship_ai_attackmove_ring_slot_009e5530` stores 0.

### The random stream

`00BD2F10` with `ECX = 1` is stream 1. In this host every ship-AI stream-1 draw goes through
`GameGunneryHost::ship_ai_draw`:
- With `BSP_GUNNERY_RNG_STREAMS=1`, which every reference and pair run uses, that is the unit's own
  `ship_ai_torpedo` generator, keyed by unit index. The sixty draws per re-seed shift only that
  unit's later ship-AI draws: its torpedo response, its sub-attack switch and its tangent enter.
- Without the variable it is the shared generator, where they would shift every later draw in the
  process.

### The planned binding (`kApproachEnterReseedBound`, OFF)

- **`009F3220`** in `AttackMoveSelectorBinding::member_enter`:
  - `approach_scores[i] = {}`, `approach_ring[i].reset_44 = 1000.0f` and
    `approach_ring[i].jitter_48 = ship_ai_draw(unit, 0, 2)` for i = 0..59, in that order;
  - `traffic.clear()`;
  - `approach.avoid_refresh_11f4 = -1.0f`;
  - `ApproachUpdateBinding::frame_state_009f1bc0(0.0f)`;
  - `retarget_timer_11d8 = 0`, `flag_11d6 = false`,
    `selected_bearing_11f8 = commanded_heading_120c = unit_heading_11ec`, `flag_1208 = false`;
  - `substate_ring_timer_14b4 = 1.0f`.
- **`009E86C0` / `009E86E0`** at the host's state switch (`select_for_command`):
  - entering attackmove zeroes `selector.countdown_1500` and re-enters the current member;
  - leaving it exits the current member.
- **Census:** re-seeds per unit and their draws, printed on both sides.

### Predictions, written before any ON run

The rows with approach frames on reference i are USN02 (29287 frames), JM06 (4603) and LOMP06 (236).

**USN02 9200/9000: exit 3.**
- Each of its attackmove ships re-seeds once at its first selection, and again at each re-entry
  into attackmove.
- Its ring probes start from random phases in [0, 2) s instead of 0.
- Its first avoidance refresh comes one pass earlier.

**LOMP06 1200/1000: exit 3 or 1.** Its approach runs only 236 frames.

**JM06 3200/3000: exit 1.** Its six attackmove ships target submarines and go straight to lead
pursuit (section 2), so no approach member is entered.

**USN04 4700/4500 and USN01 3200/3000: exit 0 or 1.** Neither has approach frames.

**Mechanism check:** re-seeds > 0 on USN02, with 60 draws each; none on USN04 or USN01.

## 5. The party brain's engagement pass `00A179E0` (rank 8, read only, packet `cc9_engagement_pass_read`)

Worker cc9-ships7, 2026-09-28. **It is the AI party's power-up use, and it cannot be bound in
`src/game_hosts_ai.cpp` alone.**

### Why the earlier read stopped halfway

- docs/AI_PLANNERS.md "`00A179E0`, the brain's engagement pass" read only the pair-building half,
  `00A179E0..00A17D55`. Ghidra's body stops there: `ghidra disasm` lists 261 lines, ending at the
  second `00A16B60` call.
- The cause is the checked-iterator failure call `00BF6713` (`LIBCRT_unmatched_00bf6713`). Ghidra
  treats it as non-returning, so the decompiler dropped 66 "unreachable" blocks. Those are not EH
  funclet tails, as AI_PLANNERS assumed. They are the rest of the body.
- This read uses `disasm-raw 00A179E0 --length 1974`: 555 lines, `RET` at `00A18195`.

### The whole pass

- **`00A17A19..00A17A2F`:** `008EA0C0(ECX = [00F88C30], party = brain+20h, &vec)`.
  - `00F88C30` is the power-up manager (`PowerupConfigOwner`, docs for 008E9AF0 / 008ED9C0).
  - `008EA0C0` fills `vec` only when the byte `00E0C978` is set (`008EA0F7`).
  - That byte is `powerups_enabled`, which the single-player lobby branch forces to 1
    (`mission_lobby_settings.hpp`, 005E2FAB).
  - The entries come from the manager's per-party list at `manager + party*0Ch + 24h`.
- **`00A17A34..00A17D55`:** the pair build that AI_PLANNERS records. It pairs this party's groups
  against the enemy team's groups within 3000 units and within a factor of two in strength. Each
  pair is a `1Ch`-byte record, with its weight at `+18h`.
- **`00A17D5A..00A180A8`:** the choice. For each entry `e` of `vec`, with the power-up object at
  `[e+4]`:
  - `[obj+8] == 1` walks the pairs and their group members.
    - Where the object's `+84h` is 1: the candidate is the member entity when
      `008E35F0(member, obj)` accepts it. Its score is `00A0F680(pair.b, obj) * pair+18h`.
    - Where `+84h` is 2: the other group's members, through `008E35F0` and then `00A0F680` (`00A17EDE`, `00A17EFD`).
  - A second switch on the same word (`00A17F49..00A17FBC`) sends 1 and 2 to a member loop at `00A17FC3`, which uses `008E35F0` then `00A046C0`. Value 4 scores `00A04860(ECX = brain+20h, the party)`, and value 5 scores `00A04910(ECX = brain+24h)`.
  - **Coverage: partial.** The arm structure and the callees are read. The operands inside `00A17D5A..00A180A8` are not all traced, and none of the six callees is read.
  - It keeps the best score, the entry and the target (`[ESP+68h]`, `[ESP+34h]`, `[ESP+80h]`).
- **`00A180AD..00A180C9`:** when an entry won, `008EADA0(ECX = [00F88C30], entry, party, target)`.
  That is the use: once per party think, the best power-up is fired at the best target.
- **`00A180CE..00A18195`:** the frees of the two snapshots and of `vec` (`00BF65AC`), then `RET`.

### What the host does

- `AiParties::party_brain_plan_tail` records the call. Its label reads "contract: unread".
- The call count on reference i is 491, on all nine rows. The count is one per party think, and
  the host's think count changes with rank 5's flip, so reference j will differ.
- The power-up manager `00F88C30` has no model in this process. Its per-frame
  `PowerUps::pre_pass` (`008EAC80`) and `post_pass` (`00613760`) are records, 38500 calls on the
  reference i rows.
- So the host neither fills `vec` nor fires a power-up. The image fires one whenever a party's
  list holds a usable entry.

### Reach and binding

- **Reach:** 3 to 4 wherever an AI party holds power-ups. Whether a campaign mission gives the AI
  party any is set by the manager's list filler, which is not read here. That filler is the first
  question for a binding packet.
- **Binding:** it needs the power-up manager itself: the lists, `008EA0C0`, `008E35F0`,
  `00A0F680`, `00A046C0`, `00A04860`, `00A04910` and the use `008EADA0`. None of those sits in
  `src/game_hosts_ai.cpp`. The AI side is one host call, `party_brain_plan_tail`, which already has
  its seam in `ai_group_think`.
- **The rank stays 8, with its reach column now known.** A power-up subsystem packet is the
  prerequisite. It is the lead's to site.

## 6. Handoff (cc9-ships7, 2026-09-28, at about 76% context)

**State.**
- Landed on main:
  - the ranking, as `d5238d1cb`;
  - the submarine-target sub-states, ON, as `bed195cbf` and `d04ccffcd`;
  - the party replan flag, ON, as `01e85c9b5`.
- Unlanded on `agent/cc9-ships7`:
  - `84814f2f7`, the reseed read (section 4);
  - `458c405ff`, the main merge;
  - `0fc65c31b`, the engagement-pass read (section 5);
  - this handoff.
- No lease is held.

**Queued, in the lead's order.** All but the last need `src/game_hosts_ship_ai.cpp`, which
cc9-gunnery8 holds for `cc9_periscope_out`. The lead sends "ship_ai free" when it releases.

1. **`cc9_approach_enter_reseed`.**
   - Section 4 has the read, the stream answer and the planned binding. Its predictions are
     written.
   - Bind it OFF as `kApproachEnterReseedBound`:
     - the approach-member branch of `AttackMoveSelectorBinding::member_enter`;
     - the attackmove state's enter `009E86C0` and exit `009E86E0`, at the host's state switch in
       `select_for_command`, where the generic `ShipAiState::enter_vtable04` / `exit_vtable08`
       records sit.
   - The frame-state call is `ApproachUpdateBinding(owner, ctl, row, index).frame_state_009f1bc0(0.0f)`.
     It is defined after the selector binding, so the reseed body goes in a free function
     defined after that class.
   - Pair JM06, USN02 and USN04, plus USN01 for identity.
2. **Wire the two entry points from main `c89abeb5a`** into the sub-target bindings (section 2):
   - `GameUnitsHost::unit_class_yaw_rate_0082ecb0(index, rudder, speed, 1.0f)` replaces
     `SubTargetLeadBinding::yaw_rate_from_rudder_0082ecb0`'s 00811940 stand-in. Its inputs are
     `unit_ordered_rudder_0984`, which is `[unit+984h]` and returns 0 today (fix that), and
     `unit_forward_speed_vtable_0038`.
   - `GameMissionLuaHost::sub_attack_submarine_lost_time_04d4()` replaces
     `kSubTargetSubmarineLostTime`. The ship-AI host reaches that Lua host through
     `settings_owner`.
   - Keep the switch ON, and re-pair JM06 once.
3. **Rank 2, the follower's station point.** Section 1 has the evidence. The binding is small:
   - the leader's yaw rate is `owner_.units.unit_current_yaw_rate_00811940(leader_)`;
   - the zone push is the ring probe's `zones.group_for_layer` / `zones.offset` pair, with
     margin 20.
4. **Rank 10, `00A0F970`** (`BSP_AiGroup_TargetValueAgainstGroup`), in `src/game_hosts_ai.cpp`,
   which is unleased.
   - It is already read in docs/PLANNER_KATE_TARGETING.md section 3: `__fastcall` with ECX the
     attacker group and EDX the target group, five stack arguments, `RET 14h`, over `00A0C650`
     and `00A07E40`.
   - The host's `candidate_base_weight` weighs by member count instead.
   - The lead numbers it rank 9. In section 1 rank 9 is `00A1443D`, and this is rank 10.

5. **Rank 9, `00A1443D`, the close attack's busy member**, which the lead queued on 2026-09-28. In `src/game_hosts_ai.cpp`, `AiCommand::close_controller_busy` answers false with "contract: unread". The site is `[member+538h]->vtable[2Ch]` inside `00A13B60` (`BSP_AiCommand_CloseAttackTargetPass`), 5991 calls on reference i. The same slot decides the Cargo capture weight in `capture_weight_00a03510`. Read the slot's target in the class descriptor's vtable first; `[unit+538h]` is the class descriptor (docs/AI_BRAIN_PLAYER_EXEMPTION.md). Then bind OFF, predict, pair and flip by verdict.

**Not to redo.**
- Section 1's census scripts, `local\ships7_census.py` and `local\ships7_sites.py`, point at
  cc9-gunnery7's reference i logs. For reference j, change their `root` and `rb9` prefix.
- Section 2's OFF shadow counters exist on both sides. Section 3's `summary mission ai replan
  flag` line is its census.
- Launch and wait helpers: `local\ships7_run.ps1` (`-Exe`, `-Prefix`, rows
  `tag:MISSION:frames:mission_frames`) and `local\ships7_wait.ps1`.

**Traps met in this lane.**
- The selector asks `007B6EE0` for its member on every call. Any binding of member enters must
  keep its early return.
- Ghidra's bodies of routines that call `00BF6713` are truncated. For them, use `disasm-raw` with
  the full length.
- `neighbour_settings()` fills only the ShipAvoidance block of the settings object.

## 7. The planner candidate's group target value `00A0F970` (packet `cc9_planner_group_target_value`, rank 10, `kPlannerGroupTargetValueBound`)

Worker cc9-ships8, 2026-09-28. Every name is a hypothesis. The switch was committed OFF at
`743fcd622`, with the predictions below written before any ON run.

### The image

| Routine | ABI | What it does | Coverage |
| --- | --- | --- | --- |
| `00A0F970` `BSP_AiGroup_TargetValueAgainstGroup` | `__fastcall(ECX = attacker group, EDX = target group, a1, a2, a3, a4, a5)`, `RET 14h`, result in ST0 | 0 when either group's `+5644h` count is 0 (`00A0F98B`, `00A0F99A`). Otherwise it builds both groups' records with `00A07E40` and returns `00A0C650(ECX = attacker records, EDX = target records, a1..a5)`, the five arguments passed through in order (`00A0F9DF`-`00A0FA06`) | complete |
| `00A07E40` | `__fastcall(ECX = group, EDX = out vector)`, `RET 4` | walks the member list at `group+5640h` (node `+8h` is the member) and pushes one `00A04560(member, 1)` record per member, in list order | complete |
| `00A04560` `BSP_Ai_EntityRecordBuild` | `__fastcall(ECX = out, EDX = entity)`, `RET 4` | docs/AI_TARGET_WEIGHT_TERMS.md term 3 has the record. One correction: `record+10h` is not always 0. `00A04619`-`00A0464E` set it to `[X+C54h]` when `007B9140(X, 1)` answers true, where X is `[entity+3D0h]` for kind 18h and the entity for kind 0Fh. A ship record's `+10h` is 0 | complete |
| `00A04240` | `__thiscall(entity)`, float | `1.0` [`00D7A24C`] times the entry for the entity in the hint-weight map at `00F8A740`, times the entry in `00F8A750 + [00E0E344]*0Ch`. Only `00A07F60` / `00A07F80` write them, the `SetHintWeight` native (docs/LUA_BINDING_AI.md). In this installation only `scripts/missions/multi/competitive*.lua` call it, so the factor is 1.0 on every reference row | complete |
| `00A0C650` `BSP_AiGroup_ComposeAttackValue` | `__fastcall(ECX = attacker records, EDX = target records, a1 byte, a2 float, a3 debug text, a4 byte, a5 float)`, `RET 14h`; body `00A0C650`-`00A0D1C4`, checked against the INT3 run | two float vectors sized to the two counts, filled with 0 (`004A8F10` with `FLDZ`). For every attacker `i` (outer) and target `j` (inner): `v = 00A0C3C0(ECX = attacker i, EDX = target j, a1, a2, text, flag)`, where the flag is 1 unless `a4` is set and `i` is not the last attacker. `v` is scaled by `a5` only when the flag is 0. `attack sum += v`, and both vectors keep their running max. Then `maxes = sum of the attacker vector` (`00A0CC82`-`00A0CC9B`), `base = (+218h x attack sum + maxes) / +214h` (`00A0CDC6`-`00A0CDE3`), speed bonus `min(+22Ch x 00A07C10(), +228h x base)`, and three penalties. **`00A0CE71 CMP byte [EBP+8],0` zeroes all three penalties when `a1` is 0.** The result is `max(0, base + bonus - penalties)` | complete for the planner path; the penalty counts (`00A0CCB8`-`00A0CDBB`) are read but not bound, because they are zeroed here |
| `00A0C3C0` `BSP_AiEntityRecord_PairAttackValue` | `__fastcall(ECX = attacker record, EDX = target record, a1, distance, text, flag)`, `RET 10h` | `base = 00A0C330(...)`. If the attacker record's `+0h` class answers `vtable[+18h](6)` (a ship class): a negative distance is replaced by `00414C60` over the two records' pose deltas when both `+0Ch` bytes are set (0 otherwise); `d -= +D8h`; if `d > 0`, `t = d / class+500h` and `base *= 00419010(+DCh, +E8h, +E0h, +E4h, t)`. The plane class (`vtable[+18h](0Fh)`) arm is the same over `+ECh`, `class+188h` and `+F0h, +FCh, +F4h, +F8h` (`00A0C5BD`-`00A0C609`). Anything else returns `base` | complete |
| `00A0C330` `BSP_AiEntityRecord_PairBaseValue` | `__fastcall(ECX = attacker record, EDX = target record, text, flag)`, `RET 8` | `w = 00A08460(ECX = a+0h, EDX = a+10h, t+0h, t+1Ch)`; 0 when `a+1Ch` is set and `[a+0h]->vtable[+18h](1Ch)` answers true; `rnd = a+18h x t+18h` when the flag is set, else 1.0; returns `rnd x w x t+14h` | complete |

**The planner's call.** `00A1CC4B`-`00A1CC65` pushes `a1 = 0`, `a2 = -1.0` [`00D7A260`], `a3 = 0`,
`a4 = 0`, `a5 = 1.0`, with `ECX = EBX` (the planner's group) and `EDX = ESI` (the candidate). So on
this path: no debug text, the flag is 1 for every pair (rnd applies, `a5` does not), the distance
is measured from the records, and **the penalties are zero**.

**The authored values.** This installation's `scripts/datatables/highlvlaiglobals.lua` (mtime
2024-07-13, untouched bulk) authors the same values in all seven mode tables:
`ComposeGroup_ReferenceWeight` 5.0, `ComposeGroup_AttackSumMul` 0.33, both speed-bonus keys 0,
`ShipDistWeight_AriveDist` 3000, `ShipDistWeight_TravelTime` {60, 300} and `ShipDistWeight_WeightMul`
{1.0, 0.1}. `PlaneDistWeight_WeightMul` is {1.0, 1.0} everywhere, so a plane attacker's multiplier
is 1.0 at any range. `ValueRandomMul` is {0.95, 1.05} in the three IslandCapture tables and
{0.85, 1.1} in the other four.

So for the planner the value is **`(0.33 x sum over every pair + sum over attackers of the best
pair) / 5`**. A pair is `00A08460(attacker, target) x spread(a) x spread(t) x class weight(t) x
ship-travel multiplier`, and the multiplier falls from 1.0 when the ship arrives within 60 s to
0.1 at 300 s, measured to 3 km short of the target.

### The binding (`kPlannerGroupTargetValueBound`, `src/game_hosts_ai.cpp`)

- `AiPlannerHost::candidate_base_weight` now takes the planner's group as well as the candidate
  (`include/bsp/ai_planners.hpp`, `src/ai_planners.cpp` at `00A1CC65`).
- `group_target_value_00a0f970`, `group_value_record_00a04560` and `group_value_pair_00a0c3c0`
  project the routines above. A record's unit is `proxy(member)`, so a squadron answers through
  its flight leader.
- `00A08460` runs through the same `AiWeightModelBinding` the close-attack weight uses, when both
  weapon-facts rows are complete; otherwise the pair takes the identity 1.0 (labelled, counted as
  `stand_in_pairs`).
- **Labelled stand-ins.**
  - The spread's argument is the entity pointer modulo 79. This process has no stable entity
    addresses, so every record takes the midpoint 39. A constant spread scales every candidate
    alike and moves no pick; the per-entity spread of up to 10% is lost.
  - The class query `[record+0h]->vtable[+18h]` is answered from the unit's own kind: a squadron or
    kind 0Fh is a plane class, kind 6 a ship class.
  - The `a+1Ch` / `vtable[+18h](1Ch)` zeroing is unread, as at `00A0F859`, and never fires.
  - The authored tuning values are constants in the binding, because `AiTuningBlock` loads only
    the 33-key subset.
- **OFF** keeps the population stand-in and still computes the value, for the census line
  `summary mission ai group target value` and up to 400 sample lines
  `ai group target value own_lead=... value=... population=... leader_dist=... range=...`. The
  sample lines carry the planner's own range factor, so `local\ships8_gtv.py <log>` replays each
  planner round both ways, with the sticky 2.0 on each side's own previous pick.

### The OFF census (`local\ships8_off_<row>.log`, this tree at `743fcd622`)

| Row | calls | model pairs | stand-in pairs | rounds where the replayed pick differs |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | 551 | 10740 | 0 | 0 of 400 sampled. Every sampled round has one candidate |
| USN02 9200/9000 | 2316 | 11177 | 0 | 48 of 58 sampled rounds |
| JM06 3200/3000 | 296 | 3626 | 0 | 74 of 148 |
| LOMP06 1200/1000 | 130 | 650 | 0 | 13 of 13 |
| USN01 3200/3000 | 0 | 0 | 0 | none; the planner never scores |

`00A08460` runs for every pair on all four rows, so the stand-in 1.0 is never used.

### Predictions, written before any ON run

**USN02 9200/9000: exit 3.**
- OFF sends all seven ABDA groups at Haguro (4 members).
- ON keeps DeRuyter on Haguro. It sends Houston, John1, John2 and John3 at Yamakaze, and Exeter,
  Encounter and Witte at Kawakaze.
- The first `ai diag order_attack` lines change their `target_leader` accordingly.

**JM06 3200/3000: exit 3.**
- OFF sends Fletcher-class 08 (9 members) and Fletcher-class 09 (3) at PlayerSub 01.
- ON sends both at the one-member "Static Mavis, Crashed 01" group, which scores 0.2945 against
  0.2809 and 0.1425 against 0.1192. The sticky then holds them there.
- The Narwhal-class Submarine 01 and PBY Catalina 01 groups keep PlayerSub 01.
- **Knife-edge.** The margin is 5% and 20%, inside the image's per-entity spread. On the image the
  pick could go either way. A wreck outscoring the submarine group is what the formula gives for
  these inputs, not a judgement that the image does it.

**LOMP06 1200/1000: exit 1 or 3.**
- OFF orders the Narwhal group at "Storage - Raktar03 01" (27 members).
- ON orders it at Yugiri: 0.0312 against the storage group's 0.0210, both at range 1.0.
- The Narwhal is the player's controlled unit, so the order may not move it.

**USN04 4700/4500: exit 1.** Every sampled round has a single candidate, so no pick can change.
Only the host-method line for `candidate_base_weight` changes from UNIMPLEMENTED to concrete.

**USN01 3200/3000: exit 0.** The planner never scores a candidate.

**Mechanism check:** on the ON logs the census line reads `bound=1`, with the same `calls` and
`model_pairs` as OFF up to the first moved order; and the first `order_attack` of each group
matches the ON column above.

### The pairs (OFF `local\ships8_off_<row>.log`, the tree's build; ON `local\ships8_on_<row>.log`, `tools/pair_export.py --commit cede2e73f --flip kPlannerGroupTargetValueBound=true --out local\ships8_gtv_on`, SHA-256 prefix `B26187F20785`)

| Row | pair_diff | Predicted | What moved |
| --- | --- | --- | --- |
| USN02 9200/9000 | 3 | 3 | The orders are exactly the predicted ones: DeRuyter at Haguro; Houston, John1 and John3 at Yamakaze; Exeter, Encounter and Witte at Kawakaze. Deaths 12 to 11, with seven death rows only OFF and six only ON. Damage 57117.1 to 45311.4. **Houston sinks at 20.55 s instead of 313.30 s**, and the mission fails at 29.75 s instead of 34.70 s |
| JM06 3200/3000 | 3 | 3 | Fletcher-class 08 and Fletcher-class 09 are ordered at "Static Mavis, Crashed 01", and the Narwhal-class and PBY groups keep PlayerSub 01, as predicted. Deaths identical at 1. Hits 299 to 392, damage 4330.6 to 6253.3, shots 380 to 500 |
| LOMP06 1200/1000 | 3 | 1 or 3 | The Narwhal is ordered at Yugiri instead of the storage group, as predicted, and the idle player's Narwhal acts on it. Ryujin Maru sinks at 36.90 s to a blast from Yugiri (`killer_cat=7`, `killer_blast=1`, range 139 m). Before, nothing died |
| USN04 4700/4500 | 1 | 1 | Only the census line and `candidate_base_weight` from UNIMPLEMENTED to concrete |
| USN01 3200/3000 | 1 | 0 | Only the census line, whose `bound=` field changes. The miss is this packet's own census line, not a behaviour |

**Mechanism check: passed.** On every ON log the census reads `bound=1` and `stand_in_pairs=0`.
Each group's first `order_attack` matches the ON column of the predictions.

**Verdict: ON** (`kPlannerGroupTargetValueBound = true`). The three moved rows move in the
predicted direction, through the predicted orders.

**What to watch.**
- **JM06.** The two Fletcher groups chase a crashed flying-boat wreck (kind 1Bh) instead of the
  player's submarines. Three things decide this, none of them the binding's arithmetic:
  - the wreck is in an enemy group at all (the AI group seed);
  - `00A08460`'s answer for ship-against-wreck (0.123 per pair) against ship-against-submarine
    (0.052 per pair);
  - the per-entity spread, which this host flattens. The margin is 5% for Fletcher-class 08.
- **USN02.** Houston's early loss is back: the party replan flag had moved it to 313.30 s
  (section 3). It is now the planner splitting the ABDA groups across three targets.

The rank-10 row of section 1 is closed by this packet.

## 8. The approach enter re-seed, bound (packet `cc9_approach_enter_reseed`, `kShipAiApproachEnterReseedBound`)

Worker cc9-ships8, 2026-09-28. Section 4 is the read; this section is the binding and its pairs. The
switch was committed OFF at `93ca747a5`, on a tree that holds main `f708c1eb2` and this worker's
rank-10 flip (section 7). The predictions below were written before any ON run.

### The binding

- **`009F3220`** is `ship_ai_approach_enter_009f3220`, called from the approach branch of
  `AttackMoveSelectorBinding::member_enter`. It runs, in the image's order:
  - the ring is built first when the host has not yet built it, so the lazy build cannot undo the
    re-seed;
  - sixty records: `approach_scores[i]` cleared (+18h..+3Ch and the byte +40h), then
    `approach_ring[i].jitter_48` = the unit's stream-1 draw in [0, 2), then `reset_44` = 1000;
  - `traffic.clear()`, `avoid_refresh_11f4 = -1`;
  - `ApproachUpdateBinding::frame_state_009f1bc0(0.0f)`;
  - `retarget_timer_11d8 = 0`, `flag_11d6 = false`, `selected_bearing_11f8` and
    `commanded_heading_120c` set to `unit_heading_11ec`, `flag_1208 = false`;
  - `substate_ring_timer_14b4 = 1.0f`.
- **`009E86C0`** (the state enter) runs at the state switch when the incoming state is attackmove.
  It sets `selector.countdown_1500 = 0` and enters the current member. The constructor's member is
  the initial one, which this host holds as 0; `member_enter` answers it with the initial record.
- **`009E86E0`** (the state exit) runs when the outgoing state is attackmove. It exits the current
  member; only the tangent's exit `009DB7D0` has a body.
- **The census.** `summary mission ship ai approach enter` prints the approach member enters, the
  state enters and exits (both sides), and the re-seeds and their draws (ON only).

### The OFF census (`local\ships8_r0_<row>.log`, this tree's build at `93ca747a5`)

| Row | approach member enters | attackmove state enters / exits | member records on OFF |
| --- | --- | --- | --- |
| USN02 9200/9000 | 28 | 44 / 21 | no lead-pursuit or tangent member |
| JM06 3200/3000 | 0 | 22 / 16 | lead pursuit entered 12 times, the tangent entered and exited 6 times |
| LOMP06 1200/1000 | 0 | 1 / 0 | none |
| USN01 3200/3000 | 3 | 5 / 2 | none |
| USN04 4700/4500 | 0 | 0 / 0 | none |

Section 4 expected USN01 to be an identity row. On this base it has three approach enters.

### Predictions, written before any ON run

**USN02: exit 3.**
- There will be at least 28 re-seeds, each with 60 draws.
- More re-seeds come from the state enters whose current member is the approach, so the re-seeds
  can exceed 28, up to 28 plus 44.
- The ring probes start at random phases, and each re-entered ship's first avoidance refresh
  comes at once.

**JM06: exit 1 or 3.**
- No re-seed, because no approach member is entered.
- Each of the 16 state exits now runs the current member's exit. The tangent's exit is the only
  one with a body.
- Each of the 22 state enters re-enters the current member, a lead pursuit or tangent enter.
  The tangent enter draws from the unit's stream.
- So JM06 moves only if a ship leaves attackmove while it holds the tangent or lead pursuit, and
  comes back to it.

**LOMP06: exit 1.**
- One state enter, whose current member is the initial one: a record and the countdown set to 0,
  where the constructor had seeded it negative.
- The census line changes; nothing else does.

**USN01: exit 3 or 1.**
- It has three approach enters, and up to five re-entries, so there are three to eight re-seeds.
- Whether a re-seeded ring changes an outcome inside 3000 frames is open.

**USN04: exit 1.** Nothing is entered or left. Only the census line's `bound=` changes.

**Mechanism check:** on ON, `reseeds` equals the number of approach enters, direct plus state
re-entries, and `draws = 60 x reseeds`.

## 9. The sub-target entry points (packet `cc9_sub_target_entry_points`, `kShipAiSubTargetEntryPointsBound`)

Worker cc9-ships8, 2026-09-28. The switch was committed OFF at `ce6a0d253`. The OFF base for this
pair is this tree at `dd489d6d3`, logs `local\ships8_b0_<row>.log`. `kShipAiSubTargetSubStatesBound`
stays ON. The kind-24h filter of `009E2B60` is read as the **depth-charge launcher** filter: class
27h `MDepthChargeLauncher` is a kind 24h, and its `vtable[1F0h]` is `006FDC50`, which is
`vtable[1E8h](1)`, the latch.

### The three stand-ins and what replaces them

| Site | Before (OFF) | ON |
| --- | --- | --- |
| `009E2A6A` / `009E2A97`, the lead pursuit's yaw rate `0082ECB0(class, [unit+984h], vtable[38h](), 1.0)` | `00811940`'s current yaw rate. Its accessor answers 0 for every unit, because its binding's forward speed is never set: the units-host fix was sent to the lead. The rudder input was 0 | `GameUnitsHost::unit_class_yaw_rate_0082ecb0(unit, rudder, forward speed, 1.0)`. The rudder is `unit+984h` as the unit's row holds it (`GameUnitRow::ordered_rudder`, from `refresh_row`; labelled, because that row copy is the motion's `to_turn`) |
| `009F369F` / `009F36E1`, `SubAttack.SubmarineLostTime` (`settings+4D4h`) | the constant 30 | `GameMissionLuaHost::sub_attack_submarine_lost_time_04d4()`, the loaded `ShipGlobals` value, 0 when absent |
| `009E2B60` from the tangent step and the tangent enter (`009E2BD2`) | a counted record | `GameGunneryHost::fire_function_guns_now_009e2b60(unit, 8)`: the unit's Function-8 gun rows marked to fire at their next pass. The census adds `fires=` to each `sub target` line |

### The OFF census (`local\ships8_b0_<row>.log`)

| Row | sub-target brains | lead enters / steps | tangent enters / steps | 009E2B60 calls |
| --- | --- | --- | --- | --- |
| JM06 | four transports, Fletcher-class 08 and 09 | the transports 2 / 508 each; Fletcher-class 08 3 / 185; Fletcher-class 09 1 / 363 | the transports 1 / 80 each; Fletcher-class 08 2 / 236 | 1, Fletcher-class 08 |
| LOMP06 | Yugiri, on the Narwhal | 1 / 8 | 1 / 69 | 1 |
| USN02, USN04, USN01 | none | - | - | - |

### Predictions, written before any ON run

**JM06: exit 3.**
- The lead pursuit now turns with a non-zero yaw rate from the ordered rudder, where it had 0.
  Every lead step of the six brains moves.
- The lost time stays 30 if `ShipGlobals` is loaded, so Fletcher-class 08's `lost_ends=16` holds
  unless the path moves first.
- Fletcher-class 08's one `009E2B60` call marks its depth-charge launchers (`fires=1`, if its rows
  carry Function 8).
- **Deaths can move.** A depth-charge pattern near PlayerSub 01 can now damage it, where before
  nothing fired from this path.

**LOMP06: exit 3.**
- Yugiri's 8 lead steps move.
- Its one `009E2B60` call fires its depth charges at the Narwhal (`fires=1`). The Narwhal can take
  damage, and the deaths can move from 0.

**USN02, USN04, USN01: exit 0.** No brain reaches the sub-target sub-states, and no census line
changes.

## 10. The follower's station point (packet `cc9_follow_station_point`, rank 2, `kShipFollowStationPointBound`)

Worker cc9-ships8, 2026-09-28. The switch was committed OFF at `dd489d6d3`. Section 1's rank-2 row
is the evidence. The contract is `bsp::ShipAiFollowFormationPointHost`
(`include/bsp/ship_ai_formation.hpp`), and the projection is `src/ship_ai_formation.cpp`.

### The binding (`FollowFormationPointBinding`, `src/game_hosts_ship_ai.cpp`)

- **`009DF41A`, the zone set.** The follower's `vtable[218h]` is `006DFD90`:
  `ECX = [unit+538h]`, then `0082ADA0(0)` = `004120D0(manager, [class+560h])`. Bound as
  `zones.group_for_layer(leaf_tuning.array[0])`, exactly as the ring probe binds `009E66EB`. An
  unready runtime keeps 0.
- **`009DF432` / `009DF4C5`, the two pushes.** `00417B10(ECX = zone set, &out, &in, 20.0f, 1)` is
  bound as `zones.offset(set, point, 20, true)`. A zero set returns the point.
- **`009DF607`, the leader yaw rate.** `00811940` with `ECX = the leader`, `RET 0`, so it reads no
  argument. It is bound to `GameUnitsHost::unit_current_yaw_rate_00811940(leader)`.
  - **That accessor answers 0 for every unit** (section 9; the units-host fix is with the lead).
  - So on this base the yaw-rate half of the binding is inert. The OFF census counts non-zero
    answers (`leader_turning=`), and it reads 0 on every row.
- **The census.** `summary mission ship ai follow station zone_sets= pushes= moved= leader_turning=
  bound=`.

### The OFF census (`local\ships8_b0_<row>.log`)

| Row | `00417B10` push calls | `00811940` non-zero answers | ring probe moved starts, for comparison |
| --- | --- | --- | --- |
| USN04 | 16372 | 0 | 0 of 0 |
| USN02 | 4002 | 0 | 0 of 185700 |
| LOMP06 | 5744 | 0 | 0 of 0 |
| JM06 | 1130 | 0 | 0 of 0 |
| USN01 | 52 | 0 | 0 of 11760 |

### Predictions, written before any ON run

- **The yaw rate:** no effect on any row until the accessor is fixed.
- **The pushes:** a station point moves only if it lies inside an avoid zone's outline plus 20 m.
  Formation stations sit in open water, and the ring probe, with a margin of 3, moved no start on
  any row.
- **USN04, USN02, JM06, USN01: exit 1**, with `moved=0`. Only the census line and host-method
  statuses change.
- **LOMP06: exit 1 or 3.** Its followers work near the harbour's storage and PT hangar groups, so
  some pushes may move a point.
- **Mechanism check:** `zone_sets` > 0 wherever the runtime is ready, and `pushes` equals twice
  `zone_sets`.

### Section 8: the pairs (OFF `local\ships8_b0_<row>.log`, this tree at `dd489d6d3`; ON `local\ships8_rs1_<row>.log`, `pair_export --commit dd489d6d3 --flip kShipAiApproachEnterReseedBound=true --out local\ships8_rs_on`, SHA-256 prefix `ADA2E003FB61`)

The first ON attempt, at 11:10, died at renderer init with exit code 4 (`hr=0x8876086a`, session 1
on rdp-tcp) and was discarded. These are the runs after the session recovered.

| Row | pair_diff | Predicted | Census ON | What moved |
| --- | --- | --- | --- | --- |
| USN02 | 3 | 3 | approach enters 43 = re-seeds 43, draws 2580; state enters 43 / exits 21 | Death rows: Asagumo only OFF, John2 only ON; still 11 deaths. Damage 45311.4 to 43307.6; Kortenaer ends at 1809 health instead of 250. The mission still fails at 29.75 s |
| JM06 | 3 | 1 or 3 | re-seeds 0; state enters 7 / exits 1, against 22 / 16 OFF | The state exits and re-enters of lead pursuit and tangent change the paths. Deaths identical at 1; hits 392 to 518, damage 6253.3 to 5080.3 |
| LOMP06 | 1 | 1 | one state enter, re-seeds 0 | the census line only |
| USN01 | 1 | 3 or 1 | re-seeds 5, draws 300 | gameplay identical: the re-seeded rings change no outcome in 3000 frames |
| USN04 | 1 | 1 | nothing entered | the census line only |

**Mechanism check: passed.** On every row `reseeds` equals the approach enters, and `draws` is 60
times `reseeds`.

**Verdict: ON.**

### Section 9: the pairs (OFF `local\ships8_b0_<row>.log`; ON `local\ships8_ep1_<row>.log`, `pair_export --commit dd489d6d3 --flip kShipAiSubTargetEntryPointsBound=true --out local\ships8_ep_on`, SHA-256 prefix `6551844E47F3`)

| Row | pair_diff | Predicted | What moved |
| --- | --- | --- | --- |
| JM06 | 3 | 3 | Fletcher-class 08's one `009E2B60` call marks two Function-8 guns (`FireFunctionGunsNow: unit=18 function=8 marked=2`), and its shots go from 15 to 16. Hits, damage, deaths and every path are identical |
| LOMP06 | 3 | 3 | Yugiri's one call marks two guns (`unit=183 ... marked=2`), and its shots go from 9 to 10. Deaths identical at 1 (Ryujin Maru). The Narwhal takes no damage |
| USN01 | 0 | 0 | none |

**The lead pursuit's yaw rate did not move JM06, and the prediction was wrong about why it would.**
- `009E2A6A..009E2A97` runs only in the budget arm (`state+0Ch` set). It adds `|yaw x dt|` to
  the turn budget and leaves budget mode after a full turn, 2 pi [`00CE3828`].
- A shadow census, `yaw_max=` on the `sub target` lines (`b09ae784b`, `local\ships8_b1_jm06.log`),
  shows the class yaw rate is not zero. The largest values are 0.0433 to 0.0435 rad/s on the four
  transports, 0.0310 on Fletcher-class 08 and 0.0281 on Fletcher-class 09.
- About 430 budget steps of 0.05 s each add up to at most about 0.9 rad, short of 2 pi. So no
  brain leaves budget mode inside 3000 frames, either way.
- The binding is live; it acts only on a pursuit that lasts longer.

`SubmarineLostTime`: Fletcher-class 08's `lost_ends=16` is unchanged, which is consistent with the
loaded value being the 30 this installation authors.

**Mechanism check: passed** (`fires=1` on the two brains that call `009E2B60`). **Verdict: ON.**

### Section 10: the pairs (OFF `local\ships8_b0_<row>.log`; ON `local\ships8_fs1_<row>.log`, `pair_export --commit dd489d6d3 --flip kShipFollowStationPointBound=true --out local\ships8_fs_on`, SHA-256 prefix `BD59DC1078C8`)

| Row | pair_diff | Predicted | Census ON |
| --- | --- | --- | --- |
| USN04 | 1 | 1 | zone_sets 8186, pushes 16372, moved 0 |
| USN02 | 1 | 1 | 2001 / 4002 / 0 |
| JM06 | 1 | 1 | 565 / 1130 / 0 |
| LOMP06 | 1 | 1 or 3 | 2872 / 5744 / 0 |
| USN01 | 1 | 1 | 26 / 52 / 0 |

**Mechanism check: passed.**
- Every follower resolves its zone set, and every row has `pushes = 2 x zone_sets`.
- No station point lies within 20 m of an avoid zone on any row, so no push moves a point.
- `leader_turning=0` everywhere. That is the units-host accessor fault (section 10's binding), not
  this binding.

**Verdict: ON.**
- The zone half is faithful and identity on the five rows.
- The yaw-rate half becomes live when `GameUnitsHost::unit_current_yaw_rate_00811940` sets its
  binding's forward speed. The one-line fix is with the lead. **That landing will move every row
  with a turning formation leader, and needs its own pair.**

## 11. No AutoTarget on plane rows (packet `cc9_plane_row_autotarget`, queue item 5, `kPlaneRowAutoTargetBound`)

Worker cc9-ships8, 2026-09-28. The read is cc9-gunnery9's (docs/GUNNERY_OPEN_ITEMS.md section 42):
- `009F6A20` builds the AutoTarget selector. Its one caller is `0083676A`, in the ship director's
  constructor `008366D0`.
- A plane's fire-target provider is `vtable[114h]` = `0047F180`, which answers null. A squadron's
  is `007ECFD0`, which answers `[+348h]`, the `0084D810` controller.
- So no plane or squadron runs `009F5DA0`.

**The host.** `ControllerUpdateBinding::step_auto_target` ran the ship director's AutoTarget on
every ship-AI row, the load-time plane rows included.

**The binding.**
- ON, a row answering `IsKindOf(0Fh)` or `IsKindOf(18h)` returns before the tick.
- The census line is `summary mission ship ai plane row autotarget ticks= thinks= bound=`. It counts
  the plane-row ticks on both sides and their thinks on OFF.
- The gunnery side (`kPlaneNullFireTargetProviderBound`, ON on main) already drops a plane row's
  stored target in the pass. This packet removes the producer too.

### The OFF census (`local\ships8_c0_<row>.log`, this tree at `58b617200`)

| Row | plane-row ticks | thinks |
| --- | --- | --- |
| USN04 4700/4500 | 13500 | 678 |
| USN01 3200/3000 | 15000 | 755 |
| USN13 3200/3000 | 0 | 0 |
| JM06 / JM05 / JM08 / LOMP10 | 3000 / 27000 / 30000 / 30000 | 151 / 1359 / 1510 / 1510 |

### Predictions, written before any ON run

- **USN04 and USN01: exit 1 or 3.**
  - The gunnery pass already ignores these rows' stored targets.
  - What remains is the tick's own side effects: its stream-1 draws and the row's think counters.
    With per-unit streams, a draw moves only that plane's later draws.
- **USN13: exit 1.** No plane row ticks. Only the census line changes.
- **Mechanism check:** ON reads `ticks` equal to OFF's until the tracks diverge, and no plane-row
  think is run.

## 12. The AutoTarget follower gate (packet `cc9_autotarget_follower_gate`, queue item 7, `kAutoTargetFollowerGateBound`)

Worker cc9-ships8, 2026-09-28. The read is cc9-gunnery9's (docs/GUNNERY_OPEN_ITEMS.md section 44).
The binding is 44.4's code as written, with two counters added: follower thinks, on both sides,
and leaves run, ON only.

### The OFF census (`local\ships8_c0_<row>.log`)

| Row | follower thinks |
| --- | --- |
| USN04 | 3616 |
| USN01 | 1575 |
| USN13 | 7097 |
| JM06 | 1509 |
| JM05 | 6027 |
| JM08 | 2717 |
| LOMP10 | 1207 |

**Correction to 44.3 and 44.4.**
- 44.3 counted the Lua follow joins. The scene's own formation groups make many more followers:
  every row above has formation followers, USN04 included.
- So 44.4's "USN04 identity" does not hold on this base.

### Predictions, written before any ON run

- **Every row above: exit 3.**
  - A follower no longer picks its own fire target. Its guns take targets only from its
    director's commands.
  - A follower whose current command is neither null nor `follow` leaves its formation
    (`leaves` > 0). Its group then loses a member, and the follow and formation summaries move.
  - Death rows can move on the fighting rows (JM06, JM05, USN04, USN13).
- **Mechanism check:**
  - ON reads `follower_thinks` > 0 on every row.
  - Each leave is followed by a formation-group change for that unit.
  - After its leave a unit stops being counted as a follower.

### Section 11: the pairs (OFF `local\ships8_c0_<row>.log`, this tree at `58b617200`; ON `local\ships8_c5_<row>.log`, `pair_export --commit 58b617200 --flip kPlaneRowAutoTargetBound=true`, SHA-256 prefix `6A4E92A17363`)

| Row | pair_diff | Predicted | Census ON |
| --- | --- | --- | --- |
| USN04 | 1 | 1 or 3 | ticks 13500, thinks 0 |
| USN01 | 1 | 1 or 3 | ticks 15000, thinks 0 |
| USN13 | 1 | 1 | ticks 0 |

**Mechanism check: passed.** The tick counts equal OFF's, and no plane-row think runs. Gameplay is
identical, because the gunnery pass already drops a plane row's stored target. **Verdict: ON.**

### Section 12: the pairs (ON `local\ships8_c7_<row>.log`, `pair_export --commit 58b617200 --flip kAutoTargetFollowerGateBound=true`, SHA-256 prefix `0549AC29D977`)

| Row | pair_diff | Predicted | follower thinks ON | leaves | deaths |
| --- | --- | --- | --- | --- | --- |
| USN04 | 3 | 3 | 2069 | 7 | 40 to 41; 2 death rows only ON, 1 only OFF |
| USN01 | 3 | 3 | 171 | 163 | 5, identical |
| USN13 | 3 | 3 | 2552 | 1344 | 27 to 26 |
| JM06 | 3 | 3 | 161 | 11 | 1, identical |
| JM05 | 3 | 3 | 4694 | 157 | 1, identical |
| JM08 | 3 | 3 | 714 | 714 | 11; 10 rows changed |
| LOMP10 | 3 | 3 | 61 | 61 | 2; 2 rows changed |

**Mechanism check: FAILED on its third clause.** A unit that leaves does not stay out of its
formation:
- On USN01, Ralph, McCall and Blue each leave 51 times: once per AutoTarget think, and they rejoin
  in between.
- On USN13, Monterey, Intrepid and Cowpens each leave 53 times.
- On JM08, the transports and landing ships loop the same way.

What rejoins them is the AI group's follower pass: `AiCommand::request_join_formation`,
`0077C8D0` -> `0077F940` in `src/game_hosts_ai.cpp`, the `ai diag follow <unit> -> <leader>`
lines. So the host now alternates leave and join about once a second.

**Verdict: OFF, recorded.** The gate is the image's code. Whether the image loops the same way
depends on one thing this packet did not read: does the image's join path also leave `follow`
(`00E08F60`) in the follower director's first command slot?
- If it does, the gate returns at `009F5DE0` and never leaves. The host's join is then missing
  that command, and that is the fix.
- If it does not, the image loops too, and the switch can flip.

The next read is the caller of `0077C8D0` in the AI group pass, and what it writes to
`[director+54h]`.

**The follow-up read, done in the same turn.** The image's join does leave `follow` in the slot, so
the loop is the host's gap and not the image's behaviour:
- `00A10DC0`, the AI follower pass, calls only `0077C8D0` for a ship follower.
- The join `0077F940` ends at `0077FA8D..0077FAB8`: `ordered->vtable[114h]()` (the director);
  when that is not null, `director->vtable[58h](target)`, with the target taken from `[ESP+80h]`,
  the join's argument.
- The director's `vtable[58h]` is `00720CD0` (docs/WEAPON_DIRECTOR.md). It clears the ten command
  slots at `+54h` and issues `this->vtable[60h](00E08F60, block)`: a `follow` of the target.
- So in the image, a unit that joins holds `follow` in `[director+54h]`, and the gate returns at
  `009F5DE0` without a leave.
- The host's `GameUnitsHost::formation_join_0077f940` does not issue it. That was noted as unread
  in docs/SHIP_UNIT_GROUP_FOLLOW.md, line 709.

**The fix is in the units host, not in this lane.** At the end of a successful
`formation_join_0077f940(follower, leader)`, the follower's director should run `00720CD0` with
the leader. It is already reconstructed as `bsp::issue_target_command_00720cd0` in
`src/weapon_director.cpp` and `src/command_execution.cpp`. It went to the lead. Once it lands,
`kAutoTargetFollowerGateBound` re-pairs on the same seven rows; the expected leaves are those
of followers whose director was given a different command after the join.

## 13. Rank 9: the close attack's busy member `00A1443D` (packet `cc9_close_member_class_trait`, read)

Worker cc9-ships8, 2026-09-28. This is a read only; the binding waits on a units-host accessor.

### The slot

`00A14435 MOV ECX,[ESI+538h]` then `00A1443D CALL [EDX+2Ch]` asks the member's vehicle class
`vtable[2Ch]`. A true answer skips the member (`00A14444 JNE 00A14D4D`). The slot was read out of
the image on disk (`local\ships8_vt.py`) in the nine class vtables whose `+24h` is `009635D0`:

| vtable | `+28h` (the unit creator) | `+2Ch` |
| --- | --- | --- |
| `00D1ACC4`, `00D1ACF8` (`006FE590`, `MDestroyer`), `00D1AD38`, `00D1ADBC` (`006EB290`, the Cargo class), `00D1ADF8`, `00D1AE38`, `00D1AE78`, `00D1AEBC` | various | `00827FB0` |
| `00D1AD78` | `0074BE00`, the landing-ship class (`00963C80` answers kinds 0Ch, 6, 5) | `00963C70` |

- **`00827FB0`** (`00827FB0-00827FC7`, RET then INT3) answers
  `[class+78Ch] != 0 && [class+790h] != 0`. Those are `LandingShip`, a class pointer, and
  `LandingShipAmount` (docs/SHIP_CLASS_FIELDS.md). So it asks whether the class **carries landing
  craft**.
- **`00963C70`** (`00963C70-00963C7C`) answers `[class+809h] == 0`. `+809h` is the landing ship's
  `Rocketer` (docs/VEHICLE_CLASS_LUA_LOAD.md, `0074C754` / `0074C770`). So a landing ship that is
  **not a rocket ship** answers true.

So the slot is **"a troop-landing unit"**: a landing ship without rockets, or a class that carries
landing ships. The close attack does not send such members; they belong to capture. The name
`close_controller_busy` is wrong and should become a trait name.

### The same slot elsewhere

- `capture_weight_00a03510`: Cargo 0Bh answers 3.0 when the slot is true. The Cargo class's slot
  is `00827FB0`. No Cargo class in this installation authors `LandingShip`, so the answer stays
  0, and the host's 0 is right for this installation.
- `009F347E` and `009F35E3`, the approach warn sweep (docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md).

### Reach in this installation

`scripts/datatables/autoload/vehicleclasses.lua` (mtime 2026-05-09, locally modified):
- `LandingShip` with `LandingShipAmount` 4 is authored on two `LandFort` classes (lines 61063 and
  62199).
- `Rocketer = true` is authored once, on "LSM Rockets" (line 7754).
- So every LST and LSM landing ship except the rocket variant answers true.
- Those rows, JM08 (LST and LSM) and USN13, would lose their landing ships from the close attack.

### What the binding needs

The ship-AI and AI hosts cannot read `+78Ch`, `+790h` or `+809h`. The units host holds them:
`ShipClassFields::landing_ship_class` and `landing_ship_amount`, and the landing-ship reader's
`landing_ship_is_rocketer`. The proposed declaration went to the lead:

```cpp
// [unit+538h]->vtable[2Ch](): 00827FB0 on a ship-family class ([class+78Ch] LandingShip and
// [class+790h] LandingShipAmount both non-zero), 00963C70 on the landing-ship class
// (creator 0074BE00: [class+809h] Rocketer clear). False for a unit with no class.
bool unit_class_lands_troops_vtable_2c(std::size_t index) const;
```

With it, `AiCommand::close_member_controller_busy` answers it behind a new switch, and
`capture_weight_00a03510`'s Cargo arm answers `trait ? 3.0 : 0.0`. The rows to pair are JM08,
USN13, JM05 and LOMP10 (landings), plus USN04 for identity.

## 14. Rank 3: the free-bearing query `009DC2E0` (packet `cc9_free_bearing_query`, `kShipAiFreeBearingBound`)

Worker cc9-ships8, 2026-09-28. The listing was scripted whole: 836 lines, `009DC2E0-009DCEA2`, the
function's last instruction at `009DCE9E`, read against the decompile. Every name is a hypothesis.
The reconstruction is `src/ship_ai_free_bearing.cpp` (header `include/bsp/ship_ai_free_bearing.hpp`).

### The image

- **ABI.** `char __thiscall(searcher)(query*)`, `RET 4`.
  - `ECX` is a searcher: `blk+0A24h` for the sector scan's `009EC0C1`, and
    `blk+0A24h + index*20h` for the arm final step's `009DF0FA`.
  - The query is `ShipAiSectorFreeBearingQuery`, with origin, direction, the two widths `+10h`
    and `+14h`, range, the output bearing `+1Ch` and a layer word `+20h`.
- **Gates.** It answers 0 when the searcher's `+0h` byte is clear (`009DC2F4`), or when range
  < 10 [`00CE38B8`] (`009DC313`).
- **Refresh (`009DC319..009DC3B2`).** `009D7050` on the searcher, with the origin, half side
  `sqrt(max(width_a, width_b)^2 + range^2)` twice, and the layer word. It answers 0 when the
  selected list `searcher+18h` is then empty.
- **Ahead (`009DC3C3..009DC462`).** `004158E0` from the origin to `origin + range*dir`.
  - With no crossing it goes straight to the clearance pass.
  - With one, the "clear end" is the crossing less one unit of `dir`, and the base heading is
    `00414EB0(dir)`.
- **Corner candidates (`009DC4AA..009DC8D2`).**
  - A candidate walks the outline from the hit segment with `00416270` (below). It pulls the
    corner back one unit along `dir`.
  - If the leg from the origin to that point crosses the outline again (`004158E0`), it walks
    from that crossing the other way, monotone, and takes that point.
  - Its deviation is `|00438B10(00414EB0(corner - origin), base)|`.
  - **Round one** walks monotone. The forward side is valid under 75 degrees [`00D1F6F8`]; the
    backward side under 75 degrees selects at once.
  - **Round two** runs only when round one validated neither side. It walks non-monotone with
    pi/4 [`00CEDCD0`, a double].
  - A selection takes the smaller deviation, the forward side on a tie. It writes the bearing,
    turns `dir` to it (`006BC0C0`), recasts the leg, and moves the clear end to that leg's
    crossing less `dir`, or its end. The answer becomes 1.
- **Clearance pass (`009DC9B5..009DCE54`).** It runs only when a width is at least 1.
  - `leg = |origin - clear end|` (`00414C60`). A leg under 10 m answers **0**, even after a
    selection.
  - Every selected segment is visited: the run's next edge unless `closes_run`, else `next_run`.
    Each is taken in the frame `along = dir . r`, `lateral = (-dir.z, dir.x) . r`.
  - It is skipped outside `0 < along < leg` and `-width_b < lateral < width_a`, and otherwise
    clipped to `0..leg`.
  - A segment mostly on the positive side gives `(lat_end - min(width_a, along_end / 1.25)) /
    max(along_end, 1)`, kept as a minimum.
  - A segment mostly on the negative side gives `(min(width_b, along_start / 1.25) + lat_start) /
    max(along_start, 1)`, kept as a maximum.
  - The image visits a segment that reaches past plus or minus 1 on the other side a second time
    (`009DCDB8..009DCDDA`). The second pass computes the same values, so the reconstruction takes
    each segment once.
  - When `|max + min| > 0.001` [`00D7A23C`], the bearing becomes
    `00438B10(00414EB0(dir), _CIatan(max + min))` and the answer 1.

`00416270`, the outline walk, is `__stdcall(seg, &origin, radius, forward, monotone, &out)`,
`RET 18h`, body `00416270-004166D5` (verified: `JMP` at `004166D0`, INT3 from `004166D5`).
- It walks forward (`+10h`) or backward (`+14h`), with n = (e.z - s.z, -(e.x - s.x)).
- A segment facing away from the origin answers its near end (`0041636A` / `004165CD`).
- A far end outside the circle answers `004F3BA0`'s crossing (`004163C2`, `00416622`): of two,
  the one nearer the far end; one, that one; none, the far end.
- The end of the chain answers the far end.
- Without `monotone`, a far end nearer than the previous far end answers the near end.

### Uncertainty

- The x87 intermediates between stores are not modelled; every listed float store is.
- The sector scan's layer word is read from the searcher's own `+14h` (`blk+0A38h`, copied at
  `009EC02A`).
- `004F3BA0`'s ambient-stack case (docs/AVOID_ZONE_ARC.md) is inherited.

### The binding

- `kShipAiFreeBearingBound` was committed OFF at `2ea6ea916`.
- The sector scan's `009EC0C1` uses searcher 0, with the layer word the searcher's own `+14h`.
- The arm final step's `009DF0FA` uses searcher 1 or 2, with its area key.
- Each runs `bsp::ship_ai_free_bearing_009dc2e0` over the controller's `ShipAiSearchStorage`
  cache/list pair, refreshed through `GameAvoidZoneRuntime::refresh_search`.
- OFF keeps the record answering false.
- The census line is `summary mission ship ai free bearing scan_calls= arm_calls= ... answers=
  bound=`.

### The OFF census (`local\ships8_d0_<row>.log`, this tree at `2ea6ea916`)

| Row | sector-scan calls | arm-final calls |
| --- | --- | --- |
| USN02 9200/9000 | 0 | 151987 |
| USN04 4700/4500 | 0 | 80693 |
| USN13 3200/3000 | 0 | 53219 |
| JM06 3200/3000 | 0 | 41246 |
| LOMP06 1200/1000 | 0 | 18240 |
| JM08 3200/3000 | 0 | 14720 |
| USN01 3200/3000 | 0 | 12980 |

No sector scan reaches its avoid-zone arm on these rows, so every call is the arm final step's.

### Predictions, written before any ON run

- **USN02 and USN04: exit 1.** Section 1 found them without avoid zones: the early out is exact
  there. Every call refreshes, finds an empty list and answers false (`empty` = calls). Only the
  census line and the host-method statuses change.
- **JM06, LOMP06, USN01 and USN13: exit 3.**
  - Ships near land now get a bearing from the query. `ahead_hits` counts the ones whose look-ahead
    crosses an outline, and `answers` the ones whose bearing is used.
  - The arm final step steers by that bearing instead of blk+324h, so their paths change.
  - Deaths may move where ships fight near land (JM06, USN13).
- **JM08: exit 1 or 3.** It has the fewest zone calls.
- **Mechanism check:**
  - `refills` > 0 on the zone rows, and `empty` equals the calls on USN02 and USN04.
  - `answers` stays at or below `ahead_hits` plus `lateral_turns`.
  - No ship that sailed clear of land OFF runs aground ON. The query can only turn a bearing away
    from an outline.

### The pairs (OFF `local\ships8_d0_<row>.log`; ON `local\ships8_d1_<row>.log`, `pair_export --commit 2ea6ea916 --flip kShipAiFreeBearingBound=true`, SHA-256 prefix `FC2236C756B4`)

| Row | pair_diff | Predicted | calls / empty / ahead hits / lateral turns / answers | What moved |
| --- | --- | --- | --- | --- |
| JM06 | 3 | 3 | 41246 / 37283 / 96 / 0 / 96 | 96 forward-corner answers. Damage 5080.3 to 5105.9; deaths identical |
| USN01 | 3 | 3 | 12969 / 7672 / 0 / 257 / 257 | 257 clearance leans. Northampton's shots 167 to 213; the five Mavis deaths move by up to 0.7 s |
| LOMP06 | 1 | 3 | 18240 / 17294 / 0 / 0 / 0 | none. 946 calls find outline but nothing ahead, and no width to lean in |
| USN13 | 1 | 3 | 53219 / 51338 / 0 / 0 / 0 | none, for the same reason |
| JM08 | 1 | 1 or 3 | 14720 / 14715 / 0 / 0 / 0 | none |
| USN04 | 1 | 1 | 80693 / 80682 / 0 / 0 / 0 | none |
| USN02 | 1 | 1 | 151987 / 151976 / 0 / 0 / 0 | none |

**Mechanism check: passed.**
- `answers` equals `ahead_hits` plus `lateral_turns` on every row.
- Every answer comes from a list the refresh filled; `refills` > 0 on every row.
- No death appears.

**Two spread misses.**
- LOMP06 and USN13 stay identical: their near-land queries find outline in the box, but none
  ahead of the ship, and the arm final step's widths there are under 1, so nothing leans.
- USN04 and USN02 have 11 non-empty calls each, against section 1's "no zone" early out. They
  answer nothing. Section 1's count was the old searcher box; this query's box is larger.

**Verdict: ON.** The mechanism matches, and the moves are the two predicted rows.

### Section 10: the yaw-rate half, re-paired after `kUnitYawRateForwardSpeedBound` (main `5d12669e0`)

- The base is this tree at `28e776eb1`: main merged, `kShipFollowStationPointBound` ON, and the
  units fix ON.
- **OFF** is `pair_export --commit 28e776eb1 --flip kShipFollowStationPointBound=false`, SHA-256
  prefix `5928B1898F5C`, logs `local\ships8_y0_<row>.log`.
- **ON** is the tree's own build, logs `local\ships8_y1_<row>.log`.
- The zone half was identity on every row (above), so this pair measures the leader yaw rate at
  `009DF607`.

**OFF census.** `leader_turning` counts the calls whose `00811940` now answers non-zero:

| Row | `00811940` calls | non-zero |
| --- | --- | --- |
| USN04 | 8186 | 7991 |
| USN02 | 1496 | 1371 |
| JM06 | 565 | 556 |
| USN12 | 10 | 10 |

**Predictions, written before the ON runs.**
- The follower's speed blend `009DACD0` at `009DF612` now takes the leader's real yaw rate, where
  it took 0.
- So a follower on the outside of a turn speeds up and one on the inside slows.
- USN04, USN02 and JM06: exit 3, with formation followers' tracks and speeds moving. Death rows may
  move on USN04 and USN02.
- USN12: exit 1 or 3; only 10 calls.
- Mechanism check: `leader_turning` on ON matches OFF until the tracks diverge.

**The pairs (the yaw-rate half):**

| Row | pair_diff | Predicted | ON `leader_turning` | What moved |
| --- | --- | --- | --- | --- |
| JM06 | 3 | 3 | 556 | Deaths 2 to 1: USTroopTransport 02 survives. USTroopTransport 01 takes 61 damage instead of 1335 |
| USN02 | 3 | 3 | 1371 | Deaths 11 to 13: Witte and Perth are sunk only ON |
| USN04 | 1 | 3 | 7991 | none |
| USN12 | 1 | 1 or 3 | 10 | none |

**Mechanism check: passed.**
- `leader_turning` on ON equals OFF on all four rows.
- The USN04 miss is the blend's own geometry. `009DF61B..009DF64A` interpolates from the leader's
  ratio at `along = 0` to the wake ratio at 400 m [`kShipAiFormationSpeedBlendDistance`]. A
  follower 400 m or more astern takes the wake ratio alone, so the leader's yaw rate cannot reach
  it.
- USN04's columns keep their followers that far back. JM06's and USN02's are closer, and they move.

**Verdict: `kShipFollowStationPointBound` stays ON,** now with both halves live. The yaw-rate half
is no longer blocked.

## 15. Handoff (cc9-ships8, 2026-09-28, at about 78% context)

**State.** No lease is held. Every packet below is committed on `agent/cc9-ships8`.

| Section | Switch | State |
| --- | --- | --- |
| 7 | `kPlannerGroupTargetValueBound` | ON, landed |
| 8 | `kShipAiApproachEnterReseedBound` | ON, landed |
| 9 | `kShipAiSubTargetEntryPointsBound` | ON, landed |
| 10 | `kShipFollowStationPointBound` | ON. The yaw-rate half was re-paired after `kUnitYawRateForwardSpeedBound` (`5350cc4d9`, unlanded at writing) |
| 11 | `kPlaneRowAutoTargetBound` | ON, landed |
| 12 | `kAutoTargetFollowerGateBound` | **OFF.** Waits on the units host's join follow-up |
| 13 | rank 9 | read only. Waits on the units host's `unit_class_lands_troops_vtable_2c` |
| 14 | `kShipAiFreeBearingBound` | ON (`d86f5b131`, unlanded at writing) |

**Waiting on cc9-lua9 (the lead sends each sha):**
1. **The join follow-up.** `formation_join_0077f940` must run the follower director's `00720CD0`
   (`follow` on the leader). Then re-pair `kAutoTargetFollowerGateBound` on USN04, USN01, USN13,
   JM06, JM05, JM08 and LOMP10:
   - flip the switch in an export, OFF = the tree;
   - the check is that `leaves` falls to the followers whose director was re-commanded after the
     join, and that no unit leaves repeatedly.
2. **The rank-9 accessor.** Bind `AiCommand::close_member_controller_busy`: rename it to a trait
   name and answer the accessor, behind a new switch.
   - Also answer the Cargo arm of `capture_weight_00a03510`: 3.0 when the trait holds. It is still
     0 on this installation.
   - Pair JM08, USN13, JM05, LOMP10 and USN04.

**Not to redo.**
- `local\ships8_gtv.py <log>` replays the planner's rounds from the section 7 sample lines.
- `local\ships8_run.ps1` (`-Exe`, `-Prefix`, one `-Rows tag:MISSION:frames:mission_frames` per
  call; `run_game.ps1` queues past three slots) and `local\ships8_wait.sh <seconds> <logs...>`.
- `local\ships8_vt.py` reads vtable slots from the image on disk. `local\ships8_rd.py
  f:<addr>|d:<addr>` reads float and double constants.

**Traps met.**
- `pwsh -File script.ps1 -Rows a b c` binds only the first row. Launch one row per call.
- Python `write_text` on Windows turns LF sources into CRLF. Use `open(p, 'w', newline='')`, and
  keep edit scripts in files: this bash tool's heredocs break on some quotes.
- A leader-yaw effect reaches only followers within 400 m along the column (`009DACD0`'s blend).
  Check `along` before predicting a follower move.
- The arm final step's 009DC2E0 widths are often under 1, so its clearance pass is skipped there.

**Also open (added at retirement).**
- The item-7 re-pair keys on cc9-lua9's `kFormationJoinFollowBound`.
- **Rank 8,** the power-up use in `00A179E0` (section 5), needs a power-up subsystem before it
  can be bound.
- Section 1's other ranks remain as ranked. Ranks 1, 2, 3, 5 and 10 and the approach re-seed are
  done; ranks 8 and 9 are the only ones read and not bound.

**Reference k (cc9-gunnery10, added after this handoff).** Reference k (docs/GAME_EXECUTABLE.md,
"Mission reference baselines, 2026-09-28 k (main 5aaa4948a)", `reports/cc9_reference_rebaseline_11.json`)
replaces j. On it:
- USN02's 13 deaths need six landings, each ON: the sub-target pair, the party replan, the group
  target value, the approach reseed, follow station point and the yaw-rate forward speed.
- JM06's US Cargo Transport 02 survival is redundant inside the ship-AI group.
- LOMP06's Ryujin Maru sinking needs the sub-target pair and the group target value.

k's rows do not contain these switches, which went ON later:
- this document's sections 17 to 24: `kShipAiClearancePathFadeBound`, `kShipAiClearanceOutcomeWiringBound`,
  `kShipAiArmFinalAreaKeyBound`, `kTroopLandingTraitBound` and `kAutoTargetFollowerGateBound`;
- cc9-lua10's follow-law and landing switches: `kFormationJoinFollowBound`,
  `kFollowLeaderTurnRateBound`, `kFollowLeaderLiveSpeedBound`, `kFollowTargetDirAcosBound`,
  `kLandingSequencerBound`, `kLandingApproachBitBound` and `kLandStandbyStateBound`.

These are reference l's first flags.

## 16. The second ranking (packet `cc9_ship_ai_open_ranking_2`, cc9-ships9, 2026-09-28)

**It replaces section 1's table.** Section 1 was built on reference i; every lane flip since then
is in this base.

**Source.** Ten rows run once on `agent/cc9-ships9` at main `941fd197b`, with no flip:
`build\win32\Release\bsp_game.exe` in that worktree (SHA-256 prefix `D69A82E0C609`).
- The launch is reference j's: `tools/run_game.ps1`, `BSP_GUNNERY_RNG_STREAMS=1`,
  `BSP_DEATH_TABLE=1`, lockstep 0.05, `--press-start-frame 30 --menu-select <mission>`, idle
  player, present interval immediate. A 300-frame USN01 smoke ran first.
- Logs: `local\ships9_b0_<row>.log` in that worktree. Every log has its milestone line, the
  module directory under `cc9-ships9` and the final COM release.
- `local\ships9_census.py ships9_b0 rows <regex>` sums the non-concrete host rows per mission.
  `local\ships9_sites.py <row name>` prints the record site in `src/`.
- `local\ships9_vsj.py ships9_b0 <rows>` diffs each row against reference j (`rb10_<row>`,
  worktree cc9-gunnery8).

**The base rows.** These are the OFF logs for this lane's next pairs. `pair_diff` against j exits 3 on all ten rows.
On JM08, USN12 and JM05 the deaths, hit records, shots and damage are j's; the other seven moved. The moves belong to the post-j landings (the section 15 table,
the plane-lane dead-bot think, the periscope, the yaw-rate speed and others). They are not
attributed here.

| mission | frames | deaths | hit records (hull) | shots | damage | first hit | torpedo / dive releases |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN01 | 3200/3000 | 5 | 474 (81) | 1480 | 2831.2 | 49.10 s | 0 of 5 / 2 of 2 |
| USN02 | 9200/9000 | 13 | 3179 (442) | 2473 | 46042.5 | 18.90 s | - |
| USN04 | 4700/4500 | 39 | 711 (128) | 9488 | 10136.8 | 101.05 s | 1 of 16 / 0 of 19 |
| JM06 | 3200/3000 | 1 | 328 (322) | 378 | 4231.2 | 79.15 s | - |
| JM08 | 3200/3000 | 11 | 361 (106) | 2324 | 4149.8 | 5.25 s | - |
| USN13 | 3200/3000 | 27 | 646 (196) | 6989 | 8450.6 | 96.40 s | 0 of 60 / - |
| LOMP06 | 1200/1000 | 1 | 3 (1) | 10 | 2400.0 | 36.90 s | - |
| USN12 | 3200/3000 | 0 | 30 (15) | 83 | 978.1 | 8.30 s | - |
| JM05 | 3200/3000 | 1 | 41 (24) | 100 | 2396.3 | 9.90 s | 0 of 12 / 0 of 6 |
| LOMP10 | 3200/3000 | 2 | 94 (77) | 442 | 980.0 | 90.50 s | - / - |

**The still-false switches.**
- This lane has one: `kAutoTargetFollowerGateBound` (`src/game_hosts_ship_ai.cpp`, section 12).
- Outside the lane: `kNavigatorForceTorpedoBound` (`include/bsp/game_hosts_script_orders.hpp`)
  and `kSquadronSetCommandBound` (`src/game_hosts_commands.cpp`). The rest of the `= false`
  constants in `src/` and `include/` are trace and debug switches or the image's own constants.

### Closed since section 1

| item | switch | where |
| --- | --- | --- |
| a surface ship attacking a submarine (rank 1) | `kShipAiSubTargetSubStatesBound`, `kShipAiSubTargetEntryPointsBound`, ON | sections 2 and 9 |
| the follower's station point (rank 2) | `kShipFollowStationPointBound`, ON | section 10 |
| the free-bearing query (rank 3) | `kShipAiFreeBearingBound`, ON | section 14 |
| the party brain's replan flag (rank 5) | `kAiPartyReplanFlagBound`, ON | section 3 |
| the planner candidate's group target value (rank 10) | `kPlannerGroupTargetValueBound`, ON | section 7 |
| the approach enter re-seed | `kShipAiApproachEnterReseedBound`, ON | sections 4 and 8 |
| no AutoTarget on plane rows | `kPlaneRowAutoTargetBound`, ON | section 11 |

Section 1's "exact or closed" table still holds for the rows it lists. The AutoTarget rows
`009F5E69`, `00521EA0`, `00465080`, `00E08F70+vtable0C` and `00CFC3D0+vtable140` are the gunnery
lane's (docs/GUNNERY_OPEN_ITEMS.md sections 17 and 31), so they are not ranked here.

### The ranking

Reach as in section 1: 4 decides who is shot at, 3 an order or a movement, 2 a count, 1
presentation. Calls are the sum over the ten base rows.

| rank | item | image | host file and label | calls | reach, in one line |
| --- | --- | --- | --- | --- | --- |
| 1 | **the AutoTarget follower gate** (section 12; queue item 3) | `007788B0` at `009F5DC4` | `game_hosts_ship_ai.cpp` `AutoTarget::controller_belongs_to_another`, `kAutoTargetFollowerGateBound` OFF | 252379 (JM08 56021, JM05 54205, LOMP10 47716, USN13 43186) | 4: a formation follower does not pick its own target. Waits on cc9-lua9's join follow-up |
| 2 | **the troop-landing class trait** (section 13; queue item 2) | `[unit+538h]->vtable[2Ch]` at `00A1443D` (close attack), `009F35D8` (approach warn sweep) and the Cargo arm of `00A03510` | `game_hosts_ai.cpp` `AiCommand::close_controller_busy` (false); `game_hosts_ship_ai.cpp` `ShipAiApproach::unit_armament_ready` and `member_armament_ready_vtable_002c` (false) | 13381 close attack (JM05 7689, LOMP10 1290, USN02 1207); 460 approach (USN01 325, LOMP10 120) | 3: the close attack sends troop landers; the approach's warn sweep runs with an empty list. **The approach name "armament ready" is wrong**: it is the same slot as the close attack's. Waits on cc9-lua9's accessor |
| 3 | **the turn clearance's path fade** | `009EFFF2..009F0072` in `009EF910`: `00778890(unit)` **or** `[[unit+738h]+54h] == 00E08F80` (`moveonpath`) fades the heading error by `00419010` over `blk+330h` and `00811A30` | `game_hosts_ship_ai.cpp` `ShipAiClearance::path_fade_00778890` answers false. Its comment reads the test as a conjunction and says neither half has a producer. The listing is `JNE 009F0022` after the leader call, so either half suffices, and the units host now answers the leader half (`unit_formation_group_0284`, `formation_leader_0014`) | 37443 on every row (USN02 12309, JM05 8573, USN04 6170, USN13 4184, JM06 3297) | 3: a formation leader, or a ship on `moveonpath`, gets a smaller heading error, so `blk+370h` = 1 (heading error large) is set less often at `009F00BF`. The approach answers the same routine false at three more sites: the sub-state step `009F3429` (`unit_is_group_leader`, 460 calls, USN01 325, with the stale label "No AI group object exists"), the standoff `009E6F6E` / `009E70A7` and the throttle limit `009E6C3C` (no calls on these rows). The arm final, the layer choice and the path corridor already answer it from the formation accessors |
| 4 | **the AI command's avoid-zone point** | `00417B10` from the command tick's leader and follower arms | `game_hosts_ai.cpp` `AiCommand::avoid_zone_offset_point` returns the requested point ("contract: unread", stale) | 2305 (USN13 955, JM05 403, USN04 297, JM08 210) | 3: the destination the AI orders when it lies inside an avoid zone. The routine is bound for the follower station point (`zones.offset`); the AI host has to reach the ship-AI host's `GameAvoidZoneRuntime`. Read the call's margin and mode first |
| 5 | **the group reference release** | `00A2B8F0` `__thiscall(group+24h)(emptyGroup)`, from the compose drain `00A2E784` | `game_hosts_ai.cpp` `AiGroups::release_group_reference`: reverts a command whose target group emptied, "labelled substitution", "contract: unread" | 53515 (JM05 22079, JM08 15823, USN13 10026) | 3: what a group does after the group it was ordered against empties. JM05 creates 224 groups and destroys 201 by proximity merge. The body is unread |
| 6 | the approach frame state's unread spans (section 1 rank 12) | `009F1BC0` spans as section 1 lists; `[target+740h]` at `009F1E36`; `[unit+494h]` at `009F32A0`; the sub-heading and sub-throttle producers `009E5E90` / `009E6A90` | `ShipAiApproach::frame_state_unread_spans`, `target_zone_object_0740`, `unit_depth_reference`, `sub_heading_command`, `sub_throttle_command` | 31634 (USN02 28784, USN01 1748, LOMP10 1002) | 3 for a zone-object target and a submarine approach. USN02's mode 0 matched the image's standoff (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md 1) |
| 7 | the heading wrap after a heading store (section 1 rank 7) | `00605070` on `blk+1D8h` at `009DFF81` / `009E00FA` and on `brain+1E0h` at `009F3360` | `ShipAiControls::after_heading_stored` and `ShipAiApproach::wrap_brain_heading` record; the host interface passes the heading by value, so a binding needs a reference | 141156 + 31576 (USN13 70431, JM08 21318, USN12 13929, JM05 13641) | 0 to 3. A counter of out-of-range stores decides it cheaply; it was not added here |
| 8 | the carrier arm of the squadron exclusion (section 1 rank 13) | `009FFEB0` | `AiCommand::squadron_excluded_009ffeb0` answers `007EDA90`'s false | 15232 (JM05 8260, USN13 2058, JM08 1528) | 3 for a carrier's squadrons in an AI group. Borders the plane lane |
| 9 | BigLandingShip (section 1 rank 11) | `class+808h` at `00827F95` and in `00A03510` | `ShipAiNeighbour::big_landing_ship_808` answers 0 | 1705 (JM08 only) | 3 when an enemy submarine is near an LSM or LST (this installation's `vehicleclasses.lua`, mtime 2026-05-09), and in capture scoring |
| 10 | the group's area key (section 1 rank 6) | `0070E450` (`0070E450-0070E4B3`, RET then INT3), **read here**: over the formation's `[+4F8h]` members at `+18h` stride `34h`, the maximum of `vtable[214h]()` among members that answer `vtable[5Ch](6)`, floored at 0 | `ShipAiArmFinal::group_area_key_0070e450` answers the leader's own layer | 81292 (USN02 25845, JM05 13990, USN04 8882, USN13 8792) | 3 only for a formation whose members sit on different layers, or whose leader is not kind 6. Likely exact on these rows. Cheap: `ShipAiLayer::group_layer_0070e450` already computes the whole routine, so the arm final can share it |
| 11 | a director `stop` and the command begin (section 1 rank 16) | `009E1170`, `00835C70` | `game_hosts_commands.cpp` `CruiseCommand::stop_state_step` (cc9-gunnery9's file); `CommandController::begin_command` | 1608 / 2077 | 3: what a `stop` asks of the ship |
| 12 | the navigator's avoidance receivers (section 1 rank 15) | `0071C1E0` from `00721A93`; setters `009DABB0` / `009DABD0` | `game_hosts_script_orders.cpp` `Navigator::avoidance_receiver_torpedo` / `_land` | 150 / 140 | 3, on few calls. Not this lane's file |
| 13 | the follow request's OwnerPlayer arm (section 1 rank 17) | `00779DB4`, `entity+188h` | `game_hosts_ai.cpp` `owner_player_known = false` | follow requests 152 (USN01) to 916 (JM05) | 3 and rare: it can only admit a follow the image refuses between differently owned ships |
| - | the AI party's power-up use (section 5) | `00A179E0` | `AiParties::party_brain_plan_tail` | 462 | 3 to 4. Skipped until a power-up subsystem exists (docs/GUNNERY_OPEN_ITEMS.md section 43) |

**Not ranked, and why:**
- `ShipAi::unit_weapon_director`, `drive_heading_vtable50` and `ShipAiOrder::slot_to_order_ring`
  (978274 each) are structure, as in section 1.
- `AiGroups::seed_collection` (35700) walks one flat collection. With `kAiGroupSeedPerEntityBound`
  ON every admitted entity gets its own group, so the split into five collections changes only the
  creation order. That order is not shown to matter.
- `ShipAiFollow::refresh_world_pose` `00414DB0` (36000), `ShipAiPlanner::release_node_list`,
  `ShipAiSearch::release_path_node` and `ShipAiApproach::scratch_00954940` are cache and memory
  housekeeping with no gameplay reader found.
- The random stand-ins `ShipAiApproach::traffic_random_00bd2f10`, `avoid_random_00bd2f10`,
  `ShipAiApproachPoint::random_stream1` and `ShipAiNavBlock::uniform_00bd2f10` answer the low
  bound. They are labelled, and binding them would couple the lane to the shared generator
  (the RNG-stream note in docs/GUNNERY_OPEN_ITEMS.md).
- The unread state leaves `ShipAiState::enter_vtable04`, `step_vtable0c` and `exit_vtable08`
  (422 / 169 / 149) and `ShipAiAttack::initial_enter_009db590` (45) run on few calls; each needs its
  own read.
- `AiPlanners::capture_spawn_arm_00a2b400` and `defend_spawn_tail_00a29b8e` are units-host
  quick-spawns.

**Labels that no longer hold:**
- `ShipAiClearance::path_fade_00778890`: "neither has a producer here", and the conjunction.
- `ShipAiApproach::unit_is_group_leader`: "No AI group object exists". The formation groups exist.
- `ShipAiApproach::unit_armament_ready`: the slot is the troop-landing trait (section 13).
- `AiCommand::avoid_zone_offset_point`: "contract: unread" (the routine is complete).
- `AiCommand::close_controller_busy`: the name (section 13).

**Top three.** The follower gate and the troop-landing trait wait on cc9-lua9. The first packet
free to take now is rank 3, the path fade: the leader half binds from the units host's formation
accessors, the `moveonpath` half from the director's current command, and it reaches every row.

## 17. Rank 3: the turn clearance's path fade (packet `cc9_clearance_path_fade`, `kShipAiClearancePathFadeBound`)

Worker cc9-ships9, 2026-09-28.

### The site

`009EF910` (`BSP_ShipAi_RefreshTurnClearance`, body `009EF910-009F00F3`), read with
`disasm-raw 009EFFE0 --length 290`:

```
009EFFF2  MOV ECX,[ESI+3FCh]        ; the unit
009EFFF8  TEST ECX,ECX / JE 009F0076
009F0000  CALL 00778890             ; the unit leads its formation
009F0005  TEST AL,AL / JNE 009F0022 ; a leader takes the fade
009F0009  MOV EAX,[ESI+3FCh] / MOV EAX,[EAX+738h] ; the command controller
009F0015  TEST EAX,EAX / JE 009F0076
009F0019  CMP [EAX+54h],00E08F80 / JNE 009F0076   ; slot 0 is `moveonpath`
009F0022  ...                       ; error *= 00419010(1, 1, 2, 0, [blk+330h] / 00811A30(unit, 1))
009F0076  ...                       ; |error| > settings +214h / +218h -> [blk+370h] = 1 (009F00BF)
```

- `00778890` (`00778890-007788A8`, RET then INT3): `[unit+284h]` non-null and `[[unit+284h]+14h]`
  equal to the unit. The units host answers it from `unit_formation_group_0284` and
  `formation_leader_0014`, as three other ship-AI bindings already do.
- `[controller+54h]` is the command slot 0's singleton pointer (docs/COMMAND_EXECUTION.md, slot
  layout). The units host forwards it as `director_slot_command(index, 0)`. `00E08F80` is
  `bsp::kCommandMoveOnPath`.
- **The gate is a disjunction.** The host's old comment read it as one predicate with no producer
  for either half, and answered false.
- **The effect.** The faded error falls to 0 when the remaining path `blk+330h` is at least two
  turn lengths, so outcome 1 (`HeadingErrorLarge`) is not set. `009F3F80` reads outcome 1 at
  `009F4A44` and `009F4A51` (`src/ship_ai_obstacle_tables.cpp`): it raises escape request 4 and the
  turn-assist load. The fade only lowers the error, so it can only remove outcome-1 frames.

**ABI.** `00778890` is `__thiscall` (ECX the unit), returns AL, plain `RET`. The host interface
`ShipAiClearanceHost::path_fade_applies_00778890()` keeps the one-predicate shape.

**Uncertainty.** `formation_leader_0014` is the host's model of `[group+14h]`. The director's
slot 0 is the host's queue; the image's `+54h` is read without the override slot, and the host
reads slot 0 the same way.

### The binding

`ClearanceBinding::path_fade_applies_00778890` in `src/game_hosts_ship_ai.cpp` answers
`leader || moveonpath` when `kShipAiClearancePathFadeBound` is true, and false otherwise. Both
sides count into `summary mission ship ai clearance path fade`:
- `tests`, the calls reaching the gate;
- `leader`, and `moveonpath` for a non-leader on `moveonpath`;
- `applied`, ON only;
- `heading_error_large`, the frames that end the clearance refresh with outcome 1. It counts
  held outcomes too, so it can exceed `tests`.

### The OFF counts (`local\ships9_c0_<row>.log`)

Each OFF row is gameplay-identical to its section 16 base row (`pair_diff` exit 1 on all ten).

| row | tests | leader | moveonpath | heading_error_large |
| --- | --- | --- | --- | --- |
| USN01 | 739 | 409 | 0 | 348 |
| USN02 | 12309 | 1779 | 395 | 12978 |
| USN04 | 6170 | 317 | 478 | 1134 |
| JM06 | 3297 | 465 | 255 | 894 |
| JM08 | 207 | 0 | 155 | 0 |
| USN13 | 4184 | 384 | 1129 | 744 |
| LOMP06 | 991 | 82 | 0 | 0 |
| USN12 | 481 | 170 | 179 | 0 |
| JM05 | 8573 | 1069 | 0 | 4794 |
| LOMP10 | 389 | 157 | 0 | 312 |

### Predictions, written before the ON runs

1. **JM08, LOMP06 and USN12 are identical** (exit 0 or 1). OFF has no outcome-1 frame there, and
   the fade can only remove outcome-1 frames.
2. **`applied` is at most `leader + moveonpath`** on every row, and positive on every row.
3. **`heading_error_large` falls on USN01, USN02, USN04, JM06, USN13, JM05 and LOMP10.** It cannot
   rise until the rows diverge.
4. **Movement.** Where a fade-eligible ship had outcome 1, fewer escape requests follow, so its
   turn and speed change. The rows above may move (exit 3). How many do is not predicted: the OFF
   counters do not split outcome-1 frames by eligibility.

### The pairs and the verdict: ON, and the outcome has no reader in the host

OFF is the tree build of `3c63148ea` (`local\ships9_e0_<row>.log`). ON is
`pair_export --commit 3c63148ea --flip kShipAiClearancePathFadeBound=true`, SHA-256 prefix
`79D2F46EBDCF` (`local\ships9_pfon_<row>.log`). A 300-frame USN01 smoke ran first.

| row | pair_diff | applied (= leader + moveonpath) | heading_error_large OFF -> ON |
| --- | --- | --- | --- |
| USN01 | 1 | 409 | 348 -> 6 |
| USN02 | 1 | 2174 | 12978 -> 11406 |
| USN04 | 1 | 795 | 1134 -> 1092 |
| JM06 | 1 | 720 | 894 -> 582 |
| JM08 | 1 | 155 | 0 -> 0 |
| USN13 | 1 | 1513 | 744 -> 708 |
| LOMP06 | 1 | 82 | 0 -> 0 |
| USN12 | 1 | 349 | 0 -> 0 |
| JM05 | 1 | 1069 | 4794 -> 4668 |
| LOMP10 | 1 | 157 | 312 -> 72 |

- Predictions 1 to 3 held: the three rows without outcome-1 frames are identical, `applied` equals
  `leader + moveonpath` on every row, and outcome-1 frames fall on all seven other rows.
- Prediction 4's movement did not happen on any row. **The reason is a host wiring gap, not the
  fade.** `009F3F80` reads `blk+370h` at `009F4999` (request 2 only while it is 0) and `009F4A02`
  (request 4 for outcome 1 or 3), on the same block `009EF910` writes (both routines take ESI =
  blk, with the unit at `+3FCh`). The host keeps the clearance outcome in
  `ctl.clearance.outcome_370` and gives the escape reader `ctl.obstacle.escape_mode_370`. Nothing
  writes the second, so the escape reader always sees 0. `drive_order_ring_009f3f80` copies only
  `clearance_37c` across.
- **Verdict: ON.** The mechanism matches as far as the host lets the outcome travel. The movement
  waits on the `+370h` wiring, which is now the lane's next item (section 20).

## 18. Rank 10: the arm final's group area key (packet `cc9_arm_final_area_key`, `kShipAiArmFinalAreaKeyBound`)

Worker cc9-ships9, 2026-09-28.

**The routine.** `0070E450` (`0070E450-0070E4B3`, RET then INT3, `__thiscall` with ECX the
formation, no stack arguments), read with `disasm-raw 0070E450 --length 104`:
- `0070E459`: the loop runs over `[group+4F8h]` members, at `group+18h` with stride `34h`.
- `0070E478..0070E483`: a member counts only when `vtable[5Ch](6)` answers true.
- `0070E487..0070E497`: the answer is the largest `vtable[214h]()`, starting from 0 (`0070E452`).

**The two host sites.**
- The layer choice (`ShipAiLayer::group_layer_0070e450`, `009ECA20`) already ran the whole routine.
  Its body moves unchanged into `Impl::formation_navigation_layer_0070e450`.
- The arm final step (`009DEEE9` and `009DEFD3`, only under the leader test `009DEE1E`) answered
  the leader's own travel layer `blk+30Ch`. That answer never differs from the key it is compared
  with, so the "moved" path `009DEF83..009DF060` never ran.

**The switch.** `kShipAiArmFinalAreaKeyBound` true answers the whole routine at the arm final.
Both sides count `calls` and `differs` (the whole answer against the stand-in) in
`summary mission ship ai arm final area key`.

**Uncertainty.** `vtable[214h]` is modelled as `ship_ai_unit_navigation_layer_006dfd80` on the
member's leaf tuning, as the layer choice has it. A member without loaded tuning is skipped.

**The OFF counters** (`local\ships9_d0_<row>.log`, tree build of `d7f56deb3`):

| row | calls | differs |
| --- | --- | --- |
| USN02 | 25845 | 25845 |
| USN04 | 8882 | 8882 |
| USN01, JM06, USN13, LOMP06, USN12, JM05, LOMP10 | 2881 to 13990 | 0 |
| JM08 | 0 | 0 |

The comparison is the image's own. `009DE5FF` seeds the key from `blk+30Ch`, the travel layer after
the layer choice's clamp and goal adjustment (`009ED067..009ED0E7`), while `0070E450` answers the
raw largest member layer. On USN02 and USN04 the two differ on every call, so the image takes the
"moved" path `009DEF83..009DF060` on every arm-final pass of those leaders.

**Predictions, written before any ON run.**
1. **USN01, JM06, JM08, USN13, LOMP06, USN12, JM05 and LOMP10 are identical** (exit 0 or 1):
   `differs=0`.
2. **USN02 and USN04 move.** Their formation leaders re-aim the heading query from the pose and
   `blk+324h` and ask the second searcher. A leader's heading target can change, and the followers
   follow it.

### The pairs and the verdict: ON (spread miss recorded)

ON is `pair_export --commit 3c63148ea --flip kShipAiArmFinalAreaKeyBound=true`, SHA-256 prefix
`E5C18454A683` (`local\ships9_akon_<row>.log`), against the same OFF logs as section 17.

- **All ten rows are gameplay-identical** (exit 1).
- Prediction 1 held: the eight `differs=0` rows are identical.
- On USN02 and USN04 the mechanism runs as read. `calls` doubles (25845 -> 51690, 8882 -> 17764)
  because every pass now also reads the key again at `009DEFD3` on the "moved" path. The free
  searcher's refill counter moves on USN02 (559 -> 546).
- Prediction 2's movement did not appear: the second searcher's heading did not change a
  gameplay line in these windows. That is a spread miss with the mechanism matching.
- **Verdict: ON.**

## 19. Rank 2: the troop-landing class trait (packet `cc9_close_member_class_trait`, `kTroopLandingTraitBound`)

Worker cc9-ships9, 2026-09-28. Section 13 read the slot. This section binds it at its three sites.

**The accessor.** cc9-lua9's `GameUnitsHost::unit_class_lands_troops_vtable_2c` (main `94be34af2`)
answers `[unit+538h]->vtable[2Ch]()`:
- `00827FB0` on a ship class: `LandingShip` and `LandingShipAmount` are both set.
- `00963C70` on the landing-ship class: `Rocketer` is clear.
- Plane, land and building descriptors answer false; their slot `2Ch` is unread.

**The three sites, one switch** (`include/bsp/game_hosts_ai.hpp`, OFF):

| site | image | host method, renamed | effect when true |
| --- | --- | --- | --- |
| the close attack's member pass | `00A14435 MOV ECX,[ESI+538h]`, `00A1443D CALL [EDX+2Ch]`, `00A14444 JNE 00A14D4D` in `00A13B60` | `AiCommand::close_member_lands_troops` (was `close_member_controller_busy`) | the member is not served |
| the approach warn sweep | `009F347E` (group members), `009F35D8` / `009F35E3` (the unit itself) in the sub-state step | `ShipAiApproach::unit_lands_troops` (was `unit_armament_ready`; the member arm answered false with no record) | the unit joins the warning candidates |
| the Cargo capture weight | `00A03510`, class `0Bh` | `capture_weight_00a03510(class, lands_troops)` | 3.0 instead of 0 |

The rename follows the callee's body (checklist rule 1). "Busy" and "armament ready" were call-site
guesses.

**Counters, on both sides.**
- `summary mission ai troop landing trait`: `close_landers`, the close-attack asks that answer
  true, and `cargo_landers`.
- The ship-AI host counts `approach_trait_tests` and `approach_troop_landers` (in
  `GameShipAiSummary`, not printed).

**ABI.** The slot is `__thiscall` on the class descriptor with no arguments and returns AL.

**Uncertainty.** The approach's member arm walks the formation's members only for a leader
(`009F3429`). The host still answers that leader query false (rank 3, section 17), so only the
unit's own arm `009F35E3` asks. A kind other than ship answers false because its slot is unread.

### Section 13's census was wrong: the trait holds on troop transports

The OFF counters of the first run contradicted the static prediction that no Cargo unit answers
true. A bounded diagnostic (`ai troop landing trait site=<close|cargo> unit=<name> class=<id>`,
the first answer per unit and site) names the units:

| row | site | units (class 0Bh, Cargo) |
| --- | --- | --- |
| JM06 | close attack | USTroopTransport 01 to 04 |
| JM08 | Cargo capture weight | USTroopTransport 01 to 06 |
| JM05 | Cargo capture weight | Japan Troop Transport 01 to 05 |

- In this installation's `vehicleclasses.lua` (mtime 2026-05-09), `LandingShip` 90 / 40 with
  `LandingShipAmount` 4 are authored on `VehicleClass[224]` "IJN Troop Transport (Strafeable)"
  (`Type` "Cargo" at line 61360) and `VehicleClass[234]` "US Troop Transport (Strafeable)"
  (`Type` "Cargo").
- Section 13 took them for `LandFort` classes. An awk over the table picked up nested `Type`
  keys of other classes. The top-level `Type` of these tables comes late in each table.
- So the Cargo arm of `00A03510` answers 3.0 for these transports in the image, not 0.

**The OFF counters** (`local\ships9_d0_<row>.log`, tree build of `d7f56deb3`):

| row | close_landers | cargo_landers |
| --- | --- | --- |
| JM06 | 200 | 0 |
| JM08 | 0 | 246 |
| JM05 | 0 | 585 |
| USN01, USN02, USN04, USN13, LOMP06, USN12, LOMP10 | 0 | 0 |

### Predictions, written before any ON run (they replace the static ones)

1. **USN04, USN13 and LOMP10 are identical** (exit 0 or 1). Both counters are 0 there, and the
   approach half cannot move gameplay (point 4).
2. **JM06 moves through the close attack.** USTroopTransport 01 to 04 stop being served by the
   close-attack pass (`close members served` falls), so any close-attack order to them stops.
   Whether a death or a hit moves is not predicted.
3. **JM08 and JM05 move only if the capture or defend scoring uses the transports' weight.** In
   those scorings (`00A03760` arrival value and the defend collect), the transports weigh 3.0
   instead of 0. The rows are identical if no capture or defend think sums them.
4. **The approach half moves nothing.** After the trait, the warn sweep still meets records:
   - `target_warn_radius_07c4` answers 0, so the range gate `009F3585` fails;
   - `candidate_accepts_warning_vtable_0234` answers false;
   - `route_warning_message_0077c2a0` is a record.

### The pairs and the verdict: ON

OFF is the tree build of `3c63148ea` (`local\ships9_e0_<row>.log`). ON is
`pair_export --commit 3c63148ea --flip kTroopLandingTraitBound=true`, SHA-256 prefix `E2841412EE4B`
(`local\ships9_tton_<row>.log`).

| row | pair_diff | what moved |
| --- | --- | --- |
| USN04 | 1 | nothing (prediction 1) |
| USN13 | 1 | nothing (prediction 1) |
| LOMP10 | 1 | nothing (prediction 1) |
| JM08 | 1 | nothing. The transports' 3.0 is summed by no capture or defend think in the window (prediction 3) |
| JM05 | 1 | nothing, as JM08 (prediction 3) |
| JM06 | 3 | the close attack: `served` 566 -> 366, exactly the 200 lander asks; `attackmove` 304 -> 104; scored 3606 -> 1406. Hit records 328 -> 247, shots 378 -> 266, first hit 79.15 -> 74.65 s, deaths 1 -> 1 (prediction 2) |

- On JM06, USTroopTransport 01 to 03 take more or fewer hits, and US Tanker 01 is no longer hit
  (30 hits -> 0). The Fletcher-class 08 the player controls moves 540.68 -> 75.88 m.
- **Every prediction held and the mechanism matches exactly, so the switch is flipped ON.**
- The approach half and the Cargo arm stay bound and inert on these rows.

## 20. The clearance outcome `blk+370h` never reaches the escape reader (new, found by section 17)

Worker cc9-ships9, 2026-09-28. Read only; the first item for the next packet.

- **Image.** `009EF910` writes `blk+370h`: 0 at `009EF969`, 1 at `009F00BF`, 2 at `009EFFA5` and
  3 at `009EFFB8`. `009F3F80` reads it at `009F4999` (request 2 is raised only while it is 0) and
  at `009F4A02..009F4A51` (a non-zero outcome with a clear latch `+378h` and outcome 1 or 3, or
  the stall time `+384h` past its threshold, raises request 4 and the turn-assist load
  `[unit+102Ch]`). Both routines run on ESI = the same control block.
- **Host.** `src/game_hosts_ship_ai.cpp` keeps two copies: `ctl.clearance.outcome_370` (written by
  the reconstructed `009EF910`) and `ctl.obstacle.escape_mode_370` (read by the reconstructed
  `009F3F80`). `drive_order_ring_009f3f80` copies `clearance_37c` from one to the other and not
  `outcome_370`, so the reader always sees 0.
- **Reach 3, on every row.** With the wiring, request 2 stops while an outcome is set and request 4
  starts for outcomes 1 and 3. Section 17's OFF counts give the outcome-1 frames alone: USN02
  12978, JM05 4794, USN04 1134, JM06 894, USN13 744, USN01 348, LOMP10 312.
- **Binding.** One copy, `ctl.obstacle.escape_mode_370 = static_cast<int>(ctl.clearance.outcome_370)`,
  beside the `clearance_37c` copy, behind a new switch. Count requests 2 and 4 on both sides first.

### Bound OFF (`060d3c8a7`, `kShipAiClearanceOutcomeWiringBound`) and the OFF counts

The switch copies `ctl.clearance.outcome_370` into `ctl.obstacle.escape_mode_370` beside the
`clearance_37c` copy in `drive_order_ring_009f3f80`. Both sides print
`summary mission ship ai clearance outcome wiring`: the frames per outcome value as `009F3F80`
would see them, and the obstacle routine's two load raises (all three sites of each). The tree has
the path fade and the troop-landing trait ON.

OFF (`local\ships9_f0_<row>.log`, tree build of `060d3c8a7`):

| row | heading (1) | blocked moving (2) | blocked stopped (3) | turn-assist raises | secondary raises |
| --- | --- | --- | --- | --- | --- |
| USN02 | 11406 | 10362 | 6846 | 44954 | 21733 |
| USN04 | 1092 | 3786 | 30 | 31881 | 23964 |
| JM05 | 4668 | 3528 | 1740 | 15459 | 11399 |
| JM06 | 48 | 3072 | 882 | 7930 | 4994 |
| USN13 | 708 | 1638 | 114 | 4509 | 1836 |
| LOMP06 | 0 | 460 | 24 | 1148 | 710 |
| USN12 | 0 | 138 | 0 | 209 | 63 |
| LOMP10 | 72 | 0 | 0 | 0 | 2 |
| USN01 | 6 | 0 | 0 | 0 | 24 |
| JM08 | 0 | 0 | 0 | 0 | 5 |

### Predictions, written before any ON run

1. **JM08 is identical** (exit 0 or 1): every frame's outcome is 0, so the reader sees what it saw.
2. **USN02, USN04, JM05, JM06, USN13, LOMP06 and USN12 move** (exit 3). There the raises happen
   while an outcome is set. With the wiring, request 2 is refused at `009F4999` on those frames
   and request 4 opens at `009F4A21..009F4A44` for outcomes 1 and 3. The secondary-raise count
   changes on each of these rows. Its direction is not predicted.
3. **LOMP10 and USN01 are identical or move little.** Their only outcomes are heading frames (72 and
   6) on rows with no turn-assist raise, so the escape arms are rarely reached with an outcome set.

### The pairs and the verdict: ON (one spread miss recorded)

OFF is `local\ships9_f0_<row>.log`. ON is `pair_export --commit 060d3c8a7 --flip
kShipAiClearanceOutcomeWiringBound=true`, SHA-256 prefix `656807825A6A`
(`local\ships9_owon_<row>.log`).

| row | pair_diff | turn-assist / secondary raises OFF -> ON | what moved |
| --- | --- | --- | --- |
| USN02 | 3 | 44954 / 21733 -> 35532 / 16259 | deaths 13 -> 11 (Witte and Perth survive), hit records 3179 -> 2827, shots 2473 -> 2409, damage 46042.5 -> 40763.0 |
| USN04 | 3 | 31881 / 23964 -> 30163 / 23931 | deaths 39 -> 41, shots 9488 -> 10661, damage 10136.8 -> 10850.0. The death rows that flip are planes (D3A Val #3.1, #7.1, A6M Zero #7.2, #8.2), which the shared RNG stream couples |
| JM06 | 3 | 7930 / 4994 -> 5604 / 2490 | hit records 247 -> 344, shots 266 -> 406, damage 4214.2 -> 4972.3; deaths equal |
| USN13 | 3 | 4509 / 1836 -> 5358 / 2784 | shots 6989 -> 6839, one hit record; deaths equal |
| JM05 | 3 | 15459 / 11399 -> 17371 / 15362 | USS Phelps 2567.69 -> 2348.82 m; 89 unit rows move; deaths equal |
| LOMP06 | 3 | 1148 / 710 -> 1186 / 748 | two unit rows; combat equal |
| USN01 | 3 | 0 / 24 -> 0 / 24 | Enterprise's nearest distance 9009 -> 9017; combat equal |
| USN12 | 1 | 209 / 63 -> 209 / 63 | nothing |
| LOMP10 | 1 | 0 / 2 -> 0 / 2 | nothing |
| JM08 | 1 | 0 / 5 -> 0 / 5 | nothing |

- Prediction 1 held (JM08), and prediction 3 held (LOMP10 identical, USN01 a small move).
- Prediction 2 held on six of seven rows. **USN12 is identical: a spread miss.** Its only outcomes
  are 138 blocked-moving frames, and none of them met an escape arm.
- The load raises move on every moving row. They fall where heading and stopped outcomes dominate
  (USN02, JM06), because request 2 is refused while an outcome is set. They rise on JM05 and USN13,
  where request 4 opens.
- **Verdict: ON.** The path fade (section 17) now reaches the escape through this wiring.

## 21. Rank 5: the group reference release `00A2B8F0` (read only)

Worker cc9-ships9, 2026-09-28. **The routine maintains a per-group score list and touches no
command.**

- **`00A2B8F0`** (`00A2B8F0-00A2B94D`, `RET 4`, `__thiscall` with ECX = `group+24h`, one stack
  argument, the emptied group), read with `disasm-raw 00A2B8F0 --length 120`.
  - It walks `[ECX+5600h]` records of `0ACh` bytes from `ECX+0`.
  - It finds the first record whose dword at `+0A8h` equals the argument (`00A2B921..00A2B929`).
  - It shifts every later record down one slot with `REP MOVSD` of `2Bh` dwords
    (`00A2B90C..00A2B91D`), then decrements the count (`00A2B941`).
- **`00A2B950`**, its only other caller (`00A2B95F`), inserts a record into the same list.
  - It first removes the group's old record through `00A2B8F0`.
  - It then finds the first slot whose float at `+0` is below the new score (`00A2B976..00A2B987`).
  - It shifts the tail up (`00A2B98B..00A2B9B4`) and writes the score and the factors from
    `+0h` on (`00A2B9C1..`).
  - So `group+24h` is a list of up to 128 candidate records sorted by descending score, keyed by
    the candidate group at `+0A8h`.
- **Writers.** `00A2B950` is called from `BSP_AiPlanner_ChooseAttackTarget` at `00A1CE93`, and from
  `BSP_AiPlanner_CaptureThink` at `00A2A77C`, `00A2A844` and `00A2AD58`.
  - The `00A1CE93` insert is gated at `00A1CDE0` by the stack byte `[ESP+13h]`.
  - docs/AI_PLANNERS.md calls that insert a debug arm that prints the four factors. The gate
    byte's writer was not traced here, because the frame moves between `00A1CBE7` and `00A1CDE0`.
- **Readers.** None known. docs/AI_GROUP_THINK.md (the `+24h..+5623h` row) found no producer in the
  constructor and no consumer.

**What that means for the host.** `AiGroups::release_group_reference` in `src/game_hosts_ai.cpp`
reverts a command whose target group emptied, and calls that a labelled substitution for the
image's dangling `command+1Ch`. That label is accurate: `00A2B8F0` itself never touches a command.
The revert is a host rule with no image counterpart at this site.

**Next step, if the item is taken.** Read how the attack command tick uses `command+1Ch` after its
group is freed (docs/AI_COMMAND_OBJECT.md), and decide whether the host's revert changes an order
the image would issue. Until then the rank stays 5, with its reach unproven.

## 22. Rank 7: the heading wrap `00605070`, census (packet `cc9_heading_wrap_census`)

Worker cc9-ships9, 2026-09-28. This is a census only, with no switch.

**Counter.** `summary mission ship ai heading wrap` counts the heading values the host stores
where the image wraps them in place with `00605070`: `009DFF81`, `009E00FA`, the setter
`009DFFB0` and the approach `009F3360`. It also counts the values outside (-pi, pi] and the
largest magnitude. Logs: `local\ships9_g0_<row>.log`, tree build with sections 17 to 20 ON.

| row | stores | out of range | max magnitude |
| --- | --- | --- | --- |
| JM06 | 2595 | **79** | 6.2785 |
| USN13 | 70431 | 0 | 1.5970 |
| USN02 | 31891 | 0 | 3.1416 |
| JM08 | 21318 | 0 | 3.1416 |
| USN12 | 13929 | 0 | 1.8588 |
| JM05 | 13651 | 0 | 3.1416 |
| LOMP10 | 11720 | 0 | 3.1416 |
| USN01 | 6878 | 0 | 2.8274 |
| USN04 | 59 | 0 | 1.3351 |
| LOMP06 | 28 | 0 | 1.6494 |

**Reach.** Only JM06 stores unwrapped headings, near 2pi. Their readers go through
`wrapped_angle_subtract_00438b10` and `wrapped_angle_add_00438aa0`. Those loop by 2pi until the
result lies in (-pi, pi] (`00438AB0..00438B0B`, `00438B20..00438B7B`), so an unwrapped input
changes a result by float rounding only. One direct copy exists, `blk+324h = blk+1D8h` at
`009ED947`, and its readers use the same helpers.

**Rank 7 drops to reach 1**, a rounding difference on JM06's 79 stores. A faithful binding would
pass the heading by reference through `after_heading_stored_00605070` and
`wrap_brain_heading_00605070`, and wrap it with the recovered `00605070`. It is not worth a pair
set on its own.

## 23. Handoff (cc9-ships9, 2026-09-28, at about 76% context)

**State.** No lease is held. Every commit below is on `agent/cc9-ships9`. `e67503663` and
`ed050fcab` landed on main as `a24903f07` and `514770df9`.

| Section | Switch | State |
| --- | --- | --- |
| 16 | - | the second ranking (docs) |
| 17 | `kShipAiClearancePathFadeBound` | ON (`3edbe26f0`) |
| 18 | `kShipAiArmFinalAreaKeyBound` | ON (`3edbe26f0`), spread miss recorded |
| 19 | `kTroopLandingTraitBound` (`include/bsp/game_hosts_ai.hpp`) | ON (`4fbfd759d`). Section 13's census was corrected: the troop transports carry the trait |
| 20 | `kShipAiClearanceOutcomeWiringBound` | ON (`72c09257e`). It moves USN02 (deaths 13 -> 11) and USN04 (39 -> 41, plane rows) |
| 21 | - | rank 5 read (`bbae7f3e2`) |
| 22 | - | rank 7 census (`4e50a3aa2`); reach 1 |
| 12 | `kAutoTargetFollowerGateBound` | **OFF.** It waits on cc9-lua10's verdict for lua9's `kFormationJoinFollowBound` (main `f515961f3`) |

**What remains, by section 16's ranking.**
1. **Item 7, the follower gate re-pair** (section 12). Once the join follow-up is ON:
   - pair `kAutoTargetFollowerGateBound` on USN01, JM06, JM08, USN13, JM05, LOMP10 and USN04;
   - write the predictions first; the leave counts should fall to the join counts.
2. **Rank 4, the AI command's avoid-zone point.**
   - The call is `00A020F0` in `00A02020`, and only for a kind-6 member.
   - Zone set: `0082ADA0(class, 0)`, which is `group_for_layer([class+560h])`.
   - Margin 30.0 (`00CE38C8`), mode 1.
   - The AI host cannot reach the ship-AI host's `GameAvoidZoneRuntime`. It needs a cross-host query
     and startup wiring outside this lane's files. Send the lead the declaration.
3. **Rank 6, the approach frame state's unread spans.** USN02 carries 28784 calls. It is large.
4. Rank 5's next step (section 21) and ranks 8, 9, 11 to 13 of section 16, unchanged.

**Not to redo.**
- `local\ships9_run.ps1 -Exe <exe> -Prefix <p> -Row tag:MISSION:frames:mission_frames`, one row per
  call. `local\ships9_wait.sh <s> <logs>` waits for the final COM release.
- `local\ships9_census.py <prefix> rows <regex>` sums the non-concrete host rows.
  `local\ships9_sites.py <row>` prints a record site.
- `local\ships9_vsj.py <prefix> <rows>` diffs against reference j.
- OFF logs of the current tree: `ships9_g0_*` (all four switches ON, with the census counters).

**Traps met.**
- An RDP session fails runs in two ways. A **Disc** session dies at FMOD init (result 61). An
  **Active** rdp-tcp session fails renderer init (`hr=0x8876086a`) only intermittently: 7 of 16
  runs once, then none. Relaunch the failed rows only.
- `pair_diff` returns exit 2 (`Permission denied`) when a log is still held open for a second
  after its final COM release. Retry.
- `Add-Content` writes CRLF into an LF doc; the index normalises it, the working copy does not.
  Strip `chr(13)` in edit scripts.
- A census over `vehicleclasses.lua` must read each table's top-level `Type`, which comes late in
  the table. Nested `Type` keys of sub-tables misled section 13.

## 24. Item 7: the AutoTarget follower gate, re-paired after the join follow-up (section 12)

Worker cc9-ships9, 2026-09-28. The base is `f01935cde`, which is `agent/cc9-ships9` with main
`e0f07c8fd` merged: `kFormationJoinFollowBound` is ON, so a successful join `0077F940` runs
`00720CD0` and leaves `follow` in the follower's director.

**OFF** (`local\ships9_h0_<row>.log`, tree build of `f01935cde`):

| row | follower thinks | follow requests / joins | section 12's ON leaves (before the join follow-up) |
| --- | --- | --- | --- |
| USN01 | 1575 | 152 / 0 | 163 |
| USN04 | 3616 | 676 / 0 | 7 |
| JM06 | 1509 | 56 / 4 | 11 |
| JM08 | 2717 | 695 / 1 | 714 |
| USN13 | 7097 | 1313 / 0 | 1344 |
| JM05 | 6027 | 895 / 3 | 157 |
| LOMP10 | 1207 | 50 / 1 | 61 |

### Predictions, written before any ON run

1. **The leave-and-rejoin loop is gone.** On every row, `leaves` on ON is far below section 12's
   ON value. A follower whose director holds `follow` returns at `009F5DE0` without a leave.
2. **What leaves remain** belong to followers whose director was given a different command after
   they joined, by the mission script or the AI. They are few per row, at most the number of
   distinct followers times the number of times a script re-commands them.
3. **Rows move** (exit 3) wherever a leave remains, because the unit leaves its formation. A row
   with `leaves=0` on ON is identical apart from the gate's own counters.
4. **No death prediction.** Section 12's loop moved USN04 and USN13 deaths. Without the loop, fewer
   rows should move and each by less.

### The pairs and the verdict: ON

ON is `pair_export --commit f01935cde --flip kAutoTargetFollowerGateBound=true`, SHA-256 prefix
`80874933424C` (`local\ships9_fgon_<row>.log`).

| row | pair_diff | leaves ON (section 12's ON) | joins OFF -> ON | combat OFF -> ON |
| --- | --- | --- | --- | --- |
| USN01 | 3 | 13 (163) | 0 -> 3 | hit records 475 -> 393, shots 1485 -> 1290; deaths equal |
| USN04 | 3 | 7 (7) | 0 -> 0 | hit records 784 -> 701, shots 10458 -> 10212; deaths equal |
| JM06 | 3 | 6 (11) | 4 -> 4 | hit records 288 -> 276; deaths equal |
| JM08 | 3 | 17 (714) | 1 -> 14 | hit records 321 -> 411; deaths equal |
| USN13 | 3 | 39 (1344) | 0 -> 26 | deaths 27 -> 32 (five plane rows of "bruh #1.5" and "#1.9", RNG-coupled), shots 7068 -> 8194 |
| JM05 | 3 | 7 (157) | 3 -> 8 | combat equal |
| LOMP10 | 3 | 8 (61) | 1 -> 2 | hit records 94 -> 95, shots 442 -> 417 |

- **Prediction 1 held.** The leave-and-rejoin loop is gone. Leaves fall 12 to 42 times on USN01,
  JM08, USN13 and LOMP10. They fall by half on JM06 and to a twentieth on JM05, and stay 7 on
  USN04, which never looped.
- **Prediction 2 holds in the aggregate.** The remaining leaves are of the order of the rejoins
  (JM08 17 leaves against 14 joins, USN13 39 against 26, JM05 7 against 8). No per-unit leave
  line exists, so "no unit leaves repeatedly" is not checked unit by unit.
- **Prediction 3 held.** Every row has leaves, and every row moves.
- **Verdict: ON.** Section 12's recorded mechanism failure is resolved by the join follow-up.

## 25. Rank 4: the AI command's avoid-zone point (packet `cc9_ai_command_avoid_zone_point`, `kAiCommandAvoidZonePointBound`)

Worker cc9-ships10, 2026-09-28.

**The image.** `00A02020` (`BSP_AiCommand_IssueMoveToMember`, `__fastcall(member, point)`,
body `00A02020-00A02175`) reaches `00417B10` only on its ship arm:
- `00A0204E XOR EBX,EBX`; no later write to EBX before `00A020BD` (listing filtered for EBX).
- `00A020A2 JA 00A0216F` skips everything when the squared planar distance is below the double
  at `00D21530` (6400.0). `00A020B1` asks `vtable[+5Ch](6)` again; a non-ship takes the point as
  given at `00A0211C`.
- `00A020B7 MOV ECX,[ESI+538h]`, `00A020BD PUSH EBX`, `00A020BE CALL 0082ADA0`: `0082ADA0` reads
  `[class+EBX*4+560h]` and tail-jumps to `004120D0` (the group for that layer or the nearest
  below). So the set is the group of `[class+560h]`, the same one the follower's `009DF41A`
  resolves.
- `00A020C3 FLD [00CE38C8]` (`00 00 F0 41`, 30.0f) is the margin, `00A020CD PUSH 1` the
  containment test, and the input is `{point.x, point.z}` (`00A020C9`, `00A020E2`).
  `00A020F5..00A02113` store the answer's two floats in x and z and zero y.

**The binding.** `GameShipAiHost::avoid_zone_offset_point_00a020f0` answers from the host's
`GameAvoidZoneRuntime` (`group_for_layer`, then `offset(group, xz, 30.0, true)`). The AI host
reaches it through `GameUnitsHost::ship_ai()`, which the mission frame already sets, so no
construction site changed. The ticks now ask for the point only where the image reaches
`00A020F0` (`ai_order_bridge_takes_zone_point_00a02020`); before, the stand-in was recorded on
every leader order, squadrons and near members included, which is why section 16's 2305 calls
overstate the reach. Answers fall back to the requested point when the runtime is not ready,
the unit has no ship controller or no group answers the layer. Coverage: complete for
`00A020B7..00A02113`.

**Counters.** `summary mission ai command zone point asks / answers / moved` (AI host; asks on
both sides) and `summary mission ship ai command zone points answered / moved` (ship-AI host, ON
only).

**OFF** (`local\ships10_a0_<row>.log`, the tree build of `4e4a7f989`, main `9dcf7f537`; launch as
section 16's, one row per `local\ships10_run.ps1` call):

| row | frames | asks (00A020F0 reached) |
| --- | --- | --- |
| USN13 | 3200/3000 | 493 |
| JM05 | 3200/3000 | 393 |
| USN04 | 4700/4500 | 295 |
| JM08 | 3200/3000 | 99 |
| LOMP10 | 3200/3000 | 91 |
| USN01 | 3200/3000 | 49 |
| JM06 | 3200/3000 | 19 |

### Predictions, written before any ON run

1. **Asks equal on both sides**, and every ask is answered on ON (a ship class always has a
   `[class+560h]` layer and every mission here loads the avoid-zone table).
2. **Moved is rare.** An AI group is ordered at an enemy group's position or a planner point, both
   at sea; only a point inside an island's avoid polygon (or within 30 m of it) moves. Expect
   moved = 0 on most rows and a handful at most on the island-heavy rows (USN13, JM05, JM08).
3. **A row with moved = 0 is identical** apart from the new counters (`pair_diff` 0 or 1). A row
   with moved > 0 moves (exit 3): the ship steers for a different point.
4. **No death prediction.**

### The pairs and the verdict: ON

ON is `pair_export --commit 4e4a7f989 --flip kAiCommandAvoidZonePointBound=true`, SHA-256 prefix
`C0F43BC7CE0A` (`local\ships10_a1_<row>.log`). The diagnostic lines come from a second export of
`c513fab40` (`local\ships10_a2_<row>.log`, USN13, JM05 and LOMP10), which adds only a log line.

| row | pair_diff | asks OFF / ON | answered / moved | combat OFF -> ON |
| --- | --- | --- | --- | --- |
| USN13 | 3 | 493 / 493 | 493 / 493 | hit records 653 -> 658, shots 7287 -> 7323; deaths 32 equal, all 32 death rows changed in time or killer |
| JM05 | 3 | 393 / 393 | 393 / 393 | combat equal; the controlled ship's path moves by 0.01 m |
| JM08 | 3 | 99 / 99 | 99 / 99 | hit records 411 -> 395, shots 2248 -> 2390; deaths 11 equal, 9 rows changed |
| USN01 | 3 | 49 / 49 | 49 / 49 | combat and death rows equal |
| LOMP10 | 1 | 91 / 91 | 91 / 48 | identical |
| USN04 | 1 | 295 / 295 | 295 / 0 | identical |
| JM06 | 1 | 19 / 19 | 19 / 0 | identical |

- **Prediction 1 held.** Asks are equal on both sides, and every ask was answered.
- **Prediction 2 failed on its premise.** The AI does not order its groups at sea points on these
  rows. Every point on USN13, JM05, JM08 and USN01 lies inside an avoid-zone polygon (land or shoal; the zones were not named), and so
  do 48 of LOMP10's 91. The diagnostic lines show one point per order wave, shared by the whole
  group: all sixteen logged USN13 asks, from ten ships, carry (2973.4, -2182.2), and the push moves it 475.5 m for layer-11
  hulls. JM05's point (-1087.1, -817.3) moves 663.5 m for layer 11 and 436.6 m for layer 5, because
  each hull's `[class+560h]` picks its own group. LOMP10's layer-5 hull keeps its point, which is
  outside the layer-5 polygons.
- **Prediction 3 held.** The three rows with nothing moved, or with moves that change no order
  the executable carries out (LOMP10), are gameplay-identical. The four rows with moved points
  move.
- **Verdict: ON.** The mechanism is the image's: 00A02020 pushes a point inside a zone out of the
  hull's layer group before it issues `moveto`. The miss is in the spread, not the mechanism.
- **Open.** Why the AI's order points sit inside zones is not read here. They are the planners'
  points (00A124E0, 00A12A90, 00A15490's leader arms); a planner that targets an enemy base would
  explain it. The LOMP10 row moves 48 points and no gameplay line; its ordered ship was not
  followed further.

## 26. Rank 6: the approach frame state's mode latch (packet `cc9_approach_mode_latch`, `kShipAiApproachModeLatchBound`)

Worker cc9-ships10, 2026-09-28. Rank 6 of section 16 is `009F1BC0`'s unread spans. This section
takes the first of them, `009F1DBF-009F1E16` and the latch `009F1F47-009F2124`, read whole.

**The image.**
- `009F1DBF-009F1E12` split the raw target `brain+0B20h`. ESI is the target when it answers
  `vtable[5Ch](6)` (a ship), else 0. EBX is the target when it answers `vtable[5Ch](1Ch)` (a command
  building), else 0.
- `009F1E25 MOV EDI,2`, then `009F1E30 JE 009F2003` takes the no-ship path. EDI is not written again
  before `009F2022` and `009F2112`, so **both store mode 2**. docs/SHIP_AI_APPROACH_UPDATE.md's
  latch table gives 1 at those two addresses and says 2 is never assigned; both are wrong.
- A ship target: mode 1 when the unit is not kind 8, the target is kind 8, the point was not
  displaced, `00827F70(brain+0AACh)` is false and `00811A30(unit, 1.0)` times 2.1 (`00D0B3C8`,
  when the mode already is 1) or 1.9 (`00D21A94`) exceeds `nested+11E0h`. Mode 1 also sets
  `nested+11F0h = 00415510(&11F0h, &11E0h)`, the smaller. Otherwise mode 0. **Both jump to
  `009F272D`, past the retarget arm.**
- No ship target: no building gives mode 0. A building on the unit's side gives mode 2. A troop
  lander (`[brain+0AACh]->vtable[2Ch]`) gets mode 3 when it is kind `0Ch`, `006F2D90(target)`
  holds and `(double)[target+7C4h] + max(300.0, 2 * turn radius) >= nested+11E0h`, else mode 4,
  and resets `nested+11D6h`, `+11D8h` and `+121Ch` when `+121Ch` is negative. Any other building
  gives mode 2 when its side is 2, else 0. Then `009F2124` runs the retarget arm when
  `nested+11D6h` is clear.
- `006F2D90`, read: it walks the building's list at `+794h` / `+798h` and answers true when one
  element's `006AC220` returns 0. Name hypothesis: "has a free landing spot".

**The binding.** `ship_ai_approach_mode_latch_009f1f47` (`src/ship_ai_approach_update.cpp`) is
computed on every frame-state pass and counted on both sides; ON stores the mode, the clamp and
the reset. LABELLED inputs: the point is never displaced (no target zone object), `00827F70`
asks the unit's own kinds `0Eh` / `0Ch` and reads BigLandingShip as 0, `006F2D90` answers false
and `[target+7C4h]` 0, so a lander is always in mode 4. Coverage: complete for
`009F1DBF-009F1E16` and `009F1F47-009F2124`.

**OFF** (`local\ships10_b0_<row>.log`, the tree build of `5a5cfd3b4`; main `9dcf7f537` plus this
branch's rank 4 flip). `summary mission ship ai approach latch`:

| row | frames | no target | ship | building | other | modes 0/1/2/3/4 | retarget arm reachable |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN02 | 27786 | 5 | 27781 | 0 | 0 | 27786/0/0/0/0 | 5 |
| USN01 | 1748 | 0 | 5 | 1631 | 112 | 1748/0/0/0/0 | 1743 |
| LOMP10 | 619 | 0 | 18 | 601 | 0 | 619/0/0/0/0 | 601 |
| JM05 | 13 | 0 | 13 | 0 | 0 | 13/0/0/0/0 | 0 |

USN13, USN04, JM08, JM06, USN12 and LOMP06 run no frame state. No row has a submarine target,
a building of its own side or a troop lander on a building.

### Predictions, written before any ON run

1. **The latch is identical ON.** Every frame computes mode 0, which is what the field already
   holds. The four rows above are identical apart from the record counts (`pair_diff` 0 or 1).
2. **What the rows do reach is the retarget arm**, not the latch: USN01's 1631 and LOMP10's 601
   frames against an enemy building fall to `009F2124` in mode 0. That arm is the next read.

### The pairs and the verdict: ON

ON is `pair_export --commit 5a5cfd3b4 --flip kShipAiApproachModeLatchBound=true`, SHA-256 prefix
`B730F06BED3D` (`local\ships10_b1_<row>.log`).

| row | pair_diff | modes ON |
| --- | --- | --- |
| USN02 | 1 | 27786/0/0/0/0 |
| USN01 | 1 | 1748/0/0/0/0 |
| LOMP10 | 1 | 619/0/0/0/0 |
| JM05 | 1 | 13/0/0/0/0 |

- **Prediction 1 held.** All four rows are gameplay-identical.
- **Verdict: ON**, with the reach recorded: no reference row exercises modes 1 to 4, so the pairs
  test only that mode 0 is the latch's answer on these rows. A submarine hunt (mode 1) or a troop
  landing on a building (modes 3 and 4) needs its own row before its readers can be judged.

### What rank 6 still holds: the retarget arm `009F2124-009F272D`

It is what USN01 (1631 frames) and LOMP10 (601) reach: an attackmove against an enemy command
building, mode 0. Today the host copies the goal (the building) into the approach point on every
frame, so the ship steers at the building itself. The image does something else:
- `009F2124`: only when `nested+11D6h` is clear, which `009F1DAA` does every 2 to 3 s. It sets the
  byte and raises `nested+11D8h` to at least 1.0 (`00D7A24C`).
- Mode 3 (`009F21A0..009F2338`): `006F2DE0` / `006F2E60` on the building, `006AC5D0` for the
  point, and when in range `0x749D90` and `0077C2A0` issue a command (the landing). Not reached.
- Mode 4 (`009F2342..009F2395`): the point is `006F3AF0(building)(&out, unit+FCh, [class+570h])`.
  Not reached.
- Modes 0 and 2 (`009F239A..009F272D`): the point is the goal (`009F23B5`); then, when the goal
  lies in a zone of the unit's `0082ADC0` group (`004178F0`, `009F23E8`), a 60-slot loop over the
  ring directions at `nested+18h` (stride `4Ch`) casts from a point near the target
  (`009E6120`, `008FE120`) through `00416DD0` against that zone and through `00904400(44h)`, the
  Landscape segment query the gunnery host runs for line of sight, and keeps the last slot whose
  clearances pass (`00414C60` twice) as the approach point (`009F26F0`). This is x87-dense and
  needs `009E6120` and `008FE120` read first; it also needs a public Landscape segment query on
  the gunnery host (a line outside this lane).

## 27. Rank 6, continued: the retarget arm, modes 0 and 2 (packet `cc9_approach_retarget_ring`, `kShipAiApproachRetargetRingBound`)

Worker cc9-ships10, 2026-09-28. Read whole: `009F2124-009F2161` (the head) and
`009F239A-009F272D` (modes 0 and 2). Modes 3 and 4 (`009F21A0-009F2395`) are not covered.

**The image.**
- **The head.** `009F2124` skips the arm when `nested+11D6h` is set. `009F2131` sets it, and
  `009F2138..009F2161` raise `nested+11D8h` to at least 1.0 (`00D7A24C`; `FLD1`, `FCOMI`, `JBE`
  keeps the timer, unordered included). The byte is cleared again by `009F1DAA` when that timer
  runs out, so the arm runs once every 2 to 3 s.
- **The point is the goal** (`009F23B5`). Then `0082ADC0([unit+538h])` gives the unit's
  `[class+570h]` group and `004178F0` the first zone of it that holds the goal's x, z
  (`009F23E8`). No zone ends the arm (`009F23F1`).
- **The origin.** `008FE120(target)` is the target when it answers kind 5, else null. `h` is its
  `[[t+538h]+0A8h]` (the class height extent), else 0.0. The origin is
  `(goal.x, goal.y + max(50.0f 00CEB4D4, h), goal.z)` (`009F2474..009F248F`).
- **The ring.** ESI walks the sixty slots from `nested+18h` at stride `4Ch`. `[ESI-8]`, `[ESI-4]`
  and `[ESI]` are the slot's direction at slot `+0Ch`, `+10h` and `+14h`: cos, 0 and sin of its
  heading (`ShipAiAttackMoveRingSlot`). For each slot:
  - `R = max(0.6 * [unit+490h], [unit+490h] - 600.0)` (`00CEFF98`, `00D20198`), where
    `unit+490h` is the artillery-only maximum range (docs/SHIP_AI_FIREPOWER_INPUTS.md). With a
    building target in mode 2, `R = [building+7A0h] - 600.0` (`009F2505`).
  - `P = goal + dir * 100000.0` (`00CF81F0`), and `00416DD0(zone)(&P, &goal, &hit, &edge)` moves
    `hit` from the goal to the crossing nearest `P`: the outermost coast crossing on that bearing.
    Its answer is not tested.
  - `Q = (hit.x + dx * 100.0, 60.0f, hit.z + dz * 100.0)` (`00D7A220`, `00CEB4B0`).
  - `00904400(44h, &origin, &Q, &record, 0)` at `009F25FA`: a Landscape hit skips the slot.
  - The slot must have `R > |Q - goal|` and `best > |Q - unit|` (`00414C60` twice, `FCOMIP`,
    `JBE`), `best` starting at `FLT_MAX` (`00D7A248`).
  - Then `best = |Q - unit|` and the point becomes `hit + dir * 10.0` with y `dy * 10 + 0.0`
    (`00CE3DC0`, `00D7A258`), stored at `009F26F0..009F2704`.
- So the approach point becomes the spot 10 m off the coast, on the bearing nearest the unit,
  whose 100 m-seaward probe sees the target's top over the land and lies inside the unit's
  artillery reach. With no such bearing the point stays the goal.
- **The ship path skips all of it.** `009F1E30 JE 009F2003` sends the no-ship path past the goal
  copy at `009F1F0D`, so on that path the approach point changes only here.

**The binding** (`ship_ai_approach_retarget_ring_009f239a`, `src/ship_ai_approach_update.cpp`).
The Landscape cast needs a public query on the gunnery host, requested from the lead on
2026-09-28 (`GameGunneryHost::landscape_segment_hit_00904400`). Until it lands this section has
no committed code.

**Re-read by cc9-ships11 (2026-09-29).** The listing agrees with the reading above; five points
it did not state:
- **The stack slots.** `[ESP+58h..60h]` is the unit's `+0FCh..+104h` stored at `009F1C53..009F1C66`
  on this frame, `[ESP+70h]`/`+74h` the goal's x, z stored at `009F1CF6`, `[ESP+28h]` the kind-1Ch
  target of `009F1E04` (else 0). `009E6120` copies `[brain+0B2Ch..0B34h]`, so the origin is the goal.
  `0082ADC0` is `004218E0` then `004120D0([class+570h])`. `00414C60` is the 2-D length with the
  `1e-10` cutoff (`00CE3820`), the idiom `ship_ai_approach_goal_range_009f1bc0` already has.
- **The 0.6 is 0.6f widened** (`00CEFF98` = `3FE3333340000000`).
- **`00416DD0` writes `running` only past its AABB test** (`00416DE9`, copy at `00416DFB`), so a
  rejected slot keeps the previous slot's crossing. The goal is inside the zone, so the test
  passes from slot 0 on; the binding copies first and says so.
- **Mode 1 never reaches the arm** (`retarget_arm_reachable` is false for a ship target), so the
  arm is modes 0 and 2 plus the unread 3 and 4.
- **The point holds between arm runs.** The no-ship path skips the per-frame goal copy, and the
  arm re-runs only when `009F1DAA` clears `nested+11D6h` (every 2.0 s here: the reseed stand-in
  takes the low end). The host used to copy the goal every frame; ON restores the frame's
  starting point on that path before the head.

**The binding** (committed OFF). `ship_ai_approach_retarget_head_009f2124` and
`ship_ai_approach_retarget_ring_009f239a` in `src/ship_ai_approach_update.cpp`; the host is
`RetargetRingBinding` and `run_retarget_arm` in `src/game_hosts_ship_ai.cpp`: the zone from
`GameAvoidZoneRuntime::table()` with `004120D0` / `004178F0` on `class_reference_0570`, the crossing
from `avoid_zone_segment_hit_00416dd0`, the Landscape cast from the gunnery host, `unit+490h` from
the gunnery row, `[class+0A8h]` from `unit_class_extents` when the target answers kind 5, and
`[building+7A0h]` from `command_building_capture_range_07a0`. Coverage: partial, modes 3 and 4
(`009F21A0-009F2395`) unread and recorded as `ShipAiApproach::retarget_modes_3_4`. OFF counts
`off_zone_frames` (arm frames whose goal lies in a zone of the class's group); ON logs
`summary mission ship ai approach retarget` and the first sixteen zone runs as `retarget diag`.

### Predictions, written before any ON run

Base: section 28's latch census (`local\ships10_b0_*.log` in cc9-ships10): the arm is reachable on
USN01 (1743 frames: 1631 building, 112 other), LOMP10 (601, all building, mode 0) and USN02 (5,
no target), and on no other row.
1. **OFF is identical** to its own base on every row (a summary line and record rows only).
2. **JM05, JM06, JM08, LOMP06, USN04, USN12, USN13 are identical ON** (exit 0 or 1): no frame
   reaches the arm.
3. **USN02 is gameplay-identical ON.** Five no-target frames run the arm once per unit at most,
   and a no-target goal lies at sea (no zone), so the point stays the goal.
4. **USN01 and LOMP10 move ON (exit 3).** A command building stands on an island, so the goal
   lies in a zone and `zone_runs` > 0; with artillery reach `max(0.6r, r - 600)` of several km
   and a 50 m origin over the building, most bearings pass the Landscape cast and one lands
   inside reach, so `moved_runs` > 0 and the attacking ships steer for a point 10 m off the
   coast instead of the building. Expected downstream: different approach points in the
   per-unit rows, different paths, and first-hit / damage numbers that move. A death-table move
   is possible but not predicted.
5. **Mechanism failure** would be `zone_runs` = 0 on USN01 and LOMP10 (the building's goal is not
   inside the class group's zones, or the group key selects a different layer) or `moved_runs` = 0
   with every slot a Landscape hit; either keeps the switch OFF.

### The pairs (cc9-ships11, 2026-09-29)

OFF is `agent/cc9-ships11` at `8d4f73dce` (main `b6a9d2ece` plus the binding), built in the tree;
ON is `python tools/pair_export.py --commit 8d4f73dce --flip kShipAiApproachRetargetRingBound=true
--out local\s11_rt1` (exe `CBD6D40E79F7`). Launch as the reference rows (`local\s11_run.ps1`:
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, `--press-start-frame 30 --menu-select <row>
--mission-frame-seconds 0.05`). A 300-frame USN01 smoke ran first. Logs `local\s11_off_<row>.log`
and `local\s11_on_<row>.log`.

| row | frames | arm runs ON | zone runs | moved runs | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- | --- |
| USN01 | 3200/3000 | 200 | 0 | 0 | 3, moved | 4 missed on the mechanism |
| LOMP10 | 3200/3000 | 68 | 0 | 0 | 1, gameplay identical | 4 missed |
| USN02 | 9200/9000 | 5 | 0 | 0 | 1, gameplay identical | 3 held |

OFF counts `off_zone_frames=0` on all three. JM05, JM06, JM08, LOMP06, USN04, USN12 and USN13 were
not run: `run_retarget_arm` returns before anything on a frame the latch does not mark reachable,
and their base latch census has none.

**Why no zone: `[class+570h]` is 0 in single player.** A diagnostic build (not committed) logged
Northampton, Dunlap and the other USN01 attackers with `class_reference_0570 = 0`; `004120D0(0)`
selects the key-0 group, which holds no zones (six groups; the goal at (3973.4, -3182.2) lies in a
zone of the key-1 group). docs/GAME_SHIP_DEPTH_INPUT.md has the source: every ship leaf's scalar is
0 in this installation's single-player record and 1 or 3 in the multiplayer one (`settings+F0h`).
So the ring part of the arm is exact and unreachable in single-player missions; the 0.6 / 600 m
reach, the crossings and the Landscape cast are unexercised.

**What moved USN01: the point hold.** The first moved line is Northampton at step 1000: OFF steers
at a new goal 2470 m away, ON still at the previous one (`d32c` 1583 m) until the arm re-runs at
step 1020. The attackers target Mavis flying boats there (`other` in the latch census, 112 frames),
whose goal moves every frame. Per entity: the same five Mavis deaths on both sides, Mav2 at 74.15 ->
74.20 s, killer guns and hit splits shifted; Northampton dealt 1838 -> 1797, Salt Lake City 901 ->
942. No death flips. The known ship-avoidance refill counter also moved on LOMP10 and USN02.

**Verdict: OFF, recorded.** Prediction 4 named the retarget as the mechanism and it never fired;
the move came from the hold, which the section describes but did not predict. The contract keeps a
mechanism miss OFF. The binding stays in place: the hold is the image's behaviour (009F1E30 JE
009F2003) and the ring is exact.

**Open item (the lead, 2026-09-29): the no-ship hold of `nested+1228h` as its own switch.** Split
the hold (keep the frame's starting point on the no-ship path between arm runs) out of
`kShipAiApproachRetargetRingBound`, and judge it with predictions written first, on rows the ring
pair did not use (a mission where AI ships attack planes or buildings other than USN01, LOMP10 and
USN02). No multiplayer row.

## 28. The third ranking (packet `cc9_ship_ai_open_ranking_3`, cc9-ships10, 2026-09-28)

**It replaces section 16's table.** Every lane switch named in sections 17 to 27 is in this base.

**Source.** Ten rows run once on `agent/cc9-ships10` at `5a5cfd3b4`: main `9dcf7f537` plus this
branch's rank 4 flip (section 25) and the mode latch committed OFF (section 26, identical ON).
`build\win32\Release\bsp_game.exe` in that worktree, launch as section 16 (`local\ships10_run.ps1`),
logs `local\ships10_b0_<row>.log`. `local\ships10_census.py ships10_b0 rows <regex>` sums the
non-concrete host rows; `local\ships10_sites.py <row>` prints a record site. The rows are
section 16's ten, which include USN12 and JM05.

### Closed since section 16

| item | switch | where |
| --- | --- | --- |
| the turn clearance's path fade (rank 3) | `kShipAiClearancePathFadeBound`, ON | section 17 |
| the arm final's group area key (rank 10) | `kShipAiArmFinalAreaKeyBound`, ON (spread miss recorded) | section 18 |
| the troop-landing class trait (rank 2) | `kTroopLandingTraitBound`, ON | section 19 |
| the clearance outcome to the escape reader (new) | `kShipAiClearanceOutcomeWiringBound`, ON | section 20 |
| the group reference release (rank 5) | read only | section 21 |
| the heading wrap after a heading store (rank 7) | census, reach 1 | section 22 |
| the AutoTarget follower gate (rank 1) | `kAutoTargetFollowerGateBound`, ON | sections 12 and 24 |
| the AI command's avoid-zone point (rank 4) | `kAiCommandAvoidZonePointBound`, ON (spread miss recorded) | section 25 |
| the approach mode latch (part of rank 6) | `kShipAiApproachModeLatchBound`, ON (modes 1 to 4 unexercised) | section 26 |

Every switch in this lane is ON except `kShipAiApproachRetargetRingBound` (section 27, OFF by
verdict).

### The ranking

Reach as in section 1. Calls are the sum over the ten base rows.

| rank | item | image | host label | calls | reach, in one line |
| --- | --- | --- | --- | --- | --- |
| 1 | **the approach retarget arm, modes 0 and 2** (section 27) | `009F2124-009F272D` | `ShipAiApproach::frame_state_unread_spans`; the goal is copied every frame | 1743 USN01 + 601 LOMP10 entry frames | 3: a ship attacking a coastal command building steers at the building itself instead of a point 10 m off the coast that sees it |
| 2 | **the approach ring's sight test** | `00864FD0` (thunk onto `00864D90`) at `009E8116` in `009E7FC0` | `ShipAiApproach::zone_allows_target_00864680` answers true | 25067 (USN02 23525, USN01 1542) | 3: a hidden target stops mode 0's slot scoring. The label is stale: `kGunneryLineOfSightBound` is ON and the gunnery host runs `00864680`. Binding needs a public visibility query on the gunnery host |
| 3 | the group reference release | `00A2B8F0` from `00A2E784` | `AiGroups::release_group_reference` | 52930 (JM05 22079, JM08 15823, USN13 10026) | 3: section 21's next step |
| 4 | the carrier arm of the squadron exclusion | `009FFEB0` | `AiCommand::squadron_excluded_009ffeb0` | 15560 (JM05 8428, USN13 2079, JM08 1541) | 3 for a carrier's squadrons in an AI group. Borders the plane lane |
| 5 | the engage gate's kamikaze fields | `[class+510h]` / `+514h` at `009E85CD` in `009E85B0` | `ShipAiEngageGate::armament_readiness` answers 0 / 0 | 7546 (USN02 6951, USN01 437, LOMP10 155) | 3 only for Kaiten (`VehicleClass[4]`) and Shinyo (`[43]`), the two classes with `KamikazeDamage` in this installation's `vehicleclasses.lua` (mtime 2026-05-09). Exact for every other class. The "armament readiness" name is wrong (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md 2.1) |
| 6 | BigLandingShip | `class+808h` at `00827F95` | `ShipAiNeighbour::big_landing_ship_808` answers 0 | 1550 (JM08) | 3 when an enemy submarine is near an LSM or LST |
| 7 | a submarine target in the standoff choice | `009E6EFC` in `009E6E80` | `ShipAiApproach::target_kind_005c` answers false | 30105 | 0 on these rows: section 26 counts no kind-8 target. 3 on a submarine hunt |

**Not ranked, and why:**
- `ShipAi::unit_weapon_director`, `drive_heading_vtable50` and `ShipAiOrder::slot_to_order_ring`
  (982855 each) are structure, as in section 1.
- `ShipAiMoveOnPath::brain_leg_scale_0308` (15383) is a store with no reader; the value is kept.
- `ShipAiTorpedoStandoff::torpedo_bot_accuracy_008fb530` (13317) answers this installation's
  robots.lua values; the substitution is labelled, not open.
- `ShipAiApproach::sub_heading_command`, `sub_throttle_command` and `unit_depth_reference`
  (30105 each) belong to the submarine sub-states; section 26 counts no submarine approach.
- `ShipAiFollow::refresh_world_pose` (73578), `ShipAiApproachPoint::refresh_unit_pose`,
  `ShipAiApproach::scratch_00954940` and the release rows are housekeeping.
- The random stand-ins (`traffic_random`, `avoid_random`, `random_stream1`, `uniform_00bd2f10`)
  stay unbound for the RNG-stream reason in section 16.
- `AiCommand::tick_000c` (9829) is recorded only for the NonControl and Idle command types; the
  other types count as done.
- The AI party's power-up use `00A179E0` (461) waits on a power-up subsystem.

**Top item.** Rank 1 is section 27, in progress; it waits on the gunnery host's Landscape
query. Rank 2 is the next free packet once a gunnery-host visibility query exists; rank 5 is
free now but exact on every reference row.

## 29. Rank 5 of section 28: the engage gate's kamikaze fields (packet `cc9_engage_kamikaze_gate`, `kShipAiEngageKamikazeGateBound`)

Worker cc9-ships10, 2026-09-28, taken while section 27 waits on the gunnery host.

**The image.** `009E85B0`'s first conjunct reads `[class+510h]` and `[class+514h]` at `009E85CD`
and `009E85DD` and fails the gate when both are at most 0.0 (docs/ATTACKMOVE_ENGAGEMENT_RANGE.md
2.1). `BSP_ShipClass_ReadLuaFields` (`00831840`) stores them from the class table:
`KamikazeDamage` at `00831A22` and `KamikazeBlastDamage` at `00831A67`, each read through
`00B66330` with the default 0.0 (`FLDZ` at `00831A0A` and `00831A4F`).

**The binding.** `ShipAiEngageGate::armament_readiness` (the name is wrong and kept for the
interface) answers the two numbers from the unit's class row through the host's
`class_number`. Coverage: complete for the two reads. In this installation's
`vehicleclasses.lua` (mtime 2026-05-09) only Kaiten (`VehicleClass[4]`, 3000 / 3000) and Shinyo
(`[43]`, 3000 / 1500) carry them.

### Predictions, written before any ON run

1. **Every reference row is identical.** No base log names a Kaiten or a Shinyo, so
   `kamikaze_classes` is 0 on both sides and the gate fails as before.
2. **The reach is unmeasured.** A row with a Kaiten or a Shinyo under AI attackmove is needed to
   see the gate pass. None of the ten base rows is one.

### The row, and a second switch (cc9-ships11, 2026-09-29)

**USNOS.** This installation's `scripts/datatables/missiontree.lua` (mtime 2025-06-02, modded) has
the bonus mission `USNOS` ("New - Battle of the Osumi Islands", `COTP-USN/us_osumi.scn`); its
`scripts/missions/COTP-USN/us_osumi.lua` (mtime 2024-10-29) `PrepareClass(43)` / `PrepareClass(4)`
and `luaSpawnAttackers` (line 1338), run when phase 1's attacker list is empty, spawns nine
`Type 43` (Shinyo) 6300 m and six `Type 4` (Kaiten) 6100 m from the first troop ship, each told
`NavigatorAttackMove(unit, luaPickRnd(Mission.Troops))` (`luaShinyoSpawned`, `luaSubSpawned`, line
1593). A base run at main `3f1499210` (`local\s11_base_usnos.log`, `--frames 3200
--press-start-frame 30 --menu-select USNOS --mission-frames 3000 --mission-frame-seconds 0.05`)
logs the three `SpawnNew` groups and `engage kamikaze reads=990 kamikaze_classes=990`.

**The gate alone would stop the boats.** When `009E85B0` opens, `009E87E3` moves the selector to
the engage member `state+14C0h`, whose enter `009DB5E0` and step `009E23B0` were records: a boat
handed to it would no longer be steered by anyone. So the member is bound too, behind
`kShipAiEngageSubStateBound` (committed OFF): the enter clears `sub+8h` and stores 1225.0f
(`00D216E8`, 35 m squared) at `blk+234h` and `blk+29Ch`, which no modelled reader consumes
(labelled); the step is the existing `ship_ai_attackmove_engage_step_009e23b0` with
`EngageStepBinding` (velocities through the lead pursuit's `neighbour_world_velocity`
substitution, `009DE050` through `run_navigation_goal_009de050`, `009DA610` and `009DFF40` through
their reconstructions). Counters: `summary mission ship ai engage member enters / steps /
run_steps`. The two switches are paired together.

### Predictions for USNOS, written before any ON run

Rows: USNOS at 3200/3000 and 9200/9000 (the boats start about 6 km out; a Shinyo needs minutes to
come within the gate's 2000 m of its troop ship).
1. **OFF is identical to the base** apart from the new summary line and record rows.
2. **3000 frames: probably gameplay-identical.** The boats are unlikely to reach 2000 m of the
   troop ship in the 150 s; if `enters` is 0 the row is identical.
3. **9000 frames: the gate opens and the row moves (exit 3).** `enters` > 0 for Shinyo and Kaiten
   within 2000 m of their destination, `steps` > 0, and `run_steps` > 0 once a boat is within
   250 m with a latched goal (the run arm steers straight at the intercept point). The boats'
   paths change near the troop ships; which deaths change is not predicted.
4. **Mechanism failure:** `enters` = 0 on the 9000-frame row with a boat's nearest approach under
   2000 m, or boats that enter and then stop (the step not steering), keeps both switches OFF.
5. **Every other reference row is identical** (`kamikaze_classes` 0: no gate pass, no member).

### The pairs (cc9-ships11, 2026-09-29)

OFF is `86801174c` built in the tree; ON is `pair_export --commit 86801174c --flip
kShipAiEngageKamikazeGateBound=true --flip kShipAiEngageSubStateBound=true --out local\s11_kami`.
Logs `local\s11_koff_<row>.log` / `local\s11_kon_<row>.log`.

| row | frames | kamikaze reads | enters | steps | run steps | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USNOS | 3200/3000 | 990 | 0 | 0 | 0 | 1, gameplay identical | 2 held |
| USNOS long | 9200/9000 | 1776 OFF, 1762 ON | 1 | 20 | 0 | 3, moved | 3 held except the run arm |

OFF against the `3f1499210` base run is gameplay-identical (prediction 1).

**What moved.** One Shinyo, `unit #2.7`, came within 2000 m of TroopTrans4 (`d32c` 1797 m at
step 6040), entered the member and stepped it twenty times on the close arm (`009DA610` answered
false, so the run latch never set): throttle 1.0 and a live rudder, so it was steered, not
stopped. OFF it kept the approach member (`navigate_astern`). Per entity: the same 21 deaths on
both sides; `unit #2.7` died at 304.15 s instead of 313.70 s (killer Gear13 both sides, killer
range 1496 -> 1193 m, nearest Portland2 at 121 m instead of Portland1). Downstream: TroopTrans1
took 0 damage instead of 125, NH fired 3345 shots instead of 3761. No death flips.

**Verdict: both ON**, with a spread miss recorded: the run arm (`009E25BC..009E262F`, within
250 m and a latched goal) is unexercised, because the one boat that entered died at about 1.2 km.
The gate's avoid-zone conjunct is still the stand-in that answers no zone
(`ShipAiEngageGate::avoid_zone_list`); section 27 found `[class+570h]` = 0 in single player, whose
key-0 group has no zones, so the stand-in agrees with the image there.

## 30. Rank 3 of section 28: what replaces an attack on a destroyed group (packet `cc9_group_release_idle`, `kAiTargetGroupDestroyedIdleBound`)

Worker cc9-ships11, 2026-09-29. Section 21's next step: what the image does with a command whose
target group is freed.

**The image never leaves `command+1Ch` dangling.**
- `00A10710` (the ATTACK base constructor) stores the target at `+1Ch` and registers the command's
  observer sub-object `+8h` on that group: `MOV ECX,[ESP+1Ch]` (the target), `EDX = ESI+8`, `CALL
  00694A60` (BSP_Observer_RegisterPair, ECX = endpoint, EDX = callback owner) at `00A10767`.
- The four observer vtables are the base's `00D22B64` and the derived `00D22BA4` (MoveToAttack,
  `00A108AA`), `00D22BEC` (CautiousAttack, `00A109DC`), `00D22C2C` (CloseAttack, `00A10AFA`). Each
  has slot `+4h` = `00A10040` (disk bytes).
- `00A10040`, `__thiscall(observer)(subject)`, `RET 4`: when the subject equals `[observer+14h]`
  (`command+1Ch`), `new(8)`, `+4h = [observer-4]` (the command's group), vtable `00D229E0` (IDLE),
  then `00A2BD00` on that group (`00A2BD00` deletes the old command through its vtable slot 0 with
  1 and stores the new one at `group+564Ch`).
- The notifier is the group destructor: `00A2D8C0` -> `00A2D440` -> `00696330`
  (BSP_ObserverEndpoint_NotifySlot04). `ai_groups_compose_00a2e720` calls the host's
  `destroy_group` right after the release loop, so the timing is the same pass.

**The host rule it replaces.** `AiGroups::release_group_reference` reverted such a command to its
group's birth class (`initial_command_for`: NONCONTROL or IDLE). The image always installs IDLE.
The two differ only for a group whose birth class is NONCONTROL (an AI-disabled or negative party
slot). ON, `destroy_group` installs IDLE on every registry group whose command aims at the
destroyed group (`AiCommand::attack_target_destroyed_00a10040`), and the release loop only counts.
`00A2B8F0` itself stays a record (section 21: a score-list removal with no reader).

**OFF census** (`local\s11_goff_<row>.log`, this branch with the switch OFF): `target_releases` /
`target_releases_non_idle_birth` are 0 / 0 on JM05, JM08, USN13 (3200/3000), USN04 (4700/4500) and
USN01 (3200/3000), and 6 / 0 on USN02 (9200/9000).

### Predictions, written before any ON run

1. **Every row is gameplay-identical.** No measured release has a non-IDLE birth class, so ON
   installs the same IDLE command the OFF rule did, in the same compose pass.
2. **USN02 9000 is identical in its summary lines too**, apart from the switch field and a new done
   row (six `attack_target_destroyed_00a10040` calls).
3. **Reach**: a player-side or AI-disabled group given an ATTACK command whose target dies. No
   reference row has one, so the change is exact and unexercised.

### The pairs (cc9-ships11, 2026-09-29)

OFF `08aaaf7db` in the tree, ON `pair_export --commit 08aaaf7db --flip
kAiTargetGroupDestroyedIdleBound=true` (exe `69709265C0AC`); logs `local\s11_goff_<row>.log` /
`local\s11_gon_<row>.log`. USN02 9200/9000: exit 1, with the six `attack_target_destroyed_00a10040`
calls and the ship avoidance refill counter (known noise). JM08 3200/3000: exit 1, JM08's known
noise only. Predictions 1 and 2 held. **Verdict: ON.**

### Section 29 addendum: each flip alone (cc9-ships11, 2026-09-29)

- **The engage member alone** is identical by construction: `009E87E3` is the only way into
  `state+14C0h`, and it is reached only when `009E85B0` opens, which the gate switch keeps shut.
- **The gate alone** (`pair_export --commit 86801174c --flip kShipAiEngageKamikazeGateBound=true`,
  `local\s11_kgate_usnosl.log`, USNOS 9200/9000 against `local\s11_koff_usnosl.log`): exit 3.
  `enters=1 steps=161`, and every step is a record, so `unit #2.7` sits unsteered from about
  1.8 km (`navigate_astern`, throttle -0.625 to 0.5 in the step lines). Deaths go from 21 to 22:
  **TroopTrans1 dies only in this variant**. Damage goes from 4267.6 to 8330.2, and NH deals 2550
  instead of 434. That is why the two switches landed together.

## 31. Rank 2 of section 28: the approach ring's sight test (packet `cc9_approach_sight_test`, `kShipAiApproachSightTestBound`)

Worker cc9-ships11, 2026-09-29.

**The image.** `009E7FC0` has passed its range gates (`009E80DF`, `009E80FF`). Then:
- With a target (`ESI = [brain+0B20h]`, `009E810B`): `MOV ECX,[ECX+6DCh]; PUSH ESI; CALL 00864FD0`
  (`009E8116`). `00864FD0` is `MOV ECX,[ECX+68h]; JMP 00864D90`, the unit's own gunnery-pass
  visibility cache.
- Without one: `009E6120` copies the goal (`[brain+0B2Ch..0B34h]`) and `00864BA0` (`009E8130`)
  passes it by value to `00864680` on the same cache.
- `009E8137`: a false answer jumps to `009E82F1`, so no slot is scored this frame.

**The binding.** The host used to answer the target test true and the point test false (the goal
was a zero point). ON asks `GameGunneryHost::unit_sees_unit_00864d90`, which shares the pass's own
cache entry and TTL draw, and `unit_sees_point_00864680` with the goal (commits `264493a00`,
`739f19aae`). The point test is read-only, so it is asked on both sides. The target test writes
the cache, so it runs ON only.

**OFF census** (`local\s11_soff_<row>.log`): `point_tests` is 0 on USN02 9200/9000 and USN01
3200/3000. The no-target arm is not reached on either row: USN02's five no-target latch frames
leave `009E7FC0` at an earlier gate.

### Predictions, written before any ON run

1. **USN02 9200/9000 moves (exit 3).** `target_tests` is near section 28's 23525. `target_hidden`
   is above 0, because ships in the line screen each other and the query answers over units. Each
   hidden frame skips the ring scoring. The shared cache also changes when the TTL draws of the
   ship-target entries are taken.
2. **USN01 3200/3000 moves (exit 3)** through the shared cache: its attackers' targets (Mavis) are
   gunnery targets too. `target_tests` is near 1542.
3. **`point_tests` stays 0 on both rows.**
4. **Mechanism failure:** `target_tests` = 0 ON, or a move on a row where the ship AI never asked
   (tests 0). Either keeps the switch OFF.

### The pairs (cc9-ships11, 2026-09-29)

OFF is `a6b00c4b2` in the tree. ON is `pair_export --commit a6b00c4b2 --flip
kShipAiApproachSightTestBound=true --out local\s11_rt1`. Logs are `local\s11_soff_<row>.log` and
`local\s11_son_<row>.log`.

| row | target tests | hidden | point tests | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- |
| USN02 9200/9000 | 23525 | 0 | 0 | 1, gameplay identical | 1 missed |
| USN01 3200/3000 | 1542 | 345 | 0 | 3, moved | 2 held on the outcome, not on the cause |

- **USN02.** No ship target is ever hidden. The shared cache takes 77 more appends
  (`visibility_cache_append_00864d90` 3781 -> 3858), and no gameplay line moves. Prediction 1 said
  it would move.
- **USN01.** 345 of 1542 tests answer hidden, so the attackers skip the ring scoring on those
  frames and stand further off: CB2 and Dunlap's nearest approach goes 759 -> 954 m, the coastal
  guns' 1443..1493 -> 1737..1796 m, and shots go 1290 -> 1267. The same five deaths, no flips. The
  targets on those frames are the coastal buildings (1631 building latch frames).
- **Prediction 3 held** (`point_tests` 0 on both).

**Why the hidden answers are suspect.** `line_of_sight_00864680` swaps the image's roles (commit
`264493a00`): it casts from the target's raised point toward the observer and hides the target
when the first hit lies more than 25 m from the OBSERVER. The image casts from the observer's
raised point (`cache+14h`) toward the target's and measures the 25 m from the TARGET's point. The
host's segment query answers over units, so a cast that starts at a large building can hit the
building itself, far from the observer: hidden under the swap, visible in the image. So the one
move this pair shows may come from that swap and not from the image's sight test.

**Verdict: OFF, recorded.** Prediction 1 missed, and prediction 2's move has a different cause from
the one written. Next step: fix the role swap in the gunnery lane (its own pairs, since the gunnery
pass uses the same routine), then re-pair this switch on USN01 and USN02 with new predictions.

## 32. Handoff (cc9-ships11, 2026-09-29, at about 70% context)

**State of the lane.** Branch `agent/cc9-ships11`; no leases held. Switches:

| switch | state | section |
| --- | --- | --- |
| `kShipAiApproachRetargetRingBound` | OFF by verdict (the ring is exact but unreachable in single player: `[class+570h]` = 0) | 27 |
| `kShipAiEngageKamikazeGateBound` + `kShipAiEngageSubStateBound` | ON (paired together; the gate alone flips a death) | 29 |
| `kAiTargetGroupDestroyedIdleBound` | ON (`src/game_hosts_ai.cpp`) | 30 |
| `kShipAiApproachSightTestBound` | OFF by verdict | 31 |

**New gunnery-host queries** (commits `264493a00`, `739f19aae`):
`GameGunneryHost::unit_sees_unit_00864d90` and `unit_sees_point_00864680`.

**Open, in order:**
1. **The line-of-sight role swap** (gunnery lane). `line_of_sight_00864680` casts target ->
   observer and measures the 25 m from the observer. The image casts observer (cache owner,
   `[pass+50h]`, raised by Globals+94h) -> target (raised by +90h in `00864D90`) and measures from
   the target. Evidence is in commit `264493a00`. After the fix, re-pair section 31 on USN01 and
   USN02 with new predictions.
2. **The no-ship hold of `nested+1228h`** as its own switch (section 27 open item).
3. **Section 28 ranks 4, 6 and 7:** the carrier squadron exclusion `009FFEB0` (borders the plane
   lane, so ask the lead), BigLandingShip `class+808h` at `00827F95` (JM08), and a submarine target
   in the standoff choice.
4. **Stand-ins that agree with the image only in single player:** the engage gate's avoid-zone
   conjunct (`ShipAiEngageGate::avoid_zone_list` answers no zone), and the `[class+570h]` key
   generally.

**Useful files** in the cc9-ships11 tree: `local\s11_run.ps1 -Exe <exe> -Prefix <p> -Row
tag:MISSION:frames:mission_frames` (launches in the background; wait on the log's final COM
release line); logs `local\s11_*`. The new pair row is USNOS (section 29; launch line and mtimes
there).

### Rank 4 read (cc9-ships11, 2026-09-29): the carrier arm of `009FFEB0`, not bound

`009FFEB0-009FFF1D`, `__thiscall(squadron)`, plain `RET`, read whole from the disk bytes:
- `[00E17BF2]` set -> false (`009FFEB3`), as the host already has.
- `EDI = [[squadron+3D0h]+0C4h]`, the lead plane's class id (`009FFEC6`).
- `007EDAD0` (BSP_PlaneSquadron_AmmoType, ECX = the squadron) nonzero -> false (`009FFED8`): a
  squadron still carrying ordnance is never excluded.
- Otherwise the class id must be `10h`, `12h` or `11h` (`009FFEDC..009FFEE9`), i.e. level, dive or
  torpedo bomber; anything else -> false.
- `007F16D0(&out)` (BSP_Plane_ResolveReturnToBase, ECX = the squadron) at `009FFEF2`. A null
  `out` -> false. Otherwise `0077D600(squadron, out, &[ESP+10h], 1)` (BSP_Entity_IssueCommand,
  `009FFF09`) and true (`009FFF0F`).

So the arm sends a bomber squadron that has spent its ordnance home (the resolved `returntobase`
command) and keeps it out of the AI command's member orders.

**Why it is not bound in this lane:**
1. The host has no ordnance state. `squadron_ammo_type_stand_in` (`src/game_hosts_ai.cpp`) answers
   by class: 10h -> 5, 11h -> 2, 12h -> 1. So for exactly the three classes the arm admits,
   `007EDAD0` is never 0 and the arm cannot fire. A faithful binding needs a real "first ordnance
   kind still carried" reader over the squadron's planes (the release tasks know when a torpedo
   or bomb has dropped). That is plane-lane state.
2. The resolve (`record_return_to_base_007f16d0`, `bsp::resolve_return_to_base_007f16d0`) and the
   command issue live in `src/game_hosts_units.cpp`. The flown `land` task behind a resolved
   `returntobase` is bound only as far as docs/SQUADRON_LAND_TASK.md records (the landing states
   are refused).

**Next step:** a units/plane-lane packet exposing `squadron_ammo_type_007edad0(squadron)` from
the real ordnance state, plus a public `issue_return_to_base_007f16d0(squadron)` that resolves and
places the command the way the Lua `returntobase` path does. Then this host's
`tick_squadron_excluded_009ffeb0` can bind the arm behind its own switch, with predictions on the
rows with carrier strikes (JM05, USN13, JM08, which make 15560 calls between them per section 28).

## 33. The line-of-sight role swap (packet `cc9_los_role_swap`, `kGunneryLosRoleSwapBound`)

Worker cc9-ships12, 2026-09-29. Item 1 of section 32. The switch lives in `src/game_hosts_gunnery.cpp`.

**The image, re-read from the listing.**
- `00864D90` (`this` = the pass's visibility cache, argument = the target unit, `RET 4`): on a cache
  miss it copies the TARGET's pose +FCh..+104h (`00864DE8..00864E0C`), reads Globals +90h
  (`00864E17`), and when `vtable[5Ch](5)` (the unit base, ships and planes) and `[unit+538h]` are
  set adds class +A8h (`00864E96`; the section-span branch `00864E3F..00864E94` is labelled not
  taken). `00864EA4..00864ECE` push (x, y + raise, z) by value and call `00864680` (`00864ED3`).
- `00864680` (`this` = the same cache, three floats by value, `RET 0Ch`): `[cache+0]` is the
  cache OWNER, the observer. When the 0.5 s point cache (`[cache+10h]`, reset from `00CE3800`
  = 0.5 at `008647A4..008647B0`) has run out it stores the owner's pose +FCh..+104h at cache
  +14h..+1Ch (`008646F0..00864708`) and raises the y by Globals +94h, plus class +A8h under the
  same unit-base test (`00864710..008647B6`).
- `008647C3..008647F4`: `00904400(44h, &cache+14h, &passed point, &record, 0)`, callee-cleaned
  (no `ADD ESP` after `008647F4`). The segment runs OBSERVER -> TARGET.
- `008647FD..00864852`: (passed point - record point), squared and summed, against `00CFBC80`
  (625.0); greater hides. The 25 m is measured from the TARGET.

The host's `line_of_sight_00864680(observer, target)` did the reverse: target raised by +94h,
observer by +90h, cast target -> observer, 25 m from the observer. Both adds are 5.0 in this
installation (`kInstalledLosTargetHeightAdd`, `kInstalledLosViewerHeightAdd`), so ON changes
only the cast direction and the end the 25 m is measured from. Still labelled on both sides:
`0081DE10` (answers 0), the section-span raise, and the 0.5 s owner point cache (the host takes
the point at every test).

**The census.** `BSP_LOS_CENSUS=1` adds `summary mission gunnery line of sight landscape hits=`
and the first 24 hit lines (`  los landscape hit observer= target= from= to= hit= d_from= d_to=`).
It changes no gameplay line and is off unless set.

**OFF census** (this tree at the commit that adds the switch, `local\s12_off_<row>.log`, run with
the census on):

| row | tests | blocked | hits | where the logged hits lie |
| --- | --- | --- | --- | --- |
| USN01 3200/3000 | 190 | 36 | 36 | Dunlap -> CB2: 318..359 m from CB2, 627..964 m from Dunlap |
| JM05 3200/3000 | 874 | 42 | 42 | coastal gun US 01 -> Mogami-class 01: 244..277 m from the gun, 1028..1218 m from the ship |
| USN12 3200/3000 | 283 | 87 | 87 | Fortress-07 -> Shigure / Samidare / Shiratsuyu: 30.9..37.8 m from the fortress, 1060..1248 m from the ships |

Every hit blocks, and every logged hit is terrain well inside the segment: more than 25 m from
both ends.

### Predictions, written before any ON run

1. **USN01, JM05, USN12: blocked ON = blocked OFF, gameplay identical (exit 1).** The terrain is
   a height field, so a segment that crosses it one way crosses it the other way. ON reports the
   observer-side crossing, which is still far more than 25 m from the target.
2. **The ON hit lines move toward the observer:** the same observer/target pairs, with the hit
   point on the observer's side of the same terrain (for USN12 within about 40 m of the fortress,
   now measured from the ship, 1000+ m: still blocked).
3. **USN02 9200/9000 and JM06 3200/3000: hits 0 both sides, identical.**
4. **Mechanism failure:** a row where ON reports hits that OFF does not (or the reverse), or
   blocked changes on any row. That would mean the host's terrain march is one-sided (a start
   point below the surface), which is a trace property, not the image's sight test; the switch
   would then stay OFF and the march would be the next item.

If 1 to 3 hold, the switch flips ON: gameplay-identical and faithful to the listing, and the
ship AI's `unit_sees_unit_00864d90` (section 31) then asks the image's direction.

### The pairs (cc9-ships12, 2026-09-29)

OFF is this tree's build of `b89663ad5`; ON is `pair_export --commit b89663ad5 --flip
kGunneryLosRoleSwapBound=true --out local\s12_los` (bsp_game SHA-256 prefix `8119FA3EB6E2`). Logs are
`local\s12_off_<row>.log` and `local\s12_on_<row>.log`, both with `BSP_LOS_CENSUS=1`.

| row | tests OFF / ON | blocked OFF / ON | hits OFF / ON | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- |
| USN01 3200/3000 | 190 / 190 | 36 / 34 | 36 / 34 | 3, moved | 1 **missed** |
| JM05 3200/3000 | 874 / 874 | 42 / 42 | 42 / 42 | 1, gameplay identical | 1 held |
| USN12 3200/3000 | 283 / 283 | 87 / 87 | 87 / 87 | 1, gameplay identical | 1 held |
| USN02 9200/9000 | 3781 / 3781 | 0 / 0 | 0 / 0 | 1, gameplay identical | 3 held |
| JM06 3200/3000 | 432 / 432 | 0 / 0 | 0 / 0 | 1, gameplay identical | 3 held |

- **Prediction 2 held.** The ON hit lines are the same pairs with the hit on the observer's side:
  the OFF ray CB2 -> Dunlap (cast from Dunlap, hit 358.7 m from it) reappears ON as observer CB2,
  target Dunlap with the same 358.7 / 964.1 m, and Dunlap -> CB2 now hits 135..768 m from Dunlap.
  The only other summary moves on the identical rows are the landscape attach leaf and cell counts
  (the walk visits different cells when it starts from the other end) and the ship avoidance
  refill counter (known noise).
- **USN01 moved: Coastal Gun 01 fires 2 shots at Dunlap (36 damage, Dunlap 2400 -> 2311).** The
  death table is identical (5 rows, same times), and so is the plane death-mode table.
- **Why.** A census build of this tree (`local\s12_cen_usn01.log`, OFF, with the reverse cast
  asked alongside each test and not used) finds exactly two tests whose verdicts differ between
  the two conventions, both observer Coastal Gun 01, target Dunlap: cast from Dunlap the walk hits
  terrain 256.7 / 258.1 m from the gun (blocked), cast from the gun it hits nothing (visible). That
  is the cell test's one-sidedness: 00ADEB80 is Ericson's `IntersectLineQuad` form (scalar-triple
  signs, docs/SCENE_CONTENTS_HOSTS.md section 10.1), which only meets a quad from one side, and it
  is the image's own walk (`kTerrainSegmentQuadtreeBound` ON). So the image, casting gun ->
  Dunlap, sees Dunlap on those two tests.
- **Prediction 1's premise was wrong,** and so was prediction 4's rule: it assumed the host march
  was symmetric and that any asymmetry would be a host property. The asymmetric walk is the
  image's, reached through the listing's own cast direction.

**Verdict: ON.** The mechanism is the listing's (both points, the cast direction and the measured
end), four rows are gameplay-identical, and the one move is two tests the image's one-sided cell
test answers in the image's direction; no death flips. Recorded as a spread miss with the
mechanism matching. The census diagnostic stays in the code (`BSP_LOS_CENSUS=1`: the hit count,
the count of tests whose reverse cast would answer differently, and the first 24 of those).

**Still labelled on both sides:** `0081DE10` answers 0, the section-span raise is not taken, and
the 0.5 s owner point cache is not kept.

## 34. Section 31 re-paired on top of the role-swap fix (`kShipAiApproachSightTestBound`)

Worker cc9-ships12, 2026-09-29. Item 2 of the cc9-ships12 queue. The base is `2a684d17a`
(`kGunneryLosRoleSwapBound` ON). OFF is this tree's build. ON flips only
`kShipAiApproachSightTestBound`. Both sides run with `BSP_LOS_CENSUS=1`. Logs are
`local\s12_soff_<row>.log` and `local\s12_son_<row>.log`.

**What the census already shows about these rows** (section 33, gunnery-pass tests, now in the
image's direction). USN01 has terrain between Dunlap and CB2, and between Coastal Gun 01 and
Dunlap. JM05 has terrain 244..277 m in front of coastal gun US 01 toward Mogami-class 01. USN12 has
terrain 31..38 m in front of Fortress-07 toward three destroyers. Casting from a ship toward any
of those land units reaches that terrain more than 25 m from the land unit's point, so the answer
is hidden.

### Predictions, written before any ON run

1. **USN01 3200/3000 moves (exit 3).** `target_tests` is near 1542. `target_hidden` is above 0
   and **below section 31's 345**. The old cast started at the building's point; a start near or
   under the terrain surface counted the terrain beside the building as a block. From the ship,
   the one-sided cell test (section 33) meets that surface within 25 m of the building. What stays
   hidden is terrain that really lies between the two. The stand-off move of section 31 (CB2 and
   Dunlap 759 -> 954 m) shrinks.
2. **USN02 9200/9000 is gameplay-identical (exit 1)** with `target_hidden` 0, as in section 31.
3. **JM05 and USN12 move (exit 3) if their ships ask 009E7FC0 about the land units**, with
   `target_hidden` > 0 (the terrain above). If `target_tests` is 0 there, they stay identical.
4. **`point_tests` stays 0** on all four rows.
5. **Mechanism failure** keeps the switch OFF: `target_tests` is 0 on USN01, or a row moves where
   `target_tests` is 0.

### The pairs (cc9-ships12, 2026-09-29)

ON is `pair_export --commit faab22ed4 --flip kShipAiApproachSightTestBound=true --out
local\s12_sgt` (bsp_game SHA-256 prefix `D4A31E9E3430`).

| row | target tests | hidden | point tests | LOS tests OFF / ON | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- | --- |
| USN01 3200/3000 | 1542 | 345 | 0 | 190 / 192 | 3, moved | 1: the move held; "below 345" and "shrinks" **missed** |
| USN02 9200/9000 | 23525 | 0 | 0 | 3781 / 3858 | 1, gameplay identical | 2 held |
| JM05 3200/3000 | 0 | 0 | 0 | - | 1, gameplay identical | 3 held (no tests) |
| USN12 3200/3000 | 0 | 0 | 0 | - | 1, gameplay identical | 3 held (no tests) |

- **USN01 moves exactly as in section 31.** CB2 and Dunlap's nearest approach goes 759 -> 954 m, and
  Coastal Guns 01..03 go 1443..1493 -> 1737..1796 m. Dunlap fires 26 -> 11 shots and CB2 15 -> 7.
  Section 33's two Coastal Gun 01 shots on Dunlap go away again. The death rows and plane death
  modes are identical (5 rows each).
- **The swap was not the cause.** The hidden count is 345 with the listing's cast direction, the
  same as with the old one. The ship AI adds only 2 line-of-sight computations (190 -> 192); the
  other answers are cache entries from the gunnery pass. Dunlap's target is CB2
  (`command target 0071EBF0: unit=Dunlap token="CB2"`). CB2 is static at (3973.4, 32.0, -3182.2)
  in the raised frame. The census puts terrain between the two:
  - in the image's direction the first hit is 135..768 m from Dunlap and 555..964 m from CB2;
  - `reverse_verdict_differs=0` on the ON run, so both directions agree on every test.

  Section 31's suspicion that the old direction started inside the building does not apply.
- **Uncertainty.** A ridge about 25 m high, about 320 m in front of CB2, is the host's terrain:
  the height field, the island placement and the height of CB2's class. Those belong to their
  own packets. The sight test's mechanism is the listing's: `009E8116` asks the unit's own cache
  through `00864FD0`, and a hidden answer skips the ring scoring at `009E8137`.

**Verdict: ON.** The mechanism matches, three rows are identical, and the USN01 move is the
image's answer to terrain the image's own cast direction sees. No death flips. Prediction 1's
magnitude missed and is recorded. Section 31's reason for keeping it OFF (the role swap) is gone.

**Open item, for a terrain packet (the lead, 2026-09-29): the ridge in front of CB2 on USN01.** CB2
is static; its raised point is at (3973.4, 32.0, -3182.2), y 32. The host's height field puts a
ridge about 25 m high about 320 m out from it, toward Dunlap: the census hit points are at y
24..26, 318..359 m from CB2. That ridge hides CB2 from Dunlap on 345 of 1542 sight tests and
from the gunnery pass on 34 of 190. It needs checking against the island's authored height
field, its placement, and CB2's class height. It was not audited here.

## 35. The no-ship hold of `nested+1228h` on its own switch (packet `cc9_approach_no_ship_hold`, `kShipAiApproachNoShipHoldBound`)

Worker cc9-ships12, 2026-09-29. This is section 27's open item. The switch is in
`include/bsp/ship_ai_approach_update.hpp` and the host is `run_retarget_arm` in
`src/game_hosts_ship_ai.cpp`.

**The image, re-read.**
- `009F1DC2..009F1DDB`: ESI = `[brain+0B20h]`, kept only when `vtable[5Ch](6)` answers (a ship),
  else 0.
- `009F1E30 JE 009F2003`: with no ship target the frame skips both writers of `nested+1228h` on
  the ship path. Those are the zone point at `009F1EA2..009F1EB5` and the goal copy at
  `009F1F10..009F1F3D`.
- So on the no-ship path the point is written only by the retarget arm. `009F2124`'s head runs
  when `nested+11D6h` is clear and sets it; `009F1DAA` clears it when the `+11D8h` timer runs out.
  `009F23B5..009F23C5` store the goal before the zone lookup, and in single player the zone
  lookup ends the arm at `009F23F1` (section 27: `[class+570h]` = 0, the key-0 group has no
  zone).

**The binding.** ON, with the ring OFF, applies to no-ship frames in modes 0 and 2. The point
restores the frame's starting value, then the head runs:
- when it passes, the point is the goal (`hold_arm_runs`);
- otherwise the point holds (`hold_frames`; `frames_differ` counts held frames whose point is not
  the goal the host copied).

Still labelled: modes 3 and 4 (`009F21A0..009F2395`, unread) copy the goal every frame.
`kShipAiApproachRetargetRingBound` ON carries the same hold and overrides this switch. OFF copies
the goal every frame, as before. The summary line is
`summary mission ship ai approach no-ship hold arm_runs= frames= frames_differ= bound=`.

**Row census** (this tree at the commit below, 3200/3000, `local\s12_lc_<row>.log`). It looks for
rows where the arm is reachable, other than USN01, LOMP10 and USN02:

| row | latch frames | reachable (no-ship) | target kinds |
| --- | --- | --- | --- |
| IJN01 | 5930 | 5872 | 58 ship, 5872 other (the A7M fighters; PTs, LST1 and Curtiss shoot them down) |
| USNOS | 3960 | 0 | all ship |
| IJN05 | 24 | 0 | all ship |
| USN03, USN05, USN22, IJN02, IJN03, IJN08 | 0 | 0 | - |

### Predictions, written before any ON run

1. **IJN01 3200/3000 moves (exit 3).**
   - `arm_runs` is roughly reachable / 40 per unit-run: the head re-arms when the 2.0 s timer runs
     out. The expected count is between 100 and 300.
   - `frames` is the rest of the 5872, and `frames_differ` is most of them, because an aircraft
     goal moves every frame.
   - The ships steer for a point up to 2 s stale, so paths, AA ranges and hit splits move.
   - As on USN01 (section 27), I expect no death flip. A small change in plane death times and
     killers is likely, because the shared RNG couples them.
2. **USNOS 3200/3000 and IJN05 3200/3000 are identical** (exit 0 or 1): no no-ship frame. `frames`
   is 0 and `arm_runs` is 0.
3. **Mechanism failure** keeps the switch OFF. That is `arm_runs` = 0 on IJN01, `frames_differ` = 0
   with a move, or any move on USNOS or IJN05.

### The pairs (cc9-ships12, 2026-09-29)

ON is `pair_export --commit 85211f323 --flip kShipAiApproachNoShipHoldBound=true --out
local\s12_hold`. OFF is this tree's build of `85211f323`: `local\s12_hoff_ijn01.log`, and the
census logs `local\s12_lc_usnos.log` and `local\s12_lc_ijn05.log` (same build). ON is
`local\s12_hon_<row>.log`.

| row | arm runs | held frames | differ | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- |
| IJN01 3200/3000 | 709 | 5119 | 5119 | 3, moved | 1: the move held; the arm-run range and "no death flip" **missed** |
| USNOS 3200/3000 | 0 | 0 | 0 | 1, gameplay identical | 2 held |
| IJN05 3200/3000 | 0 | 0 | 0 | 1, gameplay identical | 2 held |

- **IJN01.** Every held frame differs from the goal copy: the A7M goals move every frame.
  - The latch's `retarget_entries` equals the arm runs (709 of 5828 reachable frames). OFF
    counts 5872 entries of 5872, because OFF never sets `nested+11D6h`.
  - 709 is above my 100..300. The flag is cleared more often than the 2..3 s timer alone would
    clear it; the likely clear is the attackmove enter reseed (`009F31C2..009F31ED` clears
    `+11D6h` and `+11D8h`) when a ship switches plane targets. That is not isolated. Also, the
    head's `009F214E` write (timer raised to 1.0) changes when `009F1DB4` draws from stream 1.
- **Deaths 24 -> 23: one flip.** A7M_1|.-5, killed OFF, survives ON. 17 more death rows move in
  time or killer, for example A7M_5|.-5 at 97.85 -> 107.20 s (PT3 -> Zeilin) and A7M_5|.-3 at
  102.45 -> 111.90 s (PT4 -> Curtiss). The ships steer for points up to one arm period stale, so AA
  ranges change (Downes moves 814 -> 787 m). No ship death moves.

**Verdict: ON.** The mechanism is the one named: a hold on no-ship frames only, arm runs storing
the goal, and nothing on rows without such frames. The misses are spread: the arm-run count, and
a plane death flip my prediction ruled out. Both are recorded. The hold is the listing's
behaviour (`009F1E30 JE 009F2003`). The death flip is a plane surviving AA whose ships steer at a
point up to one arm period old, which is what the image does on that path.

**Candidate reference row (the lead, 2026-09-29): IJN01 3200/3000.** It is the only row found
where ships attack planes on the no-ship path (5872 latch frames, A7M targets). Launch it like
the reference rows, with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`:

```
./tools/run_game.ps1 -Exe <exe> -Log local\<name>_ijn01.log -- --frames 3200 --press-start-frame 30 --menu-select IJN01 --mission-frames 3000 --mission-frame-seconds 0.05
```

## 36. Rank 4 of section 28: the squadron exclusion `009FFEB0`, read and counted (packet `cc9_squadron_rtb_exclusion`)

Worker cc9-ships12, 2026-09-29. No switch: the arm is not reached on any row counted.

**The image** (`009FFEB0..009FFF1D`, `__thiscall`, ECX = the squadron, plain `RET`). The whole
body was read:
- `009FFEB3`: `[00E17BF2]` set returns 0. It is 0 in the image.
- `009FFECD`: EDI = `[[sq+3D0h]+0C4h]`, the head plane's most-derived class id.
- `009FFED3`: `007EDAD0(sq)` (BSP_PlaneSquadron_AmmoType). It walks the `+3CCh` planes at
  `+3D0h`, and on each one tests through the weapon controller `007B91C0(kind, 1)`: `2Bh` -> 2,
  `2Ch` -> 3, `33h` -> 4, `31h` -> 5, `2Fh` -> 6, and `007B9320` (the general bomb) -> 1. It
  answers the first kind found, else 0 (`007EDB43`).
- Only when that answer is 0 AND the head's id is `10h`, `11h` or `12h` (the bomber leaves,
  `009FFEDC..009FFEE9`): `007F16D0(sq, &record)` resolves the squadron's `returntobase`
  (`include/bsp/return_to_base.hpp`).
- A non-null descriptor in `[record]` is issued as `0077D600(desc, &record+4, 1)` at `009FFF09`, and
  the routine answers 1: the member is excluded from the leader's moveto. Otherwise it answers 0.

So the "carrier arm" in section 28's label is a **return-to-base arm for a bomber squadron with
no ordnance left**. The host answers `007EDA90`'s value in its place.

**The census** (`src/game_hosts_ai.cpp`, `rtb_exclusion_census`, no behaviour). It counts the
calls that name a squadron, those whose head is a bomber leaf, and those where the ordnance test
answers 0. The test is `GameUnitsHost::unit_ordnance` over the planes still listed, a union mask
that drops clear, and it is labelled. The summary line is
`summary mission ai squadron rtb exclusion squadron_calls= bomber_calls= spent_calls=`. Rows at
3200/3000 are `local\s12_rc_<row>.log`:

| row | squadron calls | bomber calls | spent |
| --- | --- | --- | --- |
| USN13 | 416 | 0 | 0 |
| LOMP10 | 2 | 0 | 0 |
| JM05, JM08, USN04, USN01 | 0 | 0 | 0 |

Section 28's call counts (JM05 8428, USN13 2079, JM08 1541) are inflated: the host evaluates
`tick_squadron_excluded_009ffeb0` for every follower, ships included, while the image asks only
after the squadron test. On these rows every squadron that follows a leader is a fighter group.

**Decision: not bound.** A binding needs three things:
- the ordnance reader above;
- a units-host entry that resolves `007F16D0` for a squadron and places the resolved command
  (the units lane: `record_return_to_base_007f16d0` / `rtb_census` exist but are private);
- a row where an AI group's bomber squadron spends its load while following a leader.

None of the counted rows has one.

## 37. Rank 6 of section 28: BigLandingShip, `class+808h` (packet `cc9_big_landing_ship`, `kShipAiBigLandingShipBound`)

Worker cc9-ships12, 2026-09-29. The switch is in `include/bsp/game_hosts_ship_ai.hpp`.

**The image.** `00827F70` is a class method:
- TorpedoBoat `0Eh` answers small.
- LandingShip `0Ch` answers small only when the byte at `class+808h` is 0 (`00827F95`).
- Everything else answers not small.

The byte is `VehicleClass[type].BigLandingShip`, read exact-Boolean-or-false by the LandingShip
leaf (`0074C630`). The same boolean picks that leaf's tuning pair (`"BigLandingShip true"`, scalar
source `20h`, `src/vehicle_class_lua_load.cpp`). In this installation's `vehicleclasses.lua`
(mtime 2026-05-09) it is true for `VehicleClass[12]` (LSM), `[41]` (US LST), `[91]` (IJN LST) and
`[345]` (US LST, strafeable).

`00827F70` has fourteen call sites (`ghidra xrefs`). This lane owns four:
- the neighbour admission `009F0D82` (a small ship admits an enemy submarine as an obstacle);
- the approach mode latch `009F1F76` (a small ship holds against a submarine target);
- the AI bullet accuracy group `009FE2D4..009FE67C` (small ship or big ship);
- the capture weight `00A0360B` (0.1 when small, else 1.0, `JZ 00A03636`).

Not bound here: `007EEB74` (BSP_Unit_AttackCommandApplies), `0081639D`
(BSP_Entity_CommandAvailableAgainstTarget), `0096ACB4`, and the standoff choice `009E6F11`
(rank 7, behind a target-kind stub).

**The binding.** The ship AI host records the byte at load, when the depth reader selects the
"BigLandingShip true" pair for a class-`0Ch` unit. It logs `unit big landing ship unit= type_id=`
and serves the byte through `GameShipAiHost::unit_big_landing_ship_0808`. ON, the four sites read
it: the AI host reaches it through `units.ship_ai()`. OFF, every landing ship is small, as before.

**OFF census** (this tree, `local\s12_blsoff_<row>.log`):
- JM08 has five big landing ships: LSM 01, LSM 02 (type 12) and LST 01..03 (type 41).
- IJN01 has eleven of type 345.
- JM08 has no submarine, and IJN01's latch counts 0 submarine targets.

### Predictions, written before any ON run

1. **JM08 3200/3000 moves (exit 3).** The Japanese AI's accuracy against the five landing ships
   switches from the small-ship to the big-ship offsets. Their capture weight rises 0.1 -> 1.0,
   which changes the planner's arrival values and group targets. Hits on the LSTs and LSMs move;
   a death flip among them is possible.
2. **IJN01 3200/3000 moves (exit 3)** the same way, through the A7Ms' accuracy against the eleven
   US LSTs and their capture weight.
3. **The admission and latch sites change nothing on either row**: there is no submarine.
4. **JM06 3200/3000 and USN12 3200/3000 are identical** (exit 0 or 1): no landing ship.
5. **Mechanism failure** keeps the switch OFF: no `unit big landing ship` line on JM08, or any
   move on JM06 or USN12.

### The pairs (cc9-ships12, 2026-09-29)

ON is `pair_export --commit b03cddfbc --flip kShipAiBigLandingShipBound=true --out local\s12_bls`.
OFF is `local\s12_blsoff_<row>.log`, ON `local\s12_blson_<row>.log`.

| row | admission reads (`big_landing_ship_808`) | AI-site big reads | `pair_diff` | prediction |
| --- | --- | --- | --- | --- |
| JM08 3200/3000 | 1550, concrete ON | 0 | 1, gameplay identical | 1 **missed** |
| IJN01 3200/3000 | 4588, concrete ON | 200 | 1, gameplay identical | 2 **missed** |
| JM06 3200/3000 | 0 | 0 | 1, gameplay identical | 4 held |

- **The admission is reached, and prediction 3 held.** ON, each landing ship's admission runs the
  enemy-submarine test that small ships skip (`unit_is_kind_vtable5c` 3972 -> 5522 on JM08, 16923
  -> 21511 on IJN01). No submarine exists, so no node changes.
- **The AI sites.** Counted afterwards with `summary mission ai big landing ship reads=` in a
  census build of the same tree (`local\s12_blscen_<row>.log`, gameplay-identical to the OFF
  logs). JM08 asks 0 times: no AI accuracy or capture read names its landing ships. IJN01 asks 200
  times, and the ON run still chooses and fires identically. So a 1.0 capture weight and the
  big-ship accuracy group do not change a choice on that row.
- Predictions 1 and 2 expected those reads to move the rows; they missed.

**Verdict: ON.** It is exact to the listing (`00827F95` on the Lua byte), gameplay-identical on
all three rows, and its mechanism is reached at the admission site. The four other callers of
`00827F70` (`007EEB74`, `0081639D`, `0096ACB4`, and the standoff `009E6F11` behind rank 7's
target-kind stub) still answer "small" for a big landing ship.

## 38. Handoff (cc9-ships12, 2026-09-29, at about 70% context)

**State of the lane.** Branch `agent/cc9-ships12`; no leases held. Switches this worker touched:

| switch | state | section |
| --- | --- | --- |
| `kGunneryLosRoleSwapBound` (`src/game_hosts_gunnery.cpp`) | ON (USN01 moves by two one-sided cell tests; four rows identical) | 33 |
| `kShipAiApproachSightTestBound` | ON (re-paired; USN01 moves as in section 31, three rows identical) | 34 |
| `kShipAiApproachNoShipHoldBound` | ON (IJN01 moves, one plane death flip; USNOS and IJN05 identical) | 35 |
| `kShipAiBigLandingShipBound` | ON (identical on JM08, IJN01 and JM06) | 37 |
| rank 4, `009FFEB0` | read and counted, not bound: the arm is not reached on six rows | 36 |

**Expect reference moves** from 33, 34 and 35: USN01 (the sight test and the hold, as in sections
27 and 31), IJN01 (the hold), and LOMP10/USN02 wherever the hold reaches.

**Diagnostics left in the code:**
- `BSP_LOS_CENSUS=1`: landscape hits, the count of tests whose reverse cast would answer
  differently, and the first 24 of those.
- `summary mission ship ai approach no-ship hold`.
- `summary mission ai squadron rtb exclusion`.
- `summary mission ai big landing ship reads`.
- `unit big landing ship` at load.

**Open, in order:**
1. **Rank 7 and a wider stub in the same host.** `StandoffBinding::target_is_kind_vtable_005c`
   (`009E6E80`) answers false for EVERY kind, not only kind 8, so the building (`1Ch`) arms of
   modes 4 and 2 (`009E6F29..`, `009E700B..`) never take the target's `+7C4h` / `+7A0h`. Those two
   reads (`target_radius_07c4`, `target_gun_range_07a0`) are records answering 0 as well. Binding
   kind 8 alone has no reach on any row counted: no row has a submarine target. The `1Ch` arms
   need modes 2 or 4. The latch census shows mode 0 on every row counted, except USN01's buildings
   of the other side, which are mode 0 too. So find a row with modes 2 or 4 before binding.
2. **The other `00827F70` callers** (`007EEB74`, `0081639D`, `0096ACB4`) still class a big landing
   ship as small. They belong to the units, commands and script-order lanes.
3. **Rank 4's binding is unblocked** (the lead, 2026-09-29; main merge `2eb7e0a9c`,
   docs/SQUADRON_ORDNANCE_STATE.md, `kSquadronOrdnanceReaderBound` ON). Two new
   `GameUnitsHost` entries replace the census's union-mask test:
   - `squadron_ammo_type_007edad0(unit)` takes the squadron or any member, and answers 0 when
     nothing is carried.
   - `issue_return_to_base_007f16d0(unit, source)` places `returntobase` on each member plane
     through the Lua path, including `007F16D0`, and returns the planes placed.

   Plan:
   - Bind `tick_squadron_excluded_009ffeb0` behind a new switch, committed OFF. The answer is
     true when the head class id is `10h`, `11h` or `12h`, the ammo type is 0 and the issue
     placed at least one plane.
   - Keep `rtb_exclusion_census` for the reach counts.
   - No squadron runs dry within 3000 frames on JM05, USN13 or JM08, so pair on 9000-frame rows
     (or a row where bombers do drop). Check `bomber_calls` > 0 first; section 36 counted 0 on
     six rows at 3000.
4. **The terrain in front of CB2 on USN01** (section 34): a ridge about 25 m high, about 320 m out,
   hides Dunlap's target. Check it against the island's height field if USN01's stand-off
   distances look wrong.
5. From section 32, still open: the engage gate's avoid-zone conjunct, and `[class+570h]` in
   multiplayer.

**Useful files** in the cc9-ships12 tree: `local\s12_run.ps1 -Exe <exe> -Prefix <p> -Row
tag:MISSION:frames:mission_frames`. It launches in the background with `BSP_LOS_CENSUS=1` as well
as the reference variables; wait on the log's final COM release line. The logs are `local\s12_*`.

## 39. Section 38 item 1: the standoff's target kind (packet `cc9_standoff_target_kind`, `kShipAiStandoffTargetKindBound`)

Worker cc9-ships13, 2026-09-29. The switch is in `include/bsp/game_hosts_ship_ai.hpp`.

**The image** (`009E6E80`, read at `009E6EF0..009E7095` with `disasm-raw`):
- `009E6EF2` loads the target, `[brain+0B20h]`; a null target skips every kind query.
- `009E6F01` asks `vtable[5Ch](8)`. When that is true, `009E6F11` calls `00827F70` with ECX =
  `[brain+0AACh]`, the unit's own class, the same ECX as the latch's call at `009F1F76`. A false
  answer (not small) disables the standoff (`JE 009E6EA6`).
- Mode 4 (`009E6F29`): `vtable[5Ch](1Ch)` at `009E6F3A`; true takes `FILD [target+7C4h]` minus
  300.0 (`009E6F4E`, `00CE3CA8`), else 1000.0 (`00CE47A0`).
- Mode 2 (`009E700B`): `vtable[5Ch](1Ch)` at `009E701C`; true takes `00419010(...)` times
  `FIMUL [target+7A0h]` (`009E706F`) and `FILD [target+7A0h]` times the high factor (`009E7087`).
- `1Ch` is MCommandBuilding. `006F2780` fills `+7A0h` from `CaptureRange` and `+7C4h` from
  `LandingRange` (key `00CFAE30`, `006F2847..006F285F`), both with the default 500
  (docs/SENTITY_INIT_PASSES.md). In this installation `universe/library/commandbuilding.props`
  (mtime 2024-07-13) has `LandingRange = I 500`, and `ijn_07_invasion_of_midway.scn` authors
  `I 2000` and `I 2100`.

**The binding.** ON:
- The three kind queries answer the target's kinds (`unit_is_kind_of` on `raw_target_0b20 - 1`).
- `00827F70` answers the latch's expression: kind `0Eh`, or kind `0Ch` without BigLandingShip.
- Mode 2 reads `command_building_capture_range_07a0`.
- LABELLED: the units host has no `LandingRange` field. So while the latched mode is 4, the `1Ch`
  query still answers false and the arm keeps 1000.0. It is counted as `building_mode4_deferred`.
  The accessor is routed to the units lane (`command_building_landing_range_07c4`, the same shape
  as the capture accessor).

OFF, every kind answers false, as before. The census counters run in both states. The summary
line is `summary mission ship ai standoff target kind calls= kind_08= building_mode2=
building_mode4_deferred= small_class=`.

**Other stubs in the same binding, not changed:** `unit_is_group_leader_00778890` answers false,
`unit_cruise_speed_0490` answers 0, and `random_stream1_00bd2f10` answers `low` (the draw is not
made, so the shared stream is not advanced).

### The census: no row reaches modes 2 or 4

The latch summary (`summary mission ship ai approach latch`, section 26) counted on this tree's
OFF build (`local\s13c_<row>.log` at 3200/3000, `local\s13l_<row>.log` at 9200/9000). Launch
lines are the reference rows' (`--press-start-frame 30 --menu-select <M> --frames F
--mission-frames MF --mission-frame-seconds 0.05`, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`).
The candidates are this installation's landing and invasion missions (`missiontree.lua`, mtime
2025-06-02):

| row | frames | latch frames | ship / building / other target | modes 0/1/2/3/4 |
| --- | --- | --- | --- | --- |
| JM07 (Invasion of Midway) | 3000 | 0 | - | 0/0/0/0/0 |
| JM12 (Seizing the Fijis) | 3000 | 41 | 41 / 0 / 0 | 41/0/0/0/0 |
| JM13 (Wake Island) | 3000 | 0 | - | 0/0/0/0/0 |
| IJN10 (Port Moresby) | 3000 | 27 | 27 / 0 / 0 | 27/0/0/0/0 |
| IJN17 (Guadalcanal) | 3000 | 80 | 80 / 0 / 0 | 80/0/0/0/0 |
| USN06 (Attack on Guadalcanal) | 9000 | 0 | - | 0/0/0/0/0 |
| USN20 (Invading Iwo Jima) | 3000 | 0 | - | 0/0/0/0/0 |
| JM05 (Invasion of Port Moresby) | 9000 | 13 | 13 / 0 / 0 | 13/0/0/0/0 |
| IJN05 (Andaman Islands) | 9000 | 24 | 24 / 0 / 0 | 24/0/0/0/0 |
| JM16 (Invasion of Hawaii) | 9000 | 24455 | 21543 / 0 / 2896 | 24455/0/0/0/0 |

Every existing latch line in the other trees' logs (387 logs, sections 26 to 38 rows) is mode 0
too. The only building targets are USN01's, of the other side, with no lander. No row has a
submarine target either, so the kind-8 arm has no reach. The landers on the invasion rows never
enter the approach with a building target: the approach is not where this host lands troops.

### Predictions, written before any ON run

1. **Zero reach on every row counted:** `kind_08`, `building_mode2` and
   `building_mode4_deferred` are 0 on both sides of every pair.
2. **USN01 3200/3000 is gameplay identical** (exit 0 or 1). Its building targets are enemy
   buildings and the unit is no lander, so the latch gives mode 0 and neither `1Ch` arm runs.
3. **JM16 3200/3000 is gameplay identical**: ship and other targets only.
4. **Mechanism failure** keeps the switch OFF: any nonzero reach counter, or a move on either row.

### The pairs (cc9-ships13, 2026-09-29)

OFF is this tree at `f72fb98a5` (`local\s13koff_<row>.log`); ON is `pair_export --commit f72fb98a5
--flip kShipAiStandoffTargetKindBound=true --out local\s13_k` (`local\s13kon_<row>.log`). A
300-frame USN01 smoke ran first (`local\s13koff_smoke.log`, final COM release).

| row | kind calls | kind_08 / building_mode2 / building_mode4_deferred / small_class | `pair_diff` | prediction |
| --- | --- | --- | --- | --- |
| USN01 3200/3000 | 3784 both | 0 / 0 / 0 / 0 both | 1, gameplay identical | 1, 2 held |
| JM16 3200/3000 | 1740 both | 0 / 0 / 0 / 0 both | 1, gameplay identical | 1, 3 held |

The only moved lines are the switch's own `bound=` field and the ship avoidance refill counter
(known noise: 107 -> 100 on USN01, 1222 -> 1236 on JM16).

**Verdict: ON.** The binding is read whole except the mode-4 `+7C4h` read, which stays deferred
and keeps the OFF answer, so ON is never further from the image than OFF. Both rows are gameplay
identical and the reach is zero, as predicted. When the units lane lands
`command_building_landing_range_07c4`, drop the mode-4 guard in `StandoffBinding` and re-pair on a
row with a lander (none is known yet).

## 40. Rank 4, bound: the squadron exclusion `009FFEB0` (packet `cc9_squadron_rtb_exclusion_bind`, `kAiSquadronRtbExclusionBound`)

Worker cc9-ships13, 2026-09-29. The switch is in `include/bsp/game_hosts_ai.hpp`. Section 36 read
the body whole (`009FFEB0..009FFF1D`).

**The binding.** It uses the units-host entries from main `2eb7e0a9c`
(docs/SQUADRON_ORDNANCE_STATE.md):
- `squadron_ammo_type_007edad0(index)` answers `007EDAD0`, 0 when nothing is carried.
- `issue_return_to_base_007f16d0(index, source)` places `returntobase` (`00E08F98`) on each member
  plane through the path a Lua `returntobase` takes. That path is `007F16D0`'s resolution and
  `0077D600`'s delivery with flags 1.

ON, `tick_squadron_excluded_009ffeb0` runs the arm:
- It applies to a squadron whose head plane's class id is `10h`, `11h` or `12h` and whose ammo
  type is 0.
- It issues `returntobase` and answers true when an order was placed. That is `009FFF09`; a null
  descriptor answers 0.
- Every other call answers false.

OFF, it answers `007EDA90`'s value, the stand-in used before. The census counts run in both
states: `summary mission ai squadron rtb exclusion bind stand_in_true= issues= excluded=`, next to
section 36's `squadron_calls= bomber_calls= spent_calls=`.

**The two ways ON differs from OFF:**
- The arm, where a spent bomber squadron follows a leader.
- Calls where the `007EDA90` stand-in answered true: ON they answer false, as the image does.
  They are counted as `stand_in_true`.

### The census (this tree, OFF, 9200/9000)

`local\s13r_<row>.log`, plus `local\s13l_usn06.log` from section 39's census:

| row | squadron calls | bomber calls | spent | stand-in true |
| --- | --- | --- | --- | --- |
| USN13 | 2037 | 576 | 0 | 0 |
| USN06 | 971 | 800 | 0 | 0 |
| IJN01 | 17 | 17 | 0 | 0 |
| IJN10 | 2 | 2 | 0 | 0 |
| USN04 | 0 | 0 | 0 | 0 |

The ordnance reader's census (`squadron ordnance <name>: ammo type N -> 0`) finds four spent
squadrons: USN04's B5N Kate #2.1 and #6.1 at 127.80 s and 207.41 s, and JM16's Bogue-class 04
sqn12 and sqn18 at 247.06 s and 247.86 s (`local\s13l_jm16.log`). None of them follows a leader in
an AI group: USN04 and JM16 make no squadron call at all. So no row reaches the arm.

### Predictions, written before any ON run

1. **USN13 and IJN01 at 9200/9000 are gameplay identical** (exit 0 or 1), with `issues=0`,
   `excluded=0` and `stand_in_true=0` on both sides.
2. **Mechanism failure** keeps the switch OFF: a nonzero `stand_in_true` or `issues`, or any move.
3. If both are identical, the binding may flip as exact with zero reach. The body is read whole,
   and both of its answers match the image.

### Section 38 item 2: the other `00827F70` callers, read (no edit in this lane)

- **`007EEB74`** is in `BSP_Unit_AttackCommandApplies` (`007EE8F0..007EEBFD`), in the kamikaze
  arm `007EEB36..007EEB91`:
  - ECX is `[target+538h]`, the target's class.
  - When the target is a ship (`vtable[5Ch](6)`, `007EEB64`) and `00827F70` answers small,
    `00604A50([unit+3D0h])` must answer true, or the arm rejects (`007EEB8A JE 007EEBF6`, `XOR AL,AL`).
  - `00604A50` (`00604A50..00604A74`, `RET` then `INT3`) is `vtable[5Ch](17h) && byte [+C24h] == 0`:
    a kamikaze plane whose authored `PilotFires` is clear. That is `007EDA90`'s shape.
  - In the reconstruction this is `AttackFeasibilityInputs::kamikaze_ship_blocked`
    (`include/bsp/attack_commands.hpp`). `src/game_hosts_script_orders.cpp` never sets it, so the
    kamikaze arm never rejects a small ship.
  - The edit belongs to the script-orders lane: `blocked = small(target) && !(slot-0 plane kind
    17h && !PilotFires)`, where small is kind `0Eh`, or kind `0Ch` without BigLandingShip.
- **`0081639D`** is in `BSP_Entity_CommandAvailableAgainstTarget` (`008162B0..00816408`). Its
  reconstruction, `src/ship_ai_states.cpp` (`host.call_00827f70()`), has no game-host
  implementation: the routine is not bound in the executable, so there is nothing to edit yet.
- **`0096ACB4`** is in an unnamed method `0096AC60..0096ACD3` (`__thiscall`, `RET 4`):
  - It is slot 5 of the vtable at `00D1B2CC`. The object is built by `009766D0`, which is called
    by `BSP_WarningManager_LoadEventTable` (`00980380`).
  - For an event whose `vtable[10h]()` answers 3, it takes the entity at `[event+6Ch]`. It admits
    a squadron (`18h`), a plane (`0Fh`) or a small ship (kind 6 and `00827F70` on `[+538h]`), then
    compares the event name through `0096AA40`.
  - Nothing reconstructs it and no lane owns it. It is a warning-event filter.

### First pair: a mechanism failure in the binding (cc9-ships13, 2026-09-29)

ON was `pair_export --commit 6ddde4cc6 --flip kAiSquadronRtbExclusionBound=true --out local\s13_r`,
logged to `local\s13ron_<row>.log`. OFF was `local\s13r_<row>.log`.
- USN13 and IJN01 were both gameplay identical (exit 1).
- But `issues=576` on USN13 and `issues=17` on IJN01, one per bomber call, with `excluded=0`.
- The arm had passed the AI host's squadron index to the units host. That index lies past
  `units.count()` (`squadron_of`), while the units host keys a squadron by its registry unit or a
  member plane (`squadron_record_of`). So the ordnance reader found no record and answered 0 on
  every call, and the issue found none either.
- Prediction 2 caught it. The fix passes the head plane `[sq+3D0h]`, which `009FFECD` reads
  anyway.
- The OFF side needs no rerun: OFF never calls the arm.

### Second pair (cc9-ships13, 2026-09-29)

ON is `pair_export --commit d009efe4e --flip kAiSquadronRtbExclusionBound=true --out local\s13_r2`
(`local\s13ron2_<row>.log`). OFF is still `local\s13r_<row>.log`.

| row | bomber calls | stand_in_true / issues / excluded (ON) | `pair_diff` | prediction |
| --- | --- | --- | --- | --- |
| USN13 9200/9000 | 576 both | 0 / 0 / 0 | 1, gameplay identical | 1 held |
| IJN01 9200/9000 | 17 both | 0 / 0 / 0 | 1, gameplay identical | 1 held |

The only moved lines are the switch's `bound=` field and, on IJN01, the free-bearing scan's
`empty=` / `refills=` pair (0 -> 817, 58 -> 57). That pair is the ship avoidance refill noise:
every other counter on the line is equal, and it moves the same way on two same-source OFF logs
in the cc9-ships12 tree (`empty=0` on three IJN01 logs, `empty=137` on four).

**Verdict: ON, exact with zero reach.** The body is read whole (section 36). Both answers match the
image: the arm, and 0 for every other call. The reach is zero on every row counted, and no call
on those rows has the `007EDA90` stand-in answering true. The arm's first reach needs a bomber
squadron that follows a leader in an AI group and spends its load. USN13 has 576 bomber calls
whose squadrons never spend it.

## 41. Section 39's mode-4 arm bound: LandingRange `[target+7C4h]` (packet `cc9_standoff_landing_range`)

Worker cc9-ships13, 2026-09-29. It lands under the switch that is already ON,
`kShipAiStandoffTargetKindBound`.

- The units lane landed `GameUnitsHost::command_building_landing_range_07c4` (main `a19a551ba`,
  merge `52f5a0f71`). It answers `006F2780`'s `LandingRange` store at `006F285F`, and 500.0 for a
  unit that is not kind `1Ch`.
- `StandoffBinding` no longer answers false for the `1Ch` query in mode 4.
- `target_radius_07c4` (`009E6F4E` `FILD`) returns that accessor's value truncated to `int`. The
  field is an authored integer: `I 500` in `commandbuilding.props`, `I 2000` / `I 2100` in
  `ijn_07_invasion_of_midway.scn`.
- Mode 4's base is now `LandingRange - 300.0` for a building target, as the listing has it.
- The summary field `building_mode4_deferred=` is renamed `building_mode4=`.

**Evidence of no move.** Section 39's census found no mode 4 frame on any of its ten landing and
invasion rows, or in 387 earlier logs. So the arm has zero reach, and section 39's pairs stand as
they are. The lead accepted a build plus those pairs.
- The build passes.
- A 300-frame USN01 smoke (`local\s13m4_smoke.log`) reaches the final COM release.

The latch's own `+7C4h` read (`009F20A4`, mode 3 vs 4) is not bound. It needs `006F2D90`, the
free-landing-spot test, as well, and it has the same zero reach.

## 42. Section 38 item 2, first caller: the kamikaze arm's small-ship test `007EEB74` (packet `cc9_kamikaze_ship_blocked`, `kKamikazeShipBlockedBound`)

Worker cc9-ships13, 2026-09-29. The switch is in `include/bsp/game_hosts_script_orders.hpp`.

**The image** (`BSP_Unit_AttackCommandApplies`, `007EE8F0..007EEBFD`, the kamikaze arm
`007EEB36..007EEB91`, read with `disasm-raw`). After the kamikaze-capable test
(`vtable[5Ch](17h)` on `[unit+3D0h]`, `007EEB53`):
- `007EEB64`: the target answers `vtable[5Ch](6)`, a ship.
- `007EEB6E`: ECX is `[target+538h]`, the target's class, and `007EEB74` calls `00827F70` on it.
- When both are true, `007EEB83` calls `00604A50([unit+3D0h])`, and a false answer rejects
  (`007EEB8A JE 007EEBF6`: `XOR AL,AL`, `RET 10h`).
- `00604A50` (`00604A50..00604A74`, `RET` then `INT3`) answers
  `vtable[5Ch](17h) && byte [plane+C24h] == 0`. `+C24h` is the plane's `PilotFires` (`007CD930`).

So a kamikaze attack on a small ship (TorpedoBoat `0Eh`, or LandingShip `0Ch` without
BigLandingShip) is allowed only for a kamikaze plane whose `PilotFires` is clear.

**The reconstruction** already has the input, `AttackFeasibilityInputs::kamikaze_ship_blocked`
(`include/bsp/attack_commands.hpp`), and `kamikaze_applies` tests it. Its one filler,
`GameScriptOrdersHost`'s PilotSetTarget path, never set it. ON, it sets
`blocked = small(target) && !(plane kind 17h && !PilotFires)`:
- `small` is the expression the ship AI sites use: kind `0Eh`, or kind `0Ch` without
  BigLandingShip, the latter through `GameShipAiHost::unit_big_landing_ship_0808`.
- PilotFires comes from `GameUnitsHost::plane_pilot_fires_0c24` (units lane).

The census counts `small`, `blocked`, and the choices it changes. That covers both switch states.

### The census (this tree, OFF, 3200/3000)

`PilotSetTarget caps` lines with `kamikaze_capable=1`, `local\s13kz_<row>.log`:

| row | PilotSetTarget calls | from kamikaze-capable planes | target classes |
| --- | --- | --- | --- |
| USN19 (Battle of Ormoc Bay) | 33 | 33 (self class 23) | 9 (24), 11 (9) |
| USN17 (Samar) | 344 | 0 | - |
| USN18 (Cape Engano) | 16 | 0 | - |
| USN21 (Okinawa) | 1 | 0 | - |

No log in any worker tree has a kamikaze-capable PilotSetTarget call apart from USN19.

**The scene scan** is read-only over this installation's `universe/scenes/missions/**.scn`. It
takes the class names from `universe/library/global.enums` and the kinds from
`scripts/datatables/autoload/vehicleclasses.lua` (mtime 2026-05-09):
- Kamikaze planes (`KamikazeZero`, `KamikazeVal`, `KamikazeJudy`, `KamikazeOscar` and the
  `_light` variants).
- Small ships:
  - TorpedoBoat: `Elco` 27, `Kamikazeboat` 43, `JapPT` 77;
  - LandingShip without BigLandingShip: `Higgins` 40, `Daihatsu` 90.

Kamikaze planes appear in ten single-player scenes, and **none of them holds an American small
ship**. Japanese small ships appear beside them only in the PRCP Leyte (1), Endgame at Kure (2) and
`us_asw` (3) scenes, where the kamikazes are on the same side. The scenes that do mix kamikazes
with US PT boats are all under `multi/`. So no single-player row can reach the test, and USN19 is
the reach row for the zero-change prediction.

**The PilotFires reader** is routed to the units lane (`GameUnitsHost::plane_pilot_fires_0c24`,
lua14). Until it lands, the binding reads the byte as set, and it records
`Plane::pilot_fires_0c24` whenever a small target is met. So ON would block every kamikaze order
against a small ship. That is the image's answer for a plane whose PilotFires is set, and it is
labelled. The reader replaces it when it lands.

### Predictions, written before any ON run

1. **USN19 3200/3000 is gameplay identical** (exit 0 or 1). Its kamikaze orders name ship
   classes 9 and 11, which answer neither `0Eh` nor `0Ch`, so `small` is 0 and nothing is
   blocked.
2. **Mechanism failure** keeps the switch OFF: a nonzero `blocked` on USN19, or any move.

### Section 38 item 2, second caller: `0081639D`, recorded and not bound

`0081639D` is inside `008162B0`, the command-availability predicate, and is reached only past
`008162BF`'s `00779D50`. Two game-host copies of that predicate exist:
- the AI path, `tick_request_join_formation` in `src/game_hosts_ai.cpp`;
- the script path, `GameScriptOrdersHost::entity_command_is_available`.

Both implement only the `follow` arm (`bsp::entity_may_follow_target_00779d50`). When `00779D50`
answers false, both answer false.
- The image instead continues through `008162DB..00816406` (`src/ship_ai_states.cpp`,
  `ship_ai_command_available_008162b0`), whose host interface nothing implements.
- `0081639D` in that continuation needs a follower that is itself kind 8, a submarine
  (`0081636F`).
- Binding it means binding the whole continuation in both copies. No counted row has been shown
  to reach it, so it is left recorded.

`0096ACB4` (section 40) stays unowned and unreconstructed.

### The pair (cc9-ships13, 2026-09-29)

- OFF is this tree at `908e8b8b3` (`local\s13zoff_usn19.log`).
- ON is `pair_export --commit 908e8b8b3 --flip kKamikazeShipBlockedBound=true --out local\s13_z`
  (`local\s13zon_usn19.log`).
- The export's configure could not download `lua-5.1.1.tar.gz` (lua.org timed out twice). The
  tarball was copied from this tree's own `build\win32\_deps` and the export's `build.ps1` was
  re-run.

| row | kamikaze PilotSetTarget calls | small_targets / blocked | `pair_diff` | prediction |
| --- | --- | --- | --- | --- |
| USN19 3200/3000 | 33 both | 0 / 0 both | 1, gameplay identical | 1 held |

The only moved lines are the switch's `bound=` field and the free-bearing scan's `empty=`
(225938 -> 228879), which is refill noise (section 40).

**Verdict: stays OFF until the PilotFires reader lands, then flips.** The mechanism matches and has
zero reach, but ON currently reads PilotFires as set, a labelled stand-in, so ON is not yet exact.
When `plane_pilot_fires_0c24` lands:
- replace the stand-in with it;
- flip the switch. A build is enough: this pair already shows zero reach, and the reader is
  consulted only when a target is small.

## 43. Section 34's open item: the ridge in front of CB2 on USN01, audited (packet `cc9_usn01_ridge_audit`)

Worker cc9-ships13, 2026-09-29. Read-only against this installation's data. **Closed: the host's
terrain matches the data, and there is no substitution to fix.**

**What the host loads.** CB2 stands on `Landscape 03`, FilePath `islands/m07_a`. The host takes the
height field from `terrain/islands/m07_a_heightmap.tdt` (mtime 2024-07-13; byte-identical to
`models/terrain/islands/m07_a_heightmap.tdt`) through `load_terrain_height_field_00adda60`. It
logs `tiles=11x12 blocks=75 origin=(-900.0,-1200.0) node=(3000.0,0.0,-4000.0)`. The origin comes
from the `m07_a.mmod` BoundingBox minimum (-599.68, -711.07), a labelled substitution for
`00ADA420`'s box (docs/SCENE_CONTENTS_HOSTS.md 6). The segment query runs `0098ADD0` -> `0087FF80`
-> `00ADA240` (`landscape_entry_segment_hit`).

**The independent check** (`local\s13_ridge.py`, `local\s13_profile.py`). Neither script uses host
code:
- A fresh decode of the TRNV2 file. Each chunk is a u32 tag length, the tag, and a u32 size. There
  are 75 `NODE` tiles, each a `U16` chunk of `offset, scale` and 33 x 33 samples, rows along z. The
  height is `s / scale + offset` and 65535 is a hole.
- The island's render mesh, taken from the model_dump glTF of `m07_a.mmod` in this installation
  (44142 triangles, y range -119.41..90.60; the field's range is -119.413..90.548).
- **Placement.** On 35 random points on land, the field matches the mesh with a median |dh| of
  0.08 m and a p90 of 0.62 m, but only with the host's origin (-900, -1200) and z-row layout. Every
  other 300 m origin is off by a median of 27 to 90 m. So the host's BoundingBox origin is the one
  that puts the field on the island's own mesh.
- **CB2** is at (3973.4, 3.0, -3182.2). The field there is 2.99 m, so the building sits on the
  sampled ground.

**The segments.** A diagnostic in this tree, env-gated `BSP_SIGHT_POS_LOG=1` in
`zone_allows_target_00864fd0` with no behaviour, logs every tenth hidden answer
(`local\s13rg_usn01.log`, USN01 3200/3000, 369 hidden of 1542). All the hidden tests are Dunlap
(unit 44) against CB2, with Dunlap at sea 954..1320 m out to the north-east. Along every logged
segment, the field and the mesh agree within about 1 m:

| Dunlap at | length | highest ground on the line | 300..370 m from CB2 |
| --- | --- | --- | --- |
| (5003, -2356) | 1320 m | 47.9 m at 505 m (mesh 48.1) | 34.1 (mesh 34.5) |
| (4520, -2298) | 1040 m | 72.3 m at 705 m (mesh 72.7) | 33.3 (33.4) |
| (4437, -2294) | 1002 m | 90.1 m at 695 m (mesh 90.1) | 25.7 (25.7) |
| (4304, -2287) | 954 m | 76.5 m at 680 m (mesh 77.4) | 28.5 (29.3) |

So the "ridge about 25 m high about 320 m out" is the island's own ground rising inland of CB2. It
climbs to hills of 48 to 90 m between CB2 and the sea where Dunlap sails. The data puts the hill
there, and the host's hit points at y 24..26 lie on that slope.

**What stays open.** The mesh here is the render mesh. The image also attaches a collision node
(`+1E4h`, `0098BA10`), which the host does not build; the segment query's terrain answer is the
height field, as the host has it. CB2's class height (the raised point y 32) was not re-audited.

## 44. Handoff (cc9-ships13, 2026-09-29, at about 65% context)

**State of the lane.** Branch `agent/cc9-ships13`. The one lease still held is
`cc9_kamikaze_ship_blocked`, on `src/game_hosts_script_orders.cpp`/`.hpp` and
`src/attack_commands.cpp`/`.hpp`. Switches this worker touched:

| switch | state | section |
| --- | --- | --- |
| `kShipAiStandoffTargetKindBound` (`include/bsp/game_hosts_ship_ai.hpp`) | ON: kind queries, `00827F70`, mode-2 CaptureRange, mode-4 LandingRange; zero reach | 39, 41 |
| `kAiSquadronRtbExclusionBound` (`include/bsp/game_hosts_ai.hpp`) | ON: read whole, zero reach on USN13 and IJN01 at 9000 frames | 40 |
| `kKamikazeShipBlockedBound` (`include/bsp/game_hosts_script_orders.hpp`) | OFF, **pending** `GameUnitsHost::plane_pilot_fires_0c24` (units lane, lua14) | 42 |

**Pending: the kamikaze flip.** When the reader lands:
1. Merge main.
2. In `game_hosts_script_orders.cpp`, replace the labelled stand-in (`record_unimplemented("Plane::pilot_fires_0c24", ...)` and `uncommitted_kamikaze = false`) with `!units_.plane_pilot_fires_0c24(row.unit_index)`.
3. Set the switch true.
4. Build and commit. The USN19 pair already shows zero reach, and no single-player scene puts kamikaze planes beside US small ships.

**Diagnostics left in the code:**
- `summary mission ship ai standoff target kind calls= kind_08= building_mode2= building_mode4= small_class=`.
- `summary mission ai squadron rtb exclusion bind stand_in_true= issues= excluded=`.
- `summary mission script kamikaze small-ship test bound= small_targets= blocked=`.
- `BSP_SIGHT_POS_LOG=1`: the unit and target positions of every tenth hidden approach sight test.

**Open, in order:**
1. **CB2's class height on USN01** (section 43). The raised sight point sits at y 32 on a building
   whose ground is y 3.0. It was not re-audited against CB2's class (`type_id=6`, kind `1Ch`).
2. **The image's island collision node.** `+1E4h` is attached as a static root at `0098BA10`
   (`00884078`), and the host does not build it. The segment query's terrain answer is the height
   field. Whether the image's cast also meets the collision mesh was not read.
3. **Landers never enter the approach with a building target** (section 39). No row reaches modes
   2, 3 or 4. The latch's own `+7C4h` read (`009F20A4`) and `006F2D90` (the free-landing-spot
   test, mode 3) are unbound for that reason.
4. **`0081639D`** (section 42). The remainder of `008162B0` past `00779D50` is unbound in both
   host copies. It is reached only for a submarine follower.
5. **`0096ACB4`** (section 40) is an unowned, unreconstructed warning-event filter.
6. The standoff binding's other stand-ins: `unit_is_group_leader_00778890` answers false,
   `unit_cruise_speed_0490` answers 0, and `random_stream1_00bd2f10` answers `low` without drawing.

**Useful files** in the cc9-ships13 tree:
- `local\s13_run.ps1 -Exe <exe> -Prefix <p> -Row tag:MISSION:frames:mission_frames`. It launches
  in the background with the reference environment; wait on the log's final COM release line.
- `local\s13_ridge.py` and `local\s13_profile.py`: the independent TRNV2 and glTF terrain decode
  and profile.
- Census logs: `local\s13c_*`, `s13l_*`, `s13r_*` and `s13kz_*`.

**Section 42 closed (cc9-ships13, 2026-09-29).** `GameUnitsHost::plane_pilot_fires_0c24` landed on main (`9ff636740`,
merge `ea5775f9b`). It replaced the stand-in, and `kKamikazeShipBlockedBound` is ON by section 42's verdict: USN19 is
gameplay identical with zero reach. The build passes. Section 44's pending item is done.
## 45. The kamikaze_attack step `009E2020` (packet `cc9_kamikaze_attack_step`, `kShipAiKamikazeAttackStepBound`)

Worker cc9-ships14, 2026-09-29 (11:57 UTC). This is GAMEPLAY_GAP_RANKING rank 3.

### 45.1 The image

**`009E2020` `BSP_ShipAi_KamikazeAttackStateStep`** (a hypothesis name).
- ABI: `__thiscall(state)(float seconds)`, `RET 4`. `seconds` is never read.
- Body: `009E2020`-`009E23A6` exclusive, read whole from the listing. `RET 4` is at `009E23A3` and
  INT3 starts at `009E23A6`. The name ledger's end `009E23AD` is 7 bytes long. The reconstruction
  ledger now carries the verified end.
- Place: vtable `00D216B8` slot `+0Ch`. The state object is `brain+2254h`.
- The vtable's other slots:
  - enter `+4h` = `009DB320` (body `009DB320`-`009DB343`): `blk+234h = blk+29Ch = 1225.0f`
    (`00D216E8`) and `state+8h = 0`. This is the engage member's enter `009DB5E0` again.
  - exit `+8h` = `007B3DC0`, a bare `RET`.

**With a target at `[brain+0B20h]`** (`009E203A`..`009E2326`), the body is `009E23B0`'s (the
attackmove engage sub-state) **instruction for instruction**:
- the same pose guards;
- the same range with its `1e-10` epsilon (`00CE3820`);
- the same run-latch exit at 300 (`00CE3AE8`);
- the closing speed with the same 0.4 floor (`00CE65D0` / `00CE7804`);
- the lead `range/closing - 2.0` (`00D7A308`), clamped to [0, 12] (`00CEB4B8`);
- the intercept at `target + lead * target velocity`;
- `brain+3F8h = [unit+54h]`;
- the close arm: `brain+3FCh = 1`, `009DE050(blk, &intercept, 0, 0)`, and `009DA610`, which latches
  when the range is under 250 (`00CF8850`);
- the run arm: `brain+3FCh = 0`, the heading from `atan2(dz, dx)` through `00CE3830` / `00CE3828`,
  `009DFF40`, and `brain+0AF0h = 1.0f` (`00D7A24C`).

**The null-target arm** (`009E2329`..`009E239C`) is `009DFF40` inlined with the unit's own heading
(`unit->vtable[50h]()`), followed by `brain+0AF0h = 1.0f`. `009E23B0` takes `009E00A0` here
instead, which also zeroes `blk+1C8h` and `blk+1D0h`. This arm does not.

### 45.2 The reconstruction

- `src/ship_ai_kamikaze_attack.cpp` holds `ship_ai_kamikaze_attack_step_009e2020`.
  - Its null arm is its own.
  - Its target arm calls `ship_ai_attackmove_engage_step_009e23b0`.
- The host binding: `EngageStepBinding` became the template `EngageStepBindingT`, and
  `KamikazeStepBinding` adds only the unit heading.
- The latch lives in `Controller::kamikaze`. The enter clears it.
- With the switch OFF, the step and the enter are records, as before.
- Diagnostics: one summary line per boat,
  `summary mission ship ai kamikaze attack <unit>: steps= run_steps= null_target= min_range= latch=`,
  plus `summary mission ship ai kamikaze attack bound=`.

### 45.3 Predictions (written before the pair)

**The only reach is USNOS.** The six boats `unit #3.1`..`#3.6` hold the state:
- they are **Kaiten**, not Shinyo: `models/ships/japan/Kaiten.mmod`, submarine creator `008531A0`;
- `state=kamikaze_attack` is reached at step 820;
- their fire targets are TroopTrans5 (#3.1-#3.3), TroopTrans4 (#3.4) and TroopTrans1 (#3.5).

OFF, they sit at `throttle 0.000 dir=stopped` and do not move.

**USNOS 3200/3000:**
- **P1.** Every one of the six boats prints a kamikaze line with `steps` about 437 (2622/6) and
  `null_target=0`. `009E2020` UNIMPLEMENTED disappears from the table.
- **P2.** The boats now steer at the intercept through `009DE050`, so each boat's position moves. The
  ship AI table shows `mode=navigate`, not `rudder/stopped`. `min_range` falls well below the
  starting range.
- **P3.** The close arm latches only under 250 m. Some boats may reach the run arm (`run_steps > 0`)
  in 150 s. The prediction is uncertain because the Kaiten's speed and starting range are not in the
  log.
- **P4.** Deaths stay at 6. The six scripted movie deaths at 29.60 s are unchanged. A Kaiten's
  contact detonation is not modelled here: the collision path that would spend KamikazeDamage has
  no host. So no transport takes ramming damage, and the damage and hit totals move only through
  AA and gun fire at the now-moving boats.

**USNOS 9200/9000:**
- **P5.** On m, the six Kaiten die at 160.81 s with no damage (`first_damage=-1`, no killer). Their
  air runs down (depth trace: `air=0.917` at step 201) with `air=120/40`, so this is taken to be
  air exhaustion. The death time should not move unless the steering changes the depth.
- **P6.** If a boat is shot before 160.81 s it dies earlier, with a killer. With the boats moving,
  this is possible. The remaining 15 death rows (the `#2.x` group from 168.61 s on) are expected to
  stay the same, unless the moving Kaiten pull the escorts' fire.

**Verdict rule.** The switch flips ON if P1 and P2 hold (the mechanism) on both rows. Spread misses
in P3 to P6 are recorded.

### 45.4 The pair and the verdict: ON

**Setup.**
- Binaries: `f975ff9d8`, OFF `AD9B3C51053C`, ON (flip) `4D66ECD8BE94`, built with
  `tools/pair_export.py` in the cc9-ships14 tree.
- The reference environment, the console session, and a 300-frame USNOS smoke (ON) first.
- Logs: `local\s14koff_usnos{,l}.log` and `local\s14kon_usnos{,l}.log`. `pair_diff` exits 3 on both
  rows.

**USNOS 3200/3000.**
- P1 holds. Every boat prints `steps=437 null_target=0`, and `009E2020` leaves the UNIMPLEMENTED
  table (521 -> 520).
- P2 holds:
  - The boats go from `mode=rudder dir=stopped throttle 0.000` to `mode=navigate dir=ahead`.
  - Throttle is `-0.625` on the entry step (step 820), then `1.000`.
  - `d32c` falls about 7.7 m per 10 steps, which is the class's 15.43 m/s.
  - `min_range` is 2659 to 3468 m at 150 s, from about 5500 m at step 820.
- P3 misses. `run_steps=0`: no boat gets under 250 m of its transport in 150 s.
- P4 holds. The death rows are identical (6). The fire at the moving boats moves the counters:
  - shots 2316 -> 4968;
  - hit records 800 -> 862;
  - hull hits 79 -> 84;
  - damage 1675.3 -> 2039.7.

**USNOS 9200/9000.**
- P5 holds for four boats. `#3.3` to `#3.6` still die at 160.81 s with no damage. They are now much
  closer to the fleet: `#3.3` dies 154 m from Gear13, which was 1633 m OFF.
- P6 happens:
  - `#3.1` is killed at 154.25 s and `#3.2` at 159.11 s, both by NH (a gun category 6 kill, and
    killer_blast 1 for #3.1), instead of the 160.81 s air death;
  - the moved fire reshuffles the `#2.x` group's deaths: the same nine victims, the times 3.7 to
    29 s apart, and the killers reassigned among NH, Portland2 and Gear15/16.
- Deaths are 21 -> 21 with the same victims; 15 rows changed. `run_steps=0` again: the transports
  keep their distance (`min_range` 2363 to 3166 m) and the Kaiten close on the escorts instead.

**Verdict: ON.** The mechanism (P1, P2) holds on both rows. P3 is a spread miss: nobody reaches the
run arm, so `009E2020`'s run arm stays unexercised, as `009E23B0`'s already was. The death-row moves
are the boats being shot while they move. `kShipAiKamikazeAttackStepBound` is now `true`.

**Still open.**
- A Kaiten's contact detonation (the KamikazeDamage / KamikazeBlastDamage spend on collision) has no
  host. A boat that reached its target would pass through it.
- The air model kills the four unshot boats at 160.81 s, whatever the steering does.
- Reference n will see USNOS 3000 and 9000 move.
- For the lead to apply in Ghidra: the name ledger's evidence for `009E2020` still says
  `Body 009E2020-009E23AD` and "NOT projected".

## 46. Kamikaze contact detonation (packet `cc9_kaiten_contact_detonation`, `kKamikazeContactDetonationBound`)

Worker cc9-ships14, 2026-09-29. The switch lives in `src/game_hosts_gunnery.cpp`, a gunnery12 file
claimed for this packet by the lead's leave.

### 46.1 The image

The chain is **physics contact -> `009377E0` -> `008145B0` -> message 70h -> `00819A20`**.

- **`009377E0` `BSP_UnitController_OnCollisionContact`** (docs/UNIT_CONTROLLER_UPDATE.md).
  - It is slot 0 of `00D1961C`, called by the physics library for a body contact.
  - The damage gate opens when either party's class has `+510h > 0` or `+514h > 0`.
  - Two cancels:
    - `unit+6B8h >= 0` and `00779AD0()` (the frames since `unit+294h`, times 0.05) below 3.0
      (`00CE3854`);
    - `0092CE70` non-zero on the other.
  - It then compares the contact-point speeds (`v + w x r`, both bodies) and calls `008145B0` for the
    body with the smaller one.
- **`008145B0`**, read whole from the listing.
  - ABI: `__thiscall(unit)(const float3* contact, float magnitude, entity* other, int)`, `RET 10h`.
    Body `008145B0`-`008146DB`. `magnitude` and the last argument are unread.
  - It requires `game+1FE4h != 2`, a non-null other, and `00803510(unit+54h, other+54h) == 1`
    (enemy).
  - Then, for each party that is live (`+5Dh`, `+5Eh`, `+5Fh`, `+60h` clear) and a kamikaze
    (**`00779AA0`**: `[class+510h] > 0 || [class+514h] > 0`), it builds message 70h with the other
    as the target (`0080FF30`, with `00427C90(contact, 00427EB0(party))`). It routes it through
    `0077C2A0` route 7.
- **`00819A20` `BSP_UnitInstance_KamikazeDetonate`**, the 70h arm at `0082217D`. Read from the
  listing to the effect call.
  1. `unit+100Ah = 1`. The only reader is the getter `00951A90`.
  2. **The direct arm** (`00819A51..00819BCF`) needs a target with `+5Ch` set, `+5Dh`/`+60h`/`+5Eh`
     clear, and `+510h > 0.0` (`00D7A218`).
     - The record is `+14h = unit->vtable[54h]()`, which is `0042B8D0` = `FLDZ; RET`, so 0.
     - `+18h..+20h = unit+FCh`, `+24h = +518h`, `+28h = +514h`.
     - A trace from `unit+FCh` along the message point times 1000.0 (`00CE47A0`), against
       `target->vtable[B0h]` / `0098AC20`.
     - On a hit: `00926E80` and **`00915F20(target, record)`**.
  3. **The blast arm** (`00819BDA..00819C14`): when `+514h > 0`, `0084BAD0` with centre `unit+FCh`,
     radius `&[class+518h]`, damage `&[class+514h]`, and ignore, shot and source all 0. With no
     source entity, no gathered record is skipped, so **the burst reaches the kamikaze's own hull**
     (docs/EXPLOSION_RADIAL_DAMAGE.md, the `sourceEntity == 0` consequence).
  4. A point effect from `[class+51Ch]`.

  The kamikaze's own death is its blast, not a Kill. This installation's `vehicleclasses.lua` (mtime
  2026-05-09, modded) gives the Kaiten and the Shinyo `KamikazeBlastRange = 50`.

### 46.2 The binding (committed OFF)

`GameGunneryHost::Impl::run_kamikaze_contacts` runs once per gunnery step, after the projectiles.
Its substitutions are labelled in the code:
- **The contact.** The host has no body contacts. A kamikaze touches a ship when its bow point or
  its centre is inside that ship's hull box. The contact point is the bow point.
- **Gates not modelled:** the slower-body test and `009377E0`'s two cancels. Every unit here is
  scene-placed.
- **The blast.** It is gathered over the hull boxes, as the projectile burst is, at the box
  distance. Records go through `apply_gunless_blast_hit`, which is `apply_hit`'s tail with no gun and
  no shot, and so has no attribution.
- **The direct arm's delivery `00915F20`** is unread. It is a record,
  `KamikazeDetonate::direct_hit_00915f20`.

Diagnostic: `summary mission gunnery kamikaze contacts= hostile_refused= detonations= direct_arms=
blast_records= self_records= blast_damage= min_gap= at bound=`. `min_gap` is the closest any live
kamikaze bow came to a hostile hull box.

### 46.3 Predictions (written before the pair)

**Reach is probably zero.**
- On section 45's ON 9000 row, no kamikaze got within 146 m of any ship. The distances at death:
  - Kaiten: 146 to 1023 m;
  - Shinyo `#2.x` (`Suicide_boat.mmod`): 283 to 1394 m, all shot first.
- USNOS is the only reference row with kamikaze classes (the engage gate's `kamikaze_classes`
  counter).

**P1.** USNOS 3200/3000 and 9200/9000 print `detonations=0`. `pair_diff` exits 1 (gameplay
identical) on both. `min_gap` is tens to hundreds of metres.

**P2.** If a boat does touch, then:
- one `kamikaze detonation` line appears;
- the kamikaze dies from its own blast in the same step, with `self_records >= 1`;
- the struck ship takes up to 3000 (Kaiten) or 1500 (Shinyo) blast damage;
- that ship's death row moves.

**Verdict rule.**
- Identical with zero reach: the switch may flip ON, recorded as zero reach, as sections 39 and 42
  did.
- Moved: judge the mechanism by P2.

### 46.4 The pair and the verdict: ON, zero reach

**Setup.**
- Binaries: `2ebb264f1`, OFF and ON (flip), built with `tools/pair_export.py` in the cc9-ships14
  tree.
- The reference environment, the console session, and a 300-frame USNOS smoke (ON) first.
- Logs: `local\s14doff_usnos{,l}.log` and `local\s14don_usnos{,l}.log`.

**P1 holds.** `pair_diff` reports `GAMEPLAY: identical` on both rows.

| row | contacts | detonations | min_gap |
| --- | --- | --- | --- |
| USNOS 3000 | 0 | 0 | 249.2 m at 150.00 s |
| USNOS 9000 | 0 | 0 | 110.5 m at 159.51 s |

On the 9000 row the 110.5 m is a Kaiten, a second before the 160.81 s air deaths. **No kamikaze
touches a hostile hull on any reference row.** There are no false contacts either: `hostile_refused`
is 0 and `contacts` is 0, so no spawn overlap trips the hull-box stand-in.

**Verdict: ON, recorded as zero reach**, as sections 39 and 42 were. The chain `008145B0` ->
`00819A20` is therefore unexercised in any run: P2 (the self-kill by the own blast, the struck
ship's damage) is unverified.

**What would give it reach.**
1. A Kaiten that lives past 160.81 s. The air model (SUBMARINE_MODEL) kills them first on USNOS.
2. A Shinyo that survives the escorts' fire to under 50 m.
3. A mission where they start closer.

**Still open.**
- `00915F20`, the direct arm's delivery, is read only to its entry gates. Its record would carry
  `+28h = +514h` with no part entries, so what it applies needs the body.
- The contact gates `0092CE70` and `unit+6B8h` / `00779AD0`, and the slower-body test, are not
  modelled.

## 47. The AI planner spawn `00A25B90` -> `00A25A30` (packet `cc9_ai_planner_spawn`, read in part)

Worker cc9-ships14, 2026-09-29 (13:12 UTC). This is GAMEPLAY_GAP_RANKING rank 8. The listing was
read and nothing was bound. The packet is handed off unfinished at the handoff threshold.

**Correction first: the Ghidra decompile of `00A25A30` is wrong.** It "removes unreachable blocks"
`00A25AC1..00A25B24` because it believes the list at `[ESP+14h]` is still empty after `0066E590`.
The listing shows the list is filled there. Read `00A25A30` from the assembly
(`python tools/bsp.py ghidra disasm 00a25a30`).

**`00A25B90`**, the planner spawn (`__thiscall(planner)(...)`). Its gates, in order:
1. `[00F8AB6A] == 0`.
2. `00946970([00F89B3C], [[planner+1Ch]+20h]) == 0.0` exactly. That is `FUCOMIP` against `FLDZ`,
   then `LAHF` / `TEST AH,44h` / `JP` to the exit on not-equal or unordered. `00946970` is tagged
   `stl_probable`, and its segment carries the `supportmanager` and `resourceusage` strings.

Then:
- `00A08420(...)` fills a local vector.
- The quick-spawn position hint (AI_GROUP_THINK section 4) is taken.
- It calls `00A25A30(planner)(pos-or-null, 1.0f, &vector, tag, 1)`, `RET 14h`.

**`00A25A30`**, body `00A25A30`-`00A25B87`.
1. It returns false when the weight is below `[00CE3800]`, or when the vector (32-byte entries) is
   empty.
2. Otherwise it gathers `0066E590(ECX = [planner+30h], EDX = &list, party = [[planner+1Ch]+20h])`:
   - `0066E510`, and in it `0066E2B0`, walk the `[planner+30h]` list and keep entries whose entity
     `+54h` equals the party;
   - the entries with `+14h == 0` are then removed.
3. With a non-empty list, it runs:
   - `00A236F0(&sorted, &list, pos or 00F87574)`, which looks like an order by distance from the
     hint;
   - `00A23980(planner)(pos, &sorted, weight, &vector, tag, flag)`, whose byte result is the
     return.
4. `00A23980`'s callees include `00964790 BSP_VehicleClass_GetOrCreate`, `00942130` and
   `00947940` (support-manager segment), `0084D560` and `00A21D90`.

**Reading so far, provisional.** The spawn buys reinforcements through the support manager at spawn
points the party owns (the `[planner+30h]` list, filtered by `+54h`). It creates **units**, not
just a group object: `VehicleClass_GetOrCreate` is on the path. Neither claim is proven. The
producers of `[00F8AB6A]` and of `00946970`'s answer decide whether a single-player mission ever
passes the gates. `BannSupportmanager` / `PermitSupportmanager` (`008D1F50`-`008D2BB0`) are
candidates for `[00F8AB6A]`.

**Next steps.**
1. Find the writers of `[00F8AB6A]` (scan `6A AB F8 00`) and read `00946970`. If a single-player
   row never passes the gates, close the item with that evidence. The host's `spawn_due` counts
   only the planner-side conditions.
2. Otherwise, read `00A23980` whole and decide how the host would create a unit at run time.
   SpawnNew `0094C480` exists as a model.

## 48. Handoff (cc9-ships14, 2026-09-29 13:12 UTC, at about 78% context)

**State.** Branch `agent/cc9-ships14`.
- On main: 945e105a5, f975ff9d8, 04e56970d, 9319e32f6, 2ebb264f1, eccebd3ca and 4d0a51c9d.
- This commit adds sections 47 and 48.
- No lease is held after this commit.

**Switches this worker turned ON:**

| switch | file | section |
| --- | --- | --- |
| `kShipAiKamikazeAttackStepBound` | `src/game_hosts_ship_ai.cpp` | 45 |
| `kKamikazeContactDetonationBound` (zero reach) | `src/game_hosts_gunnery.cpp` | 46 |

**The lead's queue, not started or unfinished:**
1. `cc9_ai_planner_spawn` (ranking 8). Read in part in section 47; start from its next steps.
2. `cc9_scripted_order_natives` (ranking 9 and 13), in `src/mission_lua_host.cpp`: `UnitHoldFire`
   `008A6AC0`, `PilotLand` `008A47B0`, `SetShipMaxSpeed` `00890A10`, `NavigatorEnable` `008A7060`,
   `Scoring_SetMissionCompleted` `008B8AD0` and `GetCapturePercentage` `0089B840`. Predict per row
   on IJN01, BSM01, JM05, USN02, LOMP10 and USN01 (GAMEPLAY_GAP_RANKING row 9 lists the rows).
3. The periscope sub-state `009E4DC1` (ranking 11).

**Open from sections 45 and 46:**
- the direct-hit delivery `00915F20` of `00819A20`;
- the contact gates `0092CE70` and `unit+6B8h` / `00779AD0`;
- the Kaiten air deaths at 160.81 s, which keep every kamikaze short of a hull on USNOS. The run arm of `009E2020` / `009E23B0` (latch under 250 m, `009DFF40` heading, `brain+0AF0h`) is still unexercised on every row.

**Useful files in the cc9-ships14 tree (`local\`):**
- `s14_run.ps1 -Exe <exe> -Prefix <p> -Row tag:MISSION:frames:mission_frames` launches in the
  background with the reference environment.
- `s14_wait.ps1 -Logs <names>` is the foreground wait on the final COM release.
- `s14_gap.py <out.tsv> <logs...>` is the UNIMPLEMENTED and refusal census.
- `s14_ctx.py <n> <names...>` prints the comments before each host record site.
- Logs: `s14m_*` (fresh main rows), `s14k*` (section 45 pairs), `s14d*` (section 46 pairs).

## 49. Scripted-order natives (packet `cc9_scripted_order_natives`, `kScriptedOrderNativesBound`)

Worker cc9-ships15, 2026-09-29 (started 13:36 UTC), base `81bf3f3fd`. Ghidra was read-only.
This covers GAMEPLAY_GAP_RANKING rows 9 and 13. The call counts come from the `rb14_*` reference
logs (GAME_EXECUTABLE.md reference n).

### 49.1 The natives, read whole

| native | body (V = listing read) | rows, calls | this packet |
| --- | --- | --- | --- |
| `NavigatorEnable` `008A7060` | arg0 through `00888AA0` (no kind check), then `[unit+740h]+11h` = arg1 as a boolean. `unit+740h` is the ship AI controller (`00810DF7`). `+11h` is the enabled byte of its tick sub-node: `0072BBD0` sets it to 1 at `0072BBEF`. `008759C4 CMP [ESI+11h],BL / JE` skips the `+0Ch` tick, which is `009F50E0` for vtable `00D21AE8`. The weapon director is a separate sub-node, so it keeps ticking. | JM05, 1: `luaJM5InitCapPt` (`jm05.lua:4502-4509`, this installation, mtime 2024-07-13) calls it with `false` right after `GenerateObject("Event2Pt")`. The `true` call at `:1344` needs a capture that is not reached. | **bound**. `GameShipAiHost::set_navigator_enabled_0011`, a per-unit flag, gates the `009F50E0` step. |
| `UnitHoldFire` `008A6AC0` | arg0 through `00888AA0`, `vtable[114h]`, then `0071BED0` on the answer with **no null check** (`008A6BD6-008A6BE4`). `0071BED0` is `0071BE80` with the stance literal 0 (`0071BEDB`, `0071BEEA`). | IJN01, 2: `ijn_1_pearl.lua:973`, B-17 01/02. This installation's copy has mtime **2024-08-26**, so it is modded. **Both are squadrons** (`GenerateObject squadron B-17 01: WingCount=2`), so the squadron arm is the one taken. An earlier reading took them for single planes, from the `plane spawn` rows; that was wrong. | **bound**. Squadron: the `+348h` block's answers for stance 0 (allowFire 0 from `0084D910`, allowMove 1 from `0084D930`), in the host's permission map, which is record-only. **Plane:** `vtable[114h]` of `00D06638` is `0047F180` (`XOR EAX,EAX`), so the image reads through null at `0071BED6`. The host counts that and does nothing, because it cannot reproduce the fault. Whether the game survives the fault (an SEH catch in the Lua call path) is unread. **Ship:** stays a record. `0071D560` allows fire for stance 1 or 2 and `0071D580` allows move for stance 2 or 3, then `0071DA50` sends 5Ah; that consumer is the gunnery host's. |
| `EntityTurnToEntity` `008A0A10`, the arms after the squadron arm | `008A0DE9`: `vtable[5Ch](6)`. **Ship arm** (`008A0DF8-008A0E68`): `00414DB0` when `+C8h` is clear, then translation = `+FCh` (`esp+64h..6Ch` = m[12..14]), then `vtable[88h]` (`006E00A0` for TBoat vtable `00D0C648`, read at `00D0C6D0`). Then `00C336C0` and `00C56CF0` on `[0080E490()+2Ch]`, the physics body pose. **Other arm** (`008A0E6F-008A0EB8`): `vtable[88h]` alone. | LOMP10 3000 and 9000, 2 each: `10_san_jose.lua:430`, PT 01 and PT 02 in `luaIntroMovieEnd`, after `SetShipSpeed` and `PutTo`. | **bound**: `set_local_matrix_006e00a0` for kind 6; `007C9540` for a plane; any other kind stays a record (`008a0eb8`). **SUBSTITUTION:** the host's ship pose and physics state are one motion state, and velocity is not touched. |
| `UnitSetFireStance` `008A6490`, ship arm | the ship director's `0071D560` / `0071D580` answers, then `0071DA50` / `0071DAD0` messages. | USN01, 9: `usn_1_marshall.lua:296` sets stance 0 at init on Northampton, SaltLakeCity and Dunlap. `:611` sets stance 2 on the first think and `:640` 3 s later (`secNarr`). Stance 2 is fire and move, the host's defaulted stance. | **not bound.** The consumer is the gunnery host's `director+3Ch/+3Dh`, which is gunnery12's. Predicted effect: only the window from init to the first think. |
| `SetShipMaxSpeed` `00890A10` | arg0 through `00888AA0` (no kind check), then `00890B51 FSTP [ESI+9C0h]`. `unit+9C0h` is the host's `motion.max_speed` (`00822C20` seeds it; `0080FC30` reads it). | BSM01, 2: Whitney and Tautog set to 6 (`bsm_01_stationed_at_pearl.lua:1872/1888`, mtime 2024-07-13). | **routed:** the lead has the setter edit for lua16. Binding follows once it is on main. |
| `PilotLand` `008A47B0` | `0077D600(entity, land 00E08FA0, target from arg1, flags 1)` at `008A4907`. The receiver is `0099A170`'s land arm `0099A3DD`, with the site from the command target. | IJN01, 2 (the B-17 planes to Mission.AF2) | **routed** to lua16: the land task install needs an explicit site owner. |
| `Scoring_SetMissionCompleted` `008B8AD0` | reconstructed as `run_set_mission_completed_008b8ad0` (`src/mission_result.cpp`), which writes a `MissionResultHost` scoring slot | USN02, 1, at the scripted failure | **not bound:** the scoring slot is the mission-frame host's result object, which the Lua host has no route to. The score record has no in-mission reader. |
| `GetCapturePercentage` `0089B840` | `006F1F90`: returns `|float [cb+7A8h] / int [cb+7A4h]|`, or 0 when `+7A4h` is 0 (FABS at `0089B966`). `006F2780` seeds `+7A4h` = CaptureValue (default 1000) and **`+7A8h` = 0**. The capture tick `006F6760` moves it and is unmodelled. | JM05, 48 (score text only, `jm05.lua:5178-5216`) | **closed, no gap in these rows:** before any capture progress the image answers 0.0, the host's neutral value. It opens with `006F6760`. |

### 49.2 Predictions for the flip (written before any pair)

- **JM05 (3000):** Event2Pt's controller never steps. It stays at its spawn (324.5, -0.5, -3224.7),
  so the reference's 2374.31 m run to (-727.6, -1096.2) becomes about 0 m. Its `ship ai step` rows
  freeze at their initial values. `summary mission ship ai navigator enable calls=1
  disabled_units=1`, with skipped steps close to the step count. The rest of the row can move
  through the neighbour and avoidance lists (Event2Pt is on them). Deaths stay 0.
- **LOMP10 (3000 and 9000 long):** PT 01 and PT 02 are posed to face Ashigara at
  `luaIntroMovieEnd`, and their tracks and end positions move. The planes' squadron arm is
  unchanged. The `008a0d1c` record disappears. Deaths stay 0 (the reference has none).
- **IJN01 (3000):** two null-director records, nothing done: gameplay identical (exit 0 or 1).
- **BSM01 (3000), control:** none of the bound natives is called there: identical (0).

### 49.3 The pairs, and the flip

The pairs are same-tree exports of `9273d482b`: `local\s15_off` (SHA-256 `16AE88B2F53A`) and
`local\s15_on` (`AB6154A1431E`). The logs are `local\s15{off,on}_<row>.log`, launched in the
reference form with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`. A 300-frame USN01 smoke
ran first (`local\s15s_smoke.log`). Every run presented its full frame count, and the session
stayed on the console.

| row | pair_diff | what moved | against the prediction |
| --- | --- | --- | --- |
| JM05 3000 | 3 | Event2Pt ends at its spawn (324.5, -3224.7), 0.00 m, against the reference's 2374.31 m run. `navigator enable calls=1 disabled_units=1 skipped_steps=3000`. Ship AI steps drop 152940 -> 149940, plus the plan, path and director counts that follow from them. Death rows are identical (0); damage 2118.3 and shots 99 are unchanged. | as predicted |
| LOMP10 3000 | 3 | PT 01 is posed to forward (0.678, 0, 0.735) and PT 02 to (0.687, 0, 0.726). End positions: PT 01 (-735.0, -2002.3) -> (-251.1, -1610.0); PT 02 (-2698.0, -2154.3) -> (-2568.8, -2593.4). The `008a0d1c` record is gone. Death rows identical (0). | as predicted |
| LOMP10 9000 | 3 | Both PTs reach their goals. PT 01 ends within 0.4 m of its OFF position; PT 02 ends at (820.6, -5187.9) -> (786.2, -5238.3). Death rows identical (0). | as predicted |
| IJN01 3000 | 1 | two `UnitHoldFire: squadron B-17 0n allowFire 0 allowMove 1` notes. Nothing reads the host's permission map. | the rows are squadrons, not planes (see 49.1), and the gameplay is identical, as predicted |
| BSM01 3000 | 0 | nothing | as predicted |

**Flipped ON.** In each row the mechanism matched and no death row moved.

**Still open from this packet:**
- `SetShipMaxSpeed`, once lua16's `set_unit_max_speed_09c0` is on main;
- `PilotLand` (lua16; the IJN01 targets are squadrons);
- the ship arms of `UnitSetFireStance` and `UnitHoldFire` (gunnery12, `director+3Ch/+3Dh`
  through the 5Ah message);
- `Scoring_SetMissionCompleted`, whose route is the mission-frame result host;
- `GetCapturePercentage`, which waits on the capture tick `006F6760`.

## 50. Scripted-order natives, part 2: `SetShipMaxSpeed` and `PilotLand` (`kScriptedOrderNatives2Bound`)

Worker cc9-ships15, 2026-09-29. The branch merged main `fb349958a`, which brings lua16's two
units-host entry points: `set_unit_max_speed_09c0` (commit `d7b093d01`) and
`land_at_site_0099a3dd` (commit `49cb3ae26`).

**The bodies are read in section 49.1.**
- `SetShipMaxSpeed` `00890A10`: `00890B51 FSTP [ESI+9C0h]`. `unit+9C0h` is the host's
  `motion.max_speed`, which `0080FC30` reads.
- `PilotLand` `008A47B0`: `0077D600(entity, land 00E08FA0, target, 1)` at `008A4907`. The
  receiving bots' land arm `0099A3DD` takes the site from the command target.
  - The binding issues the command once through the host's `0077D600` path.
  - After delivery it calls `land_at_site_0099a3dd`, which fans a squadron out to its members.
  - The site is resolved through the target entity. IJN01's `Mission.AF2` is
    `FindEntity("Airfield 02")`, whose deck is named "AirField 02".

**UnitHoldFire's plane arm: is the null read swallowed? (V, lead's question.)** No handler was
found between the binding and the Lua VM. The case is not reached on IJN01 (section 49.3: the
targets are squadrons).
- `008A6AC0`'s frame handler `00C9C09F` loads FuncInfo `00DCE5D4` for `___CxxFrameHandler3`
  (`00BF6B43`). The record is:
  - magic `19930522`;
  - 4 unwind states;
  - **0 try blocks**;
  - EHFlags = 1, which is the `/EHs` synchronous model. Even a `catch(...)` in such a frame
    does not take an access violation.
- The Lua core unwinds with `setjmp`/`longjmp`, not SEH:
  - `luaD_rawrunprotected` `00A68B80` calls `00C034F4` (`_setjmp3`) at `00A68BA6`;
  - `luaD_throw` `00A69330` longjmps at `00A69346`, or exits at `00A69369`.
- APP_INIT_BOOTSTRAP.md records no exception filter installed by the game.
- **Not read:** every frame between `lua_pcall`'s callers and `WinMain` was not scanned for an
  `__except`. Only the CRT's own `__try` in the entry point is known, and its filter does not
  handle an access violation.
- So in the original a UnitHoldFire on a lone plane most likely ends the process. This is
  uncertain until that frame scan is done.

### 50.1 Predictions (written before any pair)

- **BSM01 (3000).**
  - At init, Whitney's `unit+9C0h` goes from 12.8611 (`reference_speed` in the pre-step input
    line) to 6, and Tautog's from 15.4332 to 6. The ship AI's reference speed and throttle
    ceiling follow.
  - Whitney (state follow, 494.11 m in OFF) moves less. Tautog (state stop, 187.19 m) changes
    little.
  - Deaths stay 0. Other ships move only through the neighbour lists.
- **IJN01 (3000).**
  - Two `PilotLand` calls, when the B-17 squadrons are generated (their world-list push is at
    68.20 s). Each installs land tasks toward AirField 02 on its two members, unless the planes
    are not airborne; that refusal is logged by lua16's install core.
  - The B-17 tracks change. AA contacts with them can move kills through the shared RNG stream,
    so the death rows may move. Judge from the per-entity table.
- **LOMP10 (9000 long).** The n reference has no `PilotLand` call on this row, so identical is
  expected unless the fighter-bomber landing check at `10_san_jose.lua:605` is reached.

### 50.2 The pairs (`18b5e6710`, `local\s15_off2` / `local\s15_on2`; logs `local\s15{off2,on2}_<row>.log`)

| row | pair_diff | result | against the prediction |
| --- | --- | --- | --- |
| BSM01 3000 | 3 | `SetShipMaxSpeed: Whitney unit+9C0h = 6.0000 stored=1`, and Tautog the same. Whitney ends at (2002.4, -1947.1) -> (2137.7, -2091.3), 494.11 -> 314.97 m; Tautog 187.19 -> 150.43 m. HenryPT (the controlled PT, AI-driven while the player is idle) moves 596.41 -> 603.30 m. Death rows identical (0); the unit table is otherwise identical. | as predicted |
| IJN01 3000 | 1 | Two `PilotLand` calls, and both targets are unresolved (`-> (no unit)`), so no land task is installed. The two land commands still go through `0077D600` with a position target. Gameplay identical; the 9 death rows are identical. | **the mechanism was not exercised**; see below |
| LOMP10 9000 | 1 | no call; only the known LOMP10 camera noise and the refill counter | as predicted |

**Why IJN01's target is nil.** `Mission.AF2 = FindEntity("Airfield 02")` (`ijn_1_pearl.lua:526`),
but the unit is "AirField 02". The two FindEntity implementations match names differently:
- **The image matches case-insensitively.** `BSP_EntityRegistry_FindEntityByName` `00925A90`
  delegates to `009251F0`, which compares the whole name through `00438E10`
  (`BSP_CString_CompareInsensitive`) and path prefixes through `__strnicmp` at `00925273`.
- **This host matches case-sensitively.** `FindEntity` looks the name up in a `std::map<std::string,
  int>` (`src/game_hosts_lua.cpp`, `scene_entity_ids_.find(name)`).

So in the original, AF2 is the airfield and the B-17s are ordered to land. This host answers nil.
This is a units/Lua host gap (lua16's file), routed through the lead. It is not a PilotLand
defect, but it means the land install was never exercised.

**Switches.**
- `kScriptedOrderNatives2Bound` (SetShipMaxSpeed) is flipped **ON**.
- PilotLand moves to its own switch, `kPilotLandNativeBound`, which stays **OFF**. Re-pair IJN01
  once FindEntity matches case-insensitively.

## 51. The AI planner spawn, gates read (packet `cc9_ai_planner_spawn`, continued from 47)

Worker cc9-ships15, 2026-09-29 14:41 UTC. Ghidra was read-only. The call sites come from byte
censuses (`tools/callsite_census.py`), not from `ghidra callers`.

### 51.1 Correction to section 47: the single-player spawn does not pass through `00A25B90`

| routine | rel32 callers |
| --- | --- |
| `00A25B90` (the `[00F8AB6A]` gate) | only the mode planners: Duel `00A25FBE`, Escort `00A26255`, Siege `00A26554`, Competitive `00A26634` |
| `00A25A30` | `00A25C3E` (in `00A25B90`), DefendThink `00A29DAA`, CaptureThink `00A2B758` |
| `00A23980` | `00A25B0B` only (in `00A25A30`) |

So Capture and Defend, which are the planners the single-player rows run, call `00A25A30`
directly and never read `[00F8AB6A]`. Its writers are `00A32DF0` (clears it; referenced only from
the table at `00D23240`) and `00A38DA0` (`__fastcall`, CL = the value: `00A38DCF` stores 1 and
`00A39009` stores 0). `00A38DA0` is called from `00778180`, `0088C838` and `0088C9C8`. None of these
decides the single-player path.

### 51.2 The Capture spawn arm `00A2B477-00A2B7A6`, gate by gate (V, listing)

1. **`00946970([00F89B3C], team) == 0.0`**, at `00A2B488` / `00A2B491`. `00946970` sums, over the
   spawn manager's request lists, `00946870`. That is the `ResourceUsage` property (`00CFACAC`) of
   each queued record whose `OwnerPlayer` (`00CF882C`) is the team. The gate is **"no spawn request
   of this team is queued"**. On these rows the queue holds only what `SpawnNew` put there.
2. **`[ESP+4Fh]`, the ramp** (`00A2A0B3` / `00A2A0C1`, the host's `ai_tail_capture_spawn_due`).
3. **The budget** (`00A2B4A6-00A2B4EE`), w =
   `(1 - [00F8A8BC + 4*009FFC80()]) * 00942130(team) - 00A1C900(planner)`. It spawns only when
   w >= 1.0.
   - `00942130`: `[00E0CFB4]` (2400.0 in `.data`, rewritten from the lobby by `005E3273`) times
     the double `[00D7A280]` = 0.5, divided by the number of the eight slot records (`game+18CCh`)
     that have `+8h` set, `+9h` clear or `+0Ah` set, and `+28h` equal to `slot[team]+28h`. It is 0
     only when that count is 0.
   - `004C6890` sets `+8h` = 1 on every slot below the scene's `MaxPlayerNum`. The logs show
     `max_players=8 authored=1` on JM05 and IJN01. So `slot[team]` counts itself, the count is at
     least 1, and the budget is 1200 / count.
   - With the defend percent at its 0.35 default and no group resources (`ResourceUsage` is
     unauthored), w = 780 / count >= 97.5. **The budget passes.**
4. **The best target**: the loop `00A2B5D5-00A2B65B` over the plan's targets (`+2Ch` > `[00D7A218]`,
   the largest `+14h`). Then:
   - `00A24870` fills the vector with the entity records within tuning `+1A4h` of the target
     (`00A07D40`), or one default record from `BSP_Ai_EntityRecordBuild(1)` when there are none;
   - `00A25A30(planner)(target pos, min(w, rec+1Ch), &vector, "[capture]"+id, 0)`.
5. **The sources, `00A25A30` -> `0066E590`**:
   - `0066DD00` walks world list 28 (`[[00E188A8]+19CCh]+16Ch`, the CommandBuildings) for those
     with `+78Ch` != 0. That is the size of the `+784h` list `006F5CC0
     BSP_CommandBuilding_AdoptNearbyGarrison` fills with the forts (1Bh), airfields (45h) and
     shipyards (46h) near the building.
   - `0066E2B0` keeps the ones of the team (`+54h`), and `0066E590` drops the entries whose
     `+14h` is 0.
6. **`00A23980`** (body `00A23980-00A243C7`, `RET 18h`) prices and places the force, through
   `VehicleClass_GetOrCreate`, `00942130` and `00A21D90`. **Ghidra's listing drops
   `00A2410C-00A243AC`**, after the `_free` at `00A24107` (the CALL_RETURN problem;
   `ghidra flow` would show it). The dropped block is where the request goes out:
   - `00A24328 MOV ECX,[00F89B3C]`, then `00A24337 CALL 0094C830`;
   - `0094C830` builds the request (`00947BC0`) and, in session mode 0 or 1, calls `0094B600`,
     which validates and enqueues through **`BSP_SpawnManager_EnqueueRequest` `00949530`**. That
     is the queue `SpawnNew` feeds, and the host already drains it (`src/game_hosts_lua.cpp`,
     `spawn_request_queue`).

### 51.3 Verdict so far

In single player the gates **pass** wherever the team owns a CommandBuilding with an adopted
airfield, shipyard or fort, and the spawn then **creates units** through the SpawnNew queue.
- AI_PLANNERS.md's "the spawn arm is unreachable in this process" describes this host, not the
  image.
- Section 47's `[00F8AB6A]` question does not apply to single player.

This is ranking item 8, and it is real. Not yet read:
- `00A23980` (with the dropped tail from the disk bytes), `00A236F0`, `00A21D90`, `00A24870`'s
  visitor `00A07D40`, `00947BC0` and `0094B600`;
- the Defend arm `00A29B8E-00A29E2A` (its gate is at `00A29BF8` and its call at `00A29DAA`; its
  gate order was not read).

Binding needs those bodies and a host route from the planner into `spawn_request_queue()`.
**Next step:** a Ghidra flow repair of `00A23980` by the lead (`tools/ghidra_flow_repair.py`), then
read it whole.

## 52. The AI planner spawn, closed for these rows (packet `cc9_ai_planner_spawn`)

Worker cc9-ships15, 2026-09-29. This packet followed the lead's flow repair of `00A23980`
(`b67893b0a`: three gaps after `_free`, 0 left). Ghidra was read-only.

### 52.1 `00A23980` (body `00A23980-00A243C7`, `RET 18h`), read whole from the repaired decompile

`__thiscall(planner)(pos, sources, weight, threats, tag, flag)`. For each source site, in the
order `00A236F0` sorted them:
1. **`0084D560(site)(&list, team, 0)`** (ECX = the site, set at `00A23A47`) builds the classes this
   site can supply. Each is resolved through `BSP_VehicleClass_GetOrCreate` (`00A23A7B`).
2. **The budget** is `00942130(team)`. When `004BCA50` < 4, `00A0D1D0(classes, budget, threats,
   site+FCh, 0, flag, -1.0)` scores the site and fills the chosen composition (32-byte entries).
   The mode 4+ arm uses `00A07D40` / `00A06260` instead.
3. **A site with a non-empty composition is weighted.** The weight is 1.0 with one source.
   Otherwise it is `00419010` over tuning `+100h..+10Ch` of distance / `00A07C10()`, times the
   score. The best site's composition is kept through `00A21D90`.
4. **After the loop, with a best site**, it builds three dword vectors from the composition:
   - the class;
   - entry `+10h`;
   - `0084CF80(class)`, which overwrites the `vt[18h](0Fh)` / `(14h)` 1-or-3 value in the same
     slot.
   Then **`0094C830(team, &classes, site, [slot[team]]+2Ch, &values, &kinds, 0, flag)`** at
   `00A24337`, and the result is 1.

### 52.2 The deciding input: the site's stock list

`0084D560` copies the site's `+328h` list (team 0) or `+31Ch` list (any other team). It then
filters that list with `006F4A00(46h/45h)` and the class `vt[18h]`. **It adds nothing of its
own.** Both lists are filled only by `0084D170` (the site's activation, which reads `maxPlanes`,
`maxShips`, `SPActive` and `Angle`):
- the `JapanList` / `AlliedList` property sub-bags;
- one entry per `"Stock %d"` sub-bag, with `Count` != 0 (and `SquadSize`, default 3).

**Which scenes author a stock list.** A byte search of this installation's mission scenes (mtimes
2024-07-13) finds `JapanList` / `AlliedList` outside `multi\` in only four files:
- `ijn\ijn_11_operation_to.scn` (136 `Stock N` strings);
- three copies of `ijn_02_force_z.scn` (600).
No reference row's scene authors one:

| row | scene |
| --- | --- |
| USN01 | `usn_1_marshall` |
| USN02 | `usn_2_java` |
| JM05 | `ijn_05_invasion_of_port_moresby` |
| IJN01 | `ijn_1_pearl` |
| BSM01 | `bsm_01_stationed_at_pearl` |
| LOMP10 | `10_san_jose` |
| USN13, USNOS, JM06, JM08, USN12, LOMP06 | none of them are in the four files |

So on every reference row each site's class list is empty, and `00A0D1D0` has nothing to buy.
- **Uncertain:** `00A0D1D0` was read only at its head. That an empty class list gives an empty
  composition is inferred from its inputs, not traced.

**The two authored scenes, checked in the host (3000 frames, main-equivalent build):**
- **JM02** (`local\s15p_jm02.log`): `--menu-select JM02` loads
  `COTP-IJN/PRCPIJN/prcpijn_02_force_z.scn`, which is **not** one of the four files, and every
  Capture and Defend summary reads `thinks=0`.
- **IJN11** (`local\s15p_ijn11.log`): 40 capture thinks and 40 defend thinks, but the capture
  target path (the arm that holds the spawn) runs 0 times (`spawn_due=0`), and the defend records
  path runs 0 times (`spawn_arms=0`).

### 52.3 Verdict: closed for these rows, with no binding

- The single-player gates pass (section 51), but the spawn buys only from authored stock lists,
  and no reference row has one. So the image spawns nothing on these rows either, and the host's
  record is the right model there.
- **AI_PLANNERS.md's "unreachable in this process" is right in effect for these rows, for a
  different reason:** no stock, rather than no unit creation.
- **If a stock scene is ever paired:** the route is `0094C830` -> `00947BC0` -> `0094B600` ->
  `00949530`, the SpawnNew queue the host already drains (`spawn_request_queue()`,
  `src/game_hosts_lua.cpp`).
  - **Still unread** on that route: `00A0D1D0`, `00A236F0`, `00A21D90`, `00947BC0` and `0094B600`,
    plus the Defend arm `00A29B8E-00A29E2A`.

## 53. Handoff (cc9-ships15, 2026-09-29 14:47 UTC, at about 72% context)

**State.** Branch `agent/cc9-ships15`.
- On main: 9273d482b, b454cf885, 18b5e6710, 968d6218f, f2b75abbd and 2d84eaf4c.
- This commit adds sections 52 and 53.
- No lease is held after this commit.

**Switches this worker added:**

| switch | file | state | section |
| --- | --- | --- | --- |
| `kScriptedOrderNativesBound` (NavigatorEnable, UnitHoldFire squadron arm, EntityTurnToEntity ship arm) | `src/game_hosts_script_orders.cpp` | ON | 49 |
| `kScriptedOrderNatives2Bound` (SetShipMaxSpeed) | same | ON | 50 |
| `kPilotLandNativeBound` (PilotLand -> `land_at_site_0099a3dd`) | same | **OFF** | 50.2 |

**Queue for the successor:**
1. **Flip `kPilotLandNativeBound`** once cc9-lua17's case-insensitive `FindEntity` is on main
   (the lead sends the sha).
   - Pair IJN01 3000: expect `PilotLand: B-17 0n -> AirField 02` and land tasks installed.
   - Pair LOMP10 9200/9000: no call on the n reference; `unitcommand` must answer `land` if
     line 605 is reached.
2. **The periscope sub-state `009E4DC1`** (ranking 11): the `ShipAiSubAttack` `+122Ch` state arm;
   reach is JM06 132 and USNOS 61.
3. **Parked:**
   - `Scoring_SetMissionCompleted`: score only; route it through the mission-frame result host
     if it is ever needed.
   - The planner spawn, which needs a stock scene (section 52).
   - The UnitSetFireStance / UnitHoldFire ship arms: gunnery13 pairs them behind
     `kShipFireStanceBound`, and the callers are in (f2b75abbd).

**Useful files in the cc9-ships15 tree (`local\`):**
- `s15_run.ps1 -Exe <exe> -Prefix <p> -Row tag:MISSION:frames:mission_frames` launches in the
  background with the reference environment.
- `s15_wait.ps1 -Logs <names>` is the foreground wait on the final COM release.
- Pair logs:
  - `s15{off,on}_*`: section 49;
  - `s15{off2,on2}_*`: section 50;
  - `s15p_jm02`, `s15p_ijn11`: section 52.
- `s15_a23980.c` / `s15_a23980.asm`: the repaired `00A23980`.

## 54. The periscope sub-state: the pre-pass writer `009DB8F0` (packet `cc9_submarine_periscope_substate`)

Worker cc9-ships16, 2026-09-29 (stamped 15:05 UTC). Ranking item 11. Ghidra read-only.

**What the ranking row was.** `ShipAiSubAttack::periscope_state_122c [009e4dc1]` counted as
UNIMPLEMENTED (JM06 128 on reference n, USNOS 61). The store itself, `009E4D90`, was already
reconstructed (`src/ship_ai_sub_attack.cpp`) and has been mirrored into the units host's mast
(`00854650`, `kSubmarinePeriscopeOutBound`, ON) since packet `cc9_periscope_out`. The row stayed
a record because the census of `+122Ch` writers was incomplete.

**The writer census.** `local\s16_disp_ctx.py` decodes every `2C 12 00 00` hit on disk (56 hits):
- ship AI: `009E4DDA` (`009E4D90`, the fire step's raise), `009EA8FB` (the fire step's lower) and
  **`009DB9AC` / `009DB9D4` / `009DB9EC` in `009DB8F0`**, which this process never ran;
- the broken state 2: `009327F7` (`BSP_SubmarineUnit_BreakPeriscope`) and `009373E7`
  (`BSP_UnitController_ClearShapeCollisionBits`); units/damage lane, not modelled;
- the unit: `00853CA9` (SEntityInit), `00853FA1` (serialize), `00854ECF` (the repair or
  auto-raise arm of `00854650`);
- the player HUD: `00650402`, `00651799`, `00651D07`, `00651F36`, `00652467`;
- readers: `00852E45`, `00650785`, `0089408B`, `0069265B`, `009E4EED`.
- The `009F1EAF..009F26FA` hits are `ebp+122Ch` on the approach frame's own object, not a unit.

**`009DB8F0`, read whole (`009DB8F0`-`009DB9FA`, RET 4 then INT3).** `__thiscall(holder,
float seconds)`; ECX is `brain+0AC4h`, a 4-byte holder of `[brain+0AB4h]` (the unit). The float
is never read. `009F11DD..009F11F7` allocate the holder only when `unit->vtable[5Ch](8)` is
true at construction, so only a submarine has it. `009F1B57..009F1B70` call it after the
neighbour walk and before the avoidance request, on every pre-pass.
1. `009DB8F6`: `depthLevel [unit+1268h] != 1` -> the lower arm.
2. `009DB916..009DB96C`: world Y `[unit+100h]` (after `00414DB0`) outside
   `float(bands[1] + 2.5)` / `float(bands[1] - 2.5)` (`00CE3DE0` = 2.5 double; JA, so unordered
   stays in) -> the lower arm.
3. `009DB96E..009DB981`: role 1 (`[unit+1B0h]`) must be 8 or `00927F10`-AI-held, else return
   with no write.
4. `009DB985..009DB9AC`: `[ai+2264h]` non-null and `vtable[20h]()` true -> `+122Ch = 1` unless 2.
   Slot `20h` is `009DAA80` (XOR AL,AL) for cruise, stop, follow, land, movetopos, moveonpath and
   attackmove (`00D21598..00D219D0`), `009E4910` (MOV AL,1) for sub_attack (`00D2195C`) and
   `009DB310` (MOV AL,1) for kamikaze_attack (`00D216B8`).
5. `009DB9BB..009DB9D4`: else `00521E70(0)` (role 0 AI-held) -> `+122Ch = 0` unless 2; else no
   write.
6. The lower arm `009DB9E3..009DB9EC`: `+122Ch = 0` unless 2.

**The binding.** `bsp::ship_ai_periscope_prepass_009db8f0` (`src/ship_ai_sub_attack.cpp`), run
from the controller's `replan_prepare_009f1420` for `unit_is_kind_of(8)`, behind
`kSubmarinePeriscopePrepassBound` (`src/game_hosts_ship_ai.cpp`), committed OFF. Both writers
share one store helper, so the units host's mast sees each write. With the switch ON the
`009E4DC1` row reports concrete. A new summary block, `periscope prepass`, lists calls, raises,
lowers and the arm census per submarine; a note line marks each state change.
- LABELLED: role answers the host cannot give (`unit_current_role_slot` or `00927F10`
  unavailable) make no write and are counted as `role_unavailable`; the pose refresh
  `00414DB0` is the host's position; the state-2 guard never fires (no producer).

**Predictions (written before the pair).**
- **JM06 3200/3000.** `PlayerSub 03` raised its periscope once, at 142.10 s, through the fire
  step. ON: the pre-pass raises it on the first pre-pass where the boat is in sub_attack at
  depthLevel 1 within 2.5 of bands[1], so at or before 142.10 s, and lowers it when the fire step
  dives it (level 2/3), so at least one `out=0` line appears. The sensor answer moves from
  PeriscopeIn toward PeriscopeOut for those windows. Any other submarine in cruise at level 1
  already holds 0, so it shows only `lowered` arms with no transition. Deaths: no change
  expected (low confidence; detection of the boat is the only route). Verdict expected: exit 1
  or 3 on sensor counters; flip unless the mechanism misbehaves.
- **USNOS 3200/3000.** `Gato` raised at 8.80 s. ON: raised at or before 8.80 s, lowered when it
  dives. Same death expectation.
- **LOMP06 1200/1000.** The Narwhal is the player's unit, so role 1 is not AI-held: every
  in-band call takes `player_role1` with no write; out of band it writes 0 over 0. Expect
  gameplay identical (exit 0 or 1).

### 54.1 The pairs and the verdict: ON

Same-tree pair on `5ce677984`: the OFF binary is this tree's build and the ON binary is
`local\s16_peri` (`pair_export --flip kSubmarinePeriscopePrepassBound=true`, SHA-256 prefix
`B8759AA30F83`). Reference environment, lockstep 0.05, console session. A 300-frame USN01 smoke
on the ON binary was clean. Logs are `local\s16{off,on}_{jm06,usnos,lomp06}.log` and the diffs
are `local\s16_diff_<row>.txt` in the cc9-ships16 tree.

| row | pair_diff | death rows | what moved |
| --- | --- | --- | --- |
| JM06 3200/3000 | 1 | identical (1) | `009E4DC1` concrete; `009DB8F0` 4211 calls; `00855045` 638 -> 21000 |
| USNOS 3200/3000 | 3 | identical (6) | Gato's periscope goes down at 23.30 s; recon `identified` 18883 -> 18879; 17 unit-table `nearest` values move by 1 to 31 m; gunnery candidate counts move a little |
| LOMP06 1200/1000 | 1 | identical (1) | `009DB8F0` 218 calls; `00855045` 0 -> 1000 |

**The mechanism against the predictions.**
- **JM06: matches.**
  - `PlayerSub 03` and `PlayerSub 02` enter sub_attack at 51.10 s. The pre-pass raises both at
    once (0 -> 1, level 1) and lowers both at 51.35 s, when the boats reach level 2.
  - From 140.85 s the fire step's `009EA8FB` lowers `PlayerSub 03` and the next pre-pass raises it
    again (the notes repeat 0 -> 1). The mast still comes out at 142.10 s, as on OFF.
  - The four other boats (`Narwhal-class Submarine 01` and the two TypeB boats, plus
    `PlayerSub 01`) only write 0 over 0.
  - The `00855045` count grows because each boat's mast arm now runs from its first store. On
    OFF it ran only from the first fire-step store, and in the image every non-kamikaze boat has
    `+1214h` from construction. The lowering arm moves a mast from 0 toward 0, so nothing is
    visible.
- **USNOS: matches, with one correction.**
  - The Gato is raised by the pre-pass at 8.30 s (OFF: the mast is out at 8.80 s from the fire
    step; ON: out at 8.80 s too).
  - Six times from 10.05 s the pre-pass takes the out-of-band arm at level 1. The boat is above
    `bands[1] + 2.5`, a height that `009E4D00` still accepts (`y > bands[1] - 2.0`). The fire step
    re-raises each time until 23.30 s, when it does not, and the mast goes in (`out=0`).
  - On OFF the mast stayed out to the end.
  - The prediction said "lowered when it dives"; it is lowered when it rides above the band.
  - The moved counters all follow from a submerged periscope: the recon pass identifies the boat
    four fewer times, and targeting and the unit-table nearest distances shift with it.
  - Deaths, damage, hits and shots are identical.
- **LOMP06: gameplay identical; the arm prediction was wrong.**
  - The Narwhal is AI-held on role 1 before the player takes it. It selects sub_attack from
    cruise at 2.85 s, is raised at 2.85 s and lowered at 3.10 s (level 2).
  - Later in-band calls take `player_role0` (27) with no write.

**Verdict: flipped ON.** The mechanism matches the image on every row, and the death table is
identical on all three.

**Still open:**
- The broken state 2. `009327F7` (`BSP_SubmarineUnit_BreakPeriscope`) and `009373E7` write it;
  those are the units/damage lanes.
- The repair and auto-raise arm at `00854E44..00854ECF`, in the units host.
- The HUD's player writes.
