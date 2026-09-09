#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace bsp {
// Semantic projection of native bytes +42h, +181h and +43h respectively.
struct PlatformLoopState {
    bool frames_enabled{};
    bool exit_requested{};
    bool loop_finished{};
};

// Required integration boundaries, not default implementations or XLive replacements.
// Native pretranslation is XLivePreTranslateMessage; frame is vtable slot +20h.
struct PlatformLoopCallbacks {
    virtual ~PlatformLoopCallbacks() = default;
    virtual bool pretranslate(MSG& message) = 0;
    virtual void frame() = 0;
};

void platform_run_loop_00bec1a0(PlatformLoopState& state, PlatformLoopCallbacks& callbacks);
}
