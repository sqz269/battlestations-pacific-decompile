#include "bsp/platform_loop.hpp"
#include <iostream>
#include <stdexcept>

namespace {
// Probe-only callbacks exercise the real OS queue. They do not simulate game/XLive behavior.
class ProbeCallbacks final : public bsp::PlatformLoopCallbacks {
public:
    explicit ProbeCallbacks(bsp::PlatformLoopState& state) : state_(state) {}
    unsigned frames{}, consumed{};
    bool pretranslate(MSG& message) override {
        if (message.message != WM_APP + 1) return false;
        ++consumed;
        state_.exit_requested = true;
        return true;
    }
    void frame() override {
        ++frames;
        // PeekMessage has created this thread's queue before the first idle callback.
        if (!PostThreadMessageA(GetCurrentThreadId(), WM_APP + 1, 0, 0))
            throw std::runtime_error("Could not post probe shutdown message");
    }
private:
    bsp::PlatformLoopState& state_;
};
}

int main() {
    bsp::PlatformLoopState state{true, false, false};
    ProbeCallbacks callbacks(state);
    bsp::platform_run_loop_00bec1a0(state, callbacks);
    std::cout << "Reconstructed platform loop: frames=" << callbacks.frames
              << " consumed=" << callbacks.consumed << " finished=" << state.loop_finished << '\n';
    std::cout << "Uses the real Windows queue and probe callbacks; game/XLive integration is pending.\n";
    return callbacks.frames == 1 && callbacks.consumed == 1 && state.loop_finished ? 0 : 1;
}
