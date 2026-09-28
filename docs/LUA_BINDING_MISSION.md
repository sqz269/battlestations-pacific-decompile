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

## FillPathPoints, 0089A190 (packet `cc9_fill_path_points`, `kFillPathPointsBound`, committed OFF)

Worker cc9-ships2, on main `002675288`. Ghidra was read-only. The switch is in
`include/bsp/game_hosts_script_orders.hpp`.

### The read

0089A190 is the `FillPathPoints` row of the binding table. Ghidra has no callers for it; it is
reached only through the table. It is `lua_CFunction`-shaped (`__fastcall(lua_State*)`), with
the usual "luaMW_FillPathPoints failed:" error prefix and the "luakod" static.
1. Argument 0 goes through 00888AA0 (the entity handle from its Lua table), then 007AC9D0
   BSP_Entity_PathInterfaceForKind.
2. 00B67930 BSP_LuaObject_NewTable makes the result table (0089A2B7).
3. For `i` from 0 while `i < 00415870(path)` (the point count):
   - 00B66670 and 00B67720 (BSP_LuaObject_GetByIndex) make entry `i + 1`, and 00B67700
     destroys the temporary (0089A317);
   - 007AF800 BSP_ScenePath_TransformPointToWorld gives point `i` in world space;
   - 0088BA30 writes it as the fields `x`, `y` and `z` (0089A2E7..0089A32D).
4. The table is the one result.

The host already has these points. At scene load, `retain_path_points` keeps each Path's
`PathPoints/Point%02i/Pos` (007B352E..007B3604). The scene path registry holds them after the same
007AF800 step, and NavigatorMoveOnPath already looks paths up there by name.

### Callers in this installation's Lua (read-only)

| file | line | reached? |
| --- | --- | --- |
| bsm_01_stationed_at_pearl.lua | 1833, `luaGenerateHarborTrafic`, called from `luaInit` (607) | yes, at init, over pt_path1..4 |
| bsm_01_stationed_at_pearl.lua | 1571, `luaGeneratePanicTraffic`, called from `luaMoveToPh2` (1178) | phase 2 only |
| usn_2_java.lua | 867, `luaShowPath` | no caller; USN02's OFF log has no FillPathPoints row |
| usn_1_marshall.lua | 1057 | not a reference mission |
| commandhelpers.lua | 6827, 9209, 9251, 12389, 12401 | helpers; USN02 and USN04 OFF logs have no FillPathPoints row |

**Why BSM01 stops today.** With the native unimplemented, `pathTbl` is nil.
- `luaInit` fails at line 1857 (`sst_off_bsm01.log`: "attempt to index local pathTbl").
- So `SetThink(this, "lua_Think")` (line 608) never runs: the timer census shows
  `think_registrations=0 failures=1`.
- Therefore `luaStartMission` never runs either.

BSM01's four PT paths in the scene, with the host's retention lines agreeing:

| path | points |
| --- | --- |
| pt_path1 | 16 |
| pt_path2 | 11 |
| pt_path3 | 18 |
| pt_path4 | 12 |

### The binding (under `kFillPathPointsBound`)

- The script-orders host handles the native.
  - It resolves argument 0 as NavigatorMoveOnPath does: the marker registry first, then the name.
  - It answers a new table whose entry `i + 1` is the registry's world point `i`, as `{x, y, z}`
    (the same helper GetPosition uses for 0088BA30).
- **LABELLED:** an argument that is not a retained path answers an empty table. The image
  dereferences whatever 007AC9D0 returned.
- **Census:** `summary mission script fill path points bound=.. calls=.. empty=..`, and one
  `FillPathPoints(<path>) points=..` line per call.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

| row | prediction |
| --- | --- |
| USN02 `pair_diff` | 1, `calls=0` on both sides |
| USN04 `pair_diff` | 1, `calls=0` on both sides |
| BSM01 FillPathPoints lines, ON | at least four; the first four are pt_path1..4 with points 16, 11, 18 and 12; `empty=0` |
| BSM01 `luaInit` | no failure on ON: the timer census shows `think_registrations` at least 1, where OFF shows 0 with `failures=1` |
| BSM01 `luaStartMission` | reached on ON: the native table's `ShipSetTorpedoStock` row shows `calls=1` (an unimplemented record, since `kShipSetTorpedoStockBound` is still OFF), where OFF has no row |
| BSM01 script failures after init | none, so `failures=0` on ON |
| BSM01 `pair_diff` | 3: the PT boats and rescue craft are generated and put on the paths only on ON |

### The pairs, measured, and the verdict

- OFF is this tree's `build\` at `215640b92`; ON is `pair_export --flip kFillPathPointsBound=true`
  of the same commit (SHA-256 `067803BF3411`).
- All sides ran with the streams and the death table on. Logs: `local\fpp_{off,on}_{bsm01,usn02,usn04}.log`.
- Every log was checked for its milestone line, its module directory and its final COM release
  line.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 `pair_diff` | - | 1, `calls=0` both sides | 1 | holds |
| USN04 `pair_diff` | - | 1, `calls=0` both sides | 1 | holds |
| BSM01 FillPathPoints lines | none | pt_path1 16, pt_path2 11, pt_path3 18, pt_path4 12; `calls=4 empty=0` | the first four as listed, `empty=0` | holds |
| BSM01 think registrations | 0, `failures=1` | 3 | at least 1 | holds |
| BSM01 `ShipSetTorpedoStock` row | none | `UNIMPLEMENTED calls=1` | `calls=1` | holds |
| BSM01 script failures after init | - | **149**, every one at line 683 | none | **failed** |
| BSM01 `pair_diff` | - | 3 (the controlled unit is HenryPT; 201 native rows added) | 3 | holds |

**The next blocker is `Scoring_GetPlayerShotDown` (008BC9B0).**
- bsm_01_stationed_at_pearl.lua:681 reads `local sd = Scoring_GetPlayerShotDown()`, and line 683
  compares `sd > 0`.
- The native is unimplemented here and answers nothing, so every `lua_Think` pass from the first
  one on stops at 683 (149 calls, 149 failures).
- `luaStartMission` (line 650) runs before that point, on the first pass, so the ShipSetTorpedoStock
  call is reached.
- `PutTo` (008A9F90, 8 calls) is also unimplemented. The PT boats and rescue craft are generated
  and given their paths through NavigatorMoveOnPath (10 calls), but they are not placed at the
  path points.

**Verdict: `kFillPathPointsBound` ON.** The native answers the image's table on every call, the
reference missions are identical, and BSM01 now loads its script and runs `luaStartMission`.
BSM01's think still needs `Scoring_GetPlayerShotDown` and `PutTo`; that is a separate packet.

## Scoring_GetPlayerShotDown and PutTo, BSM01's think natives (packet `cc9_bsm01_think_natives`, committed OFF)

Worker cc9-ships2, on main `c247b2476`. Ghidra was read-only. The two switches are
`kScoringPlayerShotDownBound` and `kPutToBound` in `include/bsp/game_hosts_script_orders.hpp`.

### Scoring_GetPlayerShotDown, 008BC9B0

`docs/SCORING_BODIES.md` section 6 has the read.
- The native takes the slot from Lua argument 0 as an integer, and 0 when there is none
  (008BCAA7..008BCAAE).
- It walks record `slot*284h+4h`'s tree at `+B4h`, level-1 key 1 (ENEMY), every level-2 key, and
  sums the level-3 keys 7..0Ch, the six aircraft classes.
- The tree is written by 0091BDA0 at a kill. It goes to the credited player slot `[unit+2D8h]`,
  into `+B4h` when that equals the originating slot `+2DCh`.

**The binding** counts from the gunnery host's death rows. It takes a sunk unit of the plane
family whose `killed_by` is the controlled unit and whose side is ENEMY to it (00803510). Other
slots answer 0.

**LABELLED:** this process keeps no scoring record and no per-unit player slot. The controlled
unit stands for slot 0, and "the six aircraft classes" is the plane family (`IsKindOf(0Fh)`),
since the unit class ids here (10h..17h for planes) are not the scoring enum.

### PutTo, 008A9F90

- The native reads argument 0 through 00888AA0 and argument 1 through 00888760 as a Vector3. With
  three arguments it reads argument 2 as degrees and turns it to radians (`* pi / 180`,
  008AA136..008AA14D).
- 007788B0 is called when the entity answers `IsKindOf(2)`; its answer is unused (008AA0BF).
- Single player (`[00E188A8]+1FE4h == 0`, 008AA164..008AA16F) calls the entity's `vtable[118h](pos)`,
  then `vtable[11Ch](heading)` when a heading was given. Otherwise the placement goes out as a
  session message (00888F30 or 007EDF00, then 0077C2A0 at 008AA19D / 008AA1C7).
- `vtable[118h]` is 008193A0 SetWorldPosition, read in `docs/SHIP_ESCORT_SCREEN.md` section 1.
- `vtable[11Ch]` is **008196B0..0081983A, no Ghidra function**. It starts after 008193A0's `RET 4`
  at 008196A8 and INT3 padding 008196AB..008196AF, and ends with `RET 4` at 00819838 and INT3 from
  0081983B. It is unread here.

**The binding:**
- `GameUnitsHost::place_at_world_position_008193a0` is 008193A0's arm for a unit that is not a
  formation follower: the position store 009583C0 and the wake refill 00818EA0 along the current
  heading.
- **LABELLED:**
  - a follower is not placed;
  - a leader's group snap is recorded, not applied;
  - `vtable[0D8h]` (00955970, the scene-node refresh) and `unit+0BCCh = 1.25` have no host
    counterpart;
  - the heading argument is recorded, not applied.

### Callers in this installation's Lua (read-only)

- **BSM01:**
  - line 681 calls Scoring_GetPlayerShotDown on every `lua_Think` pass, and line 683 compares the
    result with 0;
  - lines 1849..1858 call PutTo on the four PT boats and four rescue craft (two arguments);
  - the OFF log (`fpp_on_bsm01.log`, FillPathPoints ON) shows `PutTo calls=8` and
    `Scoring_GetPlayerShotDown calls=149`, both unimplemented.
- **USN02:** calls neither.
- **USN04:** usn_19_coralus.lua calls PutTo at 1242..2253, but no USN04 log has a PutTo row, so
  none of those arms runs in the harness.

### Predictions (written before the pairs; streams and the death table on)

The pairs are three sides of one commit:
- OFF;
- `S`, only `kScoringPlayerShotDownBound=true`;
- `SP`, both switches true.

BSM01 runs at 9200/9000; USN02 and USN04 run OFF against `SP`.

| row | prediction |
| --- | --- |
| USN02 / USN04 `pair_diff` (OFF vs SP) | 1; `shot down calls=0` and `put to calls=0` on both |
| BSM01 OFF vs S: `lua_Think` failures at line 683 | 149 -> 0 |
| BSM01 OFF vs S: shot-down calls | at least 149 (one per think pass); `last` is 0 or small |
| BSM01 OFF vs S: the timer census | `failures` at most 1 on S (any later stop is a different line, reported by name) |
| BSM01 OFF vs S: `pair_diff` | 3, since the think now runs past line 683 and its phases proceed |
| BSM01 S vs SP: `put to` | `calls=8 placed=8`: PT-21..PT-24 (the pt_path1..4 boats of `luaGenerateHarborTrafic`) and the four rescue craft, each at a point `pathTbl[random(2,10)]` of its path, y 0 |
| BSM01 S vs SP: `pair_diff` | 3, since the eight craft start at their path points instead of their generated positions |
| BSM01 deaths / hit records | predicted to move on both pairs; OFF has 0 and 0 |

### The pairs, measured, and the verdicts

- Three builds of `9df07aa60`:
  - OFF, this tree's `build\`;
  - `S`, `pair_export --flip kScoringPlayerShotDownBound=true`, SHA-256 `01B64C27E428`;
  - `SP`, both flips, SHA-256 `542464D48C01`.
- All runs had the streams and the death table on. Logs: `local\bt_{off,s,sp}_bsm01.log` and
  `local\bt_{off,sp}_{usn02,usn04}.log`.
- Every log was checked for its milestone line, its module directory and its final COM release
  line.

| row | OFF | S | SP | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| USN02 `pair_diff` (OFF vs SP) | - | - | 1; both natives `calls=0` | 1 | holds |
| USN04 `pair_diff` (OFF vs SP) | - | - | 1; both natives `calls=0` | 1 | holds |
| BSM01 `lua_Think` failures at line 683 | 149 | 0 | 0 | 149 -> 0 | holds |
| BSM01 shot-down calls / last | 0 | 149 / 0 | 149 / 0 | at least 149, last 0 or small | holds |
| BSM01 timer census | `failures=149` | `failures=0`, 4 think registrations | the same | at most 1 failure | holds |
| BSM01 script work | script calls 621, dialogs started 1 | 2545, 3 | the same | the think runs past 683 | holds |
| BSM01 `pair_diff` OFF vs S | - | 1, gameplay identical | - | 3 | **failed** |
| BSM01 `put to` | - | `calls=0` | `calls=8 placed=8`, at the eight path points (y 0) | 8 of 8 | holds |
| BSM01 craft positions | - | every PT and Rescue at the generated spawn (2428.8, -3072.4) | at their path points; the unit rows and the formation column lines move | placed | holds |
| BSM01 `pair_diff` S vs SP | - | - | 1, gameplay identical | 3 | **failed** |
| BSM01 deaths / hit records | 0 / 0 | 0 / 0 | 0 / 0 | move | **failed** |

**Why the three rows failed.**
- Past line 683 the think reaches natives this host does not implement: Effect (32 calls),
  ExplodeToParts (15), SetDamagedGFXLevel (8), DisablePhysics (8), AddMatrixInterpolator (9), and
  `GetHpPercentage` through the unimplemented health slot 00923BF6. These are the scripted Pearl
  Harbor explosions, so nothing in the gunnery tables moves.
- The eight craft are placed, but they fight nothing in 450 s. `pair_diff`'s gameplay rows are the
  gunnery and death tables, so a moved position alone is "gameplay identical".

**Verdicts.**
- **`kScoringPlayerShotDownBound` ON.** The native answers the read's count, the think runs through
  every pass, and the reference missions are identical.
- **`kPutToBound` ON.** All eight placements land where the script asks, and the reference missions
  are identical.
- BSM01's next gaps are the effect natives listed above; they are outside this packet.

## GetHpPercentage's health slot (packet `cc9_get_hp_percentage`, `kUnitHealthFractionBound`, committed OFF)

Worker cc9-ships2, on main `0a86f3d5b`. Ghidra was read-only.

### The read

- **GetHpPercentage** (0088E9D0, `lua_binding_get_hp_percentage`) pushes 00923BE0's answer
  unscaled. It is a fraction in `[0, 1]`, not a percentage.
- **00923BE0 BSP_UnitInstance_GetHealth:**
  - `[entity+5Dh]` set answers 0.0 (00923BE4..00923BEE);
  - otherwise it calls `vtable[110h]` (00923BF1..00923BF7), float-stores the result, answers 0.0
    below 0 (storing 0 at `+164h`), and clamps it to 1.0 (00D7A24C);
  - the clamped value is cached at `+164h`.
- **vtable[110h]** is 00876260 BSP_UnitInstance_GetHealthFraction in all nine unit vtables
  (00CF90B0, 00CFA778, 00CFB738, 00CFC3D0, 00CFFA30, 00D01630, 00D09678, 00D0BF80 and 00D0C648,
  each `+110h`). Its body is `FLD [ecx+370h]`, `FDIV [ecx+36Ch]`, then a float store: health over
  max health.
- **The base entity's slot** is 0042BB50, `FLD1`. A non-unit entity reads 1.0.
- **The host today** clears the gate and answers 0 from the slot, so every unit reads 0.0 to the
  scripts.

### Callers in this installation's Lua (read-only)

| mission | line | branch | today |
| --- | --- | --- | --- |
| USN02 | usn_2_java.lua:533 | phase 1, while primary 1 is active: `hp = GetHpPercentage(DeRuyter)`. With `hp < 0.15` or all eight `EnemyDestroya` dead, it completes primary 1 and runs `luaPh1FadeOut`, which is `Blackout(true, "luaMoveToPh2", 1)` | `calls=1`: DeRuyter reads 0 at the first check, so phase 1 ends and `luaMoveToPh2` runs at about 25 s (`bt_off_usn02.log`: the call before the 500-step line; the `Blackout(true, "luaMoveToPh2")` line; the callback line) |
| USN04 | usn_19_coralus.lua:1996, `luaGetHP` | used for the HUD text | `calls=0` in every USN04 log |
| USN01 | usn_1_marshall.lua:555 | `hp = GetHpPercentage(Mission.Katori)` | not measured in this packet |
| BSM01 | bsm_01_stationed_at_pearl.lua:703..759 | per battleship, `if GetHpPercentage(ship) <= 0.31 and not ...Sunk`: that ship's scripted sinking | `calls=1192`: all eight read 0, so every one takes its sinking branch |

### The binding (under `kUnitHealthFractionBound`)

- The gate is the gunnery host's death record (`GameGunneryHost::unit_dead`).
- The slot is the gunnery row's `health / max_health` for a unit, and 1.0 for a scene marker
  (0042BB50).
- **LABELLED:** `+5Dh` stands for the gunnery host's death; the host has no torn-down byte.
- **Census:** `summary mission script unit health reads bound=.. reads=.. dead=.. markers=..`.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

| row | prediction |
| --- | --- |
| USN02 GetHpPercentage calls | 1 -> many (one per think while phase 1 and primary 1 are active) |
| USN02 first value | DeRuyter 1.0 at the first check, against 0 |
| USN02 phase change | `luaMoveToPh2` at about 25 s on OFF; later on ON, or not within 450 s |
| USN02 `pair_diff` | 3: phase 2's moves start later, so ship positions, hits, deaths and the mission end move |
| USN04 `pair_diff` | 1, `calls=0` |
| BSM01 battleship sinking branches | none taken on ON, since the eight read 1.0 until damaged: Effect (32), ExplodeToParts (15), SetDamagedGFXLevel (8), DisablePhysics (8) and AddMatrixInterpolator (9) go to 0 |
| BSM01 `pair_diff` | 1: those natives are unimplemented, so nothing in the gunnery tables moves |

### BSM01's remaining gaps (for a later packet)

These natives are the scripted sinking at lines 703..759 and elsewhere. Each is unimplemented in
this host. The labels below come from the names and the calling code, not from a read of the
bodies.

| native | label |
| --- | --- |
| `Effect` (008A9730) | display-only: spawns a named visual effect at an entity or point |
| `ExplodeToParts` (0088E1B0) | state: breaks the entity into its parts (a destruction step) |
| `SetDamagedGFXLevel` (0088F710) | display-only: the damage texture level |
| `DisablePhysics` (00891380) | state: stops the entity's physics (its motion) |
| `AddMatrixInterpolator` (008ADE00) | state: drives the entity's world matrix along an interpolation (the sinking pose) |

### The pairs, measured, and the verdict

- OFF is this tree's `build\` at `de6053b5c`; ON is `pair_export --flip
  kUnitHealthFractionBound=true` of the same commit (SHA-256 `F2259A46169F`).
- All runs had the streams and the death table on. Logs: `local\hp_{off,on}_{usn02,usn04,bsm01}.log`.
- Every log was checked for its milestone line, its module directory and its final COM release
  line.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 health reads (GetHpPercentage) | 1 | 3 | many | **failed**: the check runs only from the intro's end to the mission failure at 39.65 s |
| USN02 first value | 0 (phase 1 ends) | at least 0.15 (the branch is not taken; the value is not logged) | 1.0 | not resolvable from the log |
| USN02 phase 2 (`luaMoveToPh2`) | about 25 s | never within 450 s | later or never | holds |
| USN02 `pair_diff` | - | 3 | 3 | holds |
| USN04 `pair_diff` | - | 1; `reads=0` | 1 | holds |
| BSM01 health reads | 1192 at 0 | 1192 | - | - |
| BSM01 scripted sinkings | Effect 32, ExplodeToParts 15, SetDamagedGFXLevel 8, DisablePhysics 8, AddMatrixInterpolator 9 | all 0 (no row) | all 0 | holds |
| BSM01 `pair_diff` | - | 1 | 1 | holds |

**USN02's new shape** (the reference e row is superseded by this):

| quantity | OFF (phase 1 ended at the first check) | ON |
| --- | --- | --- |
| phase 2, `luaMoveToPh2` | at about 25 s | not reached within 450 s |
| units | 32 | 28: phase 2's Nachi, Sazanami, Naka and Ushio are never generated |
| mission end | failed at 39.65 s, "Game Over" | failed at 39.65 s, "Game Over" |
| its cause | Exeter sunk at 35.80 s by Tokitsukaze, then usn_2_java.lua:521 (`Houston.Dead or Exeter.Dead`) | the same kill at the same time |
| deaths | 21 | 20 |
| hit records | 652 | 611 |
| hull hits | 296 | 227 |
| shots | 1095 | 865 |
| ship torpedo launches | 221 | 165 |
| death rows | - | 16 changed; Houston, Witte, Alden and John1 survive; Minegumo, Perth and Asagumo die |

The mission-end row's `entity` label moves from "Encounter" to "Houston" (the script object the
host names at the store). The time, the text and the cause are the same.

**Verdict: `kUnitHealthFractionBound` ON.**
- The slot answers the read's fraction.
- USN02's phase 1 now ends the way the script says, on DeRuyter's hit points or the enemy
  destroyers.
- BSM01's battleships no longer sink by script at full health.

## BSM01's state natives (packet `cc9_bsm01_state_natives`, three switches committed OFF)

Worker cc9-terrain2, 2026-09-27, on main 4c0a7ad1e. Ghidra was read only. Every body below is
a defined Ghidra function, so there are no `no_ghidra_function` bodies.

### What each native does (read)

- **`DisablePhysics` 00891380.**
  - It resolves argument 0, calls 0080E490 (`MOV EAX,[ECX+1018h]`, the ship's force controller)
    and stores 1 at controller +14h, with no null test.
  - 009329C9 tests that byte at the head of the controller's apply-forces callback. When it is set,
    both velocities go to zero, the body's no-gravity bit is set, and the accumulators are cleared
    (docs/SHIP_HYDRO_FORCES.md, docs/UNIT_CONTROLLER_UPDATE.md).
  - So the ship stops moving under its own physics from the next step.
- **`AddMatrixInterpolator` 008ADE00.** It reads argument 3 as a number, arguments 1 and 2 as
  Vector3 tables and argument 0 as an entity, then calls 00905080(world, entity, translation,
  rotation, duration) at 008ADF92. The record takes the entity's +74h matrix and the clock
  (docs/WORLD_ENTITY_UPDATE.md). Every frame, the world pass 00904600 then:
  - composes `RotZ(-p·rz) RotY(-p·ry) RotX(-p·rx) Translate(p·t) · base`, where p =
    clamp((clock - start) / duration, 0, 1);
  - writes that through the entity's slot 88h, which is 006E00A0 for a unit
    (docs/ENTITY_LOCAL_MATRIX.md): it copies into +74h, invalidates the pose and calls
    (entity+310h)->vtable[0Ch].
  - In BSM01 this is the scripted settling of a sunk battleship: down 4 to 15 m and rolled over 1
    to 65 s.
- **`ExplodeToParts` 0088E1B0.** It resolves argument 0, calls 0080E490, then 00935C70 on the
  controller: every part whose health is above 0 is set to -10000.0 and detached through 00934150
  (bsp `parts_detach_all_live_00935c70`, src/unit_damage.cpp).

### Callers in this installation's Lua (read-only)

- **bsm_01_stationed_at_pearl.lua:**
  - `ExplodeToParts` at lines 1191 (Phoenix) and 2448 (a helper's `ship`);
  - `DisablePhysics` and `AddMatrixInterpolator` in pairs at 1776/1781 (Raleigh) and 2075..2289
    (Arizona, West Virginia, Nevada, Utah, Maryland, Oklahoma, Tennessee, California);
  - each pair sits inside a battleship's sinking branch.
- **usn_2_java.lua:906 and usn_1_marshall.lua:1096:** `ExplodeToParts(ship)`.
- **usn_19_coralus.lua:1658..1671:** Shokaku and Zuikaku sinking.
- **On the current base none of these is reached.**
  - BSM01 9200/9000 (`local\BASE_BSM01.log`, worktree cc9-terrain2) fires no shot, and no
    battleship is damaged: the idle player's side has nothing attacking. With
    `kUnitHealthFractionBound` ON the sinking branches read full health.
  - The native table has no row for any of the three.
  - USN01, USN02 and USN04 show no row either (`local\AC_ON_*.log`).

### The bindings (each committed OFF)

- **`kDisablePhysicsBound`.** The native sets the host slot's `controller_disabled_14`, which feeds
  the hydrodynamics' `in.disabled`. That input used to be fixed at false, and its disabled path
  is already reconstructed.
  - A unit with no buoyancy elements (no controller in the host) is refused and logged.
- **`kAddMatrixInterpolatorBound`.** The native queues a request, and the world pass registers it
  through `add_matrix_interpolator_00905080` with the unit's `local` matrix and the world clock.
  - The pass's slot 88h call now writes the matrix into the unit's motion pose and publishes it
    (`set_local_matrix_006e00a0`).
- **`kExplodeToPartsBound`.** The native queues the unit, and the gunnery host's next fixed step
  runs 00935C70 over its 20 hull segments:
  - each live segment goes to -10000;
  - a present segment is published as destroyed, as when a segment is shot away (0092D1F0's
    path).
- **LABELLED:**
  - the two queues run at the next world pass or fixed step, not inside the Lua call;
  - (entity+310h)->vtable[0Ch] is unread and not reproduced;
  - 00934150's debris, effects and dynamics are the ship motion's reading of the published list;
  - `entity_active` stays the existing stand-in (always true).

### Predictions (written before the pairs; streams and the death table on, lockstep 0.05, idle player)

- **BSM01 9200/9000, USN01 3200/3000, USN04 4700/4500, USN02 9200/9000.** None of the three natives
  is called, so no queue is filled and no record or flag is set.
  - `pair_diff` exits 0 on all four (1 only on listed noise).
  - No unit is frozen or moved by script, and deaths and hit records do not move.
  - This is a pair of three switches flipped together in one export: every one of them is
    unreached.

### The pairs and the verdict

- **The runs.** OFF is `local\bin\bs_off`, a build of b14ea3683. ON is `pair_export` of b14ea3683
  with all three switches flipped (SHA-256 099CE22D7AEB). Streams and the death table were on,
  lockstep 0.05, idle player. Logs: `local\BS_{OFF,ON}_{BSM01,USN01,USN04,USN02}.log` in worktree
  cc9-terrain2. Each has its milestone line, module directory and final COM release.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| BSM01 native rows for the three | none | none | none | held |
| BSM01 deaths, hit records, shots | 0, 0, 0 | identical | identical | held |
| USN01 deaths, hits, shots | 7, 150, 561 | identical | identical | held |
| USN04 deaths, hits, shots | 44, 789, 6321 | identical | identical | held |
| USN02 deaths, hits, shots | 19, 566, 850 | identical | identical | held |
| `pair_diff` (all four) | | exit 0 | exit 0 | held |

**Verdict: all three ON, bound but unexercised.** No reference run reaches a call. The first
measurement needs a run in which the player's side damages a Pearl Harbor battleship (BSM01) or
sinks Shokaku or Zuikaku (USN04's late phase). That run should show:
- the frozen hull (`DisablePhysics: ... controller+14h=1`);
- the settling pose (the world's `interpolators=` field in the per-frame line);
- `ExplodeToParts` segment lines.

## The unimplemented Lua natives, ranked (packet `cc9_lua_natives_ranking`, a read)

Worker cc9-lua2, 2026-09-27. Docs only.

### Sources

The census reads the final native table (`<name> <address> UNIMPLEMENTED calls=N`) of the latest
completed log per mission. "Completed" means the log carries `loop_finished=1`; logs after 17:12
crashed at renderer init under the lock screen.

| mission | log (read-only) |
| --- | --- |
| USN01 3200/3000 | `cc9-lua2/local/sq_off_usn01.log` |
| USN02 9200/9000 | `cc9-lua2/local/gp_on_usn02.log` |
| USN04 4700/4500 | `cc9-gunnery3/local/RA_OFF_USN04.log` |
| USN13 3200/3000 | `cc9-plane2/local/RM_USN13.log` |
| JM06 3200/3000 | `cc9-lua2/local/gp_on_jm06.log` |
| JM08 9200/9000 | `cc9-ships2/local/lo_base_jm08.log` |
| BSM01 3200/3000 | `cc9-ships2/local/rb6_bsm01.log` |
| LOMP06 1200/1000 | `cc9-lua2/local/gp_on_lomp06.log` |

- **The script lines** come from this installation's mission files and `scripts/global`
  (read-only).
- **The tools:** `local/cc9-lua2-natives.py` builds the census, and `local/cc9-lua2-callsites.py`
  finds the lines. Both are in the cc9-lua2 tree.
- **What "neutral" means:** every UNIMPLEMENTED native returns nothing to Lua, so the script sees
  nil.

### The ranking

**Scope.** Only the rows a mission script reaches through a Lua native (`MissionLuaNative::*`)
are ranked. They are ordered by the missions that reach them, then by the calls. The other
UNIMPLEMENTED rows whose names mention Lua or Script are host records of the load or of the
renderer, and are left out: `MissionLoad(lua)::*`, `SceneUnit::vehicle_class_descriptor` and
`HudMarkers::viewport_descriptor`.

| native | address | missions | calls | example line | what nil does |
| --- | --- | --- | --- | --- | --- |
| `Kill` | `008AC5C0` | 7 (all but BSM01) | 13 | JM06.lua 212 `Kill(FindEntity("Gato-class Submarine 01"), true)` (difficulty below 2); usn_1_marshall 791 (phase 3 removes `Mission.MainAttack`); usn_19_coralus 1303/1309 (plane cleanup); commandhelpers 7830 (`Mission.CamScript`) | **the entity lives on.** On JM06 the Gato submarine the script removes at init stays in the battle; `gp_on_jm06` still lists it in the unit table |
| `SetInvincible` | `00897A50` | 6 | 87 (BSM01 58) | usn_19_coralus 456 `SetInvincible(Mission.Town, 0.24)`; 23 lines in commandhelpers | the unit is not protected: scripted survivors can die, and a mission branch keyed on their survival moves |
| `AddListener` / `IsListenerActive` | `008C6760` / `008C6BB0` | 7 / 6 | 17 / 10 | usn_19_coralus 1201 `AddListener("hit", "LexHitListener", {callback = "luaLexHit", target = {Mission.Lex}})`; JM06 1205.., BSM01 1897.. | **no callback ever fires.** `IsListenerActive` returns nil, which reads false |
| `PrepareClass` | `008C8F70` | 7 | 45 | usn_1_marshall 13 and every mission's preload | none that a run shows: a class preload |
| `EnableInput`, `MissionNarrative`, `BlackBars`, `DisplayScores`, `DisplayUnitHP`, `HideUnitHP`, `MissionNarrativeClear`, `CountdownCancel`, `Loading_Start`/`_Finish`, `ShowHint*`, `AddStoredHint`, `RemoveStoredHint` | | 1-7 | 1-28 | intro and outro presentation | presentation only for an idle player |
| `IsGUIActive` | `008CA010` | 5 | 172 | commandhelpers 13270 `IsGUIActive("GUI_map")` (music); LOMP06 713 (`GUI_periscope`) | music selection; LOMP06 falls through to its depth test |
| `SetGuiName`, `SetNumbering` | `008A8F90`, `0088FE30` | 4, 2 | 56, 30 | usn_19_coralus 155, 165 | display names |
| `IsClassChanged` | `008CC4B0` | 4 | 54 | JM06 1699, LOMP06 862, commandhelpers 18741 | the class-change branch never runs |
| `SetForcedReconLevel` | `008AA8F0` | 1 (USN13 15) | 15 | usn_13_truk 1140; commandhelpers 4201.. | the forced level is not applied. The recon pass stays on detection alone, so the AI's visibility and the reconlevel tables move |
| `AAEnable` | `0089C740` | 1 (BSM01 33) | 33 | bsm_01 404; commandhelpers 3823 | anti-aircraft guns keep their default state where the script switches them |
| `SetShipSpeed`, `SetShipMaxSpeed` | `00890D30`, `00890A10` | 1, 1 | 19, 2 | LOMP06 111.. (6 lines) | scripted ship speeds are ignored |
| `SquadronSetSpeed`, `SquadronSetTravelAlt` | `0089F780`, `0089F550` | 1, 2 | 15, 3 | usn_13_truk 1641 | squadrons keep their own speed and altitude |
| `SetAirBaseSlotCount` | `008963E0` | 2 | 5 | usn_19_coralus 212, usn_13_truk 559 | the deck keeps its authored slot count |
| `SetFireTarget` | `0089A8B0` | 1 (USN02 14) | 14 | commandhelpers 2951 | the scripted fire target is not set |
| `UnitGetAttackTarget`, `ForceRecon` | `008A6DE0`, `008AADF0` | 1 (LOMP06) | 6, 6 | LOMP06 696, 530 | nil target; `ForceRecon` is bound behind `kReconLevelTableBound` |
| `SetSubmarineDepthLevel`, `SetUnlimitedAirSupply` | `00893F40`, `00893C00` | 1 (JM06) | 5, 2 | JM06 | the scripted depth is ignored; air supply is limited |
| `PilotMoveTo`, `PilotMoveOnPath` | `008A4150`, `008A3E70` | 1, 2 | 4, 3 | JM08, JM06, BSM01 | pilots keep their own orders |
| the rest | | 1 or 2 | 1-2 each | `SetDeviceReloadEnabled`, `UnitSetPlayerCommandsEnabled`, `Scoring_SetMissionCompleted`, `BannSupportmanager`, `GetClosestBorderZone`, `LoadCheckpoint`, `ForceEnableInput` | player or end-of-mission paths |

### The top five, as proposed packets

1. **`Kill`, `008AC5C0`.** It is measured on JM06 3200/3000: the Gato removal at line 212 runs
   whenever the difficulty is below 2.
   - **Read first:** what `Kill(entity, silent)` does. It probably reaches
     `00926D90 BSP_MissionEntity_Kill` with a cause, which the units host already models for
     deaths. The read must show what the second argument changes.
   - **Prediction sketch:** JM06 loses the Gato from its unit table at the init stage. There is
     one more death row, or a silent removal with none, and the convoy battle moves.
   - USN01's phase-3 kill, USN04's plane cleanup and LOMP06's seaplane kill come into range only
     if their stages are reached.
2. **`SetInvincible`, `00897A50`.** It is measured on BSM01 (58 calls) and on USN04 (Town at
   0.24).
   - **Read first:** the argument (a health fraction floor or a flag), and where the damage path
     tests it.
   - **Prediction sketch:** on USN04, Town's health never falls below 0.24 of its maximum. Death
     rows change only for units that died OFF.
3. **Listeners, `008C6760` `AddListener` and `008C6BB0` `IsListenerActive`.** They are measured on
   USN04 (`LexHitListener`) and JM06.
   - **Prediction sketch:** callbacks such as `luaLexHit` start to fire, and the census counts
     listener events.
   - This is the largest of the five: event kinds, matching and callback delivery.
4. **`SetForcedReconLevel`, `008AA8F0`.** It is measured on USN13 (15 calls).
   - The forced level is already read into the recon record: `+8h`, used when the force byte
     `+10h` is set (`docs/RECON_SENSOR_PASS_BINDING.md`).
   - **Prediction sketch:** USN13's recon counters and the AI's contact lists move for the forced
     units only.
5. **`AAEnable`, `0089C740`.** It is measured on BSM01 (33 calls).
   - **Prediction sketch:** BSM01's anti-aircraft shot counts change for the switched units.

This packet takes item 1 next, as a read plus an OFF binding.

## Kill, 008AC5C0 (packet `cc9_lua_kill`, `kLuaKillBound`, committed OFF)

Worker cc9-lua2, 2026-09-27. This is item 1 of the ranking above. Ghidra was read only.

### The image (V, `docs/UNIT_DAMAGE_AND_DEATH.md`, `src/unit_damage.cpp`)

**`Kill(entity [, hard])`, body `008AC5C0`-`008AC7A5`.**
- It resolves argument 0 through `00888AA0`.
- The cause is 1, or 2 when a second argument reads true (`008AC6DF`..`008AC71B`,
  `bsp::kill_cause_from_lua_008ac5c0`).
- It then dispatches on the class:

| site | class | what it calls |
| --- | --- | --- |
| `008AC729` | a squadron (18h) | `007ED380`, which kills the members of `+3D0h` |
| `008AC740` | a LandConvoy (1Ah) | `00742210`, which kills its member vector (with the boolean form of the cause) |
| `008AC756` | any other entity | `00926D90` |

**`00926D90`:**
- It returns when `+5Fh` is already set.
- Otherwise it sets `+5Fh` and runs `vtable[70h](1)` (the destroy path) with `+70h = cause`.
- It recurses over the children and queues the entity on the kill list, whose flush runs the
  on-killed hook `00928C80`.
- Lua `Kill` is the path that reaches both queues; damage reaches only the destroy list.
- `bsp::lua_kill_008ac5c0` already reconstructs the dispatch over the abstract `UnitDamageHost`.
  This packet binds it in the mission Lua host.

### The binding

- **The switch** is `kLuaKillBound` in `include/bsp/game_hosts_lua.hpp`, committed OFF.
- **The route.** `GameMissionLuaHost::run_kill_008ac5c0` takes the row in `binding_trampoline`.
- **Resolving.** The entity's `ID` minus one gives the units-host slot.
- **A squadron's fused slot** (the registry record's `squadron_unit`) kills every live member.
  This stands in for `007ED380`.
- **Any other slot** is killed itself. An already dead unit is skipped, as `+5Fh` makes the image
  return.
- **The kill itself** is `GameGunneryHost::kill_unit_00926d90(unit, cause)`, the host's one death
  funnel.
- **The census:**
  - `summary mission script kill bound=.. calls=.. units=.. unresolved=.. already_dead=..
    squadrons=..`;
  - one `Kill 008ac5c0: "<name>" cause=N victims=N` line per call.

**SUBSTITUTIONS, labelled:**
- An entity with no units-host slot is not killed: a script entity such as `Mission.CamScript`
  (`luaCamOnTargetExt`, `commandhelpers.lua` 7830), a path or a marker. Such calls count as
  `unresolved`.
- A LandConvoy kills only its own slot, because the host keeps no member vector.
- The gunnery funnel ignores the cause, so 1 and 2 die alike. It also counts the call in its
  water-depth census (`water_depth_kills`). The gunnery files are leased to another worker, so the
  funnel is not changed here.

### Predictions, before any run

Streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player. The pair is this tree at the OFF
commit against a `pair_export` of it with the switch flipped. The measured call counts come from
the ranking.

| row | prediction |
| --- | --- |
| JM06 3200/3000 | `calls=1 units=1`: `Kill 008ac5c0: "Gato-class Submarine 01" cause=2` in the stage init (`JM06.lua` 212, the difficulty-below-2 arm). The Gato, which OFF carries a scene `attackmove` to the end, dies at about 0 s: one more death row with no killer (deaths 4 -> 5). The convoy battle moves, **exit 3**. Every moved row must trace to the Gato's absence |
| USN01 3200/3000 | `calls=2`. The first call is argc 1 in the stage init, which fits `Kill(Mission.CamScript)` and is `unresolved`. If both calls are that, the pair is identity (exit 1). If one is `usn_1_marshall.lua` 791 (phase 3, `Mission.MainAttack`), those planes die and USN01 moves. The per-call line settles which |
| USN04 4700/4500 | `calls=3`, at `usn_19_coralus.lua` 1303/1309 (the Japanese plane cleanup, `Kill(unit, true)`) or 3248 (`Mission.movieval`). Planes that OFF keeps alive die, so exit 3 is likely. The per-call lines name them |
| USN02 9200/9000 | `calls=1`, one line: its victim decides between identity and a move |
| LOMP06 1200/1000 | `calls=2`: identity if both are unresolved script entities |

**The risk named in advance.** A victim whose death the script later tests (`unit.Dead`) moves a
mission branch. That is the image's behaviour, and not a failed prediction, as long as the
victim is the one the script named.

## SetInvincible, 00897A50 (packet `cc9_set_invincible`, a read; the binding waits for the gunnery host)

Worker cc9-lua2, 2026-09-27. This is item 2 of the ranking. Ghidra was read only.

### The image (V, `docs/UNIT_DAMAGE_AND_DEATH.md` "The invincibility gate", `src/unit_damage.cpp`)

- **The value.** `SetInvincible(entity, value)` resolves argument 0 and turns argument 1 into a
  float:
  - a boolean gives 1.0 or 0.0 (`00897B6F`);
  - otherwise the number itself, a fraction of maximum health.
- **The dispatch.** It calls the entity's `vtable[F4h]` (`00897C63`), which is `0042ED80`. That
  routine stores the float at `+150h` and passes it to every child's `vtable[F4h]`, so turrets and
  parts carry it too.
- **Three readers:**
  - **Damage, `00879070` (`008790D0`).** The amount is clamped so that health cannot fall below
    `inv * max` (`bsp::invincibility_floor_00879070`, with the float-store rounding). A repair is
    unaffected.
  - **Sink, `008110F0` (`008110F9`).** Any value above 0 refuses the sink.
  - **Query, `00897CB0`.** `IsInvincible` is `inv > 0`.
- **`Kill` is not gated.** `008AC5C0` goes straight to `00926D90`.
- **So `SetInvincible(Town, 0.24)`** keeps Yorktown-class01's health at or above 24% of its
  maximum against damage. It does not protect against `Kill`.

### What the host does

- The three damage sites of `src/game_hosts_gunnery.cpp` build `bsp::UnitHealth` with
  `invincibility` at its default of 0: the hull pass (`add_damage`) and the two sites near
  `bsp::apply_damage_00879070`, at the file's 7242 and 7291 on main.
- So the floor never applies, and the sink gate reads 0.
- The binding needs a per-unit `+150h` in the gunnery host's unit state, set by the Lua native
  and read at those three sites and at the sink.
- `src/game_hosts_gunnery.cpp` is leased to cc9-gunnery3 now, so this packet commits the read and
  the plan only.

### Which calls the idle runs reach

| mission | calls | the lines, and the units |
| --- | --- | --- |
| USN04 4700/4500 | 1 | `usn_19_coralus.lua` 456, `SetInvincible(Mission.Town, 0.24)` in the stage init. Town is `Yorktown-class01`, which **takes no damage** on the OFF run (`RA_OFF_USN04`: `taken 0`, health 8000) |
| BSM01 3200/3000 | 58 | loops at `bsm_01` 405 (0.3), 425 (Raleigh 0.4), 998 (`true`) and the rest. **No BSM01 unit takes damage** on the idle run (`rb6_bsm01`: every row has `taken 0`) |
| USN02 9200/9000 | 10 | `usn_2_java.lua` 230: `SetInvincible(unit, 0.1)` over `Mission.DRGrp` (DeRuyter, Java, Kortenaer, Electra) in `luaInit`. Line 310: 0.5 over `Mission.DRKillers` (Samidare, Murasame, Harusame), also in `luaInit`. Lines 755/761 release them in `luaPh2MovieEnd`, whose `DRGrp` loop then runs `AddDamage(unit, 100000000)`. The 10 fit 4 + 3 + 3; how far the release loop runs was not established |
| USN01, USN13, JM08 | 7, 10, 1 | not examined in this read |

### Predictions for the binding (switch `kUnitInvincibilityBound`, to be committed OFF)

| row | prediction |
| --- | --- |
| USN04 4700/4500 | identity, exit 1: the floored unit takes no damage |
| BSM01 3200/3000 | identity, exit 1: no unit takes damage |
| USN02 9200/9000 | **moves, exit 3** |

**The USN02 move.** On OFF, all seven floored ships die: Kortenaer at 68.30 s to Samidare,
Electra at 108.60, DeRuyter at 176.86, Java at 184.26, Samidare at 161.56, Murasame at 190.66 and
Harusame at 212.81. With the floors in force:
- none of them can die to gunfire before `luaPh2MovieEnd` releases them;
- a hit that would cross the floor is clamped instead;
- a floored ship that reaches 0 would also refuse the sink.

The death rows before the release disappear or move later, and the battle's hit and death counts
move with them. The first row to check is Kortenaer, which does not die at 68.30 s.

**So the measuring pair is USN02, not USN04 or BSM01.** Those two resolve as identity only.

### Kill: the pairs and the verdict

**Setup.**
- OFF is this tree's build of `5a63f68e2`, which carries the switch from `f9da06393` and the merges
  of main up to `bb829bd85`.
- ON is `pair_export --commit HEAD --flip kLuaKillBound=true` (`local/kill_on`, SHA-256
  `49EC86544C98`).
- Both environment options were set, at lockstep 0.05 with an idle player.
- The logs are `local/kill_{off,on}_<mission>.log` in the cc9-lua2 tree.

| row | calls / units / unresolved | pair_diff | prediction | verdict |
| --- | --- | --- | --- | --- |
| JM06 3200/3000 | 1 / 1 / 0: `"Gato-class Submarine 01" cause=2` | exit 3 | Gato dies at about 0 s, deaths 4 -> 5, the convoy battle moves | **partly failed**: the Gato dies at 0.00 s and deaths go 4 -> 5, but hits (405), shots (652) and damage are **unchanged**, and the Gato's is the only unit row that changes |
| USN01 3200/3000 | 2 / 0 / 2 (entity ids 100003, 100025) | exit 1 | identity if both are script entities | held |
| USN02 9200/9000 | 1 / 0 / 1 (100002) | exit 1 | its victim decides | held: a script entity, identity |
| USN04 4700/4500 | 3 / 3 / 2: `"movieval" cause=2 victims=3`, a squadron | exit 3 | exit 3 likely, the victims named by the per-call lines | held |
| LOMP06 1200/1000 | 2 / 0 / 2 (100003, 100007) | exit 1 | identity if both are unresolved | held |

**The details.**
- **JM06:** the Gato never engages in OFF: it has no shots, no hits and no damage either way. So
  removing it changes only its own row and the death count. The prediction overstated the spread.
- **USN04:** `Kill(Mission.movieval, true)` (`usn_19_coralus.lua` 3248) removes the three movie
  planes at 26.55 s. OFF shoots them down at 150.25, 154.40 and 157.81 s. The air battle moves from
  there:
  - hit records 739 -> 722;
  - shots 5482 -> 5654;
  - torpedo drops 1 -> 0.
  These moves come from removing three anti-aircraft targets at 26.55 s. They are not RNG
  coupling: both sides ran with `BSP_GUNNERY_RNG_STREAMS=1`.
- **Script entities:** the entity ids of 100000 and up belong to script entities with no
  units-host slot. They stay `unresolved` as labelled.

**Verdict: `kLuaKillBound` is flipped ON.** Every mechanism prediction held. Each move is the
named victim's removal, which is the image's behaviour. The JM06 failure is a smaller spread than
predicted and is recorded above.

## The listener natives, first read (packet `cc9_lua_listeners`, a read)

Worker cc9-lua2, 2026-09-27. This is item 3 of the ranking. Ghidra was read only, and the listings
were read from disk.

### Correction to the ranking's example

The ranking gave USN04's `LexHitListener` (`usn_19_coralus.lua` 1201) as the example. It is
registered only at the tail of the Zuikaku-sinking cinematic, which a 4500-frame idle run never
reaches (`docs/HANDOFF_USN04_LUA_NATIVES.md` section 3). USN04's two measured `AddListener` calls
are other listeners. The script offers the `kill` listeners at 1612/1635 (the Zeros over Zuikaku
and Shokaku), the `recon` listener at 1932 and the helpers' `input` listeners. Which two run is
not established.

### The registry (V)

- **`AddListener(channel, id, params)`, `008C6760`.** It reads arguments 0 and 1 as strings and
  passes the table in argument 2 to `00980C10`.
- **`00980C10`**, under the manager's lock at `+24h`:
  - `00980150 BSP_WarningManager_ChannelIndex` selects the channel by name, with a
    case-insensitive map;
  - `00978D60` finds or creates the slot for `id` in that channel;
  - the slot takes the subscription that `0097E360 BSP_WarningManager_ParseEventBlock` builds from
    the params.
- **The consequences.**
  - A listener is keyed by (channel, id), and re-adding the same id replaces it.
  - `IsListenerActive(channel, id)` (`008C6BB0`, through `00980E00`) is that lookup, pushed as a
    boolean.
  - `RemoveListener` (`008C6990`) erases it.
- **Firing.** Each channel's dispatcher (`docs/MISSION_EVENTS_UPDATE.md`, "The named event
  channels") evaluates the channel's subscriptions (`0097B8C0`: `subscription->vtable[3](params)`)
  and calls each passing subscription's callback (`subscription+4h`) through the mission Lua
  host.

### The parser's channel table (V, from the listing of `0097E360`)

- **How it tests.** Each arm compares the channel name, `__stricmp` for the first and
  `00425850` for the rest.
- **How it builds.** On a match it falls through to `operator new(size)` and the subscription's
  constructor. The pairing below is read from the listing, because the pseudocode's else-chain
  hides it.
- **The check.** The first row matches the pseudocode's visible `recon` arm, `operator_new(0x4c)`
  then `FUN_0097a220`.

| channel | size | constructor | channel | size | constructor |
| --- | --- | --- | --- | --- | --- |
| `recon` | 4Ch | `0097A220` | `zone` | 3Ch | `0097C6D0` |
| `kill` | 3Ch | `0097A450` | `command` | 4Ch | `0097C8A0` |
| `hit` | 80h | `0097C2C0` | `target` | 2Ch | `0097AD80` |
| `exitzone` | 2Ch | `0097A620` | `ammoType` | 2Ch | `0097AEF0` |
| `input` | 1Ch | `0097A790` | `stock` | 3Ch | `0097B060` |
| `surrender` | 1Ch | `0097A8B0` | `gui` | 2Ch | `0097CAD0` |
| `failure` | 38h | `0097C560` | `generate` | 2Ch | `0097CC40` |
| `leak` | 28h | `0097A9D0` | `player` | 2Ch | `0097CDB0` |
| `fire` | 28h | `0097AAF0` | `musicOver` | 1Ch | `0097CF20` |
| `repair` | 2Ch | `0097AC10` | `chat` | 3Ch | `0097D040` |
| `entityKilled` | 2Ch | `0097D210` | `shipLanded` | 1Ch | `0097B230` |
| `hpEvent` | 30h | `0097D380` | | | |

The tool is `local/cc9-lua2-parse-map.py` in the cc9-lua2 tree.

### The `kill` subscription (partial)

- **The constructor `0097A450`** installs vtable `00D1B68C` and three empty sets: at `+0Ch` and
  `+1Ch` (node factory `0096BA00`) and at `+2Ch` (`0096BA50`).
- **The block's keys are not read in the constructor.** The three slots below push no string, so
  the `target` and `callback` keys are read elsewhere: in `0097E360` after the constructor, or in
  a slot not listed here. **Not read.**

| slot | address | what it is |
| --- | --- | --- |
| 1 | `00972520` | the loader (see the second read); Ghidra function since dda4de1f6 |
| 2 | `0096E850` | set operations through `0096E3A0` and `0096E460`; Ghidra function since dda4de1f6 |
| 3 | `0096ACE0` | the condition `0097B8C0` calls; three `00BF6713` range checks and `004254B0`; Ghidra function since dda4de1f6 |
| 4 | `0097B390` | a destructor |
| 5, 6, 7 | `009726D0`, `0096E9F0`, `00968710` | not read |

### What binding them takes, in slices

1. **The registry alone.**
   - The work is `AddListener`, `RemoveListener` and `IsListenerActive` over (channel, id).
   - Scripts test `IsListenerActive` only to add or remove, for example
     `luaAddZuikakuZeroListener` (`usn_19_coralus.lua` 1610) and LOMP06 `luaNarwhalDC` 632.
   - With no firing, **identity on every measured mission** is predicted: nil reads false
     today, and true only changes whether a listener is added or removed again.
   - This slice is the ground for the next one.
2. **Firing `kill` and `hit`.**
   - This needs slot 3's condition and the key reads.
   - The producers are the host's own death and hit rows (`0077CE60` and `00959450` in the image,
     `docs/MISSION_EVENTS_UPDATE.md`).
   - **Measure:** USN04's Zero `kill` listeners, if one of its two calls is `ZuikakuZeroKillListener`.
     Its callback `luaZuikakuZeroDead` runs when a listened Zero dies (Zeros die at 143.45 s and
     after). LOMP06's `listener_NarwhalDC` (`hit`) only starts a dialog.
3. **Firing `recon` and `command`.**
   - The recon pass already produces the levels, and the commands host already has
     `report_command_event_00984300`.
   - **Measure:** JM06's `submove` command listener (1205) and `cvListener`/`usnsubListener`
     (1582/1601), and LOMP06's `listener_SeaplaneSpotted` (328), whose callback
     `luaSeaplaneSpotted` (638) runs on a detection.

### no_ghidra_function

| start | end (inclusive) | ABI | exits |
| --- | --- | --- | --- |
| `00972520` | `009725AC` | `__thiscall`, `RET 4`, INT3 after | one |
| `0096E850` | `0096E8E5` | `__thiscall`, `RET 4`, INT3 after | one |
| `0096ACE0` | `0096ADA6` | `__thiscall`, `RET 4` | two: `0096AD9C` and `0096ADA4`, INT3 after the second |

All three are slots of vtable `00D1B68C`.

**Correction.** The lead defined all three in Ghidra under `dda4de1f6`, so they are no longer
bodies without a function. The table above keeps the extents that were reported. `00979140` is
defined to `00979191`.

### The `kill` subscription's keys and its condition (packet `cc9_lua_listeners`, second read)

The listings were read from disk. The three slots are defined in Ghidra since `dda4de1f6`.

**Slot 1, `00972520`, is the loader.** Its argument is the block's property reader.
- `callback` (`00CE49C0`) is read into `+4h` through the reader's `vtable[10h]`
  (`0097254B`..`00972552`).
- The three sets are filled from their keys:

| key | string | set | through |
| --- | --- | --- | --- |
| `entity` | `00CE5818` | `+0Ch` | `009721C0` |
| `lastAttacker` | `00D1B0B8` | `+1Ch` | `009721C0` |
| `lastAttackerPlayerIndex` | `00D1B0A0` | `+2Ch` | `009722D0` |

- The scripts write exactly this shape, for example `usn_19_coralus.lua` 1612:
  `AddListener("kill", "ZuikakuZeroKillListener", {callback = "luaZuikakuZeroDead", entity =
  Mission.ZuikakuZeroes, lastAttacker = {}, lastAttackerPlayerIndex = {}})`.

**Slot 3, `0096ACE0`, is the condition** `0097B8C0` calls.
- **The parameters** are a vector of three boxed values (`params+4h`..`+8h`, bounds-checked
  through `00BF6713`):
  - [0] the victim;
  - [1] the last attacker;
  - [2] the last attacker's player index.
- **The test.** Each value goes to its set's `vtable[0]` (`00979140`):

| set | value | call site |
| --- | --- | --- |
| `+0Ch` | [0] | `0096AD0D` |
| `+1Ch` | [1] | `0096AD3B` |
| `+2Ch` | [2] | `0096AD6C` |

- **A match** needs all three. The routine then logs through `004254B0` (format `00D1AF4C`, the
  callback name, or `00F8A0C8` when it is null) and returns 1. Otherwise it returns 0.
- **`00979140` answers 1 for an empty set,** and otherwise tests membership (`009790D0`). So `{}`
  means any.

**What a measured `kill` listener does.**
- `luaZuikakuZeroDead` (`usn_19_coralus.lua` 1624) removes its own listener and clears
  `Mission.ZeroOverZuikaku`. That is the flag `luaAddZuikakuZeroListener` (1610) tests before it
  registers the listener again. `luaTownCatDead` (2518) is the same shape.
- The callbacks take no argument, apart from `luaSeaplaneSpotted(entity)` on the `recon` channel.

### The binding, concretely (slices 1 and 2)

- **The registry, in the mission Lua host.**
  - A map keyed by channel and id, both case-insensitive as `00980150` is, holds the callback
    and, for `kill`, the three sets. The sets are read like the loader: an `entity` given as an
    entity table or a table of them, with the attacker sets the same.
  - `AddListener` replaces the entry, `RemoveListener` erases it, and `IsListenerActive` answers
    from the map.
- **Firing `kill`.**
  - Once per mission frame, every newly destroyed units-host unit is evaluated against each `kill`
    subscription. The source is `GameUnitsHost::destroyed_units`, the same rows the HUD observer
    uses.
  - The victim is tested against `entity`. The callback runs through the host's named-call path,
    which stands in for `00887E50`.
  - **SUBSTITUTIONS, labelled:**
    - The dispatch runs at the host's frame, not inside the kill flush.
    - A subscription with a non-empty `lastAttacker` or `lastAttackerPlayerIndex` set is not
      matched, because the host's death row does not carry the attacker as an entity. No measured
      listener sets them.
- **The predictions are to be written with the binding.**
  - USN04 is the measure, if one of its two `AddListener` calls is a Zero `kill` listener.
  - The binding's own census will show which.

### The listener binding (`kLuaListenersBound`, committed OFF)

- **The switch** is in `include/bsp/game_hosts_lua.hpp`.
- **The natives.** `AddListener` (`008C6760`), `RemoveListener` (`008C6990`) and
  `IsListenerActive` (`008C6BB0`) run on a case-insensitive (channel, id) registry in
  `GameMissionLuaHost`.
  - A `kill` entry carries the callback, the `entity` ids and whether either attacker set is
    non-empty.
  - `IsListenerActive` pushes a boolean.
- **Firing `kill`.** `dispatch_kill_listeners_009813a0` runs once per mission frame, at the head of
  the spawn-queue step.
  - For every newly destroyed units-host unit, it collects the callback of each `kill` entry
    whose `entity` set is empty or holds the victim, then calls each callback with no argument.
- **The census:**
  - `summary mission script listeners bound=.. adds=.. removes=.. queries=.. registered=..
    kill_deaths=.. kill_fires=.. attacker_filtered=..`;
  - one `AddListener 008c6760: channel= id= callback= entities= attacker_filters=` line per new
    entry;
  - one `kill listener 009813a0: victim "<name>" -> <callback>()` line per call.

**SUBSTITUTIONS, labelled:**
- `kill` is evaluated at the host's frame, not inside the kill flush.
- An entry with a non-empty `lastAttacker` or `lastAttackerPlayerIndex` set is not matched,
  because the host's death row has no attacker entity. Such entries count as `attacker_filtered`.
- A squadron's fused slot counts as dead only when the registry record's live count is 0, the
  same rule as `kSquadronObserverLivenessBound`.
- Only `kill` fires. `hit`, `recon`, `command`, `input` and the other channels are registered
  and answer `IsListenerActive`, but never call back.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player; the census names
each registered entry, which settles the rows that depend on it):

| row | prediction |
| --- | --- |
| USN01 3200/3000 | 3 adds (`ConLeadListener` is `hit`, the rest `input`), no `kill` entry, `kill_fires=0`; identity, exit 1 |
| USN02 9200/9000 | 1 add (an `input` helper), `kill_fires=0`; identity |
| LOMP06 1200/1000 | the adds are `hit`, `recon` and `input`, with no `kill` entry; identity |
| JM06 3200/3000 | `kill_fires=0` unless `CVKill` (1761) is registered and its entity dies; otherwise identity |
| USN04 4700/4500 | if a Zero `kill` entry (`ZuikakuZeroKillListener`/`ShokakuZeroKillListener`) is registered and its squadron's last plane dies, `luaZuikakuZeroDead`/`luaShokakuZeroDead` fires. That clears `Mission.ZeroOverZuikaku`, so the next launch pass (1386) may launch a fresh Zero squadron: exit 3. With no such entry, or no such death, identity |

### The listener pairs and the verdict

**Setup.**
- OFF is this tree's build of `66f4f9596`, with Kill, squadron liveness and objective kind ON.
- ON is `pair_export --flip kLuaListenersBound=true` of the same commit (`local/ls_on`, SHA-256
  `1912FE1AE50B`).
- The logs are `local/ls_{off,on}_<mission>.log`.

| row | registered on ON (channel id -> callback) | census | pair_diff | verdict |
| --- | --- | --- | --- | --- |
| USN01 | `input IngameMovieInputListenerID` twice (added and removed), `hit ConLeadListener -> luaConLeadHit` | `adds=3 removes=2 registered=1 kill_deaths=7 kill_fires=0` | exit 1 | held |
| USN02 | `input IngameMovieInputListenerID` | `adds=1 removes=1 kill_deaths=19 kill_fires=0` | exit 1 | held |
| USN04 | `input IngameMovieInputListenerID` twice; **no Zero `kill` listener** | `adds=2 removes=2 kill_deaths=44 kill_fires=0` | exit 1 | held, on the no-entry branch |
| JM06 | `hit playerHit -> luaJM6PlayerHit`, `hit hshit -> luaJM6HospitalShipHitByPlayer`, `recon usnsubListener -> luaJM6USNSubSighted`, `recon fleetrecon -> luaJM6FleetSpotted` | `adds=4 registered=4 kill_fires=0` | exit 1 | held |
| LOMP06 | `input` twice, `hit listener_NarwhalDC -> luaNarwhalDC`, `recon listener_SeaplaneSpotted -> luaSeaplaneSpotted` | `adds=4 removes=2 registered=2 kill_fires=0` | exit 1 | held |

**What the census settles.**
- No measured idle run registers a `kill` listener, so slice 2 is inert on this set.
- The listeners these runs do register are `hit` (USN01, JM06, LOMP06) and `recon` (JM06,
  LOMP06).
- **So the next slice with a measurable effect is firing `hit` and `recon`.** The dispatchers are
  `00988510` for `hit` (producer `0077CE60`) and `00980E50` for `recon` (no native producer; the
  recon pass's level changes, `docs/RECON_SENSOR_PASS_BINDING.md`). The subscriptions are `hit`
  at `0097C2C0` (80h) and `recon` at `0097A220` (4Ch), both still to be read.

**Verdict: `kLuaListenersBound = true`.** Every row is identity, as predicted.

### Firing `recon` (packet `cc9_lua_recon_listeners`, `kLuaReconListenersBound`, committed OFF)

**The image (V, from the listings).**
- **The subscription** is vtable `00D1B67C`, constructor `0097A220` (4Ch). Its loader `00972450`
  reads:
  - `callback` into `+4h`;
  - `entity` into `+0Ch`, through `009721C0`;
  - `oldLevel`, `newLevel` and `party` into `+1Ch`, `+2Ch` and `+3Ch`, through `009722D0`.
- **The condition `00968470`** passes when each of the four sets is empty or holds its boxed
  value.
- **The dispatcher `00980E50`** is `__thiscall(manager, party, unit, old, new)`, called from
  `0077B0C0` on a record's level change.
  - It dispatches only a live unit: `+5Ch` set, `+5Dh`/`+5Eh`/`+60h` clear.
  - It boxes `(unit id +174h, old, new, party)`.
  - It calls each passing callback with `(unit, old, new, party)`.

**The binding.**
- `dispatch_recon_listeners_00980e50` runs once per new recon pass generation.
- It compares each unit's per-party level with the last one seen and fires matching `recon`
  entries on every change, with the four arguments.
- The census is `summary mission script recon listeners bound=.. changes=.. fires=..` and one
  `recon listener 00980e50:` line per call.

**SUBSTITUTIONS, labelled:**
- The comparison runs at the host's frame, once per pass, not inside `0077B0C0`.
- The unit's own party steps `0 -> 1 -> 2`, one level per pass. That is `008065B0`'s `+1`
  refresh, and it keeps a 0-to-2 filter from firing on own units.
- Forced levels are not modelled.

**Predictions** (same setup as the listener pairs; OFF is this tree, and ON flips this switch):

| row | prediction |
| --- | --- |
| JM06 3200/3000 | `usnsubListener` (entity `Mission.USNSubs`, `0 -> {1, 2}`, Japanese) fires if the Japanese detect the Narwhal; `luaJM6USNSubSighted` only starts a dialog and a hint. `fleetrecon` (entity `Mission.Cargos`, `0 -> 2`, Japanese) fires if a cargo is identified; `luaJM6FleetSpotted` then orders `Mission.IJNSubsGrp1` (all but the first) to attack a random cargo, **exit 3**. If no cargo is identified in the run, identity |
| LOMP06 1200/1000 | `listener_SeaplaneSpotted` (any entity, `0 -> 2`, Allied) fires on each first identification, and the callback acts only on a `SmallReconPlane` (a dialog); identity |
| USN01, USN04 | no `recon` entry is registered (`changes=0 fires=0`); identity |

#### Recon listener pairs and verdict

**Setup.**
- OFF is this tree's build of `2cb5a3608`.
- ON is `pair_export --flip kLuaReconListenersBound=true` of the same commit (`local/rl_on`,
  SHA-256 `14A1773EB1E5`).
- The logs are `local/rl_{off,on}_<mission>.log`.

| row | census | pair_diff | prediction | verdict |
| --- | --- | --- | --- | --- |
| JM06 | `changes=74 fires=2`: `"US Cargo Transport 02" party 1 0 -> 2 -> luaJM6FleetSpotted()`, `"Narwhal-class Submarine 01" party 1 0 -> 2 -> luaJM6USNSubSighted()` | exit 3 | `fleetrecon` orders the group-1 submarines at a cargo, exit 3 | held |
| LOMP06 | `changes=387 fires=0` | exit 1 | identity; the listener fires on each first identification | identity held. **Failed sub-prediction:** nothing fired. The cause was not traced: no Allied `0 -> 2` change reached the listener while it was registered, perhaps because the Allied levels pass through 1 |
| USN01 | `changes=0 fires=0` | exit 1 | identity | held |
| USN04 | `changes=0 fires=0` | exit 1 | identity | held |

**What moved on JM06.**
- `luaJM6FleetSpotted` issued three `NavigatorAttackMove` orders (bindings: attackmove 0 -> 3,
  issued 5 -> 8).
- The battle followed: deaths 5 -> 2, hit records 405 -> 168, shots 652 -> 246, and one more
  unit row.
- Every moved row traces to those orders, the callback's own effect.

**Verdict: `kLuaReconListenersBound = true`.**

### Provisional ledger names for the listener slots

These are hypotheses, not recovered symbols, with their evidence in the ledger note.

| address | name |
| --- | --- |
| `00972520` | `BSP_KillSubscription_Load` |
| `0096ACE0` | `BSP_KillSubscription_Matches` |
| `00979140` | `BSP_SubscriptionSet_MatchesOrEmpty` |
| `00972450` | `BSP_ReconSubscription_Load` |
| `00968470` | `BSP_ReconSubscription_Matches` |
| `00980E50` | `BSP_WarningManager_DispatchReconChannel` |

### SetInvincible, the native (packet `cc9_set_invincible_native`)

**The binding.**
- `GameMissionLuaHost::run_set_invincible_00897a50` takes the row in `binding_trampoline`.
- It reads argument 1 as the image does:
  - a boolean gives 1.0 or 0.0 (`00897B6F`);
  - anything else is its number, so a nil or missing argument reads 0.0, the release.
- It resolves argument 0 as `Kill` does.
- It calls `GameGunneryHost::set_unit_invincibility` for the slot and, for a squadron's fused
  slot, for each live member. That is `0042ED80`'s fan-out to children. The host's other children
  are guns, which carry no health.
- **No switch of its own.** The setter only stores `unit+150h`, and every reader (the damage
  floor, the sink gate and the `007BC5B0` water gate) is gated on `kUnitInvincibilityFloorBound`
  in `src/game_hosts_gunnery.cpp`. That switch is file-local, so the Lua host cannot share it.
  Until it is on, this binding changes only the native table row and a census line.
- **The census:**
  - `summary mission script set invincible calls=.. units=.. unresolved=..`;
  - one `SetInvincible 00897a50: "<name>" value=.. units=..` line per call.
- **SUBSTITUTION, labelled:** an entity with no units-host slot counts as `unresolved`.

**Predictions for the joint pair** (gunnery3 runs it: `kUnitInvincibilityFloorBound` flipped,
this native present on both sides; USN02 9200/9000; streams ON, `BSP_DEATH_TABLE=1`, lockstep
0.05, idle player):

- **The calls.** There are 10 calls, and the per-call lines name them:
  - `luaInit` floors DeRuyter, Java, Kortenaer and Electra at 0.1 (`usn_2_java.lua` 230);
  - `luaInit` floors Samidare, Murasame and Harusame at 0.5 (310);
  - the three remaining calls fit the release of Samidare, Murasame and Harusame in
    `luaPh2MovieEnd` (755).
- **Death rows, with the floor reading the field:**

| unit | OFF (`kill_off_usn02`) | ON |
| --- | --- | --- |
| Kortenaer | 68.30 s | no death, unless the release has run first |
| Electra | 108.60 s | no death, unless the release has run first |
| DeRuyter | 176.86 s | no death, unless the release has run first |
| Java | 184.26 s | no death, unless the release has run first |
| Samidare, Murasame, Harusame | 161.56, 190.66, 212.81 s | no death before the release line |

  - None of the seven floored ships can die to damage while its floor holds.
  - The Dutch group's release is `SetInvincible(unit, false)` at 761, followed by
    `AddDamage(unit, 100000000)`. If the census shows no such calls, the four Dutch ships
    survive the run.
  - The remaining death rows move with the battle.
  - **Expected verdict: exit 3.**
- **USN04 4700/4500.** One call, `Yorktown-class01` at 0.24, which takes no damage. Identity,
  exit 1.
- **USN01.** Its calls (`usn_1_marshall.lua` 254, 412, 713) land on units that the OFF log
  should show taking damage or not. Identity is predicted unless one of them dies OFF below its
  floor. The per-call lines name them.

## SetForcedReconLevel, 008AA8F0 (packet `cc9_forced_recon_level`, a read and a plan)

Worker cc9-lua2, 2026-09-28. This is item 4 of the ranking. Ghidra was read only.

### The image (V)

- **`SetForcedReconLevel(entity, level, party)`.** It resolves argument 0 (`00888AA0`), reads
  argument 1 as an integer (the level, `00B66290`) and argument 2 as an integer (the party).
- It calls `00805CF0` on the entity's recon record for that party.
  - For a squadron (`IsKindOf(18h)`) or a LandConvoy (`1Ah`), it calls it on each of the `+3CCh`
    members instead.
- **`00805CF0`**, `__thiscall(record, level)`:
  - it takes the effective level before (`+8h` when the force byte `+10h` is set, else `+4h`);
  - it sets `+10h = 1` and `+8h = level`, and drops any pending observer pair at `+28h`;
  - when the effective level changed, it calls `[record+2Ch]->vtable[0](record+30h party, old,
    new)`. That is `0077B0C0`, the same notify that fills `reconlevel` and fires the `recon`
    listeners.
- **The recon pass then publishes the forced level.** It skips the sensor test for that record
  (`det+10h`, `00806883`).

### What the host lacks

- `ReconSensorPassHost::unit_detection_forced(index)` and `unit_forced_level(index)`
  (`include/bsp/recon_sensor_pass.hpp`) are keyed by **target only**. The image's force byte is per
  (target, observing party) record.
- The gunnery host implements both as constant false/none (`src/game_hosts_gunnery.cpp` 4437).
- The pass loop already has the side in hand (`src/recon_sensor_pass.cpp`, `env.slot_index =
  side` before the target loop), so the fix is a signature change and a store.

### The plan

1. **`include/bsp/recon_sensor_pass.hpp` and `src/recon_sensor_pass.cpp`:**
   - the two hooks become `unit_detection_forced(int side, std::size_t index)` and
     `unit_forced_level(int side, std::size_t index)`;
   - the loop passes `side`.
2. **The gunnery host** (gunnery3's file):
   - it keeps `std::map<std::pair<int, std::size_t>, int> forced_recon;`;
   - it exposes `void set_forced_recon_level_00805cf0(std::size_t unit, int side, int level);`;
   - the two overrides answer from the map (the level maps to none/blip/identified as 0/1/2).
3. **The Lua host** (this worker): `kForcedReconLevelBound`, OFF.
   - The native resolves the entity as `Kill` does (a squadron's fused slot to its live members)
     and calls the setter.
   - The recon table and listener paths already follow the pass, so they see the forced level at
     the next pass.
   - **SUBSTITUTION, labelled:** `00805CF0`'s immediate notify becomes the next pass's change.

### Predictions sketch (USN13 3200/3000)

- **The calls.** There are 15:
  - `luaObj_AddUnit("primary", 3, unit)` then `SetForcedReconLevel(unit, 2, PARTY_ALLIED)` over
    `Mission.TotalTrgs` (`usn_13_truk.lua` 1140);
  - one per attack-wave squadron (1647).
- **The effect.** Those Japanese units become identified to the Allied side from the next pass.
  - Allied recon counters move.
  - The ship and aircraft AI that reads the pass's levels (target choice, `luaGetShipsAround`'s
    recon tables) can engage earlier, so **exit 3 is likely**.
- **Elsewhere.** USN01, USN04 and USN02 make no call on the idle runs, so they are identity.

### The binding (`kForcedReconLevelBound`, committed OFF)

- **The table.** `bsp::set_forced_recon_level_00805cf0(target, side, level)` fills a process-wide
  (side, target) table in `src/recon_sensor_pass.cpp`.
  - `recon_sensor_pass_step_008073c0` consults it for every tested target, after the host's
    per-target hooks. A forced entry skips the sensor test and publishes the forced level, as
    `det+10h` does.
  - The table stays empty unless the native writes it, so the pass is unchanged with the switch
    off.
  - The gunnery host is not touched.
- **The native.** `GameMissionLuaHost::run_set_forced_recon_level_008aa8f0` reads
  (entity, level, party) and resolves the entity as `Kill` does. A squadron's fused slot forces
  its live members.
- **The census:**
  - `summary mission script forced recon bound=.. calls=.. units=.. unresolved=..`;
  - one `SetForcedReconLevel 008aa8f0:` line per call.

**SUBSTITUTIONS, labelled:**
- The force is published at the next pass; `00805CF0` notifies at once.
- A LandConvoy forces its own slot.
- Unresolved entities are counted.
- The table is not cleared between missions; a run is one mission.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| row | prediction |
| --- | --- |
| USN13 3200/3000 | `calls=15`. The Japanese primary-3 targets and the attack-wave squadrons are published as identified to the Allied side (party 0) from the next pass on. `recon sensor_pass identified` rises, and every Allied AI path that reads the pass's levels sees them sooner. **Exit 3 likely**; every moved row should trace to Allied engagement of those units |
| USN01 3200/3000 | no call on the current OFF log (`kill_off_usn01`; the script's line 836 is not reached); identity |
| USN04 4700/4500 | no call, identity |

#### SetForcedReconLevel pairs and verdict

**Setup.**
- OFF is this tree's build of `f23f4f1d1`.
- ON is `pair_export --flip kForcedReconLevelBound=true` of the same commit (`local/fr_on`,
  SHA-256 `58214AE63F3E`).
- The logs are `local/fr_{off,on}_<mission>.log`.

| row | census | pair_diff | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN13 3200/3000 | `calls=15 units=60`; recon pass `forced=0 -> 2254`, `identified 22993 -> 23622`, `blip 7 -> 0`, `none 13270 -> 12648` | exit 1: gameplay identical, with 27 death rows and 211 unit rows | exit 3 likely | **failed on spread**: the force is published (the pass counters move), but no AI path on this idle run acts on the earlier identification |
| USN01 3200/3000 | `calls=0` | exit 1 | identity | held |
| USN04 4700/4500 | `calls=0` | exit 1 | identity | held |

**Verdict: `kForcedReconLevelBound = true`.**
- The mechanism is the image's: the forced records publish their level, and the rows the
  script names reach Allied level 2 at once.
- The gameplay prediction overstated the spread and is recorded as failed.

### Why LOMP06's seaplane listener was silent (packet `cc9_recon_level_step_check`)

**The question.** Does the image's recon pass step a party's level `0 -> 1 -> 2` across passes, or
jump `0 -> 2`? **Neither, in the listener's terms.** It re-notifies every level on every pass.

**The image (V).**
- **`008073C0` resets every record** at the start of each pass (`00807490 CALL 00805BE0`).
- **`00805BE0 BSP_Recon_ResetDetection`:**
  - it takes the record's effective level: `+8h` when the force byte `+10h` is set, else `+4h`;
  - it zeroes the value `+0Ch` and the level `+4h`;
  - it drops the observer pair at `+28h`;
  - when the effective level changed, it calls `[+2Ch]->vtable[0](party +30h, old, new)`. That
    is `0077B0C0`, which feeds `reconlevel` and the `recon` channel (`00980E50`).
- **So a detected record notifies `old -> 0` at the reset,** and `00805AF0` then notifies `0 -> new`
  when the pass publishes its level again (`00805B98`..`00805BD8`).
- **A forced record keeps its effective level** through the reset, so it does not cycle.
- **An own unit is refreshed with `+1.0` each pass** (`008065B0`), which saturates the value, so it
  reads 2 from every pass.

**The consequences.**
- A listener asking for `oldLevel {0}, newLevel {2}` fires on **every pass** for every record that
  is identified at that pass. That includes the party's own units.
- `luaSeaplaneSpotted` guards itself for exactly this: it acts only when
  `entity.Class.Type == "SmallReconPlane"`, then removes the listener.

**What the host did.**
- `dispatch_recon_listeners_00980e50` fired only on net changes between passes.
- It stepped the own party `0 -> 1 -> 2`.
- The LOMP06 pass publishes mostly blips (`blip=8229`, `identified=3787` over 99 passes).
- So an Allied `0 -> 2` jump was rare, and none reached the listener: `fires=0`.

**The correction** is `kReconListenerResetCycleBound` in `include/bsp/game_hosts_lua.hpp`,
committed OFF.
- Each pass, every non-forced record with a level fires `old -> 0`, then `0 -> new`.
- A forced record (`bsp::forced_recon_level`) fires only on a net change.
- The own party reads 2 each pass.
- **SUBSTITUTION, labelled:** the two transitions fire together at the host's frame, not inside
  the pass.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| row | prediction |
| --- | --- |
| LOMP06 1200/1000 | `listener_SeaplaneSpotted` fires on every pass for each Allied-identified unit and each own Allied unit, so `fires` goes from 0 to hundreds. The callback returns unless the entity is a `SmallReconPlane`; if one is identified, a dialog starts and the listener is removed. **Identity (exit 1)**, since the dialog is presentation |
| JM06 3200/3000 | `usnsubListener` and `fleetrecon` fire at or before their current times (the first pass in which a matching unit is identified after registration). If `fleetrecon` fires earlier, the group-1 submarines' attack orders move earlier and **JM06 moves (exit 3)**. If at the same pass, identity |
| USN01, USN04 | no `recon` entry is registered; identity |

#### Reset-cycle pairs and verdict

**Setup.**
- OFF is this tree's build of `477d7c758`.
- ON is `pair_export --flip kReconListenerResetCycleBound=true` of the same commit (`local/rc_on`,
  SHA-256 `7CC083D93298`).
- The logs are `local/rc_{off,on}_<mission>.log`.

| row | OFF census | ON census | pair_diff | verdict |
| --- | --- | --- | --- | --- |
| LOMP06 1200/1000 | `changes=387 fires=0` | `changes=35137 fires=2174`, for example the four `Overcast boat 0N` at `party 0 0 -> 2`, 30 passes each | exit 1 | held: the listener fires every pass, and the callback returns because no `SmallReconPlane` is identified |
| JM06 3200/3000 | `changes=74 fires=2` | `changes=1921 fires=2`: the same two callbacks, `luaJM6USNSubSighted` and `luaJM6FleetSpotted` | exit 1 | held: both listeners remove themselves on their first fire, and it comes at the same pass |

**Verdict: `kReconListenerResetCycleBound = true`.** The silent-listener failure of the recon verdict
is explained and corrected: the host fired only net changes, while the image re-notifies every
pass.

## AddDamage, 0088E000 (packet `cc9_lua_add_damage`, `kLuaAddDamageBound`, committed OFF)

Worker cc9-lua2, 2026-09-28. The contract is `docs/USN02_PHASES.md` section 3.

**The image.**
- `AddDamage(entity, amount)`, body `0088E000`-`0088E1A3`, resolves argument 0 (`00888AA0`)
  and reads argument 1 as a number (`0088E0DE`).
- It calls `entity->vtable[1ACh](amount)` at `0088E15B`. That is the unit's routed damage entry,
  `0095DA00` -> `0087D730` -> `00879070`: the party multiplier, the invincibility floor and the
  death rule.
- `bsp::lua_add_damage_0088e000` reconstructs the native.

**The binding.**
- `GameMissionLuaHost::run_add_damage_0088e000` resolves the entity as `Kill` does.
- It calls `GameGunneryHost::apply_script_damage_0095da00(unit, amount)`, which is gunnery3's
  routed path.
- The census is `summary mission script add damage bound=.. calls=.. units=.. unresolved=..`,
  plus one `AddDamage 0088e000:` line per call.
- **SUBSTITUTION, labelled:** an entity with no units-host slot is counted `unresolved`.

**Predictions.**
- **The current runs** (main `32f3d4f74`, invincibility floor ON). USN02 fails in phase 1
  (Exeter sunk at 385.68 s) and never reaches `luaPh2MovieEnd`. So USN02 9200/9000, USN01
  3200/3000 and USN04 4700/4500 are **identity, with `calls=0`**.
- **A phase-2 USN02 run** (kept for the flip pair).
  - `luaPh2MovieEnd` calls `SetInvincible(unit, false)` then `AddDamage(unit, 100000000)` for
    each DRGrp ship still alive.
  - DeRuyter, Java, Kortenaer and Electra die at the movie's end, in the same tick, unless
    already sunk.
  - Deaths rise by those not yet sunk; the FinalShips' targets and the later hit rows shift.
  - The switch stays OFF until the lead calls that pair.

### AddDamage identity pairs

**Setup.**
- OFF is this tree's build of `330419917`.
- ON is `pair_export --flip kLuaAddDamageBound=true` (`local/ad_on`, SHA-256 `DE070BDEB689`).
- The logs are `local/ad_{off,on}_<mission>.log`.

**Result.** USN01 3200/3000, USN04 4700/4500 and USN02 9200/9000 are each pair_diff exit 1
(gameplay identical), and the ON census reads `bound=1 calls=0 units=0 unresolved=0` on all
three. That is as predicted: USN02 fails in phase 1 and never reaches `luaPh2MovieEnd`.

**The switch stays OFF.** The flip pair waits for a phase-2 USN02 run, on the lead's call.

## Firing `hit` (packet `cc9_lua_hit_listeners`, `kLuaHitListenersBound`, committed OFF)

Worker cc9-lua2, 2026-09-28.

**The image (V, from the listings).**
- The subscription is vtable `00D1B740`, constructor `0097C2C0` (80h), loader `009725B0`. The
  loader reads:
  - `callback` into `+4h`;
  - `target` into `+0Ch`, `targetDevice` into `+1Ch` and `attacker` into `+2Ch`, through
    `009721C0`;
  - `attackType` into `+3Ch`, through `00970FF0`;
  - `attackerPlayerIndex` into `+4Ch`, through `009722D0`;
  - `damageCaused` into `+5Ch`, `fireCaused` into `+68h` and `leakCaused` into `+74h`, through
    `0096AAD0`.
- The dispatcher is `00988510`, whose producer is `0077CE60`, the attribution step of every
  applied hit.

**The binding.**
- `dispatch_hit_listeners_00988510` drains `GameGunneryHost::take_hit_events()` once per frame.
  gunnery3's queue is pushed after `0077CE60`'s attribution.
  - It drains even with the switch off, so the queue cannot grow.
- For each hit, it fires each `hit` entry that meets all of these:
  - `target` is empty or holds the victim;
  - `attacker` is empty or holds the shooter;
  - `attackType` is empty or holds the bullet class's `Type`, compared case-insensitively;
  - `damageCaused`, if it has two values, brackets the applied damage.
- **The census:**
  - `summary mission script hit listeners bound=.. events=.. fires=.. unmodelled=..`;
  - one `hit listener 00988510:` line per call.

**SUBSTITUTIONS, labelled:**
- The channel is evaluated at the host's frame.
- An entry with a non-empty `targetDevice`, `attackerPlayerIndex`, `fireCaused` or `leakCaused`
  is not matched; it is counted `unmodelled`.
- Callbacks are called with no argument. The measured callbacks take none.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| row | registered `hit` entries (from the listener census) | prediction |
| --- | --- | --- |
| USN01 3200/3000 | `ConLeadListener`: `target {Mission.ConLeader}`, `attackType {TORPEDO, BOMB, ROCKET}` | fires only if the convoy leader takes a torpedo, bomb or rocket hit. `luaConLeadHit` then `GenerateObject`s six attack planes (ConTBD1..3, ConSBD1..3), so **USN01 moves (exit 3)**. With no such hit, identity |
| JM06 3200/3000 | `playerHit` (`target {Mission.PlayerUnit}`, all else empty); `hshit` (`attackerPlayerIndex {PLAYER_1}`) | `playerHit` fires on the first hit on the player's submarine. `luaJM6PlayerHit` removes it and starts a dialog, so identity. `hshit` is `unmodelled` and never fires |
| LOMP06 1200/1000 | `listener_NarwhalDC` (`target {Mission.PlayerUnit}`, `attackType {DEPTHCHARGE}`) | fires only on a depth-charge hit on the player: a dialog, so identity |
| USN02, USN04 | none | identity |

### `hit` pairs and verdict

**Setup.**
- OFF is this tree's build of `aa15a4e0c`.
- ON is `pair_export --flip kLuaHitListenersBound=true` (`local/ht_on`, SHA-256 `31CF32B31F3B`).
- The logs are `local/ht_{off,on}_<mission>.log`.

| row | ON census | pair_diff | verdict |
| --- | --- | --- | --- |
| USN01 3200/3000 | `events=186 fires=0` | exit 1 | held on the no-hit branch: the convoy leader takes no torpedo, bomb or rocket hit, so `luaConLeadHit` never runs |
| JM06 3200/3000 | `events=147 fires=0` | exit 1 | held: the player's submarine, `PlayerSub 01`, takes no hit (`taken 0`), so `playerHit` stays silent |
| LOMP06 1200/1000 | `events=0 fires=0` | exit 1 | held |
| USN04 4700/4500 | `events=670 fires=0` | exit 1 | held: no `hit` entry |
| USN02 9200/9000 | `events=4918 fires=0` | exit 1 | held: no `hit` entry |

**Verdict: `kLuaHitListenersBound = true`.** It is inert on these idle runs. The first run in
which the convoy leader or the player is hit exercises it.

## AAEnable, 0089C740 (packet `cc9_lua_aa_enable`, `kLuaAAEnableBound`, committed OFF)

Worker cc9-lua2, 2026-09-28. This is item 5 of the ranking. Ghidra was read only.

**The image (V).**
- `AAEnable(entity, flag)` resolves argument 0 (`00888AA0`), takes the director through
  `vtable[114h]` and reads argument 1 with `00B66250` (lua_toboolean).
- When the director exists, it calls `0071E050 BSP_WeaponDirector_SendSubKind4Message(flag)`.
  That sends session message `5Ah` with sub-kind 4, whose receiver stores the flag at
  **director+221h** (`0071C246`): `aaEnabled`, next to the artillery flag at `+220h`.
- **The readers:**
  - the gunnery stance `008624C0` (`anti_air`, the flak category, `include/bsp/unit_gunnery_pass.hpp`);
  - the ship AI's approach (`009F2E0F`, `aa_12b8`);
  - `009F1BC0`.

**The host.**
- The four director enables are the name-keyed scene table
  (`bsp::game::scene_director_enables_set/find`). The gunnery host reads it each pass
  (`apply_director_stance_008624c0`, `kShipDirectorEnablesBound`, ships only), and the ship AI
  reads it through `enables()`.
- **The binding** copies the unit's entry, or the constructor's all-true default (`007202FD`),
  sets `anti_air` and writes it back.
- **The census** is `summary mission script aa enable bound=.. calls=.. disables=..
  unresolved=..`, plus one line per call.

**SUBSTITUTIONS, labelled:**
- The flag is written at the call, not delivered by session message `5Ah`.
- Every units-host slot is taken to have a director.
- An entity with no slot is counted `unresolved`.
- The gunnery stance applies it to ships only, as `kShipDirectorEnablesBound` does.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| row | prediction |
| --- | --- |
| BSM01 3200/3000 | `calls=33`, most of them disables. They come from the stage init's loops (`bsm_01` 404..468 over the battleship-row groups and others) and the PT-boat lines (1579, 1841); the `true` lines (1632..1670) belong to a later phase. **Identity, exit 1**: the last BSM01 log (`rb6_bsm01`) has no hit, no shot and no death in 3000 frames, so no AA target is in reach. **Named risk:** the ship AI's approach reads `+221h` (`aa_12b8`) only with a raw target, and there is none |
| USN01 3200/3000 | no call, identity |
| USN04 4700/4500 | no call, identity |

### AAEnable pairs and verdict

**Setup.**
- OFF is this tree's build of `d6d6e7d16`.
- ON is `pair_export --flip kLuaAAEnableBound=true` (`local/aa_on`, SHA-256 `A519BA433D22`).
- The logs are `local/aa_{off,on}_<mission>.log`.

| row | ON census | pair_diff | verdict |
| --- | --- | --- | --- |
| BSM01 3200/3000 | `calls=33 disables=33 unresolved=0` | exit 1 | held: no AA target within 3000 frames (the OFF run has no hits) |
| USN01 3200/3000 | `calls=0` | exit 1 | held |
| USN04 4700/4500 | `calls=0` | exit 1 | held |

**Verdict: `kLuaAAEnableBound = true`.** The 33 stage-init disables are applied. The first BSM01
run that reaches the air raid measures them.

### AddDamage: the flip pair's prediction, before the ON run (main `119ad0a44`)

**The fresh OFF run.**
- The log is `local/ad2_off_usn02.log`, USN02 9200/9000.
- It reaches phase 2, and `AddDamage` is recorded `calls=4` (UNIMPLEMENTED).
- `luaPh2MovieEnd` runs at **about mission frame 3131, 156.5 s**. That is the release block: its
  `SetInvincible(..., false)` lines for Yudachi, Samidare, Murasame and Harusame, then DeRuyter,
  Java, Kortenaer and Electra.
- The four Dutch ships die in combat later: Kortenaer at 157.51 s, Electra at 175.41, Java at
  190.46 and DeRuyter at 199.21.

**Predicted ON** (`kLuaAddDamageBound` flipped only):
- `calls=4 units=4`.
- DeRuyter, Java, Kortenaer and Electra die together at about 156.5 s (frame 3131), with no
  combat killer.
- Their later combat death rows go.
- Every other moved row lies downstream of those four removals: the FinalShips' targets, and the
  Japanese and Allied hits and deaths after 156.5 s.
- USN01 and USN04 are identity.

### AddDamage flip pair and verdict

**Setup.**
- OFF is this tree's build of `119ad0a44` (`local/ad2_off_<mission>.log`).
- ON is `pair_export --flip kLuaAddDamageBound=true` of the same commit (`local/ad_on`, SHA-256
  `A851697B4EF2`, logs `local/ad2_on_<mission>.log`).

| row | result | verdict |
| --- | --- | --- |
| USN02 9200/9000 | `calls=4 units=4`: DeRuyter, Java, Kortenaer and Electra, each `amount=1e8`, all dying at **156.60 s** (they died at 199.21, 190.46, 157.51 and 175.41 s OFF). pair_diff exit 3: deaths 22 -> 23 (John2 only ON), hit records 881 -> 747, damage 49930 -> 45340, with 23 unit rows changed downstream | held |
| USN01 3200/3000 | exit 1 | held |
| USN04 4700/4500 | exit 1 | held |

**Failed sub-prediction.** I predicted "no combat killer". The four death rows keep each ship's
last attacker from before the scuttle (Yudachi, Samidare, Murasame, Houston), because the routed
damage leaves the victim's attribution block as the last hit set it.

**Verdict: `kLuaAddDamageBound = true`.**

## The unimplemented Lua natives, refreshed (packet `cc9_lua_natives_ranking`, head `13fd0df8b`)

Worker cc9-lua2, 2026-09-28. The census reads the final native tables of these runs, all from
this tree at the head (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| mission | log |
| --- | --- |
| USN02 9200/9000 | `local/ad2_on_usn02.log`, which reaches phase 2 |
| JM06 3200/3000 | `local/rk_jm06.log` |
| LOMP06 1200/1000 | `local/rk_lomp06.log` |
| USN13 3200/3000 | `local/rk_usn13.log` |

Kill, SetInvincible, the listeners, SetForcedReconLevel, AAEnable and AddDamage are bound since
the first ranking.

| native | address | missions | calls | reach |
| --- | --- | --- | --- | --- |
| `SetShipSpeed` | `00890D30` | LOMP06 | 20 | **gameplay**: the convoy's speed (`Mission.ConvoySpeed`, `06_crucial_cargo.lua` 205..219), two escorts at 20 (558, 570), and the player at its maximum (111) |
| `UnitGetAttackTarget` | `008A6DE0` | LOMP06 | 21 | the report branch (696): nil returns early; a target leads to `luaGetReconLevel` and `luaSubC1AddUnit` (objectives) |
| `SquadronSetSpeed` | `0089F780` | USN13 | 15 | gameplay: the Japanese attack waves' speed (1641) |
| `IsClassChanged` | `008CC4B0` | USN13, JM06, LOMP06 | 47 | a script branch on the player's class change |
| `SetSubmarineDepthLevel` | `00893F40` | JM06 | 5 | gameplay: scripted submarine depth |
| `SetAirBaseSlotCount` | `008963E0` | USN13 | 3 | the deck slot count |
| `IsGUIActive`, `DisplayScores`, `EnableInput`, `BlackBars`, `MissionNarrative`, `SetGuiName`, `SetNumbering`, hints, `Loading_*`, `DisplayUnitHP`/`HideUnitHP`, `PrepareClass` | | 1..4 | 1..116 | presentation |
| `SetUnlimitedAirSupply`, `SetDeviceReloadEnabled`, `LoadCheckpoint`, `IsInFormation` | | 1 | 1..2 | small |

**Next packet: `SetShipSpeed` (`00890D30`),** measured on LOMP06 1200/1000. The convoy should
move at its scripted speed from the stage init, so exit 3 is likely.

## SetShipSpeed, 00890D30 (packet `cc9_lua_set_ship_speed`, `kLuaSetShipSpeedBound`, committed OFF)

Worker cc9-lua2, 2026-09-28. This is item 1 of the refreshed ranking.

**The image (V).**
- `SetShipSpeed(entity, speed)` resolves argument 0 (`00888AA0`) and reads argument 1 as a number.
- It stores `max(speed, 0)` at `[entity+73Ch]+24h` and the mission clock `[00F876A4]` at `+28h`
  (`00890E6F`), with no class test.
- That is the commanded-speed pair the cruise path divides by the reference speed
  (`docs/UNIT_COMMANDED_SPEED.md`, `docs/CRUISE_SPEED_SETTING.md`). It also makes the weapon
  director's idle tail choose `cruise` over `stop`.

**The binding.**
- `GameMissionLuaHost::run_set_ship_speed_00890d30` calls the existing
  `GameUnitsHost::store_commanded_speed_00890e6f`, the same store the `--order speed=` harness
  option makes.
- The census is `summary mission script ship speed bound=.. calls=.. units=.. unresolved=..`,
  plus one line per call.
- **SUBSTITUTION, labelled:** an entity with no units-host slot is counted `unresolved`.

**Predictions** (streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle player):

| row | prediction |
| --- | --- |
| LOMP06 1200/1000 | **exit 3 likely**. There are 20 calls: `Mission.PlayerUnit` at its class `MaxSpeed` (111); the convoy at `Mission.ConvoySpeed` (205..219); and two escorts at 20 (558, 570), if reached. The convoy's ships cruise at the scripted speed instead of their authored or default one, and every moved row should trace to their changed positions. The player's call lands on the controlled unit, which the idle player holds |
| USN01, USN04, USN02 | no call, identity |

### SetShipSpeed pairs and verdict

**Setup.**
- OFF is this tree's build of `d0a527386`.
- ON is `pair_export --flip kLuaSetShipSpeedBound=true` (`local/sp_on`, SHA-256 `04633BE5E0C4`).
- The logs are `local/sp_{off,on}_<mission>.log`.

| row | result | verdict |
| --- | --- | --- |
| LOMP06 1200/1000 | `calls=20 units=20`. The convoy and escorts (Asagiri, Kitakami, Gyoraitei 1/2, the Maru freighters and the rest) take their scripted speeds; `00890E6F` stores go 3 -> 23. The cruise path runs (`CruiseState` and `ShipAiState::cruise_step` rows, 63 calls each). pair_diff exit 3: 19 unit rows moved, first hit -1 -> 38.95 s, hits 0 -> 4, shots 6 -> 9, no deaths | held |
| USN01 3200/3000 | `calls=0`, exit 1 | held |
| USN04 4700/4500 | `calls=0`, exit 1 | held |

**Failed sub-prediction.** I said the player's call (111) lands on a controlled unit the idle
player holds. The controlled Narwhal moved **352.61 -> 421.38 m**, so the commanded speed does
drive the controlled submarine on this path.

**Verdict: `kLuaSetShipSpeedBound = true`.**

## UnitGetAttackTarget, 008A6DE0 (packet `cc9_lua_unit_attack_target`, a read; binding open)

Worker cc9-lua2, 2026-09-28. This is item 2 of the refreshed ranking: LOMP06, 21 calls.

**The image (V, from the pseudocode).**
- It resolves argument 0 (`00888AA0`) and takes the director (`vtable[114h]`).
- **When `director->vtable[48h](2)` answers true**, the target is `director->vtable[2Ch]()`.
- **Otherwise** it reads the current command (`0071BE40`). When that command's
  `vtable[0Ch]()` kind is 1 or 2, the target is the resolved command target
  (`BSP_EntityCommand_ActiveTargetDescriptor`, then `BSP_CommandTarget_ResolveObject`).
- **The result.** A target with `+5Dh` clear is pushed as `thisTable[tostring(target+174h)]`;
  anything else is nil.

**Open before binding.**
- The director slot `48h` and its argument 2 are unread. It is likely a role or mode test that
  picks the player's own target over the AI command's.
- The director's `vtable[2Ch]` is unread.
- The command's `vtable[0Ch]` kinds 1 and 2 need mapping onto the host's command classes. The
  commands host has `current_command_0071be40` and `active_command_descriptor_0071eb60`.
- **What it would change.** LOMP06 asks on `Mission.PlayerUnit` (`06_crucial_cargo.lua` 696).
  With a live target, `luaSubC1luaReportEnemy` reaches `luaGetReconLevel` and `luaSubC1AddUnit`,
  which are objectives, so gameplay could move.

### The director slots and the command kinds (packet `cc9_unit_get_attack_target`, V)

Worker cc9-lua3, 2026-09-28. This closes the two open reads above.

**The director is the weapon director.** For a ship, the entity's `vtable[114h]` is `0080E150`
`MOV EAX,[ECX+738h]`. That object is built by `008366D0`, which stores the vtable `00D09F58`
over the base's `00D09EC0` (`008363E0`). For a squadron, `vtable[114h]` is `007ECFD0`
`MOV EAX,[ECX+348h]`, the `22Ch` block `0084D810` builds with the vtable `00D0BD98`.

| vtable | slot `2Ch` | slot `48h` | `48h(2)` |
| --- | --- | --- | --- |
| `00D09EC0` weapon director base | `008364E0` `MOV EAX,[ECX+238h]` (the fire target) | `008364B0`: true for 0 and 2 | true |
| `00D09F58` weapon director (ships) | `008364E0` | `00836790`: true for 0, 2 and 3 | true |
| `00D0BD98` squadron command block | `0071F150`: `0071EBF0` then `00521EA0` | `0084D8F0`: true for 0 and 1 | false |

- **Slot `48h` is a constant test on its argument.** No body reads the object. What the
  argument means is not established; the name is left open.
- **A ship always answers its fire target,** `director+238h`, the field `00835860` writes and
  the gunnery host reads as `fire_target`. Only a squadron reaches the command arm.
- **`00836790` has no Ghidra function.** Its body is `00836790..008367AE` inclusive (`RET 4` at
  `008367AC`, `INT3` at `008367AF`). It sits inside the range Ghidra gives `008366D0`.

**The command kinds.** The command's `vtable[0Ch]` is the class category in
`kEntityOrderCommandClasses` (`src/entity_orders.cpp`). Category 1 is `settarget`,
`artillery`, `strafe` and `dogfight`. Category 2 is `torpedo`, `divebomb`, `levelbomb`,
`dropkamikaze`, `depthcharge`, `rocket`, `kamikaze` and `attackmove`. So the command arm answers
the target of an attack order and nil for movement, `cleartarget` and the rest.

### The binding (`kLuaUnitGetAttackTargetBound`, committed OFF)

- `GameMissionLuaHost::run_unit_get_attack_target_008a6de0` takes the fire-target arm when the
  entity has a ship AI row, and the command arm otherwise.
- The fire-target arm resolves the ship AI row's `fire_target`, the name `00835860` last
  stored.
- The command arm uses the units host's `0071BE40`, the class category, `0071EB60` and
  `00521EA0`.
- A target that is `unit_active` is pushed as its `thisTable` slot. Anything else is nil.
- The census is `summary mission script attack target bound=.. calls=.. fire_arm=..
  command_arm=.. pushed=.. nil=.. unresolved=..`, plus one line per call.
- **SUBSTITUTIONS, labelled.** "Has a ship AI row" stands in for the ship's director class.
  `unit_active` stands in for the `+5Dh` removed byte. An entity with no units-host slot
  answers nil.

**Predictions** (written before the runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| LOMP06 1200/1000 | **exit 1.** All 21 calls take the fire arm on the Narwhal. The host already gives the Narwhal a fire target (`fire=Komaki Maru` in cc9-lua2's `sp_on_lomp06.log`), so most calls push it. `GetSubmarineDepthLevel` goes 21 -> about 42, since line 699 now runs. `GetProperty` `reconlevel` asks rise by the same number, since the seeded depth level is 1. `luaSubC1AddUnit` runs only if the target is one of the two crucial cargo ships picked by `luaPickRnd` and its allied recon level is at least 2. That would add `luaObj_AddUnit` and `MissionNarrative` calls, which are script state, not unit motion |
| USN01, USN02, USN04 | no call, exit 1 |

### UnitGetAttackTarget pairs and verdict

**Setup.**
- OFF is this tree's build of `8ff75c1d8`.
- ON is `pair_export --flip kLuaUnitGetAttackTargetBound=true` (`local/ga_on`).
- The logs are `local/ga_{off,on}_<mission>.log`.

| row | result | verdict |
| --- | --- | --- |
| LOMP06 1200/1000 | `calls=21 fire_arm=21 pushed=21`. Every call answers the Narwhal's fire target, **Yugiri**. `GetSubmarineDepthLevel` 21 -> 42, `GetProperty` `reconlevel` 0 -> 21, `MissionNarrativeClear` appears with 21 calls. Yugiri is a destroyer, not one of the cargo candidates, so `luaSubC1AddUnit` does not run and `MissionNarrative` stays at 1. pair_diff exit 1: gameplay, deaths and the unit table identical | held |
| USN01 3200/3000 | no call, exit 1, native table identical | held |
| USN04 4700/4500 | no call, exit 1, native table identical | held |
| USN02 9200/9000 | no call, exit 1, native table identical | held |

**Verdict: `kLuaUnitGetAttackTargetBound = true`.** The answer depends on the ship AI host's
fire target for the player's own submarine. The auto-target enable gate `009F5610` has no
player test (it reads director+3Dh and the head command's category only). Whether the tick
`009F5DA0` is reached for a player-controlled unit is the ship AI host's modelling, not
established by this packet.

## SquadronSetSpeed, 0089F780 (packet `cc9_squadron_set_speed`, `kLuaSquadronSetSpeedBound`, committed OFF)

Worker cc9-lua3, 2026-09-28. This is item 3 of the refreshed ranking.

**The image (V).**
- `SquadronSetSpeed(squadron, speed)` resolves argument 0 (`00888AA0`, `0089F880`) and reads
  argument 1 as a number (`00B66270`, `0089F8B1`).
- For `i` below `[entity+3CCh]` it takes `member = i < 5 ? [entity+3D0h+4i] : null` and calls
  `member->vtable[3Ch](speed)` (`0089F8CA..0089F8FF`). There is no class test. It returns no
  value.
- On all nine plane vtables, `vtable[3Ch]` is `0074E1E0` (the nine `.rdata` hits of its
  address, for example `00D19D64` = `00D19D28+3Ch`). That body calls `007D9E80` on
  `unit+AB0h`, the flight controller.
- **`007D9E80` sets the controller's velocity outright.** The body linear velocity becomes
  `(0, 0, speed)` at `ctl+3Ch..44h`. The body angular velocity at `ctl+48h..50h` becomes the
  zero vector at `00F87574`. Then `007D9C80` rotates both into world, and the world linear
  velocity is copied to `ctl+30h..38h`.
- So the speed is a one-shot airspeed along the plane's own forward axis, not a held setting.

**The binding.**
- `GameUnitsHost::set_plane_forward_speed_007d9e80` writes `plane_world_velocity` as the pose's
  forward row times the speed, mirrors it into the body's linear velocity and zeroes
  `plane_body_angular`. The spawn seed builds the same vector.
- `GameMissionLuaHost::run_squadron_set_speed_0089f780` walks the squadron registry's active
  `member_units`, at most five, as `SetForcedReconLevel` does.
- The census is `summary mission script squadron speed bound=.. calls=.. planes=..
  unresolved=..`, plus one line per call.
- **SUBSTITUTIONS, labelled.** The registry's active members stand in for the compacted
  `+3D0h` array. The `ctl+30h..38h` copy has no host field. An entity with no squadron record
  is counted unresolved.

**Predictions** (written before the runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| USN13 3200/3000 | **exit 3.** There are 15 calls from `luaAttackWaveSpawned` (`usn_13_truk.lua` 1641, mtime 2024-08-13), each on a spawned squadron at 67, with 4 planes each (the forced-recon lines of cc9-lua2's `rk_usn13.log` resolve 4 members). So `planes=60`. The wave's planes start at 67 m/s instead of their spawn seeds (61.1 and 55.6 m/s in that log). Their positions move from the spawn on, so the attack-wave unit rows, the first hits on the US carriers and possibly the AA kills move. That is RNG-coupled, so per-kill attribution is not claimed |
| USN04 4700/4500 | no call, exit 1 |

### SquadronSetSpeed pairs and verdict

**Setup.**
- OFF is this tree's build of `8a64f9a18`.
- ON is `pair_export --flip kLuaSquadronSetSpeedBound=true` (`local/ss_on`).
- The logs are `local/ss_{off,on}_<mission>.log`. The ON USN13 run was repeated after a
  renderer-init outage (every binary failed at `CreateDevice`, hr `0x88760868`, cleared by 22:46 local time).

| row | result | verdict |
| --- | --- | --- |
| USN13 3200/3000 | `calls=15 planes=60`, 4 per squadron, each at 67. pair_diff exit 3: deaths 24 -> 23, hit records 720 -> 572, shots 6607 -> 4265, first hit 68.10 -> 67.90 s. Every moved death row is an attack-wave plane (`bruh #1.*`), with its time, altitude, killer and range moved. Torpedo-task releases hold at 2 of 60 | held |
| USN04 4700/4500 | no call, exit 1, native table identical | held |

The attack-wave kills are coupled through the shared RNG stream and the AA engagement, so no
single kill is attributed to the speed. The Enterprise's distance moved by 0.06 m.

**Verdict: `kLuaSquadronSetSpeedBound = true`.**

## IsClassChanged, 008CC4B0 (packet `cc9_is_class_changed`, `kLuaIsClassChangedBound`, committed OFF)

Worker cc9-lua3, 2026-09-28. This is item 4 of the refreshed ranking.

**The image (V).**
- `IsClassChanged(id)` reads argument 0 as an integer (`00B66290`, `008CC5AF`).
- It pushes `[registry+2010h+id*4] != id` as a boolean (`008CC5CB CMP`, `SETNZ`, `00B66450`).
- `registry+2010h` is the inverse class-index map (`include/bsp/vehicle_class.hpp`).
- **Only two functions write it,** `00506550` and `00592640`. Each resets all 800h pairs to the
  identity and then stores one pair, the player's chosen ship.
- `00592640` is the `continue` footer command. Its pair comes from the profile's record for the
  selected mission (`007FC490` over `005806A0` `BSP_MainMenu_GetSelectedMission`).
- `00506550` does the same from a menu list selection (`+4B0h` against `+134h`); its caller is
  `00516010`.
- **So the answer is true only for the class the player swapped in.**

**Callers in this installation's Lua.** There are 175 lines naming it under `scripts/` (`*.lua`, one commented out). Every one is a
truthiness test (`if IsClassChanged(unit.ClassID) then`, six `not IsClassChanged(...)`, one `and IsClassChanged(...)`).
So nil and false read the same.

**The binding.**
- `GameMissionLuaHost::run_is_class_changed_008cc4b0` answers from a
  `bsp::VehicleClassIndexMap` reset to the identity.
- The census is `summary mission script class changed bound=.. calls=.. true=..`.
- **SUBSTITUTION, labelled.** This process does not model the profile record, and the footer
  command is recorded unimplemented (`src/game_hosts_mission.cpp`). So the map is the identity
  and every answer is false.
- An id outside the 800h entries answers false, where the image would read past the map.
- A harness that selects an alternative ship would need `00592640`'s pair first.

**Predictions** (written before the runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| LOMP06 1200/1000, JM06 3200/3000, USN13 3200/3000 | **exit 1** on each. `IsClassChanged` goes `UNIMPLEMENTED` -> concrete with the same call count, and `true=0`. Every caller's branch is unchanged, because false and nil are both falsy |

### IsClassChanged pairs and verdict

**Setup.**
- OFF is this tree's build of `10422054f`.
- ON is `pair_export --flip kLuaIsClassChangedBound=true` (`local/cc_on`).
- The logs are `local/cc_{off,on}_<mission>.log`.

| row | result | verdict |
| --- | --- | --- |
| LOMP06 1200/1000 | `calls=1 true=0`, exit 1. The only other line is the movie camera pose at frame 441 (z -5961.5 -> -5961.4). A repeat of the OFF run on the same binary (`cc_off2_lomp06.log`) gives -5961.4, so it is run-to-run noise | held |
| JM06 3200/3000 | `calls=4 true=0`, exit 1, death rows and unit table identical | held |
| USN13 3200/3000 | `calls=42 true=0`, exit 1, death rows and unit table identical | held |

The 47 calls match the refreshed ranking.

**Census defect, fixed in the same commit.** The native table showed these calls as
`UNIMPLEMENTED` with doubled counts (1 -> 2, 4 -> 8, 42 -> 84). The four rows this worker added
(`UnitGetAttackTarget`, `SquadronSetSpeed`, `IsClassChanged`, `SetSubmarineDepthLevel`) were
missing from the dispatcher's `handled` list. The unhandled path only logs, so no value was
pushed twice and behaviour is unaffected. The rows are now in the list.

**Verdict: `kLuaIsClassChangedBound = true`.**

## SetSubmarineDepthLevel, 00893F40 (packet `cc9_set_submarine_depth_level`, `kLuaSetSubmarineDepthLevelBound`, committed OFF)

Worker cc9-lua3, 2026-09-28. This is item 5 of the refreshed ranking.

**What the landed switch covers.** `kSubmarineDepthLevelBound` (`docs/SUBMARINE_MODEL.md`
section 11) binds only the getter `00894100` and the scene seed `00853630`. No depth writer
was modelled, so this native is the whole gap.

**The image (V).**
- `00893F40` resolves argument 0 (`00888AA0`) and reads argument 1 as an integer.
- A request for 1 becomes 0 when `+122Ch` (`periscopeState`) is 2, broken, or `+1214h` (the
  periscope node) is null. Only a kamikaze class has no node.
- Then it calls `008528B0` `BSP_SubmarineUnit_SetDepthLevel`, and it returns no value.
- `008528B0` clamps to 0..3 (`008528CC..008528E0`). It forces 1 when class `+510h` or `+514h` is
  above 0 (`008528E5..00852908`).
- **Only when `+1268h` differs** (`0085290D JE`), it stores the level (`0085291E`) and posts
  session message `A2h` with it through `0077C7B0` (`00852915..00852956`).
- The dive that follows, the hull moving to `bands[level]`, is the submarine's own per-frame
  work.

**The binding.**
- `GameUnitsHost::set_submarine_depth_level_008528b0` clamps with the existing
  `submarine_clamp_depth_command_008528b0` and stores into `GameUnitRow::submarine_depth_level`
  when it differs.
- `GameMissionLuaHost::run_set_submarine_depth_level_00893f40` routes the row there.
- The census is `summary mission script submarine depth set bound=.. calls=.. stored=..
  unresolved=..`, plus one line per call.
- **SUBSTITUTIONS, labelled.** `periscopeState` is never 2 here, and the kamikaze test reads
  false, as in the seed. The `A2h` message is recorded, not posted.
- **The dive is not modelled.** The host hull keeps its authored Y. So the only effect is the
  level that `GetSubmarineDepthLevel` answers.

**Callers.** JM06 runs `COTP-IJN/PRCPIJN/jm06.lua` (mtime 2024-07-13). It calls this native at
1380 (the spawned I-400 to 3), 1439 (the I-400 to 0), 1796 (`luaJM6SubInit`, each sub to 1 before
a `PutTo` at y = -20), 1842 (group 1 followers to 1) and 2373. Its `GetSubmarineDepthLevel` lines
(794, 1232) are commented out.

**Predictions** (written before the runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| JM06 3200/3000 | **exit 1.** 5 calls, all resolved. `stored` counts only the calls that change a seeded submarine's level. The subs that `luaJM6SubInit` sets to 1 were most likely seeded at 1 and store nothing; a sub seeded at 0 stores. Nothing in JM06 reads the level back, and the host has no dive, so gameplay is identical |
| LOMP06 1200/1000 | no call, exit 1 (the Narwhal's getter answers stay 1) |

### SetSubmarineDepthLevel pairs and verdict

**Setup.**
- OFF is a clean `pair_export` of `30a5d1615` (`local/sd_off`).
- ON is `pair_export --flip kLuaSetSubmarineDepthLevelBound=true` (`local/sd_on`).
- The logs are `local/sd_{off,on}_<mission>.log`.
- Both exports predate the `handled`-list fix in `b0c027e02`, so the native table counts the
  calls twice (5 -> 10).

| row | result | verdict |
| --- | --- | --- |
| JM06 3200/3000 | `calls=5 stored=0`, exit 1, death rows and unit table identical. All five calls are `luaJM6SubInit`'s and the group-1 followers' requests for level 1 on `PlayerSub 01..03`, each already seeded at 1, so `008528B0` stores nothing (`0085290D JE`). The I-400 calls (to 3, then to 0) are not reached in 3000 frames | held |
| LOMP06 1200/1000 | no call, exit 1 | held |
| USN01 3200/3000 (added at the lead's request) | no call, exit 1, native table identical | held |
| USN04 4700/4500 (added at the lead's request) | no call, exit 1, native table identical | held |

**Verdict: `kLuaSetSubmarineDepthLevelBound = true`.** The binding stores the level only.

**The gameplay gap that remains is the dive itself.** `A2h` is not posted, and the hull move to
`bands[level]` is not modelled. The first run that reaches the I-400's `SetSubmarineDepthLevel(unit, 3)`
(1380) changes the level the getter answers, but not where the hull sits.

## SetAirBaseSlotCount, 008963E0 (packet `cc9_set_air_base_slot_count`, `kLuaSetAirBaseSlotCountBound`, committed OFF)

Worker cc9-lua3, 2026-09-28. This is item 6 of the refreshed ranking.

**The image (V).**
- `008963E0` resolves argument 0 (`00888AA0`) and takes its air-ops block
  (`BSP_AirOps_GetBlock`). It reads argument 1 as an integer and calls `006C7E20(n)` on the
  block. It returns no value.
- `006C7E20` has one caller, this native. It resizes the `58h` slot array at `block+4Ch`, with
  its count at `+50h`, to exactly `n`.
- While the count is below `n` (`006C7E73 JAE`, looping back at `006C8060`), it appends a
  default record, built at `006C7EA3..006C7F1C`: class 0, assigned 0, requested 3, class+134h
  copy 0, no squadron, state 1, timer 0.0 and the launch request clear.
- While the count is above `n` (`006C8069..006C8089`), it destroys the tail record through its
  vtable.
- The count comparison is unsigned. There is no test for an entity without a block.

**The binding.**
- `GameMissionLuaHost::run_set_air_base_slot_count_008963e0` resizes the entity's deck in
  `bsp::air_ops_decks()` the same way.
- The census is `summary mission script air base slot count bound=.. calls=.. resized=..
  unresolved=..`, plus one line per call.
- **SUBSTITUTIONS, labelled.** An entity with no deck is counted unresolved. A negative `n` is
  ignored.

**Callers.** `usn_13_truk.lua` (mtime 2024-08-13) sets its three airfields to 4 slots, or 6 on
difficulty 2 (559, 563). It sets each US carrier to 4 (1096). `usn_19_coralus.lua` 212 is not a
measured row.

**Predictions** (written before the runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| USN13 3200/3000 | **exit 1.** The harness runs at difficulty 1 (`set_effective_difficulty game+6ACh=1`), so the airfields ask for 4. Every deck in `ss_off_usn13.log` is authored with `NumSlots=4`, so each call is a no-op (`resized=0`). The carrier loop at 1096 is a later stage and may not be reached |
| USN04 4700/4500 | no call, exit 1 |

### SetAirBaseSlotCount pairs and verdict

**Setup.**
- OFF is this tree's build of `b2b454eeb`.
- ON is `pair_export --flip kLuaSetAirBaseSlotCountBound=true` (`local/sc_on`).
- The logs are `local/sc_{off,on}_<mission>.log`.

| row | result | verdict |
| --- | --- | --- |
| USN13 3200/3000 | `calls=3 resized=0`. Airfield2, Airfield5 and JapAF go 4 -> 4. Exit 1, death rows and unit table identical | held |
| USN04 4700/4500 | **Two calls, not none.** USN04 runs `usn_19_coralus.lua` (mtime 2024-08-26), whose line 212 sets `Mission.Town` (`Yorktown-class01`) to 0 slots and 213 sets `Mission.Lex` to 4. `resized=1` (Yorktown 4 -> 0), air-ops `slot_ticks` 36000 -> 25858, two `IsReadyToSendPlanes` answers on Yorktown's deck gone. Exit 1: gameplay, 40 death rows and the unit table identical | **missed caller** |
| USN04 9200/9000 (added for the miss) | the same two calls; `slot_ticks` 72000 -> 52858. Exit 1, 51 death rows and the unit table identical | held |

**The failed prediction is a caller census miss, not the mechanism.** The ranking's first table
already listed `usn_19_coralus 212`, and I did not map that script to USN04. The binding does
what the script asks, and Yorktown's deck is never used for a launch on these rows.

**Verdict: `kLuaSetAirBaseSlotCountBound = true`, recorded.** `luaZuikakuMovieEnd` (1949)
restores Yorktown to 4 slots. Those slots are default records (class 0, requested 3), as in the
image, so the first run that reaches that stage may launch differently from Yorktown.

## The unfired `command` and `input` channels, a census (cc9-lua3, 2026-09-28)

Item 3 of the cc9-lua2 handoff. Before binding, I checked which measured row could show either
channel firing. This is a read of this installation's scripts; nothing is bound.

**`command`: no measured mission registers one.**
- **Correction.** The handoff and the plan above name JM06's `submove` (`jm06.lua` 1205) as the
  measure. It is registered only inside `luaJM6SubPathListener`, and no script in this
  installation calls that function (its only occurrence under `scripts/` is the definition). So
  JM06 never registers a `command` listener.
- The registrations that do run are in JM02 (`BettyRetreat` 1561, `NellRetreat` 1601), JM07
  (`fleet1move` 1435, `atlantamove` 2495) and JM13 (`junklistener` 2146), under
  `missions/COTP-IJN/`. None of these is a reference row.
- Binding `command` therefore needs one of those missions as its measured row. `submove`'s keys
  (`command = {"moveonpath"}`, `status = {"finish"}`) show that the channel reports a command's
  start or finish per entity.

**`input`: an idle player cannot fire it.**
- Every registration comes from `global/commandhelpers.lua`. It is either the in-game movie skip
  (`IngameMovieInputListenerID` -> `luaCamOnTargetExt`) or the `Launch_Airbase_Stock_N` keys.
- Both answer to player input. The harness plays idle, so on every reference row the ON run would
  be byte-identical to the OFF run for this channel.

### The `command` channel: no reachable row (cc9-lua3, 2026-09-28)

The lead approved JM07 3200/3000 as the measured row for `command`, with JM02 as the fallback.
Neither runs a `command` listener in this build. 300-frame smokes and the mission tree
(`scripts/datatables/missiontree.lua`, mtime 2025-06-02, modded) show why:

| menu id | scene | script the host runs | `command` listener |
| --- | --- | --- | --- |
| JM07 | `COTP-IJN/PRCPIJN/prcpijn_midway.scn` | `PRCPIJN/prcpijn_midway.lua` | none |
| JM02 | `COTP-IJN/PRCPIJN/prcpijn_02_force_z.scn` | `PRCPIJN/prcpjm02.lua` | none |
| JM13 | `COTP-IJN/ijn_16_ambushed_at_wake_island.scn` | `COTP-IJN/JM16.lua` | none |
| IJN13 | `IJN/ijn_13_yorktown.scn` | `Ijn/ijn_13_yorktown.lua` | none (only the movie-skip `input`) |
| JM06 | `COTP-IJN/PRCPIJN/ijn_06_prelude_to_midway.scn` | `PRCPIJN/JM06.lua` | `submove` is never registered (above) |

- **The scripts that do register one** are `jm02`, `jm06`, `jm07` and `jm13` under
  `missions/COTP-IJN/`, `jm06`, `jm07` and `prcpjm07` under `PRCPIJN/`, and four copies under
  `missions/ijn/JM/`.
- **No tree entry reaches them.** Under the host's script derivation, no scene in the tree maps
  to any of those files. The `ijn_02_force_z` and `ijn_07_invasion_of_midway` scenes exist on
  disk but no tree entry names them.
- **The caveat is withdrawn** (packet `cc9_scene_script_table`). The script is not derived:
  under `kSceneStageScriptBound` (`src/game_hosts_mission.cpp`) the host takes the scene record's
  `+928h` slot 8 (`+968h`). `004F1D70` fills it from the header property `GameStageScript`, or
  `StageScript` when that is absent. Every smoke and reference row logs
  `mission script name from the scene header`:

  | row | header script |
  | --- | --- |
  | JM07 | `COTP-IJN\PRCPIJN\prcpijn_midway` |
  | JM02 | `COTP-IJN\PRCPIJN\prcpjm02` |
  | JM13 | `COTP-IJN\JM16` |
  | IJN13 | `Ijn\ijn_13_yorktown` |
  | JM06 | `COTP-IJN\PRCPIJN\JM06` |
  | LOMP06 | `USN\LOMP\06_crucial_cargo` |
  | USN01 | `USN\usn_1_marshall` |

  So the scripts above are the image's own choice, and no reachable mission registers a `command`
  listener. The line that said "derived from the scene path ... not filled by the header pass"
  printed the final name whichever source won. It now names its source.
- **So the `command` channel stays unbound.** Binding it needs either the `+928h` script table,
  or a harness option that runs a named mission script.

### The unmodelled `hit` filters: no reachable test (cc9-lua3, 2026-09-28)

Item 4 of the cc9-lua2 handoff. `targetDevice`, `attackerPlayerIndex`, `fireCaused` and
`leakCaused` are still counted `unmodelled`, and nothing has reached that count on any run.

- **The only live user is `hshit`.** Among the entered rows it is the only entry that uses one of
  the four filters: JM06, `attackerPlayerIndex {PLAYER_1}` on the hospital ship, callback
  `luaJM6HospitalShipHitByPlayer`.
- **The hospital ship is never hit.** `unmodelled=0` on JM06 with the dive law off
  (`local/dv_off_jm06.log`) and on (`local/dv_on_jm06.log`). The count only rises after the
  `target` filter matches.
- **What the image does (read, 00988510).** The attacker player index is `+1Ch` of the record the
  ordnance's `vtable[108h]` returns, or `-1` with no ordnance. The dispatch only goes ahead when
  that index is below 8 and the shooting unit's role-0 slot `+1ACh` is below 8.
- **Not bound.** The producer of the record's `+1Ch` (stamped at fire time) is not read, and the
  host's hit events do not carry it. Binding the filter needs that producer and a row where the
  hospital ship is hit.

### The unmodelled `hit` filters, bound (packet `cc9_hit_listener_filters`, `kLuaHitFilterFieldsBound`, committed OFF)

Worker cc9-lua3, 2026-09-28. The lead asked for this after the census above: bind what the hit
queue can supply.

**What 00988510 hands the channel (read from the pseudocode).** There are eight parameters. The two entity boxes are taken in the loader's key order, which is
not traced in the listing:

| # | key | value | source |
| --- | --- | --- | --- |
| 1 | `target` | the victim | `00968060`, the victim entity |
| 2 | `targetDevice` | the hit record's own entity `[hit+0h]`, when it is live and not the victim; else none | the dispatcher's head |
| 3 | `attacker` | the shooting unit's id (`u16`) | boxed with vtable `00D1AF24` |
| 4 | `attackType` | the ordnance type name | `(&PTR_DAT_00e08e58)[kind]` |
| 5 | `attackerPlayerIndex` | `[src+1Ch]`, `src` being the ordnance's `vtable[108h]()`; -1 with no ordnance | boxed with vtable `00D1AF2C` |
| 6 | `damageCaused` | `hit+48h` | a float, vtable `00D1AF3C` |
| 7 | `fireCaused` | `hit+4Ch` | a float |
| 8 | `leakCaused` | `hit+50h` | a float |

**What the host can supply.**
- **`targetDevice`:** none. This process gives a ship's devices (guns, turrets) no entity of
  their own, and every hit lands on the unit. A non-empty `targetDevice` set therefore never
  holds the parameter.
- **`fireCaused` and `leakCaused`:** 0.0. No hit in this process starts a fire or a leak. The
  leak model has no leak points (`src/game_hosts_units.cpp`, `ShipHydroBinding`), and there is no
  fire model. A two-value range filter is tested against 0.0, the bracket `damageCaused` uses.
- **`attackerPlayerIndex`:** left unmodelled, still counted. The fire-time producer of
  `[src+1Ch]` is unread. The host's attribution source sets `origin_slot` from the shooter's side
  (`src/game_hosts_gunnery.cpp`, `unit_side_0054`), which is a stand-in, not `+1Ch`. The line
  gunnery4 would need is below.

**Accessor request for gunnery4** (`include/bsp/game_hosts_gunnery.hpp`, `GameGunneryHitEvent`):

```
        int attacker_player_index{-1};   // [src+1Ch], src = ordnance vtable[108h](); -1 with no ordnance (00988510)
        std::uint32_t device_entity{0};  // [hit+0h] when a live child other than the victim, else 0
        float fire_caused{0.0f};         // hit+4Ch
        float leak_caused{0.0f};         // hit+50h
```

**Also found, not bound: a rate limit.** Before evaluating the channel, `00988510` looks a key
pair up in the map at `this+168h` (`009882F0`, `00499030`). The pair is the victim and the
attacking unit, according to the pseudocode; the stack slots were not traced in the listing. It
skips the evaluation unless the stored time has passed, then stores `clock + 2.0` (`00CE3958`),
or `clock + 1e-4` (`00CE3C68`) for ordnance kinds 8..0Fh, 12h and 13h.
- So a gun duel fires a `hit` callback at most once per 2 s per pair. Torpedoes, bombs and the
  like are effectively unlimited.
- The host evaluates every hit. That matters only once a `hit` listener fires on gunfire. None
  does on the measured rows.
- It needs the ordnance kind `[[src+4h]+8h]`, which the host's `gun.category` does not map onto
  one to one.

**Predictions** (written before the ON runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player):

| row | prediction |
| --- | --- |
| JM06 3200/3000 | **exit 1.** The only entry with one of the four keys is `hshit` (`attackerPlayerIndex`), which stays unmodelled; the hospital ship is not hit anyway |
| LOMP06 1200/1000 | **exit 1.** `listener_NarwhalDC` uses `target` and `attackType` only |
| USN01 3200/3000 | **exit 1.** `ConLeadListener` uses `target` and `attackType` only |

#### Hit filter pairs and verdict

- OFF is this tree's build of `508502014`.
- ON is `pair_export --flip kLuaHitFilterFieldsBound=true` (`local/hf_on`).
- The logs are `local/hf_{off,on}_<mission>.log`.

| row | pair_diff | verdict |
| --- | --- | --- |
| JM06 3200/3000 | exit 0, byte-identical | held |
| LOMP06 1200/1000 | exit 1: only the frame-441 movie camera pose (z -5961.5 -> -5961.4), the run-to-run noise recorded above | held |
| USN01 3200/3000 | exit 0, byte-identical | held |

**Verdict: `kLuaHitFilterFieldsBound = true`.** It is inert on these rows, as predicted.
`attackerPlayerIndex` and the rate limit remain open, with the accessor request above.

### The hit-callback rate limit (packet `cc9_hit_rate_limit`, `kLuaHitRateLimitBound`, committed OFF)

Worker cc9-lua3, 2026-09-28, on gunnery4's hit-event fields (main `8eb180a29`).

**The image (V, listing).**
- `00988949` calls `009882F0` and `00988957` calls `00499030` on the map at `this+168h`. That
  returns the float stored for the pair, 0.0 for a new pair.
- `0098895C..00988969` (`FLD [00F876A4]`, `FLD [EBP]`, `FCOMIP`, `JBE`) evaluate only when that
  float is at or before the clock `DAT_00F876A4`. Otherwise the channel is skipped for this hit.
- On a pass it stores `clock + 2.0` (`00CE3958`), or `clock + 1e-4` (`00CE3C68`) for the kinds
  8..0Fh, 12h and 13h.
- `this+1A4h..+1ACh` shortens it to `clock + 1e-4` (`00D7A268`) when a list there is non-empty and
  its iterator is not at the head. That list is not read.
- **The key is the victim and the attacking unit.** In the listing, the victim half is the slot
  `[base+34h]`: `0098859B` stores `param_2->vtable[140h]()` there, masked by the `vtable[5Ch](2)`
  test, and `00988917` loads it into the key. The attacker half, `[base+80h]`, is `piStack_9c` in
  the pseudocode and was not traced in the listing.
- The limit runs on every hit before the channel lookup, whether or not any `hit` listener
  exists.

**The kind (verified).** `00988510` indexes the attack-type name table at `00E08E58` with the same
kind, `[[src+4h]+8h]`, forcing 11h for a kamikaze shooter. Its entries are:

| kind | name | kind | name |
| --- | --- | --- | --- |
| 0 | NONE | 0Ah | TORPEDO |
| 1 | BULLET | 0Bh | DEPTHCHARGE |
| 2 | MACHINEGUN | 0Ch | DUMMYTARGET |
| 3 | AAMACHINEGUN | 0Dh | DUMMYKAMIKAZEPLANE |
| 4 | ARTILLERY | 0Eh | DUMMYSUBMARINE |
| 5 | LIGHTARTILLERY | 0Fh | PARATROOPER |
| 6 | MEDIUMARTILLERY | 10h | FLAK |
| 7 | HEAVYARTILLERY | 11h | KAMIKAZE |
| 8 | EXTRAPOLATED | 12h | ROCKET |
| 9 | BOMB | 13h | WATERMINE |

- This is the post-`006E9890` id space: 1 becomes 2 or 3, and 4 becomes 5, 6 or 7. That is exactly
  what gunnery4's `ordnance_kind` carries: `bullet_sub_type`, the bullet class record's `+8h`
  after the rewrite.
- So guns (1..7), flak and kamikaze are limited to one evaluation per 2 s per pair. Bombs,
  torpedoes, depth charges, rockets, mines and the dummies are effectively unlimited.

**The binding.** The host's hit dispatcher keeps the map as `hit_rate_limit_` and applies the test
before its listener loop. The clock is the host's `DAT_00F876A4` (`spawn_world_clock_`). The
census is `summary mission script hit rate limit bound=.. passed=.. throttled=.. pairs=..`.

**SUBSTITUTIONS, labelled.**
- The kind is the event's `ordnance_kind`, and a kamikaze shooter's forced 11h is not modelled.
- The `this+1A4h` list test is not modelled.

**Predictions** (written before the ON runs). The OFF logs of this tree (`local/rl_off_*.log`)
show `fires=0` on every row. No `hit` callback fires today, so none can be suppressed.

| row | OFF hit census | prediction |
| --- | --- | --- |
| JM06 3200/3000 | `events=320 fires=0` | **exit 1.** `throttled` counts the repeat gun hits on a pair within 2 s, and no callback changes |
| USN01 3200/3000 | `events=538 fires=0` | **exit 1**, as above. `ConLeadListener` wants torpedo, bomb or rocket hits, which are unlimited anyway |
| LOMP06 1200/1000 | `events=0` | **exit 1**, and `throttled=0` |

#### Rate-limit pairs and verdict

- OFF is this tree's build of `c88401ac4`.
- ON is `pair_export --flip kLuaHitRateLimitBound=true` (`local/rl_on`).
- The logs are `local/rl_{off,on}_<mission>.log`.

| row | ON census | pair_diff | verdict |
| --- | --- | --- | --- |
| JM06 3200/3000 | `passed=73 throttled=247 pairs=18` of 320 hits | exit 1: only the summary line moves | held |
| USN01 3200/3000 | `passed=74 throttled=464 pairs=20` of 538 hits | exit 1: only the summary line | held |
| LOMP06 1200/1000 | `passed=0 throttled=0` | exit 1: the summary line and the known frame-441 camera noise | held |

Three quarters of the hits on these rows repeat a (victim, attacker) pair within 2 s. Once a gun
`hit` listener exists on a row, it fires about a quarter as often as the host would have fired it
before.

**Verdict: `kLuaHitRateLimitBound = true`.**

## The unimplemented Lua natives, third refresh (packet `cc9_lua_natives_ranking_3`, head `3287e40f1`)

Worker cc9-lua4, 2026-09-28. The census reads every `MissionLuaNative::* UNIMPLEMENTED` row of the
final host tables of these runs. All come from this tree's build at `3287e40f1`, with streams ON,
`BSP_DEATH_TABLE=1`, lockstep 0.05, an idle player and present interval immediate.

| mission | frames | log | unimplemented |
| --- | --- | --- | --- |
| USN02 | 9200/9000 | `local/l4_rk_usn02.log` | 510 |
| JM06 | 3200/3000 | `local/l4_rk_jm06.log` | 527 |
| LOMP06 | 1200/1000 | `local/l4_rk_lomp06.log` | 489 |
| USN13 | 3200/3000 | `local/l4_rk_usn13.log` | 506 |
| JM08 | 3200/3000 | `local/l4_rk_jm08.log` | 495 |

The unimplemented totals equal reference h's on all five rows. USN02 still fails at 29.75 s in
phase 1, so its rows are the failure path's. SetShipSpeed, UnitGetAttackTarget, SquadronSetSpeed,
IsClassChanged, SetSubmarineDepthLevel and SetAirBaseSlotCount are bound since the second ranking.
The scripts are this installation's, which is modded: `global/commandhelpers.lua` is dated
2024-10-29, `jm06.lua` and `06_crucial_cargo.lua` 2024-07-13, `prcpjm08.lua` 2024-08-26.

| rank | native | address | missions (calls) | reach |
| --- | --- | --- | --- | --- |
| 1 | `SetDeviceReloadEnabled` | `008C1350` | JM06 (1), JM08 (1) | **gameplay**: it stores its boolean in `00E17BF2`. Every reader of that byte pairs it with the squadron's `+369h`, which is the scene property `ReloadEnabled` (`007F1FE0`) over a constructor default of 1 (`007F2D09`). No scene in this installation carries `ReloadEnabled` (`FireStance` is found, as the control), so every squadron holds 1 and the pair is decided by this native. The readers are the loadout index (`007EEC00`), the break-off tests of the depth-charge, kamikaze and level-bomb tasks, the dive-bomb approach and go-away, the torpedo arm, and `009FFEB0`. The host feeds both bytes as constant false |
| 2 | `GetClosestBorderZone` | `008AECD0` | USN02 (1) | gameplay after the failure: `commandhelpers.lua` 9577 orders the failed ship to the nearest border zone with `NavigatorMoveToRange`; nil leaves it without that order |
| 3 | `IsInFormation` | `008996A0` | LOMP06 (1) | an order: `06_crucial_cargo.lua` 537, 554 and 566 call `LeaveFormation` or pick `JoinFormation`'s target on its answer. Neutral false skips `LeaveFormation` before `NavigatorAttackMove` |
| 4 | `SquadronSetTravelAlt` | `0089F550` | JM08 (1) | the invincible movie plane at 750 m (`prcpjm08.lua` 574). The attack waves' calls (1075, 1169, 1267) are not reached by 3000 frames |
| 5 | `CountdownCancel`, `Scoring_SetMissionCompleted`, `BannSupportmanager` | | USN02 (1 each) | the failure path's timer and scoring |
| 6 | `LoadCheckpoint` | `008ACA30` | JM06 (1) | nil means no saved checkpoint (`commandhelpers.lua` 16949), which a fresh run has anyway |
| - | `IsGUIActive` (114), `DisplayScores` (55), `SetGuiName` (34), `PrepareClass` (27), `MissionNarrativeClear` (22), `SetNumbering` (12), `EnableInput`, `BlackBars`, `MissionNarrative`, hints, `Loading_*`, `DisplayUnitHP` | | 1..4 each | presentation. `IsGUIActive`'s one live use (`commandhelpers.lua` 13270) picks the music |

Outside the natives, `Entity::on_killed_lua_self_00928c80` (up to 10 calls) and
`SceneContents::launch_class_from_lua` (up to 118) are host structure rows, not script natives.

**Next packet: `SetDeviceReloadEnabled` (`008C1350`).** Its Lua side is this lane's. The feeds of
`00E17BF2` and `+369h` sit in `src/game_hosts_units.cpp` (the dive-bomb, torpedo and go-away
inputs) and `src/game_hosts_ai.cpp` (`009FFEB0`), so the binding exposes one host value and routes
the feed lines. Readers without a host model (`007B58D0`, `007B6240`, `007EEC00`, `0084E010`, and
the break-off tests at `009A6600`, `009AE1D0`, `009B7AB0`, `009B7C90`, `009B8DB0`) stay
unmodelled.

## SetDeviceReloadEnabled, 008C1350 (packet `cc9_device_reload_enabled`, `kLuaDeviceReloadEnabledBound`, committed OFF)

Worker cc9-lua4, 2026-09-28.

### The image (V)

- **The native.** `008C1350` reads argument 0 through `00B66250` (`lua_toboolean`, `008C144F`)
  and stores it in the byte `00E17BF2` (`008C1458`). It takes no entity, makes no class test and
  returns no value.
- **The other writers.** The mission load's lobby sync writes the byte at `005E2FB2` (0) and at
  `005E3017` (the settings' `sete`), which the host models as
  `LobbySettingsModeFlags::reload_payload_on`. `BSP_Session_SetMode` writes 0 at `0076FE6C`.
- **The partner byte.** Every reader tests `00E17BF2` together with the squadron's `+369h`, except
  `009FFEB0`, which tests `00E17BF2` alone. `007F1FE0` fills `+369h` from the scene property
  `ReloadEnabled` when it is a boolean (`type 3`), over the constructor's 1 (`007F2D09`).
  `007F3500` and `007F1FE0`'s second arm copy it from a source object's `+124h`.
- **Its absence here.** No `.scn` under this installation's `universe/Scenes/missions` contains
  the string `ReloadEnabled`, while `FireStance`, read by the same function, is found. So every
  squadron built from these scenes holds 1.
- **The readers.** The dive-bomb approach (`009C7C08`, `009C7C50`), go-away (`009C7F60`,
  `009C4A1F`) and entry (`009C8361`), the torpedo arm (`009D315C`), `009FFEB0`, and readers with no
  host model: `007B58D0`, `007B6240`, the loadout index `007EEC00`, `0084E010`, and the break-off
  tests `009A53CA`, `009A662D`, `009AD49E`, `009AE1DF`, `009B7ABF`, `009B7C90`, `009B8DBD`.

### The script reach

88 uncommented lines in this installation's scripts call `SetDeviceReloadEnabled(true)`, and 8 call it with
false. None of the reference rows' scripts does (`usn_19_coralus.lua` for USN04,
`usn_1_marshall.lua`, `usn_2_java.lua`, `usn_13_truk.lua`, `bsm_01_stationed_at_pearl.lua`,
`06_crucial_cargo.lua`). The smoke rows JM06 (`jm06.lua` 333) and JM08 (`prcpjm08.lua` 104) call
it once, with true.

### The binding

- **Lua side (this commit).** The row routes to `run_set_device_reload_enabled_008c1350`, which
  stores the flag in one process-wide value. The lobby flags' publish resets that value from
  `reload_payload_on`. `lua_device_reload_enabled_00e17bf2()` reports it, and reports false while
  the switch is OFF. The summary line is `summary mission script device reload`.
- **The feeds (routed, not in this commit).** The plane-task inputs in `src/game_hosts_units.cpp`
  (`2605`, `2902`, `3110`, `10285`, `10360`, `10466`, `13651`) and `009FFEB0` in
  `src/game_hosts_ai.cpp` still feed constant false. Each needs `global_e17bf2` from the accessor
  and `control_flag_369` as `kLuaDeviceReloadEnabledBound`, the image's default of 1.
- **SUBSTITUTION (labelled).** Every squadron's `+369h` is taken as 1. The clone paths that copy
  `source+124h` are not modelled.

### Predictions, before any run

- **USN02, USN13, LOMP06: exit 0.** No script calls the native, and the lobby publish keeps the
  byte at 0.
- **JM06 and JM08: exit 1, gameplay identical.** The byte goes to 1 at the call, but neither run
  orders a plane attack: `pilot attack` reports no attack order on both, and JM06 builds one
  squadron (a PBY). The diff should be the native's note, summary and host-table lines.
- **With the feeds routed**, the same verdicts are predicted on these rows. A mission whose idle
  AI flies dive-bomb or torpedo attacks after the call would move: the entry's break-off at
  `009C8361` holds instead of finishing an empty bomber, and the go-away turn scales by 1.5.

### SetDeviceReloadEnabled: the Lua-side pairs (feeds not yet routed)

OFF is this tree's build at `ed959c372`. ON is `local\dr_on`, a `pair_export` of `ed959c372` with
`kLuaDeviceReloadEnabledBound=true` (SHA-256 prefix `167D28835E80`). Logs are
`local\dr_{off,on}_<mission>.log`.

| mission | pair_diff | reading |
| --- | --- | --- |
| JM06 3200/3000 | exit 1, gameplay identical | the native fires once with true; the byte reads 1 at the end |
| JM08 3200/3000 | exit 1, gameplay identical | the same |
| USN13 3200/3000 | exit 1, gameplay identical | no call; the byte stays 0 |

- **The USN13 prediction missed on its exit code, not its mechanism.** I predicted exit 0, but the
  new summary line prints `bound=`, so every flipped run differs in text. Gameplay is identical,
  as predicted.
- **These pairs do not test the feeds.** The plane-task inputs still read constant false, so the
  switch stays OFF until the routed feed lines land and the pairs are re-run.

## IsInFormation 008996A0 and LeaveFormation 00899EB0 (packet `cc9_lua_formation_query`, `kLuaFormationQueryBound`, committed OFF)

Worker cc9-lua4, 2026-09-28. Taken ahead of `GetClosestBorderZone` (rank 2), whose answer needs
`004C7730`. That body is unreconstructed, and the host holds no border-zone data
(`docs/PILOT_ORDER_BINDINGS.md`, `PilotRetreat`). It is left as its own packet.

### The image (V)

- **IsInFormation.** `008996A0` resolves argument 0 through `BSP_ObjectHandle_FromLuaTable` and
  pushes `[unit+284h] != 0`, the unit's formation group. One result.
- **LeaveFormation.** `00899EB0` resolves argument 0 the same way and calls `0077C980(unit, 0)`.
  With `unit+284h` set and a null second argument, `0077C980` bumps
  `BSP_SlotCounter_Increment([unit+1ACh])` for a slot below 8 and routes session message 77h with
  target 0 (`BSP_Session_RouteMessage(msg, 7, 0)`, vtable `00D02D44`).
- **The delivery.** `0077FE80` maps kind 77h to arm 3, which calls `0077BD70(unit, null)`, the
  leave (`docs/SHIP_AI_FORMATION.md`, "The read: the image removes a dead unit at its destroy").
  The host models that leave as `GameUnitsHost::leave_group_on_destroy_0077bd70`.

### The binding

- `run_is_in_formation_008996a0` answers `unit_formation_group_0284(index) >= 0`.
- `run_leave_formation_00899eb0` calls `leave_group_on_destroy_0077bd70(index)` when the unit is
  in a group.
- **SUBSTITUTIONS (labelled).**
  - The leave runs at the call, not through the session route.
  - The slot counter bump is not modelled.
  - The host's leave counts the unit in its death-leave counter, and its note names the destroy
    path.
  - An entity with no units-host slot answers false and leaves nothing.

### The script reach

`06_crucial_cargo.lua` (this installation, 2024-07-13) has the calls at 537, 554, 555, 566 and
567, inside `SubC1Attack`. `jm06.lua` has four, which the 3000-frame JM06 run does not reach. No
reference row's script calls either native. In the LOMP06 census the one call is
`IsInFormation(Yugiri)` at frame 617 (30.85 s), from line 554 or 566. Yugiri is an escort in
group 0 (leader Mikuma), per the host's formation table.

### Predictions, before any run

- **LOMP06 1200/1000: exit 3.**
  - `IsInFormation(Yugiri)` answers true.
  - The script then calls `LeaveFormation(Yugiri)`, so the native table gains one LeaveFormation
    call.
  - Yugiri leaves group 0 at 30.85 s, and the group's member list compacts past it. Mikuma stays
    leader.
  - `SetShipSpeed(Yugiri, 20)` and the attack move follow as before.
  - Yugiri's and the later group-0 followers' tracks should move after 30.85 s. The mission's
    idle damage is 0, so the death and hit tables should stay empty.
- **USN02, USN13, JM06, JM08: exit 1, gameplay identical.** No call is reached. The text differs
  only in the new summary line's `bound=`.

### IsInFormation and LeaveFormation: the pairs and the verdict

OFF is this tree's build at `28bcf320d`. ON is `local\fq_on`, a `pair_export` of `28bcf320d` with
`kLuaFormationQueryBound=true`. Logs are `local\fq_{off,on}_<mission>.log`.

| mission | pair_diff | reading |
| --- | --- | --- |
| LOMP06 1200/1000 | exit 1, gameplay identical | the mechanism fires as predicted; the tracks do not move |
| JM06 3200/3000 | exit 1, gameplay identical | no call |
| USN13 3200/3000 | exit 1, gameplay identical | no call |

- **The mechanism matched.** `IsInFormation(Yugiri)` answered true (group 0). The script then
  called `LeaveFormation(Yugiri)`, and Yugiri left group 0 at 30.90 s. Mikuma stayed leader with
  17 members. The native table gained the LeaveFormation row.
- **The spread prediction failed.** I predicted exit 3, with Yugiri's and the later followers'
  tracks moving. Every `ship ai step` line and all 22 unit-table rows are identical.
  - Yugiri was already in the attack move on both sides, which does not steer by the group.
  - The remaining followers keep their stations after the compaction.
  - What moved is bookkeeping: one more ship-AI plan seed and path swap, and two fewer
    motion-tail pairs. The frame-441 movie camera pose moved by 0.1 m, which is the known
    run-to-run noise (handoff cc9-lua3, item 5).
- **Verdict: `kLuaFormationQueryBound = true`.** The mechanism matches the image, and the miss is
  on spread only, which the flip rule allows when recorded.

## SquadronSetTravelAlt, 0089F550 (a read; the binding needs plane-side state)

Worker cc9-lua4, 2026-09-28. This is rank 4 of the third refresh.

### The image (V)

- **The native.** `0089F550` resolves argument 0 through `BSP_ObjectHandle_FromLuaTable`, which
  gives the squadron. It reads argument 1 as a number. With exactly three arguments, it reads
  argument 2 as a boolean, else 0. It then stores five fields:
  - `+380h = 0.5` (`[00CE3800]`, bytes `00 00 00 3F`);
  - `+38Dh =` the boolean;
  - `+394h =` the altitude, the squadron's cruising altitude;
  - `+3A9h = 1`;
  - `+3ADh = 0`.
  It returns no value.
- **The consumer.** The cruise profile `009C3650` returns early when `+38Dh` is set. It overwrites
  `+394h` with its own altitude, and sets `+3ADh = 1` and `+3A9h = 0`, only when `+380h` is below
  0 and `+3A9h` is clear. The script sets `+3A9h = 1`, so the profile keeps the script's
  altitude either way; the forced call (`true`) also skips the profile's other arms. Who clears
  `+3A9h` or counts `+380h` down, apart from that overwrite, is unread.

### Why it is not bound here

The host keeps no per-squadron `+394h`. The moveto task computes its cruise altitude on each
refresh from Small/LargePlaneTravelAlt plus 0.6 times TravelAltRandom (`src/game_hosts_units.cpp`,
`moveto_refresh_009beba0`), and `009C3650` itself is not modelled. A binding needs these fields on
the units host's squadron state and the profile's gate in the moveto refresh. That is plane-side
work outside this lane's files.

### Reach

On the census rows only JM08 calls it, once: `SquadronSetTravelAlt(Mission.MovPlane, 750, true)`
(`prcpjm08.lua` 574, this installation, 2024-08-26). The movie plane is made invincible on the next
line. The attack waves' calls (1075, 1169, 1267) are not reached by 3000 frames.

### `attackerPlayerIndex`: the fire-time producer (packet `cc9_attacker_player_index`, a read)

Worker cc9-lua4, 2026-09-28. This closes item 2 of the cc9-lua3 handoff as a read. The binding
needs gunnery-side lines, which are routed below.

**The record (V).**
- `00988510` takes `src` from `[hit+4]->vtable[108h]()`. On the projectile vtable `00CF9DF0`, slot
  `108h` (`00CF9EF8`) is `006E7C40`, which returns `ECX + 170h`, or 0 for a null projectile.
- `006E7B00` constructs the `ProjectileShotBase` at `+170h` (`006E7B24` `LEA EDI,[ESI+170h]`,
  then `006E22D0`). So `src` is the shot sub-object, and `[src+1Ch]` is projectile `+18Ch`, which
  `include/bsp/projectile_impact.hpp` already names `kProjectileOffTeamId`.
- `006E22D0` initialises the field to -1 (`param_1[7] = 0xFFFFFFFF`).

**The producers (V).**
- **Guns: `0072BF10` BSP_Gun_CreateProjectile.** It creates the shot through the class
  descriptor's `vtable[20h]` at `0072C006` (`006E8430` for a shell). The torpedo class's create is
  `00856420` (`docs/PROJECTILE_KINDS.md`); whether torpedo tubes fire through this function was
  not traced.
  - `0072C0F2` `EAX = gun[+1ACh]`, the gun's own seat. `0072C0FB` stores it at `shot+1Ch`.
  - The fallback applies when the seat is 8 (PLAYER_AI), `gun->vtable[5Ch](21h)` holds (the
    `MRFSGun` family, `docs/ENTITY_CLASS_IDS.md`), and the owner `gun[+3F0h]` is non-null. Then
    `EBX` is the owner, and when the owner is a plane (`vtable[5Ch](0Fh)`) with `+914h > 0.0`,
    `EBX = [[owner+9D4h]+3D0h]`, the squadron's member-array head. `0072C14B` stores
    `[EBX+1B0h]`, the unit's role-1 slot, at `shot+1Ch`.
- **Bombs: `006E4D50` BSP_MBombPlatform_Drop.** `006E5521`-`006E5527` store
  `[platform[+3F0h]+1B0h]`, the owning plane's role-1 slot, at `shot+1Ch`.
- Depth charges were not traced.

**The values.** This installation's scripts define `PLAYER_1 = 0`, `PLAYER_AI = 8` and
`PLAYER_ANY = 9`. An AI gun gives 8, or the owner's role-1 slot for an `MRFSGun`. A gun seated by
message 79h gives its holder's slot (`docs/PLAYER_GUN_SEAT.md`).

**What the host lacks.**
- `projectile_spawn_0072bf10` (`src/projectile_impact.cpp`) already carries `team_id` and
  `shot_set_team`, but no host calls it.
- The gunnery host builds its own `GameProjectileRow` at `src/game_hosts_gunnery.cpp` 5927-5930
  and carries no team id to `apply_hit`. `GameGunneryHitEvent::attacker_player_index` stays -1
  (7370).

**Routed to the gunnery lane** (`include/bsp/game_hosts_gunnery.hpp`,
`src/game_hosts_gunnery.cpp`):
1. `GameProjectileRow`: `int team_id_1c{-1};  // shot+1Ch = projectile+18Ch (0072C0FB, 0072C14B)`.
2. At the shot's creation (after `shot.owner_side`, 5930): `shot.team_id_1c = gun.seat_1ac;`.
   When that is 8 and the gun is an `MRFSGun`, take the owner's role-1 slot through
   `units.unit_current_role_slot(owner_unit, 1, out)`. For a plane owner with `+914h > 0`, take
   the squadron head's slot instead.
3. `apply_hit` takes the shot's `team_id_1c` (the call at 6683), and 7370 sets
   `ev.attacker_player_index` from it.
4. The bomb drop's shot takes the plane's role-1 slot the same way.

**The Lua half, when the field lands.** Under a new switch, the `hit` entry keeps the
`attackerPlayerIndex` set instead of counting it unmodelled, and dispatch tests the event's index
against it. Set membership is assumed, as for the `recon` party set; the 009725B0 test itself is
unread. The only live user is JM06's `hshit` (`{PLAYER_1}` on the hospital ship), which the idle
runs never hit.

### `attackerPlayerIndex`, bound (packet `cc9_hit_attacker_player_index`, `kLuaHitAttackerPlayerIndexBound`, committed OFF)

Worker cc9-lua5, 2026-09-28. Item 1 of the cc9-lua4 handoff, after gunnery5 landed the field
(main `e49be76ba`).

**The binding.**
- The `hit` entry keeps its `attackerPlayerIndex` set (`+4Ch`, loaded through `009722D0`, a plain
  list of integers filled by `00971250`) instead of counting it unmodelled.
- Dispatch tests `GameGunneryHitEvent::attacker_player_index` for membership, after the
  targetDevice, fireCaused and leakCaused tests. An empty set holds any index.
- The host's stamp is the gunnery host's `shot_team_id_0072c0f2`: the gun's seat `+1ACh`, or, for
  a seat of 8 on a plane gun, the owner's role-1 slot. A bomb takes the plane's role-1 slot.
  Torpedoes and depth charges carry -1.
- **Assumed, not read:** set membership. The comparison behind `009725B0`'s entry is not traced,
  as for the `recon` party set, which the same `009722D0` loads.
- **Not part of the channel:** the `uVar14 < 8` and `piVar8[0x6B] < 8` gate in `00988510` guards
  a network warning message (`BSP_Session_SendMessageToNonlocalPeer`), not the Lua dispatch.

**The users (census of this installation's scripts).** The earlier "only `hshit`" count covered
the entered rows only. Across `scripts/missions` there are 84 non-empty sets, all on the `hit`
channel.
- `{PLAYER_1}` is the common form: the IJN campaign (JM01-JM04, JM06, JM09, JM13), USN02 Cape
  Esperance and USN03 Santa Cruz (both directories), ESMP 02 and 03, LOMP03 and LOMP05.
- `{PLAYER_AI}`: JM03 `luaJM3CargosHit`, JM05 `luaJM5HangarHit`, JM08 `luaJM8AirfieldHitByAI` and
  `luaJM8ShipyardHitByAI` (attacker `Mission.AirPatrol`), LOMP05.
- `{PLAYER_AI, PLAYER_1}`: JM03 `luaJM3DeRuyterHit` and `luaJM3ExeterHit`.

**Why the measured rows cannot move.** The count `unmodelled` rises only when an entry with a set
passes every earlier filter. It is 0 on all nine rows of reference h (`rb8_*.log`, worktree
cc9-gunnery4). So on those rows no hit reaches the new test, and ON dispatches nothing new.

**Predictions** (written before the ON runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player, present interval immediate):

| row | prediction |
| --- | --- |
| JM06 3200/3000 | **exit 0.** `hshit` targets the hospital ship, which no idle run hits (`unmodelled=0`) |
| USN01 3200/3000 | **exit 0.** No `attackerPlayerIndex` entry is registered |
| JM08 3200/3000 | **exit 0.** The AirPatrol does not hit the Airfield or the shipyards in 150 s (`unmodelled=0`) |

**Scenarios that would exercise it.**
- JM06 with the player's own gun seat firing on the hospital ship: the seat gives 0, so
  `luaJM6HospitalShipHitByPlayer` fires ON and never OFF.
- JM08 run long enough for the AirPatrol to bomb the Airfield: `luaJM8AirfieldHitByAI` fires ON
  only if the host's role-1 slot for an AI plane is 8. That slot is the check to make first.

#### attackerPlayerIndex pairs and verdict

- OFF is this tree's build of `196fa0ec4`.
- ON is `pair_export --commit 196fa0ec4 --flip kLuaHitAttackerPlayerIndexBound=true` (`local/ap_on`).
- The logs are `local/ap_{off,on}_<mission>.log` in worktree cc9-lua5.

| row | pair_diff | hit listeners (both sides) | verdict |
| --- | --- | --- | --- |
| JM06 3200/3000 | exit 0, gameplay identical | events=320 fires=0 unmodelled=0 | held |
| USN01 3200/3000 | exit 0, gameplay identical | events=538 fires=0 unmodelled=0 | held |
| JM08 3200/3000 | exit 0, gameplay identical | events=342 fires=0 unmodelled=0 | held |

**Verdict: `kLuaHitAttackerPlayerIndexBound = true`.** It is inert on these rows, as predicted. No
`hit` filter key is left unmodelled now; the rate limit's `this+1A4h` list and the forced kind 11h
stay as recorded below.

### The rate limit's unmodelled bits, read and closed (cc9-lua4, 2026-09-28)

Item 3 of the cc9-lua3 handoff. Both stay unmodelled, because neither has an input in this
installation's runs.

**The `this+1A4h` shortcut (V).**
- `00988510`'s `this` is the WarningManager: its constructor `0098A020` builds the member at
  `+1A4h` (`0098A248`). `00988A0E`-`00988A56` test it after storing the 2 s or 1e-4 s stamp.
- When the set is non-empty (`+1ACh`), `005A16F0` looks up the victim key built at `0098859B`
  (`param_2->vtable[140h]()`, stored at `[base+34h]`). A hit on a listed victim overwrites the
  stamp with `clock + 1e-4` (`00D7A268`, the double `0x3F1A36E2E0000000`), so its `hit` callbacks
  are never throttled.
- **The only writer is the Lua native `HackSensitiveUnit` (`008CBEB0`).** It reads argument 0
  through `BSP_LuaTable_GetPtrField` and inserts it with `008CAEE0` on
  `[00F8A0C4] + 1A4h` (`008CBFC6`-`008CBFE0`). `008CB570` is a second inserter on the same member,
  which Ghidra shows with no callers.
- **No script in this installation calls `HackSensitiveUnit`** (a search of every `.lua` under
  `scripts`, excluding the `.bak` copies). The set stays empty, and the shortcut never fires.

**The forced kind 11h (V).**
- `009885EC`-`00988663` read the hit's source `[hit+4]`, not the shot record. The kind becomes 11h
  (KAMIKAZE) and the attacker becomes the source itself in two cases:
  - the source answers `vtable[5Ch](6)`, the ship base, and its `[+538h]` object has `+510h` or
    `+514h` above 0.0;
  - the source answers `vtable[5Ch](17h)`, `MPlaneKamikaze` (`docs/ENTITY_CLASS_IDS.md`).
- Both are hits whose source is a unit, not a projectile. The host's hit events come only from
  shells and their blasts (`apply_hit`, `src/game_hosts_gunnery.cpp` 6683 and 7668), so no host
  event has such a source.
- The forced kind matters only once a unit-source hit exists, such as a kamikaze crash or that
  ship arm. Which ship state `+538h`'s `+510h` and `+514h` hold was not read.

**Verdict:** no binding. Both are recorded as unreachable on this installation's rows.

### SetDeviceReloadEnabled: the feeds (packet `cc9_device_reload_feeds`)

Worker cc9-lua4, 2026-09-28. The lead gave this lane `src/game_hosts_units.cpp` for the packet.

**The change.** The seven plane-task feeds now take `control_flag_369 = kLuaDeviceReloadEnabledBound`
(the squadron's `ReloadEnabled`, default 1) and `global_e17bf2 = lua_device_reload_enabled_00e17bf2()`.
The sites are the dive-bomb approach latch, the dive-bomb entry, the go-away completion
(`009C7F00`), the torpedo arm entry, the torpedo class-extra refusal, the torpedo go-away, and the
go-away turn entry (`009C4950`). With the switch OFF, both read false as before.

**The one site left as a stand-in.** `009FFEB0` in `src/game_hosts_ai.cpp` 1616 belongs to another
lease. With the byte set, the image returns false there without reading the carrier arm. The host
already answers false on that arm, so the stand-in gives the same answer. The replacement line, for
routing, is:

```
        if (lua_device_reload_enabled_00e17bf2()) return false;   // 009FFEB0, [00E17BF2] set
```

**What changes once both bytes are set** (from the readers' docs):
- the dive-bomb entry (`009C8361`, `docs/DIVE_BOMB_TASK.md` row 2): a bomber with no bomb left
  (`task+4C9h == 0`) no longer finishes the task at once;
- the dive-bomb approach threshold (`docs/DIVE_BOMB_TASK.md` 169) drops the `approach+B8h + 100`
  arm;
- the go-away turn scales by 1.5 (`009C4A1F`), and the go-away completion by 0.9 (`009C7F00`);
- the torpedo arm's latch and its `* 0.4` arm (`009D315C`, `docs/TORPEDO_AFTER_THE_DROP.md`).

**Predictions, before the runs** (ON = the switch flipped in a `pair_export`):
- **JM06 3200/3000: exit 1, gameplay identical.** The native sets the byte at `jm06.lua` 333. The
  run builds one squadron, a PBY Catalina, and orders no attack (`pilot attack` reports no order),
  so no fed task runs.
- **JM08 3200/3000: exit 1, gameplay identical.** The byte is set at `prcpjm08.lua` 104. Nine
  squadrons are built, but no dive-bomb or torpedo task is ordered in 3000 frames.
- **USN13 3200/3000: exit 1, gameplay identical.** No call is made, so the byte stays 0, and the
  conjunction stays false even with `+369h` at 1.
- **What would move.** A row whose script calls the native and whose idle AI then flies dive-bomb or
  torpedo attacks. None of the measured rows does: the reference rows never call it, and JM06/JM08
  order no attack in their windows.

### SetDeviceReloadEnabled: the feed pairs and the verdict

OFF is this tree's build at `9859b3ea7`. ON is `local\rf_on`, a `pair_export` of `9859b3ea7` with
`kLuaDeviceReloadEnabledBound=true`. Logs are `local\rf_{off,on}_<mission>.log`.

| mission | pair_diff | reading |
| --- | --- | --- |
| JM06 3200/3000 | exit 1, gameplay identical | the native fires once with true; only its host row and summary line differ |
| JM08 3200/3000 | exit 1, gameplay identical | the same |
| USN13 3200/3000 | exit 1, gameplay identical | no call; only the `bound=` field of the summary differs |

All three match the predictions. No fed plane task runs after the call on these rows.

**Verdict: `kLuaDeviceReloadEnabledBound = true`.** The `009FFEB0` line in `src/game_hosts_ai.cpp`
stays a labelled stand-in until it is routed. It gives the same answer, false, either way.

### SquadronSetTravelAlt, bound (packet `cc9_squadron_travel_alt`, `kSquadronTravelAltBound`, committed OFF)

Worker cc9-lua4, 2026-09-28. The lead gave this lane `src/game_hosts_units.cpp` for it.

**The image, term by term (V, listing).**
- **The block.** `007F2BD0` (`BSP_PlaneSquadron_InitTimerBlock`, from the constructor at
  `007F2D2B`) makes `squadron+37Ch..+388h` four countdowns, all `[00D7A260]` = -1.0, and
  `+38Ch..+38Fh` their freeze bytes, all 0. `+3A9h` starts at 0.
- **The tick.** `007F3BA0` subtracts the step from each countdown whose freeze byte is clear
  (`squadron_tick_advance_sim_007f3ba0`).
- **The native.** `0089F550` stores `+380h = 0.5` (countdown 1), `+38Dh = force` (its freeze
  byte), `+394h = altitude`, `+3A9h = 1` and `+3ADh = 0`. The scan for `+380h` stores finds only
  this native and the tick, plus the ship AI's own `+380h` at `009F3FDB`/`009F47A7`, which is a
  different class.
- **The profile `009C3650`** is a task's `vtable[54h]` (`00D20BBC`, `00D20C34`). Every ordnance
  arm and both fighter arms end the same way on the block `[task+404h]+37Ch`:
  - `CMP [+38Dh],0 / JNZ` returns at once;
  - `COMISS 0,[+380h] / JBE` skips the write unless `+380h < 0`;
  - `CMP [+3A9h],0 / JNZ` skips it while the lock is set;
  - otherwise it writes `+394h` (for example `009C3757` `FSTP [ESI+18h]`) and sets `+3ADh = 1`;
  - every skip lands on `009C3943`, which clears `+3A9h`.
- **So:**
  - with `force`, the script's altitude holds for good;
  - without it, the altitude holds until the countdown passes zero, 0.5 s after the call, and
    the profile has run once to clear the lock; the next profile call replaces it.

**The binding.**
- The Lua row routes to `run_squadron_set_travel_alt_0089f550`, which calls
  `GameUnitsHost::set_squadron_travel_alt_0089f550`.
- That stores the block on the squadron's slot: the registry squadron unit, which is the fused
  leader. The countdown is kept as the clock at which it goes below zero.
- The moveto refresh (`moveto_refresh_009beba0`) applies the profile's gate to the value it
  already computes, SmallPlaneTravelAlt / LargePlaneTravelAlt plus 0.6 times TravelAltRandom.
- **SUBSTITUTIONS (labelled).**
  - The host's 0.5 s moveto refresh stands for the profile's call. When `009C3650` itself runs
    was not traced; its callers are task vtables.
  - The other `+394h` readers (the torpedo go-away's `squadron_cruising_alt_394`) keep their
    tuning value.
- The census is `summary mission squadron travel alt`, with one line per squadron the native
  touched.

**Predictions, before any run.**
- **JM08 3200/3000: exit 3.**
  - `prcpjm08.lua` 574 calls `SquadronSetTravelAlt(Mission.MovPlane, 750, true)` in the stage init
    on "Movie Mavis", a large recon flying boat. Its moveto to MoviePoint currently steers at
    LargePlaneTravelAlt 1400 + 0.6 * 50 = 1430 m (`planeglobals.lua` 461-463).
  - With the binding and `force` it steers at 750 m (less its +2Ch offset) for the whole run.
    `last_cruise` should read 750.0, and its altitude `y` should end well below the OFF run's.
  - The Mavis is invincible. What else moves depends on who can reach it lower down: more AA
    shots at it, and through the shared stream, other draws. The deaths may shift. They should
    not change in kind.
- **USN04 4700/4500 and USN13 3200/3000: exit 1, gameplay identical.** Neither run reaches the
  native (USN04 has one call, `usn_19_coralus.lua` 2182, not reached in 4500 frames); only the
  summary's `bound=` differs.

### SquadronSetTravelAlt: the pairs and the verdict

OFF is this tree's build at `0c48b1e44`. ON is `local\ta_on`, a `pair_export` of `0c48b1e44` with
`kSquadronTravelAltBound=true`. Logs are `local\ta_{off,on}_<mission>.log`.

| mission | pair_diff | what moved |
| --- | --- | --- |
| JM08 3200/3000 | exit 3 | shots 1689 -> 2091, hit records 310 -> 342, hull hits 89 -> 86, damage 3745.4 -> 3681.8; the same 9 death rows, 8 changed |
| USN13 3200/3000 | exit 1 | nothing (no call) |
| USN04 4700/4500 | exit 1 | nothing (no call in the window) |

- **The mechanism held.**
  - JM08's `squadron travel alt Movie Mavis: alt=750.0 force=1 active=1 last_cruise=750.0`, with
    121 gated refreshes.
  - The script kills the Mavis at 40.05 s in both runs (`KillReason=harm`, no first damage). It
    dies at 759 m instead of 1391 m.
- **What followed.**
  - Lower down, AA reaches the Mavis: its death row now names "Stephen Potter" at 1313 m, where
    OFF had no killer.
  - The extra AA fire moves the shared stream and the later raid. H6K Mavis 01, the three Gekkos
    and the three Oscars die 1 to 3 s earlier, with other killers.
  - The victims are the same nine, as predicted.

**Verdict: `kSquadronTravelAltBound = true`.**

## The unimplemented Lua natives, fourth refresh (cc9-lua5, head `57acc6f7f`)

Worker cc9-lua5, 2026-09-28. Same method as the third refresh, same run parameters.

| mission | frames | log (worktree cc9-lua5) |
| --- | --- | --- |
| USN02 | 9200/9000 | `local/l5_rk_usn02.log` |
| JM06 | 3200/3000 | `local/ap_on_jm06.log` (the `583e3eee1` code) |
| LOMP06 | 1200/1000 | `local/l5_rk_lomp06.log` |
| USN13 | 3200/3000 | `local/l5_rk_usn13.log` |
| JM08 | 3200/3000 | `local/ap_on_jm08.log` (the `583e3eee1` code) |

Since the third refresh, `SetDeviceReloadEnabled`, `IsInFormation`, `LeaveFormation` and
`SquadronSetTravelAlt` are bound. What is left with any gameplay reach:

| rank | native | address | missions (calls) | reach |
| --- | --- | --- | --- | --- |
| 1 | `GetClosestBorderZone` | `008AECD0` | USN02 (1) | after the failure at 29.75 s, `commandhelpers.lua` 9577 (mtime 2024-10-29) sends the failed ship Alden to the answer with `NavigatorMoveToRange` |
| 2 | `CountdownCancel`, `Scoring_SetMissionCompleted`, `BannSupportmanager` | | USN02 (1 each) | the failure path's timer and scoring |
| 3 | `LoadCheckpoint` | `008ACA30` | JM06 (1) | nil means no saved checkpoint, which a fresh run has anyway |
| - | the presentation natives of the third refresh | | | unchanged |

LOMP06, USN13 and JM08 call no unimplemented native with gameplay reach.

## `GetClosestBorderZone`, bound (packet `cc9_get_closest_border_zone`, `kLuaClosestBorderZoneBound`, committed OFF)

Worker cc9-lua5, 2026-09-28. The image read and the zone records are in
`docs/WORLD_MAP_BOUNDS.md`, "Border zones".

**The binding.**
- `008AECD0` reads argument 0 as a vector. The offset is 500.0 (`00CE397C`) unless there are
  exactly two arguments and the second is a number.
- It calls `004C7730` for any side (`EDI = -1`, `008AED36`) and answers a table with `x`, `y` and
  `z` (`0088BA30`).
- The answer is the nearest zone edge point, moved `offset` further:
  - along (edge point - position), when the offset is positive, the position is inside the map
    (`0071C4F0`) and the edge point is more than 1 away;
  - otherwise along the zone's outward direction.
- The mission frame hands the Lua host the map bounds at the avoid-zone load step, from the same
  `Map` block and `004D5EDE` selection as the ship AI. The host builds the twelve records from them.
- **Labelled substitutions:**
  - the records are built at the avoid-zone load, not inside `004E6C00`;
  - argument 0 goes through the host's `00888760` reader;
  - with no records the image's outputs are unwritten stack; the host answers the position itself
    and counts it `missing`.

**What OFF does on USN02.** The neutral answer still reaches `NavigatorMoveToRange`. Alden enters
`movetopos` at 29.75 s toward a point 6268 m away (step 600, heading target -0.1883). It is back in
`attackmove` by step 890.

**Predictions** (written before the ON runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player, present interval immediate):

| row | prediction |
| --- | --- |
| USN02 9200/9000 | **exit 3.** One call, `found`, `missing=0`. Alden's `movetopos` target after 29.75 s moves to a point beyond the nearest map edge, so its step-600 `d32c` and heading target change, and its track and the fights it takes part in move after that. Nothing before 29.75 s changes |
| USN01 3200/3000 | **exit 0.** No call: the mission does not end |
| JM06 3200/3000 | **exit 0.** No call |

#### GetClosestBorderZone pairs and verdict

- OFF is this tree's build of `4c9a1b2fc`.
- ON is `pair_export --commit 4c9a1b2fc --flip kLuaClosestBorderZoneBound=true` (`local/bz_on`).
- The logs are `local/bz_{off,on}_<mission>.log` in worktree cc9-lua5.
- All three rows load the zones (`loaded=1`).

| row | pair_diff | what moved | verdict |
| --- | --- | --- | --- |
| USN02 9200/9000 | exit 3 | see below | held |
| USN01 3200/3000 | exit 1, gameplay identical | only the new summary line's `bound 0 -> 1` | mechanism held; the exit code missed |
| JM06 3200/3000 | exit 1, gameplay identical | the same summary line, plus the recorded ShipAiSectorScan / ShipAiClearance noise | mechanism held; the exit code missed |

The two exit-code misses come from the summary line printing its own switch. That line is new in
this packet; the prediction did not allow for it.

**USN02.** There is one call and it finds a zone:

```
GetClosestBorderZone 008aecd0: (1174.14, -0.10, -6161.64) offset 500.0 -> (1174.14, -0.10, -8550.00)
```

- The nearest record is on the south edge. Its B..C line is `SE.z - 50`, and the answer is 500
  beyond it.
- At step 600 Alden's `movetopos` heads due south (target 3.1413, `d32c` 2392.64). OFF headed
  -0.1883 at 6268.18.
- Nothing before 29.75 s moves: the first hit is 19.20 s on both sides, and the step-590 rows are
  equal.
- Afterwards the fight moves, as predicted:

| measure | OFF | ON |
| --- | --- | --- |
| deaths | 10 | 11 |
| hit records | 4226 | 4899 |
| damage | 50892.0 | 53092.0 |
| controlled moved (Kortenaer) | 1136.82 m | 1218.80 m |

- The death rows: John3 is no longer sunk. Asagumo and Encounter now are. Alden still sinks, at
  135.25 s instead of 135.20 s.

**Verdict: `kLuaClosestBorderZoneBound = true`.**
- USN02's reference row changes after its failure. The failure itself, at 29.75 s, is unchanged.
- The lead should re-anchor USN02 on the next re-baseline.
- **For reference j:** USN02's row moves after 29.75 s under this flip. Alden's post-failure
  order now targets (1174.14, -8550.00), and deaths, hits and damage after that time change.
  Everything up to the failure at 29.75 s is unchanged.
- Provisional ledger names: `004C71C0` BSP_World_LayoutBorderZoneRecords, `004C7150`
  BSP_World_SetBorderZoneSide, `004C7730` BSP_World_FindClosestBorderZone, `008AECD0`
  BSP_LuaNative_GetClosestBorderZone.

**USN04 4700/4500, added at the lead's request** (prediction written before its runs):
- **Prediction: exit 1, gameplay identical.** USN04 does not end, so nothing calls the native. The
  summary line's `bound 0 -> 1` is the only expected difference.
- OFF is `pair_export --commit 4c9a1b2fc` with no flip (`local/bz_off`); ON is `local/bz_on`.
- **Measured: exit 1, gameplay identical; held.** The only changed line is the summary's
  `bound 0 -> 1` (`local/bz_diff_usn04.txt`).

## The unimplemented Lua natives, fifth refresh (cc9-lua6, head `2291cd778`)

Worker cc9-lua6, 2026-09-28. The method and run parameters are the third refresh's. The build is
this tree's at `2291cd778`. JM05 and LOMP10 are new rows: JM05 now has land convoys, and LOMP10
is the controlled-plane row.

| mission | frames | log (worktree cc9-lua6) | host methods unimplemented |
| --- | --- | --- | --- |
| USN02 | 9200/9000 | `local/l6_rk_usn02.log` | 499 |
| JM06 | 3200/3000 | `local/l6_rk_jm06.log` | 509 |
| LOMP06 | 1200/1000 | `local/l6_rk_lomp06.log` | 484 |
| USN13 | 3200/3000 | `local/l6_rk_usn13.log` | 501 |
| JM08 | 3200/3000 | `local/l6_rk_jm08.log` | 486 |
| JM05 | 3200/3000 | `local/l6_rk_jm05.log` | 550 |
| LOMP10 | 3200/3000 | `local/l6_rk_lomp10.log` | 492 |

The scripts are this installation's:
- `PRCPIJN/jm05.lua`, mtime 2024-07-13;
- `USN/LOMP/10_san_jose.lua`, mtime 2024-07-13;
- `global/timetable.lua`, mtime 2024-07-13.

| rank | native | address | missions (calls) | reach |
| --- | --- | --- | --- | --- |
| 1 | `OverrideHP` | `008C1930` | LOMP10 (8) | **gameplay.** `10_san_jose.lua` 291-301 sets the eight San Jose ships to `Class.HP` × 1.0, 1.25 or 1.5 by `Mission.Difficulty`. This run's effective difficulty is 1 (`game+6ACh`). The unit's HP is the gunnery host's, not this lane's |
| 2 | `SquadronSetAttackAlt` | `008A22B0` | LOMP10 (3) | **gameplay.** Lines 315 and 520 set 150 m for the first bombers. It is the sibling of the bound `SquadronSetTravelAlt`, so it needs the units host's plane-side state |
| 3 | `SpawnNewIDIsRequested` | `00946380` | JM05 (16) | **gameplay, beyond 150 s.** `jm05.lua` 1970 re-requests a shipyard Fletcher only when no `SH2SpawnRequest` is queued. The host's queue holds two such requests unfulfilled (`fulfilled=0 still_queued=2`), which the image would answer true for. Its sibling `SpawnNewIDRemove` (`00946390`) is unreached on these rows. The queue and `00945850`/`00945A20` are this lane's `src/lua_spawn_new.cpp` |
| 4 | `AddUntouchableUnit` | `008AC140` | JM05 (3) | the capture PT and `Mission.UntouchUnits` (`jm05.lua` 597, 4508) |
| 5 | `GetLastCatapulted` | `00892860` | JM05 (47) | `SetSkillLevel(pete, SKILL_ELITE)` on the cruiser's last catapulted plane (5812). nil skips it, as in the image when nothing has been catapulted |
| 6 | `GetFormationLeader` | `00899AF0` | JM05 (49) | a nil answer falls back to the table's first ship (1992, 2574, 3159). Line 2042's `GetFormationLeader(unit).ID` raises no error on these rows |
| 7 | `NavigatorEnable`, `GetFailure`, `Countdown` | | JM05 (1, 2), LOMP10 (1) | the capture PT's navigator, a failure query, and the three-minute "operational" timer |
| 8 | `GetCapturePercentage` | `0089B840` | JM05 (48) | presentation: the objective text. nil makes `luaJM5Sec1Score` raise "arithmetic on a nil value" at line 5216, 48 times in 150 s. The raise stops that timer before it reschedules, and `luaTimetable` retries it (`global/timetable.lua` 13-44). No gameplay |
| - | `IsHintActive` (49), `DisplayScores`, `SetGuiName`, `PrepareClass`, `IsGUIActive`, `BlackBars`, `EnableInput`, `Loading_*`, `MissionNarrative*`, hints, `Scoring_IsUnlocked`, `Effect`, `SpawnNewIDRemove`, `LoadCheckpoint`, `CountdownCancel`, `Scoring_SetMissionCompleted`, `BannSupportmanager` | | | presentation, or the failure paths the fourth refresh listed |

**`FindEntity` is listed UNIMPLEMENTED on JM05, and that is right.** Of its 146 calls, 9 answer
nil. (`entity_resolves=139` also counts the other entity-returning natives.) The misses, traced
with `BSP_LUA_FIND_ENTITY_MISSES=1`, are nine names `jm05.lua` asks for that the JM05 scene does
not author:
- `Clemson Class Damaged 01` and `02`;
- `Landing Ship, Tank 02` and `03`;
- `PT Boat 80' Elco 04`;
- `US Cargo Transport 01` and `02`;
- `US Tanker 01`;
- `USTroopTransport 01`.

The image answers nil for them too. The status comes from the host's per-call outcome.

**The host's `SpawnNew` queue never fulfils on JM05:** 290 attempts, 290 requeues, 0 units. That
is the drain `0094C490`'s placement, not a native, and it is recorded here for its owner.

**Next packet: `SpawnNewIDIsRequested` and `SpawnNewIDRemove`,** the highest-ranked item this
lane owns outright. Ranks 1 and 2 need the gunnery host's HP and the units host's plane state.
Both of those files are leased to cc9-gunnery7 at this refresh.

## `SpawnNewIDIsRequested` and `SpawnNewIDRemove`, bound (packet `cc9_spawn_new_id_queries`, `kLuaSpawnNewIdQueriesBound`, ON)

Worker cc9-lua6, 2026-09-28. This is rank 3 of the fifth refresh, and the first rank this lane
owns.

**The read (V, listings `00945850`-`00945A1x` and `00945A20`-`00945C0x`).** Both thunks pass the
`lua_State` to the manager `*(00F89B3C)` (`docs/LUA_BINDING_SPAWN.md`).
- **The argument.** It is Lua slot 1, read through `00B677E0(.., 0)` and `00B662B0` into a
  NativeString (`009458F2`..`00945919`). A value that is not a string gives the empty string.
- **The match.** The scan walks the list from `manager+4h`. A record matches when the lengths of
  `record+B8h` and the argument are equal, and either both are empty or `00BF7FBF` **`__stricmp`**
  returns 0 (`00945948`..`00945986`, and `00945B28`..`00945B5C` in the remove).
  - So the match is case-insensitive.
  - The host's `id_is_requested_00945850` and `remove_id_00945a20` compared exactly. They now go
    through `spawn_request_id_matches`. Nothing called them before this packet.
- **`00945850`** stops at the first match, pushes one boolean (`009459A8` `00B66450`) and returns
  one result (`00B66400`).
- **`00945A20`** frees every match and decrements `+8h`. This is the "twelve bytes Ghidra left
  undisassembled" loop back to `00945B07`. It returns no result.

**The binding.** The two rows run on the host's queue (`bsp::spawn_request_queue()`). A summary
line counts the queries, the true answers and the removals. A replayed error call answers but does
not count, and does not remove.

**Predictions** (written before the ON runs; streams ON, `BSP_DEATH_TABLE=1`, lockstep 0.05, idle
player, present interval immediate):

| row | prediction |
| --- | --- |
| JM05 3200/3000 | **exit 1.** The native row turns concrete (16 calls). `requested=16 true=0 removes=0 removed=0`. The only ids queued in 150 s are the two first `SH2SpawnRequest` Fletchers, and the one test that would see them, `jm05.lua` 1970, is not reached: the OFF run makes no third `SpawnNew`. The other tests ask for `SH1SpawnRequest`, `JapAirGrpSpawnRequest` or `ACargoSpawnRequest`, which are not queued |
| USN01 3200/3000 | **exit 1.** No call; only the new summary line |

#### Spawn id query pairs and verdict

- OFF is this tree's build of `db4547b8c`.
- ON is `pair_export --commit db4547b8c --flip kLuaSpawnNewIdQueriesBound=true` (`local/sq_on`).
- The logs are `local/sq_{off,on}_<mission>.log` in worktree cc9-lua6.

| row | pair_diff | what moved | verdict |
| --- | --- | --- | --- |
| JM05 3200/3000 | exit 1 | The UNIMPLEMENTED row leaves the native table, because a handled binding is not listed there. The census line reads `requested=16 true=0 removes=0 removed=0`, as predicted | held |
| USN01 3200/3000 | exit 0 (predicted 1) | nothing. The census line prints only beside the SpawnNew summary, and USN01 makes no `SpawnNew` | held; the miss is the summary's placement |

**Verdict: `kLuaSpawnNewIdQueriesBound = true`.**
