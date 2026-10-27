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
#include <vector>

#include "bsp/main_menu_command_bar.hpp"
#include "bsp/mission_tree_data.hpp"
#include "bsp/mission_tree_screens.hpp"

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

// ---------------------------------------------------------------------------
// The HUD unit icon index (0059CB90)
// ---------------------------------------------------------------------------

// 0059CB90: __fastcall(ECX = unit or null, EDX = class descriptor or null),
// RET, EAX = icon index. Callers: 0063D1E0 (BSP_HudMarker_WriteUnitMarkerWidgets),
// 0064AE20 (BSP_HudShipScreen45_ApplyUnit), 0063C620, 005AAC00 and 005ADDA0.
// Class ids are the entity IsKindOf ids of docs/RECON_SLOT_LISTS.md and
// docs/ENTITY_CLASS_IDS.md; projectile kinds are award_trackers.hpp's.
struct HudUnitRef {
    void* native{};
    explicit operator bool() const noexcept { return native != nullptr; }
};
struct HudClassRef {
    void* native{};
    explicit operator bool() const noexcept { return native != nullptr; }
};

struct HudUnitIconHost {
    virtual ~HudUnitIconHost() = default;
    virtual bool unit_is_kind_5c(HudUnitRef unit, int class_id) = 0;    // unit virtual +5Ch
    virtual bool class_is_kind_18(HudClassRef cls, int class_id) = 0;   // descriptor virtual +18h
    virtual HudClassRef unit_class_538(HudUnitRef unit) = 0;            // unit+538h
    virtual HudUnitRef squadron_leader_3d0(HudUnitRef squadron) = 0;    // unit+3D0h
    virtual std::uint8_t class_byte(HudClassRef cls, std::uint16_t offset) = 0;
    virtual std::uint8_t unit_byte_c24(HudUnitRef unit) = 0;
    // 007BCA50: +96Ch, or the first IsKindOf(20h) member whose +3F4h->+80h is
    // 10 answers its +3F8h->+34h->+8h; 0 otherwise.
    virtual std::int32_t plane_projectile_kind_007bca50(HudUnitRef plane) = 0;
    virtual HudUnitRef first_member_48(HudUnitRef unit) = 0;            // unit+48h
    virtual HudUnitRef next_member_44(HudUnitRef member) = 0;           // member+44h
};

std::int32_t hud_unit_icon_index_0059cb90(HudUnitRef unit, HudClassRef cls, HudUnitIconHost& host);

// ---------------------------------------------------------------------------
// The tactical map's nearest-icon pick (005A1310)
// ---------------------------------------------------------------------------

// 005A1310: __thiscall(GUI_map screen, bool skip_own_group), RET 4, EAX = the
// picked unit (a vehicle, or a squadron's +3D0h leader) or null. Callers:
// 00526A40 BSP_HudUnitPickScreen_PickUnit and 005A3210. It also stores the
// hovered entity at screen+520h.
//
// Screen fields read: +554h/+558h the cursor's world x/z, +54Ch/+550h the map
// origin x/z, +30h/+34h the map extent. World positions map to screen units as
// (x - origin_x) / extent_x + 0.5 and (origin_z - z) / extent_z + 0.5.
struct HudMapPickView {
    float cursor_x_554{};
    float cursor_z_558{};
    float origin_x_54c{};
    float origin_z_550{};
    float extent_x_30{};
    float extent_z_34{};
};

// The squared screen-unit radius a candidate must beat (00CF0008).
inline constexpr float kHudMapPickRadiusSquared = 0.0009f;
// 00E08890 / 00E08894, both 0.5f in the image (.data, not written here).
inline constexpr float kHudMapPickCentre = 0.5f;

// One node of the icon map at screen+4F4h/+4F8h: node+10h is the icon, whose
// +4h is the entity and +8h an active byte.
struct HudMapPickIcon {
    HudUnitRef unit;
    bool active{};
};

struct HudMapPickHost {
    virtual ~HudMapPickHost() = default;
    virtual bool unit_is_kind_5c(HudUnitRef unit, int class_id) = 0;
    virtual HudUnitRef unit_owner_140(HudUnitRef unit) = 0;              // virtual +140h
    virtual std::array<float, 3> unit_position_c4(HudUnitRef unit) = 0;  // virtual +C4h
    virtual HudUnitRef controlled_unit_00e188d8() = 0;
    virtual HudUnitRef squadron_leader_3d0(HudUnitRef squadron) = 0;
    virtual void set_hovered_520(HudUnitRef unit) = 0;
};

// The x87 arithmetic runs at the CRT's 53-bit precision control and stores to
// float at each FSTP, so the projection computes in double and narrows at the
// same points. Ties keep the earlier icon (FCOMIP/JBE).
HudUnitRef pick_nearest_map_unit_005a1310(const HudMapPickView& view,
    const std::vector<HudMapPickIcon>& icons, bool skip_own_group, HudMapPickHost& host);

// ---------------------------------------------------------------------------
// Minimap icon entries and the nearby capture point (005BD590, 005BD420)
// ---------------------------------------------------------------------------

// The minimap screen's templates and colours, bound by 005BE240 from
// `minimap_units_white_Group` (templates) and each colour group's
// `item_ship_Icon` (16-byte colours). This corrects docs/HUD_MINIMAP.md, which
// lists +6Ch..+7Ch as "the six groups' item_ship_Icon".
enum class MinimapField : std::uint16_t {
    ShipTemplate = 0x6C,       // item_ship_Icon      005BE397
    PlaneTemplate = 0x70,      // item_plane_Icon     005BE406
    SubTemplate = 0x74,        // item_sub_Icon       005BE475
    BuildingTemplate = 0x78,   // item_building_Icon  005BE4E4
    MarkerTemplate = 0x7C,     // item_marker_Icon    005BE553
    RedColour = 0x80,          // 005BE6A2
    BlueColour = 0x90,         // 005BE7BA
    YellowColour = 0xA0,       // 005BE9EA
    SilverColour = 0xB0,       // 005BEB02
    GreyColour = 0xC0,         // 005BE8D2
    WhiteColour = 0xD0,        // 005BE58A
    UnitMarkerGroup = 0xF0,    // unit_marker_Group, the entries' parent
    NearbyCapturePoint = 0xF4,
};

// 0Ch bytes: vtable 00CF1158 (one slot, the scalar deleting destructor
// 005BDB10, which deletes `widget` through its virtual +4(1)), +4h the unit,
// +8h the cloned icon.
struct MinimapIconEntry {
    HudUnitRef unit;
    MainMenuWidgetRef widget;
};

struct MinimapIconHost {
    virtual ~MinimapIconHost() = default;
    virtual bool unit_is_kind_5c(HudUnitRef unit, int class_id) = 0;
    virtual std::int32_t unit_side_54(HudUnitRef unit) = 0;
    // 00AAB4C0 on the template, parent 0. The minimap screen is *(00E198C4)+58h.
    virtual MainMenuWidgetRef clone_template_00aab4c0(MinimapField template_field) = 0;
    virtual void widget_set_colour_50(MainMenuWidgetRef widget, MinimapField colour_field) = 0;
    virtual void attach_to_unit_marker_group_00aaa5a0(MainMenuWidgetRef widget) = 0;  // (widget, 0)
    virtual void widget_set_visible_34(MainMenuWidgetRef widget, bool visible) = 0;
};

// 005BD590: __thiscall(entry, unit), RET 4, EAX = entry. Template by
// IsKindOf: submarine 08h, ship 06h, plane 0Fh, otherwise the building icon.
// Colour by unit+54h: 0 blue, 1 red, 2 grey; any other side keeps the
// template's own colour.
MinimapIconEntry construct_minimap_icon_entry_005bd590(HudUnitRef unit, MinimapIconHost& host);

struct CapturePointCandidate {
    HudUnitRef point;
    std::array<float, 3> position{};   // point+FCh..+104h after a pose refresh
    std::int32_t radius_7c4{};         // point+7C4h, an int
    std::int32_t side_54{};            // point+54h; 2 is the neutral side
};

// 005BD420: __fastcall(ECX = minimap screen), RET. +F4h = 0, then when the
// controlled unit 00E188D8 is a ship (06h) and the capture-point list at
// *(*(00E188A8)+19CCh)+16Ch is non-empty, the first point (list order, next at
// node+4h, point at node+8h) whose squared distance to the controlled unit is
// not above float(radius)^2 and whose side is 2 becomes +F4h. Both poses are
// refreshed first (00414DB0 when +C8h is clear), the point's before the unit's.
// The sum is (dy*dy + dx*dx) + dz*dz, stored to float, as is
// float(radius)*float(radius); an unordered compare selects (JA only). Main-thread
// x87 runs at 24-bit precision control, so every product and sum rounds to float.
// This pure form is the walk over candidates whose poses are already refreshed.
HudUnitRef select_nearby_capture_point_005bd420(const std::array<float, 3>& controlled_position,
    const std::vector<CapturePointCandidate>& points);

// The whole body of 005BD420 over a host (cc13-P4).
struct CapturePointSelectHost {
    virtual ~CapturePointSelectHost() = default;
    virtual void set_selected_capture_point_f4(HudUnitRef point) = 0;          // screen+F4h
    virtual HudUnitRef controlled_unit_00e188d8() = 0;                         // [00E188D8]
    virtual bool unit_is_kind_5c(HudUnitRef unit, std::int32_t kind) = 0;      // vtable+5Ch
    // *(*(00E188A8)+19CCh)+16Ch in list order (next node+4h, point node+8h).
    virtual std::vector<HudUnitRef> capture_point_list_16c() = 0;
    virtual void refresh_pose_if_stale_00414db0(HudUnitRef unit) = 0;          // when byte +C8h is 0
    virtual std::array<float, 3> unit_position_fc(HudUnitRef unit) = 0;        // +FCh..+104h
    virtual std::int32_t capture_radius_7c4(HudUnitRef point) = 0;             // +7C4h, int
    virtual std::int32_t unit_side_54(HudUnitRef unit) = 0;                    // +54h
};
void update_nearby_capture_point_005bd420(CapturePointSelectHost& host);

// ---------------------------------------------------------------------------
// Advancing the campaign selection after a mission (005C3DE0)
// ---------------------------------------------------------------------------

struct MissionAdvanceHost : FrontEndMenuFailureHost {
    virtual bool mission_completed_0090c560(const MissionRecord& record) = 0;
    // The NativeString at *(00E188A8)+6B8h, which is profile (+650h) +68h, the
    // string 00586150 copies out.
    virtual void set_profile_selected_mission_6b8(std::string_view id) = 0;
};

// 005C3DE0: __fastcall(ECX = mission-tree screen), RET, AL. Called from the
// scoring screen's enter 0062C2C0 (0062C866). Looks up the current mission key
// game+2198h (005C3470), stores the pair in +0Ch/+10h, steps the mission index
// by one when the current mission is completed and not the group's last,
// clamps it to [0, found + 1] with signed compares, copies that record's id to
// profile+68h and answers whether the index moved past the found mission.
bool advance_campaign_selection_005c3de0(MissionTreeScreenState& state,
    std::string_view current_mission_key, MissionAdvanceHost& host);

// ---------------------------------------------------------------------------
// PC controls screen (screen 0Fh, constructor 00555770): sensitivity rows
// ---------------------------------------------------------------------------

// 0055CCC0 reads the current device entry (input settings 005547D0 +18h
// vector, 8-byte stride, index screen+16Ch), its per-device record (0055C110
// on the map at settings+8h) and, keyed by the selected setting name at
// screen+184h, the setting kind (0055A9A0, map at record+18h) and float value
// (00444BE0, map at record+24h). Kinds 3 and 4 are the inversion toggles.
inline constexpr float kControlSensitivityStep = 0.25f;   // 00D7A348, a double
inline constexpr float kControlSensitivityMin = 0.25f;    // 00CE3868
inline constexpr float kControlSensitivityMax = 2.75f;    // 00CEE668
inline constexpr double kControlSliderScale = -4.0;       // 00CEE4E0
inline constexpr std::string_view kControlNormalLabel = "FE.opt_normal";      // 00CEE688
inline constexpr std::string_view kControlInvertedLabel = "FE.opt_inverted";  // 00CEE678

struct ControlSettingHost : FrontEndMenuFailureHost {
    virtual std::int32_t setting_kind_0055a9a0() = 0;
    virtual float setting_value_00444be0() = 0;
    // 006A9E70(device entry, &name, value): writes the value back.
    virtual void set_setting_value_006a9e70(float value) = 0;
    // 00ABBE50 on the `var_Text` child (00AA7E00) of the row at *(screen+190h)+70h.
    virtual void set_row_label_00abbe50(std::string_view label) = 0;
    // 005F3750 on the same row, EDX = the slider position.
    virtual void set_row_slider_005f3750(std::int32_t position) = 0;
    virtual void mark_changed_19c() = 0;   // screen+19Ch = 1
};

// 0055CCC0: __thiscall(screen, int delta), RET 4. Inversion kinds negate the
// value (SSE `-0.0 - v`) and label the row normal when the result is above
// 0.0f. Other kinds add delta * 0.25 (x87, stored to float), clamp to
// [0.25, 2.75], write it, then re-read it and set the slider to
// -1 - trunc(value * -4.0), i.e. 0..10. Both paths set +19Ch.
void adjust_control_setting_0055ccc0(std::int32_t delta, ControlSettingHost& host);

// The new value alone (both branches), for hosts that only model the data.
float adjusted_control_value_0055ccc0(std::int32_t kind, float old_value, std::int32_t delta) noexcept;
std::int32_t control_slider_position_0055ccc0(float value) noexcept;

struct ControlSliderListenerHost {
    virtual ~ControlSliderListenerHost() = default;
    virtual std::int32_t page_168() = 0;                         // screen+168h
    virtual std::uint32_t widget_id_d8() = 0;                    // widget+D8h
    virtual std::int32_t list_selected_index_00a9c920() = 0;     // list at screen+20h
    virtual std::int32_t slider_delta_005f2290() = 0;            // ECX = widget
    virtual void adjust_0055ccc0(std::int32_t delta) = 0;
};

// 0055D330: slot +00h of the controls screen's widget-listener vtable
// 00CEE4F0 (ECX = screen+0Ch), stdcall widget, RET 4. Outside page 3, a widget
// whose +D8h id is in 1..FFFFh and whose low byte equals the list's selected
// index forwards the slider's non-zero delta to 0055CCC0.
void control_slider_changed_0055d330(ControlSliderListenerHost& host);

// ---------------------------------------------------------------------------
// Controls screen update, 00560430 (cc13-frontend2)
// ---------------------------------------------------------------------------

// The raw input event 00A90530 fills on *(00F8BBF4): kind at +0, +4 and the
// code at +0Ch are written, +8 is not; +10h is a byte the caller sets for an
// unchanged slider. Kind 0 is a keyboard key (codes are DirectInput scan
// codes: D3h Delete, 1Ch Return), kind 2 an axis named through 0069E500.
struct ControlInputEvent {
    std::int32_t kind{-1};
    std::int32_t field04{};
    std::int32_t field08{};
    std::int32_t code{};
    bool flag10{};
};
inline constexpr std::int32_t kControlKeyDelete = 0xD3;
inline constexpr std::int32_t kControlKeyReturn = 0x1C;
inline constexpr std::int32_t kControlActionBack = 0x4B;
inline constexpr std::int32_t kControlActionLeft = 0x4C;
inline constexpr std::int32_t kControlActionRight = 0x4D;
inline constexpr std::string_view kControlPresetDirectory = "Presets";        // 00CEE76C
inline constexpr std::string_view kControlPresetPathFormat = "Presets/%S.preset";  // 00CEE8C8
inline constexpr std::string_view kControlPresetRoot = "CONTROL_PRESET";       // 00CEE75C
inline constexpr std::string_view kControlSliderEventName = "Slider";         // 00CEE8C0

struct ControlBindingUpdateState {
    float hint_timer{};                // +10h
    bool keyboard_pending{};           // +E4h, XOVERLAPPED at +44h
    std::u16string preset_name;        // +60h, wide, zero-terminated in place
    std::int32_t page{};               // +168h
    std::array<std::int32_t, 5> device_selection{};  // +16Ch..+17Ch, +16Ch the device index
    std::int32_t binding_slot{};       // +180h, 0 or 1
    bool editing_setting{};            // +18Ch
    bool input_seen{};                 // +194h
    bool skip_next_update{};           // +195h
    float input_hold{};                // +198h
    bool unsaved_changes{};            // +19Ch
    std::int32_t scroll_target{};      // +1C0h
    bool scroll_dragging{};            // +1E8h
    float last_slider_value{};         // +24Ch
};

using ControlBindingKey = std::uintptr_t;  // 00427AE0's tree node for the device

struct ControlBindingUpdateHost {
    virtual ~ControlBindingUpdateHost() = default;
    virtual float game_frame_seconds_21ec() = 0;                     // *(00E188A8)+21ECh
    virtual bool overlapped_pending_is_io_pending() = 0;              // +44h == 3E5h
    virtual std::uint32_t xget_overlapped_result_00a4d42e() = 0;      // (+44h, 0, 1)
    // 005EF500 on *(*(00E198B8)+40h) with a pointer to one name character.
    virtual void filter_name_char_005ef500(char16_t& c) = 0;
    virtual bool directory_exists_0109ceec_8(std::string_view path) = 0;  // *0109CEEC slot +8
    virtual void make_directory_00bf94c6(std::string_view path) = 0;
    virtual std::string format_preset_path_sprintf_s(std::u16string_view name) = 0;  // %S, 100h buffer
    virtual void preset_writer_open_00bd5d60(const std::string& path) = 0;
    virtual void preset_writer_root_00bd6db0(std::string_view root) = 0;
    virtual void write_control_preset_006a5640() = 0;                 // 005547D0 singleton
    virtual void preset_writer_close_00bd5860() = 0;
    virtual bool input_pending_00a90f90() = 0;                        // *(00F8BBF4)
    virtual bool action_004d92b0(std::int32_t action) = 0;
    virtual void set_global_e19650(std::int32_t value) = 0;
    virtual void push_interface_request_004cc460(std::int32_t id, std::int32_t arg) = 0;  // *(00E198B8)
    virtual bool x360comp_00f88a30() = 0;
    virtual void input_action_configuration_00698a10() = 0;           // *(00E188A8)+3Ch
    virtual void page_widget_virtual_34(std::int32_t arg) = 0;        // *(screen+210h) slot +34h
    virtual void refresh_page_0055e3a0(std::int32_t page) = 0;
    virtual void read_input_event_00a90530(ControlInputEvent& event, float& value) = 0;
    virtual std::string input_event_name_0069e500(const ControlInputEvent& event) = 0;
    virtual ControlBindingKey binding_key_00427ae0(std::int32_t device) = 0;  // settings+14h
    virtual void device_tree_index_0055c110(ControlBindingKey key) = 0;       // settings+8, result unused
    virtual void set_binding_006a9a40(ControlBindingKey key, std::int32_t slot,
        const ControlInputEvent& event, float value) = 0;           // name screen+184h
    virtual std::string binding_slot_text_006a9850(ControlBindingKey key, std::int32_t slot) = 0;
    virtual void binding_text_submit_00abaed0(const std::string& text) = 0;  // *(screen+190h), flag 1
    virtual void update_binding_row_text_0055c5a0() = 0;
    virtual void begin_edit_0055c3b0() = 0;
    virtual void confirm_or_next_device_00560210() = 0;
    virtual void adjust_setting_0055ccc0(std::int32_t delta) = 0;
    virtual void scroller_update_00683820() = 0;                      // screen+1A0h, arg 1
    virtual void scroll_to_selection_0055cec0(std::int32_t arg) = 0;
    virtual void focus_00553b00(bool binding_text_widget) = 0;        // *(screen+190h) or null
    virtual bool input_device_present_004ba6d0(std::int32_t device_class, std::int32_t index) = 0;
    virtual void set_global_e19428(bool value) = 0;
    virtual void dismiss_prompt_slot_00532a20(std::int32_t slot) = 0;   // 00425D10 singleton
};

// 00560430: slot +20h of the controls screen vtable 00CEE524,
// __thiscall(screen, float seconds), RET 4; the argument slot is overwritten
// with *(00E188A8)+21ECh before use. Provisional page meanings (+168h): 0 device
// list, 1 waiting for back, 2 setting list, 3 capturing a binding, 4 waiting
// for a class-2 device.
void control_binding_screen_update_00560430(ControlBindingUpdateState& screen,
    ControlBindingUpdateHost& host);

// ---------------------------------------------------------------------------
// Multiplayer mode selector enter (005EC5B0) and update (005ECD80)
// ---------------------------------------------------------------------------

// Row labels: the .data pointer table 00E0898C (initial values), rows enabled
// by the byte table 00CF2B00 (all 1 in the image). Row 5 is enabled only when
// 00E19610 == 2.
inline constexpr std::array<std::string_view, 6> kMultiModeLabels{{
    "globals.gamemode_escort", "globals.gamemode_siege", "globals.gamemode_competitive",
    "globals.gamemode_duel", "globals.gamemode_islandcapture", "globals.gamemode_any"}};
inline constexpr std::array<std::uint8_t, 6> kMultiModeRowFlags{{1, 1, 1, 1, 1, 1}};  // 00CF2B00
inline constexpr std::string_view kMultiModeSelectorTitle = "globals.multimodeselector_title";  // 00CF2CC0
inline constexpr std::string_view kMultiSkirmishTitle = "FE.main_skirmish_title";               // 00CF2CE0

// Widgets addressed through the screen: +1Ch, the mode list +20h, the option
// list +A8h, and +B0h/+B4h which follow the option list's +F4h height.
enum class MultiModeWidget : std::uint8_t { Panel1C, ModeList20, OptionListA8, FollowerB0, FollowerB4 };

struct MultiModeSelectorState {
    bool option_list_active{};     // +E0h
    std::int32_t field_e4{};       // +E4h..+F0h set by enter to 8, 0, 13h, 0
    std::int32_t field_e8{};
    std::int32_t field_ec{};
    std::int32_t field_f0{};
    std::int32_t pending_selection{-1};  // +F4h
    std::int32_t session_stage{};  // +F8h: 3 idle; 0, 1, 2 run the +FCh timer
    float session_timer{};         // +FCh
    std::uint32_t qos_handle{};    // +10h, XNQOS*
};

struct MultiModeSelectorHost {
    virtual ~MultiModeSelectorHost() = default;
    // Enter.
    virtual bool interface_query_00687860() = 0;  // *(00E198B4), (4, 0)
    virtual bool skirmish_218c() = 0;             // *(00E188A8)+218Ch
    // 00518D60: ECX = 0, EDX = 0, stack 0 and the title, as frontend_menus' list pages.
    virtual void set_frame_title_00518d60(std::string_view title) = 0;
    virtual void set_frame_flag_00518b90(std::uint32_t flag) = 0;  // ECX = 2
    virtual std::int32_t global_e19610() = 0;
    virtual void widget_virtual(MultiModeWidget widget, std::uint16_t slot, std::int32_t arg) = 0;  // +34h/+60h/+80h
    virtual void set_byte_77(MultiModeWidget widget, bool value) = 0;
    virtual void listbox_set_listener_00a9ac40(MultiModeWidget list, bool screen_plus_8) = 0;
    virtual void listbox_set_widget_listener_00aa6bc0(MultiModeWidget list, bool screen_plus_c) = 0;  // (ptr, 0)
    virtual void listbox_clear_rows_00a9bec0() = 0;                      // mode list
    // Clone of the mode list's "MainListbox_Text" child (00AA7E00 name, 1;
    // 00AAB4C0(0)), appended with 00A9D750(row, 0, 0).
    virtual MainMenuWidgetRef add_mode_row() = 0;
    virtual void row_set_label_00abbe50(MainMenuWidgetRef row, std::string_view label) = 0;  // localise 1
    virtual void row_virtual_34(MainMenuWidgetRef row, bool enabled) = 0;
    virtual void row_set_byte_77(MainMenuWidgetRef row, bool value) = 0;
    virtual std::int32_t global_e19614() = 0;
    virtual void set_global_e19614(std::int32_t value) = 0;
    virtual void listbox_call_00a9cf30(bool a, bool b) = 0;              // mode list (1, 1)
    virtual void listbox_select_index_00a9c7c0(MultiModeWidget list, std::int32_t index) = 0;
    virtual void listbox_select_first_selectable_00a9c310() = 0;         // mode list
    virtual void input_owner_update_00a92c40(float seconds) = 0;         // 004BEC00 singleton
    virtual bool restore_options_e08700() = 0;
    virtual void open_options_005ec250() = 0;
    virtual void layout_options_005ebdd0() = 0;
    virtual bool session_present_f8a2fc() = 0;
    virtual bool session_virtual(std::uint16_t slot, std::int32_t arg) = 0;  // *(00F8A2FC) +170h/+180h/+198h
    virtual void xnet_qos_release_00a4d45e(std::uint32_t handle) = 0;
    virtual std::int32_t xnet_qos_service_lookup_00a4d458(std::uint32_t& handle) = 0;  // (0, 0, &handle)
    virtual void xnet_get_opt_00a4d452() = 0;                            // (6, buffer, &size = 8)
    // Update.
    virtual bool main_menu_child_blocked_60_4() = 0;                     // *(*(00E198AC)+60h)+4
    virtual bool action_004d92b0(std::int32_t action) = 0;
    virtual void back_005eca90() = 0;
    virtual void scroller_update_00683820() = 0;                         // screen+3Ch, arg 1
    virtual bool scroller_has_range_96() = 0;                            // screen+96h
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual std::int32_t option_list_selected_00a9c920() = 0;
    virtual void change_option_005ecba0(std::int32_t delta) = 0;
    virtual void poll_session_005eac60() = 0;
    virtual void cancel_session_00688c70() = 0;                          // *(00E198B4)
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;       // *(00E188A8)+1EF0h
    virtual void push_request_interface_logged_00688ad0(std::int32_t id) = 0;
    virtual float option_list_height_f4() = 0;                           // *(screen+A8h)+F4h
    // 00AA6750 position read and 00AA8240 resolved-position write on +B0h/+B4h.
    virtual std::array<float, 3> widget_position_00aa6750(MultiModeWidget widget) = 0;
    virtual void widget_set_position_00aa8240(MultiModeWidget widget, const std::array<float, 3>& position) = 0;
    virtual void profile_hints_00427190(std::int32_t hint) = 0;          // 004C1E90 singleton
};

// 005EC5B0: slot +18h of vtable 00CF2B44, __thiscall(screen), RET.
void multi_mode_selector_enter_005ec5b0(MultiModeSelectorState& screen, MultiModeSelectorHost& host);

// 005ECD80: slot +20h of vtable 00CF2B44, __thiscall(screen, float seconds), RET 4.
void multi_mode_selector_update_005ecd80(MultiModeSelectorState& screen, float seconds,
    MultiModeSelectorHost& host);

// ---------------------------------------------------------------------------
// Multiplayer game lobby enter (005E51D0)
// ---------------------------------------------------------------------------

inline constexpr std::string_view kLobbySkirmishTitlePrefix = "FE.main_skirmish_title|. - |";  // 00CF2554
inline constexpr std::uint32_t kLobbyModeOptionCategory = 2;  // 00575E80 key on the option registry
inline constexpr std::size_t kLobbyPlayerRows = 8;            // +250h..+26Ch
inline constexpr double kLobbyRowStepX = 0.0;                 // 00D7A258, used for X and Z
inline constexpr float kLobbyListScrollA = -0.0020833334F;    // 00CF254C -> list +12Ch
inline constexpr float kLobbyListScrollB = 0.0055555557F;     // 00CF2548 -> list +130h and +128h
inline constexpr float kLobbyListValue = 0.4875F;             // 00CF2550, 00A9CEB0's float
inline constexpr float kLobbyRowField30 = -0.011458334F;      // 00CF2534 -> row +30h
inline constexpr float kLobbyField2D0 = 100.0F;               // 00CF208C
inline constexpr float kLobbyNoseArtWidth = 0.13333334F;      // 00CF2318

// Widgets the lobby enter touches: screen+130h, +B0h, the player list (found
// as "Players_Listbox" under +14h and stored at +A8h), its "Player1_Group"
// row template, +94h, the three arrows +144h/+148h/+14Ch and the
// noseart_Group/New_5_Text text under +14h.
enum class LobbyWidget : std::uint8_t {
    Panel130, PanelB0, PlayerList, RowTemplate, Panel94, Arrow144, Arrow148, Arrow14C, NoseArtText
};

struct GuiPoint3 {
    float x;
    float y;
    float z;
};

struct MultiGameLobbyEnterState {
    std::int32_t field10{};          // +10h: 0 while entering, 1 at the end
    std::uint32_t field1c{};         // +1Ch, else +A4h, passed to 005D67E0
    std::uint32_t field_a4{};
    std::int32_t field278{};         // +278h
    bool field280{};
    bool field281{};
    bool spectating282{};            // +282h, copy of *(00E188A8)+2164h
    std::uint64_t mode_mask_2c0{};   // +2C0h, 005D9C50
    std::uint64_t mode_mask_2c8{};   // +2C8h, 005DF510
    float field2d0{};
    std::int32_t field2d4{};
};

struct MultiGameLobbyEnterHost {
    virtual ~MultiGameLobbyEnterHost() = default;
    virtual bool session_present_f8a2fc() = 0;
    virtual void session_clear_byte_75() = 0;                     // *(00F8A2FC)+75h = 0
    virtual bool interface_query_00687860() = 0;                  // *(00E198B4), (4, 0)
    virtual bool skirmish_218c() = 0;                             // *(00E188A8)+218Ch
    virtual std::int32_t global_e19564() = 0;
    // 00576B10 singleton +10h: 00575E80(&category, &value) then 005732C0, a C string.
    virtual std::string option_label_00575e80(std::uint32_t category, std::int32_t value) = 0;
    // 00518D60: ECX = 0Ah, EDX = 0, stack 0 and the title.
    virtual void set_frame_title_00518d60(std::string_view title) = 0;
    virtual void widget_virtual(LobbyWidget widget, std::uint16_t slot, std::int32_t arg) = 0;  // +34h/+60h
    virtual void widget_virtual_88(LobbyWidget widget, std::int32_t state) = 0;  // (state, 0, 1.0f)
    virtual void option_panel_005ce750() = 0;                     // *(00E198B4)+54h
    virtual void session_00771dd0() = 0;                          // game+1EF0h, (game+18C8h, 1, 1)
    virtual void bind_player_list() = 0;                          // 00AA7E00 "Players_Listbox" -> +A8h
    virtual void player_list_set_byte(std::uint16_t offset, std::uint8_t value) = 0;  // +149h, +14Ah
    virtual void player_list_set_float(std::uint16_t offset, float value) = 0;        // +128h..+134h
    virtual void player_list_00a9ceb0(float value, std::int32_t arg) = 0;
    virtual void bind_row_template() = 0;                         // "Player1_Group" in the list
    virtual float row_template_height_00aa6740() = 0;             // size +4
    virtual GuiPoint3 row_template_position_00aa67f0() = 0;
    // One cloned row (00AAB4C0(0) into +250h + 4*index).
    virtual void clone_row(std::size_t index) = 0;
    virtual void row_set_position_00aa7dc0(std::size_t index, GuiPoint3 position) = 0;
    virtual void row_set_byte_78(std::size_t index, bool value) = 0;
    virtual void row_set_floats_30_34(std::size_t index, float field30, float field34) = 0;
    // 00AA7E00 child by name, then 00ABBE50(text, localise 0).
    virtual void row_child_set_text(std::size_t index, std::string_view child, std::string_view text) = 0;
    virtual void player_list_append_row_00a9d750(std::size_t index) = 0;   // (row, 0, 0)
    virtual void row_set_widget_listener_00aa6bc0(std::size_t index) = 0;  // (screen+0Ch, 1)
    virtual void row_set_index_d8(std::size_t index, std::int32_t value) = 0;
    virtual void player_list_select_index_00a9c7c0(std::int32_t index) = 0;
    virtual void player_list_clear_listener_00a9ac40() = 0;
    virtual bool option_panel_byte_8c() = 0;                      // *(*(00E198B4)+54h)+8Ch
    virtual std::int32_t effective_game_mode_004bca50() = 0;
    virtual void set_global(std::uint32_t address, std::int32_t value) = 0;  // 00E19564/68/94, 00E08958
    virtual std::int32_t game_dword(std::uint32_t offset) = 0;    // *(00E188A8)+offset
    virtual void game_set_dword(std::uint32_t offset, std::uint32_t value) = 0;
    virtual void lobby_005e20a0() = 0;
    virtual void lobby_005e3580() = 0;
    virtual void lobby_005d5870(std::int32_t arg) = 0;
    virtual void lobby_005d67e0(std::uint32_t arg) = 0;
    virtual std::uint64_t mode_mask_005df510(std::int32_t mode) = 0;
    virtual std::uint64_t mode_mask_005d9c50(std::int32_t mode) = 0;
    virtual void session_virtual_174(std::int32_t arg) = 0;       // *(00F8A2FC)
    virtual void media_00a4c2d0() = 0;                            // 004C1710 singleton, 0 pushed
    virtual bool game_byte(std::uint32_t offset) = 0;
    virtual void set_e19698_byte4(bool value) = 0;
    virtual void lobby_005d6c10() = 0;
    virtual void set_help_variant_00549620(std::int32_t variant) = 0;  // *(00E1930C)
    virtual void nose_art_set_text_00abbe50(std::string_view text) = 0;
    virtual void nose_art_set_ellipsized_00abb000(std::string_view key, float width) = 0;  // flag 1
    virtual void lobby_005d5340() = 0;
    virtual void game_004bb3e0() = 0;
};

// 005E51D0: slot +18h of vtable 00CF23E4, __thiscall(screen), RET. EBX = 0
// throughout; EBP is 1 from 005E57E3.
void multi_game_lobby_enter_005e51d0(MultiGameLobbyEnterState& screen, MultiGameLobbyEnterHost& host);

// ---------------------------------------------------------------------------
// Multiplayer game lobby update (005E5B30)
// ---------------------------------------------------------------------------

inline constexpr double kLobbySessionRefreshPeriod = 100.0;  // 00D7A220, against +2D0h
inline constexpr std::size_t kLobbySettingsDwords = 14;      // +284h -> 00E1955C.. and game+2020h..
inline constexpr std::uint8_t kLobbyBackCommand = 0xA3;      // pushed as -5Dh

// The two option scrollers (+158h outside skirmish, +CCh in skirmish) and
// the one at +30h; each has floats +0Ch and +1Ch.
enum class LobbyScroller : std::uint8_t { Options158, SkirmishCC, Players30 };

// Widgets read for sizes or cleared: the scroller contents +134h/+B4h and
// the viewports +1B8h/+12Ch; +1B4h/+128h go to 005D0C60 with the contents.
enum class LobbyUpdateWidget : std::uint8_t { Content134, ContentB4, Viewport1B8, Viewport12C, Key1B4, Key128 };

// A player-slot record, *(game+18CCh + 4*slot): +09h, +0Ah, +0Ch (ready),
// +28h and the XUID at +80h.
struct LobbySlotRecord {
    bool byte9{};
    bool byte_a{};
    bool ready_c{};
    std::uint32_t field28{};
    std::uint64_t xuid80{};
};

struct MultiGameLobbyUpdateState {
    std::int32_t field10{};
    // +1Ch is the focused widget; the others are widget pointers compared
    // with it (005D67E0 stores its argument at +1Ch, 005D6833).
    std::uint32_t focus_1c{};
    std::uint32_t target_20{};
    std::uint32_t target_28{};   // nose art
    std::uint32_t target_8c{};   // settings
    std::uint32_t target_a4{};   // the player list (listbox at +A8h)
    std::uint32_t target_c4{};   // skirmish options
    std::uint32_t target_150{};  // options
    bool scroll_8a{};
    bool scroll_126{};
    bool scroll_1b2{};
    bool waiting_270{};
    bool leave_pending_271{};
    bool field280{};
    bool ready_281{};
    bool spectating_282{};
    std::array<std::uint32_t, kLobbySettingsDwords> settings_284{};
    std::int32_t scene_2bc{};
    std::uint64_t mode_mask_2c0{};
    std::int32_t field2f0{};
    std::int32_t field2f4{};
    std::int32_t field2f8{};
    std::int32_t field2fc{};
    float refresh_timer_2d0{};
    bool unlock_2dc{};
};

struct MultiGameLobbyUpdateHost {
    virtual ~MultiGameLobbyUpdateHost() = default;
    virtual bool interface_busy_e198ac() = 0;                     // *(*(00E198AC)+60h)+4
    virtual std::int32_t poll_leave_0076f400() = 0;               // ECX = game+1EF0h
    virtual void session_virtual_174(std::int32_t arg) = 0;       // *(00F8A2FC)
    virtual bool session_virtual_180() = 0;
    virtual bool session_virtual_198() = 0;
    virtual bool session_virtual_19c() = 0;
    virtual bool session_byte_4d() = 0;
    // Slot +17Ch(game+2020h, (int)(char)game+201Ch, (int)(char)game+201Dh,
    // game+2060h, +2064h, +2068h, +206Ch).
    virtual void session_publish_17c() = 0;
    virtual void interface_virtual_0c() = 0;                      // *(00E198B4) slot +0Ch
    virtual void game_004d7940() = 0;
    virtual std::int32_t game_dword(std::uint32_t offset) = 0;    // *(00E188A8)+offset
    virtual bool game_byte(std::uint32_t offset) = 0;
    virtual float game_float(std::uint32_t offset) = 0;
    virtual void metrics_00753810(std::int32_t event, std::int32_t arg) = 0;  // ECX = *(00E1AED4)
    virtual void option_panel_005cfb80() = 0;                     // ECX = *(00E198B4)+54h
    virtual bool option_panel_changed_005d0c60(LobbyUpdateWidget key, LobbyUpdateWidget content) = 0;
    virtual void option_panel_005d11f0(std::int32_t arg) = 0;
    virtual void option_panel_sub8_virtual_0c() = 0;              // ECX = *(00E198B4)+54h+8
    virtual bool option_panel_byte_64() = 0;
    virtual void widget_00aa83f0(LobbyUpdateWidget widget) = 0;
    virtual float widget_height_00aa6740(LobbyUpdateWidget widget) = 0;  // size +4
    virtual void scroller_set_range_006834a0(LobbyScroller scroller, float range) = 0;
    virtual void scroller_attach_content_00683790(LobbyScroller scroller, std::uint32_t content) = 0;
    virtual void scroller_sync_thumb_00683440(LobbyScroller scroller) = 0;
    virtual void scroller_update_00683820(LobbyScroller scroller) = 0;   // (1)
    virtual float scroller_float_0c(LobbyScroller scroller) = 0;
    virtual void scroller_set_float_1c(LobbyScroller scroller, float value) = 0;
    virtual bool settings_open_005d5490() = 0;                    // (+18h) slot +38h and (+228h)+85h
    virtual bool action_004d92b0(std::int32_t action) = 0;        // ECX = *(00E188A8)
    virtual void set_focus_005d67e0(std::uint32_t target) = 0;
    virtual void input_context_level_00a933f0(std::int32_t level, std::int32_t arg) = 0;  // 004BEC00 owner
    virtual void input_owner_update_00a92c40(float seconds) = 0;  // 004BEC00 owner
    virtual void lobby_005da210() = 0;
    virtual void lobby_005d8410() = 0;
    virtual void lobby_005d9f90() = 0;
    virtual void lobby_005d5870(std::int32_t arg) = 0;
    virtual bool lobby_005d71b0() = 0;
    virtual void lobby_005d62b0() = 0;
    virtual void lobby_005e1cf0(std::int32_t direction) = 0;
    virtual void lobby_005da530() = 0;
    virtual void lobby_005e3580() = 0;
    virtual void lobby_005d9f50() = 0;
    virtual bool lobby_005d5260() = 0;
    virtual bool lobby_005d4a10() = 0;
    virtual void lobby_005d9660(bool action52, bool action51) = 0;
    virtual void lobby_005e4fc0() = 0;
    virtual void lobby_005d73a0() = 0;
    virtual void lobby_005deca0() = 0;
    virtual std::uint64_t mode_mask_005d9c50(std::int32_t mode) = 0;
    // 005D5160(+2FCh) then 00427AE0 on its +4: a C string (00E19605 if null),
    // passed to 004C8950 on the game.
    virtual void select_named_entry_005d5160(std::int32_t index) = 0;
    // Both copies of the settings block: 00E1955C.. and game+2020h..
    virtual void store_settings(const std::array<std::uint32_t, kLobbySettingsDwords>& settings) = 0;
    virtual void set_game_mode_004bc890(std::uint32_t mode) = 0;  // (mode, 0)
    virtual void set_global(std::uint32_t address, std::int32_t value) = 0;  // 00E19594, 00E08958
    virtual std::int32_t global_e08958() = 0;
    virtual float global_e19598() = 0;
    virtual std::uint8_t global_e1960a() = 0;
    virtual void set_global_e1960a(std::uint8_t value) = 0;
    virtual void select_scene_record_004c6890(std::int32_t scene) = 0;
    virtual std::int32_t effective_game_mode_004bca50() = 0;
    virtual void clear_e19698_byte4() = 0;
    virtual void widget_listener_activate(std::uint32_t row) = 0;  // (screen+8) slot +4
    virtual std::uint32_t player_list_selected_row_00425e50() = 0;  // listbox +A8h
    virtual std::int32_t player_list_selected_index_00a9c920() = 0;
    virtual bool player_list_byte_85() = 0;
    // game+18CCh[index]; 0 when empty. Slot 0..7 of game+18ECh is the local one.
    virtual std::uint32_t slot_record(std::int32_t index) = 0;
    virtual LobbySlotRecord slot_record_fields(std::uint32_t record) = 0;
    virtual std::uint32_t local_slot_record_005d5130() = 0;       // ECX = game
    virtual bool local_slot_valid_005d51f0() = 0;
    virtual bool local_ready_005d5220() = 0;
    virtual void slot_record_toggle_005d8530(std::uint32_t record) = 0;
    virtual bool input_device_action_d3() = 0;                    // 004BA6D0(0, 0) slot +20h(0D3h)
    virtual void noseart_request_prepare() = 0;                   // *(00E198AC)+70h: +98h, +94h = 8
    virtual void push_request_interface_logged_00688ad0(std::int32_t id) = 0;
    virtual bool signed_into_live_004b44f0() = 0;                 // ECX = *(00F8ABE8)
    virtual void show_gamer_card(std::uint64_t xuid) = 0;         // 00A4D440(00A3EAC0(), xuid)
    virtual bool session_counts_mission_004b6260() = 0;           // ECX = game+1EF0h
    // 0054B530, ECX = *(00E1930C).
    virtual void rebuild_command_bar_0054b530(const std::array<MainMenuCommandArgument, 5>& arguments) = 0;
    virtual void profile_hints_00427190(std::int32_t hint) = 0;   // 004C1E90 singleton
    // +220h/+224h keep X and Z and take (+228h)+F4h as Y (00AA6750 / 00AA8240).
    virtual void align_rows_to_list_y() = 0;
};

// (multi_game_lobby_update_005e5b30 is declared below the main-menu block.)

// ---------------------------------------------------------------------------
// Multiplayer main menu enter (005E8B20)
// ---------------------------------------------------------------------------

inline constexpr std::array<std::string_view, 2> kMultiMenuRankedRows = {
    "FE.multi_ranked", "FE.multi_player"};                      // 00E08964
inline constexpr std::array<std::string_view, 3> kMultiMenuQuickRows = {
    "FE.multi_quick", "FE.multi_custom", "FE.multi_create"};    // 00E08974
inline constexpr std::size_t kPlayerRankCount = 20;             // FE.playerrank_01..20, 00E08E00
inline constexpr std::uint8_t kPlayerRankMax = 0x14;
inline constexpr double kMultiMenuRowMarginA = 0.005208333333333333;  // 00CF2958
inline constexpr double kMultiMenuRowMarginB = 0.019999999552965164;  // 00D7A2F8 (0.02f widened)
inline constexpr float kMultiMenuWidthFloor = 1.17549435e-38F;         // 00CF2960, FLT_MIN
inline constexpr double kMultiMenuBarInset = 0.00625;                  // 00CF2920
inline constexpr double kGuiTextWidthScale = 960.0;                    // 00CEC380

enum class MultiMenuWidget : std::uint8_t {
    RankedList20,   // +20h listbox
    QuickList24,    // +24h listbox
    AnchorD0,       // +D0h, its X bounds the row text
    NameLabel30,    // +30h
    NameText34,     // +34h
    Hidden38,       // +38h
    ScoreText3C,    // +3Ch
    RankText44,     // +44h
    Text48,         // +48h
    Text4C,         // +4Ch
    RankIcon50,     // +50h
    PreviousBar54,  // +54h
    CurrentBar58,   // +58h
    BarFrame5C,     // +5Ch
};

// One node of the list at *(screen+D8h)+64h: byte +76h marks an
// achievement-gated widget, slot +28h gives its key.
struct MultiMenuGatedWidget {
    bool gated{};
    std::string key;
};

struct MultiMainMenuEnterState {
    bool live_14{};                 // +14h, session slot +198h
    std::int32_t selected_f4{-1};   // +F4h, the remembered row of +24h
};

struct MultiMainMenuEnterHost {
    virtual ~MultiMainMenuEnterHost() = default;
    virtual bool session_virtual_198() = 0;                       // *(00F8A2FC)
    virtual bool session_byte_4c() = 0;
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;  // ECX = game+1EF0h
    virtual void set_frame_title_00518d60(std::string_view title) = 0;  // ECX = EDX = 0, stack 0
    virtual void frame_00518b90(std::int32_t arg) = 0;            // ECX = 2
    virtual float widget_x_00aa6750(MultiMenuWidget widget) = 0;  // position +0
    virtual GuiWidgetSize widget_size_00aa6740(MultiMenuWidget widget) = 0;
    virtual void widget_set_size_58(MultiMenuWidget widget, GuiWidgetSize size) = 0;
    virtual void widget_virtual(MultiMenuWidget widget, std::uint16_t slot, std::int32_t arg) = 0;  // +60h/+34h
    virtual void widget_set_state_88(MultiMenuWidget widget, std::int32_t state) = 0;  // (state, 0, 1.0f)
    virtual void list_set_byte_77(MultiMenuWidget list, bool value) = 0;
    virtual void list_set_listener_00a9ac40(MultiMenuWidget list) = 0;  // screen+8
    // "MainListbox_Text" under the list (00AA7E00, 1), cloned (00AAB4C0(0)),
    // appended with 00A9DA30(row, data, 0, 0); returns the row's handle.
    virtual void* list_append_row(MultiMenuWidget list, std::int32_t data) = 0;
    virtual void row_set_ellipsized_00abb000(void* row, std::string_view key, float width) = 0;  // flag 1
    virtual float row_text_width_114(void* row) = 0;
    virtual void list_00a9cf30(MultiMenuWidget list, std::int32_t a, std::int32_t b) = 0;
    virtual void list_00a9ceb0(MultiMenuWidget list, float value, std::int32_t arg) = 0;
    virtual void list_select_first_00a9c310(MultiMenuWidget list) = 0;
    virtual void list_select_index_00a9c7c0(MultiMenuWidget list, std::int32_t index) = 0;
    virtual bool live_enabled_account_00a3e520() = 0;             // ECX = *(00F8ABE8)
    virtual void set_text_localised(MultiMenuWidget widget, std::string_view key) = 0;  // 00ABBE50, 1
    virtual void set_text_source(MultiMenuWidget widget, std::string_view text, bool localise) = 0;  // 00ABAED0
    // The profile is *(00E188A8)+650h: name +50h when its length is nonzero,
    // else +3Ch; rank 00574050; score 0090BF50 on +64h; last score +30h;
    // achievement counters in the case-insensitive map +A0h.
    virtual std::string profile_name() = 0;
    virtual std::uint8_t profile_rank_00574050() = 0;
    virtual std::int32_t profile_score_0090bf50() = 0;
    virtual std::int32_t profile_last_score_30() = 0;
    virtual void profile_set_last_score_30(std::int32_t score) = 0;
    virtual std::int32_t profile_achievement_count(std::string_view key) = 0;  // 0 when absent
    // 0050FC30(&"RANK") on *(00E19900)+10h, then its vector +48h at index.
    virtual std::int32_t rank_threshold(std::uint32_t index) = 0;
    virtual std::vector<MultiMenuGatedWidget> gated_widgets() = 0;
    virtual void gated_widget_lock_icon_show(std::size_t index, bool show) = 0;  // "achievement_lock_Icon" +34h
};

// 005E8B20: slot +18h of vtable 00CF2708, __thiscall(screen), RET.
void multi_main_menu_enter_005e8b20(MultiMainMenuEnterState& screen, MultiMainMenuEnterHost& host);

// 005E5B30: slot +20h of vtable 00CF23E4, __thiscall(screen, float seconds), RET 4.
void multi_game_lobby_update_005e5b30(MultiGameLobbyUpdateState& screen, float seconds,
    MultiGameLobbyUpdateHost& host);

} // namespace bsp
#endif
