#pragma once

#include "bsp/native_online_notifications.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
// The six XLive imports have Win32 stdcall ABI and DWORD returns. Each pointer
// occupies one stack word. The output pointers address the captured 3F0h owner;
// the SDK and manager must outlive every outstanding operation. There is no
// projected XLiveSystemPumpContext or XLiveOwnerAllocation in this interface.
struct NativeOnlineStorageSdk final {
    using Download = std::uint32_t (__stdcall*)(std::uint32_t, const wchar_t*,
        std::uint32_t, void*, std::uint32_t, void*, void*);
    using Upload = std::uint32_t (__stdcall*)(std::uint32_t, const wchar_t*,
        std::uint32_t, const void*, void*);
    using Progress = std::uint32_t (__stdcall*)(void*, std::uint32_t*, void*, void*);
    using Result = std::uint32_t (__stdcall*)(void*, void*, std::int32_t);
    using Error = std::uint32_t (__stdcall*)(void*);

    Download download{};       // XStorageDownloadToMemory, ordinal 5345
    Upload upload{};           // XStorageUploadFromMemory, ordinal 5305
    Progress download_progress{}; // ordinal 5307
    Progress upload_progress{};   // ordinal 5304
    Result overlapped_result{};   // ordinal 1083
    Error overlapped_error{};     // ordinal 1082
};

// Resolves the observed imports from an ALREADY loaded XLive module. Throws
// when an ordinal is absent; does not load a DLL or perform a storage request.
NativeOnlineStorageSdk resolve_native_online_storage_sdk(void* loaded_module);

// The original allocation calls operator new (BF55BE, backed by its CRT malloc)
// and release calls that same CRT's free (BF6989). Supply a matching pair; the
// caller owns these entrypoints and must preserve their lifetime. Allocation
// must return non-null or throw, just as the original operator new does.
struct NativeOnlineStorageMemory final {
    void* (__cdecl* allocate)(std::size_t);
    void (__cdecl* release)(void*);
};

// Complete normal A3ED60/A3EF20 bodies, original ECX=captured manager, RET.
// Explicit references are the C++ source interface, not binary replacements.
// The manager's +3B4 index must designate a readable DWORD at +8C+4*index;
// its +14C pointer, when non-null, must come from the matching allocation pair.
// Never retire the manager/module, replace the path, or reenter storage on it
// while an SDK request is pending. No callback/queue is owned by these bodies.
void download_native_online_storage_00a3ed60(NativeOnlineManagerStorage& manager,
    const NativeOnlineStorageSdk& sdk, const NativeOnlineStorageMemory& memory);
void upload_native_online_storage_00a3ef20(NativeOnlineManagerStorage& manager,
    const NativeOnlineStorageSdk& sdk, const NativeOnlineStorageMemory& memory);
} // namespace bsp
