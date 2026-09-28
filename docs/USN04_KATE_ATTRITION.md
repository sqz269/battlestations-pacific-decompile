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

- **The Kates dwell in the aim state.** The per-Kate torpedo lines show 150-260 aim ticks. These
  are pilot think ticks of 0.1 s, not 0.05 s frames (section 7), so 15 to 26 s. The aim-completion flag `aim_complete_2Ch` (009D15F0) is 0 for all 16 Kates,
  including the six that release.
- **They fly slowly there.** 3 of the 6 releases are at 29-36 m/s, against the class
  `TravelSpeed` 61.1 m/s and `max_spd` 69.4 m/s.
- **Each Kate survives about 4.8 s under fire**, so an aim run of 15-26 s that ends inside the
  escorts' range loses most of them before release.
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

## 5. Correction to section 3: the slow releases are dead Kates, and USN04 drops one torpedo

Read after the first commit, from the same log. It supersedes section 3's first two bullets and
section 4's plane-side prediction.

- **Five of the six task "releases" come from dead aircraft.** For each releaser, the log line
  order of its `plane death mode` row against its `release census` row shows:

| Kate | died | death mode | release after death | speed at release |
| --- | --- | --- | --- | --- |
| #2.1\|.-3 | 127.15 s | powerlost | yes | 33.50 m/s |
| #4.1\|.-2 | 130.40 s | explosion | yes (the delayed-explosion window) | 72.05 m/s |
| #4.1\|.-3 | 141.35 s | powerlost | **no, released alive** | 73.05 m/s |
| #6.1\|.-2 | 207.01 s | powerlost | yes | 35.04 m/s |
| #6.1\|.-4 | 207.46 s | powerlost | yes | 36.36 m/s |
| #8.1\|.-2 | 213.06 s | powerlost | yes | 28.88 m/s |

- **The 29-36 m/s speeds are power-lost gliders.** #2.1\|.-3's velocity census reads 84 m/s at
  aim tick 151 with throttle 1.00, then 41.6 m/s at tick 201 with throttle 0.00, after its death
  at 127.15 s. docs/PLANE_DEATH_MODES.md: a power-lost aircraft glides at throttle 0 under pilot
  steering. The aim state's throttle is not at fault.
- **None of them spawns a torpedo.** `summary mission gunnery torpedo_drop drops=1`, and the one
  `torpedo from` row is #4.1\|.-3's. The torpedo task's `releases=6` counts task-level releases,
  including the dead aircraft whose rounds never spawn. docs/PLANE_DEATH_MODES.md step 6 says the
  image refuses every release from a dead aircraft at `007CEA1C`.
- **So USN04 at 4700/4500 puts one torpedo in the water, not six.** docs/SCENE_CONTENTS_HOSTS.md
  27.2's "6 of them after releasing" read the task counter. The `torpedo_drop drops` line is the
  count to use.
- **The aim-state rules are the image's.** 009D15F0 writes no throttle: its `+2C8h` output is the
  bank cap (docs/TORPEDO_AIM_TICK.md, correction from `cc8_plane_pose_throttle_altitude`). The
  throttle is the planner's speed hold, with the target at `TravelSpeed * NewTravelSpeedMul`,
  which runs a live Kate at full throttle (1.00 in the census). The release gate is the lead flag,
  `envelope > range`, with the SPNormal 450/650 m pair and the aspect scale
  (docs/KATE_RELEASE_CONDITION.md). SPNormal is the image's row for these Kates
  (docs/IJN_SKILL_ROWS.md).
- **The Kates die before range falls under the envelope.** 15 of 16 are killed at 330-930 m,
  mostly outside 450 m, in the last seconds of an aim run of 15-26 s.

**Plane side: nothing to bind.** Every term on the aim run that was read is the image's.

## 6. The four gunnery comparisons, closed

| comparison | finding | state |
| --- | --- | --- |
| AA gunner error weight and spread, with the ship-globals modifier | `00902B38`-`00902EF7`: SPVeteran AngleDiffErrorRatio 0 and ConstAngleError 0 (docs/FLAK_PROXIMITY_BURST.md 5). The ship-globals `AAGunnerErrorModifier` enters as the 0.05 s + 0.1 s/km time bias in the intercept (docs/AA_LEAD.md), which is bound. The +750h/+754h divisor applies to ship targets only | exact at the row USN04 sets |
| Flak distance error at the veteran row | `008FDBE0` / `bot+60h` -> the burst's `[+290h]`: robots.lua AAFlakBot SPVeteran DistErr 0/0/0 | exact; the host's 0 is the row's value |
| The muzzle-origin placeholder | per-barrel offsets are bound (`cc9_muzzle_offsets`). The 2084 fallback shots all come from devices with no `Mesh`: 54 (DC launcher), 80/85/87/89 (bomb racks), 93/95/98/101 (plane guns). **No ship AA mount falls back** | exact for every ship-AA kill of a Kate; the one fighter kill (category 0) uses the fallback |
| The AA lead from the body axis | replaced by the world velocity `007BBB70` (`kAaTargetWorldVelocityBound` ON, docs/AA_LETHALITY_AUDIT.md 3) | exact |

**Verdict for the packet.** USN04's Kate attrition is the image's AA, at the image's SPVeteran
row, against Kates flying the image's aim run at the image's SPNormal release distances. No term
is bound, and no pair is needed.

**Predictions this read stands behind:**
- **USN04 4700/4500.** Kate deaths 16, torpedo drops 1, 44 deaths and 739 hits, unchanged by
  anything in this packet.
- **USN13 3200/3000.** The reference log (worktree cc9-plane2, `local\RM_USN13.log`) reads task
  `releases=2` but `torpedo_drop drops=0`. No torpedo is in the water there either.
- **For the next owner.** The task's `releases=` counter should either skip dead aircraft or be
  labelled task-level. It is a log-reading hazard, not a gameplay divergence.

## 7. The aim state's law (packet `cc9_torpedo_aim_state_law`, read)

The lead's four questions, answered from the same log and the landed reads of 009D15F0
(docs/TORPEDO_AIM_TICK.md, docs/KATE_RELEASE_CONDITION.md, docs/PILOT_BOT_THROTTLE_ARM.md).

- **What speed and throttle the image commands in the aim run.** 009D15F0 writes no throttle and no
  target speed. Its `+2C8h` output is the bank cap. The planner re-seeds the target speed every
  think to `TravelSpeed * NewTravelSpeedMul` (`0099B503`/`0099B511`), and the speed-hold arm cuts
  the throttle only when the plane is faster than that. A live Kate therefore flies the run at full
  throttle. The census bears this out: #2.1\|.-3 reads throttle 1.00 at 90, 93 and 98 m/s on aim
  ticks 1, 51 and 101, and 84 m/s at tick 151.
- **Why the host's Kates fly 29 to 36 m/s.** They are dead. The four slow releasers lost power at
  127.15 s, 207.01 s and 207.46 s, and #8.1\|.-2 at 213.06 s. They then glide at throttle 0
  (#2.1\|.-3: 41.6 m/s, throttle 0.00 at tick 201). No live Kate is slow.
- **What completes the aim, and why no Kate sets the flag.** `state+2Ch` is set at `009D236E` when
  either clause holds:
  - the remaining range less a ramp of up to half the release distance (225 m on the 450 m slot,
    325 m on the 650 m slot) falls under the height still to lose;
  - the steering delta in radians exceeds the time to target.
  It marks the end of the run, past the release point, and the release does not wait for it. The
  one live releaser, #4.1\|.-3, went from aim to goaway without it. No Kate sets it because every
  run ends in a death at 330-930 m, before range falls under about 250-350 m.
- **How long the image's aim run lasts.** The aim ticks are pilot thinks: `kPilotThinkInterval` is
  0.09 s (`include/bsp/pilot_plan_slots.hpp`), so at 0.05 s frames a think runs every second frame,
  0.1 s. #4.1\|.-4's census shows range 2198, 1767, 1371 and 885 m at ticks 1, 51, 101 and 151,
  about 86 m/s per 0.1 s tick. Entry is at about 2200 m. At that closing speed the 450 m lead flag
  opens after about 20 s. That matches docs/KATE_RELEASE_CONDITION.md, where the Yorktown Kates
  opened it 19.5-26.1 s after aim entry.

**Nothing to bind.** Every term of the aim run that was read is the image's. USN04's release count
is set by how many Kates survive 15-26 s of closing on escorts at SPVeteran, and one does.

**Predictions (no switch).** USN04 4700/4500: aim ticks 150-260 per Kate, 16 Kate deaths,
1 torpedo drop (6 task releases), 44 deaths, 739 hits, all unchanged. USN13 3200/3000: 27 deaths,
2 task releases, 0 drops. USN02: identical.

## 8. The two small open gunnery terms (packet `cc9_aa_small_terms`, read)

### 8.1 The flak passing rule, `0070C7B6`-`0070C806`: unreachable, proven

docs/FLAK_PROXIMITY_BURST.md step 7 left it unmodelled because "it cannot act once a lock exists".
The listing, read from disk bytes, proves the stronger claim that its 10% burst can never fire.

- **What XMM0 holds at the rule.** `0070C4B1`/`0070C4F7` seed `[ESP+20h]` = FLT_MAX (`00D7A248`)
  before the entity search. The search's only write to it is `0070C671 MOVSS [ESP+30h],XMM0`,
  made after four pushes, so it is `[ESP+20h]`. That write is on the lock path, right after
  `0070C661` sets the lock byte `+288h` (`[ESI+44h]`, ESI = proj+244h). `0070C6F7` loads it into
  XMM0. So XMM0 is the squared distance of an entity locked this tick, or FLT_MAX.
- **The two ways in.** `0070C705` tests the lock byte:
  - no lock: jump to `0070C7B4`, then the rule;
  - lock with the aim point still more than one step ahead: `0070C7AD` shortens the remaining
    distance, then `0070C7B2` jumps to the rule. When the aim point is within the step, the round
    bursts at `0070C79B` instead.
- **The gate.** At `0070C7B6` the rule does nothing unless `90000 > XMM0`, which needs a lock this
  tick. The search reaches its lock for every entity it finds (`0070C611`-`0070C63D`, then
  `0070C661`), so an unlocked tick always carries FLT_MAX.
- **The only tick it can act on is the lock tick.** There it compares the stored `+284h`
  (`[ESI+40h]`) with XMM0. The flak constructor `0070CAE0` seeds `+284h` = FLT_MAX
  (`0070CAE8`, stored at `0070CB30`), and nothing else writes it before the first lock. So
  `+284h >= d2` always holds, and the rule takes the store branch (`0070C7D0`), never the
  receding branch with its `U(0, 100) < 10` draw at `0070C7F7`.
- **After the lock tick,** later ticks skip the search, so `[ESP+20h]` stays FLT_MAX and the gate
  fails.

**Nothing to bind.** The host's omission is exact, and it also draws nothing from the shared
stream here, as the image does not.

### 8.2 The turn-rate average, `0085E4D0` from `00901C20`

- **The rule** (docs/AA_LEAD.md 2): when the target is a plane whose world angular rate at
  `+AF8h..+B00h` has a square above 0.001 (`00D7A23C`), about 1.8 degrees per second,
  `0085E4D0` rotates the velocity by that rate over the flight time. `00901C20` then averages
  it with the unrotated velocity, times 0.5. `0085E4D0` itself returns early when the rate's
  length is below `[00D7A350]`.
- **The host.** It keeps each plane's body angular rate (`plane_body_angular`, written by the
  rate law near `src/game_hosts_units.cpp:17173`), but not the world-frame rate `007D9C80`
  produces at `+AF8h`. The AA lead therefore always takes the straight branch.
- **Reach on the Kate kills.** It is inert where 14 of the 16 Kates die. #4.1\|.-4's aim census
  commands heading 2.4218, 2.4429, 2.4649 and 2.5033 rad on ticks 1, 51, 101 and 151, which is
  about 0.3 degrees per second, far under the 1.8 degrees per second gate. It would act on turning
  Zeros and on the post-release turn-away.
- **Cost of a binding.** It needs the world-rate transform `007D9C80` published on the unit, and a
  transcription of `0085E4D0`: a look-at and a Z-rotation matrix built from the normalised rate,
  not yet read in full. It lives in both `src/game_hosts_units.cpp`, which
  cc9-ships2 has leased, and `src/game_hosts_gunnery.cpp`.
- **Predictions, if bound.** USN04: the 16 Kate death rows keep their killers and categories
  within the RNG coupling, and Kate death times move by under 1 s. Zero deaths move, and the
  category-1 and category-6 hits against Zeros change by more than 5%. Missions with no AA
  engagement of a turning plane stay identical.

**Recommendation.** Bind 8.2 only after the world-rate producer `007D9C80` is published for other
reasons. Its reach is the fighters, not the attrition this doc is about.

## 9. The plane-gun muzzle origin (packet `cc9_plane_gun_mounts`, switch `kPlanePlatformAttachmentBound`, OFF)

The third small term. docs/MUZZLE_OFFSETS.md section 4 left "planes" unplaced, and every one of
USN04's 2084 fallback shots is a plane gun, bomb rack or depth charge (section 6).

**The image.**
- **The plane class runs the ship's slot pass.** `007D3E81 CALL 0095F500` sits in
  `BSP_PlaneClass_BindModelData_Provisional` (`007D3E60`), the same routine that builds a ship's
  platform frames from the model's `("slot", key)` point groups
  (docs/SHIP_PLATFORM_ATTACHMENT.md 1.2).
- **The gun takes that frame at setup.** Every gun copies it through `0072DD20` at `0072E99C`,
  whatever class it is. A plane gun is `MRFSGun` (21h, under the base gun).
- **Plane models carry the groups.** This installation's host already loads them: `B5N_Kate`,
  `zero` and `D3A_Val` have 6 slot groups each, and `F4F_Wildcat` has 8
  (`gunnery: vehicle class ... slot groups=` in `local\RA_OFF_USN04.log`).
- **Plane guns have no device `Mesh`.** Devices 93, 95, 98 and 101 have none. So `007325A0`'s
  list is empty, and the round leaves from the mount itself, the image's empty-list fallback.

**The host until now.** A plane gun fired from the plane's origin raised by the class `Height`
along world up (`gun_muzzle_point`'s fallback), whatever the plane's attitude.

**The binding.** `kPlanePlatformAttachmentBound`, OFF, lets the existing ship mount placement
(`gun_platform_slot_frame_0095f500`, carried to world by the unit pose) take plane units too.
A new summary line reports `plane mounts from model=`.

**Labelled.**
- The axes are taken as for ships: model +x starboard, +y up, +z nose.
- The store that places a gun entity at its platform is not read, as for ships.
- The plane's own node chain is not applied.

**Predictions, recorded before the pair.** Same tree, OFF against ON, RNG streams on.

| row | OFF | ON prediction |
| --- | --- | --- |
| USN04 `plane mounts from model` | 0 | between 150 and 241 (241 plane guns: devices 93/95/98/101/110 carry 70/72/32/32/35) |
| USN04 fighter (category 0) hits | as OFF | moves by more than 5% |
| USN04 deaths by fighters (11 in OFF) | 11 | 9 to 14 |
| USN04 deaths | 44 | 42 to 47 |
| USN04 Kate deaths | 16 | 16 (ship AA kills 15 of them) |
| USN04 torpedo drops | 1 | 0 to 2 |
| USN13 3200/3000 deaths | 27 | 25 to 29 |
| USN02 9200/9000 | - | identical (no plane spawns) |
| USN01 3200/3000 | 7 deaths | 7 unless a Mavis gun kill moves |

## 10. The release counter split (packet `cc9_torpedo_release_counter`)

`src/game_hosts_units.cpp` now counts, beside the task-level `torpedo_releases`, the releases made by an
aircraft the gunnery host already has dead (`torpedo_releases_dead`). A new line reports
`summary mission torpedo task releases: task= live= dead=`. The task counter keeps its value,
because the task reads it (the `1 - torpedo_releases` budget and the `< 1` test), so gameplay is
unchanged.

**Predictions.** Gameplay is identical on every mission. USN04 4700/4500 prints task 6, live 1,
dead 5, with `torpedo_drop drops=1`. USN13 3200/3000 prints task 2, live 0, dead 2, with
`drops=0`.

## 11. The turn-rate average's input is the BODY rate, which the host already has (packet `cc9_plane_world_rate`, read)

The plan was to publish `007D9C80`'s world angular rate as the first half of the `0085E4D0` binding.
The listing says the AA lead does not use it.

- **What `00901C20` reads.** It reads `unit+AF8h..+B00h` (`00901CE5`, `00901CEF`, `00901CF9`,
  on the unit once `vtable[5Ch](0Fh)` answers plane). The flight controller is `unit+AB0h`
  (docs/GAMEPLAY_LOOSE_ENDS_2.md:39), so this is `ctl+48h..50h`: the **body-frame** angular rate
  that `007DA710`'s rate law integrates (docs/PLANE_ANGULAR_VELOCITY.md: body `ctl+48h`, world
  `ctl+24h`). `007D9C80` writes the world copy at `ctl+24h` = `unit+AD4h`, which this path does
  not touch.
- **The host already integrates that rate.** `plane_body_angular` in `src/game_hosts_units.cpp`
  carries `ctl+48h/+4Ch/+50h` and is written by the rate law near line 17173. No producer
  needs porting. The missing piece is a `GameUnitsHost` accessor in
  `include/bsp/game_hosts_units.hpp`, and cc9-ships2 has that header leased
  (`cc9_prcp03_phase_progress`, until 08:42 UTC).
- **The rest of the rule, from `00901D23`-`00901EEE`.**
  - The gate: `|w|^2 > 0.001` (`00D7A23C`; `JBE` skips at or below).
  - An identity matrix, then `0085E4D0(out, identity, w, 1.0)` with `FLD1` as the scale. That is
    `rotate_about_axis_0085e4d0` in `include/bsp/plane_advance_pose.hpp`, with
    `bsp::NativeAdvanceMatrixOps`.
  - Then `004142E0` rotates the target velocity `V` by `out`, giving `V'` (`transform_point_004142e0`,
    whose translation row stays zero).
  - Then `V = (V + V') * 0.5` (`00D7A280`, double) at `00901E92`-`00901EEE`, before the shooter's
    velocity is subtracted at `00901EF9`.
  - The body components go in as the rotation axis unchanged, in the image as in the host
    reconstruction. Nothing turns them into world axes first.
- **The binding, when the header frees.**
  - A `GameUnitsHost::unit_plane_body_angular_rate(index, out[3])` accessor.
  - In the gunnery host's AA lead, behind `kAaTargetTurnAverageBound` (OFF): when the target is a
    plane and `|w|^2 > 0.001`, apply the rotation-and-average above to the lead velocity.

**Predictions for that pair** (section 8.2 stands).
- USN04: the 16 Kate death rows keep their killers and categories within the RNG coupling, and
  their death times move by under 1 s.
- USN04: Zero deaths move, and the category-1 and category-6 hits on Zeros change by more than 5%.
- USN04: deaths stay within 44 +- 3.
- USN02 is identical.
