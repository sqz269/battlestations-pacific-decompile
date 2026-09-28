# Ship AI for units generated after load (packet `cc9_generated_ship_ai_registration`)

Worker cc9-ships3, on main `b83cdbb90`. Ghidra was read-only. Descriptive names are hypotheses.

## 1. The gap

`GameShipAiHost::register_units` runs once, from the mission frame host
(`src/game_hosts_mission_frame.cpp`), and sizes the controllers to the units present then.
USN02's phase 2 generates four ships through `GenerateObject` (00944FD0) at about mission frame
3089 (154 s). They are Nachi (type 293, Myoko), Sazanami (73, Fubuki), Naka (70, Kuma) and
Ushio (73, Fubuki).

**They have no ship AI controller.**
- Their `NavigatorAttackMove` orders are recorded, but no ship AI step runs.
- They sit at their spawn points: (4200, -7500), (4700, -7500), (-4200, -7500) and
  (-4700, -7500).
- **They fire only what their own gun pass finds.** In the head run `local\s3_head_usn02.log`,
  Nachi fires 10 shots, Sazanami 12, Naka 8 and Ushio 9, all with 0 hits. Their nearest targets
  are 3631..4092 m away.

## 2. The image: a generated ship gets its brain on the load-time path

**The brain.** `BSP_Ship_EnsureAiOwners` `00810DD0` (body `00810DD0-00810E8A`):

```
00810DE9  CMP dword [ESI+740h],0 / JNZ 00810DFD
00810DF2  CALL 009F3F20                ; BSP_ShipAi_AllocateBrain (009F3BA0 inside)
00810DF7  MOV [ESI+740h],EAX           ; the outer bot, ship brain at +58h
00810E15  CMP dword [ESI+6DCh],0 / JNZ ; then the 558h gun object (00864580, attach 00864BD0)
```

`00810DD0` sits at vtable `+210h` of the ship vtables. `00CFB948 - 00CFB738 = 210h` for the
cruiser class.

**Its caller** is `00810F60` (`BSP_Unit_InitializeDirectorAndHullDimensions`), which sits at
vtable `+9Ch` of the same vtables (`00CFB7D4 - 00CFB738 = 9Ch`):

```
00810FA0  CALL 008366D0                ; the director, stored at +738h
00811074  MOV EAX,[ESI+0C0h] / TEST / JZ 008110C0   ; the spawn descriptor
0081107E  MOV EAX,[EAX+4h] / CMP EAX,1 / JNZ 008110A0
0081108D  CALL 0095A880                ; kind 1 (scene property bag): (0, 1, 0)
00811092  MOV EAX,[00E188A8] / CMP [EAX+1FE4h],2 / JMP 008110B2
008110A0  CMP EAX,3 / JNZ 008110C0     ; kind 3 (saved Lua table)
008110AB  CMP dword [ECX+1FE4h],2 / JZ 008110C0     ; not a network client
008110B4  MOV EDX,[ESI] / MOV EAX,[EDX+210h] / CALL EAX   ; 00810DD0
```

**Its caller** is `SEntity_InitAll` `00925F20` pass A, `[vt+9Ch]` at `0092604E`
(docs/CONSTRUCT_WORLD.md, "What it does").

**InitAll** runs over the pending-entity list from two kinds of site
(docs/CONSTRUCT_WORLD.md):
- the fixed-step row 12 at `00875EA2`, which is the load-time route;
- `BSP_Game_RunExtraFixedStep` `00874D79`, reached from the Lua `Spawn`, `GenerateObject` and
  `LaunchAirBaseSlot` bindings.

**`GenerateObject` creates from the scene record** (`0046DBE8`,
`BSP_SceneDatabase_CreateEntityByName`). The spawn descriptor is therefore the kind-1 scene
property bag, the same as a loaded ship's (docs/ENTITY_LIFECYCLE_TAILS.md, kind 1).

**So a generated ship reaches `00810DD0` through the same arm as a loaded one.** Its brain,
director and gun object are built during the `GenerateObject` call, before the next world tick.
The brain constructor `009F1160` makes its seven stream-1 draws there.

**The party brain's walk.** The AI coordinator host (`00A32350`) reads the units host live
(`src/game_hosts_units.cpp`, the comment at the coordinator guard), so a generated unit is
already visible to it. The ship AI host is the only per-unit table that was frozen at load.

## 3. The binding

The switch is `kGeneratedShipAiBound` in `src/game_hosts_ship_ai.cpp`, committed OFF.
- **The per-unit body is shared.** `register_units`' per-unit body moved unchanged into
  `register_ship_ai_unit`. It covers the 009E4330 navigation block, the depth and navigation
  inputs, the avoidance request and the think phase.
- **ON: late units are registered.** `controller_step` calls `register_generated_units` first
  on every step. That grows the controllers and rows to the units host's count and registers
  each new unit through `register_ship_ai_unit`. When at least one is a ship, it re-runs
  `seed_brain_draws_009f1160`, which seeds only unseeded controllers. That stands in for
  `009F1160`'s draws at generation.
- **A generated unit that is not a ship keeps no controller work.** Examples are USN01's
  scout squadron and USN04's SpawnNew bombers. The step loop and both fire-target stores skip
  it, as before, because `00810DD0` is a ship-vtable slot. This keeps the packet to ships.
  Whether a generated plane's director update belongs here is a separate question.
- **Labelled substitution: timing.** The image registers during the `GenerateObject` call.
  The host registers at the head of the next ship AI step, which is before any step of the new
  unit, so no controller tick is lost.
- **New log lines when ON:** `ship ai generated unit registered: unit=... ship=0|1` and
  `summary mission ship ai generated units registered=N ships=M`.

## 4. Predictions, written before any ON run

OFF is this tree's build. It should match the head run above, which was a plain run of main
`b83cdbb90` with streams and the death table on.

**USN02 9200/9000, pair_diff exit 3:**
- **The census.** `registered=4 ships=4` at about 154 s, for Nachi, Sazanami, Naka and Ushio.
  No generated non-ships.
- **The four ships steer.** Each enters `attackmove`: Nachi and Sazanami at Houston, Naka and
  Ushio at Exeter (luaMoveToPh2). Each gets a standoff row. Their speed rises above 0, and
  they close from about 5 km toward curve standoffs of about 2 km for the cruisers and about
  1.0..1.5 km for the destroyers. At about 15..18 m/s that takes about 3 minutes, so they
  reach the fight at about 330..380 s.
- **More shots and hits.** Their shots rise above 10/12/8/9 and their hits rise above 0.
  Hit records rise from 797.
- **Houston and Exeter take more fire.**
  - OFF, Houston takes 900 and ends with 6037.
  - OFF, Exeter takes 3859 and ends with 4178.
  - ON, both take more.
  - Exeter's loss is possible late in the run. If it happens, usn_2_java.lua:521 fails the
    mission at that time, where OFF has no mission end.
  - Direction only; the times are not predicted.
- **Deaths move.** OFF has 23. The FinalShips can themselves be sunk, so the direction is not
  predicted.
- **Nothing moves before about 154 s.**

**USN01 3200/3000: identity, exit 0 or 1.** Its GenerateObject is the ScoutDauntless squadron,
a non-ship. Only census lines differ: the registration line, the summary line and the unit hull
input line.

**USN04 4700/4500: identity, exit 0 or 1.** Its creations after load are SpawnNew bomber
squadrons and air-ops launches, all non-ships.

**JM06 3200/3000: identity, exit 0 or 1.** Any generated units are planes (the air-ops deck is
held back for GenerateObject); no ship is generated.

**Helm row, USN02 with `3135 takehelm Houston 1.0 EscapePoint`:**
- It is identical to OFF until about 154 s.
- OFF sinks Houston at 208.26 s by Haguro. ON, the FinalShips are still about 4 km away at
  208 s. Houston's loss time stays near 208 s or comes earlier, never later by more than the
  RNG coupling (docs/SHIP_AI_TAILS.md, the shared stream).

## 5. The pairs, and the flip

- **OFF** is `d7b44375a`, this tree's build. It is gameplay-identical to the head run (`pair_diff`
  exit 1 against `local\s3_head_usn02.log`).
- **ON** is `pair_export --commit d7b44375a --flip kGeneratedShipAiBound=true` into
  `local\gs_on`.
- **Logs:** `local\gs_{off,on}_{usn02,usn01,usn04,jm06,helm}.log`, run with streams and the death
  table on, lockstep 0.05, idle player.

| mission | pair_diff | result |
| --- | --- | --- |
| USN02 9200/9000 | exit 3 | shown below |
| helm `3135 takehelm Houston 1.0 EscapePoint` | exit 3 | shown below |
| USN01 3200/3000 | exit 1 | identical; no generated ship |
| USN04 4700/4500 | exit 1 | identical |
| JM06 3200/3000 | exit 1 | identical |

**USN02, OFF against ON:**

| measure | OFF | ON |
| --- | --- | --- |
| deaths | 23 | 26 |
| hit records | 797 | 847 |
| damage | 44672 | 56440 |
| shots | 1052 | 1117 |
| registered | none | 4 of 4 ships, at mission step 3000 |
| Exeter | survives, took 3859 | sunk at 210.81 s by an Ushio torpedo, took 6309 |
| mission end | none | `EndMission` failure at 212.91 s |
| Houston | survives, took 900 | sunk at 295.95 s by Jintsu, took 2845 |

- **The four ships enter `attackmove` and close.** Their nearest-target ranges fall:

  | ship | OFF | ON |
  | --- | --- | --- |
  | Nachi | 3631 | 2437 |
  | Sazanami | 4076 | 2534 |
  | Naka | 3631 | 3002 |
  | Ushio | 4092 | 3360 |

- **Their standoffs:**

  | ship | first | last |
  | --- | --- | --- |
  | Nachi | 900 | 1719 |
  | Sazanami | 900 | 1450 |
  | Naka | 1500 | 1795 |
  | Ushio | 900 | 900 |

- **Naka and Ushio go to `stop` at about 219 s,** after their target Exeter dies.
- **Nothing moved before phase 2.** The death list is identical to 196.96 s. The first moved
  death is Perth, 203.71 s against 203.86 s, killed by Ushio both times.

**Helm row.**
- Houston is sunk at 208.26 s by Haguro from 1878 m, the same in both runs.
- ON, Exeter is also sunk, at 211.56 s by an Ushio torpedo from 4606 m.
- Both runs fail at 212.91 s.

**Predictions:**
- **Held:**
  - the census, 4 of 4;
  - the four ships steer and close;
  - Houston and Exeter take more fire, and each is lost;
  - the mission failure appears where OFF had none;
  - deaths move;
  - the three identities;
  - Houston's helm-row loss time.
- **Failed on spread:**
  - **Timing.** Exeter's loss came at 210.81 s, not "late in the run". It came from Ushio's
    torpedoes launched on the approach, not from the gun fight at the standoff, which is
    about 3 minutes away.
  - **Gun hits.** The four ships' gun hits stayed 0. Ushio's damage, 6993, is torpedoes.
  - **Naka's shots** fell from 8 to 4 instead of rising.
- **The mechanism matched the image**: registration through the load-time body, then
  `attackmove` at the scripted target.

**Decision: `kGeneratedShipAiBound` is ON.** The predictions failed on spread only, which is
recorded above.

**For reference g:** USN02 now fails in phase 2 at 212.91 s, when Exeter is lost.

## 6. Part 2: the probe length at `009F1D3C`

**The image.** `009F1BC0` forms `nested+11F0h` as follows:

```
009F1D1E  FLD [ECX+500h] / FMUL qword [00CE3DC0]   ; class max speed * 10.0
009F1D37  FLD1 / FSTP [ESP] / CALL 00811A30        ; the turn circle at full helm
009F1D41  FMUL qword [00CE3D78]                    ; * 1.5
009F1D53  FCOMI / JBE ; 009F1D6D MOVSS [EBP+11F0h] ; the larger of the two
```

`nested+11F0h` is the length `009E6640` probes each ring slot with against the avoid zones
(docs/SHIP_AI_RING_SCAN.md).

**The host.**
- It answered `00811A30` here with 0, a record, so the probe was `class+500h * 10`. For a
  cruiser that is about 170 m. With the circle it is roughly 1.5 times the turn circle, often
  above 500 m.
- The same call in `009E6E80`'s tail (`009E6FCB`) was already answered by the units host's
  `unit_class_turn_circle_radius_0082e960`.
- `kApproachTurnRadiusBound`, committed OFF, answers this site the same way.

**Predictions, written before the ON runs:**
- **The unit_turn_radius census flips.** The record becomes concrete on every approach frame.
- **USN02 9200/9000: identity, exit 0 or 1.** The fight is in open water, and OFF records
  `ring probe casts=159600 hits=0` (`local\tr_off_usn02.log`). Hits stay 0, so every slot scores
  1.0 as before.
- **USN13 3200/3000, the islands.**
  - OFF has 34 attackmove ships and `casts=52020 hits=0` (`local\tr_off_usn13.log`).
  - ON, the longer probes reach the island zones: hits rise above 0.
  - Blocked slots then change some ring winners: exit 3, with small moves and no predicted
    direction on deaths.
  - If hits stay 0, the row is identity, and the "hits rise" half of the prediction failed.

**The pairs.**
- **OFF** is this tree's build at `14f1cee4e`.
- **ON** is `pair_export --commit abd2ab61f --flip kApproachTurnRadiusBound=true` into
  `local\tr_on`. `abd2ab61f` only restructures the branch so the ON build has no unreachable
  code.
- **Logs:** `local\tr_{off,on}_{usn02,usn13}.log`.

| mission | pair_diff | ring probe casts / hits | unit_turn_radius |
| --- | --- | --- | --- |
| USN02 9200/9000 | exit 1, identical | 159600 / 0 both | record -> concrete (23830) |
| USN13 3200/3000 | exit 1, identical | 52020 / 0 both | record -> concrete (7696) |

**Predictions:**
- **Held:** the census flip and USN02's identity.
- **Failed:** the USN13 "hits rise" half. The longer probes still find no avoid-zone crossing,
  so the slot scores and the gameplay are unchanged. Why no USN13 probe reaches an island zone
  is not established here. Two things may contribute:
  - the probe length is zero within 20 degrees of the reference bearing (`009E67AA`);
  - each USN13 attackmove ship holds the state for only about 12 s (245 choices).

**Decision: `kApproachTurnRadiusBound` is ON.** The value is the image's, the mechanism matches,
and the failed half was on spread with no gameplay change.

## 7. Squadrons generated after load (packet `cc9_generated_squadron_brain_membership`)

**The host.** `GameAiCoordinatorHost::create_00a32350` runs `build_squadrons` once, at the first
`create_units` (`src/game_hosts_ai.cpp`).
- **A later squadron is never a squadron candidate.** SpawnNew waves, air-ops launches and
  GenerateObject squadrons after load all miss the list.
- **Its planes are seedable as plain units**, but `009FE080` refuses the plane base.
- **A second defect rides on the same growth.** A squadron's candidate index is
  `units.count() + i`. When units are created after the build, every stored squadron index,
  in group members, `group_of_unit` and `last_order`, silently turns into the index of a new
  unit.

**The image admits squadrons live.** Compose phase 3 walks the entity lists hung off
`world+19CCh` on every pass (docs/AI_GROUP_THINK.md; docs/AI_COORDINATOR_TICK.md):

```
00A2E835  MOV ESI,[EDI+8]             ; the node's entity
00A2E838  CMP byte [ESI+5Ch],0 / JE next
00A2E83E  CMP byte [ESI+5Dh],0 / JNE next ; 00A2E844 +60h ; 00A2E84A +5Eh
00A2E850  CMP dword [ESI+16Ch],0 / JNE next    ; not already grouped
00A2E859  CMP [ESI+54h],EBP (2) / JGE next     ; party 0 or 1
00A2E85E  PUSH 5660h / CALL 00BF681B ; 00A2E881 CALL 00A2DFA0   ; a new group seeded on it
00A2E88D  MOV EDI,[EDI+4] / TEST / JNE 00A2E835                ; the next node
```

- The lists are live, and the walk has no load-time snapshot. A squadron entity is therefore
  a candidate on the first pass after it is in the world, whatever created it.
- Its identity is the entity pointer, which does not move when other entities are created.

**The binding.** `kGeneratedSquadronBrainBound`, committed OFF. At the head of each coordinator
fixed step, when the unit count has grown since the last build:
- every stored squadron index is shifted by the growth: group members, `group_of_unit`,
  `last_order` and the seed cursor;
- `seed_squadron_for_unit` runs on each new unit. That is build_squadrons' own per-unit body,
  moved unchanged, so a new flight leader builds its squadron from the registry;
- two ON-only lines report it: `ai squadron generated after load: leader=...` and
  `summary mission ai generated squadrons=N index_shifts=M`.

**Predictions, written before the ON runs.** OFF is this tree's build (main `734ee35a2` plus
the docs-only `9fc5b352a`).

**USN04 4700/4500, exit 3.**
- **What is created after the build.** The build holds one squadron (movieval). Eight SpawnNew
  bomber waves of two, and seven air-ops launches, are created after it.
- **Squadrons.** Generated squadrons rise above 0, and `index_shifts` is above 0 for movieval's
  stored index.
- **Brain orders reach them.** OFF has `squadron_commands=1 member_orders=3`, and ON both rise.
  Groups created rise from 15.
- **Moves.** The brain's orders compete with the script's bomber orders, so torpedo and dive
  releases move. Direction is not predicted. Deaths move, from 40 OFF.

**USN13 3200/3000, exit 3.** OFF has no squadrons (`built=0`). One SpawnNew and twelve launches
come after the build, so generated squadrons rise above 0, squadron commands rise from 0, and
releases and deaths move.

**USN01 3200/3000.**
- ScoutDauntless is generated after the build (log line 15373, after the build at 1686). It
  should be one generated squadron.
- Squadron commands rise from 0 only if the brain orders it. The prediction is exit 1 or 3,
  with any move limited to ScoutDauntless's flight and what it spots.

**USN02 9200/9000: identity, exit 0 or 1.** No squadron exists at load or after, so no growth
touches a squadron index.

**The pairs.**
- **OFF** is this tree's build. **ON** is `pair_export --commit 4376ade2a --flip
  kGeneratedSquadronBrainBound=true` into `local\sq_on`.
- **Logs:** `local\sq_{off,on}_{usn04,usn13,usn01,usn02}.log`.

| mission | pair_diff | generated / index shifts | groups created | squadron commands / member orders |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | exit 3 | 20 / 93 | 15 -> 12 | 1 / 3 -> 4 / 12 |
| USN13 3200/3000 | exit 3 | 24 / 9 | 5 -> 5 | 0 / 0 -> 466 / 1398 |
| USN01 3200/3000 | exit 1, identical | 1 / 0 | 5 -> 5 | 0 / 0 -> 0 / 0 |
| USN02 9200/9000 | exit 1, identical | 0 / 0 | 3 -> 3 | 0 -> 0 |

**USN04, OFF -> ON:**

| measure | OFF | ON |
| --- | --- | --- |
| deaths | 40 | 29 |
| hit records | 644 | 501 |
| shots | 5333 | 3751 |
| torpedo-task releases | 5 of 16 | 10 of 16 |
| dive-bomb-task releases | 1 of 19 | 6 of 19 |
| torpedo drops | 0 | 8 |
| first hit | 92.50 s | 100.85 s |

- **Deaths.** The eleven deaths only OFF has are Japanese planes: nine A6M Zeros, a D3A Val and
  a B5N Kate.
- **Why they survive.** The player's launched fighters (`Lexington-class01_sqn01` and the
  others) are now brain-tasked and no longer meet the raids early.

**USN13, OFF -> ON:**

| measure | OFF | ON |
| --- | --- | --- |
| deaths | 24 | 16 |
| hit records | 720 | 294 |
| shots | 6607 | 2090 |
| first hit | 68.10 s | 97.05 s |

- **Deaths.** The eight deaths only OFF has are `bruh` attackers. The early kills by
  `Yorktown_sqn02` and `Enterprise_sqn01` are gone.
- **What the squadrons now do.** The player-launched squadrons receive repeated brain `moveto`
  orders (`ai_command_tick`), about 20 per squadron.
- **This is the image's rule.** docs/AI_BRAIN_PLAYER_EXEMPTION.md establishes that the image's
  brain has no player exemption on its order path.
- **One labelled host substitution sits on that path.** The host appends to the order ring
  where `0077D600` replaces, with a duplicate filter. That may amplify the churn, and it is
  not separated here.

**Predictions:**
- **Held:**
  - generated squadrons and index shifts above 0 on USN04;
  - brain orders reaching them on USN04 and USN13;
  - releases and deaths moving;
  - USN01 at one generated squadron and exit 1;
  - USN02's identity.
- **Failed on spread:** groups created fell on USN04, 15 -> 12, where I predicted a rise. The
  size of the USN13 moves was not anticipated.

**Decision: `kGeneratedSquadronBrainBound` is ON.** The failure is on spread only. The
admission is the image's live-list rule, and the index shift removes a host defect.
- **Flagged for reference g and the lead:** USN13's damage roughly halves.
- Whether the brain's `moveto` churn on player squadrons matches the image depends on the
  order ring's append-versus-replace substitution. That is a follow-up worth routing.
