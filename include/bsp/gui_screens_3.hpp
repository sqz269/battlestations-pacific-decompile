#pragma once

// HUD and GUI screens of partition segments 16, 28 and 4 (packet cc13_gui5_screens3).
// Evidence: docs/CC13_GUI_SCREENS_3.md. Every name here is a hypothesis; addresses,
// ABIs and listing ranges are in the comments of src/gui_screens_3.cpp.

#include <array>
#include <cstdint>
#include <map>
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
    virtual void game_set_gui_layers_visible_004d9830(bool visible) = 0;  // restores the GUI layers
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

// Idle camera shot loader 00551440: the scene header property "IdleCameraPos" (applier
// 004F1D70 at 004F220A..004F2218, owned by cc9's mission-load lane) fills both shot arrays.
inline constexpr std::string_view kIdleCameraPosProperty = "IdleCameraPos";  // 00CEA420
inline constexpr std::array<std::string_view, 8> kIdleCameraModeKeys{
    "Capture1",     // 00CEE480, mode 0
    "Capture2",     // 00CEE474
    "Capture3",     // 00CEE468
    "Capture4",     // 00CEE45C
    "Duel",         // 00CE73BC
    "Escort",       // 00CE73B4
    "Siege",        // 00CE73AC
    "Competitive",  // 00CE73A0, mode 7
};
inline constexpr std::string_view kIdleCameraAlliedKey = "Allied";     // 00CEE454 -> 00E19328
inline constexpr std::string_view kIdleCameraJapaneseKey = "Japanese";  // 00CEE430 -> 00E193A8
inline constexpr std::string_view kIdleCameraPairFormat = "Pair%02d";  // 00CEE448
inline constexpr std::string_view kIdleCameraFromKey = "From";         // 00CEE440, eye
inline constexpr std::string_view kIdleCameraToKey = "To";             // 00CEE43C, target
inline constexpr std::int32_t kIdleCameraMaxPairs = 4;                 // 0055183A / 00551AB3 CMP EAX,4

// A scene property table as 00551440 reads it: 008F2260 find, then the found node's +0Ch for a
// sub-table or 008F0DF0 for a string. The image tests neither the find result nor the kind
// below the root.
struct IdleCameraPosTable {
    virtual ~IdleCameraPosTable() = default;
    virtual const IdleCameraPosTable* table(std::string_view key) const = 0;
    virtual const char* string(std::string_view key) const = 0;  // may be null
};

using IdleCameraShotArray = std::array<std::vector<IdleCameraShot>, 8>;

// 00551440: __fastcall(property node or null /*ECX*/), RET. `root` is the node's +0Ch table and
// is used only when the node exists and its kind (+4) is 6.
void idle_camera_load_shots_00551440(IdleCameraShotArray& allied_e19328, IdleCameraShotArray& japanese_e193a8,
                                     const IdleCameraPosTable* root, std::int32_t root_kind);

// 0054F900: slot +18h of vtable 00CEE408, __thiscall(screen), RET; SEH frame.
void idle_camera_enter_0054f900(IdleCameraState& screen, IdleCameraHost& host);
// 0054EFC0: slot +1Ch, __thiscall(screen), RET.
void idle_camera_exit_0054efc0(IdleCameraState& screen, IdleCameraHost& host);
// 00550410: slot +20h, __thiscall(screen, float dt), RET 4.
void idle_camera_update_00550410(IdleCameraState& screen, IdleCameraHost& host, float dt);
// 0054F3E0: observer vtable 00CEE3F0 +04h, __thiscall(screen+8, observed), RET 4.
void idle_camera_on_released_0054f3e0(IdleCameraState& screen, IdleCameraHost& host, std::uintptr_t observed);

// ---------------------------------------------------------------------------
// Hint tracker stored-hint helpers. 00690B20 and 00690C30 back the HintSystem Lua bindings
// AddStoredHint and RemoveStoredHint (table 00E0B7F0: ShowHint 008D1F50, ShowHintForced
// 008D2190, AddStoredHint 008D24F0, RemoveStoredHint 008D2340, HideHint 0088CEC0,
// IsHintActive 008D2850).
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kHintUnitClassReset = 0x13;  // tracker+18h after a removal (00690C5C)

struct HintStoreHost {
    virtual ~HintStoreHost() = default;
    virtual const HintDefinition* find_definition_0068e450(const std::string& name) = 0;  // null = end()
    virtual bool has_been_shown_0068e600(const std::string& name) = 0;
    virtual bool profile_has_seen_007f8e00(const std::string& name) = 0;   // ECX = game+650h
    virtual void profile_mark_seen_007fae20(const std::string& name) = 0;  // ECX = game+650h
    virtual void profile_unmark_007fae60(const std::string& name) = 0;     // game+650h+8 -> 005FE8B0
    virtual void mission_seen_push_back_48(const std::string& name) = 0;   // 004D2660 on +48h
    // The list +48h, rebuilt by 0068EE00.
    virtual std::vector<std::string>& mission_seen_48() = 0;
    virtual bool player_unit_present_e188d8() = 0;
    virtual std::int32_t player_unit_class_170() = 0;   // (*(unit+170h))->vtable[0]()
    virtual void history7c_clear_00690ca0() = 0;        // tree at +7Ch emptied (004DCDB0 on the root)
    virtual void history94_clear() = 0;                 // tree at +94h emptied (00690CEC..00690D1B)
    virtual bool history94_has_key(const std::string& key) = 0;   // 005FD120 lower bound + compare
    virtual void history94_push_back(const std::string& key, const std::string& name) = 0;
    virtual void history94_assign_single(const std::string& key, const std::string& name) = 0;
    virtual std::vector<std::string> profile_shown_names_65c() = 0;  // list at *(game+65Ch)
};

struct HintStoreState {
    std::int32_t last_unit_class_18{};  // +18h
};

// 00690B20: __thiscall(tracker, NativeString* name), RET 4; Lua AddStoredHint.
void hint_tracker_add_stored_00690b20(HintStoreHost& host, const std::string& name);
// 00690C30: __thiscall(tracker, NativeString* name), RET 4; Lua RemoveStoredHint.
void hint_tracker_remove_stored_00690c30(HintStoreState& tracker, HintStoreHost& host, const std::string& name);
// 0068EE00: __thiscall(tracker, NativeString* name), RET 4; rebuilds +48h without the name.
void hint_tracker_drop_mission_seen_0068ee00(HintStoreHost& host, const std::string& name);
// 00690CA0: __fastcall(tracker), RET; called by the pause screen's Register 00602280.
void hint_tracker_clear_new_history_00690ca0(HintStoreHost& host);
// 00690CD0: __fastcall(tracker), RET; called by 005922F0 (play selected mission) and 006921B0.
void hint_tracker_rebuild_history_00690cd0(HintStoreHost& host);

// ---------------------------------------------------------------------------
// Hint definition loading: 006921B0 (reload) runs Scripts/datatables/HintSystemMessages.lua and
// hands five sub-tables to 0068F030.
// ---------------------------------------------------------------------------

inline constexpr std::string_view kHintScript = "Scripts/datatables/HintSystemMessages.lua";  // 00692249
inline constexpr std::string_view kHintRootTable = "HintSystemMessages";   // 00CF7CA4
inline constexpr std::string_view kHintDefaultType = "hints.gameplay_general_hint";  // 00CF7B10
inline constexpr std::string_view kHintDefaultRule = "repeatable";          // 00CF7B48
inline constexpr std::int32_t kHintDefaultDuration = 12;                    // 0069008A
inline constexpr std::size_t kHintTitleDescRows = 8;

// One entry of a hint table as the Lua object accessors see it (00B67800 get_by_name, 00B65FB0
// is_nil, 00B662B0 string, 00B66250 boolean, 00B66290 integer, 00B67080/00B67190 iteration).
struct HintLuaEntry {
    virtual ~HintLuaEntry() = default;
    virtual std::string id() = 0;                        // "id" (00CF16BC), read without a nil test
    virtual bool is_nil(std::string_view key) = 0;
    virtual std::string string(std::string_view key) = 0;
    virtual bool boolean(std::string_view key) = 0;
    virtual std::int32_t integer(std::string_view key) = 0;
    virtual std::size_t row_count() = 0;                 // TitleDescList elements in iteration order
    virtual bool row_is_nil(std::size_t row, std::string_view key) = 0;
    virtual std::string row_string(std::size_t row, std::string_view key) = 0;
};

struct HintTableHost {
    virtual ~HintTableHost() = default;
    // Iterates one sub-table's entries; null at the end.
    virtual HintLuaEntry* next_entry() = 0;
    virtual void store_definition_005fdd90(const std::string& id, const HintDefinition& def) = 0;  // map+30h[id] = def
};

// 0068F030: __thiscall(tracker, LuaObject table by value, char other_category), RET 18h.
// A missing cooldowntime gives 0 (0068FFF4 stores EDI = 0); nothing carries between entries.
void hint_tracker_load_definitions_0068f030(HintTableHost& host, bool other_category);

struct HintReloadHost {
    virtual ~HintReloadHost() = default;
    virtual void history7c_clear() = 0;                       // 004DCDB0 on +7Ch (006921C0..)
    virtual bool run_script_with_overrides(std::string_view path) = 0;   // fresh LuaStateOwner
    virtual void definitions_clear_004d7370() = 0;            // map +30h emptied
    // Selects the sub-table HintSystemMessages.<group>.<name> for the next loader pass.
    virtual void select_table(std::string_view group, std::string_view name) = 0;
    virtual void load_selected_0068f030(bool other_category) = 0;
    virtual void rebuild_history_00690cd0() = 0;
    virtual void close_script() = 0;
};

struct HintReloadState {
    bool force_next_04{};
    std::int32_t last_unit_class_18{};
    std::string last_timed_1c;
    float last_timed_seconds_24{};
    std::string active_60;
};

// 006921B0: __fastcall(tracker), RET.
void hint_tracker_reload_006921b0(HintReloadState& tracker, HintReloadHost& host);

// ---------------------------------------------------------------------------
// Segment 10: GUI parking of in-game screens (game singleton 00E188A8 methods).
// ---------------------------------------------------------------------------

inline constexpr std::int32_t kGuiScreenSlots = 0x5F;          // 004D6725: screens 0..5Eh of 00E18B60
inline constexpr double kGuiParkOffsetX = 1000.0;               // 00CE47A0
// Screens left in place while a hint pauses the game (004D94F0): 28h, 55h (menu prompt),
// 57h (frame stats), 5Dh, 34h (hints).
inline constexpr std::array<std::int32_t, 5> kHintPauseKeptScreens{0x28, 0x55, 0x57, 0x5D, 0x34};
// Screens left in place while the game+61Fh pause is on (004D95F0).
inline constexpr std::array<std::int32_t, 16> kSystemPauseKeptScreens{
    0x0D, 0x0E, 0x0F, 0x28, 0x3C, 0x3D, 0x53, 0x55, 0x57, 0x58, 0x59, 0x5C, 0x5D, 0x19, 0x5E, 0x0C};

struct GuiParkHost {
    virtual ~GuiParkHost() = default;
    virtual bool screen_present_e18b60(std::int32_t slot) = 0;
    virtual std::vector<std::uintptr_t> screen_collect_24(std::int32_t slot) = 0;  // screen vtable +24h
    virtual bool widget_parked_121(std::uintptr_t widget) = 0;
    virtual void widget_set_parked_121(std::uintptr_t widget, bool parked) = 0;
    virtual std::array<float, 3> widget_resolved_position_00aa6750(std::uintptr_t widget) = 0;
    virtual void widget_set_resolved_position_00aa8240(std::uintptr_t widget, const std::array<float, 3>& p) = 0;
};

// 004D6480: __stdcall-like (std::vector<int>* kept, char park), RET 8 (ECX unused).
void gui_park_screens_004d6480(GuiParkHost& host, const std::vector<std::int32_t>& kept, bool park);

struct GamePauseState {
    bool system_pause_61f{};   // game+61Fh
    bool hint_pause_620{};     // game+620h
    std::int32_t pause_lock_624{};  // game+624h, blocks 004D94F0 when non-zero
    std::int32_t session_kind_1fe4{};
};

struct GamePauseHost : GuiParkHost {
    virtual void hints_advance_basic_step_0054e440() = 0;     // ECX = *(*(00E198C4)+A8h)
    virtual bool hud_screen_e8_flag() = 0;                    // 00E198C4 && +E8h && *(+E8h)+4
    virtual void input_force_refresh() = 0;                   // input+20h = 1; 00A93020
};

// 004D94F0: __thiscall(game, char paused), RET 4; SEH 00C66588.
void game_set_hint_pause_004d94f0(GamePauseState& game, GamePauseHost& host, bool paused);
// 004D95F0: __thiscall(game, char paused), RET 4.
void game_set_system_pause_004d95f0(GamePauseState& game, GamePauseHost& host, bool paused);

// ---------------------------------------------------------------------------
// 004D9830: GUI layer visibility (game+61Eh flag, saved factors in the map game+628h).
// The cg_scalar_deleting_dtor tag is wrong: it is a direct-call game method, called with 0 by
// the hi-res screenshot 004DC020 and with 1 by the hints screen (0054E674, 0054EA52).
// ---------------------------------------------------------------------------

inline constexpr std::string_view kGuiKeepConsoleNode = "_Console";  // 00CE7AA0, 004D99B5
inline constexpr std::string_view kGuiKeepFpsNode = "_Fps";          // 00CE7A98, 004D9A19

struct GuiLayerVisibilityHost {
    virtual ~GuiLayerVisibilityHost() = default;
    // GuiManager 004C12B0, map at +8 in order; for each value v the nodes start at
    // *(*(v+1Ch)+0Ch) and chain through node+3Ch.
    virtual std::vector<std::uintptr_t> gui_layer_nodes() = 0;
    virtual std::string node_name_00b6d800(std::uintptr_t node) = 0;
    virtual float node_visibility_ac(std::uintptr_t node) = 0;
    virtual void node_set_visibility_00b6da70(std::uintptr_t node, float value, bool recurse) = 0;
};

struct GuiLayerVisibilityState {
    bool gui_visible_61e{};                       // game+61Eh
    std::map<std::uintptr_t, float> saved_628;    // game+628h (head +62Ch, size +630h), 004D69D0 operator[]
};

// 004D9830: __thiscall(game, char visible), RET 4; SEH 00C665F2.
void game_set_gui_layers_visible_004d9830(GuiLayerVisibilityState& game, GuiLayerVisibilityHost& host,
                                          bool visible);

// ---------------------------------------------------------------------------
// 004E7220: scene header "Cloud" areas (004F1D70 at 004F2368 with the "Cloud" table 00CE7524,
// and 004EE450 at 004EE481 passing its own table). Fills record+C84h/+C88h, then tail-calls
// 004BA870 BSP_SceneRecord_ScatterClouds (docs/SCENE_CONTENTS_HOSTS.md calls +C84h a gate; it
// is this area count).
// ---------------------------------------------------------------------------

inline constexpr std::string_view kCloudAreaFormat = "Area%02d";  // 00CE8B98
inline constexpr std::int32_t kCloudAreaSlots = 5;                 // 004E73F1 CMP EBP,5

// One 28h-byte area at record+C88h + n*28h.
struct SceneCloudArea {
    std::array<float, 3> box_min{};  // +00h BoxMinX/Y/Z (00CE8B8C, 00CE8B84, 00CE8B7C)
    std::array<float, 3> box_max{};  // +0Ch BoxMaxX/Y/Z (00CE8B74, 00CE8B6C, 00CE8B64)
    std::int32_t num{};              // +18h Num (00CE8B94), copied raw
    float small_weight{};            // +1Ch SmallWeight (00CE8B58)
    float medium_weight{};           // +20h MediumWeight (00CE8B48)
    float big_weight{};              // +24h BigWeight (00CE8B3C)
};

struct SceneCloudRecord {
    std::int32_t area_count_c84{};                          // +C84h
    std::array<SceneCloudArea, kCloudAreaSlots> areas{};    // +C88h
};

// A scene property node as 004E7220 reads it: kind +4 (0 int, else float bits; 6 table) and
// the value +0Ch. find() is 008F2260 on a table.
struct ScenePropertyValue {
    std::int32_t kind{};
    std::int32_t int_value{};
    float float_value{};
};
struct ScenePropertyTable {
    virtual ~ScenePropertyTable() = default;
    virtual ScenePropertyValue value(std::string_view key) const = 0;        // unchecked find
    virtual const ScenePropertyTable* table(std::string_view key) const = 0;  // kind 6 and +0Ch != 0, else null
};

struct SceneCloudHost {
    virtual ~SceneCloudHost() = default;
    virtual void scatter_clouds_004ba870() = 0;  // ECX = record, tail call at 004E7403
};

// 004E7220: __fastcall(table /*ECX*/, record /*EDX*/), RET (the tail JMP).
void scene_load_cloud_areas_004e7220(SceneCloudRecord& record, SceneCloudHost& host, const ScenePropertyTable& cloud);

// ---------------------------------------------------------------------------
// HintSystem mission-Lua wrappers (binding table 00E0B7B8, rows at 00E0B7F0..00E0B830; the
// host registration list is src/mission_lua_host.cpp, owned by cc9). Every wrapper is
// __fastcall(lua_State* L /*ECX*/) returning the number of results: it borrows L
// (00B66C00), opens a call frame (00B679B0), works, and returns 00B66400 (values pushed
// above the arguments). Each first builds a dead "luaMW_<Name> failed:" NativeString and a
// function-static "luakod" string; neither is used afterwards.
// ---------------------------------------------------------------------------

struct HintLuaHost {
    virtual ~HintLuaHost() = default;
    virtual std::int32_t argument_count_00b663f0() = 0;           // frame+0Ch
    virtual std::string argument_string(std::int32_t index) = 0;  // 00B677E0 then 00B662B0
    virtual std::int32_t game_termination_reason_624() = 0;       // game+624h
    // Tracker singleton 004E1CA0 (00E198D0).
    virtual void tracker_trigger_hint_00690fd0(const std::string& name, bool requested) = 0;
    virtual void tracker_trigger_forced_00692fb0(const std::string& name) = 0;
    virtual void tracker_add_stored_00690b20(const std::string& name) = 0;
    virtual void tracker_remove_stored_00690c30(const std::string& name) = 0;
    virtual std::string tracker_active_hint_0068e5b0() = 0;
    virtual bool tracker_definition_critical_00690b00(const std::string& name) = 0;
    virtual bool tracker_has_been_shown_0068e600(const std::string& name) = 0;
    // 00770870 on game+1EF0h (the session): mode 1 (host) sends message 34 with the name.
    virtual void session_send_hint_00770870(const std::string& name) = 0;
    // The hints screen *(*(00E198C4)+A8h).
    virtual void hints_dismiss_0054e610() = 0;
    virtual std::int32_t hints_seconds_left_0054d3c0() = 0;
    // Results.
    virtual void push_boolean(bool value) = 0;                         // 00B66450
    virtual void push_integer(std::int32_t value) = 0;                 // 00B664B0
    virtual void push_hint_table(const std::string& id, std::int32_t duration) = 0;  // 00B67930 + fields
    virtual std::int32_t result_count_00b66400() = 0;
};

// 008D1F50 ShowHint(name): ignored once game+624h is set; else TriggerHint(name, 1) and the
// session copy.
std::int32_t lua_show_hint_008d1f50(HintLuaHost& host);
// 008D2190 ShowHintForced(name): 00692FB0 (tracker+4 = 1, then TriggerHint(name, 1)); no gate.
std::int32_t lua_show_hint_forced_008d2190(HintLuaHost& host);
// 008D24F0 AddStoredHint(name) / 008D2340 RemoveStoredHint(name).
std::int32_t lua_add_stored_hint_008d24f0(HintLuaHost& host);
std::int32_t lua_remove_stored_hint_008d2340(HintLuaHost& host);
// 0088CEC0 HideHint(): dismisses the hints screen.
std::int32_t lua_hide_hint_0088cec0(HintLuaHost& host);
// 008D2850 IsHintActive([name]).
std::int32_t lua_is_hint_active_008d2850(HintLuaHost& host);
// 008D26A0 IsHintCritical(name) / 008D2BB0 IsHintStored(name).
std::int32_t lua_is_hint_critical_008d26a0(HintLuaHost& host);
std::int32_t lua_is_hint_stored_008d2bb0(HintLuaHost& host);

}  // namespace bsp
