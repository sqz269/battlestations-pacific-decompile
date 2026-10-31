#include "bsp/native_mission_lua_scalar_push.hpp"

extern "C" {
#include <lua.h>
}

#include <climits>
#include <cstring>
#include <intrin.h>
#include <limits>
#include <type_traits>

namespace bsp {
namespace {

static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(int) == 4);
static_assert(sizeof(std::uint8_t) == 1 && std::is_same_v<int, std::int32_t>);
static_assert(sizeof(float) == 4 && sizeof(double) == 8);
static_assert(std::is_same_v<lua_Number, double>);
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::numeric_limits<double>::is_iec559);
using LinkedPushNumber = void (__cdecl*)(lua_State*, lua_Number);
static_assert(std::is_same_v<decltype(&lua_pushnumber), LinkedPushNumber>);

lua_State* current_state(const void* actual_machine) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits,
        static_cast<const unsigned char*>(actual_machine) + 4u, sizeof bits);
    _ReadWriteBarrier();
    return reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(bits));
}

} // namespace

void push_boolean_native_mission_lua_006b80c0(
    const void* actual_machine, std::uint8_t value) {
    lua_State* const state = current_state(actual_machine);
    const int normalized = value != std::uint8_t{0} ? 1 : 0;
    lua_pushboolean(state, normalized);
}

__declspec(noinline) void push_number_native_mission_lua_006b80e0(
    const void* actual_machine, const void* actual_float32_home) {
    _ReadWriteBarrier();
    __asm {
        mov eax, actual_float32_home
        fld dword ptr [eax]
        mov eax, actual_machine
        mov ecx, dword ptr [eax + 4]
        sub esp, 8
        fstp qword ptr [esp]
        push ecx
        call lua_pushnumber
        add esp, 12
    }
    _ReadWriteBarrier();
}

void push_string_native_mission_lua_006b8120(
    const void* actual_machine, const char* text) {
    const char* const captured_text = text;
    lua_State* const state = current_state(actual_machine);
    lua_pushstring(state, captured_text);
}

} // namespace bsp
