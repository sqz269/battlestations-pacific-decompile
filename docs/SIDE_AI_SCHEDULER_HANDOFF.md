# Handoff: the side-AI scheduler's passes, and the pick screen's squadron members

Packet `cc9_side_ai_handoff`, 2026-09-26, cc9-hud (worktree `battlestations-pacific-decompile-cc9-platform2`),
written at main 3ffcce532 for a fresh worker. Docs only. Background:
- `docs/BOT_SCHEDULER_WRITERS.md`: the gate, the periods and their writers, now ON;
- `docs/BOT_SCHEDULER_OUTPUT.md`: the three callees, read in packet `cc_unit_orders_apply`;
- `docs/GAME_WORLD_ENTITIES.md` section 00914EF0.

## 1. What runs, when, in what order

The object is the mission scoring object at [game+21A0h], 14B8h bytes. It holds eight per-side
records of 284h bytes from +4h (`include/bsp/bot_scheduler_output.hpp`, `kBotSideRecordBase`).
00914EF0 (`bsp::update_bot_scheduler_00914ef0`, `src/world_entities.cpp`) runs once per world tick
from the host's world-tick binding (`src/game_hosts_mission_frame.cpp`,
`update_bot_scheduler_00914ef0`), on `WorldTickState::bots`.

| step | address | cadence (this installation) | gate |
| --- | --- | --- | --- |
| outer gate | 00914F0D | every tick | mode (+1FE4h) 0 or 1, and +1498h |
| update countdown | 00914F36 | +149Ch -= delta; fires every `InGameScoreUpdateTimeInterval` = 1 s | |
| session message 15h | 0075B430 (`retarget_begin`) | once per update | |
| per slot (8) | 00914390 (`slot_prepare`) | once per slot per update | +14A4h, `Scoring_RealPlayTimeRunning(true)` |
| per slot, mode 1 only | 0076A9F0 (`slot_dispatch`) | | mode == 1 (this host: 0) |
| recalc countdown | 00915003 | +1494h -= delta; fires every `ReCalcTimeInterval` = 5 s | |
| per slot (8) | 00911E80 (`think_a`), then 00912A60 (`think_b`) | once per slot per recalc | |
| play time | 00915058 | +14A0h += delta | mode != 2 and +14A4h |

Measured with the gate ON (`local\bs_on_usn04.log`, `local\bs_on_e2.log`):

| row | USN04 4500 | E2 9000 |
| --- | ---: | ---: |
| `retarget_begin` | 225 | 450 |
| `slot_prepare` | 1,800 | 3,600 |
| `think_a` / `think_b` | 360 / 360 | 720 / 720 |
| `slot_dispatch` | 0 | 0 |

All five are host records in `src/game_hosts_mission_frame.cpp`'s `WorldTickBinding`
(`BotScheduler::*`).

## 2. The five records

From `docs/BOT_SCHEDULER_OUTPUT.md`, which is authoritative for the details. **None of the three
callees writes a unit or a controller, and none makes a virtual call.** The layer keeps score,
publishes demand and goal lists, and grants awards. Ship and plane orders come from the per-unit
chain (`docs/UNIT_COMMAND_PRODUCERS.md`), not from here.

| record | image routine | reads | writes | hands out |
| --- | --- | --- | --- | --- |
| `retarget_begin` 0075B430 | `BSP_SessionMessage_ConstructBase(15h)` | - | a stack message | the header of the per-slot message below |
| `slot_prepare` 00914390 | `BSP_BotSideAi_RecomputeAssetScores`, body 00914390..00914EE9 | the record's asset lists (one at +DCh weighted through 00910570 by node+10h) | the six category sums +1C8h..+1DCh and their total +1E0h | nothing outside the record |
| `slot_dispatch` 0076A9F0 | a session-message thunk | record+1E0h, the literal 12h, the slot | - | the only value that leaves the process, in mode 1 (networked) |
| `think_a` 00911E80 | `BSP_BotSideAi_RebuildUnitDemands`, body 00911E80..00912A1E | +1470h (a demand threshold on the scoring object), the config vector `[0050FC30(key)]+48h` | clears and rebuilds the demand list +78h (count +80h) | `0090EDE0(record+250h, key, 1, 0)`, the award grant (`docs/AWARD_GRANT.md`) |
| `think_b` 00912A60 | `BSP_BotSideAi_RebuildGoalRequests`, body 00912A60..00913A0E | the same kind of inputs | clears and rebuilds the goal list +88h (count +8Ch) | 0090EDE0 at 009133C9 and 009139C4 |

**Open questions** (the three follow-ups in `docs/BOT_SCHEDULER_OUTPUT.md`):
- which asset class feeds each of the six sums;
- the demand token table and the config vector gate;
- the class of message 15h and its receive side.

**No reader of the demand and goal lists outside this object has been found.** Whether anything
(Lua, the HUD, the AI planners in `src/game_hosts_ai.cpp`) reads +78h or +88h is the first thing
a successor should settle. It decides whether binding these passes can ever move a unit.

## 3. What is reconstructed, and what binding still needs

`src/bot_scheduler_output.cpp` / `include/bsp/bot_scheduler_output.hpp` have:
- the record layout constants;
- `aggregate_bot_side_scores_00914390`: only the final sum, 00914EB7..00914EC8, in native order;
- `accumulate_weighted_bot_side_term` (00914BD3);
- `clamp_bot_side_term_at_zero_00914ea0`;
- `bot_demand_reaches_threshold`.

**Not reconstructed:**
- the body of 00914390 before the sum (the list walks and the kernels 00910570, 00911C00,
  00911CE0, 00914270);
- 00911E80 and 00912A60 whole;
- 0090EDE0's award grant as reached from here.

**To bind:**
1. A per-side record store in `WorldTickState::bots` (eight records: the asset lists, the six
   sums and the aggregate, the demand and goal lists, +250h the player slot).
2. The asset-list feed. Which units populate each side's lists is not read. It is probably
   0090xxxx or 0091xxxx routines that register units per side.
3. Reconstruct the three bodies, and bind them in `WorldTickBinding` under one switch, OFF first.
4. The award grant 0090EDE0 through whatever the host already has for awards
   (`docs/AWARD_GRANT.md`).

## 4. Files and owners (as of 3ffcce532)

| file | what the binding touches | owner |
| --- | --- | --- |
| `src/game_hosts_mission_frame.cpp`, `include/bsp/game_hosts_mission_frame.hpp` | `WorldTickBinding`'s five `BotScheduler::*` methods; the `kBotSchedulerWritersBound` block | free (released by cc9-hud) |
| `src/game_hosts_lua.cpp`, `include/bsp/game_hosts_lua.hpp` | only if Lua reads the lists | free |
| `src/bot_scheduler_output.cpp`, `include/bsp/bot_scheduler_output.hpp` | the reconstructions | free |
| `src/world_entities.cpp`, `include/bsp/world_entities.hpp` | `BotSchedulerState`, if the records live there | free |
| `src/game_hosts_units.cpp` | only if the asset lists are fed from unit creation | cc9-circle-steer (release-issue stage) |
| `src/game_hosts_ship_ai.cpp`, `src/game_hosts_gunnery.cpp` | none expected | cc9-gunnery2 |

Check `python tools/bsp.py lease list` before claiming; this table is a snapshot.

## 5. Pairs and rows to predict

Take the same-tree pairs one switch at a time: USN04 4700/4500, E2 = USN04 9200/9000, and USN02
9200/9000 if ship sides score differently. Use `BSP_GUNNERY_RNG_STREAMS=1`, `tools/run_game.ps1`
only, and confirm the `1600x900` fit line.
- `BotScheduler::slot_prepare`, `think_a` and `think_b` turn concrete at 1,800 / 3,600 and 360 /
  720.
- New award rows from 0090EDE0.
- The demand and goal list sizes per side. Add a census line.
- **Gameplay.** By the read in section 2, no per-entity row or gunnery line should move. If one
  does, name the reader of +78h, +88h or +1E0h behind it.
- The mode stays 0 in this host, so `slot_dispatch` stays 0.

## 6. Read-ahead: the pick screen's squadron members (00526A40)

29h's pick (`src/hud_warning_screen.cpp`, `bsp::unit_pick_00526a40`) handles list entries this way:
- An entry that passes IsKindOf(18h) (00526F33) is replaced by up to five members:
  `member_3d0(squadron, k)` for `k < member_count_3cc(squadron)`, `k <= 4`.
- Each member must pass IsKindOf(5) (00526EA9) and the other candidate tests.
- The host answers in `src/game_hosts_hud.cpp` (`UnitPickBinding`): `member_count_3cc` returns 0,
  and `member_3d0` records `UnitPickScreen::squadron_members` (00526E58). So a squadron entry
  contributes no candidate today.

What a binding needs:
- **Where members live.** The native squadron is a 414h-byte container: +3CCh the live count,
  +3D0h..+3E0h five plane pointers, written at 007F4B55/007F4B60 (`docs/PLANE_SQUADRON.md`). This
  host fuses the container with its wing-0 leader (`docs/PLANE_SQUADRON_HOST.md` section 1). The
  unit carrying the authored squadron name is wing 0 and also stands for the container. Wings
  1..WingCount-1 are created beside it.
- **The member list in the host.** `resolve_plane_squadron_members` (declared in
  `include/bsp/game_hosts_script_orders.hpp`) resolves the squadron table's +3D0h names to unit
  indices. `GameScriptOrdersHost::resolve_plane_squadron_members` re-runs when the unit count
  moves. The pick binding would read that table: count = the resolved slots, member k = the
  resolved unit index plus 1.
- **The IsKindOf(18h) check.** Because the leader is fused, the question is whether the pick's
  list entries for a squadron are the leader unit, with a plane class, or a class-18h record:
  - The recon aggregates (packet `cc9_recon_aggregates`, ON since 052b23f50) put squadron
    records into the triples. 004C3CB0's walk 0 keeps them in +1970h, and walk 1 files enemy
    ones under +1988h, which merges into +19B8h (`docs/RECON_CALL_SITES.md` section 3).
  - If those records answer IsKindOf(18h) in `bsp::unit_is_kind_of` (chain [18h,2,1,0]), the
    pick expands them. If they are the fused leader with a plane chain, the pick treats them as
    one plane and never reaches the member arm.
  - Check which, with a one-line census in `UnitPickBinding::list_size` of each list entry's
    `unit_class_id`, before binding anything.
- **Files.**
  - `src/game_hosts_hud.cpp` (free once cc9-hud releases);
  - the squadron table's owner (`src/game_hosts_script_orders.cpp`; check the lease list);
  - `src/game_hosts_units.cpp` only if a new accessor is needed there (cc9-circle-steer).
- **Pairs.** USN04 4700/4500 and E2. Predict `UnitPickScreen::squadron_members` turning concrete
  and the grey-arrow and `gui_extent` counts rising by the members per call. `owner_140` and
  `ray_pick_*` stay unless a member lands nearest the screen centre. Nothing moves in gameplay
  unless an enemy member is picked while the weapon group's gunner path runs
  (`docs/HUD_PICK_SEGMENT_QUERY.md` section 5).

## 7. Outcome of packet `cc9_side_ai_scheduler` (cc9-side-ai, 2026-09-26)

Step A settled the open question in section 2: **no reader outside the scoring object turns
`+78h`, `+84h`/`+88h` or `+1E0h` into an order.** The records are per-player-slot
`MissionScoreRecord`s:
- The two think passes evaluate the badges and achievements of achievements.lua.
- 00914390 recomputes the category totals the debrief shows.

The only gameplay reader is Lua `Scoring_GetTotalMissionScore` in competitive multiplayer.
The three records stay host records, and no switch was added. The evidence, the census and
the corrections to sections 1-3 are in `docs/SIDE_AI_SCHEDULER_READERS.md`.
