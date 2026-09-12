# The mission Lua bindings that decide `usn_2_java`

Addresses: 00898750 00897fb0 00898150 00898490 00898ac0 0088e9d0 008a7b00 0088d8e0 008a9320 0088c160 0088ba30 00929460 00923be0 0088a330 00926d90 00bd2f10 009290a0 00888aa0 008d1340 005b9ba0 008cd440 008bd340 008bd900 008cdd60 008ce510

Packet `cc_lua_binding_audit`, worktree `agent/cc-lua-binding-audit`. Ghidra was read-only: no
rename, comment, prototype or save was made from this packet. Every descriptive name here is a
hypothesis, not a recovered symbol.

`docs/LUA_BINDING_TABLE.md` exported the 560 rows of `00E0B7B8` and measured which rows the
installed scripts reach at all. This document asks a narrower question: of the rows the shipped
mission `Scripts/missions/usn/usn_2_java.lua` reaches, which ones decide what the mission does,
which of those the executable runs, and what the rest answer instead. It then reconstructs the
nine that decide it and runs the mission with them.

The reconstruction is `include/bsp/lua_binding_mission.hpp` and `src/lua_binding_mission.cpp`;
the executable's host is `src/game_hosts_script_orders.cpp`. The navigator family is
`docs/LUA_BINDING_NAVIGATOR.md`, the ten most-reached rows are `docs/LUA_BINDING_CORE.md`, the
entity return convention is `docs/LUA_BINDING_ENTITY.md`, and the think walk this packet drives
is `docs/ENTITY_THINK_DISPATCH.md`.

## What the mission script reaches

`local/audit_scan.py` takes the transitive closure of call sites from the mission file over the
Lua sources the mission loads (`Scripts/global/commandhelpers.lua`, `timetable.lua`,
`messagesender.lua`, `luamw_init.lua`, `fundamentals.lua` and the two files the mission
`DoFile`s) and intersects it with the exported binding table.

| Measure | Value |
| --- | --- |
| rows in the binding table | 560 |
| rows reachable from `usn_2_java.lua` | 179 |
| called from the mission file itself | 41 |
| reachable only through the helper library | 138 |
| called at least once in the 125 s run below | 28 |

## The audit

Every row the mission file calls directly, plus `SetWait` and `DeleteScript`, which it reaches
only through `luaDelay` and `luaTimetable` and which this packet reconstructs. The columns are
the handler, whether the executable runs a reconstructed body or keeps a record, the calls in the
125 s run, and what the row leaves on the Lua stack. The `pushes` column is the mechanical
push-site scan (`local/audit_returns.py`): it reports which push helper the body calls, not what
the value means. A row that pushes nothing answers the same thing as a record does, which is why
so much of this table is not a gap at all.

| Binding | Handler | State | Calls | Pushes | What the record answers today, and what the game's handler would |
| --- | --- | --- | --- | --- | --- |
| `random` | `0088C160` | reconstructed | 40 | number | was: nothing, so `luaRnd` returned nil and `luaInit` died at `commandhelpers.lua:1003`. Now: `trunc(uniform(min, max))` |
| `FindEntity` | `00898E30` | record | 36 | nil / entity slot | pushes `nil`; the game pushes `thisTable[<found entity>]`. The executable's own entity tail answers for the created scene instances (milestone 2l), so 36 of 36 resolve |
| `NavigatorSetAvoidLandCollision` | `008A3B10` | record | 28 | nothing | nothing either way; the difference is the navigator flag the body writes |
| `NavigatorSetTorpedoEvasion` | `008A3CD0` | record | 28 | nothing | as above |
| `CreateScript` | `00898750` | reconstructed | 27 | entity slot | was: nothing, and the named global was called once by a stand-in. Now: the script entity's own `thisTable` slot |
| `SetThink` | `00897FB0` | reconstructed | 27 | nothing | was: nothing happened. Now: the entity joins the pending think list |
| `SetWait` | `00898150` | reconstructed | 26 | nothing | was: nothing happened. Now: `+1E0h` and `+1DCh` are armed |
| `SetSkillLevel` | `00895250` | reconstructed | 22 | nothing | `docs/LUA_BINDING_NAVIGATOR.md` |
| `DeleteScript` | `00898AC0` | reconstructed | 25 | nothing | was: the timer entity never died. Now: killed with cause 2 and `Dead` set |
| `JoinFormation` | `00899D10` | reconstructed | 14 | nothing | `docs/LUA_BINDING_NAVIGATOR.md` |
| `NavigatorAttackMove` | `008A30D0` | reconstructed | 14 | nothing | `docs/LUA_BINDING_NAVIGATOR.md` |
| `SetInvincible` | `00897A50` | record | 10 | nothing | nothing either way; the difference is the invincibility field |
| `SetRoleAvailable` | `008AB850` | reconstructed | 4 | nothing | `docs/LUA_BINDING_NAVIGATOR.md` |
| `Blackout` | `008D1340` | record | 2 | nothing | nothing either way, and that is the point: the effect is the *callback name* it hands `005B9BA0`, which the game calls back when the fade ends. See "Where the mission stops" |
| `EnableMessages` | `008CFE40` | record | 1 | nothing | `docs/LUA_BINDING_CORE.md`, reconstructed there but not wired |
| `GameTime` | `008A9320` | reconstructed | 1 | number | was: nothing. Now: the mission clock in seconds |
| `GetDifficulty` | `008AE030` | record | 1 | number | pushes the difficulty; `docs/LUA_BINDING_CORE.md` reconstructed it, not wired here |
| `LoadMessageMap` | `008C61C0` | record | 1 | nothing | `docs/LUA_BINDING_CORE.md` |
| `MissionNarrative` | `008B0C10` | record | 1 | nothing | the narrative line; display only |
| `Music_Control_SetLevel` | `008C4D10` | record | 1 | nothing | `docs/LUA_BINDING_CORE.md` |
| `NavigatorMoveToRange` | `008A2F20` | reconstructed | 1 | nothing | `docs/LUA_BINDING_NAVIGATOR.md` |
| `Scoring_RealPlayTimeRunning` | `008B87F0` | record | 1 | boolean | pushes the flag back; `docs/LUA_BINDING_CORE.md` |
| `Scoring_SetFinalScoringFunctionName` | `008B8640` | record | 1 | nothing | `docs/LUA_BINDING_CORE.md` |
| `SetParty` | `008A8930` | record | 1 | number | `docs/LUA_BINDING_CORE.md` |
| `StartDialog` | `008B0540` | record | 1 | nothing | the dialogue queue; audio only |
| `GetHpPercentage` | `0088E9D0` | reconstructed | 0 | number | the unit's health fraction. Never reached this run: see "Where the mission stops" |
| `GetPosition` | `008A7B00` | reconstructed | 0 | new table | `{x, y, z}` from the world matrix. Never reached this run |
| `GetMeasure` | `0088D8E0` | reconstructed | 0 | globals value | `globals.kilometer` or `globals.mile`. Never reached this run |
| `GetSelectedUnit` | `008AB070` | record | 0 | nil / entity slot | pushes `nil`; the game pushes the selected unit's slot |
| `AddDamage` `AddPowerup` `DisplayScores` `DisplayUnitHP` `Effect` `ExplodeToParts` `FillPathPoints` `GenerateObject` `HideScoreDisplay` `HideUnitHP` `SetSelectedUnit` `Sink` `TorpedoEnable` | | record | 0 | mostly nothing; `Effect`, `FillPathPoints` and `GenerateObject` push | reached only from the mission's phase 1 and 2 bodies, which this run does not enter |

The 138 rows reachable only through the helper library are in `local/audit_scan.json`. Two of
them matter to this document and neither answers anything: `Objectives_Add` `008CD440`,
`Objectives_Completed` `008BD340`, `Objectives_Failed` `008BD900`, `Objectives_AddUnit`
`008CDD60` and `Objectives_RemoveUnit` `008CE510` all marshal into the objective set
(`008E1F80`, `008E20D0`, `008E2200`) and return zero results. See the correction below.

## What actually decides the mission

Reading `usn_2_java.lua` end to end, the mission's flow is four decisions, and only two of them
are made by a binding at all.

1. **When the objective checker runs again.** `luaCheckObjectives` ends with
   `luaDelay(luaCheckObjectives, MissionObjectiveCheckerDelay)` (`usn_2_java.lua:603`;
   `MissionObjectiveCheckerDelay` is 5 in `Scripts/global/luamw_init.lua`). `luaDelay` is
   `CreateScript("luaDoTimeTable", timer, paramTable)`, and `luaDoTimeTable`
   (`Scripts/global/timetable.lua`) does `SetThink(this, "luaTimetable")` then `SetWait(this, t)`;
   when the wait expires the think walk `00929460` calls `luaTimetable`, which calls the delayed
   function and then `DeleteScript(this)`. Five bindings and one walk. With any of them a record
   the checker runs exactly once for the whole mission.
2. **Phase 1 closes** when `GetHpPercentage(Mission.DeRuyter) < 0.15` or every ship in
   `Mission.EnemyDestroya` has `Dead` set (`usn_2_java.lua:531-545`).
3. **Phase 2 closes, and the mission is won,** when
   `luaGetDistance3D(Mission.CATable[1], Mission.EscapePoint) < 500` (`usn_2_java.lua:562-579`).
   `luaGetDistance3D` is `GetPosition` twice and `math.sqrt` in Lua.
4. **The mission is lost** when `Mission.Houston.Dead or Mission.Exeter.Dead`
   (`usn_2_java.lua:521`).

The objective state itself is not native. `Mission.Objectives[level][num].Active` and `.Success`
are Lua fields that `luaObj_Add`, `luaObj_Completed`, `luaObj_Failed`, `luaObj_IsActive` and
`luaObj_GetSuccess` read and write in `commandhelpers.lua` (lines 5741-6027). `Dead` is native,
but it is a *field*, not a binding: `00928A00` seeds `thisTable[key].Dead = false` and `00929800`
sets it true on the entity's on-killed dispatch.

So the reconstruction order is the scheduler first, then the three queries, and `random` before
either because `luaInit` cannot finish without it.

## The nine routines

Every one is `__fastcall(lua_State* in ECX)` with the result count in EAX, the shared machine
prologue `00B66C00` / `00B679B0` and epilogue `00B66400` / `00B669A0`, and a guarded `luakod`
static-local log category that has no per-call effect and is not modelled, exactly as
`docs/LUA_BINDING_CORE.md` records for its own ten.

### `CreateScript` `00898750`

`00898834 PUSH 0x1E4` into `00BF55BE`, zeroed by `00BF79F0` at `0089884F`, constructed by
`00928630` at `0089886F`. Four words are written at `00898874` (`+0h`), `0089887A` (`+10h`),
`00898881` (`+24h`) and `00898888` (`+170h`), then `00898892 MOV [ESI+0C4h],EBX` with `EBX = 3`
from `0089885D`. (EBX held the incoming `lua_State` until the `PUSH` at `0089880B`; the listing's
only other EBX writes are `00898771` and that push, so the `3` is unambiguous.) `0089892C` calls
the entity's vtable `+98h` with a sixteen-float matrix staged at `008988BA`-`00898914` from
`00D7A24C = 1.0f` and zero, and `[game+19CCh]`; `00898932` calls `00927610` with `DL = 0`.

Then the part that matters to the scheduler. `00898940` asks the machine for the stack top and
`00898945 CMP EAX,1` / `JLE` leaves `EDI` at the zero set at `0089893E`, otherwise `0089894A MOV
EDI,2`. That `EDI` is `stack_first` of `0089898B CALL 009290A0`, whose `stack_last` is the `-1`
pushed at `00898978`. Per `docs/MISSION_NAMED_CALL_ARGS.md` that forwards
`BSP_MissionLuaHost_CallNamedThreadSafe(self_key = entity+178h, name, 0, stack_first, -1)`, and
the self-key block pushes `thisTable[self_key]` as argument 1. So `CreateScript("luaInit")` calls
`luaInit(this)` and `CreateScript("luaDoTimeTable", timer, params)` calls
`luaDoTimeTable(this, timer, params)`. That is how `luaDelay`'s two tables arrive.

The return value is the same entity tail `docs/LUA_BINDING_ENTITY.md` describes: `008989C7`
formats the u16 at `+174h` through `004260B0`, `008989F6` fetches `thisTable` (`00CE7494`) with
`00B67910`, `00898A0C` indexes it with `00B678E0` and `00898A1B` pushes with `00B663D0`. One
result.

### `SetThink` `00897FB0`

Already reconstructed as `bsp::lua_binding_set_think` (`docs/LUA_BINDING_CORE.md`). Its one
callee is `008980E8 CALL 0088A330`, whose `0088A34B` appends to the pending think list only on
the null-to-name edge (`docs/ENTITY_THINK_DISPATCH.md`). This packet supplies that callee for a
script entity rather than writing a second copy of the binding.

### `SetWait` `00898150`

Entity at argument 0 (`0089824F` / `00888AA0`), seconds at argument 1 (`00898280` / `00B66270`)
narrowed to float32 by the `FSTP` at `00898285`. `00898289 FLD [00CE3800]` loads `0.5f`,
`00898293 FCOMIP` compares, and the `JBE` at `00898297` takes the `0.5f` when the request is at
or below it: the delay is `max(requested, 0.5f)`, which is `bsp::clamp_think_delay`. Stored by
`008982C9 MOVSS [ESI+1E0h]` with `008982D1 MOV byte [ESI+1DCh],1`. No result.

### `ClearThink` `00898490`

Entity at argument 0 (`0089858F` / `00888AA0`). `008985A6 MOV EAX,[ESI+1D8h]`; when non-null,
`008985B1 CALL 00BF6989` frees it. The nine bytes at `008985B6`-`008985BE` are undisassembled in
Ghidra because of the `CALL_RETURN` override on the CRT free helper; the disk image has
`83 C4 04 89 AE D8 01 00 00`, which is `ADD ESP,4` then `MOV [ESI+1D8h],EBP` with `EBP` zero
since `008984B3`. Then `008985C3 MOV byte [ESI+1DCh],0`. So it frees the name, nulls the slot and
disarms the delay. No result.

### `DeleteScript` `00898AC0`

Entity at argument 0 (`00898BC2` / `00888AA0`). `00898BD9 CMP byte [ESI+5Eh],0`: a set byte takes
`00898C00 XOR EAX,EAX`, returning zero without even asking for the result count; a clear byte
takes `00898C07 CALL 00926D90` with the cause pushed at `00898C04`. `EBX` is `2` from
`00898BA3` (it held the `lua_State` until the push at `00898B7A`), and
`docs/UNIT_DAMAGE_AND_DEATH.md` names the second parameter of `00926D90` the kill cause. Both
arms leave no result.

### `GetHpPercentage` `0088E9D0`

Entity at argument 0 (`0088EACF` / `00888AA0`), then `0088EAE8 CALL 00923BE0` and
`0088EAF5 CALL 00B66480`. One result, a float. `00923BE0` is already reconstructed: zero while
`+5Dh` is set, otherwise the virtual at the unit's vtable `+110h` floored at zero and clamped to
`00D7A24C = 1.0f`, with the clamped value cached at `+164h`. The value is therefore a fraction,
which is why the script compares it with `0.15`.

### `GetPosition` `008A7B00`

Entity at argument 0 (`008A7BFF` / `00888AA0`); a fresh table from `008A7C1F` / `00B67930`;
`008A7C24 CMP byte [ESI+0C8h],0` runs `008A7C37` / `00414DB0` `BSP_EntityPose_RefreshWorld` only
while the flag is clear; then `008A7C3C LEA EDX,[ESI+0FCh]` and `008A7C46 CALL 0088BA30`.
`+0CCh` is the world matrix (`00414DB0`'s own copy target) and `+30h` is its translation row, so
`+0FCh` is the world position. `0088BA30` writes three fields from `src[0..2]` with the key
literals `00CEB488` `x`, `00D045F8` `y` and `00CFD718` `z`. One result.

### `GetMeasure` `0088D8E0`

No argument. `0088D9BD CMP byte [00F88988],0`; the `JZ` arm pushes the globals value at
`globals.kilometer` and the fall-through the one at `globals.mile` (`00CF5914`), both through
`00B672B0`. One result. It answers the localised unit *name* out of the globals table, not a
literal of its own, which is why the script concatenates it after a formatted distance.

### `GameTime` `008A9320`

No argument. `008A93FE FLD float [00F876A4]`, `008A9414 CALL 00B66480`. One result. `00F876A4` is
written by `0087464C` inside `00874640`, which also fills the minute and fractional fields at
`+4h`, `+8h`, `+0Ch` and `+10h` from the same value.

### `random` `0088C160`

`0088C24A` takes the argument count and `0088C251` / `0088C25A` / `0088C263` select the arm:

| Arguments | Site | Bounds handed `00BD2F10` |
| --- | --- | --- |
| 0 | `0088C377` | `0.0f`, `00D11318` = `32767.0f` |
| 1 | `0088C339` | `0.0f`, `(float)(a + 1)` |
| 2 | `0088C2C7` | `(float)a`, `(float)(b + 1)` |
| 3 or more | none | `0088C263 JNZ 0088C38B`, nothing is pushed |

In the two-argument arm the upper bound is read first (`0088C274` is argument 1, `0088C28E`
argument 0), so the range is `[a, b]` inclusive. Each result goes through the CRT float-to-long
`00BF7420` and is pushed by `00B664B0`. `00BD2F10` `BSP_Random_UniformFloatRange` is already
reconstructed; this packet calls it, it does not restate it.

## The host, in call order

Every method of `bsp::LuaBindingMissionHost` with the native call site it stands for. The
containing function is the packet routine in each case. `src/game_hosts_script_orders.cpp`
implements them over this process's created instances and its own script entities.

| Host method | address | native | Contract |
| --- | --- | --- | --- |
| `script_entity_create_00898841` | `00898841` | `00bf55be` | `operator new(0x1E4)`; the zero fill is `0089884F` / `00BF79F0` and the construct `0089886F` / `00928630` |
| `script_entity_vcall_98_0089892c` | `0089892c` | vtable `+98h` | unread: a virtual with no resolved concrete vtable. Named by slot |
| `script_entity_call_00927610_00898932` | `00898932` | `00927610` | body not read; called with `DL = 0`. Named by address |
| `lua_stack_top_00b65eb0` | `00898940` | `00b65eb0` | `BSP_LuaStateOwner_GetTop` |
| `entity_call_named_009290a0` | `0089898b` | `009290a0` | `BSP_MissionEntity_RegisterLuaScript`; forwards the self key at `+178h` |
| `push_self_table_slot_008989f6` | `008989f6` | `00b67910` | `thisTable`, then `00B678E0` at `00898A0C` and `00B663D0` at `00898A1B` |
| `entity_set_think_script_name_0088a330` | `008980e8` | `0088a330` | `BSP_Entity_SetThinkScriptName`; body read for the pending-list edge only, and the doc says so |
| `entity_arm_think_delay_008982c9` | `00898280` | `00b66270` | the delay read that precedes the store at `008982C9` |
| `entity_clear_think_name_008985a6` | `008985b1` | `00bf6989` | `_free` on the think name; the null store follows at `008985B9` |
| `entity_flag_5e_00898bd9` / `entity_kill_00926d90` | `00898c07` | `00926d90` | `BSP_MissionEntity_Kill`, already reconstructed |
| `unit_health_gate_5d_00923be4` etc. | `0088eae8` | `00923be0` | `BSP_UnitInstance_GetHealth`, already reconstructed; its vtable `+110h` call is unread |
| `push_number_float_00b66480` | `0088eaf5`, `008a9414` | `00b66480` | the float number push |
| `entity_pose_refresh_00414db0` | `008a7c37` | `00414db0` | `BSP_EntityPose_RefreshWorld`, already reconstructed |
| `push_vector3_table_0088ba30` | `008a7c1f`, `008a7c46` | `00b67930`, `0088ba30` | the fresh table and the three keys |
| `push_global_path_value_00b672b0` | `0088d9ea` | `00b672b0` | pushes the globals value at a dotted path |
| `random_uniform_00bd2f10` | `0088c377`, `0088c339`, `0088c2c7` | `00bd2f10` | `BSP_Random_UniformFloatRange`, already reconstructed |
| `LuaBindingResultWriter::push_number` | `0088c386` | `00b664b0` | the integer push, after `00BF7420` |

What the host supplies rather than recovers, each labelled at its site in the source:

- **the health** (`unit_health_vtable_110_00923bf6`). No created instance carries a health field
  and no concrete unit vtable is resolved, so the virtual at `+110h` is a record and
  `GetHpPercentage` answers the zero the rule then returns. This is the one query whose value is
  still not the game's.
- **the random stream**. `00BD2F10` draws from the thread's random state object (`00BD2ED0`);
  this process has no such object, so the draw is a fixed-seed linear congruential sequence. The
  distribution and the bounds are the native's; the sequence is not.
- **the script entity itself**. `GameScriptEntity` keeps only the fields the five script bindings
  and `00929460` read. The 0x1E4-byte native block has no other reader here, and creation stops
  at 512 entities because the records are handed to Lua as light userdata and must not move.
- **the timing of the kill's list erase**. `luaTimetable` calls `DeleteScript(this)` from inside
  its own think, so the erase reaches the live list while the walk is on it. The native unlinks a
  node under a cursor that captured its successor at `0092948B`; `bsp::run_entity_think_list_00929460`
  walks a vector, so the host holds the erase until the walk returns. The end state is the same
  list. Applying it immediately is a real bug and was one: before the fix the same run reported
  81 heartbeats and 106 fires instead of 41 and 66.

## Coverage

| Routine | Address | Coverage |
| --- | --- | --- |
| `lua_binding_create_script` | `00898750` | complete for the binding's own work. The sixteen-float matrix staged at `008988BA`-`00898914` is passed to one host call and its element meanings are not modelled |
| `lua_binding_set_wait` | `00898150` | complete |
| `lua_binding_clear_think` | `00898490` | complete, including the `008985B6`-`008985BE` bytes read from the disk image |
| `lua_binding_delete_script` | `00898AC0` | complete, both arms |
| `lua_binding_get_hp_percentage` | `0088E9D0` | complete |
| `lua_binding_get_position` | `008A7B00` | complete |
| `lua_binding_get_measure` | `0088D8E0` | complete |
| `lua_binding_game_time` | `008A9320` | complete |
| `lua_binding_random` | `0088C160` | complete, all four arms |
| `unit_health_value_00923be0` | `00923BE0` | the value rule only; that routine's own record is the reconstruction |

## Run-time evidence

```
build/win32/Release/bsp_game.exe --frames 2700 --press-start-frame 30 --menu-select USN02
  --mission-frames 2500 --mission-frame-seconds 0.05 --log local/audit_long.log
  --xlive-dll build/win32/Release/xlive_stub.dll
  --game-root "I:/SteamLibrary/steamapps/common/Battlestations Pacific"

summary mission script timers created=27 think_registrations=27 waits=26 clears=0
        deletes=25 passes=2500 fires=66 failures=0
  script entity 100000 created_for=luaInit         think=lua_Think     armed=0 thinks=41 dead=0
  script entity 100001 created_for=luaDoTimeTable  think=luaTimetable  armed=1 thinks=1 dead=1
  ... 24 more of the same, then
  script entity 100026 created_for=luaDoTimeTable  think=luaTimetable  armed=1 delay=3.75 thinks=0
host methods 622 concrete, 464 unimplemented
```

125 simulated seconds. `luaStageInit` called `CreateScript("luaInit")`; `luaInit` ran to the end
for the first time (before `random` it died at `commandhelpers.lua:1003`, where `luaPickRnd`
compares `luaRnd()` with a number); `SetThink(this, "lua_Think")` registered the mission's own
entity, which then took the untimed three-second heartbeat 41 times, the 125 s / 3 s the
countdown rule predicts; and `luaCheckObjectives` rescheduled itself 25 times at the authored
five-second interval, each time through a fresh `luaDoTimeTable` entity that armed a wait, fired
once and deleted itself. 41 + 25 = the 66 fires. No Lua call failed.

Before this packet the same line produced one `luaInit` call from milestone 2l's stand-in, no
script entities, no waits and no reschedules.

### Where the mission stops

**No objective updated and the script reached neither `MissionComplete` nor `MissionFailed`.**
`Objectives_Add` was never called, and neither were `GetHpPercentage`, `GetPosition` or
`GetMeasure`, because `luaCheckObjectives` returns before every one of its tests while
`Mission.MissionPhase` is 0 (`usn_2_java.lua:497`).

`MissionPhase` becomes 1 only in `luaIn`, and the only route to `luaIn` is a `Blackout`
completion callback: `lua_Think` calls `luaIntroMovie`, which calls
`luaIngameMovie(..., bo = true)`, whose first act is `Blackout(true, "luaIngameMovieBOStart", 1)`
(`commandhelpers.lua:7747` onward); the camera movie then ends in `luaIntroMovieEnd`, which is
`Blackout(true, "luaIn", 3)`. `Blackout` `008D1340` reads the callback name at argument 1, the
duration at argument 2 and the alpha at argument 3, then calls
`005B9BA0(this = [[00E198C4]+0A4h], alpha, duration, &name)`, which stores
`duration + [00D7A268]` at `this+0C8h`, the alpha at `+0C4h` and the name at `+0CCh`, then calls
`005B9800`. What reads `+0C8h` and calls back the name at `+0CCh` was not read by this packet and
is follow-up 1; nothing in this process advances that object, so the name is never called back.
`Blackout` ran twice in the run and that is exactly where the mission sits.

This is a real answer, not a missing measurement: the two remaining gates between the executable
and a mission that plays itself are the fade object's update and the camera-movie listener, not
the objective or victory bindings.

## Corrections

- **`Objectives_*` do not answer the script's objective queries.** The brief ordered this packet
  by "the record bindings the script depends on for objectives, victory/defeat and unit state".
  `Objectives_Add` `008CD440`, `Objectives_Completed` `008BD340`, `Objectives_Failed` `008BD900`,
  `Objectives_AddUnit` `008CDD60` and `Objectives_RemoveUnit` `008CE510` push **no result**; they
  marshal into the objective set at `008E1F80` / `008E20D0` / `008E2200`, which is the display.
  The state the script branches on is `Mission.Objectives[level][num].Active` and `.Success`,
  plain Lua fields (`commandhelpers.lua:5741-6027`). Reconstructing them would change what the
  player sees and nothing the script decides.
- **The scheduler is upstream of all of it.** Was: the queries decide the mission. Is: the
  queries are never reached unless `CreateScript` / `SetThink` / `SetWait` / `DeleteScript` and
  the think walk run, because the checker re-enters only through `luaDelay`. Evidence:
  `usn_2_java.lua:603`, `Scripts/global/timetable.lua`, and the 25 reschedules above against
  zero before.
- **`config/lua_bindings.json`'s `ghidra_function` column is stale.** Was:
  `docs/LUA_BINDING_TABLE.md` counts 24 handlers with no Ghidra function, among them `AddPowerup`
  `008EE410` and `Spawn` `00944680`. Is: both have a function today, bodies
  `008EE410`-`008EE5AA` and `00944680`-`00944FC0` (`python tools/bsp.py ghidra proto <a> --brief`).
  The exported table is a snapshot; regenerate it with `tools/lua_bindings_export.py` before
  quoting that column.
- **Milestone 2l's created-script stand-in is superseded.** Was:
  `GameMissionLuaHost::run_created_scripts` called each name `CreateScript` was handed, once.
  Is: `00898750` calls the named global itself at `0089898B`, so the stand-in would run `luaInit`
  a second time. `src/game_hosts_mission_frame.cpp` now runs it only while no script entity has
  been created. `src/game_hosts_lua.cpp` is leased to another owner and was not edited; when it
  is free, the `note_created_script` bookkeeping in `binding_trampoline` can go.

## Follow-up packets

1. **`blackout_fade_callback`** (`008D1340`, `005B9BA0`, `005B9800`, the object at
   `[[00E198C4]+0A4h]`, `0076D310`). What advances `+0C8h`, and where the name at `+0CCh` is
   called back. This is the single gate between the current run and `Mission.MissionPhase = 1`.
   Its other three callers are `0045DB40`, `004660B0` and `0076F4C0`.
2. **`ingame_movie_listener`** (`AddListener` `008C6760`, `RemoveListener` `008C6990`,
   `IsListenerActive` `008C6BB0`, and the camera-movie completion that calls
   `luaIngameMovie`'s `cb`). The second gate, immediately after the first.
3. **`objective_set_display`** (`008CD440`, `008BD340`, `008BD900`, `008CDD60`, `008CE510` over
   `008E1F80` / `008E20D0` / `008E2200` / `008DF5D0` / `008DF9B0`). Not a flow gate; it is what
   makes the objective list on the HUD real, and `docs/OBJECTIVE_UNIT_LIST.md` and
   `docs/MISSION_RESULT_DECISION.md` already have the set side.
4. **`unit_health_accessor`** (the unit vtable `+110h` that `00923BE0` calls). Until it is read,
   `GetHpPercentage` answers zero and phase 1's first test cannot be exercised.
5. **`entity_dead_field`** (`00929800`, `00929B60`, `0077D270`). The `Dead` field every
   `luaRemoveDeadsFromTable` in the mission reads. `00929800`'s body is read here only as far as
   the `Dead` write; the `Message` and `KillReason` fields it also sets were not read.

## no_ghidra_function

None. Every routine this packet read has a Ghidra function, and the one listing gap
(`008985B6`-`008985BE`, inside `00898490`) is a `CALL_RETURN` flow override on the CRT free
helper, not a missing function; the bytes were read from the disk image and are quoted in the
`ClearThink` section above. `tools/ghidra_flow_repair.py 00898490 --apply` would close it.
