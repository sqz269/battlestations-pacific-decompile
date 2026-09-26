# The dogfight throttle wiring, re-taken on current main (packet cc9_dogfight_throttle_retake)

2026-09-25. `kDogfightThrottleBound` wires `007B4ED0` `BSP_PilotPlan_SetDirectThrottle` into the
dogfight aim tick's head-on arm and the maneuver tail's full throttle
(docs/DOGFIGHT_GUN.md, docs/PILOT_THROTTLE_SLOT.md). Its last ON measurement, S1T, was taken on
2026-09-23 at about 08:57. In it the Yorktown fighter leader and `.-2` flew into the sea inside aim, at
|v| 105.7 and 100.5.

## 1. What changed since S1T

- **The speed-hold multiplier fix** (`kPilotThrottleSlotBound`) is ON on main. S1T had it too.
- **The pilot's vehicle and terrain avoidance arms** landed at 19:03 that day (`b4a0e984d`, both
  switches ON). `0099F1C0` is the only floor the image puts under aim's dive: `009F9ED0` caps the
  pitch upward only (docs/PLANNER_HEADING_WRITES.md 7.2). S1T had no floor at all.
- **The planner's heading writes and yaw-base gate** are ON (docs/PLANNER_HEADING_WRITES.md).
- **The friendly-in-line hold's call site** is applied (docs/E2_RELEASE_BISECT.md).
- **The attackrun squadron probe** is at its verdict (docs/ATTACKRUN_SQUADRON_PROBE.md).

## 2. Predictions, written before the pair

The pair is `kDogfightThrottleBound` OFF against ON, one tree (current main), E2 9000,
`BSP_GUNNERY_RNG_STREAMS=1`.

| row | prediction for the ON side |
| --- | --- |
| US fighter water contacts | 0, the same as OFF; the avoidance arm now bounds aim's dive |
| US fighter losses | 0-3 |
| fighter bursts / fire ticks | within -50% to +100% of OFF |
| Kate / Val deaths | 12-16 / 12-16 |
| hit records | 450-700 |
| torpedo / dive-bomb releases | 2-8 / 0 |
| Lexington moved | 5.5-7.5 km |
| mission end | none |

If a US fighter reaches the water with the switch ON, the verdict is OFF. The next read is then
`0099F1C0`'s look-ahead depth against a full-throttle dive.

## 3. The pair, measured

`local\THR0_9000.log` (binary `local\thr0`, switch OFF) and `local\THR1_9000.log` (`local\thr1`,
switch ON), built from `2d1e2f167`, which is main `51e22e56d` plus the squadron probe ON. The module
directory was checked in both logs.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| US fighter water contacts or depth kills | 0 | **1**: Lexington-class01_sqn03 | 0 | **missed** |
| US fighter losses | 0 | 2 | 0-3 | held |
| fighter bursts / fire ticks | 9 / 159 | 13 / 344 | -50% to +100% | fire ticks **missed, high** (+116%) |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 / 12-16 | held |
| hit records | 586 | 616 | 450-700 | held |
| torpedo / dive-bomb releases | 5 / 0 | 7 / 0 | 2-8 / 0 | held |
| Lexington moved | 6281.42 m | 6190.88 m | 5.5-7.5 km | held |
| terrain-avoidance ticks / bands / water ticks | 1315 / 3827 / 0 | 1386 / 3956 / 0 | - | - |
| mission end | none | none | none | held |

**The loss that decides it.** Lexington-class01_sqn03 dies at 340.39 s by the depth kill
(`alt = -31.61`, below -30 m; `007CE3A7`, packet cc9_water_surface_law).
- Its last transition was maneuver to aim, at 410 m from D3A Val #5.1, the target it was chasing.
- One second before, the surface probe shows alt 4.5 m, vy -34.6 m/s, speed 57.7 m/s, pitch +0.79 rad
  nose up (commanded +0.78), and throttle 1.0.
- That is a stall mush, not a dive. The fighter points 45 degrees up at full throttle and sinks at
  34 m/s. S1T's fighters instead went in at 100-106 m/s. The avoidance arm did not flag a water tick.
  It bands the pitch, and the pitch was already nose-up.
- The other loss, Lexington-class01_sqn01 at 192.81 s, is a gun kill: D3A Val #5.1|.-4's rear gunner,
  280 damage taken. It is RNG-coupled and judged by band.

**Verdict:** `kDogfightThrottleBound` stays **OFF**, as the prediction's rule says. The dive half of
the S1T blocker is gone: the terrain arm now bounds the pitch, and no fighter goes in fast. What is
left is a low-speed stall inside aim with the direct throttle wired.

**Next read.**
- Why a fighter at full throttle and 58 m/s with its nose 45 degrees up cannot hold altitude. Its
  class stall speed and the planner's pitch arm under `plan+2BCh` = +0.78 are the candidates.
- Whether the image's aim tick commands that pitch at all. `009F9ED0` would give +0.78 only for an
  aim point well above, and the Val was near the sea.
- The command target log names D3A Val #5.1, so the aim point's source, the approach's `vtable[0]`,
  is the first thing to check.
- The same probe line reads `cmd_alt=661.5`, the surface climb-out's commanded altitude (packet
  cc9_pilot_surface_climbout). So the +0.78 pitch is probably the climb-out law pulling up at low
  height, not aim's `009F9ED0`, and the loss is a climb-out at a speed too low to climb. Read that
  law's speed gate against the direct throttle's value on leaving aim first.
