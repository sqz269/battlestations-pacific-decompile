#pragma once
#include "bsp/native_string.hpp"
#include <cstdint>

namespace bsp {
// Complete ordinary normal004D2620 through a new MSVC Win32 interface.
// Always destroy the genuine initialized10Ch registry via008F4F00, including
// its real first clear and second empty clear. Iff flags bit0 is set, free
// the root with the matching current CRT service. Return its ORIGINAL address
// bits captured before destruction/free; the integer grants no deleted access.
std::uintptr_t delete_native_scene_enum_registry_004d2620(void* actual_10c_owner,
    std::uint32_t flags, void* actual_table_pool, void* actual_symbol_pool,
    NativeStringRawPoolContext& actual_strings);

// Same genuine current-CRT10Ch root/initializer004D3069, distinct real14h
// tableE175E8/symbolE17578 pools, rawstrings and unique owning current-CRT19Ch
// CEnum-or-null domain as008F4F00. Ordinary destruction once per live root;
// a retained root needs explicit release or genuine reconstruction before reuse.
// No reset/null/default, callbacks, aliases/reentry/concurrency/fault policy,
// foreign heaps/classes or post-free access. Original ECX/flags/RET4 class ABI,
// FS/EH/historical CRT, profile dispatch/global startup/world/game are external.
// Nested004D0EB0 remains separate: owner+4 is an interior map and cannot be freed.
} // namespace bsp
