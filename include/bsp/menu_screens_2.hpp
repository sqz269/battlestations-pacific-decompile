#pragma once

// Front-end and in-game screens of partition segments 12, 15 and 19 (packet cc13-menus4).
// Evidence: docs/CC13_MENU_SCREENS_2.md. Every name here is a hypothesis; addresses,
// ABIs and listing ranges are in the comments of src/menu_screens_2.cpp.

#include "bsp/main_menu_command_bar.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
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
inline constexpr float kSessionBrowserFilterRowTop = 0.075F;              // 00CEED68
inline constexpr double kSessionBrowserFilterRowPad = 0.011111111111111112;  // 00CEED60

// The screen's widget pointers that these routines dereference.
enum class SessionBrowserWidget : std::uint16_t {
    FilterPanel2C = 0x2C,  // virtual +38h answers "open"; byte +85h tested
    SessionList30 = 0x30,  // the result listbox; byte +85h tested
    FilterList34 = 0x34,
    Toggle3C = 0x3C,       // virtual +84h(state), paired with +50h
    Toggle40 = 0x40,       // virtual +84h(state), paired with +51h
    EmptyText4C = 0x4C,    // shown when no session is listed
    Viewport44 = 0x44,     // the filter rows' viewport (height read by 00568930)
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
    float field_240{};               // +240h, laid-out filter content height (enter resets it)
    float row_spacing_25c{};         // +25Ch, filter row step
    bool scroller_12c_174{};         // +174h (scroller +12Ch, field +48h)
    bool scroller_188_1d0{};         // +1D0h (scroller +188h, field +48h)
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
    bool dirty_e08701{};             // 00E08701, set when the client's result list changed
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
    std::uint64_t xuid_98{};         // record+98h/+9Ch
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
    // Filter-row layout (00568930): rows are the nine widgets +54h..+74h.
    virtual float widget_f4_x_00aa67f0() = 0;                 // first float of 00AA67F0(widget +F4h)
    virtual void filter_row_set_position_00aa7dc0(std::int32_t row, float x, float y, float z) = 0;
    virtual bool filter_row_visible_38(std::int32_t row) = 0;
    virtual void scroller_attach_content_00683790(std::uint16_t scroller_offset, std::int32_t arg) = 0;
    virtual float widget_height_00aa6740(SessionBrowserWidget widget) = 0;  // returned vector +4
    virtual void scroller_set_scroll_range_006834a0(std::uint16_t scroller_offset, float range) = 0;
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;   // ECX = game+1EF0h
    virtual void filters_tick_0056dd80() = 0;
    virtual void command_bar_tick_0056e3e0() = 0;
    virtual void begin_filter_edit_0056e360() = 0;
    virtual void apply_filter_edit_0057a910() = 0;
    virtual void build_rows_default_00576d20() = 0;
    virtual void build_rows_005789c0(std::int32_t sort_key) = 0;
    virtual void refresh_rows_00577650() = 0;
    virtual void reset_search_0056ac40() = 0;              // ECX = screen+278h
    virtual void scroll_follow_0056b610() = 0;             // (screen+1E4h, list +30h, widget +84h, 0)
    virtual void filter_left_0056ee00() = 0;
    virtual void filter_right_0056efb0() = 0;
    // 0057A9C0(record, arg): the record comes from the screen's vector +2A0h (005663B0) or from the
    // client's +94h result vector (00566370).
    virtual void join_session_0057a9c0(bool from_client_results, std::size_t index, std::int32_t arg) = 0;
    virtual std::size_t client_result_count_94() = 0;     // size of the vector the last +94h call returned
    virtual void clear_search_list_0056a090() = 0;        // list +278h: erase all, relink, +280h = 0
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
// 0057B530: __thiscall(screen), RET; the page-0 poll once the client reports idle.
void session_browser_poll_session_list_0057b530(SessionBrowserState& screen, SessionBrowserHost& host);
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

// 0057AF20: HandleGuiEvent, slot +04h of the widget listener 00CEF100 (ECX = screen+0Ch),
// stack (event widget), RET 4. Screen offsets below are the pointers the event is compared with.
inline constexpr float kSessionBrowserAutoScrollStep = 0.002F;  // 00CE779C
inline constexpr std::int32_t kGuiEventCommandButton = 3;       // event virtual +5Ch
inline constexpr std::string_view kSessionBrowserArrowRight = "scroll_right_Icon";  // 00CEF0D0
inline constexpr std::string_view kSessionBrowserArrowLeft = "scroll_left_Icon";    // 00CEF0BC

struct SessionBrowserEventHost {
    virtual ~SessionBrowserEventHost() = default;
    virtual std::int32_t event_kind_5c() = 0;                 // event virtual +5Ch (read again at the tail)
    virtual std::uint8_t event_command() = 0;                 // *(event+F0h), or *(00E194B0) when null
    virtual bool event_is(std::uint16_t screen_offset) = 0;   // event == *(screen+offset)
    virtual bool event_name_equals(std::string_view name) = 0;  // event virtual +28h, 00425850
    virtual std::int32_t event_row_d8() = 0;                  // event+D8h
    virtual void event_virtual_88(std::int32_t a, std::int32_t b, float value) = 0;
    virtual void event_virtual_84(std::int32_t state) = 0;
    virtual void filter_list_select_00a9c8f0(std::int32_t row) = 0;  // ECX = list +34h
    virtual void scroller_begin_thumb_drag_00683a40(std::uint16_t scroller_offset) = 0;  // screen+offset
    virtual void scroller_set_auto_scroll_00683300(std::uint16_t scroller_offset, std::int32_t direction,
        float step, std::int32_t arg) = 0;
    virtual bool session_signed_in_198() = 0;
    virtual bool event_byte_d4() = 0;                         // event+D4h
    virtual void event_virtual_4c(float value) = 0;
    virtual void scroller_end_thumb_drag_00683320(std::uint16_t scroller_offset) = 0;
};

// Segment-18 browser helpers (ECX = screen unless noted).
// 00568930: __thiscall(screen), RET. Lays out the nine filter rows and the +12Ch scroll range.
void session_browser_layout_filter_rows_00568930(SessionBrowserState& screen, SessionBrowserHost& host);
// 00568A30: widget-listener slot +0Ch (ECX = screen+0Ch), stack (event), RET 4: release.
void session_browser_on_widget_released_00568a30(SessionBrowserState& screen, SessionBrowserEventHost& event);
// 00568B60: widget-listener slot +18h (ECX = screen+0Ch), stack (event, bool hovered), RET 8.
void session_browser_on_widget_hover_00568b60(SessionBrowserState& screen, bool hovered, SessionBrowserHost& host,
    SessionBrowserEventHost& event);
// 00568CB0: __thiscall(screen), RET (tail JMP 006881F0).
void session_browser_back_from_results_00568cb0(SessionBrowserHost& host);
// 00568D40: __thiscall(screen), RET, 64-bit result in EDX:EAX (0 when nothing valid is selected).
std::uint64_t session_browser_selected_xuid_00568d40(SessionBrowserHost& host);
// 00568DF0: __thiscall(screen, int which), RET 4: 0 selects toggle +40h/+51h, 1 toggle +3Ch/+50h.
void session_browser_select_toggle_00568df0(SessionBrowserState& screen, std::int32_t which, SessionBrowserHost& host);
// 0056AA70: __thiscall(screen, list), RET 4; every caller passes +30h.
void session_browser_focus_list_0056aa70(SessionBrowserHost& host);
// 0056AAD0: __thiscall(screen), RET: leave a toggle and return to a non-empty result list.
void session_browser_return_to_list_0056aad0(SessionBrowserState& screen, SessionBrowserHost& host);

void session_browser_handle_gui_event_0057af20(SessionBrowserState& screen, SessionBrowserHost& host,
    SessionBrowserEventHost& event);

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
// Session browser join path (packet cc13-menus5): JoinSession 0057A9C0, FinishJoin 0057AC10,
// the joinable test 00574100 and the content-mask test 00568490. Reconstructed only; the
// multiplayer session layer is not wired.
// ---------------------------------------------------------------------------

inline constexpr float kSessionJoinRetryTimerReset = -1.0F;  // 00D7A260 -> 00E08760
inline constexpr std::int8_t kSessionRecordKindOnline = -1;  // record+04h == 0FFh
inline constexpr std::int32_t kSessionJoinNetworkMode = 2;   // game+1FE4h and 0076FAD0(2)
inline constexpr std::int32_t kSessionClientModeJoining = 6; // *(game+207Ch)+9Ch
inline constexpr std::int32_t kSessionClientModeJoined = 7;
inline constexpr std::int32_t kRequestNoSessionFound = 0x10;  // 00688AD0(10h) from FinishJoin
inline constexpr std::int32_t kRequestJoinFailed = 0x1B;      // 00688AD0(1Bh) from JoinSession
inline constexpr std::int32_t kErrorScreenJoinFailed = 4;     // 005D2E70(4)

// The 3Ch-byte block at *(game+207Ch)+96Ch (cNetworkClient = +C8h, so client +8A4h, the
// pSessionInfo argument 00A39B80 passes to XSessionCreate). The layout matches the Xbox 360
// XSESSION_INFO: sessionID +0h, hostAddress (XNADDR, 24h bytes) +8h, keyExchangeKey +2Ch.
struct SessionInfoBlock {
    std::array<std::uint32_t, 2> session_id{};       // +96Ch, +970h
    std::array<std::uint32_t, 9> host_address{};     // +974h..+997h
    std::array<std::uint32_t, 4> key_exchange_key{}; // +998h..+9A7h
};

// The 88h-byte object at *(record+00h), as copied by 00568CE0 (vtable 00CEED04): +04h the
// session id, +0Ch the key, +1Ch the host address; +40h (10 dwords) and +68h (8 dwords) are
// copied but not read by the join.
struct SessionSearchResultView {
    std::array<std::uint32_t, 2> session_id_04{};
    std::array<std::uint32_t, 4> key_0c{};
    std::array<std::uint32_t, 9> host_address_1c{};
};

// One 0A0h-byte session record, the fields the join path reads.
struct SessionJoinRecord {
    std::uint32_t key_00{};          // record+00h, the search-result pointer; the tried-set key
    SessionSearchResultView result;  // *(record+00h), read only when kind_04 == -1
    std::int8_t kind_04{};           // record+04h: -1 online result, else a listed-server index
    std::wstring name_08;            // record+08h (buffer +0Ch, capacity +20h, inline below 8)
    std::int32_t open_slots_24{};    // record+24h, must be > 0
    std::uint32_t players_28{};      // record+28h, must be below +2Ch (unsigned)
    std::uint32_t max_players_2c{};  // record+2Ch
    std::uint32_t field_30{};        // record+30h, rejected by filter byte +17h above screen+24Ch
    std::uint32_t field_34{};        // record+34h, sort key 4 (descending)
    std::int32_t connection_38{};    // record+38h, compared with the five ping thresholds
    std::uint8_t rank_min_48{};      // record+48h
    std::uint8_t rank_max_49{};      // record+49h
    std::array<std::uint32_t, 14> lobby_options_4c{};  // record+4Ch, handed to 005D49E0
    std::uint32_t required_mask_88{};  // record+88h, tested against game+728h
    std::uint32_t required_mask_8c{};  // record+8Ch, tested against game+72Ch
    std::uint32_t ratio_den_90{};    // record+90h, filter byte +18h: +94h / +90h
    std::uint32_t ratio_num_94{};    // record+94h
};

struct SessionJoinHost {
    virtual ~SessionJoinHost() = default;
    // Session client *(00F8A2FC).
    virtual void session_cancel_8c() = 0;                          // vtable +8Ch
    virtual const std::vector<SessionJoinRecord>& client_results_94() = 0;  // vtable +94h, vector +4h/+8h
    virtual bool client_byte_4d() = 0;                             // client+4Dh
    // Game *(00E188A8).
    virtual std::int32_t network_mode_1fe4() = 0;                  // game+1FE4h
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0; // ECX = game+1EF0h
    virtual void select_listed_server_007701f0(std::int32_t index) = 0;  // ECX = game+1EF0h
    virtual std::int32_t client_mode_9c() = 0;                     // *(game+207Ch)+9Ch
    virtual void set_client_mode_9c(std::int32_t mode) = 0;
    virtual SessionInfoBlock& client_session_info_96c() = 0;       // *(game+207Ch)+96Ch
    virtual void network_client_connect_00a39b80() = 0;            // ECX = *(game+207Ch)+C8h
    virtual void set_game_byte_2059(bool value) = 0;
    virtual std::uint32_t content_mask_728() = 0;                  // game+728h
    virtual std::uint32_t content_mask_72c() = 0;                  // game+72Ch
    virtual std::uint8_t profile_rank_00574050() = 0;              // "RANK" on game+650h
    // Xenon system manager.
    virtual bool has_selected_user() = 0;                          // BSP_XenonSystemManager_HasSelectedUser
    virtual std::string selected_username() = 0;                   // BSP_XenonSystemManager_GetSelectedUsername
    // Globals and front-end managers.
    virtual void copy_stored_lobby_options_005d49e0(const std::array<std::uint32_t, 14>& options) = 0;
    virtual void lua_set_global_string(std::string_view name, const std::string& value) = 0;  // 00B67630
    virtual bool search_flag_e08700() = 0;
    virtual void set_join_retry_timer_e08760(float value) = 0;
    virtual void set_global_e19610(std::int32_t value) = 0;
    virtual std::int32_t request_count_006878d0() = 0;             // ECX = *(00E198B4)
    virtual void pop_request_interface_006881f0() = 0;
    virtual void push_request_interface_logged_00688ad0(std::int32_t id) = 0;
    virtual void error_screen_show_005d2e70(std::int32_t kind) = 0;  // ECX = *(*(00E198B4)+50h)
    virtual void set_page_0057a0f0(std::int32_t page) = 0;         // session_browser_set_page_0057a0f0
};

// The screen fields the join path owns besides SessionBrowserState: the tried-session set
// (std::set<record key>, header +27Ch, size +280h). JoinSession stores the joined key in +24h
// (SessionBrowserState::field_24).
struct SessionJoinState {
    std::vector<std::uint32_t> tried_keys_278;  // kept sorted, unique (the original is a red-black tree)
};

// 00568490: __thiscall(record), RET.
bool session_record_content_allowed_00568490(const SessionJoinRecord& record, SessionJoinHost& host);
// 00574100: __thiscall(record), RET; SEH 00C6F1B9.
bool session_record_joinable_00574100(const SessionJoinRecord& record, SessionJoinHost& host);
// 00568CE0 subset: the XSESSION_INFO fields JoinSession copies out of the 00568CE0 object.
SessionInfoBlock session_info_from_search_result(const SessionSearchResultView& result);
// 0057A9C0: __thiscall(screen, record, arg), RET 8; the second argument is never read.
void session_browser_join_session_0057a9c0(SessionBrowserState& screen, const SessionJoinRecord& record,
    SessionJoinHost& host);
// 0057AC10: __thiscall(screen), RET; no SEH frame.
void session_browser_finish_join_0057ac10(SessionBrowserState& screen, SessionJoinState& join,
    SessionJoinHost& host);

// ---------------------------------------------------------------------------
// Session browser rows (packet cc13-menus5)
// ---------------------------------------------------------------------------

using GuiColour = std::array<float, 4>;
inline constexpr GuiColour kSessionRowJoinable{0.976471F, 0.858824F, 0.6F, 1.0F};  // 00CEF16C, 00CEF168, 00CE3D30, 00D7A24C
inline constexpr GuiColour kSessionRowLocked{1.0F, 0.0F, 0.0F, 1.0F};   // static 00E19498 (guard 00E194A8 bit 0)
inline constexpr GuiColour kSessionRowIdle{0.5F, 0.5F, 0.5F, 1.0F};     // static 00E19484 (guard 00E19494 bit 0)
inline constexpr std::int32_t kLobbyOptionGameMode = 2;  // registry key handed to 00575E80 with record+54h

// Widget handles are opaque pointer values from 00AA7E00 (find child by name, recursive flag 1).
struct SessionRowHost {
    virtual ~SessionRowHost() = default;
    virtual std::int32_t network_mode_1fe4() = 0;                   // *(00E188A8)+1FE4h
    virtual std::uint32_t content_mask_728() = 0;
    virtual std::uint32_t content_mask_72c() = 0;
    virtual const std::vector<SessionJoinRecord>& client_results_94() = 0;  // session client vtable +94h
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;
    virtual void text_set_00abaed0(std::uintptr_t widget, const std::string& text) = 0;           // arg 0
    virtual void text_set_ellipsized_00abb000(std::uintptr_t widget, const std::string& text, float width) = 0;  // arg 1
    virtual void widget_set_colour_50(std::uintptr_t widget, const GuiColour& colour) = 0;       // vtable +50h
    // 00576B10 registry, +10h map, 00575E80 find(kind, value), 005732C0 the entry's text.
    virtual std::string lobby_option_text(std::int32_t kind, std::uint32_t value) = 0;
    // Lobby option registry 00576B10 fields.
    virtual std::int32_t ping_threshold_90(std::size_t index) = 0;  // registry+90h + 4*index, 0..4
    virtual std::uint32_t ratio_minimum_b8() = 0;                   // registry+B8h
    virtual float ratio_maximum_bc() = 0;                           // registry+BCh
    virtual std::int32_t game_type_filter_e086c0() = 0;             // -1 = any
    // Widgets (vtable slots of the GUI widget class).
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual void widget_set_state_88(std::uintptr_t widget, std::int32_t state, std::int32_t arg, float weight) = 0;
    virtual bool widget_byte_85(std::uintptr_t widget) = 0;
    virtual std::string widget_text_f4(std::uintptr_t widget) = 0;  // NativeString at widget+F4h
    virtual std::uintptr_t clone_subtree_00aab4c0(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual void widget_set_floats_30(std::uintptr_t widget, float f30, float f34) = 0;  // plain stores
    virtual void widget_set_byte_77(std::uintptr_t widget, bool value) = 0;
    virtual void widget_virtual_80(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual void widget_focus_60(std::uintptr_t widget, bool value) = 0;
    // Listbox 00A9xxxx runtime.
    virtual std::uintptr_t list_selected_row_00425e50(std::uintptr_t list) = 0;
    virtual void list_set_listener_00a9ac40(std::uintptr_t list, bool screen_plus_8) = 0;
    virtual void list_clear_rows_00a9bec0(std::uintptr_t list) = 0;
    virtual void list_append_row_00a9d750(std::uintptr_t list, std::uintptr_t row) = 0;  // (row, 0, 0)
    virtual std::int32_t list_row_count_104(std::uintptr_t list) = 0;
    virtual std::uintptr_t list_row_at_00a9be00(std::uintptr_t list, std::int32_t index) = 0;
    virtual float group_height_00aa6740(std::uintptr_t widget) = 0;  // returned vector +4
    virtual void scroller_attach_content_00683790(std::int32_t arg) = 0;  // ECX = screen+1E4h
    virtual void scroller_set_scroll_range_006834a0(float range) = 0;     // ECX = screen+1E4h
    virtual void refresh_row_details_00577650() = 0;
};

// Screen fields the row routines read or write (beyond SessionBrowserState).
struct SessionRowState {
    std::uintptr_t filter_panel_2c{};               // +2Ch (its byte +85h)
    std::uintptr_t session_list_30{};               // +30h
    float viewport_height_8c{};                      // +8Ch, subtracted from the content height
    std::uintptr_t row_template_e8{};               // +E8h
    std::array<std::uint32_t, 3> applied_counts_244{};  // +244h, +248h, +24Ch
    std::uintptr_t selected_row_26c{};               // +26Ch
    std::vector<SessionJoinRecord> results_290;      // +290h, the last client snapshot
    std::vector<SessionJoinRecord> records_2a0;      // +2A0h, the filtered, sorted list
    std::int32_t sort_key_2b0{};                     // +2B0h
    std::array<std::uintptr_t, 4> sort_icons_2b4{};  // +2B4h keys 0, 1, 5, 4 (in that order)
};

// The six sort keys of 0056A870 (the record comparator; descending ones compare b < a).
enum class SessionSortKey : std::int32_t {
    Name = 0,           // _wcsicmp(a.name, b.name) < 0
    GameMode = 1,       // a+54h < b+54h
    MaxPlayers = 2,     // descending +2Ch
    FreeSlots = 3,      // descending +2Ch - +28h
    Field34 = 4,        // descending +34h
    Fullness = 5,       // descending +2Ch * 16 + +28h
    KeepCurrent = 6,    // 005789C0 only: keep +2B0h and refresh +290h from the client
};
// 00E1947C[key]: one reverse flag per sort key (bytes, toggled when the same key is chosen again).
using SessionSortReverseFlags = std::array<bool, 6>;

// 0056A870: __thiscall(key*, a, b), RET 8.
bool session_record_less_0056a870(SessionSortKey key, const SessionJoinRecord& a, const SessionJoinRecord& b);
// 00576BE0: __thiscall(screen, record), RET 4; the filter test 005789C0 applies to +290h.
bool session_record_passes_filters_00576be0(const SessionBrowserState& filters, const SessionRowState& screen,
    const SessionJoinRecord& record, SessionRowHost& host);
// 00578320: __fastcall(ECX = Connection_Group widget, EDX = record+38h); SEH 00C6F4A8.
void session_row_set_connection_icons_00578320(std::uintptr_t group, std::int32_t connection,
    SessionRowHost& host);
// 0056BDA0 BindLayout: loads the two browser pages and binds the widget fields of the 2D0h-byte
// screen. Widgets are keyed by their screen offset; kLocal* keys are temporaries the original
// keeps in registers.
inline constexpr std::uint16_t kBindLocalCucc = 0xFFFF;         // EDI, cucc_Group
inline constexpr std::uint16_t kBindLocalSliderPos = 0xFFFE;    // the twice-searched slider_pos_Group
inline constexpr float kSessionListRowSpan = 0.47395834F;       // 00CEEED0 -> 00A9CEB0(value, 0)
inline constexpr float kSessionListPad128 = 0.0069444445F;      // 00CEEECC -> list+128h
inline constexpr float kSessionListPad130 = 0.0027777778F;      // 00CEE600 -> list+130h

struct SessionBrowserLayoutHost {
    virtual ~SessionBrowserLayoutHost() = default;
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;   // GUI manager 004C12B0, (name, 1, 0)
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual void set_listener_00aa6bc0(std::uintptr_t widget) = 0;           // (screen+0Ch, 0): +DCh, +79h
    virtual float group_height_00aa6740(std::uintptr_t widget) = 0;          // returned vector +4
    virtual void list_configure_00a9ceb0(std::uintptr_t list, float value, std::int32_t arg) = 0;
    virtual void list_set_pads_128(std::uintptr_t list, float f128, float f12c, float f130, float f134) = 0;
    virtual void list_set_byte_149(std::uintptr_t list, bool value) = 0;
    virtual void scroller_attach_content_00683790(std::uint16_t scroller, std::uintptr_t content) = 0;
    virtual void scroller_attach_scroll_bar_00683380(std::uint16_t scroller, std::uintptr_t frame, float extent,
        std::uintptr_t up, std::uintptr_t down) = 0;
};

struct SessionBrowserLayout {
    std::vector<std::pair<std::uint16_t, std::uintptr_t>> widgets;  // (screen offset, widget), in bind order
    float slider_back_height_90{};  // +90h
    float clip_height_8c{};         // +8Ch
    std::uintptr_t at(std::uint16_t offset) const noexcept {
        for (const auto& [key, widget] : widgets) if (key == offset) return widget;
        return 0;
    }
};

// 0056BDA0: __thiscall(screen), RET; SEH 00C6E9E0. Slot +14h of vtable 00CEF134 (00CEF148).
void session_browser_bind_layout_0056bda0(SessionBrowserLayout& layout, SessionBrowserLayoutHost& host);

// 005789C0: __thiscall(screen, sort key), RET 4; SEH 00C6F5C0.
void session_browser_build_rows_005789c0(const SessionBrowserState& filters, SessionRowState& screen,
    SessionSortReverseFlags& reverse, std::int32_t sort_key, SessionRowHost& host);

// 00579980: __thiscall(screen), RET; SEH 00C6F62F. No caller or data reference in the image: appends
// one row per +2A0h record to the list +30h, as BuildRows' row body does, without clearing,
// filtering, sorting or the selected-row match.
void session_browser_append_record_rows_00579980(const SessionRowState& screen, SessionRowHost& host);

// 00576D20: __thiscall(screen), returns int (1 on one path, otherwise the colour virtual's
// result; SetPage ignores it); SEH 00C6F43B.
void session_browser_update_selected_row_00576d20(SessionRowState& screen, SessionRowHost& host);

// RefreshRowDetails 00577650 (packet cc13-menus6): the settings panel for the selected record.
inline constexpr std::string_view kSessionDetailsBlank = "-";                         // 00CEED40
inline constexpr std::string_view kSessionUnknownMapName = "FE.unknown_mapname";      // 00CEF1D8
inline constexpr std::string_view kSessionRatioSuffix = " %";                        // 00CEF1AC
inline constexpr std::int32_t kLobbyOptionMap = 1;     // the key 00575E80/00575FD0 take with record+24h
inline constexpr double kSessionMapWorldOrigin = -15000.0;   // 00CEF1D0
inline constexpr double kSessionMapWorldSize = 30000.0;      // 00CEF1C8
inline constexpr double kSessionMapIconPixels = 150.0;       // 00CE3DD8
inline constexpr double kSessionScreenWidth = 960.0;         // 00CEC380
inline constexpr double kSessionScreenHeight = 720.0;        // 00CEF1B8
inline constexpr float kSessionMapIconWidth = 0.15625F;      // 00CEF1C0 (150 / 960)
inline constexpr float kSessionMapIconHeight = 0.20833333F;  // 00CEF1B0 (150 / 720)
inline constexpr std::size_t kSessionMapDefinitionSize = 0x434;  // *(00E198AC)+5Ch, vector +24h

// Widgets of the details panel (BindLayout names).
struct SessionDetailsWidgets {
    std::uintptr_t tilt_icon_9c{};        // +9Ch
    std::uintptr_t map_icon_a0{};         // +A0h
    std::uintptr_t rank_min_text_c4{};    // +C4h
    std::uintptr_t rank_max_text_cc{};    // +CCh
    std::uintptr_t map_name_text_d0{};    // +D0h (game_mode_var_Text in BindLayout's map)
    std::uintptr_t ratio_text_d8{};       // +D8h
    std::uintptr_t rank_min_icon_dc{};    // +DCh
    std::uintptr_t rank_max_icon_e0{};    // +E0h
    std::uintptr_t ratio_icon_e4{};       // +E4h
};

struct SessionDetailsHost {
    virtual ~SessionDetailsHost() = default;
    virtual std::int32_t list_selected_index_00a9c920(std::uintptr_t list) = 0;
    virtual std::int32_t list_scroll_offset_164(std::uintptr_t list) = 0;  // list+148h ? list+164h : 0
    // Lobby option registry 00576B10: +34h option -> (value -> localisation key), +1Ch option ->
    // (value -> map icon index), +B8h/+BCh the ratio bounds.
    virtual bool lobby_option_text_known_34(std::int32_t option, std::int32_t value) = 0;  // on a copy (00573340)
    virtual std::string lobby_option_text_34(std::int32_t option, std::int32_t value) = 0;  // 005732C0
    virtual bool map_icon_known_1c(std::int32_t option, std::int32_t value) = 0;            // 0056B930
    virtual std::int32_t map_icon_index_1c(std::int32_t option, std::int32_t value) = 0;    // 00573410
    virtual std::uint32_t ratio_minimum_b8() = 0;
    virtual float ratio_maximum_bc() = 0;
    // Map definitions *(00E198AC)+5Ch, vector +24h of 434h-byte records (name string at +00h).
    virtual std::size_t map_definition_count() = 0;
    virtual std::string map_definition_name(std::size_t index) = 0;   // null buffer = "" (00E194AC)
    // {x, z} at +offset and +offset+0Ch: the two corners of one world extent.
    virtual std::array<float, 4> map_definition_extent(std::size_t index, std::uint16_t offset) = 0;
    virtual bool content_allowed_00568490(const SessionJoinRecord& record) = 0;
    virtual std::int32_t session_client_virtual_198() = 0;            // *(00F8A2FC) vtable +198h
    virtual void invalid_parameter_00bf6713() = 0;
    // Widgets.
    virtual void text_submit_00abaed0(std::uintptr_t text, const std::string& source, std::int32_t arg) = 0;
    virtual void text_set_localised_00abbe50(std::uintptr_t text, std::string_view key, std::int32_t arg) = 0;
    virtual void widget_show_34(std::uintptr_t widget, std::int32_t shown) = 0;
    virtual void widget_set_state_88(std::uintptr_t widget, std::int32_t state, std::int32_t arg, float weight) = 0;
    virtual void widget_set_size_58(std::uintptr_t widget, const std::array<float, 2>& size) = 0;
    virtual void widget_set_colour_50(std::uintptr_t widget, const GuiColour& colour) = 0;
    // 00568290: stores the four floats at widget+118h..+124h and calls virtual +8Ch.
    virtual void icon_set_uv_rect_00568290(std::uintptr_t icon, const std::array<float, 4>& uv) = 0;
};

// 00577650: __thiscall(screen), RET; SEH 00C6F480; 8-aligned frame. Writes screen+264h.
void session_browser_refresh_row_details_00577650(const SessionRowState& screen,
    const SessionDetailsWidgets& widgets, bool& content_allowed_264, SessionDetailsHost& host);

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

struct CreditsExitHost {
    virtual ~CreditsExitHost() = default;
    // Front-end manager *(00E198AC): +54h the credits stream, +50h the menu stream.
    virtual void stream54_request_stop_00a86f40() = 0;
    virtual void stream50_start_00a867b0(std::string_view name) = 0;
    virtual void stream50_request_fade_in_00a85c20() = 0;
    virtual std::string menu_music_name_e18e64() = 0;    // .data string 00E18E64
    virtual void gui_pause_movie_pages_00aa33a0(std::int32_t arg) = 0;  // ECX = 004C12B0()
    virtual void item_scalar_delete(std::uintptr_t item) = 0;           // item virtual +4(1)
};

// Credits Enter 0052EE30 (packet cc13-menus5). One 24h-byte item per Credits_Items row, read by
// 0052D9C0 (Type, Text, Offset, Color, Align) into the vector at screen+4Ch.
enum class CreditsItemType : std::int32_t {
    Category = 0,     // clone of the category template +10h, narrow text
    Person = 1,       // clone of the person template +14h; a "%" text is swprintf'd with U+2122
    Image = 2,        // clone of the "<Text>_Icon" child of the moving group +1Ch
    BlankLine = 3,    // height 00CED1B8 (1/36)
    VerticalGap = 4,  // y += Offset (height -0.0 - RowSpace cancels the row step)
    SetIndent = 5,    // +48h = Offset
    SameLine = 6,     // next item continues the previous row: +48h = width + indent, y = row y
};
enum class CreditsAlign : std::int32_t { Left = 0, Right = 1, Centre = 2 };

struct CreditsItem {
    CreditsItemType type{CreditsItemType::Person};  // +00h, default 1
    std::string text;                                // +04h
    GuiColour colour{1.0F, 1.0F, 1.0F, 1.0F};        // +0Ch, static 00E18E50 (guard 00E18E60)
    float offset{};                                  // +1Ch
    std::int32_t align{2};                           // +20h; values above 2 keep x = 00CE7638
};

inline constexpr float kCreditsBlankLineHeight = 0.027777778F;  // 00CED1B8
inline constexpr float kCreditsStaleX = 0.05F;                  // 00CE7638, x when Align > 2
inline constexpr double kCreditsTextPixels = 960.0;             // 00CEC380, widget+114h / 960
inline constexpr float kCreditsItemDepth = -100.0F;             // 00CE65D8, position z
inline constexpr wchar_t kCreditsPercentArgument = 0x2122;      // TRADE MARK SIGN passed to vswprintf

struct CreditsLayoutState {
    std::uintptr_t category_template_10{};  // +10h
    std::uintptr_t person_template_14{};    // +14h
    std::uintptr_t frame_18{};              // +18h clipbox_Clipbox (Register 0052FFC0); origin/size reference, vtable +70h after layout
    std::uintptr_t moving_group_1c{};       // +1Ch moving_Group (child of +18h), parent of every clone, scrolled by the update
    float speed_offset_20{};                // +20h
    std::vector<std::uintptr_t> widgets_24; // +24h (begin +28h), deleted by the exit 0052E150
    float scroll_y_34{};                    // +34h
    float content_height_38{};              // +38h
    float start_y_40{kCreditsField40};      // +40h ("StartY")
    float row_space_44{kCreditsField44};    // +44h ("RowSpace")
    float indent_48{};                      // +48h
    std::vector<CreditsItem> items_4c;      // +4Ch
};

struct CreditsEnterHost {
    virtual ~CreditsEnterHost() = default;
    // Front-end manager *(00E198AC).
    virtual bool stream54_exists() = 0;
    virtual void stream54_construct_00a877d0() = 0;   // operator new(54h), name = copy of manager+48h
    virtual void stream50_request_stop_00a86f40() = 0;
    virtual void stream54_start_00a867b0(std::string_view name) = 0;
    virtual std::string stream_name_e18e64() = 0;     // .data string 00E18E64
    virtual void stream54_request_fade_in_00a85c20() = 0;
    virtual void gui_pause_movie_pages_00aa33a0(std::int32_t arg) = 0;  // ECX = 004C12B0()
    virtual void set_frame_title_005189b0(std::string_view title) = 0;
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    // Widgets.
    virtual void widget_set_local_y_00aa78f0(std::uintptr_t widget, float y) = 0;
    virtual std::uintptr_t clone_subtree_00aab4c0(std::uintptr_t widget, std::uintptr_t parent) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;
    virtual void text_set_00abaed0(std::uintptr_t widget, const std::string& text) = 0;
    virtual void text_set_utf16_00ab6ab0(std::uintptr_t widget, const std::wstring& text) = 0;
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual void widget_set_colour_50(std::uintptr_t widget, const GuiColour& colour) = 0;
    virtual float text_pixel_width_114(std::uintptr_t widget) = 0;              // float at widget+114h
    virtual float text_normalized_height_00ab6bd0(std::uintptr_t widget) = 0;
    virtual std::array<float, 2> group_size_00aa6740(std::uintptr_t widget) = 0;
    virtual float group_origin_x_00aa6750(std::uintptr_t widget) = 0;          // first float of the result
    virtual void set_resolved_position_00aa8240(std::uintptr_t widget, const std::array<float, 3>& position) = 0;
    virtual void widget_virtual_70(std::uintptr_t widget) = 0;
};

// 0052EE30: slot +18h of vtable 00CED200, __thiscall(screen), RET; SEH 00C6BD92; frame 89Ch.
void credits_enter_0052ee30(CreditsLayoutState& screen, CreditsEnterHost& host);

// Credits Register 0052FFC0 and ReadItemStyle 0052D9C0 (packet cc13-menus6). The Lua reader keys
// are (kind, value) pairs: kind 0 a field name, kind 1 an array index. Read types follow
// GuiLuaFieldType (0 string, 1 int, 2 float, 8 vec4).
struct CreditsLuaReader {
    virtual ~CreditsLuaReader() = default;
    virtual bool has_key_00bd5eb0(std::string_view key) = 0;           // vtable +14h
    virtual bool has_index_00bd5eb0(std::int32_t index) = 0;
    virtual void read_int_00bd6830(std::string_view key, std::int32_t& out) = 0;   // vtable +10h, type 1
    virtual void read_string_00bd6830(std::string_view key, std::string& out) = 0; // type 0
    virtual void read_float_00bd6830(std::string_view key, float& out) = 0;        // type 2
    virtual void read_vec4_00bd6830(std::string_view key, GuiColour& out) = 0;     // type 8
    virtual void enter_key_00bd8e20(std::string_view key) = 0;
    virtual void enter_index_4(std::int32_t index) = 0;                // vtable +4h, (1, index)
    virtual void leave_8() = 0;                                        // vtable +8h
    virtual void leave_key_00bd7a20() = 0;
};

// 0052D9C0: __thiscall(item, reader*), RET 4. Every default is written before any read; Align is
// read through a pointer to item+20h spilled at [esp+10h] (Ghidra lost the spill and shows 0).
void credits_read_item_style_0052d9c0(CreditsItem& item, CreditsLuaReader& reader);

inline constexpr std::string_view kCreditsPage = "FE_credits";                         // 00CED2C4
inline constexpr std::string_view kCreditsScript = "Scripts/datatables/Credits.lua";   // 00CED254

struct CreditsRegisterHost {
    virtual ~CreditsRegisterHost() = default;
    virtual void register_front_end_screen_004f71d0() = 0;             // ECX = screen
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;  // ECX = 004C12B0(), (name, 1, 0)
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;  // (name, 1)
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    // 00B66BD0 construct, 00B6A020 open, 00B69D40 run (path, 0), 00B67980 globals, 00B67800
    // "Credits", then a reader (004425C0) over that table; closed (00B669A0) on return.
    virtual CreditsLuaReader& open_credits_table(std::string_view script, std::string_view table) = 0;
    virtual void close_credits_table() = 0;
};

// 0052FFC0: slot +10h of vtable 00CED200, __thiscall(screen), RET; SEH 00C6BEB5; frame 534h.
void credits_register_0052ffc0(CreditsLayoutState& screen, std::uintptr_t& page_0c, float& base_speed_3c,
    CreditsRegisterHost& host);

// 0052E150: slot +1Ch of vtable 00CED200, __thiscall(screen), RET; SEH 00C6BC48.
// `items` is the pointer vector +28h..+2Ch; entries are deleted and nulled in place.
void credits_exit_0052e150(std::vector<std::uintptr_t>& items, CreditsExitHost& host);

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
    std::int32_t current_index{};    // screen+434h (listener+3D0h): index word of the +434h selector, FILD
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
    std::int32_t view_90{};          // +90h, 0 = 2D panel, 1 = 3D model view (00511B40 toggles)
    std::int32_t entry_kind_94{};    // +94h, 2/5/6 tested
    std::int32_t field_9c{};         // +9Ch, picks 0Bh or 0Ch for 00E08874
};

struct TacticalLibraryNavHost {
    virtual ~TacticalLibraryNavHost() = default;
    virtual bool online_1fe4() = 0;                         // *(00E188A8)+1FE4h != 0
    virtual bool front_end_manager_present_e198ac() = 0;    // *(00E198AC) != 0
    virtual bool interface_manager_present_e198b4() = 0;    // *(00E198B4) != 0
    virtual bool interface_query_00687860(std::int32_t id, std::int32_t arg) = 0;  // ECX = *(00E198B4)
    virtual void toggle_view_00511b40() = 0;                // ECX = screen
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

// 00514C20 view toggle (2D panel <-> 3D model) and its mode switch 00511B40.
struct TacticalLibraryViewHost : TacticalLibraryNavHost {
    virtual std::uint32_t selector_434_field_48_004fa890() = 0;   // *(&records[+434h index].+48h)
    virtual void set_field_4b0(std::uint32_t value) = 0;           // screen+4B0h
    virtual void load_vehicle_model_0050aa10() = 0;                // ECX = screen
    virtual void panel_widget_virtual_34(std::uint16_t screen_offset, std::int32_t shown) = 0;  // +508h..+518h
    virtual void refresh_entry_00510c20() = 0;                     // ECX = screen, tail JMP
};

// 00511B40: __thiscall(screen), RET. 0 -> 1 hides +508h/+50Ch/+510h/+518h; 1 -> 0 shows them and
// tail-jumps 00510C20.
void tactical_library_toggle_view_00511b40(TacticalLibraryNavState& screen, TacticalLibraryViewHost& host);
// 00514C20: __thiscall(screen), RET; SEH 00C6A670. Toggles, then refreshes the page of the new mode.
void tactical_library_switch_view_00514c20(TacticalLibraryNavState& screen, TacticalLibraryViewHost& host);

// ---------------------------------------------------------------------------
// HUD free camera (screen 2Bh, INTF_FREECAMERA, vtable 00CEDCA8): Enter 0053DD90, Update 00540390
// (packet cc13-menus5). Reconstructed only; the camera mover is a host object.
// ---------------------------------------------------------------------------

inline constexpr double kFreeCameraForwardScale = 200.0;           // 00CE4D70
inline constexpr double kFreeCameraStrafeScale = 100.0;            // 00D7A220 (strafe and vertical)
inline constexpr double kFreeCameraTurnScale = 1.7453292608261108; // 00CEDCD8 (float 100 deg in rad)
inline constexpr double kFreeCameraRollScale = 0.7853981852531433; // 00CEDCD0 (float pi/4)
inline constexpr double kFreeCameraResetRate = 1.5707963705062866; // 00CE3830 (float pi/2)
inline constexpr float kFreeCameraPitchLimit = 1.5533431F;          // 00CEC400 / 00CEC574 = +-89 deg
inline constexpr float kFreeCameraNoUnitsMessageA = 750.0F;         // 00CEDCE0
inline constexpr float kFreeCameraNoUnitsMessageB = 10.0F;          // 00CE38B8
inline constexpr std::string_view kFreeCameraNoUnitsText = "NO MORE PLAYER UNITS";  // 00CEDCE4

// The input owner (004BEC00()+4) analog slots the update reads, 30h apart.
enum class FreeCameraAxis : std::uint16_t {
    Forward = 0x1164, Strafe = 0x1194, Vertical = 0x11C4, Yaw = 0x11F4, Pitch = 0x1224, Roll = 0x1344,
};
inline constexpr std::uint16_t kFreeCameraResetPitchControl = 0x1260;  // byte +28h active, float +24h value
inline constexpr std::uint16_t kFreeCameraResetAllControl = 0x1290;
inline constexpr std::int32_t kFreeCameraMarkAction = 0x61;           // 004C43C0(61h)

using FreeCameraMatrix = std::array<float, 16>;

struct FreeCameraState {
    bool active_04{};                // +04h
    std::uint32_t field_20{};        // +20h, copied to +64h each update
    std::uintptr_t mover_24{};       // +24h, 900-byte camera mover (00432750), 0 until the first Enter
    float acceleration_34{};         // +34h, multiplies every rate step
    float speed_forward_38{};        // +38h
    float speed_strafe_3c{};         // +3Ch
    float speed_vertical_40{};       // +40h
    float rate_yaw_44{};             // +44h (applied to angle Y)
    float rate_pitch_48{};           // +48h (applied to angle X, clamped +-89 deg)
    std::array<float, 3> angles_54{};  // +54h, +58h, +5Ch from 0042D2E0 at Enter
    std::uint32_t burst_64{};        // +64h, first dword of the 0053F2D0 container
};

struct FreeCameraHost {
    virtual ~FreeCameraHost() = default;
    // Enter.
    virtual std::uintptr_t create_mover_00432750() = 0;            // operator new(900), memset 0, construct
    virtual void mover_place_00923870(std::uintptr_t mover, std::uintptr_t parent, const FreeCameraMatrix& world) = 0;  // (0, game+19CCh, &m)
    virtual void observer_register_pair_00694a60(std::uintptr_t mover) = 0;  // EDX = screen+8
    virtual bool take_saved_pose_e192a9() = 0;                      // byte 00E192A9 set: clears it, true
    virtual FreeCameraMatrix saved_pose_e192b0() = 0;
    virtual FreeCameraMatrix player_camera_world_19fc() = 0;       // *(game+19FCh)+F0h after 00B6DB70 unless +5Ch bit 1
    virtual void mover_set_matrix_74_004134f0(std::uintptr_t mover, const FreeCameraMatrix& matrix) = 0;
    virtual std::array<float, 3> mover_angles_0042d2e0(std::uintptr_t mover) = 0;  // ECX = mover+74h
    virtual void mover_invalidate_pose_tree(std::uintptr_t mover) = 0;  // +C8h = +10Ch = 0, 0042ED50 on the +48h/+44h children
    virtual void game_set_camera_mover_004bc410(std::uintptr_t mover) = 0;
    // Update.
    virtual bool game_has_player_units_1970() = 0;                  // *(game+1970h) != 0
    virtual void show_message_004c7070(float a, float b, std::string_view text) = 0;  // cdecl(game, a, b, text)
    virtual float input_axis(FreeCameraAxis axis) = 0;
    virtual bool input_control_active(std::uint16_t control) = 0;   // byte +28h set and float +24h > 0.0
    virtual std::array<float, 9> mover_world_rows_cc(std::uintptr_t mover) = 0;  // after 00414DB0 when +C8h clear: rows +CCh, +DCh, +ECh
    virtual std::array<float, 3> mover_world_translation_fc(std::uintptr_t mover) = 0;  // the same matrix, +30h..+38h
    virtual std::array<float, 3>& mover_position_a4(std::uintptr_t mover) = 0;
    virtual void mover_set_rotation_00467050(std::uintptr_t mover, float x, float y, float z) = 0;  // ECX = mover+74h
    virtual bool action_pressed_004c43c0(std::int32_t action) = 0;
    virtual void mark_position_0053d230(const std::array<float, 3>& position) = 0;  // ECX = screen
    virtual void mover_invalidate_subtree_0042ed50(std::uintptr_t mover) = 0;
    virtual void mover_apply_shake_and_publish_004329d0(std::uintptr_t mover, float seconds) = 0;
    virtual void burst_update_0053f2d0(float seconds) = 0;          // ECX = screen+64h, (dt, mover+CCh)
};

// 0053DD90: slot +18h of vtable 00CEDCA8, __thiscall(screen), RET; SEH 00C6C7C8.
void free_camera_enter_0053dd90(FreeCameraState& screen, FreeCameraHost& host);
// 00540390: slot +20h of vtable 00CEDCA8, __thiscall(screen, float seconds), RET 4.
void free_camera_update_00540390(FreeCameraState& screen, float seconds, FreeCameraHost& host);

// Tactical library Enter 00511F40 (packet cc13-menus5). Library kinds (+94h) as the Enter's
// jump table 00514BD8 lays them out.
enum class TacticalLibraryKind : std::int32_t {
    PersonalAchievements = 0, PowerUps = 1, UnitsWeapons = 2, Movies = 3, NavalAcademy = 4,
    OrderOfBattle = 5, Unlocks = 6, None = 7, NoseArt = 8,
};
inline constexpr std::size_t kTacticalLibraryRowPairs = 0x28;           // the clone loop 005124CB
inline constexpr float kTacticalLibraryListPad130Oob = -0.018F;         // 00CEC03C (kind 5)
inline constexpr float kTacticalLibraryListPad130 = -0.022F;            // 00CEC038
inline constexpr float kTacticalLibraryListPad134 = -509.0F;            // 00CEC034
inline constexpr float kTacticalLibraryPanelSpacing = 0.46F;            // 00CEBE7C -> +43Ch
inline constexpr float kTacticalLibrarySidePanelScale = 0.5F;           // 00CE3800
inline constexpr std::string_view kTacticalLibraryControlNamesLua = "luaLoadControlFunctionNames()";  // 00CEC110

struct TacticalLibraryEnterState {
    TacticalLibraryKind kind_94{TacticalLibraryKind::None};
    std::uintptr_t entry_9c{};        // +9Ch, the entry the library was opened for (0 = none)
    std::int32_t entry_arg_a0{};      // +A0h, 2 = no entry page
    bool entry_unlocks_a4{};          // +A4h: kind 6 instead of 5
    std::uintptr_t item_group_ac{};   // +ACh
    std::uintptr_t main_text_b0{};    // +B0h
    std::uintptr_t list_b8{};         // +B8h, = +BCh for kind 5, else +C0h
    std::uintptr_t list_bc{};         // +BCh, order-of-battle list
    std::uintptr_t list_c0{};         // +C0h, menu list
    std::uintptr_t list_frame_c4{};   // +C4h (MainListbox_Text parent)
    std::array<std::uintptr_t, 3> hidden_128{};  // +12Ch, +128h, +130h (hidden first, in that order)
    std::uint32_t selected_value_3c0{};  // +3C0h, record +40h of the last marked academy entry
    float panel_spacing_43c{};
    float centre_x_440{};
    std::array<std::uintptr_t, 6> templates_4ec{};  // +4ECh, +4F0h, +4F4h, +4F8h, +500h, +504h (+4FCh unused)
    std::uintptr_t info_parent_4dc{};               // +4DCh
    std::array<std::uintptr_t, 3> panels_508{};     // +508h, +50Ch, +510h
    std::array<std::uintptr_t, 3> video_51c{};      // +51Ch.. (kind 4)
    std::array<std::uintptr_t, 3> video_pic_528{};  // +528h.. (kind 4, pic_BG_Icon clones)
    std::array<std::uintptr_t, 3> lock_540{};       // +540h.. (kind 0)
    std::array<std::uintptr_t, 3> hide_54c{};       // +54Ch.. (kind 3)
    bool show_side_arrows_560{};                    // +560h
    std::uintptr_t row_template_56c{};              // +56Ch
    std::uintptr_t row_template_570{};              // +570h
    std::array<std::uintptr_t, 2> arrows_574{};     // +574h, +578h
    std::array<std::uintptr_t, 3> big_bg_57c{};     // +57Ch, +588h, +594h (stride 0Ch, per panel)
    std::array<std::uintptr_t, 3> pic_bg_580{};     // +580h, +58Ch, +598h
    std::array<std::uintptr_t, 3> big_bg2_584{};    // +584h, +590h, +59Ch
    std::array<std::uintptr_t, 3> frames_5a0{};     // +5A0h, +5A4h, +5A8h (keret_Framebox)
    float field_5b4{};                              // +5B4h, -1 at exit
    std::array<std::uintptr_t, kTacticalLibraryRowPairs> rows_1d8{};  // +1D8h
    std::array<std::uintptr_t, kTacticalLibraryRowPairs> rows_278{};  // +278h
};

struct TacticalLibraryEnterHost {
    virtual ~TacticalLibraryEnterHost() = default;
    virtual void singleton_f8d394_00b33ac0(std::int32_t arg) = 0;   // ECX = *(00F8D394), 16 bytes
    virtual void reset_004f8fa0() = 0;                              // ECX = screen
    virtual bool lua_run_string_006b8ad0(std::string_view code) = 0;  // ECX = *(*(game+1A08h)+4), (code, 0, 0, 2); true = 0
    virtual void diagnostic_004254b0(std::string_view text) = 0;
    virtual void open_entry_unlocks_0050e3c0(std::uintptr_t entry, std::int32_t arg) = 0;  // (+9Ch, +A0h, 0)
    virtual void open_entry_order_of_battle_0050ef80(std::uintptr_t entry, std::int32_t arg) = 0;
    // Widgets.
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name, bool recursive) = 0;
    virtual std::uintptr_t clone_subtree_00aab4c0(std::uintptr_t widget, std::uintptr_t parent) = 0;
    virtual void widget_set_dword_e0(std::uintptr_t widget, std::int32_t value) = 0;
    virtual void widget_set_byte_77(std::uintptr_t widget, bool value) = 0;
    virtual void widget_set_byte_f4(std::uintptr_t widget, bool value) = 0;
    virtual void set_listener_00aa6bc0(std::uintptr_t widget, bool screen_plus_68) = 0;  // (screen+68h or 0, 0)
    virtual void widget_set_scale_48(std::uintptr_t widget, float x, float y) = 0;
    virtual float widget_local_x_00aa67f0(std::uintptr_t widget) = 0;
    virtual void widget_set_local_x_00aa78d0(std::uintptr_t widget, float x) = 0;
    virtual std::array<float, 2> group_size_00aa6740(std::uintptr_t widget) = 0;
    virtual void widget_set_size_58(std::uintptr_t widget, const std::array<float, 2>& size) = 0;
    virtual std::array<float, 3> group_origin_00aa6750(std::uintptr_t widget) = 0;
    virtual void set_resolved_position_00aa8240(std::uintptr_t widget, const std::array<float, 3>& position) = 0;
    virtual void list_set_pads_12c(std::uintptr_t list, float f12c, float f130, float f134) = 0;
    virtual void list_clear_rows_00a9bec0(std::uintptr_t list) = 0;
    virtual void widget_virtual_60(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual void widget_virtual_80(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual void list_set_listener_00a9ac40(std::uintptr_t list) = 0;   // screen+64h
    virtual void list_select_index_00a9c7c0(std::uintptr_t list, std::int32_t index) = 0;
    // Front end.
    virtual void frame_select_00518d60(std::string_view title) = 0;     // ECX = -1, EDX = 0, (0, title)
    virtual void set_help_line_0054a0c0(std::string_view text) = 0;     // ECX = *(00E1930C)
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void show_order_of_battle_page_004fff20() = 0;
    virtual void show_unlocks_page_004ffd00() = 0;
    // Screen-local builders (ECX = screen), one per kind; names are hypotheses from that use.
    virtual void prepare_achievements_00509de0() = 0;
    virtual void build_achievements_list_005012f0() = 0;
    virtual void build_power_up_list_00500e90() = 0;
    virtual void build_units_weapons_list_00500860() = 0;
    virtual void preview_setup_00502480() = 0;
    virtual void prepare_movies_004fac50() = 0;
    virtual void build_movie_list_00500450() = 0;
    virtual void build_academy_list_005010c0() = 0;
    virtual void build_order_of_battle_list_00506660() = 0;
    virtual void vehicle_class_registry_004356f0() = 0;   // ECX = BSP_VehicleClassRegistry_GetSingleton()
    virtual void prepare_unlocks_005021e0() = 0;
    virtual void build_unlocks_list_005001a0() = 0;
    virtual void build_nose_art_list_00500640() = 0;
    // Naval academy records: vector +404h..+40Ch (6Ch bytes), +40h value, +44h marked byte.
    virtual std::size_t academy_record_count() = 0;
    virtual void academy_set_marked_44(std::size_t index, bool marked) = 0;
    virtual std::uint32_t academy_value_40(std::size_t index) = 0;
    virtual std::vector<std::uint32_t> entry_ids_8c(std::uintptr_t entry) = 0;   // *(entry+8Ch..+90h)
    virtual std::size_t academy_find_004faac0(std::uint32_t id) = 0;           // (screen+404h, id)
    virtual void select_list_entry_0050d900(std::uint16_t list_offset) = 0;    // (screen+offset, screen+444h)
    virtual void show_selection_00511bf0(std::uint32_t value) = 0;
    virtual void nose_art_refresh_004fc8c0() = 0;
    virtual void unlocks_refresh_004fbe90(std::int32_t arg) = 0;
};

// 00511F40: slot +18h of vtable 00CEB9B4, __thiscall(screen), RET; SEH 00C6A616; 11488 bytes.
void tactical_library_enter_00511f40(TacticalLibraryEnterState& screen, TacticalLibraryEnterHost& host);

// 00515BC0: __thiscall(screen), RET; SEH 00C6A708. Leaves the 3D view if active, then the screen.
void tactical_library_leave_00515bc0(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host);
// 00515E00: __thiscall(screen), RET; SEH 00C6A748. Leaves the 3D view, or the screen through 00515BC0.
void tactical_library_back_00515e00(const TacticalLibraryNavState& screen, TacticalLibraryNavHost& host);

// ---------------------------------------------------------------------------
// Tactical library Update 005162B0, carousel step 00516010 and nose-art apply 005161C0
// (packet cc13-menus6). Reconstructed from the listing; the row loops' registers (ESI = screen,
// EDI = record index, EBP = vector/byte offset, [esp+10h] = row counter) were traced there.
// ---------------------------------------------------------------------------

inline constexpr float kTacticalLibraryNoCountdown = -1.0F;           // 00D7A260, +5B4h idle value
inline constexpr float kTacticalLibraryCountdownStart = 0.5F;         // 00CE3800 (kind 7 continue)
inline constexpr float kTacticalLibraryCarouselTime = 0.5F;           // 00CE3800 -> +8Ch
inline constexpr double kTacticalLibraryArrowWidthDivisor = 1.2000000476837158;  // 00CEC160
inline constexpr double kTacticalLibraryArrowDropOob = 0.017999999225139618;     // 00CEC158 (kind 5)
inline constexpr double kTacticalLibraryArrowDropUnlocks = 0.023000000044703484; // 00CEC150 (kind 6)
inline constexpr float kTacticalLibraryArrowZOob = -10.0F;            // 00CE6848 (kind 5 only)
inline constexpr float kTacticalLibraryCarouselSettleDt = 0.001F;     // 00D7A23C
inline constexpr double kTacticalLibraryAcademyScrollPad = 0.013888888888888888;  // 00CEBDD0 (kind 4)

// Arrow frames passed to widget virtual +88h(frame, 0, 1.0f).
inline constexpr std::uint16_t kTacticalLibraryArrowNormal = 0;
inline constexpr std::uint16_t kTacticalLibraryArrowDisabled = 3;
inline constexpr std::uint16_t kTacticalLibraryArrowLocked = 4;

// 2Ch records of the order-of-battle (+45Ch, begin +460h) and unlocks (+46Ch, begin +470h) vectors.
struct TacticalLibraryRowRecord {
    std::uint32_t key_00{};          // +00h, handed to 004FB090 (kind 5)
    std::uint8_t flag_0c{};          // +0Ch, kind 5: 0 hides the row's arrows, 1 locks a childless row
    std::size_t child_count_18{};    // vector +18h (+1Ch begin, +20h end, 4-byte elements); null = 0
};

struct TacticalLibraryUpdateState {
    TacticalLibraryKind kind_94{TacticalLibraryKind::None};
    std::int32_t view_90{};                                    // 0 = 2D list, 1 = 3D model view
    float carousel_timer_8c{};                                 // +8Ch
    float countdown_5b4{kTacticalLibraryNoCountdown};          // +5B4h
    float scroll_value_438{};                                  // +438h (selector +434h's value)
    std::uintptr_t list_b8{};                                  // +B8h
    std::uintptr_t list_c4{};                                  // +C4h
    std::array<std::int32_t, kTacticalLibraryRowPairs> sub_index_138{};  // +138h, per-row child index
    std::array<std::uintptr_t, kTacticalLibraryRowPairs> rows_1d8{};     // +1D8h, left arrows
    std::array<std::uintptr_t, kTacticalLibraryRowPairs> rows_278{};     // +278h, right arrows
    std::vector<TacticalLibraryRowRecord> order_of_battle_460;
    std::vector<TacticalLibraryRowRecord> unlocks_470;
    std::array<std::uintptr_t, 3> panels_508{};                // +508h, +50Ch, +510h
    std::array<std::uintptr_t, 3> frames_5a0{};                // +5A0h, +5A4h, +5A8h
};

struct TacticalLibraryUpdateHost : TacticalLibraryNavHost {
    virtual void scroller_update_00683820(std::int32_t arg) = 0;      // ECX = screen+CCh
    virtual void loading_queue_stop_and_wait_005092e0() = 0;           // ECX = 004FDE20()
    virtual void gui_reset_screens_00aa0f70() = 0;                     // ECX = 004C12B0()
    virtual std::int32_t list_selected_index_00a9c920(std::uintptr_t list) = 0;
    virtual void set_game_selection_6ac(std::int32_t index) = 0;       // game+6B0h, then game+6ACh
    virtual void play_selected_mission_005922f0() = 0;                 // ECX = *(*(00E198AC)+58h)
    virtual bool movie_playing_004f89a0() = 0;                         // ECX = *(00E18D48)
    virtual void movie_stop_004f8ac0() = 0;
    virtual void menu_stream_start_00a867b0(std::string_view name) = 0; // ECX = *(*(00E198AC)+50h)
    virtual void menu_stream_fade_in_00a85c20() = 0;
    virtual bool action_004d92b0(std::int32_t action) = 0;             // ECX = *(00E188A8)
    virtual void profile_hints_00427190(std::int32_t scene) = 0;       // ECX = 004C1E90()
    virtual void order_of_battle_cancel_004ff6f0() = 0;                // ECX = screen
    virtual std::uintptr_t list_row_at_00a9be00(std::uintptr_t list, std::int32_t index) = 0;
    virtual std::uintptr_t list_selected_row_00a9ca00(std::uintptr_t list) = 0;
    virtual std::string row_data_text_28(std::uintptr_t row) = 0;      // virtual +28h, then +4 (null = "")
    virtual void order_of_battle_row_004fb090(std::uint32_t key) = 0;  // ECX = screen
    virtual std::uintptr_t gui_highlight_widget_00aa0f50(std::int32_t arg) = 0;  // ECX = 004C12B0()
    virtual std::array<float, 2> group_size_00aa6740(std::uintptr_t widget) = 0;
    virtual std::array<float, 3> group_origin_00aa6750(std::uintptr_t widget) = 0;
    virtual void set_resolved_position_00aa8240(std::uintptr_t widget, const std::array<float, 3>& position) = 0;
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual void widget_set_dword_d8(std::uintptr_t widget, std::int32_t value) = 0;
    virtual std::int16_t widget_frame_94(std::uintptr_t widget) = 0;
    virtual void widget_virtual_88(std::uintptr_t widget, std::uint16_t frame, std::int32_t arg, float value) = 0;
    virtual void widget_virtual_60(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual void widget_virtual_84(std::uintptr_t widget, bool value) = 0;
    virtual bool widget_virtual_38(std::uintptr_t widget) = 0;
    virtual bool list_byte_85(std::uintptr_t list) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void kind7_activate_00514fb0() = 0;                        // ECX = screen
    virtual void switch_view_00514c20() = 0;                           // ECX = screen
    virtual void back_00515e00() = 0;                                  // ECX = screen
    virtual bool console_flag_f88a30() = 0;                            // byte 00F88A30
    virtual void show_selection_00511bf0(std::int32_t value) = 0;
    virtual void movies_cancel_004ff580() = 0;
    virtual void nose_art_apply_005161c0() = 0;
    virtual bool unlocks_cancel_004fc9d0() = 0;
    virtual void carousel_step_00516010(bool forward) = 0;
    virtual void carousel_animate_00515050(float dt) = 0;
    // Kind 8: the current profile (game+18CCh[game+18ECh]) and its nose art (game+734h / +730h).
    virtual bool profile_field_28() = 0;
    virtual std::uint32_t game_nose_art_734() = 0;
    virtual std::uint32_t game_nose_art_730() = 0;
    virtual std::uint32_t selector_record_48_004fa8f0() = 0;   // ECX = screen+434h (+508h panel)
    virtual std::uint32_t selector_record_48_004fa960() = 0;   // +50Ch panel
    virtual std::uint32_t selector_record_48_004faa00() = 0;   // +510h panel
};

// 005162B0: slot +20h of vtable 00CEB9B4, __thiscall(screen, float dt), RET 4; SEH 00C6A7E0.
void tactical_library_update_005162b0(TacticalLibraryUpdateState& screen, float dt,
    TacticalLibraryUpdateHost& host);

struct TacticalLibraryCarouselHost {
    virtual ~TacticalLibraryCarouselHost() = default;
    virtual void selector_step_004fc720() = 0;      // ECX = screen+434h, forward (action 4Ch)
    virtual void selector_step_004fc7a0() = 0;      // ECX = screen+434h, backward (action 4Dh)
    virtual void refresh_entry_00510c20() = 0;      // ECX = screen
    virtual void order_of_battle_refresh_00506550() = 0;
    virtual void unlocks_refresh_00504c60() = 0;
    virtual void carousel_animate_00515050(float dt) = 0;
    virtual void scroller_attach_00683790(std::int32_t content) = 0;     // ECX = screen+CCh
    virtual double text_normalized_height_00ab6bd0() = 0;                // ECX = screen+A8h, x87 result
    virtual std::array<float, 2> group_size_00aa6740(std::uintptr_t widget) = 0;
    virtual void scroller_set_range_006834a0(float range) = 0;           // ECX = screen+CCh
};

struct TacticalLibraryCarouselState {
    TacticalLibraryKind kind_94{TacticalLibraryKind::None};
    float carousel_timer_8c{};
    std::uintptr_t scroll_view_5ac{};   // +5ACh, its size.y is subtracted from the range
};

// 00516010: __thiscall(screen, bool forward), RET 4.
void tactical_library_carousel_step_00516010(TacticalLibraryCarouselState& screen, bool forward,
    TacticalLibraryCarouselHost& host);

struct TacticalLibraryNoseArtHost {
    virtual ~TacticalLibraryNoseArtHost() = default;
    virtual std::size_t nose_art_count_448() = 0;                 // vector +448h..+44Ch, 6Ch records
    virtual std::uint32_t nose_art_value_48(std::size_t index) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // *(*(00E198AC)+70h)+B8h: the selected row of the library's list, virtual +28h text.
    virtual std::string library_selected_row_text() = 0;
    virtual void set_game_nose_art_734(std::uint32_t value) = 0;
    virtual void set_game_nose_art_730(std::uint32_t value) = 0;
    virtual std::uint32_t game_nose_art_734() = 0;
    virtual std::uint32_t game_nose_art_730() = 0;
    // 007F8690, ECX = the current profile game+18CCh[game+18ECh], arg = {734h, 730h, 0}.
    virtual void profile_store_nose_art_007f8690(const std::array<std::uint32_t, 3>& record) = 0;
    virtual void back_00515e00() = 0;
};

// 005161C0: __thiscall(screen), RET. `selector_index` is screen+434h.
void tactical_library_apply_nose_art_005161c0(std::uint32_t selector_index, TacticalLibraryNoseArtHost& host);

// ---------------------------------------------------------------------------
// Menu prompt Exit 00530970 and HUD formation Enter/Exit 0053A4D0 / 00536800 (packet cc13-menus6)
// ---------------------------------------------------------------------------

// The four manager globals the prompt exit tests (each byte +3Ch).
enum class InterfaceManagerGlobal : std::int32_t { FrontEnd = 0, Interface = 1, Manager198b8 = 2, Title = 3 };  // 00E198AC, 00E198B4, 00E198B8, 00E198C8

struct MenuPromptExitHost {
    virtual ~MenuPromptExitHost() = default;
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual bool manager_present(InterfaceManagerGlobal which) = 0;
    virtual bool manager_byte_3c(InterfaceManagerGlobal which) = 0;
    virtual bool command_bar_present_e1930c() = 0;
    virtual void set_command_bar_byte_4() = 0;               // *(00E1930C)+4 = 1
};

struct MenuPromptExitState {
    std::uintptr_t panel_18{};      // +18h
    std::uintptr_t panel_24{};      // +24h
    bool flag_288{};                // +288h
    std::int32_t selection_284{};   // +284h
};

// 00530970: slot +1Ch of vtable 00CED3C4, __thiscall(screen), RET.
void menu_prompt_exit_00530970(MenuPromptExitState& screen, MenuPromptExitHost& host);

inline constexpr std::string_view kFormationHint = "FORMATIONPERMANENT";   // 00CED9CC
inline constexpr float kFormationLinkDepth = -1000.0F;                      // 00D7A240
inline constexpr GuiColour kFormationLinkColour{0.5F, 1.0F, 0.5F, 0.5F};    // 00CE3800 / 00D7A24C
inline constexpr std::int32_t kFormationLinkWidgetType = 0x0D;              // ECX of 00AA6640

struct HudFormationState {
    std::uintptr_t group_138{};                 // +138h, *(*(00E188D8)+284h)
    std::uintptr_t link_parent_98{};            // +98h
    std::uintptr_t member_list_ac{};            // +ACh
    std::uintptr_t row_template_b0{};           // +B0h
    std::uintptr_t marker_174{};                // +174h (virtual +30h position)
    std::uintptr_t marker_178{};                // +178h
    float leader_x_17c{};                       // +17Ch
    float leader_z_180{};                       // +180h
    float marker_y_188{};                       // +188h (for +174h)
    float marker_y_18c{};                       // +18Ch (for +178h)
    std::uint32_t leader_value_13c{};           // +13Ch
    bool flag_7c{};
    bool flag_7d{};
    // std::map<member, 1A0h marker> pairs +110h and +11Ch (values in key order) and the link
    // widgets vector +128h (begin +12Ch, end +130h).
    std::vector<std::uintptr_t> markers_110;
    std::vector<std::uintptr_t> markers_11c;
    std::vector<std::uintptr_t> links_12c;
};

struct HudFormationHost {
    virtual ~HudFormationHost() = default;
    virtual bool group_owner_present_e188d8() = 0;
    virtual std::uintptr_t group_owner_284() = 0;                       // *(00E188D8)+284h
    virtual void collapse_overlays_0068ab80(std::int32_t a, std::int32_t b) = 0;  // ECX = *(00E198C4)
    virtual std::uintptr_t group_leader_14(std::uintptr_t group) = 0;
    virtual std::int32_t group_member_count_4f8(std::uintptr_t group) = 0;
    virtual std::uintptr_t group_member_at_0070d060(std::uintptr_t group, std::int32_t index) = 0;
    virtual bool unit_pose_fresh_c8(std::uintptr_t unit) = 0;           // byte +C8h
    virtual void unit_refresh_pose_00414db0(std::uintptr_t unit) = 0;
    virtual float unit_matrix_x_fc(std::uintptr_t unit) = 0;            // +CCh matrix +30h
    virtual float unit_matrix_z_104(std::uintptr_t unit) = 0;           // +CCh matrix +38h
    virtual std::int32_t unit_icon_key_538(std::uintptr_t unit) = 0;
    virtual std::uint32_t unit_738_virtual_50(std::uintptr_t unit) = 0;  // *(unit+738h) vtable +50h
    virtual void widget_virtual_30(std::uintptr_t widget, const std::array<float, 3>& position) = 0;
    virtual void award_tracker_0068e420() = 0;                          // ECX = 004E1CA0()
    virtual void hud_refresh_0054e610() = 0;                            // ECX = *(*(00E198C4)+A8h)
    virtual void trigger_hint_00690fd0(std::string_view hint, std::int32_t arg) = 0;  // ECX = 004E1CA0()
    virtual void register_observer_pair_00694a60(std::uintptr_t unit) = 0;    // EDX = screen+64h
    virtual void unregister_pair_006952a0(std::uintptr_t registrar) = 0;      // EDX = screen+64h
    // 00538D60(screen, map, member, 1.0f): map[member] = new 1A0h marker (005A6AD0).
    virtual std::uintptr_t add_member_marker_00538d60(std::uint16_t map_offset, std::uintptr_t member, float scale) = 0;
    virtual std::uintptr_t clone_subtree_00aab4c0(std::uintptr_t widget, std::int32_t arg) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;  // (name, 1)
    virtual std::int32_t icon_frame_00653200(std::int32_t key) = 0;      // ECX = *(*(00E198C4)+BCh)
    virtual void widget_virtual_88(std::uintptr_t widget, std::int32_t frame, std::int32_t arg, float value) = 0;
    virtual void widget_show_34(std::uintptr_t widget, bool shown) = 0;
    virtual void list_append_row_00a9d750(std::uintptr_t list, std::uintptr_t row) = 0;   // (row, 0, 0)
    virtual void list_select_row_00a9c740(std::uintptr_t list, std::uintptr_t row) = 0;
    virtual void list_clear_rows_00a9bec0(std::uintptr_t list) = 0;
    virtual std::uintptr_t create_widget_00aa6640(std::int32_t type, std::string_view name) = 0;  // (type, name, 0)
    virtual void widget_set_local_z_00aa7d40(std::uintptr_t widget, float z) = 0;
    virtual std::string link_texture_e08694() = 0;                       // char* global 00E08694
    virtual void curve_set_texture_00abdd80(std::uintptr_t widget, const std::string& texture) = 0;
    virtual void widget_set_colour_50(std::uintptr_t widget, const GuiColour& colour) = 0;
    virtual void append_child_00aaa5a0(std::uintptr_t parent, std::uintptr_t child) = 0;   // (child, 0)
    virtual void screen_refresh_00533a30() = 0;                          // ECX = screen
    virtual std::uintptr_t marker_registrar_4(std::uintptr_t marker) = 0;   // marker+4
    virtual void delete_virtual_0(std::uintptr_t object) = 0;            // vtable +0 (1)
    virtual void delete_virtual_4(std::uintptr_t object) = 0;            // vtable +4 (1)
};

// 0053A4D0: slot +18h of vtable 00CED6FC, __thiscall(screen), RET; SEH 00C6C640.
void hud_formation_enter_0053a4d0(HudFormationState& screen, HudFormationHost& host);
// 00536800: slot +1Ch of vtable 00CED6FC, __thiscall(screen), RET; no SEH frame.
void hud_formation_exit_00536800(HudFormationState& screen, HudFormationHost& host);

// Frame stats Register 0053C740 (screen 57h, vtable 00CEDA10).
struct FrameStatsState {
    std::uintptr_t page_0c{};      // "_Fps"
    std::uintptr_t fps_10{};       // Fps_Text
    std::uintptr_t warning_14{};   // Warning_Text
    std::uintptr_t min_fps_18{};   // MinFps_Text
    std::uintptr_t max_fps_1c{};   // MaxFps_Text
    // +20h..+64h: PG_<n>p_Text, PG_<n>_Text, PG_<n>s_Text for n = 1..6, in that order.
    std::array<std::array<std::uintptr_t, 3>, 6> groups_20{};
};

struct FrameStatsRegisterHost {
    virtual ~FrameStatsRegisterHost() = default;
    virtual void register_front_end_screen_004f71d0() = 0;
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;     // (name, 1, 0)
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;  // (name, 1)
    // 0053BC00 singleton, 00BE9620 returns singleton+4: the decoded payload (fingerprint_payload.hpp).
    virtual std::string fingerprint_payload_text() = 0;
    virtual void text_set_localised_00abbe50(std::uintptr_t text, const std::string& key, std::int32_t arg) = 0;
};

// 0053C740: slot +10h of vtable 00CEDA10, __thiscall(screen), RET; SEH 00C6C790.
void frame_stats_register_0053c740(FrameStatsState& screen, FrameStatsRegisterHost& host);

}  // namespace bsp
