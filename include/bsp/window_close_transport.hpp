#pragma once

#include "bsp/native_online_notifications.hpp"

#include <cstdint>
#include <string>

namespace bsp {
struct Win32PlatformState;
struct FrontEndPromptScreen;
struct FrontEndPromptHost;
class NativeStringStorage;

// Source bindings, not a native object layout. All references must outlive the
// frame and any confirmation callback. The online slot is the actual mutable
// F8ABE8 publication, not a cached boolean or an independently created manager.
struct WindowCloseTransportContext final {
    Win32PlatformState& platform;
    bool& global_exit_00e1ae75;
    NativeOnlineManagerStorage* volatile& online_00f8abe8;
};

// The two native calls after 004CA35F need the actual prompt owner. No default
// prompt, response, or fallback exit is supplied by the transport.
struct WindowClosePromptCalls {
    virtual ~WindowClosePromptCalls() = default;
    virtual void raise_00531b00(std::int32_t slot, const std::string& message,
        std::int32_t kind, std::uint32_t callback, std::int32_t unused,
        const std::string& title, std::int32_t timeout, std::int32_t result) = 0;
    virtual void focus_last_00530c20() = 0;
};

struct WindowClosePromptTarget final {
    FrontEndPromptScreen& screen;
    FrontEndPromptHost& host;
};
struct WindowClosePromptGetter {
    virtual ~WindowClosePromptGetter() = default;
    // 00425D10 is a real lazy singleton getter. Resolve it independently at
    // 004CA3A2 and 004CA41B; do not substitute a made-up global or screen.
    virtual WindowClosePromptTarget get_00425d10() = 0;
};

// Concrete composition with the recovered prompt bodies and native strings.
// The application's prompt getter/GUI owner is a separate required dependency.
class RetainedWindowClosePromptCalls final : public WindowClosePromptCalls {
public:
    RetainedWindowClosePromptCalls(NativeStringStorage& strings,
        WindowClosePromptGetter& getter) noexcept : strings_(strings), getter_(getter) {}
    void raise_00531b00(std::int32_t, const std::string&, std::int32_t,
        std::uint32_t, std::int32_t, const std::string&, std::int32_t, std::int32_t) override;
    void focus_last_00530c20() override;
private:
    NativeStringStorage& strings_;
    WindowClosePromptGetter& getter_;
};

enum class WindowCloseDispatch {
    no_request,
    suppressed,
    exit_requested,
    confirmation_opened,
    missing_online_owner,
    missing_prompt_owner,
};

// Executes existing 004CA2F0 policy against the live close/online/exit fields.
// Missing required owners are source admission failures BEFORE native effects;
// the request remains pending. This is not a reconstruction of a native null
// check or an alternate native result. A null prompt is admissible only when
// the current native branch does not use it. Call on the owning UI thread.
WindowCloseDispatch process_window_close_transport_004ca2f0(
    WindowCloseTransportContext&, std::int32_t game_state_5d4,
    WindowClosePromptCalls* prompt = nullptr);

// Dispatch from the real prompt host's invoke_callback. Only address 004BBC50
// is consumed; it clears +180, and response 1 sets the sticky global exit byte.
// It neither sets platform +181 nor schedules BEBF70/power restoration.
bool dispatch_window_close_confirmation_004bbc50(WindowCloseTransportContext&,
    std::uint32_t callback, std::int32_t response) noexcept;
} // namespace bsp
