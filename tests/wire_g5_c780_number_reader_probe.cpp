// Focused linked-Lua ownership probe for 00700330. Build invocation is recorded
// in reports/wire-g5-c780.json; the probe links the existing Win32 Release libs.
#include "bsp/dummy_depth_seg34_argon.hpp"
#include "bsp/native_lua_objects.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <float.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

static void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}

static int row_index(lua_State* state) {
    const char* const key = lua_tostring(state, 2);
    if (key && std::strcmp(key, "meta") == 0) lua_pushliteral(state, "6.25");
    else lua_pushnil(state);
    return 1;
}

int main() {
    unsigned int saved_control;
    require(_controlfp_s(&saved_control, _PC_24, _MCW_PC) == 0, "PC24 setup");
    lua_State* const state = luaL_newstate();
    require(state != nullptr, "Lua allocation");
    lua_newtable(state);
    lua_pushliteral(state, "12.5");
    lua_setfield(state, -2, "number");
    lua_newtable(state);
    lua_pushcfunction(state, row_index);
    lua_setfield(state, -2, "__index");
    lua_setmetatable(state, -2);
    lua_setglobal(state, "ProbeRow");
    require(lua_checkstack(state, 90) != 0, "deep stack capacity");
    for (int i = 0; i < 80; ++i) lua_pushinteger(state, i);
    const int baseline = lua_gettop(state);

    bsp::NativeLuaStateStorage owner;
    bsp::construct_native_lua_state_00b66bd0(&owner);
    owner.state_04 = state;
    // Match the caller's relative tracking domain while retaining the actual
    // interpreter stack indices. The deep prefix remains borrowed throughout.
    owner.stack_offset_0c = -(baseline + 1);
    bsp::NativeLuaObjectStorage globals, table, copy;
    bsp::native_lua_globals_00b67980(owner, &globals);
    bsp::native_lua_get_by_name_00b67800(globals, &table, "ProbeRow");
    const int table_index = lua_gettop(state);
    bsp::construct_native_lua_object_00b65f50(&copy);

    const char* keys[] = {"number", "meta", "missing"};
    const float expected[] = {12.5f, 6.25f, 0.0f};
    for (int i = 0; i < 3; ++i) {
        bsp::assign_native_lua_object_00b67690(copy, table);
        float result = -1.0f;
        bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_00700330(keys[i], &result, copy);
        require(result == expected[i], "GetNumber coercion / GetByName metamethod");
        require(copy.kind_04 == 0, "by-value copy was not consumed");
        require(lua_gettop(state) == table_index && table.kind_04 == 2,
            "reader removed the surviving row alias or leaked the field");
        require(owner.slots_14[0].count_14 == 1 && owner.slots_14[1].count_14 == 0,
            "field-before-copy tracking cleanup");
    }
    bsp::NativeLuaObjectStorage view{&owner, 2, table.index_08, 0, 0, {}};
    float result;
    bsp::dummy_depth_seg34_argon::LuaObject_ReadNumberField_00700330("number", &result, view);
    require(result == 12.5f && view.kind_04 == 0 && lua_gettop(state) == table_index &&
        owner.slots_14[0].count_14 == 1 && owner.slots_14[1].count_14 == 0,
        "untracked caller view removed or registered the borrowed row");
    bsp::destroy_native_lua_object_00b67700(table);
    bsp::destroy_native_lua_object_00b67700(globals);
    require(lua_gettop(state) == baseline && lua_tointeger(state, -1) == 79,
        "borrowed deep-stack prefix changed");
    require(owner.slots_14[0].count_14 == 0, "last row tracking reference leaked");
    lua_close(state);
    unsigned int discarded;
    _controlfp_s(&discarded, saved_control, _MCW_PC);
    std::puts("c780 numeric reader: coercion, metamethod, deep stack and owned-copy cleanup passed");
}
