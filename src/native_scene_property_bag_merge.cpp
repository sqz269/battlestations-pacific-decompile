#include "bsp/native_scene_property_bag_merge.hpp"

#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_scene_property_record_assignment.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The original property storage is Win32.");

__forceinline std::uint32_t load_word(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void store_word(void* root, std::size_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

__forceinline void* pointer(std::uint32_t word) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(word));
}
} // namespace

__declspec(noinline) void merge_native_scene_property_bag_008f54f0(
    void* actual_destination_bag, const void* actual_source_bag,
    std::uint32_t keep_existing, NativeScenePropertyCloneContext& context) {
    auto& insertion = context.insertion();
    auto& keys = insertion.lifetimes().keys();
    auto* iterator = static_cast<cc13_seg7::Object>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 12, 12}));
    if (iterator != nullptr) {
        const auto source_map = reinterpret_cast<std::uintptr_t>(actual_source_bag) + 4u;
        store_word(iterator, 0, static_cast<std::uint32_t>(source_map));
        cc13_seg7::property_bag_iterator_begin_00480690(iterator);
    }

    while (load_word(iterator, 4) != 0) {
        const char* const key = cc13_seg7::property_bag_iterator_key_00484d20(iterator);
        alignas(std::uint32_t) unsigned char query[8];
        construct_native_string_header_0041e870(query, keys, key);
        std::uint32_t bucket;
        void* const node = find_native_property_map_node_0043b8b0(
            static_cast<unsigned char*>(actual_destination_bag) + 4,
            query, bucket, insertion.property_empty(), insertion.query_empty());
        const bool hit = node != nullptr;
        void* mapped = nullptr;
        if (hit)
            mapped = pointer(load_word(node, 8));

        // The hit record is already captured; return the query before branching.
        destroy_native_string_header_0041dd20(query, keys);
        if (hit) {
            if (load_word(mapped, 4) == 6) {
                void* const current_node = pointer(load_word(iterator, 4));
                void* const current_source = pointer(load_word(current_node, 8));
                void* const source_child = pointer(load_word(current_source, 0x0c));
                void* const destination_child = pointer(load_word(mapped, 0x0c));
                merge_native_scene_property_bag_008f54f0(
                    destination_child, source_child, keep_existing, context);
            } else if (static_cast<std::uint8_t>(keep_existing) == 0) {
                void* const current_node = pointer(load_word(iterator, 4));
                void* const current_source = pointer(load_word(current_node, 8));
                assign_native_scene_property_record_008f0700(mapped, current_source);
            }
        } else {
            void* const current_node = pointer(load_word(iterator, 4));
            void* const current_source = pointer(load_word(current_node, 8));
            void* const cloned = clone_native_scene_property_record_008f4f60(current_source, context);

            // Original5622..562E rereads after clone, even though clone copied it.
            void* const reread_node = pointer(load_word(iterator, 4));
            void* const reread_source = pointer(load_word(reread_node, 8));
            store_word(cloned, 0x34, load_word(reread_source, 0x34));
            const char* const insertion_key = cc13_seg7::property_bag_iterator_key_00484d20(iterator);
            insert_native_scene_property_bag_008f33f0(
                actual_destination_bag, insertion_key, cloned, insertion);
        }
        cc13_seg7::property_bag_iterator_next_0047e480(iterator);
    }
    singleton_lifetime_free(iterator);
}

} // namespace bsp
