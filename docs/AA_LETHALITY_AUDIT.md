# How lethal the fleet's AA is against the Kates, and whether the image agrees

Addresses:
- `009030C0` (AAFlakBot tick) and `008FDBE0` (its error roll);
- `00902920` (AAGunnerBot);
- `00730160`-`0073075E` (the per-shot throw cone, `docs/GUN_DISPERSION.md`);
- `0070C370` (flak projectile tick);
- `007BBB70` (the plane's `vtable[34h]`);
- `00901C20`;
- `007325A0` and `00718870` (section 5).

Packet `cc9_aa_lethality_audit`, 2026-09-23. Ghidra was read, not written. All names are
hypotheses. Nothing here is ABI-compatible or game-validated.

## 1. The six Lexington kills, traced

The log is `local\lA_9000.log`: main `5f88773b5`, USN04 E2 9000, RNG option on,
`BSP_AA_TRACE_UNIT=Lexington-class01,Fletcher-class01..04`. It is a measurement trace; the
option now takes a comma list, and the hit line prints the applied damage.
- The Kates have health 220 and armour 6.0.
- A Flak (class 44) round that strikes a Kate logs a direct hit worth 0, then a blast. The blast
  base is 35, so after armour 29 is applied.
- A class 42 machine-gun round does a direct hit of 40 to 45, so after armour about 35 is applied.
- "Rounds" counts every traced ship's rounds at that Kate.

| Kate | killed | damaging hits (last one kills) | window of the hits | killers | rounds at it: cat 1 / 6 / 5 |
| --- | --- | --- | --- | --- | --- |
| B5N Kate #2.1 | 201.96 s | 8: six cat-6 blasts, two cat-1 | 201.81-201.96 | Fletcher-class04 4, Lexington 4 | 58 / 32 / 12 |
| B5N Kate #2.1\|.-2 | 232.06 s | 7: four cat-5 blasts, three cat-1 | 231.46-232.06 | Lexington 7 | 35 / 20 / 12 |
| B5N Kate #6.1 | 281.95 s | 8: five cat-6, one cat-5 blast, two cat-1 | 281.75-281.95 | Lexington 4, Fletcher-class04 4 | 42 / 22 / 12 |
| B5N Kate #6.1\|.-2 | 312.05 s | 7: four cat-5 blasts, three cat-1 | 311.45-312.05 | Lexington 7 | 35 / 22 / 12 |
| B5N Kate #6.1\|.-3 | 330.04 s | 7: one cat-6, six cat-1 | 329.49-330.04 | Lexington 4, Fletcher-class04 3 | 22 / 8 / 4 |
| B5N Kate #6.1\|.-4 | 327.34 s | 7: four cat-6, three cat-1 | 326.79-327.34 | Fletcher-class04 4, Lexington 3 | 46 / 24 / 12 |

**What kills them.**
- Every Kate takes 7 or 8 hits, all inside the last 0.2 to 0.6 s of its life.
- Those hits come from one or two synchronised salvos. The Lexington's two cat-6 mounts and
  Fletcher-class04's two fire together, each twin-counted mount putting two rounds in the air.
- The rounds were fired at 820-1000 m. Every earlier round, fired at 1000-1500 m, missed. That is
  44 to 102 rounds per Kate before the fatal salvo.
- The V0 is 800 m/s for every mount, so the flight time at the kill range is 1.0 to 1.2 s.
- The intercept is on (`docs/AA_LEAD.md`). It includes the 0.05 s + 0.1 s/km AAGunnerErrorModifier
  time biases, 0.14 s at 900 m, which at a Kate's 84 m/s put the aim point about 12 m ahead.

## 2. The chain, term by term

| term | host | image | faithful? | effect on the six kills |
| --- | --- | --- | --- | --- |
| **Gunner skill row** | not applied to AA | `SetSkillLevel(unit, 2)` for all 18 US ships (script, `local\lA_9000.log`), row 2 = SPVeteran | - | decides the next two rows |
| **Flak bot error** `008FDBE0` | none | robots.lua AAFlakBot SPVeteran: GoodRatio 1, AngleErr 0/0/0, DistErr 0/0/0 | **yes**: zero at this skill | 0 |
| **Throw cone** `00730160` | none: the flattened `Throw` is never read | `Throw * BulletThrowMul`; AAFlakBot and AAGunnerBot SPVeteran BulletThrowMul = **0** (`gun_throw_magnitude_0073031d`) | **yes**: zero at this skill | 0 |
| **AAGunner swinging error** (`00902B38`-`00902EF7`) | unbound | unread | unknown | cat 1 only: 17 of the 44 damaging hits |
| **Negative-vertical halving** | bound | `00902F62` | yes | already applied on both sides |
| **Target velocity** | body axis × `0092D730` speed | `vtable[34h]` = `007BBB70`, the world velocity `unit+AC8h` | **no** | drops the velocity off the nose; at an angle of attack of 0.01-0.03 rad, 1-2.5 m/s, 1.5-3 m over a 1.1 s flight |
| **Turn-rate average** `0085E4D0` | not bound | averages V with its rotated image for a turning plane | no | none on a straight torpedo run, which these are at the kill |
| **Flak detonation** | only on a direct strike, then a 35 m blast at distance 0 | `0070C370` proximity search (`0070C4A4`-`0070C795`, unread), then `0070C210`, with the fuse arming time at `classDesc+D8h` | **no** | the image bursts near the target without a strike. With perfect SPVeteran aim that makes flak **more** lethal than the host's, not less |
| **Barrel count** | 2 for every cat-6 dual-purpose mount (it counts Bullet records) | `max(1, "fire" points)` (section 5) | unknown | the cat-6 blasts are 16 of 44 hits. Halving their rate would cost about 8 hits, which delays each kill by one salvo interval |
| **Damage per hit** | Flak blast 35 - armour 6; MG 40-45 - 6 | the same classes (`bulletclasses.lua`) | yes | 7-8 hits per 220-health Kate |

**Verdict.**
- At the skill the mission script sets, the image's ship AA has no aim error and no dispersion.
- The host's AA misses only through geometry. The one term that would make the host more lethal
  than the image is the dual-purpose barrel count, which is unreachable (section 5).
- The largest term that makes the host **less** lethal than the image is the flak proximity
  detonation, which is unread.
- The one bindable divergence is the plane target velocity. Its effect is small.

**So the lethality that kills the Lexington Kates is the image's.** The perfect SPVeteran gunners
are the image's design, not a host artefact. The one bindable term is bound below; it should move
only small things.

## 3. The binding (`kAaTargetWorldVelocityBound`, default true)

The AA branch leads a plane target with `vtable[34h]`, which for a plane is `007BBB70`: a copy of
`unit+AC8h..+AD0h`, its world linear velocity. The host keeps it as the plane's world velocity and
mirrors it into `motion.linear_velocity` every free-flight step. `GameUnitsHost::unit_linear_velocity`
exposes it. The switch replaces the body-axis × `0092D730` substitution. Ship targets are
unchanged.

### Predictions, written before the pair

E2 9000, RNG option on both sides, switch off against on, same tree.
- **The six Lexington Kates** still die before release. Their death times move by less than a
  salvo interval, about 2 s.
- **Torpedo drops** on the Lexington stay at 0.
- **Category 1 and 6 hits** are within ±10%. The lateral lead changes by a few metres only while
  a target turns.
- **Yorktown rows** move only through the coupling of changed kills, if at all.

## 5. The barrel count's source: `007325A0` and `00718870`, read

**`007325A0` BSP_GunClass_LoadFireNodeMuzzleOffsets.** Body `007325A0`-`007327A0`.
- It first calls `00879AD0` BSP_DamageableClass_BindModelPoints on the gun class (`007325BE`).
- It builds `std::string "fire"` (`00CE6798`, length 4) and calls
  `00718870(ECX = [class+50h], &"fire", 0)` at `007325F3`. It then calls the same with index 1 at
  `00732652`, and keeps that second result in `EDI` for branches past `007326DA`, which are unread.
- When the index-0 result is non-null, `00732689`-`007326BF` copy that item's 12-byte element range
  (`[item+48h]..[item+4Ch]`, vector header at `item+44h`) into the class's muzzle list at
  `class+98h..+A0h`, through `00732560`.

**`00718870` BSP_GameResource_LookupNamedPointGroup** (`docs/NATIVE_GAME_RESOURCE_NAMED_GROUPS_BF.md`,
already reconstructed and fixture-tested there).
- It scans the game resource's classified pointer vector at `resource+64h` for a `Points` item
  whose name equals `"fire"` (counted, case-sensitive) and whose identifier index `item+24h`
  equals the second argument. `00717F20` is the contains scan and `00718000` the find, which
  returns the last match.
- The items come from the reader `0071B3E0`: the name at `item+8`, the U32 identifier at
  `item+24h`, the category string at `item+28h`, and `Vector12` points pushed to `item+44h`. That
  reader is audited but not reconstructed.

**What binding the count still needs.**
1. **The object at the gun class's `+50h`, the game resource `007325A0` queries.** Its producer is
   not identified. `docs/MODEL_REACHES_UNIT.md` shows that a unit class's `+50h` does not hold the
   decoded mesh. This installation's `deviceclasses.lua` has no `Model` or `Mesh` key, so the
   resource is not reached through a device row.
2. **A host reader for that resource's `Points` items,** `0071B3E0`. The host's
   `native_resource_hierarchy_parser` reads `Hierarchy`/`Item` nodes, which are a different
   chunk.

With both, `barrel_num = gun_muzzle_count_0072ab80(number of "fire" points, index 0)`.

## 4. The pair

The logs are `local\vC_9000.log` (switch off) and `local\vT_9000.log` (on). E2 9000, RNG option
on both sides, same tree, with the same trace.

| quantity | off | on |
| --- | --- | --- |
| the six Lexington Kates | die at 201.96 / 232.06 / 281.95 / 312.05 / 330.04 / 327.34 s | die at 202.86 / 233.06 / 282.35 / 312.50 / 330.39 / 327.54 s |
| their killers | Lexington 6 | Lexington 3, Fletcher-class04 3 |
| torpedo drops (on the Lexington) | 8 (0) | 8 (0) |
| category 1 hits | 163 | 168 |
| category 6 hits | 138 | 156 |
| category 5 hits | 74 | 48 |
| fighter (cat 0) hits | 76 | 64 |
| deaths / damage | 35 / 7799.5 | 35 / 7770.3 |
| Yorktown damage taken | 0 | 0 |

**Against the predictions:**
- **Held:** all six still die before release, 0.2 to 1.0 s later. There are no torpedo drops on
  the Lexington.
- **Held:** category 1 moved +3%.
- **Missed the ±10% band:** category 6 +13%, and category 5 -35%, where 26 flak blasts moved to
  other mounts and targets.
- **Missed in scope: the Yorktown-side Kates moved directly.** The binding changes the lead
  against every plane, not only the Lexington's attackers. The four #4.1 Kates release in both
  runs, and then die 36 to 63 s later on their turn away (for example #4.1\|.-3 at 243.91 s against
  279.36 s). That is where a plane's velocity leaves its nose and the term matters. Yorktown is
  untouched in both runs.

**Decision:** `kAaTargetWorldVelocityBound` lands ON. The lethality against the Lexington's Kates is
unchanged in kind: they die to the same perfect SPVeteran fire, a fraction of a second later.

## Correction, 2026-09-23 (packet cc9_flak_proximity_burst)

- **Was:** "this installation's `deviceclasses.lua` has no `Model` or `Mesh` key".
- **Is:** `scripts/datatables/autoload/deviceclasses.lua` is a 17-line wrapper. It loads
  `classtables/arcade/deviceclasses.lua` (or the realistic table) into `DeviceClass`. Those rows
  do carry `["Mesh"] = Platform("models/devices/...mmod", ...)` (350 of 416 rows). That mesh is the
  gun class's `+50h` resource, and its `Aux` items carry `Identifier` = (`"fire"`, index) with a
  `Points` list. `docs/FLAK_PROXIMITY_BURST.md` section 6 has the count rule and the three
  Lexington mounts.
- **Evidence:** the earlier grep ran on the wrapper, not on the class table.
  `classtables/arcade/deviceclasses.lua` row 12 (Atlanta 5'' 2X DP) has
  `"models/devices/us/atlanta_turret.mmod"`.

## 7. SPNormal shooters: the AA bots' own errors are missing (packet `cc9_aa_lethality_audit`, cc9-gunnery12, 2026-09-29)

Sections 1-4 found the host's AA faithful **at SPVeteran**, the row USN04's script sets on its US
ships. Two image terms were left out as "zero at this row":
- the AAGunnerBot swinging error;
- the AAFlakBot roll `008FDBE0`.

Neither is zero at any other row.

### 7.1 Census: who shoots the attackers down, at which skill

| row | shooters | their skill | how it is set |
| --- | --- | --- | --- |
| USN04 | Lexington, Yorktown and the US escorts | 2 SPVeteran | `Mission.SkillLevelOwn` (18 `SetSkillLevel` calls) |
| USN13 | the US fleet against the Japanese strikers | 2 SPVeteran | `usn_13_truk.lua` 250-254 at difficulty 1 |
| USN13 | the Japanese escape and Katori groups against US planes | 1 SPNormal | `Mission.SkillLevel` (lines 486, 515) |
| JM05 | Arike, Haguro, Shigure, Ushio, Yugure and the other IJN ships | **1 SPNormal** | no call (`skills=0`); the scene entity has neither `Skill` nor `Crew`, so `00927A80` falls back to 1 (`00822C20` at `008238C1`, `[unit+C0h]+4 == 1`) |

- **JM05 9200/9000** is lua15's `l15_rin_jm05l.log`: 0 of 24 torpedo-task releases, 29 deaths.
- **Every torpedo plane lost there dies at 30 to 76 m, 775 to 1089 m from its killer, to
  category 1** (AAGunnerBot, 13 to 15 hits of about 15 each) or to a category 6 blast.
- USN04's SPVeteran shooters are the image's (section 2). **The divergence is on the SPNormal
  rows.**

**The scene path the host does not take.** `00822C20` sets a scene-placed unit's skill from its bag
at `008238B1..008238CB`: `Skill`, else `Crew` through `006E6210` (Rookie 0 -> Stun 0, Regular 1,
Veteran 2, Elite 3 -> 5), else 1. The host keeps 1 until a script call.
- In JM05 this reaches only land forts and convoys (50 `Crew = Rookie`, so **Stun** in the image)
  and the two carriers (`Skill = SPVeteran`), not the AA ships.
- It is **a separate host gap**, recorded here and not bound. It changes the forts' guns from row
  1 to row 0.

### 7.2 The two errors, from the listing

**AAGunnerBot, `00902920`, `00902AE1..00902F5D`.** `R = [00E19998] + skill * 10h` holds
`AngleDiffErrorRatio` at +0Ch and `ConstAngleError` at +14h.
1. **The period.** `bot+60h -= dt`. When it goes negative it reloads with
   `U(3, 8) * InterpolateClamped(0, 1, 6.0 (00CE6630), 0.2 (00CE54A0), skill)` (`00419010` at
   `00902B38`). That is 0.6 to 1.6 s at SPNormal and 18 to 48 s at Stun.
2. **The spread.**
   - `e = R+0Ch * AddWrapped(dh, dv)`, where dh and dv are the angle gaps between the target's
     `vtable[100h]` hull-box point and the solved lead aim (`00902BFD..00902CD5`).
   - Plus `U(0, R+14h)` degrees (`00902CF0`).
   - A ship target divides `e` by settings +750h (2.0, AI slot) or +754h (1.2).
3. **The new offsets.** They are two Gaussian draws `N(0, e)`: `00BD2F90` is polar Box-Muller,
   `00BF0DF0`. The rates `bot+6Ch/+70h` are `(new - current) / period`.
4. **Every tick.** The offsets step by `rate * dt`. The clamp is `25 / distance` (`00CE3880`,
   25 m at the target). An offset past it is clamped (`00415690`), the timer is set to 1.0, and
   both rates become `-0.5 * offset` (`00902E56..00902EA9`, `00902EE0..00902F2B`).
5. **The offsets are added to the solved angles** (`00438AA0`, `00902F3E` / `00902F58`), before
   the negative-vertical halving `00902F62`.
6. **At SPNormal** (ratio 5, constant 4 degrees) against a plane at 1000 m, the lead gap alone is
   about 5 degrees. So `e` is about 25 to 30 degrees, and nearly every draw is clamped. **The MG
   aim point wanders about 25 m off the lead point in each axis.**

**AAFlakBot, `009030C0` and `008FDBE0`.** Row `[00E1999C] + skill * 20h`.
- **The angles** are solved plus `bot+58h` / `bot+5Ch` (`00903280..00903293`).
- **The reroll.** After a shot (`bot+64h < gun+474h`, `0090330A..0090331B`), `008FDBE0` rolls
  again:
  - with `U(0, 100) < GoodRatio * 100` each value takes `U(Min, Max)`, otherwise `U(Max, Bad)`;
  - each has a random sign;
  - the angles are converted from degrees.
- **The distance** is handed to the next round as `"distErr"` (`00730F70`, the string at
  `00CFD51C`). That is the `+290h` which `0070C6C6` adds, unscaled, to the burst distance.
- **At SPNormal:** GoodRatio 0.4, angle 0 to 2 or 2 to 4.5 degrees (35 to 78 m at 1000 m, against
  a 35 m blast), distance 0.4 to 3 m.

### 7.3 The bindings, prepared but not landed

The two switches are `kAaGunnerSwingErrorBound` and `kAaFlakAimErrorBound`, committed OFF with
the rows from this installation's `robots.lua` (2025-06-01) and `shipglobals.lua` (2024-07-13).
- They are written in `src/game_hosts_gunnery.cpp`, with one field in its header.
- **These files are leased to cc9-ships14** (`cc9_kaiten_contact_detonation`, until 20:38 UTC).
- The patch is `local\g12_aa_patch.diff` in the cc9-gunnery12 tree, and it lands when the lease
  frees.
- **Substitutions:**
  - the target point is the host's aim point, not the hull-box draw;
  - the clamp distance is the muzzle-to-lead distance.
- **Census counters:** a `summary mission gunnery aa bot error` line gives the rolls by level,
  the clamps and the mean miss distance.

### 7.4 Predictions (written before any ON run)

The pairs are same-tree exports with the inertia switch as on main, the two switches flipped
together on the ON side. The rows are JM05 9200/9000, USN13 9200/9000 and USN04 E2 9200/9000, with
the RNG option on.
- **P1, the census.** On every row the rolls by level show skill 1 for the SPNormal shooters. On
  USN04 and USN13 the US ships roll at level 2 with zero spread. Their offsets stay 0.
- **P2, JM05.**
  - Torpedo-task releases rise from 0 of 24 to at least 6.
  - Category 1 hits on aircraft fall by at least 40%.
  - Aircraft deaths below 150 m fall by at least a third.
  - The gunner's mean miss is 15 to 25 m (the clamp).
- **P3, USN04 E2.** The Kates attacking the Lexington still die to SPVeteran fire: their death
  times move only by coupling, and there are no Lexington torpedo drops. Only fire from the IJN
  ships (SPNormal) against US aircraft becomes less lethal.
- **P4, USN13 9000.** The Japanese strikers' losses to the US fleet are unchanged in kind. US
  aircraft losses to the Japanese escape groups fall.
- **Mechanism failure:**
  - any SPVeteran-row gun with a non-zero offset;
  - JM05 torpedo releases staying at 0 while category 1 hits on aircraft fall by less than 20%.

### 7.5 The pairs and the verdict: ON, with P2's size recorded as a miss

**Corrections to 7.1 and 7.2, made before the pairs were scored:**
- **The period.** `00419010` takes (x0, y0, x1, y1, x). `00902B38` pushes (0, 1.0, 6.0, 0.2, skill),
  so the multiplier is 1.0 at Stun and 0.867 at SPNormal. The existing
  `gun_bot_lead_error_span_00902920` (src/gun_bot_ticks.cpp) has it right, and the binding calls
  it.
  - The SPNormal period is **2.6 to 6.9 s**, not 0.6 to 1.6 s.
  - The 25 m clamp is `gun_bot_lead_error_limit_00902920`.
- **The scene skill (lua16, `docs/SCENE_UNIT_SKILL.md`, main `da90b8653`).**
  - The binding is ON and faithful.
  - 7.1's "50 Rookie forts become Stun" is **refuted**. `00927A80` reads the merged bag, and this
    installation's library gives Ship, LandFort, LandConvoy and PlaneSquadronWNavpoint a group
    default `Skill = SPNormal`, so they resolve to 1.
  - The SPNormal premise for JM05's IJN ships stands.

**The pairs.** Same-tree exports of `a3da8863f` (both switches committed OFF), with the ON side
flipping both. Main's inertia switch is on in both. RNG option on. Logs are
`local\aa<off|on>_<row>.log`. A 300-frame USN01 smoke ran first.

| row | exit | census (ON) | headline OFF -> ON |
| --- | --- | --- | --- |
| E2 9000 | **1** | gunner rolls 880, all level 2; flak rolls 851, all level 2; mean miss 0.00 m | gameplay identical |
| JM05 9000 | 3 | gunner rolls 1372, all level 1; clamps 1377; mean miss 18.45 m; flak rolls 159 (mean 2.38 deg, distErr 1.79 m) | deaths 29 -> 27 (Japan Troop Transports 04 and 05 survive), hit records 1326 -> 1059, torpedo-task releases 0 -> 2 of 24 |
| USN13 9000 | 3 | gunner rolls 5906 at level 1 and 2186 at level 2; flak rolls 370 / 2131 | deaths 142 -> 123, hit records 4557 -> 4405, shots 26345 -> 32856, dive-bomb releases 0 -> 1 of 50 |

Aircraft deaths below 150 m (`local\g12_aacount.py`):

| row | OFF: deaths, killers, category 1 hits, killer range | ON |
| --- | --- | --- |
| JM05 | 12; category 1 ×10, category 6 ×2; 146; 705-1269 m | 12; category 1 ×11, category 6 ×1; 162; 200-1190 m |
| USN13 | 33; category 1 ×30, category 6 ×3; 450 | 14; category 1 ×14; 234 (-48%) |

**Against 7.4:**
- **P1, the census: held.** Only level-1 guns get offsets, and every level-2 gun keeps 0 (E2
  mean miss 0.00 m).
- **P3, E2: held.** The row is gameplay-identical.
- **P4, USN13: held.** Low aircraft losses fall from 33 to 14 and category 1 hits by 48%. Losses to
  the SPVeteran US fleet stay.
- **P2, JM05: missed in size.**
  - The gunner's miss is 18.45 m, inside the predicted 15-25 m, and nearly every roll is clamped.
  - Some torpedo planes now get to 200 m, and two releases happen.
  - But the low deaths stay at 12, category 1 hits on them do not fall (146 -> 162), and the
    releases reach 2 against the predicted 6 or more.
  - JM05's shooters are close-in escorts, with 13 to 15 MG hits per kill from many barrels. A 25 m
    wander at 700-1200 m still lands enough of them.
- **Mechanism failure (7.4): not met.** No level-2 offset appeared. JM05's releases did not stay
  at 0.

**Verdict: ON** (both switches), with P2's size recorded as a miss. The mechanism is the image's
and acts only where the image's rows say it should. What still kills JM05's torpedo planes
before release is a separate question. Next candidates:
- the image's `vtable[100h]` hull-box point, which the binding substitutes with the aim point;
- plane HP and armour against the MG bullet class;
- the barrel counts of the IJN destroyers' MG mounts.
