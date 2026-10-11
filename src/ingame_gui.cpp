// In-game GUI pieces of segments 26 and 27. Evidence: docs/CC13_INGAME_GUI.md.

#include "bsp/ingame_gui.hpp"

#include "bsp/gui_layer.hpp"

namespace bsp {

const SupportManagerSound* support_manager_sound_for_player(std::uint32_t player) noexcept {
    for (const SupportManagerSound& sound : kSupportManagerSounds) {
        if (sound.player == player) return &sound;
    }
    return nullptr;
}

void play_support_manager_sound(const SupportManagerSound& sound, SupportManagerSoundHost& host) {
    // e.g. 0065F18D..0065F1DD: the function-static class index.
    if (!host.class_cached(sound.guard_global)) {
        host.cache_class(sound.guard_global, sound.class_global,
            host.find_sound_class_00a7acf0(kSupportManagerSoundClass));
    }
    const std::int64_t type = host.find_sound_type_00a7b0a0(kSupportManagerSoundType);
    void* handle = host.create_tracked_sound_00a7e490(sound.sample_field,
        host.cached_class(sound.class_global), type, true);
    if (handle != nullptr) host.release_handle(handle);
}

std::string format_marker_distance_0063e5c0(float distance, bool nautical_miles) {
    // 0063E69A..0063E6AF: UCOMISS with 0.0f; equality skips the text.
    if (distance == 0.0f) return {};
    const float tenths = static_cast<float>(static_cast<double>(distance) /
        (nautical_miles ? kDistanceTenthNauticalMile : kDistanceTenthKilometre));
    // 0063E6E7..0063E72A: CVTTSS2SI, signed /10 and %10, x87 remainder test.
    const std::int32_t whole = static_cast<std::int32_t>(tenths);
    std::int32_t tens = whole / 10;
    std::int32_t digit = whole % 10;
    if (static_cast<double>(tenths) - whole > 0.5) ++digit;
    if (digit == 10) {
        ++tens;
        digit = 0;
    }
    std::string text = std::to_string(tens);
    text += '.';
    text += std::to_string(digit);
    text += " |";
    text += nautical_miles ? kDistanceMileKey : kDistanceKilometreKey;
    return text;
}

std::int32_t device_class_icon_index_0065ea30(std::string_view name) noexcept {
    const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c; };
    for (const DeviceClassIcon& entry : kDeviceClassIcons) {
        if (entry.name.size() != name.size()) continue;
        bool equal = true;
        for (std::size_t i = 0; i < name.size() && equal; ++i) equal = lower(entry.name[i]) == lower(name[i]);
        if (equal) return entry.index;
    }
    return kDeviceClassIconUnknown;  // 0065EE53..0065EE76, after the "%s" diagnostic
}

void rewards_screen_update_0062b640(RewardsScreenUpdateState& screen, RewardsScreenUpdateHost& host) {
    // 0062B646: the leaving byte goes straight to 0062B110.
    if (screen.leaving) {
        host.leave_0062b110();
        return;
    }
    // 0062B64C..0062B670: a pending storage flush counts down through zero.
    if (screen.storage_flush_countdown >= 0) {
        if (screen.storage_flush_countdown == 0) host.flush_storage_007fa1b0(screen.storage_slot);
        --screen.storage_flush_countdown;
    }
    // 0062B674..0062B6A6: FCOMI of the double 0.03 with the frame time; the
    // result replaces the seconds argument in its stack slot.
    const float frame = host.game_frame_seconds_21ec();
    const float step = frame < kRewardsMaxStep ? frame : kRewardsMaxStep;

    if (host.action_004d92b0(kRewardsActionLeave)) {
        host.leave_0062b110();
        return;
    }
    if (screen.input_locked) return;  // 0062B6C3

    // 0062B6D8..0062B70B: the action is read before the page test.
    if (host.action_004d92b0(kRewardsActionGamerCard) && screen.page == 1 &&
        host.gamer_card_available_0061d8b0()) {
        const std::uint64_t xuid = host.shown_player_xuid_0061d710();
        host.show_gamer_card_00a4d440(host.active_user_slot_00a3eac0(), xuid);
    }

    if (screen.page == 1) {
        // 0062B718..0062B783: review anyone but the local player.
        if (host.session_virtual_198() && host.action_004d92b0(kRewardsActionPlayerReview) &&
            host.local_player_xuid_00a3eb00() != host.shown_player_xuid_0061d710()) {
            const std::uint64_t xuid = host.shown_player_xuid_0061d710();
            host.show_player_review_00a4d470(host.active_user_slot_00a3eac0(), xuid);
        }
    } else if (screen.page == 4) {
        // 0062B791..0062B815: UCOMISS/LAHF/JNP, so NaN also grows. The width
        // moves toward the target by target * step * 0.25 (x87, stored as
        // float) and snaps to it, clearing the target, once it gets there.
        if (screen.rank_grow_target != 0.0f) {
            GuiSize2 size = host.rank_size_00aa6740();
            const float target = screen.rank_grow_target;
            size.x = static_cast<float>(static_cast<double>(target) * step * kRewardsRankGrowRate + size.x);
            if (size.x >= target) {
                size.x = target;
                screen.rank_grow_target = 0.0f;
            }
            host.rank_set_size_58(size);
        }
        // 0062B817..0062B8AC: the alpha bounces between 0 and 1. The direction
        // flips when the current alpha is at or beyond either end, then moves
        // by one step and is clamped through 004155B0.
        const float alpha = host.rank_alpha_54();
        if (alpha >= kRewardsRankAlphaMax || alpha <= 0.0f) screen.rank_alpha_rising = !screen.rank_alpha_rising;
        const float moved = screen.rank_alpha_rising ? alpha + step : alpha - step;
        host.rank_set_alpha_4c(clamp_pointer_004155b0(moved, {0.0f, kRewardsRankAlphaMax}));
    }

    host.scroller_update_00683820(0x68);
    host.scroller_update_00683820(0x128);
    host.scroller_update_00683820(0x19C);

    // 0062B8D2..0062B92C: page arrows flash for 0.1 s.
    bool turned = false;
    if (host.action_004d92b0(kRewardsActionPagePrevious)) {
        host.change_page_00622e30(-1);
        host.arrow_set_state_88(RewardsArrow::Previous, 2);
        turned = true;
    } else if (host.action_004d92b0(kRewardsActionPageNext)) {
        host.change_page_00622e30(1);
        host.arrow_set_state_88(RewardsArrow::Next, 2);
        turned = true;
    }
    if (turned) screen.arrow_flash = kRewardsArrowFlashSeconds;

    // 0062B931..0062B994: the countdown is stored before the <= 0 test.
    if (screen.arrow_flash != 0.0f) {
        screen.arrow_flash -= step;
        if (screen.arrow_flash <= 0.0f) {
            screen.arrow_flash = 0.0f;
            host.arrow_set_state_88(RewardsArrow::Previous, 0);
            host.arrow_set_state_88(RewardsArrow::Next, 0);
        }
    }
}

} // namespace bsp