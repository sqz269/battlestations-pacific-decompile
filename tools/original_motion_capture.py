#!/usr/bin/env python3
"""Read-only, wall-clock motion samples from the original x86 game on Windows.

The caller supplies a unit discovered at 00825F20 (ECX - 310h). Native offsets
and evidence are in docs/ORIGINAL_MOTION_ABI.md and docs/MOTION_DIFFERENTIAL.md
section 3. This tool neither starts nor stops the game, attaches a debugger,
injects code, nor writes process memory.
It opens only PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ.

The CSV is accepted by tools/motion_trace_compare.py, but is NOT a tick trace:
dt stays empty, engine_dt is the stored scaled OnMove frame delta, sample_dt is
a wall-clock interval, and sample_game_dt is a consecutive sample clock delta.
Schema 3 adds native game_time/game_frame brackets; see docs/ORIGINAL_GAME_CLOCK.md.
Remote reads are not atomic and repeated engine states are
possible. Pointer checks reject some torn reads; they cannot prove consistency
within an engine tick or detect every reuse of the supplied unit address.

Example (unit is hexadecimal, with or without 0x):
    python tools/original_motion_capture.py --pid 1234 --unit 1234AB00 \
        --seconds 20 --hz 20 --output local/original_motion.csv

A JSON sidecar records executable identity, timing limitations, failures, and
completion state. Existing outputs are never overwritten. No dependencies
beyond Python's standard library are required.

For a one-shot validated read of the controlled-unit global (no debugger):
    python tools/original_motion_capture.py --pid 1234 --discover-controlled
Discovery prints JSON, or writes a new JSON file when --output is supplied.
It does not start a capture or follow later selection changes.

For a clock-only coverage-window marker (no unit required):
    python tools/original_motion_capture.py --pid 1234 --clock-only
Clock-only JSON reports both brackets and returns 2 if the final read is torn/active.

For several fixed units, replace --unit with --units-file PATH. Its JSON is
{"units": [{"unit_address": "0x1234AB00", "label": "USS Lexington"}, ...]}.
The same wall deadline samples each unit; unit_id remains its supplied address.
"""

from __future__ import annotations

import argparse
from collections import Counter
import csv
import ctypes
from ctypes import wintypes
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import sys
import time


IMAGE_BASE = 0x00400000
CONTROLLED_GLOBAL = 0x00E188D8  # include/bsp/controlled_unit.hpp; store at 004C0893
GAME_GLOBAL = 0x00E188A8
MISSION_CLOCK_BLOCK = 0x00F876A0
FIXED_STEP_DT_GLOBAL = 0x00D0DE84
PROCESS_VM_READ = 0x0010
PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
STILL_ACTIVE = 259
UNIT_START = 0xC8
UNIT_END = 0x101C
REPO = Path(__file__).resolve().parents[1]
FIELDS = (
    "t", "dt", "pos_x", "pos_y", "pos_z", "fwd_x", "fwd_y", "fwd_z",
    "fwd_speed", "throttle", "rudder", "yaw_rate", "sample_dt", "engine_dt",
    "monotonic_ns", "wall_utc", "sample_span", "unit_id", "class_ptr",
    "controller_ptr", "rigidbody_ptr", "motion_state_ptr", "smoothed_rudder",
    "up_x", "up_y", "up_z",
    "vel_x", "vel_y", "vel_z", "angvel_x", "angvel_y", "angvel_z",
    "controlled_global_before", "controlled_global_after", "controlled_relationship",
    "unit_label", "game_ptr", "game_time", "game_frame", "fixed_step_count", "fixed_step_dt",
    "sample_game_dt", "game_clock_epoch", "clock_torn", "clock_retry_count", "clock_torn_reason",
    "game_time_before", "game_time_after", "game_frame_before", "game_frame_after",
    "engine_dt_before", "engine_dt_after", "fixed_step_count_before", "fixed_step_count_after",
    "fixed_step_active_before", "fixed_step_active_after", "game_elapsed_before", "game_elapsed_after",
    "fixed_remainder_before", "fixed_remainder_after",
)


class CaptureError(RuntimeError):
    pass


class SampleError(CaptureError):
    pass


class IdentityChanged(SampleError):
    pass


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def pointer(value: int, size: int = 4) -> int:
    if value < 0x10000 or value % 4 or value + size > 0x100000000:
        raise SampleError(f"invalid x86 pointer/span: 0x{value:X}+0x{size:X}")
    return value


def pe_identity(read) -> dict:
    """Read only the DOS/PE headers, from a file or the fixed process image."""
    dos = read(0, 0x40)
    if dos[:2] != b"MZ":
        raise CaptureError("image does not have a DOS header")
    offset = struct.unpack_from("<I", dos, 0x3C)[0]
    if not 0x40 <= offset <= 0x100000:
        raise CaptureError("invalid PE header offset")
    pe = read(offset, 0x78)
    if pe[:4] != b"PE\0\0":
        raise CaptureError("image does not have a PE signature")
    machine, = struct.unpack_from("<H", pe, 4)
    timestamp, = struct.unpack_from("<I", pe, 8)
    optional_size, = struct.unpack_from("<H", pe, 20)
    magic, = struct.unpack_from("<H", pe, 24)
    entry, = struct.unpack_from("<I", pe, 24 + 16)
    image_base, = struct.unpack_from("<I", pe, 24 + 28)
    image_size, = struct.unpack_from("<I", pe, 24 + 56)
    if machine != 0x14C or magic != 0x10B or optional_size < 96:
        raise CaptureError("expected an x86 PE32 executable")
    if image_base != IMAGE_BASE:
        raise CaptureError(f"expected preferred image base 00400000, got {image_base:08X}")
    return {"machine": "014C", "magic": "010B", "image_base": f"{image_base:08X}",
            "image_size": image_size, "entry_rva": entry, "timestamp": timestamp}


class ProcessReader:
    def __init__(self, pid: int):
        if os.name != "nt":
            raise CaptureError("process capture requires Windows")
        self.api = ctypes.WinDLL("kernel32", use_last_error=True)
        signatures = {
            "OpenProcess": ([wintypes.DWORD, wintypes.BOOL, wintypes.DWORD], wintypes.HANDLE),
            "ReadProcessMemory": ([wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p,
                                   ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)], wintypes.BOOL),
            "QueryFullProcessImageNameW": ([wintypes.HANDLE, wintypes.DWORD, wintypes.LPWSTR,
                                            ctypes.POINTER(wintypes.DWORD)], wintypes.BOOL),
            "GetProcessTimes": ([wintypes.HANDLE] + [ctypes.POINTER(wintypes.FILETIME)] * 4,
                                wintypes.BOOL),
            "GetExitCodeProcess": ([wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD)], wintypes.BOOL),
            "CloseHandle": ([wintypes.HANDLE], wintypes.BOOL),
        }
        for name, (args, result) in signatures.items():
            function = getattr(self.api, name)
            function.argtypes, function.restype = args, result
        self.handle = self.api.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                                           False, pid)
        if not self.handle:
            raise CaptureError(f"OpenProcess({pid}): {ctypes.WinError(ctypes.get_last_error())}")

    def close(self):
        if self.handle:
            self.api.CloseHandle(self.handle)
            self.handle = None

    def read(self, address: int, size: int) -> bytes:
        pointer(address, size)
        data = ctypes.create_string_buffer(size)
        count = ctypes.c_size_t()
        ok = self.api.ReadProcessMemory(self.handle, address, data, size, ctypes.byref(count))
        if not ok or count.value != size:
            raise SampleError(f"ReadProcessMemory 0x{address:08X}+0x{size:X}: "
                              f"read {count.value}; {ctypes.WinError(ctypes.get_last_error())}")
        return data.raw

    def u32(self, address: int) -> int:
        return struct.unpack("<I", self.read(address, 4))[0]

    def exit_code(self) -> int:
        result = wintypes.DWORD()
        if not self.api.GetExitCodeProcess(self.handle, ctypes.byref(result)):
            raise CaptureError(f"GetExitCodeProcess: {ctypes.WinError(ctypes.get_last_error())}")
        return result.value

    def identity(self, expected_hash: str | None) -> dict:
        path_buffer = ctypes.create_unicode_buffer(32768)
        length = wintypes.DWORD(len(path_buffer))
        if not self.api.QueryFullProcessImageNameW(self.handle, 0, path_buffer, ctypes.byref(length)):
            raise CaptureError(f"QueryFullProcessImageNameW: {ctypes.WinError(ctypes.get_last_error())}")
        path = Path(path_buffer.value)
        if path.name.lower() != "battlestationspacific.exe":
            raise CaptureError(f"refusing non-original executable basename: {path.name}")
        with path.open("rb") as stream:
            def read_file(offset, size):
                stream.seek(offset)
                data = stream.read(size)
                if len(data) != size:
                    raise CaptureError("truncated executable header")
                return data
            disk_pe = pe_identity(read_file)
            stream.seek(0)
            digest = hashlib.file_digest(stream, "sha256").hexdigest()
        if expected_hash and digest != expected_hash.lower():
            raise CaptureError("executable SHA-256 differs from --expect-sha256")
        memory_pe = pe_identity(lambda offset, size: self.read(IMAGE_BASE + offset, size))
        if disk_pe != memory_pe:
            raise CaptureError("on-disk and fixed-base process PE identities differ")
        times = [wintypes.FILETIME() for _ in range(4)]
        if not self.api.GetProcessTimes(self.handle, *(ctypes.byref(item) for item in times)):
            raise CaptureError(f"GetProcessTimes: {ctypes.WinError(ctypes.get_last_error())}")
        ticks = (times[0].dwHighDateTime << 32) | times[0].dwLowDateTime
        created = datetime.fromtimestamp((ticks - 116444736000000000) / 10000000,
                                        timezone.utc).isoformat()
        return {"path": str(path), "disk_sha256": digest, "pe": disk_pe,
                "process_created_utc": created, "process_created_filetime": ticks,
                "hash_scope": "on-disk executable, not a hash of all mapped process memory"}


def finite_range(values, bound: float, label: str):
    if any(not math.isfinite(value) or abs(value) > bound for value in values):
        raise SampleError(f"nonfinite/out-of-range {label} (absolute bound {bound:g})")


def read_game_clock(reader: ProcessReader) -> dict:
    """Borrow actual native clock cells; this is not a reconstructed C++ layout."""
    game = reader.u32(GAME_GLOBAL)
    pointer(game, 0x21F4)
    frame, elapsed = struct.unpack("<If", reader.read(game + 0x648, 8))
    unscaled_dt, engine_dt = struct.unpack("<2f", reader.read(game + 0x21EC, 8))
    raw = reader.read(MISSION_CLOCK_BLOCK, 0x18)
    game_time, accumulated, remainder, steps, interpolation = struct.unpack_from("<3fIf", raw, 4)
    fixed_dt, = struct.unpack("<f", reader.read(FIXED_STEP_DT_GLOBAL, 4))
    if reader.u32(GAME_GLOBAL) != game:
        raise SampleError("game singleton changed during clock read")
    finite_range((elapsed, game_time, accumulated), 1e9, "game clock")
    finite_range((unscaled_dt, engine_dt, fixed_dt, remainder, interpolation), 1e4, "game clock intervals")
    if engine_dt < 0 or fixed_dt <= 0 or raw[1] not in (0, 1):
        raise SampleError("invalid native clock delta/phase")
    return {"game_ptr": game, "game_time": game_time, "game_frame": frame,
            "engine_dt": engine_dt, "fixed_step_dt": fixed_dt, "fixed_step_count": steps,
            "fixed_step_active": raw[1], "game_elapsed": elapsed, "unscaled_dt": unscaled_dt,
            "accumulated": accumulated, "fixed_remainder": remainder, "interpolation": interpolation}


def clocked_motion_sample(reader: ProcessReader, unit: int) -> tuple[dict, tuple[int, int, int, int]]:
    """Bracket the pose and retry one changed/active clock observation."""
    for attempt in range(2):
        before = read_game_clock(reader)
        sample, identity = motion_sample(reader, unit)
        after = read_game_clock(reader)
        reasons = []
        if before != after:
            reasons.append("clock_changed:" + ",".join(field for field in before if before[field] != after[field]))
        if before["fixed_step_active"] or after["fixed_step_active"]:
            reasons.append("fixed_step_active")
        if not reasons or attempt == 1:
            break
    for field in ("game_time", "game_frame", "engine_dt", "fixed_step_count", "fixed_step_active",
                  "game_elapsed", "fixed_remainder"):
        sample[f"{field}_before"] = before[field]
        sample[f"{field}_after"] = after[field]
    sample.update(game_ptr=f"0x{after['game_ptr']:08X}", clock_torn=int(bool(reasons)),
                  clock_retry_count=attempt, clock_torn_reason=";".join(reasons))
    # Blank canonical clocks on the final torn attempt; diagnostic brackets remain.
    for field in ("game_time", "game_frame", "engine_dt", "fixed_step_count", "fixed_step_dt"):
        sample[field] = "" if reasons else after[field]
    return sample, identity


def record_game_clock(series: dict, sample: dict, midpoint: int):
    """Keep per-unit epochs; a reset must not become a negative sample interval."""
    series["retry_samples"] += sample["clock_retry_count"]
    sample["sample_game_dt"] = ""
    sample["game_clock_epoch"] = ""
    if sample["clock_torn"]:
        series["torn_samples"] += 1
        return
    series["clean_samples"] += 1
    epochs = series["epochs"]
    now = sample["game_time"]
    if (not epochs or sample["game_ptr"] != epochs[-1]["game_ptr"]
            or now < epochs[-1]["last_game_time"]
            or sample["game_frame"] < epochs[-1]["last_game_frame"]):
        epochs.append({"game_ptr": sample["game_ptr"], "first_game_time": now,
                       "first_monotonic_ns": midpoint})
    else:
        sample["sample_game_dt"] = now - epochs[-1]["last_game_time"]
    sample["game_clock_epoch"] = len(epochs) - 1
    epochs[-1].update(last_game_time=now, last_monotonic_ns=midpoint,
                      last_game_frame=sample["game_frame"])


def finish_game_clock(series: dict):
    for epoch in series["epochs"]:
        wall = (epoch["last_monotonic_ns"] - epoch["first_monotonic_ns"]) / 1e9
        simulated = epoch["last_game_time"] - epoch["first_game_time"]
        epoch.update(wall_seconds=wall, game_seconds=simulated,
                     game_to_wall_ratio=simulated / wall if wall > 0 else None)
    epochs = series["epochs"]
    series["game_to_wall_ratio"] = epochs[0]["game_to_wall_ratio"] if len(epochs) == 1 else None


def motion_sample(reader: ProcessReader, unit: int) -> tuple[dict, tuple[int, int, int, int]]:
    """Read native unit/controller/body/motion state, then recheck the chain."""
    raw = reader.read(unit + UNIT_START, UNIT_END - UNIT_START)
    def values(fmt, offset):
        return struct.unpack_from("<" + fmt, raw, offset - UNIT_START)
    if raw[0] != 1:
        raise SampleError("pose-valid byte at unit+0C8h is not 1")
    up = values("3f", 0xDC)
    forward = values("3f", 0xEC)
    position = values("3f", 0xFC)
    throttle, rudder = values("2f", 0x980)
    class_ptr, = values("I", 0x538)
    controller, = values("I", 0x1018)
    pointer(class_ptr)
    pointer(controller, 0x84)
    control = reader.read(controller, 0x84)
    back, = struct.unpack_from("<I", control, 0x1C)
    body, = struct.unpack_from("<I", control, 0x2C)
    smoothed, = struct.unpack_from("<f", control, 0x80)
    if back != unit:
        raise SampleError("controller unit back-pointer mismatch")
    pointer(body, 8)
    # Native Dyn body B owns M = [B+4]; velocity offsets are relative to M.
    # 00C41587 loads M; 00C5B1EA and 00C5B1F8..2B9 consume its two vectors.
    motion_state = reader.u32(body + 4)
    pointer(motion_state, 0x18)
    velocity = struct.unpack("<6f", reader.read(motion_state, 0x18))
    recheck = reader.read(controller + 0x1C, 0x14)
    if (reader.u32(unit + 0x1018) != controller or reader.u32(unit + 0x538) != class_ptr
            or struct.unpack_from("<I", recheck, 0)[0] != unit
            or struct.unpack_from("<I", recheck, 0x10)[0] != body
            or reader.u32(body + 4) != motion_state):
        raise SampleError("unit/controller/body/motion-state pointer chain changed during sample")
    finite_range(position, 1e8, "position")
    finite_range((*up, *forward), 2, "pose axes")
    for axis in (up, forward):
        if not 0.5 <= sum(value * value for value in axis) <= 2:
            raise SampleError("pose-axis squared length outside [0.5, 2]")
    if abs(sum(a * b for a, b in zip(up, forward))) > 0.25:
        raise SampleError("pose up/forward axes are not approximately orthogonal")
    finite_range(velocity[:3], 1e5, "linear velocity")
    finite_range(velocity[3:], 1e4, "angular velocity")
    finite_range((throttle, rudder, smoothed), 8, "orders/smoothed rudder")
    result = dict(zip(("pos_x", "pos_y", "pos_z"), position))
    for prefix, vector in (("fwd", forward), ("up", up), ("vel", velocity[:3]),
                           ("angvel", velocity[3:])):
        result.update({f"{prefix}_{axis}": value for axis, value in zip("xyz", vector)})
    result.update(fwd_speed=sum(a * b for a, b in zip(velocity[:3], forward)),
                  yaw_rate=sum(a * b for a, b in zip(velocity[3:], up)),
                  throttle=throttle, rudder=rudder, smoothed_rudder=smoothed,
                  unit_id=f"0x{unit:08X}", class_ptr=f"0x{class_ptr:08X}",
                  controller_ptr=f"0x{controller:08X}", rigidbody_ptr=f"0x{body:08X}",
                  motion_state_ptr=f"0x{motion_state:08X}")
    return result, (class_ptr, controller, body, motion_state)


def read_controlled(reader: ProcessReader) -> tuple[int | None, str | None]:
    try:
        return reader.u32(CONTROLLED_GLOBAL), None
    except SampleError as error:
        return None, str(error)


def controlled_observation(unit: int, before: int | None, after: int | None,
                           errors: tuple[str | None, str | None], valid_sample: bool = True) -> dict:
    direct = valid_sample and unit != 0 and before == after == unit
    result = {"global_address": f"0x{CONTROLLED_GLOBAL:08X}",
              "pointer_before": None if before is None else f"0x{before:08X}",
              "pointer_after": None if after is None else f"0x{after:08X}",
              "candidate_unit": f"0x{unit:08X}",
              "relationship": "direct_match" if direct else "relationship_unverified"}
    if any(errors):
        result["read_errors"] = [error for error in errors if error]
    if not direct:
        result["reason"] = ("global read unavailable" if any(errors) else
                            "no valid ship motion state" if not valid_sample else
                            "global changed during sample" if before != after else
                            "no controlled unit" if before == 0 else
                            "different pointer; driven-sub-unit relationship was not evaluated")
    return result


def discover_controlled(args) -> int:
    reader = ProcessReader(args.pid)
    try:
        identity = reader.identity(args.expect_sha256)
        if reader.exit_code() != STILL_ACTIVE:
            raise CaptureError("target process has already exited")
        before_time = time.perf_counter_ns()
        before, before_error = read_controlled(reader)
        unit = args.unit if args.unit is not None else (before or 0)
        report = {"schema_version": 3, "kind": "original_game_controlled_unit_discovery",
                  "pid": args.pid, "recorded_utc": utc_now(), "executable": identity,
                  "status": "no_valid_ship_state", "unit_address": f"0x{unit:08X}",
                  "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
                  "offset_evidence": "docs/ORIGINAL_MOTION_ABI.md; docs/ORIGINAL_GAME_CLOCK.md",
                  "limitations": ["Unsuspended reads are not atomic or a motion-tick trace.",
                                  "A different pointer leaves any driven-sub-unit relationship unverified.",
                                  "A valid ship snapshot does not establish the mission or visible state."]}
        try:
            pointer(unit, UNIT_END)
            sample, _ = clocked_motion_sample(reader, unit)
            sample.update(unit_label=args.unit_label, sample_game_dt="", game_clock_epoch="")
        except SampleError as error:
            report["sample_error"] = str(error)
        else:
            report["motion"] = sample
        after, after_error = read_controlled(reader)
        observation = controlled_observation(unit, before, after, (before_error, after_error),
                                             valid_sample="motion" in report)
        report["controlled_global"] = observation
        report["sample_span"] = (time.perf_counter_ns() - before_time) / 1e9
        if "motion" in report:
            report["status"] = observation["relationship"]
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open("x", encoding="utf-8") as stream:
                json.dump(report, stream, indent=2)
                stream.write("\n")
        print(json.dumps(report, indent=2))
        return 0 if report["status"] == "direct_match" else 2
    finally:
        reader.close()


def clock_only(args) -> int:
    reader = ProcessReader(args.pid)
    try:
        identity = reader.identity(args.expect_sha256)
        if reader.exit_code() != STILL_ACTIVE:
            raise CaptureError("target process has already exited")
        started = time.perf_counter_ns()
        for attempt in range(2):
            before = read_game_clock(reader)
            after = read_game_clock(reader)
            equal = before == after
            active = bool(before["fixed_step_active"] or after["fixed_step_active"])
            if equal and not active:
                break
        ended = time.perf_counter_ns()
        consistent = equal and not active
        report = {"schema_version": 3, "kind": "original_game_clock_only",
                  "pid": args.pid, "recorded_utc": utc_now(), "executable": identity,
                  "status": "consistent" if consistent else "torn_or_active",
                  "consistent": consistent, "clock_cells_equal": equal, "fixed_step_active": active,
                  "clock_torn": int(not consistent), "clock_retry_count": attempt,
                  "game_time": after["game_time"] if consistent else None,
                  "game_frame": after["game_frame"] if consistent else None,
                  "fixed_step_count": after["fixed_step_count"] if consistent else None,
                  "engine_dt": after["engine_dt"] if consistent else None,
                  "clock_before": {**before, "game_ptr": f"0x{before['game_ptr']:08X}"},
                  "clock_after": {**after, "game_ptr": f"0x{after['game_ptr']:08X}"},
                  "monotonic_ns": (started + ended) // 2, "sample_span": (ended - started) / 1e9,
                  "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
                  "offset_evidence": "docs/ORIGINAL_GAME_CLOCK.md",
                  "limitations": ["External clock brackets do not prove an atomic engine-tick boundary.",
                                  "Equal clocks can include pose interpolation after the fixed-step flag clears."]}
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open("x", encoding="utf-8") as stream:
                json.dump(report, stream, indent=2)
                stream.write("\n")
        print(json.dumps(report, indent=2))
        return 0 if consistent else 2
    finally:
        reader.close()


def load_fixed_units(path: Path) -> list[dict]:
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    records = data.get("units") if isinstance(data, dict) else data
    if not isinstance(records, list) or not records:
        raise CaptureError("--units-file must contain a nonempty units array")
    units = []
    seen = set()
    for item in records:
        if not isinstance(item, dict):
            raise CaptureError("each fixed unit must be an object with unit_address and label")
        value = item.get("unit_address")
        try:
            address = value if type(value) is int else int(value, 16)
        except (TypeError, ValueError):
            raise CaptureError("unit_address must be a hexadecimal string or JSON integer") from None
        pointer(address, UNIT_END)
        label = item.get("label")
        if not isinstance(label, str) or not label.strip():
            raise CaptureError("each fixed unit needs a nonempty label")
        if address in seen:
            raise CaptureError(f"duplicate fixed unit address 0x{address:08X}")
        seen.add(address)
        units.append({"unit_address": address, "label": label})
    return units


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--pid", required=True, type=int)
    selection = parser.add_mutually_exclusive_group()
    selection.add_argument("--unit", type=lambda value: int(value, 16),
                           help="hexadecimal unit base, ECX - 310h at 00825F20 entry")
    selection.add_argument("--units-file", type=Path, help="JSON units array of fixed unit_address/label objects")
    parser.add_argument("--unit-label", default="", help="supplied label for --unit or controlled discovery")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--discover-controlled", action="store_true",
                      help="one-shot read/validation of global 00E188D8; --unit optionally checks a candidate")
    mode.add_argument("--clock-only", action="store_true", help="bounded mission-clock JSON, no unit required")
    parser.add_argument("--seconds", type=float, default=20, help="wall duration, >0 and <=300 (default 20)")
    parser.add_argument("--hz", type=float, default=20, help="wall sample rate, 1..100 (default 20)")
    parser.add_argument("--output", type=Path, help="new CSV (capture) or JSON (discovery); discovery defaults to stdout")
    parser.add_argument("--manifest", type=Path, help="new JSON sidecar; default <CSV>.metadata.json")
    parser.add_argument("--expect-sha256", help="require this on-disk original executable SHA-256")
    parser.add_argument("--max-failures", type=int, default=20, help="stop after consecutive failed samples (default 20)")
    args = parser.parse_args(argv)
    if not 1 <= args.pid <= 0xFFFFFFFF:
        parser.error("--pid must be a positive DWORD")
    if args.unit is None and args.units_file is None and not (args.discover_controlled or args.clock_only):
        parser.error("--unit or --units-file is required unless --discover-controlled or --clock-only is supplied")
    if args.clock_only and (args.unit is not None or args.units_file or args.unit_label):
        parser.error("--clock-only does not take unit selection or labels")
    if args.units_file and (args.discover_controlled or args.unit_label):
        parser.error("--units-file supplies its own labels and applies only to capture")
    if args.unit is not None:
        try:
            pointer(args.unit, UNIT_END)
        except SampleError as error:
            parser.error(str(error))
    if (args.discover_controlled or args.clock_only) and args.manifest:
        parser.error("--manifest applies only to captures; one-shot modes use --output for JSON")
    try:
        args.units = (load_fixed_units(args.units_file) if args.units_file else
                      [] if args.unit is None else [{"unit_address": args.unit, "label": args.unit_label}])
    except (CaptureError, OSError, ValueError) as error:
        parser.error(str(error))
    if not math.isfinite(args.seconds) or not 0 < args.seconds <= 300:
        parser.error("--seconds must be finite, >0 and <=300")
    if not math.isfinite(args.hz) or not 1 <= args.hz <= 100:
        parser.error("--hz must be finite and in [1, 100]")
    if not 1 <= args.max_failures <= 1000:
        parser.error("--max-failures must be in [1, 1000]")
    if args.expect_sha256 and (len(args.expect_sha256) != 64
                              or any(c not in "0123456789abcdefABCDEF" for c in args.expect_sha256)):
        parser.error("--expect-sha256 must contain exactly 64 hexadecimal digits")
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    if args.output is not None:
        args.output = args.output.resolve()
    if not (args.discover_controlled or args.clock_only):
        args.output = args.output or REPO / "local" / f"original_motion_{args.pid}_{stamp}.csv"
        args.manifest = (args.manifest or args.output.with_suffix(".metadata.json")).resolve()
    if args.output is not None and args.output == args.manifest:
        parser.error("CSV and manifest must be different files")
    return args


def capture(args) -> int:
    reader = ProcessReader(args.pid)
    try:
        identity = reader.identity(args.expect_sha256)
        if reader.exit_code() != STILL_ACTIVE:
            raise CaptureError("target process has already exited")
        for path in (args.output, args.manifest):
            if path.exists():
                raise CaptureError(f"refusing to overwrite existing output: {path}")
            path.parent.mkdir(parents=True, exist_ok=True)
        metadata = {
            "schema_version": 3, "kind": "original_game_external_read_only_motion",
            "status": "capturing", "started_utc": utc_now(), "pid": args.pid,
            "unit_address": f"0x{args.units[0]['unit_address']:08X}" if len(args.units) == 1 else None,
            "units": [{"unit_address": f"0x{item['unit_address']:08X}", "label": item["label"]}
                      for item in args.units],
            "unit_id_semantics": "supplied fixed address, not a recovered game ID; unit_label is supplied text",
            "executable": identity, "csv": str(args.output), "requested_seconds": args.seconds,
            "requested_hz": args.hz, "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
            "rate_semantics": "Hz per fixed unit; units are read sequentially on each wall deadline",
            "offset_evidence": "docs/ORIGINAL_MOTION_ABI.md; docs/ORIGINAL_GAME_CLOCK.md",
            "native_motion_layout": {"body": "B=[controller+0x2C]", "motion_state": "M=[B+0x04]",
                                     "linear_velocity": "M+0x00, 3 float32, world space",
                                     "angular_velocity": "M+0x0C, 3 float32, world space",
                                     "fwd_speed": "dot(linear_velocity, pose row 2)",
                                     "yaw_rate": "dot(angular_velocity, pose row 1)"},
            "timing": {"clock": "time.perf_counter_ns", "t": "seconds from first valid sample midpoint",
                       "dt": "empty: external rows are not individual engine ticks",
                       "engine_dt": "float32 [[00E188A8]+21F0], scaled OnMove frame delta; not fixed motion dt",
                       "fixed_step_dt": "float32 [00D0DE84], native fixed simulation step",
                       "game_time": "float32 [00F876A4], native Lua GameTime including remainder outside fixed steps",
                       "game_frame": "uint32 [[00E188A8]+648], OnMove frame counter, not fixed-step count",
                       "fixed_step_count": "uint32 [00F876B0], fixed simulation-step count",
                       "sample_game_dt": "difference from previous clean same-unit game_time in this clock epoch",
                       "sample_dt": "wall seconds between valid same-unit sample midpoints; first row empty",
                       "sample_span": "seconds spent reading/validating sample",
                       "monotonic_ns": "absolute local monotonic sample midpoint"},
            "clock_policy": {"bracketing": "clock before, unit/controller/body/motion state, clock after",
                             "retry_limit": 1, "torn": "changed clock cells or active fixed-step phase",
                             "final_torn": "retain pose and brackets; blank canonical clocks and sample_game_dt",
                             "epoch": "increment on game pointer change or clean time/frame regression; never bridge reset",
                             "summary": "per-unit game/wall endpoint ratio within each clean clock epoch"},
            "limitations": ["External wall-clock sampling is not per-engine-tick or deterministic evidence.",
                            "Reads do not suspend the game; pose/body/orders may span engine updates.",
                            "Pointer-chain rechecks detect some changes, not atomicity or all unit-address reuse.",
                            "Repeated states can mean pause, idle motion, or sampling the same engine tick.",
                            "Motion-state velocity excludes the pseudo-velocity added by position integration.",
                            "Equal clock brackets do not prove pose atomicity; interpolation jobs run after the fixed-step flag clears.",
                            "Clock resets and pause must be interpreted in their native lifecycle, not as process uptime.",
                            "Read-only capture does not prove visual behavior or reconstruction parity."],
            "range_checks": {"position_abs_max": 1e8, "linear_velocity_abs_max": 1e5,
                             "angular_velocity_abs_max": 1e4, "order_abs_max": 8,
                             "axis_squared_length": [0.5, 2], "up_forward_dot_abs_max": 0.25},
            "attempted_samples": 0, "successful_samples": 0, "failed_samples": 0,
            "skipped_deadlines": 0, "failure_counts": {}, "failure_examples": [],
            "controlled_global": {
                "global_address": f"0x{CONTROLLED_GLOBAL:08X}",
                "evidence": "include/bsp/controlled_unit.hpp; store 004C0893",
                "semantics": "before/after each valid sample; direct equality only, never auto-follow selection",
                "relationship_counts": {}, "first_valid_sample": None, "last_valid_sample": None},
        }
        with args.output.open("x", newline="", encoding="utf-8") as csv_stream:
            with args.manifest.open("x", encoding="utf-8") as manifest_stream:
                json.dump(metadata, manifest_stream, indent=2)
                manifest_stream.write("\n")
            writer = csv.DictWriter(csv_stream, fieldnames=FIELDS)
            writer.writeheader()
            csv_stream.flush()
            start = time.perf_counter_ns()
            deadline = start + round(args.seconds * 1e9)
            interval = round(1e9 / args.hz)
            next_due, previous, origin, stable_identity = start, {}, None, {}
            unit_index = 0
            clock_series = {item["unit_address"]: {"clean_samples": 0, "torn_samples": 0,
                                                 "retry_samples": 0, "epochs": []} for item in args.units}
            failures = Counter()
            relationships = Counter()
            consecutive = Counter()
            result_code = 0
            try:
                while time.perf_counter_ns() < deadline:
                    remaining = next_due - time.perf_counter_ns() if unit_index == 0 else 0
                    if remaining > 0:
                        time.sleep(max(0, min(remaining, deadline - time.perf_counter_ns())) / 1e9)
                    if time.perf_counter_ns() >= deadline:
                        break
                    code = reader.exit_code()
                    if code != STILL_ACTIVE:
                        metadata.update(status="process_exited", process_exit_code=code)
                        result_code = 2
                        break
                    metadata["attempted_samples"] += 1
                    unit_spec = args.units[unit_index]
                    unit = unit_spec["unit_address"]
                    before = time.perf_counter_ns()
                    controlled_before, controlled_before_error = read_controlled(reader)
                    try:
                        sample, current_identity = clocked_motion_sample(reader, unit)
                        if unit in stable_identity and current_identity != stable_identity[unit]:
                            raise IdentityChanged("class/controller/body/motion-state identity changed from first valid sample")
                        stable_identity[unit] = current_identity
                    except SampleError as error:
                        consecutive[unit] += 1
                        metadata["failed_samples"] += 1
                        failures[str(error)] += 1
                        if len(metadata["failure_examples"]) < 10:
                            metadata["failure_examples"].append({"t_from_start": (time.perf_counter_ns() - start) / 1e9,
                                                                 "unit_address": f"0x{unit:08X}",
                                                                 "error": str(error)})
                        if isinstance(error, IdentityChanged) or consecutive[unit] >= args.max_failures:
                            metadata["status"] = "identity_changed" if isinstance(error, IdentityChanged) else "failed_sample_limit"
                            result_code = 2
                            break
                    else:
                        controlled_after, controlled_after_error = read_controlled(reader)
                        observation = controlled_observation(unit, controlled_before, controlled_after,
                                                             (controlled_before_error, controlled_after_error))
                        relationship = observation["relationship"]
                        relationships[relationship] += 1
                        controlled_meta = metadata["controlled_global"]
                        if controlled_meta["first_valid_sample"] is None:
                            controlled_meta["first_valid_sample"] = observation
                        controlled_meta["last_valid_sample"] = observation
                        sample.update(controlled_global_before=observation["pointer_before"] or "",
                                      controlled_global_after=observation["pointer_after"] or "",
                                      controlled_relationship=relationship)
                        after = time.perf_counter_ns()
                        midpoint = (before + after) // 2
                        if origin is None:
                            origin = midpoint
                        record_game_clock(clock_series[unit], sample, midpoint)
                        sample.update(t=(midpoint - origin) / 1e9, dt="", unit_label=unit_spec["label"],
                                      sample_dt="" if unit not in previous else (midpoint - previous[unit]) / 1e9,
                                      monotonic_ns=midpoint, wall_utc=utc_now(), sample_span=(after - before) / 1e9)
                        writer.writerow(sample)
                        csv_stream.flush()
                        previous[unit], consecutive[unit] = midpoint, 0
                        metadata["successful_samples"] += 1
                    unit_index = (unit_index + 1) % len(args.units)
                    if unit_index == 0:
                        next_due += interval
                        now = time.perf_counter_ns()
                        if next_due < now:
                            skipped = (now - next_due) // interval + 1
                            next_due += skipped * interval
                            metadata["skipped_deadlines"] += skipped
                if metadata["status"] == "capturing":
                    metadata["status"] = "completed" if metadata["successful_samples"] else "no_valid_samples"
                    result_code = 0 if metadata["successful_samples"] else 2
            except KeyboardInterrupt:
                metadata["status"], result_code = "interrupted", 130
            except (CaptureError, OSError) as error:
                metadata.update(status="process_unavailable_or_io_error", terminal_error=str(error))
                result_code = 2
            finally:
                metadata.update(ended_utc=utc_now(), elapsed_wall_seconds=(time.perf_counter_ns() - start) / 1e9,
                                failure_counts=dict(failures))
                metadata["controlled_global"]["relationship_counts"] = dict(relationships)
                for series in clock_series.values():
                    finish_game_clock(series)
                metadata["game_clock_summary"] = {f"0x{unit:08X}": series for unit, series in clock_series.items()}
                # We own this exclusively-created sidecar; preserve incomplete status
                # if an external termination prevents reaching this final update.
                with args.manifest.open("w", encoding="utf-8") as stream:
                    json.dump(metadata, stream, indent=2)
                    stream.write("\n")
            print(json.dumps({key: metadata[key] for key in
                              ("status", "successful_samples", "failed_samples", "elapsed_wall_seconds")}))
            print(f"CSV: {args.output}\nMetadata: {args.manifest}")
            return result_code
    finally:
        reader.close()


def main(argv=None) -> int:
    args = parse_args(argv)
    try:
        if args.clock_only:
            return clock_only(args)
        return discover_controlled(args) if args.discover_controlled else capture(args)
    except (CaptureError, OSError, ValueError) as error:
        print(f"original_motion_capture: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
