# The dive response: every stage matches, and the measurement does not

Addresses: 007DC205, 007DC208, 007DC20E, 007DC246, 007DC448, 007DC451, 007DC45B, 007DC4AC,
007DC4B5, 007DC4BF, 007DC601, 007DC60A, 007DC614, 007DC68C, 007DCDD0, 007DB680, 007DB744,
007DB875, 007DB990, 007DBA32, 007DBB0E, 007DBB23, 007DBD37, 007D9050, 007D9140, 007C4990,
007C499C, 007D20C6, 0099E490, 009FB800, 00D7A280, 00CE3D48.

Packet `cc8_plane_dive_response`, owner `agent/cc8-torpedo-run-in`, on main `51a8d977a` merged.
Reading only; no run, because main past `48F293197` is still under the startup-crash bisect.

**Every stage of the response chain matches the listing. The measurement does not match the
chain.** That is the packet's result, and it is a sharper statement than "the host diverges": the
host's own law cannot produce the speed the host's own run recorded, at any dive angle.

## 1. The water arm is a force accumulator, and nothing pre-contact lives in it

`007DC205`-`007DC68C`, selected by `ctl+FCh == 2`, which only `007DCDD0
BSP_PlaneFlight_WaterSurfaceLaw` sets on entry (`007DCDDC`). So the arm runs **after** the flight
state has already changed, and **no pull-out, altitude-floor override or speed cap can live in it
or on its way in**. The state change itself is `007CB7F0`'s tail, which
`docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md` bound.

Its head is a damage fade:

```
007dc208  FLD  float ptr [ECX + 0xc3c]     ; the DeadMeat timer
007dc20e  FMUL double ptr [0x00d7a280]     ; * 0.5
007dc218  FSUBRP ST2,ST0                   ; 1 - timer * 0.5
007dc22a  JBE  007dc237                    ; clamped at 0 below
```

and its body is **three force triples added into the same `dyn` accumulators the free-flight arm
uses**, at `007DC448`/`451`/`45B`, `007DC4AC`/`4B5`/`4BF` and `007DC601`/`60A`/`614`. Every write in
the arm is `FSTP [EAX]`, `[EAX+4]`, `[EAX+8]` after an `FADD` of the same slot: accumulate, never
assign.

**It writes no pose, no velocity and no flight state.** So an aircraft on the water is integrated by
the same fold and the same step as one in the air, with buoyancy and water drag in place of lift and
air drag.

**Coverage: partial.** The three triples' formulas were not transcribed - the arm is 316 lines - and
the doc claims only their shape and their destination.

## 2. The response chain, stage by stage

| stage | native | host | verdict |
| --- | --- | --- | --- |
| the pitch command | `009FB800`, `-min(DropAngle·t, max(DropAngle·1.6, DEG(60)))` | `pitch_command_009fb800` | **matches**, `docs/ATTACK_RUN_DESCENT.md` |
| the nose-up floor | `0099E490`-`0099E512`, `max(target, DEG(6) - 2.5(1-q))` | `pilot_pitch_demand_0099e490` | **matches** |
| mode 2's target limit | `0099DD4C`, `min(commanded, held + inc)` | not modelled | **substituted**, and inert for a dive: it caps a climb only |
| the slot slew | `0099BC00` at `kPilotSlewRate` | `pilot_evaluate_plan_slots_0099bc00` | matches |
| the quantiser | `007BB6E0`, the signed-byte round trip | `pilot_quantize_control_axis_007bb6e0` | matches |
| the latch | `007B9770` | `latch_control_input_007b9770` | matches |
| the rate law | `007DA710` | `control_step_007da710` | matches |
| the attitude integration | `007D9C80` then `0085E4D0` | `advance_pose_0085e4d0` | matches |
| thrust | `007D9050`, `Accel · throttle`, gated on the latched throttle `> 0.01` | bound | **matches in form**; `007D20C6`'s in-place scale of `Accel` is **substituted** |
| drag | `007D9140`, `-sgn·v²·k·desc+50Ch·brake` with `desc+50Ch = Accel / MaxSpd²` | bound | matches |
| the drag ramps | `007DBB0E`/`007DBB23` | bound | matches, and both are `1.0` for a healthy aircraft |
| lift | `007DB875`, `AccelCheatMul · 9.81` body up once `v >= 1.8·StallSpd` | `lift_accel_007db875` | matches |
| gravity | `007DB990`, `AccelCheatMul · 9.81` world down | `gravity_accel_007db990` | matches |
| the body damping | `007DBD37`, `XDrag`/`YDrag` | bound | matches |

**The velocity does not follow the nose directly.** It follows through the body-frame damping at
`007DBD37`, which drives the body lateral and vertical velocity components toward zero at `XDrag`
and `YDrag` per second; the forward component is untouched by it, and lift acts body-up while
gravity acts world-down. That is `docs/TORPEDO_RUN_IN_VELOCITY.md`'s finding and it is unchanged.

**One substitution could matter and is named.** `007D20C6` scales `desc+164h Accel` in place by
`tuning+31Ch · tuning+320h` when the second exceeds `1.0`. This installation authors
`AccelCheatMul = 1.5` and `AccelCheatMulMul = 1.15`, so the image's `desc+164h` is plausibly
`6 · 1.725 = 10.35` where the host uses the raw `6`. It **cannot** move level flight, because thrust
and `desc+50Ch` read the same field, but it **does** move a dive: a larger `Accel` raises the drag
coefficient faster than the thrust, so the modelled terminal speed would *fall* to about 104 m/s.
It is therefore not the explanation for a measurement that is too *high*.

## 3. The numbers, and the contradiction

`local/dive_sim.py` integrates the host's own terms - the same thrust, the same
`desc+50Ch = Accel / MaxSpd²`, the same `AccelCheatMul · 9.81` gravity share - down a commanded
dive from the census's own starting point, 806 m at 63.2 m/s, with the `Mav` row the run printed
(`Accel 6`, `MaxSpd 69.44`, `StallSpd 19.4`):

| quantity | value |
| --- | --- |
| drag coefficient `desc+50Ch` | 0.001244 |
| level-flight equilibrium | **69.44 m/s**, exactly `MaxSpd` |
| lift gate `1.8 · StallSpd` | 34.92 m/s |
| terminal speed in a 60-degree dive | **122.74 m/s** |

| dive angle | speed at sea level | time |
| --- | --- | --- |
| 30 degrees | 103.0 m/s | 17.3 s |
| 45 degrees | 112.5 m/s | 11.7 s |
| **60 degrees** | **118.3 m/s** | **9.3 s** |
| 75 degrees | 121.4 m/s | 8.2 s |

**The runs measured 141.5 to 141.9 m/s** (`local/usn01_alt_after.log`,
`local/usn01_wire_after.log`). Even a vertical dive tops out at about 129 m/s under these terms, so
**no dive angle in this law reaches the measured speed.** The level-flight figure landing exactly on
`MaxSpd` says the thrust and drag magnitudes are right, because that equilibrium is where they
balance; it says nothing about gravity, which lift cancels in level flight.

So a term between the law and the integrator is not the one this doc just verified, and the
unvalidated one is **gravity's share**. The candidates, by address, are `007DB990`'s own
`AccelCheatMul` scale, the world-to-body fold `007D8470` and the transpose the free-flight arm
applies to `body.total` before integrating - that rotation was corrected once already
(`docs/PLANE_FREE_FLIGHT_PHYSICS.md`, "Open: the host's integration frame").

## 4. What this is and is not

**It is not a located divergence**, and I am not claiming one. Every stage reads as matching, and
the contradiction is between the law and a measurement rather than between the host and the image.
Settling it needs one instrumented run - the along-path acceleration and the four accumulator
triples printed once per second for one diving aircraft - which today's startup-crash bisect rules
out. That run is the packet's own next step and it is cheap.

**What it does retire**: "the host dives wrong because it commands the wrong pitch" is dead, from
`docs/ATTACK_RUN_DESCENT.md`; and "the host dives wrong because a stage of the response chain is
missing" is dead from section 2. What is left is narrow and numeric.

## ABI

* `007DB680` `BSP_PlaneFlight_CoreLaw`, as recorded; the water arm is `007DC205`-`007DC68C`.
* `007DCDD0` `BSP_PlaneFlight_WaterSurfaceLaw`, which sets `ctl+FCh = 2` at `007DCDDC`.

## Uncertainty

* The water arm's three force formulas. Shape and destination only.
* `007D20C6`'s `Accel` scale, named above and substituted.
* Mode 2's increment, inert for a dive.
* **The 20 per cent gap between the law and the measurement.** Named, not resolved.

## Host methods

**None.** Reading only, plus `local/dive_sim.py`, which is a scratch calculator and not part of the
build.

## Corrections

None. Every stage this packet compared already matched.

## no_ghidra_function

None.

## Validation

No game run: main past `48F293197` is under the startup-crash bisect. The numbers in section 3 come
from integrating the host's own terms in `local/dive_sim.py`, against measurements already recorded
in `local/usn01_alt_after.log` and `local/usn01_wire_after.log`.

## Follow-up packets

1. **One instrumented run**: print the along-path acceleration and the four `dyn` triples once a
   second for a diving aircraft, and the 20 per cent gap resolves in a single pass. This is the
   packet.
2. **`007D20C6`'s `Accel` scale**, which is a real unmodelled term even though it cannot explain
   this gap.
3. **The water arm's three force formulas**, if an aircraft on the water ever needs to behave.


## Resolved by the instrumented run: the surplus is the authored fall cheat (packet `cc8_plane_dive_instrumented`)

**Section 3's contradiction is mine, not the host's.** The run says the host is faithful and both
of my models were missing a term the host had bound all along.

### The measurement

One aircraft, `Mav3`, from the 60-degree command to the water, once a second:

| alt | speed | pitch | path | aoa | along | thrust | drag |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 782.4 | 72.24 | -0.3037 | -0.2721 | 0.034 | 4.976 | **8.212** | -7.143 |
| 755.6 | 79.17 | -0.4800 | -0.4456 | 0.036 | 8.673 | **11.017** | -8.577 |
| 712.3 | 89.12 | -0.6614 | -0.6240 | 0.038 | 10.873 | **13.377** | -10.869 |
| 649.7 | 100.46 | -0.8444 | -0.8045 | 0.041 | 11.506 | **14.968** | -13.655 |
| 567.0 | 112.02 | -0.9845 | -0.9542 | 0.035 | 10.770 | **15.539** | -16.373 |
| 468.9 | 121.27 | -1.0179 | -1.0093 | 0.028 | 8.816 | **15.587** | -18.671 |
| 361.6 | 129.26 | -1.0678 | -1.0480 | 0.030 | 6.608 | **15.600** | -21.263 |
| 245.7 | 134.96 | -1.0754 | -1.0694 | 0.018 | 5.029 | **15.600** | -23.268 |

### The three gravity candidates are exonerated

`grav=(0.000 -14.715 0.000)` on every line, which is exactly `AccelCheatMul * 9.81` world down with
`cheat=1.50`, so `007DB990`'s scale is right. The folded body total rotates back through the
transpose to a world vector consistent with it, so `007D8470` and the transpose are right too. None
of the three carries a surplus.

### The surplus is the thrust, and it is authored

**`thrust` is 15.600, not 6.0.** `007D9050`'s fall cheat multiplies `Accel` by
`1 + (AccelCheatFallMul - 1) * sin(Interp(FallPitchRange1, 0, FallPitchRange2, pi/2, -pitch))`, and
this installation authors `AccelCheatFallMul = 2.6` with `AccelCheatFallPitchRange = { DEG(10),
DEG(60) }`. Past `DEG(60)` of nose-down the sine saturates, so the multiplier is the full `2.6` and
`6 * 2.6 = 15.6` - the probe's number to three decimals. `planeglobals.lua`'s own comment says so:
*"lefele repulve, a zuhanas szogenek fuggvenyeben, max ennyivel nagyobb gyorsulassal megy lefele"*,
flying downward it accelerates by up to this much more, as a function of the dive angle.

**The host bound this correctly in `cc8_plane_pose_throttle_altitude`.** It was my arithmetic in
`local/dive_sim.py` and `local/dive_sim2.py` that used the raw `Accel`, and section 3 published the
gap as though it were the host's.

Re-running the model with the fall cheat:

| quantity | value |
| --- | --- |
| thrust at -60 degrees | **15.600**, matching the probe exactly |
| terminal in a 60-degree dive | 150.9 m/s |
| modelled arrival, 60 degrees | **144.7 m/s** |
| measured arrival | **141.5 to 141.9 m/s** |

The residual three metres per second is the pitch ramp: the probe's own trace shows the aircraft
shallower than 60 degrees for the first three seconds, which the constant-angle model does not
carry.

### So there is no divergence, and nothing to fix

Every stage of section 2 matched, gravity matches, the fold matches, and the thrust matches once
the authored cheat is included. **The 141.7 m/s arrival is the image's own behaviour on this
placement**, and `docs/ATTACK_RUN_DESCENT.md`'s conclusion stands unqualified: the aircraft is
commanded into a 60-degree dive from 800 m and the physics carries it to the water at about
142 m/s.

One incidental confirmation: the angle of attack stays between 0.018 and 0.041 radians for the whole
dive, which is `docs/TORPEDO_RUN_IN_VELOCITY.md`'s body damping doing its job and is why
`local/dive_sim2.py`'s vector treatment made no difference.
