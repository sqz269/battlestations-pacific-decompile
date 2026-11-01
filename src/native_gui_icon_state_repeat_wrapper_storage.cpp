#include "bsp/native_gui_icon_state_repeat_wrapper_storage.hpp"
#include "bsp/native_gui_icon_state_repeat_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This normal raw wrapper requires MSVC Win32 DWORD storage contracts.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));
std::uint32_t repeat_native_gui_icon_states_wrapper_storage_00ab56c0(
    std::uint32_t actual_entry_ecx_bits,
    const volatile std::uint32_t (&actual_outer_stack_homes)[3],
    std::uint32_t& actual_tag_home,
    volatile std::uint32_t (&actual_callee_stack_homes)[4],
    NativeGuiIconStateRepeatStorageFrame& frame,
    NativeGuiIconStorageContext& context) {
    const std::uint32_t captured_entry_ecx = actual_entry_ecx_bits;
    volatile std::uint32_t& tag = actual_tag_home;
    tag = captured_entry_ecx;
    const std::uint32_t first_prototype = actual_outer_stack_homes[2];
    const std::uint32_t count = actual_outer_stack_homes[1];
    const std::uint32_t destination = actual_outer_stack_homes[0];
    *reinterpret_cast<volatile std::uint8_t*>(&actual_tag_home) = 0;
    const std::uint32_t masked_tag = tag;
    actual_callee_stack_homes[3] = masked_tag;
    const std::uint32_t second_prototype = actual_outer_stack_homes[2];
    actual_callee_stack_homes[2] = first_prototype;
    actual_callee_stack_homes[1] = captured_entry_ecx;
    actual_callee_stack_homes[0] = second_prototype;
    repeat_construct_native_gui_icon_states_storage_00ab5400(
        reinterpret_cast<void*>(destination), count, actual_callee_stack_homes, frame, context);
    return destination + (count << 6u);
}
} // namespace bsp
