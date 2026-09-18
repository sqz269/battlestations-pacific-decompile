# Which mission orders a dive bomber (packet `cc8_dive_bomb_mission_survey`)

The analogue of `docs/TORPEDO_MISSION_SURVEY.md`, whose method this follows: the harness reaches
all 143 missions by id through `--menu-select`, so nothing about the harness limits the survey.

**Result: yes.** USN04 orders a dive bomber, the chooser gives it `00E08F20`, and a run of it
installs a real kind 8 task. The census then stops at a gate this survey could measure to the metre.

## 1. What the chooser needs

`007EE9C5` gives `divebomb` `00E08F20` to a surface target when the unit does **not** answer
`IsKindOf(10h)` (level bomber, which `007EE946` claims first), carries general bomb ordnance
(`007ED7E0` -> `007B9320`, kind `2Ah` excluding `2Ch`/`31h`/`2Bh`/`33h`/`2Dh`), and the target does
not answer `IsKindOf(0Eh)`. So the survey is for `PilotSetTarget` calls whose **unit** is a dive
bomber and whose **target** is a ship.

This installation's dive-bomber class ids, from `usn_19_coralus.lua`'s own constant block:

| id | class |
| --- | --- |
| `158` | `D3A` Val |
| `159` | `D4Y` Judy |
| `38` | `SB2C` Helldiver |
| `332` | `SBD` Dauntless |
| `162` | `B5N` Kate, a **torpedo** bomber, listed because it is the coin-flip partner below |

## 2. The calls, by mission

`PilotSetTarget` appears in 36 base-campaign mission scripts. Most calls pass a loop variable over a
fighter or torpedo group. These are the ones whose ordered unit is a dive bomber:

| mission | id | call | ordered unit | target | how |
| --- | --- | --- | --- | --- | --- |
| Coral Sea | **USN04** | `usn_19_coralus.lua:1411`, `:1446`, `:1483`, `:1518`, `:1557`, `:1594` | `launchedStriker`, from `LaunchSquadron(carrier, luaPickRnd(planeTypes), 3)` where `planeTypes` is `{158 Val, 162 Kate}` at four sites and `{159 Judy, 162 Kate}` at two | `luaPickRnd(luaRemoveDeadsFromTable(Mission.USCVs))`, a US carrier | **at the moment of launch**, from `Mission.Zuikaku` and `Mission.Shokaku` slots |
| Coral Sea | **USN04** | the scripted `movieval` | a Val placed for the opening | `Mission.ConLeader`-style ship | at `luaStageInit` |
| Marshall Islands | **USN01** | `usn_1_marshall.lua:711` | `Mission.ScoutBomba = GenerateObject("ScoutDauntless")` | `Mission.ConLeader` | at the **phase 2** trigger (`luaMoveToPh2`), and the aircraft is `SetInvincible` |

Everything else the campaign orders is a fighter, a torpedo bomber or a level bomber, or targets
another aircraft, which sends the chooser to `dogfight` rather than `divebomb`.

The modded directories `COTP-USN`, `COTP-IJN` and `multi` carry more calls and are excluded, as the
torpedo survey excludes them: they are this installation's additions rather than the campaign
(`docs/GAME_EXECUTABLE.md` on the modded install).

## 3. The recommendation, and why

**USN04.** Six of its launch blocks order a striker the instant it leaves a carrier deck, and each
block picks its type at random between a dive bomber and a Kate, so roughly half of them are the
class this task wants. That is the geometry the engaged test admits: the aircraft starts on a deck
and the target is wherever the enemy carriers are.

USN01's Dauntless is a real dive-bomb order but a poor test. It fires only at the phase 2 trigger,
which a 3000-frame run never reaches, and the aircraft is made invincible, so it is scenery.

## 4. The run

```
./tools/run_game.ps1 -Log local\usn04_dive.log -- --frames 3200 --press-start-frame 30 `
  --menu-select USN04 --mission-frames 3000 --mission-frame-seconds 0.05
```

`EXITCODE=0`, 3199 frames presented. Environment before the run: two `bsp_game` processes and the
lock held by `cc8-ai-squadron`, so the launcher queued and then ran, which is the atomic lock of
`d8dfc77be` working.

**A dive-bomb task exists.** `usn04_dive.log:3199`-`3205`:

```
PilotSetTarget choose: 007EEC50 -> 00e08f20  (weapon_controller=1 ordnance lb=0 gb=1 dk=0 torp=0, sides 1/0)
PilotSetTarget task: 0099A170 -> 1 (unit=movieval command=00e08f20 target_token=1 refusals=0)
```

`self_class=18`, `level_bomber=0`, `gb=1`: exactly the arm `007EE9C5` describes. The command table
carries the row `movieval  divebomb  script:PilotSetTarget`.

The census, `usn04_dive.log:28033`-`28035`:

| measure | value |
| --- | --- |
| aircraft on the task | 1, `movieval` |
| arm ticks | 1470 |
| transitions | 1 |
| states | `attackrun` 1470 |
| releases | 0 |
| `approach+BCh` | **1169.5 m** |
| `approach+B8h` | **1100.0 m** |
| `approach+D0h` latch | 0, for all 1470 ticks |
| `approach+D1h` bombs | 1, for all 1470 ticks |

## 5. The next gate, to the metre

The machine gets further than it ever has. The class gate passes, the task installs, the entry
chooser takes the `attackrun` arm at `009C83AE` because the latch is clear, and the aircraft holds
`attackrun` for the whole run.

**It is 69.5 metres short.** The in-range latch at `009C7C31` arms when `approach+BCh` drops below
`approach+B8h`, and the measured pair is `1169.5` against `1100.0`. Nothing closes that gap, because
the state that would fly the run-in, the **attackrun tick `009C4220`** (vtable `00D20C68` slot
`+Ch`, Ghidra body `009C4220`-`009C447D`), is not bound in this host. The pilot planner's yaw arm
steers at `command_target_plus_one`, which is the unit's `moveto` row rather than its attack target,
so the aircraft flies its movement order and the attack range never closes.

So the gate is no longer the order path, and no longer the class. It is one unbound tick:

> Bind `009C4220` and the latch arms; the entry chooser then takes `flyabove` and the roll-in,
> turndown, aimdive chain this packet already reconstructed has its first live inputs.

## 6. What this changes about the injection request

The request to the Codex side stands but is **no longer blocking**. USN04 reaches a real kind 8 task
from the campaign's own script, so the dive-bomb chain can be exercised without a new switch. A
`--pilot-set-target unit:target` switch would still help, because it would let a run put a dive
bomber at a chosen range instead of waiting on a mission's own geometry, but it is now a
convenience rather than the only route.

## Uncertainty

The six `launchedStriker` blocks were not observed firing: a 3000-frame run is 150 s and the
carriers had not launched by then. Their coin flip means a longer run gives dive bombers about half
the time. The single task measured here is the scripted `movieval`, which is enough to prove the
path and to measure the gate, but it is one aircraft rather than a strike.

`movieval` is a cinematic unit. Whether the mission intends it to attack or only to fly past was not
established.

## Validation

`./scripts/build.ps1` succeeds; `ctest` passes both suites. One run, `local/usn04_dive.log`,
`EXITCODE=0`, 3199 frames presented, environment recorded above. Ghidra was not mutated.

## Follow-up

1. **Bind `009C4220`**, the attackrun tick. It is the whole remaining gap between this task and a
   bomb, and unlike the flyabove tick it has a Ghidra function already.
2. A longer USN04 run, past the carrier launch window, to see the six striker blocks fire and give
   the task a squadron rather than one aircraft.
3. USN01 past its phase 2 trigger, for the `ScoutDauntless` order, if a run that long is ever cheap.
