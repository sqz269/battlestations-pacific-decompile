#pragma once

#include "bsp/game_sound_platform.hpp"
#include "bsp/joystick_input.hpp"
#include "bsp/xinput_device.hpp"

namespace bsp {
struct XLiveManagerOwner;
}

namespace bsp::game {

// Concrete load/cursor adapter over the existing native policy, reset, pump
// and device implementations. It owns only dispatch adapters, never an input,
// action or online manager, publication slot, SDK library or COM reference.
//
// Borrow the application's actual mutable publications. Both native slots are
// loader-zero until AFTER sound startup: online constructor at0073DC7C, input
// constructor at0073DD8E. BECB20 first checks the online slot, so this adapter
// is usable for that real pre-online phase without fabricating any owner.
//
// actions_provider is a HOST provider slot, not another native singleton. It
// may remain null until focus reset actually reaches004BEC00; once reached it
// must supply the genuine lazy action-manager getter and its existing domain.
// Its current_backend method is deliberately unused: input_ is authoritative.
// An absent required provider is a binding error, never a substitute operation.
// All supplied slots/services must outlive this object and borrowed load_events.
// platform is the same stable Win32PlatformState published at0109CF04.
class GamePlatformServices final : public PlatformCursorHost,
    private InputFocusResetHost {
public:
    GamePlatformServices(Win32PlatformState&, PlatformCursorGlobals,
        InputFocusBackendState* volatile& input_00f8bbf4,
        XLiveManagerOwner* volatile& online_00f8abe8,
        InputFocusResetHost* volatile& actions_provider,
        XLiveLibrary&) noexcept;
    GamePlatformServices(const GamePlatformServices&) = delete;
    GamePlatformServices& operator=(const GamePlatformServices&) = delete;

    GameSoundLoadEvents& load_events() noexcept { return load_events_; }
    // Same backend's activation/poll and mouse-reset deletion. This is not a
    // complete backend owner/destructor (keyboard deletion remains external).
    InputFocusDeviceHost& devices() noexcept { return keyboard_mouse_; }

    PlatformManagerFlags* current_platform_manager_00f8abe8() noexcept override;
    const InputBindingDeviceGroups* current_input_device_groups_00f8bbf4()
        noexcept override;
    void pump_platform_manager_00a409f0(PlatformManagerFlags&) override;
    void reset_focus_input_00beca40(Win32PlatformState&) override;
    void set_mouse_cooperative_level_00a9a140(InputDevice&, std::uint32_t) override;
    void update_current_input_backend_vslot_04(float seconds) override;
    int show_cursor(bool visible) override;

private:
    InputFocusBackendState* current_backend_00f8bbf4() override;
    InputTickState& input_manager_004bec00() override;

    Win32PlatformState& platform_;
    InputFocusBackendState* volatile& input_;
    XLiveManagerOwner* volatile& online_;
    InputFocusResetHost* volatile& actions_provider_;
    JoystickFocusDeviceHost joystick_;
    XInputFocusDeviceHost xinput_;
    KeyboardMouseFocusDeviceHost keyboard_mouse_;
    GameSoundLoadEvents load_events_;
};
} // namespace bsp::game
