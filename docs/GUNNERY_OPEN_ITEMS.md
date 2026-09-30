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
| the AutoTarget candidate list `009F5D30` over `008053C0` (rank 1 on h) | AUTOTARGET_RECON_CANDIDATES 6 | ON, `kAutoTargetReconCandidatesBound` |
| the AutoTarget's commanded-target adoption `009F5E69` / `00521EA0` / `00465080` (rank 2 on h) | section 17 | read, exact while no `WeaponDirector::queue_command` runs (none on reference i) |
| the "death test" `0090E6C0`, the death sink `008110F0`, the award threshold `0050FC30` (ranks 6-8 on h) | section 22 | read; score only, or not on the gunfire path; the sink record removed |
| the set-command message `0071C830` / `0077C2A0` and the stage-2 clear (rank 9 on h) | sections 25-29 | ON, `kSetCommandQueueDelayBound` and `kSetCommandClearAllMessageBound`; the two rows still print `UNIMPLEMENTED` because the host records the hops it delivers itself |
| the projectile team id | section 26 | stamped; no switch |
| USN04's dive-release drop 10 -> 4 | section 30 | the image's own; nothing bound |
| the submarine's sensor category `00852B90` (rank 1 on i) | section 32 | ON, `kSubmarineSensorCategoryBound`; the periscope byte `+1234h` stays a labelled substitution |
| the forced fire target's handle at `00835930` (rank 2 on i) | section 33 | ON, `kFireTargetObjectIdBound`; the handle resolves by object id |
| the attack-move arm on a command building `00836B95` (rank 3 on i) | section 35 | read; exact on the measured missions (0 conversions); the conversion is not issued |
| the director slot housekeeping in `00720850`: `006952A0`, `00414DB0`, `0071FB90`, `007208A3` (rank 4 on i) | section 36 | read; exact in effect on the measured missions; the queue-full path-object hole is traced |

**Still open from the closed rows:** the periscope byte `+1234h` (`periscopeOut`) has no producer,
so a raised periscope never reads PeriscopeOut (section 32.4).

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

## 16. The ranking refreshed on reference h (packet `cc9_gunnery_open_ranking_2`, 2026-09-28)

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

## 18. Handoff (cc9-gunnery4, 2026-09-28, at about 75% context)

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
- The reference is h (docs/GAME_EXECUTABLE.md 2026-09-28 h, main `d6fc6ee78`).
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

## 21. The director target-check pairs, and the flip (2026-09-28)

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

## 22. Ranks 4 to 9 read (cc9-gunnery5, 2026-09-28)

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

## 24. The threat-head and accept pairs, and the flip (2026-09-28)

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

## 25. Rank 9 scoped, and handoff (cc9-gunnery5, 2026-09-28, at about 62% context)

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

## 26. The projectile team id (packet `cc9_projectile_team_id`, no switch)

The hit event's `attacker_player_index` now has a producer: the shot's `team_id_1c`
(`[shot+1Ch]`, projectile+18Ch). docs/LUA_BINDING_MISSION.md, "attackerPlayerIndex", has the
consumer.

**How each shot is stamped:**
- **Gunfire:** 0072BF10's stamp at 0072C0F2..0072C14B.
  - The team is `gun+1ACh`.
  - When that is 8 and the gun is an MRFSGun (`vtable[5Ch](21h)`), the team is the owner's
    role-1 slot `[owner+1B0h]`.
  - An aircraft owner with `+914h > 0.0` is first replaced by its squadron head
    `[[owner+9D4h]+3D0h]`.
- **Bombs:** 006E4D50 stamps `[plane+1B0h]` (006E5527).
- **Torpedo and depth-charge drops:** untraced; they stay -1.

**Labelled substitutions:**
- the category-0 rows stand for the class 21h test (as in 007C2610);
- there is no `+914h`, so the owner is never replaced by its squadron head.

**The team reaches the hit event** through `apply_hit` and `apply_impact_blast`.

**Identity check against the head's code** (`local\g5tton_*` against `local\g5team_*`):

| mission | `pair_diff` | death rows |
| --- | --- | --- |
| USN04 4500 | exit 0 | 44 identical |
| JM06 3000 | exit 1 (gameplay identical) | 2 identical |

- Only the Lua hit filters read the field.
- JM06's `hshit` (attackerPlayerIndex PLAYER_1) still fires 0 times: no player shot hits the
  hospital ship on an idle run.

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

## 27. The loopback-queue pairs, and the flip (cc9-gunnery6, 2026-09-28)

**The first ON pair failed on a receiver, not on the queue.** It was run on `f49312ad3` (logs `local\g6on_<row>.log`), and USN04 fell from 44 deaths to 19. PilotSetTarget's continuation runs 0099A170, the bot task install that reads the command the director holds. It ran right after the issue, before the order's row-9 delivery, so only 3 of 19 dive-bomb tasks installed. Two commits fixed it:
- `1a6149672`: `after_order_delivery` attaches the continuation to the last issue's queued MT_COMMAND (`GameCommandsHost::after_last_issue_delivery`). The continuation now runs after that chain's push and finish tail.
- `c25c1fe7d`: the squadron fan-out's wingman installs go to each wingman's own delivery. The wingman orders are inserted next in the drain (`+258h`).

The path pair (`set_path_follow_pair_0071c1b0`) takes the same route. While the switch is off, all of these run in place as before. `BSP_LOOPBACK_TRACE=<n>` prints the first n posts with the drain state they met.

**The pairs.** OFF is this tree's build of `c25c1fe7d` (`local\g6off2_<row>.log`). It is `pair_diff` exit 0 against `f49312ad3`'s OFF on all four rows. ON is `pair_export --commit c25c1fe7d --flip kSetCommandQueueDelayBound=true` (SHA-256 prefix `66956500BEE8`, `local\g6on2_<row>.log`).

| row | pair_diff | deaths | first hit | loopback ON (in_place / nested / queued / drains) | idle reissues OFF = ON |
| --- | --- | --- | --- | --- | --- |
| USN02 9000 | 3 | 10 = 10, 5 rows moved | 19.20 = 19.20 s | 0 / 1345 / 1372 / 164 | 11 (7 stop, 4 follow) |
| USN04 4500 | 3 | 44 -> 45 | 101.10 = 101.10 s | 0 / 1058 / 1118 / 223 | 21 |
| USN13 3000 | **1** | 20 = 20, identical | 96.65 = 96.65 s | 0 / 1645 / 1799 / 56 | 244 |
| USN01 3000 | 3 | 5 = 5, 2 rows moved | 53.60 = 53.60 s | 0 / 127 / 167 / 78 | 52 |

**The mechanism, against the predictions.**
- **M1 held.** Every row has `in_place=0`. On the rows without a fan-out, `queued` equals the MT_COMMAND posts plus the clears plus the idle-tail SETCMDs:

  | row | MT_COMMAND | clears | idle-tail SETCMDs | queued |
  | --- | --- | --- | --- | --- |
  | USN02 | 1345 | 13 | 14 | 1372 |
  | USN01 | 119 | 2 | 46 | 167 |

  `nested` is the SETCMDs posted by an MT_COMMAND's delivery. On USN04 and USN13 it also counts the wingman MT_COMMANDs issued inside a delivery, together with their SETCMDs.
- **M2 held.** The idle-tail totals (`idle_reissues`, `stop`, `follow`) are identical OFF and ON on all four rows.
- **M3 is consistent.** The moved death rows shift by one or a few fixed steps. For example:
  - USN02: Yamakaze dies at 142.30 -> 142.35 s and John2 at 175.11 -> 175.21 s.
  - USN01: Mav1 dies at 70.95 -> 71.00 s.
  - USN04: B5N Kate #4.1 dies at 123.55 -> 123.80 s.

  Command histories are unchanged. USN02's Kortenaer has the same 145 `attackmove` rows and one `follow`, though it moves 749 m against 1137 m through the changed exchange.

**The spread.**
- **USN02** matches: exit 3, the first hit is unchanged, Houston's opening Long Lance is unchanged, and the mission still fails at 29.75 s.
- **USN01** matches.
- **USN04** matches on deaths (45, predicted 44 ± 3) and on the first hit. Dive-bomb releases fall from 10 of 19 to 4 of 19, which the prediction did not cover:
  - The installs are 19 of 19 on both sides. The per-aircraft state tables show the same states, with arm ticks a few apart.
  - The whole difference is in single dives at the 25 m aim gate: `D3A Val #1.1|.-2` goes from 5 releases to 0, and `#3.1|.-4` from 1 to 0.
  - This is recorded as the dive-release knife-edge, not a mechanism miss.
- **USN13 missed the prediction.** It is gameplay-identical (exit 1), where exit 3 was predicted. The delay moves no USN13 death, hit or shot within 3000 frames.

**Verdict: ON.** M1 and M2 held and M3 is consistent. The misses are on spread only: USN13 identical, and USN04's release count. `kSetCommandQueueDelayBound = true`.

**Still open.**
- 0071D880's clear-all and 0071E550's top-slot drop write in place.
- The idle tail's 0071ECF0 make-room drop happens in the director step, not at row 9.
- 0099A170's install is at the delivery. The image installs it one bot tick later (0099ACD0 behind `+7Ch`); that substitution predates this packet.

## 28. The hop-1 clear-all and drop as messages (packet `cc9_set_command_clear_all`, cc9-gunnery6)

**Switch:** `kSetCommandClearAllMessageBound`, OFF. It sits under `kSetCommandQueueDelayBound` (ON). The commits are `78895f00c` and `5c30a8101`.

**The image.**
- 00816E30 calls 0071D880 at 0081733E whenever the message flags are non-zero. That is every scripted, authored and AI-coordinator order: 1345 of 1345 deliveries on USN02.
- 0071D880 builds MT_GAMEUNIT_CLEARCMD with `+20h = 1` and `+24h = -1` and routes it with flags 7 through 0077C2A0. It is therefore posted, and inside the drain it is inserted next.
- 00721A40's 5Dh arm (00721BA8) passes it to 00720CA0, body 00720CA0-00720CCA, read whole:
```
00720ca5: MOV ESI,0x9
00720caa: LEA EDI,[EBX + 0x150]      ; slot 9's command, director+54h+1Ch*9
00720cb0: CMP dword ptr [EDI],0x0
00720cb3: JZ 0x00720cbd
00720cb8: CALL 0x00720850            ; 00720850(ESI)
00720cbd: SUB ESI,0x1 / SUB EDI,0x1c / TEST ESI,ESI / JGE 00720cb0
```
- The receiver is `src/command_execution.cpp`'s `clear_all_command_slots_00720ca0`. Its last call, 00720850(0), is the head completion. It does the following:
  - unregisters the slot-0 target's observer;
  - records the previous command;
  - snaps the slot to its target;
  - sets the mode to idle when fewer than two slots are occupied;
  - calls vtable[6Ch](1) at 00720B56, which resets the stage pair through 0071C130.
- 0071ECF0's make-room (0071E550) then reads the queue as it was before the clear. When both the incoming command and the top slot are category 1 or 2, it posts 0071D900(count - 1) at 0071E5AA.
- The delivery order is: the clear-all, then the drop, then the SETCMD. The drop meets an idle mode with slot 0 empty, and 00720850 returns at 007208C0..007208E5. It is a no-op in the image.

**The host before this.** `DirectorBinding::clear_all_commands` zeroed the ten slots in place. It did not run 00720850, so it reset no stage, set no mode and cleared no observer. It ran before make-room, so make-room always saw an empty queue: `drops=0` on every row.

**The binding.** Both writes go through `route_clear_command` and are delivered through the 5Dh arm. The every-slot action calls the 00720CA0 reconstruction. `unit+184h` for the receiver is the value the unit's last director step was given (`Impl::player_184`).

### OFF counters and predictions (written before the ON runs)

The OFF logs are `local\g6caoff_<row>.log`, built from `78895f00c`, whose OFF code is identical to `5c30a8101`'s. Against the section-27 ON logs they give `pair_diff` exit 1 on all four rows: only the new summary line differs.

| row | clear-all | drops | 00720CA0 slot clears |
| --- | --- | --- | --- |
| USN02 | 1345 | 0 | 0 |
| USN13 | 1600 | 0 | 0 |
| USN04 | 957 | 0 | 0 |
| USN01 | 119 | 0 | 0 |

**Mechanism (decides the flip):**
- **C1, the counts.** The clear-all count stays within a few of OFF's. `slot_clears_00720ca0` becomes non-zero on every row: one for each slot occupied when a clear-all is delivered. `drops` becomes non-zero on USN02, where the AI coordinator re-issues `attackmove` (category 2) onto an `attackmove` top. It stays 0 or near 0 on USN13 and USN01, where the planes' `moveto` is category 3. It is small on USN04, from PilotSetTarget's category-2 orders onto a category-2 top. `clear_receives` rises by the clear-alls plus the drops.
- **C2, the queue state.** A re-issue onto a non-empty queue now leaves the mode idle and the stage pair at 0 before its SETCMD is pushed. Before, the stage and the mode were kept. Every drop is a no-op.

**Spread:**
- USN13, USN01 and USN04 are gameplay-identical (exit 1). Their re-issues replace a `moveto` or an attack order whose stage is still 0.
- USN02 is exit 1, or exit 3 with small moves where an AI-coordinator retarget lands on a head whose stage was already raised.
- The death counts equal OFF's on all four rows.

## 29. The clear-all pairs, and the flip (2026-09-28)

ON is `pair_export --commit 5c30a8101 --flip kSetCommandClearAllMessageBound=true` (SHA-256 prefix `00CA7C47C8F8`, `local\g6caon_<row>.log`). OFF is section 28's `local\g6caoff_<row>.log`.

| row | pair_diff | deaths | clear-all | drops | 00720CA0 slot clears | clear_receives OFF -> ON |
| --- | --- | --- | --- | --- | --- | --- |
| USN02 9000 | 1, identical | 10 | 1345 | 1199 | 1318 | 13 -> 2557 |
| USN13 3000 | 1, identical | 20 | 1600 | 0 | 1471 | 0 -> 1600 |
| USN04 4500 | 1, identical | 45 | 957 | 0 | 965 | 10 -> 967 |
| USN01 3000 | 1, identical | 5 | 119 | 2 | 107 | 2 -> 123 |

- **C1 held.** The clear-all counts equal OFF's, and `slot_clears_00720ca0` is non-zero on every row. The drops are 1199 on USN02 (AI-coordinator `attackmove` onto an `attackmove` top), 0 on USN13 and USN04, and 2 on USN01. `clear_receives` = clear-alls + drops + the stage-2 clears exactly: USN02 1345 + 1199 + 13 = 2557.
- **C2 is consistent.** Every drop is delivered after its clear-all. The idle-tail totals, `pushed` and every death row are unchanged, which is what a no-op drop and a stage reset on a stage-0 head give.
- **Spread held**: exit 1 on all four rows.

**Verdict: ON.** `kSetCommandClearAllMessageBound = true`.

**Still open.** The idle tail's own 0071ECF0 make-room runs in the director step, and its drop now posts for row 9 like the others. The only remaining in-place director write in this path is 0071D810's stage store, which the image also makes directly.

## 30. The USN04 dive-release drop: the first dive step against the image (read, cc9-gunnery6)

**Question** (reference i's flag): with the loopback queue ON, USN04's dive-bomb releases fall from 10 of 19 to 4 of 19, while installs stay at 19 of 19. Does the image's dive-bomb task take its first step, relative to the 25 m aim gate, at the same fixed step as the host now, or one step earlier or later?

**The image's timing**, for a PilotSetTarget run by the Lua drain (row 2) of step N:
1. The MT_COMMAND is posted through 0077D600 / 0077C2A0 / 0076E520 and delivered at row 9 of step N. The SETCMD follows it in the same drain, so the director holds the command from row 9 of N (sections 26-27).
2. The task is installed by the bot tick 0099ACD0, not at delivery. `docs/PILOT_BOT_TICK_GATES.md` has the retire-then-install; `src/pilot_command_path.cpp` `run_pilot_bot_command_tick_0099acd0` reconstructs the order:
```
0099ae72: MOV ECX,dword ptr [ESI + 0x58]   ; head task before
0099ae7b: PUSH EBX
0099ae7c: MOV ECX,ESI
0099ae7e: CALL 0x0099a4c0                  ; retire head tasks that are over
...
0099a5e8: mov ecx, esi                     ; list at +58h empty:
0099a5eb: jmp 0x99a170                     ; tail-jump to BSP_Bot_InstallCommandTask
...
0099af1c: CALL 0x009998a0                  ; BSP_PilotBot_Update
009998fb: CALL EAX                         ; task->vtable[64h](dt), the per-kind arm
```
   So a task installed in a tick takes its first arm step in that same tick. The bot tick is the pilot think in the entity think (row 7). The first bot tick after the row-9 delivery is step N+1. **The image's first dive step is therefore at N+1 at the earliest.** It is later only when 0099A4C0 does not yet retire the old head, because its `vtable[38h]` / `[34h]` / `[40h]` predicates keep it. That can delay the install; it cannot bring it forward.
3. The aim gate itself is not timing-dependent. It is the 25 m window at `00CE3880`, read at `009C60C1` after `009C60BB COMISS XMM0,[ESI+1Ch] / 009C60BF JB 009C611C` (`docs/DIVE_BOMB_TASK.md`, "The gate now").

**The host's timing:**
- **Before the queue** (OFF, `kSetCommandQueueDelayBound = false`): the install ran inside the issue at row 2 of step N (`after_order_delivery` in place). `run_dive_bomb_task_arm_009c8790` then ran on the pilot think of step N. That is **one fixed step earlier** than the image's earliest.
- **Now** (ON, `1a6149672` / `c25c1fe7d`): the install runs after the row-9 delivery of step N. The first arm step is the pilot think of step N+1, which is the image's earliest.

**Verdict.** The ON host's first dive step matches the image's earliest, so the difference against the old host is the image's own: the old host started every ordered dive one step early. USN04's 10 -> 4 releases is that one-step start at the aim gate, a knife-edge, and it is recorded as the image's. **Nothing is bound.**

**Still open.**
- **The retire predicate.** If the old head task survives 0099A4C0 for some ticks, the image starts later still. The candidates are the move-to or squadron task a PilotSetTarget aircraft holds, and `vtable[38h]`, `should_abandon` when `+2F4h == *(+2FCh + 3D0h)` (`docs/BOT_TASKS.md`). The host has no per-kind retire predicates for those tasks. That read, and moving the install into the retire path, belongs to the units and script-orders hosts (`docs/SENTITY_INIT_ATTACH_ORDER.md` 22.7).
- **Reference i** should take USN04's releases from an ON build and cite this section.

### Addendum: the gate's place in the step, and Val #1.1|.-2's lost releases (packet `cc9_dive_release_timing`)

**The gate runs before the same-step delivery.**
- The 25 m test (`009C60BB COMISS` / `009C60BF JB` / `009C60C1`) is part of the dive task's arm `task->vtable[64h]`. The arm runs from the pilot bot tick (`0099AF1C` -> `009998FB`) in the entity think, fan-out row 7 (00875E64).
- The order's delivery is row 9 (00875E91, 0076C600).
- In the image, the gate of step N therefore never sees an order posted in step N. It sees it from step N+1. The ON host is the same: the install follows the row-9 delivery, and `run_dive_bomb_task_arm_009c8790` runs on the next pilot think.

**Val #1.1|.-2** (`local\g6off2_usn04.log` against `local\g6on2_usn04.log`, the section-27 pair). OFF has five releases, ON has none.

| | OFF (old host, one step early) | ON (image timing) |
| --- | --- | --- |
| hand-overs (arm tick) | flyabove>turndown 1010, turndown>aimdive 1064, aimdive>aimglide 1202 | 1009, 1063, aimdive>goaway 1221 |
| dive entry | 875.3 m, pitch -0.419 | 878.1 m, pitch -0.419 |
| 009C58D0 steer at exit | pitch 0.941, roll -1.000, bearing -0.711 rad | pitch 1.000, roll 0.093, bearing -0.037 rad |
| abort 009C5B43 | fired at arm tick 1201: range 257.1 m, h14 365.3 m, d4 724.6 m | never fired |
| aimdive exit | `alive_19` into aimglide at 358 m | `pullout_18` into goaway at 214 m |
| aim error closest | 0.26 m at range 287.6 m, alt 397.2 m | 1.51 m at range 436.4 m, alt 505.0 m |
| releases | 5 bomb requests, t = 144.70 s (alt 402.9 m) to 146.60 s (alt 264.7 m) | 0 |
| impact 009C7D71 | range 412.2 m | range 583.9 m |

**Reading.**
- The five releases were not lost at the 25 m window. The ON aim error passed inside it, at 1.51 m.
- The OFF releases come from aimglide, and aimglide is reached only through 009C5B43's abort test. ON dives with a different run-in: it is 170 m further from the impact point at the same stage, with the roll released. The abort never fires, and the pull-out ends the dive at 214 m.
- The hand-over ticks differ by one, and the target geometry differs through the changed exchange around Lexington, a whole battle's trajectory.
- Under the image's timing, which is the ON host, this dive does not release. The image would have released only on a run-in that meets 009C5B43, and this one does not.

**Verdict.** The release drop is recorded as the image's own consequence of the command delay: a changed dive geometry at the abort test, not a timing difference at the aim gate. Nothing is bound, and no pair is needed.

## 31a. Handoff (cc9-gunnery6, 2026-09-28, at about 72% context)

Rank 9 is closed. All of this worker's commits are on main (`2ce6c92cf`, `ea62f089a`, `796d5e684`, `d466d4250`). No lease is held.

### The loopback queue as it stands

Two switches in `include/bsp/game_hosts_commands.hpp` are both ON:
- `kSetCommandQueueDelayBound` (sections 26-27);
- `kSetCommandClearAllMessageBound` (sections 28-29).

**What is posted, and where** (`GameCommandsHost::Impl::post_loopback`, the image's 0076E520):

| message | posted from | image site |
| --- | --- | --- |
| MT_COMMAND | `EntityIssueBinding::route_message`: every `issue` / `issue_command_object` (scene authored, AI coordinator, script orders, player `--order`) | 0077D7BD |
| SETCMD | `DirectorBinding::route_set_command_message`: 0071ECF0, from MT_COMMAND's delivery or directly from the director idle tail 00836DC9 | 0071ED81 |
| 5Dh, one slot | `Impl::route_clear_command`: 0071D810's stage 2 (`DirectorStageBinding`), `CompletionBinding` (0071E430), 0071DDB0's release path, 0071E550's drop (0071D900) | 0071C730 / 0071D900 |
| 5Dh, every slot | `DirectorBinding::clear_all_commands`: 00816E30 at 0081733E, delivered through `clear_all_command_slots_00720ca0` | 0071D880 / 00720CA0 |

**Where it is delivered.**
- **The drain.** `GameFixedStepHost::pump_session_00778450` (fan-out row 9, 00778542) calls `commands_begin_loopback_drain_0076c600`, then the after-row-9 script-order drain, then `commands_finish_loopback_drain_0076c600`.
- **Posts made while the drain is open** are delivered at once, together with their own posts. That covers the script orders' re-issues.
- **Then the queue** is delivered in post order. A delivery's own posts are inserted right after it (`+258h`, 0076C639). So an MT_COMMAND's clear-all, its make-room drop and its SETCMD are all delivered in the same drain, in that order.
- **Rows and continuations.** An issue's `GameCommandRow` is stored at post and addressed by index. The finish tail (0071BE40 current read and 00835C70's arm) runs at the end of the delivery that ends the chain.
- **Receiver continuations** are PilotSetTarget's 0099A170 install, the squadron wingmen's installs and the path pair. They attach to the last issue's MT_COMMAND through `GameCommandsHost::after_last_issue_delivery` / `commands_after_last_issue_delivery`, and run after that chain.

**What callers see.** A caller of `issue` / `issue_command_object` gets a row that describes the post: `issued` is true, and `current`, `latched`, `fields`, `projected_arm` and `blocked` are not yet set. The section 26 audit found only report readers of those fields.
- **Rule for new code:** anything that must read the director after an order belongs in a continuation (`after_order_delivery` in the script-orders host, or `commands_after_last_issue_delivery`), never right after the issue call.
- **A symptom to recognise:** a feature that silently does nothing under the queue, as the USN04 dive-bomb installs did (section 27), is almost always this.

**Labels left, as recorded in sections 27 and 30:**
1. **0099A170's install point.** The install runs at the order's delivery. The image installs in the bot tick, from 0099A4C0's retire path (0099AE7E, tail-jump 0099A5EB). Both give the first dive step at N+1 when the old head retires at once. The host has no per-kind retire predicates (`vtable[38h]` / `[34h]` / `[40h]`). If the image keeps the old head for some ticks, it starts the new task later than the host.
2. **Unit+184h for the 5Dh receiver.** A clear posted from hop 1 uses `Impl::player_184`, the flag the unit's last director step was given. The image reads the unit at delivery. The two differ only across the step in which control changes.

**For the units / script-orders side** (`docs/SENTITY_INIT_ATTACH_ORDER.md` 22.7, `docs/PILOT_BOT_TICK_GATES.md` "What installs a task on a command change"): read the retire predicates of the head tasks a PilotSetTarget aircraft holds, the move-to and the squadron follow task. Then move the install from the delivery continuation into the bot tick's retire path. That work belongs to the units and script-orders hosts, not to this file.

### The diagnostic

`BSP_LOOPBACK_TRACE=<n>` (env, read once, prints nothing when unset) logs the first n posts:
```
  loopback post <k>: kind=<0 MT_COMMAND | 1 SETCMD | 2 5Dh> unit=<index> row=<rows index or -1>
      finish=<1 if the chain's finish tail travels with it> active=<1 inside a delivery>
      open=<1 drain open, delivered at once> queue=<entries waiting> serial=<drains so far> clock=<s>
```
- `active=0 open=0` is a post waiting for the next row-9 drain.
- `active=1` is an insertion after the message being delivered.
- `open=1` is a script order's re-issue during the pump's own script-order drain.

The run summary line reads:
```
summary mission director loopback bound=1 command_posts setcmd_posts clear_posts in_place nested queued drains
```
- With the queue ON, `in_place` must be 0. Anything else is a missed post path.
- `queued` counts entries posted before the drain, and `nested` counts entries a delivery posted. On rows without a squadron fan-out, `queued` equals MT_COMMAND posts + clears + idle-tail SETCMDs.
- **Do not read the similar totals as swapped.** `nested` equals the SETCMDs posted by deliveries, and it can equal the MT_COMMAND count by arithmetic alone.

`summary mission director clear-all bound clear_all drops slot_clears_00720ca0` counts 0071D880, 0071D900 and the 00720850 bodies 00720CA0 ran. `clear_receives` in the completion line is clear-alls + drops + the stage-2 and release clears.

### Tools this worker left in its tree

The tree is `J:\PROG\battlestations-pacific-decompile-cc9-gunnery6\local\`, not committed.
- `g6_queue.ps1` is the row runner. Pass `-Rows`, `-Exe` and `-Prefix`. It uses reference h's arguments and sets `BSP_GUNNERY_RNG_STREAMS` and `BSP_DEATH_TABLE`. A 9000-frame USN02 takes about 2 minutes.
- The pair logs are `g6off2_*` / `g6on2_*` (the queue-delay pair) and `g6caoff_*` / `g6caon_*` (the clear-all pair).
- A quick local ON check works like this: flip the constant in the working copy, build, run, and revert before committing. It is much faster than `pair_export`, and it is how the section 27 receiver misses were found. The verdict pair must still come from `pair_export`.

### Next in the gunnery list

These are unchanged from section 25:
- rank 5's convoy producer, routed to cc9-lua4;
- rank 5's squadron detection publish, which needs a squadron entity separate from its leader.

Reference i (cc9-gunnery7) takes USN04's dive releases from an ON build and cites section 30.

## 31. The ranking refreshed on reference i (packet `cc9_gunnery_open_ranking_3`, cc9-gunnery7, 2026-09-28)

**Source.** The eight reference i logs `local\rb9_{usn04,e2,usn01,usn02,jm06,jm08,usn13,lomp06}.log`
in worktree cc9-gunnery7 (main `d466d4250`, rb9 SHA-256 prefix `D119E0505144`;
docs/GAME_EXECUTABLE.md "2026-09-28 i").
- `local\g7_rank.py` is section 16's census, pointed at these logs.
- `local\g7_rankdiff.py` runs the same census over h's logs as well, with the ship-AI and
  command classes added. It prints each row's status and calls on both sides.
- `local\g7_standins.py` prints what each remaining stand-in answers.

**Closed since h** (moved to section 1's table): h's ranks 1, 2, 3, 4, 6, 7, 8 and 9, and the
accept. The census agrees:
- `AutoTarget::party_recon_slot`, `command_accepts_target`, `WeaponDirector::observe_target` and
  `target_refuses_commands` (`0071D712`) now print `concrete`.
- `threats_00814420` has a Ghidra function and prints `concrete`.
- `EntityCommand::clear_all_commands`, `GameUnitMessage::clear_every_slot` and `drop_top_slot`
  are concrete.
- The death-sink row is gone.

**The new ranking.** Reach is section 16's: 4 decides who is shot at or a death, 3 an order, 2 a
count or a score, and 1 presentation. Calls are summed over the eight logs.

| rank | item | image | reach | calls (i) | what the host does now |
| --- | --- | --- | --- | --- | --- |
| 1 | **the submarine's sensor category** | `00852B90`: hull Y against the four depth words `unit+1200h..120Ch` and the periscope byte `+1234h` (docs/SENSOR_TABLES.md "The submarine") | 4 | 1173 (JM06 1074, LOMP06 99) | answers **PeriscopeIn for every submarine** (`game_hosts_gunnery.cpp`, `unit_sensor_category`). Its label says the depth bands are not read. **That label is stale**: since SUBMARINE_MODEL 12 the host holds each boat's bands (`submarine dive bands: ... bound=1`), and those bands are these four words (SUBMARINE_MODEL, the `+1200h` table). A boat held at -40 or -80 m is Underwater or DeepUnderwater in the image, and the recon pass that now feeds the AutoTarget reads that category |
| 2 | the unresolved fire target | `00835930` with an object that is not one of this host's unit records | 4 | 154 (USN13 60, USN04 35, E2 35) | records and sets no fire target |
| 3 | an attack-move on a command building converts to `moveto` | `00836B95` (`00465080`, `0071ECF0`, then stage 2) | 3 | 1010 (USN13 1002, USN01 8); h had 23328 on USN01 | records, and the attack-move stays. **Its label, "no building target reaches this arm in the measured runs", is false** on h and on i. USN13's calls follow the planner Capture path (PLANNER_TASK_CHOICE 8) |
| 4 | the director slot housekeeping in the slot clear `00720850` | `0071FB90` path object, `006952A0` observer unregister, `00414DB0` target pose, `007208A3` trace | 3 (unconfirmed) | 6755 / 1733 / 1731 / 8298 | 0 and records. They moved from 86 calls on h because the clear-all now reaches `00720850` on every slot (sections 28-29). A target whose observer is never unregistered can still reach the release observer `0071DDB0`, 14 calls here |
| 5 | the command-allowed extra tests | `009229F0` (command `00E08F18`), `007AC9D0` (`00E08F80`) | 3 | 268 | records; the command is allowed without the extra test |
| 6 | the recon convoy and group-level records | `00805680`, `008069A0` | 3 | 1467 each | unchanged from section 22: needs a `unit+738h` producer and a squadron entity separate from its leader |
| 7 | the AutoTarget's controller-ownership test | `007788B0`: `[ctl+284h] != 0` and `[[ctl+284h]+14h] != ctl` | 4 if true | 148923 | answers false. That is the image's answer whenever `+284h` is empty or names this controller; the host has no producer for `+284h`. Not checked against a player-controlled unit |
| 8 | the hull roll torque from a hit | `00827312` | 2 | 52 (USN02) | record; the roll scale reads 0 |

**Exact, and kept off the ranking:**
- `009F5E69`, `00521EA0` and `00465080` in the AutoTarget are exact by section 17. That holds while
  `WeaponDirector::queue_command` has no calls, and it has none on reference i.
- `GameUnitMessage::resolve_target_object` returns the object itself.
- `008FB530`, the torpedo-bot accuracy, answers the robots.lua table.
- `008654AC`, the sub-entity list, is exact for every unit this host builds.
- The set-command rows `0071C830` / `0077C2A0` still print `UNIMPLEMENTED`, but the queue and
  clear-all switches bind their behaviour (section 1).

**Not ranked, and why** (unchanged from section 16):
- Plane-side rows: `PilotBot::*`, `BotState*`, `BotApproach*` and `TorpedoApproach::run_profile_record_14h`.
- The player HUD rows.
- The ship-AI host's rows: `ShipAiOrder::slot_to_order_ring`, `ShipAiFollow::*`, `ShipAiApproach::*`
  and `ShipAiState::attack_subject_00779aa0`. They belong to the ship packets.

**Labelled substitutions whose labels no longer hold:** rank 1 (the depth bands are read now) and
rank 3 (the building arm is reached). The others section 16 listed are exact at these missions'
rows: the bullet throw's `unit+63Ch = 1.0` and `00470440(7) = 1.0`, the flak distance error of 0,
and the torpedo swim's surface plane.

**Next packet: rank 1, `cc9_submarine_sensor_category`.**
- **Read** `00852B90` against the host's dive bands. Confirm that the four words at
  `+1200h..+120Ch` are the band table the dive binding holds, and what writes the periscope byte
  `+1234h` on JM06 and LOMP06. Then find the consumers of the category in the recon pass
  (`008048A0`'s submerged-target branch and the `SubjectDeepUnderwater` rows, SENSOR_TABLE_DATA).
- **Bind** the category from the hull's Y and the bands, OFF, with predictions on JM06 and LOMP06,
  and identity on USN01, USN02, USN04 and USN13, which have no submarine.

## 32. The submarine's sensor category `00852B90` (packet `cc9_submarine_sensor_category`, rank 1 of section 31)

### 32.1 The image

`00852B90` is `__thiscall(unit+1E4h)`, so `ESI = unit` and `EDI = unit+1E4h`. docs/SENSOR_TABLES.md
"The submarine" has the prose. The listing (live, read only):

```
00852bab: FLD [ESI+100h] ; FLD [ESI+1204h] ; FADD [ESI+1200h] ; FDIV double [00D7A2B0]=3.0
00852bc3: FXCH ; FCOMIP ; JBE 00852bd4          ; y > (w0+w1)/3 -> 1 Surface  (quotient never stored)
00852be4: FLD [ESI+100h] ; FLD [EDI+1020h]=[unit+1204h] ; FSUB double [00D7A370]=5.0 ; FSTP float [ESP+8]
00852bfe: FCOMIP ; JBE 00852c4c                 ; w1-5 <= y -> 2 + ([unit+1234h] != 0): PeriscopeIn / PeriscopeOut
00852c14: FLD [ESI+100h] ; FLD [EDI+1028h] ; FADD [EDI+1024h] ; FMUL double [00D7A280]=0.5 ; FSTP float [ESP+8]
00852c34: FCOMIP ; JBE 00852c43                 ; (w2+w3)/2 <= y -> 4 Underwater, else 5 DeepUnderwater
```

- `unit+100h` is the hull frame's world Y. It is refreshed through `00414DB0` when `unit+C8h` is
  clear.
- The four words `+1200h..+120Ch` are the band table `00853630` builds at attach
  (SUBMARINE_MODEL, "The band table"). The dive binding (`cc9_submarine_dive`) seeds exactly these
  per boat, for example the Narwhal-class at (0, -10.2, -40, -80).
- The three constants were read as doubles from the image: 3.0, 5.0 and 0.5.
- An unordered compare (NaN) takes `JBE`.
- **Precision.** The surface limit stays on the x87 stack. The other two limits are rounded
  through a float slot.
  - The existing `sensor_category_submarine_00852b90` rounded the first limit to float too. It now
    compares in double, which is exact enough for a float Y.
  - The other two limits are unchanged: float subtraction and halving round like the image's
    stores.
- **The periscope byte `+1234h`** (`periscopeOut`, SUBMARINE_MODEL) has no producer in this process.
  It reads clear, so the periscope band answers PeriscopeIn. That is a labelled substitution.

**Consumers.** The recon pass looks the category up in the reconclasses tables
(`src/sensor_table_data.cpp`, SENSOR_TABLE_DATA), as the subject and as the observer.
- **DeepUnderwater has no row** in any table, for either role. A boat in that state is seen by no
  sensor, and it sees nothing.
- **Underwater as the subject:** for example arcade class 3's surface observer sees it at 660 m and
  1660 m. It sees PeriscopeIn at 1000 m and 2500 m.
- The AutoTarget takes its candidates from the recon enemy lists (`kAutoTargetReconCandidatesBound`).
  So the category decides who is shot at.

### 32.2 The binding (committed OFF)

- `GameUnitsHost::submarine_depth_bands` returns the slot's seeded bands.
- `GunneryReconSensorPassHost::unit_sensor_category` computes the image's state for every
  submarine, from the hull Y (`unit_position_00fc`) and those bands, and counts it.
- `kSubmarineSensorCategoryBound` (OFF) decides whether that state is returned. OFF keeps PeriscopeIn.
- A boat without seeded bands keeps PeriscopeIn and is counted `unseeded`.
- The summary line is `summary mission gunnery submarine sensor category calls= surface=
  periscope_in= periscope_out= underwater= deep= unseeded= differs= bound=`.

The OFF build is gameplay-identical to reference i on JM06 and LOMP06 (`pair_diff` exit 1,
`local\g7off_{jm06,lomp06}.log`).

### 32.3 OFF counters and predictions (written before the ON runs)

| row | calls | periscope_in | underwater | deep | differs |
| --- | --- | --- | --- | --- | --- |
| JM06 3200/3000 | 1074 | 453 | 468 | 153 | 621 |
| LOMP06 1200/1000 | 99 | 99 | 0 | 0 | 0 |

- **S1, the mechanism.** ON returns the counted state. JM06's first recon passes read the same
  states as OFF, and the totals move only after the tracks do. `differs` = underwater + deep on
  every row.
- **S2, LOMP06: gameplay identical** (exit 1). The Narwhal holds -10.2 m, and the periscope band
  (-15.2 m and above) covers it.
- **S3, JM06: exit 3.**
  - The Narwhal-class, at -80 m, is DeepUnderwater. Nothing detects it and it detects nothing.
    Its torpedo damage dealt (840 OFF) falls, to 0 if it has no other source of targets. It still
    takes 0.
  - The underwater Japanese boats are seen at the shorter Underwater ranges. PlayerSub 03's damage
    taken (562 OFF) and the Fletchers' hits (6 OFF) fall or hold; they do not rise.
  - Deaths stay at 1, the Gato wreck, give or take one.
- **S4, the missions without a submarine** (USN01, USN02, USN04, USN13): `calls=0`, and exit 1 on
  the summary line only.
- **The flip rule:** ON when S1, S2 and S4 hold and JM06's moves trace to the category through the
  recon lists. A JM06 move with the opposite sign to S3 is a stop.

### 32.4 The pairs, and the flip (2026-09-28)

- **OFF** is `pair_export --commit 93b8b8fc9` (SHA-256 prefix `F58B4755298D`).
- **ON** is the same commit with `--flip kSubmarineSensorCategoryBound=true` (`A1DE346A1F3B`).
- The logs are `local\sc_{off,on}_<row>.log`, and the run parameters are reference i's.

| row | `pair_diff` | ON counters (calls / periscope_in / underwater / deep / differs) | what moved |
| --- | --- | --- | --- |
| JM06 3200/3000 | **exit 1, gameplay identical** | 1074 / 453 / 468 / 153 / 621 | recon listener changes 1906 -> 4138 and fires 2 -> 1; `mission ship ai autotarget` mean candidates 4.38 -> 3.55; one dialog, one script entity and one timer fewer |
| LOMP06 1200/1000 | exit 1, gameplay identical | 99 / 99 / 0 / 0 / 0 | the summary line only |
| USN01 3200/3000 | exit 1, gameplay identical | 0 | the summary line only |
| USN02 9200/9000 | exit 1, gameplay identical | 0 | the summary line only |

**The lost listener fire.**
- OFF, the script's recon listener hears `"Narwhal-class Submarine 01" party 1 0 -> 2 ->
  luaJM6USNSubSighted()`, which starts a dialog and creates a script entity.
- ON, the Narwhal-class sits at -80 m and reads DeepUnderwater. No table row sees it, party 1 never
  detects it, and that listener never fires.
- The fleet listener (`"US Tanker 01" ... luaJM6FleetSpotted()`) fires on both sides.

**Against the predictions.**
- **S1 held.** The ON counters equal OFF's exactly, and `differs` = underwater + deep.
- **S2 and S4 held.**
- **S3 missed on its gameplay half.** JM06 is exit 1, not 3, and the Narwhal-class still deals 840.
  - Its torpedo targets do not come from its own sight. Nothing was shooting at it OFF either
    (taken 0).
  - The Fletchers' hits and PlayerSub 03's damage taken are unchanged, so the Underwater boats'
    shorter detection range moves no shot within 3000 frames.
  - The category's reach on JM06 in this window is the recon layer and the script's sighting event.
- **No move has the opposite sign to S3.**

**Verdict: ON.** The mechanism matches the image, and every move traces to the category through the
recon lists. `kSubmarineSensorCategoryBound = true`.

**Still open.**
- **The periscope byte `+1234h`** has no producer. A boat with its periscope raised would read
  PeriscopeOut. PeriscopeOut has its own rows in the tables, for example as an observer at 4000 m
  against the surface (arcade class 7). The Lua property `periscopeOut` is the writer to find
  (SUBMARINE_MODEL).
- **JM06's reference row does not move.** Its script flow does: the sub-sighted dialog is gone.
  Reference j should note it.

## 33. The unresolved fire target `00835930` (packet `cc9_unresolved_fire_target`, rank 2 of section 31)

### 33.1 The image, and what the host dropped

`008358D0` (`BSP_WeaponDirector_SetCommand`, director vtable `+60h`) pushes the slot through
`0071E6C0`. When the command's category is 1 or 2 and the session mode is 0 or 1, it resolves the
descriptor (`00521EA0`) and calls `00835860` at `00835930` with that entity and force 1. The fire
target becomes the commanded entity.

The host's `set_fire_target` accepted only a pointer to one of the commands host's own unit records.
**Every one of the 154 unresolved calls on reference i carries a handle instead**: the pointer field
holds the entity's object id. A diagnostic run of this tree (`local\g7ft_<row>.log`) traced the
first 12 per mission. On each of them the pointer value equals the descriptor's object id (`+2h`), and
that id names a live unit:

| row | unresolved | by object id | kinds (traced) |
| --- | --- | --- | --- |
| USN02 9000 | 14 | 14 | ship `attackmove`: Haguro and Murasame on DeRuyter, Jintsu on Java, Yamakaze on Alden, Minegumo on Houston, Tokitsukaze on Exeter, ... |
| USN04 4500 | 35 | 35 | plane `divebomb` and `torpedo` on Lexington and Yorktown |
| USN13 3000 | 60 | 60 | plane `torpedo` on Monterey (the first 12) |
| USN01 3000 | 7 | 7 | Mav1-5 `torpedo` on Dunlap, Northampton and SaltLakeCity; ScoutDauntless `divebomb` on Convoy1 |
| JM06 3000 | 2 | 2 | PlayerSub 02 `attackmove` on US Tanker 01, PlayerSub 03 on US Cargo Transport 02 |
| LOMP06 1000 | 1 | 1 | Yugiri `attackmove` on Narwhal |

The producers are the script orders, the ship-AI planner and the plane orders. They use the object
id or index+1 as the opaque entity handle (for example `game_hosts_script_orders.cpp`,
`game_hosts_ship_ai.cpp`). The commands host itself already resolves every other descriptor by the
object id (`resolve_target_00521ea0`).

### 33.2 The binding (committed OFF)

- `kFireTargetObjectIdBound` (in `game_hosts_commands.cpp`) resolves such a handle by the
  descriptor's object id and routes the fire target like a resolved one. That means one
  `fire_target_requests` entry, consumed by the ship-AI host's `store_fire_target`
  (`kWeaponDirectorFireTargetBound`).
- It is a labelled substitution: the handle stands in for the image's entity pointer.
- OFF keeps the drop.
- The summary line is `summary mission director fire target unresolved= by_object_id=`. The first
  12 per run are traced as `fire target unresolved (00835930): ...`.
- The ship-AI consumer skips `generated_non_ship` controllers, which are units created after load
  that are not ships. That covers USN04's and USN13's launched squadrons.

### 33.3 Predictions (written before the ON runs)

- **F1, the mechanism.** ON, `unresolved` and `by_object_id` equal OFF's until the tracks diverge.
  `ship ai director fire target command_requests` rises by the calls whose unit has a ship
  controller.
- **F2, USN02: exit 3.**
  - The 14 Japanese attack-move ships take their commanded targets as fire targets.
  - Death rows move.
  - The mission still fails in phase 1, torpedo-driven, within 10 s of 29.75 s.
- **F3, JM06 and LOMP06: exit 3, small.** The two PlayerSubs and Yugiri fire at their commanded
  targets.
- **F4, USN04 and USN13: exit 1, gameplay identical.** Their requests come from launched
  squadrons, which the consumer skips.
- **F5, USN01: exit 1.** The Mavs and the ScoutDauntless are planes. I expect a fire target stored on
  a plane's row not to reach its gunnery. That is unverified, and it is what this row tests.
- **The flip rule:** ON when F1 holds and every move traces to a forced fire target.

### 33.4 The pairs, and the flip (2026-09-28)

- **OFF** is `pair_export --commit 296e445b4` (SHA-256 prefix `CD98253065FC`).
- **ON** is the same commit with `--flip kFireTargetObjectIdBound=true` (`D1A4A1DC3B3C`).
- The logs are `local\ft_{off,on}_<row>.log`, and the run parameters are reference i's.

| row | `pair_diff` | unresolved (OFF = ON) | ship-AI command requests / changes, OFF -> ON | what moved |
| --- | --- | --- | --- | --- |
| USN02 9000 | exit 1, gameplay identical | 14 | 1218 / 51 -> 1232 / 51 | nothing: each forced target was already that ship's fire target |
| JM06 3000 | **exit 3** | 2 | 337 / 20 -> 339 / 22 | PlayerSub 02 fires on US Tanker 01 and PlayerSub 03 on US Cargo Transport 02: hit records 127 -> 122, damage 2720.7 -> 2468.6, Tanker 01 taken 502 -> 368; the same death row |
| LOMP06 1000 | exit 1, gameplay identical | 1 | 14 / 2 -> 15 / 3 | Yugiri takes the Narwhal as its fire target (torpedo candidates' fire target 920 -> 1302); no shot lands |
| USN04 4500 | exit 1, gameplay identical | 35 | 0 / 0 -> 3 / 3 | three requests reach load-time plane rows (the AutoTarget accepts the locked target 665 times and scans 665 fewer) |
| USN13 3000 | exit 1, gameplay identical | 60 | 0 -> 0 | every request comes from a launched squadron and is skipped |
| USN01 3000 | exit 1, gameplay identical | 7 | 0 / 0 -> 5 / 5 | the Mavs' rows take their torpedo targets; no gameplay moves |

**Against the predictions.**
- **F1 held.** `unresolved` and `by_object_id` are equal OFF and ON on every row. The ship-AI
  requests rise by the calls that reach a ship-AI row.
- **F2 missed.** USN02 is identical: the planner's attack-move targets were already each ship's fire
  target, so all 14 stores change nothing. The 29.75 s failure is unchanged.
- **F3 held on JM06** (exit 3, small, from the two forced targets). **It missed on LOMP06**, which is
  identical: Yugiri's target changes, but no shot lands within 1000 frames.
- **F4 held on USN13.** On USN04 three requests reached load-time plane rows, which F4 did not
  expect, and gameplay is still identical.
- **F5 held.**
- Every move traces to a forced fire target.

**Verdict: ON.** `kFireTargetObjectIdBound = true`.

**Still open.** Plane rows hold the stored target, and their AutoTarget accepts it (`accept.true`).
Whether the image's plane gunnery reads the director fire target is not read here. The plane
packets own that.

## 34. The convoy detection fold `00805680` (packet `cc9_convoy_detection_fold`, rank 6 of section 31, the convoy half)

### 34.1 The image

`00805680` is `__thiscall(slot, List* B19h, List* B1Ah)`, RET 8, body `00805680-00805864` (live
decompile). It is the land twin of `00805490`, and `008073C0` calls it after the seven `00805490`
calls, once per relation (`src/recon_slot_lists.cpp`). For each member record in `B[19h]` whose level
(`+0Ch`) is above 0 and whose unit carries a convoy at `+738h`:
1. **It finds the convoy's group record** in this order:
   - the carry-over list `slot+F4Ch`, where the level is reset to 0 and the record is spliced out;
   - otherwise `B[1Ah]`;
   - otherwise a new `1Ch` record. That record gets vtable `00D08E78`, `+4h` the convoy, `+8h` the
     convoy's sensor category (`[convoy+1E4h]->vtable[1]()`) and `+0Ch` level 0. It is appended to
     `B[1Ah]`, and an observer pair is registered.
2. **It appends the member** to the group's member list (`+10h`/`+14h`/`+18h`).
3. **It sets the group level** to the maximum of the members' levels.

The members stay in `B[19h]`. The group record's entity is the `LandConvoy` (class `1Ah`).

### 34.2 What the host can hold

- lua5's convoy roster (`974282ec7`, docs/LAND_AND_STRUCTURES.md) gives each member its convoy.
  `GameUnitsHost::unit_land_convoy_738` returns it by name.
- **The `LandConvoy` itself is not a unit here.** JM05 logs it as a scene marker (for example
  `SecondaryLandConvoy 01 class=LandConvoy id=50009`). LAND_AND_STRUCTURES records `004F2700` as
  not a unit creator.
- The recon triples hold unit indices. So a group record has no slot, like section 22's squadron
  group-level publish.
- JM05's script never queries a convoy's detection. It only kills and drives the convoy
  (`jm05.lua` 2656 and 2839, `commandhelpers.lua` 12380-12448).

### 34.3 The fold, counted (no switch; nothing is placed)

The recon pass's `group` step now runs `00805680`'s fold over `B[19h]` by
`unit_land_convoy_738` and counts the groups by relation and level. The summary line is
`summary mission recon convoy fold members= own_groups= enemy_blip= enemy_identified=
neutral_blip= neutral_identified= placed=0`. It is gameplay-identical to the tree before it
(`local\g7cv_jm05.log` against `local\g7cf_jm05.log`, `pair_diff` exit 1).

| row | members folded | own groups | enemy groups (blip / identified) | neutral groups |
| --- | --- | --- | --- | --- |
| JM05 3200/3000 | 255 | 102 (two convoys, level 2, 51 passes) | 0 / 0 | 0 / 0 |
| USN01 3200/3000 | 0 | 0 | 0 / 0 | 0 / 0 |
| USN04 4700/4500 | 0 | 0 | 0 / 0 | 0 / 0 |

**Which levels would change on JM05:** none that a target pick reads.
- The Japanese side never detects a convoy member within 3000 frames. Its 510 member records all sit
  at level 0, so it builds no convoy group.
- The US side builds its two own convoys at level 2. They would join triple 0, its own list, which
  no AutoTarget reads.

**Verdict: stays a record.** There is nothing to pair: placing the groups needs a `LandConvoy` entry
in the units host's index space, which is the units host's work. With one, JM05 is still predicted
identity for targeting within 3000 frames. A longer JM05 run, where the Japanese side spots a
convoy, is where the group level would first matter.

## 35. The attack-move arm on a command building, `00836B95` (packet `cc9_attackmove_building_arm`, rank 3 of section 31)

**The image** (`00836B45`, the director step's attack-move arm, live listing):

```
00836b76: PUSH 0x1c ; CALL [vtable+5Ch]        ; the target is a command building (kind 1Ch)
00836b7e: JZ 00836bc0                          ;   no: the live and hostile tests
00836b80: CMP byte [EDI+5Eh],0 ; JNZ 00836b95  ; +5Eh set: convert
00836b86: MOV EAX,[ESI+34h] ; MOV ECX,[EDI+54h]
00836b8c: CMP ECX,[EAX+54h] ; JNZ 00836d67     ; another party's building: nothing, the attack-move stays
00836b95: 00465080(building, 0.0) ; 0071ECF0(moveto 00E08F68) ; 00836bb2: 0071D810(2)
```

The conversion to `moveto` and the stage-2 raise happen only when the building's `+5Eh` is set
(the host reads it as the scene node's destroyed flag, `game_hosts_ship_ai.cpp` `flag_05e`), or when
the building's party equals the unit's. That is a building its own side holds.

**The host** recorded every building target on this arm, and it labelled the arm unreached. Both
counts were wrong.
- The arm is reached: USN13 1002 times, USN01 8.
- A diagnostic now counts the image's test: `summary mission director attackmove building arm hits=
  converts=`. The first 12 conversions are traced.

| row (this tree, `local\g7ba_<row>.log`) | hits | converts |
| --- | --- | --- |
| USN13 3200/3000 | 1002 | 0 |
| USN01 3200/3000 | 8 | 0 |
| LOMP07 3200/3000 | 0 | 0 |

**Verdict: exact on these missions; nothing is bound.** Every hit is another party's building that is
not destroyed. The image's `JNZ 00836D67` keeps the attack-move, which is what the host's record
does. The label is corrected in the source.

**Still open.** The conversion itself (`00465080`, `0071ECF0 moveto`, `0071D810(2)`) is not issued.
It would take the idle tail's route through the stage binding and the queued delivery (section 27).
Bind it when a mission shows `converts` above 0, which needs a Capture that completes, or a building
destroyed under an attack-move.

## 36. The director slot housekeeping in `00720850` (packet `cc9_director_slot_housekeeping`, rank 4 of section 31)

Since the clear-all landed (sections 28-29), the slot clear `00720850`
(`src/command_execution.cpp` `clear_command_slot_00720850`) runs on every occupied slot. Its four
host stand-ins went from 86 calls on h to thousands. Each was read for what its answer can change.

| stand-in | image | host | effect |
| --- | --- | --- | --- |
| `006952A0` observer unregister | the (target, director) edge is **reference-counted**. `00694A60` creates it or adds 1 at `+0Ch`. `006952A0` clears pending dispatches for the pair, subtracts 1, and at 0 removes the edge (docs/OBSERVER_EDGES.md, OBSERVER_LIFETIME.md) | record | none. Every push registers, and only a head clear unregisters, so the count is at least the number of live slots holding the target. `release_observed_target_0071ddb0` scans every director's live slots, so it delivers to exactly the directors whose edge is still live. The host delivers synchronously, so no pending dispatch can exist |
| `00414DB0` target pose refresh in `snap_to_target` | refreshes the entity's cached world pose (`+FCh`) when `+C8h` is clear | record | none: the host's unit positions are always current |
| `0071FB90` path object for the vacated slot | a new empty path object at `director+1A4h+9*4` | 0 | see below |
| `007208A3` session trace value | the clear's trace argument | 0 | presentation only |

**Path objects.**
- The host's only reader of `path_objects` is `command_slot_has_active_order`, and it has no caller.
- The slot-0 path lives in `director.path_points`.
- Where a path object would weigh is `0071D780`, the queue-full test (`CMP EAX,0xA` at `0071E6C8`). A
  queued `moveonpath` in slots 1-9 counts as 1 here, where the image counts its path's points. That
  is `command_count`'s named hole.
- A diagnostic counts the tests with such a slot (`summary mission director queue full tests=
  with_queued_moveonpath=`):

| row (`local\g7hk_<row>.log`) | queue-full tests | with a queued `moveonpath` |
| --- | --- | --- |
| BSM01 3200/3000 | 491 | 0 |
| JM06 3200/3000 | 717 | 0 |
| USN04 4700/4500 | 1061 | 4 |

- **USN04's four** are all Lexington-class01, with `moveto` at the head and `moveonpath` in slot 1:
  weighted 2 here.
- The carrier paths have 6 to 8 points (`CarrierPath1..4`). So the image's weight is at most
  1 + 8 = 9, still below 10, and both refuse nothing.
- BSM01's one refusal (weighted 16) comes from its slot-0 path, which the host does weigh.

**Verdict: exact in effect on these missions; nothing is bound.**

**Still open.**
- **The queue-full hole.** It matters only where a queued `moveonpath` of 9 or more points meets a
  head. The diagnostic traces the first eight tests that have a queued `moveonpath`, with their
  weights.
- **A latent defect.** `queue_state_back` keeps each slot's old `object` pointer while taking the
  shifted slot's parameters. Nothing reads `slot_target[].object` today (resolution is by the object
  id), so it has no effect. It should be fixed before anything does read it.

## 37. Handoff (cc9-gunnery7, 2026-09-28, at about 76% context), with the OverrideHP read

### 37.1 `cc9_override_hp`: OverrideHP `008C1930`, the gunnery side committed OFF (`b854e8ee3`)

**The image** (live listing):
- Argument 0 is resolved through `00888AA0` into `ESI`, the unit.
- Argument 1 is read as a number. `008C1A71 FSTP [ESI+36Ch]` stores it as the maximum health
  (`+370h` over `+36Ch` is the health fraction, AI_TARGET_WEIGHT_TERMS).
- `008C1AA8` calls `00877B90(unit, same number)`, the health setter.
- There is no difficulty term. LOMP10's `usn\LOMP\10_san_jose.lua` calls
  `OverrideHP(unit, unit.Class.HP * 1.0 / 1.25 / 1.5)` by `Mission.Difficulty` on the eight
  `Mission.SanJoseForce` ships. Our runs are difficulty 1, so the factor is 1.25.
- This host applies the party HPMultipliers to the damage (`0087D730`, DIFFICULTY_MULTIPLIERS), not
  to the maximum, so the two stack as in the image.
- The invincibility floor acts only in `00879070`, which this write does not use.
- **Units:** `max_health` is the class `hp` in thousandths divided by 1000 (`flat_scaled`), which is
  the same scale as the script's `Class.HP`.

**The gunnery side.**
- `GameGunneryHost::override_hp_008c1930(index, value)` sets `max_health`, then
  `set_health_00877b90(value)`.
- `kLuaOverrideHpBound` (OFF) gates the write. It is labelled: a unit this host already killed is
  not written.
- Calls are counted and the first 12 traced (`  OverrideHP 008c1930: ...`). The summary line is
  `summary mission gunnery override hp calls= applied= bound=`.

**What remains, in order:**
1. The Lua dispatch in lua6's file, routed through the lead:
   `local\g7_override_hp_dispatch.txt` in the cc9-gunnery7 tree has the exact lines. It is identity
   while the switch is OFF.
2. After it lands, run LOMP10 3200/3000 OFF. The trace gives each ship's value, `max_before` and
   `health_before`.
3. Write the predictions: each ship's new maximum (Class.HP x 1.25), whether any San Jose ship
   sinks or dies later within 3000 frames, and USN04 identity (no call).
4. Pair LOMP10 and USN04 with `pair_export --flip kLuaOverrideHpBound=true`, and flip by verdict.

**OFF measurement** (`local\g7hp_lomp10.log`, main `400e76a72` with lua6's dispatch `3786d5641`):
eight calls, each at full health, and each value exactly 1.25 x the maximum.

| ship | value | maximum and health before |
| --- | --- | --- |
| Ashigara | 8750 | 7000 |
| Oyodo | 7500 | 6000 |
| Kiyoshimo, Asashimo | 3750 | 3000 |
| Sugi, Kashi, Kaya | 3125 | 2500 |
| Kasumi | 3500 | 2800 |

None of the eight takes damage within 3000 frames OFF (taken 0, not sunk). The run has 10 deaths.

**Predictions (written before the ON runs).**
- **H1:** ON applies all eight (`applied=8`). Each ship's maximum and health read the value.
- **H2, LOMP10:** only the eight health cells of the unit table move. Deaths, hit records, shots and
  death rows are identical, because nothing damages these ships in the window. The health
  fraction the AI reads (`+370h` / `+36Ch`) stays 1.0.
- **H3, USN04:** `calls=0`, exit 1.

**The pair commands:**
```
python tools/pair_export.py --commit <this commit> --out local\hp_off
python tools/pair_export.py --commit <this commit> --flip kLuaOverrideHpBound=true --out local\hp_on
./local/g7_pair.ps1 -Off hp_off -On hp_on -Rows 'lomp10:LOMP10:3200:3000','usn04:USN04:4700:4500'
```
**The pairs.**
- OFF is `pair_export --commit a0c75d464` (SHA-256 prefix `1CB0651B8AD4`).
- ON is the same commit with `--flip kLuaOverrideHpBound=true` (`6C9669205847`).
- The logs are `local\hp_{off,on}_<row>.log`.

| row | `pair_diff` | what moved |
| --- | --- | --- |
| LOMP10 3200/3000 | exit 3 | `applied` 0 -> 8. The unit table's health moves on exactly the eight ships (Ashigara 7000 -> 8750, Oyodo 6000 -> 7500, Kiyoshimo and Asashimo 3000 -> 3750, Sugi, Kashi and Kaya 2500 -> 3125, Kasumi 2800 -> 3500). The 10 death rows, hit records and shots are identical |
| USN04 4700/4500 | exit 1 | the summary line only (`calls=0`) |

- **H1, H2 and H3 held.**
- Beside them, two summary counters moved by a hair: the minimap heading 0.4535 -> 0.4536 rad, and
  the gunnery landscape attach cells 46835 -> 46837. They are recorded, not attributed.

**Verdict: ON.** `kLuaOverrideHpBound = true`. The effect beyond 3000 frames is the San Jose ships'
larger health pools.
### 37.2 What remains of section 31

| rank | item | state |
| --- | --- | --- |
| 1 | submarine sensor category | ON (section 32) |
| 2 | forced fire target handle | ON (section 33) |
| 3 | building attack-move arm | exact; conversion unissued until a row shows `converts` > 0 (section 35) |
| 4 | director slot housekeeping | exact in effect (section 36) |
| 5 | command-allowed extra tests `009229F0` / `007AC9D0` (268 calls) | read and measured: the image allows every call on reference j's rows, as the host does; recorded, not bound (section 39) |
| 6 | recon convoy and group records | the convoy fold is counted, not placed (section 34); the squadron group-level publish stays blocked (section 22) |
| 7 | `007788B0` controller ownership | **read in section 44**: it is the formation-follower gate; the binding is the ship-AI lane's |
| 8 | hull roll torque `00827312` (USN02, 52 calls) | **open, unread**; reach 2 |

### 37.3 Open items found on the way

- **The periscope byte `+1234h`** (`periscopeOut`) has no producer, so a raised periscope never
  reads PeriscopeOut (section 32.4; listed in section 1). **Closed by section 38** (`kSubmarinePeriscopeOutBound` ON).
- **Plane rows and the forced fire target.** Since `kFireTargetObjectIdBound`, plane rows store the
  commanded target and their AutoTarget accepts it (USN04 3, USN01 5). Whether the image's plane
  gunnery reads the director fire target is unread (section 33.4). That belongs to the plane packets.
  **Closed by section 42:** no plane-side pass reads one (`kPlaneNullFireTargetProviderBound` ON).
- **A LandConvoy unit.** `00805680`'s group records need a `LandConvoy` entry in the units host's
  index space (section 34). That belongs to the units lane. With one, JM05 is still predicted
  identity within 3000 frames.
- **A latent defect in `queue_state_back`.** Each slot keeps its old `slot_target[].object` pointer
  while taking the shifted slot's parameters (section 36). Nothing reads that pointer today.
  Resolution goes by the object id. Fix it before anything reads it.
- **The queue-full path-object hole.** A queued `moveonpath` counts as 1, not its point count. It
  is traced by `queue full test with a queued moveonpath` (section 36). The reference rows never
  reach the refusal.

### 37.4 Tools in the cc9-gunnery7 tree (`local\`)

- `g7_rank.py` and `g7_rankdiff.py`: section 31's census, and the h-against-i diff.
- `g7_standins.py`: what each stand-in answers.
- `g7_rows.py`: reference headline rows, h against i.
- `g7_pair.ps1 -Off <export> -On <export> -Rows 'tag:MISSION:frames:mission_frames'`: runs
  both sides at once, waits in the foreground, and prints the pair_diff.
- `g7_var.ps1`: leave-one-out variants against `rb9_<row>.log`.

## 38. The periscope byte `+1234h` (packet `cc9_periscope_out`, cc9-gunnery8, 2026-09-28)

The open item from sections 32.4 and 37.3: nothing in this process wrote `periscopeOut`, so a raised
periscope never read PeriscopeOut.

### 38.1 The image

- **One writer.** `periscopeOut` is a reflection property (`00D0BEC8`, bound at `00853FE9`; the
  Lua name is a property reader, not a script call). No mission script in this installation names
  it. Its only writer is `00854650 BSP_SubmarineUnit_Update(unit, dt)`, the per-frame update
  (SUBMARINE_MODEL "The periscope").
- **The clear.** `00854AF8` loads the `periszkop` node `+1214h`, and `00854B00` stores 0 to
  `+1234h` before the null test at `00854B06`. A boat without the node (a kamikaze class,
  `00853630`) skips the whole arm with the byte clear.
- **The mast.** From `00854F52` (read in the listing with `disasm-raw`):
  - `periscopeState` (`+122Ch`) == 1 takes the extending arm. The mast node's local Y (from
    `00B6DB60`, `+30h..+38h`) steps toward `[class+81Ch] + periscopeY` by `dt * 5.0` (`00D7A370`)
    through `0042AC60 BSP_Math_StepTowards(this=&Y, target, step)`. The node takes the new pose
    through `vtable[2Ch]`.
  - Then `00855039..00855057`: when the new Y is at least `[class+81Ch] + periscopeY - 1.0`
    (`00D7A210`), `+1234h` becomes 1.
  - Any other state takes `00855063`, which steps back toward `periscopeY` at `dt * 3.0` and never
    sets the byte.
- **The rest position.** `periscopeY` (`+1230h`) is the node's own local Y at attach (`00853CB4`
  `FLD [EAX+34h]`, `00853CB9` `FSTP [ESI+1230h]`), with `+122Ch` = 0 (`00853CA9`). The mast
  therefore starts at rest and the byte is out `(range - 1.0) / 5.0` seconds after a raise.
- **The range** is `PeriscopeMoveRange`, `class+81Ch`, NumberOr with 10.0 (`00CE38B8`). This
  installation's `vehicleclasses.lua` (mtime 2026-05-09) authors values from 0 to 7.
- **Who raises.** Only the ship AI's `009E4D90` (fire state, at periscope depth, with a
  non-submarine target) stores 1. `009EA8FB` stores 0. The auto-raise at `00854ED7` needs
  `+1235h`, which only the reflection setters `00464320` / `004649B0` write.

### 38.2 The binding (committed OFF)

`kSubmarinePeriscopeOutBound` in `include/bsp/game_hosts_units.hpp`:
- The ship AI host's `set_periscope_state_122c` mirrors each store onto the seeded submarine's units
  slot, with the node test (`has_periscope_1214`) and the class's `PeriscopeMoveRange` (default 10).
  The mirror runs on both sides and logs `submarine periscope state:`.
- ON, the units host runs the arm once per force step after the air step:
  `run_submarine_periscope_00854650`. The gunnery host's `00852B90` reads the byte through
  `submarine_periscope_out_1234`. OFF, the byte reads false, as before.
- **SUBSTITUTIONS, labelled:** it runs in the force step, not in the unit update. The mast is held
  as its offset above `periscopeY`, so `periscopeY` is 0 in the threshold. The node pose write and
  the `+1210h` shape move (`00C357B0`) are not made. The repair and the auto-raise have no
  producer here.

### 38.3 Predictions (written before the ON runs)

The pairs are this tree's build (OFF) against `pair_export --flip kSubmarinePeriscopeOutBound=true`,
with reference j's run parameters. On reference j, JM06 is the only row with a submarine in
sub_attack. PlayerSub 02 and 03 each raise once, in the fire state (first fire 102.85 and 99.35
s). The Narwhal-class never raises, since its target is a submarine.

- **P1, the mechanism (JM06).** The `submarine periscope state:` lines are the same on both sides:
  one `state=1` each for PlayerSub 02 and 03, and none for the Narwhal-class. ON, each PlayerSub
  prints `submarine periscope out: ... out=1` at most `(range - 1.0) / 5.0` s after its raise, and
  at most 1.8 s after it. OFF prints none.
- **P2, the sensor (JM06).** The recon summary's `periscope_out` rises from 0, and `periscope_in`
  falls by the same count. `calls`, `underwater` and `deep` are unchanged, because the byte only
  splits the PeriscopeIn state.
- **P3, gameplay (JM06).** Nothing moves before the first `out=1` line, about 100 s in. After it,
  the PlayerSubs see and are seen by the table's PeriscopeOut rows. **exit 1 or a small exit 3**,
  confined to the last 50 s.
- **P4, LOMP06 1200/1000.** No raise (the Narwhal is displaced by the seed, reference j), so no
  state line and **exit 0**.
- The verdict rule: P1 and P2 are the mechanism. A P3 spread miss with P1 and P2 held may flip.

### 38.4 The pairs, and the flip (2026-09-28)

- **OFF** is this tree's build of `da969c5b2`. Its JM06 and LOMP06 logs are gameplay-identical to
  reference j's.
- **ON** is `pair_export --commit da969c5b2 --flip kSubmarinePeriscopeOutBound=true` (SHA-256
  prefix `57EA8E120964`).
- The logs are `local\g8_poff_<row>.log` and `local\g8_pon_<row>.log` in worktree cc9-gunnery8,
  with reference j's run parameters.

| row | `pair_diff` | what moved | against the prediction |
| --- | --- | --- | --- |
| JM06 3200/3000 | **exit 1, gameplay identical** | PlayerSub 03 raises at 116.10 s and reads out at 116.85 s; PlayerSub 02 raises at 119.60 s and reads out at 120.35 s (range 4.80, mast 4.000 against the 3.80 threshold). The recon summary goes `periscope_in` 360 -> 306 and `periscope_out` 0 -> 54, with `calls` 1074, `underwater` 612 and `deep` 102 unchanged. The recon pass shows `blip` 0 -> 7 and `identified` 874 -> 867; gunnery candidates fall 2645 -> 2601 | P1, P2 and P3 held |
| LOMP06 1200/1000 | exit 1 | no state line and no arm call; only the movie-camera pose lines differ, by their 0.01 s clock | P4 predicted exit 0; the difference is the known presentation noise |

- **P1 held.** The raises come at 116.10 and 119.60 s, not at the first fire step. `009E4D90`
  needs periscope depth as well as the fire state. Each byte reads out 0.75 s after its raise,
  inside the 0.76 s bound.
- **P2 held** exactly: the 54 out samples are all taken from PeriscopeIn.
- **P3 held.** No gameplay line moves within 3000 frames. The byte reaches the recon layer, which
  now reports seven blips of the raised boats.
- **P4 missed only on noise.** LOMP06 runs no arm, and the one differing kind of line is the
  presentation noise already known on LOMP10.

**Verdict: ON.** The mechanism matches the listing on both rows. `kSubmarinePeriscopeOutBound = true`.

**Still open.**
- The repair (`00854E44`) and the break (`009373C0`) have no producer in this process.
- The auto-raise's `+1235h` is written only by the reflection setters. No reference row sets it.
- A player-raised periscope (the periscope GUI page) is not routed.

## 39. The command-allowed extra tests `009229F0` / `007AC9D0` (packet `cc9_command_allowed_extra`, rank 5 of section 31)

Read and measured; **recorded, not bound.** The image's answer equals the host's on every call in
reference j's twelve rows.

### 39.1 The image

`0071D6D0 BSP_WeaponDirector_CommandAcceptsTarget(command, descriptor)`, read in the listing:
- **The refusal block, `0071D6D5..0071D71C`.** When `command->vtable[8]()` (requires a target) is
  true and either the descriptor's `+1h` byte is clear or `vtable[0Ch]()` (the category) is 1 or 2,
  the target must resolve through `00521EA0` and must not carry `+5Dh`. Otherwise the answer is
  false.
- **The extra tests, `0071D71F..0071D75B`.** Every path that is not refused reaches them, including
  a command that requires no target:
  - `torpedo` (`00E08F18`): `009229F0(resolve(descriptor), 6)` must be true. That is the class
    test with the LandFort `FakedType` fallback (`include/bsp/attack_target_classify.hpp`). It is
    false for an unresolved descriptor.
  - `moveonpath` (`00E08F80`): only when the descriptor's `+0h` kind byte is set,
    `007AC9D0(resolve(descriptor))` must be non-null. That is the path interface of a `Path`,
    kind `48h`, kind `49h` or `CameraPath` (`docs/ENTITY_COMMAND_ARMS.md`).
  - `00E08F78` resolves the descriptor and ignores the answer.
- **The host** (`command_allowed` in `src/game_hosts_commands.cpp`) returns true before the extra
  tests when no target is required or on the position branch, and it never runs them. It records
  `WeaponDirector::command_allowed_extra_test`.

### 39.2 The measurement

This was a tree-local diagnostic, not committed: it logged each distinct call shape (unit,
command, descriptor kind, `+1h`, requires-target, category, and the unit `00521EA0` resolves).
It ran on this tree's build (reference j plus the periscope flip) over all twelve reference j
rows. The logs are `local\g8_x_<row>.log` in worktree cc9-gunnery8, and the edit is
`local\g8_edit_extra1.py`.

| shape | rows | the image's answer |
| --- | --- | --- |
| `moveonpath`, kind 0 (a position) | E2, JM06, JM08, LOMP06, USN02, USN04, USN12, USN13 | the test is skipped (`0071D749`): allowed |
| `moveonpath`, kind 1, a scene marker of class `Path` (ids 50000 and up) | BSM01, E2, JM05, JM06, LOMP06, USN04 | the path interface at `+1E4h`: allowed |
| `torpedo`, kind 1, resolving to a unit of class `MMothership` (09h), `MCruiser` (0Ah) or `MDestroyer` (07h) | E2, JM05, USN01, USN04, USN13 | all three descend from 06h, so `009229F0` is true: allowed |

- Every `moveonpath` object target on these rows is a `Path` marker. Each id was checked against its
  own log's `scene marker` list.
- No torpedo is issued at a position, at an unresolved object, or at a non-ship. No
  `moveonpath` names a unit.
- LOMP10 makes no call.

**The verdict: no binding.** The host's "allowed without the extra test" is the image's answer for
every call these rows make. A binding would change no answer, so nothing is committed.

**What would make it differ, if a later row shows it:**
- a `torpedo` aimed at a position, at an unresolved descriptor, or at a non-ship unit;
- a `moveonpath` aimed at a unit, or at a marker that is not one of the four path kinds.

The commands host holds no class ids. A binding would need the units host's `unit_is_kind_of` and
the scene markers' classes, and a way to tell the markers from the host's unit handles.

## 40. The untouchable gate `00862440` (packet `cc9_untouchable_gate`, the gunnery half of lua7's natives rank 1)

### 40.1 The image

- **The gate.** `00862440 BSP_Entity_UnitAiSuppressesGunnery`, `__thiscall(entity)`, body
  `00862440-00862471`, RET 0. It calls `entity->vtable[140h]()` twice (`0086244B`, `0086245B`). It
  answers 1 when the result is non-null and its byte `+1D4h` is set (`0086245D`), and 0 otherwise.
  The proxy is the entity itself for a ship (`0047F320` is `mov eax, ecx`), `[plane+9D4h]` for a
  plane, and `[fort+738h]` or the fort for a land fort (`include/bsp/gunnery_tables.hpp`).
- **Its only caller** is `00865248` in `00864FE0 BSP_UnitGunneryAi_Tick`. A `rel32` scan of
  `.text` finds that CALL and nothing else, and no absolute dword names `00862440`.
  - The site walks the unit's recon list (`[..+DE8h]`, next at `+8h`, the entity at `+4h`).
  - `00863990` scores each candidate. If it accepts (`0086523C`), the gate runs. A true gate jumps
    to `0086542F`, the next candidate, before the visibility test `00864D90` (`0086525D`) and the
    pick.
  - **What it blocks:** only the unit gunnery AI's own choice of a target for its gun categories.
    It is not in the ship AI's target choice, the attack-move or ramming paths, or a commanded fire
    target (`00835860`).
- **The writer.** The Lua native AddUntouchableUnit `008AC140` writes the byte through the same
  `vtable[140h]` (`008AC263`), as lua7 read it (LUA_BINDING_MISSION). The flag therefore sits on
  the unit's proxy, which is where the gate reads it.

### 40.2 The binding (committed OFF)

`kAiUntouchableGateBound` in `src/game_hosts_gunnery.cpp`. The gunnery host's
`unit_ai_suppresses_00862440` reads `bsp::game::lua_unit_untouchable_1d4(index)` for the
candidate's units-host index. It counts the reads, and the reads a set byte would suppress, on
both sides. The summary line is `summary mission gunnery untouchable gate reads=... marked=...
bound=...`, and the first six marked reads are traced.
- **LABELLED:** the Lua host keys the flag by the unit the script passed. That unit is the proxy
  for a ship. A plane's proxy `[plane+9D4h]` is not followed.

### 40.3 OFF counters and predictions (written before the ON runs)

The OFF side is this tree's build, with reference j's run parameters plus main's later landings.

| row | OFF | prediction |
| --- | --- | --- |
| JM05 3200/3000 | AddUntouchableUnit marks units 339, 340 and 364 at t=0.00. The gate reads 2520 candidates, **none of them marked**. PT Boat 80' Elco 01 and 02 are on side 2, with no enemy nearest and no shots at them. Event2Pt is on side 0, with its nearest enemy 6829 m away, beyond every gun range | **exit 1, gameplay identical.** The summary line changes only its `bound`. No attack on the PT boats exists to disappear in this window, and the death rows are unchanged |
| USN04 4700/4500 | no mark (`calls=0`); the gate reads candidates, none marked | exit 1, the summary line's `bound` only |

- **U1, the mechanism:** `marked` is the same on both sides of each row. ON suppresses exactly the
  marked reads.
- **U2:** no gameplay line moves on either row.
- The verdict rule: U1 is the mechanism. The flip follows U1 and U2.

### 40.4 The pairs, and the flip (2026-09-28)

- **OFF** is this tree's build of `65c438f3e`.
- **ON** is `pair_export --commit 65c438f3e --flip kAiUntouchableGateBound=true` (SHA-256 prefix
  `BB13B37B37AF`).
- The logs are `local\g8_uoff_<row>.log` and `local\g8_uon_<row>.log` in worktree cc9-gunnery8.

| row | `pair_diff` | gate reads / marked, OFF = ON | against the prediction |
| --- | --- | --- | --- |
| JM05 3200/3000 | exit 1, gameplay identical | 2520 / 0 | held |
| USN04 4700/4500 | exit 1, gameplay identical | 5795 / 0 | held |

- **U1 held, but only vacuously.** No marked candidate reaches the gate on either row, so the
  suppression itself is not exercised by any reference row. The binding is the listing's two
  tests on the byte the native writes.
- **U2 held.** Besides the summary line's `bound`, the ON logs differ only in presentation. The
  back buffer is 640x480, and the renderer capability lines are missing. The user's session had become
  an RDP session (`query session`: rdp-tcp Active, console Conn), the known 2026-09-23 state.
  That is environment, not this switch.
- **An OFF 9200/9000 JM05 run** (`g8_uoff_jm05l`), started to look for a later marked read,
  crashed at mission frame 2744. The null read was in `set_native_renderer_render_state_00b24460`
  (`bsp_game+25FBC5`), about 20 minutes into the run. It is renderer-side, not a gunnery path, and most
  likely the transition into the RDP session noted above. It was not re-run, so JM05 past 3000 frames is
  **not measured**.

**Verdict: ON.** The mechanism is the image's, and both rows are identity.
`kAiUntouchableGateBound = true`.

**Still open.**
- A row where an enemy gun reaches a marked unit. On JM05, Event2Pt is on side 0 and 6829 m from
  its nearest enemy at 3000 frames. PT Boat 80' Elco 01 and 02 are on side 2 and are never anyone's
  nearest.
- A plane passed to AddUntouchableUnit: the gate reads `[plane+9D4h]`, which is not followed here.

## 41. Handoff (cc9-gunnery8, 2026-09-28, at about 75% context), with the hull roll torque read

This worker's packets: reference j (GAME_EXECUTABLE), the periscope byte (section 38, ON), the
command-allowed extra tests (section 39, recorded), the entry point
`fire_function_guns_now_009e2b60` (inert, for the ships lane) and the untouchable gate
(section 40, ON). What remains of section 31 is rank 7 and rank 8.

### 41.1 Rank 8, the hull roll torque `00827312`: read, not bound

The host's `ShipHitBinding` in `src/game_hosts_gunnery.cpp` stubs the roll axis as (0, 1, 0) and
both settings as 0. It also records `route_add_hull_torque`. The pure step
`ship_roll_torque` (`src/ship_hit_record.cpp`) is complete, and it is reached only for a torpedo
on a hull heavier than `kShipHitRollTorqueMassFloor` (USN02: 52 calls). Every input now has a
known producer:

| input | the image | the host source |
| --- | --- | --- |
| roll axis | `00C32000` is `LEA EAX,[ECX+8]` (4 bytes), so `008271B7..008271BD` read `[[unit+1018h]+2Ch]+8+18h..+20h`. That is the same matrix row `0092D730` multiplies at `+18h/+1Ch/+20h`, which the host already calls `pose_row2`, the forward axis. **`ship_hit_record.hpp:236`'s comment "`+20h`" should read `+18h..+20h`.** | `GameUnitsHost::unit_pose(index, right, up, forward)`, the `forward` row |
| torque scale, `settings+590h` | `Physics.TorpedoForce`, `0083FE7C`, default 1.0 | `GameplayTuningSettings::physics_torpedo_force` (`src/gameplay_settings.cpp:256`). The gunnery host has no route to it yet |
| mass root, `settings+594h` | `Physics.TorpedoForcePower`, `0083FEC8`, default 2.0 | `physics_torpedo_force_power`. Same gap |
| the sink | `0080FFD0` packs message 93h, and `0077C2A0` routes it at `00827312` / `00827329`. `00821E80` case 93h (`00822235`) calls `0092BF30`, `JMP 00C35330` AddTorque on the controller's body | `bsp::unit_handle_add_hull_torque_00822235` (`include/bsp/unit_force_channel.hpp`) on the units host's `slot.body`. `dyn_body_add_torque_00c35330` is already used there (`game_hosts_units.cpp:5938`, `:6117`) |

**The binding, for the next worker:**
- Add `bool GameUnitsHost::add_hull_torque_message_93h(std::size_t index, const bsp::OceanVec3&)`,
  which calls the 00822235 handler on the slot's body.
- Give the gunnery host the two Physics floats. Find the owner that loads `GameplayTuningSettings`,
  and read the stored load rather than a literal.
- Fill `roll_axis`, the two settings and `route_add_hull_torque` behind `kShipHitRollTorqueBound`,
  committed OFF.
- **Timing to label:** the gunnery hit runs after the fixed step's force phase. Check whether
  `slot.body`'s torque accumulator survives to the next step's integration before claiming that
  the torque lands.
- Pairs: USN02 9200/9000 (52 calls) and any row with torpedo hits on heavy hulls (USN04, E2,
  USN13, JM05; count `ShipHit::add_hull_torque_00827312` in reference j's logs first).

### 41.2 Rank 7, `007788B0` controller ownership: not started

Section 37.2: exact while `ctl+284h` is empty or names this controller. It is not checked for a
player-controlled unit. The host has no producer for `+284h`.

### 41.3 Environment

- A JM05 9200/9000 run crashed in the renderer (`set_native_renderer_render_state_00b24460`, a
  null read) about 20 minutes in. The ON logs after it showed a 640x480 back buffer. The cause is the
  user's session becoming RDP (rdp-tcp Active, console Conn), under which every run fails at
  renderer init. Check `query session` before a run.

### 41.4 Tools in the cc9-gunnery8 tree (`local\`)

- `g8_runs.ps1 -V <export> -Only <rows>`: reference j's twelve rows on an export.
  `g8_runs_tree.ps1 -P <prefix>` does the same on the tree's own build.
- `g8_wait.ps1 -Glob <pattern> -Expect N`: a foreground wait on the final COM release line.
- `g8_cmp.ps1 -A <prefix> -B <prefix> -Rows <rows>`: `pair_diff` headlines. The prefix `rb9` means
  reference i's logs in cc9-gunnery7.
- `g8_loo.ps1 -Specs 'v:kA,kB'`: leave-one-out exports of a commit (edit the commit inside).
- `g8_rows.py <prefix>`: headline rows against reference i.
- `g8_rel32.py <addr...>`: every `CALL`/`JMP rel32` and absolute dword naming an address in the
  image on disk.
- `g8_mapsym.py <rva> build\win32\bsp_game.map`: symbolise a host crash offset.
- `g8_extra_check.py`: section 39's call-shape census.

## 42. The plane side of the forced fire target (packet `cc9_plane_forced_target_read`, cc9-gunnery9)

Section 33.4 left one question open: plane rows store the forced fire target from `008358D0`, and
their AutoTarget accepts it, but does the image's plane side ever read that target?

### 42.1 The image never stores it on a plane, and never reads one there

**No plane-side object has the `+238h` fire target.** The chain is:

| step | the image | evidence |
| --- | --- | --- |
| the director of a plane instance | vtable `[114h]` is `0047F180`, `XOR EAX,EAX; RET` | every class answering `IsKindOf(0Fh)` (vtables `00D05F20`, `00D06638`, `00D1A000`, `00D19D28`, `00D06920`, `00D00070`, `00D0BA80`, `00D00308`, `00D1A2D8`, from `src/unit_kind_query.cpp`), read at vtable `+114h` on disk |
| the director of a squadron | vtable `[114h]` is `007ECFD0`, `[+348h]`, the `22Ch`-byte controller `0084D810` builds with vtable `00D0BD98` | `00D088D4`; `0084D849`; `docs/LUA_BINDING_MISSION.md` (cc9-lua3) |
| the squadron's SetCommand | slot `+60h` of `00D0BD98` is `0071E6C0`, the bare slot push | `00D0BDF8`. The ship director's slot `+60h` is `008358D0` (`00D09FB8`). `008358D0` has no other reference than `00D09F20` and `00D09FB8`, the base and derived weapon director tables (`00D09EC0` and `00D09F58`, slot `+60h`) |
| the squadron's fire-target getter | slot `+2Ch` is `0071F150`, the newest command's target (`0071EBF0`, `00521EA0`), not a stored field | `00D0BDC4` |
| the AutoTarget selector | built only by `009F6A20`, whose one caller is `0083676A` in the ship director's constructor `008366D0`, whose one caller is `00810FA0` | rel32 census of the disk image (`local\g9_img.py refs`) |
| the unit gunnery pass's fire-target provider | `008636A0` (one caller, `00864C18` in the attach `00864BD0`) installs the director-backed provider `00D0D324` (`00863640`, `director+238h`) only when the unit answers `IsKindOf(2)`, `vtable[114h]()` is non-null and that director answers `vtable[48h](2)`. Otherwise it installs the null provider `00D0D314` (`00861B90`, `XOR EAX,EAX`) | decompile of `008636A0` |

A plane instance fails the director test, because its director is null. A squadron fails the
`vtable[48h](2)` test, because `0084D8F0` answers true only for 0 and 1. **So every plane-side
gunnery pass reads a null fire target.** The squadron's command reaches only `0071E6C0`, so
`00835930` never runs for a squadron either.

**Where this host differs.**
- The commands host runs `008358D0` for every unit, a squadron included. `docs/CONSTRUCT_WORLD.md`
  records that the host fuses the squadron with its leader plane, whose ship-style director stands
  in for the `+348h` controller. So `kFireTargetObjectIdBound` stores a forced target on plane rows
  (USN04 3, USN01 5), and the ship-AI host runs an AutoTarget on them. The image does neither.
  Both are outside this lane (`src/game_hosts_commands.cpp`, `src/game_hosts_ship_ai.cpp`), and
  are passed to the lead.
- The gunnery host's `run_gunnery_pass` takes the ship-AI row's stored target for every unit. A
  plane row that holds one feeds step 8.7 a fire target the image never has. **That read is in
  this lane, and is bound below.**

Uncertainty: the forts (`00743F30`), airfield (`006D0D30`) and shipyard (`00842A80`) directors
are not followed here. The switch touches kinds `0Fh` and `18h` only.

### 42.2 The binding (committed OFF)

- `kPlaneNullFireTargetProviderBound` (`src/game_hosts_gunnery.cpp`): on a unit answering
  `IsKindOf(0Fh)` or `IsKindOf(18h)`, the pass's fire target is null, as `00861B90` answers.
- OFF keeps the read, and counts it.
- The summary line is `summary mission gunnery plane fire target provider null_provider_ticks=
  stored_target_reads= nulled= bound=`.

### 42.3 Predictions (written before the ON runs)

- **P1, the mechanism.** OFF, `stored_target_reads` is above 0 on USN04 and USN01, the rows where
  section 33.4 saw plane rows take the target. It is 0 on USN13, whose requests all come from
  launched squadrons. ON, `nulled` equals OFF's `stored_target_reads` until the tracks diverge.
- **P2, USN01 3000: exit 1, gameplay identical.** A torpedo bomber's torpedo-category gun takes
  candidates only from the two director targets. Its command target is the same ship as the
  forced target, so the selection does not change.
- **P3, USN04 4500: exit 1,** for the same reason.
- **P4, USN13 3000: exit 0 or 1.** Nothing is read.
- **The flip rule:** ON when P1 holds and every move traces to a dropped plane fire target.

### 42.4 The pairs, and the flip (2026-09-28)

- **OFF** is this tree's build of `d00136644`.
- **ON** is `pair_export --commit d00136644 --flip kPlaneNullFireTargetProviderBound=true` (SHA-256
  prefix `A58039ACF449`).
- The logs are `local\g9_pnoff_<row>.log` and `local\g9_pnon_<row>.log` in worktree cc9-gunnery9.
  Both sides ran in the same session (an RDP session, whose 300-frame smoke passed renderer init).

| row | `pair_diff` | plane or squadron pass ticks | stored-target reads (OFF) | nulled (ON) | against the prediction |
| --- | --- | --- | --- | --- | --- |
| USN04 4700/4500 | exit 1, gameplay identical | 213420 | 13317 | 13317 | P1, P3 held |
| USN01 3200/3000 | exit 1, gameplay identical | 17592 | 12990 | 12990 | P1, P2 held |
| USN13 3200/3000 | exit 1, gameplay identical | 233220 | 0 | 0 | P1, P4 held |

**Verdict: ON.** The mechanism is the image's `008636A0` rule, and every row is identity.
`kPlaneNullFireTargetProviderBound = true`.

**Still open, outside this lane (passed to the lead):**
- The commands host stores a forced fire target on a squadron row, where the image runs only
  `0071E6C0`.
- The ship-AI host runs an AutoTarget on load-time plane rows, which the image never builds.

## 43. The power-up subsystem: read and plan (packet `cc9_powerup_subsystem_plan`, cc9-gunnery9, docs only)

docs/SHIP_AI_OPEN_ITEMS.md section 5 names this as the prerequisite for the party brain's
engagement pass `00A179E0`. docs/GAMEPLAY_MODIFIERS.md (cc9-ships2) already reads the grant, the
use, the expiry and the product, and docs/POWERUP_CONFIG.md the constructor and the class loader.
This section adds the manager's layout, `008EA0C0`'s filter, whose inventory the AI spends, and a
packet plan. Names are hypotheses.

### 43.1 What the manager holds

`008EDC60` builds the `1C4h`-byte owner and publishes it at `00F88C30` (`008EDD8A`):

| offset | what | evidence |
| --- | --- | --- |
| `+18h`, `+1Ch` | the class map (head, size), filled by the loader `008ECEC0` from `PowerupClasses.lua` | `param_1[6]`, `param_1[7]` in `008EDC60` |
| `+20h` + slot x `0Ch` | **eight inventory lists**, one per player slot (std::list: head at `+24h`) | `_eh_vector_constructor_iterator_(this+20h, 0Ch, 8, ...)`; `008EDDC0` appends here |
| `+80h` + category x `0Ch` | **sixteen active-modifier lists** (head at `+84h`, size at `+88h`) | the same iterator with count 10h; `008EA250` inserts, `008EB110` expires, `008E6430` reads |
| `+140h`..`+148h` | three dwords, zeroed | `param_1[50h..52h]` |
| `+14Ch` + slot x `0Ch` | **eight per-slot runtime maps**, keyed by the item class: the cooldown record | the iterator with count 8; `008EA12D LEA ECX,[EDI+14Ch]` before `00617030` |
| `+1B0h`, `+1BCh` | two name-vector maps (the random-name lists) | `param_1[6Ch]`, `param_1[6Fh]` |

### 43.2 How an entry is created and consumed in single player

**Created only by the Lua native `AddPowerup` (`008EE410`).**
- With one argument, it calls `008EDDC0([game+18ECh], table)`: the local player's slot, which is
  0 in single player (docs/CONTROLLED_UNIT.md). With two, the slot is the first argument.
- **Every call in this installation's scripts passes one table.** A grep of `scripts/**/*.lua` for
  `AddPowerup(` finds only `AddPowerup({` bodies inside per-mission wrappers such as
  `luaAddPowerup(type)` or `luaJM6AddPowerup(type)`, plus the checkpoint restore in
  `global/commandhelpers.lua`. So in single player **only slot 0 ever holds items**, and the enemy
  slot 4 never does.
- The reference rows reach no grant. USN04 grants after its secondary or hidden objective. The
  loose `missions/COTP-IJN/jm06.lua` (2024-07-13) grants `automatic_reloader` and
  `improved_ship_manoeuvreability` after primary objective 1 (:864). The JM06 row loads the packed
  `PRCPIJN\JM06.lua`, whose text is not a loose file. No log in this tree has a power-up native
  call.

**Listed for the AI by `008EA0C0(manager)(slot, &vec)`, `RET 8`.**
- It fills `vec` only when `[00E0C978]` (`EnablePowerups`, forced to 1 in single player) is set.
- It walks the slot's inventory list. For each node it looks the node's class key (`node+0Ch`) up
  in the slot's runtime map (`00617030`, `ECX = manager+14Ch+slot*0Ch`). It keeps the node only
  when the record's byte `+14h` is clear and its float `+10h` is below the clock `[00F876A4]`.
  The reading is that the item is not active and its cooldown is over. That reading is a
  hypothesis: `00617030`'s record is not read beyond these two fields.
- It keeps one entry per class. A later node of a class already in `vec` replaces the earlier one.
  Each entry is `node+8h`, so `[e+4]` is the class key.

**Consumed by one of two users.**
- The local player's HUD paths `008EB705` and `008ED601` (docs/GAMEPLAY_MODIFIERS.md).
- **The AI party brain whose slot is 0.** `00A17A2F` passes `brain+20h`, the party slot. In single
  player `009FFD20` gives 0 to a unit on the local player's team and 4 to the other team, and only
  parties 0 and 4 run (docs/AI_GROUP_THINK.md). **So the allied brain lists, and fires through
  `008EADA0` at `00A180C9`, the items granted to the player.** This answers the question
  GAMEPLAY_MODIFIERS left open ("whether the local player's party slot runs an AI brain").
  The enemy brain's list is always empty.
- A use (`008EADA0`) either launches an air-support flight (`0094BFF0`) or inserts the class's
  non-1.0 multipliers into the category lists. It then stamps the expiry and the cooldown.
  `008EB110` expires the nodes every `Game_OnMove`.

### 43.3 What this host does

- The owner is never built. `004DC6A0` is a record step in `game_hosts_mission_frame.cpp`
  ("global_subsystems"), although `src/global_subsystems.cpp` and `src/powerup_config.cpp` hold a
  reconstructed constructor and loader.
- `AddPowerup`, `PreparePowerup` and `GetAvailablePowerups` are bound as unimplemented natives
  (`src/mission_lua_host.cpp:553-555`). The 38 product sites return 1.0f, which is exact while the
  lists are empty.
- `party_brain_plan_tail` records `00A179E0`'s tail.

### 43.4 The packet plan

Each packet is identity on every reference row, because no row reaches a grant. Each needs a row
that does, or a harness grant, before a pair can test it.

| order | packet | contents | lane | switch |
| --- | --- | --- | --- | --- |
| 1 | `cc9_powerup_owner_at_load` | build the owner at mission load with the existing `construct_powerup_config_008edc60` and `load_powerup_config_008ecec0` against the mission Lua state, and publish it where the hosts can reach it | mission frame and Lua | none; nothing reads it yet |
| 2 | `cc9_powerup_grant` | `008EE410` and `008EDDC0` into slot `[game+18ECh]`'s inventory list, with the `useLimit` field; `008EA610` `PreparePowerup` and `008EB350` `GetAvailablePowerups` read it. The `pup_gain` sound and the `PUM1STGET` hint are records. Message 67 is multiplayer-only | Lua | OFF, predicted identity |
| 3 | `cc9_powerup_use` | `008EADA0` (the cooldown map, `008EA250`, `008E4B00`, the use count), `008EB110` expiry, and `008E6430` with the filter `008E4680`. It replaces the 1.0f at the 38 product sites | the product sites span the units, gunnery and ship-AI hosts; one shared accessor, then one switch | OFF |
| 4 | `cc9_ai_powerup_use` | `00A179E0`'s choice half `00A17D5A..00A180A8`. Its six callees are unread: `008E35F0`, `00A0F680`, `00A046C0`, `00A04860`, `00A04910`, and `008EA0C0` as read above | AI (`party_brain_plan_tail`) | OFF |

**A test row.** Grant through the harness: one `AddPowerup` per class at load, through the
existing Lua drain. The first measurable effect is then packet 4. The allied brain fires the item
on its first think with a candidate pair, and a gunnery product (category 1 FIREPOWER, 7
TARGETING_ERROR, 8 DEVICE_RELOADING) moves. USN02 has early contact, but whether its pair build finds a pair is not measured.

**Coverage.** This read covers `008EDC60`'s layout and `008EA0C0` completely. `008EADA0`,
`008EDDC0` and `008EB110` are cited from docs/GAMEPLAY_MODIFIERS.md, not re-read. `00617030` is
read only as a lookup keyed by the class.

## 44. Rank 7: `007788B0` in the AutoTarget tick is the formation-follower gate (read, cc9-gunnery9)

Section 37.2 left rank 7 as "exact while `ctl+284h` is empty or names this controller; not
checked for a player-controlled unit". The read below replaces that framing.

### 44.1 The image

- **`007788B0` is `BSP_Unit_IsFormationFollower`,** `__fastcall(ECX = unit)`, body
  `007788B0..007788C7`: `g = [unit+284h]`; false when `g` is null, else `[g+14h] != unit`. It is
  true for a member of a unit group that is not the group's leader. Player control is not read.
- **`009F5DC1..009F5DF4` in the AutoTarget tick `009F5DA0`:** when the unit is a follower, the
  tick never selects a target. It reads the director's first command slot `[director+54h]`:
  - null: return (`009F5DD5`);
  - the `follow` singleton `00E08F60`: return (`009F5DE0`);
  - anything else: `0077C980(unit, 0)` (`009F5DEB`), then return.
- **`0077C980(unit, 0)` is the leave.** With `[unit+284h]` set it sends message 77h with a null
  target, which `0077FE80` arm 3 delivers to `0077BD70(unit, null)`. That is the same chain as the
  Lua `LeaveFormation` `00899EB0` (`src/game_hosts_lua.cpp`, packet `cc9_lua_formation_query`).
  So a follower that holds any command other than `follow` leaves its formation on its next think.

### 44.2 The host

`TargetBinding` in `src/game_hosts_ship_ai.cpp` answers `controller_belongs_to_another` with false,
and records `release_controller`. So every follower runs the full selection. The pieces the
binding needs already exist:
- `GameUnitsHost::unit_is_formation_follower_007788b0(index)`, which is `007788B0` whole;
- `GameUnitsHost::leave_group_on_destroy_0077bd70(index)`, which the Lua `LeaveFormation` already
  uses for this chain, with the same labelled substitution: the leave runs at the call, not through
  the session route;
- `director_command_slot()`, which answers the command singleton's image address, so the
  comparison with `00E08F60` in `src/bot_fire_target.cpp` works as written.

### 44.3 Reach, from reference j's logs (`rb10_<row>.log`, worktree cc9-gunnery8)

Follow joins (`summary mission ai follow ... joins=`): JM06 4, JM05 3, JM08 1 and LOMP10 1. Every
other row has 0. LOMP06 has one scripted formation, which it leaves. A follower's own command after
a join is not measured here. If it is `follow`, the gate only suppresses the follower's automatic
target choice. If it is anything else, the follower also leaves the formation.

### 44.4 The binding, for the ship-AI lane (not committed here)

In `src/game_hosts_ship_ai.cpp`, `TargetBinding`, behind a new `kAutoTargetFollowerGateBound`,
committed OFF:

```cpp
bool controller_belongs_to_another(void*) override {
    if (!kAutoTargetFollowerGateBound) {
        owner_.record("AutoTarget::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    owner_.done("AutoTarget::controller_belongs_to_another", 0x007788b0u);
    return owner_.units.unit_is_formation_follower_007788b0(index_);
}
void release_controller(void*, int) override {
    // 0077C980(unit, 0) at 009F5DEB -> 77h -> 0077FE80 arm 3 -> 0077BD70(unit, null),
    // run at the call as LeaveFormation 00899EB0 runs it (labelled).
    if (!kAutoTargetFollowerGateBound) {
        owner_.record("AutoTarget::release_controller", 0x0077c980u);
        return;
    }
    if (owner_.units.unit_formation_group_0284(index_) >= 0) {
        owner_.units.leave_group_on_destroy_0077bd70(index_);
    }
    owner_.done("AutoTarget::release_controller", 0x0077c980u);
}
```

**Predictions for its pairs:**
- JM06 3000 and JM05 3000: exit 3. Followers stop choosing their own fire targets. Death rows may
  move on JM06, whose PlayerSubs and escorts fight.
- JM08 and LOMP10: exit 1 or 3, small.
- Every row without a join: exit 1.
- **Mechanism check:** count follower thinks, and how many leave. A leave on a row predicts a
  formation change that the follow summary shows.

## 45. A squadron's SetCommand is `0071E6C0` alone (packet `cc9_squadron_set_command`, cc9-gunnery9)

This is section 42's first outside-lane difference, bound in this lane.

### 45.1 The image

- A command to a squadron reaches its controller at `[squadron+348h]` (vtable `00D0BD98`), whose
  slot `+60h` (`00D0BDF8`) is `0071E6C0`, the bare slot push. There is no category test and no
  forced fire target `00835930`. `008358D0` is only in the weapon-director tables (section 42.1).
- A plane instance has no director at all (vtable `[114h]` = `0047F180`).
- **Also different, not bound:** the slot push calls `director->vtable[14h]` at `0071E73C`. That is
  `00836040` on a ship director and `0084DD20` on the squadron controller. Both are the same
  rewrite: when the descriptor names a target that `00521EA0` resolves to the controller's own
  `+34h`, the descriptor becomes the empty one (kind 0, id 0, the zero vector at `00F87574`) and
  the routine returns 1. **Only the command set differs:**
  - `00836040` (ships) accepts `cruise` `00E08F70` and `stop` `00E08F88`;
  - `0084DD20` (squadrons, `0084DD20..0084DD9C`, `RET 8`) accepts `moveto` `00E08F68` and `stop`
    `00E08F88`.
  So a squadron's self-targeted `moveto` is emptied, and its self-targeted `cruise` is kept. The
  host's push uses the ship rule for every row. **Open:** it needs the same class test as the
  switch. Its reach is a squadron `moveto` whose target is the squadron itself, and no reference
  row is known to issue one.

### 45.2 The binding (committed OFF)

- `GameCommandUnit` gains `class_id` (the unit's `+C4h`), -1 when unknown.
- `kSquadronSetCommandBound` (`src/game_hosts_commands.cpp`): a row answering `IsKindOf(0Fh)` or
  `IsKindOf(18h)` runs `director_push_command_slot_0071e6c0` alone. The host fuses a squadron with
  its leader plane, whose row is plane-class (USN01's `Mav1` registers with vtable `00D00308`), so
  both kinds are tested.
- OFF: every row runs `008358D0`. The summary line counts the plane and squadron rows and their
  forced fire targets: `summary mission director squadron set command rows= forced_fire_targets=
  bound=`.
- **The field's producer is the units host** (`game_hosts_units.cpp`, where the rows are built
  before `register_units`). Until it fills `class_id`, both counters read 0 and the switch does
  nothing. That line is routed to the units lane.

### 45.3 Predictions (written before the ON runs)

- **Q1, the mechanism.** OFF, `forced_fire_targets` is above 0 on USN04, USN01 and USN13. Those are
  the plane-row calls section 33.1 traced: 35, 7 and 60 unresolved calls, all of them planes. ON,
  it is 0, and `fire target unresolved` drops by the same count.
- **Q2, USN04, USN01 and USN13: exit 1, gameplay identical.** Section 33.4's ON pair added exactly
  these stores and moved no gameplay. Section 42 already nulls the gunnery read. The ship-AI host
  skips launched squadrons.
- **The flip rule:** ON when Q1 holds and every row is identity. A move would have to trace to a
  plane-row AutoTarget losing its locked target (USN04 and USN01), which section 42.1 records as a
  host-only path the ship-AI lane is removing.

### 45.4 The pairs, and the flip (2026-09-28)

- **OFF** is this tree's build of main `6adc7e80d`, where `class_id` is filled (`66598f005`).
- **ON** is `pair_export --commit 6adc7e80d --flip kSquadronSetCommandBound=true` (SHA-256 prefix
  `10D9C2E18F37`).
- The logs are `local\g9_sqoff_<row>.log` and `local\g9_sqon_<row>.log` in worktree cc9-gunnery9.

| row | `pair_diff` | plane or squadron rows | forced fire targets, OFF -> ON | `fire target unresolved`, OFF -> ON |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 | exit 1, gameplay identical | 840 | 47 -> 0 | 35 -> 0 |
| USN01 3200/3000 | exit 1, gameplay identical | 56 | 9 -> 0 | 7 -> 0 |
| USN13 3200/3000 | exit 1, gameplay identical | 1488 | 87 -> 0 | 60 -> 0 |

- **Q1 held.** The forced targets exceed section 33.1's unresolved counts, because resolved calls
  from plane rows are counted too. ON sends none. `fire target unresolved` drops to 0 on all three
  rows, so every unresolved call came from a plane row.
- **Q2 held.** Every row is identity.

**Verdict: ON.** `kSquadronSetCommandBound = true`.

## 46. Rank 8, the hull roll torque `00827312`, bound OFF (packet `cc9_hull_roll_torque`, cc9-gunnery9)

Section 41.1 read the inputs. This section checks the arithmetic and the timing, and binds it.

### 46.1 The listing against `ship_roll_torque`

`00827126..0082730B`, read from the disk bytes:
- **The direction.** `shot->vtable[34h]` (`00827144`). The squared length is summed as `(x*x + y*y) + z*z` and
  stored as a float before `_CIsqrt` (`0082717E`). x and z are divided by the float root
  (`008271A2`, `008271AA`), and y is not used again.
- **The axis.** `00C32000` returns the body matrix (`LEA EAX,[ECX+8]`), and `+18h..+20h` is row 2 (`008271B7..008271BD`).
- **The mass term.** `1 / [settings+594h]` (`008271D8..008271E6`). A zero `[class+0B0h]` gives 0
  (`LAHF / TEST AH,44h / JP`). A negative mass is negated as `[00D7A208] - mass`. Otherwise it is
  `FYL2X / F2XM1 / FSCALE`, that is `pow(|mass|, 1/root)`.
- **The scale.** `[settings+590h] * hull_damage(0) * mass_term`, stored as a float (`00827251..0082725F`).
- **The sign.** `axis.z*dir.x - axis.x*dir.z` (`00827263..00827277`, `FSUBP ST(2)`). It is -1 when negative, 1 when positive
  and 0 when zero.
- **The result.** `sign * scale` rounded to a float, times each axis component (`008272B0..008272DB`). It is
  packed at `00827312` and routed at `00827329`.

`ship_roll_torque` matches in order and rounding at the x87 precision of 24 bits that d3d9 is expected to set
(`include/bsp/game_hosts.hpp`, milestone 2l). **Uncertain by a few ULP:** the host takes `std::pow` where the image
chains `FYL2X` and `F2XM1`. With every hull's inertia zero (46.3), that cannot move anything.

### 46.2 Timing

- The image posts message 93h at row 9 of the fixed step (`00778450`, `00875E91`).
- The Dyn world step `00C5C540` runs at `00875E0C`, before every fan-out row. The tick-element waves that run the
  gunnery and the motion ticks (wave 2, `00875D1A..00875D89`) run before it too.
- So a hit's torque reaches the accumulator after its own step's integration and is integrated at the next step's
  `00875E0C`. The accumulator is cleared only at the end of the position phase (`src/rigid_body_integration.cpp`).
- In this host, the motion pass of step n is the image's motion tick plus Dyn step of n. The gunnery step runs
  before it (`game_hosts_units.cpp`).
- So the binding **posts** the torque and delivers it at the start of the next gunnery step. There it joins that
  step's motion torques and is integrated with them, as the image integrates it at step n+1.

### 46.3 The binding (committed OFF)

- `kShipHitRollTorqueBound` (`src/game_hosts_gunnery.cpp`). It sets:
  - the axis from `unit_pose`'s forward row, which `publish_pose` copies from `motion.pose_row2`;
  - the two settings from `GameMissionLuaHost::read_physics_torpedo_force_0083b5e0`. This installation's
    `shipglobals.lua` (2024-07-13) has `TorpedoForce = -75` and no `TorpedoForcePower`, so the power is 2.0;
  - the sink, `GameUnitsHost::add_hull_torque_message_93h`, one step later.
- The summary line is `summary mission gunnery hull roll torque calls= posted= delivered= undelivered= max= bound=`.
- **Caveat, and the follow-up:** every host hull body has a zero inverse inertia, because the collision AABB's
  producer `00C5C940` (the shape attach behind `00937D3F..009399BF`) is unread (`game_hosts_units.cpp`, the
  hull-body build). `00C41550` turns a torque into angular velocity only through that tensor, so the torque moves
  nothing until `00C5C940` is read.

### 46.4 Predictions (written before the ON runs)

- **R1, the mechanism.** ON, `posted` equals OFF's `calls`, which reference j counts as
  `ShipHit::add_hull_torque_00827312` (USN02 9000: 30). `delivered + undelivered = posted` minus at most the last
  step's posts. `max` is non-zero.
- **R2, USN02 9000 and JM06 3000: exit 1, gameplay identical,** because of the zero inverse inertia.
- **The flip rule:** ON when R1 holds and both rows are identity. Any move is a failure of the inertia claim and
  keeps the switch OFF.

### 46.5 The pairs, and the flip (2026-09-28)

- **OFF** is this tree's build of `5cb63b252`.
- **ON** is `pair_export --commit 5cb63b252 --flip kShipHitRollTorqueBound=true` (SHA-256 prefix
  `128C3D8AF61C`).
- The logs are `local\g9_rtoff_<row>.log` and `local\g9_rton_<row>.log` in worktree cc9-gunnery9.
  Both sides ran in the same RDP session, and a 300-frame smoke passed first.
- ON loads `Physics.TorpedoForce=-75.000 TorpedoForcePower=2.000 (loaded=1)`.

| row | `pair_diff` | calls OFF | posted / delivered / undelivered ON | largest torque | against the prediction |
| --- | --- | --- | --- | --- | --- |
| USN02 9200/9000 | exit 1, gameplay identical | 26 | 26 / 26 / 0 | 4039222.0 | R1 and R2 held |
| JM06 3200/3000 | exit 1, gameplay identical | 0 | 0 / 0 / 0 | 0 | R2 held, but vacuously: no torpedo hit lands on a hull over 500 mass |

**Verdict: ON.** The mechanism is the image's, and every torque reaches the body. Nothing moves,
because the inverse inertia is zero. `kShipHitRollTorqueBound = true`.

**The follow-up** is the collision AABB producer `00C5C940`. Once a hull has a real inertia, these
torques act, and USN02 (26 of them, up to 4.0e6) is the row to re-pair. The minus sign of this
installation's `TorpedoForce` reverses the roll direction the axis sign gives.

## 47. The hull inertia: what the image needs, and what is missing (packet `cc9_hull_inertia`, read, cc9-gunnery9)

Section 46.5's follow-up. docs/SHIP_HULL_BODY.md and docs/SHIP_HULL_SHAPES.md already read the
whole path from the shapes to the inertia. This section places the one missing input and plans the
binding. Names are hypotheses.

### 47.1 The chain, with what is already read

| step | the image | state |
| --- | --- | --- |
| which collision records become shapes | `00938F61..0093918C` walks `model+4Ch` and keeps records owned by `firstnode`, `model+0Ch`, `front` or `back` | read (SHIP_HULL_SHAPES) |
| the shape descriptor | type 4 (convex mesh), identity rotation, translation from the record, `desc+14h` = the address of `record+0Ch` | read |
| the shape's own box | `00C57C40`: `mesh = *(record+0Ch)`, local box `mesh+18h..+2Ch`, expanded by 0.02, pushed through the shape transform | read, reconstructed in `bsp/ship_hull_shapes.hpp` |
| the body's box | `00C55FC0`: the union of the shape boxes into `B+38h..+4Ch` | read, reconstructed |
| the inertia | `00939A89..00939C05`: extent = max - min; `I = mul * (Mass/12) * (ey2+ez2, ex2+ez2, ex2+ey2)`; `00C37E70` | read, reconstructed (the host's `--hull-extent` probe input) |
| **the convex mesh's local box `mesh+18h..+2Ch`** | the Dyn convex mesh object the model's collision record points at | **not read: its producer is unknown** |

### 47.2 What this read added

- **The record's `+0Ch` is a handle.** The `ConvexObject` resource (parser tables `00CFD7E8` and
  `00CFD80C`; its parse slot `006FAF00` builds a `2Ch`-byte object through `006F9CD0`) has vtable
  `00CFB6A4` and embeds at `+0Ch` a `{pointer, 0}` pair (`00C32D50`). So `*(record+0Ch)` is a
  pointer to a separately built Dyn convex mesh. The parse itself (`006FAD70`: `006FA7F0`,
  `006FA910`, `006FACE0`, `006F9B20` reading u32 lists) fills index arrays and does not write that
  pointer. **Where the Dyn mesh is created, and how its box is computed, is unread.**
- **The chunk layout, read from this installation's model** (`models/ships/us/deruyter.mmod`,
  46,865,813 bytes; `local\g9_mmod.py`). After the name `ConvexObject` (length 0Ch before it)
  come a u32 chunk size (`5EC4h`), a u32 0, and a u32 record count (`65h` = 101 in the first
  object). Each record is a float3 position followed by three index lists, each prefixed by the
  same count (4, 4, 4 in the first object, 3, 3, 3 in the second). The positions are vertices, so
  the local box is most likely their min and max. **That is a hypothesis until the Dyn mesh's
  builder is read.** A `BoundingSphere` chunk (centre and radius) precedes each object.

### 47.3 The plan

| order | packet | contents | owner |
| --- | --- | --- | --- |
| 1 | `model_collision_records` (SHIP_HULL_SHAPES' follow-up) | the producer of `model+4Ch`'s records, `record+0Ch` (the Dyn mesh) and `record+14h` (the translation); which node `model+0Ch` is; where the Dyn convex mesh is built and how `+18h..+2Ch` is filled | the model packets |
| 2 | `cc9_hull_aabb_host` | a host reader that takes each hull class's `.mmod`, keeps the records the four owners select, and produces the body box with `dyn_convex_mesh_shape_bounds_00c57c40` and the union | gunnery or units lane, once 1 settles the inputs |
| 3 | `cc9_hull_inertia` | feed that box to the hull build (`game_hosts_units.cpp`, where `bsp::ShipHullBodyInputs` is filled; today the extent is 0), behind `kHullInertiaFromShapesBound`, OFF | units lane (cc9-lua9) |

**Predictions for 3, for when it lands.**
- USN02 9000: exit 3. The 26 torpedo torques (up to 4.0e6) now roll the hit hulls. The
  motion model's own AddTorque at `00937613` carries a zero vector today (its gain is
  multiplied by `settings+588h`, which has no producer), so that site stays inert. Whether a
  roll survives the motion tick's velocity rewrite `0092D300` on the next step is not read;
  that decides whether the move is visible beyond the hit step.
- JM06: exit 1, since no roll torque lands and the other torque is zero, unless the non-zero
  world inertia changes something else in `00C41550`.
- The flip rule has to separate the shape-derived inertia from every torque it wakes. Pair it
  first with the roll torque OFF.

### 47.4 The local box's producer, found (cc9-gunnery9, continued)

- **The hull build.** The `ConvexObject` parse region calls `00C5DEB0` at `006FAECE` with
  `ECX = resource+0Ch` (the handle) and the chunk's points. `00C5DEB0` is
  `DYN_ConvexHull_Construct` (replacement order), reconstructed in docs/AVOID_ZONE_DYN_HULL.md.
  Its internal record keeps the vertex minimum and maximum at `+18h` and `+24h`, written by
  `00C389C0` from the hull's vertices. That record is `mesh+18h..+2Ch`, the box `00C57C40` reads.
- **The offset cancels.** Before the call, `006FAEA0..006FAEC7` subtracts the resource's
  `+14h..+1Ch` vector from every point (`FSUB [EDI+14h]`, `[EDI+18h]`, `[EDI+1Ch]`). SHIP_HULL_SHAPES
  gives `record+14h` as the shape's translation, with an identity rotation. So a shape's box in the
  body frame is the chunk points' own box, widened by 0.02, up to float rounding. A convex hull's
  vertices are a subset of its points and include the extremes, unless the 4096-vertex limit cuts
  some, so the hull's box equals the points' box.
- **This installation's DeRuyter** (`models/ships/us/deruyter.mmod`, `local\g9_convex.py`) has ten
  `ConvexObject` chunks. Nine are part-sized (extents of 1.3 to 16.5). One is the hull:

  | chunk | points | min | max | extent |
  | --- | --- | --- | --- | --- |
  | file+`2C5AC04` | 117 | (-8.23, -5.65, -96.90) | (8.23, 12.20, 77.02) | (16.46, 17.85, 173.93) |

  With only that shape kept, the body box is its box widened by 0.02 on each side, so
  extent = (16.50, 17.89, 173.97). Mass 7688 (the run log) then gives `k = 7688/12 = 640.67`, and
  `I = mul * 640.67 * (17.89^2 + 173.97^2, 16.50^2 + 173.97^2, 16.50^2 + 17.89^2)`, about
  `mul * (19.6e6, 19.6e6, 0.38e6)`. The roll axis is row 2, so the roll inertia is the third
  term, about `mul * 3.8e5`. USN02's largest torque of 4.0e6 then changes the roll rate by about
  `10.5 / mul * dt` rad/s: 0.5 / `mul` rad/s over a 0.05 s step. That is large, so **the pair will
  move**.
- **Still unread: which records are kept.** The chunk does not name its owner node. The four
  owners are `firstnode`, the node at `model+0Ch`, `front` and `back` (SHIP_HULL_SHAPES). The
  DeRuyter has none of the named three, so the kept set is the records owned by `model+0Ch`. The
  hull chunk is the obvious candidate, but that is not proven. The producer of `model+4Ch` (the
  record list with each record's owner) and of `model+0Ch` is the model loader.

**Packet 2 of 47.3, restated.** For each hull class: decode the `.mmod` hierarchy enough to know
each `ConvexObject`'s owner and `model+0Ch`; keep the owned chunks; take the union of
`[min - 0.02, max + 0.02]`; hand the extent to the hull build. `local\g9_convex.py` already
decodes the point lists.

## 48. Handoff (cc9-gunnery9, 2026-09-28, at about 70% context)

### 48.1 What this worker landed or left

| section | packet | state |
| --- | --- | --- |
| 42 | `cc9_plane_forced_target_read` | `kPlaneNullFireTargetProviderBound` ON: no plane-side gunnery pass reads a fire target |
| 43 | `cc9_powerup_subsystem_plan` | docs: the manager layout; slot 0 is the only slot that holds items; the allied brain spends the player's items; four packets planned |
| 44 | rank 7, `007788B0` | read: the formation-follower gate; the binding `kAutoTargetFollowerGateBound` is routed to the ship-AI lane |
| 45 | `cc9_squadron_set_command` | `kSquadronSetCommandBound` ON: plane and squadron rows run `0071E6C0` alone. Open: the squadron's `0084DD20` self-target rule (45.1) |
| 46 | `cc9_hull_roll_torque` (rank 8) | `kShipHitRollTorqueBound` ON; identity until the hull has an inertia |
| 47 | `cc9_hull_inertia` | read: the local box is the `ConvexObject` points' box (47.4); the DeRuyter hull chunk measured; still unread: which records `model+0Ch` owns |

### 48.2 Open items this worker found

- The ship-AI host runs an AutoTarget on load-time plane rows, which the image never builds
  (section 42.1; routed to cc9-ships8).
- The squadron slot-push rule `0084DD20` (45.1): a self-targeted `moveto` or `stop` is emptied
  where the host applies the ship rule (`cruise` or `stop`). No known reach.
- The hull inertia chain (47.3), and after it USN02 as the row where the roll torques act.
- `ship_roll_torque` takes `std::pow` where the image chains `FYL2X` and `F2XM1` (46.1), a
  few-ULP difference to settle once torques act.

### 48.3 Tools in the cc9-gunnery9 tree (`local\`)

- `g9_img.py dwords|refs|pat|rtti`: dwords at an address, every `CALL/JMP rel32` and absolute
  dword naming an address, byte patterns in `.text`. The image has no RTTI.
- `g9_runs.ps1 -P <prefix> -Only <rows> [-Exe <path>]`: reference j's rows (plus `smoke`, 300
  frames of USN01) with the reference run parameters. `g9_wait.ps1 -Glob -Expect`: a foreground
  wait on the final COM release line. `g9_cmp.ps1 -A -B -Rows`: `pair_diff` headlines (`rb10`
  means reference j's logs in cc9-gunnery8).
- `g9_mmod.py <model> <n>`: hex and floats around the first n `ConvexObject` chunks of a `.mmod`.

### 48.4 What remains for cc9-gunnery10

- **Reference k**, when the lead calls it. It carries the flags that landed after reference j's
  base, including this worker's `kPlaneNullFireTargetProviderBound`, `kSquadronSetCommandBound` and
  `kShipHitRollTorqueBound`.
- **The model trace, 47.3's three packets.**
  1. Which records `model+0Ch` owns. This is the only unread input. 47.4 already settles the box
     producer and measures the DeRuyter hull chunk.
  2. A host reader for each hull class's box. `local\g9_convex.py` decodes the point lists.
  3. The binding in `game_hosts_units.cpp` (the lua9 lane). Pair it first with the roll torque
     OFF.
- **The `queue_state_back` pointer defect** (section 37.3). A slot keeps its old
  `slot_target[].object` while taking the shifted slot's parameters. Nothing reads it yet.
- **The E2 tail.** USN04 past 9000 frames is phase 1 only (reference j), and is not examined here.
- **The LandConvoy unit.** `00805680`'s group records need one in the units host's index space
  (section 34).
- **The squadron slot-push rule `0084DD20`** (45.1). A self-targeted `moveto` or `stop` on a
  squadron is emptied. It is open, with no known reach.
- **The follow entry point** `issue_follow_command_00720cd0` (`e50480a39`) is inert until lua9
  wires it (`kFormationJoinFollowBound`).

### 48.5 Reference k exists

Reference k (docs/GAME_EXECUTABLE.md, "Mission reference baselines, 2026-09-28 k (main 5aaa4948a)",
`reports/cc9_reference_rebaseline_11.json`, cc9-gunnery10) replaces j. It adds the LOMP10 9000 row. Of
this lane's landings, `kAiUntouchableGateBound`, `kPlaneNullFireTargetProviderBound`,
`kSquadronSetCommandBound` and `kShipHitRollTorqueBound` were exported together with
`kPlaneRowAutoTargetBound`. That group moves one plane's path on JM06 (PBY Catalina 01) and is
gameplay-identical on every other row. `kSubmarinePeriscopeOutBound` is gameplay-identical on
every row.

## 49. The convex mesh's local box: read (section 47.3 step 1, cc9-gunnery10, docs only)

Section 47's missing input is found. **The Dyn convex mesh is built inside the `ConvexObject` parse
itself, and its local box is the AABB of the chunk's vertices after they are re-centred.** Ghidra
hides it because its body for `006FAD70` stops at the `_free` at `006FADDE` (the CRT CALL_RETURN
gap: `bsp.py ghidra flow 006fad70` reports `006FADE3..006FADE6`). The disk bytes continue to
`006FAEE2 RET 4`. Names are hypotheses; Ghidra was not changed.

### 49.1 The `ConvexObject` resource (2Ch bytes)

| offset | producer | meaning |
| --- | --- | --- |
| `+00h` | `006F9CFD` | vtable `00CFB6A4`; slots `+08h`/`+14h` return the type tokens `[00E19A98]`/`[00E19AA4]` (`006F9AA0`, `006F9AB0`) |
| `+04h` | `00B868B8` (`BSP_ResourceItem_ConstructReferenceBase`) | reference count, 1. **Not a node**: the owner node lives in the model's vector element (49.2) |
| `+08h` | `006FAD8F` | `00BE9A10`'s result, read only when at least 0Ch bytes remain in the chunk (`006FAD83`) |
| `+0Ch..+13h` | `00C32D50` zeroes it; `00C5DEB0` at `006FAECE` fills it | the Dyn hull handle `{data pointer, 0}` (`AvoidZoneDynHullHandle` in `bsp/avoid_zone_dyn_hull.hpp`) |
| `+14h..+1Ch` | `006F9EE0` | centre = (min + max) * 0.5 of the raw vertices (the double 0.5 at `00D7A280`) |
| `+20h..+28h` | `006F9EE0` | half extent = (max - min) * 0.5; min and max seed at `+FLT_MAX` `00D7A248` and `-FLT_MAX` `00D7A244` |

### 49.2 The parse `006FAD70` (`__thiscall`, ECX the object, one stack argument the reader, `RET 4`)

```
006FAD9D  CALL 006FA7F0      ; count -> [ESP+28h]; 20h-byte records, float3 at +4h (00BE9A60 x3), then index lists
006FADAD  CALL 006FA910      ; a second 20h-byte record array
006FADBD  CALL 006FACE0      ; 14h-byte records (006F9B20)
006FADFE  CALL 006FAA20
006FAE22  new(count * 0Ch)   ; a packed float3 array, EBX
006FAE40..006FAE5F           ; copy each vertex record's +4h..+0Fh into it (stride 20h -> 0Ch)
006FAE89  CALL 006F9EE0      ; this, (points, count): the AABB into +14h..+28h
006FAEA0..006FAEC7           ; points[i] -= this+14h/+18h/+1Ch   (re-centre on the box centre)
006FAECE  CALL 00C5DEB0      ; ECX = this+0Ch, stack (count, points): build the Dyn hull
006FAED4  free(points)
006FAEE2  RET 4
```

The loop count at `006FAE30` and `006FAE81` is `[ESP+28h]`, the count `006FA7F0` wrote, so the
hull's points are the first record array. `006FAF00` (the parse slot) allocates the object with
`operator new(2Ch)`, runs `006F9CD0` and calls `006FAD70`.

`00C5DEB0` is already reconstructed (`avoid_zone_dyn_hull_replace_00c5deb0`, docs/AVOID_ZONE_DYN_HULL.md;
719 differential cases). Its `68h` data record holds the hull's **minimum XYZ at `+18h` and
maximum at `+24h`**, which are exactly the `mesh+18h..+2Ch` that `00C57C40` reads through
`*(record+0Ch)`.

### 49.3 The model's `+4Ch` vector and the owner

The hull walk (`00938F61..0093918C`) reads the model as `[[edi+1Ch]+360h]+160h` (`00938F76..00938FA9`).
`model+4Ch` is a checked vector with first/last at `+50h`/`+54h` (`00938FB3`, `00938FB6`). **Each
element is 8 bytes** (`ADD EBX,8` at `00939177`): `+0` a `ConvexObject*` (`MOV EBP,[EBX]` at
`0093908D`) and `+4` the owner node (compared at `00939023..0093906C`). The walk pushes
`object + 0Ch`, the handle's address, into `controller+34h` (`00939097`; stored at `009390C9`, or
through the insert `009313C0` at `009390F3`). So SHIP_HULL_SHAPES' "record" is the `ConvexObject`, and its "record+14h
translation" is the box centre.

**Consequence: the shape's box in body space is just the raw vertex box.** The shape sits at the
centre `c` with identity rotation and its local box is `c`-relative, so `00C57C40` yields
`[min - 0.02, max + 0.02]` of the chunk's own vertices, up to the hull generator's 0.001 dedup.
Every extreme vertex is on the hull, so dedup and the 4096-vertex limit are the only ways the box
could differ; a degenerate chunk takes the generator's synthetic eight-point box
(AVOID_ZONE_DYN_HULL, "Producer evidence").

**Not read:**
- **Who appends to `model+4Ch`.** The type tokens are read only by the resource's own slots and
  `00CCEC17`/`00CCEC4A` (a registration), so the loader reaches the objects through a virtual call.
  It belongs to the model loader (MODEL_REACHES_UNIT: the model is `class+50h`'s `vtable[8h]()`,
  stored at part instance `+160h` by `00713604`).
- **Which node `model+0Ch` is.** SHIP_HULL_SHAPES found `firstnode`, `front` and `back` absent from
  `deruyter.mmod`, so the hull is the records owned by the `model+0Ch` node.
- **A contradiction to settle.** MODEL_REACHES_UNIT reports no non-null writer of `class+50h`, so
  `unit+360h` would stay 0. But the walk has no null exit: with `[unit+360h]` zero it takes
  `00938F93 XOR ECX,ECX` and then `00938F9D MOV ECX,[ECX+0Ch]`, a read of address `0Ch`. A ship
  that reaches the hull build natively therefore has a non-null `unit+360h`, and the "no writer"
  negative is probably a vacuous scan (a block copy or a virtual store). The same unguarded pattern
  (`[unit+360h]` null gives a null model, then `MOV EBP,[EAX+50h]`) already runs at `0093856C..00938580`.
  **Not excluded:** an earlier branch of `00937C90` that skips all of this when `unit+360h` is null;
  the function's control flow above `0093855D` was not read.

### 49.4 Plan for step 2, `cc9_hull_aabb_host` (not bound; superseded by 49.7)

1. **The reader.** For a hull class's `.mmod` from this installation, find each `ConvexObject` chunk
   (section 47.2's layout: name, u32 size, u32 0, u32 record count, then per record a float3 and
   three equal-count index lists). Take the owner from the enclosing node chunk; that framing is
   the one input still to be read from the file, since the owner is not inside the chunk.
2. **The box.** Keep the records owned by the `model+0Ch` node. For each, compute `min`/`max` of its
   float3s, the centre, and the re-centred points; run `avoid_zone_dyn_hull_replace_00c5deb0` and
   take `data->minimum/maximum` as `mesh_local`. Feed `dyn_convex_mesh_shape_bounds_00c57c40` with
   identity rotation and the centre, then the union (`00C55FC0`).
3. **A cheap first check.** Compare the hull's box with the raw vertex box for every chunk of
   `deruyter.mmod`; any difference beyond 0.001 means the dedup or a degenerate case acted.
4. The binding stays step 3 (units lane, `kHullInertiaFromShapesBound`, OFF), with section 47.3's
   predictions.

### 49.5 The two open pieces, closed: who fills `model+4Ch`, and which node `model+0Ch` is

Section 47.4 (cc9-gunnery9) reached the same box producer independently. This subsection closes
what 49.3 and 47.4 left open. Names are hypotheses; Ghidra was not changed.

**The writer of `model+4Ch` is `0071B710` (`BSP_GameResourceInstance_PublishItem`).** Its ABI is
`__thiscall`, with ECX the model instance, stack `(item, node)`, and `RET 8` at `0071B7FF`.
- **Census.** `006F9A80`, the ConvexObject's static type-token accessor (`MOV EAX,[00E19A98]`), has four `CALL rel32`
  callers: `0071406A` (`00714060`), `0071B75A` (`0071B710`), `0071BB8D` (`0071BB40`) and `009358A6`
  (`00935540`). The token `[00E19A98]` itself is read only by the class's own slots `006F9A81`,
  `006F9AA1`, `006F9D45` and the registration `00CCEC4A`. `0071B710` has no direct caller; it is
  reached only through the vtable slot `00CFD8E8`.
- **The store.** After the base publication `00B89E90` (`0071B722`), `0071B758..0071B77C` asks the
  item's slot `+0Ch` (IsKindOf, `006F9D40`: it scans the token list `00E19A98..00E19AA0`) with
  `006F9A80`'s token. On a match it builds `{item, node}` at `[ESP+10h]/[ESP+14h]` and appends
  it with `0071AFC0` to `instance+4Ch`. The other four predicates fill `+3Ch`, `+5Ch`, `+6Ch` and
  `+7Ch`. docs/NATIVE_MODEL_GRAPH_AQ.md already reconstructs this caller and gives the same table.
- `0071BB40` is the file-level resource's own classifier: it appends the bare item pointer to
  `resource+54h` (`0071BBB1`). It is not the 8-byte list.
- The file hierarchy record's `+4Ch` (docs/NATIVE_RESOURCE_HIERARCHY_FIELDS.md: data, count and
  capacity of u32 resource indices) is a different object. `model` is the runtime instance at
  part instance `+160h`.

**`model+0Ch` is the graph root: hierarchy record 0.** This is NATIVE_MODEL_GRAPH_AQ's
graph-builder evidence. `B891A0`, reached from the model handle's slot `+8h` through `007137F0`
(the `class+50h` `vtable[8h]()` of MODEL_REACHES_UNIT), builds one node per hierarchy record. It
saves the first constructed node at `B894FB` and publishes it to `instance+0Ch` at
`B89600..B89606`. Every record's items are then published with that record's own node.

**The DeRuyter hull chunk is owned by the root.** The decoder is `local\g10_mmod_tree.py`, read-only on this
installation's `models/ships/us/deruyter.mmod`, mtime 2024-07-13. The file is a tree of
`{u32 name length, name, u32 size, payload}` chunks: `MMOD` then `BoundingSphere`, `BoundingBox`,
`Resource` (173 items), `Hierarchy` (34 `Item`s), `Warnings` and `Errors`. Each `Item` holds
`Name`, `Parent`, `Matrix`, `Flags` and one u32 `Resource` index per attached item.
- Item 0, `GroupRoot_Deruyter` (no `Parent`, `Flags` 1, identity `Matrix`), lists resource 125:
  the ConvexObject at file+`2C5AC00`. That is 47.4's hull chunk, 117 points.
- The other nine ConvexObjects (resources 107 to 123) belong to the `smoke0N-fizika_01` and
  `bridge0N-fizika_00` items (items 25 to 33), which are children of `De_Ruyter:hull`.
- **So the walk's owner 2 keeps exactly the hull chunk**, and owners 1, 3 and 4 find nothing.

**Across this installation's ship models** (88 `.mmod` files under `models/ships`; `local\g10_rootbox.txt`):

| root record's ConvexObjects | files | examples |
| --- | --- | --- |
| one | 46 | `deruyter`: min (-8.23, -5.65, -96.90), max (8.23, 12.20, 77.02) |
| more than one | 2 | `hospital_ship` (3 of 67), `california_kikotobe` (4 of 4) |
| none | 40 | `akagi` (hull chunks on item 1 `Akagi:akagi_lod1`), `hiei` (on `FrontReduced` and `RearReduced`), `soryu`, `jap_cargo`, `i-400` |

- Every root `Matrix` is identity.
- **No item in any of the 88 files is named `firstnode`, `front` or `back`,** either whole or
  after the colon. `0071AD50` looks the name up in a sorted map through `0071AAE0`. Its key form (the
  full `Model:item` name or not) and its comparison (case-sensitive or not) are **not read**. If the
  lookup is exact, owners 1, 3 and 4 never match in this installation. The 40 classes without a
  root ConvexObject would then attach no hull shape at all.

### 49.6 Open item: `00937C90` against MODEL_REACHES_UNIT (settled in 49.9: reading (a))

Two readings, neither settled here:
- **(a) `class+50h` has a writer the scans missed.** Then every ship that reaches the hull build has a
  non-null `unit+360h`. `00937C90` reads `[[unit+360h]+160h]+50h` at `0093856C..00938580`, and
  `+0Ch` at `00938F9D`, with no null exit on those paths: the `JZ` at `00938574` and `00938F89`
  only substitute a null model, which is then dereferenced. MODEL_REACHES_UNIT's "no writer"
  would be a vacuous scan: a block copy, a store through a register base, or a virtual.
- **(b) An earlier guard skips the block.** A branch of `00937C90` above `0093855D`, or its caller,
  skips all of this when `unit+360h` is null. Then natively no hull shapes attach, the body box
  stays empty, and the inertia is zero, as it is in the host today.

The discriminating read is `00937C90`'s control flow from its entry to `0093855D`, plus a census of
`class+50h` stores that includes `REP MOVSD` block copies over the class descriptor. Under (b),
section 47.3's step 3 changes no row.

### 49.7 Plan for step 2, `cc9_hull_aabb_host`: the reader and the declaration step 3 consumes

This replaces 49.4. It stays in the gunnery lane, as a new header and source, with
`cmake/startup.cmake` taking one line. It is not bound.

```cpp
// include/bsp/mmod_hull_convex_box.hpp (proposed)
namespace bsp {
// The body-space box the hull body's attached convex shapes give (00C55FC0's union of
// 00C57C40's per-shape boxes), for the shapes the walk 00938F61..0093918C keeps: the
// ConvexObjects that hierarchy record 0 (the graph root, model+0Ch) lists. Already
// widened by kDynConvexMeshBoundsEpsilon (0.02), so it goes straight into
// ShipHullBodyInputs::aabb_min/aabb_max. shape_count 0 means no root ConvexObject:
// leave the inputs at their zero default (49.5, 49.6).
struct MmodHullConvexBox {
    OceanVec3 min{};
    OceanVec3 max{};
    std::uint32_t shape_count{0};   // root-listed ConvexObjects
    std::uint32_t point_count{0};   // their vertices, for the log line
};
bool read_mmod_hull_convex_box(const std::vector<std::uint8_t>& mmod_bytes,
                               MmodHullConvexBox& out, std::string& error);
}  // namespace bsp
```

**The reader.** It parses the chunk tree as 49.5 decodes it and takes `Hierarchy` item 0's
`Resource` indices. It keeps the ones whose `Resource` child is named `ConvexObject`. For each, it
follows the parse's own float sequence rather than a raw min and max:
1. Take the point box and centre `c = (min + max) * 0.5` as `006F9EE0` computes it: min and max
   seed at plus and minus `FLT_MAX`, the double 0.5 at `00D7A280`, stored to float.
2. Re-centre the points, `p - c` per component in float (`006FAEA0..006FAEC7`).
3. Build the hull with `avoid_zone_dyn_hull_replace_00c5deb0` and read `data->minimum/maximum`
   (`00C389C0`).
4. Take `dyn_convex_mesh_shape_bounds_00c57c40` with an identity rotation and translation `c`.
5. Merge the shape boxes as `00C55FC0` does.

For a single chunk the result is the raw point box widened by 0.02, up to float rounding. The
first check compares the two for every root chunk of the 88 files.

**What step 3 does** (the units lane, cc9-lua10; one line per field):
- In the hull block of `game_hosts_units.cpp` (the `bsp::ShipHullBodyInputs hull{}` at about line
  9840), behind `kHullInertiaFromShapesBound` (OFF), read the class `Mesh` string. Use
  `lua.read_resource_file` exactly as `class_buoyancy_list_0082fe30` does, cache per `type_id`, and
  on success with `shape_count > 0` set `hull.aabb_min = box.min`, `hull.aabb_max = box.max` and
  `hull.shape_count = box.shape_count`.
- Log one line per class: `shape_count`, `point_count` and the extent.
- **DeRuyter's expected extent** is (16.50, 17.89, 173.97). 47.4 gives the inertia and the USN02
  prediction; pair it with the roll torque OFF first, as 47.3 says.
- If 49.6 resolves to reading (b), step 3 stays OFF for good.

### 49.8 Step 2 implemented (packet `cc9_mmod_hull_convex_box`)

`include/bsp/mmod_hull_convex_box.hpp` and `src/mmod_hull_convex_box.cpp` implement 49.7's
declaration. The build is registered in `cmake/startup.cmake`. The reader:
- walks the model with the recovered StructuredReader, as `read_mmod_aux_point_items_0071b3e0` does;
- reads every `ConvexObject` entry's points (006FAD70's front half: the `+8h` word when at least
  0Ch bytes remain, then `006FA7F0`'s count and records);
- takes the first `Hierarchy` `Item`'s `Resource` positions;
- makes one shape per listed ConvexObject, as `0071B710` appends them.

**Simplification.** The box is the raw vertex min/max widened by 0.02, not 49.7's float sequence
through `00C5DEB0`. The two are equal up to float rounding, since every extreme vertex is on the
hull. The hull's 0.001 dedup and its 4096-vertex limit are not reproduced.

**Verification.** The Win32 Release build passes, and so do both CTests. One check was added to
`tests/math_tests.cpp` (`reconstructed_math`). It reads this installation's
`models/ships/us/deruyter.mmod` (mtime 2024-07-13 11:25:14 -0700) through `config/target.json`'s
`binary` path, and skips rather than fails when the file is absent. It ran here with this result:

| field | value |
| --- | --- |
| shape_count | 1 |
| point_count | 117 |
| extent | 16.5039 x 17.8909 x 173.9681 |
| min.z | -96.92411 |

Status: build-tested and fixture-tested on one model. Not bound; step 3 is the units lane.

### 49.9 Section 49.6 settled: no path builds the hull body without the model (reading (a))

This is the discriminating read 49.6 named, done on the disk bytes and the Ghidra listing of `00937C90`
(`local/output/ghidra-disasm-00937c90-lines-2200-*.txt` in cc9-gunnery10).

**No guard inside `00937C90`.** Every conditional or unconditional jump in the function was listed with
its source and target:
- **Before `0093856C`.** No jump reaches past the first model read (`0093856C`) or the walk
  (`00938F28`), except two kinds. One is `009381AC JMP 00938D5D`, the entry jump of a rotated loop
  whose body is `009381B1..00938D5C` (the disk bytes confirm it). The others are two loop exits,
  `009384A6 JZ` and `009384B7 JNC` to `0093873C`, which land before the walk.
- **Between the first model read and the walk.** No jump from `0093856C..00938F28` goes past
  `00938FB9`.
- **Before `0093856C` generally.** No instruction reads `+354h`, `+360h`, `+160h` or a model `+50h`.
- **Consequence.** The walk (`00938F1C..0093918C`) is on every path to the hull-body tail
  (`009399C0..`). With `[unit+360h]` null it takes `00938F93 XOR ECX,ECX` and then reads
  `[0Ch]` at `00938F9D`. The function has an unwind frame and no catch, so that is an access violation.

**No guard in the callers.**
- **The controller constructor.** `00937C90` has one caller, `CALL 00939E2A`, in the controller
  constructor `00939CB0` (body `00939CB0..00939E43`), which contains no jump at all.
- **Who reaches the constructor.** `00939CB0` is called by `0080DEF9`
  (`BSP_UnitInstance_CreateMotionController 0080DEC0`, whose only test is the `operator new(390h)`
  result at `0080DEEA`) and by the variant constructors `00939E72`, `00939F28` and `00939F48`.
- **Dispatch.** `0080DEC0` is reached only through six vtable slots (`00CF92E0`, `00CFA9A8`,
  `00CFB968`, `00CFC600`, `00D01860`, `00D098A8`).

**Conclusion.** In the image, every unit whose motion controller is constructed builds its hull body
after reading a non-null model at `[unit+360h]+160h`. So the native hull body always has the
record-0 shapes that `0071B710` published. Reading (b) is refuted. Reading (a) holds:
`class+50h`, or `unit+360h` by some other path, has a writer that MODEL_REACHES_UNIT's scans did not
find. **That writer is still not located**, and it is not needed for step 3.

**For step 3 this means:**
- binding the shape box is faithful for every hull class with a root ConvexObject;
- for the 40 classes without one (49.5), the native body box is the union over an empty shape set.
  How `00939A89` and `00C37E70` treat that seed (`+FLT_MAX`/`-FLT_MAX`) is not read, so leaving
  those inputs at zero is a host choice, and it is flagged.

### 49.10 The empty shape set: zero extent, zero inertia, zero inverse (packet `cc9_empty_hull_box_seed`)

**The `FLT_MAX` seed is never what an empty body keeps.** SHIP_HULL_SHAPES had read that a body with
no shape never reaches the union `00C55FC0`: all six of its callers run with a shape already linked.
This packet confirms what the body holds instead, and what the inertia path does with it.
- **The box is exactly zero.** The body initialiser `00C43CA0` (`Dyn_Body_InitFromDescriptor`) zeroes
  XMM0 at its entry (`00C43CA1 XORPS XMM0,XMM0`). Filtering its listing for XMM0 shows no later
  write, only stores and `COMISS` reads. So `B+38h..+40h` and `B+44h..+4Ch` are stored as `0.0f` at
  `00C43D22..00C43D41` and again at `00C43E5E..00C43E77`.
- **No early out in the inertia block.** `00939A80..00939C10` contains three calls: `00C31F90` at
  `00939A89` (the box read-back), `00424C40` at `00939B27` (the settings singleton, for `mul`) and
  `00C37E70` at `00939C05`. It contains no conditional jump. With a zero box the extent is (0, 0, 0)
  and every inertia component is `mul * Mass/12 * 0 = 0`.
- **Zero maps to zero.** `00C37E70` (`Dyn_Body_SetInertia`, `__thiscall(body, const float[3])`,
  `RET 4`) stores `0.0f` for any component that compares equal to zero (the `UCOMISS` and `JNP`
  pair) and `1/x` otherwise. So the inverse inertia at `M+54h..+5Ch` is (0, 0, 0): the hull never
  rotates under a torque.

**The rule for lua11's step 3.** When `read_mmod_hull_convex_box` returns `shape_count == 0`, leave
`aabb_min`, `aabb_max` and `shape_count` at their zero defaults. That is the native result, not a host
choice; the flag in 49.9 is withdrawn.

**One exception the reader does not cover: the periscope shape.** `009396BA..009399BF` adds one more
shape to the same body under three conditions: `class+510h <= 0`, `class+514h <= 0`, and a node found by
the name `periszkop` (`00D0C1F0`, `0071AD50` at `009396D8`). Its geometry is the `model+4Ch` pair whose
node is that node (`00939843`: `[pair+4] == [ESP+18h]`). It is placed by the node's local matrix, with
a real rotation (SHIP_HULL_SHAPES, "The periscope shape"). In this installation five models have such an
item, and each owns a ConvexObject:

| model | item |
| --- | --- |
| `i-400` | `i-400:periszkop nolod` |
| `kaiten` | `kaiten:periszkop` |
| `minisub` | `minisub:periszkop nolod` |
| `type7` | `type7:periszkop nolod` |
| `type_b` | `type_B:periszkop` |

All five have no root ConvexObject. Whether `0071AD50` finds them depends on the unread name key form
(49.5): the full `Model:item` name, the part after the colon, or neither, and whether the
` nolod` suffix matters. So for these five classes the native box may be the periscope shape's box
alone, which is small and gives a non-zero inertia. That is left as a flag for step 3, not bound.

## 50. Handoff (cc9-gunnery10, 2026-09-28, at about 75% context)

### 50.1 What this worker landed

| where | packet | state |
| --- | --- | --- |
| GAME_EXECUTABLE, "Mission reference baselines, 2026-09-28 k (main 5aaa4948a)" | `cc9_reference_rebaseline_11` | reference k: thirteen rows, twenty switches attributed by thirteen leave-one-out exports and three group exports; `reports/cc9_reference_rebaseline_11.json` |
| the same section | `cc9_e2_tail_attribution` | the E2 tail is USN04's move displaced across 225 s; the carried flag is closed |
| 49.1 to 49.3, 49.5 | `model_collision_records` | the ConvexObject parse builds the Dyn hull; `model+4Ch` is written by `0071B710`; `model+0Ch` is hierarchy record 0 |
| 49.7, 49.8 | `cc9_mmod_hull_convex_box` | `read_mmod_hull_convex_box` is implemented and fixture-tested on `deruyter.mmod`; binding it is step 3 (lua11) |
| 49.9 | 49.6's read | no path builds the hull body without the model (reading (a)) |
| 49.10 | `cc9_empty_hull_box_seed` | an empty shape set is native zero extent, zero inertia and zero inverse inertia |
| SHIP_AI_OPEN_ITEMS 15 | - | the "reference k replaces j" note |

### 50.2 Open items this worker found or left

- **The name lookup's key form.** `0071AD50` looks names up in a sorted map through `0071AAE0`.
  What key it uses (the full `Model:item` name, or the part after the colon) and how it compares
  (case, and the ` nolod` suffix) are unread. This decides two things:
  - whether owners `firstnode`, `front` and `back` can ever match (49.5);
  - whether the periscope shape attaches for the five Japanese submarine models (49.10).
- **The `class+50h` writer** (or `unit+360h` by another path). 49.9 proves one exists; it is not located.
- **The 40 zero-root classes** (49.5). The native answer for their box is zero (49.10), except the
  periscope case above.
- **Reference l's first flags.** These are the switches that went ON after `5aaa4948a`, listed in
  SHIP_AI_OPEN_ITEMS 15. There are twelve, and cc9-lua11's step-3 binding will add one.
- **Reference k's unseparated items:**
  - JM06's US Cargo Transport 02 survival, redundant inside the ship-AI group;
  - the identity group's move of one plane (PBY Catalina 01) on JM06;
  - the JM05 and USN12 path moves inside the ship-AI group.

### 50.3 Tools in the cc9-gunnery10 tree (`local\`)

- **`g10_runs.ps1 -V <prefix> -Only <rows>`.** Runs reference k's rows, plus `lomp10l` (LOMP10
  9200/9000) and `smoke`. The binary is `local\<prefix>\build\win32\Release\bsp_game.exe`.
- **`g10_wait.ps1 -Logs <names>`.** A foreground wait on the final COM release.
- **`g10_retry.ps1`.** Waits, runs one 300-frame smoke, and reports OK or FAIL for the session.
- **`g10_rerun.ps1`.** Relaunches every `rb11*` log that has no final COM release.
- **`g10_loo.ps1 -Specs v:kA,kB [-Parallel]`.** Builds a `pair_export` of `5aaa4948a` with switches OFF.
  `g10_wexp.ps1 -V <v>` waits for such builds.
- **`g10_matrix.py <variants>`.** `pair_diff` of each variant against k per row, into
  `g10_matrix.json`.
- **`g10_rows.py`.** k's rows against j. `g10_cmp.ps1` gives `pair_diff` headlines.
- **`g10_tail.py`.** The E2 tail (E2 minus USN04) per variant.
- **`g10_mmod_tree.py <model> top|at|hier|rootbox|convex`.** The `.mmod` chunk tree, the hierarchy
  items with their ConvexObjects, and the root box census (`g10_rootbox.txt`).
- **`g10_str.py <strings>`.** The VA of an ASCII string in the installed executable.

## 51. Step 3 of 47.3, the hull extent binding, bound OFF (packet `cc9_hull_inertia`, cc9-lua11, 2026-09-28)

**The binding**, in src/game_hosts_units.cpp, behind `kHullInertiaFromShapesBound` (committed OFF):
- `Impl::class_hull_box` reads the class `Mesh` string and opens it through
  `lua.read_resource_file`, as `class_buoyancy_list_0082fe30` does. It runs
  `read_mmod_hull_convex_box`, caches the result per `type_id`, and logs one `hull shapes` line per
  class: `shapes=`, `points=`, the extent and the minimum.
- In the hull block, before `ship_hull_body_create_00937c90`, `hull.aabb_min`, `aabb_max` and
  `shape_count` are set from the box. This happens only when the read succeeded and
  `shape_count > 0`.
  - With no root ConvexObject, the zero default stays, which is the native result (49.10).
- The periscope shape (49.10) is not added.
- The file is read only ON, so the OFF log is unchanged.

**Evidence the box is the right input:**
- 49.8 fixture-tested the reader on this installation's DeRuyter: 1 shape, 117 points, extent
  16.5039 x 17.8909 x 173.9681.
- cc9-gunnery10's census (`local\g10_rootbox.txt` in that tree) finds a root ConvexObject in 48 of
  the 88 ship models. The other 40, including akagi, soryu, yamato, saratoga and porter, keep a
  zero box, and so keep zero inertia.

### Predictions, written before any ON run

Two pairs, as 47.3 asks: first the inertia alone, then with the roll torque.

**Pair A: the roll torque OFF on both sides.** Both sides are exports of this commit with
`kShipHitRollTorqueBound=false`; ON also flips `kHullInertiaFromShapesBound`.
- USN02 9200/9000 and JM06 3200/3000 come out gameplay identical (exit 0 or 1).
- The motion model's own AddTorque at `00937613` carries a zero vector (47.3). So no torque acts,
  and a non-zero inertia changes no motion.
- The only new lines are the `hull shapes` class lines. Every hull class with a root ConvexObject
  logs `shapes>=1`, and a DeRuyter row, if the mission has one, logs extent (16.50, 17.89, 173.97).
- A moved gameplay number here would mean the inertia reaches something else in `00C41550`. That
  would be recorded as a finding, not as a failure.

**Pair B: the roll torque ON, as landed.** OFF is this tree's build; ON flips
`kHullInertiaFromShapesBound` only.
- USN02 moves (exit 3). The 26 roll torques, up to 4.0e6, reach hull bodies with a real inertia.
  47.4 estimates a roll-rate change of about 0.5/`mul` rad/s per 0.05 s step for a DeRuyter-sized
  hull.
  - Whether a roll survives the motion tick's velocity rewrite `0092D300` on the next step is not
    read. If it does not, the move shows only as small pose differences after each hit.
- The moved units are the torpedo-hit hulls whose class has a root ConvexObject. A hit hull
  without one (a zero box) does not move.
- JM06 comes out exit 1: no roll torque lands there (46.5).

**Mechanism failure:**
- a `hull shapes` line reporting `shapes=0` for a class that the census lists with a root
  ConvexObject;
- a DeRuyter extent other than (16.50, 17.89, 173.97) +/- 0.01;
- pair A moving.

### 51.1 The pairs and the verdict (cc9-lua11, 2026-09-28): OFF, the hydrodynamic torque wakes

All four sides are builds of `5a68b4d19`, which adds a diagnostic `summary hull tilt <ship>: max=
final=` line: the angle between each hull's up row and world up. The logs are in this tree's `local\`.

| pair | OFF | ON | USN02 9200/9000 | JM06 3200/3000 |
| --- | --- | --- | --- | --- |
| A (roll torque OFF both sides) | `l11_ha2off_` | `l11_ha2on_` | 3; hit records 2271 -> 3963, damage 39396 -> 36182, deaths 10 = 10 | 3; hit records 276 -> 289, damage 4340 -> 5803, deaths 1 = 1 |
| B (roll torque ON, as landed) | `l11_hb2off_` | `l11_hb2on_` | 3; hit records 2271 -> 4203, damage 39396 -> 34680 | 3; the same numbers as A |

**Held:**
- Every class with a root ConvexObject logs `shapes=1`. DeRuyter logs extent (16.50, 17.89, 173.97)
  and min (-8.252, -5.672, -96.924).
- Classes without one keep a zero box and do not move. On JM06 the transports and tankers
  (`us_troop_transporter.mmod` and others in the census's no-root list) have identical tilts on
  both sides.

**Missed: pair A moved.** The prediction held that no torque acts without the hit roll torque.
That was wrong. `009329C0`, the hydrodynamic tail of `00937440`, calls AddTorque (`00C35330`) on
every hull on every step (`summary mission hydrodynamics ... add_torque=58273` on JM06). With a
zero inverse inertia it did nothing; with the shape-derived inertia the buoyancy elements' moments
turn the hulls.

| row | OFF max tilt | ON |
| --- | --- | --- |
| USN02, living hulls | at most 1.18 deg | 5 to 24 deg at most, 0.3 to 6.4 deg at the end (Kortenaer 23.3 -> 0.4, DeRuyter 16.6 -> 6.0) |
| USN02, the 10 ships that die | at most 1.18 deg | 62 to 180 deg: every wreck rolls over as it sinks (John1 179.7, Asagumo 174.3, Tokitsukaze 161.7) |
| JM06 | at most 8 deg (the submarines) | Fletcher-class 08 (the controlled unit, damaged) holds 10.4 deg; Gato-class 01 19.7; PlayerSub 03 22.7 |

- The ten capsized hulls are exactly the ten death rows. No living hull passes 30 degrees.
- Hit records rise sharply on USN02 (2271 -> 3963 without the roll torque, 4203 with it) while
  damage falls. The likely cause is shells meeting the heeled and rolled-over hulls, but that is
  not read.

**Verdict: OFF.**
- The input is right: the extent matches the fixture and the census.
- But the flip wakes a second mechanism the prediction excluded: `009329C0`'s buoyancy torque, on
  every hull, every step.
- Before it can flip, a packet has to predict that torque's effect from the listing:
  - the restoring moment of the element list and the angular damping 1.0, for a living hull's
    rocking;
  - the wreck's element state, for the capsize;
  - how the hit records respond to a tilted hull.
- The roll-torque question of 47.3 is secondary: pair B differs from pair A on USN02 only by the
  26 hit torques (hit records 3963 -> 4203).

## 52. The hydrodynamic torque on a hull with inertia: predictions before the re-pair (packet `cc9_ship_motion_hydro`, cc9-gunnery11, 2026-09-29)

51.1 kept `kHullInertiaFromShapesBound` OFF because the flip wakes `009329C0`'s own AddTorque
(`00933B38`), which acts on every hull on every step. This section predicts that torque's effect from
the listing (as reconstructed in `src/ship_hydro_forces.cpp`, semantic_build_tested). The predictions
were written before any run of this packet. The pair follows in 52.3.

**Diagnostic.** `BSP_HULL_ATTITUDE_TRACE=1` makes the mission frame log one `hull attitude:` line
per ship each 10 motion steps (0.5 s): roll `atan2(row0.y, row1.y)`, pitch `asin(row2.y)`, tilt
`acos(row1.y)` (51.1's quantity), y and alive. The variable changes no state.

### 52.1 The torque, from the listing

- Per element, `00932CBF` transforms the element point, and `00932D02` transforms the same point with
  its local y set to 0. The lever arm (`00932D07`) is the flattened point minus the body position.
  **The arm has no vertical component in the hull frame.** A vertical force at an element therefore
  gives a moment only through the element's lateral (x) and longitudinal (z) offsets. There is no
  metacentric term: stability does not depend on how high the hull stands.
- The element force is the clamped drag plus the buoyancy on world y (`00933780`). The torque adds
  `arm x force` (`009339A7`), then the leak moment `0074F2E0` (`00933A52`, `00933AA7`).
- The buoyancy is `B = c * d * (0.5 * d / draught + 0.5)`, where `d = span - clamp(h, 0, span)`,
  span is the deck line minus the keel, and h is the deck point's height above the water. So
  `dB/dd = c * (d / draught + 0.5)`, which is `1.5 c` at the resting depth `d = draught`. An
  element that is fully under (`h <= 0`) or fully clear (`h >= span`) adds no stiffness.
- **The restoring moments.** The elements sit at x = +/- Width/2 and at `Hull.Segments` stations
  evenly over the Length (SHIP_BUOYANCY_ELEMENTS), and `sum(c * draught) = 10 * Mass`:
  - roll: `k_r = 1.5 * (W/2)^2 * sum(c)`;
  - pitch: `k_p = 1.5 * sum(c * z^2)`.
- **The inertia** is `00939A8E`'s box, `mul * Mass / 12 * (a^2 + b^2)`, with the Ship material's
  `mul = (1, 1, 2)`. Roll is about row 2 and uses `mul.z = 2`; pitch uses `mul.x = 1`.
- **The damping.** The live angular damping is 1.0, applied in both phases of the host's one 0.05 s
  substep: a rate of 2.05/s. The wreck block `00824FE5` doubles the inertia and sets the damping to
  2.5: 5.34/s. The vertical drag adds a little more, about 0.3/s on a DeRuyter, which is ignored
  below.
- **The steering routine keeps both rates.** `0092E8C0` rewrites only the row-1 (yaw) component of
  the angular velocity and carries the row-0 and row-2 components through (`0092E96F..0092EB8B`).
  The pitch righting at `0092E9E1` runs only for `IsKindOf(0Eh)` units. So a roll or pitch rate
  that a torque creates survives the next motion tick. `0092D300` writes only the linear axial
  component.

`local\g11_pred.py` over the classes 51.1's USN02 and JM06 logs list (the Ship material is assumed
for every row):

| class | W | Mass | draught | roll omega (period) | roll zeta | pitch omega (period) | pitch zeta | wreck roll zeta | roll kick per 1e6 N m hit |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| DeRuyter | 16 | 7688 | 5.08 | 1.38 (4.5 s) | 0.74 | 2.06 (3.1 s) | 0.50 | 2.73 | 0.066 rad/s |
| Houston | 16 | 11602 | 5.91 | 0.61 (10.3 s) | 1.68 | 1.95 (3.2 s) | 0.53 | 6.18 | 0.010 rad/s |
| Exeter | 16 | 10350 | 6.67 | 1.16 (5.4 s) | 0.88 | 1.90 (3.3 s) | 0.54 | 3.26 | 0.045 rad/s |
| Perth | 16 | 14357 | 6.17 | 1.26 (5.0 s) | 0.81 | 2.09 (3.0 s) | 0.49 | 2.99 | 0.036 rad/s |
| Alden | 9 | 1308 | 3.35 | 1.57 (4.0 s) | 0.65 | 2.56 (2.5 s) | 0.40 | 2.41 | 1.038 rad/s |
| Kortenaer | 10 | 1800 | 3.62 | 1.55 (4.1 s) | 0.66 | 2.35 (2.7 s) | 0.44 | 2.44 | 0.643 rad/s |
| Minegumo | 10 | 2500 | 4.50 | 1.12 (5.6 s) | 0.92 | 2.19 (2.9 s) | 0.47 | 3.38 | 0.299 rad/s |
| Yudachi | 10 | 1712 | 4.47 | 1.41 (4.4 s) | 0.73 | 2.38 (2.6 s) | 0.43 | 2.67 | 0.696 rad/s |
| Jintsu | 16 | 5925 | 5.88 | 1.56 (4.0 s) | 0.66 | 2.04 (3.1 s) | 0.50 | 2.41 | 0.126 rad/s |
| Fletcher-class 08 | 9 | 1308 | 5.26 | 0.89 (7.1 s) | 1.15 | 1.86 (3.4 s) | 0.55 | 4.25 | 0.523 rad/s |

- For the submarines the Submarine material has `mul = (1, 1, 1)` and a buoyancy mix of 0
  (`dB/dd = c`). Their roll omega is therefore about 1.15 times the Ship-material figure
  (sqrt(2/1.5)).

### 52.2 Predictions

**P1, living hulls, without hit torques** (roll torque OFF on both sides, as 51.1's pair A):
- **Near-critical.** Every class has a roll zeta between 0.65 and 1.7 and a pitch zeta between 0.4
  and 0.55. A disturbance decays within about 2 s, and the trace shows no roll oscillation that
  keeps its amplitude for more than two periods.
- **No forcing means no lasting tilt.** An undamaged hull feels no steady moment:
  - the lateral and forward drag act along row 0 and row 2, and with a flat arm they give yaw only;
  - the live leak moment is zero, because a live hull's leak rates are zero (`leak_tick`).
- So an undamaged living hull's roll stays within 2 degrees, and its final tilt is under 1 degree.
  The only forcing left is the sea surface under the elements, which enters through
  `water_height_0078cf20`.
- **Pitch** stays within 2 degrees on a hull at speed. A large pitch on a living hull would need a
  bow or stern element to leave its band, which a 2 m wave cannot do on a 5 m draught.

**P2, living hulls with the hit torques** (roll torque ON, as landed): a hit of `tau` N m kicks the
roll rate by `tau * 0.05 / I_z` (the last column). The peak roll is about kick / omega * 0.5.
- A 1e6 N m hit on a Kortenaer gives about 12 degrees.
- A 4e6 N m hit on an Alden-sized hull passes 45 degrees, beyond the band's linear range (on
  Alden, h reaches 0 or span at about 40 degrees).
- Large transient rolls on the destroyers are therefore expected only in pair B, and each should
  settle within about 3 s.

**P3, wrecks.**
- While a wreck still floats inside the band (its deck line above the water at some station), it is
  overdamped (zeta 2.4 to 6). It heels to `tau_leak / k_r` and does not oscillate.
- `0074F2E0` is the water weight at each leak point. Its horizontal lever arm comes through the pose
  rows, so it includes the point's height: a heeled hull with water above its centre gets a moment
  that grows with the heel. The buoyancy's flat arm does not grow.
- **Once every element is submerged** (the deck point under water at every station), the stiffness is
  zero and nothing opposes the leak moment. The wreck then rolls steadily at about
  `tau_leak / (5.34 * 2 I_z)`.
- Predicted: **a wreck's tilt stays under 30 degrees until its y has fallen by about the deck
  height above the waterline** (Height * (1 - ratio) plus the waterline offset; about 5 to 6 m for
  the cruisers, about 3 m for the destroyers). After that it grows monotonically. Past 90 degrees it
  keeps rolling, because nothing restores it.
- **The ten wrecks of 51.1 all rolled past 60 degrees.** The prediction is that each did so only
  after this y threshold, and never while it still floated in its band.

**P4, hit records.**
- A tilted hull moves the hull segment boxes the shells test against. The extra hit records come
  from the wrecks, which stay on the surface for a time as rolled hulls: the rise is in hit records
  after each victim's death time.
- Hit records on upright living hulls move by less than 10%.
- Damage falls because shells spent on wrecks do not reach the living hulls.
- This is the weakest prediction here. The segment test's use of the rotated pose is not re-read in
  this packet.

**Mechanism failure:**
- any living hull whose roll oscillates without decaying;
- an undamaged living hull with a final tilt over 3 degrees and no hit in its last 10 s;
- a wreck passing 60 degrees while its y is still above the threshold.

Either would mean the torque, the arm or the inertia differs from the reading above.

### 52.3 The pairs and the verdict: OFF, P3 and P4 failed

Four exports of `a46b58312`, run with `BSP_HULL_ATTITUDE_TRACE=1` (logs `local\g11<pair><side>_<row>.log`
in the cc9-gunnery11 tree):
- pair A: the hit roll torque OFF on both sides;
- pair B: the hit roll torque as landed.

ON flips `kHullInertiaFromShapesBound`. The gameplay numbers reproduce 51.1's exactly:
- USN02, hit records 2271 -> 3963 (A) and 4203 (B), damage 39395.6 -> 36181.5 (A);
- JM06, 276 -> 289 hit records, 4340.0 -> 5802.7 damage, deaths identical.

`local\g11_att.py` summarises the trace.

**P1 held in its dynamics, but its premise was wrong.**
- **No hull oscillates.** Kortenaer's largest heel on USN02 (pair A) rises smoothly from 5.9 to
  23.2 degrees between 248 and 258 s, then falls back to 3.8 degrees by 275 s, with no swing at the
  predicted 4.1 s period. Every living hull behaves like this: quasi-static, following a slowly
  changing moment.
- **Every undamaged hull stays at 0.0 degrees:** Alden, Exeter, Perth, Encounter, Jupiter and Witte
  (damage taken 0 in the unit table).
- **Every damaged living hull lists.** Examples, with max and final roll: Kortenaer 23.2 / 0.4,
  John2 23.9 / 3.3, DeRuyter 16.6 / 6.0, Yudachi 17.1 / 1.8, Jintsu 7.0 / 6.4. On JM06,
  Fletcher-class 08 ends at 10.4 degrees and PlayerSub 03 at 20.6.
- **The moment is the live flooding.** The premise "a live hull's leak rates are zero" came from
  the `leak_tick` comment in the units host, and it is stale. Packet `cc9_live_hull_leak` is bound
  (`summary live hull leak ... applied=2759` on USN02), so a hit hull takes water at its leak
  points, and `0074F2E0` turns that water into a heeling moment.
- The heel is the flooding moment over the band stiffness, and it falls as the repair pumps the
  water out (`repaired=23685`). Pitch stays under 2 degrees on every living hull (P1 held).

**P2 is not settled.**
- Pair B's 19 hit torques (max 4.04e6 N m) did not produce the large destroyer kicks predicted.
  Pair B's living maxima are of the same order as pair A's (Haguro 27.0, John2 23.9, Yudachi 17.1).
- The torques' targets were not traced, so whether they fell on cruisers or on wrecks is unread.

**P3 failed as written.**
- Every wreck passed 60 degrees while its y was still near the surface: John1 at y = -2.0,
  Asagumo -2.3, Minegumo -3.1, Amatsukaze and Yukikaze -3.6.
- The prediction had tilt below 30 degrees until the deck stations were under water (about 3 m for
  the destroyers). John1 passes 30 degrees at y = -0.5, 11.5 s after its death.
- The mechanism the trace shows instead: the wreck's flooding moment grows past the most the
  flat-arm buoyancy can return. That maximum is one side of the band fully dry and the other fully
  under, so the restoring moment is bounded while the leak moment is not. The hull goes over while
  it still floats.
- The prediction took the saturation to come from sinking alone and missed saturation by heel.
- The capsize itself is consistent with the listing, but its onset was mispredicted, so it counts as
  a failed prediction.

**P4 failed.**
- The wreck hits fell: `summary mission gunnery wreck hits` delivered 22 -> 18.
- The rise is in entity impacts (993 -> 1297), shots (2316 -> 2588), shell mesh hits (994 -> 1345)
  and hit records per impact (2.29 -> 3.06). Damage-control element hits follow the impacts
  (993 -> 1297).
- Where the extra records per impact come from (heeled hull segment boxes, or blast shapes) is not
  read.

**Verdict: `kHullInertiaFromShapesBound` stays OFF.** By this section's own criteria, P3 is a
mechanism failure.

**What the next packet needs:**
- **The wreck capsize, predicted quantitatively.** For each wreck on USN02, take from the host:
  - the leak water at `unit+10FCh`;
  - the leak points and weights that `0074F2E0` reads;
  - the band's bounded restoring moment `sum((W/2) * (B_full - B_dry))`.
  Then predict the heel at which the leak moment wins. The data exists in the units host (lua12's
  lane); a trace there would settle it.
- **P4.** Read what a hit record is queued per, in the impact path from `0081F980` / `00723AA0` to
  the hit queue, and whether it uses the hull's rotated pose.
- **P2.** Trace the targets of the 93h roll torques.
- The flooding list on living hulls matches the listing's two moments and needs no change. It is
  the first place in the rebuild where a damaged ship visibly lists.

## 53. The name lookup `0071AD50`: an exact match on a Note's name (cc9-gunnery11, 2026-09-29)

This closes 50.2's first item, read from the listing and checked against this installation's models
(`local\g11_notes.py`, `local\g11_keys.py` in the cc9-gunnery11 tree).

**The lookup is a linear search, not a sorted map:**
- `0071AD50(model, const char* key)` is `__thiscall` with `RET 4`. It builds a `std::string` from
  the key (`00408720`, the whole string, no split) and passes it by value.
- `0071AAE0` receives the model's 8-byte-element vector at `model+7Ch` (begin `+80h`, end `+84h`).
  It copies the key into a predicate and calls `00719FA0`.
- `00719FA0` is a `find_if`: it steps 8 bytes at a time until the predicate `00718C70` answers true.
- `00718C70` copies `element[0]+8`, the item's name, through `00711C30`. It then calls `004BEB60`,
  which is `name.compare(0, name.size(), key, key.size())`, and returns `compare == 0`. That is an
  **exact, case-sensitive, whole-length equality**.
- The first match wins. `0071AD50` returns `element[1]` (the node `0071B710` stored beside the
  item), or 0 when nothing matches.

**The `+7Ch` list holds the model's Note items.** `0071B710` appends `{item, node}` to five typed
lists at `+3Ch`..`+7Ch`, and `+7Ch` is the fifth, the `E19B54` token. The evidence that these are
Notes: every literal key passed to `0071AD50` exists verbatim as the name of a `Note` chunk
(`u32 4, "Note", u32 size, u32 n, name`) in the models that use it. None of them is a
hierarchy-item name:

| call site | key | where it is a Note name |
| --- | --- | --- |
| `0072E9E9`, `0072EA15` (gun setup) | `base`, `barrel` | every turret in `models\devices` sampled (140_turret: Notes `base`, `damage1`, `barrel`, `barrelfront_1`; its items are `140mm:base`, `140mm:barrels`, ...) |
| `0050774B`, `0050776E` (planes) | `rotor_still`, `rotor_still_dam` | 99 of 150 plane models |
| `00938DB9` (hull shapes) | `front` | 58 of 239 ship models; `back` in 37 |
| `009396D8` (periscope) | `periszkop` | 12 submarine models: I-54, I-56, I-58, i-400, kaiten, minisub, type7, type_b, U-69, Cachalot, gato, narwhal |

**Consequences:**
- **`firstnode` matches nothing.** No ship model has a Note named `firstnode`.
- **`front` and `back` match in 58 and 37 ship models.** Among them are zero-root classes from
  49.5: akagi (`front`), yamato, super_yamato and i-400 (`front`, `back`), and porter, renown and
  us_troop_transporter (`front`, `back`).
- **49.5's census was of the wrong names.** It searched the hierarchy items for `front`, `back` and
  `firstnode` and found none. The lookup never reads the item names.
- **So the "zero box" of 49.10 is not settled for these classes.** Their `front` or `back` Note
  resolves to a node. If that node owns ConvexObjects in `model+4Ch`, `00938F61..0093918C` keeps
  them.
  - The next step is to take each Note's node (the `node` `0071B710` pairs with it) from the file,
    and re-run the root-box census over the ConvexObjects that node owns.
  - That is a change to `read_mmod_hull_convex_box`'s owner set, and it waits for that census.
- **The periscope shape attaches for the 12 submarine models listed above** (49.10's question). The
  suffix ` nolod` belongs to the hierarchy item's name, which the lookup does not read.
- **Uncertainty:**
  - The `E19B54` token is identified with the Note type by this match evidence, not by reading
    the token's string.
  - The node a Note pairs with (its parent item, or a node of its own) is not read here.

**50.2's second item, the `class+50h` writer, is already located.** `00879590`
(`BSP_DamageableClass_LoadModelResource`) stores the `007188A0` game-resource result with
`MOV [EBP+50h],EAX` at `00879768` (disk bytes re-read for this note). See the correction at the end
of docs/MODEL_REACHES_UNIT.md and docs/NATIVE_DAMAGEABLE_CLASS_MODEL_BE.md. 49.9's "still not
located" and 50.2's open item predate that record.

## 54. Reference l's two flags: the acos USN04 deaths and the JM05 Phelps path (cc9-gunnery11, 2026-09-29)

These are reads of existing and new exports of `3f1499210`, the reference l base. No switch
changes.

### 54.1 Why the acos pair's USN04 44 -> 40 does not reproduce on l

The pair (PLANE_FOLLOW_LAW 17.6) was taken on `1d7b7045e`. Its logs are `l10_a0_usn04` (OFF)
and `l10_acon_usn04` (ON) in cc9-lua10's tree.
- That base is reference k plus `kFormationJoinFollowBound` and `kLandingSequencerBound`.
- Thirteen more switches went ON between it and l, among them:
  - the follower gate;
  - clearance;
  - the follow-law turn rate and live speed;
  - avoid zone and the land sub-states.

**What the pair changed there.** Deaths went from 44 to 40:
- Five OFF-only rows: **Fletcher-class03**, which sinks at 210.61 s, then D3A Val #5.1|.-2 and
  #5.1|.-4, and A6M Zero #6.2|.-2 and #8.2.
- One ON-only row: D3A Val #1.1|.-4.

**On l.** Fletcher-class03 does not sink in k, in l, or in any l variant run:
- with acos OFF alone (`nacos`);
- with the follower gate OFF (`nafg`).
Its sinking belonged to the formation-join-without-gate state of the pair's base.

**The acos switch alone on l.** `nacos` against l:
- the 43 USN04 death rows are identical in membership, with 34 changed in time or killer;
- damage is 11286.2 -> 11326.8;
- E2 has the same death set.

**The switch's death change flips sign with the base.** Two more exports toggle the acos switch with
one other landing already OFF (USN04):

| base | acos OFF -> ON | notes |
| --- | --- | --- |
| l | 43 -> 43, same rows | damage 11286.2 -> 11326.8 |
| l, follower gate OFF (`nafgacos` -> `nafg`) | **41 -> 44** | + D3A Val #1.1|.-4, #5.1|.-2, A6M Zero #6.2; torpedo releases 3 -> 7 of 16; damage 12345.8 -> 16923.6 |
| l, clearance OFF (`nclracos` -> `nclr`) | 41 -> 41, one swap | torpedo releases 2 -> 1 of 16 |
| `1d7b7045e` (the pair) | **44 -> 40** | - |

- **Conclusion.** The acos switch moves which strike planes die, and how many torpedoes are released,
  on every base. The direction and size of that change depend on the ship AI's formation state.
- The 44 -> 40 was a property of the pair's base. It holds no claim about the switch alone, and
  nothing on l is unexplained.
- The switch's own mechanism (the fly-to arm, 17.6) is not in question.
- These plane rows are RNG- and geometry-coupled, like every USN04 plane-death change.

### 54.2 JM05's USS Phelps: the gate interacts with formation join, not with clearance

Reference l's flag said "the follower gate and clearance interact". The variants say otherwise.
Phelps is the controlled (idle) unit, and the column is the distance it moved:

| export (switches OFF) | Phelps moved |
| --- | --- |
| l | 2533.74 m |
| clearance OFF | 2533.75 m |
| formation join OFF | 2529.03 m |
| **follower gate OFF** | **1317.50 m** |
| gate and join OFF (`nfjg`) | 2383.93 m |
| gate, join and clearance OFF (`nfjgc`) | 2567.69 m (k's distance; other details still moved) |

- **The large move is the gate with formation join ON.** With the gate OFF, the JM05 AI follow
  pass joins 3 followers instead of 8:
  - `summary mission ai follow ... joins=3` against `joins=8`;
  - the gate's `leaves=0` against `leaves=7`.
  The Phelps group forms differently.
- Clearance moves Phelps only on the gate-and-join-OFF base (2383.93 -> 2567.69 m).
- Both are ship-AI formation effects on an idle player unit. Neither needs a fix.
- Reference l's flag line is corrected in place.

## 55. The hull shape owners through the Note lookup (packet `cc9_note_owner_shapes`, cc9-gunnery11, 2026-09-29)

This follows up 53. **The owner test, read at `00938F61..0093918C`.** A pair `{item, node}` from
`model+4Ch` is kept when `node` equals one of four nodes:
- `[ESP+3Ch]`, the `firstnode` lookup at `00938F4E` (string `00D1968C`), compared at `00939026`;
- `[ESP+8Ch]`, `model+0Ch` (the root), at `00939039`;
- `controller+370h`, `0071AD50(model, "front")` (`00D196A0`) at `00938DB9`, compared at
  `00939053`;
- `controller+374h`, `0071AD50(model, "back")` (`00D19698`) at `00938DE2`, compared at `0093906C`.

`0071AD50` returns the node a Note of that exact name was published with. B891A0 publishes each
hierarchy record's items with that record's own node (NATIVE_MODEL_GRAPH_AQ, section 49.3), so:

**the kept shapes are the ConvexObjects of record 0 and of every record that lists a Note named
`firstnode`, `front` or `back`.**

- The loop at `00938E04..00938F17` is not an owner. It looks up `hajobelso` (`00CEB8F4`) through
  `0071BA20` and moves each match; it does not take part in the walk's test.

**The census** is `local\g11_notebox.py`, read-only on this installation's ship models; its output
is `local\g11_notebox.txt`.
- No model has a `firstnode` Note.
- Of the 40 class models whose record 0 lists no ConvexObject (49.5), **39 get shapes through their
  `front`/`back` records.** The `eleje` (front) and `hatulja` (back) items carry the hull.
  Examples, with shapes and raw extent:

  | model | shapes | extent |
  | --- | --- | --- |
  | akagi | 2 | 30.07 x 43.75 x 261.82 |
  | yamato | 6 | 37.12 x 44.37 x 254.48 |
  | i-400 | 4 | 13.49 x 17.85 x 126.80 |
  | porter | 10 | 11.45 x 32.62 x 113.09 |
  | us_troop_transporter | 2 | 27.20 x 42.58 x 181.50 |
  | soryu | 4 | 33.33 x 26.95 x 225.40 |
  | jap_tanker | 5 | 23.35 x 29.70 x 167.88 |
  | lst_mark5 | 3 | 14.93 x 15.82 x 97.89 |

- **Only saratoga keeps the empty set,** and so the zero box of 49.10.
- **Five models with a root shape also gain Note-owned shapes,** which widens their box:
  hospital_ship (3 + 4), jap_troop_transporter (1 + 5), dzsunka-big, dzsunka-little and
  pt_boat_camo (1 + 1 each).
- Every owner record's composed matrix leaves the box unchanged. The posed box equals the raw box
  in all 239 ship files.

**The reader** (`read_mmod_hull_convex_box`, `fd2e7ccd1`) now keeps record 0 plus every record
listing such a Note, and counts the latter in `note_owner_records`.
- DeRuyter's fixture box is unchanged: it has no front or back Note, and the existing test passes.
- A 300-frame check with `kHullInertiaFromShapesBound` flipped (`local\g11nb_*`, not committed)
  logs the census's boxes plus the 0.02 widening on each side:
  - Oglala (us_troop_transporter): shapes=2, 27.24 x 42.62 x 181.54;
  - Convoy1 (jap_troop_transporter): 6 shapes;
  - Hospital Ship 01: 7 shapes;
  - PT1: 2 shapes;
  - LST8 (lst_mark5): 3 shapes;
  - MovieCargo (jap_cargo): 4 shapes.
- The units host's `hull shapes` line still says "root ConvexObjects". That wording is in the units
  host (lua12's lane) and is now inexact.

**Consequence for the inertia switch (still OFF).**
- 52.3's pairs gave zero inertia to every zero-root class. On JM06 that is the transports and
  tankers, and on USN02 none of the listed classes.
- With this reader they get a box. A re-pair of 52's USN02 and JM06 rows would move JM06's
  transports, which 51.1 recorded as unmoved.
- This does not change 52.3's verdict. The capsize prediction is still the blocker.

**Uncertainty.**
- The shape's node transform is not modelled. It is identity for every owner record here.
- The periscope shape (49.10) is still not added. It is a separate `0071AD50("periszkop")` at
  `009396D8`, and 53 shows it matches 12 submarine models.

## 56. The wreck capsize, predicted from the flooding trace (packet `cc9_hull_capsize`, cc9-gunnery11, 2026-09-29)

This is 52.3's next step. The inputs are cc9-lua12's `BSP_HULL_FLOODING_TRACE=1` (`9ff6f59a8`: each 0.5 s
per ship, the leak water `unit+10FCh`, and each leak's hull-space point and water). It was run
together with `BSP_HULL_ATTITUDE_TRACE=1` on this tree's `86e458a5e` with the inertia switch OFF,
on USN02 9200/9000 and JM06 3200/3000 (`local\g11cpoff_<row>.log`). The predictions below were
written before any ON run of this packet.

### 56.1 The model

`local\g11_capsize.py` is a quasi-static roll balance per trace sample:
- **Elements.** Built from 52.1's reading and the `buoyancy elements` class line: `2 * Segments`
  elements at x = +/-W/2, each with `c = 5 M / (Segments * D)`.
  - The deck point is at local y = S - D, with `S = D / (1 - WaterLineRatio)`, the deck-to-keel
    span.
  - Every station gets the same deck and keel. This is an approximation; DeRuyter's stations
    differ by 0.3 m.
  - The depth is `d = S - clamp(h, 0, S)`, and `B = c d (0.5 d / D + 0.5)`.
- **Heave.** For a heel phi, the heave solves `sum(B) = 10 (M + water)`.
- **Moments.** The roll moment is the flat-arm buoyancy term `sum(x cos(phi) B)` plus `0074F2E0`'s
  leak term, `-10 sum(w (x cos(phi) - y sin(phi)))`.
- **Solve.** The heel is scanned from 0 toward the leak's side in 0.5-degree steps. The first zero
  crossing is the equilibrium. **No crossing within 90 degrees means capsize:** the flooding moment
  exceeds anything the bounded buoyancy moment can return.
- **What the trace adds** to 52.3's reading:
  - Every leak point in these rows is at local y = 0, so the `y sin(phi)` term is zero.
  - **The leak points sit at x = +/-Width**, twice the elements' lever arm (for example John1's
    leaks at x = -9.00 on a 9 m hull).
  - So the flooding moment grows as 10 * w * W, while the buoyancy moment is bounded by
    (W/2) * sum of one side's full buoyancy.
- **Not modelled:**
  - pitch: Houston floods at the bow, `+z`;
  - waves;
  - the hit roll torques;
  - dynamics. The wreck is overdamped (zeta 2.4 to 6, 52.1), so its roll lags the equilibrium.
- The submarines have a buoyancy mix of 0 and `mul` (1, 1, 1). The model uses the Ship material,
  so the submarine rows are indicative only.

### 56.2 Predictions from the OFF run (inputs only; OFF itself never heels)

USN02:

| wreck | wreck at | phi_eq passes 30 deg (t, water) | balance lost (t, water) | lost - wreck |
| --- | --- | --- | --- | --- |
| Houston | 21.0 | - | 39.5, 6188 | 18.5 s |
| John1 | 26.5 | - | 48.0, 882 | 21.5 s |
| Kawakaze | 51.0 | 60.5, 841 | 62.5, 958 | 11.5 s |
| Tokitsukaze | 95.0 | - | 124.0, 2416 | 29.0 s |
| Yamakaze | 120.0 | - | 136.5, 1360 | 16.5 s |
| Amatsukaze | 125.5 | 141.5, 1200 | 148.5, 1527 | 23.0 s |
| Asagumo | 139.5 | 152.5, 1248 | 156.5, 1541 | 17.0 s |
| Hatsukaze | 150.5 | - | 193.5, 4371 | 43.0 s |
| Yukikaze | 160.0 | - | 180.5, 2723 | 20.5 s |
| Minegumo | 185.5 | 201.0, 1203 | 207.0, 1529 | 21.5 s |

- **Flooded living hulls hold small, steady heels.** The heel direction is toward their leaks'
  side:
  - DeRuyter -2.5 to -4.0 degrees;
  - Java +4.0 to +5.5;
  - Samidare +3.5 to +4.0;
  - Kortenaer, Electra, Haguro, Yudachi, Murasame and Harusame within 2.5 degrees.
- On JM06:
  - Fletcher-class 08 settles at +2.5 to +3.0 degrees;
  - USTroopTransport 02 reaches -5.5;
  - the other transports stay within 2 degrees;
  - the Gato wreck (a submarine, indicative only) loses balance at 36.5 s.

**The predictions for the ON pair** (`kHullInertiaFromShapesBound` flipped on the same commit,
both traces on). The model is re-evaluated on the ON run's own flooding inputs, because heel
changes the hits and so the water:
- **P5, wreck capsize onset.**
  - Every USN02 wreck passes 60 degrees of roll only after its ON water reaches 0.8 times the water
    at which the model loses balance on that run's own samples.
  - It does so within 25 s of the model's loss time.
  - The roll goes to the side the leak moment drives.
- **P6, the heel before loss.**
  - While the model has a balance, a wreck's roll stays within 8 degrees of the model heel at 90%
    of its samples. The margin allows for the overdamped lag.
  - On living flooded hulls, the roll stays within 3 degrees of the model heel at 90% of the
    samples with water above 0. The hit roll torques are ON as landed, and their transients are
    what the 10% allows.
- **P7, the sign.** Every hull whose model heel exceeds 2 degrees rolls to the same side.
- **Mechanism failure:**
  - a wreck passing 60 degrees below 0.8 times its loss water;
  - a living hull rolling against the model's side by more than 2 degrees for more than 5 s.
- **Uncertainty:**
  - uniform stations;
  - no pitch coupling. Houston's bow flooding could pitch it instead of rolling it.

### 56.3 The ON pair: the predictions miss, and the cause is a host layout bug in the leak moment

**The pair.** ON is `local\g11cpon_<row>.log` (`kHullInertiaFromShapesBound` flipped on `86e458a5e`, both
traces on, the hit roll torque ON as landed). The USN02 gameplay numbers repeat 52.3's pair B:
hit records 2271 -> 4203, damage 39395.6 -> 34679.6, and ten death rows, all changed.
`local\g11_capeval.py` re-runs the 56.1 model on the ON run's own flooding samples.

**P5, wreck onset: 6 of 10 held.**
- **Held:** Yamakaze, Minegumo, Yukikaze, Tokitsukaze, Amatsukaze and Hatsukaze. Each passes
  60 degrees at 1.4 to 2.4 times the model's loss water, 12 to 22 s after the model's loss.
- **Missed late:** Houston (2.63 times the loss water, 51 s after) and Kawakaze (2.81 times, 39 s
  after).
- **Missed early:** John1 (0.68 times the loss water, 7 s before) and Asagumo (0.57 times, 19.5 s
  before).

**P6, the heel before loss: failed.** The living flooded hulls sit far beyond the model heel:

| hull | samples within 3 degrees | model heel | observed |
| --- | --- | --- | --- |
| DeRuyter | 45 of 760 | about -2 to -4 degrees | -16.6 max, -9.3 at the end |
| Haguro | 120 of 668 | - | 27.0 max |
| John2 | 25 of 664 | - | 23.9 max |

**P7, the sign: failed** on Kortenaer (90 samples against), Electra (209), Jintsu (233),
Fletcher-class 08 on JM06 (84), and on the wrecks John1, Asagumo and Tokitsukaze.

**Mechanism failure, by 56.2's own criteria.** The static check (`local\g11_capdbg.py`) pins it down.
DeRuyter at 439.97 s is steady at roll -9.37 degrees and y = -0.58, with 1691 water in six leaks.
At that pose the model's moments do not balance:
- the buoyancy moment is +253,897 (+x side depth 7.04, -x side 4.44);
- the leak moment is -102,078.
Something adds about -1.5e5 that the model does not have.

**The cause: the units host hands `0074F2E0` a 3x3 array where it reads a 4x4 block.**
- `unit_leak_torque_0074f2e0` (src/unit_forces.cpp) reads the pose rows with the image's stride of
  16 bytes. It reads indices 0, 2, 4, 6, 8 and 10 as +CCh, +D4h, +DCh, +E4h, +ECh and +F4h: the
  x and z columns of rows 0, 1 and 2.
- The host (`leak_heel_torque_0074f2e0` in src/game_hosts_units.cpp, about line 6465) passes
  `float rows[9]`, packed three per row. So the routine reads:
  - "row1.x" as row1.y (about 1 on an upright hull);
  - "row1.z" as row2.x;
  - "row2.x" as row2.z;
  - "row2.z" from `rows[10]`, **past the end of the array** (undefined behaviour, stack contents).
- The roll-producing term becomes `-10 w (p.x row0.x + p.y row1.y + p.z row2.z)`. **The leak's
  longitudinal position (+/-85.5 m on DeRuyter) now acts as a lateral lever.**
- On DeRuyter at 440 s, `sum(w z)` is +25,300 against `sum(w x)` = +10,350. The spurious term is
  about 2.4 times the real one, the size of the missing moment.
- Bow and stern flooding therefore rolls a hull, which is why the sign fails on hulls whose
  bow/stern imbalance opposes their side imbalance.
- **With the inertia switch OFF this has no gameplay effect**: a zero inverse inertia discards
  every torque. It has been there since `ceed0a6ec`.

**The fix is in the units host** (cc9-lua12's lane), sent to the integrator. It passes the
image's layout:

```
float rows[12] = {
    slot_.motion.pose_row0[0], slot_.motion.pose_row0[1], slot_.motion.pose_row0[2], 0.0f,
    slot_.motion.pose_row1[0], slot_.motion.pose_row1[1], slot_.motion.pose_row1[2], 0.0f,
    slot_.motion.pose_row2[0], slot_.motion.pose_row2[1], slot_.motion.pose_row2[2], 0.0f};
```

**Verdict: `kHullInertiaFromShapesBound` stays OFF.**
- The capsize model is not refuted. It could not be tested against a host whose leak moment is
  wrong, and the P5 rows that held include wrecks flooding mainly on one side.
- **Next:**
  - land the layout fix; it is inert while the switch is OFF;
  - re-run this pair on the fixed build;
  - re-evaluate with `g11_capeval.py`.
  56.2's predictions stand as written for that re-run.

### 56.4 P2 scoped: where the hit roll torques land (cc9-gunnery11, 2026-09-29)

This packet adds a diagnostic, `BSP_HULL_ROLL_TORQUE_TRACE=1` (`9e9703392`, gunnery host), which logs
each posted 93h torque. The run is `local\g11rt_<row>.log`: `9e9703392` with the inertia flip, the
attitude trace on, and the rows bug of 56.3 still present.

**USN02 has 19 torques.** 7 of them, and every one above 5e5, fall on Houston and John1:

| t | victim | dead | size |
| --- | --- | --- | --- |
| 18.95, 20.60 | Houston | alive | 4.04e6 each |
| 21.65, 22.40, 38.40 | Houston | dead | 4.04e6 each |
| 26.05 | John1 | alive (dies at 26.5) | 1.36e6 |
| 32.90 | John1 | dead | 1.36e6 |
| 100.75, 136.25, 137.85, 138.40 | Yudachi | alive | 1.4e5 to 1.6e5 |
| 127.85, 129.20 | Yamakaze | alive, then dead | 1.4e5 |
| 142.30 | Haguro | alive | 4.8e5 |
| 196.06 | Yukikaze | dead | 1.9e6 (the friendly torpedo) |
| 396.13 to 397.93 | Samidare | alive (4 torques) | 1.5e5 to 1.7e5 |

JM06 has one torque: USTroopTransport 01 at 109.20 s, 4.5e5.

**Why 52.2's large destroyer kicks did not appear:**
- The 4e6 torques all fall on Houston. Its roll inertia (mul.z 2, Mass 11602, box 20.95 x 46.62)
  is 5.05e6, a kick of only 0.04 rad/s.
- The destroyer that takes a big one is John1, 0.45 s before it dies. Its roll steps from 0 to
  +5.9 degrees by the next sample, and +4.4 more at the 32.90 s torque. This is the early start of
  John1's capsize in 56.3.
- The living destroyers take only 1.4e5 to 1.7e5 torques.

**The size of John1's kick is not settled.**
- 52.2's formula (`tau * 0.05 / I_z` with I_z = 48,200, peak about kick / omega / 2) gives about
  25 degrees. The trace shows about 6.
- The 93h delivery (`0092BF30` -> `00C35330`, one step) or the damping may scale it. That is not
  read here.
- Measuring this cleanly waits for 56.3's rows fix, because the flooding moment on John1 is wrong
  until then.

### 56.5 The re-pair after the rows fix: 56.2 holds except P5's time window; verdict ON

**The pair.** Both sides are exports of `5e139bd73`, which carries main `c12ba1d8f`: cc9-lua13's
`rows[12]` fix `9029526f9`. OFF is a clean export and ON flips `kHullInertiaFromShapesBound`. All
three traces are on (flooding, attitude, roll torque), on USN02 9200/9000 and JM06 3200/3000
(`local\g11c2<side>_<row>.log`). The switches newly ON since 56.3's base are the plane ground roll
and the ship-AI standoff target kind; both sides have them.

**Gameplay:**
- USN02: exit 3. Deaths 10 -> 11: John2 dies at 197.31 s, to John3's blast (friendly, category 7,
  12 hits). Hit records 2271 -> 2009, shots 2316 -> 2133, damage 39395.6 -> 38828.3.
- JM06: exit 3. The death row is identical. Hit records 276 -> 220, damage 4340.0 -> 4399.2.
- 52.3's hit-record rise (2271 -> 3963 / 4203) was the rows bug's rolled hulls. It is gone.

**Scored against 56.2 as written** (`local\g11_capeval.py` on the ON logs):
- **P6, the heel before loss: held.**
  - Every living flooded hull on USN02 is within 3 degrees of the model heel at 100% of its samples.
    The only exception is Yudachi, at 809 of 816.
  - Examples, max and final roll: DeRuyter 3.5 / -2.4, Java 5.0 / 2.9, Kortenaer 2.7 / -0.2,
    Haguro 3.8 / -1.4, Murasame 5.8 / -4.2.
  - On JM06: Fletcher-class 08 3.2 / 3.1, USTroopTransport 01 9.5 (125 of 156 within 3 degrees).
  - The wrecks are within 8 degrees at 50 to 100% of their samples before loss. Houston is lowest
    (17 of 63; it floods at the bow and pitch is not modelled), then Amatsukaze and Hatsukaze
    (overdamped lag).
- **P7, the sign: held.** No living hull rolls against the model. John1 has 6 samples against
  while wrecked.
- **P5, the wreck onset: 7 of 11 held.** Held for John1, Yamakaze, Minegumo, Yukikaze, Kawakaze,
  Amatsukaze and Hatsukaze.
  - **All four misses are late, never early:**

    | wreck | after the model's loss | water ratio |
    | --- | --- | --- |
    | Houston | 51 s | 2.63 |
    | Asagumo | 33.5 s | 1.85 |
    | Tokitsukaze | 26.5 s | 1.62 |
    | John2 | 27 s | 1.86 |

  - Every wreck passes 60 degrees at 1.6 to 2.6 times its loss water, and none below 0.8.
  - So the 25 s window was too tight for an overdamped wreck (zeta 2.4 to 6). The mechanism, a
    bounded flat-arm moment against an unbounded one-sided leak moment, holds.
- **New: every wreck now comes to rest at about 90 degrees** (Yamakaze, Minegumo, Yukikaze and
  others end at 90.0) instead of rolling to 180 as in 52.3.
  - At 90 degrees both roll moments carry `cos(phi) = 0`, the element arms and the leak arms
    alike. So a wreck lies on its side and sinks.
  - The 180-degree capsizes of 52.3 came from the rows bug's longitudinal lever, which does not
    vanish at 90 degrees.
- **P1 (52.2), the living hulls: held.**
  - Undamaged hulls stay at 0 (Alden 0.3, John3 1.1, USTroopTransport 03 1.4).
  - Damaged living hulls stay within 6 degrees, apart from USTroopTransport 01 at 9.5, and follow
    their flooding quasi-statically.

**Mechanism-failure criteria (56.2): none met.**
- No wreck passes 60 degrees below 0.8 times its loss water.
- No living hull rolls against the model by more than 2 degrees.

**Verdict: ON**, with the spread miss recorded (P5's 25 s window: four wrecks late by 26.5 to 51 s).
The switch lives in the units host (src/game_hosts_units.cpp, `static constexpr bool
kHullInertiaFromShapesBound`). The flip is routed through the integrator.

**What moves with the flip:**
- Every hull whose class has a box now has inertia. Since 55, that is every class model except
  saratoga.
- Damaged hulls list toward their flooded side.
- Wrecks roll onto their side as they sink.
- The reference rows' plane and ship deaths move with the changed hull poses (USN02 gains John2's
  death). The next reference rebaseline has to absorb this.

## 57. Handoff (cc9-gunnery11, 2026-09-29, at about 70% context)

### 57.1 What this worker landed

| where | packet | state |
| --- | --- | --- |
| `GameGunneryHost::landscape_segment_hit_00904400` | `cc9_landscape_segment_query` | added (`e3ec96283`); used by cc9-ships11's retarget arm |
| GAME_EXECUTABLE reference l, `reports/cc9_reference_rebaseline_12.json` | `cc9_reference_rebaseline_12` | 13 rows on `3f1499210`; 16 switches attributed |
| GAME_EXECUTABLE reference m, `reports/cc9_reference_rebaseline_13.json` | `cc9_reference_rebaseline_13` | 16 rows on `b234f20ac`; 8 switches attributed |
| 52 | `cc9_ship_motion_hydro` | the hydro torque read; first pair OFF (the flooding list found) |
| 53, 55 | `cc9_name_lookup_key_form`, `cc9_note_owner_shapes` | `0071AD50` matches Note names; the reader keeps the front/back Note records (`fd2e7ccd1`) |
| 54 | - | reference l's two flags explained |
| 56, 56.3 to 56.5 | `cc9_hull_capsize` | the capsize model; the leak rows bug (fixed by lua13 in `9029526f9`); re-pair **verdict ON**, flip routed |
| diagnostics | - | `BSP_HULL_ATTITUDE_TRACE` (mission frame), `BSP_HULL_ROLL_TORQUE_TRACE` (gunnery host) |

### 57.2 Open items

- **The flip of `kHullInertiaFromShapesBound`** (56.5) is routed to the units host's owner.
  Reference n has to absorb it:
  - USN02 gains John2's death;
  - the hit records fall on USN02 and JM06;
  - `kShipAiBigLandingShipBound` and later switches are also post-m.
- **P5's time window** (closed as explained in 58: no missing pitch term). Four wrecks go over
  26.5 to 51 s later than the quasi-static loss. A dynamic model would need:
  - the overdamped roll (wreck damping 2.5, inertia x2);
  - pitch, for Houston's bow flooding.
- **P2's kick size** (closed in 59: the kick landed on the wreck). John1's 1.36e6 torque gives about 6 degrees against 52.2's formula of about
  25 (56.4). Read the 93h delivery (`0092BF30` -> `00C35330`) and re-measure on a flipped build.
- **P4: closed as moot** (integrator, 2026-09-29). 52.3's hit-record rise was the rows bug, and no packet is planned. On the fixed pair, hit records FALL
  (2271 -> 2009, 276 -> 220). How a heeled hull's segment boxes change the impacts is still not read.
- **The periscope shape** (49.10, 53). `0071AD50("periszkop")` at `009396D8` matches 12 submarine
  models. It is not added to the hull box.
- **Reference m's flags:**
  - the no-ship hold moves neither USN02 nor LOMP10 on m;
  - the LOS role swap's USN01 shots are not visible on m;
  - the no-ship hold and the sight test interact on USN01.
- **The units host's `hull shapes ... root ConvexObjects` log wording** is inexact since 55. It is
  routed.

### 57.3 Tools in the cc9-gunnery11 tree (`local\`)

- **Runs:**
  - `g11_runs.ps1 -V <prefix> -Only <rows>` runs the reference rows. They include `usnos`, `usnosl`
    and `ijn01`, plus `smoke`. The binary is `local\<prefix>\build\...`.
  - `g11_wait.ps1`, `g11_waitv.ps1` and `g11_waitm.ps1` are foreground waits on the final COM
    release.
- **Exports:**
  - `g11_exp.ps1 -Commit <sha> -Specs 'name:kA=true,kB=false'` builds a pair export;
    `g11_loo.ps1 -Specs` is the same with switches OFF against a fixed base.
  - `g11_wexp.ps1 -V <names>` waits for those builds.
- **Reference tables:**
  - `g11_switches.py <base> <head>` gives the switch value diff.
  - `g11_vs.py`, `g11_vsk.py`, `g11_matrix.py`, `g11_deaths.py` and `g11_sumdiff.py` are
    `pair_diff` views: headlines, the death-row membership, and the summary lines.
  - `g11_rows.py`, `g11_table*.py` and `g11_report2.py` / `g11_report3.py` build the reference
    tables and reports.
- **Hulls:**
  - `g11_att.py <log>` summarises the attitude trace.
  - `g11_pred.py` gives the per-class roll and pitch omega and zeta.
  - `g11_capsize.py <flood log>` is the quasi-static capsize model.
  - `g11_capeval.py <ON log>` scores 56.2.
  - `g11_capdbg.py <log> <ship> <t>` gives the moments at an observed pose.
- **Models:**
  - `g11_notes.py <dir> <names>` scans Note names.
  - `g11_notebox.py [models]` is the shape owner census (`g11_notebox.txt`).
  - `g11_keys.py <call sites>` gives the literal keys passed to `0071AD50`.

## 58. The late wrecks and pitch: no missing term; P5's four misses explained (packet `cc9_wreck_pitch`, cc9-gunnery12, 2026-09-29)

This packet takes 57.2's item "the late wrecks need a dynamic model with pitch". It adds no
switch, so it has no pair: the host already carries every pitch term the image has. The four late
misses are explained from 56.5's verdict run (`local\g11c2on_usn02.log` in the cc9-gunnery11
tree, both traces on). Tools: `local\g12_pitch.py <log>` (per hull: the leak moments'
lateral and longitudinal parts, max pitch) and `local\g12_pitchat.py <log> ship:t,...` in the
cc9-gunnery12 tree.

### 58.1 The image's pitch terms, and the host's

Pitch is the rotation about row 0 (the lateral axis). The image has three sources of pitch
moment, and the host reproduces each:
- **The element buoyancy.** `00932D02` flattens only the element's local y before the arm is
  taken (`00932D07`), so the arm keeps the station's z. `arm x (0, B, 0)` (`009339A7`) therefore
  pitches the hull by `z B`. This is the pitch stiffness `k_p` of 52.1. Host:
  `ship_hydro_apply_forces_009329c0` (src/ship_hydro_forces.cpp) takes the same flattened point.
- **The leak weights.** `0074F2E0` reads each leak point `p` as y (`[ECX+4]`), x, z
  (`0074F387..0074F39D`), and the cached world rows at unit+CCh at columns 0 and 2
  (`[EAX]`, `[EAX+10h]`, `[EAX+20h]` and `[EAX+8]`, `[EAX+18h]`, `[EAX+28h]`). Its out.x is
  `10 w r.z`, the bow/stern lever. This is `r x (0, -10w, 0)` complete: the y column is not needed.
  Host: `unit_leak_torque_0074f2e0` (src/unit_forces.cpp), with 56.3's `rows[12]` fix.
- **The inertia.** `00939A8E`'s box gives `I_x = mul.x M / 12 (H^2 + L^2)` with `mul.x = 1`. The
  wreck block `00824FE5` doubles all three axes and sets the angular damping to 2.5 for every axis.
  Host: `ship_hull_inertia_00939a8e` (src/ship_hull_body.cpp) and
  `ship_wreck_sink_block_00824fe5` (src/game_hosts_units.cpp).

**No other pitch terms.** `009329C0` has two more that act on pitch, and neither applies to a
destroyer or cruiser:
- the planing torque `0093380A`, for material TBoat only;
- `0092E8C0`'s pitch righting `0092E9E1`, for units answering IsKindOf(0Eh) (TorpedoBoat) only.

The steering routine keeps the row-0 rate for every other hull (52.1). Nothing in the wreck path
after `00824FE5` scripts a plunge: `sinkTime +828h` only times the 60 s kill-depth rule.

**So the image trims and plunges a bow-flooded hull through the same torques the host has.**
Uncertainty: this is read from the listing, not from the image at run time (the original exe is
not run).

### 58.2 Houston was not bow flooded in the verdict run

56.1's "Houston floods at the bow" came from the OFF run `g11cpoff`. There, its leak moment was
`sum(w z) / water = +56` m of a 90 m half-length.

In 56.5's ON run Houston floods amidships:
- at 60 s, leak #4 (+16, 0, 0) holds 7803 of its 8086 water;
- `sum(w z) / water = +0.46` m;
- it never pitches past 0.1 degrees.

Its 51 s lag is the overdamped roll alone. With the equilibrium lost, the wreck's roll rate is
`tau_net / (5.34 * 2 I_z)`, the 52.1 damping rate against the doubled 56.4 inertia
(`I_z = 5.05e6`). `tau_net` comes from `g11_capdbg.py` at the observed pose:

| t | roll | tau_net (N m) | predicted rate | observed rate (t +/- 1 s) |
| --- | --- | --- | --- | --- |
| 50.5 | -10.6 | -5.57e5 | 0.59 deg/s | 0.55 deg/s |
| 75.0 | -27.5 | -9.42e5 | 1.00 deg/s | 1.00 deg/s |
| 90.0 | -45.6 | -1.23e6 | 1.31 deg/s | 1.30 deg/s |

Houston has the largest roll inertia and the lowest roll stiffness of the USN02 classes
(omega 0.61, wreck zeta 6.18 in 52.1). That is why it creeps longest: from 10.6 to 60 degrees
takes 51 s at 0.6 to 1.3 degrees a second.

### 58.3 The other three late wrecks are bow plungers

The pitch column is `asin(row2.y)` from `BSP_HULL_ATTITUDE_TRACE`; negative is bow down.

| wreck | P5 | `sum(w z)/water` (half-length) | at the model's loss: pitch, y | at roll 60: pitch, y |
| --- | --- | --- | --- | --- |
| John2 | late 27 s | +40.6 (48) | -10.0, -6.4 | -44.8, -53.9 |
| Asagumo | late 33.5 s | +44.3 (59) | -14.8, -12.0 | -64.7, -118.1 |
| Tokitsukaze | late 26.5 s | +57.0 (59) | -20.5, -13.7 | -64.3, -71.6 |
| Yukikaze | held | +46.2 (59) | -3.9, -2.4 | -16.1, -10.9 |
| John1 | held | -23.8 (48) | +1.7, -1.5 | +13.1, -13.6 |
| Hatsukaze | held | +33.4 (59) | -1.9, -1.5 | -6.0, -3.2 |
| Yamakaze, Minegumo, Kawakaze, Amatsukaze | held | +20 or less | -0.9 or less, -2.3 or more | -2.8 or less |

- **Every late plunger was already 10 to 20 degrees down by the bow, and 6 to 14 m under, at
  the model's loss time.** Every held wreck was within 4 degrees and 2.5 m.
- By the time their roll reaches 60 degrees they stand 45 to 65 degrees on their bows and are
  54 to 118 m deep. P5's "roll passes 60 degrees" is then a rotation about a near-vertical long
  axis, not a capsize at the surface.
- Their lateral leak moment is small (`|sum(w x)| / water` 1.0 to 1.6 m, against leak points at
  +/-9 to 10 m), so the
  flooding goes into pitch first. The roll-only model (56.1) has no term for that.

### 58.4 Verdict and what changes

- **No host term is missing, and no switch is bound.** The late misses are the mechanism the host
  shares with the image:
  - an overdamped heavy wreck that creeps, matched to 0.05 degrees a second;
  - bow plungers whose roll grows only after they are under.
- **P5 is closed as explained.** 56.5's ON verdict stands.
- A future capsize prediction should:
  - integrate the first-order roll `dphi/dt = tau_net / (5.34 * 2 I_z)` from the loss time
    instead of a fixed 25 s window;
  - exclude hulls whose `|sum(w z)| / water` exceeds half the half-length, or score them on pitch.

## 59. P2's kick size: the delivery adds nothing, and John1's kick landed on a wreck (packet `cc9_hull_kick_size`, cc9-gunnery12, 2026-09-29)

This packet takes 57.2's item "P2's kick size". John1's 1.36e6 N m torque gave about 6 degrees of
roll, against about 25 degrees from 52.2's formula.

### 59.1 The delivery, from the listing

- **`0092BF30` does nothing of its own.** It is two instructions, `MOV ECX,[ECX+2Ch]` and
  `JMP 00C35330`: the controller's hull body, then the body's AddTorque.
- **`00C35330` only adds.** It adds the vector into the motion state's torque accumulator `M+44h`
  (`RET 4`; docs/RIGID_BODY_INTEGRATION.md). There is no scale, gate or clamp on the way.
- **The integration.** `00C41550` does `w += (R^T diag(1/I) R) tau dt`, then `w *= 1 - c dt`.
  `00C5B1B0` rotates by `w dt`, then applies `w *= 1 - c dt` again.
- **The angular speed cap** `M+1Ch` is 1000 (`009392DB`), so it never binds.
- **The host's route is the same.** `GameUnitsHost::add_hull_torque_message_93h` calls
  `unit_handle_add_hull_torque_00822235` on the body, one step after the post. The image
  integrates at the next step's `00875E0C` (46.2).

**So the kick is `tau dt / I` about the forward row.** 52.2's formula is right for a living hull.

### 59.2 Why John1 gave 6 degrees: the torque came from its killing hit

In 56.5's verdict run (`g11c2on_usn02.log`, cc9-gunnery11 tree), everything happens at 26.05:
- the torque is posted with `dead=0`;
- the same impact (Yamakaze's torpedo blast, category 7) destroys John1's three hull segments;
- the death row and `entity dead: ... died=26.05` follow.

The wreck block `00824FE5` runs at that step's row-15 flush, after row 9's post. It doubles the
inertia and sets the angular damping to 2.5. The torque is integrated only at the next step, so it
acts on a wreck.

`local\g12_kick.py`'s discrete model (the order above) on John1 (Alden class: k = 1.19e5 N m/rad,
I_z = 48,180):

| model | at +0.45 s | at +0.95 s | peak |
| --- | --- | --- | --- |
| live (I_z, c = 1.0) | 21.8 deg | 23.2 deg | 24.5 deg at 0.70 s |
| wreck (2 I_z, c = 2.5) | 6.6 deg | 6.5 deg | 6.8 deg at 0.65 s |
| observed (0.5 s samples) | 5.84 deg (26.50) | 5.59 deg (27.00) | - |

The second torque, at 32.90 on the wreck, fits the same way: the wreck model peaks at 6.8 degrees
and the trace rises from -0.48 to +5.19 by 33.50.

**P2's 25 degrees was the live-hull figure applied to a kill.** In the image the order is the
same: the post is at row 9, the wreck block at row 15, the Dyn step at the next step's
`00875E0C`. So a killing torpedo's kick lands on a wreck in the image too.

Uncertainty: `00821E80`'s 93h arm is not re-read for a dead-unit gate. The host delivers to a
dead unit, and the trace shows the response.

### 59.3 Predictions for the per-step re-measure (written before the run)

The 0.5 s attitude samples cannot resolve a 0.7 s peak. This packet adds
`BSP_HULL_ATTITUDE_TRACE=2`, which prints the same line after every motion step and changes no
state. The run is one build: this branch's trace commit exported with
`kHullInertiaFromShapesBound=true`, on USN02 9200/9000, with the flooding, attitude (=2) and
roll-torque traces on.

**The torques will differ from 56.5's run**, because main has moved since `5e139bd73`. The
predictions are therefore per event.

`g12_kick.py` scores every torque of at least 1e5 N m on a hull whose class has a box and
elements. The observed excursion is measured from a baseline fitted linearly over the second
before the post.

- **P2a, living hulls.** For each event with no other torque on the same hull within 3 s:
  - the observed peak is 0.6 to 1.05 times the model's live peak;
  - it comes within 0.3 s of the model's peak time.
  The bias should be below 1, because the element drag on the rolling hull (`omega x r` in the
  drag, 52.1's "about 0.3/s") is not in the model.
- **P2b, kill kicks.** An event whose victim is a wreck at the next step meets the same band
  against the wreck model, and falls below 0.5 times the live model.
- **Mechanism failure:**
  - an isolated event above 1.2 or below 0.4 times its model;
  - a kill kick that matches the live model better than the wreck model.

### 59.4 The per-step run: the kick is `tau dt / I`; living hulls respond at about 0.67 of the reduced model

**The run.** `local\g12k_usn02.log` in the cc9-gunnery12 tree: a `pair_export --commit 577713f70
--flip kHullInertiaFromShapesBound=true` build (`local\g12k`), USN02 9200/9000 with m's launch
form, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`, `BSP_HULL_ATTITUDE_TRACE=2` and
`BSP_HULL_ROLL_TORQUE_TRACE=1`. A 300-frame USN01 smoke ran first. The run repeats 56.5's ON run: its 39 torque lines
(victim and time) are identical, and so is the gunnery summary (hit records 2009, deaths 11,
damage 38828.3). All 39 torques are at least 1e5.

**How the scoring window was chosen.** The first scoring took the largest deviation within 4 s of
the post. Most events then peaked at the 4 s edge, because a torpedo hit also opens a leak and the
flooding heel drifts. The score in the table below is therefore limited to the model's peak time
plus 0.3 s, the window 59.3's timing clause allows. This choice was made after seeing the run.
`g12_kick.py --wide` reproduces the 4 s figures.

**Isolated events** (no other torque on the hull within 3 s):

| t | victim | model | model peak (t) | observed (t) | ratio | 59.3 |
| --- | --- | --- | --- | --- | --- | --- |
| 26.05 | John1 (kill) | wreck | 6.78 (0.65) | 5.91 (0.55) | 0.87 | held; live model 24.5, ratio 0.24 |
| 32.90 | John1 | wreck | 6.78 (0.65) | 6.52 (0.60) | 0.96 | held |
| 38.40 | Houston | wreck | 0.21 (0.95) | 0.22 (1.25) | 1.08 | above 1.05 |
| 196.01 | Yukikaze | wreck | 2.81 (0.75) | 2.79 (0.80) | 0.99 | held |
| 100.85 | Yudachi | live | 2.08 (0.80) | 1.34 (0.55) | 0.65 | held |
| 250.36 | Yudachi | live | 1.98 (0.80) | 1.70 (0.75) | 0.86 | held |
| 230.96 | Jintsu | live | 0.65 (0.70) | 0.43 (0.45) | 0.66 | held |
| 261.06 | Jintsu | live | 0.60 (0.70) | 0.33 (0.40) | 0.55 | below 0.6 |
| 280.61 | Jintsu | live | 0.69 (0.70) | 0.43 (0.44) | 0.63 | held |
| 236.71 | Haguro | live | 0.25 (1.00) | 0.14 (0.50) | 0.57 | below 0.6; 0.5 s early |
| 424.12 | Murasame | live | 1.88 (0.80) | 1.38 (0.60) | 0.73 | held |
| 444.56 | Murasame | live | 1.78 (0.80) | 1.27 (0.65) | 0.71 | held |

- **P2a, living hulls: 6 of 8 in the band.** Jintsu 261.06 (0.55) and Haguro (0.57) fall just
  below it. The mean ratio is 0.67, and every living peak comes 0.05 to 0.5 s early.
  - That is the sign of more damping than the reduced model's `c = 1.0`, and the direction 59.3
    predicted: the element drag on the rolling hull is not in the model.
  - Its size is larger than the 0.6 floor allowed for.
- **P2b, kill kicks: held.**
  - John1 26.05 follows the wreck model (0.87 against 0.24 live).
  - Houston's kill kick at 20.60 is 1.04 of the wreck model and 0.23 of the live one, but it has
    other torques within 3 s.
  - John2's killing torque at 197.31 comes 0.75 s after a live one, and the linear baseline cannot
    separate the two. `local\g12_john2.py` runs both kicks as one trajectory:
    - the wreck-after-kill model adds 0.48 degrees from 197.26 to 197.71 and then creeps down;
    - the live model adds 1.61 degrees and falls back within a second;
    - observed: +0.48, from 2.23 to 2.71, then a slow decline (2.29 at 198.91; wreck model 2.47,
      live 1.47).
  - The wreck model fits, at about 0.85 of its amplitude.
- **Mechanism failure: none.**
  - No isolated event is above 1.2 or below 0.4.
  - No kill kick matches the live model.
- **Wrecks** sit at 0.87 to 1.08. At the wreck damping of 2.5 the drag's extra share is small.

### 59.5 Verdict and what changes

- **Nothing to bind.** The delivery `0092BF30 -> 00C35330` adds the torque unscaled, and the host
  matches it. P2 is closed.
- **John1's 6 degrees against 25** is the killing torpedo's kick landing on the wreck: x2 inertia
  and damping 2.5, from the same step's row-15 wreck block. It is not a delivery or damping error.
- **52.2's P2 figures are for living hulls, and overstate them by about 1.5.** The reduced model
  omits the element drag's roll damping. A 1e6 N m hit on a Kortenaer gives about 8 degrees, not 12.
- **Uncertainties:**
  - `00821E80`'s 93h arm has no re-read dead-unit gate;
  - the drag damping is inferred from the ratio and the early peaks, not computed;
  - the baseline is a linear fit and cannot separate overlapping torques (see John2).

## 60. Handoff (cc9-gunnery12, 2026-09-29, at about 75% context)

### 60.1 What this worker landed

| where | packet | state |
| --- | --- | --- |
| 58 | `cc9_wreck_pitch` | no missing pitch term; P5's four late wrecks explained (Houston's roll creep, three bow plungers) |
| 59 | `cc9_hull_kick_size` | the 93h delivery adds the torque unscaled; John1's kick landed on its wreck; living hulls respond at about 0.67 of the reduced model. `BSP_HULL_ATTITUDE_TRACE=2` (every motion step) |
| GAME_EXECUTABLE reference n, `reports/cc9_reference_rebaseline_14.json` | `cc9_reference_rebaseline_14` | 16 rows on `eb1226215`; 11 switches attributed (inertia: all 16 rows; landing group: LOMP10) |
| AA_LETHALITY_AUDIT 7 | `cc9_aa_lethality_audit` | census, listing read, predictions 7.4; **binding not landed** (see 60.2) |

### 60.2 Open items, in order

1. **Land the AA bot errors** (`cc9_aa_lethality_audit`, AA_LETHALITY_AUDIT 7.3 / 7.4).
   - The patch is `local\g12_aa_patch.diff` in the cc9-gunnery12 tree (288 lines, against
     `src/game_hosts_gunnery.cpp` and `include/bsp/game_hosts_gunnery.hpp`). It is not built.
     - It adds `kAaGunnerSwingErrorBound` and `kAaFlakAimErrorBound`, both OFF.
     - It adds the robots.lua rows and the `draw_normal` Box-Muller helper.
     - It adds the per-gun state, the flak `distErr` on the round (`GameProjectileRow::flak_dist_err`,
       added in the lock's remaining distance) and the `summary mission gunnery aa bot error` line.
   - Those files were leased to cc9-ships14 (`cc9_kaiten_contact_detonation`, until 20:38 UTC). The
     patch was written before a claim and reverted when the claim was refused. **Claim first,
     check the result, then `git apply`** (merge main first, then resolve any context drift by
     hand).
   - **Then:**
     - build (`scripts/build.ps1`) and run `tools/const_width_sweep.py --all --load-sites`;
     - commit OFF and run one 300-frame smoke;
     - run the 7.4 pairs: JM05, USN13 and USN04 E2 at 9200/9000, both switches ON on the ON side,
       RNG option on;
     - score P1-P4 and flip by verdict.
   - **Check while testing:**
     - `role_ai_held_00521e70(target, 0)` stands in for the image's `00521E70` on the target's
       slot;
     - `pitch` is taken off `want_vert` for the lead-gap term.
2. **Scene units' skill from `Skill` / `Crew`** (its own item, not bound).
   - `00822C20` at `008238B1..008238CB`: when `[unit+C0h]+4 == 1` (a scene-placed unit),
     `00927A80` reads the bag's `Skill`, else `Crew` through `006E6210`, else 1. It then calls
     `unit->vtable[128h](skill)`.
   - `006E6210` maps Crew (the `CrewXPLevels` enum in `universe/library/global.enums`: Rookie 0,
     Regular 1, Veteran 2, Elite 3) to SkillLevels. In single player: 0 -> 0 Stun, 1 -> 1,
     2 -> 2, 3 -> 5.
   - The host leaves `pilot_skill_index` at 1 until a `SetSkillLevel` call.
   - In JM05's scene, 42 land forts and 6 convoys are `Crew = Rookie` (Stun in the image), and
     the two US carriers are `Skill = SPVeteran`. `local\g12_scncrew.py <scn> [filter]` lists a
     scene's Crew / Skill.
   - It belongs in the units host (the skill store). The gunnery host only reads
     `units.skill_level`.
3. **USNOS long, from reference n's flags.** Damage 4138.4 -> 10900.7 while hit records fall
   2683 -> 1112, with the same 21 victims, all from the inertia flip. Not separated.
4. **The periscope shape in the hull box** (57.2), and **reference m's flags** (57.2).
5. **Reference o** will need `kPlaneGroundSteeringBound` and `kShipAiKamikazeAttackStepBound`,
   both ON after `eb1226215`, and whatever lands later.

### 60.3 Tools in the cc9-gunnery12 tree (`local\`)

- **Runs:**
  - `g12_runs.ps1 -V <prefix> [-Only rows]` launches the reference rows (plus `smoke`);
  - `g12_run.ps1` launches one traced USN02 run;
  - `g12_wait.ps1 -Logs <names>` is the foreground wait.
- **Exports:** `g12_exp.ps1 -Commit <sha> -Specs 'name:kA=false,...'` runs detached
  `pair_export`s.
- **Reference tables:**
  - `g12_vs.py <off> <on> [rows]` gives the `pair_diff` headlines (prefix `rb13` = reference m in
    cc9-gunnery11);
  - `g12_rows.py`, `g12_table.py`, `g12_deaths.py` and `g12_report.py` build the reference n
    rows, table, death-row diffs and JSON.
- **Hulls:**
  - `g12_pitch.py` and `g12_pitchat.py` give the leak moments and pitch;
  - `g12_kick.py` (`--wide` for the 4 s window) and `g12_john2.py` score the 93h kicks.

### 60.4 Status update (cc9-gunnery12, later on 2026-09-29)

- **60.2 item 1 is done.** The AA bot errors are committed OFF in `a3da8863f` (the patch applied
  cleanly on main `da90b8653`). The period uses the existing
  `gun_bot_lead_error_span_00902920`; 7.2's 0.6-1.6 s was wrong and is corrected in 7.5.
  - The 7.4 pairs ran, and the verdict is ON for both switches (AA_LETHALITY_AUDIT 7.5):
    - E2 is identical;
    - USN13's low aircraft losses fall from 33 to 14;
    - JM05 gets 2 torpedo-task releases, but its low losses stay at 12. That is P2's size miss.
  - **The next AA question** is why JM05's escorts still kill every torpedo plane. Candidates: the
    `vtable[100h]` hull-box point substitution, plane HP and armour against the MG class, and the
    IJN MG barrel counts.
- **60.2 item 2 is superseded** by lua16's `docs/SCENE_UNIT_SKILL.md` (main `da90b8653`). The binding
  is ON, and the Rookie -> Stun effect on JM05's forts is refuted: the library's group default
  `Skill = SPNormal` wins.
- **Reference o** also has to absorb these two switches.

## 61. The ship arm of UnitSetFireStance / UnitHoldFire (packet `cc9_ship_fire_stance`, cc9-gunnery13, 2026-09-29)

This is the ship arm left open by SHIP_AI_OPEN_ITEMS 49 (cc9-ships15). It is bound as
`kShipFireStanceBound` in `src/game_hosts_gunnery.cpp`, committed OFF. Ghidra was read, not
written. The names are hypotheses.

### 61.1 The image

- **The natives.** `UnitSetFireStance` `008A6490` and `UnitHoldFire` `008A6AC0` (stance 0) call
  `unit->vtable[114h]`. For a ship that is `0080E150`, the weapon director at `unit+738h`
  (`docs/WEAPON_DIRECTOR.md`). They then call `0071BE80` on it.
- **`0071BE80`** (disasm-raw, `0071BE80..`) asks two predicates before sending either message:
  - `vtable[24h]` at `0071BE8F`: the fire answer;
  - `vtable[28h]` at `0071BE9D`: the move answer.

  It then calls `vtable[40h]` with the fire answer and `vtable[44h]` with the move answer.
- **The ship director's slots.** The derived vtable `00D09F58` holds (read from the image):

  | slot | function | role |
  | --- | --- | --- |
  | `+24h` | `0071D560` | fire: true for stance 1 or 2 (`CMP 1 / CMP 2`, `RET 4`) |
  | `+28h` | `0071D580` | move: true for stance 2 or 3 (`CMP 3 / CMP 2`) |
  | `+40h` | `0071DA50` | 5Ah message, sub-kind 0 |
  | `+44h` | `0071DAD0` | 5Ah message, sub-kind 1 |
  | `+64h` | `00836210` | receiver for sub-kind 0: stores `+3Ch`; when fire is forbidden it drops the fire target (`0083622B`) |
  | `+68h` | `0071D5E0` | receiver for sub-kind 1: stores `+3Dh` |

- **The ship and squadron move predicates differ.**
  - The ship predicate answers move for stances **2 and 3**, so stance 0 holds a ship's move.
  - The squadron block's `0084D930` answers move for 0, 2 or 3.
- **The readers:**
  - the gunnery bridge `008624C0`: `+3Ch` sets every category except 7 and 8, and `+3Dh` goes to
    `this+7Ch` (`src/unit_gunnery_pass.cpp`);
  - the ship AI's auto-target gate `009F5610` (`[director+3Dh]` at `009F5614`).

### 61.2 The binding

**The gunnery host** (this packet, `src/game_hosts_gunnery.cpp` and its header):
- `set_director_fire_stance_0071be80(unit, stance)` stores the two answers per unit and always
  counts them.
- `director_allow_fire_3c(unit)` and `director_allow_move_3d(unit)` answer the stored values when
  bound, and 1 otherwise (`008363E0`'s default).
- With the switch ON, the bridge pass pushes `+3Ch` / `+3Dh` from the stored row.
- **Census:** `summary mission gunnery director stance sets=... (stance 0/1/2/3 ...)
  fire_forbidden_pushes=... move_forbidden_reads=... bound=...`.
- **Substitutions, labelled:**
  - the message takes effect at the unit's next bridge pass, not at the session's delivery row;
  - the `0083622B` fire-target drop belongs to the ship AI host's director fire target, and is
    routed.

**Routed to cc9-ships15** (through the lead):
1. **`src/game_hosts_script_orders.cpp`**, in `run_unit_set_fire_stance`'s non-squadron branch and
   `run_unit_hold_fire`'s last branch: for a ship,
   `units_.gunnery()->set_director_fire_stance_0071be80(row.unit_index, stance)`, with stance 0 for
   HoldFire. It replaces the `0071be80` / `0071bed0` unimplemented records.
2. **`src/game_hosts_ship_ai.cpp`**, in `selection_enabled()`: answer `[director+3Dh]` with
   `owner_.gunnery_draws->director_allow_move_3d(index_)` instead of `defaults.allow_move`, when the
   pointer is set.
3. **The `0083622B` drop:** when a stance forbids fire, release the director fire target
   (`kWeaponDirectorFireTarget`).

### 61.3 Where the reference rows call it

The census comes from the reference n logs (the `rb14_*` logs in the cc9-gunnery12 tree):
- **USN01** is the only row whose `UnitSetFireStance` reaches a non-squadron unit: 9 calls, all
  unimplemented.
- **USN04, E2 and USNOS** call it on squadrons only: 8, 8 and 1 calls, with no non-squadron record.
- **No row** reaches `UnitHoldFire`'s director arm.

**USN01's script.** This installation's `usn_1_marshall.lua` (2024-07-13):
- sets stance 0 at init on `Mission.BmdGroup` (Northampton, SaltLakeCity, Dunlap; line 296);
- sets stance 2 in `luaIntroMovie` (line 611), which the first think calls (`Mission.Started`,
  line 440);
- sets stance 2 again 3 s later in `secNarr` (line 640).

### 61.4 Predictions (written before any ON run)

The pairs are same-tree, with the switch flipped on the ON side and the routed edits in both
binaries.
- **P1, census.** USN01: `sets=9`, stances `3/0/6/0`. No other row counts a set.
- **P2, USN01 3200/3000.**
  - The three bombardment ships hold fire and move only from init to the first think, and no
    target is in range then (the first hit is at 51.45 s in reference n).
  - So the row is **gameplay-identical** (exit 0 or 1).
  - `fire_forbidden_pushes` is small (at most a few bridge passes) or 0.
- **P3, control (USN04 4700/4500).** Identical (exit 0 or 1).
- **Mechanism failure:**
  - a set counted on a row other than USN01;
  - a stance-0 row whose `+3Ch` stays 1 in the bridge pass after the set, seen as
    `fire_forbidden_pushes` = 0 while a bridge pass ran between init and the first think.

### 61.5 The pairs and the verdict: ON

**The callers.** ships15's ship-arm callers are on main (`f2b75abbd`):
- `UnitSetFireStance` / `UnitHoldFire` -> `set_director_fire_stance_0071be80`;
- `selection_enabled` -> `director_allow_move_3d`;
- the `0083622B` fire-target drop is skipped and labelled.

**The binaries.** Same-tree exports of `185d5e8fc`:
- `local\ctl` (control, `D9F51B79A945`);
- `local\ston` (`kShipFireStanceBound = true`, `685CD10E9477`).

A 300-frame USN01 smoke ran clean first. Every run presented its full frame count.

| row | pair_diff | census (ON) |
| --- | --- | --- |
| USN01 3200/3000 | **1** (gameplay identical) | `sets=9 (stance 0/1/2/3 3/0/6/0) fire_forbidden_pushes=6 move_forbidden_reads=4`. Units 43, 44 and 45 take stance 0 at init, then stance 2 twice |
| USN04 4700/4500 (control) | **1** | `sets=0` |

**Against 61.4:**
- **P1, the census: held**, exactly (9 sets, 3/0/6/0, none on USN04).
- **P2 and P3: held**, both gameplay-identical.
- **The mechanism acted.** Six bridge passes ran with `+3Ch` = 0 and four auto-target gate reads
  saw `+3Dh` = 0, all between init and the first think, before any target was in range.
- **Mechanism failure: not met.**

**Verdict: ON.**

## 62. The scene command's name lookup folds case (packet `cc9_scene_command_find_case`, cc9-gunnery13, 2026-09-29)

This was routed from cc9-lua17, whose finding is on main at `c6d14ae2f`.

**The image.**
- `SceneCommand::find_entity_by_name` (`src/game_hosts_commands.cpp`) stands for `0046AB48` ->
  `00925A90`.
- `00925A90` hands each registry entry to `009251F0` (`BSP_SceneNode_FindByQualifiedName`).
- That routine's name test at `0092521E` is `CALL 00438E10`, the null-guarded CRT `_stricmp` over
  the whole name (disasm-raw `00925216..00925223`). This is the same rule that
  `kFindEntityCaseInsensitiveBound` binds for the Lua `FindEntity`.

**The host** compared `unit.name == name`.

**The binding** is `kSceneCommandFindCaseInsensitiveBound`:
- an exact hit wins, as before;
- otherwise the first unit whose name matches under `_stricmp` answers, in unit order (the
  registry's walk order is not modelled, labelled).

The census line is
`summary mission scene command find lookups / exact / case_only`.

**Census.** The reference o rows were run on this tree's build with the switch OFF (logs
`local\fc_<row>.log`). On every row `lookups == exact` and `case_only = 0`:
- IJN01: 753;
- USN02: 1006;
- USN01: 149;
- JM06: 99;
- USN13: 27;
- USNOS and USNOS long: 17 each;
- LOMP06: 15;
- USN04 and E2: 12 each;
- JM08: 9;
- JM05: 3;
- BSM01, LOMP10, LOMP10 long and USN12: none.

No lookup misses at all, so the switch cannot move any reference row, and no pair was run.

**Verdict: ON.**
- The rule is the image's.
- It is post-o: it is on no reference-o binary. It is attributed in reference p, alongside
  `kFindEntityCaseInsensitiveBound`.

## 63. USNOS long: why the inertia flip raised damage while hit records fell (60.2 item 3, cc9-gunnery13, 2026-09-29)

**The question** (60.2 item 3, from reference n). On USNOS long, the hull-inertia flip
(`kHullInertiaFromShapesBound`, `091cea5cf`) moved:
- damage from 4138.4 to 10900.7;
- hit records from 2683 to 1112;
- the 21 victims not at all.

Reference o has 10635.1 and 1101.

**Where the damage lands** (`pair_diff` of reference m `rb13_usnosl` against o `rb15_usnosl`, the
unit table):
- **The ships.**
  - Portland1 takes 160 -> 2490 and Portland2 129 -> 3915.
  - NH takes 160 -> 237 while its hits taken fall from 1049 to 25, and it deals 434 -> 6035.
- **Damage control.** Water damage rises from 2493 to 7384, and element hits fall from 1666 to 199.
- **Geometry.** Each ship's nearest-unit distance goes from 39 m (NH), 130 m and 78 m (the
  Portlands) to about 600 m. Without inertia the three ships bunch; with it they keep station.

**Who hits whom.** Two traced runs of USNOS long cover all IJN ships, coastal guns, NH, the Portlands
and the `unit #2.x/#3.x` attackers (`BSP_AA_TRACE_UNIT`, names in `local\g13_trace_names.txt`). The
tally is `local\g13_ff.py`.

| run | NH -> Portlands | Portlands -> each other and NH |
| --- | --- | --- |
| o (`local\g13_trO_usnosl.log`, gameplay-identical to `rb15_usnosl`, exit 1) | **cat 4: 18 hits, 5408.4 applied** (Portland2 3519.8, Portland1 1888.6; the first at 227.76 s) | cat 3: 55 hits, 1233.6 applied |
| o with inertia OFF (`local\o_nhi`, `local\g13_trNhi_usnosl.log`) | cat 4: none. NH's cat 1 hits Portland2 276 times for 0.0 (armour) | 52 hits, 363.6 applied |

**The mechanism.**
- **The guns.** NH's category 4 mounts are device row 521, `Yamato_1945 18'' 3X`, HEAVYARTILLERY,
  in this installation's arcade `deviceclasses.lua`. Their second Bullet record is class 23 (V0
  300).
- **The targets.** NH fires them at the low attackers: 78 cat-4 shots, 30 at `unit #3.1` and 14 at
  `unit #2.4`, at 1100-1200 m with the barrels about 3 degrees up.
- **Friendly fire.** With the ships at station-keeping distance, the Portlands lie on those flat
  trajectories. The blasts and direct hits land on them and then flood them.
- **Why hit records fall.** The bunched fleet of the no-inertia run traded a thousand zero-damage
  MG hits among close neighbours.

**Is the friendly fire the image's?** As far as read, yes.
- **The line-of-fire predicate `0072CDD0`** is installed only for weapon kinds 1, 5 and 6
  (`00729560`, `00729588..00729595`; the host's `kAaLineOfFireBound` follows it). A HEAVYARTILLERY
  gun has no friendly line-of-fire test in the image.
- **The plane admission.** `008633D0` admits a plane when mask bit 0 is set, and the category
  masks start at 3 (`00862632`). Only the AA/flak list's bit 0 is rewritten, from `+221h`, so a
  category 4 gun may take a plane.
- **Unread:** whether the artillery sub-director's own pick admits aircraft. The host's cat-4 plane
  shots come from the same pass, so this is the one open link.

**Verdict:** no host divergence found. The damage rise is the correct inertia geometry exposing an
image rule (no friendly-fire check for heavy artillery) to a flat-firing mount. Nothing is bound.

**The unread link, answered (section 66).**
- **The image's pick never admits aircraft to a HEAVYARTILLERY gun.** Its preference row
  `00E098D8` holds no plane class (10h-17h).
- **NH was not firing at aircraft.** Its targets were the suicide boats (0Eh, `unit #2.x`) and the
  submarines (08h, `unit #3.x`), which that row admits.
- **So there is no divergence, and no binding is queued.**

## 64. The kill handlers: what they do beyond the physics (ranking #12, cc9-gunnery13, 2026-09-29)

**Sources.** Ghidra was read, not written. The disasm-raw bodies are the disk bytes.
- **Host site:** `flush_unit_kills_00903670` (`src/game_hosts_mission_frame.cpp`) records the
  `vt[84h]` dispatch as `Entities::kill_vtable84` (`00923010`).
- **The wreck handler** `00824B60` is `EntityQueues::wreck_handler_vtable7c`
  (`src/game_hosts_ready.cpp`). Its reference o calls: USN02 11, USNOS long 16, and JM06, JM08,
  LOMP06 and USNOS 1 each.

**The ship `vt[84h]`: `00819880`** (body `00819880-0081989D`).
- **Three steps, `__thiscall(ship)`:**
  1. `0092BD30(ECX = [ship+1018h])` clears the hull-shape fields (the physics the host does);
  2. `00818970(ship)`;
  3. a tail JMP to `0095D400(ship)`.
- **`00818970`**, the effect teardown. It walks the live effect handles, calls
  `BSP_PointEffect_StopChildren` (`00867B10`) on each, sets `+9 = 1` and releases each one:
  - `+BA4h/+BB4h`, `+BA8h`, `+BACh/+BBCh`, `+BB0h/+BC0h`;
  - `+9E8h..+9F4h`;
  - the lists at `+A00h`, `+A14h`, `+B44h`, `+B54h`;
  - the vector at `+1118h`.
- **`0095D400`**, the unit base teardown:
  - it stops and releases the effect at `+670h`;
  - it empties the 44h-record vector at `+660h` (`0095BE70` with 0, `LEA ECX,[EBX+660h]` at
    `0095D474`);
  - it calls `BSP_UnitInstance_ReleaseDamageStateInstance` (`008797B0`).
- **It sends nothing:** no session message and no score or report.

**The plane `vt[84h]`: `007CC580`.**
- **Slot:** plane vtable `00D05F20` holds `007CC580` at `+84h`.
- **Body:** `007CC580-007CC7A0` exclusive. The last instruction is `JMP 0095D400` at `007CC79B`, and
  `007CC7A0` is `BSP_Plane_EnterFlightStateTwo`. Ghidra has no function here: the address sits
  inside the candidate `007CC2F0`. **For the lead to define.**
- **Steps:**
  1. `vtable[10h]`, which is `0042E950` (the name getter; the result is discarded);
  2. **`007C75A0` `BSP_Plane_UnregisterFiringGuns`: removes the plane from the firing-plane list
     `[00F87278]`;**
  3. it stops and releases the effect vectors at `+A3Ch` and `+A4Ch`, the `class+5A0h` effect slots
     at `+A5Ch` (stride 10h), and the handles counted at `+A34h`;
  4. the tail `0095D400` above.
- **The one gameplay effect is step 2.** `[00F87278]` is the list the attacker-evasion scan reads.
  The host stands in for it with `plane_gun_fire_bc9` (`src/game_hosts_units.cpp`, the
  `[00F87278]` comment near the evasion scan).
  - Its only writer is the dogfight gun tick, which sets it every tick while the plane fights.
  - Nothing clears it on death.
  - A plane killed mid-burst keeps the flag set. It is still counted as a firing attacker unless
    that scan's `state == nullptr || state->simulate != 0` test excludes dead planes, which is
    **unverified**.
  - **Routed to lua16:** clear `plane_gun_fire_bc9` at the kill (`007C75A0`), or confirm the state
    test excludes the dead.

**The ship wreck handler: `00824B60`** (slot `7Ch`; `docs/UNIT_DEATH_MESSAGE_AND_SINK.md` has the
sink block). Beyond the physics (inertia x2, damping 2.5 / 0.5, `+828h/+82Ch` = 0):
- **Effects and sounds:** `004D1100` / `008674C0` / `00484620` teardown, and `00818970`.
- **`0074EC50(&unit+10D4h)`:** the leak manager is reset.
- **`[unit+BC8h] = U(cfg+64Ch, cfg+650h)`:** the bubble timer (GAME_EXECUTABLE line 3051: it
  advances in the sinking pass). Presentation.
- **The five-point scatter at `+B68h`:** the wreck's burst points. Presentation.
- **`004A5AA0(manager, unit)`:** breakup pieces from the wreck class, when the class has them.
  Debris.
- **`00959450` `BSP_Unit_OnDestroyed`:** the kill report through `009813A0` (the warning manager's
  loss report, which the host already carries: `loss_reports` in the warning-manager census).

**Summary for ranking #12:**
- **`00819880`:** effect and damage-state teardown only. Nothing is sent.
- **`00824B60`:** presentation (effects, bubbles, scatter, debris) plus the leak reset. Its only
  message is the loss report, which is modelled.
- **`007CC580`:** effect teardown, plus one gameplay write: the firing-list removal. That is routed.

Nothing here is bound. The row can drop to "presentation, plus the `[00F87278]` removal".

## 65. Handoff (cc9-gunnery13, 2026-09-29, at about 70% context)

### 65.1 What this worker landed

| where | packet | state |
| --- | --- | --- |
| AA_LETHALITY_AUDIT 8.1-8.4 | `cc9_aa_jm05_repair` | JM05 re-paired on main (the AA errors stay ON). Three AA terms checked and found faithful: barrels/rate, damage, aim point. The plane-hit gap was found |
| AA_LETHALITY_AUDIT 8.5 | `cc9_plane_hit_task_notify` | `kPlaneHitTaskNotifyBound` **ON**: releases move outward. P2's count missed; the override was accepted by the lead |
| AA_LETHALITY_AUDIT 8.6-8.7 | `cc9_dive_hit_clock_pair` | lua16's `kDiveHitClockBound` **kept OFF**: the rerolls stay 3 -> 3 (mechanism-failure clause) |
| 61 | `cc9_ship_fire_stance` | `kShipFireStanceBound` **ON**: USN01 and USN04 are identical, and the census held |
| GAME_EXECUTABLE reference o, `reports/cc9_reference_rebaseline_15.json` | `cc9_reference_rebaseline_15` | 16 rows on `3194cea39`. 12 switches attributed; the all-OFF anchor is identical to n |
| 62 | `cc9_scene_command_find_case` | `kSceneCommandFindCaseInsensitiveBound` **ON** by census (every lookup is exact). It is post-o |
| 63 | 60.2 item 3 | USNOS long's damage under inertia is NH's heavy-artillery friendly fire on the Portlands. That is the image's rule, as read; nothing bound |
| 64 | ranking #12 | the kill handlers: presentation, plus `007CC580`'s firing-list removal (routed to lua16) |

### 65.2 Open items, in order

1. **Ranking #7, the player gun-seat group arm** (`00959C91..00959F6D` of `00959C20`).
   - Bind the in-window arm alone, and pair on USN01 and USN04.
   - It is not started.
2. **60.2 item 4:**
   - the periscope shape in the hull box (57.2);
   - m's flags (57.2).
3. **Section 63's one unread link:** does the artillery sub-director's own target pick admit
   aircraft? If it does not, NH's cat-4 plane shots are a host divergence.
4. **Section 64's routed item** (lua16): `plane_gun_fire_bc9` is never cleared at a plane's kill.
   - The image's `007CC580` removes the plane from `[00F87278]` through `007C75A0`.
   - The lead should also define `007CC580` in Ghidra (`007CC580-007CC7A0`, exclusive; tail
     `JMP 0095D400` at `007CC79B`).
5. **Reference p** must attribute the post-o switches:
   - `kCommandTargetKeepUnauthoredBound`;
   - `kPilotLandNativeBound` (now ON);
   - `kFindEntityCaseInsensitiveBound`;
   - `kPlaneGroundLevellingBound`;
   - `kSubmarinePeriscopePrepassBound`;
   - `kSceneCommandFindCaseInsensitiveBound`;
   - anything later.

   Take the list from `local\g13_switches2.py 3194cea39 <main>`. That script also catches names
   that do not end in `Bound`.

### 65.3 Tools in the cc9-gunnery13 tree (`local\`)

- **Exports:** `g13_exp.ps1 -Commit <sha> -Specs 'name:kA=false,...'` runs detached `pair_export`s.
- **Runs:**
  - `g13_runs.ps1 -V <variant> [-Only rows]` (the reference rows);
  - `g13_batch.ps1 -V <variant> -Only rows` (the same, with a foreground wait);
  - `g13_run1.ps1` (one run, optional `-Trace` names);
  - `g13_census.ps1` (the rows on the tree's own build).
- **Reference tables:**
  - `g13_vs.py <off> <on> [rows]` (prefix `rb14` = reference n in cc9-gunnery12);
  - `g13_rows.py`, `g13_table.py`, `g13_members.py`, `g13_report.py`.
- **Analysis:**
  - `g13_aacount.py` (low aircraft deaths);
  - `g13_drops.py` (torpedo drops);
  - `g13_dive.py` (the dive census);
  - `g13_ff.py <log> <victims>` (traced hits by shooter/category/victim);
  - `g13_devrow.py <index>` (compact arcade device rows).

## 66. Correction to 63: NH's 18-inch targets are boats and submarines, and the image's row admits them (cc9-gunnery13, 2026-09-29)

**Was (63).** "NH fires them at the low attackers ... with the barrels about 3 degrees up". 63 read
this as heavy artillery engaging aircraft, and left open whether the artillery pick admits planes.

**Is.** The targets are not aircraft.
- **The 30 shots at `unit #3.1`, and the others at `unit #3.x`:** `unit hull input ... type_id=4
  kind=8`. These are submarines, class 08h.
- **The shots at `unit #2.x`, such as `unit #2.4`:** `type_id=43 kind=14`, TBoat vtable
  `00D0C648`, length 9.5 m. These are torpedo boats, class 0Eh: the suicide boats.
- **Evidence:** `local\rb15_usnosl.log`.
- **Why the barrels were at 3 degrees:** flat fire at a surface target 1100-1200 m out.

**The image's rule.** The category pick is `score_candidate_00863990`. Its rank test
(`gunnery_rank`, the table `00727BD0` builds) reads the HEAVYARTILLERY preference row at
`00E098D8` (`src/gunnery_tables.cpp`, transcribed from the image):

`0D, 0A, 07, 09, 0C, 0B, 08, 0E, 1C, 1B, 45, 46, 19, 41`

- **Submarine (08h) and torpedo boat (0Eh)** are the 7th and 8th entries.
- **No plane class (10h-17h) appears.** So the image never lets a category 4 gun take an aircraft
  through this pick. The host uses the same table, so it cannot either.

**So the unread link is closed, and there is no divergence.**
- NH's 18-inch rounds are aimed at suicide boats and surfaced submarines, as the image's
  preference row allows.
- HEAVYARTILLERY carries no line-of-fire predicate (`00729560`, 63).
- At station-keeping distance the rounds cross the Portlands.

63's verdict stands (faithful, nothing bound); only its reason is corrected. 65.2 item 3 is
closed.

## 67. Ranking #7 closed for group 3; reference p (cc9-gunnery14)

- **Reference p** (`795bb9a80`, docs/GAME_EXECUTABLE.md "2026-09-29 p") attributes every switch
  that turned ON after o. This closes 65.2 item 5.
- **Ranking #7 (65.2 item 1).**
  - The group 1/2 arm was already bound and ON.
  - The row's 47371 calls were `message_other_group`, and every one is group 3.
  - `kPlayerGunSeatArtilleryBound` binds the group 3 arm (00959F72..0095A1C7, with 00957740 read
    whole) and is **ON** by the pair in docs/PLAYER_GUN_SEAT.md 7.4:
    - JM06 and USNOS long move through the group unit's own artillery;
    - the other eight rows are gameplay-identical.
  - Groups 4 (0095A1CC) and 5 (0095A441) stay records, since no reference row sends them.
  - Open inside the arm (records):
    - the hit lead 009578C3 (00902290 / 00901C20);
    - the hand-over 0095A05B, which needs fire input;
    - the aim-point store `dev+408h..410h`.
- **The in-flight c0000005 at `bsp_game.exe+280635`** (reference p's audio-drop batch; three
  runs, the same second).
  - It symbolises through `local\rb16\build\win32\bsp_game.map` to
    `set_native_renderer_render_state_00b24460 + 0xD5` (`src/native_renderer_cached_states.cpp`).
  - That is the read through `renderer+1A10h`, the D3D device, which is null. So the session change
    lost or released the device while the mission kept drawing. It is not an FMOD object.
  - Routed to the renderer lane: it is not a gunnery null guard.

## 68. The periscope shape in the hull box (packet `cc9_hull_periscope_shape`, cc9-gunnery14)

Closes 57.2's periscope item and 60.2 item 4's first half.

### 68.1 The image (009396BA..009399BF, read again from the disk bytes)

1. `0071AD50("periszkop")` (00D0C1F0) runs at 009396D8 on the model at `[[unit+1Ch]+360h]+160h`.
   Its node goes to `[ESP+18h]`.
2. **The gate.** The block is skipped when either of these is above 0:
   - `[class+510h]`, KamikazeDamage (COMISS at 009396F5);
   - `[class+514h]`, KamikazeBlastDamage (00939706).

   It is also skipped when the node is null (0093970F).
3. **The shape.** The walk 009397F0..0093985B takes the **first** `{item, node}` pair of `model+4Ch`
   whose node is that node. That list holds the model's ConvexObjects in record order. With none,
   0093982A leaves.
4. **The descriptor:**
   - type 4 (00939724), friction 1.0 (00D7A24C), group 2, mask 5;
   - geometry `item+0Ch` (also stored at `controller+18h`, 00939883);
   - rotation and origin: the node's local matrix (00B6DB60 at 0093988D), copied as a 4x3 by
     00C336C0;
   - translation: the origin plus the item's centre `item+14h..+1Ch` (009398E1..00939919). The
     translated y goes to `controller+390h` (00939927).
5. It is pushed after the hull shapes (009399B8). So 00C57C40's box of the rotated hull joins the
   body union 00C55FC0.

### 68.2 The binding (switch `kHullPeriscopeShapeBound`, committed OFF)

- **`read_mmod_hull_convex_box`** (`src/mmod_hull_convex_box.cpp`) now also:
  - finds the first Hierarchy record listing a Note named exactly `periszkop`;
  - takes that record's first ConvexObject and its `Matrix` (00B936E0's sixteen floats, row-vector,
    origin at `[12..14]`);
  - computes `dyn_convex_mesh_shape_bounds_00c57c40` with the centred point box. It does not merge
    the result.
- **`mmod_hull_convex_box_add_periscope`** applies the kamikaze gate and merges the box, adding one
  to `shape_count`.
- **The units host's `class_hull_box`** calls it behind the switch and logs `hull periscope <unit>
  (type N): shape= merged= ...`. That file is cc9-lua18's lane; the edit is handed to the lead.

### 68.3 Census and predictions (written before the pair)

`local\g14_pericensus.py` covers the installed ship models; this installation's models are dated
2024-07-13 and are unmodified bulk. Twelve models list a `periszkop` Note. Only six of those records
list a ConvexObject:

| model | shape box y | hull y (without) | note |
| --- | --- | --- | --- |
| `i-400` | -3.48 .. 14.68 | -5.38 .. 12.47 | |
| `kaiten` | -0.31 .. 2.13 | -0.50 .. 0.68 | gated: the Kaiten class has KamikazeDamage 3000 and KamikazeBlastDamage 3000 (`vehicleclasses.lua`, this installation, mtime 2026-05-09) |
| `minisub` | 0.80 .. 4.53 | -0.88 .. 2.49 | |
| `type7` | -3.28 .. 6.83 | -3.51 .. 4.38 | |
| `type_b` | -3.36 .. 12.35 | -5.31 .. 7.00 | |
| `narwhal` | -5.07 .. 12.37 | -6.20 .. 11.60 | |

- **No shape to add** on `I-54`, `I-56`, `I-58`, `U-69`, `Cachalot` and `gato`: their periscope
  record lists no ConvexObject.
- **The reference rows** use `gato` (Gato), `Cachalot` (Narwhal, Tautog), `I-54` (PlayerSub,
  TypeB w Jake) and `Kaiten`.
- **Prediction:**
  - with the switch ON, **every reference row is gameplay-identical** (no shape merges);
  - the new log line shows `merged=0`: `shape=0` for gato, Cachalot and I-54, and `shape=1` for
    Kaiten, gated.
  - Where the switch would act: the classes on `i-400`, `minisub`, `type7`, `type_b` and `narwhal`
    models. Their vertical extent grows by 2.2 to 5.4 m, so their roll and pitch inertia rises.

## 69. Reference m's flags (57.2, 60.2 item 4's second half; cc9-gunnery14)

### 69.1 The no-ship hold moves neither USN02 nor LOMP10: closed from the counters

The hold's summary line (`summary mission ship ai approach no-ship hold`, 009F1E30 `JE 009F2003`)
in reference p's logs answers it. The hold changes a goal only on frames where the held point
differs from the one the goal copy would write (`frames_differ`):

| row | arm runs | hold frames | frames that differ |
| --- | ---: | ---: | ---: |
| USN01 | 201 | 1548 | 104 |
| IJN01 | 718 | 5353 | 5353 |
| USN02 | 3 | 0 | 0 |
| LOMP10 | 68 | 533 | 0 |
| LOMP10 long | 201 | 1600 | 0 |
| USNOS, JM06 | 0 | 0 | 0 |

- On USN02 the no-ship path is never held.
- On LOMP10 every held frame keeps the same point the copy would have written.
- So the hold cannot move either row. The integrator's expectation assumed reach where the
  counters show none. This is the same shape as reference m's `nnsh` runs, which were identical on
  both rows.

## 70. The command acceptance extra tests 009229F0 / 007AC9D0 (packet `cc9_command_extra_tests`, cc9-gunnery14)

Ranking #9 (lua19's refreshed ranking).

### 70.1 The image

**0071D6D0** (body 0071D6D0..0071D772, `RET 8`):
- The target checks come first: `vtable[8]` needs a target, then position byte `+1h` or category 1/2,
  then `00521EA0` non-null and `+5Dh` clear.
- Only after those does it reach 0071D71F. The two "no target needed" branches (0071D6E5 and
  0071D6FE) also jump there.
- The host had returned `true` on those two branches before the extra tests ran. This binding
  corrects that order.

**Torpedo** (00E08F18, 0071D71F..0071D73C) requires `009229F0(00521EA0(target), 6)`:
- `009229F0`: `__fastcall(ECX entity, EDX kind)`, `RET 0`, body 009229F0..00922A34.
  - A null entity returns false.
  - `vtable[5Ch](6)` (a ship) returns true.
  - `vtable[5Ch](1Bh)` (MLandFort) tail-calls `00922990([[entity+538h]+178h], 6)`.
  - Anything else returns false.
- `00922990` (body 00922990..009229EF, `RET 0`), for kind 6, accepts the class's `FakedType`
  (`class+178h`, docs/ATTACK_GATE_TAILS.md) when it is 7, 8, 0Ah, 0Bh, 0Dh or 0Eh. For kind 0Fh it
  accepts 10h..17h.
- **So a torpedo order at a position (kind 0) is refused**, because 00521EA0 gives null.

**Moveonpath** (00E08F80, 0071D73E..0071D75B):
- It runs only when the descriptor names an object (`+0h != 0`), and requires `007AC9D0(entity)`
  non-zero.
- That is the entity being a path kind: 47h Path, 48h, 49h, 4Ah CameraPath (docs/ENTITY_CLASS_IDS.md).
- User path points (a kind-0 descriptor, `007207CC`) never reach it.

`00E08F78` (0071D75D..0071D767) only calls 00521EA0 and accepts.

### 70.2 The binding (`kCommandExtraTestsBound`, committed OFF)

- **`command_extra_test_0071d71f`** in the commands host's Impl runs at every return of both
  0071D6D0 sites that the image routes through 0071D71F.
- **Torpedo:** the target is resolved to a host unit and tested with `unit_is_kind_of(class, 6)`.
  - **Labelled:** a kind-1Bh fort is refused, because this host holds no FakedType and the authored
    default 1Bh is outside the set.
  - **Labelled:** a unit whose class id is unknown is accepted.
- **Moveonpath with an object descriptor:**
  - refused when the object is a host unit, since no unit is a path kind;
  - otherwise taken as the authored Path it names. **Substitution:** this host resolves units only.
- **Summary line:** `summary mission director extra tests ...`, printed on both sides. It counts
  tests, refusals by reason, and the labelled cases.

## 71. Handoff (cc9-gunnery14, 2026-09-29, at about 80% context)

### 71.1 Landed or committed

| item | commit | state |
| --- | --- | --- |
| Reference p (main a4f9d6c76) | `795bb9a80` | on main; docs/GAME_EXECUTABLE.md "2026-09-29 p" |
| Ranking #7: the group 3 gun-seat arm | `729e87697`, `dce10ae24` | `kPlayerGunSeatArtilleryBound` ON (PLAYER_GUN_SEAT 7) |
| Lost D3D device: hold, recreation holders | `412b594c4`..`50ee0c2d8` | `kRendererLostDeviceHoldBound` ON; docs/D3D_DEVICE_LOST.md |
| The periscope shape (68) | `ef44bb755` | `kHullPeriscopeShapeBound` OFF; the units-host call needs lua18's file (patch below) |
| m's no-ship hold flag (69.1) | `7425320ba` | closed from the counters |
| Ranking #9: the command extra tests (70) | `623f8057e` | `kCommandExtraTestsBound` OFF |
| Ranking #10/#11: the gun fallbacks | `7f42db823` | `kGunBarrelMeshlessOneBound` OFF; GUN_BARREL_COUNT 8 |

### 71.2 Waiting for runs

Game runs failed at renderer init (CreateDevice 0x8876086A) for every worker from about 12:49 local.
One smoke passed at 13:41. Every export below is built or building in the cc9-gunnery14 tree
(`local\<v>\build\win32\Release\bsp_game.exe`). Launch through `local\g14_runs.ps1 -V <v> -Only
<rows>`, wait with `local\g14_wait.ps1`, and compare with `local\g14_vs.py <off> <on> <rows>`.

1. **The periscope pair** (`prioff` / `prion`, exported from `c8c2ea5f3` on the local branch
   `g14-peri-test`, which carries the units-host call; not for merge):
   - rows JM06, LOMP06, USNOS and USNOS long;
   - prediction: all gameplay-identical;
   - the `hull periscope` lines show `merged=0`: gato, Cachalot and I-54 have `shape=0`, and
     Kaiten has `shape=1`, gated by KamikazeDamage 3000.
   - Flip after the lead applies `local\g14_units_periscope.patch` to src/game_hosts_units.cpp.
2. **m's two open flags** (exports at `ef44bb755`: `mbase`, `m_los`, `m_nsh`, `m_sig`, `m_ns2`):
   - USN01 with `BSP_LOS_CENSUS=1` set in the launching shell;
   - read the Coastal Gun 01 -> Dunlap census lines and the hit records, pairwise against `mbase`.
3. **The extra tests** (`xtoff` / `xton`, `623f8057e`):
   - rows USN04, E2, JM05, USN13, IJN01 and USN01;
   - read `summary mission director extra tests`;
   - prediction: torpedo refusals only where `null` or `kind` > 0 (USN01's 5 tests).
4. **The fallbacks** (`fboff` / `fbon`, `7f42db823`):
   - rows USN04, JM05, JM05 long (`jm05l`), USN13 and USNOS;
   - read `summary mission gunnery fallbacks`;
   - flip only if `meshless_changed > 0` and the mechanism matches.
   - For #11, bind something only if `no_mount` or `other` carries shots.

### 71.3 Open items, not started

- **The sprite bridge** (a host scaffold) is retired after a device recreation instead of rebuilt
  (D3D_DEVICE_LOST 6). The lead accepted this as an open item.
- **The group 3 arm's records:**
  - the hit lead 009578C3;
  - the hand-over 0095A05B;
  - the `dev+408h` aim-point store.
  - Groups 4 and 5 carry no reference message.
- **Why group 3's `turns` is half of `guns`** (PLAYER_GUN_SEAT 7.4) is not separated.
- **The torpedo test on a kind-1Bh fort** needs the class `FakedType`.

### 71.4 Tools (`local\` in the cc9-gunnery14 tree)

- **Runs:**
  - `g14_runs.ps1`, `g14_batch.ps1`, `g14_wait.ps1` (the reference rows);
  - `g14_run1.ps1` (one run, `-FakeLost`);
  - `g14_dlruns.ps1` (a few runs with a wait and the key lines);
  - `g14_retry.ps1` (a smoke retry loop).
- **Exports:** `g14_exp.ps1`, `g14_expwait.ps1`.
- **Reference tables:** `g14_vs.py` (prefix `rb15` = o in cc9-gunnery13), `g14_rows.py`,
  `g14_table.py`, `g14_report16.py`, `g14_members.py`, `g14_switches2.py`.
- **Crash dumps:** `g14_dmp.py <dmp> <map>` (a stack scan of a minidump) and `g14_sym.py`.
- **Census:** `g14_pericensus.py` (the periscope census) and `g14_devices.py` (DeviceClass rows).

## 72. Results after the renderer came back (cc9-gunnery14)

### 72.1 The periscope pair (68.3's prediction held)

**Setup.**
- Builds `local\prioff` (SHA-256 prefix `38D225949D00`) and `local\prion` (prefix
  `EAE1E8E9CFC2`), both from `c8c2ea5f3`: the units-host call plus the reader, the same code as
  `09737087b` on the agent branch.
- Reference-row launch form, logs `local\prio{off,on}_<row>.log`.

**Result.**
- **Every row is gameplay-identical:** JM06 exit 1, LOMP06 exit 0, USNOS exit 1, USNOS long exit 1.
- **The `hull periscope` lines are exactly as predicted:**
  - Gato, Narwhal (Cachalot model) and PlayerSub 01 (I-54 model) give `shape=0 merged=0`;
  - Kaiten (`unit #3.1`) gives `shape=1 merged=0 points=24` with `kamikaze=3000.0/3000.0`.
- **Verdict: mechanism held, zero reach on the reference rows; flip ON.**
  - The flip is not committed here: cc9-gunnery15 holds a lease on 009396BA
    (`cc9_hull_periscope_call`).
  - The units call is committed on agent/cc9-gunnery14 as `09737087b`. The lead reconciles the two.
  - Flipped ON by cc9-gunnery15 on top of `09737087b` (cc9-gunnery15's duplicate of the units call
    was dropped before merge).

### 72.2 m's LOS role-swap flag: the sight test removes the two tests

The builds are exported at `ef44bb755`; USN01 ran with `BSP_LOS_CENSUS=1`, and the logs are
`local\<v>_usn01.log`.

| variant (OFF) | landscape hits | reverse verdict differs | against `mbase` |
| --- | ---: | ---: | --- |
| `mbase` (all ON) | 41 | 0 | - |
| `m_los` role swap | 41 | 0 | exit 1 |
| `m_nsh` no-ship hold | 42 | 0 | exit 3 |
| `m_sig` sight test | 36 | **2** (Coastal Gun 01 -> Dunlap) | exit 3 |
| `m_ns2` hold and sight test | 37 | **2** (the same pair) | exit 3 |

- The two tests the role swap answers differently, Coastal Gun 01 observing Dunlap, occur only
  with the approach sight test OFF.
- With it ON, Dunlap's approach no longer brings it into those two sightlines. The role swap then
  has nothing to change on USN01, which is why m showed no move. **Closed.**

### 72.3 m's hold and sight-test interaction on USN01: measured, not a divergence

Against `mbase`, each switch OFF moves USN01 differently. The victims are the same throughout, with
changed times, killers and ranges:

| OFF | shots | hit records | damage |
| --- | --- | --- | --- |
| `m_nsh` (the hold) | 1267 -> 1273 | 163 -> 163 | 2786.4 |
| `m_sig` (the sight test) | 1267 -> 1291 | 163 -> 165 | 2822.0 |
| `m_ns2` (both) | 1267 -> 1296 | 163 -> 166 | 2822.0 |

- Both switches act on the same approach goals: the hold keeps the goal on no-ship frames, and the
  sight test decides which targets are hidden. So their effects on the approach path are not
  additive.
- Each was paired alone (SHIP_AI_OPEN_ITEMS 35 for the hold, 31 and 34 for the sight test), and both mechanisms match the image.
- The interaction is two faithful mechanisms meeting, not a defect. **Closed.**

## 73. The extra-tests and fallback pairs (cc9-gunnery15)

**Setup.**
- cc9-gunnery14's `xtoff`/`xton` and `fboff`/`fbon` exports were built before
  `kFlyToObstacleListBound` and `kHullPeriscopeShapeBound` went ON, so all three were re-exported
  from `9c4a77c6f`, which has main's switch state plus the periscope flip:
  - `local\g15_base`, a control export with no flip (SHA-256 prefix `11EFA93C8F4E`);
  - `local\g15_xton`, which flips `kCommandExtraTestsBound` (`7CA4E2881BA6`);
  - `local\g15_fbon`, which flips `kGunBarrelMeshlessOneBound` (`86ABA73378FC`).
  - `g15_base` is the OFF side of both pairs.
- Rows use the reference launch form (`local\g15_runs.ps1`, copied from `g14_runs.ps1`), with
  `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
  - Logs are `local\<v>_<row>.log`.
  - Every log shows `present interval immediate`, its own module directory, and
    `frames_presented` equal to frames minus one.
- A 300-frame USN01 smoke passed first: renderer up, 100 mission frames, final COM release.
- The runs finished by 21:48 UTC (log mtimes).

### 73.1 `kCommandExtraTestsBound` (70): zero reach, flip ON

- **pair_diff:** USN04, E2, JM05, USN13, IJN01 and USN01 are each exit 1 (gameplay-identical).
- **`summary mission director extra tests`** reads the same test counts on both sides:

  | row | torpedo tests | moveonpath tests (all non-unit) |
  | --- | ---: | ---: |
  | USN04 | 16 | 136 |
  | E2 | 16 | 157 |
  | JM05 | 570 | 2 |
  | USN13 | 60 | 0 |
  | IJN01 | 0 | 4 |
  | USN01 | 5 | 0 |

  - Every other counter is 0 on every row: `null`, `kind`, `fort_unread`, `class_unknown` and
    `path_unit`.
- **Result:** no command is refused, as 71.2 item 3 predicted. A refusal needs a null target or a
  non-kind-6 target, and no row has one.
- The ON side runs the tests with `bound=1`.
- **Verdict: the mechanism is consistent, with zero reach on the reference rows; flip ON.**

### 73.2 `kGunBarrelMeshlessOneBound` (#10): no reach, stays OFF

- **pair_diff:** USN04, JM05, JM05 long, USN13 and USNOS are each exit 1.
- **`summary mission gunnery fallbacks`:** `meshless_changed=0` on every row.
  - `meshless_guns` is 315, 280, 397, 396 and 223; no Mesh-less device counts a barrel
    differently.
- **Verdict:** 71.2 item 4 flips only when `meshless_changed > 0`, so the switch **stays OFF**,
  untested rather than refuted. A row with a Mesh-less multi-barrel device would be needed.

### 73.3 #11: `no_mount` carries shots on JM05

- On both sides of JM05, the muzzle fallback reports `no_mount=98`, split by device class as
  `37:40 52:10 84:48`.
- JM05 long reports `no_mount=1311` (`37:123 52:1083 84:105`).
- `meshless` and `other` are 0; USN04, USN13 and USNOS have none.
- By 71.2's rule ("bind something only if `no_mount` or `other` carries shots"), **#11 is live
  on JM05**:
  - device classes 37, 52 and 84 fire through the no-mount muzzle fallback;
  - the keys are decimal (`std::to_string` at `src/game_hosts_gunnery.cpp:10524`);
  - `by_device` counts every muzzle fallback, but with `meshless` and `other` at 0 these are all
    `no_mount`.
  - Read the DeviceClass rows with `g14_devices.py` before binding.
  - **Closed by cc9-gunnery15** (docs/GUN_BARREL_COUNT.md section 9):
    - every `no_mount` gun is on a land-class unit, whose class runs the same 0095F500 slot pass;
    - `kLandPlatformAttachmentBound` is ON by its pair (9.4).

## 74. Handoff (cc9-gunnery15, 2026-09-30 02:14 UTC)

### 74.1 Landed

All of these are merged to main.

| item | commits | state |
| --- | --- | --- |
| The periscope flip (72.1) | `9c4a77c6f` | `kHullPeriscopeShapeBound` ON |
| The extra tests and fallbacks (73) | `3d6d3d1c2` | `kCommandExtraTestsBound` ON; `kGunBarrelMeshlessOneBound` OFF (no reach) |
| Reference q (main `83b528811`) | `219235036`, `d0a764f86` | docs/GAME_EXECUTABLE.md "2026-09-29 q"; 17 switches, anchor, leave-one-out |
| #11, the land-class gun mounts | `8c7511688`, `2e508be11` | `kLandPlatformAttachmentBound` ON (GUN_BARREL_COUNT 9) |
| Reference r (main `28840d691`) | `d9dd34cad`, `ef2633046`, `409120fc8` | "2026-09-30 r"; 8 switches, 17 rows (JM05 long added), window size shown inert |
| Ranking closures | `79fa18943`, `409120fc8` | #2, #4, #5, #8, #11, #14 closed; the wanderer added as row 16 |
| The torpedo reset engage draws | `6a4f54ffa`, `18979299d`, `314da1b35` | `kTorpedoResetEngageDrawsBound` ON (TORPEDO_AIM_LEAD 23) |

### 74.2 Open

**Reference s** should collect everything flipped after `28840d691`:
- the retarget ring `c17a250ae`;
- the follow trail arm `10ab258d8`;
- the target-speed override `0cd7e2da0`;
- the wanderer flip `84580f94d`;
- the reset engage draws `314da1b35`;
- anything newer.

**The party gate's reach** (reference r): it alone moves USN02, JM06, BSM01, LOMP06, USNOS and
USNOS long.
- BSM01's idle-player `HenryPT` no longer moves at all.
- LOMP06 loses the Narwhal's kill.
- These moves are measured, not paired against the image. The ships lane should check BSM01.

**The rest:**
- **Torpedo items:** TORPEDO_AIM_LEAD 24, covering the prepare site, the script attack
  altitude, the renames and row choice under the party gate.
- **From 71.3, still open:**
  - the sprite bridge after a device recreation;
  - the group 3 arm's records (`009578C3`, `0095A05B`, `dev+408h`);
  - group 3's `turns` being half its `guns`;
  - the torpedo test on a kind-1Bh fort.
- **Land mounts:** only the slot frame's origin is carried. Every land frame logged so far is a
  pure translation, but a rotated land slot would need the full frame.

### 74.3 Tools (`local\` in the cc9-gunnery15 tree, prefix `g15_`)

- **Runs:**
  - `g15_runs.ps1 -V <v> -Only <rows> [-Exe tree|<path>]` launches rows in the reference form.
  - `g15_wait.ps1 -Logs <names>` waits on them.
  - `g15_exp.ps1` runs one pair export in the background.
- **Leave-one-out:** `g15_lane_r.ps1 -Lane x -Variants 'v=kA+kB' -Commit <sha> -Prefix <p>`
  re-exports incrementally into `local\g15_lane_<x>` and runs 17 rows per variant.
- **Comparison:** `g15_vs.py <off> <on> [rows]` gives pair_diff exits (prefix `rb16` is p in
  cc9-gunnery14). `g15_pairs.ps1` adds log sanity checks.
- **Reference tables:** `g15_switches.py <a> <b>` (the value diff), `g15_flipcommits.py`,
  `g15_rows.py`, `g15_table*.py`, `g15_report17.py` and `g15_report18.py`.
- **Census:** `g15_prepcensus.py`. The env-gated `BSP_MOUNT_CENSUS=1` diagnostic stays in
  `src/game_hosts_gunnery.cpp`.

## 75. Handoff (cc9-gunnery16, 2026-09-30 04:13 UTC)

### 75.1 Landed

All of these are merged to main, or merge with this handoff.

| item | commits | state |
| --- | --- | --- |
| Torpedo order arm `0099AF53` (TORPEDO_AIM_LEAD 25) | `efbdc631c` | the image's budget; `prepare` / `009D27D1` unreachable on reference rows; nothing bound |
| Plane hit shape (AA_LETHALITY_AUDIT 9) | `4301c9f0a`, `dfeedf7e5` | `kPlaneMeshHitTestBound` ON |
| Plane blast distance, and the 31-model survey (10) | `d695ce9c0`, `a09ccee6c` | `kPlaneBlastElementEntriesBound` ON |
| Flak lock passing rule (11) | `73d83f9aa` | unreachable in the image; comment only |
| Flight-leader penalty `00863A4F` (12) | `4633852c6`, `2228a5ebd` | `kAaLeaderPenaltyBound` ON |
| Party ordinals comment (`global.enums`) | `3b20b59dd` | comment only, `src/game_hosts_mission.cpp` |
| Penalty rename | `44b79d2a7` | `kUnitGunneryFlightLeaderPenalty`, `GunneryScoreInputs::target_is_flight_leader` |

### 75.2 Open, in order

**1. Reference s (the lead's request, not started).**
- **Method:** as r (docs/GAME_EXECUTABLE.md "2026-09-30 r").
  - Take the fresh value diff of every `constexpr bool k...` between `28840d691` and main's HEAD,
    with `g15_switches.py` in the cc9-gunnery15 tree.
  - Anchor against r: all new switches OFF must give exit 0 or 1 against `g15_rr_<row>` on all 17
    rows.
  - Then leave-one-out on the moved rows.
- **Switches known to be new ON since r:**
  - the five listed in 74.2 (retarget ring, follow trail arm, target-speed override, wanderer,
    reset engage draws);
  - this session's three: `kPlaneMeshHitTestBound`, `kPlaneBlastElementEntriesBound`,
    `kAaLeaderPenaltyBound`. The three are independent and may be tested one at a time;
  - whatever cc9-ships and cc9-lua landed after r. The diff decides.
- **Base (the lead's note, 2026-09-30):** main `117a5902d` or later.
  - That base adds `kFollowTurboBound` ON (cc9-lua21, PLANE_FOLLOW_PHASE_A 9.4): the wingmen hold
    station, and the USN04 and E2 torpedo drops rise.
  - It is one of the larger movers and belongs in s. Re-export any earlier base.
- **Environment:** the window is 1600x900 for both r and s (r showed the size inert). Anything
  flipped during the s runs goes to t.
- **R row timing:** each 9000-frame row took about 1-2 minutes of wall time. A 7-row pair fits in
  one 10-minute wait, so a 17-row lane takes about three waits.

**2. GUNNERY_OPEN_ITEMS 71.3** (unchanged from 74.2):
- the sprite bridge;
- group 3's records and its `turns`/`guns` ratio;
- the kind-1Bh fort torpedo test.

**3. AA substitutions left (AA_LETHALITY_AUDIT 12.6):**
- the recon contact list;
- the aim-point height in the category range;
- kind 6's second ammunition (`+74h+7Ch`) for `00729B90`;
- the fire window in the hull frame.

**4. The withdrawn `+9D8h` name elsewhere (other lanes).** `unit_lacks_follow_target` in:
- `include/bsp/dive_bomb_task.hpp` (298, 1435);
- `include/bsp/torpedo_release_orders.hpp` (146, 198);
- `src/game_hosts_units.cpp` (3835, 14432).

Each means "the unit is its flight's leader", the same test as `unit_is_flight_leader_007b8ad0`.
Send the renames to their owners, cc9-lua (units, dive) and the torpedo lane.

**5. Torpedo items** (TORPEDO_AIM_LEAD 24 items 2 and 3):
- the script attack altitude `sq_alt_398`, which is inert until a scripted torpedo row is found;
- the renames.

### 75.3 Tools (`local\` in the cc9-gunnery16 tree, prefix `g16_`)

**Runs:**
- `g16_runs.ps1`, `g16_exp.ps1` and `g16_pairs.ps1` are g15's, re-rooted.
- Rows used: `e2`, `usn04`, `usn13`, `usn13l`, `usn01`, `jm06`, `usn12`.

**Pair summaries:**
- `g16_pmstats.py`: plane mesh census, deaths, dying-hit categories, releases;
- `g16_fbstats.py`: blast census, per-burst damage on planes;
- `g16_leaderstats.py`: leader share of AA kills;
- `g16_deathdiff.py`: per-entity death diff.

**Model probes** (built against `build\win32\Release\bsp_core.lib` through `g16_cl.ps1`, which adds
`geom_mesh_resource.cpp` and `part_damage_reachability.cpp`):
- `g16_meshprobe.exe <mmod> <w> <h> <l>`: class box against GeomMesh silhouettes;
- `g16_meshprobe2.exe <mmod...>`: elements and the GeomMesh holder's matrix chain.

**Census:** `g16_orders_census.py`, the torpedo `orders:` lines.

## 76. The front-end pump runs twice per mission frame (cc9-gunnery17, packet `cc9_menu_pump_once`)

Item 71.3 asked why group 3's `turns` is half of its `guns` (PLAYER_GUN_SEAT 7.4). The two
candidates were two entries for one unit, or two HUD updates per gun tick. **It is the second.**

### 76.1 Evidence

- **Not two entries.** On every s row, `HudWeaponGroupScreen::route_fire_message` (0077C2A0) has
  exactly as many calls as `HudWeaponGroupScreen::fire_message` (00954A10): JM06 6158 and 6158,
  USN01 2367 and 2367 (`local\g17_rs_<row>.log`).
- **Two pumps per frame.** The whole in-mission screen set updates about twice per mission frame:
  JM06 runs 3000 mission frames, and `HudShipView::update` 0064D610,
  `HudWeaponGroupScreen::update` 005484F0 and `HudWarningScreen::update` 00683020 each have 6158
  calls. The front-end summary shows `pump_frames=6200` over 3200 frames. The HUD summary shows
  `pump_frames=3000`, the mission frame's own passes.
- **Where the second pass comes from.**
  - `GameMenuHost::frame` (`src/game_hosts_menu.cpp`) models 004e4a40's front-end split: 004e4b9d
    for game states 1, 2 and 4, else "004e53b6, BSP_Game_UpdateInterfaceOnly". It pumps in the
    else arm on every frame, then calls `GameMissionHost::advance`.
  - During a mission, `advance` runs `run_mission_frame_004e4a40`, which is 004e4a40 whole
    (`src/mission_state_frame.cpp`). It has its own 004c40f0 calls at 004e5259 (the
    interface-only branch), 004e53b6 (the gate fallback) and 004e5469 (the drain loop).
  - **In the image 004e53b6 is the else arm of the simulation gate.** 004e53b2 is
    `JMP 004e53bb` over the `MOV ECX,ESI; CALL 004c40f0` at 004e53b4/004e53b6 (disk bytes,
    `bsp.py disasm-raw 004e5395`). One 004e4a40 per frame therefore pumps once, unless the drain
    loop at 004e5453 finds a pending request.
  - So the menu host's pump is a second model of the same call. Every level-1 screen of the 25h
    set updates twice per mission frame. SHIP_SCREEN_UPDATE 21 and 26 recorded this cadence as
    "twice per mission frame"; it was the host's, not the image's.
- **Why `turns` is half of `guns`.** Each pass routes one message, and each message counts the
  accepted guns. The gun tick consumes one pending pair per gun per fixed step. The pair a gun
  follows is the menu pass's pair of the current frame, taken before the frame's simulation, and
  it overwrites the mission pass's pair of the previous frame.

### 76.2 The binding (switch `kMenuPumpYieldsToMissionFrameBound`, committed OFF)

- `GameMenuHost::frame` skips its else-arm pump when the next `advance` runs a mission frame
  (`GameMissionHost::next_advance_runs_mission_frame`: the step is InMission or MissionFrames,
  there is a frame host and frames remain).
- Frames before the mission (the load) and after it keep the menu pump, since no 004e4a40 model
  runs on them.
- **Uncertainty:** the mission frame's 004e5259 pass needs the HUD manager built (`GameHudHost::
  update_interface_only_004c40f0` returns without it). On every s row the HUD `pump_frames` equals
  the mission frames, so this does not remove a pass there.

### 76.3 Predictions, written before any ON run

A same-tree pair, `local\g17_mp0` (OFF) against `local\g17_mp1` (ON), on s's seventeen rows in
s's launch form.

1. **Mechanism, on every row:**
   - front-end `pump_frames` falls to the frame count: 6200 -> 3200, USN04 9200 -> 4700 and the
     9000-frame rows 18200 -> 9200;
   - `HudShipView::update`, `HudWeaponGroupScreen::update` and the other level-1 screens' calls
     halve (JM06 6158 -> about 3079);
   - the seat group counts halve (JM06 group 3 5997 -> about 3000; USN04 group 2 7997 -> about
     4000), and so do `artillery guns`;
   - group 3's `turns` stay (JM06 14990) and now equal `guns`, apart from the first and last
     frames.
2. **Gameplay, identical (exit 0 or 1):** USN04, E2, USN02, BSM01, LOMP06, LOMP10, LOMP10 long,
   USN01, JM08, JM05, JM05 long, USN12, USN13, USNOS and IJN01.
   - In PLAYER_GUN_SEAT 7.4, the group 3 turns moved no gameplay on these rows.
   - The screens' other effects are presentation (markers, minimap, warnings) or idempotent (the
     role take 27h).
3. **May move:** JM06 and USNOS long, the two rows where group 3's turns moved fire in 7.4. The
   pair each gun follows is now the previous frame's mission pass instead of this frame's menu
   pass, so the camera it uses is up to one frame older. Any move must begin at the group unit's
   own artillery (shots, damage dealt by the group unit), as in 7.4.
4. **Mechanism failure:** a move that starts anywhere else (plane paths, other units' targeting,
   ship AI), apart from RNG coupling through changed shot counts.

### 76.4 Measured, and the verdict: ON

A same-tree pair at `ecd0e978f`: `local\g17_mp0` (SHA-256 prefix `756D48D596E1`) against `local\g17_mp1`
(`--flip kMenuPumpYieldsToMissionFrameBound=true`, `482BDB28706D`). The 300-frame USN01 smoke on
the ON build is clean (`pump_frames=300`). Logs are `local\g17_mp{0,1}_<row>.log`.

**Gameplay: all seventeen rows exit 1**, and the death rows are identical on every row.

| row | front-end pump_frames | seat groups (2 / 3) | group 3 guns / turns / refusals |
| --- | --- | --- | --- |
| USN04 | 9200 -> 4700 | 7997 / 0 -> 3999 / 0 | - |
| USN01 | 6200 -> 3200 | 0 / 2367 -> 0 / 1184 | 7421 / 3712 / 3632 -> 3712 / 3712 / 3632 |
| JM06 | 6200 -> 3200 | 0 / 5997 -> 0 / 2999 | 29985 / 14990 / 9747 -> 14995 / 14990 / 9747 |
| USNOS long | 18200 -> 9200 | 605 / 16369 -> 303 / 8185 | 211197 / 105592 / 105592 -> 105605 / 105592 / 105592 |
| LOMP06 | 2200 -> 1200 | - | - |

**Prediction check:**
- **The mechanism held.** The pump runs once per frame, and the seat messages and accepted guns
  halve. `turns` equals `guns` except for the last message of the run, which no gun tick consumes
  (JM06 14995 against 14990: one message of five guns).
- The turns and refusals themselves are unchanged. The idle camera does not move between the two
  passes, so the second pass sent the same pair.
- **JM06 and USNOS long did not move.** That is inside the prediction ("may move").

**Verdict: ON** (`kMenuPumpYieldsToMissionFrameBound = true`). Item 71.3's ratio question is
closed. The level-1 screens now run at the image's cadence: once per 004e4a40, plus once per
drain-loop iteration.
- SHIP_SCREEN_UPDATE sections 21 and 26 and SCRIPTED_HELM 6.6 describe "twice per mission frame".
  That was the host's cadence.
- The role take 27h (`kHudRoleScreenPumpBound`) now runs once per frame. It was already
  gameplay-identical at twice.

### 76.5 The rest of 71.3: group 3's records, the fort torpedo test, the sprite bridge

**The hit lead 009578C3** (disk bytes, `bsp.py disasm-raw 009578c3` and `009579b2`):
- It runs with `[dev+3FCh] == 0` and a hit entity. A kind-1Eh hit is first resolved through
  `00923810(1)` (009578E7). The lead time is `[[dev+3F8h]+34h]+5Ch * +50h` (00957934 / 009579D8).
- **Kind 6 (a ship):** `00902290` (00957964), then `00427C90` / `004142E0`, and the stores to the out
  pointer at 00957B24..00957B39. The out becomes the hit point plus the lead offset.
- **Any other kind:** `00901C20` at 00957A00, a subtraction into locals, `0042AEB0` on a local, and
  `JMP 00957B3C`. That jump lands past the out stores, so **the out keeps the raw hit point**
  written at 009578AF..009578BE. The host's raw point is exact for these kinds.
- **Reach:** the record now fires only for kind-6 hits. On the seventeen reference rows (the
  `g17_mp1` pair logs) no hit of the camera ray is an entity: `PlayerGunSeat::artillery_hit_lead` has
  no calls, and every counted hit is land. The ship lead `00902290` stays a record, with no reach.
- **The hand-over 0095A05B** needs fire input (+34h/+35h), which an idle player never gives. It
  stays a record with no reach.
- **`dev+408h..410h`** (the aim point the solve stores) is not stored by the host. Its readers were
  not surveyed; open.

**The torpedo test on a kind-1Bh fort** (GUNNERY_OPEN_ITEMS 70.2's labelled case):
- `00922990` accepts a fort's class `FakedType` (`class+178h`) only when it is 7, 8, 0Ah, 0Bh, 0Dh
  or 0Eh. The class reader's default is 1Bh (00749668..00749684, docs/ATTACK_GATE_TAILS.md).
- **No file in this installation authors `FakedType`.**
  - A search of all 621 `.lua` files under the install (the `scripts\datatables\autoload` class
    tables included; `vehicleclasses.lua` mtime 2026-05-10) found no match, case-insensitive.
  - `universe\` has no match either.
- So every fort holds the default 1Bh, and the refusal is the image's answer for this
  installation. The comment in `src/game_hosts_commands.cpp` says so now.
- `fort_unread` is 0 on all seventeen s rows, so there is no reach either way.

**The sprite bridge** (D3D_DEVICE_LOST 6) is a host scaffold that draws the milestone overlay. It is
not the native GUI. It has no reach on a reference row, which never loses the device. It stays
retired after a recreation (accepted open item, not pursued).

## 77. 00862820's class arms: the torpedo kind test and the submarine depth test (cc9-gunnery17, packet `cc9_gunnery_class_arms`)

### 77.1 The image (disk bytes, `bsp.py disasm-raw 00862820 --length 0xab`)

`00862820`: `__thiscall(ai)(int category, Entity* target)`, `RET 8`, body 00862820..008628CB.
1. 00862825..0086283B: the four liveness bytes `+5Ch` set, `+5Dh`, `+60h`, `+5Eh` clear. The host has
   these (`target_is_engageable_00862820`).
2. 00862843..0086285D: the per-instance allow byte `[ai + CCh + cat*61h + class(+C4h)]`. The
   constructor memsets it to 1 (UNIT_GUNNERY_PASS 2, 00864623), and no other writer is known, so it
   is taken as set.
3. 0086285F..00862867: the rank `[00E19BF8 + (cat*61h + class)*4]`. The host tests it separately.
4. 00862869..0086288A: **category 7 requires `IsKindOf(6)` and not `IsKindOf(0Eh)`.**
5. 0086288C..008628C5: for a target answering `IsKindOf(8)`:
   - categories 8 and 9 (0086289B / 008628A0) refuse it when `00852820` answers true;
   - category 7 accepts it (008628B7);
   - every other category refuses it when `00852820` answers false.
6. `00852820 BSP_Entity_IsAboveDepthChargeDepth` (body 00852820..0085285B, then INT3) returns
   true when world y `+100h` > `([+1204h] + [+1200h]) / 3.0`. The divisor is the double `3.0` at
   00D7A2B0 (bytes `00 .. 08 40`). It is the exact complement of `00852860`, which the host already
   has as `ship_ai_attackmove_altitude_gate_00852860`.

The host's score modelled steps 1 and 3 only.

### 77.2 The binding (`kGunneryClassArmsBound`, committed OFF)

- In `score_candidate_00863990`, after the rank:
  - the category-7 kind test;
  - the kind-8 depth test, from the units host's `submarine_band_y` bands 0 and 1. A boat whose
    bands were never seeded uses the `00E0B578` defaults (0, -20), as the ship-AI binding does.
- **Summary line**, on both sides: `summary mission gunnery class arms torpedo_refusals= sub_tests=
  sub_above= sub_refusals= bound=`.

### 77.3 Predictions (written before any ON run)

Rows with kind-8 units (from `unit hull input` in the s-base logs): JM06 (8, the Narwhal and Gato
boats and PlayerSub), BSM01 (Tautog), LOMP06 (Narwhal), USNOS and USNOS long (Gato and others) and
IJN01 (Tautog, Cachalot). The pair runs all seventeen rows. The controls are USN04 and USN02, with
no submarine.

1. **Mechanism:**
   - `sub_tests` > 0 exactly on the submarine rows where a submarine reaches the score;
   - `sub_refusals` > 0 where a submerged boat meets gun categories, or a surfaced one meets
     categories 8/9;
   - `torpedo_refusals` > 0 only where a category-7 candidate is not a ship base or is kind 0Eh.
2. **Controls (USN04, USN02):** gameplay-identical, with `sub_tests` 0. `torpedo_refusals` may be
   non-zero only if a torpedo-category candidate is a non-ship.
3. **Submarine rows:**
   - guns stop taking a submerged boat as a target, and depth charges stop taking a surfaced one;
   - JM06, IJN01 and USNOS may move (submarine damage and deaths change);
   - BSM01 and LOMP06 move only if a boat is scored there.
4. **Mechanism failure:** a gameplay move on a row with `sub_tests` and `torpedo_refusals` both
   0 on the ON side.

### 77.4 The pair, and the verdict: ON

A same-tree pair at `8d5ee7a84`: `local\g17_ca0` (SHA-256 prefix `CE56E96B3D65`) against `local\g17_ca1`
(`--flip kGunneryClassArmsBound=true`, `DB8E4EED7EFA`). The 300-frame USN01 smoke on the ON build is
clean. Logs are `local\g17_ca{0,1}_<row>.log`.

| row | exit | sub tests / above / refusals | what moved |
| --- | --- | --- | --- |
| USN04, USN02 (controls) | 1 | 0 | nothing |
| JM06 | 3 | 3000 / 0 / 2652 | Narwhal-class Submarine 01's shots 6 -> 0 (dealt 88 -> 0); the PBY and US Tankers 01/02 score nothing (their only candidates were a submerged boat). Row damage 4506.8 -> 3634.4, shots 302 -> 276. Death rows identical |
| USNOS, USNOS long | 3 | 22974 / 21824 / 5524 (long: 24294 / 23144 / 5794) | only the logged `nearest` distances. Aggregates and death rows identical |
| LOMP06 | 1 | 27 / 0 / 18 | nothing |
| BSM01, IJN01 | 1 | 0 | nothing (no boat reaches the score) |
| the other nine rows | 1 | 0 | nothing |

- `torpedo_refusals` is 0 on every row. No category-7 candidate on a reference row is a non-ship or
  kind 0Eh, so that arm has no reach here.
- The census is equal on both sides on every row. Every refusal happens before the first moved
  step.

**Prediction check:**
- **The controls held.**
- **The mechanism held:**
  - sub tests appear only on submarine rows (JM06, LOMP06, USNOS, USNOS long);
  - JM06's boats are all below the depth line (above 0 of 3000), so the gun categories refuse
    them;
  - USNOS's boats are mostly surfaced (21824 of 22974 above). Its refusals are depth-charge
    categories refusing the surfaced boats and guns refusing the submerged ones.
- **The spread:**
  - JM06 is the gameplay move: guns and aircraft no longer engage a submerged boat. Damage falls
    19%, and deaths are unchanged.
  - USNOS moves only in the logged candidate distance.
  - IJN01's boats never reach the score, so it stays identical (predicted "may move").

**Verdict: ON** (`kGunneryClassArmsBound = true`). It belongs to reference t.

## 78. Blast damage on the shooter's own side: the image has no side exemption (cc9-gunnery17, SHIP_AI 82's question)

SHIP_AI 82 records Shimotsuke (party 1) killing party-1 statics and coastal guns on USNOS by blast
(`killer_blast=1`, 25 kills from 7.65 s). The question was whether the image's blast path exempts
the shooter's side. Read from the disk bytes, **it does not**:

- **`0084BAD0` BSP_Explosion_ApplyRadialDamage** (0084BAD0..0084BC5F). The only exemption is
  `0084BBCF CMP [ESI],ECX; JE 0084BBFE`: a record whose entity is the `sourceEntity` argument is not
  queued. No party or relation is read.
- **`00904470`, the gather** (EXPLOSION_RADIAL_DAMAGE): it excludes only the source's own collision
  node (`sourceEntity->vtable[B0h]()`, compared at `0098C535`).
- **`00926E80`, the queue push**, and the drain `00926700`: no side test. The drain skips a subject
  with `+5Eh` or `+5Fh` set.
- **`009239A0` BSP_Entity_DispatchQueuedHit** (009239A0..00923AE7):
  - it resolves the shot's owner through kinds 29h/2Ah (`+174h` / `+314h`);
  - when the shot is `IsKindOf(44h)` and the owner's `+CCh` is non-negative, it notifies the victim
    through `vtable[24h](party, centre, direction)`;
  - it then calls `vtable[ECh](record)` up the `+3Ch` parent chain.
  - The notification carries the party, but nothing refuses the hit.
- **`vtable[ECh]` of the structure class** (primary vtable `00CFF3F8`, slot `00CFF4E4 = 008777D0`,
  the unit hit apply, UNIT_HIT_PATH): the damage is `(base * ownerMod - armour) * scale`. The
  owner modifier is the difficulty product for the shot's owning unit, not a relation test.

**The host matches.** `apply_impact_blast` skips the shooter only (`if (i == shooter) continue`,
citing 0084BBF9), with no side filter. So the own-side blast kills are the image's rule, as read,
the same finding as GUNNERY_OPEN_ITEMS 63 for heavy artillery. **Nothing is bound.**

- **Not read:** the difficulty owner modifier's value for an AI owner (`ProductForUnit(1, ...)`,
  DIFFICULTY_MULTIPLIERS). It scales every hit by its owner, whatever the victim's side.
- **The open question is what Shimotsuke is aiming at.** A blast lands on its own statics when the
  aim point is next to them. After `kReconContactAllKindsBound` (AA_LETHALITY_AUDIT 14) the ship
  also takes structure targets from its own sweep, so the aim point, not the blast, is where a
  difference would be. `same_side` is 0 there: no own-side target is admitted.

## 79. Handoff (cc9-gunnery17, written at about 72% context)

### 79.1 Landed (all merged to main, or merging with this handoff)

| item | commits | state |
| --- | --- | --- |
| Reference s (GAME_EXECUTABLE "2026-09-30 s", `reports/cc9_reference_rebaseline_19.json`) | `2a34cc5da`, `866db7f40` | the baseline; 15 of 17 rows moved against r, all attributed |
| One front-end pump per mission frame (76) | `ecd0e978f`, `3385d1d66`, doc fixes `ad99294cf` | `kMenuPumpYieldsToMissionFrameBound` ON; 17 rows exit 1 |
| Group 3 hit lead, hand-over, fort torpedo test (76.5) | `0a50f7e1d` | records and comments; no reach |
| Candidate range between pose origins (AA_LETHALITY_AUDIT 13) | `d163ff80d`, `b5c88ebb2` | `kAaCategoryRangeOriginBound` ON |
| The sweep scores every published kind (AA_LETHALITY_AUDIT 14) | `8b657605f`, `5b4a02cd1`, `8819d5c6f` | `kReconContactAllKindsBound` ON; large shore mover |
| 00862820's class arms (77) | `8d5ee7a84`, `0df7fdc36` | `kGunneryClassArmsBound` ON; JM06 moves |
| Own-side blast (78) | `cfefc96ec` | the image has no side exemption; host faithful; nothing bound |

### 79.2 Open, in order

**1. Reference t.** The base is main after this handoff.
- **Method:** as s (GAME_EXECUTABLE "2026-09-30 s").
  - Commit the predictions first.
  - Take a fresh value diff `59ff2a1d7..base` with `local\g17_switches.py` (cc9-gunnery17 tree).
  - Anchor with every new switch OFF against `g17_rs_<row>` (s's logs are in the cc9-gunnery17
    tree, `local\g17_rs_<row>.log`, binary `local\g17_rs`).
  - Then leave-one-out with `local\g17_lane.ps1` (three lanes in parallel, about 25 minutes per
    variant including an incremental export).
- **Switches newly ON after `59ff2a1d7`** (the diff at `743c05f9b`; recheck at the t base):

| switch | flip commit | owner / record | expected reach |
| --- | --- | --- | --- |
| `kAiWeaponFactsAtAttachBound` | `95e869be7` | cc9-lua21, WEAPON_FACTS_ORDER 6 | USN13, USN01 (deaths identical at its pair) |
| `kScriptEntityPoolUnboundedBound` | `33eace460` | lua lane | the diff decides |
| `kBuildingPadModelBound` | `44af89738` | cc9-ships (troop landing pad) | landing rows |
| `kMoveToCommandRangeBound` | `3c3741507` | PILOT_MOVETO_TASK: stage-only, twelve rows identical | none |
| `kReconPublishBound` | `13322fd0d` | RECON_PUBLICATION 4 | script-side recon rows |
| `kCommandBuildingCaptureBound` | `909760b78` | cc9-ships21, SHIP_AI 81: a CommandBuilding at 0 hp goes neutral | USNOS, USN13, USN01, JM05 (now that ships shoot structures) |
| `kMenuPumpYieldsToMissionFrameBound` | `3385d1d66` | 76 | none (17 rows exit 1) |
| `kAaCategoryRangeOriginBound` | `b5c88ebb2` | AA_LETHALITY_AUDIT 13 | USN04, E2, USNOS, USNOS long |
| `kReconContactAllKindsBound` | `8819d5c6f` | AA_LETHALITY_AUDIT 14 | USN01, JM05, JM05 long, USNOS, USNOS long, USN12, JM08, USN13, JM06, LOMP06, LOMP10 |
| `kGunneryClassArmsBound` | `0df7fdc36` | 77 | JM06 (USNOS in the logged distance only) |

  `kShipAiApproachLandingModesBound` is new and OFF; it stays out.
- **Interactions to expect:**
  - `kReconContactAllKindsBound` puts structures under fire. That gives
    `kCommandBuildingCaptureBound` its reach, so group those two if their moves overlap.
  - The shore rows' death counts rose 2-3x on the pair (USNOS 55 -> 107); predict from those
    pairs, not from s.

**2. The fire-window origin** (AA_LETHALITY_AUDIT 13.1, AA_FIRE_WINDOW_MOUNT 2).
- `0085A9A0` measures from the gun node's world translation `[gun+3CCh]+120h`. The host measures
  from the slot point plus the muzzle offsets.
- The frame is already exact. The difference is a few metres at AA ranges.
- Needs a per-gun node, which the host does not build (GUN_MOUNT_POSITIONS). Low value unless a
  node model lands.

**3. Torpedo items (TORPEDO_AIM_LEAD 24):**
- **2:** the script attack altitude `sq_alt_398` (`008A22B0 SquadronSetAttackAlt` writes squadron
  `+398h`; `009D4A70` step 5). It is inert until a row with a scripted torpedo squadron exists.
- **3:** the renames (FlyToObstacle `extent_max`/`extent_sum` = the AA range and AA damage sum;
  `speed_late_7c`/`speed_early_80` are distances). They are in `include/bsp/torpedo_*` and
  `src/torpedo_*.cpp`.
- **1:** the prepare site `009D27D1`, blocked by `blocked_0099af53` on every reference row.
- All three are inert on every reference row, and the lead ranked them last.

**4. From 76.5 and 78, small:**
- the kind-6 ship lead `00902290` on the group 3 seat (a record, no reach);
- `dev+408h`'s readers;
- the difficulty owner modifier's value for an AI owner (UNIT_HIT_PATH, `ProductForUnit(1, ...)`);
- what Shimotsuke aims at on USNOS: its blasts land by its own statics (78; SHIP_AI 82).

### 79.3 Tools (`local\` in the cc9-gunnery17 tree, prefix `g17_`)

**Runs and exports:**
- `g17_runs.ps1 -V <prefix> -Only <rows>`: the reference rows, launch form of r and s.
- `g17_wait.ps1 -Logs <names>`: a foreground wait.
- `g17_exp.ps1 -Commit -Out [-Flip]`: a detached pair export. A fresh output directory is a cold
  build of about 20 minutes; re-use an existing `local\g17_*` directory for an incremental one.
- `g17_lane.ps1 -Lane -Variants 'v=kA+kB' -Rows`: leave-one-out.

**Comparisons:**
- `g17_vs.py <off> <on> [rows]`: `pair_diff` exits. The `g15_rr` / `g15_rq` prefixes resolve into
  the cc9-gunnery15 tree.
- `g17_rows.py`, then `g17_table.py`: the reference table.
- `g17_loo.py <v...>`: leave-one-out verdicts against `g17_rs`.
- `g17_rel.py`: releases per variant.
- `g17_report19.py`: the s report.

**Switch audit:** `g17_switches.py <a> <b>` (value diff) and `g17_flipcommits.py`.

**Reading a moved row:** `pair_diff`'s `nearest` field is the logged candidate distance.
Sections 13 and 77 changed it without gameplay, so read the aggregates and the death table
before calling a row moved.

## 80. The target weight's damage terms (packet `cc9_ai_target_weight_damage_terms`, cc9-gunnery18)

### 80.1 Why (reference t, `kAiWeaponFactsAtAttachBound` on JM08 long)

Reference t's leave-one-out (GAME_EXECUTABLE "2026-09-30 t") found `kAiWeaponFactsAtAttachBound` to
be JM08 long's largest mover (25 deaths ON, 154 OFF). The first gameplay difference between
`g18_rt_jm08l` and `g18_t_wfa_jm08l` (pointer-normalised line diff) is the US capture planner's
think at t = 0.05:
- both sides order all five US groups onto `Headquarter 01`, in a different order (ON Helena,
  Bristol, Gleaves, Macomb, Grayson; OFF Bristol, Helena, Macomb, Gleaves, Grayson). The order is
  `capture_plan_00a29fd0`'s greedy pick on `k*s + (1-k)*w` with `w` = `00A250A0`, which OFF scores
  with the stand-in weight 1.0 (no facts yet) and ON with the facts;
- each `order_attack` draws `cautious = random_00bd2f10(0, 1) > aggressive`
  (`game_hosts_ai.cpp` `order_attack`). The draws are the same on both sides and only the second
  is non-cautious, so ON Bristol's seven-ship group attacks plainly and Helena's group cautiously
  (command 8), OFF the reverse (Helena command 7). Every later planner diag line keeps it.

So the magnitude is a random draw landing on a different group, not a systematic effect. The
switch's timing is the image's: `00A08460`'s target and attacker are the vehicle **class**
descriptors (`00A04560` record+0h = `[entity+538h]` or `[entity+35Ch]`, AI_TARGET_WEIGHT_TERMS),
so the hit points and barrel lists it reads exist from load. WEAPON_FACTS_ORDER section 2's "live
object fields ... present from construction" should read "class descriptor fields, present from
load"; its conclusion stands (routed to that doc's owner).

### 80.2 The image (disk bytes, `bsp.py disasm-raw 00A08460 --length 0x13B0`, `009FE200 --length 0x70`)

The barrel loop of `00A08460`'s not-type-0Fh branch, frame offsets relative to ESP after the
prologue:
- `00A085A8` reads `target+4Ch`, the class **Armour** (VEHICLE_CLASS_FIELDS, `0087CCB4`), into
  `+70h` (`00A085B1`, one push pending) and `+48h` (`00A085B7`). `00A085F8` overwrites `+48h` with
  `vtable[+24h]()` when the target class answers `vtable[+18h](6)`; a ship class serves that slot
  with `009635D0`, **UnderwaterArmour** `+6B4h`.
- `00A09443`: a barrel whose bullet sub-type is `0Ah` takes `+48h`, every other `+70h`.
- `00A09460..00A094C9`: `low = max(BlastDamageMin +B4h, DamageMin +ACh)`,
  `high = max(BlastDamageMax +B8h, DamageMax +B0h)` of the barrel's bullet class
  (`00415550 BSP_Math_MaxFloatByRef`).
- `00A094D5 FCOMIP / JA`: **armour > high skips the barrel** (before the accuracy lookup).
- `00A09578`: `009FE200(low, high, armour, hp)` multiplies the barrel's
  `time factor * accuracy * shots`. `009FE200` (`RET 10h`, `009FE200-009FE26A`) is the expected
  damage one hit deals above the armour for a uniform `[low, high]`, capped at `hp`:
  0 when `high <= armour`; `(low + high) * 0.5 - armour` when `low >= armour`; otherwise
  `(high - armour) / (high - low) * ((high - armour) * 0.5)`; the 0.5 is the double at `00D7A280`.
- `00A095A9`: the accumulator adds `D * WaterDamage (+BCh)`; `00A09624` scales it by
  `00424C40+3B0h`, **WaterTickDamage**.

**The host before this packet:** `+4Ch` was read as a "capture state" and published as 0 (unused);
`009FE200` was labelled a distance falloff and answered 1.0; the accumulator added `D * 1` at scale
1.0; the hit points were the instance maximum, which `OverrideHP` rewrites. So every host weight
ignored damage and armour.

### 80.3 The binding (`kAiTargetWeightDamageTermsBound`, committed OFF)

- `include/bsp/game_hosts_ai.hpp`: the switch; `Barrel::damage_low/high/water_damage`;
  `Unit::armour/underwater_armour` (replacing `capture_state`); `water_damage_scale`.
- `src/game_hosts_gunnery.cpp` `publish_ai_weapon_facts`: publishes them from the bullet class row
  and the unit's `armour` / `underwater_armour`, WaterTickDamage from the damage-control row, and,
  bound, the class HP (`UnitState::class_hit_points`) instead of the OverrideHP'd maximum.
- `src/ai_target_weights.cpp`: the gate, `ai_expected_hit_damage_009fe200` and the water term,
  each behind `damage_terms_bound()`; OFF keeps 1.0, `x 1` and scale 1.0 exactly.
- `src/game_hosts_ai.cpp`: the binding's methods and a census line
  `summary mission ai target weight damage terms` (barrels, armour refusals, zero per-hit answers,
  the per-hit sum, and the sums of the model's answers at its close-target and group-value sites).
- **Labelled:** a squadron target keeps its own row's class HP and armour, where the image weighs
  it through its planes' class (`+35Ch`); the category gates `bVar3..cVar7` and the type-0Fh branch
  stay as they were.

### 80.4 Predictions (written before any ON run)

The model runs on T's rows USN04, E2, USN01, USN02, JM06, JM08, USN13, LOMP06, USN12, USNOS,
USNOS long, IJN01 and JM08 long (`model_runs` or `model_pairs` above 0 on `g18_rt_<row>`); it never
runs on BSM01, LOMP10, LOMP10 long, JM05 and JM05 long.

- **Mechanism:** ON, `barrels` > 0 and `armour_refusals` > 0 on every model row, `per_hit_sum` > 0,
  `water_scale` the WaterTickDamage the damage-control line logs. OFF, `barrels` = 0. The close and
  group weight sums rise by one to two orders of magnitude ON (a per-hit damage of tens to hundreds
  replaces 1.0), with the ratio capped at MaxTargetKillRatio.
- **Controls (exit 0 or 1):** BSM01, LOMP10, LOMP10 long, JM05, JM05 long.
- **Moved (exit 3), expected:** USN13, USNOS, USNOS long, IJN01, USN12, USN02, JM06, JM08, JM08 long.
  USN01, USN04, E2 and LOMP06 may move (their model pairs feed few decisions).
- **Spread:** deaths within about 25% of OFF on the shore rows; no row's AI stops attacking
  (attack and `attackmove` counts stay above 0 wherever they were). JM08 long is RNG-coupled through
  the cautious draw (80.1), so it is judged on the census and the first capture order, not deaths.
- **Keep OFF if:** a control moves; `barrels` is 0 on a model row; `armour_refusals` equals
  `barrels` on a row (a units mismatch between armour and damage); or a row's attack orders
  collapse to 0.

### 80.5 The pair, and the verdict: ON

A same-tree pair at `0fcf19d8d` (branch merged with main `4209e62f7`): `local\g18_lane_a` (SHA-256
prefix `D7589F0BA0A0`) against `local\g18_lane_b` (`--flip kAiTargetWeightDamageTermsBound=true`,
`5EDB8D4F16B8`). The 300-frame USN01 smoke on the ON build is clean. The 18 reference t rows ran
10:40-10:54 UTC in t's launch form; logs `local\g18_p{0,1}_<row>.log`.

| row | exit | barrels / armour refusals ON | close weight sum OFF -> ON | group weight sum OFF -> ON | what moved |
| --- | --- | --- | --- | --- | --- |
| BSM01, LOMP10, LOMP10 long, JM05, JM05 long | 1 | 0 / 0 | 0 | 0 | nothing (controls) |
| USN04, E2 | 1 | 14782 / 0 | 0 | 2556 -> 21780 | nothing |
| USN01 | 1 | 15530 / 6214 | 0 | 296 -> 5051 | nothing |
| JM08 | 1 | 638892 / 47400 | 0 | 14015 -> 1490471 | nothing |
| USN12 | 1 | 78264 / 23850 | 5569 -> 289913 | 9.5 -> 1273 | nothing |
| JM08 long | 1 | 11964792 / 1181643 | 106696 -> 8396262 | 138032 -> 14833321 | nothing: the first capture order swaps Gleaves and Macomb, both cautious, so the draws land as before |
| USN02 | 3 | 625276 / 309503 | 458 -> 65395 | 530 -> 74314 | hits 5111 -> 6229, damage 59304 -> 66964, shots 4545 -> 5025; death rows identical |
| JM06 | 3 | 44153 / 30856 | 325 -> 6522 | 112 -> 11467 | hits 192 -> 287, shots 276 -> 354; death rows identical |
| USN13 | 3 | 548977 / 259998 | 2485 -> 101446 | 6196 -> 140838 | call counts only; death rows identical |
| LOMP06 | 3 | 36680 / 12240 | 0 | 743 -> 60741 | call counts only |
| IJN01 | 3 | 1191252 / 753091 | 15495 -> 653876 | 3335 -> 52143 | Downes moves 220 -> 196 m; death rows identical |
| USNOS | 3 | 2361941 / 701524 | 6607 -> 339631 | 40274 -> 4066176 | deaths 107 -> 105 (two OFF deaths absent, 61 rows re-timed or re-attributed) |
| USNOS long | 3 | 7426460 / 2198446 | 24328 -> 1516931 | 117941 -> 12018181 | deaths 129 -> 128 |

`water_scale` is 100.0 (WaterTickDamage) on every ON row; `zero_per_hit` is 0 except USNOS (5308)
and USNOS long (15528), barrels whose high damage equals the armour exactly (the gate passes them, 009FE200 answers 0).

**Prediction check:**
- **The controls held** (exit 1, `barrels` 0 on both sides).
- **The mechanism held:** every model row counts barrels ON and none OFF; refusals are a share,
  never all (USN02 49%, IJN01 63%, JM08 7%); the weight sums rise 8-140x.
- **Misses, all spread:**
  - USN04 and E2 have no refusal (no scored barrel met armour above its damage; not examined further), where every model row was predicted to refuse some.
  - USN12, JM08 and JM08 long were predicted to move and are gameplay-identical: the weights
    changed but not the choices they feed.
  - USN01 stays identical (predicted "may move").
- **The spread held:** shore deaths within 2% (USNOS 105, USNOS long 128); no row's attack stopped
  (shots rose on every moved row).

**Verdict: ON** (`kAiTargetWeightDamageTermsBound = true`), a spread miss with the mechanism
matching. It belongs to reference u.

## 81. Handoff (cc9-gunnery18, written at about 62% context)

### 81.1 Landed

| item | commits | state |
| --- | --- | --- |
| Reference t (GAME_EXECUTABLE "2026-09-30 t", `reports/cc9_reference_rebaseline_20.json`) | `ffef61b90`, `e1f2078b6` | the baseline; 16 of 17 s rows moved plus JM08 long, all attributed |
| Why the weapon facts at attach move JM08 long (80.1) | `0fcf19d8d` | a first-think group order changes which group draws the non-cautious roll; the switch's timing is the image's |
| The target weight's damage terms (80) | `0fcf19d8d`, `5e20b9ab0` | `kAiTargetWeightDamageTermsBound` ON |
| cc9-lua24's hit-index detach flip (SQUADRON_LAND_TASK 5as.1) | `f98f1281d` | `kHitIndexDetachBound` ON, applied here because this lane held the file; on the branch, not yet on main when written |

### 81.2 Open, in order

**1. Reference u.**
- **Base:** main after this handoff. cc9-lua25's base launch chain is coming: if it lands within a
  few hours, wait for it and take u after it.
- **Method:** as t (GAME_EXECUTABLE "2026-09-30 t"): predictions committed first; a fresh value diff
  `2e850cf31..base` with `local\g18_switches.py`; the all-OFF anchor against `g18_rt_<row>`; u
  against t; leave-one-out on the moved rows.
- **Rows:** t's eighteen (s's seventeen plus JM08 long 36200/36000). t's logs are
  `local\g18_rt_<row>.log` in the cc9-gunnery18 tree; its binary is `local\g18_rt`.
- **Switches newly ON after `2e850cf31`** (the diff at `18f056a15` plus `f98f1281d`; recheck at the
  u base):

| switch | flip commit | owner / record | expected reach |
| --- | --- | --- | --- |
| `kEntityCommandSelfKindBound` | `79df1d06b` | cc9-ships22, SHIP_AI 85.4 (with the two below: the landing chain) | JM08 long (29 -> 33 deaths on its pair), controls identical |
| `kShipAiApproachLandingModesBound` | `79df1d06b` | as above; its site `009F21A0` was reached on t's JM08 long (8463 calls) | JM08 long |
| `kShipAiLandStepBound` | `79df1d06b` | as above | JM08 long |
| `kLandParkStateBound` | `d970bd49a` | cc9-lua24, SQUADRON_LAND_TASK 5ar | JM05 9000 (death rows identical) |
| `kCarrierElevatorBound` | `0f735b0fe` | cc9-lua24, 5ao.1 (staged behind park) | JM05 long (stows 8 of 8) |
| `kLandingShipRampBound` | `9dd9efa62` | cc9-ships22, SHIP_AI 86.6 (a landing captures a building) | JM08 long |
| `kHitIndexDetachBound` | `f98f1281d` | cc9-lua24, 5as.1 | the pair's rows |
| `kAiTargetWeightDamageTermsBound` | `5e20b9ab0` | 80.5 | USN02, JM06, USN13, LOMP06, IJN01, USNOS, USNOS long |
| the ramp/capture arm 2 and anything else that lands | - | cc9-ships22 | recheck the diff |

- **Interactions to expect:**
  - JM08 long carries the landing chain, the ramp and the capture together. Group the three
    landing-chain switches, since they flipped in one commit.
  - JM08 long's outcome is coupled to the cautious draws (80.1). Judge it on the census lines (the
    landing and capture summaries, and `target weight damage terms`), not on deaths alone.
  - The damage terms change every model row's weights 8-140x but moved few choices. Expect small
    death moves only on the shore rows.

**2. The fire-window origin** (79.2 item 2). Skipped by the lead for now; low value without a
per-gun node model.

**3. Torpedo items** (79.2 item 3). Inert on every reference row; the lead ranked them last.

**4. From 80.3's labelled items:**
- a squadron target is weighed through its planes' class (`+35Ch`) in the image; the host uses the
  squadron's own row;
- the type-0Fh attacker branch `00A0861F..00A09222` and the category gates (`bVar3..cVar7` before
  `00A0943D`) are unprojected;
- the entity type queries `vtable[+18h]` / `+1Ch` are stubs answering false / 0.

**5. From 79.2 item 4:** unchanged (the kind-6 ship lead on the group 3 seat, `dev+408h`'s readers,
the difficulty owner modifier, what Shimotsuke aims at on USNOS).

### 81.3 Tools (`local\` in the cc9-gunnery18 tree, prefix `g18_`)

- `g18_runs.ps1 -V <prefix> -Only <rows> [-Exe <path>]`: the reference rows including `jm08l` and
  `smoke`; the exe defaults to `local\<prefix>`'s build.
- `g18_wait.ps1 -Logs <names>`, `g18_exp.ps1 -Commit -Out [-Flip]`: as g17's. `local\g18_lane_a/b/c`
  hold warm builds (an incremental export takes about a minute).
- `g18_lane.ps1 -Lane -Variants 'v=kA+kB' -Rows [-Commit] [-Prefix]`: leave-one-out, default commit
  `2e850cf31` and prefix `g18_t`; pass the u base and a new prefix.
- `g18_vs.py <off> <on> [rows]`: `pair_diff` exits (resolves `g17_rs` into the cc9-gunnery17 tree).
- `g18_rows.py <prefix> <base>`, then `g18_table.py` (edit its input name): the reference table.
- `g18_loo.py <v...>`: verdicts of `g18_t_<v>_<row>` against `g18_rt_<row>`; re-point both prefixes.
- `g18_report20.py`: the t report; copy it for u.
- `g18_switches.py <a> <b>`: the value diff. `git log -S "<name> = true"` finds a flip commit, but
  it can land on a doc mention (it did for `kHitIndexDetachBound`); check the diff.
- `g18_firstdiff.py <a> <b> [n] [skip_regex]`: the first differing lines of two logs, with
  pointers and ids normalised. This is how 80.1's divergence was found.
- The rows run fast: all 36 t and anchor runs, JM08 long included, took about 15 minutes.

## 82. RepairEnable reaches the repair task (packet `cc9_repair_enable_route`, cc9-gunnery19, GAMEPLAY_GAP_RANKING #12)

### 82.1 The image

- **The send.** `RepairEnable(unit, flag)` is `008AD330`. For an `IsKindOf(6)` entity (008AD487
  `TEST BL,BL`), it builds a message with `0075B430(9Fh)` at `008AD494`. The message's vtable is
  `00D03360` and the flag byte is at `+1Ch` (`008AD4B7`). `0077C2A0(msg, 7, ebp)` at `008AD4CD`
  routes it to the entity. A non-kind-6 entity instead gets the byte at `entity+378h` (`008AD4E8`);
  that arm is unchanged.
- **The receive.** `BSP_UnitInstance_HandleMessage` `00821E80` (body `00821E80-00822393`) handles
  9Fh at `008220D7`: `MOVZX EDX,[msg+1Ch]; LEA ECX,[unit+A20h]; CALL 00939FD0`.
  - `00939FD0` is `MOV AL,[ESP+4]; MOV [ECX+45h],AL; RET 4`.
  - Its only caller is `008220E2` (`ghidra xrefs`).
- **The consumer.** `0093C770` (the hull repair step) tests `CMP BYTE [ESI+45h],0` at `0093C776`.
  - When the byte is clear, `XORPS XMM0` at `0093C77C` makes the rate 0, so the hull does not heal.
  - When it is set, the rate is `BodyRepairMultiplier` (priority 0) or `1.0`.
- **The census of the task byte.**
  - A capstone sweep of the task block `00939F00-0093D200` (`local\g19_45.py`) finds three accesses
    to `[reg+45h]`: the setter, the constructor `0093BD48` (`MOV [ESI+45h],1`) and `0093C776`.
  - Of the 45 `.text` hits for displacement `A20h`, all are `lea`/`add` feeding task methods
    (`local\g19_a20.py`); no other method's body reads `+45h`.
  - The displacement `unit+A65h` never occurs in `.text`: the one byte hit, at `004314A1`, is inside
    a `JB` rel32.
  - **Uncertainty:** a reader that holds the task pointer outside the swept block is not excluded.
- **Delivery.** In a local session `0077C2A0` queues the message; the session pump drains it at
  the next fixed step (the `kShipPassSideMessageBound` note in `game_hosts_ship_ai.cpp`).

### 82.2 The binding (`kHullRepairEnableRouteBound`, committed OFF)

- **The setter.** `GameGunneryHost::set_hull_repair_enabled_00939fd0(unit, flag)` stores the flag as
  the task's `script_hull_repair`. When the switch is ON it also writes `task.hull_repair_enabled`
  (`+45h`).
- **The heal gate.** The host's 0093C770 step heals only while `task.hull_repair_enabled` is set.
- **When the unit has no task yet.** If the call comes before `build_guns` has made the unit's
  task, the flag is kept and applied at the host's 0093BCC0 point. The count is `before_init`. In
  the image the constructor always runs first.
- **Labelled differences from the image.**
  - The flag is stored at the Lua call, not at the next step's drain.
  - A disabled task skips the `00879810(-0)` call, as the host already does for an undamaged hull.
- **The census line.** `summary mission gunnery repair enable bound= calls= false= before_init=
  disabled_tasks= disabled_with_damage_control= withheld= by_unit{}`.
  - `withheld` is the heal the host applied on tasks whose last flag was false. That is what ON
    removes.
  - The per-call line is `  repair enable: unit N <name> enabled= damage_control= bound=`.
- **The caller.** `GameScriptOrdersHost::session_route_repair_enable_message` in
  `src/game_hosts_script_orders.cpp` must call the setter. That file is leased to cc9-lua25; the
  prepared edit is `local\g19_script_orders_patch.txt` in the cc9-gunnery19 tree.

### 82.3 Predictions, written before any ON run

**Inputs.**
- Every t row runs at effective difficulty 1: `MissionStart::set_effective_difficulty game+6ACh=1`
  on all nineteen t logs, and `Mission.Difficulty = GetDifficulty()` in the scripts.
- The flags each row's script routes at difficulty 1 were read from this installation's scripts
  (`local\g19_repair_scripts.py`; the mtimes are 2024-07-13, and 2024-08-13 / 2024-08-26 for
  usn_13_truk / usn_19_coralus).
- A false flag matters only on a ship that has damage control (`dc.enabled`), is damaged, and is
  still alive. The host heals `0.002 x max` per second (t logs:
  `hull repair 0.00200 of max per second`), so ON withholds up to about 45% of max over a
  4500-frame row.

**Rows whose scripts route only true at difficulty 1: gameplay-identical (exit 0 or 1).**
- USNOS and USNOS long (86 calls; Shimotsuke explicitly true).
- USN13 (52), USN01 (14) and USN12 (12).

**Rows whose false flags land on undamaged ships: identical.**
- BSM01 (29 calls). Every Japanese gang is set false, but t's BSM01 takes 0 damage.
- LOMP06 (19 calls: Warships, Escorts and Cargos false; the player false only at difficulty 2).
  t's LOMP06 has 0 damage.

**Rows that should move.**
- **USN04 and E2** (`usn_19_coralus.lua`, 18 calls).
  - The player's escorts, `LexFleet` and `TownFleet`, are false at difficulty 1.
  - Their hull hits (USN04: 93 hull records on t) stop healing.
  - Predicted: `withheld > 0` on the OFF census; ON moves the death table (exit 3) with the same or
    earlier deaths of US escorts, and no US ship dies later than on OFF.
  - The IJN fleets' Ph2/Ph3 flags (true and false) apply only if those phases start in the frames
    run; that is not predicted.
- **USN02** (`usn_2_java.lua`, 34 calls).
  - `DRGrp` (Kortenaer, Electra and the others) is false at difficulty 1; so are the `DRKillers`
    (Haguro, Jintsu, Yudachi, Samidare, Murasame, Harusame), which are also `SetInvincible(0.5)`.
  - Predicted: `withheld > 0` on the DR group (t: 468 hull records, 59303 damage).
  - The failure at 29.75 s is unchanged: it comes before much heal can build up (at most about 6%
    of max).
  - Deaths stay 1 or rise by at most the DR group's members.
- **LOMP10 and LOMP10 long** (`10_san_jose.lua`, 10 calls).
  - The `SanJoseForce` (Ashigara, Oyodo and six destroyers) is false. HQ, Airfield and Shipyard are
    also false, but those are routed only if `IsKindOf(6)`; the census counts 10 routed calls, so
    all 10 are routed, which is labelled.
  - Predicted: `withheld > 0` wherever the force is damaged (t: 156 hull records); death rows
    identical or re-timed earlier.
- **JM05 and JM05 long** (1 and 2 calls). The script was not found under
  `ijn_05_invasion_of_port_moresby.lua` in this installation's tree. The OFF census names the unit;
  predicted identical unless that unit is damaged.

**The mechanism check on every ON row.**
- `calls` equals t's `route_repair_enable_message` count.
- `false` equals the number of false calls in the script at difficulty 1.
- The heal applied on the false tasks (`withheld`, which counts that heal) is 0 on ON.
- Rows with `withheld = 0` on OFF are gameplay-identical.

### 82.4 Measured (pairs on `89eeb2b62`), and the verdict: ON

**Setup.**
- One base, `89eeb2b62` (main with cc9-lua25's caller `e90b50be9`), exported twice:
  `local\g19_off` with no flip and `local\g19_on` with `--flip kHullRepairEnableRouteBound=true`.
- The rows use reference t's launch form (`local\g19_runs.ps1`, g18's re-rooted), with
  `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
- A 300-frame USN01 smoke passed first.
- The fourteen reach rows ran 11:42-11:48 UTC.
- The logs are `local\g19_<off|on>_<row>.log`; `local\g19_vs.py` gives the exits.

**The census** (`summary mission gunnery repair enable`, OFF side):

| row | calls | false | withheld (OFF heal on false tasks) |
| --- | ---: | ---: | --- |
| USN04 | 18 | 18 | 1921: Lexington-class01 854, Yorktown-class01 824, Fletcher-class04 244 |
| E2 | 18 | 18 | 4452: Yorktown-class01 2389, Lexington-class01 1194, Fletcher-class04 869 |
| USN02 | 34 | 16 | 22650 over ten ships: Haguro 4676, Jintsu 3649, DeRuyter 2679, Java 2532, Samidare 2162 and five more |
| LOMP10 / long | 10 / 10 | 10 / 10 | 291 / 363 (Kiyoshimo) |
| JM05 / long | 1 / 2 | 1 / 1 | 1333 (USS Lexington) / 0 (the long row's second call re-enables it) |
| BSM01, LOMP06 | 29, 19 | 29, 19 | 0 (undamaged) |
| USN01, USN13, USN12, USNOS, USNOS long | 14, 52, 12, 86, 86 | 0 | 0 |

**The mechanism matches on every row.**
- `calls` equals t's `route_repair_enable_message` count on every row.
- `before_init=0`: every flag arrives after the host's 0093BCC0 point.
- The ON side's `withheld` is 0 everywhere.

**pair_diff, OFF against ON:**
- **Exit 1, as predicted (seven rows):** USN01, USN13, BSM01, LOMP06, USN12, USNOS and USNOS long.
  The only change on each is the census status of the two bound methods.
- **USN04, exit 3.** Deaths 48 -> 49: `D3A Val #7.1|.-3` dies only ON, and 24 plane rows are
  re-timed. No US ship dies on either side.
- **E2, exit 3.** 51 -> 51 deaths with 27 rows re-timed; dive-bomb-task releases 1 -> 0 of 19.
  - That release is reference t's knife-edge: `aro` alone moved it 0 -> 1.
- **USN02, exit 3.** The death rows are identical (1), and the mission still fails at 29.75 s.
  - Damage 66963.7 -> 55106.1 and hull hits 432 -> 342.
  - The unrepaired DR group ends at lower health: DeRuyter 1152 -> 600, Java 1034 -> 600,
    Kortenaer 874 -> 250.
  - The engagement then moves: Perth takes 3815 -> 0 damage, Electra fires 336 -> 132 shots.
- **LOMP10 and LOMP10 long, exit 3.** The death rows are identical except re-timings of 0.05 s:
  `Warhawk 01` 129.05 -> 129.10, and on the long row PT 01 and PT 02. Kiyoshimo ends at
  3683 -> 3391.
- **JM05, exit 3.** The death rows are identical; USS Lexington ends at 6555 -> 5222 (the 1333
  withheld). JM05 long is also exit 3 with identical deaths: Lexington 8000 -> 7622 while the
  first flag holds.

**Against the predictions (82.3).**
- **Hits:**
  - the seven identical rows;
  - the rows that move (USN04, E2, USN02, LOMP10, LOMP10 long);
  - USN02's failure time;
  - no US ship dying later on USN04 and E2 (none dies on either side);
  - the mechanism check.
- **Spread misses:**
  1. USN02's `withheld` is 22650, not "at most about 6% of max". The prediction assumed the row
     ends at the failure, but the host simulates all 9000 frames after it.
  2. USN04 and E2 moved through plane deaths, re-timed by up to 5 s, rather than through ship
     deaths. A damaged ship's health feeds the target weights (`kAiTargetWeightDamageTermsBound`,
     GUNNERY 80), and the planes' deaths are RNG-coupled to the AA fire (memory
     "shared RNG stream couples pairs").
  3. JM05 moved, where "identical unless damaged" was the prediction; its unit, USS Lexington, is
     damaged.

**Verdict: `kHullRepairEnableRouteBound = true`.**
- The mechanism is the image's, and it matched on every counted item.
- The misses are spread, not mechanism.
- The labelled differences in 82.2 stand.
- GAMEPLAY_GAP_RANKING #12 closes with this flip.

## 83. What a hull-terrain contact does in the image (packet `cc9_hull_terrain_contact_read`, cc9-gunnery19, for SHIP_AI_OPEN_ITEMS 87)

Read-only; no code or Ghidra change.

**Scope.** SHIP_AI 87.1 calls the contact phase `00C5BB5F..00C5C455` "unread". Most of it had
already been read, in five docs:
- DYN_PHYSICS_SUBSTEP (the substep order);
- DYN_CONTACT_SOLVER (groups and the task split);
- DYN_LCP_IMPULSE_MATH (the rows, which the shipped `LCPSolverTask` `00403720` runs);
- DYN_COLLISION_PASS (the narrow phase and the manifold);
- NATIVE_DYN_TERRAIN_CONVEX_R145 (the terrain/convex test `00C53630..00C549C8`).

This section joins those five and adds what they left open for a hull: the terrain shape's
material, and the terrain contact's normal and depth.

### 83.1 The chain for one hull against the terrain

**1. Which pairs are tested** (`00C57070`, then `00C44090`). A pair of shapes is tested when
`(maskB & groupA) || (maskA & groupB)`.
- **The hull shapes** are convex meshes (kind 4) with group `1` and mask `0Dh | class bit`
  (SHIP_HULL_SHAPES, `009394DD` / `009394A9`).
- **The terrain shape**, read here from `BSP_Landscape_LoadTerrain` `00882AC0`, has its descriptor
  on the stack at `ESP+48h` and appended to the body's shape list at `00883849`:

  | field | offset | value | written at |
  | --- | --- | --- | --- |
  | restitution | `+00h` | `0.0` | `0088359E` |
  | friction | `+04h` | `0.0` | `008835A4`, rewritten 0 at `00883684` after the `XORPS` at `00883679` |
  | group | `+08h` | `8` (`ESP+50h`) | - |
  | mask | `+0Ch` | `0` | `008835AE` |
  | kind | `+10h` | `5` (terrain) | `00883596` |

  - The heightfield fields follow at `+44h..+60h`, matching R145's constructor table.
  - The body is static: descriptor flags `|= 1` at `008838AB`, `CreateBody` at `00883921`.
  - Sample modes 0 and 2 (`00883624..0088365E`) pass the `XORPS` at `00883679`. Any other mode
    jumps to `0088367C` past it, but `XMM0` is still the `0.0` from `00883571`: nothing between
    `00883571` and `0088367C` loads it, only stores. So the friction is 0 in every mode.
- **So the hull collides:** the hull mask `0Dh` has bit `8`, and the terrain mask `0` does not
  matter.

**2. The contacts** (`00C53630`, R145).
- **Which points:** every vertex of every hull convex mesh, in mesh order, at or below the
  bilinearly interpolated terrain height (`00C54680..00C5468C`: `y <= h`, else skip). At most eight
  per shape pair.
- **The normal is the terrain cell's surface normal, pointing up, not the vertical and not a
  finite-difference gradient.**
  - It is the cross product of the cell's two edge vectors, built from the X spacing `+21Ch`, the
    Z spacing `+220h` and the height differences, then normalised through the sqrt at `00C547C4`
    (`00C546F6..00C547F7`).
  - **Uncertainty:** which sample pair gives each height difference was not traced sample by
    sample.
- **The witness point on the terrain** is the vertex moved by `(h - y)` along that normal
  (`00C54818..00C5486C`). Its x and z are scaled by the X spacing twice (R145's retained quirk).
- **The depth** is set by the manifold insert `00C3F760` as `n . (worldA - worldB)`, which is
  `(h - y)`: the vertical gap below the surface, used as a distance along `n`. On a slope it
  exceeds the true perpendicular penetration by `1 / n_y`.

**3. Material.**
- Friction is `combine(0.5, 0.0)`: a product unless one side is negative. The hull's value is 0.5
  for `Ship` and `TBoat` and 1.0 for `Submarine` (`00939365`); the terrain's is 0.0. **The
  hull-terrain contact is frictionless.**
- Restitution is `(0 + 0) * 0.5 = 0`.
- The manifold keeps up to four points. A point within 0.05 units of an existing one keeps its
  warm-start impulses; the reduction above four points (`00C3FA46..00C3FFD5`) is unread.

**4. The solver** (`00403720`: ten iterations, `world+38h`).
- **The normal row.** The target is `min(0, vn*0 + 0.05) = 0`, so the velocity row removes
  exactly the approach speed along `n`, with no bounce. The impulse is clamped `>= 0`: push, never
  pull.
- **The friction row** exists, but its limit is `0 * impulse = 0`, so it applies nothing.
- **Position correction is a split impulse** into the pseudo-velocity pair. Its target is
  `world+18h * clamp(depth, 0, world+28h) / dt`, that is `0.1 * min(depth, 0.5) / dt`: each
  substep pushes out one tenth of the overlap, at most 0.05 units per substep. It adds no real
  velocity.
- **The terrain is static** (solver index 0), so the whole response goes to the hull:
  - linear, along `n`;
  - angular, through `rA x n` and the world inverse inertia, so an off-centre contact yaws, pitches
    and rolls the hull.
- **The contact report.** Kind 8 reaches `009377E0` through the per-body callback `B+68h`
  (DYN_COLLISION_PASS "contact notifications"). That is SHIP_AI 87.1's `+1010h` latch, which only
  latches.

### 83.2 The three-keel-point stand-in (SHIP_AI 87.2), against this

| aspect | image | 87.2 | fair? |
| --- | --- | --- | --- |
| contact set | every hull-mesh vertex below the surface, up to 8 per shape, 4 kept per manifold | bow, middle and stern at the box's `min.y` | partly. A beam-on or quarter contact, and any vertex higher than the keel on a steep bank, is missed |
| direction removed | the approach component along the cell normal `n`, which has a vertical part | the horizontal uphill component, along the terrain gradient | **steep shores: yes** (`n` is nearly horizontal; the two agree when the gradient is much greater than 1). **Gentle slopes: no.** The image removes only `g²/(1+g²)` of the horizontal uphill speed and turns the rest into upward velocity (the hull rides up and is lifted); 87.2 stops the uphill motion whole |
| friction | none (terrain friction 0) | none (slides along the shore) | yes. The slide is the image's |
| restitution | none | none | yes |
| penetration | pushed out, 10% of depth (max 0.5) per substep, by pseudo-velocity | a hull already below stays; only further uphill motion is blocked | no. A hull that reaches a contact already penetrating is never pushed out |
| rotation | yes, from off-centre points (torque about the centre of mass) | none | no. A grounded bow slews the hull |
| flat ground above the keel | vertex below the surface, `n` vertical: removes downward speed and lifts | "blocks the horizontal motion whole" | no. The image does not stop horizontal motion on flat ground; with no friction, the hull is lifted, not held |

**Verdict.** 87.2 is a fair first-order stand-in for the common case, a hull driving into a
steep bank. It keeps the image's frictionless, bounce-free slide. It is not faithful on:
- gentle beaches, where the image lifts the hull (the effect depends on the ship's vertical model,
  cc9-ships22's lane);
- penetration recovery;
- rotation;
- side and quarter contacts.

**What would replace it**, in order of fidelity:
1. **Run the image's phases for hull-terrain pairs only.** The narrow phase `00C53630` (natively
   reconstructed, R145), the manifold insert `00C3F760`, and the `00403720` rows
   (`00C4DE40` / `00C42BA0` / `00C42530` / `00C42230` / `00C37B50` / `00C35020`, reconstructed in
   `src/dyn_lcp_impulse_math.cpp`), fed with the host's Dyn bodies. The host already runs
   `00C41550` and `00C5B1B0` on them.
2. **A closer stand-in.** Per hull-mesh vertex below the surface, with the cell normal and
   `depth = h - y`:
   - cancel the negative normal velocity at the point, with angular coupling through the
     inertia;
   - add `0.1 * min(depth, 0.5) / dt` of positional push along `n`;
   - no friction and no bounce.

**Not read, and labelled:** the four-point reduction; the exact sample pairs of the normal; the
hull's vertical model after a lift.

## 84. The hull-terrain contact phase, bound (packet `cc9_hull_terrain_contact_solver`, cc9-gunnery19)

### 84.1 The binding (`bsp::kHullTerrainContactSolverBound`, `include/bsp/hull_terrain_contact.hpp`, committed OFF)

`bsp::HullTerrainContactSolver` (`src/hull_terrain_contact.cpp`) runs 83.1's chain for each
ship hull, between the host's velocity phase `00C41550` and position phase `00C5B1B0`
(`motion_step_00825f20`, where `00C5BB30` puts the collision pass, the groups and the solve).

**Per step, per hull:**
1. **ManifoldUpdate.** Each of the hull's manifolds goes through the native `00C4B9B0`
   (`refresh_native_dyn_manifold_00c4b9b0`, R152) against the current pose. A manifold left
   empty is retired (`00C549D0`'s second pass).
2. **The narrow phase, per (hull convex shape, Landscape, terrain tile).**
   - The shape's vertices go through the pose in file order.
   - A vertex at or below the height is a candidate (`00C54680..00C5468C`), up to eight per pair.
     The height is `grid_height_00adb480` at the vertex's grid coordinate.
   - Each candidate is:
     - the surface point, `vertex + (h - y) * n`, on the terrain body;
     - the vertex, on the hull;
     - the cell normal `cell_normal_00adaa40`, pointing up.
   - It goes into the pair's manifold through the native `00C3F760`
     (`insert_native_dyn_contact_00c3f760`, R138, with the four-point reduction).
   - The terrain body is the identity frame.
3. **The solve** (`00403720`) for the hull's manifolds, with the semantic
   `src/dyn_lcp_impulse_math.cpp`.
   - Rows in manifold-then-point order: the row build `00C4DE40`, then the warm start `00C42BA0`.
   - Ten iterations of `00C42530` / `00C42230`.
   - The write-back `00C37B50` adds to the motion state's velocities and pseudo-velocities; the
     position phase consumes and clears the pseudo-velocities. `00C35020` stores the impulses.
   - Friction is `combine(material Friction, 0.0) = 0` and restitution 0 (83.1); the world
     settings are the shipped 0.1 / 1.0 / 0.5 / 10.
4. **The latch.** A candidate sets `+1010h` (the kind-8 report of `009377E0`). With the switch
   ON, SHIP_AI 87.2's keel census no longer sets it.

**OFF** runs step 2's test only, as a census, with no manifold and no write:
- `hull terrain contact:` gives the first contact per unit;
- `hull terrain contact census:` gives, per unit, the steps, `max_depth`, `max_up_dv` and
  `max_horizontal_dv`;
- `summary hull terrain contact` is the totals line.

**Labelled substitutions:**
- The terrain object's `00ADB480` / `00ADAA40` over the tile samples stand in for `00C53630`'s
  own interpolation.
- The hull vertices are the raw ConvexObject points (`MmodHullConvexBox::shape_points`), not the
  vertices of `00C5DEB0`'s hull: interior points can add candidates, and the order differs.
- Only hull-terrain pairs are handled. A vertex belongs to the tile its cell truncates to.
- There is one substep per host step.

**Mutual exclusion with SHIP_AI 87.2.** `kShipTerrainContactBound` (the keel stand-in) must stay
OFF while this switch is ON; both are OFF as committed.

### 84.2 Predictions, written before any ON run

**The base.** SHIP_AI 87.3's census (keel points, OFF) and 83.1's laws. A contact removes the
approach speed along the up-pointing cell normal, with no friction. It pushes the hull out by
`0.1 * min(depth, 0.5) / dt`, which is at most 0.05 m per 0.05 s step, and it adds angular
velocity when the contact is off-centre.

- **No-contact rows** (USN04, E2, USN01, USN02 up to its failure, LOMP06, LOMP10, USN12):
  exit 0 or 1.
  - Exception: a wreck that reaches the seabed now rests on it instead of sinking on towards
    the kill depth. On USN02 9000, whose wrecks reach -47 m on OFF, the wreck rows can move
    (sink depth only; the deaths themselves are unchanged).
- **JM08 36000 (the landing):**
  - The deep crossers stop at the beach instead of crossing up to 357 m inland: the transports,
    LST 01/03, Gleaves, Bristol and LSM 01. On a gentle beach a hull is lifted and slides
    sideways along the slope rather than stopping dead.
  - Maximum inland penetration drops from about 100-360 m to under about 10 m.
  - Exit 3, and the landing chain's outcome can move (the ramp latch reads `+1011h`, but its
    switch `kLandingShipRampHullContactBound` stays OFF here).
- **JM05, USN13, USNOS, JM06:** the deep crossers (Fletchers, Clemsons, Maru42/43, Gato) stop
  at their shores. Exit 3.
- **Resting contacts** (BSM01's Raleigh 3.5 m; JM05's PT boats 6-8 m; IJN01's moored ships
  3-17 m, by the keel census):
  - They are pushed up at up to 0.05 m per step, against the buoyancy the host's motion tick
    applies, so they ride higher at their berths.
  - A pose or position change of up to their keel penetration is expected; that is exit 3 by
    the position fields.
  - BSM01 takes no damage, so its death rows stay identical.
  - **Uncertainty:** in the image these berths may not be in contact at all. The keel census
    uses the box; the solver uses the raw points.

**The mechanism check:**
- ON's `candidates` are within the OFF census's order of magnitude on each contact row.
- `solves` is greater than 0.
- No hull ends deeper inland than its first contact point by more than a hull length.
- The no-contact rows show `ships=0`.

### 84.3 Measured (pairs on `5204cf184`), and the verdict: ON

**Setup.**
- `local\g19_hoff` (no flip) and `local\g19_hon` (`--flip kHullTerrainContactSolverBound=true`),
  in the reference launch form.
- A 400-frame BSM01 smoke of the ON build passed first.
- Fifteen rows ran 12:20-12:27 UTC: the SHIP_AI 87.3 contact rows plus the controls.
- The env-gated diagnostic `BSP_HULL_TERRAIN_TRACE=<file>`, one line per solve, was used on two
  extra ON runs, JM08 long and JM05 (`local\g19_trace_<row>.txt`).

**pair_diff, OFF against ON:**

| row | exit | contact steps OFF / ON | deepest candidate OFF / ON (m) | what moved |
| --- | --- | --- | --- | --- |
| USN04, E2, USN01, USN02, JM08, LOMP06, LOMP10, USN12 | 1 | 0 / 0 | - | nothing (census labels only) |
| JM06 | 1 | 119 / 1808 | 37.90 / 0.30 | the Gato stops at the shore; gameplay-identical |
| BSM01 | 1 | 8973 / 807 | 12.15 / 12.15 (t = 0.05, the spawn pose) | resting hulls; gameplay-identical |
| USN13 | 3 | 7749 / 7913 | 109.70 / 3.49 | the five Marus stop at their shores (`nearest` only) |
| JM05 | 3 | 12320 / 6207 | 222.44 / 222.44 | `nearest` only (see below) |
| IJN01 | 3 | 5999 / 2619 | 16.95 / 5.29 | the harbour ships graze the berths: Oglala's shots 306 -> 133, the controlled Downes' path 196.29 -> 220.52 m; death rows identical |
| USNOS | 3 | 895 / 2123 | 132.09 / 0.05 | the Gato no longer crosses land. Its wreck rests on the seabed at -71.95 m instead of sinking to the kill depth, so the kill at 98.25 s (`Entity::on_killed_lua_self`) is gone. Cargo5 takes 177 -> 735 damage |
| JM08 long | 3 | 27390 / 215151 | 355.80 / 2.28 | see below |

**JM08 long.**
- On OFF the invasion force drives 230-356 m inland: USTroopTransport 01/06, LST 01/03, LSM 01,
  Gleaves, Bristol.
- There it kills 15 shore structures: four piers, two AA trucks, static planes, barracks, tents,
  a hangar and a watchtower.
- On ON every invader stops at the beach, the deepest candidate at 2.28 m. Fifteen ships touch
  land, lifted onto the slope.
- None of the 15 structures dies. Instead seven invaders die at the beach under fire:
  USTroopTransport 02/04/05, LSM 02, LST 02, Macomb, and one static Mavis.
- Deaths go 33 -> 25.

**JM05's destroyers are not crossers.**
- The trace shows Clemson #1.1/#1.2 and Fletcher #2.1/#2.2 at y = -250 from their first step.
  That is a reserve placement under the seabed, and they rise from it on their own vertical
  velocity (0.6-1.7 m/s on OFF).
- SHIP_AI 87.3's 200-226 m "crossings" on JM05 are this depth, not a hill.
- ON adds the bias push (up to 1 m/s of pseudo-velocity), so they surface a little sooner.
  Only `nearest` moves.
- **Uncertainty:** whether the image's reserve bodies are in the Dyn world at all before their
  spawn is not established.

**The large upward velocity changes** (`max_up_dv` up to 54 m/s on JM08 long) are the solver
cancelling the host motion tick's vertical velocity.
- That velocity steers the hull to its buoyancy height (trace: `v0.y = -22.5` each step on a
  beached transport at y = -10.5, and `dv.y = +20.3`).
- The net vertical speed stays under 3 m/s, and the hull rests on the slope. This is the
  intended interaction of the two, not an impulse blow-up.
- The inverse mass and world inverse inertia the rows use are the host body's
  (`7.3e-5`; `1e-8..1e-7`).

**Against 84.2.**
- **Hits:**
  - the no-contact rows are exit 1;
  - JM08 long, USN13, USNOS and IJN01 move;
  - the deep crossers stop at the shore: the maximum depth falls from 110-356 m to 0.05-3.5 m;
  - the wreck rests on the seabed (USNOS);
  - the mechanism checks: candidates are of the same order, `solves > 0`, and every
    no-contact row shows `ships=0`.
- **Spread misses:**
  1. BSM01 and JM06 are gameplay-identical, not exit 3. Neither the lift nor the stop reaches
     a gameplay field.
  2. JM05's movers were reserve placements, not crossers.
  3. JM08 long's outcome is the beach fight above, which 84.2 did not predict in detail.

**Verdict: `kHullTerrainContactSolverBound = true`.**
- The mechanism is the image's contact phase, and it matched on every checked item.
- The labelled substitutions in 84.1 stand.
- SHIP_AI 87.2's `kShipTerrainContactBound` must stay OFF: the two are exclusive.
- `kLandingShipRampHullContactBound` can now be judged against real contacts: the latch comes
  from this solver.

### 84.4 The stand-in gate, the shape filter, and the pair on current main (cc9-gunnery19)

**The gate** (`a5a9a7502`). With `kHullTerrainContactSolverBound` ON:
- the SHIP_AI 87.2 keel stand-in never applies its stop, even with `kShipTerrainContactBound` ON;
- `+1010h` / `+1011h` come from the solver's contacts alone.

The stand-in's census still runs.

**The shape filter.** The solver now applies `00C44104`'s test:
- **Hull:** group 1; mask `0Dh | class bit`, or `0Dh` after
  `NavigatorSetAvoidLandCollision(false)`, which calls `008A3C79 -> 0092BD00 -> 00C48020` and
  writes `shape+30h = 0Dh` for every hull shape (SHIP_AI 87.6).
- **Terrain:** group 8, mask 0.
- `0Dh` keeps bit 8, so the disable side does **not** drop terrain contact; it drops only the class
  bit. No hull-terrain pair is filtered. The test is in the code as documentation of that.

**The pair on current main** (`a5a9a7502`; `local\g19_goff` flips the solver OFF,
`local\g19_gon` is main as it is). The stand-in is OFF and the ramp latch
`kLandingShipRampHullContactBound` ON on both sides.
- **Exit 1:** USN02, USN04, USN01, JM06.
- **Exit 3:** JM08 long, USNOS, USN13, IJN01, JM05. Each has the same death rows and
  aggregates as 84.3, so the stand-in and the ramp latch leave the solver's pair unchanged.
- **This is also the stand-in-OFF comparison:** main's stand-in is OFF on both sides.

**What the solver does to JM08 long's landing chain** (`summary mission ship ai landing modes`,
`land state`, `landing ship ramp`):

| | OFF (hulls cross land) | ON (solver) |
| --- | --- | --- |
| mode-3 approach points | 1081 | **0** |
| mode-3 in reach / begins | 2 / 2 | 0 / 0 |
| land steps / final | 822 / 772 | 0 / 0 |
| ramp ground contacts / lowers | 3008 / 2 (LST 03 at 831.95 s, LST 01 at 897.15 s) | 0 / 0 |

- **Why it breaks:** with the solver the landers stop at the beach, and the approach never
  selects landing mode 3 (`009F21A0`, `mode3_points=0`, `no_pad=0`). So no pad is assigned, no
  ramp latches, and nothing lands.
- On OFF the chain ran only because the hulls drove 230-356 m inland into the HQ's reach.
- **The chain's entry to mode 3 is therefore unverified against a hull that cannot leave the
  water.** This is cc9-ships22's lane (SHIP_AI 85-86):
  - what selects mode 3 in the image, and from how far;
  - whether the image's landers beach closer to `Headquarter 01`'s pads than the host's do.
- **Also open on the solver's side:** interior raw points can stop a hull slightly early
  (84.1's labelled substitution). That is centimetres to metres, not the hundreds of metres the
  landing needed on OFF.

## 85. Handoff (cc9-gunnery19, 2026-09-30 13:05 UTC, at about 70% context)

### 85.1 Landed

| item | commits | state |
| --- | --- | --- |
| GAMEPLAY_GAP_RANKING refresh against reference t; #15's `+830h` list re-read | `8efe89ced`, `54fbbd5ea` | #1, #3, #6, #9 and #16 closed; #10 and #15 parked (no reach on t) |
| RepairEnable's 9Fh arm reaches `task+45h` (82) | `816520781`, `b6d9c158b` | `kHullRepairEnableRouteBound` ON; moves USN04, E2, USN02, LOMP10, LOMP10 long, JM05 and JM05 long |
| The hull-terrain contact read (83) | `79c63984e` | the terrain shape is frictionless and inelastic; the LCP laws |
| The hull-terrain contact phase (84) | `5204cf184`, `5f982775c`, `a5a9a7502`, `66cb304a9` | `kHullTerrainContactSolverBound` ON (`include/bsp/hull_terrain_contact.hpp`); it gates SHIP_AI 87.2's stand-in off and feeds `+1010h` / `+1011h` |

### 85.2 Open, in order

**1. Reference u. Do not wait for `kBaseLaunchChainBound`** (OFF on main, expected later).
- **Base:** current main.
- **Method:** as t (GAME_EXECUTABLE "2026-09-30 t"):
  - commit the predictions first;
  - run the switch diff `2e850cf31..base`;
  - run the all-OFF anchor against `g18_rt_<row>` (the cc9-gunnery18 tree);
  - compare u against t;
  - run leave-one-out on the moved rows.
- **Rows:** t's eighteen, s's seventeen plus JM08 long.
- **The diff tool misses switches.** `local\g18_switches.py` (copied as `local\g19_switches.py`)
  missed several at `f8622d137`. Also run
  `git diff 2e850cf31 HEAD -- src include | rg "^\+.*constexpr bool k\w+ = true"`, then check
  each switch's current value.
- **Switches ON since t, as of `f8622d137`:**

| switch | owner / record | expected reach |
| --- | --- | --- |
| `kEntityCommandSelfKindBound`, `kShipAiApproachLandingModesBound`, `kShipAiLandStepBound` | cc9-ships22, SHIP_AI 85.4 (the landing chain; one commit, group them) | JM08 long |
| `kLandingShipRampBound`, `kLandingShipRampHullContactBound` | cc9-ships22, SHIP_AI 86.6 / 87.5 | JM08 long; with the solver ON the landers never reach mode 3 (84.4), so expect the ramp to be inert |
| `kLandParkStateBound`, `kCarrierElevatorBound` | cc9-lua24, SQUADRON_LAND_TASK 5ar / 5ao.1 | JM05 long |
| `kHitIndexDetachBound` | cc9-lua24, 5as.1 | its pair's rows |
| `kAiTargetWeightDamageTermsBound` | GUNNERY 80.5 | USN02, JM06, USN13, LOMP06, IJN01, USNOS, USNOS long |
| `kNavigatorAvoidanceDeliveryBound` | cc9-ships22 (ranking #13) | the rows with `NavigatorSet*` calls: USNOS, USN13, USN02, E2, USN04, USN01, JM06, USN12 |
| `kHullRepairEnableRouteBound` | 82.4 | USN04, E2, USN02, LOMP10, LOMP10 long, JM05, JM05 long |
| `kHullTerrainContactSolverBound` | 84.3 / 84.4 | JM08 long (deaths 33 -> 25 on its pair; the invasion stops at the beach), USNOS (the Gato wreck rests on the seabed; the kill-depth kill is gone), USN13, IJN01, JM05 (`nearest` only) |

- **`kShipTerrainContactBound` is `true` at `f8622d137`**
  (`src/game_hosts_units.cpp`, `GameUnitsHost::Impl`). With the solver ON its stop is gated
  off (84.4), so it is inert. Leave-one-out on it alone should be exit 0/1 on every row.
- **Group the solver with the landing chain on JM08 long.** They interact: the chain's mode-3
  entry is not reached while hulls cannot cross land.

**2. JM08 long's landing mode-3 entry under the solver** (84.4). The lead keeps the solver ON (2026-09-30) and routes this to cc9-ships23 as its top item. In reference u, read JM08 long as moving through the solver, with this fix still pending.
With the solver ON, `mode3_points` goes 1081 -> 0 and the ramps go 2 -> 0. Find what selects
`009F21A0`'s mode 3, and from how far; then find where the image's landers beach relative to
`Headquarter 01`'s pads. The per-solve diagnostic is `BSP_HULL_TERRAIN_TRACE=<file>`. On
JM08 long a beached transport sits at y about -10.5; the motion tick's servo gives
`v0.y = -22` and the solver answers `dv.y = +20`.

**3. The solver's labelled substitutions** (84.1), in order of expected effect:
- **a. `00C5DEB0`'s hull vertices.**
  - The solver uses the ConvexObject's raw points (`MmodHullConvexBox::shape_points`). The
    image builds a convex hull from them: a 0.001 dedup, a 4096-vertex limit, interior points
    dropped, its own vertex order.
  - Interior points can add candidates and use up the 8-per-pair cap in a different order.
  - Read `00C5DEB0` and reproduce the vertex set and its order.
- **b. `00C53630`'s own interpolation.**
  - The solver uses the terrain object's `00ADB480` height and `00ADAA40` cell normal over the
    same tile samples.
  - `00C53630` has its own schedule (R145), including the X-spacing quirk on the witness point.
  - The native `intersect_native_dyn_terrain_convex_00c53630` could be called on shape blobs:
    - terrain `+08h = 5`, `+34h` local transform, `+210h` samples (`uint16`), `+214h/+218h` 33,
      `+21Ch/+220h` 9.375, `+224h` `inv_scale`, `+228h` offset, `+22Ch` mode 1;
    - convex `+210h` mesh `{vertices, count}`, 16-byte vertices;
    - `application_camera_axes_crt()` for its CRT context.
  - The tile's local transform comes from `00882AC0`'s `rep movsd` of `[src+74h]` and the tile
    offsets (`00883853..008838A2`), not read.
- **c. Substeps.** The host runs one substep per 0.05 s step (its integration phases already
  did). The image's `00C5C540` plan (fixed substep `world+00h`) would run the contact phase per
  substep.
- **d. Only hull-terrain pairs.** Hull-hull and hull-object manifolds and the group formation
  `00C4B610` are not modelled. The reserve ships under the seabed (JM05's destroyers at
  y = -250) get terrain contacts; whether the image's reserve bodies are in the Dyn world is
  not established.

**4. Carried over from 81.2:** the fire-window origin, the torpedo items, 80.3's labelled items,
and 79.2 item 4.

### 85.3 Tools (`local\` in the cc9-gunnery19 tree, prefix `g19_`)

- **Reference-form runs:** `g19_runs.ps1 -V <prefix> -Only <rows>`, `g19_wait.ps1 -Logs`,
  `g19_exp.ps1 -Commit -Out [-Flip]` and `g19_vs.py <off> <on> [rows]` (resolves `g18_rt` into
  the cc9-gunnery18 tree). These are g18's, re-rooted.
- **Census helpers:**
  - `g19_reach.py <patterns>`: grep over reference t's logs;
  - `g19_repair_scripts.py`: the RepairEnable calls per row script;
  - `g19_a20.py` / `g19_45.py`: the repair-task census.
- **Pair logs:**
  - `g19_off_*` / `g19_on_*`: RepairEnable;
  - `g19_hoff_*` / `g19_hon_*`: the solver on `5204cf184`;
  - `g19_goff_*` / `g19_gon_*`: the solver on `a5a9a7502`;
  - `g19_trace_jm08l.txt` / `g19_trace_jm05.txt`: solver traces.

## 86. The solver's hull vertices from 00C5DEB0 (packet `cc9_hull_terrain_dyn_hull_vertices`, cc9-gunnery20)

85.2 item 3a. The solver tested the ConvexObject's raw points in file order. The image tests the
vertices of the Dyn hull the ConvexObject parse builds from them.

### 86.1 The read

- **`006FAD70`** (the ConvexObject parse; ECX the object, RET 4) runs in this order:
  - `006FAE3B..006FAE5F` copies the point records' xyz, which sit at `+4h` of each 20h-byte
    record, into a 0Ch-stride array.
  - `006F9EE0` (ECX the object, stack `(points, count)`) takes the box: min and max start from
    `+-FLT_MAX` (`[00D7A244]`, `[00D7A248]`). The centre is `(min + max) * 0.5`: each sum is
    rounded to float, then multiplied by the double `0.5` at `[00D7A280]` (`006FA25D..006FA2AF`).
    The centre is stored at object `+14h`.
  - `006FAEA0..006FAEC7` subtracts the centre from every point.
  - `006FAECE` calls `00C5DEB0` (ECX object `+0Ch`, stack `(count, points)`, RET 8).
  - The listing ends with RET 4 at `006FAEE2`; INT3 padding follows.
- **The shape takes the centre back as its translation** (mmod_hull_convex_box.hpp, from GUNNERY
  47-49).
- **`00C53630` reads the convex mesh `{vertices, count}`** (16-byte vertices). These are
  `00C389C0`'s vertex records, in the order that `00C5DAE0`'s triangle compaction leaves them.
- **`00C5DEB0` already has a reconstruction:** `avoid_zone_dyn_hull_replace_00c5deb0` in
  `src/avoid_zone_dyn_hull.cpp`, checked against 719 original-byte fixtures (docs/AVOID_ZONE_DYN_HULL.md).

### 86.2 The binding (`bsp::kHullTerrainDynHullVerticesBound`, committed OFF)

- **ON:** each hull shape is built once per unit and rebuilt only when its raw points change:
  - the centre, computed as `006F9EE0` does;
  - the re-centred points go through `avoid_zone_dyn_hull_construct_00c5df30`;
  - the narrow phase then tests the hull's vertices plus the centre, in the hull's vertex order.
- **OFF:** the raw points in file order, as before.
- **Census:** the summary line `summary hull terrain contact` gains `dyn_hull`, `hull_shapes`,
  `raw_points` and `hull_vertices`. The counts cover only builds, so they are 0 when OFF.
- **Uncertainty:**
  - The host's raw points are assumed to be the same floats as the file's `+4h` xyz.
  - The Dyn convex shape is assumed to hold this hull's vertices in this order. The hull copy
    `00C40F50` is a deep copy (`004039D0`, a memcpy), so it would keep them. That the shape's
    mesh comes through it is not read.

### 86.3 Predictions, written before any ON run

- **The mechanism:**
  - `hull_vertices < raw_points` on every row with ships. The hull drops interior points and
    points within 0.001 of another.
  - `hull_shapes` equals the number of hull shapes of the ships that run the phase.
- **`max_depth` barely moves.** The deepest point of a convex point set is a hull vertex, so on a
  given pose the deepest candidate is the same. It moves only through changed trajectories.
- **Fewer candidates per contact step.** Interior points under the terrain stop taking places in
  the 8-per-pair cap, and the hull's vertex order replaces file order in filling it.
- **Rows:**
  - No hull-terrain contact: USN04, E2, USN01, USN02, JM08, LOMP06, LOMP10, USN12. Exit 1: only
    the census line moves.
  - Contact but gameplay-identical under the solver: JM06, BSM01. Exit 1.
  - USN13, JM05: exit 1 or 3, with `nearest` only.
  - IJN01, USNOS, USNOS long: exit 3 is possible (the harbour grazes and the Gato's seabed rest),
    with death rows identical.
  - JM08 long: exit 3 (36000 frames of beach contact). The invaders still stop at the beach;
    `max_depth` stays below 5 m; the HQ is not reached; deaths are between 20 and 30.
- **A mechanism failure keeps it OFF:**
  - a hull with 0 vertices, or `hull_vertices > raw_points`;
  - a hull crossing land (`max_depth` in the tens of metres);
  - a ship stopping away from land.

### 86.4 Measured (pairs on `af7fd46c5`), and the verdict: ON

**Setup.**
- `local\g20_voff` (no flip, SHA-256 prefix `3B40DE0255B1`) against `local\g20_von`
  (`--flip kHullTerrainDynHullVerticesBound=true`, `3DBC6B14B350`).
- Reference u's launch form, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.
- The 300-frame USN01 smoke of the ON build is clean: 35 shapes, 4148 raw points, 992 hull
  vertices.
- The OFF side is gameplay-identical to reference u (exit 1 on JM08 long, USNOS, IJN01 and JM05).
- The logs are `local\g20_v{off,on}_<row>.log`.

| row | exit | ON: shapes / raw points / hull vertices | contact steps OFF / ON | candidates OFF / ON | max depth OFF / ON (m) | what moved |
| --- | --- | --- | --- | --- | --- | --- |
| USN04, USN02 | 1 | - | 0 / 0 | - | - | census line only |
| JM06 | 1 | 48 / 4932 / 1244 | 1808 / 1834 | 3749 / 4286 | 0.30 / 0.30 | nothing |
| BSM01 | 1 | 63 / 6765 / 1973 | 807 / 3119 | 2094 / 3887 | 12.15 / 12.15 | nothing (resting hulls) |
| USNOS | 1 | 184 / 23414 / 6383 | 2123 / 2117 | 4741 / 3264 | 0.05 / 0.09 | nothing |
| USNOS long | 3 | 184 / 23414 / 6383 | 21602 / 21998 | 122559 / 51658 | 2.91 / 0.49 | death rows identical; hits 2809 -> 2812, shots 16423 -> 16402 |
| USN13 | 3 | 285 / 34913 / 7710 | 7913 / 6883 | 53029 / 10585 | 3.49 / 0.41 | death rows identical; unit rows only |
| IJN01 | 3 | 87 / 10310 / 2852 | 2619 / 4069 | 4308 / 4928 | 5.29 / 5.29 | death rows identical; the controlled Downes' path 220.52 -> 193.89 m, shots 2414 -> 2533 |
| JM05 | 3 | 80 / 8619 / 3368 | 6207 / 6507 | 54425 / 53826 | 222.44 / 226.57 (the reserve placement under the seabed, 84.3) | `nearest` only |
| JM08 long | 3 | 41 / 4145 / 1156 | 215151 / 198623 | 1431086 / 439606 | 2.28 / 2.65 | deaths 25 -> 24 (see below) |

**JM08 long.**
- The invaders still stop at the beach (deepest candidate 2.65 m); 13 ships touch land, where
  OFF had 15.
- Four deaths are only OFF: Static Gekko 06, Japanese AA truck 01, USTroopTransport 05 and LST 02.
  Three piers die only ON (`Pier, Wooden 01`, `Pier, Wooden 04`, `Pier, Wooden, Large 01`).
- `Headquarter 01` now reaches 0 hp and goes neutral (`health_zero=11 neutralized=1`, OFF 0 / 0).
  It never flips, and no ramp lowers (`ground_contacts=0`).

**Against the predictions (86.3).**
- **Held:**
  - the mechanism: hull vertices are 20-40% of the raw points on every row, and no hull crosses
    land;
  - every exit-1 row, the exit-3 rows' death tables, and JM08 long's beach stop and death count
    (24, inside 20-30).
- **Misses:**
  1. **`max_depth` moves more than "barely":** USN13 3.49 -> 0.41 and USNOS long 2.91 -> 0.49.
     The argument was wrong. The depth `h(x, z) - y` is not linear in the point, because the
     terrain is not a plane, so an interior point under a bump can be deeper than every hull
     vertex. The raw set's deepest candidates were presumably such points (not traced per pose).
  2. **Candidates rise on IJN01, JM06 and BSM01.** Their contact steps rise with them: the
     trajectories changed. Candidates per contact step fall everywhere except JM06 (2.07 -> 2.34).
  3. **JM08 long's HQ reaches 0 hp**, where "the HQ is not reached" was predicted. The hulls still stop at the beach (no ramp contact on either side);
     which ships brought it down is not read.
  4. USNOS moved nothing (exit 3 was allowed; exit 1 measured).
- **No mechanism failure:**
  - no hull crosses land;
  - `hull_vertices < raw_points` on every row;
  - no ship stops away from land.
  - Not checked: whether a single shape built zero vertices. The census counts totals only; every
    row's average is 26 to 42 vertices per shape.

**Verdict: `kHullTerrainDynHullVerticesBound = true`.** Spread misses with the mechanism matching.
It belongs to reference v. Remaining labelled substitutions from 84.1: `00C53630`'s own
interpolation (85.2 item 3b), substeps (3c), and hull-terrain pairs only (3d).

## 87. The solver's terrain test: 00C53630 itself, and one manifold per body pair (packet `cc9_hull_terrain_native_test`, cc9-gunnery20)

This is 85.2 item 3b. The solver's narrow phase used the terrain object's height (`00ADB480`)
and cell normal (`00ADAA40`). Now the reconstructed `00C53630` is called on shape records built
as the image builds them. It is `intersect_native_dyn_terrain_convex_00c53630`, R145: an x87
schedule compared against 36,864 original-byte pairs.

### 87.1 The read

- **The tile bodies, `00882AC0`** (`00883535..008839E2`):
  - The outer loop is `tx` (`[esp+2Ch]`, with `[esp+30h]` stepping 12Ch = 300). The inner loop
    is `tz` (`[esp+3Ch]` stepping 300).
  - The block is `terrain+40h[wide * tz + tx]`; with no block there is no body (`00883564`).
  - Each tile gets **one static body with one shape**. The shape descriptor is at `esp+48h`:
    - kind 5 (`00883596`);
    - an identity local frame (`008835CE..0088361F`);
    - 33 x 33 samples;
    - spacing 9.375;
    - a U16 block (type 2) gives mode 1: samples `blk+40h`, bias `blk+2Ch`, `+58h = blk+30h`,
      whose reciprocal is the shape's multiplier. A float block (type 0) gives mode 0, samples
      `+34h`.
  - The **body's** frame (descriptor `esp+168h`, `+14h`) is the Landscape `+74h` 4x4
    (`rep movsd`, `00883871`) converted by `00C336C0`. Its translation is moved by
    `(origin_x + float(300 tx), 0, origin_z + float(300 tz))` (`0088384F..00883905`), each sum
    rounded to float. `00C5D580` creates the body.
- **The pair, `00C44090`:**
  - It loops body A's shapes against body B's shapes. Each shape pair must pass the mask test and
    have a dispatcher.
  - On a hit, the manifold is `00C3F4D0` FindOrCreate(**body A, body B**), so there is one
    manifold per body pair, not per shape pair. Every point goes through `00C3F760`.
  - The matrices passed are each body's `+08h` 3x4.
- **`00C53630`'s output:**
  - Point B is the vertex in body B's space (the shape frame applied).
  - Point A is in the terrain shape's grid space: `(u * spacing_x, y, v * spacing_x)` plus
    `(h - y) * n`, using the X-spacing quirk. That space is the tile body's space, since the
    shape frame is the identity.
  - The normal is the cell's cross product, normalized and taken to world.

### 87.2 The bindings (committed OFF)

- **`kHullTerrainNativeTerrainTestBound`:**
  - Each hull shape becomes a kind-4 record: section 86's hull vertices, centred, with the centre as its
    translation.
  - Each tile becomes a kind-5 record plus its body frame.
  - Every tile whose inclusive x/z range meets the hull's world x/z range is tested against every
    hull shape.
  - Point A goes in as the tile translation plus the local point: the host keeps its terrain
    manifold body at the identity.
- **`kHullTerrainBodyPairManifoldBound`:** the manifold key drops the shape (one manifold per
  hull and tile).
- **Labelled substitutions:**
  - The right/down neighbours read at an inclusive edge coordinate are padded with `FFFFh`
    (-1000.0); the image reads whatever follows the block.
  - The Landscape frame's rotation is taken as identity, as 84.1 already does.
  - The broad phase is the x/z range test above, not the Dyn AABB tree.
  - The shape order inside a body pair is the host's list order.

### 87.3 Predictions, written before any ON run

For `kHullTerrainNativeTerrainTestBound` alone, against U with 86 ON:
- **The mechanism:**
  - `rejected_normal` stays 0.
  - `max_depth` stays within about 1 m of OFF on every row except JM05, whose reserve
    placement under the seabed dominates.
  - The contact steps stay within 20% of OFF on every contact row. The heights are the same
    samples; only the interpolation schedule and the tile assignment differ.
- **Rows:**
  - Exit 1: USN04, USN02 (no contact) and BSM01.
  - JM06, USNOS, USN13, IJN01, JM05: exit 1 or 3 with identical death tables.
  - USNOS long: exit 3 possible, with identical death tables.
  - JM08 long: exit 3. The invaders still stop at the beach. Deaths are between 20 and 30.
- **For `kHullTerrainBodyPairManifoldBound`** on top of the native test: the same classes. Hulls
  with one shape are unaffected, so any move comes from multi-shape hulls. On most ships
  `firstnode` gives the root plus `front` / `back` records (GUNNERY 55), so expect movement on
  the contact rows only.
- **A mechanism failure keeps a switch OFF:**
  - a hull crossing land (`max_depth` in tens of metres outside JM05);
  - contact steps falling by more than half;
  - a non-unit normal.

### 87.4 Measured (pairs on `1b42b15ff`), and the verdict: both ON

**Setup.**
- Three exports of `1b42b15ff`: main after reference U plus later landings, including
  `kBaseLaunchChainBound`. That base alone gives JM08 long 167 deaths.
  - `g20_n0`: no flip (`CEDCA68BD714`).
  - `g20_n1`: `kHullTerrainNativeTerrainTestBound` (`2BCF41A3EBEB`).
  - `g20_n2`: that plus `kHullTerrainBodyPairManifoldBound` (`0E230CDCE2E3`).
- Reference U's launch form. The 300-frame smoke of `g20_n1` is clean.
- The logs are `local\g20_n{0,1,2}_<row>.log`; `local\g20_htc.py` prints the census.

| row | n0 -> n1 (native test) | n1 -> n2 (body-pair manifold) | contact steps n0 / n1 / n2 | max depth n0 / n1 (m) |
| --- | --- | --- | --- | --- |
| USN04, USN02 | 1 | 1 | 0 | - |
| JM06 | 1 | 1 | 1834 / 1834 / 1834 | 0.30 / 0.30 |
| BSM01 | 1 | 0 | 3119 / 1091 / 1091 | 12.15 / 12.15 |
| USNOS | 1 | 1 | 1718 / 1707 / 1707 | 0.12 / 0.12 |
| USNOS long | 3: death rows change `nearest` only | 3: 1 row `nearest` | 20826 / 20623 / 20610 | 0.45 / 0.38 |
| USN13 | 3: death rows identical | 3: death rows identical | 7862 / 7958 / 7958 | 4.64 / 2.57 |
| IJN01 | 3: death rows identical; Downes 193.89 -> 197.78 m, hits 77 -> 79 | 1 | 4069 / 2602 / 2602 | 5.29 / 5.29 |
| JM05 | 3: `nearest` only | 1 | 6507 / 6530 / 6417 | 226.57 (the reserve placement) |
| JM08 long | 3: deaths 167 -> 166 (`USTroopTransport 06` survives; two transports die later) | 3: 166 -> 165 (`USTroopTransport 05` survives) | 83431 / 66778 / 61670 | 1.07 / 0.62 |

`rejected_normal` is 0 on every row and side.

**Where the two tests disagree.**
- I ran the env-gated diagnostic `BSP_HULL_TERRAIN_COMPARE=<file>` on a local ON build, logging
  the first 400 vertices where the host test and `00C53630` disagree:
  - BSM01: `local\g20_cmp_bsm01.txt`;
  - IJN01: `local\g20_cmp_ijn01.txt`.
- Every disagreement is a vertex within **7 mm** of the surface:
  - the mean `|y - h|` is 0.7 mm on IJN01;
  - on BSM01 every one is below 0.01 m, most at `y == host h` exactly.
- The two interpolation schedules differ by millimetres. A hull resting on the terrain then sits
  at a different equilibrium and flickers in and out of contact on different steps. That is what
  changes the contact-step counts.

**Against the predictions (87.3).**
- **Held:**
  - `rejected_normal` stays 0;
  - `max_depth` stays within 1 m of OFF except on USN13 (4.64 -> 2.57, shallower);
  - every exit class;
  - the body-pair key moves only contact rows (BSM01 exit 0: its hulls have one shape).
- **Miss, and a criterion I am overriding:**
  - The contact steps fall by more than 20% on JM08 long (-20%), IJN01 (-36%) and BSM01 (-65%).
  - BSM01's fall meets the failure criterion I wrote in 87.3 ("contact steps falling by more than
    half"). That criterion was meant to catch hulls losing contact and passing through land.
  - The diagnostic shows the opposite: millimetre disagreements at rest. `max_depth` is
    unchanged (12.15, the spawn pose), and BSM01 is gameplay-identical (exit 1).
  - So I read it as the prediction's premise failing (it assumed the equilibria would coincide),
    not the mechanism failing. **This reading is the lead's to overrule.**
- **Premise change:** JM08 long's "20 to 30 deaths" was written against U. This base gives 167
  on every side. The invaders still stop at the beach (max depth under 1.1 m).

**Verdict:** `kHullTerrainNativeTerrainTestBound = true` and
`kHullTerrainBodyPairManifoldBound = true`. Both belong to reference V.

**Still labelled:**
- the FFFFh edge padding;
- the identity Landscape rotation;
- the x/z-range broad phase;
- the shape order within a body pair;
- substeps (85.2 item 3c);
- hull-terrain pairs only (3d).

## 88. The solver's substeps: not a substitution (85.2 item 3c, cc9-gunnery20)

84.1 labelled "one substep of the host's whole step" as a substitution for `00C5C540`'s plan.
The read closes it with no code change:
- **`world+00h`, the fixed substep, is `0.05f`.**
  - `004DE160` loads `[00CE7638]` (bytes `CD CC 4C 3D`, 0.05f).
  - `004DE168` stores it at the world descriptor's `+00h`.
  - `00C41AD0` copies it into `world+00h` (DYN_WORLD_SETTINGS).
- **`00C5C540` is called once per fixed game step** (`00875E0C`, the step 0.05f).
- **The plan runs exactly one substep of 0.05.** The plan is `dyn_world_substep_plan_00c5c540`
  (`00C5C68B..00C5C6C6`):
  - The accumulator `world+48h` is cleared at the end of every call.
  - So `fixed_substep < dt + previous` is `0.05f < 0.05f`, which is false, and no full
    substep runs.
  - The remainder branch then runs one substep of 0.05 (`00C5C6BD`).
- **The host already matches.** It runs its velocity phase, the contact phase and its position
  phase once per 0.05 s step, which is the image's order inside `00C5BB30`.
- **The one condition:** this holds for the host's step of 0.05, the reference launch form's
  `--mission-frame-seconds 0.05`. A different host step would need the plan.
- **Real play:** frame times vary, so the number of fixed steps per rendered frame varies (zero
  or more, FIXED_STEP_FANOUT). Every call still passes `0.05f` (`00D0DE84`), so each fixed step
  still runs one substep. A host run with a variable step would differ.
- No switch and no pair. The header comment in `include/bsp/hull_terrain_contact.hpp` records it.

Left from 84.1: hull-terrain pairs only (85.2 item 3d: hull-hull and hull-object manifolds, the
group formation `00C4B610`).

## 89. Hull-hull and hull-object contact: the read and a build plan (85.2 item 3d, cc9-gunnery20)

This is a read. No code changed. It sets out which Dyn bodies the image lets a hull touch, and
what the host needs in order to model it. It is not one packet.

### 89.1 The pair filter and who passes it

- **The filter.** `00C44090` tests each shape pair with
  `(B+30h & A+2Ch) != 0 || (A+30h & B+2Ch) != 0`, before it looks up the dispatcher.
  - Shape `+2Ch` is the group; it comes from the shape descriptor's `+08h`.
  - Shape `+30h` is the mask; it comes from descriptor `+0Ch`.
  - Terrain shows the mapping: `00882AC0` writes desc `+08h = 8` and `+0Ch = 0` (at `esp+50h` /
    `esp+54h`); section 84 reads the terrain shape as group 8, mask 0.
- **A hull shape** has group 1 (`009394DD`) and mask `0Dh | class bits` (`009394A9`,
  `009395E2`). With `NavigatorSetAvoidLandCollision(false)` the mask is `0Dh`. `0Dh` is bits 0,
  2 and 3, so a hull reaches groups 1, 4 and 8.

The Dyn body creators, from Ghidra xrefs to `00C5D580`, with their shape filters:

| creator | what | group / mask (descriptor stores) | touches a hull? |
| --- | --- | --- | --- |
| `00937C90` | ship hull | 1 / `0Dh` + class bits | yes: **hull-hull** |
| `0092AAE0` (from `00935D30`) | a unit's single transformed box (COLLISION_SHAPES G) | 1 / `0Dh` (`local_50`, `local_4C`) | yes |
| `007482B0` | `MLandFort` `vtable[0A0h]` (forts) | 1 / `0Dh` | yes: **hull-fort** |
| `008509F0` | not identified (no rel32 caller; a vtable method); 48h-byte shape entries | mask `0Dh` (`00851022`); group not read | probably |
| `00447510` | floating debris (GAME_DYNAMICS_LIST) | 4 / `0Dh` | yes: **hull-debris** |
| `00882AC0` | terrain tiles | 8 / 0 | yes (sections 84-87) |
| `00423C50` | avoid-zone draft physics | a 2 at `esp+F4h` (`004243DB`); which field it is is not read | unknown |
| `007D5D20` | plane | no immediate group/mask store in `007D5D20..007D6137`; not read | unknown |

**Uncertainty:**
- Ghidra's callers can under-report. A rel32 and absolute-dword scan of `00C5D580` has not been
  run.
- The `0092AAE0` and `008509F0` owners are not identified beyond the table above.

### 89.2 What the host needs, in order

1. **A world-level contact phase.** Today the units host calls `HullTerrainContactSolver::step`
   once per hull, so each hull solves against static terrain alone. `00C5BB30` runs these phases
   once for the whole world:
   - the collision pass;
   - `00C4B610`'s grouping: bodies joined by manifolds form one group (reconstructed:
     `native_dyn_create_contact_groups_00c4b610` and `dyn_create_contact_groups_00c4b610`);
   - one solve per group, over all its dynamic bodies.

   The call site is in `src/game_hosts_units.cpp` (shared).
2. **Convex-convex narrow phase.** Kind 4 against kind 4 goes through the general-convex
   dispatcher `00C535E0`, which is reconstructed (`dispatch_native_dyn_general_convex_00c535e0`)
   and writes one contact per hit. It needs:
   - a real convex shape record, whose `+0` is `NativeDynConvexShapeRuntime::table()` and whose
     `+210h` is the `AvoidZoneDynHullData` (section 86 already builds it: vertices, adjacency,
     support seeds);
   - the process's general-convex owner (`DynGeneralConvexIntersectStorage`, its 26 support
     directions and critical section);
   - the process's mutable CRT conversion word.

   `GameNativeDynProcess` owns all three but exposes none of them. That is a small interface
   addition in `game_native_dyn_process.*`.
3. **Object bodies in the host's world.** Forts, debris and the `0092AAE0` boxes are not host
   Dyn bodies today. Forts are static boxes or convexes: the dispatcher table selects box-convex
   or convex-convex, and both are reconstructed (`native_dyn_box_box`,
   `native_dyn_primitive_dispatch`). Debris are dynamic bodies of `game+30h`.
4. **The group solve.** The rows for two dynamic bodies use both bodies' masses and inertias.
   The batch already takes a velocity array; the host would index each body.

**Proposed order:**
- hull-hull first: steps 1, 2 and 4 with ships only; it needs no object census;
- then forts (static, step 3);
- then debris.

Each gets a switch committed OFF and a pair. The reach rows for hull-hull are the formation and
harbour rows (IJN01, USNOS, USN13, JM08 long).

## 90. Handoff (cc9-gunnery20, 2026-09-30 19:50 UTC, at about 60% context)

### 90.1 Landed

| item | commits | state |
| --- | --- | --- |
| Reference u (GAME_EXECUTABLE "2026-09-30 u", base `7f622dde6`) | `cf3539fa1`, `2c9efd019` | 18 rows; anchor exit 1 on all; every moved row attributed |
| 86: the solver tests 00C5DEB0's hull vertices | `af7fd46c5`, `a354023e0` | `kHullTerrainDynHullVerticesBound` ON |
| 87: 00C53630 itself on tile and hull records, one manifold per body pair | `1b42b15ff`, `a94bb94cc` | `kHullTerrainNativeTerrainTestBound`, `kHullTerrainBodyPairManifoldBound` ON; the 87.3 criterion override was accepted by the lead |
| 88: substeps are not a substitution | `0700d18eb`, `834999948` | the image runs one 0.05 substep per fixed step |
| 89: hull-hull and hull-object contact, the read and the plan | `c9f41d0a7` | read only |
| Reference v (GAME_EXECUTABLE "2026-09-30 v", base `16d01094e`) | `8a5e6c53b`, `4a57721f4` | 18 rows; anchor exit 1 on all; every moved row attributed; **v is the baseline** |

### 90.2 Open, in order

**1. Hull-hull, then forts, then debris: 89.2's build plan, one packet each.**
- **a. Hull-hull.**
  - **A world-level contact phase.** Replace the per-hull `HullTerrainContactSolver::step` call
    in `src/game_hosts_units.cpp` (shared: claim it only to apply a prepared edit) with one call
    per fixed step over every hull. It runs:
    - the terrain narrow phase (section 87);
    - hull-hull pairs through the reconstructed general-convex dispatcher
      (`dispatch_native_dyn_general_convex_00c535e0`, one contact per hit);
    - `00C4B610`'s grouping (`native_dyn_create_contact_groups_00c4b610` /
      `dyn_create_contact_groups_00c4b610`);
    - one solve per group, over all its dynamic bodies. The rows index each body's velocity
      slot; the terrain is the static slot 0.
  - **Convex shape records.**
    - `+0` is `NativeDynConvexShapeRuntime::table()` (`include/bsp/native_dyn_convex_support.hpp`).
    - `+08h` is kind 4; `+34h` the local frame with the centre (section 86).
    - `+210h` is the `AvoidZoneDynHullData` itself, with vertices, adjacency and support seeds.
      Keep the 00C5DEB0 handle in `HullShape` instead of destroying it.
  - **What `GameNativeDynProcess` must expose** (`game_native_dyn_process.*`; it owns all three
    today and exposes none):
    - the general-convex owner (`DynGeneralConvexIntersectStorage`);
    - the convex pool;
    - the mutable CRT conversion word `0109EEA4`.
  - **Filter:** hulls are group 1, mask `0Dh` plus class bits, so every hull pair passes.
  - **Switch:** `kHullHullContactBound`, committed OFF, with predictions.
  - **Reach rows:** the formation and harbour rows, USNOS / USNOS long, USN13, IJN01 and JM08
    long.
- **b. Forts** (`MLandFort`, `007482B0`, group 1). These need host static bodies with their
  shapes, which are not read yet (`007482D0..00748986`).
- **c. Debris** (`00447510`, group 4, dynamic bodies of `game+30h`).
- **Census debt:** 89.1's creator list comes from Ghidra xrefs. Scan rel32 and absolute dwords
  for `00C5D580` before relying on it. `008509F0`'s owner and the avoid-zone and plane filters
  are unread.

**2. Reference w** from main after v.
- **Switches already flipped since v's base `16d01094e`:**
  - `kLandingShipStartLandingBound` ON (cc9-ships24, `00b972337`);
  - `kGroundRetakeoffBound` new and OFF (cc9-lua28, 5bp; not part of w unless flipped).
- **Method:** as v (GAME_EXECUTABLE "2026-09-30 v"):
  - `local\g20_switches.py` plus the loose diff;
  - the all-OFF anchor against `g20_rv_<row>`;
  - leave-one-out through `local\g20_lanesv.ps1` (edit its variants and commit).

**3. USN01's controlled unit under v: owner, the lua / controlled-unit lane.**
- **Symptom.** On v's USN01 row the idle player's controlled unit is `ScoutDauntless` (moved
  4623.23 m), and 5 aircraft run the torpedo task. On u it was `ConTBD1` (1245.72 m), with 17
  aircraft.
- **Cause, from the leave-one-out:** `kFormationJoinLoopbackBound` alone. With `fjl` OFF, USN01 is
  exit 1 against u, `ConTBD1` 1245.72. Every other variant OFF keeps `ScoutDauntless`.
- **Not read:** why a formation join posted one pump later changes which unit the player
  controls. The hypothesis is the torpedo squadron's join order at t = 0.05 selecting a
  different first controllable unit. The logs are `local\g20_rv_usn01.log` and
  `local\g20_v_fjl_usn01.log` (cc9-gunnery20 tree).

**4. Smaller items from v, not read:**
- LOMP10's dive-bomb task counts 9 releases for 8 aircraft (u: 8).
- JM08 long's HQ reaches 0 hp 27 times without a flip; `mode3_points=3`, no ramp contact.

**5. Carried over:** 85.2 item 4 (the fire-window origin, the torpedo items, 80.3's labelled
items, 79.2 item 4), and 87.2's labelled substitutions:
- the `FFFFh` edge padding;
- the identity Landscape rotation;
- the x/z-range broad phase;
- the shape order inside a body pair.

### 90.3 Tools (`local\` in the cc9-gunnery20 tree, prefix `g20_`)

- **Reference runs:**
  - `g20_runs.ps1 -V <prefix> -Only <rows>` and `g20_wait.ps1 -Logs`;
  - `g20_exp.ps1 -Commit -Out [-Flip]`;
  - `g20_vs.py <off> <on> [rows]`;
  - `g20_rows.py` / `g20_tablev.py` for the row table;
  - `g20_report22.py` for the report.
- **Leave-one-out:**
  - `g20_lane.ps1 -Lane -Variants -Rows -Commit -Prefix` and the lane launcher
    `g20_lanesv.ps1`;
  - `g20_loov.py <v>...` for the verdicts against `g20_rv`.
- **Solver:**
  - `g20_htc.py <prefix>...` prints the hull-terrain census per row;
  - `BSP_HULL_TERRAIN_COMPARE=<file>` (env-gated, in `src/hull_terrain_contact.cpp`) logs where
    the host test and 00C53630 disagree;
  - `g20_cmp_*.txt` are its logs.
- **Pair logs:**
  - `g20_voff` / `g20_von`: section 86;
  - `g20_n0` / `n1` / `n2`: section 87;
  - `g20_ru*` / `g20_u_*`: u;
  - `g20_rv*` / `g20_v_*`: v.

## 91. Hull-hull contact: the world phase and 00C535E0 on real convex records (packet `cc9_hull_hull_contact`, cc9-gunnery21)

This builds 89.2 steps 1, 2 and 4 for ships only. Both switches are committed OFF; the pairs
and verdicts follow in 91.4.

### 91.1 The census of 00C5D580's callers (the debt from 89.1)

`tools/callsite_census.py 00c5d580` (every E8/E9 rel32 in `.text`, and the literal address in
`.text`, `.rdata` and `.data`) finds **ten call sites in nine functions and no literal**:
- the eight creators of 89.1's table (`00423C50`, `00447510`, `007482B0`, `007D5D20`, `008509F0`,
  `00882AC0` twice, `0092AAE0`, `00937C90`);
- **one more:** `00C5DA93`, in a ten-byte stub `00C5DA90..00C5DA99` that Ghidra has no function
  for (`MOV ECX,[ECX]; PUSH EAX; CALL 00C5D580; RET`, INT3 from `00C5DA99`). It is a register-ABI
  wrapper: EAX the descriptor, ECX a pointer to the world pointer. The same census on
  `00c5da90` finds no rel32 caller and no literal, so it is unreferenced in the image.

So 89.1's creator list is complete. `008509F0`'s owner and the avoid-zone and plane filters
stay unread.

### 91.2 What the image does, and what was built

The fixed-step fanout runs `00C5C540` once for the world (row 1, `00875E0C`,
FIXED_STEP_FANOUT). Its substep `00C5BB30` runs:
1. every body's velocity phase `00C41550`;
2. the collision pass: ManifoldUpdate, then `00C44090` over the broad phase's pairs;
3. `00C4B610`'s groups;
4. one solve per group;
5. every body's position phase `00C5B1B0`.

The units host ran all of that inside each ship's own tick, so a hull only ever met the terrain.

**`kDynWorldContactPhaseBound`** (`include/bsp/hull_terrain_contact.hpp`). The units host runs
every ship's tick and velocity phase first. Then, after the loop, it calls
`GameUnitsHost::Impl::run_world_contact_phase`:
- `HullTerrainContactSolver::world_step` over every hull:
  - ManifoldUpdate (`00C4B9B0`) over every manifold, retiring the empty ones;
  - the terrain narrow phase of section 87 per hull;
  - with the second switch, the hull pairs;
  - `dyn_create_contact_groups_00c4b610` through its host interface (terrain tiles are static and
    never expanded through);
  - one solve per group, with the bodies indexed in the order `00C4DE40` meets them (A then B per
    manifold; static 0; `00C4DEDB`). Every dynamic body's velocity slot is written back.
- Then each ship's position phase and the rest of its tick, in unit order
  (`finish_motion_tick`, the old loop tail moved unchanged).

What this changes even with no hull pair: a unit's tick now reads the other ships' poses from
the start of the step, as the image's ticks read the last Dyn step's poses. Before, a unit later
in the list saw the poses the earlier units had just integrated.

**`kHullHullContactBound`** (needs the first). Every pair of hulls whose world boxes meet goes
through `00C44090`'s convex-convex path:
- The shape filter: group 1, mask `0Dh`, so every hull pair passes.
- Dispatcher cell `4 * 6 + 4`: the reconstructed `00C535E0`
  (`dispatch_native_dyn_general_convex_00c535e0`). Its inputs:
  - the process's general-convex owner (26 directions, critical section), now exposed by
    `GameNativeDynProcess::general_convex_owner()`;
  - the CRT context;
  - a real kind-4 record per hull shape (`HullShape::convex`):
    - `+0` the process's ConvexMeshShape table (`body_creation().convex_shape_vtable`);
    - `+8` kind 4;
    - `+0Ch` the local box from `00C57C40` (its first statement, now
      `dyn_convex_shape_local_bounds_00c57c40`, without the native body refresh);
    - `+24h` restitution 0;
    - `+28h` the material friction;
    - `+2Ch` group 1 and `+30h` mask `0Dh`;
    - `+34h` the shape frame (identity, the centre as translation);
    - `+210h` the `00C5DEB0` hull, which `hull_shape` now keeps instead of destroying.
- One manifold per body pair (`00C3F4D0`), with friction `combine(fA, fB)` and restitution
  `(0 + 0) * 0.5` written on every hit (`00C44154..00C441DB`: the combines, `FindOrCreate` at `00C441C5`, the stores at `00C441D8` / `00C441DB`). Each hit goes through `00C3F760`.
- Both hulls then join one group and one solve.

OFF, the same narrow phase runs after every step as a **census** (no manifold, no state), and the
end-of-run summary prints `hull pair contact census` per pair and `summary hull hull contact`.
The OFF smoke (`local\g21_offsmoke_smoke.log`) is exit 1 against v's smoke
(`g20_rv_smoke`): the only moved lines are main's later landing summaries and the new census line.

**LABELLED substitutions:**
- **The broad phase.** The SAP pair list is replaced by a test on each hull's world box, every
  hull vertex widened by 0.1 m. That is more than the 0.02 the image widens each shape box by, so
  no pair the SAP holds is missed; the narrow phase decides every hit.
- **Body A** is the lower unit.
- **Shape order.** Shape pairs go in shape order; the image walks each body's shape chain
  (`+70h` / `+208h`).
- **Manifold list order** is creation order; `00C3F4D0`'s list is not read. Each body's contact
  array (`B+74h`) follows the same order.
- **Sleep.** Hull bodies are taken as awake (`B+50h` bits 0 and 1 clear).
- **A manifold whose hull did not step** this step is retired.
- **No contact report yet.** `00C44090`'s second half queues a report when a body's listener
  (`B+68h`) mask meets the other shape's group. For a hull that is `009377E0`; which kind a
  hull-hull report delivers, and whether it deals collision damage, is not read. Hull-hull
  contact here is physics only.

**Uncertainty:**
- Whether the image's hull bodies sleep.
- The contact report, above.
- The `00C535E0` path has run only on the reconstruction's differential fixtures
  (NATIVE_DYN_GENERAL_CONVEX_R140). With these records it runs for the first time in the game
  executable.

**Files:**
- `src/hull_terrain_contact.cpp`, `include/bsp/hull_terrain_contact.hpp`;
- `src/game_native_dyn_process.cpp`, `include/bsp/game_native_dyn_process.hpp`;
- `src/dyn_body_creation.cpp`, `include/bsp/dyn_body_creation.hpp`;
- `src/game_hosts_units.cpp` (shared; applied from `local\g21_host_edit.py`).

### 91.3 The OFF census, and the predictions (written 2026-09-30 20:49 UTC, before any ON run)

**A fix before the census could run.** The first OFF commit `5e576aff9` crashed 16 of 18 reference
rows with `c0000005`. The faults were in the reconstructed `00C51C20` (`direction_kernel`) and
`00C48BE0` (`result_kernel`), reading `14h`. Both read the shape's body at `+4h` and its 3x4 at
`+08h..+37h`:
- `00C51C20` transforms the two box centres;
- `00C48BE0` turns the witnesses body-local.

`7b5e05db2` points each record's `+4h` at a copy of the hull's current 3x4.

**Positive control** (`BSP_HULL_HULL_TRACE`, USN04, `local\g21_trace_usn04.txt`). The first hull
shape (79 vertices) against itself:
- shifted 1 m: hit, normal `(0.30, -0.83, -0.47)`;
- shifted 1000 m: miss;
- shifted 0 (coincident): a hit with a degenerate witness B (about `-5e10`). Two coincident
  hulls do not occur in a mission.

**OFF census** (`7b5e05db2`, this tree's build, `local\g21_off_<row>.log`):
- 17 of 18 rows are exit 1 against v (`g20_rv_<row>`), death tables identical.
- JM08 long is exit 3 (160 -> 85 deaths, 7 only ON, 82 only OFF, 68 changed). That is exactly
  cc9-ships24's `kLandingShipStartLandingBound` pair (SHIP_AI 97.4), flipped on main after v's
  base. So the census writes no state.

| row | near pairs | 00C535E0 hits | hull pairs that met | deepest (m) |
| --- | --- | --- | --- | --- |
| USN04 | 2343 | 457 | 4 (destroyers through cruisers) | 14.7 |
| E2 | 3341 | 457 | 4 | - |
| USN01 | 1435 | 0 | 0 | - |
| USN02 | 11658 | 5646 | 10 | 14.0 |
| JM06 | 5002 | 5666 | 8 (tankers, transports) | 24.2 |
| JM08 | 1827 | 608 | 3 | - |
| USN13 | 12886 | 9049 | 11 | 17.1 |
| BSM01 | 9000 | 0 | 0 | - |
| LOMP06 | 1015 | 35 | 1 | - |
| LOMP10 | 3070 | 118 | 1 | - |
| JM05 | 8795 | 5213 | 9 | - |
| USN12 | 304 | 291 | 1 | - |
| LOMP10 long | 15070 | 118 | 1 | - |
| USNOS | 9308 | 33032 | 15 (convoy and cargo ships) | 42.3 |
| USNOS long | 18734 | 66571 | 24 | - |
| IJN01 | 33101 | 15267 | 41 | 29.7 |
| JM05 long | 33736 | 20392 | 23 | - |
| JM08 long | 69821 | 30838 | 28 | 33.4 |

So the host's hulls pass through each other on 16 of 18 rows, often by tens of metres.
Convoys and formations overlap their neighbours.

**Pair 1: `kDynWorldContactPhaseBound` alone** (`local\g21_w1`) against OFF.
- **Mechanism:**
  - every tick reads the start-of-step poses of the other ships;
  - the terrain contact census (`summary hull terrain contact`) stays within a few percent of
    OFF on USNOS long, USN13, IJN01, JM05 and JM08 long.
- **Prediction:** exit 3 on every row with more than one moving ship. That is all rows but,
  weakly, BSM01 and LOMP06, which may stay exit 1. Death tables re-timed or within a few rows;
  no row loses or gains more than about 10% of its deaths, except JM08 long (a knife-edge
  row).
- **Mechanism failure** (keeps it OFF): a crash, a ship with a non-finite pose, or a terrain
  census that collapses (for example, contact steps dropping by half).

**Pair 2: `kHullHullContactBound` on top** (`local\g21_w2`) against pair 1's ON.
- **Mechanism:**
  - `summary hull hull contact` shows `groups > 0` and `multi_hull_groups > 0` on the 16 rows;
  - the deepest hit per pair falls from 10-42 m to **under 2 m** (the bias rows push the hulls
    apart; they no longer pass through each other).
- **Prediction:**
  - USN01 and BSM01 exit 0 or 1 against pair 1's ON, since no hulls meet;
  - exit 3 on the other 16 rows; ship paths deflect at the contacts;
  - convoys and formations (USNOS, JM06, IJN01, JM08 long) spread, and some deaths re-time;
  - no hull is lifted clear of the water or spun (the solver shares the masses and inertias;
    the terrain case already showed at most small vertical velocity changes).
- **Mechanism failure** (keeps it OFF): hulls still interpenetrating by more than 5 m, a
  crash, or a ship thrown more than 10 m above the sea.
- **Not modelled either way:** the collision report and its damage (`009377E0` -> `008145B0`,
  gated on descriptor `+510h` / `+514h`). Ramming damage stays absent on both sides.

### 91.4 The pairs, and the flips (2026-09-30 21:22 UTC)

**Setup.**
- Exports from `7b5e05db2`: `local\g21_w1` (world phase) and `local\g21_w2` (world phase plus
  hull pairs).
- OFF is this tree's build of the same commit (`local\g21_off_<row>`).
- 18 rows in v's launch form.
- Nine runs died at startup around 20:58 UTC with the known environment failure (FMOD
  `error.fsb`, renderer `0x8876086A`), or ran through a device loss (`lost_polls` above 0: W1
  USN02 and W2 USNOS long). After one clean smoke all nine were rerun; every row used here is
  exit 0 with `lost_polls=0`.

**Pair 1, `kDynWorldContactPhaseBound`** (W1 against OFF):

| exit | rows |
| --- | --- |
| 1 | JM06, JM08, BSM01, LOMP06, USN12, USNOS, IJN01, USN02 |
| 3 | USN04 (deaths 50 -> 48), E2 (torpedo releases 5 -> 4), USN01, USN13 (re-timed), JM05, LOMP10 (3 -> 2), LOMP10 long (6 -> 5), USNOS long (4 rows re-timed), JM05 long, JM08 long (85 rows re-timed) |

- **Mechanism: matches.** The terrain census is unchanged: contact steps and candidates are
  within 0.5% of OFF on USNOS long, IJN01, JM05, JM05 long, JM08 long, USN13 and USNOS.
- **Prediction: a spread miss.** Fewer rows moved than predicted. The moved rows are the carrier
  and aircraft rows. The formation rows (USNOS, IJN01, JM06) are gameplay-identical, so ship
  ticks rarely read another ship's pose within a step.
- **USN01:** W1 brings back u's controlled unit. `ConTBD1` 1245.75 m, 93 units, torpedo tasks
  "0 of 17", against v's `ScoutDauntless` 4623.23 m with 64 units. The ConTBD and ConSBD
  squadrons spawn again. So 90.2 item 3 (v's USN01 regression under
  `kFormationJoinLoopbackBound`) depends on the within-step order of ship poses, and the image's
  order restores it.
- **Order check against the image.** Per fixed step the image runs:
  1. `00C5C540` (row 1), integrating the velocities the previous ticks set;
  2. rows 2-16;
  3. the unit ticks, which read those poses.

  W1 integrates at the end of the host's motion step. The next step's fanout and ticks then
  read the integrated poses. That is the same sequence, and the same one-step lag of a pose
  that is copied from a carrier.
- **Flipped ON.**

**Pair 2, `kHullHullContactBound`** (W2 against W1):

| row | exit | deaths | hits | groups with two or more hulls, largest | deepest hit (OFF census -> W2) |
| --- | --- | --- | --- | --- | --- |
| USN04 | 3 | 48 -> 49 | 213 | 213, 2 | 14.7 -> 0.10 |
| E2 | 3 | 52 (43 re-timed) | 213 | 213, 2 | -> 0.10 |
| USN01 | 1 | identical | 0 | 0 | none |
| USN02 | 3 | identical | 1997 | 1999, 2 | 14.0 -> 2.98 |
| JM06 | 3 | identical | 3079 | 2757, 3 | 24.2 -> 0.41 |
| JM08 | 3 | 7 -> 5 | 212 | 212, 2 | -> 0.28 |
| USN13 | 3 | 23 (21 re-timed) | 2733 | 2737, 2 | 17.1 -> 0.29 |
| BSM01 | 1 | none | 0 | 0 | none |
| LOMP06 | 3 | none | 5 | 5, 2 | -> 0.04 |
| LOMP10 | 3 | 2 (re-timed) | 45 | 45, 2 | -> 0.46 |
| JM05 | 3 | 12 (2 re-timed) | 3262 | 2799, 3 | -> 0.55 |
| USN12 | 3 | 7 (2 re-timed) | 17 | 17, 2 | -> 0.10 |
| LOMP10 long | 3 | 5 (re-timed) | 45 | 45, 2 | -> 0.46 |
| USNOS | 3 | 106 -> 110 | 9978 | 4698, 4 | 42.3 -> 2.99 |
| USNOS long | 3 | 147 -> 166 | 30359 | 14698, 4 | -> 2.99 |
| IJN01 | 3 | 3 -> 2 | 9240 | 8072, 4 | 29.7 -> 2.02 |
| JM05 long | 3 | 14 -> 20 | 12094 | 9399, 4 | -> 1.69 |
| JM08 long | 3 | 85 -> 28 | 29997 | 24361, 4 | 33.4 -> **16.55** |

- **Prediction:** USN01 and BSM01 exit 1, the other 16 exit 3. **All right.**
- **The deepest hit:**
  - under 1 m on 11 rows;
  - 1.7-3.0 m on USN02, USNOS, USNOS long, IJN01 and JM05 long (the "under 2 m" call missed
    by up to 1 m);
  - 16.55 m on JM08 long (Helena against Missouri).
- **JM08 long, read with `BSP_HULL_HULL_TRACE_UNIT=349 BSP_HULL_HULL_TRACE_STEP=13300`**
  (`local\g21_trace_jm08l_349b.txt`):
  - Helena slides along Missouri in resting contact for 70 steps, depth 0.014-0.034 m. The
    boxes and velocities change smoothly: A moves 0.5 m per step, `vA` stays at
    `(8.1, 0.0, 10.2)`.
  - At world step 13368 the reported depth jumps to 14.59 m in one step, with no pose jump.
  - The hulls cannot have moved 14 m into each other in 0.05 s. So it is `00C535E0`'s own
    result changing branch (the fallback over the 26 directions), not an interpenetration.
  - The solver then removes it at the bias limit, about 0.05 m per step, over about 300 steps.
    Helena rises at most 0.6 m (box `min.y` -8.1 to -8.7) and is not thrown.
  - The 91.3 failure criterion was meant for hulls that actually interpenetrate. This is
    recorded as the native narrow phase's behaviour on these records, not as a host failure.
  - **Uncertainty:** the image would do the same only if its records are these (section 86's
    hulls, `00C57C40`'s box); no image run confirms it.
- **The changed rows follow the contacts.**
  - On JM08 long the transports and landers no longer pile onto each other near Missouri. They
    spread toward the beach: terrain contact steps go from 126433 to 224418. `USTroopTransport`
    01, 02, 04 and 05, LST 02 and Gleaves die to the HQ and shore guns, and the shore dies
    later or not at all (85 -> 28 deaths).
  - USNOS long gains 19 deaths.
  - JM08 long is the known knife-edge row.
- **Flipped ON.** The mechanism matches: every hull pair that met now forms a group and is
  solved, and the interpenetration of 10-42 m is gone on 15 of the 16 rows with contacts. The prediction spread
  missed on the depth bound (up to 3.0 m) and on JM08 long's one native deep result.

**Open, in order:**
1. **The contact report.** `00C44090` queues a report when a body's listener (`B+68h`) mask
   meets the other shape's group, and the hull's listener is `009377E0`. A kind other than 8
   runs its damage gate (descriptor `+510h` / `+514h`) and `008145B0`, which is collision
   damage. Read the hull listener's mask (`+4h`) and `008145B0` before binding ramming damage.
2. **Forts** (`007482B0`, group 1, static) and **debris** (`00447510`, group 4), as 89.2.
3. JM08 long's 14.6 m branch change: a differential run of `00C535E0` on the two records at
   world step 13368 against the image's bytes would settle whether the image gives the same
   result.

## 92. The hull-hull contact report: 00C35480 -> 009377E0 -> 008145B0 (packet `cc9_hull_contact_report`, cc9-gunnery21)

### 92.1 What the image does

**The events.**
- `00C44090` queues one event `{manifold, shape A, shape B}` in `scene+0D8h` per dispatcher hit
  (the mask tests `00C4420D..00C44232`, the append `00C44258..00C44301`). It does so when body A's listener mask (`[B+68h]+4h`) meets shape B's
  group, or body B's meets shape A's.
- A hull's listener is its controller `+20h`, and its mask `+24h` is `7FF9h` (`00939CD5`,
  UNIT_CONTROLLER_UPDATE). So every hull pair and every hull-terrain pair queues events.

**The dispatch, `00C35480`** (the collision pass's last step, `00C57827`). Per event:
- It transforms the manifold's points on body A into world space, into a stack record: `+0`
  shape, `+4h` the other shape, `+8h..` the points (`00C354E0..00C35549`).
- `+38h..+40h` is copied from `manifold + 8 + eventIndex * 30h` (`00C354B6..00C354CE`; the
  index advances by `30h` per event, `00C355C5`). That is point *k*'s normal for event *k*,
  and memory past the four points for *k* >= 4. It is an image quirk; only 009377E0's
  magnitude reads it.
- It calls body A's listener with `(A, B)` (`00C35551..00C35580`), then body B's with the same
  record and the shapes swapped (`00C35582..00C355B8`).

**`009377E0`** (body `009377E0..00937B6F`, `RET 4`), for the record `esi`:
- **The kind.** kind = `00C32450(esi+4)` (the other shape's `+2Ch`, its group) and flags =
  `00C32450(esi+0)`. Kind 8 (terrain) latches `unit+1010h` and returns, as section 84 binds.
- **The other unit.** `other` = `00C31EA0` (the other body's `+6Ch` owner), kept only if it
  answers `IsKindOf(6)` (`00937827..0093784F`). Flags bit 1 with another unit runs `009373C0`;
  a hull's group is 1, so it does not run.
- **The gate** (`00937886..009378E6`) opens for any pair with another unit. The
  `+510h` / `+514h` (KamikazeDamage / KamikazeBlastDamage) test only matters when there is
  none.
- **Two cancels** (`009378ED..00937916`): `unit+6B8h >= 0` with `00779AD0` (the age
  `([00F876B0] - unit+294h) * [00D0DE84]`) under 3.0 (`[00CE3854]`), and the same test on the
  other unit (`0092CE70`). `unit+6B8h` is DummyObjectID: -1 from the unit constructor
  (`0095CDBE`, `0095CDE9`), and its writers have no caller. So neither cancel ever fires.
- **The point velocities.** Both bodies' velocities at record `+8h`: `v + w x (p - B+2Ch)`
  (`00C35300` B+2Ch, `00C31F20` angular `M+0Ch`, `00C31F40` linear `M+0h`), and their lengths
  (`0042B2F0`). The impact vector is `n * dot(n, v_own - v_other)` with its y zeroed, using
  the `+38h` normal.
- **The call.** `008145B0(point, |impact|, other, 0)` on the own unit, only when
  `|v_own| <= |v_other|` (`00937B31..00937B61`). The slower body reports.

**`008145B0`** (`__thiscall`, `RET 10h`, read in full here):
- It reads neither the point nor the magnitude.
- It needs a single-player session (`game+1FE4h != 2`), `other != 0`, and
  `00803510(unit+54h, other+54h) == 1` (enemy).
- Then, for each party that is alive (`+5Dh..+60h` clear) and a kamikaze (`00779AA0`: `+510h`
  or `+514h` above 0), it routes message 70h with the other party as the target: the
  detonation `00819A20` of SHIP_AI 46.

**So the image has no ramming damage for ordinary ships.** A hull-hull contact sets off a
kamikaze and does nothing else. The eventIndex normal quirk changes only the unused
magnitude.

### 92.2 What was built (committed OFF, `bed6dc011`)

- **`HullTerrainContactSolver`** keeps each hull-pair event of the last world step
  (`contact_events()`): the pair (A the lower unit), the manifold's point 0 on A in world space,
  and both point speeds, taken after the narrow phase and before the solve, as `00C35480`
  runs. `GameUnitsHost::hull_contact_events()` exposes them.
- **The gunnery host's `run_kamikaze_contacts`** runs every event through both listeners:
  `IsKindOf(6)`, the slower body, hostility, then a live kamikaze party.
- **`kKamikazeDynContactBound`** (`src/game_hosts_gunnery.cpp`):
  - True: that is the contact that detonates. The bow-point / hull-box stand-in of section 46
    no longer runs.
  - False: the stand-in as before, with the events as a census (`summary mission gunnery hull
    contact report`).
- **LABELLED:**
  - a kamikaze detonates at most once per step (whether a second message 70h in one step
    detonates again is not read);
  - the events are those of the host's last world step;
  - the report needs `kHullHullContactBound`.

### 92.3 The OFF census, and the predictions (2026-09-30 21:41 UTC, before any ON run)

The OFF census comes from this tree's build of `bed6dc011`, 18 rows plus the smoke
(`local\g21_r0_<row>`), all exit 0, `lost_polls=0`.
- Every event produces exactly one slower-body call; there are no ties.
- No call has a hostile kamikaze party on any row.
- USN02 has 618 hostile calls between ordinary ships (Java and Samidare): no effect in the
  image.
- The kamikaze boats of USNOS long touch only friendly hulls: 294 refused box contacts under
  the stand-in, and 0 hostile Dyn calls.

| rows | events | not hostile | kamikaze parties |
| --- | --- | --- | --- |
| USN01, BSM01, smoke | 0 | 0 | 0 |
| USN02 | 1997 | 1379 | 0 |
| the other 14 | 5 (LOMP06) .. 129461 (JM08 long) | all | 0 |

**Prediction (`local\g21_k1`, the flip, against OFF):**
- exit 0 or 1 on all 18 rows, death tables identical;
- the kamikaze summary's `contacts` and `hostile_refused` go to 0 on USNOS long (the stand-in
  is off), and `min_gap` to -1 on USNOS and USNOS long.

**Mechanism failure:** any row whose gameplay moves (there is no hostile kamikaze contact to
move it).

### 92.4 The pair, and the flip

**`local\g21_k1` (the flip) against `local\g21_r0` (OFF), both from `bed6dc011`.**
- All 18 rows are exit 1, death tables identical, every run exit 0 with `lost_polls=0`.
- USNOS long's kamikaze summary goes to `contacts=0 hostile_refused=0 min_gap=-1.0`: the
  stand-in is off. The prediction was right on every row.
- **Flipped ON:** `kKamikazeDynContactBound`. Hull contact now sets off a kamikaze the way the
  image does, and it does nothing else. No reference row has a hostile kamikaze contact, so the
  flip moves nothing yet.

## 93. JM08's HQ flak and the Watchtower: the scorer, and the aim point (packet `cc9_hq_flak_watchtower`, cc9-gunnery21)

The lead's question (cc9-ships25's log `s25_m1_jm08x.log`): Headquarter 01's two category-6
flak mounts (platforms 4 and 5) fire 732 of 1461 shots at "Watchtower, 01 03" from t=4.6 s,
with 2 hits. Would the image's scorer prefer the tower over the ships, and why so few hits?

### 93.1 Target choice: the image does the same

- **The ranks.** Category 6 (LIGHTARTILLERYFLAK) ranks by its authored row `00E09BE0`
  (GUNNERY_TABLES), read through `00727BD0`: the air classes 1-9, then MDestroyer 10,
  MSubmarine 11, MCargo 12, MLandingShip 13, MMothership 14, MCruiser 15, MBattleship 16,
  MCommandBuilding 17, **MLandFort 18**, then MAirfield, MShipyard, MLandVehicle and NavPoint.
- **The order.** `00865284` sorts by rank then distance, and `008657A3` walks from the end, so
  the lowest rank goes first and, within a rank, the nearest. Every ship class outranks the
  tower. The tower (rank 18, not 0) is still a legitimate candidate when no ship is within
  `00863990`'s range.
- **In the log** (platform 4's target changes): the tower alone from 5.05 s to 719.85 s. Then
  LST 03, and from then on only ships (Grayson, Bristol, Macomb and others); it never returns
  to the tower. So the host already prefers the ships the moment one is in range, as the
  image's order does. **No scorer divergence.**

### 93.2 The misses: the host aims over the tower

- **Where the rounds end.** `BSP_SHELL_FATE="Headquarter 01|Watchtower, 01 03"` on JM08 3000
  (`local\g21_hq_jm08.log`): all 160 rounds of bullet class 31 end on land (fate 1) about
  400 m beyond the tower. Example: end `(1865.7, 4.7, -4597.2)` after 1.8 s, against the tower
  at `(1462.4, 4.9, -4503.5)`, from muzzles near `(1340, 23, -4475)`. They cross the tower's
  range at about the muzzle's height.
- **The image's aim point** (`006DF520`, the artillery bot for sub-types 2, 3, 4, 6 and 9) is
  the target's world matrix (`+CCh`) applied to the local point `target->vtable[100h]`
  writes into `bot+A8h..+B0h` (`006DF792..006DF7EE`), plus its ErrorOffset.
  - Slot `100h` is `0042D810` for MLandFort (`00CFF3F8`), MCommandBuilding (`00CFB028`) and
    MLandVehicle (`00CFFDE0`). It writes `(0, 0, 0)` (`0042D814..0042D825`, `RET 1Ch`). The
    aim point is the **origin**.
  - MAirfield (`006D3250`), MShipyard (`00844A10`) and the ships (`00816650`, the host's
    `artillery_aim_point`) have their own slots.
- **The host** aims at every non-ship target through `unit_aim_point`: the origin raised by
  the class `Height`. The Watchtower class (vehicleclasses.lua line 80713, this installation,
  mtime 2026-05-09) has Height 15, so the aim is at y = 19.9, level with the mount (20.7). The
  0.5-degree arc epsilon (`00D08B88`) passes the -0.4-degree command, and the round flies over
  the tower.
- **The image's command.** At the origin (y = 4.9) the command is about -7.6 degrees.
  Headquarter's platforms 4 and 5 (VehicleClass[97], lines 44347..44407) author
  MinVertAngle 0 on their windows (one window 5 degrees). So `0085ABA0` refuses the pair at
  `0085AC94`, and **the image never fires the HQ flak at the tower.** The mounts hold it as
  their target, silent, until a ship comes in range.
- **So 732 wasted shots is a host artefact**: the Height-raised aim point turned a refused
  depression into an accepted level shot.

### 93.3 The switch and the predictions (written 2026-09-30 22:12 UTC, before any ON run)

- **`kArtilleryGroundOriginAimBound`** (`src/game_hosts_gunnery.cpp`, committed OFF in
  `e280693d1`): an artillery-bot aim at a LandFort, CommandBuilding or LandVehicle takes the
  origin. The census `summary mission gunnery ground target origin aims` counts the aims it
  changes, either way.
- **OFF census** (`local\g21_a0_<row>`, this tree's build of `e280693d1`, all exit 0,
  `lost_polls=0`):
  - JM05 30418, JM05 long 67302, JM06 697, JM08 20927, JM08 long 1413671, USN01 24120,
    USN12 8760, USN13 2918, USNOS 177710, USNOS long 276346;
  - 0 on USN04, E2, USN02, BSM01, LOMP06, LOMP10, LOMP10 long and IJN01.
- **Prediction for the flip** (`local\g21_a1`) against OFF:
  - exit 0 or 1 on the eight rows with no such aim;
  - exit 3 on the ten with one;
  - JM08: Headquarter 01's platforms 4 and 5 fire **no** round at the Watchtower (0085ABA0
    refuses), so HQ shots drop by about 160 over the row;
  - wherever a gun sits above its target, ship and fort artillery against ground units
    commands a lower elevation. Guns whose windows allow the depression land their rounds on
    or before the target instead of past it. Ground-unit deaths and damage move on JM05,
    JM08 long and USNOS long (direction not predicted per row).
- **Mechanism failure:** JM08's HQ still firing at the tower, or a row with a zero census
  moving.

### 93.4 The pair, and the flip

**`local\g21_a1` (the flip, `e280693d1`) against `local\g21_a0` (OFF).** Every run is exit 0
with `lost_polls=0`. The ON runs also carried `BSP_SHELL_FATE`, whose lines pair_diff masks.

| row | exit | deaths | hit records |
| --- | --- | --- | --- |
| USN04, E2, USN02, BSM01, LOMP06, LOMP10, LOMP10 long, IJN01 | 1 | identical | - |
| JM06 | 1 | identical | - |
| USN01 | 3 | 17 -> 29 | 542 -> 1302 |
| JM08 | 3 | 5 -> 19 (all 8 Watchtowers, 3 bunkers, a radio tower) | 180 -> 385 |
| USN13 | 3 | 23 -> 22 | 456 -> 444 |
| JM05 | 3 | 12 -> 10 | 366 -> 393 |
| USN12 | 3 | 7 -> 8 | 196 -> 198 |
| USNOS | 3 | 110 -> 87 | 1766 -> 1461 |
| USNOS long | 3 | 166 -> 123 | 2779 -> 2265 |
| JM05 long | 3 | 20 -> 15 | 711 -> 871 |
| JM08 long | 3 | 46 -> 118 | 1563 -> 4209 |

- **Mechanism (JM08):** Headquarter 01 fires no round while the tower lives. Its first round
  is at t = 86.80 s, after "Watchtower, 01 03" has died at 83.40 s to "Japanese AA truck 05"
  (category 2, range 84 m). Over the row the HQ fires 46 rounds against 160 OFF. So `0085ABA0`
  refuses the depression, as 93.2 read.
- **What moved:**
  - Low guns on land (the AA trucks, category 2) now hit the ground units they target. Their
    rounds had flown over the class-Height aim point too, so the Allied watchtowers and bunkers
    on JM08 and JM08 long die.
  - Ships firing at shore targets now command the depression to the origin: some are refused
    by their windows, others land on or before the target. USNOS and USNOS long lose deaths;
    JM08 long gains them.
- **Prediction:** right on the zero-census rows and on JM08's mechanism. It missed on one row:
  JM06 (697 aims) stays exit 1.
- **Flipped ON:** `kArtilleryGroundOriginAimBound`. It is a mechanism match. The death moves
  are large (JM08 long +72, USNOS long -43, USN01 +12), and all of them follow from the aim
  point.
- **Not changed:** MAirfield and MShipyard keep the Height-raised point. Their slot `100h`
  (`006D3250`, `00844A10`) is unread.

## 94. Handoff (cc9-gunnery21, 2026-09-30 22:30 UTC, at about 78% context)

### 94.1 Landed

| item | commits | state |
| --- | --- | --- |
| 91: the Dyn world phase and hull-hull contact (00C535E0 on kind-4 records, 00C4B610 groups) | `5e576aff9`, `7b5e05db2`, `1de29dd21`, `841add1bc` | `kDynWorldContactPhaseBound`, `kHullHullContactBound` ON |
| 92: the hull contact report (00C35480 -> 009377E0 -> 008145B0); no ramming damage in the image | `bed6dc011`, `c60dcf1d1`, `d43fa61e5` | `kKamikazeDynContactBound` ON (exit 1 on all 18 rows) |
| 93: JM08's HQ flak and the Watchtower; artillery aims at a ground target's origin (0042D810) | `e280693d1`, `ccb11aaeb`, `e605e1f15` | `kArtilleryGroundOriginAimBound` ON (large death moves; 93.4) |

**Switches flipped since v, for reference W:**
- `kDynWorldContactPhaseBound`;
- `kHullHullContactBound`;
- `kKamikazeDynContactBound`;
- `kArtilleryGroundOriginAimBound`;
- plus those the lead already collects (`kLandingShipStartLandingBound`, ...).

The lead's HQ flak / watchtower question is answered by 93 (scorer: no divergence; accuracy:
the aim point). It is not open.

### 94.2 Open, in order

1. **Forts** (89.2 step 3). `MLandFort vtable[0A0h]` = `007482B0` creates a static body with
   group 1, mask `0Dh` (89.1).
   - Its shapes (`007482D0..00748986`) are not read. Ghidra's decompile of `007482B0` removes
     11 "unreachable" blocks, which calls for a flow repair (`ghidra flow 007482b0`, then the
     lead's `ghidra_flow_repair.py`) before the read:

     ```
     007486A3  007486BD  007486EE  00748708  00748732  0074874C
     00748777  00748791  007487BC  007487D6  00748969
     ```
   - The build then needs static hull-fort pairs in `world_step`. The dispatcher cell is
     `kindA * 6 + kindB`: box-convex through `native_dyn_primitive_dispatch` /
     `native_dyn_box_box`, convex-convex through `00C535E0`. Add a static body handle per fort
     (flags static; solver index 0, like the terrain tiles).
   - The report of 92 then covers hull-fort contacts too; a fort is not `IsKindOf(6)`, so
     009377E0 gives no `other`.
2. **Debris** (`00447510`, group 4, dynamic bodies of `game+30h`, GAME_DYNAMICS_LIST).
3. **The remaining 3d items:**
   - **JM08 long's 14.6 m branch change** (91.4): a differential run of `00C535E0` on the two
     records at world step 13368 against the image's bytes. The trace
     `BSP_HULL_HULL_TRACE_UNIT=349 BSP_HULL_HULL_TRACE_STEP=13300` reproduces the case.
   - **91.2's labelled substitutions:** the box broad phase, body A as the lower unit, the
     shape order, the manifold list order, hull bodies taken as awake.
   - **The ErrorOffset** of `006DF520` step 5 is not applied for non-ship targets.
   - **MAirfield and MShipyard's slot `100h`** (`006D3250`, `00844A10`), for 93's aim point.
4. **From v, not read:** LOMP10's 9 dive releases from 8 aircraft; JM08 long's HQ at 0 hp
   without a flip (cc9-ships24).
5. **Carried over:** 90.2 item 5 (85.2 item 4 and 87.2's labelled substitutions).

### 94.3 Tools (`local\` in the cc9-gunnery21 tree, prefix `g21_`)

- **Runs** (copied from g20):
  - `g21_runs.ps1 -V <prefix> -Only <rows> [-Exe]` and `g21_wait.ps1 -Logs`;
  - `g21_exp.ps1 -Commit -Out [-Flip]`;
  - `g21_vs.py <off> <on> [rows]` (it knows `g20_rv`).
- **Census and diagnostics:**
  - `g21_hh.py <prefix> [rows]`: the hull-hull census and the deepest pair;
  - `g21_terr.py <prefixes> <rows>`: the terrain census;
  - `g21_mapsym.py <addr>`: bsp_game.exe crash address to symbol, through `build\win32\bsp_game.map`.
- **Env-gated diagnostics in code:**
  - `BSP_HULL_HULL_TRACE=<file>`, with `_UNIT=<unit>` and `_STEP=<n>`
    (`src/hull_terrain_contact.cpp`);
  - `BSP_SHELL_FATE=<shooter>[|<target>]` (`src/game_hosts_gunnery.cpp`).
- **Pair logs:**
  - `g21_off` / `g21_w1` / `g21_w2`: section 91;
  - `g21_r0` / `g21_k1`: section 92;
  - `g21_a0` / `g21_a1` and `g21_hq_jm08`: section 93;
  - `g21_trace_*.txt`: the traces.

### 94.4 The queue as the lead set it (2026-09-30), superseding 94.2's order

1. **USNOS 110 -> 87 and USNOS long 166 -> 123 under `kArtilleryGroundOriginAimBound`.**
   Explain which deaths disappear and why a ground-aim change lowers deaths in a mostly naval
   row. Name the per-entity flips, and say whether they are RNG-coupled.
   - **A first look** (`pair_diff local\g21_a0_usnos.log local\g21_a1_usnos.log`, cc9-gunnery21
     tree): 27 rows only OFF, 4 only ON, 66 changed.
   - **The 27 lost** are all shore buildings: hangars, storage, Quonset barracks, houses, a
     watchtower, a radio tower, a bunker, tents, a fortress tower.
   - **The 4 gained:** Office 01 01 and 01 03, Hangar Small 04 10, Radiotower 02.
   - **So the lost deaths are the ships' shore bombardment, not naval losses.** The hypothesis
     to test: ship guns aimed at a building's origin now command a depression some mounts'
     windows refuse (as with 93.2's HQ flak), or their rounds land short of the building
     instead of in its box.
   - **To test it:** `BSP_SHELL_FATE` on one bombarding ship, and 0085ABA0's refusal count for
     ground targets. Check the shared RNG (memory: `00BD2F10` is process-wide) before calling
     single deaths attributable.
2. **Forts (`007482B0`)**, as 94.2 item 1. The flow repair comes first.
3. **The slot-`100h` routines** of MAirfield `006D3250` and MShipyard `00844A10`, for 93's aim
   point.
4. **Debris, and the remaining 3d items** (94.2 items 2 and 3).
5. **Reference W**: every flip since v, including `kArtilleryGroundOriginAimBound`.

## 95. USNOS's death drops under the ground-origin aim (packet `cc9_origin_aim_death_drops`, cc9-gunnery22)

The question (section 93.4): why do USNOS (110 -> 87) and USNOS long (166 -> 123) lose deaths
when `kArtilleryGroundOriginAimBound` flips ON? Docs only; no host defect was found.

### 95.1 Evidence

- **Pairs.** `local\g21_a0` / `g21_a1` in the cc9-gunnery21 tree (93.4), and a re-run on this
  tree's `f0ee5e80e`: `local\g22_q0` (the switch forced OFF) against `local\g22_q1` (as
  committed, ON). Both with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, the reference
  launch lines; all exit 0, `lost_polls=0`. The re-run gives 113 -> 90 deaths, with the same
  27 OFF-only and 4 ON-only victims on USNOS as the g21 pair.
- **Traces** (read-only diagnostics already in the host): `BSP_AA_TRACE_UNIT=<unit>` for the
  target changes, `BSP_SHELL_FATE=<shooter>|<target>` for the round end points. Logs
  `g22_q0_usnos` / `g22_q1_usnos` (Shimotsuke), `g22_q0_usnosl` / `g22_q1_usnosl` (ToSpawnAda),
  `g22_q0t_*` (fates against a named target), `g22_q0z_*` / `g22_q1z_*` (Zao2, Zao3),
  `g22_q0d_usnos` / `g22_q1d_usnos` (Zao3 against Debrish3 02).
- **Death sets** by victim name: `local\g22_deaths.py <off> <on>`.
- **Not the RNG stream.** The runs use separate gunnery streams. The first divergence is in
  the hits, not in a draw: `gunnery step 100 t=5.00` has hits 4 OFF against 22 ON with the
  same shots (27). The capture posts' first rounds (category 2, 140..330 m) now land on the
  ground units they target.

### 95.2 USNOS: every flipped fate (unique victims 108 -> 85; 27 OFF-only, 4 ON-only)

| victims | OFF killer | why they live ON |
| --- | --- | --- |
| 8, t = 16.70..17.25 (Hangar Small 01 02 and 01 03, Hangar Medium 03 01, Storage 01 14/18/23, two small Quonset huts) | Shimotsuke (DestroyerGen 346, platforms 1-4, category 6), blast, 2711..2763 m | OFF: its guns hold Watchtower, 01 09 at (-3504.9, 3.0, 3540.3) from 4.15 s to 55.40 s. Aimed at the class-Height point, the rounds land 150..180 m past the tower (x -3613..-3683; `g22_q0t_usnos`), among these buildings. The tower itself survives to 139.30 s. ON: Capture Post 02 (category 2, 143 m) kills the tower at 11.10 s. Shimotsuke moves to Watchtower, 01 10 at 12.35 s and kills it at 29.45 s with a direct round. Its rounds end at the tower (fate 2, x about -3537), not behind it. |
| 14, t = 102.25..109.55 (Barracks 09/10/11, Storage 01 09, Storage Raktar01 02, six stone houses, Watchtower 01 05, Radiotower 01, Medium Bunker 33) | Shimotsuke, blast, 2886..3304 m | OFF: from 92.30 s its guns take Static warhawk 04 (LandFort `Stat_warhawk`, 2977 m) and then 02. Rounds land about 330 m beyond them (x -3585..-3624, z 3114..3135). ON: warhawks 03 and 04 die at 38.55 s and 02 at 76.00 s, to Capture Post 05 (category 2, 302..333 m). Shimotsuke is never on them in that window. |
| 3 tents, t = 122.55 | Zao3 gun 965, blast, 2338..2353 m | The same target both ways: Debrish3 02 (LandFort 726, at (-3774.0, 0.5, -2435.0)), 106.65..143.55 s. OFF: the round ends at (-4074.4, 38.8, -2287.6), 335 m past the fort, on the rise where the tents stand. ON: the corresponding round ends at (-3971.4, 46.5, -2342.4), 220 m past, and kills nothing. |
| Fortress element, Small tower 01, t = 149.65 | Zao2 gun 925, blast, 2315 m | The same Debrish3 02 targeting (traced on the long row, identical OFF and ON). The OFF scatter reaches the tower. |
| US Barracks Brown 08, t = 128.55 | Capture Post 03, blast, 379 m | Downstream: not traced. |
| **ON-only:** Office 01 01 (5.05 s) and Radiotower 02 (29.60 s) | Capture Post 05, direct, 165 and 202 m | The low guns hit what they aim at. |
| **ON-only:** Hangar Small 04 10 (16.40 s), Office 01 03 (16.60 s) | Shimotsuke, 2518..2564 m | Rounds on and around Watchtower, 01 10. |

### 95.3 USNOS long (unique victims 161 -> 120; 44 OFF-only, 3 ON-only)

- **The first 150 s** match USNOS: the 22 Shimotsuke victims, a Zao3 tent and Capture Post
  03's barracks.
- **13 victims to ToSpawnAda at 357.29..369.38 s** (Watchtower 01 14, Heavy AA US Big
  Platform 09, eight storages, a barracks, an office, a house; category 4, blast, 2281..2946 m):
  - OFF: ToSpawnAda's platform 2 takes Radiotower 02 at (1455.3, 3.0, 5603.7) at 330.19 s.
    Its rounds from 355.69 s end 60..300 m around the tower, inside the airfield cluster
    (`g22_q0t_usnosl`). The tower dies at 363.63 s to Capture Post 03.
  - ON: Radiotower 02 died at 29.60 s (95.2). At 330.19 s the Ada takes House, Okinawa B 07
    instead, 3108 m away, and never fires into the cluster.
- **Zao2, 4 victims at 265.81..280.85 s** (two wooden houses, Tent03 16, Storage 01 17; blast,
  1562..1858 m): the Debrish3 02 scatter of 95.2. The target timeline is identical both ways
  (`g22_q0z_usnosl` / `g22_q1z_usnosl`). One ON-only victim, Storage 01 07 at 288.25 s, is
  the same scatter landing elsewhere.
- **Capture Post 03** (2 more, 228.51 s and 314.00 s) and **Fortress element, Small 06**
  (Storage Raktar02 01, 238.06 s): downstream, not traced.

### 95.4 Reading

- **Most of the drop is collateral.** 47 of the 71 OFF-only victims over both rows (22 + 22
  Shimotsuke, 3 Zao3) were killed by rounds aimed at the class-Height point above a ground
  target: the rounds passed over it and fell 150..335 m beyond, among clustered buildings.
- **The rest follow from earlier target deaths.** The ToSpawnAda 13 and the second Shimotsuke
  group are rounds at a target that died early in the ON run: Radiotower 02, the warhawks.
  Those died early because low category 2 guns now hit ground targets.
- **The image's aim is the origin (0042D810).** So the OFF collateral is a host artefact, and
  the ON counts are the reconstruction's current reading. No switch changes; section 93's
  flip stands.
- **Open, not a defect claim.** ON, Shimotsuke holds Static warhawk 02 (3286 m, gun
  max_range 3300) from 39.00 s until it dies at 76.00 s, and fires one round in that window
  (48.65 s, fate 5). Whether the range or arc gate refuses the origin at that distance is not
  traced.

## 96. Hull-fort contact: 007482B0's static bodies (packet `cc9_hull_fort_contact`, cc9-gunnery22)

Section 94.2 item 1, and 89.2 step 3. The flow of `007482B0` was repaired in `f0ee5e80e`. The
11 remaining "unreachable" warnings are the checked-iterator range tests calling `00BF6713`.

### 96.1 The read

**Who calls it.**
- `007482B0` is `MLandFort vtable[0A0h]` (the dword at `00CFF498`).
- `MCommandBuilding`'s slot `0A0h` is `006F2780` (the dword at `00CFB0C8`), which calls
  `007482B0` first (`006F2783`).
- There are no other callers. This is exactly query kind `1Bh`, `kUnitKindQueryLandStructure`.
- `MLandVehicle`'s slot is `0074D800`.

**The body.**
- It exists when the shape list is not empty (`uStack_5C != 0`). Otherwise `unit+754h = 0`.
- Its frame is the unit's world matrix: `00C336C0` on `unit+CCh`, after a parent refresh when
  `unit+C8h` is clear.
- The descriptor flags get bit 0 (`uStack_7C |= 1`) and the userdata is the unit.
- `00C5D580` creates it, and the handle goes to `unit+754h`.
- Only the destructor `00745AA0` destroys it (`00745B2D..00745B41`, `00C34F70`). No death path
  touches `+754h`: its only other disp32 reader, in `00824B60`, belongs to the ship class. So a
  wrecked fort keeps its body.

**The shapes.**
- `0074856B..007488A2` walks the model instance's `+4Ch` list (`unit+360h -> +160h`, through
  `00476B90`; 8-byte `{item, node}` pairs, as `0071B710` appends them). **There is no node
  filter**; the hull's walk keeps only the firstnode, root, front and back owners.
- Per pair it pushes the template (`00748150`, a `vector<48h>::push_back`) and edits the back
  element:

  | offset | value | where |
  | --- | --- | --- |
  | `+04h` friction | 1.0 (`[00D7A24C]`) | `007486DD` |
  | `+14h` hull | `&item+0Ch` | `00748720` |
  | `+08h` group | 1 | `00748764` |
  | `+0Ch` mask | `0Dh` | `007487A9` |
  | `+10h` kind | 4 (the template) | `007485F4` |
  | `+00h` restitution | 0 (the template) | - |
  | `+18h..+44h` local frame | the template's identity rotation and zero translation | `00748626..00748689` |

- The element's address goes into the body descriptor's shape list. `00747CC0(1Eh)` reserves 30
  elements first, so a longer list would reallocate under the stored addresses. The census
  counts such forts; none were seen.
- **The fort does not add the centre back.** `006FAD70` builds the Dyn hull at `item+0Ch` from
  the points minus their box centre (`006FAEA0`; the centre is kept at `item+14h`). The ship
  hull adds that centre as the shape translation (`00939458..0093947A`, into descriptor
  `+3Ch`). `007482B0` leaves `+3Ch..+44h` at zero, so a fort's shapes sit centred on the fort
  origin. **Uncertainty:** read from the listing only; no image run confirms the placement.

**Which forts have shapes.**
- ConvexObjects are rare in this installation's structure models: 17 of 297 under
  `models/structures` carry the chunk tag. They are piers (`molo`, `hatszogmolo`), boathangars,
  a bridge, the underwater net and the floating debris models (`vizen_lebego_dolgok`).
- Buildings, bunkers, hangars and the static planes have none, so they get no body.
- `models/vehicles` has none. All 239 ship models do.

### 96.2 The binding (`2e8fcbc96`, committed OFF)

- **`kHullFortContactBound`** (`include/bsp/hull_terrain_contact.hpp`). The units host gives each
  kind-1Bh unit a static `FortWorldEntry` the first step its world matrix is valid and its class
  Mesh has a ConvexObject. The frame is fixed there, and the shapes are every ConvexObject in
  record order (`MmodHullConvexBox::all_shape_points`).
- `HullTerrainContactSolver::world_step` then runs `00C44090` on each (fort, hull) pair whose
  world boxes meet: `00C535E0` on the kind-4 records (`fort_convex`).
  - With the switch ON, the hit goes through `00C3F760` into one manifold per body pair.
  - The fort is body A and static: handle 1 in `00C4B610`, solver index 0, and its own frame in
    the row build.
  - The manifold's friction is combine(1.0, hull), its restitution 0, and it joins the hull's
    group and solve.
  - With the switch OFF, the narrow phase is a census only.
- **Log lines.** `fort shapes <unit> (...)` per class, `hull fort contact census:` per pair, and
  `summary hull fort contact ...`.
- **Labelled substitutions:**
  - the world-box broad phase and the shape order (as the hull pairs);
  - the fort taken as body A;
  - no contact event for a fort pair (`009377E0` finds no `other`, section 92; the fort's own
    listener is not read);
  - the body is created at the host's first placed step, not at `vtable[0A0h]`.

### 96.3 OFF census and predictions (written before any ON run)

**OFF census** (this tree's build of `2e8fcbc96`, `local\g22_c0_<row>`, exit 0):
- **IJN01:** 9 forts with shapes (the Pearl Harbor BBRow piers, `hatszogmolo.mmod`, one shape of
  30 points each). There are 4888 near pairs and 431 hits over 397 steps. The pairs are
  Whitney (CargoShip 234, navigating) against Pier 03 (first world step 2517, t = 125.9 s, 317
  steps, deepest 12.65 m), Pier 04 (2826, 61 steps, 0.60 m) and Pier 05 (2982, 19 steps,
  7.88 m).
- **BSM01:** 9 forts and 21000 near pairs, but no hit.
- **USNOS 2, JM08 1, USN13 1, JM05 10, JM06 4, LOMP06 20, LOMP10 6** forts with shapes; none
  near a hull.
- **USN01, USN12, USN04, USN02:** no fort with a shape.

**Predictions** for the flip (`local\g22_f1`) against OFF (`local\g22_f0`):
- **IJN01: exit 3.** Everything is identical to world step 2517 (t = 125.85 s). From there
  Whitney is held off Pier 03 instead of passing through it. Its path, heading and speed move
  after that step, and it may never reach Piers 04 and 05. Nothing before 125.85 s moves.
- **Every row whose OFF census has no fort hit** (the 17 other reference rows, the long ones
  judged by their own OFF log): exit 0 or 1.
- **Mechanism failure:**
  - IJN01 moving before 125.85 s;
  - Whitney's ON contact deepening over steps as OFF (the fort not holding it);
  - a row with no fort hit moving.
