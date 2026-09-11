#include "bsp/xlive_system_pump.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t pending = 0x3e5;

void clear_first_five(XLiveOverlapped& overlapped) noexcept {
    std::fill_n(overlapped.words.begin(), 5, 0u);
}

std::wstring localized(XLiveSystemPumpContext& c, const char* text) {
    NativeString key;
    key.assign_0041e870(c.strings, text);
    try {
        auto result = c.host.resolve_localization_00a9fad0(key);
        key.release_to(c.strings);
        return result;
    } catch (...) {
        key.release_to(c.strings);
        throw;
    }
}

// Preserve the native x87 subtraction without an intermediate float spill.
// FCOMIP + JA rejects unordered values as well as equality at two seconds.
bool heartbeat_elapsed(float now, float previous) noexcept {
    const double threshold = 2.0; // double at 00D7A308
    unsigned char elapsed;
    __asm {
        fld now
        fsub previous
        fld threshold
        fxch st(1)
        fcomip st(0), st(1)
        fstp st(0)
        seta elapsed
    }
    return elapsed != 0;
}
} // namespace

void reset_online_ui_slots_00a3e700(OnlineSystemState& online) noexcept {
    online.state = 0;
    online.field_3b4 = 1;
    online.field_3b8 = 1;
}

void build_online_storage_path_00a3ed10(XLiveSystemPumpContext& c) {
    const auto user = c.online.signin_state_11c;
    if (c.host.read_manager_dword_8c_indexed(user) != 2) return;
    std::uint32_t path_bytes = 0x200;
    if (c.host.x_storage_build_server_path(user, 3, nullptr, 0, L"DropRates",
            c.pump.storage_path_154.data(), path_bytes) == 0) {
        c.pump.storage_state_12c = 1;
    }
}

void download_online_storage_00a3ed60(XLiveSystemPumpContext& c) {
    if (c.host.read_manager_dword_8c_indexed(c.online.field_3b4) != 2) return;
    auto& p = c.pump;
    if (p.storage_state_12c == 1 || p.storage_state_12c == 9 || p.storage_state_12c == 4) {
        p.storage_buffer_14c.reset();
        p.storage_buffer_bytes_150 = 0x100;
        p.storage_buffer_14c.reset(new std::uint8_t[p.storage_buffer_bytes_150]);
        std::memset(p.storage_buffer_14c.get(), 0, p.storage_buffer_bytes_150);
        clear_first_five(p.storage_overlapped_130);
        p.download_results_370.fill(0);
        p.storage_state_12c = 2;
    }
    if (p.storage_state_12c == 2) {
        const auto result = c.host.x_storage_download_to_memory(c.online.signin_state_11c,
            p.storage_path_154.data(), p.storage_buffer_bytes_150, p.storage_buffer_14c.get(),
            0x14, p.download_results_370, p.storage_overlapped_130);
        p.storage_result_354 = result;
        p.storage_state_12c = result == pending ? 3u : 5u;
    }
    if (p.storage_state_12c != 3) return;
    if (p.storage_overlapped_130.words[0] == pending) {
        std::uint32_t progress;
        c.host.x_storage_download_progress(p.storage_overlapped_130, progress, nullptr, nullptr);
        return;
    }
    if (c.host.x_get_overlapped_result(p.storage_overlapped_130, nullptr, true) == 0) {
        p.storage_state_12c = 4;
        if (p.download_results_370[0] == 9) {
            std::memcpy(&p.drop_value_358, p.storage_buffer_14c.get(), 4);
            std::memcpy(&p.drop_value_35c, p.storage_buffer_14c.get() + 4, 4);
        }
    } else {
        p.storage_state_12c = 5;
        const auto error = c.host.x_get_overlapped_extended_error(p.storage_overlapped_130);
        p.storage_result_354 = error;
        p.storage_state_12c = error == 0x8015c004u ? 10u : 5u;
    }
}

void upload_online_storage_00a3ef20(XLiveSystemPumpContext& c) {
    if (c.host.read_manager_dword_8c_indexed(c.online.field_3b4) != 2) return;
    auto& p = c.pump;
    if (p.storage_state_12c == 1 || p.storage_state_12c == 9 || p.storage_state_12c == 4 ||
        p.storage_state_12c == 10 || p.storage_state_12c == 5) {
        p.storage_buffer_14c.reset();
        p.storage_buffer_bytes_150 = 9;
        p.storage_buffer_14c.reset(new std::uint8_t[p.storage_buffer_bytes_150]);
        std::memcpy(p.storage_buffer_14c.get(), &p.drop_value_358, 4);
        std::memcpy(p.storage_buffer_14c.get() + 4, &p.drop_value_35c, 4);
        p.storage_buffer_14c[p.storage_buffer_bytes_150 - 1] = 0;
        clear_first_five(p.storage_overlapped_130);
        p.storage_state_12c = 6;
    }
    if (p.storage_state_12c == 6) {
        const auto result = c.host.x_storage_upload_from_memory(c.online.signin_state_11c,
            p.storage_path_154.data(), p.storage_buffer_bytes_150, p.storage_buffer_14c.get(),
            p.storage_overlapped_130);
        p.storage_result_354 = result;
        p.storage_state_12c = result == pending ? 7u : 8u;
    }
    if (p.storage_state_12c != 7) return;
    if (p.storage_overlapped_130.words[0] == pending) {
        std::uint32_t progress;
        c.host.x_storage_upload_progress(p.storage_overlapped_130, progress, nullptr, nullptr);
        return;
    }
    if (c.host.x_get_overlapped_result(p.storage_overlapped_130, nullptr, true) == 0) {
        p.storage_state_12c = 9;
    } else {
        p.storage_state_12c = 8;
        p.storage_result_354 = c.host.x_get_overlapped_extended_error(p.storage_overlapped_130);
    }
}

void pump_online_achievements_00a3fa70(XLiveSystemPumpContext& c, bool force) {
    auto& queue = c.online.pending_notifications; // native achievement-ID queue +360
    if (queue.empty()) return;
    auto& p = c.pump;
    if (force || (p.achievement_result_3a8 != pending && p.achievement_count_3a4 == 0 &&
                  p.achievements_overlapped_384.words[0] != pending)) {
        clear_first_five(p.achievements_overlapped_384);
        p.achievements_3a0.reset();
        // Native saturates an overflowing count*8 allocation to UINT32_MAX and
        // subsequently assumes allocation success. Such invalid-size states
        // are outside this projection's allocation domain.
        if (queue.size() > std::numeric_limits<std::uint32_t>::max() / 8u)
            throw std::length_error("native XLive achievement allocation exceeds DWORD size");
        p.achievement_count_3a4 = static_cast<std::uint32_t>(queue.size());
        p.achievements_3a0.reset(new XLiveAchievement[p.achievement_count_3a4]);
        for (std::uint32_t i = 0; i < p.achievement_count_3a4; ++i) {
            p.achievements_3a0[i].id = queue[i];
            p.achievements_3a0[i].user = c.online.signin_state_11c;
        }
        p.achievement_result_3a8 = c.host.x_user_write_achievements(
            p.achievement_count_3a4, p.achievements_3a0.get(), p.achievements_overlapped_384);
        return;
    }
    if (p.achievements_overlapped_384.words[0] == pending) {
        p.achievement_result_3a8 =
            c.host.x_get_overlapped_extended_error(p.achievements_overlapped_384);
        return;
    }
    if (c.host.x_get_overlapped_result(p.achievements_overlapped_384, nullptr, true) != 0)
        return;
    for (std::uint32_t i = 0; i < p.achievement_count_3a4; ++i) {
        const auto found = std::find(queue.begin(), queue.end(), p.achievements_3a0[i].id);
        if (found != queue.end()) queue.erase(found);
    }
    p.achievement_count_3a4 = 0;
    p.achievements_3a0.reset();
    p.achievement_result_3a8 = 0;
}

void pump_online_signin_ui_00a40510(XLiveSystemPumpContext& c) {
    auto& o = c.online;
    auto& p = c.pump;
    auto& flags = c.flags;
    auto& overlapped = flags.profile_overlapped_3c0;
    switch (o.state) {
    case 1: {
        if (flags.profile_changed) {
            reset_online_ui_slots_00a3e700(o);
            return;
        }
        const auto title = localized(c, "FE_xbox.xsm_SignIn_Title");
        const auto question = localized(c, "FE_xbox.xsm_SignIn_Question");
        const auto signin = localized(c, "FE_xbox.xsm_signin_signinuser_pc");
        const auto offline = localized(c, "FE_xbox.xsm_signin_continuewithoutsigningin_pc");
        overlapped.words.fill(0);
        const wchar_t* buttons[] = {signin.c_str(), offline.c_str()};
        if (!flags.system_ui_visible && c.host.x_show_message_box_ui(o.field_3b4,
                title.c_str(), question.c_str(), 2, buttons, 0, 1,
                p.message_choice_3dc, overlapped) == pending) {
            o.state = 2;
        }
        return;
    }
    case 2:
        if (overlapped.words[0] == pending) return;
        if (flags.profile_changed) {
            reset_online_ui_slots_00a3e700(o);
            return;
        }
        if (c.host.x_get_overlapped_result(overlapped, nullptr, true) != 0) {
            reset_online_signin_state(c.startup_host, o);
            return;
        }
        if (p.message_choice_3dc == 0) {
            o.state = 3;
        } else if (p.message_choice_3dc == 1) {
            o.field_3b8 = o.field_3b4;
            o.signin_flag_119 = false;
            o.signin_flag_11a = false;
            o.signin_state_11c = 1;
            p.signin_flag_120 = false;
            o.signin_slot_124 = -1;
            o.state = 5;
        }
        return;
    case 3:
        if (!flags.profile_changed) {
            if (flags.system_ui_visible || c.host.x_show_signin_ui(1, 0) != 0) return;
        }
        reset_online_signin_state(c.startup_host, o);
        return;
    case 4:
        if (flags.profile_changed) {
            reset_online_ui_slots_00a3e700(o);
            return;
        }
        o.field_3b8 = o.field_3b4;
        p.field_3e4 = 0;
        p.signin_flag_3bc = true;
        o.signin_flag_119 = true;
        o.signin_state_11c = o.field_3b4;
        p.signin_flag_120 = true;
        o.signin_slot_124 = 0;
        o.state = 5;
        return;
    case 5: {
        if (flags.profile_changed) {
            reset_online_ui_slots_00a3e700(o);
            return;
        }
        std::uint8_t info_flags;
        if (o.signin_flag_119 && c.host.x_user_get_signin_info_flags(
                o.signin_state_11c, 1, info_flags) == 0) {
            o.signin_flag_11a = (info_flags & 1) != 0;
            if (c.host.read_manager_dword_8c_indexed(o.signin_state_11c) == 2)
                build_online_storage_path_00a3ed10(c);
        } else {
            o.signin_flag_11a = false;
        }
        o.state = 6;
        return;
    }
    case 6:
        if (flags.profile_changed) {
            reset_online_ui_slots_00a3e700(o);
            return;
        }
        o.flag_3bd = false;
        p.field_28 = 2;
        o.state = 7;
        return;
    default:
        return;
    }
}

void pump_xlive_system_00a409f0(XLiveSystemPumpContext& c) {
    c.host.drain_notifications_00a40110();
    pump_online_signin_ui_00a40510(c);
    static_cast<void>(c.host.sample_clock_vslot20());
    if (c.pump.storage_state_12c == 7) upload_online_storage_00a3ef20(c);
    if (c.pump.storage_state_12c == 3) download_online_storage_00a3ed60(c);
    pump_online_achievements_00a3fa70(c, false);
    const float now = timestamp_seconds_x87(c.host.sample_clock_vslot20());
    auto& heartbeat = c.heartbeat;
    if (heartbeat.first_sample_e0e3ec) heartbeat.last_seconds_f8abfc = now;
    heartbeat.first_sample_e0e3ec = false;
    if (!heartbeat_elapsed(now, heartbeat.last_seconds_f8abfc)) return;
    bool ready = c.online.connected_flag != 0;
    if (c.online.signin_flag_119 && c.online.signin_state_11c == 0) ready = false;
    const auto callback = c.online.callback_20;
    if (callback != nullptr && ready) c.startup_host.invoke_state_callback(callback);
    heartbeat.last_seconds_f8abfc = now;
}

} // namespace bsp
