// In-game HUD screens of segment 25 and GUI widgets of segment 79
// (packets cc13_hud1_seg25 / cc13_hud1_seg79). docs/CC13_HUD_AND_WIDGETS.md
// carries the evidence for every routine named here.
//
// New C++ interfaces, not drop-in binary replacements; offsets are comments.
// Widget handles are opaque std::uintptr_t values; NativeString storage is
// projected into std::string and is not reimplemented.

#ifndef BSP_HUD_WIDGETS_HPP
#define BSP_HUD_WIDGETS_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/main_menu_command_bar.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The multiplayer scoreboard, HUD screen slot 5Eh (manager +E0h)
// ---------------------------------------------------------------------------
//
// Object (constructor 0060FD90, destructor body 0060FE50):
//   +00h vtable 00CF4B98      +04h requested byte   +05h applied byte
//   +08h listbox listener, vtable 00CF4B84
//   +0Ch widget listener, vtable 00CF4B64
//   +10h Header_Group          +14h Player_Group clone (the row template)
//   +18h Main_Listbox          +1Ch..+28h team 0 rows (std::vector, begin +20h)
//   +2Ch..+38h team 1 rows (begin +30h)
//   +3Ch team 0 header clone   +40h team 1 header clone
//   +44h the GUI_scoring page

// One 24h-byte row record (copy 0060DFB0 / 0060F020, destroy 0060DE10).
struct HudScoreRow {
    std::string name;            // +00h NativeString
    std::string clan;            // +08h NativeString
    int score{};                 // +10h
    bool valid{};                // +14h; the enter's row loop stops at the first false
    int slot{};                  // +18h index into game+18CCh[8]
    std::uintptr_t player{};     // +1Ch the player record
    std::uintptr_t row_widget{}; // +20h the listbox row the enter cloned
};

// What the scoreboard reads from one game+18CCh player record.
struct HudScoringPlayer {
    bool active{};            // +08h
    bool ai{};                // +09h
    bool ai_listed{};         // +0Ah; an AI slot without it gets no row
    int team{};               // +28h; 0 -> rows +1Ch, 1 -> rows +2Ch, else none
    int ai_level{};           // +2Ch index into the 00E08940 name table
    std::string name;         // C string at +58h
    std::string clan;         // C string at +78h
    std::uint32_t xuid_lo{};  // +80h
    std::uint32_t xuid_hi{};  // +84h
    int score_index{-1};      // +20h; -1 means no score record (update skips the row)
};

// 00E08940: the AI difficulty labels the enter shows as an AI player's name.
inline constexpr std::array<std::string_view, 6> kHudScoringAiLevelNames{
    "FE.easy", "FE.easy", "FE.easy", "FE.easy", "FE.normal", "FE.hard"};

// 00E08C5C..00E08C70: the five ping icons 0060EF50 hides.
inline constexpr std::array<std::string_view, 5> kHudScoringPingIcons{
    "ping_1_Icon", "ping_2_Icon", "ping_3_Icon", "ping_4_Icon", "ping_5_Icon"};

inline constexpr float kHudScoringListboxSpacing = 0.0225f;  // 00CF4C34 -> listbox+12Ch
inline constexpr double kHudScoringRowStep = 0.03;           // 00CEB690 (double)
inline constexpr double kHudScoringTeamGap = 0.015;          // 00CEE6C8 (double)

// Command-bar action bytes carried in the first byte of a button's name
// (widget+F0h) and passed to 0054B530.
inline constexpr std::uint8_t kHudScoringBackCommand = 0xA3;      // -5Dh
inline constexpr std::uint8_t kHudScoringGamerCardCommand = 0xA5; // -5Bh

struct HudScoringScreenState {
    bool requested_04{};
    bool applied_05{};
    std::uintptr_t header_group_10{};
    std::uintptr_t row_template_14{};
    std::uintptr_t listbox_18{};
    std::vector<HudScoreRow> team0_rows_1c;
    std::vector<HudScoreRow> team1_rows_2c;
    std::uintptr_t team0_header_3c{};
    std::uintptr_t team1_header_40{};
    std::uintptr_t page_44{};
};

struct HudScoringHost {
    virtual ~HudScoringHost() = default;
    // GUI manager and widgets.
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;  // (name, 1, 0)
    virtual void remove_and_destroy_page(std::uintptr_t page) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;
    virtual std::uintptr_t clone_subtree_00aab4c0(std::uintptr_t widget) = 0;  // argument 0
    virtual void set_visible(std::uintptr_t widget, bool visible) = 0;          // vtable +34h
    virtual void set_alpha(std::uintptr_t widget, float alpha) = 0;             // vtable +4Ch
    virtual int widget_kind(std::uintptr_t widget) = 0;                         // vtable +5Ch
    virtual void listbox_virtual_60(std::uintptr_t listbox, bool value) = 0;   // vtable +60h
    virtual void select_state(std::uintptr_t icon, int state, int sub, float value) = 0;  // vtable +88h
    virtual void set_name(std::uintptr_t widget, const std::string& name) = 0;  // vtable +2Ch
    virtual void set_text(std::uintptr_t text, const std::string& value, bool localise) = 0;  // 00ABAED0
    virtual void set_text_key(std::uintptr_t text, std::string_view key) = 0;   // 00ABBE50(key, 1)
    virtual void get_resolved_position(std::uintptr_t widget, float out[3]) = 0;  // 00AA6750
    virtual void set_resolved_position(std::uintptr_t widget, const float in[3]) = 0;  // 00AA8240
    virtual void set_listener_fields_00aa6bc0(std::uintptr_t widget, bool widget_listener) = 0;
    virtual void set_row_flag_78(std::uintptr_t widget) = 0;                    // byte +78h = 1
    // Listbox.
    virtual void listbox_set_listener_00a9ac40(std::uintptr_t listbox) = 0;     // screen+8
    virtual void listbox_set_input_filter_00a9ac70(std::uintptr_t listbox) = 0; // 0060DBC0
    virtual void listbox_clear_rows_00a9bec0(std::uintptr_t listbox) = 0;
    virtual void listbox_set_spacing(std::uintptr_t listbox, float spacing) = 0; // +12Ch, +130h = +134h = 0
    virtual std::uint32_t listbox_field_104(std::uintptr_t listbox) = 0;
    virtual void set_row_field_d8(std::uintptr_t row, std::uint32_t value) = 0;
    virtual void listbox_append_row_00a9d750(std::uintptr_t listbox, std::uintptr_t row) = 0;  // (row, 0, 0)
    virtual void listbox_select_row_00a9c740(std::uintptr_t listbox, std::uintptr_t row) = 0;
    virtual void listbox_select_index_00a9c7c0(std::uintptr_t listbox, std::uint32_t index) = 0;
    // The selected row's name (00425E50, then the row's vtable +28h, string at +4h).
    virtual std::string selected_row_name(std::uintptr_t listbox) = 0;
    // Game state (00E188A8).
    virtual bool game_online_1fe4() = 0;                 // game+1FE4h != 0
    virtual bool game_flag_218c() = 0;                   // byte game+218Ch
    virtual HudScoringPlayer player(int slot) = 0;       // *(game+18CCh+4*slot)
    virtual int local_player_slot() = 0;                 // game+18ECh
    virtual int score_by_slot(int slot) = 0;             // game+21A0h -> +1E4h + slot*284h
    virtual int score_by_index(int index) = 0;           // the same field, index = player+20h
    virtual bool player_recently_spoke_007f74b0(int slot) = 0;
    // 00778740 on game+2080h: the 8x8 mute byte for (local, remote).
    virtual bool voice_muted_00778740(const HudScoringPlayer& local,
        const HudScoringPlayer& remote) = 0;
    // Front-end actions and platform (004D92B0 on game, 00F8ABE8, 00F8A2FC).
    virtual bool action_edge_004d92b0(int action) = 0;
    virtual bool pad_axis_10(float& value) = 0;          // first device of 00F8BBF4+94h, vtable +24h(10)
    virtual bool xenon_has_selected_user_00a3e510() = 0;
    virtual int xenon_selected_sign_in_state_00a3ead0() = 0;
    virtual bool xenon_signed_into_live_004b44f0() = 0;
    virtual bool platform_virtual_19c() = 0;             // *(00F8A2FC) vtable +19Ch
    virtual void show_gamer_card(const HudScoringPlayer& player) = 0;  // 00A3EAC0 slot, XShowGamerCardUI
    virtual void command_bar_rebuild_0054b530(
        const std::array<MainMenuCommandArgument, 5>& commands) = 0;   // ECX = *(00E1930C)
    // Screens.
    virtual void front_end_register_004f71d0() = 0;
    virtual void pop_request_interface_006881f0() = 0;
    virtual void raise_manager_screen_d8() = 0;          // *(00E198C4)+D8h: +4 = +5 = 1, commit, enter
    virtual void input_manager_update_00a92c40() = 0;    // 004BEC00 singleton, argument 0.0
    virtual void commit_visibility_004f83b0() = 0;
    // *(00E1930C): exit (+1Ch) if +5, clear +4/+5, then 004F83B0 on that screen.
    virtual void command_bar_exit_and_commit() = 0;
    virtual void destroy_widget(std::uintptr_t widget) = 0;  // vtable +4(1)
};

// 0060EF50: __thiscall(screen, Connection_Group), RET 4. Hides the five ping
// icons under the group.
void hud_scoring_hide_ping_icons_0060ef50(std::uintptr_t connection_group, HudScoringHost& host);

// 0060F210, slot +14h (layout), __thiscall(screen), RET.
void hud_scoring_layout_0060f210(HudScoringScreenState& screen, HudScoringHost& host);

// 0060D740, slot +10h (register), __thiscall(screen), RET: 004F71D0, the
// layout virtual, then the page is hidden.
void hud_scoring_register_0060d740(HudScoringScreenState& screen, HudScoringHost& host);

// 0060DD10, __thiscall(screen), RET: clear the requested byte, then hand back
// to the multiplayer request stack (online) or to the manager's +D8h screen.
void hud_scoring_close_0060dd10(HudScoringScreenState& screen, HudScoringHost& host);

// 0060D980, slot +1Ch (exit), __thiscall(screen), RET.
void hud_scoring_exit_0060d980(HudScoringScreenState& screen, HudScoringHost& host);

// 0060DBC0: the listbox input filter installed by 00A9AC70.
// __fastcall(ECX = up*, EDX = down*, stack select*), RET 4.
struct HudListboxInput {
    bool up{};
    bool down{};
    bool select{};
};
HudListboxInput hud_scoring_listbox_input_0060dbc0(HudScoringHost& host);

// Listbox listener 00CF4B84 slot +08h, 0060E0E0: __thiscall(listener =
// screen+8, sender, listbox), RET 8. Rebuilds the command bar for the
// selected row.
void hud_scoring_on_selection_changed_0060e0e0(HudScoringScreenState& screen,
    std::uintptr_t listbox, HudScoringHost& host);

// Widget listener 00CF4B64 slots +0Ch (0060D9F0) and +18h (0060DA40), RET 4:
// a kind-3 widget gets alpha 0.5 when byte +D4h is set, else 1.0.
void hud_scoring_button_alpha_0060d9f0(std::uintptr_t widget, bool selected_d4,
    HudScoringHost& host);

// Widget listener slot +04h, 0060DE90, RET 4: command buttons by name byte.
void hud_scoring_on_widget_activated_0060de90(HudScoringScreenState& screen,
    std::uintptr_t widget, std::uint8_t name_first_byte, std::uint32_t row_index_d8,
    HudScoringHost& host);

// 0060E440, slot +20h (update), __thiscall(screen, float), RET 4.
void hud_scoring_update_0060e440(HudScoringScreenState& screen, HudScoringHost& host);

// 00610070, slot +18h (enter), __thiscall(screen), RET.
void hud_scoring_enter_00610070(HudScoringScreenState& screen, HudScoringHost& host);

// ---------------------------------------------------------------------------
// The _PleaseWait screen (00E19698, registry slot 53h) outside its slots
// ---------------------------------------------------------------------------
//
// 14h-byte object: +04h requested, +05h applied, +08h the _PleaseWait page,
// +0Ch Background_Icon (0060D50D). docs/INTERFACE_RUNTIME_TAIL.md has the slots.

struct PleaseWaitScreenState {
    bool requested_04{};
    bool applied_05{};
    std::uintptr_t page_08{};
    std::uintptr_t background_0c{};
};

struct PleaseWaitHost {
    virtual ~PleaseWaitHost() = default;
    virtual void stop_render_worker_00aa2b80() = 0;       // on the GUI manager
    virtual void start_render_worker_00aa4040(std::uintptr_t descriptor,
        std::uintptr_t context, int rate) = 0;            // callback 0060D650
    virtual void commit_visibility_004f83b0() = 0;        // this screen
    virtual void screen_enter() = 0;                      // vtable +18h (0060D390)
    virtual void screen_exit() = 0;                       // vtable +1Ch (0060D2D0)
    virtual void set_visible(std::uintptr_t widget, bool visible) = 0;  // vtable +34h
    // 00AA3F70 on the GUI manager over a vector<page*> copy (004D35D0) whose
    // trailing float is 0.1 (00D7A2F0).
    virtual void gui_update_pages_00aa3f70(const std::vector<std::uintptr_t>& pages, float dt) = 0;
    virtual std::uintptr_t front_end_root_page() = 0;    // *(00E18D48)+1Ch
    virtual void game_render_frame() = 0;                // 004C6C30, 004CA440, 004CA1F0 on 00E188A8
};

inline constexpr float kPleaseWaitGuiStep = 0.1f;  // 00D7A2F0
inline constexpr int kPleaseWaitWorkerRate = 0x19;  // 0060D710

// 0060D3E0: __thiscall(screen), tail JMP 004F83B0. Callers 008C77F0
// (Loading_Finish) and 0076FAD0.
void please_wait_hide_0060d3e0(PleaseWaitScreenState& screen, PleaseWaitHost& host);
// 0060D6E0: __thiscall(screen, bool background_visible), RET 4. Callers
// 004D2BB0 BSP_Game_DestroyWorld and 008CC850.
void please_wait_show_0060d6e0(PleaseWaitScreenState& screen, bool background_visible,
    PleaseWaitHost& host);
// 0060D650: the render-worker callback, ECX = the context (the page), RET.
void please_wait_worker_frame_0060d650(std::uintptr_t page, PleaseWaitHost& host);
// 0060D570: no arguments, RET. Draws one synchronous frame while the screen
// at 00E19698 is applied. Callers 005635F0 and 0076FAD0 BSP_Session_SetMode.
void please_wait_render_frame_0060d570(const PleaseWaitScreenState* screen, PleaseWaitHost& host);

// ---------------------------------------------------------------------------
// The plane-spawn screen, HUD slot 41h (manager +74h), vtable 00CF4A7C
// ---------------------------------------------------------------------------
//
// Object (constructor 0060CAE0, destructor body 0060D130): +08h the observer
// listener (vtable 00CF4A64 over base 00CE3CD4; base fields +0Ch..+18h),
// +1Ch an observed entity, +20h the observed plane, +24h an object whose +7Ch
// is the observed entity, +28h the GUI_plane_spawn page.

struct PlaneSpawnScreenState {
    std::uintptr_t observed_1c{};
    std::uintptr_t plane_20{};
    std::uintptr_t holder_24{};
    std::uintptr_t page_28{};
};

// What 0060CA20 reads from the plane at +20h.
struct PlaneSpawnTargetView {
    bool flag_904{};
    bool has_bf4{};
    std::uint32_t bf4_entity_id{};   // *(*(+BF4h)+4)+7Ch
    float field_538_198{};           // *(+538h)+198h
    bool has_9d4{};
    std::uint32_t field_9d4_404{};   // *(+9D4h)+404h
};

struct PlaneSpawnHost {
    virtual ~PlaneSpawnHost() = default;
    virtual void unregister_pair_006952a0(std::uintptr_t observed) = 0;   // EDX = screen+8
    virtual std::uintptr_t holder_entity_7c(std::uintptr_t holder) = 0;   // holder+7Ch
    virtual void manager_c4_00681ec0() = 0;                               // ECX = *(00E198C4)+C4h
    virtual void hud_root_force_select_unit_006485a0() = 0;               // ECX = *(00E198C4)+40h
    virtual bool pilot_local_input_enabled_007bb9a0(std::uintptr_t plane) = 0;
    virtual std::uintptr_t plane_field_9d4(std::uintptr_t plane) = 0;
    virtual void hud_root_set_spectated_unit_00647300(std::uintptr_t unit) = 0;
};

// Listener 00CF4A64 slot +04h, 0060C7D0: __thiscall(screen+8, object), RET 4.
void plane_spawn_on_object_released_0060c7d0(PlaneSpawnScreenState& screen,
    std::uintptr_t object, PlaneSpawnHost& host);
// Slot +1Ch (exit), 0060C880: __thiscall(screen), tail JMP 00681EC0.
void plane_spawn_exit_0060c880(PlaneSpawnScreenState& screen, PlaneSpawnHost& host);
// Slot +20h (update), 0060C830: __thiscall(screen, float), RET 4.
void plane_spawn_update_0060c830(PlaneSpawnScreenState& screen, PlaneSpawnHost& host);
// Slot +28h, 0060CA20: __thiscall(screen), id in EAX, RET.
std::uint32_t plane_spawn_target_id_0060ca20(const PlaneSpawnTargetView& plane) noexcept;
inline constexpr float kPlaneSpawnTargetUnset = 0.0f;  // 00D7A218 (ucomiss, NaN counts as set)
}  // namespace bsp

#endif  // BSP_HUD_WIDGETS_HPP
