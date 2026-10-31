#include "bsp/native_mission_lua_integer.hpp"

#include "bsp/native_render_batch_keys.hpp"

extern "C" {
#include <lua.h>
}

#include <climits>
#include <cstring>
#include <intrin.h>
#include <type_traits>

namespace bsp {
namespace {
static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(std::int32_t) == 4);
static_assert(std::is_same_v<int, std::int32_t>);
static_assert(std::is_same_v<lua_Number, double> && sizeof(lua_Number) == 8);
static_assert(std::is_same_v<decltype(&lua_tonumber),
    lua_Number(__cdecl*)(lua_State*, int)>);

lua_State* current_state(const void* actual_machine) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits,
        static_cast<const unsigned char*>(actual_machine) + 4u, sizeof bits);
    _ReadWriteBarrier();
    return reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(bits));
}

// This is a call-boundary bridge, not a Lua or CRT body. Keep both genuine
// calls in one asm region: ST0 must reach BF7420 without an intervening helper,
// snapshot or conversion. BF7420 receives the actual word ADDRESS in ECX.
__declspec(noinline) std::uint32_t current_integer_bits(lua_State* state,
    std::int32_t index, const volatile std::uint32_t* conversion_word) {
    std::uint32_t result;
    __asm {
        push index
        push state
        call lua_tonumber
        add esp, 8
        mov ecx, conversion_word
        call native_crt_truncate_st0_00bf7420
        mov result, eax
    }
    return result;
}
} // namespace

std::uint32_t to_integer_bits_native_mission_lua_006b8040(
    const void* actual_machine, std::int32_t index,
    const volatile std::uint32_t& actual_conversion_word_0109eea4) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return current_integer_bits(state, captured_index,
        &actual_conversion_word_0109eea4);
}

} // namespace bsp
