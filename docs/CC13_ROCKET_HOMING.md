# The rocket's air step and its homing branch (packet cc13-homing1)

Status: **reconstructed, build-tested** (`src/rocket_homing_cc13.cpp`,
`include/bsp/rocket_homing_cc13.hpp`). The host glue in section 6 is a **suggested diff, NOT applied**,
behind `bsp::kRocketHomingBound = false`. It was applied in the worker tree, built, and reverted. No
fixture, no game run. Names are hypotheses.

Background: docs/CC13_ROCKET_TASK.md sections 5, 7 and 9; docs/BOMB_FAMILY_TICK.md (the tick element
that calls the step); docs/CC13_ROCKET_KAMIKAZE_SCOPE.md (the rocket census of this installation).

## 0. Short answer

- **Who homes.** Only rockets that the plane AI fires. `0080AA20` writes the homing block, and it
  has exactly two callers. The first is the rocket task's aim tick (`007B56F6`, angle =
  `StrikeHomingAngle`). The second is the opportunistic rocket sub-object (`009FD4FB`, angle =
  `Rocket_SmallPlane/LargePlane/Ship/Landfort.HomingAngle`). Both angles come from **robots.lua
  `PilotBot`**, not from the bullet table: no bullet row carries a homing field. A rocket fired by a
  gun or a ship never homes, because the constructor zeroes `+47Ch` at `0080ABB5`.
- **The steering law.** Each step, after ignition:
  1. Take the target's body point (`+480h`, in the target's frame) to world.
  2. Lead it by the target's velocity times `00809E10`'s flight time.
  3. Express the lead point in the rocket's frame and divide by `0.5 x max(z, VMax)`.
  4. Clamp x and y to `+/-0.05 x HomingAngle`.
  5. Set the forward row to `forward + x'·right + y'·up`, then orthonormalize (`0085DC80`).
  6. Rebuild the velocity as `axial_speed x forward`.

  Steering is skipped when the target is null, when `HomingAngle <= 0`, or when the body point or
  the lead point lies within 50 m ahead (local z).
- **The turn rate.** `0.05` is the tick element's fixed step written as a literal, so the cap works
  out to **HomingAngle rad/s at time scale 1**. That matches the Hungarian comment in robots.lua
  ("the rocket can turn this many degrees toward the target per second").
- **Timing.** Steering runs only after ignition (`+494h`). Ignition is the class `IgnitionDelay`:
  Tiny Tim 1 s, HVAR and R4M 0 s. Before ignition the rocket falls. After ignition the timer drains
  and **the same gravity is given back**, so a lit rocket regains the vertical speed it lost.
- **The host.**
  - Nothing in the host spawns a rocket, so the lead's "host rockets fly purely ballistic" is
    stronger than the code supports: host rockets do not fly at all.
  - The two homing producers, the rocket task (`kRocketTaskBound = false`) and `009FD0E0`
    (unhosted), are both off.
  - The plane rack path refuses kind `33h`.
  - The bomb-family kernels for ignition and thrust (`bomb_torpedo_tick.cpp`) have no host caller.
  - The shot integrator is cc9's `GameGunneryHost::Impl::run_projectiles` (`src/game_hosts_gunnery.cpp`).
- **Effect of the diff alone: none.** The diff adds a spawn entry point
  (`release_rocket_0080aa20`) and the step. It does not move a single number until a caller
  releases rockets. The natural caller is the rocket task's `release_rocket` (CC13_ROCKET_TASK
  section 7).

## 1. Where 0080A000 sits

- `0080A000` is MRocket's vtable `+19Ch` slot. A byte scan finds exactly one absolute reference, at
  `00D09284`, which is table `00D090E8` + `19Ch`. It has no direct callers.
- The tick element `006E1300` calls it as `record->vtable[+19Ch](dt)` when the motion mode is 0
  (in air), with `dt = classDesc+5Ch x step` and the fixed step `0.05f` (docs/BOMB_FAMILY_TICK.md
  step 5).
- It replaces the air advance `006E1510`, so a rocket's only gravity is the bookkeeping in section 2.
- ABI: `__thiscall(record; float dt)`, `RET 4`. The frame is `SUB ESP,D4h` plus `PUSH ESI/EDI`. All
  `[ESP+n]` offsets below are relative to that base, called S.

Old Ghidra names (recorded by `ghidra_annotate.py` in the worker's `local/ghidra-annotations-*.json`):
`FUN_0080a000` became `BSP_Rocket_AirStep`, and `FUN_0042ae80` became
`BSP_Vector3f_DivideByScalarInPlace`. Neither had a comment before the annotation.

## 2. Ignition and gravity (0080A024..0080A148)

`T = +490h`, `E0 = class IgnitionDelay`, `g = [00CF9058]` double `9.8100004196167`.

| path | pre `[S+24h]` | credit `[S+18h]` |
| --- | --- | --- |
| lit (`+494h` set) | 0 | `T > dt ? dt : T` (`0080A04C` FCOMIP, JBE) |
| not lit | `rem = float(E0 - T)`; `rem > dt ? dt : rem` (`0080A091` JA) | `post = float(dt - pre)`; `T > post ? post : T` (`0080A0BD`) |

- **Ignition** (`0080A0D0..0080A0FE`): the motor lights when `credit != 0` (unordered counts as
  non-zero, `TEST AH,44h; JP`) or when `pre == 0`. It then sets `+494h = 1` and calls
  `record+310h->vtable[3Ch]()`, the notice, which is not reconstructed.
- **Timer and gravity:**
  - `T = float(float(pre + T) - credit)`;
  - `v.y = float(v.y - g·pre)`;
  - `v.y = float(v.y - g·(-credit))`.

  The `FMUL ST3` at `0080A130` is the `DC C8+i` form (`ST3 *= ST0`), which keeps `g` on the stack for
  the `FCHS; FMULP` at `0080A13E`.
- **The give-back.** Before ignition the timer accumulates the time spent falling. After ignition
  it drains at `dt` per step and adds `g·dt` back to `v.y`.
- The existing `bomb_torpedo_tick.cpp` kernels (`rocket_pre_ignition_slice`,
  `rocket_ignites_this_step`) agree on the test. The give-back is not in them.

## 3. Thrust (0080A15B..0080A250)

- `axial = float(f.y·v.y + f.x·v.x + f.z·v.z)` (`0080A179..0080A198`). `f` is the **world** forward
  row `+ECh..+F4h` (`+CCh + 20h`).
- `[S+0Ch] = VMax` (class `+D8h`). This slot survives into the homing branch.
- `VMax > axial`:
  - `new = float(accel·dt + axial)`, then `min(new, VMax)`;
  - `d = float(new - axial)`;
  - `v += float(d·f)` per component, each a float store;
  - `[S+24h] = new`.
- Otherwise `[S+24h]` keeps `axial`.

## 4. Homing (0080A25C..0080A559)

Operand widths are given in brackets. "ext" is an x87 intermediate.

1. `0080A25C`: `+47Ch` null, or `+48Ch <= [00D7A218] 0.0f` (COMISS, JBE), exits.
2. `0080A28F` (`004142E0`): `W = +480h` through the target's world `+CCh`, after `00414DB0` refreshes
   the target. `0080A2DA` (`004142E0`): `L = W` through the rocket's `+110h`, the derived affine
   inverse of `+CCh` (`00414E10 -> 00B63D50`).
3. `0080A2DF`: `L.z [float] > [00CE3938] 50.0 [double]` (FCOMI, JBE to `0080A55B`), else exit.
4. `0080A317`: `t = 00809E10(L.z)` on the rocket itself (`ECX = ESI`), stored `[float]`. `00809E10`'s
   `v0` is `[+3Ch]->vt[38h]` or `|+318h|` (`0042B2F0`). For a released rocket `+3Ch` is read as null
   (provisional, as in CC13_ROCKET_TASK section 9). `+318h` already includes this step's thrust.
5. `0080A326`: `vel = target->vtable[34h]()`, the linear velocity. The fuze loop further down subtracts the
   same slot's result from the rocket's `+318h`.
   - `P.x = float(float(vel.x·t) + W.x)` [ext product, float store, float add, float store];
   - likewise for y and z.
6. `0080A385` (`00414D10`): `L2 = P` through `+110h`. `0080A39B`: `L2.z > 50.0`, else exit.
7. `0080A3C2`: `m = 00415550(&L2.z, &VMax)`, which returns `VMax < L2.z ? L2.z : VMax`, in ST0.
   `s = float(m x [00D7A280] 0.5 [double])`. `0080A3DD` (`0042AE80`): `L2 /= s`, three float stores.
8. `0080A3E2`:
   - `cap = float([00D0DE84] 0.05f x +48Ch)`;
   - `lo = [00D7A208] -0.0f - cap` (SUBSS);
   - `00415690` (low bound first, ECX value, EDX `&lo`, stack `&cap`) clamps `L2.x`, then `L2.y`.
9. `0080A43C..0080A4FD`, with rows `+74h` right, `+84h` up, `+94h` forward:
   - `aim = forward`;
   - `aim += float(x'·right)` per component;
   - `aim += float(y'·up)`.
   
   The forward weight is the literal 1: `L2.z / s` (2 when `z >= VMax`) is computed and never read.
10. `0080A501` (`0042D5E0`): forward row (`matrix+20h`) = aim, then `0085DC80` orthonormalizes.
    `0042ED50` invalidates the subtree pose.
11. `0080A50D..0080A553`: `+318h = 0042D0D0((0, 0, [S+24h]), world, normalize 0)`. That is the
    new forward times the axial speed from section 3. The gravity part of `v.y` is discarded.

**What this means.**
- Beyond VMax metres (HVAR 500, R4M 800, Tiny Tim 250), the lateral aim is `2x/z`: the rocket
  points at twice the bearing tangent of the lead point, which over-corrects. Closer in, the
  divisor stays at `0.5·VMax`, so the response weakens with range.
- The cap is per step and does not scale with `dt` or with the class time scale.
- Loss of lock is the observer callback `004C2D90` (block vtable `00CE75CC` slot 1). When the
  registered subject notifies, it nulls `+47Ch` and unregisters. The rocket then flies on its last
  heading with thrust. Nothing else clears the target: a rocket that passes the target (z <= 50)
  stops steering for that step only.

## 5. Who uses homing rockets in this installation

**Producers of the block** (byte scan of `.text` rel32 and every section's absolute dwords, with the
pattern known to occur because the same scan finds `0080A000`'s vtable entry):

| caller | context | angle source |
| --- | --- | --- |
| `007B56F6` | rocket task aim tick `007B5430` | robot row `+114h` `StrikeHomingAngle` |
| `009FD4FB` | `009FD0E0`, the pilot bot's opportunistic rocket sub-object | robot row `+174h` / `+1A8h` / `+1DCh` / `+210h` (`Rocket_SmallPlane` / `LargePlane` / `Ship` / `Landfort`, chosen by the target: plane small or large by `0047B880`, ship kind 6, land kinds `1Bh`/`45h`/`46h`) |

`009FD0E0` is called from `00997373` and `0099998C` (`BSP_PilotBot_Update`). It also fires at
**planes**. The rocket task never does (CC13_ROCKET_KAMIKAZE_SCOPE).

**The angles**, from `scripts/datatables/robots.lua` (mtime 2025-06-01 16:03, modified relative to
the 2024-07-13 bulk; `Robots["PilotBot"]` at line 552). They are in radians via `DEG()`:

| PilotBot row (lines) | StrikeHomingAngle | Rocket_SmallPlane | LargePlane | Ship | Landfort |
| --- | --- | --- | --- | --- | --- |
| SPNormal (554-692) | 0.75 (660) | 0.6 (620) | 0.5 (630) | 0.2 (640) | 0.2 (650) |
| SPVeteran (693-830) | 1.5 (756) | 2.0 (787) | 2.0 (797) | 2.0 (807) | 2.0 (817) |
| MPNormal (831-968) | 1 (894) | 0.5 (925) | 1.0 (935) | 1.5 (945) | 1.5 (955) |
| MPVeteran (969-1106) | 1.5 (1032) | 2.0 (1074) | 2.0 (1084) | 1.75 (1094) | 1.75 (1104) |
| Elite (1107-1244) | 1.5 (1170) | 2.0 (1201) | 2.0 (1211) | 2.0 (1221) | 2.0 (1231) |
| Stun (1245-) | **0.0** (1350) | 0.2 (1310) | 0.2 (1320) | 0.2 (1330) | 0.2 (1340) |

Every PilotBot row except Stun's strike gives a positive angle, so **every plane rocket fired by the
AI homes**. The rate is slow. An SPNormal rocket-task Tiny Tim turns at most 0.75 deg/s. After its
1 s ignition and roughly 4 s of burn and coast over 1 km, that is about 3 degrees of correction.

**The rocket classes** (`classtables/arcade/bulletclasses.lua`, mtime 2026-05-09, modified;
`realistic` 2025-06-02):

| class | arcade line | IgnitionDelay | VMax | Acceleration |
| --- | --- | --- | --- | --- |
| Tiny Tim | 200-210 | 1 | 250 | 50 |
| 5" HVAR | 271-281 | 0 | 500 | 500 |
| R4M AA | 374-384 | 0 | 800 | 1000 |
| 5" HVAR (51) | 1497-1508 | 0 | 500 | 500 |
| R4M AA (116, arcade only) | 3001-3011 | 0 | 800 | 1000 |

Row 5, `4.7'' AA rocket`, is `Type "Flak"` with `--["Type"] = "Rocket"` commented out in **both**
tables (arcade line 167, realistic line 163). That is a mod edit, so it is not a rocket here.
CC13_ROCKET_KAMIKAZE_SCOPE labels arcade bullet 116 "Tiny Tim" for the A8M, but this installation's
row 116 is named "R4M AA" (arcade line 3004).

**Carriers and missions**: CC13_ROCKET_KAMIKAZE_SCOPE (the census) lists only planes: AvengerTinyTim,
Corsair, Raiden, Kikka, Shinden, A8M and Mustang. **No ship class carries rocket ammunition**, and a
ship-fired rocket could not home anyway (section 0). So the missions that move are the rocket
missions in that census:
- **USN13** (1 Raiden, R4M);
- **USN22** (`usn_ormoc`, 1 AvengerTinyTim);
- **JM08** (Kikka, Shinden);
- **USN19** (`prcp_14_okinawa`, 2 Shinden);
- also IJN10, IJN14, ALT02, IJN17, IJN19, IJN20, IJN23, IJN29, USN07 and USNBR (Corsair and
  Raiden lines, scope table lines 143-163).

They move only once a rocket producer is bound (section 7).

## 6. Host glue (suggested diff, NOT applied)

The block below is the exact `git diff` of the glue as it was built in the worker tree. `./scripts/build.ps1` passed with it applied, and the glue was then reverted. `git apply --check` passes against origin/main `4eb5b854a`. To apply it, copy the block to a file and run `git apply`. It touches only cc9's `include/bsp/game_hosts_gunnery.hpp` and `src/game_hosts_gunnery.cpp`, and needs `src/rocket_homing_cc13.cpp` (landed with this packet).

```diff
diff --git a/include/bsp/game_hosts_gunnery.hpp b/include/bsp/game_hosts_gunnery.hpp
index 23e9ec443..365405bc2 100644
--- a/include/bsp/game_hosts_gunnery.hpp
+++ b/include/bsp/game_hosts_gunnery.hpp
@@ -82,6 +82,7 @@
 #include "bsp/gun_platform_arc.hpp"
 #include "bsp/gunnery_tables.hpp"
 #include "bsp/projectile_impact.hpp"
+#include "bsp/rocket_homing_cc13.hpp"
 #include "bsp/unit_gunnery_pass.hpp"
 
 namespace bsp {
@@ -145,6 +146,10 @@ struct GameBulletClassRow {
     // "HeadingTurn", classDesc+0E8h, degrees per second: 00857061's turn rate
     // toward the commanded heading. Packet cc9_usn02_sameside_torpedoes.
     float heading_turn{0.0f};
+    // Packet cc13-homing1: the MRocket class fields 0080A000 reads.
+    float ignition_delay{0.0f};       // "IgnitionDelay", classDesc+E0h
+    float rocket_vmax{0.0f};          // "VMax", classDesc+D8h
+    float rocket_acceleration{0.0f};  // "Acceleration", classDesc+DCh
 };
 
 // One authored `DeviceClass` row, as the gun needs it.
@@ -555,6 +560,11 @@ struct GameProjectileRow {
     // keeps neither flag nor point on the round.
     bool is_bomb{false};
     float predicted_impact[3]{};
+    // Packet cc13-homing1: a rocket record's motion state (0080A000) and its
+    // homing block (0080AA20). docs/CC13_ROCKET_HOMING.md.
+    bool is_rocket{false};
+    bsp::RocketFlight rocket{};
+    bsp::RocketClassParams rocket_class{};
     // Packet cc8_dive_aim item 2. The aimdive summary's `tf=` is LAST-SAMPLED -
     // it is read after the aircraft has left the dive, so it is not the fall
     // time that was in force when the round left. These four are sampled AT THE
@@ -673,6 +683,12 @@ struct GameGunneryUnitRow {
 };
 
 struct GameGunnerySummary {
+    // Packet cc13-homing1 (kRocketHomingBound): rocket launches and 0080A000.
+    unsigned long long rocket_launches{0};
+    unsigned long long rocket_guided_launches{0};
+    unsigned long long rocket_ignitions{0};
+    unsigned long long rocket_homing_steps{0};
+    unsigned long long rocket_homing_lock_lost{0};
     std::size_t units_with_guns{0};
     std::size_t guns{0};
     std::size_t passes_attached{0};
@@ -998,6 +1014,14 @@ public:
                            float release_fall_time, int skill_row = -1);
     // True when 006E4D50's scatter draws are taken (kBombDropScatterBound).
     static bool bomb_drop_scatter_bound() noexcept;
+    // Packet cc13-homing1 (kRocketHomingBound, OFF): spawn one rocket from the
+    // unit's Rocket-type gun row and fill its homing block as 0080AA20 does.
+    // `target_plus_one` 0 or a target failing IsKindOf(5) leaves it unguided.
+    // SUBSTITUTION, labelled: the launch geometry (the rack's mount and the
+    // platform release vtable 00CFE308 are unread): the plane's origin and
+    // rows, at the plane's forward speed, like release_ordnance_drop.
+    bool release_rocket_0080aa20(std::size_t unit_index, std::size_t target_plus_one,
+                                 const float body_point[3], float homing_angle);
     const std::vector<GameGunneryUnitRow>& unit_rows() const noexcept;
     // Packet cc9_water_surface_law. The gunnery host owns every death, so the
     // units host asks it two things for an aircraft in the water:
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index c7844a874..36b07fb1a 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -4410,6 +4410,14 @@ void GameGunneryHost::Impl::build_guns(std::size_t first_unit) {
                     gun.bullet_class, "MaxFall", 0.0f);
                 b.heading_turn = lua.read_bullet_class_number(
                     gun.bullet_class, "HeadingTurn", 0.0f);
+                if constexpr (bsp::kRocketHomingBound) {
+                    // Packet cc13-homing1: classDesc+E0h/+D8h/+DCh for 0080A000.
+                    b.ignition_delay = lua.read_bullet_class_number(
+                        gun.bullet_class, "IgnitionDelay", 0.0f);
+                    b.rocket_vmax = lua.read_bullet_class_number(gun.bullet_class, "VMax", 0.0f);
+                    b.rocket_acceleration = lua.read_bullet_class_number(
+                        gun.bullet_class, "Acceleration", 0.0f);
+                }
                 b.damage_min = flat_scaled(type_id, make("dmin"), kMilliScale, 0.0f);
                 b.damage_max = flat_scaled(type_id, make("dmax"), kMilliScale, 0.0f);
                 b.water_damage = flat_scaled(type_id, make("wdmg"), kMilliScale, 0.0f);
@@ -8726,6 +8734,42 @@ void GameGunneryHost::Impl::run_projectiles(float dt) {
                 }
             }
         }
+        if (bsp::kRocketHomingBound && shot.is_rocket) {
+            // Packet cc13-homing1: 0080A000, MRocket's vtable+19Ch, replaces the air
+            // advance (gravity is its own; the round is flagged NoGravity at spawn).
+            bsp::RocketHomingTargetView view;
+            const bsp::RocketHomingTargetView* target = nullptr;
+            const std::size_t t1 = shot.rocket.homing.target_plus_one;
+            if (t1 != 0 && t1 - 1 < unit_state.size()
+                && (unit_state[t1 - 1].dead || unit_state[t1 - 1].health <= 0.0f)) {
+                // 004C2D90: the observer callback nulls +47Ch (loss of lock).
+                // LABELLED: the host stands death for the notice.
+                shot.rocket.homing.target_plus_one = 0;
+                ++summary.rocket_homing_lock_lost;
+            } else if (t1 != 0) {
+                float r[3], u[3], f[3], o[3];
+                unit_pose(t1 - 1, r, u, f, o);
+                view.world = {r[0], r[1], r[2], 0.0f, u[0], u[1], u[2], 0.0f,
+                              f[0], f[1], f[2], 0.0f, o[0], o[1], o[2], 1.0f};
+                // vtable[34h]: the linear velocity. 0092D730 as the drops use,
+                // because GameUnitRow::forward_speed is zero for a plane.
+                const float speed = units.unit_forward_speed_0092d730(t1 - 1);
+                view.velocity = {f[0] * speed, f[1] * speed, f[2] * speed};
+                target = &view;
+            }
+            shot.rocket.velocity = {shot.flight.velocity.x, shot.flight.velocity.y,
+                                    shot.flight.velocity.z};
+            shot.rocket.pose[12] = shot.position[0];
+            shot.rocket.pose[13] = shot.position[1];
+            shot.rocket.pose[14] = shot.position[2];
+            const bsp::RocketAirStepResult step =
+                bsp::rocket_air_step_0080a000(shot.rocket, shot.rocket_class, target, dt);
+            shot.flight.velocity = bsp::TickPoint3{shot.rocket.velocity[0],
+                shot.rocket.velocity[1], shot.rocket.velocity[2]};
+            if (step.ignited_this_step) ++summary.rocket_ignitions;
+            if (step.homing == bsp::RocketHomingExit::kSteered) ++summary.rocket_homing_steps;
+            done("Rocket::air_step_0080a000", 0x0080a000u);
+        }
         const float from[3] = {shot.position[0], shot.position[1], shot.position[2]};
         shot.flight = bsp::projectile_flight_step(shot.flight, dt);
         if (kBombDropDriftBound && shot.is_bomb && !shot.flight.class_disables_gravity
@@ -12589,6 +12633,54 @@ const GameGunnerySummary& GameGunneryHost::summary() const noexcept {
     return impl_->summary;
 }
 
+bool GameGunneryHost::release_rocket_0080aa20(std::size_t unit_index,
+                                              std::size_t target_plus_one,
+                                              const float body_point[3], float homing_angle) {
+    Impl& h = *impl_;
+    if (!bsp::kRocketHomingBound) return false;
+    const GameGunRow* chosen = nullptr;
+    for (const GameGunRow& gun : h.guns) {
+        if (gun.unit_index != unit_index) continue;
+        const GameBulletClassRow* round = h.bullet(gun.bullet_class);
+        if (round == nullptr || round->type != "Rocket") continue;
+        chosen = &gun;
+        break;
+    }
+    if (chosen == nullptr) return false;
+    const GameBulletClassRow* round = h.bullet(chosen->bullet_class);
+    float right[3], up[3], forward[3], origin[3];
+    h.unit_pose(unit_index, right, up, forward, origin);
+    const float speed = h.units.unit_forward_speed_0092d730(unit_index);
+
+    GameProjectileRow shot;
+    shot.gun_row = static_cast<std::size_t>(chosen - h.guns.data());
+    shot.owner_unit = unit_index + 1;
+    shot.owner_side = h.units.unit_side_0054(unit_index);
+    shot.bullet_class = chosen->bullet_class;
+    shot.alive = true;
+    for (int i = 0; i < 3; ++i) shot.position[i] = origin[i];
+    shot.flight.velocity = bsp::TickPoint3{forward[0] * speed, forward[1] * speed,
+                                           forward[2] * speed};
+    shot.flight.snapshot_current = bsp::TickPoint3{origin[0], origin[1], origin[2]};
+    shot.flight.local_position = shot.flight.snapshot_current;
+    shot.flight.mode = bsp::ProjectileMotionMode::kBallistic;
+    shot.flight.class_disables_gravity = true;   // 0080A000 owns the rocket's gravity
+    shot.is_rocket = true;
+    shot.rocket_class = {round->ignition_delay, round->rocket_vmax, round->rocket_acceleration};
+    shot.rocket.pose = {right[0], right[1], right[2], 0.0f, up[0], up[1], up[2], 0.0f,
+                        forward[0], forward[1], forward[2], 0.0f,
+                        origin[0], origin[1], origin[2], 1.0f};
+    const bool kind_5 = target_plus_one != 0
+        && h.units.unit_is_kind_of(target_plus_one - 1, 0x05);
+    bsp::rocket_set_homing_target_0080aa20(shot.rocket.homing, target_plus_one, kind_5,
+        {body_point[0], body_point[1], body_point[2]}, homing_angle);
+    h.shots.push_back(shot);
+    ++h.summary.projectiles;
+    ++h.summary.rocket_launches;
+    if (shot.rocket.homing.target_plus_one != 0) ++h.summary.rocket_guided_launches;
+    return true;
+}
+
 bool GameGunneryHost::release_ordnance_drop(std::size_t unit_index) {
     Impl& h = *impl_;
     // The unit's torpedo-capable gun rows. `swim_speed > 0` is the same test the
```

What it does, all behind `bsp::kRocketHomingBound`:
- **The bullet row** gains `IgnitionDelay`, `VMax` and `Acceleration`, read only when the switch is on.
- **`GameProjectileRow`** gains `is_rocket`, `bsp::RocketFlight` and `bsp::RocketClassParams`.
- **`release_rocket_0080aa20(unit, target+1, body_point, angle)`** spawns one rocket from the unit's
  `Type "Rocket"` gun row and fills the homing block as `0080AA20` does, including the
  `IsKindOf(5)` test.
  - **SUBSTITUTION, labelled:** the launch geometry is the plane's origin and rows at
    `0092D730` speed, exactly as `release_ordnance_drop`.
  - The rocket is flagged `class_disables_gravity`, because `0080A000` owns its gravity.
- **`run_projectiles`**, before `projectile_flight_step`, calls `rocket_air_step_0080a000`:
  - the target's world matrix comes from `unit_pose`;
  - the target's velocity is its forward row times `0092D730`;
  - a dead target nulls the block. **LABELLED:** death stands for the observer notice `004C2D90`.
- **Summary counters:** `rocket_launches`, `rocket_guided_launches`, `rocket_ignitions`,
  `rocket_homing_steps` and `rocket_homing_lock_lost`.

**LABELLED substitutions inside the step:**
- The rocket's pose is both `+74h` and `+CCh` (no parent).
- `+110h` is recomputed each step with `00B63D50`.
- `00809E10`'s `v0` is `|velocity|`.
- Between steps the pose changes only through homing. Whether the shot interface realigns the
  rocket with its velocity is unread.

**RNG:** the step draws nothing. The spawn draws nothing either: the rack's `Throw` scatter is not
modelled.

## 7. Expected effect and order of landing

- **The diff alone:** no change. `rocket_launches = 0` on every mission, because nothing calls
  `release_rocket_0080aa20`.
- **With the rocket task bound** (`kRocketTaskBound`, CC13_ROCKET_TASK section 7):
  - its adapter's `release_rocket(const RocketRelease& r)` should call
    `release_rocket_0080aa20(unit, unit_.rk_target_plus_one, r.homing_point, r.homing_angle)` on the
    gunnery host before `release_ordnance_007bbba0`. `RocketRelease` carries no target: the block's
    target is the task's `approach+70h`, which is the task's own target ref;
  - rockets then fly ignite-then-thrust and steer at the robot's StrikeHomingAngle;
  - hits on manoeuvring ships rise slightly;
  - the Tiny Tim's 1 s drop is recovered after ignition (the give-back).
- **The opportunistic path `009FD0E0`** (planes firing at planes and ships with the
  `Rocket_*` rows) is a separate, unhosted packet. Without it, R4M air-to-air fire stays absent.
- **Proximity fuze (`0080A55D`..):**
  - it applies with class `+E4h` or the global `[00F874FD]`;
  - the fuze is not reconstructed, so a host R4M (an air-to-air rocket) detonates only on contact
    through the gunnery impact path;
  - **next packet** if air-to-air rockets matter.

## 8. Uncertainty and open items

- **x87:** double intermediates with float stores at each `FSTP dword` (section 4 widths). The main
  thread runs at 24 bits (docs/CC13_FPU_PRECISION.md), so the last bit can differ.
- **`+3Ch` of a flying rocket** (00809E10's `v0` source): unread.
- **Rocket orientation between steps:** if something other than homing writes `+74h` (for example
  the shot interface aligning to velocity), the host's frame for the lateral terms differs. It is
  unread.
- **The ignition notice** `record+310h->vt[3Ch]` and the fuze are not reconstructed.
- **`009FD0E0`:**
  - read here only to its `0080AA20` call;
  - its gates are `RocketChance`, `AttackDist`, `AttackTime`, `AttackAngle`, `NumRockets` and
    `RocketDelay`;
  - its aim point is `param+18h`.
- **Verification:** build only. No fixture, no game run.

## 9. Ledger and Ghidra

- Names:
  - `0080A000` `BSP_Rocket_AirStep` (was `FUN_0080a000`);
  - `0042AE80` `BSP_Vector3f_DivideByScalarInPlace` (was `FUN_0042ae80`).
- Function records:
  - `rocket_air_step_0080a000` (partial: `0080A000-0080A559`);
  - `vector3_divide_in_place_0042ae80`;
  - `rocket_set_homing_target_0080aa20`.
- The evidence comments were applied with `tools/ghidra_annotate.py --apply`, and the project was saved.
