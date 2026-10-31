#include "bsp/native_mission_lua_to_string.hpp"

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
static_assert(sizeof(std::uint32_t) == 4 && sizeof(int) == 4);
static_assert(std::is_same_v<int, std::int32_t>);

lua_State* current_state(const void* actual_machine) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits,
        static_cast<const unsigned char*>(actual_machine) + 4u, sizeof bits);
    _ReadWriteBarrier();
    return reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(bits));
}

} // namespace

const char* to_string_native_mission_lua_006b8060(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return lua_tolstring(state, captured_index, nullptr);
}

} // namespace bsp
