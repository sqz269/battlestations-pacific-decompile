#include "bsp/platform_loop.hpp"
#include "bsp/win32_event.hpp"
#include "bsp/text_input.hpp"
#include <iostream>
#include <stdexcept>

namespace {
// Probe-only owner checks delivery order; it is not a text-editor fallback.
class ProbeTextOwner final : public bsp::TextInputCallbacks {
public:
    explicit ProbeTextOwner(bsp::TextInputQueue& queue) : queue_(queue) { enabled = true; }
    bool removed_before_callback{}, fallback_after_disable{};
    bool on_event(bsp::TextInputEvent event) override {
        removed_before_callback = queue_.size() == 1 && event.value == 'A' && event.key_event == 0;
        enabled = false;
        return false;
    }
    void fallback(bsp::TextInputEvent event) override {
        fallback_after_disable = !enabled && event.value == 'A' && event.key_event == 0;
    }
private:
    bsp::TextInputQueue& queue_;
};
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
    bsp::PlatformTextInput input_state;
    auto& text = input_state.queue;
    input_state.enqueue_message_00bed3b0_fragment(WM_CHAR, 'X');
    if (text.size() != 0) return 2;
    input_state.enable_00a965a0(true);
    input_state.enqueue_message_00bed3b0_fragment(WM_CHAR, 'X');
    input_state.enable_00a965a0(true); // Even same-state enable clears pending input.
    if (text.size() != 0) return 2;
    input_state.enqueue_message_00bed3b0_fragment(WM_KEYDOWN, 0x10000u | VK_LEFT);
    input_state.enqueue_message_00bed3b0_fragment(WM_CHAR, 'A');
    input_state.enqueue_message_00bed3b0_fragment(WM_KEYDOWN, VK_LEFT);
    ProbeTextOwner owner(text);
    bsp::dispatch_text_input_00a96f40_fragment(text, owner);
    bsp::TextInputEvent input{};
    const bool text_ok = owner.removed_before_callback && owner.fallback_after_disable
        && text.size() == 1 && text.pop_00bece90(input)
        && input.value == VK_LEFT && input.key_event == 1 && text.size() == 0;
    input_state.enqueue_message_00bed3b0_fragment(WM_CHAR, 0x16);
    if (!input_state.clipboard_requested || !text.pop_00bece90(input)
        || input.value != 0x16 || input.key_event != 0) return 2;
    // Leave one node for the destructor's nonempty cleanup path.
    text.append_00bed370({'B', 0});
    std::cout << "Text input queue: byte_event_order=" << text_ok << '\n';
    std::cout << "Text input policy: enable_clear_full_key_filter_clipboard_and_callback_order=" << text_ok << '\n';
    if (!text_ok) return 2;
    // Same auto-reset event type used for worker wake/idle acknowledgment.
    bsp::Win32Event event(false);
    const bool event_ok = event.valid()
        && WaitForSingleObject(event.native(), 0) == WAIT_TIMEOUT
        && event.signal_00bd1910()
        && event.wait_00bd17c0() == WAIT_OBJECT_0
        && WaitForSingleObject(event.native(), 0) == WAIT_TIMEOUT
        && event.signal_00bd1910() && event.reset_00bd1960()
        && WaitForSingleObject(event.native(), 0) == WAIT_TIMEOUT;
    std::cout << "Reconstructed event: auto_reset_and_explicit_reset=" << event_ok << '\n';
    if (!event_ok) return 1;
    bsp::PlatformLoopState state{true, false, false};
    ProbeCallbacks callbacks(state);
    bsp::platform_run_loop_00bec1a0(state, callbacks);
    std::cout << "Reconstructed platform loop: frames=" << callbacks.frames
              << " consumed=" << callbacks.consumed << " finished=" << state.loop_finished << '\n';
    std::cout << "Uses the real Windows queue and probe callbacks; game/XLive integration is pending.\n";
    return callbacks.frames == 1 && callbacks.consumed == 1 && state.loop_finished ? 0 : 1;
}
