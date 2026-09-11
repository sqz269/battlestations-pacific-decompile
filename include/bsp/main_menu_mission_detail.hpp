#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "bsp/main_menu_screen.hpp"
#include "bsp/main_menu_screens.hpp"

// The main menu's mission-detail page, built by 0058C010.
//
// 0058C010 is the page builder the campaign arm of the main-menu screen calls
// once a mission has been picked out of a mission list. It is __thiscall with
// no stack argument (0058C035 `MOV ESI,ECX`), an aligned frame
// (0058C010..0058C016 `PUSH EBP / MOV EBP,ESP / AND ESP,-8`), one SEH frame,
// 1040 instructions, and a bare `RET` at 0058CE1F. `this` is the 578h main-menu
// screen of include/bsp/main_menu_screen.hpp, not an adjusted sub-object.
//
// Seven sites call it: 00598AD5 (Enter), 00599318 (the campaign arm of
// 00598B60), 005997C1 and 005998D0 (00599340), 00592722 (00592640), 0058F552,
// and 0059A444 (the per-frame update). Nothing calls it with arguments.
//
// The page it produces is the one the rebuilt executable needs next: it turns
// the mission-list selection into the briefing text, the preview movie, the
// streamed dialogue request, the map flag/point icons of the whole group, the
// two footer commands, and finally `00E08874 = 9` at 0058CAC3.
//
// Nothing here is a binary-compatible layout. Names are hypotheses.

namespace bsp {

// ---------------------------------------------------------------------------
// What the page is made of
// ---------------------------------------------------------------------------

// The three page roots 005861B0 loads with 00AA5840 and keeps as fields. Every
// 00AA7E00 lookup in 005861B0 runs against the first of them.
inline constexpr std::string_view kMainMenuWorldMapPage = "FE_worldmap_historical";  // +2F0h, 0058629C
inline constexpr std::string_view kMainMenuBriefingGridPage = "FE_briefing_grid";    // +244h, 0058641A
inline constexpr std::string_view kMainMenuBriefingPage = "FE_briefing";             // +248h, 005864A0

// One widget handle 005861B0 binds and 0058C010 then drives. `offset` is the
// byte offset in the 578h screen, `widget` the name passed to 00AA7E00, `bound`
// the address of the store in 005861B0.
struct MissionDetailWidget {
    std::uint16_t offset;
    std::string_view widget;
    std::uint32_t bound;
    std::string_view role;
};

inline constexpr std::array<MissionDetailWidget, 14> kMissionDetailWidgets{{
    {0x1B8, "Main_Listbox", 0x00587D3Bu,
        "0058CCF6 and 0058CD12 hide it, 0058CD26 clears it through 00A9BEC0, "
        "0058CD9E/0058CDAB set its two bytes at +149h/+14Ah, 0058CDF5 moves it "
        "to the screen vector at +558h..+560h and 0058CE05 finishes with "
        "00AA6BC0(this+40h, 0)"},
    {0x244, "FE_briefing_grid", 0x0058641Au, "0058C7D6 hides it"},
    {0x248, "FE_briefing", 0x005864A0u, "0058C7E5 hides it"},
    {0x2F0, "FE_worldmap_historical", 0x0058629Cu, "0058C6E0 shows it"},
    {0x310, "missions_US_Group", 0x0058775Eu, "0058C74B shows it when us && !dlc"},
    {0x314, "missions_JP_Group", 0x005877D4u, "0058C725 shows it when !us && !dlc"},
    {0x318, "missions_US_DLC_Group", 0x0058784Au, "0058C797 shows it when us && dlc"},
    {0x31C, "missions_JP_DLC_Group", 0x005878C0u, "0058C771 shows it when !us && dlc"},
    {0x320, "training_Group", 0x00587936u, "0058C7A5 always hides it on this page"},
    {0x324, "mission_pic_Group", 0x00586E8Bu, "0058C7B4 hides it"},
    {0x328, "bg_01_Icon", 0x005879ACu, "0058C6EF shows it"},
    {0x32C, "background_Icon", 0x00587A48u,
        "0058C04C drives its vtable +88h with (0, 0, 1.0f) before anything else, "
        "and 0058C6FE shows it"},
    {0x33C, "video_Movie", 0x00586963u,
        "0058C8A0 points it at the record's backgroundMovie and 0058C8AB starts it"},
    {0x344, "historical_Group", 0x005868EDu, "0058C7FB shows it"},
}};

// Widgets 0058C010 drives that 005861B0 binds outside the 00AA7E00 sweep or
// leaves for the page builder. +330h is cleared to null at 00587AC8 and filled
// only here; +3DCh and +41Ch are hidden at 0058CCE8 and 0058CD04 but no store
// into either was found in 005861B0, so their names are still open.
inline constexpr std::uint16_t kMissionDetailSelectedPointField = 0x330;   // 0058C190
inline constexpr std::uint16_t kMissionDetailTextClipField = 0x3B0;        // "test_Clipbox", 00586A4F
inline constexpr std::uint16_t kMissionDetailBriefingTextField = 0x3B8;    // "content_main_Text", 00586B4F
inline constexpr std::uint16_t kMissionDetailScrollerField = 0x354;        // 00586AF9, the +354h animator
inline constexpr std::uint16_t kMissionDetailUnnamedHideA = 0x3DC;         // 0058CCE8
inline constexpr std::uint16_t kMissionDetailUnnamedHideB = 0x41C;         // 0058CD04
inline constexpr std::uint16_t kMissionDetailListBoxPositionField = 0x558; // three floats, 0058CDB8
inline constexpr std::uint16_t kMissionDetailAudioSuppressField = 0x570;   // 0058C8B0

// ---------------------------------------------------------------------------
// Picking the group, 0058C092..0058C11C
// ---------------------------------------------------------------------------

// 0058C092 subtracts 1 from the published group at 00E194D8 and rejects
// anything above 3 (`CMP EAX,3 / JA 0058CE0A`), so only groups 1..4 build a
// page. Group 0, the training grounds, falls straight through to the epilogue:
// it never reaches the widget work and never sets 00E08874 to 9. The jump table
// is at 0058CE20.
bool mission_detail_group_is_supported(int published_group) noexcept;

// The five-way choice of include/bsp/main_menu_screen.hpp, reached here from
// the published group instead of from the page. 0058C0BE, 0058C0AA, 0058C0DC
// and 0058C0FD load +314h, +310h, +31Ch and +318h and set EDI to 0, 1, 3 and 4,
// the same point-list index kMissionGroupBindings uses.
MissionGroup mission_detail_group(int published_group) noexcept;

// The two campaign bytes the same four arms write. `us_campaign` is screen+564h
// and `dlc_campaign` is screen+565h.
struct MissionDetailCampaignFlags {
    bool us_campaign{};
    bool dlc_campaign{};
};

MissionDetailCampaignFlags mission_detail_campaign_flags(int published_group) noexcept;

// The inverse relation, used by the visibility block at 0058C70D..0058C7A5:
// exactly one of the four campaign groups is shown and training is always
// hidden. Index 0..4 is MissionGroup.
std::array<bool, 5> mission_detail_group_visibility(
    bool us_campaign, bool dlc_campaign) noexcept;

// ---------------------------------------------------------------------------
// The map icons, 0058C11C..0058C6DB
// ---------------------------------------------------------------------------

// Two icons exist per mission of the group, looked up by ordinal under the
// group widget that was just copied into +110h. The ordinal is one-based: the
// loop counter starts at 1 (0058C248) and 00E194DC is compared as `index + 1`
// (0058C474, 0058C617). The template widgets the layout ships with are bound
// separately at +114h/+118h and are not touched here.
inline constexpr std::string_view kMissionMapFlagPrefix = "mission_mapflag_";    // 00CEFB90
inline constexpr std::string_view kMissionMapPointPrefix = "mission_mappoint_";  // 00CEFBA4
inline constexpr std::string_view kMissionMapIconSuffix = "_Icon";               // 00CED1BC
inline constexpr std::string_view kMissionMapFlagTemplate =
    "mission_mapflag_template_Icon";  // +118h, 00587084
inline constexpr std::string_view kMissionMapPointTemplate =
    "mission_mappoint_template_Icon";  // +114h, 00587000

std::string mission_map_flag_widget_name(int one_based_ordinal);
std::string mission_map_point_widget_name(int one_based_ordinal);

// The flag icon's float argument to vtable +4Ch. 0058C482 loads 00D7A24C for
// the selected mission and 0058C48C loads 00CE7804 for every other.
inline constexpr float kMissionMapFlagSelectedLevel = 1.0f;    // 00D7A24C
inline constexpr float kMissionMapFlagUnselectedLevel = 0.4f;  // 00CE7804

float mission_map_flag_level(bool selected) noexcept;

// The point icon's first argument to vtable +88h, a two-bit code built at
// 0058C62E..0058C65D. Bit 0 is "this is the selected mission", bit 1 is the
// record's `sideMission` byte at record+360h. The selected arm uses
// `LEA EAX,[EAX+EAX+1]` and the unselected arm `NEG CL / SBB ECX,ECX / AND 2`.
int mission_map_point_state(bool selected, bool side_mission) noexcept;

// The point icon's remaining two arguments are the literals 0 and 1.0f, pushed
// at 0058C62A and 0058C615.
inline constexpr int kMissionMapPointStateArg = 0;
inline constexpr float kMissionMapPointBlend = 1.0f;

// ---------------------------------------------------------------------------
// The briefing text and its scroller, 0058C80A..0058C88D
// ---------------------------------------------------------------------------

// The scroll range at 0058C877. The addend is a double at 00CEED60 whose bytes
// are 17 6C C1 16 6C C1 86 3F, exactly 1/90. `text_height` comes from
// 00AB6BD0 on the text widget (an x87 double) and `clip_height` is the second
// float of the 00AA6740 size pair on "test_Clipbox" at +3B0h.
inline constexpr double kMissionDetailScrollPadding = 1.0 / 90.0;  // 00CEED60

double mission_detail_scroll_range(double text_height, float clip_height) noexcept;

// ---------------------------------------------------------------------------
// The streamed dialogue request, 0058C8B0..0058CAB5
// ---------------------------------------------------------------------------

// Reached only when screen+570h is zero. 0058C8B0..0058C8C0 spells that as
// `NEG ECX / SBB ECX,ECX / TEST ECX,0xE18B5C / JNZ`; ECX is 0 or -1 at that
// point, so the mask is immaterial and the test is `screen+570h == 0`.
inline constexpr std::string_view kStreamedDialogRoot = "sound/messages/";      // 00CEFBB8
inline constexpr std::string_view kStreamedDialogFolder = "/streamed_dialogs/"; // 00CEFBC8

// The path built by the three 004261A0 concatenations at 0058C971, 0058C989 and
// 0058C9AA, in that order. `language` is whatever 008D57A0 returns for the
// object at 00F88980; `voice_key` is the record's `backgroundVoice` at
// record+48h.
std::string streamed_dialog_path(std::string_view language, std::string_view voice_key);

// 0058C9BE pushes 00588460 as the completion handler for both
// BSP_FileStoreFactory_GetOrCreate and BSP_FileStore_RequestFile.
inline constexpr std::uint32_t kStreamedDialogCallback = 0x00588460;

// ---------------------------------------------------------------------------
// The footer commands, 0058CABA..0058CC1F
// ---------------------------------------------------------------------------

// 0058CC1F calls 0054B530 on the singleton at 00E1930C with fifteen stack
// dwords (`RET 3Ch` at 0054C045). Two of them are command ids and two are
// labels; the other eleven are empty strings and small integers whose grouping
// is not settled here. The ids are stored as negative bytes.
inline constexpr int kMissionDetailContinueCommand = -0x5E;  // 0058CC15
inline constexpr int kMissionDetailBackCommand = -0x5D;      // 0058CBF7
inline constexpr std::string_view kMissionDetailContinueLabel = "globals.continue";  // 00CEB460
inline constexpr std::string_view kMissionDetailBackLabel = "globals.back";          // 00CEB3A8

struct MissionDetailCommand {
    int id;
    std::string_view label;
};

inline constexpr std::array<MissionDetailCommand, 2> kMissionDetailCommands{{
    {kMissionDetailContinueCommand, kMissionDetailContinueLabel},
    {kMissionDetailBackCommand, kMissionDetailBackLabel},
}};

// The literal argument vector, left to right, with -1 standing for "the empty
// string at 00CE3A0C". Recorded because the grouping of 0054B530 is open.
inline constexpr std::array<int, 15> kMissionDetailCommandArguments{{
    -0x5E, -1, 0, 0, -1, 1, 0, -1, 1, -0x5D, -1, 2, 0, -1, 1,
}};

// 0058CD73 then calls 0054A0C0 on the same singleton with one more empty
// string, which clears the help line the previous page left behind.

// ---------------------------------------------------------------------------
// The sequence
// ---------------------------------------------------------------------------

// One pure virtual per native call site. The host owns the widgets, the global
// page word and the mission tables; the routine owns the order.
struct MissionDetailHost {
    MissionDetailHost() = default;
    MissionDetailHost(const MissionDetailHost&) = delete;
    MissionDetailHost& operator=(const MissionDetailHost&) = delete;
    virtual ~MissionDetailHost() = default;

    // 0058C037 then 0058C047: 00518D60 with ECX = 10 and the record's `name` at
    // record+8h. The same shape 00597870 uses on every mission-list page.
    virtual void request_page_audio(std::string_view mission_name) = 0;

    // 0058C04C..0058C064: [this+32Ch]->vtable+88h(0, 0, 1.0f).
    virtual void set_background_icon_state(int state, int reserved, float blend) = 0;

    // 0058C066..0058C071 and 0058C07A..0058C08D: the mission-tree screen at
    // [00E198AC]+5Ch supplies the group at +10h and the index of its selected
    // record, and both are republished to 00E194D8 and 00E194DC.
    virtual int read_tree_selected_group() = 0;
    virtual int read_tree_selected_mission_index() = 0;
    virtual void publish_selection(int group, int mission_index) = 0;

    // 0058C0AA..0058C116: the group handle copied into +110h and the two
    // campaign bytes.
    virtual void set_active_group_widget(MissionGroup group) = 0;
    virtual void set_campaign_flags(MissionDetailCampaignFlags flags) = 0;

    // 0058C18B: 00AA7E00 on the widget now in +110h, recursive flag 0, then the
    // store into +330h at 0058C190.
    virtual void bind_selected_map_point(std::string_view widget_name) = 0;

    // 0058C238..0058C26F: the point list at +134h + index*10h. The count is the
    // byte extent divided by 0Ch, so the elements are three floats.
    virtual std::size_t map_point_list_size(MissionGroup group) = 0;
    virtual void append_map_point(MissionGroup group, std::array<float, 3> point) = 0;

    // The four fields of the selected record that this page consumes, in the
    // schema of docs/MISSION_TREE_LUA_READER.md. Each one is fetched with its
    // own 005806A0 call, at 0058C037, 0058C80C/0058C81E, 0058C88F and 0058C99A.
    virtual std::string_view selected_mission_name() = 0;           // record+08h, `name`
    virtual std::string_view selected_background_key() = 0;         // record+30h, `background`
    virtual std::string_view selected_background_movie_key() = 0;   // record+40h, `backgroundMovie`
    virtual std::string_view selected_background_voice_key() = 0;   // record+48h, `backgroundVoice`

    // 0058C274..0058C6DB: the group's mission vector. 00580650 returns the same
    // group record the tree screen holds; the records are 434h bytes apart
    // (0058C6C6).
    virtual std::size_t group_mission_count() = 0;
    virtual bool mission_is_side_mission(std::size_t mission) = 0;

    // 0058C301: 005C2F70 on the record decides whether the flag icon is shown.
    virtual bool map_flag_visible(std::size_t mission) = 0;

    // 0058C3C5 and 0058C565: 00AA7E00 with the recursive flag 1.
    virtual void set_map_flag(std::string_view widget_name, bool visible, float level) = 0;
    virtual void set_map_point(std::string_view widget_name, int state, int reserved,
        float blend) = 0;

    // 0058C673: 00AA6750 on the point icon that was just configured.
    virtual std::array<float, 3> map_point_position(std::string_view widget_name) = 0;

    // 0058C67A..0058C6AC subtract the backdrop position at +124h..+12Ch.
    virtual std::array<float, 3> backdrop_position() = 0;

    // 0058C6E0..0058C7F2: every visibility call of the block, in source order.
    virtual void set_widget_visible(std::uint16_t field, bool visible) = 0;

    // 0058C7F4 and 0058C7FB.
    virtual void commit_page_state() = 0;

    // 0058C80C..0058C83F: the record's `background` goes to the text widget at
    // +3B8h through 00ABAED0, or the empty string through 00ABBE50 when the
    // field is null (0058C816 tests the first dword of the string object).
    virtual void set_briefing_text(std::string_view background_key) = 0;

    // 0058C84E, 0058C853..0058C888.
    virtual void reset_scroller() = 0;
    virtual double briefing_text_height() = 0;
    virtual float text_clip_height() = 0;
    virtual void set_scroll_range(double range) = 0;

    // 0058C8A0 and 0058C8AB.
    virtual void start_preview_movie(std::string_view movie_key) = 0;

    // 0058C8B0 and the block it guards.
    virtual bool page_audio_suppressed() = 0;
    virtual std::string_view audio_language_folder() = 0;
    virtual void request_streamed_dialog(std::string_view path, std::uint32_t callback) = 0;

    // 0058CAC3.
    virtual void set_page(MainMenuPage page) = 0;

    // 0058CC1F and 0058CD73.
    virtual void set_footer_commands(const std::array<MissionDetailCommand, 2>& commands) = 0;
    virtual void clear_help_line() = 0;

    // 0058CD26, 0058CD9E, 0058CDAB.
    virtual void clear_list_box() = 0;
    virtual void set_list_box_flags(bool a, bool b) = 0;

    // 0058CDB8..0058CE05. The vector is read straight out of +558h..+560h with
    // 0.0f added to each component, which is what the FLDZ/FADD pair does.
    virtual std::array<float, 3> list_box_position() = 0;
    virtual void move_list_box(std::array<float, 3> position) = 0;
    virtual void finish_list_box() = 0;
};

struct MissionDetailOutcome {
    bool built{};                // false for the training group
    int published_group{};       // 00E194D8
    int published_mission{};     // 00E194DC
    MissionGroup group{MissionGroup::Ijn};
    MissionDetailCampaignFlags flags{};
    std::size_t missions_visited{};
    bool filled_point_list{};    // the +134h vector was empty on entry
    bool requested_dialog{};
    MainMenuPage page{MainMenuPage::MissionDetail};
};

// 0058C010. __thiscall(MainMenuScreen* this), no stack argument, bare RET.
MissionDetailOutcome build_mission_detail_page_0058c010(MissionDetailHost& host);

}  // namespace bsp
