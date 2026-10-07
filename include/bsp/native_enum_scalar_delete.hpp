#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Complete ordinary normal008F59C0 through a new MSVC Win32 interface.
// Always destroy the genuine initialized19Ch CEnum owner via008F4E70; iff
// flags bit0 is set, release its root with the existing current CRT service.
// Return the ORIGINAL captured address bits, including on deletion. The
// integer result neither owns storage nor permits a deleted-root access.
std::uintptr_t delete_native_scene_enum_owner_008f59c0(void* actual_19c_owner,
    std::uint32_t flags, void* actual_initialized_symbol_pool_00e17578,
    NativeStringRawPoolContext& actual_strings);

// Same successful coherent/disjoint owning-key/pool domain as008F4E70, with
// a real compatible current-CRT root allocation. Ordinary destruction once
// per live owning header; retained roots need explicit free or reconstruction.
// No reset/null guard/default, allocator/profile/global/phase callback, or
// post-free access. Original ECX/stack/RET4 class ABI, EH/fault/historical CRT,
// global startup, namespace/mapped virtual lifetime, traffic/game are external.
} // namespace bsp
