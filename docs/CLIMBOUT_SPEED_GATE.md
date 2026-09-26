# The fighter stall inside aim: there is no climb-out law (packet cc9_climbout_speed_gate)

2026-09-25. Read-only on Ghidra. Names are hypotheses. The packet asked for "the surface climb-out
law that produced `cmd_alt` 661.5" for Lexington-class01_sqn03 in `local\THR1_9000.log`. There is no
such law, and the premise came from my own wrong reading in docs/DOGFIGHT_THROTTLE_RETAKE.md. That
doc is corrected by this one.

## 1. The premise, corrected

- **The "surface probe" line is a census.** It is not a law. The packet cc9_pilot_surface_climbout
  logs a live aircraft below 5 m once a second, and docs/PILOT_SURFACE_CLIMBOUT.md section 1 says the
  image has no climb-out: a live aircraft at the surface keeps flying the pilot's commands, and dies
  by the depth kill below -30 m.
- **`cmd_alt` and `cmd_pitch` are stale in aim.** The census prints `plane_commanded_altitude` and
  `plane_commanded_pitch`, which only the cruise-altitude path (`009FBA50` -> `009FB800`) writes, in
  moveto and follow. The aim tick steers pitch through `009F9ED0` into `plan+2BCh` and writes neither
  field. So 661.5 m was the fighter's last moveto command.

## 2. The pitch chain that did act, and its caps

- **Aim.** `009A78BB` calls `009F9ED0(aim y - unit y, approach+D8h)`, capped upward only at
  `class+1E4h` (docs/PLANNER_HEADING_WRITES.md 7.2).
- **Moveto and follow.** `009FB800` climbs with the limit `max(class+1ECh * 1.6, 40 deg)`
  (`include/bsp/plane_flight.hpp`).
- **Both caps come from one number.**
  - `class+1E4h` is `007D98F0`'s steepest sustainable climb at `LevelFlight * StallSpd`, which is
    1.8 x 17.5 = 31.5 m/s for this fighter.
  - `class+1ECh` is `0.6 * class+1E4h` (the host's `plane_climb_angle_1ec`).
- **The values, from the trace below:** `+1ECh` = 0.468, so `+1E4h` = 0.78 rad, and the moveto limit
  is 1.6 x 0.468 = 0.749 rad.
- **The death pitch matches aim's cap.** THR1's sqn03 died with pitch +0.79 rad, which is aim's cap
  of 0.78.
- **StallSpd 17.5 is authored.** This installation's `vehicleclasses.lua` carries `["StallSpd"] =
  17.5` on several rows, which is also `PlaneFreeFlightClass`'s fallback. So the caps are the image's
  own numbers, not a host default standing in.
- **The planner has no speed guard on pitch.**
  - `0099E490`-`0099E512` is an attitude floor that rises with bank and heading error. It reads no
    speed (docs/PILOT_PLANNER_PITCH_ROLL.md 2c).
  - The one low-airspeed gate in `0099D300` (`0099D3EA`-`0099D425`) levels yaw and roll only. It
    applies when the component at `unit+72Ch` answers `vtable[38h]` true and `unit+908h` is below
    4.0 (`00CE3D34`). It writes no pitch.
- **So the image has no minimum-speed or stall guard** that the host lacks, anywhere on this path.

## 3. What the trace shows

Run `local\CSGT_9000.log`: current main (`60291afaf` plus nothing), `kDogfightThrottleBound` ON and a
diagnostic that logs every live dogfight fighter below 400 m once a second (`kFighterLowTraceDiag`,
committed OFF).

- **In this run no US fighter goes into the sea.** The two depth kills are A6M Zeros. sqn03
  survives, and sqn01 is again a gun kill. The stall is RNG-coupled.
- **The climb bleeds speed at full throttle.** At 148-155 s sqn03 is in moveto, climbing from 150 m
  to its commanded altitude. `plan+2BCh` is 0.749 and the throttle is 1.0. The nose goes to 0.805 and
  the speed falls from 83.3 to 52.4 m/s in 7 s. That is the image's own limit asking for more than the
  aircraft can sustain above 31.5 m/s. It is sustainable only near 31.5 m/s, which is where the climb
  decays to.
- **A mushing fall, and a recovery.** At 273 s, in maneuver at 393 m, sqn03 sinks at 44.8 m/s at 58.4
  m/s with pitch +0.31. It recovers within 5 s: the maneuver commands pitch -0.06, the nose drops,
  speed returns.
- **What killed sqn03 in THR1.** It stalled at low height inside aim, where the command stays at the
  0.78 cap for as long as the target is above. The direct throttle (`007B4ED0`, head-on arm) is what
  the switch adds; it slows the fighter first.
- **58.37-58.40 m/s recurs** as the speed of every mushing fall: THR1's sqn03, this run's sqn03, and
  the Zeros' sea contacts in `local\PHW1_9000.log`. It looks like the host flight model's settled
  speed in a deep stall. It is not read here.

## 4. Verdict

- **No host gap on the command side.** The image's aim pitch cap, climb limit and planner pitch arm
  are what the host runs, with the image's class values. Nothing was bound, and there was no pair to
  run.
- **`kDogfightThrottleBound` stays OFF.** With it, a fighter slowed by the head-on throttle can be
  held nose-up at the aim cap near the sea until it stalls in. Whether the image does the same
  depends on the flight model's high-angle-of-attack behaviour, not on the pilot. Does the image's
  free-flight law drop the nose, or bleed less speed, in a deep stall?
- **Next read.** The free-flight law's lift and pitching moment at high angle of attack and below
  `StallSpd`, `007DA710` and its callees. Compare the host's mush at a settled 58.37 m/s with what
  the image's law gives for the same state.
- **Unverified.** Whether the image fighter stalls the same way needs a run of the original game,
  which this project cannot do unattended.
