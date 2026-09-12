#!/usr/bin/env python3
"""Compare a live motion trace with the reconstructed ship-motion probe's trajectory.

The probe (`bsp_ship_motion_probe.exe`, `src/ship_motion_probe.cpp`) prints a
fixed-width table every ten steps:

   step        t         x         y         z    heading   fwd_spd   throttle    rudder   yaw_rate

A capture taken before the Dyn integrator added the `y` column, with eight
numbers instead of nine, is still accepted.

The live side is a CSV captured from the game under a debugger at the
`00825F20` breakpoint; `docs/MOTION_DIFFERENTIAL.md` defines the sampling plan
and the exact column meanings. Required columns:

    t,dt,pos_x,pos_y,pos_z,fwd_x,fwd_y,fwd_z,fwd_speed,throttle,rudder,yaw_rate

`heading` is derived from the forward axis rather than trusted from the trace,
so a capture that omits a heading column still compares.

The probe's ocean sampler and gameplay scale are still stand-ins; its rudder
denominator curve is not, as of the build that prints "rudder curve
+438h..+44Ch from shipglobals.lua" in its header. `--apply-curve` is therefore
a compatibility option for probe output captured *before* the authored knots
landed: it divides the *probe* side's yaw rate by the curve of
`docs/UNIT_RUDDER_CURVE.md`, which is what the shipped game divides
`MaxRotAngle` by. Applying it to a current probe run double-counts the
denominator. Check the probe header before using it.

It does not correct the trace side. A `bsp_game.exe` milestone-2i (or earlier) log
forced the three knots to 1 (milestone 2j loads the authored knots) and so reports a yaw rate a full denominator too
high; fed in as a trace it will diverge from a current probe run by exactly
that factor, which is the point of comparing them.

Usage:
    python tools/motion_trace_compare.py --trace local/motion_trace.csv \\
        --probe local/probe_class11.txt [--apply-curve] [--max-speed 17.4911]
"""

from __future__ import annotations

import argparse
import csv
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Optional, Sequence

# The authored denominator knots, ShipGlobals["Navigator"]["TurnMultipliers"] in
# <install>/scripts/datatables/shipglobals.lua, read into the gameplay settings
# singleton [00F8753C] at +444h/+440h, +44Ch/+448h and +43Ch/+438h.
# See docs/UNIT_RUDDER_CURVE.md.
CURVE_MIN = (0.0, 0.4)   # +444h, +440h   TurnMultiplierMinSpeed
CURVE_MED = (0.5, 1.5)   # +44Ch, +448h   TurnMultiplierMedSpeed
CURVE_MAX = (1.0, 2.0)   # +43Ch, +438h   TurnMultiplierMaxSpeed

# The probe's table gained a `y` column when the Dyn integrator landed, so a
# current run prints nine numbers after the step index and an older capture
# prints eight. Both forms are accepted and the `y` is dropped, because the
# comparison is planar. docs/GAME_EXECUTABLE.md milestone 2o, correction 7.
PROBE_COLUMNS = ("step", "t", "x", "y", "z", "heading", "fwd_spd", "throttle", "rudder", "yaw_rate")
PROBE_COLUMNS_LEGACY = ("step", "t", "x", "z", "heading", "fwd_spd", "throttle", "rudder", "yaw_rate")
_PROBE_NUMBER = r"(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)"
_PROBE_ROW = re.compile(r"^\s*(\d+)\s+" + r"\s+".join([_PROBE_NUMBER] * 9) + r"\s*$")
_PROBE_ROW_LEGACY = re.compile(r"^\s*(\d+)\s+" + r"\s+".join([_PROBE_NUMBER] * 8) + r"\s*$")


class CompareError(RuntimeError):
    """A malformed input that the caller should report rather than trace back."""


@dataclass
class Sample:
    """One aligned motion sample. Angles in degrees, rates in rad/s."""

    t: float
    x: float
    z: float
    heading: float
    speed: float
    throttle: float
    rudder: float
    yaw_rate: float
    dt: Optional[float] = None


def clamped_lerp(x0: float, y0: float, x1: float, y1: float, x: float) -> float:
    """`00419010`: lerp then clamp to the two ordinates, `y0` for `x1 == x0`.

    The native routine evaluates in x87 intermediates and clamps to the
    ordinates in their given order; this reproduces the value, not the
    precision. See docs/UNIT_RUDDER_CURVE.md.
    """
    if x1 == x0:
        return y0
    value = ((x - x0) / (x1 - x0)) * (y1 - y0) + y0
    low, high = (y0, y1) if y0 <= y1 else (y1, y0)
    return min(max(value, low), high)


def turn_denominator(speed_ratio: float) -> float:
    """`0082E890`: the two-segment clamped interpolation on |speed / MaxSpeed|.

    `0082ECB0` divides `MaxRotAngle` by this, so a larger turning circle is a
    smaller yaw rate. At full throttle it is 2.0, so a hull turns at half its
    `MaxRotAngle`.
    """
    magnitude = abs(speed_ratio)
    if magnitude <= CURVE_MED[0]:
        return clamped_lerp(CURVE_MIN[0], CURVE_MIN[1], CURVE_MED[0], CURVE_MED[1], magnitude)
    return clamped_lerp(CURVE_MED[0], CURVE_MED[1], CURVE_MAX[0], CURVE_MAX[1], magnitude)


def heading_from_forward(fx: float, fz: float) -> float:
    """Degrees in [0, 360) from the pose's forward axis (unit+0ECh, +0F4h)."""
    return math.degrees(math.atan2(fx, fz)) % 360.0


def angle_delta(a: float, b: float) -> float:
    """Signed smallest difference a - b in degrees, in (-180, 180]."""
    return (a - b + 180.0) % 360.0 - 180.0


def parse_probe_table(text: str) -> List[Sample]:
    """Pull the trajectory rows out of the probe's stdout.

    Rows are matched structurally (an integer step then nine floats, or eight
    for a capture taken before the probe's `y` column landed) so the
    surrounding header and trailer lines are ignored without needing to be
    parsed.
    """
    samples: List[Sample] = []
    for line in text.splitlines():
        match = _PROBE_ROW.match(line)
        if match:
            _, t, x, _y, z, heading, speed, throttle, rudder, yaw_rate = (
                float(g) for g in match.groups()
            )
        else:
            match = _PROBE_ROW_LEGACY.match(line)
            if not match:
                continue
            _, t, x, z, heading, speed, throttle, rudder, yaw_rate = (
                float(g) for g in match.groups()
            )
        samples.append(
            Sample(
                t=t,
                x=x,
                z=z,
                heading=heading % 360.0,
                speed=speed,
                throttle=throttle,
                rudder=rudder,
                yaw_rate=yaw_rate,
            )
        )
    if not samples:
        raise CompareError(
            "no probe trajectory rows matched; expected the table printed by "
            "bsp_ship_motion_probe.exe (columns: " + " ".join(PROBE_COLUMNS)
            + "; a pre-Dyn capture without `y` is also accepted)"
        )
    return samples


def _column(row: dict, *names: str) -> Optional[float]:
    for name in names:
        if name in row and row[name] not in (None, ""):
            return float(row[name])
    return None


def parse_trace_csv(path: Path) -> List[Sample]:
    """Read the live capture. See docs/MOTION_DIFFERENTIAL.md for the columns."""
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise CompareError(f"{path}: no header row")
        fields = {name.strip() for name in reader.fieldnames}
        missing = {"t"} - fields
        if missing:
            raise CompareError(f"{path}: missing required column(s): {', '.join(sorted(missing))}")

        samples: List[Sample] = []
        for index, raw in enumerate(reader, start=2):
            row = {(k.strip() if k else k): v for k, v in raw.items()}
            try:
                t = _column(row, "t", "time")
                if t is None:
                    raise CompareError(f"{path}:{index}: empty 't'")
                fx = _column(row, "fwd_x", "forward_x")
                fz = _column(row, "fwd_z", "forward_z")
                heading = _column(row, "heading", "heading_deg")
                if fx is not None and fz is not None:
                    heading = heading_from_forward(fx, fz)
                elif heading is None:
                    raise CompareError(
                        f"{path}:{index}: need either fwd_x/fwd_z or a heading column"
                    )
                samples.append(
                    Sample(
                        t=t,
                        x=_column(row, "pos_x", "x") or 0.0,
                        z=_column(row, "pos_z", "z") or 0.0,
                        heading=heading % 360.0,
                        speed=_column(row, "fwd_speed", "fwd_spd", "speed") or 0.0,
                        throttle=_column(row, "throttle") or 0.0,
                        rudder=_column(row, "rudder") or 0.0,
                        yaw_rate=_column(row, "yaw_rate") or 0.0,
                        dt=_column(row, "dt"),
                    )
                )
            except ValueError as error:
                raise CompareError(f"{path}:{index}: {error}") from error
    if not samples:
        raise CompareError(f"{path}: no data rows")
    return samples


def interpolate_at(samples: Sequence[Sample], t: float) -> Optional[Sample]:
    """Linear interpolation of a trajectory at time `t`, headings unwrapped.

    Returns None outside the sampled interval rather than extrapolating.
    """
    if not samples or t < samples[0].t or t > samples[-1].t:
        return None
    for previous, current in zip(samples, samples[1:]):
        if previous.t <= t <= current.t:
            span = current.t - previous.t
            f = 0.0 if span == 0 else (t - previous.t) / span
            heading = (previous.heading + angle_delta(current.heading, previous.heading) * f) % 360.0
            return Sample(
                t=t,
                x=previous.x + (current.x - previous.x) * f,
                z=previous.z + (current.z - previous.z) * f,
                heading=heading,
                speed=previous.speed + (current.speed - previous.speed) * f,
                throttle=previous.throttle + (current.throttle - previous.throttle) * f,
                rudder=previous.rudder + (current.rudder - previous.rudder) * f,
                yaw_rate=previous.yaw_rate + (current.yaw_rate - previous.yaw_rate) * f,
            )
    return samples[-1]


def apply_curve(samples: Iterable[Sample], max_speed: float) -> List[Sample]:
    """Rescale the probe's yaw rate by the authored denominator.

    The probe forces the three denominator knots to 1.0, so its yaw rate is
    `MaxRotAngle * r * rudder * efficiency`. The shipped game divides by
    `turn_denominator(r)`. This does not re-integrate the heading; it corrects
    the rate channel so the two are comparable, and reports the heading
    separately.
    """
    if max_speed <= 0.0:
        raise CompareError("--max-speed must be positive to apply the curve")
    corrected = []
    for sample in samples:
        denominator = turn_denominator(sample.speed / max_speed)
        corrected.append(
            Sample(
                t=sample.t,
                x=sample.x,
                z=sample.z,
                heading=sample.heading,
                speed=sample.speed,
                throttle=sample.throttle,
                rudder=sample.rudder,
                yaw_rate=sample.yaw_rate / denominator if denominator else sample.yaw_rate,
            )
        )
    return corrected


@dataclass
class Divergence:
    """The first sample at which one channel leaves its tolerance."""

    channel: str
    t: float
    trace: float
    probe: float
    delta: float


def align_origin(samples: Sequence[Sample]) -> List[Sample]:
    """Shift a trajectory so its first sample sits at the origin on a heading of 0.

    A live capture reads the pose translation at `unit+0FCh`, which is a world
    position, and a heading taken from the scene's placement; the probe starts
    every run at the origin pointing down +z. Comparing positions at all needs
    one of the two moved onto the other's frame, and only a rigid motion is
    legitimate - the rotation is by the trace's own initial heading, so no
    scale or reflection is introduced.
    """
    if not samples:
        return []
    first = samples[0]
    angle = math.radians(first.heading)
    cos_a, sin_a = math.cos(-angle), math.sin(-angle)
    shifted = []
    for sample in samples:
        dx = sample.x - first.x
        dz = sample.z - first.z
        shifted.append(
            Sample(
                t=sample.t,
                x=dx * cos_a + dz * sin_a,
                z=-dx * sin_a + dz * cos_a,
                heading=(sample.heading - first.heading) % 360.0,
                speed=sample.speed,
                throttle=sample.throttle,
                rudder=sample.rudder,
                yaw_rate=sample.yaw_rate,
                dt=sample.dt,
            )
        )
    return shifted


def compare(
    trace: Sequence[Sample],
    probe: Sequence[Sample],
    speed_tol: float,
    heading_tol: float,
    position_tol: float,
) -> dict:
    """Align the two trajectories on the trace's timestamps and difference them."""
    rows = []
    divergences: dict = {}
    for sample in trace:
        other = interpolate_at(probe, sample.t)
        if other is None:
            continue
        speed_delta = sample.speed - other.speed
        heading_delta = angle_delta(sample.heading, other.heading)
        dx = sample.x - other.x
        dz = sample.z - other.z
        distance = math.hypot(dx, dz)
        yaw_delta = sample.yaw_rate - other.yaw_rate
        rows.append(
            {
                "t": sample.t,
                "trace_speed": sample.speed,
                "probe_speed": other.speed,
                "speed_delta": speed_delta,
                "trace_heading": sample.heading,
                "probe_heading": other.heading,
                "heading_delta": heading_delta,
                "position_delta": distance,
                "dx": dx,
                "dz": dz,
                "trace_yaw_rate": sample.yaw_rate,
                "probe_yaw_rate": other.yaw_rate,
                "yaw_rate_delta": yaw_delta,
            }
        )
        for channel, value, tolerance, a, b in (
            ("speed", abs(speed_delta), speed_tol, sample.speed, other.speed),
            ("heading", abs(heading_delta), heading_tol, sample.heading, other.heading),
            ("position", distance, position_tol, sample.x, other.x),
        ):
            if channel not in divergences and value > tolerance:
                divergences[channel] = Divergence(channel, sample.t, a, b, value)

    if not rows:
        raise CompareError(
            "no overlapping time range between the trace and the probe run; "
            "check that both start at t = 0 and use the same step"
        )

    def peak(key: str) -> float:
        return max(abs(row[key]) for row in rows)

    return {
        "samples": len(rows),
        "time_span": [rows[0]["t"], rows[-1]["t"]],
        "peak_speed_delta": peak("speed_delta"),
        "peak_heading_delta": peak("heading_delta"),
        "peak_position_delta": peak("position_delta"),
        "peak_yaw_rate_delta": peak("yaw_rate_delta"),
        "final_speed_delta": rows[-1]["speed_delta"],
        "final_heading_delta": rows[-1]["heading_delta"],
        "final_position_delta": rows[-1]["position_delta"],
        "divergences": {k: vars(v) for k, v in divergences.items()},
        "rows": rows,
    }


def format_report(result: dict, apply_curve_flag: bool) -> str:
    lines = [
        "motion differential: live trace vs reconstructed probe",
        f"  samples compared   {result['samples']}",
        f"  time span          {result['time_span'][0]:.2f} .. {result['time_span'][1]:.2f} s",
        f"  probe yaw rate     {'divided here by the authored turn curve' if apply_curve_flag else 'as printed by the probe'}",
        "",
        f"  {'channel':<14}{'peak |delta|':>14}{'final delta':>14}",
        f"  {'speed (m/s)':<14}{result['peak_speed_delta']:>14.4f}{result['final_speed_delta']:>14.4f}",
        f"  {'heading (deg)':<14}{result['peak_heading_delta']:>14.4f}{result['final_heading_delta']:>14.4f}",
        f"  {'position (m)':<14}{result['peak_position_delta']:>14.4f}{result['final_position_delta']:>14.4f}",
        f"  {'yaw (rad/s)':<14}{result['peak_yaw_rate_delta']:>14.5f}{'':>14}",
        "",
    ]
    if result["divergences"]:
        lines.append("  first divergence beyond tolerance:")
        for divergence in result["divergences"].values():
            lines.append(
                f"    {divergence['channel']:<10} t = {divergence['t']:6.2f} s  "
                f"trace {divergence['trace']:10.4f}  probe {divergence['probe']:10.4f}  "
                f"delta {divergence['delta']:.4f}"
            )
    else:
        lines.append("  no channel left its tolerance over the compared span")
    return "\n".join(lines)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--trace", required=True, type=Path, help="live capture CSV")
    parser.add_argument("--probe", required=True, type=Path, help="saved stdout of bsp_ship_motion_probe.exe")
    parser.add_argument(
        "--apply-curve",
        action="store_true",
        help="divide the probe's yaw rate by the authored turn-multiplier denominator",
    )
    parser.add_argument(
        "--max-speed",
        type=float,
        default=None,
        help="class MaxSpeed (class+500h), required with --apply-curve",
    )
    parser.add_argument(
        "--align-origin",
        action="store_true",
        help="rigidly move the trace onto the probe's frame: first sample to the origin, "
        "initial heading to 0. A live capture reads world coordinates; the probe starts at 0.",
    )
    parser.add_argument("--speed-tol", type=float, default=0.5, help="m/s (default 0.5)")
    parser.add_argument("--heading-tol", type=float, default=2.0, help="degrees (default 2.0)")
    parser.add_argument("--position-tol", type=float, default=5.0, help="metres (default 5.0)")
    parser.add_argument("--json", type=Path, default=None, help="write the full per-step table here")
    args = parser.parse_args(argv)

    try:
        probe = parse_probe_table(args.probe.read_text(encoding="utf-8", errors="replace"))
        trace = parse_trace_csv(args.trace)
        if args.align_origin:
            trace = align_origin(trace)
        if args.apply_curve:
            if args.max_speed is None:
                raise CompareError("--apply-curve needs --max-speed (the class's MaxSpeed)")
            probe = apply_curve(probe, args.max_speed)
        result = compare(trace, probe, args.speed_tol, args.heading_tol, args.position_tol)
    except (CompareError, OSError) as error:
        print(f"motion_trace_compare: {error}", file=sys.stderr)
        return 2

    print(format_report(result, args.apply_curve))
    if args.json:
        import json

        args.json.write_text(json.dumps(result, indent=2), encoding="utf-8")
        print(f"\n  per-step table written to {args.json}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
