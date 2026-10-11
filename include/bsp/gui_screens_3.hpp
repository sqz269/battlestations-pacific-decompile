#pragma once

// HUD and GUI screens of partition segments 16, 28 and 4 (packet cc13_gui5_screens3).
// Evidence: docs/CC13_GUI_SCREENS_3.md. Every name here is a hypothesis; addresses,
// ABIs and listing ranges are in the comments of src/gui_screens_3.cpp.

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace bsp {

// ---------------------------------------------------------------------------
// HUD hints screen (screen 34h, HUD manager +A8h, page GUI_hints, constructor 0054D680,
// primary vtable 00CEE290: +00 0054D660 id, +04 0054D670 (MOV AL,1), +10 Register 0054D860,
// +14 Layout 0054D980, +18 Enter 0054E380, +1C Exit 0054E3D0, +20 Update 0054E6B0,
// +24 Collect 0054E370)
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kHudHintsScreenId = 0x34;          // 0054D660
inline constexpr std::string_view kHudHintsPage = "GUI_hints";    // 0054D8A4
inline constexpr float kHudHintsGrowSeconds = 0.15F;              // 00CE7818 -> +58h
// The two warning cues the constructor loads from the char* table 00E08698..00E086A0.
inline constexpr std::array<std::string_view, 2> kHudHintsCueSamples{
    "environment/environment_warning_hint.fsb",      // 00CEE260, cue 0: normal hint
    "environment/environment_warning_critical.fsb",  // 00CEE230, cue 1: pausing hint
};
inline constexpr std::string_view kHudHintsCueClass = "InGameGUI";  // 00CE7854, cached in 00E19314
inline constexpr std::string_view kHudHintsCueType = "Normal";      // 00CECD20
inline constexpr std::string_view kHudHintsPressPad = "hints.pressa";        // 00CEE3E0
inline constexpr std::string_view kHudHintsPressKey = "hints.pressenter";    // 00CEE3CC
inline constexpr std::int32_t kHudHintsConfirmAction = 0x4A;     // input action record +DE0h = 4Ah * 30h
inline constexpr std::int32_t kHudHintsResetAction = 0xFF;       // input action record +2FD0h = FFh * 30h
inline constexpr std::int32_t kHudHintsInputContext = 0x1B;      // 00A933F0 context index
inline constexpr std::int32_t kHudHintsProfileQuery = 0x0E;      // 00427190 query kind
inline constexpr std::int32_t kHudHintsTutorialGameMode = 8;     // 004BCA50 result tested by Update

// The 7Ch-byte screen. Widgets are bound by Layout 0054D980 (parent in brackets).
struct HudHintsState {
    bool registered_04{};          // +04h, base-screen byte set last by Register
    bool pausing_08{};             // +08h, the shown hint pauses the game (pause group)
    bool byte_09{};                // +09h, cleared with +08h; no reader found in these routines
    bool persistent_50{};          // +50h, shown with duration 0: never times out
    float remaining_54{};          // +54h, seconds until the hint hides
    float grow_seconds_58{};       // +58h, frame-box grow time (0.15)
    float grow_elapsed_5c{};       // +5Ch
    std::array<float, 2> pause_frame_size_60{};   // +60h, +64h
    std::array<float, 2> normal_frame_size_68{};  // +68h, +6Ch
    std::array<float, 3> normal_text_pos_70{};    // +70h..+78h, local position of +34h at Register
    std::uintptr_t page_1c{};          // GUI_hints
    std::uintptr_t pause_group_20{};   // hints_pause_Group [page]
    std::uintptr_t normal_group_24{};  // hints_normal_Group [page]
    std::uintptr_t pause_title_28{};   // hints_title_Text [pause]
    std::uintptr_t pause_text_2c{};    // hints_description_Text [pause]
    std::uintptr_t normal_text_34{};   // hints_description_Text [normal]
    std::uintptr_t pause_button_38{};  // hints_button_Text [pause]
    std::uintptr_t pause_icon_3c{};    // hints_1_Icon [pause]
    std::uintptr_t pause_icon_40{};    // hints_2_Icon [pause]
    std::uintptr_t normal_icon_44{};   // hints_1_Icon [normal]
    std::uintptr_t pause_frame_48{};   // hints_FrameBox [pause]
    std::uintptr_t normal_frame_4c{};  // hints_FrameBox [normal]
};

// Next step of the four basic tutorial chains (0054E4FD..0054E58A, 0054EAA7..0054EB0B);
// the comparisons are case-insensitive (__stricmp, 00425850). Empty: no next step.
std::string_view hud_hints_next_basic_step(std::string_view active);

struct HudHintsHost {
    virtual ~HudHintsHost() = default;
    // Screen base and page plumbing.
    virtual void register_front_end_screen_004f71d0() = 0;
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;        // (name, 1, 0)
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;  // (name, 1)
    virtual void screen_layout_14() = 0;                                         // own vtable +14h
    virtual std::array<float, 2> widget_size_00aa6740(std::uintptr_t widget) = 0;
    virtual std::array<float, 3> widget_local_position_00aa67f0(std::uintptr_t widget) = 0;
    virtual void widget_set_local_xy_00aa7d00(std::uintptr_t widget, float x, float y) = 0;
    virtual void widget_set_size_58(std::uintptr_t widget, const std::array<float, 2>& size) = 0;  // vtable +58h
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;          // vtable +34h
    virtual bool widget_shown_38(std::uintptr_t widget) = 0;                     // vtable +38h
    virtual void collect_page_004d6790(std::uintptr_t list, std::uintptr_t page_slot) = 0;  // ECX = list
    // Text.
    virtual void text_set_source_00abaed0(std::uintptr_t text, const std::string& key) = 0;  // (key, 1)
    virtual float text_normalized_height_00ab6bd0(std::uintptr_t text) = 0;
    virtual std::int32_t text_line_count_ec(std::uintptr_t text) = 0;            // int +ECh
    virtual float text_width_114(std::uintptr_t text) = 0;                       // float +114h
    virtual float subtitle_seconds_per_line_7c() = 0;   // *(*(00E198C4)+A4h)+7Ch
    virtual float subtitle_seconds_base_80() = 0;       // *(*(00E198C4)+A4h)+80h
    virtual bool pad_prompts_00f88a30() = 0;
    // Game singleton 00E188A8.
    virtual std::int32_t game_session_kind_1fe4() = 0;
    virtual bool game_byte_218c() = 0;
    virtual bool game_byte_61f() = 0;
    virtual bool game_cinematic_hidden_634() = 0;
    virtual std::int32_t game_effective_mode_004bca50() = 0;
    // 004D94F0(game, flag): when game+620h differs and game+624h is 0, store it and pass
    // screens 28h, 55h, 57h, 5Dh, 34h with the flag to 004D6480.
    virtual void game_hint_pause_004d94f0(bool paused) = 0;
    virtual void game_set_cinematic_004cd0f0(bool hide, bool allow_simulation, bool p4) = 0;
    virtual void game_004d9830(bool flag) = 0;        // ledger tag says scalar deleting dtor; called on the game with 1
    // Input manager 004BEC00.
    virtual bool input_action_pressed_00535ee0(std::int32_t action) = 0;
    virtual void input_action_reset_00a91e20(std::int32_t action) = 0;
    virtual void input_context_level_00a933f0(std::int32_t context, std::int32_t level) = 0;
    virtual void input_context_level5_004c5140(std::int32_t context) = 0;   // 00A933F0(context, 5)
    virtual bool profile_hints_query_00427190(std::int32_t kind) = 0;      // ECX = 004C1E90()
    // Hint tracker 004E1CA0.
    virtual std::string tracker_active_hint_0068e5b0() = 0;
    virtual void tracker_finish_active_0068e420() = 0;
    virtual void tracker_clear_queue_0068ea00() = 0;
    virtual void tracker_clear_list_48_004d05e0() = 0;   // tracker+48h
    virtual void tracker_clear_active_name_1c() = 0;      // tracker+1Ch := "" (00CE3A0C)
    virtual void tracker_trigger_hint_00690fd0(const std::string& name, bool forced) = 0;
    // Cue sounds: the constructor's samples +0Ch/+10h and the tracked sounds +14h/+18h.
    virtual bool cue_playing(std::int32_t cue) = 0;                // tracked slot non-null
    virtual void cue_start_00a7e490(std::int32_t cue, std::string_view sound_class, std::string_view sound_type) = 0;
    virtual void cue_release_0054d510(std::int32_t cue) = 0;       // 0054D510(slot, null)
};

// 0054D860: slot +10h of vtable 00CEE290, __thiscall(screen), RET; SEH 00C6D348.
void hud_hints_register_0054d860(HudHintsState& screen, HudHintsHost& host);
// 0054D980: slot +14h, __thiscall(screen), RET; SEH 00C6D3B8.
void hud_hints_layout_0054d980(HudHintsState& screen, HudHintsHost& host);
// 0054E380: slot +18h, __thiscall(screen), RET.
void hud_hints_enter_0054e380(HudHintsState& screen, HudHintsHost& host);
// 0054E3D0: slot +1Ch, __thiscall(screen), RET.
void hud_hints_exit_0054e3d0(HudHintsState& screen, HudHintsHost& host);
// 0054E370: slot +24h, __thiscall(screen, list), RET 4.
void hud_hints_collect_0054e370(HudHintsHost& host, std::uintptr_t list, std::uintptr_t page_slot);  // page_slot = &screen+1Ch
// 0054DE90: __thiscall(screen, cue), RET 4; SEH 00C6D3F6.
void hud_hints_play_cue_0054de90(std::int32_t cue, HudHintsHost& host);
// 0054E020: __thiscall(screen, char pausing, text, title, int seconds), RET 10h.
void hud_hints_show_0054e020(HudHintsState& screen, HudHintsHost& host, bool pausing,
                             const std::string& text, const std::string& title, std::int32_t seconds);
// 0054E440: __thiscall(screen), RET; SEH 00C6D410.
void hud_hints_advance_basic_step_0054e440(HudHintsState& screen, HudHintsHost& host);
// 0054E610: __thiscall(screen), RET.
void hud_hints_dismiss_0054e610(HudHintsState& screen, HudHintsHost& host);
// 0054E6B0: slot +20h, __thiscall(screen, float dt), RET 4; SEH 00C6D438.
void hud_hints_update_0054e6b0(HudHintsState& screen, HudHintsHost& host, float dt);

}  // namespace bsp
