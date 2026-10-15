#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace bsp {
// Standalone projections own these three cells. An actual platform view binds
// the SAME +42/+181/+43 bytes; copying a standalone loop copies values rather
// than retargeting its references. This is a Source interface, not native ABI.
struct PlatformLoopState {
private:
    bool owned_frames_{};
    bool owned_exit_{};
    bool owned_finished_{};
protected:
    struct NativeCells {};
    PlatformLoopState(bool& frames, bool& exit, bool& finished, NativeCells) noexcept
        : frames_enabled(frames), exit_requested(exit), loop_finished(finished) {}
public:
    bool& frames_enabled;
    bool& exit_requested;
    bool& loop_finished;
    PlatformLoopState() noexcept
        : frames_enabled(owned_frames_), exit_requested(owned_exit_), loop_finished(owned_finished_) {}
    PlatformLoopState(bool frames, bool exit, bool finished) noexcept
        : owned_frames_(frames), owned_exit_(exit), owned_finished_(finished),
          frames_enabled(owned_frames_), exit_requested(owned_exit_), loop_finished(owned_finished_) {}
    PlatformLoopState(const PlatformLoopState& other) noexcept
        : PlatformLoopState(other.frames_enabled, other.exit_requested, other.loop_finished) {}
    PlatformLoopState(PlatformLoopState&& other) noexcept : PlatformLoopState(other) {}
    PlatformLoopState& operator=(const PlatformLoopState& other) noexcept {
        frames_enabled = other.frames_enabled;
        exit_requested = other.exit_requested;
        loop_finished = other.loop_finished;
        return *this;
    }
    PlatformLoopState& operator=(PlatformLoopState&& other) noexcept { return *this = other; }
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
