# Handoff: six USN04 torpedoes are dropped and none of them ever swims

Written by packet `cc8_torpedo_retire` at its context limit. Everything below is measured or read
from this repository; nothing here is a conclusion about why. The run is
`local/usn04_kates.log` (USN04, `usn_19_coralus.lua`, 3000 mission frames, on the tip that is now
main).

## The measurement

The torpedo task chain runs correctly on the `SpawnNew` Kates - `moveto 642 -> attackrun 319 ->
aim 182 -> goaway once each`, `009D4C10` live, six aircraft releasing - and then:

```
gunnery: torpedo drop 1 by B5N Kate #2.1 at 12 m, speed 80.9 m/s, bullet 69, swim 30.9 m/s   (x6)
summary mission gunnery torpedo_drop  drops=6 refusals=0 water_entry_breakups=0
summary mission gunnery torpedo_ranges_derived=34 swims_started=0 snaps=0
summary mission gunnery torpedo_closest_approach swims=0
summary mission gunnery projectiles created=492 steps=25614 sweeps=25614
                        entity_impacts=21 water=32 expired=417 in_flight=22
```

USN01 starts five swims from five drops through the same code (`local/retire_tip_usn01.log`), with
its aircraft at 73.1 m/s at the drop against these 80.9 m/s.

**The closest-approach census being empty is a consequence, not a second defect**: it iterates
`if (!row.swimming) continue`, so with no swim there is nothing to print.

## Where the swim is supposed to start

`src/game_hosts_gunnery.cpp`, in the projectile step's water handler. Two arms in order:

1. the break-up arm - `if (swim > 0.0f && !shot.swimming && hit_limit > 0.0f && entry_speed >
   hit_limit)`, where `hit_limit` is the bullet entry row's `max_water_hit_vel` (`008568E0`). It
   increments `summary.water_entry_breakups` and kills the round;
2. the swim arm - `if (swim > 0.0f && !shot.swimming) { shot.swimming = true; ... }`, which levels
   the round onto the surface plane at the swim speed and increments
   `summary.torpedo_swims_started`.

`water_entry_breakups=0`, so **arm 1 did not remove them**: `MaxWaterHitVel` is not what happened,
and the 80.9-versus-73.1 m/s difference is not by itself the answer. `swims_started=0` says arm 2
never ran either. So the six rounds never reached this handler at all, and the question is what
became of them between the drop and the water.

## What the next worker should do, in one instrumented run

Follow those six rounds specifically, by the id on their drop lines:

* position, velocity and `alive`/`swimming` per tick until each one disappears;
* which arm took it - life expiry (there are 417 expired), the `water=32` counter, an entity impact
  (21), or leaving the world;
* **the water height the scene answers under each round against the round's own altitude.** The
  static trace is the water surface and the comment at the sweep site names `0078CF20` as the
  routine that answers it, "at height zero for open sea". Whether it answers the same in this scene
  as at Pearl Harbour is exactly the open question; say what it returned;
* the inputs of the `swim > 0.0f && !shot.swimming` test on the tick it should have fired.

Measure for a fix: `swims_started=6` and the closest-approach census printing rows against the
Lexington. Same binary before and after apart from the change under test.

## One thing deliberately not chased

The three aircraft of each Kate squadron report identical numbers to the last digit - same
`arm_ticks`, same state counts, same `range_peak_in_goaway=428.2`. That is either the flight-lead
binding working or three tasks running one solution. One grep of how the task inputs are filled for
a wing member settles it; this packet had no context left to run it.

Also open from the same log, and already routed to the dive-bomb worker: twelve divebomb aircraft
(`movieval`, `D3A Val`) sit in `attackrun` for their entire lives with `releases=0 bombs_spawned=0
rounds_left=2`.
