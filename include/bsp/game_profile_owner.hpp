#pragma once

#include "bsp/award_trackers.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/gui_screens_3.hpp"
#include "bsp/mission_progress.hpp"
#include "bsp/profile_persistence.hpp"
#include "bsp/profile_reset.hpp"

#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

// cc15 object bring-up, lane L2: the GGame profile block (game+650h) with its
// MissionProgress owner (+64h, game+6B4h), and the hint ("award") tracker
// singleton [00E198D0]. Evidence, lifetime and the rows each switch wires:
// docs/CC15_OBJECT_PROFILE_HINTS.md. These are host projections over the
// existing typed bodies, not native layouts.
namespace bsp::game {

class GameHostLog;
class GameVfsHost;
class GameHintsScreen;
struct HintFrameQueries;
struct HintScreenRegistry;

// Off: the host keeps the earlier stand-ins (TitleInit/PressStart reset
// unimplemented, map_flag_visible = true, mission_result's own progress_).
// On: the profile owner is reset where the image resets it and the map-flag
// and mission-record rows read and write its MissionProgress.
inline constexpr bool kProfileOwnerCc15Bound = true;
// Off: 0068EA00 and 00690CD0 stay unimplemented. On: the tracker is
// built by the first 004E1CA0 call, OnInit's 006921B0 loads the definitions
// map +30h from HintSystemMessages.lua, and the rows run on it.
inline constexpr bool kHintTrackerCc15Bound = true;

// The profile block GGame holds inline at game+650h. Natively the profile
// constructor 007FEE20 already calls 007FDB20, so +64h is allocated from game
// construction on; OnInitTitle (004C9AFA) and the press-start sign-in reset it
// again. Only an EMPTY (new) profile is modelled: the storage read
// 007FF100 -> 007FEFE0 -> 007FDF00 is not run.
class GameProfileOwner final : public ProfileResetHost {
public:
    explicit GameProfileOwner(GameHostLog& log);

    // 007FDB20, ECX = game+650h. The settings half (008D4820 control and
    // 008D41C0 gameplay defaults) is applied to a private copy and discarded:
    // the host's live settings stay those phase 5 loaded. See the doc.
    void reset_to_defaults_007fdb20();

    ProfileResetState& profile() noexcept { return profile_; }
    const ProfileResetState& profile() const noexcept { return profile_; }
    // game+6B4h. Null only before the first reset or after a failed new(24h).
    MissionProgress* mission_progress() noexcept;
    std::uint32_t reset_count() const noexcept { return resets_; }

    // ProfileResetHost.
    void destroy_mission_progress_007fd780() override;
    bool construct_mission_progress_00920e10() override;
    bool xenon_state_is_two() override;
    bool xenon_user_selected() override;
    void restore_selected_user_controls_008d45d0() override;

private:
    GameHostLog& log_;
    ProfileResetState profile_;
    MissionProgressOwner progress_;
    GameSettingsBlock discarded_settings_{};
    std::uint32_t resets_{0};
};

// The A0h tracker 004E18E0 builds. Each member carries the native offset it
// stands for (include/bsp/award_trackers.hpp); the containers are C++.
class GameHintTracker final : public HintStoreHost, public HintTrackerHost {
public:
    // 004E18E0: every container empty; +4h and +18h are left alone natively
    // (+18h reads as kAwardTrackerNoUnitClass here, as award_trackers.hpp does).
    GameHintTracker(GameHostLog& log, GameProfileOwner& profile);
    ~GameHintTracker() override;
    GameHintTracker(const GameHintTracker&) = delete;
    GameHintTracker& operator=(const GameHintTracker&) = delete;

    // 0068EA00 __thiscall(tracker), RET: 004D05E0 empties the list +48h, then
    // 0068E780 pops the deque +68h until its size +78h is 0. The raw body is
    // award_tracker_clear_hint_queue_0068ea00 (frontend_seg16_24_28_cc13.cpp).
    void clear_hint_queue_0068ea00();
    // 00690CD0 __fastcall(tracker), RET, through hint_tracker_rebuild_history_00690cd0.
    void rebuild_history_00690cd0();
    // 006921B0 __fastcall(tracker), RET, through hint_tracker_reload_006921b0:
    // a private Lua state opened with mask 1 runs HintSystemMessages.lua (with
    // the VFS overrides, 00B69D40), and the five sub-tables go through
    // hint_tracker_load_definitions_0068f030 into +30h.
    void reload_hint_tables_006921b0(GameVfsHost& vfs);

    // Counts from the last reload, for the run log.
    struct ReloadSummary {
        bool script_called{false};     // a chunk of the script ran
        std::size_t entries{0};        // sub-table entries the loader visited
        std::size_t replaced{0};       // entries whose id was already in +30h
        std::size_t other_category{0}; // definitions with +50h set (OTHER.*)
        std::size_t hidden{0};
        std::size_t critical{0};
        std::size_t mission_specific{0};
    };
    const ReloadSummary& reload_summary() const noexcept { return reload_summary_; }

    std::size_t definition_count() const noexcept { return definitions_30_.size(); }
    std::size_t history94_count() const noexcept { return history_94_.size(); }

    // ---- cc15-hint2 (kHintTriggerPassesCc15Bound), src/game_hint_passes.cpp ----
    // The mission frame binds its reads for as long as it lives; the menu host
    // binds its registry once. Both may be null (no mission, no registry).
    void bind_frame(HintFrameQueries* frame) noexcept { frame_ = frame; }
    void unbind_frame(HintFrameQueries* frame) noexcept {
        if (frame_ == frame) frame_ = nullptr;
    }
    void bind_registry(HintScreenRegistry* registry) noexcept { registry_ = registry; }
    // [00E198C4]+A8h, the hints screen 34h. Owned here, pumped by the menu host.
    GameHintsScreen& hints_screen_34();
    // 00690FD0 __thiscall(tracker, NativeString* name, char forced), RET 8.
    void trigger_hint_00690fd0(const std::string& name, bool forced);
    // 0068E5B0 (copy of +60h), 0068E420 (+60h := ""), 0068E600, 004D05E0 on
    // +48h, and the hints screen's Exit store of +1Ch := "".
    const std::string& active_hint_60() const noexcept { return state_.active_60; }
    void finish_active_0068e420();
    void clear_list_48_004d05e0() { mission_seen_48_.clear(); }
    void clear_last_timed_1c() { state_.last_timed_1c.clear(); }
    // The seven passes 004E525E..004E52BA, in OnMove order.
    void tick_cooldowns_0068ec10(float raw_delta);
    void drain_queued_00692b00();
    void update_unit_class_00692b60();
    void update_weapon_006926f0();
    void update_environment_00692580();
    void update_zone_00692fd0();
    void update_strategic_map_00692960();
    // The run summary: TriggerHint calls and outcomes per name.
    void report() const;

    // HintTrackerHost (TriggerHint's reads and writes).
    std::int32_t hints_setting_00f8898c() override;
    bool hud_screen_e4_busy() override;
    void queue_push_back_0068eb90(const std::string& name) override;
    bool mission_seen_contains_48(const std::string& name) override;
    void touch_recent_0068ea70(const std::string& name) override;
    bool cooldown_contains_0068e4c0(const std::string& name) override;
    void cooldown_set_00444be0(const std::string& name, float seconds) override;
    bool pad_prompts_00f88a30() override;
    bool localised_key_exists(const std::string& key) override;
    std::int32_t local_player_slot_18ec() override;
    std::int32_t local_side_nation_28() override;
    std::int32_t hints_seconds_left_0054d3c0() override;
    void hints_dismiss_0054e610() override;
    void hints_show_0054e020(bool pausing, const std::string& text, const std::string& title,
                             std::int32_t seconds) override;
    std::int32_t lua_run_string_006b8ad0(const std::string& command) override;
    void diagnostic_lua_result_004254b0(bool failed, const std::string& command) override;
    bool history94_erase_name(const std::string& key, const std::string& name) override;
    void history7c_push_back(const std::string& key, const std::string& name) override;
    void history_assign_single(const std::string& key, const std::string& name) override;

    // HintStoreHost.
    const HintDefinition* find_definition_0068e450(const std::string& name) override;
    bool has_been_shown_0068e600(const std::string& name) override;
    bool profile_has_seen_007f8e00(const std::string& name) override;
    void profile_mark_seen_007fae20(const std::string& name) override;
    void profile_unmark_007fae60(const std::string& name) override;
    void mission_seen_push_back_48(const std::string& name) override;
    std::vector<std::string>& mission_seen_48() override { return mission_seen_48_; }
    bool player_unit_present_e188d8() override;
    std::int32_t player_unit_class_170() override;
    void history7c_clear_00690ca0() override { history_7c_.clear(); }
    void history94_clear() override { history_94_.clear(); }
    bool history94_has_key(const std::string& key) override;
    void history94_push_back(const std::string& key, const std::string& name) override;
    void history94_assign_single(const std::string& key, const std::string& name) override;
    std::vector<std::string> profile_shown_names_65c() override;

private:
    bool has_been_shown_cc15_0068e600(const std::string& name);
    bool profile_has_seen_cc15_007f8e00(const std::string& name);
    void profile_mark_seen_cc15_007fae20(const std::string& name);
    double mission_seconds() const;

    using HintHistory =
        std::map<std::string, std::vector<std::string>, NativeStringCaseInsensitiveLess>;

    GameHostLog& log_;
    GameProfileOwner& profile_;
    HintTrackerState state_{};                                  // +04h, +1Ch, +24h, +28h, +60h
    std::int32_t last_unit_class_18_{kAwardTrackerNoUnitClass}; // +18h
    std::map<std::string, HintDefinition, NativeStringCaseInsensitiveLess> definitions_30_;
    std::map<std::string, float, NativeStringCaseInsensitiveLess> cooldowns_3c_;
    std::vector<std::string> mission_seen_48_;                  // +48h list
    std::vector<std::string> recent_54_;                        // +54h list
    std::deque<std::string> queue_68_;                          // +68h deque, size +78h
    HintHistory history_7c_;
    HintHistory history_88_;
    HintHistory history_94_;
    ReloadSummary reload_summary_{};
    // cc15-hint2.
    HintFrameQueries* frame_{nullptr};
    HintScreenRegistry* registry_{nullptr};
    std::unique_ptr<GameHintsScreen> screen_34_;
    struct TriggerCount {
        std::size_t calls{0};
        std::size_t shown{0};
        std::size_t queued{0};
        std::size_t hidden{0};      // became active without a show
        std::size_t refused{0};     // a seen-list or cooldown gate refused it
        std::string first_refusal;
    };
    void note_refused(const std::string& name, const char* reason);
    std::map<std::string, TriggerCount> trigger_counts_;
    std::string triggering_;   // the name of the TriggerHint call in progress

    friend class HintTablesReload;
    friend class GameHintsScreen;
};

// [00E198D0] and its getter 004E1CA0: new(A0h) + 004E18E0 on the first call,
// registered with the singleton-lifetime manager (00BD0C30) and destroyed by
// its drain (004E1C80 -> 004E1A50). Here the slot owner destroys it.
class GameHintTrackerSlot final {
public:
    GameHintTrackerSlot(GameHostLog& log, GameProfileOwner& profile);
    ~GameHintTrackerSlot();
    GameHintTrackerSlot(const GameHintTrackerSlot&) = delete;
    GameHintTrackerSlot& operator=(const GameHintTrackerSlot&) = delete;
    GameHintTracker& get_004e1ca0();
    GameHintTracker* built() noexcept { return tracker_.get(); }
    // The process's one slot, as the global [00E198D0] is reached from any
    // caller through 004E1CA0; null before the menu host builds it.
    static GameHintTrackerSlot* process_slot() noexcept;

private:
    GameHostLog& log_;
    GameProfileOwner& profile_;
    std::unique_ptr<GameHintTracker> tracker_;
};

} // namespace bsp::game
