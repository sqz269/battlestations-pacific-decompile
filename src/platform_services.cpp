#include "bsp/platform_services.hpp"
#include "bsp/xlive_manager_owner.hpp"

#include <stdexcept>

namespace bsp {
PlatformServices::PlatformServices(Win32PlatformState& platform,
    PlatformCursorGlobals cursor, InputFocusBackendState* volatile& input,
    XLiveSystemPumpContext* volatile& online, InputFocusResetHost& reset,
    InputFocusDeviceHost& devices, PlatformApplicationServiceHost& application,
    XLiveLibrary& library) noexcept
    : platform_(platform), cursor_(cursor), input_(input), online_(&online),
      reset_(reset), devices_(devices), application_(application), library_(library) {}

PlatformServices::PlatformServices(Win32PlatformState& platform,
    PlatformCursorGlobals cursor, InputFocusBackendState* volatile& input,
    XLiveManagerOwner* volatile& online, InputFocusResetHost& reset,
    InputFocusDeviceHost& devices, PlatformApplicationServiceHost& application,
    XLiveLibrary& library) noexcept
    : platform_(platform), cursor_(cursor), input_(input), online_owner_(&online),
      reset_(reset), devices_(devices), application_(application), library_(library) {}

XLiveSystemPumpContext* PlatformServices::current_online_context() const noexcept {
    if (online_owner_) {
        auto* owner = *online_owner_;
        return owner ? &owner->context : nullptr;
    }
    return *online_;
}

bool PlatformServices::pretranslate(MSG& message) { return library_.pretranslate(message); }
void PlatformServices::frame() {
    run_platform_application_service_00bece70(platform_, cursor_, *this, application_);
}
void PlatformServices::update_cursor_focus_00becb20(bool loading) {
    update_platform_cursor_focus_00becb20(platform_, loading, cursor_, *this);
}
PlatformManagerFlags* PlatformServices::current_platform_manager_00f8abe8() noexcept {
    auto* context = current_online_context();
    return context ? &context->flags : nullptr;
}
const InputBindingDeviceGroups* PlatformServices::current_input_device_groups_00f8bbf4()
    noexcept {
    auto* backend = input_;
    return backend ? &backend->groups : nullptr;
}
void PlatformServices::pump_platform_manager_00a409f0(PlatformManagerFlags& flags) {
    auto* context = current_online_context();
    if (!context || &context->flags != &flags)
        throw std::logic_error("platform cursor requires the current online manager binding");
    pump_xlive_system_00a409f0(*context);
}
void PlatformServices::reset_focus_input_00beca40(Win32PlatformState&) {
    bsp::reset_focus_input_00beca40(*this, devices_);
}
InputFocusBackendState* PlatformServices::current_backend_00f8bbf4() { return input_; }
InputTickState& PlatformServices::input_manager_004bec00() {
    return reset_.input_manager_004bec00();
}
void PlatformServices::set_mouse_cooperative_level_00a9a140(InputDevice& device,
    std::uint32_t flags) {
    auto* mouse = dynamic_cast<MouseInputDevice*>(&device);
    if (!mouse) throw std::logic_error("native mouse slot requires its actual mouse implementation");
    bsp::set_mouse_cooperative_level_00a9a140(*mouse,
        get_platform_window_00bec230(platform_), flags);
}
void PlatformServices::update_current_input_backend_vslot_04(float seconds) {
    auto* backend = input_;
    if (!backend) throw std::logic_error("native input update requires the current backend");
    update_input_backend_00a918a0(*backend, seconds, devices_);
}
int PlatformServices::show_cursor(bool visible) { return ShowCursor(visible ? TRUE : FALSE); }

ApplicationFrameService::ApplicationFrameService(void* application,
    ApplicationFrameState& frame, FrameMarkerColor& marker, ApplicationFrameHost& host) noexcept
    : application_(application), frame_(frame), marker_(marker), host_(host) {}
void ApplicationFrameService::run_application_frame_vslot_10(void* application) {
    if (!application || application != application_)
        throw std::logic_error("platform application frame binding does not match the current owner");
    run_application_frame(frame_, marker_, host_);
}
} // namespace bsp
