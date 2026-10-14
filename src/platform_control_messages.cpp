#include "bsp/platform_control_messages.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {

PlatformPowerImports win32_platform_power_imports() noexcept {
    return {&GetActivePwrScheme, &ReadPwrScheme, &SetActivePwrScheme,
        &SystemParametersInfoA, &PostQuitMessage};
}

void initialize_platform_power_00bed223_fragment(const PlatformPowerImports& imports,
    Win32PlatformState& state, POWER_POLICY& saved, POWER_POLICY& modified) noexcept {
    // BED227 sets the sentinel before the API receives the actual +4C cell.
    state.power_scheme = (std::numeric_limits<std::uint32_t>::max)();
    if (imports.get_active_pwr_scheme(&state.power_scheme)) {
        imports.read_pwr_scheme(state.power_scheme, &saved); // BED23D, result ignored
        // BED242 captures the scheme before the 24h-dword copy.
        const auto scheme = state.power_scheme;
        std::memcpy(&modified, &saved, sizeof(modified));
        modified.user.VideoTimeoutDc = 0; // native +11C, BED258
        modified.user.VideoTimeoutAc = 0; // native +118, BED25E
        imports.set_active_pwr_scheme(scheme, nullptr, &modified); // BED264
    }
    imports.system_parameters_info(0x11, 0, nullptr, 0); // BED271
}

void disable_platform_frames_and_post_quit_00bebf70(const PlatformPowerImports& imports,
    Win32PlatformState& state, POWER_POLICY& saved) noexcept {
    const auto scheme = state.power_scheme;
    if (scheme != (std::numeric_limits<std::uint32_t>::max)()) {
        imports.set_active_pwr_scheme(scheme, nullptr, &saved); // BEBF82
    }
    state.frames_enabled = false; // BEBF89, including restore failure
    imports.post_quit_message(0); // BEBF8D
}

PlatformPowerPolicyContext::PlatformPowerPolicyContext(PlatformPowerImports imports) noexcept
    : imports_(imports) {
    // Native BECDA0/BE2AC0 perform no +50/+E0 stores. Do not manufacture a
    // zero policy: BED23D's ignored read failure retains the allocation bytes.
}

void PlatformPowerPolicyContext::initialize_after_render_entry_cache(Win32PlatformState& state) {
    if (state_ != nullptr && state_ != &state)
        throw std::logic_error("platform power policy belongs to another platform receiver");
    initialize_platform_power_00bed223_fragment(imports_, state, saved_, modified_);
    state_ = &state;
}

bool PlatformPowerPolicyContext::initialized_for(const Win32PlatformState& state) const noexcept {
    return state_ == &state;
}

void PlatformPowerPolicyContext::stop_existing_window(Win32PlatformState& state) {
    // This check is the projected owner contract, outside native BEBF70. A
    // constructor-normalized +4C cannot replace BED223's actual scheme capture.
    if (!initialized_for(state))
        throw std::logic_error("platform stop requires this receiver's completed power-policy producer");
    disable_platform_frames_and_post_quit_00bebf70(imports_, state, saved_);
}

void enable_platform_screensaver_00bece41_fragment(
    const PlatformPowerImports& imports) noexcept {
    imports.system_parameters_info(0x11, 1, nullptr, 0);
}

PlatformControlMessageImports win32_platform_control_message_imports() noexcept {
    return {&GetWindowLongA, &DefWindowProcA};
}

bool handle_platform_control_message_00bed3b0_fragment(
    const PlatformControlMessageImports& imports, Win32PlatformState& active,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) noexcept {
    switch (message) {
    case WM_PAINT:
    case WM_GETMINMAXINFO:
    case WM_SETTINGCHANGE:
    case WM_EXITSIZEMOVE:
    case WM_ENTERSIZEMOVE:
    case WM_SYSCOMMAND:
        break;
    default:
        return false;
    }

    // BED3C5 precedes dispatch for every non-WM_CREATE message. Merely retain
    // the pointer here: early WM_GETMINMAXINFO does not require a stored state.
    auto* const state = reinterpret_cast<Win32PlatformState*>(
        imports.get_window_long(window, 0));
    switch (message) {
    case WM_PAINT:
        imports.default_message(window, message, wparam, lparam); // BED5CE
        result = 0; // BED5D7 deliberately discards DefWindowProcA's result
        return true;
    case WM_GETMINMAXINFO: {
        auto& limits = *reinterpret_cast<MINMAXINFO*>(lparam);
        limits.ptMinTrackSize.x = 100; // BED687, LPARAM+18h
        limits.ptMinTrackSize.y = 100; // BED68A, LPARAM+1Ch
        break;
    }
    case WM_SETTINGCHANGE:
        if (wparam == 0x20 || wparam == 0x21) {
            active.settings_changed_2c = true; // BED6CA uses explicit receiver
        }
        break;
    case WM_EXITSIZEMOVE:
        state->frames_enabled = true; // BED720 uses captured window-extra state
        break;
    case WM_ENTERSIZEMOVE:
        state->frames_enabled = false; // BED739
        break;
    case WM_SYSCOMMAND:
        if (wparam == 0xf140) { // BED74A exact DWORD equality, no FFF0h mask
            result = 1;
            return true;
        }
        break;
    }
    result = imports.default_message(window, message, wparam, lparam);
    return true;
}

} // namespace bsp
