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
// ---------------------------------------------------------------------------
// Segment 79 widget types: Grid (8), Model (9), AnimIcon (12), Sound (14)
// ---------------------------------------------------------------------------
//
// Property visitors (the reader/describer object passed to vtable +18h/+1Ch):
// +0Ch with six stack dwords reads with a default, +10h with four reads
// without one; a describer's +0Ch takes four dwords and is only called when
// 00BD5680 says the value differs from the default (except where noted).
// Each (kind, pointer) and (0, key) pair is passed as an 8-byte value.

enum class GuiPropertyKind : std::uint8_t {
    String = 0,
    Float = 2,
    Bool = 3,
    Vec3 = 5,
    Vec2 = 6,
};

struct GuiPropertySpec {
    std::string_view key;
    GuiPropertyKind kind;
    std::uint16_t field;          // widget+N
    bool reads_with_default;      // visitor +0Ch (true) or +10h (false)
    std::array<float, 3> default_value;  // bools use [0] != 0; strings ignore it
};

// Grid (vtable 00D5CE60, 124h bytes, pool 00F8C050): +18h 00AC9900 reads
// through +10h, +1Ch 00AC9950 always describes.
inline constexpr std::array<GuiPropertySpec, 1> kGuiGridProperties{{
    {"CellSize", GuiPropertyKind::Vec2, 0x104, false, {0, 0, 0}}}};
// Model (vtable 00D5CF38, 11Ch bytes): +18h 00ACCE20 then 00ACCCF0(+10Ch, +114h).
inline constexpr std::array<GuiPropertySpec, 4> kGuiModelProperties{{
    {"ModelName", GuiPropertyKind::String, 0x10C, false, {0, 0, 0}},
    {"ModelTextureOverride", GuiPropertyKind::String, 0x114, true, {0, 0, 0}},  // default "" (00CE3A0C)
    {"ScaleVector", GuiPropertyKind::Vec3, 0xF4, true, {1, 1, 1}},
    {"RotationEuler", GuiPropertyKind::Vec3, 0x100, true, {0, 0, 0}}}};
// Sound (vtable 00D5CFD0, 108h bytes): +18h 00ACDDB0, +1Ch 00ACD800 (always
// describes, empty C string 00F8C134 for a null buffer).
inline constexpr std::array<GuiPropertySpec, 2> kGuiSoundProperties{{
    {"SoundEfx", GuiPropertyKind::String, 0xF8, false, {0, 0, 0}},
    {"Category", GuiPropertyKind::String, 0x100, false, {0, 0, 0}}}};
// AnimIcon (vtable 00D5BCE0 over the Icon class, 144h bytes): +18h 00AD3430
// after BSP_GuiIcon_ReadProperties, +1Ch 00AD3550 describes non-defaults.
inline constexpr std::array<GuiPropertySpec, 4> kGuiAnimIconProperties{{
    {"FPS", GuiPropertyKind::Float, 0x138, true, {30.0f, 0, 0}},  // 00CE38C8
    {"PlaybyDefault", GuiPropertyKind::Bool, 0x140, true, {1, 0, 0}},
    {"Loop", GuiPropertyKind::Bool, 0x141, true, {1, 0, 0}},
    {"Forward", GuiPropertyKind::Bool, 0x142, true, {1, 0, 0}}}};

struct GuiPropertyVisitor {
    virtual ~GuiPropertyVisitor() = default;
    virtual void base_properties() = 0;   // 00AAA710 / 00AAA700 / the Icon reader or describer
    virtual void read(const GuiPropertySpec& spec) = 0;               // visitor +10h
    virtual void read_with_default(const GuiPropertySpec& spec) = 0;  // visitor +0Ch, six dwords
    virtual void describe(const GuiPropertySpec& spec) = 0;           // describer +0Ch, four dwords
    virtual bool equals_default_00bd5680(const GuiPropertySpec& spec) = 0;
};

// The shared reader/describer shape of 00AC9900, 00AC9950, 00ACCE20 (first
// half), 00ACDDB0 (first half), 00ACD800, 00AD3430, 00AD3550.
template <std::size_t N>
void gui_read_properties(const std::array<GuiPropertySpec, N>& specs, GuiPropertyVisitor& visitor) {
    visitor.base_properties();
    for (const GuiPropertySpec& spec : specs) {
        if (spec.reads_with_default) {
            visitor.read_with_default(spec);
        } else {
            visitor.read(spec);
        }
    }
}
template <std::size_t N>
void gui_describe_properties(const std::array<GuiPropertySpec, N>& specs, bool always,
    GuiPropertyVisitor& visitor) {
    visitor.base_properties();
    for (const GuiPropertySpec& spec : specs) {
        if (always || !visitor.equals_default_00bd5680(spec)) {
            visitor.describe(spec);
        }
    }
}

// Grid object: +20h/+24h size (base), +ECh..+F8h columns (std::vector of
// 14h-byte records: width +0h, cell widgets std::vector at +4h..+10h),
// +104h/+108h CellSize, +110h/+114h scroll offsets, +118h Visual_Group,
// +11Ch/+120h content size.
struct GuiGridColumn {
    float width{};
    std::vector<std::uintptr_t> cells;  // null entries are skipped
};

struct GuiGridState {
    float width_20{};
    float height_24{};
    std::vector<GuiGridColumn> columns_ec;
    float cell_w_104{};
    float cell_h_108{};
    float scroll_x_110{};
    float scroll_y_114{};
    std::uintptr_t visual_group_118{};
    float content_w_11c{};
    float content_h_120{};
};

struct GuiGridHost {
    virtual ~GuiGridHost() = default;
    virtual std::uintptr_t create_widget_00aa6640(int type, std::string_view name) = 0;
    virtual void append_child_00aaa5a0(std::uintptr_t child) = 0;            // (child, 0)
    virtual std::uintptr_t find_child_00aaa2b0(std::string_view name) = 0;  // on the grid
    virtual void set_local_xy_00aa7d00(std::uintptr_t widget, float x, float y) = 0;
    virtual void scrollbar_set_maximum_00ac7de0(std::uintptr_t scrollbar, float maximum) = 0;
    virtual void scrollbar_set_value_00ac7d00(std::uintptr_t scrollbar, float value) = 0;
};

// 00ACA0B0, Grid vtable +74h (constructed), __thiscall, RET: a Group (type 2)
// named Visual_Group at +118h, appended as a child.
void gui_grid_constructed_00aca0b0(GuiGridState& grid, GuiGridHost& host);
// 00ACA170, +78h (loaded), RET: with a non-zero size, the vertical scrollbar
// goes to (width, 0) and the horizontal one to (0, height).
void gui_grid_loaded_00aca170(const GuiGridState& grid, GuiGridHost& host);
// 00ACA290, +84h, RET: lays the cells out column by column, records the
// content size and sets both scrollbars' maxima.
void gui_grid_layout_00aca290(GuiGridState& grid, GuiGridHost& host);
// 00ACA540, +7Ch, RET: scrollbar values = -offset (-0.0 at 00D7A208 minus).
void gui_grid_sync_scrollbars_00aca540(const GuiGridState& grid, GuiGridHost& host);
// 00ACB140, +0Ch, RET 4: true when the type id is in the chain 00F8C088..00F8C094.
bool gui_grid_is_kind_of_00acb140(const std::array<std::uint32_t, 3>& chain, std::uint32_t id) noexcept;

// AnimIcon setters, vtable 00D5BCE0 +A8h/+ACh/+B0h, RET 4.
struct GuiAnimIconFlags {
    float fps_138{30.0f};
    bool play_by_default_140{true};
    bool loop_141{true};
    bool forward_142{true};
};
inline void gui_anim_icon_set_forward_00ad2e00(GuiAnimIconFlags& icon, bool value) noexcept { icon.forward_142 = value; }
inline void gui_anim_icon_set_loop_00ad2e10(GuiAnimIconFlags& icon, bool value) noexcept { icon.loop_141 = value; }
inline void gui_anim_icon_set_fps_00ad2e20(GuiAnimIconFlags& icon, float value) noexcept { icon.fps_138 = value; }
// Sound widget (type 14): +ECh the sample (refcounted), +F0h the tracked
// sound, +F4h the sound class, +F8h SoundEfx, +100h Category.
struct GuiSoundWidgetState {
    std::uintptr_t sample_ec{};
    std::uintptr_t sound_f0{};
    std::int32_t sound_class_f4{};
    std::string sound_efx_f8;
    std::string category_100;
};

struct GuiSoundWidgetHost {
    virtual ~GuiSoundWidgetHost() = default;
    virtual void base_set_visible_00aa8530(bool visible) = 0;
    virtual std::int64_t find_sound_type_00a7b0a0(std::string_view name) = 0;  // EDX:EAX
    // 00A7E490 on *(00F8BBD8): (out, sample, class, type, 1); the temporary
    // handle is released after 0054D4C0 stores it at +F0h.
    virtual std::uintptr_t create_tracked_sound_00a7e490(std::uintptr_t sample,
        std::int32_t sound_class, std::int64_t sound_type) = 0;
    virtual void sound_stop(std::uintptr_t sound, bool flag) = 0;      // sound vtable +08h
    virtual bool sound_finished(std::uintptr_t sound) = 0;             // sound vtable +0Ch
    virtual void release_sound_0054d510(std::uintptr_t sound) = 0;      // adopt null into +F0h
    virtual std::uintptr_t acquire_sample_00a83fd0(const std::string& name) = 0;  // *(00F8BBE8)
    virtual std::int32_t find_sound_class_00a7acf0(const std::string& name) = 0;  // *(00F8BBD8)
    virtual void retain(std::uintptr_t object) = 0;                     // InterlockedIncrement +4h
    virtual void release(std::uintptr_t object) = 0;                    // decrement, slot 0 at zero
};

// 00ACDF10, Sound vtable +34h, __thiscall(bool), RET 4.
void gui_sound_set_visible_00acdf10(GuiSoundWidgetState& sound, bool visible, GuiSoundWidgetHost& host);
// 00ACD2C0, +40h (update), __thiscall(float), RET 4: hides the widget once
// its sound reports finished.
bool gui_sound_update_should_hide_00acd2c0(const GuiSoundWidgetState& sound, GuiSoundWidgetHost& host);
// The tail of 00ACDDB0 (+18h) after the two string reads.
void gui_sound_bind_resources_00acddb0(GuiSoundWidgetState& sound, GuiSoundWidgetHost& host);
// ---------------------------------------------------------------------------
// The powerup-manager screen, HUD slot 4Fh (manager +C0h), vtable 00CF4DEC
// ---------------------------------------------------------------------------

// 00614600 (layout, existing name BSP_HudPowerupManagerScreen_LoadLayout):
// two pages, then sixteen children. Parent field 0 means "the previous row's
// result" (center_Group is only held in EBX, never stored).
struct HudPowerupBinding {
    std::uint16_t field;         // screen+N that receives the widget (0: not stored)
    std::uint16_t parent_field;  // screen+N searched, or 0 for the previous row
    std::string_view name;
};
inline constexpr std::string_view kHudPowerupSupportPage = "GUI_support_full";   // -> +24h
inline constexpr std::string_view kHudPowerupInfoPage = "GUI_powerups_info";     // -> +B0h
inline constexpr std::array<HudPowerupBinding, 15> kHudPowerupBindings{{
    {0xB4, 0xB0, "New_Group"},
    {0xD4, 0xB4, "text_Text"},
    {0xE0, 0xB4, "left_circle_Icon"},
    {0xE4, 0xB4, "right_circle_Icon"},
    {0xE8, 0xB4, "empty_circle_Icon"},
    {0xEC, 0xB4, "tile_Icon"},
    {0xF0, 0xB4, "icon_Icon"},
    {0x00, 0x24, "center_Group"},
    {0xB8, 0x00, "power_up_Group"},
    {0xC0, 0xB8, "color_Icon"},
    {0xBC, 0xB8, "radar_Icon"},
    {0xC4, 0xB8, "sipp_Icon"},
    {0xC8, 0xB8, "szub_Icon"},
    {0xCC, 0xB8, "plane_Icon"},
    {0xD0, 0xB8, "command_Icon"},
}};

struct HudPowerupScreenState {
    std::uintptr_t support_page_24{};
    std::uintptr_t info_page_b0{};
    std::array<std::uintptr_t, 0x100 / 4> widgets{};  // indexed by field / 4 for +B4h..+F0h
    int mode_20{};
    int field_228{};
    float field_1e8{};
    int field_1ec{};
    int field_1f0{};
    int field_1f4{};
    bool flag_1f9{};
    float field_1fc{};
    bool flag_22c{};
    float field_230{};
    float field_234{};
    float right_circle_23c[3]{};  // +23Ch..+244h: resolved position, x minus +234h
    float info_page_f4[3]{};      // +F4h..+FCh: the info page's resolved position
    std::vector<std::uint32_t> list_1a4;
};

struct HudPowerupHost {
    virtual ~HudPowerupHost() = default;
    virtual void front_end_register_004f71d0() = 0;
    virtual std::uintptr_t load_page_00aa5840(std::string_view name) = 0;
    virtual std::uintptr_t find_child_00aa7e00(std::uintptr_t parent, std::string_view name) = 0;
    virtual void set_visible(std::uintptr_t widget, bool visible) = 0;
    virtual void get_resolved_position(std::uintptr_t widget, float out[3]) = 0;  // 00AA6750
};

// 00614600, slot +14h, __thiscall(screen), RET.
void hud_powerup_layout_00614600(HudPowerupScreenState& screen, HudPowerupHost& host);
// 00614170, slot +10h, __thiscall(screen), RET.
void hud_powerup_register_00614170(HudPowerupScreenState& screen, HudPowerupHost& host);
// ---------------------------------------------------------------------------
// The plane-spawn camera, vtable 00CF4930 (3CCh bytes over base 00432750)
// ---------------------------------------------------------------------------
//
// Fields: +74h the published 4x4 matrix (rows +74h, +84h up, +94h forward,
// +A4h eye), +10h observer listener (00CF4914), +384h the home entity,
// +388h the plane, +38Ch a 4x4 seed matrix whose translation row is
// +3BCh..+3C4h.

struct Vec3f {
    float x{};
    float y{};
    float z{};
};

struct PlaneSpawnCameraState {
    std::uintptr_t home_384{};
    std::uintptr_t plane_388{};
    std::array<float, 16> seed_38c{};   // [12..14] = +3BCh..+3C4h
    std::array<float, 16> matrix_74{};  // [4..6] up, [8..10] forward, [12..14] eye
};

// The plane and home reads 0060CB60/0060CD50 make.
struct PlaneSpawnPlaneView {
    bool current_5c{};          // byte +5Ch
    int state_900{};            // +900h (1 = the parked case)
    std::uintptr_t parent_3c{};
    Vec3f local_a4;             // +A4h
    Vec3f world_fc;             // +FCh, the pose translation row
    Vec3f row0_cc;              // pose rows +CCh / +ECh after 00414DB0 refresh
    Vec3f row2_ec;
};

struct PlaneSpawnCameraHost {
    virtual ~PlaneSpawnCameraHost() = default;
    virtual bool is_kind_of(std::uintptr_t entity, int kind) = 0;            // vtable +5Ch
    virtual Vec3f home_kind9_position(std::uintptr_t home) = 0;               // *(home+538h)+808h
    virtual PlaneSpawnPlaneView plane(std::uintptr_t plane) = 0;              // refreshes the pose
    // 006BCD20(ECX = home, DL = 0): the AirOps block, 0 when none.
    virtual std::uintptr_t air_ops_block_006bcd20(std::uintptr_t home) = 0;
    // block+3Ch vtable +40h(out, plane) and its translation at +30h.
    virtual Vec3f parked_position(std::uintptr_t block, std::uintptr_t plane) = 0;
    // 00414E10 on home, then 00414D10 on plane+FCh.
    virtual Vec3f transform_into_home(std::uintptr_t home, std::uintptr_t plane) = 0;
    virtual void unregister_pair_006952a0(std::uintptr_t observed) = 0;       // EDX = camera+10h
    virtual void register_pair_00694a60(std::uintptr_t observed) = 0;
    virtual std::array<float, 16> block_matrix(std::uintptr_t block) = 0;     // block vtable +18h
    virtual std::array<float, 16> plane_matrix(std::uintptr_t plane) = 0;     // plane+CCh
    virtual float tuning_7c() = 0;                                            // 0042E740 singleton +7Ch
    virtual void camera_virtual_ac(std::uintptr_t home) = 0;                  // vtable +ACh
    virtual void orthonormalize_0085dc80(std::array<float, 16>& matrix) = 0;
    virtual void publish_pose_004329d0(float dt) = 0;
};

// 0060CB60: __thiscall(camera, Vec3* out), out in EAX, RET 4.
Vec3f plane_spawn_camera_target_0060cb60(const PlaneSpawnCameraState& camera,
    PlaneSpawnCameraHost& host);
// 0060CC90, vtable +DCh: __thiscall(camera, float dt), RET 4.
void plane_spawn_camera_update_0060cc90(PlaneSpawnCameraState& camera, float dt,
    PlaneSpawnCameraHost& host);
// 0060CD50: __thiscall(camera, home, plane), RET 8.
void plane_spawn_camera_set_targets_0060cd50(PlaneSpawnCameraState& camera,
    std::uintptr_t home, std::uintptr_t plane, PlaneSpawnCameraHost& host);
// The eye offset 0060CE67..0060CF56 adds to +3BCh..+3C4h (x87, FSTP points cast).
Vec3f plane_spawn_camera_eye_offset(const Vec3f& row0, const Vec3f& row2, float tuning) noexcept;
inline constexpr double kPlaneSpawnEyeUp = 0.0;      // 00D7A258
inline constexpr double kPlaneSpawnEyeBack = 1.5;    // 00CE3D78
inline constexpr double kPlaneSpawnEyeHeight = 1.2000000476837158;  // 00CEC160
struct PlaneSpawnAttachHost {
    virtual ~PlaneSpawnAttachHost() = default;
    virtual void register_pair_00694a60(std::uintptr_t observed) = 0;     // EDX = screen+8
    virtual std::uintptr_t screen_home_entity() = 0;                      // vtable +28h (0060CA20)
    virtual std::uintptr_t air_ops_block_006bcd20(std::uintptr_t home) = 0;
    virtual std::uintptr_t holder_entity_7c(std::uintptr_t block) = 0;     // block+7Ch
    // operator new(3CCh) + memset + 00432750 + the four vtable stores; 0 on failure.
    virtual std::uintptr_t create_camera() = 0;
    // 00923870(ECX = camera, 0, game+19CCh, identity 4x4); runs even for a null camera.
    virtual void init_camera_00923870(std::uintptr_t camera) = 0;
    virtual void set_targets_0060cd50(std::uintptr_t camera, std::uintptr_t home, std::uintptr_t plane) = 0;
    virtual void set_camera_mover_004bc410(std::uintptr_t camera) = 0;
};

// 0060CF90 BSP_HudPlaneSpawnScreen_AttachPlane: __thiscall(screen, plane), RET 4.
// Caller 0068ACA0 BSP_InGameInterface_ApplyPendingInterface.
void plane_spawn_attach_plane_0060cf90(PlaneSpawnScreenState& screen, std::uintptr_t plane,
    PlaneSpawnAttachHost& host);
}  // namespace bsp

#endif  // BSP_HUD_WIDGETS_HPP
