// Focused 004D32A0 fixture on the linked Lua 5.1.1 interpreter. This does not
// run bsp_game or establish game/runtime/ABI parity. Link /MANIFEST:EMBED.
#include "bsp/cc13_sweepA3.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_string.hpp"
#include <iostream>
#include <stdexcept>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

namespace {
void require(bool value) {
    if (!value) throw std::runtime_error("c722 script-global cleanup failed");
}
int function(lua_State*) { return 0; }
int kind(lua_State* state, const char* name) {
    lua_getfield(state, LUA_GLOBALSINDEX, name);
    const int result = lua_type(state, -1);
    lua_pop(state, 1);
    return result;
}
}

int main() {
    lua_State* const state = luaL_newstate();
    require(state != nullptr);
    bsp::NativeLuaStateStorage owner;
    bsp::construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    std::vector<std::string> baseline;
    // The real pre-script state supplies the baseline, rather than names
    // invented for cleanup. This is the same function-valued global census.
    lua_pushcfunction(state, function);
    lua_setfield(state, LUA_GLOBALSINDEX, "Baseline");
    lua_pushcfunction(state, function);
    lua_setfield(state, LUA_GLOBALSINDEX, "Persistent");
    lua_pushnil(state);
    while (lua_next(state, LUA_GLOBALSINDEX) != 0) {
        if (lua_isfunction(state, -1)) baseline.emplace_back(lua_tostring(state, -2));
        lua_pop(state, 1);
    }
    require(luaL_dostring(state,
        "baseline = function() end; Added = function() end; "
        "Persistent = 7; Data = { value = 9 }") == 0);
    // ExecuteString failure is ignored by the original. A non-identifier key
    // therefore survives its invalid assignment and leaves Lua's error value.
    lua_pushcfunction(state, function);
    lua_setfield(state, LUA_GLOBALSINDEX, "invalid-key");
    const int top = lua_gettop(state);
    bsp::cc13_sweepA3::game_nil_script_added_globals_004d32a0(
        baseline, owner, bsp::crt_string_storage());
    require(baseline.empty() && lua_gettop(state) == top + 1);
    require(kind(state, "Baseline") == LUA_TFUNCTION
        && kind(state, "baseline") == LUA_TFUNCTION
        && kind(state, "Added") == LUA_TNIL
        && kind(state, "Persistent") == LUA_TNUMBER
        && kind(state, "Data") == LUA_TTABLE
        && kind(state, "invalid-key") == LUA_TFUNCTION);
    lua_settop(state, top);
    lua_pushcfunction(state, function);
    lua_setfield(state, LUA_GLOBALSINDEX, "AfterCleanup");
    // The consumed/empty baseline must take the native early return.
    bsp::cc13_sweepA3::game_nil_script_added_globals_004d32a0(
        baseline, owner, bsp::crt_string_storage());
    require(kind(state, "AfterCleanup") == LUA_TFUNCTION && lua_gettop(state) == top);
    for (const auto& slot : owner.slots_14) require(slot.count_14 == 0);
    lua_close(state);
    std::cout << "c722 real Lua cleanup: baseline, case-folding, nonfunctions, "
        "ignored ExecuteString error, tracking and empty-set return passed\n";
}
