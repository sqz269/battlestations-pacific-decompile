// cc15 lane L2: the profile block game+650h with its +64h MissionProgress, and
// the hint tracker singleton [00E198D0]. Evidence and uncertainty:
// docs/CC15_OBJECT_PROFILE_HINTS.md. Both objects reuse the existing typed
// bodies (reset_profile_007fdb20, MissionProgressOwner,
// hint_tracker_rebuild_history_00690cd0); nothing here re-derives them.

#include "bsp/game_profile_owner.hpp"

#include "bsp/game_hosts.hpp"

namespace bsp::game {

// ---------------------------------------------------------------------------
// game+650h
// ---------------------------------------------------------------------------

GameProfileOwner::GameProfileOwner(GameHostLog& log) : log_(log) {}

void GameProfileOwner::reset_to_defaults_007fdb20() {
    // The image hands 007FDB20 the live settings object; the host keeps the
    // settings phase 5 loaded and lets only the profile half take effect.
    discarded_settings_ = GameSettingsBlock{};
    reset_profile_007fdb20(profile_, discarded_settings_, *this);
    ++resets_;
}

MissionProgress* GameProfileOwner::mission_progress() noexcept {
    return profile_.mission_progress_present ? &progress_.get() : nullptr;
}

void GameProfileOwner::destroy_mission_progress_007fd780() {
    progress_.destroy_mission_progress_007fd780();
}

bool GameProfileOwner::construct_mission_progress_00920e10() {
    // 007FDC11 PUSH 24h, operator new, 00920E10, stored at 007FDC41.
    return progress_.construct_mission_progress_00920e10();
}

bool GameProfileOwner::xenon_state_is_two() {
    // XenonSystemManager [00F8ABE8] is not built in single player; the answer
    // only gates the settings half, which this owner discards.
    log_.unimplemented("ProfileReset::xenon_state_is_two", "00f8abe8");
    return false;
}

bool GameProfileOwner::xenon_user_selected() {
    log_.unimplemented("ProfileReset::xenon_user_selected", "00f8abe8");
    return false;
}

void GameProfileOwner::restore_selected_user_controls_008d45d0() {
    log_.unimplemented("ProfileReset::restore_selected_user_controls", "008d45d0");
}

// ---------------------------------------------------------------------------
// [00E198D0]
// ---------------------------------------------------------------------------

GameHintTracker::GameHintTracker(GameHostLog& log, GameProfileOwner& profile)
    : log_(log), profile_(profile) {}

void GameHintTracker::clear_hint_queue_0068ea00() {
    mission_seen_48_.clear();   // 004D05E0 on +48h
    queue_68_.clear();          // 0068E780 until +78h == 0
}

void GameHintTracker::rebuild_history_00690cd0() {
    hint_tracker_rebuild_history_00690cd0(*this);
}

const HintDefinition* GameHintTracker::find_definition_0068e450(const std::string& name) {
    const auto found = definitions_30_.find(name);
    return found != definitions_30_.end() ? &found->second : nullptr;
}

// The remaining HintStoreHost hooks belong to routines this lane does not wire
// (00690B20, 00690C30, 0068EE00, 00690FD0); reaching one is recorded.
bool GameHintTracker::has_been_shown_0068e600(const std::string&) {
    log_.unimplemented("HintTracker::has_been_shown", "0068e600");
    return false;
}

bool GameHintTracker::profile_has_seen_007f8e00(const std::string&) {
    log_.unimplemented("HintTracker::profile_has_seen", "007f8e00");
    return false;
}

void GameHintTracker::profile_mark_seen_007fae20(const std::string&) {
    log_.unimplemented("HintTracker::profile_mark_seen", "007fae20");
}

void GameHintTracker::profile_unmark_007fae60(const std::string&) {
    log_.unimplemented("HintTracker::profile_unmark", "007fae60");
}

void GameHintTracker::mission_seen_push_back_48(const std::string& name) {
    mission_seen_48_.push_back(name);   // 004D2660 on +48h
}

bool GameHintTracker::player_unit_present_e188d8() {
    log_.unimplemented("HintTracker::player_unit_present", "00e188d8");
    return false;
}

std::int32_t GameHintTracker::player_unit_class_170() {
    log_.unimplemented("HintTracker::player_unit_class", "00e188d8+170h");
    return 0;
}

bool GameHintTracker::history94_has_key(const std::string& key) {
    return history_94_.find(key) != history_94_.end();
}

void GameHintTracker::history94_push_back(const std::string& key, const std::string& name) {
    history_94_[key].push_back(name);
}

void GameHintTracker::history94_assign_single(const std::string& key, const std::string& name) {
    history_94_[key] = std::vector<std::string>{name};
}

std::vector<std::string> GameHintTracker::profile_shown_names_65c() {
    // *(game+65Ch) is the head of the profile's list at +08h (+0Ch head).
    return profile_.profile().string_list_08;
}

GameHintTracker& GameHintTrackerSlot::get_004e1ca0() {
    if (!tracker_) {
        tracker_ = std::make_unique<GameHintTracker>(log_, profile_);
        log_.implemented("HintTracker::construct", "004e18e0");
    }
    return *tracker_;
}

} // namespace bsp::game
