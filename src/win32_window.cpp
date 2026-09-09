#include "bsp/win32_window.hpp"
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
int rectangle_extent(LONG high, LONG low) noexcept {
    const auto bits = static_cast<std::uint32_t>(high) - static_cast<std::uint32_t>(low);
    int result;
    static_assert(sizeof(result) == sizeof(bits));
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
bool set_style(HWND window, int index, LONG value, DWORD& error) noexcept {
    SetLastError(ERROR_SUCCESS);
    const auto previous = SetWindowLongA(window, index, value);
    error = GetLastError();
    return previous != 0 || error == ERROR_SUCCESS;
}
}

bool create_platform_window_00becee0_fragment(const PlatformWindowOptions& options,
    HWND& output, ATOM& registered_class, DWORD& error) noexcept {
    if (output || registered_class || !options.procedure) {
        error = ERROR_INVALID_PARAMETER;
        return false;
    }
    const char* name = options.name ? options.name : "";
    WNDCLASSA window_class{};
    window_class.style = CS_GLOBALCLASS;
    window_class.lpfnWndProc = options.procedure;
    window_class.cbWndExtra = 24;
    window_class.hInstance = options.instance;
    window_class.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(32512));
    window_class.lpszClassName = name;
    registered_class = RegisterClassA(&window_class);
    if (!registered_class) { error = GetLastError(); return false; }
    RECT rectangle{0, 0, options.width, options.height};
    if (!AdjustWindowRect(&rectangle, WS_CAPTION, FALSE)) {
        error = GetLastError(); return false;
    }
    const auto outer_width = rectangle_extent(rectangle.right, rectangle.left);
    const auto outer_height = rectangle_extent(rectangle.bottom, rectangle.top);
    output = CreateWindowExA(0, name, name, WS_CAPTION, options.x, options.y,
        outer_width, outer_height, nullptr, nullptr, options.instance, options.platform);
    if (!output) { error = GetLastError(); return false; }
    const LONG style = options.fullscreen ? static_cast<LONG>(WS_POPUP) : WS_CAPTION;
    const LONG extended_style = options.fullscreen ? WS_EX_TOPMOST : WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE;
    if (!set_style(output, GWL_STYLE, style, error)
        || !set_style(output, GWL_EXSTYLE, extended_style, error)) return false;
    if (!SetWindowPos(output, options.fullscreen ? HWND_TOPMOST : nullptr, 0, 0,
        options.fullscreen ? options.width : outer_width,
        options.fullscreen ? options.height : outer_height, SWP_FRAMECHANGED)) {
        error = GetLastError(); return false;
    }
    error = ERROR_SUCCESS;
    return true;
}
}
