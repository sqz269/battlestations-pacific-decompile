#pragma once

#include "bsp/audio_online_startup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/session_polls.hpp"

#include <array>
#include <cstddef>
#include <functional>
#include <string>

namespace bsp {

// Exactly the 0x15 DWORDs (84 bytes) copied to manager+32h at 00A4034C. Decoding the
// consumer's selected fields must not discard the remaining invite payload.
using XLiveAcceptedInvite = std::array<std::byte, 84>;

struct XLiveNotificationLibrary {
    virtual ~XLiveNotificationLibrary() = default;
    virtual void* notify_create_listener(std::uint64_t areas) = 0;
    virtual bool notify_get_next(void* listener, std::uint32_t filter,
        std::uint32_t& id, std::uint32_t& parameter) = 0;
    // The native caller ignores the result and copies all 84 output bytes.
    // The adapter must supply a complete payload or report unavailable output.
    virtual XLiveAcceptedInvite invite_get_accepted_info(std::uint32_t user) = 0;
    virtual std::int32_t update_system(const wchar_t* path) = 0;
};

struct XLiveNotificationGameHost {
    virtual ~XLiveNotificationGameHost() = default;
    virtual void poll_signin_debounce_00a3f3e0() = 0;
    virtual void refresh_signin_00a3f440() = 0;
    virtual void profile_setting_changed_00a3e600(std::uint8_t mask) = 0;
    virtual void pump_achievements_00a3fa70(bool force) = 0;
    virtual void title_update_path_00a3ff20(NativeString& output) = 0;
    virtual void system_update_path_00a3fde0(NativeString& output) = 0;
    virtual std::wstring widen_update_path_004c5e60(const char* path) = 0;
    virtual void launch_update_00a3e560(const char* executable,
        const char* path) = 0;
    virtual void sleep_milliseconds(std::uint32_t milliseconds) = 0;
    // Native _exit(0). Returning violates this required host contract.
    [[noreturn]] virtual void exit_process(int code) = 0;
};

struct XLiveNotificationGlobals {
    // Optional native pointers F8ABEC/F8ABF0 and current F8A2FC virtual+28h.
    // UI hook receives the bool in CL, BEFORE manager+3E8 is written.
    std::function<void(bool)> system_ui_hook;
    std::function<void()> storage_hook;
    std::function<void()> online_client_friends_changed;
};

// 00A40110, ECX=manager, no stack arguments. Listener is re-read for every
// XNotifyGetNext; only INVALID_HANDLE_VALUE (-1), not null, recreates it.
// Logs call native 004254B0 (bare RET) and have no application-side effect.
void drain_xlive_notifications_00a40110(OnlineSystemState& online,
    PlatformManagerFlags& flags, XLiveAcceptedInvite& accepted_invite,
    XLiveNotificationGlobals& globals, XLiveNotificationLibrary& library,
    XLiveNotificationGameHost& game, NativeStringStorage& strings);

} // namespace bsp
