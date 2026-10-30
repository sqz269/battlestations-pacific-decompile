#include "bsp/native_mission_lua_type.hpp"

extern "C" {
#include <lua.h>
}

#include <climits>
#include <cstring>
#include <intrin.h>
#include <type_traits>

namespace bsp {

static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(int) == 4);
static_assert(std::is_same_v<int, std::int32_t>);

std::int32_t type_native_mission_lua_006b7ec0(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    std::uint32_t state_bits;
    _ReadWriteBarrier();
    std::memcpy(&state_bits,
        static_cast<const unsigned char*>(actual_machine) + 4u, sizeof state_bits);
    _ReadWriteBarrier();
    auto* const state = reinterpret_cast<lua_State*>(
        static_cast<std::uintptr_t>(state_bits));
    return lua_type(state, captured_index);
}

} // namespace bsp
