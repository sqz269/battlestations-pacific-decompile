#pragma once

#include "bsp/platform_window.hpp"
#include <powrprof.h>

namespace bsp {

// Producer BED223..BED268 copies exactly 24h dwords from native +50 to +E0.
// The projected policies are supplied by the owner, not synthesized by the
// stop routine. These SDK types are the actual imported PowrProf contracts.
static_assert(sizeof(POWER_POLICY) == 0x90);
static_assert(offsetof(POWER_POLICY, user) == 0);
static_assert(offsetof(USER_POWER_POLICY, VideoTimeoutAc) == 0x38);
static_assert(offsetof(USER_POWER_POLICY, VideoTimeoutDc) == 0x3c);

struct PlatformPowerImports {
    decltype(&GetActivePwrScheme) get_active_pwr_scheme;
    decltype(&ReadPwrScheme) read_pwr_scheme;
    decltype(&SetActivePwrScheme) set_active_pwr_scheme;
    decltype(&SystemParametersInfoA) system_parameters_info;
    decltype(&PostQuitMessage) post_quit_message;
};
PlatformPowerImports win32_platform_power_imports() noexcept;

// Partial projection of BECEE0: only BED223..BED276, after renderer and render
// entry cache initialization, before BECEE0 returns. The native constructor
// does not initialize these policies. Read/set failures are deliberately not
// promoted to success or used as new control-flow gates: native ignores them.
void initialize_platform_power_00bed223_fragment(const PlatformPowerImports& imports,
    Win32PlatformState& state, POWER_POLICY& saved, POWER_POLICY& modified) noexcept;

// Complete BEBF70, ECX=this, RET. saved denotes original native +50 policy.
// Restores only when native +4C != -1, then clears +42 and posts WM_QUIT(0).
// Does not destroy HWND or assign close+180, exit+181, or finished+43.
void disable_platform_frames_and_post_quit_00bebf70(const PlatformPowerImports& imports,
    Win32PlatformState& state, POWER_POLICY& saved) noexcept;

// Retained owner for native +50/+E0 policy storage. BECDA0 and its BE2AC0
// base leave both policies and +4C untouched; BED223 is the first producer.
// Its storage is deliberately default-initialized, retaining allocation bytes
// when native ReadPwrScheme fails. No constructor/destructor calls an OS API.
// This is a projected service, not the 184h-byte native platform object ABI.
class PlatformPowerPolicyContext final {
public:
    explicit PlatformPowerPolicyContext(PlatformPowerImports imports) noexcept;
    PlatformPowerPolicyContext(const PlatformPowerPolicyContext&) = delete;
    PlatformPowerPolicyContext& operator=(const PlatformPowerPolicyContext&) = delete;

    // Explicit BECEE0 BED223 boundary: call after the actual renderer and
    // render-entry cache initialization. It does not schedule ordinary shutdown.
    void initialize_after_render_entry_cache(Win32PlatformState& state);
    // BECF27 calls this only for an already configured HWND. A fresh projected
    // state has no genuine saved-policy producer and cannot satisfy this call.
    void stop_existing_window(Win32PlatformState& state);
    bool initialized_for(const Win32PlatformState& state) const noexcept;
    const POWER_POLICY& saved_policy() const noexcept { return saved_; }
    const POWER_POLICY& modified_policy() const noexcept { return modified_; }

private:
    PlatformPowerImports imports_;
    POWER_POLICY saved_;
    POWER_POLICY modified_;
    Win32PlatformState* state_{};
};

// Only the SystemParametersInfoA call at BECE41, with its argument setup at
// BECE31..BECE39. The deleting destructor BECE30 also changes the vtable,
// destroys the text queue/base and conditionally frees; none are covered here.
// It enables the screensaver unconditionally, not a saved prior setting.
void enable_platform_screensaver_00bece41_fragment(
    const PlatformPowerImports& imports) noexcept;

struct PlatformControlMessageImports {
    decltype(&GetWindowLongA) get_window_long;
    decltype(&DefWindowProcA) default_message;
};
PlatformControlMessageImports win32_platform_control_message_imports() noexcept;

// Partial BED3B0 projection: WM_PAINT BED5C1..BED5DC; WM_GETMINMAXINFO
// BED67E..BED69F; WM_SETTINGCHANGE BED6B4..BED6DA; WM_EXITSIZEMOVE
// BED718..BED730; WM_ENTERSIZEMOVE BED731..BED749; WM_SYSCOMMAND
// BED74A..BED761, borrowing common dispatch/default blocks.
// False leaves result unchanged and performs no calls. Other BED3B0 arms
// (create/size/activation/close/text) belong to their separate implementations.
// Window-extra state and active are distinct receivers; early messages may
// have a null window-extra pointer which only sizing-loop arms dereference.
bool handle_platform_control_message_00bed3b0_fragment(
    const PlatformControlMessageImports& imports, Win32PlatformState& active,
    HWND window, UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) noexcept;

} // namespace bsp
