#pragma once

#include "bsp/native_online_storage.hpp"

namespace bsp {
struct NativeOnlineAchievementsSdk final {
    using Write = std::uint32_t (__stdcall*)(std::uint32_t, const void*, void*);
    Write write{}; // XUserWriteAchievements, ordinal 5278; pairs are {user,id} DWORDs
    NativeOnlineStorageSdk::Result overlapped_result{}; // ordinal 1083
    NativeOnlineStorageSdk::Error overlapped_error{};   // ordinal 1082
};

// Resolves imports from an ALREADY loaded module; no loading or SDK request.
NativeOnlineAchievementsSdk resolve_native_online_achievements_sdk(void* loaded_module);

// Bind these to the same CRT policy as the original BF6713/BF67A7 calls.
// BF6713 calls BF66EF with five zero arguments; a configured handler can return.
// BF67A7 reports errno 22/34 through that handler, does not zero the destination,
// and returns the error if the handler returns. Its result is ignored here.
struct NativeOnlineAchievementsCrt final {
    void (__cdecl* invalid_parameter_noinfo)();
    int (__cdecl* memmove_s)(void*, std::size_t, const void*, std::size_t);
};

// Complete normal A3FA70..A3FD1F body, original ECX=manager, low byte of a DWORD
// force argument, RET 4. This explicit C++ interface is not a binary replacement.
// Queue +364/+368 and batch +3A0/+3A4 are actual 32-bit pointers/counts in the
// captured 3F0h owner. No vector projection, state mirror or owner map is used.
// The queue, owner, module and matching allocation/CRT entries must remain alive.
// Mutation through synchronous SDK/CRT/allocation calls is observed by native
// reloads; concurrent access and dangling pointers have no safe source contract.
// Native force can free a pending batch: its external SDK lifetime hazard is
// preserved. Allocation must return nonnull or throw; no rollback is added.
void pump_native_online_achievements_00a3fa70(NativeOnlineManagerStorage& manager,
    std::uint32_t force_word, const NativeOnlineAchievementsSdk& sdk,
    const NativeOnlineStorageMemory& memory, const NativeOnlineAchievementsCrt& crt);
} // namespace bsp
