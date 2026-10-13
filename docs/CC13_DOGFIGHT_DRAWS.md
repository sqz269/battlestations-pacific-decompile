# Dogfight draws: three midpoints made into keyed draws (packet cc13-dogfight1)

Worker cc13-dogfight1, 2026-10-12, base origin/main 4e6dba5c4. **Read-only on cc9's files.** The
deliverable is the unified diff in section 6. It was applied in this worktree, built with
`./scripts/build.ps1`, then reverted, and the copy below passes `git apply --check` against
4e6dba5c4. Nothing was run in game. No Ghidra writes, no ledger records.

Status words: the image sites are **read from the disk listing** (`bsp.py disasm-raw`). The host
change is **reconstructed and build-tested** (with the switches OFF as shipped, and once with all
three ON). It is **not game-validated**: no pair was run.

Source request: `docs/CC13_RNG_STREAM_CENSUS.md` 10.3, the dogfight item. Today the host's dogfight
task takes every stream-1 draw at its midpoint (label `kDogfightManeuverBodyBound`,
`src/game_hosts_units.cpp:5483-5489`). Three of those midpoints change *what* a pilot does, not
only how much. The diff draws them for real, each behind its own new switch, OFF by default:

| switch | site | today | with the switch |
| --- | --- | --- | --- |
| `kDogfightTurnAfterDrawBound` | `009A8712` maneuver expiry | r = 0.5, so TurnAfterChance acts as a threshold | r = U(0, 1) on key `<unit>#dfta` |
| `kDogfightAvoidPickDrawBound` | `009A99C6` avoid pick | roll iff wr >= (wr + wt) / 2, a majority rule | roll iff wr >= U(0, wt + wr) on key `<unit>#dfap`, drawn only where `009A9970` runs |
| `kDogfightGunBurstDrawBound` | `009FCED1` gun burst clock | 3.75 s bursts, 1.15 s pauses, every skill | U(AimShootTime) / U(AimShootDelayTime) of the pilot's skill row on key `<unit>#dfgb` |

## 1. The stream and the keying

All three image sites call `00BD2F10` with `ECX = 1` (stream 1, the thread stream the bots share).
`00BD2F10` -> `00BD2E60` returns `lo + (hi - lo) * u`, `u = word * 2^-32` stored to a float
(`00BD2E9B`-`00BD2EC2`), with `lo` the first (last-pushed) argument. Because `u` is rounded to a
float it can equal 1.0 for words at or above `2^32 - 128`, so `hi` is reachable with probability
about 3e-8. The host's generator returns `[0, 1)` exactly. The difference is ignored.

The diff uses the units host's existing keyed generator, `GameUnitsHost::Impl::release_altitude_draw_00bd2f10`
(`src/game_hosts_units.cpp:6267`), the one under `st_draw` / `lb_draw` and the `n + "#t88"` torpedo
draws. Its key is the unit name plus a suffix, so each draw is per unit and per consumer:

- `#dfta`: the maneuver expiry roll;
- `#dfap`: the avoid pick;
- `#dfgb`: the gun burst clock, both arms (they are one call site in the image).

None of this uses the shared stream-0 generator. **One caveat for cc9.** That function keys its
streams only when `BSP_GUNNERY_RNG_STREAMS=1`. Without it, every caller, these three included,
shares the single host state `db_release_rng`. So with a switch ON and the variable unset, a new
dogfight draw shifts every later units-host draw (the dive-bomb release altitude, the torpedo
reset, the strafe sides). Run pairs with `BSP_GUNNERY_RNG_STREAMS=1` to keep them decoupled.

## 2. `009A8712`: the maneuver-expiry roll (`009A86F0`, BSP_BotStateDogfightManeuver_Expire)

Listing (`009A86F0`-`009A87DF`), read from disk:

- `009A8701 FLDZ` / `009A86F3 FLD1` -> `U(0, 1)`, `009A8712 CALL 00BD2F10`, `ECX = 1` (`009A8703`).
  `009A870B` clears `+18h` (the mode) first. The draw is taken **on every call**, before any test.
- Threat test `009A871B`-`009A875D`: the target `[bot+B4h]` is non-null, and `007BBC10(bot+4)` holds
  (the target's pilot target is us). Then either the flag byte `[target + [00F876B8]*8 + 9C0h]` is 0,
  or `007B9140(0)` on our unit returns true. If all hold, `009A8763 FADD ST0,ST0` doubles `r`.
- `009A876F FLD [row+218h]` (row = `[bot+14h]`, the robots row viewed 0Ch in, so robot_config's
  `dogfight_turn_after_chance_224`, Lua `DogfightTurnAfterChance`). `009A8781 FCOMI ST1` compares the
  chance against `r`, and `009A8783 JBE` skips the turn when `chance <= r`. **Turn when chance > r,
  strict.** A NaN chance does not turn.
- Turn arm: `+34h = BSP_Math_InterpolateClamped(0, 0.25, 0.4, -0.1, chance - r) * row+20Ch * bot+24h`
  (constants `00CE3868` 0.25, `00CE7804` 0.4, `00CE3CB4` -0.1). Otherwise `009A87DF JMP 009A8560`.

Callers: `009A8B20` (the maneuver tick) and `009AAFA0` (avoid -> maneuver). The host
`df_maneuver_expire_009a86f0` serves both, so the draw goes inside it.

**What the midpoint changes.** This installation's robots.lua (mtime 2025-06-01 23:03 UTC) sets
`DogfightTurnAfterChance` to 1.0 for SPNormal (:679), which is the only row the host uses
(`DogfightPilotRow`). Image: unthreatened, r < 1 and the pilot always turns. Threatened, r = U(0, 2),
so the pilot turns half the time. Host today: unthreatened, r = 0.5, always turns. Threatened, r = 1.0
and `1.0 > 1.0` is false, so the pilot **never** turns. The switch restores the 50 %. It also spreads
the turn arm's `+34h`, which depends on `chance - r`.

Not changed here (for cc9):
- The host's threat test (`df_pilot_targets_us`) drops the `00F876B8` flag byte and the
  `007B9140(0)` term.
- The other skill rows set the chance to 0.5 (Stun :1369), 1.8 (SPVeteran :775), 1.4
  (MPNormal :913), 1.6 (MPVeteran :1051) and 1.8 (Elite :1189). Above 1.0 the threatened turn
  probability is chance / 2. The host's `DogfightPilotRow` is SPNormal for every skill, the same
  substitution `kPilotAimShootRows` lifts for the burst clock below. A per-skill
  `turn_after_chance` would be a separate switch.

## 3. `009A99C6`: the avoid pick (`009A9970`, BSP_BotTaskDogfight_PickAvoidState)

Listing (`009A9970`-`009A99DE`):

- `wr = (+708h)->vtable[1Ch]()` (`009A9988`) and `wt = (+730h)->vtable[1Ch]()` (`009A999F`). The host
  names these `009A7F70` (roll weight) and `009A83B0` (turn weight).
- `009A99A5 FLD wt`, `009A99AC FADD wr`, so the high end is `wt + wr`; the low end is `009A99C1 FLDZ`.
  `009A99C6 CALL 00BD2F10`, `ECX = 1` (`009A99B0`).
- `009A99CB FLD wr`, `009A99D1 FCOMPI ST1`, `009A99D5 JAE`: return `+708h` (avoid **roll**) when
  `wr >= r`, else `+730h` (avoid **turn**). A NaN goes to turn.

So P(roll) = wr / (wr + wt) for non-negative weights. The host today computes the weights every tick
and picks turn exactly when `wr < (wr + wt) / 2` (`src/game_hosts_units.cpp:20303-20314`).

**When it is drawn.** `009A9970` has one caller, `009AB0AF` in `009AAFA0`. It runs only in the aim
state (`009AB0A0 CMP EAX,EBX`) with `+6A0h` set (`009AB0A4`). In the host's
`dogfight_transition_009aafa0` (`src/dogfight_task.cpp:178-181`), that branch is the only way from
aim to an avoid state. So the diff leaves the transition function alone. When the old state was aim
and `tr.next` is an avoid state, it draws once and re-picks. The weights are const member functions
with no side effects (`src/game_hosts_units.cpp:19528`, `:19535`). Drawing every tick would be
wrong: it would advance the key's sequence on ticks where the image takes no draw.

## 4. `009FCED1`: the gun burst clock (`009FC7C0`, BSP_BotTaskGun_Tick)

Listing `009FCE49`-`009FCED9`, the end of the tick:

- `+4Bh` set (bursting), `009FCE70 COMISS xmm0,[+50h]` / `JBE`: when `+50h < 0` (xmm0 is the 0.0
  stored to `+40h`/`+68h`/`+6Ch`/`+70h` just above), `+4Bh = 0`, then push `row+240h` (high) and
  `row+23Ch` (low).
- `+4Bh` clear, `009FCE9A`/`009FCEA0`: when `+50h < 0` and `+48h` (fire this tick), `+4Bh = 1`, then
  push `row+238h` (high) and `row+234h` (low).
- Both arms meet at `009FCEC8`: `ECX = 1`, `009FCED1 CALL 00BD2F10`, `009FCED6 FSTP [+50h]`, and the
  function returns. The store is the tick's last write.

`row` is `0099C210` (BSP_BotTask_PilotRobotsRow): `[[task+3F4h]+34h] * 248h + [00F8A30C] + 0Ch`,
the pilot's skill-level row viewed 0Ch in. View `+234h/+238h/+23Ch/+240h` are robot_config's
`aim_shoot_time_2_240`, `aim_shoot_time_1_244`, `aim_shoot_delay_time_2_248`,
`aim_shoot_delay_time_1_24c` (`src/robot_config.cpp:132-135`; array index 1 is the first Lua element,
as `DiveBombReleaseAlt` {350, 450} -> `+44h` 350 confirms). So the draw is
**U(AimShootTime[2], AimShootTime[1])** for a burst and **U(AimShootDelayTime[2], AimShootDelayTime[1])**
for a pause.

How the host loads them: it does not. The PilotBot registry is out of the host's reach, so cc9's
dive-bomb and strafe rows (`kPilotDiveBombRows`, `kPilotStrafeRows`, `src/game_hosts_units.cpp:3326-3405`)
are copied from this installation's robots.lua and indexed by `pilot_skill_index` under
`kSkillLevelBound`. The diff adds `kPilotAimShootRows` and `aim_shoot_row()` the same way. robots.lua
is modded in this installation (mtime 2025-06-01 23:03 UTC), so these are this installation's values,
not retail's:

| level | row (robots.lua lines) | AimShootTime | AimShootDelayTime | burst U(lo, hi) | pause U(lo, hi) |
| --- | --- | --- | --- | --- | --- |
| 0 | Stun :1297-1298 | {3.0, 2.0} | {2.0, 1.0} | U(2.0, 3.0) | U(1.0, 2.0) |
| 1 | SPNormal :607-608 | {4.5, 3.0} | {1.6, 0.7} | U(3.0, 4.5) | U(0.7, 1.6) |
| 2 | SPVeteran :745-746 | {10.0, 8.0} | {0.1, 0.0} | U(8.0, 10.0) | U(0.0, 0.1) |
| 3 | MPNormal :883-884 | {5.5, 4.0} | {1.4, 0.9} | U(4.0, 5.5) | U(0.9, 1.4) |
| 4 | MPVeteran :1021-1022 | {7.0, 5.0} | {1.0, 0.5} | U(5.0, 7.0) | U(0.5, 1.0) |
| 5 | Elite :1159-1160 | {10.0, 8.0} | {0.1, 0.0} | U(8.0, 10.0) | U(0.0, 0.1) |

The level order (Stun 0, SPNormal 1, ...) is the one `kPilotDiveBombRows` documents
(`register_robot_config_009013d0`). The host's 3.75 / 1.15 are the SPNormal midpoints. **This is the
largest behaviour change of the three.** SPVeteran and Elite fighters fire 8-10 s bursts with
pauses of at most 0.1 s, where the host now gives them 3.75 s and 1.15 s.

Host placement: `bsp::dogfight_gun_tick_009fc7c0` (`src/dogfight_task.cpp:302-312`) stores
`in.burst_draw` / `in.delay_draw` on the same two edges. Its caller already keeps `was_burst`
(`src/game_hosts_units.cpp:19866`), and nothing else writes `burst_4b`. So the diff overwrites
`burst_timer_50` right after the call whenever `burst_4b` changed. That is equivalent, because the
image's store is the tick's last write, and the fine aim the host runs next does not read `+50h`.
`src/dogfight_task.cpp` and its header are untouched.

Skill index: the image reads the level live through `[task+3F4h]+34h`. The diff uses
`pilot_skill_index`, as `strafe_row()` does. It does not use the dive-bomb approach's captured
`db_skill_row_14`. That mapping is not verified for the gun task (section 7).

## 5. The other dogfight midpoint sites

From the census rows (`reports/cc13_rng_stream1_host_status.json`). "Spread" means the midpoint keeps
the mean of a continuous quantity and only removes its variance. "Behaviour" means a discrete choice
changes. None of these is in the diff.

| site | function | image draw | at its midpoint | verdict |
| --- | --- | --- | --- | --- |
| `009A75EE` | aim enter `009A75C0` | bored limit `U(0.8, 1.4) * BoringTime` | 1.1 | spread: when aim gives up for maneuver |
| `009A7624` | aim enter `009A75C0` | too-close `U(0.5, 0.8) * FollowDist` | 0.65 | spread: the distance that triggers avoid |
| `009A7D72` | avoid common tick | timer > 2 -> `U(1, 2)` | 1.5 | spread: avoid duration clamp |
| `009A7E09` | avoid-roll enter | timer `U(0.75, 1.5) * AvoidTime` | 1.125 | spread |
| `009A7E4E` / `009A7E6C` | avoid-roll enter | bank `U(0.8, 1.0)` / `U(-1.0, -0.8)` | 0.9 / -0.9 | spread: a separate test picks the side |
| `009A8049` | avoid-turn enter | timer `U(0.75, 1.5) * AvoidTime` | 1.125 | spread |
| `009A807B` | avoid-turn enter | `+20h U(0.75, 1.0)` | 0.875 | spread |
| `009A80B5` / `009A80D3` | avoid-turn enter | heading `U(70, 100)` / `U(-100, -70)` deg | +/-85 deg | spread: a separate test picks the side |
| `009A8596` | maneuver from aim `009A8560` | pursuit range `U(0.3, 1.1) * ShootDistance` | 0.7 | spread |
| `009A88BD` | maneuver enter | heading `+ U(-0.5, 0.5)` toward a threat | 0 | spread: zero-mean jitter |
| `009A89A5` | maneuver enter | heading `+ U(-0.8, 0.8)` | 0 | spread: zero-mean jitter |
| `009A8AD5` | maneuver enter | pitch `U(lo, hi)` | (lo+hi)/2 | spread |
| `009A8AFE` | maneuver enter | timer `U(0.8, 1.3) * ManeuverChangeTime` | 1.05 | spread |
| `009A906B` | maneuver tick | `U(lo, hi)` on recompute | (lo+hi)/2 | spread |
| `009AA65A` | select target | reselect timer `U(1, 3)` | 2 | spread: reselect cadence |
| `009AA97F` | select target | a non-current candidate's score `* U(0.8, 1.0)` | 0.9 | **mild behaviour**: a fixed 10 % hysteresis replaces a random 0-20 % one, so near-equal candidates switch deterministically. The mean is close |
| `007E21AA` | neighbour finder `007E2090` | `score += U(0, noise)` per candidate | `+ noise/2` to every positive score | **behaviour**: a constant added to every candidate cannot reorder them. The host finder is a strict argmax where the image randomises among near-equal targets (`src/game_hosts_units.cpp:20049`) |

The two constructor phase draws, `009A713A` (attackrun) and `009A853A` (maneuver), `-U(0, 1)`, are not
taken at all. They stagger the first countdown, so their effect is timing (spread). The neighbour
clocks `007E1F94` / `007E1FB9` are seeded at midpoints, also spread.

If cc9 wants a fourth, `007E21AA` is the candidate. It is outside the dogfight task proper (the
`unit+C50h` finder), and its draw count per tick depends on the candidate count, so it would need its
own key.

## 6. The diff (against origin/main 4e6dba5c4, `src/game_hosts_units.cpp` only)

Five hunks: the per-skill table after `strafe_row()`, the three switches after
`kDogfightAvoidPickBound`, and one hunk at each site. Line numbers are 4e6dba5c4's. Apply with
`git apply` from the repository root.

```diff
diff --git a/src/game_hosts_units.cpp b/src/game_hosts_units.cpp
index f8634b115..528d3372a 100644
--- a/src/game_hosts_units.cpp
+++ b/src/game_hosts_units.cpp
@@ -3403,6 +3403,33 @@ struct GameUnitsHost::Impl {
         if (!kSkillLevelBound || i < 0 || i > 5) return kPilotStrafeRows[1];
         return kPilotStrafeRows[i];
     }
+    // Packet cc13-dogfight1. AimShootTime and AimShootDelayTime of the six
+    // PilotBot rows, which 009FC7C0 reads through 0099C210 (00F8A30C +
+    // level * 248h + 0Ch): view +234h/+238h/+23Ch/+240h are robot_config.hpp's
+    // aim_shoot_time_2_240/_1_244 and aim_shoot_delay_time_2_248/_1_24c, so the
+    // draw's low end is the Lua table's second element. SUBSTITUTION, labelled,
+    // as above: this installation's scripts/datatables/robots.lua (mtime
+    // 2025-06-01 23:03 UTC), by line: SPNormal :607-608, SPVeteran :745-746,
+    // MPNormal :883-884, MPVeteran :1021-1022, Elite :1159-1160, Stun :1297-1298.
+    struct PilotAimShootRow {
+        float time_lo_234;
+        float time_hi_238;
+        float delay_lo_23c;
+        float delay_hi_240;
+    };
+    static constexpr PilotAimShootRow kPilotAimShootRows[6] = {
+        {2.0f, 3.0f, 1.0f, 2.0f},     // Stun
+        {3.0f, 4.5f, 0.7f, 1.6f},     // SPNormal
+        {8.0f, 10.0f, 0.0f, 0.1f},    // SPVeteran
+        {4.0f, 5.5f, 0.9f, 1.4f},     // MPNormal
+        {5.0f, 7.0f, 0.5f, 1.0f},     // MPVeteran
+        {8.0f, 10.0f, 0.0f, 0.1f},    // Elite
+    };
+    static const PilotAimShootRow& aim_shoot_row(const GameUnitSlot& slot) noexcept {
+        const int i = slot.pilot_skill_index;
+        if (!kSkillLevelBound || i < 0 || i > 5) return kPilotAimShootRows[1];
+        return kPilotAimShootRows[i];
+    }
 
     // Packet cc9_ship_motion_tail. ShipGlobals.Formacio.UpdateInterval (0083F0AD
     // stores it at settings+430h); 10.0 in this installation's shipglobals.lua.
@@ -5489,6 +5516,14 @@ struct GameUnitsHost::Impl {
     static constexpr bool kDogfightManeuverBodyBound = true;
     static constexpr bool kDogfightAvoidBodiesBound = true;
     static constexpr bool kDogfightAvoidPickBound = true;
+    // Packet cc13-dogfight1 (docs/CC13_DOGFIGHT_DRAWS.md): three of those draws
+    // taken on keyed host streams (release_altitude_draw_00bd2f10, unit name +
+    // suffix) instead of at their midpoints: 009A8712 the maneuver-expiry roll
+    // against TurnAfterChance, 009A99C6 the avoid pick, 009FCED1 the gun burst
+    // clock from the pilot's skill row. False: the midpoints, as before.
+    static constexpr bool kDogfightTurnAfterDrawBound = false;
+    static constexpr bool kDogfightAvoidPickDrawBound = false;
+    static constexpr bool kDogfightGunBurstDrawBound = false;
     // ON since packet cc9_planner_heading_writes: with the torpedo moveto and
     // attackrun heading writes bound (kPilotStateHeadingWritesBound) the Kates
     // close again; pair PHW0/PHW1, docs/PLANNER_HEADING_WRITES.md section 6.
@@ -19225,6 +19260,13 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                         ++unit_.mv_expires;
                         unit_.mv_mode_18 = 0;
                         float r = 0.5f;                                              // U(0, 1)
+                        if constexpr (GameUnitsHost::Impl::kDogfightTurnAfterDrawBound) {
+                            // 009A8712: 00BD2F10(0 [FLDZ], 1 [FLD1]) on ECX = 1, taken
+                            // on every call before the threat test; 009A8781 FCOMI and
+                            // 009A8783 JBE turn only when TurnAfterChance > r.
+                            r = owner_.release_altitude_draw_00bd2f10(
+                                unit_.row.name + "#dfta", 0.0f, 1.0f);
+                        }
                         const GameUnitSlot* t = df_target_slot();
                         if (t != nullptr && df_pilot_targets_us(*t)) r = r + r;     // 009A8763
                         const float c = unit_.df_row.turn_after_chance;             // row+218h
@@ -19882,6 +19924,20 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                             ++unit_.st_gun_point_ticks;
                         }
                         bsp::dogfight_gun_tick_009fc7c0(unit_.df_gun, gi);
+                        if constexpr (GameUnitsHost::Impl::kDogfightGunBurstDrawBound) {
+                            // 009FCE70-009FCED6: each +4Bh edge stores one
+                            // 00BD2F10(low, high) on ECX = 1 in +50h, the tick's last
+                            // write, so replacing the midpoint here is the same.
+                            // On: U(row+234h, +238h); off: U(row+23Ch, +240h).
+                            if (unit_.df_gun.burst_4b != was_burst) {
+                                const GameUnitsHost::Impl::PilotAimShootRow& sr =
+                                    GameUnitsHost::Impl::aim_shoot_row(unit_);
+                                const bool on = unit_.df_gun.burst_4b;
+                                unit_.df_gun.burst_timer_50 = owner_.release_altitude_draw_00bd2f10(
+                                    unit_.row.name + "#dfgb", on ? sr.time_lo_234 : sr.delay_lo_23c,
+                                    on ? sr.time_hi_238 : sr.delay_hi_240);
+                            }
+                        }
                         if (strafe_point && unit_.df_gun.fire_48) ++unit_.st_gun_point_fires;
                         if constexpr (GameUnitsHost::Impl::kFighterGunLeadBound) {
                             const bool ran = df_fine_aim_009f9fc0(gi);
@@ -20313,7 +20369,25 @@ void GameUnitsHost::motion_step_00825f20(float step_seconds) {
                                 const float sum = wt + wr;
                                 ti.avoid_pick_turn = !(wr >= sum * 0.5f);
                             }
-                            const bsp::DogfightTransition tr = bsp::dogfight_transition_009aafa0(ti);
+                            bsp::DogfightTransition tr = bsp::dogfight_transition_009aafa0(ti);
+                            if constexpr (GameUnitsHost::Impl::kDogfightAvoidPickBound &&
+                                          GameUnitsHost::Impl::kDogfightAvoidPickDrawBound) {
+                                // 009AB0AF calls 009A9970 only on aim with +6A0h set,
+                                // the transition's only aim -> avoid arm, so the draw
+                                // is taken there alone. 009A99C6 00BD2F10(0, wt + wr)
+                                // on ECX = 1; 009A99D1 FCOMPI, 009A99D5 JAE: roll
+                                // (+708h) when wr >= r, else turn (+730h).
+                                if (was == bsp::DogfightState::kAim &&
+                                    (tr.next == bsp::DogfightState::kAvoidRoll ||
+                                     tr.next == bsp::DogfightState::kAvoidTurn)) {
+                                    const float wr = df_avoid_roll_weight_009a7f70();
+                                    const float wt = df_avoid_turn_weight_009a83b0();
+                                    const float r = owner_.release_altitude_draw_00bd2f10(
+                                        unit_.row.name + "#dfap", 0.0f, wt + wr);
+                                    tr.next = wr >= r ? bsp::DogfightState::kAvoidRoll
+                                                      : bsp::DogfightState::kAvoidTurn;
+                                }
+                            }
                             unit_.dogfight_state = tr.next;
                             if (tr.maneuver_from_aim_8560) {
                                 unit_.mv_mode_18 = 0;   // 009A8567
```

## 7. Uncertainty

- The skill index. The image reads the level through `[task+3F4h]+34h` at every edge. The diff takes
  `pilot_skill_index`, which is what `strafe_row()` uses. That the two agree for fighters in the gun
  task was not traced.
- The weights `009A7F70` / `009A83B0` are the host's reconstruction. Their vtable slot `+1Ch` mapping
  on `+708h` / `+730h` is taken from the host's own comment, not re-derived here.
- `xmm0 = 0.0` at `009FCE70` / `009FCE9A` is inferred from the stores just above it, which match
  the host's existing "zeroed each tick end" note. Its last write was not traced back.
- The draws match the image in distribution, not in sequence. The image takes them on stream 1, shared
  with every other bot. The keyed host streams decouple them by design, as cc9 asked.
- Nothing was run. The behaviour claims in sections 2-4 are arithmetic on the listing and on this
  installation's robots.lua. They are not measurements.

## 8. Reproduction

`python tools/bsp.py disasm-raw 009a86f0 --length 0x150`, `... 009a9970 --length 0x70`,
`... 009fce30 --length 0xB0`, `... 0099c210 --length 0x20`, `... 00bd2e60 --length 0x70`. Values:
`Select-String robots.lua -Pattern 'AimShootTime|AimShootDelayTime|DogfightTurnAfterChance'`. The diff
is also at `local/df1b_dogfight_draws.diff` in this worktree. Apply-check:
`git apply --check` from the root of a 4e6dba5c4 checkout.

## 9. Handoff

Done: the three sites are read, the diff is written, built OFF and ON, and reverted. Open, for cc9: run
on/off pairs per switch with `BSP_GUNNERY_RNG_STREAMS=1`. Then decide on a per-skill
`turn_after_chance` (section 2) and on `007E21AA` (section 5).