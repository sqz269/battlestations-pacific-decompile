#include "bsp/native_mission_lua_conversions.hpp"

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
static_assert(std::is_same_v<int, std::int32_t>);
static_assert(sizeof(bool) == 1 && sizeof(float) == 4);
static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::is_same_v<lua_Number, double> && sizeof(lua_Number) == 8);

lua_State* current_state(const void* machine) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits, static_cast<const unsigned char*>(machine) + 4u,
        sizeof bits);
    _ReadWriteBarrier();
    return reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(bits));
}

// Consume the genuine API's REAL x87 return before any C++ double snapshot.
// This repeats only the accepted narrow call-boundary pattern, not Lua code.
__declspec(noinline) float call_current_lua_number_float32(lua_State* state,
    std::int32_t index) {
    float narrowed;
    __asm {
        push index
        push state
        call lua_tonumber
        add esp, 8
        fstp narrowed
    }
    return narrowed; // emitted FLD must reload this same actual FLOAT32
}
} // namespace

bool to_boolean_native_mission_lua_006b8000(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return lua_toboolean(state, captured_index) != 0;
}

float to_float32_native_mission_lua_006b8020(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return call_current_lua_number_float32(state, captured_index);
}

void* to_userdata_native_mission_lua_006b80a0(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return lua_touserdata(state, captured_index);
}

} // namespace bsp
