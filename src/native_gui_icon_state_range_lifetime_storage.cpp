#include "bsp/native_gui_icon_state_range_lifetime_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source range supplier requires MSVC Win32 address DWORDs.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));

void destroy_native_gui_icon_state_range_storage_00ab57e0(
    const volatile std::uint32_t (&actual_argument_homes)[2],
    NativeGuiIconStorageContext& context) {
    std::uint32_t current = actual_argument_homes[0];
    const std::uint32_t end = actual_argument_homes[1];
    while (current != end) {
        destroy_native_gui_icon_state_00ab3280(reinterpret_cast<void*>(current), context);
        current += 0x40u;
    }
}
} // namespace bsp
