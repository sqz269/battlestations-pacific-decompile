#include "bsp/xlive_updates.hpp"
#include "bsp/winmain_startup.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {

LSTATUS Win32XLiveUpdatePlatform::reg_open_key(HKEY root, const char* subkey,
    DWORD options, REGSAM access, HKEY& result) {
    return RegOpenKeyExA(root, subkey, options, access, &result);
}
LSTATUS Win32XLiveUpdatePlatform::reg_query_value(HKEY key, const char* name,
    DWORD* reserved, DWORD& type, BYTE* data, DWORD& bytes) {
    return RegQueryValueExA(key, name, reserved, &type, data, &bytes);
}
LSTATUS Win32XLiveUpdatePlatform::reg_close_key(HKEY key) { return RegCloseKey(key); }
BOOL Win32XLiveUpdatePlatform::shell_execute(SHELLEXECUTEINFOA& request) {
    return ShellExecuteExA(&request);
}

NativeString& construct_narrow_from_wide_00436630(NativeString& destination,
    const wchar_t* source, NativeStringStorage& storage) {
    // The canonical empty constructor overwrites exactly the same two header
    // words before any source read, with no allocation or release.
    destination.assign_0041e870(storage, "");
    const wchar_t* end = source;
    while (*end != 0) ++end;
    const auto length = static_cast<std::uint32_t>(end - source);
    destination.resize_0041dd40(storage, length, true);
    char* output = destination.data();
    if (output != nullptr) {
        unsigned char byte;
        do {
            byte = static_cast<unsigned char>(*source++ & 0xffu);
            *output++ = static_cast<char>(byte);
        } while (byte != 0);
    }
    return destination;
}

NativeString& assign_native_cstring_0041e350(NativeString& destination,
    const char* source, NativeStringStorage& storage) {
    const auto length = source ? static_cast<std::uint32_t>(std::strlen(source)) : 0u;
    destination.resize_0041dd40(storage, length, false);
    if (destination.data() != nullptr)
        std::memcpy(destination.data(), source, destination.length());
    return destination;
}

std::wstring widen_update_path_004c5e60(const char* source) {
    if (source == nullptr)
        throw std::invalid_argument("Native byte widening requires a nonnull C string");
    return startup_widen_path(source);
}

bool title_update_path_00a3ff20(NativeString& output, XLiveUpdateInformationHost& sdk,
    NativeStringStorage& storage) {
    XLiveUpdateInformation info{};
    info.cb_size_00 = 0x218;
    if (sdk.x_live_get_update_information(info) < 0 || info.type_04 != 0) return false;
    NativeString temporary;
    construct_narrow_from_wide_00436630(temporary, info.path_10.data(), storage);
    // Native unwind state becomes0 only after the narrow constructor returns.
    const auto* data = temporary.data();
    const auto length = temporary.length();
    try {
        if (&output != &temporary) {
            output.resize_0041dd40(storage, length, true);
            if (length != 0) std::memcpy(output.data(), data, output.length());
        }
    } catch (...) {
        temporary.release_to(storage);
        throw;
    }
    temporary.release_to(storage);
    return true;
}

bool system_update_path_00a3fde0(NativeString& output, XLiveUpdatePlatformHost& platform,
    NativeStringStorage& storage) {
    bool assigned_directory = false;
    HKEY key;
    if (platform.reg_open_key(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Eidos\\Battlestations Pacific", 0, 0x20019, key) == ERROR_SUCCESS) {
        DWORD bytes = 0x400;
        DWORD type;
        std::array<BYTE, 1024> directory;
        if (platform.reg_query_value(key, "ApplicationDir", nullptr, type,
                directory.data(), bytes) == ERROR_SUCCESS && type == REG_SZ) {
            assign_native_cstring_0041e350(output,
                reinterpret_cast<const char*>(directory.data()), storage);
            assigned_directory = true;
        }
        // No native EH guard owns this key: assignment exceptions bypass close.
        platform.reg_close_key(key);
    }
    NativeString suffix;
    suffix.assign_0041e870(storage, "BattlestationsPacific.exe"); // length25, copy26
    const auto* data = suffix.data();
    const auto length = suffix.length();
    try {
        if (length != 0) {
            const auto previous = output.length();
            output.resize_0041dd40(storage, previous + length, true);
            std::memcpy(output.data() + previous, data, length);
        }
    } catch (...) {
        suffix.release_to(storage);
        throw;
    }
    suffix.release_to(storage);
    return assigned_directory;
}

bool launch_update_00a3e560(const char* executable, const char* parameters,
    XLiveUpdatePlatformHost& platform) {
    if (executable == nullptr) return false;
    static_assert(sizeof(SHELLEXECUTEINFOA) == 0x3c);
    SHELLEXECUTEINFOA request{};
    request.cbSize = 0x3c;
    request.fMask = 0x400;
    request.lpVerb = "open";
    request.lpFile = executable;
    request.lpParameters = parameters;
    request.nShow = 5;
    return platform.shell_execute(request) != FALSE;
}
} // namespace bsp
