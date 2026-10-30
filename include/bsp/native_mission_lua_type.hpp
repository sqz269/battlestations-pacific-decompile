#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua type access requires MSVC Win32.
#endif

namespace bsp {

// Complete normal 006B7EC0..006B7ECE. Original ECX=actual raw machine,
// stack(index), RET4. Ordinary Source interface, not its private/binary ABI.
// Capture index first, then read CURRENT machine+4 once and call the genuine
// linked lua_type once; return its actual signed result without mapping.
// No LuaMachine cast, state/profile/index validation, default, owner creation,
// cleanup, stack restoration, catch or retry. No post-call payload reads.
//
// Caller retains genuine machine backing and the reached live state/stack/code
// graph valid for the linked Lua provider and actual index. A numeric Original
// state image is not an admitted Source Lua state. No factory, raw886E00,
// VM/setup, caller-stack alias, fault, runtime or startup permission is supplied.
std::int32_t type_native_mission_lua_006b7ec0(
    const void* actual_machine, std::int32_t index);

} // namespace bsp
