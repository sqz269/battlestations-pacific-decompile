#include "bsp/native_mission_lua_predicates.hpp"
#include "bsp/native_mission_lua_type.hpp"

namespace bsp {

std::int32_t is_nil_native_mission_lua_006b7ee0(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    const std::int32_t actual_type =
        type_native_mission_lua_006b7ec0(actual_machine, captured_index);
    return actual_type == 0 ? std::int32_t{1} : std::int32_t{0};
}

std::int32_t is_table_native_mission_lua_006b7f00(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    const std::int32_t actual_type =
        type_native_mission_lua_006b7ec0(actual_machine, captured_index);
    return actual_type == 5 ? std::int32_t{1} : std::int32_t{0};
}

std::int32_t is_string_native_mission_lua_006b7f60(
    const void* actual_machine, std::int32_t index) {
    const std::int32_t captured_index = index;
    const std::int32_t actual_type =
        type_native_mission_lua_006b7ec0(actual_machine, captured_index);
    return actual_type == 4 ? std::int32_t{1} : std::int32_t{0};
}

} // namespace bsp
