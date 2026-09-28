# Ship AI and AI command: open items, ranked

Addresses: 00852860 009E873B 009E26C0 009F3670 00417B10 00811940 009DF41A 009DF432 009DF4C5 009DF607 009DC2E0 00A15970 0070E450 00605070 00A179E0 00A1443D 00827F95 009F1BC0 009FFEB0 00778890 00A0F970 0071C1E0 009E1170 00835C70

This file ranks what is still open in the ship-AI and AI-command lane, as
docs/GUNNERY_OPEN_ITEMS.md section 31 does for gunnery and docs/LUA_BINDING_MISSION.md does for the
Lua natives. Each later section is one packet taken from the ranking.

## 1. The first ranking (packet `cc9_ship_ai_open_ranking_1`, cc9-ships7, 2026-09-28)

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
| 2 | **the follower's station point** | `009DF2D0`: the zone set `vtable[218h]` at `009DF41A`, the push `00417B10` at `009DF432` / `009DF4C5`, the leader yaw rate `00811940` at `009DF607` | `00417B10` complete (`avoid_zone_group_offset_00417b10`, bound for the ring probe as `GameAvoidZoneRuntime::offset`); `00811940` reconstructed (`GameUnitsHost::unit_current_yaw_rate_00811940`); `vtable[218h]` = `006DFD90`, which the ring probe binds as `zones.group_for_layer` | `game_hosts_ship_ai.cpp` `FollowFormationPointBinding`: `zone_set_218` answers 0, `push_out_of_zones` returns the point, `leader_yaw_rate` answers 0 | push 105916, yaw rate 52958, zone set 52958, on all nine rows | yaw rate: yes, whenever a leader turns. Push: only near a zone, and JM06's ring probe moved no start (`moved_starts=0`) | 3: the speed blend `009DF5E4..009DF65B` takes the leader's current yaw rate, so a follower of a turning leader keeps too much speed. The push label ("the body is unread") is stale |
| 3 | the free-bearing query | `009DC2E0` (`009DC2E0-009DCEA2`), at `009DF0FA` (the arm final step) and `009EC0C1` (the sector scan) | early outs only (docs/SHIP_NEIGHBOUR_AVOIDANCE.md 6). About 300 pseudocode lines are unread | `game_hosts_ship_ai.cpp`: `ShipAiArmFinal::free_bearing_009dc2e0` and `ShipAiSectorScan::free_bearing_009dc2e0` answer false | 450328. Zone rows: JM06 42457, LOMP06 18410, USN01 10438, USN13 6172, BSM01 6069, JM08 2886. No-zone rows (exact by the early out): E2 161339, USN02 122021, USN04 80536 | only for a ship with avoid-zone segments inside its query box; how often that happens is not counted | 3: it replaces `blk+324h`, the heading target, near land. A binding needs the runtime's segment search (`refresh_search`, `search_segment`, `search_arc`). It is the largest read here |
| 4 | the AI command's avoid-zone point | `00417B10` from the command tick (`ai_command_tick.cpp`) | complete, as rank 2 | `game_hosts_ai.cpp`: `AiCommand::avoid_zone_offset_point` returns the requested point. Label "contract: unread", stale | 1193 (USN13 517, E2 255, JM08 160, BSM01 104, USN04 78, USN01 74) | only for a point inside a zone | 3: the destination the AI command orders. It fits in rank 2's packet as the same routine and runtime |
| 5 | the party brain's replan flag | `00A15970`, from `00A182C0`: outside modes 4 to 7 it returns the OR of `brain+0h..+0Ch` `vtable[30h]()` (`00A159E8..00A15A6A`). For those four planners that is `00A18480`, which reads and clears the replan byte `planner+2Ch` (docs/AI_PLANNERS.md). The claim sets that byte | complete (listing read here) | `game_hosts_ai.cpp`: `AiGroups::brain_wants_immediate_think` answers false. Its comment reads only the mode 4 to 7 arms and says no planner sets a replan request; `planner_claim_group` sets no flag | 38491 (E2 8999, USN02 8999, USN04 4499, the others 2999 or 999) | yes, once after each planner claim: 1 to 3 claims per row (`ai parties claims=`) | 3: in the image a claim makes the party think again on the next call instead of 3 to 5 s later, so the first orders come earlier. Cheap: a flag set at the claim and cleared by the query. Where the claim sets `+2Ch` must be quoted from `00A22750` first |
| 6 | the group's area key | `0070E450` (`0070E450-0070E4B2`) at `009DE5B0` and `009ECA20` | unread (98 bytes) | `game_hosts_ship_ai.cpp`: `ShipAiArmFinal::group_area_key_0070e450` answers the leader's own travel layer, so the "moved" searcher is never chosen | 31576 (E2 8938, USN02 6858, USN01 4470, USN04 4438, JM06 3000, USN13 2886, LOMP06 986) | only for a group whose members sit on different travel layers. Per-entity seeding changes every group, so recount | 3: the path search's layer |
| 7 | the heading wrap after a heading store | `00605070` with ECX = `&blk+1D8h` at `009DFF81`, `009E00FA`, and the setter `009DFFB0`; `&brain+1E0h` at `009F3360` | complete (`BSP_Math_WrapAngleInPlace_Provisional`, `ship_ai_firepower_wrap_angle_00605070`) | `game_hosts_ship_ai.cpp`: `ShipAiControls::after_heading_stored` and `ShipAiApproach::wrap_brain_heading` record and do not wrap. `include/bsp/ship_ai_state_steps.hpp` still says "Body unread" | 154026 (USN13 93957, JM08 27233, BSM01 16734, USN02 7720, USN01 5700); 34126 for the approach | unknown: yes only if a heading outside (-pi, pi] is stored and a reader compares it unwrapped | 0 to 3. The binding is a one-line change; its value depends on a census of `blk+1D8h`'s readers, which was not done |
| 8 | the party brain's engagement pass | `00A179E0` (`00A179E0-00A18195`), from `BSP_AiPartyBrain_Think` | unread | `game_hosts_ai.cpp`: `AiParties::party_brain_plan_tail` records | 491, on all nine rows | unknown | unknown, possibly 3. It builds a vector from `brain+20h` (`008EA0C0`) and snapshots the group lists. Whether it issues orders is not established |
| 9 | the close attack's busy member | `[member+538h]->vtable[2Ch]` at `00A1443D`, in `00A13B60` | unread | `game_hosts_ai.cpp`: `AiCommand::close_controller_busy` answers false | 5991 (BSM01 1500, USN02 1315, JM08 782, JM06 672) | unknown | 3: a busy member is not served by the close-attack pass. The same slot decides the Cargo capture weight (3.0 or 0 in `capture_weight_00a03510`) |
| 10 | the planner candidate's base weight | `00A0F970` | unread | `game_hosts_ai.cpp`: `AiPlanners::candidate_base_weight` weighs by member count | 451 (E2 139, USN02 111, USN04 107, JM06 72, LOMP06 22) | likely | 3: which group the planner picks. Recount after per-entity seeding |
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
    `vtable[5Ch](24h)` and whose `[+3F4h]+80h` is 8, it calls `vtable[1F0h]()`, the immediate fire
    (docs/GUN_SHOT_CADENCE.md 10). Function 8 is in `009542B0`'s depth-charge group 5
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
    unit_index, int function)`. It would call `vtable[1F0h]` (`006FDF60`, the immediate fire) for
    every kind-24h child whose `[+3F4h]+80h` equals `function`, and return whether any fired.
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
