#include "bsp/xlive_notifications.hpp"

#include <cstring>
#include <exception>

namespace bsp {
namespace {
struct ScopedNotificationString {
    NativeString value;
    NativeStringStorage& storage;
    explicit ScopedNotificationString(NativeStringStorage& s) : storage(s) {}
    ~ScopedNotificationString() { value.release_to(storage); }
};

[[noreturn]] void exit_notification_process(XLiveNotificationGameHost& host) {
    host.exit_process(0);
    std::terminate();
}
}

void drain_xlive_notifications_00a40110(OnlineSystemState& online,
    PlatformManagerFlags& flags, XLiveAcceptedInvite& accepted_invite,
    XLiveNotificationGlobals& globals, XLiveNotificationLibrary& library,
    XLiveNotificationGameHost& game, NativeStringStorage& strings) {
    if (reinterpret_cast<std::uintptr_t>(online.notification_listener) ==
        static_cast<std::uintptr_t>(-1)) {
        online.notification_listener = library.notify_create_listener(0x2full);
        // This validity member is host metadata; the native drain only uses
        // the actual handle and still calls GetNext after a failed creation.
        online.notification_listener_valid = online.notification_listener &&
            reinterpret_cast<std::uintptr_t>(online.notification_listener) !=
                static_cast<std::uintptr_t>(-1);
    }
    game.poll_signin_debounce_00a3f3e0();
    std::uint32_t id, parameter;
    while (library.notify_get_next(online.notification_listener, 0, id, parameter)) {
        switch (id) {
        case 0x09: {
            const bool visible = parameter != 0;
            const auto hook = globals.system_ui_hook;
            if (hook) hook(visible);
            flags.system_ui_visible = visible;
            break;
        }
        case 0x0a:
            game.refresh_signin_00a3f440();
            break;
        case 0x0b: {
            const auto hook = globals.storage_hook;
            if (hook) hook();
            break;
        }
        case 0x0e:
            game.profile_setting_changed_00a3e600(static_cast<std::uint8_t>(parameter));
            break;
        case 0x15: {
            ScopedNotificationString path(strings);
            game.title_update_path_00a3ff20(path.value);
            // Native compares against a zero-length temporary first. It does
            // not compare the path with "\\setup.exe" as older analysis said.
            if (path.value.length() != 0) {
                ScopedNotificationString suffix(strings);
                suffix.value.assign_0041e870(strings, "\\setup.exe");
                const auto old_length = path.value.length();
                path.value.resize_0041dd40(strings,
                    old_length + suffix.value.length(), true);
                std::memcpy(path.value.data() + old_length,
                    suffix.value.data(), suffix.value.length());
                suffix.value.release_to(strings);
                game.launch_update_00a3e560("update.exe",
                    path.value.data() ? path.value.data() : "");
                game.sleep_milliseconds(200);
                exit_notification_process(game);
            }
            break;
        }
        case 0x16: {
            ScopedNotificationString path(strings);
            game.system_update_path_00a3fde0(path.value);
            const auto wide = game.widen_update_path_004c5e60(
                path.value.data() ? path.value.data() : "");
            library.update_system(wide.c_str());
            exit_notification_process(game);
        }
        case 0x02000001:
            if (parameter == 0x001510f0)
                game.service_online_ui_00a3fa70(true);
            else if (parameter == 0x80151005)
                flags.link_failure = true;
            break;
        case 0x02000002: {
            const auto payload = library.invite_get_accepted_info(parameter);
            flags.invite_accepted = true;
            accepted_invite = payload;
            break;
        }
        case 0x02000007:
            flags.content_installed = true;
            break;
        case 0x04000002:
        case 0x04000003: {
            const auto callback = globals.online_client_friends_changed;
            if (callback) callback();
            break;
        }
        default:
            // Includes 02000003, whose entire native arm is the bare-RET log.
            break;
        }
    }
}
} // namespace bsp
