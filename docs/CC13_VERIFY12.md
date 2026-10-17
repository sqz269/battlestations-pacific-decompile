# CC13_VERIFY12: `game_hosts_units.cpp` from line 20001 to the end

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify12 on 2026-10-17 against
base 56f4b923a. It covers `src/game_hosts_units.cpp` ("GHU") from line 20001 to its end (41642).
cc13-verify11 has lines 1-20000. No code was changed and no ledger records were added. Line numbers are
against the base. "Image" means battlestationspacific.exe, read with `bsp.py disasm-raw`,
`bsp.py ghidra decompile` and `tools/pe_const_read.py`. Authored values are from this installation's
`scripts/datatables/planeglobals.lua` and `shipglobals.lua`.

Excluded, as briefed:
- the plane attack arms already covered by CC13_VERIFY1: dogfight, strafe, kamikaze, drop-kamikaze and
  the torpedo task;
- the level-bomb and dive-bomb arms (CC13_VERIFY8);
- the torpedo aim and approach bindings (CC13_VERIFY10);
- `squadron_free_attack` wiring;
- the arms under `kFreeFlightStallForceBound`, `kFreeFlightDyingSpinBound` and the `kRack*` switches
  (GHU `:36190-36310` and the rack census).

GHU `:20001-20270` is the torpedo release-order issue binding, whose consumers 007EE7F0 and 007EEF30
lie in the struck range 007EE000-007F2FFF. It was read for labels only (U1). The OFF switches
`kDiveBombMoveToOriginBound`, `kDiveHitClockBound`, `kPlaneFormationPlacementEnabled`,
`kLandAbortGroundArmBound` and `kRackUnequippedEmptyBound` were judged in CC13_VERIFY8 and are not
repeated. `kLanding904` (`:38514`) is S8 below.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, NaN-only arms, width with no effect, presentation, an arm no
  test row reaches, or RNG draw count only.
- MISMATCH REAL: a wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

## Census

`local/verify12_census.py` swept lines 20001 onward for
`substitut|approximat|stub|simplif|fixed value|pins? draw|no producer|not in the struct|host constant|record-only|unbound|not bound|unimplemented\(|placeholder|stand-in|PARTIAL|taken as|instead of the image|the image calls|not yet|TODO`.
It matched 188 lines. A second sweep for `record\(|unimplemented\(` matched 116. Lines in the excluded
arms were dropped. The rest were grouped by image function and read in context.

The work was done in this order:
1. the air-operations tasks (`:30452-34440`): moveto, land, takeoff, retreat, follow and moveonpath;
2. the pilot planner and commit, and the flight physics (`:34440-36190`);
3. the ship motion tail (`:36310-36788`);
4. the public `GameUnitsHost` methods (`:36788-38960`).

`report()` and the summary (`:38953-end`) only log, and were swept for labels only.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | `:30591-30600`: the kind-7 moveto cruise altitude is Small/LargePlaneTravelAlt + 0.6 x TravelAltRandom for every plane (URGENT, torpedo) | 009C3650 has ordnance arms for torpedo, depth-charge, level-bomb and dive-bomb. The fighter arm has no random term | yes: every scripted MoveTo with no forced SquadronSetTravelAlt | none |
| R2 | `:31060-31083`, `:31122-31132`: the land path point is taken with no owner velocity | 009AFC71-009AFD52 lead the carrier by velocity x min(A8'/speed, 10 s) through 006BCA80 | every carrier landing in begin, final and abort | none |
| R3 | `:31530-31538`: the land/final ground half always uses air brake 0.2 | 009B201B-009B2074: a carrier gets 1.0 | every carrier touchdown | none |
| R4 | `:30645`, `:33708-33712`: moveto planes never register with the leg manager | 009BEC85 009FD050, then 009BECA9/009BECBA 009FC260 into approach+4Ch | movetos within 3000 m of their point | none |
| R5 | `:34994-35020`: the planner's mode-1 roll arm uses contract constants (URGENT, level-bomb, follow) | 0099E27B-0099E3AE: the +2C8h clamp, SoftRollCtrl/Mul/+580h and t = RollSpd^2/(RollAccel x WaggleLimit) | every plane in plan+2CCh mode 1 | none |
| R6 | `:35175-35179`: plan+2B8h is pinned at 1.0 | 0099D924-0099D970 raise it in a dive (TrgSpeedCorr), 0099D756-0099D79A decay it | every descent with plan+2B0h clear, including follow | none |
| R7 | `:35460-35500`: control authority omits 007C0F40 (URGENT, torpedo, bombers) | 007D9A70 multiplies by the bomb-load factor (BombControlLimit) and the turbo factor | every loaded torpedo bomber or bomber | none |
| R8 | `:37053-37074`: PutTo or PutFormationTo on a formation leader leaves the followers where they were (script-visible) | 00819672-00819681 re-place every member at the zero vector, which takes the station arm | PutFormationTo calls, and PutTo on leaders (jm08.lua:1600) | none |
| R9 | `:8826-8847` and `:33912-33918`: the follow tick's release arm is missing (URGENT, torpedo) | 009C1FFD-009C2063: level flight at TravelSpeed and return, with no station keeping | torpedo wingmen holding release orders; level-bomb prepare | none |

### R1 The moveto cruise altitude ignores 009C3650's ordnance arms

`moveto_refresh_009beba0` (`:30574-30646`) sets `cruise = (large ? LargePlaneTravelAlt : SmallPlaneTravelAlt)
+ 0.6 x TravelAltRandom`. With no script travel altitude (`sq_alt_set` false), that value is the altitude
the task flies at. The `kSquadronTravelAltBound` "profile overwrites" arm (`:30625-30626`) uses the same
value.

Image, 009C3650. The leader runs it at activation:
- 009C36DD-009C36F2: local = tuning+364h x 0.6 (00CEFF98).
- Torpedo: 009C36F6 007B93F0, and tuning+43Ch != 0 -> 009C374D +394h = tuning+430h + local.
- Depth charge: 009C376D 007B94F0, and +4B4h -> 009C37C4 +4A4h + local.
- Bomb: 009C37E4 007B9320, then 0047B850.
  - Level bomber: +454h -> 009C384E +444h + local.
  - Otherwise (dive): +4D4h -> 009C38B1 +4C0h + local.
- Any other plane, or a plane whose move-on flag is false: 009C38D0 0047B880, then 009C38FD
  +394h = tuning+35Ch or +360h. This arm has no 0.6 x random term.

This installation's values:
- Torpedo: CruisingAlt 500, MoveOnCruisingAlt true.
- DiveBomb: 1300, true.
- DepthCharge: 700, true.
- LevelBomb: MoveOn false.
- Travel altitudes: 800 (small) and 1400 (large). TravelAltRandom: 50.

Image against host on a scripted MoveTo, before the +2Ch offset that both sides subtract:

| Plane | Image | Host |
|---|---|---|
| torpedo | 530 | 830 |
| dive bomber | 1330 | 830 |
| depth charge | 730 | 830 |
| fighter | 800 | 830 |
| level bomber | 1400 | 1430 |

CC13_AUDIT13_RECENT_FLIPS A4 flagged the 007B93F0/+43Ch arms as "not checked". The same arms are already
modelled in `moveonpath_cruise_profile_009bd6e0` (`:33856-33888`).

### R2 Carrier landings: the approach point has no velocity lead

The label at `:31070-31071` ("the owner velocity is 0, a static airfield") predates
`kCarrierLandingDeckBound` (ON). `:33494-33499` runs `land_geometry_009afaf0(true)` every tick of
land/begin, final and abort.

Image, 009AFAF0's on-ground arm:
1. 009AFC71-009AFD1A store A8' = A8 + min(0.5 (00D7A280) x class+268h x |heading error|, 0.18 x A8
   (00D05AA0)).
2. 009AFD20: t = A8' / the plane's vtable[38h] speed.
3. 009AFD2C-009AFD40: t = min(t, 10.0) (00CE38B8).
4. 009AFD52: 006BCA80(out, t).
5. 006BCBF8-006BCC51 add [holder+4]->+7Ch vtable[34h] (the owner's linear velocity) x t to T.

A8 and +8Ch/+90h/+94h are then measured to that led point. For an airfield the two sides agree. For a
carrier under way the image aims up to 10 s of ship travel ahead.

### R3 Carrier landings: the ground-roll air brake

`:31530-31538` writes air brake 0.2 with the label "this host refuses mother-ship holders". That premise
is stale: carrier holders are now built.

Image: 009B201B-009B2046 checks approach+30h vtable[5Ch](9). A ship gets brake 1.0 (00D7A24C); anything
else gets 0.2 (00CE54A0, read at 009B205E).

### R4 Moveto planes are not separated by the leg altitude manager

`kLegAltitudeManagerBound` is ON, but only moveonpath planes register. The moveto refresh records 009FD050
at `:30645`.

Image, 009BEBA0:
- 009BEC85 009FD050(d, point) registers the plane while 0 <= d < 3000 and releases it otherwise. It
  writes the weight to approach+28h.
- When +28h > 0, 009BECA9 009BD400 and 009BECBA 009FC260 write the separated altitude to approach+4Ch.

`docs/CC13_LEG_ALTITUDE_MANAGER.md:221-223` already names this as a separate cc9 change. It is listed
here because it is live.

### R5 The planner's mode-1 roll arm

`:34994-35020` calls `pilot_roll_bank_demand_0099e2ba` with error_scale 1, band 0, gain 1 and rate 0, then
`pilot_roll_rate_limited_0099e344(demand, 1.0f)`. The label calls "[ESP+28h] and the EBX tuning block"
contracts. That is stale:
- 0099D47F-0099D487 set EBX = GameTuning + 538h, which is loaded.
- 0099D4B3-0099D4E6 set [ESP+28h] = 1/dtScale = 1.
- The mode-2 branch (`:34954-34957`) already feeds SoftRollCtrl, SoftRollMul, derived_580 and
  WaggleLimit to `pilot_plan_roll_0099e2ba` (`plane_ai_control.cpp:548-565`).

The image runs one arm for both modes (0099E26E-0099E3AE):
- 0099E27B-0099E2B5 clamp +2C4h to +-plan+2C8h when +2C8h < pi. The host's mode-1 arm does not.
- 0099E30B-0099E337: |e| < SoftRollCtrl (0.05) gives e x SoftRollMul (0.7). Otherwise e -/+ 0.015.
- 0099E344-0099E367: t = class+1A8h^2 / (class+1BCh x WaggleLimit (0.5)).
- 0099E390: 00419010(-t, 1, t, -1, a).

The mode-1 writers in GHU:
- the follow law's bank target (`:8477-8480`);
- dogfight manoeuvres (`:21899-22071`);
- strafe (`:23678`, `:23697`, `:24080`);
- level-bomb bank holds (`:24810` 009B66DE, `:24847` 009B5A35), and `:25441`;
- the dive glide yaw arm (`:27433`) and the turndown (`:28118`);
- the land final, park and abort ground halves;
- takeoff slow and run.

### R6 The speed-error divisor plan+2B8h

The host sets `tin.speed_scale = 1.0f` (`:35175-35179`), labelled "its producer is unread". The divisor is
applied at `plane_ai_control.cpp:644`.

Image, 0099D300:
- 0099D924 tests +2B0h. When it is clear, 0099D92D-0099D970 set +2B8h = max(+2B8h,
  00419010(-TrgSpeedCorrMinPitch, TrgSpeedCorrSpeedMul, 0, 1, pitch)).
- 0099D756-0099D79A decay +2B8h by TrgSpeedCorrMulDecay x dt down to 1.0.
- 0099D99E-0099D9A3 divide the measured speed by it.

The authored values are DEG(70), 2.6 and 0.8/s. In a 30-degree dive the image's speed hold sees the plane
1.69 times slower and opens the throttle. These host writers of +2B0h = 0 reach it:
- the follow law (`:9161`);
- moveto (`:34179`) and circle (`:34081`);
- land standby (`:30835`) and line (`:31030`);
- retreat (`:33370`, `:33387`) and takeoff (`:33276`, `:33304`);
- moveonpath (`:33796`);
- `:22977`, `:23564`, `:27678` and `:34339`.

`docs/FIGHTER_ROLL_TRACE.md` already notes that the 1.0 hold equals the image only in aim, where +2B0h = 1.

### R7 Control authority without the bomb-load factor

`control_authority` (`:35471-35500`) returns t^2 "x 007C0F40, which is 1.0 here". The label says
007C0F40 is a part-damage scale that returns 1.0 for every plane. Both statements are wrong:
- 007C0F40 is the bomb-load factor. Its Ghidra comment and `include/bsp/plane_ai_control.hpp:291-302`
  say so, and `bomb_load_factor_007c0f40` (`plane_ai_control.cpp:283`) is never called.
- unit+974h/+994h are the weapon slots.

Image: 007D9A70 returns 007C0F40(unit) x t^2. 007C0F40 works like this:
- It starts at 1.0.
- For the first slot whose vtable[210h](2Ah, 0) answers true (007C0F72; torpedo racks answer 2Ah):
  f = 00419010(0, 1.0, 0.8 (00CE74F8), class+15Ch BombControlLimit, remaining load fraction from the slot's
  vtable[214h]).
- If unit+BC8h is set, f x class+608h TurboControlLimit.

This installation authors BombControlLimit 0.7, 0.8 or 0.9 (mostly 0.8) and TurboControlLimit mostly 0.8.
So a loaded plane has 70-90% of the host's rotation authority until it drops its load.

Side finding, outside GHU: `plane_ai_control.hpp:320-323` says nothing writes 00F8731C/00F87320. In fact
007EAADC-007EAAE1 REP MOVSD copies 4Eh dwords to 00F872F0. GHU `:35477-35483` already uses the Lua values.

### R8 PutTo / PutFormationTo on a formation leader

`place_at_world_position_008193a0` places the unit. When the unit leads its group, it only records
`UnitPlace::group_snap_0081963e` (`:37069-37072`). It refuses every point for a follower (`:37057-37061`).

Image, 008193A0, in single player ([00E188A8]+1FE4h != 2):
- 00819672-00819681: when 00778890 answers true, every other member of [unit+284h] gets vtable[118h](&00F87574,
  the zero vector) and vtable[0D8h].
- A follower asked for |pos|^2 <= 25.0 (00CE3880) is placed at its station (008194AE-0081950A). It is then
  turned by vtable[11Ch] to the bearing from the atan2 tail.
- So PutFormationTo moves the whole formation, each ship onto its station.

The host leaves every follower in place.

Live calls:
- PutFormationTo: usn_02 `:985`/`:990`, usn_03, usn_06, prcp_06;
- PutTo on a leader: jm08.lua:1600 (luaJM8USFormation).

CC13_VERIFY4 R4/R5 cover only the heading and the reshape.

### R9 The follow tick's release arm 009C1FFD

Image, 009C1FD0. After 009BFD70 answers true:
- 009C1FFD/009C2006: with unit+C25h set or unit+C20h > 0, the tick goes to 009C200F-009C2063 and returns.
- That arm writes plan+2D0h = 2 with +2BCh = 0.0, +2C4h = 0.0 with +2CCh = 1, +2B4h = class+190h
  (TravelSpeed), +2B0h = 0 and +2D8h = 1.
- 009BFEE0 and 009BEE30 do not run on that tick.

The host has two copies, and neither matches:
- `Impl::run_follow_tick_009c1fd0` (`:8826-8847`) always runs the follow law. It only skips the trail arm.
  It is used by the torpedo task's follow state (`:28661-28667`), level-bomb prepare (`:24806`, `:25073`),
  land follow (`:32513`) and retreat follow (`:33408`).
- `run_moveto_follow_tick_009c1fd0` (`:33912-33918`, used by moveto and moveonpath) records the arm and
  returns without the six stores.

## Other findings

| Id | Host | Verdict | Note |
|---|---|---|---|
| U1 | `:20120-20133` ctl+390h = 1.0 x 0.95 | UNRESOLVED (torpedo; struck consumer) | The label cites 0079CD36 as the seed. That site is in 0079CBD0 BSP_GameCameraModifier_EnsureCamera, which writes [[edi+8]+38Ch..+398h] = unit class+A0h (Length) x 0.95 on a camera object, not the squadron. `docs/PLANE_SQUADRON.md:375` says squadron+390h has no located producer. `docs/TORPEDO_RELEASE_ORDERS.md:273-291` repeats the camera attribution. A disp32 store census (fstp, movss, mov, mov-imm at +390h) finds no squadron writer. A sub-object store (base+37Ch, +14h) was not searched. |
| U2 | `:32650-32668` takeoff floor | MINOR / UNRESOLVED | The image samples squadron+350h (the squadron record's own layer). The host uses squadron+34Ch's tan(70 deg) layer. The done arm is usually decided by the speed test. The tail 0071E430 (end a moveto command whose target is the plane's own squadron, 009CFB04) is recorded, not done, and its reach is unknown. |
| U3 | `:33933-33939` follow sight search | UNRESOLVED | 009C211D-009C2355 runs only while +85h is raised. Its tail always stores dir+4Ch = 00CE3958 or 0, and clears plan+2E5h under +85h and plan+268h bit 0. The host records it. Its reach depends on +85h. |
| U4 | `:36696-36701` the "unreconstructed_override_remainder" for 0074BE00 (vtable 00CFF9EC, 00749B20) and 00758D30 (vtable 00D015E8, 00758270) | UNRESOLVED | The bodies after the base call were not read. The submarine one (00855420) is covered at GHU `:10382-10422` (air, crush, seabed scan), so that half of the record is stale. |
| U5 | `:33419-33425` 007C6C30 only for retreating planes | MINOR | The image also counts down a plane outside the map that has no squadron (+9D4h == 0). Squadron planes not leaving on purpose reset in both. 007EE2E0 is in the struck range and was not read. |
| U6 | `:31267-31281` land/begin W = bot lifetime | MINOR | Exact when the land task is at least 10 s old. A plane spawned less than 10 s before land/begin takes w_gain 1 instead of interp(5, 0.01, 10, 1, W). |
| U7 | `:30776`, `:34113-34115` CheatTurbo = 0; `:34803-34805` dtScale 1 | MINOR | SetCheatTurbo is a debug cheat (commandhelpers.lua:4050). Only bsm_09 and chg_1 call it; no test row does. CC13_VER_3 D1 covers the same input. |
| U8 | `:36587-36602`, `:38436-38447` the kill-list drain and the 00928C80 Lua self release | MINOR | 00928C80 sets self.Ptr = nil and self.LastPosition. The host records it on the KillDepth and kamikaze paths. LastPosition is read only by commandhelpers' mission-fail camera after TrulyDead, and TrulyDead never answers true (CC13_VERIFY5 R3). |
| U9 | `:37152`, `:37186` group leave: director notice vtable[114h]->[5Ch](successor) and observer unregister 00694A60 | UNRESOLVED (minor) | Recorded and not read. |
| U10 | `:37112` the 006E00E3 +310h notice | MINOR | The label holds. |
| U11 | `:37741-37744` +520h taken as 0 when 00828F66's gate fails | MINOR | The constructor value was not read. |
| U12 | `:38164-38172` the initial stop on water uses the leader's position | MINOR | 007EF8F0/00468560 are unread. The point is the squadron's own position in both. |

## STALE LABEL

| Host | Label | Why stale |
|---|---|---|
| `:31727-31730` | "the carrier arm is not reached: mother-ship holders are refused" | `:31762-31771` takes the carrier arm under `kCarrierElevatorBound` (ON). |
| `:31070-31071`, `:31532-31534` | "static airfield", "never class 9" | carrier holders are built (`kCarrierLandingDeckBound` ON); these are R2 and R3. |
| `:34472-34477` | "the non-dogfight cones are not delivered"; "a plane with no modelled task has no arm" | `kTaskGunConeBound` (ON) delivers the cones (`:32496`, `:33377`, `:33398`). `kTaskGunUntaskedPlanesBound` (ON) ticks untasked planes. Only the generic_suppress_520 ordering remains, which is MINOR. |
| `:34647-34650` | "PilotFires ... not loaded here, so ... stays a contract" | `kPilotFiresBound` is ON and `:34653` reads it. |
| `:35003-35008` | "[ESP+28h] and the EBX tuning block are contracts" | the tuning is loaded; this is R5. |
| `:35203-35207` | "B1Ch and AE0h have no displacement writer, so the term is taken as zero" | `kPlaneVelocityLowpassBound` (ON) computes the term at `:35208-35220`. |
| `:35464-35470` | "(ctl+10h)->+0C0h has no identified owner so its term is zero" | `kPlaneDirectionHoldBound` (ON) supplies it at `:35486-35491`. The 007C0F40 half is R7. |
| `:35840-35842` | "Counted, not sent: the takeoff 007C7110 is not bound" | `kBaseLaunchChainBound` (ON) sends C6h at `:35843-35870`. `:35878-35881` is in the dead OFF branch. |
| `:36015-36016` | "powerlost (step 6) is counted, not raised" | `kPlaneWaterPowerLostBound` (ON) raises it and kills the plane (`:36077-36094`). |
| `:38277-38281` | "the units host does not load Repair, so every class answers 0" | `kLiveHullRepairBound` (ON) reads `class_repair_00d0` (`:38285`). |
| `:38501`, `:38514` | "+904h (the landing byte) is not carried: taken clear" | `plane_landed_904` is carried under `kPlaneTouchdownBound`. A landed plane in mode 4 answers C0Ch = 1 here and 0 in the image. This is MINOR: it is the local-input gate of 007BB9A0. |
| `:36696-36698` | submarine override remainder "open" | covered at GHU `:10382-10422`; see U4. |
| `:30978`, `:37591-37594`, `:33687`, `:33696` | "probe unbound", "role bookkeeping not bound", "not bound here" | these sit in dead OFF branches of switches that are ON (`kNearFieldProbeBound`, `kPlayerRoleBookkeepingBound`, `kMoveToCircleSteerBound`, `kMoveToFollowBound`). |

## MATCH

| Host | Image | Note |
|---|---|---|
| `:36577` KillDepth -200.0 | 008265FA GameSettings+3F4h | shipglobals.lua:377 authors -200.0. |
| `:38311-38315` DologSzorzo 2 | 0074F4A1 GameSettings+40Ch | shipglobals.lua:388 authors 2. |
| `:30591-30600` the large/small choice | 0047B880 | Large is IsKindOf(10h) or IsKindOf(16h) on both sides. R1 is about the arms, not this choice. |
| `:30763-30863` land/standby tick 009B0FE0 | constants only | The PE holds 40.0 (00CE685C), 65.0 (00D1FF10), 0.8 (00CE74F8) and 1.6 (00D06BB4), as cited. The branch order was not re-read. |
| `:31085-31147` 009AFAF0 for an airfield | 009AFB46-009AFE59 | Equal while the owner velocity is 0; R2 is the carrier case. |
| `:32741-32751` 009CFB60 with +AA0h = 0 | 007D614B | A launched plane that is not a ShipYardLaunch has 0.0 or -1.0. |
| `:33603-33615` approach+6Ch | 009C359F | Bound by `kMoveToCommandRangeBound`. |
| `:38255-38258` leak frame | 0074F440 | The hit point goes out and back on the same pose. |

## Not covered

- `:20281-30451`: the attack-task bindings excluded above. Only their labels were swept.
- `:34440-34640` (the pilot think head: retakeoff and the gun tick) was read for labels only.
- `:35600-35830` (ground operations) was read for labels only.
- `:36310-36540` (ship sink and wreck motion) was read for labels only, and KillDepth was checked.
- `:38540-38960`, the public getters: these are thin accessors and were not opened one by one.
- The arithmetic of land/line, begin, final, park and abort and of the takeoff steps (`:30881-33300`) was
  spot-checked only where the census pointed.
