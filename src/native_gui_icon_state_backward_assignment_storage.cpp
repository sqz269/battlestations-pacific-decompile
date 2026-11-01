#include "bsp/native_gui_icon_state_backward_assignment_storage.hpp"
#include "bsp/native_gui_icon_state_assignment_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source backward supplier requires MSVC Win32 DWORD/asm contracts.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

std::uint32_t assign_native_gui_icon_state_range_backward_storage_00ab5650(
    const volatile std::uint32_t (&actual_argument_homes)[3],
    std::uint32_t& actual_assignment_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const std::uint32_t captured_destination_end = actual_argument_homes[2];
    std::uint32_t source_cursor = actual_argument_homes[1];
    const std::uint32_t captured_source_begin = actual_argument_homes[0];
    std::uint32_t captured_result;
    __asm {
        mov eax, source_cursor
        sub eax, captured_source_begin
        sar eax, 6
        shl eax, 6
        mov edx, captured_destination_end
        sub edx, eax
        mov captured_result, edx
    }
    if (captured_source_begin != source_cursor) {
        const std::uint32_t destination_displacement = captured_destination_end - source_cursor;
        do {
            source_cursor -= 0x40u;
            const std::uint32_t destination = destination_displacement + source_cursor;
            (void)assign_native_gui_icon_state_storage_00ab3c10(
                reinterpret_cast<void*>(destination), reinterpret_cast<const void*>(source_cursor),
                actual_assignment_preserve_home, strings, actual_feature_word_0109eea4);
        } while (source_cursor != captured_source_begin);
    }
    return captured_result;
}
} // namespace bsp
