# Mission Lua machine: the host contract checked against the installed scripts

Addresses: 005e2f00 00884be0 006b8740 006b8610 006b8ad0 006b89f0 00885110 00885fb0 008860b0 00887750 00887b30 00887e50 0045f440 0045f520 00887220 00885da0 006b7e70 006b7ea0 006b8120 006b8460 004d30f0 00886900 00b6a303 00b69e00 00a71350

Packet `cc_mission_lua`, worktree `agent/cc-mission-lua`. Ghidra was read-only. Every name here
is a hypothesis, not a recovered symbol.

`docs/MISSION_LUA_HOST.md` already recovered the machine itself: the seven standard libraries,
the 560-row binding table, the chunk rule, the named-call rule and the `game+644h` guard, all
reconstructed in `include/bsp/mission_lua_host.hpp` and `src/mission_lua_host.cpp`. This packet
did not redo any of that. It did two things that document could not: it **checked the recovered
contract against the real installed scripts** by building a stock Lua 5.1.1 state the way
`006B8740` does, and it closed the three gaps the check exposed.

The check is `src/mission_script_probe.cpp`, executable `bsp_mission_script_probe`. Over the 299
installed mission scripts, **293 load and run all four engine entry points with no error**. The
six that do not are characterised at the end; none of them is a contract error.

## What the check confirmed

Confirmed unchanged from `docs/MISSION_LUA_HOST.md`, now against real scripts rather than by
reading alone:

| Claim | How the probe shows it |
| --- | --- |
| Seven libraries, no `package` | the global `require` is absent and no script asks for it |
| 560 bindings as plain globals | first `Log`, last `TerminateExecution`; the table at `00E0B7B8` runs to the `{"", NULL}` row at `00E0C938`, and 560 rows of eight bytes is exactly that span |
| `errfunc` 0 for a chunk, `debugtrap` for a named call | the handler is reached as a call with one argument, only on the named-call path |
| the four entry-point names and their wrappers | all four resolve and run; a script that omits one is skipped, which is the `0045F440`/`0045F520` defined-check |

`debugtrap` returning no results is not cosmetic. Lua 5.1 replaces the error object with the
handler's return value, so a handler that returns nothing leaves the caller holding nil. The
probe reruns a failed entry point with `errfunc` 0 purely to recover the message the native
discards. **A failed mission entry point tells the engine nothing except that it failed**, which
matches `00885110` discarding its error string unread.

## Corrections

`docs/MISSION_LUA_HOST.md` states twice that `00886900` is the host teardown reached from
`BSP_Game_OnDestroy`, in "The two objects" and in "What remains". That is wrong. `00886900` is
`BSP_MissionLua_RunGlobalScriptFolders`: its callers are `004DC6A0`
`BSP_Game_ConstructGlobalSubsystems` and `004E3AA0` `BSP_Game_OnInit`, it runs `Scripts/global/`
then `Scripts/datatables/autoload/` through `00886370`, and it is already reconstructed in
`src/global_script_folders.cpp`. It is a **bring-up** step, not a teardown, and the mission
scripts depend on it. The teardown at `004DC729` is a different address and remains unread.

Previous value recorded before this correction: "`00886900`, the host teardown reached from
`BSP_Game_OnDestroy`."

## Gap 1: the binding table is not the whole global namespace

The first executable line of `Scripts/missions/usn/usn_2_java.lua` is `DoFile(...)`, and
**`DoFile` is not one of the 560 rows**. It is installed from the LuaStateOwner layer at
`00B6A303` with callback `00B69E00`, over the same `lua_State`: `game+1A0Ch` is an inline
`LuaStateOwner` built at `004DD641` from `[[game+1A08h]+4h]+4h`. A rebuild that installs only the
binding table produces a state on which every stock mission script fails at its first line.

Installed-file-checked: both paths the stock script passes, `Scripts/datatables/Inputs.lua` and
`shipnames.lua`, resolve against the install root. The callback applies no directory of its own,
so the argument is a plain VFS-root-relative path.

`include/bsp/mission_lua_machine.hpp` records the five sources in the order the game reaches
them, as `kMissionLuaGlobalSources`.

## Gap 2: the mission script name carries its subdirectory

`008860B0` concatenates `"Scripts/missions/" + name + ".lua"` with no directory walk.
**`Scripts/missions/` contains no loose `.lua` file at all**, only the eight subdirectories
`COTP-IJN`, `COTP-USN`, `bsm`, `chg`, `ijn`, `multi`, `traininggrounds`, `usn`. So the name the
scene record carries at `+928h` must include the subdirectory: `usn/usn_2_java`, not
`usn_2_java`. The probe prints both resolutions; the bare form is absent on disk.

This is not the short name `derive_scene_short_name` (`004CD7F0`) produces, which truncates at
the second `_` and has no directory. The two are different fields of the same record, and
`docs/MISSION_SCENE_LOAD.md`'s line "`008860B0` runs `Scripts/missions/<name>.lua`" is correct
only when `<name>` is read as the record's script-table entry rather than the short name.

## Gap 3: the matched Lua library is built with the old long-string behaviour

Two shipped scripts nest a long bracket inside a long bracket:
`Scripts/global/commandhelpers.lua` at line 17625 nests `--[[` inside `--[[`, and
`Scripts/datatables/autoload/vehicleclasses.lua` does the same at line 190011. Stock
`lua-5.1.1` sets `LUA_COMPAT_LSTR` to 1 in `luaconf.h`, and `llex.c` then rejects that with
`nesting of [[...]] is deprecated`. Under the stock setting neither file compiles, and
`luaLoadControlFunctionNames`, which `commandhelpers.lua` defines and which 282 of the 299
mission scripts call from `luaStageInitMulti`, never exists.

The executable settles it. `00A71350` is that same lexer function and carries its other two
messages, `unfinished long comment` and `unfinished long string`, but **not** the nesting
message, which sits inside `#if LUA_COMPAT_LSTR == 1` in the same function. The game therefore
built with the value 2, the old behaviour.

`cmake/lua.cmake` now rewrites the vendored `luaconf.h` at configure time. A command-line `-D`
cannot do it: `luaconf.h` defines the macro unconditionally and wins. The rewrite is idempotent
and nothing else in the repository referenced either the macro or the message. **This changes a
target other workers share, `bsp_lua511`, and should be reviewed at integration**; it only adds
compatibility, in the direction of matching the executable.

## The load-path Lua steps

`kMissionLoadLuaSteps` in the header, in call order. `00886900` runs earlier than the others and
is listed because the mission chunk depends on the globals it leaves behind.

| Address | Name | Effect | Inside `game+644h` |
| --- | --- | --- | --- |
| `00886900` | `BSP_MissionLua_RunGlobalScriptFolders` | `Scripts/global/` then `Scripts/datatables/autoload/`; from `004DC6A0` and `004E3AA0` | no |
| `005E2F00` | `BSP_Game_SyncLobbySettingsFromLua` | the `LobbySettings` global table and the three mode flags | no |
| `004D30F0` | `BSP_Game_RefillScriptedNameList` | rebuilds the name set at `game+1934h` from a Lua global table | no |
| `008860B0` | `BSP_MissionScript_RunFile` | the mission chunk, variants enabled | yes |
| `0045F520` | `BSP_Game_CallLuaEntryPointForced` | `luaPrecacheUnits`, `luaStageInitMulti` | yes |
| `0045F440` | `BSP_Game_CallLuaEntryPointThreadSafe` | `luaStageInit`, and `luaEngineMovieInit` for slot 9 | yes |

### `005E2F00`, the `LobbySettings` table

`__fastcall void(void)`, body `005E2F00..005E3282`, sole caller `004DFB70`. It opens the global
table named by `00CF1F28` through the `LuaObject` API and walks the pointer array at `00E08908`.
The array has fourteen slots and the fourteenth is null, which is why the loop at `005E2F6F`
skips index `0Dh` **by number** rather than by testing the pointer. Slots 0 and 2 are excluded
at `005E2F8B` from the branch that reads a stored value back, so `PlayerCount` and `GameMode`
are always written from the game side.

| Slot | Field | Slot | Field |
| --- | --- | --- | --- |
| 0 | `PlayerCount` (game-owned) | 7 | `TimeLimit_IC` |
| 1 | `Map` | 8 | `PointLimit` |
| 2 | `GameMode` (game-owned) | 9 | `RoundLimit` |
| 3 | `MapSize` | 10 | `EnablePowerups` |
| 4 | `UnitType` | 11 | `EnableMap` |
| 5 | `ResourceLimit` | 12 | `ReloadPayload` |
| 6 | `TimeLimit` | 13 | null, skipped |

Installed-file-checked both ways. The multiplayer scripts under `Scripts/missions/multi` read
exactly eight of these thirteen names and no name outside the table: `GameMode`, `MapSize`,
`PointLimit`, `ReloadPayload`, `RoundLimit`, `TimeLimit`, `TimeLimit_IC`, `UnitType`. And four
duel scripts that index `LobbySettings` at their first line fail without the step and pass with
it, which is what places `005E2F00` before the entry points.

## Host methods the executable must implement, in call order

Every one is an existing pure virtual on `MissionLuaHostServices` in
`include/bsp/mission_lua_host.hpp` unless marked new. The native call site is given for each.

| Order | Method | Native site |
| --- | --- | --- |
| 1 | `create_state` | `006B8740` -> `luaL_newstate` `00A6A260` |
| 2 | `set_panic_function` | `006B8759` -> `lua_atpanic` `00A67390` |
| 3 | `set_gc_pause` | `006B8768` -> `lua_gc` `00A68280` |
| 4 | `open_standard_library`, seven times | `006B8790` loop over `00CF8350` |
| 5 | chunk runner for `PC=true` | `00884BE0` -> `006B8AD0` |
| 6 | `register_global_function`, 560 times | `006B8610` |
| 7 | chunk runner for `fundamentals` | `00884BE0` -> `006B89F0` |
| 8 | **new**: install the `DoFile` global | `00B6A303`, callback `00B69E00` |
| 9 | folder walk, `.luab` then `.lua` | `00886900` -> `00886370` |
| 10 | **new**: publish the `LobbySettings` table | `005E2F00` |
| 11 | `open_script` / `script_size` / `read_script` / `close_script` | `00885110` on `[0109CEEC]` |
| 12 | `script_variant_names` | `00BDEF90`, reconstructed in `src/vfs_lua_scripts.cpp` |
| 13 | `global_is_defined` | `00B66200` via `0045F440` / `0045F520` |
| 14 | named call: `lua_getglobal`, `lua_pushstring`, `lua_gettable`, `lua_remove`, `lua_pushvalue`, `push_argument`, `lua_pcall`, `collect_results` | `00887750` |
| 15 | `game_lifecycle_state`, `adjust_call_stack_marker`, `adjust_reentrancy_depth` | `00887782`, `00887986`, `00F87900` |
| 16 | `on_frame_job_thread`, `queue_named_call_for_main_thread` | `004C1130`, `00887C30` |

Steps 8 and 10 are the two the existing seam does not cover; both are recorded in
`include/bsp/mission_lua_machine.hpp`.

## The probe

`src/mission_script_probe.cpp`, `bsp_mission_script_probe [game-root] [mission-name]
[--stub-dofile] [--skip-global-folders] [--skip-lobby-settings]`. It links the repository's real
Lua 5.1.1 and reuses `mission_lua_bindings()`, `split_lua_entry_point_name()`,
`kMissionLuaEntryPoints` and `mission_script_path()` rather than restating them.

Two deliberate divergences, both noted in the source. Each binding is installed as one C
function plus a row-index upvalue, because `006B8610`'s `nup` of 0 works only when every row is
a distinct native function; the global is still a plain C closure under a plain name. And the
folder walk sorts by name, because the native's order comes from the VFS provider list, which a
disk-only probe cannot reproduce.

Sweep over all 299 installed mission scripts:

| Outcome | Count |
| --- | --- |
| chunk loads, all defined entry points run, no error | 293 |
| an entry point indexes the result of a stubbed binding | 6 |

The six are `COTP-IJN/jm04`, `ijn/JM/jm04`, both copies of
`COTP-USN/usn_02_battle_of_cape_esperance`, `bsm/bsm_04_vengance_at_luzon` and
`bsm/bsm_06_holding_lombok`. Two index `FindEntity(...).Class.Height`; the rest reach
`commandhelpers.lua` code that indexes the global `Mission`, which mission scripts assign from
the engine-supplied `this` inside `luaStageInit`. All six need a binding to return a real game
object, which a logging stub cannot. They are a limit of stubbing, not a contract error.

## Uncertainties

- The probe supplies `LobbySettings` field values of 0 because they come from game state it does
  not have. Which slots hold strings is not established; `UnitType` is compared against
  `"globals.unitcat_destroyer"` in the multiplayer scripts, so at least that one is a string.
- `006B8720`, the panic function, was not read; the probe installs its own.
- `004D30F0`'s container was not identified, and the probe does not perform that step. No
  installed script failed for want of it, so its globals are not read at mission load time.
- The `.luab` branch of `00886370` is never taken on the installed copy: neither folder ships a
  compiled script, so the shadowing rule is carried but unexercised.
- Whether the LuaStateOwner over `game+1A0Ch` installs anything beyond `DoFile` onto the mission
  state was not established. No installed mission script needed a second such global.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_lua_self_table` | 00887750 00884240 00ce7494 | docs/MISSION_LUA_SELF_TABLE.md | The `thisTable` self-object: who populates it, what `this` is inside `luaStageInit`, and how the global `Mission` the six failing scripts need comes to exist |
| `lua_binding_find_entity` | the `FindEntity` and `Effect` rows of 00e0b7b8 | docs/LUA_BINDING_ENTITY.md | The return convention of the entity-returning bindings, which is what the six remaining probe failures need |
| `mission_lobby_settings_values` | 005e2f00 00e19564 00e19584 | docs/MISSION_LOBBY_SETTINGS.md | Where each `LobbySettings` slot's value comes from and which are strings |
| `mission_lua_teardown` | 004dc729 and the host destructor | docs/MISSION_LUA_TEARDOWN.md | The real teardown, which the corrected `00886900` reading leaves unread |

## no_ghidra_function

none. Every address this packet touched has a Ghidra function, verified with
`python tools/bsp.py ghidra proto` over all twenty. In particular `00887750`
(`00887750..008879E4`) and `00887B30` (`00887B30..00887B54`) now have functions, so the packet
brief's expectation that they would have to be read from the raw listing no longer holds.

## Reconstruction state

| Address | Name | State |
| --- | --- | --- |
| 005e2f00 | `BSP_Game_SyncLobbySettingsFromLua` | analysed; field table reconstructed and installed-script-checked |
| 00886900 | `BSP_MissionLua_RunGlobalScriptFolders` | corrected reading; reconstruction already in `src/global_script_folders.cpp` |
| 008860b0 | `BSP_MissionScript_RunFile` | name rule corrected, installed-file-checked |
| 00b6a303 | `DoFile` registration | analysed; placed in the global-source order |
| 00a71350 | Lua 5.1.1 lexer | string evidence only; settles `LUA_COMPAT_LSTR` |
| all other packet addresses | see `docs/MISSION_LUA_HOST.md` | unchanged; checked against the installed scripts by this packet |

Build-tested at `/W4 /WX`. Installed-file-checked: 299 mission scripts, 21 global and autoload
scripts, `Scripts/fundamentals.lua`. Not game-validated.
