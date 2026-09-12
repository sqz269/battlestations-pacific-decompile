# AI planner tails: the Defend and Capture scoring, the command factory, the script spawner

Addresses: 00A1E250, 00A03760, 009FFC10, 00946FC0, 00A13340, 00A16EF0, 00A1A720, 00A1A6D0,
00A2BE20, 00A10D50, 00A10C60, 00A2C4C0, 00A17880, 00A17960, 00A28A60, 00A29FD0, 00D22CC4,
00D22CC8, 00D22C9C, 00D22CA8, 00E0E308

This packet closes the open questions of `docs/AI_PLANNERS.md` line 318 onward: the scoring inside
the `Defend` think `00A28A60` and the `Capture` think `00A29FD0`, the `"capture: %.2f ..."`
formula, `00A16EF0`, the command factory `00A13340`, the two merge tests `00A10D50` / `00A10C60`,
the three unread helpers `00A17880` / `00A17960` / `00A2C4C0`, and the Lua key names of the tuning
fields the target pass reads. Ghidra was **read-only** for this packet: no renames, comments,
prototypes or saves. Every descriptive name below is a hypothesis, not a recovered symbol, with
one exception that is a genuine string literal in the image: the two group-name prefixes
`"[defend]"` and `"[capture]"`.

## Headline: the capture formula is not in the Capture think

`00D22CC4` is **not** a format string. It is the float `-999999.0`, the initial best score of the
shared target pass `00A1CB80` (`MOVSS XMM0,[0x00D22CC4]` at `00A1CC17`), which
`include/bsp/ai_planners.hpp` already carries correctly as `kAiPlannerScoreFloor`. The format
string `"capture: %.2f\n=(%.2f-%.2f/2)+(%.2f-%.2f/2)+%.2f"` starts four bytes later at
`00D22CC8`, and its only user in the image is `00A1E250` (`PUSH 0xd22cc8` at `00A1E8BB`; a
byte scan of `.text` for the literal `c8 2c d2 00` returns that one site). `00A1E250` is called
from `BSP_AiPlanner_CaptureThink` (`00A2A184`) and from `FUN_00a16cc0`.

So the capture score is a separate routine, and the Capture think consumes it.

## 1. The Capture scoring

### `00A1E250 BSP_AiPlanner_CaptureTargetScore`, body `00A1E250-00A1E981`, complete

`__fastcall(ECX = int ownTeam, EDX = Entity* target)(float out[6], NativeString* explain)`,
`RET 8`, returns the total in `ST0`. `ECX` is the team id the caller wants scored, taken at
`00A2A184` from `brain+24h` (`MOV EAX,[EBP+0x1c]; MOV ECX,[EAX+0x24]`, the planner's brain at
`planner+1Ch`). `out` may be null and `explain` may be null; both are null-checked
(`00A1E834`, `00A1E880`).

The **enemy team is `(ownTeam == 0)`** — `SETZ AL` on `TEST EBX,EBX` at `00A1E27D`, stored at
`[ESP+14h]`. This is the same team flip every planner uses; it distinguishes teams 0 and 1 only
and maps team 2 onto team 0.

| Term | Rule | Evidence |
| --- | --- | --- |
| `a` | sum of `00A03760(unit, target)` over every unit in the four world lists whose `unit+54h == ownTeam` | `00A1E2A0`, `00A1E2F1`, `00A1E343`, `00A1E395` |
| `b` | the same sum for `unit+54h == enemyTeam` | `00A1E2C9`, `00A1E31A`, `00A1E36C`, `00A1E3BE` |
| `term1` | `a - b * 0.5` | `FMUL double [0x00D7A280]` (= 0.5), `FSUBP` at `00A1E3E2`-`00A1E3FB` |
| `ownDist` | min over `world+19CCh+16Ch` of the XZ distance from a unit of `ownTeam` to the target, `1.0e10` when the team has none | seed `0x501502F9` at `00A1E3D5`, loop `00A1E410`-`00A1E64E`, read at `00A1E654` |
| `enemyDist` | the same for the enemy team | read at `00A1E692` |
| `c` | `AvailableResources(ownTeam) * (1000 / max(ownDist, 1000)) / Capture_CapturePointResourceValue` | `00A1E6C0`-`00A1E6E2` |
| `d` | the same for the enemy team | `00A1E6E6`-`00A1E70A` |
| `term2` | `max(0, c - d * 0.5)` | `00A1E70E`-`00A1E739` |
| `e` | the target's Lua field `StrategicGain` (`00D22CF8`), default `0.0` | `00A1E7B1`-`00A1E7DF` |
| total | `max(term1 + term2, Capture_MinimalCBTargetWeight) + e` | floor at `00A1E74B`-`00A1E773`, add at `00A1E81D` |

The floor and the two clamps are invisible in the printed decomposition: the format string shows
`total = (a - b/2) + (c - d/2) + e`, so a printed line whose terms do not add up to the printed
total is the `max(0, ...)` on `term2` or the `Capture_MinimalCBTargetWeight` floor biting.

The three world lists that feed `a` and `b` hang off `world+19CCh` at `+64h`, `+13Ch`, `+358h`
and `+364h`; the distance loop uses a fifth list at `+16Ch`. The per-team minimum is a three-entry
stack array at `[ESP+34h]`, indexed by `unit+54h`, so only teams 0, 1 and 2 exist.

`out[0..5]` receives `total, a, b, c, d, e` in that order (`00A1E836`-`00A1E872`), which is the
same order the format string prints.

### `00A03760 BSP_AiCapture_UnitArrivalValue`, body `00A03760-00A038B5`, complete

`__fastcall(ECX = Entity* unit, EDX = Entity* target)`, no stack arguments, `RET`, returns a float.

```
w = 00A03510(unit)                        ; the unit's Lua CaptureWeight (00CFD940-ish key)
d = HorizontalLength(unit.worldPos - target.worldPos)      ; 009FFC10, x and z only
R = (float)(int)target[+7A0h]                              ; FILD, an integer field
if (d <= R)  return w                                      ; already in the capture radius
speed = unit->IsType(6)    ? *(float*)(unit[+538h] + 0x500)
      : unit->IsType(0x18) ? *(float*)(unit[+35Ch] + 0x188)
      :                      0.0f
if (speed <= 0.1)  return 0                                ; 00D7A3A0, a double
t     = (d - R) / speed
limit = max(Capture_ArriveToRangeTime, 1.0f)               ; tuning+198h, shipped 30
if (t >= limit) return 0
return (1 - t / limit) * w
```

So a unit contributes its full `CaptureWeight` inside the point's radius and a linear fade to zero
as its time-to-arrive approaches `Capture_ArriveToRangeTime`. `IsType(6)` is the `ATTACK`-family
id of `docs/AI_PLANNERS.md`'s table used as an entity class query here, not a command query; the
two `+538h`/`+35Ch` sub-objects hold a speed at `+500h` and `+188h`.

### `009FFC10 BSP_Math_HorizontalLength`, body `009FFC10-009FFC65`, complete

`__thiscall(const float v[3])`, returns `sqrt(v[0]*v[0] + v[2]*v[2])`, or exactly `0.0f` when the
square is at or below `00CE3820` (a double, `1.0e-11`). The `y` component is never read.

### `00946FC0 BSP_AiParty_AvailableResources`, body `00946FC0-00947084`, complete as a rule

`__fastcall(int team)`, returns a float. Starts from `[00E0CFB4] * 0.5`, subtracts `unit+304h`
for every unit of `world+19CCh+58h` whose `unit+54h == team` and whose four state bytes
(`+5Ch`, `+5Dh`, `+60h`, `+5Eh`) are clear, following the `IsType(0xF)` indirection through
`unit+9D4h` when `BSP_Unit_LacksFollowTarget` says so, then subtracts `FUN_009469f0(team)`.
`coverage: complete for the accumulation; the two sub-helpers 009469F0 and the +9D4h indirection
were not read`.

### What the Capture think does with the score, `00A2A130`-`00A2A1E7`

For each candidate target whose `target+54h != planner+30h` (`planner+30h` is `brain+24h`, so the
gate is "not already on our side"), the think inserts a record through `00A22F60` and fills it:

| Field | Value | Evidence |
| --- | --- | --- |
| `rec+0h` | the target entity | `00A2A174` |
| `rec+4h..rec+18h` | `BSP_AiPlanner_CaptureTargetScore`'s six floats: total, a, b, c, d, e | `00A2A184` |
| `rec+1Ch` | `max(Capture_CapturePointResourceValue * (b + d), Capture_MinimalResource * CaptureWeight(target))` | `00A2A18B`-`00A2A1E2` |

`rec+1Ch` is the resource price the planner charges itself for taking that point: the larger of
what the enemy has invested there (`b + d`, scaled by the 35-point value of a capture point) and a
floor proportional to the point's own `CaptureWeight` (`Capture_MinimalResource`, 150). The score
return value itself is discarded at the call (`FSTP ST0` at `00A2A189`); only the six-float
breakdown is used.

### The Capture think's other scoring rules

| Site | Rule | Tuning fields |
| --- | --- | --- |
| `00A2A046`-`00A2A0C1` | `planner+38h` (planner age) and `planner+3Ch` (time since last spawn) both accumulate `dt`; the spawn gate is `planner+3Ch > InterpolateClamped(0, Capture_SpawnDelay[1], Capture_SpawnDelayTime, Capture_SpawnDelay[2], planner+38h)`, so the delay ramps from 30 s at mission start to 5 s after 120 s of planner life | `+1C0h`, `+1C4h`, `+1C8h` |
| `00A2A5FA`-`00A2A66E` | `w = InterpolateClamped(Capture_ActAttackTargetWeightMulDist[1], Capture_ActAttackTargetWeightMul[1], Capture_ActAttackTargetWeightMulDist[2], Capture_ActAttackTargetWeightMul[2], dist)`, i.e. 100x inside 3000 units falling to 1.5x at 4500 and beyond | `+1B0h`, `+1ACh`, `+1B4h`, `+1A8h` |
| `00A2AC11`-`00A2AC4E` | the best-target pick blends two weights: `score = (1 - Capture_CommandBuildingStrategicWeightMul) * w + Capture_CommandBuildingStrategicWeightMul * s`, kept only while `score` beats the running best | `+1BCh` (twice) |
| `00A2B4A6`-`00A2B4EE` | the spawn budget: `available = PartyResourceCap(brain+20h) * (1 - Defend_ResourcePercent[mode]) - 00A1C900(planner)`, and composing a new group needs `available > 1.0` | `00F8A8BC[mode]` |

`00A2B4A6` is a call to `009FFC80 BSP_Ai_EffectiveGameModeIndex`, **not** to `00A371A0`:
`docs/AI_PLANNERS.md` listed it among the `00A371A0` sites. The twelve real `00A371A0` sites in the
Capture think are `00A2A071`, `00A2A080`, `00A2A08F`, `00A2A18B`, `00A2A1B0`, `00A2A605`,
`00A2A614`, `00A2A623`, `00A2A632`, `00A2A670`, `00A2AC11`, `00A2AC28`.

## 2. The Defend scoring

The Defend think has exactly two `00A371A0` sites, `00A298E2` and `00A29AB0`, and both belong to
one pass: the group merge inside a defended target's group list.

`00A28A60`-`00A29E2A`, `coverage: partial` — the merge pass `00A29860-00A29BE7` is read in full,
the claim and issue decisions are as `docs/AI_PLANNERS.md` already recorded them, and the
collection passes `00A28A87-00A29850` and the spawn tail `00A29B8E-00A29E2A` are read only for
their call sequence.

The planner's working set is a container of per-target records; each record has the defended
entity at `+0Ch`, a `std::list<AiGroup*>` object at `+10h` and its size at `+18h`.

| Rule | Condition | Action |
| --- | --- | --- |
| skip small records | `record+18h <= 1` | next record (`00A298AD`) |
| target proximity | `dist2D(g1.leader, record+0Ch)^2 > Defend_MergeTargetDist^2` | next `g1` (`00A298E2` reads `tuning+1DCh`, squares it at `00A298FC`, compares at `00A29A52`) |
| group proximity | `Defend_MergeGroupsDist^2 > dist2D(g1.leader, g2.leader)^2` | merge: `00A2DB80(g1, g2)` then `00A1D1D0(list, &g2)` erases `g2` (`00A29AB0` reads `tuning+1E0h`, squares it at `00A29ACA`, compares at `00A29B7B`, acts at `00A29B8E`) |

Both distances are XZ only (`+0FCh` and `+104h` of the leader's cached world pose), both are
compared squared, and an empty group substitutes the zero vector at `00F87574` for its leader
position. The shipped defaults are 2000 and 500, so a group is merged into another only while both
sit within 2 km of the thing they defend and within 500 m of each other.

## 3. `00A16EF0 BSP_AiGroup_CreateFromScript`, body `00A16EF0-00A171FF`, complete

**Correction: this is not the `Attack` planner's group producer.** It never touches `brain+4h`.

`__fastcall(ECX = NativeString* groupName, EDX = std::vector<Entity*>* members)`, no stack
arguments, `RET`. Its only caller is `FUN_00947a30`.

1. `party = BSP_Unit_LossCountingSlot(members[0])` (`00A16F29`); `brain = [00F8A89C + party*4]`;
   return when that is null (`00A16F3D`).
2. `operator new(0x5660)`, `BSP_AiGroup_Construct(group, members[0])` (`00A16F7B`), then
   `BSP_AiGroup_AddEntity(group, m)` for every remaining member (`00A16FD6`).
3. `mode = BSP_Game_GetEffectiveGameMode([00E188A8])` (`00A16FED`) and then:

| mode | action |
| --- | --- |
| 4 | `planner_claim_group(brain+10h, group)` — `Duel` (`00A17000`) |
| 5 | `brain+14h` — `Escort` (`00A17024`) |
| 6 | `brain+18h` — `Siege` (`00A17048`) |
| 7 | `brain+1Ch` — `Competitive` (`00A1706C`) |
| 0..3 | the name-prefix path below |

For modes 0 through 3 it first issues `BSP_AiGroup_IssueDefendPosition(group)` (`00A17085`) and
then routes on a **case-insensitive prefix of the group's name**:

| Prefix | Length | Planner claimed | Argument | Order helper |
| --- | --- | --- | --- | --- |
| `"[capture]"` (`00D22CA8`) | 9 | `brain+0Ch`, `Capture` (`00A170DB`) | `name.substr(9)` through `atoi` and `BSP_EntityHandleTable_Resolve` | `00A1A720` at `00A1711C` |
| `"[defend]"` (`00D22C9C`) | 8 | `brain+0h`, `Defend` (`00A171A6`) | `name.substr(8)` through `00425BD0` and `BSP_EntityHandleTable_Resolve` | `00A1A6D0` at `00A171D8` |
| anything else | — | none | — | — |

A group with neither prefix is left unclaimed and is picked up by the brain's own think on the
next party tick. The substring calls are `BSP_NativeString_Substring` and the comparison is
`BSP_NativeString_EqualsCStringInsensitive`.

### `00A1A720 BSP_AiPlanner_OrderCaptureGroup`, body `00A1A720-00A1A795`, complete

`__thiscall(planner)(AiGroup* group, Entity* target)`, `RET 8`.

- `target+16Ch` non-null (the target already belongs to an AI group): `00A2CBD0(group)(target+16Ch,
  aggression)` where `aggression = [00F8A8D0 + p*28]` and `p = planner->brain->partySlot`
  (`[[planner+1Ch]+20h]`). `00A2CBD0` is the attack-order chooser of `docs/AI_PLANNERS.md`.
- else `00A2C600(group)` true: `BSP_AiGroup_IssueDefendPosition(group)`.
- else: refresh the target pose and `00A2C310(group)(&target.worldPos)`, the `PATROLTO` issue.

### `00A1A6D0 BSP_AiPlanner_OrderDefendGroup`, body `00A1A6D0-00A1A712`, complete

`__thiscall(planner)(AiGroup* group, Entity* target)`, `RET 8`. `HasGroupableCombatant(group)`
(`00A2C5A0`) picks `PATROLTO` to the target position; otherwise `DEFENDPOSITION`. The planner
pointer in `ECX` is not read in this body.

### `00A2BE20 BSP_AiGroup_IssueDefendPosition`, body `00A2BE20-00A2BE6E`, complete

`__thiscall(AiGroup* group)`, no arguments. Returns immediately when the group's current command
`group+564Ch` already answers `IsType(11)`. Otherwise it allocates 8 bytes, writes the
`DEFENDPOSITION` vtable `00D22A38` and the owning group at `+4h`, destroys the old command through
`vt+0h` with the deleting flag `1`, and stores the new one at `group+564Ch`.
`docs/AI_PLANNERS.md`'s class table lists `00A2BE20` in a "Ctor seen" column; it is the issue
helper, and the object itself is constructed inline here.

## 4. `00A13340 BSP_AiCommand_CreateFromLua`, body `00A13340-00A137C9`, complete

`__thiscall(AiGroup* group)(...)`, `RET 0x14` — five stack arguments; the Lua object the routine
reads sits in the frame at `[ESP+8Ch]`. It reads the key `"commandType"` (`00D22A88`) through
`BSP_LuaObject_GetByName` + `00B662B0`, assigns it into a native string, and compares it
case-insensitively against the ten entries of the command type name table `00E0E308`. Every
allocation writes the owning group at `command+4h`.

| Name global | Id | Name | Allocation | Installed by | Extra Lua key |
| --- | --- | --- | --- | --- | --- |
| `00E0E308` | 0 | `NONCONTROL` | 8 | vtable `00D22990` at `00A13430` | — |
| `00E0E30C` | 1 | `IDLE` | 8 | vtable `00D229E0` at `00A133F6` | — |
| `00E0E314` | 3 | `MOVETO` | `14h` | vtable `00D22ABC` at `00A134D0` | `targetPos` (`00D22C78`), stored at `+8h`, `+0Ch`, `+10h` |
| `00E0E318` | 4 | `CAUTIOUSMOVE` | `20h` | ctor `00A102D0` at `00A13560` | `targetPos` |
| `00E0E31C` | 5 | `REGROUPINGMOVE` | `14h` | ctor `00A10370` at `00A135D2` | `targetPos` |
| `00E0E324` | 7 | `MOVETOATTACK` | `20h` | ctor `00A10890` at `00A13651` | `target` (`00CFD964`) through `BSP_AiGroup_FromLuaEntityArgument` |
| `00E0E328` | 8 | `CAUTIOUSATTACK` | `2Ch` | ctor `00A109B0` at `00A136B2` | `target` |
| `00E0E32C` | 9 | `CLOSEATTACK` | `20h` | ctor `00A10AE0` at `00A13712` | `target` |
| `00E0E334` | 11 | `DEFENDPOSITION` | 8 | vtable `00D22A38` at `00A13740` | — |
| `00E0E33C` | 13 | `RETREAT` | 8 | vtable `00D22A60` at `00A1376E` | — |
| — | — | (no match, and no `commandType` key) | 8 | vtable `00D229E0` = `IDLE` at `00A13787` | — |

Ten classes plus the `IDLE` fallback, not nine. The six the factory cannot build are the four
abstract bases (`MOVE`, `ATTACK`, `DEFEND` and the root), `PATROLTO` (id 12) and `SELLING`
(id 14) — those three concrete ones are only ever issued from native code. The vec3 reader is
`BSP_LuaObject_ReadVector3`. Every size here matches or extends
`docs/AI_PLANNERS.md`'s class table, which had sizes only for ids 7, 8 and 9.

## 5. The two merge tests

Both are `__thiscall(AiCommand* command)(AiGroup* other)`, `RET 4`, and both take "the group's
leader" to be the first member of the `group+5640h` list, substituting the zero vector at
`00F87574` when `group+5644h` is zero.

| Address | Body | Rule |
| --- | --- | --- |
| `00A10D50` | `00A10D50-00A10DBB` | `AiLeaderWeight(other.leader) >= AiLeaderWeight(command->group.leader)` — merge only into an equal or stronger lead unit (`FXCH` at `00A10D9F` then `FCOMIP`, `JC` returns 0) |
| `00A10C60` | `00A10C60-00A10D4E` | `AutoMerge_MergeDist^2 > dist2D(command->group.leader, other.leader)^2`, exactly the rule `ai_group_within_auto_merge_distance` already carries for the compose pass at `00A2EDB6` |

The weight is `009FFD70`, an adjustor thunk that loads `entity+0C4h` into `ECX` and tail-jumps to
`009FDF30`, the same body Ghidra already reaches through `009FFD80 BSP_Entity_AiLeaderWeight`.
`00A10C60` reads the tuning field at `00A10CF5` and, because of the `POP` sequence between the two,
lands the squared limit in the caller's argument slot at `[ESP+18h]`; the arithmetic is still the
plain `dx*dx + dz*dz` compare.

## 6. The three unread helpers

| Address | Body | What it is |
| --- | --- | --- |
| `00A2C4C0` | `00A2C4C0-00A2C520` | `BSP_AiGroup_TotalLeaderWeight`, `__thiscall(AiGroup*)`, no arguments, returns the sum of `AiLeaderWeight(member)` over `group+563Ch`. This is the group strength the engagement pass `00A179E0` compares within a factor of two. |
| `00A17880` | `00A17880-00A17925` | `std::vector<T>::push_back` for a 28-byte element (`0x92492493` reciprocal, `ADD EDI,0x1C`); in-place construct through `00A169F0` while `size < capacity`, otherwise the reallocating insert `00A176A0`. Library code, not reconstructed. |
| `00A17960` | `00A17960-00A179CB` | `std::list<T>` copy construction: `this+4h = 00A168A0()` (a self-linked 12-byte sentinel), `this+8h = 0`, then the range insert `00A17760`. Library code, not reconstructed. |

Both STL helpers are called from `BSP_AiPartyBrain_EngagementPass` `00A179E0`; `00A17960` is also
called from `00A25350`, `00A253F0`, `00A27A00` and `00A27AE0`.

## 7. The tuning keys

The open question was already answered elsewhere: the four fields the shared target pass
`00A1CB80` reads, and `+208h`'s neighbours, are named in
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md`'s record table. `00A371A0` returns
`coordinator + 4 + mode*23Ch`, so a reader-side `tuning+N` is that table's `record +N` column.

| Reader offset | Lua key | Shipped default | Used as |
| --- | --- | --- | --- |
| `+1CCh` | `FreeAttack_ObjectiveTargetMul` | 2 | the own-world-set multiplier in `00A1CB80` |
| `+1D0h` | `FreeAttack_NearDist` | 5000 | the near end of the range interpolation |
| `+1D4h` | `FreeAttack_FarDist` | 12000 | the far end |
| `+1D8h` | `FreeAttack_ExistingTargetMul` | 1.5 | the sticky-target multiplier |
| `+204h` | `CloseAttack_TargetGroupMemberMul` | 2 | `+208h`'s low neighbour |
| `+208h` | `AutoMerge_MergeDist` | 650 | `00A10C60` and the compose pass |
| `+20Ch` | `AutoMerge_LeaveDist` | 1200 | `+208h`'s high neighbour |

The fields this packet adds to that list of consumers: `+198h` (`Capture_ArriveToRangeTime`) in
`00A03760`, `+19Ch` and `+1A0h` in `00A1E250` and the Capture think, `+1A8h`..`+1BCh` in the
Capture think's target weighting, `+1C0h`..`+1C8h` in its spawn ramp, and `+1DCh` / `+1E0h` in the
Defend merge pass.

## Corrections to `docs/AI_PLANNERS.md`

1. The `"capture: ..."` format string is at `00D22CC8`, not `00D22CC4`; `00D22CC4` is the float
   `-999999.0`. The format string's only user is `00A1E250`, not the Capture think.
2. `00A2B4A6` is `CALL 009FFC80`, not a `00A371A0` site.
3. `00A16EF0` is not the `Attack` planner's group producer. It claims into `Duel`, `Escort`,
   `Siege`, `Competitive`, `Capture` or `Defend` and never into `brain+4h`.
4. `00A13340` installs ten named classes plus an `IDLE` fallback, not nine.
5. `00A2BE20` is the `DEFENDPOSITION` issue helper, not a constructor.

## Reconstruction

`include/bsp/ai_planner_tails.hpp` and `src/ai_planner_tails.cpp`. Complete as pure rules: the
capture formula and its clamps, the per-unit arrival value, the horizontal length, the two Defend
merge distance tests, the Capture spawn-delay ramp and budget test, the attack-weight
interpolation, the record price at `rec+1Ch`, the command factory's name-to-class map, the script
spawner's prefix routing, and the two command merge tests. The sequence routine
`ai_capture_target_score` runs over `AiCaptureScoreHost`, one method per native call site. Nothing
here is ABI-compatible.

## Open questions

- The Defend think's collection passes `00A28A87-00A29850` (the `00A2DEF0` / `00A243D0` /
  `00A28300` / `00A1C140` chain that builds the per-target records) and its spawn tail
  `00A29B8E-00A29E2A` were read only for their call sequence.
- The Capture think outside the four scoring blocks above: `00A2A200-00A2A5C0`,
  `00A2A6E0-00A2AC00`, `00A2AC90-00A2B480` and `00A2B4F0-00A2B7EB`.
- `00A1C900`, the planner's already-committed resource total, and `00942130`, the party resource
  cap, were not read.
- `00A22D10` and `00A22F60`, the two record accessors the Capture think uses, were not read; the
  record layout above comes from the store sites only.
- `009469F0` and the `+9D4h` indirection inside `00946FC0`.
- The second value blended at `00A2AC33` (`[ESP+0A0h]`) has no producer this packet found.
- Which of `00A03760`'s two speed sub-objects (`+538h`, `+35Ch`) belongs to which entity family.
