# CC13 fix14: GetShipMaxSpeed, GetNumCatapulted, GetDirection host answers

Packet cc13-fix14b (2026-10-17). Three mission Lua natives that the hosts left
UNIMPLEMENTED (no row answered them, so each pushed nothing) now have host answers,
each behind its own switch, all three committed OFF. With a switch off the row is
refused exactly as before (`GameScriptOrdersHost::handles` returns false, or the
trampoline flag is false), records UNIMPLEMENTED and pushes nothing; the new summary
lines print only when the switch is on (or, for the script-orders pair, only after a
call, which cannot happen while off). No game runs were made; status is
build-tested only.

| Native | Address | Switch | Host file |
|---|---|---|---|
| GetShipMaxSpeed | 00890BB0 | `kLuaGetShipMaxSpeedBound` | src/game_hosts_script_orders.cpp |
| GetDirection | 008A7CB0 | `kLuaGetDirectionBound` | src/game_hosts_script_orders.cpp |
| GetNumCatapulted | 00892AA0 | `kLuaGetNumCatapultedBound` | src/game_hosts_lua.cpp |

GetShipMaxSpeed sits beside SetShipMaxSpeed 00890A10 and GetDirection beside
GetPosition 008A7B00 / GetRotation 008A7E60, which the script-orders host already
answers; GetNumCatapulted sits beside GetLastCatapulted 00892860 and GetCatapultStock
00892DE0 in the mission Lua host. None of the three draws from the image RNG
00BD2F10 (no call in any of the three bodies or in 0080FC30 / 00953920 / 0088BA30).

## GetShipMaxSpeed 00890BB0

Image (disk bytes): `__fastcall`, ECX the Lua state, plain RET. 00890CAF 00888AA0 on
argument 0, with no null test and no class test; 00890CC8 0080FC30 with ECX the
entity, result in ST0; 00890CD2 FSTP dword (rounded to float); 00890CD5 00B66480
PushFloat; 00890CDE 00B66400 the result count, one.

0080FC30 BSP_UnitInstance_GetReferenceSpeed: when the powerups byte 00E0C978 and
`[[00F88C30]+B8h]` are both set, `float(scale * [unit+9C0h])` with the scale from
008E6430(4, unit); otherwise `[unit+9C0h] * 1.0f` (00D7A24C), stored to a float.
unit+9C0h is the class `MaxSpeed` that 00822C20 (BSP_UnitInstance_SEntityInit) seeds
and SetShipMaxSpeed 00890B51 overwrites, in the class file's speed units.

Host: the units-host slot's `motion.max_speed` (unit+9C0h) through
`GameUnitsHost::throttle_ceiling_inputs(index).reference_speed`, which is
`bsp::unit_reference_speed_0080fc30(max_speed, kUnitReferenceSpeedUnscaled)`, pushed
through `push_number_float_00b66480`. The multiply by 1.0f is exact, so x87 precision
control does not matter here.

Labelled divergences:
- The powerup scale (008E6430) is not built; the host takes the unscaled 1.0f, the
  same substitution the ship-AI host makes for every 0080FC30 call. Whether a
  campaign run has both 00E0C978 and `[[00F88C30]+B8h]` set (the scaled arm) is not
  established here; include/bsp/mission_lobby_settings.hpp defaults the byte true.
- An entity with no units-host slot (a scene marker, an unresolved table, nil)
  pushes nothing and records `LuaNative::GetShipMaxSpeed(entity without a unit
  slot)` UNIMPLEMENTED; the image reads +9C0h of whatever object 00888AA0 returned
  and faults on null.
- Every units-host slot is answered from its slot value, as the image answers every
  object without a class test. Whether a plane or squadron object's own +9C0h holds
  the same value is not verified; every shipped caller passes a ship or submarine.

## GetDirection 008A7CB0

Image: `__fastcall`, ECX the Lua state, RET. 008A7DAF 00888AA0 on argument 0 (no null
or class test); 008A7DCF 00B67930 creates the result table; 008A7DD4 `CMP byte
[ESI+0C8h],0` / JNE skips 008A7DE7 00414DB0 (the world refresh) when the pose is
valid; 008A7DEC `LEA EAX,[ESI+0CCh]`, 008A7DF2 `LEA EDX,[EAX+20h]`, so the vector is
+ECh..+F4h, the world matrix's forward row; 008A7DF9 0088BA30 writes x, y, z into
the table; one result. Values are the stored floats pushed as numbers; no arithmetic.

Host: `GameUnitsHost::unit_pose(index, ...)`'s forward row (the slot world matrix
row 2, `world[8..10]`), pushed by `push_vector3_table_0088ba30`. The stale test and
refresh go through the same `entity_pose_stale_008a7c24` /
`entity_pose_refresh_00414db0` hooks GetPosition uses (never stale in this host).

Labelled divergences:
- A scene marker has no basis in this host (`SceneMarker` keeps a position only),
  and an entity with no units-host slot has no pose; both push nothing and record
  `LuaNative::GetDirection(entity without a unit slot)` UNIMPLEMENTED, where the
  image reads +ECh of whatever object it is handed.
- A squadron or plane slot answers the slot's world matrix; whether that equals
  the image object's +ECh row for a PlaneSquadronGen is not verified (jm02 passes
  `Mission.PlayerBetty`, a plane, on one arm).

## GetNumCatapulted 00892AA0

Image: `__fastcall`, ECX the Lua state, RET. 00892B9F 00888AA0 on argument 0;
00892BB6 `CMP ESI,EBP` (EBP zeroed at 00892AC3) / JE to 00892BD3 `PUSH EBP` (0);
00892BC3 vtable[5Ch](6) (the ship family), JE on false to the same 0; otherwise
00892BCB 00953920 with ECX the ship, which counts the ten 18h-byte launch slots
+550h..+628h whose first dword is non-zero; 00892BD8 00B664B0 PushInteger. One
integer result on every path, including null and non-ship.

Host: `GameMissionLuaHost::run_get_num_catapulted_00892aa0` resolves argument 0's id
(`air_ops_entity_id`), tests `unit_is_kind_of(index, kUnitKindQueryShip)` and pushes
an integer, one result.

Labelled divergence: the host keeps no catapult launch slots. Nothing builds the
catapult launch (006EC8E0, and 00957450's slot and +630h writes), the same gap
GetLastCatapulted documents, so a ship's count is the count over ten empty slots,
0, until cc9's catapult adapter (ShipUseCatapult) exists. This packet does not
implement catapult launching.

## Missions that reach each native (this installation's Lua)

Paths are under `scripts/` of the game root in config/target.json.

- GetShipMaxSpeed: global/commandhelpers.lua:4028 and :4039 (the Alt+F10 speed
  booster cheat, ships and submarines only); missions/COTP-USN/usn_02 (and PRCPUS
  copy) :821 / :844 `SetShipMaxSpeed(unit, GetShipMaxSpeed(unit)*1.5)`;
  missions/usn/LOMP/07_Komandorski.lua:603; usn_10 (COTP-USN and PRCPUS); jm12 /
  prcpjm12 (COTP-IJN/PRCPIJN); chg_1, chg_5, chg_9; multiplayer competitive04/05/07/
  08/11/13, escort10-13, siege01. While OFF each arithmetic use of the nil result
  raises.
- GetNumCatapulted: missions/ijn/JM/jm02, jm03, jm04, jm14 (and the COTP-IJN and
  PRCPIJN copies); missions/usn/LOMP/01_saving_force_z.lua:703 `num > 0`. jm02's
  luaJM2CheckReconplanes (:2443) tests `not inAir and num < 4`: while OFF the nil
  compare raises; ON, a ship's 0 passes that test, so the script takes its
  launch arm, which then depends on the catapult natives this host does not build.
- GetDirection: missions/ijn/JM/jm01 (:2320, :2335, :2350, :2584) and jm02 (:2736),
  and their COTP-IJN / PRCPIJN copies; commandhelpers' luaGetDirectionVector is a
  different (script) function. jm01 hands the table to luaJM1GetFlakPos.

## Flip guidance

Each switch is independent. Judge any flip from paired runs of the missions above;
GetNumCatapulted ON moves jm02's recon-plane branch (expect launch attempts that the
host then refuses), and GetShipMaxSpeed ON changes usn_02's ship speeds (1.5x of the
class value instead of a raised error).
