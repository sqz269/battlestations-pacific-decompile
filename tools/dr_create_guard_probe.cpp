// PRIVATE FIXTURE ONLY. Never accepts a retail image or an arbitrary command.
// Compile Win32 /MT /link /MANIFEST:EMBED. No on-disk SDK or target patches.
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#define WINDOWS
#define X86_32
#include <windows.h>
#include <sddl.h>
#include <bcrypt.h>
#include <dr_defines.h>
#include <dr_inject.h>
#include <dr_config.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "bcrypt.lib")

static std::wstring stage, target, expected_hash, dr_root, client;
static HANDLE root_job, root_token;
static std::vector<HANDLE> source_handles;
static unsigned create_calls;
static DWORD root_pid;
static void **iat_slot;
static void *original_create;
static FILE *journal;

static void event(const char *name, unsigned long value = 0) {
    fprintf(journal, "%s=%lu\n", name, value); fflush(journal);
}
static void require(bool ok, const char *what) {
    if (!ok) { event(what, GetLastError()); throw what; }
}
static std::string utf8(const std::wstring &s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string r(n, '\0'); WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &r[0], n, nullptr, nullptr);
    r.pop_back(); return r;
}
static std::wstring env(const wchar_t *name) {
    wchar_t value[32768]; DWORD n = GetEnvironmentVariableW(name, value, 32768);
    require(n && n < 32768, "missing_environment"); return value;
}
static std::vector<HANDLE> handles(const std::wstring &s) {
    std::vector<HANDLE> result; const wchar_t *p = s.c_str();
    while (*p) {
        wchar_t *next; unsigned long value = wcstoul(p, &next, 10);
        require(next != p && value && value != 0x400000 && (*next == ',' || !*next), "bad_handle_list");
        result.push_back(reinterpret_cast<HANDLE>(value)); p = *next ? next + 1 : next;
    }
    require(!result.empty(), "empty_handle_list"); return result;
}
static void check_flags(const std::vector<HANDLE> &list) {
    for (HANDLE h : list) { DWORD flags; require(GetHandleInformation(h, &flags) && flags == 3, "source_handle_flags"); }
}
static std::wstring sha256(const std::wstring &path) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    require(f != INVALID_HANDLE_VALUE, "hash_open");
    BCRYPT_ALG_HANDLE alg = nullptr; BCRYPT_HASH_HANDLE hash = nullptr;
    require(BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) == 0, "hash_algorithm");
    require(BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0) == 0, "hash_create");
    BYTE block[65536], digest[32]; DWORD count;
    do { require(ReadFile(f, block, sizeof(block), &count, nullptr) != 0, "hash_read");
        if (count) require(BCryptHashData(hash, block, count, 0) == 0, "hash_data");
    } while (count);
    require(BCryptFinishHash(hash, digest, sizeof(digest), 0) == 0, "hash_finish");
    BCryptDestroyHash(hash); BCryptCloseAlgorithmProvider(alg, 0); CloseHandle(f);
    std::wstring result; for (BYTE b : digest) { wchar_t pair[3]; swprintf_s(pair, L"%02x", b); result += pair; }
    return result;
}
static void token_check(HANDLE process, const char *prefix) {
    HANDLE token; require(OpenProcessToken(process, TOKEN_QUERY, &token) != 0, "query_token");
    DWORD size, value; BYTE buffer[8192];
    require(GetTokenInformation(token, TokenIntegrityLevel, buffer, sizeof(buffer), &size) != 0, "query_integrity");
    PSID sid = reinterpret_cast<TOKEN_MANDATORY_LABEL *>(buffer)->Label.Sid;
    value = *GetSidSubAuthority(sid, *GetSidSubAuthorityCount(sid) - 1);
    event(prefix, value); require(value == 8192, "unexpected_integrity");
    require(IsTokenRestricted(token) != 0, "unrestricted_token");
    require(GetTokenInformation(token, TokenUIAccess, &value, sizeof(value), &size) && value == 0, "ui_access");
    require(GetTokenInformation(token, TokenMandatoryPolicy, &value, sizeof(value), &size) && value == 3, "mandatory_policy");
    require(GetTokenInformation(token, TokenRestrictedSids, buffer, sizeof(buffer), &size) != 0, "restricted_sids");
    TOKEN_GROUPS *groups = reinterpret_cast<TOKEN_GROUPS *>(buffer);
    require(groups->GroupCount == 3, "restricted_sid_count");
    bool users = false, world = false, logon = false;
    std::wstring expected_logon = env(L"BSP_EXPECT_LOGON_SID");
    for (DWORD i = 0; i < groups->GroupCount; ++i) {
        LPWSTR text; require(ConvertSidToStringSidW(groups->Groups[i].Sid, &text) != 0, "sid_string");
        std::wstring s(text); LocalFree(text);
        if (s == L"S-1-5-32-545") users = true; else if (s == L"S-1-1-0") world = true;
        else if (s == expected_logon) logon = true; else require(false, "unexpected_restricting_sid");
    }
    require(users && world && logon, "restricted_sid_set"); CloseHandle(token);
}
// Reject the entire image IFEO tree if it could select a debugger. Read-only.
static void ifeo_key(HKEY parent, const std::wstring &name, REGSAM view, unsigned depth) {
    require(depth < 8, "ifeo_depth"); HKEY key;
    LONG status = RegOpenKeyExW(parent, name.c_str(), 0, KEY_READ | view, &key);
    if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND) return;
    require(status == ERROR_SUCCESS, "ifeo_read");
    DWORD type, count = 0; status = RegQueryValueExW(key, L"Debugger", nullptr, &type, nullptr, &count);
    require(status == ERROR_FILE_NOT_FOUND, "ifeo_debugger_configured");
    for (DWORD index = 0;; ++index) {
        wchar_t sub[256]; count = 256; status = RegEnumKeyExW(key, index, sub, &count, nullptr, nullptr, nullptr, nullptr);
        if (status == ERROR_NO_MORE_ITEMS) break;
        require(status == ERROR_SUCCESS, "ifeo_enum"); ifeo_key(key, sub, 0, depth + 1);
    }
    RegCloseKey(key);
}
static void **find_create_iat(HMODULE module) {
    BYTE *base = reinterpret_cast<BYTE *>(module);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS32 *>(base + dos->e_lfanew);
    require(dos->e_magic == IMAGE_DOS_SIGNATURE && nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386, "sdk_not_x86");
    auto imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    for (; imp->Name; ++imp) {
        if (_stricmp(reinterpret_cast<char *>(base + imp->Name), "KERNEL32.dll")) continue;
        auto names = reinterpret_cast<IMAGE_THUNK_DATA32 *>(base + imp->OriginalFirstThunk);
        auto values = reinterpret_cast<IMAGE_THUNK_DATA32 *>(base + imp->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++values) {
            if (IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal)) continue;
            auto name = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(base + names->u1.AddressOfData);
            if (!strcmp(reinterpret_cast<char *>(name->Name), "CreateProcessW")) {
                require(reinterpret_cast<BYTE *>(&values->u1.Function) - base == 0x3415c, "unexpected_sdk_iat_rva");
                return reinterpret_cast<void **>(&values->u1.Function);
            }
        }
    }
    require(false, "missing_sdk_create_iat"); return nullptr;
}
static BOOL WINAPI guarded_create(LPCWSTR app, LPWSTR command, LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta, BOOL inherit, DWORD flags, LPVOID environment, LPCWSTR cwd,
    LPSTARTUPINFOW startup, LPPROCESS_INFORMATION pi) {
    PROCESS_INFORMATION made = {}; LPPROC_THREAD_ATTRIBUTE_LIST attrs = nullptr;
    std::vector<BYTE> storage;
    try {
        ++create_calls; event("shim_calls", create_calls);
        std::wstring expected_command = target.find_first_of(L" \t") == std::wstring::npos ? target : L"\"" + target + L"\"";
        require(create_calls == 1 && app && target == app && command &&
            std::wstring(command) == expected_command + L" root", "unexpected_sdk_command");
        require(!pa && !ta && inherit && flags == CREATE_SUSPENDED && !environment && !cwd &&
            startup && startup->cb == sizeof(STARTUPINFOW) && pi, "unexpected_sdk_create_contract");
        require(sha256(target) == expected_hash, "target_hash_mismatch"); check_flags(source_handles);
        SIZE_T bytes = 0; InitializeProcThreadAttributeList(nullptr, 1, 0, &bytes);
        storage.resize(bytes); attrs = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());
        require(InitializeProcThreadAttributeList(attrs, 1, 0, &bytes) != 0, "root_attribute_init");
        require(UpdateProcThreadAttribute(attrs, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, source_handles.data(),
            source_handles.size() * sizeof(HANDLE), nullptr, nullptr) != 0, "root_exact_handle_list");
        STARTUPINFOEXW si = {}; si.StartupInfo.cb = sizeof(si); si.lpAttributeList = attrs;
        LPWCH block = GetEnvironmentStringsW(); require(block != nullptr, "root_environment");
        BOOL ok = CreateProcessAsUserW(root_token, target.c_str(), command, nullptr, nullptr, TRUE,
            CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT | EXTENDED_STARTUPINFO_PRESENT |
            NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, block, stage.c_str(), &si.StartupInfo, &made);
        DWORD error = GetLastError(); FreeEnvironmentStringsW(block);
        DeleteProcThreadAttributeList(attrs); attrs = nullptr; SetLastError(error);
        require(ok != 0, "root_create_failed");
        require(AssignProcessToJobObject(root_job, made.hProcess) != 0, "root_job_assignment");
        token_check(made.hProcess, "root_pre_run_integrity");
        BOOL in_job; require(IsProcessInJob(made.hProcess, root_job, &in_job) && in_job, "root_job_membership");
        root_pid = made.dwProcessId; event("root_job_assigned_before_sdk_pi", root_pid);
        *pi = made; return TRUE;
    } catch (...) {
        DWORD error = GetLastError(); if (!error) error = ERROR_ACCESS_DENIED;
        if (attrs) DeleteProcThreadAttributeList(attrs);
        if (made.hProcess) { TerminateProcess(made.hProcess, 0xe0000020); WaitForSingleObject(made.hProcess, 5000); CloseHandle(made.hThread); CloseHandle(made.hProcess); }
        if (pi) ZeroMemory(pi, sizeof(*pi)); SetLastError(error); return FALSE;
    }
}
template<class F> static F symbol(HMODULE module, const char *name) {
    auto value = reinterpret_cast<F>(GetProcAddress(module, name)); require(value != nullptr, "missing_sdk_export"); return value;
}
static void swap_iat(void *value) {
    DWORD old; require(VirtualProtect(iat_slot, sizeof(void *), PAGE_READWRITE, &old) != 0, "iat_protection");
    InterlockedExchangePointer(iat_slot, value); DWORD ignored;
    require(VirtualProtect(iat_slot, sizeof(void *), old, &ignored) != 0, "iat_restore_protection");
}
static int helper() {
    void *data = nullptr; bool registered = false; DWORD pid = 0; int result = 1;
    decltype(&dr_inject_process_exit) finish = nullptr;
    decltype(&dr_unregister_process) unregister = nullptr;
    try {
        stage = env(L"BSP_STAGE"); target = env(L"BSP_TARGET"); expected_hash = env(L"BSP_TARGET_SHA256");
        require(target == stage + L"\\dr_root_probe.exe", "not_private_fixture_image");
        dr_root = env(L"BSP_DR_ROOT"); client = env(L"BSP_DR_CLIENT");
        root_job = reinterpret_cast<HANDLE>(wcstoul(env(L"BSP_ROOT_JOB").c_str(), nullptr, 10));
        root_token = reinterpret_cast<HANDLE>(wcstoul(env(L"BSP_ROOT_TOKEN").c_str(), nullptr, 10));
        source_handles = handles(env(L"BSP_DUMMY_HANDLES")); check_flags(source_handles); token_check(GetCurrentProcess(), "helper_integrity");
        require(std::find(source_handles.begin(), source_handles.end(), root_job) == source_handles.end() &&
            std::find(source_handles.begin(), source_handles.end(), root_token) == source_handles.end(), "control_handle_in_root_list");
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {};
        require(QueryInformationJobObject(root_job, JobObjectExtendedLimitInformation, &limits, sizeof(limits), nullptr) &&
            limits.BasicLimitInformation.LimitFlags == (JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_ACTIVE_PROCESS) &&
            limits.BasicLimitInformation.ActiveProcessLimit == 1, "root_job_limits");
        event("root_exact_source_handle_count", DWORD(source_handles.size())); event("root_control_handles_excluded");
        std::wstring ifeo = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\dr_root_probe.exe";
        for (REGSAM view : {KEY_WOW64_32KEY, KEY_WOW64_64KEY}) ifeo_key(HKEY_LOCAL_MACHINE, ifeo, view, 0);
        event("ifeo_preflight_clear");
        HMODULE inject = LoadLibraryW((dr_root + L"\\bin32\\drinjectlib.dll").c_str()); require(inject != nullptr, "load_inject_sdk");
        HMODULE config = LoadLibraryW((dr_root + L"\\bin32\\drconfiglib.dll").c_str()); require(config != nullptr, "load_config_sdk");
        auto create = symbol<decltype(&dr_inject_process_create)>(inject, "dr_inject_process_create");
        auto get_pid = symbol<decltype(&dr_inject_get_process_id)>(inject, "dr_inject_get_process_id");
        auto get_process = symbol<decltype(&dr_inject_get_process_handle)>(inject, "dr_inject_get_process_handle");
        auto debug_key = symbol<decltype(&dr_inject_using_debug_key)>(inject, "dr_inject_using_debug_key");
        auto inject_process = symbol<decltype(&dr_inject_process_inject)>(inject, "dr_inject_process_inject");
        auto run = symbol<decltype(&dr_inject_process_run)>(inject, "dr_inject_process_run");
        finish = symbol<decltype(&dr_inject_process_exit)>(inject, "dr_inject_process_exit");
        auto reg = symbol<decltype(&dr_register_process)>(config, "dr_register_process");
        auto reg_client = symbol<decltype(&dr_register_client)>(config, "dr_register_client");
        unregister = symbol<decltype(&dr_unregister_process)>(config, "dr_unregister_process");
        iat_slot = find_create_iat(inject); original_create = *iat_slot;
        require(original_create == reinterpret_cast<void *>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateProcessW")), "unexpected_original_create_pointer");
        swap_iat(reinterpret_cast<void *>(&guarded_create)); event("sdk_create_iat_rva", 0x3415c);
        std::string image = utf8(target); const char *args[] = {image.c_str(), "root", nullptr};
        int status = create(image.c_str(), args, &data); event("sdk_create_status", status);
        require(status == 0 && create_calls == 1 && data, "sdk_create_failed");
        swap_iat(original_create); iat_slot = nullptr;
        pid = get_pid(data); require(pid == root_pid && get_process(data), "sdk_real_process_information");
        require(!debug_key(data), "sdk_debugger_branch"); event("sdk_non_debugger_path");
        std::string dr = utf8(dr_root), library = utf8(dr_root + L"\\lib32\\release\\dynamorio.dll");
        status = reg("dr_root_probe.exe", pid, false, dr.c_str(), DR_MODE_CODE_MANIPULATION, false, DR_PLATFORM_32BIT, "-no_follow_children");
        event("sdk_register_status", status); require(status == DR_SUCCESS, "sdk_register_failed"); registered = true;
        std::string dll = utf8(client), options = "\"" + utf8(stage + L"\\client.log") + "\"";
        status = reg_client("dr_root_probe.exe", pid, false, DR_PLATFORM_32BIT, 0, 0, dll.c_str(), options.c_str());
        event("sdk_register_client_status", status); require(status == DR_SUCCESS, "sdk_client_registration_failed");
        require(inject_process(data, true, library.c_str()), "sdk_injection_failed"); event("sdk_injected");
        require(GetFileAttributesW((stage + L"\\root-entry.txt").c_str()) == INVALID_FILE_ATTRIBUTES, "premature_root_entry");
        require(run(data), "sdk_run_failed"); event("sdk_run_only_resume");
        require(WaitForSingleObject(get_process(data), 20000) == WAIT_OBJECT_0, "root_wait_timeout");
        DWORD code; require(GetExitCodeProcess(get_process(data), &code) != 0, "root_exit_query"); event("root_exit_code", code);
        require(code == 0, "root_failed"); result = 0;
    } catch (...) { event("helper_failed"); }
    if (iat_slot) { try { swap_iat(original_create); } catch (...) {} }
    if (data && finish) finish(data, result != 0);
    if (registered && unregister) event("sdk_unregister_status", unregister("dr_root_probe.exe", pid, false, DR_PLATFORM_32BIT));
    return result;
}
static void file_event(const wchar_t *path, const char *text) {
    HANDLE f = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, 0, nullptr); DWORD n;
    require(f != INVALID_HANDLE_VALUE, "private_file_create");
    require(WriteFile(f, text, DWORD(strlen(text)), &n, nullptr) != 0 && n == strlen(text), "private_file_write"); CloseHandle(f);
}
static void deny_open(const std::wstring &path, DWORD access, const char *name) {
    HANDLE h = CreateFileW(path.c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    DWORD error = GetLastError(); if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    event(name, h == INVALID_HANDLE_VALUE ? error : 0); require(h == INVALID_HANDLE_VALUE && (error == 5 || error == 32), "protected_open_allowed");
}
static int root() {
    try {
        stage = env(L"BSP_STAGE"); file_event((stage + L"\\root-entry.txt").c_str(), "fixture-root-entry\n");
        token_check(GetCurrentProcess(), "root_runtime_integrity");
        auto list = handles(env(L"BSP_DUMMY_HANDLES")); check_flags(list); event("inherited_source_count", DWORD(list.size()));
        for (HANDLE h : list) { SetLastError(0); BOOL ok = CloseHandle(h); event("protected_close_error", GetLastError()); require(!ok && GetLastError() == ERROR_INVALID_HANDLE, "protected_close_succeeded"); }
        std::wstring protected_dir = env(L"BSP_PROTECTED"), file = protected_dir + L"\\protected.txt";
        deny_open(file, GENERIC_WRITE, "write_denied"); deny_open(file, WRITE_DAC, "write_dac_denied");
        deny_open(file, WRITE_OWNER, "write_owner_denied"); deny_open(file, FILE_WRITE_ATTRIBUTES, "attributes_denied");
        BOOL ok = DeleteFileW(file.c_str()); event("delete_denied", ok ? 0 : GetLastError()); require(!ok && GetLastError() == ERROR_SHARING_VIOLATION, "delete_allowed");
        ok = MoveFileExW(protected_dir.c_str(), (protected_dir + L"-renamed").c_str(), 0); event("rename_denied", ok ? 0 : GetLastError()); require(!ok && (GetLastError() == ERROR_ACCESS_DENIED || GetLastError() == ERROR_SHARING_VIOLATION), "rename_allowed");
        ok = RemoveDirectoryW((protected_dir + L"\\empty").c_str()); event("rmdir_denied", ok ? 0 : GetLastError()); require(!ok && (GetLastError() == ERROR_ACCESS_DENIED || GetLastError() == ERROR_SHARING_VIOLATION), "rmdir_allowed");
        file_event((stage + L"\\private-write.txt").c_str(), "private fixture write\n"); event("private_write_passed");
        wchar_t self[MAX_PATH]; require(GetModuleFileNameW(nullptr, self, MAX_PATH) != 0, "self_path");
        std::wstring command = L"\"" + std::wstring(self) + L"\" child"; STARTUPINFOW si = {}; si.cb = sizeof(si); PROCESS_INFORMATION pi = {};
        ok = CreateProcessW(self, &command[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, stage.c_str(), &si, &pi);
        event("root_child_create_error", ok ? 0 : GetLastError());
        if (ok) { WaitForSingleObject(pi.hProcess, 2000); TerminateProcess(pi.hProcess, 0xe0000021); CloseHandle(pi.hThread); CloseHandle(pi.hProcess); }
        require(!ok && GetFileAttributesW((stage + L"\\child-entry.txt").c_str()) == INVALID_FILE_ATTRIBUTES, "root_child_admission_failed");
        event("root_child_before_code_denied"); return 0;
    } catch (...) { event("root_failed"); return 1; }
}
int wmain(int argc, wchar_t **argv) {
    if (argc != 2) return 2;
    std::wstring location; wchar_t path[32768];
    if (!GetEnvironmentVariableW(L"BSP_STAGE", path, 32768)) return 3; location = path;
    if (!wcscmp(argv[1], L"child")) { HANDLE f = CreateFileW((location + L"\\child-entry.txt").c_str(), GENERIC_WRITE, 3, nullptr, CREATE_NEW, 0, nullptr); if (f != INVALID_HANDLE_VALUE) CloseHandle(f); return 0; }
    bool is_helper = !wcscmp(argv[1], L"helper"); if (!is_helper && wcscmp(argv[1], L"root")) return 4;
    if (_wfopen_s(&journal, (location + (is_helper ? L"\\helper.log" : L"\\root.log")).c_str(), L"w")) return 5;
    int code = is_helper ? helper() : root(); fclose(journal); return code;
}
