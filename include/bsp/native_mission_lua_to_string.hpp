#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua ToString requires MSVC Win32.
#endif

namespace bsp {

// Whole game-owned 006B8060, ordinary Source API. Capture signed index FIRST,
// read CURRENT machine+4 once, then real lua_tolstring(state,index,nullptr).
// Preserve the actual NULL length sink and returned borrowed pointer, including
// genuine conversion-failure NULL. No wrapper strlen, length result or copy.
//
// Caller retains actual raw backing and a matching genuine linked-Source
// VM/stack/code/index/capacity graph through the lower call, including captured
// state lifetime if publication changes. Keep the actual Lua string rooted
// through subsequent borrowed-pointer reads and serialize mutation/GC/reentry.
// No LuaMachine cast, state creation/default/guard, Context/Frame, owner,
// cleanup, catch/pcall, stack restoration, retry or post-call payload read.
// Original interpreter/private ABI/FH3/SEH/fault/runtime and parent/factory/
// VM/startup are unproved; no Native/vendor body is reconstructed here.
const char* to_string_native_mission_lua_006b8060(
    const void* actual_machine, std::int32_t index);

} // namespace bsp
