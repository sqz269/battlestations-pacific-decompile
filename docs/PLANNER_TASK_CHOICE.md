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

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| `00A1E210` | `00A1E246` | Capture/Attack vtable `+24h` (`00D22E34+24h`, `00D22D94+24h` both read `00A1E210` from the PE). `RET 4` (`C2 04 00`) at `00A1E244`; `INT3` at `00A1E247`. `ghidra proto` finds no function |
