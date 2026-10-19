# cc13-jm08end: why JM08 ends with no result on current main

Packet cc13-jm08end, from cc9 priority 2 (SHIP_AI_OPEN_ITEMS sections 302 and 304). Read-only,
no game runs. Names are hypotheses, not recovered symbols.

## Verdict

**No host gap blocks the end. The script waits on two kill conditions that the scripted player
does not meet within the run.**
- At the end of the deterministic current-main run, the USS Missouri and five landing craft are
  still afloat: LST 01, LST 02, LST 03, LSM 01 and LSM 02.
- So primary objectives 1 and 2 never complete. The second phase, which holds the win and the
  usual loss, never starts.
- Every link of the script's chain does work in this host. The same build with a different
  random-stream setting runs the whole chain to a result: all targets die, phase 2 runs, and the
  mission ends with "Mission Failed - The HQ has been destroyed!" at 1476.36 s.
- The visible difference is the scripted torpedo-boat strike. All 12 Gyoraitei boats die by
  703 s. Five of the deaths are to the US shore bunker "Medium Bunker, Concrete 05" at about
  500 m (four boats and Japanese Patrolboat 01). Only one boat launches, two torpedoes; the
  nearest passes 35.7 m from the Missouri. In older completing runs the Missouri sank at about
  365-405 s, and that bunker killed no boat.

Nothing is proposed for a switch. One item for the gunnery lane is listed under Next.

## The script

**Which script.** The run log's mission tree line maps JM08 to the scene
`universe/Scenes/missions/COTP-IJN/PRCPIJN/prcpijn_08_defend_guadalcanal.scn`. Its script is
`scripts/missions/COTP-IJN/PRCPIJN/jm08.lua` in this installation (32,355 bytes). The two other
jm08.lua files, under COTP-IJN and ijn/JM, belong to other scene trees.

**Objectives (`this.Objectives`, lines 119-171):**

| kind | # | id | condition | added at |
| --- | --- | --- | --- | --- |
| primary | 1 | landingships | `luaRemoveDeadsFromTable(Mission.LandShips)` empty: 6 USTroopTransports, LST 01-03, LSM 01-02 (`CheckPrim1`, 727) | `luaIntroMovieEnd` 631 |
| primary | 2 | Missouri | `Mission.Flagship.Dead` (`CheckPrim2`, 747) | 630 |
| secondary | 1 | Nevada | `Mission.Nevada.Dead` (`CheckSec1`, 738) | `NevadaMovie` 934 |
| hidden | 1 | Hosho | the Hosho alive when primary 3 completes (`CheckPrim3`, 1300, completed at 1303) | `HoshoMovie` 952 (no unit, so `units=0` in the log is expected) |
| primary | 3 | bombing | every Dakota (class 136) of `luaBomberWave3` dead (`CheckPrim3`, 1300, completed at 1305) | `Phase2Movie` 1283 |

**Win.** The chain runs as follows:
1. `CheckCompletion` (790) polls each second until `luaObj_GetSuccess("primary",2)` and
   `("primary",1)` are both true.
2. It then plays the dialog "Run". Its callback `FadeIn` leads to `PreparePhase2` (1005), which
   starts `luaBomberWave1`, and wave 1 chains to waves 2 and 3, 180 s apart.
3. `Phase2Movie` adds primary 3.
4. When the wave-3 Dakotas are all dead, `CheckPrim3` calls `Victory`, which calls
   `luaMissionCompletedNew(Mission.CommandBuilding, "Mission Completed - Guadalacanal is safe
   from invasion!")` (1313).

**Lose.** `CheckHQ` (1320) polls `Mission.CommandBuilding.Party == 2`. When it holds, `Fail`
calls `luaMissionFailedNew(..., "Mission Failed - The HQ has been destroyed!")`. The Dakotas of all three waves target `Mission.CommandBuilding` (`PilotSetTarget` at 1077, 1178 and 1272; `Mission.CommandBuilding` is `Headquarter 01`, line 109). A landing capture could also flip the
HQ.

**There is no timer end.** The script has no time limit.

## The runs (cc9-physics1 local\, the newest JM08 runs, all with config/win_orders/jm08_l65_j8_p2.txt, 30000 frames = 1500 s)

| log | end | objectives completed (`Objectives_Completed` bindings, status `sets`) |
| --- | --- | --- |
| p24s3_j8_def.log (deterministic, no `BSP_GUNNERY_RNG_STREAMS`) | none (`Mission.EndMission never true`) | Nevada only; sets=1 |
| p24s3_j8.log (per-consumer streams ON, a measurement substitution) | failed at 1476.36 s, "The HQ has been destroyed!" | Nevada, Missouri, landingships; sets=5 |
| p7a1_j8.log (older main) | completed at 1603.74 s | Missouri, Nevada, landingships, bombing; sets=4 |

**When the targets left the world.** local/j8_last_step.py takes each unit's last `ship ai step`
line and its first "dead or hidden" refusal:

| unit | p24s3_j8_def | p24s3_j8 | p7a1_j8 |
| --- | --- | --- | --- |
| Missouri | alive at 29850 (attackmove) | dead by 16301 | dead by 7500 |
| USTroopTransport 01-06 | all dead by 17200 | all dead by 17100 | all dead by 20700 |
| LST 01, 02, 03 | alive at 30000 (attackmove) | dead by 16400 / 15900 / 13900 | dead by 10700 / 17200 / 24400 |
| LSM 01, 02 | alive at 30000 / 29960 | dead by 16000 / 15300 | dead by 10400 / 21600 |

**The HQ.** The capture summary of p24s3_j8_def reads `health_zero=0 neutralized=0`. In
p24s3_j8 the phase-2 bombers neutralize the HQ (`health_zero=1 neutralized=1`). The party Lua
mirror is bound (`kCommandBuildingPartyLuaMirrorBound`, game_hosts_ship_ai.cpp,
`mirror_capture_party_00928f50`; SHIP_AI_OPEN_ITEMS section 158 had found it missing), so
`CheckHQ` fires.

**The landing capture never contributes.** No run has a landing: `landing ship ramp
ground_contacts=0 lowers=0 landed_capture_adds=0` in all three. The landers do get the HQ as
their target in the current runs (379 `command target ... LST n token="Headquarter 01"` lines).

## The pending condition, traced to the host

**`CheckPrim2`'s `Mission.Flagship.Dead` and `CheckPrim1`'s `luaRemoveDeadsFromTable`.** Both read
the unit's Lua `Dead`. The host sets it on death: every transport's death is seen by the same
polls, and p24s3_j8 and p7a1_j8 complete both primaries through them.

**`luaObj_Completed` / `luaObj_GetSuccess`** run through `Objectives_Completed` (008BD340 /
008BD900, packet cc9_objectives_completed). The Nevada completion goes through it in the
deterministic run.

**So the conditions are live and correctly bound.** They stay false because the Missouri and the
five landing craft are not sunk.

**Why the Missouri survives the deterministic run.** The scripted player's main weapon against it
is the torpedo-boat group. The `death row` and `torpedo launch` lines:

- **p24s3_j8_def.** The 12 Gyoraitei die between 452.86 s and 703.00 s:
  - Medium Bunker, Concrete 05: 4 (#Y5, #Y8, #Y9, #Y11; range 503-510 m, gun category 1, about
    80 hits each);
  - Missouri: 4;
  - Wildcat2, Helena, Auilick and Stephen Potter: one each.

  The bunker also kills Japanese Patrolboat 01 at 329.14 s. One boat launches (#Y7, two
  torpedoes at 625.67 s and 628.47 s). Its nearest pass is 35.7 m at the end of a 15.85 s run.
- **p7a1_j8.** #Y2 launches four torpedoes at 362.49-364.58 s. Two runs from #Y4 and #Y3 pass
  16.0 m and 26.0 m from the Missouri, which sinks by frame 7500. Only 6 boats die, none to a
  bunker. That run's only bunker kill is Static Mavis 02 at 14.10 s.
- **The bunker is on the US side.** The local-player unit list shows the island bunkers with
  party 0, the side of the US ships in the same list. Its fire on the boats is enemy fire, not
  friendly fire.

**The helm orders are mostly refused in every run, completing ones included.** The orders file
is frame-timed (local/j8_orders.py):
- **p24s3_j8_def:** launch 34 applied / 27,718 refused; attack 879 / 6,517.
- **p7a1_j8:** launch 32 / 24,736; attack 2,157 / 4,455.

The refusal reasons are the same kinds in both: slot not ready, target dead or not yet spawned,
not the controlled unit. So the harness drives the same small subset of orders. The outcome
turns on the battle, not on a refused native.

## Next (not built)

- **Gunnery lane.** Check whether the image's land fort guns (bunker class, kind 27, gun
  category 1) engage a fast boat at 500 m as this host's do. No completing run shows a bunker
  kill of a boat. If the host now engages where it did not before, a recent gunnery switch is the
  first suspect.
- **The landing capture** never lowers a ramp in any run (SHIP_AI_OPEN_ITEMS section 91 covers an
  earlier form of this). It does not decide the end here.
- **Run length.** A deterministic run longer than 1500 s would show whether the Missouri and the
  landing craft die later, or whether the mission sits until the frame cap. SHIP_AI section 302
  reports no end in 72000 frames for this orders file with every switch OFF. That run's log
  (cc9-ships60) no longer exists.

## Status

Read-only. Evidence from logs and the installation's script. No code change.
