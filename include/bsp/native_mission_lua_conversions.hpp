#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua conversions require MSVC Win32.
#endif

namespace bsp {

// Whole normal 006B8000/8020/80A0 over genuine retained raw machine backing.
// Capture index first, then read CURRENT machine+4 exactly once and call the
// matching linked Source Lua C API once. No typed LuaMachine adoption, cached
// state, validation/default, owner/VM creation, Context/Frame or local cleanup.
// Caller retains the matching actual state/stack/code/index domain through
// the call; a numeric Original VM image is not an admitted Source Lua state.

// TEST full API int / SETNZ AL: logical bool0/1, not upper24 Native EAX.
bool to_boolean_native_mission_lua_006b8000(
    const void* actual_machine, std::int32_t index);

// Consume the real cdecl lua_tonumber ST0 result directly, FSTP private F32,
// then FLD that same F32 for the float return. No F64 snapshot, C++float cast,
// SSE substitution or extra narrowing in the public caller. Actual CURRENT
// x87 environment and instruction status/exception effects remain operative;
// no CW/SW/MXCSR snapshot/reset/restore/mask/clear policy is added.
float to_float32_native_mission_lua_006b8020(
    const void* actual_machine, std::int32_t index);

// Actual pointer result (including real null), not ownership/free permission.
void* to_userdata_native_mission_lua_006b80a0(
    const void* actual_machine, std::int32_t index);

// No post-call machine/state payload read, Lua stack restoration, catch,
// noexcept or retry. Ordinary Source/private-F32 normalization, not Original
// argument-home addresses/overwrite, register ABI/FH3/SEH/fault/nonlocal or
// concurrency equivalence. No factory/VM/parent-builder/startup/runtime grant.
} // namespace bsp
