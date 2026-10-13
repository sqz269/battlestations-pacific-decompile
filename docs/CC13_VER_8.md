# CC13 VER 8: verification of the mission frame and objective evaluation

Packet cc13-ver8 (Claude, cc13 lineage), 2026-10-13. This is a check, against the disk bytes, of
the host code that decides when a mission is won or lost: the Lua natives a win check reads, the
order in which events reach Lua relative to the script tick, and the timer and countdown natives.
It follows docs/CC13_VER_5.md (D1 entityKilled, D2 kill before Dead), docs/ENTITY_DEAD_FLAG.md,
docs/CC13_GPR_38.md section 1.6 and docs/CC13_WIN_PATH_NATIVES.md.

Sources: `bsp.py disasm-raw` (Capstone over the PE on disk), the PE's `.rdata` vtables
(`local/v8_vtscan.py`, `local/v8_vtslots.py`), and existing cc9 logs (`local/v8_killscan.py`).
There were no game runs. Line numbers are from origin/main b7599fad9. Every name is a hypothesis,
not a recovered symbol. Status words: *exported*, *reconstructed*, *build-tested* as in AGENTS.md;
nothing here is game-validated.

## 1. The step, image against host

The image's fixed step (`00875BB0`, docs/FIXED_STEP_FANOUT.md) runs, per 0.05 s step:

1. `00875C68..00875C80`: the mission clock `[00F876A4]` += 0.05 **before** the fan-out. Outside the
   fan-out the clock also carries the accumulator's leftover (`00875C21` subtracts it at entry,
   `00875F20` adds it back), so a HUD-side reader sees `steps * 0.05 + leftover`.
2. Rows 1-6: physics, buoyancy, the fixed-step callbacks and the deferred event drain. Hits land
   here: `0077CE60` fires `hit` (`00988510`); a lethal one runs `0077D1A0`, which fires
   `entityKilled` (`00986480`) and queues the entity on the destroy list.
3. Row 8 `00929460`: the script thinks (`luaTimetable`, every `luaDelay`).
4. Row 15 `009273A0`: the flush. `vtable[74h]` -> `vtable[7Ch]` writes `Dead` / `KillReason`
   (`00929800`) and, for the unit classes, reaches `00959450`, which fires `kill` (`009813A0`); the
   kill list then runs `vtable[80h]` (the squadron's `kill` report `007F3B10`).
5. After the step: the HUD update (`00735100`: countdown, narrative) on the frame clock.

The host frame (lockstep 0.05 s, `kScriptThinkOnFixedStep`): `run_mission_frame` (world tick; the
spawn drain `run_spawn_queue_0094c490` fires `hit`, `entityKilled` (OFF) and `kill`), then
`run_script_timers` (game_hosts_mission_frame.cpp:3601): `publish_unit_deaths_00929800`, the think
pass, then the blackout / countdown / narrative update.

| item | image | host | verdict |
|---|---|---|---|
| `Countdown` `008B16E0` -> `00733FF0` | +40h level, +44h seconds, +48h = `[00F876A4]`, +4Ch callback, +54h args (old vector freed); no active test | game_hosts_script_orders.cpp:3762 | match |
| `CountdownCancel` `008B19A0` -> `007340A0` | left = +44h - (clock - +48h), FSTP float, `FLDZ/FCOMIP/JBE`: 0 when negative, NaN kept; +3Ch = 0; callback and args freed | :3794, :4633, :4641 | match |
| `CountdownTimeLeft` `008B1B40` | the same expression, no +3Ch test | :3932 | match |
| countdown expiry `00735140..0073521F` | `FLDZ/FCOMIP/JA` then `COMISS [00D7A218]/JBE`: expires when left <= 0 or NaN; +3Ch = 0; callback through `00887E50` | :4878 | match (clock leftover: T2) |
| `GameTime` `008A9320` | `FLD [00F876A4]` | `mission_clock_` (:4330) | match inside a think; T2 outside |
| `GetHpPercentage` `0088E9D0` -> `00923BE0` | +5Dh set -> 0; else `vtable[110h]` clamped to [0, 1], cached at +164h | lua_binding_mission.cpp:77, script_orders.cpp:4216 | match for ships, planes, buildings; **V2** for squadrons and convoys |
| `Dead` / `KillReason` `00929800` | written at the row-15 flush | publish_unit_deaths_00929800 (:4949) | match in value (cause 2: V4); **T1** in order |
| `kill` channel `009813A0` | only from `00959450` and `007F3B10`, both cause 1 only | dispatch_kill_listeners_009813a0 (game_hosts_lua.cpp:5236) | **V1**, fixed OFF |
| `exitzone` channel `00982120` | four producers | none | **V5** (known gap) |
| think walk `00929460` | timed arm decremented in the walk, before the think | run_script_think_pass (:5356) | **V3** |
| `Objectives_*` natives | push no result scripts read (commandhelpers.lua:5763..5976) | recorded | no branch depends on them |
| `EndScene` `008B01B0` | prompt or `004D7970` | recorded only (:3875) | known RECORD (docs/MISSION_END.md) |

## 2. Divergences

**V1. The host fires `kill` for every death; the image only for cause 1.** Fixed behind
`kKillListenerHarmOnlyBound` (OFF), 80dd2a3f2 and 63e0f4185.
- Image. `009813A0` has two callers (Ghidra `callers`), and both require cause 1 ("harm"):
  - `00959450`: `00959460` `COMISS [00F876A4], [00D7A24C]` (= 1.0f, bytes `00 00 80 3F`) / `JBE`;
    `00959475` `CMP [EDI+70h],1` / `JNE`; `0095948A` world+4ACh; `009594C4..009594FE` the
    controller veto; `00959507` `CALL 009813A0`. The unit vtables reach it through +7Ch:
    plane `007BCAA0` (12 vtables), ship/sub `00824B60` OnWrecked (8, plus the landing ship
    `007497F0` by tail jump), LandFort `00747170` (2), LandVehicle `0074DB70`, and four vtables that
    hold `00959450` itself. The convoy (`00CEA570`, +7Ch `0077D270`) and the base
    `UnitTickableEntity` (`00D0DF70`, +7Ch `00878990`) publish `Dead` but never report `kill`.
  - `007F3B10`, the squadron's vtable +80h (vtable `00D087C0`; +74h `00926390`, +7Ch `007F3A60`):
    `007F3B19` `CMP [ESI+70h],1` / `JNE`; world+4ACh; the veto at +35Ch with +36Ah; `007F3B56`
    `CALL 009813A0`. No clock gate.
  - The cause: `00926D90` (Kill) stores it at `00926E02` before `vtable[70h](1)`; a damage death
    gets 1 from `00926C80`. A landing kills with 5 (`007F1D53`), the leave-map kill with 4, and Lua
    `Kill(x, true)` with 2 (`008AC6DF..008AC71B`). The squadron dies with its last plane's cause
    (`007F3A2A`).
- Host. game_hosts_lua.cpp:5236 dispatches every destroyed row. The host records cause 4
  (game_hosts_units.cpp:8213, :8234) and 5 (:14259); the gunnery funnel drops Lua Kill's cause
  (game_hosts_gunnery.cpp:13454 `(void)cause`), so those read as 1.
- Effect. Every kill callback whose victim landed, left the map, was soft-killed by script
  (882 `Kill(..., true)` calls in this installation's scripts, mtime 2024-07-13), or died in the
  first second, runs in the host and never in the image.
  - Existing log: cc9-lua52 `l56_R_p01r.log` (PRCP01) fires `luaUS03HintReinforcement` for
    `Enterprise_sqn05` with `KillReason=landed` at 496.70 s; the image would not (a hint and a
    `RemoveListener`). `p6e8_Br2L`, `s46_b_a4`, `s46_b_a5`, `s46_g_i08idle` and `g40_tc1_u1g` have
    landed or exitzone deaths but no matching listener: no change predicted there.
  - JM09 `luaJM9KingfisherDead` (jm09.lua:1627) takes `SecFailed` on a Kingfisher exit in the
    host; the image never calls it for an exit.
  - A kill-count objective over enemy squadrons stops counting squadrons that landed or retreated.
- The switch: a death is dispatched only when `unit_death_00959450` (mission_result.cpp:235;
  its `unit_party_70` input is the cause, not a party) or `squadron_death_reports_kill_007f3b10`
  reports it. The cause is the units host's, or the one this host's own Lua Kill passed
  (`lua_kill_cause_`); a fused squadron slot takes its last member's cause.
  SUBSTITUTIONS (labelled): the controller veto (+C41h / +100Ah / +36Ah) is not held; the death
  time is the gunnery host's death clock; the convoy entity's own death is still dispatched.

**V2. `GetHpPercentage` on a squadron or a convoy.** Adoption note: game_hosts_script_orders.cpp
is leased by cc9-lua52.
- Image: the squadron's `vtable[110h]` is `007EDC30`: the float sum of `00923BE0` over the
  +3CCh current members, `FIDIV` the initial count +3C8h (removed planes count as 0). The convoy's
  is `00742940` (docs/CC13_GPR_41.md).
- Host: `unit_health_vtable_110_00923bf6` (script_orders.cpp:4230) reads the fused slot's own
  row, so a squadron answers its wing-0 plane's health fraction.
- Model: `bsp::squadron_average_health_007edc30` (src/mission_ver_cc13.cpp).
- Effect: no reach found. Of the 336 `GetHpPercentage` / `GetHPPercents` calls, the named
  arguments are ships, buildings and convoy members; the generic `unit` sites
  (commandhelpers.lua:18777 `luaAddHP`, the JM/PRCP rows) are ships and buildings.

**V3. Every `luaDelay` interval is one step short.** Adoption note: game_hosts_script_orders.cpp
is leased by cc9-lua52.
- Image `00929460`: for a live, eligible entity with +1DCh set and +1E0h > 0,
  `009294C7..009294D7` stores `delay - step` into +1E0h, and only then `009294E5` calls the think.
  A `SetWait` (`008982C9`) inside that think writes the new delay, which is first decremented on
  the next pass. An entity registered this pass is on the pending list (spliced at `0092958D`)
  and is not decremented either.
- Host (script_orders.cpp:5356-5363): after the walk, every script entity with `delay_armed` and
  `delay_seconds > 0` loses one step, including the delay the think just re-armed
  (`luaTimetable`'s `SetWait`, scripts/global/timetable.lua:47) and every timer created during the
  walk (`luaDelay` -> `CreateScript` -> `luaDoTimeTable` -> `SetWait`). It also decrements entities
  the image's walk never visits.
- Effect: each link of a timetable fires one pass (0.05 s) early, so a chain of n links drifts by
  n steps. No single win or fail branch is tied to it, but every race between a timer chain and
  combat shifts.
- Fix for the holder: decrement in the walk. For example, keep an arm generation on
  `GameScriptEntity`, record the walked ids and their pre-walk delay, and after the walk write
  `pre - step` only where the generation did not change. The decision helper
  `bsp::entity_think_branch` already matches the bytes.

**V4. `KillReason` after Lua `Kill(x, true)` reads "harm".** Adoption note: the cause is dropped
in game_hosts_gunnery.cpp:13454, which cc9 leases.
- Image: cause 2, `KillReason = "soft"` (`00E0CF04[2]`).
- Host: `kill_cause_00926d90` defaults to 1 for these units.
- Effect: none found. The only `"soft"` test (jm13.lua:1218) is commented out, and the `"harm"`
  test (jm09.lua:1630) is inside a kill callback, which V1 removes for cause 2 anyway.

**V5. The `exitzone` channel is never dispatched.** Already the cc9 Lua lane's recommended binding
(docs/SQUADRON_LAND_TASK.md 5gi). Reported to the lead; not edited here.
- 36 subscriptions. Failure and phase branches that never run in the host:
  - COTP USN01: `luaUS01CargoesExited` (usn_01_battle_of_eastern_solomons.lua:1445, primary 3
    failed plus `luaMissionFailedNew`) and `luaUS01BomberExited` (:2181, `MissionPhase = 666` plus
    `luaMissionFailedNew`).
  - USN03: Enterprise exit -> `MissionPhase = 5`.
  - USN05/06/10/12: exits handled as the unit's death.
  - BSM09, CHG4, CHG6, escort10, and the seven training grounds' `PlayerExitListener`.
- Producers: `007C6C30` and `007F31A0` (game_hosts_units.cpp, cc9), `0081BEB0` and `00826D70`
  (the ship out-of-map countdown, not hosted).

## 3. Timing-only differences

**T1. A think sees `Dead` one step earlier in the host.** In the image, the thinks of step N
(row 8) run before that step's flush (row 15), so they see a step-N victim alive. They also run
before its `kill` callbacks. In the host, `publish_unit_deaths_00929800` and the kill dispatch both
run before the frame's think pass. So the host's thinks see the death, and the kill callbacks'
side effects, one step early.
- The comment above publish_unit_deaths_00929800 (script_orders.cpp:4943) calls this order an
  assumption. The bytes settle it the other way.
- The host's `GetHpPercentage` gate (`unit_dead`, set at the kill) shares this. That matters
  only for deaths without zero health: `Kill`, landings and exits.
- Adoption note: run the step's thinks before the death publication and the kill dispatch of that
  step's deaths.

**T2. Clock reads outside the step.** `[00F876A4]` includes the accumulator leftover outside the
fan-out, and it is rebuilt each frame (`clock - acc`, `+0.05` per step, `+acc`). The host's
`mission_clock_` is a pure sum of 0.05f. Under the lockstep frame the leftover is below 1e-6 s, so
the countdown expiry and `GameTime` in HUD-side callbacks agree to within one float rounding.

## 4. Records

Recorded with `bsp.py ledger add-function` (doc docs/CC13_VER_8.md):
- `007F3B10`: reconstructed and build-tested (`squadron_death_reports_kill_007f3b10`).
- `007EDC30`: reconstructed and build-tested (`squadron_average_health_007edc30`).
- `00733FF0`, `007340A0`, `008B1B40`: host checked against the listing.
- `00959450`, `00929460`: partial divergence, listing checked (the host wiring; the existing
  library models match).

## 5. Status and open items

- Build: `scripts/build.ps1` passes, with ctest 2/2, on agent/cc13-ver8 after merging
  origin/main (14cc2453f).
- Switch: `kKillListenerHarmOnlyBound` OFF. When it is flipped, pair it with `kKillListenerDeadFirstBound`
  (cc13-ver5 D2), because both change what a kill callback sees.
- Open:
  - V5 `exitzone` (awaiting the lead).
  - V2 and V3 adoption by the script-orders holder; V4 by the gunnery holder.
  - The controller veto bytes +C41h, +100Ah and +36Ah (who sets them, and whether the player's
    own unit is vetoed).
  - The convoy entity's own `kill` (image: never; host: when its slot dies).
  - T1's reorder.

## 6. Handoff

- Shas on agent/cc13-ver8: bf698267a (models), 80dd2a3f2 (V1 switch), 63e0f4185 (reuse of
  `unit_death_00959450`), 1c5b03284 (this doc and the records), 14cc2453f (origin/main merge).
- To flip V1: set `kKillListenerHarmOnlyBound` true and pair it with the OFF build. Read the new
  `summary mission script kill harm-only` line (`not_harm`, `grace`). PRCP01 is the first row
  with a predicted change (`luaUS03HintReinforcement` stops firing on a landed squadron).
- Next work: the `exitzone` dispatcher (V5). Planes: `007F31A0` fires `exitzone(plane, 1)` at
  `007F31E6` when squadron +361h is clear and the game state is not 9. `007C6C30` fires
  `(plane, 0)` at `007C6D65` (the warning). The entity is boxed as `vtable[140h]()`, the
  squadron. Ships need `00826D70` hosted first.

## 7. The `exitzone` dispatcher (packet cc13-fix3)

Sha: 8ed7e5176 ("Dispatch the exitzone channel behind kLuaExitZoneListenersBound, OFF") on
agent/cc13-fix3. Switch: `kLuaExitZoneListenersBound` (include/bsp/game_hosts_lua.hpp), **OFF**.
Build-tested (scripts/build.ps1 with ctest); not game-validated, no runs made.

Image, read for this packet (disk bytes and exports):
- `00982120` (`BSP_WarningManager_DispatchExitZoneChannel`), `__thiscall(manager, entity,
  exited byte)`, `RET 8`. The entity becomes `entity->vtable[140h]()`; the boxes are the word
  `+174h` (vtable `00D1AF24`) and the byte (`00D1AF34`); Lua arguments (entity, boolean) through
  `00884A90` and `00884A30`.
- The plane's `vtable[140h]` is `007B97E0` `BSP_Plane_GetSquadron`, `MOV EAX,[ECX+9D4h]; RET`, in
  all nine vtables that hold the plane's `vtable[ECh]` `007BBCF0` (`00D00070`, `00D00308`,
  `00D05F20`, `00D06638`, `00D06920`, `00D0BA80`, `00D19D28`, `00D1A000`, `00D1A2D8`; each slot
  `+140h` = `007B97E0`). So the entity
  is the **squadron**, never the plane. A ship's is `0047F320`, itself (docs/CONTROLLED_UNIT.md).
- Subscription vtable `00D1B69C`: Load `009726D0` (callback, `entity`, `exited` boolean set),
  Save `0096E9F0`, Matches `00968710` (entity set `+0Ch`, then exited set `+1Ch`).
- `007F31A0` `BSP_PlaneSquadron_OnPlaneLeftMap`: `007F31C6` state 9 skips everything;
  `007F31CF` `CMP [ESI+361h],0` / `JNE` past `007F31E3 PUSH 1`, `007F31E5 PUSH EDI` (the plane,
  argument 1), `007F31E6 CALL 00982120`. The leader arm's members are sent message 50h through
  `0077C2A0` (`007F328F`), which queues locally (docs/CRUISE_COMMAND.md), so their own `007F31A0`
  runs after the leader's has latched `+361h` (`007F32EA`) when survivors remain.
- `007C6C30` `BSP_Plane_OutOfActionCountdown`: `007C6D62 PUSH 0`, `PUSH ESI`, `007C6D65 CALL
  00982120`, the warning.

Host: each units-host death of cause 4 (only `retreat_leave_map_007ee410` records it, both
`note_kill_cause(..., 4)` sites) of a plane in a squadron record fires (squadron slot + 1, true),
before `entityKilled` and `kill`. `note_exit_zone_00982120(unit, exited)` is the public hook for
the ship producer (cc9 ships46: `00826D70`, `0081BEB0`; pass the ship's own index) and for the
plane warning. Substitutions are labelled at the switch: next-frame evaluation; the `+361h`
latch modelled as "only the deaths at the squadron's first exit clock fire" (later exits are
counted as `latched`); the state-9 gate and the exited = 0 HUD warning not modelled. With the
switch ON the summary prints `summary mission script exitzone listeners bound=1 events= latched=
fires=`.

### Prediction

The 36 subscriptions in this installation (`AddListener("exitzone"`), by what they watch:

| Watches | Subscriptions | With this switch |
|---|---|---|
| planes (squadrons) | COTP usn_01 `exitlistenerID` (`Mission.AlliedPlanes`, :2181, `luaUS01BomberExited`); COTP usn_06 and PRCPUS usn_06 / prcp_06 `Listener_HiddenPlaneExit` (`PlanesHidden`, the three E13A Jakes); escort10 `Listener_PlaneExitZone` (`JAPPlane`); ijn/usn_airtraining `PlayerExitListener` (`playerunit`) | live, when a watched squadron's plane leaves the map through the host's retreat leave-map arm |
| ships | COTP usn_01 `CargoesExitID` (:1445, `Mission.Cargoes` = Cargo1..3); usn_03 Enterprise; usn_05 UsShips / FlagShip; usn_06 `USN_Sub`; usn_10 Iowa / Enterprise; usn_12 `listenerConvoyExited` (`ConvoyShips`); escort10 `JAPPT`; the five sea-training `PlayerExitListener`; BSM09 JapCVs, CHG4 AlliedCVs, CHG6 AlliedFleet (all three `exited = {false}`) | dormant until a ship producer calls the hook |

**The USN01 and USN12 win rows are not these scripts.** Their logs load
`Scripts/missions/USN\usn_1_marshall.lua` and `Scripts/missions/USN\usn_12_augusta.lua`
(cc9-gunnery36 `g36_sb1on_usn01.log`, `g40_bm1_u12.log`), which subscribe no `exitzone` channel
and read no `exitzone` KillReason; no file under scripts/missions/usn does. The branches named in
V5 belong to the COTP-USN campaign (`usn_01_battle_of_eastern_solomons.lua`,
`usn_12_indochina.lua`). So the switch cannot change USN01 or USN12, or any other win row
(USN02, LOMP10, LOMP06, BSM01, USN15, ESMP08: no subscription in their scripts).

The COTP branches, if those missions are run: `luaUS01CargoesExited` and `luaUS12ConvoyExited`
watch ships and stay dormant; `luaUS01BomberExited` watches squadrons and becomes reachable
after `luaUS01BombersMovie_end`, if an allied squadron then leaves the map (no PilotRetreat of
allied planes follows that point in the script; the Japanese retreats at :688 are not watched).
Existing logs: the only run of a COTP-USN script found (`g39_*_p8.log`, usn_12_Indochina) logs no
`retreat leave map`; cause-4 deaths appear only in BSM01 and USNRM01 runs, whose scripts have no
subscription. No change to any existing pair is predicted; RNG streams and order are untouched.
