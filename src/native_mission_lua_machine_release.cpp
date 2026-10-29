#include "bsp/native_mission_lua_machine_release.hpp"
#include "bsp/native_game_current_virtual_calls.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission Lua machine release requires MSVC Win32.
#endif

namespace bsp {
void release_native_mission_lua_machine_008844f0(
    void* actual_host, NativeGameCurrentVirtualCalls& actual_dispatch) {
    static_assert(sizeof(void*)==4 && sizeof(std::uint32_t)==4);
    const auto address=reinterpret_cast<std::uint32_t>(actual_host)+4u;
    auto& machine_cell=*reinterpret_cast<volatile std::uint32_t*>(address);
    void* const captured=reinterpret_cast<void*>(machine_cell);
    if(captured) {
        // Qualified real implementation, not an overridable empty provider or
        // a cast to the projected MissionLuaHostState/LuaMachine C++ classes.
        actual_dispatch.NativeGameCurrentVirtualCalls::virtual_scalar(captured,0,1);
        machine_cell=0;
    }
}
} // namespace bsp
