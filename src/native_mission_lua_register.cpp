#include "bsp/native_mission_lua_register.hpp"

extern "C" {
#include <lua.h>
}

#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(NativeMissionLuaCFunction) == 4);
static_assert(LUA_GLOBALSINDEX == -10002);

lua_State* current_state(void* machine) noexcept {
    return *reinterpret_cast<lua_State* volatile*>(
        reinterpret_cast<std::uint32_t>(machine) + 4u);
}

} // namespace

void register_native_mission_lua_global_006b8610(void* actual_machine,
    const char* volatile name, NativeMissionLuaCFunction function) {
    void* const captured_machine = actual_machine;
    NativeMissionLuaCFunction const captured_function = function;
    lua_State* const closure_state = current_state(captured_machine);
    lua_pushcclosure(closure_state, captured_function, 0);

    const char* const captured_name = name;
    lua_State* const global_state = current_state(captured_machine);
    lua_setfield(global_state, LUA_GLOBALSINDEX, captured_name);
}

} // namespace bsp
