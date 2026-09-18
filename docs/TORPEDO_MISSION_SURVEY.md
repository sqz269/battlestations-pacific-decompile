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
| Coral Sea | **USN04** | `launchedStriker`, 12 | **launched from `Mission.Zuikaku` and `Mission.Shokaku` slots and ordered at launch** (`usn_19_coralus.lua:1409`-`1446`) | **Best candidate.** A carrier-launched strike is ordered the moment it leaves the deck, and carriers stand off from the enemy force |
| Ormoc Bay | **USN22** | `Mission.Avenger` at `Mission.FinConvoy[5]`; `Mission.KatKillers` | at a script line, against a convoy | Possible; the convoy is a separate force, so the order range may be long |

**USN04 is the recommendation.** It is the only one of the three whose torpedo aircraft are ordered
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

## Uncertainty

* The ranges and altitudes of section 3's candidates, as section 5 says.
* Whether USN04's `launchedStriker` groups contain torpedo-armed aircraft specifically; the script
  names them only through the carrier's `slots` squadron property, and the class is resolved at run
  time.
* The modded `COTP-*` and `multi` missions were counted but not examined.

## Host methods

**None.** Read-only survey.

## Corrections

None.

## no_ghidra_function

None.

## Validation

No run; section 4 says why and gives the command.

## Follow-up

1. **Run USN04 once** when the crash is cleared. If `range_first_mean` exceeds 4840 m, everything
   this thread has built downstream of `moveto` becomes reachable in one pass.
2. **USN22 as the fallback**, same command with the other id.
3. If neither exceeds the threshold, the request to the Codex side is narrow and is stated here so
   it can be lifted whole: **a way to place or order an aircraft at a chosen range for a test**, for
   example a harness switch that issues `PilotSetTarget` to a named unit at a chosen frame, or a
   scene-file reader that reports authored positions. Either would let this reconstruction exercise
   `moveto` without depending on a mission's authored geometry.
