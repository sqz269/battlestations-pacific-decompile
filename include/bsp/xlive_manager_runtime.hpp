#pragma once

#include "bsp/xlive_notifications.hpp"
#include "bsp/xlive_sdk_adapter.hpp"

namespace bsp {

// Remaining game-owned lookup/update helpers. Each operation is an actual
// native call; these are required dependencies with no successful defaults.
struct XLiveGameServices {
    virtual ~XLiveGameServices() = default;
    virtual std::wstring resolve_localization_00a9fad0(const NativeString&) = 0;
    virtual void title_update_path_00a3ff20(NativeString&) = 0;
    virtual void system_update_path_00a3fde0(NativeString&) = 0;
    virtual std::wstring widen_update_path_004c5e60(const char*) = 0;
    virtual void launch_update_00a3e560(const char* executable, const char* path) = 0;
};

// Binds the recovered manager routines to the actual DLL and existing clock.
// All input state is borrowed. Constructor defaults are not reconstructed here.
// Keep this runtime and its input owners fixed in memory until SDK work finishes;
// publish context() as the current platform manager only for that same lifetime.
class XLiveManagerRuntime final : public XLiveSdkAdapter,
    public XLiveNotificationGameHost {
public:
    XLiveManagerRuntime(XLiveLibrary&, OnlineSystemState&, PlatformManagerFlags&,
        XLiveSystemPumpState&, XLivePumpHeartbeat&, XLiveSigninStorage&,
        XLiveAcceptedInvite&, XLiveNotificationGlobals&, FrameClock* volatile&,
        XLiveStartupHost&, XLiveGameServices&, NativeStringStorage&);
    XLiveSystemPumpContext& context() noexcept { return context_; }
    void drain_notifications_00a40110() override;
    ClockTimestamp sample_clock_vslot20() override;
    ClockTimestamp sample_clock_vslot_20() override;
    std::uint32_t read_manager_dword_8c_indexed(std::uint32_t) override;
    std::wstring resolve_localization_00a9fad0(const NativeString&) override;
    void invoke_state_callback(const void*) override;
    void poll_signin_debounce_00a3f3e0() override;
    void refresh_signin_00a3f440() override;
    void profile_setting_changed_00a3e600(std::uint8_t) override;
    void pump_achievements_00a3fa70(bool force) override;
    void title_update_path_00a3ff20(NativeString&) override;
    void system_update_path_00a3fde0(NativeString&) override;
    std::wstring widen_update_path_004c5e60(const char*) override;
    void launch_update_00a3e560(const char*, const char*) override;
    void sleep_milliseconds(std::uint32_t) override;
    [[noreturn]] void exit_process(int) override;
private:
    XLiveLibrary& library_;
    XLiveSystemPumpContext context_;
    XLiveSigninState signin_;
    XLiveAcceptedInvite& invite_;
    XLiveNotificationGlobals& globals_;
    FrameClock* volatile& clock_;
    XLiveGameServices& game_;
};
} // namespace bsp
