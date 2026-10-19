# cc13-mvspeed: census row 20, 009C1850 the moveto desired speed

Packet cc13-mvspeed, from docs/CC13_STUB_CENSUS.md row 20 ("the speed is bound, only the shaping
is substituted", 7 missions / 321,246 calls). Names are hypotheses, not recovered symbols. No game
runs. Logs: current-main OFF runs of 2026-10-19 in the cc9 trees.

## Verdict

- **The shaping is not substituted.** 009BECD0's blend is reconstructed and matches the listing,
  and so are 007EF2C0 and 007C47F0. The census count comes from
  `owner_.record("BotStateMoveTo::set_desired_speed")`, which logs "UNIMPLEMENTED" at three sites
  that do run the bound body. The census note repeats a stale comment.
- **The real difference is the separation the speed is shaped by.**
  - Torpedo site: LIVE in code. The host feeds approach+90h, the range to the hull aim point, and
    skips the write when that is not positive.
  - Every site: the sep is computed in double where the image uses float at PC24 (ulp level).
- **New OFF switch** `kMovetoSpeedShapingCc13Bound`, with a static_assert on
  `kMovetoSpeedBlendBound`.

## The image

**009C1850** (vtable 00D20AEC +1Ch) is `__thiscall(state; float sep)`, RET 4:
- 009C1856-009C1866: a = `[[state+4]+0Ch]+3A0h`, the squadron's travel speed. There is no null
  test on +0Ch.
- 009C186A: b = 007C47F0(`[[state+4]+8h]`), LevelFlight × StallSpd.
- 009C1895: cmd+2B4h = 009BECD0(state; a, b, sep). 009C18A0 clears cmd+2B0h, and 009C18A7 sets
  cmd+2D8h = 1.

**009BECD0** (`__thiscall(state; a, b, sep)`, RET 0Ch):
- k = 00419010(tuning+5CCh WingmenWaitDist1 → 0, tuning+5D0h WingmenWaitDist2 → 0.5 [00CE3800],
  sep) (009BED0F).
- If `[state+4]+0Ch` and `+8h` are both set: m = k when k > 007EF2C0(squadron), else the wingmen
  value (009BED35-009BED4C, FCOMIP/JBE). The result is b + (a - b) × m (009BED4C-009BED66, one
  FSTP float).
- Otherwise the result is a (009BED72).

**007EF2C0**:
- Returns 1.0 when the squadron's count (+3CCh) is ≤ 1 or its formation shape (+3E4h) is 0.
- Otherwise it takes the minimum of the non-negative values members 1..n-1 published at
  unit+9C4h[step index × 8]. 007BCC20 publishes these at 007CDCE7 each step.

**Draws:** none. 009C1850, 009BECD0, 00419010, 007EF2C0 and 007C47F0 make no call to 00BD2F10.

**The sep** comes from 009C18C0 (the moveto tick that calls +1Ch):
- 009C18CB: ESI = [state+2Ch], the moveto target. When it is null, sep stays 0 (009C18D0) and
  009C18D6 jumps straight to the speed call.
- 009C18EC-009C1913 copy the target's origin X/Y/Z (after the 00414DB0 pose refresh). 009C1919
  takes the unit at [[state+4]+4].
- 009C192C-009C1958: dx and dz are each FSTP float. d2 = dz·dz + dx·dx is FSTP float (PC24).
- 009C195C-009C196A: d2 above double 1e-10 (00CE3820) or 0. 009C196C calls 00BF7030 (sqrt at
  PC24) and FSTP float.
- 009C198A-009C1999: [vt+1Ch](sep), **unconditional**, before 009C19A1's pending-release return
  and 009C19EB's no-target return.
- The same sep feeds the glide: the denominator at 009C1A71 (`sep - 1000`) and rangeHigh at
  009C1B01.

At PC24, each FSTP float of an x87 result equals the float operation. sqrtf is correctly
rounded, as fsqrt at PC24 is. A double-precision host gives the same value up to one rounding of
d2 and the root: 53 bits against 24.

## The host sites (switches as on main)

| site | caller (state) | sep the host passes | difference |
|---|---|---|---|
| units.cpp `st_moveto_tick_009c18c0` | strafe-task moveto | target origin, double | PC24 only (ulp) |
| `lb_moveto_tick_009b6d20` | level-bomb moveto (009B6D20 → 009C18C0) | target origin, double | PC24 only |
| `kz_moveto_tick_009c18c0` | kamikaze moveto | target origin, double | PC24 only |
| `run_dive_bomb_move_to_tick_009c18c0` | dive-bomb moveto | `db_planar_bc` (hull point) while `kDiveBombMoveToOriginBound` is OFF; origin, double, when ON | the point is that switch's matter; PC24 when ON |
| `run_move_to_tick_009c18c0` | torpedo moveto | approach+90h `range_90`, the range to the 009FADA0 hull aim point; returns before the speed when it is not positive | **wrong point; skipped write** |
| `run_land_moveto_tick_009c18c0` | landing moveto | landing-site origin, double | PC24 only |
| `run_moveto_circle_tick_009c26d0` | kind-7 circle | 0 (009C26D3 passes 0) | none |

The kind-7 moveto task tick 009C2430 (units.cpp, `moveto_speed_009c1850(..., true)`) calls 009BECD0
itself, not 009C1850, and computes its own distance. It is out of this row. The dogfight moveto
uses 009C1BC0, not 009C1850.

Inputs a and b are the same at every site: `squadron_speed_3a0_live` (the class TravelSpeed ×
NewTravelSpeedMul, or the script-set squadron value) and `bot_desired_speed_007c47f0`. Using the
unit's own class for the squadron's class is labelled in the existing code.
- The host returns a when the registry has no squadron for the unit. The image always has one,
  because 009C1850 dereferences +0Ch unchecked. For a one-member squadron 007EF2C0 gives 1.0 and
  the blend also returns a.
- Timing: the host computes the wingmen value live at the leader's think. The image reads the
  value each member published at its last step. The difference is a sub-step.

## Reach

**The speed.** k depends on sep only for 3000 < sep < 5000 (WaitDist1/2). The blend moves only
when the wingmen value is below k (a lagging wing).
- |range_90 - sep| is at most the planar hull offset |aim - origin|. The hull_aim lines
  (local/mv_hull.py) give mean 40.0 m and max 68.7 m in USN21 (533 samples, l70r_n_vu21_main_ref),
  and mean 39.2 m and max 76.1 m in USN01 (97, l70r_n_u1s_main_ref).
- So Δk ≤ 0.5 × 76/2000 ≈ 0.019. a - b is not logged for the torpedo planes. The land summary's
  a=97.78 and b=34.77 give a - b ≈ 63 m/s, so the effect is of the order of 1 m/s at most. It
  applies only inside that band, with a waiting wing.
- The torpedo moveto ends no nearer than the attackrun entry: the first `descent census` line
  of each run (n=1), which comes from the attackrun tick. That range is at least 3571 m in USN21
  (97 entries) and 3351 m in USN01 (8 entries). So the band can be the end of a run. The sampled
  moveto lines themselves (`glide census`, below) never go under 7765 m.
- The logs carry no per-tick wingmen value, so the per-call size is bounded, not measured. The ON
  switch prints `summary torpedo moveto sep` to measure it (below).

**The glide.** In the same tick the host's glide used range_90 for the 009C1A71 denominator and the
009C1B01 rangeHigh.
- **Correction (packet cc13-c25h).** An earlier version cited the `torpedo ... descent census`
  lines here. Those come from the attackrun tick (009D0951, game_hosts_units.cpp `descent
  census`), not from moveto.
- **Direct evidence.** The moveto tick's own line is `torpedo ... glide census`, logged every 50
  speed commands. In USN21 there are 1407 lines, all with commanded = 500.0, at a minimum range
  of 7765.3. USN01 logs none.
- **The end of moveto, by inference.** The glide leaves the 500 m cap only when (range - low) is
  under (500 - base) / (t × gain):
  - base is 5 to 6 m on every glide census line;
  - the census's `t` is the 009FBA50 scale (`cin.scale = t`), 1.600 on every line, and the gain
    is 0.727, so the slope is about 1.16 and the cap lifts only below range - low ≈ 425;
  - even at the scale's floor of 0.4 the threshold is about 1700.
  At attackrun entry (descent n=1), range - low is at least 2353 in USN21 and
  2715 in USN01. The range only shrinks on the way in, so the glide stays capped through the
  whole moveto.
- **Result.** The glide output does not move in these missions. The same holds for row 15's
  target lift (docs/CC13_TORPRANGES.md).

**The other sites** differ in the last bit of sep. Through k that is at most an ulp of speed.

## The switch

`kMovetoSpeedShapingCc13Bound` (OFF; `static_assert(!ON || kMovetoSpeedBlendBound)`):
- **`moveto_sep_009c18ec(unit, target)`**: 0 with no target. Otherwise float dx and dz, float d2,
  `!(d2 > 1e-10)` → 0, else `std::sqrt(float)`.
- **The strafe, level-bomb, kamikaze and land sites**, and the dive site's origin branch (still
  under `kDiveBombMoveToOriginBound`), take that sep.
- **The torpedo tick:**
  - It forms sep from the task target, the slot `torpedo_task_target_plus_one`; 009D2E55 passes
    the same target to 009C2AC0.
  - It writes cmd+2B4h/+2B0h/+2D8h first, unconditionally, as 009C198A does.
  - It returns when there is no target, as 009C19EB does. That return skips the glide, but not
    009C1B69's command stores, which the host's task (always targeted) does not reach.
  - Otherwise it runs the glide with the same sep.
- **Diagnostic, ON only:** `summary torpedo moveto sep` gives
  - n;
  - mean and max |sep - range_90|;
  - how many calls the old sep would have given a different speed, and the largest difference
    (a shadow 009BECD0 call on the stored a, b, wait distances and wingmen value; no side effect);
  - how often range_90 was not positive.

**OFF is unchanged.** The only additions are three diagnostic fields that `moveto_speed_009c1850`
stores (w1, w2, has-squadron) and the counters. Neither changes a value or a log line with the
switch OFF.

**ON data lifetime.**
- Every input is read live in the call: the unit's and target's positions, the target index set
  at task install, a and b, and the wingmen values.
- Nothing is stored for a later tick except cmd+2B4h, which the image also overwrites every tick.
- The shadow fields are written in `moveto_speed_009c1850` immediately before the diagnostic reads
  them.
- The torpedo target is `torpedo_task_target_plus_one`. A changed command target retires and
  re-installs the task (`kAttackTaskTargetRetireBound`).
- Uncertainty: the image's [state+2Ch] is an observer link, cleared if the target object dies
  (not read). The host keeps the last position. See docs/CC13_TORPRANGES.md.

Both settings were built (the ON build with the switch flipped locally, then reverted). The
committed state is OFF. The flip is cc9's call. With the speed only and a lagging wing, the
expected effect is of the order of 1 m/s of commanded speed at most, near the end of a torpedo
moveto.

## Status

Reconstructed under an OFF switch and build-tested (ON and OFF). Not game-validated.
