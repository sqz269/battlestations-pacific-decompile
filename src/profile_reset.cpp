#include "bsp/profile_reset.hpp"

#include <algorithm>
#include <utility>

namespace bsp {

void reset_profile_007fdb20(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileResetHost& host)
{
    profile.save_name_34.clear();
    profile.player_name_3c = "globals.newplayer";
    profile.display_name_50.clear();
    profile.voice_59 = true;
    profile.selected_difficulty_60 = 1;
    profile.difficulty_5c = 1;

    if (profile.mission_progress_present) {
        host.destroy_mission_progress_007fd780();
        profile.mission_progress_present = false;
    }
    profile.unlock_state.mission_completion.clear();
    profile.mission_progress_present = host.construct_mission_progress_00920e10();
    profile.selected_mission_id_68.clear();
    profile.unlock_state.unlocks.clear();
    profile.unlock_state.pending_unlocks.clear();
    profile.seen_unlocks_88.clear();
    profile.unlock_state.named_counters.clear();
    profile.bonus_ac.clear();
    profile.transient_records_94.clear();
    profile.japan_nose_art_e0 = 1;
    profile.allied_nose_art_e4 = 1;
    // Native repeats this clear at 007fdd89, then obtains operator[] at +A0h.
    profile.unlock_state.named_counters.clear();
    profile.unlock_state.named_counters["RANK"] = 1;

    profile.string_list_08.clear();
    profile.lobby_filters.player_count_20 = 0;
    profile.lobby_filters.free_slots_24 = 0;
    profile.lobby_filters.latency_28 = 0;
    profile.lobby_filters.array_size_2c = 9;
    profile.lobby_filters.entries_14.clear();
    for (int index = 0; index < 9; ++index) {
        profile.lobby_filters.entries_14.push_back(index < 5 ? 1 : 0);
    }
    profile.score_total_30 = -1;
    profile.drop_rate_dc_f0 = 0;
    profile.drop_rate_tc_ec = 0;
    profile.unlock_state.content_ids.clear();
    profile.content_mask_d8 = 0;

    // Reuse the established settings reset. The native writes controls before
    // asking the platform about state/user, and short-circuits the second query.
    reset_control_defaults_008d4820(settings, false, false);
    if (host.xenon_state_is_two() && host.xenon_user_selected()) {
        host.restore_selected_user_controls_008d45d0();
    }
    reset_game_defaults_008d41c0(settings);
}

namespace {
void mirror_profile_name(const ProfileResetState& profile, std::array<char, 32>& game_name)
{
    // Both native setters select +50h when its header length is nonzero,
    // otherwise +3Ch, regardless of which field was just assigned.
    const auto& selected = profile.display_name_50.empty()
        ? profile.player_name_3c : profile.display_name_50;
    const auto count = std::min({selected.size(), selected.find('\0'), std::size_t{31}});
    std::copy_n(selected.data(), count, game_name.data());
    game_name[count] = '\0';
}

bool saved_names_differ(std::string_view previous, std::string_view next)
{
    // 00449AF0 checks header lengths first, then __stricmp sees only each
    // null-terminated prefix. A nonempty header beginning with NUL still
    // differs from an empty header even though both C strings look empty.
    if (previous.empty()) return !next.empty();
    if (next.empty()) return true;
    previous = previous.substr(0, previous.find('\0'));
    next = next.substr(0, next.find('\0'));
    const NativeStringCaseInsensitiveLess less;
    return less(previous, next) || less(next, previous);
}
} // namespace

void set_profile_name_007f9290(
    ProfileResetState& profile, std::array<char, 32>& game_name, std::string_view name)
{
    profile.player_name_3c = name;
    mirror_profile_name(profile, game_name);
}

void set_profile_display_name_007f9340(
    ProfileResetState& profile, std::array<char, 32>& game_name, std::string_view name)
{
    profile.display_name_50 = name;
    mirror_profile_name(profile, game_name);
}

void request_profile_read_007ff100(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileIoState& io,
    ProfileIoHost& host, std::string_view name, ProfileCompletion completion)
{
    profile.save_name_34 = name;
    if (host.storage_query_1c(profile.save_name_34, true)) {
        io.completion = std::move(completion);
        host.request_read_00bd3d70(profile.save_name_34, true);
        host.run_storage_operation_006adb50(ProfileIoTask::ReadCompleted);
        return;
    }
    reset_profile_007fdb20(profile, settings, host);
    if (completion) {
        completion();
    }
}

void complete_profile_read_007fefe0(
    ProfileResetState& profile, GameSettingsBlock& settings, ProfileIoState& io,
    ProfileIoHost& host)
{
    if (host.storage_state_08() == 1) {
        reset_profile_007fdb20(profile, settings, host);
        auto completion = std::exchange(io.completion, {});
        if (completion) {
            completion();
        }
        return;
    }
    host.begin_profile_reader_004425c0();
    host.deserialize_profile_007fdf00(profile);
    if (host.has_storage_buffer_30()) {
        host.free_and_clear_storage_buffer_30();
    }
    host.close_storage_archive_00b65e80();
    if (host.has_manager_00f8a2fc()) {
        host.notify_manager_virtual_a0();
    }
    auto completion = std::exchange(io.completion, {});
    host.restore_settings_008d7a50(std::move(completion));
    host.apply_settings_008d5b50();
    host.commit_profile_007fae70(profile);
    host.destroy_profile_reader_00441a20();
}

void request_profile_write_007fa710(
    ProfileResetState& profile, ProfileIoState& io, ProfileIoHost& host,
    std::string_view name, ProfileCompletion completion, bool force)
{
    const bool should_queue = force || saved_names_differ(profile.save_name_34, name);
    profile.save_name_34 = name;
    io.completion = std::move(completion);
    if (should_queue) {
        host.request_write_00bd3dc0(profile.save_name_34);
        host.run_storage_operation_006adb50(ProfileIoTask::WriteCompleted);
    }
}

} // namespace bsp
