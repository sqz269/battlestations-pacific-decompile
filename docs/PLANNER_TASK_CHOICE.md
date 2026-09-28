# Which planner commands a group, and what it orders (packet `cc9_planner_close_attack_choice`)

Worker cc9-ships3. Ghidra was read-only. Descriptive names are hypotheses.

## 1. Answer

**The host runs the wrong planners.**
- **Its slot table is permuted.** The host fills a party brain's slots 0..3 with
  Attack, Defend, Capture and Duel. The image fills them with **Defend, Attack, Sell, Capture**.
- **Every host planner runs the same attack-target shape.** Any host planner that owns a
  group runs the Siege/Competitive think: `00A1CB80` target pass, then `00A2CBD0`
  MOVETOATTACK, then CLOSEATTACK within 5000. The image's **Defend** (`00A28A60`) and
  **Capture** (`00A29FD0`) thinks are not reconstructed.
- **A new group is claimed by slot 0 or slot 3** (`ai_planner_for_group`, the image's rule).
  - In the image that is Defend or Capture.
  - In the host it is Attack or Duel, and both run the attack shape.
  - Neither image planner is the one the host runs.

**The two cases:**
- **USN02 at 168.8 s: CLOSEATTACK for Houston's group.** The image would most likely reach
  CLOSEATTACK too, by a different road. Its target choice is not established.
  - With no world-set producer (`summary mission ai world sets ... hits=0`), a combatant group
    goes to slot 3. That is **Capture** in the image.
  - Capture orders its group through `00A1A720` (below). A target that already belongs to an AI
    group gets `00A2CBD0`, which is MOVETOATTACK and then CLOSEATTACK once the leaders are within
    `CloseAttack_CollectDist`.
  - So an attack on the Japanese group is the image's outcome as well. **Which** enemy group is
    decided by the unreconstructed Capture scoring (`00A1E250`, docs/AI_PLANNER_TAILS.md
    section 1), not by `00A1CB80`'s shared pass.
- **USN13: the player's launched squadrons get repeated `moveto`s.** This is the image's.
  - Squadron groups are groupable combatants, so they go to slot 3, Capture.
  - Any Capture outcome other than DEFENDPOSITION (MOVETOATTACK, CAUTIOUSATTACK, PATROLTO) ticks
    through `00A02020`. That issues a `moveto` to the leader on every command tick unless the
    leader is within 80 m (`00A0206E`, `00D21530` = 6400.0 as a double; docs/AI_COMMAND_TICK.md).
    It fans out to the member planes.
  - `00A02020`'s squadron test is `!007EDA90`. That is not a carrier link.
    `BSP_PlaneSquadron_LeaderIsUncommittedKamikaze` (`007EDA90-007EDAB9`) is true only for a
    class-17h leader with `+C24h` clear:

```
007EDA91  MOV ESI,[ECX+3D0h] / TEST / JE false
007EDAA0  PUSH 17h / CALL [vt+5Ch]      ; a kamikaze class
007EDAAA  CMP byte [ESI+0C24h],0 / JNE false
007EDAB3  MOV AL,1
```

  - Player-launched Wildcats and Dauntlesses therefore receive the movetos in the image as well.
    Their **destination** is Capture's choice, not the host's attack pass.

## 2. The image

**The brain's slots** (`BSP_AiPartyBrain_Construct` `00A15A70`, modes 0..3):

```
00A15C31  CALL 00A1EFC0   ; Defend ctor
00A15C44  MOV [ESI],EAX
00A15C61  CALL 00A1F0F0   ; Attack ctor
00A15C74  MOV [ESI+4h],EAX
00A15C92  CALL 00A1F220   ; Sell ctor
00A15CA5  MOV [ESI+8h],EAX
00A15CC3  CALL 00A1F390   ; Capture ctor
00A15CD0  MOV [ESI+0Ch],EAX
00A15ABC  CMP EAX,4 ... 00A15ADA CALL 00A1F500 ; 00A15ADF MOV [ESI+10h],EAX   ; mode 4: Duel
```

So slot i holds kind i in `AiPlannerKind`'s declared order (`src/ai_planners.cpp`'s table,
offsets `00h..1Ch`).

**The claim** (`00A181A0`, `src/ai_group_think.cpp`, matches the image): no groupable combatant,
or a combatant in the brain's world set, goes to `brain+0h`; any other combatant goes to
`brain+0Ch`.

**`00A1A720 BSP_AiPlanner_OrderCaptureGroup`**, complete (docs/AI_PLANNER_TAILS.md 3):
- If `target+16Ch` is set (the target belongs to an AI group), it calls `00A2CBD0(group)(that
  group, party aggressive)`.
- Otherwise, if `00A2C600(group)`, it issues DEFENDPOSITION.
- Otherwise it issues PATROLTO to the target's position.

**The Defend and Capture thinks** (`00A28A60-00A29E2A`, `00A29FD0-00A2B7EB`) are partially read.
Their target lists, scoring and claim loops are what the host lacks.

## 3. The host, term by term

| term | image | host |
| --- | --- | --- |
| slot 0 / 1 / 2 / 3 | Defend / Attack / Sell / Capture | Attack / Defend / Capture / Duel (`planner_kind_for_slot`) |
| mode 4 slot 4 | Duel | Escort |
| group claim | slot 0 or 3 by `00A2C5A0`/`00A2C450` | same rule, same slots |
| owned-group think | per kind: Defend `00A28A60`, Attack `00A1CF90` (`00A1CB80`), Sell `00A22800`, Capture `00A29FD0` | every kind: `ai_mode_planner_tick`, the Siege shape `00A1CB80` |
| spawn when owning none | Capture `[capture]`, Duel `[duel]` and the mode planners | by kind tag; `quick_spawn_named_group` is a record |
| `00A02020` squadron test | `!007EDA90`, the uncommitted-kamikaze leader | same (`tick_squadron_excluded_007eda90`) |

**Only the slot table is bindable here.** The host reads a planner's kind in two places: the
spawn tag, which is a record, and the diagnostic line. The think does not depend on the kind. So
correcting the table changes no gameplay by itself. The thinks are the real divergence, and they
are a reconstruction, not a switch (section 5).

## 4. The binding and predictions

`kAiPlannerSlotKindsBound`, in `src/game_hosts_ai.cpp`, is committed OFF. When ON, slot i holds
kind i.

**Predictions, written before the ON runs:**
- **USN02 9200/9000, USN13 3200/3000, USN04 4700/4500: identity**, exit 0 or 1.
- **Census.** The `ai diag planner kind=` lines change: slots 0..3 read 0, 1, 2, 3 instead of
  1, 0, 3, 4. The quick-spawn record may name a different tag. Nothing else moves.

**The pairs.**
- **OFF** is `80fab5bd4`, this tree's build. **ON** is `pair_export --commit 80fab5bd4 --flip
  kAiPlannerSlotKindsBound=true` into `local\pk_on`.
- **Logs:** `local\pk_{off,on}_{usn02,usn13,usn04}.log`.

| mission | pair_diff | deaths | hit records |
| --- | --- | --- | --- |
| USN02 9200/9000 | exit 1, identical | 9 | 4415 |
| USN13 3200/3000 | exit 1, identical | 16 | 312 |
| USN04 4700/4500 | exit 1, identical | 27 | 501 |

The diagnostic planner lines read `kind=0 1 2 3`. **The prediction held.**

**Decision: `kAiPlannerSlotKindsBound` is ON.**

## 5. Follow-up

**`cc9_planner_defend_capture_thinks`** would reconstruct the Defend think (`00A28A60-00A29E2A`)
and the Capture think (`00A29FD0-00A2B7EB`):
- their target collections;
- `BSP_AiPlanner_CaptureTargetScore` `00A1E250`, which is read;
- the claim loops (`00A1C8B0` inlined);
- the order calls `00A1A720` and `00A1A6D0`.

It would bind them behind a switch in place of the Siege shape for kinds Defend and Capture. That
settles which enemy group Houston's group and the USN13 squadrons are sent at.

## 6. The Capture think, part 1: its targets and the no-target hand-off (packet `cc9_planner_defend_capture_thinks`)

### 6.1 The image

**The targets are CommandBuildings only.** `00A29FD0` builds its target records by walking
`[[00E188A8]+19CCh]+16Ch`, which is world list 28. The only one of the 21 read creators that
registers there is CommandBuilding (docs/UNIT_WORLD_REGISTRATION.md, row CommandBuilding: lists
1, 2, 4, 5, 27, 28).
- Each entity with `+54h != planner+30h` gets a record through `00A22F60`. That record holds
  `00A1E250`'s six floats and the price `max(value * (b + d), minimal * weight)`
  (docs/AI_PLANNER_TAILS.md section 1).
- USN02 (`usn_2_java.scn`) and USN04 (`usn_19_coralus.scn`) contain no CommandBuilding. USN01
  (`usn_1_marshall.scn`) and USN13 (`usn_13_truk.scn`) do, 12 and 36 occurrences (this
  installation's scene files).

**The group records.** For every owned group (planner `+24h`), `00A287C0` inserts a map node
keyed by the group. Its value is:
- `[0]` the group;
- `[1]` `00A2C530()`;
- `[2]` `00A1A7A0(group)`;
- `[3]` the **nearest own-side list-28 entity**, 0 when there is none. It is set at
  `00A2A2xx`..: `local_144[3] = e` and `[4] = d2` when `d2` is below the running minimum;
- `[4]` that squared distance, seeded from `00CE4970`.

**The assignment loop needs both lists.** It is `while (targets != 0 && groups != 0)`, from
`00A2AA..` to `00A2AD77`. It blends `(1 - CBStrategicMul) * w + CBStrategicMul * s` (tuning
`+1BCh`), keeps the best pair, and issues `00A1A720(group, target)` at `00A2AD77`. **With no
enemy CommandBuilding it never runs, and no Capture order is issued.**

**The unassigned groups are handed on.** The listing, with `EBX` the map node and `EBP` = 0:

```
00A2AFC0  CMP dword [EBX+1Ch],EBP      ; value[3], the nearest own list-28 entity
00A2AFC7  JZ  00A2AFED                 ; none -> the brain+4h list, else the brain+8h list
00A2B1C7  MOV EAX,[EDX+24h] / CALL EAX ; planner vtable+24h = 00A1E210, release
00A2B1CF  MOV ECX,[EDI+1Ch]            ; planner+1Ch, the brain
00A2B1D2  MOV EDI,[ECX+4h]             ; brain+4h: Attack
00A2B1E3  CALL 00A1C8B0                ; not already owned -> claim (inlined), 00A2B226 observer
00A2B2AF  MOV ECX,[EDI+1Ch] / 00A2B2B2 MOV EDI,[ECX+8h]   ; the other list: brain+8h
```

- **`00A1E210` has no Ghidra function.** Its body is `00A1E210..00A1E246` inclusive, `RET 4` at
  `00A1E244`, `INT3` at `00A1E247`. It runs: `if 00A1C8B0(group)` then `006952A0` (observer
  unregister), `00A1D1D0` erases the group from `+20h`, and `group+5654h = 0`.
- **With no target the think then returns** (`local_17c == 0` frees and returns) before the
  spawn arm.
- **Attack gets the group next think.** The brain ticks slots 0..3 in order, and Attack (slot 1)
  ticks before Capture (slot 3), so a handed group gets its first Attack order on the next
  think, 3 to 5 s later.

**The Attack think** `00A1CF90` (`00A1CF90-00A1D00F`) runs `00A1CB80` on every owned group, with
the party's aggressive ratio and `reset = 0`:

```
00A1CF94  MOV EAX,[EBX+1Ch] / MOV EAX,[EAX+20h]      ; brain+20h, the party
00A1CFAB  MOVSS XMM0,[ECX*4 + 00F8A8D0]              ; party*1Ch: the aggressive ratio
00A1CFEE  PUSH 0                                     ; resetTarget
00A1CFF7  CALL 00A1CB80                              ; per owned group (00A1CFEB [EDI+8])
```

- The ratio's only writers are the init, `00A32F29`'s reset to 0.5 (`00CE3800`) and
  `AIEnable`'s table branch (docs/AI_GROUP_THINK.md).
- No installed mission script sets `aggressiveRatio`, and the host has no store for it, so 0.5
  applies.

### 6.2 The host, and the binding

**Before this packet**, every host kind ran `ai_mode_planner_tick`:
- it served only the **first** owned group;
- it passed an aggressive factor of 1.0 and the `[00F8A9E0] == 3` reset;
- it ran in the same think as the claim.

**`kAiCaptureThinkBound`, committed OFF.** When ON:
- **The Attack kind runs `attack_think_00a1cf90`**: every owned group, aggressive 0.5, reset 0.
- **The Capture kind runs `capture_think_00a29fd0`.** The enemy-side CommandBuilding test is a
  kind-1Ch unit that is alive and on another side, standing in for list 28, since only
  CommandBuilding registers there.
  - With no such target, every owned group is released and claimed by slot 1 (Attack), or by
    slot 2 when an own-side CommandBuilding exists.
  - With a target it returns false, and **the Siege-shape stand-in still runs**. The target path
    is not reconstructed yet: the `00A1E250` records, the assignment loop, the merge of groups
    assigned to one target (`00A1D010`) and the spawn arm. It is labelled and counted as
    `target_fallbacks`.
- **ON-only census line:** `summary mission ai capture thinks=... target_fallbacks=...
  handoffs=... attack_thinks=...`.

### 6.3 Predictions, written before the ON runs

OFF is this tree's build: the head `96ec4c232` with the landed switches.

**USN02 9200/9000.** On this head the mission fails at 34.70 s, after Houston is torpedoed at
33.80 s. Party 1's group (Haguro's, 12 members) is never claimed: only party 0 thinks.
- Party 0's single group (DeRuyter's, 7 members) is handed from Capture to Attack. Attack orders
  it against **the only enemy group, Haguro's**: MOVETOATTACK, because its first member is a
  ship, then CLOSEATTACK, the same target as OFF.
- The first order comes **one party think later** than OFF.
- Prediction: exit 1 or a small exit 3, with the failure time unchanged or within a few seconds.
- The FinalShips (party 1) receive no planner orders either way.

**USN04 4700/4500, exit 3.**
- Party 0 owns two claimed groups: Lexington's (18 members, MOVETOATTACK) and the
  `Lexington-class01_sqn01` squadron group (4 members, IDLE). OFF orders only the first.
- ON, both go to Attack. The squadron group gets an attack order: CAUTIOUSATTACK or
  MOVETOATTACK, drawn against 0.5.
- `attack_orders` rise from 1, and commands rise.
- Plane deaths and releases move. The direction is predicted as more Japanese planes engaged, so
  Japanese plane deaths rise.

**USN13 3200/3000: identity, exit 1.** Enemy CommandBuildings exist, so every Capture think falls
back (`target_fallbacks` above 0, `handoffs=0`), and Attack owns nothing.

**USN01 3200/3000: identity, exit 1**, for the same reason: its scene has CommandBuildings.

### 6.4 The pairs

- **OFF** is this tree's build at `70d02b3ad`. **ON** is `pair_export --commit 70d02b3ad --flip
  kAiCaptureThinkBound=true` into `local\cp_on`.
- **Logs:** `local\cp_{off,on}_{usn02,usn04,usn13,usn01}.log`.

| mission | pair_diff | capture thinks / fallbacks / handoffs | attack orders OFF -> ON |
| --- | --- | --- | --- |
| USN02 9200/9000 | exit 1, identical (12 deaths, 5166 hit records) | 112 / 0 / 1 | 1 -> 1 |
| USN04 4700/4500 | exit 1, identical (28 deaths, 491 hit records) | 58 / 0 / 2 | 1 -> 2; commands 277 -> 346; member orders 609 -> 819 |
| USN13 3200/3000 | exit 1, identical | 0 / 39 / 0 | 2 -> 2 |
| USN01 3200/3000 | exit 1, identical | 0 / 38 / 0 | 2 -> 2 |

**What moved on USN04.** The squadron group `Lexington-class01_sqn01` ends in CLOSEATTACK
(target group 1) where OFF left it IDLE. Its extra orders change no gameplay number.

**Predictions:**
- **Held:** USN02, USN13 and USN01 are identical. The hand-off and fallback census is as
  predicted. USN04's squadron group now receives an attack order.
- **Failed on spread:** USN04 was predicted to move (exit 3, more Japanese planes engaged) and
  is identical.

**Decision: `kAiCaptureThinkBound` is ON.** The mechanism is the image's, and the failure is
on spread only.

**On this head USN02 fails at 34.70 s,** before any planner order matters. The 168.8 s
question therefore no longer arises on the reference runs. Section 6.1 answers it for the
image: Attack, against Haguro's group, one think after the Capture hand-off.

## 7. Handoff: what is left of `cc9_planner_defend_capture_thinks`

**Done and ON:**
- the Attack think `00A1CF90`;
- the Capture think's no-target path (release, then hand to brain+4h or brain+8h).

**Left, in the order the lead set.**

1. **The Capture target path.** It runs only where an enemy CommandBuilding exists, which on
   the reference set is USN01 and USN13. The fallback counts it: USN13 `target_fallbacks=39`,
   USN01 38. Read, with `local\s3_capture_asm.txt` as the listing and
   `exports/bsp/functions/00a29fd0/decompiled.c` as the pseudocode:
   - **The target records** (`00A2A120..00A2A1E7`). `00A22F60` inserts a node keyed by the
     entity. `00A1E250(rec+4, 0)` fills six floats: `ai_planner_tails.hpp` has the reconstruction.
     `rec+1Ch = max(tuning+19Ch * (b + d), tuning+1A0h * 00A03510())`.
   - **The per-group pass** (decompiled lines 158..405). For each owned group: `00A2C530`
     (value[1]), `00A1A7A0` (value[2]), and the nearest own list-28 entity (value[3]/[4]). For
     each enemy target it computes `00A250A0(group, target)` times an interpolated weight
     (`tuning+1ACh..+1B4h`, 100x inside 3000 falling to 1.5x at 4500). It stores
     `w * f`, and `(1 - tuning+1BCh) * that + tuning+1BCh * rec+4h` into a per-group map
     (`00A22D10`).
   - **The assignment loop** (`00A2AA..00A2AD77`). It takes the best `(group, target)` by the
     blended score above `00D7A218`, calls `00A1A720(group, target)`, subtracts the group's
     value from `target+1Ch` (`00A2ADD7..00A2ADE4`), and erases the group. It loops while both
     lists are non-empty.
   - **The merge** of groups assigned to one target (`00A1D010`, decompiled lines 640..690) and
     `00A1B360`.
   - **The spawn arm** (decompiled 777..880). It is gated by `00946970(brain+20h) <= 0` and the
     spawn-delay ramp, then the resource budget `(1 - [00F8A8BC + mode*4]) * 00942130() -
     00A1C900`. The host's `quick_spawn_named_group` is a record, so this arm can stay a record.
   - **`00A1A720` is complete** (docs/AI_PLANNER_TAILS.md 3):
     - a CommandBuilding's `+16Ch` is normally null, so the usual order is `00A2C600(group)`,
       which gives DEFENDPOSITION or PATROLTO to the building's position;
     - the host has DEFENDPOSITION and PATROLTO command objects (`src/game_hosts_ai.cpp`,
       `issue_defend_position`/`PATROLTO` paths);
     - so USN13's player squadrons would be sent PATROLTO at an enemy CommandBuilding, not
       MOVETOATTACK at a group.
   - **Prediction to carry:** USN13's squadron destinations become enemy CommandBuilding
     positions. `squadron_commands` stays high, since `00A02020` ticks PATROLTO too. Hits on the
     CommandBuildings, not on the raids, would move.
2. **The Defend think** `00A28A60-00A29E2A` (828 decompiled lines). It serves slot 0, which a
   group reaches with no groupable combatant or with a member in the world set. On the reference
   runs that is none: every group is a combatant, and world sets are empty because
   `Objectives_Add` is unbound. It can therefore be bound with an identity prediction until the
   world sets are produced. Its merge pass (`00A29860..00A29BE7`) is already reconstructed in
   `ai_planner_tails`.
3. **Sell** (brain+8h, `00A22800`) receives Capture's groups beside an own CommandBuilding. It
   still runs the Siege stand-in. It is small and should be read before USN13's pair is judged.

**Files and state:**
- Branch `agent/cc9-ships3`. The switches in `src/game_hosts_ai.cpp` are `kAiPlannerSlotKindsBound`
  and `kAiCaptureThinkBound`, both ON.
- The Capture think is `capture_think_00a29fd0`. Its early `return false` on `have_target` is the
  stand-in to replace.
- The lease is `cc9_planner_defend_capture_thinks`, which also covers a new
  `src/ai_planner_thinks.cpp` (not created yet; add it through `cmake/startup.cmake`).

## 8. The Capture target path, part 2 (packet `cc9_planner_defend_capture_thinks`)

Worker cc9-ships4. Ghidra was read-only. The listing is `00A29FD0`'s export (`s3_capture_asm`).

### 8.1 The image

**The target records** (`00A2A130-00A2A1E7`), one per list-28 entity not on the planner's side:

```
00A2A158  MOV EAX,[EDI+54h] / CMP EAX,[EBP+30h] / JZ skip   ; own side: no record
00A2A16D  CALL 00A22F60                                     ; the map node, keyed by the entity
00A2A176  MOV EAX,[EBP+1Ch] / MOV ECX,[EAX+24h]             ; ECX = brain+24h, the planner's side
00A2A184  CALL 00A1E250 (EDX = entity, out = rec+4h) / FSTP ST0
00A2A190  FLD [tuning+19Ch] / FLD [ESI+14h] / FADD [ESI+0Ch] / FMULP   ; 35 * (d + b)
00A2A1A4  CALL 00A03510 (ECX = the entity)                  ; its CaptureWeight
00A2A1B5  FLD [tuning+1A0h] / FMUL double                   ; 200 * CaptureWeight
00A2A1CE  FCOMIP / JBE                                      ; the larger of the two
00A2A1E2  MOVSS [ESI+1Ch],XMM0                              ; rec+1Ch, the price
```

`00A1E250`'s terms are in docs/AI_PLANNER_TAILS.md section 1. With this installation's values:
- **a, b** sum `00A03760` over lists 6 (ships) and 24 (squadrons). Lists 69 and 70 (AirField,
  Shipyard) take `00A03510`'s default arm, weight 0, so they add nothing.
- **`00A03510`** switches on entity `+C4h`: 7 -> 2.0 (`00CE3958`); 8, 0Eh, 18h -> 1.0; 9 -> 3.0
  (`00CE3854`); 0Ah -> 4.0 (`00CE3D34`); 0Dh -> 5.0 (`00CE3850`); 0Ch -> 0.1 (`00D7A2F0`) when
  `00827F70` holds, else 1.0; 0Bh -> 3.0 when `[unit+538h]->vt[+2Ch]()`, else 0; 1Ch -> the Lua
  `CaptureWeight`, default 1.0; anything else 0.
- **c, d** are `00946FC0(team) * 1000 / max(nearest own-team list-28 distance, 1000) / 35`.
  `00946FC0` starts from `[00E0CFB4] * 0.5`. `[00E0CFB4]` is 2400.0 in `.data` and is rewritten
  to 2400.0 (`00CE396C`) at `005E3262` whenever `004BCA50` answers above 3; a campaign mission
  answers 8. It then subtracts every unit's `+304h` `ResourceUsage` (`0077E864`), which no script or
  reference scene of this installation authors, so 0. The target is itself in list 28, so the
  enemy's reach factor is 1: `d = 1200 / 35 = 34.286`.
- **e** is the Lua `StrategicGain`, default 0; nothing authors it.

**The Capture_ tuning** that `00A335D0` stores (the loader writes `+19Ch..+1CCh`; `00A371A0`'s
reader sees each 4 bytes lower). This installation's `highlvlaiglobals.lua` (mtime 2024-07-13),
`IslandCaptureParams_Rookie` lines 94-103:

| reader | key | value |
| --- | --- | --- |
| `+198h` | `Capture_ArriveToRangeTime` | 30 |
| `+19Ch` | `Capture_CapturePointResourceValue` | 35 |
| `+1A0h` | `Capture_MinimalResource` | 200 |
| `+1A4h` | `Capture_CollectDefendersDist` | 3000 |
| `+1ACh` / `+1A8h` | `Capture_ActAttackTargetWeightMul` [1] / [2] | 15.0 / 1.15 |
| `+1B0h` / `+1B4h` | `Capture_ActAttackTargetWeightMulDist` [1] / [2] | 3000 / 4500 |
| `+1B8h` | `Capture_MinimalCBTargetWeight` | 0.5 |
| `+1BCh` | `Capture_CommandBuildingStrategicWeightMul` | 0.8 |
| `+1C0h` / `+1C4h` / `+1C8h` | `Capture_SpawnDelay` [1] / [2], `Capture_SpawnDelayTime` | 60 / 45 / 150 |

The Regular and Veteran blocks differ only in `Capture_SpawnDelay`. The 100x and 1.5x in section
6 were the image defaults; the Lua values are 15x and 1.15x.

**The per-group pass** (`00A2A263-00A2AA90`), one record per owned group:

```
00A2A2C7  CALL 00A2C530 / FSTP [ESI+4h]        ; value[1]: sum of member +304h, 0 here
00A2A2D2  CALL 00A1A7A0 (ECX = planner, group) ; value[2]: the group's current target
00A2A2E9  MOVSS [ESI+10h],[00CE4970]           ; value[4] = 1.0e10, value[3] = 0
00A2A319  CMP EBP,EBX / JZ next                ; entity+16Ch is this group: skipped
00A2A32C  CMP EAX,[EDI+30h] / JZ own           ; own side: the nearest-own test
00A2A380  CALL 00A250A0 (planner; group, entity) / FSTP [ESP+0BCh]
00A2A390  CMP ESI,[ECX+8h] / MOVSS XMM0,[00D7A24C] / JNZ   ; 1.0 unless value[2]
00A2A60A..00A2A641  00419010(+1B0h, +1ACh, +1B4h, +1A8h, d)  ; pushes in reverse order
00A2A65B  CALL 00A22D10 / FLD [ESP+0C4h] / FMUL [ESP+0BCh] / FSTP [EAX]  ; map[entity] = mul * w
```

- **`00A1A7A0`** returns the first list-28 entity not on the planner's side that the group's
  command already serves. That is `00A2BDB0` (an ATTACK whose `+1Ch` is the entity's `+16Ch`),
  or `00A2C150` / `00A2C230` / `00A2C1C0` (IsType 2, 0Ch or 5, with `+8h/+10h` within 1.0 squared of
  the entity).
- **`00A250A0`** is `00A0C650(00A07E40(group), 00A24870(entity), 0, -1.0, 0, 0, 1.0)`.
  - `00A24870` collects, from list 2, the entities with `+5Dh` clear, `+54h == planner+34h`, and
    IsKindOf 6, 18h or 1Bh within `+1A4h` of the target (3-D, `00A1A660`). With none, it takes
    the target itself (`00A248FD`).
  - `planner+34h` is the enemy side: `00A1EEB2 CMP [EAX+24h],EBX / SETZ DL / 00A1EEC1 MOV
    [ESI+34h],EDX`.

**The assignment loop** (`00A2AAA0-00A2AF40`):

```
00A2ABF9  CALL 00A22D10 / MOVSS XMM0,[EAX] / COMISS XMM0,[00D7A218] / JBE skip   ; w > 0.0
00A2AC16  FLD [tuning+1BCh] / FLD1 / FSUBRP / FMUL w / FSTP double               ; (1 - k) w
00A2AC2D  FLD [tuning+1BCh] / FMUL [ESP+0A0h] / FADD double / FSTP float          ; + k * rec+4h
00A2AC4A  FCOMIP / JBE skip                                                       ; beats the best
00A2AD77  CALL 00A1A720 (group, target)
00A2ADD7..00A2ADE4  rec+1Ch -= value[1]; the group leaves the map
```

- The best starts at -1.0e10 (`00CE4ADC`). Targets are the outer loop and groups the inner one,
  both in map (pointer) order, and a strict `>` keeps the first of equal scores.
- A target leaves when its price drops below 0. Every group's `value[1]` is 0 here, so no target
  ever leaves and every group with a positive weight is assigned.

**The merge** (`00A2AF60-00A2B15C`): for each target with more than one group, `00A1D010(earlier,
later)` for every pair (`00A2B129 PUSH ECX (later) / 00A2B12E PUSH EDX (earlier)`). `00A1D010`
merges `later` into `earlier` (`00A2DB80`) only when:
- both are populated and grouping-enabled, on one team, and neither answers `00A2C600`;
- they hold fewer than four members together;
- their leaders are within 1000, 3-D squared against the double `1.0e6` at `00CE4C08`;
- both first members answer IsKindOf(6) alike.

**`PATROLTO`'s tick** (`00A15570-00A156D3`), which the host had no arm for:

```
00A155AE  CALL 009FE080 / JZ tail       ; a groupable leader only
00A15623  FLD [00D22C98] (float 202500.0) / FCOMI / JA   ; far: d2 > 450^2
00A15638  FLD [tuning+1F4h] / FMUL double [00CE3D78] (1.5) / FMUL ST0 / FCOMIP / JA  ; near
00A1565E  CALL 00A10DC0 / CALL 00A11070
00A15683  CALL 00A13B60 (1.0, &+8h, 0, 0) when near; AL = a member found a candidate (00A149F3)
00A15690  CALL 00A11B80(0)
00A156C8  JMP 00A02020 (leader, &+8h) when far and AL == 0
```

### 8.2 The host, and the binding

**`kAiCaptureTargetPathBound`, committed OFF.** When ON, a Capture think that has an enemy
CommandBuilding runs `capture_target_path_00a29fd0`:
- the records, the per-group pass, the loop and `00A1A720` (attack, DEFENDPOSITION, or PATROLTO);
- the merges;
- the hand-off of every unassigned group to brain+4h or brain+8h by `value[3]`;
- the timers and the spawn arm, as a record.

**The PATROLTO arm** is added to `ai_command_tick_vt000c`, with the `00A13B60` call and the leader
tail in the host. The close-attack pass now reports its AL (`candidate_found`). Nothing issues
PATROLTO when the switch is OFF, so the arm is dormant there.

**Labelled substitutions:**
- **`00A250A0`'s value** is members times defenders, a pair count, because `00A0C650`'s pair terms
  are not reconstructed. It is always positive, so every group is assigned.
- **The map order** is unit index for targets and allocation order for groups, not pointer order.
- **CaptureRange** (`+7A0h`) is the `006F2780` default 500. The four reference CommandBuildings
  author 100.
- **A squadron's speed** in `00A03760` is 0, so a squadron counts only inside the radius.
- **Cargo and LandingShip weights:** the Cargo `vt[+2Ch]` arm and the LandingShip `+808h` byte have
  no reader, so Cargo takes 0 and LandingShip 0.1.
- **`+5Dh`** in `00A24870` is read as the unit's active row.
- **`009469F0`** is 0, because this process creates no `[00F89B3C]` records.

**Contracts for the units host** would remove two of these: a CommandBuilding's authored
`CaptureRange`, and the plane class MaxSpd (`+188h`) by unit index.

**Diagnostic.** `BSP_CAPTURE_DIAG=1` prints each Capture think's records and assignments. OFF, it
prints what the path would do and does nothing with it.

### 8.3 Predictions, written before the ON runs

They come from `BSP_CAPTURE_DIAG=1` OFF runs of this tree (`local\s4_plan_{usn13,usn01}.log`).
- **Every target scores `total = 0.5`**, the floor: `a = b = c = 0` and `d = 34.286`.
- **Every price is 1200**, and no price falls.
- **The weight decides.** The score is `0.2 * members * defenders + 0.4`.

**USN13 3200/3000: exit 3.**
- **The records.** The US party's Capture planner sees CB2 (20 defenders), CB4 (14) and CBT (11).
  All three share one Japanese host group, 28 members led by CB2.
- **The first think** (t = 4.20 s) assigns Enterprise's group (51 members) to CB2, scoring 204.4.
  `00A1A720` finds CB2's `+16Ch` set, so it issues `00A2CBD0` at **the CommandBuilding group**. OFF,
  the Siege stand-in orders the same group at **Agano's group** (62 members).
- **The draw.** Aggressive is 0.5 instead of the stand-in's 1.0, so the order is CAUTIOUSATTACK
  when the draw exceeds 0.5. The host ticks no CAUTIOUSATTACK arm, so the group would then get no
  leader movetos.
- **Later thinks keep CB2.** It becomes `value[2]`, so its weight takes the 15x (inside 3000) or
  1.15x factor. `00A2CBD0` then returns early.
- **Census:** `attack=` about 1 per think while the order stands, which is 39 thinks. There is no
  PATROLTO or DEFENDPOSITION, no merge (one group), no hand-off, and `spawn_due` above 0 after the
  60 s ramp.
- **Direction:** hits on the Japanese ships of Agano's group fall. If MOVETOATTACK is drawn,
  hits on land structures (Storage and CB rows) rise, and US plane and ship deaths move.

**USN01 3200/3000: exit 3, from about 91 s.**
- **Until 91.35 s this is identity.** The one owned group is Enterprise's (7 members). OFF already
  orders it at the CB2 group (9 members), and so does ON, with the same single draw at t = 4.20 s.
  The first-order class can still differ: CAUTIOUSATTACK when that draw exceeds 0.5.
- **From 91.35 s the ScoutDauntless squadron group** (1 member, IDLE) is owned too.
  - The stand-in serves only the first owned group, so OFF leaves it IDLE.
  - ON assigns it to CB2 (score 2.0) and orders it at the CB2 group.
  - There is no merge, since 7 + 1 >= 4.
- **Direction:** the ScoutDauntless leaves its idle position (reference g: "controlled moved
  0.00"). Plane deaths or hits on CB2's group may move.

**USN02 9200/9000 and USN04 4700/4500: identity, exit 0 or 1.** Their scenes hold no
CommandBuilding, so the path never runs.

### 8.4 The pairs

- **OFF** is this tree's build at `8fca6d242`. **ON** is `pair_export --commit 8fca6d242 --flip
  kAiCaptureTargetPathBound=true` into `local\ct_on`.
- **Logs:** `local\ct_{off,on}_{usn13,usn01,usn04,usn02}.log`. Every run used
  `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1` without `BSP_CAPTURE_DIAG`.

| mission | pair_diff | deaths | hit records (hull) | ON census |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | exit 3 | 16 -> 17 | 312 (103) -> 283 (117) | 39 thinks, 38 assignments, 38 attack orders |
| USN01 3200/3000 | exit 3 | 5 -> 5 | 177 (74) -> 172 (80) | 38 thinks, 53 assignments, 53 attack orders |
| USN04 4700/4500 | exit 1, identical | 28 | 491 (93) | the path never runs |
| USN02 9200/9000 | exit 1, identical | 12 | 5166 (495) | the path never runs |

In both moved missions the census shows no DEFENDPOSITION or PATROLTO, no merge and no hand-off.

**What moved on USN13.**
- At t = 4.20 s Enterprise's group (51 members) is ordered at the CommandBuilding group (28 members,
  leader CB2) instead of Agano's group. The draw exceeded 0.5, so the order is **CAUTIOUSATTACK**
  (`ai diag order_attack ... target_leader=CB2 cautious=1`).
- The host ticks no CAUTIOUSATTACK arm, so the group's leader gets no movetos:
  - `tick_orders` fell from 514 to 25;
  - the ship-AI plan requests fell from 2886 to 0;
  - Enterprise moved 920 m instead of 2051 m.
- The member orders now reach a building target: `WeaponDirector::attackmove_arm_building_moveto_00836b95`
  appears, UNIMPLEMENTED, 78732 calls.
- One more Japanese plane dies (`bruh #1.1`), and the other 16 death rows shift by seconds.

**What moved on USN01.**
- The first order goes to the same CB2 group as OFF, but its draw exceeded 0.5, so Enterprise's
  group holds CAUTIOUSATTACK from t = 4.20 s instead of MOVETOATTACK.
- The five Mav deaths shift by up to 1.9 s. Hull hits rise by 6.
- From 91.35 s the ScoutDauntless group is assigned to CB2 as predicted, and it also drew
  CAUTIOUSATTACK (`cautious=1`). It therefore stays where it was: "controlled moved 0.00" both ways.

**Predictions:**
- **Held:** every assignment and order target:
  - USN13's switch from Agano's group to the CommandBuilding group at 4.20 s;
  - USN01's second group ordered from 91.35 s;
  - no merge, PATROLTO, DEFENDPOSITION or hand-off;
  - USN02 and USN04 identical.
- **The target choice does not rest on the weight stand-in.** Each mission has one target group:
  USN13's three CommandBuildings share one host group, and USN01 has one CommandBuilding. Any
  positive `00A250A0` value gives the same orders.
- **Failed on spread:**
  - USN01 moved from 4.20 s, not from 91 s. The prediction named this branch: the draw against 0.5
    exceeded it.
  - The ScoutDauntless did not leave its position, because it drew CAUTIOUSATTACK.

**Decision: `kAiCaptureTargetPathBound` is ON.** The mechanism is the image's, and the failures
are on spread.

**The next gap is the CAUTIOUSATTACK tick.** Every 0.5 draw that lands on CAUTIOUSATTACK now leaves a
group without movement. That arm (`vt+0Ch` of the CAUTIOUSATTACK class) is unread, and so is its
`00A14DD0` path shared with CAUTIOUSMOVE. It is the most valuable follow-up for USN13 and USN01.

## 9. Sell, part 3 (packet `cc9_planner_defend_capture_thinks`)

### 9.1 The image

`00A22800 BSP_AiPlanner_SellThink`, body `00A22800-00A228D2`, is the brain+8h think, read in full.

```
00A22841  CALL 00A2E4C0 (ECX = group)          ; split the air members off
00A2284D  CALL 00A22750 (planner)(new group)   ; claim it when one was made
00A22893  CALL 00A2BE10 (ECX = group)          ; command->vt[+8h](0Eh): already SELLING?
00A2289E  CALL 00BF681B (8) / 00A228AD MOV [EAX],00D229B8   ; new SELLING, owner at +4h
00A228BA  CALL 00A2BD00                        ; install it
```

- **`00A2E4C0`** collects the members that `BSP_Entity_IsPlaneOrSquadron` accepts. When there are
  some, but fewer than the population, each leaves the group: `0077BEA0`, its `+16Ch` is cleared,
  and `006956A0` runs. The first air member builds a new group (`BSP_AiGroup_Construct`), and the
  rest join it (`BSP_AiGroup_AddEntity`). An emptied source group is filed on `00F8AA7C`, which
  cannot happen here because some members stay.
- **`00A22750`** claims the new group: it is appended to `+24h` unless `00A1C8B0` finds it there,
  `group+5654h` is set to the planner, the observer is registered, and `planner+2Ch` is set to 1.
  The first walk therefore reaches the new group too, and splits nothing more from it.
- **The SELLING tick** `00A11FF0` was read only to its first block. For a group with no air member
  it finds the nearest own-team list-28 entity (3-D) before it reaches `00A02020`.

### 9.2 The binding

**`kAiSellThinkBound`, committed OFF.** When ON, the Sell kind runs `sell_think_00a22800`, and a
census line `summary mission ai sell thinks= splits= selling=` is printed.

**Labelled:** the host has no SELLING tick arm, so a SELLING group is not moved.

### 9.3 Predictions, written before the ON runs

**USN13 3200/3000 and USN01 3200/3000: identity, exit 0 or 1.**
- Brain+8h receives groups only from Capture's hand-off, beside an own CommandBuilding.
- Only party 0 thinks on these missions: `ai party 1 ... ai_enabled=0 brain=0`.
- Party 0 owns no CommandBuilding: all four are Japanese.
- The Sell planner therefore owns nothing, and the census prints `thinks=` at the tick count with
  `splits=0 selling=0`. The Siege stand-in it replaces also ran with no owned group.

**Which mission would exercise it.** It needs a thinking party with its own CommandBuilding, whose
Capture planner hands a group on. Of this installation's scenes, `COTP-USN/def_aleutians.scn`
holds an Allied CommandBuilding next to a Japanese one. `COTP-IJN/ijn_08_defend_guadalcanal.scn`
and `ijn_16_ambushed_at_wake_island.scn` hold Japanese ones, for a Japanese player party.

### 9.4 The pairs

- **OFF** is this tree's build at `2075be5eb`. **ON** is `pair_export --commit 2075be5eb --flip
  kAiSellThinkBound=true` into `local\se_on`.
- **Logs:** `local\se_{off,on}_{usn13,usn01}.log`.

| mission | pair_diff | deaths | hit records | ON census |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | exit 1, identical | 17 | 283 | `sell thinks=39 splits=0 selling=0` |
| USN01 3200/3000 | exit 1, identical | 5 | 172 | `sell thinks=38 splits=0 selling=0` |

**The prediction held. Decision: `kAiSellThinkBound` is ON.** Its first gameplay effect will come
with a mission from 9.3, and it will be limited while SELLING has no tick arm.

## 10. The Defend think, part 3 (packet `cc9_planner_defend_capture_thinks`)

### 10.1 The image: the no-record path

`00A28A60 BSP_AiPlanner_DefendThink` is the brain+0h think, body `00A28A60-00A29E2A`.

```
00A28AE5  CALL 00A2DEF0 (group)(planner+30h, &list)   ; per owned group: defend candidates
00A28C2F  CALL 00A243D0 (rec+4h, 0)                   ; one 34h-byte record per candidate
00A28D31  CALL 00A2C5A0 / 00A28D44 CALL 00A2C450      ; combatant, no world-set member ...
00A28D6E  CALL 00A186F0                               ; ... and an anchor outside the records: queue
00A28E69  CALL 00A2C5A0 / 00A28E7C CALL 00A2C450      ; records empty: per owned group
00A28EBB  CALL 00A2BE20                               ; no combatant, or a world-set member: DEFENDPOSITION
00A28F22  CALL [vtable+24h]                           ; each queued group released
00A28F38  MOV ESI,[[planner+1Ch]+0Ch]                 ; ... and claimed by brain+0Ch, Capture
00A28F49  CALL 00A1C8B0 / 00A28F88 CALL 00694A60      ; unless already owned; observer
```

- **`00A2DEF0`** appends the members that answer `vtable[+5Ch](1Ch)` (CommandBuilding) or that
  `BSP_SzurkeNyil_ContainsUnit` finds. A record scoring 0 or less is dropped.
- **With no record** the think frees its lists and returns before `00A290F5` (`00A28300`).
- **The records path** runs from `00A290F5` to the end: the sort `00A28300`, `00A1C140`, the anchors,
  the per-record group lists, `00A1A6D0` orders, the merge pass `00A29860-00A29BE7` (docs/AI_PLANNER_TAILS.md
  section 2), and the spawn tail. It was not reconstructed.

### 10.2 The binding

**`kAiDefendThinkBound`, committed OFF.** When ON, the Defend kind runs `defend_think_00a28a60`:
- **No candidate:** when no owned group holds a CommandBuilding member or a world-set member, the
  no-record path runs as above.
- **Any candidate:** the records path is the Siege-shape stand-in, counted as `record_fallbacks`.
  This is labelled: `00A243D0` is not reconstructed, so a candidate may not have yielded a record
  in the image.

The census line is `summary mission ai defend thinks= record_fallbacks= defendposition=`.
`BSP_CAPTURE_DIAG=1` names the members of each group given DEFENDPOSITION.

### 10.3 Predictions, written before the ON runs

**Slot 0 is reached on the reference set, contrary to section 7.**
- On USN13 and USN01, party 0's Defend planner owns the "Storage, 05 01" group: 8 LandFort members
  (kind 27, creator `00747000`), no groupable combatant.
- OFF, the Siege stand-in orders it with `00A2CBD0`. On USN13 it goes at Agano's group (it holds
  CLOSEATTACK, `first_command=9`); on USN01 at the CB2 group.

**USN13 3200/3000 and USN01 3200/3000: exit 3.**
- **No candidate:** the group holds no CommandBuilding, and the world sets are empty. So
  `record_fallbacks=0`.
- **The first Defend think gives it DEFENDPOSITION.** Later thinks keep it (`00A2BE20` returns),
  so `defendposition=1`.
- **Its close-attack pass changes centre.** It moves from the target group's leader (factor 1.5)
  to its own leader (1.0), so its member orders move.
- **The shared RNG stream shifts.** The stand-in's `00A2CBD0` draw (`00BD2F10`) disappears, and
  every later draw moves. That includes Enterprise's CAUTIOUSATTACK draw, so its class may change.
- **Direction:** unit shots and hits by the Storage members change. If Enterprise's class flips to
  MOVETOATTACK, its movement returns and USN13's `tick_orders` rise again.

**USN02 9200/9000 and USN04 4700/4500: identity, exit 0 or 1.** No planner line shows the Defend
kind owning a group there, so the census reads `thinks=0`.

### 10.4 The pairs

- **OFF** is this tree's build at `4c037dc68`. **ON** is `pair_export --commit 4c037dc68 --flip
  kAiDefendThinkBound=true` into `local\df_on`.
- **Logs:** `local\df_{off,on}_{usn13,usn01,usn04,usn02}.log`.

| mission | pair_diff | deaths | hit records (hull) | ON census |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | exit 3 | 17 -> 16 | 283 (117) -> 290 (98) | 39 thinks, 0 fallbacks, DEFENDPOSITION 1 |
| USN01 3200/3000 | exit 3 | 5 -> 5 | 172 (80) -> 177 (74) | 37 thinks, 0 fallbacks, DEFENDPOSITION 1 |
| USN04 4700/4500 | exit 1, identical | 28 | 491 | 58 thinks, DEFENDPOSITION 0 |
| USN02 9200/9000 | exit 1, identical | 12 | 5166 | 112 thinks, DEFENDPOSITION 0 |

**What moved.**
- The Storage group holds DEFENDPOSITION from the first Defend think.
- The removed `00A2CBD0` draw shifts the stream, so Enterprise's group now draws MOVETOATTACK
  (`cautious=0`) on both missions. That is the conditional branch 10.3 named.
- On USN13 `tick_orders` return from 25 to 514, and Enterprise moves 1845.78 m instead of 920.22 m.
  On USN01 they rise from 0 to 51.

**Predictions:**
- **Held:** the mechanism and its census:
  - `record_fallbacks=0` everywhere;
  - one DEFENDPOSITION on USN13 and USN01;
  - exit 3 on those two, exit 1 on USN02 and USN04.
- **Failed on the census only:** `thinks=` is not 0 on USN02 and USN04. The Defend planner is ticked
  every party think with nothing owned, and the no-record path counts that tick.

**Decision: `kAiDefendThinkBound` is ON.**

**The records path is next.** It is reached when a thinking party's Defend planner owns a group
with a CommandBuilding member or a world-set member. That happens in the section 9.3 scenes with a
thinking party that owns a CommandBuilding, and everywhere once `Objectives_Add` produces the world
sets. What it needs, in order:
- `00A243D0`, the Defend record score;
- `00A28300` and `00A1C140`;
- `BSP_AiGroup_PickAnchorEntity` and `00A186F0`;
- the per-record assignment with `00A1A6D0`;
- the spawn tail `00A29B8E-00A29E2A`.

Its merge pass is already in `ai_planner_tails`.

## 11. The CAUTIOUSATTACK tick (packet `cc9_cautious_attack_tick`)

### 11.1 The image

**`vt+0Ch` of CAUTIOUSATTACK**, `00A152E0` (no Ghidra function; the listing is `disasm-raw`):

```
00A152FC  MOV EAX,[EDI+1Ch] / CMP [EAX+5644h],0      ; the target group's first member +FCh,
00A15308  MOV EAX,00F87574                           ;   or the zero vector when it is empty
00A1533F  LEA ECX,[EDI+20h] / CALL 00A14DD0          ; the cautious approach pass (owner, point)
00A15349  CALL 00A10DC0 / 00A15350 CALL 00A11690     ; followers, then an unread pass
00A1535A  MOVSS XMM0,[tuning+1F4h]                   ; CloseAttack_CollectDist
00A153E2..00A15421  d2 = (own - target) x/z squared; collect^2
00A15425  FCOMPI / JBE 00A15478                      ; collect^2 > d2 promotes
00A1542D  new(20h) / 00A15451 CALL 00A10710 / MOV [ESI],00D22C44   ; CLOSEATTACK
00A15473  CALL 00A2BD00
```

**`00A109B0`**, the CAUTIOUSATTACK constructor (instance 2Ch), sets up the `+20h` base as
`00D22BE4`, with the byte `+24h` = 0 and the counter `+28h` = 4.

**`00A14DD0 BSP_AiCommand_CautiousApproachPass`**, `__thiscall(base)(group, point)`:
- **The gate:** it returns unless the group's first member passes `009FE080`.
- **The route object** is the leader's director slot 0 (`00778860`: `vt[+114h]()`, then
  `0071BFC0(0)`, which reads `+1A4h`).
- **With the slot's `vt[+4h]` false, or the counter below 1,** it issues `00A02020(leader, point)`,
  a moveto.
- **Otherwise**, while the flag `+4h` is clear or `vt[+10h]` is above 0:
  - **with the slot's `vt[+0Ch]` false:** it decrements the counter and builds that many waypoints.
    Each leg is split into thirds of `leader -> point`, and each waypoint takes the best of three
    candidates scored by `00A010F0`, pushed with `MSVC_Vector12_PushBack`. Then comes
    `vt[+18h](point)`, and the flag is set;
  - **with it true:** it issues `clearorders` (`00E08F08`, `0077D600` at `00A14EA7`) and clears the
    flag.

**The SELLING tick `00A11FF0`**, read here, does not share this shape:
- **A group without an air member** walks to the nearest own-team list-28 entity (3-D). It takes
  the leader point when within `CaptureRange` (`+7A0h`) times the double `00CE3D40`, else the
  entity's position, through `00A02020`. Then `00A10DC0` and `00A11070` run, and each member with
  `+308h == 0` that passes `005F98F0` routes session message 51h.
- **An air group** issues `00E08F98` with the zero vector to every squadron member whose
  `+361h` and `+3B0h` bytes are clear.

It stays unbound, and a SELLING group is still not moved (section 9.2).

### 11.2 The binding

**`kCautiousAttackTickBound`** is in `include/bsp/ai_command_tick.hpp`, committed OFF. When ON,
`ai_command_tick_vt000c` gains the CAUTIOUSATTACK arm:
- the target group's leader point;
- `00A14DD0`'s no-route arm, a leader moveto through `00A02020`;
- the follower pass;
- the promotion to CLOSEATTACK.

**Labelled:** the director slot objects have no host counterpart, so the no-route arm is taken every
tick. The waypoint build and the `clearorders` arm are not issued, and `00A11690` is not read.

### 11.3 Predictions, written before the ON runs

On main `dcfebd652` the census `cautious=` is 0 on USN13, USN02 and USN04. On USN01 it is 1: the
ScoutDauntless squadron group, ordered at the CB2 group from 91.35 s.

**USN13 3200/3000, USN02 9200/9000, USN04 4700/4500: identity, exit 0 or 1.** No group holds
CAUTIOUSATTACK there.

**USN01 3200/3000: exit 3, from about 91 s.**
- **The ScoutDauntless group** gets a leader moveto at the CB2 group's leader every command tick.
  It is a groupable combatant, and `007EDA90` is false for a non-kamikaze leader. `tick_orders`
  rise above 51.
- **The squadron flies toward CB2,** so "controlled moved ScoutDauntless" rises above 0.00.
- **Once inside 3000 it is promoted to CLOSEATTACK** (`ai command promote ... leader=ScoutDauntless`),
  and the close-attack pass orders it at candidates around CB2.
- **Direction:** the ScoutDauntless may take AA fire near CB2: one more plane death or later hits.
  Nothing before 91 s moves.

**Addendum, written after the four reference pairs and before any other ON run.** On `dcfebd652` the
USN01 premise above is false: the ScoutDauntless now draws MOVETOATTACK (`cautious=0`). The tables
in 11.3 came from pre-sync logs. With no CAUTIOUSATTACK anywhere, all four reference pairs are
identical (11.4), so they do not exercise the arm. An OFF sweep of USN03 to USN14 (3200/3000) finds
four missions that hold it:

| mission | cautious group -> target | OFF `tick_orders` |
| --- | --- | --- |
| USN07 | PBY Catalina 01 (1) -> the Office 01 group (4) | 0 |
| USN09 | Enterprise (17) and Enterprise_sqn01 (1) -> the Zuikaku group (12) | 0 |
| USN10 | Atlanta-class 01 (8) -> the South_Tone_1 group (11); Cleveland-class 01 holds MOVETOATTACK | 46 |
| USN12 | Montpelier (12) -> the Shigure group (3) | 0 |

**Predictions for those four, exit 3 each:**
- **`tick_orders` rise** above the OFF values: every command tick the cautious leader gets
  `00A02020` toward the target leader.
- **The cautious leaders move toward their targets.** Where Montpelier or Enterprise is the
  controlled unit, its "controlled moved" rises.
- **Promotion:** the two air leaders (PBY Catalina 01, Enterprise_sqn01) close inside 3000 within the
  150 s and are promoted to CLOSEATTACK (`ai command promote`). A ship leader is promoted only if it
  starts within about 5 km.
- **Direction:** hits and deaths move toward the targeted groups: Office 01's group, Zuikaku's,
  South_Tone_1's and Shigure's.

### 11.4 The pairs

- **OFF** is this tree's build at `0e1b3080a` (main `dcfebd652` plus the arm, OFF). **ON** is
  `pair_export --commit 0e1b3080a --flip kCautiousAttackTickBound=true` into `local\ca_on`.
- **Logs:** `local\ca_{off,on}_<mission>.log`.

| mission | pair_diff | `tick_orders` OFF -> ON | promotions | what moved |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | exit 0 | 514 -> 514 | 0 | nothing (no CAUTIOUSATTACK) |
| USN01 3200/3000 | exit 0 | 74 -> 74 | 0 | nothing (the ScoutDauntless drew MOVETOATTACK) |
| USN04 4700/4500 | exit 0 | 78 -> 78 | 0 | nothing |
| USN02 9200/9000 | exit 0 | 1 -> 1 | 0 | nothing |
| USN07 3200/3000 | exit 1 | 0 -> 47 | 0 | orders only; the PBY Catalina 01 is the controlled unit, 0.00 m both ways |
| USN09 3200/3000 | exit 1 | 0 -> 97 | 0 | orders only; the controlled Enterprise_sqn01 stays at 0.00 m |
| USN10 3200/3000 | exit 3 | 46 -> 95 | 0 | hit records 23 -> 18, hull 10 -> 8, shots 68 -> 70, deaths 3 both |
| USN12 3200/3000 | exit 3 | 0 -> 49 | 0 | Montpelier moves 2428.02 m instead of 1490.02 m toward Shigure; hits unchanged |

**Predictions:**
- **Failed on premise:** USN01 (11.3). On `dcfebd652` no reference group holds CAUTIOUSATTACK, so
  the four reference pairs are identical and do not test the arm.
- **Held, on the four exercising missions:**
  - `tick_orders` rise everywhere: every command tick the cautious leader gets `00A02020`;
  - Montpelier closes on Shigure;
  - USN10's hits move.
- **Failed on spread:**
  - No group was promoted to CLOSEATTACK within 150 s.
  - The two air leaders are the controlled planes, which the idle player's host does not move on an
    AI moveto (0.00 m both ways, as the ScoutDauntless in section 8.4). USN07 and USN09 therefore
    change orders only.

**Decision: `kCautiousAttackTickBound` is ON.** The arm reproduces the image's no-route moveto, and
every failure is on premise or spread. Before it, a CAUTIOUSATTACK group got no leader order at
all.

**Left open:**
- the director-slot route of `00A14DD0`, which is its waypoint build and `clearorders` arm;
- `00A11690`;
- why the controlled planes ignore an AI moveto (a units-host question, not this arm's).

## 12. The Defend records path (packet `cc9_defend_records_path`)

### 12.1 The image

**`00A243D0`, the Defend record score**, `__fastcall(ECX = own side, EDX = candidate)(out[7],
explain)`, `RET 8`, read in full. It returns T.

```
00A2441E  FLD [tuning+1E4h]                  ; Defend_CollectEnemiesDist, R
00A24432  world+19CCh -> [+34h]              ; world list 2
00A24458  PUSH 1Ch / CALL [vt+5Ch]           ; CommandBuildings are skipped
00A2446B  CALL 00A03510                      ; w, must be above 0
          in = dx*dx + dz*dz < R*R           ; x/z, candidate minus entity
00A2450B  PUSH 6 / 00A2451A PUSH 2 -> [cmd vt+8h]   ; own side: its group neither attacks nor moves
          own side, in:          a += w
          (own == 0) side, in:   b += w; c += w when +C4h == 0Eh, or 18h with 007EDAD0 in {0, 2, 3}
          T = (b - c) - a
00A2460F..00A24642  StrategicGain (Lua, 00D22CF8) + 1.0 (00D7A210)  ; gain
00A24697..00A246D2  max(+1E8h * +19Ch * b, +1ECh)                   ; requirement
out = {T, gain*T, a, b, c, gain, requirement}
```

**The call site** is `00A28C2B MOV ECX,[planner+30h] / 00A28C2F CALL 00A243D0`. The record is
kept only when b > 0: `00A28C39 XORPS / COMISS XMM0,[ESI+10h]`.

**`00A1CF10 BSP_AiGroup_PickAnchorEntity`:** a group with no groupable combatant answers its first
member. Any other group answers the first candidate that its PATROLTO targets (`00A2C230`), or none.

**The records path** (`00A290F5-00A29E2A`):
- `00A28300` sorts the records.
- For each owned group, `00A2919D` picks the anchor. With none:
  - a group already patrolling to an enemy list-28 entity is queued as a Capture pair;
  - otherwise the nearest record entity is the anchor (x/z, strict `<`, seed `1.0e10`).
- The anchor's record `+30h` loses the group's `00A2C530`.
- A group without a combatant gets DEFENDPOSITION (`00A2972C`); any other gets PATROLTO to the
  anchor (`00A29723`, `00A2C310`). The group joins the anchor's list (`00A27A00`).
- Each Capture pair is released, claimed by brain+0Ch (`00A297E2`) and ordered by `00A1A720`.
- The merge pass (`00A29860-00A29BE7`) runs per list, then the spawn tail (`00A29B8E..`), which
  quick-spawns `"[defend]<id>"` for the first record with a positive remainder when
  `00946970 <= 0` and the budget is at least 1.

**The Defend_ tuning** (`IslandCaptureParams_Rookie` lines 105-110; loader `+1E0h..+1F0h`, reader 4
lower):

| reader | key | value |
| --- | --- | --- |
| `+1DCh` | `Defend_MergeTargetDist` | 500 |
| `+1E0h` | `Defend_MergeGroupsDist` | 300 |
| `+1E4h` | `Defend_CollectEnemiesDist` | 4000 |
| `+1E8h` | `Defend_AgainstEnemyResourceMul` | 1.5 |
| `+1ECh` | `Defend_MinimalResource` | 0 |

### 12.2 The binding

**`kAiDefendRecordsPathBound`, committed OFF.** When ON, `defend_think_00a28a60` builds the records.
- **With none kept**, it takes the no-record path of section 10, with pass B's queue.
- **With some**, it runs the records path.

The census line is `summary mission ai defend records thinks= records= patrolto= capture_pairs=
merges= spawn_arms=`. `BSP_CAPTURE_DIAG=1` prints each candidate's terms OFF.

**Labelled substitutions:**
- the `00A28300` comparator is unread, so records keep their build order;
- maps keyed by pointer are in unit-index order;
- `007EDAD0` is answered from the leader plane's class;
- the squadrons of list 2 are the host's squadron candidates;
- the SzurkeNyil candidates are empty, as section 10 found;
- the spawn tail is a record.

### 12.3 Predictions, written before the ON runs

The OFF diagnostics are `local\rp_plan_{LOMP07,LOMP10}.log`.

**No record is kept on either mission.** Every candidate has `b = 0`, because no enemy comes within
4000 of the CommandBuildings in 150 s. Candidates per think:
- LOMP07: CB - Bering and CB - Bering2, in one 32-member group;
- LOMP10: CB4, `a = 2`, `T = -2`.

ON, both therefore take the no-record path.

**LOMP07 3200/3000 and LOMP10 3200/3000: exit 3.**
- **The CommandBuilding group** (CB - Bering's, CB4's; no groupable combatant) gets DEFENDPOSITION
  on the first think. It keeps it after that: `defendposition=1`, `record_fallbacks=0`, records
  census `thinks=0 records=0`.
- **OFF, the stand-in orders the group with `00A2CBD0`:** at Nachi's group on LOMP07; at Ashigara's
  and then Static Emily 01's on LOMP10. ON issues no attack order, so its RNG draws go and the
  shared stream shifts.
- **The group's close-attack pass** moves from the target group's leader to its own leader, so the
  CommandBuilding groups' guns take other targets.

**USN13, USN01, USN02, USN04: identity, exit 0 or 1.** Their Defend planners have no candidate
(`record_fallbacks=0` in section 10.4), and without one the new pass A is empty and pass B queues
nothing.

### 12.4 The pairs

- **OFF** is this tree's build at `3be3ea32c` (main `56b4eec58` plus the path, OFF). **ON** is
  `pair_export --commit 3be3ea32c --flip kAiDefendRecordsPathBound=true` into `local\rp_on`.
- **Logs:** `local\rp_{off,on}_<mission>.log`.

| mission | pair_diff | defend census ON | records census ON | what moved |
| --- | --- | --- | --- | --- |
| LOMP07 3200/3000 | exit 3 | 39 thinks, 0 fallbacks (OFF 37), DEFENDPOSITION 1 | 0 records | unit table only: CB - Bering's range 1500 -> 0, nearest 19138 -> 0; deaths, hits, shots 0 both |
| LOMP10 3200/3000 | exit 1, identical | 38 thinks, 0 fallbacks (OFF 39), DEFENDPOSITION 1 | 0 records | `attack_orders` 2 -> 0; 9 deaths, 340 hit records both |
| USN13 3200/3000 | exit 1 | DEFENDPOSITION 1 | 0 records | nothing |
| USN01 3200/3000 | exit 1 | DEFENDPOSITION 1 | 0 records | nothing |
| USN04 4700/4500 | exit 1 | DEFENDPOSITION 0 | 0 records | nothing |
| USN02 9200/9000 | exit 1 | DEFENDPOSITION 0 | 0 records | nothing |

**Predictions:**
- **Held:** the mechanism:
  - LOMP07's and LOMP10's CommandBuilding groups take DEFENDPOSITION instead of the stand-in's
    attack orders;
  - no record is kept anywhere;
  - `record_fallbacks` falls to 0;
  - the reference four are identical.
- **Failed on spread:**
  - LOMP10 is identical, although the stand-in's two attack orders and their draws are gone;
  - LOMP07 moves only in its unit table: the CommandBuilding group loses its CLOSEATTACK target.

**Decision: `kAiDefendRecordsPathBound` is ON.**

**The records path itself is still untested.** On these runs no Allied CommandBuilding has an enemy
inside 4000 within 150 s. A longer LOMP07 or LOMP10 run, or a mission with an assault on an own
CommandBuilding, would exercise it. Its first observable is the census `records=`.

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| `00A1E210` | `00A1E246` | Capture/Attack vtable `+24h` (`00D22E34+24h`, `00D22D94+24h` both read `00A1E210` from the PE). `RET 4` (`C2 04 00`) at `00A1E244`; `INT3` at `00A1E247`. `ghidra proto` finds no function |
