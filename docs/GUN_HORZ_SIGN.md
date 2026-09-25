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
- **Open.** Whether the hull heading is subtracted, and in which order, before `b` reaches
  `0085B7D0` was not settled. If `b` arrives as target heading minus hull heading, it is
  starboard-positive, and the image's own ship-AI bearing test is mirrored against its gun bots.
  The switch does not change this path either way.

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

Not run yet. At 16:01 on 2026-09-25 two short runs died during start-up, after
`load_game_settings`, with session 1 disconnected (`query session`); a disconnected session gives
the process no renderer or audio endpoint. The binaries are staged in `local\hzO` and `local\hzT`.
The commit lands the switch OFF until the pairs are measured.
