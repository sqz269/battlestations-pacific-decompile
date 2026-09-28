# Gameplay modifiers: the power-up registry behind `008E6430` (packet `cc9_gameplay_modifier_registry`)

Worker cc9-ships2, on main `2f0217432`. Ghidra was read-only, and every body named here has a
Ghidra function. Names are hypotheses. The installation files cited are under
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/scripts/`, with their mtimes.

## Answer

**The "gameplay modifiers" are active power-ups.**
- The lists at `[00F88C30]+80h + category x 0Ch` belong to the power-up manager.
- `00E0C978` is `EnablePowerups`. The lobby routine `005E2F00` forces it to 1 in single player at
  `005E2FAB` (docs/MISSION_LOBBY_SETTINGS.md).

**A modifier enters a list only when a player or an AI party brain uses a power-up item it holds.**
Items reach an inventory only through the Lua native `AddPowerup`.

**On every mission measured so far, no modifier can register:**
- USN02, USN01, E2 (bsm_01), LOMP06 and USN13 grant nothing.
- USN04 grants two items, only after its secondary or hidden objectives complete.
- JM06 and JM08 load scripts that are not loose files here.
- The host logs record **zero calls** to `AddPowerup`, `PreparePowerup` or `GetAvailablePowerups`
  in any run in this tree.

With empty lists, every product site returns exactly 1.0f, and it does so before walking. The
host's empty-list 1.0 is therefore the image's value on these runs. **The residual is closed. No
binding contract is written.**

## The registry

| step | address | what |
| --- | --- | --- |
| grant | `008EE410` `AddPowerup` | one argument: `008EDDC0([game+18ECh], table)`, the local player. Two arguments: `008EDDC0(int arg1, arg0)` |
| grant body | `008EDDC0` | gated on `00E0C978`. Appends the item to the player's inventory list at `manager+20h + player x 0Ch`, plays `pup_gain`, and shows the `PUM1STGET` hint once. When `game+1FE4h` = 1 (host) it sends session message 67. **It does not touch the category lists** |
| use | `008EADA0` (item, player, target) | gated on `00E0C978`. On a multiplayer client (`game+1FE4h` = 2) it only sends the request. Otherwise it checks the item's cooldown (`manager+14Ch + player x 0Ch`). An air-support item (class+8h = 1) launches a flight (`0094BFF0`). Any other item walks the class's 16 multipliers at class+8Ch and calls **`008EA250(category, value, ...)` for each value that is not 1.0f**, the only list insert. Then `008E4B00` stamps the expiry (class+7Ch duration + now) and the cooldown (class+80h + now), and the use count drops |
| expire | `008EB110` `BSP_PowerUpManager_Update` | every `Game_OnMove`: for each of the 16 lists (`+84h`, stride 0Ch), a node whose end time (node+20h) is past goes to `008E8C30` |
| product | `008E6430` | 1.0f times node+1Ch for each node whose filter `008E4680`(node+8h, unit) accepts |

The use routine's insert loop, from the listing:

```
008EAFA8  UCOMISS XMM0,[00D7A24C]     ; the multiplier against 1.0f
008EAFB9  JNP 008EAFE0                ; equal: skip
008EAFC6  PUSH EAX / EBX / EBP / ESI / ECX(value) / EDI(category)
008EAFD3  CALL 008EA250               ; the only caller of the insert
```

**A rel32 and absolute-dword census of the image on disk** (`local\cc9-ships2-rel32.py`) finds:
- `008EA250`: one caller, `008EAFD3`.
- `008EDDC0`: the two arms of `AddPowerup`, `008EE4F0` and `008EE53C`.
- `008EE410`: only its binding row, `00E0C884`.
- `008EADA0`: four callers.
  - `008EB705` and `008ED601`, the local use paths.
  - `008EE3C0`, the multiplayer message handler `008EE020`, which runs only when
    `game+1FE4h` = 2.
  - **`00A180C9` in the AI party brain's engagement pass `00A179E0`.**

```
00A180B5  MOV ECX,[ESP+34h]           ; target
00A180B9  MOV EDX,[ESP+38h]           ; the brain
00A180BE  MOV ECX,[EDX+20h]           ; brain+20h, its party slot
00A180C2  MOV ECX,[00F88C30]
00A180C8  PUSH EAX                    ; the item
00A180C9  CALL 008EADA0
```

The AI spends only the inventory of its own slot. `00A17A2F` fills its candidate vector with
`008EA0C0(brain+20h)`, which lists that slot's items whose cooldown is past, and it returns when
the vector is empty.

## The filter `008E4680` (node+8h, unit)

It returns true when any of these holds:
- **The unit itself:** record+0Ch is a unit id (short). The unit is live (+5Ch set, and +5Dh,
  +60h and +5Eh clear) and `unit->vtable[140h]()+174h` equals it. This is PUTT_FRIENDUNIT or
  PUTT_ENEMYUNIT.
- **A party:** record+8h < 2 and equals unit+54h. This is PUTT_PARTY.
- **A player slot:** record+10h != -1 and equals `BSP_Unit_LossCountingSlot(unit)`. This is
  PUTT_PLAYER.
- **A range:** record+0Eh names a live centre unit of the unit's party, and the unit is within the
  radius the class gives through `centre->vtable[64h]()->vtable[0]()`. This is PUTT_RANGE.

## Every category the product is queried with

This is a census of all 38 calls to `008E6430` and to its gated wrapper `00470440`. The category
is the last push before the call (`local\cc9-ships2-prodcat.py`). Every direct site first tests
the list size at `[00F88C30]+88h + category x 0Ch` and substitutes 1.0f when it is zero.

| category (powerupclasses.lua `PUET_`) | call sites |
| --- | --- |
| 1 FIREPOWER | `0047057F`, `0047062F` (HitRecord hull and part damage) |
| 2 ARMOR | `0087784F`, `00877875`, `00877965`, `008779A3` |
| 3 SHIP_REPAIR | `0082791B`, `0093C16A`, `0093C25A`, `0093C592`, `0093C7C5`, `0093C8C5`; HUD `006496A1`, `006496B7` through `00470440` |
| 4 SHIP_SPEED | `0080FC4F`, `00824252`, `00824538`, `00824736`, `00826A21` |
| 5 SHIP_TURNFACTOR | `008118BA`, `00811A59` (the yaw rate, docs/SHIP_TURN_RATE.md) |
| 6 PLANE_SPEED | `007DB7DB` |
| 7 TARGETING_ERROR | `00730619` through `00470440` |
| 8 DEVICE_RELOADING | `0072CF4E` |
| 9 TURBO_RELOADING | `007CEBCC` |
| 10 CAPTURE_POWER | `006F69B9`, `006F6A13`, `006F6B4C`, `006F6BA3`, `006F6CFA` |
| 11 CAPTURE_RESISTANCE | `006F6D90` |
| 12 RECON | `008043D5`, `0080466E`, `0080491E` |
| 13 PLANE_TURN | `007DA067` |
| 14 TORPEDO_SPEED | `008576A4` |
| 15 TORPEDO_TURN | `008570B8` |

`0047045E` is the wrapper's own tail jump. Category 0 is never queried.

## Single player: who gets items, from this installation's Lua

**The data tables** are `datatables/powerupclasses.lua` and `datatables/poweruplib.lua`, both
2024-07-13 08:26.
- There are three groups: `PUT_AIRSUPPORT`, `PUT_ACTIVE` and `PUT_PASSIVE`.
- The `PASSIVE` group is repair_bay, advanced_repair_tools, advanced_recon_devices,
  veteran_crewmen, advanced_planes, advanced_weapons, face_hardened_armour, veteran_pilots,
  heavy_assault_troops and advanced_torpedoes.
- The use routine above treats a passive item like an active one. It needs a use.

**The scripts behind the measured missions** (log names mapped from the `missions/*.lua` load
line):

| mission | script | mtime | grants |
| --- | --- | --- | --- |
| USN02 | `missions/usn/usn_2_java.lua` | 2024-07-13 08:26 | none. `luaAddPowerup` is defined at :969 and never called |
| USN01 | `missions/usn/usn_1_marshall.lua` | 2024-07-13 08:26 | none. It is defined at :1159 and never called |
| USN04 | `missions/usn/usn_19_coralus.lua` | 2024-08-26 16:10 | `radar_sweep` at :625, once the secondary objective (the Shoho escorts dead) completes; `full_throttle` at :804, once the hidden objective (the transports dead) completes |
| E2 | `missions/bsm/bsm_01_stationed_at_pearl.lua` | 2024-07-13 08:26 | none |
| LOMP06 | `missions/usn/LOMP/06_crucial_cargo.lua` | 2024-07-13 08:26 | none |
| USN13 | `missions/usn/usn_13_truk.lua` | 2024-08-13 09:32 | none |
| JM06, JM08 | `ijn_06_prelude_to_midway.lua`, `prcpijn_08_defend_guadalcanal.lua` | not loose files in this installation | not readable. The logs record no power-up native call |

`global/commandhelpers.lua` (2024-10-29 12:54) also calls `AddPowerup` when restoring a
checkpoint, at :16591, from the `GetAvailablePowerups(PLAYER_1)` snapshot at :16370. The
harness never loads a checkpoint.

**Both USN04 grants are one-argument calls,** so they go to the local player's inventory. Neither
touches the turn factor:
- `radar_sweep` is `PUET_RECON` = 100, `PUTT_PARTY`, for 10 s.
- `full_throttle` is `PUET_TURBO_RELOADING` = 4 for planes, for 60 s.

Their effect needs a use by the local player (the idle harness never presses one), or by the AI
brain whose party slot is that player's. This packet does not establish whether the human's slot
also has an enabled AI brain in single player.

## Run-time evidence

The mission Lua host binds all three natives (`src/mission_lua_host.cpp:553-555`). An
unimplemented native prints a `MissionLuaNative::<name> ... calls=N` row. None of the host logs in
this tree has a row for any power-up native: 40 USN04 runs, 36 USN02, 15 USN01, 13 E2, 8 JM06,
7 LOMP06, 4 JM08 and single USN13 and PRCP runs. So no grant has ever happened here, and no list
can be non-empty.

## Verdict

- **Closed for every measured mission:** the modifier lists are empty in the image as in the host.
  The 1.0 the host returns at each product site is the image's value.
- **When a future USN04 run completes the Shoho-escort or transport objective**, `AddPowerup`
  will be called and should then be bound, as a grant into the inventory. The list effect still
  needs a use, and the two classes change only recon (category 12) and plane turbo reloading
  (category 9).
- **Not claimed:** the AI brain use rule itself (`00A179E0` from `00A17FD0` to `00A180C9`, and
  `008E35F0`), and whether the local player's party slot runs an AI brain in single player.
