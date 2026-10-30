"""Bounded PRIVATE x86 DR CREATE/import-shim fixture. Never launches retail.

Only dummy fixtures are locked. Production guard/wrapper and SDK disk bytes are
unchanged. A successful result does not establish Documents redirection.
"""
from pathlib import Path
import argparse
import ctypes as C
import json
import os
import shutil
import subprocess
import time
import winreg

import original_baseline_guard as g


CLIENT = r'''#include "dr_api.h"
static app_pc entry;
static char log_path[MAXIMUM_PATH];
static void log_event(const char *name) {
 file_t f=dr_open_file(log_path,DR_FILE_WRITE_APPEND);
 if(f!=INVALID_FILE){dr_fprintf(f,"%s pid=%d\n",name,dr_get_process_id());dr_close_file(f);}
}
static void at_entry(void){log_event("entry_execution");}
static dr_emit_flags_t bb(void *dc,void *tag,instrlist_t *list,bool trace,bool translating){
 if(!translating && tag==entry)dr_insert_clean_call(dc,list,instrlist_first_app(list),(void*)at_entry,true,0);
 return DR_EMIT_DEFAULT;
}
DR_EXPORT void dr_client_main(client_id_t id,int argc,const char *argv[]){
 int i;if(argc!=2)dr_abort();
 for(i=0;argv[1][i] && i+1<MAXIMUM_PATH;++i)log_path[i]=argv[1][i];log_path[i]=0;
 module_data_t *m=dr_get_main_module();if(!m)dr_abort();entry=m->entry_point;dr_free_module_data(m);
 dr_register_bb_event(bb);log_event("client_ready");
}
'''


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dr-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    root = g.private(args.output, exists=False)
    if root.exists():
        raise RuntimeError("Use a new private output directory")
    root.mkdir(parents=True)
    dr = g.ordinary(args.dr_root)
    sdk = dr / "bin32/drinjectlib.dll"
    before_sdk = g.sha(sdk)
    if before_sdk != "173c8d46926c9b7a05c71fd7521526baf2878e668e84b80e02388c4e64a1fc9e":
        raise RuntimeError("Unreviewed DR injector binary")
    sdk_files = [sdk, dr / "bin32/drconfiglib.dll", dr / "lib32/release/dynamorio.dll"]
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
    for name in ("profile", "temp", "drconfig"):
        (stage / name).mkdir()
    helper = stage / "dr_helper_probe.exe"
    target = stage / "dr_root_probe.exe"
    batch = root / "compile.cmd"
    batch.write_text('@echo off\ncall "' + str(vs / "VC/Auxiliary/Build/vcvarsall.bat") + '" x86 >nul\n'
        'if errorlevel 1 exit /b %errorlevel%\n' + subprocess.list2cmdline([
        "cl.exe", "/nologo", "/O2", "/EHsc", "/MT", "/DUNICODE", "/D_UNICODE",
        "/I" + str(dr / "include"), str(Path(__file__).with_suffix(".cpp")),
        "/Fe:" + str(helper), "/Fo:" + str(root / "helper.obj"), "/link", "/MANIFEST:EMBED"]) + "\n", encoding="utf-8")
    run(["cmd.exe", "/d", "/c", str(batch)], root, root / "compile.log")
    shutil.copyfile(helper, target)
    (root / "client.c").write_text(CLIENT, encoding="utf-8")
    (root / "CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.15)\nproject(create_probe C CXX)\n'
        'find_package(DynamoRIO REQUIRED)\nadd_library(create_client SHARED client.c)\n'
        'configure_DynamoRIO_client(create_client)\n', encoding="utf-8")
    cmake = vs / "Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
    run([str(cmake), "-S", str(root), "-B", str(root / "build"), "-A", "Win32",
         "-DDynamoRIO_DIR=" + str(dr / "cmake")], root, root / "cmake-configure.log")
    run([str(cmake), "--build", str(root / "build"), "--config", "Release"], root, root / "cmake-build.log")
    client = root / "build/Release/create_client.dll"
    ifeo_clear(target.name)
    report = dict(schema="bsp.dr-create-guard.fixture.v1", fixture_only=True,
        original_launched=False, original_handles_opened=False, debugger_flags=False,
        dr_injector_sha256=before_sdk, sdk_sha256=sdk_hashes, target_sha256=g.sha(target), started_unix=time.time(),
        root_job_active_limit=1, helper_job_active_limit=2, source_lock_scope="dummy fixtures plus filesystem ancestors",
        source_parent_independent_handles_retained_until_jobs_empty=True, documents_redirect_tested=False)
    token = None; locks = None; attributes = None; jobs = []; pi = g.PROCESS_INFORMATION()

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
        root_job = job(1); helper_job = job(2)
        # Control handles are inherited by the helper, never by the root.
        for handle in (token, root_job):
            g.checked(g.K.SetHandleInformation(handle, 1, 1), "helper control inheritance")
        inherited = list(locks.handles) + [token, root_job]
        size = g.SIZE(); g.K.InitializeProcThreadAttributeList(None, 1, 0, C.byref(size))
        attributes = C.create_string_buffer(size.value)
        g.checked(g.K.InitializeProcThreadAttributeList(attributes, 1, 0, C.byref(size)), "helper attrs")
        handle_list = (g.HANDLE * len(inherited))(*inherited)
        g.checked(g.K.UpdateProcThreadAttribute(attributes, 0, 0x20002, handle_list, C.sizeof(handle_list), None, None), "helper exact handle list")
        environment = dict(os.environ)
        environment.update(USERPROFILE=str(stage / "profile"), APPDATA=str(stage / "profile"),
            LOCALAPPDATA=str(stage / "profile"), TEMP=str(stage / "temp"), TMP=str(stage / "temp"),
            DYNAMORIO_CONFIGDIR=str(stage / "drconfig"), BSP_STAGE=str(stage), BSP_TARGET=str(target),
            BSP_TARGET_SHA256=report["target_sha256"], BSP_DR_ROOT=str(dr), BSP_DR_CLIENT=str(client),
            BSP_PROTECTED=str(protected), BSP_ROOT_JOB=str(root_job), BSP_ROOT_TOKEN=str(token),
            BSP_DUMMY_HANDLES=",".join(map(str, locks.handles)), BSP_EXPECT_LOGON_SID=g.expected_restricting_sids()[1])
        envbuf = C.create_unicode_buffer("\0".join(k + "=" + v for k, v in sorted(environment.items())) + "\0\0")
        si = g.STARTUPINFOEX(); si.startup.cb = C.sizeof(si); si.attributes = C.addressof(attributes)
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
        deadline = time.monotonic() + 30
        while g.K.WaitForSingleObject(pi.hProcess, 100) != 0:
            if time.monotonic() >= deadline or Path("C:/Users/sqz269/.bsp/DRAIN").exists():
                raise RuntimeError("Fixture timeout or DRAIN")
        code = g.DWORD(); g.checked(g.K.GetExitCodeProcess(pi.hProcess, C.byref(code)), "helper exit code")
        report["helper_exit_code"] = code.value
        helper_log = (stage / "helper.log").read_text(encoding="utf-8") if (stage / "helper.log").exists() else ""
        root_log = (stage / "root.log").read_text(encoding="utf-8") if (stage / "root.log").exists() else ""
        client_log = (stage / "client.log").read_text(encoding="utf-8") if (stage / "client.log").exists() else ""
        report.update(helper_log=helper_log, root_log=root_log, client_log=client_log,
            root_entry=(stage / "root-entry.txt").exists(), child_entry=(stage / "child-entry.txt").exists(),
            private_write=(stage / "private-write.txt").exists())
        if code.value or "root_child_before_code_denied=0" not in root_log:
            raise RuntimeError("CREATE/inject/root safety fixture failed; see logs")
        if not (client_log.startswith("client_ready ") and "entry_execution " in client_log):
            raise RuntimeError("DR client not ready before original entry")
        if not report["root_entry"] or report["child_entry"] or not report["private_write"]:
            raise RuntimeError("Root admission/child policy/private write failed")
        report["passed"] = True
    except BaseException as exc:
        report.update(passed=False, error=str(exc))
    finally:
        for handle in reversed(jobs):
            g.K.TerminateJobObject(handle, 0xe0000022)
        # Also cover failure between creating the suspended helper and assigning
        # its job. This PI was returned by our own CreateProcessAsUserW only.
        if pi.hProcess and g.K.WaitForSingleObject(pi.hProcess, 0) != 0:
            g.checked(g.K.TerminateProcess(pi.hProcess, 0xe0000022), "terminate owned helper")
            g.checked(g.K.WaitForSingleObject(pi.hProcess, 5000) == 0, "owned helper stopped")
        deadline = time.monotonic() + 10
        while jobs and any(active(handle) for handle in jobs) and time.monotonic() < deadline:
            time.sleep(.05)
        report["final_job_active"] = [active(handle) for handle in jobs]
        if any(report["final_job_active"]):
            raise RuntimeError("Owned job not empty; refusing source-lock cleanup")
        if locks:
            locks.close()
        if attributes:
            g.K.DeleteProcThreadAttributeList(attributes)
        for handle in (pi.hThread, pi.hProcess, *jobs, token):
            g.close(handle)
        report.update(protected_fixture_unchanged=source.exists() and g.sha(source) == source_hash,
            sdk_disk_unchanged=all(g.sha(dr / path) == value for path, value in sdk_hashes.items()), ended_unix=time.time())
        report["passed"] = bool(report.get("passed") and report["protected_fixture_unchanged"] and report["sdk_disk_unchanged"])
        g.write_receipt(root / "result.json", report)
    print(json.dumps({k: report.get(k) for k in ("passed", "helper_pid", "helper_exit_code", "error", "final_job_active", "protected_fixture_unchanged", "sdk_disk_unchanged")}))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
