# The flak proximity burst `0070C370`

Addresses: `0070C370` (BSP_FlakProjectile_TickAdvance, body `0070C370`-`0070CAD3`), `0070C210`
(the detonation), `008053C0` (the world entity list, `+DE8h`), `00901C20`, `00427EB0`, `00902920`
(section 5), `0071B3E0` (section 6).

Packet `cc9_flak_proximity_burst`, 2026-09-23. Ghidra was read, not written. All names are
hypotheses. Nothing here is ABI-compatible or game-validated.

## 1. The read, from the listing

`ESI` is the round's tick element and the round is `ESI - 244h`, so every offset below is on the
round.
1. **Base tick.** `0070C387` runs the base tick `006E6490`: the flight step and the direct-strike
   sweep. The host already does this.
2. **Expiry.** `0070C3AA`-`0070C3C8`: once the flight time `+1C4h` passes FlyTime (`class+54h`),
   `0070C210(proj, 0)` expires the round. The host's life bound already does this.
3. **Arming.** `0070C3DA`-`0070C400`: the round is armed only when the flight time exceeds
   FlakFuseTime (`class+D8h`) and the scaled step is non-zero. No reader writes `+D8h`, so it is 0
   from the constructor `006E8320`.
4. **The step segment.** `0070C41C`-`0070C491`: `D = pos(+FCh) - prev snapshot (+1DCh)`, its length
   `L` (`0042B2F0`) and unit direction. `0070C4A8`-`0070C4FF`: the midpoint `M = (pos + prev) / 2`.
5. **The search, only while the lock byte `+288h` is clear** (`0070C4A4`, `0070C503`).
   - Radius. `reach = 0.5 L + 2 * BlastRange` (`class+70h`, `0070C50F`-`0070C526`), and the
     limit is `min(90000 (00CFD508), reach^2)`.
   - Candidates. It walks `008053C0()+DE8h`, the world entity list. It keeps an entity answering
     `IsKindOf(5)` (a unit) and one of `0Fh` (plane), `0Eh` (MTorpedoBoat) or `0Ch` (MLandingShip)
     (`0070C553`-`0070C58C`). **There is no side test,** so a friendly aircraft qualifies.
   - Test. `d2 = |entity pos (+FCh) - M|^2` must be below the limit and below the best so far
     (`0070C611`-`0070C63D`).
   - Lock. Each such entity is locked: `+288h = 1`, `+294h = entity` (`0070C661`, `0070C665`).
     `00901C20` is then called from the round's own position (`00427EB0`) with the class V0
     (`+50h`) and a zero shooter velocity (`00F87574`), which gives the aim point `A`. The
     remaining distance is `dot(A - prev, dir) + [+290h]`, floored at 0 (`0070C685`-`0070C6E3`).
   - Result: the nearest qualifying entity wins.
6. **Locked.** When `L < remaining`, the remaining distance falls by `L` (`0070C7AD`). Otherwise
   the round moves to `prev + dir * remaining` (`0070C72A`-`0070C791`) and `0070C210(proj, 1)`
   bursts it (`0070C79B`). `docs/EXPLOSION_RADIAL_DAMAGE.md` covers `0070C210`: the radial blast
   at `proj+FCh`, radius BlastRange, damage `U(BlastDamageMin, BlastDamageMax)`, with falloff.
7. **Passing rule.** `0070C7B6`-`0070C806` apply only when the search ran and found an entity
   inside 300 m. If the distance is still closing, the rule stores the best `d2` at `+284h`.
   Otherwise, beyond 50 m (`00CFD50C`), it bursts with 10% chance: `U(0, 100) < 10` (`00CE3D08`,
   `00CE38B8`). Since any entity inside the limit locks on that same tick, this rule cannot act
   after a lock.

**Which rounds.** Every round whose class `Type` is `"Flak"`: the flak vtable `00CFD554`. In USN04
that is class 44, fired by the category 5 flak mounts and by the category 6 dual-purpose mounts
at aircraft. Its BlastRange is 35 and its blast damage is 35, so the lock radius per step is
`0.5 * 40 + 70 = 90 m`.

**`+290h`, the distance error.** This is the AAFlakBot's DistErr, which `008FDBE0` rolls into
`bot+60h` and hands to the gun. At the SPVeteran row USN04 sets, it is 0 (`docs/AA_LETHALITY_AUDIT.md`).

## 2. The binding (`kFlakProximityBurstBound`, default true)

This is in `run_projectiles`, after the direct-strike sweep. The OFF path is the direct strike only.

**Substitutions:**
- `+290h` is 0, which is exact at the SPVeteran row.
- The entity list is this host's units. Dead ones are skipped, because the image's list drops a
  destroyed entity.
- The passing rule is not modelled, because it cannot act once a lock exists.

The census line is `summary mission gunnery flak proximity locks= bursts=`.

## 3. Predictions, written before the runs

USN04 4500 and E2 9000, RNG option on both sides, switch off against on, same tree.
- **Category 5 and 6 hits rise strongly,** by a factor of 2 to 4. Before, a flak round had to strike
  the Kate's box. Now every round whose track passes within 90 m of a plane bursts at the
  intercept, and with the SPVeteran rounds' zero error the burst lands within the 35 m blast
  radius.
- **The six Lexington Kates** (control death times 202.86 / 233.06 / 282.35 / 312.50 / 330.39 /
  327.54 s) die earlier, by 2 to 10 s. Salvos fired at 1000-1500 m that missed before now damage
  them. None releases.
- **The Yorktown-side Kates (#4.1)** die earlier as well. Whether any of them loses its release
  depends on how close its death was to it. Expect one or two fewer torpedo drops.
- **Friendly bursts.** The lock has no side test. US fighters near a Kate can take blast damage,
  so there may be US fighter hits taken from US flak.
- **Rows expected flat:** category 1 hit rates, ship damage from bombs, and torpedoes that were
  already dropped.

## 4. The pairs

The logs are `local\fC_*.log` (switch off) and `local\fT_*.log` (on), on main `3addc9235` plus
this packet, with the RNG option on both sides. This tree's control differs from `vC_9000`
because main moved: some Lexington Kates now die earlier, so each pair is read within itself.

| quantity | 4500 off | 4500 on | 9000 off | 9000 on |
| --- | --- | --- | --- | --- |
| category 6 shots / hits | 505 / 96 | 312 / 199 | 1012 / 188 | 564 / 371 |
| category 5 shots / hits | 126 / 17 | 96 / 62 | 326 / 24 | 163 / 92 |
| category 1 shots / hits | 1312 / 109 | 746 / 26 | 2973 / 187 | 1344 / 48 |
| torpedo drops | 5 | 4 | 8 | 8 |
| deaths | 19 | 19 | 35 | 37 |
| damage | 4250.5 | 4473.5 | 7550.5 | 7793.6 |

At 9000 the burst's census reads 591 locks and 570 bursts.

**Kate deaths at 9000, off then on.**
- The Lexington attackers:
  - #2.1: 204.41 to 200.81 s;
  - #2.1\|.-2: 157.20 to 151.45 s;
  - #2.1\|.-3: 201.86 to 197.51 s;
  - #2.1\|.-4: 155.25 to 151.45 s;
  - #6.1: 284.35 to 280.75 s;
  - #6.1\|.-2: 237.26 to 233.46 s;
  - #6.1\|.-3: 281.80 to 277.46 s;
  - #6.1\|.-4: 235.86 to 233.61 s.
- The Yorktown attackers #4.1: 216.91 / 214.66 / 191.41 / 162.16 s become 174.81 / 175.61 /
  148.75 / 153.65 s. All four still release.
- #8.1: 300.35 / 270.31 / 273.21 / 276.31 s become 264.56 / 263.11 / 236.36 / 241.46 s.

**Against the predictions:**
- **Held: category 5 and 6 hits,** up 2.0 to 3.8 times.
- **Held: the Lexington Kates** die 2.3 to 5.8 s earlier. Kate #6.1\|.-2, the one Lexington
  attacker that released in the control, no longer releases.
- **Held: the Yorktown Kates** die 8 to 42 s earlier, but all still release.
- **Held: torpedo drops.** They are 8 = 8 at 9000. At 4500 they fell by one, 5 to 4, inside the
  one-or-two I predicted.
- **Friendly bursts:** none happened. No trace line shows a US aircraft taking a flak hit.
- **Failed: category 1 is not flat.** Its hits fell by three quarters, because the aircraft now
  die before they reach machine-gun range. That is a consequence of the burst, not a change to
  category 1.
- **Yorktown damage** is 1678 against 395 at 9000. D3A Val #1.1\|.-2 now releases twice; in the
  control it did not.

**Decision:** `kFlakProximityBurstBound` lands ON. The read covers everything the host reaches, and
its one substituted term, DistErr, is exactly 0 at SPVeteran.

## 5. The AAGunner swinging error (`00902920`, `00902B38`-`00902EF7`)

The row is `R = [00E19998] + bot+34h * 10h`, from robots.lua AAGunnerBot (reader `008FC6D0`). Its
fields are `+0Ch` AngleDiffErrorRatio, `+10h` Dist2AngleErrRatio, `+14h` ConstAngleError and
`+18h` BulletThrowMul.
- **Period.** When the swing timer `bot+60h` runs out, it is reloaded with
  `U(3, 8) * InterpolateClamped(0, 1, [00CE6630], [00CE54A0], skill)`.
- **Magnitude.** When `R+0Ch > 0`, it is the wrapped angle between the target's `vtable[100h]`
  section point and the barrel's current direction, times `R+0Ch`. To that it adds
  `U(0, R+14h)` degrees, converted by pi / 180 (`00CE3D28`, `00CE3D20`). Against a ship target it
  is divided by settings `+750h`/`+754h`.
- **Direction and swing.** Two `00BD2F90(0, e)` draws give the new offset pair. The per-second
  swing rates `bot+6Ch`/`+70h` are (new - current) / period. `bot+64h`/`+68h` step by those rates
  every tick and are clamped against `00CE3880`.
- **SPVeteran zeroes it.** AAGunnerBot SPVeteran has AngleDiffErrorRatio 0 and ConstAngleError 0,
  so e = 0 and the offsets stay 0. SPNormal would give 5 and 4 degrees. **Landed as a read;
  nothing to bind at the level USN04 sets.**

## 6. The Points reader and the gun class's `+50h`, read

- **`+50h` is the device's mesh.** This installation's device rows live in
  `classtables/arcade/deviceclasses.lua`; the autoload file is only a wrapper that picks the
  table. The rows carry `["Mesh"] = Platform("models/devices/...mmod", ...)`. My earlier "no Model
  key" note was wrong, and dated corrections are appended to `docs/GUN_AIM_TERMS.md` and
  `docs/AA_LETHALITY_AUDIT.md`.
- **The Points items.** In the `.mmod`, each muzzle is an `Aux` item. Its `Identifier` chunk holds
  a length-prefixed name and a U32 index, which `0071B3E0` reads into `item+8` and `item+24h`.
  A `Points` chunk of 12-byte vectors follows it.
- **The count rule, all of `007325A0`.**
  - First it asks for `("fire", 0)`. If that item exists, its whole point list is copied
    (`00732689`-`007326BF`).
  - Otherwise, from `007326DA`, it walks index 1, 2, 3 and on while `("fire", index)` exists,
    appending the **first point** of each item (`007326F0`-`00732788`).
  - So the muzzle count is the number of consecutive `"fire"` items from index 1.
- **The Lexington's three mounts,** scanned offline with `local\mmod_ids.py`:

| device | mesh | `"fire"` items | image count | host `barrel_num` |
| --- | --- | --- | --- | --- |
| 12, Atlanta 5'' 2X DP (cat 6) | `models/devices/us/atlanta_turret.mmod` | indices 1, 2 (3 points each) | 2 | 2 (from the Bullet records) |
| 20, Flak Gun US (cat 5) | `models/devices/japan/flak_gun_us.mmod` | index 1 | 1 | 1 |
| 42, AA Bofors 40mm (cat 1) | `models/devices/us/bofors_aa.mmod` | indices 1, 2 | 2 | **1** |

- **Consequence.** The host matches the image for the two flak mounts. It fires the twin Bofors at
  half the image's rate, which makes category 1 **less** lethal than the image's.
- **Cost of binding.** It needs the class table's `Mesh` path, the host's VFS open, and a scan for
  `Identifier`/`Points` chunk pairs in the model's structured stream. That is a small reader, but it
  is new file access, so it is not coded in this packet. It is the next binding.
