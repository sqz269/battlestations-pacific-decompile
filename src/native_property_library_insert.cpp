#include "bsp/native_property_library_insert.hpp"

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_enum_node_pool.hpp"
#include "bsp/native_property_library_lookup.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4);

// Physical operations retain Original publication and post-provider rereads.
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

__forceinline void increment_map_count(void* actual_map) noexcept {
    __asm {
        mov eax, actual_map
        add dword ptr [eax + 4], 1
    }
}
} // namespace

NativePropertyLibraryInsertContext::NativePropertyLibraryInsertContext(
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical)
    : strings_(actual_strings), lifetime_(actual_strings, actual_physical) {}

__declspec(noinline) std::uint32_t
insert_or_replace_native_property_library_entry_008f29a0(void* actual_map,
    const void* actual_query_header, void* incoming_bag,
    NativePropertyLibraryInsertContext& context) {
    std::uint32_t bucket;
    void* node = find_native_property_library_node_00468ed0(actual_map,
        actual_query_header, bucket, context.strings().library_empty_00e18560(),
        context.strings().query_empty_00e17654());
    if (node) {
        void* const old_bag = load_pointer(node, 8);
        if (old_bag) {
            scalar_delete_native_scene_property_bag_008f59e0(
                old_bag, 1, context.lifetime().bags());
            store_word(node, 8, 0);
        }
        store_word(node, 8, reinterpret_cast<std::uint32_t>(incoming_bag));
        return load_word(actual_map, 4);
    }

    node = allocate_native_enum_node_004e7c00(context.lifetime().library_pool());
    if (node) {
        store_word(node, 0, 0);
        store_word(node, 4, 0);
    }
    // Keep Original identity branch even though ordinary admission is disjoint.
    // Failed allocation continues through the native faulting path, not recovery.
    if (node != actual_query_header) {
        const std::uint32_t requested = load_word(actual_query_header, 0);
        resize_native_string_header_0041dd40(node,
            context.lifetime().bags().keys(), requested, true);
        if (load_word(actual_query_header, 0) != 0) {
            const std::uint32_t current_length = load_word(node, 0);
            const void* const source = load_pointer(actual_query_header, 4);
            void* const destination = load_pointer(node, 4);
            std::memcpy(destination, source, current_length);
        }
    }
    store_word(node, 8, reinterpret_cast<std::uint32_t>(incoming_bag));
    const std::uint32_t head_offset = 8u + bucket * 4u;
    void* const current_head = load_pointer(actual_map, head_offset);
    store_word(node, 0x0c, reinterpret_cast<std::uint32_t>(current_head));
    store_word(actual_map, head_offset, reinterpret_cast<std::uint32_t>(node));
    increment_map_count(actual_map);
    return load_word(actual_map, 4);
}

} // namespace bsp
