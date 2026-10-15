#include "bsp/native_scene_name_node_holder.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

void release_native_enum_symbol_node_holder_008f3d80(void* actual_holder_slot,
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical) {
    auto* const holder = static_cast<volatile std::uint32_t*>(actual_holder_slot);
    const std::uint32_t captured_node = *holder;
    if (captured_node == 0) {
        return;
    }
    void* const node = reinterpret_cast<void*>(captured_node);
    destroy_native_string_header_0041dd20(node, actual_strings.raw_context());
    void* const pool = actual_physical.enum_symbol_pool_storage_00e17578();
    return_native_enum_node_0043b0a0(pool, node);
    *holder = 0;
}

void release_native_property_library_node_holder_008f3dc0(void* actual_holder_slot,
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical) {
    auto* const holder = static_cast<volatile std::uint32_t*>(actual_holder_slot);
    const std::uint32_t captured_node = *holder;
    if (captured_node == 0) {
        return;
    }
    void* const node = reinterpret_cast<void*>(captured_node);
    destroy_native_string_header_0041dd20(node, actual_strings.raw_context());
    void* const pool = actual_physical.property_library_node_pool_storage_00e17540();
    return_native_enum_node_0043b0a0(pool, node);
    *holder = 0;
}
} // namespace bsp
