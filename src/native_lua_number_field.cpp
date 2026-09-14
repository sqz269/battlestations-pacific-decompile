#include "bsp/native_lua_number_field.hpp"

extern "C" {
#include <lua.h>
}

namespace bsp {

void native_lua_set_number_field_00b67400(NativeLuaObjectStorage& object,
    const NativeString& key, float value) {
    static_assert(sizeof(NativeString) == 8);
    auto& live = static_cast<volatile NativeLuaObjectStorage&>(object);
    auto* owner = live.owner_00;
    auto* state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    (void)lua_checkstack(state, 2);

    // NativeString's established actual eight-byte header; these are current
    // lvalue reads, not another string owner or an allocation/copy operation.
    const auto* header = reinterpret_cast<const unsigned char*>(&key);
    const auto length = *reinterpret_cast<const volatile std::uint32_t*>(header);
    const auto* data = *reinterpret_cast<const char* const volatile*>(header + 4);
    if (data == nullptr) data = ""; // native0108FF2C, length remains unchanged
    owner = live.owner_00;
    state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_pushlstring(state, data, length);

    // Preserve FLD before the current owner/state reads, and FSTP after them.
    // A C++ double cast can choose SSE and has different denormal/NaN status.
    double number;
    lua_State* number_state;
    auto* actual_object = &object;
    __asm {
        fld value
        mov eax, actual_object
        mov edx, dword ptr [eax]
        mov ecx, dword ptr [edx + 4]
        mov number_state, ecx
        fstp number
    }
    lua_pushnumber(number_state, number);

    owner = live.owner_00;
    const auto index = live.index_08;
    state = static_cast<volatile NativeLuaStateStorage&>(*owner).state_04;
    lua_settable(state, index);
}

} // namespace bsp
