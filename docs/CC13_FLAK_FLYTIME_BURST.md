# Flak FlyTime expiry burst `0070C3C8` -> `0070C210` (packet cc13-flak1)

Addresses: `0070C370` (BSP_FlakProjectile_TickAdvance, the expiry at `0070C3AA`-`0070C3C8`),
`0070C210` (BSP_FlakProjectile_Detonate, body `0070C210`-`0070C366`), `006E6490`
(BSP_Projectile_TickAdvance, its own expiry at `006E6584`-`006E65A9`), `0084BAD0`
(BSP_Explosion_ApplyRadialDamage), `0084BE74` (the impact burst's ignore-falloff test),
`004705C0` (the falloff fraction).

Packet cc13-flak1, 2026-10-12, for cc9 (owner of `src/game_hosts_gunnery.cpp`). Read from the
disk listing (`bsp.py disasm-raw`). Ghidra was not written. All names are hypotheses. Nothing here
is game-validated: the diff below is **build-tested** in this worktree with both switches OFF and
then reverted. cc9 owns the pair runs and the decision.

## 0. Short answer

- In the image a Flak round that outlives FlyTime **detonates**. It is not removed silently.
  `0070C3B7 PUSH 0; CALL 0070C210`, and `0070C210` never reads its argument. The expiry therefore
  runs the same code as the proximity lock's burst: the draw `U(BlastDamageMin, BlastDamageMax)` on
  stream 1 (`0070C247`), then the radial blast `0084BAD0` at the round's position with radius
  BlastRange and **ignoreFalloff = 1**.
- The host removes the round silently at its life bound (`game_hosts_gunnery.cpp`, the
  `range > 0 && life * speed > range` test after the water crossing). That is one draw and one
  blast missing per expiring flak round.
- A second difference came up while reading this code. **Every flak blast in the image ignores
  falloff.** The detonation pushes 1 at `0070C26E`. The impact burst sets the flag from
  `classDesc+8h == 10h` (`0084BE74 SETE`), and sub-type 10h is Flak
  (`kProjectileSubTypeFlak`). The host's `apply_impact_blast` hard-codes `ignore_falloff = false`
  for every blast, so the flak proximity bursts already landed ON (`kFlakProximityBurstBound`)
  fall off with distance, and the image's do not.
- The diff (section 5) adds two switches that default OFF:
  - `kFlakFlyTimeBurstBound`: the expiry burst, with ignoreFalloff 1.
  - `kFlakBlastIgnoreFalloffBound`: ignoreFalloff 1 on the flak proximity burst and on the flak
    impact burst.

  With both OFF, the build output is the same as main's behaviour. The two switches can be paired
  separately.

## 1. The image

### 1.1 The expiry (`0070C370`, `0070C387`-`0070C3D7`)

`ESI` is the tick node, so the round is `ESI - 244h`.
1. `0070C387 CALL 006E6490`: the base tick. It does the flight step, the direct-strike sweep and
   the tracer. It also adds the scaled step to flight time `+1C4h` (`006E64B8`-`006E64BF`), where
   the step is `step * class+5Ch` and `+5Ch` is 1 for Flak.
2. `006E6584`-`006E65A9`, inside the base tick: when `+1C4h > class+54h`, it calls
   `00696350(proj, 0)` and then `00926D90(proj, 2)` (`BSP_MissionEntity_Kill`, cause 2). This only
   **marks** the round dead. `[node-8]` is the round itself (`include/bsp/tick_element_overrides.hpp`
   `projectile_expire_00696350`).
3. `0070C3AA`-`0070C3B5`: the flak tick tests again. `FLD [ecx+54h]` (FlyTime), `FLD [esp+14h]`
   (flight time `+1C4h`), `FCOMI`, then `JBE 0070C3DA`. It expires when flight time > FlyTime,
   strictly. This is the same test as in step 2.
4. `0070C3C1 MOV [esi+50h], 0` clears `+294h` (the lock target), and `0070C3C8 CALL 0070C210` runs
   with `ECX = round` and the stack argument 0. Then the tick returns (`0070C3CD`-`0070C3D7`), so
   the lock search and the locked-distance step do not run on this tick.

`00926D90` returns at once when `[this+5Fh]` is already 1 (ledger), so the second kill inside
`0070C210` is a no-op. **Nothing between step 2 and the blast tests whether the round is dead.** The
expiry's burst therefore always runs.

**FlyTime is the finalised field.** `006E9941` rewrites `+54h` as `Range / V0` when a class
authors `Range` (`src/bullet_engagement_range.cpp`). None of this installation's Flak classes
authors `Range` (section 3), so `+54h` is the authored FlyTime.

### 1.2 The detonation (`0070C210`, `__thiscall(round, int unused)`, `RET 4`)

| site | what |
| --- | --- |
| `0070C217`-`0070C221` | `BSP_EntityPose_RefreshWorld(round)` unless the pose byte `+C8h` is set |
| `0070C226`-`0070C247` | `ECX = 1`, stack `[class+B4h], [class+B8h]`, `CALL 00BD2F10`: the draw `U(BlastDamageMin, BlastDamageMax)` on **stream 1**; the result goes to a local (`0070C253`) |
| `0070C257` | `EDI = [round+238h]`, the source entity |
| `0070C25F`-`0070C261` | a second pose refresh, under the same gate |
| `0070C266`-`0070C280` | `0084BAD0(ECX = &round+FCh (centre), EDX = &class+70h (BlastRange); &damage, 1 (ignoreFalloff), [round+238h], round)` |
| `0070C285`-`0070C325` | when `class+2Ch` is non-null: `008685E0`, the point effect at the centre in the world's `+19ECh` effect set (a handle with a reference count, released at `0070C315`) |
| `0070C327`-`0070C350` | when `round+198h` is non-null: `004842C0`, the trail effect's world point is set to the centre |
| `0070C355`-`0070C359` | `00926D90(round, 1)` |

The read finds no stack read between the prologue and `0070C280`, and the stack argument is never
loaded (`[ESP+18h]` after the three pushes). That confirms the cc13-rng2 finding
(`docs/CC13_RNG_STREAM_CENSUS.md` 10.2): proximity bursts and expiry bursts are the same burst.

**Draw order on the tick.** In the image the expiry tick draws nothing in the base tick or the
expiry test before `0070C247`. The draw is the round's last.

### 1.3 The blast (`0084BAD0`)

The host already reconstructs it. `src/blast_damage.cpp` and `include/bsp/blast_damage.hpp`
(`blast_apply_record_fill_0084bad0`) model the record fill. The game host's own version is
`GameGunneryHost::Impl::apply_impact_blast`, which the proximity burst and the impact burst both
use. That helper draws `Draw::blast_damage` (`U(min, max)`; on the keyed H-G stream when the RNG
option is on, otherwise `random_range_00bd2f10`), gathers the units inside BlastRange, skips the
shooter (`0084BBF9` skips the source entity), and calls `apply_hit` with a blast record. Its one
departure that matters here is the flag: `blast.ignore_falloff = false` for every blast.

The falloff is `004705C0`. It computes `f = 1 - d / BlastRange`, and when ignoreFalloff is set it
raises any `f > 0` to 1 (`src/blast_damage.cpp`, `src/unit_hit_path.cpp:34`). With the flag, a
flak blast deals its full damage to every part within BlastRange.

### 1.4 Which rounds

Only rounds whose class `Type` is `"Flak"` run `0070C370` (the flak vtable `00CFD554`, slot `+8h`).
Every other projectile reaches only `006E6490`'s expiry pair, which has no draw and no blast. The
guns that fire Flak rounds are the category 5 flak mounts and the category 6 dual-purpose mounts
firing their second ammunition at aircraft (`docs/FLAK_PROXIMITY_BURST.md` 1). The host's rows
carry `type == "Flak"` for both, because the second-ammunition row copies `type2`.

## 2. The host

- **The life bound.** This is the last statement of the projectile loop. The test is
  `range > 0 && life * speed > range`, where for Flak `range` is the row's derived engagement
  range `FlyTime * V0` (`bullet_engagement_range.cpp`, gun group) and `speed` is V0. It sets
  `fate = 5`, counts `summary.expired`, and sets `alive = false`. It has no draw and no blast.
- **Order.** In the host the direct strike runs first, then the proximity search, then the water
  crossing, then the life bound. In the image it is the base tick (direct strike), then the
  expiry, then the search. The diff puts the ON branch where the image has it: after the direct
  strike, before the proximity block.
- **ignoreFalloff.** `apply_impact_blast` sets `false` for every blast (`game_hosts_gunnery.cpp`,
  the `blast.ignore_falloff = false;` line), including the flak proximity burst.

## 3. This installation's Flak classes

`scripts/datatables/classtables/arcade/bulletclasses.lua` (modified 2026-05-09 in this
installation; the install is modded). Read with `local/fk1_flak.py`. Every Flak class has
BlastRange > 0, none authors `Range`, and none authors `FlakFuseTime`.

| Name | V0 | FlyTime | MinRange | BlastRange | BlastDamage |
| --- | --- | --- | --- | --- | --- |
| Akizuki Flak bullet (3 rows) | 800 | 2.5 | 200 | 30 / 40 / 50 | 35 / 35 / 55-75 |
| 5inch/38 destroyer AA Flak bullet (2 rows) | 800 | 3 / 2.5 | 200 | 35 | 35 |
| Jap Flak bullet | 800 | 2.5 | 200 | 35 | 35 |
| 5inch/38 AUSTIN AA Flak bullet | 800 | 2.5 | 200 | 60 | 60 |
| Yamato 1945 Flak bullet (7 rows) | 900 / 1200 | 3.5 | 1000 | 130-250 | 125-400 |
| Yamato 1945 Type 3 Flak Bullet | 900 | **10** | 1000 | **4500** | **3500-4000** |

So a 5-inch round expires 2000-2400 m out. The last row is a modded outlier. With the expiry
switch ON, each of its rounds that does not lock bursts 9 km out with a 4.5 km radius, 3500+
damage and no falloff. Check which missions arm that class before you read a pair that includes it.

## 4. Precision

Nothing in the diff adds arithmetic that is sensitive to precision (`docs/CC13_FPU_PRECISION.md`).
- Flight time `+1C4h` is stored to a float on every tick (`006E64BF FSTP`). It is the sum of two
  floats, and double rounding of that sum through 53 bits is innocuous, so it is bit-identical at
  24 and 53 bits. The expiry tick does not depend on the thread that runs the round.
- The image compares `+1C4h > +54h` directly. The host compares `life * V0 > FlyTime * V0`, with
  the right-hand side rounded to a float when the range was derived. The two can disagree by one
  tick only when the product rounds across the boundary. The diff keeps the host's own test, so
  that ON and OFF expire a round on the same tick and the pair isolates the burst. This is labelled
  in the code.
- For all of the 5-inch and Japanese classes the draw has min = max, so its value is exact whatever
  the precision. For the Yamato rows (min != max) the `low + (high - low) * u` step carries the same
  precision caveat as every other `00BD2F10` draw the host makes.

## 5. The diff

It applies to `src/game_hosts_gunnery.cpp` at origin/main `865e3acf0`. I checked it with
`git apply --check` against a clean copy of that file and built it in this worktree
(`./scripts/build.ps1`) with both switches OFF. The constexpr branches are compiled in either
state. The source was then reverted. Nothing outside this document changed in the tree.

```diff
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index bcd6c40f0..5a534bd59 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -364,6 +364,21 @@ constexpr bool kAaTargetTurnAverageBound = true;
 //    along its track (0070C210, the radial blast). OFF: direct strikes only.
 //    Packet cc9_flak_proximity_burst.
 constexpr bool kFlakProximityBurstBound = true;
+//  * kFlakFlyTimeBurstBound: a Flak-type round whose flight time passes FlyTime
+//    bursts where it is. 0070C3B1 -> 0070C210(proj, 0): the argument is never
+//    read, so the expiry runs the same detonation as the proximity lock - the
+//    U(BlastDamageMin, BlastDamageMax) draw (0070C247, stream 1) and the radial
+//    blast 0084BAD0 with ignoreFalloff = 1 (0070C26E) - before the lock search.
+//    OFF: the life bound below removes the round silently.
+//    Packet cc13-flak1, docs/CC13_FLAK_FLYTIME_BURST.md.
+constexpr bool kFlakFlyTimeBurstBound = false;
+//  * kFlakBlastIgnoreFalloffBound: every flak blast takes full damage inside
+//    BlastRange. The detonation pushes ignoreFalloff = 1 (0070C26E) and the
+//    impact burst sets it from classDesc+8h == 10h, the Flak sub-type
+//    (0084BE74 SETE); 004705C0 then lifts a positive falloff fraction to 1.
+//    OFF: the proximity and impact bursts of a flak round fall off with
+//    distance. Packet cc13-flak1.
+constexpr bool kFlakBlastIgnoreFalloffBound = false;
 //  * kGunBarrelCountBound: gun+448h, the barrel count, is 0072AB80 on the
 //    muzzle list 007325A0 builds from the device model (the row's `Mesh`,
 //    [class+50h]): ("fire", 0)'s whole Points list, else one first point per
@@ -2172,6 +2187,7 @@ struct GameGunneryHost::Impl {
     std::string smoke_first_unit;
     unsigned long long flak_locks{0};           // packet cc9_flak_proximity_burst
     unsigned long long flak_bursts{0};
+    unsigned long long flak_expiry_bursts{0};   // packet cc13-flak1, 0070C3C8
     // Packet cc9_gun_barrel_count: the device model's muzzle list, one load
     // per device class (the image does it once per weapon class, 007325A0).
     struct DeviceFirePoints {
@@ -3100,8 +3116,9 @@ struct GameGunneryHost::Impl {
     // 0084BC60 step 7 (0084BE28..0084BEE3) and the gather behind 0084BAD0: the
     // radial burst every class with a Blast table makes on impact, and the one
     // that carries a torpedo's warhead. docs/TORPEDO_WARHEAD.md.
+    // ignore_falloff is 0084BAD0's [ESP+8] byte, copied into record+2Ch.
     void apply_impact_blast(std::size_t shooter, std::size_t gun_row, int team_id,
-        const float point[3], const float direction[3]);
+        const float point[3], const float direction[3], bool ignore_falloff = false);
     void kill_unit(std::size_t victim);
     // Packet cc9_gunnery_line_of_sight: 00864680 for (observer, target).
     bool line_of_sight_00864680(std::size_t observer, std::size_t target);
@@ -8883,6 +8900,42 @@ void GameGunneryHost::Impl::run_projectiles(float dt) {
         // FLT_MAX, 0070C4B1), and on that tick +284h is still FLT_MAX from
         // 0070CB30, so 0070C7CE records the distance and never draws.
         // docs/AA_LETHALITY_AUDIT.md section 11.
+        //
+        // Packet cc13-flak1. 0070C3AA..0070C3C8 run after the base tick and
+        // before the lock search: flight time (+1C4h) > FlyTime (+54h, FCOMI /
+        // JBE) clears +294h and calls 0070C210(proj, 0), which never reads its
+        // argument (RET 4; no [ESP+..] read before 0070C280). So the expiry is
+        // the same detonation as the lock's, at the round's own position
+        // (proj+FCh): the 0070C247 draw, then 0084BAD0 with radius BlastRange
+        // (+70h), ignoreFalloff 1 and the round's source entity (+238h)
+        // skipped. The base tick's own expiry pair at 006E6584 (00696350,
+        // 00926D90(proj, 2)) only marks the round dead first; 0070C210 does
+        // not test that, and its 00926D90(proj, 1) returns on +5Fh.
+        // SUBSTITUTIONS, labelled: the test is the life bound's own
+        // life * V0 > FlyTime * V0 (the row keeps the derived range, not
+        // FlyTime), so ON and OFF expire on the same tick; the source entity
+        // is the owner unit, as for the lock's burst; the point effect
+        // (0070C2EC, class+2Ch) and the trail's world point (0070C350) are
+        // presentation and are not modelled.
+        if constexpr (kFlakFlyTimeBurstBound) {
+            const GameBulletClassRow* const xc = bullet(shot.bullet_class);
+            if (xc != nullptr && xc->type == "Flak" && xc->range > 0.0f
+                && xc->muzzle_speed > 0.0f && shot.life * xc->muzzle_speed > xc->range) {
+                const float no_dir[3] = {0.0f, 0.0f, 0.0f};
+                const float at[3] = {shot.position[0], shot.position[1], shot.position[2]};
+                shot.flak_target = 0;   // 0070C3C1
+                ++flak_expiry_bursts;
+                ++summary.expired;
+                shot.fate = 5;
+                round_bullet_class = shot.bullet_class;
+                apply_impact_blast(shot.owner_unit - 1, shot.gun_row, shot.team_id_1c, at,
+                    no_dir, true);
+                round_bullet_class = -1;
+                done("FlakProjectile::expiry_detonate_0070c3c8", 0x0070c3c8u);
+                shot.alive = false;
+                continue;
+            }
+        }
         if constexpr (kFlakProximityBurstBound) {
             const GameBulletClassRow* const fc = bullet(shot.bullet_class);
             if (fc != nullptr && fc->type == "Flak" && fc->blast_range > 0.0f
@@ -10160,7 +10213,8 @@ std::vector<int> GameGunneryHost::destroyed_hull_segments(std::size_t unit_index
 }
 
 void GameGunneryHost::Impl::apply_impact_blast(std::size_t shooter,
-    std::size_t gun_row, int team_id, const float point[3], const float direction[3]) {
+    std::size_t gun_row, int team_id, const float point[3], const float direction[3],
+    bool ignore_falloff) {
     if (gun_row >= guns.size() || shooter >= unit_state.size()) return;
     const GameGunRow& gun = guns[gun_row];
     const GameBulletClassRow* weapon = bullet(round_bullet_class >= 0 ? round_bullet_class : gun.bullet_class);
@@ -10283,7 +10337,8 @@ void GameGunneryHost::Impl::apply_impact_blast(std::size_t shooter,
         blast.hull_damage_base = 0.0f;  // the burst's damage is +28h, see above
         blast.part_damage_base = damage;
         blast.falloff_range = weapon->blast_range;
-        blast.ignore_falloff = false;
+        blast.ignore_falloff = ignore_falloff
+            || (kFlakBlastIgnoreFalloffBound && weapon->type == "Flak");   // 0084BE74
         blast.armour_selector = 0.0f;
         blast.hull_segment = kDirectHitHullSegment;
         blast.weapon_scale = 1.0f;
@@ -13240,6 +13295,10 @@ void GameGunneryHost::report() {
         host.log.notef("summary mission gunnery flak proximity locks=%llu bursts=%llu bound=%d "
             "(0070C370, packet cc9_flak_proximity_burst)", host.flak_locks, host.flak_bursts,
             kFlakProximityBurstBound ? 1 : 0);
+        host.log.notef("summary mission gunnery flak expiry bursts=%llu bound=%d "
+            "ignore_falloff_bound=%d (0070C3C8 -> 0070C210, packet cc13-flak1)",
+            host.flak_expiry_bursts, kFlakFlyTimeBurstBound ? 1 : 0,
+            kFlakBlastIgnoreFalloffBound ? 1 : 0);
         host.log.notef("summary mission gunnery aa bot error gunner_rolls=%llu (by level %llu/%llu/"
             "%llu/%llu/%llu/%llu) clamps=%llu ticks=%llu mean_miss_m=%.2f gunner_bound=%d "
             "flak_rolls=%llu (by level %llu/%llu/%llu/%llu/%llu/%llu) mean_angle_deg=%.3f "
```

What it does:
- **`kFlakFlyTimeBurstBound` (OFF).** ON: before the proximity block, a Flak row with
  `life * V0 > range` clears `flak_target`, counts `flak_expiry_bursts` and `summary.expired`, sets
  `fate = 5`, and calls `apply_impact_blast(..., shot.position, no_dir, true)`. That call is the
  draw on `Draw::blast_damage` (the same key and stream as the proximity burst's) followed by the
  blast with ignoreFalloff. The round is then dead. There is no back-off, because the direction is
  zero, as in the proximity burst.
- **`kFlakBlastIgnoreFalloffBound` (OFF).** ON: `apply_impact_blast` sets the record's
  `ignore_falloff` for any row whose `type == "Flak"`. That covers the proximity burst and the
  direct-impact burst (`0084BE74`). The new parameter carries the flag for the expiry burst
  independently, so the expiry switch is faithful on its own.
- **Census line.** `summary mission gunnery flak expiry bursts=N bound=B ignore_falloff_bound=F`.

**Substitutions (labelled in the code):**
- The expiry test is the host's range form (section 4).
- The source entity is the owner unit, as for the lock's burst.
- The point effect and the trail world point are presentation, and they are not modelled.

## 6. Expected effect, written before any run

Reference: the cc9 pair in `docs/FLAK_PROXIMITY_BURST.md` 4, USN04 at 9000, burst ON. There,
category 5+6 fired 727 flak rounds, of which 591 locked and 570 burst.

**`kFlakFlyTimeBurstBound` ON alone.**
- Up to about 150 more blasts per USN04-9000 run (727 - 570, less the direct strikes). That is
  every flak round that neither burst nor struck directly; the census line counts them.
- Most do nothing. A round that never locked never passed within `0.5 L + 2 * BlastRange` of a
  plane, where L is V0 times the step (at least 70 m), and its blast radius is 35 m. A plane can be
  inside the radius at expiry only if it moved into it on the last step.
- At most about 20 do something: the rounds that locked but neither burst nor struck
  (591 - 570 = 21 at 9000, less the direct strikes). These are the rounds that expired before they
  reached the intercept. The image bursts them where they are, with a plane in or near the lock
  sphere.
- **Prediction:** category 5/6 hits rise a little (+0 to +25 at 9000). Kate deaths move by at most a
  few seconds, and torpedo drops do not change. Expect no friendly hits, because the expiry points
  are in empty air far from the fleet.
- The RNG option is on, so the extra `Draw::blast_damage` draws sit on their own keys and do not
  shift other streams. With the option off, the shared generator takes about 150 extra draws, and
  everything after the first expiry moves (the RNG coupling rule).

**`kFlakBlastIgnoreFalloffBound` ON alone.** This has more reach. Every proximity burst that hits
a part at distance `d` deals the full damage instead of `(1 - d/35)` of it. With DistErr 0 at
SPVeteran the bursts land near the aim point, but the part distances still run from 0 to 35 m.
- **Prediction:** category 5/6 damage per hit rises by 10-50%, Kate deaths come earlier by a few
  seconds more than the burst landing did, and one Lexington or Yorktown release may be lost.

**Missions.** USN04 (US flak against Kates and Vals) is the reference. It applies to any mission
where flak mounts or dual-purpose mounts fire at aircraft: Japanese flak (Akizuki, "Jap Flak",
Yamato classes) against US bombers in the IJN campaign, and US flak against Bettys and Nells. I
did not establish which missions arm the 4500 m Yamato Type 3 row. Run that mission separately if
it is armed.

## 7. Uncertainty

- `0070C210`'s argument: the read finds no stack read, but `0070C210` is the only consumer. A
  virtual call inside `0084BAD0` cannot see the caller's frame argument, so the expiry burst is
  certain from the listing.
- The base tick's `00696350(proj, 0)` (an observer notification, event 0) runs before the burst. I
  did not trace it to the end. It is the same call a non-flak round gets at expiry, and the host
  models neither.
- Whether the round's death by cause 2 before the blast changes the blast record's shot credit
  (`00470350(shot)` reads the shot pointer, not its state): I read it as no change, but did not
  verify it beyond the ledger's description of `00926D90`.
- `include/bsp/projectile_impact.hpp` names sub-type 10h `kProjectileSubTypeDecalVariant`. In the
  impact test it is `kProjectileSubTypeFlak` (`projectile_kinds.hpp:136`), the same value. The
  first name should be corrected by that file's owner.

## 8. Status

Exported: no new export. Reconstructed: the diff, for cc9. Build-tested: yes (this worktree,
switches OFF). Fixture-tested and game-validated: no. The pair is cc9's to run.
