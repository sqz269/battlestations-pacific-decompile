#include "bsp/recon_values.hpp"

extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {
constexpr std::uint32_t table_scope_vtable = 0x00d08e5c;
}

void push_recon_global_table_006b8190(ReconLuaInstanceView instance,
    const char* key) {
    // Unlike the raw child constructors, every primitive reloads +04. Global
    // get/set can execute Lua metamethods and change the captured state slot.
    lua_pushstring(instance.state_04, key);
    lua_gettable(instance.state_04, LUA_GLOBALSINDEX);
    if (lua_type(instance.state_04, -1) == LUA_TNIL) {
        lua_settop(instance.state_04, -2);
        lua_pushstring(instance.state_04, key);
        lua_createtable(instance.state_04, 0, 0);
        lua_settable(instance.state_04, LUA_GLOBALSINDEX);
        lua_pushstring(instance.state_04, key);
        lua_gettable(instance.state_04, LUA_GLOBALSINDEX);
    }
}

void pop_recon_global_table_006b8210(ReconLuaInstanceView instance) {
    lua_settop(instance.state_04, -2);
}

ReconTableScope push_recon_index_table_00803750(ReconLuaInstanceView instance,
    std::int32_t key) {
    ReconTableScope scope{table_scope_vtable, instance};
    lua_State* const state = instance.state_04;
    (void)lua_checkstack(state, 2);
    lua_rawgeti(state, -1, key);
    if (lua_type(state, -1) == LUA_TNIL) {
        lua_settop(state, -2);
        lua_createtable(state, 0, 0);
        lua_rawseti(state, -2, key);
        lua_rawgeti(state, -1, key);
    }
    return scope;
}

ReconTableScope push_recon_named_table_008037d0(ReconLuaInstanceView instance,
    const char* key) {
    ReconTableScope scope{table_scope_vtable, instance};
    lua_State* const state = instance.state_04;
    (void)lua_checkstack(state, 2);
    lua_pushstring(state, key);
    lua_rawget(state, -2);
    if (lua_type(state, -1) == LUA_TNIL) {
        lua_settop(state, -2);
        lua_pushstring(state, key);
        lua_createtable(state, 0, 0);
        lua_rawset(state, -3);
        lua_pushstring(state, key);
        lua_rawget(state, -2);
    }
    return scope;
}

void pop_recon_table_scope(ReconTableScope& scope) {
    lua_State* const state = scope.instance_04.state_04;
    scope.vtable_00 = table_scope_vtable;
    lua_settop(state, -2);
}

void install_recon_category_tables_008039e0(ReconValuesContext& context) {
    // The table entries are not copied: a later iteration reads the live word.
    for (std::size_t index = 0; index != 19; ++index) {
        const auto instance = context.host.current_mission_lua_instance_1a08_04();
        const char* const key = context.category_names_00e0b590[index];
        auto scope = push_recon_named_table_008037d0(instance, key);
        pop_recon_table_scope(scope);
    }
}

void install_recon_values_00803a40(ReconValuesContext& context) {
    const auto instance = context.host.current_mission_lua_instance_1a08_04();
    push_recon_global_table_006b8190(instance, "recon");
    for (std::int32_t index = 0; index != 3; ++index) {
        auto indexed = push_recon_index_table_00803750(instance, index);
        for (const char* relation : {"enemy", "neutral", "unknown", "own"}) {
            auto related = push_recon_named_table_008037d0(instance, relation);
            install_recon_category_tables_008039e0(context);
            pop_recon_table_scope(related);
        }
        pop_recon_table_scope(indexed);
    }
    pop_recon_global_table_006b8210(instance);
}

} // namespace bsp
