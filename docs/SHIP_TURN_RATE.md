# Ship turn rate under the torpedo override (packet `cc9_ship_turn_rate_check`)

Worker cc9-ships2, on main `2ea0b4d3c`. Ghidra was read-only, and every body named here has a
Ghidra function. Names are hypotheses.

## Answer

**The host turns at the image's rate.** Its yaw law is the image's own reconstructed chain.
Measured against that law's full-rudder rate at the sampled speeds, Exeter and Perth reach 0.95
of it once the rudder is hard over. The 5% lag is the rudder and yaw-rate slews.

The **0.025 rad/s** in docs/TORPEDO_EVASION.md was a mean over 148 s to 160.2 s. That window
includes 150 s to 153.8 s, when the override was off and the ship turned back, and a speed dip to
7.8 m/s. It is not the turn rate.

A York-class cruiser at full speed and full rudder turns at **0.0524 rad/s (3.0 deg/s)**, which is
**30 s for 90 degrees**. That is the image's figure from this installation's class data. Nothing is
bound.

## The image's law, hop by hop

1. **Heading error to ordered rudder: `009DA250`** (docs/SHIP_AI_CLASS_FIELD_0524.md,
   docs/SHIP_AI_HEADING_TO_RUDDER.md).
   - rudder = -error / (class+524h x 1.2), clamped to [-1, +1].
   - class+524h = 0.5 x MaxRotAngle / MaxRotAngleChangeRatio. For York that is
     0.5 x 0.10472 / 0.523599 = 0.1 rad, so the rudder is hard over beyond **0.12 rad** of error.
   - The AI deadband and slew are at `009F4BA7..009F4C12` (step dt x 1.5). The ring then
     carries the rudder to unit+984h (`0080E190`, `00813020` / `0042AC60`).
2. **The steering: `0092E8C0`** (docs/SHIP_MOTION.md).
   - The smoothed rudder slews at **0.5 per second**: `0092E8C3` step = dt x 0.5 (the double
     at `00D7A280`), and `0092E8EF` calls `0042AC60`.
   - The commanded yaw rate is -`00811890`(unit, rudder), then `00825DE0`, the propeller turn
     assist.
   - The body yaw rate slews toward the command by at most **2 x dt** per tick (`0092EA75`
     maxStep = dt + dt).
3. **`00811890`** gives `0082ECB0`(rudder, `0092D730` forward speed, unit+9DCh turnEfficiency).
   - turnEfficiency is 1.0 from the constructor at `0082371C`.
   - The result is scaled by `008E6430`(5, unit) only when `00E0C978` and the list at
     (*00F88C30)+C4h are both set.
4. **`0082ECB0`** (body `0082ECB0-0082ED10`), in full:

```
0082ECB3  FLD  [ESP+14h]            ; forward speed
0082ECB8  FDIV [ECX+500h]           ; / MaxSpeed
0082ECBE  FSTP [ESP+18h]            ; r, rounded to float
0082ECCA  FLD  [ECX+4F8h]           ; MaxRotAngle
0082ECD0  FSTP qword [ESP+8]
0082ECD8  AND  EAX,7FFFFFFFh        ; |r|
0082ECE8  CALL 0082E890             ; d(|r|), the TurnMultipliers curve
0082ECED  FDIVR qword [ESP+4]       ; MaxRotAngle / d
0082ECF1  FSTP [ESP]                ; base, rounded to float
0082ECF7  FMUL [ESP+14h]            ; x r
0082ECFB  FMUL [ESP+10h]            ; x rudder
0082ECFF  FMUL [ESP+18h]            ; x turnEfficiency
0082ED03  FSTP [ESP+14h] / 0082ED0E RET 0Ch
```

`0082E890` is shipglobals.lua `Navigator.TurnMultipliers`: d = 0.4 at r = 0, 1.5 at r = 0.5, and
2.0 at r = 1, linear and clamped between (docs/UNIT_RUDDER_CURVE.md).

So the steady rate is **MaxRotAngle x r / d(r) x rudder**, with no other speed floor. At rest it
goes to zero through r.

## The host's law, term by term

| hop | host | same as the image |
| --- | --- | --- |
| `009DA250` | `bsp::ship_ai_rudder_from_heading_error_009da250` (src/ship_ai_throttle_ring.cpp) | reconstructed |
| `0092E8C0` | `bsp::ship_apply_steering_0092e8c0` (src/ship_motion.cpp), bound in src/game_hosts_units.cpp `yaw_rate_target` | complete: both slews and the order |
| `00811890` / `0082ECB0` / `0082E890` | `bsp::unit_yaw_rate_00811890` (src/unit_rudder.cpp), inline x87 | curve fixture-checked |
| turnEfficiency unit+9DCh | 1.0 (`0082371C`) | yes |
| `008E6430`(5) | runs whole over an empty modifier list, so 1.0 | yes when no modifier is registered. See the residuals |
| `00825DE0` propeller assist | settings+220h / +224h passed as 0 | inert while the load is 0. The torpedo override does not raise it (below) |

The torpedo override is section 5 of `009DE5B0`, `009DE8F1..009DE96C`. It writes only
blk+324h, at `009DE945` and `009DE962`. The propeller load unit+1030h is raised only by section
4's `009DE853`, so the unrecovered assist settings play no part in a torpedo turn.

## Measured (`local\te_main_usn02.log` for Exeter, `local\te_main2_usn02.log` for the others)

The rows below come from `BSP_TORPEDO_TRACE` on main `2a1d54495`. The measurement script is
`local\cc9-ships2-tr_measure.py`. "Law" is 0082ECB0's rate at full rudder, averaged over the
window's sampled speeds.

The class data come from `scripts/datatables/autoload/vehicleclasses.lua` in this installation,
which is locally modified (mtime 2026-05-09 21:52). No unmodified copy was compared.
- Exeter, VehicleClass[21] York 1942: MaxRotAngle 0.10472, MaxSpeed 16.590833.
- Alden, VehicleClass[25] Clemson 1930: MaxRotAngle 0.139626, MaxSpeed 18.26278.
- Perth, VehicleClass[263] Leander 1942: MaxRotAngle 0.122173, MaxSpeed 16.205.

| ship | window | heading error at start | speed | measured | law, full rudder | ratio |
| --- | --- | --- | --- | --- | --- | --- |
| Exeter | 148.00-150.00 s | 0.873 rad | 17.38 to 14.17 m/s | 0.0485 rad/s | 0.0512 | 0.95 |
| Exeter | 157.00-160.30 s | 0.708 rad | 13.24 to 16.59 m/s | 0.0484 rad/s | 0.0508 | 0.95 |
| Perth | 200.00-203.60 s | 0.394 rad | 16.20 to 8.87 m/s | 0.0503 rad/s | 0.0531 | 0.95 |
| Alden | 163.90-166.15 s | -1.507 rad | 7.48 to 10.87 m/s | -0.0246 rad/s | 0.0452 | 0.54 |

Every error is past the 0.12 rad saturation, so the ordered rudder is hard over. Alden's override
began at 163.90 s. Its smoothed rudder was still slewing toward the new side at 0.5/s when it was
struck 2.25 s later.

**Full-speed full-rudder figures from the law:**

| class | rate | 90 degrees |
| --- | --- | --- |
| York | 0.0524 rad/s | 30.0 s |
| Leander | 0.0611 rad/s | 25.7 s |
| Clemson | 0.0698 rad/s | 22.5 s |

## Verdict

- **The host matches the image, so no switch was added and no pair was run.**
- **A cruiser's torpedo turn is slow by the class data.** York is 3 deg/s at full speed, and at
  half speed the curve gives MaxRotAngle x 0.5 / 1.5, or 2.0 deg/s.
- **The evasion also has the slews:** hard over to hard over takes 4 s at 0.5 per second.
  Evasion therefore helps a cruiser only against torpedoes it tracks for tens of seconds. That
  is the image's behaviour, not a host shortfall.

## Residuals, not claimed

- **`008E6430` category 5.** The image multiplies the yaw rate by the registered gameplay
  modifiers of category 5, when `00E0C978` and the list are set. This host registers none, and
  the filter `008E4680` is unread. Whether a single-player mission registers any, from crew or
  upgrades, is not established here.
- **The x87 precision control word** (docs/SHIP_MOTION.md) is still unsettled. It cannot move a
  5% gap.
