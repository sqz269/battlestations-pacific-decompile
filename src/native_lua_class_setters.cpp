#include "bsp/native_lua_class_setters.hpp"
extern "C" {
#include <lua.h>
}
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua class setters require MSVC Win32.
#endif

namespace bsp {
namespace {
__forceinline NativeLuaStateStorage* owner_of(NativeLuaObjectStorage& target) {
    return static_cast<volatile NativeLuaObjectStorage&>(target).owner_00;
}
__forceinline lua_State* state_of(NativeLuaStateStorage* owner) {
    return static_cast<volatile NativeLuaStateStorage*>(owner)->state_04;
}
__forceinline std::int32_t index_of(const NativeLuaObjectStorage& object) {
    return static_cast<const volatile NativeLuaObjectStorage&>(object).index_08;
}
__forceinline std::uint32_t key_length(const NativeString& key) {
    return *reinterpret_cast<const volatile std::uint32_t*>(&key);
}
__forceinline const char* key_data(const NativeString& key) {
    return *reinterpret_cast<const char* const volatile*>(
        reinterpret_cast<const std::byte*>(&key) + 4);
}
} // namespace
void native_lua_set_integer_00b67460(NativeLuaObjectStorage& target,
    const NativeString& key, std::int32_t value) {
    auto* owner = owner_of(target);
    auto* state = state_of(owner);
    (void)lua_checkstack(state, 2);
    const auto length = key_length(key);
    const char* const data = key_data(key);
    const char* const text = data ? data : "";
    owner = owner_of(target);
    state = state_of(owner);
    lua_pushlstring(state, text, length);
    double number;
    // 00B67491 FILD signed DWORD; 00B6749D FSTP binary64. Every int32 is
    // exactly representable; keep the original x87 conversion explicit.
    __asm { fild value }
    owner = owner_of(target);
    state = state_of(owner);
    __asm { fstp number }
    lua_pushnumber(state, number);
    owner = owner_of(target);
    const auto index = index_of(target);
    state = state_of(owner);
    lua_settable(state, index);
}

void native_lua_set_cstring_00b66790(NativeLuaObjectStorage& target,
    const char* key, const char* value) {
    auto* owner = owner_of(target);
    auto* state = state_of(owner);
    (void)lua_checkstack(state, 2);
    owner = owner_of(target);
    state = state_of(owner);
    lua_pushstring(state, key);
    owner = owner_of(target);
    state = state_of(owner);
    lua_pushstring(state, value);
    owner = owner_of(target);
    const auto index = index_of(target);
    state = state_of(owner);
    lua_settable(state, index);
}

void native_lua_set_object_00b675d0(NativeLuaObjectStorage& target,
    const NativeString& key, const NativeLuaObjectStorage& value) {
    auto* owner = owner_of(target);
    auto* state = state_of(owner);
    (void)lua_checkstack(state, 2);
    const auto length = key_length(key);
    const char* const data = key_data(key);
    const char* const text = data ? data : "";
    owner = owner_of(target);
    state = state_of(owner);
    lua_pushlstring(state, text, length);
    owner = owner_of(target);
    const auto value_index = index_of(value);
    state = state_of(owner);
    lua_pushvalue(state, value_index);
    owner = owner_of(target);
    const auto index = index_of(target);
    state = state_of(owner);
    lua_settable(state, index);
}
} // namespace bsp
