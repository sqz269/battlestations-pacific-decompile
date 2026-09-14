#include "bsp/native_lua_string_field.hpp"

#include <cstddef>
#include <cstdint>
#include <intrin.h>

extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua string field requires MSVC Win32.
#endif

namespace bsp {
namespace {

// Each public slot is the ORIGINAL stack word, not an entry-time copy.
// Volatile field accesses retain the native reload order across Lua calls.
__declspec(noinline) void __cdecl set_string_field_body(
    NativeLuaObjectStorage* actual_object,
    const char* actual_empty_0108ff2c,
    const void* const volatile* key_public_slot,
    const void* const volatile* value_public_slot) {
    static_assert(sizeof(void*) == 4);
    static_assert(sizeof(NativeString) == 8);

    auto* owner = static_cast<volatile NativeLuaObjectStorage&>(*actual_object).owner_00;
    auto* state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    (void)lua_checkstack(state, 2);

    const void* key_header = *key_public_slot;
    const auto* key_bytes = static_cast<const std::byte*>(key_header);
    const auto key_length = *reinterpret_cast<const volatile std::uint32_t*>(key_bytes);
    const char* key_data = *reinterpret_cast<const char* const volatile*>(key_bytes + 4);
    if (key_data == nullptr) key_data = actual_empty_0108ff2c;
    _ReadWriteBarrier(); // Keep the owner reload after the native fallback.
    owner = static_cast<volatile NativeLuaObjectStorage&>(*actual_object).owner_00;
    state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_pushlstring(state, key_data, key_length);

    const void* value_header = *value_public_slot;
    const auto* value_bytes = static_cast<const std::byte*>(value_header);
    const auto value_length = *reinterpret_cast<const volatile std::uint32_t*>(value_bytes);
    const char* value_data = *reinterpret_cast<const char* const volatile*>(value_bytes + 4);
    if (value_data == nullptr) value_data = actual_empty_0108ff2c;
    _ReadWriteBarrier(); // The second push uses the post-header owner.
    owner = static_cast<volatile NativeLuaObjectStorage&>(*actual_object).owner_00;
    state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_pushlstring(state, value_data, value_length);

    owner = static_cast<volatile NativeLuaObjectStorage&>(*actual_object).owner_00;
    const auto index = static_cast<volatile NativeLuaObjectStorage&>(*actual_object).index_08;
    state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_settable(state, index);
}

} // namespace

__declspec(naked) void __fastcall set_native_lua_string_field_00b674c0(
    NativeLuaObjectStorage*, const char*, const void*, const void*) {
    __asm {
        lea eax, [esp + 8]  // original value public slot
        push eax
        lea eax, [esp + 8]  // original key public slot after first push
        push eax
        push edx             // borrowed actual 0108FF2C address
        push ecx             // stable actual Lua object
        call set_string_field_body
        add esp, 16
        ret 8
    }
}

} // namespace bsp
