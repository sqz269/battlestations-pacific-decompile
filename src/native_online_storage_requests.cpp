#include "bsp/native_online_storage_requests.hpp"

#include <Windows.h>

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(wchar_t) == 2);
static_assert(sizeof(NativeOnlineStorageBuildPath) == 4);

std::byte* base(NativeOnlineManagerStorage& manager) {
    return reinterpret_cast<std::byte*>(&manager);
}
std::uint32_t read32(NativeOnlineManagerStorage& manager, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, base(manager) + offset, sizeof(value));
    return value;
}
void write32(NativeOnlineManagerStorage& manager, std::size_t offset, std::uint32_t value) {
    std::memcpy(base(manager) + offset, &value, sizeof(value));
}
} // namespace

NativeOnlineStorageBuildPath resolve_native_online_storage_build_path(void* loaded_module) {
    if (!loaded_module) throw std::invalid_argument("XLive module is not loaded");
    const FARPROC entry = GetProcAddress(static_cast<HMODULE>(loaded_module), MAKEINTRESOURCEA(5344));
    if (!entry) throw std::runtime_error("Missing XLive ordinal 5344");
    NativeOnlineStorageBuildPath build_path;
    static_assert(sizeof(entry) == sizeof(build_path));
    std::memcpy(&build_path, &entry, sizeof(build_path));
    return build_path;
}

void build_native_online_storage_path_00a3ed10(
    NativeOnlineManagerStorage& manager, NativeOnlineStorageBuildPath build_path) {
    const auto user = read32(manager, 0x11c); // original EAX is reused for SDK argument 1
    std::uint32_t path_bytes = 0x200; // original [ESP+4] local
    if (read32(manager, 0x8c + static_cast<std::size_t>(user) * 4) != 2) return;
    auto* output = reinterpret_cast<wchar_t*>(base(manager) + 0x154);
    if (build_path(user, 3, nullptr, 0, L"DropRates", output, &path_bytes) == 0)
        write32(manager, 0x12c, 1);
}

void request_native_online_storage_upload_00a3f4a0(
    NativeOnlineManagerStorage& manager, std::uint32_t first,
    std::uint32_t second, const NativeOnlineStorageSdk& sdk,
    const NativeOnlineStorageMemory& memory) {
    // The native CMP/SBB/AND normalization is the identity for every DWORD,
    // including zero. Preserve its observable write/state-read/write order.
    write32(manager, 0x358, first);
    const auto state = read32(manager, 0x12c);
    write32(manager, 0x35c, second);
    if (state == 1 || state == 9 || state == 4 ||
        state == 5 || state == 8 || state == 10)
        upload_native_online_storage_00a3ef20(manager, sdk, memory);
    // The following native diagnostic target 4254B0 is exactly RET.
}

void request_native_online_storage_download_00a3f500(
    NativeOnlineManagerStorage& manager, const NativeOnlineStorageSdk& sdk,
    const NativeOnlineStorageMemory& memory) {
    const auto state = read32(manager, 0x12c);
    if (state == 1 || state == 4 || state == 9 || state == 5 || state == 8)
        download_native_online_storage_00a3ed60(manager, sdk, memory);
    // The following native diagnostic target 4254B0 is exactly RET.
}
} // namespace bsp
