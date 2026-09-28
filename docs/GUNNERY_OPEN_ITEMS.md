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
| 7 | `007788B0` controller ownership | exact while `ctl+284h` is empty or names this controller (decompile); not checked for a player-controlled unit |
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
