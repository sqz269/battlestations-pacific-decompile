#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "bsp/text_input.hpp"

namespace bsp {

// Win32 boundary for the WM_KEYDOWN and WM_CHAR fragments of 00BED3B0.
// The caller owns text for the lifetime of the window and gives its queue to
// the actual text-owner dispatcher. Native text arms use the explicit platform
// receiver, not the GetWindowLongA(window, 0) window-extra receiver. This
// function does not enable text input.
// Returns false for messages outside these fragments. Handled messages still
// reach DefWindowProcA; their LRESULT is written to result.
bool handle_platform_text_message_00bed3b0_fragment(PlatformTextInput& text,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);

} // namespace bsp
