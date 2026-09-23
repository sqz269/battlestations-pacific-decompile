# The faithful dive set, switched on together

Packet `cc9_faithful_dive_set`. The integrator's decision: main's 32 USN04 releases come from a
host whose throttle never moved (`docs/PILOT_THROTTLE_SLOT.md`), so they are not a reference. Every
switch below is the image's law with a read behind it, and they flip on as one change. This packet
changes switch constants only.

## 1. The switches

| switch | the read that makes it image law | the pair that measured it |
| --- | --- | --- |
| `kPilotThrottleSlotBound` | 0099D300's speed-hold multiplier is dt in speed mode: 0099D79F jumps past the pending block, 0099D4EE, 0099DBBF (`docs/PILOT_THROTTLE_SLOT.md` 1) | S0/S1 (`docs/PILOT_THROTTLE_SLOT.md` 5); C0/T1 (`docs/DIVE_THROTTLE.md` 4) |
| `kPlaneAccelCheatScaleBound` | class+164h scaled by AccelCheatMulMul × AccelCheatMul at 007D20F8-007D2127, before +50Ch (`docs/DIVE_FLIGHT_RESPONSE.md` 1) | R0/R1 16 → 26; S0/S1 32 → 24 (`docs/DIVE_FLIGHT_RESPONSE.md` 5, 7) |
| `kAimDiveTailBound` | the aimdive tail's yaw, throttle and air brake, 009C5DB8-009C6080 (`docs/AIMDIVE_RESPONSE.md` 1); its rejection rested on the frozen throttle (`docs/DIVE_THROTTLE.md` 4) | T1 (`docs/DIVE_THROTTLE.md` 4); U1-U4 (`docs/AIMGLIDE_PITCH.md` 4) |
| `kAimGlidePitchBound` | the glide's pitch target, 009C5484-009C55DF (`docs/AIMGLIDE_PITCH.md` 1) | U1/U2 (`docs/AIMGLIDE_PITCH.md` 4) |
| `kAimGlideYawBound` | the glide's yaw arm below a 140 m miss, 009C53D0-009C542A (`docs/AIMGLIDE_PITCH.md` 3a) | U3/U4 (`docs/AIMGLIDE_PITCH.md` 4) |
| `kReleaseAltitudeDrawBound` | approach+A8h = 00BD2F10(row+38h, row+3Ch) at 009C3F29, once per task (`docs/AIMGLIDE_PITCH.md` 2); the generator is a labelled copy of the gunnery host's | U1-U4 |
| `kDogfightMovetoGenericBound` | the dogfight moveto is the generic 009C18C0 (009C2CA0, vtable 00D20B24) with ranges 500/100/1000 from 009A955B (`docs/DIVE_MODES.md` 3) | V1/V2 (`docs/DIVE_MODES.md` 5): the leader holds 1475 m and 82.2 m/s; G against the control on main |
| `kAttackDistDrawBound` | approach+B4h = 00BD2F10(0.6, 0.8) × class+268h at 009C3F63-009C3F97 (`docs/FIGHTER_GUNFIRE_RATE.md` 4) | B0/B1 (`docs/FIGHTER_GUNFIRE_RATE.md` 6) |

All eight are ON in this packet's commit. `kGoawayThrottleBound` and `kAimGlideThrottleBound` were
already on (`docs/DIVE_THROTTLE.md`). `kHullAimOffsetEnabled` and the diagnostic traces stay off.

## 2. Predictions, written before the runs

The pair is E2 USN04 9000 with `BSP_GUNNERY_RNG_STREAMS=1`, both sides built in this tree from
abe724891. **OFF** (`local\fdOFF`) is the tree's build with every switch as main has it. It is not
S0, because main has taken other packets since S0 was built. **ON** (`local\fdON`) has the eight
switches on.

1. **Releases.** ON gives about 26: R1's set plus the two draws and the generic moveto. The B4h
   draw was measured only on the frozen-throttle configuration, where it moved release altitudes
   but not the count. I expect **22-30**. OFF gives about 32.
2. **Mode B.** Flights #3.1 and #7.1 still fail their first pass, and a few release on later passes,
   as #3.1|.-2 and #7.1 did in R1.
3. **Mode A** stays gone. The #1.1 and #5.1 wingmen and movieval|.-3 release from the glide.
4. **Water contacts.** About 8, the post-release descents, as in R1.
5. **The Yorktown leader** settles near 1475 m and 82 m/s in moveto, as in V2, and not at 1547 m
   and 34 m/s.
6. **Fighter trigger ticks.** Near R1's 87, from the Yorktown flight only.
7. **The Lexington's fate** follows the RNG coupling. In R1 the mission failed at 258 s, in R0 at
   297 s and in OFF-type runs at 228 s. I expect it lost between 180 and 300 s.

The reference rows (`docs/GAME_EXECUTABLE.md`, re-baseline of this packet) are the ON set at USN04 4700/4500 and USN01 3200/3000, run
**without** the RNG-streams option, as the previous reference sections were.

## 3. The pair (E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`)

| run | log | bomb drops | torpedo drops | kill credits | queued hits | water | projectiles | fighter trigger ticks | mission end |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| OFF | `local\fdOFF_9000.log` | 32 | 16 | 30 | 354 | 8 | 14611 | 6 | failed 228.06 s |
| ON | `local\fdON_9000.log` | **25** | 16 | 34 | 449 | 7 | 4305 | **263** | failed 261.06 s |

OFF matches B0 (`docs/FIGHTER_GUNFIRE_RATE.md` 6) on drops, water and projectiles. B0 was built
from c51373373 with the same switch states.

**Against the predictions:**
1. **Releases: 25, inside 22-30.** Thirteen aircraft release, each a salvo or two drops, and one
   single round.
2. **Mode B held.** Flights #3.1 and #7.1 fail every first pass: every aimdive ends at the floor,
   1046-1431 m out. #3.1 and #3.1|.-4 release from the glide on a later pass. #3.1|.-2, #3.1|.-3 and
   all of #7.1 never release.
3. **Mode A held.** The #1.1 and #5.1 wingmen and movieval|.-3 release from the glide.
4. **Water: 7**, all post-release descents at about 68 m/s.
5. **The fighters: not predicted.** Trigger ticks are **263, not about 87**.
   * The Yorktown flight holds as in R1: 3 bursts, 94 ticks.
   * The **Lexington flight now engages.** Its leader and |.-2 fire 7 bursts (169 ticks) at the
     #3.1 flight, at 289-851 m. Its leader's closest range is 1757 m, where OFF and R1 both show
     17 km or more.
   * This is `kDogfightMovetoGenericBound`, which R1 did not have. The read showed the stand-in
     hung the leader at the 1500 m ceiling at 34 m/s. Under the generic moveto the flight holds
     about 1475 m at about 82 m/s (`docs/DIVE_MODES.md` 5), fast enough to close.
   * Hits are still unled (`docs/FIGHTER_GUNFIRE_RATE.md` 3).
6. **The Lexington** is lost at 261 s, inside 180-300 s.
7. **The Yorktown leader's altitude** is not checked in ON, because the throttle trace is off in
   landed builds. V2 measured 1475 m and 82.2 m/s with the same moveto.

**Projectiles** fall from 14611 to 4305. That fits the measured pairs: the throttle fix alone
brought them to 6873-7661 (R0/R1), and the B4h draw alone cut them by 25% on the frozen
configuration (B0/B1, 14611 to 10897). Not traced row by row.

## 4. Decision

Nothing is outside the reads. The one unpredicted row, the Lexington flight engaging, is the generic
moveto doing what its read said. **All eight switches land ON.**
