#pragma once

// HUD and GUI screens of partition segments 16, 28 and 4 (packet cc13_gui5_screens3).
// Evidence: docs/CC13_GUI_SCREENS_3.md. Every name here is a hypothesis; addresses,
// ABIs and listing ranges are in the comments of src/gui_screens_3.cpp.

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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

// ---------------------------------------------------------------------------
// Hint tracker TriggerHint 00690FD0 (singleton 004E1CA0 / 00E198D0, 0A0h bytes; layout in
// docs/GAME_AWARD_TRACKERS.md). The only caller of ShowHint 0054E020.
// ---------------------------------------------------------------------------

inline constexpr std::string_view kHintRuleUnique = "unique";                // 00CF7BE8
inline constexpr std::string_view kHintRuleMissionUnique = "missionunique";  // 00CF7BD8
inline constexpr std::string_view kHintRuleCooldown = "cooldown";            // 00CF7C5C
inline constexpr std::string_view kHintPcSuffix = "_pc";                     // 00CEBD30
inline constexpr std::string_view kHintMissionType = "hints.missionhint";    // 00CF7AEC
inline constexpr std::string_view kHintMissionHistoryPrefix = "AAAAAA";      // 00CF3F58
inline constexpr double kHintSeenFraction = 0.25;                            // 00D7A348

// One TitleDescList row (18h bytes), indexed by the local player's slot game+18ECh.
struct HintTitleDescRow {
    std::string title;        // +00h
    std::string description;  // +08h
    std::string lua_command;  // +10h
};

// The 128h-byte definition value of the map at tracker+30h. Offsets from the copy constructor
// 005FE010 and the Lua loader 0068F030 (definition on its stack at +98h).
struct HintDefinition {
    std::string title;                  // +00h Title
    std::string type;                   // +08h Type (default hints.gameplay_general_hint)
    std::string description;            // +10h Description
    std::string lua_command;            // +18h LuaCommand
    std::string allied_title;           // +20h AlliedTitle
    std::string allied_lua_command;     // +28h AlliedLuaCommand
    std::string allied_description;     // +30h AlliedDescription
    std::string japanese_title;         // +38h JapaneseTitle
    std::string japanese_lua_command;   // +40h JapaneseLuaCommand
    std::string japanese_description;   // +48h JapaneseDescription
    bool loader_flag_50{};              // +50h, the loader's byte argument (0068FE65 -> +50h)
    std::array<HintTitleDescRow, 8> title_desc_list{};  // +54h TitleDescList
    bool critical{};                    // +114h critical: a pausing hint
    bool hidden{};                      // +115h hidden: recorded but never shown
    bool mission_specific{};            // +116h missionspecific
    std::string storing_rule;           // +118h storingrule (default "repeatable")
    std::int32_t cooldown_time{};       // +120h cooldowntime
    std::int32_t duration{};            // +124h duration (seconds handed to ShowHint)
};

// Tracker fields TriggerHint reads or writes (the containers stay host-side).
struct HintTrackerState {
    bool force_next_04{};               // +04h, consumed by the next trigger
    std::string last_timed_1c;          // +1Ch, last timed hint, checked by the next trigger
    float last_timed_seconds_24{};      // +24h, its seconds as (int) hints+54h at show time
    std::string persistent_28;          // +28h, last untimed (persistent) hint
    std::string active_60;              // +60h, active hint name
};

struct HintTrackerHost {
    virtual ~HintTrackerHost() = default;
    virtual std::int32_t hints_setting_00f8898c() = 0;                 // 0 off, 1 forced only
    virtual const HintDefinition* find_definition_0068e450(const std::string& name) = 0;  // map +30h; null = end()
    virtual bool hud_screen_e4_busy() = 0;                             // *(*(00E198C4)+E4h)+4
    virtual void queue_push_back_0068eb90(const std::string& name) = 0;  // deque +68h
    virtual bool profile_has_seen_007f8e00(const std::string& name) = 0;  // ECX = game+650h
    virtual void profile_mark_seen_007fae20(const std::string& name) = 0;  // ECX = game+650h
    virtual bool mission_seen_contains_48(const std::string& name) = 0;  // list +48h, case-insensitive
    virtual void mission_seen_push_back_48(const std::string& name) = 0;
    virtual void touch_recent_0068ea70(const std::string& name) = 0;     // list +54h
    virtual bool cooldown_contains_0068e4c0(const std::string& name) = 0;  // map +3Ch
    virtual void cooldown_set_00444be0(const std::string& name, float seconds) = 0;
    virtual bool pad_prompts_00f88a30() = 0;
    // 00A9FAD0 on 00F8BC4C, 00436630 narrow, 00449AF0: the key resolves to other text.
    virtual bool localised_key_exists(const std::string& key) = 0;
    virtual std::int32_t local_player_slot_18ec() = 0;                 // game+18ECh
    virtual std::int32_t local_side_nation_28() = 0;                   // *(game+18CCh+slot*4)+28h
    virtual std::int32_t hints_seconds_left_0054d3c0() = 0;            // (int) hints screen +54h
    virtual void hints_dismiss_0054e610() = 0;
    virtual void hints_show_0054e020(bool pausing, const std::string& text, const std::string& title,
                                     std::int32_t seconds) = 0;
    // 006B8AD0(command, 0, 0, 2) on *(game+1A08h)+4; non-zero = failure. Then the Hungarian
    // diagnostic 004254B0 ("Nem sikerult vegrehajtani..." / "...lua parancs vegrehajtva...").
    virtual std::int32_t lua_run_string_006b8ad0(const std::string& command) = 0;
    virtual void diagnostic_lua_result_004254b0(bool failed, const std::string& command) = 0;
    // History maps of lists keyed by Type: +94h (005FD4B0 find, 006002B0 operator[]) and +7Ch.
    virtual bool history94_has_key(const std::string& key) = 0;
    virtual bool history94_erase_name(const std::string& key, const std::string& name) = 0;  // true when found
    virtual void history94_push_back(const std::string& key, const std::string& name) = 0;
    virtual void history7c_push_back(const std::string& key, const std::string& name) = 0;
    virtual void history_assign_single(const std::string& key, const std::string& name) = 0;  // both maps = {name}
};

// 00690FD0: __thiscall(tracker, NativeString* name, char forced), RET 8; SEH 00C7E723.
void hint_tracker_trigger_00690fd0(HintTrackerState& tracker, HintTrackerHost& host, const std::string& name,
                                   bool forced);

// ---------------------------------------------------------------------------
// Idle camera screen 2Ch (HUD manager +94h, constructor 0054F8C0, primary vtable 00CEE408,
// observer sub-object at +8 with vtable 00CEE3F0; member of the idle-camera interface set
// {2Ch, 29h, 49h}). It cycles a camera through named eye/target entity pairs.
// ---------------------------------------------------------------------------

inline constexpr float kIdleCameraShotSeconds = 6.0F;     // 00CE6630
inline constexpr float kIdleCameraStartTimer = 10.0F;     // 00CE38B8, above 6 so Update switches at once
inline constexpr std::int32_t kIdleCameraNextAction = 0x11A;  // 004C43C0
inline constexpr std::int32_t kIdleCameraPrevAction = 0x11B;
inline constexpr std::uint32_t kIdleCameraNodeBytes = 0x384;  // operator new, memset, 00432750

// One 1Ch-byte shot of the vectors at 00E19328 + mode*10h (allied, nation 0) and
// 00E193A8 + mode*10h (japanese, nation 1); begin pointers at +4 (00E1932C / 00E193AC).
struct IdleCameraShot {
    std::uintptr_t eye_entity{};     // +00h, resolved by Enter from eye_name
    std::uintptr_t target_entity{};  // +04h, resolved by Enter from target_name
    std::string eye_name;            // +08h (data +0Ch; null reads as 00E1931C "")
    std::string target_name;         // +10h (data +14h)
    bool enabled{};                  // +18h, both entities found
};

struct IdleCameraState {
    std::uintptr_t camera_1c{};      // +1Ch, the camera scene node (cleared by the observer slot)
    float timer_20{};                // +20h, seconds on the current shot
    std::int32_t shot_24{};          // +24h, current shot index
};

struct IdleCameraHost {
    virtual ~IdleCameraHost() = default;
    virtual std::int32_t game_effective_mode_004bca50() = 0;
    virtual std::int32_t local_side_nation_28() = 0;   // *(game+18CCh + game+18ECh*4)+28h
    // The shot vector of a nation (0 allied, 1 japanese) for a game mode; null while its begin
    // pointer is null.
    virtual std::vector<IdleCameraShot>* shots(std::int32_t nation, std::int32_t mode) = 0;
    virtual std::uintptr_t create_camera_00432750() = 0;                  // 384h bytes, zeroed
    // camera.00923870(0, game+19CCh, identity 4x4)
    virtual void camera_attach_00923870(std::uintptr_t camera) = 0;
    virtual bool observer_registered_00694af0(std::uintptr_t observed) = 0;   // EDX = screen+8
    virtual void observer_register_00694a60(std::uintptr_t observed) = 0;     // EDX = screen+8
    virtual void observer_unregister_006952a0(std::uintptr_t observed) = 0;   // EDX = screen+8
    // node = game+19FCh: 00B6DB70 when (node+5Ch & 2) == 0, then 004134F0(camera+74h, node+F0h).
    virtual void camera_copy_game_view_19fc(std::uintptr_t camera) = 0;
    virtual void game_set_camera_mover_004bc410(std::uintptr_t camera) = 0;
    virtual std::uintptr_t find_entity_00925a90(const std::string& name) = 0;  // ECX = game+19CCh
    virtual bool input_action_pressed_004c43c0(std::int32_t action) = 0;
    virtual bool hud_bc_blocks_input_006529e0() = 0;     // ECX = *(*(00E198C4)+BCh)
    virtual bool entity_pose_fresh_c8(std::uintptr_t entity) = 0;
    virtual void entity_refresh_pose_00414db0(std::uintptr_t entity) = 0;
    virtual std::array<float, 3> entity_position_fc(std::uintptr_t entity) = 0;
    using Matrix = std::array<float, 16>;
    virtual Matrix look_at_00b63f10(const std::array<float, 3>& eye, const std::array<float, 3>& target,
                                    const std::array<float, 3>& up) = 0;
    virtual Matrix invert_affine_00b63b30(const Matrix& m) = 0;
    virtual void camera_set_world_004134f0(std::uintptr_t camera, const Matrix& m) = 0;  // camera+74h
    virtual void invalidate_subtree_0042ed50(std::uintptr_t camera) = 0;
};

// 0054F900: slot +18h of vtable 00CEE408, __thiscall(screen), RET; SEH frame.
void idle_camera_enter_0054f900(IdleCameraState& screen, IdleCameraHost& host);
// 0054EFC0: slot +1Ch, __thiscall(screen), RET.
void idle_camera_exit_0054efc0(IdleCameraState& screen, IdleCameraHost& host);
// 00550410: slot +20h, __thiscall(screen, float dt), RET 4.
void idle_camera_update_00550410(IdleCameraState& screen, IdleCameraHost& host, float dt);
// 0054F3E0: observer vtable 00CEE3F0 +04h, __thiscall(screen+8, observed), RET 4.
void idle_camera_on_released_0054f3e0(IdleCameraState& screen, IdleCameraHost& host, std::uintptr_t observed);

}  // namespace bsp
