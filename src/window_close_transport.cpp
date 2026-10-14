#include "bsp/window_close_transport.hpp"

#include "bsp/engine_seg4_6_9_cc13.hpp"
#include "bsp/frontend_prompts.hpp"
#include "bsp/platform_window.hpp"

#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
struct NativeStringScope final {
    NativeString value;
    NativeStringStorage& storage;
    ~NativeStringScope() { value.release_to(storage); }
};

class BoundWindowCloseHost final : public cc13_sys::WindowCloseHost {
public:
    BoundWindowCloseHost(WindowCloseTransportContext& context, WindowClosePromptCalls* prompt)
        : context_(context), prompt_(prompt) {}
    bool take_close_request_180() override {
        const bool pending = context_.platform.close_requested;
        context_.platform.close_requested = false;
        return pending;
    }
    bool platform_busy_3e8() override {
        // Native loads the current publication after consuming the request.
        return context_.online_00f8abe8->notification9_3e8 != 0;
    }
    void set_exit_flag_00e1ae75() override { context_.global_exit_00e1ae75 = true; }
    void raise_prompt_00531b00(std::int32_t slot, const std::string& message,
        std::int32_t kind, std::uint32_t callback, std::int32_t unused,
        const std::string& title, std::int32_t timeout, std::int32_t result) override {
        if (prompt_ == nullptr) throw std::logic_error("window close prompt owner is unbound");
        prompt_->raise_00531b00(slot, message, kind, callback, unused, title, timeout, result);
    }
    void prompt_screen_00530c20() override { prompt_->focus_last_00530c20(); }
private:
    WindowCloseTransportContext& context_;
    WindowClosePromptCalls* prompt_;
};
} // namespace

void RetainedWindowClosePromptCalls::raise_00531b00(std::int32_t slot,
    const std::string& message, std::int32_t kind, std::uint32_t callback,
    std::int32_t unused, const std::string& title, std::int32_t timeout,
    std::int32_t result) {
    // 004CA35F..004CA398 constructs title, message, then the owned by-value
    // empty countdown argument. 00531B00 consumes that last argument.
    NativeStringScope title_string{{}, strings_};
    title_string.value.assign_0041e870(strings_, title.c_str());
    NativeStringScope message_string{{}, strings_};
    message_string.value.assign_0041e870(strings_, message.c_str());
    NativeStringScope countdown{{}, strings_};
    countdown.value.assign_0041e870(strings_, "");
    const auto target = getter_.get_00425d10();
    raise_prompt_00531b00(target.screen, target.host, slot, message_string.value,
        static_cast<PromptKind>(kind), callback, static_cast<std::uint32_t>(unused),
        title_string.value, static_cast<float>(timeout), result,
        std::move(countdown.value), true);
    // Message then title release at 004CA3D0..004CA416, before the next getter.
}

void RetainedWindowClosePromptCalls::focus_last_00530c20() {
    const auto target = getter_.get_00425d10();
    focus_last_prompt_navigation_00530c20(target.screen, target.host);
}

WindowCloseDispatch process_window_close_transport_004ca2f0(
    WindowCloseTransportContext& context, std::int32_t game_state,
    WindowClosePromptCalls* prompt) {
    if (!context.platform.close_requested) return WindowCloseDispatch::no_request;
    auto* const online = context.online_00f8abe8;
    if (online == nullptr) return WindowCloseDispatch::missing_online_owner;
    const bool suppressed = online->notification9_3e8 != 0;
    const bool direct_exit = game_state == 1 || game_state == 2 || game_state == 4;
    if (!suppressed && !direct_exit && prompt == nullptr)
        return WindowCloseDispatch::missing_prompt_owner;

    BoundWindowCloseHost host(context, prompt);
    cc13_sys::game_process_window_close_request_004ca2f0(game_state, host);
    return suppressed ? WindowCloseDispatch::suppressed
        : direct_exit ? WindowCloseDispatch::exit_requested
                      : WindowCloseDispatch::confirmation_opened;
}

bool dispatch_window_close_confirmation_004bbc50(WindowCloseTransportContext& context,
    std::uint32_t callback, std::int32_t response) noexcept {
    if (callback != 0x004bbc50u) return false;
    context.platform.close_requested = false;
    if (response == 1) context.global_exit_00e1ae75 = true;
    return true;
}
} // namespace bsp
