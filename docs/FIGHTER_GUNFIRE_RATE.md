# Why the fighters rarely fire and never hit

Addresses: 009FC7C0, 007E2090, 007E11D0, 009AB1C0, 009AAC70, 00954650, 007C2610, 009FADA0,
0077D600, 009C3EA0, 009C3F63-009C3F97, 00D1A630.

Packet `cc9_fighter_gunfire_rate`. Every name is a hypothesis, not a recovered symbol. The logs
are `local\S0fr_9000.log` (main at 327366749, with the fighter gun hook 635e7b27b) and
`local\R1fr_9000.log` (the V1 throttle-fix set plus the Accel scale). Both are E2 9000 runs with
`BSP_GUNNERY_RNG_STREAMS=1`. A 120-frame probe (`local\probeO.log`) passed at about 12:30, so R0 and
the B4h pair were run (sections 4 and 5).

## 1. The image's fighter trigger (from `docs/DOGFIGHT_GUN.md`, not re-read)

The task gun controller 009FC7C0 (task+314h) is ticked by 00999979 after the dogfight arm 009AB1C0
while PilotFires (unit+C24h) is set. Its `+48h` becomes task+2E0h, then cmd+16h, unit+9FAh and
finally unit+BC9h, the gunFire that 007CE9F4 hands to each gun's SetTriggerHeld. The conjuncts:

| conjunct | constant | source | host |
| --- | --- | --- | --- |
| a target at +74h | 007B96F0, then 007E2090 on unit+C50h, fed from an enemy-plane list refreshed every 3 s within 1200 m ((3 + 3) × 180, floored at 1200; 007E11D0) and scanned inside a cone max(0.4, 1.25 × +40h) out to +30h = ShootDistance + 650 = 1500 | `docs/PLANE_GUNFIRE.md` 3 | bound (`kPlaneFinderBound`, `df_finder_007e2090`) |
| the lead point +5Ch | the target's vtable[48h] prediction at t = d / 007C2610(), iterated twice | 009FC7C0 step 2 | **SUBSTITUTED**: the target's current position, "unled" (`df_gun_tick_009fc7c0`) |
| range | 1 < d < max(+30h, +34h + 200) = 1500 | 009FC7C0 step 3 | bound |
| cone | lateral < d × +38h, with +38h = max(AimDistortAngle1 × 3, 0.08) = 0.09, a half-angle of about 5° | step 4 | bound |
| ahead and in range | 1 < z < +34h = ShootDistance 850 | step 4 | bound |
| don't-shoot | lateral > +3Ch / 1.8, and +3Ch = 0, so lateral > 0 | step 4 | bound |
| alive, finder idle | !007BA760, !007B96D0 | step 6 | bound |
| burst clock | AimShootTime {4.5, 3.0} and AimShootDelayTime {1.6, 0.7} | step 7 | bound |

## 2. What the logs show

**S0 (main):**
* The six fighters hold the trigger for **6 ticks in the whole run**: one burst, by
  Yorktown-class01_sqn02|.-3 at d = 846.5 m, lateral 56.6 m, in aim. The census totals are
  trigger_ticks 72 and rounds 35 over all plane guns, including the Vals' rear guns.
* **The Yorktown flight** (#sqn02, target flight D3A Val #3.1) enters aim at d = 1977-1997 m and
  drops back to follow or moveto at d = 2151-2162 m. The leader's closest approach over 3822
  moveto ticks is 1910 m, and it has 8 ticks inside the attack distance.
* **The Lexington flight** (#sqn01) is given the same target, D3A Val #3.1, at 26.8 km. It stays in
  moveto or follow all 4214 ticks, and its closest approach is **17.2 km**.

**R1 (throttle fix):** 3 bursts and 87 trigger ticks, all from the Yorktown flight. The Lexington
flight again stays at 17.7 km or more.

**The closed conjunct is range.** In S0 the Yorktown fighters sit around 2000 m, outside the
enemy-list radius (1200 m), the finder range (1500 m) and the shoot distance (850 m), for all but
a few ticks. The Lexington flight is never within 17 km. When a fighter does get inside, the
cone, ahead and burst conjuncts pass. That is the one S0 burst.

## 3. Image or host

1. **The aim exit.** Aim runs while the approach latch holds:
   `d < AttackDist × approach+24h + (latched ? 150 : 0)` (`docs/DOGFIGHT_ENGAGED.md`, 009AADC5).
   The exit at about 2150 m and the entry at about 1980 m are that latch's two edges. That is the
   image's rule.
2. **Why the Yorktown flight cannot close.** In S0 the Vals fly the frozen-throttle profile: full
   throttle, 69 m/s level and 126-135 m/s in the dive (`docs/PILOT_THROTTLE_SLOT.md`). The Wildcat
   tops out at 83.3 m/s, and its head-on arm cuts it to 0.37-0.45 of MaxSpd for about 200 ticks
   (`head_on_throttle_min` 0.374-0.445). A fighter behind a Val at a 14 m/s closure, or behind a
   diving Val, falls back past the latch edge. With the image's Val speeds (65.97 m/s in the
   fly-over, about 67 m/s in the dive, `docs/DIVE_FLIGHT_RESPONSE.md` 3) the closure would be
   about 17 m/s on the level and positive in the dive. **So S0's range conjunct is partly the
   frozen-throttle artefact.**
3. **Why the Lexington flight never engages.** Its target is chosen upstream of the dogfight. The
   AI planner's `ai diag order_attack group_members=6 group_leader=Lexington-class01_sqn01
   target_members=6 target_leader=D3A Val #3.1` merges both fighter flights into one six-member
   group and orders it onto the #3.1 Val group. 0077D600 issues the member orders. The Kates that
   sink the Lexington (`docs/TURNDOWN_EXIT.md` 4) are never anyone's dogfight target. Whether the
   image's planner would pick the Kates for the Lexington's own flight is the planner's target
   weighting (00A0F810 / 00A08460), which is outside this packet.
4. **Zero hits: the lead point.**
   * The image tests the envelope, and steers the fine aim (009F9FC0), against the target's
     predicted position at t = d / muzzle speed, iterated twice. The host uses the target's
     current position.
   * The guns are fixed forward, so unled fire misses a crossing target by v⊥ × t. At d = 850 m
     and a muzzle speed of about 850 m/s (t ≈ 1 s), a Val crossing at 67-130 m/s is missed by
     67-130 m. The whole cone is ±76 m at that range (0.09 × 850).
   * **This is a host divergence, and it explains 0 hits on the rounds that are fired.**

**The prediction, 00954650** (the plane class's vtable+48h, `vtable 00D19D28`,
`__thiscall(out, t)`, `RET 8`):
* v = vtable+34h (007BBB70, unit+AC8h), and s = \|v\|.
* When s ≤ 1.0, or the turn term built from unit+654h-65Ch against [00D1A630] (a double of about
  0.0087) is small, it takes the **straight arm** (009548C5-00954937): **out = unit+FCh..104h +
  v × t**.
* Otherwise it takes an **arc arm**. It normalises the axis through 0042B2F0 and 004F9B30, then
  rotates about it through 00952340 (009546F6-009548BE). Those three helpers are unread.

**Why it is not bound here.** 007C2610, the muzzle speed, is the minimum of
`[[part+3F8h]+34h]+50h` over the gun parts. Those values live in the gunnery host's projectile
tables, and the units host has no accessor. The change needs:
1. **In `src/game_hosts_gunnery.cpp` (cc9-gunnery-host):** a public
   `float plane_muzzle_speed_007c2610(std::size_t unit)` that returns the minimum projectile speed
   over the unit's gun parts that answer vtable[5Ch](21h), as
   007C2610 does (`docs/DOGFIGHT_ENGAGED.md`).
2. **In `src/game_hosts_units.cpp`, `df_gun_tick_009fc7c0` and `df_finder_007e2090`:** replace the
   unled `lead_local` with 00954650's straight arm, evaluated twice as in 009FC7C0 step 2
   (t0 = d / muzzle, p1 = pos + v × t0, t1 = \|p1 - own\| / muzzle, p2 = pos + v × t1). The arc arm
   stays labelled unmodelled until 0042B2F0, 004F9B30 and 00952340 are read.
3. **Predictions, for the lead to queue after R0:**
   * Fighter hits go from 0 to non-zero on the Yorktown flight's bursts, a few per burst.
   * Trigger ticks change little, because the range conjunct still gates.
   * Kate deaths do not change: no fighter engages the Kates.
   * The Lexington's fate does not change: it still dies to the Kate torpedo at about 226 s.

## 4. B4h draw (secondary)

* `kAttackDistDrawBound` binds approach+B4h = 00BD2F10(0.6 [00CE3D30], 0.8 [00CE74F8]) ×
  class+268h (009C3F63-009C3F97), drawn after +A8h. It uses the same labelled generator as
  `kReleaseAltitudeDrawBound` (keyed per unit under the RNG-streams option). It lands **OFF**,
  because it is unmeasured.
* **Prediction.** Only the fly-over roll-in distance moves, through its B4h-scaled terms. Mode B
  does not move, because the leave tolerance stays at its 20° floor for every draw
  (`docs/MODE_B_INPUTS.md` 2).

## 5. Decision

* **The closed conjunct is range.**
  * For the Yorktown flight, the approach latch's 1980/2150 m edges are the image's rule, reached
    against the frozen-throttle Val speeds, which are the host's artefact.
  * For the Lexington flight, it is the planner's target choice: #3.1 at 17-27 km.
* **The divergence is the lead point.** The host fires unled while the image fires at 00954650's
  prediction. It explains the 0 hits. It is not bound, because it needs the gunnery host's muzzle
  speed; the change is written in section 3 for routing.
* **Switches landed:** `kAttackDistDrawBound` OFF. Nothing else changed.
* R0 is still queued. This packet's probe result is in the report.

## 6. Runs taken after the probe passed

**R0.** R0 ran first, as queued: `local\R0fr_9000.log`, 16 drops, 34 kills, 7 water, mission failed
at 297.05 s. R1 against R0 is 26 against 16 in the same tree. See `docs/DIVE_FLIGHT_RESPONSE.md` 7.

**The B4h pair.** Main's configuration, built in this tree from c51373373, differing only by
`kAttackDistDrawBound`.

| run | log | drops | kill credits | water | Lexington dies | mission end |
| --- | --- | --- | --- | --- | --- | --- |
| B0 | `local\B0_9000.log` | 32 | 30 | 8 | 225.8 s | failed 228.06 s |
| B1 | `local\B1_9000.log` | 31 | 34 | 11 | **180.6 s** | failed 183.06 s |

* **The prediction failed.** It was that only the roll-in distances move. In fact every release
  row moves: release altitudes by up to about 70 m (for example #7.1|.-4 from 349/291 to 284/219).
* The per-aircraft rounds are nearly unchanged (31 against 32).
* The Lexington dies 45 s earlier.
* Three water contacts are new, and one of them, D3A Val #3.1 at 27.1 m/s, is a slow one.
* So B4h reaches more than the fly-over roll-in. It is also the attack-run and approach distance
  that the other dive states and the timing read.
* **`kAttackDistDrawBound` stays OFF.** The draw is the image's, but its knock-on to the
  Lexington's death time is not explained.
