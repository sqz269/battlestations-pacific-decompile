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
