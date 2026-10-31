#include "bsp/native_mission_lua_caller_info.hpp"

extern "C" {
#include <lua.h>
}

#include <climits>
#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <type_traits>

namespace bsp {
namespace {

static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4 && sizeof(int) == 4);
static_assert(std::is_same_v<int, std::int32_t>);
static_assert(std::is_standard_layout_v<lua_Debug>);
static_assert(sizeof(lua_Debug) == 0x64 && alignof(lua_Debug) == 4);
static_assert(offsetof(lua_Debug, short_src) == 0x24);
static_assert(sizeof(lua_Debug::short_src) == 60);
static_assert(offsetof(lua_Debug, i_ci) == 0x60);
using LinkedGetStack = int (__cdecl*)(lua_State*, int, lua_Debug*);
using LinkedGetInfo = int (__cdecl*)(lua_State*, const char*, lua_Debug*);
static_assert(std::is_same_v<decltype(&lua_getstack), LinkedGetStack>);
static_assert(std::is_same_v<decltype(&lua_getinfo), LinkedGetInfo>);

lua_State* current_state(const void* actual_machine) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits,
        static_cast<const unsigned char*>(actual_machine) + 4u, sizeof bits);
    _ReadWriteBarrier();
    return reinterpret_cast<lua_State*>(static_cast<std::uintptr_t>(bits));
}

} // namespace

std::int32_t get_caller_info_native_mission_lua_006b86b0(
    const void* actual_machine, std::int32_t level,
    lua_Debug* actual_source_record, const char* actual_nSlu_00cf8398) {
    const std::int32_t captured_level = level;
    lua_Debug* const captured_record = actual_source_record;
    const void* const captured_machine = actual_machine;
    lua_State* const first_state = current_state(captured_machine);
    const int found = lua_getstack(first_state, captured_level, captured_record);
    if (found == 0) return found;
    lua_State* const second_state = current_state(captured_machine);
    return lua_getinfo(second_state, actual_nSlu_00cf8398, captured_record);
}

} // namespace bsp
