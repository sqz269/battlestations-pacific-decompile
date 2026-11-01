#include "bsp/native_gui_icon_state_fill_storage.hpp"
#include "bsp/native_gui_icon_state_assignment_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source fill supplier requires MSVC Win32 address DWORDs.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));
void fill_native_gui_icon_state_range_storage_00ab4af0(
    const volatile std::uint32_t (&actual_argument_homes)[3],
    std::uint32_t& actual_assignment_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    std::uint32_t current = actual_argument_homes[0];
    const std::uint32_t end = actual_argument_homes[1];
    if (current != end) {
        const std::uint32_t source = actual_argument_homes[2];
        do {
            (void)assign_native_gui_icon_state_storage_00ab3c10(
                reinterpret_cast<void*>(current), reinterpret_cast<const void*>(source),
                actual_assignment_preserve_home, strings, actual_feature_word_0109eea4);
            current += 0x40u;
        } while (current != end);
    }
}
} // namespace bsp
