#include "bsp/native_online_notification_leaves.hpp"
#include "bsp/xlive_library.hpp"
#include "bsp/xlive_updates.hpp"

#include <cstring>
#include <stdexcept>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native online notification leaves require MSVC Win32 storage and ABI.
#endif

namespace bsp {
namespace {
template<class T> T read_manager(const NativeOnlineManagerStorage& manager,
    std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, reinterpret_cast<const std::byte*>(&manager) + offset, sizeof(result));
    return result;
}

// Match the image's two-byte-at-a-time strcmp on valid bounded SDK names.
// A successful provider must define the entire 128-byte image because A3E668
// copies it all even when only the prefix participates in the comparison.
bool differs_from_cached_name(const NativeOnlineManagerStorage& manager,
    const NativeOnlineName128& name) {
    for (std::size_t i = 0; i != name.size(); ++i) {
        const auto incoming = std::to_integer<std::uint8_t>(name[i]);
        if (read_manager<std::uint8_t>(manager, 0x90 + i) != incoming) return true;
        if (incoming == 0) return false;
    }
    throw std::runtime_error("Native online profile name has no bounded terminator");
}

template<class Result, class... Args>
Result ordinal(void* module, std::uint16_t number, Args... args) {
    static_assert(sizeof(void*) == 4);
    const auto address = GetProcAddress(static_cast<HMODULE>(module), MAKEINTRESOURCEA(number));
    if (!address) throw std::runtime_error("Missing XLive ordinal " + std::to_string(number));
    using Function = Result (__stdcall*)(Args...);
    Function function;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function(args...);
}
} // namespace

NativeOnlineNotificationLeafRuntime::NativeOnlineNotificationLeafRuntime(
    const XLiveLibrary& library) : module_(library.module_handle()) {
    if (!module_) throw std::invalid_argument("Native online notification leaves need live XLive");
}

std::uint32_t NativeOnlineNotificationLeafRuntime::user_get_name_00a4d566(
    std::uint32_t user, NativeOnlineName128& output, std::uint32_t capacity) {
    if (capacity > output.size()) throw std::invalid_argument("Native name capacity exceeds 128");
    return ordinal<std::uint32_t>(module_, 5263, user,
        reinterpret_cast<char*>(output.data()), capacity);
}

void NativeOnlineNotificationLeafRuntime::call_callback18(std::uint32_t target,
    std::uint32_t incoming_ecx) {
    if (target != 0x00735520u)
        throw std::runtime_error("Unbound native online manager callback18 identity");
    static_cast<void>(ordinal<std::uint32_t>(module_, 5277, incoming_ecx, 0x8001u, 4u));
}

std::int32_t NativeOnlineNotificationLeafRuntime::get_update_information_00a4d59c(
    NativeOnlineUpdateInformation218& output) {
    return ordinal<std::int32_t>(module_, 5022, &output);
}
LSTATUS NativeOnlineNotificationLeafRuntime::reg_open_key(HKEY root, const char* subkey,
    DWORD options, REGSAM access, HKEY& result) {
    return RegOpenKeyExA(root, subkey, options, access, &result);
}
LSTATUS NativeOnlineNotificationLeafRuntime::reg_query_value(HKEY key, const char* name,
    DWORD* reserved, DWORD& type, BYTE* data, DWORD& bytes) {
    return RegQueryValueExA(key, name, reserved, &type, data, &bytes);
}
LSTATUS NativeOnlineNotificationLeafRuntime::reg_close_key(HKEY key) {
    return RegCloseKey(key);
}
BOOL NativeOnlineNotificationLeafRuntime::shell_execute(SHELLEXECUTEINFOA& request) {
    return ShellExecuteExA(&request);
}

void refresh_native_online_profile_name_00a3e600(NativeOnlineManagerStorage& manager,
    std::uint8_t mask, NativeOnlineProfileNameCalls& calls) {
    if ((mask & 1u) == 0) return;
    NativeOnlineName128 name; // Raw stack preimage is unspecified, not zero-filled.
    if (calls.user_get_name_00a4d566(0, name, 0x80) != 0) return;
    if (!differs_from_cached_name(manager, name)) return;
    std::memcpy(reinterpret_cast<std::byte*>(&manager) + 0x90, name.data(), name.size());
    if (read_manager<std::uint32_t>(manager, 0x11c) != 0) return;
    const auto callback = read_manager<std::uint32_t>(manager, 0x18);
    if (callback != 0) calls.call_callback18(callback, 0);
}

bool title_native_online_update_path_00a3ff20(NativeString& output,
    NativeStringStorage& storage, NativeOnlineUpdateCalls& calls) {
    NativeOnlineUpdateInformation218 info{};
    info.cb_size_00 = 0x218;
    if (calls.get_update_information_00a4d59c(info) < 0 || info.type_04 != 0)
        return false;

    // Win32 wchar_t is 16 bits. 00436630 measures the full UTF16 length,
    // allocates that many narrow bytes, then copies low bytes through low-byte
    // zero. It does not Unicode-convert or zero the unfilled allocation tail.
    NativeString temporary;
    construct_narrow_from_wide_00436630(temporary, info.path_10.data(), storage);
    const auto* const source = temporary.data();
    const auto length = temporary.length();
    try {
        if (&output != &temporary) {
            output.resize_0041dd40(storage, length, true);
            if (length != 0) std::memcpy(output.data(), source, output.length());
        }
    } catch (...) {
        temporary.release_to(storage);
        throw;
    }
    temporary.release_to(storage);
    return true;
}

bool system_native_online_update_path_00a3fde0(NativeString& output,
    NativeStringStorage& storage, NativeOnlineUpdateCalls& calls) {
    bool assigned_directory = false;
    HKEY key;
    if (calls.reg_open_key(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Eidos\\Battlestations Pacific", 0, 0x20019, key) == ERROR_SUCCESS) {
        DWORD bytes = 0x400;
        DWORD type;
        std::array<BYTE, 1024> directory; // Query result preimage is unspecified.
        if (calls.reg_query_value(key, "ApplicationDir", nullptr, type,
                directory.data(), bytes) == ERROR_SUCCESS && type == REG_SZ) {
            assign_native_cstring_0041e350(output,
                reinterpret_cast<const char*>(directory.data()), storage);
            assigned_directory = true;
        }
        // Original has no registry-key EH owner: assignment failure bypasses close.
        calls.reg_close_key(key);
    }

    NativeString suffix;
    suffix.assign_0041e870(storage, "BattlestationsPacific.exe");
    const auto* const source = suffix.data();
    const auto length = suffix.length();
    try {
        if (length != 0) {
            const auto previous = output.length();
            output.resize_0041dd40(storage, previous + length, true);
            std::memcpy(output.data() + previous, source, length);
        }
    } catch (...) {
        suffix.release_to(storage);
        throw;
    }
    suffix.release_to(storage);
    return assigned_directory;
}

bool launch_native_online_update_00a3e560(const char* executable,
    const char* parameters, NativeOnlineUpdateCalls& calls) {
    if (executable == nullptr) return false;
    static_assert(sizeof(SHELLEXECUTEINFOA) == 0x3c);
    SHELLEXECUTEINFOA request{};
    request.cbSize = 0x3c;
    request.fMask = 0x400;
    request.lpVerb = "open";
    request.lpFile = executable;
    request.lpParameters = parameters;
    request.nShow = 5;
    return calls.shell_execute(request) != FALSE;
}
} // namespace bsp
