#pragma once

#include "bsp/native_online_storage.hpp"

#include <cstdint>

namespace bsp {
// XStorageBuildServerPath: game thunk A4D578, XLive ordinal 5344, seven
// four-byte Win32 stdcall arguments and DWORD return. The UTF-16 output and
// size pointer are borrowed by this synchronous call; the owner keeps the
// resulting +154 path for later asynchronous transfers.
using NativeOnlineStorageBuildPath = std::uint32_t (__stdcall*)(
    std::uint32_t, std::uint32_t, const void*, std::uint32_t,
    const wchar_t*, wchar_t*, std::uint32_t*);

// Resolve from an already-loaded XLive module. No load, SDK call or request.
NativeOnlineStorageBuildPath resolve_native_online_storage_build_path(void* loaded_module);

// Complete A3ED10 normal body: ECX=captured actual 3F0h manager; RET.
// The +11C user DWORD must designate a readable +8C+4*user DWORD. The
// supplied ordinal is required only when the indexed gate equals 2.
void build_native_online_storage_path_00a3ed10(
    NativeOnlineManagerStorage& manager, NativeOnlineStorageBuildPath build_path);

// Complete A3F4A0: ECX=actual manager, two stacked DWORDs, RET 8. The
// high-level C++ call expresses those original stack inputs explicitly.
// It writes both payload DWORDs even when no upload is submitted.
void request_native_online_storage_upload_00a3f4a0(
    NativeOnlineManagerStorage& manager, std::uint32_t first,
    std::uint32_t second, const NativeOnlineStorageSdk& sdk,
    const NativeOnlineStorageMemory& memory);

// Complete A3F500: ECX=actual manager, RET. The caller gate may enter the
// download callee in states where the callee intentionally does nothing.
void request_native_online_storage_download_00a3f500(
    NativeOnlineManagerStorage& manager, const NativeOnlineStorageSdk& sdk,
    const NativeOnlineStorageMemory& memory);
} // namespace bsp
