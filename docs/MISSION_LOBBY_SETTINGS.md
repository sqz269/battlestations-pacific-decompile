# Mission lobby settings (`005E2F00`, the `LobbySettings` table)

Addresses: 005e2f00 005d5500 005e2320 004bca50 00576b10 00575fd0 00573410 00575e80 005732c0 008d2f50 005d49e0 005d6350 0076c2c0 00b67350 00b67460 00b67580 00b67630 00b67700 00b67800 00b67980 00e1955c 00e19564 00e19584 00e19588 00e1958c 00e19594 00e0c978 00e17bf2 00e08880 00e0cfb4 00e08908

Packet `cc2_lobby_settings`, worktree `agent/cc2-lobby-settings`. Ghidra was read-only for this
packet; every descriptive name is a hypothesis, not a recovered symbol. This is the follow-up to
the section "`005E2F00`, the `LobbySettings` table" in `docs/MISSION_LUA_MACHINE.md`, which
recovered the thirteen field names and the slot numbering but not the values. The reconstruction
is `include/bsp/mission_lobby_settings.hpp` and `src/mission_lobby_settings.cpp`.

## The routine

`__fastcall void(void)`, body `005E2F00..005E3282`, sole caller `004DFB70`
`BSP_Game_LoadMissionScene` at `004E02D0`. Coverage: **complete** for the normal flow
`005E2F00..005E3282`; the SEH unwind funclet at `00C74F38` was not read.

| Step | Site | What it does |
| --- | --- | --- |
| 1 | `005E2F2A` | build the `NativeString` `"LobbySettings"` from `[00E08904]` |
| 2 | `005E2F48` | `LuaStateOwner::GetGlobals` on **`game+1A0Ch`** (`MOV ECX,[00E188A8]` + `ADD ECX,1A0Ch`) |
| 3 | `005E2F59` | `globals.LobbySettings = nil` |
| 4 | `005E2F93..005E2FE2` | the mode gate and the three mode flags (below) |
| 5 | `005E304B` | `globals.LobbySettings = {}` (a fresh table) |
| 6 | `005E30B1` | `t = globals.LobbySettings` |
| 7 | `005E30C7..005E3202` | the fourteen-slot loop, index `0Dh` skipped by number |
| 8 | `005E3214` | release the table object |
| 9 | `005E321F..005E326F` | the command-point float `00E0CFB4` |

Step 3 runs before the gate, so the **single-player early-out leaves the global nil**: at
`005E2FC0` the single-player branch jumps straight to `005E321F`, past steps 5 to 8. A mission
script that indexes `LobbySettings` outside multiplayer therefore indexes nil.

## The gate

`005E2F99` reads `game+1FE4h` (the network session) and `005E2FA2`/`005E2FC5` read `game+61Ch`
(the forced-mode byte that `004BCA50` `BSP_Game_GetEffectiveGameMode` also tests).

| `game+1FE4h` | `game+61Ch` | Behaviour |
| --- | --- | --- |
| 0 | 0 | single player: flags forced to `(1, 0, 1)` at `005E2FAB..005E2FB9`, table left nil |
| any | non-zero | `005E2FCE`: `00E19564 = BSP_Game_GetEffectiveGameMode()`, and `00E19568 = ` the same value when it is below 4; then publish |
| non-zero | 0 | skip the mode write (the session's own values stand), then publish |

`ECX` at the `004BCA50` call sites is the game object: `005E2F93` loads `[00E188A8]` into `ECX`
and nothing between there and `005E2FCE` writes `ECX` (the multiplayer path reloads it at
`005E3021` and `005E3219`; only `CL` is written, at `005E300A`, on a path that does not reach
either call). Filtered from the whole `005E2F00` listing for `ECX`, not from one idiom.

The `005E2FDD` write is the one non-obvious rule in the routine: **the four game modes below 4
are the four map sizes of Island Capture**, so the mode index doubles as the `MapSize` option
index. `MultiLobbySettings[2]` in the installed datatable confirms it: options 0 to 3 are all
`globals.gamemode_islandcapture`, and `MultiLobbySettings[3]` is small/medium/large/huge in that
order.

## The stored values, `00E1955C`

`00E1955C` is an array of **fourteen `int` option indices**, one per slot, mirroring the
fourteen-slot name array at `00E08908`. It is *not* the option value; it is the index into that
slot's option list.

| Address | Producer / consumer | Evidence |
| --- | --- | --- |
| `005D49EA` | copies fourteen dwords from a source block into `00E1955C..00E19590` | loop bound `< 0E19594h` in `FUN_005D49E0` |
| `005D6388` | the lobby UI writes the slot whose menu row is 1 | `FUN_005D6350` walks `005D5500` for a row of 1 |
| `0076C310` | the network state block copies `00E1955C..00E19593` into a `88h`-byte record at `+30h..+67h` | `FUN_0076C2C0`, stride 8, two dwords per iteration |
| `005E3158` | this routine reads `[ESI*4 + 00E1955C]` | the loop |

So the array is host-authored lobby state that is replicated to clients, and `005E2F00` is the
step that republishes it into Lua on every scene load. The two addresses the packet named are
slot 2 (`00E19564`, `GameMode`) and slot 10 (`00E19584`, `EnablePowerups`).

## The option registry

`00576B10` is a lock-guarded singleton getter (`operator new(0C4h)`, constructor `005769E0`,
registered with the singleton lifetime manager). Two of its members matter here, both
`std::map<int, std::map<int, T>>`:

| Member | Accessors | Holds |
| --- | --- | --- |
| `this+1Ch` | `00575FD0` then `00573410` | the **integer payload** of `(slot, option)` |
| another member | `00575E80` then `005732C0` | the **label pointer** of `(slot, option)` |

The `+1Ch` offset is read from `LEA ECX,[EAX+1Ch]` at `005E313D`, `005E3185` and `005E3249`. The
label map's own offset was not read; the constructor `005769E0` builds five maps, at `+4h`, `+10h`,
`+1Ch`, `+28h` and `+34h`.

Both accessor pairs are `std::map::operator[]`, so a missing key inserts a zero rather than
failing. `00575FD0` ends `RET 4` and `00573410` consumes the second pushed pointer: the two
pushes at a call site are `(outer key = slot, inner key = option)`, in that order after the
last-push-is-first-argument rule.

`008D2F50` populates both maps from `Scripts\datatables\MultiGlobals.lua`, table
`MultiLobbySettings` (`BSP_LuaObject_GetByName` at the site listed below), iterating slot then
option then the one key/value pair of each option:

- a **string** key writes the label into the label map and **0** into the integer map;
- an **integer** key (`BSP_LuaObject_IsInteger`) writes 0 into the label map and
  `BSP_LuaObject_GetInteger`'s result into the integer map.

The three integer-map writes are at `008D378A`, `008D387F` and `008D3B16`; which branch each one
belongs to was taken from the decompiled body, not from the listing.

## The thirteen slots

`005E2F00` chooses the Lua type per slot by probing option 0 of that slot in the integer map
(`005E3140`/`005E3147`, key `(slot, 0)`): a non-zero payload means the slot's options carry
numbers, a zero means they carry labels. Checked against the installed datatable, which gives
integer keys for exactly one slot.

| Slot | Field | Lua type | Source | Conversion |
| --- | --- | --- | --- | --- |
| 0 | `PlayerCount` | string | `game+218Ch`, `game+2017h`, `game+2018h`, or the eight slot records at `game+18D0h-4` | `005E2320`: `"<+2018h>/<+2017h>"` when `game+218Ch` is 0; otherwise the count of active slots when the mode is 4 or above, else the literal 2 |
| 1 | `Map` | string | mission record `00E19594` through `004BE940`, field `+920h` | `005E2320`: the record's name string, or `"FE.unknown_mapname"` when the index is past `game+5F0h`, the record is null, or `+914h` is 0 |
| 2 | `GameMode` | string | `00E19564`, overwritten at `005E2FD6` from `004BCA50` when forced | label of `(2, value)` |
| 3 | `MapSize` | string | `00E19568`, overwritten at `005E2FDD` with the mode when it is below 4 | label of `(3, value)` |
| 4 | `UnitType` | string | `00E1956C` | label of `(4, value)` |
| 5 | `ResourceLimit` | **number** | `00E19570` | `payload(5, value)`, the command-point count (9600, 4800, 3600, 2400, 1200, 900, 600, 300) |
| 6 | `TimeLimit` | string | `00E19574` | label of `(6, value)`: `globals.none`, `globals.5m` .. `globals.30m` |
| 7 | `TimeLimit_IC` | string | `00E19578` | label of `(7, value)`: `globals.none`, `globals.30m`, `globals.1h`, `globals.2h`, `globals.4h` |
| 8 | `PointLimit` | string | `00E1957C` | label of `(8, value)`: `globals.none`, `"500"` .. `"5000"` |
| 9 | `RoundLimit` | string | `00E19580` | label of `(9, value)`: `globals.none`, `"1"`, `"3"`, `"5"`, `"7"`, `"9"` |
| 10 | `EnablePowerups` | string | `00E19584` | label of `(10, value)`: `globals.enable`, `globals.disable` |
| 11 | `EnableMap` | string | `00E19588` | label of `(11, value)`: `globals.enable`, `globals.disable` |
| 12 | `ReloadPayload` | string | `00E1958C` | label of `(12, value)`: `globals.on`, `globals.off` |
| 13 | null name | not published | `00E19590` | skipped by number at `005E30C7` |

**Twelve of the thirteen fields are strings and only `ResourceLimit` is a number**, because
`MultiLobbySettings[5]` is the only slot whose option keys are integers. Installed-script checked:
every use of a `LobbySettings` field in `Scripts\` is a string comparison, including
`LobbySettings.PointLimit == "500"` (`missions\multi\competitive01.lua:121`) and
`LobbySettings.RoundLimit == "1"` (`missions\multi\duelOne.lua:311`), which are the numeric-looking
labels the datatable spells as strings. `MultiLobbySettings[0]` and `[1]` are empty tables, which
is why slots 0 and 1 fall to the zero payload and take the string branch as well.

The label itself comes from `005E2320`, which for a normal slot reads the label map at
`(slot, value)` and, when that entry is null, formats the integer payload with
`BSP_NativeString_FromInt` after the integer-map lookup at `005E26FB`. That is the only place a
numeric option can
still reach a script, and it reaches it as a string.

## The nil rule

A slot is published as nil when `005D5500(slot)` returns `-1` **and** the slot is neither 0 nor 2
(`005E30D7`, `005E30DC`, `005E30E3`). `005D5500` is `__fastcall int(int slot)`: it returns 0 for
slot 1 unconditionally, otherwise switches on `004BCA50`'s effective mode. The return value is a
lobby menu row, and `-1` means "this setting is not offered in this mode".

| Effective mode | Slots with a row (published) | Slots published as nil |
| --- | --- | --- |
| 0..3 (Island Capture) | 1, 3, 5, 7, 8, 10, 11, 12 | 4, 6, 9 |
| 4 | 1, 4, 6, 9, 11, 12 | 3, 5, 7, 8, 10 |
| 5, 6 | 1, 11, 12 | 3, 4, 5, 6, 7, 8, 9, 10 |
| 7 | 1, 6, 8, 11, 12 | 3, 4, 5, 7, 9, 10 |
| anything else | 1 | 3..12 |

Slots 0 and 2 are always published. `005D5500` also answers for slot `0Dh`, which the publishing
loop never reaches; that row belongs to the menu, not to the table.

The mode-to-row map is the reason `TimeLimit` and `TimeLimit_IC` coexist: Island Capture publishes
slot 7 and nils slot 6, mode 4 publishes slot 6 and nils slot 7. `duelOne.lua` reads `TimeLimit`,
`islandcapture*.lua` read `TimeLimit_IC`.

## The three mode flags

`005E2FE2..005E3017`, one `SETZ` each, all three inverted against option index 0:

| Global | Rule | Single-player value (`005E2FAB`) | Reader |
| --- | --- | --- | --- |
| `00E0C978` | `00E19584 == 0`, so `EnablePowerups == globals.enable` | 1 | `008E9BE1` `BSP_PowerUpManager_CollectActive` |
| `00E17BF2` | `00E1958C == 0`, so `ReloadPayload == globals.on` | 0 | `007B6381`, `007B58D9`, `007EEC09`, `0084E53D` |
| `00E08880` | `00E19588 == 0`, so `EnableMap == globals.enable` | 1 | `0068AC30` |

Each is a byte. `00E17BF2` is also written outside this routine (`0076FE6C`, `008C1458`), so it is
not owned by the lobby alone; the reader attributions are provisional, only the write rule here is
established.

## The command-point float

`005E321F..005E326F` runs on **both** paths, single player included. With an effective mode below
4 it reads `payload(5, 00E19570)`, doubles it and stores the float in `00E0CFB4`; otherwise it
stores the constant `2400.0f` from `00CE396C` (`45160000h`). Read by `009421D5` and `00946FC8`.
`00E0CFB4` is written nowhere else.

## Host methods

One row per native call site inside `005E2F00`. `this` is `game+1A0Ch` for the state owner, the
returned globals object for the three globals writes, and the table object built at `005E30B1`
for the per-slot writes.

| Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `005E2F48` | `00B67980` | `lua_globals` | `game+1A0Ch`, out object / none | none |
| `005E2F59` | `00B67350` | `set_global_nil` | globals, name `"LobbySettings"` | none |
| `005E2FCE` | `004BCA50` | `effective_game_mode` | game / int | `game+61Ch != 0` |
| `005E304B` | `00B67580` | `set_global_new_table` | globals, name | multiplayer path |
| `005E30B1` | `00B67800` | `open_global_table` | globals, out, name | multiplayer path |
| `005E30D2` | `005D5500` | `menu_row_for_slot` | slot in `ECX` / int | per slot |
| `005E3104` | `00B67350` | `table_set_nil` | table, name | row `-1` and slot not 0 or 2 |
| `005E312E` | `00576B10` | `option_registry` | none / registry | per slot |
| `005E3140` | `00575FD0` | `registry_numbers_for_slot` | registry`+1Ch`, `&slot` | per slot |
| `005E3147` | `00573410` | `number_for_option` | inner map, `&0` | per slot |
| `005E31A0` | `00B67460` | `table_set_number` | table, name object, int | payload of option 0 non-zero |
| `005E31BD` | `005E2320` | `option_label` | out string in `ECX`, slot in `EDX` | payload of option 0 zero |
| `005E31D4` | `00B67630` | `table_set_string` | table, name `char*`, string object | payload of option 0 zero |
| `005E3214` | `00B67700` | `close_table` | table | multiplayer path |
| `005E321F` | `004BCA50` | `effective_game_mode` | game / int | none |

`00B67460` is the number setter: its body is `lua_checkstack`, `lua_pushlstring(name)`,
`lua_pushnumber((double)value)`, `lua_settable` (callee body read at `00B67460..00B674B5`, rule 1).
`0041E870`, `00419CC0`, `00BD1510` and `00B67700` are the string, pool and object-lifetime
contracts and are not host methods.

## Uncertainties

- The registry's contents are read from `MultiGlobals.lua` in the installed game and from the
  shape of `008D2F50`, not from a run. A modded datatable with an integer key in another slot
  would move that slot to the number branch; the rule, not the table of types, is what is
  established.
- `game+2017h`/`+2018h` are bytes read for `PlayerCount`; whether they are per-team sizes or a
  current/maximum pair is not established. `game+218Ch` selects between that form and the live
  count and is not identified.
- The reader attributions for the three flag globals are single call sites, not full surveys.
- No run-time evidence: `bsp_game.exe` reports this step as an unimplemented host
  (`docs/GAME_EXECUTABLE.md` milestone 2f), so there is no log line to quote yet. Rule 6 is
  unsatisfied for this packet and will stay so until the host is implemented.
