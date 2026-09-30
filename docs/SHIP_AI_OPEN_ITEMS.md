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

**`009DB8F0`, read whole (`009DB8F0`-`009DB9FC` exclusive: RET 4 at `009DB9F9` is 3 bytes, INT3 at `009DB9FC`; named `BSP_ShipAi_SubmarinePeriscopePrepass` by the lead, bdcc793eb).** `__thiscall(holder,
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

## 55. The PilotLand flip, re-paired with FindEntity case-insensitive (packet `cc9_pilot_land_flip`)

Worker cc9-ships16, 2026-09-29 (stamped 15:33 UTC). It continues 50.2. The tree carries main's
merge `c6d14ae2f` (`kFindEntityCaseInsensitiveBound` ON), so `Mission.AF2 = FindEntity("Airfield
02")` now resolves to AirField 02.

**Predictions (written before the pair).**
- **IJN01 3200/3000.** Both `PilotLand` calls resolve (`-> AirField 02`). Each routes its B-17
  squadron through `land_at_site_0099a3dd`, so land tasks are installed on the members, unless
  lua16's install core refuses a plane that is not airborne; any refusal is logged. The B-17
  tracks change, and AA kills may move through the shared RNG stream. Judge from the per-entity
  death table.
- **LOMP10 9200/9000.** No `PilotLand` call on the n reference, so identical (exit 0 or 1) unless
  `10_san_jose.lua:605` is reached.

### 55.1 The pairs and the verdict: stays OFF (a host clobber, not a PilotLand defect)

Same-tree pair on `cdb0b0542` (main merged, `c6d14ae2f` included). The OFF binary is this
tree's build, and the ON binary is `local\s16_pl` (SHA-256 prefix `8747823ED741`). Logs are
`local\s16pl{off,on}_{ijn01,lomp10l}.log`; the diffs are `local\s16_pldiff_<row>.txt`.

| row | pair_diff | result |
| --- | --- | --- |
| IJN01 3200/3000 | 1 | Gameplay identical; the death rows (9), the plane death modes and the unit table are identical. |
| LOMP10 9200/9000 | 1 | No `PilotLand` call; only GuiText counts and the known LOMP10 noise move. As predicted. |

**IJN01, the mechanism.** The first half matches the prediction:
- Both calls now resolve: `PilotLand: B-17 0n -> AirField 02 (target object=1 id=2)`.
- `land_at_site_0099a3dd` installs 2 tasks per squadron at 68.20 s: moveto (land) on the leader
  and follow (land) on the wingman.
- The source is recorded as `explicit land command`.

**The failure.** All four tasks retire at 68.30 s with `the squadron's command is no longer land
at this site (009B34D0)`.
- For an explicit site, `land_command_still_valid_009b34d0` (`src/game_hosts_units.cpp`) requires
  `attack_command_class == 00E08FA0` and `command_target_plus_one == land site`.
- `land_at_site_0099a3dd` writes both. Between the install and the retire, the gunnery host's
  command-target refresh (`src/game_hosts_gunnery.cpp`, the `0071EBF0` block) runs again. That
  refresh runs 468 times in the run, and here it follows the squadrons' generation.
- The refresh then calls `units.store_unit_command_target(i, command_target_by_unit[i])` for
  **every** unit.
- `command_target_by_unit` is resolved only from authored command rows (categories 1 and 2, by
  target name). The B-17s have no such row, so their target is overwritten with 0 and the check
  fails.
- **Second observation, not traced (ON only).** At 68.25 s the director's attackmove arm `00836B45`
  runs for the new squadrons (units 330 and 332) with `target=2` (AirField 02), logs `target not
  hostile`, and raises stage 2.
  - So `director.slot_command[0]` reads `kCommandAttackMove` after the land order is delivered.
  - OFF has no such line.
  - Whether the commands host stores the `land` class 00E08FA0 as attackmove is unread; it belongs
    to the commands lane.

In the image the target is read live from the plane's current command (`vtable[178h]` ->
`00521EA0`), so nothing clears it.

**Verdict: `kPilotLandNativeBound` stays OFF (mechanism failure, recorded).** The land install
itself works; the task dies to the host's refresh. The fix is outside this lane.
- The gunnery host's refresh should not overwrite a unit whose command came from a script order.
  One way: store only non-zero resolutions. Another: keep the script-order target in its own
  field and have 009B34D0 read that.
- Routed through the lead to cc9-gunnery13 (the gunnery host) and cc9-lua17 (the units host).
- Re-pair IJN01 after that change. The expected result is B-17 tracks bending toward AirField 02
  after 68.20 s.

## 56. The command-target refresh keeps unauthored targets (packet `cc9_command_target_refresh_keep`)

Worker cc9-ships16, 2026-09-29 (stamped 15:55 UTC). It follows
55.1.

**The binding.** `refresh_command_targets()` (`src/game_hosts_gunnery.cpp`, 0071EBF0's rule)
resolves a target only from a unit's current category 1/2 command row. For every other unit it
writes 0 into the units host's `command_target_plus_one`.
- That field also stands for the current command's own target (`vtable[178h]` -> `00521EA0`).
  `land_at_site_0099a3dd` stores PilotLand's site there, and `009B34D0` reads it.
- Under `kCommandTargetKeepUnauthoredBound`, committed OFF, a unit with no accepted row (no
  current category 1/2 row) is left unwritten.
- A unit whose accepted row names nothing is still written 0. That is the image's rule, because
  the accepted slot's target is returned whatever it holds.
- A summary line counts the skipped stores.
- LABELLED risk: a unit whose last category 1/2 row goes stale, with no replacement, keeps its old
  target where the image's 0071EBF0 would answer the neutral record. The USN01 and JM05 pairs
  test for it.

**Predictions (written before the pairs).**
- **Pair A, keep ON alone (PilotLand OFF).**
  - **USN01 3000 and JM05 3000:** gameplay identical (exit 0 or 1). No authored-row unit
    changes, because nothing but the land paths writes the field non-zero, and a unit keeps a
    target only if its last authored row went stale.
  - **IJN01 3200/3000:** identical. Without PilotLand the B-17s never get a non-zero target.
- **Pair B, keep ON plus `kPilotLandNativeBound` ON, against keep ON alone.**
  - **IJN01:** the four land tasks install at 68.20 s and are **not** retired at 68.30 s.
  - The B-17s then fly the land task's moveto/follow toward AirField 02, so their tracks bend
    after 68.20 s (exit 3 on their unit-table rows).
  - AA contacts may move deaths through the shared RNG stream; judge from the per-entity table.
  - The attackmove-after-land observation (55.1) stays open and may still appear.
  - **LOMP10 9200/9000:** no PilotLand call; identical.

### 56.1 The pairs and the verdict: both ON

Committed-OFF base `e6831c321`. The OFF binary is this tree's build.
- **K1** is `--flip kCommandTargetKeepUnauthoredBound=true` (`local\s16_k1`, `CF7E7E3AB121`).
- **K2** adds `--flip kPilotLandNativeBound=true` (`local\s16_k2`, `2B0BAFCBFE27`).

The logs are `local\s16{koff,k1,k2}_<row>.log`, and the diffs are `local\s16_kdiff_<row>.txt`.

| pair | row | pair_diff | result |
| --- | --- | --- | --- |
| A: OFF vs K1 | USN01 3200/3000 | 1 | identical; only the ship-AI refill counter (known noise) and the new keep count (12766) move |
| A: OFF vs K1 | JM05 3200/3000 | 1 | identical; keeps 308725 |
| A: OFF vs K1 | IJN01 3200/3000 | 1 | identical; keeps 157765 |
| B: K1 vs K2 | IJN01 3200/3000 | 3 | four land tasks install at 68.20 s and **none retires** (`retired_invalid=0`) |
| B: K1 vs K2 | LOMP10 9200/9000 | 1 | no call; GuiText counts and known LOMP10 presentation noise only |

**Pair A, as predicted.** No authored-row unit changes on USN01, JM05 or IJN01. The skipped stores
only avoid writing 0 over 0 there.

**Pair B, IJN01, as predicted.**
- The B-17 leaders fly the land moveto from 4153.6 m (B-17 01) and 4828.7 m (B-17 02).
- By 88.30 s B-17 01 is at 3256 m and descending (alt 1200 -> 988). By 128.30 s it is in mode 3
  at 188 m altitude. At 148.30 s it is 1360 m out.
- Unit-table nearest distances: B-17 01 5378 -> 2496 m, B-17 02 6026 -> 2965 m, and the wingmen
  alike.
- The death rows (9) are identical.
- The `settarget` player commands that reach the B-17s after fixed step 1382 do not displace the land command.
- The land path's own unimplemented rows now run and are recorded: `006C0B50`, `006BD080`,
  `006C5380`, `009C1850`, `009C18C0`, `006C4790`, and the `0099A3DD` arm record. These are
  landing-sequencer and moveto internals on the lua17 side.

**Verdict: `kCommandTargetKeepUnauthoredBound` ON, and `kPilotLandNativeBound` ON on top of it.**

**Open: the attackmove after the land order.** Still seen with both ON:
`attackmove arm 00836b45: unit=330/332 target=2 target not hostile ... stage 2` at 68.25 s.

**A read of the commands host (unmodified).** The row and the slot both keep the land class, so
the attackmove is not how the host stores `land`.
- `issue_script_command` (`src/game_hosts_units.cpp:11370`) calls
  `GameCommandsHost::issue_command_object` (`src/game_hosts_commands.cpp:1996`).
- That builds the row from `class_of(00E08FA0)`, which is `entity_orders.cpp`'s
  `{22, 00E08FA0, 00CFB600, "land", category 3}`.
- It runs the arm chain, and the director's `store_slot` (`src/game_hosts_commands.cpp:1033`,
  0071E764) stores the command object **as given** into `slot_command[slot]`.
- So `slot_command[0] == kCommandAttackMove (00E08F78)` on those two units at 68.25 s comes from
  another push. The squadron's own generated order is the likely source; the commands lane should
  dump `slot_command[]` for units 330 and 332 at 68.20..68.25 s.
- The arm only raises stage 2 on a friendly target; it does not disturb the land tasks here.

## 57. Lua `Kill` on script entities (packet `cc9_lua_kill_script_entity`, ranking 14)

Worker cc9-ships16, 2026-09-29 (stamped 16:21 UTC). The lead granted `src/game_hosts_lua.cpp` for
this packet.

**What the misses are.** `run_kill_008ac5c0` (`src/game_hosts_lua.cpp`) resolves only units, and
counts anything else as `unresolved`. Every miss on the fresh rows is a script entity:
- The counts are IJN01 2, USN01 2, JM05 1, USNOS 3 and LOMP10 1; the logs are `local\s16*` in the
  cc9-ships16 tree.
- The id is 100000 or above (`kScriptEntityIdBase`), and the entity was `created_for=luaDoTimeTable
  think=luaTimetable`.
- The first miss on each row follows `IsListenerActive` and `RemoveListener`. That is
  `luaCamOnTargetExt` (scripts/global/commandhelpers.lua:7830, this installation, mtime
  2024-10-29): `if Mission.CamScript.Dead == false then Kill(Mission.CamScript)`.
- The binding trace lists only first calls, so the later misses are attributed to the same site by
  their shape (timetable entities), not by a trace.

**What the timetable is.** `Mission.CamScript` is `luaCamIngameMovieAuto`'s return value (7785,
7813):
```
luaDelay(luaCamOnTargetExt, callbackTime, ...)
  = CreateScript("luaDoTimeTable", {{false, t}, {luaCamOnTargetExt, 0}}, params)
```
So the Kill is normally the timetable killing itself from its own second entry.
- In the image, 00926D90 sets Dead. `luaTimetable` (scripts/global/timetable.lua, mtime
  2024-07-13) returns at `if this.Dead`.
- In the host, Dead stays false, so `luaTimetable` reaches `ttt[idx][2] == 0` and calls
  `DeleteScript(this)` in the same think.
- The end state is the same (dead, off the think lists), and the rest of `luaCamOnTargetExt` runs
  identically either way.
- **The grep for mission work after the Kill point:** a luaDelay timetable has no entry after the
  callback, so no timetable entry runs after the Kill.
- The one other route is the player skip (`IC_ENDMOVIEPLAY` calls `luaCamOnTargetExt` with no
  table while the timetable waits). There the host leaves the timetable alive and the callback
  fires a second time. An idle player never skips.

**The binding.** Under `kLuaKillScriptEntityBound`, committed OFF, a Kill whose argument's `Ptr`
is a script entity runs `GameScriptOrdersHost::entity_kill_00926d90`. That call already exists and
is reached from other paths; it sets Dead and `+5Eh`, erases the entity from both think lists and
rebuilds the self table.

**Predictions (written before the pairs).**
- `unresolved` goes from 1..3 to 0 on every row, with the same count of `script entity id N
  killed` lines.
- Gameplay identical (exit 0 or 1) on USN04 4700/4500, USN13, JM05, USNOS, IJN01 and LOMP10
  3200/3000.
- DeleteScript's own count may drop by the same number, because the timetable now returns
  before it.
- No movie-camera pose change is expected, because the end state is reached in the same think.

### 57.1 The pairs and the verdict: ON

Same-tree pair on `de409e8e3`. The OFF binary is this tree's build; the ON binary is
`local\s16_kl` (`--flip kLuaKillScriptEntityBound=true`, SHA-256 prefix `1612520D7C82`). Logs are
`local\s16kl{off,on}_<row>.log`, and the diffs are `local\s16_kldiff_<row>.txt`.

| row | pair_diff | unresolved | death rows |
| --- | --- | --- | --- |
| USN04 4700/4500 | 1 | 2 -> 0 | identical (46) |
| USN13 3200/3000 | 1 | 2 -> 0 | identical (31) |
| JM05 3200/3000 | 1 | 1 -> 0 | identical (0) |
| USNOS 3200/3000 | 1 | 3 -> 0 | identical (6) |
| IJN01 3200/3000 | 1 | 2 -> 0 | identical (9) |
| LOMP10 3200/3000 | 1 | 1 -> 0 | identical (0) |

**Against the predictions.**
- Every miss becomes a `script entity id N killed` line, and every row is gameplay identical.
- No movie-camera pose line moves outside LOMP10's known presentation noise.
- Beyond the Kill lines, the only moves are the known ship-avoidance refill counter and the LOMP10
  minimap heading.

**Verdict: ON.** Ranking row 14 is closed.

## 58. Handoff (cc9-ships16, 2026-09-29 16:38 UTC, at about 78% context)

**State.** Branch `agent/cc9-ships16`. Sections 54 to 57 are this worker's. This commit adds the
57.1 flip and this handoff. No lease is held after this commit.

**Switches this worker added:**

| switch | file | state | section |
| --- | --- | --- | --- |
| `kSubmarinePeriscopePrepassBound` (`009DB8F0`) | `src/game_hosts_ship_ai.cpp` | ON | 54 |
| `kCommandTargetKeepUnauthoredBound` (0071EBF0 refresh) | `src/game_hosts_gunnery.cpp` | ON | 56 |
| `kPilotLandNativeBound` (flipped; the switch was cc9-ships15's) | `src/game_hosts_script_orders.cpp` | ON | 56.1 |
| `kLuaKillScriptEntityBound` (Kill -> 00926D90 on script entities) | `src/game_hosts_lua.cpp` | ON | 57 |

**Open items this worker leaves:**
- **Attackmove after land.** At 68.25 s on IJN01, the director's attackmove arm `00836B45` runs
  for B-17 01/02 (units 330/332) with `target=2`. The commands host stores `land` as given
  (56.1), so the attackmove in `slot_command[0]` comes from another push. This is the commands
  lane's item.
- **The now-reached land internals.** `006C0B50`, `006BD080`, `006C5380`, `009C1850`, `009C18C0`,
  `006C4790` and the `0099A3DD` arm. The lead passed these to lua17.
- **The periscope's broken state 2 and the repair/auto-raise arm** (`009327F7`, `009373E7`,
  `00854E44..00854ECF`). These are lua-lane items.
- **Parked from 53:** `Scoring_SetMissionCompleted`, and the planner spawn (it needs a stock scene).

**Useful files in the cc9-ships16 tree (`local\`):**
- `s16_run.ps1 -Exe <exe> -Prefix <p> -Row tag:MISSION:frames:mission_frames` launches in the
  background with the reference environment.
- `s16_wait.ps1 -Logs <names>` is the foreground wait on the final COM release.
- `s16_disp_ctx.py <scan-bytes output> <disp hex>` decodes the instruction around each
  displacement hit.
- `s16_vslot.py <slot hex> <vtables...>` reads a vtable slot from the PE on disk.
- Pair logs: `s16{off,on}_*` (54), `s16pl{off,on}_*` (55), `s16{koff,k1,k2}_*` (56) and
  `s16kl{off,on}_*` (57).

## 59. Does the Sell think really recall JM05's carrier strikes? (packet `cc9_sell_think_carrier_groups`, cc9-ships17, 2026-09-29)

The question is SQUADRON_LAND_TASK 5ad.2's: on JM05, `kReturnToBaseSiteKeyBound` ON removes the US
carrier strikes, because the SELLING tick `00A11FF0` sends `returntobase` to the freshly launched
squadrons from 6.45 s. Is that SELLING assignment the image's? Read-only; no switch is bound.

### 59.1 Answer: yes on JM05, and the carrier groups are not where the squadrons are

**The carrier groups hold no squadrons.** From cc9-lua19's OFF log (`l19_rtb0_jm05l.log`, 9000 frames):
- `USS Lexington` and `USS Yorktown` lead groups of 6: the carrier and the five escorts that the
  `ai diag follow` lines attach to it. `sell ... splits=0`, so `00A2E4C0` never found an air member
  in an owned group at a Sell think.
- **All 17 squadrons sit in one all-air SELLING group led by `F4F Wildcat 01`.** That is the 7
  scene squadrons plus the 10 launched ones (`ai squadron generated after load` x10). Every
  `ai_selling_tick` returntobase row names a member of this group.
- **The image can never put a squadron in a carrier's group.** `00A2C8D0` refuses to merge a group
  with a ship into one with an air member, and the reverse (AI_GROUP_THINK section 2). The Sell
  think's split (`00A22841` -> `00A2E4C0`, then the claim at `00A2284D`) exists to move air members
  out of a mixed group, so that they get their own SELLING.

**Every link from launch to `returntobase` is the image's.**

| link | image | host |
| --- | --- | --- |
| seed | phase 3 walks world list 24, the squadron list (`00A2E8A0 MOV EDI,[EDX+13Ch]`, GAMEPLAY_LOOSE_ENDS_1 section 2). A launched squadron is a candidate on the next pass. | `kGeneratedSquadronBrainBound` |
| join | phase 4 `00A2C8D0`. The absorbed group's command decides: a new group is IDLE, and `00A12450` merges it within `AutoMerge_MergeDist` (650.0, Rookie) of the SELLING air group. An unmerged group is claimed through `00A181A0`; the squadron is groupable (`009FE080`'s 18h arm), so Capture claims it. | same (`auto_merges` is never incremented; all 210 merges print as `prox_merges`) |
| Capture | no enemy list-28 entity: the assignment loop never runs, and each group whose record `[3]` holds an own list-28 entity goes to brain+8h (`00A2AFC0` / `00A2B2AF`). `[4]` is seeded 1.0e10 (`00CE4970`), so any own CommandBuilding qualifies. JM05's three CommandBuildings are all Allied. | `capture_think_00a29fd0`, `handoffs=9` |
| Sell | `00A22800`, read in full here (`00A22800-00A228D2`): SELLING (`00D229B8`) on every owned group without one. | matches |
| air test | `00A2C660` -> `009FE0F0`: `vtable[5Ch](0Fh)`, then `(18h)`. | matches |
| tick | `00A11FF0`'s air arm: `0077D600(returntobase)` per squadron when `+361h` and `+3B0h` are clear. CONTROLLED_UNIT establishes both clear (the byte-store scans are not vacuous). | `ai_selling_tick` |

**JM05's US side does have a brain in the image**, but not for the reason the host gives (59.2):
- `ijn_05_invasion_of_port_moresby.scn` (this installation, mtime 2024-07-13) authors `Player1`
  `Party = Japanese` and `Player5` `Party = Allied`. The mission tree enables only the Japanese side.
- So the local player is Japanese. `009FFD20` gives the US units slot 4, and `009FFE50` admits slot
  4 (59.2).
- The slot-4 brain's `brain+24h` is `[[game+18CCh]+4*4]+28h` (`00A15A90 CALL 009FFD60`), Player5's
  Allied, team 0.

**Verdict.** On JM05, SELLING on the US air group, and `returntobase` to every launched US squadron
on each command tick, is the image's rule. `kReturnToBaseSiteKeyBound`'s JM05 9000 move (US strikes
gone, deaths 29 -> 3) is what the image predicts from these inputs. The flip is not blocked by this
question; ranking #3 (the carrier deck) still decides where the recalled planes go.

LABELLED, not established here:
- In retail, jm05.lua's airstrike managers (`commandhelpers.lua`, mtime 2024-10-29 in this
  installation) re-issue `PilotSetTarget`, and SELLING re-issues `returntobase` every command tick.
  The outcome of that tug is not predicted here.
- `00A12450`'s distance and strength tests are taken from their earlier reads (`ai_command_can_merge_with`).

### 59.2 New gap: the host gives the brain to the wrong side in single player

`009FFE50 BSP_Ai_IsPartyAiEnabled`, `__fastcall(slot)`. Its second arm is read here for the first
time; AI_GROUP_THINK's host table had it as "unread, contract: partial":

```
009FFE59  CMP byte [game+61Ch],0 / JE 009FFE84     ; the forced-mode byte
009FFE62  CALL 004BCA50 / CMP EAX,3 / JA -> 1      ; forced: modes 4..: every slot
009FFE6C  slot == 0 or slot == 4 -> 1, else 0      ; forced: modes 0..3
009FFE84  CMP dword [game+1FE4h],0 / JNE 009FFE96  ; not forced, single player:
009FFE8D  XOR EAX,EAX / TEST ESI,ESI / SETNE AL     ;   slot != 0
009FFE96  slot <= 7: JMP 004B5510([game+18CCh+4*slot])   ; multiplayer: the slot record
```

- **`game+61Ch` is 0 in a campaign.** Its writers are `004BC890 BSP_Game_SetGameMode`
  (`004BC894`) and the game constructor (`004DDFB7`); `scan-bytes 88 ?? 1c 06 00 00` finds those
  two, and no `C6` form exists.
  - `004BC890`'s `forced` is 1 only from the command-line switches in `004E27E0`
    (`CL_duel`, `escort`, `competitive`, ...).
  - `BSP_Session_SetMode` pushes `EBX`, which is 0 from `0076FC97` onward.
  - The in-mission call `004D548A` pushes 0.
- **So in single player `004BCA50` is 8, and the brain runs for every slot except 0.**
  - `009FFD20` puts the local player's team in slot 0 and the other team in slot 4 (AI_BRAIN_PLAYER_EXEMPTION).
  - **The player's own side is never planned for; the enemy side is.**
  - The group constructor asks the same gate (`00A2E124`): NONCONTROL (`00D22990`) for slot 0,
    IDLE (`00D229E0`) for slot 4.
- **The host uses `kCampaignGameMode = 0` (a labelled substitution) and the forced arm, with
  `party = team`** (`game_hosts_ai.cpp`, `create_group`: "files a group under its own team"). So
  team 0 gets the brain and team 1 gets NONCONTROL, on every row:
  - JM05 and IJN01, where the player is Japanese: the image's slot-4 brain is the US brain, as
    in the host.
  - USN04, USN13, USN01 and USN02, where the player is US (LOMP10 not checked): **the image commands the
    Japanese and leaves the US alone.** The host commands the US and leaves the Japanese NONCONTROL.
- **Corrections.**
  - AI_BRAIN_PLAYER_EXEMPTION's "no player exemption" is wrong in single player. The exemption is
    the party gate itself, which covers the player's whole side, not only the helm.
  - AI_COORDINATOR_TICK's "only side 0 gets a brain ... the shipped rule" was measured on the host.
- **Open before binding:**
  1. USN04's `usn_19_coralus.scn` authors `Player5` `Party = Allied`. The slot-4 brain would then
     carry `brain+24h` = Allied while commanding Japanese groups, unless slot 4's record `+28h`
     is written from somewhere else in single player (`004BB440`, `004C6890`).
  2. `004BCA50` = 8 also gates phase 5 (skipped above mode 3) and every other mode read in the AI.
- This belongs at or near the top of GAMEPLAY_GAP_RANKING. It decides which side's ships the
  whole AI lane moves.

## 60. The single-player party gate (packet `cc9_ai_party_gate`, cc9-ships17, 2026-09-29)

This follows 59.2. The switch is `kAiPartyGateUnforcedBound` in `src/game_hosts_ai.cpp`, committed OFF.

### 60.1 The image: which slot plans, and under which team

- **The gate, `009FFE50`** (read in full, `009FFE50-009FFEAB`). In a campaign `game+61Ch` is 0
  (59.2), so the arm at `009FFE84` answers `slot != 0` in single player.
- **A unit's slot, `009FFD20`.** `+180h` in 0..7 names that slot; 8 (`PLAYER_AI`, "AI control")
  gives -1. The unauthored 9 gives 0 on the local team, else 4. The local team is
  `[game+18CCh + 4*[game+18ECh]]+28h`:
  - `004DFB70` (`004DFD57..004DFD77`) points slot 0 at the participant that `004BB440` built,
    copies `game+1030h` (slot record 0's `+28h`) into it, and stores `EBX` into `game+18ECh`.
    EBX is taken as 0 (the local index, as CONTROLLED_UNIT labels it).
  - So the local team is Player1's `Party`.
- **A brain's team.** `00A15A70` stores the slot at `brain+20h`, and at `00A15A90..00A15A97` it
  stores `009FFD60(slot)` = `[game+18CCh + 4*slot]+28h` at `brain+24h`.
  - Slots 1..7 still point at the slot records that `004BB160` set (`game+1008h + i*118h`).
  - Their `+28h` is side block `+0h`, which `004C6890` copies from the scene's
    `MultiPlay.PlayerN` `Party` (SCENE_RECORD_SIDE_BLOCKS). The slot-4 brain's team is Player5's
    `Party`.
- **A group's command.** The group constructor `00A2DFA0` stores `009FFD20` of its first member at
  `+5634h`. A negative slot gets NONCONTROL. Otherwise `00A2E124` asks `009FFE50`: IDLE when the
  slot is admitted, NONCONTROL when it is not.
- **Other mode readers.** `004BCA50` = 8 skips phase 5 (`00A2EB08`, `JA` above 3). The tuning
  record is Rookie at 8, as at 0 (`009FFC92`).
- **Enum ordinals, LABELLED.** `Party` is taken as this installation's `PARTY_ALLIED 0` /
  `PARTY_JAPANESE 1` (`luamw_init.lua` 73-74). The native enum table was not read.

**The reference rows.** Player1 and Player5 are from this installation's scenes; the mtimes are
2024-07-13 to 2024-10-29.

| rows | scene | Player1 (local) | Player5 (slot-4 brain team) | image: planned side, brain team | host OFF: planned side |
| --- | --- | --- | --- | --- | --- |
| USN04, USN04 E2 | usn_19_coralus | Allied | Allied | Japanese (slot 4), team Allied | US (party 0) |
| USN01 | usn_1_marshall | Allied | Allied | Japanese, team Allied | US |
| USN02 | usn_2_java | Allied | Allied | Japanese, team Allied | US |
| USN12 | usn_12_augusta | Allied | Allied | Japanese, team Allied | US |
| USN13 | usn_13_truk | Allied | Allied | Japanese, team Allied | US |
| USNOS, USNOS long | us_osumi | Allied | Allied | Japanese, team Allied | US |
| LOMP06 | 06_crucial_cargo | Allied | Allied | Japanese, team Allied | US |
| LOMP10, LOMP10 long | 10_san_jose | Allied | Allied | Japanese, team Allied | US |
| BSM01 | bsm_01_stationed_at_pearl | Allied | Allied | Japanese, team Allied | US |
| JM05, JM06, JM08 | ijn_05, ijn_06, prcpijn_08 | Japanese | Allied | US (slot 4), team Allied | US (party 0) |
| IJN01 | ijn_1_pearl | Japanese | Allied | US, team Allied | US |

**The consequence on the US-player rows.** Every one of them authors Player5 = Allied, so the
slot-4 brain carries team 0 while its groups are Japanese.
- Its planners' own side is Allied (planner `+30h`) and their enemy side is Japanese (`+34h`).
- So Capture's targets and Attack's candidate groups are the Japanese groups that the same brain
  owns.
- This follows from the reads above. It is the most surprising part of this packet, and the ON
  census (the group table and the planner lines) is what shows whether the host reproduces it.

**The host's controlled unit.** The gate takes the local team from Player1, never from the
controlled unit. On JM05 the host still controls USS Phelps (Allied), but the gate's local team is
Japanese. Nothing else in this packet reads the controlled unit.

LABELLED:
- `+180h` is taken as 9 for every unit. JM05 (x4), JM08 (x1), LOMP10 (x3), USN12 (x4) and USNOS
  (x2) author `OwnerPlayer = "AI control"` inside `MultiType` blocks, which would give -1.
- The multiplayer arm is not modelled.

### 60.2 The binding

With `kAiPartyGateUnforcedBound` ON:
- **The published slot parties.** The mission host publishes the eight `PlayerN` parties that it
  already reads with `read_scene_record_slot_table_004f1d70`, through
  `ai_publish_scene_slot_parties`.
- **Group party.** `create_group` sets `party = 009FFD20(team)`, which is 0 or 4, and files the
  group in a per-party list (`00F8A9E8 + p*0Ch`). The brain walks that list. The eviction
  pass compares the member's slot the same way.
- **The command a group is born with** comes from `009FFE50`'s unforced arm: slot 0 gets
  NONCONTROL. The think pass's brain gate asks `009FFE50` the same way.
- **The brain's team.** The ticking brain's team (`current_team`) is its slot's `PlayerN` Party.
  - It is used as the planner's own team by `in.own_team`, Attack's own and enemy teams, and the
    Capture and Defend sides. The brain itself is still looked up by slot.
  - `brain+24h` (`world_set`) is that team.
- **`game_mode()` is 8.**
- **OFF** is the old rule: mode 0, the forced arm, and party = team.
- **The census.** The creation line `ai party gate bound=... local_team=... slot_teams=...`, and
  the summary line `summary mission ai party gate bound=... local_team=... slot4_team=...
  local_slot_groups=... other_slot_groups=...`.

### 60.3 Predictions, written before any ON run

- **OFF** is gameplay-identical to main on all six standard rows (exit 0 or 1). It only adds the
  two census lines.
- **ON, every row:** `game_mode=8`, and the party table shows `ai party 0 ... ai_enabled=0
  brain=0` and `ai party 4 ... ai_enabled=1 brain=1`.
- **USN04, USN01, USN13 (US player), ON: exit 3, large movement.**
  - The US groups are `party=0 command=NONCONTROL`, and no brain order reaches a US unit. The
    brain orders issued on OFF to US ships and squadrons (for example Enterprise's CAUTIOUSATTACK
    on USN13, USN04's escort attack) disappear.
  - The Japanese groups are `party=4`, born IDLE and claimed by the slot-4 brain, with the
    planner lines' `enemy_groups` counting team 1.
  - USN04 has no CommandBuilding. Capture has no target and no own CB, so it hands to Attack
    (brain+4h). Attack scores team-1 groups, that is, Japanese groups ordered against Japanese
    groups.
  - USN01 and USN13 have Japanese CommandBuildings. Capture's targets are those buildings (`+54h`
    != 0), assigned to Japanese groups.
  - Deaths, damage and first hit move (exit 3).
- **JM05 and IJN01 (Japanese player), ON: exit 0 or 1 on gameplay.**
  - The US groups move from party 0 to party 4 with the same team. The Japanese groups move from
    party 1 to party 0, still NONCONTROL.
  - The group table's `party` column and the party rows change.
  - **The risk:** the party pass visits slot 4 after slots 0..3. If the brain's think draws from
    the shared generator in a different order relative to the other slots' interval draws, the
    ordering moves and so do deaths (then exit 3, with the mechanism unchanged).
- **LOMP10: exit 1 or 3.** There is no combat on the reference row. Whether its three US
  squadrons (the controlled `B-25 01`) move depends on the brain orders they got OFF.

### 60.4 Measured (same-tree pairs on `e90151156`)

- **The builds.** OFF is `local\s17_off` (`pair_export --flip kAiPartyGateUnforcedBound=false`,
  SHA-256 prefix `5864662509BB`). ON is `local\s17_on` (`C948E5DDB894`).
- **The launch.** Reference p's form, with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`
  (`local\s17_runs.ps1`).
- **The smoke** (USN01, 300 frames, ON) passed renderer init.

| row | pair_diff | deaths | damage | first hit |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 48 -> 50 | 16260.6 -> 11938.8 | 100.35 -> 98.70 s |
| USN01 3200/3000 | 3 | 5 -> 5 | 2786.4 -> 3885.8 | 51.45 -> 53.65 s |
| USN13 3200/3000 | 3 | 31 -> 22 | 9241.9 -> 6740.5 | 89.90 -> 98.70 s |
| JM05 3200/3000 | 3 | 0 -> 0 | identical | identical |
| IJN01 3200/3000 | 3 | 9 -> 6 | 3515.1 -> 3324.0 | 80.15 -> 83.25 s |
| LOMP10 3200/3000 | 3 | 0 -> 10 | 0.0 -> 3500.0 | - -> 90.50 s |

**The mechanism, from the ON census. It holds on every row.**
- **The gate.** `game_mode=8`. `ai party 0 ... ai_enabled=0 brain=0` and `ai party 4 ...
  ai_enabled=1 brain=1` on all six rows. `local_team` is 0 on the US rows and 1 on JM05 and
  IJN01; `slot4_team` is 0 everywhere.
- **US-player rows: the player's side is left alone.**
  - Every US group is `party=0 command=NONCONTROL claimed=0`, including USN04's Lexington and
    Yorktown groups, USN01's Enterprise and Northampton, and LOMP10's CB4.
  - The Japanese groups are `party=4` and claimed:
    - USN01: CB2 and the coastal guns DEFENDPOSITION; Convoy1 MOVETOATTACK; Convoy3 and Katori
      CAUTIOUSATTACK.
    - USN13: the CBs DEFENDPOSITION; the Maru groups MOVETOATTACK, CAUTIOUSATTACK or
      CLOSEATTACK.
    - LOMP10: Ashigara, Oyodo and the destroyers SELLING (own CB4 present, no enemy CB).
- **The Allied-team slot-4 brain acts as predicted.** On USN04 every `ai group target value`
  line pairs a Japanese group with itself (`D3A Val #1.1 -> D3A Val #1.1` x26, `#5.1` x15,
  `movieval` x7). Attack's candidates are team 1, the brain's own groups.
- **JM05 and IJN01 (Japanese player): the same side is planned.** The US groups are now party 4
  under team 0, and the Japanese groups are party 0 NONCONTROL, as they were as party 1.
  - JM05's combat is identical: deaths 0, the 39 hits, the damage and the first hit. Only paths
    move (USS Phelps 2567.65 -> 2561.04 m). Section 59's SELLING table is unchanged apart from
    the party column.
  - IJN01 moves through the brain's cadence: `thinks` 38 -> 39, because slot 4 keeps its own
    think timer and draws its interval at a different place in the party pass. Five A7M deaths
    re-time, and four A7M_1 deaths become one A7M_5 death.
  - This is the RNG and timing risk named in 60.3, not a change of side.
- **LOMP10.** The US squadrons (B-25 01, Warhawk 01, Lightning 01) no longer get brain orders.
  They fly their script tasks into the Japanese force, which shoots down ten of them from 99.4 s
  (killers Kiyoshimo, Asashimo, Ashigara).

**Spread against 60.3.** USN04, USN01 and USN13 moved as predicted. JM05 and IJN01 moved on
timing only, the named risk; JM05 combat is identical. LOMP10 moved (exit 3, as allowed).

### 60.5 Verdict: flipped ON

- The mechanism matches the image reading on every row. That covers the gate arm, the slot
  mapping, the brain's team, and the NONCONTROL birth of the local side's groups.
- **What the flip does to the baseline.** On every US-player row the brain stops commanding the
  player's own ships and squadrons. The Japanese side is planned instead, under a brain whose
  team is Player5's `Party`.
- **The link to re-check if the result looks wrong in play: the slot-4 brain's team.** Every
  US-player reference scene authors Player5 = Allied, so that brain plans Japanese groups
  against Japanese groups. The chain is:
  - `00A15A90 CALL 009FFD60` gives `[game+18CCh+4*slot]+28h`;
  - `004BB160` points slot 4 at slot record 4;
  - `004C6890` copies Player5's side block `+0h` into its `+28h`.
- **LABELLED:**
  - the Party ordinals come from `luamw_init.lua`;
  - the local index is taken as 0;
  - `OwnerPlayer "AI control"` is not modelled.
- **Unchanged:** the host's controlled unit (JM05 still controls USS Phelps). The gate does not
  read it.

### 60.6 The brain-team chain, re-read (packet `cc9_ai_party_team_chain`, cc9-ships17, 2026-09-29)

The lead held the flip. The concern: an Allied-team brain planning the Japanese groups against
each other would break every US campaign mission. The switch is back OFF in `1b2660639`, and
60.1-60.5 stand as the record.

**The census method.** Each writer set below comes from a linear capstone sweep of `.text` in the
PE on disk (`local\s17_slotwrites.py`) plus a rel32 and abs32 scan (`local\s17_rel32.py`), not
from Ghidra xrefs. The sweep reports:
- stores whose disp32 is a mission record's `+28h` (`game+1030h + i*118h`), a player record's
  `+28h` (`game+770h + i*118h`), a slot pointer (`game+18CCh..18E8h`) or `game+18ECh`;
- stores to `[r+28h]` or `[r+24h]` within 40 instructions of `r` being loaded from `[..+18CCh]`.

The loop writers, which go through an interior cursor, come from the reset and copy bodies read
in full (SESSION_PARTICIPANT_AI_FLAG, SCENE_RECORD_SIDE_BLOCKS).

**`game+18ECh`, the local slot.** It has four writers:

| writer | effect | reached in single player |
| --- | --- | --- |
| `004DDFB7`/`004DE0DB` game constructor | initial | yes |
| `004BB32A` in `004BB160` reset | -1 | yes, from `004DFD18` |
| `004DFD77` in `004DFB70` | `EBX` | yes. `EBX` is zeroed at `004DFB91`; on the SP branch (`004DFC13 CMP [game+1FE4h],EBX / JZ 004DFD16`) it is only compared and pushed before `004DFD77`. The only rewrite, `004DFCD4 MOV EBX,EAX`, is on the session branch, which re-zeroes it at `004DFD0F`. **The local slot is 0, now read, not assumed.** |
| `004B4744` setter `004B4740` | argument | only from `0076C915`/`0076C98E` (`0076C840`) and `00770609`/`007706E7` (`007705A0`). Those are session join and leave routines reached from `00777850`, which the pump calls only in session modes 1 and 2 (SESSION_MESSAGE_DISPATCH), and from the multiplayer lobby screens (`005D580E`, `005D9E71`) |

**A record's `+28h` (Party), and the slot pointers.**

| writer | effect | reached in single player |
| --- | --- | --- |
| `004BB160` reset, first loop | each mission record `i < MaxPlayerNum`: `+8h`/`+9h`/`+0Ah` = 1, and `+24h`/`+28h` from `MultiPlay.PlayerN` | yes (`004DFD18`) |
| `004C6890` | the same side-block copy | yes (scene selection) |
| `004BB160` pointers `004BB175..004BB1C3` | slot `i` -> mission record `i` | yes |
| `004DFD5C..004DFD74` | slot 0 -> the claimed player record 0, whose `+28h` = `game+1030h` (mission record 0's Party) | yes |
| `004BB630` bind / `004BB660` restore | repoint a slot, and write the new record's `+28h` | callers `0076C92C`, `007706D3` / `0076C8B3`, `00770637`, `007734BB`: session paths only, as above |
| `004BC890` SetGameMode, `004BC8DF..004BCA08` | every record's `+28h` = 0 or CompetitiveModeParty | only when `game+1FE4h != 0` (docs/MISSION_SCENE_CONTENTS) |
| `006EDB40` (`006EDB65`) | the local record's `+28h` toggled 0 <-> 1, then the unit lists rebuilt | only from `006EF12C`: game state 0Dh, input action 0Dh pressed (`004C43C0`). A debug side swap |
| `008C8560` (`008C87E8`, `008C882F`) | the local record's `+28h` written | the Lua `class_enumeration` entry in the table at `00E0C860`, next to `debugtrap`. A debug console command |

Nothing on the single-player load or in the mission tick overwrites `+28h` from the campaign
side, the chosen nation or the mission tree's side blocks:
- the IJN missions' `japanese enabled=1` reaches the game only through `Player1 = Japanese` in
  their scenes;
- no installed script calls `AIEnable` (`rg -i AIEnable scripts/` finds nothing).

**The enable byte agrees.** `00A32DF0` (a vtable method, `00D23240`) sets each party record's
enable byte `[00F8A8C8 + p*1Ch]` to `slot record +8h && +9h && +0Ah` (`00A32F36..00A32F53`).
- In single player slot 0 is the player record, whose AI byte `+9h` is 0 after the reset.
- Slots 1..7 are mission records with 1.
- So the image refuses slot 0 twice: through this byte, and through `009FFE50`.

**Enemy-ness in the target choice.** `00A1CB80` walks `g_aiGroupsByTeam[planner+34h]`
(`00A1CBF5 MOV EAX,[ESI+34h]`, list `00F8AA48 + t*0Ch`):
- `00A1EE50` stores `planner+30h = brain+24h` (`00A1EEA6`) and `planner+34h = (brain+24h == 0)`
  (`00A1EEB2..00A1EEC1`);
- a group's `+5638h` is its first member's `+54h` (`00A2E042..00A2E045`), and `+5634h` is
  `009FFD20` (`00A2E03C`);
- the head of the candidate loop (`00A1CC3B..00A1CC65`) has no own-group, party or `IsEnemy`
  filter before `00A0F970` scores the candidate.

So on a US-player row, the slot-4 brain's candidate list is team 1, its own groups.

**Verdict: the chain is confirmed.** No writer on the single-player path replaces Player5's
Party in the slot-4 record, the local slot is 0 by the listing, and the target choice takes
enemy-ness only from `brain+24h`. The 60.4 mechanism, Japanese groups scored against Japanese
groups on USN04, is the image's rule as far as the listings go.

What the self-pairing does in play, not established here:
- A CLOSEATTACK or ATTACK aimed at a friendly group ends in `attackmove` orders whose fire is
  refused by the gunnery side tests.
- The enemy ships of the US campaign are largely driven by their mission scripts (`NavigatorMove*`,
  `PilotSetTarget`), which the brain's orders then contend with.
- "Trivially broken" is therefore a play-level claim this packet cannot settle from the listings.
  The remaining cross-check is an original-exe observation, which this lane may not run.

Still LABELLED:
- the Party ordinals (`luamw_init.lua`);
- `OwnerPlayer "AI control"` on 14 authored entries.

The switch stays OFF for the lead's decision after reference q.

## 61. Ranking #5: the group score-list release `00A2B8F0` (packet `cc9_group_score_list_release`, cc9-ships17, 2026-09-29)

This is GAMEPLAY_GAP_RANKING #5 (6 of 6 rows, 58583 calls). Section 21 read the body. This packet
answers what was left open there: does anything read the list?

### 61.1 The image

**The list lives in the group object.**
- The records are at `group+24h..+5623h`: up to 128 records of `0ACh` bytes. The key (the
  candidate group) is at `+0A8h` and a mark byte at `+0A4h`.
- The count is at `group+5624h`, and `+5608h`/`+560Ch` of the sub-object (`group+562Ch`/`+5630h`)
  hold its state.

**The census.** `scan-bytes` covers every disp32 of `5600h`, `5608h`, `560Ch` (sub-object
relative), `5624h`, `5628h`, `562Ch` and `5630h` (group relative); `local\s17_rel32.py` covers
callers.

| site | role |
| --- | --- |
| `00A2B8F0` | removal (compose drain `00A2E784`, and `00A2B950`'s first step) |
| `00A2B950` | sorted insert, from `00A1CE93` (ChooseAttackTarget's gated arm) and `00A2A77C`, `00A2A844`, `00A2AD58` (CaptureThink) |
| `00A2BCC0` | marks `+0A4h` = 1 on the record for a candidate, from `00A1CEFF` and `00A2AD61` |
| `00A1CB9C`, `00A2A148`, `00A2A29B` | the planners clear the count (and set `+5628h` = -1) |
| `00A2DFFF`, `00A2B8A4` | the constructor and the sub-object initialiser |
| `00A2CDD0` | **the only reader of the records**. It walks them (`00A2CF77..00A2D321`) and builds " -- " text |

**The reader is never reached.**
- `00A2CDD0` is called only from `00A2D580` (`00A2D87D`).
- `00A2D580` is called only from `00A3109E`.
- That call is inside the body starting `00A31000`, which runs only when the interface
  manager's `[[00E198C4]+54h]+5` byte is set (`00A31015..00A31025`).
- `00A31000` has **no rel32 caller and no abs32 occurrence** in the image. The previous function
  ends with `RET 4` at `00A30FFD`, so this is a function start, and nothing references it: an
  unreached debug overlay.

**Verdict.** The list has no gameplay reader. The 58583 calls of `release_group_reference` are
the removal from a list this process has no reason to keep. The ranking's "groups are not
released" is wrong: groups are released by the host's `destroy_group`, and commands aimed at
them are re-installed as IDLE (`kAiTargetGroupDestroyedIdleBound`, section 24).

**A lead for the AI lane, not this packet.** `group+5628h`, next to the count, is read by gameplay.
- `00A2DB50` is the group's observer slot (vtable entry `00D23078`), `RET 0Ch`. On event 6, with an
  argument different from `group+5628h`, it removes the notifying entity through
  `00A2D9D0 BSP_AiGroup_RemoveEntity`.
- The planners write `+5628h`: -1 at `00A1CBA6` and `00A2A14E`, and 8 at `00A2A786` after a
  Capture insert.
- Whether the host models that event-6 removal was not checked here.

### 61.2 The binding

- `kAiGroupScoreListReleaseBound` (`src/game_hosts_ai.cpp`), committed OFF.
- ON: `release_group_reference` reports `00A2B8F0` as concrete. It keeps no list, since nothing
  reads one. The command-target census it already carried is unchanged.

### 61.3 Predictions, written before any ON run

- Every row is **exit 0**, byte-identical gameplay.
- The only difference is the native table: `AiGroups::release_group_reference 00a2b8f0` moves
  from UNIMPLEMENTED to concrete with the same call count, and the unimplemented total falls by one.
- Pairs: JM05 and USN13 3200/3000, the two largest callers.

### 61.4 Measured and verdict: flipped ON

- **The pair.** OFF is `local\s17_off` (`15FFE787E6C2`) and ON is `local\s17_on` (`E062363F4075`),
  both exported from `b4c2d4cc1`, in reference p's launch form.
- **A lost run.** The session locked at about 16:21 local. The first OFF JM05, OFF USN13 and ON
  JM05 died at renderer init (`hr=0x8876086a`, `logonui=1`), and the first ON USN13 lost its
  presents from frame 1974. All four were re-run once a 300-frame smoke passed, and the cited logs
  are the re-runs.
- **JM05 3200/3000: exit 1, gameplay identical.** The native table changes one status,
  `AiGroups::release_group_reference 00a2b8f0 UNIMPLEMENTED -> concrete`, with calls 22079 on
  both sides.
- **USN13 3200/3000: exit 1, gameplay identical.** The same status change, with 10026 calls.
- **Against 61.3.** 61.3 predicted exit 0; the rows are exit 1. The only other moved lines are the
  known same-binary `ship ai free` / `refills` counters. The mechanism matches.
- **Flipped ON.** GAMEPLAY_GAP_RANKING #5 can be retired: the routine was a removal from a list
  that has no gameplay reader, and group release itself was already the host's.

### 60.7 Re-paired after reference q, and flipped ON (packet `cc9_ai_party_gate_flip`, cc9-ships17, 2026-09-29)

**The tree.** `0baeaad27` is main `560d11765` (reference q landed) merged into this branch,
together with section 61's score-list flip, which has no gameplay effect.
- OFF is `local\s17_off` (`4CC72D010B94`) and ON is `local\s17_on` (`D0E2A3BB0C5E`).
- The launch form is reference q's, which is p's (`local\s17_runs.ps1`). Every log has the
  module directory of its export and the final COM release, and none shows a renderer or present
  failure.

**OFF against reference q** (`local\g15_rq_<row>.log`, main `83b528811`):
- JM05, IJN01 and LOMP10 are exit 1.
- USN04, USN01 and USN13 are exit 3. They move because of main's own commits after
  `83b528811` in `src/game_hosts_units.cpp` and `src/torpedo_task_arm.cpp`
  (`kTorpedoRunTimeUpdateBound` and `kMoveToArrivalEndCommandBound` ON, and cc9-lua19's deck
  part 1 OFF), not because of this branch: its two switches are OFF (60) and gameplay-neutral (61).
- So the flip's expected movement below is measured against this OFF, not against the q rows
  directly.

**The pairs repeat 60.4 number for number.** Every OFF and ON value equals section 60.4's.

| row | pair_diff | deaths | damage | death table OFF -> ON |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | 3 | 48 -> 50 | 16260.6 -> 11938.8 | 48 changed (re-timed), plus A6M Zero #7.2 and D3A Val #7.1\|.-3 only ON |
| USN01 3200/3000 | 3 | 5 -> 5 | 2786.4 -> 3885.8 | five rows change victim: OFF loses Airfield2, Multi Hangar, the Containers and Oil Tank group; ON loses ConTBD1 and ConTBD2 with their wingmen |
| USN13 3200/3000 | 3 | 31 -> 22 | 9241.9 -> 6740.5 | nine `bruh` Kate deaths (#1.4 x4, #1.5 x3, #1.9 x2) only OFF; 22 re-timed |
| JM05 3200/3000 | 3 | 0 -> 0 | identical | identical (paths only) |
| IJN01 3200/3000 | 3 | 9 -> 6 | 3515.1 -> 3324.0 | A7M_1 x4 only OFF, A7M_5\|.-4 only ON, five A7M_7 re-timed |
| LOMP10 3200/3000 | 3 | 0 -> 10 | 0.0 -> 3500.0 | ten only ON: B-25 01 x2, Warhawk 01 x4, Lightning 01 x4 (killers Kiyoshimo, Asashimo, Ashigara) |

**The mechanism matches 60.4-60.6.**
- Every ON row reads `ai party 0 ... ai_enabled=0 brain=0` and `ai party 4 ... brain=1`.
- `local_team` is 0 on the US rows and 1 on JM05 and IJN01. `slot4_team` is 0 everywhere.
- The slot-4 brain's thinks, claims and commands are the same as 60.4's on every row.

**Flipped ON.** These death tables are the expected movement of the next reference against q.

### 60.8 The re-pair stands on main `1a3978da3`, and the `+5628h` lead

- **The re-pair stands.** Main `1a3978da3` (section 61 landed) is merged into this branch as
  `c4e158756`. Against the 60.7 pair tree `0baeaad27`, the only `src`/`include`/`cmake` change is
  the flip itself (`src/game_hosts_ai.cpp`, 2 lines). So 60.7's six pairs and death tables are
  the pair for this main, and no re-run was needed.
- **The `group+5628h` lead (61.1).** The host does not model it:
  - `src` has no reference to `00A2DB50`, `00A2D9D0` or `+5628h`.
  - The only nearby host method is `clear_group_target_cache`, a `done()` with no state.
- **The image.** The group's observer sub-object at `group+10h` (vtable `00D2306C`: `00A2D570`,
  `00A2DA60`, `00A2BD40`, `00A2DB50`, `0042B140`) is registered on every member by
  `00A2D8E0` (`00A2D906`). Its slot `+0Ch`, `00A2DB50` (`RET 0Ch`), handles an event 6 whose value
  differs from `group+5628h` by removing the notifying member from the group:
  `00A2D9D0(member, 1)`, with ECX = the group.
- **The planners set `+5628h`:** -1 at `00A1CBA6` and `00A2A14E`, 8 at `00A2A786`.
- **Not found:** who notifies slot `+0Ch` with event 6. The slot-04 and slot-08 selectors
  `00696330`/`00696340` are documented (OBSERVER_EVENT_PRODUCER); no slot-0C selector was found
  in this pass.
- **Why it matters.** If event 6 is "a new order reached this unit" and the value is the order's
  source, a scripted order (for example JM05's `PilotSetTarget`) would take a squadron out of its
  AI group. Section 59's SELLING recall would then stop reaching it. That is a hypothesis to test,
  not a finding.

## 62. Handoff (cc9-ships17, 2026-09-29, at about 78% context)

**Landed on main:** 59, 60 (the party gate, OFF), 60.6, 61 (the score list). **Unlanded on
`agent/cc9-ships17`:**
- `d17310fda`: `kAiPartyGateUnforcedBound` flipped ON (60.7), pair on `0baeaad27`;
- `c4e158756`: main `1a3978da3` merged;
- this section.

**State of the lane:**
- **Section 59.** JM05's SELLING recall of the US strikes is the image's rule, provided the
  squadrons stay in the SELLING air group. `kReturnToBaseSiteKeyBound` is not blocked by it.
  The 60.8 lead could change the membership premise.
- **Section 60 (the party gate).** In single player the image plans slot 4 (the non-local side,
  under Player5's Party) and never slot 0. It is ON in the unlanded `d17310fda`. The expected
  movement against reference q is the 60.7 death table.
- **Section 61 (ranking #5).** Retired: `00A2B8F0` maintains a debug-only list.

**Queue for the next ships worker:**
1. **The `+5628h` event-6 removal** (60.8). Find the notifier of group observer slot `+0Ch`: scan
   for callers that load an observer table entry at `+0Ch` with three pushes and `RET 0Ch`
   callees, and look for a slot-0C twin of `00696330`/`00696340`.
   - Decide what event 6 and its value are.
   - Bind the removal OFF if the host lacks it.
   - Predict JM05, USN13 and IJN01 first: they are the rows with scripted orders to AI-group
     members.
2. **The approach retarget arm `009F2124-009F272D`** (ranking #4). SHIP_AI 27 kept it OFF because
   the zone key was 0 in single player.
   - It now has reach: IJN01 6071 of 6100 frames, JM05 9000 3845.
   - Re-check that verdict against the new reach. Note that the party gate (60) changes which
     side's ships receive brain orders on the US rows, so take the reach from an ON-gate build.
3. **Section 60's labelled links**, if the flip is ever questioned:
   - the Party ordinals come from `luamw_init.lua`;
   - `OwnerPlayer "AI control"` (14 authored entries, -1) is not modelled;
   - how the self-pairing plays out (60.6).

**Tools** (all in `J:\PROG\battlestations-pacific-decompile-cc9-ships17\local\`, `s17_` prefix):
- `s17_rel32.py <hex>...`: every E8/E9 rel32 caller and abs32 occurrence from the PE on disk.
- `s17_slotwrites.py`: a capstone linear sweep of `.text` for stores by disp32, and via a
  `[..+18CCh]` load. Adapt the target set.
- `s17_peek.py <addr> [n]`: dwords, with string pointers resolved.
- `s17_scnparty.py <scene paths>`: the Player1..8 Party and OwnerPlayer counts of a scene.
- `s17_runs.ps1 -V <export> [-Only rows]` and `s17_wait.ps1`: the six-row launcher and the
  foreground wait.
- Pair logs: `s17_{off,on}_<row>.log`. The last set is the 60.7 pair.

## 63. The group observer's event 6 is a party change (packet `cc9_group_party_change_removal`, cc9-ships18, 2026-09-29)

**Verdict.** Event 6 is "this entity's party was set", and its value is the new party. A group member
whose party is set to anything other than the group's own party leaves the group. Scripted orders
do not notify event 6. **Section 59's SELLING premise stands**: JM05's squadrons are not taken out of
their air group by `PilotSetTarget` or any other order. Nothing was bound: the host has no unit
SetParty for the removal to hang off, and no row reaches one (63.4).

### 63.1 Correction to 60.8 and 61.1: the compared field is `group+5638h`, not `+5628h`

`00A2DB50` is entered with ECX = the observer sub-object at `group+10h` (`00A2D8FD LEA EDX,[ESI+10h]`
is what `00A2D906` registers), so its `CMP EAX,[ECX+5628h]` (`00A2DB5B`) reads `group+5638h`. The
`ADD ECX,-10h` at `00A2DB6A` that recovers the group for `00A2D9D0` confirms the base.
- A displacement sweep of `.text` (`local\s18_disp.py 5638 5628`) finds one writer of `+5638h`:
  `00A2E045 MOV [ESI+5638h],ECX` in `BSP_AiGroup_Construct` (`00A2DFA0`), with ECX = the founding
  entity's `+54h` (`00A2E042`). **`group+5638h` is the group's party**, fixed at construction.
  Readers: 24 sites in `009FDEF0`-`00A3134E`, among them `00A2C3D4`/`00A2C9F6` (`CMP ..,2`).
- The planners' `+5628h` writes (-1 at `00A1CBA6`, `00A2A14E`, `00A2A2A5`, `00A2E015`; 8 at
  `00A2A786`) are a different field and have nothing to do with the removal.

### 63.2 The notifier census

- **The slot-0C dispatcher is `00696120`** (`RET 8`, ECX = the notifying entity, stack = event,
  value). Its loop (`0069625B`-`0069626E`) loads each edge's observer (`+8`), and calls
  `vtable[0Ch](entity, event, value)`. It is the slot-0C sibling of `00695F90`'s slot-04/08 pair.
- **Its only caller is `00696350`** (`00696350..0069635E`, `RET 4`: ECX = entity, EDX = event,
  one stack argument = value; `00696356`). A rel32/abs32 scan of the PE (`s18_rel32.py`) finds
  exactly one reference to `00696120` and 13 CALLs of `00696350`:
  - event 4: `006BF175`, `006C0E0E`, `006C487A`, `006C5842`, `006C5AF3`, `006C7C74`; `006C0CBB` sets EDX outside the seven
    instructions read (not attributed);
  - event 0: `006E14D2`, `006E659F`; event 1: `007F1D37` (`EBX = 1` at `007F1CFE`);
  - event 5: `00927E31`, `00927EF5` (`LEA EDX,[EBP+5]` with EBP = 0 after the loop);
  - **event 6: `00923BD0` only**, value = EBX = the first argument.
- **`00923B80`** (`00923B80..00923BDB`, `RET 0Ch`, ECX = entity; args party, race, out) is the
  base SetParty behind vtable `+2Ch` (31 `.rdata` dwords, vtable entries, name it; its one rel32 caller is `00928F50`'s
  `00928F7D`). It stores party at `+54h` and race at `+58h` (`00923B92`/`00923B95`), walks the
  `+48h`/`+44h` child list re-calling each child's `+2Ch` unless its `+5Ch(entity)` answers
  (`00923B9F`-`00923BC5`), then notifies event 6 with the new party (`00923BC8`-`00923BD0`).
- So on a party change, `00A2DB50` calls `00A2D9D0 BSP_AiGroup_RemoveEntity(member, 1)` unless
  the new party equals the group's.

Uncertainty: a dispatcher that walks the observer edges itself, without `00696120`, would not be in
this census. None was seen, but no sweep for `CALL [reg+0Ch]` with three pushes was run.

### 63.3 Who sets a unit's party at run time

A sweep for vtable `+2Ch` calls with three pushes (`local\s18_vslot.py 2C 3`, validated by finding
the known Lua binding site `008A8AE7`) gives 24 sites. The party-set shape (party, the entity's
`+58h` race or `+54h` party, a zeroed out pointer) is at:
- `008A88DA`, `008A8AE7`, `008A8D2B`: the Lua bindings in `008A8720`, `008A8930` (SetParty) and
  `008A8B40`;
- `0095ADBE`, `0095ADF0`: `BSP_Unit_HandleMessage`'s party and race arms (`MT_VEHICLE_SET_PARTY`,
  a session message);
- `00744A63` (`00744A20`, called from the airfield hangar reader `006D5220` and `00849F70`);
- `006F50CA`, `006F5136` in `006F4D10` (no static callers; it also writes `unit+180h`, see
  docs/AI_BRAIN_PLAYER_EXEMPTION.md);
- `007F012F`/`007F0192` in `BSP_PilotControl_HandleMessage`, and `00923BBE` (the child walk).
The other sites (`006D20B6`, `00784C20`, `00A496EC`, `00A4D21F` and the library ones) are other
classes' `+2Ch`. Not proven: which of `006F4D10`, `00744A20` and the pilot message run in single
player.

### 63.4 The host, and reach on the six rows

- The host's SetParty binding (`src/game_hosts_script_orders.cpp`, `entity_vcall_2c`) handles only
  the mission script entity (`set_script_entity_party_00928f50`); on a unit it records
  `LuaBindingCore::entity_set_party_vtable_2c` as unimplemented and changes nothing.
- The 60.7 pair logs (`cc9-ships17\local\s17_on_*.log`) show one `SetParty` binding call per row,
  at `luaStageInit` (the mission's `this.Party = SetParty(this, ...)`), and no
  `entity_set_party_vtable_2c` record on any row.
- The row scripts in this installation: JM05 (`COTP-IJN\PRCPIJN\JM05.lua`) has no `SetParty`.
  USN01, USN13 and IJN01 set only `this` (the others are commented out). LOMP10 sets only `this`.
  USN04 (`usn_19_coralus.lua`, mtime 2024-08-26) calls `SetParty(Mission.Lex, PARTY_NEUTRAL)` in
  `luaAddFinalObj` (line 879), after the Lexington's scuttle sequence; the 4500-frame row does not
  get there.
- **Prediction for a binding: identical on all six rows.** A pair would be vacuous, so none was
  run and no switch was added.

### 63.5 What would bind it

The removal belongs in a unit SetParty (the `00923B80` store and child walk, then the event-6
notification into the AI group's membership), not in the AI planner. That is a unit-host packet
(`src/game_hosts_units.cpp`), and its first row with reach would be a USN04 run long enough to reach
`luaAddFinalObj`, or a mission that changes a grouped unit's side mid-run. Ghidra: `00A2DB50` has no
function; its definition is `00A2DB50..00A2DB75` (`RET 0Ch` at `00A2DB72`, INT3 from `00A2DB75`).

## 64. The retarget ring re-checked with the party gate ON (packet `cc9_approach_retarget_ring_recheck`, cc9-ships18, 2026-09-29)

Ranking #4 (docs/GAMEPLAY_GAP_RANKING.md). Section 27 kept `kShipAiApproachRetargetRingBound` OFF
because the class group's zone lookup never found a zone on USN01, LOMP10 or USN02. Since then the
hold is its own switch (`kShipAiApproachNoShipHoldBound`, ON, section 35), and the party gate
(section 60, ON) changes which side's ships are planned. This packet takes the reach from gate-ON
logs and re-pairs the ring. No code changes: the binding and its counters are section 27's.

### 64.1 Reach on a gate-ON build

From the 60.7 pair's ON logs (`cc9-ships17\local\s17_on_<row>.log`, gate ON, ring OFF, hold ON).
`off_zone_frames` counts arm-reachable frames whose goal lies in a zone of the unit's class group,
which is exactly where the ring can move the point.

| row | latch frames | targets | retarget reachable | entries | `off_zone_frames` |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 0 | - | 0 | 0 | 0 |
| USN01 3200/3000 | 0 | - | 0 | 0 | 0 |
| USN13 3200/3000 | 1178 | 1178 other | 1178 | 134 | 0 |
| JM05 3200/3000 | 14 | 14 ship | 0 | 0 | 0 |
| IJN01 3200/3000 | 6214 | 11 ship, 6203 other | 6203 | 723 | **4126** |
| LOMP10 3200/3000 | 1383 | 782 ship, 601 building | 601 | 68 | 0 |

**IJN01 is the first row where the ring's zone lookup succeeds.** The goals are the A7M fighters
(section 35) over Oahu. USN01 no longer reaches the arm at all with the gate ON.

### 64.2 Predictions, written before any ON run

OFF is `pair_export --commit <base>` and ON the same with `--flip kShipAiApproachRetargetRingBound=true`,
both from this branch. Rows: the six above in the reference launch form, plus JM05 9200/9000
(GAMEPLAY_GAP_RANKING's 3845 reachable frames).
1. **USN04, USN01, JM05 3000 are identical ON** (exit 0 or 1): no frame reaches the arm.
2. **USN13 and LOMP10 are gameplay identical ON** (exit 1). The ON path runs the head and stores the
   goal (`009F23B5`) exactly as the hold path does, and the zone lookup then fails (`zone_runs` = 0,
   `moved_runs` = 0), so the point is the same.
3. **IJN01 moves ON (exit 3).** `zone_runs` > 0, about two thirds of the arm runs (4126 of 6203
   frames). `moved_runs` > 0: the attacking US ships steer for a point 10 m off the coast on the
   bearing nearest them instead of the A7M's position. Downstream, the per-unit approach points,
   the ships' paths and their AA engagement of the A7Ms move. A death-table move is possible; its
   direction is not predicted.
4. **JM05 9000:** ON moves iff the OFF log's `off_zone_frames` > 0; otherwise it is gameplay
   identical like prediction 2.
5. **Mechanism failure** keeps the switch OFF: IJN01 with `zone_runs` = 0, or `moved_runs` = 0
   (every slot a Landscape hit or out of reach), or a move on a row whose `zone_runs` is 0.

### 64.3 The pairs

OFF is `pair_export --commit af96f70c7 --out local\s18_off` (exe `F6629C16A3CA`), ON the same with
`--flip kShipAiApproachRetargetRingBound=true --out local\s18_on`. Launch as the reference rows
(`local\s18_runs.ps1`, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`). A 300-frame USN01 smoke on ON
ran first. Its first attempt stopped at startup on `sound/gui/error.fsb` (`create_result=78`); the
immediate retry ran clean, and no pair run hit it. Logs `local\s18_{off,on}_<row>.log`.

| row | runs ON | zone runs | moved runs | `pair_diff` | prediction |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 0 | 0 | 0 | 1, gameplay identical | 1 held |
| USN01 3200/3000 | 0 | 0 | 0 | 1, gameplay identical | 1 held |
| JM05 3200/3000 | 0 | 0 | 0 | 1, gameplay identical | 1 held |
| USN13 3200/3000 | 134 | 0 | 0 | 1, gameplay identical | 2 held |
| LOMP10 3200/3000 | 68 | 0 | 0 | 1, gameplay identical | 2 held |
| IJN01 3200/3000 | 723 | 496 | 94 | 3, moved | 3 held |
| JM05 9200/9000 | 0 | 0 | 0 | 1, gameplay identical | 4 held (`off_zone_frames` 0) |

- **IJN01.** 496 of the 723 arm runs find the goal in a zone (69%; the OFF count was 4126 of 6203
  frames). They make 29760 Landscape queries, of which 18633 hit, and 7719 slots fail the reach
  test. 94 runs move the point.
  - The first sixteen zone runs are units 281 to 283 with `reach_0490 = 10.00`. That is the
    `00956C20` floor for a unit with no Function 2/3/4/6 guns, so `R` = 6 m and no slot qualifies.
    The moved runs come from armed units.
  - Per entity: the same six deaths in the same order, with identical plane death modes. Zeilin took
    680 -> 1365 damage (hits 12 -> 24), Phoenix dealt 680 -> 1366, LST6 hits 4 -> 6 (taken 0 -> 2), and
    Mona's nearest moved 2909 -> 2872. Total damage 3324.0 -> 4010.2; shots 3066 on both sides. The
    player's Downes moved 259.54 -> 216.74 m.
- **The JM05 9000 reach is gone.** GAMEPLAY_GAP_RANKING's 3845 reachable frames predate the party
  gate. With the gate ON, the arm is never reached (0 runs on both sides).
- **Unexplained, gameplay-identical lines.** The end-of-run ship AI table's `clear_37c` column
  changed on one unit where the arm cannot act: USN13's Maru6 went FLT_MAX -> 9999.0, and JM05 9000's
  Fubuki-class 05 went 5258.7 -> FLT_MAX (JM05 has 0 arm runs). Also moved: the `free: empty`
  counter on JM05 and USN01, the known refill counter, and LOMP10's presentation lines. The `free: empty` pair is the ship avoidance noise recorded in section 40. The clearance
  column matches the known USN13 clearance-counter noise, but on JM05 it is unexplained. It is not
  the ring, which never runs on that row.

**Verdict: ON.** Every prediction held. The mechanism is exercised on IJN01 (zone runs, Landscape
casts, reach tests and moved points), and the move appears only on the row with zone runs. The
switch is flipped in `include/bsp/ship_ai_approach_update.hpp`. Modes 3 and 4 (`009F21A0-009F2395`)
stay unread and labelled. The expected movement for reference R is IJN01's damage split above,
with no death flip.

## 65. Yorktown astern: the obstacle back-off latch is never counted down (packet `cc9_ship_ai_backoff_countdown`, cc9-ships18, 2026-09-29)

The lead's question: cc9-lua20 saw USS Yorktown in JM05 moving astern at 10.4 m/s while it recovered
aircraft, with velocity (-7.6, -7.2) against a heading of 46.7 degrees. Lexington steamed ahead. Is
the astern motion the image's recovery behaviour, or a host substitution?

**Answer: a host defect in the ship AI drive, not carrier recovery.** The host never runs the head of
`009F3F80` (`BSP_ShipAi_DriveOrderRing`), which counts down the obstacle astern latch `blk+380h`.
The latch is armed at 1.0 s by `009F47A7`. Once it has been armed, it stays positive forever. The ship
is then held on the astern-only throttle window, and the escape section that would re-commit it ahead
never runs again.

### 65.1 What Yorktown does in the host (JM05 9200/9000, `local\s18_on_jm05l.log`, 60.7 + section 64 build)

- From step 60 (3.0 s) Yorktown is on `movetopos`, a scripted or planner order. Its nav goal is
  (-431.7, -920.9), 13.1 km away on heading 0.8172 rad (46.8 degrees). No carrier-recovery code
  touches its throttle or heading.
- From step 180 its sampled desired throttle is -0.625. That is `009EC7C0`'s astern floor
  (`kShipAiAsternThrottleFloor`, `00D21A78`), taken because `blk+364h` is clear: the ship is committed
  astern. The latched direction stays `ahead`.
- The throttle stays at -0.625 to step 9000. Its distance to the goal (`d32c`) grows from 8585 to
  12615 m. The hull has turned onto the goal bearing (rudder 0), so it backs away from its goal
  stern first: velocity (-7.6, -7.2) is heading 46.7 degrees reversed. **That is lua20's
  observation.**
- The recovered squadrons land on a carrier moving backwards. Recovery is a consumer of the motion,
  not its cause.

### 65.2 The image

`009F3F80` (`void __thiscall(blk)(float dt)`, `RET 4`, body `009F3F80-009F4D06`) begins, before its
only early out at `009F3FEB`, with this head (disk bytes, `local\s18_before.py 009F3FC5`):

```
009F3F89 MOVSS XMM0,[ESI+380h] / MOVSS [ESP+24h],XMM0 / COMISS XMM0,XMM1(0) / JB 009F3FEB
009F3FA4 COMISS XMM0(3.0f 00CE3854),[ESI+354h] / JBE / MOVSS [ESI+354h],XMM0
009F3FB5 FLD [ESP+24h] / FSUB [ESP+2Ch](dt) / FSTP [ESP+24h] / FLD [ESP+24h] / FST [ESI+380h]
009F3FCB FLDZ / FCOMI ST1 / FSTP ST0 / JBE 009F3FEB
009F3FD3 MOVSS [ESI+380h],-1.0f (00D7A260) / MOVSS [ESI+384h],0
```

So while `blk+380h >= 0`, it holds `blk+354h` at no less than 3.0, counts the latch down by `dt`, and
when it goes negative resets it to -1.0 and clears the stall accumulator `blk+384h`. A displacement sweep
(`local\s18_disp.py 380 384`, 009D0000-009FFFFF) finds `+380h` accessed only by this head,
`009F47A7`/`009F47A9` (the 1.0 on a back-off) and the read at `009F488D`. Its other hits are
misdecodes that overlap those instructions.

**The host.** `drive_order_ring_009f3f80` in `src/game_hosts_ship_ai.cpp` starts at the `009F3FEB`
early out. The obstacle middle (`src/ship_ai_obstacle_tables.cpp`) arms `backoff_timer_380` and reads
it at `009F47FC` and `009F488D`, but nothing counts it down. docs/GAME_EXECUTABLE.md's span table for
`009F3F80` starts at `009F3FEB`, and docs/SHIP_AI_OBSTACLE_TABLES.md says the head belongs to
another packet. The countdown fell into the gap between them.

**Census of stuck ships** (`local\s18_astern.py`, the section 64 ON logs; negative sampled throttle
up to the last step):
- JM05 3000: Yorktown, Minneapolis, Chicago, Portland, Farragut, Aylwin, Haguro and Zuikaku at -0.625
  or close to it; several IJN destroyers carry smaller negatives to the end.
- USN13: Maru4, Maru6, Maru18 and Maru24 at -0.625, and Maru14, Maru15, Maru39 and Maru48 negative.
- IJN01: PT1 and LST3 at -0.625.
- USN04: Northampton-class01 (from step 4410).
- USN01: none at the end.

### 65.3 The binding

`bsp::ship_ai_backoff_countdown_009f3f89` (`src/ship_ai_obstacle_tables.cpp`), called at the top of
`drive_order_ring_009f3f80` behind `kShipAiObstacleBackoffCountdownBound`
(`include/bsp/ship_ai_obstacle_tables.hpp`). It is committed OFF. `blk+354h` is
`ShipAiControlBlock::clamp_354`, the field `009DE763` raises the same way. Both sides count
`held_steps` (entries with the latch at or above zero) and the units that had any. ON also counts
`expiries`. The summary line is `summary mission ship ai backoff countdown`, with one
`ship ai backoff held` row per unit.

### 65.4 Predictions, written before any ON run

Rows: section 64's seven, same launch form.
1. **OFF is gameplay identical to its own base** (only the new summary lines and a record row).
2. **ON moves every row whose OFF `units` > 0 (exit 3), and `expiries` > 0 there.**
   - ON `held_steps` falls to a small fraction of OFF: each arming lasts about 1 s (20 steps at
     0.05) unless the same blocker re-arms it.
3. **JM05 (3000 and 9000):**
   - Yorktown's sampled throttle leaves -0.625 within a few seconds of step 180, and it makes
     headway toward (-431.7, -920.9): `d32c` falls instead of growing.
   - Minneapolis, Chicago, Portland, Farragut, Haguro and Zuikaku no longer end the run astern.
   - The recovered squadrons land on a Yorktown moving ahead. Landing and death rows may move, in a
     direction not predicted.
4. **USN13:** the Maru transports listed above stop backing astern and follow or move to their
   points. Positions, and so the bombing and gunnery exchanges, move; the death table can move.
5. **IJN01:** PT1 and LST3 stop backing astern.
6. **Mechanism failure** keeps the switch OFF:
   - `expiries` = 0 ON;
   - or Yorktown still ends the JM05 row at -0.625 with `d32c` growing. Then something other than
     this latch holds it astern, and 65.1 is wrong.

### 65.5 The countdown pair: a mechanism failure, and the real cause

**The countdown pair** (`094c5e31c`; OFF `local\s18_b0`, ON `--flip kShipAiObstacleBackoffCountdownBound=true`
`local\s18_b1` exe `67ACFB92B536`; logs `local\s18_{b0,b1}_<row>.log`): all seven rows are gameplay
identical (exit 1), with `held_steps=0 units=0 expiries=0` on both sides. The astern latch `blk+380h`
is never armed on these rows. Prediction 2 failed on the mechanism, so
`kShipAiObstacleBackoffCountdownBound` **stays OFF, recorded**. The head it binds is still the
image's (65.2); it just has no reach here. 65.1's cause was wrong.

The OFF base also differs from section 64's ON logs. The branch took main's gunnery15 and lua20 merges
(`28840d691`..`c17a250ae`) between the two builds, which is why `s18_on_*` against `s18_b0_*` moves
JM05. It is not an OFF-versus-base comparison.

**Trace.** `BSP_SHIP_ESCAPE_TRACE=USS Yorktown` (an env-gated diagnostic in `drive_order_ring_009f3f80`)
on JM05 250 frames, in-tree build (`local\s18_tr_jm05.log`):

| middle run | dir | `+364h` | `+36Ch` | `+378h` | speed | herr | thr | note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | 1 | 0 -> 1 | 0 | 2 | 10.29 | 0 | 0 | in formation, committed ahead |
| 20..160 | 1 | 1 | **1** | 0 | 9.3 -> 1.35 | -0.77 | **0** | left formation at 3.05 s |
| 170 | 1 | 1 -> 0 | 1 | 1 | 0.35 | -0.73 | 0 | the escape flip commits astern |
| 180.. | 1 | 0 | 1 | 1 | -0.65 -> -10.44 | -0.73 -> -0.30 | -0.625 | astern for good |

- The throttle ceiling (`009EC7C0`) answers 0.81 on those steps: settings loaded, danger 0, and the
  profile bypassed (`bypass_41 = 1`). The 0 comes from the escape section: `009F4A59..009F4A96`,
  with `blk+36Ch` set, committed ahead and no request, runs the flip whenever `+378h != 2`.
  `009F4A10` resets `+378h` to 0 every step with no request, and `009F4AD2` zeroes a positive
  throttle.
- Once the hull is stopped, `009F4B0F` commits astern. From then on the ship is not committed with
  its latch, and with `+36Ch` set there is no request 2 (`009F498C JNE 009F4A59`). The gate
  (`009F4A62`..`009F4A70`) then runs the flip only on a request, so nothing commits it ahead again.
  All of the host's projection matches the listing (`009F4977-009F4A9A`, read from disk bytes).

**The missing store.** `blk+36Ch` is set only by `009F4DA0`'s station arm (as brain+374h: 1 at
`009F5003`, 0 at `009F4FC8` and `009F4FEA`). The station arm sets it when the station throttle has
to go against the latched direction. `009ED6B0` clears it on every step: `009ED788 MOV byte
[ESI+36Ch],0` (ESI = blk, `009ED6C0`), in the unconditional reset span `009ED759..009ED795` together
with `+2FDh`, `+2FEh`, `+330h`, `+304h` and `+338h`. The host's `navigate_009ed6b0` models `+2FDh`,
`+2FEh`, `+330h` and `+340h` from that span, but not `+36Ch`. So in the host, a ship whose last station-arm
step left the byte set keeps it after leaving the formation.
- Yorktown was a follower of Lexington (`formation column`, group 2) until the `formation leave`
  at 3.05 s. That is when it moved to `movetopos` with the byte still set.

**Binding.** `kShipAiEscapeByteResetBound` (`include/bsp/ship_ai_obstacle_tables.hpp`), committed
OFF: the clear in `navigate_009ed6b0`. Both sides count `set_on_entry` (steps whose `009ED6B0` entry
found the byte set) per unit (`summary mission ship ai escape byte reset`). The `+304h` and `+338h`
clears of the same span were not checked against the host and are not part of this binding.

### 65.6 Predictions for `kShipAiEscapeByteResetBound`, written before any ON run

Rows: section 64's seven.
1. **OFF is gameplay identical to the countdown pair's OFF** (`s18_b0`): only the new summary lines
   and a record row.
2. **JM05 3000 and 9000 move (exit 3).**
   - Yorktown keeps a positive throttle after the formation leave. It is never committed astern for
     good, and its `d32c` falls toward (-431.7, -920.9).
   - Its landings happen on a carrier moving ahead.
   - The death table may move; its direction is not predicted.
3. **Every row where OFF shows a unit with `set_on_entry` > 0 that is not in a station arm moves.**
   USN13's `movetopos` Marus (4, 6, 18 and 24) no longer end astern; IJN01's PT1 and LST3 likewise if
   the byte is their cause.
4. **Followers still inside a station arm keep whatever the arm asks for.** The arm sets the byte
   again every step after the clear. So an astern station command, as the USN13 `follow` Marus may
   have, can still show negative throttles: that is the image's station keeping, not this defect.
5. **Mechanism failure** keeps it OFF: Yorktown still at -0.625 to the end of JM05 with the switch ON,
   or no row moving.

### 65.7 The escape-byte pairs and the verdict: ON

OFF is `pair_export --commit f9e933541 --out local\s18_b2`; ON is the same with
`--flip kShipAiEscapeByteResetBound=true` (`local\s18_b3`). A 300-frame USN01 smoke ran on ON first.
Logs are `local\s18_{b2,b3}_<row>.log`.

| row | OFF `set_on_entry` (units) | OFF vs countdown OFF | `pair_diff` ON | death rows | prediction |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | 4985 (11) | 1 | 1, gameplay identical | identical (50) | 1 held; 3 n/a (station arms) |
| USN01 3200/3000 | 967 (2) | 1 | 1, gameplay identical | identical (5) | 1 held |
| USN13 3200/3000 | 16480 (7) | 1 | 3, moved | identical (22) | 3 **partly missed** |
| JM05 3200/3000 | 13181 (22) | 1 | 3, moved | identical (1) | 2 held |
| IJN01 3200/3000 | 4598 (25) | 1 | 1, gameplay identical | identical (6) | 3 missed for PT1/LST3 |
| LOMP10 3200/3000 | 5 (1) | 1 | 1, gameplay identical | identical (10) | 1 held |
| JM05 9200/9000 | 27656 (24) | 1 | 3, moved | identical (5) | 2 held |

**JM05 9000, the question.**
- **Yorktown.** OFF, it is astern from step 180 to 9000 at -0.625, and its `d32c` grows
  8585 -> 13111 m. ON, it has a positive throttle from step 250 and is at 1.000 from step 1050. Its
  `d32c` falls 8531 -> 7437 (step 3050) -> 2477 m at 9000: it steams ahead to its `movetopos` goal.
  The unit table's `nearest` for Yorktown goes 15813 -> 5791.
- **Other ships.** Minneapolis, Chicago, Portland, Farragut and Aylwin, all followers, stop ending
  astern. Each ends ahead at 0.87 to 1.00, and their `nearest` falls from about 15 km to about 6 km:
  they stay with the fleet.
- **Unchanged.** Haguro and Zuikaku keep their negative throttles on both sides (IJN station arms).
- **Totals.** Deaths 5/5 with identical rows. Hits, damage and shots are identical. Plane water
  contacts 4 -> 3.
- **Newly reached, both lua20's lane:** `BotStateLandAbort::tick` `009B09C0` (116 calls) with its
  `direction_hold_007c07a0` record, `PlayerGunSeat::artillery_hit_lead` `009578C3` (56), and a
  Yorktown squadron's land task at 371 s. The carrier now moves ahead during recovery.

**JM05 3000.** The same shape. Yorktown's `nearest` goes 15813 -> 12511, and the death row is
identical.

**The USN13 miss.** Maru24 changes (its final throttle goes -0.625 -> -0.500), but Maru4 and Maru6
still end astern. A trace on the ON build (`BSP_SHIP_ESCAPE_TRACE=Maru4`, `local\s18_tr_usn13.log`)
shows the cause is not this byte:
- the escape byte stays at 0, and the committed direction stays astern;
- the latched direction `blk+35Ch` is 2 (Astern) from middle run 80, with `blk+1C8h` = 1;
- the throttle comes from the ceiling in the astern direction.
So those Marus are sent astern by the navigation arm's own direction latch. That is a separate item,
opened below, not a failure of this mechanism.

IJN01's PT1 and LST3 are not moved either. Their byte is set by their station arms every step, which
prediction 4 allowed.

**Verdict: ON.** The mechanism is confirmed where predicted: Yorktown and the JM05 US followers.
Prediction 3 over-reached on USN13 and IJN01. This is a spread miss with the mechanism matching,
recorded. `kShipAiObstacleBackoffCountdownBound` stays OFF (65.5).

**Expected movement for reference R:** JM05 (the US carrier group's positions and the landing
geometry), and USN13's Maru24. No death flip on these rows.

**Open items.**
1. USN13 Maru4 and Maru6 on `movetopos` latch `blk+35Ch` = Astern. Check that `009ED6B0`'s direction
   latch (`009ED7D8..009ED8BD`, the `blk+1D0h` sign and the `00D7A270` dead band) and the goal's
   side agree with the image.
2. The countdown binding (65.2) waits for a row where `009F47A7` arms.
3. The `+304h` and `+338h` clears of `009ED6B0`'s reset span (`009ED78F`, `009ED795`) are unchecked
   against the host.

## 66. The party gate's unpredicted rows: BSM01 and USNOS (cc9-ships18, 2026-09-29)

Reference r (docs/GAME_EXECUTABLE.md, "2026-09-30 r") attributes the moves on USN02, JM06, BSM01,
LOMP06, USNOS and USNOS long to `kAiPartyGateUnforcedBound` alone. SHIP_AI 60.7 had not predicted any
of them. This section checks the mechanism on two of those rows, using cc9-gunnery15's logs: r is
`g15_rr_<row>.log`, and r with the gate OFF is `g15_r_apg_<row>.log`, both in that tree's `local\`.
It is a log reading; nothing was re-run.

**BSM01: `HenryPT` stands still because it loses its brain's formation order, not a script order.**
- **Gate OFF.** The AI planners ask for 1174 formation follows, `00779D50` admits 25, and `0077F940`
  makes all 25 joins (5 groups). One of them is `follow issued (00720CD0, source join 0077F940):
  "HenryPT" -> "Medusa"` (group 3). HenryPT then station-keeps on Medusa: 601 station requests and
  1051 arm runs. That movement is its 603.30 m.
- **Gate ON.** The gate line reads `local_team=0 slot4_team=0`. It plans only slot 4, and in BSM01
  slot 4 is also team 0, so `other_slot_groups=0`: no group is planned for anyone. The follow
  requests, joins and formation groups are all 0. HenryPT is on `stop` from step 10, with no
  formation role, and moves 0.00 m.
- **The script plays no part.** It is `bsm\bsm_01_stationed_at_pearl.lua` (this installation, mtime
  2024-07-13), and it issues no formation or movement order to `Mission.Henry`. The calls on
  HenryPT that run within 3000 frames are `SetInvincible` (49 on both sides), `ShipSetTorpedoStock`
  and `SetSelectedUnit`. Its `SetRoleAvailable` calls are not reached on either side.
- **So this is the section 60 rule working as designed.** In single player the image plans slot 4 and
  never slot 0. The player side's formation follows came from the slot-0 planning that the gate
  removed.

**USNOS: +12 deaths are the other side's squadrons now receiving the planner's attacks.**
- **What is new.** The 12 new death rows are the Japanese squadrons `plane #1.1`, `#1.2` and `#1.3`,
  4 planes each, shot down by US ships (for example `plane #1.1` by Portland1 at 110.25 s).
- **Gate ON.** The planners issue those squadrons orders through `AiPlanners::issue_member_order`
  (`0077D600`): `artillery` at 42.45 s, followed by `attackmove building arm 00836B95 ... own_side=1`.
  The gate line reads `other_slot_groups=315`, so the non-local side is planned.
- **Gate OFF.** The same squadrons only carry the script's `PilotSetTarget` (declined at `007EEC50`)
  and their target token; the planner never sends them in.
- **The rest.** Damage 2004.5 -> 8636.9 and shots 4968 -> 1206 follow from the same change: the US
  side is no longer planned, and the Japanese side is.

**Verdict.** Both rows move by the rule 60.7 bound: slot 0, the player's side, loses its brain, and
slot 4 gains one. The misses were in 60.7's row list, not in the mechanism. USN02, JM06 and LOMP06
were not re-read here. r's attribution covers them, and the same two effects are the expected
reading.

## 67. 65.7's open items: the USN13 astern Marus, and the `+304h`/`+338h` resets (packet `cc9_ship_ai_reset_span_304_338`, cc9-ships18, 2026-09-29)

### 67.1 Maru4 and Maru6 go astern by the image's choice rule

Source: `local\s18_b3_usn13.log` (the 65.7 ON pair) and the `Maru4` trace `local\s18_tr_usn13.log`.
- **Both are on `movetopos` with a goal close behind them.** Maru4 enters it at step 100 with the
  goal 651.8 m away on bearing -2.32 rad, against a heading of 0.70 rad (about 173 degrees off). Maru6
  enters at step 90 with the goal 507.8 m away on bearing 2.835 rad.
- **The latch is `009EF0A8`'s choice** (`ship_ai_astern_choice_009ef0a8`; the `+1CCh` gear order is
  0, so the choice arm runs):
  - astern when the remaining distance `+330h` is under `max(3 x +9C8h, 2 x turn radius)`, with 80 m
    of hysteresis once astern, and the heading error is over 130 degrees (120 once astern);
  - their `+9C8h` is 180.0 (the table's `len_9c8`), so the first term alone is 540 m;
  - for Maru4's first latch at 651.8 m, the threshold must exceed 651.8 m, so `2 x turn radius`
    does. The table shows `turn_3c8` 534.76 and `turn_3cc` 508.02; which of these, if either, is
    `0082E850`'s radius was not established;
  - both are latched astern from their first `movetopos` sample.
- **The ships do close their goals.** Maru4's `d330` falls 651.8 -> 101.8 m by step 2070 while
  backing. It never reaches `stop_3d4` = 72 m: it circles at 100 to 200 m with full rudder
  (`d330` 200.3 at step 2870). Maru6 ends 138.5 m out.
- **Verdict:** the astern latch is the rule, not a host substitution. **Open:** why a reversing
  transport orbits its goal instead of arriving. That is astern steering (the rudder law with the
  pi-turned heading, `009F409E`, and the arm final's reverse sense) against a stop radius smaller
  than its reversing turn circle. It was not read here, and it may be the image's own behaviour.

### 67.2 The `+304h` and `+338h` resets

`009ED78F MOV [ESI+304h],ECX` and `009ED795 MOV byte [ESI+338h],CL` (ECX = 0 from `009ED767`) sit in
`009ED6B0`'s unconditional reset span next to `009ED788`. A displacement sweep over
`009ED000-009EFFFF` (`local\s18_disp.py 304 338`):
- **Writers:** `+304h` at `009EE765`, `+338h` at `009EE783` and `009EE7E4`, all in the navigation
  arm.
- **Reader in the body:** `+304h` at `009EE90C`, also in that arm and after its writer.
- **The reader outside the arm:** `009DEBB9`, the separation turn of `009DE5B0`. The host reads
  `nav.side_304` there.
- **The station arm writes neither** (`009EDA28..009EE57B`), and its `009EE57B JMP 009EF206` still
  reaches `009DE5B0`. So in the image a station-keeping ship's separation turn sees side 0. The host
  hands it whatever turn side that ship's last navigation step left.
- **`+338h`** has no reader outside the navigation arm in the host; its reset is bound for
  completeness.

**Binding:** `kShipAiNavResetSpanBound` (`include/bsp/ship_ai_navigation.hpp`), committed OFF: the two
clears in `navigate_009ed6b0`. Both sides count `side_304_set_on_entry` and
`station_separation_sided` (station-arm steps with a live neighbour count whose side is non-zero).
The summary line is `summary mission ship ai nav reset span`.

### 67.3 Predictions, written before any ON run

Rows: section 64's seven.
1. **OFF is gameplay identical to 65.7's ON** (`s18_b3`), apart from the new summary line and record
   row.
2. **Rows with `station_separation_sided` = 0 on OFF are gameplay identical ON** (exit 0 or 1).
3. **Rows with `station_separation_sided` > 0 on OFF may move (exit 3):** the followers' separation
   turns change sides. A death flip is not predicted.
4. **Mechanism failure:** a move on a row with `station_separation_sided` = 0.

### 67.4 The pairs and the verdict: ON

OFF is `pair_export --commit 7d535d21f --out local\s18_b4`, ON adds
`--flip kShipAiNavResetSpanBound=true` (`local\s18_b5`). Smoke first; logs `local\s18_{b4,b5}_<row>.log`.

| row | OFF `side_304_set_on_entry` | OFF `station_separation_sided` | `pair_diff` ON | death rows |
| --- | --- | --- | --- | --- |
| USN04 | 0 | 0 | 1, gameplay identical | identical (50) |
| USN01 | 0 | 0 | 1 | identical (7) |
| USN13 | 23706 | 1938 | 1 | identical (25) |
| JM05 | 17740 | 0 | 1 | identical (1) |
| IJN01 | 61196 | 0 | 1 | identical (6) |
| LOMP10 | 3590 | 0 | 1 | identical (10) |
| JM05 9000 | 42176 | 0 | 1 | identical (5) |

- **Predictions 2 and 4 held.** Prediction 3 allowed a move on USN13 and none came: its 1938 sided
  steps did not change a separation outcome.
- **Prediction 1 could not be judged.** Between `s18_b3` (65.7) and `s18_b4` the branch took main's
  gunnery15 and lua20 merges (`dafed8a1a`, `d90daefdc`: the torpedo reset draws, the plane wanderer,
  follow placement), so `s18_b3` against `s18_b4` moves on every row for reasons outside this switch.
  The pair is same-tree, and that is what the verdict uses.

**Verdict: ON.** The mechanism is exact, gameplay identical on these seven rows, and nothing moves
for reference R.

## 68. The Maru reverse "orbit" is a close-attack chase, not an astern-steering defect (packet `cc9_ship_ai_reverse_orbit`, cc9-ships18, 2026-09-29)

**Question (the lead, after 67.1).** USN13's Maru4 and Maru6 back toward their goals with full rudder
and never reach `stop_3d4` (72 m). Does the host steer astern differently from the image?

**Trace.** `BSP_SHIP_ESCAPE_TRACE=Maru4` on USN13 3200/3000, in-tree build at `0e2c392bb` plus a
steering line (hull heading, the drive's heading, `+324h`, rudder, position, goal, `+330h`); the log
is `local\s18_tr2_usn13.log`.

| middle run | hull heading | drive heading (hull + pi) | `+324h` | error | rudder | ship | goal | `+330h` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 80 | 0.698 | -2.443 | -2.268 | 0.176 | 0.00 | (3400.0, -1000.0) | (3144.0, -1709.3) | 652.6 |
| 800 | 0.819 | -2.323 | -2.259 | 0.064 | 0.71 | (3200.3, -1200.4) | (3070.7, -1783.2) | 419.7 |
| 1600 | 0.990 | -2.152 | -2.148 | 0.005 | 0.05 | (2933.6, -1417.0) | (2959.8, -1874.6) | 192.2 |
| 2000 | 1.078 | -2.064 | -2.124 | -0.060 | -0.67 | (2783.9, -1501.4) | (2904.7, -1906.4) | 103.4 |
| 2400 | 0.773 | -2.369 | -2.545 | -0.177 | -1.00 | (2646.0, -1603.4) | (2848.9, -1944.5) | 160.1 |
| 2980 | 0.337 | -2.805 | -3.000 | -0.196 | -1.00 | (2513.9, -1813.0) | (2790.1, -2033.1) | 158.1 |

**Reading.**
- **Astern steering works as the image's rule describes.**
  - The drive steers the pi-turned heading (`009F409E`, host `drive_order_ring_009f3f80`), and the
    error against `+324h` stays within +-0.2 rad the whole run.
  - The hull heading follows `+324h` + pi (0.70 -> 1.08 -> 0.34 rad), and the stern points where the
    arm asks.
  - Full rudder appears only while `+324h` swings. No sign inversion or runaway turn is visible, so
    there is nothing in the rudder law or the reverse sense to bind.
- **The goal moves.** It is not a fixed point: it drifts (3144, -1709) -> (2790, -2033), about 480 m
  in 145 s. Maru4 leads AI group team 1 (party 4), with `command=CLOSEATTACK target=1 leader=Maru4`
  and `ai diag movetoattack leader=Maru4 dist=1256.9`, against the US cruiser group led by CB2. The
  goal is the close-attack point, which follows the target. This is the party gate's slot-4 planning
  (section 60) sending the Japanese convoy against CB2.
- **The 100 to 260 m is `+330h`, the remaining path to the path point, not the distance to the goal.**
  The ship itself stays 350 to 450 m from the moving goal (754 m at run 80, 353 m at run 2980).
  `stop_3d4` is tested against `+330h` plus the setback at `009EEF5C..009EEF70` (the setback is 0
  here: `max_setback=0.0`). It can only pass on the final leg, at a goal that stops moving.
- **The astern latch persists by `009EF0A8`'s hysteresis.** The goal stays within
  `max(540, 2 x turn) + 80` m and about 170 degrees off the raw hull heading, so the choice keeps
  answering astern.

**Verdict: no host defect found, nothing bound.** A cargo ship chasing a moving close-attack point
stern first is what this rule gives for these inputs. Two things were not checked:
- whether the image would give an unarmed `FleetOilerJ` group a CLOSEATTACK at all. That is the
  planner's side, AI_PLANNERS; the Maru group's order came from the slot-4 planning;
- whether the image's close-attack point for a ship this size lies behind it.

Both are planner-lane questions, for the next ships worker.

## 69. Handoff (cc9-ships18, 2026-09-29, at about 66% context)

**Landed on main:** 63 (event 6 is a party change), 64 (the retarget ring ON), 65 (the escape-byte
reset ON; the back-off countdown bound OFF), 66 (the party gate's BSM01/USNOS moves are the rule), 67
(the `+304h`/`+338h` resets ON; the astern Marus are the choice rule). **Unlanded on
`agent/cc9-ships18`:** section 68 and this section (docs), plus the `ship escape steer` line added
to the env-gated `BSP_SHIP_ESCAPE_TRACE` diagnostic in `src/game_hosts_ship_ai.cpp`.

**Switches this lane changed:**

| switch | state | section |
| --- | --- | --- |
| `kShipAiApproachRetargetRingBound` | ON | 64 |
| `kShipAiEscapeByteResetBound` | ON | 65.7 |
| `kShipAiNavResetSpanBound` | ON | 67.4 |
| `kShipAiObstacleBackoffCountdownBound` | OFF, exact, no reach on the seven rows | 65.5 |

**Queue for the next ships worker:**
1. **Section 60's labelled links** (the old queue item 3), if the party-gate flip is questioned:
   - the Party ordinals come from `luamw_init.lua`;
   - `OwnerPlayer "AI control"` (14 authored entries, -1) is not modelled;
   - how the self-pairing plays out (60.6).
   Section 66 settles BSM01 and USNOS as the rule. USN02, JM06 and LOMP06 were not re-read.
2. **The planner questions 68 left:**
   - does the image give an unarmed `FleetOilerJ` group (USN13's Maru4/5/6) a CLOSEATTACK against
     CB2's group;
   - does its close-attack point for that ship lie behind it?
   Start at the `ai diag order_attack group_leader=Maru4` / `movetoattack` lines of
   `local\s18_tr2_usn13.log` and docs/AI_PLANNERS.md.
3. **Modes 3 and 4 of the approach retarget arm** (`009F21A0-009F2395`), unread and labelled
   (`ShipAiApproach::retarget_modes_3_4`). Section 64's ring is ON.
4. **The back-off countdown** (65.2) waits for a row where `009F47A7` arms (`held_steps` > 0 in
   `summary mission ship ai backoff countdown`). Flip it by a pair on that row.
5. **Routed elsewhere, not ours:**
   - the unit `SetParty` (63.5, with the lua lane);
   - JM05 9000's newly reached `BotStateLandAbort` `009B09C0` / `007C07A0` and
     `PlayerGunSeat::artillery_hit_lead` `009578C3` (to cc9-lua21, via the lead).

**Tools** (in `J:\PROG\battlestations-pacific-decompile-cc9-ships18\local\`, `s18_` prefix):
- `s18_rel32.py <hex>...`: rel32 callers and abs32 references, from the PE on disk.
- `s18_disp.py <disp>...`: every `.text` operand with that displacement. It misdecodes some
  overlapping instructions (spurious `adc`), so read each hit with `s18_before.py`.
- `s18_before.py <addr> [n_before] [n_after]`: a back-synced listing around an address.
- `s18_vslot.py <slot> <npush>`: virtual calls through a slot with n pushes.
- `s18_kinds.py <log> <needle>`: a log's lines about one unit, grouped by kind.
- `s18_astern.py <log>...`: the units whose sampled throttle is negative, and until when.
- `s18_runs.ps1 -V <export> [-Only rows]` and `s18_wait.ps1 -Logs ...`: the seven-row launcher
  (with JM05 9000) and the foreground wait.
- **The diagnostic:** `BSP_SHIP_ESCAPE_TRACE=<unit name>` logs, every 20 drive steps, the escape
  inputs, the throttle ceiling, the profile and the steering of one unit.

## 70. Section 68's planner questions: which groups the Capture planner keeps (packet `cc9_capture_group_value`, cc9-ships19, 2026-09-30)

**Questions (68).** Does the image give USN13's unarmed Maru4 convoy group a CLOSEATTACK at all, and
where does it put the close-attack point?

**How the Maru groups get their order.** USN13's slot-4 brain runs the Capture target path
(`00A29FD0`, `kAiCaptureTargetPathBound`, docs/PLANNER_TASK_CHOICE.md section 8), because the US side
holds three CommandBuildings (CB2, CB4, CBT). The first think (t=0.05) assigns all 42 groups. Every
assignment answers `order=attack`: `00A1A720` finds the target's `+16Ch` group set, so `00A2CBD0`
issues MOVETOATTACK. The command tick `00A12A90` then promotes it to CLOSEATTACK inside
`CloseAttack_CollectDist` (3000 here). `local\s19_diag_usn13.log` shows the chain:
`assign leader=Maru4 -> CB2`, `order_attack ... Maru4 ... CB2`, and `ai command promote ... Maru4
dist=1256.9`.

**The host difference.** The per-group weight at `00A2A380` is `00A250A0(group, entity)`:
`00A0C650(00A07E40(group), 00A24870(entity), 0, -1.0 [00D7A260], 0, 0, 1.0)` (live decompile of
`00A250A0`; it has no empty-group test, unlike `00A0F970`). The host still carries the labelled
stand-in `members x defenders`, which is always positive, so every group is assigned (section 8's
substitution list). `00A0C650` is already reconstructed for `00A0F970`
(`kPlannerGroupTargetValueBound` ON). The assignment loop skips a pair unless `w > 0`
(`00A2AC02 COMISS [00D7A218] / JBE`). A group with no pair value is therefore never assigned. It is
handed to `brain+4h` (Attack) when its record's nearest own list-28 entity is null (`00A2AFC0`), and
to `brain+8h` otherwise.

**Bound OFF:** `kCaptureGroupValueBound` (`src/game_hosts_ai.cpp`). ON computes `00A250A0` from the
`00A04560` records of the group members and of `00A24870`'s entities (the defenders, or the target
alone), through the extracted `compose_attack_value_00a0c650`. `BSP_CAPTURE_DIAG=1` logs the value in
both states (`capture value t= leader= barrels= target= value= stand_in=`). The summary line is
`summary mission ai capture group value bound= calls= zero=`.

**What the value is on USN13 (OFF diag, `local\s19_diag2_usn13.log`):**
- **At t=0.05 every value is positive.** The weapon-facts rows are not published yet, so
  `00A08460` takes its stand-in weight of 1.0. Maru4's group scores 21.3 against CB2, 11.6 against
  CB4 and 15.7 against CBT; the stand-in gives 60, 42 and 33.
- **From t=0.10 the Maru groups score 0 against all three targets:** 27 groups, Maru1 through
  Maru50, with 5 to 31 barrels each (`barrels=17` for Maru4's). Inferred, not traced per barrel: their
  barrels have no accuracy entry against the defenders' class groups, so `009FE270` answers the
  reject offset 0. Only the
  eight warship groups (Agano, Fumizuki, Katori, Matsukaze, Naka, Nowaki, Oite, Yamagumo) stay
  positive.
- **Census:** 4308 calls, 3150 zero.

**Predictions (written before the ON runs):**
- **USN13 moves (exit 3).**
  - From the t=0.10 think the 27 Maru groups are not assigned. `near_own` is `-` for every group,
    so each one is handed to `brain+4h` once: capture-path assignments drop far below 1436, and the
    handoffs appear.
  - `summary mission ai group target value calls` rises from 0, because the Attack think
    (`00A1CF90` -> `00A1CB80`) now owns groups.
  - For a Maru group, the `00A1CB80` score is nonzero only against enemy groups its barrels can
    hit, which are plausibly air groups. When every candidate scores 0, the first populated enemy
    group wins, because 0 beats the -999999 floor at `00D22CC4`. `00A2CBD0` returns early while the
    command already targets that group, so a group whose first pick is CB2's keeps its t=0.05
    order.
  - The eight warship groups stay assigned to CommandBuilding targets. Their target can change
    with the real values (Agano: CB2 7.1, CBT 3.7, CB4 1.3).
- **USN04 and JM05 are gameplay-identical (exit 0 or 1).** They run no capture path
  (`capture path thinks=0` on `s18_b5`).
- **USN01 runs the capture path (38 thinks) and moves (exit 3).** OFF diag
  (`local\s19_diag_usn01.log`): one target (CB2) and five groups. From t=0.10, Convoy1 (19
  barrels), Convoy2, Convoy3 and Mav1 score 0, and Katori's group stays positive. Those four are
  handed to `brain+4h` at t=0.10; only Katori's group stays assigned.

**Answer to 68's second question (static).** CLOSEATTACK's tick hands `00A13B60` the target
group's leader point (`00A15490`, radius argument 1.5). `00A13B60` scores the candidates collected
around that point. An attacker whose weight is 0 admits only target-group members (`00A146CD` /
`00A146D9`, `ai_close_attack_candidate_admitted`). A ship member is then given an attack-move
(`E08F78`, `00A149FC`) at the chosen **entity**. So the close-attack point is an enemy unit of the
target group. It moves when that unit moves, and it lies behind the Maru only because that unit
is behind it. The image places no point relative to the attacker's own hull.

### 70.1 The pairs, and the decision

**Runs.** Pair `3350621c1`: OFF is the tree build and ON is `local\s19_p1` (flip
`kCaptureGroupValueBound=true`, ON binary `26EDE0E5132A`). Reference launch form, lockstep 0.05,
`BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | exit | predicted | what moved |
| --- | --- | --- | --- |
| USN13 | 3 | moved | `handoffs 0 -> 28`; capture-path assignments `1436 -> 356`; `00A0F970` calls `0 -> 42756` (41860 zero); death rows identical (25), 56 unit rows moved (mostly nearest-enemy distances) |
| USN01 | 3 | moved | `handoffs 0 -> 3`; assignments `134 -> 44`; deaths `7 -> 6`: Convoy1 no longer sinks (damage taken 1121 -> 549); KatTBD-3 dies at 148.15 instead of 148.65 |
| USN04 | 1 | identical | none |
| JM05 | 1 | identical | none |

**The mechanism held.**
- The zero-valued groups are handed to `brain+4h`, and the Attack think then scores them through
  `00A1CB80`.
- Every one of the 400 sampled `00A0F970` values with a Maru group as attacker is 0, so the pick is
  the first populated enemy group.
- Maru4's group keeps its t=0.05 MOVETOATTACK at CB2's group. It is promoted to CLOSEATTACK at the
  same step as OFF (`dist=1256.9`), so section 68's chase is unchanged: it is the image's rule for
  these inputs.

**Spread misses.**
- USN13 hands off 28 groups, not 27. The OFF diag was capped at 450 lines, which covered only the
  second think; one more group scores 0 later.
- USN01 hands off 3, not 4. From t=0.10 that row has four groups (`groups=4`), and Katori's is the
  only positive one. The prediction had counted Convoy2 separately, from late lines.

**Decision: `kCaptureGroupValueBound` is ON.** The mechanism is the image's, the controls are
identical, and the misses are counts.

**Answer to section 68, first question:**
- The image does give an unarmed or AA-only group an attack order. The Capture planner drops it
  (value 0 against every defender), and the Attack planner's `00A1CB80` still picks the first
  populated enemy group, because 0 beats its -999999 floor.
- The MOVETOATTACK tick then promotes that order to CLOSEATTACK inside CollectDist.
- The t=0.05 orders still come from the host's stand-in weight of 1.0: the weapon-facts rows are
  published after the first think. That is a publication-timing substitution in the units host,
  not this switch.

## 71. Section 60's labelled links: the Party ordinals and `OwnerPlayer "AI control"` (packet `cc9_ai_owner_player_slot`, cc9-ships19, 2026-09-30)

### 71.1 The Party ordinals are settled

- The scene reader resolves `E <table> : <symbol>` through the library's enum tables
  (docs/SCENE_PROPERTY_BAG.md, `0048E840`). These are not native strings: `"Players"` and
  `"AI control"` occur nowhere in the exe.
- This installation's `universe/library/global.enums` (mtime 2024-10-29) declares
  `enum Party { Allied = 0, Japanese = 1, Neutral = 2 }` at lines 1705-1710. That matches the
  `luamw_init.lua` ordinals the party gate took. The link is closed and changes no behaviour.
- The host's own library (`game_hosts_scene_contents.cpp`, `PropertyLibrary::resolve_symbol`)
  already resolves `Party` this way for unit records.
- The hard-coded `Allied/Japanese/Neutral` mapping for the slot parties in
  `game_hosts_mission.cpp` (not this lane's file) gives the same numbers. Its comment still cites
  `luamw_init.lua`; the source of truth is `global.enums`.
- The native `00E0CF24` table (`Allied`, `Axis`, `Neutral`) is a different list. It is not the
  scene enum.

### 71.2 `OwnerPlayer` is a unit property, not a `MultiType` one

Section 60.1 said the entries sit inside `MultiType` blocks. They do not. In, for example,
`ijn_05_invasion_of_port_moresby.scn` lines 1167-1169 (mtime 2024-07-13), `"MultiType" { }` is empty
and `OwnerPlayer = E Players :"AI control" ;` is its sibling in the entity's bag.

- **The enum.** `global.enums` 1725-1737 declares `enum Players`: `"Player 1"`..`"Player 8"` = 0..7,
  `"AI control"` = 8, `"Any player"` = 9. The group default is
  `OwnerPlayer = E Players:"Any player"` (line 1757).
- **The producer.** `0077F0E0` activation hands the found record's `+0Ch` to vtable `[144h]` at
  `0077F1F9`, which stores `unit+180h`. It passes 9 when the bag has no `OwnerPlayer`
  (`0077F1F1`, docs/AI_BRAIN_PLAYER_EXEMPTION.md).
- **The consumer, `009FFD20`** (live decompile, `009FFD20-009FFD5C`):
  - `+180h == 8` returns -1;
  - `+180h < 8` returns `+180h`;
  - otherwise the single-player team rule applies: 0 on the local team, else 4.
- **The AI sites** are `00A2DFA0` (the group's `+5634h`), `00A2DDE0` (evict a member whose slot
  differs) and `00A16EF0`.
- **The consequence.** A group led by an "AI control" unit is NONCONTROL (`00A2E124`, slot -1). Any
  other group evicts such a unit on its next evict pass.

**The reference rows' entries.** `local\s19_owners.py` walks every scene. The OFF census line is
`ai owner player`, run from `local\s19_c_<row>.log`, 300 frames.

| row | units with `+180h = 8` | side | slot, OFF (+180h taken as 9) | slot, ON |
| --- | --- | --- | --- | --- |
| JM05 | SecondaryAirfieldEntity 01, MainAirfieldEntity 01, MainShipyardEntity 01, MainShipyardEntity 02 | 0 | 4 (planned) | -1 |
| LOMP10 | CB4_AF, CargoShip | 0 | 0 | -1 |
| LOMP10 | CB4_AF_Hangar | 2 | 4 (planned) | -1 |
| USN12 | Fortress-07..10 | 0 | 0 | -1 |
| USNOS | Airfield3, Multi Hangar 1 | 1 | 4 (planned) | -1 |
| JM08 | MainAirFieldEntity 01 | 1 | 0 | -1 |

That is 14 entries on these rows, as section 60 counted. Across all scenes, `s19_owners.py` finds
2392 `OwnerPlayer` lines, and the non-reference scenes author "AI control" on many ships (BSM, CHG).
No reference row authors `"Player N"`.

**Bound OFF: `kAiOwnerPlayerSlotBound`** (`src/game_hosts_ai.cpp`, which needs the party gate).
- **The scene side.** The scene contents host resolves each record's `OwnerPlayer` through its
  library and publishes the values other than 9 by entity name. The call is
  `ai_publish_scene_owner_players`, and the log line is `scene owner players: ...`.
- **The ON path.** `unit_slot_009ffd20` replaces the team-only rule at the three host sites:
  `create_group`, `evict_invalid_members` and the party-record census.
- **LABELLED.** The key is the entity name, because this process has no entity object; a name
  authored with two values reads as 9 (`conflicts=` in the census line). Generated squadrons and
  wing members take 9. The session and message writers of `+180h` (docs/AI_BRAIN_PLAYER_EXEMPTION.md)
  are not modelled; none is reached in single player.

**Predictions (written before the ON runs):**
- **JM05 and USNOS** have entries whose OFF slot is 4, the planned slot.
  - ON, those units leave slot 4. `other_slot_groups` in `summary mission ai party gate` drops,
    because the airfields and shipyards lead groups of their own or are evicted from mixed ones.
  - The deaths are predicted unchanged. These are static installations, and the close-attack
    member gate (`00A143ED`/`00A14427`) serves no airfield (`served=0` on the OFF log). Any
    gameplay move comes only through a group that one of these units led and that held ships.
  - Exit 1 is the prediction; exit 3 is possible on that one path.
- **LOMP10** also has CB4_AF_Hangar at slot 4 -> -1: the same prediction as JM05.
- **USN12 and JM08**: slot 0 -> -1, and both are NONCONTROL either way. The census moves
  (`local_slot_groups` can drop), and the rows are gameplay-identical (exit 1).
- **Controls USN04 and USN13** author no `OwnerPlayer` 8. They are identical apart from the new
  census lines (exit 0 or 1).

### 71.3 The pairs, and the decision

**Runs.** Pair `9de336d8b`: OFF is the tree build and ON is `local\s19_p2`. Reference launch form,
3200/3000, lockstep 0.05, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | exit | predicted | what moved |
| --- | --- | --- | --- |
| JM05 | 3 | 1 (3 possible) | death rows identical (1); the four airfields and shipyards leave slot 4; `prox_merges 200 -> 195`, `members_added 539 -> 436`, `tick_orders 1189 -> 1160`; USS Phelps moved 2649 -> 2625 m; the other units change only in nearest-enemy distance |
| USN12 | 3 | **1, missed** | death rows identical (4); `local_slot_groups 16 -> 12`, `prox_merges 12 -> 9`; Fortress-10 deals 452 -> 462; Shigure and Samidare damage taken moves |
| LOMP10 | 1 | 1 | none |
| USNOS | 1 | 1 | none |
| JM08 | 1 | 1 | none |
| USN04 | 1 | 1 | none (control) |
| USN13 | 1 | 1 | none (control) |

**The USN12 miss is a missed consequence of the same rule, not a different mechanism.**
- `00A2DFA0` stores the slot at `00A2E03C` and inserts the group into the per-party list
  `00F8A9E8 + slot*0Ch` only when the slot is not below 0 (`00A2E03A CMP EAX,EDI / 00A2E04B JL
  00A2E086`, disk bytes). The host has the same guard in `create_group`.
- A slot -1 group is therefore in its team list, where it is a target candidate, and in no party
  list. It never thinks, so it never proximity-merges.
- On USN12 the four fortress groups stop merging into the slot-0 groups (`prox_merges 12 -> 9`).
  The Japanese brain then sees four more candidate groups, which moves its orders.
- The prediction had reasoned only that both states are NONCONTROL.

**Decision: `kAiOwnerPlayerSlotBound` is ON.** The mechanism is the image's (`009FFD20` and the
`00A2E04B` guard, both read). No death row moved on any of the seven rows. The five rows without a
slot-4 or merge consequence are gameplay-identical. The USN12 miss is recorded above.

**Still LABELLED:**
- the key is the entity name;
- generated squadrons and wing members take 9;
- the `+180h` writers other than `0077F1F9` are not modelled (the message arm `0095AC28`, the
  session dispatchers, `006F4D10`).

## 72. The retarget arm's modes 3 and 4, read (packet `cc9_approach_retarget_modes_3_4`, cc9-ships19, 2026-09-30)

**Read whole:** `009F2161-009F2395` (disk bytes, `local\s19_arm34.txt`), and the callees `006F2DE0`,
`006F3AF0` and `00749D90` (live decompile). The latch (`ship_ai_approach_mode_latch_009f1f47`)
gives mode 3 or 4 only when the target at `ctl+0B20h` is an enemy CommandBuilding (IsKindOf 1Ch)
and the unit's class lands troops (vtable `+2Ch`). Mode 3 is the one inside the reach;
mode 4 is the standoff.

**The shared head (`009F2169-009F219A`).** `EDI` = `[ctl+0B20h]` when it answers IsKindOf(1Ch), else
0; that value is spilled to `[ESP+14h]`. Mode 3 continues; mode 4 is taken at `009F233D`.

**Mode 3 (`009F21A0-009F2338`):**
```
ESI = unit ([ctl+0AA8h]) when IsKindOf(0Ch), a landing ship, else 0
006F2DE0(building)(ESI)              ; releases every pad of the building's +794h vector whose
                                     ; occupant (006AC220, pad+1F8h) is this unit (006AC490(0)),
                                     ; under the critical section at building+764h
EBX = 006F2E60(building)(ESI, 0)     ; the pad the unit holds, else the nearest free one
if (EBX == 0) done                   ; 009F21D4
point = 006AC5D0(pad)(&out, &unit+0FCh, [[unit+538h]+570h], 200.0f [00CE386C])
nested+1228h..1230h = point          ; 009F2214-009F2228
d = |point - unit+0FCh| (0042B2F0)
reach = max(00811A30(unit, 1.0) * 2.5 [00CE3DE0 double], 300.0f [00CE3AE8])   ; 00415550
if (!(reach > d)) done               ; 009F22E3 FCOMIP / JBE
[ctl]+3FCh = 0 (byte)                ; 009F22F0
if (unit+1200h != 0) done            ; 009F22F7
msg = 00749D90(&local, pad, building); 0077C2A0(unit)(msg, 7, 0)
```
`00749D90` (`00749D90-00749DEF`, `RET 8`) builds session message 0A5h
(`BSP_SessionMessage_ConstructBase(0A5h)`, vtable `00CFF908`). It carries the pad's `+174h` id at
`+1Ch` and the building's `+174h` id at `+1Eh`. `0077C2A0` is `EntityOrder::route_message`, so
the landing ship is told to land at that pad.

**Mode 4 (`009F2342-009F2395`):**
- The point is `006F3AF0(building)(&out, &unit+0FCh, [[unit+538h]+570h])`.
- `006F3AF0` (`006F3AF0-006F3CBB`, `RET 0Ch`) takes the xz of the pad nearest the unit, by 3-D
  squared distance over the `+794h` vector. The seed is `FLT_MAX` (`00D7A248`). With no pad it
  takes the building's own xz.
- It then hands that point to `00417E60` on the avoid-zone manager singleton, with 10.0f
  (`00CE38B8`) and the class's zone group. Its answer's x and z are stored with y = 0.
- `00417E60` is unread; presumably it pushes the point out of that zone group.

**Why nothing is bound.** Neither mode can run in this process:
- no building carries a landing-pad vector (`+794h`/`+798h`), and no pad carries an occupant
  (`+1F8h`), an id (`+174h`) or an approach cache (`+208h..`, 006AC5D0);
- `00417E60` and the unit's handler for message 0A5h are unreconstructed. `ship_ai_follow_land`
  declares the pad seams (`pick_landing_pad_006f2e60`, `pad_approach_point_006ac5d0`), but no host
  implements them;
- `unit+1200h` and `ctl+3FCh` have no reader here.

**No reference row reaches them.** The latch census in reference r (`summary mission ship ai
approach latch`, `local\g15_rr_*.log` in cc9-gunnery15's tree) shows `lander=0` and modes 3 and 4
at 0 on all 17 rows. Only LOMP10 and LOMP10 long latch a building target (601 and 1801 frames),
and neither of those is a landing ship. The close-attack gate also refuses landing-ship members
(`00A1443D`, `ai_close_attack_member_served`). A planner reaches these modes only through a
script attack order or the Capture spawn arm (`00A2B400`, a record here).

**Decision: labelled, no switch.**
- The host comment at `ShipAiApproach::retarget_modes_3_4` now names the missing producers.
- A future packet needs the building pad model first. That is the units/landing lane:
  - the pads are built with the building (`+794h`);
  - `006F2FB0`, `006AC490`, `006AC5D0`'s cache and `00417E60` come next;
  - then the message-0A5h landing handler.
- Only after that can modes 3 and 4 be bound. They also need a row with a troop landing on an enemy
  CommandBuilding: this installation's BSM03 and BSM08 author LandingShipGen units, but as
  "AI control" (section 71), so they are NONCONTROL and not planned.

**Names for the lead (hypotheses, bodies verified RET then INT3):**
- `006F2DE0-006F2E54` `BSP_CommandBuilding_ReleaseUnitPads` (`__thiscall(building)(unit)`, `RET 4`);
- `006F3AF0-006F3CBB` `BSP_CommandBuilding_NearestPadStandoffPoint` (`__thiscall(building)(float3*
  out, const float3* from, int zone_group)`, `RET 0Ch`);
- `00749D90-00749DEF` `BSP_SessionMessage_LandAtPad_Construct` (message 0A5h,
  `__thiscall(msg)(pad, building)`, `RET 8`).

## 73. Handoff (cc9-ships19, 2026-09-30, at about 70% context)

**Landed on main:**
- 70 (`kCaptureGroupValueBound` ON: 00A250A0 replaces the members x defenders stand-in);
- 71 (`kAiOwnerPlayerSlotBound` ON: `OwnerPlayer` reaches 009FFD20; the Party ordinals are settled
  from `global.enums`);
- 72 (retarget modes 3 and 4 read; nothing bound).

The lead applied the three names from 72.

**Switches this lane changed:**

| switch | state | section |
| --- | --- | --- |
| `kCaptureGroupValueBound` | ON | 70.1 |
| `kAiOwnerPlayerSlotBound` | ON | 71.3 |
| `kShipAiObstacleBackoffCountdownBound` | still OFF, exact, no reach | 65.5 |

### The next main packet: the troop-landing / CommandBuilding pad model

The goal is for retarget modes 3 and 4 (`009F21A0-009F2395`, section 72) to run. Every address below
has been read, unless it is marked unread.

1. **Build the pads with the building.**
   - A CommandBuilding holds a pad vector at `+794h` (begin) / `+798h` (count, stride 4), guarded by
     the critical section at `+764h`.
   - Each pad carries:
     - its occupant at `+1F8h` (`006AC220`: `MOV EAX,[ECX+1F8h]; RET`);
     - an id at `+174h` (read by `00749D90`);
     - its owner base at `+220h`;
     - a cached approach line at `+208h..+21Ch` (docs/SHIP_AI_OPEN_ITEMS / `ship_ai_follow_land.hpp`
       line 294).
   - The producer that fills `+794h` has not been found yet.
     - Start from the CommandBuilding creator `006F2780`, which also stores `CaptureRange +7A0h` and
       `LandingRange +7C4h`, and from the Landscape/model attach.
     - Scan disp32 `794h` stores with `local\s19_disp.py 794`, and read every hit with
       `s19_before.py`.
2. **The pad routines.**
   - `006F2E60` (pick the pad: the one this unit holds, else the nearest free one; read) and
     `006F2DE0` `BSP_CommandBuilding_ReleaseUnitPads` (read).
   - `006F2FB0` (assign; read, but its inner calls are unread) and `006AC490` (clear or set the
     occupant; unread).
   - `006AC5D0`: the per-call arm is reconstructed as `ship_ai_land_pad_approach_point_006ac5d0`; the
     cache refresh is not.
   - `006F3AF0` `BSP_CommandBuilding_NearestPadStandoffPoint` (read).
   - `00417E60` on the avoid-zone manager (unread; takes 10.0f `00CE38B8` and the class zone group).
   - The seams are already declared in `include/bsp/ship_ai_follow_land.hpp`
     (`pick_landing_pad_006f2e60`, `assign_landing_pad_006f2fb0`, `pad_approach_point_006ac5d0`,
     `pad_occupant_006ac220`). No host implements them. The units host owns the buildings, so this is
     shared with the units lane.
3. **The landing message.**
   - `00749D90` `BSP_SessionMessage_LandAtPad_Construct` builds message 0A5h (vtable `00CFF908`, pad
     id at `+1Ch`, building id at `+1Eh`). `0077C2A0` routes it at class 7.
   - The unit's 0A5h handler is unread; start from `Unit_HandleMessage` `0095ABE0`.
   - Mode 3 also reads `unit+1200h` and writes `[ctl]+3FCh`; neither has a host counterpart.
4. **Bind modes 3 and 4 OFF, then pair on a row that reaches them.**

**Which rows could reach a troop landing:**
- **JM08 (scripted, the best candidate).** `scripts/missions/COTP-IJN/PRCPIJN/jm08.lua` (and
  `prcpjm08.lua`; mtimes 2024-07-13 and 2024-08-26, and the second is the one this installation
  loads, section 74.1):
  - line 757, `CheckInvasion`: when Allied ships come within 300 m of (0,0,0),
    `StartInvasion` orders `NavigatorAttackMove(unit, Mission.HQ)` for the invasion force. It also
    sends `USTroopTransport 01..06` (`Mission.APs`, line 533) to `Mission.LandPoints`.
  - line 825, `CheckAP1..6`: within 200 m of its land point, each transport is ordered to
    `NavigatorAttackMove(AP, Mission.HQ)`.
  - `Mission.HQ` is `Headquarter 01`, a `CommandBuilding` (`prcpijn_08_defend_guadalcanal.scn` line
    5132). So a landing-class unit gets an enemy CommandBuilding target from the script, not from a
    planner.
  - The reference JM08 row (3000 frames) latches nothing (`approach latch frames=0` in
    `g15_rr_jm08.log`), so the invasion has not started by then.
  - **First step:** run JM08 at 9200/9000. Check `summary mission ship ai approach latch` for
    `lander>0` and modes 3/4 > 0, and check which script file the row loads.
- **BSM02 and BSM06** have landing scripts too (`luaSpawnLandingWave`, `luaCommenceLandings`,
  `luaLCVPLanding`); they are not reference rows. `multi/siege907.lua` is multiplayer.
- **BSM03 and BSM08** author their LandingShipGen units as `OwnerPlayer "AI control"` (section 71),
  so no planner orders them. Only their scripts could.
- **Planner reach** is nearly closed:
  - the close-attack gate refuses landing-ship members (`00A1443D`);
  - the Capture spawn arm (`00A2B400`, a record here) would be the planner path to a landing.

**Still waiting:**
- **The back-off countdown** (65.2, `kShipAiObstacleBackoffCountdownBound` OFF) needs a row where
  `009F47A7` arms (`held_steps > 0` in `summary mission ship ai backoff countdown`). None of the
  rows run in 70-72 armed it; I did not check each log for it.
- **The t=0.05 weapon-facts timing** (70.1) was routed by the lead to cc9-lua21.
- **The `global.enums` comment** in `game_hosts_mission.cpp` (71.1) was routed to the mission-host
  owner.

**Tools** (in `J:\PROG\battlestations-pacific-decompile-cc9-ships19\local\`, `s19_` prefix):
- the s18 tools, retargeted: `s19_rel32.py`, `s19_disp.py`, `s19_before.py`, `s19_vslot.py`,
  `s19_kinds.py`;
- `s19_runs.ps1 -V <name> [-Exe <path>] -Only <rows>` (adds usn12, usnos and jm08) and
  `s19_wait.ps1`;
- `s19_short.ps1 -Menus <rows>` (300-frame census runs) and `s19_diag.ps1` (one run with
  `BSP_CAPTURE_DIAG=1`);
- `s19_capval.py <log>` (the capture group values per group);
- `s19_owners.py` (every authored `OwnerPlayer` in this installation's scenes);
- `s19_str.py <text>...` (string VA and abs32 refs in the PE) and `s19_dump.py <va> <n>` (dwords
  with the strings they point at).

## 74. Which row reaches a troop landing (packet `cc9_script_entity_pool`, cc9-ships20, 2026-09-30)

Section 73's step (0): run JM08 long and see whether the scripted invasion starts.

### 74.1 The census on main `15f066a2d` (runs 2026-09-30 04:19-04:27 UTC)

Launch form of the reference rows (`--press-start-frame 30 --mission-frame-seconds 0.05`,
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, 1600x900), logs `local\s20_m_<row>.log`:

| row | frames | script | approach latch | lander | modes 3/4 | script `attackmove` |
| --- | --- | --- | --- | --- | --- | --- |
| JM08 | 9200/9000 | `PRCPJM08.lua` | 336 frames, all `other`, all mode 0 | 0 | 0/0 | 0 |
| JM08 | 36200/36000 | same | 1828 frames, all `other`, all mode 0 | 0 | 0/0 | 0 |
| BSM02 | 9200/9000 and 36200/36000 | `bsm_02_defense_of_the_philippines.lua` | 0 | 0 | 0/0 | 0 |
| BSM06 | 9200/9000 and 36200/36000 | `bsm_06_holding_lombok.lua` | 0 | 0 | 0/0 | 0 |

- The script this installation loads for JM08 is `scripts/missions/COTP-IJN/PRCPIJN/prcpjm08.lua`
  (mtime 2024-08-26 16:10, not 2024-07-13 as section 73 says; `jm08.lua` is 2024-07-13 and differs
  only in its script path, name and message map).
- **JM08 does not reach, and the reason is the host, not the mission.** JM08's `script_calls`
  stops growing at mission frame 2001 (2889, the same at 3000, 9000, 18000 and 36000). The end
  summary says `timers created=512 ... deletes=511`.
- `GameScriptOrdersHost::script_entity_create_00898841` stops at `kScriptEntityCapacity = 512`
  and answers no entity from then on. Every `luaDelay` makes one script entity. JM08 re-arms about
  ten one-second checks (`CheckInvasion`, `CheckAP1..6`, `CheckPrim1/2`, `CheckCompletion`, ...), so
  its 512 are spent by about 100 s of mission time, and every delayed chain then stops.
- `CheckInvasion` polls for an Allied ship within 300 m of (0,0,0) once a second. Its chain died at
  frame 2001, so even an Allied fleet arriving later would start nothing.
- The image has no bound: `BSP_LuaBinding_CreateScript` allocates each entity with `operator_new`
  (0x1E4 bytes at `00898834`-`00898841`, then `_memset` and the construct `00928630` at `0089886F`).
- BSM02 (1421 calls by frame 36000) and BSM06 (27 calls) spend less than 512, so the cap is not
  their limit. Both issue their landings with `NavigatorMoveOnPath` / `NavigatorMoveToRange` to
  path points, not to a CommandBuilding (`bsm_02...lua` lines 1039-1098, `bsm_06...lua` 611-640),
  and their latch stays 0.

### 74.2 The pool switch, `kScriptEntityPoolUnboundedBound` (committed OFF, flipped ON in 74.3)

- ON: the records live in a `std::deque` (element addresses stay valid on `push_back`, which is
  why the vector needed a reserved capacity), the cap is not applied, and the id is still
  `kScriptEntityIdBase + index`. OFF keeps the 512 cap exactly. The storage type changes in both
  modes; OFF behaviour is unchanged because a reserved vector never reallocated below 512.
- **Predictions (written before any flip run):**
  - JM08 long moves (exit 3): `script_calls` keeps growing past frame 2001, and `timers created`
    goes past 512. Whether the invasion starts depends on an Allied ship reaching 300 m of the
    origin, which this reading cannot predict; if it does, `attackmove` > 0 and some transport
    latches `building` or `lander`.
  - Every other row whose script makes more than 512 delayed calls moves too. BSM02 long, BSM06
    long, and the 3000-frame reference rows that stay under 512, are identical (exit 0 or 1).
  - The run-time cost is linear scans over more records (`script_entity`, the think pass); no
    crash.

### 74.3 The pairs, and the flip

Same-tree pairs on `4e3280266` (OFF = the tree build, ON = `local\s20_p1`, `96ACBC609FE5`), run
2026-09-30 04:46-04:55 UTC, `local\s20_pd1_<row>.txt`:

| row | pair_diff | script entities created (ON) | cap hits (OFF) |
| --- | --- | --- | --- |
| USN04, USN01, USN13, JM05, IJN01, LOMP10, JM05 long, USNOS | 1 | 10 to 346 | 0 |
| USN12 | 0 | 33 | 0 |
| JM08 (3000) | 1 | 1048 | 9 |
| JM08 long (9000) | 1 | 3892 | 9 |
| JM08 36000 | 1 | 16681 | 9 |

- The mechanism matches the prediction: past 512 the ON side keeps creating (`CreateScript`
  521 -> 16681 calls on JM08 36000, `GetHpPercentage` 171 -> 5559), and the OFF side logs the cap.
- The gameplay prediction for JM08 missed: every JM08 row is gameplay-identical (27 deaths, the
  same per-entity death rows and unit table). The script now runs, but `CheckInvasion` never finds
  a ship (74.4).
- **Verdict: flipped ON.** The image has no bound, and no row's gameplay moves. The flip commit is
  the one that adds this subsection.

### 74.4 Why the invasion still does not start: the `recon` tables are empty

`BSP_ORIGIN_DIAG=1` (added with this packet, env-gated, in `run_script_think_pass`) logs the
nearest live unit of each party to (0,0,0) every 30 s. It also runs a Lua chunk that calls
`luaGetShipsAroundCoordinate(origin, 300, PARTY_ALLIED, "own")` and counts `recon[PARTY_ALLIED].own`.
Runs: `local\s20_diag_jm08x.log` (pool OFF) and `local\s20_diag2_jm08x.log` (pool ON), JM08
36200/36000.

- **The ships do arrive.** Party 0 is `PARTY_ALLIED` in this installation
  (`scripts/global/luamw_init.lua` 73-75, cited at `src/game_hosts_lua.cpp` 2316). Its nearest unit
  to the origin is `Grayson` at 248.6 m at t = 540.6 s, and `USTroopTransport 05` at 42.8 m at
  t = 720.8 s. Both are inside `CheckInvasion`'s 300 m.
- **The script cannot see them.** At every sample, `PARTY_ALLIED` is 0, `around` is 0 and
  `recon[0].own` has **0 entries across all nineteen categories**.
- `luaGetShipsAroundCoordinate` (`scripts/global/commandhelpers.lua` line 408, mtime 2024-10-29,
  this installation) walks only `recon[party][allegiance]`.
- The host builds the shell (`install_recon_tables_00803a40`, `src/game_hosts_lua.cpp` 2256) and
  records `Recon::publish_slot_table 00806b10` as unimplemented (one call in every run). Nothing
  fills the category maps. `docs/FIXED_STEP_COUNTDOWN.md` says `00806B10` was read only to
  `00806BAE`.
- **Consequence:** every `luaGetShipsAround*` / `luaGetOwnUnits`-style query in every mission
  answers nil or empty. This matters beyond JM08: any script that waits on a proximity test never
  advances.
- This is the Lua host's and the recon pass's lane (`src/game_hosts_lua.cpp`,
  `src/fixed_step_countdown.cpp`, `00806B10`/`00805D90`/`008073C0`), so I have not touched it.
  Once the `own` maps are published, JM08 should start its invasion at about t = 540 s
  (mission frame about 10800), so JM08 at 36200/36000 is the reaching row for section 73's pad
  model.

### 74.5 Section 73's step (1), read ahead: what fills the pad vector

- **The producer** is `BSP_CommandBuilding_AdoptNearbyGarrison` `006F5CC0`, CommandBuilding
  vtable `00CFB028` slot `0A4h` (via `006F6740`, which runs `00748CC0` first, then this, then copies
  `+54h` into `+7B8h`).
  - The ledger's body end `006F5EC8` is wrong: the routine runs on to `006F673E`, and
    `006F5EC8` is the start of its second loop.
  - It makes three passes over the world registry `[[00E188A8]+19CCh]`, whose lists are
    `{count, head, tail}` at `+18h + id*0Ch`:
    1. list 5 (`+58h`) for kinds `1Bh`/`45h`/`46h` within `InferiorRange`;
    2. list `4Dh` (`+3B8h`, `006F5ED3`) into the list at `+788h`, setting `entity+344h = building`;
    3. **list `1Dh` (`+178h`, `006F6442`), the LandingPoints.**
  - The third pass keeps every LandingPoint whose squared distance to the building is
    `<= (float)(LandingPointRange * LandingPointRange)`. `LandingPointRange` is `+7CCh`, an integer
    product. It push_backs the pad into `+794h` (begin) / `+798h` (count) / `+79Ch` (capacity),
    growing by `2n + 2` (`006F66B1`-`006F66F9`), and sets `pad+220h = building`.
- **Ghidra drops blocks after `_free`**: the decompile shows `_free(old); return;` inside the
  growth. The listing continues (the new begin is stored at `006F66F9`). Flow repair is needed
  before anyone reads this routine from the pseudocode.
- **Offset correction:** `docs/LAND_AND_STRUCTURES.md` (line 340) names `+7CCh` `InferiorRange`.
  `006F2780` stores `InferiorRange` (`00CFAE20`) at `+7C8h` (`006F288C`) and `LandingPointRange`
  (`00CFAE0C`) at `+7CCh` (`006F28B3`), as `docs/SENTITY_INIT_PASSES.md` (line 966) already says.
- **The pad is a `LandingPoint`**: class `1Dh`, factory `004E9D40` (0x224 bytes), construct
  `004E9520`. The construct gives:
  - an observer handle at `+1E4h` (vtable `00CE75CC`), whose target is the occupant at `+1F8h`
    (`006AC490`: unregister the old occupant, store the new one, `BSP_Observer_RegisterPair` on
    the new one);
  - `+1F4h` byte 1, `+200h = 004E7A10()`, `+208h = -99` (the approach-cache key, `006AC5D0`
    compares it with its layer argument), `+218h = [00CE3804]`, `+220h = 0`.
- **In JM08**, `Headquarter 01` authors `LandingPointRange = I 1000`, `LandingRange = I 4000` and
  `InferiorRange = I 100000`. It is a child of `Landscape 01` at (-19500, -15900), so its world
  position is about (1349, -4468). `LandingPoint 01` (top level, at (1820, -4010)) is about 657 m
  away, so it becomes a pad. Checking the other seven is the model packet's job.
- **The pad routines, read:**
  - `006F2E60` pick: for each pad, a free one is a candidate by squared distance to the unit;
    one held by the unit returns at once unless the ignore-held byte is set; otherwise the
    nearest free pad, or 0.
  - `006F2FB0` assign: returns early if the pad already holds the unit; otherwise, under `+764h`,
    `ReleaseUnitPads(unit)` and then `006AC490(unit)` on the pad.
  - `006AC490` is `__thiscall(pad)(unit)`, `RET 4`, body `006AC490`-`006AC4C3`.
- The host has LandingPoints as scene markers (`GameSceneMarkerSeed`, class id 1Dh, world frame),
  registered through `GameScriptOrdersHost::register_scene_marker` and
  `GameUnitsHost::register_scene_marker_frame`. It has no pad vector.

## 75. The building pad model (packet `cc9_building_pad_model`, cc9-ships20, 2026-09-30)

This is section 73's steps (1) and (2), modelled. The reading is in 74.5, and the flow repair of
`006F5CC0` is on main (`4222a3f86`).

### 75.1 What is modelled

The model is `bsp::BuildingPadModel` (`include/bsp/building_pads.hpp`, `src/building_pads.cpp`),
built behind `kBuildingPadModelBound` (committed OFF):

| routine | coverage | host |
| --- | --- | --- |
| `006F5CC0` pass 3 (`006F6436-006F673E`) | complete for pass 3; passes 1 and 2 not modelled | `adopt_landing_pads_006f5cc0`, `building_pad_in_range_006f5cc0` |
| `006F2E60` pick | complete | `pick_006f2e60` |
| `006F2DE0` release | complete | `release_unit_pads_006f2de0` |
| `006F2FB0` assign | complete | `assign_006f2fb0` |
| `006AC490` set occupant | complete; the observer pair is a SUBSTITUTION (`forget_unit`) | `set_occupant_006ac490` |
| `006F3AF0` standoff point | partial: `006F3AF0-006F3C5A` (nearest pad xz); the `00417E60` push-out is unread | `nearest_pad_xz_006f3af0` |
| `006F2780` `LandingPointRange` | complete for this key (`006F2895-006F28B3`, default 1F4h) | `GameSceneEntityRecord::landing_point_range_raw`, `GameUnitsHost::command_building_landing_point_range_07cc` |

- **Where it is built.** `GameScriptOrdersHost::building_pads()` builds the model on its first
  call, from the LandingPoint markers (class `1Dh`) in registration order and every `1Ch` unit's
  position and `+7CCh`. The image builds it at InitAll pass C. Both buildings and pads are
  stationary, so the only difference is timing.
- **SUBSTITUTIONS (labelled):**
  - Registry list `1Dh`'s order is taken as marker registration order. That order only breaks
    exact distance ties.
  - The x87 sums are done in double and rounded to float at the image's store.
  - A pad's occupant is a units-host index.
- **Nothing reads the pads yet.** Retarget modes 3/4 (section 72) and the land step
  (`ship_ai_follow_land`) are separate switches, for the next packets.
- **The marker class needs one line in `src/game_hosts_mission_frame.cpp`** (cc9-gunnery16's file,
  routed to the lead). At about line 2179, `register_scene_marker(marker.id, marker.name,
  marker.position)` must pass `marker.class_id` as a fourth argument. Until then every
  marker's class is -1 and the model adopts no pads.

### 75.2 Predictions (written before any flip run)

These are from `local\s20_pads_scn.py`, this installation's scenes (world position = own
translation plus the parents'):

| row | scene | CommandBuildings, LandingPointRange | pads adopted |
| --- | --- | --- | --- |
| JM08 | `prcpijn_08_defend_guadalcanal.scn` | `Headquarter 01` (1349, -4468), 1000 | 8 (LandingPoint 01..08 at 582-809 m) |
| USN01 | `usn_1_marshall.scn` | `CB2`, 1000 | 8 |
| USN13 | `usn_13_truk.scn` | `CB2`, `CB4`, `CBT`, 1000 each | 16 in total |
| JM05 | `ijn_05_invasion_of_port_moresby.scn` | `MainCommandBuilding 01` 800, `SecondaryCommandBuilding 01` 1500, `RadarStation 01` 500 | 28 in total |
| USNOS | `us_osumi.scn` | `HQ1` 1500, `HQ2` 1500, `CB2` 650 | 8 in total |
| USN04, IJN01, LOMP10, USN12 | - | none with a pad in range | 0 |

- The flip only adds the `summary building pads` line. **Every pair is predicted exit 0 or 1**,
  because no reader exists.
- The host's y may differ from the authored y (`__SnapToTerrain`). The nearest margin is 191 m
  (JM08's `LandingPoint 08`, 809 m against 1000), so no adoption should flip.

### 75.3 The census, and the flip

Runs 2026-09-30 05:33-05:55 UTC. OFF is `a64444960`'s build (`local\s20_off3`). ON is that build
plus the local-only mission-frame line from 75.1 and the switch on (`local\s20_on3`; the patch is
`local\s20_edit_mf.py` and was reverted before any commit). Logs are `local\s20_{off3,on3}_<row>.log`
and the diffs `local\s20_pd3_<row>.txt`.

| row | pair_diff | `summary building pads` (ON) | predicted |
| --- | --- | --- | --- |
| JM08 | 1 | `Headquarter 01=8` | 8 |
| USN01 | 1 | `CB2=8` | 8 |
| USN13 | 1 | `CB2=8 CB4=8 CBT=0` | 16 |
| JM05 | 1 | `MainCommandBuilding 01=13 SecondaryCommandBuilding 01=11 RadarStation 01=4` | 28 |
| USNOS | 1 | `HQ1=0 HQ2=0 CB2=8` | 8 |
| USN04 | 1 | no building | 0 |

- Every count matches the scene prediction. Every pair is gameplay-identical, as predicted, since
  there is no reader.
- **Verdict: flipped ON.** It stays inert until two things land: the mission-frame line, and a
  reader (modes 3/4, or the land step).
- The pads carry a units-host index for their occupant. Nothing clears it on a unit's death yet
  (`forget_unit` has no caller). The reader packet has to call it from the death path, or assert
  the substitution there.

## 76. The land-at-pad message 0A5h and what follows it (packet `cc9_land_at_pad_read`, cc9-ships20, 2026-09-30)

This is section 73's step (3). It is read only; nothing is bound. The listings are disk bytes
(`disasm-raw`), because Ghidra has no function at several of these starts.

### 76.1 The landing ship's message handler

`0074B570` is `MLandingShip`'s HandleMessage, at vtable `00CFFA30` slot `164h` (`00CFFB94`). It
is the same slot that holds `00821E80` in the battleship (`00CF9214`) and cruiser (`00CFB89C`)
vtables. It is `__thiscall(ship)(msg)`, `RET 4`, body `0074B570-0074B68B`, and switches on the byte
`msg+10h`:

| message | what the handler does |
| --- | --- |
| `0A5h` (land at pad) | `ship->vtable[148h](1FFh, 8)` (unread). Resolves the building from its `u16` id at `msg+1Eh` and the pad from `msg+1Ch`, through the id tables at `00F89A54` / `00F89AA8` (split at `[00F89A10]`). Then **`0074A990(ship)(pad, building)`**. If the ship is the HUD's selected unit (`[[00E198C4]+40h]` through `00644A60`), it also updates the HUD (`00566050`, `004CC460(34h, 0)`, one vtable call). Returns 1. |
| `0A6h` | `0074A420(ship)`: sets `+1188h` and `+1189h` to 1 and copies a part pose into `+11B4h..+11BCh` (the ramp). Returns 1. |
| anything else | `00821E80`, the ship handler. |

### 76.2 `0074A990`: begin landing at the pad

It is `__thiscall(ship)(pad, building)`, `RET 8`, body `0074A990-0074AA8B`. Everything runs under
the building's lock (`006F1EE0` / `006F1F00` with `ECX = building`):

1. `ship+1204h = building`, `ship+1200h = pad`.
2. `006AC490(pad)(ship)`: the pad's occupant becomes the ship.
3. **`ship+1210h = (uniform(0, 0.75f) + (float)ship+1208h) * 1.5`**:
   - `uniform` is `00BD2F10`; the 0.75f is at `00CEE07C`;
   - `ship+1208h` is an int, via `FIADD`;
   - the 1.5 is a double at `00CE3D78`.
   - `ship+1208h` is the scene key at `00CFFC94`, `"LandingCommanderPlayer"`, stored at `0074C5F4`. What that key means is not read.
4. `004A4520([game+21D0h])(pad, building, ship)`. `game+21D0h` is the TrafficConfig, and this is
   the ground-troop traffic (body `004A4520-004A489F`, read to about `004A4700`):
   - for each path handle in the pad's list at `pad+200h` (built by the construct's `004E7A10`),
     it either creates a 0xE4-byte traffic record (`004A0410`) carrying the building and the ship,
     or re-targets an existing record's `+0DCh` to the ship;
   - so the landing puts soldiers on the pad's authored `Path`/`PathEndZ` routes toward the
     building.
5. Unless `[game+1FE4h] == 2`, it issues the **`land` command (`00E08FA0`)** to the ship through
   `0077D600`, carrying the pad's world position (`pad+0FCh..+104h`), a flag byte 1 and 0.0f.
   So the ship's own `land` state (`009E1950`) takes over.

### 76.3 The land state's enter, and what the building gets

- **`009E18D0`** is the `land` state's enter (vtable `00D21658` slot 4), body `009E18D0-009E194C`:
  - `state+8h = 0.0f`, then `state+0Ch = 1.0f` (`00D7A24C`);
  - when the unit at `brain+0AA8h` answers IsKindOf(0Ch), **`state+8h = unit->vtable[248h]()`**.
    That slot is `0074BC10`, `FLD [ECX+1210h]; RET`, so it is the landing time from 76.2;
  - `state+10h = 0`, then `0080E490(unit, 0)` and `0092BD70` on the result (unread).
- **This settles the open point in `ShipAiLandState::speed_ramp_08`**: its producer is this enter,
  not the construct `009F39C0`.
- **The building effect is not a message to the building.**
  - `0074A5A0` (vtable slot `17Ch`, body `0074A5A0-0074A624`) builds message 0A4h (`00749BF0`: pad
    id `+12Ch`, building id `+12Eh`, `+1208h` at `+130h`). It sends that only to non-local peers
    (`00779FC0` -> `BSP_Session_SendMessageToNonlocalPeer`), so it is multiplayer replication.
  - The CommandBuilding's handler (`006F5460`, slot `164h`) takes only `D3h..D6h` and passes the
    rest to `00744BE0`.
  - What the landing does to the building therefore goes through the soldier traffic of 76.2 step 4.
    Whether a soldier reaching `PathEndZ` feeds the capture countdown (`+7C0h`,
    LAND_AND_STRUCTURES section 4) is **unread**: the next reading target is the `004A0410` record
    and its tick.
- **`0074A4C0`** (vtable slot `238h`, body `0074A4C0-0074A59A`, SEH) is a second way in:
  - it returns -8 when `ship+1200h` is already set;
  - otherwise it finds a building with `006F2C30(&ship+0FCh, party ship+54h, 2, &err)` and fails
    with `err - 8` when there is none;
  - it takes a pad with `006F2A50(building)(ship)` and returns -4 when there is none;
  - otherwise it **writes `ship+1200h = pad` before** building the 0A5h message
    (`00749D90`) and routing it (`0077C2A0`, class 7), and returns 1;
  - its callers (through slot `238h`) and the two helpers are unread.

### 76.4 `00417E60`, mode 4's push-out

- It is `__thiscall(manager)(float2* out, const float2* point, float margin, int layer)`,
  `RET 10h`, body `00417E60-00417E8C`, complete:
  - `group = 00412120(manager)(layer)` (`GroupForLayerOrBelow`);
  - then `00417B10(group)(out, point, margin, 1)` (`OffsetPointSequential`);
  - it returns `out`.
- The host already has both halves in `game_hosts_ship_ai.cpp`: `zones.group_for_layer` and
  `zones.offset(set, xz, margin, true)`, as the follow step's `push_out_of_zones_00417b10` uses them.
- So mode 4's point is `zones.offset(zones.group_for_layer([[unit+538h]+570h]), nearest pad xz,
  10.0f, true)`, stored with y = 0.

### 76.5 What binding modes 3 and 4 needs (the next packet)

- **Mode 3:** `pick_006f2e60`, the approach point `006AC5D0` (cache refresh plus per-call arm), the
  reach test, `[ctl]+3FCh = 0`, and on `unit+1200h == 0` the 0A5h path, which is 76.2 in the host:
  - set the unit's pad and building;
  - `set_occupant`;
  - the landing time;
  - issue `land` at the pad.
  The soldier traffic (step 4) stays a record.
- **Mode 4:** 76.4.
- **The unit's `+1200h` / `+1204h` / `+1210h` need a host home.** No host field exists for them.
  The pad model's occupant is the pad side of the same link.
- **Unit death calls `BuildingPadModel::forget_unit`** (the observer substitution).

**Names for the lead** (hypotheses; each end is the byte after `RET` and is followed by `INT3`, from
`disasm-raw`):

| range | name | ABI |
| --- | --- | --- |
| `0074B570-0074B68B` | `BSP_LandingShip_HandleMessage` | `__thiscall(ship)(msg)`, `RET 4` |
| `0074A990-0074AA8B` | `BSP_LandingShip_BeginLandingAtPad` | `__thiscall(ship)(pad, building)`, `RET 8` |
| `0074A4C0-0074A59A` | `BSP_LandingShip_RequestLandingAtNearestBuilding` (provisional) | `__thiscall(ship)()`, `RET`, int result |
| `0074A5A0-0074A624` | `BSP_LandingShip_ReplicateLandingState` | `__thiscall(ship)(arg)`, `RET 4` |
| `0074BC10-0074BC17` | `BSP_LandingShip_LandingTime_1210` | `__thiscall(ship)()`, float in ST0 |
| `009E18D0-009E194C` | `BSP_ShipAiLand_Enter` | `__thiscall(state)()`, `RET` |
| `00417E60-00417E8C` | `BSP_AvoidZoneManager_PushOutForLayer` | `__thiscall(manager)(out, point, margin, layer)`, `RET 10h` |

`0074B570`, `0074A4C0`, `0074BC10` and `009E18D0` have no Ghidra function (lookup finds only an
enclosing candidate: `0074AF20`, `0074A420`, `0074BB00`, `009E1610`), so they need
`ghidra_define_function.py`. `0074A990`, `0074A5A0` and `00417E60` exist as `FUN_`.

## 77. Retarget modes 3 and 4, bound OFF (packet `cc9_landing_modes_3_4`, cc9-ships20, 2026-09-30)

This is section 73's step (4). The reading is in sections 72 and 76, and the pad model in 75.

### 77.1 The binding (`kShipAiApproachLandingModesBound`, committed OFF)

`GameShipAiHost` `run_landing_modes_3_4` (`src/game_hosts_ship_ai.cpp`) runs over
`bsp::building_pad_model()`. The coverage column names what is left out.

| step | image | host | coverage |
| --- | --- | --- | --- |
| building | `009F2169-009F2189` | target `ctl+0B20h` when IsKindOf(1Ch), else a record (the image would call with `ECX = 0`) | complete |
| mode 4 | `009F2342-009F2395` | `nearest_pad_xz_006f3af0`, then `zones.offset(zones.group_for_layer(class+570h), xz, 10.0f, true)`, y = 0 | complete. An empty zone table leaves the point unmoved (the image reads slot 0). |
| mode 3 unit | `009F21A0-009F21BC` | the unit when IsKindOf(0Ch), else -1 | complete |
| release, then pick | `009F21C1`, `009F21CB` | `release_unit_pads_006f2de0`, then `pick_006f2e60(unit, false)` | complete |
| approach line | `006AC5D0` first arm (`006AC5F2-006AC927`) | `bsp::refresh_pad_line_006ac5d0` over `PadLineZoneQueries`. It uses the new `GameAvoidZoneRuntime::group_containing` (004178F0) and `zone_segment_point` (00416DD0), plus `group_segment_point` (0041B4E0). | complete; the x87 products are done in double |
| approach point | `006AC5D0` per-call arm | `ship_ai_land_pad_approach_point_006ac5d0` (existing) | y taken as 0 (SUBSTITUTION) |
| reach | `009F2247-009F22E7` | `max(turn circle(1.0) * 2.5, 300)` against the 3-D distance | complete |
| `[ctl]+3FCh = 0` | `009F22F0` | record `ShipAiApproach::brain_byte_3fc` | not modelled |
| 0A5h | `009F2304-009F2328` -> `0074B570` -> `0074A990` | delivered at once (SUBSTITUTION: no route) | see the next rows |
| `vtable[148h](1FFh, 8)` | `0074B5B0` | record | unread |
| link, occupant, landing time | `0074A9AA-0074A9E6` | `begin_landing_0074a990` with `ship_ai_draw(unit, 0, 0.75)`. SpawnPhase is 0: no scene in this installation authors it. | complete |
| soldier traffic | `004A4520` | record `TrafficConfig::launch_pad_troops` | **not modelled (packet 5)** |
| `land` command | `0074AA0B-0074AA75` | `issue_script_command(unit, 00E08FA0, {position, pad world position}, 1)` | complete |

- **Death:** `GameScriptOrdersHost::run_script_timers` calls `forget_unit` for every destroyed
  unit (the observer substitution of 75.1).
- **Summary line:** `summary mission ship ai landing modes`.
- **The OFF smoke** (USN01, 300 frames, `local\s20_off4_smoke.log`) passed, with `bound=0` and all
  counters at 0.

### 77.2 Predictions for JM08 36200/36000 (written before any flip run)

These are for after cc9-lua22's recon publication lets `CheckInvasion` start the invasion (section
74.4: an Allied ship is inside 300 m of the origin at about t = 540 s).

- **Who can land.** `StartInvasion` orders `NavigatorAttackMove(unit, Mission.HQ)` for all of
  `Mission.InvasionForce`.
  - Only the `LandingShipGen` members are IsKindOf(0Ch): `LST 01`, `LST 02`, `LSM 01`, `LSM 02`
    (`LST 03` is in `LandShips` but not in the invasion force).
  - The six `USTroopTransport` units are `MCargo` (creator `006EB290`, vtable `00CFA778`, `kind=11`
    in the hull line). If they latch mode 3, the unit is null, so `pick` answers none
    (`no_pad`) and they never land.
  - The latch also needs vtable `+2Ch` (the class lands troops), the host's
    `unit_class_lands_troops_vtable_2c`:
    - an `MLandingShip` answers yes unless it is a rocketer (`+809h`);
    - a cargo ship answers yes when its class carries a landing craft (`+78Ch` and `+790h`
      non-zero).
    - So the transports may latch too. For them mode 3 counts `no_pad`, and their own landing
      (spawning landing craft, class field `+794h` LandingShipCoolDown) is a different path that
      is not read.
- **Expected ON counts**, if the latch admits them:
  - `mode4_points > 0` (the approach from outside the reach);
  - `mode3_points > 0` and `line_casts = 8` at most (one per pad per layer; the SP class layer is
    0);
  - `begins` from 1 to 4 (each landing ship once: after it lands, `+1200h` stays set);
  - `in_reach >= begins`.
- **Gameplay:** exit 3 on JM08 36000. The `land` command moves each lander into the `land` state
  (`009E1950`). The death table should differ only through those four ships and whatever shoots at
  them.
- **The other rows** latch no lander (reference r: `lander=0` on all 17), so they are predicted
  exit 0 or 1.
- **The building is not captured** in either run: the soldier traffic is a record (packet 5).

## 78. How a landing captures: the CommandBuilding capture tick `006F6760` (packet `cc9_capture_tick_read`, cc9-ships20, 2026-09-30)

This is the lead's packet 5. It is read only; nothing is bound. The decompile is
`local\s20_6f6760.txt` (399 lines, paged).

### 78.1 The soldier traffic does not capture

- `004A4520` (76.2 step 4) builds 0xE4-byte traffic records (`004A0410`, mis-tagged
  `CG_array_ctor_helper`, 31 callees including `SoldierClass_GetOrLoad_004B1400`) on the pad's
  paths.
- Nothing in the capture tick reads them. **They are the visible soldiers, not the capture
  input**, so they can stay a record.

### 78.2 What the capture tick counts

`006F6760` is `__fastcall(building)`, body `006F6760-006F7352`, the CommandBuilding's capture
tick. It sums a capture strength per party (`local_16c[party]`, party `+54h` < 2), and per player
slot (`+188h`, < 8) for the scoring:

1. **Ships in CaptureRange.** It walks world list 6 (`[[00E188A8]+19CCh]+64h`, the ships). A unit
   counts when all of these hold:
   - its squared distance is `<= (float)(CaptureRange * CaptureRange)` (`+7A0h`, int product);
   - it is not IsKindOf(0Ch) with `+1188h` set;
   - it is alive and active (`+5Ch` set; `+5Dh`, `+60h`, `+5Eh` clear);
   - its party is < 2.

   The value it adds comes from `__ftol` (`00BF7420`) of an x87 value that the decompile does not
   show, scaled by `BSP_GameplayModifiers_ProductForUnit(10, unit)` when the modifier table is
   active. **This value is unread (needs `--asm`).**
2. **Landed landing ships on the pads.** For each pad of the `+794h` vector whose occupant
   (`006AC220`):
   - is IsKindOf(0Ch);
   - is alive and active;
   - **has `+1188h` set** (the ramp is down);
   - has party < 2;

   the capture strength of that party gets `[[occupant+538h]+810h]`, an int on the class
   descriptor (the troop capture strength). The occupant's player slot is also marked for
   scoring.
3. **The garrison list** `+7DCh` / `+7E0h`: each member adds `[[member+314h]+0D8h]` (a float) to
   its party.

### 78.3 How the strengths move the progress

- For a building held by party 0 or 1, the owner's opponent's strength is divided by 1.0f
  (`00D7A24C`) or by a modifier.
- The progress `+7A8h` moves by the difference of the two parties' strengths:
  - A positive progress is party 0's and a negative one is party 1's.
  - With neither party present, the progress decays toward 0 by the lobby option at
    `[MultiLobbyOptionRegistry+0C0h]`.
  - A party capturing against the opposite sign first decays through 0.
- When `|+7A8h|` reaches `CaptureValue` (`+7A4h`):
  - the progress resets to 0;
  - D5h is routed (class 4, state 2);
  - the best-scoring player slot is chosen from the per-slot sums, and D3h is routed (class 7)
    with that slot;
  - the score goes through `0090F860`.
  - The ownership change itself is `vtable[1B0h]` `006F3270` (LAND_AND_STRUCTURES section 4);
    the D3h handler `006F29D0` only records `+7A4h` / `+7A8h` / `+7ACh`.
- A neutral building (`+54h == 2`) routes D5h progress updates on every change.
- `+7ACh` = the winning side of the comparison (0, 1 or 2).

### 78.4 Where `+1188h` comes from

- `0074A420` (the 0A6h handler, 76.1) sets `+1188h` and `+1189h`.
- Its other caller is the landing ship's update `0074AF20`, at `0074B080`:
  - it runs only when `ship+1200h` (a pad) is set and `[game+1FE4h]` is not 2;
  - it counts down `+11A8h` / `+11ACh` against the step;
  - when the countdown passes 0 with a condition held in `CL` (unread), it lowers the ramp
    (`0074A420`) and routes 0A6h to class 4 (`00749AA0`, then `0077C2A0`).
- `0074AF20` has no vtable reference (only an SEH catch handler is listed as a caller), so its
  caller is unread.

### 78.5 What the host needs for a landing to capture

1. The land step (`009E1950`) running for the lander, so that it reaches the pad and stops.
2. `0074AF20`'s ramp countdown and its `CL` condition.
3. The capture tick `006F6760`, at least its pad arm (class `+810h`) and the progress rule.
   Arm 1 (ships in CaptureRange) matters for every CommandBuilding row, landing or not:
   **today no building in this process is ever captured by anything.** Binding arm 1 moves
   JM05, USN01, USN13 and USNOS (their buildings with ships in range), so it needs its own
   pair set.

## 79. Handoff (cc9-ships20, 2026-09-30, at about 75% context)

### Landed on main, or on this branch at handoff

| section | what | switch |
| --- | --- | --- |
| 74 | script-entity cap removed; JM08's reach census; recon maps found empty (`00806B10`) | `kScriptEntityPoolUnboundedBound` ON |
| 75 | `bsp::BuildingPadModel` (`006F5CC0` pass 3, `006F2E60`, `006F2DE0`, `006F2FB0`, `006AC490`, half of `006F3AF0`); `LandingPointRange` parsed; marker class passed from the mission frame | `kBuildingPadModelBound` ON (inert without a reader) |
| 76 | the 0A5h handler `0074B570`, `0074A990`, the land-state enter `009E18D0`, `00417E60` read; seven routines named by the lead | - |
| 77 | retarget modes 3 and 4 over the pad model; `006AC5D0`'s cache refresh; the `land` command at the pad; `forget_unit` on death | `kShipAiApproachLandingModesBound` **OFF** |
| 78 | the capture tick `006F6760` read: soldiers do not capture; landed ships on pads (class `+810h`, ramp `+1188h`) and ships in CaptureRange do | - |

### Pending: the section 77 pairs

1. **Wait until cc9-lua22's recon publication (`00806B10`) is on main.** The lead will say
   when. Merge main.
2. **Check that JM08's invasion starts before pairing.** Run JM08 36200/36000 and confirm
   `summary mission script bindings ... attackmove>0`. `BSP_ORIGIN_DIAG=1` shows
   `around>0` once an Allied ship is inside 300 m of the origin (about t = 540 s).
3. **Pair** with
   `python tools/pair_export.py --commit <sha> --flip kShipAiApproachLandingModesBound=true --out local\<x>`
   on JM08 36000, plus the rows from section 74.3. Compare against the 77.2 predictions. Read
   the `summary mission ship ai landing modes` line and the approach latch's `lander` and
   `modes` fields.
4. **Expect the landers to stop being useful after the `land` command.** The `land` state step
   `009E1950` has a reconstruction (`ship_ai_follow_land.cpp`), but no host implements
   `ShipAiLandStepHost`. The state table row for `land` (`game_hosts_ship_ai.cpp` about line 635)
   selects the state, but its step is not run. So after `begins>0` the landers will sit in an
   unrun state. That is the next binding.

### The next packets, in order

1. **The land step host** (`ShipAiLandStepHost` over the pad model):
   - `unit_landing_pad_1200` is `BuildingPadModel::lander_pad_1200`;
   - `pad_occupant` / `pad_owner` / `pick` / `assign` are in the model;
   - `pad_approach_point` is 77's `refresh_pad_line_006ac5d0` plus the per-call arm;
   - the enter `009E18D0` sets `speed_ramp_08` from `Lander::landing_time_1210` (76.3).
2. **The landing ship's ramp**: `0074AF20` (per-frame, with a pad set). Read the `+11A8h` /
   `+11ACh` countdown and the `CL` condition at `0074B07A`, then set `+1188h` (78.4).
3. **The capture tick** `006F6760`, in two binds:
   - the pad arm plus the progress/ownership rule (landing-only rows);
   - arm 1, ships in CaptureRange. This moves every CommandBuilding row (JM05, USN01, USN13,
     USNOS), so it needs its own pair set. First read the `__ftol` input with `--asm`.
   - Ownership: `vtable[1B0h]` `006F3270` then `006F2940`.
   - Also `GetCapturePercentage` (section 49's open item) closes then.
4. **The MCargo transports' own landing** (JM08's `USTroopTransport 01..06`).
   - They are cargo ships whose class carries a landing craft: class `+78Ch` (the
     landing-ship class) and `+790h` (the amount), which is why
     `unit_class_lands_troops_vtable_2c` answers yes.
   - Mode 3 gives them no pad (the unit must be IsKindOf(0Ch)), so they land by spawning landing
     craft instead. `LandingShipCoolDown` is class `+794h` (`ship_class_fields.cpp`,
     `00833C27`).
   - Start from the readers of class `+78Ch` / `+790h` / `+794h` (scan disp32 `78C` / `790`)
     and from `GenerateObject`-style spawns of `MLandingShip` (factory `0074BE00`).
   - JM08's script also orders them `NavigatorAttackMove(AP, HQ)` near their land points
     (`CheckAP1..6`).
5. **Unchanged from section 73:** the back-off countdown (65.2, OFF, no row arms it).

### Tools

In `J:\PROG\battlestations-pacific-decompile-cc9-ships20\local\`, `s20_` prefix:
- `s20_runs.ps1 -V <name> [-Exe tree|<path>] -Only <rows>` adds `jm08l` (9200/9000) and `jm08x`,
  `bsm02x`, `bsm06x` (36200/36000);
- `s20_wait.ps1`;
- `s20_pads_scn.py <scn>` predicts pad adoption from a scene;
- `s20_near.py`;
- `s20_vt.py refs|slots`;
- `s20_bytes.py <hex>`;
- `s20_consts.py <va>:f|d`;
- `s20_disp.py`, `s20_rel32.py`, `s20_dump.py` (from s19);
- `s20_edit_mf.py` (the local census patch pattern: apply, build, snapshot the build under
  `local\<x>\build\win32\Release`, revert).

## 80. The capture tick read whole, and what a flip changes (packet `cc9_capture_tick_full_read`, cc9-ships21, 2026-09-30)

Read only; nothing is bound. Listings: `local\s21_6760asm.txt` (740 lines, `006F6760`),
`local\s21_4d10asm.txt` (`006F4D10`), `local\s21_7360.txt` (`006F7360`), all from the PE on
disk. **Headline: the capture tick runs only on a neutral building (`+54h == 2`), and no
CommandBuilding in any reference row is ever neutral**, so binding the tick's arms alone moves
no row. Section 78.5's "arm 1 moves JM05, USN01, USN13 and USNOS" is withdrawn.

### 80.1 Corrections to section 78

| was (78) | is | evidence |
| --- | --- | --- |
| the tick is the building's capture tick, run on every building | it runs only when the building's party is 2 (neutral), once per `+7BCh` = 1.0 s | `006F7360` `006F755D` `CMP [EDI-2BCh],2` (EDI = unit+310h, so unit+54h); `006F75ED..006F7617` the `+7C0h` countdown (`fsubr [+7BCh]`, `faddp`) then `CALL 006F6760`; the ctor seeds `+7BCh` = 1.0f (LAND_AND_STRUCTURES section 4); `006F7617` is the tick's only caller (rel32 scan) |
| the owner's opponent's strength is divided by 1.0f or a modifier | that arm (`+54h < 2`) is unreachable from the only caller | as above |
| D3h's handler `006F29D0` records `+7A4h/+7A8h/+7ACh` | `006F29D0` is **D5h**'s; D3h's is `006F4D10` | the CB message slot `vtable[164h]` `006F5460`: `movzx eax,[msg+10h]; add eax,-0D3h; jmp [006F55B8+eax*4]`; table `006F549E` (D3h -> `006F4D10(msg+1Ch, byte msg+20h)`), `006F54B5` (D4h -> `006F38E0`), `006F5481` (D5h -> `006F29D0(f[+1Ch], [+20h], [+24h])`), `006F54CB` (D6h) |
| ownership changes through `vtable[1B0h]` `006F3270` | `006F3270` is the **health-changed** slot; at health <= 0 it neutralizes. The capture's ownership change is D3h's `006F4D10` | `00877B90 BSP_UnitInstance_SetHealth` calls `vtable[1B0h]` at `00877C3A` on every write (not in game mode 2); `006F3270`: `COMISS 0,[+370h]`, health > 0 -> base `00958A30`; else party != 2 -> `+7B0h` = party, `+7B4h` = `+2D8h`, tail `006F2940` (`+7A8h` = 0, route D3h slot 9 through `0077C2A0` class 7) |
| arm 3 is "the garrison list" | `+7DCh` holds landed **paratroopers** (`MParatrooper`, class 31h) | `006F39B0` (adds under `+764h`) is called from `007AB590` (vtable `00D04FE4`); `006F34B0` (removes) from `007AAE10` (vtable `00D050DC`), the paratrooper's on-killed method, when its `+48Ch` building is set; segment keywords `paratrooper, soldieranim` |

### 80.2 The tick `006F6760` (`__fastcall(ECX = building)`, body `006F6760`-`006F7353`, `RET`)

`s[2]` (party strengths), `slot[8]` (`local_160`), `landed[8]` (`local_128`) and two flag
byte arrays (landed, paratrooper) start at 0. `mod` below is 1.0f (`00D7A24C`) unless
`[00E0C978]` and `[[00F88C30]+100h]` are set, then `008E6430(10, unit)`; this host's
modifier list is empty (1.0f).

1. **Ships in CaptureRange** (world list `[[00E188A8]+19CCh]+64h`): condition as 78.2.
   `006F69D2..006F69ED`: `s[party] = __ftol(class[+804h] * mod + (float)s[party])`, where
   class `+804h` is `CapturePower` (`ship_class_fields.cpp`, `IntegerOrAsFloat`, default 10).
   Then, when `+188h` (OwnerPlayer) < 8, `slot[+188h]` gets the same sum (`006F6A2C..006F6A41`).
2. **Landed landing ships on the pads** (78.2): `s[party] += class[+810h]`
   (`LandedCapturePower`, `vehicle_class_lua_load.cpp`, default 0), an integer add at
   `006F6AF7`. Scoring: slot = `+188h`, else `+180h`, else `+120Ch`; when the class's
   CapturePower is 0.0 (`UCOMISS`/`LAHF`/`TEST AH,44h`/`JP` at `006F6B15..006F6B28` skips when
   unequal) `landed[slot] += LandedCapturePower * mod`; when `+188h` < 8, `slot[+188h]` gets its
   sum and the landed flag is set.
3. **Paratroopers** `+7DCh`/`+7E0h`: `s[member+54h] = (int)((float)s + [[member+314h]+0D8h])`;
   scoring through the member's `vtable[108h]()+0ACh` owner (`+180h`) or its own `+4B8h`.
4. **Progress** `+7A8h` (a float; positive is party 0's): both present -> `+= s0 - s1`; one
   present -> the side with the opposite sign first decays by `F` then adds; none -> decays by
   `F` toward 0 and clamps at 0. `F` = `[MultiLobbyOptionRegistry+0C0h]` =
   `CaptureSettings.FallbackCapturePower`, loaded by `008D2F50` (`008D3C3E..008D3C6D`, default
   20.0f `00CE3930`); this installation's `scripts/datatables/multiglobals.lua` (mtime
   2024-07-13) authors 20 ("if nobody is in range, the counter is held toward 0 with this
   power"). The registry's ctor `005769E0` leaves `+0C0h` unwritten; `008D2F50`'s one caller is
   `00689540 BSP_MultiMenu_Init`. This host does not load MultiGlobals (`game_hosts_lua.cpp`
   6113), so a bind takes 20 as a LABELLED constant.
5. **Side** `local_174` = 0 when `s0 > s1`, 1 when `s1 > s0`, 2 when equal.
6. **D5h** (progress update, `0077C2A0` class 4, `ECX` = building): when the progress changed
   (`|new - old| > 0`) and the building is neutral (always, here): `{progress, CaptureValue,
   side}`. Its handler `006F29D0` stores them back.
7. **Completion** at `|progress| >= (float)CaptureValue` (`+7A4h`): pick `best` among the eight
   slot records `[[00E188A8]+18CCh+i*4]`: eligible when `+28h == side`, or `+8h` clear, or
   (`+9h` set and `+0Ah` clear) (`006F7100..006F711C`, the decompile is right); the first
   eligible slot, then any with a strictly larger `slot[i] + landed[i]`. In single player every
   record has `+8h` set, slot 0 has `+9h` clear and slots 1..7 have `+9h` and `+0Ah` set
   (section 60.6's table), so **only slots whose Party equals the winning side are eligible**;
   none -> `best` = 8. Then `+7A8h` = 0, D5h `{0, CaptureValue, 2}` (class 4), D3h
   `{best, silent 0}` (class 7), `0090F860(best, landed ? 0Ch : 37h, para ? 31h : 37h)`
   (scoring, one caller), `+7ACh` = side.

### 80.3 What a flip does: D3h `006F4D10` (`__thiscall(slot, silent byte)`, body `006F4D10`-`006F5454`, `RET 8`)

- `+7A8h` = 0; `+528h` = slot the first time (it starts -1).
- Not silent: the `globals.cblost_you` / `cblost_we` / `cbneutralized_we` announcements
  (`00734870`, `005CF3D0`), presentation.
- **slot 8** (no eligible slot): straight to the neutral set below, without recording.
- **slot 9** (neutralize, from `006F2940`): when the old party != 2 and game mode `[+1FE4h]` is 2,
  `+7B0h` = old party; then the neutral set: `+7D8h` = class `+190h` (a float the fixed step
  decays), `vtable[2Ch]` `00951F30` -> `00928F50` SetPartyRace`(2, +58h)`.
- **slot 0..7**: party `p` = `slot record+28h`; SetPartyRace`(p, race)` with race 2 for party 0,
  1 for party 1 (`006F4F83..006F4FB8`). When the previous owner's slot `+7B4h` still has party
  `+7B0h` and that equals `p` (the old owner retook it): `+2D8h` = -1 and the observer at
  `+2B0h` is released. Otherwise (or when `+7B0h` is 2): **repair** `006F47F0(this, 1.0, 1.0)`
  and the same for every garrison slot occupant (`+778h`, stride 64h, occupant at `+14h`), then
  `vtable[214h]` `006F3010`, which runs `006AC4D0` on every pad of `+794h` (walks the pad's
  `+1FCh` list when `[game+21D0h]` is set; contract unread).
- Every path: `vtable[144h](slot)` (`0077F2D0`), then for each garrison occupant
  `vtable[1CCh]()`, SetPartyRace`(new party, occupant+58h)`, `vtable[144h](slot)`. **The garrison
  guns change side with the building.**
- Not silent: more announcements; D4h (`00CFADD0`, the level message) through `0077C2A0` class 7
  unless game mode 2; the local player's selection refresh when the building is the selected
  unit (`00644A60`); `BSP_WarningManager_ReportCapturePoint`; `0095D3C0`; `0095DE00` rebuilds the
  `MFlag` model ("zaszlo"). No airfield state is touched.
- `006F47F0(unit, a, b)`: `hp += maxHp * ([004C1D10()+2Ch+level*4] * a / b)` clamped to max
  (`+36Ch`/`+370h`), then each part (`IsKindOf(4)`) by `rate[level] * a / [004C1D10()+40h]`,
  replaying `"destroyed"` through `vtable[19Ch]` on a part that comes back to full.

### 80.4 Reach in this process

- **Authored parties.** Every reference row's CommandBuildings are owned: JM05 (three, Allied),
  JM08 (`Headquarter 01`, Japanese), USN01 (CB2), USN13 (CB2, CB4, CBT), USNOS (HQ1, HQ2, CB2),
  LOMP10 (CB4, Allied) (`scene type CommandBuilding` lines, reference s logs).
- **Health.** A building is neutralized only when its health reaches 0. In reference s the most
  damaged is USNOS long's CB2 at 11588 of 12000; no CommandBuilding has a death row in any row.
- **This host kills a CommandBuilding at health 0.** `GameGunneryHost::Impl::kill_unit`
  (the `unit_is_dead` funnel, `game_hosts_gunnery.cpp`) makes no class exception, where the image
  has `vtable[1A8h]` `006F1F80` = `RET` and neutralizes through `006F3270`. No row reaches it
  today.
- **Paratroopers:** none (`summary mission gunnery ordnance ... paratrooper=0`), so arm 3 has no
  reach. Arm 2 needs the landing chain (section 79).
- The missions that need capture: JM05's script reads `GetCapturePercentage` on its three CBs
  (the player's objective, idle in the reference); JM08 fails when `Headquarter 01`'s Party is
  not Japanese (line 451, mtime 2024-07-13), so the Allied invasion must shell it to 0 and then
  land or sail within CaptureRange.

### 80.5 The bind (next packet), with predictions

One switch `kCommandBuildingCaptureBound` (OFF) for: the neutralize at health <= 0 in place of
the kill for class 1Ch (`006F3270` -> `006F2940` -> `006F4D10` slot 9), the fixed-step countdown
(`006F75ED`), the tick's arms 1 and 3 with the progress rule, and the D3h flip (party of the
building and of its garrison occupants, the repair, `+7B0h`/`+7B4h`/`+528h`). Arm 2 goes behind
a second switch with the landing chain. The kill funnel is in cc9-gunnery17's file, so the hook
there is one prepared call.

**Predictions:** exit 0 or 1 on every reference row (no CommandBuilding reaches health 0). The
mechanism is shown by an env-gated diagnostic that sets one building's health to 0 at a chosen
time (for example USN13's CB2 at t = 60 s): the building goes neutral, and when an enemy ship
sits inside CaptureRange for about `CaptureValue / (sum of CapturePower)` one-second ticks (the
decay of 20 applies only while no ship is in range), it flips to that side with its garrison guns.

## 81. The capture bound OFF: neutralize, countdown, tick, flip (packet `cc9_command_building_capture_bind`, cc9-ships21, 2026-09-30)

`kCommandBuildingCaptureBound` (`src/game_hosts_ship_ai.cpp`, committed OFF) over
`bsp/command_building_capture.hpp` (the pure rules of section 80). What it runs:

| step | image | host |
| --- | --- | --- |
| health 0 | `00877B90` -> `vtable[1B0h]` `006F3270` -> `006F2940` -> D3h slot 9 -> `006F4D10` | `GameGunneryHost::Impl::kill_unit` asks `GameShipAiHost::command_building_health_zero_006f3270` first; for a CommandBuilding it neutralizes (party 2, progress 0, `+7B0h` = old party) and the building does not die |
| countdown | `006F755D..006F761C` in `006F7360`, 1.0 s | `command_building_capture_countdown_006f75ed`, at the head of `controller_step` (the image's row 5 runs after the job waves; LABELLED order) |
| arm 1 | ships (IsKindOf 6) within `CaptureRange`, party < 2, the four alive cells | the same; CapturePower from `VehicleClass[type].CapturePower` (default 10); the ramp exclusion `+1188h` never applies (no ramp is ever lowered) |
| arm 3 | the paratrooper list `+7DCh` | empty (no paratroopers exist here) |
| progress | `006F6DEF..006F7334` | `command_building_capture_tick_006f6760`, fallback 20 (LABELLED, multiglobals.lua) |
| flip | D3h `006F4D10`, slot chosen at `006F70E6` | side 0/1 -> that party (LABELLED: each party has a slot record); a tie -> slot 8, stays neutral; repair to full (`006F47F0`, Repair >= 100 at every level); the gunnery row's side refreshed |

Not modelled (records or labels): the announcements, D4h, the flag model, `0090F860` scoring,
the per-pad `006AC4D0`, the parts repair, garrison slot occupants as separate units (the
building's own gun mounts follow its party), the AI host's party lists (built at load; a
flipped building stays in its first group), `+2D8h` (so the retake exemption never applies),
the ctor's `-uniform(0,1)` countdown phase (not drawn, to keep the shared stream).

`CaptureValue` (`+7A4h`) is now parsed from the scene (`game_hosts_scene_contents.cpp`, 1000
when absent) and `GameUnitsHost::set_unit_side_0054` writes the party.

The diagnostic `BSP_CB_FORCE_ZERO=<unit>@<seconds>` (both builds) applies one 1e9 script damage
(`apply_script_damage_0095da00`) to the named building at that mission time.

### 81.1 Predictions (written before any run)

- **Plain reference rows, ON against OFF:** exit 0 or 1 on every row. No CommandBuilding
  reaches health 0 (section 80.4), so `health_zero=0 neutralized=0 countdown_fires=0` on both
  sides; the new summary line is the same on both.
- **USN13, `CB2@60`:** OFF: CB2 gets a death row at t = 60.0 and `kill` follows as for any unit.
  ON: `neutralized (prior party=1) at t=60.0`, no CB2 death row, `countdown_fires` about one per
  second from then on. Progress moves only if a ship of either side is within CaptureRange
  (100 on this installation's CommandBuildings); a Japanese ship there drives it negative and
  flips it back to party 1 after about `CaptureValue / (10 * ships)` seconds; with nobody in
  range it stays 0 and CB2 stays neutral to the end. Everything that shoots at or through CB2
  may move after t = 60 in both builds, since the OFF build removes it.
- **JM08 long (`jm08x`, 36000), `Headquarter 01@500`:** OFF: the HQ dies at 500 s. ON: neutral
  at 500 s. JM08's script (line 451, mtime 2024-07-13) fails the mission when the HQ's Party is
  not Japanese, so the ON run may end the mission early through that check (a party 2 building
  answers the check the same way the OFF build's dead one may not). If the Allied landers or
  escorts come within CaptureRange (the invasion reaches the origin around t = 540 s), the HQ
  flips to party 0.

### 81.2 The pairs, and the decision

**Runs.** OFF `local\s21_off` (`a12a93f60`, no flip, binary `53A2F31276FB`), ON `local\s21_on`
(flip `kCommandBuildingCaptureBound=true`, `B4C6A980BBE7`), and for the flip diagnostic
`local\s21_on2` (`b5696554b`, the CaptureValue override, flipped, `50194FB13C85`). Reference
launch form, lockstep 0.05, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | exit | predicted | what moved |
| --- | --- | --- | --- |
| USN13 | 1 | identical | none; `health_zero=0 neutralized=0 countdown_fires=0` on both |
| USNOS | 1 | identical | none |
| JM05 | 1 | identical | none |
| USN01 | 1 | identical | none |
| LOMP10 | 1 | identical | none |
| JM08 | 1 | identical | none |
| USN04 | 1 | identical | none |

**The authored capture values** (the new census line): USN01 and USN13 `CaptureRange` 100,
`CaptureValue` 10000; USNOS HQ1/HQ2/CB2 100 / 10000; LOMP10 CB4 1100 / 7000; JM05
MainCommandBuilding 0 / 10000, SecondaryCommandBuilding 20 / 60000, RadarStation 10 / 60000;
JM08 `Headquarter 01` 500 / 2000000. At 10 CapturePower per ship per second these take
hundreds to hundreds of thousands of ship-seconds, so in these rows a capture by ships alone is
rare; JM05's small ranges suggest its buildings are meant to be taken by landing (arm 2).

**Diagnostics** (`BSP_CB_FORCE_ZERO`, same binaries):
- **USN13 long, `CB2@20`.** OFF: CB2 dies at t = 19.95 (a death row, `killer=-`). ON:
  `neutralized (prior party=1) at t=20.00`, no CB2 death row, 431 countdown fires (one per
  second to the end). From t = 391.95 two Japanese ships sit inside the 100 range (`s1=20`) and
  the progress runs to -200 by t = 400.95; they leave, and the fallback decay takes it back to 0
  by t = 410.95 (20 D5h updates). No flip (10000 needed). The ON-vs-OFF pair is exit 3 (106 vs 93
  deaths): expected, because OFF removes CB2 at 20 s and everything aimed at it moves.
- **USN13 long, `CB2@20:150` (ON2).** Neutral at 20.00; the same two ships drive the progress to
  -160; `flipped to party=1 side=1 repair=1 at t=398.95`; the countdown stops (380 fires), as the
  building is no longer neutral. The logged `slot` is the side (slot choice not modelled).
- **JM08 long (36000), `Headquarter 01@500`.** OFF: the HQ dies at 499.90. ON: neutral at 500.00,
  1301 countdown fires, no D5h update (no ship within 500 of the HQ to the end, t = 1800), no
  flip; the HQ takes 379 more health-zero hits while neutral. Pair exit 3 (11 vs 20 deaths): with
  the HQ standing, the invasion keeps firing on it and nine props near it (containers, tents,
  two Static Gekko) die from t = 945. `MissionFailedRan=nil` on both: the script's line-451 check
  did not end the mission in this process.

**Decision: ON.** Every plain row is gameplay-identical as predicted, and the mechanism matches
the read (neutralize instead of death, one tick per second, the progress rule including the
fallback decay, the flip and the repair). Open: arm 2 (landed landing ships, with the landing
chain), the slot choice and `0090F860` scoring, what a neutral building's own gun mounts do (not checked; the image's
garrison occupants are separate units), and the AI host's party lists for
a flipped building.

## 82. USNOS's death rise in reference s and `kCaptureGroupValueBound` (packet `cc9_usnos_capture_value_check`, cc9-ships21, 2026-09-30)

Question: reference s's USNOS deaths 18 -> 63 were attributed to `kCaptureGroupValueBound`
(section 70) by the `cgv` leave-one-out. Do the planner handoffs follow the `00A250A0` rule, or is
it a host side effect? Logs: `g17_rs_usnos.log` (s, cgv ON) and `g17_s_cgv_usnos.log` (cgv OFF)
in the cc9-gunnery17 tree. Read only.

- **The planner half follows the rule.** ON: `capture group value bound=1 calls=1872 zero=63`,
  `handoffs=21`, capture-path assignments 603. OFF: `calls=0`, `handoffs=0`, assignments 1083. The
  zero-value groups stop being assigned and are handed on, as section 70 predicted for USN13 and
  USN01. The Attack think's census (`ai group target value`) then scores six handed-off groups
  (MovieCargo, Cargo1, 4, 6, 7, 9) at value 0 against every candidate (HQ1, HQ2,
  `Storage, 09 02`, ...), which is section 70's "0 beats the floor, first populated group wins"
  case.
- **The orders change for the warship groups that stay assigned.** OFF: Ada1, Zao2, Zao3 and
  Ada3 are ordered at HQ2. ON: Ada1 at CB2, and Zao2 and Shimotsuke (party 1) at HQ1,
  `cautious=1`.
- **The extra deaths are blast kills of static objects by three of those ships.** 49 of the 63
  death rows have killer Shimotsuke (25, cat 6), Ada2 (16, cat 4) or Zao1 (8, cat 6), all
  `killer_blast=1`, at ranges of about 1000 to 2800 and from t = 7.65. The victims are containers,
  storage, oil tanks, tents, barracks, hangars, two coastal guns (`coastal_gun_us`, authored party
  1) and static aircraft (`Static dauntless`, `Static Jill`). The OFF run has none of these rows.
- **Reading:** the `00A250A0` rule does what section 70 says. The death rise is a consequence:
  the re-targeted warships fire from new positions, and their blasts reach static objects near
  the bases. The coastal guns are the ships' own party; the other props' party is not in the log.
  So whether the image lets a ship's blast damage its own side's statics is a question for the
  gunnery blast path (cc9-gunnery17), not for the planner. The planner switch stays ON.

## 83. The land state's step bound OFF, and the section 77 pairs (packet `cc9_land_step_host`, cc9-ships21, 2026-09-30)

`kShipAiLandStepBound` (`src/game_hosts_ship_ai.cpp`, committed OFF) runs the `land` state
(vtable `00D21658`) where it was a record:

| slot | image | host |
| --- | --- | --- |
| enter `+4h` | `009E18D0`: `+8h` = 0, `+0Ch` = 1.0f, `+8h` = landing time `+1210h` for a kind-0Ch unit, `+10h` = 0 | the same, from `BuildingPadModel::Lander::landing_time_1210`; the tail `0080E490` / `0092BD70` is a record |
| step `+0Ch` | `009E1950` (`ship_ai_follow_land.cpp`) | `LandStepBinding` over the pad model: `lander_pad_1200`, `006AC220` occupant, `+220h` owner, `brain+0B20h` fallback base, `006F2E60` pick, `006F2FB0` assign, `006AC5D0` (line refresh plus per-call arm), the shared-stream draw for the 3..5 s re-scan |
| `vtable[2Ch]` | `009DAB10` (slot read from the PE: `00D21658+2Ch`) | the navigation states' shared arrival test, as `movetopos` |

Host substitutions (LABELLED): `0082ADC0`'s zone set is `group_for_layer(class+570h)` (the
landing modes' `00417E60` arm does the same); `blk+300h` (brain+308h) has no reader and is a
record; the pose-cache refreshes are records.

### 83.1 Predictions (written before any run)

The land state is only entered through the `land` command that `0074A990` issues (section 77), so
the land step changes nothing unless `kShipAiApproachLandingModesBound` is also on.

- **Pair A, the pending section 77 pair:** `kShipAiApproachLandingModesBound` OFF vs ON (land
  step OFF on both), JM08 36000 plus controls USN13, USNOS and USN04. JM08: 77.2's predictions
  (`mode4_points > 0`, `mode3_points > 0`, `begins` 1..4, exit 3); the landers sit in an unrun
  `land` state after `begins`. Controls exit 0 or 1 (no lander latches).
- **Pair B:** landing modes ON on both sides, land step OFF vs ON, same rows. JM08: `enters` equal
  to `begins`, `steps > 0`, `with_pad > 0` (each lander holds the pad `0074A990` gave it), the
  landers drive toward their pads' approach points and reach `final > 0`; exit 3. Nothing lowers
  a ramp yet (`0074AF20` is the next packet), so `LandedCapturePower` never counts and the HQ is
  not captured. Controls exit 0 or 1.

### 83.2 First pair: no lander ever latches mode 3, and why

Exports of `0d1c01d2b`: `s21_b0` (no flip), `s21_b1` (landing modes ON), `s21_b2` (landing modes
and land step ON). JM08 36000 (recon ON: `attackmove=21`):

- `b1` and `b2`: `mode4_points=5478`, `mode3_points=0`, `begins=0`; the latch census is
  `lander=5478 modes=6961/0/0/0/5478`. Every lander frame is mode 4. `land enters=0`.
- The landers still reach the base and fight there (LST 01 is killed by `Headquarter 01` at
  919.45; LSM 01 by an AA truck at 1157.30), so the distance is not what keeps them out.
- **Cause:** the latch's two landing inputs were never filled. `009F2095` calls `006F2D90`
  (`ECX` = the building, body `006F2D90`-`006F2DDB`, `RET`: true at the first pad of `+794h`
  whose occupant `006AC220` is null) and `009F20A4` FILDs the building's `+7C4h` LandingRange;
  the host left both at 0/false (`ShipAiApproachLatchInputs` defaults), so `inside` was always
  false.
- **Fix (under `kShipAiApproachLandingModesBound`, still OFF):** both inputs from the pad model
  and `command_building_landing_range_07c4`. Section 77's pair and this one are re-run on the
  new commit with the same predictions (83.1).

### 83.3 The pairs on `1c07444b6`, and the decision

Exports `s21_c0` (no flip), `s21_c1` (landing modes ON), `s21_c2` (landing modes and land step
ON). Reference launch form, lockstep 0.05, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | A: c0 vs c1 | B: c1 vs c2 |
| --- | --- | --- |
| JM08 36000 | 3 (deaths 25 -> 29) | 1 |
| USN13 | 1 | 1 |
| USNOS | 1 | 1 |
| USN04 | 1 | 1 |

- **Pair A matches 77.2 up to the begin:** latch `modes=6955/0/0/1081/6001` (mode 3 now
  latched), `mode3_points=1081`, `line_casts=2`, `in_reach=2`, `begins=2` (LST 01 and LST 03),
  `mode4_points=6001`. On the first commit (83.2) the same flip gave 5478 mode-4 points and no
  begin, and still exit 3 (the mode-4 standoff points alone move the landers).
- **Pair B is identical: `land enters=0 steps=0`.** The `land` command `0074A990` issues
  (`0077D600`, `00E08FA0`, ord 22, flags 1) is logged in the command table with `issue=1` but
  `curr=0` for both landers, and a later `director idle tail` row (ord 14) is the current one. So
  the land command never becomes the director's current command, and the state selection never
  picks `land`. **Unread:** why the command path does not make it current (the category-3 `land`
  row in `entity_orders.cpp` may be a plane order the ship director does not take, or the
  script's repeated `NavigatorAttackMove` may override it). That is the next packet.
- **Decision: both switches stay OFF.** `kShipAiApproachLandingModesBound`'s own arms match their
  predictions, but its purpose (a lander in the `land` state) fails downstream, and
  `kShipAiLandStepBound` has no reach until it does. The latch-input fix (83.2) is kept under
  `kShipAiApproachLandingModesBound`.

## 84. Handoff (cc9-ships21, 2026-09-30, at about 75% context)

### Landed on this branch

| section | what | switch |
| --- | --- | --- |
| 80 | the capture tick read whole; corrections to 78 | - |
| 81 | the capture: neutralize at 0 hp, 1 s countdown, arms 1 and 3, progress, flip, repair; `BSP_CB_FORCE_ZERO=<unit>@<s>[:<value>]` diagnostic | `kCommandBuildingCaptureBound` ON |
| 82 | USNOS's 18 -> 63 deaths: planner rule confirmed, blast kills of statics | - |
| 83 | the land state's enter and step host; the latch's landing inputs | `kShipAiLandStepBound` OFF, `kShipAiApproachLandingModesBound` OFF |

### The next packets, in order

1. **Why the `land` command never becomes current** (83.3). Start from the command table rows
   of `local\s21_c1_jm08x.log` in the cc9-ships21 tree (`grep "land at pad"`) and the commands
   host's handling of ord 22 (`entity_orders.cpp` row `{22, 00E08FA0, 00CFB600, "land", 3,
   true}`), then `0077D600`'s path for a ship. Once `land enters > 0` on JM08, re-run pair B
   (`s21_runs.ps1 -V <x> -Only jm08x,usn13,usnos,usn04`).
2. **The ramp** `0074AF20` (the `+11A8h`/`+11ACh` countdown and the CL condition at `0074B07A`),
   then capture arm 2 (`LandedCapturePower`, class `+810h`) in `capture_step` (the
   `kCommandBuildingCaptureBound` block in `game_hosts_ship_ai.cpp`).
3. **What a neutral building's own gun mounts do** (lead's queue item).
4. From 79: the MCargo transports' landing craft; the back-off countdown (65.2).

### Tools (in `J:\PROG\battlestations-pacific-decompile-cc9-ships21\local\`, `s21_` prefix)

`s21_runs.ps1 -V <name> [-Exe tree|<path>] -Only <rows> [-Force '<unit>@<s>[:<value>]']` (rows
include `jm08x`, `usn13l`, `usnosl`); `s21_wait.ps1 -Logs <names>`; `s21_vcall.py <slot>`
(sites that load or call `[reg+slot]`); `s21_vt.py`, `s21_bytes.py`, `s21_rel32.py`,
`s21_consts.py`, `s21_disp.py`, `s21_str.py`, `s21_dump.py` (from s20).

## 85. Why the `land` command never became current (packet `cc9_land_command_current`, cc9-ships22, 2026-09-30)

### 85.1 The cause: the arm cascade rewrote `land` to `attackmove`

The 83.3 command-table rows already said so: both `0074A990 land at pad` rows print the command
name `attackmove` beside ordinal 22 (`LST 03 attackmove 0074A990 land at pad 22 3 1 0 0`). The
name is `row.command`, which `deliver_entity_command` overwrites with the substituted class when
the arm cascade rewrites `EBP`. `slot=0` is the pushed-slot flag: nothing reached `0071E6C0` as
`land`.

The image, `00816E30` (MT_COMMAND's delivery, `vtable[160h]`; for `MLandingShip` too: the
`MLandingShip` vtable `00CFFA30` holds `00816E30` at `+160h` and `0074B570` at `+164h`, read from
the PE):

```
00816E6A  MOV EDI,ECX                 ; the unit; no later write to EDI before the land arm
00816FBE  CMP EBP,00E08FA0 ; land      (00816FC4 JNE 00816FE3)
00816FC6  MOV EDX,[EDI] / MOV EAX,[EDX+5Ch] / PUSH 0Ch / MOV ECX,EDI / CALL EAX  ; IsKindOf(0Ch)
00816FD1  TEST AL,AL / JNE 00817330   ; MLandingShip: keep `land`, the checked tail
00816FD9  MOV EBP,00E08F78            ; anything else: `land` becomes `attackmove`
00816FDE  JMP 00817334
```

`0Ch` is `MLandingShip` (docs/ENTITY_CLASS_IDS.md). The host's `self_is_kind_of_vtable5c`
(`EntityCommandArmsBinding`, `src/game_hosts_commands.cpp`) answered `false` for every unit, a
record, so every lander's `land` became an `attackmove` at the pad's position. That attackmove has
no target entity, so `00836920`'s attackmove arm finds the target gone and raises stage 2, the
queue clears, and the director's idle tail issues `follow` (the ord-14 row). The land state
(`00E08FA0` in the state table) is therefore never selected.

Neither candidate from 83.3 was the cause: the category-3 row is taken (the director has no
per-category refusal for it: `0071D6D0` passes a position-valid category-3 descriptor without a
resolve), and no script attackmove had to override it.

### 85.2 The fix, committed OFF

`kEntityCommandSelfKindBound` (in `src/game_hosts_commands.cpp`): `self_is_kind_of_vtable5c(kind)`
answers `unit_is_kind_of(chain.unit.class_id, kind)` from the recovered class chain. Its only
caller is the land arm, and only `MLandingShip` units answer true, so the switch can reach only a
`land` delivered to a landing ship: `0074A990` (under `kShipAiApproachLandingModesBound`), or a
scripted `PilotLand` on a landing ship (none authored in the rows below).

### 85.3 Predictions (written before any run)

- **Pair C:** landing modes and land step ON on both sides; `kEntityCommandSelfKindBound` OFF vs
  ON. JM08 36000: the two land rows read `land` with `slot=1` and become current (`curr=1`);
  `land enters` = 2 (= `begins`), `steps > 0`, `with_pad > 0`; the landers drive to their pads'
  approach points and at least one reaches `final > 0`. No ramp lowers (`0074AF20` unbound), so no
  capture by landing. Exit 3. USN13, USNOS, USN04: 0 or 1 (no lander begins there).
- **Pair D** (reach check, all three ON vs landing modes OFF): not needed if C matches; the flip
  of the whole chain is decided on C.

### 85.4 Pair C on `09e2deb58`, and the flip

`48c944487` committed the switch OFF; its ON export failed on C4702 (unreachable code after the
`if constexpr` return), fixed in `09e2deb58`. Exports `s22_e0` (landing modes and land step ON)
and `s22_e1` (the same plus `kEntityCommandSelfKindBound`). Reference launch form, lockstep 0.05,
`BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`; the 300-frame smoke of `48c944487` passed.

| row | e0 vs e1 |
| --- | --- |
| JM08 36000 | 3 (deaths 29 -> 33) |
| USN13 | 1 |
| USNOS | 1 |
| USN04 | 1 |

- **The prediction holds on JM08.** Both command rows now read
  `LST 0x land 0074A990 land at pad 22 3 1 1 1` (was `attackmove ... 1 0 0`). `land state
  enters=2 steps=822 with_pad=822 final=772 pad_assigns=0` (was all 0). `begins=2`, `mode3_points`
  1081 on both sides; `mode4_points` 6001 -> 3945 (the landers leave the approach state for `land`).
- **Deaths (per-entity table):** LST 03 dies 871.71 -> 900.60, now to `Headquarter 01` at 597
  (was an AA truck at 1190); LST 01 919.20 -> 974.39, killer range 901 -> 592: both landers now
  sit at their pads under the HQ. Ten statics die only ON (tents, a hangar, a watchtower, two
  static planes, an AA truck, a barracks), six units only OFF (Grayson, Macomb, LSM 02, LST 02,
  a pier, a troop transport). Why the escorts' fights moved is not read (unverified: the landers
  now hold at the pads for about 30-55 s longer and draw the base's fire).
  No ramp lowers (`0074AF20` unbound), so nothing is captured by landing, as predicted.
- **Decision: flipped ON, all three** (`kShipAiApproachLandingModesBound`, `kShipAiLandStepBound`,
  `kEntityCommandSelfKindBound`). Every arm matched its prediction (77.2, 83.1 and 85.3); the
  controls stay gameplay identical on every pair (A, B, C). `kEntityCommandSelfKindBound` alone
  has no reach while `kShipAiApproachLandingModesBound` is OFF (only `0074A990` sends a landing
  ship `land` on these rows), so the three move together. The post-flip JM08 36000 is `s22_e1`.
- **Open:** the landers hold at the final arm until the ramp `0074AF20` is bound (the next packet).

## 86. The landing ship's ramp and capture arm 2 (packet `cc9_landing_ramp_capture`, cc9-ships22, 2026-09-30)

### 86.1 The ramp is `0074AF50`, not `0074AF20`

`0074AF20` is a 0x25-byte forwarder (`0074AF42 RET 0Ch`, INT3 padding) to `0074A160`. The code
section 78.4 read (`0074B07A`, `0074B080`) belongs to the next body. That body starts at
`0074AF50` (SEH prologue), and Ghidra has no function there. `0074AF50` is `MLandingShip`'s
per-frame update: its one reference is the vtable `00CFFA30` slot `0DCh` (`00CFFB0C`, a PE
scan). It is `__thiscall(ship)(float dt)`. It calls the base update `008255B0(dt)` and then gates
on `+5Ch` being set and `+5Dh`/`+60h`/`+5Eh` being clear (`0074AF7B..0074AF9D`).
The ramp latch, `0074AFA3..0074B0AC`:

| site | what |
| --- | --- |
| `0074AFB2`, `0074AFBF` | skipped when `[game+1FE4h]` == 2 or `+1200h` (the pad) is null |
| `0074AFCC..0074AFF3` | `held_before = +11A8h > (clock - 1) - dt`. The x87 sequence is `FLD +11A8h`, `FLD [00F876A4]`, `FLD1`, `FSUB ST1,ST0`, `FLD dt`, `FLD ST0`, `FSUBP ST3`, `FXCH ST3`, `FCOMIP ST2`, `JBE` |
| `0074AFF5..0074B006` | a set `+1011h` stores the clock into `+11A8h` |
| `0074B00E..0074B026` | `held_now = +11A8h > clock - 1` (`FSUBRP ST2`, `FCOMIP ST1`) |
| `0074B028..0074B03A` | `held_now != held_before`: `+11ACh = +11B0h` |
| `0074B03C..0074B06B` | otherwise a positive `+11ACh` counts down by `dt`, and processing continues only once it is `<= 0` (`FLDZ`/`FCOMIP`, `JB`) |
| `0074B06D..0074B0AC` | ramp already down (`+1188h`): skip. Otherwise, when `held_now` (the `CL` at `0074B07A`): `0074A420` (`+1188h = +1189h = 1`, then the ramp node pose) and the 0A6h route (`00749AA0`, `0077C2A0` class 4) |

Evidence for the fields:
- The construct `0074C0CD..0074C108` seeds `+11A4h` = 0, `+11ACh` = 0,
  `+11A8h` = -1e10 (`00CE4ADC`) and `+11B0h` = 2.0 (`00CE3958`).
- A displacement census finds no other writer of `+11B0h`.
- The Lua property pairs at `0074C455..0074C4A8` name `+11A8h` `lastTalaj` ("last ground") and
  `+11ACh` `nyitzarTimer` ("open/close timer").
- `+1011h` is `008255B0`'s one-frame copy of `+1010h`. The only `1` store to `+1010h` is
  `00937878` in the contact callback `009377E0`, on a contact whose kind (`contact+2Ch`) is 8.
  The HUD's ship warning at `006830A5` reads the same latch.

So the ramp lowers once the hull has been in continuous ground contact for 2 s.
`0074B0B0..` (the pad re-request, the ramp animation `+11A4h` over `+1190h`, the unload) is not
covered.

### 86.2 The host

`kLandingShipRampBound` (`src/game_hosts_ship_ai.cpp`, committed OFF). The latch itself is
`bsp::landing_ship_ramp_latch_0074afcc` (`building_pads.cpp`, over new `Lander` fields). It runs
for every landing ship that holds a pad, after the capture tick of the same step (LABELLED
order). The same switch turns on two parts of the capture tick:
- **Arm 2** (`006F6A58..006F6BF8`, verified in the listing): every pad occupant that is
  IsKindOf(0Ch), alive, has its ramp down and has party < 2 adds class `+810h`
  `LandedCapturePower` (an integer ADD at `006F6AF7`). This installation's vehicleclasses.lua
  (mtime 2026-05-10) authors 150 for the US LST (class 41) and 100 for the Higgins.
- **Arm 1's exclusion**: a ramp-down landing ship is skipped there.

**SUBSTITUTION (labelled): the ground contact.** This process has no hull contacts. The host
counts contact when the terrain height (`00903860`) under the bow, the centre or the stern
(plus or minus half of `+9C8h` along the heading) is above the waterline less 3.71 (the LST
hull's min y).

**Found on the way, not fixed:** the landers drive across the island. `BSP_LANDER_DIAG=1` on
JM08 36000 (tree build, landing chain ON) shows LST 01 holding its heading at full throttle in
the land state's final arm (`009E1E3B`), with the ground under it at +3 to +7. It is 890 units
past its pad at t = 967. The image's ship is stopped by the hull-terrain contact itself. The
pad stays held, so under this host arm 2 still counts the lander wherever it has gone.

### 86.3 Predictions (written before any run)

- **Plain rows, OFF vs ON (JM08 36000, JM05, USN13, USNOS, USN04): exit 1.** On JM08,
  `ground_contacts > 0`, `lowers = 2` (LST 03 about t = 837, LST 01 about t = 899: first
  contact plus 2 s) and `landed_capture_adds = 0`. The capture tick runs only for a neutral
  building, and JM08's HQ never reaches 0 hp (sections 80.4 and 81.2). Nothing else reads the
  ramp. USN13, USNOS and USN04 have `lowers = 0`, because no lander begins there (pairs A to
  C). JM05 was not in those pairs: it may lower ramps, which moves nothing unless one of its
  buildings is neutral.
- **Can the landing capture JM08's HQ in the 1800 s window? No.** It needs a neutral HQ first,
  which does not happen. Even neutral at t = 0, 2,000,000 / (2 x 150) is about 6667 s.
- **Diagnostic `BSP_CB_FORCE_ZERO=Headquarter 01@500:3000`, JM08 36000, OFF vs ON.**
  - OFF: neutral at 500 and no flip (as 81.2).
  - ON: from LST 03's lowering (about 837 s), about 150 per second (300 once LST 01 lowers
    too), so progress +150 per tick. The HQ flips to party 0 about 20 ticks later (about
    t = 857), if LST 03 survives with its ramp down.
  - Pair exit 3.

### 86.4 The pairs on `38a9729cf`

Exports `s22_f0` (no flip) and `s22_f1` (`kLandingShipRampBound`). Reference launch form,
lockstep 0.05, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`. The 300-frame smoke passed.

| row | exit | what the ON side logs |
| --- | --- | --- |
| JM08 36000 | 1 | `lowers=2`: LST 03 at t = 836.50 and LST 01 at 898.35 (predicted about 837 and 899); `ground_contacts=2893`; `landed_capture_adds=0`; `landed_capture_power=150` for both |
| JM05 | 1 | `lowers=0`; no lander begins |
| USN13 | 1 | `lowers=0` |
| USNOS | 1 | `lowers=0` |
| USN04 | 1 | `lowers=0` |

- **One prediction was wrong: JM08's HQ does reach 0 hp.** Since section 85's flip, the HQ is
  neutralized at t = 1179.50 on both sides (`health_zero=92`, 621 countdown fires). The two
  landers have died by then: LST 03 at 900.60 and LST 01 at 974.39, both to `Headquarter 01`. So
  arm 2 still adds nothing and no flip happens. The answer on the capture window is unchanged:
  the capture cannot complete within the 1800 s window, because while the HQ is owned the tick
  does not run, and once it is neutral no lander is left.
- **The diagnostic as predicted (`Headquarter 01@500:3000`) cannot show the cycle.** The `s22_g0`
  and `s22_g1` runs are identical (exit 1) with `lowers=0` and `ground_contacts=0`: with the HQ
  neutral from t = 500, no lander takes a pad. That fits the landing modes, which go for an enemy
  building (section 77). The mechanism is not contradicted, but the diagnostic's timing was wrong.

### 86.5 The whole cycle, `Headquarter 01@845:3000` (predictions written before the run)

The HQ is neutralized at t = 845, after LST 03's ramp is down (836.50) and before LST 03 dies
(900.60 on the OFF side).
- **OFF** (`s22_h0`): the HQ is neutral from 845, with no ramp and no arm 2. Ships in
  CaptureRange 500 add 10 each; the landers are not within 500 (their killer ranges are
  592-597). Expect no flip, or a slow one.
- **ON** (`s22_h1`): LST 03 (ramp down, the occupant of pad 7) adds 150 on every 1 s tick from
  the first tick after 845. The progress climbs +150 per tick and passes 3000 about 20 ticks
  later: `flipped to party=0` at about t = 865. The HQ is repaired and becomes Allied. LST 01
  adds 150 more if it lowers its ramp before the flip. Everything after the flip may move.
  Expected pair exit 3.

### 86.6 The cycle, and the flip

`s22_h0` vs `s22_h1` (`Headquarter 01@845:3000`): pair exit 3, deaths 38 -> 65.
- **ON matches 86.5.**
  - LST 03's ramp lowers at 836.50.
  - The HQ is neutralized at 845.00.
  - Every tick from 845.00 logs `s0=150 s1=0 side=0`, and the progress climbs 150 per tick
    (arm 2 alone; `landed_capture_adds=20`).
  - At 863.95 the HQ is `flipped to party=0 side=0 slot=0 repair=1` (predicted about 865).
- **After the flip:**
  - The Japanese take the HQ back down to 0 hp later (`neutralized=2`).
  - LST 01 never lowers its ramp (`lowers=1`) and survives: its death row is OFF-only. Its
    landing is presumably dropped once the HQ is no longer an enemy building (unverified).
  - Everything else after 864 moves (35 deaths only ON, 8 only OFF).
- **OFF:** neutral at 845, no flip, no D5h update.

**Decision: `kLandingShipRampBound` ON.**
- The five plain rows are gameplay identical, as predicted (86.4).
- The latch lowers each ramp 2 s after the first contact.
- Arm 2 adds `LandedCapturePower` per tick and completes a capture as read.
- The one wrong prediction was about the plain JM08 HQ (it now reaches 0 hp at 1179.50). It
  was a fact about the row, not the mechanism, and it changes no outcome, because no lander is
  alive by then.

**Open:**
- The landers cross the island (86.2): the host has no hull-terrain stop, so a lander keeps its
  pad while driving on.
- The `+1189h` byte, and `0074B0B0..` (the ramp animation, the troop unload, the pad
  re-request).
- The 0A6h route (a record).
- The scoring slots.

## 87. Hull-terrain contact (packet `cc9_ship_terrain_contact`, cc9-ships22, 2026-09-30)

### 87.1 What the image does

- **Contact kind 8 is the hull touching ground.** `009377E0` (the contact callback: slot 0 of
  `00D1961C`, with `this` = `controller+20h`) reads `kind = contact+2Ch`. On kind 8 it sets
  `unit+1010h = 1` and returns (`0093786A..0093787F`), before the collision-damage gate.
  `008255B0` rotates the byte into `+1011h` on the next update (`00825824..0082583E`). Two
  readers of `+1011h` are known:
  - the HUD warning at `006830A5`, alert 2, which is `ingame.warning_shallowwater`
    (docs/SHIP_SCREEN_UPDATE.md, row 2 of the alert table);
  - the landing ship's ramp latch at `0074AFF5` (section 86).
- **The callback does nothing to the hull's motion for kind 8:** no damage, no velocity write,
  no stop. The stop belongs to the `Dyn` library's contact phase (`00C5BB5F..00C5C455`, an LCP
  solver: `Dyn::Scene::LCPSolver2Task`, docs/RIGID_BODY_INTEGRATION.md). That phase is unread.
  So the stop is the solver's non-penetration of the hull's convex shapes against the terrain,
  not a game-side rule. No `.text` store of an immediate to a contact's `+2Ch` exists in
  `00C30000..00C5FFFF` (scan-bytes `C7 4x 2C ?? 00 00 00`; the pattern does occur elsewhere,
  e.g. `004D6BCD`). The producer of kind 8 is therefore a register store in the unread phase.
- **How the host moves ships today:** `motion_step_00825f20` runs `00825F20`. It then runs the
  Dyn library's velocity phase `00C41550` and position phase `00C5B1B0`, and no collision or
  contact phase. A hull passes through terrain. The AI's terrain avoidance (`0099F1C0` /
  `009A1420`) is steering only.

### 87.2 The host (committed OFF)

This is a labelled substitution for the unread contact phase, in `GameUnitsHost::Impl::
ship_terrain_contact`, after the position phase:
- The hull box (the class model's convex shapes, `class_hull_box`) gives three keel points:
  bow (`max.z`), middle and stern (`min.z`), all at the box's `min.y`, through the pose.
- A point below the terrain height `00903860` is a contact. It sets `+1010h`, and the latch
  rotates at the next motion step. This runs in both builds (it is also the census).
- `kShipTerrainContactBound` (ON) makes the contact stop the hull. At each contact point, the
  step's horizontal displacement and the linear velocity lose their uphill component (the
  central-difference terrain gradient over 1 unit), so a hull cannot climb onto land and slides
  along a shore. Flat ground above the keel blocks the horizontal motion whole.
- `kLandingShipRampHullContactBound` (ON) points the ramp latch at `+1011h` instead of section
  86's stand-in.

### 87.3 Census: which reference ships cross land today (`s22_k0`, `bcf024627`, switch OFF)

The contact latch runs in both builds, so the OFF runs are the census. Reference launch form.
The rows are the reference set's 3200-frame rows plus USN04 4700 and E2, USN02 9200, and
JM08 36000.

| row | ships in contact | steps | deepest keel penetration (m) and who |
| --- | --- | --- | --- |
| USN04, E2, USN01, USN02, LOMP06, LOMP10, USN12 | 0 | 0 | - |
| JM06 | 1 | 125 | Gato-class Submarine 01, 40.1 |
| JM08 36000 | 11 | 28417 | USTroopTransport 01 357.1, 06 286.8; Gleaves 291.3; LST 03 273.3; Bristol 259.7; LSM 01 242.8; LST 01 232.7; Japanese Patrolboat 01 183.7; USTroopTransport 03 16.2, 05 5.1; LSM 02 4.7 |
| USN13 | 5 | 7721 | Maru42 115.3, Maru43 108.3, Maru16 30.9, Maru46 23.5, Maru27 19.3 |
| BSM01 | 1 | 3000 (every step) | Raleigh 3.5, resting |
| JM05 | 6 | 12313 | Fletcher #2.2 226.6, #2.1 216.3; Clemson #1.2 212.3, #1.1 210.8; both PT Boats 7.9 and 6.4 on every step (resting) |
| USNOS | 2 | 1047 | Gato 132.4, Cargo5 4.5 |
| IJN01 | 8 | 7162 | Oglala 17.3, Curtiss 17.3, Pennsylvania 16.0, Downes 9.1, Iowa 8.7, Whitney 8.4, Helena 8.2, Cassin 3.3 (Pearl Harbor's berths and dry dock) |

Penetrations of 100-360 m are hulls driven deep into hills: in seven of the sixteen rows, ships
cross land today. Resting contacts (BSM01's Raleigh, JM05's PT boats, IJN01's moored ships) are
hulls whose keel sits below the terrain at a berth or in shallows.

**The first rule is revised before any ON run.** As committed, the stop blocked all horizontal
motion over flat ground, which would freeze every resting hull. The revision
(`s22_edit_terrain2.py`) resists a step only at a point whose penetration the step deepened. A
hull resting in contact moves freely on the level or away, and cannot climb.

### 87.4 Predictions for the revised rule (both switches ON vs OFF), written before any ON run

- **Rows with no contact** (USN04, E2, USN01, USN02, LOMP06, LOMP10, USN12): exit 0 or 1.
- **JM08 36000:**
  - LST 01 and LST 03 stop at the beach within about 150 of their pads (the diagnostic's
    `pad_dist` reached 34.5 on the stand-in run) and stay there.
  - Their ramps lower about 2 s after their first contact (near 837 and 899, give or take
    the keel geometry).
  - The troop transports, Gleaves, Bristol and LSM 01 stop at the shoreline instead of
    crossing.
  - Exit 3; the landers' and the escorts' death rows move.
- **JM05, USN13, USNOS, JM06:** the deep crossers are stopped at the shore. Exit 3 is likely.
  A hull pinned against a shore by its own AI stays there (the AI's avoidance is unchanged).
- **BSM01 and IJN01:** the resting hulls keep moving where the step does not deepen them. Exit
  0/1 if none of them climbs; otherwise small moves.

### 87.5 The pairs on `d4638209e`, and the flip

Exports `s22_m0` (no flip) and `s22_m1` (`kShipTerrainContactBound`,
`kLandingShipRampHullContactBound`). Reference launch form.

| row | exit | ON: ships / contact steps / stops |
| --- | --- | --- |
| JM08 36000 | 3 (deaths 33 -> 29) | 14 / 27294 / 15617 |
| USN13 | 3 (death rows identical, 7 unit rows moved) | 5 / 7542 / 3831 |
| USNOS | 3 (2 death rows moved in time) | 2 / 926 / 148 |
| IJN01 | 3 (death rows identical; hits 81 -> 79; controlled Downes moved 196 -> 228) | 9 / 4083 / 2559 |
| JM05 | 1 | 6 / 11154 / 478 |
| JM06 | 1 | 1 / 127 / 127 |
| BSM01 | 1 | 1 / 3000 / 57 |
| USN04, E2, USN01, USN02, LOMP06, LOMP10, USN12 | 1 | 0 |

- **JM08, as predicted.**
  - LST 03 first touches at t = 829.67 at (1067, -3542); it stops there, and its ramp lowers at
    831.95 (latch plus one frame).
  - LST 01 touches at 895.20 at (1198, -3652), and its ramp lowers at 899.00.
  - Neither crosses the island any more. Their deaths move: LST 03 at 878.16 to an AA truck at
    1207, instead of 900.60 to the HQ at 597; LST 01 at 920.05 to the HQ at 810, instead of
    974.39 at 592.
  - The HQ is neutralized at 1415.25 instead of 1179.50. No landed capture (both landers are
    dead by then).
- **USN13's Marus** stay at a keel penetration of 0.6-1.2 m (OFF: 19-115 m); they are held at
  the shore.
- **Correction to 87.3.** Most penetrations of 100 m or more are not ships crossing land alive.
  They are wrecks sinking through the seabed, and diving submarines. The stop is horizontal, so
  those stay deep with the switch ON:
  - JM08's dead landers at about 245 m;
  - JM05's Clemsons and Fletchers at 210-227 m, where the row is identical;
  - USNOS's Gato at 132 m.
  The ships that do cross land alive are:
  - JM08's landers and escorts: the stand-in diagnostic shows LST 01 at y = 0 over ground of
    +3 to +7;
  - USN13's Marus;
  - some of IJN01's berth hulls.
  JM05 was predicted to move and did not. Its contacts are wrecks and its resting PT boats.
- **Decision: both switches ON.**
  - Every no-contact row is gameplay identical.
  - The landers beach and lower their ramps as the image's contact latch would make them.
  - The contact rows move only where a live hull used to climb.
- **Open:**
  - The Dyn contact phase itself; the stop here is a labelled substitution.
  - A sunk hull sinks through the seabed, and a submarine can dive into it: no vertical
    response.
  - The IJN01 dry-dock hulls (Downes, Cassin, Pennsylvania) sit in contact.
  - The navigator's land-collision avoidance setter (ranking #13, `0071C1E0` case 3-6,
    director `+220h..+223h`), next.

### 87.6 The lead's decision, and the ramp's own pair

The image's hull-terrain contact has been read (GUNNERY_OPEN_ITEMS 83):
- terrain friction and restitution are 0;
- every hull convex vertex at or below the terrain height is a contact;
- the solver removes the approach speed along the cell normal and pushes out;
- there is an angular response.
On gentle beaches the image lifts a hull rather than stopping it, and it does not stop a hull
over flat ground. The keel-point stand-in is therefore kept only as a record:
`kShipTerrainContactBound` goes back OFF (`18cf1abe1`), and cc9-gunnery19 binds the image's
contact phase on this section's census rows. `kLandingShipRampHullContactBound` stays ON as the
latch's consumer. Its effect with the stop OFF is paired here.

**Predictions** (written before the run): `s22_n0` (ramp latch on the stand-in) against `s22_n1`
(ramp latch on `+1011h`). The stop is OFF on both sides. Commit `18cf1abe1`.
- **JM08 36000:**
  - LST 03's ramp lowers at about 832 instead of 836.50 (hull contact at 829.67, plus the 2 s
    latch and one frame).
  - LST 01's lowers at about 898-899.
  - The landers still cross the island.
  - The HQ is not neutral while either lander lives, so no capture tick counts them. Exit 1.
- **USN13, USNOS, USN04:** exit 1 (no lander begins).

**Result** (`s22_n0` vs `s22_n1`): all four rows are gameplay identical (exit 1). On JM08 the
ramps lower at 831.95 (LST 03) and 897.15 (LST 01), against the stand-in's 836.50 and 898.35.
`kLandingShipRampHullContactBound` stays ON.

**For the image contact bind (cc9-gunnery19), from ranking #13's read.** On the disable side
only (`008A3C6C..008A3C79`), `NavigatorSetAvoidLandCollision` (`008A3B10`) calls `0092BD00` on
the unit's controller (`0080E490`). `0092BD00` walks the hull body's shapes (`[ctl+2Ch]`,
`00C31DC0`, next at `shape+208h`) and sets each shape's mask `shape+30h` to 0Dh (`00C48020`).
So a script that turns land avoidance off also changes the hull's collision mask. Whether 0Dh
excludes the terrain pair is for the contact-phase reader to settle.

**Status of 87.5 (lead's final decision):** `kShipTerrainContactBound` and
`kLandingShipRampHullContactBound` stay ON as a labelled interim stand-in: `18cf1abe1` is
reverted. cc9-gunnery19's reconstruction of the image's contact solver (GUNNERY_OPEN_ITEMS 83)
will replace the stand-in, and turn it OFF when that solver flips.

## 88. The navigator's avoidance setters (packet `cc9_navigator_avoidance`, ranking #13, cc9-ships22, 2026-09-30)

- **Send side.** `NavigatorSetAvoidLandCollision` (`008A3B10`) and `NavigatorSetTorpedoEvasion`
  (`008A3CD0`) route a 5Ah message through `0077C2A0`, with sub-kind 9 and 7 respectively
  (`00835A40`, `00835940`). This is already reconstructed in `lua_binding_navigator.cpp`.
  `NavigatorSetAvoidShipCollision` (`008A3970`, sub-kind 8) has no host binding.
- **Receive side.** `00721A93` hands the message to the director's `vtable[38h]` = `00835640`.
  Sub-kinds 7, 8 and 9 store `msg+24h != 0` into `+240h`, `+241h` and `+242h`; every other
  sub-kind tail-jumps to `0071C1E0`. The host already has these arms, as
  `GameCommandsHost::apply_director_avoidance_message_00835640`.
- **Readers.** Both bytes are already consulted by bound code:
  - `+240h` by `009DA231` (the torpedo gate);
  - `+242h` by the avoid-zone searchers `009DA6FB..009DA7E2` and the turn clearance `009EFCBD`.
- **Defaults.** The constructor `008366D0` seeds all three bytes to 1 (`008366F4 MOV EBX,1`;
  EBX is callee-saved through the `00836700` and `00836705` calls; stores at
  `00836724..00836730`). So only a `false` changes behaviour.
- **The gap.** `GameScriptOrdersHost::session_route_avoidance_message` counted the message and
  never delivered it. `kNavigatorAvoidanceDeliveryBound` (committed OFF) delivers it at once,
  labelled as a loopback like the other 5Ah senders. A per-order log and a summary line are
  added in both builds.
- **The land setter's disable arm, read.** `0092BD00` on the unit's controller (`0080E490`) sets
  every hull shape's collision mask `shape+30h` to 0Dh (`00C48020`). This is recorded, not
  modelled: it belongs to the contact phase (section 87.6).
- **Where the calls come from.** In this installation (2024-10-29 mtime),
  `scripts/global/commandhelpers.lua`'s `luaEnableNavigator(entity, enable)` sets all three
  avoidances to `enable`, so a script that disables a unit's navigator also turns its land and
  torpedo avoidance off.

### 88.1 Census (`s22_p0`, `86503a897`, OFF) and predictions (written before the ON run)

| row | land orders | torpedo orders | orders with `false` |
| --- | --- | --- | --- |
| USNOS | 86 | 86 | 0 |
| USN13 | 52 | 52 | 0 |
| USN02 | 28 | 28 | 4 (torpedo: DeRuyter, Java, Kortenaer and one more) |
| USN04, E2 | 18 | 18 | 0 |
| USN01 | 14 | 14 | 0 |
| USN12 | 12 | 12 | 0 |
| JM06 | 0 | 12 | 12 (torpedo: the tankers, the hospital ship, the transports) |
| JM05 | 2 | 0 | 2 (land: both PT Boats) |
| JM08 36000, BSM01, LOMP06, LOMP10, IJN01 | 0 | 0 | 0 |

- **Rows with no `false`: exit 0 or 1.** Delivering a `true` into a byte the constructor
  already set to 1 changes nothing.
- **JM06:** twelve merchant hulls stop evading torpedoes (`009DA231` false). If the Japanese
  submarines or torpedo planes attack them, they hold course and take more hits: exit 3.
  Otherwise exit 1.
- **USN02:** the four Allied cruisers and destroyers stop evading torpedoes: exit 3 if they are
  torpedoed in the 9000 frames, otherwise exit 1.
- **JM05:** the two PT boats' avoid-zone searches drop land (`009DA6FB..`, `009EFCBD`). They
  path through land zones and the interim hull stop (87.5) holds them at the shore: exit 1 or 3.

### 88.2 The pairs (`s22_p0` vs `s22_p1`), and the flip

- **Rows with no `false` order: exit 1, as predicted.** USNOS (172 deliveries), USN13 (104),
  USN04, E2, USN01, USN12, JM08 36000, BSM01, LOMP06, LOMP10, IJN01. Death rows identical.
- **USN02: exit 3.** Death rows are identical (one row); 27 unit rows move.
  - DeRuyter, Java and Kortenaer (torpedo evasion off) change course.
  - Kortenaer, the controlled unit, moved 6746.73 -> 6812.68.
  - Damage dealt and taken moves on both sides.
  - The mission ends at 29.75 s on both sides (`Mission.EndMission`, "Game Over").
- **JM06 and JM05: exit 1.** JM06's twelve merchant hulls were not torpedoed within the run;
  JM05's two PT boats with land avoidance off did not move.
- **Decision: `kNavigatorAvoidanceDeliveryBound` ON.** The mechanism matches the read. Only a
  `false` moves anything, and only USN02's does within these rows.
- **Open:** `NavigatorSetAvoidShipCollision` (`008A3970`, sub-kind 8 into `+241h`) has no host
  binding, and the land setter's disable arm (the hull shape mask 0Dh) is recorded only.

## 89. Handoff (cc9-ships22, 2026-09-30, at about 72% context)

### Landed on this branch

| section | what | switch |
| --- | --- | --- |
| 85 | the land arm asks the unit's own IsKindOf(0Ch) | `kEntityCommandSelfKindBound`, `kShipAiApproachLandingModesBound`, `kShipAiLandStepBound` ON |
| 86 | the ramp latch `0074AF50` and capture arm 2 | `kLandingShipRampBound` ON |
| 87 | hull-terrain contact latch and census; the stop as an interim stand-in | `kShipTerrainContactBound` ON (interim), `kLandingShipRampHullContactBound` ON |
| 88 | the navigator avoidance setters delivered | `kNavigatorAvoidanceDeliveryBound` ON |

### The next packets, in order

1. **`NavigatorSetAvoidShipCollision`** (`008A3970`, sub-kind 8, director `+241h`): the Lua
   binding reaches no host code. The receiver arm already exists.
2. **What a neutral building's own gun mounts do** (the lead's queue item 3): section 81 left
   the building's guns following its party.
3. **The MCargo landing craft, and the back-off countdown** (65.2, 79).
4. **`0074B0B0..`, the rest of the landing ship's update:** the pad re-request, the ramp
   animation `+11A4h` over `+1190h`, and the unload.

### Tools (in `J:\PROG\battlestations-pacific-decompile-cc9-ships22\local\`, `s22_` prefix)

- `s22_runs.ps1 -V <name> [-Exe tree|<path>] -Only <rows> [-Force '<unit>@<s>[:<value>]']`
  (rows include `jm08x`, `ijn01`, `usn02`).
- `s22_wait.ps1 -Logs <names>`.
- `BSP_LANDER_DIAG=1` prints each lander's position, ground and ramp state once a second.
- `s22_disp.py`, `s22_vt.py`, `s22_consts.py` (from s21).

## 90. `NavigatorSetAvoidShipCollision` delivered (packet `cc9_navigator_ship_avoidance`, cc9-ships23, 2026-09-30)

- **The binding, read whole.** `008A3970..008A3B0A` (RET at `008A3B09`, INT3 from `008A3B0A`).
  It is the same sequence as `008A3B10`:
  - the entity from argument 0 through `00888AA0` (`008A3A6F`);
  - the boolean from argument 1 (`008A3AA6`);
  - `*(entity+738h)` read after the boolean (`008A3A96`);
  - `008359C0` at `008A3AAE` with ECX = the director.
  There is no disable-side arm (like `008A3CD0`).
- **The sender.** `008359C0` is byte-for-byte `00835940` except `MOV dword [ESP+24h],8` at
  `00835A0A`. So it is a 5Ah message, vtable `00CFD9C4`, routed through `0077C2A0` with flags 7.
- **The receiver** already exists: `00835640` stores sub-kind 8 into `+241h` (`00835668`), as
  `GameCommandsHost::apply_director_avoidance_message_00835640` (`flags.ship`).
- **The readers are all bound.** A displacement sweep of `.text` for byte operands at `+241h`
  (`s23_disp.py 241`) finds:
  - `009EC787` in `009EC770`, the clearance category gate;
  - `009EF37B` in `009EF350`, the traffic pass;
  - `009F106C` in `009F0EA0`, the neighbour refresh;
  - the director's own `00720E30`/`00720E62` (copy), `0072150C` (state message build),
    `00721A03` (state message apply), `008362DD` (property dump) and `0083672A` (constructor).
  The three ship-AI readers read `GameDirectorAvoidance::ship` in `game_hosts_ship_ai.cpp`.
- **Default.** The constructor stores 1 (`0083672A`, EBX = 1 from `008366F4`), so only a
  `false` moves anything.
- **The gap.** The row was not in `kScriptOrderBindings`. `GameMissionLuaHost` recorded it as an
  unimplemented native and nothing reached the director.
- **The change.** The row is now routed in both builds. `kNavigatorShipAvoidanceDeliveryBound`
  (committed OFF) delivers the message at once: the same loopback SUBSTITUTION as section 88. A
  per-order log line (`ship=`) and a summary line are added:
  `summary mission script navigator ship avoidance orders= disables= delivered= bound=`.
- **Correction to section 88.** Section 88 said `luaEnableNavigator` sets all three avoidances;
  it does (`commandhelpers.lua:6894..6896`, this installation, 2024-10-29 mtime). But no
  reference row calls `luaEnableNavigator`. Their land and torpedo orders are direct calls in the
  mission scripts. `s22_p0`/`s22_p1` logs have no `NavigatorSetAvoidShipCollision` line in any
  row, so no reference row calls it.
- **Also fixed:** the comment above `tick_squadron_excluded_009ffeb0` (`game_hosts_ai.cpp`) said
  the carrier arm "was not read". Section 36 read it and section 40 bound it.

### 90.1 Census and predictions (written before the ON run)

Loose scripts in this installation (`s23_census.py`, comments excluded):

| row | script | ship-collision calls |
| --- | --- | --- |
| all 14 reference rows | see `s22_p1_*.log` | 0 |
| USN16 (LOMP "09 Samar") | `usn/LOMP/09_samar.lua:367`, `luaInitMission` | 1 per Taffy carrier, `false` |
| BSM06 | `bsm/bsm_06_holding_lombok.lua:732`, `luaInvasionWaveSpawned` | 1 per spawned transport, `false` |
| BSM02 | `bsm_02_defense_of_the_philippines.lua:941/993/1076`, phase 2 and the landing waves | `false` |

Predictions:
- **Reference rows:** exit 0 or 1 (no call).
- **USN16:** the Taffy escort carriers get `+241h = 0` at mission init.
  - `009EC770` and `009EF350` stop accepting any party (the filter needs the director byte).
  - `009F0EA0` answers 0 for their side filter.
  - So the carriers stop steering around other ships and hold their formation course: exit 3.
    Carrier courses and the ships near them move; deaths may move.
- **BSM06 / BSM02:** exit 3 only if a landing wave or phase 2 spawns within 9000 frames;
  otherwise exit 1 (the order count line moves in neither).

### 90.2 The pairs (`s23_a0` vs `s23_a1`, both from `4080faf71`), and the flip

| row | frames | ship-collision orders | exit | what moved |
| --- | --- | --- | --- | --- |
| USN16 | 3200 | 6, all `false` | 1 | the mechanism lines only |
| USN16 | 9200 | 6, all `false` | 1 | the mechanism lines only |
| BSM06 | 9200 | 0 | 1 | nothing (no landing wave spawned) |
| BSM02 | 9200 | 0 | 1 | nothing (phase 2 not reached); 116 death rows identical |
| USN02 | 9200 | 0 | 1 | nothing; death row identical |

- **USN16:** the six orders are Fanshaw Bay, Saint Lo, White Plains, Kalinin Bay, Kitkun Bay
  and Gambier Bay, from `luaInitMission`. ON delivers all six (`delivered=6`). The mechanism
  matches the read:
  - `ShipAiClearance::neighbour_blocks_sweep_009dd010` (958 calls OFF) no longer runs, because
    `009EC770` accepts no party.
  - Kitkun Bay, the only unit with the zone and neighbour diagnostic lines: `traffic_writes`
    255 -> 0 (`009EF350` accepts no party) and `rudder_gate_open` 8745 -> 9000 of 9000.
- **Why gameplay did not move.** The OFF side's traffic terms were all zero (`traffic_max=0.000`,
  `max_turn=0.000`), and the 255 closed rudder-gate frames did not change a position.
  The unit table (24 rows) is identical, and so is the controlled Fanshaw Bay's distance.
- **Prediction miss (spread, not mechanism).** I predicted exit 3 for USN16. The carriers lose
  their ship avoidance as read, but no other ship came close enough within 450 s for it to
  steer them.
- **Decision: `kNavigatorShipAvoidanceDeliveryBound` ON.** The mechanism matches the read; no row
  moves a death or unit row.
- **Still unexercised:** the BSM06 and BSM02 landing waves (`false` on each spawned transport)
  and the siege multiplayer scripts.
