#include "bsp/native_lua_field_setters.hpp"

extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

// Each native API call reloads the owner's current state through the actual
// object. Volatile lvalue reads keep that schedule visible across Lua hooks.
lua_State* current_state(NativeLuaObjectStorage& object) {
    auto* const owner = static_cast<volatile NativeLuaObjectStorage&>(object).owner_00;
    return static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
}

void set_current_table(NativeLuaObjectStorage& object) {
    auto* const owner = static_cast<volatile NativeLuaObjectStorage&>(object).owner_00;
    const auto index = static_cast<volatile NativeLuaObjectStorage&>(object).index_08;
    auto* const state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_settable(state, index);
}

} // namespace

void native_lua_set_lightuserdata_00b67530(
    NativeLuaObjectStorage& object, const NativeString& key, void* value) {
    (void)lua_checkstack(current_state(object), 2);

    // Native reads the length before the data pointer. A null data pointer
    // selects the global empty byte at 0108FF2C, even for a nonzero length.
    const auto length = key.length();
    const char* data = key.data();
    if (!data) data = "";
    lua_pushlstring(current_state(object), data, length);

    lua_pushlightuserdata(current_state(object), value);
    set_current_table(object);
}

void native_lua_set_new_table_00b67580(
    NativeLuaObjectStorage& object, const NativeString& key) {
    (void)lua_checkstack(current_state(object), 2);

    const auto length = key.length();
    const char* data = key.data();
    if (!data) data = "";
    lua_pushlstring(current_state(object), data, length);

    lua_createtable(current_state(object), 0, 0);
    set_current_table(object);
}

} // namespace bsp
