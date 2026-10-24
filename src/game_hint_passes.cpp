// cc15 packet cc15-hint2: the hint tracker's per-frame callers (the seven
// OnMove passes, TriggerHint 00690FD0) and the hints screen 34h that shows,
// times and finishes a hint. Evidence, original ABI and every labelled
// substitution: docs/CC15_OBJECT_PROFILE_HINTS.md section 6. The decisions are
// the existing typed bodies (hint_tracker_trigger_00690fd0, hud_hints_*,
// award_trackers.cpp helpers); this file is their host projection.

#include "bsp/game_hint_passes.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_profile_owner.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace bsp::game {
namespace {

// _stricmp over the two NativeStrings (00425850 / 00435C40 / 00449AF0).
bool equals_ci(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i]))
            != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

bool contains_ci(const std::vector<std::string>& list, std::string_view name) {
    return std::any_of(list.begin(), list.end(),
        [name](const std::string& entry) { return equals_ci(entry, name); });
}

// Class ids 0068EA00..00692FD0 name, as literals in the image.
constexpr const char* kBasicSub = "BASICSUB";        // 00CEE384
constexpr const char* kBasicPlane = "BASICPLANE";    // 00CEE3C0
constexpr const char* kBasicShip = "BASICSHIP";      // 00CEE39C

// [00E198C4]+68h is screen 3Eh (plane), +78h screen 45h (ship); +E4h screen 19h.
constexpr int kPlaneScreenSlot = 0x3E;
constexpr int kShipScreenSlot = 0x45;
constexpr int kPauseMultiScreenSlot = 0x19;

// Scripts/datatables/DialogGlobals.lua in this installation (mtime 2024-07-13):
// TempCharWait 0.04 and TempWaitBase 1.0, which 005BB130 stores at the
// narrative screen's +7Ch and +80h (docs/MISSION_END.md).
constexpr float kTempCharWait = 0.04F;
constexpr float kTempWaitBase = 1.0F;

}  // namespace

// ---------------------------------------------------------------------------
// Screen 34h
// ---------------------------------------------------------------------------

GameHintsScreen::GameHintsScreen(GameHostLog& log, GameHintTracker& tracker)
    : log_(log), tracker_(tracker) {}

void GameHintsScreen::register_0054d860() {
    hud_hints_register_0054d860(state_, *this);
    log_.implemented("HintsScreen::register", "0054d860");
}

void GameHintsScreen::enter_0054e380() {
    hud_hints_enter_0054e380(state_, *this);
    log_.implemented("HintsScreen::enter", "0054e380");
}

void GameHintsScreen::exit_0054e3d0() {
    hud_hints_exit_0054e3d0(state_, *this);
    log_.implemented("HintsScreen::exit", "0054e3d0");
}

void GameHintsScreen::update_0054e6b0(float dt) {
    hud_hints_update_0054e6b0(state_, *this, dt);
    log_.implemented("HintsScreen::update", "0054e6b0");
}

void GameHintsScreen::show_0054e020(bool pausing, const std::string& text, const std::string& title,
                                    std::int32_t seconds) {
    hud_hints_show_0054e020(state_, *this, pausing, text, title, seconds);
    log_.implemented("HintsScreen::show", "0054e020");
}

void GameHintsScreen::dismiss_0054e610() {
    hud_hints_dismiss_0054e610(state_, *this);
    log_.implemented("HintsScreen::dismiss", "0054e610");
}

std::int32_t GameHintsScreen::seconds_left_0054d3c0() const {
    return static_cast<std::int32_t>(state_.remaining_54);  // FLD +54h, _ftol
}

GameHintsScreen::Widget* GameHintsScreen::widget(std::uintptr_t handle) {
    const auto found = widgets_.find(handle);
    return found != widgets_.end() ? &found->second : nullptr;
}

std::uintptr_t GameHintsScreen::load_page_00aa5840(std::string_view name) {
    return find_child_00aa7e00(0, name);
}

// Handles are stable per (parent, name); the page and its eleven widgets are
// GUI_hints' own names (docs/HUD_SCREEN_PAGES.md, 11 widgets bound).
std::uintptr_t GameHintsScreen::find_child_00aa7e00(std::uintptr_t parent, std::string_view name) {
    const std::string key = std::to_string(parent) + "/" + std::string(name);
    const auto found = handles_.find(key);
    if (found != handles_.end()) return found->second;
    const std::uintptr_t handle = handles_.size() + 1;
    handles_.emplace(key, handle);
    widgets_.emplace(handle, Widget{});
    return handle;
}

void GameHintsScreen::screen_layout_14() { hud_hints_layout_0054d980(state_, *this); }

std::array<float, 2> GameHintsScreen::widget_size_00aa6740(std::uintptr_t handle) {
    const Widget* w = widget(handle);
    return w != nullptr ? w->size : std::array<float, 2>{};
}

std::array<float, 3> GameHintsScreen::widget_local_position_00aa67f0(std::uintptr_t handle) {
    const Widget* w = widget(handle);
    return w != nullptr ? w->position : std::array<float, 3>{};
}

void GameHintsScreen::widget_set_local_xy_00aa7d00(std::uintptr_t handle, float x, float y) {
    if (Widget* w = widget(handle)) {
        w->position[0] = x;
        w->position[1] = y;
    }
}

void GameHintsScreen::widget_set_size_58(std::uintptr_t handle, const std::array<float, 2>& size) {
    if (Widget* w = widget(handle)) w->size = size;
}

void GameHintsScreen::widget_show_34(std::uintptr_t handle, bool shown) {
    if (Widget* w = widget(handle)) w->shown = shown;
}

bool GameHintsScreen::widget_shown_38(std::uintptr_t handle) {
    const Widget* w = widget(handle);
    return w != nullptr && w->shown;
}

void GameHintsScreen::text_set_source_00abaed0(std::uintptr_t handle, const std::string& key) {
    if (Widget* w = widget(handle)) w->text_key = key;
}

// SUBSTITUTION (labelled): +ECh is the text widget's character count after
// localisation (docs/MISSION_END.md). Here: the locale table entry's length,
// with '#' context tokens unexpanded, or the key's length when the table has
// no entry (00A9F4B0's fallback widens the key).
std::int32_t GameHintsScreen::text_line_count_ec(std::uintptr_t handle) {
    const Widget* w = widget(handle);
    if (w == nullptr) return 0;
    HintScreenRegistry* registry = tracker_.registry_;
    const std::u16string* text = registry != nullptr ? registry->locale_text(w->text_key) : nullptr;
    return static_cast<std::int32_t>(text != nullptr ? text->size() : w->text_key.size());
}

float GameHintsScreen::subtitle_seconds_per_line_7c() { return kTempCharWait; }
float GameHintsScreen::subtitle_seconds_base_80() { return kTempWaitBase; }

bool GameHintsScreen::pad_prompts_00f88a30() { return tracker_.pad_prompts_00f88a30(); }

bool GameHintsScreen::game_cinematic_hidden_634() {
    HintFrameQueries* frame = tracker_.frame_;
    return frame != nullptr && frame->game_cinematic_hidden_634();
}

std::int32_t GameHintsScreen::game_effective_mode_004bca50() {
    HintFrameQueries* frame = tracker_.frame_;
    return frame != nullptr ? frame->effective_game_mode_004bca50() : 0;
}

// 004D94F0: when game+620h changes (and +624h is 0) it is stored and every
// screen but 28h, 55h, 57h, 5Dh and 34h has its widgets parked (004D6480).
// The flag is kept; the parking is GUI-only and recorded.
void GameHintsScreen::game_hint_pause_004d94f0(bool paused) {
    if (hint_paused_ == paused) return;
    hint_paused_ = paused;
    log_.unimplemented("HintsScreen::park_gui_screens", "004d6480");
    const double t = tracker_.mission_seconds();
    log_.notef("hint pause t=%.2f %d (game+620h; GUI screens parked, simulation untouched)", t,
               paused ? 1 : 0);
}

void GameHintsScreen::game_set_cinematic_004cd0f0(bool hide, bool allow_simulation, bool p4) {
    HintFrameQueries* frame = tracker_.frame_;
    if (frame == nullptr) {
        log_.unimplemented("HintsScreen::set_cinematic", "004cd0f0");
        return;
    }
    log_.notef("hint cinematic t=%.2f hide=%d allow_simulation=%d", tracker_.mission_seconds(),
               hide ? 1 : 0, allow_simulation ? 1 : 0);
    frame->game_set_cinematic_004cd0f0(hide, allow_simulation, p4);
}

void GameHintsScreen::game_set_gui_layers_visible_004d9830(bool) {
    log_.unimplemented("HintsScreen::gui_layers_visible", "004d9830");
}

bool GameHintsScreen::input_action_pressed_00535ee0(std::int32_t action) {
    HintScreenRegistry* registry = tracker_.registry_;
    return registry != nullptr && registry->input_action_pressed_00535ee0(action);
}

void GameHintsScreen::input_action_reset_00a91e20(std::int32_t) {
    log_.unimplemented("HintsScreen::input_action_reset", "00a91e20");
}

void GameHintsScreen::input_context_level_00a933f0(std::int32_t, std::int32_t) {
    log_.unimplemented("HintsScreen::input_context_level", "00a933f0");
}

void GameHintsScreen::input_context_level5_004c5140(std::int32_t) {
    log_.unimplemented("HintsScreen::input_context_level5", "004c5140");
}

// 00427190 is the autotest step (BSP_AutoTest_Step); the driver is off in a
// normal run, so its answer is 0.
bool GameHintsScreen::profile_hints_query_00427190(std::int32_t) { return false; }

std::string GameHintsScreen::tracker_active_hint_0068e5b0() { return tracker_.active_hint_60(); }
void GameHintsScreen::tracker_finish_active_0068e420() { tracker_.finish_active_0068e420(); }
void GameHintsScreen::tracker_clear_queue_0068ea00() { tracker_.clear_hint_queue_0068ea00(); }
void GameHintsScreen::tracker_clear_list_48_004d05e0() { tracker_.clear_list_48_004d05e0(); }
void GameHintsScreen::tracker_clear_active_name_1c() { tracker_.clear_last_timed_1c(); }
void GameHintsScreen::tracker_trigger_hint_00690fd0(const std::string& name, bool forced) {
    tracker_.trigger_hint_00690fd0(name, forced);
}

void GameHintsScreen::cue_start_00a7e490(std::int32_t, std::string_view, std::string_view) {
    log_.unimplemented("HintsScreen::cue_start", "00a7e490");
}

// ---------------------------------------------------------------------------
// The tracker: TriggerHint and its reads
// ---------------------------------------------------------------------------

bool game_hint_pause_620_cc15() noexcept {
    GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot();
    const GameHintTracker* tracker = slot != nullptr ? slot->built() : nullptr;
    const GameHintsScreen* screen = tracker != nullptr ? tracker->hints_screen_34_if_built() : nullptr;
    return screen != nullptr && screen->hint_paused_620();
}

bool hints_seconds_left_0054d3c0_cc15(std::int32_t& seconds_left) noexcept {
    GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot();
    GameHintTracker* tracker = slot != nullptr ? slot->built() : nullptr;
    if (tracker == nullptr) return false;
    seconds_left = tracker->hints_screen_34().seconds_left_0054d3c0();
    return true;
}

GameHintsScreen& GameHintTracker::hints_screen_34() {
    if (!screen_34_) screen_34_ = std::make_unique<GameHintsScreen>(log_, *this);
    return *screen_34_;
}

double GameHintTracker::mission_seconds() const {
    return frame_ != nullptr ? frame_->mission_seconds() : -1.0;
}

void GameHintTracker::trigger_hint_00690fd0(const std::string& name, bool forced) {
    TriggerCount& count = trigger_counts_[name];
    ++count.calls;
    const std::size_t shown_before = count.shown;
    const std::string active_before = state_.active_60;
    const std::string outer = triggering_;
    triggering_ = name;
    hint_tracker_trigger_00690fd0(state_, *this, name, forced);
    triggering_ = outer;
    // A hidden definition becomes the active hint without 0054E020.
    if (count.shown == shown_before && !name.empty() && equals_ci(state_.active_60, name)
        && state_.active_60 != active_before) {
        ++count.hidden;
        log_.notef("hint hidden t=%.2f %s", mission_seconds(), name.c_str());
    }
    log_.implemented("HintTracker::trigger_hint", "00690fd0");
}

// Run log only: a TriggerHint gate that refused the call in progress. The
// first refusal per name and reason is logged with its time, the rest counted.
void GameHintTracker::note_refused(const std::string& name, const char* reason) {
    if (name.empty() || !equals_ci(triggering_, name)) return;
    TriggerCount& count = trigger_counts_[triggering_];
    ++count.refused;
    if (count.first_refusal.empty()) {
        count.first_refusal = reason;
        log_.notef("hint refused t=%.2f %s (%s)", mission_seconds(), name.c_str(), reason);
    }
}

void GameHintTracker::finish_active_0068e420() {
    if (!state_.active_60.empty()) {
        log_.notef("hint finish t=%.2f %s", mission_seconds(), state_.active_60.c_str());
    }
    state_.active_60.clear();
}

bool GameHintTracker::has_been_shown_cc15_0068e600(const std::string& name) {
    // 0068E600: 007F8E00 on game+650h first, then the list +48h.
    if (profile_has_seen_cc15_007f8e00(name)) return true;
    return contains_ci(mission_seen_48_, name);
}

bool GameHintTracker::profile_has_seen_cc15_007f8e00(const std::string& name) {
    const bool seen = contains_ci(profile_.profile().string_list_08, name);
    if (seen) note_refused(name, "profile seen, Unique");
    return seen;
}

// 007FAE20 removes the name case-insensitively (005FE8B0) and appends a copy
// to the profile's list (AllSeenHints). The host keeps it in memory only:
// nothing in this process writes a profile back.
void GameHintTracker::profile_mark_seen_cc15_007fae20(const std::string& name) {
    std::vector<std::string>& seen = profile_.profile().string_list_08;
    seen.erase(std::remove_if(seen.begin(), seen.end(),
                   [&name](const std::string& entry) { return equals_ci(entry, name); }),
               seen.end());
    seen.push_back(name);
    log_.notef("hint profile seen += %s t=%.2f (in memory only)", name.c_str(), mission_seconds());
}

std::int32_t GameHintTracker::hints_setting_00f8898c() {
    // SUBSTITUTION (labelled) without a mission frame: 008D41C0's default 2.
    return frame_ != nullptr ? frame_->hints_setting_00f8898c() : 2;
}

bool GameHintTracker::hud_screen_e4_busy() {
    return registry_ != nullptr && registry_->screen_wanted_04(kPauseMultiScreenSlot);
}

void GameHintTracker::queue_push_back_0068eb90(const std::string& name) {
    queue_68_.push_back(name);
    ++trigger_counts_[name].queued;
    log_.notef("hint queued t=%.2f %s (queue %zu)", mission_seconds(), name.c_str(), queue_68_.size());
}

bool GameHintTracker::mission_seen_contains_48(const std::string& name) {
    const bool seen = contains_ci(mission_seen_48_, name);
    if (seen) note_refused(name, "mission seen, MissionUnique");
    return seen;
}

void GameHintTracker::touch_recent_0068ea70(const std::string& name) {
    // 0068EA70: erase the matching entry (same length, _stricmp), append last.
    const auto found = std::find_if(recent_54_.begin(), recent_54_.end(),
        [&name](const std::string& entry) { return equals_ci(entry, name); });
    if (found != recent_54_.end()) recent_54_.erase(found);
    recent_54_.push_back(name);
}

bool GameHintTracker::cooldown_contains_0068e4c0(const std::string& name) {
    const bool cooling = cooldowns_3c_.find(name) != cooldowns_3c_.end();
    if (cooling) note_refused(name, "cooldown");
    return cooling;
}

void GameHintTracker::cooldown_set_00444be0(const std::string& name, float seconds) {
    cooldowns_3c_[name] = seconds;
}

// [00F88A30]: the pad-prompt setting. SUBSTITUTION (labelled): false, the PC
// keyboard prompts this host's input always uses.
bool GameHintTracker::pad_prompts_00f88a30() { return false; }

bool GameHintTracker::localised_key_exists(const std::string& key) {
    return registry_ != nullptr && registry_->locale_text(key) != nullptr;
}

std::int32_t GameHintTracker::local_player_slot_18ec() {
    return frame_ != nullptr ? frame_->local_player_slot_18ec() : 0;
}

std::int32_t GameHintTracker::local_side_nation_28() {
    return frame_ != nullptr ? frame_->local_side_nation_28() : 0;
}

std::int32_t GameHintTracker::hints_seconds_left_0054d3c0() {
    return hints_screen_34().seconds_left_0054d3c0();
}

void GameHintTracker::hints_dismiss_0054e610() { hints_screen_34().dismiss_0054e610(); }

void GameHintTracker::hints_show_0054e020(bool pausing, const std::string& text, const std::string& title,
                                          std::int32_t seconds) {
    ++trigger_counts_[triggering_].shown;
    GameHintsScreen& screen = hints_screen_34();
    screen.show_0054e020(pausing, text, title, seconds);
    log_.notef("hint show t=%.2f %s pausing=%d duration=%d seconds=%.2f text=%s title=%s",
               mission_seconds(), triggering_.c_str(), pausing ? 1 : 0, seconds,
               static_cast<double>(screen.state().remaining_54), text.c_str(), title.c_str());
}

// SUBSTITUTION (labelled): the mission Lua state has no host entry for
// 006B8AD0 here; every LuaCommand of this installation's definitions is
// luaSetNarrativeParam / luaSetNarrativeSticks (docs/CC13_HINT_WAITS.md), so
// the call is recorded and answers success.
std::int32_t GameHintTracker::lua_run_string_006b8ad0(const std::string& command) {
    if (frame_ != nullptr) return frame_->lua_run_string_006b8ad0(command);
    log_.unimplemented("HintTracker::lua_run_string", "006b8ad0");
    return 0;
}

void GameHintTracker::diagnostic_lua_result_004254b0(bool failed, const std::string& command) {
    log_.notef("hint lua t=%.2f %s failed=%d", mission_seconds(), command.c_str(), failed ? 1 : 0);
}

bool GameHintTracker::history94_erase_name(const std::string& key, const std::string& name) {
    auto found = history_94_.find(key);
    if (found == history_94_.end()) return false;
    std::vector<std::string>& names = found->second;
    const auto entry = std::find_if(names.begin(), names.end(),
        [&name](const std::string& n) { return equals_ci(n, name); });
    if (entry == names.end()) return false;
    names.erase(entry);
    return true;
}

void GameHintTracker::history7c_push_back(const std::string& key, const std::string& name) {
    history_7c_[key].push_back(name);
}

void GameHintTracker::history_assign_single(const std::string& key, const std::string& name) {
    history_94_[key] = std::vector<std::string>{name};
    history_7c_[key] = std::vector<std::string>{name};
}

// ---------------------------------------------------------------------------
// The seven passes, 004E525E..004E52BA
// ---------------------------------------------------------------------------

// 0068EC10 __thiscall(tracker, float raw_delta), RET 4: every entry loses the
// delta; those strictly below zero are collected, then erased.
void GameHintTracker::tick_cooldowns_0068ec10(float raw_delta) {
    std::vector<std::string> expired;
    for (auto& [name, remaining] : cooldowns_3c_) {
        remaining = remaining - raw_delta;   // 0068ECA6..0068ECBC
        if (remaining < 0.0F) expired.push_back(name);
    }
    for (const std::string& name : expired) cooldowns_3c_.erase(name);
}

// 00692B00 __fastcall(tracker), RET: front() to TriggerHint(forced 1), pop.
void GameHintTracker::drain_queued_00692b00() {
    if (frame_ == nullptr) return;
    QueuedHintDrainInputs in{};
    in.game_mode = frame_->effective_game_mode_004bca50();
    if (in.game_mode != kAwardTrackerSuppressedGameMode) {
        in.transition_blend_active = frame_->narrative_blend_005b5d50();
    }
    in.queue_size = queue_68_.size();
    in.hint_active = !state_.active_60.empty();
    if (!should_drain_queued_hint_00692b00(in)) return;
    const std::string front = queue_68_.front();   // 0068E730
    log_.notef("hint dequeued t=%.2f %s", mission_seconds(), front.c_str());
    trigger_hint_00690fd0(front, true);             // 00692B37 push 1
    queue_68_.pop_front();                          // 0068E780
}

// 00692B60 __fastcall(tracker), RET.
void GameHintTracker::update_unit_class_00692b60() {
    if (frame_ == nullptr) return;
    HintFrameQueries& q = *frame_;
    if (q.effective_game_mode_004bca50() == kAwardTrackerSuppressedGameMode) return;
    if (q.narrative_blend_005b5d50()) return;
    if (!q.controlled_unit_present_e188d8()) return;
    const int type = q.controlled_type_170();
    if (last_unit_class_18_ == type) return;
    const bool plane_screen = registry_ != nullptr && registry_->screen_applied_05(kPlaneScreenSlot);
    const bool ship_screen = registry_ != nullptr && registry_->screen_applied_05(kShipScreenSlot);
    if (q.controlled_kind_of_5c(kUnitCategoryPlaneAlt) && !plane_screen) return;
    if (q.controlled_kind_of_5c(kUnitCategoryShip) && !ship_screen) return;
    // 00692C10..00692DCB: the first-use tutorial of the unit's category, each
    // only while the profile has not seen it; a trigger ends the pass.
    const char* basic = nullptr;
    if (q.controlled_kind_of_5c(kUnitCategorySubmarine)) {
        basic = kBasicSub;
    } else if (q.controlled_kind_of_5c(kUnitCategoryPlane) || q.controlled_kind_of_5c(kUnitCategoryPlaneAlt)) {
        basic = kBasicPlane;
    } else if (q.controlled_kind_of_5c(kUnitCategoryShip)) {
        basic = kBasicShip;
    }
    if (basic != nullptr && !profile_has_seen_007f8e00(basic)) {
        trigger_hint_00690fd0(basic, false);
        return;
    }
    // 00692DD0..00692E0F: a blocking (critical) active hint holds the class.
    if (!state_.active_60.empty()) {
        const HintDefinition* active = find_definition_0068e450(state_.active_60);  // 00690B00
        if (active != nullptr && active->critical) return;
    }
    last_unit_class_18_ = q.controlled_type_170();   // 00692E1A
    UnitClassHintInputs in{};
    in.class_id = last_unit_class_18_;
    if (in.class_id == kUnitClassLevelBomber) in.payload_kind = q.controlled_payload_007edad0();
    if (in.class_id == kUnitClassKamikaze) in.ohka_variant = q.controlled_ohka_144();
    if (const char* name = unit_class_hint_00692b60(in)) trigger_hint_00690fd0(name, false);
}

// 006926F0 __fastcall(tracker), RET. The name starts empty and the call at
// 00692916 is always made, so a fall-through triggers "" (no definition).
void GameHintTracker::update_weapon_006926f0() {
    if (frame_ == nullptr) return;
    HintFrameQueries& q = *frame_;
    if (q.local_slot_flag_19()) return;
    if (q.effective_game_mode_004bca50() == kAwardTrackerSuppressedGameMode) return;
    if (q.narrative_blend_005b5d50()) return;
    if (!state_.active_60.empty()) return;            // 0068E5B0 copy, length tested
    if (!q.controlled_unit_present_e188d8()) return;
    const bool plane_screen = registry_ != nullptr && registry_->screen_applied_05(kPlaneScreenSlot);
    const bool ship_screen = registry_ != nullptr && registry_->screen_applied_05(kShipScreenSlot);
    if (q.controlled_kind_of_5c(kUnitCategoryPlaneAlt) && !plane_screen) return;
    if (q.controlled_kind_of_5c(kUnitCategoryShip) && !ship_screen) return;
    const char* name = nullptr;
    if (q.controlled_kind_of_5c(kUnitCategoryShip) || q.controlled_kind_of_5c(kUnitCategoryGunnery)) {
        name = surface_weapon_hint_006926f0(q.weapon_group_44());
    } else if (q.controlled_kind_of_5c(kUnitCategoryPlaneAlt) || q.controlled_kind_of_5c(kUnitCategoryPlane)) {
        const bool leader = q.controlled_kind_of_5c(kUnitCategoryPlaneAlt);   // *(unit+3D0h)
        const int kind = q.plane_ordnance_007bca50(leader);
        const bool named = kind == kProjectileBomb || kind == kProjectileTorpedo || kind == kProjectileRocket
            || kind == kProjectileDepthCharge || kind == kProjectileParatrooperA || kind == kProjectileParatrooperB;
        name = air_weapon_hint_006926f0(kind, named ? false : q.plane_machinegun_c24(leader));
    }
    trigger_hint_00690fd0(name != nullptr ? std::string(name) : std::string(), false);
}

// 00692580 __fastcall(tracker), RET.
void GameHintTracker::update_environment_00692580() {
    if (frame_ == nullptr) return;
    HintFrameQueries& q = *frame_;
    if (q.effective_game_mode_004bca50() == kAwardTrackerSuppressedGameMode) return;
    if (q.narrative_blend_005b5d50()) return;
    if (!q.controlled_unit_present_e188d8()) return;
    if (!q.controlled_kind_of_5c(kUnitCategoryShip)) return;
    if (!q.class_repair_d0()) return;
    EnvironmentHintInputs in{};
    in.fire_amount = q.repair_fire_38();     // 00939F70 first
    in.water_amount = q.repair_water_34();   // 00939F80
    in.submerged = q.controlled_kind_of_5c(kUnitCategorySubmarine) && q.submarine_state_122c() == 2;
    if (static_cast<int>(in.water_amount) == 0 && static_cast<int>(in.fire_amount) == 0 && !in.submerged) {
        in.engine_signal = q.engine_signal_9e5();
    }
    const char* name = environment_hint_00692580(in);
    if (name != nullptr && !equals_ci(state_.active_60, name)) trigger_hint_00690fd0(name, false);
}

// 00692FD0 __fastcall(tracker), RET. No mode or blend gate. The pass returns
// early only when both first-get hints have been shown.
void GameHintTracker::update_zone_00692fd0() {
    if (frame_ == nullptr) return;
    HintFrameQueries& q = *frame_;
    if (!q.controlled_unit_present_e188d8()) return;
    if (!q.controlled_kind_of_5c(kUnitCategoryShip)) return;
    if (has_been_shown_0068e600(kLandingFirstGetHint) && has_been_shown_0068e600(kCaptureFirstGetHint)) return;
    std::vector<CaptureZoneSample> zones;
    q.command_building_zones_16c(zones);
    if (zones.empty()) return;
    float px = 0.0F;
    float py = 0.0F;
    float pz = 0.0F;
    q.controlled_position_fc(px, py, pz);
    const ZoneProximity proximity = scan_capture_zones_00692fd0(px, py, pz, zones.data(), zones.size());
    if (!proximity.in_capture_radius) {
        if (!proximity.in_landing_radius) return;
        if (!q.controlled_kind_of_5c(kUnitCategoryReconPlane)) return;
        if (!has_been_shown_0068e600(kLandingFirstGetHint) && !equals_ci(state_.active_60, kLandingFirstGetHint)) {
            trigger_hint_00690fd0(kLandingFirstGetHint, false);
        }
        return;
    }
    if (!has_been_shown_0068e600(kCaptureFirstGetHint) && !equals_ci(state_.active_60, kCaptureFirstGetHint)) {
        trigger_hint_00690fd0(kCaptureFirstGetHint, false);
    }
}

// 00692960 __fastcall(tracker), RET.
void GameHintTracker::update_strategic_map_00692960() {
    if (frame_ == nullptr) return;
    HintFrameQueries& q = *frame_;
    StrategicMapHintInputs in{};
    in.game_mode = q.effective_game_mode_004bca50();
    if (in.game_mode == kAwardTrackerSuppressedGameMode) return;
    in.transition_blend_active = q.narrative_blend_005b5d50();
    if (in.transition_blend_active) return;
    in.already_shown = has_been_shown_0068e600(kStrategicMapFirstGetHint);
    if (!in.already_shown) {
        in.map_available = q.support_manager_available_0066ffa0();
        if (in.map_available) in.map_open = q.support_manager_free_camera_00652a30();
    }
    in.hint_active = !state_.active_60.empty();
    if (should_show_strategic_map_hint_00692960(in) && !equals_ci(state_.active_60, kStrategicMapFirstGetHint)) {
        trigger_hint_00690fd0(kStrategicMapFirstGetHint, false);
    }
}

void GameHintTracker::report() const {
    std::size_t calls = 0;
    for (const auto& [name, count] : trigger_counts_) calls += count.calls;
    log_.notef("hint tracker summary: trigger calls=%zu names=%zu queue=%zu cooldowns=%zu "
               "mission_seen48=%zu profile_seen=%zu active=%s",
               calls, trigger_counts_.size(), queue_68_.size(), cooldowns_3c_.size(),
               mission_seen_48_.size(), profile_.profile().string_list_08.size(),
               state_.active_60.empty() ? "-" : state_.active_60.c_str());
    for (const auto& [name, count] : trigger_counts_) {
        log_.notef("  hint trigger %-24s calls=%zu shown=%zu hidden=%zu queued=%zu refused=%zu%s%s",
                   name.empty() ? "\"\"" : name.c_str(), count.calls, count.shown, count.hidden,
                   count.queued, count.refused, count.first_refusal.empty() ? "" : " first=",
                   count.first_refusal.c_str());
    }
}

} // namespace bsp::game
