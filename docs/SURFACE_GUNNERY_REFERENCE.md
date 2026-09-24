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

## 7. The landed-state reference (2026-09-24)

This is re-taken on `cf2bab541`: main with `330b81cdc` (`kPlayerRoleBookkeepingBound`) merged and
`kArtilleryAimPointBound` ON, option off. Log: `local/sR2_usn02.log`.
- Damage 69856.9, 18 deaths (5 Allied, 13 IJN), 556 queued hits and 1181 shots.
- Rounds: 272 hit a unit and 894 fell in the water.
- The mission **fails at 188.16 s** on Exeter's sinking (187.36 s, credited to Haguro).
- Some kill credits fall on ships of the same side: Perth to Encounter, and Haguro and Jintsu to
  Tokitsukaze. That is the last-attacker credit; it was not examined here.

docs/GAME_EXECUTABLE.md carries this row as the current USN02 reference, below the
role-bookkeeping section.

## 8. The hull sections and the shell hull test (packet cc9_hull_sections)

### 8.1 Where the sections come from

- **Loader.** `0081F980` fills `unit+A68h` (magazine), `+A78h` (fuel tank) and `+A88h` (engine
  room), each a point plus a validity byte (+Ch), which it clears first (0082042C-00820488).
  - It walks the ship model instance's GeomMesh list (`[[unit+360h]+160h]+3Ch`). For each
    element (stride 2Ch) it checks the element kind at `+4`: kind 8 goes to +A68h (00820566),
    kind 5 to +A88h (008205D7) and kind 6 to +A78h (00820648). The last matching element wins.
  - The point is `00723030(element)`: the midpoint of `element+20h[0]` and `element+24h[0]`, the
    root of the element's min/max box arrays (docs/HIT_HULL_SEGMENT.md correction).
- **Data.** Section elements are authored in each ship model's `GeomMesh` resource. Every
  USN02 class carries all three, except class 289, which has no engine room. The classes hold
  2337 to 10808 triangles.
- **Draw.** `00816650` keeps a section only while its weight is positive, its byte is set, and
  `0093A570` does not list it as destroyed (ids 5, 8 and 6). It then picks by the running sums
  of EngineRoomWeight, MagazineWeight and FueltankWeight.
  - The chance and weights come from the gunner's skill row (robots.lua ArtilleryGunnerBot),
    for example SPNormal 0.2 / 1.0 / 0.1 / 0.1 and SPVeteran 1.0 / 0.5 / 1.0 / 1.0.
  - The roll is drawn only when the chance is positive (00816659).
- **Binding.** `kShipSectionPointsBound`.
  - **Labelled:** the element box is taken as its triangles' box in model space (the producer of
    `element+20h`/`+24h` is unread), and no section is ever destroyed.

### 8.2 The shell hull test

- **Chain.** `00724510` passes the segment in the collision node's space to `00723E90`.
  `00723D60` then tests every element in turn, shortening the far end on each hit, and
  `00723AA0` walks each element's triangle list. So the hit is the closest triangle of the
  ship's GeomMesh (docs/HIT_HULL_SEGMENT.md section 1a).
- **Binding.** `kShellHullHitTestBound`. For a ship target, the segment in the ship's frame is
  tested against every GeomMesh triangle (Möller-Trumbore), and the closest hit wins. The broad
  phase is the mesh bounds, posed.
- **Labelled:**
  - model space with identity nodes (the hull nodes in the models read sit at the origin);
  - no per-element AABB tree (007238E0 unread; this is a speed difference only);
  - planes keep the class box.

### 8.3 Why shots fell 28 % in the aim-point pair

The trigger, not a CanFire conjunct.
- In `local/uC_usn02.log` against `local/uT_usn02.log`, `trigger_rises` goes from 552 to 985
  and the angle steps from 165465 to 133992. `fire_messages` falls from 235345 to 191219 with
  them.
- 006DF520 step 12 arms the trigger only while the gun is within 0.1 degree of both commanded
  angles (006DEE40). Step 4 replaces the body point every 5 s, at once. So each new point moves
  the commanded angles, the turret slews at its rotation speed, and the trigger drops until it
  settles.
- This is the image's rule applied to the image's aim point. It is not a host term.

### 8.4 Predictions (written before the runs)

Pair: USN02 9000, option on. OFF is both new switches off (`local\hC`, aim point ON); ON is both
on (`local\hT`).
- **Hits.** Hits per round fall from about 25 % to 15-22 %: the mesh is thinner than the box at
  bow, stern and above the deck. Category 3 and 6 hits fall 10-30 %.
- **Section picks.** SPVeteran (Allied cruisers) always go to a section when one exists. SPNormal
  (IJN) do so 20 % of the time. Section points sit inside the hull, so rounds aimed at them hit
  the mesh about as often as hull-box points do.
- **Damage per hit:** unchanged. Sections change where the round lands, not its damage class, and
  part damage is not modelled.
- **Sinkings** later by 10-40 s on average. Exeter (169.46 s in the aim-point pair) survives
  longer, and the mission end (173.31 s) moves later or does not happen in 9000 frames.
- **Torpedo rows flat** in launches. Torpedo hits may move with the ship positions.
- **USN04 4500 check** (AA only, same binaries): category 0, 1 and 5 rows identical in shots.
  Category 6 dual-purpose hits against aircraft identical, since the mesh test is for ships
  only. The only rounds that can change are those that strike a ship.

### 8.5 The USN02 pair (option on)

Logs `local/hC_usn02.log` (OFF) and `local/hT_usn02.log` (ON). Both binaries include main
`330b81cdc` and later, so the OFF side differs from section 5's uC/uT runs: its mission fails at
396.03 s, not 173.31 s.

| row | OFF | ON | prediction | held? |
| --- | --- | --- | --- | --- |
| hits per round | 340 / 1324 = 25.7 % | 369 / 1203 = 30.7 % | falls to 15-22 % | **no**: rose |
| category 2 / 3 / 6 hits | 179 / 293 / 197 | 206 / 367 / 174 | categories 3 and 6 down 10-30 % | **no** (3 up 25 %, 6 down 12 %) |
| shots | 1324 | 1203 | - | - |
| section picks / mesh hits | 0 / 0 | 627 / 369 | - | - |
| category 7 launches / hits | 341 / 18 | 346 / 16 | launches flat | yes |
| deaths | 21 | 20 | - | - |
| mission end | failed at 396.03 s | **none** | later or none | yes |

- **Sinking flips:**
  - DeRuyter (144.70 s), Perth (202.81 s), Jupiter (237.11 s) and Asagumo (261.16 s) now sink.
  - Houston, Alden and John1 (all 395.73 s by Asagumo on the OFF side), John3 and Exeter now
    survive.
  - Houston's survival is why the mission does not fail.
- **Why the hit prediction failed.** The mesh is not thinner where it matters: the model's
  triangles include the superstructure, turrets and masts, and the mesh bounds are taller than
  the class box. The section points are inside the hull, at the model's engine room, magazine
  and fuel tank. So rounds aimed at them land on the mesh at least as often as rounds aimed at
  the hull-box points did.

### 8.6 The USN04 check and the decision

- **USN04 4500** (`local/hC_4500.log` against `local/hT_4500.log`, option on): 2 of 726 gun rows
  differ, both category 5. Their hits go from 75 to 79, from two shell mesh hits on a friendly
  hull. Categories 0, 1 and 6 are identical in shots and hits. Deaths are 27 on both sides and
  damage 6154.8 on both. The AA rows are flat, as predicted.
- **Decision: `kShipSectionPointsBound` and `kShellHullHitTestBound` land ON.** The sections are
  the image's GeomMesh section elements, and the hit test is the image's triangle test, both on
  this installation's models. The labelled substitutions are those of sections 8.1 and 8.2.

### 8.7 The landed-state USN02 reference (2026-09-24)

`local\hT` (all switches landed), reference parameters, option off. Log: `local/sR3_usn02.log`.

| damage | deaths | queued hits | shots | rounds hitting a unit / water / expired | section picks | mesh hits | first hit | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 67754.4 | 17 (3 Allied, 14 IJN) | 975 | 1527 | 470 / 1009 / 359 | 599 | 470 | 37.85 s | none |

- **Sunk, Allied:** Witte (39.20 s), Kortenaer (98.25 s) and Electra (136.90 s).
- **Sunk, IJN:** eleven destroyers, and Jintsu (177.76 s) and Haguro (290.40 s).
- Houston and Exeter survive, so the mission does not fail in 9000 frames.
