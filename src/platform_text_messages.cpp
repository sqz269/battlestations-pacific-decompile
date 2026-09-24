#include "bsp/platform_text_messages.hpp"

#include <cstdint>

namespace bsp {

bool handle_platform_text_message_00bed3b0_fragment(PlatformTextInput& text,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) {
    static_assert(sizeof(WPARAM) == sizeof(std::uint32_t)); // target: MSVC Win32
    if (message != WM_KEYDOWN && message != WM_CHAR) return false;

    // 00BED607..67D and 00BED6DB..6FE use the explicit platform receiver's
    // +170 gate and append into its +174 queue. Preserve full WPARAM for key
    // comparison and the exact 16h clipboard request before low-byte storage.
    text.enqueue_message_00bed3b0_fragment(message,
        static_cast<std::uint32_t>(wparam));
    result = DefWindowProcA(window, message, wparam, lparam);
    return true;
}

} // namespace bsp
