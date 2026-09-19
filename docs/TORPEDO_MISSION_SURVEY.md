# Which mission can exercise move-to, the glide slope and a release above the water

Addresses: 009D3210, 009D325B, 009C18C0, 009C1850, 00D05AC8.

Packet `cc8_torpedo_mission_survey`, owner `agent/cc8-torpedo-run-in`, on main `60f89a411` merged.
Read-only against this installation's game root; every path and date below is from
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`.

**Reachability is not the constraint.** The range at order time is, and getting it needs either one
run per candidate or a scene reader this reconstruction does not have. The recommendation is one
run, and the command is in section 4.

## 1. The harness reaches all 143 missions by id

USN01's own run prints:

```
summary mission select=USN01 tree=1 groups=5 missions=143 selected=USN01 list_page=5 detail=1
mission tree selection: group=2 mission=12 id=USN01 name=New - Marshall Islands Raid
                        scene=universe/Scenes/missions/USN/usn_1_marshall.scn
  group 1 IJN campaign  missions=30 ids: IJN01 IJN02 IJN03 IJN04 ...
```

so `--menu-select <id>` resolves against a 143-entry tree in five groups, and the ids are the
`missiontree.lua` `["id"]` strings. **Nothing about the harness limits the survey**; any of the 143
can be selected today.

`scripts/datatables/missiontree.lua` (dated **2 Jun 2025** in this installation, so locally
modified - the shipped datatables are dated 13 Jul 2024) maps scene files to ids. The three that
matter here:

| scene | id |
| --- | --- |
| `USN/usn_1_marshall.scn` | **USN01** |
| `USN/usn_19_coralus.scn` | **USN04** |
| `usn/usn_ormoc.scn` | **USN22** |

## 2. The order call, and who gets one

`PilotSetTarget(unit, target)` is the binding, and USN01 uses it at
`scripts/missions/usn/usn_1_marshall.lua:659`-`662` (dated 13 Jul 2024) on `Mission.MavisGang`,
which is the group the run's census reports as `Mav1`..`Mav5` with torpedo tasks installed.

Counting the call across this installation's base campaign, by the group it is given:

| group | calls | mission |
| --- | --- | --- |
| `unit` (a loop variable) | 95 | many |
| `Mission.FunryuGrp` | 32 | `ijn/` |
| `launchedStriker` | 12 | `usn/usn_19_coralus.lua` |
| `Mission.MainAttack` | 6 | `usn/usn_1_marshall.lua` |
| `Mission.MavisGang` | 5 | `usn/usn_1_marshall.lua` |
| `Mission.KatKillers` | 2 | `usn/usn_ormoc.lua` |
| `Mission.Avenger` | 1 | `usn/usn_ormoc.lua` |

The modded directories `COTP-USN`, `COTP-IJN` and `multi` carry more (`escort501.lua` alone has 42),
and they are excluded here because they are this installation's additions rather than the campaign.

## 3. The candidates

| mission | id | ordered torpedo aircraft | how they are ordered | why it may exceed 4840 m |
| --- | --- | --- | --- | --- |
| Marshall Islands | **USN01** | `Mission.MavisGang`, 5 | at a fixed line in the script, aircraft already placed | **No.** Measured: `range_first_mean = 1490.8 m`, inside the threshold from the first arm tick |
| Coral Sea | **USN04** | `launchedStriker`, 12 | **launched from `Mission.Zuikaku` and `Mission.Shokaku` slots and ordered at launch** (`usn_19_coralus.lua:1409`-`1446`) | **Measured: unreachable.** The strike never launches in this harness; see section 6. The only two ordered aircraft are placed dive bombers and no torpedo task is built |
| Ormoc Bay | **USN22** | `Mission.Avenger`, one `TBM_1`, at `Mission.FinConvoy[5]` | at a script line, against a convoy | **Measured: the mission does not start.** Scene load refuses after four units; see section 7 |

**USN04 was the recommendation, and section 6 records what the run found.** It is the only one of the three whose torpedo aircraft are ordered
*at the moment of launch* rather than from a placed position, which is exactly the geometry the
engaged test admits: the aircraft begins on a carrier deck and the target is wherever the enemy
force is, with no reason for that to be inside 4840 m.

## 4. The one run, and what it would show

```
./tools/run_game.ps1 -Log local\usn04_glide.log -- --frames 3200 --press-start-frame 30 `
    --menu-select USN04 --mission-frames 3000 --mission-frame-seconds 0.05
```

The single number that decides it is already printed by every run:
`summary mission pilot attack: ordered=N range_first_mean=... m`. Above **4840 m** the aircraft
occupy `moveto`, and then the glide census, the commanded speed pair, the release altitude and the
drop counters all follow in the same log.

**The run was not taken.** The startup crash on main past `48F293197` is still under the dive-bomb
bisect and no all-clear has arrived, and the safe-base method is no longer available now that this
branch's wiring is on main.

## 5. What could not be established, and what it would take

**The range at order time, for any mission but USN01.** It is the product of two authored
positions - the aircraft's and its target's - that live in the binary `.scn` scene files, not in the
Lua. This reconstruction has no scene reader, so the only ways to get it are:

1. **One run per candidate**, reading `range_first_mean`. Two runs settle the whole table. This is
   the cheap path and it needs nothing new.
2. A scene-file reader, which is a packet of its own and is not worth it for this question.

**The altitudes** have the same source and the same answer.

**One thing worth flagging to whoever runs it**: `AttackDist` is read from the tuning singleton, not
from a mission, so 4840 m is the threshold for every mission unless a mission's own script writes
`Pilot/Torpedo/AttackDist`. A grep of this installation's campaign scripts for `AttackDist` finds no
such write, so the threshold is uniform.

## 6. Measured: the USN04 run

Run of 2026-09-18, this worktree, `local/usn04_glide.log`, 3000 mission frames at 0.05 s, so 150 s
of mission time. Preconditions recorded before launch: two `bsp_game` processes belonging to the
`cc8-ai-squadron` worktree and the machine lock held by that owner, so the launcher waited; the run
itself started alone and exited 0 with `frames_presented=3199 loop_finished=1 exit_code=0`.

| measurement | value |
| --- | --- |
| `ordered` | 2 |
| `range_first_mean` | 0.0 m (degenerate: no target position at the first sample) |
| `range_last_mean` | 683.3 m |
| torpedo task | none built |
| `moveto` ticks, glide lines, dive-probe lines | 0, 0, 0 |
| water contacts, drops, breakups, swims | 0, 0, 0, 0 |
| plane step arms | `steps=6000 free_flight=6000 ground_roll=0 surface=0` |

**The mission never fields a torpedo aircraft in this harness.** The host prints it directly:

```
summary mission torpedo task: no ordered aircraft carries torpedo ordnance (kind 2Bh),
so 0099A170 builds no kind Eh task
```

The two ordered aircraft are placed dive bombers, and they get the kind 8 task at mission frame 62:
`dive-bomb task 009C8C70 kind 8 installed for an ordered aircraft`. `steps=6000` over 3000 frames is
exactly two aircraft, so the whole mission holds two, and the twelve `launchedStriker` aircraft that
section 3 counted never exist.

### Why the strike never launches

`MissionLuaNative::GetProperty` (`0088bf80`) is unimplemented in the host and returns a neutral
value. The carrier launch path reads the deck through it, in
`scripts/global/commandhelpers.lua:2495`-`2496` of this installation (mtime 2024-10-29, bulk install
date, not locally modified):

```lua
airbaseEnt.slots = GetProperty(airbaseEnt, "slots")
for idx, slot in pairs(airbaseEnt.slots) do
```

`nil` reaches `pairs`, so the mission's own think aborts:

```
script call Think failed: [string "scripts/global/commandhelpers.lua"]:2496:
bad argument #1 to 'pairs' (table expected, got nil)
stack traceback:
  [C]: in function 'pairs'
  [string "scripts/global/commandhelpers.lua"]:2496: in function 'luaGetSlotsAndSquads'
  [string "Scripts/missions/usn/usn_19_coralus.lua"]:538: in function <...:470>
```

That failure is recorded 41 times with an identical message, matching the 41 `GetProperty` calls, and
it is the mission think, so every gate downstream of it is dead: `IsReadyToSendPlanes`,
`LaunchSquadron` and the `PilotSetTarget(launchedStriker, bombertrg)` at `usn_19_coralus.lua:1411`
are never reached. Neither `LaunchSquadron` nor `IsReadyToSendPlanes` appears anywhere in the log.

The binary confirms the identity of the native: `0088bf80` is still `FUN_0088bf80` in the ledger, has
no recorded callers because the Lua registration table holds the pointer, and carries the literal
`luaMW_GetProperty failed:` in its body.

**What would unblock it.** One reconstruction: `0088bf80` returning, for the key `slots`, a Lua array
of tables each carrying a `squadron` field (nil for an empty slot), which is the only shape
`luaGetSlotsAndSquads` requires. That is a narrower request than section Follow-up item 3 and should
replace it if the Ormoc run also fails.

## 7. Measured: the USN22 run

Same command with `--menu-select USN22`, `local/usn22_glide.log`. The launcher waited on the machine
lock again and the run started alone. It exited 1, and the log says why:

```
startup failed: unit observer creator projection is unavailable
summary window_created=1 device_created=1 back_buffer=640x480 frames_presented=0
loop_finished=0 exit_code=1
```

This is not the startup crash under investigation elsewhere. There is no access violation: the host
raises `std::logic_error` from `src/game_hosts_units.cpp:2241` when
`publish_unit_leaf_observer_tables_for_creator` finds no row for a unit's creator. The refusal is
deterministic, so the one-retry rule for crashes does not apply and no retry was spent.

Four units registered before it stopped, the last being `Hangar1`, all of them `LandFort` (`00747000`)
or `AirField` (`006d3110`). The unit that follows has a creator that is not among the 22 rows in
`src/native_unit_observer_endpoint.cpp`, and the reason is one layer up: Ormoc Bay is a land battle.

| scene type | distinct rows | instances |
| --- | --- | --- |
| `Stationary` | 44 | 189 |
| `DestroyerGen` | 11 | - |
| `LandFort` | 6 | - |
| `PlaneSquadronGen` | 3 | - |
| `SubmarineGen`, `AirField`, `Path` | 1 each | - |

**RETRACTED.** This paragraph originally blamed `SceneContents::class_registration_creator`
(`004E5B00`), which the host records as unimplemented 192 times. `004E5B00` is a single `RET`
between `INT3` padding: a no-op stub shared by eight scene classes, and implementing it would
change nothing. The 192 records are records of a routine that does nothing. The real cause is one
layer further on and is set out in `docs/SCENE_STATIONARY_REGISTRATION.md`: a scene row typed from
`StationaryTypes` resolves to a `StationaryClass`, and this installation's 220 `StationaryClass`
rows carry no `Type` literal at all, so the vehicle-class chain at `00964790` returns a null
descriptor, the unit's creator is `0`, and the observer lookup for creator `0` throws.

USN04 starts because it has no stationary rows at all: its scene types are `DestroyerGen`,
`MotherShipGen`, `PlaneSquadronGen`, `TBoatGen` and `Path`, and `class_registration_creator` is never
called in that run.

## 8. Conclusion

Neither fallback reaches the torpedo move-to state, and neither fails on the geometry this survey set
out to measure. Both fail earlier, on a different unimplemented host method each:

| mission | blocker | native |
| --- | --- | --- |
| USN01 | starts inside the 4840 m engaged threshold (`range_first_mean = 1490.8 m`) | none; a real geometry result |
| USN04 | carrier strike never launches; mission think aborts every tick | `GetProperty`, `0088bf80` |
| USN22 | scene load refuses; a stationary unit has no vehicle-class descriptor | none; `004E5B00` was the wrong answer, see section 7 |

One named native came out of this, not two. `0088BF80` returning the `slots` table is real work
and is done: see `docs/MISSION_LUA_GETPROPERTY.md`. The USN22 half was wrong, as section 7 now
says, and Ormoc Bay needs a stationary unit path rather than a native body. Neither of those is
Codex's; both are in Claude-lineage files.

## Uncertainty

* The ranges and altitudes of section 3's candidates, as section 5 says.
* Whether USN04's `launchedStriker` groups contain torpedo-armed aircraft specifically is still
  open and now unanswerable from a run: the aircraft never spawn, so their classes never resolve.
  The script picks from plane types 158 and 162 (`usn_19_coralus.lua:1405`-`1406`), which are
  type ids, not names, and this survey did not resolve them.
* The modded `COTP-*` and `multi` missions were counted but not examined.

## Host methods

**None.** Read-only survey.

## Corrections

* **Retraction, section 7 and section 8.** `004E5B00` was named as the native whose absence stops
  USN22 loading. It is a one-byte `RET`, a no-op stub, and its 192 `UNIMPLEMENTED` records are
  records of a routine that does nothing. The real cause is a stationary class carrying no `Type`,
  in `docs/SCENE_STATIONARY_REGISTRATION.md`. The error was counting a summary's calls to an
  unimplemented record without reading what the routine is.
* Section 3's USN22 row credited `Mission.KatKillers` to Ormoc Bay. It is not there: the only
  definitions of `Mission.KatKillers` in this installation's campaign scripts are
  `usn_1_marshall.lua:954`-`959`, which is USN01. USN22's torpedo aircraft is `Mission.Avenger`
  alone, a `TBM_1` from `GenerateObject` at `usn_ormoc.lua:1287`, targeted at `Mission.FinConvoy[5]`
  on the next line. The group-call census that produced the row counted the name in the wrong file.
* Section 3 called USN04 the best candidate on the strength of its launch geometry. The geometry was
  never tested, because the launch does not run; see section 6.

## Correction, packet cc8_airops_launch_tick (2026-09-18)

**The launch runs now, so the Uncertainty item at line 231 is answerable from a run again.** That
item says whether `launchedStriker` carries torpedo aircraft is "now unanswerable from a run: the
aircraft never spawn, so their classes never resolve". Aircraft now spawn: the slot tick `006C0510`
and the creation seam are on this branch, and `local/usn04_tick.log` has four squadrons created as
real units with resolved class rows and ordnance in the gun census.

What that run measured, and what it did **not**:

* Four squadrons of three launched, from `Lexington-class01` and `Yorktown-class01` — the **American**
  carriers — with class **101**, and the ordnance census moved by exactly four units into
  `general_bomb` and by none into `torpedo` (2Bh). So the American strike is a dive-bomber strike,
  which is what those two carriers flew at Coral Sea.
* `Zuikaku-class01` and `Shokaku-class01`, which section 3 names as `launchedStriker`'s source, **did
  not launch in the 150 s window**. Its plane types, 158 and 162 by line 233, were never asked for,
  so the open item stands exactly as written — but it is now one run away rather than unanswerable.
* USN04 has six decks in all: the two American carriers, `Zuiho-class01`, `Zuikaku-class01`,
  `Shokaku-class01` and `dummylex`, each with `NumSlots=4 MaxInAirPlanes=12`.

The blocker is no longer the launch. It is that the mission's own timetable had not reached the
strike: `script entity 100006 created_for=luaDoTimeTable think=luaTimetable armed=1 delay=34.95
thinks=0`, the same line as in the before log. A run of `--mission-frames 6000` is what settles
whether 158 and 162 carry 2Bh. Section 8's USN04 row, "carrier strike never launches", is
superseded. docs/AIROPS_LAUNCH_TICK.md.

## no_ghidra_function

None.

## Validation

Two runs. USN04, `local/usn04_glide.log`, exit 0, summarised in section 6. USN22, `local/usn22_glide.log`, exit 1 on a deterministic scene-load refusal, summarised in section 7. Not game-validated beyond
the harness: the measurement is of this reconstruction's behaviour, not the retail game's.

## Follow-up

Items 1 and 2 are done; sections 6 and 7 hold their results. What remains is item 3, and the two
runs have made it specific.

1. **`MissionLuaNative::GetProperty` (`0088BF80`) is done**, in
   `docs/MISSION_LUA_GETPROPERTY.md`. It stops USN04's mission think aborting. It does not launch
   the strike: `IsReadyToSendPlanes` and `LaunchSquadron` both need an air-operations block this
   process does not build, and `006CADD0 BSP_AirOps_LoadFromScene` is what would give all three
   real data. The `004E5B00` half of this item is withdrawn; see section 7 and
   `docs/SCENE_STATIONARY_REGISTRATION.md`.
2. **The general capability is still worth having** and is unchanged from the original wording: a
   harness switch that issues `PilotSetTarget` to a named unit at a chosen frame, or a scene-file
   reader that reports authored positions, would let this reconstruction exercise `moveto` without
   depending on any mission's authored geometry. That is the only option that does not wait on
   another harness's work.
3. **Do not re-run USN04 or USN22 for this question** until one of the above lands. Both failures are
   deterministic and cost about fifteen minutes of the shared game lock each.
