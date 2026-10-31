#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua predicates require MSVC Win32.
#endif

namespace bsp {

// Ordinary Source interfaces for complete normal 006B7EE0/006B7F00/006B7F60.
// Original: ECX raw machine, stack index, RET4, full EAX0/1 (not incidental
// upper bits). Each captures index and calls the genuine raw type leaf ONCE,
// inheriting its CURRENT machine+4 read and actual linked Source Lua domain.
// Caller retains real machine/state/stack/code/index provenance and lifetime.
// No model cast, guard/default, extra observation, Frame, owner, cleanup,
// stack restoration, catch, retry, Original private ABI or VM/startup grant.
std::int32_t is_nil_native_mission_lua_006b7ee0(
    const void* actual_machine, std::int32_t index);
std::int32_t is_table_native_mission_lua_006b7f00(
    const void* actual_machine, std::int32_t index);

// Strict type4 only. This is not lua_isstring, which also admits number3.
std::int32_t is_string_native_mission_lua_006b7f60(
    const void* actual_machine, std::int32_t index);

} // namespace bsp
