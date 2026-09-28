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

## 5. Follow-up

**`cc9_planner_defend_capture_thinks`** would reconstruct the Defend think (`00A28A60-00A29E2A`)
and the Capture think (`00A29FD0-00A2B7EB`):
- their target collections;
- `BSP_AiPlanner_CaptureTargetScore` `00A1E250`, which is read;
- the claim loops (`00A1C8B0` inlined);
- the order calls `00A1A720` and `00A1A6D0`.

It would bind them behind a switch in place of the Siege shape for kinds Defend and Capture. That
settles which enemy group Houston's group and the USN13 squadrons are sent at.

## no_ghidra_function

None. Every address named lies inside a Ghidra function.
