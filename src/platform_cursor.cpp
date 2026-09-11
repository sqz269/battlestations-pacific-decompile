#include "bsp/platform_cursor.hpp"

namespace bsp {

InputDevice* get_input_class_device_004ba6d0(const InputBindingDeviceGroups& groups,
    std::int32_t device_class, std::uint32_t device_index) noexcept {
    const auto& devices = groups[static_cast<std::size_t>(device_class)];
    if (device_index >= devices.size()) return nullptr;
    return devices[device_index];
}

void update_platform_cursor_focus_00becb20(Win32PlatformState& platform,
    bool loading, PlatformCursorGlobals globals, PlatformCursorHost& host) {
    if (!host.current_platform_manager_00f8abe8()) return;
    const auto* groups = host.current_input_device_groups_00f8bbf4();
    if (!groups) return;
    if (!get_input_class_device_004ba6d0(*groups, 1, 0)) return;

    if (loading) {
        host.pump_platform_manager_00a409f0(*host.current_platform_manager_00f8abe8());
    }

    // CL and local AL are captured before any focus/input callbacks. In
    // particular a callback changing the manager's UI flag cannot change this
    // call's final previous-UI write or its desired cursor visibility.
    const bool focused = platform.byte_041;
    const bool system_ui = host.current_platform_manager_00f8abe8()->system_ui_visible;
    const bool should_show = !focused || system_ui;
    bool focus_was_reset = false; // BL in the native body

    if (system_ui && !globals.previous_system_ui_0109db90) {
        host.reset_focus_input_00beca40(platform);
        focus_was_reset = true;
        auto* mouse = get_input_class_device_004ba6d0(
            *host.current_input_device_groups_00f8bbf4(), 1, 0);
        host.set_mouse_cooperative_level_00a9a140(*mouse, 6);
        if (loading) host.update_current_input_backend_vslot_04(kCursorLoadInputStep_00d7a2f0);
    }

    if (!system_ui && globals.previous_system_ui_0109db90 && should_show) {
        globals.focus_reset_pending_0109db8f = 1;
    }
    if (!should_show && (globals.previous_system_ui_0109db90
        || globals.focus_reset_pending_0109db8f)) {
        host.reset_focus_input_00beca40(platform);
        focus_was_reset = true;
        if (loading) host.update_current_input_backend_vslot_04(kCursorLoadInputStep_00d7a2f0);
        globals.focus_reset_pending_0109db8f = 0;
    }

    if (should_show) {
        if (!globals.cursor_shown_0109db8e) {
            globals.cursor_shown_0109db8e = 1;
            while (host.show_cursor(true) < 0) {}
        }
    } else if (globals.cursor_shown_0109db8e) {
        if (!focus_was_reset) {
            host.reset_focus_input_00beca40(platform);
            if (loading) host.update_current_input_backend_vslot_04(kCursorLoadInputStep_00d7a2f0);
        }
        globals.cursor_shown_0109db8e = 0;
        while (host.show_cursor(false) >= 0) {}
    }
    globals.previous_system_ui_0109db90 = static_cast<std::uint8_t>(system_ui);
}

void run_platform_application_service_00bece70(Win32PlatformState& platform,
    PlatformCursorGlobals globals, PlatformCursorHost& cursor,
    PlatformApplicationServiceHost& application) {
    application.run_application_frame_vslot_10(platform.application);
    update_platform_cursor_focus_00becb20(platform, false, globals, cursor);
}

} // namespace bsp
