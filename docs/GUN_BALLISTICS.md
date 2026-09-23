# Gun ballistics: the gravity arc, the round's gravity, and the aim-error envelope

Addresses: `00955630` (BSP_Gun_SolveGravityArc), `006DF520` (BSP_GunBot_MuzzleSolutionAimTick),
`006DEFF0` (BSP_GunBot_RerollAimErrorEnvelope), `006DF5A0`, `006DFB0B`, `00902920`, `009030C0`,
`00901C20`, `0072C6A0`, `00729BC0`, `006E65C0`, `006E7670`.

Packet `cc9_gun_ballistics`, 2026-09-23. The host counted the arc as its second most-called
unimplemented native (472,875 calls in one USN04 run). That run's 4495 rounds at the cycling goaway
Vals (`docs/AA_TARGETING.md` section 4) never hit.

## 1. The arc `00955630`, read

It is fully reconstructed already, in `src/gun_gravity_arc.cpp` and `docs/GUN_GRAVITY_ARC.md`
(packet `cc7_gun_gravity_arc`, fixture-tested). This packet re-states the rule and corrects where
the host uses it.

- **ABI.** `bool __fastcall(mount ECX, const float3* aimPoint EDX; const float3* muzzlePos, float
  muzzleSpeed, float* outPitch, float* outYaw)`, `RET 10h`.
- **Inputs.** The aim point already carries the lead, from `006DF520` step 6. The muzzle speed is
  the round's V0, `[[gun+3F8h]+34h]+50h`. Gravity is the float `9.81` at `00CF9058`, an immediate
  value and not a field. There is no drag, air density or time-of-flight term.
- **Method: closed form, no iteration.**
  - `k = R^2 * g / (v * 2v)` is rounded to float.
  - `D = R^2 - 4k(k + h)`, where h is the height of the aim point over the muzzle.
  - When `D >= 0`, `tan(pitch) = (R - sqrt(D)) / 2k`. That is the **minus** root, the low flat
    trajectory, never the lob.
  - When `D < 0` it returns **false** and writes `pitch = pi/4` (`00CEB5A8`) toward the target's
    bearing.
- **Output.** Both angles are always written: the pitch, and the yaw in the world frame or, with
  a mount, in the mount's local frame after the round trip. The routine answers in AL.

**Who uses it (new).** Only `006DF520`, the ArtilleryGunnerBot tick, and the non-bot callers
`00547480`, `0085B7D0` and `00959C20`. `0072C6A0` gives a gun its bots by weapon sub-type
(`docs/GUN_BOT_TICKS.md` section 3):

| sub-type | bot | aim law |
| --- | --- | --- |
| 1 AAMACHINEGUN (ship mount) | AAGunnerBot `00902920` | `00901C20` lead point, **no gravity term** (`docs/AA_VERTICAL_WINDOW.md` section 2) |
| 5 FLAK, and 6 through its second ammunition against a plane | AAFlakBot `009030C0` | `00901C20` lead point, no arc |
| 2, 3, 4, 6 (first ammunition), 9 | ArtilleryGunnerBot `006DF520` | pre-estimate `asin(gR/v^2)/2` clamped to `pi/4`, then the arc, then the aim-error envelope |

**The host before this packet** ran `006DF520`'s pre-estimate and the arc for every gun except
torpedoes, so every AA machine gun and flak mount was superelevated. It also flew every round
with gravity (`class_disables_gravity = false`, hard-coded).

## 2. The round's gravity (`NoGravity`, `classDesc+20h`)

`006E65C0` and `006E7670` apply `v.y -= g*dt` and `y -= g*dt^2/2` only while the class's
`NoGravity` byte at `+20h` is clear (`docs/PROJECTILE_IMPACT.md`). This installation's active
bullet table is `scripts/datatables/classtables/realistic/bulletclasses.lua` (dated 2025-06-02,
chosen through `autoload/bulletclasses.lua` by `GameMode`). The host's load now prints each class
(`gunnery: bullet class` lines):

| classes | Type | V0 | range | NoGravity | used by |
| --- | --- | --- | --- | --- | --- |
| 40, 41, 42, 89, 107 | Bullet | 800 | 800-1600 | **1** | AAMACHINEGUN (category 1) |
| 84, 86, 88, 91 | Bullet | 800 | 800 | **1** | PLANEGUN (category 0) |
| 44 | Flak | 800 | 2000 | **1** | FLAK (category 5) |
| 15, 31 | Artillery | 300 | 1500 | 0 | LIGHTARTILLERYFLAK (category 6, first ammunition) |
| 9, 13 | Artillery | 300 | 1900-2300 | 0 | MEDIUMARTILLERY (category 3) |

So in the image **the AA rounds fly straight** to a lead point aimed with no superelevation. The
host had them falling from a superelevated muzzle.

## 3. The aim-error envelope `006DEFF0`

This is also reconstructed already, in `src/gun_dispersion.cpp` and `docs/GUN_DISPERSION.md`
section 7. It belongs to the ArtilleryGunnerBot alone.

- **Law.** Three draws on stream 1, in order: `t = U(0,1)`, `r = U(0, MaxAngleError) * t^Power`
  (0 when t is 0), and `phi = U(0, 2pi)`. The pair is `(horz, vert) = (r sin phi, r cos phi)` in
  radians.
- **Cadence.** Each reroll installs a period of `U(3, 8)` s (`006DF5C6`). The attach
  `006DF1F0` leaves the countdown at 0, so the first tick rerolls.
- **Interpolation.** From the old pair to the new one as the countdown falls (`006DF623`,
  `006DF651`).
- **Application.** Added to the arc's angles with a wrap (`006DFB0B`), before `0085ABA0`.
- **Rows.** `[00E19990] + 1Ch * bot+34h + 0Ch/10h`, where `bot+34h` is the owner's skill
  (`BSP_GunBot_Attach 008FBC80`, `unit->vtable[12Ch]`). This installation's `robots.lua`
  (2025-06-01), `Robots["ArtilleryGunnerBot"]`:

| level | row | MaxAngleError | Power |
| --- | --- | --- | --- |
| 0 | Stun | 10 deg | 1 |
| 1 | SPNormal | 5 deg | 1 |
| 2 | SPVeteran | **0 deg** | 2 |
| 3 | MPNormal | 4 deg | 1.5 |
| 4 | MPVeteran | 2 deg | 1.8 |
| 5 | Elite | 0 deg | 2 |

The index order is `00901610`'s, the same as `luamw_init.lua`'s `SKILL_*` numbers
(`docs/GAME_DIFFICULTY.md`), not the order of the entries in the file.

**US ships carry SPVeteran, so their envelope is exactly zero.**

## 4. The binding

There are three switches in `src/game_hosts_gunnery.cpp`, each measured as its own pair with
`BSP_GUNNERY_RNG_STREAMS=1`:
- **`kGunGravityArcBound`.** Categories 1 and 5 aim at the lead point with no pitch term. The
  lead is the target's velocity times distance over V0, a labelled substitution for `00901C20`'s
  closed-form intercept. Every other ballistic gun keeps the pre-estimate and the arc, which is
  now counted as implemented, on the null-mount path with no gun node frame.
- **`kBulletNoGravityBound`.** A shot's `class_disables_gravity` comes from its class's
  `NoGravity`.
- **`kGunAimErrorBound`.** Artillery-bot guns (2, 3, 4, 6, 9) roll the envelope on their owner's
  `units.skill_level()` row, drawing from the per-gun `aim_error` stream key.

**Category 6, labelled.** A sub-type 6 gun meets a plane with its second ammunition (variant 1,
`+74h+7Ch`, a Flak round) through the AAFlakBot. The host loads only the first entry, an
Artillery round with V0 300 and gravity on, so a host category-6 shot is an artillery-bot shot
at any target. The AA law reaches category 6 only once the second ammunition is loaded.

Item 3: `GameBulletClassRow::type` is now filled from the class's `Type` string. **Nothing reads
it yet.** The ship-AI firepower rating keys on `bullet_sub_type`, and the ordnance masks use the
same string through `ordnance_kinds_for_bullet_type` at build time.

## 5. Predictions, written before each treatment build

The controls are all three switches off, with `BSP_GUNNERY_RNG_STREAMS=1`. `local\g0_4500.log`
is USN04 at 4700/4500. `local\g0_9000.log` is USN04 at 9200/9000 with `BSP_AA_TRACE_TARGET="D3A
Val"`. Per-category sums of the `gunrow` lines:

| category | guns | shots 4500 | hits 4500 | shots 9000 | hits 9000 |
| --- | --- | --- | --- | --- | --- |
| 1 AAMACHINEGUN | 259 | 3693 | 55 | 17756 | 76 |
| 5 FLAK | 24 | 128 | 8 | 284 | 10 |
| 6 LIGHTARTILLERYFLAK | 83 | 1172 | 11 | 2886 | 21 |
| 10 BOMBPLATFORM | 91 | 0 | 30 | 0 | 40 |

In the 9000 control the cycling goaway Vals (#3.1 and #7.1 families) draw 42 to 2108 rounds each
at minimum ranges of 383-758 m, with verticals up to 87-89 degrees.

**T1, arc only (`kGunGravityArcBound`).**
- Categories 1 and 5 lose their superelevation, and their rounds still fall under gravity. So
  **their hits go down**. At 1000 m and V0 800 the flight time is 1.25 s and the drop is 7.7 m,
  larger than a plane.
- The rows that move first are the category 1 and 5 gun rows (shots, refusals, hits).
- Artillery-bot guns (categories 3 and 6) are unchanged at first order, because their pre-estimate
  and arc are untouched. They move only through changed hits, since an aircraft that now lives or
  dies changes every later engagement.

**T2, T1 plus `kBulletNoGravityBound`.**
- Every category 0, 1 and 5 round (Bullet and Flak classes, all `NoGravity = 1`) now flies
  straight to a lead point aimed without superelevation, which is the image's pairing.
- **Category 1 and 5 hits rise above T1 and above the control.**
- Category 6, 3, torpedo and bomb rounds keep gravity.
- On the goaway Vals: some hits where the control had none, because the rounds reach the lead
  point.

**T3, T2 plus `kGunAimErrorBound`.**
- Only artillery-bot guns (2, 3, 4, 6, 9) draw the envelope, and USN04's US ships carry SPVeteran,
  whose `MaxAngleError` is 0.
- **So no angle moves.** The only difference is the rerolls counted. With the option on, the new
  `aim_error` key's draws cannot shift any other stream.
- Prediction: 0 differing gun rows, and a log identical except the `aim_error_rerolls` count.

## 6. The pairs

All runs have `BSP_GUNNERY_RNG_STREAMS=1` on both sides. 4500 means `--frames 4700
--press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05`.
9000 means `--frames 9200 ... --mission-frames 9000`, with `BSP_AA_TRACE_TARGET="D3A Val"`.
Logs are in `local\`. Per-category figures are sums of the `gunrow` lines; hits and damage come
from the new per-gun `hits`/`dealt` columns.

### 6.1 First attempt: the predictions failed, and the failure found two host defects

T1 (arc only) and T2 (arc plus `NoGravity`) as first built, against the control:

| pair | category 1 shots | category 1 hits | deaths |
| --- | --- | --- | --- |
| control `g0_4500` | 3693 | 55 | 10 |
| T1 `g1_4500` | 3689 | **75** (predicted: down) | 10 |
| T2 `g2_4500`, against T1 | 3703 | **52** (predicted: up) | 9 |
| control `g0_9000` | 17756 | 76 | 16 |
| T1 `g1_9000` | 10803 | 116 | 17 |

Both directions were wrong. Removing superelevation raised hits, and straight flight lowered
them. Rounds that fall were hitting more than rounds that fly true, which means the host aimed
**above** its targets. Two inputs of `00901C20`, the AA bots' lead routine, were wrong in the host:
- **The aim point.** `00901C55..00901C74` reads the target's pose position `+FCh..+104h`. The host
  aimed at `unit_aim_point`, the position raised by the class `Height`, which is `00864D90`'s
  visibility-test point. A straight round aimed there passes `Height/2` over the hit box.
- **The lead.** The host's `unit_velocity` reads the row's `forward_speed`, which is zero for a
  plane: the same reason `release_ordnance_drop` already reads `0092D730`. So every plane was
  led by nothing.

Both are now part of the AA branch behind `kGunGravityArcBound`: aim from the target's pose
position, and lead a plane with its body axis times `0092D730`'s speed.

### 6.2 The arc pair, corrected: control against T1'' (`g1c`)

| | control 4500 | T1'' 4500 | control 9000 | T1'' 9000 |
| --- | --- | --- | --- | --- |
| category 1 shots / hits | 3693 / 55 | 2391 / **201** | 17756 / 76 | 6236 / **272** |
| category 1 hit rate | 1.5% | 8.4% | 0.43% | 4.4% |
| category 5 shots / hits | 128 / 8 | 121 / 12 | 284 / 10 | 304 / 14 |
| category 6 shots / hits | 1172 / 11 | 780 / 3 | 2886 / 21 | 1516 / 10 |
| `queued_hits` | 104 | 246 | 147 | 335 |
| deaths | 10 | **22** | 16 | **36** |

- **Gun rows that move.** At 4500, 381 of 726: 208 in category 1 and 24 in category 5, which are
  the aim change itself. The other 149 are downstream: category 6 (83), category 0 plane guns
  (44) and category 10 bomb platforms (22) are aboard or aimed at aircraft whose lives changed.
  No category 3, 7 or 8 row moved.
- **Deaths at 4500.** Thirteen aircraft now die: Val #1.1|.-2, #3.1|.-2, #3.1|.-3, #3.1|.-4,
  Kates #2.1|.-2, #4.1, #6.1|.-2 to #6.1|.-4, and #8.1 to #8.1|.-4. They fall to Yorktown-class01,
  Fletcher-class02, Northampton-class05 and -03, and Lexington-class01. One death is lost:
  movieval|.-2.
- **The goaway Vals, 9000.** The control's cycling Vals draw thousands of rounds and die of none
  (two lose 24 and 205 hp). With T1'', **seven of the eight die**: #3.1 at 241.06 s,
  #3.1|.-2 163.21, #3.1|.-3 152.10, #3.1|.-4 154.85, #7.1 365.53, #7.1|.-2 363.43, #7.1|.-3
  274.61. Their killers are Yorktown-class01, Northampton-class05, -03 and Fletcher-class08. Only
  #7.1|.-4 lives, with 2773 rounds fired at it.
- **The dual-purpose guns' drop** (category 6, 11 -> 3 hits) is downstream: their aircraft
  targets now die to AA first. Their own aim law, artillery with the arc, is unchanged.

### 6.3 The `NoGravity` pair: T1'' against T2'' (`g2c`)

| | T1'' 4500 | T2'' 4500 | T1'' 9000 | T2'' 9000 |
| --- | --- | --- | --- | --- |
| category 1 shots / hits | 2391 / 201 | 1493 / 194 | 6236 / 272 | 5431 / 249 |
| category 1 hit rate | 8.4% | **13.0%** | 4.4% | **4.6%** |
| category 1 damage dealt | 5039.1 | 5485.8 | 6651.3 | 6779.9 |
| category 5 shots / hits | 121 / 12 | 85 / 10 | 304 / 14 | 257 / 12 |
| `no_gravity_shots` | 0 | 1578 | 0 | 5688 |
| deaths | 22 | **26** | 36 | 35 |

- Straight flight raises the hit rate and the damage dealt per round. Fewer rounds are needed,
  because targets die sooner: the goaway Val #7.1 at 283.05 s instead of 365.53 s.
- Category 6, 3, torpedo and bomb rounds keep gravity, and their rows move only downstream.
- The first-attempt reversal (6.1) is explained: straight rounds at a raised aim point pass over
  the target.

### 6.4 The aim-error pair: T2'' against T3 (`g3b`)

The first T3 build indexed the rows in the file's entry order (SPNormal first), so US ships at
index 2 drew MPNormal's 4 degrees. It moved 204 gun rows (`local\g3_4500.log`). That was a table
error in this packet, not a finding about the image. With the rows in `00901610`'s order (Stun 0,
SPNormal 1, **SPVeteran 2**), the prediction holds exactly:

| | T2'' `g2c_4500` | T3 `g3b_4500` |
| --- | --- | --- |
| gun rows that differ | | **0 of 726** |
| simulation lines that differ | | 2 (method-call tallies) |
| `aim_error_rerolls` | 0 | 718 |

The envelope runs on every artillery-bot gun that holds a target, and draws its three values
and period on its own stream key. SPVeteran's `MaxAngleError` of 0 makes every pair exactly zero.
A mission with a non-veteran artillery owner is where it will show.

## 7. Decisions

| switch | decision | why |
| --- | --- | --- |
| `kGunGravityArcBound` | **landed** | The arc belongs to the artillery bot alone; the AA bots aim at `00901C20`'s lead from the target's pose position and velocity. The corrected pair moved category 1 and 5 first and every other row downstream, with the hits up as predicted once the two defects the first attempt exposed were fixed. |
| `kBulletNoGravityBound` | **landed** | `NoGravity` is the class field `006E65C0`/`006E7670` test; the pair raised the AA hit rate from 8.4% to 13.0% (4500) and from 4.4% to 4.6% (9000). |
| `kGunAimErrorBound` | **landed** | Null for US ships by the table (SPVeteran is 0 degrees), measured null (0 rows). |

**What is still a substitution:**
- **The lead.** `00901C20`'s closed-form intercept is replaced by the target's velocity times
  distance over V0.
- **The muzzle.** It is the hull origin raised by the class `Height`, with no gun node frame, so
  the arc takes the null-mount path.
- **Category 6 against planes.** It fires its first, artillery, ammunition with the arc,
  because the second ammunition entry is not loaded.
- **The artillery bot's aim point.** It is still the Height-raised point, where the image aims at
  the target origin plus its `target->vtable[100h]` section offset (006DF520 step 4). Only AA
  bots were corrected here.

## 8. Open

- **Load kind 6's second ammunition** (`+74h+7Ch`). The dual-purpose guns then take the AAFlakBot
  law against planes: 83 guns in USN04.
- **The artillery bot's aim point and section offset** (006DF520 step 4, `descriptor + 18h..24h`).
- **`00901C20`'s closed form**, to replace the lead substitution.
- **The AA gunner's swinging-error pair** (`00902B38`-`00902EF7`) and the negative-vertical
  halving (`00902F62`), still unmodelled (`docs/AA_VERTICAL_WINDOW.md` sections 5.3 and 6).
