# The gun horizontal angle's sign (packet `cc9_gun_horz_sign`)

The host's gun horizontal angle has had the opposite sign to the image's since the gunnery host
was written. The authored firing windows are loaded unnegated, so every window has been mirrored
in the host. This packet binds the image's sign under `kGunHorzImageSignBound`
(`src/game_hosts_gunnery.cpp`). The finding was first recorded in `docs/MUZZLE_OFFSETS.md`
section 2. Ghidra was read-only.

## 1. The image's sign

| address | rule |
| --- | --- |
| `005213A7`..`005213AC` | `00521370` loads `[row]` (x), then `[row+8]` (z), then calls `_CIatan2`: yaw = `atan2(x, z)` |
| `008FDAF0` | the gun bots' direction-to-angles: horz = `-0.0f - yaw` (`gun_bot_angles_from_local_008fdaf0`, `src/bot_fire_target.cpp`) |
| `0085AACC`, `0085AB17` | `0085A9A0`, the AA acceptance window test: `00521370`, wrap past pi, then `-0.0f - yaw` (`00D7A208`) before `007F60A0` at `0085AB32` |
| `00859550` | the node pose: root local = RotY(-horz), whose row 2 is `(-sin horz, 0, cos horz)` |

So a positive image angle turns towards model `-x`, which is port (`docs/SHIP_PLATFORM_ATTACHMENT.md`:
model `+x` is starboard). The windows (`MinHorzAngle`/`MaxHorzAngle`) and `RestAngles` are read
from `vehicleclasses.lua` without negation, in the host as in the image.

## 2. The host sites

`kGunHorzSign` is `-1` when the switch is on and `+1` when it is off. It multiplies the starboard
component at every site where a horizontal gun angle meets a direction:

| site | image counterpart |
| --- | --- |
| the bot aim `want_horz` | `008FDAF0` |
| the AA acceptance window `horz` | `0085A9A0` at `0085AB17` |
| the shot direction `forward cos h + right sin h` | the node's row 2 (`0073022A`) under `00859550` |
| the muzzle pose, `turning_gun_apply_angles_00859550(-sign * horz)` | `00859550` directly; OFF keeps the bridge `docs/MUZZLE_OFFSETS.md` used |
| the torpedo gate's node axis | `[gun+3CCh]+110h` under `00859550` |

The torpedo heading snap and the settle and window tests take the angle as they get it, so they
follow automatically.

**The ship AI's `device_can_bear`, `0085B7D0`, re-read (partial).** It is outside the switch: the
host reconstructs it verbatim (`ship_ai_gun_can_bear_0085b7d0`, `src/ship_ai_bearing_rating.cpp`),
fed by the ship AI's own words, and nothing in it reads the gun host's angles.
- Every arm tests the authored windows against the bearing word `b` itself. Function 7 hands `b`
  to `0085AB50`'s snap directly (`0085B80B`..`0085B844`). The default arm uses `b` as the
  horizontal angle (`[ESP+28h]`).
- The artillery arm (functions 2, 3, 4, 6 and 9) builds the direction `(sin b, 0, cos b)` from
  `a = pi/2 - b` and solves it with `00955630`. That solver returns `-0.0 - atan2(x, z)`
  (`009557F8`), and `0085B8F1`..`0085B8FF` negate it again. The two negations cancel, so this arm
  also tests `+b`.
- So `0085B7D0` agrees with the gun bots exactly when `b` is port-positive. Its two producers
  here are compass headings: `009E634C` stores `heading_118` from the shooter-to-target delta,
  and `009E8153` stores the approach word `nested+11DCh`, from which `009E5DB4` subtracts the
  slot's own angle. The heading helper `00414EB0` is `pi/2 - atan2(z, x)`, which equals
  `atan2(x, z)`, positive towards `+x` (starboard).
- **Settled for the `009E634C` path: no hull heading is subtracted anywhere.** `009E6240`
  normalises the delta, calls `007B4E90 BSP_Vector3_CompassHeading` at `009E6334` (`pi/2 -
  atan2(z, x)` wrapped into `[0, 2pi)`), stores the result at `record+118h` (`009E6345`) and
  into query word 5 (`+14h`, `009E634C`). `0095EB40` only wraps it (`00605070` at `0095EB9E`).
  `0085B7D0` then reads the word as `[ESP+2Ch]` (`0085B828` for function 7, `0085B85D` for the
  default arm, `0085B881` for artillery) with no subtraction. So on this path the image tests
  **hull-relative** windows against a **world compass heading**, starboard-positive, in
  `[0, 2pi)`. That is neither the gun bots' sign nor a hull-relative angle; it is right only for
  a ship heading due north, and even then it is mirrored against the gun bots. This is the
  image's behaviour, and the host reproduces it verbatim.
- **Settled for the `009E8153` path: world minus world, with no hull heading either.**
  - Word 5 is `nested+11DCh`. The arc-centre step `009E46F0` writes it as the compass heading of
    the ship-to-point delta: the point comes first in both subtractions (`009E481C`,
    `009E4826`), then `atan2(dz, dx)` at `009E4865`, `FSUBR` of pi/2 at `009E4872`, and the
    `[0, 2pi)` wrap at `009E4882`.
  - Each ring slot's angle is a fixed world spoke. `009E5530` stores
    `angle_08 = wrap((i / N) * 2pi)` at `009E56F9` and builds the slot's world direction from it
    as `(cos(pi/2 - a), 0, sin(pi/2 - a)) = (sin a, 0, cos a)` (`009E571E`..`009E574A`). That is
    the compass convention of `007B4E90`: starboard-positive, measured from world `+z`. No hull
    heading enters the construction.
  - `009E5DA0` replaces word 5 in place with `wrap(word5 - angle_08)` (`009E5DA3`..`009E5DBB`,
    `00438B10`), then rates the block through `0095EB40` (`009E5DBF`).
  - So on this route `0085B7D0` receives the angle from the ring slot's world spoke to the
    arc-centre direction, clockwise-positive. It is relative to the spoke, not to the hull.
- **Both routes, then.** Neither passes a hull-relative angle to `0085B7D0`. The `009E634C` route
  passes a world heading. The `009E8153` route passes a spoke-relative angle. Both are
  starboard/clockwise-positive, which is the opposite sense to the gun bots' port-positive angle.
  The image's ship-AI bearing test is therefore not the gun bots' test on either route. The host
  reproduces both routes verbatim, and `kGunHorzImageSignBound` does not touch them.
- Neither path reads the gun host's angles. `kGunHorzImageSignBound` does not change them, and no
  change is proposed: fixing them would depart from the image.

## 3. Which mounts change their trainable side

`local/horz_sides.py` joins the authored windows in this installation's `vehicleclasses.lua` with
the slot positions the host logs (`gunnery: mount ... local=(x ...)`). A mount is one-sided when
all its horizontal windows lie on one side of zero, and off-centre when |x| > 1 m.

| mission | one-sided off-centre mounts | face their own side, image sign | face their own side, host sign |
| --- | --- | --- | --- |
| USN02 | 94 | 94 | 0 |
| USN04 | 108 | 104 | 4 |

- **Every port and starboard sponson changes side.** Examples: Houston 20..40 (`[10..170]` at
  x < 0, `[-170..-10]` at x > 0); Exeter 10..21; Haguro 6..41; Jintsu 3, 4 and 20..35; Perth
  12..31; Alden 2, 3 and 10..13. The side torpedo mounts on Alden, Exeter, Haguro, Jintsu and
  Perth change with them.
- **The island group, Lexington 14..17 (x = +/-19 m, category 5).** Under the image's sign,
  14 (x = +19.0, `[-170..-10]`) and 16 (x = -19.1, `[10..170]`) face their own side. 15
  (x = +18.8, `[10..170]`) and 17 (x = -18.8, `[-170..-10]`) are authored to fire across the
  deck. These two are the four USN04 mounts that fit the host's sign, and they fit no rule in
  either sign; they are authored that way.
- Mounts whose windows straddle zero (the main turrets, `[-145..0] [0..145]`) keep their reach.
  Their rest angle sign flips, which only matters on a zero `RestAngles` horizontal.

## 4. Predictions, written before the pairs

Pairs: USN02 9200/9000 and USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`,
OFF `local\hzO` against ON `local\hzT`, one tree differing only in `kGunHorzImageSignBound`.

- **AA line of fire (USN04).** `aa line of fire blocked` falls by more than half from its OFF
  value (209 on the muzzle ON run): a sponson no longer shoots across its own superstructure.
- **AA effect (USN04).** Category 1 and 5 hits rise; plane deaths rise by 0 to 5. The Lexington
  row is judged only as a knife-edge.
- **Surface rows (USN02).** Side secondary rows (category 6 on Exeter, Haguro and Perth; category 2
  on Alden and Jintsu) change targets and totals. Main-turret rows move only by cascade. Total
  shots stay within 20 percent. Torpedo launches stay within 15 percent, because each ship still
  has one tube set facing each side.
- **Headline.** USN02 deaths within 4 of the OFF count; Houston and Exeter survive on both sides.
  USN04 deaths within 4.
- The OFF side must match the muzzle-offsets ON run (`local/mzT_*.log`) in every gameplay row:
  the OFF binary differs from it only by the new `horz side` log line.

## 5. The pairs, and the decision

Both pairs were rebuilt from the merged tree (main at `51e22e56d`, which includes the HUD worker's
player-seat code with its switch OFF), in `local\hzO` and `local\hzT`, and run one pair at a time.
USN04: `local/hzO_usn04.log` against `local/hzT_usn04.log`. USN02: `local/hzO_usn02.log`
against `local/hzT_usn02.log`. All four runs presented every frame and exited 0.

| row | USN04 OFF | USN04 ON | USN02 OFF | USN02 ON |
| --- | --- | --- | --- | --- |
| shots | 3672 | 3654 | 1412 | 1155 |
| queued hits | 448 | 445 | 952 | 647 |
| deaths | 30 | 30 | 18 | 16 |
| AA line of fire blocked / refusals | 258 / 786 | 293 / 985 | - | - |
| AA hits (cat 1 / cat 5) | 84 / 66 | 82 / 62 | - | - |
| torpedo launches | - | - | 335 | 305 |
| gun rows that differ | - | 289 of 726 | - | 301 of 464 |

**The mirror, seen directly.** Lexington's four island-group mounts swap roles exactly.

| platform (x, window) | OFF shots / hits | ON shots / hits |
| --- | --- | --- |
| 14 (+19.0 m, `[-170..-10]`) | 8 / 7 | 4 / 4 |
| 15 (+18.8 m, `[10..170]`) | 3 / 3 | 9 / 8 |
| 16 (-19.1 m, `[10..170]`) | 3 / 3 | 8 / 6 |
| 17 (-18.8 m, `[-170..-10]`) | 8 / 5 | 4 / 4 |

The attack comes from port. OFF, the busy pair is 14 and 17, the two windows the host was
turning to port. ON, it is 15 and 16, the two the image faces to port.

Verdict per prediction:
- **USN04 AA line of fire falls by more than half: failed, and the premise was wrong.**
  `line_of_fire_blocked_0072cdd0` skips the firing ship (`u == owner`). A sponson was never
  blocked by its own hull; the count tracks which other ships sit between a gun and the targets
  it now admits.
- **USN04 AA hits rise, plane deaths +0..5: failed, flat instead** (cat 1 84 to 82, cat 5 66 to
  62, deaths 30 on both sides). Every mirrored mount has a twin on the other beam, so correcting
  the sign swaps which twin serves which side; coverage is unchanged. The swap is the effect.
- **USN02 total shots within 20 percent: held** (-18 percent).
- **USN02 torpedo launches within 15 percent: held** (-9 percent).
- **USN02 main turrets move only by cascade: held in kind, large in size.** Main-turret shots fall
  from 293 to 193. Their windows cover both sides, so the change comes through what the side
  mounts and torpedoes now do first: Witte, Java and Encounter survive, and Haguro and Jintsu die
  early to Harusame's torpedoes (131.35 s and 128.45 s). Both are same-side kills (IJN on IJN), and the torpedo friendly gate did not hold either launch. 17 of 20 death rows change.
- **Deaths within 4, Houston and Exeter survive: held** on both missions.

A log fix went in with the verdict. The `horz side` line started at arc index 1, but `007F5A10`
inserts the windows sorted by angle, so a window beginning at -180 sits at index 0. The line
therefore misreported aft turrets such as Houston's platform 3 (`[45..180]` plus `[-180..-45]`)
as one-sided. The side table in section 3 comes from the Lua file through
`local/horz_sides.py` and was not affected.

**Decision: ON.** The windows fit the image's sign on 198 of 202 one-sided mounts in the two
missions, and on 4 of 202 under the host's old sign. USN04 barely moves. USN02 churns as it did for
the muzzle offsets.