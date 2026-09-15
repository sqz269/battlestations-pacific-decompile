#include "bsp/native_lua_object_boolean.hpp"

extern "C" {
#include <lua.h>
}

namespace bsp {
namespace {

__forceinline NativeLuaStateStorage* owner_of(NativeLuaObjectStorage& object) {
    return static_cast<volatile NativeLuaObjectStorage&>(object).owner_00;
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

void native_lua_set_boolean_00b673a0(
    NativeLuaObjectStorage& object, const NativeString& key, std::uint32_t value) {
    auto* owner = owner_of(object);
    auto* state = state_of(owner);
    (void)lua_checkstack(state, 2);

    const auto length = key_length(key);
    const char* const data = key_data(key);
    const char* const text = data ? data : "";
    owner = owner_of(object);
    state = state_of(owner);
    lua_pushlstring(state, text, length);

    owner = owner_of(object);
    const auto low_byte = static_cast<int>(static_cast<std::uint8_t>(value));
    state = state_of(owner);
    lua_pushboolean(state, low_byte);

    owner = owner_of(object);
    const auto index = index_of(object);
    state = state_of(owner);
    lua_settable(state, index);
}

} // namespace bsp
