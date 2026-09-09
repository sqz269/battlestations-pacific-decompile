#include "bsp/platform_loop.hpp"

namespace bsp {
void platform_run_loop_00bec1a0(PlatformLoopState& state, PlatformLoopCallbacks& callbacks) {
    MSG message;
    do {
        if (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (!callbacks.pretranslate(message)) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        } else if (state.frames_enabled) {
            callbacks.frame();
        }
    } while (!state.exit_requested);
    state.loop_finished = true;
}
}
