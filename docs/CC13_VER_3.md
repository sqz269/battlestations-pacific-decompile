# CC13 VER 3: byte verification of the ship motion host

Packet `cc13-ver3` (cc13 lineage, Claude), 2026-10-13. This is a verification pass over cc9's ship
motion host. I made no host edits, no wiring and no game runs. Every cc9 file was read only.

Targets:
- the command tail of `00825F20` (`00826754`-`00826B84`) and its motion tail (`00826C34`-`00826D69`);
- the callees `0092D300`, `0092E8C0`, `00811890` (with `0092D730`, `0082ECB0` and `0082E890`),
  `00825DE0`, `00825EC0`, `00937440` and `009329C0`;
- the propeller update `00834E90`;
- the helm save fields `00815520` and `00815590`.

The ship AI navigator `00826D70`-`0081BEB0` was excluded. The part of `00825F20` before `00826754`
(jam timers, repair zone, sinking) was not checked.

Method (the same as `docs/CC13_GPR_48.md` section 8):
- I compared instruction by instruction, x87 included, against the disk bytes. Tools:
  Capstone (`local/ver3_dis.py`), `sc2_x87.py` block by block, and `bsp.py disasm-raw`.
- Every DC- and DE-form operation was read from its opcode bytes, never from listing text:
  - `DC C9` at `00834F03`;
  - `DE E2` and `DE E9` at `0092EA53` and `0092EA6B`;
  - `DE F1` and `DE FA` in `0092D300`;
  - `DC F9` at `009375F9`;
  - `DE E1` at `0093757F` and `00932E67`.
- The constants were read from the image as bytes.
- The class and settings values were read from `scripts/datatables/autoload/vehicleclasses.lua` and
  `scripts/datatables/shipglobals.lua` in this installation. That `vehicleclasses.lua` is locally
  modified (see the memory note on the modded install).
- `reports/cc13_ver3_calls.json`: `tools/verify_report_calls.py` checked 19 call rows against live
  Ghidra, and 0 failed.

Precision. `docs/CC13_FPU_PRECISION.md` shows that ship motion runs as a job: the main thread runs it
at 24 bits, and a pool worker runs it at 53 bits. The image's own result therefore depends on the
thread. The host models the two cases differently:
- `src/ship_motion.cpp` keeps x87 intermediates in double and narrows at each `FSTP`. That is the
  53-bit case.
- `src/unit_forces.cpp` computes in float. That is the 24-bit case.

Either way, any difference is in the last place. I list such cases as "last ulp" and do not count
them as divergences.

## 1. Results

| function | host | verdict |
| --- | --- | --- |
| `00825F20` command tail `00826754`-`00826B84` | `ship_motion_step_00825f20` and its helpers (src/ship_motion.cpp:139-394), bound by `ShipMotionBinding` (src/game_hosts_units.cpp:9517-10026) | **partial**: S1, B1 |
| `00825F20` motion tail `00826C34`-`00826D69` | `ship_motion_tail_00826c34` (src/ship_ai_rudder_hop.cpp:15), with `tin.dt = step_seconds` (src/game_hosts_units.cpp:34321) | **partial**: D1, which is latent |
| `0092D300` | `unit_approach_axial_speed_0092d300` (src/unit_forces.cpp:149) | **matches** |
| `0092E8C0` | `ship_apply_steering_0092e8c0`, `ship_righting_rate_0092e9e1` (src/ship_motion.cpp:49-133) | **partial**: R1 (constants) |
| `00811890`, `0092D730`, `0082ECB0`, `0082E890` | `unit_yaw_rate_00811890` and its helpers (src/unit_rudder.cpp:53-120), `unit_forward_speed_0092d730` (src/unit_motion.cpp:66) | **match** |
| `00825DE0` | `unit_propeller_turn_assist_00825de0` (src/unit_forces.cpp:218); settings passed as 0 | **formula matches**; inert in both (section 2.3) |
| `00825EC0` | `unit_forward_acceleration_00825ec0` (src/unit_forces.cpp:140); settings+228h passed as 0 | **formula matches**; inert in both |
| `00937440` | `unit_steering_torque_00937440` (src/unit_forces.cpp:236), called for every surface ship (src/game_hosts_units.cpp:9578-9609) | **partial**: T1 and T2 |
| `009329C0` | `ship_hydro_apply_forces_009329c0` (src/ship_hydro_forces.cpp:282) | **formulas match**. Leased to `cc13-ver5`, so I added no record (section 2.5) |
| `00834E90` | `unit_update_propellers_00834e90` (src/unit_motion.cpp:375), bound by `run_propellers_00834e90` (src/unit_instance_step11.cpp:207) | **head matches**, including the `DC C9` form. The per-propeller loop is presentation and is not reached in the host |
| `00815520`, `00815590` | not hosted by cc9; modelled by `kHelmSaveKeys_00815520` (include/bsp/gameplay_seg50_cc13.hpp:129) | **model matches** |

## 2. What matches

### 2.1 `00825F20`, the command tail

- **The engine gate (`00826754`-`00826792`).** It is 1.0 only when the jam byte unit+9E5h is clear
  and unit+9D8h compares unequal to XMM1:
  - XMM1 is zeroed at `008266AF` or `008266B4` on both paths into `008266C1`;
  - the test is `UCOMISS` / `LAHF` / `TEST AH,44h` / `JNP`, so a NaN gives 1.0, as `!= 0.0f` does.
- **The keel point (`00826866`-`00826972`).**
  - `along = float(class+A0h * -0.5)`, with the double -0.5 at `00CEC9E0`;
  - `base_i = float(along*row2_i) + T_i`;
  - `keel_i = float(float(class+A8h*-0.5)*row1_i) + base_i`.

  It is then sampled at (x, z) through `0078CF20`. Every product and sum is stored as a float.
- **The gate (`0082698A`-`008269A9`).** It keeps the command when `keel.y <= float(wave*0.5)`.
- **The target speed (`00826A3A`-`00826A5F`).** `float(9C0h*scale)*throttle*gate`, rounded once.
  The gate is 0 or 1, so the product is exact.
- **The boost block (`00826A6F`-`00826B04`).** Its structure matches. The values it is fed do not
  (B1).
- **Call order and gating.**
  - The force model `00826A6D` runs before the boost, `0092D300` and `0092E8C0`.
  - BL gates both command calls (`00826B0A`).
  - The jam byte unit+9E4h gates the steering call (`00826B2E`).
  - `0092BE80` is a bare `RET 4`.
  - `vtable[1ECh]` runs on every path.
- **Offsets.** Here EDI is unit+310h; `[EDI+228h]` is class pointer unit+538h. The offsets resolve to
  throttle +980h, toTurn +984h, kind +988h (the ring tick `00813020` writes ring+148h/+14Ch/+150h),
  +5Dh and class id +C4h, as the host binds them.

### 2.2 `0092D300`

- **The flattened axis.** `l = sqrt((x*x + 0*0) + z*z)`, with an `FLDZ`/`FMUL ST0,ST0` pair at
  `0092D346`/`0092D34F`; `h = (x/l, 0/l, z/l)` (`DE FA`, `DE F1`, `D8 FDIVR`).
  - This settles the square-root argument that unit_forces.cpp:152-156 calls provisional.
  - The y term contributes 0*0, so the host's `x*x + z*z` gives the same bits.
- **The current speed.** `f = (vx*hx + vy*hy) + vz*hz`. The host's order differs only by
  commutation, and since hy = 0 the bits are the same.
- **The target.** `target = (1 - |row2.y|) * cmd`, with the subtraction by `DE E9`.
- **The acceleration choice (`0092D451`-`0092D4DF`).**

  | condition | acceleration |
  | --- | --- |
  | `f > target`, `\|f\| < 3.0f` | 2 x class+504h (MaxAccel) |
  | `f > target`, `f > 0` | `00825EC0` |
  | `f > target`, otherwise | class+504h |
  | `f <= target`, `f < 0` | `00825EC0` |
  | `f <= target`, otherwise | class+504h |

- **The limiter.**
  - `step = accel*dt` and `gap = |float(f - target)|`, the absolute value by a bit mask;
  - `step > gap` snaps to the target, otherwise `f -/+ step` (`DE E1`, `DE C1`);
  - `v_i = float(h_i*f') + r_i`, where `r_i = float(v_i - float(h_i*f))`.
- **The divergences are NaN-only.** The image's `JBE` sends an unordered compare to the opposite arm
  from the host's comparisons. That happens in the acceleration choice and in the limiter.

### 2.3 `0092E8C0`, `00811890` and the assist

- **The slew in `0092E8C0`.** The rudder steps toward toTurn by `float(dt*0.5)`, through `0042AC60`.
- **The yaw-rate target.** `00811890` gets the slewed controller+80h, then the result is
  negated (`FCHS`) and passed to `00825DE0`.
- **The rows.** Each row's rate is `(r[1]*wy + r[0]*wx) + r[2]*wz`, for row 1, then row 2, then
  row 0.
- **The yaw slew.** The step is `float(dt+dt)` (`DC C0`) and the gap is the bit-mask absolute value:
  - `step > gap` takes the target;
  - otherwise `DF F2` / `JBE` gives `rate1 - step` when `target <= rate1`, else `rate1 + step`.
- **The recomposition.** `(p1 + p2) + p0` per component, each product stored as a float.
- **The righting gate and formula.** The gate is IsKindOf(0Eh). The formula's shape matches; the
  constants do not (R1).
- **`00811890`.**
  - `0092D730` is a plain `RET`. So the turnEfficiency unit+9DCh that `008118D3` pushes is the
    third argument of `0082ECB0` (`RET 0Ch`), as the host passes it.
  - `0082ECB0` computes `(((MaxRotAngle/d) * ratio) * rudder) * eff`.
    - `ratio = float(speed / class+500h)`;
    - `d = 0082E890(|ratio|)`, which is `00419010` on the two segments of settings+438h..+44Ch.
      An unordered compare takes the upper segment.
  - With `[00F88C30]` present the result is always multiplied by the scale, which is 1.0 unless the
    modifier flags are set. The host does the same.
- **`00825DE0` and `00825EC0` are inert in the image too.** Their formulas match. They read unit+1030h
  and unit+1038h. A displacement scan of `.text` (`local/ver3_dispscan.py`) finds only one writer of
  either field on a unit: the constructor stores at `0081F2EC` and `0081F304`.
  - The other `+1038h` writer, `004ED287`, is the weather scene record.
  - `009DE853`, `009D4FB0` and `009F4xxx` write +102Ch and +1034h, not +1030h or +1038h.
  - So the host's zero settings change nothing.
  - `docs/SHIP_TURN_RATE.md:76` says `009DE853` raises unit+1030h. The bytes show that it raises
    +102Ch. Its conclusion still holds: the assist is inert.
  - Caveat: a block copy or a pointer write would not show in a displacement scan. See the memory
    note on block-copy writers.

### 2.4 `00834E90`

- **The head.** s is toTurn +984h, or +FDCh when +61h is set. The steering target is
  `((s - (-1.0)) * K) - K`:
  - K is the double `00CEDCD0` = float(pi/4);
  - `DC C9` is `FMUL ST1,ST0` and `DE E9` is `FSUBP ST1,ST0`;
  - this is the corrected `(s+1)*pi/4 - pi/4`. It is slewed by `0042AC60` at a step of dt.
  - The host's constants have the same bits: -1.0, 0.78539818525314331, 1.2000000476837158, 4.0
    and 5.0.
- **The base rate (`008350F2`-`00835175`).**
  - `loaded = float((6A0h*load)*1.2 + 69Ch)`;
  - the clamp of `float(throttle*4)` to [-1, 1];
  - `(1 - |throttle|) * clamped * loaded * steering`, with the subtraction by `DE E1`.
- **The per-propeller terms.** The side term is `-0.0 - base` for a port node. The target is
  `6A0h*throttle + side` and the slew is `dt*5.0`.
- **What the host leaves out.** The host has no propeller nodes, so the loop never runs. Its zeroed
  class block (+69Ch/+6A0h/+6A4h) only affects presentation.

### 2.5 `009329C0` (informational; `cc13-ver5` holds the lease)

I checked these against the bytes before I saw the lease. I added no ledger record. They match
src/ship_hydro_forces.cpp:
- **The material.** Submarine (IsKindOf 8) is 2. Otherwise it is 1 when the double 100.0 at
  `00D7A220` is greater than the class mass, else 0. The stride is 38h from settings+4E0h.
- **The record mapping.** It was read from the reader `0083FFAA`-`008401B7`:
  - `+4E0h` L, `+4E4h` N, `+4E8h` LFel, `+4ECh` NFel;
  - `+4F0h` LOldalra, `+4F4h` NOldalra, `+4F8h` LElore;
  - `+4FCh` NElore is never read by `009329C0`;
  - `+50Ch` Kitevo, forced to 2.0 for material 1 (`00932B53`).
- **The hard-coded records** equal this installation's ShipGlobals.Physics rows.
- **The total mass.** `max(class+B0h + unit+10FCh, 1.0)`, with the leak water read once at entry.
- **The depth.**
  - The height is `float(double(y) - water)`, with `DC 6C FSUBR m64`.
  - It is clamped to `[0, span]` with the image's `JBE` sides, and the depth is `span - clamped`.
- **The buoyancy.** `coef * (d * ((d/shape)*k + (1-k)))`, where k = 0 for a submarine and 0.5
  otherwise.
- **The gates.** depth > 0 and `speed^2 != 0`, which is `JNP`, so a NaN passes.
- **The depth-factor ladder.** It uses `UCOMISS`/`JP`, and falls through to the previous value.
- **The drag scale.** `((mass/count)*factor)*10.0`, with the count converted as unsigned.
- **The drag.**
  - Lateral: `(v^2*4F4h + v*4F0h)`, then times the scale, then negated.
  - Forward: linear only, `(v*4F8h)*scale`.
  - Vertical: L/N when `vy < 0`, LFel/NFel when `vy >= 0` or NaN.
- **The impulse clamp.**
  - The limit is `-1.0 / count`, the double `00D7A250`. The order is forward, lateral, up.
  - The sum is `(forward + lateral) + up`.
- **The buoyancy direction.** The buoyancy is added on world Y.
- **The planing torque.**
  - Gates: +5Dh clear, last element, material 1, forward speed > 0.
  - Magnitude: `((Width*Length*d)/span)*fwd`.
  - It is subtracted as cross(lift, arm). The cross helper `004F9B30` computes a x b with ECX as the
    output, EDX as a and the stack as b.
- **The flooding weight.** `0 - water*10.0`, then the heel torque `0074F2E0`.

The only differences are the sign of a zero (`0 - x` against `-1*x`, `x + 0`) and NaN arms.

## 3. Divergences, with predicted effects

- **S1, the submerged throttle scale is 1.0 instead of 0.834.**
  - Host: src/ship_motion.cpp:307 sets `gate_in.submerged_scale = 1.0f`. The gate itself
    (ship_motion.cpp:185) is the image's.
  - Image (`008269B1`-`008269F0`): when class id is 8 and `-3.0f > unit+100h`, then
    `throttle = float(settings+4B4h * throttle)`. settings+4B4h is `Submarine.SubmarineDepthSpeedMul`
    (gameplay_settings.hpp:193, read at `0083F8A0`), which is **0.834** in this installation
    (shipglobals.lua:457).
  - Effect:
    - Every submarine with its pose below -3 m aims at 1/0.834 = 1.20x the image's speed. That
      covers periscope depth and every deeper band.
    - At full throttle it reaches MaxSpeed instead of 0.834 x MaxSpeed. The Lua comment says the
      image's value is 25 knots.
    - It reaches every mission with a dived submarine: the I-boat and US submarine missions, the
      submarine escorts and the torpedo attacks from depth.
    - The submarine closes about 20% faster, so detection, torpedo-solution and evasion timing
      shift.
    - On the surface (y >= -3) nothing changes.
  - Model: `ship_submerged_throttle_008269b1` and `ShipMotionSettingsCc13::submarine_depth_speed_mul_4b4`.
- **B1, the torpedo boat turbo has no class values and no seeded reserve.**
  - Host:
    - `boost_capacity` (class+808h) and `boost_speed_scale` (class+80Ch) stay 0;
    - `boost_refill_time` (class+810h) is 1.0 (src/game_hosts_units.cpp:15806);
    - the reserve unit+1188h starts at 0.
  - Image:
    - `00857F40` reads TurboTime (default 10.0), TurboStrength (default 3.0) and
      TurboRechargingTime (default 10.0) into +808h, +80Ch and +810h. This installation's PT rows set
      30/2/60 or 20/2/60, and some rows set 0/1/0.
    - `00857C80` seeds the reserve +1188h = TurboTime.
    - With an order kind != 0, `00826AAB` replaces the target speed with `0080FC30 * TurboStrength`
      and drains the reserve by dt.
    - Otherwise the reserve refills at `dt * TurboTime / TurboRechargingTime`, clamped at TurboTime.
  - Effect:
    - In the image, a TorpedoBoat holding a kind != 0 order runs at 2x its reference speed for
      20-30 s, then recovers in 60 s. In the host it never boosts. With the reserve at 0 the block
      is a no-op, and the target does not drop to 0 either.
    - The only kind producer I found is the HUD direct-turn path: `0067C4F0` passes the byte
      `[esi+20h]` to `00816A40`. The other two HUD callers pass 0.
    - So the turbo is a player control, and AI-only runs are unaffected unless an AI ring writer
      sets slot+1Ch. I did not trace those writers.
  - Model: `torpedo_boat_turbo_class_00857f40`. The seed is already modelled as
    `torpedo_boat_sentity_init_00857c80` (gameplay_seg53_cc13.hpp:484).
- **T1, the torpedo boat rudder torque is zero.**
  - Host: src/game_hosts_units.cpp:9595 sets `in.settings_rudder_torque = 0.0f` ("settings+588h, not
    recovered").
  - Image: settings+588h is `Physics.TBoatNyomatekSzorzo`, **1500** here (shipglobals.lua:553;
    reader default 30000 at `0083FDDF`). `00937440` adds a torque along the hull's forward axis:
    `T = row2 * (((1-|row0.y|)*|fwd/ref|*toTurn*1500) * mass^2 / 10000)`.
  - Effect:
    - In the image, a torpedo boat at speed with the rudder over gets a roll torque proportional to
      its speed fraction and rudder, so it heels in turns.
    - The heel shifts the buoyancy elements and so feeds back through `009329C0`.
    - The host's boats do not heel from the rudder.
    - The size depends on the body inertia, which I did not measure.
    - The host comment at game_hosts_units.cpp:9560-9569 says its inverse inertia is zero anyway. If so, wiring the value
      alone would move nothing.
  - Model: `tboat_rudder_torque_00937440`, in the image's grouping.
- **T2, the host applies `00937440` to every surface ship.**
  - `00937440` sits only in vtable `00D196F0` (`00939F2F`). That vtable is built by `00939F20`,
    whose sole caller `00857C20` is in the TorpedoBoat vtable `00D0C648`, at slot +230h (`00D0C878`).
    The same vtable holds `00857C80` at +A0h (`00D0C6E8`).
  - The base controller vtable `00D19630` has `009329C0` in slot 0, so other ships get the
    hydrodynamics only.
  - Effect today: none, because the gain is 0 (T1).
  - If settings+588h is ever wired, the host must restrict the torque to TorpedoBoats. Otherwise a
    40000 t battleship would get mass^2 x 0.15 per unit of rudder and speed.
  - The host's grouping `(m2*g)*row2_i / K` also differs from the image's `(g*row2_i)*m2 / K`. That
    is last ulp.
- **R1, the righting constants.**
  - Image:
    - `00D19628` is the double `0x3FD0C15240000000`;
    - `00D05AA8` is the float `0x3E860A92`;
    - both are float(pi/12) = 0.2617993950843811.
  - Host (include/bsp/ship_motion.hpp):
    - `kShipMotionRighting = 0.2617991025660515` (`0x3FD0C15105E92C7F`);
    - `kShipMotionRightingThreshold = 0.2617843747f` (`0x3E86089A`).
  - Effect:
    - The torpedo boat pitch limiter engages at |pitch| > 14.9991 deg instead of 15.0000 deg.
    - It aims at an angle 2.9e-7 rad lower.
    - That is a 1.5e-5 rad band and a sub-ulp target shift: negligible.
  - Model: `kShipRightingAngle00D19628`, `kShipRightingThreshold00D05AA8` and
    `ship_righting_rate_0092e9e1_cc13`.
- **D1, the motion tail uses the raw dt (latent).**
  - Image: `00826D04 FSUB [ESP+0C4h]` subtracts the slot that `00826144` overwrote with the scaled
    dt. The same slot feeds `00826B59`/`00826B6F`.
  - Host: src/game_hosts_units.cpp:34321 passes `step_seconds`.
  - Effect: it differs only when unit+340h (the per-unit time scale, CheatTurbo) is > 0. The host
    never sets it, so there is no effect now.
- **NaN-only differences.** None of these is reachable with finite inputs:
  - the keel gate: an unordered compare keeps the command in the image and drops it in the host;
  - the boost clamp: unordered keeps the value in the image; the host clamps it;
  - the `0092D300` arms;
  - the hull material choice for a NaN mass.

## 4. Records added

Ledger records (`config/reconstruction`). Status `host_reconstruction_checked_against_listing` marks a
match. `host_reconstruction_partial_divergence_listing_checked` marks a record that names its
divergences.

cc9's own records were left untouched. Each new record is named `cc13_ver3_check_<host symbol>` and
carries the host name in `host_symbol`. The one exception is `0092D730`: it had no record under the
host symbol, so its record uses `unit_forward_speed_0092d730`.

| address | name | verdict |
| --- | --- | --- |
| `00825F20` | `ship_motion_step_00825f20` | partial (S1, B1) |
| `00826C34` | `ship_motion_tail_00826c34` | partial (D1, latent) |
| `0092D300` | `unit_approach_axial_speed_0092d300` | checked |
| `0092E8C0` | `ship_apply_steering_0092e8c0` | partial (R1) |
| `00811890` | `unit_yaw_rate_00811890` | checked |
| `0092D730` | `unit_forward_speed_0092d730` | checked |
| `0082ECB0` | `unit_class_yaw_rate_0082ecb0` | checked |
| `0082E890` | `unit_rudder_denominator_0082e890` | checked |
| `00825DE0` | `unit_propeller_turn_assist_00825de0` | checked (inert) |
| `00825EC0` | `unit_forward_acceleration_00825ec0` | checked (inert) |
| `00937440` | `unit_steering_torque_00937440` | partial (T1, T2) |
| `00834E90` | `unit_update_propellers_00834e90` | checked (head); the loop is not reached |
| `00815520` | `kHelmSaveKeys_00815520` | checked (model) |

`00815590` has no Ghidra function. It is described in the `00815520` record and was not given a
record of its own.

Models: `src/ship_motion_ver_cc13.cpp` and `include/bsp/ship_motion_ver_cc13.hpp`, registered in
`cmake/startup.cmake`. They are reconstructed and build-tested only. They are not ABI-compatible, not
wired and not game-validated.

## 5. Open

- **The order-kind producers for B1.** I did not trace the AI ring writers of slot+1Ch.
- **Whether the physics library also calls controller slot 0.** The `009329C0` ledger note says the
  Dyn library invokes it. If it does, the image runs the hydrodynamics once from `00826A6D` and again
  as a Dyn force callback, and the host runs it once.
- **The part of `00825F20` before `00826754`.** That covers the jam timers (unit+768h..+814h), the
  repair zone `00825450` and the sinking block. This packet did not check it.
- **The heel magnitude for T1.** Measuring it needs the hull inertia (`00C5C940`), which the host
  marks unread.

## 6. Host fixes and adoption notes (HOST FIX TERMS, 2026-10-13)

Applied behind OFF switches in files no cc9 lease covered (each built, ctest passed):

| divergence | switch | file | commit |
| --- | --- | --- | --- |
| R1, righting constants | `kShipRightingImageConstantsBound` | include/bsp/ship_motion.hpp, src/ship_motion.cpp | `126c138c7` |
| S1, submerged throttle scale | `kShipSubmergedThrottleBound` (installed 0.834 as a labelled constant) | include/bsp/ship_motion.hpp, src/ship_motion.cpp | `9f68f389b` |

Adoption notes only. All three sit in src/game_hosts_units.cpp, leased to `agent/cc9-lua52` when
I checked:
- **B1.** In the class-row block (now near :15806), fill `motion_class.boost_capacity`,
  `boost_speed_scale` and `boost_refill_time` from the TorpedoBoat row's TurboTime,
  TurboStrength and TurboRechargingTime. When a key is missing, use the defaults from
  `torpedo_boat_turbo_class_00857f40`. Then seed `motion.boost_reserve` from TurboTime
  (`torpedo_boat_sentity_init_00857c80`).
  - Rows with TurboRechargingTime = 0 divide by zero at `00826AD6`, as the image does.
- **T1 and T2.** In `ShipMotionBinding::run_force_model` (near :9578), call the torque only
  when the unit is a TorpedoBoat. Those are the units built with controller variant C,
  `00857C20`. Pass `settings+588h` (`physics_tboat_nyomatek_szorzo`, 1500 here).
  - Every other surface ship goes straight to `run_hydrodynamics_00937622`.
  - Check the body's inverse inertia before predicting any heel.
  - `tboat_rudder_torque_00937440` has the image's grouping.
- **S1 follow-up.** Pass the loaded settings+4B4h into the gate and drop the installed
  constant.
- **D1.** Near :34321, pass `result.dt_scaled` as `tin.dt`. This is latent while unit+340h is
  0.
