#pragma once

#include "bsp/vfs_lua_scripts.hpp"

namespace bsp {

struct GameSettingsBlock;

// The image's zero-initialized 0108ff20 byte and 0108ff24/0108ff28
// NativeString, before the first input singleton call at 0073da94. Empty
// region projects the native null pointer: owner open emits no REGION chunk.
// This factory belongs to process startup; do not reset live globals with it.
LuaRuntimeGlobals make_initial_lua_runtime_globals_0108ff20();

// 00439100..0043917f, cdecl(void), EAX points to a static C string, RET.
// Queries Windows GEO_NATION/GEO_FRIENDLYNAME. US and Canada -> USA, Japan ->
// JAP, everything else (including either API failure) -> EU. This is called
// later by settings ApplyAll at 008d6112, not by initial input Lua bootstrap.
const char* user_geography_region_00439100() noexcept;

// Compose the existing settings setter with its missing global-byte mirror.
// Return its optional command text for a caller with a live 006b8ad0 sink to
// execute using arguments (text, 0, 0, 2). This helper does not deliver it.
const char* publish_lua_xbox_compatibility_008d44c0(
    GameSettingsBlock&, LuaRuntimeGlobals&, bool enabled,
    bool command_sink_present) noexcept;

// 008d612f..008d6153 within ApplyAll: assign the C-string result of 00439100
// to the global NativeString. Existing open Lua states are not rewritten.
void publish_lua_region_008d6132(LuaRuntimeGlobals&, const std::string& region);

} // namespace bsp
