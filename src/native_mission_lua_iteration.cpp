#include "bsp/native_mission_lua_iteration.hpp"

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

void push_nil_native_mission_lua_006b8130(const void* actual_machine) {
    lua_State* const state = current_state(actual_machine);
    lua_pushnil(state);
}

std::int32_t next_native_mission_lua_006b85b0(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    lua_State* const state = current_state(actual_machine);
    return lua_next(state, captured_index);
}

void pop_native_mission_lua_006b85f0(
    const void* actual_machine, std::int32_t count) {
    lua_State* const state = current_state(actual_machine);
    const std::uint32_t count_bits = static_cast<std::uint32_t>(count);
    const std::uint32_t index_bits = std::uint32_t{0xffffffffu} - count_bits;
    std::int32_t target_index;
    std::memcpy(&target_index, &index_bits, sizeof target_index);
    lua_settop(state, target_index);
}

} // namespace bsp
