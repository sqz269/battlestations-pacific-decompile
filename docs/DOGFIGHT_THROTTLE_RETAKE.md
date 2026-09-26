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
