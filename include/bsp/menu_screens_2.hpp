#pragma once

// Front-end and in-game screens of partition segments 12, 15 and 19 (packet cc13-menus4).
// Evidence: docs/CC13_MENU_SCREENS_2.md. Every name here is a hypothesis; addresses,
// ABIs and listing ranges are in the comments of src/menu_screens_2.cpp.

#include "bsp/main_menu_command_bar.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Multiplayer session browser (screen 1Ch, INTF_MULTISESSIONBROWSER, constructor
// 00574240, 2D0h bytes, primary vtable 00CEF134)
// ---------------------------------------------------------------------------

inline constexpr std::string_view kSessionBrowserTitle = "FE.multi_browser_title";  // 00CEF22C
inline constexpr float kSessionBrowserRefreshSeconds = 10.0F;    // 00CE38B8 -> +268h
inline constexpr float kSessionBrowserPollSeconds = 2.5F;        // 00CEECEC -> +270h
inline constexpr float kSessionBrowserJoinTimeout = 12.0F;       // 00CEECF4 -> +284h
inline constexpr float kSessionBrowserJoinStepCap = 0.1F;        // 00D7A2F0
inline constexpr std::uint32_t kSessionBrowserRecordBytes = 0xA0;  // (end - begin) / 0A0h
inline constexpr std::int32_t kSessionBrowserJoinStatusReady = 0x217;  // 00A39750 result
inline constexpr std::size_t kSessionBrowserEditFilterBase = 9;          // +19h - +10h

// The screen's widget pointers that these routines dereference.
enum class SessionBrowserWidget : std::uint16_t {
    FilterPanel2C = 0x2C,  // virtual +38h answers "open"; byte +85h tested
    SessionList30 = 0x30,  // the result listbox; byte +85h tested
    FilterList34 = 0x34,
    Toggle3C = 0x3C,       // virtual +84h(state), paired with +50h
    Toggle40 = 0x40,       // virtual +84h(state), paired with +51h
    EmptyText4C = 0x4C,    // shown when no session is listed
    Widget74 = 0x74,
    Widget84 = 0x84,       // passed to 0056B610
    Widget88 = 0x88,
    WidgetD4 = 0xD4,
};

// Fields of the 2D0h-byte screen that the routines below read or write.
struct SessionBrowserState {
    bool base_byte_04{};             // +04h, base-screen byte; SetPage acts only with +04h and +05h set
    bool base_byte_05{};             // +05h
    std::uint32_t field_24{};        // +24h, argument of 0076A7A0
    // +10h..+21h: nine applied filter bytes (+10h..+18h; 0..4 game types, 5..7 count filters, 8 a
    // yes/no filter) followed by their nine-byte edit copy (+19h..+21h); 0056E360 copies applied to
    // edit, 0057A910 edit to applied.
    std::array<bool, 18> filter_bytes{};
    std::array<std::int32_t, 3> edit_counts{};  // +250h, +254h (0..7), +258h (0..500 step 100)
    bool toggle_50{};                // +50h, pairs with widget +3Ch
    bool toggle_51{};                // +51h, pairs with widget +40h
    bool filters_shown_f0{};         // +F0h
    bool filters_reset_f1{};         // +F1h
    std::int32_t scroller_1e4_204{};  // +204h (scroller +1E4h, field +20h)
    bool scroller_1e4_22c{};         // +22Ch (scroller +1E4h, field +48h)
    float field_240{};               // +240h
    bool refresh_pending{};          // +265h
    float refresh_timer{};           // +268h
    std::uint32_t refresh_row{};     // +26Ch, 00425E50 selected row of the list
    float poll_timer{};              // +270h
    std::int32_t field_274{-1};      // +274h
    float join_elapsed{};            // +284h
    bool join_cancel_requested{};    // +288h
    std::int32_t page{};             // +28Ch: 0 search, 1 results, 2 joining
    std::int32_t field_2cc{};        // +2CCh, compared around 0056B610
};

// The global block 00E08700.. handed to the session client's +88h virtual.
struct SessionSearchRequest {
    bool flag_e08700{};              // 00E08700, tested by the join wait
    bool refresh_target_e08702{};    // 00E08702
    std::uint32_t target_e08710{};   // 00E08710
    std::uint32_t target_e08714{};   // 00E08714
    std::uint32_t type_mask_e08730{};  // 00E08730
};

// One session record (0A0h bytes in the vector at +2A0h); only the fields read here.
struct SessionRecordView {
    std::uint8_t kind_04{};          // record+04h, 0FFh tested
    bool has_target{};               // record+00h is non-null
    std::uint32_t target_04{};       // *(record+00h)+04h
    std::uint32_t target_08{};       // *(record+00h)+08h
};

struct SessionBrowserHost {
    virtual ~SessionBrowserHost() = default;
    // Session client *(00F8A2FC).
    virtual bool session_query(std::uint16_t slot) = 0;  // +90h, +198h (signed in), +19Ch
    virtual void session_call(std::uint16_t slot) = 0;   // +8Ch, +94h
    virtual void session_start_search_88(SessionSearchRequest& request) = 0;
    virtual SessionSearchRequest& search_request_e08700() = 0;
    // Game *(00E188A8).
    virtual bool online_1fe4() = 0;                       // game+1FE4h != 0
    virtual std::int32_t mode_9c() = 0;                   // *(game+207Ch)+9Ch
    virtual float frame_seconds_21ec() = 0;               // game+21ECh
    virtual std::int32_t occupied_player_slots() = 0;     // bytes game+750h + i*118h, i = 0..7
    virtual std::int32_t field_18c8() = 0;                // game+18C8h
    virtual bool check_004b6bc0() = 0;                    // ECX = game
    virtual std::int32_t join_status_00a39750() = 0;      // ECX = *(game+207Ch)+C8h
    virtual void session_set_00076a7a0(std::uint32_t value) = 0;  // ECX = game+1EF0h
    virtual bool action_004d92b0(std::int32_t action) = 0;        // ECX = game
    virtual bool pressed_004c43c0(std::int32_t action) = 0;       // ECX = game
    virtual bool held_004c5090(std::int32_t action) = 0;          // ECX = game
    // Filter global 00E086C0 (-1 = none).
    virtual std::int32_t game_type_filter_e086c0() = 0;
    virtual void set_game_type_filter_e086c0(std::int32_t value) = 0;
    // Widgets.
    virtual void widget_virtual(SessionBrowserWidget widget, std::uint16_t slot, std::int32_t arg) = 0;  // +34h/+60h/+84h
    virtual void widget_virtual_40(SessionBrowserWidget widget, float value) = 0;
    virtual bool widget_open_38(SessionBrowserWidget widget) = 0;
    virtual std::uintptr_t widget_identity(SessionBrowserWidget widget) = 0;  // the pointer value
    virtual bool widget_byte_85(SessionBrowserWidget widget) = 0;
    virtual void widget_set_byte_77(SessionBrowserWidget widget, bool value) = 0;
    virtual void list_row_set_byte_77(SessionBrowserWidget list, std::int32_t row, bool value) = 0;  // 00A9BE00(row)
    virtual void list_row_virtual_34(SessionBrowserWidget list, std::int32_t row, std::int32_t arg) = 0;
    virtual void list_set_listener_00a9ac40(SessionBrowserWidget list, bool screen_plus_8) = 0;
    virtual bool list_has_rows_00a9ac50() = 0;            // *(00A9AC50(list +30h))+8 != 0
    virtual void list_clear_00a9cdc0(SessionBrowserWidget list) = 0;
    virtual void list_clear_00a9bf90(SessionBrowserWidget list) = 0;
    virtual std::int32_t list_absolute_selection_005661c0() = 0;   // list +30h
    virtual std::int32_t list_selected_index_00a9c920() = 0;       // list +30h
    virtual std::uint32_t list_selected_row_00425e50() = 0;        // list +30h
    virtual void reset_cucc_group_00aa83f0() = 0;  // 00AA7E00("cucc_Group", 1) on +2Ch, then 00AA83F0
    virtual void scroller_update_00683820(std::uint16_t scroller_offset) = 0;  // screen+offset, arg 1
    // Session records, vector +2A0h.
    virtual std::size_t session_count_00566310() = 0;
    virtual SessionRecordView session_record_005663b0(std::size_t index) = 0;
    // Please-wait screen *(00E19698), command bar *(00E1930C), interface manager *(00E198B4).
    virtual void please_wait_clear_byte_4() = 0;
    virtual void please_wait_show_004b6e30() = 0;
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void cancel_session_00688c70() = 0;
    virtual void pop_request_interface_006881f0() = 0;
    virtual void push_request_interface_logged_00688ad0(std::int32_t id) = 0;
    virtual void error_screen_show_005d2e70(std::int32_t kind) = 0;   // ECX = *(*(00E198B4)+50h)
    virtual void input_owner_update_00a92c40(float seconds) = 0;      // 004BEC00 singleton
    virtual void show_gamer_card_00a4d440(std::uint32_t user, std::uint64_t xuid) = 0;  // XShowGamerCardUI
    virtual std::uint32_t signed_in_user_00a3eac0() = 0;              // ECX = *(00F8ABE8)
    virtual void set_frame_title_00518d60(std::string_view title) = 0;  // ECX = 0Ah, EDX = 0, stack 0
    // Screen-local helpers not reconstructed here (ECX = screen unless noted).
    virtual void prepare_00570300() = 0;
    virtual void layout_00568930() = 0;
    virtual void filters_tick_0056dd80() = 0;
    virtual void command_bar_tick_0056e3e0() = 0;
    virtual void begin_filter_edit_0056e360() = 0;
    virtual void apply_filter_edit_0057a910() = 0;
    virtual void build_rows_default_00576d20() = 0;
    virtual void build_rows_005789c0(std::int32_t count) = 0;
    virtual void refresh_rows_00577650() = 0;
    virtual void select_first_0056aa70() = 0;              // arg = list +30h
    virtual void reset_search_0056ac40() = 0;              // ECX = screen+278h
    virtual void scroll_follow_0056b610() = 0;             // (screen+1E4h, list +30h, widget +84h, 0)
    virtual void poll_session_list_0057b530() = 0;
    virtual void back_from_results_00568cb0() = 0;
    virtual void list_action_00568df0(std::int32_t arg) = 0;
    virtual void filter_left_0056ee00() = 0;
    virtual void filter_right_0056efb0() = 0;
    virtual void list_left_0056aad0() = 0;
    virtual std::uint64_t selected_player_xuid_00568d40() = 0;
    virtual void join_session_0057a9c0(std::size_t record_index, std::int32_t arg) = 0;
    virtual void join_finished_0057ac10() = 0;
};

// 0057A680: slot +18h of vtable 00CEF134, __thiscall(screen), RET.
void session_browser_enter_0057a680(SessionBrowserState& screen, SessionBrowserHost& host);
// 0056DCB0: slot +1Ch, __thiscall(screen), RET.
void session_browser_exit_0056dcb0(SessionBrowserState& screen, SessionBrowserHost& host);
// 0057A0F0: __thiscall(screen, int page), RET 4.
void session_browser_set_page_0057a0f0(SessionBrowserState& screen, std::int32_t page, SessionBrowserHost& host);
// 0057B600: slot +20h, __thiscall(screen, float seconds), RET 4.
void session_browser_update_0057b600(SessionBrowserState& screen, float seconds, SessionBrowserHost& host);
// 0057AE50: __thiscall(screen), RET; joins the selected listed session.
void session_browser_join_selected_0057ae50(SessionBrowserState& screen, SessionBrowserHost& host);
// 0057AEF0: slot +04h of the listbox listener 00CEF120 (ECX = screen+8), RET 8.
// The second stack argument is the event's listbox; the caller reports whether it is +30h.
void session_browser_on_listbox_activate_0057aef0(SessionBrowserState& screen, bool event_list_is_session_list,
    SessionBrowserHost& host);

// Filter-row value steps (0056EE00 on action 4Ch, 0056EFB0 on 4Dh) and their text setter 0056ED20.
inline constexpr std::string_view kSessionBrowserFilterValueText = "setting_value_Text";  // 00CEEFDC
inline constexpr std::string_view kSessionBrowserYes = "globals.yes";  // 00CED4F0
inline constexpr std::string_view kSessionBrowserNo = "globals.no";    // 00CED4E4

struct SessionBrowserFilterHost {
    virtual ~SessionBrowserFilterHost() = default;
    virtual std::int32_t filter_list_selected_index_00a9c920() = 0;   // list +34h
    virtual std::uintptr_t filter_list_selected_row_00425e50() = 0;   // list +34h
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t widget, std::string_view name) = 0;  // (name, 1)
    virtual void text_submit_00abaed0(std::uintptr_t text, std::string_view source) = 0;          // (source, 1)
    virtual std::string format_int_004260b0(std::int32_t value) = 0;  // BSP_NativeString_FromInt
    virtual void set_game_type_filter_e086c0(std::int32_t value) = 0;
};

// 0056ED20: __thiscall(screen, native string by value), RET 8.
void session_browser_set_filter_value_text_0056ed20(std::string_view text, SessionBrowserFilterHost& host);
// 0056EE00 (down) / 0056EFB0 (up): __thiscall(screen), RET.
void session_browser_filter_value_step_0056ee00(SessionBrowserState& screen, SessionBrowserFilterHost& host);
void session_browser_filter_value_step_0056efb0(SessionBrowserState& screen, SessionBrowserFilterHost& host);

// 0056E3E0: per-frame command bar of the browser, __thiscall(screen), RET; SEH 00C6ECB0.
// Inputs it reads besides the host: +50h, +51h and the byte +264h.
struct SessionBrowserCommandBarView {
    bool filter_list_focused{};      // *(+34h)+85h
    bool session_list_focused{};     // *(+30h)+85h
    bool toggle_50{};
    bool toggle_51{};
    bool byte_264{};                 // offers globals.join when set
    bool online{};                   // *(00E188A8)+1FE4h != 0
    std::size_t session_count{};     // 00566310 on +2A0h
    std::int32_t selection{};        // 005661C0 on +30h
    bool signed_in{};                // session client +198h (read only on the selected-session path)
};

// Returns false when the original returns without rebuilding (no list focused, +50h and +51h clear).
bool session_browser_command_bar_0056e3e0(const SessionBrowserCommandBarView& view,
    std::array<MainMenuCommandArgument, 5>& arguments);

// ---------------------------------------------------------------------------
// Credits screen (constructor 0052FCE0, primary vtable 00CED200)
// ---------------------------------------------------------------------------

inline constexpr double kCreditsSpeedStep = 0.10000000149011612;  // 00D7A3A0, (double)0.1F
inline constexpr float kCreditsSpeedMax = 1.0F;                   // 00D7A24C
inline constexpr float kCreditsSpeedMin = -1.0F;                  // 00D7A260
inline constexpr float kCreditsBaseSpeed = 0.044444446F;          // 00CED1DC -> +3Ch (constructor)
inline constexpr float kCreditsField40 = 0.5F;                    // 00CE3800 -> +40h (constructor)
inline constexpr float kCreditsField44 = 0.013888889F;            // 00CED1D8 -> +44h (constructor)

struct CreditsState {
    float speed_offset{};            // +20h, clamped to -1..1
    float scroll_y{};                // +34h, runs from 0 down to -height
    float content_height{};          // +38h
    float base_speed{kCreditsBaseSpeed};  // +3Ch
    bool has_scroll_widget{};        // +1Ch non-null
};

struct CreditsHost {
    virtual ~CreditsHost() = default;
    virtual float frame_seconds_21ec() = 0;                       // *(00E188A8)+21ECh
    virtual bool action_004d92b0(std::int32_t action) = 0;        // ECX = *(00E188A8)
    virtual void push_interface_request_004cc460(std::int32_t a, std::int32_t b) = 0;  // ECX = *(00E198AC)
    virtual void widget_set_local_y_00aa78f0(float y) = 0;        // ECX = +1Ch
};

// 0052E4B0: slot +20h of vtable 00CED200, __thiscall(screen, float seconds), RET 4. The
// argument is overwritten with *(00E188A8)+21ECh before use, so `seconds` is unused.
void credits_update_0052e4b0(CreditsState& screen, float seconds, CreditsHost& host);

// ---------------------------------------------------------------------------
// In-game screens that observe one unit: screen 23h (005178B0) and the bomber
// screen 25h (vtable 00CEC590)
// ---------------------------------------------------------------------------

struct ObservedUnitScreenState {
    std::uintptr_t unit_1c{};        // +1Ch, the observed unit (0 = none)
    std::uintptr_t field_28{};       // +28h, the bomber screen's second BindUnit argument
};

struct ObservedUnitScreenHost {
    virtual ~ObservedUnitScreenHost() = default;
    // ECX = subject, EDX = observer (screen+8).
    virtual bool observer_registered_00694af0(std::uintptr_t unit) = 0;
    virtual void observer_register_00694a60(std::uintptr_t unit) = 0;
    virtual void unit_send_0077c470(std::uintptr_t unit, std::int32_t code, std::int32_t arg) = 0;
    virtual std::uintptr_t unit_child_9d4(std::uintptr_t unit) = 0;     // unit+9D4h
    virtual void game_set_view_fov_004dc940(float fov, std::int32_t arg) = 0;  // ECX = *(00E188A8)
    // In-mission interface manager *(00E198C4).
    virtual void weapon_group_bind_unit_00549260(std::uintptr_t unit, std::uintptr_t arg) = 0;  // ECX = +50h
    virtual void weapon_group_release_005470c0() = 0;                   // ECX = +50h
    virtual void screen26_call_0051e8e0() = 0;                          // ECX = +4Ch, tail JMP
};

inline constexpr std::int32_t kObservedUnitReleaseCode = 0x1FE;  // 0077C470 first argument

// 00517820 / 00517860: slots +18h/+1Ch of vtable 00CEC194, __thiscall(screen), RET.
void in_game_screen23_enter_00517820(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host);
void in_game_screen23_exit_00517860(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host);
// 00519120 / 00519160: slots +18h/+1Ch of vtable 00CEC590, __thiscall(screen), RET.
void hud_bomber_enter_00519120(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host);
void hud_bomber_exit_00519160(ObservedUnitScreenState& screen, ObservedUnitScreenHost& host);

// ---------------------------------------------------------------------------
// HUD counters screen (51h, constructor 0052B030, vtable 00CED0AC)
// ---------------------------------------------------------------------------

struct HudCountersHost {
    virtual ~HudCountersHost() = default;
    virtual void widget8_virtual_34(std::int32_t arg) = 0;            // *(screen+8)
    virtual void counter_tick_0052b080(std::uintptr_t counter, float seconds) = 0;  // ECX = counter
    virtual void finish_00529810() = 0;                               // ECX = screen
};

struct HudCountersState {
    std::uintptr_t counter_0c{};                // +0Ch, ticked without a null test
    std::uintptr_t counter_10{};                // +10h
    std::vector<std::uintptr_t> counters_14;    // +14h array, +18h count; null entries skipped
};

// 00528C40 / 00528C50: slots +18h/+1Ch, __thiscall(screen), RET.
void hud_counters_enter_00528c40(HudCountersHost& host);
void hud_counters_exit_00528c50(HudCountersHost& host);
// 0052BAA0: slot +20h, __thiscall(screen, float seconds), RET 4.
void hud_counters_update_0052baa0(const HudCountersState& screen, float seconds, HudCountersHost& host);

// ---------------------------------------------------------------------------
// Tactical library (screen 0Bh, constructor 005098B0): listbox activate 00517280
// ---------------------------------------------------------------------------

inline constexpr std::string_view kTacticalLibrarySpecialRow = "99999";  // 00CEC170
inline constexpr double kTacticalLibraryScrollStep = 0.0020999999251216650;  // 00CEC168

struct TacticalLibraryScrollState {
    std::int32_t row_count{};        // screen+434h (listener+3D0h), loaded with FILD
    float scroll_value{};            // screen+438h (listener+3D4h)
};

struct TacticalLibraryActivateHost {
    virtual ~TacticalLibraryActivateHost() = default;
    // Row virtual +28h (text), compared by 00425850 BSP_NativeString_EqualsCStringInsensitive.
    virtual bool row_text_equals_insensitive(std::string_view text) = 0;
    virtual void open_special_00515e00() = 0;                          // ECX = screen
};

// 00517280: slot +04h of vtable 00CEB99C (ECX = screen+64h), stack (row, list), RET 8.
// `list_is_main_list` stands for list == listener+54h (screen+B8h).
void tactical_library_on_listbox_activate_00517280(TacticalLibraryScrollState& screen, bool list_is_main_list,
    TacticalLibraryActivateHost& host);

// Tactical library back/leave (00515E00 / 00515BC0).
struct TacticalLibraryNavState {
    std::int32_t view_90{};          // +90h, 1 while an entry view is open
    std::int32_t entry_kind_94{};    // +94h, 2/5/6 tested
    std::int32_t field_9c{};         // +9Ch, picks 0Bh or 0Ch for 00E08874
};

struct TacticalLibraryNavHost {
    virtual ~TacticalLibraryNavHost() = default;
    virtual bool online_1fe4() = 0;                         // *(00E188A8)+1FE4h != 0
    virtual bool front_end_manager_present_e198ac() = 0;    // *(00E198AC) != 0
    virtual bool interface_manager_present_e198b4() = 0;    // *(00E198B4) != 0
    virtual bool interface_query_00687860(std::int32_t id, std::int32_t arg) = 0;  // ECX = *(00E198B4)
    virtual void close_view_00511b40() = 0;                 // ECX = screen
    virtual void show_order_of_battle_page_004fff20() = 0;                // ECX = screen
    virtual void show_unlocks_page_004ffd00() = 0;                // ECX = screen
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void pop_request_interface_006881f0() = 0;      // ECX = *(00E198B4)
    virtual void set_return_screen_e08874(std::int32_t value) = 0;
    virtual void push_interface_request_004cc460(std::int32_t a, std::int32_t b) = 0;  // ECX = *(00E198AC)
};

// ---------------------------------------------------------------------------
// Front-end frame singleton (004C1AC0, 164h bytes, screen 0Ch; docs/GAME_TITLE_INIT.md):
// selection, flag and title setters 00518D60, 00518C80, 00518B90, 005189B0
// ---------------------------------------------------------------------------

inline constexpr std::size_t kFrontEndFrameBackdropSlots = 13;   // +0Ch, set stride 34h
inline constexpr std::size_t kFrontEndFramePanelSlots = 2;       // +110h, set stride 8h
inline constexpr std::size_t kFrontEndFrameTitleSlots = 2;       // +138h, set stride 8h
inline constexpr std::size_t kFrontEndFrameSets = 5;
inline constexpr std::int32_t kFrontEndFrameSkipBackdropMode = 0x0D;  // game+5D4h
inline constexpr double kFrontEndTitlePixelsPerUnit = 960.0;      // 00CEC380
inline constexpr double kFrontEndTitlePad = 0.26666666666666666;  // 00CEC3E8
inline constexpr float kFrontEndTitleMinWidth = 0.53333336F;      // 00CEC3E4
inline constexpr float kFrontEndTitleFixedWidth = 0.7F;           // 00CE3E18
inline constexpr std::string_view kFrontEndFrameFlagIcon = "frame_flag_Icon";     // 00CEC3F0
inline constexpr std::string_view kFrontEndFrameTitleText = "title_Text";         // 00CEC3D8
inline constexpr std::string_view kFrontEndFrameTitleBox = "szarnyas_FrameBox";   // 00CEC3C4

// Globals the setters share.
struct FrontEndFrameSelection {
    std::int32_t backdrop_e18d84{};
    std::int32_t panel_e18d88{};
    std::int32_t title_e18d8c{};
    std::uint32_t flag_e08654{};
};

// The singleton's handle arrays and its current set (+160h).
struct FrontEndFrameState {
    std::array<std::uintptr_t, kFrontEndFrameSets * kFrontEndFrameBackdropSlots> backdrops{};
    std::array<std::uintptr_t, kFrontEndFrameSets * kFrontEndFramePanelSlots> panels{};
    std::array<std::uintptr_t, kFrontEndFrameSets * kFrontEndFrameTitleSlots> titles{};
    std::int32_t set{};
};

struct FrontEndFrameHost {
    virtual ~FrontEndFrameHost() = default;
    virtual FrontEndFrameState& frame_004c1ac0() = 0;     // BSP_FrontEndFrame_GetOrCreate
    virtual FrontEndFrameSelection& selection() = 0;
    virtual void widget_virtual_34(std::uintptr_t widget, bool shown) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t layout, std::string_view name) = 0;  // (name, 1)
    virtual void widget_virtual_88(std::uintptr_t widget, std::uint16_t frame, std::int32_t arg, float value) = 0;
    virtual void text_submit_00abaed0(std::uintptr_t text, std::string_view source) = 0;  // (source, 1)
    virtual float text_extent_114(std::uintptr_t text) = 0;          // text+114h
    virtual float widget_height_00aa6740(std::uintptr_t widget) = 0; // returned vector +4
    virtual void widget_set_size_58(std::uintptr_t widget, float width, float height) = 0;  // virtual +58h(&size)
    virtual bool game_byte_61f() = 0;                    // *(00E188A8)+61Fh
    virtual bool game_byte_634() = 0;                    // *(00E188A8)+634h
    virtual std::int32_t game_mode_5d4() = 0;            // *(00E188A8)+5D4h
};

// 00518D60: ECX = backdrop slot, EDX = panel slot, stack (title slot, const string*), RET 8.
void front_end_frame_select_00518d60(std::int32_t backdrop, std::int32_t panel, std::int32_t title,
    std::string_view text, FrontEndFrameHost& host);
// 00518C80: __thiscall(frame), RET; tail JMP 00518B90 with ECX = 00E08654.
void front_end_frame_apply_selection_00518c80(FrontEndFrameHost& host);
// 00518B90: ECX = flag frame index, RET; SEH 00C6A998.
void front_end_frame_set_flag_00518b90(std::uint32_t flag, FrontEndFrameHost& host);
// 005189B0: ECX = const string*, RET; SEH 00C6A980.
void front_end_frame_set_title_005189b0(std::string_view text, FrontEndFrameHost& host);

// 00515BC0: __thiscall(screen), RET; SEH 00C6A708. Closes an open view, then leaves the screen.
void tactical_library_leave_00515bc0(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host);
// 00515E00: __thiscall(screen), RET; SEH 00C6A748. Closes an open view, or leaves through 00515BC0.
void tactical_library_back_00515e00(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host);

}  // namespace bsp
