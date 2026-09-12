#include "bsp/game_platform_services.hpp"
#include "bsp/xlive_manager_owner.hpp"

#include <stdexcept>

namespace bsp::game {
GamePlatformServices::GamePlatformServices(Win32PlatformState& platform,
    PlatformCursorGlobals cursor, InputFocusBackendState* volatile& input,
    XLiveManagerOwner* volatile& online, InputFocusResetHost* volatile& actions,
    XLiveLibrary& library) noexcept
    : platform_(platform), input_(input), online_(online), actions_provider_(actions),
      xinput_(&joystick_), keyboard_mouse_(platform, &xinput_),
      load_events_(platform, cursor, *this, library) {}

PlatformManagerFlags* GamePlatformServices::current_platform_manager_00f8abe8() noexcept {
    auto* owner = online_;
    return owner ? &owner->context.flags : nullptr;
}
const InputBindingDeviceGroups* GamePlatformServices::current_input_device_groups_00f8bbf4()
    noexcept {
    auto* backend = input_;
    return backend ? &backend->groups : nullptr;
}
void GamePlatformServices::pump_platform_manager_00a409f0(PlatformManagerFlags& flags) {
    auto* owner = online_;
    if (!owner || &owner->context.flags != &flags)
        throw std::logic_error("cursor pump requires the current published online owner");
    pump_xlive_system_00a409f0(owner->context);
}
void GamePlatformServices::reset_focus_input_00beca40(Win32PlatformState& platform) {
    if (&platform != &platform_)
        throw std::logic_error("cursor reset must use the shared platform owner");
    bsp::reset_focus_input_00beca40(*this, keyboard_mouse_);
}
InputFocusBackendState* GamePlatformServices::current_backend_00f8bbf4() { return input_; }
InputTickState& GamePlatformServices::input_manager_004bec00() {
    auto* provider = actions_provider_; // only at the original lazy-getter call
    if (!provider)
        throw std::logic_error("focus reset reached an unbound real action-manager getter");
    return provider->input_manager_004bec00();
}
void GamePlatformServices::set_mouse_cooperative_level_00a9a140(InputDevice& device,
    std::uint32_t flags) {
    auto* mouse = dynamic_cast<MouseInputDevice*>(&device);
    if (!mouse)
        throw std::logic_error("class1 cursor device must be the actual mouse implementation");
    bsp::set_mouse_cooperative_level_00a9a140(*mouse,
        get_platform_window_00bec230(platform_), flags);
}
void GamePlatformServices::update_current_input_backend_vslot_04(float seconds) {
    auto* backend = input_;
    if (!backend)
        throw std::logic_error("cursor update requires the current published input backend");
    update_input_backend_00a918a0(*backend, seconds, keyboard_mouse_);
}
int GamePlatformServices::show_cursor(bool visible) {
    return ::ShowCursor(visible ? TRUE : FALSE);
}
} // namespace bsp::game
