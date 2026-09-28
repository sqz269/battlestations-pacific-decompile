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

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| `00A1E210` | `00A1E246` | Capture/Attack vtable `+24h` (`00D22E34+24h`, `00D22D94+24h` both read `00A1E210` from the PE). `RET 4` (`C2 04 00`) at `00A1E244`; `INT3` at `00A1E247`. `ghidra proto` finds no function |
