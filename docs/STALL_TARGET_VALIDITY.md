# The stalled fighters were chasing a corpse (packet cc9_stall_target_validity)

2026-09-25. Ghidra read-only. Names are hypotheses. The question
(docs/FIGHTER_ROLL_TRACE.md section 4): which target do the Yorktown fighters close on when aim's
speed arm cuts their throttle? Is it one the image would also chase, or one it would have dropped?

## 1. The target at closure

- **The build.** One diagnostic build (`local\stv`): the landed switches, plus
  `kRateLawAttitudeTermsBound` ON, plus `kFighterChaseTraceDiag`. The chase trace now also names the
  target's class, side, liveness (`df_slot_live`), death latch (`plane_death_c3a`),
  `simulate` flag, flight mode, throttle, task flags and states, and commanded speed. The diagnostic
  is committed OFF.
- **The run.** `local\STV_9000.log`, E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`,
  window `2560x1440 -> 1600x900`. The same two depth kills as `local\RTD_9000.log`, both Yorktown
  sqn04.

Yorktown-class01_sqn04|.-3, whose target throughout is **D3A Val #7.1|.-4**:

| t (s) | state | range (m) | own speed | target speed | target alt | throttle | commanded speed | target live / death / simulate |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 296.9 | maneuver | 469 | 60.1 | 39.22 | 234.6 | 1.0 | 120 | 0 / 1 / 1 |
| 306.9 | aim | 719 | 87.8 | 39.22 | 234.6 | 1.0 | 457.9 | 0 / 1 / 1 |
| 310.9 | aim | 385 | 82.8 | 39.22 | 234.6 | 1.0 | 124.2 | 0 / 1 / 1 |
| 312.9 | aim | 234 | 60.3 | 39.22 | 234.6 | **0.0** | **-26.5** | 0 / 1 / 1 |
| 318.9 | aim | 27 | 21.0 | 39.22 | 234.6 | 0.0 | -233.8 | 0 / 1 / 1 |
| 326.8 | aim | 215 | 29.0 | 39.22 | 234.6 | 0.0 | -45.8 | 0 / 1 / 1 |

**The target is dead.**
- D3A Val #7.1|.-4 was shot down by Northampton-class03's AA at 264.26 s (death row, explosion mode,
  `entity dead`).
- It left its squadron at 264.31 s with `live=0`: the squadron is now empty.
- Its position and velocity stay frozen at the moment of death: 234.6 m and 39.22 m/s, unchanged for
  60 s.
- The fighter's aim closes on that frozen point. The commanded speed, `(d - FollowDist) + target
  speed`, goes negative, the throttle falls to 0, and the fighter stalls into the sea.

## 2. Why the host keeps the target and the image drops it

- **The host.** `df_approach_update_009aac70` keeps the order's squadron after its planes die
  (`find_by_member_or_departed_unit`). When the current target is not live it calls the re-select,
  and `df_select_009aa630` **returns at `n == 0`**, the empty squadron, leaving the dead target in
  place.
- **The image.** `009AA630` tests `squadron+3CCh <= 1` at `009AA66B`, then calls `009A7650`
  (SetTarget) with `+3D0h[0]` (`009AA678`-`009AA685`). `BSP_Squadron_RemovePlane` `007F3970` shifts the
  remaining members down and then **nulls the vacated slot** (`007F39ED` decrements `+3CCh`,
  `007F39FA` stores 0 at `+3D0h[count]`). So for an emptied squadron `+3D0h[0]` is 0, and the image
  clears the target.
- **The live-candidate filter matches the host.** It skips a candidate unless +5Ch is set and
  +5Dh/+60h/+5Eh are clear (`009AA6E1`-`009AA71D`), and member 0 is the fallback
  (`009AA9B1`-`009AA9D1`). The one untested term is `0071C4F0`, the outside-the-map filter at
  `009AA72E`. It is answered false in the host (labelled) and is not involved here.

**Verdict: outcome (b).** The image would have dropped this target. The stall comes from a host
validity gap, not from the image's regime.

## 3. The binding

`kDogfightEmptySquadronClearBound` (`src/game_hosts_units.cpp`, `df_select_009aa630`): with the
target squadron empty, the dogfight target is cleared (`df_target_plus_one = 0`, logged as
`target -> none (empty squadron)`). The approach update then takes its `cur == nullptr` path.

## 4. Predictions for the pair, written before the runs

The pair has two sides, one tree (main `44631c067` plus this commit):
- **OFF:** `kDogfightEmptySquadronClearBound` and `kRateLawAttitudeTermsBound` both OFF;
- **ON:** both ON.

Everything else is as landed: the integrator, the throttle wiring and the DummyAI gate are ON. E2
9000, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | OFF | ON prediction |
| --- | --- | --- |
| **US fighter depth kills and sea contacts (the gate)** | 0 | **0-1** |
| `target -> none (empty squadron)` events | 0 | at least 1 |
| US fighter losses | 0-1 | 0-4 |
| Kate death rows with fighter hits | 0 | at least 1 |
| fighter kills | about 5 | 4-20 |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | 450-750 | 450-750 |
| torpedo / dive-bomb releases | 2-8 / 0 | 2-8 / 0-4 |
| Lexington moved | 5.5-7.5 km | 5.5-7.5 km |
| mission end | none | none |

**Flip rule.**
- If the sea-loss row holds, both switches land ON.
- If it misses, both stay OFF and the loss is traced again: the chase trace names the target.

## 5. The pair, measured

`local\SV_OFF_9000.log` (binary `local\sv_off`) and `local\SV_ON_9000.log` (`local\sv_on`), one tree,
module directory checked, window `2560x1440 -> 1600x900` in both.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| **US fighter depth kills and sea contacts** | **2** (Lexington sqn01 and its .-3) | **0** | 0-1 | **held** |
| `target -> none (empty squadron)` events | 0 | 55 | at least 1 | held |
| US fighter losses | 2 | 0 | 0-4 | held |
| Kate death rows with fighter hits | 1 | **0** | at least 1 | **missed** |
| Kate deaths by category (0 / 1 / 5 / 6) | 1 / 7 / 2 / 6 | 0 / 9 / 0 / 7 | - | - |
| fighter kills | 9 | 9 | 4-20 | held |
| fighter bursts / fire ticks | 13 / 280 | 15 / 287 | - | - |
| Zero depth kills (A6M) | 2 | 0 | - | - |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 each | held |
| hit records | 553 | 586 | 450-750 | held |
| torpedo / dive-bomb releases | 6 / 0 | 6 / 2 | 2-8 / 0-4 | held |
| Lexington moved | 6648 m | 6616 m | 5.5-7.5 km | held |
| plane distance moved | 1,085,244 m | 1,168,477 m | - | +7.7% |
| mission end | none | none | none | held |

**The OFF side already drowns two fighters.** The OFF side is main as landed: the turn chain OFF,
and the integrator, the throttle wiring and the DummyAI gate ON. It loses Lexington-class01_sqn01 and
its .-3 to the sea at 432 s, with no damage, and without the slide term. Their targets were not
traced in this run, so a corpse chase is the likely cause but not a proven one. Nothing in the host's
re-select limits the corpse chase to the turn chain.

**The Kate row missed.** With both switches ON, no Kate takes a fighter hit in this run. The earlier
KE1b pair had 4. Which targets the fighters meet is RNG- and path-coupled. The 55 target clears also
change who pursues whom after each squadron is destroyed. The fighter kill total is unchanged at 9.

The Zeros' two depth kills also vanish in the ON run. Their mechanism is separate (section 5 of
docs/FIGHTER_ROLL_TRACE.md), so this is coupling, not a fix. The packet `cc9_taskless_plan_arms`
takes them up.

## 6. Verdict

`kDogfightEmptySquadronClearBound` and `kRateLawAttitudeTermsBound` land **ON**. The gate row held:
0 US sea losses, against 2 on the OFF side. Every headline row holds its band. The Kate-hit row missed
and is recorded as RNG-coupled.
