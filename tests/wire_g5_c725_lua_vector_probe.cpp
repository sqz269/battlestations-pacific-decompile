#include "bsp/gui_lua_runtime.hpp"
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static void require(bool value) {
    if (!value) throw std::runtime_error("chain725 Lua vector probe failed");
}
static int vector_index(lua_State* state) {
    const int index = static_cast<int>(lua_tointeger(state, 2));
    lua_getglobal(state, "Visits");
    const int visits = static_cast<int>(lua_tointeger(state, -1));
    lua_pop(state, 1);
    lua_pushinteger(state, visits * 10 + index);
    lua_setglobal(state, "Visits");
    lua_getglobal(state, "Fail");
    const bool fail = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    if (index == 4 && fail) return luaL_error(state, "vector read");
    return 0;
}
struct Call {
    bsp::GuiLua51Host* lua;
    bsp::GuiLuaRef vector;
    const std::array<float, 4>* fallback;
};
static int protected_convert(lua_State* state) {
    auto& call = *static_cast<Call*>(lua_touserdata(state, lua_upvalueindex(1)));
    call.lua->to_vector4_or(call.vector, *call.fallback);
    return 0;
}

int main() {
    lua_State* state = luaL_newstate();
    require(state != nullptr);
    {
        bsp::GuiLua51Host lua(*state);
        std::string error;
        require(lua.execute_archive(
            "Visits=0; Fail=true; Vector={[1]='1.25',[3]=3,[6]=99}", error));
        lua_getglobal(state, "Vector");
        lua_newtable(state);
        lua_pushcfunction(state, vector_index);
        lua_setfield(state, -2, "__index");
        lua_setmetatable(state, -2);
        lua_pop(state, 1);
        const auto globals = lua.globals();
        const auto vector = lua.get_by_name(globals, "Vector");
        // The operation's tracking offset is derived from the actual stack,
        // even when the interpreter has more than 50 live outer entries.
        require(lua_checkstack(state, 80) != 0);
        for (int i = 0; i != 70; ++i) lua_pushinteger(state, i);
        const int top = lua_gettop(state);
        const std::array<float, 4> fallback{9, 8, 7, 6};
        Call call{&lua, vector, &fallback};
        lua_pushlightuserdata(state, &call);
        lua_pushcclosure(state, protected_convert, 1);
        require(lua_pcall(state, 0, 0, 0) == LUA_ERRRUN);
        lua_pop(state, 1); // enclosing Lua protection owns the error value
        require(lua_gettop(state) == top);
        require(lua.execute_archive("Fail=false; Visits=0", error));
        const auto result = lua.to_vector4_or(vector, fallback);
        require((result == std::array<float, 4>{1.25f, 0, 3, 0}));
        require(lua_gettop(state) == top);
        const auto visits = lua.get_by_name(globals, "Visits");
        require(lua.to_number(visits) == 524); // fifth gate, then missing 2 and 4
        lua.release(visits);
        require(lua.execute_archive("Vector[5]=false; Visits=0", error));
        require(lua.to_vector4_or(vector, fallback) == fallback);
        require(lua_gettop(state) == top);
        lua_settop(state, 0);
        lua.release(vector);
        lua.release(globals);
    }
    lua_close(state);
    std::cout << "PASS: native vector4 sparse reads/fifth gate, error recovery and stack isolation\n";
}
