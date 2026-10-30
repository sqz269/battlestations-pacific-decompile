"""Opt-in owned DR backend. No debugger, native fallback or unrestricted retry."""
from pathlib import Path
import ctypes as C
import json
import math
import os
import re
import secrets
import shutil
import struct
import subprocess
import sys
import time

import original_baseline_guard as g

ROOT = Path(__file__).resolve().parents[1]
RETAIL_SHA = "b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6"
XLIVE_SHA = "71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2"
ENTRY_BYTES = "e89cae0100e916feffff558bec515157"
SCENE = "universe/Scenes/missions/USN/usn_19_coralus.scn"
SDK_HASHES = {
    "bin32/drconfiglib.dll": "ac8a71ef0cc3476d1c80735984835f45714c1fde17d05feaaa3129360d2f5e6c",
    "bin32/drinjectlib.dll": "173c8d46926c9b7a05c71fd7521526baf2878e668e84b80e02388c4e64a1fc9e",
    "bin32/drrun.exe": "112433d9fe45c8e7bbd0ee4f61f4d7ff2de8f6ee76554e2421805dbe0cbdf166",
    "ext/lib32/release/drcovlib.dll": "8769820d9bfd6d17fc8569963795ef443eb06a5c7af9f2c9413a9282f6c14187",
    "ext/lib32/release/drmgr.dll": "725b60f5f277758e6c6b92eabf42b1e99b412e312c2ce87c9160ba8dc1e55e1a",
    "ext/lib32/release/drx.dll": "608068ff96a4d64dd23c4741b2ae5897898d2aa4aef8a5d8be98e7ee102f4b74",
    "lib32/release/dynamorio.dll": "6cd8936841554de5cec5d56090d224fe526208b261865584e3615b9fa3e8fd7f",
    "tools/lib32/release/drcov.dll": "e1a01bdcdf08beefb5fcc67daa4fb9c7a9f6ed82a323a3345146d3836d444381",
}


def plain_tree(path):
    count = 0
    for base, dirs, files in os.walk(g.private(path), followlinks=False, onerror=g.walk_error):
        for candidate in [Path(base), *(Path(base) / n for n in [*dirs, *files])]:
            candidate = g.ordinary(candidate)
            if candidate.is_file() and candidate.stat().st_nlink != 1:
                raise RuntimeError(f"Private hardlink refused: {candidate}")
            count += 1
            if count > 300000:
                raise RuntimeError("Private scan limit exceeded")
    return {"objects_checked": count, "no_reparse_or_hardlinks": True}


def sdk_identity(dr):
    dr = g.ordinary(dr)
    if any(c.isspace() or c in '\";`\'' for c in str(dr)):
        raise RuntimeError("Supported DR root must be ASCII without option quoting")
    str(dr).encode("ascii")
    actual = {name: g.sha(g.ordinary(dr / name)) for name in SDK_HASHES}
    if actual != SDK_HASHES:
        raise RuntimeError("Unsupported or changed DynamoRIO SDK components")
    return actual


def target_identity(executable, original_exe=None):
    executable = g.private(executable)
    if g.sha(executable) != RETAIL_SHA:
        raise RuntimeError("Unsupported retail SHA; production refuses fixtures")
    pe = g.pe_info(executable)
    if (pe["image_base"], pe["entry_rva"], pe["iat_rva"]) != (0x400000, 0x7FD2BD, 0x8E22F0):
        raise RuntimeError("Unsupported retail PE layout")
    if pe["imports_set_handle_information"]:
        raise RuntimeError("Unsupported retail protected-handle contract")
    if original_exe and g.sha(g.ordinary(original_exe)) != RETAIL_SHA:
        raise RuntimeError("Original executable differs from the supported build")
    if g.sha(g.private(executable.parent / "xlive.dll")) != XLIVE_SHA:
        raise RuntimeError("Unsupported private xlive.dll")
    import pefile
    image = pefile.PE(str(executable))
    try:
        if image.get_data(pe["entry_rva"], 16).hex() != ENTRY_BYTES:
            raise RuntimeError("Unsupported retail original entry instruction")
    finally:
        image.close()
    return pe, ENTRY_BYTES


def records(records, base, ignored=()):
    result = {}
    for row in records:
        name = row.get("path")
        if not isinstance(name, str) or not name or Path(name).is_absolute() or ".." in Path(name).parts or name in result:
            raise RuntimeError("Invalid preparation hash record")
        path = g.ordinary(base / name)
        digest = g.sha(path)
        if name.lower() not in ignored and (digest != row["sha256"] or path.stat().st_size != row["length"]):
            raise RuntimeError(f"Preparation identity changed: {path}")
        result[name] = {"sha256": digest, "length": path.stat().st_size}
    return result


def preparation(stage, executable, documents, original_exe):
    prepared = json.loads(g.private(stage / "preparation.json").read_text(encoding="utf-8-sig"))
    protected_docs = g.folder(5) / "Battlestations-Pacific"
    expected = {"stage_root": stage, "source_game": original_exe.parent, "source_profile": protected_docs,
                "private_game": executable.parent, "private_documents": documents}
    if prepared.get("schema") != "bsp.original-baseline.preparation.v1" or any(Path(prepared.get(k, "")) != v for k, v in expected.items()):
        raise RuntimeError("Preparation receipt paths do not bind this source/stage")
    if executable != stage / "game/battlestationspacific.exe":
        raise RuntimeError("DR target must be the declared private game executable")
    medium = json.loads(g.private(stage / ".guard-medium.json").read_text(encoding="utf-8"))
    if (medium.get("schema") != "bsp.original-baseline.guard.medium.prepare.v1" or medium.get("phase") != "prepared_medium"
            or Path(medium.get("stage_root", "")) != stage or Path(medium.get("executable", "")) != executable
            or medium.get("sha256") != RETAIL_SHA):
        raise RuntimeError("Private medium preparation identity is absent or stale")
    source = records(prepared["root_files_sha256"], original_exe.parent)
    if source.get("battlestationspacific.exe", {}).get("sha256") != RETAIL_SHA or source.get("xlive.dll", {}).get("sha256") != XLIVE_SHA:
        raise RuntimeError("Required original EXE/xlive inventory does not match")
    records(prepared["root_files_sha256"], executable.parent, ("bsp_debug.log", "xlive_debug.log"))
    profiles = list(prepared["profile_files_sha256"])
    records(profiles, protected_docs)
    options = stage / "native-options.json"
    if options.exists():
        override = json.loads(g.private(options).read_text(encoding="utf-8-sig"))
        if (override.get("schema") != "bsp.original-baseline.native-options.v1" or override.get("original_unchanged") is not True
                or Path(override.get("source_path", "")) != protected_docs / "options.txt"
                or Path(override.get("private_path", "")) != documents / "Battlestations-Pacific/options.txt"):
            raise RuntimeError("Invalid private native-options receipt")
        old = [r for r in profiles if r["path"] == "options.txt"]
        if len(old) != 1 or old[0] != override["source_record"]:
            raise RuntimeError("Private options override source record differs")
        profiles = [r for r in profiles if r["path"] != "options.txt"] + [override["private_record"]]
    records(profiles, documents / "Battlestations-Pacific")
    return protected_docs, prepared, source


def compile_tools(dr, isolated=False):
    cpp, client_source = ROOT / "tools/dr_create_guard_probe.cpp", ROOT / "tools/dr_original_entry_probe.c"
    key = g.sha(cpp)[:16] + g.sha(client_source)[:16] + ("-test" if isolated else "-retail")
    directory = g.private(ROOT / "local/drcov-tools" / key, exists=False)
    directory.mkdir(parents=True, exist_ok=True)
    marker = directory / "build.json"
    if marker.exists():
        built = json.loads(marker.read_text())
        if (built.get("isolated_test") is isolated and built.get("source_sha256") == {str(cpp): g.sha(cpp), str(client_source): g.sha(client_source)}
                and set(built.get("files", {})) == {"original_dr_creator_probe.exe", "build/Release/admission_client.dll"}
                and all(g.sha(g.private(directory / name, directory)) == digest for name, digest in built["files"].items())):
            return directory / "original_dr_creator_probe.exe", directory / "build/Release/admission_client.dll", built
        raise RuntimeError("Cached coverage tools changed")
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = Path(subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip())
    helper = directory / "original_dr_creator_probe.exe"
    batch = directory / "compile.cmd"
    define = "/DBSP_DR_PRODUCTION_TEST" if isolated else "/DBSP_DR_PRODUCTION"
    batch.write_text('@echo off\ncall "' + str(vs / "VC/Auxiliary/Build/vcvarsall.bat") + '" x86 >nul\n'
        'if errorlevel 1 exit /b %errorlevel%\n' + subprocess.list2cmdline(["cl.exe", "/nologo", "/O2", "/EHsc", "/MT",
        "/DUNICODE", "/D_UNICODE", define, "/I" + str(dr / "include"), str(cpp), "/Fe:" + str(helper),
        "/Fo:" + str(directory / "helper.obj"), "/link", "/MANIFEST:EMBED"]) + "\n")
    cmake = vs / "Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    (directory / "CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.15)\nproject(admission C CXX)\n'
        'find_package(DynamoRIO REQUIRED)\nadd_library(admission_client SHARED "' + client_source.as_posix() + '")\n'
        'configure_DynamoRIO_client(admission_client)\nuse_DynamoRIO_extension(admission_client drmgr)\n')
    commands = [["cmd.exe", "/d", "/c", str(batch)], [str(cmake), "-S", str(directory), "-B", str(directory / "build"),
        "-A", "Win32", "-DDynamoRIO_DIR=" + str(dr / "cmake")], [str(cmake), "--build", str(directory / "build"), "--config", "Release"]]
    for index, command in enumerate(commands):
        run = subprocess.run(command, cwd=directory, capture_output=True, text=True)
        (directory / f"build-{index}.log").write_text(run.stdout + run.stderr)
        if run.returncode:
            raise RuntimeError(f"Coverage tool build failed: {directory}/build-{index}.log")
    if g.label(helper)["rid"] != g.MEDIUM:
        raise RuntimeError("Coverage helper executable must be medium integrity")
    built = {"isolated_test": isolated, "source_sha256": {str(cpp): g.sha(cpp), str(client_source): g.sha(client_source)},
        "files": {"original_dr_creator_probe.exe": g.sha(helper), "build/Release/admission_client.dll": g.sha(directory / "build/Release/admission_client.dll")}}
    g.write_receipt(marker, built)
    return helper, directory / "build/Release/admission_client.dll", built


def filetime(process):
    get_times = g.api(g.K, "GetProcessTimes", g.W.BOOL, g.HANDLE, *([C.POINTER(g.W.FILETIME)] * 4))
    fields = [g.W.FILETIME() for _ in range(4)]
    g.checked(get_times(process, *[C.byref(x) for x in fields]), "owned process creation time")
    return (fields[0].dwHighDateTime << 32) | fields[0].dwLowDateTime


def atomic_control(path, text):
    path = g.private(path, exists=False)
    temporary = path.with_name(path.name + "." + secrets.token_hex(8) + ".tmp")
    try:
        with open(temporary, "x", encoding="ascii", newline="\n") as stream:
            stream.write(text)
        deadline = time.monotonic() + .5
        while True:
            try:
                os.replace(temporary, path); break
            except OSError as exc:
                if exc.winerror not in (5, 32, 33) or time.monotonic() >= deadline:
                    raise
                time.sleep(.02)
    finally:
        temporary.unlink(missing_ok=True)


def trace_receipts(directory, executable, entry_rva, pid):
    from coverage_census import read_drcov
    import pefile
    image = pefile.PE(str(executable), fast_load=True)
    try:
        metadata = {"image_size": image.OPTIONAL_HEADER.SizeOfImage, "timestamp": image.FILE_HEADER.TimeDateStamp}
    finally:
        image.close()
    result = []
    paths = list(directory.glob("drcov.*.log"))
    if len(paths) != 1 or paths[0].name.lower() != f"drcov.{executable.name}.{pid:05d}.0000.proc.log".lower():
        raise RuntimeError("Trace filenames do not bind exactly the owned root PID")
    for path in paths:
        g.private(path)
        observations, parsed = read_drcov(path, executable.name, metadata, original=True)
        modules = parsed["selected_modules"]
        if (not parsed["bb_table_complete"] or not any(row["id"] == row["containing_id"] and row["start"] == 0x400000 for row in modules)
                or any(Path(row["path"]) != executable for row in modules)):
            raise RuntimeError("Completed trace module identity differs from the owned private root")
        sizes = sorted({observation.size for observation in observations if observation.rva == entry_rva})
        result.append({"path": str(path), "sha256": g.sha(path), "size": path.stat().st_size,
            "bb_count": parsed["bb_records"], "bb_table_complete": True, "entry_bb_sizes": sizes,
            "selected_modules": modules, "root_pid": pid})
    return result


def observer_exit_failure(data, primary_failure, returncode):
    """An observer following a dead root must not replace the root diagnosis."""
    message = f"Clock observer did not exit successfully (exit_code={returncode})"
    data["observer_error"] = message
    if primary_failure is not None:
        data.setdefault("secondary_errors", []).append({"component": "clock_observer",
            "exit_code": returncode, "error": message})
        return primary_failure
    failure = RuntimeError(message)
    data.update(phase="failed", error=str(failure))
    return failure


def run_guard(stage, executable, cwd, documents, receipt, seconds, original_exe, arguments, token_mode, dr_root,
              *, isolated=None):
    if token_mode != "medium-write-locked":
        raise RuntimeError("DR is supported only with medium-write-locked protection")
    if not 5 <= seconds <= 3600:
        raise RuntimeError("DR runtime bound must be 5..3600 seconds")
    stage = g.private(stage); executable, cwd, documents = [g.private(p, stage) for p in (executable, cwd, documents)]
    receipt = g.private(receipt, exists=False); dr = g.ordinary(dr_root)
    if any(c.isspace() or c in '\";`\'' for c in str(stage)):
        raise RuntimeError("DR stage requires a path without option quoting")
    str(stage).encode("ascii"); sdk = sdk_identity(dr)
    if isolated is None:
        if tuple(arguments) != (SCENE,) or cwd != executable.parent:
            raise RuntimeError("DR admits only the full USN04 direct-scene argument")
        pe, entry_bytes = target_identity(executable)
        original_exe = g.ordinary(original_exe)
        if original_exe != g.ordinary(g.RETAIL):
            raise RuntimeError("DR source audit must bind the declared original installation")
        protected_docs, prepared, original_roots = preparation(stage, executable, documents, original_exe)
        protected_roots = [g.ordinary(original_exe.parent), protected_docs if protected_docs.exists() else protected_docs.parent]
        snapshot = g.documents_snapshot(protected_docs)
        scan = g.scan_tree(stage, True, medium_executable=executable)
        observer_script = g.ordinary(ROOT / "tools/original_coverage_clock.py")
        if not observer_script.is_file():
            raise RuntimeError("Integrated read-only coverage clock observer is required")
        observer_hash = g.sha(observer_script)
    else:
        if not stage.is_relative_to(g.private(ROOT / "local" / isolated["root_name"])) or executable.name != "dr_root_probe.exe":
            raise RuntimeError("Isolated test target escaped its fixed private fixture")
        pe, entry_bytes, protected_roots = isolated["pe"], isolated["entry_bytes"], isolated["protected_roots"]
        protected_docs = protected_roots[0]; snapshot = g.documents_snapshot(protected_docs)
        prepared, original_roots, observer_hash = None, None, None
        scan = plain_tree(stage)
    helper, client, build = compile_tools(dr, isolated is not None)
    run_dir = g.private(stage / ("coverage-" + time.strftime("%Y%m%dT%H%M%S") + "-" + secrets.token_hex(6)), exists=False)
    run_dir.mkdir()
    for name in ("drconfig", "drdebug", "drcov"):
        (run_dir / name).mkdir()
    nonce = secrets.token_hex(8)
    while int(nonce, 16) == 0:
        nonce = secrets.token_hex(8)
    data = {"schema": "bsp.original-baseline.drcov.launch.v1", "phase": "checking", "stage": str(stage),
        "receipt": str(receipt), "run_directory": str(run_dir), "guard_pid": os.getpid(), "root_executable": str(executable),
        "root_executable_sha256": g.sha(executable), "arguments": list(arguments), "direct_scene_path": SCENE,
        "no_debugger": True, "native_entry_barrier": "EBFE remains in application memory; original CALL5 exists only in verified DR IR",
        "pe": pe, "original_entry_bytes": entry_bytes, "restricted_token_flags": 9,
        "job_policy": {"helper_active_limit": 2, "root_active_limit": 1, "kill_on_close": True, "breakaway": False},
        "protected_documents": str(protected_docs), "protected_documents_before": snapshot,
        "admitted": False, "token_mode": token_mode, "sdk_sha256": sdk, "tools": build, "sandbox_scan": scan,
        "stop_request_path": str(run_dir / "stop-request.json"), "stop_nonce": nonce,
        "scene_log_path": str(cwd / "bsp_debug.log"), "scene_log_start_size": 0, "scene_log_reset_before_launch": True,
        "observer_source_sha256": observer_hash, "started_unix": time.time(), "isolated_test": isolated is not None,
        "disk_binary_modified": False, "processes": [], "intentional_instrumentation_stop": False}
    g.write_receipt(receipt, data)
    token = root_job = helper_job = sink = root_process = attributes = None
    attributes_initialized = False
    locks = None; pi = g.PROCESS_INFORMATION(); observer = observer_output = None
    failure = None; stop_reason = None; stop_time = None; last_write = 0; bound = False
    drain = Path(os.environ["USERPROFILE"]) / ".bsp/DRAIN"

    def publish():
        g.write_receipt(receipt, data)

    def check_drain():
        if drain.exists():
            raise RuntimeError("DRAIN is set")

    def job(limit):
        handle = g.K.CreateJobObjectW(None, None); g.checked(handle, "DR owned job")
        limits = g.JOB_EXTENDED(); limits.basic.flags = 0x2000 | 0x8; limits.basic.active_limit = limit
        try:
            g.checked(g.K.SetInformationJobObject(handle, 9, C.byref(limits), C.sizeof(limits)), "DR active-process limit")
        except BaseException:
            g.close(handle); raise
        return handle

    def active(handle):
        if not handle:
            return 0
        info, size = g.JOB_ACCOUNTING(), g.DWORD()
        g.checked(g.K.QueryInformationJobObject(handle, 1, C.byref(info), C.sizeof(info), C.byref(size)), "DR owned job accounting")
        return info.active

    def stop(reason, payload=None):
        nonlocal stop_reason, stop_time
        if stop_reason:
            return
        if not bound:
            raise RuntimeError("Cannot issue a stop before owned root identity is bound")
        stop_reason, stop_time = reason, time.monotonic()
        data.update(phase="stopping", stop_reason=reason, stop_request=payload)
        atomic_control(run_dir / "native-stop-command.txt", f'{nonce} {data["root_pid"]} {data["root_created_filetime"]} {reason}\n')
        publish()

    try:
        check_drain()
        token = g.restricted_token(token_mode); data["token"] = g.verify_token(token, token_mode)
        data["source_write_audit"] = g.audit_source_writes(token, protected_roots, token_mode=token_mode)
        publish()
        if not data["source_write_audit"]["complete"] or not data["source_write_audit"]["passed"]:
            raise RuntimeError("Actual restricted token permits source mutation")
        locks = g.SourceReadLocks(protected_roots, data["source_write_audit"], run_dir / "source-handles.jsonl", check_cancel=check_drain)
        data["source_locks"] = locks.receipt; data["source_handle_count"] = len(locks.handles)
        (run_dir / "source-handle-list.bin").write_bytes(struct.pack("<I", len(locks.handles)) +
            struct.pack("<" + "I" * len(locks.handles), *locks.handles))
        archived = []
        for name in ("bsp_debug.log", "xlive_debug.log"):
            path = g.private(cwd / name, stage, exists=False)
            if path.exists():
                if path.stat().st_nlink != 1:
                    raise RuntimeError("Private generated log is a hardlink")
                before = g.sha(path); shutil.copyfile(path, run_dir / ("prelaunch-" + name))
                if g.sha(run_dir / ("prelaunch-" + name)) != before:
                    raise RuntimeError("Private log archival mismatch")
                archived.append({"path": str(path), "sha256": before})
            with open(path, "wb"):
                pass
        data["private_generated_logs_archived"] = archived
        sink_path = g.private(run_dir / "stdio.log", stage, exists=False)
        sink = g.K.CreateFileW(str(sink_path), 0x40000080, 3, None, 1, 0, None)
        g.checked(sink != g.PVOID(-1).value and sink != 0x400000 and sink <= 0xffffffff, "DR private sink")
        info = g.FILE_INFORMATION(); canonical = C.create_unicode_buffer(32768)
        g.checked(g.K.GetFileInformationByHandle(sink, C.byref(info)), "DR sink identity")
        g.checked(g.K.GetFinalPathNameByHandleW(sink, canonical, len(canonical), 0), "DR sink path")
        if Path(canonical.value.removeprefix("\\\\?\\")) != sink_path or info.links != 1 or info.attributes & 0x410:
            raise RuntimeError("DR sink canonical/link/type mismatch")
        query = g.api(g.N, "NtQueryObject", C.c_long, g.HANDLE, g.DWORD, g.PVOID, g.DWORD, C.POINTER(g.DWORD))
        basic, returned = (g.DWORD * 14)(), g.DWORD()
        if query(sink, 0, basic, C.sizeof(basic), C.byref(returned)) < 0 or basic[1] != 0x120196:
            raise RuntimeError("DR sink access mode mismatch")
        data["diagnostic_sink"] = {"path": str(sink_path), "source": False, "links": 1,
            "volume": info.volume, "file_id": (info.index_high << 32) | info.index_low, "granted_access": basic[1], "count": 1}
        root_job, helper_job = job(1), job(2)
        for handle in (token, root_job, sink):
            g.checked(g.K.SetHandleInformation(handle, 1, 1), "DR explicit helper control inheritance")
        inherited = [*locks.handles, token, root_job, sink]
        size = g.SIZE(); g.K.InitializeProcThreadAttributeList(None, 1, 0, C.byref(size)); attributes = C.create_string_buffer(size.value)
        g.checked(g.K.InitializeProcThreadAttributeList(attributes, 1, 0, C.byref(size)), "DR helper attribute list")
        attributes_initialized = True
        handles = (g.HANDLE * len(inherited))(*inherited)
        g.checked(g.K.UpdateProcThreadAttribute(attributes, 0, 0x20002, handles, C.sizeof(handles), None, None), "DR explicit helper handles")
        env = dict(os.environ)
        profile = stage / "profile"
        for path in (profile / "AppData/Roaming", profile / "AppData/Local", stage / "Temp"):
            path.mkdir(parents=True, exist_ok=True)
        options = subprocess.list2cmdline([str(run_dir / "client.log"), str(documents), str(executable), f'{pe["image_base"]:x}',
            f'{pe["entry_rva"]:x}', f'{pe["iat_rva"]:x}', entry_bytes, "0", str(run_dir / "drcov")])
        env.update(USERPROFILE=str(profile), APPDATA=str(profile / "AppData/Roaming"), LOCALAPPDATA=str(profile / "AppData/Local"),
            TEMP=str(stage / "Temp"), TMP=str(stage / "Temp"), DYNAMORIO_CONFIGDIR=str(run_dir / "drconfig"),
            HOMEDRIVE=profile.drive, HOMEPATH=str(profile)[len(profile.drive):], __COMPAT_LAYER="RunAsInvoker",
            DYNAMORIO_LOGDIR=str(run_dir / "drdebug"), BSP_STAGE=str(run_dir), BSP_TARGET_ROOT=str(stage), BSP_CWD=str(cwd),
            BSP_TARGET=str(executable), BSP_TARGET_SHA256=data["root_executable_sha256"], BSP_DR_ROOT=str(dr), BSP_DR_CLIENT=str(client),
            BSP_ROOT_JOB=str(root_job), BSP_ROOT_TOKEN=str(token), BSP_DIAGNOSTIC_SINK=str(sink), BSP_SINK_VOLUME=str(info.volume),
            BSP_SINK_INDEX_HIGH=str(info.index_high), BSP_SINK_INDEX_LOW=str(info.index_low), BSP_ENTRY_RVA=f'{pe["entry_rva"]:x}',
            BSP_ENTRY_BYTES=entry_bytes, BSP_HANDLES_FILE=str(run_dir / "source-handle-list.bin"), BSP_DOCUMENTS=str(documents),
            BSP_EXPECT_LOGON_SID=g.expected_restricting_sids()[1], BSP_DR_DIAGNOSTIC="0", BSP_NEGATIVE="none", BSP_ENTRY_CLIENT_OPTIONS=options,
            BSP_STOP_NONCE=nonce, BSP_HELPER_CONTROL=str(run_dir / "native-stop-command.txt"), BSP_RUNTIME_SECONDS=str(seconds + 15),
            BSP_ISOLATED_TEST="1" if isolated is not None else "0", BSP_PROBE_WAIT_FOR_NUDGE="1" if isolated is not None else "0")
        if isolated is not None:
            env["BSP_PROTECTED"] = str(protected_roots[0])
        block = C.create_unicode_buffer("\0".join(k + "=" + v for k, v in sorted(env.items(), key=lambda kv: kv[0].upper())) + "\0\0")
        si = g.STARTUPINFOEX(); si.startup.cb = C.sizeof(si); si.attributes = C.addressof(attributes)
        si.startup.dwFlags = 0x100; si.startup.hStdOutput = sink; si.startup.hStdError = sink
        command = C.create_unicode_buffer(subprocess.list2cmdline([str(helper), "helper"]))
        check_drain()
        g.checked(g.A.CreateProcessAsUserW(token, str(helper), command, None, None, True,
            4 | 0x400 | 0x20 | 0x80000 | 0x8000000, block, str(run_dir), C.cast(C.byref(si), C.POINTER(g.STARTUPINFO)), C.byref(pi)), "DR owned suspended helper")
        data.update(helper_pid=pi.dwProcessId, helper_created_filetime=filetime(pi.hProcess), helper_token=g.process_token_info(pi.hProcess, token_mode), phase="starting")
        g.checked(g.K.AssignProcessToJobObject(helper_job, pi.hProcess), "DR helper job before resume")
        for handle in (token, root_job, sink):
            g.checked(g.K.SetHandleInformation(handle, 1, 0), "clear parent control inheritance")
        publish(); g.checked(g.K.ResumeThread(pi.hThread) != 0xffffffff, "resume DR helper only")
        started = time.monotonic(); test_request = False
        while g.K.WaitForSingleObject(pi.hProcess, 100) != 0:
            helper_log = (run_dir / "helper.log").read_text(errors="replace") if (run_dir / "helper.log").exists() else ""
            client_log = (run_dir / "client.log").read_text(errors="replace") if (run_dir / "client.log").exists() else ""
            if not bound:
                pid = re.search(r"^root_job_assigned_before_sdk_pi=(\d+)$", helper_log, re.M)
                created = re.search(r"^root_created_filetime=(\d+)$", helper_log, re.M)
                if pid and created:
                    data.update(root_pid=int(pid.group(1)), root_created_filetime=int(created.group(1)))
                    root_process = g.K.OpenProcess(0x1000 | 0x10 | 0x100000, False, data["root_pid"])
                    g.checked(root_process, "bind owned DR root")
                    if filetime(root_process) != data["root_created_filetime"]:
                        raise RuntimeError("Owned DR root creation identity differs")
                    query_image = g.api(g.K, "QueryFullProcessImageNameW", g.W.BOOL, g.HANDLE, g.DWORD, g.W.LPWSTR, C.POINTER(g.DWORD))
                    image_path, image_size = C.create_unicode_buffer(32768), g.DWORD(32768)
                    if not query_image(root_process, 0, image_path, C.byref(image_size)) or g.private(Path(image_path.value), stage) != executable:
                        raise RuntimeError("Owned DR root executable path differs")
                    data["root_executable_verified"] = True
                    data["root_token"] = g.process_token_info(root_process, token_mode); bound = True; publish()
            ready = all(marker in client_log for marker in ("hook_installed ", "tracer_ready ", "entry_admitted ",
                "fp_preservation_verified ", "entry_call_target_and_return_verified "))
            if ready and not data["admitted"]:
                if not bound:
                    raise RuntimeError("Entry admitted before own root identity binding")
                count = g.DWORD(); g.checked(g.K.GetProcessHandleCount(root_process, C.byref(count)), "DR inherited source handle count")
                if count.value < len(locks.handles) + 1:
                    raise RuntimeError("DR root lost inherited handles before admission")
                data.update(admitted=True, root_handle_count=count.value, phase="running", hook_before_entry=True)
                publish()
                if isolated is None:
                    observer_output = open(run_dir / "observer-stdio.log", "xb")
                    observer_args = [sys.executable, str(observer_script), "--pid", str(data["root_pid"]), "--expected-exe", str(executable),
                        "--expect-sha256", RETAIL_SHA, "--launch-receipt", str(receipt), "--output", str(run_dir / "observer.json"),
                        "--stop-request", data["stop_request_path"], "--wall-timeout", str(max(5, seconds - (time.monotonic() - started)))]
                    observer = subprocess.Popen(observer_args, cwd=ROOT, stdin=subprocess.DEVNULL, stdout=observer_output,
                        stderr=subprocess.STDOUT, close_fds=True, creationflags=0x08000000)
                    data.update(observer_pid=observer.pid, observer_arguments=observer_args); publish()
            if isolated is not None and ready and not test_request:
                root_log = (run_dir / "root.log").read_text(errors="replace") if (run_dir / "root.log").exists() else ""
                if "probe_waiting_for_nudge=0" in root_log:
                    g.write_receipt(Path(data["stop_request_path"]), {"schema": "bsp.original-coverage.stop.v1", "pid": data["root_pid"],
                        "process_created_filetime": data["root_created_filetime"], "expected_exe": str(executable), "nonce": nonce,
                        "reason": "game_time_target", "clock": {"game_time": 240.0, "fixed_step_count": 1}, "isolated_test": True})
                    test_request = True
            request = Path(data["stop_request_path"])
            if request.exists() and not stop_reason:
                payload = json.loads(g.private(request).read_text(encoding="utf-8"))
                if (type(payload.get("pid")) is not int or type(payload.get("process_created_filetime")) is not int or
                        payload.get("schema") != "bsp.original-coverage.stop.v1" or payload.get("pid") != data.get("root_pid") or
                        payload.get("process_created_filetime") != data.get("root_created_filetime") or
                        Path(payload.get("expected_exe", "")) != executable or payload.get("nonce") != nonce or
                        payload.get("reason") not in ("game_time_target", "clock_reset", "timeout", "drain")):
                    raise RuntimeError("Observer stop identity/nonce/reason mismatch")
                if payload["reason"] == "game_time_target":
                    clock = payload.get("clock", {})
                    if (type(clock.get("game_time")) not in (int, float) or not math.isfinite(clock["game_time"]) or clock["game_time"] < 240 or
                            type(clock.get("fixed_step_count")) is not int or clock["fixed_step_count"] <= 0):
                        raise RuntimeError("Observer target stop lacks native GameTime evidence")
                stop(payload["reason"], payload)
            elapsed = time.monotonic() - started
            if drain.exists() and not stop_reason:
                stop("drain")
            if elapsed >= min(90, seconds) and not data["admitted"] and not stop_reason:
                stop("startup_gate_timeout")
            if elapsed >= seconds and not stop_reason:
                stop("timeout")
            if stop_time and time.monotonic() - stop_time >= 10:
                raise RuntimeError("Owned root did not flush/exit within stop grace period")
            if observer and observer.poll() is not None and not request.exists() and not stop_reason:
                raise RuntimeError("Clock observer exited without an identity-bound stop request")
            if time.monotonic() - last_write >= .5:
                publish(); last_write = time.monotonic()
        helper_code = g.DWORD(); g.checked(g.K.GetExitCodeProcess(pi.hProcess, C.byref(helper_code)), "DR helper exit")
        data["helper_exit_code"] = helper_code.value
        data["helper_log"] = (run_dir / "helper.log").read_text(errors="replace") if (run_dir / "helper.log").exists() else ""
        data["client_log"] = (run_dir / "client.log").read_text(errors="replace") if (run_dir / "client.log").exists() else ""
        code = re.search(r"^root_exit_code=(\d+)$", data["helper_log"], re.M)
        data["root_exit_code"] = int(code.group(1)) if code else None
        traces = trace_receipts(run_dir / "drcov", executable, pe["entry_rva"], data["root_pid"]); data["drcov_logs"] = traces
        ack = f'controlled_trace_stop pid={data.get("root_pid")} expected_created_filetime={data.get("root_created_filetime")} reason=nonce_nudge exit_code=0'
        data["intentional_instrumentation_stop"] = ack in data["client_log"] and "admission_client_exit intentional_stop=1" in data["client_log"]
        data["trace_flushed"] = bool(traces and all(t["bb_count"] > 0 and t["entry_bb_sizes"] and all(x == 5 for x in t["entry_bb_sizes"]) for t in traces))
        if (helper_code.value or data["root_exit_code"] != 0 or not data["admitted"] or not data["trace_flushed"] or
                not data["intentional_instrumentation_stop"] or stop_reason != "game_time_target"):
            data["phase"] = "crashed" if data["root_exit_code"] not in (None, 0) else {
                "timeout": "timed_out", "drain": "drained", "clock_reset": "observer_aborted",
                "startup_gate_timeout": "admission_timed_out"}.get(stop_reason, "natural_exit" if not stop_reason else "aborted")
            raise RuntimeError(f"Coverage incomplete: {data['phase']}, stop={stop_reason}, root_exit={data['root_exit_code']}")
        data["phase"] = "instrumentation_stopped"
    except BaseException as exc:
        failure = exc; data.update(error=f"{type(exc).__name__}: {exc}")
        if data["phase"] not in ("crashed", "natural_exit", "aborted"):
            data["phase"] = "failed"
    finally:
        for handle in (helper_job, root_job):
            if handle:
                g.K.TerminateJobObject(handle, 0xe0000040)
        if pi.hProcess and g.K.WaitForSingleObject(pi.hProcess, 5000) != 0:
            terminated = g.K.TerminateProcess(pi.hProcess, 0xe0000040)
            if not terminated and C.get_last_error() != 5:
                data["helper_cleanup_error"] = C.get_last_error()
            g.K.WaitForSingleObject(pi.hProcess, 5000)
        while active(helper_job) or active(root_job):
            data.update(waiting_for_owned_job_exit=True); publish()
            for handle in (helper_job, root_job):
                if handle:
                    g.K.TerminateJobObject(handle, 0xe0000040)
            time.sleep(.1)
        data["final_job_active"] = [active(helper_job), active(root_job)]
        if observer:
            try:
                try:
                    observer.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    observer.terminate(); observer.wait(timeout=5); data["observer_terminated_by_guard"] = True
                data["observer_exit_code"] = observer.returncode
            except BaseException as exc:
                data["observer_cleanup_error"] = f"{type(exc).__name__}: {exc}"
            if observer.returncode != 0:
                failure = observer_exit_failure(data, failure, observer.returncode)
        if observer_output:
            observer_output.close()
        verification_errors = []
        def verify(name, operation):
            try:
                data[name] = bool(operation())
            except BaseException as exc:
                data[name] = False; verification_errors.append(f"{name}: {type(exc).__name__}: {exc}")
        verify("protected_documents_unchanged", lambda: g.documents_snapshot(protected_docs) == snapshot)
        verify("disk_executable_unchanged", lambda: g.sha(executable) == data["root_executable_sha256"])
        verify("sdk_unchanged", lambda: sdk_identity(dr) == sdk)
        if isolated is None:
            verify("original_root_inventory_unchanged", lambda: records(prepared["root_files_sha256"], original_exe.parent) == original_roots)
            verify("original_executable_unchanged", lambda: g.sha(original_exe) == RETAIL_SHA)
        if sink and "diagnostic_sink" in data:
            def same_sink():
                actual = g.FILE_INFORMATION()
                return bool(g.K.GetFileInformationByHandle(sink, C.byref(actual)) and actual.links == 1 and
                    actual.volume == data["diagnostic_sink"]["volume"] and
                    ((actual.index_high << 32) | actual.index_low) == data["diagnostic_sink"]["file_id"])
            verify("private_sink_identity_retained", same_sink)
        if locks:
            data["source_locks"]["owned_jobs_empty_before_parent_release"] = True
            try:
                locks.close()
            except BaseException as exc:
                failure = RuntimeError(f"Source handle cleanup failed: {exc}"); data.update(phase="failed", error=str(failure))
        if attributes_initialized:
            g.K.DeleteProcThreadAttributeList(attributes)
        for handle in (pi.hThread, pi.hProcess, root_process, root_job, helper_job, sink, token):
            g.close(handle)
        data.update(ended_unix=time.time(), live_launch_allowed=False)
        required_checks = ["protected_documents_unchanged", "disk_executable_unchanged", "sdk_unchanged"]
        if "diagnostic_sink" in data:
            required_checks.append("private_sink_identity_retained")
        if isolated is None:
            required_checks += ["original_root_inventory_unchanged", "original_executable_unchanged"]
        if verification_errors:
            data["verification_errors"] = verification_errors
        if not all(data.get(k) for k in required_checks):
            failure = RuntimeError("Coverage preservation verification failed"); data.update(phase="failed", error=str(failure))
        publish()
    if failure:
        raise RuntimeError(f'{data["error"]}; receipt: {receipt}') from failure
    return data


def selftest(root, dr_root):
    """Only compiled dummy targets; production selection still rejects them."""
    root = g.private(root, exists=False)
    if root.exists():
        raise RuntimeError("DR backend selftest requires a new private directory")
    root.mkdir(parents=True)
    dr = g.ordinary(dr_root); sdk_identity(dr)
    command = [sys.executable, str(ROOT / "tools/dr_create_guard_probe.py"), "--dr-root", str(dr),
        "--output", str(root / "fixture"), "--prepare-only"]
    built = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    (root / "prepare.log").write_text(built.stdout + built.stderr)
    if built.returncode:
        raise RuntimeError(f"Dummy preparation failed: {root}/prepare.log")
    stage = root / "fixture/stage"; executable = stage / "dr_root_probe.exe"
    try:
        target_identity(executable)
    except RuntimeError as error:
        if "production refuses fixtures" not in str(error):
            raise
    else:
        raise RuntimeError("Production retail allowlist accepted a dummy image")
    compile_tools(dr, False)  # Compile the real pinned selection, never execute it.
    import pefile
    image = pefile.PE(str(executable))
    pe = g.pe_info(executable)
    entry_bytes = image.get_data(pe["entry_rva"], 16).hex(); image.close()
    test = {"root_name": str(root.relative_to(ROOT / "local")), "pe": pe, "entry_bytes": entry_bytes,
        "protected_roots": [root / "fixture/dummy-source"]}
    result = run_guard(stage, executable, stage, stage / "Documents", root / "backend-launch.json", 40, None,
        ("root",), "medium-write-locked", dr, isolated=test)
    result.update(dummy_refused_by_production=True, original_launched=False, original_install_handles_opened=False,
        mission_clock_evidence=False, explicit_compiled_test_selection=True)
    g.write_receipt(root / "backend-launch.json", result)
    return {"phase": "drcov_probe_passed", "root": str(root), "root_pid": result["root_pid"],
        "receipt": str(root / "backend-launch.json"), "evidence": "isolated production-routing/stop/strict-trace fixture only"}
