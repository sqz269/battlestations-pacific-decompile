#include "bsp/xlive_pipe_transport.hpp"

#include <aclapi.h>
#include <strsafe.h>
#include <array>
#include <new>

#pragma comment(lib, "advapi32.lib")

namespace bsp {
static_assert(sizeof(XLivePipeNativeState) == 0x30);
static_assert(offsetof(XLivePipeNativeState, overlapped_0c) == 0x0c);
static_assert(offsetof(XLivePipeNativeState, io_state_20) == 0x20);
static_assert(offsetof(XLivePipeNativeState, wait_handle_count_2c) == 0x2c);
static_assert(sizeof(PROCESSENTRY32W) == 0x22c);
static_assert(sizeof(EXPLICIT_ACCESS_W) == 0x20);
namespace {
DWORD last_error(XLivePipeSystemHost& host) {
    const auto value = host.get_last_error();
    return value ? value : 0x507u;
}
std::int32_t to_hresult(DWORD value) noexcept {
    auto result = static_cast<std::int32_t>(value);
    if (result > 0) result = static_cast<std::int32_t>((value & 0xffffu) | 0x80070000u);
    return result;
}
}

XLivePipeTransport* Win32XLivePipeSystemHost::allocate_owner() { return new XLivePipeTransport; }
void Win32XLivePipeSystemHost::free_owner(XLivePipeTransport* value) noexcept { delete value; }
void Win32XLivePipeSystemHost::set_last_error(DWORD value) { SetLastError(value); }
DWORD Win32XLivePipeSystemHost::get_last_error() { return GetLastError(); }
void Win32XLivePipeSystemHost::close_handle(HANDLE value) { static_cast<void>(CloseHandle(value)); }
DWORD Win32XLivePipeSystemHost::current_process_id() { return GetCurrentProcessId(); }
HANDLE Win32XLivePipeSystemHost::current_process() { return GetCurrentProcess(); }
HANDLE Win32XLivePipeSystemHost::snapshot_processes() { return CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); }
bool Win32XLivePipeSystemHost::process_first(HANDLE snapshot, PROCESSENTRY32W& entry) { return Process32FirstW(snapshot, &entry) != FALSE; }
bool Win32XLivePipeSystemHost::process_next(HANDLE snapshot, PROCESSENTRY32W& entry) { return Process32NextW(snapshot, &entry) != FALSE; }
HANDLE Win32XLivePipeSystemHost::open_parent_process(DWORD pid) { return OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid); }
bool Win32XLivePipeSystemHost::process_times(HANDLE process, FILETIME& created, FILETIME& exited, FILETIME& kernel, FILETIME& user) {
    return GetProcessTimes(process, &created, &exited, &kernel, &user) != FALSE;
}
LONG Win32XLivePipeSystemHost::compare_file_time(const FILETIME& a, const FILETIME& b) { return CompareFileTime(&a, &b); }
bool Win32XLivePipeSystemHost::open_process_token(HANDLE process, HANDLE& token) { return OpenProcessToken(process, TOKEN_QUERY, &token) != FALSE; }
bool Win32XLivePipeSystemHost::token_groups(HANDLE token, void* buffer, DWORD bytes, DWORD& required) { return GetTokenInformation(token, TokenGroups, buffer, bytes, &required) != FALSE; }
HLOCAL Win32XLivePipeSystemHost::local_alloc(SIZE_T bytes) { return LocalAlloc(LPTR, bytes); }
void Win32XLivePipeSystemHost::local_free(HLOCAL value) { static_cast<void>(LocalFree(value)); }
DWORD Win32XLivePipeSystemHost::sid_length(PSID sid) { return GetLengthSid(sid); }
bool Win32XLivePipeSystemHost::copy_sid(DWORD bytes, PSID target, PSID source) { return CopySid(bytes, target, source) != FALSE; }
bool Win32XLivePipeSystemHost::allocate_network_sid(PSID& sid) {
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    return AllocateAndInitializeSid(&authority, 1, SECURITY_NETWORK_RID, 0, 0, 0, 0, 0, 0, 0, &sid) != FALSE;
}
void Win32XLivePipeSystemHost::free_sid(PSID sid) { static_cast<void>(FreeSid(sid)); }
DWORD Win32XLivePipeSystemHost::set_acl_entries(EXPLICIT_ACCESS_W* entries, PACL& acl) { return SetEntriesInAclW(2, entries, nullptr, &acl); }
bool Win32XLivePipeSystemHost::initialize_security_descriptor(PSECURITY_DESCRIPTOR descriptor) { return InitializeSecurityDescriptor(descriptor, 1) != FALSE; }
bool Win32XLivePipeSystemHost::set_security_dacl(PSECURITY_DESCRIPTOR descriptor, PACL acl) { return SetSecurityDescriptorDacl(descriptor, TRUE, acl, TRUE) != FALSE; }
HANDLE Win32XLivePipeSystemHost::create_server_pipe(const wchar_t* path, SECURITY_ATTRIBUTES& attributes) {
    return CreateNamedPipeW(path, 0x40080003, 6, 1, 0x400, 0x400, 5000, &attributes);
}
HANDLE Win32XLivePipeSystemHost::open_client_pipe(const wchar_t* path) { return CreateFileW(path, 0xc0000000, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr); }
bool Win32XLivePipeSystemHost::set_client_message_mode(HANDLE pipe, DWORD& mode) { return SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr) != FALSE; }
HANDLE Win32XLivePipeSystemHost::create_completion_event() { return CreateEventW(nullptr, TRUE, TRUE, nullptr); }

XLivePipeTransport* construct_xlive_pipe_00a5e145(XLivePipeTransport& owner) {
    owner.protocol->initialize_protocol_00a5f416(owner.native.protocol_00);
    owner.native.pipe_08 = INVALID_HANDLE_VALUE;
    owner.native.wait_handle_count_2c = 0;
    owner.native.io_state_20 = 3;
    owner.native.completion_event_24 = nullptr;
    owner.native.stop_event_28 = nullptr;
    return &owner;
}

void destroy_xlive_pipe_00a5e710(XLivePipeTransport* owner) {
    if (!owner) return;
    auto& n = owner->native;
    auto& system = *owner->system;
    if (n.pipe_08 != INVALID_HANDLE_VALUE) {
        system.close_handle(n.pipe_08);
        n.pipe_08 = INVALID_HANDLE_VALUE;
    }
    if (n.completion_event_24) {
        system.close_handle(n.completion_event_24);
        n.completion_event_24 = nullptr;
    }
    owner->protocol->destroy_protocol_00a5f371(n.protocol_00);
    system.free_owner(owner);
}
void close_xlive_pipe_00a5de68(XLivePipeTransport* owner) {
    if (owner && reinterpret_cast<std::uintptr_t>(owner) != static_cast<std::uintptr_t>(-1))
        destroy_xlive_pipe_00a5e710(owner);
}

DWORD find_xlive_pipe_parent_00a5e557(DWORD& output, XLivePipeSystemHost& system) {
    system.set_last_error(0);
    const auto snapshot = system.snapshot_processes();
    if (snapshot == INVALID_HANDLE_VALUE) return last_error(system);
    PROCESSENTRY32W entry;
    entry.dwSize = 0x22c;
    const auto current_id = system.current_process_id();
    system.set_last_error(0);
    bool found = system.process_first(snapshot, entry);
    DWORD error;
    for (;;) {
        if (!found) { error = last_error(system); break; }
        system.set_last_error(0);
        if (entry.th32ProcessID != current_id) {
            found = system.process_next(snapshot, entry);
            continue;
        }
        const auto parent = system.open_parent_process(entry.th32ParentProcessID);
        if (!parent) error = last_error(system);
        else {
            FILETIME parent_created, current_created, exited, kernel, user;
            system.set_last_error(0);
            if (!system.process_times(parent, parent_created, exited, kernel, user)) error = last_error(system);
            else {
                system.set_last_error(0);
                const auto process = system.current_process();
                if (!system.process_times(process, current_created, exited, kernel, user)) error = last_error(system);
                else if (system.compare_file_time(parent_created, current_created) == -1) {
                    output = entry.th32ParentProcessID;
                    error = 0;
                } else error = 0xe9;
            }
            system.close_handle(parent);
        }
        break;
    }
    system.close_handle(snapshot);
    return error;
}

DWORD copy_xlive_pipe_logon_sid_00a5e3d0(HLOCAL& output, XLivePipeSystemHost& system) {
    DWORD bytes = 0;
    HLOCAL copied = nullptr;
    system.set_last_error(0);
    HANDLE token;
    const auto process = system.current_process();
    if (!system.open_process_token(process, token)) return last_error(system);
    system.set_last_error(0);
    DWORD error = system.token_groups(token, nullptr, 0, bytes) ? 0xdu : last_error(system);
    if (error == ERROR_INSUFFICIENT_BUFFER) {
        system.set_last_error(0);
        auto* groups = static_cast<TOKEN_GROUPS*>(system.local_alloc(bytes));
        if (!groups) error = last_error(system);
        else {
            system.set_last_error(0);
            if (!system.token_groups(token, groups, bytes, bytes)) error = last_error(system);
            else {
                error = 0x545;
                for (DWORD index = 0; index < groups->GroupCount; ++index) {
                    if ((groups->Groups[index].Attributes & 0xc0000000u) != 0xc0000000u) continue;
                    const auto* group = &groups->Groups[index];
                    bytes = system.sid_length(group->Sid);
                    system.set_last_error(0);
                    copied = system.local_alloc(bytes);
                    error = 0;
                    if (!copied) error = last_error(system);
                    else {
                        system.set_last_error(0);
                        if (!system.copy_sid(bytes, copied, group->Sid)) error = last_error(system);
                    }
                    break;
                }
            }
            system.local_free(groups);
        }
    }
    system.close_handle(token);
    if (!error) output = copied;
    else if (copied) system.local_free(copied);
    return error;
}

std::int32_t create_xlive_pipe_00a5e844(std::uint32_t mode, HANDLE stop_event,
    XLivePipeTransport** output, XLivePipeProtocolHost& protocol, XLivePipeSystemHost& system) {
    if (!output) return static_cast<std::int32_t>(0x80070057u);
    PSID network_sid = nullptr;
    HLOCAL logon_sid = nullptr;
    PACL acl = nullptr;
    HLOCAL descriptor = nullptr;
    auto* owner = system.allocate_owner();
    DWORD error = ERROR_OUTOFMEMORY;
    if (owner) {
        owner->protocol = &protocol;
        owner->system = &system;
        construct_xlive_pipe_00a5e145(*owner);
        auto& n = owner->native;
        n.mode_04 = mode;
        DWORD pid;
        if (mode) { pid = system.current_process_id(); error = 0; }
        else error = find_xlive_pipe_parent_00a5e557(pid, system);
        if (!error) {
            std::array<wchar_t, 18> path;
            // Fixed A5E6CD invocation: exact 17-character path plus NUL.
            // Use the SDK's safe formatter; do not port its generic CRT body.
            static_cast<void>(StringCchPrintfW(path.data(), path.size(), L"\\\\.\\pipe\\%08x", static_cast<unsigned>(pid)));
            if (mode) {
                system.set_last_error(0);
                if (!system.allocate_network_sid(network_sid)) {
                    error = last_error(system);
                    network_sid = nullptr; // native explicitly drops failed output
                } else {
                    EXPLICIT_ACCESS_W entries[2]{};
                    entries[0].grfAccessPermissions = 0x1fffff;
                    entries[0].grfAccessMode = DENY_ACCESS;
                    entries[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
                    entries[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
                    entries[0].Trustee.ptstrName = static_cast<LPWSTR>(network_sid);
                    error = copy_xlive_pipe_logon_sid_00a5e3d0(logon_sid, system);
                    if (!error) {
                        entries[1].grfAccessPermissions = 0xc0000000;
                        entries[1].grfAccessMode = SET_ACCESS;
                        entries[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
                        entries[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
                        entries[1].Trustee.ptstrName = static_cast<LPWSTR>(logon_sid);
                        error = system.set_acl_entries(entries, acl);
                        if (!error) {
                            system.set_last_error(0);
                            descriptor = system.local_alloc(0x14);
                            if (!descriptor) error = last_error(system);
                            else {
                                system.set_last_error(0);
                                if (!system.initialize_security_descriptor(descriptor)) error = last_error(system);
                                else {
                                    system.set_last_error(0);
                                    if (!system.set_security_dacl(descriptor, acl)) error = last_error(system);
                                    else {
                                        SECURITY_ATTRIBUTES attributes{0xc, descriptor, FALSE};
                                        system.set_last_error(0);
                                        n.pipe_08 = system.create_server_pipe(path.data(), attributes);
                                        if (n.pipe_08 == INVALID_HANDLE_VALUE) error = last_error(system);
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                system.set_last_error(0);
                n.pipe_08 = system.open_client_pipe(path.data());
                if (n.pipe_08 == INVALID_HANDLE_VALUE) error = last_error(system);
                else {
                    system.set_last_error(0);
                    DWORD message_mode = 2;
                    if (!system.set_client_message_mode(n.pipe_08, message_mode)) error = last_error(system);
                }
            }
            if (!error) {
                system.set_last_error(0);
                n.completion_event_24 = system.create_completion_event();
                if (!n.completion_event_24) error = last_error(system);
                else {
                    n.overlapped_0c.hEvent = n.completion_event_24;
                    if (stop_event) {
                        n.stop_event_28 = stop_event;
                        n.wait_handle_count_2c = 2;
                    } else n.wait_handle_count_2c = 1;
                }
            }
        }
    }
    if (network_sid) system.free_sid(network_sid);
    if (logon_sid) system.local_free(logon_sid);
    if (acl) system.local_free(acl);
    if (descriptor) system.local_free(descriptor);
    if (error && owner) {
        // Failure cleanup closes without rewriting fields, unlike E710.
        if (owner->native.pipe_08 != INVALID_HANDLE_VALUE) system.close_handle(owner->native.pipe_08);
        if (owner->native.completion_event_24) system.close_handle(owner->native.completion_event_24);
        protocol.destroy_protocol_00a5f371(owner->native.protocol_00);
        system.free_owner(owner);
        owner = nullptr; // raw missing A5EB46 XOR EDI,EDI
    }
    *output = owner;
    return to_hresult(error);
}

std::int32_t open_xlive_pipe_00a5de34(std::uint32_t mode, HANDLE stop_event,
    XLivePipeTransport** output, XLivePipeProtocolHost& protocol, XLivePipeSystemHost& system) {
    if (!output) return static_cast<std::int32_t>(0x80070057u);
    XLivePipeTransport* local;
    const auto result = create_xlive_pipe_00a5e844(mode, stop_event, &local, protocol, system);
    *output = result < 0 ? nullptr : local;
    return result;
}
} // namespace bsp
