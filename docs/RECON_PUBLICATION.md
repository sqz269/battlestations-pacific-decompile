# The recon publication: `recon[party][relation][category]` (packet `cc9_recon_publication`)

Worker cc9-lua22. The queue item is SQUADRON_LAND_TASK 5aj item 1; the finding is SHIP_AI_OPEN_ITEMS
74.4. Switch: `kReconPublishBound` (`include/bsp/game_hosts_lua.hpp`).

## 1. What the image does (both routines read whole)

**`00806B10` `BSP_Recon_PublishSlotTable`**, `__thiscall void(ReconSlot*, LuaInstance*)`, `RET 4`,
body `00806B10`-`00806CCD`. The only caller is `008079B0`, after a slot's rebuild, and only when
the slot's `+25h` byte is set. The pseudocode and docs/RECON_SLOT_LISTS.md section 4 agree. The
ledger's "read to `00806BAE`" is stale: cc2_recon_slot_lists read the rest.

1. Pass 1 (`00806B30`..`00806B9D`): `recon[slot+28h]` through `006B8190` / `00803750`. Then
   `enemy`, `own`, `neutral` and `unknown` are set to nil through `006B8390` (pushstring, pushnil,
   `lua_settable(-3)`).
2. Pass 2 (`00806BA2`..`00806CB7`): each relation is created through `008037D0` (a rawget; when
   the value is nil, a new table is rawset) and filled by `00805D90`:
   - enemy from triple 1 (`+DE4h`);
   - own from triple 0 (`+DD8h`);
   - neutral from triple 2 (`+DF0h`);
   - unknown from triple 3 (`+DFCh`).

**`00805D90` `BSP_Recon_PublishRelationCategories`**, `__thiscall void(LuaInstance*, List*, int)`,
`RET 0Ch`, body `00805D90`-`00805F29`. The listing was checked at `00805DE0`..`00805F0A`.
1. It builds nineteen local `{count, head, tail}` lists.
2. It walks the triple (`node+4` is next, `node+8` is the record, `record+4` is the unit):
   - it skips unit `+C4h` classes `0Fh`, `10h`-`12h`, `14h`-`17h`, `13h` and `19h`;
   - it takes the category `[unit+170h]->vtable[0]()`, and skips `13h`;
   - it pushes the record into list `category` (`LEA ECX,[EAX+EAX*2]` then `[ESP+ECX*4+40h]` at
     `00805E3C`).
3. For each name of `00E0B590` (`mothership` .. `path`), in table order, it calls
   `008037D0(name)`. This creates the table even when the bucket is empty. Then, for each record
   in bucket order, it does
   `table[itoa(u16 unit+174h)] = thisTable[itoa(u16 unit+174h)]`:
   - `00927BF0` is `__thiscall(unit)(LuaInstance*)`: ECX is the unit, loaded at `00805EAD`. It does
     getglobal `thisTable`, pushstring, `gettable(-2)` and `remove(-2)`;
   - `006B84D0(-3)` is `lua_settable`.
4. The third argument is never read.

**The category** (`[unit+170h]` vtable slot 0). `local\l22_cat.py` collects every
`MOV [reg+170h], imm32` in `.text`. `local\l22_pair.py` then checks, for each class, that the same
constructor stores the primary vtable of docs/ENTITY_CLASS_IDS.md within `60h` bytes, and decodes
slot 0.

| category | classes (slot-0 function) |
| --- | --- |
| 0 mothership | 09h (`007581F0`) |
| 1 destroyer | 07h (`006FE4D0`) |
| 2 torpedoboat | 0Eh (`00857D60`) |
| 3 battleship | 0Dh (`006DFDC0`) |
| 4 cruiser | 0Ah (`006FB370`) |
| 5 cargo | 0Bh (`006EB1D0`) |
| 6 landingship | 0Ch (`0074BBC0`) |
| 7-12 | the plane leaves 10h, 12h, 11h, 13h, 14h/15h/16h, 17h (all excluded from the publish) |
| 13 submarine | 08h (`00852FB0`) |
| 14 landvehicle | 1Ah LandConvoy (`004F24E0`), 19h (excluded) |
| 15 landfort | 1Bh (`00745A40`), 1Ch (`006F5830`) |
| 16 airfield | 45h (`006D1D20`) |
| 17 shipyard | 46h (`00846B40`) |
| 18 path | 47h (`0047B790`) |
| 13h, none | 06h, 2Bh, 34h, 35h (`004E63F0` / `00700200`) |

Two classes return a field instead of a constant:
- **0Fh, the plane base** (`007D0350`, `unit+AACh`). It is excluded anyway.
- **18h, the squadron** (`007EFA90`, `unit+354h`). The constructor stores `13h` (`007F2DF6`). Then
  `007F4BA0` (the init slot A4h) overwrites it with the slot-0 member's (`+3D0h`) category, at
  `007F4BE8` and again at `007F5438`. The displacement scan (`local\l22_disp354.txt`) finds no
  other writer in the squadron's range.

## 2. The host binding

`GameMissionLuaHost::publish_recon_slot_tables_00806b10` (`src/game_hosts_lua.cpp`). It runs once
per recon pass generation, after the recon listeners, when `kReconPublishBound` is set. The class
map is `bsp::recon_publish_category_for_class` (`src/recon_slot_lists.cpp`). The Lua operations
reuse `recon_values.cpp`'s `006B8190` / `00803750` / `008037D0` helpers, in the image's order.

**Labelled substitutions:**
- **Cadence:** publication happens at the mission frame after the host's recon pass, not inside
  `008079B0`.
- **The `+25h` dirty byte:** the image sets it through `0077B0C0` on any level change the slot
  sees, and through `00803BA0` on a death it saw. The host instead publishes a party when the
  (relation, category, id) sequence differs from its last publication. An unchanged sequence
  gives the same tables, so the difference is confined to stale entries. If a unit left without
  a level change or death, the image would keep its entry and the host drops it.
- **The id:** the `+174h` id is the host's entity id (unit index + 1), which is also the
  `thisTable` key.
- **A squadron's `+354h`:** its first resolved member's class category (`member_units[0]`, else
  the first departed unit), frozen at first resolution. An unresolved squadron keeps `13h` and is
  not published.

## 3. Predictions (written before any run)

1. **Mechanism.** With the switch ON, the summary line `recon publication` shows `passes`,
   `slots` and `entries` above 0 on every row that has units. OFF shows `bound=0` and zeros, and
   the rows are identical to main.
2. **JM08 36200/36000 with `BSP_ORIGIN_DIAG=1`:** from the first sample on, `recon[0].own` has
   non-zero entries (destroyer, cruiser, landingship, cargo), and `around` becomes non-zero once
   an Allied ship is within 300 m of the origin. SHIP_AI 74.4 says this happens by t = 720.8 s,
   and possibly from t = 540.6 s (Grayson at 248.6 m). The invasion should therefore start near
   mission frame 10800-14400. This installation's `jm08.lua` (mtime 2024-07-13) also reads
   `recon[PARTY_ALLIED].own.landingship` in `luaJM8CheckUSNFleet`, and `recon[PARTY_JAPANESE].own.*`
   (landfort, torpedoboat, reconplane, fighter, levelbomber) in six more places. Those loops were
   empty and now run.
3. **Other rows.** `commandhelpers.lua` (mtime 2024-10-29) reads `recon[...]` in
   `luaGetNearestEnemy`, `luaGetNearestEnemyNot`, `luaGetShipsAround*`, `luaGetPlanesAround*`,
   `luaGetVisibleEnemies`, `luaGetOwnPlanes`, `luaIsVisible*` and more. Any mission script or
   script think that calls them changed answer.
   - **Predicted moved (exit 3):** JM08, and every row whose mission script calls these helpers or
     reads `recon` directly: USN04/E2, USN01, JM05/JM05 long, JM06, BSM01 and USN12.
   - **Predicted identical or gameplay-identical:** rows whose scripts never reach a recon read.
     These are listed from the logs once run, not assumed.
4. **Failure modes that would keep it OFF:** a Lua error from a helper that now iterates real
   entities (the `first_error` line), a stack imbalance (the host's `lua_gettop` is restored, so
   it would surface as an error), or no publication on a row that has units.

## 4. The pairs, and the flip

Setup:
- Same tree, commit `54162190a`. `local\l22_off` (SHA-256 `FAACD01BA8E1`) is a clean export;
  `local\l22_on` (`BA840A4C77F6`) is `--flip kReconPublishBound=true`.
- The launch form is the reference rows' (`local\l22_runs.ps1`, copied from cc9-gunnery17 and
  cc9-ships20), with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
- JM08 36200/36000 was also run with `BSP_ORIGIN_DIAG=1` on both sides.
- Runs ended by 2026-09-30 06:30 UTC.

| row | `pair_diff` | ON passes / publishes / entries |
| --- | --- | --- |
| USN04, E2, USN01, JM06, JM08 3000, USN13, BSM01, LOMP06, LOMP10, USN12, LOMP10 long, USNOS, USNOS long, IJN01, USN13 long | exit 1 (gameplay identical) | e.g. USN04 75 / 12 / 154, USN13 50 / 42 / 10061 |
| **JM08 36000** | exit 3 | 600 / 91 / 31871 |
| **USN02** 9000 | exit 3 | 150 / 11 / 274 |
| **JM05** 3000 and **JM05 long** | exit 3 | 50 / 35 / 7690; 150 / 75 / 18001 |

**Prediction 1 (the mechanism) held.**
- Every ON row publishes. `no_category` is 0 everywhere. OFF is `bound=0`.
- No row's error-line count changes.

**Prediction 2 (JM08) held.**
- `recon[0].own` has 36 entries from t = 0.1 s (OFF: 0 at every sample).
- `around` is 1 at t = 720.8 s (`USTroopTransport 05` at 19.2 m). OFF never gets there.
- This installation's `PRCPJM08.lua` (the script the row loads, mtime 2024-08-26) polls
  `CheckInvasion` once a second, and it fires. `StartInvasion` issues the 21 `NavigatorAttackMove`
  orders onto `Headquarter 01` and the six `NavigatorMoveToPos` orders onto `USNLandingNavpoint
  01`-`06`. The first comes at mission frame 14005, inside the predicted 10800-14400.
- `CheckAP*` then re-issues AttackMove for transports 01, 04 and 02.
- **Death table:** 18 -> 18 rows. TroopTransports 01, 02, 04 and LST 03 no longer die; kontener
  01, 02, 03 and 05 now die; 10 rows change time or killer. The controlled Auilick moves
  4565 -> 10504 m.
- This row is now the reaching row for SHIP_AI 73's pad model (cc9-ships20).

**Prediction 3 (which rows move) partly missed.** The mechanism matches, so the switch flips; the
miss is recorded here.
- **Predicted to move but identical:** USN04/E2, USN01, JM06, BSM01 and USN12. Their scripts
  read recon only in branches these 3000-9000-frame rows do not reach. `local\l22_scripts.py`
  lists each row's script and its reads: USN01 `usn_1_marshall.lua` and USN12 `usn_12_augusta.lua`
  have none.
- **Moved, not individually predicted:**
  - **USN02:** 14 -> 1 deaths. Its `usn_2_java.lua` has no recon read. The mission fails at
    29.75 s on both sides, and `luaInitMissionEnd` (`commandhelpers.lua` 13643) then calls
    `SetInvincible(value, 0.1)` on every unit of `luaGetOwnUnits(nil, party)` for all three
    parties. With the maps empty, only the scripted ten were protected; now all 37 are. So the
    thirteen post-failure sinkings stop (Alden, John1-3 and others stop at their floor health),
    and hit records double.
    This is script behaviour after mission end, not combat.
  - **JM05 (3000 and 9000):** the script's recon reads now answer. `SetFireTarget` and
    `NavigatorAttackMove` run once, on `Clemson class 1930 #1.1` (range 1600 -> 1852). Deaths,
    hits and damage are identical.

**Decision: ON** (`kReconPublishBound = true`).

## 5. Why USN02 fails at 29.75 s with an idle player (the lead's check)

**Verdict: this is authored behaviour of this installation, not a host gap.** The recon
publication does not cause it: the failure time is 29.75 s on both sides of the pair.

The trigger:
- The row loads `Scripts/missions/USN/usn_2_java.lua` (mtime 2024-07-13).
- `luaCheckObjectives` (line 493) calls `luaMissionFailed()` as soon as
  `Mission.Houston.Dead or Mission.Exeter.Dead` (line 521), in any phase above 0.
- `luaMissionFailed` (line 821) ends through `luaMissionFailedNew(<random live ally>, "Game Over")`.
  That call is where the log's `entity="Alden"` comes from.

What sinks Houston, from `local\l22_off_usn02.log`:
- The IJN destroyers start about 2.4-2.7 km from Houston's column. Their formation leader
  Yamakaze is at (3500, -4500); Houston is at (1200, -6000).
- The IJN destroyers launch torpedoes from t = 1.45 s. Yamakaze's first launch is at t = 1.55 s
  (`gunnery: torpedo launch`).
- The torpedoes are the Type 93, bullet class 67. Its `WaterTravelSpeed` is 170.444 in this
  installation's arcade `bulletclasses.lua`, whose mtime is 2026-05-09, so it is locally modified.
  The host reads that table the way the image does (docs/CLASSTABLE_SELECTION.md section 3; the
  realistic table has 51.444).
- Two Type 93 blasts hit Houston at 19.25 s and 20.85 s (4075.9 and 1891.2 damage against
  6500 HP). The death row gives killer Yamakaze, range 2438 m. The next objective check fails the
  mission.
- The script protects only the player's `DRGrp` (`SetInvincible(unit, 0.1)`, line 230). The
  Houston group (line 246, Houston, Alden and John1-3) gets no protection, so an idle player
  cannot save Houston.

Remaining uncertainty:
- The launch timing (t = 1.45 s) and the steering to the lead point are the host's torpedo
  chain, landed and paired elsewhere (TORPEDO_FRIENDLY_CROSSING, CLASSTABLE_SELECTION section 5).
  They are not validated against the original game, which is never run here.
- An early loss is consistent with the modded 170 m/s arcade torpedo and a scene that starts the
  two forces inside torpedo range. No unit, objective or query answers wrongly: Houston and
  Exeter resolve (`entity_resolves=36`), and `luaCheckObjectives` reads only `.Dead`.
