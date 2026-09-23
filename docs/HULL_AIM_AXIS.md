# Hull aim axis: why dive-bomb releases fell from 23 to 8, and whether the offset can land

Addresses: 004142E0, 009FADA0, 009FA260, 00816650, 0042D810, 0042BB20, 00414DB0, 009C7A80, 009C7A9F, 009C7B32, 009C40A0, 009C62B0, 009C6342, 009C58D0, 009C5988, 009C59CD, 009C5B43, 009C60DE, 009C5180, 009C5278, 00825F20, 00826866

Packet `cc9_hull_axis`, 2026-09-22. It follows `docs/HANDOFF_HULL_AIM_POINT.md` section (f).
The report is `reports/hull_aim_axis.json`. All names are hypotheses, not recovered symbols.

**Result.** Candidates (i) and (ii) from the handoff are measured clean. The image does
measure every dive-bomb gate to the hull point, so candidate (iii) is true as a fact about the
image. But the gates are not "tuned for the origin": every gate this host binds already takes
the fed point. A fourth finding came up. Three more image sites read the hull point, and this
host still feeds them the origin. Feeding them does not bring the releases back. The loss grows
with the size of the horizontal offset and not with its height. It is not yet traced to a
single input. `kHullAimOffsetEnabled` **stays false**.

## 1. What the live run shows

Source: `J:\PROG\battlestations-pacific-decompile-cc8-hull-aim\local\hullaim_before_usn04.log`
(switch absent) against `hullaim_after2_usn04.log` (switch on, base `ec14870c3`). Run A below
reproduces the after2 task rows line for line on base `1cc9d3dea`, and prints each draw. So the
drawn columns come from run A.

Per-aircraft `divebomb` task rows. `drawn` is the body-frame offset along the hull and across it,
in metres. `end` is the final state.

| aircraft | target | drawn along / across | before: releases, end | live: releases, end, states |
|---|---|---|---|---|
| movieval | Lexington | +19.2 / -2.0 | 2, done | 2, done |
| movieval\|.-2 | Lexington | -101.6 / +2.4 | 2, done | 0, goaway (aimdive exit by pull-out) |
| movieval\|.-3 | Lexington | -102.6 / -4.9 | 2, done | 0, goaway (pull-out) |
| D3A Val #1.1 | Lexington | -55.4 / -6.3 | 2, done | 1, goaway via aimglide (abort 009C5B43) |
| D3A Val #1.1\|.-2 | Lexington | -84.6 / -3.1 | 2, done | 0, goaway via aimglide (abort) |
| D3A Val #1.1\|.-3 | Lexington | +61.9 / -12.9 | 2, done | 2, done (abort fired, still released) |
| D3A Val #3.1 | Yorktown | +47.5 / +11.9 | 2, done | 0, goaway via aimglide (abort) |
| D3A Val #3.1\|.-2 | Yorktown | +66.8 / +1.0 | 2, done | 0, goaway via aimglide (abort) |
| D3A Val #3.1\|.-3 | Yorktown | -65.8 / -8.2 | 2, done | 2, done |
| D3A Val #5.1 | Lexington | +62.6 / +7.8 | 1, in aimdive at mission end | 1, same |
| D3A Val #5.1\|.-2 | Lexington | -103.8 / +3.9 | 0, in aimdive at mission end | 0, same |
| D3A Val #5.1\|.-3 | Lexington | +64.0 / +7.8 | 0, in aimdive at mission end | 0, same |
| D3A Val #7.1 | Yorktown | -111.8 / -1.4 | 2, done | 0, goaway (20 transitions) |
| D3A Val #7.1\|.-2 | Yorktown | -27.4 / +10.3 | 0, goaway | 0, goaway |
| D3A Val #7.1\|.-3 | Yorktown | +53.5 / -13.4 | 2, done | 0, goaway (pull-out) |

The abort at 009C5B43 fired 2 times in the before-run and 6 times live. Every failed dive has
the same shape. Its predicted impact makes its closest pass at the aim point early, at 420 to 796 m altitude in
run A. That is far above the release floor of about 350 m. In run B the pass comes within 0.4 to
5.5 m. By the time the aircraft is below the floor, the last signed along-track error is 76 to
670 m in run A. The dive then ends in a pull-out or an abort.

**Do the bombs that land fall near their drawn point?** Not reliably. Each landed bomb in run A
below is paired with its own aircraft's draw. The miss is given in the target's course frame. The
Lexington is stationary, so its course is the hull axis.

| aircraft | drawn along | impact along | impact across | note |
|---|---|---|---|---|
| movieval | +19.2 | +22.4 / +14.0 | -33.2 / -19.9 | lands at its point along the hull |
| D3A Val #1.1 | -55.4 | -26.7 | +28.3 | 29 m toward the origin |
| D3A Val #1.1\|.-3 | +61.9 | +20.0 / +16.3 | -41.5 / -14.0 | 42-46 m toward the origin |
| D3A Val #3.1\|.-3 | -65.8 | -86.1 / -63.6 | -58.1 / -40.3 | the impact row names York-class02, not the drawn Yorktown-class01, so the pair is not comparable |

**Spread along the hull within a squadron.** The before-run's twelve Lexington bombs land at -0.8 to
+16.1 m along the hull, a 17 m band around the origin. Live, the five Lexington bombs span
-26.7 to +22.4 m, a 49 m band. So there is some scatter, but it covers the 7 survivors of 23 and not the
draws, which span -104 to +64 m. The release gate at 009C60DE tests only the along-track
projection of the miss, minus an authored lead. A dive whose point sits far from the origin
either fails that gate or passes it with a large residual.

## 2. Candidate (ii): the axis convention

**The image is row-vector, and body z goes through row 2.** 004142E0 takes ECX as the source
point, then the output pointer and the matrix on the stack:

```
004142FE  FLD [ECX+10h]   ; M[4]  * y
00414308  FLD [ECX]       ; M[0]  * x
00414316  FLD [ECX+20h]   ; M[8]  * z
00414325  FADD [ECX+30h]  ; + M[12]
00414328  FSTP [EAX]      ; out.x      (out.y: +04h/+14h/+24h/+34h, out.z: +08h/+18h/+28h/+38h)
```

So `out = x*row0 + y*row1 + z*row2 + row3`, with row n at matrix `+10h*n`. At 009FAED0 to 009FAEDF,
009FADA0 passes `target+CCh` as the matrix, after 00414DB0 refreshes it, and `sub+28h` as the body
point. The pick 00816650 writes `class+A4h` (Width) into body x and `class+A0h` (Length) into body z.
So the image puts the hull length on **row 2 of the target's world matrix**, at unit+ECh..F4h.

**Row 2 is the forward axis in the image.** At 00826866 inside 00825F20, the image multiplies
`class+A0h` by `[EBP+20h..28h]`, which is row 2 of the matrix at EBP. The keel point in
`include/bsp/ship_motion.hpp` takes that row as `row_forward`.

**The host matches.** `publish_pose` copies `motion.pose_row2` into `world[8..10]`, and
`transform_point_004142e0` in `src/camera_affine.cpp` is the image's own x87 sequence.

**Measured.** Run A prints each draw with the target's rows. The Yorktown's row 2 is
(-0.770, 0.003, 0.638), which is a heading of atan2(-0.770, 0.638) = -0.879 rad. Its velocity
heading is -0.881 rad. The Lexington's row 2 heading is 1.012 rad, the same as the course the
impact census reports. In every draw, the world offset resolves back to `along = body z` and
`across = body x` to 0.01 m. **Candidate (ii) is clean.**

## 3. Candidate (i), the runs, and the fourth finding

All runs used the before-log's parameters, and each ran its own binary copy:

```
./tools/run_game.ps1 -Exe local\bin_<v>\bsp_game.exe -Log local\hullaxis_<v>_usn04.log -- --frames 5000 --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05
```

| log (`local\`, cc9-hull-axis tree) | build | releases | bomb impacts | torpedo swims |
|---|---|---|---|---|
| `hullaxis_off_usn04.log` | switch off (control on base 1cc9d3dea) | 23 | 20 | 12 |
| `hullaxis_on_usn04.log` (run A) | switch on, prints | 8 | 7 | 12 |
| `hullaxis_b_usn04.log` (run B) | A + the three unfed sites fed | 8 | 6 | 12 |
| `hullaxis_s01_usn04.log` | B, offset x 0.01 | 23 | 20 | 12 |
| `hullaxis_s10_usn04.log` | B, offset x 0.1 | 23 | 20 | 12 |
| `hullaxis_s50_usn04.log` | B, offset x 0.5 | 16 | 13 | 12 |
| `hullaxis_y0_usn04.log` | B, full offset, height zeroed | 6 | 5 | 12 |

The control's task rows equal the before-log's rows exactly. Run A's rows equal the after2 rows
exactly. So the prints do not change behaviour, and the base moved nothing on this path. The
0.01 and 0.1 runs reproduce the control's release count for every aircraft; their state
tick counts differ from the control's by at most 9 ticks.

**Candidate (i) is clean.** Run A has 15 `hull_aim draw` lines for 15 aircraft, one per
(attacker, target), and no second draw. All 15 `hull_aim inrange` lines fire at 2072.6 to
2079.2 m from the aim point, the range where the attackrun hands over to the fly-over.

**Fourth finding: three unfed image sites.** The dive-bomb approach's vtable slot 0 is
009C40A0, which has no Ghidra function; it copies approach+4Ch..54h, the hull point. The image
calls it at five sites:

| site | routine | what it builds | host before this packet |
|---|---|---|---|
| 009C7B32 | 009C7A80 | range approach+BCh | fed |
| 009C5988 | 009C58D0 | aimdive first sqrt | fed |
| 009C6342 | 009C62B0 | fly-over 3-second lead point | **origin** |
| 009C59CD | 009C58D0 | aimdive height [ESP+14h] | **origin height** |
| 009C5278 | 009C5180 | aimglide height | **origin height** |

Run B feeds the last three. With them fed, the turndown and dive-entry ranges and bearings
measured to the hull point match the control's values measured to the origin (1223-1282 m, and
0.06 or 0.18-0.24 rad). But releases stay at 8. Those three edits are outside this packet's
hunk and are **not committed**. The diff is kept at `local/hullaxis_experiments.diff`.

## 4. Candidate (iii): what the gates measure

In the image every gate reads the hull point, never the target origin:

* **Range latch approach+BCh.** From 009C7B34 to 009C7B56 it subtracts unit+FCh/100h/104h from the
  point that `vtable[0]` returns at 009C7B32.
* **25 m gate at 00CE3880.** At 009C60DE to 009C60EC it tests |error|. The error comes from
  009C5C9B as gain x (cos(bearing) x distance - lead). The distance and bearing are taken from
  the predicted impact approach+D8h/E0h to the `vtable[0]` point. The lead and gain are
  interpolated on the height from 009C59CD, which is also measured to the `vtable[0]` point.
* **Abort at 009C5B43.** At 009C5B01 to 009C5B3E it compares that same height [ESP+14h] against
  approach+D4h + approach+50h. Here approach+50h is the hull point's y. It also compares
  0.3 x height + 150 against [ESP+1Ch], the first sqrt, measured to the `vtable[0]` point.

So in the image, the gates do move with the point. The host binds every one of them to the fed
point, and run B also feeds the heights. **Candidate (iii), read as "downstream gates tuned for the
origin", does not survive:** no gate in this host still measures to the origin.

## 5. Decision, and what survives

**The switch stays FALSE.** Releases never came back to 23 with the offset at full size, so the
landing condition was not met. The committed change is only a print behind the switch: one
`hull_aim draw` line per draw and one `hull_aim inrange` line per (attacker, target). Each line
carries the offset, the world point, the target origin, rows 0 and 2, and the along and across
distances.

**What survives** is a host-side sensitivity that is not yet localised:

* The loss scales with the **horizontal** offset: 23 releases at 1% and 10% of the offset, 16 at
  50%, 8 at 100%. Zeroing the height leaves 6, so the height term is not the cause.
* With every phase keyed to the hull point, the geometry at dive entry relative to that point
  matches the control's geometry relative to the origin. So the loss is not an input still
  measured to the origin.
* A per-tick trace of movieval|.-2 compared the control with run B. Both use the same entry
  range, 704 against 712 m, and the same early error swing, -200 to +220 m. They part at the
  predicted impact's first pass. The control passes 25.5 m from the point, then settles to 16 m
  at the release floor. Run B passes 48 m to the side, where 29 m from the origin, and stays
  95 to 120 m off.
  The trace was a local print and is not committed; it is in `local/hullaxis_experiments.diff`.

The next step is to extend that per-tick trace through the turndown for this one aircraft, in both
runs. It should print the roll command, heading, bank, lead bearing and predicted impact. The
first tick where a relative input differs is the input still bound to an absolute frame.

## Correction to earlier docs

`docs/HANDOFF_HULL_AIM_POINT.md` (f) calls (ii) "the strongest candidate". It is measured clean,
and a correction section is appended there.

## Correction from docs/HULL_AIM_TURNDOWN.md (2026-09-22, packet cc9_hull_turndown)

* Section 3 says 009C40A0 "has no Ghidra function". That is wrong: it is a defined function named
  `dive_bomb_approach_aim_point_009c40a0`. This packet's lookup index was stale. The
  `no_ghidra_function` row in `reports/hull_aim_axis.json` is wrong for the same reason.
* Section 5 calls the surviving candidate "a host-side sensitivity that is not yet localised". It
  is now localised. The builds leave the fly-over within a few metres and milliradians of each other
  relative to their own aim points. The aimdive amplifies that at the first pass of the predicted
  impact, through the roll input that steers on the bearing from the impact to the aim point. The
  three sites this doc listed as unfed are fed as of commit `4341d8f55`.
