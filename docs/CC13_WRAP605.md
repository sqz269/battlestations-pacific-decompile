# The ship AI heading-store wrap `00605070` (packet cc13-wrap605)

Switch `kShipAiHeadingStoreWrapCc13Bound` (`include/bsp/cc13_heading_wrap.hpp`), landed OFF.
It closes rank 7 of the cc9 ship AI open items, which the `cc9_heading_wrap_census` (cc9-ships9,
2026-09-28) measured but left without a switch.

## The image

Four ship AI bodies store a heading at `blk+1D8h` and then wrap it in place with `00605070`
(`ECX = &blk+1D8h`, read from the disk bytes 2026-10-19):

| store | wrap call | body | host reconstruction |
| --- | --- | --- | --- |
| `009DFF7D` | `009DFF81` | `009DFF40` set heading, drop path | `ship_ai_set_heading_drop_path_009dff40` |
| `009E00F6` | `009E00FA` | `009E00A0` hold heading and stop | `ship_ai_hold_heading_and_stop_009e00a0` |
| `009E007F` | `009E0083` | `009E0040` set desired heading | `ship_ai_set_desired_heading_009e0040` |
| `009F335C` | `009F3360` | the attackmove approach | `ShipAiApproach::set_brain_heading` / `wrap_brain_heading` |

The census named the setter as `009DFFB0`. That body (the rudder setter) has no call to
`00605070`; the wrap in the setter family is `009E0083` in `009E0040`. Ghidra's xrefs to
`00605070` list none of the four sites; they were read with `bsp.py disasm-raw`.

**The other `00605070` sites on ship AI headings** (status from rev4's review), all outside this
switch:

| site | status |
| --- | --- |
| `009DE268` | dead code |
| `009DE2BD` | in `009DE280`, reached only from the dead `009E7FA0`; already wrapped in `cc13_seg67b.cpp` (`ship_ai_set_navigation_goal_with_penalty_009de280`) |
| `009E1DB4` | the land path; already wrapped unconditionally by `wrap_blk_desired_heading_00605070` (`game_hosts_ship_ai.cpp`) |
| `009E237A` | `009DFF40`'s body inlined with the kamikaze heading; covered through `009DFF40` / `HeadingHoldBinding` |
| `009E76F4` | the ring scan; it wraps nested+1214h (the wobble phase), not `blk+1D8h` |

`00605070` (`00605074..006050B9`): `fmod(*angle, 2pi)` through the CRT `00BF857A` with the
double `00CE3828`, stored as a float; then `+2pi` when the double `-pi` at `00CE3D18` is not
below the result (`FCOMPI` / `JB`), else `-2pi` when the result is above the double `pi` at
`00CE3D28` (`FCOMI` / `JBE`). The result lies in (-pi, pi]. The host's
`ship_ai_firepower_wrap_angle_00605070` (`src/ship_ai_bearing_rating.cpp`) is this body; the land
path's `wrap_blk_desired_heading_00605070` already applies it unconditionally.

## The host, OFF

Three hooks cover the four sites:
- `after_heading_stored_00605070` in `SetterBinding` covers `009E0083`;
- `after_heading_stored_00605070` in `HeadingHoldBinding` covers `009DFF81` and `009E00FA`;
- `wrap_brain_heading_00605070` in `ApproachStepBinding` covers `009F3360`.

They only count the stored value (`count_heading_wrap_store`) and record the call. The field keeps
whatever was stored.

The land path (`009E1DB4`, `wrap_blk_desired_heading_00605070`) counts after its own wrap, so its
stores are never out of range. The census therefore measures only the four sites above.

## ON

The same three hooks wrap `ctl_.blk.desired_heading` in place with
`ship_ai_firepower_wrap_angle_00605070`. Every caller of the three step bodies passes
`ctl_.blk` together with a binding built on the same `ctl_`, including the cruise chain
(`game_hosts_ship_ai.cpp` passes `ctl_.blk, setters` to it). The census counter still counts
the value as stored, before the wrap, so its out-of-range count stays comparable across modes.

**Data lifetime.** The wrapped value is read by:
- `009ED947` (`ship_ai_states.cpp`), which copies it to `blk+324h` in steering modes 1 and 3;
- the snapshot save and restore of the control block, which copy it unchanged;
- the per-row log (`row.desired_heading`), which prints it.

Every reader of `blk+324h` reached from that copy goes through `wrapped_angle_add_00438aa0` /
`wrapped_angle_subtract_00438b10` (which loop by 2pi into (-pi, pi]), through `cos`/`sin`
(`ship_ai_neighbour_clips.cpp` target bearing, `ship_ai_clearance_profile.cpp` direction,
`heading_to_direction_006bc0c0`), or into a log. The torpedo-override turn in
`game_hosts_ship_ai.cpp` is wrapped before use and is a log column.

**Expected movers.** These are the rows that store a heading outside (-pi, pi]. On those rows ON
changes the logged `desired_1d8` / heading values and can move results by float rounding. The
current census is the `summary mission ship ai heading wrap` line in cc9-lua52's
`local\l70r_n_<row>_main_ref.log`, all written on 2026-10-19 between 04:33 and 06:25 local time:

| row | stores | out of range | max abs |
| --- | --- | --- | --- |
| j6k | 84994 | 14134 | 6.2828 |
| j6m | 48475 | 4895 | 6.2824 |
| j6n | 14596 | 1221 | 6.2824 |
| w_i18 | 112256 | 614 | 4.5341 |
| l6i | 2340 | 428 | 5.8770 |
| l6h | 2132 | 427 | 5.8763 |
| l6g | 2357 | 237 | 4.7140 |
| vu21 | 831955 | 222 | 5.4856 |
| l6e | 3013 | 99 | 5.0174 |
| l6j | 799 | 99 | 5.0174 |
| l6k | 559 | 99 | 5.0174 |
| l6w | 3013 | 99 | 5.0174 |
| b1d | 168345 | 12 | 3.1416 |
| w_i08 | 159242 | 3 | 3.1416 |

- **b1d and w_i08:** their max is pi, so their out-of-range stores sit exactly at -pi. The census
  counts -pi as outside (-pi, pi], and `00605070` maps it to +pi, so ON changes only the sign of
  those values.
- **Zero out of range:** u02a, u04, u12e8, u15e, u1s, vb9, w_e8, w_l10, w_p01 and w_u15. ON is
  identical there.

**Census bound (packet cc13-wrapnits).** Review nit N4 suspected the census compare was off at
+float(pi): it assumed `00605070` uses the double pi 3.14159265358979, which is below the float
literal `3.14159265f` (3.14159274). The bytes show otherwise.
- `00CE3D18` is `00000060FB2109C0` and `00CE3D28` is `00000060FB210940`, that is -/+3.1415927410125732.
  These are float pi widened to double. `00CE3828` (2pi) is likewise 6.2831854820251465.
- `00605070` stores the fmod result as a float and reloads it. It then wraps when x <= -pi
  (`0060508B FLD [00CE3D18]`, `00605091 FCOMIP`, `00605093 JB` past the +2pi arm) or when x > pi
  (`006050A0 FLD [00CE3D28]`, `006050A8 FCOMI`, `006050AC JBE` past the -2pi arm).
- `count_heading_wrap_store` tests `heading > 3.14159265f || heading <= -3.14159265f` on that same
  float value, so its arms match the image exactly.

So a store of exactly +float(pi) is left alone by both the image and the census. The zero rows, and
the -pi rows b1d and w_i08, need no correction, and the census numbers above stand. No code was
changed for N4.

The superseded 2026-09-28 census (cc9-ships9) had found only JM06, with 79 of 2595 stores.

**Recommendation.** Thousands of stores move on j6k and hundreds on w_i18. Run a dedicated pair
on j6k and w_i18 (ON against OFF) instead of folding the flip into a batch.

## Uncertainty

- The host's fmod and fixups are computed in double and then rounded to float. The image's
  `FADD` / `FSUB` run at the thread's x87 precision control and store once; on the main thread
  (24-bit precision) a result can differ in the last place from the host's double-then-float.
- The census table is from cc9-lua52's 2026-10-19 main_ref builds; later main may store
  differently. The counts are of pre-wrap values at the four sites only.
