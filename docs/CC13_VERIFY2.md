# CC13_VERIFY2: labelled substitutions in the ship AI host

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify2 on 2026-10-17 against
base 868bb503c. It covers `src/game_hosts_ship_ai.cpp`: ship navigation, avoidance, targeting and
capture. No code was changed and no ledger records were added. Every line number below is against
that base. "Image" means battlestationspacific.exe, read with `bsp.py disasm-raw` on disk bytes,
`ghidra decompile` and `scan-bytes`.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  or RNG draw count only.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound (ON) arm already
  does what the image does.
- UNRESOLVED.

Every `k*Bound` switch in this file is `true` at this base, so a label that sits only in an OFF arm is
STALE. "RNG" names draws on 00BD2F10 (ECX = stream). The host's `ship_ai_draw` uses one shared LCG
while `BSP_GUNNERY_RNG_STREAMS` is unset. That generator is not the image's, so a missing or extra
draw only moves the host's own sequence. Those cases are listed as MINOR.

## Census

The census matched `substitut|approximat|stub|simplif|deterministic|not in the struct|host constant|record-only|placeholder|in place of`
(41 lines), plus every `LABELLED` / `stand-in` / `no producer` comment (about 80 more). It also took the
96 `record(...)` calls that are not inside an OFF arm, because those are record-only sites on live
paths. Every census line is covered below. No line ranges are left uncovered.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `:2083` `from_submarine = false` (torpedo candidate) | torpedo+49Ch set at 008578B0 (00857780) when +354h, which is y at or below the water at 006E2E7F; read at 009F0C0E: observation += row+24h | yes: a submerged AI sub firing at a ship with torpedo avoidance on | none |
| R2 | `:5339-5366` AvoidBinding::candidate_count_008053c0 | 009E9217..009E9225 walk the recon enemy triple `[008053C0(party)]+0DE8h` | yes: every attackmove approach with enemy ships near; changes the outcome when one is a blip or neutral | none |
| R3 | `:443`, `:9420-9425`, `:9479`; own hull `:960` + `ship_ai_hull_geometry.cpp:181` | 009EAFC0 / 009DE2F0: model world bounds through 0098A8E0; 009EB03C / 009EB053 Y-overlap bail | yes: submerged subs near ships | indirect |
| R4 | `:1159`, `:13217-13226` garrison MinLevel 0, kill arm recorded | 006F3660: MinLevel above level -> 006F36AE..006F36F0 00926D90(member, 2) | yes: usn_13, usn_14, ijn_05, ijn_16, hawaii and others | none |
| R5 | `:3440-3443`, `:3605-3608` target_range_07a0 = 0 | 009E58CE / 009E5BA6 FILD [target+7A0h] (CommandBuilding capture range, 006F280C) - unit+9C8h | yes: a ship sent to a command building | none |
| R6 | `:4963-4966`, `:5238-5240`, `:5570-5572` unit_cruise_speed_0490 = 0 | +490h = artillery max range (00956C20) used at 009E7138, 009E7E7D, 009E76D0 | yes: every gun ship's attackmove approach | yes, stream 1 at 009E719E |
| R7 | `:13126-13131` shipLanded 00986820 recorded | 0074AD90 -> 00986820 dispatches `shipLanded` | yes: JM11, JM13, USN07 landing listeners | none |
| R8 | `:1654-1655` (ship AI), `game_hosts_gunnery.cpp:12629-12632` | 008132C0 "Periscope" -> 009373C0 -> 009373E7 [sub+122Ch] = 2 | yes: a sub that takes the Periscope failure | none |
| R9 | `:9777-9782` allow_max_depth_0020 = 1 | [unit+73Ch]+20h, written by luaMW_NavigatorAllowMaxDepth | scoped: bsm_07 and bsm_11 only | none |

### R1 torpedo from a submerged submarine (URGENT, torpedo)

The label "no host submarine launches underwater" is false now:
`kSubmergedTorpedoLaunchSwimBound` (`game_hosts_gunnery.cpp:833`) is ON, and host subs fire
submerged. In the image:
- 006E2E10 stores `+354h = (y +100h <= water height 0078CF20)` at 006E2E7F. The JBE at 006E2E6B means
  NaN also sets it.
- 00857780 BSP_TorpedoProjectile_Activate sets `+49Ch = 1` at 008578B0 when +354h is set.
- 009F0AD0 tests +49Ch at 009F0C0E and adds `TorpedoObservationSubAddon` (+24h) to the observation
  draw.

`ship_ai_torpedo_response.cpp:74` never adds it. The add-on is 4.0 s at SPNormal, 3.0 at MPNormal, 1.5 at
MPVeteran, 1.0 at Stun and 0 at SPVeteran and Elite (robots.lua rows). Ships therefore see submarine
torpedoes up to 4 s early. The observation draw is taken either way.

### R2 approach traffic candidates

`kAutoTargetReconCandidatesBound` switched 009F5D30 to `recon_triple_units(party, 1)` (`:11204`). The
traffic site 009E9220 still builds its own list. The two lists differ:
- Image: identified (level 2) enemies only, in class-bucket order, from the snapshot the recon pass
  rebuilds every 3.0 s.
- Host: every other party (neutral included), any recon level other than none (blips included), live
  every call, in unit order.

A blip or neutral ship inside its own weapon range + 200 is inserted (009E92D5) and rated (0095EB40),
and the approacher steers away from it. The image does not.

### R3 neighbour vertical band (URGENT, submarine)

Every hull gets the constant world band -10..+50. The own hull gets it from the null-model arm of
009DE2F0, and the observed hull from `kShipNeighbourNullModelBandSubstituted`. In the image both
bands are model world bounds, so they follow depth. A deep submarine and a surface ship fail
009EB03C/009EB053 and ignore each other in the image. In the host they always overlap. The label only
argues the surface-vs-surface case.

### R4 garrison MinLevel

The label says "the scene record carries only whether the key exists ... no row has a member above
its building's level". This installation's text `.scn` files author non-Basic values on
`CommandBuildingInferior` LandForts, for example:
- `COTP-IJN/ijn_05_invasion_of_port_moresby.scn` ("RadarStationFortress 01", Medium, 4 entities);
- `ijn_16_ambushed_at_wake_island.scn` (Advanced);
- `COTP-USN/usn_13_battle_of_iwo_jima.scn` and `usn_14_battle_of_okinawa.scn`;
- most multiplayer scenes.

The kill arm also only records the kill, though `kill_unit_00926d90` exists (`:12960`). Those members
live from the start and after every neutralize, where the image kills them until the building climbs
back to their level.

### R5 MoveTo / MoveOnPath arrival at a command building

The gate is 009E585F IsKindOf(1Ch) on the command target. With range 0, the arrival arm never runs:
- 009E5770: 009E58E6..009E59A2, which posts "finished" through 00984300; the same arm then runs 0071E430(cmd, 1) and holds
  heading and stops (009E00A0).
- 009E59C0: 009E5BBC..009E5C7B, the same steps.

The units host already has `command_building_capture_range_07a0`, and the approach uses it at `:5220`
and `:6244`.

### R6 unit+490h is the artillery range, not a cruise speed

00956C20 seeds +490h and +494h with 10.0f (00CE38B8, 00956D49 / 00956D51). It then raises +490h
(00956E35..00956E43) to the longest range of any gun whose Function `[gun+3F4h]+80h` is 2, 3, 4 or 6.
The three consumers:
- `ship_ai_approach_update.cpp:728`, the flag_1208 cruise band (009E7138..009E719E). It re-draws the
  standoff `U(low*r, high*r)` when the standoff is out of band. With r = 0 the host draws U(0,0) once
  and then holds 0.
- `ship_ai_approach_update.cpp:1039`, the reverse-goal test at 009E7E7D.
- `ship_ai_ring_scan.cpp:391`, the reverse-goal test 009E76D0.

With r = 0 both reverse-goal tests are never true. The draw schedule on stream 1 also differs from the
image's.

### R7 shipLanded

The scripts that listen for this event:
- `jm11.lua:2283`: AddListener("shipLanded", "landingListener"), callback luaJM11ShipsLanded;
- `jm13.lua:2696`;
- `usn_07_invasion_of_tarawa.lua:975`.

None of them is ever called. `jm08.lua:621/629` polls `unit.LandingFinished`, which the host does queue.

### R8 broken periscope (submarine, gunnery lane)

The host rolls the Periscope failure, but `periscope_state_122c` never becomes 2. The pre-pass, the
fire step, the mast arm 00854650 and the sensor category therefore keep treating the periscope as
usable. 009377E0, a collision contact, is the second caller of 009373C0.

### R9 NavigatorAllowMaxDepth (submarine, scoped)

There is no field behind it. `bsm_07_rand_java_sea.lua:830` and `bsm_11_endgame_at_midway.lua:971`
pass false. No JM/USN/LOMP script calls it.

## UNRESOLVED

| Host | Item | Why |
|---|---|---|
| `:294-297`, ArmTailBinding `:8846-8900` | list 6 never drops a sunk ship (009EEAAB setback walk) | The point where the image unlinks a sinking ship from `[[00E188A8]+19CCh]+60h` was not read. In the host the wreck circle keeps pushing goals back. Live after any sinking near traffic. |
| `:4614-4617` | 0095CF80 flak alternate recorded | 0095ECB0..0095ECD4: Function 6 and query+3Dh == 0 and +3Ch != 0 call 0095CF80(ECX = [[dev+354h]+74h], ammo+48h). That copies the alternate's +4/+8/+0Ch (ref-counted class) into the ammo record, a persistent swap, and the rating then reads the flak class. Reach needs gate bytes 12B8 = 1 and 12B9 = 0 out of 009F1BC0, which was not traced. |
| `:12783-12789` | retake exemption: prior slot passed as -1 | 006F4FC2 reads CMP [esi+7B4h],7. The "+2D8h" producer was not found in this pass. |
| `:13005-13006` | 0074713B master LandFort +738h AddDamage forward recorded | No census of capture buildings that have a master. |
| `:10991-11009` | CommandController reset_path_vector 0071F2D1, begin_command 00835C70, raise_override_stage 0071D9E0, raise_queue_stage 0071D810 recorded | The commands host owns the stage model. Not compared here. |
| `:11296-11307` | 00521EA0 on director+18Ch returns null | `kAutoTargetOverrideBound` is OFF (`game_hosts_commands.hpp:214`), so this is the live arm. It belongs to the override lane. |
| `:7793-7799` | goal-record observer 00694A60 / 006952A0 recorded | The death notice of the observed target is handled elsewhere in the host. The callback effect was not compared. |

## MISMATCH MINOR

| Host | Item | Note |
|---|---|---|
| `:9261-9268` neighbour_world_velocity (used at `:3798`, `:3911`, `:3917`, `:9457`, `:9505`, `:9681`) | 00812090 = 0092D730 x axis row +94h..+9Ch (three components, y kept) | The host computes sin/cos(atan2(row2.x, row2.z)) x speed with y = 0, which drops pitch and normalises xz. `slot.motion.pose_row2` already holds the exact row. |
| `:2077-2079`, `:1915` | torpedo +46Ch = atan2(vx, vz) | +46Ch is rewritten by 00856E25 (Steer) and 008574E8 (StepMotion), so it is the current heading and the velocity heading is close to it. |
| `:2120-2129` | 009E4F90's sub-attack +CCh draw (stream 1) is drawn only for submarines | 009F39C0 calls 009F1160 (7 draws), then 009E8450 (state+1500h = -U(0,1), overwritten on enter by 009E86C0), then 009E4F90 (+CCh = -U(0,1), ECX = ebx+1 = 1) for every ship brain. There is no branch at 009F39E5 / 009F3AB2 / 009F3AC3. The host skips the 1500h draw for all ships and the +CCh draw for non-subs. Draw count only. |
| `:3977-3989` | SubmarineLostTime | Read from Lua on every call, where the image stores it once at load (0083F7A1). Same value unless a script rewrites ShipGlobals. The label is stale (see below). |
| `:4561-4570`, `:9809-9810` | 00729F10 called with (false, false, false) | The label's reason is stale: the gunnery fire gate now reads unit+720h (`kShipyardDestructionRuleBound`). That byte is only set on shipyard and airfield masters, never on the ships rated here. |
| `:9814-9835` | sub tube split by cos(horz), in category-7 build order | The image walks child list unit+48h and takes node+110h. Sub tubes are fixed, so only the order can differ. |
| `:9562-9566` | periszkop node taken as found (00853630) | Differs only for a sub class without the node. |
| `:6120-6121` | landing mode 3 point y = 0 | 006AC5D0 writes out[1] = pad+210h -/+ [00D7A258]. Only `\|point - unit\|` against reach >= 300 (009F2247) reads it. |
| `:6145`, `:13431`, `:13722` | 0A5h delivered at the call instead of through 0077C2A0 | Ordering. |
| `:12176-12192`, `:12644`, `:12672`, `:526`, `:11058` | step-order labels | Ordering within one step. |
| `:12524-12527` | +7C0h = -U(0,1) phase not drawn | Capture tick phase within 1 s, plus a draw count. |
| `:748-751`, `:12905` | LandFort class+2Ch point draw taken as count 0 | Draw count, presentation. |
| `:12873` | LandFort +744h arm not hosted | Effect arm. |
| `:12983-12987` | vtable[1ACh] of a building that is not a CommandBuilding | Records only. Landing pads belong to command buildings. |
| `:697` | 00928F50 Party mirror on every named unit | Rows that did not change keep their value. |
| `:831`, `:13232-13235` | garrison re-create is a revive in place | Static garrison at the same matrix. |
| `:863` | 1Bh fort whose vt+204h answers 45h/46h | Only IsKindOf(45h/46h) picks the rate here. |
| `:12767-12786` | slot pick = first slot whose Party is the side | The larger slot + landed pick is not modelled. Single player has one slot per party. |
| `:12491-12498` | +368h = class Armour at build | Level messages rescale it at `:13180`. Whether the image rescales for a scene's initial Level at load was not read. |
| `:6955-6957` | 9999 sentinel guard on sub+1218h | Host-only guard (rule 3). Section 139 found the value identical. |
| `:8080-8084` | Retardation 0 gives braking 0 | The image would FDIV by zero (inf) at 009EF2A1. No installed class has 0. |
| `:11266-11270` | locked script fire target released when the row goes inactive | The image releases it through the observer pair. Moment only. |
| `:1749` | b60 = pending destroy, or the unit is dead | +60h timing. |
| `:12944` | 00926D90 removal of a landed wreck recorded | The host unit is already dead. |
| `:13117` | ramp message 0A6h routed through 0077C2A0 recorded | The latch itself runs. |
| `:13458-13470` | 004F49F0 written as the textbook segment distance in double | Stale too: 004F3730, 00419AB0 and 004F49F0 are reconstructed (`include/bsp/avoid_zone_distance_followup.hpp:19`, `segment_pair_distance_004f49f0`). The image works in float. 008206F0, the landing-craft launch, uses it. |

## MATCH

| Host | Item |
|---|---|
| `:2101-2121` | 009F1160's seven stream-1 draws: order, bounds and stream 1 checked at 009F12CD..009F13F0. B44 = U(+1ECh, +1F0h) = U(1.5, 2.0) and B4C = U(+190h, +194h) = U(1, 2) as host constants, equal to TorpedoAvoidance.CollectTimer and ShipAvoidance.CollectTimer in this installation. |
| `:5811-5822`, `:4845-4856` | 008FB530 FireTargetAccuracy. Row (lv+1)*14h is field 2 of row lv with a 0Ch header (008FB550 BulletThrowMul reads lv*14h+1Ch). Values equal robots.lua 392-432. The image returns 1.0 when gun+39Ch is null (00729F60); every host gun has a bot. |
| `:219` | `kShipTurnRadiusMultiplier438 = 2.0` = shipglobals.lua 342/400 TurnMultiplierMaxSpeed {1.0, 2.0}. |
| `:836-848` | CommandBuilding Armor / HP / Repair tables and SingleInvincibleTime 20 = commandbuildingglobals.lua 3-16. |
| `:4764-4770` | tick damage +3B0h/+3ACh = 100 / 40 (shipglobals.lua 77-78). |
| `:9962-9967` | 0082ADC0 = 004218E0 then 004120D0([class+570h]); the host's group_for_layer walks as 004120D0 does. |
| `:2861-2864`, `:4313-4316` | blk+234h / +29Ch = 1225 "stored nowhere". A byte scan of 009C-009F finds only FST/FSTP writers (009D5CDE, 009D5CE4, 009DB2F4, 009DB2FA, 009DB5EE, 009DB5F6) and no FLD/MOVSS/COMISS reader. |
| `:3554`, `:6505`, `:9914` | brain+308h (= blk+300h) stores. The only reader is 009ED6E6 in 009ED6B0, which counts the field down (009ED72A). |
| `:9768` | unit+0A8h is the world Y for a parent-less ship. |
| `:1915-1930` | class+570h for a threat target: every ship navigation class loads it (`:11985`). |
| `:4450-4456` | ship target with a bot: every host ship has a controller. |
| `:7930` | 009E2FC4's zero triple: the image's own behaviour. |
| `:4990`, `:5152`, `:11151`, `:11903`, `:12411`, `:10451` | pure queries, pass-throughs or diagnostics as labelled. |
| `:5331-5338`, `:4887`, `:3160`, `:5867` | bound draw (009E9209), or a fallback used only while the zone runtime is not ready. |

## STALE LABEL (a bound arm already does what the label says is missing)

These need comment cleanup only:
- `:3833` (0082ECB0 now gets the image's arguments: [unit+984h], the vtable[38h] speed, 1.0; 009E2A6A..009E2A97);
- `:3977` (SubmarineLostTime is read);
- `:4409`, `:5470`, `:11928` (the timer draws are taken);
- `:5059` and `:5088` (the sight tests are bound);
- `:5920` (all four latch inputs are filled at 5947-5975);
- `:6523` (+123Ch is the unit length, `:2016`);
- `:2654` (the target words come from `fill_target_block_127ch`);
- `:6748` and `:6802`;
- `:7458`;
- `:8098` and `:8555` (the neighbours are passed and the clearance neighbour hooks are bound);
- `:8841`;
- `:1603` and `:10547` (`kShipNeighbourListBound`; 10547 is the fallback after the bound branch returns at 10533);
- `:1855`;
- `:2341`;
- `:635` (`kLandingShipRampHullContactBound`);
- `:796` and `:13169` (the +36Ch rescale, the garrison and the gunfire armour are bound);
- `:12715`;
- `:12992`;
- `:13025`;
- `:7078`;
- `:4540`, `:4577` and `:4738` (the firepower predicates are bound).
