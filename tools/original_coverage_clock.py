#!/usr/bin/env python3
"""Read one original-game clock; publish a nonce-bound request, never stop it."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import stat
import struct
import sys
import time
import uuid

from original_motion_capture import (CaptureError, CONTROLLED_GLOBAL, GAME_GLOBAL,
                                     ProcessReader, SampleError, STILL_ACTIVE,
                                     pointer, read_game_clock, utc_now)

ROOT = Path(__file__).resolve().parents[1]
LOCAL = ROOT / "local"
SCENE = "universe/Scenes/missions/USN/usn_19_coralus.scn"
LAUNCH_SCHEMA = "bsp.original-baseline.drcov.launch.v1"
STOP_SCHEMA = "bsp.original-coverage.stop.v1"


def no_reparse(path: Path):
    for item in [path, *path.parents]:
        try:
            attributes = item.lstat().st_file_attributes
        except FileNotFoundError:
            continue
        if attributes & stat.FILE_ATTRIBUTE_REPARSE_POINT:
            raise ValueError(f"reparse point refused: {item}")


def local_path(value: str, *, fresh=False) -> Path:
    path = Path(os.path.abspath(ROOT / value))
    if not path.is_relative_to(LOCAL) or path == LOCAL:
        raise ValueError("observer evidence/control paths must stay inside this repo's local/")
    no_reparse(path)
    if fresh and path.exists():
        raise FileExistsError(f"refusing existing output/control file: {path}")
    return path


def private_executable(value: str) -> Path:
    path = Path(value)
    if not path.is_absolute() or path.name.lower() != "battlestationspacific.exe":
        raise ValueError("--expected-exe must be an absolute private original-executable path")
    no_reparse(path)
    if "local" not in [part.lower() for part in path.parts] or path.stat().st_nlink != 1:
        raise ValueError("expected executable must be an independent private copy under local/")
    return path.resolve(strict=True)


def retry_sharing(operation):
    deadline = time.monotonic() + 2.0
    while True:
        try:
            return operation()
        except OSError as error:
            if getattr(error, "winerror", None) not in (5, 32, 33) or time.monotonic() >= deadline:
                raise
            time.sleep(0.05)


def publish_json(path: Path, value: dict) -> str:
    """Windows rename is atomic and refuses an existing destination."""
    if os.name != "nt":
        raise ValueError("exclusive atomic control publication requires Windows")
    local_path(str(path), fresh=True)
    path.parent.mkdir(parents=True, exist_ok=True)
    no_reparse(path)
    temporary = path.with_name(f".{path.name}.{uuid.uuid4().hex}.tmp")
    data = (json.dumps(value, indent=2, allow_nan=False) + "\n").encode("utf-8")
    with temporary.open("xb") as stream:
        stream.write(data)
        stream.flush()
        os.fsync(stream.fileno())
    # No replace/overwrite. On publication failure retain our complete temp evidence.
    retry_sharing(lambda: os.rename(temporary, path))
    return hashlib.sha256(data).hexdigest()


def receipt_identity(receipt: dict, args, expected: Path) -> int:
    if receipt.get("schema") != LAUNCH_SCHEMA or receipt.get("admitted") is not True:
        raise ValueError("launch receipt is not an admitted original drcov launch")
    for field in ("root_executable", "root_executable_sha256", "stop_request_path",
                  "run_directory", "stop_nonce", "scene_log_path", "direct_scene_path"):
        if not isinstance(receipt.get(field), str) or not receipt[field]:
            raise ValueError(f"launch receipt requires string {field}")
    created = receipt.get("root_created_filetime")
    if type(created) is not int or created <= 0:
        raise ValueError("root_created_filetime must be an exact positive integer")
    if (type(receipt.get("root_pid")) is not int or receipt["root_pid"] != args.pid
            or Path(receipt.get("root_executable", "")) != expected
            or receipt.get("root_executable_sha256", "").lower() != args.expect_sha256):
        raise ValueError("launch receipt PID/path/hash differs from the required fixed target")
    if local_path(receipt.get("stop_request_path", "")) != args.stop_request:
        raise ValueError("stop-request path differs from launch receipt")
    run = local_path(receipt.get("run_directory", ""))
    if not args.stop_request.is_relative_to(run) or not args.output.is_relative_to(run):
        raise ValueError("output and stop request must belong to the admitted run directory")
    if not re.fullmatch(r"[0-9a-fA-F]{16}", receipt.get("stop_nonce", "")):
        raise ValueError("launch receipt needs a 16-hex stop_nonce")
    if (type(receipt.get("scene_log_start_size")) is not int
            or receipt["scene_log_start_size"] != 0
            or receipt.get("scene_log_reset_before_launch") is not True):
        raise ValueError("scene log must be privately reset/fresh before launch, with start size zero")
    if str(receipt.get("direct_scene_path", "")).replace("\\", "/").lower() != args.scene_marker.replace("\\", "/").lower():
        raise ValueError("receipt direct scene differs from the required scene-log marker")
    scene_log = Path(receipt.get("scene_log_path", ""))
    if not scene_log.is_absolute() or "local" not in [part.lower() for part in scene_log.parts]:
        raise ValueError("scene log must be a declared private absolute path")
    no_reparse(scene_log)
    return created


class SceneMarker:
    """Require engine load + later SceneInit lines; never accept an argv echo."""
    def __init__(self, receipt: dict, marker: str, read_chunk=1024 * 1024):
        self.path = Path(receipt["scene_log_path"])
        self.marker = marker.replace("\\", "/").lower().encode("ascii")
        if not 1 <= read_chunk <= 1024 * 1024:
            raise ValueError("scene-log read chunk must be bounded to 1 MiB")
        self.read_chunk = read_chunk
        self.offset = 0
        self.pending = b""
        self.discarding = False
        self.oversize_lines = 0
        self.identity = None
        self.found = None
        self.loading_scene = self.scene_init = None
        self.epochs = 0
        self.epoch_history = []

    def reset(self, reason):
        self.epoch_history.append({"file_epoch": self.epochs, "reset_reason": reason,
                                   "loading_scene": self.loading_scene, "scene_init": self.scene_init})
        self.epoch_history = self.epoch_history[-8:]
        self.offset, self.pending, self.discarding = 0, b"", False
        self.loading_scene = self.scene_init = self.found = None
        self.epochs += 1

    def line(self, data: bytes, offset: int):
        normalized = data.rstrip(b"\r\n").replace(b"\\", b"/").lower()
        prefix = b" [engine] "
        position = normalized.find(prefix)
        if position < 0:
            return
        raw_payload = normalized[position + len(prefix):]
        payload = raw_payload.strip(b" \t")
        payload_offset = offset + position + len(prefix) + len(raw_payload) - len(raw_payload.lstrip(b" \t"))
        if payload.startswith(b"loading scene "):
            self.loading_scene = self.scene_init = self.found = None
            if payload == b"loading scene " + self.marker:
                self.loading_scene = {"line_byte_offset": offset, "byte_offset": payload_offset,
                                      "scene_path_byte_offset": payload_offset + len(b"loading scene "),
                                      "file_epoch": self.epochs, "observed_utc": utc_now()}
        elif (payload == b"ggame::sceneinit()" and self.loading_scene
              and offset > self.loading_scene["line_byte_offset"]):
            self.scene_init = {"line_byte_offset": offset, "byte_offset": payload_offset,
                               "file_epoch": self.epochs, "observed_utc": utc_now()}
            self.found = {"path": str(self.path), "marker": self.marker.decode(),
                          "byte_offset": self.loading_scene["scene_path_byte_offset"],
                          "loading_scene": self.loading_scene, "scene_init": self.scene_init,
                          "observed_utc": utc_now(), "file_epoch": self.epochs}

    def poll(self):
        try:
            no_reparse(self.path)
            info = self.path.stat()
            identity = (info.st_dev, info.st_ino)
            if self.identity is not None and (identity != self.identity or info.st_size < self.offset):
                self.reset("replaced" if identity != self.identity else "truncated")
            self.identity = identity
            with self.path.open("rb") as stream:
                stream.seek(self.offset)
                data = stream.read(self.read_chunk)
            start = self.offset - len(self.pending)
            self.offset += len(data)
            combined, self.pending = self.pending + data, b""
            parts = combined.split(b"\n")
            for index, content in enumerate(parts):
                if index == len(parts) - 1:
                    part = content
                    if self.discarding or len(part) > 65536:
                        if not self.discarding:
                            self.oversize_lines += 1
                        self.discarding = True
                    else:
                        self.pending = part
                    break
                part = content + b"\n"
                if self.discarding:
                    self.discarding = False
                elif len(part) <= 65536:
                    self.line(part, start)
                else:
                    self.oversize_lines += 1
                start += len(part)
        except FileNotFoundError:
            if self.identity is not None:
                self.reset("missing")
                self.identity = None
        return self.found

    def report(self):
        return {"file_epoch": self.epochs, "loading_scene": self.loading_scene,
                "scene_init": self.scene_init, "bytes_read": self.offset,
                "oversize_lines_discarded": self.oversize_lines,
                "recent_reset_epochs": self.epoch_history}


def read_context(reader, game: int) -> dict:
    controlled = reader.u32(CONTROLLED_GLOBAL)
    if controlled:
        pointer(controlled)
    non_campaign = reader.u32(game + 0x1FE4)
    effective, = struct.unpack("<i", reader.read(game + 0x6AC, 4))
    if reader.u32(GAME_GLOBAL) != game:
        raise SampleError("game singleton changed during context read")
    difficulty = 2 if non_campaign != 0 else effective
    return {"non_campaign_flag": non_campaign,
            "effective_difficulty": effective, "reported_get_difficulty": difficulty,
            "difficulty_matches_bsp_reference_1": difficulty == 1,
            "controlled_pointer": f"0x{controlled:08X}"}


def bracket_clock(reader) -> dict:
    for attempt in range(2):
        before = read_game_clock(reader)
        context = read_context(reader, before["game_ptr"])
        context_after = read_context(reader, before["game_ptr"])
        after = read_game_clock(reader)
        reasons = []
        if before != after:
            reasons.append("clock_changed")
        if context != context_after:
            reasons.append("context_changed")
        if before["fixed_step_active"] or after["fixed_step_active"]:
            reasons.append("fixed_step_active")
        if not reasons:
            break
    return {"clock": None if reasons else after, "context": context_after,
            "clock_before": before, "clock_after": after,
            "clock_torn": bool(reasons), "clock_retry_count": attempt,
            "clock_torn_reason": reasons}


class ClockWindow:
    def __init__(self, target: float):
        self.target = target
        self.first_clean = self.first = self.last = self.last_observation = None
        self.epochs = []
        self.clean = self.torn = self.retries = self.failures = 0
        self.last_error = None

    def observe(self, sample: dict, wall_seconds: float, scene_ready: bool):
        observation = {"wall_seconds": wall_seconds, "utc": utc_now(), **sample}
        self.last_observation = observation
        self.retries += sample["clock_retry_count"]
        if sample["clock_torn"]:
            self.torn += 1
            return None
        self.clean += 1
        self.first_clean = self.first_clean or observation
        clock = sample["clock"]
        if self.last:
            previous = self.last["clock"]
            reset = [field for field in ("game_time", "game_frame", "fixed_step_count")
                     if clock[field] < previous[field]]
            if clock["game_ptr"] != previous["game_ptr"]:
                reset.append("game_ptr")
            if reset:
                self.epochs.append({"index": 1, "reset_fields": reset, "first": observation,
                                    "previous": self.last})
                self.last = observation
                return "clock_reset"
        ready = scene_ready and clock["fixed_step_count"] > 0 and clock["game_time"] > 0
        if not self.first:
            if not ready:
                return None
            self.first = observation
            self.epochs.append({"index": 0, "first": observation})
        self.last = observation
        self.epochs[0]["last"] = observation
        return "game_time_target" if ready and clock["game_time"] >= self.target else None

    def report(self):
        return {"first_clean_clock_read": self.first_clean, "first_mission_clock": self.first,
                "end_clock": self.last, "last_observation": self.last_observation,
                "epochs": self.epochs, "clean_reads": self.clean, "torn_reads": self.torn,
                "retry_samples": self.retries, "failed_reads": self.failures,
                "last_read_error": self.last_error}


def same_identity(identity: dict, expected: Path, digest: str, created: int):
    if (Path(identity["path"]) != expected or identity["disk_sha256"] != digest
            or type(identity["process_created_filetime"]) is not int
            or identity["process_created_filetime"] != created):
        raise CaptureError("live process identity differs from the admitted fixed target")


def monitor(args, reader, marker, window, started, *, monotonic=time.monotonic,
            sleep=time.sleep, drain=lambda: (Path.home() / ".bsp/DRAIN").exists()):
    while True:
        elapsed = monotonic() - started
        if reader.exit_code() != STILL_ACTIVE:
            return "process_exited"
        if drain():
            return "drain"
        if elapsed >= args.wall_timeout:
            return "timeout"
        try:
            scene_ready = bool(marker.poll())
            reason = window.observe(bracket_clock(reader), elapsed, scene_ready)
            if reason:
                return reason
        except (SampleError, OSError) as error:
            window.failures += 1
            window.last_error = str(error)
        sleep(min(args.poll_seconds, max(0, args.wall_timeout - (monotonic() - started))))


def run(args):
    expected = private_executable(args.expected_exe)
    for path in (args.output, args.stop_request):
        local_path(str(path), fresh=True)
    if len({args.output, args.stop_request, args.launch_receipt}) != 3:
        raise ValueError("receipt, output and stop paths must be distinct")
    started = time.monotonic()
    window = ClockWindow(args.target_game_seconds)
    reader = None
    report = {"schema": "bsp.original-coverage.clock.v1", "pid": args.pid,
              "expected_exe": str(expected), "expect_sha256": args.expect_sha256,
              "started_utc": utc_now(), "target_absolute_game_time": args.target_game_seconds,
              "wall_timeout": args.wall_timeout, "poll_seconds": args.poll_seconds,
              "launch_receipt": str(args.launch_receipt), "stop_request_published": False,
              "offset_evidence": ["docs/ORIGINAL_GAME_CLOCK.md", "src/lua_binding_core.cpp:26"],
              "read_rights": ["PROCESS_QUERY_LIMITED_INFORMATION", "PROCESS_VM_READ"],
              "limitations": ["Unsuspended brackets are not an atomic engine-tick snapshot.",
                             "game_frame is AppOnMove, not a mission-relative frame count.",
                             "Target is absolute native GameTime in the first admitted mission epoch.",
                             "Difficulty differences are recorded, never changed."]}
    try:
        while True:
            if (Path.home() / ".bsp/DRAIN").exists():
                report["reason"] = "drain_before_admission"
                return report
            if time.monotonic() - started >= args.wall_timeout:
                report["reason"] = "timeout_before_admission"
                return report
            try:
                receipt = retry_sharing(lambda: json.loads(args.launch_receipt.read_text(encoding="utf-8-sig")))
                if not isinstance(receipt, dict):
                    raise ValueError("launch receipt must be a JSON object")
                if receipt.get("admitted") is True:
                    break
            except (FileNotFoundError, json.JSONDecodeError):
                pass
            time.sleep(args.poll_seconds)
        created = receipt_identity(receipt, args, expected)
        reader = ProcessReader(args.pid)
        identity = reader.identity(args.expect_sha256)
        same_identity(identity, expected, args.expect_sha256, created)
        report.update(executable=identity, process_created_filetime=created,
                      scene_log_fresh_contract={key: receipt[key] for key in
                                               ("scene_log_path", "scene_log_start_size", "scene_log_reset_before_launch")})
        marker = SceneMarker(receipt, args.scene_marker)
        reason = monitor(args, reader, marker, window, started)
        report.update(reason=reason, scene_marker=marker.found, scene_log_epochs=marker.epochs,
                      scene_gate=marker.report())
        if reason in ("game_time_target", "clock_reset", "timeout", "drain"):
            same_identity(reader.identity(args.expect_sha256), expected, args.expect_sha256, created)
            report["identity_verified_before_request"] = True
            clock = ({**window.last["clock"], "context": window.last["context"],
                      "observed_wall_seconds": window.last["wall_seconds"],
                      "observed_utc": window.last["utc"]} if window.last else None)
            request = {"schema": STOP_SCHEMA, "pid": args.pid,
                       "process_created_filetime": created, "expected_exe": str(expected),
                       "nonce": receipt["stop_nonce"], "reason": reason,
                       "clock": clock, "epochs": window.epochs, "recorded_utc": utc_now()}
            report["stop_request_sha256"] = publish_json(args.stop_request, request)
            report["stop_request_published"] = True
        return report
    except KeyboardInterrupt:
        report["reason"] = "observer_interrupted"
        return report
    except (CaptureError, OSError, ValueError) as error:
        report.update(reason="observer_error", error=str(error))
        return report
    finally:
        report.update(window.report(), finished_utc=utc_now(), wall_seconds=time.monotonic() - started)
        if reader:
            reader.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pid", required=True, type=int)
    parser.add_argument("--expected-exe", required=True)
    parser.add_argument("--expect-sha256", required=True, type=str.lower)
    parser.add_argument("--launch-receipt", required=True, type=local_path)
    parser.add_argument("--target-game-seconds", type=float, default=240)
    parser.add_argument("--wall-timeout", type=float, default=3600)
    parser.add_argument("--poll-seconds", type=float, default=0.1)
    parser.add_argument("--scene-marker", default=SCENE)
    parser.add_argument("--output", required=True, type=local_path)
    parser.add_argument("--stop-request", required=True, type=local_path)
    args = parser.parse_args()
    if (args.pid <= 0 or not re.fullmatch(r"[0-9a-f]{64}", args.expect_sha256)
            or not math.isfinite(args.target_game_seconds) or not 0 < args.target_game_seconds <= 1e9
            or not math.isfinite(args.wall_timeout) or not 0 < args.wall_timeout <= 3600
            or not math.isfinite(args.poll_seconds) or not 0.02 <= args.poll_seconds <= 10):
        parser.error("invalid PID/hash/target or wall timeout (maximum3600s)/poll interval")
    try:
        args.scene_marker.encode("ascii")
        if not args.scene_marker.lower().endswith(".scn") or len(args.scene_marker) < 12:
            raise ValueError("expected a full virtual .scn path")
    except (UnicodeEncodeError, ValueError) as error:
        parser.error(f"invalid scene marker: {error}")
    if os.name != "nt":
        parser.error("original-process observation requires Windows")
    try:
        report = run(args)
        publish_json(args.output, report)
    except (CaptureError, OSError, ValueError) as error:
        print(f"coverage clock: {error}", file=sys.stderr)
        return 2
    print(json.dumps({"output": str(args.output), "reason": report["reason"],
                      "stop_request_published": report["stop_request_published"]}))
    return 0 if report["reason"] == "game_time_target" and report["stop_request_published"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
