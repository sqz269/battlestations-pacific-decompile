#include "bsp/native_lua_field_setters.hpp"

extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

struct CurrentKey {
    std::uint32_t length;
    const char* data;
};

CurrentKey current_key(const NativeString& key) {
    static_assert(sizeof(NativeString) == 8);
    // The established actual header is {DWORD length, char* data}. Volatile
    // reads enforce the native load order even with MSVC optimization.
    const auto* header = reinterpret_cast<const unsigned char*>(&key);
    const auto length = *reinterpret_cast<const volatile std::uint32_t*>(header);
    const auto* data = *reinterpret_cast<const char* const volatile*>(header + 4);
    if (!data) data = ""; // original empty byte at 0108FF2C
    return {length, data};
}

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

    const auto current = current_key(key);
    lua_pushlstring(current_state(object), current.data, current.length);

    lua_pushlightuserdata(current_state(object), value);
    set_current_table(object);
}

void native_lua_set_new_table_00b67580(
    NativeLuaObjectStorage& object, const NativeString& key) {
    (void)lua_checkstack(current_state(object), 2);

    const auto current = current_key(key);
    lua_pushlstring(current_state(object), current.data, current.length);

    lua_createtable(current_state(object), 0, 0);
    set_current_table(object);
}

} // namespace bsp
