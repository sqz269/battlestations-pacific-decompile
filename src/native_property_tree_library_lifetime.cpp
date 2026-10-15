#include "bsp/native_property_tree_library_lifetime.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// Physical loads/stores retain the captured-next and post-retirement schedule.
__forceinline std::uint32_t load_word(const void* root,
    std::uint32_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void* load_pointer(const void* root,
    std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(load_word(root, byte_offset));
}

__forceinline void store_word(void* root, std::uint32_t byte_offset,
    std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
} // namespace

NativePropertyTreeLibraryLifetimeContext::NativePropertyTreeLibraryLifetimeContext(
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical)
    : bags_(actual_strings, actual_physical),
      library_pool_(actual_physical.property_library_node_pool_storage_00e17540()) {}

__declspec(noinline) void* clear_native_property_library_map_008f4c50(
    void* actual_map, NativePropertyTreeLibraryLifetimeContext& context) {
    for (std::uint32_t bucket = 0; bucket != 64; ++bucket) {
        const std::uint32_t head_offset = 8u + bucket * 4u;
        void* node = load_pointer(actual_map, head_offset);
        while (node) {
            void* const mapped = load_pointer(node, 8);
            void* const next = load_pointer(node, 0x0c);
            if (mapped) {
                scalar_delete_native_scene_property_bag_008f59e0(
                    mapped, 1, context.bags());
                store_word(node, 8, 0);
            }
            destroy_native_string_header_0041dd20(node, context.bags().keys());
            return_native_enum_node_0043b0a0(context.library_pool(), node);
            node = next;
        }
        store_word(actual_map, head_offset, 0);
    }
    store_word(actual_map, 4, 0);
    return actual_map;
}

__declspec(noinline) void destroy_native_property_tree_library_008f56a0(
    void* actual_library, NativePropertyTreeLibraryLifetimeContext& context) {
    store_word(actual_library, 0, 0x00d1650cu);
    void* const map = static_cast<std::uint8_t*>(actual_library) + 4;
    clear_native_property_library_map_008f4c50(map, context);
    store_word(map, 0, 0x00d162c8u);
    clear_native_property_library_map_008f4c50(map, context);
}

} // namespace bsp
