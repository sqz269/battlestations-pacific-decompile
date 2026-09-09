#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace bsp {
struct PlatformWindowOptions {
    HINSTANCE instance{};
    const char* name{}; // both ANSI class name and initial title
    WNDPROC procedure{}; // required native thunk or explicitly diagnostic procedure
    void* platform{}; // CreateWindowEx lpParam, normally native platform object
    bool fullscreen{};
    int x{}, y{}, width{}, height{};
};

// Fragment00becf29..00bed087 of cdecl eleven-argument00becee0. Output references
// must initially be zero. On failure, any created HWND/class remain owned by the
// caller and must be destroyed/unregistered. Does not show the window or invoke
// unresolved renderer/application/power-setting initialization.
bool create_platform_window_00becee0_fragment(const PlatformWindowOptions& options,
    HWND& output, ATOM& registered_class, DWORD& error) noexcept;
}
