# USNOS: why Haku1/Haku2 survive

Worker cc13-mission1, 2026-10-12. This is a read-only investigation for cc9. Nothing in `src/` changed and
Ghidra was not written.

Sources:
- The mission script is `scripts/missions/COTP-USN/us_osumi.lua` in this installation (mtime 2024-10-29
  12:54:22, 2396 lines). The installation is modded, so these are not retail claims.
- The helper script is `scripts/global/commandhelpers.lua` (same mtime).
- The run logs are cc9's, read with bounded greps only:
  `J:\PROG\battlestations-pacific-decompile-cc9-ships43\local\s43_p3_osjf.log` (called **p3** below) and
  `s43_p6_osnf.log` (called **p6**).

Status: exported and read only. Nothing here is reconstructed or game-validated.

## 1. Answer

The script does nothing to Haku1 or Haku2 that would protect them:
- no `SetInvincible`;
- no hide, hold or `SetSkipDeadCheck`-style call;
- no change of party.

It forces both of them visible to the Allied side. They are ordinary Japanese carriers that steam to
HakuNav1, stop, and then launch their own strikes.

Nothing in the script orders any Allied unit to attack them. Only these can damage them:
- the player (cc9's helm orders);
- the Allied ships' own autonomous gunnery and AA once they are in range.

In the logs read here, they survive for four reasons:

1. **No strike gets off the deck.** p3 records 248 `player air ops launch REFUSED ... "no such slot"`
   refusals for Haku1-4 from Enterprise (§4).
   - Slot 0 means "pick one". The host's `air_ops_pick_launch_slot_006c7210` returns `slots.size()`
     when no slot is in state 1 or 5, and the harness refuses that index.
   - The image does not refuse. `006C7210` continues into its growth path: it appends a new 58h slot
     record whose `+2Ch` state is 1 and returns that new index (§3.3).
   - This is a host divergence on cc9's air-ops path, and it is the main blocker.
2. **The strikes that do launch die before release.**
   - p3 Enterprise_sqn52 (at Haku1) and sqn53 (at Haku2) reach the target: `aim lead ... -> Haku1/2`
     lines from L523561 on.
   - All six planes are then shot down by Shima9/Shima11, the escorts, and by Haku1/2's own AA.
   - They die 0.4-1.3 km from the target at 135-316 m altitude, t=2472-2552 s, still above torpedo
     drop height.
   - No `torpedo drop` by a Haku strike appears anywhere in p3. The only Enterprise drop in the run is
     at L394124, before the reinforcements arrive.
3. **Surface gunfire does damage them, but repair undoes it.**
   - In p3, Loui1 and NH attack-move onto Haku1 (L898188 on), and Haku2 is driven down to
     health 1653 (L1382116).
   - The Loui target then moves to Haku3 (L2054827), and Haku2 is never finished.
   - The script enables repair on both of them (§2.3), and `kHullRepairEnableRouteBound` is ON, so
     partial damage regenerates.
4. **In p6 (variant n) they never reached HakuNav1.**
   - At the last `ship ai step` (48120, L510147), Haku1 is still in `movetopos` at d32c=16877 m.
   - Enterprise is dead by L496560.
   - The only strikes are one Lexington pair, queued at L503348 and L503465.
   - No impact on Haku1/2 is logged.
   - A p6-style run needs more time, or the Haku approach checked, before the strike question even
     arises.

Also note that `ReinDead` (lines 1181-1187) needs every entry in `Mission.FinalTrgs` dead:
- Yamato and Musashi;
- Haku1-4;
- the six Betty squadrons that `luaFuntime` adds (lines 1795-1813).

In both logs the Bettys die by t~1930 s (p3) and t~1776 s (p6). Yamato, Musashi and Haku1-4 never die
in either log. Fixing Haku alone does not complete primary 5.

## 2. What the script does to Haku1/Haku2

### 2.1 Spawn (luaReinTime, lines 1678-1712)

- Line 1681, `luaSpawnAndJoinGroup(Mission.HiddenHaku, false, Mission.HakuGrp)`.
  - `HiddenHaku` is Haku1, Haku2 and Shima8-11 (lines 712-718).
  - The helper at lines 2218-2265 calls `GenerateObject` on each of them.
  - It then calls `JoinFormation(unit, spawnedTable[1])`, so Haku2 and the four Shimas follow Haku1.
  - It also calls `NavigatorSetTorpedoEvasion(unit, true)` and `NavigatorSetAvoidLandCollision(unit, true)`.
  - The not-yours branch adds `SetSkillLevel`, `TorpedoEnable` and `RepairEnable` (§2.3).
- Line 1684, `NavigatorMoveToRange(Mission.HakuGrp[1], FindEntity("HakuNav1"))`. This is the only
  movement order the script gives them. Nothing re-orders them later.
- Lines 1687-1696: `Mission.JapCV = {Haku1..4}`, then `SetAirBaseSlotCount(unit, 3)`.
- Lines 1698-1708: `Mission.FinalTrgs = {Yamato, Musashi, Haku1..4}`, then
  `SetForcedReconLevel(unit, 2, PARTY_ALLIED)`. This forces the Allied recon record to level 2, so
  the targets are revealed to the Allied side rather than hidden.

### 2.2 After parking (the objective loop, lines 1150-1187)

While `Mission.ReinHere` is set and primary 5 is active, each live JapCV member goes through
`luaAirfieldManager(unit, {312}, {163, 159}, luaRemoveDeadsFromTable(Mission.USCV))` (line 1173).
That is their own CAP and strike launches against the US carriers (commandhelpers.lua:7872ff).
- It uses `GetProperty(NumSlots)`, `IsReadyToSendPlanes`, `LaunchSquadron`, `UnitGetAttackTarget`,
  `PilotSetTarget` and `SquadronSetTravelAlt`.
- None of these acts on the carrier's own vulnerability.

The p3 log agrees:
- Haku1/2 alternate `movetopos` and `follow`, then `stop` (389 `state=stop` steps from L759151);
- their squadrons (Haku1_sqn36/38, Haku2_sqn37) strike Enterprise and the other carriers.

### 2.3 Calls that are absent, or that work against a kill

- **`SetInvincible`.** It appears only in:
  - a commented-out debug block on `Mission.RllyBruh` (lines 902-911, `SetInvincible(unit, 0.4)`);
  - `Mission.EnolaGay`, `Mission.Bearcat` and `Mission.Hayate` in the finale (lines 1843 and
    1863-1864).
  Haku is never named in any of them.
- **No hide, hold or party change.** There is no `SetParty`, `Hide*`, `FreezeUnit` or
  `SetSkipDeadCheck`-type call on Haku. The only `SetParty` calls are line 59 (`this`) and line 1106
  (captured bases).
- **`RepairEnable(unit, true)`.** For non-player groups it is set at Difficulty 1 or 2 (lines
  2253-2261). Haku therefore repairs.
- **Torpedo strike targets.** `luaAirfieldManager` filters them to MotherShip, BattleShip, Cruiser,
  Cargo and Destroyer (commandhelpers.lua, the `AMMO_TORPEDO` branch). That filter only applies to the
  Japanese launches. It has no bearing on Allied strikes at Haku.

## 3. The image and the host, native by native

These are the natives the script uses on Haku1/2. "Host" is the binding row in `src/` on
origin/main as merged into agent/cc13-mission1 on 2026-10-12, read with `git grep`.

| Native | Image | Host |
|---|---|---|
| GenerateObject | 00944FD0 | bound (`src/mission_lua_host.cpp:280`, `include/bsp/mission_lua_bindings.hpp:112`) |
| JoinFormation | 00899D10 | bound (`src/game_hosts_script_orders.cpp:176`, 3283) |
| NavigatorSetTorpedoEvasion | 008A3CD0 | bound (`game_hosts_script_orders.cpp:170`, 3279) |
| NavigatorSetAvoidLandCollision | 008A3B10 | bound (`game_hosts_script_orders.cpp:169`, 3277) |
| SetSkillLevel | 00895250 | bound (`game_hosts_script_orders.cpp:177`, 3288) |
| TorpedoEnable | 0089C8F0 | bound (`game_hosts_script_orders.cpp:182`, 542) |
| RepairEnable | 008AD330 | bound (`game_hosts_script_orders.cpp:178`, 3539); `kHullRepairEnableRouteBound = true` (`include/bsp/game_hosts_gunnery.hpp:429`) |
| NavigatorMoveToRange | 008A2F20 | bound (`game_hosts_script_orders.cpp:173`) |
| SetAirBaseSlotCount | 008963E0 | `kLuaSetAirBaseSlotCountBound = true` (`include/bsp/game_hosts_lua.hpp:273`) |
| SetForcedReconLevel | 008AA8F0 -> 00805CF0 | `kForcedReconLevelBound = true` (`include/bsp/game_hosts_lua.hpp:187`) |
| GetProperty / IsReadyToSendPlanes / LaunchSquadron | 0088BF80 / 00895D20 / 0089E3C0 | table rows in `src/mission_lua_host.cpp` (19, 80, 136) |
| UnitGetAttackTarget / PilotSetTarget / SquadronSetTravelAlt | 008A6DE0 / 008A4C90 / 0089F550 | rows in `mission_lua_host.cpp` (236, 204) and `game_hosts_script_orders.cpp:134` |
| SetInvincible (not used on Haku) | 00897A50 | row in `mission_lua_host.cpp:75`; floor in `game_hosts_gunnery.hpp:1045ff` |

Every native the script applies to Haku1/2 has a host row. None of them makes the target immune.

### 3.1 Recon

- `SetForcedReconLevel` -> `00805CF0` sets record `+10h = 1` and `+8h = level`.
- The recon pass then skips the sensor test for that record (`00806883`; see
  docs/LUA_BINDING_MISSION.md "SetForcedReconLevel, 008AA8F0").
- Haku is therefore at least level 2 for the Allied side throughout. Detection is not the reason
  strikes fail, and the p3 `command target` and `aim lead` lines confirm the Allied planes have the
  target.

### 3.2 Target validity

Allied strikes reach terminal aiming at Haku1/2 (`aim lead Enterprise_sqn52 -> Haku1`, p3 L523561). No
filter rejected the target. The failure happens later, in AA attrition before release.

### 3.3 The launch slot

- Live Ghidra decompile, 2026-10-12, of `006C7210` (one caller, `006CC690`, from
  `LaunchSquadron 0089E3C0`):
  - it walks `block+4Ch` at stride 58h while the index is below `+50h`;
  - it returns the first slot whose `+2Ch` is 1 or 5;
  - otherwise it grows the array (`+54h` doubled+2 when full) and builds a temporary record;
  - in that record, `local_64`..`local_38` lands at `+2Ch` = 1 (`local_38 = 1`, at 64h-38h = 2Ch);
  - it copies the record in with `0065CAF0`, increments `+50h`, and returns `+50h - 1`.
- The host reconstruction is `src/air_operations.cpp:378`. It returns `deck.slots.size()` and does not
  add the record. `GameScriptOrdersHost::player_air_ops_launch` (`src/game_hosts_script_orders.cpp:1573-1575`)
  then refuses the launch with "no such slot".
- The p3 tally of refusals by target is:
  - 32 at Haku4;
  - 31 each at Haku1/339, Haku2/331, Haku2/339, Haku3/331, Haku3/339 and Haku4/339;
  - 30 at Haku1/331;
  - 6 at Shima6;
  - 11 at the TroopTrans.
- docs/AIROPS_LAUNCH_GATES.md (lines 99 and 201) already records the growth path. The host's
  `size()` return marks where the record would land, but nothing creates it.
- Whether stock for the appended slot exists is a separate gate (`006BF230` counts stock reserved by
  states 1 and 5). That is for cc9 to check before the harness relies on the growth.

## 4. What would let a strike or surface ship kill them

In order of leverage:

1. **Model the `006C7210` growth path, or have the harness name a real slot.** A Haku strike can then
   launch whenever stock exists, instead of being refused 248 times.
2. **Send larger or escorted strikes, or a surface group that stays on target.**
   - Three-plane torpedo flights are destroyed by Shima8-11 and the carriers' AA before release (§1.2).
   - In p3, the Loui/NH surface attack brought Haku2 to 1653 health and then left for Haku3, so
     `RepairEnable(true)` lets Haku2 recover.
   - A target order that holds on Haku1/2 until they are dead (helm order) would finish them.
3. **Check the Haku approach time in variant n.** In p6, Haku1 is still 16.9 km from HakuNav1 at the
   last logged step. If the HQ capture comes earlier in that variant, the run window may be too short.
4. **Kill Yamato and Musashi too.** They are in `FinalTrgs` and are equally required.

## 5. Open questions for cc9

- What `GetDifficulty()` returns in the harness. The p6 scene line L563 says `difficulty=3`, but
  `Mission.Difficulty` only branches on 0, 1 and 2. If it is 3, `SetSkillLevel` gets a nil level and
  neither `RepairEnable` branch runs.
- Whether the AA lethality against torpedo flights at 135-316 m altitude and 0.4-1.3 km range matches
  the image. The descent to drop height may start later in the host than in the image.
