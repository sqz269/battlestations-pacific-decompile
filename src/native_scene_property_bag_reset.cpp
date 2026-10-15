#include "bsp/native_scene_property_bag_reset.hpp"

#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The original property storage is Win32.");

__forceinline void* load_pointer(const void* actual, std::uint32_t byte_offset) noexcept {
    std::uint32_t bits;
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov bits, eax
    }
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

// Whole Original [008F5470,008F54E5). Original ECX is the OUTER bag;
// only the initial clear receives its actual map interior at +4.
__declspec(noinline) void reset_native_scene_property_bag_008f5470(
    void* actual_destination_bag, const void* optional_source_bag,
    NativeScenePropertyCloneContext& context) {
    clear_native_scene_property_record_map_008f3f30(
        static_cast<std::byte*>(actual_destination_bag) + 4,
        context.insertion().lifetimes());
    if (optional_source_bag == nullptr) return;

    auto* const iterator = static_cast<std::uint8_t*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, 0x0c}));
    cc13_seg7::native_at<const void*>(iterator, 0) =
        static_cast<const std::byte*>(optional_source_bag) + 4;
    cc13_seg7::property_bag_iterator_begin_00480690(iterator);
    for (;;) {
        void* const node = load_pointer(iterator, 4);
        if (node == nullptr) break;
        const void* const source_record = load_pointer(node, 8);
        void* const cloned_record = clone_native_scene_property_record_008f4f60(
            source_record, context);
        const char* const key = cc13_seg7::property_bag_iterator_key_00484d20(iterator);
        insert_native_scene_property_bag_008f33f0(actual_destination_bag, key,
            cloned_record, context.insertion());
        cc13_seg7::property_bag_iterator_next_0047e480(iterator);
    }
    singleton_lifetime_free(iterator);
}

} // namespace bsp
