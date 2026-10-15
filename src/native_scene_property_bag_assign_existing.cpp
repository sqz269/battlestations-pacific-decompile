#include "bsp/native_scene_property_bag_assign_existing.hpp"

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

__forceinline std::uint32_t load_word(const void* actual,
    std::uint32_t byte_offset) noexcept {
    std::uint32_t bits;
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov bits, eax
    }
    return bits;
}

__forceinline void* load_pointer(const void* actual,
    std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(
        load_word(actual, byte_offset)));
}
} // namespace

// Whole Original [008F23E0,008F253F). Keep mapped capture/query retirement,
// destination-tag observation and conditional source loads in that order.
__declspec(noinline) void assign_existing_native_scene_property_bag_008f23e0(
    void* actual_destination_bag, const void* actual_source_bag,
    NativeScenePropertyInsertionContext& context) {
    auto* const iterator = static_cast<std::uint8_t*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, 0x0c}));
    cc13_seg7::native_at<const void*>(iterator, 0) =
        static_cast<const std::byte*>(actual_source_bag) + 4;
    cc13_seg7::property_bag_iterator_begin_00480690(iterator);
    auto& keys = context.lifetimes().keys();

    while (load_pointer(iterator, 4) != nullptr) {
        const char* const key = cc13_seg7::property_bag_iterator_key_00484d20(iterator);
        alignas(std::uint32_t) std::byte query[8];
        construct_native_string_header_0041e870(query, keys, key);
        std::uint32_t bucket;
        void* const node = find_native_property_map_node_0043b8b0(
            static_cast<std::byte*>(actual_destination_bag) + 4, query, bucket,
            context.property_empty(), context.query_empty());
        const bool found = node != nullptr;
        void* const mapped_record = found ? load_pointer(node, 8) : nullptr;
        destroy_native_string_header_0041dd20(query, keys);

        if (found) {
            const auto destination_tag = load_word(mapped_record, 4);
            if (destination_tag == 6) {
                const void* const source_node = load_pointer(iterator, 4);
                const void* const source_record = load_pointer(source_node, 8);
                const void* const source_child = load_pointer(source_record, 0x0c);
                void* const destination_child = load_pointer(mapped_record, 0x0c);
                assign_existing_native_scene_property_bag_008f23e0(
                    destination_child, source_child, context);
            } else {
                const void* const source_node = load_pointer(iterator, 4);
                const void* const source_record = load_pointer(source_node, 8);
                assign_native_scene_property_record_008f0700(mapped_record, source_record);
            }
        }
        cc13_seg7::property_bag_iterator_next_0047e480(iterator);
    }
    singleton_lifetime_free(iterator);
}

} // namespace bsp
