# CC13_VERIFY13: the AI planner host (game_hosts_ai.cpp)

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify13 on 2026-10-17 against
base 56f4b923a. It covers `src/game_hosts_ai.cpp`: the coordinator, the group brain, the
capture/defend/attack/sell planners, the command ticks and the target weights. `include/bsp/game_hosts_ai.hpp`
was read with it because it holds the switch comments. No code was changed and no ledger records were added.
Line numbers are against the base. "Image" means battlestationspacific.exe, read with
`bsp.py disasm-raw` and `ghidra bytes`. Items already in earlier passes are not repeated: CC13_VERIFY6 R2
(scene Follow and 0077C8D0), CC13_VERIFY4 R7 (the JoinFormation +188h arm, here `:3016-3020`) and
CC13_VER_13 D3 (capture defenders and +5Dh).

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width or rounding with no decision effect,
  presentation, or RNG draw values only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Liveness: the planner reaches no unit on any harness row

`kAiCoordinatorLoadGateBound` is ON and `kAiCoordinatorCreatedAtLoad` is false (`:1019-1021`).
`GameAiCoordinatorHost::fixed_step` (`:6497-6515`) only counts `coordinator_steps_absent`. The image gate
was re-read from disk bytes:
- `004E17FD CMP [ESI+1FE4h],1 / JE 004E180F`;
- `004E1806 CMP byte [ESI+61Ch],0 / JE 004E183D` (skip the create);
- `004E180F..004E1834`: the slot loop over `+8h`, `+9h` and `+0Ah`;
- `004E1838 CALL 00A32350`.

This matches SHIP_AI_OPEN_ITEMS 150.2 and 150.6. Every mission-tree launch has session word 0 and
`61Ch` 0. cc9's runs agree: `g36_*_usn13l.log` prints `steps_absent=8128 groups_created=0 tick_orders=0`.

So on JM08, USN01, USN04, USN13 and LOMP10:
- no planner, brain or command tick decides anything;
- this file makes no stream-1 draw;
- every planner finding below is **inert**.

The planner would be live only for a hosted session (`1FE4h == 1`, the skirmish/multiplayer lobby) or a
developer mode switch (`61Ch`). The harness models neither.

These parts of the file stay live, because other hosts read them:
- `GameObjectiveSets` (Lua `Objectives_*`; read by the HUD grey arrows, the ship-AI and script-order
  surface-set tests);
- `scene_slot_party` and `local_player_party` (gunnery, HUD, world, Lua, ready);
- the owner-player publication (`ai_set_unit_owner_player` from the ship-AI host).

**Structural point.** The host planner runs effective game mode 8 with session 0 (`game_mode()` `:3325`,
`kSinglePlayerEffectiveGameMode` `:1036`). The image never builds a coordinator in that combination. Where it
does build one (session 1; modes 0-7), parts are stubbed. For example, the mode 4..7 arm of 00A15970 answers
false with a record (`:3643-3647`), and the mode planners' `+30h` bodies are not bound.

## Summary: MISMATCH REAL (all inert on the harness rows; sent to team-lead)

| # | Host | Image | Live? | RNG |
|---|---|---|---|---|
| R1 | `candidate_has_member_in_own_set` `:5790-5793` answers `g->team == own_team` | 00A1CDA2-00A1CDAB `00A2C450(ECX = candidate group, [[..+1Ch]+24h])`, then 00A1CDB9 `FreeAttack_ObjectiveTargetMul` (+1CCh) | hosted sessions only | none |
| R2 | `squadron_ammo_type_stand_in` `:4796-4807`, used by `defend_score_00a243d0` `:4849` | 007EDAD0's ordnance code | hosted sessions only | none |
| R3 | spawn and power-up arms are `record()` only, though bodies exist | 00A25B90, 00A2B400, 00A29B8E, 00A16450, 00A179E0 | hosted sessions only | none |

### R1 The planner never applies the objective-target multiplier

`ai_planner_choose_attack_target` (`src/ai_planners.cpp:244`) asks
`host.candidate_has_member_in_own_set(node, own_team)`. The node is an ENEMY-team group, and this host
answers `g->team == own_team`, so the answer is always false. The multiplier is therefore always 1.0.

Image, 00A1CB80:
- 00A1CD9E-00A1CDAB: `MOV EDX,[ESP+2Ch]; MOV EAX,[EDX+1Ch]; MOV EAX,[EAX+24h]; PUSH EAX; MOV ECX,ESI;
  CALL 00A2C450`. ESI is the candidate group, and the pushed value is the brain's objective-set index.
- 00A1CDB2 `JE`. On true, 00A1CDB9 loads `[record+1CCh]`, which ai_tuning_globals.hpp names
  `FreeAttack_ObjectiveTargetMul`.

00A2C450 is already reconstructed in this file as `group_has_member_in_world_set` (`:5338-5385`). It
calls 008DDF90 over `GameObjectiveSets`, which the mission Lua fills. In a hosted session, an enemy group
holding a unit in the attacking side's objective set should be scored up. The host never does that, so
the side's attack picks change.

### R2 The defend score's squadron ordnance is a stale class stand-in

00A243D0 adds an enemy squadron's weight to the "excluded" term when 007EDAD0 answers 0, 2 or 3 (`:4847-4851`).

The host asks `squadron_ammo_type_stand_in`, labelled "the host has no ordnance reader". That premise is
stale: `units.squadron_ammo_type_007edad0` exists and this file already calls it at `:2814`. The units
host header gives 007EDAD0's codes: 1 bomb, 2 torpedo, 3 depth charge, 4 rocket, 5 paratrooper,
6 kamikaze, 0 nothing.

The stand-in maps the leader's class:
- TorpedoBomber 11h -> 2;
- DiveBomber 12h -> 1;
- LevelBomber 10h -> 5 (the paratrooper code);
- Kamikaze 17h -> 6;
- anything else -> 0.

The excluded weight is therefore wrong in three cases:
- a spent dive or level bomber (image 0, excluded; host 1 or 5, not excluded);
- a rocket-armed fighter (image 4, not excluded; host 0, excluded);
- a depth-charge carrier that is not class 11h.

Loaded torpedo bombers match (2 on both sides).

### R3 Spawn and power-up arms are records, though reconstructions exist

| Image | Host | Body available |
|---|---|---|
| 00A25B90 quick spawn (mode planner with no owned group) | `quick_spawn_named_group` `:5795-5802` | `planner_quick_spawn_00a25b90`, `src/cc13_seg70.cpp` |
| 00A16450 spawn bias | `publish_spawn_bias` `:5803-5806` | not checked |
| 00A2B400 capture spawn arm | `:4706-4715` | not checked |
| 00A29B8E defend spawn tail | `:5133-5140` | not checked |
| 00A179E0 engagement pass (AI power-up use, 008EADA0) | `party_brain_plan_tail` `:5408-5413`, labelled "contract: unread" | `ai_party_brain_engagement_pass_00a179e0`, `src/gameplay_seg69_70_cc13.cpp` (included by this file) |

In an island-capture or skirmish session these spawns are the AI's reinforcements, and none is created.
`kCaptureAvailableResources` (`:4097`) stays 1200 for the same reason.

## Switches committed OFF whose body exists

| Switch | Image rule | Classification |
|---|---|---|
| `kAiTickMovetoRetasksPlaneBound` `:293` | 0099A170's moveto arm re-tasks a plane that the tick's moveto reaches (5ch). 5co turned it OFF for a USN13 stall that only the host's ungated coordinator produced. | MISMATCH (inert); the image rule is ON |
| `kAiGroupPartyChangeRemovalBound` `:1002` | 00A2DB50 removes the member on a SetParty event. OFF, the next 00A2DDE0 eviction removes it instead. | MISMATCH MINOR (timing only; inert) |
| `kAiCoordinatorCreatedAtLoad` `:1021` | the gate's answer | MATCH for every harness row |

## Live parts: GameObjectiveSets (script-visible)

`GameObjectiveSets::add_unit` (`:151-158`) stands in for 008DF2B0, which 008CDD60 Objectives_AddUnit
reaches. It was compared with 008DF2B0's bytes:

| Item | Image (008DF2B0) | Host | Class |
|---|---|---|---|
| O1 | 008DF2FC-008DF2FE: the name walk ends at `JE 008DF3DC`. A missing objective adds nothing; there is no create. | `add_unit` calls `add_objective`, which creates the objective | UNRESOLVED (live only if a script adds a unit to an objective not yet added for that player slot; the Lua binding is game_hosts_lua.cpp, read-only) |
| O2 | 008DF2CD-008DF2DF: a unit whose `+5Dh` or `+5Eh` is set is refused | no liveness gate | MISMATCH MINOR (a sunk unit can enter set 0; the readers ask about live units) |
| O3 | 008DF358 008DEE50 appends an entry with no duplicate test (only the observer pair is deduplicated, 008DF338 / 008DF345) | duplicates are dropped | MISMATCH MINOR (one RemoveUnit takes away a unit that was added twice) |
| O4 | 008DF345 registers the objective as the unit's observer (00694A60) | no observer, so a dead unit stays listed until a status change | UNRESOLVED (the removal callback was not read) |

These match:
- `set_status`: the hidden-objective early return (008DFE6E) behind `kObjectiveKindBound` ON, the state
  store and the unit drop;
- the case-insensitive name compare, which matches 00435C40.

## Planner arithmetic spot-checks

| Site | Image | Host | Class |
|---|---|---|---|
| 00A1CD25-00A1CD4D scoring distance | `FCOMI` against the double 1e-10 at 00CE3820, `JBE` -> 0, else sqrt | `ai_target_scoring_distance` | MATCH |
| 00A1CD53-00A1CD95 range factor | `00419010(near +1D0h, 1.0, far +1D4h, 0.1 [00D7A2F0 = 3DCCCCCD], d)` | `range_interpolation` `:5769-5783`, written inline rather than calling `clamped_interpolate_00419010` | MATCH (the same operations; NaN passes through on both) |
| 00A1CDE5-00A1CE05 score product | `((range x base) x objective) x sticky`, rounded after each multiply | `ai_target_score` = `sticky x own_set x base x range` (`src/ai_planners.cpp`) | MISMATCH MINOR (a different association, so 1 ulp when a factor is not 1.0; it can flip an exact tie at the strict compare) |
| 00A1CEAA-00A1CEB2 best pick | `value > best`, ordered (`JBE` skips NaN); floor `[00D22CC4]` = C97423F0 (-1e6) | `value > best_score` | MATCH |
| 00A2CC2E-00A2CC65 cautious arm | the ship-base test, then 00A10C20 / 00A2C9F0, then `00BD2F40(ECX=1)`; `draw > aggressive` takes CAUTIOUSATTACK (`FCOMIP` / `JBE`) | `ai_attack_arm_00a2cbd0` with the draw only after both gates (`:6103-6127`) | MATCH. Draw values are MINOR: the host's `random_00bd2f10` (`:1583-1588`) is a private LCG, `rng*1664525+1013904223`, unlabelled, while 00BD2F40 is the per-thread generator (00BD2ED0 / 00BA2C20) |
| 00A0CDC1-00A0CDE7, 00A0D093-00A0D0D1 group value | `(+218h x sum + maxes) / +214h`; speed bonus `min(+22Ch x speed, +228h x base)`; minus three penalties; `0 > r` clamps, NaN kept | `compose_attack_value_00a0c650` `:5705-5737` with 0.33 / 5.0 and the bonus 0 | MATCH (authored constants, equal values) |
| 00A010F0 danger cost | `dx*dx + 0*0 + dz*dz` stored to float, `> 1e-10` sqrt; `0 < factor` (`FCOMPI` / `JBE`); `weight x factor + sum` in float; no liveness test on list `+58h` | `tick_danger_cost_00a010f0` `:3135-3166`: the same order, but d2 and the sum are carried in double, and it adds a `unit_active` test | MISMATCH MINOR (1 ulp; the extra gate lets only live units count) |
| 009FFEB0 RTB exclusion | `[sq+3D0h]+C4h`, 007EDAD0 != 0 returns, class 10h/12h/11h, 007F16D0, 0077D600(desc, &rec, 1) | `rtb_exclusion_arm` `:2804-2827` | MATCH (the host adds a null-leader guard) |

## STALE LABELS (comments only, except where noted)

| Host | Says | Current tree |
|---|---|---|
| `:185-187` | the hidden-objective early return "is not applied" | applied under `kObjectiveKindBound` (ON, header `:175`) |
| header `:25-49` | no group command object; "the plane tokens remain a labelled substitution" | `kOrderAttackNoMemberIssueBound` ON returns at `:6194`; commands are held per group |
| `:939-944`, `:1827-1828` | the campaign is mode 0; slots 0 and 4 are admitted | `game_mode()` answers 8 and the unforced 009FFE50 arm runs (`kAiPartyGateUnforcedBound`) |
| `:1325-1327` | PilotFires +C24h "has no reader", so it is passed as false to 007EDA90 | `GameUnitsHost::plane_pilot_fires_0c24` exists (game_hosts_units.hpp:913). **Behaviour:** a kamikaze-led squadron with PilotFires is never excluded (inert) |
| `:2025`, `:2164`, `:2210` | 00A10EC0 and 00A11B80 "not read / not bound" | bound (`kAiAutoMergeLeaveBound`, `transport_moves_00a11b80` at `:2178` and `:2204`); call order checked against 00A154E3 / 00A154EE / 00A154F7 |
| `:2381`, `:5591-5593` | 00A0F859 / 00A0C34E vtable[+18h](1Ch) "unread", so the arm stays off | the same file answers vtable[+18h] with the unit's kind test (`kAiWeightBarrelGatesBound`). The arm zeroes a neutral (side >= 2) command-building attacker; inert |
| `:2386-2389` | the record +18h factors use "unread" tuning offsets, so 1.0 | `group_value_spread` (`:5491`) reads ValueRandomMul +50h/+54h for the same record. A uniform constant would scale every candidate equally, so MINOR |
| `:2397-2401`, `:5355-5363` | Objectives_Add / AddUnit are "UNIMPLEMENTED" and every set is empty | both are bound (game_hosts_lua.cpp:691-749); `:5366` still calls `record()` on a bound lookup |
| `:3989-3991` | an enemy CommandBuilding makes the think return false and run the Siege stand-in | `kAiCaptureTargetPathBound` ON (`:4004-4009`) |
| `:4101-4104`, `:4208-4210`, `:5203` | CaptureRange and plane MaxSpd "have no reader" | `kCaptureAccessorsBound` ON uses `command_building_capture_range_07a0` / `plane_class_max_speed_0188` |
| `:4747-4748` | the SELLING tick "has no host arm" | `kSellingTickBound` ON (`:2137`, `:5205`) |
| `:4916-4918` | the 00A28300 comparator "was not read" | `kDefendThinkImageBound` ON sorts the records (`:5030-5040`; above 32 records the build order still stays, MINOR) |
| `:5839-5841` | "nothing creates a PlaneSquadronGen" | squadrons are built at `:6293-6357` and answered at `:5843` |
| `:6366-6371`, `:1188-1195` | the extra wings "are NOT spawned" | `cc8_plane_squadron_host` spawns them; `seed_squadron_for_unit` says so at `:6297-6305` |

## Other labels checked

These are all MATCH or MINOR, and inert:
- `:1231` the order duplicate filter (`kAiOrderReissueBound` ON reissues);
- `:1491` BigLandingShip (now read);
- `:3141-3146` PartyPresence 2000/4000 (authored values, used as a fallback when the block holds 0);
- `:3210` the formation shape-0 columns;
- `:5486-5490` the spread midpoint 39. LABELLED. The per-entity +-5% is lost, and it scales the target
  side per record, so it can reorder candidates: UNRESOLVED, inert;
- `:6010`, `:6050` the squadron fan-out. Each plane gets its own 0077D600 order in place of the 007F0030
  dispatcher. LABELLED, UNRESOLVED, inert.

`memo_lookup` / `memo_store` (`:653-654`) skip the 00F8A734 memo. 00A038C0, which clears it, is called only
by 00A32460 (the controller destructor) and 00A32500 (a forced-weight insert). So the image keeps a
weight for the whole mission. The key is (attacker, record+10h loadout, target, neutral) and the inputs are
class data, so skipping the memo changes no answer, except for a reused entity pointer: MINOR, inert.

`selling_tick_00a11ff0` reads squadron `+361h` and `+3B0h` as clear (`:5204`), so `returntobase` is
re-sent on every SELLING tick, and with `kAiOrderReissueBound` ON that restarts the command. This is
UNRESOLVED (the meaning of the two flags was not read) and inert.

## Not covered

- `src/ai_planners.cpp`, `ai_command_tick.cpp`, `ai_close_attack_tick.cpp` and `ai_group_think.cpp` bodies,
  apart from the scoring sites above.
- The capture target path body `capture_target_path_00a29fd0` (`:4661-4716`) and its plan builder above it and the defend records path
  (`:5040-5130`), apart from their labels.
- The 008CE510 RemoveUnit path and the observer callback behind O4.
