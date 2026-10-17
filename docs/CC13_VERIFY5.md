# CC13_VERIFY5: labelled substitutions in the mission Lua host

This is a verify-only pass for the cc9 gameplay lead, done by worker cc13-verify5 on 2026-10-17 against
base f30aa7714. It covers `src/game_hosts_lua.cpp` (the only `src/game_hosts_lua*.cpp` file) and the switch
comments in `include/bsp/game_hosts_lua.hpp`: the unit and class property readers, GenerateObject, and the
mission event dispatch. No code was changed and no ledger records were added. Every line number below is
against that base. "Image" means battlestationspacific.exe, read with `bsp.py show` and `ghidra disasm`.
Mission scripts are this installation's `scripts/` tree (a modded install; see the memory note on
BSPRM/AlterBSP). Call counts come from `local/cc13-verify5_uses.py`, which counts uncommented calls.
`local/cc13-verify5_genobj.py` lists GenerateObject names that a mission file repeats.

Excluded, as briefed (live cc9 packets): the Cloud arm of `run_generate_object_00944fd0`, and the
`shipLanded` dispatch point and 00986820 (cc9-lua65). `kReconDetectionRecordsBound`
(`include/bsp/recon_sensor_pass.hpp:43`, OFF) is cc9-gunnery36's live packet (its lease holds
`recon_sensor_pass.hpp`), so its arm in `dispatch_recon_listeners_00980e50` is not judged.

Rubric (cc13 common brief):
- MATCH.
- MISMATCH MINOR: edge guards, error paths, width with no effect, ordering within a step, presentation,
  or RNG draw count only.
- MISMATCH REAL: wrong result, or a missing or extra call, write or side effect on a normal path.
- STALE LABEL: the comment says something is missing or substituted, but a bound arm or another host
  already does what the image does.
- UNRESOLVED.

Most switches this file reads are ON at this base. The OFF ones are `kLuaSquadronForceGunnerModeBound`,
`kLuaTrulyDeadBound`, `kLuaIsInBorderZoneBound`, `kLuaSquadronSetReleaseAltBound`,
`kLuaSquadronLandAndKillBound` (`game_hosts_lua.hpp:415/443/508/581/589`) and `kMissionCheckpointBound`
(`game_hosts_lua.cpp:312`). Each of them already has a body (`:6138`, `:6235`, `:7544`, `:6902`, `:6941`,
`:8697`). An OFF switch in front of a native that scripts call is treated as a live substitution, because
the native is then an unimplemented record that pushes nothing.

## Census

The census matched
`substitut|approximat|stub|simplif|deterministic|not in the struct|host constant|record-only|placeholder|in place of|no producer|stand-in|LABELLED|fixed value|unresolved`
(about 120 lines, about 40 of which are `*_unresolved` summary counters). It also took in every
`log_.unimplemented` call (34 sites), the six OFF switches, the GetProperty key set against the keys scripts
read, and the AddListener channels scripts subscribe to against the channels the host fires.

## Summary: MISMATCH REAL (each one sent to team-lead when confirmed)

| # | Host | Image | Live in a normal mission? | RNG |
|---|---|---|---|---|
| R1 | hpp `:508` `kLuaIsInBorderZoneBound = false` | IsInBorderZone 008AF020: 0071C4F0, then the zone side pushed (008AF20C) | jm06 `:996` and jm02 `:1233` fail the mission when a ship leaves the map; jm03/04/05/13, PC_cargo. No test row | none |
| R2 | hpp `:581` `kLuaSquadronSetReleaseAltBound = false` | 008A24E0 stores +39Ch / +38Eh; the dive profile 009C8920 / 009C7A80 reads +39Ch | USN01 test row `:621`, `:1055` (Vals, 700, force); usn_07, usn_12, bsm_09, competitive09 | none |
| R3 | `:6231-6234`, hpp `:443` TrulyDead | 00899500 pushes `Ptr == null`; 00928C80 nulls `Ptr` | 129 calls; the sampled sites choose an end-camera or fail entity | none |
| R4 | hpp `:589` `kLuaSquadronLandAndKillBound = false` | 008A20E0 frees the launch slots and lands and kills each plane | usn_03 `:1189` (PRCP01) | none |
| R5 | `:3174-3199` GenerateObject returns the live earlier instance | 0046D930 instantiates on every call | usn_03 `:1623` (PRCP01) wave loop; ijn_23, usn_19 and 16 more files | none |
| R6 | RemoveUntouchableUnit 008AC2B0 / IsUnitUntouchable 008AC420 unbound; label `:6798-6800` | 008AC3D6 `MOV byte [EAX+1D4h],0` | jm05 `:1242`, usn_02 `:636`, usn_03, usn_10, jm10, jm13, bsm_09 (23 calls) | none |
| R7 | `:7669-7676` SetFailure has no plane arm | plane vtable+194h: "enginefire" sends 65h | LOMP08 `:902`, LOMP09 `:1157` | image draws 1 on STREAM 1 (007BC0FC); host none |
| R8 | `:6702-6706` shipyard-built unit Skill 1 / OwnerPlayer 9 | the shipyard bag's Skill / Race / OwnerPlayer | wherever a shipyard builds and its authored Skill is not 1 | none |
| R9 | `generate` channel never fired | 00985590 from 0077FE50 (entity init) | JM08 test row `:1240` (built Mavis gets PilotMoveToRange) | none |
| R10 | `hpEvent` channel never fired | 00986B00 from 00879070 ApplyDamage | usn_05, usn_06, usn_08, usn_14 | none |
| R11 | GetProperty has no `TorpedoStock` arm (URGENT, torpedo) | 00815870 pushes +104Ch plus loaded tubes | JM08 test row `:749/:771`: the error aborts the US PT attack loop; 70 calls in 44 files | none |

### R1 IsInBorderZone is unbound

The ON body exists (`:7544`). It uses this host's own `border_bounds_` / `border_zones_` and
`bsp::point_outside_world_map_0071c4f0`. With the switch OFF, the row is an unimplemented record that
pushes nothing, so every call reads nil, which is the "inside" answer. That makes three mission-failure
checks unreachable: jm06 `:996` (the Lexington exits, `ijn06.obj_missionfailed_hornetexited`), jm02
`:1233` (the Prince of Wales exits, `ijn02.obj_missionfailed_2`), and the same checks in the PRCPIJN copies.
PC_cargo `:361` (a convoy retreats) and jm04 `:676` (the choice of spawn point) also read it.
docs/CC13_WIN_PATH_NATIVES.md section 6 item 4 asks for Lexington to be logged against the map bounds before
the switch is flipped.

### R2 SquadronSetReleaseAlt is unbound

The body (`:6902`) and its consumer (`game_hosts_units.cpp:4420-4437`, `sq_release_alt_set`) both exist.
In the image, 008A26AA stores +39Ch = altitude and 008A26A0 stores the force byte +38Eh. 009C8920 resets
+39Ch to 9999 only when +38Eh is clear, and 009C7A80 caps the dive altitude by +39Ch. USN01 passes force
`true` with 700 for "Val 1" / "Val 2" (`:621`) and at `:1055`, so in the image their dive altitude stays
capped at 700 for the rest of the mission; the host keeps 9999. USN04 `:2580` calls the native on level
bombers, but the only reader of +39Ch is the dive profile (CC13_WIN_PATH_NATIVES section 7, the disp32
scan), so it has no level-bomb effect there.

### R3 TrulyDead never answers true

Flipping `kLuaTrulyDeadBound` would not fix this. The ON body (`:6235`) reads `Ptr`, but nothing in the
host nulls it. `mission_entity_on_killed_00928c80` exists (`src/mission_entity_lua_attach.cpp:103`), but
nothing calls it. In the image, 00928C80 nulls `Ptr` when the kill list drains. The sampled readers are
commandhelpers `:6158` (the end-of-mission loser unit), jm05 `:2563` (`FailEnt`) and jm13 `:2878` (a
camera target). They change which unit is chosen, not the outcome.

### R4 SquadronLandAndKill is unbound

The body exists (`:6941`). usn_03 `:1189` (and its PRCPUS copy) keeps its extra squadrons flying.
Separately, the label at `:6936-6938` ("the gunnery host's kill ignores the cause, so the published
KillReason is not landed") is stale. `kKillFunnelCauseBound` is ON (`game_hosts_gunnery.cpp:447`), and
`kill_unit_00926d90` stores the normalized cause through `note_kill_cause_00926e02`
(`game_hosts_gunnery.cpp:15169-15173`).

### R5 GenerateObject returns the earlier instance

The DEVIATION at `:3174-3177`, narrowed by `kGenerateObjectRespawnsDeadBound`, returns the earlier handle
whenever that unit is not yet Dead. In the image, 0046D930 instantiates on every call. usn_03 `:1621-1636`
runs `for wave = 1, Max do {GenerateObject("Kate1"), GenerateObject("Val1")}`. The image makes Max pairs
of strikers; the host makes one pair and moves it once per pass. The periodic killers at `:1642` / `:1657`
have the same problem while an earlier pair lives. 19 mission files repeat a literal name, among them
ijn_23_def_fiji `:910/:917/:923` (FinalJP1..3, three times each), usn_19_coralus (Takao-class01 three
times, Minekaze twice), bulls_run, ijn_1_pearl, ijn_13, ijn_14, real_prelude, jm01 (PT, AIVal 0,
AIKate 0) and jm12. USN01's Wildcat (`:112`, `:1336`) is a respawn from its kill listener, which the Dead
test already covers.

### R6 Untouchable can be set but not cleared

AddUntouchableUnit is ON, and the gunnery gate reads it (`game_hosts_gunnery.cpp:6195`). Neither
RemoveUntouchableUnit nor IsUnitUntouchable has a row in this host or in the orders host. In the image,
008AC3AF calls 00888AA0 and 008AC3D0 makes the vtable[140h] call. The store at 008AC3D6 then clears the
byte. The label at
`:6798-6800` says no reference row calls RemoveUntouchableUnit, but 23 calls in 17 files do. One of them
is jm05 `:1242`: a captured PT boat is handed to the player and stays untargetable in the host. usn_06
`:1023` gates the call on IsUnitUntouchable, which reads nil here, so that unit is never released either.

### R7 SetFailure on a plane does nothing

The host routes only the mothership's runway and hangar arms and the ship arm. The plane's vtable+194h
handler is described in docs/CC13_GPR_47.md section 1.1. For "enginefire", it picks the engine index with
NextU32 on stream 1 (`007BC0FC`, ECX = 1) modulo desc+7Ch, and sends message 65h. LOMP08 `:902` uses it
for the damaged fortress that returns, and LOMP09 `:1157` for luaKamikazeEngineFire. The image draws once
on stream 1 for each call; the host does not draw. Stream 1 is not the shared stream 0.

### R8 Shipyard-built units take default bag values

`create_shipyard_unit_00844fc0` uses Skill 1, OwnerPlayer 9 and the SpawnNew Race default. The image
copies them from the shipyard's bag. `kShipyardProductionBound` is ON
(`include/bsp/shipyard_production.hpp:31`). The units differ only where a shipyard's authored Skill or Race
is not the default.

### R9, R10 Listener channels the host never fires

The host fires `kill`, `hit`, `recon`, `exitzone` and `entityKilled` (and `shipLanded` belongs to cc9-lua65).
It stores subscriptions on other channels and never fires them. docs/CC13_EVENT_CHANNELS.md gives the
dispatchers.
- `generate` (00985590), 17 subscriptions. JM08 `:1240` luaJM8PTGenerated gives each shipyard-built
  "Gyoraitei" PT `SetRoleAvailable(PLAYER_1)`, and each "Mavis" that too plus
  `PilotMoveToRange(unit, Mission.SpawnTo)`. It also counts both, but only the checkpoint reads the counts.
  USN04 `:2742`'s callback is empty.
- `hpEvent` (00986B00), 10 subscriptions: usn_05 `:1021`, usn_06 `:3080`, usn_08 `:604/:610/:648/:662`,
  usn_14 `:961/:2198/:2261`. The sampled callbacks are warnings and dialogue.
- Also never fired: `repair` (jm03, usn_02, usn_1_pearl), `command` (jm02/06/07/13; player orders),
  `surrender` (multiplayer only), `input` and `gui` (player UI), `ammoType`, `target`.

### R11 GetProperty(ship, "TorpedoStock") pushes nothing

`run_get_property_0088bf80` answers `unitcommand`, `reconlevel`, `ammoType` and the four deck keys. Any
other key returns no value. In the image, the eight ship vtables' +138h reader 00815870 runs 00779BB0 and
then, for `__stricmp(key, "TorpedoStock") == 0`, pushes `[unit+104Ch]`. When that spare is not negative it
pushes the spare plus 00810E90's loaded tubes instead. The gunnery host already keeps the spare
(`game_hosts_gunnery.cpp:1996-2000`, `torpedo_spare`).

In JM08, the US PT loop at `:742-752` calls `luaJM8Checktorp(unit)` (`:767-775`). There, `torp < 12` on nil
raises "attempt to compare nil with number". The error aborts the loop before `NavigatorAttackMove` and
`luaSetScriptTarget`, so with enemy ships within 3000 m no US PT receives its attack order. commandhelpers
`:14958` (the shipyard PT reload helper) raises the same error, and `:17697` (realistic stats) skips
`ShipSetTorpedoStock`.

Scripts also read the keys `WingCount` (3 calls) and `State` (1). They take the same no-value arm; their
image readers were not traced.

## Other census sites

| Host line | Image | Class | Note |
|---|---|---|---|
| `:167` objective 00888AA0 stand-in | 00888AA0 | MATCH | the host's identity |
| `:224` objective slot mask | 008CDEF2 | MISMATCH MINOR | one player record; the party arm is exact under `kLocalPartyFromSceneBound` (ON) |
| `:266` Kill script entity, OFF text | 00926D90 | MATCH | `kLuaKillScriptEntityBound` is ON |
| `:286` class row `ID` | 00964790 | MISMATCH MINOR | the ship's second recursion is not modelled |
| `:306`, `:8697` checkpoint id; `:312` switch OFF | 008ACB70 / 008ACA30 | MISMATCH MINOR | OFF: SaveCheckpoint pushes nothing, so commandhelpers `:16291` skips the "globals.checkpoint_r" narrative; LoadCheckpoint is nil on a fresh run in both |
| `:593`, `:908`, `:2948` simplified multipliers | 008B2320..008B2770 | MATCH | default 1.0 (00444D20); the forward is ON (`game_hosts_gunnery.cpp:357`) |
| `:686` nil tail | 0089903C | MATCH | |
| `:1243` Accel raw | 007D20F3 | STALE LABEL | the units host scales it (`kPlaneAccelCheatScaleBound` ON, `game_hosts_units.cpp:2438`) |
| `:3095` vector reader zero fill | 00888760 | MISMATCH MINOR | the image leaves uninitialised stack |
| `:3344` generated LandConvoy | 00743450 | MISMATCH MINOR | architectural; an unresolved Path keeps members standing |
| `:3521` refPos handle | 008889C0 | MATCH | the host's `ID` identity |
| `:3624` kill channel cadence | 009813A0 | MISMATCH MINOR | once per mission frame, ordered as in the image |
| `:3654` party-active walk | 0094C508 | MISMATCH MINOR | single player |
| `:3689` spawn placement never outside the map | 0071C4F0 in 00941D30 | STALE LABEL, behaviour REAL (conditional) | this same host holds `border_bounds_` (`:7562`); a SpawnNew member outside the map is accepted where the image retries or requeues; no run evidence |
| `:3714` ref frame | 008F8680 | MATCH | |
| `:3805` kind DEVIATION | 00948519 | STALE LABEL | the kind is now read under `kSpawnNewEntityRefPosBound` (ON) |
| `:3821` ship creator | vtable+28h(0) | MATCH | architectural |
| `:3887` Race 0 | 009420B9 | MISMATCH MINOR | an authored Race 0 is replaced; no call site authors one |
| `:4040` kamikaze rounds per frame | 006FEC00 | MISMATCH MINOR | sub-step timing |
| `:4204` `Class` stand-in | 009292B0 | MATCH | pass B writes it (`kSEntityInitThisTableStepsBound` ON) |
| `:4705` squadron node from the registry | vtable 00D087C0 | MATCH | |
| `:4725` leader has no SquadronID | 007C97E8 | MISMATCH MINOR | the host fuses squadron and leader |
| `:5000`, `:5010` GetProperty, no-slot entities and a director per slot | 00927AD0 | MISMATCH MINOR | paths and markers answer nothing |
| `:5061` reconlevel: "forced levels not modelled" | 00779C12 | STALE LABEL | the recon pass applies the forced table (`recon_sensor_pass.cpp:130`, `:206`); one pass late |
| `:5333`, `:5499` squadron Dead held to the last member | 007F3970 | MATCH | |
| `:5427` exitzone +361h latch | 007F31CF | MISMATCH MINOR | first exit batch only |
| `:5542` controller veto not held | 009594C4..009594FE | UNRESOLVED | the image skips `kill` when a kind-17h unit has +C41h set or a kind-6 unit has +100Ah set (00819A20 kamikaze detonate sets +100Ah); the host reports them |
| `:5591` credited slot | 0077CE60 | MISMATCH MINOR | |
| `:5627` attacker filter not matched | 0096ACE0 | STALE LABEL | dead arm, `kLuaKillAttackerBound` is ON |
| `:5641` second pass | 00981AC2 | MATCH | |
| `:5714-5752` recon listener steps | 0077B0C0 | STALE LABEL | forced levels are handled at `:5762`; the reset cycle is ON |
| `:5841` recon table publish cadence | 00806B10 | MISMATCH MINOR | |
| `:5976`, `:6085`, `:6195`, `:6285`, `:6336`, `:6375`, `:6458`, `:6541`, `:6655`, `:7011`, `:7108`, `:7232`, `:7423`, `:7735`, `:7775` no-slot entity counted unresolved | 00888AA0 | MISMATCH MINOR | the image would write through a non-unit or null object |
| `:6010` GetAttackTarget | 008A6DE0 | MISMATCH MINOR | slot 48h(2) is also true on the base director, so a non-ship, non-squadron unit with a director answers its fire target in the image; the host takes the command arm. Scripts pass ships and squadrons |
| `:6135` ForceGunnerMode (OFF) | 0089FA9C | MISMATCH MINOR | HUD-only reader; USN13 `:926` calls it |
| `:6170` IsClassChanged identity | 008CC5CB | MISMATCH MINOR | no frontend ship choice |
| `:6253` periscope state never 2 | 00893F40 | MATCH | accurate: the ship AI stores only 0 and 1 (`ship_ai_sub_attack.cpp`), and no periscope damage is modelled |
| `:6331` party preload mark | 0095BA60 | MATCH | no resource preload |
| `:6423` StartLanding2 non-ship | 0089AD50 | MISMATCH MINOR | |
| `:6503`, `:7146` leave at the call, slot counter not bumped | 0077C980 / 00905300 | MISMATCH MINOR | |
| `:6591` LaunchAirBaseSlot partial 006CDC70 | 006CDC70 | MISMATCH MINOR | |
| `:6810` untouchable for a marker | 008AC255 | MATCH | |
| `:6901` ReleaseAlt with no slot | 008A2692 | MATCH | |
| `:6927` LandAndKill substitutions | 008A20E0 | STALE in part | the KillReason text is stale (see R4) |
| `:7037`, `:7068` catapult index -1, stock = class value | 00953A60 / 0095E5B0 | MATCH | accurate: no catapult launch is built; SetCatapultStock (48 calls) is unbound, which only affects luaCheckpoint data |
| `:7174` unlimited air on a non-sub | 00893C00 | MISMATCH MINOR | |
| `:7199` AAEnable at the call | 0071E050 | MISMATCH MINOR | |
| `:7257` hit callbacks "with no argument" | 00988D87 | STALE LABEL | args are pushed (`:7381-7405`, `kLuaListenerCallbackArgsBound` ON) |
| `:7277` kamikaze attackType 11h | 00988510 | MISMATCH MINOR | no script filters on KAMIKAZE |
| `:7325` fireCaused / leakCaused compared as 0.0 | 00988510 | STALE LABEL | the gunnery host now fills `fire_caused` / `leak_caused` (`game_hosts_gunnery.cpp:11504`); no script in this installation sets a non-empty range |
| `:7384` callback target is the plane | 00988530 | STALE LABEL | `victim` is already the squadron (`kLuaHitTargetSquadronBound` ON, `:7293-7301`) |
| `:7457` effective level | 00805D02 | MISMATCH MINOR | |
| `:7498`, `:7540` vector reader, ID test | 008AECD0 / 008AF11E | MATCH | |
| `:7637-7640` SetFailure ship test, Explosion fields | 008132C0 | MISMATCH MINOR | non-ship arms: see R7 |
| `:7695` GetFailure Periscope / Runway / Hangar false | 008135C0 / 006D2350 | MISMATCH MINOR | every script reads only InferiorFailure (jm05/07/08/11) |
| `:7818` convoy kill | 00742210 | MISMATCH MINOR | |
| `:7828` "funnel ignores the cause" | 00926E02 | STALE LABEL | `kKillFunnelCauseBound` ON |
| `:8286` device +F4h = 0 | 00A08720 | UNRESOLVED (torpedo/submarine) | the AI torpedo plane never scores a submarine (`ai_target_weights.cpp:489`); the image reads past an E8h allocation, so its value depends on the heap |
| `:8656` lobby settings | 005E2F00 | MATCH | |
| `:8998` FindEntity case fold | 009251F0 | MISMATCH MINOR | an exact match wins here |

`log_.unimplemented` sites: the loaders `:1419` (gated on load), `:1432-1438`, `:1945-1951` (0087D7B0:
consumers read their own fields through per-field readers) and `:2679` (gated on `kReconPublishBound`, ON)
are records with no behaviour of their own: MATCH. `:2980` and `:6443` are null-object arms: MINOR.
`:4297` is the loading bar: MATCH. `:4407` / `:4442` (per-class virtual slots A0h / A4h in pass B and pass C)
are UNRESOLVED; their per-class bodies are outside this pass. `:4522` is the OFF arm: MATCH. `:4538`
00807A50 (the recon refresh after InitAll): MINOR, because a new unit is published one pass late. `:6352`,
`:6362`, `:6407-6409` and `:6624` are notices and peer messages: MATCH. `:9121-9123` is a stand-in that runs
only without the orders host: MATCH. `:9518`, `:9553`, `:9722` and `:9728` are records: MATCH.

## Not covered

- The per-class bodies behind 00926110 / 009261A1 (`:4407`, `:4442`).
- The image readers for the GetProperty keys `WingCount` and `State`.
- Mission lists for R8 (which shipyards author a Skill other than 1) and for the `:5542` veto (who sets
  +C41h).
