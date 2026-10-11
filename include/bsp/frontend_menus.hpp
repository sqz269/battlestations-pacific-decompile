// Main-menu list pages, the new-profile text owner and the mission-tree DLC
// gates (packet cc13_frontend_menus, segment 20, 005805A0..005C5600).
//
// Addresses: 005805A0, 00580530 (DLC gates); 00584F50, 005853C0, 005886F0
// (list-page builders); 00588370, 00590E60, 005807D0, 00580780 (new-profile
// text owner, vtable 00CEFC24 at MainMenuScreen+0Ch); 00585EB0 (NativeString
// left trim). docs/CC13_FRONTEND_MENUS.md carries the evidence.
//
// These are new C++ interfaces, not drop-in binary replacements. Every native
// call the routines make is one host method, called in native order; widgets
// are named by the MainMenuScreen field that holds them. No layout here is
// binary compatible: offsets are comments.

#ifndef BSP_FRONTEND_MENUS_HPP
#define BSP_FRONTEND_MENUS_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bsp/main_menu_command_bar.hpp"
#include "bsp/mission_tree_data.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Mission-tree DLC gates
// ---------------------------------------------------------------------------

// 005805A0 (group 3) and 00580530 (group 4): __thiscall, ECX = the mission-tree
// screen (*(00E198AC)+5Ch), RET. Both bounds-check the 34h-stride group vector
// at +18h/+1Ch like vector::at (00BF6713 when the vector is null or shorter
// than group+1) and return whether that group's 434h-stride mission vector
// (+28h/+2Ch inside the entry) is non-empty. The single-player page uses them
// to hide rows 2 and 3, the two DLC campaigns.
inline constexpr std::size_t kIjnDlcMissionGroup = 3;  // 005805A0, +C4h/+C8h
inline constexpr std::size_t kUsnDlcMissionGroup = 4;  // 00580530, +F8h/+FCh

struct FrontEndMenuFailureHost {
    virtual ~FrontEndMenuFailureHost() = default;
    // 00BF6713, the CRT invalid-parameter path a failed vector::at reaches.
    // It does not return in the image; an implementation must not return.
    [[noreturn]] virtual void invalid_parameter_00bf6713() = 0;
};

bool mission_group_has_missions_005805a0_00580530(const MissionTreeTables& tree,
    std::size_t group, FrontEndMenuFailureHost& failure);

// ---------------------------------------------------------------------------
// The list-page builders
// ---------------------------------------------------------------------------

// The four widgets the builders hide before refilling the list box, named by
// their MainMenuScreen field.
enum class MainMenuPageWidget : std::uint16_t {
    MapPanel2F0 = 0x2F0,
    Panel3DC = 0x3DC,
    ListBox1B8 = 0x1B8,
    Panel41C = 0x41C,
};

struct MainMenuListPageSpec {
    std::uint32_t address;
    std::string_view title;             // 00518D60's name argument
    int page;                           // stored to 00E08874
    bool sets_frame_flag;               // 00518B90(ECX=2)
    bool hides_map_panel_2f0;
    bool detaches_listener;             // 00A9AC40(0) before, (screen+8) after
    bool starts_title_music;            // 005884A0 after clearing the rows
    bool marks_listbox_149;             // list box byte +149h = 1 before the rows
    std::uint32_t label_table;          // array of C-string pointers
    std::size_t label_count;
    std::uint32_t selection_global;     // the remembered row
    // true: 00584F50 selects `row count - 1` when the item index equals the
    // remembered index, inside the loop. false: the others select the
    // remembered value once, after the loop.
    bool selects_inside_loop;
    // Listbox virtual call order after the rows: +80h/+34h/+60h on 00584F50 and
    // 005853C0, +34h/+60h/+80h on 005886F0.
    bool virtual_80_first;
    bool marks_listbox_14a;
    bool places_from_558;               // 00AA8240(screen+558h..560h + 0.0f)
    bool sets_widget_listener_40;       // 00AA6BC0(screen+40h, 0)
    bool resets_selected_00e08878;      // 00E08878 = -1
    bool clears_multi_menu_flags;       // 005853C0's 00E198B4 tail
};

// Label tables read from the image (.data, never written in this segment).
inline constexpr std::array<std::string_view, 6> kSinglePlayerLabels{{
    "FE.main_ijn_campaign", "FE.main_usn_campaign", "FE.main_ijn_dlc",
    "FE.main_usn_dlc", "FE.main_training_grounds", "FE.main_skirmish"}}; // 00E087F0
// 00CEF784: one byte per single-player row; the row's +77h byte is set to
// (flag == 0). All six bytes are 1 in this image, so every row has +77h = 0.
inline constexpr std::array<std::uint8_t, 6> kSinglePlayerRowFlags{{1, 1, 1, 1, 1, 1}};
inline constexpr std::array<std::string_view, 2> kMultiplayerLabels{{
    "FE_pc.main_live", "FE_pc.main_lan"}}; // 00E08820
inline constexpr std::array<std::string_view, 7> kTacticalLibraryLabels{{
    "FE.main_navalacademy", "FE.main_unitsweapons", "FE.main_powerups",
    "FE.main_movies", "FE.main_personalachievements", "FE.main_leaderboards",
    "FE.main_credits"}}; // 00E08830

inline constexpr MainMenuListPageSpec kSinglePlayerPage{
    0x00584F50, "FE.main_singleplayer_title", 2, true, true, true, false, true,
    0x00E087F0, 6, 0x00E194C8, true, true, true, true, true, true, false};
inline constexpr MainMenuListPageSpec kMultiplayerPage{
    0x005853C0, "FE.main_multiplayer_title", 3, true, true, true, false, false,
    0x00E08820, 2, 0x00E194CC, false, true, true, true, true, true, true};
inline constexpr MainMenuListPageSpec kTacticalLibraryPage{
    0x005886F0, "FE.main_tacticallibrary", 0x0B, false, false, false, true, false,
    0x00E08830, 7, 0x00E194D0, false, false, false, false, false, false, false};

// The five command-bar triples all three builders pass to 0054B530 (ECX =
// *00E1930C): select A2h at placement 0, navigate B3h, an empty slot, back A3h
// at placement 2, an empty slot. The empty labels are the C string 00CE3A0C.
std::array<MainMenuCommandArgument, 5> main_menu_list_page_commands() noexcept;

// Opaque handle for a list row the clone call returns.
struct MainMenuWidgetRef {
    void* native{};
};

struct MainMenuListPageHost : FrontEndMenuFailureHost {
    // 00518D60: ECX = 0Ch, EDX = 0, stack 0 and the title NativeString, RET 8.
    virtual void set_frame_title_00518d60(std::uint32_t ecx, std::uint32_t edx,
        std::uint32_t stack_word, std::string_view title) = 0;
    // 00518B90, ECX = 2: stores 00E08654 and sets the frame_flag_Icon frame.
    virtual void set_frame_flag_00518b90(std::uint32_t flag) = 0;
    virtual void set_page_00e08874(int page) = 0;
    virtual void set_widget_visible(MainMenuPageWidget widget, bool visible) = 0; // virtual +34h
    virtual void listbox_set_listener_00a9ac40(bool screen_plus_8) = 0;  // null or screen+8
    virtual void listbox_clear_rows_00a9bec0() = 0;
    virtual void listbox_set_byte(std::uint16_t offset, std::uint8_t value) = 0; // +149h / +14Ah
    virtual void start_title_music_005884a0() = 0;                       // ECX = screen
    virtual const MissionTreeTables& mission_tree_00e198ac_5c() = 0;
    virtual MainMenuWidgetRef clone_row_00aab4c0() = 0;                        // ECX = screen+238h, 0
    virtual void listbox_insert_row_00a9d750(MainMenuWidgetRef row) = 0;       // (row, 0, 0)
    virtual void row_set_label_00abbe50(MainMenuWidgetRef row, std::string_view label) = 0; // localise 1
    virtual void row_set_index_d8(MainMenuWidgetRef row, int index) = 0;
    virtual void row_set_byte_77(MainMenuWidgetRef row, bool value) = 0;
    virtual int listbox_row_count_104() = 0;
    virtual void listbox_select_index_00a9c7c0(int index) = 0;
    virtual void listbox_virtual(std::uint16_t slot, bool value) = 0;    // +34h / +60h / +80h
    virtual void listbox_call_00a9cf30(bool a, bool b) = 0;
    // 00AA8240 on the list box with screen+558h..560h each added to 0.0f
    // through x87 and stored as float.
    virtual void listbox_place_from_558_00aa8240() = 0;
    virtual void listbox_set_widget_listener_00aa6bc0() = 0;            // (screen+40h, 0)
    virtual void rebuild_command_bar_0054b530(
        const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void set_selected_00e08878(std::int32_t value) = 0;
    // 005853C0 tail: when 00E198B4 is set, 005EA7F0 on its +44h (if set), then
    // (+40h)->F4h = 0 (if set).
    virtual void clear_multi_menu_flags_00e198b4() = 0;
    virtual std::int32_t remembered_selection(std::uint32_t global) = 0;
};

// 00584F50 / 005853C0 / 005886F0: __thiscall, ECX = main-menu screen, RET.
// One body driven by the spec; the builders differ only in the recorded flags.
void build_main_menu_list_page(const MainMenuListPageSpec& spec, MainMenuListPageHost& host);

// ---------------------------------------------------------------------------
// The numbered mission map icons (0058D9E0)
// ---------------------------------------------------------------------------

// 0058D9E0: __thiscall(screen, bool destroy), RET 4. Callers: the screen's
// enter (005987F0) and exit (0058F560). This is the routine
// docs/GAME_EXECUTABLE.md left open: it clones mission_mappoint_template_Icon
// (+114h) and mission_mapflag_template_Icon (+118h) into numbered icons.
//
// Five unrolled loops, one per mission-tree group, in this order. The tree's
// group numbering (0 training, 1 IJN, 2 USN, 3 IJN DLC, 4 USN DLC) is not the
// MissionGroup numbering 00599DB0 uses in main_menu_screen.hpp.
struct MissionMapIconGroup {
    std::size_t tree_group;          // vector::at index into the 34h groups
    std::uint16_t parent_field;      // the group widget both clones go under
    std::uint16_t point_vector;      // the screen vector the point is appended to
    bool us_flag_frame;              // flag virtual +88h(1, 0, 1.0f)
    std::uint32_t loop_start;
};

inline constexpr std::array<MissionMapIconGroup, 5> kMissionMapIconGroups{{
    {1, 0x314, 0x070, false, 0x0058DA65},  // missions_JP_Group
    {2, 0x310, 0x090, true, 0x0058DE17},   // missions_US_Group
    {0, 0x320, 0x0B0, false, 0x0058E227},  // training_Group
    {3, 0x31C, 0x0D0, false, 0x0058E623},  // missions_JP_DLC_Group
    {4, 0x318, 0x0F0, true, 0x0058EA23},   // missions_US_DLC_Group
}};
// The destroy path's order, 0058DA0D..0058DA3F.
inline constexpr std::array<std::uint16_t, 5> kMissionMapIconDestroyOrder{{
    0x314, 0x310, 0x320, 0x31C, 0x318}};
inline constexpr std::uint16_t kMapPointTemplateField = 0x114;
inline constexpr std::uint16_t kMapFlagTemplateField = 0x118;
inline constexpr std::string_view kMapPointPrefix = "mission_mappoint_";  // 00CEFBA4
inline constexpr std::string_view kMapFlagPrefix = "mission_mapflag_";    // 00CEFB90
inline constexpr std::string_view kMapIconSuffix = "_Icon";               // 00CED1BC

struct MissionMapIconHost : FrontEndMenuFailureHost {
    virtual const MissionTreeTables& mission_tree_00e198ac_5c() = 0;
    // 00AA83F0 on the group widget: deletes cloned children (+76h set)
    // through virtual +4(1) and recurses into the rest.
    virtual void delete_cloned_children_00aa83f0(std::uint16_t group_field) = 0;
    virtual void clear_selected_point_330() = 0;  // +330h = 0
    // 00AAB4C0: ECX = the template, stack = the parent widget.
    virtual MainMenuWidgetRef clone_template_00aab4c0(std::uint16_t template_field,
        std::uint16_t parent_field) = 0;
    virtual void widget_set_name_2c(MainMenuWidgetRef widget, std::string_view name) = 0;
    // 00AA7DC0 with the mission record's +6Ch Pos.
    virtual void widget_set_local_position_00aa7dc0(MainMenuWidgetRef widget,
        const std::array<float, 3>& position) = 0;
    // 0043DCA0 for +70h; the other four inline the push_back with 0043DB60 as
    // the grow path.
    virtual void append_point(std::uint16_t vector_field, MainMenuWidgetRef point) = 0;
    virtual void widget_virtual_88(MainMenuWidgetRef widget, std::int32_t frame,
        std::int32_t second, float third) = 0;
};

// "mission_mappoint_" + ordinal + "_Icon", built as NativeString_ConcatInt
// then concatenation, the ordinal restarting at 1 in every group.
std::string mission_map_icon_name(std::string_view prefix, std::int32_t ordinal);

void build_mission_map_icons_0058d9e0(bool destroy, MissionMapIconHost& host);

// ---------------------------------------------------------------------------
// The new-profile text owner (MainMenuScreen+0Ch, vtable 00CEFC24)
// ---------------------------------------------------------------------------

// 00A96750 (the base key handler) calls slot +4 after an edit, +8 on Enter and
// +0Ch on Escape. Slot +0 is 0057D3E0, which has no Ghidra function. Owner
// fields: +18h text NativeString, +20h cursor, +24h the name Text widget.
inline constexpr std::uint32_t kNewProfileOwnerVtable = 0x00CEFC24;
inline constexpr std::size_t kProfileNameMaxLength = 15;  // 00588370
inline constexpr std::string_view kNewProfilePrompt = "FE.main_newprofile";   // 00CEF7A8
inline constexpr std::string_view kDefaultProfileName = "globals.newplayer";  // 00CEF794
inline constexpr std::string_view kProfileNameTrimSet = " ";                  // 00CE3A90
inline constexpr std::string_view kProfileNameConflict = "FE.profile_nameconflict"; // 00CEFCA0
inline constexpr std::string_view kProfileCannotCreate = "FE_pc.profile_cantcreate"; // 00CEFC84

struct NewProfileEditState {
    std::string text;       // owner+18h
    std::int32_t cursor{};  // owner+20h
};

struct NewProfileTextHost {
    virtual ~NewProfileTextHost() = default;
    // 00ABAED0 on the name widget with the owner's text, flag 0.
    virtual void name_set_source_00abaed0(std::string_view text) = 0;
    // 00ABBE50 on the name widget, localise 1.
    virtual void name_set_cstring_00abbe50(std::string_view text) = 0;
    // 00AB95F0: Text +17Ch cursor mode; 2 hides the caret.
    virtual void name_set_cursor_mode_00ab95f0(std::int32_t mode) = 0;
    // 00AB9240: caret index, clamped by the widget to its text length.
    virtual void name_set_cursor_00ab9240(std::int32_t index) = 0;
    virtual bool caret_flag_00e12f34() = 0;
    // 00A966E0 on the owner with (0, 0): disables and clears the editor.
    virtual void deactivate_editor_00a966e0() = 0;
    // virtual +60h(1) on the top-level list box, then its +77h byte = 0.
    virtual void reenable_list_box() = 0;
};

// 00588370, slot +4: truncate to 15 bytes, clamp the cursor to 15, then show
// the text and caret (mode 1 unless 00E12F34 is set, position cursor+1).
void new_profile_text_changed_00588370(NewProfileEditState& state, NewProfileTextHost& host);

// 005807D0, slot +0Ch: restore the prompt, hide the caret, deactivate, re-enable
// the list. 00580780 is the same body addressed from the screen itself (the
// list box at +1B8h instead of owner+1ACh); nothing references it.
void new_profile_cancel_005807d0(NewProfileTextHost& host);

// 00585EB0: __thiscall(NativeString*, const char* set), RET 4. strspn over the
// set from the front; all-matching erases through 005558F0(0, npos), otherwise
// memmove and resize. Null or empty set, or null data, is a no-op.
void trim_left_set_00585eb0(std::string& text, std::string_view set);

// The name 00590E60 commits: left trim " " (00585EB0), right trim " "
// (00584110, which keeps the first byte), then "globals.newplayer" when empty.
std::string normalised_profile_name(std::string_view text);

struct NewProfileCommitHost : NewProfileTextHost {
    // *0109CECC: +21h is the storage-ready byte; virtual +14h(&name) answers
    // whether a profile can be created under the name, +18h picks the message.
    virtual bool storage_ready_21() = 0;
    virtual bool storage_can_create_14(const std::string& name) = 0;
    virtual bool storage_virtual_18() = 0;
    virtual void profile_reset_to_defaults_007fdb20() = 0;        // *(00E188A8)+650h
    virtual void profile_set_name_007f9290(const std::string& name) = 0;
    virtual bool x360comp_00f88a30() = 0;
    virtual void input_settings_reset_006ac030() = 0;             // 005547D0 singleton
    // 007FA710(&name, 00590E40, 0): the write request; 00590E40 calls
    // 00590A00(1) on the main-menu screen (no Ghidra function there).
    virtual void profile_request_write_007fa710(const std::string& name) = 0;
    virtual void build_top_level_page_00584ae0(bool select_first) = 0;
    // 00531B00 on the 00425D10 singleton, in frontend_prompts.hpp's terms:
    // slot 0, the message, kind 2, callback 0057D3A0, unused 0, title "",
    // timeout 0.0f, timeout result 0, countdown key "" by value, dismissible 1.
    virtual void raise_prompt_00531b00(std::string_view message) = 0;
};

// 00590E60, slot +8 (Enter): __thiscall, ECX = owner (screen+0Ch), RET.
void new_profile_commit_00590e60(const NewProfileEditState& state, NewProfileCommitHost& host);

} // namespace bsp

#endif
