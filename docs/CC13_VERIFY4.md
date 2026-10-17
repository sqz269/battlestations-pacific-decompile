# CC13_VERIFY4: labelled substitutions in the script-orders host

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify4 on 2026-10-17 against
base 6982dc190. It covers `src/game_hosts_script_orders.cpp` (and the switch comments in
`include/bsp/game_hosts_script_orders.hpp`): the Lua natives that mission scripts drive. No code was
changed and no ledger records were added. Every line number below is against that base. "Image" means
battlestationspacific.exe, read with `bsp.py show`, `ghidra decompile` and `disasm-raw` on disk bytes.
Mission scripts are this installation's `scripts/` tree (modded install; see the memory note on
BSPRM/AlterBSP). Call counts come from `local/cc13-verify4_argcount.py`, which counts uncommented
calls by top-level argument count.

Excluded, as briefed (live cc9 packets): `EntityTurnToEntity` 008A0A10 (`:2468-2565`), the attack
chooser region (`:1512-1670`, including `kChooserSurfaceSetBranchBound`), and the
`kSquadronMemberOrdnanceBound` loops (007ED7E0 / 830 / 880 / 8D0). Also left alone are the census sites
whose image functions sit in the cc13-struck range 007EE000-007F2FFF (squadron intake 007F1940,
AutoAttackTarget 007F15F0, return-to-base 007F16D0). They are listed under "Not covered".

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  or RNG draw count only.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

Every switch this file reads is ON at this base except `kLuaUnitFreeFireBound`,
`kNavigatorForceTorpedoBound`, `kAirOpsSceneSquadronOrdersBound`, `kAutoAttackAllMembersBound`,
`kChooserSurfaceSetBranchBound` and `kTurnToEntityMemberPlaneArmBound`. The last two are excluded. An
OFF switch in front of a native that scripts call is treated as a live substitution, because the
native is then an unimplemented record.

## Census

The census matched
`substitut|approximat|stub|simplif|deterministic|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|LABELLED|fixed value|unresolved`
(about 70 lines), plus every `log_.unimplemented` and `record_unimplemented` call outside an OFF arm
(about 45 sites), plus the switch comments in the header that carry LABELLED. Every census line is
covered below except the excluded and struck ranges.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `:132`, `:643-645` `kLuaUnitFreeFireBound = false` | UnitFreeAttack 008A67E0 / UnitFreeFire 008A6950: vt[114h] then 0071BF70 (PUSH 2 at 0071BF7B) / 0071BF20 (PUSH 1 at 0071BF2B) | yes: 134 calls in 61 files; JM08, USN01, USN04 rows | none |
| R2 | hpp `:295` `kNavigatorForceTorpedoBound = false` (URGENT class, torpedo/submarine) | NavigatorForceTorpedo 008A7200 fires the unit's torpedo devices | JM06 only (`jm06.lua:2582`), and MP competitive12 | host none |
| R3 | `:3824` `need_air = false` (URGENT, submarine) | 00894100: +1281h set -> pushes 0 unless class+510h/+514h > 0 | where a boat's needAir latch is set: usn_06, usn_11, jm05, jm04, LOMP06, jm12, jm10 | none |
| R4 | `:3892-3894`, `:3927-3929` heading recorded | vtable[11Ch] 008196B0 -> BSP_UnitInstance_SetHeading, pose refresh, 00818EA0 | yes: 99 calls; JM08 `:1600`; USN04 `:3155`.. (checkpoint restore) | none |
| R5 | `:3916-3918` formation shape recorded | 0070EFD0 on [unit+284h] for shape 1..3 | usn_02 `:985/:990`, usn_03, usn_06, prcp_06 | none |
| R6 | `:3935-3971` shot-down count from the controlled unit | 008BC9B0 sums the slot's scoring tree, credited at kill time by 0091BDA0 | bsm_01, bsm_09, usn_08 and 15 more, when the player switches units | none |
| R7 | `:3351` `owner_player_known = false` | 00779DB4..00779DC5 compares follower and target +188h | conditional; no live case found | none |

### R1 UnitFreeAttack and UnitFreeFire are unbound

`kLuaUnitFreeFireBound` is false, so both natives stay unimplemented records. The ON arm already
exists: `run_unit_free_fire` (`:2724`) applies the same squadron, ship-director and plane arms that
UnitHoldFire uses, with stance 2 (0071BF70) or 1 (0071BF20). With the switch off, every UnitHoldFire
(bound ON) is permanent, and a squadron's +348h permissions stay at the 0084D810 default
(`squadron_permissions_0084d810`, `:2578`): Behaviour 0 frees both, a fighter frees fire only and
anything else frees move only.

Script use in the test rows:
- `jm08.lua:817`: UnitFreeAttack on the rocket fighters sent to the CommandBuilding by
  PilotMoveToRange. The image sets allowMove; the host keeps the fighter default (allowMove 0).
- `usn_01_battle_of_eastern_solomons.lua:1326`: UnitFreeFire(oszi) after PilotSetTarget; `:1956`
  UnitFreeAttack(Mission.Scout).
- `usn_04_defend_guadalcanal.lua:2067`: UnitFreeFire(Mission.JAP_DummyDD[1]), a ship director
  (stance 1).

### R2 NavigatorForceTorpedo is unbound (URGENT class)

Committed OFF at 6be1c7682 (2026-09-27). The host body is there
(`GameGunneryHost::force_torpedo_fire_008a7200`, dispatched at `:3742-3753`). The image walks
unit+48h and gives each kind-20h torpedo device vtable[1D8h], stopping after the first when argument 1
is true. Its one campaign caller is `luaJM6SubShot` (`jm06.lua:2582`), the two submarine shots of the
convoy-ambush movie. The script breaks its target ships itself (`luaJM6BreakShips`: DisablePhysics and
BreakShip), so no objective waits on the torpedo. But the two submarine torpedoes the image launches
are missing from the world after the movie. The RNG draws of the image launch (00730160) were not
re-read.

### R3 GetSubmarineDepthLevel ignores needAir (URGENT, submarine)

The image (00894100) loads [unit+1268h]. If [unit+1281h] needAir, [unit+5Dh] or 008522C0 is set, and
both class+510h and class+514h are at or below 0, it pushes 0 (00894236..0089425A). The host hard-codes
`need_air = false` (`:3824`). The header (`:240`) says "no air model", but `kSubmarineAirBound` is ON
(`game_hosts_units.hpp:392`) and the units host keeps `slot_.sub_air.need_air` (+1281h,
`game_hosts_units.cpp:10302`, `:10455`). So a boat whose air latch is set answers its depth level
instead of 0. The scripts that branch on the value:
- `usn_06_guadalcanal_2nd_battle.lua:1781/:1798`: an AI sub, == 3 / == 0;
- `usn_11_protecting_iowa.lua:930`: PlayerSub == 3;
- `jm05.lua:3946/:4048`: enemy subs;
- `jm04.lua:1302`: == 4;
- `06_crucial_cargo.lua:700/:713/:755`;
- `jm12.lua:888`;
- `jm10.lua:1763`: a checkpoint.

None of JM08, USN01, USN04, USN13 or LOMP10 calls it. The other two labelled reads stay MINOR: the
catapult test 008522C0 (+1279h with an MCatapult child, which only seaplane-carrying boats have) and
the kamikaze test (class+510h/+514h). The header's "nothing in this host writes the level after the
seed" (`game_hosts_units.hpp:283`) is also STALE: 008528B0 is reached from SetSubmarineDepthLevel
(`game_hosts_lua.cpp:6255`) and from the ship AI (`game_hosts_ship_ai.cpp:9746`).

### R4 PutTo and PutFormationTo drop the heading

With a third (PutTo) or fourth (PutFormationTo) argument, the image calls vtable[11Ch] with
`arg * pi / 180`. That is 008196B0, which is reconstructed in `src/cc13_sweepC2.cpp` as
BSP_UnitInstance_BroadcastRelocate. Single player skips its 97h broadcast. It then stores
+BCCh = [00CF29A8], calls BSP_UnitInstance_SetHeading(arg), refreshes the pose and runs 00818EA0, which
recomputes +1050h and restarts the wake. The host records `set_heading_vtable_011c` and the unit keeps
its old heading. 99 PutTo calls carry a heading:
- `jm08.lua:1600`: luaJM8USFormation turns the US formation leader toward USNRetreatPoint once the
  LSTs and cargos are dead;
- `usn_04_defend_guadalcanal.lua:3155/:3172/:3195/:3218`: checkpoint restore only;
- usn_05 (10 calls), jm12 (6), and jm11, usn_06, usn_07, usn_10, usn_14 (4 each).

### R5 PutFormationTo does not reshape

`:3916-3918` records 0070EFD0 when `shape - 1 < 3` and the unit is grouped. The image runs
BSP_UnitGroup_ApplyFormationShape on [unit+284h], rewriting column 0 of every live member record. That
body exists as `ship_ai_formation_reshape` (`src/ship_ai_formation.cpp`). Live calls:
`usn_02_battle_of_cape_esperance.lua:985` (shape 1) and `:990` (shape 3), plus usn_03 (2), usn_06 and
prcp_06. jm07 passes `-angle` as the shape, which takes no reshape in either.

### R6 Scoring_GetPlayerShotDown counts the current unit's kills

The host counts sunk enemy planes whose `killed_by` is the unit controlled now. The image reads slot
`arg0`'s scoring tree (record slot*284h+4h, +B4h, ENEMY, keys 7..0Ch). 0091BDA0 writes that tree at
the kill, credited to [killer+2D8h] when it equals +2DCh (docs/LUA_BINDING_MISSION.md). The two
counts differ once the player switches units. Kills by earlier player units are lost, and kills the
current unit made under AI control are added. The comment labels this ("no scoring record"). It is
listed as REAL because bsm_01 drives its HenryShotDown objective with it (`:681`, `:929`, `:2321`).
Other callers: bsm_09 `:628`, usn_08 and 15 more files.

### R7 JoinFormation's owner-player arm

`:3351` says "+188h has no producer here", which is STALE. The units host keeps `role_permission_0188`
(written by 00927D20 under `kPlayerRoleBookkeepingBound`, ON; `game_hosts_units.cpp:36509`), and
SetRoleAvailable reaches it (`:3500`). The image refuses a follow when follower +188h differs from
target +188h and the target's is not 9 (`ship_ai_states.cpp:335` already transcribes the arm). The
host passes false, so the arm never refuses. No live case was found. USN01's joins at `:630` run
before its SetRoleAvailable loop (both 9), and the joins at `:2337` follow it on ships set to 8 on
both sides. So this is a conditional REAL.

## MISMATCH MINOR

| Host | Image | Why minor |
|---|---|---|
| `:707` LandingPoint pads in registration order | 00928860's append order | breaks exact distance ties in 006F2E60 / 006F3AF0 only |
| `:986-990` dummy kamikaze bag: VelocitySI/Vel3D, Invincible, FireStance, DummyObjectID unread | 006FED18-006FEED3 | the released plane starts at the seeded speed; transient, and not a script native |
| `:2275-2277`, `:2788`, `:2828` PilotSetTarget / PilotLand / PilotRetreat install at delivery | 0099ACD0 installs one bot tick after delivery | one bot tick (docs/SENTITY_INIT_ATTACH_ORDER.md 22.7) |
| `:2829-2830` PilotRetreat with no border zone issues at (0,0,0) | the image reads a null zone record | error path |
| `:3126-3127` shipyard moveonpath path build on delivery | 0071F600 when the director begins it | timing within the step |
| `:3145`, `:1340-1346`, `:3395-3397` +174h object id = index + 1 (or marker id) | handle-table ordinal 00F89A0C / 00F89A60 | an identity, consistently inverted by 00521E30's stand-in |
| `:3198`, `:3210`, `:3731` avoidance and TorpedoEnable 5Ah stored at the call | 0077C2A0 loopback, delivered at the drain | ordering within the frame |
| `:3289-3292` parts-land avoidance: bit 8 of a sunk hull not restored, manifolds not cleared | 00C48020(0Dh) at 0092BD00 | sunk hull only; the next contact phase rebuilds manifolds |
| `:3358-3365` entity_route_slot answers -1 | [entity+1ACh] 0..7 -> 00905300 (game+21A0h + slot*284h + 170h += 1) | a per-player stats counter for a player-held follower. The label "no field carries it" is STALE: the units host has `current_roles_01ac` |
| `:3770`, `:3791`, `:4006-4007` AddMatrixInterpolator, ExplodeToParts, ShipSetTorpedoStock queued for the next pass or step | applied at the call | one step. JM08's stock writes (`:1568`, `:1776`) have no same-tick read |
| `:3840-3842`, `:3860` GetSquadronPlanes keys by unit index + 1 from the registry | [sq+3D0h] members' u16 +174h | the same identity; dead members are not dropped as 007F3970 does (bsm_04 intro only) |
| `:402-427` SetParty takes the vt[2Ch] arm for a unit | IsKindOf(2) true -> message 7Fh -> 0095ADA2 calls the same vt[2Ch] | same write, at the call rather than the drain. The label "no resolved concrete vtable" (`:403`) is STALE (docs/UNIT_MESSAGE_ARMS.md 7Fh row); header `:122` (00923B80's child recursion not carried) stands as labelled |
| `:3978-3979`, `:4257-4258`, `:4283-4285` FillPathPoints / GetRotation / GetCapturePercentage on a wrong-kind argument | the image dereferences whatever it is handed | error paths |
| `:4240-4249` EndScene records only | 004D7970 or the 00531B00 restart prompt | the host run ends at its frame budget; a harness boundary |
| `:4572-4581` `random` is a fixed-seed LCG | 00BD2F10 thread state | RNG sequence only |
| `:5049-5054` narrative duration counts the key's characters | the text widget's +ECh after localisation | 0.04 s per character (DialogGlobals TempCharWait). It shifts the end callbacks (luaMissionEnd_CamOnEnt etc.) by under a second. Note `lockit/englishauthentic.lan` exists in this installation, so the label's "no text table" is a host choice |
| `:5065-5068`, `:5023-5024` GetRotation / GetCameraState degrees, CountdownTimeLeft | x87 chains under 24-bit PC (008BF874..008BF8A9: FADD qword 00D13268, fmod qword 00CE3828, FDIV qword 00CE3D28, FMUL qword 00CE3D20) | constants match bit for bit (00D13268 = 131.9468994140625, 00CE3828 = 6.2831854820251465). The host rounds once in double where the main thread rounds each op to float: 1 ulp |
| `:5077-5080` GetCameraState with no published camera uses the controlled unit's matrix | [game+19FCh] | presentation; movies |
| `:5802-5803` script identity mirror on the next mission frame | InitAll row 12 | one frame |
| `:5875-5876` pad observer handle notification slot unread | vtable 00CE75CC | the destroyed occupant is dropped either way |
| `:6239-6242`, `:6313-6314`, `:6357-6358`, `:6390-6391` dialog clip length from the FSB4 header, entry re-lookup, state-2 reset, tick per think step | 005BBF10 / 00452740 | timing and presentation; cwd is the game root (`game_main.cpp:1253`), so the relative message-map path resolves |
| `:4136-4137`, `:4161`, `:5029`, `:5170-5277` HUD countdown and narrative widget calls recorded | 0052B9B0, 005BCA70, 0052AB90, 005BCAB0, 005B7390, 005B5D20, 005B5D40, 005BCA80 | presentation |
| `:4948-4992` local player unit null, interface requests, blackout broadcast, please-wait | 00E188D8, 004CC460, 0076D310, 005B9A04 | HUD transitions and MP-only arms |

## MATCH

- `:383-387` game_non_campaign_flag 0: a campaign session, consistent with the mission host.
- `:3480-3493` session mode 0 and effective game mode 8: single player.
- `:3452-3459` RepairEnable local field +378h: only the non-ship arm reaches it; ships take the routed
  9Fh arm into the gunnery repair task (`:3461-3478`). The reader of +378h on non-ships was not
  traced, so this is MATCH for ships and UNRESOLVED for buildings (below).
- `:3495-3510` SetRoleAvailable direct call: the units host writes the table. The
  `log_.unimplemented("RoleOwner::set_role_available")` at `:3504` is a misleading record (STALE log),
  and `:3512-3522`, the routed and party-reassign arms, are unreachable in single player.
- `:4583-4641` GetHpPercentage health gate and vtable[110h]: 00876260 FLD +370h / FDIV +36Ch; the
  squadron average 007EDC30.
- `:4648-4658` pose never stale: the host publishes every motion tick.
- `:4813-4818` MissionEntity::kill record: reached only from DeleteScript on a non-script entity
  (`lua_binding_mission.cpp:204`).
- `:5336-5416` Dead / KillReason publication: matches 00929800 within the one-frame order bound it
  states.

## STALE LABEL

- `:3182-3189`: "the RECEIVE side is the hole ... nothing calls yet" - both delivery switches are ON
  and `apply_director_avoidance_message_00835640` receives.
- `:2576`: "A ship argument is left a record" - the ship arm is at `:2643-2652`.
- `:4081-4082`: "nothing here plays a dialog to its end" - `kDialogSequencerBound` is ON and
  `dialog_advance_004527f0` erases finished entries (`:6366`).
- Header `:33-41`: vtable 16Ch "answers the neutral false and JoinFormation has no effect" (answered
  since cc8_ship_follow, `:3310`); "vtable 128h (skill), vtable 148h (role)" stay records (both
  bound).
- Header `:257`: "forced levels (SetForcedReconLevel) are not modelled" - bound in
  `game_hosts_lua.cpp:7404` (`kForcedReconLevelBound`).
- `:3351` (R7), `:3358-3365` (MINOR above), `:3824` (R3) and `:402-405` (SetParty) are stale as well;
  they are classified above by their effect.

## UNRESOLVED

- `:749-757` an air-ops squadron is one plane unit standing for the 1089-byte container: the largest
  labelled substitution, and squadron work belongs to the cc9 lanes.
- `:847-856` a base launch is made at the base origin with identity axes (the owner's rotation rows,
  owner+CCh, are not copied). State 1 starts inside the base, so the deck placement probably
  re-poses it. That placement was not read.
- `:3452-3459` RepairEnable on a non-ship unit: the image writes [entity+378h] and its reader was not
  traced.
- `:2653-2656`, `:2708-2711` UnitSetFireStance / UnitHoldFire on a unit that is neither a squadron, a
  plane nor a ship (forts, airfields): the director 5Ah arm is a record. No script call on such a unit
  was found by name pattern.
- `:3670-3673` SetFireTarget with a Vector3: the image makes a dummy entity. No position-argument call
  was found in the 42 calls.
- `:4755-4763` CreateScript's vtable[98h] placement (0089892C) and 00927610 (00898932): recorded; the
  effects were not read.
- `:1105-1107` the deck sub-updates 006C77E0 and 006C6540 of 006CDC70 are not run here.
- `:313-314` PrepareClass (008C8F70) is deliberately unrouted, so it is an unimplemented record. It is
  not a substitution in this file.

## Not covered

- Excluded by the brief: `:1512-1670` (chooser and member ordnance), `:2468-2565` (EntityTurnToEntity).
- In the struck range 007EE000-007F2FFF: `:1680-1706` squadron intake 007F1940, `:2071-2232`
  AutoAttackTarget 007F15F0 and the standing attacks (also `kAutoAttackAllMembersBound` OFF), and
  `:1375-1438` `resolve_return_to_base` 007F16D0 answering 0. So a PilotLand whose site is nil
  becomes returntobase and issues no task. The 66 PilotLand calls all pass a site.
- Harness-only player input, not script natives: `:1784-1898` (player air-ops launch) and
  `:1900-2060` (held-slot orders, `kAirOpsSceneSquadronOrdersBound` OFF; refuses JM08's
  MainAirFieldEntity 01 slots).
