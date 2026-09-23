# The aimdive entry swing is the image's own law

Addresses: 009C58D0, 009C593A, 009C59BA-009C5A58, 009C5A5E-009C5AF9, 009C5AFD-009C5B51,
009C5B54-009C5BB6, 009C5BBC-009C5C9B, 009C5C9F-009C5CFA, 009C5D33, 00438B10, 00438AA0,
009C4F80, 009C7D71, 007BCC80, 00D7A3A0, 00D7A264, 00CE3830.

Packet `cc9_aimdive_entry`. Every name is a hypothesis, not a recovered symbol. Background:
`docs/DIVE_MODES.md` section 6, which named the entry swing as the next term.

## 1. The aimdive tick 009C58D0 up to the pitch command

The frame is `SUB ESP,48h` plus four pushes, so the base is 58h and `[ESP+5Ch]` is the dt argument.
The dt slot is reused as scratch from 009C59EA. Every called helper pops its own arguments (the
thiscall getters take `RET 4`, 00438B10 and 00438AA0 take `RET 8`, 00419010 takes `RET 14h`), so
the offsets below are base offsets.

| slot | writer | value |
| --- | --- | --- |
| +10h | 009C593A | 009C4F80, the aim heading |
| +40h, +48h | 009C5959, 009C596F | fed aim point minus approach+D8h/+E0h, the predicted impact point (planar) |
| +34h, +3Ch | 009C5992, 009C59AB | fed aim point minus the unit (planar) |
| +14h | 009C59D6 | unit+100h minus the aim point's y: H, the height above the aim |
| +1Ch | 009C59FE-009C5A16 | the planar range to the aim, sqrt(+34h² + +3Ch²), 0 under 1e-10 (00CE3820) |
| +5Ch | 009C5A40-009C5A58 | the planar miss \|aim - impact\|, sqrt(+40h² + +48h²) |
| +24h | 009C5AA3 | e_air = 00438B10(+10h, bearing(aim - unit)), with the bearing wrapped as π/2 - atan2 (00CE3830, +2π at 00CE3828) |
| +18h | 009C5AF1 | e_imp = 00438B10(+10h, bearing(aim - impact)) |

00438B10 is `wrap(arg0 - arg1)`: 00438B14 FSUB [ESP+8] from [ESP+4].

**The abort** 009C5AFD-009C5B51 fires when all three hold, and matches the host's
`dive_bomb_dive_abort_009c5b43`:
* approach+D4h + approach+50h > H;
* pitch unit+C64h > -60° [00D20338];
* 0.3 [00CE3DC8] × H + 150 [00CE3DD8] > the planar range (+1Ch).

**The "flip" at 009C5B54-009C5BB6 has no effect.**
* `[ESP+1Ch] = |e_air|` (009C5B54-009C5B77). If |e_air| > π/2 (00CE3830 qword; 009C5B87 FCOMIP,
  009C5B8B JBE) and the planar range > 0.1 × H (00D7A3A0, qword `0x3FB999999A000000`; 009C5B8F
  FMUL, 009C5B97 FCOMIP, 009C5B9B JBE), 009C5BB1 calls 00438AA0.
* That call's first argument is **base+10h, the aim heading**, not e_imp. It is loaded by 009C5BAA
  `FLD [ESP+18h]` **after** 009C5BA3 `SUB ESP,8`, so [ESP+18h] there is base+10h. The second
  argument is π (00D7A264, float).
* Its result is discarded: 009C5BB6 `JMP 009C5BBA`, and 009C5BBA is `FSTP ST(0)`.
* Nothing between 009C5B54 and 009C5BBC stores to base+18h. 009C5BBC then reloads the unchanged
  e_imp.
* The branch is a dead call whose result is popped. A first reading of this packet took the
  [ESP+18h] at 009C5BAA as base+18h and bound a π flip of e_imp. That reading was wrong, and the
  binding was removed before commit. Its pair is in section 4.

**The aim error** 009C5BBC-009C5C9B, which the host already had as `dive_bomb_aim_error_009c5c9b`:
* The steep gate first: pitch > -30° [00CEC728] jumps to 009C5CEF, with pitch command -1.0.
* along = cos(+18h) × miss(+5Ch) - L, where L = 00419010(A8h + 100 [00D7A220 qword], 0,
  ACh + 50h, row+5Ch, H) (009C5C49, then 009C5C4E FSUBR).
* M = 00419010(A8h + 100, 1, ACh + 50h, row+60h, H) (009C5C92).
* error = M × along (009C5C97, stored to +5Ch).
* The pitch command cmd+29Ch is error × row+64h clamped at ≤ 1 when the error is positive, and
  error × row+68h clamped at ≥ -1 otherwise (009C5C9F-009C5CFA).

**The wide roll band** reads the same, unchanged `[ESP+18h]` at 009C5D33.

**The predicted impact point** is unchanged by this packet.
* 009C7D71 builds approach+D8h/+E0h as unit + (007BCC80(H, v.y) + 0.1) × v.
* The host's `dive_bomb_impact_point_009c7d71` and `db_impact_fall_time` follow that shape
  (`include/bsp/dive_bomb_task.hpp`, "009C7D94").
* Before the dive and the range latch, the host holds the point at the aircraft (lines 1823-1832,
  009C7D27).
* None of that decides the entry. The measured entry below has a live miss of 563-596 m, and the
  heading sweep does not depend on the fall time.

## 2. The host against the image at the entry (V1, `local\V1_9000.log`, D3A Val #1.1)

The Val comes out of the turndown still inverted. So 009C4F80's body-up arm (pitch -1.08, at or
steeper than -40° [00CE7D1C]) returns roughly heading + π.

| t | pitch | aimhdg | bearing to aim | \|e_air\| | range, 0.1H | e_imp (host) | cos e_imp | miss | host error |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1339 | -1.076 | 6.048 | 3.098 | 2.95 | 784, 84 | 2.855 | -0.96 | 563 | -294 |
| 1343 | -1.146 | 5.802 | 3.100 | 2.70 | 775, 82 | 2.634 | -0.87 | 596 | -304 |
| 1347 | -1.097 | 5.340 | 3.103 | 2.24 | 766, 80 | 2.158 | -0.55 | 573 | -211 |
| 1351 | -1.023 | 4.848 | 3.107 | 1.74 | 756, 78 | 1.585 | -0.01 | 539 | -37 |
| 1352 | -1.010 | 4.728 | 3.109 | 1.62 | 753, 77 | 1.435 | +0.14 | 534 | +11 |
| 1353 | -1.001 | 4.612 | 3.110 | 1.50 | 750, 77 | 1.291 | +0.28 | 530 | +59 |
| 1360 | -0.972 | 3.954 | 3.126 | 0.83 | 728, 73 | 0.594 | +0.83 | 483 | +257 |

**No divergence.**
* The host computes e_imp, the aim error, the pitch command and the wide roll band from the same
  slots as the image.
* At entry the Val is still inverted from the turndown, so 009C4F80's body-up arm returns about
  heading + π, and cos(e_imp) = -0.96.
* The image then forms error = M × (-0.96 × 563 - L) < 0 and commands pitch -1, exactly as the
  host does. The swing from -294 to +268 m is the aim heading sweeping from 6.05 to 3.64 as the Val
  rolls upright.
* **The swing is the image's law at this speed**, and nothing is bound.
* The 009C7D71/007BCC80 impact point is not involved. The entry miss of 563-596 m and the heading
  sweep decide it.

## 3. Predictions for the (misread) flip, written before the pairs

The flip is bound as `kAimDiveBehindFlipBound`. Each pair is built in this tree and differs only by
the switch. All runs are E2 at 9000 with `BSP_GUNNERY_RNG_STREAMS=1`.

* **P0/P1** use main's configuration (the fix and the tail off).
* **Q0/Q1** use the V1 set: the throttle fix, the tail, the glide pitch, the draw, the glide yaw
  and traces.

1. **Q1, the entry.** The Val #1.1 error is positive from the first aimdive tick. There is no
   -294 m trough, and no pitch -1 run at entry; the dive begins shallowing from t=1339 instead of
   steepening to -1.146.
2. **Q1, releases.** The final dive angle is set by where the CCIP meets the target at about 55
   m/s. The flip removes the 13-tick detour, not that geometry, so I expect a small move: 16 ± 4
   against Q0. The flip does not reach Mode B, which is upstream of the aimdive, so flights #3.1
   and #7.1 stay at 0. Mode A (the late wingman bank) happens at |e_air| < 0.35, where the flip is
   off, so it is also unchanged.
3. **P1, main.** Main's dives also start inverted, at about 110 m/s, so the flip acts at every
   entry. Aimdive rows move from the first tick. Releases stay near P0's 32 (±5), and water contacts
   stay near 8.
4. **The fighters** are unaffected in both pairs.

## 4. The pairs: evidence against the misread flip

The runs are E2 at 9000 with `BSP_GUNNERY_RNG_STREAMS=1`. Each pair was built in this tree from
main 7d1ff2166 and differs only by `kAimDiveBehindFlipBound`.

| run | log | configuration | drops | kill credits | water | mission end |
| --- | --- | --- | --- | --- | --- | --- |
| P0 | `local\P0_9000.log` | main | 32 | 34 | 8 | failed 228.06 s |
| P1 | `local\P1_9000.log` | main + flip | **1** | 36 | 0 | failed 234.06 s |
| Q0 | `local\Q0_9000.log` | V1 set | 16 | 37 | 7 | failed 297.05 s |
| Q1 | `local\Q1_9000.log` | V1 set + flip | 11 | 36 | 0 | failed 342.04 s |

* **Q1** matched prediction 1: the entry error is +244 m from t=1339. After that it does not.
  * The positive command steepens the dive to -1.385 rad.
  * The inverted Val (bank 3.12) rolls upright through -3.04 and -0.68 on a wide-band stick of -1,
    and its heading swings by about 1.5 rad.
  * It ends about 90° off the aim bearing. The miss grows to about 1100 m and the range opens from
    760 to 1023 m.
* **P1** loses 31 of main's 32 releases. Every flight is affected.
* Predictions 2 and 3 failed badly. That is consistent with the corrected read: the image leaves
  e_imp alone, and forcing the flip inverts both the pitch and the roll sense at every inverted
  entry.
* The binding was removed. The source is unchanged from main.

## 5. The G anomaly (secondary, one diff pass)

In `docs/DIVE_MODES.md`, run G (main plus `kDogfightMovetoGenericBound`) moved 46 dive-bomb rows
with the RNG streams decoupled. One diff pass of `local\G_9000.log` against `local\DC_9000.log`
restricted to Val lines finds:
* **The first moved rows are fighter gun bursts at flight #3.1.** In G,
  Yorktown-class01_sqn02|.-2 and the leader fire at D3A Val #3.1 (d = 845.9 and 674.7) where DC
  has none. A #3.1|.-3 rear gunner's first shot (DC t=110.20 s) moves with them.
* **Val #3.1's first numeric divergence** is its squadron formation geometry at tick 2000. The
  pairwise spacings differ in the first decimal, while its tick 0 and 1000 rows match.
* No impact on #3.1 precedes its dive in either run.
* The fly-over's near-field probe writes (007F0280 via 009C6B75) rise from 441 in DC to 493 in G.

That points at the fighters' new paths entering the Vals' own avoidance probe, the image's
coupling, not a shared draw. **It is not proven by one pass.** Proving it needs a per-tick trace of
the fly-over probe's hit list for D3A Val #3.1, to find the first probe hit that names a Yorktown
fighter. That would be one diagnostic line in the fly-over block and one G/DC re-run. It was not
done.

## 6. Decision

* **The entry swing is the image's law. No term is bound, and no switch lands.**
* The 009C5BB1 00438AA0 call is dead: its result is popped at 009C5BBA.
* The next term for the dive regime is therefore not in the aimdive. The dive enters inverted at
  54 m/s from the turndown, at 009C44F0 with the 34.5 m/s speed hold, and the image's aim law then
  behaves as measured. The candidates upstream are:
  * the turndown completion rule 009C7EA0, which ends the turndown while the aircraft is still
    inverted;
  * the turndown speed, 007C47F0 LevelFlight × StallSpd.
