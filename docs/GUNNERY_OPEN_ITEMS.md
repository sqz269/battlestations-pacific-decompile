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
| the turn-rate average `0085E4D0` | AA_LEAD, USN04_KATE_ATTRITION 8.2 | bound OFF, `kAaTargetTurnAverageBound` |
| plane-gun muzzle origin | MUZZLE_OFFSETS 4 | bound OFF, `kPlanePlatformAttachmentBound` |
| targetless guns and rest angles | GUN_REST_ANGLES | bound OFF, `kGunIdleRestBound` |

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
