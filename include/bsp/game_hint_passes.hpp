#pragma once

#include "bsp/award_trackers.hpp"
#include "bsp/gui_screens_3.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// cc15 packet cc15-hint2: the hint tracker's per-frame callers. The seven
// passes GGame::OnMove runs at 004E525E..004E52BA, TriggerHint 00690FD0 on the
// tracker [00E198D0], and the hints screen 34h ([00E198C4]+A8h, vtable
// 00CEE290) that shows the hint, times it and finishes it. Evidence, the
// inputs each pass reads and the labelled substitutions:
// docs/CC15_OBJECT_PROFILE_HINTS.md section 6. Host projections over the
// typed bodies in src/gui_screens_3.cpp and src/award_trackers.cpp.
namespace bsp::game {

// Off: the mission frame keeps its stand-in (a host-side cooldown vector, a
// drain that never pops, five recorded passes, PowerUps hints recorded) and
// the hints screen 34h is never entered. On: the passes run on the tracker,
// TriggerHint decides and records each hint, screen 34h is self-managed and
// pumped, and its Update times the shown hint and finishes it.
inline constexpr bool kHintTriggerPassesCc15Bound = false;

// Off: the HUD hosts read game+620h as clear (their old substitution). On:
// the HUD root 44h (00649860) and markers 4Dh (006435D0) updates read the
// hint pause 004D94F0 last stored, so a critical hint stops them as the image
// does. Nothing else reads 620h in the host: the pilot 00519BB0, map icon
// 005ADDA0 and Support Manager 4Eh 00675C40 updates are not run.
inline constexpr bool kHintPauseHudGateCc15Bound = false;
// game+620h as 004D94F0 last stored it on the process's tracker [00E198D0];
// false before the tracker or its screen 34h exists.
bool game_hint_pause_620_cc15() noexcept;

// The game-side reads of the passes and TriggerHint. The mission frame host
// implements it over its units and scene state for as long as it lives.
struct HintFrameQueries {
    virtual ~HintFrameQueries() = default;
    virtual int effective_game_mode_004bca50() = 0;
    // 005B5D50 on [[00E198C4]+A4h]: the narrative screen's +C0h != 0.0.
    virtual bool narrative_blend_005b5d50() = 0;
    // [00E188D8] and the reads the passes make through it.
    virtual bool controlled_unit_present_e188d8() = 0;
    virtual bool controlled_kind_of_5c(int category) = 0;      // vtable +5Ch
    virtual int controlled_type_170() = 0;                     // (+170h)->vtable[0]()
    virtual int controlled_payload_007edad0() = 0;             // class 7 only
    virtual bool controlled_ohka_144() = 0;                    // [[+3D0h]+538h]+144h
    virtual int plane_ordnance_007bca50(bool squadron_leader) = 0;
    virtual bool plane_machinegun_c24(bool squadron_leader) = 0;
    virtual bool class_repair_d0() = 0;                        // [unit+538h]+D0h
    virtual float repair_water_34() = 0;                       // 00939F80 on unit+A20h
    virtual float repair_fire_38() = 0;                        // 00939F70 on unit+A20h
    virtual int submarine_state_122c() = 0;
    virtual bool engine_signal_9e5() = 0;
    virtual void controlled_position_fc(float& x, float& y, float& z) = 0;
    // [[00E188A8]+19CCh]+16Ch, the command-building list (world list 28).
    virtual void command_building_zones_16c(std::vector<CaptureZoneSample>& zones) = 0;
    // *(game+18CCh + game+18ECh*4): +19h and +28h.
    virtual bool local_slot_flag_19() = 0;
    virtual int local_side_nation_28() = 0;
    virtual int local_player_slot_18ec() = 0;
    // [[00E198C4]+50h]+44h, screen 2Eh's selected weapon group.
    virtual int weapon_group_44() = 0;
    // 0066FFA0 / 00652A30 on [[00E198C4]+BCh], screen 4Eh.
    virtual bool support_manager_available_0066ffa0() = 0;
    virtual bool support_manager_free_camera_00652a30() = 0;
    virtual std::int32_t hints_setting_00f8898c() = 0;
    virtual bool pad_prompts_00f88a30() = 0;
    // 006B8AD0(command, 0, 0, 2) on *(game+1A08h)+4; non-zero is a failure.
    virtual std::int32_t lua_run_string_006b8ad0(const std::string& command) = 0;
    virtual bool game_cinematic_hidden_634() = 0;
    virtual void game_set_cinematic_004cd0f0(bool hide, bool allow_simulation, bool third) = 0;
    virtual void record(const char* label, std::uint32_t address) = 0;
    // The mission clock, for the run log only.
    virtual double mission_seconds() = 0;
};

// The menu host's side: the 95-slot registry the screen flags live in, the
// input actions and the locale table.
struct HintScreenRegistry {
    virtual ~HintScreenRegistry() = default;
    virtual bool screen_applied_05(int slot) = 0;   // +5h
    virtual bool screen_wanted_04(int slot) = 0;    // +4h
    virtual bool input_action_pressed_00535ee0(int action) = 0;
    // The localised text of a key, or null when the table has none.
    virtual const std::u16string* locale_text(const std::string& key) = 0;
};

class GameHostLog;
class GameHintTracker;

// Screen 34h over the typed bodies hud_hints_* (0054D860 register, 0054E380
// enter, 0054E3D0 exit, 0054E6B0 update, 0054E020 show, 0054E610 dismiss).
// The eleven widgets are a state model (shown, size, position, text key):
// nothing is drawn. The game-side calls go to the bound HintFrameQueries.
class GameHintsScreen final : public HudHintsHost {
public:
    GameHintsScreen(GameHostLog& log, GameHintTracker& tracker);

    void register_0054d860();
    void enter_0054e380();
    void exit_0054e3d0();
    void update_0054e6b0(float dt);
    void show_0054e020(bool pausing, const std::string& text, const std::string& title,
                       std::int32_t seconds);
    void dismiss_0054e610();
    // 0054D3C0: (int) +54h.
    std::int32_t seconds_left_0054d3c0() const;
    const HudHintsState& state() const noexcept { return state_; }
    bool hint_paused_620() const noexcept { return hint_paused_; }

    // HudHintsHost.
    void register_front_end_screen_004f71d0() override {}
    std::uintptr_t load_page_00aa5840(std::string_view name) override;
    std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) override;
    void screen_layout_14() override;
    std::array<float, 2> widget_size_00aa6740(std::uintptr_t widget) override;
    std::array<float, 3> widget_local_position_00aa67f0(std::uintptr_t widget) override;
    void widget_set_local_xy_00aa7d00(std::uintptr_t widget, float x, float y) override;
    void widget_set_size_58(std::uintptr_t widget, const std::array<float, 2>& size) override;
    void widget_show_34(std::uintptr_t widget, bool shown) override;
    bool widget_shown_38(std::uintptr_t widget) override;
    void collect_page_004d6790(std::uintptr_t, std::uintptr_t) override {}
    void text_set_source_00abaed0(std::uintptr_t text, const std::string& key) override;
    float text_normalized_height_00ab6bd0(std::uintptr_t) override { return 0.0F; }
    std::int32_t text_line_count_ec(std::uintptr_t text) override;
    float text_width_114(std::uintptr_t) override { return 0.0F; }
    float subtitle_seconds_per_line_7c() override;
    float subtitle_seconds_base_80() override;
    bool pad_prompts_00f88a30() override;
    std::int32_t game_session_kind_1fe4() override { return 0; }
    bool game_byte_218c() override { return false; }
    bool game_byte_61f() override { return false; }
    bool game_cinematic_hidden_634() override;
    std::int32_t game_effective_mode_004bca50() override;
    void game_hint_pause_004d94f0(bool paused) override;
    void game_set_cinematic_004cd0f0(bool hide, bool allow_simulation, bool p4) override;
    void game_set_gui_layers_visible_004d9830(bool visible) override;
    bool input_action_pressed_00535ee0(std::int32_t action) override;
    void input_action_reset_00a91e20(std::int32_t action) override;
    void input_context_level_00a933f0(std::int32_t context, std::int32_t level) override;
    void input_context_level5_004c5140(std::int32_t context) override;
    bool profile_hints_query_00427190(std::int32_t kind) override;
    std::string tracker_active_hint_0068e5b0() override;
    void tracker_finish_active_0068e420() override;
    void tracker_clear_queue_0068ea00() override;
    void tracker_clear_list_48_004d05e0() override;
    void tracker_clear_active_name_1c() override;
    void tracker_trigger_hint_00690fd0(const std::string& name, bool forced) override;
    bool cue_playing(std::int32_t) override { return false; }
    void cue_start_00a7e490(std::int32_t cue, std::string_view sound_class,
                            std::string_view sound_type) override;
    void cue_release_0054d510(std::int32_t) override {}

private:
    struct Widget {
        bool shown{false};
        std::array<float, 2> size{};
        std::array<float, 3> position{};
        std::string text_key;
    };
    Widget* widget(std::uintptr_t handle);

    GameHostLog& log_;
    GameHintTracker& tracker_;
    HudHintsState state_{};
    std::map<std::uintptr_t, Widget> widgets_;
    std::map<std::string, std::uintptr_t> handles_;
    bool hint_paused_{false};   // game+620h as 004D94F0 last stored it
};

} // namespace bsp::game
