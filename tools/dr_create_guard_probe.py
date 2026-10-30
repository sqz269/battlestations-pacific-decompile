"""Bounded PRIVATE x86 DR CREATE/import-shim fixture. Never launches retail.

Only dummy fixtures are locked. Production guard/wrapper and SDK disk bytes are
unchanged. A result describes the private fixture, not retail mission time.
"""
from pathlib import Path
import argparse
import ctypes as C
import json
import os
import re
import secrets
import shutil
import struct
import subprocess
import time
import traceback
import winreg
from ctypes import wintypes

import original_baseline_guard as g
from original_motion_capture import ProcessReader


def run(command, cwd, log):
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True)
    log.write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(f"Build failed; see {log}")


def ifeo_clear(image):
    def visit(parent, path, view, depth=0):
        if depth >= 8:
            raise RuntimeError("IFEO tree too deep")
        try:
            key = winreg.OpenKey(parent, path, 0, winreg.KEY_READ | view)
        except FileNotFoundError:
            return
        with key:
            try:
                winreg.QueryValueEx(key, "Debugger")
            except FileNotFoundError:
                pass
            else:
                raise RuntimeError("Configured IFEO Debugger; refusing SDK create")
            count = winreg.QueryInfoKey(key)[0]
            for index in range(count):
                visit(key, winreg.EnumKey(key, index), 0, depth + 1)
    for view in (winreg.KEY_WOW64_32KEY, winreg.KEY_WOW64_64KEY):
        visit(winreg.HKEY_LOCAL_MACHINE,
              "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\" + image, view)


def read_optional(path):
    try:
        return path.read_text(encoding="utf-8")
    except (FileNotFoundError, PermissionError):
        return ""


def write_control(path, nonce, pid, created, reason):
    temporary = path.with_name(path.name + "." + secrets.token_hex(8) + ".tmp")
    temporary.write_text(f"{nonce} {pid} {created} {reason}\n", encoding="ascii")
    deadline = time.monotonic() + 1
    try:
        while True:
            try:
                os.replace(temporary, path)
                return
            except PermissionError:
                if time.monotonic() >= deadline:
                    raise
                time.sleep(.02)
    finally:
        temporary.unlink(missing_ok=True)


def verify_waiting_root(pid, created, target, entry_address, expected_entry):
    """Read only: independently bind the fixture PID/time/path and native spin."""
    reader = ProcessReader(pid)
    try:
        times = [wintypes.FILETIME() for _ in range(4)]
        if not reader.api.GetProcessTimes(reader.handle, *(C.byref(item) for item in times)):
            raise RuntimeError("Cannot read fixture creation FILETIME")
        actual = (times[0].dwHighDateTime << 32) | times[0].dwLowDateTime
        path, size = C.create_unicode_buffer(32768), wintypes.DWORD(32768)
        if actual != created or not reader.api.QueryFullProcessImageNameW(reader.handle, 0, path, C.byref(size)) or Path(path.value).resolve() != target:
            raise RuntimeError("Fixture PID/creation time/image identity changed")
        code = wintypes.DWORD()
        if not reader.api.GetExitCodeProcess(reader.handle, C.byref(code)) or code.value != 259:
            raise RuntimeError("Wrong nonce stopped the fixture root")
        observed = reader.read(entry_address, 16)
        if observed != b"\xeb\xfe" + bytes.fromhex(expected_entry)[2:]:
            raise RuntimeError("Independent native entry barrier changed")
        return observed.hex()
    finally:
        reader.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dr-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--handles", type=int, default=0, choices=[0, 69700],
                        help="0 uses unique dummy/ancestor handles; 69700 duplicates only dummy objects")
    parser.add_argument("--fail-entry-gate", action="store_true", help="Prove rejection before application entry")
    parser.add_argument("--negative", default="none", choices=["none", "admission-omitted", "admission-missing", "admission-options", "drcov-missing"])
    parser.add_argument("--diagnostic-dr", action="store_true", help="Use official debug runtime, retaining all creation gates")
    parser.add_argument("--prepare-only", action="store_true", help="Compile and write exact planned options without creating a fixture process")
    parser.add_argument("--nudge-stop", action="store_true", help="Private wrong-nonce then correct-nonce cleanup fixture")
    args = parser.parse_args()
    if args.nudge_stop and (args.fail_entry_gate or args.negative != "none" or args.handles):
        parser.error("--nudge-stop requires the small positive private fixture")
    stop_nonce = secrets.token_hex(8) if args.nudge_stop else ""
    while args.nudge_stop and int(stop_nonce, 16) == 0:
        stop_nonce = secrets.token_hex(8)
    root = g.private(args.output, exists=False)
    if root.exists():
        raise RuntimeError("Use a new private output directory")
    root.mkdir(parents=True)
    dr = g.ordinary(args.dr_root)
    sdk = dr / "bin32/drinjectlib.dll"
    before_sdk = g.sha(sdk)
    if before_sdk != "173c8d46926c9b7a05c71fd7521526baf2878e668e84b80e02388c4e64a1fc9e":
        raise RuntimeError("Unreviewed DR injector binary")
    sdk_files = [sdk, dr / "bin32/drconfiglib.dll", dr / "lib32/release/dynamorio.dll",
                 dr / "tools/lib32/release/drcov.dll", dr / "bin32/drrun.exe",
                 *(dr / ("ext/lib32/release/" + name + ".dll") for name in ("drmgr", "drx", "drcovlib"))]
    if args.diagnostic_dr:
        sdk_files.append(dr / "lib32/debug/dynamorio.dll")
    sdk_hashes = {str(path.relative_to(dr)): g.sha(path) for path in sdk_files}
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = Path(subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip())
    stage = root / "stage"
    protected = root / "dummy-source"
    stage.mkdir(); protected.mkdir()
    for path, grant in [(stage, "*S-1-5-32-545:(OI)(CI)F"),
                        (protected, "*" + g.current_user_sid() + ":(OI)(CI)F")]:
        run(["icacls.exe", str(path), "/grant:r", grant], root, root / (path.name + "-acl.log"))
    (protected / "empty").mkdir()
    source = protected / "protected.txt"
    source.write_bytes(b"protected dummy fixture\n")
    source_hash = g.sha(source)
    for name in ("profile", "temp", "drconfig", "Documents", "drcov", "drdebug"):
        (stage / name).mkdir()
    helper = stage / "dr_helper_probe.exe"
    target = stage / "dr_root_probe.exe"
    batch = root / "compile.cmd"
    batch.write_text('@echo off\ncall "' + str(vs / "VC/Auxiliary/Build/vcvarsall.bat") + '" x86 >nul\n'
        'if errorlevel 1 exit /b %errorlevel%\n' + subprocess.list2cmdline([
        "cl.exe", "/nologo", "/O2", "/EHsc", "/MT", "/DUNICODE", "/D_UNICODE",
        "/I" + str(dr / "include"), str(Path(__file__).with_suffix(".cpp")),
        "/Fe:" + str(helper), "/Fo:" + str(root / "helper.obj"), "/link", "/MANIFEST:EMBED",
        "/DYNAMICBASE:NO", "/FIXED", "/BASE:0x400000"]) + "\n", encoding="utf-8")
    run(["cmd.exe", "/d", "/c", str(batch)], root, root / "compile.log")
    shutil.copyfile(helper, target)
    entry_client = Path(__file__).with_name("dr_original_entry_probe.c").resolve()
    (root / "CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.15)\nproject(create_probe C CXX)\n'
        'find_package(DynamoRIO REQUIRED)\nadd_library(create_client SHARED "' + entry_client.as_posix() + '")\n'
        'configure_DynamoRIO_client(create_client)\nuse_DynamoRIO_extension(create_client drmgr)\n', encoding="utf-8")
    cmake = vs / "Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    run([str(cmake), "-S", str(root), "-B", str(root / "build"), "-A", "Win32",
         "-DDynamoRIO_DIR=" + str(dr / "cmake")], root, root / "cmake-configure.log")
    run([str(cmake), "--build", str(root / "build"), "--config", "Release"], root, root / "cmake-build.log")
    client = root / "build/Release/create_client.dll"
    pe = g.pe_info(target)
    import pefile
    image = pefile.PE(str(target))
    entry_bytes = image.get_data(pe["entry_rva"], 16).hex()
    image.close()
    documents = stage / "Documents"
    str(documents).encode("ascii")  # This bounded fixture deliberately uses an ANSI-safe path.
    if any(char.isspace() for char in str(root)):
        raise RuntimeError("This bounded fixture requires a private path without spaces")
    if any(char in str(path) for path in (root, dr) for char in '\";`\'') or any(char.isspace() for char in str(dr)):
        raise RuntimeError("This bounded fixture requires paths without embedded option quoting")
    entry_options = subprocess.list2cmdline([str(stage / "client.log"), str(documents), str(target),
        f'{pe["image_base"]:x}', f'{pe["entry_rva"]:x}', f'{pe["iat_rva"]:x}', entry_bytes,
        "1" if args.fail_entry_gate else "0", str(stage / "drcov")])
    runtime_options = "-no_follow_children -nop_initial_bblock -max_elide_jmp 0 -max_elide_call 0 -msgbox_mask 0 -stderr_mask 15 -logdir " + str(stage / "drdebug")
    if args.diagnostic_dr:
        runtime_options += " -loglevel 2"
    cov_options = "-logdir " + str(stage / "drcov") + " -dump_text"
    registered_client = stage / "missing-admission.dll" if args.negative == "admission-missing" else client
    registered_options = "-invalid-fixture-admission-option" if args.negative == "admission-options" else entry_options
    registered_cov = stage / "missing-drcov.dll" if args.negative == "drcov-missing" else dr / "tools/lib32/release/drcov.dll"
    expected_serialization = 'DYNAMORIO_OPTIONS=-code_api -probe_api ' + runtime_options
    if args.negative != "admission-omitted":
        expected_serialization += ' -client_lib32 "' + str(registered_client) + ';0;' + registered_options + '"'
    expected_serialization += ' -client_lib32 "' + str(registered_cov) + ';1;' + cov_options + '"'
    launch_plan = dict(runtime_options=runtime_options, entry_client_options=entry_options,
        drcov_client_options=cov_options,
        expected_config_options_line=None if args.nudge_stop else expected_serialization,
        base_config_options_before_dynamic_stop_args=expected_serialization if args.nudge_stop else None,
        original_launched=False, fixture_processes_created=False, pe=pe,
        nudge_stop=args.nudge_stop, stop_nonce=stop_nonce,
        dynamic_client_stop_args=["nonce16", "actual_root_pid_decimal", "native_created_filetime_decimal"] if args.nudge_stop else [])
    g.write_receipt(root / "launch-plan.json", launch_plan)
    if args.prepare_only:
        print(json.dumps({"prepared_only": True, "fixture_processes_created": False,
                          "plan": str(root / "launch-plan.json")}))
        return 0
    ifeo_clear(target.name)
    report = dict(schema="bsp.dr-create-guard.fixture.v1", fixture_only=True,
        original_launched=False, original_handles_opened=False, debugger_flags=False,
        drcov_directory_fresh=True,
        dr_injector_sha256=before_sdk, sdk_sha256=sdk_hashes, target_sha256=g.sha(target), started_unix=time.time(),
        root_job_active_limit=1, helper_job_active_limit=2, source_lock_scope="dummy fixtures plus filesystem ancestors",
        source_parent_independent_handles_retained_until_jobs_empty=True, documents_redirect_tested=True,
        expected_admission_rejection=args.fail_entry_gate or args.negative != "none", negative_case=args.negative, pe=pe, entry_bytes=entry_bytes,
        diagnostic_debug_runtime=args.diagnostic_dr,
        nudge_stop_fixture=args.nudge_stop, stop_nonce=stop_nonce,
        private_documents=str(documents), clients=[str(client), str(dr / "tools/lib32/release/drcov.dll")])
    token = None; locks = None; attributes = None; sink = None; jobs = []; duplicates = []; pi = g.PROCESS_INFORMATION()

    def job(limit):
        handle = g.K.CreateJobObjectW(None, None); g.checked(handle, "fixture job")
        limits = g.JOB_EXTENDED(); limits.basic.flags = 0x2000 | 0x8; limits.basic.active_limit = limit
        g.checked(g.K.SetInformationJobObject(handle, 9, C.byref(limits), C.sizeof(limits)), "fixture job limits")
        jobs.append(handle); return handle

    def active(handle):
        data = g.JOB_ACCOUNTING(); count = g.DWORD()
        g.checked(g.K.QueryInformationJobObject(handle, 1, C.byref(data), C.sizeof(data), C.byref(count)), "fixture job accounting")
        return data.active

    try:
        token = g.restricted_token("medium-write-locked")
        report["token"] = g.verify_token(token, "medium-write-locked")
        audit = g.audit_source_writes(token, [protected], token_mode="medium-write-locked")
        report["dummy_audit"] = audit
        if not audit["passed"]:
            raise RuntimeError("Dummy source audit failed")
        locks = g.SourceReadLocks([protected], audit, root / "dummy-handles.jsonl")
        report["dummy_locks"] = locks.receipt
        root_handles = list(locks.handles)
        if args.handles:
            # These are duplicates of a dummy fixture handle, not 69,700 source
            # objects. Retain all independent parent copies until both jobs exit.
            current = g.K.GetCurrentProcess()
            dummy_base = g.K.CreateFileW(str(source), 1, 3, None, 3, 0x02200000, None)
            g.checked(dummy_base != g.PVOID(-1).value, "open duplicate-only dummy object")
            duplicates.append(dummy_base)
            g.checked(g.K.SetHandleInformation(dummy_base, 3, 3), "protected dummy base")
            root_handles.append(dummy_base)
            while len(root_handles) < args.handles:
                duplicate = g.HANDLE()
                g.checked(g.K.DuplicateHandle(current, dummy_base, current,
                    C.byref(duplicate), 0, True, 2), "duplicate dummy handle")
                duplicates.append(duplicate.value)
                g.checked(g.K.SetHandleInformation(duplicate, 3, 3), "protected dummy duplicate")
                if duplicate.value == 0x400000 or duplicate.value > 0xffffffff:
                    raise RuntimeError("Unsafe x86 handle value")
                root_handles.append(duplicate.value)
        report["explicit_root_handle_count"] = len(root_handles)
        handle_file = stage / "source-handle-list.bin"
        handle_file.write_bytes(struct.pack("<I", len(root_handles)) + struct.pack("<" + "I" * len(root_handles), *root_handles))
        report["handle_manifest_sha256"] = g.sha(handle_file)
        sink_path = g.private(stage / "stdio.log", stage, exists=False)
        sink = g.K.CreateFileW(str(sink_path), 0x40000080, 3, None, 1, 0, None)
        g.checked(sink != g.PVOID(-1).value, "create exclusive private stdout/stderr sink")
        if sink == 0x400000 or sink > 0xffffffff:
            raise RuntimeError("Unsafe private sink handle value")
        g.checked(g.K.SetHandleInformation(sink, 1, 1), "sink inheritance")
        sink_info = g.FILE_INFORMATION(); canonical = C.create_unicode_buffer(32768)
        g.checked(g.K.GetFileInformationByHandle(sink, C.byref(sink_info)), "sink identity")
        g.checked(g.K.GetFinalPathNameByHandleW(sink, canonical, len(canonical), 0), "sink canonical")
        resolved_sink = Path(canonical.value.removeprefix("\\\\?\\"))
        if g.private(resolved_sink, stage) != sink_path or sink_info.links != 1 or sink_info.attributes & 0x410:
            raise RuntimeError("Unsafe diagnostic sink identity")
        query_object = g.api(g.N, "NtQueryObject", C.c_long, g.HANDLE, g.DWORD, g.PVOID, g.DWORD, C.POINTER(g.DWORD))
        basic = (g.DWORD * 14)(); returned = g.DWORD()
        if query_object(sink, 0, basic, C.sizeof(basic), C.byref(returned)) < 0 or basic[1] != 0x120196:
            raise RuntimeError("Private sink access differs from write plus attribute-query mode")
        report["diagnostic_sink"] = dict(path=str(resolved_sink), links=1, volume=sink_info.volume,
            file_id=(sink_info.index_high << 32) | sink_info.index_low, requested_access=0x40000080,
            granted_access=basic[1], share=3, creation="CREATE_NEW", flags=1, source=False,
            count=1, standard_streams="stdout and stderr combined; no console")
        root_job = job(1); helper_job = job(2)
        # Control handles are inherited by the helper, never by the root.
        for handle in (token, root_job):
            g.checked(g.K.SetHandleInformation(handle, 1, 1), "helper control inheritance")
        inherited = root_handles + [token, root_job, sink]
        size = g.SIZE(); g.K.InitializeProcThreadAttributeList(None, 1, 0, C.byref(size))
        attributes = C.create_string_buffer(size.value)
        g.checked(g.K.InitializeProcThreadAttributeList(attributes, 1, 0, C.byref(size)), "helper attrs")
        handle_list = (g.HANDLE * len(inherited))(*inherited)
        g.checked(g.K.UpdateProcThreadAttribute(attributes, 0, 0x20002, handle_list, C.sizeof(handle_list), None, None), "helper exact handle list")
        environment = dict(os.environ)
        environment.update(USERPROFILE=str(stage / "profile"), APPDATA=str(stage / "profile"),
            LOCALAPPDATA=str(stage / "profile"), TEMP=str(stage / "temp"), TMP=str(stage / "temp"),
            DYNAMORIO_CONFIGDIR=str(stage / "drconfig"), BSP_STAGE=str(stage), BSP_TARGET=str(target),
            DYNAMORIO_LOGDIR=str(stage / "drdebug"),
            BSP_TARGET_SHA256=report["target_sha256"], BSP_DR_ROOT=str(dr), BSP_DR_CLIENT=str(client),
            BSP_PROTECTED=str(protected), BSP_ROOT_JOB=str(root_job), BSP_ROOT_TOKEN=str(token),
            BSP_HANDLES_FILE=str(handle_file), BSP_EXPECT_LOGON_SID=g.expected_restricting_sids()[1],
            BSP_DIAGNOSTIC_SINK=str(sink), BSP_SINK_VOLUME=str(sink_info.volume),
            BSP_SINK_INDEX_HIGH=str(sink_info.index_high), BSP_SINK_INDEX_LOW=str(sink_info.index_low),
            BSP_ENTRY_RVA=f'{pe["entry_rva"]:x}', BSP_ENTRY_BYTES=entry_bytes, BSP_NEGATIVE=args.negative,
            BSP_DR_DIAGNOSTIC="1" if args.diagnostic_dr else "0",
            BSP_STOP_NONCE=stop_nonce, BSP_PROBE_WAIT_FOR_NUDGE="1" if args.nudge_stop else "0",
            BSP_HELPER_CONTROL=str(stage / "helper-control.txt"),
            BSP_DOCUMENTS=str(documents), BSP_ENTRY_CLIENT_OPTIONS=entry_options)
        envbuf = C.create_unicode_buffer("\0".join(k + "=" + v for k, v in sorted(environment.items())) + "\0\0")
        si = g.STARTUPINFOEX(); si.startup.cb = C.sizeof(si); si.attributes = C.addressof(attributes)
        si.startup.dwFlags = 0x100; si.startup.hStdOutput = sink; si.startup.hStdError = sink
        command = C.create_unicode_buffer(subprocess.list2cmdline([str(helper), "helper"]))
        g.checked(g.A.CreateProcessAsUserW(token, str(helper), command, None, None, True,
            4 | 0x400 | 0x20 | 0x80000 | 0x8000000, envbuf, str(stage),
            C.cast(C.byref(si), C.POINTER(g.STARTUPINFO)), C.byref(pi)), "owned suspended helper")
        report["helper_pid"] = pi.dwProcessId
        report["helper_token"] = g.process_token_info(pi.hProcess, "medium-write-locked")
        g.checked(g.K.AssignProcessToJobObject(helper_job, pi.hProcess), "helper job before resume")
        if Path("C:/Users/sqz269/.bsp/DRAIN").exists():
            raise RuntimeError("DRAIN")
        g.checked(g.K.ResumeThread(pi.hThread) != 0xffffffff, "resume owned helper only")
        nudge_phase = "awaiting_ready"
        deadline = time.monotonic() + 60
        while g.K.WaitForSingleObject(pi.hProcess, 100) != 0:
            if time.monotonic() >= deadline or Path("C:/Users/sqz269/.bsp/DRAIN").exists():
                raise RuntimeError("Fixture timeout or DRAIN")
            if args.nudge_stop:
                helper_now = read_optional(stage / "helper.log")
                client_now = read_optional(stage / "client.log")
                root_now = read_optional(stage / "root.log")
                if nudge_phase == "awaiting_ready":
                    pids = re.findall(r"^root_job_assigned_before_sdk_pi=(\d+)$", helper_now, re.M)
                    created = re.findall(r"^root_created_filetime=(\d+)$", helper_now, re.M)
                    if (len(pids) == len(created) == 1 and "probe_waiting_for_nudge=0\n" in root_now and
                            "entry_call_target_and_return_verified " in client_now and "nudge_stop_ready " in client_now):
                        root_pid, root_created = int(pids[0]), int(created[0])
                        if not 0 < root_pid <= 0xffffffff or not 0 < root_created <= 0xffffffffffffffff:
                            raise RuntimeError("Invalid native helper PID/FILETIME")
                        verify_waiting_root(root_pid, root_created, target, pe["image_base"] + pe["entry_rva"], entry_bytes)
                        readiness = dict(schema="bsp.dr.fixture-ready.v1", pid=root_pid,
                            process_created_filetime=str(root_created), nonce=stop_nonce, expected_exe=str(target),
                            fixture_only=True, filetime_source="native GetProcessTimes, independently read by parent")
                        g.write_receipt(root / "root-readiness.json", readiness)
                        report["root_readiness"] = readiness
                        write_control(stage / "helper-control.txt", stop_nonce, root_pid, root_created, "fixture_wrong_nonce")
                        nudge_phase = "waiting_wrong_nonce"
                elif nudge_phase == "waiting_wrong_nonce":
                    if (f"nudge_ignored reason=nonce_mismatch pid={root_pid}\n" in client_now and
                            "fixture_wrong_nonce_status=0\n" in helper_now):
                        observed = verify_waiting_root(root_pid, root_created, target,
                            pe["image_base"] + pe["entry_rva"], entry_bytes)
                        if active(root_job) != 1:
                            raise RuntimeError("Wrong nonce changed the guarded leaf job")
                        report.update(wrong_nonce_ignored=True, wrong_nonce_root_alive=True,
                                      wrong_nonce_native_entry_bytes=observed)
                        request = dict(schema="bsp.dr.fixture-stop.v1", pid=root_pid,
                            process_created_filetime=str(root_created), nonce=stop_nonce,
                            reason="fixture_completion", native_control_reason="game_time_target",
                            fixture_only=True, mission_clock_evidence=False)
                        g.write_receipt(root / "stop-request.json", request)
                        write_control(stage / "helper-control.txt", stop_nonce, root_pid, root_created, "game_time_target")
                        nudge_phase = "stop_sent"
        if args.nudge_stop and nudge_phase != "stop_sent":
            raise RuntimeError("Fixture exited before wrong/correct nonce sequence completed")
        code = g.DWORD(); g.checked(g.K.GetExitCodeProcess(pi.hProcess, C.byref(code)), "helper exit code")
        report["helper_exit_code"] = code.value
        helper_log = (stage / "helper.log").read_text(encoding="utf-8") if (stage / "helper.log").exists() else ""
        root_log = (stage / "root.log").read_text(encoding="utf-8") if (stage / "root.log").exists() else ""
        client_log = (stage / "client.log").read_text(encoding="utf-8") if (stage / "client.log").exists() else ""
        report.update(helper_log=helper_log, root_log=root_log, client_log=client_log,
            root_entry=(stage / "root-entry.txt").exists(), child_entry=(stage / "child-entry.txt").exists(),
            private_write=(stage / "private-write.txt").exists())
        report["stdio_text"] = sink_path.read_text(encoding="utf-8", errors="replace")[:16384]
        cov_files = list((stage / "drcov").glob("drcov.*.log"))
        report["drcov_logs"] = []
        for path in cov_files:
            text = path.read_text(encoding="utf-8")
            table = re.search(r"^BB Table: (\d+) bbs$", text, re.M)
            target_rows = [line for line in text.splitlines() if line.rstrip().endswith(str(target))]
            target_id = int(target_rows[0].split(",", 1)[0]) if len(target_rows) == 1 else None
            target_entry_hit = target_id is not None and re.search(
                rf"^module\[\s*{target_id}\]: 0x{pe['entry_rva']:08x},", text, re.M) is not None
            entry_sizes = [int(size) for size in re.findall(
                rf"^module\[\s*{target_id}\]: 0x{pe['entry_rva']:08x},\s+(\d+)", text, re.M)] if target_id is not None else []
            bb_records = len(re.findall(r"^module\[\s*\d+\]: 0x[0-9a-fA-F]+,\s+\d+\s*$", text, re.M))
            report["drcov_logs"].append(dict(path=str(path), size=path.stat().st_size,
                sha256=g.sha(path), bb_count=int(table.group(1)) if table else 0,
                parsed_bb_records=bb_records, bb_table_complete=bool(table and bb_records == int(table.group(1))),
                target_module_id=target_id, target_entry_covered=target_entry_hit, entry_bb_sizes=entry_sizes))
        if not report["expected_admission_rejection"] and (not cov_files or not all(path.read_bytes().startswith(b"DRCOV VERSION:") for path in cov_files)):
            raise RuntimeError("No valid drcov log alongside admission client")
        if args.negative != "none":
            if code.value != 1 or report["root_entry"] or report["child_entry"] or "entry_admitted" in client_log or "root-private-stdio-entry" in report["stdio_text"]:
                raise RuntimeError("Missing/bad client escaped the independent entry barrier")
            if not all(text in helper_log for text in ("independent_entry_barrier_installed=", "sdk_options_serialization_verified=0", "sdk_run_only_resume=0")):
                raise RuntimeError("Negative did not reach the independent barrier/run gate")
            report["passed"] = True
        elif args.fail_entry_gate:
            if code.value != 1 or report["root_entry"] or report["child_entry"] or "admission_rejected reason=forced_fixture_gate_failure" not in client_log:
                raise RuntimeError("Forced entry-gate failure did not fail closed")
            if "entry_admitted" in client_log or "hook_installed" in client_log:
                raise RuntimeError("Forced rejected entry was admitted")
            report["passed"] = True
        elif code.value or "root_child_before_code_denied=0" not in root_log:
            raise RuntimeError("CREATE/inject/root safety fixture failed; see logs")
        elif not (client_log.startswith("client_ready ") and "hook_installed " in client_log and
                  "personal_stub_selftest_passed " in client_log and "entry_admitted " in client_log and
                  "tracer_ready " in client_log and "fp_preservation_verified " in client_log and
                  "entry_call_target_and_return_verified " in client_log and
                  client_log.index("hook_installed ") < client_log.index("entry_admitted ")):
            raise RuntimeError("DR client not ready before original entry")
        elif not report["root_entry"] or report["child_entry"] or not report["private_write"]:
            raise RuntimeError("Root admission/child policy/private write failed")
        else:
            if f'inherited_source_count={len(root_handles)}\n' not in root_log or "personal_redirect_flags_stdcall_passed=0" not in root_log or "nonpersonal_original_fallback_passed=0" not in root_log:
                raise RuntimeError("Folder redirect/ABI/handle count proof missing")
            report["passed"] = True
        if not report["expected_admission_rejection"] and not any(log["target_entry_covered"] for log in report["drcov_logs"]):
            raise RuntimeError("Drcov log does not cover the target entry")
        if not report["expected_admission_rejection"] and not all(log["entry_bb_sizes"] and all(size == 5 for size in log["entry_bb_sizes"]) for log in report["drcov_logs"]):
            raise RuntimeError("Drcov did not observe the original CALL5 after app2app substitution")
        if args.nudge_stop:
            marker = (f"controlled_trace_stop pid={root_pid} expected_created_filetime={root_created} "
                      "reason=nonce_nudge exit_code=0\n")
            if (marker not in client_log or "admission_client_exit intentional_stop=1 " not in client_log or
                    "stop_nudge_status=0\n" not in helper_log or f"stop_nudge_sent={root_pid}\n" not in helper_log or
                    "root_exit_code=0\n" not in helper_log or not report.get("wrong_nonce_ignored") or
                    not all(log["bb_table_complete"] for log in report["drcov_logs"])):
                raise RuntimeError("Controlled stop lacks nonce, exit-callback or complete BB-table proof")
            settling = time.monotonic() + 1
            while any(active(handle) for handle in jobs) and time.monotonic() < settling:
                time.sleep(.02)
            report["pre_cleanup_job_active"] = [active(handle) for handle in jobs]
            if any(report["pre_cleanup_job_active"]):
                raise RuntimeError("Controlled stop left jobs active before cleanup")
            report["controlled_trace_stop_verified"] = True
    except BaseException as exc:
        report.update(passed=False, error=str(exc), exception_type=type(exc).__name__,
                      exception_traceback=traceback.format_exc(), cleanup_pending=True)
        # Preserve the triggering failure even if a subsequent cleanup check fails.
        g.write_receipt(root / "result.json", report)
    finally:
        for handle in reversed(jobs):
            g.K.TerminateJobObject(handle, 0xe0000022)
        # Also cover failure between creating the suspended helper and assigning
        # its job. Job termination is asynchronous: reap it before a fallback.
        # This PI was returned by our own CreateProcessAsUserW only.
        helper_termination_error = 0
        if pi.hProcess and g.K.WaitForSingleObject(pi.hProcess, 0) != 0:
            if g.K.WaitForSingleObject(pi.hProcess, 5000) != 0:
                if not g.K.TerminateProcess(pi.hProcess, 0xe0000022):
                    helper_termination_error = C.get_last_error()
                if g.K.WaitForSingleObject(pi.hProcess, 5000) != 0:
                    report.update(passed=False, cleanup_pending=True,
                                  cleanup_error="Owned helper did not signal after job/process termination")
                    g.write_receipt(root / "result.json", report)
                    raise RuntimeError(report["cleanup_error"])
        deadline = time.monotonic() + 10
        while jobs and any(active(handle) for handle in jobs) and time.monotonic() < deadline:
            time.sleep(.05)
        report["final_job_active"] = [active(handle) for handle in jobs]
        if any(report["final_job_active"]):
            report.update(passed=False, cleanup_pending=True,
                          cleanup_error="Owned job not empty; refusing source-lock cleanup")
            g.write_receipt(root / "result.json", report)
            raise RuntimeError(report["cleanup_error"])
        if helper_termination_error:
            if helper_termination_error == 5 and g.K.WaitForSingleObject(pi.hProcess, 0) == 0:
                report["terminated_helper_access_denied_race"] = True
            else:
                report.update(passed=False, cleanup_error=f"TerminateProcess error {helper_termination_error}")
        for handle in duplicates:
            g.K.SetHandleInformation(handle, 2, 0)
            g.close(handle)
        if locks:
            locks.close()
        if attributes:
            g.K.DeleteProcThreadAttributeList(attributes)
        if sink and "diagnostic_sink" in report:
            final_sink = g.FILE_INFORMATION()
            report["private_sink_identity_retained"] = bool(g.K.GetFileInformationByHandle(sink, C.byref(final_sink)) and
                final_sink.links == 1 and final_sink.volume == sink_info.volume and
                final_sink.index_high == sink_info.index_high and final_sink.index_low == sink_info.index_low)
        for handle in (pi.hThread, pi.hProcess, *jobs, token, sink):
            g.close(handle)
        report.update(cleanup_pending=False, protected_fixture_unchanged=source.exists() and g.sha(source) == source_hash,
            target_disk_unchanged=g.sha(target) == report["target_sha256"],
            sdk_disk_unchanged=all(g.sha(dr / path) == value for path, value in sdk_hashes.items()), ended_unix=time.time())
        report["passed"] = bool(report.get("passed") and report["protected_fixture_unchanged"] and report["sdk_disk_unchanged"] and
            report["target_disk_unchanged"] and report.get("private_sink_identity_retained"))
        g.write_receipt(root / "result.json", report)
    print(json.dumps({k: report.get(k) for k in ("passed", "helper_pid", "helper_exit_code", "error", "final_job_active", "protected_fixture_unchanged", "sdk_disk_unchanged")}))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
