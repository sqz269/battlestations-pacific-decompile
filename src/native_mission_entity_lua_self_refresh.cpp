#include "bsp/native_mission_entity_lua_self_refresh.hpp"

#include "bsp/native_mission_entity_lua_self.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission entity Lua self refresh requires MSVC Win32.
#endif

namespace bsp {

void refresh_native_mission_entity_lua_self_00928b73_fragment(
    const NativeString& actual_entity_key_178,
    NativeLuaObjectStorage& persistent_self,
    void* volatile& actual_world_publication_00e188a8) {
    NativeLuaObjectStorage temporary;
    auto* const returned = construct_native_mission_entity_lua_self_00927b40(
        actual_entity_key_178, temporary, actual_world_publication_00e188a8);

    // Native state 6: the distinct temporary is complete. Assignment releases
    // persistent_self before reading the returned object's current fields.
    try {
        assign_native_lua_object_00b67690(persistent_self, *returned);
    } catch (...) {
        destroy_native_lua_object_00b67700(temporary);
        throw;
    }

    // Native state 1 precedes this attempt. Keep it outside the catch so a
    // destruction failure never triggers a second cleanup of the temporary.
    destroy_native_lua_object_00b67700(temporary);
}

} // namespace bsp
