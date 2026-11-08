// Focused Lua fixture for 006FE780's nil-only fallback and consumed row lifetime.
// This exercises the real Lua 5.1 provider, not original-game ABI or gameplay.
#include "bsp/dummy_depth_seg34_argon.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

int main() {
    lua_State* state = luaL_newstate();
    if (!state) return 1;
    {
        bsp::GuiLua51Host host(*state);
        std::string error;
        if (!host.execute_archive("Rows={{},{OpenAfterTime='2.5'},{OpenAfterTime=false}}", error))
            throw std::runtime_error(error);
        const auto globals = host.globals();
        const auto rows = host.get_by_name(globals, "Rows");
        host.release(globals);
        const int top = lua_gettop(state);
        const float expected[]{1.0f, 2.5f, 0.0f};
        for (int index = 1; index <= 3; ++index) {
            const auto row = host.get_by_index(rows, index);
            float result = -1.0f;
            bsp::dummy_depth_seg34_argon::LuaTable_ReadNumberOrDefault_006fe780(
                "OpenAfterTime", &result, host, row, 1.0f);
            if (result != expected[index - 1] || lua_gettop(state) != top)
                throw std::runtime_error("006FE780 value or Lua stack mismatch");
            if (host.type_of(row) != bsp::GuiLuaType::None)
                throw std::runtime_error("006FE780 retained its consumed row");
        }
        host.release(rows);
    }
    lua_close(state);
    std::cout << "006FE780: nil default, numeric-string coercion, nonnumber zero, row release and stack passed\n";
}
