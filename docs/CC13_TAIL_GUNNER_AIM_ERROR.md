# Tail-gunner aim error: a ready-to-apply diff for cc9 (packet cc13-tailgun1)

Worker cc13-tailgun1, 2026-10-12, Claude lineage cc13. **Read-only on cc9's files.** This document
holds one unified diff against `src/game_hosts_gunnery.cpp`, based on `origin/main` `c775c1eaf`.
It was applied in this worker's tree, built with `./scripts/build.ps1`, reverted, and checked again
with `git apply --check` (see "Verification"). Both new switches default **OFF**, and an OFF build
behaves like the base: no draw is added, no log line is added, and nothing is read or written.

Status: **exported** (the listing was read from disk), **reconstructed** (the diff),
**build-tested** (both switches OFF; see "Verification" for the ON compile). It is **not
game-validated**: no pair has been run. cc9 runs the pairs.

Source: `docs/CC13_RNG_STREAM_CENSUS.md` section 11.2, the first row (`008FFD37` with
`008FFCD9`/`008FFD05`, class NM).

## 0. Short answer

- In the image, a category-1 gun mounted on a plane (a bomber's tail or turret gunner) runs
  TailGunnerBot `008FFA20`. Each gun holds one angle pair for an *aim period*. When the period
  ends, the gun re-solves the pair and adds `U(0, AngleError)` degrees to **each** axis. The
  draws are at `008FFCD9` and `008FFD05`, both stream 1, and the error is **never negative**.
  The gun then draws the next period `U(AimPeriodMin, AimPeriodMax)` at `008FFD37`. Between
  re-solves it keeps commanding the held pair, relative to its own plane.
- The countdown is forced to `-1.0` whenever the gunnery pass hands the gun a target
  (`008FEA40`), and that pass runs on every gunnery think. So a held pair lasts at most one
  period and never outlives a think.
- The image's tail gunner aims at the target's **origin**, with **no lead**. The aim point is
  `target->vtable[100h]`, and for a plane that is `0042D810` (three zeroes). The tick also has
  no negative-vertical halving.
- The host does none of this. It solves the AA gunner's 00901C20 intercept lead every tick,
  halves a negative vertical, and adds no error (`src/game_hosts_gunnery.cpp:6896..6897` limits the
  AA gunner swing to ship-owned guns). So host tail gunners are much more accurate than the
  image's.
- The diff adds two switches, so that cc9 can pair each one separately:
  - `kTailGunnerAimErrorBound`: the error, the period, the hold and the target-set reset.
  - `kTailGunnerOwnAimBound`: aim at the target's origin with no lead, and skip the halving.
- The library function `gun_bot_turret_tick_008ffa20` (`src/gun_bot_ticks.cpp:342`) is **not
  called**. It needs a 25-virtual `GunBotTickHost` adapter and has two fidelity gaps (section 3).
  The diff reuses two of its leaf helpers instead.

## 1. What 008FFA20 does (listing `008FFA20..008FFF1F`, disk bytes)

Bot fields are the ones `008FEAC0`'s property names give (`include/bsp/gun_bot_ticks.hpp:100`).
The skill setter `008FBEF0` copies them from the descriptor `[00E199A0] + skill * 24h`:
`+84h` = AngleError (row `+0Ch`), `+88h` = AimPeriodMin (`+10h`), `+8Ch` = AimPeriodMax
(`+14h`), `+90h` = ShootRange (`+18h`) (`008FBF07..008FBF43`).

| step | sites | what |
| --- | --- | --- |
| 1 | `008FFA2D..008FFA75` | the shared validity drop, which calls `vtable[38h](0)` on failure |
| 2 | `008FFA85` | idle timer `008FBCE0(dt, gun)` |
| 3 | `008FFA8D..008FFAA0` | side gate: `[gun+1ACh] == 8` or `00927F10` |
| 4 | `008FFAA6..008FFAE7` | drop the target when the director's `+221h` (aaEnabled) is clear |
| 5 | `008FFAE9..008FFAFE` | return without a gun (`+68h`) or a target (`vtable[44h]`). **The countdown does not run without a target.** |
| 6 | `008FFB35..008FFB73` | `+6Ch -= dt`, stored. Re-solve only when `0 > +6Ch` (`FLDZ / FCOMIP / JBE`) |
| 7 | `008FFB79..008FFC0E` | re-solve: `target->vtable[100h](out, box (0.6,0.6,0.6) [00CE3D30], [00F87574], SectionTargetChance, EngineRoomWeight, MagazineWeight, FueltankWeight)` (descriptor `+20h..+2Ch`) into `+78h` |
| 8 | `008FFC1C..008FFC68` | `00414D10(+78h, target pose +0CCh)`: the local point moved into the world |
| 9 | `008FFC71..008FFCBD` | muzzle `[gun+3CCh]+120h`; `00414E10` on `[gun+3F0h]` (the owner's frame) and `008FDAF0` write the pair `+70h`/`+74h` |
| 10 | `008FFCC2..008FFCF4` | `+70h += U(0, +84h) * [00CE3D28] / [00CE3D20]`, with `ECX = 1` at `008FFCCF` and the call at **`008FFCD9`** |
| 11 | `008FFCF6..008FFD21` | `+74h += U(0, +84h) * pi / 180`, with `ECX = 1` and the call at **`008FFD05`** |
| 12 | `008FFD24..008FFD3C` | `+6Ch = U(+88h, +8Ch)`, with `ECX = 1` and the call at **`008FFD37`** |
| 13 | `008FFD3F..008FFD52` | `0085ABA0(gun, +70h, +74h)` on **every** tick, so the held pair is commanded between re-solves |
| 14 | `008FFD57..008FFDB9` | distance = `|world aim point - muzzle|` (`0042B2F0`) |
| 15 | `008FFDBE..008FFF03` | the range and angle-sum hysteresis against gun `+480h`/`+484h` and the held pair |
| 16 | `008FFF12` | `008FEF40(request, dt)` debounce |

Details behind the table:

- **Draw arguments.** `FLD [ESI+84h]` goes to `[ESP+4]` (high) and `FLDZ` to `[ESP]` (low),
  so each draw is `U(0, AngleError)`. The robots.lua comment says the same: "AngleError :
  celzasi hiba fokban (random 0..AngleError)".
- **Constants.** `[00CE3D28]` is the double `400921FB60000000h`, which is float pi widened.
  `[00CE3D20]` is `180.0`. These are `kGunBotDegreesToRadians{Numerator,Denominator}`.
- **Draw order.** Horizontal first, vertical second, period third. The period comes after
  both offsets.
- **When the draws happen.** Only when the countdown is below zero after `-= dt`, and only
  while the gun has a target and passes the side gate. So the draws happen:
  - **once per period** (between `AimPeriodMin` and `AimPeriodMax` seconds), and
  - **on the first aiming tick after any target set**, because `008FEA40` (the class's
    override of primary slot `+38h`, `008FEA40..008FEA5B`) calls the base `006DF170` and then
    stores `[00D7A260] = -1.0f` into `+6Ch` unconditionally, even when the target is the same.
    `00727F10` calls that slot for `+390h` on every gun it hands a target (`00727F13..00727F2D`).
    Its only caller is the gunnery pass `00864FE0` at `00865833`, whose body runs once per
    `GlobalConfig+88h` (2 s in this host, `docs/AA_TARGETING.md`).
- **The aim point has no lead.** `0042D810` serves every plane vtable (`00D05F20`, `00D06638`,
  `00D06920`) and writes `(0,0,0)` (`docs/GUN_BOT_REMAINDER.md` section 3). Step 8 therefore
  yields the target's pose origin. Nothing in `008FFA20` calls `00901C20`. A ship target gets
  the `00816650` hull-box sample instead.
- **No halving.** The negative-vertical halving `00902F76` belongs to `00902920`. The listing of
  `008FFA20` has no `FMUL` on the pair.
- **The initial countdown.** `008FBEF0` draws `+6Ch = U(+88h, +8Ch)` at `008FBF5C`, also stream
  1, when the skill is set. Because a target can only arrive through `008FEA40`, this value is
  always overwritten with `-1` before the first aiming tick. It only moves the stream position.

### Precision

`gun_bot_apply_aim_error_008ffa20` does the arithmetic in `float`: `angle + draw * pi_f / 180`.
That matches the image exactly at 24-bit x87 precision, where each operation rounds to 24 bits.
At 53 bits the image keeps the product and the quotient in double and rounds once at the
`FSTP`, so the two can differ by at most 1 ulp of the stored float. `docs/CC13_FPU_PRECISION.md`
shows that unit-tick math runs at 24 or 53 bits depending on the thread. Which thread runs
the bot tick was not established.

## 2. What the host does now (base `c775c1eaf`)

- **Aim point.** A category-1 gun counts as an "AA bot aim" (`aa_bot_aims`, `:1995`). A
  tail gunner on a plane therefore takes the AAGunnerBot path, which uses the 00901C20
  intercept lead against the target's world velocity (`:6714..6784`).
- **Swing error.** The 00902920 swing is restricted with `!unit_is_kind_of(owner_unit,
  kUnitGunneryKindPlaneBase)` (`:6896..6897`), so a plane's gun gets **no error**.
- **Halving.** `kAaGunnerErrorBound` halves a negative vertical for every category-1 gun,
  plane-mounted guns included (`:6994`). That is an 00902920 step applied to the 008FFA20
  class.
- **Fire test.** Only the fire hysteresis is hosted (`:7313..7334`). It compares the gun's
  angles with the pair solved fresh every tick.
- **The library tick is unused.** `gun_bot_turret_tick_008ffa20` has no caller.

## 3. The library reconstruction `gun_bot_turret_tick_008ffa20`

Checked against the listing. The control flow, the draw order, the bounds, the
degrees-to-radians step, the period draw and the hysteresis all match. Three things keep it
from being callable as it stands:

1. **No host adapter.** It runs through `GunBotTickHost`, which has about 25 pure virtuals
   (`include/bsp/gun_bot_ticks.hpp:389..433`). Binding it inside cc9's gunnery host would mean
   writing a whole adapter, which is not a small, well-anchored hunk.
2. **The aim point is wrong for ships.** It passes `0.0f` for the four vtable[100h] scalars,
   discards the result, and aims at the re-sampled target origin (`src/gun_bot_ticks.cpp:364..368`).
   The scalars are now known: they are SectionTargetChance, EngineRoomWeight, MagazineWeight
   and FueltankWeight (`docs/GUN_BOT_TICKS.md:523`). For a plane target the result is the same,
   because `0042D810` returns zero. For a ship target it is wrong.
3. **The target-set reset is missing.** The `-1.0` store lives in `008FEA40`, outside the tick,
   and the library has no hook for it.

There is also a comment slip for the library's owner. `gun_bot_apply_aim_error_008ffa20`
(`src/bot_fire_target.cpp:216`) cites "008FFE9x", but the sites are `008FFCDE..008FFCF4` and
`008FFD0A..008FFD21`.

The diff therefore binds the behaviour inline at the existing aim site and reuses the two
helpers that are faithful as written:

- `gun_bot_turret_recompute_due_008ffa20` (the `-= dt` and the `< 0` test);
- `gun_bot_apply_aim_error_008ffa20`.

## 4. The diff

What the diff changes:

| hunk | where | what |
| --- | --- | --- |
| 1 | after `kTailGunnerShootRange` (`:308`) | the two switches (`false`), with comments in the file's `k...Bound` idiom, and `kTailGunnerAimRows[6][3]` from this installation's robots.lua (2025-06-01) |
| 2 | `enum class Draw` | `tail_gunner_error = 26`, keyed `(gun, 0)` like `aa_gunner_error` |
| 3 | next to `tail_gunner_fire` | per-gun `TailGunnerAim {countdown, horz, vert}` and two counters |
| 4 | `set_bot_fire_target_00727f10` | `countdown = -1.0f` for a category-1 gun (`008FEA40`) |
| 5 | end of the AA lead branch, before `pitch = 0.0f` | `kTailGunnerOwnAimBound`: `lead = origin_of_target` for a plane-owned category-1 gun |
| 6 | right after the AA gunner swing block | `kTailGunnerAimErrorBound`: the countdown, re-solve and hold, with draws on H-G through `draw(Draw::tail_gunner_error, g, 0, ...)` in image order |
| 7 | `kAaGunnerErrorBound` halving | skipped for plane-owned guns when `kTailGunnerOwnAimBound` is on |
| 8 | summary block | one `summary mission gunnery tail gunner aim ...` line, printed only when either switch is on (`if constexpr`) |

Skill rows, in the index order the file already uses for `kTailGunnerShootRange` (Stun,
SPNormal, SPVeteran, MPNormal, MPVeteran, Elite), as `{AngleError, AimPeriodMin, AimPeriodMax}`:
`{4, 0.6, 1.2}`, `{3, 0.1, 0.8}`, `{2.5, 0.1, 0.5}`, `{2.5, 0.1, 0.3}`, `{1.5, 0.1, 0.2}`,
`{0, 0, 0.1}`. The ShootRange values in the same blocks are 950/750/800/800/850/950, which
matches `kTailGunnerShootRange` and confirms the order. The Elite row still draws `U(0, 0)`
twice, as the image does.

LABELLED substitutions, which are also stated in the diff's comments:

- **Initial draw.** The `008FBF5C` draw is not made. Its value is always overwritten before the
  first aiming tick, but the stream position moves by one draw per tail-gunner bot in the image.
- **Ship targets.** The `00816650` hull-box draw is not made for a ship target under
  `kTailGunnerOwnAimBound`. The origin is used instead.
- **Hysteresis distance.** The fire hysteresis keeps the host's muzzle-to-`unit_aim_point`
  distance. The image uses the muzzle-to-origin distance. This was not changed, to keep the
  hunks small.
- **Countdown gate.** The host's countdown runs only where its aim block runs (a target and a
  solved, non-zero direction). The image's runs on every tick that passes steps 3 to 5. The
  two agree except on ticks where the host's solve gives up.

Coordination: cc9-gunnery36 is applying other diffs to this file. The hunks are short and
anchored on unique lines. If cc9 has already taken Draw id 26, renumber it.

```diff
diff --git a/src/game_hosts_gunnery.cpp b/src/game_hosts_gunnery.cpp
index c97320364..a025ac034 100644
--- a/src/game_hosts_gunnery.cpp
+++ b/src/game_hosts_gunnery.cpp
@@ -306,6 +306,27 @@ constexpr float kTailGunnerHoldSum = 0.15707963705062866f;      // 00D18388, 9 d
 // robots.lua TailGunnerBot ShootRange by skill index (Stun, SPNormal, SPVeteran,
 // MPNormal, MPVeteran, Elite), read into the bot row +18h (008FCAF8).
 constexpr float kTailGunnerShootRange[6] = {950.0f, 750.0f, 800.0f, 800.0f, 850.0f, 950.0f};
+//  * kTailGunnerAimErrorBound: packet cc13-tailgun1, docs/CC13_TAIL_GUNNER_AIM_ERROR.md.
+//    TailGunnerBot 008FFA20 (a category-1 gun under a plane) holds its angle pair
+//    bot+70h/+74h for an aim period: countdown bot+6Ch -= dt (008FFB35) and only
+//    below zero is the pair re-solved, each axis plus U(0, AngleError) degrees
+//    (008FFCD9 / 008FFD05, stream 1, never negative) and the next period drawn
+//    U(AimPeriodMin, AimPeriodMax) (008FFD37). 008FEA40, the class's SetFireTarget
+//    that 00727F10 calls on every gunnery think, sets the countdown to -1.0
+//    (00D7A260). OFF: the pair is solved every tick with no error.
+//  * kTailGunnerOwnAimBound: the same bot's aim point is target->vtable[100h],
+//    which for a plane is 0042D810 (three zeroes), moved by the target pose
+//    (00414D10 at 008FFC3D): the target origin, with no 00901C20 lead. Nor does
+//    008FFA20 halve a negative vertical (00902F76 is 00902920's). OFF: the AA
+//    gunner's lead and halving.
+constexpr bool kTailGunnerAimErrorBound = false;
+constexpr bool kTailGunnerOwnAimBound = false;
+// This installation's robots.lua (2025-06-01), TailGunnerBot, by skill index:
+// {AngleError (+0Ch), AimPeriodMin (+10h), AimPeriodMax (+14h)}, copied to
+// bot+84h/+88h/+8Ch by 008FBEF0.
+constexpr float kTailGunnerAimRows[6][3] = {
+    {4.0f, 0.6f, 1.2f}, {3.0f, 0.1f, 0.8f}, {2.5f, 0.1f, 0.5f},
+    {2.5f, 0.1f, 0.3f}, {1.5f, 0.1f, 0.2f}, {0.0f, 0.0f, 0.1f}};
 constexpr float kTorpedoAxialDrag474 = 0.5999994277954102f;    // 00D0C5EC
 constexpr float kTorpedoLateralDrag478 = 3.0000007152557373f;  // 00D0C5E8
 // DIAGNOSTIC gate for the per-launch tube line (packet cc9_torpedo_tube_turn).
@@ -1612,6 +1633,7 @@ struct GameGunneryHost::Impl {
         depth_charge_delay = 23, // 008FC3E2, DepthChargeBot's FireDelay, key (gun, 0)
         depth_charge_launch = 24, // 006FDA4F / 006FDAE3 / 006FDB26, the activate's three draws, key (gun, 0..2)
         line_of_fire_ttl = 25, // 0072F76D, the cache record's unread expiry, key (gun, target)
+        tail_gunner_error = 26, // 008FFCD9 / 008FFD05 offsets, 008FFD37 period, key (gun, 0)
         damage_smoke = 21,    // 008227E0's respawn draws, key (unit, slot * 2 [+1 for stream 0])
     };
     unsigned long long next_projectile_serial{0};
@@ -1661,6 +1683,14 @@ struct GameGunneryHost::Impl {
         float delay_5c{0.0f};
     };
     std::map<std::size_t, TailGunnerFire> tail_gunner_fire;
+    // Packet cc13-tailgun1: 008FFA20's +6Ch countdown and +70h/+74h pair per gun.
+    struct TailGunnerAim {
+        float countdown{0.0f};
+        float horz{0.0f}, vert{0.0f};
+    };
+    std::map<std::size_t, TailGunnerAim> tail_gunner_aim;
+    unsigned long long tail_gunner_aim_rolls{0};
+    unsigned long long tail_gunner_aim_resets{0};
     unsigned long long aa_test_gunner{0};
     unsigned long long aa_test_flak{0};
     unsigned long long aa_test_tail{0};
@@ -5626,6 +5656,12 @@ public:
         row.target_name = other < owner_.unit_state.size()
             ? owner_.unit_state[other].row.name : std::string();
         row.target_is_fire_target = is_fire_target;
+        if (kTailGunnerAimErrorBound && row.category == 1) {
+            // 00727F13..00727F2D: +390h's vtable[38h], for TailGunnerBot 008FEA40,
+            // which ends with bot+6Ch = -1.0 (00D7A260) whatever the target.
+            owner_.tail_gunner_aim[slot].countdown = -1.0f;
+            ++owner_.tail_gunner_aim_resets;
+        }
         if (is_fire_target && row.category == bsp::kUnitGunneryTorpedoCategory
             && !state_.torpedo_takes_fire_target_7d) {
             ++state_.byte_7d_fire_target_assigns_at_0;
@@ -6781,6 +6817,14 @@ void GameGunneryHost::Impl::run_gun_aim_and_fire(float dt) {
                         lead[i] = origin_of_target[i] + target_velocity[i] * flight;
                     }
                 }
+                if (kTailGunnerOwnAimBound && gun.category == 1
+                    && units.unit_is_kind_of(owner_unit, bsp::kUnitGunneryKindPlaneBase)) {
+                    // 008FFBC6..008FFC3D: TailGunnerBot takes no 00901C20 lead; it
+                    // aims at target->vtable[100h] moved by the target pose. For a
+                    // plane that is 0042D810's zero offset, the origin. LABELLED: a
+                    // ship target's 00816650 hull-box draw is not made.
+                    for (int i = 0; i < 3; ++i) lead[i] = origin_of_target[i];
+                }
                 pitch = 0.0f;
                 ++aa_direct_aims;
                 record("GunBot::intercept_solution_00901c20", 0x00901c20u);
@@ -6961,6 +7005,23 @@ void GameGunneryHost::Impl::run_gun_aim_and_fire(float dt) {
                     aa_gunner_miss_sum += std::sqrt(static_cast<double>(sw.h) * sw.h
                         + static_cast<double>(sw.v) * sw.v) * distance;
                 }
+                if (kTailGunnerAimErrorBound && gun.category == 1
+                    && units.unit_is_kind_of(owner_unit, bsp::kUnitGunneryKindPlaneBase)) {
+                    // 008FFB35..008FFD3C (see kTailGunnerAimErrorBound): the pair is
+                    // re-solved only when the countdown falls below zero, then held.
+                    TailGunnerAim& ta = tail_gunner_aim[g];
+                    if (bsp::gun_bot_turret_recompute_due_008ffa20(ta.countdown, dt)) {
+                        const float* row = kTailGunnerAimRows[aa_level];
+                        ta.horz = bsp::gun_bot_apply_aim_error_008ffa20(want_horz,
+                            draw(Draw::tail_gunner_error, g, 0, 0.0f, row[0]));   // 008FFCD9
+                        ta.vert = bsp::gun_bot_apply_aim_error_008ffa20(want_vert,
+                            draw(Draw::tail_gunner_error, g, 0, 0.0f, row[0]));   // 008FFD05
+                        ta.countdown = draw(Draw::tail_gunner_error, g, 0, row[1], row[2]); // 008FFD37
+                        ++tail_gunner_aim_rolls;
+                    }
+                    want_horz = ta.horz;
+                    want_vert = ta.vert;
+                }
                 if (kAaFlakAimErrorBound
                     && (gun.category == 5
                         || (gun.category == 6 && dp_air_ammo(g, gun.category, target) != nullptr))) {
@@ -6991,7 +7052,9 @@ void GameGunneryHost::Impl::run_gun_aim_and_fire(float dt) {
                         aa_flak_dist_abs_sum += d;
                     }
                 }
-                if (kAaGunnerErrorBound && gun.category == 1 && want_vert < 0.0f) {
+                if (kAaGunnerErrorBound && gun.category == 1 && want_vert < 0.0f
+                    && !(kTailGunnerOwnAimBound
+                         && units.unit_is_kind_of(owner_unit, bsp::kUnitGunneryKindPlaneBase))) {
                     want_vert *= 0.5f;    // 00902F76 FMUL [00D7A280]
                     ++aa_negative_halvings;
                 }
@@ -13205,6 +13268,13 @@ void GameGunneryHost::report() {
                 "flak=%llu tail=%llu (00902920 / 009030C0 / 008FFA20 + 008FEF40, packet "
                 "cc9_aa_bot_fire_tests)", kAaBotFireTestsBound ? 1 : 0, host.aa_test_gunner,
                 host.aa_test_flak, host.aa_test_tail);
+            if constexpr (kTailGunnerAimErrorBound || kTailGunnerOwnAimBound) {
+                host.log.notef("summary mission gunnery tail gunner aim error_bound=%d "
+                    "own_aim_bound=%d rolls=%llu resets=%llu (008FFA20 / 008FEA40, packet "
+                    "cc13-tailgun1)", kTailGunnerAimErrorBound ? 1 : 0,
+                    kTailGunnerOwnAimBound ? 1 : 0, host.tail_gunner_aim_rolls,
+                    host.tail_gunner_aim_resets);
+            }
             host.log.notef("summary mission gunnery wave order bound=%d pre_steps=%llu "
                 "(wave 1 0085AD80 before wave 2 006DF520, packet cc9_fire_request_timing)",
                 kGunWaveOrderBound ? 1 : 0, host.wave_order_pre_steps);
```

## 5. Expected effect when ON, and where to look

The predictions below come from the arithmetic and the authored rows. No game run has
tested them.

**`kTailGunnerAimErrorBound`.** At SPNormal (index 1) each axis gets `U(0, 3)` degrees,
1.5 degrees on average, always in the positive sense of the pair.

- At 300 m, 1.5 degrees is about 7.9 m per axis. That is close to a fighter's half-span and
  larger than its half-height, so most of a burst passes the target on one side rather than
  scattering around it.
- On top of that, the pair is held, relative to the bomber, for `U(0.1, 0.8)` seconds (0.45 s
  on average). During that time both aircraft move and the turret does not follow.
- Stun, at `U(0, 4)` held `0.6..1.2` s, is worse.
- Elite has no offset and holds for at most 0.1 s, so it barely changes.
- The fire hysteresis compares the gun with the held pair. Guns therefore keep firing at the
  wrong point. The trigger-on fraction should change little, but the hit rate should drop.

**`kTailGunnerOwnAimBound`.** Dropping the lead mostly matters for crossing and head-on
passes. With a 300 m range, a muzzle speed of the order of 800 m/s and about 100 m/s of
crossing speed, the miss is tens of metres. A fighter sitting directly astern (low crossing
speed) is hardly affected.

**Combined.** Expect fewer fighters lost to bombers and dive or torpedo bombers, and
therefore more bombers lost to fighters. Mission totals move through both. Two stream
effects matter for the pairs:

- With the error switch on, the shared H-G generator takes three extra draws per re-solve.
  Every later stream-1 draw shifts, including fire stagger and the AA error rolls (memory
  item "shared RNG stream couples pairs").
- For per-kill attribution, run the measurement option `BSP_GUNNERY_RNG_STREAMS=1` on both
  sides, as with the other AA error pairs. Judge only from the per-entity loss tables.

**Missions to pair.** These are the largest `tail=` counts (tail-gunner fire-test ticks, not
engagements) in recent cc9 logs. Each row gives the count and the log:

| mission | tail= | log |
| --- | --- | --- |
| USNOS | 6,683,716 | `cc9-ships43\local\s43_p8_oswf.log` |
| ESMP08 | 5,959,028 | `cc9-physics1\local\p1c_on_e8w.log` |
| USN12 | 1,735,990 | `cc9-physics1\local\p1c_on_u12w.log` |
| USN13 | 1,097,410 | `cc9-gunnery35\local\g35_dron_usn13l.log` |
| BSM01 | 857,821 | `cc9-lua51\local\l51_A1_b1.log` |
| JM08 | 817,312 | `cc9-gunnery36\local\g36_bron_j8r7.log` |
| USN04 | 569,979 | `cc9-gunnery36\local\g36_bron_u4p1.log` |
| JM13 | 369,458 | `cc9-lua50\local\l50_H1_jm13.log` |
| USNRM01 | 244,402 | `cc9-lua50\local\l50_H1_usnrm01.log` |
| USN01 | 250,071 | `cc9-gunnery36\local\g36_broff_u1.log` |

USN02 and LOMP06 show `tail=0` and should not move. With either switch on, the log gains a
`summary mission gunnery tail gunner aim ...` line with the number of re-solves (`rolls`) and
target-set resets (`resets`), which shows the binding fired.

## 6. Verification

The steps, in this worker's tree, with base `origin/main` `c775c1eaf`:

1. Applied the edits.
2. Ran `./scripts/build.ps1` with both switches at their default (OFF). Result: passed (exit 0, no warning in game_hosts_gunnery.cpp, ctest 2/2), repeated after rebasing the diff onto c775c1eaf.
3. Built once more with both switches temporarily `true`, so that the bodies compile under the
   constants that will be paired. Result: passed (exit 0, ctest 2/2, on base 4e6dba5c4; the hunks did not change in the rebase, only their offsets). The switches were set back to false before the diff was taken.
4. Saved `git diff` (the block above), then ran `git checkout -- src/game_hosts_gunnery.cpp`.
5. Ran `git apply --check` on the copy extracted from this document. Result: clean (exit 0, no offsets) on c775c1eaf.

`tools/verify_report_calls.py` was run on this document: not applicable. The tool reads reports/*.json call tables, and this packet produced none. Every call and site this document cites was read from the disk listing in this packet.

To apply: extract the block between the fences into a file, then run
`git apply --check <file> && git apply <file>`.

## 7. Uncertainty

- **Thread and precision.** Which thread runs the gun-bot tick, and so at what x87 precision,
  was not established. The helper matches the 24-bit result exactly, and the 53-bit result
  to within 1 ulp.
- **Sign of the bias.** The bias is positive in the image's pair convention, the convention
  of `008FDAF0`'s output. The host's `want_horz` already uses that convention
  (`kGunHorzSign`), so the diff adds the draw directly. Whether positive vertical means up
  depends on the same convention, and that was not re-derived here.
- **Think cadence.** The reset cadence follows the host's reconstruction of `00864FE0`, which
  calls `set_bot_fire_target_00727f10`. If that host hook is reached on a cadence other than
  the image's, the reset cadence follows it.
- **BulletThrowMul.** The descriptor row's `+1Ch` BulletThrowMul (0.5 at MPVeteran, 0 at
  Elite) scales the tail gun's bullet throw elsewhere (`00730557`/`00730575`). It is outside
  this packet and was not checked against the host.

## 8. Handoff

The packet is complete. Nothing is left in cc9's files, and the doc is the only commit.
Next steps for cc9:

1. Apply the diff.
2. Pair `kTailGunnerAimErrorBound` alone on two or three missions from section 5, then
   `kTailGunnerOwnAimBound` alone.
3. Decide the defaults, and record the verdict next to the switches as the AA error switches
   do.
