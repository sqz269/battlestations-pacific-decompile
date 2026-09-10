#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/frontend_managers.hpp"
#include "bsp/main_menu_screens.hpp"

// The three screens the options manager at 00E198B8 constructs into +40h, +44h
// and +48h, and the settings model the first of them drives.
//
// docs/FRONTEND_MANAGERS.md establishes the manager: Init 006898C0 opens the
// GVOptions block, builds the three objects and calls each one's virtual +10h.
// docs/OPTIONS_MENU_SCREENS.md is the evidence for everything below.
//
// The screens share the 004F7180 hierarchy of docs/GAME_FRONTEND_STATES.md, so
// `bsp::FrontEndScreenSlot` from main_menu_screens.hpp names the ten vtable
// slots and is reused here rather than redeclared.
//
// Nothing here is a binary-compatible layout. Every offset quoted is one a
// constructor or a virtual demonstrably touches; every other byte is
// unrecovered.

namespace bsp {

// ---------------------------------------------------------------------------
// Interface ids
// ---------------------------------------------------------------------------

// kInterfaceOptions (0Ch) already lives in frontend_managers.hpp. The manager's
// ApplyPendingInterface override 00689820 maps 0Ch..0Eh onto screen ids
// 0Dh..0Fh, so these two complete the set.
inline constexpr int kInterfaceControlLayout = 0x0D; // 005F7B00, 005F7D28, 005F9358
inline constexpr int kInterfaceControls = 0x0E;      // 005F7CE7

// ---------------------------------------------------------------------------
// The three classes
// ---------------------------------------------------------------------------

// One recovered class per manager slot. `screen_id` is the constant the slot
// +00h leaf returns; it is also the registry index 004F71D0 uses and the id the
// manager's interface map produces.
struct OptionsScreenClass {
    std::uint16_t manager_field_offset; // 00E198B8 + this
    std::uint32_t constructor;
    std::uint32_t object_size;
    std::uint32_t primary_vtable;       // stored at +00h
    std::uint32_t secondary_vtable_08;  // stored at +08h
    std::uint32_t secondary_vtable_0c;  // stored at +0Ch
    int screen_id;                      // slot +00h leaf
    int interface_id;                   // the id 00689820 maps to screen_id
    std::uint32_t id_leaf;              // slot +00h
    std::uint32_t destroy_virtual;      // slot +0Ch
    std::uint32_t register_virtual;     // slot +10h
    std::uint32_t bind_virtual;         // slot +14h
    std::uint32_t enter_virtual;        // slot +18h
    std::uint32_t exit_virtual;         // slot +1Ch
    std::uint32_t update_virtual;       // slot +20h
    std::uint32_t dependencies_virtual; // slot +24h
    std::string_view list_page;         // the first page the register virtual binds
    std::string_view layout_page;       // the second page it binds
};

inline constexpr std::array<OptionsScreenClass, 3> kOptionsScreens{{
    // +40h. The options screen proper: one list that walks eight pages of
    // settings. The only one of the three that edits anything.
    {0x40, 0x005F6030u, 0x270u, 0x00CF3974u, 0x00CF3960u, 0x00CF3940u, 0x0D,
        kInterfaceOptions, 0x005F1CA0u, 0x005F3B30u, 0x005F0250u, 0x005F1CC0u,
        0x005F5BD0u, 0x005F7020u, 0x005F7310u, 0x005F6000u, "_Options",
        "FE_options"},
    // +44h. The gamepad layout reference. 00527C80 fills it in before the
    // options screen pushes interface 0Dh, and its update pushes 0Ch back.
    {0x44, 0x00527D00u, 0x030u, 0x00CECF48u, 0x00CECF34u, 0x00CECF14u, 0x0E,
        kInterfaceControlLayout, 0x00527D30u, 0x00527E40u, 0x00527E60u,
        0x00527F90u, 0x00528240u, 0x00528920u, 0x005289B0u, 0x00528A60u,
        "FE_controls_X360_listbox", "FE_controls_X360"},
    // +48h. The keyboard and mouse screen. Its constructor chains 00683610,
    // the front-end screen animation block owned by another packet. Its enter
    // and update virtuals were not read, so the binding model is unrecovered.
    {0x48, 0x00555770u, 0x250u, 0x00CEE524u, 0x00CEE510u, 0x00CEE4F0u, 0x0F,
        kInterfaceControls, 0x00555800u, 0x00555C40u, 0x00555C60u, 0x00555DC0u,
        0x0055FA80u, 0x00551B90u, 0x00560430u, 0x00558680u,
        "FE_controls_PC_listbox", "FE_controls_PC"},
}};

const OptionsScreenClass* options_screen_by_field_offset(int offset) noexcept;
const OptionsScreenClass* options_screen_by_id(int screen_id) noexcept;
const OptionsScreenClass* options_screen_by_interface(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// The page state, 00E19650
// ---------------------------------------------------------------------------

// Which page the options screen is showing. Not the interface id: the interface
// stays kInterfaceOptions throughout. 00588A80 and 005D43C0 store 0 before the
// screen is raised, so it always opens on its own list.
enum class OptionsPage : int {
    MainList = 0,          // 00588AEF, 005D44A2
    Game = 1,              // 005F5911
    Audio = 2,             // 005F5A13
    Video = 3,             // 005F41F5
    Control = 4,           // 005F4113
    Unused5 = 5,           // never written
    Unused6 = 6,           // never written
    DownloadedContent = 7, // 005F3BAC
};

// Where a page's rows come from. `builder` stores the page number, assigns the
// title and calls the page builder 005F24D0 with
// (title, labels, label_count, value_rows, help_table, 0). `per_frame` rewrites
// the value column every update through 005F0800.
struct OptionsPageInfo {
    OptionsPage page;
    std::string_view title;
    std::uint32_t builder;      // 0 when nothing builds this page
    std::uint32_t per_frame;    // 0 when the page has no per-frame refresh
    std::uint32_t label_table;  // 0 when the page has no fixed label array
    std::uint16_t label_count;
    std::uint16_t value_rows;
    std::uint32_t help_table;
};

inline constexpr std::array<OptionsPageInfo, 8> kOptionsPages{{
    {OptionsPage::MainList, "FE.opt_title_main", 0x005F54F0u, 0u, 0x00E08A48u, 7, 0, 0u},
    {OptionsPage::Game, "FE.opt_title_game", 0x005F58F0u, 0x005F4C90u, 0x00E08BE0u, 12, 9, 0x00CF3874u},
    {OptionsPage::Audio, "FE.opt_title_audio", 0x005F59F0u, 0x005F4FA0u, 0x00E08AC0u, 7, 4, 0x00CF3808u},
    {OptionsPage::Video, "FE.opt_title_video", 0x005F41E0u, 0x005F2F20u, 0x00E08AF8u, 16, 13, 0x00CF3820u},
    {OptionsPage::Control, "FE.opt_title_control", 0x005F40F0u, 0x005F34D0u, 0x00E08B78u, 8, 6, 0x00CF385Cu},
    {OptionsPage::Unused5, {}, 0u, 0u, 0u, 0, 0, 0u},
    {OptionsPage::Unused6, {}, 0u, 0u, 0u, 0, 0, 0u},
    {OptionsPage::DownloadedContent, "FE.opt_title_dlc", 0x005F3B50u, 0x005F2D20u, 0u, 0, 0, 0u},
}};

const OptionsPageInfo* options_page_info(OptionsPage page) noexcept;

// ---------------------------------------------------------------------------
// The main list, page 0
// ---------------------------------------------------------------------------

// What activating one of the seven rows of 00E08A48 does. 005F7C40 switches on
// the selected row; the leading '^' on every label is the marker for a row that
// opens something rather than editing a value.
enum class OptionsMainListAction {
    OpenPage,          // rows 0, 1, 2 and 6
    PushInterface,     // rows 3 and 4
    OpenClanText,      // row 5, only when 00585B40 and 00585810 both agree
};

struct OptionsMainListRow {
    int row;
    std::string_view label;
    OptionsMainListAction action;
    OptionsPage page;        // meaningful for OpenPage
    int interface_id;        // meaningful for PushInterface
    bool sends_layout_payload; // row 4 calls 00527C80 first
};

inline constexpr std::size_t kOptionsMainListRowCount = 7;

inline constexpr std::array<OptionsMainListRow, kOptionsMainListRowCount> kOptionsMainList{{
    {0, "^FE.opt_game", OptionsMainListAction::OpenPage, OptionsPage::Game, 0, false},
    {1, "^FE.opt_audio", OptionsMainListAction::OpenPage, OptionsPage::Audio, 0, false},
    {2, "^FE.opt_video", OptionsMainListAction::OpenPage, OptionsPage::Video, 0, false},
    {3, "^FE.opt_controls", OptionsMainListAction::PushInterface, OptionsPage::MainList,
        kInterfaceControls, false},
    {4, "^FE.opt_controllayout", OptionsMainListAction::PushInterface, OptionsPage::MainList,
        kInterfaceControlLayout, true},
    {5, "^FE.opt_clantext", OptionsMainListAction::OpenClanText, OptionsPage::MainList, 0, false},
    {6, "^FE.opt_downloaded_content", OptionsMainListAction::OpenPage,
        OptionsPage::DownloadedContent, 0, false},
}};

// ---------------------------------------------------------------------------
// The settings model
// ---------------------------------------------------------------------------

// How 005F42D0 edits a row.
enum class OptionsValueKind {
    Toggle,      // the byte is set to !byte
    Enumerated,  // 005EF4D0 steps it modulo a count
    Ranged,      // 004155B0 steps it between two floats
    Indexed,     // 005EF4D0 steps an index and copies a record out of a table
};

// When the edit reaches the running game.
enum class OptionsApplyTiming {
    OnCommit,     // nothing happens until reset or apply-and-save
    Immediately,  // the edit itself calls into a live system
};

struct OptionsSetting {
    OptionsPage page;
    int row;                   // the list row, and the index into the label table
    std::string_view label;
    std::uint16_t field_offset; // into the 270h screen object
    OptionsValueKind kind;
    OptionsApplyTiming timing;
    std::string_view value_table; // the strings the value column shows, when fixed
};

inline constexpr std::size_t kOptionsSettingCount = 32;

// Every editable row of pages 1..4, in the order 005F42D0 tests them. Page 7's
// rows are content entries walked out of the screen's own array at +170h, so
// they are not in this table.
inline constexpr std::array<OptionsSetting, kOptionsSettingCount> kOptionsSettings{{
    // Page 1, game. Value strings come from 00E089F4 (disabled/enabled) and
    // 00E08A2C (none/basic/full) except rows 0, 1 and 6.
    {OptionsPage::Game, 0, "FE.opt_game_lang", 0x0DC, OptionsValueKind::Enumerated,
        OptionsApplyTiming::OnCommit, "00E08A2C is not used here; 008D48C0 names the language"},
    {OptionsPage::Game, 1, "FE.opt_game_impmetric", 0x0E0, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "FE.opt_game_metric / FE.opt_game_imperial"},
    {OptionsPage::Game, 2, "FE.opt_game_subtitles", 0x0E1, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Game, 3, "FE.opt_game_hints", 0x0E4, OptionsValueKind::Enumerated,
        OptionsApplyTiming::OnCommit, "00E08A2C"},
    {OptionsPage::Game, 4, "FE.opt_game_camerashake", 0x0E8, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Game, 5, "FE.opt_vid_camdrops", 0x154, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Game, 6, "FE.opt_game_markeralpha", 0x158, OptionsValueKind::Ranged,
        OptionsApplyTiming::OnCommit, "drawn by 005F3750"},
    {OptionsPage::Game, 7, "FE.opt_game_targetindicator", 0x122, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Game, 8, "FE.opt_game_cockpitmode", 0x18A, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    // Page 2, audio. Every keystroke calls 008D5430, which is what makes the
    // preview audible; rows 2 and 3 also replay a sample.
    {OptionsPage::Audio, 0, "FE.opt_au_mastervol", 0x0F8, OptionsValueKind::Ranged,
        OptionsApplyTiming::Immediately, {}},
    {OptionsPage::Audio, 1, "FE.opt_au_musicvol", 0x100, OptionsValueKind::Ranged,
        OptionsApplyTiming::Immediately, {}},
    {OptionsPage::Audio, 2, "FE.opt_au_speechvol", 0x108, OptionsValueKind::Ranged,
        OptionsApplyTiming::Immediately, {}},
    {OptionsPage::Audio, 3, "FE.opt_au_effectvol", 0x104, OptionsValueKind::Ranged,
        OptionsApplyTiming::Immediately, {}},
    // Page 3, video.
    {OptionsPage::Video, 0, "FE.opt_vid_res", 0x150, OptionsValueKind::Indexed,
        OptionsApplyTiming::OnCommit, "00F8895C records, 8 bytes each"},
    {OptionsPage::Video, 1, "FE.opt_vid_fullscreen", 0x0F6, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Video, 2, "FE.opt_vid_antial", 0x134, OptionsValueKind::Indexed,
        OptionsApplyTiming::OnCommit, "00F88968 sample counts"},
    {OptionsPage::Video, 3, "FE.opt_vid_vsynch", 0x138, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Video, 4, "FE.opt_vid_gamma", 0x13C, OptionsValueKind::Ranged,
        OptionsApplyTiming::OnCommit, {}},
    {OptionsPage::Video, 5, "FE.opt_vid_texd", 0x140, OptionsValueKind::Enumerated,
        OptionsApplyTiming::OnCommit, "00E08A2C"},
    {OptionsPage::Video, 6, "FE.opt_vid_objd", 0x12C, OptionsValueKind::Enumerated,
        OptionsApplyTiming::OnCommit, "00E08A2C"},
    {OptionsPage::Video, 7, "FE.opt_vid_shadows", 0x15C, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Video, 8, "FE.opt_vid_oceanref", 0x144, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Video, 9, "FE.opt_vid_clouds", 0x14C, OptionsValueKind::Toggle,
        OptionsApplyTiming::Immediately, "00E089F4"},
    {OptionsPage::Video, 10, "FE.opt_vid_foliage", 0x15E, OptionsValueKind::Toggle,
        OptionsApplyTiming::Immediately, "00E089F4"},
    {OptionsPage::Video, 11, "FE.opt_vid_motionblur", 0x165, OptionsValueKind::Toggle,
        OptionsApplyTiming::Immediately, "00E089F4"},
    // The step count is 3, not 4: the row cycles over off, cutscenes and always
    // and never reaches FE.opt_oldfilmeffect_alwaysfull.
    {OptionsPage::Video, 12, "FE.opt_vid_oldfilmeffect", 0x168, OptionsValueKind::Enumerated,
        OptionsApplyTiming::Immediately, "00E08A2C + 0Ch"},
    // Page 4, control. Rows 1..4 are the four bytes 00527C80 forwards to the
    // gamepad layout screen, in the push order 118h, 119h, 11Ah, 11Bh.
    {OptionsPage::Control, 0, "FE.opt_x360_comp", 0x188, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Control, 1, "FE.opt_ctrl_vib", 0x118, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Control, 2, "FE.opt_ctrl_icamy", 0x11A, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Control, 3, "FE.opt_ctrl_iply", 0x11B, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Control, 4, "FE.opt_ctrl_swapsticks", 0x119, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
    {OptionsPage::Control, 5, "FE.opt_ctrl_swapmapsticks", 0x11C, OptionsValueKind::Toggle,
        OptionsApplyTiming::OnCommit, "00E089F4"},
}};

const OptionsSetting* options_setting(OptionsPage page, int row) noexcept;

// The two fixed value tables. 00E089F4 is the boolean pair; 00E08A2C is the
// quality run followed, at offset 0Ch, by the four old-film values.
inline constexpr std::array<std::string_view, 2> kOptionsBooleanValues{
    "FE.opt_disabled", // 00CF3778
    "FE.opt_enabled",  // 00CF3768
};
inline constexpr std::array<std::string_view, 3> kOptionsQualityValues{
    "globals.none",  // 00CF36F8
    "globals.basic", // 00CF36E8
    "globals.full",  // 00CF36D8
};
inline constexpr std::array<std::string_view, 4> kOptionsFilmEffectValues{
    "FE.opt_oldfilmeffect_off",       // 00CF36BC
    "FE.opt_oldfilmeffect_cutscenes", // 00CF369C
    "FE.opt_oldfilmeffect_always",    // 00CF3680
    "FE.opt_oldfilmeffect_alwaysfull",// 00CF3660
};

// The three command rows every page but Control carries, and the extra one
// Control has instead of reset and cancel.
inline constexpr std::string_view kOptionsApplyAndSaveLabel = "FE.opt_applyandsave"; // 00CF3494
inline constexpr std::string_view kOptionsResetLabel = "FE.opt_reset";               // 00CF3484
inline constexpr std::string_view kOptionsCancelLabel = "FE.opt_cancel";             // 00CEE874
inline constexpr std::string_view kOptionsLayoutLabel = "FE.opt_ctrl_layout";        // 00CF3570

// ---------------------------------------------------------------------------
// List commands and input actions
// ---------------------------------------------------------------------------

// The command character 005F8960 reads out of the activated row at event+0F0h,
// defaulting to the byte at 00E19658. They are signed comparisons in the
// original, so A7h reads as -59h there.
enum class OptionsCommand : unsigned char {
    Reset = 0xA7,        // FE.opt_reset
    ApplyAndSave = 0xA2, // FE.opt_applyandsave
    Cancel = 0xA3,       // FE.opt_cancel
    Layout = 0xA5,       // FE.opt_ctrl_layout, page 4 only
};

// Every action below goes through BSP_FrontEnd_ActionEdgeOrRepeat (004D92B0)
// with ECX set to the game at 00E188A8, so each obeys the edge-plus-repeat rule
// of docs/MAIN_MENU_SCREEN_UPDATE.md.
inline constexpr int kOptionsActionBack = 0x4B;        // 005F7859 and 005F79E4
inline constexpr int kOptionsActionDecrease = 0x4C;    // 005F7B16
inline constexpr int kOptionsActionIncrease = 0x4D;    // 005F7B29
inline constexpr int kOptionsActionViewLayout = 0x4F;  // 005F7AA7
inline constexpr int kOptionsActionReset = 0x50;       // 005F7A2F

// What the back action does on a given page. The byte table at 005F7C20 is
// 00 01 01 01 01 02 02 01 over pages 0..7 and selects between these three.
enum class OptionsBackTarget {
    LeaveScreen,   // 005F0A80
    ReturnToList,  // 005F5E60
    Ignored,       // 005F7A2F, pages 5 and 6
};

OptionsBackTarget options_back_target(OptionsPage page) noexcept;

// The two state requests 005F0A80 enqueues when no multiplayer menu manager
// exists, in the order it pushes them.
inline constexpr std::array<int, 2> kOptionsLeaveStateRequests{0x16, 0x04};

// ---------------------------------------------------------------------------
// The mutable screen state
// ---------------------------------------------------------------------------

// The flags of the 270h object that the navigation and apply sequences read and
// write. Volume and video values live behind the host, because the original
// keeps them in the same object and the interesting behaviour is the sequence,
// not the storage.
struct OptionsScreenState {
    OptionsPage page = OptionsPage::MainList; // 00E19650, a global in the original
    int selected_main_row = 0;                // +A8h
    bool value_changed = false;               // +250h
    bool rebuild_page = false;                // +251h
    bool save_target_available = false;       // +252h
    bool gamepad_ui = false;                  // +25Ch, snapshot of 00E188A8+61Fh
    float transition_seconds = 0.0f;          // +260h
    bool write_pending = false;               // +264h
    bool read_pending = false;                // +265h
    float storage_seconds = 0.0f;             // +268h
};

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

// One method per native call site the recovered sequences reach. Address
// comments name the routine each method stands for.
struct OptionsScreenHost {
    virtual ~OptionsScreenHost() = default;

    // Input and list queries.
    virtual bool action_fired(int action) = 0;   // 004D92B0(00E188A8, action)
    virtual int focused_row() = 0;               // 00A9C920, negative when none
    virtual int selected_row() = 0;              // 00A9C990
    virtual bool focused_row_editable() = 0;     // byte +85h of the widget at +88h
    virtual bool gamepad_ui_flag() = 0;          // 00E188A8 + 61Fh
    virtual bool gamepad_present() = 0;          // 00F88A30

    // Editing one row.
    // byte = !byte. Returns the new value, which is what the three live video
    // rows forward to the renderer.
    virtual bool toggle(std::uint16_t field_offset) = 0;
    virtual void step_enumerated(std::uint16_t field_offset, int count) = 0; // 005EF4D0
    virtual void step_ranged(std::uint16_t field_offset, int step) = 0;  // 004155B0
    virtual void step_resolution(int step) = 0;   // 005EF4D0(00F88960) then 00F8895C
    virtual void step_antialias(int step) = 0;    // 005EF4D0(00F8896C) then 00F88968
    virtual int language_count() = 0;             // 008D45C0

    // Live application.
    virtual void apply_audio() = 0;               // 008D5430
    virtual void preview_speech() = 0;            // virtual +34h on +A4h
    virtual void preview_effects() = 0;           // virtual +34h on +A0h
    virtual bool cloud_system_present() = 0;      // 00E188A8 + 19E8h
    virtual void apply_clouds(bool enabled) = 0;  // 00BBDDF0 then 00BBCFE0
    virtual void apply_foliage(bool enabled) = 0; // 00AD71C0, then 00AD7A30 when enabled
    virtual void apply_motion_blur(bool enabled) = 0; // 00F8D39C + 219h
    virtual void apply_film_effect() = 0;         // 00B0D080

    // Committing.
    virtual void write_defaults(OptionsPage page) = 0; // 008D41C0/41F0/4520/4820
    virtual void clear_content_selection() = 0;   // 00427110, page 7's reset
    virtual void set_pending_language() = 0;      // 00F88984 into +D0h, page 1's reset
    virtual void apply_all() = 0;                 // 008D5B50
    virtual bool multiplayer_menu_present() = 0;  // 00E198C4
    virtual void reset_device() = 0;              // XLiveOnResetDevice, 00A4D46A
    virtual void raise_save_prompt() = 0;         // 00531B00 with callback 005F6FB0
    virtual void raise_discard_prompt() = 0;      // 00531B00 with callback 005F5B90

    // Navigation.
    virtual void build_page(OptionsPage page) = 0; // 005F58F0/59F0/41E0/40F0/3B50
    virtual void build_main_list() = 0;            // 005F54F0
    virtual void send_layout_payload() = 0;        // 00527C80 with +118h..+11Bh
    virtual void push_interface(int interface_id) = 0; // 004CC460(00E198B8, id, null)
    virtual void request_game_state(int state) = 0;    // 004D7920
    virtual void notify_multiplayer_menu() = 0;    // virtual +08h on 00E198C4 or 00E198B4
    virtual void set_gamepad_prompt(bool shown) = 0;   // byte at 00E19698 + 4
    virtual bool gamepad_prompt_shown() = 0;           // the same byte, read back
    virtual bool clan_text_available() = 0;        // 00585B40 and 00585810
    virtual void open_clan_text() = 0;             // 005EFAB0
    virtual void synthesise_reset_command() = 0;   // 00AAB4C0, 00531030(-59h), 00ABAED0
    virtual void cancel_storage() = 0;             // XCancelOverlapped twice, then memset
    virtual void reload_from_settings() = 0;       // 005F65C0
};

// ---------------------------------------------------------------------------
// The recovered sequences
// ---------------------------------------------------------------------------

// 005F42D0. Applies one step to the focused row of the current page and marks
// the screen dirty. Returns false when no row is focused or the page has no
// editable row at that index; the original simply falls out of its switch.
bool options_adjust_005f42d0(OptionsScreenHost& host, OptionsScreenState& state,
                             int step) noexcept;

// 005F7C40's page-0 arm. Activates the selected main-list row. Returns nullptr
// when the row is out of range or, for the clan-text row, when the host says it
// is unavailable; otherwise the row it acted on.
const OptionsMainListRow* options_activate_main_row_005f7c40(
    OptionsScreenHost& host, OptionsScreenState& state) noexcept;

// 005F8960's activation arm. Runs one list command against the current page.
void options_run_command_005f8960(OptionsScreenHost& host, OptionsScreenState& state,
                                  OptionsCommand command) noexcept;

// 005F0A80. Leaves the options screen for the main menu.
void options_leave_005f0a80(OptionsScreenHost& host, OptionsScreenState& state) noexcept;

// 005F5E60. The soft back: commit and rebuild the list, or raise the discard
// prompt when a value has changed. Returns true when it rebuilt the list.
bool options_return_to_list_005f5e60(OptionsScreenHost& host,
                                     OptionsScreenState& state) noexcept;

// 005F7310. One update pass over the actions the screen polls, in the order the
// original tests them. `seconds` advances the storage timer and the transition.
void options_update_005f7310(OptionsScreenHost& host, OptionsScreenState& state,
                             float seconds) noexcept;

} // namespace bsp
