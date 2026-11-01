#include "bsp/native_gui_icon_state_range_copy_wrapper_storage.hpp"
#include "bsp/native_gui_icon_state_range_construction_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This normal raw wrapper requires MSVC Win32 DWORD address contracts.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));
std::uint32_t copy_native_gui_icon_state_range_wrapper_storage_00ab58a0(
    std::uint32_t actual_entry_ecx_bits,
    const volatile std::uint32_t (&actual_outer_stack_homes)[3],
    std::uint32_t& actual_tag_home,
    volatile std::uint32_t (&actual_callee_stack_homes)[4],
    NativeGuiIconStateRangeConstructionStorageFrame& frame,
    NativeGuiIconStorageContext& context) {
    const std::uint32_t captured_entry_ecx = actual_entry_ecx_bits;
    volatile std::uint32_t& tag = actual_tag_home;
    tag = captured_entry_ecx;
    const std::uint32_t first_destination = actual_outer_stack_homes[2];
    *reinterpret_cast<volatile std::uint8_t*>(&actual_tag_home) = 0;
    const std::uint32_t masked_tag = tag;
    actual_callee_stack_homes[3] = masked_tag;
    const std::uint32_t second_destination = actual_outer_stack_homes[2];
    actual_callee_stack_homes[2] = first_destination;
    const std::uint32_t source_end = actual_outer_stack_homes[1];
    actual_callee_stack_homes[1] = captured_entry_ecx;
    const std::uint32_t source_begin = actual_outer_stack_homes[0];
    actual_callee_stack_homes[0] = second_destination;
    return construct_native_gui_icon_state_range_storage_00ab4c50(
        reinterpret_cast<const void*>(source_begin), reinterpret_cast<const void*>(source_end),
        actual_callee_stack_homes, frame, context);
}
} // namespace bsp
