#include "bsp/xlive_manager_runtime.hpp"
#include "bsp/xlive_library.hpp"

#include <process.h>
#include <stdexcept>

namespace bsp {
XLiveManagerRuntime::XLiveManagerRuntime(XLiveLibrary& library,
    OnlineSystemState& online, PlatformManagerFlags& flags,
    XLiveSystemPumpState& pump, XLivePumpHeartbeat& heartbeat,
    XLiveSigninStorage& signin, XLiveAcceptedInvite& invite,
    XLiveNotificationGlobals& globals, FrameClock* volatile& clock,
    XLiveStartupHost& startup, XLiveGameServices& game, NativeStringStorage& strings)
    : XLiveSdkAdapter(library), library_(library),
      context_{online, flags, pump, heartbeat, *this, startup, strings},
      signin_{online, flags, signin, pump.signin_flag_120, pump.signin_flag_3bc},
      invite_(invite), globals_(globals), clock_(clock), game_(game) {}

void XLiveManagerRuntime::drain_notifications_00a40110() {
    drain_xlive_notifications_00a40110(context_.online, context_.flags, invite_,
        globals_, library_, *this, context_.strings);
}
ClockTimestamp XLiveManagerRuntime::sample_clock_vslot20() {
    auto* clock = clock_;
    if (!clock) throw std::logic_error("native XLive sampling requires the current frame clock");
    ClockTimestamp result;
    if (!sample_frame_clock_00bee080(*clock, result))
        throw std::runtime_error("XLive clock sample unavailable: QueryPerformanceCounter failed");
    return result;
}
ClockTimestamp XLiveManagerRuntime::sample_clock_vslot_20() { return sample_clock_vslot20(); }
std::uint32_t XLiveManagerRuntime::read_manager_dword_8c_indexed(std::uint32_t index) {
    return read_cached_manager_dword_8c_indexed(signin_, index);
}
std::wstring XLiveManagerRuntime::resolve_localization_00a9fad0(const NativeString& key) {
    return game_.resolve_localization_00a9fad0(key);
}
void XLiveManagerRuntime::invoke_state_callback(const void* callback) {
    context_.startup_host.invoke_state_callback(callback);
}
void XLiveManagerRuntime::poll_signin_debounce_00a3f3e0() {
    bsp::poll_signin_debounce_00a3f3e0(signin_, *this);
}
void XLiveManagerRuntime::refresh_signin_00a3f440() { bsp::refresh_signin_00a3f440(signin_, *this); }
void XLiveManagerRuntime::profile_setting_changed_00a3e600(std::uint8_t mask) {
    bsp::profile_setting_changed_00a3e600(signin_, mask, *this);
}
void XLiveManagerRuntime::pump_achievements_00a3fa70(bool force) {
    pump_online_achievements_00a3fa70(context_, force);
}
void XLiveManagerRuntime::title_update_path_00a3ff20(NativeString& path) {
    game_.title_update_path_00a3ff20(path);
}
void XLiveManagerRuntime::system_update_path_00a3fde0(NativeString& path) {
    game_.system_update_path_00a3fde0(path);
}
std::wstring XLiveManagerRuntime::widen_update_path_004c5e60(const char* path) {
    return game_.widen_update_path_004c5e60(path);
}
void XLiveManagerRuntime::launch_update_00a3e560(const char* executable, const char* path) {
    game_.launch_update_00a3e560(executable, path);
}
void XLiveManagerRuntime::sleep_milliseconds(std::uint32_t milliseconds) { Sleep(milliseconds); }
[[noreturn]] void XLiveManagerRuntime::exit_process(int code) { _exit(code); }
} // namespace bsp
