# Gun rest angles: the idle timer, the unauthored skip and the spawn seed

Packet `cc9_gun_rest_angles`, read. Worker cc9-gunnery3, 2026-09-27, base main a97df50b4.
Ghidra was read only. Nothing is bound yet: `src/game_hosts_gunnery.cpp` and its header were
leased to cc9-ships2 (`cc9_recon_level_table`) when this was written, so the binding plan and
its predictions below are recorded ahead of the pair.

Addresses: `008FBCE0`, `0085AD00`, `0085A3D0`, `0072BBD0`, `0080E290`. Data `00D7A278`,
`00D7A248`, `00D7A348`, `00CF4888`, `00D7A280`.

## 1. What the image does with a targetless gun

It does **not** command anything every tick. It waits, rests the gun once, and then holds.

| Step | Site | Rule |
| --- | --- | --- |
| Seed | `0072BBD0` (every gun-bot constructor) | `bot+54h = [00CF4888] = 999.0f`, the idle timer |
| Tick | `008FBCE0`, step 3 of the prologue of all six gun-bot ticks (`008FFA85`, `00902985`, `00903122`, `008FFFA2`, `006DF563`, and `008FC080`) | while `bot+38h` and `bot+39h` (the fire-target bytes) are both clear, `bot+54h += dt`; with either set it is held at `0` |
| Rest arm | `008FBD10`..`008FBD61` | when the side gate passes (`[gun+1ACh] == 8` or `00927F10`) and `limit < bot+54h < FLT_MAX`, pin `bot+54h = FLT_MAX` (`00D7A248`) and, if the gun answers `IsKindOf(22h)`, call `0085AD00` **once** |
| Wind-back | `008FBD7A` | when the side gate fails and the timer is pinned, `bot+54h = limit * 0.25` (`00D7A348`) |
| Limit | `[[bot+30h]+4h]` | `RobotDescriptor::no_target_time_until_rest_04`, Lua `NoTargetTimeUntilRest`, default FLT_MAX when absent (`src/robot_config.cpp`) |

The limit is **20.0 s for all six gun-bot classes** in this installation's
`scripts/datatables/robots.lua` (mtime 2025-06-01): `AAFlakBot`, `TailGunnerBot`,
`AAGunnerBot`, `ArtilleryGunnerBot`, `TorpedoBot`, `DepthChargeBot`. The author's Hungarian
comment on the first: "ha ennyi ideig nincs target, akkor visszafordul alapallasba" - if there
is no target for this long, it turns back to its base position.

Because the seed is 999.0, the first idle tick after spawn already crosses 20.0 and rests the gun.
After an engagement the gun holds its last aim for 20 s, then rests once, then holds.

### `0085AD00` `BSP_TurningGun_AimToRestAngles`, `__fastcall(gun)`, RET at `0085AD7A`

Reads the platform record `[[gun+3F0h]+538h]+94h][gun+38Ch]` and calls `0085ABA0` with
(`+94h` horizontal, `+90h` vertical) **unless `+94h` equals FLT_MAX (`00D7A278`)**, the default
the loader leaves when `RestAngles` is not authored. An unauthored gun is never sent anywhere;
it holds its last aim for good.

### `0085A3D0` `BSP_TurningGun_SetupFromDescriptor`, the spawn seed (gun vtable `A0h`)

With no saved state, the gun's current, target and previous angles (`+480h/+494h/+488h`
horizontal, `+484h/+498h/+48Ch` vertical) are seeded:

- **authored:** horizontal `RestAngles[1]` (`+94h`), vertical `RestAngles[2]` (`+90h`);
- **unauthored** (`+94h == FLT_MAX`): walk the platform's arcs (`+3Ch`, count `+40h`, stride
  `14h`) for the first with flag bits 0 **and** 1 set (traverse and fire), and take
  `clamp((min + max) * 0.5, min, max)` on each axis (`00D7A280 = 0.5`);
- then each angle is `fmod`-wrapped into the half-open turn by the `00CE3D18`/`00CE3D28`/`00CE3828`
  constants.

The listing's `0085A4xx` block before this (the `+4A4h` class flag and `007F6CA0` arc insert)
was not needed for the seed and was not re-read.

## 2. What the host does

`GameGunneryHost::Impl::run_gun_aim_and_fire` starts every gun with `want_horz = gun.rest_horz`,
`want_vert = gun.rest_vert` and, with no target, hands those to `gun_set_target_angles_0085aba0`
**every tick**. `rest_horz`/`rest_vert` default to `0.0f` when `RestAngles` is absent
(`flat_scaled(..., 0.0f)`), and the spawn path sets current and target angles to them.

So, against the image, the host (a) swings a gun back the tick it loses its target instead of
holding for 20 s, (b) swings unauthored guns to `(0, 0)` where the image holds them, and (c)
spawns unauthored guns at `(0, 0)` rather than at the first usable arc's midpoint. Where `(0, 0)`
lies outside every arc, `0085ABA0` refuses it and the host gun holds by accident.

`src/gun_bot_ticks.cpp` already reconstructs the timer as `gun_bot_idle_timer_008fbce0`; the
gunnery host does not call it.

## 3. RestAngles coverage in this installation

`scripts/datatables/autoload/vehicleclasses.lua` (mtime 2026-05-09, modified): **1540 of 3841**
platforms author no `RestAngles`. Census script: `local/cc9-gunnery3-rest_census.py` in this
worktree (a line parser over the `Platforms` blocks; not tracked). The classes on the reference
missions:

| Mission | Class (mount name in the log) | platforms | without RestAngles |
| --- | --- | --- | --- |
| USN02 | 293 Myoko (Haguro) | 24 | 11, all main turrets |
| USN02 | 289 Shiratsuyu (Yudachi) | 15 | 7, turrets, launchers, DC |
| USN02 | 276 Kagero (Minegumo) | 14 | 7, turrets, launchers, DC |
| USN02 | 25 Clemson (Alden) | 16 | 9, turrets and AA |
| USN02 | 19, 20, 21, 263 (Houston, DeRuyter, Exeter, Perth) | | 0 |
| USN04 | 294 New Orleans (Northampton-class01) | 45 | **45, including every AA mount** |
| USN04 | 288 Farragut | 13 | 9, turrets and launchers |
| USN04 | 1 Lexington, 2 Yorktown, 263, 364, 378, 380 | | DC and catapults only |
| USN01 | 297 Pensacola (SaltLakeCity) | 33 | 33 |
| USN01 | 71 Agano, 309 Mahan | 22, 14 | 10, 6 (turrets, flak) |
| USN13 | 294 New Orleans (SanFran), 295 Baltimore | 45, 46 | 45, 46 |
| USN13 | 365 Wichita, 71 Agano, 276 Kagero | | turrets |

Many plane and fixed-gun classes author none at all; they are unaffected when they never
lose a target, and are the same "hold" case otherwise.

## 4. Binding plan

One switch, OFF, in `include/bsp/game_hosts_gunnery.hpp` (name to be fixed at binding), gating:

1. a per-gun idle timer through `bsp::gun_bot_idle_timer_008fbce0`, seeded 999.0, with the
   target test taken after the host's own validity drop (the image's prologue step 2 precedes
   step 3);
2. no angle command on a targetless tick unless the timer's rest arm fires;
3. the rest arm skipping a gun whose `RestAngles` were not authored (the Lua reader must carry a
   presence flag rather than defaulting to 0);
4. the `0085A3D0` spawn seed for unauthored guns.

**Labelled substitutions.**

- **The limit:** `NoTargetTimeUntilRest` is taken as the constant 20.0, the value every gun-bot
  class authors in this installation's robots.lua. The host does not read a descriptor for it.
- **The side gate:** the host's `kPlayerGunSeatBound` test (`slot_ai_held_00927f10`) stands in for
  the gate. A player-seat gun already takes its angles from message 79h.
- **The class test:** the host has no class id per gun. `IsKindOf(22h)` is proven only for the
  22h turning class itself. 23h MRFSGun shares the 22h vtable except the destructor and
  slot `164h` (docs/GUN_AIMING.md), but what its `5Ch` test answers for 22h was not read. For
  23h, 24h and 27h (MRFS/MRT/MST) it is **unverified**. Either those class tests are read in the image before
  the switch is committed, or the switch is scoped to guns proven 22h and the rest are labelled.
- **One timer per gun row:** the host keeps one timer per gun, where the image keeps one per bot.

## 5. Predictions (recorded before any pair)

OFF values are from the latest reference logs of cc9-plane2 and cc9-ships2 on main, with
USN04 at 739 hits. The team lead's 789 is superseded. The OFF side is re-run before pairing.

| Row | OFF | Prediction ON |
| --- | --- | --- |
| USN02 9200/9000 shots / hits / deaths | 863 / 573 / 19 | shots and hits move by less than 10%; the same 19 ships die |
| USN02 first shot | 1.40 s | holds |
| USN04 4700/4500 deaths / hits | 44 / 739 | deaths 44..47; hits rise, because AA holds its aim between planes |
| USN04 first shot | 91.60 s | may move: the New Orleans AA starts at arc midpoints, not (0, 0) |
| USN01 3200/3000 deaths / hits | 7 / 150 | 7 holds; hits move by less than 10% |
| USN13 3200/3000 deaths | 27 | 27 +- 1 |
| `angle_sets + refusals` | USN04 2,684,620; USN02 2,094,106 | USN04 at least halves; USN02 falls by at least 20% (targetless ticks stop commanding) |

## 6. For docs/CONTROLLED_UNIT.md

"SetSelectedUnit's records" calls the `0080E290` release's `0085AD00` call **subsumed**, because the host
rests every targetless gun every tick. With this switch ON it is no longer subsumed: after a
release the side gate starts passing, and the wound-back timer (5.0 s) waits about 15 s more before
it rests the guns, where `0080E290` rests them at once. Idle-player reference runs never release,
so no pair moves. That doc's row should be revisited when the switch flips.
