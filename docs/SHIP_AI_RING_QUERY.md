# Ship AI ring query: the target words of the ring scan's rating block

Packet cc9_ring_query, 2026-09-23. Base: main 0338e5a11. Switches `kShipAiRingQueryBound`
and `kUnitRadiusBound` in `src/game_hosts_ship_ai.cpp`, both on. Status: reconstructed,
build-tested and run-compared on USN04 and USN01. Not ABI-compatible and not game-validated.
Descriptive names are hypotheses.

## 1. What the ring scan rates with (009E7FC0 into 009E5DA0)

`BSP_ShipAi_ApproachResetSlotScores` 009E7FC0 copies the seventeen dwords at nested+127Ch into
the adapter's own block by value (REP MOVSD, 009E8192..009E81A2). It then overrides five items:

| Item | Source | Value |
|---|---|---|
| word 5, bearing | 009E8153 | nested+11DCh |
| word 6, window seconds | 009E814B | 20.0 (00CE3930) |
| word 7, ready horizon | 009E8161 | 60.0 (00CEB4B0) |
| byte +40h, require bearing | 009E8171 | 1 |
| byte +41h, use ready rounds | 009E8178 | 1 |

It calls `BSP_ShipAi_ApproachRateSlotBearing` 009E5DA0 once per slot at 009E81A7. Every other
word is whatever `BSP_ShipAi_ApproachFrameState` 009F1BC0 wrote into nested+127Ch this frame.
With a unit target, that is Armour, the class vtable[24h] torpedo armour, health, Length and
DamageThreshold (009F2A26..009F2A8F, see docs/SHIP_AI_OWN_CURVE.md section 4). Without one, it is
the no-target constants (009F2A91..009F2AC1).

Before this packet, the host's `ring_query()` kept the no-target constants for words 1 to 4, even
with a target. It also left word 16, the fire divisor, at 0; the image's no-target value is
10000.0 (009F2AB1). The bound path first stores 10000.0, then calls the shared
`fill_target_block_127ch()`. The own-curve fill from packet cc9_own_curve_target now calls the
same helper, so the two copies of 009F29E0..009F2AC1 cannot drift apart.

## 2. The avoidance refresh (009E6240)

`BSP_ShipAi_ApproachRefreshAvoidance` 009E9190 calls 009E6240 at 009E950F with nested+1238h
(009E94DF). That is the block that describes this ship, the same one the target curve reads, not
nested+127Ch. The host never reaches that call, because its traffic-record count is always 0.
Binding the block would change nothing that runs today, so it is not bound. The traffic records
are an open item.

## 3. The unit radius (unit+9C8h)

docs/SHIP_AI_TARGET_CURVE.md section 8 has the corrected second read. 00810F60 stores
2 x max(zmax, -zmin) of the model bounds at 0081106E, which is the full length, not a radius. It
falls back to class+A0h Length at 00811068. The host has no model bounds, so
`kUnitRadiusBound` feeds `unit_hull_length_09c8()`, which returns class Length, into word 1 of
nested+1238h (009F294B). This is labelled as a substitution.

Class vtable[24h]: the ship class has 009635D0 (class+6B4h UnderwaterArmour). Every other family
read (plane, runway, door, structure, wreckable) inherits the base 004407A0, which returns
class+4Ch Armour. That base getter is ledgered as `BSP_VehicleClass_GetUnderwaterArmourDefault`;
Ghidra still has the older plane-specific name.

## 4. Measurement

All runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides. The control switches both terms off.
The refills counter is ignored.

Predictions written before the runs:
- The rating against a Val would be capped at 220 x 4 = 880. This held.
- Standoffs would be unchanged. This held.
- The target curve would stay 0. This held.
- USN01 would be unchanged. This held.
- Heading changes would fall on most ships because capped slots tie. This was WRONG in
  direction.

USN04, 4700/4500 frames:

| Ship | Heading changes ctl | trt | Ring winner changes ctl | trt |
|---|---|---|---|---|
| Northampton-class01 | 15 | 18 | 15 | 18 |
| Northampton-class02 | 41 | 116 | 6 | 9 |
| Fletcher-class01 | 57 | 28 | 25 | 11 |
| Fletcher-class02 | 39 | 71 | 9 | 14 |
| Fletcher-class03 | 58 | 63 | 11 | 16 |
| Fletcher-class04 | 42 | 69 | 11 | 19 |
| York-class01 | 5 | 33 | 5 | 11 |
| York-class02 | 37 | 43 | 9 | 19 |
| Total | 294 | 441 | 91 | 117 |

The winner-change columns came from a temporary per-scan log line in a second, identical pair.
The line was not committed. The heading counts in that pair matched the table exactly.

Term by term:
- **Only the rating moved first.** On every ship, the first differing scan is a rating
  difference, and the winner differs at that scan or later. Before it, both runs hold
  identical ratings and winners.
- **The control was not tied.** Its block described a phantom target with 10000 HP, 0 armour and
  a zero fire divisor. It rated slots in a graded way, for example 3690, 5541 and 8621. In 97
  scans it hit its own 40000 cap. That grading steered the sum toward one slot.
- **The treatment saturates.** Against a real Val, the excess damage is clamped to the Val's
  health, and the output cap is health x 4. In 405 of 622 nonzero scans the maximum is exactly
  880, and in 160 it is 723.57. Every slot that can fire then normalises to the same value.
  The winner falls to the bearing, penalty and evade words, which move from tick to tick, so
  winners switch more often. Fletcher-class01 is the exception: there the tie removed a
  control-side oscillation.
- **Scans 127 to 141 on all eight ships** rate nonzero in the control and 0 in the treatment.
  The output cap is health x 4, so a held target with health 0, or armour at or above the
  damage band, zeroes it. Which of the two it is has not been checked.
- **The radius has no effect today.** A ring-only binary reproduced the full treatment line for
  line, apart from one platform message count. The target curve stays 0 on every ship, and the
  avoidance call is unreached.

Unchanged: standoffs first and last on all eight ships, 103 queued hits, 15656.2 damage,
9 deaths, and Lexington at 141.9 HP. Two aircraft rows moved because ship positions moved:
Kate #6.1 sank at 220.76 s instead of 220.81 s, and one movieval wingman was killed by
Northampton-class02 instead of Fletcher-class04.

USN01, 3200/3000 frames: the logs match line for line, apart from one platform message count.

## 5. Open

- Which held target yields a zero rating for scans 127 to 141.
- The traffic records that would let 009E9190 reach 009E6240.
- The four allow bytes at [0080E160(unit)+220h..+223h] still have no producer; the host forces
  them to 1.

## 6. Corrections, 2026-09-23 (packet cc9_ship_traffic)

- Section 5 said the four allow bytes at [0080E160(unit)+220h..+223h] have no producer. That is
  wrong. The command-controller constructor 00720180 stores 1 into all four
  (007202FD..00720312). The base message apply 0071C1E0 rewrites them from sub-kinds 3 to 6,
  which the Lua bindings ArtilleryEnable (0089C590) and TorpedoEnable (0089C8F0), and two
  unnamed ones, send. The host's forced 1 is the constructor default. docs/SHIP_AI_TRAFFIC.md
  section 6.
- Section 5's open item on scans 127 to 141 is answered: the held target was D3A Val #1.1. It
  died at 156.20 s, just before scan 127, and a target with health 0 zeroes the output cap. It
  was not armour.
- Section 2's statement that the traffic-record count is always 0 held for the host before this
  packet. The records are now bound; see docs/SHIP_AI_TRAFFIC.md.
