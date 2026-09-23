# Hull aim turndown: where a horizontal hull offset turns into lost releases

Addresses: 009C40A0, 009FADA0, 009FB200, 009FB190, 009C7A80, 009C7A9F, 009FAEDF, 004142E0, 009FA2E0, 009C62B0, 009C6342, 009C58D0, 009C59CD, 009C5B43, 009C5C9F, 009C5D8E, 009C5180, 009C5278, 009C44F0

Packet `cc9_hull_turndown`, 2026-09-22. It follows `docs/HULL_AIM_AXIS.md`. The report is
`reports/hull_aim_turndown.json`. All names are hypotheses, not recovered symbols.

**Result.** No host computation reads the target origin where the image reads the aim point. The
aim point itself is bound correctly. The two builds leave the fly-over within a few metres and a
few milliradians of each other, measured relative to their own aim points. That small difference
comes from the approach path, because the aircraft spawn at fixed places while the point moves.
The aimdive then amplifies it. Its roll input is the bearing from the predicted impact to the aim
point, which is singular as the two pass each other. Its pitch loop swings the signed error by
±200 m. **The switch stays false.** The divergence is a sensitivity of the dive loop, not a
defect in the hull-aim hunks.

## 1. Fidelity: the three reads now take the fed point

Commit `4341d8f55` routes three reads through `hull_aim_world_point`: 009C6342 (fly-over lead
point), 009C59CD (aimdive height) and 009C5278 (aimglide height). So they read the same point as
the range latch and the aim error: the origin while `kHullAimOffsetEnabled` is false.

The neutrality run is `local\feedoff_usn04.log`, switch off, on the before-log parameters:

```
./tools/run_game.ps1 -Exe local\bin_<v>\bsp_game.exe -Log local\<v>_usn04.log -- --frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05
```

All 3,742 census lines, the summary and per-unit rows, are identical to
`local\cc9\hullaxis_logs\hullaxis_off_usn04.log` in the main checkout. The one extra line is
`summary mission gunnery ordnance_rearms`, which packet cc9_gunnery_host added on main after that
log was taken. The refills counter does not appear in either log.

## 2. What the image's aim point is

`009C40A0` copies approach+4Ch, +50h and +54h, as floats, to out[0..2]. Those three fields are
the target-reference sub-object's +1Ch, +20h and +24h, since the sub-object sits at approach+30h.
The image has two writers:

* **Every approach tick.** 009C7A80 calls 009FADA0 at 009C7A9F, as its first act. At 009FAEDF,
  009FADA0 calls 004142E0 with ECX = sub+28h (the body-frame offset) and the matrix at target+CCh.
  It stores the three result floats with `MOVSS dword` at 009FAEEA, 009FAEF5 and 009FAF00.
* **Construction.** 009FB32A-009FB346 copy target+FCh/100h/104h, the origin, into the three
  fields with `FLD`/`FSTP float ptr`. Then 009FB357 calls 009FB190, and 009FB364 calls 009FADA0
  with dt = 0.0 (009FB35C `FLDZ`). So the hull point is already in place before the first tick.
  With no target, 009FB395-009FB3A1 store the seed from 00F87574, which is loader zero-fill.

**The aim point is the target's live world matrix times a fixed body-frame offset.** It has no
velocity term and no time term. 009FA2E0, the target velocity getter, and the 3.0 s lead (the
qword at 00D7A2B0) belong to the fly-over's own lead point at 009C62ED-009C6351. That lead is
computed from the aim point, not stored into it. The host's `hull_aim_world_point` computes the
same thing: the live `world` times the drawn offset, with no lead.

The census of writers covers the whole listing of 009C7A80, which has no store to
+4Ch/+50h/+54h. It also covers the eight approach vtable slots, 009C3EA0, and the sub-object
methods 009FA260-009FB3C0. A whole-image census of disp8 stores against an approach base was
not run. `docs/HANDOFF_DIVE_BOMB_GOAWAY.md` (c) asked for this read for the lead question: the
aim point is not led.

## 3. The turndown trace

The switch `kHullAimTrace` (default false) prints one line per turndown, aimdive and aimglide tick
from `update_dive_bomb_approach`. Each line carries position, velocity, predicted impact, heading,
pitch, bank, the commands, the range, the latch, the error and the abort count, all relative to
the fed aim point. The traced runs are `local\troff_usn04.log` (switch off, 23 releases) and
`local\tron_usn04.log` (switch on, 8 releases). The comparator is `local\cmp.py`. It aligns each
aircraft on its own turndown-entry tick and rotates everything into the entry heading. That
cancels both the offset and the approach direction.

movieval|.-2, control and live. `i` is the tick index from turndown entry. Positions are metres
from the aim point: `fwd` along the entry heading, `side` across it. `dh` is heading change in
radians.

| i | state | fwd | side | dh | bank | impact fwd / side | error |
|---|---|---|---|---|---|---|---|
| 0 | turndown | -1220.0 / -1220.1 | -72.2 / -73.6 | 0 / 0 | 0.008 / 0.008 | -152 / -154, -73 / -74 | - |
| 12 | turndown | -1096.5 / -1096.7 | -74.6 / -77.3 | -0.029 / -0.053 | 0.971 / 0.975 | 127 / 115, -128 / -158 | - |
| 60 | aimdive entry | -677.1 / -677.4 | -101.6 / -114.8 | -0.117 / -0.146 | -2.85 / -2.76 | -411 / -396, -132 / -152 | -220 / -193 |
| 84 | aimdive | -520.3 / -508.8 | -117.2 / -132.0 | 0.218 / 0.202 | -0.20 / -0.29 | -108 / -47, -34 / -44 | 121 / 72 |
| 96 | aimdive | -392.7 / -379.6 | -86.5 / -104.6 | 0.241 / 0.109 | 0.04 / -0.69 | 91 / 96, 32 / -32 | -107 / -101 |
| 108 | aimdive | -277.9 / -259.8 | -58.0 / -102.4 | 0.239 / 0.004 | 0.26 / -0.96 | -18 / 52, 6 / -108 | 21 / 91 |

**First divergence.** At i = 0 the builds differ by 1.4 m sideways and by 1.5 m in height. The
height difference is the drawn aim point's height. The leftover fly-over heading command differs
by 0.0009 rad. From i = 1 the live aircraft yaws 0.002 rad per tick more, with the bank still
identical, so the difference is yaw rate carried in from the fly-over. The turndown 009C44F0 is
attitude-only in this host and has no target input. It lets that yaw carry through, so the sideways
difference reaches 13 m at aimdive entry. movieval|.-3, #3.1 and #7.1|.-3 show the same shape. At
i = 0 they differ by 0-7 m. In #3.1 and #7.1|.-3 the point is still 230-270 m to the side at
turndown entry in both builds.

**Amplification.** The split becomes large at i = 84-108, when the predicted impact first passes
the point. While the error is positive and |bank| is under 60°, roll comes from
`bearing_error_wide_18`: heading less the bearing from the predicted impact to the aim point
(009C5D33, and the map at 009C5D8E). That bearing is undefined when the two coincide, and it turns
through π as the impact passes. Otherwise roll comes from the unit-to-point bearing. At i = 84 the
live impact is 46 m short and 44 m to the side of the point, a bearing of about 45°. The control's
is 107 m short and 34 m to the side, about 18°. So the live aircraft rolls to -0.7..-1.1 rad and
its impact swings 100 m sideways. The pitch loop at 009C5C9F clamps ±1 on error x gain, and the
error swings -220 to +219 m before either build reaches the release floor.

**Which gate fires.** The live run loses its releases to the abort at 009C5B43 in eight aircraft,
and to a pull-out in #7.1|.-3. It is the same abort that `docs/HULL_AIM_AXIS.md` counted.

## 4. Decision

None of the four suspected defect classes is present:

* **World-frame vs target-frame mix:** none. Everything is measured to the fed point.
* **Lead on the origin but not the point:** none. The image applies no lead to the aim point, and
  the fly-over lead is built from the fed point.
* **Clamp tuned for the origin:** none. The clamps are the aimdive's own ±1 pitch and its roll
  bands.
* **Gate reading the origin another way:** none. After commit `4341d8f55`, all five `vtable[0]`
  reads take the fed point.

The loss is the dive loop's sensitivity to its entry state. It is not in the hull-aim hunks, so
there is nothing there to fix, and **`kHullAimOffsetEnabled` stays false**.

The control's 23 releases are themselves a weak yardstick. Its twelve Lexington attackers enter
the turndown on nearly the same geometry: 1225-1232 m and ±0.0604 rad in `hullaxis_off_usn04.log`.
So they fly one dive repeated. The 1% and 10% offset runs kept 23; the 50% run lost 7. That places
the sensitivity threshold at entry differences of well under a metre to a few metres.
Whether the image's own dive is this sensitive cannot be read from the listing.
The open question is whether this host's aimdive attitude response matches the image.
That is the pitch gains at approach+64h/+68h, the plane's roll and yaw response, and why the
fly-over leaves the point 72 to 270 m to the side at turndown entry. That is a separate packet,
outside the hull-aim region.
