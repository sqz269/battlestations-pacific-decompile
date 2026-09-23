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
