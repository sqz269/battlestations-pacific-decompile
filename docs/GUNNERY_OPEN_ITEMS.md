# Gunnery open items, ranked by reach (packet `cc9_gunnery_open_ranking`)

Worker cc9-gunnery3, 2026-09-27, on main f6080abdc. Ghidra was read only. This ranks what is still
open on the gun side: docs/GUN_SHOT_CADENCE.md sections 7-8, docs/GUN_AIMING.md,
docs/FLAK_PROXIMITY_BURST.md, docs/USN04_KATE_ATTRITION.md, and every gun-side `UNIMPLEMENTED`
row the reference runs log.

**Reach** counts the reference missions that call the item, out of four: USN04 4700/4500
(`local\RA_OFF_USN04.log`, this worktree), and USN02 9200/9000, USN01 3200/3000 and USN13
3200/3000 (`FP_OFF_USN02.log`, `FP_OFF_USN01.log` and `RM_USN13.log`, worktree cc9-plane2). **Calls**
sums the `host methods` table's `calls=` across them. The census script is
`local/cc9-gunnery3-rank.py` in this worktree. An item with no record site has no call count, and
is ranked from its own evidence.

## 1. Closed since the documents were written

| item | where | state |
| --- | --- | --- |
| `barrel_num` from the model's `"fire"` items | GUN_SHOT_CADENCE 7.1, 8 (`gun_class_muzzle_group`) | bound, `kGunBarrelCountBound` |
| the settle band 0.1 degree | GUN_SHOT_CADENCE 7.2 | fixed (its "Integration result") |
| the flak passing rule | FLAK_PROXIMITY_BURST 7 | proven unreachable (USN04_KATE_ATTRITION 8.1) |
| AA error terms, flak distance error, the lead's velocity | USN04_KATE_ATTRITION 6 | exact at the rows the missions set |
| the turn-rate average `0085E4D0` | AA_LEAD, USN04_KATE_ATTRITION 8.2, 15 | ON, `kAaTargetTurnAverageBound` |
| plane-gun muzzle origin | MUZZLE_OFFSETS 4, section 8 | ON, `kPlanePlatformAttachmentBound` |
| targetless guns and rest angles | GUN_REST_ANGLES 9 | ON, `kGunIdleRestBound` |
| the immediate-fire slot `vtable[1F0h]` (rank 7) | GUN_SHOT_CADENCE 10 | bound and ON, `kGunImmediateFireSlotBound`: MRTGun `006FDF60` / `0084C5B0`, MSTGun `006FDC50` / `006FE0D0`, the artillery bot's delayed raise; no MRTGun is artillery-armed on USN02/USN04/USN01/USN13 |
| the line of sight `00864680` (rank 1 of section 2) | sections 5, 8 | ON, `kGunneryLineOfSightBound` |
| the stop-firing hook `0072B4C0` (rank 2) | section 7 | read; presentation only |
| the director fire target `00835860` (rank 3) | sections 13-15 | ON, `kWeaponDirectorFireTargetBound` |
| the director's target refusal `0071D6D0` and release observer `00694A60` / `0071DDB0` (rank 3 on h) | sections 19-21 | ON, `kDirectorTargetChecksBound` |
| the torpedo threat list `00814420`, head node only (rank 4 on h) | sections 22-24 | ON, `kForeignTorpedoThreatHeadBound` |
| the AutoTarget's locked-target accept `0071D6D0` at 009F5E59 (section 17's last term) | sections 17, 23-24 | ON, `kAutoTargetCommandAcceptBound` |
| the target's sub-entity list `008654AC` (rank 4) | section 2 | exact for every unit this host builds |
| the unit fire cooldown (rank 6) | sections 11-12 | ON, `kUnitFireCooldownBound` |
| the invincibility floor | sections 9-10 | ON, `kUnitInvincibilityFloorBound` |
| the difficulty multipliers | DIFFICULTY_MULTIPLIERS 6 | ON, `kDifficultyMultipliersBound` |
| the wave order, and both timing labels | GUN_SHOT_CADENCE 10.7, 10.8, 10.10 | ON, `kGunWaveOrderBound` |
| the AA bots' own fire tests | GUN_SHOT_CADENCE 10.9 | ON, `kAaBotFireTestsBound` |
| the torpedo swim | TORPEDO_FRIENDLY_CROSSING 6 | ON, `kTorpedoSwimThrustBound` |
| the torpedo aim | TORPEDO_SPREAD_AIM | read, the image's; nothing bound |

## 2. The ranking

| rank | item | image | reach | calls | what the host does now |
| --- | --- | --- | --- | --- | --- |
| 1 | the visibility line of sight | `00864680` (from `00864D90` BSP_UnitGunneryVisibility_Test) | 4 | 6644 | answers visible (`visible_00864d90`, "open water, no terrain") |
| 2 | the gun's stop-firing hook | `0072B4C0` (from `0072D2C0` and `0072B5A0`) | 4 | 3055 | no-op record |
| 3 | the weapon director's fire target | `00835860` (WeaponDirector / AutoTarget `set_fire_target`) | 4 | 482 + 2235 (USN02 AutoTarget) | record, in `src/game_hosts_commands.cpp` |
| 4 | the target's sub-entity list | `008654AC` (`target->vtable[0FCh]`) | 3 | 2707 | takes the base `00432480` (the entity itself); exact for every unit this host builds |
| 5 | the plane's gun task | `009FC7C0` BotTaskGun::tick | 2 | 52188 (USN04 18276, USN13 33912) | record, in `src/game_hosts_units.cpp` |
| 6 | CanFire tests 5 and 6, the unit fire cooldown | `unit+6F8h`/`+6FCh` seeded by `00953CC0` (GUN_SHOT_CADENCE 8, `unit_fire_cooldown_6f8h`) | 4 (every CanFire) | no record site; `can_fire_refusals` USN04 32439, USN02 94170 | not modelled (`unit_fire_blocked` false, `gun_disabled` false) |
| 7 | the MRTGun immediate-fire slot | gun `vtable[1F0h]` = `006FDF60` (GUN_SHOT_CADENCE 8) | USN02 artillery, USN04 AA | no record site | the `0072D2C0`/`0ADh` latch path stands in |
| 8 | depth-charge and bomb-rack muzzle origins | devices 54 (DC launcher), 80/85/87/89 (racks) | 3 | part of 2084 USN04 fallbacks | mount point; no device Mesh, so this is the image's empty-list fallback too. **Closed, exact** |

**Two notes on the table.**
- **Rank 2 looks presentation-only.** `0072B4C0` stops the point effect at `gun+47Ch`
  (`00867B10`) and, when `gun+470h` is set, calls `00731EF0`. Nothing in its body touches fire
  state. It ranks high by calls but likely moves no gameplay row. `00731EF0` is unread.
- **Rank 5 is plane-side.** Its calls are the largest single count, but the task belongs to the
  plane owners. It is listed because its name is the gun's.

## 3. The top three as packets

### 3.1 `cc9_gunnery_line_of_sight` (rank 1)

- **Image.** `00864680`, `__thiscall(record)(float x, float y, float z)`, called from `00864D90`
  and `00864BA0`.
  - It caches the target's point every 0.5 s (`00CE3800` into `record+10h`): the pose at `+FCh`,
    raised by the class height, or by a section's span through `00862C00` and global config
    `+94h`/`+9Ch`.
  - For a target answering `IsKindOf(6)` it stores `0081DE10`'s answer at `record+20h`. That is a
    roughly 220-line test over the class extents at `+538h+A0h`. When the answer is set, the
    target is not visible.
  - It then casts `00904400(44h, point, observer, hit)` and calls the target hidden when the hit
    lies more than `00CFBC80` from the observer.
- **Missing in the host.** `00904400` (a 15-line wrapper), the world query behind it, and
  `0081DE10`.
- **Measuring missions.** All four. Which of them put land between a gun and its target is not
  checked yet. That is the first measurement.
- **Prediction sketch.** A mission with no land between guns and targets is identical unless
  `0081DE10` answers for some ship. Where land blocks a line, targeted ticks fall, so `assigns`
  and shots fall a few percent. Deaths stay within ±2. The first read is `0081DE10`, to learn what
  it hides.

### 3.2 `cc9_gun_stop_firing_hook` (rank 2)

- **Image.** `0072B4C0`, `__fastcall(gun)`: release the point effect `gun+47Ch`, then `00731EF0`
  when `gun+470h` is set.
- **Measuring missions.** All four.
- **Prediction sketch.** Gameplay identical on all four once it is shown that `00731EF0` writes
  no fire state. That would close it as a concrete marker, like docs/SCENE_CONTENTS_HOSTS.md
  section 27.3.

### 3.3 `cc9_unit_fire_cooldown` (rank 6, the widest unmodelled fire gate)

- **Image.** `CanFire`'s tests 5 and 6 read `unit+6F8h`/`+6FCh`, which `00953CC0` seeds
  (GUN_SHOT_CADENCE 1 and 8).
- **Measuring missions.** All four. It gates every shot.
- **Prediction sketch.** If the seed is a per-unit post-spawn or post-order hold, the first shot
  moves later on all four (USN02's 1.40 s first) and shot counts fall. If it is zero on this data,
  identical. The first read is `00953CC0`'s store.

Rank 3 (`00835860`) is in the commands host, not a gun file. Rank 4 is exact for the units this
host creates. So neither is proposed ahead of these.

## 4. The invincibility floor, gunnery half (packet `cc9_set_invincible_floor`, switch `kUnitInvincibilityFloorBound`, OFF)

The read is cc9-lua2's (docs/LUA_BINDING_MISSION.md, "SetInvincible, 00897A50"). This is the
gunnery host's half. cc9-lua2 binds the native, which calls the setter.

- **The store.** `GameGunneryHost::set_unit_invincibility(unit, value)` keeps unit+150h per unit
  index. It holds the value whether or not the unit's gunnery row exists yet, because a mission's
  `luaInit` can run first. `unit_invincibility(unit)` reads it back.
- **The three damage sites** all pass it as `UnitHealth::invincibility` to
  `bsp::apply_damage_00879070`, which already floors health at `inv * max`:
  - the hull pass (`add_damage`);
  - the delayed explosions;
  - damage control's water and fire.
- **The sink.** In the kill path, `bsp::sink_is_refused_008110f0` refuses the `008110F0` sink
  record for `inv > 0`, and the refusal is counted. The host's sink is a record only, so nothing
  else changes.
- **Not gated, as in the image.** Script `Kill` (`008AC5C0`) and the plane depth kill
  (`kill_unit_00926d90`).
- **The census line.** `summary mission gunnery invincibility sets= floored_writes=
  sink_refusals= bound=`.

**A misreading found on the way.** The water-surface binding describes `007BC5B0`'s test as
"the health test `unit+150h <= 0`", and answers it with `unit_dead()`
(`include/bsp/game_hosts_gunnery.hpp`, packet `cc9_water_surface_law`). The listing returns 1 when
`[unit+150h] <= 0` or when the unit is not AI-held (`+1ACh` not 8 and `00927F10` false). unit+150h
is the invincibility float, so the test means "not invincible, or player-held", not "dead". Once
the native side sets the floor, the plane owners can answer it with `unit_invincibility(i) <= 0`.
No change is made here.

**Predictions** (cc9-lua2's, for the pair with both halves bound):

| row | prediction |
| --- | --- |
| USN04 4700/4500 | identity: the floored Yorktown takes no damage |
| BSM01 3200/3000 | identity: no unit takes damage |
| USN02 9200/9000 | moves (exit 3). The seven floored ships (DeRuyter, Java, Kortenaer, Electra, Samidare, Murasame, Harusame) cannot die before `luaPh2MovieEnd` releases them. Kortenaer does not die at 68.30 s; `floored_writes` > 0 |
| this commit alone (no setter caller yet) | identity everywhere, `sets=0` |

## 5. The line of sight, `00864680` (packet `cc9_gunnery_line_of_sight`, switch `kGunneryLineOfSightBound`, OFF)

Rank 1 of section 2. Read from the pseudocode and, for the stack slots, from disk bytes.

**`00864D90`, the caller (the visibility cache).**
- **The observer's point.** Pose `+FCh..+104h`, with y raised by class `Height` (`+538h+A8h`)
  plus Globals `+90h` (5.0). The accumulator is seeded at `00864E29` (after a `PUSH 5`, so
  `[ESP+14h]`), `Height` is added at `00864E96`-`00864EA0`, and y is raised at `00864EA4`-`00864EC1`.
- **The section-span branch.** When class `+50h` exists and `00862C00` answers, the raise is
  `sect+38h + max(5.0, (sect+38h - sect+2Ch) * [+98h])` instead. Not taken here, labelled.
- **The call.** `00864680(record, x, y + raise, z)` at `00864ED3`.
- **The lifetime.** The cache entry lives `U(0.8, 1.2)` (`00CE74F8`, `00CE3814`, stream 1) times
  Globals `+A0h` (5.0) when visible, or `+A4h` (4.0) when hidden.

**`00864680`, the test.**
- **The target's point.** Refreshed every 0.5 s (`00CE3800` into `record+10h`): pose `+FCh`, with
  y raised by `Height` plus Globals `+94h` (5.0; the seed is at `00864721`, after a `PUSH 5`).
- **A ship target first.** For a target answering `IsKindOf(6)`, `0081DE10`'s answer goes to
  `record+20h`, and a set byte means hidden.
  - `0081DE10` returns 1 when `unit+1130h > 0`.
  - Otherwise it walks the list at `[[00E188A8]+19CCh]+364h` and expands each node with
    `00848410` (the same expander the objective sets use).
  - It returns 1 when the ship's bow or stern point (`+-0.5 * class+A0h` along its axis) lies
    inside an entity's footprint: `|x| < 0.6 * its class+A0h` (`00CEFF98`) and
    `|z| < [00CEC9D8] * its class+A4h`.
  - What that list holds, and what `unit+1130h` counts, were not identified. **Labelled: the host
    answers 0.**
- **The ray.** `00904400(44h, target point, observer point, hit, 0)` is
  `BSP_SpatialIndex_QuerySegment` with kind 44h, the `Landscape` class. The target is hidden when
  the hit lies more than 25 m from the observer's point (`00CFBC80` = 625.0, squared distance,
  summed z, x, then y).

**The host binding.**
- `GameGunneryHost::Impl::line_of_sight_00864680` builds both points.
- It casts through the existing `SegmentBinding` with `kind_filter = 44h`. `entity_is_kind` now
  answers kind 44h only for the Landscape entries `kLandscapeSpatialAttachBound` adds; every
  other kind stays true for everything, as before.
- The cache takes the result and the image's lifetime.
- The census line is `summary mission gunnery line of sight tests= blocked= bound=`.

**Labelled.**
- `0081DE10` answers 0.
- The section-span raise is not taken.
- The target point is taken at each test, not cached for 0.5 s. The test itself runs only when a
  cache entry has expired, at least 3.2 s apart.

**Predictions, recorded before the pair** (same tree, OFF against ON, RNG streams on). The
landscape counts come from the logs' `summary scene terrain landscapes=` line.

| row | landscapes | ON prediction |
| --- | --- | --- |
| USN02 9200/9000 | 0 | gameplay identical (pair_diff exit 1). `blocked=0`; the cache re-tests at the new lifetimes, which moves only `tests` and the visibility-TTL stream |
| USN04 4700/4500 | 0 | gameplay identical, as USN02 |
| USN13 3200/3000 | 12 | `blocked > 0`. Where an island hides a target, assignments and shots fall. Deaths 27 +- 2 |
| USN01 3200/3000 | 4 | `blocked` 0 or small. Deaths 7 +- 1 |

## 6. 007BC5B0 answered as the listing reads (packet `cc9_water_invincible_test`, under `kUnitInvincibilityFloorBound`)

This folds section 4's note. The water-surface binding in `src/game_hosts_units.cpp` now asks
`GameGunneryHost::water_gate_007bc5b0(unit, player_held)` instead of `unit_dead()`. With the
switch ON the gate answers true when `unit_invincibility(unit) <= 0` or the unit is player-held.
No aircraft is player-held here, so the host passes false. With the switch OFF it keeps
`unit_dead()`.

**What it changes when ON.** The two readings differ only for a **live** aircraft.
- A live aircraft that is not invincible now passes the gate. So a live AI aircraft of a class
  with a non-zero `MinWaterSpd` that goes under the surface takes the water contact, where the
  host used to ignore it and leave the aircraft in free flight.
- A live invincible aircraft now fails the gate, as in the image.

**Prediction.** Idle reference runs are identical: the `plane water contact ignored` line, which
the old reading prints for such a live aircraft, is absent from all four logs (USN04
`RA_OFF_USN04`, USN13 `RM_USN13`, USN01 `FP_OFF_USN01`, USN02 `FP_OFF_USN02`). Every water
contact on them (16, 8, 3, 0) is a dead aircraft, which both readings pass. A run where a live
AI aircraft touches water would move.

## 7. The stop-firing hook, `0072B4C0` (packet `cc9_gun_stop_firing_hook`, read; marker made concrete)

Rank 2 of section 2. Read from disk bytes: `0072B4C0`-`0072B534` (plain `RET`, INT3 after it),
`00731EF0` and `0072F830`'s `0072F9DD`-`0072FA26`.

- **The body.** It stops the gun's own muzzle point effect at `gun+47Ch` (`00867B10`, then byte +9
  set and the refcount released through `[00CE2220]`) and clears the pointer.
- **The class's looping effect.** When `gun+470h` is set, it calls `00731EF0` with
  `ECX = [gun+3F8h]` (the weapon class record) and the gun pushed. That removes the gun from the
  record's set (`+3Ch`..`+44h`), and when the set is empty it stops the shared effect at record
  `+38h`. Then it clears `gun+470h`.
- **The flag's meaning.** `gun+470h` is 1 while the gun is in its class's looping-effect set.
  `0072F830`, the shot spawner, registers it with `00732210` and unregisters it with `00731EF0` when
  the class record carries an effect resource at `+18h`.
- **The census, and its limit.** The ESI- and EBP-based scans of `+470h` found only these gun
  routines, plus `007AC000` and `007AB8C0`, which are not gun routines. Other base registers were
  not scanned.
- **No fire state is written.** None of the fire request `+450h`, `+454h` or `+478h`, the barrel
  timers or the trigger.

**The marker is now concrete.** `Gun::stop_firing_0072b4c0` calls `done` instead of `record`. This
host builds no point effects, so there is nothing to store.

**Prediction.** Gameplay is identical on all four reference missions (pair_diff exit 1). The
`host methods` table moves by one row from unimplemented to concrete.

## 8. The pairs for section 5 (line of sight), and the flip (2026-09-27)

OFF `local\P0_OFF_<m>.log`; ON `local\P6_ON_<m>.log` (`pair_export --commit b4fcb606b --flip
kGunneryLineOfSightBound=true`).

| row | ON | prediction | verdict |
| --- | --- | --- | --- |
| USN02 | tests 1129, blocked 0, pair_diff 1 | identical | held |
| USN04 | tests 1173, blocked 0, pair_diff 1 | identical | held |
| USN13 | tests 1901, blocked 0, pair_diff 1 | blocked > 0, assignments and shots fall | **failed**: nothing is blocked |
| USN01 | tests 44, blocked 0, pair_diff 1 | blocked 0 or small, 7 deaths | held |

**Not vacuous.** On USN13 the Landscape traces rise from 438 to 451. So 13 line-of-sight rays reached
a Landscape shape's bounds, and none of them met terrain between a gun and its target.

**Decision: `kGunneryLineOfSightBound` is ON.** It is gameplay-identical on all four and faithful to
the listing.

## 9. The invincibility floor pair, predictions (packet `cc9_set_invincible_floor`, recorded before the ON runs)

OFF is this worktree at `df7ba20c5` (main, with cc9-lua2's SetInvincible native and the
multiplier ON), in `local\IF_OFF_<m>.log`. ON flips only `kUnitInvincibilityFloorBound`, which
also carries section 6's 007BC5B0 fold.

**What the OFF logs show the native doing.**
- **USN02: 20 calls, not 10.** At frame 37 it floors DeRuyter, Java, Kortenaer and Electra at 0.1,
  and Haguro, Jintsu, Yudachi, Samidare, Murasame and Harusame at 0.5. At frame 3764 (about
  186 s, `luaPh2MovieEnd`, which the multiplier now lets the run reach) it sets all ten back to 0.
- **USN04:** Yorktown-class01 at 0.24. It takes no damage on OFF.
- **USN01: Convoy1-6 at 0.1, and both ScoutDauntlesses at 1.0** (one call, `units=2`). The
  convoys take no damage on OFF. The Dauntlesses die at 130.60 s and 140.40 s.

**Predictions.**

| row | OFF | ON prediction |
| --- | --- | --- |
| USN02 floored ships dying before 186 s | Kortenaer 87.85, Electra 127.05, Yudachi 152.80, Samidare 170.21 | none of the ten dies before the release; any of them may die after 186 s |
| USN02 census | sets 20, floored_writes 0 | floored_writes > 0; sink_refusals 0 (a floored ship never reaches 0 health by damage) |
| USN02 deaths | 17 | moves, exit 3; the fight runs differently from 88 s on |
| USN04 | Yorktown untouched | identical (pair_diff 0 or 1) |
| USN01 ScoutDauntlesses | die at 130.60 s and 140.40 s | **both survive**: at 1.0 the floor is the full health, so no damage lands. Deaths 7 -> 5, exit 3. This departs from the brief's "USN01 identity" |
| USN01 convoys | untouched | untouched |
| water gate (section 6) | - | no `plane water contact ignored` line on any of the three: no invincible aircraft touches water |

## 10. The invincibility floor pair, and the flip (2026-09-28)

OFF `local\IF_OFF_<m>.log` (`df7ba20c5`); ON `local\IF_ON_<m>.log` (`pair_export --commit
df7ba20c5 --flip kUnitInvincibilityFloorBound=true`). RNG streams and the death table were on.

| row | OFF | ON | prediction (section 9) | verdict |
| --- | --- | --- | --- | --- |
| USN02 floored ships dying before the release | Kortenaer 87.85, Electra 127.05, Yudachi 152.80, Samidare 170.21 | none of the ten dies at all | none before the release | held |
| USN02 floors | - | Kortenaer 263, Electra 281 (0.1 of max); Samidare, Harusame, Yudachi 1350 (0.5) | - | exact |
| USN02 census | floored_writes 0 | floored_writes 32337, sink_refusals 0 | > 0, 0 | held |
| USN02 deaths | 17 | 12 | moves, exit 3 | held |
| USN04 | - | pair_diff 1 | identical | held |
| USN01 ScoutDauntlesses | die at 130.60 s and 140.40 s | both survive; deaths 7 -> 5 | both survive | held |
| water gate | - | no `plane water contact ignored` line on any of the three | none | held |

**Not predicted: USN02 now fails in phase 1.**
- The release never comes. The ON run makes only the 10 floor calls, because `luaPh2MovieEnd`
  runs only after phase 1 completes.
- Phase 1 completes (`usn_2_java.lua:531`) when DeRuyter's health is below 15%, or when every
  ship of `EnemyDestroya` is dead. With the floor ON the Japanese destroyers cannot fall below 50%,
  so only DeRuyter's health can end it, and her floor is 10%. She is left at 1700 (2420 taken).
- Exeter is not floored. She is sunk at 385.68 s, and `usn_2_java.lua:521`'s test ends the
  mission (`MissionPhase=1 EndMission=true`).
- The script and the floor are the image's. What the run lacks is enough fire on DeRuyter, which
  is the ship AI's and the gunnery host's targeting. That is the next thing to read for USN02,
  not the floor.

**Decision: `kUnitInvincibilityFloorBound` is ON**, and with it section 6's 007BC5B0 fold. Every
floor holds at its exact fraction, and every recorded prediction held.

## 11. The unit fire cooldown (packet `cc9_unit_fire_cooldown`, switch `kUnitFireCooldownBound`, OFF)

Rank 6 of section 2. Read from the Ghidra listing and disk bytes. Ghidra was not written.

**The writer census.** `scan-bytes` of the displacements `F8 06 00 00` (12 hits) and `FC 06 00 00`
(19 hits) with `--limit 4000`. The hits on the unit are:
- `0095CF50` / `0095CF58`: `BSP_UnitGameObject_Construct` seeds both to XMM0. XMM0's only write
  before them in the listing is `0095CD7D XORPS XMM0,XMM0`, so both start at 0.
- `0072FB78` (`FSTP [EDX+6F8h]`) and `0072FBA9` (`FSTP [EDX+6FCh]`), with `EDX = [gun+3F0h]`, in
  `BSP_Gun_SpawnShotAndEffects` `0072F830`: the per-shot stores.
- `00729AFB` / `00729B17` in `BSP_Gun_CanFire` `00729A80`: the reads.

The rest are other structures: the settings (`0083B5E0`), the ship and plane class loaders, the
bot-task factories (`009B9030`, `009CD300`), the path search, CRT and unwind code. `00953CC0`
counts the fields down on the sub-object at `unit+310h`, so its displacements are `3E8h`/`3ECh`
and do not appear in this scan. The countdown is recorded in docs/TICK_ELEMENT_OVERRIDES.md.

**The rule.**

| step | site | rule |
| --- | --- | --- |
| seed | `0095CF50`/`58` | both 0 |
| per artillery shot | `0072FB35`..`0072FB78` | when `[gun+3F4h]+80h` is 2, 3, 4 or 6: `unit+6F8h = U(0.075, 0.225)`. The draw is `00BD2F10` on stream 1 (`ECX = 1`), with `00CEED68` = `0x3D99999A` = 0.075 and `00CFE2B4` = `0x3E666666` = 0.225, and it is stored by FSTP |
| per torpedo shot | `0072FB7E`..`0072FBA9` | when the kind is 7: `unit+6FCh = 00836EB0(settings, unit)`. That is settings `+764h` (`SubTorpedoDelay`) when the unit answers `IsKindOf(8)`, else `+768h` (`ShipTorpedoDelay`). This installation's ShipGlobals has 0.5 for both (docs/GAMEPLAY_SETTINGS.md) |
| every unit tick | `00953CC0` | while positive, each counts down by the step |
| CanFire test 5 | `00729ADD`..`00729B0A` | `006D1E50(kind)` (kind in 2, 3, 4, 6) and `unit+6F8h > 0` refuse |
| CanFire test 6 | `00729B0C`..`00729B26` | kind 7 and `unit+6FCh > 0` refuse |

**The earlier reading was wrong.** docs/GUN_DISPERSION.md section 0 calls the `0072FB6A` draw "an
effect timer". It is the unit-wide artillery fire cooldown: CanFire reads it back. So every
artillery shot holds **every** artillery gun on that unit for 0.075-0.225 s, and every torpedo
launch holds the unit's torpedo tubes for 0.5 s.

**The host until now.** `gate.unit_cooldown_applies = false`, and both cooldown inputs are 0.

**The binding.** `kUnitFireCooldownBound`, OFF.
- Two per-unit timers, counted down at the start of the gun step.
- The two per-shot stores. The artillery draw uses its own stream key `Draw::unit_fire_cooldown`.
  The delays come from ShipGlobals at run time, through the flatten chunk.
- The CanFire inputs, and a census line: `summary mission gunnery unit fire cooldown sets=
  torpedo_sets= refusals= sub_delay= ship_delay=`.
- **Labelled:** the countdown runs in the gun step, not in the unit's own tick.

**Predictions, recorded before the pair.** A refused gun retries on the next tick, so the
cooldown mostly delays shots rather than cancelling them.

| row | ON prediction |
| --- | --- |
| every mission | `ship_delay=0.50`, `sub_delay=0.50`; `sets` about equal to the artillery shots |
| USN02 9200/9000 (surface, artillery) | first shot unchanged; shots fall 1-5% (a multi-turret ship's guns that come ready together are staggered); `refusals > 0`; deaths within +-2 |
| USN04 4700/4500 | category-6 dual-purpose AA is artillery kind 6, so its shots fall a few percent; category 1 and 5 unaffected; deaths within 44 +- 3 |
| USN13 3200/3000 | shots fall 1-5%; deaths 27 +- 2 |
| USN01 3200/3000 | shots fall a few percent; deaths 7 +- 1 |
| torpedo salvos (USN02) | multi-tube launches spread over 0.5 s steps; torpedo shots unchanged or down 1-2 |

## 12. The unit fire cooldown pairs, and the flip (2026-09-28)

OFF `local\FC_OFF_<m>.log` (`d63d6198c`, with the floor and the multiplier ON); ON
`local\FC_ON_<m>.log` (`pair_export --commit d63d6198c --flip kUnitFireCooldownBound=true`). RNG
streams and the death table were on. Every ON run reads `sub_delay=0.50 ship_delay=0.50`.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 sets | - | 3052 artillery + 189 torpedo = 3241 = the ON shots | about the artillery shots | held, exactly |
| USN02 shots | 3138 | 3241 (+3.3%) | fall 1-5% | **failed**: the stagger reshuffles the engagement; hits fall 4918 -> 4191 (-15%) |
| USN02 first shot / deaths | 1.40 s / 12 | 1.40 s / 11 | unchanged / +-2 | held |
| USN02 refusals by the cooldown | - | 21187 | > 0 | held |
| USN04 shots / hits / deaths | 5433 / 707 / 44 | 5183 (-4.6%) / 681 / 43 | a few percent down (category 6); 44 +- 3 | held |
| USN13 shots / deaths | 4440 / 26 | 4422 / 25 | down 1-5%; 27 +- 2 | held (-0.4%) |
| USN01 shots / deaths | 652 / 5 | 653 / 5 | a few percent down; deaths within 1 | held on deaths; shots flat |

**Decision: `kUnitFireCooldownBound` is ON.**
- The per-shot stores match the image to the shot: each artillery and torpedo shot set its
  timer, and CanFire refused 21187 times on USN02 while a timer ran.
- The failed row is a consequence I mispredicted, not a divergence. USN02's fight runs longer
  with the floor ON, and the staggered salvos land fewer hits per shot.

## 13. The weapon director's fire target, `00835860` (packet `cc9_weapon_director_fire_target`, read)

Rank 3 of section 2. Read on main `c7c55655d`, which includes cc9-ships2's `store_fire_target_00836240`
(`2a1d54495`). No source edit; the binding follows reference g.

**The image.**
- **The setter.** `00835860` (`__thiscall(director)(target, force)`, docs/WEAPON_DIRECTOR.md) acts
  only when `force` is set, the lock byte `+23Ch` is clear, or no target is held (`+238h` null).
  It then builds a kind-5Eh message (`00835740`) and routes it.
- **The receiver.** `00836240` sets `+23Ch = force`, and replaces `+238h` when the target changes.
  So an accepted unforced call also clears the lock.
- **The callers and their `force`**, from the pushes before each call:

| call site | caller | force | target |
| --- | --- | --- | --- |
| `009F5F21` | `BSP_WeaponDirector_AutoTargetTick` `009F5DA0` | **0** (`009F5F1E PUSH 0`) | the chosen enemy |
| `0089ABDC` | `BSP_LuaBinding_SetFireTarget` | 1 | the script's target |
| `00835930` | `BSP_WeaponDirector_SetCommand` `008358D0` | 1 (`00835924 PUSH 1`) | the command's target |
| `00744B21`, `00817072`, `008171CF`, `00843941`, `006D28FA` | order and entity-command appliers | 1 | **null**: a forced clear |
| `004643BD` | `BSP_FireTargetHolder_Apply` | 1 | the holder's target |
| `00744AFD`, `00816FFD`, `00835E07`, `0084391D`, `006D28C7` | order and command appliers | pushed earlier, not traced | the order's target |

**The host.**
- **The Lua path is bound.** `SetFireTarget` goes through `GameShipAiHost::store_fire_target_00836240`
  with force 1, applying the gate and the lock.
- **The AutoTarget path bypasses both.** `AutoTarget::set_fire_target` (`src/game_hosts_ship_ai.cpp`)
  writes `ctl.fire_target` directly: no `force || !lock || !held` gate, and no lock clear. So
  after a script's forced target, the host's automatic selection overrides it, where the image
  refuses every unforced change while the lock is set and a target is held.
- **The command paths are records only.** `WeaponDirector::set_fire_target` and
  `EntityCommandArm::set_fire_target` (`src/game_hosts_commands.cpp`) never store a target, so a
  command's forced target, and the orders' forced clears, never reach the director.
- **The gunnery host reads the director's target** by name from `GameShipAiHost::rows()`
  (`run_gunnery_pass`), so every one of these writes decides what the guns engage.

**Reach** (section 2's census): `WeaponDirector::set_fire_target` 482 calls on all four reference
missions; `AutoTarget::set_fire_target` 2235 on USN02; USN02's script makes 14 `SetFireTarget` calls.

**The binding, after reference g** (switch `kWeaponDirectorFireTargetBound`, OFF):
1. `AutoTarget::set_fire_target` goes through `store_fire_target_00836240(unit, target, false)`.
2. The command records store through the same routine with the sites' `force` (1 for
   `SetCommand` and the forced clears).
3. The lines are in `src/game_hosts_ship_ai.cpp` and `src/game_hosts_commands.cpp`, not the gunnery
   file.

**Prediction sketch.**
- **USN02:** the scripted targets hold until a forced change, so target switches fall. The
  destroyers that SetFireTarget aims at DeRuyter keep that target, which bears on cc9-ships2's
  `cc9_usn02_deruyter_fire`. Shots about the same; hits on DeRuyter up.
- **USN04 and the rest:** no script fire targets, so only the command paths move; identity
  where no command carries a target.

## 14. The director fire-target binding (packet `cc9_weapon_director_fire_target`, switch `kWeaponDirectorFireTargetBound`, OFF)

Built on main `bf6afe7ff`, which already carries cc9-ships2's `kScriptFireTargetBound`. That
switch makes the AutoTarget tick read the director's lock at `009F5E37` and keep a locked
scripted target, so the tick's own override of a scripted target is closed. What this switch adds:

- **The AutoTarget write goes through `00836240`.** `AutoTarget::set_fire_target` now calls
  `Impl::store_fire_target` with the tick's `force` of 0 (`009F5F1E`): it is refused while a
  forced target holds the lock, and it clears the lock when accepted.
- **The command paths store their targets.**
  - `WeaponDirector::set_fire_target` for `SetCommand` (`00835930`, force 1; the target pointer is
    resolved against the commands host's unit records).
  - `WeaponDirector::set_fire_target` for `BeginCurrentCommand` (`00835E07`, the caller's force;
    the target is unit index + 1).
  - Both queue a `GameFireTargetRequest` (`GameCommandsHost::take_fire_target_requests`), which the
    ship-AI host applies at the top of `controller_step`, beside the session pump that delivers
    routed messages. That stands in for the kind-5Eh message's routing.
- **Labelled.** The entity-command arm (`00816E30`) stays a record: it is not reached on the
  reference runs. The five order sites whose `force` was not traced stay records.
- **The census line.** `summary mission ship ai director fire target bound= command_requests=
  refusals= changes=`.

**Reach on the OFF logs of the current head** (`TA_OFF_<m>`): `WeaponDirector::set_fire_target`
is called 28 times on USN02, 140 on USN04, 15 on USN01 and 262 on USN13; `AutoTarget::set_fire_target`
1146 times on USN02. JM06 is measured on its OFF run.

**Predictions, recorded before the pair.**

| mission | ON prediction |
| --- | --- |
| USN02 9200/9000 | `command_requests` about 28; target switches fall because forced command targets lock the director; the DRKillers keep DeRuyter longer; hits on DeRuyter up; deaths and hits move (exit 3) |
| USN04 4700/4500 | `command_requests` about 140; the ships under orders hold their ordered targets. Moves (exit 3), deaths within 40 +- 3 |
| USN01 3200/3000 | `command_requests` about 15; small moves or identity; deaths 5 +- 1 |
| JM06 3200/3000 | identity where no command carries a target; otherwise small moves |

## 15. The director fire-target pairs, and the flip (2026-09-28)

OFF `local\WD_OFF_<m>.log` (`02e5661e3`, switch off); ON `local\WD_ON_<m>.log` (`pair_export
--commit a34b68f6b --flip kWeaponDirectorFireTargetBound=true`; `a34b68f6b` only restructures a
branch so that the ON build has no unreachable code). RNG streams and the death table were on.

| mission | OFF shots / hits / deaths | ON | census | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| USN02 | 1083 / 881 / 22 | 1164 / 991 / 24, pair_diff 3 | 43 command requests, 84 target changes, 0 refusals | moves; the DRKillers keep DeRuyter | moved: held. DeRuyter: **failed**, she dies slightly earlier (194.76 s against 199.21 s) |
| USN04 | 5333 / 644 / 40 | identical (pair_diff 1) | 72 requests, 72 changes | moves | **failed**: the ordered targets equal the automatic ones |
| USN01 | 623 / 178 / 5 | identical (pair_diff 1) | 8 requests | identity or small moves | held |
| JM06 | 233 / 145 / 2 | pair_diff 3: one unit-table value (a PBY's nearest target 8163 -> 7770) | 14 requests, 3 unresolved `SetCommand` targets | small moves | held |

**USN02's outcome changes.**
- **The fight.** With forced command targets holding the directors, Houston takes 1104 instead of
  4095, Exeter 6316 instead of 3859.
- **The failure.** Exeter is sunk at 211.76 s, and `usn_2_java.lua:521` ends the mission in phase 2
  (`EndMission=true`). OFF she survives with 4178.

**The three unresolved targets on JM06.** `SetCommand`'s target pointer did not match one of the
commands host's unit records. Those calls stay records (`WeaponDirector::set_fire_target_unresolved`).

**Decision: `kWeaponDirectorFireTargetBound` is ON.** The gate and store are `00836240`'s, and
every command request was stored without a refusal. The failed rows are consequences, not
divergences. The USN02 failure is flagged for reference g.

## 16. The ranking refreshed on reference h (packet `cc9_gunnery_open_ranking_2`, 2026-09-29)

**Source.** The eight reference h logs `local\rb8_{usn04,e2,usn01,usn02,jm06,jm08,usn13,lomp06}.log`
in worktree cc9-gunnery4 (main `d6fc6ee78`, rb8 SHA-256 prefix `5A2B887AA5ED`).
- `local\g4_rank.py` sums every non-concrete native row whose class is gunnery-side, across the
  eight logs. Every such row is `UNIMPLEMENTED`.
- `local\g4_standins.py` reads what each stand-in answers.
- The labelled substitutions come from `src/game_hosts_gunnery.cpp`'s switch notes.

**Moved to section 1's closed table** (the day's landings):

| item | where | state |
| --- | --- | --- |
| the line of sight `00864680` (old rank 1) | sections 5, 8 | ON, `kGunneryLineOfSightBound` |
| the stop-firing hook `0072B4C0` (old rank 2) | section 7 | read; presentation only, the marker is concrete |
| the director fire target `00835860` (old rank 3) | sections 13-15 | ON, `kWeaponDirectorFireTargetBound` |
| the target's sub-entity list `008654AC` (old rank 4) | section 2 | exact for every unit this host builds (4729 calls, the base `00432480`) |
| the unit fire cooldown (old rank 6) | sections 11-12 | ON, `kUnitFireCooldownBound` |
| the immediate-fire slot and the wave order | GUN_SHOT_CADENCE 10.6, 10.7, 10.10 | ON, `kGunImmediateFireSlotBound`, `kGunWaveOrderBound`; both timing labels read (10.7) |
| the AA bots' own fire tests | GUN_SHOT_CADENCE 10.9 | ON, `kAaBotFireTestsBound` |
| the torpedo swim | TORPEDO_FRIENDLY_CROSSING 6 | ON, `kTorpedoSwimThrustBound` |
| the torpedo aim (lead, spread, gyro) | TORPEDO_SPREAD_AIM | read, the image's; nothing bound |
| the difficulty multipliers | DIFFICULTY_MULTIPLIERS 6 | ON, `kDifficultyMultipliersBound` |
| the invincibility floor | sections 9-10 | ON, `kUnitInvincibilityFloorBound` |
| rest angles, plane-gun mounts, the turn-rate average | GUN_REST_ANGLES 9, section 8, USN04_KATE_ATTRITION 15 | ON (`kGunIdleRestBound`, `kPlanePlatformAttachmentBound`, `kAaTargetTurnAverageBound`); the three OFF rows above now read ON |

**The new ranking.**
- **Reach 4:** decides who is shot at, or a death.
- **Reach 3:** decides an order.
- **Reach 2:** a count or a score only.
- **Reach 1:** presentation.

Calls are summed over the eight logs.

| rank | item | image | reach | calls (h) | what the host does now |
| --- | --- | --- | --- | --- | --- |
| 1 | **the AutoTarget candidate list** | `009F5D30`'s scan walks the chain at `[008053C0(party)]+0DE8h`, the party recon slot's enemy list | 4 | 148923 on all eight missions (`AutoTarget::party_recon_slot`); USN02 10689, JM08 57531, USN13 43186 | hands the scan **every live opposing unit** (`src/game_hosts_ship_ai.cpp`, `TargetBinding::scan_party_list`), detected or not. The recon team lists that `kReconTeamListsBound` builds are not used here |
| 2 | the AutoTarget's commanded-target adoption | `0071D6D0` (command accepts target), `00521EA0` (resolve command target), `009F5E69` (director command state) | 4 | 17806 / 10181 / 148923; USN02 9216 / 9224 | answers false / null / 0: a director's commanded target never becomes the AutoTarget pick through this arm |
| 3 | the director's target observation and refusal | `00694A60` (observe target), `0071D74A` (target refuses commands) | 3 | 3135 / 2078 (USN02 1245 / 1238) | records; the refusal answers a constant |
| 4 | the torpedo standoff's threat list | `00814420` (no Ghidra function) | 3 | 12708 (USN02 10484, JM06 2147) | record; the standoff's accuracy term `008FB530` answers the robots.lua table and is exact |
| 5 | the recon squadron and convoy aggregates | `00805680`, `008069A0` (classes 18h, 1Ah) | 3 | 1467 each | not built (`kReconTeamListsBound`'s SUBSTITUTIONS note) |
| 6 | the damage-control death test | `0090E6C0` (pending damage exceeds health) | 4 | 766 (USN02 749) | record; the host's own `pending_over_health` path decides |
| 7 | the death sink | `008110F0` | 2 | 140 (once per death) | record; the death itself is the host's funnel |
| 8 | the kill award threshold | `0050FC30` | 2 | 266 | answers 0; score only |
| 9 | the set-command message | `0071C830` / `0077C2A0` | 3 | 8022 each | the host stores the command directly (`pending_command`), which is the local-session loopback's effect (GUN_SHOT_CADENCE 10.7); likely exact |

**Not ranked, and why:**
- **Plane-side** (the largest counts): `PilotBot::plan_controls` 336743, the `BotApproach` and
  `BotState*` rows up to 109598, and `BotTaskGun`. They belong to the plane packets.
- **Player HUD:** `HudWeaponGroupScreen::*` and `PlayerGunSeat::message_other_group`. They are
  player-only; the reference runs are idle.
- **Labelled substitutions** that are exact at the rows these missions set:
  - the bullet throw's `unit+63Ch = 1.0` and `00470440(7) = 1.0`;
  - the flak distance error of 0 (SPVeteran);
  - the torpedo swim's surface plane.
  They are re-checked when a mission sets another row.

**Next packet: rank 1, `cc9_autotarget_recon_candidates`.**
- **Read** `009F5D30`'s list source: `008053C0`'s party recon slot and the chain at `+0DE8h`. Find
  which recon levels put an enemy on it, how a blip or unknown contact is treated, and when the
  chain is rebuilt.
- **Bind** the candidate list to the recon team lists the host already builds
  (`kReconTeamListsBound`), OFF, with predictions on USN02, USN04, USN13 and JM08. JM08 carries the
  largest count.

**Context** at the time of writing: about 60% of this worker's window. A handoff is due at about 80%.

## 17. Rank 2 read: the AutoTarget's commanded-target adoption is exact on these missions (packet `cc9_autotarget_command_adoption`, read)

`009F5DA0`'s middle, from the listing:

```
009f5e37: cmp byte [ecx+23Ch],0 ; je 009f5e66        ; the director's target lock
009f5e4b: call 0x465080 ; 009f5e59: call 0x71d6d0   ; attackmove (00E08F78) accepts the current target?
009f5e62: mov edi,ebx ; jmp 009f5ebb                 ;   yes: keep it, skip the scan
009f5e69: cmp dword [edx+30h],2 ; je 009f5e7c        ; director mode 2 (override) keeps the retained score
009f5e77: movss [esi+3Ch],FLT_MAX                    ;   otherwise it resets it
009f5ec4: call 0x521ea0 (director+18Ch) ; cmp edi,eax ; je return   ; already on it
```

- **`director+30h` is the command mode** (CRUISE_COMMAND): 1 is the queued slots and 2 is the
  override descriptor at `+18Ch`.
  - The scan `C7 ?? 30 02 00 00 00` finds one writer of mode 2 in director code:
    `0071E8FC` in `0071E7F0` `BSP_WeaponDirector_SetOverrideCommand`. Its callers are
    `00721890` and `00721A40`.
  - The host reaches that as `WeaponDirector::queue_command` (a record). **No reference h log calls
    it**, on USN02, USN04, E2, USN01, USN13, JM06, JM08 or LOMP06.
  - So mode 2 never occurs on these missions, in the image either (as far as the host routes the
    same messages). `director_command_state()` answering 0 has the image's effect: the retained
    score resets on every think.
- **`00521EA0(director+18Ch)` resolves the override descriptor.** With no override set it is a fresh
  director's empty descriptor, so it resolves no entity. The host's null is the image's answer.
- **`0071D6D0` runs only when the target is locked** (`director+23Ch`, set by the script's
  SetFireTarget). The host keeps a locked target in `scan_party_list` (`kScriptFireTargetBound`,
  `keep_locked`).
  - The commands host already has the concrete `0071D6D0` (`WeaponDirector::command_allowed`).
  - The AutoTarget stand-in answers false and falls through to the scan, which keeps the locked
    target anyway.
  - Routing the stand-in to the concrete body would change the path, not the pick. It is left as a
    labelled record.
- **The AutoTarget's own attack-move issue** (`0071D980`, `issue_move_flag`) is recorded
  `attackmove_issues=0` on every reference h mission, so it cannot set mode 2 either.

**Verdict:** rank 2 is exact on the reference missions, and nothing is bound. It becomes live only
on a mission that sends override commands (`0071E7F0` reached), which the ranking should re-check
whenever `queue_command` shows calls.

## 18. Handoff (cc9-gunnery4, 2026-09-29, at about 75% context)

**Where this worker stopped.** Everything is committed on `agent/cc9-gunnery4`.
- Section 16's ranks 1 and 2 are done:
  - Rank 1 is `kAutoTargetReconCandidatesBound` ON, identity on all five paired missions
    (AUTOTARGET_RECON_CANDIDATES).
  - Rank 2 is read and exact (section 17).
- **The next item is rank 3:** the director's target observation `00694A60` (3135 calls) and
  refusal `0071D74A` (2078; the host answers a constant).

**How to start rank 3.**
- `WeaponDirector::observe_target` and `target_refuses_commands` are records in
  `src/game_hosts_commands.cpp` (the refusal at about line 768).
- Read `0071D74A` from the listing: what makes a target refuse commands (it answers for the
  command's target in `008358D0`'s path).
- Read `00694A60`: the observer pair that releases a dead target.
- Count, per mission, how often the refusal would differ from the host's constant, using a
  diagnostic, before binding.

**Tools this worker left in its tree** (`local\`):
- `g4_rank.py`: the section-16 census of non-concrete gunnery rows over logs.
- `g4_standins.py`: what each stand-in answers.
- `g4_gunrows.py`: per-category shot and rise deltas between two logs.
- `g4_aim_check.py`: an independent `008FB8D0` port for the torpedo aim.
- `g4_*_queue.ps1`: the run queues. The exe and the log prefix are parameters.

**Standing facts for the next reader.**
- The reference is h (docs/GAME_EXECUTABLE.md 2026-09-29 h, main `d6fc6ee78`).
- USN02's 29.75 s failure is the image's own for an idle player (TORPEDO_SPREAD_AIM).
- JM06 and LOMP06 moved again with `kSubmarineDiveTeleportBound`, after h.

## 19. Rank 3 read: the director's target refusal and release observer (packet `cc9_director_target_checks`)

Both halves read the target entity's **+5Dh**, the byte 00926390 (death) and 009263C0 (removal)
set in the fixed step's destroy flush (00875EC9). The host's `SceneNodeFlags::torn_down` is that
byte.

**The refusal, 0071D6D0** (body 0071D6D0-0071D772, `RET 8`). The host's record address 0071D74A
lies inside the `JZ` at 0071D749; the test itself is:

```
0071d700: MOV ECX,EDI            ; the descriptor
0071d702: CALL 0x00521ea0        ; resolve
0071d707: TEST EAX,EAX
0071d709: JZ 0x0071d718          ; nothing resolves: refuse
0071d70b: MOV ECX,EDI
0071d70d: CALL 0x00521ea0
0071d712: CMP byte ptr [EAX + 0x5d],0x0
0071d716: JZ 0x0071d71f          ; clear: on to the torpedo / moveonpath tests
0071d718: POP EDI / XOR AL,AL / POP ESI / RET 0x8   ; set: refuse
```

A push whose target is already released is refused. The host answered the clear byte.

**The observer, 00694A60 at 0071E78A.** It adds the pair (target, `director+1Ch`) to the global
observer registry under its critical section, with a reference count. It has no other effect. The
effect is the delivery:
- `director+1Ch`'s table is **00D09EA8** (008363E0 at 00836410). Slot +4 is **0071C1A0**, slot +8
  is **0071DDB0**.
- 0071C1A0 is `MOV EAX,[ECX]; MOV EAX,[EAX+8]; JMP EAX`, so both slots end in 0071DDB0.
- 00925C90 (from 00926390, the death) dispatches slot +8. 00925C40 (from 009263C0, the removal)
  dispatches slot +4 through 00693550.

**0071DDB0**, `__thiscall(ECX = director+1Ch)(entity)`, `RET 4`:
1. It returns at once unless `[[00E188A8]+5D4h] >= 0Ch` (the game state; 0Dh in a mission).
2. The override descriptor `director+18Ch`: when it resolves to the entity, `vtable[70h]`
   (0071EDD0) with 0, then `0071D9E0(2)` unless `[[00E188A8]+1FE4h] == 2`. Nothing in this host
   writes the override command, so this arm cannot match here.
3. Slots i = 0..9 (command `director+54h+1Ch*i`, descriptor `+58h+1Ch*i`; a null command is
   skipped, not a stop). When the descriptor resolves to the entity, `0071EDD0(descriptor, 1)`.
   If that answers 1 and the world mode is not 2:
   - slot 0: `0071D810(2)`, the queue stage raise, which in a local session is the 5Dh clear
     round trip the host already routes;
   - slot i > 0: `0071DEB0(command, i)`, then `0071D900(i)` (the slot clear) when it is true.

**0071DEB0** (body 0071DEB0-0071DED3, `RET 8`) answers `category(command) in {1, 2} && i > 0`.

**0071EDD0** (the director's `vtable[70h]`), `__thiscall(director)(descriptor, flag)`:
- An empty descriptor answers 1.
- An aircraft target (`vtable[5Ch](0Fh)`) with a squadron at `+9D4h`: it walks the squadron's
  members at `+3D0h` (count `+3CCh`, the first five only). It keeps a member whose `+5Dh` is clear
  and whose squared distance to the director's unit is below the double at 00D7A278.
- That double is `0x47EFFFFFE0000000`, FLT_MAX. The x87 is `FLD double [00D7A278]; FCOMIP; JBE`.
  So every live member qualifies, and the pick is the **last** live member, not the nearest.
- With a pick, it re-registers the observer on the member (006952A0, then 00694A60), rewrites the
  descriptor to that member (kind 1, id `+174h`) and answers 0. The command continues.
- Otherwise it rewrites the descriptor to the position branch at the target's `+FCh`
  (kind 0, `+1h` = 1, no object) and answers 1.

**Bodies without a Ghidra function**, for the lead to define (each verified as a RET, then INT3):
- `0071C1A0`-`0071C1A6`: a tail `JMP EAX`, INT3 from 0071C1A7.
- `0071DDB0`-`0071DEA9`: `RET 4` at 0071DEA7, INT3 from 0071DEAA. Ghidra folds it into 0071DD30.
- `0071DEB0`-`0071DED5`: `RET 8` at 0071DED3, INT3 from 0071DED6.

**What the host did.** The refusal answered the clear byte, and the observer was a record. A
director whose slot-0 `attackmove` target died was ended one fixed step later by the step's own
`attackmove` arm (00836B45, "target not live"). On reference h that is 11 ends on USN02 and 1 on
JM06, all one step after the death. No other arm ends a slot whose target died.

**The binding** (commit `c67ca09e1`, `kDirectorTargetChecksBound`, OFF):
- The gunnery kill funnel delivers 0071DDB0 to every director (`release_observed_target_0071ddb0`).
  The observer set is every director whose push resolved the entity, so scanning every director's
  slots is the same set.
- **SUBSTITUTION, timing:** the image delivers at the destroy flush. This host delivers at the
  gunnery kill. The kill runs after every director step of the same fixed step, and before the
  flush, so no director step falls between the two.
- **SUBSTITUTION, the retarget:** the host holds no squadron member list, so an aircraft target
  takes the position branch. `plane_matches` counts every descriptor that would have been offered
  the retarget.
- The refusal reads a released mirror the delivery sets (`Impl::released_05d`).
- The summary line `summary mission director release` counts in both builds: deliveries, slot
  matches, slot-0 ends, slot clears, slots kept, plane matches and refusals.

## 20. The director target-check pair, predictions (recorded before the ON runs)

**OFF counters** (this tree's build at `c67ca09e1`, runs `local\g5off_*.log`, the reference h
arguments):

| mission | frames | deaths (deliveries) | slot matches | slot-0 ends | slot clears | planes | refusals | OFF `attackmove` arm ends |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN02 | 9000 | 10 | 13 | 13 | 0 | 0 | 0 | 11 |
| USN04 | 4500 | 44 | 0 | 0 | 0 | 0 | 0 | 0 |
| USN13 | 3000 | 21 | 0 | 0 | 0 | 0 | 0 | 0 |
| USN01 | 3000 | 5 | 0 | 0 | 0 | 0 | 0 | 0 |

- **The refusal never fires** on any of the four missions: no push names a released target. It
  is exact here either way.
- **Only slot-0 matches occur, only on USN02, and none is an aircraft.** So the two
  substitutions (the retarget and the category test for slots above 0) are not reached.

**Predictions:**
- **USN04, USN13, USN01: gameplay identical** (`pair_diff` exit 0 or 1). No descriptor matches,
  so the bound path writes nothing. The deliveries still run, and only the summary line's
  `bound=` differs.
- **USN02, the mechanism:** `head_ends` stays 13. The step's own `attackmove` arm ends drop from
  11 to 0, because the command is already ended at the death step.
- **USN02, the spread:** each of those ends moves one fixed step earlier (0.05 s). The idle tail
  00836DC9 then sees an empty queue at its own step's entry rather than after a mid-step clear.
  - Either the re-issued default command is the same, and gameplay is identical (exit 1),
  - or it lands one step earlier, and gameplay moves slightly (exit 3).
  - Either way, the failure at 29.75 s (Houston, 20.85 s, a Yamakaze Long Lance) is unchanged,
    because the first end (Minegumo, 20.90 s) follows that torpedo's launch.
- **Flip rule:** the flip goes ahead when the mechanism matches: the arm ends drop to 0, no
  refusals, and identity on the three missions without matches. A USN02 spread move is recorded.

## 21. The director target-check pairs, and the flip (2026-09-29)

OFF is this tree's build at `c67ca09e1`. ON is `pair_export --commit c67ca09e1 --flip
kDirectorTargetChecksBound=true` (`local\g5_dtc`). The runs are `local\g5off_*.log` and
`local\g5on_*.log`, with the reference h arguments.

| mission | frames | `pair_diff` | death rows | slot-0 ends (ON) | `attackmove` arm ends OFF / ON | refusals |
| --- | --- | --- | --- | --- | --- | --- |
| USN02 | 9000 | exit 1, gameplay identical | 10 identical; failed at 29.75 s both | 13 | 11 / 0 | 0 |
| USN04 | 4500 | exit 1 | 44 identical | 0 | 0 / 0 | 0 |
| USN13 | 3000 | exit 1 | identical | 0 | 0 / 0 | 0 |
| USN01 | 3000 | exit 1 | identical | 0 | 0 / 0 | 0 |
| JM06 | 3000 | exit 1 | identical | 1 | 1 / 0 | 0 |

- **Every prediction held.** The mechanism matches: the arm's ends move to the death step, and
  the refusal never fires.
- **The spread** is the first branch of section 20: the idle tail re-issues the same default
  command, so gameplay is identical.
- **Flipped ON.** The unimplemented count drops by 4 on USN02 (509 to 505).
- **Still labelled:**
  - the aircraft retarget (no plane target matched on any of the five missions);
  - the category test for slots above 0 (no match above slot 0).
  They are re-checked when a mission queues more than one targeted command or targets an
  aircraft.

## 22. Ranks 4 to 9 read (cc9-gunnery5, 2026-09-29)

**Rank 4, the torpedo threat list `00814420`** (now named `BSP_Ship_CountForeignTorpedoThreats`,
body 00814420-00814492, `RET`). **Its loop never advances its node.**
- The walk `00814450..00814489` reads `ESI = [EBX+8]` from the head `EBX = [world+220h]` on
  every pass, and decrements only the count. 00814390 advances with `MOV EDI,[EDI+4]` at
  00814402.
- The list is a push-back list (`00484540`: count +0, head +4, tail +8; 00856360 registers each
  MTorpedo), so the head is the **oldest** torpedo still registered.
- **The answer is therefore all or nothing:** the list count when that oldest torpedo is live
  (`[+310h]` vtable[38h]), not the target's own (`+4F8h`), and threatening the target
  (`target->vtable[1D0h]`); otherwise 0.
- The host's stand-in (`game_hosts_ship_ai.cpp`, `torpedoes_threatening_target_00814420`) counts
  each foreign threatening torpedo instead.
- **The fix is in the ship-AI host**, which is not this worker's; it is routed to the lead.

**Rank 5, the recon convoy and group-level records.**
- `00805680` folds class-19h members through `unit+738h`. This host has no convoy membership
  producer. Only JM08 carries class-19h records (102 on reference h; 0 on USN01, USN02, USN04,
  USN13 and JM06). Binding it needs a units-host producer for `unit+738h`.
- `008069A0` (body 008069A0-00806A59) writes each group's member maximum into **the group
  entity's own** per-side detection record, `unit+1E8h+side*34h+4h`. Then it fires that record's
  change delegate.
- In this host the squadron entity is fused with its leader plane
  (`plane_squadron_registry().squadron_unit`). Publishing would therefore overwrite the leader
  plane's own detection level, which the image never does.
- **It stays a record** until a squadron entity separate from its leader exists.

**Rank 6, `0090E6C0`: not a death test.**
- Body `__thiscall(game+21A0h, unit)`. It appends the unit's id (`+174h`), once, to a per-side
  uint16 vector in the bot scheduler's record (`+140h+side*284h`, record `+13Ch`).
- The one reader of that vector found is 00914100 (from `BSP_RepairTask_Update` 0093CA20). It
  was found by the `IMUL r,r,284h` byte scan over 73 sites, then filtering for `+140h/+144h/+148h`.
- When the listed ship's health recovers past the `GA_PL` threshold (0050FC30) while alive, it
  pushes `{id, [00F876A4]}` through 0090E7C0: an award for saving a doomed ship.
- **Score only; nothing decides a death.** Closed without a pair.

**Rank 7, the death sink `008110F0`: never on the gunfire path.**
- Its only callers are the Lua natives `Sink` (00891B20) and `SetDeadMeat` (008AC7B0; the name
  is at 00D0FC7C).
- The host recorded it once per gunfire death. That record was a mislabel, and it is removed
  (identity: a record has no effect).
- The invincibility-floor branch beside it only counts, and is unchanged.

**Rank 8, the award threshold `0050FC30`.** It is a string-keyed award lookup (`GA_PL` above).
Score only; closed.

**Rank 9, the set-command message `0071C830` / `0077C2A0`: not exact.**
- In a local session, `0077C2A0` posts to the loopback queue 0076E520. The session pump 00778450
  drains it at fan-out row 9 (00875E91), after the entity think (00875E64) of the same fixed step
  (GUN_SHOT_CADENCE 10.7).
- The host stores a set-command at once (`pending_command`), and it routes its own stage-2 clear
  (`route_clear_command`) at once. The image's director step of the same fixed step sees neither
  one. It sees only the raised stage (`director+48h`), which 0071D810 writes directly.
- **Binding it needs:**
  - a message queue in the commands host;
  - a drain call from `pump_session_00778450` in the fixed-step host, which is not this worker's.
- It is proposed to the lead as its own packet. Section 21's pairs used the host's immediate
  clear, which is the same modelling every other stage-2 raise in this host uses.

## 23. The torpedo threat head and the AutoTarget accept, bound OFF, with predictions (packet `cc9_torpedo_threat_first_node`)

**The bindings** (commit `b1e6667eb`, both OFF):
- **`kForeignTorpedoThreatHeadBound`** answers 00814420 as section 22 reads it. Only the head
  torpedo is tested: the lowest serial of `live_torpedoes()`, since 00484540 is push-back. The
  answer is the live list's size when the head is foreign and threatening, else 0.
  - SUBSTITUTION: `live_torpedoes()` stands for the registered list. An exploded torpedo leaves
    the host at once, and leaves the image at the destroy flush of the same fixed step.
- **`kAutoTargetCommandAcceptBound`** answers 009F5E59's `0071D6D0(attackmove, locked target)`
  from the commands host's concrete body, `GameCommandsHost::command_accepts_target_0071d6d0`.
  That body is the descriptor 00465080 builds (kind 1, the target's id), then 0071D6D0's tests,
  including section 19's released byte. Section 17's last rank-2 term.
- The summary line `summary mission threat head` counts in both builds: calls, and how often the
  head answer differs from the per-torpedo count; accept calls, and how many accept.

**OFF counters** (`local\g5ttoff_*.log`):

| mission | 00814420 calls | head differs | 0071D6D0 calls | accepts |
| --- | --- | --- | --- | --- |
| USN02 9000 | 10484 | 0 | 9216 | 8329 |
| USN13 3000 | 0 | 0 | 0 | 0 |
| USN04 4500 | 0 | 0 | 0 | 0 |
| USN01 3000 | 0 | 0 | 0 | 0 |

**Predictions:**
- **USN13, USN04, USN01: identity.** Neither routine is asked.
- **USN02, the threat head: no change.** The head answer equals the per-torpedo count on every
  call. At every standoff test either no torpedo is live, or the torpedoes present agree.
- **USN02, the accept.** On 8329 thinks the locked target (DRKillers' SetFireTarget) is kept
  without the scan:
  - `scans` drops by up to 8329;
  - the retained score is no longer reset on those thinks.
  - The pick is the locked target either way, since the scan's `keep_locked` holds it. So the
    fire targets, shots, hits and death rows are predicted **identical** (exit 1).
  - The risk is the retained score once a lock ends. A moved row would appear only after an
    unlock, and would be a later target switch on DRKillers' ships.
- **JM06** (2147 threat calls on reference h) is added as a check of the head. It is predicted
  identical if its `differs` reads 0 OFF, and small moves in torpedo standoff (launch timing)
  otherwise.
- **Flip rule:** each switch flips when its mechanism matches. A USN02 move that is traced to the
  accept alone keeps `kAutoTargetCommandAcceptBound` OFF pending a re-read of the retained score.

## 24. The threat-head and accept pairs, and the flip (2026-09-29)

OFF is this tree at `b1e6667eb`. ON is `pair_export --commit b1e6667eb`, flipping both
switches (`local\g5_tt`). The runs are `local\g5ttoff_*.log` / `local\g5tton_*.log`, with the
reference h arguments.

| mission | `pair_diff` | death rows | 00814420 calls (head differs) | accepts / calls | AutoTarget scans OFF / ON |
| --- | --- | --- | --- | --- | --- |
| USN02 9000 | exit 1, gameplay identical | 10 identical | 10484 (0) | 8329 / 9216 | 10689 / 2360 |
| JM06 3000 | exit 1 | 2 identical | 2147 (0) | 1129 / 1129 | 3839 / 2710 |
| USN13 3000 | exit 1 | 20 identical | 0 | 0 | unchanged |
| USN04 4500 | exit 1 | 44 identical | 0 | 0 | unchanged |
| USN01 3000 | exit 1 | 5 identical | 0 | 0 | unchanged |

- **Every prediction held.**
- **The accept:** scans drop by exactly the accepted count, 8329 on USN02 and 1129 on JM06. The
  locked target is kept either way, so gameplay is identical.
- **The threat head:** it equals the per-torpedo count on every call made on these missions.
  The two answers part only when an older foreign torpedo that does not threaten sits at the
  head while a newer one does. That leaves 0 where the host counted one, or the whole list
  where the host counted a subset.
- **Both flipped ON.** The ship-AI lease is released with this commit.

## 25. Rank 9 scoped, and handoff (cc9-gunnery5, 2026-09-29, at about 62% context)

**Packet `cc9_set_command_queue_delay`** is approved (the commands host plus the fixed-step
pump). It is **not started**; this section is its design.

**The image's timing, from the listings.**
- Every local director message goes through `0077C2A0` into the loopback vector
  `session+24Ch` (count `+250h`).
  - That covers MT_COMMAND (0077D600 at 0077D7BD), MT_GAMEUNIT_SETCMD (0071C830 at 0071ED81) and
    the 5Dh clear (0071C730, 0071D900).
- The drain `0076C600` (body 0076C600-0076C737) runs from the session pump 00778450 at 00778542.
  That pump is fan-out row 9 (00875E91), after the entity think (00875E64).
- **The drain re-reads its end on every pass** (`0076C700..0076C70B`). A post made during a
  delivery goes to the insertion pointer `+258h`, which is set to the slot after the current
  message at 0076C639. So MT_COMMAND's delivery (00816E30 -> 0071ECF0) posts SETCMD, and SETCMD
  is delivered **in the same drain, next**.
- **Net:**
  - a command issued at row 2 (the Lua drain) or inside the entity think (the planner, the idle
    tail 00836DC9) reaches the slots at row 9, after every director step of that fixed step;
  - a stage-2 clear raised in a director step advances the queue at row 9;
  - `0071D810`'s stage store itself (`director+48h`) is immediate.

**What the host does.** `issue()` / `issue_command_object()` build a `ChainState` and run all
three hops synchronously. So does `DirectorStageBinding::director_raise_primary_stage_0071d810`
-> `route_clear_command`. USN02 (on 72256c4d7): issued 1345, pushed 1356, script issues 114,
clears 13.

**Why it is not a small queue.** `ChainState` carries caller-owned pointers:
- `row`, a local `GameCommandRow` that `issue()` returns to its caller;
- `ring`, the caller's `UnitOrderRing`;
- `ai_block`, `ai_setters`, `avoidance_request` and `avoidance_inputs`, from the ship-AI caller
  (`cruise_step` at about line 2665, `director_step_00836920`).

A deferred hop 2 needs these at row 9. The design:
1. **Split `route_set_command_message`** into post and deliver. Post captures the values:
   unit index, `pending_command`, `pending_target`, `pending_flag`, the heading and a copy of the
   ring.
2. **Re-provide the ship-AI pointers at delivery** through a provider the ship-AI host registers
   once, keyed by unit (the brain block is stable per controller). Check `avoidance_inputs`'
   lifetime first: it may be a per-step local.
3. **The row:** `issue()` returns the row before the slot push has happened. Its `slot_pushed` /
   `slot_index` must be filled at delivery, by an index into `host.rows` and not a pointer. Check
   every caller of the returned row (script orders and ship AI) for fields read after the return.
4. **Defer `route_clear_command`**'s three callers (the stage binding, `CompletionBinding`, and
   section 19's release path) as posts of the 5Dh message. `queue_advanced` then stays false in
   the raising step.
5. **The drain:** `GameCommandsHost::begin_loopback_drain()` before the fixed-step host's
   `script_orders_drain_loopback_0076c600()` (those orders were posted earlier), then
   `finish_loopback_drain()`:
   - it delivers this host's queue in post order;
   - a post made while draining delivers at once, which is the `+258h` insertion;
   - one switch, `kSetCommandLoopbackBound`, OFF.
   Call site: `src/game_hosts_fixed_step.cpp` `pump_session_00778450`, beside the existing
   `kAfterRow9OrderQueueBound` drain.
6. **Predictions** to write after the OFF counters: every push and every clear lands one entity
   think later. The director step that would have begun a command begins it one fixed step late
   (0.05 s), so first shots and first moves shift by a step. Expect `pair_diff` exit 3 on USN02,
   USN04, USN13 and USN01, with small timing moves; judge the mechanism by a per-command
   post-to-delivery row count.

**Where this worker stopped.** Everything is committed on `agent/cc9-gunnery5`: ranks 3 and 4, the
0071D6D0 accept, and ranks 5-9 read (sections 19-24). No lease is held.
- Tools in `local\`: `g5_queue.ps1` (the run queue; pass `-Exe` and `-Prefix`) and `g5_*.py`
  (edit scripts).
- **The remaining gunnery open items:**
  - rank 9 (above);
  - rank 5's convoy producer (routed to cc9-lua4);
  - rank 5's squadron detection publish, which needs a squadron entity separate from its leader.

## 26. The loopback queue, bound OFF (packet `cc9_set_command_queue_delay`, cc9-gunnery6)

**Switch:** `kSetCommandQueueDelayBound` (`include/bsp/game_hosts_commands.hpp`), OFF. The commits are
`daf1dc385` (the post/deliver split) and `f49312ad3` (the pump's drain call).

### The listings

**0077C2A0, the router** (body 0077C2A0-0077C467, `RET 0Ch`). Every caller here passes routing flags 7.
- 0077C329 reads `[[00E188A8]+1FE4h]`, the session mode. When it is zero, 0077C333 sets `EBX = 1`.
- Mode 2 takes 0077C3B3 (`CALL 00779F90`), and mode 1 with bit 4 takes the per-peer send loop 0077C3CB..0077C434.
- 0077C43A `TEST BL,1` then 0077C44D `CALL 0076E520` with `ECX = [00E188A8]+1EF0h` (the session). This is the local post.

**0076E520, the post** (body 0076E520-0076E70A, `RET 8`). It copies the message (0076E575 vtable[4], 0076E5A4 00768530) and stamps +18h and +14h. Then it stores the entry:
```
0076e5d1: CMP dword ptr [ESI + 0x258],EBX        ; insertion pointer null?
0076e5d7: JNZ 0x0076e5f5
0076e5e0: CALL dword ptr [0x00ce221c]            ; grow +250h
0076e5ec: MOV dword ptr [ECX + EAX*0x4 + -0x4],EBP ; append
...
0076e6bc: MOV dword ptr [EAX + EDI*0x4],EBP       ; insert at +258h
0076e6bf: ADD dword ptr [ESI + 0x250],0x1
0076e6cc: LEA EDX,[ECX + EDI*0x4 + 0x4]
0076e6d0: MOV dword ptr [ESI + 0x258],EDX         ; +258h past the new entry
```

**0076C600, the drain** (body 0076C600-0076C737):
```
0076c634: MOV EDI,dword ptr [EBX]                 ; the entry
0076c636: LEA EBP,[EBX + 0x4]
0076c639: MOV dword ptr [ESI + 0x258],EBP         ; posts from here go next
0076c65d: CALL 0x00780670                         ; delivery
0076c6c2..0076c6de                                ; erase the entry, --[+250h]
0076c6f8: CALL EDX                                ; vtable[0](1), delete
0076c6fa: MOV EAX,dword ptr [ESI + 0x250]         ; end re-read every pass
0076c709: CMP EBX,EDX
0076c70b: JNZ 0x0076c634
0076c714: MOV dword ptr [ESI + 0x258],0x0
```

**00778450, the pump.** 00778542 `CALL 0076C600` runs when `+F4h == 0` (007784F8). The pump is fan-out row 9 (00875E91), after the entity think (00875E64) and the Lua drain (00875E55).

### What this means

A message is posted from one of three places:
- the Lua drain at row 2;
- the entity think, which covers the planner, the director step and its idle tail at 00836DC9;
- a director step's stage-2 raise.

Every such message is delivered at row 9 of the same fixed step. A delivery's own posts are delivered next, in post order. MT_COMMAND's 00816E30 -> 0071ECF0 posts SETCMD at 0071ED81, so the SETCMD is delivered right after it in the same drain. `0071D810`'s stage store (`director+48h`) stays immediate.

### The host

1. **Post points.**
   - `EntityIssueBinding::route_message` covers MT_COMMAND, 0077D7BD.
   - `DirectorBinding::route_set_command_message` covers SETCMD, 0071ED81.
   - `Impl::route_clear_command` covers the 5Dh message, for all three callers: the stage binding, `CompletionBinding` through `end_command_0071e430`, and 0071DDB0's release path.
   - While the switch is OFF, each one delivers in place through the caller's own chain, which is the old path.
2. **Which hops are deferred.** Everything after 0077D600 moves to the drain. This is more than section 25 planned, because 0077D600 posts too. The resolve (0046AAB0) and 0077D600's build stay where the caller runs them. The idle tail's 0071ECF0 stays in the director step, because it is called directly, not through MT_COMMAND. Its SETCMD is posted.
3. **No ship-AI pointer provider is needed.** `ai_block`, `ai_setters`, `avoidance_request` and `avoidance_inputs` are set only by `cruise_step` (009E1170's AI arm). No issue chain carries them: `issue`, `issue_command_object` and `director_step_00836920` all pass null. Nothing needs routing to `src/game_hosts_ship_ai.cpp`.
4. **Rows.**
   - An issue stores its row in `rows` before the post, and the message addresses it by index.
   - The finish tail runs at the end of the delivery that ends the chain. The tail is the 0071BE40 current read, 00835C70's arm and the life trace. A post hands the tail on to the message it posts. The director idle tail patches its queued SETCMD with the row and the tail.
   - **The audit of the returned row** covered the callers of `issue` and `issue_command_object`:
     - `game_hosts_units.cpp` 8330, the scene's authored commands;
     - `issue_player_command` (two sites);
     - `issue_script_command`, reached from `game_hosts_script_orders.cpp` 1607, `game_hosts_ai.cpp` 1720/1728 and `game_hosts_units.cpp` 19258.
   - Those callers read `current`, `latched`, `fields`, `issued`, `projected_arm` and `blocked`. They copy them into the unit table's report columns (`game_hosts_units.cpp` 19823), the script-order row and its summary counters, or test the pointer for null. No gameplay reader was found. While ON, those copies describe the post: `issued` is true and the rest are unset. `script_blocked` is counted at delivery.
5. **Targets.** A message's target object is one of this host's unit records. `register_units` replaces `units`, so the message keeps a one-based unit handle and re-points the target at delivery.
6. **The drain.**
   - `pump_session_00778450` calls `commands_begin_loopback_drain_0076c600`, then the after-row-9 script-order drain, then `commands_finish_loopback_drain_0076c600`.
   - The script orders were posted after the previous step's row 9, so they come first. Each one's re-issue posts its MT_COMMAND and is delivered at once, together with the posts that delivery makes.
   - Then the host's queue is delivered in post order. The `+258h` insertion is exact: a delivery's posts go into the list right after it.
7. **Not moved.** 0071D880's clear-all and 0071E550's top-slot drop (0071D900) still write in place. Both run inside 00816E30 / 0071ECF0, which are now themselves inside the drain, before the SETCMD that follows them. The order is the image's, except that the idle tail's 0071ECF0 drop happens at the director step and not at row 9.
8. **Counters.** One summary line reads `summary mission director loopback bound=... command_posts setcmd_posts clear_posts in_place nested queued drains`.

### OFF counters and predictions (written before the ON runs)

The OFF runs use this tree's build of `f49312ad3` (`local\g6off_<row>.log`). The ON runs use `python tools/pair_export.py --commit f49312ad3 --flip kSetCommandQueueDelayBound=true --out local\g6_on` (SHA-256 prefix `91F17B72B789`, logs `local\g6on_<row>.log`). Both use reference h's arguments.

| row | command posts | SETCMD posts | clear posts | director-source issues (stop / follow) | issues by source | deaths | first hit |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USN02 9000 | 1345 | 1359 | 13 | 7 / 4 | 1218 AI-coordinator `attackmove`, 98 plane `moveto`, 14 NavigatorAttackMove | 10 | 19.20 s |
| USN04 4500 | 1105 | 1061 | 10 | 3 / 17 | 880 plane `moveto`, 148 NavigatorMoveOnPath, 12 squadron pass C | 44 | 101.10 s |
| USN13 3000 | 1600 | 1844 | 0 | 60 / 0 | 1444 plane `moveto`, 27 squadron pass C, 27 `attackmove` | 20 | 96.65 s |
| USN01 3000 | 119 | 173 | 2 | 9 / 0 | 98 plane `moveto`, 5 PilotSetTarget | 5 | 53.60 s |

**The mechanism predictions**, which decide the flip:
- **M1, the counters.** ON prints `in_place=0` and `drains>0`. `queued` is about the MT_COMMAND posts, plus the clears, plus the director idle tail's SETCMDs, which are the ones posted outside a drain. `nested` is about every other SETCMD, plus the after-row-9 script orders' re-issues. A same-step delivery shows as `queued`, and a SETCMD delivered right after its MT_COMMAND shows as `nested`. Any in-place delivery while ON means a missed post path.
- **M2, the director idle tail.** Each idle-tail `stop` / `follow` issue is pushed at row 9 of its own step, not inside the director step. The step that raises stage 2 no longer advances the queue: its clear waits for row 9, so that step's idle tail still sees the finished head at stage 2. Idle reissues that follow a stage-2 raise therefore begin one fixed step (0.05 s) later. The totals `idle_reissues`, `stop` and `follow` stay within a few of OFF's.
- **M3, other issues.** A command issued by the Lua drain (row 2) or inside the entity think reaches slot 0 after that step's director step. The director's arms and the ship-AI state that reads slot 0 act on it one step later. Every director step sees its new command one fixed step late. The `attackmove` retargets of the AI coordinator dominate this on USN02, and the plane `moveto`s dominate it on USN04, USN13 and USN01.

**The spread predictions** (a miss here alone may still flip, recorded):
- **USN02:** `pair_diff` exit 3. Opening fire is the AutoTarget's, not the director's, so the first hit stays at 19.20 s within 0.1 s. Houston's Long Lance death (about 20.85 s) and the failure at 29.75 s stay, within a few steps. Later death times move by small amounts. The death count is 10 ± 2.
- **USN04:** exit 3. The plane paths start one step late, so task releases and the kills that follow move by steps. The death count is 44 ± 3, and the first hit is 101.10 s ± 0.5 s.
- **USN13:** exit 3. The death count is 20 ± 2, and the first hit is 96.65 s ± 0.5 s.
- **USN01:** exit 3 with small moves. The death count is 5, and the first hit is 53.60 s ± 0.2 s.
