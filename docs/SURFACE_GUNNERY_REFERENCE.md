# USN02, the surface gunnery reference (packet cc9_surface_gunnery_reference)

Addresses: 006DF520 (steps 4-5, 006DF6D7-006DF7E7), 00816650, 0042D810, 00414D10, 0085E4D0,
00CE3D30, 00F87574. Ghidra was read only.

## 1. The mission and its reference row

**USN02, "New - Battle of the Java Sea"** (`scripts/missions/usn/usn_2_java.lua`). This is a
surface battle.
- **Allies, side 0:** the DeRuyter group (DeRuyter, Java, Kortenaer, Electra), which the script
  sets to SKILL_STUN; the Houston group (Houston and destroyers Alden and John1-3); and the
  Exeter group (Exeter, Perth, Encounter, Jupiter, Witte). The Houston and Exeter groups take
  `Mission.SkillLevelOwn`, which is SPVeteran at difficulty 1.
- **IJN, side 1:** the heavy cruisers Haguro and Nachi, the light cruisers Jintsu and Naka, and
  destroyers. They take `Mission.SkillLevel`, which is SPNormal at difficulty 1.
- The two fleets engage with guns (categories 2, 3 and 6) and torpedoes (category 7).

Reference run on `c1ace01c4` (main, fast-forwarded into this branch), all switches landed, `BSP_GUNNERY_RNG_STREAMS`
unset. Parameters: `--frames 9200 --press-start-frame 30 --menu-select USN02 --mission-frames 9000
--mission-frame-seconds 0.05`. Log: `local\sR_usn02.log`.

| damage | deaths | queued hits | hull hits | shots | projectiles hitting a unit / water / expired | first shot | first hit | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 63221.2 | 18 (5 Allied, 13 IJN) | 448 | 221 | 1576 | 222 / 1272 / 421 | 1.40 s | 38.80 s | none |

- **Sunk, Allied:** Java (126.95 s, by Yudachi), Kortenaer (133.35 s, Jintsu), Electra
  (179.61 s, Tokitsukaze), John3 (39.90 s, Yamakaze) and Witte (39.95 s, Kawakaze).
- **Sunk, IJN:** Haguro and Jintsu, and eleven destroyers, among them Yudachi, Samidare,
  Murasame, Harusame and Yamakaze.
- **Survive:** DeRuyter, Houston, Exeter and Perth; Nachi and Naka.
- `can_fire_refusals` is 217881 of 219457 ready checks, so the guns spend most of the run not
  cleared to fire.
- 1272 of 1576 rounds fall in the water.

## 2. The surface chain against the image

| term | image | host before this packet | effect estimate on USN02 |
| --- | --- | --- | --- |
| **aim point** (006DF520 steps 4-5) | a body-frame point on the target: target->vtable[100h] is 00816650 for ships, a draw over the hull box of ±0.3 length, ±0.3 width and 0 to 0.15 Height about the origin, refreshed every TargetPointRefreshTime (5 s); a skill row with SectionTargetChance > 0 may pick the engine room, magazine or fuel tank instead | the target origin raised by the whole class Height | **largest**: the host aims at the top face of its own hit box, so falling shells pass just over the box or clip its top. The image aims 0-15 % of Height up, near the waterline |
| aim error (006DEFF0, 006DF5A0) | per-skill envelope | bound (kGunAimErrorBound) | as the image |
| gravity arc (00955630, 006DF8BF) | yes | bound (kGunGravityArcBound) | as the image |
| turn-rate average (0085E4D0) | averages a turning target's velocity with its rotated image, AA bots only | not bound (no angular velocity at +AF8h) | none on artillery: 006DF520 does not call it |
| lead on a moving ship | step 6 pushes the point along the target velocity by distance / (v cos pitch) | bound with the pre-estimate | as the image |
| dispersion (throw) | Throw × BulletThrowMul per skill | bound | as the image |
| hull hit test | the model's convex hull shapes | the class hull box (Length × Width × Height, centred on the origin) | labelled; not changed here |
| section points | ship+0A68h..+0A94h (engine room, magazine, fuel tank), from the model | not loaded | SPVeteran (SectionTargetChance 1.0) aims at sections in the image; the host falls to the hull box |
| mount position | the platform slot frame | bound (kShipPlatformAttachmentBound) | as the image |

## 3. The binding: `kArtilleryAimPointBound`

For an ArtilleryGunnerBot gun with a ship target, the host now keeps 006DF520's bot state:
- a timer, bot+B4h, reset to 5 s (TargetPointRefreshTime in every row);
- a body point, bot+A8h, drawn through `bsp::ship_lead_point_00816650` over the target class's
  hull box with the four draws on their own stream key;
- each tick, the point carried to world by the target's pose.

**Labelled:**
- The section points are not loaded, so the section path is unavailable.
- bot+90h's offset, stepped toward bot+84h at 30 per second, is not modelled.
- A target change re-draws at once.

## 4. Predictions (written before the runs)

Pair: USN02 9000, same tree, `BSP_GUNNERY_RNG_STREAMS=1`. OFF is `local\uC`, ON is `local\uT`.
- **Artillery hits** (categories 2, 3 and 6 against ships) up 30-80 %. Projectiles hitting a
  unit rise from about 14 % of rounds to 20-30 %.
- **Shots** within ±15 %: aiming changes where rounds go, not when guns fire.
- **Ship damage** up. Deaths go from 18 to 18-24, and the sinkings by gunfire come earlier.
- **Mission end:** none in 9000 frames on either side, as in the reference.
- **Torpedoes** (category 7): unchanged in count. Which ships they sink may shift, because
  targets die earlier.
- Section 5 checks the torpedo-row selection line in the same pair: no Japanese aircraft fly
  in USN02, and no E2 aircraft has a skill index other than 1. So the line changes nothing in
  either mission, by construction.

## 5. The pair

Logs `local/uC_usn02.log` (OFF) and `local/uT_usn02.log` (ON), USN02 9000, same tree, option on.

| row | OFF | ON | prediction | held? |
| --- | --- | --- | --- | --- |
| artillery hits, categories 2 / 3 / 6 | 153 / 212 / 135 | 159 / 275 / 244 | up 30-80 % in total | yes: 500 to 678, +36 % |
| rounds hitting a unit / all rounds | 261 / 1953 (13.4 %) | 347 / 1399 (24.8 %) | 20-30 % | yes |
| shots | 1953 | 1399 | within ±15 % | **no**: -28 % |
| category 7 (torpedo) shots / hits | 354 / 23 | 354 / 13 | count unchanged | shots yes |
| deaths | 18 | 18 | 18-24 | yes |
| mission end | none | **failed at 173.31 s**, "Game Over" | none | **no** |

- **Sinking flips:** Exeter is sunk at 169.46 s (killer Yudachi), and Jintsu at 336.44 s
  (Houston). Perth and Hatsukaze now survive.
- **The mission end** follows Exeter. `usn_2_java.lua:521` fails the mission when
  `Mission.Houston.Dead or Mission.Exeter.Dead`, through `luaMissionFailed`. The death flag is
  published by the entity-dead binding.
- **Shots fall** with the ready checks: `fire_if_ready` goes from 235345 to 191219. Targets that
  die at other times change who engages whom. The cause is not traced further.
- **Aim points drawn: 1676.** That is one per artillery gun per target per 5 s.
- Torpedo hits fall from 23 to 13 with the same 354 launches. Nothing in the torpedo chain
  changed; the ships' positions and survivors did.

## 6. Decision and the secondary

- **`kArtilleryAimPointBound` lands ON.** The aim point is the image's ship-slot +100h draw,
  00816650 over the hull box. Its known gap is labelled: the section points are not loaded, so
  the section path is unavailable. That matters most for SPVeteran and Elite gunners
  (SectionTargetChance 1.0).
- **The USN02 reference row therefore changes with it.** Section 1 was taken with the switch
  absent. The landed state fails the mission at 173 s on Exeter's sinking. The run is
  deterministic apart from the documented avoidance counter, so the ON log is the new reference
  shape. A reference-parameter re-run (option off) should be taken at the next re-baseline.
- **Torpedo-row selection** (docs/IJN_SKILL_ROWS.md section 4): the units host's torpedo-attack
  hunk now selects the torpedo row (release altitude, near and far distances, aspect scale) by
  the slot's `pilot_skill_index` (0 Stun .. 5 Elite). Every E2 Kate carries index 1, so the E2
  rows are identical **by construction**: the selected row is the SPNormal row the host named
  before, value for value. USN02 has no aircraft.
