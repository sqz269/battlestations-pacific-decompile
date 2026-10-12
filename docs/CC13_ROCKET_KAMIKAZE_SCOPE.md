# Scope of the plane rocket and kamikaze tasks in this installation (cc13-audit10, part 3)

Addresses: 0099A170 007EEC50 007EE8F0 007EEB36 007EEBCC 007ED880 007ED970 007B93E0 007B9480 008A4F00 008A50D0
007B7FD0 007B6A40 007B6830 007B7AF0 007B4980 009AF720 009AEEF0 009AED20 009AF480 009AEBE0 009ADBF0 009AD720 009AE1B0
009AB740 00819A20 0080A000 006FF170 008C3880

Packet `cc13-audit10` (scoping read), branch `agent/cc13-audit10`, 2026-10-12. cc9 requested it through the cc13
lead. **Read only.** It reads this installation (`I:\SteamLibrary\steamapps\common\Battlestations Pacific`, modded,
with mtimes cited) and the repository. It runs no scripts and writes nothing to Ghidra or the ledgers.

**The question.** Which missions use (1) the plane **rocket** task and (2) **kamikaze**: the kamikaze task and the
drop-kamikaze task of the Ohka/Funryu carriers? For each: which units and classes, whether the feature is on the
mission's critical path, the image entry points, and a rough size of each reconstruction. The trigger is
docs/CC13_RNG_STREAM_CENSUS.md section 11, which lists them as HOST-OMITS-FUNCTION: the rocket task (25 draws) and
kamikaze (8).

**Tools** (all in `local/`, each reading only the installation or the repository):
- `au10_rk_classes.py` and `au10_eq.py`: the class tables (bullet, device and vehicle classes, equipment sets);
- `au10_scn.py`: every `universe/scenes/missions/**.scn` except `multi/`;
- `au10_join.py`: the mission tree, the scenes and the stage scripts;
- `au10_table.py`: the table in section 4;
- `au10_size.py`: a call/vtable closure over `local/bsp_index.sqlite`, sized from the PE.

## 1. How a plane gets these tasks (image)

The attack class is chosen by `007EEC50` (`BSP_Unit_ChooseAttackCommand`; docs/ATTACK_COMMANDS.md). Its ordnance
pass tries `levelbomb, dropkamikaze, divebomb, torpedo, rocket, kamikaze, depthcharge` and only runs for a surface
target. `007EE8F0`'s arms decide applicability:

| class | precondition (image) | task factory (`0099A170`) | task | constructor / approach ctor | vtables |
| --- | --- | --- | --- | --- | --- |
| `rocket` `00E08F48` | ordnance kind `33h` on a weapon slot with descriptor `+E0h > 0` (`007ED970` -> `007B9480`); target `IsKindOf(2)` and not `IsKindOf(18h)` | `007B7FD0` | `708h`, kind `Bh` | `007B6A40` / `007B6830` | `00D05918`, `00D05910`, `00D0590C` |
| `kamikaze` `00E08F50` | the unit's class `IsKindOf(17h)`; small-ship gate `007EEB64..007EEB8A` (`kKamikazeShipBlockedBound`) | `009AF720` | `6A0h`, kind `Ch` | `009AEEF0` / `009AED20` | `00D1FD40`, `00D1FD3C`, `00D1FD38` |
| `dropkamikaze` `00E08F30` | ordnance kind `2Fh` (`007ED880` -> `007B93E0`): a `DummyKamikazePlane` bullet | `009AEBE0` | `74Ch`, kind `5` | `009ADBF0` / `009AD720` | `00D1FC70`, `00D1FC68`, `00D1FC64` |

**Script entry points.**
- `PilotSetTarget` runs `007EEC50`, so it gives any of the three classes.
- `PilotBomb` (`008A4F00`) gives `dropkamikaze` when `007ED880` holds.
- `PilotGunFire` (`008A50D0`) gives `kamikaze` for a `17h` unit against a non-aircraft target.
- The same chooser serves the AI's own attack orders: carrier air operations and a scene `CommandTarget`.
- So a rocket-armed squadron takes the rocket task whenever anything orders it against a surface target.
- A kamikaze plane takes the kamikaze task whenever it is ordered to attack at all.

**What the host does today.**
- The chooser and the factory call are hosted: `src/attack_commands.cpp` and `src/game_hosts_script_orders.cpp`'s
  `PilotSetTarget` path, which logs `PilotSetTarget task: 0099A170 -> n`.
- `src/game_hosts_units.cpp` has no `*_task_installed` flag, state machine or tick for any of the three classes.
  The flags that exist are torpedo, closetoship, dive bomb, level bomb, strafe, dogfight, moveto, takeoff, land and
  retreat. So an aircraft given one of these classes has a task record and no tick.
- Hosted pieces around the tasks:
  - the rocket's ignition and thrust `0080A000` (`include/bsp/bomb_torpedo_tick.hpp`);
  - the kamikaze impact `00819A20` (`kamikaze_detonate_00819a20`, `src/game_hosts_gunnery.cpp`);
  - the carried-kamikaze class read `006FF170` (AI target weights);
  - the kamikaze **boat** ship-AI states (`009DB320`, `009E2020`). Boats are a separate feature, not part of this
    scope.

## 2. Rocket- and kamikaze-capable classes (this installation)

Sources:
- `scripts/datatables/autoload/vehicleclasses.lua`, mtime 2026-05-09;
- `classtables/arcade/bulletclasses.lua` and `deviceclasses.lua`, both mtime 2026-05-09;
- `classtables/realistic/bulletclasses.lua` (2025-06-02) and `deviceclasses.lua` (2024-07-13);
- the scene enum `universe/library/global.enums`, mtime 2024-10-29.

**Rocket ammunition.** Bullets of `Type "Rocket"`:
- 6, Tiny Tim;
- 8, 5" HVAR;
- 11, R4M AA;
- 51, HVAR "Small DLC";
- 116, R4M AA. This one exists in the **arcade** table only.

The **arcade** device 383 "Bomb platform 800kg JP" also fires bullet 116. That looks like a mod edit. It is not
carried by any class listed below.

| enum (scene) | VehicleClass | rocket platform (device: bullet) | in equipment set | DefaultEquipment |
| --- | --- | --- | --- | --- |
| `AvengerTinyTim` | 16 TBM Avenger | 22 Tiny Tim (6); 128 HVAR (8) | 1; 2 (set 3 is not rockets) | 1 |
| `Corsair` | 102 F4U Corsair | 127 HVAR (8) | 1 (the only set) | 1 |
| `Raiden` | 151 J2M Raiden | 125 R4M (11) | 1 (the only set) | 1 |
| `Kikka` | 183 Kikka | 126 R4M (11) | 2 | 0 |
| `Shinden` | 189 J7W1 Shinden | 125 R4M (11) | 2 | 2 |
| `A8M` | 312 A8M Rikufu | 416 Tiny Tim (116, arcade) | 2 | 0 |
| `Mustang` | 392 P-51 Mustang | 149 HVAR DLC (51) | 2 | 0 |

- The Hellcat's rocket device (262) is carried by no class here. `Hellcat` (VehicleClass 26) has only guns.
- `Avenger` (enum 113, TBF) has no rockets. Only `AvengerTinyTim` (16) does.
- **Every Corsair and every Raiden is rocket-armed.** That makes the rocket task the commonest of the three.
- The R4M is an air-to-air rocket, but the ordnance pass never runs against an airborne target. An R4M-armed
  Raiden, Kikka or Shinden ordered against a ship therefore takes the rocket task.

**Kamikaze.**
- `Type "Kamikaze"` classes, which answer kind `17h`: 45 Kamikaze Zero, 46 Kamikaze Val, 100 Kamikaze Oscar, 103
  Kamikaze Judy, and the released bodies 156 MXY7 Ohka and 370 Funryu. The enum's `KamikazeZero_light` (203) and
  `KamikazeVal_light` (204) have **no** `VehicleClass` entry in this installation. Two of them appear in
  `COTP-USN/usn_09_leyte.scn`, which is not in the mission tree. They were not traced further.
- **Drop-kamikaze carriers:** 32 `BettyOhka` (device 143 "Kamikaze platform", bullet 143 `DummyKamikazePlane` with
  `KamikazePlaneClass` 156) and 811 `FunryuCarrier` (device 531, bullet 170 `DummyKamikazePlane`,
  `KamikazePlaneClass` 370).
- The released body becomes a class-156 or class-370 unit. `008C3880` `BSP_LuaBinding_GetKamikazeByDummy` maps it.
- **Not traced here:** whether the release then installs a `kamikaze` task on that unit. If it does, every
  drop-kamikaze mission needs both tasks.
- Every aircraft class also carries a `KamikazeBulletClass` (99, 21 or 98): its crash impact. That is the
  detonation, not the task.

## 3. Missions: the ones on the team's run list

"Critical path" means the mission's objectives can only be reached, or only fail as they should, if the feature runs.
"Incidental" means the units exist but no objective depends on their behaviour. These calls rest on the stage
script's `luaObj_*` calls and the tree's objective text.

| mission (tree comment / id) | feature | units | critical path? |
| --- | --- | --- | --- |
| **USN Finale / `USNOS`** (`us_osumi.scn` 2024-10-29, `us_osumi.lua` 2024-10-29) | drop-kamikaze | 9 `BettyOhka` in the scene. `luaFuntime()` (line 1795, called at 1731 in the finale) generates `Betty1`..`Betty6` (7-9 are commented out), adds them to `Mission.FinalTrgs` and orders each with `PilotSetTarget` against `Mission.TilmanGrp` (lines 474-487: NH, Portland1/2, six troop transports, Gear13-16) | **Yes, as targets and as the threat.** `luaObj_Add("primary", 5, Mission.FinalTrgs)` (line 1760) makes the six Bettys part of the final primary objective, alongside Yamato, Musashi and Haku1-4. Without the task they never attack the Tilman group; with it, their Ohkas do. The objective can be completed either way; the losses it is completed with cannot. |
| **USN18 Battle off Cape Engano / `USN17`** (`usn_17_engano.lua` 2024-07-13) | kamikaze | spawned in phase 3: `SpawnNew` 3 x class 45 and 6 x class 103 (`luaKamikaze`, line 1188), re-spawned while none is alive (lines 932-944), each `PilotSetTarget` on a random `Mission.RemainingUSN` ship | **Yes (the threat).** Primary 3 is "Escape the Kamikazes!" (`luaObj_Add("primary", 3, GetPosition(Mission.Escape))`, line 1184). It completes on reaching the point. The kamikazes are what the escape is from, and without a task they never close. |
| **USN19 Battle of Ormoc Bay / `USN22`** (`usn_ormoc.lua` 2024-10-29) | kamikaze (spawned); rocket (1 `AvengerTinyTim`, USA) | `luaSpawnKamikazes()` (line 1384): three `luaPickRnd(Mission.PlaneClasses)`, which is `{45, 103}` (line 414), plus one class-167 plain G4M Betty | **Secondary only.** Secondary 2, "Defend USS Ward and USS Mahan from kamikazes!" (`luaObj_Add("secondary", 2, Mission.SecDDs)`, line 1226), fails at 749 and completes at 787. The single Tiny Tim Avenger is incidental. |
| **USN21 Battle of Okinawa (PRCP) / `USN19`** (`prcp_14_okinawa.lua` 2024-08-26) | kamikaze, drop-kamikaze; rocket (2 `Shinden`, Japan) | `SpawnNew` 4 x 100, 5 x 45, 4 x 46 and 1 x 32; the player owns Corsairs | **Not established.** Primaries 1-3 are a phase, the HQs and Yamato. The kamikaze waves press the fleet. Read the phase scripting before relying on this. |
| **JM08 Defense of Guadalcanal (PRCP) / `JM08`** (`PRCPJM08.lua` 2024-08-26) | drop-kamikaze (1 `BettyOhka`, Japan); rocket (1 `Kikka`, 1 `Shinden`, Japan) | Japanese side, the player's own side in this campaign | **Incidental, presumably** (single units). Not read further. |
| **USN13 Raid on Truk / `USN13`** (`usn_13_truk.lua` 2024-08-13) | rocket (1 `Raiden`, Japan) | one squadron | Incidental. |
| **USN15 Battle of the Sibuyan Sea / `USN21`** (`usn_sibuyan.lua` 2024-10-29) | kamikaze | `Kamikaze1`..`Kamikaze11`, generated at lines 884-895 | **Yes.** Primary 4 is "Destroy the kamikaze wave!" (`luaRemoveDeadsFromTable(Mission.KamikazeWave)`, line 619). The wave dies by the player's guns or by striking its targets. Without the task the second route never happens, so the objective's pacing and the fleet's losses change. |

**Missions with nothing:** `USN04`, `USNRM01`, `USN01`, `BSM01`, `JM06`, `LOMP10` and `IJN01` have neither feature,
in their scene or as `SpawnNew` in their stage script. They do not appear in section 4.

## 4. All mission-tree entries with a feature unit (generated)

Columns:
- **rocket units in scene:** the class's rocket set is the one in use (the entity's `Equipment`, else
  `DefaultEquipment`). Rows where the class is present but not rocket-armed are dropped.
- **race:** `?` when the entity has no `Race` property.
- **CT:** the entity has a scene `CommandTarget`, so it attacks from the start.
- **script spawns:** `["Type"] = n` occurrences in the stage script, which are `SpawnNew` definitions.
- **player classes:** a feature class in the mission tree's `allunitsid` (owned) or `changeables` (swap) for that
  side. A player-owned Corsair or Tiny Tim Avenger takes the rocket task whenever its AI flight is ordered against
  a ship.

Entries whose only link is `PrepareClass` are omitted. PrepareClass only preloads; the units then come from a hangar
or a spawn with a computed type, which this read does not resolve. There are 17 such entries, mostly Breeze's IJN
campaign `PrepareClass(102)`.

| mission (tree comment / id) | scene | rocket units in scene | kamikaze / drop-kamikaze units in scene | script spawns of feature classes | player classes | objective text naming the feature | script (mtime) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| IJN25 Battle of the Bismarck Sea / ABYS01 | `IJN/aby_01_elim.scn` | - | 16 FunryuCarrier (Japan) | - | - | - | `scripts/missions/Ijn/aby_01_elim.lua` (2025-06-02) |
| IJN10 Port Moresby / IJN10 | `IJN/ijn_10_port.scn` | 2 Raiden (?) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_10_port.lua` (2024-07-13) |
| IJN13 Hunt for the Yorktown / IJN13 | `IJN/ijn_13_yorktown.scn` | - | 3 FunryuCarrier (Japan) | - | - | - | `scripts/missions/Ijn/ijn_13_yorktown.lua` (2024-10-29) |
| IJN14 Prelude to Midway / IJN14 | `IJN/real_prelude.scn` | 3 Corsair (?) HVAR; 5 Raiden (Japan) R4M; 2 Raiden (Japan, CT) R4M | - | {Corsair: 2} | - | - | `scripts/missions/Ijn/real_prelude.lua` (2024-07-13) |
| ALT02 Alt/Challenge Prelude to Midway / ALT02 | `IJN/ijn_14_prelude_mid.scn` | 8 Corsair (?) HVAR; 5 Raiden (Japan) R4M; 2 Raiden (Japan, CT) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_14_prelude_mid.lua` (2024-07-13) |
| IJN17 Guadalcanal / IJN17 | `IJN/ijn_17_gaud.scn` | 2 Raiden (?) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_17_gaud.lua` (2024-08-09) |
| IJN19 Second Battle of Coral Sea / IJN19 | `IJN/ijn_19_coral_two.scn` | 1 Raiden (Japan) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_19_coral_two.lua` (2024-10-29) |
| IJN20 Seizing the Fijis / IJN20 | `IJN/ijn_20_fiji.scn` | 2 Corsair (USA) HVAR; 1 Shinden (Japan, CT) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_20_fiji.lua` (2024-10-29) |
| IJN22 Saving the Zuiryu / IJN22 | `IJN/ijn_22_zuiryu.scn` | - | - | {Mustang: 1, Corsair: 2} | - | - | `scripts/missions/Ijn/ijn_22_zuiryu.lua` (2024-10-29) |
| IJN23 Defense of the Fijis / IJN23 | `IJN/ijn_23_def_fiji.scn` | 1 Corsair (?) HVAR; 2 Corsair (Japan) HVAR; 4 Corsair (Japan, CT) HVAR | - | - | - | - | `scripts/missions/Ijn/ijn_23_def_fiji.lua` (2024-10-29) |
| IJN25 Battle of the Bismarck Sea / IJN25 | `IJN/ijn_26_phil_sea.scn` | - | 16 FunryuCarrier (Japan) | - | - | - | `scripts/missions/Ijn/ijn_26_phil_sea.lua` (2025-06-02) |
| IJN26 Kantai Kessen / IJN26 | `IJN/ijn_27_kantai.scn` | - | - | {AvengerTinyTim: 4} | - | - | `scripts/missions/Ijn/ijn_27_kantai.lua` (2024-07-13) |
| IJN27 Prelude to Hawaii / IJN27 | `COTP-IJN/prelude.scn` | - | 6 FunryuCarrier (Japan) | - | - | - | `scripts/missions/COTP-IJN/prelude.lua` (2024-10-29) |
| IJN28 Battle of Mamala Bay / IJN28 | `IJN/ijn_28_prelude.scn` | - | - | {AvengerTinyTim: 3} | - | - | `scripts/missions/Ijn/ijn_28_prelude.lua` (2024-10-29) |
| IJN29 Invasion of Hawaii / IJN29 | `IJN/ijn_29_hawaii.scn` | 3 A8M (Japan, CT) TinyTim(116); 1 Raiden (Japan) R4M | - | - | - | - | `scripts/missions/Ijn/ijn_29_hawaii.lua` (2024-10-29) |
| USN07 Battle of the Eastern Solomons (PRCP) / USN07 | `COTP-USN/PRCPUS/prcp_01_eastern.scn` | 6 Raiden (Japan) R4M | - | - | - | - | `scripts/missions/COTP-USN/PRCPUS/prcp_01_eastern.lua` (2024-08-26) |
| USN13 Raid on Truk / USN13 | `USN/usn_13_truk.scn` | 1 Raiden (Japan) R4M | - | - | - | - | `scripts/missions/USN/usn_13_truk.lua` (2024-08-13) |
| USN14 Battle of the Philippine Sea / USN14 | `USN/usn_14_phil_sea.scn` | - | - | - | allied: Corsair(own) | - | `scripts/missions/USN/usn_14_phil_sea.lua` (2024-07-13) |
| USN15 Battle of the Sibuyan Sea / USN21 | `USN/usn_sibuyan.scn` | - | 3 KamikazeJudy (?); 2 KamikazeOscar (?); 2 KamikazeVal (?); 8 KamikazeZero (?) | - | - | pri: Destroy the kamikaze wave! | `scripts/missions/USN/usn_sibuyan.lua` (2024-10-29) |
| USNEX Bull's Run / USNBR | `COTP-USN/bulls_run.scn` | 4 Corsair (?) HVAR | 2 KamikazeJudy (?); 2 KamikazeOscar (?); 2 KamikazeVal (?); 2 KamikazeZero (?) | - | - | - | `scripts/missions/COTP-USN/bulls_run.lua` (2024-10-29) |
| USN18 Battle off Cape Engano / USN17 | `usn/usn_17_engano.scn` | - | - | {KamikazeZero: 3, KamikazeJudy: 6} | allied: Corsair(own) | pri: Escape the Kamikazes! | `scripts/missions/USN/usn_17_engano.lua` (2024-07-13) |
| USN19 Battle of Ormoc Bay / USN22 | `usn/usn_ormoc.scn` | 1 AvengerTinyTim (USA) TinyTim | - | - | - | sec: Defend USS Ward and USS Mahan from kamikazes! | `scripts/missions/USN/usn_ormoc.lua` (2024-10-29) |
| USN20 Invading Iwo Jima (PRCP) / USN18 | `COTP-USN/PRCPUS/prcp_13_iwojima.scn` | 4 AvengerTinyTim (USA) TinyTim; 2 Corsair (USA) HVAR; 2 Shinden (Japan) R4M | 8 BettyOhka (Japan) | {BettyOhka: 13, KamikazeJudy: 3} | allied: AvengerTinyTim(own), Corsair(own) | - | `scripts/missions/COTP-USN/PRCPUS/prcp_13_iwojima.lua` (2024-10-29) |
| USN21 Battle of Okinawa (PRCP) / USN19 | `COTP-USN/PRCPUS/prcp_14_okinawa.scn` | 2 Shinden (Japan) R4M | - | {KamikazeOscar: 4, KamikazeZero: 5, KamikazeVal: 4, BettyOhka: 1} | allied: Corsair(own) | - | `scripts/missions/COTP-USN/PRCPUS/prcp_14_okinawa.lua` (2024-08-26) |
| USN22 Endgame at Kure (PRCP) / USN20 | `COTP-USN/PRCPUS/usn_16_endgame_at_kure.scn` | - | 10 KamikazeJudy (Japan); 10 KamikazeOscar (Japan); 10 KamikazeVal (Japan); 10 KamikazeZero (Japan) | {BettyOhka: 1, KamikazeZero: 1} | - | - | `scripts/missions/COTP-USN/PRCPUS/usn_16_endgame_at_kure.lua` (2024-10-29) |
| USN23 The Final Battle / FINAL | `COTP-USN/usn_final_battle.scn` | 6 Corsair (USA) HVAR | - | {FunryuCarrier: 6} | allied: Corsair(own) | - | `scripts/missions/COTP-USN/usn_final_battle.lua` (2024-11-02) |
| JM08 Defense of Guadalcanal (PRCP) / JM08 | `COTP-IJN/PRCPIJN/prcpijn_08_defend_guadalcanal.scn` | 1 Kikka (Japan) R4M; 1 Shinden (Japan) R4M | 1 BettyOhka (Japan) | - | - | - | `scripts/missions/COTP-IJN/PRCPIJN/PRCPJM08.lua` (2024-08-26) |
| JM12 Seizing the Fijis (PRCP) / JM12 | `COTP-IJN/PRCPIJN/ijn_11_invasion_of_fiji.scn` | 2 Corsair (USA) HVAR | - | {Mustang: 1} | - | - | `scripts/missions/COTP-IJN/PRCPIJN/JM11.lua` (2024-08-09) |
| JM16 Invasion of Hawaii (PRCP) / JM16 | `COTP-IJN/PRCPIJN/ijn_14_invasion_of_havaii.scn` | 5 AvengerTinyTim (USA) TinyTim; 14 Raiden (Japan) R4M | 8 KamikazeJudy (Japan); 8 KamikazeOscar (Japan); 8 KamikazeVal (Japan); 8 KamikazeZero (Japan) | - | - | - | `scripts/missions/COTP-IJN/PRCPIJN/JM14.lua` (2024-08-26) |
| (PRCP) COTP Tarawa / PRCP03 | `COTP-USN/PRCPUS/prcp_07_tarawa.scn` | 1 Corsair (USA) HVAR | - | - | allied: Corsair(own) | - | `scripts/missions/COTP-USN/PRCPUS/prcp_07_tarawa.lua` (2024-08-26) |
| (PRCP) COTP Leyte / PRCP05 | `COTP-USN/PRCPUS/usn_09_Leyte.scn` | 3 Corsair (USA) HVAR; 2 Corsair (USA, CT) HVAR | 5 KamikazeVal (Japan); 2 KamikazeVal (Japan, CT); 7 KamikazeZero (Japan) | {Shinden: 1} | allied: Corsair(own) | - | `scripts/missions/COTP-USN/PRCPUS/usn_09_Leyte.lua` (2024-08-26) |
| (PRCP) COTP Engano / PRCP06 | `COTP-USN/PRCPUS/prcpus_engano.scn` | 4 Raiden (Japan) R4M | - | {KamikazeZero: 4, KamikazeJudy: 4} | - | - | `scripts/missions/COTP-USN/PRCPUS/prcpus_engano.lua` (2024-08-26) |
| (PRCP) COTP Patrol / PRCP07 | `COTP-USN/PRCPUS/prcp_11_patrol.scn` | 1 Corsair (USA) HVAR | - | {KamikazeOscar: 2, Corsair: 1} | - | - | `scripts/missions/COTP-USN/PRCPUS/prcp_11_patrol.lua` (2024-08-26) |
| COTP Indochina / PRCP08 | `COTP-USN/usn_12_Indochina.scn` | 5 Corsair (USA) HVAR; 10 Raiden (Japan) R4M | - | - | allied: Corsair(own) | - | `scripts/missions/COTP-USN/usn_12_Indochina.lua` (2024-07-13) |
| USN Defense of the Aleutians / USDA | `COTP-USN/def_aleutians.scn` | 12 AvengerTinyTim (USA) TinyTim | - | - | allied: AvengerTinyTim(own), Mustang(own) | - | `scripts/missions/COTP-USN/def_aleutians.lua` (2024-10-29) |
| USN Convoy Chase / USNC2 | `COTP-USN/convoychase_two.scn` | 2 Corsair (USA) HVAR | - | - | allied: Corsair(own) | - | `scripts/missions/COTP-USN/convoychase_two.lua` (2024-07-13) |
| USN Silent Strike / USNAS | `COTP-USN/us_asw.scn` | - | 1 KamikazeJudy (?); 1 KamikazeOscar (?); 1 KamikazeVal (?); 1 KamikazeZero (?) | - | - | pri: Destroy the incoming kamikazes! | `scripts/missions/COTP-USN/us_asw.lua` (2024-10-29) |
| USN Cruiser Hunters / USNCH | `COTP-USN/convoy_chase.scn` | 1 Raiden (Japan) R4M | - | - | - | - | `scripts/missions/COTP-USN/convoy_chase.lua` (2024-07-13) |
| USN Retaking the Fijis / USNFJ | `COTP-USN/us_fiji.scn` | 2 AvengerTinyTim (USA) TinyTim; 2 Corsair (Japan) HVAR | - | - | allied: AvengerTinyTim(own) | - | `scripts/missions/COTP-USN/us_fiji.lua` (2024-07-13) |
| USN Iwo Jima / USNIJ | `COTP-USN/iwojima.scn` | 2 Shinden (Japan) R4M | 8 BettyOhka (Japan) | - | - | - | `scripts/missions/COTP-USN/iwojima.lua` (2024-10-29) |
| USN Solomons Strike / USNSS | `COTP-USN/sol_strike.scn` | 4 AvengerTinyTim (USA) TinyTim | - | - | allied: AvengerTinyTim(own) | - | `scripts/missions/COTP-USN/sol_strike.lua` (2024-07-13) |
| USN Battle of the South Pacific / USNCV | `COTP-USN/us_southpac.scn` | 15 Corsair (USA) HVAR | - | - | allied: Corsair(own) | - | `scripts/missions/COTP-USN/us_southpac.lua` (2024-10-29) |
| USN Finale / USNOS | `COTP-USN/us_osumi.scn` | - | 9 BettyOhka (?) | - | - | - | `scripts/missions/COTP-USN/us_osumi.lua` (2024-10-29) |

Reading the table:
- **Rocket** reaches 35 entries. The largest are:
  - `USN Battle of the South Pacific / USNCV`: 15 Corsairs, player-owned, with 15 `PilotSetTarget`;
  - `USN Defense of the Aleutians / USDA`: 12 Tiny Tim Avengers, player-owned;
  - `JM16 Invasion of Hawaii (PRCP)`: 14 Raidens and 5 Tiny Tim Avengers;
  - `COTP Indochina / PRCP08`: 5 Corsairs and 10 Raidens;
  - Breeze's IJN campaign, with Japanese-flown Corsairs and Raidens.
  - Where Corsairs or Tiny Tim Avengers are the player's own strike aircraft, the rocket task is the player's AI
    wingmen's attack. It is critical to any objective they are meant to carry, which this read did not check
    entry by entry.
- **Kamikaze** reaches 12 entries: USN15, USN18, USN19, USN20, USN21, USN22 (40 in the scene), Bull's Run, Silent
  Strike (primary "Destroy the incoming kamikazes!"), PRCP Leyte, PRCP Engano, PRCP Black Cats and JM16.
  - USN19's kamikazes come from `luaSpawnKamikazes`, whose types the table's spawn column does not resolve, so its
    table row shows only the Tiny Tim Avenger.
- **Drop-kamikaze** reaches 11 entries.
  - With Betty Ohka: USNOS, USN20, USN21, USN22, JM08 and Iwo Jima (`USNIJ`).
  - With Funryu carriers: IJN13, IJN27, The Final Battle, and both tree entries labelled "IJN25 Battle of the
    Bismarck Sea" (ids `IJN25` and `ABYS01`).

## 5. Size of each reconstruction

Counted with `local/au10_size.py`: the closure over calls and vtable slots from the factories, constructors and
known ticks, kept inside the feature's address window. Bytes run from each function's start to the next function's
start, so padding is included and they are upper bounds. "Window" is every function in the contiguous range
(`bsp.py range`).

| feature | entries seeded | closure | window | already reconstructed | outside callees |
| --- | --- | --- | --- | --- | --- |
| rocket task | `007B7FD0`, `007B6A40`, `007B7AF0` | 56 functions, about 12.4 KB | `007B3E00..007B8100`: 100 functions, 17.2 KB, 6 of them Land* state helpers | 5, all shared Land* helpers (`BSP_LandStateObserver*`, `BSP_LandSharedState_Destroy`, `BSP_LandMoveToState_Destroy`) | 106 |
| kamikaze + drop-kamikaze tasks | `009AF720`, `009AEEF0`, `009AEBE0`, `009ADBF0`, `009AF480`, `009AB740`, `009AE1B0`, `009AE140`, `009AF560` | 42 functions, about 9.7 KB | `009AB740..009AF830`: 83 functions, 16.6 KB (73 not trivial, CG or unwind) | 0 | 59 |

- **The rocket's largest bodies:**
  - `007B7290` (about 1.5 KB);
  - `007B4980` `BSP_BotStateRocketGoTowards_Tick_Provisional` (1.0 KB);
  - `007B6240` (0.9 KB);
  - `007B5430` (0.8 KB);
  - `007B5F10` (0.8 KB);
  - `007B4F60` (0.75 KB).
  - The 11 RNG-census functions (`007B3EA0`, `007B41E0`, `007B48E0`, `007B4D90`, `007B5300`, `007B5430`, `007B5780`,
    `007B5980`, `007B59D0`, `007B5F10`, `007B6240`) are all inside it.
- **The kamikaze family's largest bodies:**
  - `009AC190` (1.6 KB);
  - `009AC910` (1.15 KB);
  - `009AD720`, the drop task's approach constructor (0.5 KB);
  - `009AED20` and `009AF0A0`, the kamikaze approach and cruise profile (0.4 KB each).
  - The RNG-census functions `009AB850`, `009ABF60`, `009AC910` and `009AE9D0` are inside it.
- Both are task families of the same shape as the hosted torpedo and dive-bomb tasks: an approach object, three to
  five state vtables, and a per-tick arm. Each is roughly the size of the strafe or level-bomb task.
- The outside callees are the shared planner, fly-to and weapon calls (`00419010`, `00438AA0`/`00438B10`, `0064A*`
  and similar), and most of them are already hosted.
- **Rough order:**
  - rocket: 50-60 functions of real work plus the weapon-release wiring for `BombPlatform`/`MultiBombPlatform`
    rockets;
  - kamikaze plus drop-kamikaze: 40-45 functions, plus the Ohka/Funryu release, which spawns the class-156/370 unit
    from the `DummyKamikazePlane` bullet (`006EA7F0` `BSP_DummyKamikazePlaneClass_Construct`, `008C3880`). The
    release was not sized.

## 6. Caveats

- **Arcade and realistic tables.** This read uses the arcade tables for platform-to-bullet mapping. The realistic
  tables carry the same rocket devices, except arcade bullet 116 and devices 383/416.
- **Hangar loadouts.** Planes in a carrier's hangar (`MotherShipGen` entities) are counted with their entity's
  equipment. A launched squadron's actual loadout was not traced.
- **Critical-path calls** were made for the run-list missions in section 3 only, from the `luaObj_*` calls and the
  tree's objective text. The other rows of section 4 say where the units are, not whether a win depends on them.
- **The released Ohka/Funryu's task** (section 2) is open.
