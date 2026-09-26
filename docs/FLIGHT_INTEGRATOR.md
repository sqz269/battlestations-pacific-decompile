# The flight integrator, 007D8470's velocity step (packet cc9_flight_integrator)

2026-09-25. Ghidra read-only. Names are hypotheses. The read is docs/FREEFLIGHT_STALL_LAW.md section
4, completed here.

## 1. The rest of the read

- **`dyn+88h..90h`.** `dyn` is `[ctl+10h]` (`007DC6E0`). Its constructor `007D7B90` copies `+88h..90h`
  from the globals `00F87574`/`78`/`7C` (`007D7BB1`-`007D7BD9`). Nothing stores to those three by
  literal address. Ghidra's xrefs are all reads, from generic entity and command code
  (`BSP_CommandTarget_FromEntity`, `BSP_Entity_IssueCommand` and others), which is the shape of a
  shared zero vector. The census's other `+88h`/`+8Ch` stores (`007DCCD3`, `007DCDAD`, `007DD84F`)
  write the controller, not `dyn`.
  So `dyn+88h` is zero in practice, **provided those globals are zero**. That is an inference:
  a block-copy or static initializer could still write them.
- **`007D8E28`-`007D8F16`, the contact response.** With `dyn+A4h` set, or `dyn+94h > 0`, it removes
  the world velocity's component into the normal `dyn+98h..A0h`, then clears `dyn+A4h`. It does not
  run in free flight.
- **`007D8F18`-`007D902F`.** It stores the step's acceleration: `(v - v_prev) / step` into body
  `dyn+7Ch..84h`, and rotated into world `dyn+58h..60h`. Then it decays the hold timer `dyn+C0h`. It
  does not integrate position. Position is advanced by the pose arm, from `ctl+18h..20h` times
  `step` (docs/PLANE_ADVANCE_POSE.md), which is what the host's `position += v * step` after the
  velocity update does.

## 2. The binding

`kFlightIntegratorBound` (`src/game_hosts_units.cpp`, the free-flight step) replaces
`v += a * step`. The steps:
1. Take the body velocity through the pose rows.
2. Apply `bsp::integrate_body_velocity_007d8611` (`include/bsp/plane_flight.hpp`), with `a =`
   `pair_1c` (lift, thrust, gravity) and the resisting fold `pair_04` (damping, drag):
   - `w = v + a*dt`;
   - the resisting term acts only against `w` and stops it at zero;
   - components under 0.01 are zeroed.
3. Rotate back to world.
4. Advance the position.

`dyn+40h` (the third fold) is empty in free flight and passes as zero. So does `dyn+88h`.

## 3. Predictions, written before the pair

The pair is both switches OFF (`local\fi0`) against `kFlightIntegratorBound` ON **and**
`kDogfightThrottleBound` ON in the same build (`local\fi1`). E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`. Reference runs: `local\THR0_9000.log` / `local\THR1_9000.log` for the throttle
switch alone, on an older tree.

| row | OFF | ON prediction |
| --- | --- | --- |
| plane distance moved | about 1.09 Mm | within +-5% (the integrator); the throttle switch moved it less than that in THR |
| US fighter sea losses (depth kills) | 0 | 0-2 |
| US fighter losses | 0 | 0-3 |
| fighter bursts / fire ticks | about 9 / 160 | fire ticks 200-500 (THR1 had 344) |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | about 570 | 450-750 |
| torpedo / dive-bomb releases | 4 / 0 | 2-8 / 0 |
| Lexington moved | about 6.0 km | 5.5-7.5 km |
| mission end | none | none |

**Expected shape.** The integrator differs from Euler only where a resisting term would reverse a
component within one step, and at the 0.01 deadband. So most of the moved rows belong to the
throttle switch. That is RNG-coupled, and judged by band.

## 4. The pair, measured

`local\FI0_9000.log` (binary `local\fi0`, both OFF) and `local\FI1_9000.log` (`local\fi1`, both ON), one
tree, module directory checked in both logs. FI0 equals KE0b on every row.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| plane distance moved | 1,087,562 m | 1,086,038 m | +-5% | held (-0.14%) |
| US fighter depth kills | 0 | 1 | 0-2 | held |
| US fighter losses | 0 | 2 | 0-3 | held |
| fighter bursts / fire ticks | 9 / 159 | 11 / 246 | fire 200-500 | held |
| fighter kills | 5 | 8 | - | - |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 each | held |
| Kate deaths by category (1 / 5 / 6) | 10 / 0 / 6 | 7 / 1 / 8 | - | - |
| hit records | 569 | 570 | 450-750 | held |
| torpedo / dive-bomb releases | 4 / 0 | 4 / 0 | 2-8 / 0 | held |
| Lexington moved | 5982 m | 6517 m | 5.5-7.5 km | held |
| mission end | none | none | none | held |

**The two US losses:**
- Lexington-class01_sqn01|.-2 is shot down at 160 s at 1,479 m by D3A Val #5.1|.-2's rear gunner.
- Lexington-class01_sqn01 is depth-killed at 429 s, the fall docs/FREEFLIGHT_STALL_LAW.md reads as
  the image's own regime.

Both are within band, and RNG-coupled.

**Verdict:** `kFlightIntegratorBound` and `kDogfightThrottleBound` land **ON**. The flight rows barely
move: distance moved changes by 0.14%. The fighter rows move with the throttle wiring, within band.
The original-game check of the low-speed stall stays queued as validation, not as a gate.
