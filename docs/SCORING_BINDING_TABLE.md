# Scoring binding table (packet `scoring_binding_table`)

Addresses: 008b8ff0, 008b9190, 008b93c0, 008b95c0, 008b97f0, 008b9a30, 008b9c90, 008b9e90,
008ba0f0, 008ba350, 008ba550, 008ba7b0, 008baa10, 008bac10, 008bae70, 008bb0d0, 008bb2d0,
008bb530, 0090c790, 0090c800, 0090c870, 0090c8e0, 0090c950, 0090f0e0, 0090f110, 0090f140,
0090f170, 0090f1a0, 0090f1d0, 0090f200, 0090f230, 0090f260, 0090f290, 009103f0, 00910480

Follow-up of `docs/MISSION_RESULT_DECISION.md`, which established the 284h score record, its
manager at `game+21A0h` (eight records at `+4h`, the commit slot at `+1424h`) and the
persistence model in `include/bsp/mission_progress.hpp`. This packet reads the keyed
`Scoring_*` bindings and the manager methods behind them, so every runtime writer of the
record's seven keyed maps is now named with its offset and its value rule.

`Scoring_SetFinalScoringFunctionName` (008b8640) belongs to `docs/LUA_BINDING_CORE.md` and
`Scoring_SetMissionCompleted` (008b8ad0), `Scoring_SetRanking` (00905350) and
`Scoring_SetMissionMedal`'s record row to `docs/MISSION_RESULT_DECISION.md`; they are cited,
not re-derived.

## The shared binding prologue

Every keyed binding is one `__fastcall(lua_State*)` with the same frame: a function-static
`NativeString` under a `luakod` first-use guard, `BSP_LuaStateOwner_ConstructBorrowed`,
`BSP_LuaObject_OpenCallFrame`, and a `BSP_LuaObject_ResultCount` return. Argument reading and
result pushing go through the `LuaObject` API already modelled in
`include/bsp/lua_binding_core.hpp`.

Two prologue variants select the player slot. Both read the multiplayer flag at `game+1FE4h`
and default the slot to `game+18ECh`, the local player index.

**Variant A**, no argument-count test (008b9190 008b93c0 008b95c0 008b97f0 008b8ff0 and all
five `Get*Score`). Evidence, 008b9273..008b92bc:

```
008b9273  MOV EDI,dword ptr [EAX + 0x18ec]   ; slot = local player
008b927b  XOR ESI,ESI                        ; base = 0
008b927d  CMP dword ptr [EAX + 0x1fe4],EBP   ; EBP = 0 (XOR EBP,EBP at 008b91b3)
008b928c  PUSH EBP / CALL 00b677e0 / CALL 00b66290
008b92ae  MOV EDI,EAX                        ; slot = arg[0].integer
008b92bc  MOV ESI,0x1                        ; base = 1
```

In single player the slot argument does not exist and the key is argument 0.

**Variant B**, an argument-count test (the Action / Ship / Plane / Command `Set*` and `Add*`).
Evidence, 008b9b4e..008b9b8a: in single player the binding calls the argument count
(00B663F0) and compares it with 3; on a match it reads argument 0 as an integer **and throws
the value away** - the multiplayer branch at 008b9b4a does `MOV EDI,EAX`, the single-player
branch at 008b9b75 falls through to 008b9b7a without it - yet still shifts `base` to 1. So a
three-argument single-player call is accepted and its slot argument is silently ignored; the
local player's record is written either way.

The tail is identical in both variants: `key = arg[base].string`, `value = arg[base+1].integer`,
`ECX = [game+21A0h]`, then the manager method with `(slot, &key, value)` pushed right to left.
Evidence for the argument order and the manager, 008b9334..008b9341:

```
008b9334  PUSH ESI                          ; value   -> [esp+0Ch]
008b9339  PUSH ECX                          ; &key    -> [esp+8]
008b933a  MOV ECX,dword ptr [EDX + 0x21a0]  ; this = the scoring manager
008b9340  PUSH EDI                          ; slot    -> [esp+4]
008b9341  CALL 0x009103f0
```

All ten manager setters end in `RET 0Ch`, which confirms three stack arguments (rule 7).

The two medal bindings add an optional trailing boolean. Evidence, 008b9961..008b996d:
`MOV byte ptr [ESP+0x10],0x1` then `CALL 00B663F0; CMP EAX,ESI; JLE` past the read, so the
default is **granted = true** and `Scoring_SetMissionMedal(key)` grants the medal.

## The record, offset by offset

The manager indexes as `this + slot*284h + X`; the record itself begins at `this + 4 +
slot*284h`, so the record-relative offset is `X - 4`. Field names are
`MissionScoreRecord`'s in `include/bsp/mission_progress.hpp`; nothing here re-declares them.

| Record | Field | Manager expr | Set | Add | Get | Value rule |
| --- | --- | --- | --- | --- | --- | --- |
| `+00h` | `mission_completed_00` | `+04h` | 00906460 | - | - | see `docs/MISSION_RESULT_DECISION.md` |
| `+04h` | `ranking_04` | `+08h` | 00905350 | - | - | see `docs/MISSION_RESULT_DECISION.md` |
| `+18h` | `score_maps[Mission]` | `+1Ch` | 009103f0 | 00910480 | 0090c790 | difficulty-scaled, below |
| `+24h` | `score_maps[Action]` | `+28h` | 0090f110 | 0090f140 | 0090c800 | plain set / plain add |
| `+30h` | `score_maps[Ship]` | `+34h` | 0090f170 | 0090f1a0 | 0090c870 | plain set / plain add |
| `+3Ch` | `score_maps[Plane]` | `+40h` | 0090f1d0 | 0090f200 | 0090c8e0 | plain set / plain add |
| `+48h` | `score_maps[Command]` | `+4Ch` | 0090f230 | 0090f260 | 0090c950 | plain set / plain add |
| `+54h` | `score_maps[MissionMedals]` | `+58h` | 0090f0e0 | - | - | boolean stored as 0 or 1 |
| `+60h` | `score_maps[ActionMedals]` | `+64h` | 0090f290 | - | - | boolean stored as 0 or 1 |
| `+1E0h` | `totals_1c8[6]` | `+1E4h` | - | - | inline in 008b8ff0 | read only, no key |

The `LEA` that fixes each row (rule 4, the producer settles the offset): 0090f0ef `+58h`,
0090f12e `+28h`, 0090f15e `+28h`, 0090f18e `+34h`, 0090f1be `+34h`, 0090f1ee `+40h`,
0090f21e `+40h`, 0090f24e `+4Ch`, 0090f27e `+4Ch`, 0090f29f `+64h`, 00910461 `+1Ch`,
009104a1 `+1Ch`. The getters use the record base instead: 0090c79f `LEA EAX,[EAX+ECX+4]`
then 0090c7a9 `LEA ESI,[EAX+0x18]`, and likewise `+24h`, `+30h`, `+3Ch`, `+48h`.

`Scoring_GetTotalMissionScore` reads the total with no map at all (008b911e..008b912a):
`MOV EDX,[EAX+0x21a0]; IMUL ESI,ESI,0x284; MOV EAX,[EDX+ESI*1+0x1e4]`.

Every keyed map is the case-insensitive `std::map<NativeString,int>` indexed by 005070C0
(`STL_CaseInsensitiveStringIntMap_Index`), which inserts a zero-valued entry on a miss; the
getters instead use 004C8B80 (`find`) and return 0 without inserting. Evidence for the
getter default, 0090c7d0..0090c7dc: `CMP ESI,EBX` against `_Myhead` at `[map+4]`, then
`XOR EAX,EAX; RET 8`; the hit path is `MOV EAX,[ESI+0x14]`, the mapped value of a node whose
8-byte `NativeString` key occupies `+0Ch..+13h`.

## The difficulty multiplier

Only the two **mission-score** accessors scale their argument. Both read a `std::vector<float>`
at `GlobalConfig()+2Ch` (`_Myfirst` at `+30h`, `_Mylast` at `+34h`) and index it with the
difficulty, forced to 2 in multiplayer. Evidence, 009103f0:

```
00910410  MOV EDI,0x2                        ; multiplayer difficulty
00910415  JNZ 0x0091041d                     ; taken when game+1FE4h != 0
00910417  MOV EDI,dword ptr [EAX + 0x6ac]    ; else the campaign difficulty
0091041d  CALL 0x00432650                    ; GlobalConfig singleton
00910436  CMP EDI,EAX / JC                   ; bounds check, else 00BF6713
0091043f  MOV EDX,dword ptr [ESI + 0x4]      ; _Myfirst
00910442  FILD dword ptr [ESP + 0x18]        ; the Lua value
00910449  FMUL float ptr [EAX]               ; * multiplier[difficulty]
0091044b  CALL 0x00bf7420                    ; __ftol, truncate toward zero
0091046b  MOV dword ptr [EAX],ESI            ; store
```

`00910480` differs only in the arithmetic, 009104ea..009104fa:
`FLD float[mult]; FIMUL dword[value]; FIADD dword[*slot]; __ftol; MOV [EBX],EAX`. The stored
value is added **unscaled** and the sum is truncated once, so `add` is
`trunc(multiplier * delta + current)` and not `current + trunc(multiplier * delta)`. The
add also indexes the map before computing, the set after; with `std::map::operator[]`
inserting a zero on a miss, both reach the same entry.

The bounds check treats the difficulty as an index into a vector whose length is not read
here, so the multiplier table's contents are `contract: unread`.

Both mission-score accessors and all eight plain score accessors clear a front-end cache byte
first (`MOV EAX,[0x00e198c4]; MOV ECX,[EAX+0xb8]; MOV byte ptr [ECX+0x14],0x0`). The two
medal setters (0090f0e0, 0090f290) do **not**, which is the only structural difference
between the medal rows and the score rows.

## Host table

One row per native call site the reconstruction leaves to an injected host. `this`, arguments
and return are the recovered ABI; the gate is the condition the site sits behind.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| 008b9273 | `[00e188a8]+18ECh` | `local_player_slot` | -/-/int | always |
| 008b927d | `[00e188a8]+1FE4h` | `is_multiplayer` | -/-/bool | always |
| 008b933a | `[00e188a8]+21A0h` | the manager `this` | -/-/ptr | always |
| 0091041d | 00432650 | `difficulty_score_multiplier` | -/int difficulty/float | mission score only |
| 00910417 | `[00e188a8]+6ACh` | `campaign_difficulty` | -/-/int | single player only |
| 009103fe | `[[00e198c4]+B8h]+14h` | `invalidate_front_end_score_cache` | -/-/void | not on the medal setters |
| 00910465 | 005070c0 | `score_map_entry` | map/key/int& | keyed set and add |
| 0090c7b3 | 004c8b80 | `score_map_find` | map/key/int | keyed get |

`00432650` is the `GlobalConfig` singleton and `[00e198c4]+B8h` the front-end object; neither
body was read here, so both are described as contracts rather than named for their behaviour
(rule 1).

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 008b9190 | `BSP_LuaScoring_SetMissionScore` | exported, analyzed, reconstructed, build-tested |
| 008b93c0 | `BSP_LuaScoring_GetMissionScore` | exported, analyzed |
| 008b95c0 | `BSP_LuaScoring_AddMissionScore` | exported, analyzed, reconstructed, build-tested |
| 008b97f0 | `BSP_LuaScoring_SetMissionMedal` | exported, analyzed, reconstructed, build-tested |
| 008b8ff0 | `BSP_LuaScoring_GetTotalMissionScore` | exported, analyzed |
| 008b9a30 | `BSP_LuaScoring_SetActionScore` | exported, analyzed, reconstructed, build-tested |
| 008b9c90 | `BSP_LuaScoring_GetActionScore` | exported, analyzed |
| 008b9e90 | `BSP_LuaScoring_AddActionScore` | exported, analyzed |
| 008ba0f0 | `BSP_LuaScoring_SetShipScore` | exported, analyzed, reconstructed, build-tested |
| 008ba350 | `BSP_LuaScoring_GetShipScore` | exported, analyzed |
| 008ba550 | `BSP_LuaScoring_AddShipScore` | exported, analyzed |
| 008ba7b0 | `BSP_LuaScoring_SetPlaneScore` | exported, analyzed, reconstructed, build-tested |
| 008baa10 | `BSP_LuaScoring_GetPlaneScore` | exported, analyzed |
| 008bac10 | `BSP_LuaScoring_AddPlaneScore` | exported, analyzed |
| 008bae70 | `BSP_LuaScoring_SetCommandScore` | exported, analyzed |
| 008bb0d0 | `BSP_LuaScoring_GetCommandScore` | exported, analyzed |
| 008bb2d0 | `BSP_LuaScoring_AddCommandScore` | exported, analyzed |
| 008bb530 | `BSP_LuaScoring_SetActionMedal` | exported, analyzed |
| 009103f0 | `BSP_MissionScoring_SetSlotMissionScore` | exported, analyzed, reconstructed, build-tested |
| 00910480 | `BSP_MissionScoring_AddSlotMissionScore` (existing) | exported, analyzed, reconstructed, build-tested |
| 0090f110 | `BSP_MissionScoring_SetSlotActionScore` | exported, analyzed, reconstructed, build-tested |
| 0090f140 | `BSP_MissionScoring_AddSlotActionScore` | exported, analyzed, reconstructed, build-tested |
| 0090f170 | `BSP_MissionScoring_SetSlotShipScore` | exported, analyzed, reconstructed, build-tested |
| 0090f1a0 | `BSP_MissionScoring_AddSlotShipScore` | exported, analyzed, reconstructed, build-tested |
| 0090f1d0 | `BSP_MissionScoring_SetSlotPlaneScore` | exported, analyzed, reconstructed, build-tested |
| 0090f200 | `BSP_MissionScoring_AddSlotPlaneScore` | exported, analyzed, reconstructed, build-tested |
| 0090f230 | `BSP_MissionScoring_SetSlotCommandScore` | exported, analyzed, reconstructed, build-tested |
| 0090f260 | `BSP_MissionScoring_AddSlotCommandScore` | exported, analyzed, reconstructed, build-tested |
| 0090f290 | `BSP_MissionScoring_SetSlotActionMedal` | exported, analyzed, reconstructed, build-tested |
| 0090c790 | `BSP_MissionScoring_GetSlotMissionScore` | exported, analyzed, reconstructed, build-tested |
| 0090c800 | `BSP_MissionScoring_GetSlotActionScore` | exported, analyzed |
| 0090c870 | `BSP_MissionScoring_GetSlotShipScore` | exported, analyzed |
| 0090c8e0 | `BSP_MissionScoring_GetSlotPlaneScore` | exported, analyzed |
| 0090c950 | `BSP_MissionScoring_GetSlotCommandScore` | exported, analyzed |

Coverage: `complete` for the record offsets and value rules of all ten setters, both
mission-score accessors and the five getters, which were read as full listings. The five
`Add*` and four `Get*` bindings whose accessor is a member of a family read end to end are
classified by their callee and their traced `LuaObject` call sequence rather than by a full
listing read; their prologue variant is in the trace, their record row is their accessor's.

## Follow-ups

- The `GlobalConfig+2Ch` multiplier vector: who fills it and how long it is. The bounds check
  is the only evidence here.
- `Scoring_GrantBonus` (008bb770), `Scoring_ClearPlayerScore` (008bc540),
  `Scoring_ClearAllMissionsScore` (008d2d60), `Scoring_SetConditionMessage` (008bbe00) and
  `Scoring_SetVictoryMessage` (008bc0c0) write outside the keyed maps and are `contract:
  unread` here; their callees (007fc9f0, 00915760, 007fd510, 0090bda0, 0090be30) are named in
  the report but not read.
- `Scoring_GetPlayerShotDown` (008bc9b0) and `Scoring_GetUnitTypeShotDown` (008d0140) read the
  kill trees at `+B4h`/`+C0h`, a different family.
