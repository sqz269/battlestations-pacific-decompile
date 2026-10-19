# Power-up grant natives (packet `cc13-powerup`, cc9 lead's order 2)

Worker cc13-powerup, 2026-10-19. Ghidra was read-only except the one rename below. Names are
hypotheses, not recovered symbols. Game files are this installation's (it is modded; see
`docs/GAMEPLAY_MODIFIERS.md` for the data tables).

## Result

- New switch `bsp::cc13_powerup::kPowerupGrantBound` (OFF), in `include/bsp/cc13_powerup.hpp`.
  It binds `AddPowerup` 008EE410, `PreparePowerup` 008EA610 and `GetAvailablePowerups`
  008EB350 against a host power-up manager (`src/cc13_powerup.cpp`), loads the class registry at
  the global_subsystems step, and turns 008EB110's `pup_ready` record into the image's report.
- **No consumer of a granted item is reachable in a single-player mission-tree run**, so no
  sub-switch for one was added and no `008E6430` / `00470440` product moves (details below).
- No draw on the shared generator 00BD2F10 is added. The two warning paths it reaches draw from
  the warning-local generator the host already uses for 00979B50 (an existing labelled
  SUBSTITUTION for the image's 00BD2FC0 stream-1 draw, `warning_pick_variant_draw`).
- The OFF build is unchanged: `handles()` refuses the three rows (they stay
  `MissionLuaNative::<name> UNIMPLEMENTED`), the load step is under `if constexpr`, and the
  `pup_ready` arm needs a published manager, which only the ON load makes.

## JM06 in this installation

`missions/COTP-IJN/jm06.lua`, `.../PRCPIJN/jm06.lua` and `missions/ijn/JM/jm06.lua` (all
2024-07-13) carry the same lines:
- `:864` `luaJM6AddPowerup("automatic_reloader")` and `:865`
  `luaJM6AddPowerup("improved_ship_manoeuvreability")`, after primary objective 1 completes with
  no cargo ship leaving through the exit zone. `:1658` wraps each as
  `AddPowerup({classID = type, useLimit = 1})`, one argument, so slot `[game+18ECh]` = 0.
- `global/commandhelpers.lua` (2024-10-29) `:16370`, at every checkpoint:
  `luaRegisterCheckpointData("Powerups", "PowerupTable", GetAvailablePowerups(PLAYER_1))`
  (`PLAYER_1 = 0`, `global/luamw_init.lua:82`). The restore at `:16585..16595` re-grants the
  snapshot; the harness never restores a checkpoint.
- Both classes are `PUT_ACTIVE` (group 2), `PUTT_FRIENDUNIT`: `automatic_reloader` duration 60,
  cooldown 600, filter `PUTFF_SHIPS_AND_SUBS + PUTFF_LANDBASED` (11), `PUET_DEVICE_RELOADING`
  1.08; `improved_ship_manoeuvreability` 60 / 300, filter 3, `PUET_SHIP_SPEED` and
  `PUET_SHIP_TURNFACTOR` 1.175.

The current JM06 log (`cc9-physics1\local\p24_off_j6.log`) has the two grants
(`MissionLuaNative::AddPowerup ... UNIMPLEMENTED`, `argc=1 phase=luaStageInit`) at the primary-1
completion and `GetAvailablePowerups argc=1` at checkpoint 1 (`luaJM6Checkpoint1`).

## What consumes a granted item

A grant only appends to the slot's inventory list. A modifier enters a category list only through
`008EADA0` (`BSP_PowerUpManager_UseItem`), whose four callers (index, and the rel32 census in
`docs/GAMEPLAY_MODIFIERS.md`) are:

| caller | reached in a single-player mission-tree run? |
| --- | --- |
| `008EB705` in `008EB6E0`, from the HUD power-up screen `0061BEA0` at `0061C772` | needs the player's input on a screen the host does not build |
| `008ED601` in `008ED4B0`, the command-building binding grant | not from AddPowerup; scenes with `PowerupClassID` here are JM05 and USN16 (`"repairtest"`, not a class in `PowerupClasses.lua`, so `008E6CB0` misses) and the multiplayer scenes |
| `008EE3C0` in `008EE020` | multiplayer message handler, `game+1FE4h` = 2 only |
| `00A180C9` in `00A179E0`, the party brain's engagement pass | **no.** `00A179E0` runs only under a party brain; brains exist only under the AI coordinator, which `00A32350` builds from `004E1838` (hosted session or forced mode) or `AICreate` `00A373AF`. A mission-tree launch has neither, and no `.lua` file of this installation calls `AICreate` (`kAiCoordinatorLoadGateBound`, `game_hosts_ai.cpp`). The JM06 log's party rows read `brain=0 thinks=0` for both parties |

So in JM06 and every WINS row the category lists stay empty in the image too: the `00470440`
sites (`006496A1`, `006496B7`, `00730619`) and the other sites of the 38-call `008E6430` census
in `docs/GAMEPLAY_MODIFIERS.md` return 1.0f, and the existing 1.0f stand-ins are exact.
`00A179E0` is not wired.

## The bodies (all __fastcall or __thiscall as noted; RET as noted)

| address | host function | what it does |
| --- | --- | --- |
| `008EE410` | `lua_add_powerup_008ee410` | `__fastcall(ECX = Lua state)`, RET. One argument: `008EDDC0([game+18ECh], arg0)` (`008EE4C4..008EE4F0`). Otherwise `008EDDC0(int arg0, arg1)`: argument 1 is fetched first (`008EE506`), argument 0 converted by `00B66290` (`008EE530`). Pushes nothing. The function-local profiler name (`00F88C4C`/`00F88C48`) has no other effect |
| `008EDDC0` | `grant_item_008eddc0` | `__thiscall(ECX = manager, slot, Lua object)`, RET 8. Gate `00E0C978` (`008EDDDD`). Appends a fresh `008E6A80` item to `manager+20h + slot*0Ch` (`008EDE04..008EDE53`), loads it (`008EDE9B` `008E7820`), reports `"pup_gain"` (`00D16270`) through `009789A0` (`008EDED4`), rebuilds the HUD power-up screen when `[00E198C4]+C0h` byte +5 is set (`0061B0F0` at `008EDF10`), triggers `"PUM1STGET"` (`00D162B4`) through `00690FD0(hint, 0)` when `0068E600` says it was not shown (`SETZ` at `008EDF42`), and sends message 67 op 0 when `[game+1FE4h]` = 1 (`008EDFBD`) |
| `008E7820` | `load_item_008e7820` | `__thiscall(ECX = item, reader)`, RET 4. Reader `+10h` (`00BD6830`, string, no default) `"classID"` (`00CF86E0`), `008E6CB0` into item+4h (`008E789D`); reader `+0Ch` (`00BD68D0`, int, default -1) `"useLimit"` (`00D16210`) into item+0Ch |
| `008E6CB0` | `find_powerup_class_008e6cb0` | `__thiscall(ECX = manager, NativeString*)`, RET 4. `008E6530` finds the name in the `+14h` tree; a hit runs `008E34F0` (air-support vehicle classes, group 1 only) and returns node+14h; a miss returns 0 |
| `008EA610` | `lua_prepare_powerup_008ea610` | `__fastcall(ECX = Lua state)`, RET. With exactly one argument, `008E9FD0(arg0)` |
| `008E9FD0` | inside `lua_prepare_powerup_008ea610` | `__stdcall(Lua object)`, RET 4. A temporary item loaded by `008E7820` (`008EA03D`), message 67 op 3 for slot 0 when `[game+1FE4h]` = 1 (`008EA05E..008EA079`), then freed. In single player only the class lookup runs |
| `008EB350` | `lua_get_available_powerups_008eb350` | `__fastcall(ECX = Lua state)`, RET. Slot `[game+18ECh]` unless an argument is given (`008EB40B` `JL`), then `008EAB00` |
| `008EAB00` | inside `lua_get_available_powerups_008eb350` | `__thiscall(ECX = manager, slot, frame)`, RET 8. `008EA0C0`, a new table (`00B67930`), then for i = 1..n `00B672F0(i, [entry+4])`: the class's name as a string at index i (`008EAB66..008EAB75`). One result |
| `008EA0C0` | `list_ready_items_008ea0c0` | `__thiscall(ECX = manager, slot, vector*)`, RET 8. Gate `00E0C978`. For each item of the slot's inventory: `00617030` on `manager+14Ch + slot*0Ch` keyed by the class pointer (an insert of the `008E3710` zeroed record on a miss); skip when record byte +14h is set (`008EA13C`) or when the clock is not above float +10h (`008EA142..008EA14F` `FCOMIP`/`JBE`: unordered skips); then the last entry of the same class is replaced, else the item is pushed |
| `008ECEC0` | `load_powerup_config_tables_008ecec0` (`src/powerup_config.cpp`) | the walk after the script call, split out of the existing `load_powerup_config_008ecec0` with its order unchanged, so the host can run it over its own state |

`008EB110` (existing `update_power_ups_008eb110`, `src/world_entities.cpp`) walks the active
slot's `+14Ch` map. A record whose deadline is past and whose byte +0h is clear latches it and,
when the slot's inventory size (`+28h + slot*0Ch`, `008EB260`) is non-zero, builds `"pup_ready"`
(`00D16228`) and calls `009789A0(slot, last inventory item, name)` (`008EB2ED`). The host's
`power_up_notify_ready_009789a0` now does that report when the manager is published.

`009789A0` (existing `report_powerup_009789a0`) reports only when manager+D0h is clear, the
slot is `[game+18ECh]` and the clock is above 4.0. Its warning is `009758E0` (vtable `00D1B4B0`,
id `"powerup" + name`); its record fields come from that vtable (`+10h` `009758A0` = 8; `+18h`,
`+1Ch`, `+20h` = `0096F220`, `0096F230`, `0096F240`), and its activate `0097FF90` has the torpedo
activate's shape (`0097F490`: the head, `008EC460`, `00979B50`, `00976B00`).

## Host bindings (`src/game_hosts_mission_frame.cpp`, `src/game_hosts_script_orders.cpp`)

- `load_powerup_classes_cc13`, at the global_subsystems step after the warning table: reads
  `Scripts/datatables/PowerupClasses.lua` from the VFS, runs it in a fresh Lua state, constructs
  the owner with the existing `construct_powerup_config_008edc60` and walks it with
  `load_powerup_config_tables_008ecec0`. 32 classes load from this installation's file.
- `PowerupManagerBinding` answers the manager's outside reads. `PowerupWarningReportBinding`
  queues the warning through `report_warning_009763e0` as the torpedo report does.
- `GameScriptOrdersHost` routes the three rows to `src/cc13_powerup.cpp` when the switch is on.
- The frame's `~Impl` destroys the manager it published (`destroy_powerup_manager_004d2d84`) only
  when the switch is on and the runtime's host is its own binding.

**Adoption note (existing argon reconstructions).** `src/powerup_seg58_argon.cpp` already holds
reconstructions of these bodies: `lua_binding_add_powerup_008ee410`,
`power_up_manager_grant_item_008eddc0`, `powerup_item_load_lua_008e7820`, `fun_008e6cb0`,
`lua_binding_prepare_powerup_008ea610`, `power_up_manager_prepare_from_lua_008e9fd0`,
`lua_binding_get_available_powerups_008eb350`, `power_up_manager_build_available_names_table_008eab00`
and `power_up_manager_list_ready_items_008ea0c0`. They are written against the raw `Seg58Host`
form: the manager is a byte image addressed by offset, lists and maps are native node chains
reached through host allocator and STL-instantiation hooks, and Lua is reached through raw
`lua_*` address hooks. Nothing in the running game constructs a `Seg58Host` or the raw manager
image, and the frame's warning manager, world clock and `PowerUpManagerState` are typed host
objects. So the binding here repeats those bodies' control flow over typed containers
(`std::list` items, sorted key/value vectors for `00617030`) and the host's Lua 5.1 state, and
calls the existing typed loaders (`construct_powerup_config_008edc60`,
`load_powerup_config_tables_008ecec0`, `release_powerup_configuration_storage`) rather than the
argon copies. The bodies here were written from the listing (the addresses in the bodies table);
the argon `008EDDC0` was spot-checked against them for the grant order (gate `00E0C978`, append,
`008E7820`, `pup_gain` through `009789A0`, the `[00E198C4]` screen, then the hint) and agrees. The
argon file stays the raw-image record; this file is the typed copy the game can run.

**SUBSTITUTIONS and host facts (labelled in the code):**
- Class textures: the atlas lookup misses and the renderer load answers null (96 requests counted
  for this file). Only the HUD power-up screens read them, and the host builds none.
- `0068E600` answers "not shown" and `00690FD0` is a record: the award tracker `004E1CA0` is not
  hosted. The answer gates only that record.
- `[00E198C4]+C0h` is never shown, so `0061B0F0` is not reached.
- `008E34F0` is a record for a group-1 class (not reached by the JM06 classes).
- `[game+1FE4h]` is 0 and `00E0C978` is 1 (forced by `005E2F00` at `005E2FAB`); message 67
  throws if ever reached.
- A non-table argument has no fields (the image's reader over a non-table was not read).
  `useLimit` is truncated as the host's other integer reads are.
- The allocator words of the 1C4h owner and the descriptor scratch are zero (uninitialised bytes
  in the image).

## ON-only lookups: container, filler, clearing

| lookup | container | filled by | cleared or moved before the read? |
| --- | --- | --- | --- |
| `008E6CB0` in AddPowerup / PreparePowerup | `owner.descriptors_14` (the class map) | `load_powerup_classes_cc13` at each load's global_subsystems step, before any mission Lua runs (004DC6A0 then the mission script) | not before the read. The frame's `~Impl` frees it (`destroy_powerup_manager_004d2d84`, the image's `004D2BB0` release at `004D2D84..004D2D96`), and the next load builds a new one, as `004DC6A0` does at `004DC882..004DC8A9`. A failed run or walk frees it at once and the natives stay records |
| `008EA0C0`'s walk, `008EB110`'s last item | `PowerupRuntime::inventories[slot]` (`manager+20h + slot*0Ch`) | `008EDDC0` only (AddPowerup) | not within a mission. The image removes items only through `008E87C0` (a use spending the last charge, or message 67 op 2) and `008E8A30` (a destroyed command-building binding), neither reached without a use. Between missions the whole manager goes (above), so the host clears the lists with it |
| `00617030` in `008EA0C0`, `008EB110`'s walk | slot 0: `world.power_ups.player_slots` (the existing `PowerUpManagerState`) with the keys in `PowerupRuntime::record_keys[0]`; other slots: `PowerupRuntime::other_records` | `008EA0C0` (inserts the zeroed record); nothing else writes it in these runs (`008E4B00`, the use's stamp, is not reached; no other code writes `player_slots`) | not within a mission: `008EB110` only latches byte +0h in place and never erases. The values live in the frame's world and the keys in the runtime; both start empty at a load (a new frame host, `game_hosts_mission.cpp` `frame_host.reset()` before the new one; a new runtime) and the old frame's `~Impl` clears the keys before the next load, so they stay index-aligned |
| `008EB110`'s gate | `world.power_ups.player_notify_enabled` | `008EDDC0`, set from the active slot's inventory size after each append | no. It starts false in the new world, matching the new manager's empty inventory |
| `009789A0`'s report | the warning manager's pending queue and the +108h table (existing, `kWarningMessageBankCc15Bound`) | existing loaders | existing behaviour |

## Expected ON effect in JM06

Gameplay identical (no use, no category list, no shared draw). Log changes:
- `power-up classes loaded: records=32 texture_requests=96` at the global_subsystems step, and
  the done row `PowerUps::load_classes [008ecec0]`.
- The two `MissionLuaNative::AddPowerup` rows and the checkpoint's `GetAvailablePowerups` row
  become concrete (no `UNIMPLEMENTED`).
- At the grant: two `WarningManager::report_powerup [009789a0]` rows and two outcomes under the id
  `poweruppup_gain` in the warning report summary (accepted only if Warnings.lua's message table
  answers that id; whether the loader flattens `["powerup"]["pup_gain"]` to it was not checked),
  and two `PowerUps::hint_has_been_shown` / `PowerUps::trigger_hint` records.
- At checkpoint 1, `GetAvailablePowerups(0)` returns `{"automatic_reloader",
  "improved_ship_manoeuvreability"}` (checked with an ad hoc probe against this installation's
  file) and inserts two records; on the next `008EB110` both latch and two `pup_ready` reports
  follow (`PowerUps::notify_ready [009789a0]` done instead of a record), the second normally a
  duplicate.
- An accepted warning activates and draws its variant from the warning-local generator, so later
  warning variant texts can differ; no gameplay counter should move.

## Uncertainties

- The text of the `PUM1STGET` hint, the HUD screen and the class textures are not modelled
  (presentation only).
- Whether the warning loader's id for these reports is `poweruppup_gain` / `poweruppup_ready`
  (the concatenation 009758E0 builds) and is known to the +108h table was not checked; the queue
  outcome decides only warning rows.
- The `00BD63B0` conversion of a non-number `useLimit` and the reader over a non-table argument
  were not read; JM06 passes a table with a number.
- The image's slot-record order is by the class pointer value; the host's class pointers differ,
  so two records may be walked in the other order. Both reports carry the same arguments.
- Resolved: the image does not carry inventories across missions. `004DFB70` calls `004DC6A0`
  once per load (`004DFEBB`), which allocates and constructs a new manager. `[00F88C30]` has
  four writers: `004DC8A9` in `004DC6A0` (the new manager); `008EDD8A` in `008EDC60` (the
  constructor's self-publish); `004D2D96` in `004D2BB0`, whose instructions `004D2D84..004D2D96`
  delete the manager through vtable slot 0 and then store 0; and `008EDAFF` in `008EDAD0` (the
  destructor's clear, `XOR EBX,EBX` at `008EDAFD`). The last writer is reached only through the
  scalar deleting destructor `008EDDA0` (its call at `008EDDA3`), which is that same vtable slot 0
  called at `004D2D94`. So every writer belongs to the construct/delete pair, and the host
  mirrors that pair per frame host.
