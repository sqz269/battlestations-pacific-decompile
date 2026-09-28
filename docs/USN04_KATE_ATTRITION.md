# USN04's Kate attrition: who kills them, and whether the gunnery is the image's

Packet `cc9_usn04_kate_attrition`, read half. Worker cc9-gunnery3, 2026-09-27, on main 5eaf8aa81.
Ghidra was read only. Nothing is bound. docs/SCENE_CONTENTS_HOSTS.md section 27.2 handed this
over: all 16 Kates die (6 after releasing), so the mission's torpedo releases are 6 of 16.

The log is `local\RA_OFF_USN04.log` in this worktree: main 6e1a50650 plus `kGunIdleRestBound`
OFF, which is gameplay-identical to main. USN04 4700/4500, lockstep 0.05, idle player,
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`. Totals: 5482 shots, 739 hits, 44 deaths, first shot
91.60 s, matching the reference logs of cc9-plane2 and cc9-ships2.

## 1. The 16 Kate deaths

Categories: 0 plane gun, 1 AAMACHINEGUN, 5 FLAK, 6 LIGHTARTILLERYFLAK. `first hit` is the death
table's `first_damage`. Every row's damage sums to 220, the Kate's health.

| Kate | first hit | death | alt m | killer | cat | range m | hits c0/c1/c5/c6 | released |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| #2.1 | 115.20 | 123.80 | 31 | Lexington | 5 | 801 | 0/1/7/4 | no |
| #4.1 | 118.65 | 124.45 | 28 | Fletcher-class05 | 1 | 748 | 0/1/2/8 | no |
| #4.1\|.-4 | 118.15 | 127.15 | 24 | Fletcher-class05 | 1 | 618 | 0/6/2/2 | no |
| #2.1\|.-3 | 126.25 | 127.15 | 24 | Lexington | 1 | 625 | 0/7/0/0 | yes |
| #2.1\|.-2 | 124.20 | 127.95 | 22 | Lexington | 6 | 557 | 0/3/1/7 | no |
| #2.1\|.-4 | 127.95 | 129.65 | 32 | Lexington sqn01 fighter | 0 | 395 | 4/3/2/0 | no |
| #4.1\|.-2 | 124.70 | 130.40 | 20 | Fletcher-class05 | 1 | 331 | 0/7/0/5 | yes |
| #4.1\|.-3 | 129.90 | 141.35 | 248 | Fletcher-class08 | 1 | 504 | 0/5/7/4 | yes |
| #6.1 | 195.01 | 201.81 | 40 | Lexington | 6 | 930 | 0/0/6/8 | no |
| #6.1\|.-3 | 204.21 | 205.16 | 27 | Lexington | 6 | 759 | 0/3/2/6 | no |
| #8.1 | 201.46 | 206.06 | 106 | Fletcher-class05 | 6 | 838 | 0/0/6/14 | no |
| #6.1\|.-2 | 202.11 | 207.01 | 22 | Fletcher-class01 | 6 | 1236 | 0/3/0/9 | yes |
| #6.1\|.-4 | 206.06 | 207.46 | 24 | Lexington | 1 | 587 | 0/6/2/2 | yes |
| #8.1\|.-3 | 208.36 | 211.31 | 34 | Northampton-class03 | 1 | 654 | 0/3/2/6 | no |
| #8.1\|.-2 | 209.61 | 213.06 | 25 | Northampton-class03 | 1 | 503 | 0/9/0/2 | yes |
| #8.1\|.-4 | 209.56 | 214.06 | 22 | Northampton-class03 | 1 | 421 | 0/6/0/5 | no |

The `released` column comes from the log's `release census` rows: #2.1\|.-3, #4.1\|.-2, #4.1\|.-3,
#6.1\|.-2, #6.1\|.-4 and #8.1\|.-2.

- **Two waves.** 8 Kates die at 123-141 s, and 8 at 201-214 s.
- **Ship AA kills 15 of 16.** By category: 9 AA machine guns, 5 light flak, 1 flak. A Lexington
  fighter kills the sixteenth.
- **At torpedo-run height and close range.** 14 of the 16 die at 20-40 m, at 330-930 m (one at
  1236 m).
- **Fast once hit.** A Kate lives 0.9 to 11.5 s after its first hit. The mean is 4.8 s.
- **The kill takes 7 to 20 damaging hits.** A category-1 hit applies about 30 after armour 6. A
  category-6 flak blast applies up to 29 (35 base minus 6), falling off over 35 m.

## 2. The gunnery terms against the image

docs/AA_LETHALITY_AUDIT.md section 2 took this chain term by term on 2026-09-23. It is re-checked
here against what has landed since.

| term | host now | image | state |
| --- | --- | --- | --- |
| Gunner skill | none applied to AA | the mission script sets SPVeteran (row 2) on the US ships | the next three rows are exact at this row |
| AAFlakBot error `008FDBE0` | 0 | SPVeteran GoodRatio 1, AngleErr and DistErr 0 | exact |
| Throw cone `0073031D` | bound (`cc9_bullet_throw`) | `Throw * BulletThrowMul`, SPVeteran mul 0 | exact |
| AAGunner swinging error `00902B38`-`00902EF7` | unbound | SPVeteran AngleDiffErrorRatio 0, ConstAngleError 0 (docs/FLAK_PROXIMITY_BURST.md 5) | exact: zero at this row |
| Negative-vertical halving `00902F62` | bound | `00902F76` | exact |
| Target velocity | world velocity, `kAaTargetWorldVelocityBound` ON | `007BBB70`, `unit+AC8h` | exact |
| Turn-rate average `0085E4D0` | not bound | averages V with its rotated image for a turning plane | open; zero on a straight run, which is how 14 of 16 die |
| Flak detonation | proximity burst, `kFlakProximityBurstBound` ON | `0070C370` / `0070C210` | bound; the unlocked 10%-per-tick passing rule beyond 50 m is not modelled |
| Barrel count | the mesh's `"fire"` items (`007325A0`) | the same | exact where the mesh loads; the Bofors (device 42) fires 2, the quad Bofors (264) 4 |
| Muzzle origin | per-barrel offsets bound (`cc9_muzzle_offsets`); 2084 of 5482 shots fall back to the shared origin | `007307A0` from the model's node | partial; the fallback devices carry no mesh path (54, 80, 85, 87, 89, 93, 95, 98, 101) |
| Damage per hit | bullet classes 42 (MG) and 44 (Flak), blast 35 over 35 m, Kate armour 6, health 220 | the same `bulletclasses.lua` and Kate class rows | exact |

**Verdict: the AA that kills USN04's Kates is the image's.** At SPVeteran every aim-error and
dispersion term the image has is zero, and each rate, blast and damage term is either bound or
exact. The one open term, the turn-rate average, cannot act on the straight low run where 14 of
the 16 die. No gunnery term is bound by this packet.

## 3. Where the attrition comes from instead

These are plane-side observations from the same log, handed to the plane owners.

- **The Kates dwell in the aim state.** The per-Kate torpedo lines show 150-260 aim ticks, or
  7.5 to 13 s. The aim-completion flag `aim_complete_2Ch` (009D15F0) is 0 for all 16 Kates,
  including the six that release.
- **They fly slowly there.** 3 of the 6 releases are at 29-36 m/s, against the class
  `TravelSpeed` 61.1 m/s and `max_spd` 69.4 m/s.
- **Each Kate survives about 4.8 s under fire**, so an aim run of 7.5-13 s near the escorts
  loses most of them before release.
- **The next reads are 009D15F0's completion clause and the aim-state throttle.** Both belong to
  the plane owners. docs/SCENE_CONTENTS_HOSTS.md section 29 items 6-7 and the throttle history in
  that doc's earlier sections are the entry points.

## 4. Predictions (for whichever change lands next)

- **A gunnery-side change** that keeps SPVeteran aim leaves the 16 Kate deaths and the 6 releases
  unchanged in count. Death times may shift by under one salvo interval, about 2 s.
- **A plane-side change** that completes the aim state sooner, or holds the Kates near
  `TravelSpeed` through it, should raise USN04 releases above 6 of 16. At about 4.8 s of survival
  under fire, halving the aim dwell should roughly double them.
- **Deaths.** USN04's 44 deaths fall only if released Kates turn away faster than the AA kills
  them. All six released Kates in this log still die, by 214.06 s.
