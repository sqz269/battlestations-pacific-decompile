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

`00605070` (`00605074..006050B9`): `fmod(*angle, 2pi)` through the CRT `00BF857A` with the
double `00CE3828`, stored as a float; then `+2pi` when the double `-pi` at `00CE3D18` is not
below the result (`FCOMPI` / `JB`), else `-2pi` when the result is above the double `pi` at
`00CE3D28` (`FCOMI` / `JBE`). The result lies in (-pi, pi]. The host's
`ship_ai_firepower_wrap_angle_00605070` (`src/ship_ai_bearing_rating.cpp`) is this body; the land
path's `wrap_blk_desired_heading_00605070` already applies it unconditionally.

## The host, OFF

The hooks `after_heading_stored_00605070` (`SetterBinding` and `HeadingHoldBinding`) and
`wrap_brain_heading_00605070` (`ApproachStepBinding`) only count the stored value
(`count_heading_wrap_store`) and record the call. The field keeps whatever was stored.

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

**Expected movers.** Only rows that store a heading outside (-pi, pi]: the census found one,
JM06 (79 of 2595 stores, largest 6.2785). There ON changes the logged `desired_1d8` / heading
values and can move results by float rounding. Every other census row stored 0 out-of-range
values, so ON is identical there. It is not worth a dedicated pair; flip it in a batch.

## Uncertainty

- The host's fmod and fixups are computed in double and then rounded to float. The image's
  `FADD` / `FSUB` run at the thread's x87 precision control and store once; on the main thread
  (24-bit precision) a result can differ in the last place from the host's double-then-float.
- The census row counts are from 2026-09-28 builds; current main may store differently.
