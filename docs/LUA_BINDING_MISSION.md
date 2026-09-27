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
