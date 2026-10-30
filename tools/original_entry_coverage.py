"""Opt-in exact-start first-hit collection through the existing classic guard.

No DR, trap flag, breakpoint rearm, call counts, or negative coverage claims.
The classic guard retains its token, Documents hook, source locks and job.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import json
import math
import os
import re
import secrets
import struct
import subprocess
import sys
import time

import original_baseline_guard as g

ROOT = Path(__file__).resolve().parents[1]
RETAIL_SHA = "b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6"
SCENE = "universe/Scenes/missions/USN/usn_19_coralus.scn"
TRACE_SCHEMA = "bsp.original-entry.trace.v1"
LAUNCH_SCHEMA = "bsp.original-baseline.function-entry.launch.v1"
HEX256 = re.compile(r"[0-9a-f]{64}\Z")


def filetime(process):
    get_times = g.api(g.K, "GetProcessTimes", g.W.BOOL, g.HANDLE,
                      *([C.POINTER(g.W.FILETIME)] * 4))
    fields = [g.W.FILETIME() for _ in range(4)]
    g.checked(get_times(process, *[C.byref(x) for x in fields]), "entry process creation time")
    return (fields[0].dwHighDateTime << 32) | fields[0].dwLowDateTime


def records(rows, root, ignored=()):
    result = {}
    for row in rows:
        name = row.get("path")
        if (not isinstance(name, str) or not name or Path(name).is_absolute()
                or ".." in Path(name).parts or name in result):
            raise RuntimeError("Invalid original preparation record")
        path = g.ordinary(root / name)
        if path == root or not path.is_relative_to(root):
            raise RuntimeError("Original preparation record escapes its declared root")
        actual = {"sha256": g.sha(path), "length": path.stat().st_size}
        if name.lower() not in ignored and actual != {k: row[k] for k in actual}:
            raise RuntimeError(f"Preparation identity changed: {path}")
        result[name] = actual
    return result


class EntrySession:
    """Lifecycle callbacks invoked only by the classic guard's debugger thread."""

    def __init__(self, executable, manifest, run_directory, *, probe=False):
        import pefile
        self.executable, self.manifest = g.private(executable), g.private(manifest)
        if self.manifest.stat().st_nlink != 1 or self.manifest.stat().st_size > 8 * 1024 * 1024:
            raise RuntimeError("Invalid private entry manifest")
        self.run = g.private(run_directory)
        self.probe = probe
        manifest_bytes = self.manifest.read_bytes()
        self.manifest_sha = hashlib.sha256(manifest_bytes).hexdigest()
        self.targets = json.loads(manifest_bytes.decode("utf-8-sig"))
        image = pefile.PE(str(self.executable))
        try:
            self.image_base = image.OPTIONAL_HEADER.ImageBase
            self.image_size = image.OPTIONAL_HEADER.SizeOfImage
            self.image_sha = g.sha(self.executable)
            if (self.targets.get("schema") != "bsp.original-entry.targets.v1"
                    or self.targets.get("image_sha256") != self.image_sha
                    or type(self.targets.get("image_base")) is not int
                    or self.targets["image_base"] != self.image_base
                    or type(self.targets.get("image_size")) is not int
                    or self.targets["image_size"] != self.image_size
                    or any(not isinstance(self.targets.get(k), str) or not HEX256.fullmatch(self.targets[k])
                           for k in ("index_sha256", "wire_sha256"))):
                raise RuntimeError("Entry manifest image/provenance identity mismatch")
            points = self.targets.get("points")
            if not isinstance(points, list) or not 1 <= len(points) <= 30000:
                raise RuntimeError("Invalid bounded entry point list")
            self.points, self.static_skips = {}, []
            previous = -1
            for point in points:
                text = point.get("address") if isinstance(point, dict) else None
                if (not isinstance(text, str) or not re.fullmatch(r"[0-9a-fA-F]{8}", text)
                        or type(point.get("expected_byte")) is not int or not 0 <= point["expected_byte"] <= 255
                        or type(point.get("board_target")) is not bool):
                    raise RuntimeError("Invalid entry manifest point")
                address = int(text, 16)
                if address <= previous or not self.image_base <= address < self.image_base + self.image_size:
                    raise RuntimeError("Entry manifest points must be sorted, unique and inside the image")
                previous = address
                rva = address - self.image_base
                section = next((s for s in image.sections if s.VirtualAddress <= rva
                    < s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData)), None)
                reason = None
                if section is None or not section.Characteristics & 0x20000000:
                    reason = "not_executable"
                elif rva - section.VirtualAddress >= section.SizeOfRawData:
                    reason = "not_file_backed"
                elif point["expected_byte"] == 0xCC:
                    reason = "preexisting_int3"
                elif image.get_data(rva, 1) != bytes([point["expected_byte"]]):
                    reason = "manifest_byte_differs_from_image"
                if reason:
                    self.static_skips.append({"address": f"{address:08x}", "reason": reason})
                else:
                    self.points[address] = point["expected_byte"]
            if not probe and (self.image_sha != RETAIL_SHA or self.image_base != 0x400000
                    or len(points) != 21770 or sum(p["board_target"] for p in points) != 20892):
                raise RuntimeError("Unsupported production image or frozen point counts")
        finally:
            image.close()
        self.armed, self.fired = set(), set()
        self.queued_duplicates = 0
        self.skips = list(self.static_skips)
        self.stream = self.observer = self.observer_stdio = None
        self.stop = None
        self.last_publish = 0.0
        self.admitted = False
        self.nonce = secrets.token_hex(8)
        self.run_id = self.run.name
        self.trace = self.run / "entry-hits.jsonl"
        self.stop_path = self.run / "stop-request.json"
        self.receipt = None

    def bind(self, data, receipt):
        self.data, self.receipt = data, receipt
        data.update(schema=LAUNCH_SCHEMA, tracer_method="debugger-function-entries",
            run_id=self.run_id, run_directory=str(self.run), entry_trace_path=str(self.trace),
            manifest_path=str(self.manifest), manifest_sha256=self.manifest_sha,
            stop_request_path=str(self.stop_path), stop_nonce=self.nonce, admitted=False,
            root_executable=str(self.executable), root_executable_sha256=self.image_sha,
            scene_log_path=str(self.executable.parent / "bsp_debug.log"),
            scene_log_start_size=0, scene_log_reset_before_launch=not self.probe,
            direct_scene_path=SCENE, coverage_semantics="first exact entry hit; all unhit UNKNOWN; no call or BB counts",
            debugger_active=True, active_process_limit_added=False)

    def created(self, process, pid):
        self.pid, self.created_filetime = pid, filetime(process)
        self.data["root_created_filetime"] = self.created_filetime
        self.stream = open(self.trace, "x", encoding="utf-8", newline="\n", buffering=65536)
        self.emit({"kind": "header", "schema": TRACE_SCHEMA, "run_id": self.run_id,
            "pid": pid, "process_created_filetime": self.created_filetime,
            "image_sha256": self.image_sha, "image_base": self.image_base, "image_size": self.image_size,
            "index_sha256": self.targets["index_sha256"], "wire_sha256": self.targets["wire_sha256"],
            "manifest_sha256": self.manifest_sha, "target_count": len(self.targets["points"]),
            "board_target_count": sum(p["board_target"] for p in self.targets["points"]),
            "tracer_method": "debugger-function-entries", "unhit": "UNKNOWN", "call_counts": None, "bb_counts": None})
        self.flush()

    def emit(self, value):
        self.stream.write(json.dumps(value, separators=(",", ":"), allow_nan=False) + "\n")

    def flush(self):
        if self.stream:
            self.stream.flush()

    def summary(self):
        return {"target_count": len(self.targets["points"]), "armed_points": len(self.armed),
            "hit_count": len(self.fired), "queued_duplicate_traps": self.queued_duplicates,
            "skipped_points": len(self.skips), "trace": str(self.trace), "breakpoint_rearm": False,
            "trap_flag_used": False, "context_written": "WOW64 CONTROL/EIP only"}

    def admit(self, process, state):
        if self.admitted or state["pid"] != self.pid or state["base"] != self.image_base or not state["hooked"]:
            raise RuntimeError("Entry collection admission identity mismatch")
        pages = {}
        for address in self.points:
            pages.setdefault(address & ~4095, []).append(address)
        for addresses in pages.values():
            start, end = addresses[0], addresses[-1] + 1
            original = g.read_remote(process, start, end - start)
            patched = bytearray(original)
            for address in addresses:
                expected = self.points[address]
                if original[address - start] != expected:
                    self.skips.append({"address": f"{address:08x}", "reason": "live_byte_mismatch",
                                       "expected_byte": expected, "live_byte": original[address - start]})
                else:
                    patched[address - start] = 0xCC
                    self.armed.add(address)
            if bytes(patched) != original:
                g.replace_remote(process, start, bytes(patched))
                if g.read_remote(process, start, len(patched)) != bytes(patched):
                    raise RuntimeError("Entry breakpoint write verification failed")
        g.write_receipt(self.run / "skipped-points.json", {"points": self.skips, "unhit": "UNKNOWN"})
        self.admitted = True
        self.data.update(admitted=True, phase="running", entry_collection=self.summary())
        g.write_receipt(self.receipt, self.data)
        if not self.probe:
            observer_script = g.ordinary(ROOT / "tools/original_coverage_clock.py")
            self.observer_stdio = open(self.run / "observer-stdio.log", "xb")
            arguments = [sys.executable, str(observer_script), "--pid", str(self.pid),
                "--expected-exe", str(self.executable), "--expect-sha256", self.image_sha,
                "--launch-receipt", str(self.receipt), "--output", str(self.run / "clock-observer.json"),
                "--stop-request", str(self.stop_path), "--wall-timeout", str(self.data["seconds"]),
                "--tracer-method", "debugger-function-entries"]
            self.observer = subprocess.Popen(arguments, cwd=ROOT, stdin=subprocess.DEVNULL,
                stdout=self.observer_stdio, stderr=subprocess.STDOUT, creationflags=0x08000020)
            self.data.update(observer_pid=self.observer.pid, observer_arguments=arguments,
                             observer_source_sha256=g.sha(observer_script))
            g.write_receipt(self.receipt, self.data)

    def owns(self, address):
        return address in self.armed or address in self.fired

    def hit(self, process, thread_id, address):
        thread = g.K.OpenThread(0x18, False, thread_id)
        g.checked(thread, "OpenThread entry breakpoint")
        try:
            context = g.WOW64_CONTEXT()
            context.flags = 0x10001  # CONTROL only. Never write x87/MXCSR or TF.
            g.checked(g.K.Wow64GetThreadContext(thread, C.byref(context)), "Wow64GetThreadContext entry")
            if context.Eip != address + 1:
                raise RuntimeError("Owned entry breakpoint has unexpected WOW64 EIP")
            current = g.read_remote(process, address, 1)
            if address in self.armed:
                if current != b"\xcc":
                    raise RuntimeError("Armed entry breakpoint byte changed unexpectedly")
                g.replace_remote(process, address, bytes([self.points[address]]))
                self.armed.remove(address)
                self.fired.add(address)
                self.emit({"kind": "entry_hit", "address": f"{address:08x}", "pid": self.pid,
                    "thread_id": thread_id, "sequence": len(self.fired)})
            else:
                # Windows can already have queued traps from several threads.
                # The byte stays original; every such thread still needs rewind.
                if current != bytes([self.points[address]]):
                    raise RuntimeError("Fired entry byte changed before queued trap rewind")
                self.queued_duplicates += 1
            context.Eip = address
            g.checked(g.K.Wow64SetThreadContext(thread, C.byref(context)), "Wow64SetThreadContext entry rewind")
        finally:
            g.close(thread)

    def poll(self):
        now = time.monotonic()
        if now - self.last_publish >= .5:
            self.flush()
            self.data["entry_collection"] = self.summary()
            g.write_receipt(self.receipt, self.data)
            self.last_publish = now
        if self.data.get("last_unhandled_exception"):
            return False  # The existing classic loop must observe the real crash exit.
        if self.stop_path.exists() and self.stop is None:
            g.private(self.stop_path, self.run)
            if self.stop_path.stat().st_nlink != 1 or self.stop_path.stat().st_size > 65536:
                raise RuntimeError("Invalid entry stop request file")
            payload = json.loads(self.stop_path.read_text(encoding="utf-8"))
            if (payload.get("schema") != "bsp.original-coverage.stop.v1"
                    or type(payload.get("pid")) is not int or payload["pid"] != self.pid
                    or type(payload.get("process_created_filetime")) is not int
                    or payload["process_created_filetime"] != self.created_filetime
                    or payload.get("expected_exe") != str(self.executable) or payload.get("nonce") != self.nonce):
                raise RuntimeError("Unbound entry stop request")
            self.stop = payload
            self.data["stop_request"] = payload
            reason = payload.get("reason")
            clock = payload.get("clock", {})
            if reason != "game_time_target":
                raise RuntimeError(f"Clock observer aborted entry collection: {reason}")
            if (not isinstance(clock, dict) or type(clock.get("game_time")) not in (int, float)
                    or not math.isfinite(clock["game_time"]) or clock["game_time"] < 240
                    or type(clock.get("fixed_step_count")) is not int or clock["fixed_step_count"] <= 0):
                raise RuntimeError("Entry stop lacks the positive native 240-second clock window")
            self.flush()
            self.data.update(phase="forced_window_stop", stop_classification="forced_window_stop",
                             intentional_instrumentation_stop=True)
            return True
        if self.observer and self.observer.poll() is not None and self.observer.returncode != 0:
            raise RuntimeError(f"Clock observer exited before a qualified stop: {self.observer.returncode}")
        return bool(self.stop and self.stop.get("reason") == "game_time_target")

    def finish(self, data, failure):
        secondary = []
        scene_verified = False
        if self.observer:
            try:
                self.observer.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.observer.terminate()
                self.observer.wait(timeout=5)
                secondary.append("Owned clock observer required termination")
            data["observer_exit_code"] = self.observer.returncode
            if self.observer.returncode:
                secondary.append(f"Clock observer exit {self.observer.returncode}")
            try:
                report_path = g.private(self.run / "clock-observer.json", self.run)
                report = json.loads(report_path.read_text(encoding="utf-8"))
                marker = report.get("scene_marker", {})
                scene_verified = (report.get("schema") == "bsp.original-coverage.clock.v1"
                    and report.get("reason") == "game_time_target"
                    and type(report.get("pid")) is int and report["pid"] == self.pid
                    and type(report.get("process_created_filetime")) is int
                    and report["process_created_filetime"] == self.created_filetime
                    and report.get("expected_exe") == str(self.executable)
                    and report.get("expect_sha256") == self.image_sha
                    and report.get("launch_receipt") == str(self.receipt)
                    and report.get("identity_verified_before_request") is True
                    and report.get("stop_request_published") is True
                    and report.get("stop_request_sha256") == g.sha(self.stop_path)
                    and isinstance(marker, dict) and marker.get("marker") == SCENE.lower()
                    and marker.get("path") == data["scene_log_path"]
                    and isinstance(marker.get("loading_scene"), dict)
                    and isinstance(marker.get("scene_init"), dict))
                data["clock_observer_sha256"] = g.sha(report_path)
            except BaseException as exc:
                secondary.append(f"Observer scene proof {type(exc).__name__}: {exc}")
        if self.observer_stdio:
            self.observer_stdio.close()
        preserved = all(data.get(k) is True for k in ("protected_documents_unchanged",
            "disk_executable_unchanged", "original_executable_unchanged", "original_root_inventory_unchanged"))
        complete = (not self.probe and failure is None and not secondary and self.admitted
            and self.stop is not None and self.stop.get("reason") == "game_time_target"
            and data.get("phase") == "forced_window_stop"
            and data.get("intentional_instrumentation_stop") is True
            and scene_verified and preserved and data.get("final_job_active") == [0]
            and data.get("source_locks", {}).get("owned_job_exited_before_parent_release") is True)
        if not self.probe and not complete and failure is None:
            failure = RuntimeError("Entry collection did not complete a preserved native 240-second window")
            data.setdefault("error", str(failure))
            data["phase"] = "incomplete"
        data.update(complete_window=complete, source_preservation_verified=preserved,
                    same_scene_verified=scene_verified, entry_collection=self.summary())
        if secondary:
            data.setdefault("secondary_errors", []).extend(secondary)
        if self.stream:
            root = next((p for p in data.get("processes", []) if p.get("pid") == getattr(self, "pid", None)), {})
            self.emit({"kind": "terminal", "schema": TRACE_SCHEMA, "run_id": self.run_id,
                "pid": self.pid, "process_created_filetime": self.created_filetime,
                "hit_count": len(self.fired), "reason": self.stop.get("reason") if self.stop else data.get("phase"),
                "stop_classification": data.get("stop_classification"), "admitted": self.admitted,
                "clock": self.stop.get("clock") if self.stop else None,
                "source_preservation_verified": preserved, "final_job_active": data.get("final_job_active"),
                "same_scene_verified": scene_verified,
                "complete_window": complete, "root_exit_code": root.get("final_exit_code", root.get("exit_code")),
                "queued_duplicate_traps": self.queued_duplicates, "skipped_points": len(self.skips)})
            self.flush()
            self.stream.close()
            data["entry_trace_sha256"] = g.sha(self.trace)
        return failure


def run_guard(stage, executable, cwd, documents, receipt, seconds, original_exe,
              arguments, token_mode, manifest, handle_known_xlive_debug_close=False):
    if token_mode != "medium-write-locked" or list(arguments) != [SCENE]:
        raise RuntimeError("Entry coverage requires medium-write-locked and the exact full USN04 argument")
    if not handle_known_xlive_debug_close:
        raise RuntimeError("Classic entry coverage requires the established exact xlive debug-CloseHandle policy")
    stage, executable, documents = g.private(stage), g.private(executable), g.private(documents)
    original_exe = g.ordinary(original_exe)
    if g.sha(executable) != RETAIL_SHA or g.sha(original_exe) != RETAIL_SHA:
        raise RuntimeError("Unsupported original image for entry coverage")
    prepared = json.loads(g.private(stage / "preparation.json").read_text(encoding="utf-8-sig"))
    protected_docs = g.folder(5) / "Battlestations-Pacific"
    expected = {"stage_root": stage, "source_game": original_exe.parent, "source_profile": protected_docs,
                "private_game": executable.parent, "private_documents": documents}
    if prepared.get("schema") != "bsp.original-baseline.preparation.v1" or any(Path(prepared.get(k, "")) != v for k, v in expected.items()):
        raise RuntimeError("Entry preparation source/private paths do not bind this run")
    before = records(prepared["root_files_sha256"], original_exe.parent)
    if before.get("xlive.dll", {}).get("sha256") != g.KNOWN_XLIVE_SHA256:
        raise RuntimeError("Unsupported original xlive inventory")
    records(prepared["root_files_sha256"], executable.parent, ("bsp_debug.log", "xlive_debug.log"))
    profiles = list(prepared["profile_files_sha256"])
    records(profiles, protected_docs)
    override_path = stage / "native-options.json"
    if override_path.exists():
        override = json.loads(g.private(override_path).read_text(encoding="utf-8-sig"))
        originals = [row for row in profiles if row["path"] == "options.txt"]
        if (override.get("schema") != "bsp.original-baseline.native-options.v1"
                or override.get("original_unchanged") is not True
                or Path(override.get("source_path", "")) != protected_docs / "options.txt"
                or Path(override.get("private_path", "")) != documents / "Battlestations-Pacific/options.txt"
                or len(originals) != 1 or originals[0] != override.get("source_record")):
            raise RuntimeError("Invalid private native-options override identity")
        profiles = [row for row in profiles if row["path"] != "options.txt"] + [override["private_record"]]
    records(profiles, documents / "Battlestations-Pacific")
    run = g.private(stage / ("entry-coverage-" + time.strftime("%Y%m%dT%H%M%S", time.gmtime()) + "-" + secrets.token_hex(6)), exists=False)
    run.mkdir()
    # Native startup truncates these two declared private generated logs.
    # Archive/reset only this private epoch, never any installed log.
    for name in ("bsp_debug.log", "xlive_debug.log"):
        path = g.private(executable.parent / name, stage, exists=False)
        if path.exists():
            if not path.is_file() or path.stat().st_nlink != 1:
                raise RuntimeError("Unsafe private generated log")
            path.replace(run / (name + ".before"))
        path.write_bytes(b"")
    session = EntrySession(executable, manifest, run)
    data, failure = None, None
    try:
        data = g.run_guard(stage, executable, cwd, documents, receipt, seconds, original_exe=original_exe,
            arguments=arguments, protected_roots=[original_exe.parent, protected_docs], motion_discovery=False,
            handle_known_xlive_debug_close=True, token_mode=token_mode, entry_coverage=session)
    except BaseException as exc:
        failure = exc
        data = getattr(session, "data", None)
    finally:
        if data is not None:
            try:
                data["original_root_inventory_unchanged"] = records(prepared["root_files_sha256"], original_exe.parent) == before
            except BaseException as exc:
                data["original_root_inventory_unchanged"] = False
                data.setdefault("secondary_errors", []).append(f"Source verification {type(exc).__name__}: {exc}")
                failure = failure or exc
            try:
                failure = session.finish(data, failure)
            except BaseException as exc:
                data.setdefault("secondary_errors", []).append(f"Trace finalization {type(exc).__name__}: {exc}")
                failure = failure or exc
            g.write_receipt(receipt, data)
    if failure:
        raise RuntimeError(f"{failure}; receipt: {receipt}") from failure
    return data


PROBE_SOURCE = r'''
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
static volatile LONG calls = 0;
static HANDLE ready;
static volatile LONG waiting = 0;
#define F(N) extern "C" __declspec(dllexport) __declspec(naked) void entry_probe_##N() { \
    __asm push ebp __asm mov ebp, esp __asm lock inc calls __asm pop ebp __asm ret }
F(0) F(1) F(2) F(3) F(4) F(5) F(6) F(7)
F(8) F(9) F(10) F(11) F(12) F(13) F(14) F(15)
F(16) F(17) F(18) F(19) F(20) F(21) F(22) F(23)
F(24) F(25) F(26) F(27) F(28) F(29) F(30) F(31)
extern "C" __declspec(dllexport) __declspec(naked) void entry_probe_cc() { __asm int 3 __asm ret }
extern "C" __declspec(dllexport) __declspec(naked) void entry_probe_mismatch() { __asm ret }
extern "C" __declspec(dllexport) int entry_probe_data = 17;
typedef void (*fn)();
static fn funcs[] = {entry_probe_0,entry_probe_1,entry_probe_2,entry_probe_3,entry_probe_4,entry_probe_5,entry_probe_6,entry_probe_7,
entry_probe_8,entry_probe_9,entry_probe_10,entry_probe_11,entry_probe_12,entry_probe_13,entry_probe_14,entry_probe_15,
entry_probe_16,entry_probe_17,entry_probe_18,entry_probe_19,entry_probe_20,entry_probe_21,entry_probe_22,entry_probe_23,
entry_probe_24,entry_probe_25,entry_probe_26,entry_probe_27,entry_probe_28,entry_probe_29,entry_probe_30,entry_probe_31};
static DWORD WINAPI worker(void*) {
    InterlockedIncrement(&waiting);
    if (WaitForSingleObject(ready, 10000) != WAIT_OBJECT_0) return 3;
    for (int pass=0;pass<2;pass++) for (int i=0;i<32;i++) funcs[i]();
    return 0;
}
int main(int argc,char** argv) {
    char docs[MAX_PATH];
    if (argc != 3 || !SHGetSpecialFolderPathA(NULL,docs,CSIDL_PERSONAL|CSIDL_FLAG_CREATE,FALSE) || strcmp(docs,argv[1])) return 10;
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    HANDLE threads[64];
    for (int i=0;i<64;i++) if (!(threads[i]=CreateThread(NULL,0,worker,NULL,0,NULL))) return 11;
    while (waiting != 64) Sleep(1);
    SetEvent(ready);
    if (WaitForMultipleObjects(64,threads,TRUE,20000) != WAIT_OBJECT_0) return 12;
    for (int i=0;i<64;i++) { DWORD code=1; GetExitCodeThread(threads[i],&code); CloseHandle(threads[i]); if(code) return 13; }
    CloseHandle(ready);
    FILE* out=NULL;
    if (fopen_s(&out,argv[2],"wb")) return 14;
    fprintf(out,"%ld\n",calls); fclose(out);
    Sleep(15000); // Let the fixture collector request its explicit synthetic stop.
    return calls == 4096 ? 0 : 15;
}
'''


def selftest(root):
    """One real-thread fixture: unique traps, queued rewind, and skip reasons."""
    import pefile
    root = g.private(root, exists=False)
    root.mkdir(parents=True, exist_ok=False)
    stage, protected = root / "stage", root / "protected"
    stage.mkdir(); protected.mkdir()
    fixture = protected / "unchanged.txt"
    fixture.write_text("entry-fixture-protected\n", encoding="ascii")
    protected_sha = g.sha(fixture)
    g.prepare(stage, stage / "guard-prepare.json")
    documents = stage / "Documents"
    documents.mkdir()
    source = root / "entry_probe.cpp"
    source.write_text(PROBE_SOURCE, encoding="utf-8")
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    executable = stage / "entry_probe.exe"
    batch = root / "compile.cmd"
    batch.write_text('@echo off\ncall "' + str(Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat")
        + '" x86 >nul\nif errorlevel 1 exit /b %errorlevel%\n'
        + subprocess.list2cmdline(["cl.exe", "/nologo", "/W4", "/Od", "/MT", str(source),
            "/Fe:" + str(executable), "/Fo:" + str(stage / "entry_probe.obj"), "/link",
            "/MANIFEST:EMBED", "/DYNAMICBASE:NO", "shell32.lib"]) + "\n", encoding="utf-8")
    built = subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=root, capture_output=True, text=True)
    (root / "compile.log").write_text(built.stdout + built.stderr, encoding="utf-8")
    if built.returncode:
        raise RuntimeError(f"Entry fixture compile failed: {root / 'compile.log'}")
    g.prepare_medium(stage, root / "medium-prepare.json", executable, executable, [protected], probe=True)
    image = pefile.PE(str(executable))
    try:
        base = image.OPTIONAL_HEADER.ImageBase
        anchors = {s.name.decode().lstrip("_"): s.address for s in image.DIRECTORY_ENTRY_EXPORT.symbols}
        points = [{"address": f"{base+rva:08x}", "expected_byte": image.get_data(rva,1)[0], "board_target": True}
                  for name,rva in anchors.items() if name.startswith("entry_probe_")]
        targets = {"schema": "bsp.original-entry.targets.v1", "image_sha256": g.sha(executable),
            "image_base": base, "image_size": image.OPTIONAL_HEADER.SizeOfImage,
            "index_sha256": "0"*64, "wire_sha256": "1"*64,
            "points": sorted(points, key=lambda p:p["address"])}
    finally:
        image.close()
    manifest = root / "targets.json"
    g.write_receipt(manifest, targets)
    run = stage / "entry-fixture"
    run.mkdir()
    class ProbeSession(EntrySession):
        def admit(self, process, state):
            # Only the manifested dummy's never-called export is changed.
            # This exercises the live-byte admission mismatch, not game logic.
            g.replace_remote(process, base + anchors["entry_probe_mismatch"], b"\x90")
            super().admit(process, state)
        def poll(self):
            result_path = stage / "result.txt"
            try:
                ready = result_path.exists() and result_path.read_text().strip() == "4096"
            except PermissionError:
                ready = False  # The dummy briefly holds its CRT output file open.
            if self.stop is None and ready:
                g.write_receipt(self.stop_path, {"schema":"bsp.original-coverage.stop.v1", "pid":self.pid,
                    "process_created_filetime":self.created_filetime, "expected_exe":str(executable),
                    "nonce":self.nonce, "reason":"game_time_target",
                    "clock":{"game_time":240.0,"fixed_step_count":1,"fixture_synthetic":True}})
                self.data["fixture_synthetic_clock_request"] = True
            return super().poll()
    session = ProbeSession(executable, manifest, run, probe=True)
    data, failure = None, None
    try:
        data = g.run_guard(stage, executable, stage, documents, root / "launch.json", 40,
            original_exe=executable, arguments=[str(documents), str(stage / "result.txt")],
            protected_roots=[protected], probe=True, motion_discovery=False,
            token_mode="medium-write-locked", entry_coverage=session)
    except BaseException as exc:
        failure = exc
        data = getattr(session,"data",{})
    finally:
        data["original_root_inventory_unchanged"] = g.sha(fixture) == protected_sha
        failure = session.finish(data, failure)
        g.write_receipt(root / "launch.json", data)
    if failure:
        raise RuntimeError(f"Entry fixture failed: {failure}; {root / 'launch.json'}") from failure
    hits = [json.loads(line) for line in session.trace.read_text().splitlines()]
    positives = [row for row in hits if row["kind"] == "entry_hit"]
    expected = {f"{base+anchors['entry_probe_'+str(i)]:08x}" for i in range(32)}
    if (len(positives) != 32 or {h["address"] for h in positives} != expected
            or [h["sequence"] for h in positives] != list(range(1,33))
            or session.queued_duplicates < 1 or (stage / "result.txt").read_text().strip() != "4096"
            or {s["reason"] for s in session.skips} != {"not_executable","preexisting_int3","live_byte_mismatch"}
            or data.get("final_job_active") != [0] or not data["source_locks"]["owned_job_exited_before_parent_release"]
            or data.get("phase") != "forced_window_stop" or data.get("owned_forced_stop_exit_code") != 0xE0000002
            or data["complete_window"] is not False or g.sha(fixture) != protected_sha):
        raise RuntimeError(f"Native entry/queue/skip fixture evidence incomplete: {root / 'launch.json'}")
    result = {"phase":"passed", "root":str(root), "receipt":str(root / "launch.json"),
        "unique_entries":32, "native_calls":4096, "queued_duplicate_traps":session.queued_duplicates,
        "native_threads":64, "skipped_points":session.skips, "final_job_active":data["final_job_active"],
        "stop_classification":data["stop_classification"], "synthetic_clock_not_qualified":True,
        "protected_sha256":protected_sha, "trace_sha256":g.sha(session.trace),
        "evidence":"manifested dummy native fixture only; no original/mission-clock proof"}
    g.write_receipt(root / "result.json", result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, help="One private manifested native fixture; never retail")
    args = parser.parse_args()
    if not args.probe:
        parser.error("Use original_baseline_guard.py launch --function-entry-coverage MANIFEST")
    print(json.dumps(selftest(args.probe)))


if __name__ == "__main__":
    main()
