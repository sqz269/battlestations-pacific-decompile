# Attributing the 2026-09-26 USN04 and USN01 moves (packet `cc9_usn04_usn01_attribution`)

The 2026-09-26 reference section (`docs/GAME_EXECUTABLE.md`) left two moves that no measured
pair explained:
- **USN04:** deaths 30 to 32, landed between `51e22e56d` and the recon tree.
- **USN01:** hit records 135 to 126 and torpedo drops 2 to 1 since the 2026-09-24 section.

This packet runs one series per mission on one build of main `5f7daab07`: the control with
everything landed, and one run per candidate switch with only that switch OFF.

## 1. The series

Binaries, all from `5f7daab07` with one switch flipped for the build and restored after it:

| binary | the one switch OFF |
| --- | --- |
| `local\at_ctl` | none (control) |
| `local\at_throw` | `kBulletThrowBound` |
| `local\at_aabb` | `kAabb0085cdb0Bound` |
| `local\at_gyro` | `kTorpedoGyroHeadingBound` (the torpedo run-line steer) |
| `local\at_avoid` | `kAvoidanceDummyAiGateBound` |

`kDogfightEmptySquadronClearBound` is not a candidate. USN01's five Mavis deaths are all ship AA
(Northampton, SaltLakeCity, Dunlap), with no fighter involved.

Every run: `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, the 1600x900 harness default, one
run queued at a time. USN04 is 4700/4500 and USN01 3200/3000.

## 2. Predictions, written before the runs

| switch | USN04 rows it can touch | USN01 rows it can touch |
| --- | --- | --- |
| `kBulletThrowBound` | AA and fighter hits, so plane deaths: Lexington's unscaled 0.4/0.57 degree guns and the fighters' 0.57 degree cone; the escorts' level-2 AA has multiplier 0 | the AA hit records on the Mavis, from whichever ships run at levels 0 or 1 |
| `kAabb0085cdb0Bound` | AA line-of-fire blocks and refusals, so AA hits (E2 moved 37 to 38) | the same for the Mavis engagements |
| `kTorpedoGyroHeadingBound` | none, or cascade only: USN04 has no ship torpedo launch against a ship in range | none: USN01's torpedo drops are aircraft drops, not bot launches |
| `kAvoidanceDummyAiGateBound` | plane flight paths (the terrain arm), so AA exposure, drops and deaths | the Mavis flight paths, so AA exposure (hits) and the torpedo drops |

Expected attribution:
- **USN04 deaths +2:** the bullet throw or the avoidance gate; the AABB possibly +1.
- **USN01 hits -9:** the bullet throw.
- **USN01 drops -1:** the avoidance gate.
- **RNG coupling:** where two OFF runs each undo part of a move, the move is coupled, and the
  earlier landing is named as the first flip.

## 3. The runs

All ten logs show `window resolution override fit: 2560x1440 -> 1600x900`, their own
`local\at_*` module directory, exit 0 and every mission step. The runs were taken
2026-09-25 21:58..22:40, one at a time. The binaries were built from main `5f7daab07`.

**USN04 4700/4500** (`local\at_*_usn04.log`):
| run | exit | deaths (IJN/US) | hit records | shots | damage | first hit | torpedo drops | task releases | plane water | deaths by category |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| at_ctl | 0 | 31 (31/0) | 528 | 3985 | 7215.6 | 93.10 | 0 | 16/6 | 14 | {(1, 0): 9, (1, 1): 10, (1, 5): 2, (1, 6): 10} |
| at_throw | 0 | 31 (31/0) | 512 | 4776 | 7099.5 | 93.25 | 0 | 16/4 | 14 | {(1, 0): 12, (1, 1): 11, (1, 5): 1, (1, 6): 7} |
| at_aabb | 0 | 31 (31/0) | 522 | 4455 | 6953.2 | 93.10 | 0 | 16/5 | 14 | {(1, 0): 11, (1, 1): 8, (1, 5): 2, (1, 6): 10} |
| at_gyro | 0 | 31 (31/0) | 528 | 3985 | 7215.6 | 93.10 | 0 | 16/6 | 14 | {(1, 0): 9, (1, 1): 10, (1, 5): 2, (1, 6): 10} |
| at_avoid | 0 | 31 (31/0) | 526 | 4602 | 6996.0 | 93.10 | 0 | 16/5 | 14 | {(1, 0): 11, (1, 1): 9, (1, 5): 1, (1, 6): 10} |

**USN01 3200/3000** (local\at_*_usn01.log):
| run | exit | deaths (IJN/US) | hit records | shots | damage | first hit | torpedo drops | task releases | plane water | deaths by category |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| at_ctl | 0 | 7 (5/2) | 141 | 458 | 2690.0 | 53.70 | 2 | 5/4 | 3 | {(0, 1): 2, (1, 1): 5} |
| at_throw | 0 | 7 (5/2) | 142 | 419 | 2690.0 | 53.70 | 2 | 5/4 | 3 | {(0, 1): 2, (1, 1): 5} |
| at_aabb | 0 | 7 (5/2) | 141 | 458 | 2690.0 | 53.70 | 2 | 5/4 | 3 | {(0, 1): 2, (1, 1): 5} |
| at_gyro | 0 | 7 (5/2) | 141 | 458 | 2690.0 | 53.70 | 2 | 5/4 | 3 | {(0, 1): 2, (1, 1): 5} |
| at_avoid | 0 | 7 (5/2) | 143 | 475 | 2690.0 | 53.70 | 2 | 5/4 | 3 | {(0, 1): 2, (1, 1): 5} |


The deaths-by-category column is keyed by (party, killing category): party 1 is IJN and 0 is US;
the categories are 0 fighter guns, 1 AA, 5 flak and 6 dual-purpose.

## 4. What the series shows

**No candidate moves a headline count on current main.**
- **USN04:** deaths are 31 in all five runs, and torpedo drops 0. The switches only
  reshuffle the killing categories and the hit records:
  - `kBulletThrowBound` OFF: 528 to 512 hits, 3985 to 4776 shots. The throw makes the AA fire
    fewer rounds for more hits, through the target cascade.
  - `kAabb0085cdb0Bound` OFF: 522 hits.
  - `kAvoidanceDummyAiGateBound` OFF: 526 hits.
  - `kTorpedoGyroHeadingBound` OFF: identical to the control in every row. No ship torpedo is
    launched at a ship in USN04, as predicted.
- **USN01:** deaths are 7, torpedo drops 2, damage 2690.0 and the first hit 53.70 s in all five
  runs. Hits are 141 to 143: the throw moves one hit and 39 shots, the avoidance gate two hits
  and 17 shots. The AABB and the gyro are identical to the control.

**The two flagged moves are not on current main.** The control already differs from the
2026-09-26 reference, which was built on `fad22c424`:

| mission | row | reference (`fad22c424`) | control (`5f7daab07`) |
| --- | --- | --- | --- |
| USN04 | deaths / hits | 32 / 476 | 31 / 528 |
| USN01 | hits / torpedo drops / deaths | 126 / 1 / 5 | 141 / 2 / 7 |

The only landings between them are three dogfight-engaged merges: `347ef9007`, `44631c067` and
`da4ba1baf`, the last with the planner's corpse-chase fix. USN01's two new deaths are US
ScoutDauntless scouts shot down by Convoy2's AA at 124.25 and 130.50 s. The scouting flight now
reaches the convoy.

## 5. Attribution

- **USN04 deaths +2 (30 on `51e22e56d`, 32 on `fad22c424`).** Not the bullet throw, the AABB
  line of fire, the torpedo steer or the avoidance gate: removing each alone leaves the count
  unchanged. By elimination, the move belongs to the flight and dogfight landings in that
  window, the dogfight-engaged merges `3a6d8b847`..`7c6e16188`. Their count is coupled to the
  air battle's cascade, and it has already moved back to 31 with the next three dogfight merges.
  **Coupled across landings; no single switch flips it.**
- **USN01 hits 135 to 126 and torpedo drops 2 to 1.** Not these four switches either: each alone
  moves hits by at most 2 and drops by 0 on current main. The reference's values were a state of
  the flight landings. The next three dogfight merges took USN01 to 141 hits and 2 drops, and
  added the two ScoutDauntless losses. **Attributed to the flight and dogfight merges,
  cascade-coupled.**
- **Caveat.** Each switch was removed on current main, not on the `fad22c424` tree the reference
  was built on. A switch whose effect depends on the three later dogfight merges would show here
  differently. The elimination holds for current main.
- No switch revealed a host bug, so there is no code change.

**Recommendation for the next reference:** take it on current main. The control rows above
(USN04 31 / 528, USN01 7 / 141 / 2) are what it will read, since none of these switches changes
them.