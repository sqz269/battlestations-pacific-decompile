#!/usr/bin/env python3
"""Read-only, wall-clock motion samples from the original x86 game on Windows.

The caller supplies a unit discovered at 00825F20 (ECX - 310h). Offsets and
evidence are in docs/MOTION_DIFFERENTIAL.md section 3. This tool neither starts
nor stops the game, attaches a debugger, injects code, nor writes process memory.
It opens only PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ.

The CSV is accepted by tools/motion_trace_compare.py, but is NOT a tick trace:
dt and engine_dt are empty, sample_dt is a wall-clock interval, and t uses a
monotonic clock. Remote reads are not atomic and repeated engine states are
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
    "controller_ptr", "rigidbody_ptr", "smoothed_rudder", "up_x", "up_y", "up_z",
    "vel_x", "vel_y", "vel_z", "angvel_x", "angvel_y", "angvel_z",
    "controlled_global_before", "controlled_global_after", "controlled_relationship",
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


def motion_sample(reader: ProcessReader, unit: int) -> tuple[dict, tuple[int, int, int]]:
    """Read a unit/controller/body snapshot, then recheck the pointer chain."""
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
    pointer(body, 0x18)
    velocity = struct.unpack("<6f", reader.read(body, 0x18))
    recheck = reader.read(controller + 0x1C, 0x14)
    if (reader.u32(unit + 0x1018) != controller or reader.u32(unit + 0x538) != class_ptr
            or struct.unpack_from("<I", recheck, 0)[0] != unit
            or struct.unpack_from("<I", recheck, 0x10)[0] != body):
        raise SampleError("unit/controller/body pointer chain changed during sample")
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
                  controller_ptr=f"0x{controller:08X}", rigidbody_ptr=f"0x{body:08X}")
    return result, (class_ptr, controller, body)


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
        report = {"schema_version": 1, "kind": "original_game_controlled_unit_discovery",
                  "pid": args.pid, "recorded_utc": utc_now(), "executable": identity,
                  "status": "no_valid_ship_state", "unit_address": f"0x{unit:08X}",
                  "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
                  "limitations": ["Unsuspended reads are not atomic or a motion-tick trace.",
                                  "A different pointer leaves any driven-sub-unit relationship unverified.",
                                  "A valid ship snapshot does not establish the mission or visible state."]}
        try:
            pointer(unit, UNIT_END)
            sample, _ = motion_sample(reader, unit)
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


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--pid", required=True, type=int)
    parser.add_argument("--unit", type=lambda value: int(value, 16),
                        help="hexadecimal unit base, ECX - 310h at 00825F20 entry")
    parser.add_argument("--discover-controlled", action="store_true",
                        help="one-shot read/validation of global 00E188D8; --unit optionally checks a candidate")
    parser.add_argument("--seconds", type=float, default=20, help="wall duration, >0 and <=300 (default 20)")
    parser.add_argument("--hz", type=float, default=20, help="wall sample rate, 1..100 (default 20)")
    parser.add_argument("--output", type=Path, help="new CSV (capture) or JSON (discovery); discovery defaults to stdout")
    parser.add_argument("--manifest", type=Path, help="new JSON sidecar; default <CSV>.metadata.json")
    parser.add_argument("--expect-sha256", help="require this on-disk original executable SHA-256")
    parser.add_argument("--max-failures", type=int, default=20, help="stop after consecutive failed samples (default 20)")
    args = parser.parse_args(argv)
    if not 1 <= args.pid <= 0xFFFFFFFF:
        parser.error("--pid must be a positive DWORD")
    if args.unit is None and not args.discover_controlled:
        parser.error("--unit is required unless --discover-controlled is supplied")
    if args.unit is not None:
        try:
            pointer(args.unit, UNIT_END)
        except SampleError as error:
            parser.error(str(error))
    if args.discover_controlled and args.manifest:
        parser.error("--manifest applies only to captures; discovery uses --output for its JSON report")
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
    if not args.discover_controlled:
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
            "schema_version": 1, "kind": "original_game_external_read_only_motion",
            "status": "capturing", "started_utc": utc_now(), "pid": args.pid,
            "unit_address": f"0x{args.unit:08X}", "unit_id_semantics": "supplied address, not a recovered game ID",
            "executable": identity, "csv": str(args.output), "requested_seconds": args.seconds,
            "requested_hz": args.hz, "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
            "offset_evidence": "docs/MOTION_DIFFERENTIAL.md section 3; 00825F20 unit=ECX-310h",
            "timing": {"clock": "time.perf_counter_ns", "t": "seconds from first valid sample midpoint",
                       "dt": "empty: engine frame delta not observed", "engine_dt": "empty: unavailable",
                       "sample_dt": "seconds between valid sample midpoints; first row empty",
                       "sample_span": "seconds spent reading/validating sample",
                       "monotonic_ns": "absolute local monotonic sample midpoint"},
            "limitations": ["External wall-clock sampling is not per-engine-tick or deterministic evidence.",
                            "Reads do not suspend the game; pose/body/orders may span engine updates.",
                            "Pointer-chain rechecks detect some changes, not atomicity or all unit-address reuse.",
                            "Repeated states can mean pause, idle motion, or sampling the same engine tick.",
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
            next_due, previous, origin, stable_identity = start, None, None, None
            failures = Counter()
            relationships = Counter()
            consecutive = 0
            result_code = 0
            try:
                while time.perf_counter_ns() < deadline:
                    remaining = next_due - time.perf_counter_ns()
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
                    before = time.perf_counter_ns()
                    controlled_before, controlled_before_error = read_controlled(reader)
                    try:
                        sample, current_identity = motion_sample(reader, args.unit)
                        if stable_identity is not None and current_identity != stable_identity:
                            raise IdentityChanged("class/controller/body identity changed from first valid sample")
                        stable_identity = current_identity
                    except SampleError as error:
                        consecutive += 1
                        metadata["failed_samples"] += 1
                        failures[str(error)] += 1
                        if len(metadata["failure_examples"]) < 10:
                            metadata["failure_examples"].append({"t_from_start": (time.perf_counter_ns() - start) / 1e9,
                                                                 "error": str(error)})
                        if isinstance(error, IdentityChanged) or consecutive >= args.max_failures:
                            metadata["status"] = "identity_changed" if isinstance(error, IdentityChanged) else "failed_sample_limit"
                            result_code = 2
                            break
                    else:
                        controlled_after, controlled_after_error = read_controlled(reader)
                        observation = controlled_observation(args.unit, controlled_before, controlled_after,
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
                        sample.update(t=(midpoint - origin) / 1e9, dt="", engine_dt="",
                                      sample_dt="" if previous is None else (midpoint - previous) / 1e9,
                                      monotonic_ns=midpoint, wall_utc=utc_now(), sample_span=(after - before) / 1e9)
                        writer.writerow(sample)
                        csv_stream.flush()
                        previous, consecutive = midpoint, 0
                        metadata["successful_samples"] += 1
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
        return discover_controlled(args) if args.discover_controlled else capture(args)
    except (CaptureError, OSError, ValueError) as error:
        print(f"original_motion_capture: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
