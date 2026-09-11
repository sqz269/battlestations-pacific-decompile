#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/frontend_managers.hpp"
#include "bsp/frontend_states.hpp"

// The seven front-end screens the main-menu manager at 00E198AC owns at
// +58h..+70h, and 005884A0, the routine the shell calls straight after the
// manager is raised.
//
// Every screen derives from the 004F7180 hierarchy of
// docs/GAME_FRONTEND_STATES.md: each constructor chains 004F7180 (which stores
// base vtable 00CEAE54 and clears the two flag bytes at +4h/+5h), then
// overwrites +00h with its own primary vtable and installs one or two secondary
// vtables at +08h/+0Ch. Slot +00h of each primary vtable is a `mov eax, imm32;
// ret` leaf that supplies the screen id, which 004F71D0 uses as the index into
// the 95-slot registry at 00E18B60. That id is also the interface id in the
// 00E08CD8 name table, so `bsp::front_end_interface_name` decodes it.
//
// Nothing here is a binary-compatible layout. The field offsets recorded below
// are the ones a constructor or a virtual demonstrably touches; every other byte
// of these objects is unrecovered.

namespace bsp {

// ---------------------------------------------------------------------------
// The ten virtual slots of the 00CEAE54 hierarchy
// ---------------------------------------------------------------------------

// Base vtable 00CEAE54 holds ten slots: 00BF698E (__purecall), 004F7570,
// 004F7580, 004F75E0, 004F71D0, 004F7590, 004F75A0, 004F75B0, 004F75C0,
// 004F75D0. 004F8B50 sits at 00CEAE7C but every derived table in this packet
// stops after slot +24h and is followed by string data, so the table is ten
// slots and 004F8B50 is the next .rdata item, not an eleventh slot.
inline constexpr std::size_t kFrontEndScreenVtableSlots = 10;

enum class FrontEndScreenSlot : int {
    ScreenId = 0x00,  // __purecall in the base; every leaf overrides it
    Slot04 = 0x04,    // 004F7570 in the base, never overridden here
    Slot08 = 0x08,    // 004F7580 in the base, never overridden here
    Destroy = 0x0C,   // 004F75E0 in the base; each leaf installs its deleting dtor
    Register = 0x10,  // 004F71D0 in the base; every leaf overrides and chains it
    BindLayout = 0x14, // 004F7590 in the base; only 005902E0 and 005098B0 override
    Enter = 0x18,     // 004F75A0
    Exit = 0x1C,      // 004F75B0
    Update = 0x20,    // 004F75C0, takes a float
    FillList = 0x24,  // 004F75D0, fills the list 004F83B0 walks
};

// ---------------------------------------------------------------------------
// The seven screens
// ---------------------------------------------------------------------------

// Manager field offsets, in the order 00686380 constructs them.
inline constexpr std::array<std::uint16_t, 7> kMainMenuScreenFieldOffsets{
    0x58, 0x5C, 0x60, 0x64, 0x68, 0x6C, 0x70,
};

// One recovered class per manager slot. `screen_id` is the constant the slot
// +00h leaf returns and is also the 00E08CD8 interface id. `register_virtual`
// is the slot +10h override; it chains 004F71D0 and, in five of the seven, is
// also where the GUI layout is bound through 004C12B0.
struct MainMenuScreenClass {
    std::uint16_t manager_field_offset; // 00E198AC + this
    std::uint32_t constructor;          // leased address
    std::uint32_t primary_vtable;       // stored at +00h
    std::uint32_t object_size;          // operator new size at the 00686380 site
    int screen_id;                      // slot +00h leaf, registry index
    std::uint32_t register_virtual;     // slot +10h
    std::uint32_t enter_virtual;        // slot +18h
    std::uint32_t exit_virtual;         // slot +1Ch
    std::uint32_t update_virtual;       // slot +20h
    std::string_view primary_layout;    // the GUI layout or data table it binds
};

inline constexpr std::array<MainMenuScreenClass, 7> kMainMenuScreens{{
    // +58h. The largest of the seven and the only one the shell touches by
    // hand. Its layout binding lives in the slot +14h override 005861B0, which
    // is also where FE_briefing, FE_briefing_grid and FE_worldmap_historical
    // are named, so this object carries the campaign surface as well as the
    // top-level list. Slot +10h (00582F30) chains 004F71D0 and then calls
    // 00B29E60, the presentation-mode change.
    {0x58, 0x005902E0u, 0x00CEFC5Cu, 0x578u, 0x01, 0x00582F30u, 0x005987F0u,
        0x0058F560u, 0x00599DB0u, "FE_main"},
    // +5Ch. The smallest. 005CAAF0 reads Scripts/datatables/MissionTree.lua and
    // its keys multiMissionInfos and missionGroups; it reports loading progress
    // through 0057BEC0 while it does.
    {0x5C, 0x005CA880u, 0x00CF170Cu, 0x034u, 0x02, 0x005CAAF0u, 0x005C27F0u,
        0x005C2800u, 0x005C4040u, "MissionTree"},
    // +60h. The post-mission scoring screen. 00629D10 binds FE_scoring and
    // names its six pages; the enter virtual 0062C2C0 is the widest routine in
    // the packet (100 callees) and drives the award and unlock text.
    {0x60, 0x00626630u, 0x00CF50CCu, 0x260u, 0x04, 0x00629D10u, 0x0062C2C0u,
        0x00621720u, 0x0062B640u, "FE_scoring"},
    // +64h. 0051E280 binds FE_briefing_grid and FE_briefing_listbox and their
    // Main_Listbox / MainListbox_Text widgets.
    {0x64, 0x0051E4D0u, 0x00CEC948u, 0x138u, 0x03, 0x0051E280u, 0x0051CDE0u,
        0x0051B1B0u, 0x0051C7D0u, "FE_briefing_grid"},
    // +68h. 0052FFC0 binds FE_credits and reads Scripts/datatables/Credits.lua
    // through the keys Credits, Credits_Items, RowSpace, StartY and Speed.
    {0x68, 0x0052FCE0u, 0x00CED200u, 0x05Cu, 0x08, 0x0052FFC0u, 0x0052EE30u,
        0x0052E150u, 0x0052E4B0u, "FE_credits"},
    // +6Ch. 00564220 binds the leaderboard list and slider widgets; the vtable
    // is followed by the keys Columns, TableDesc, ColumnName, LBoards,
    // LB_UniqueID and Leaderboards.
    {0x6C, 0x00563370u, 0x00CEE9CCu, 0x16Cu, 0x0A, 0x00564220u, 0x00562750u,
        0x00560FE0u, 0x00565540u, "Leaderboards"},
    // +70h. The tactical library. 0050FDB0 binds FE_tactical_listbox; the
    // vtable is followed by UnlockID, PowerupClasses, IsJapan and
    // Scripts\datatables\PowerUpLib.lua. Its slot +14h override is 004FE870.
    // Note the id: 0Bh decodes to INTF_ACHIEVEMENTS in the 00E08CD8 table while
    // every string in the class says tactical library. See the doc.
    {0x70, 0x005098B0u, 0x00CEB9B4u, 0x5D0u, 0x0B, 0x0050FDB0u, 0x00511F40u,
        0x0050B2D0u, 0x005162B0u, "FE_tactical_listbox"},
}};

// Interface ids the seven screens serve, named after their 00E08CD8 entry.
// kInterfaceMainMenu and kInterfaceRewards already exist in frontend_managers.
inline constexpr int kInterfaceMissionTree = 0x02;   // 005CA8E0
inline constexpr int kInterfaceBriefing = 0x03;      // 0051E250
inline constexpr int kInterfaceCredits = 0x08;       // 0052FD60
inline constexpr int kInterfaceLeaderboards = 0x0A;  // 005633F0
inline constexpr int kInterfaceAchievements = 0x0B;  // 00509D60

// Look up the recovered class by the manager field offset it occupies, or by
// the screen id it registers. Both return nullptr when nothing matches.
const MainMenuScreenClass* main_menu_screen_by_field_offset(int offset) noexcept;
const MainMenuScreenClass* main_menu_screen_by_id(int screen_id) noexcept;

// ---------------------------------------------------------------------------
// The main-menu page state, 00E08874
// ---------------------------------------------------------------------------

// The dword at 00E08874 is the page the main-menu screen (005902E0) is showing.
// It is not the interface id: the interface stays INTF_MAINMENU while the page
// walks this set. Each value is read back off a `mov dword ptr [0xE08874], imm`
// at the site named in the comment; the campaign pages come from a parameter,
// so they are read off the four-way test in the back handler 00598B60 instead.
enum class MainMenuPage : int {
    TopLevel = 0x01,        // 00584B91 in 00584AE0 (FE.main_menu); also
                            // 0059054F in the constructor and 0058F594 in the
                            // exit virtual, so leaving the screen resets it
    SinglePlayer = 0x02,    // 00584FE7 in 00584F50 (FE.main_singleplayer_title)
    Multiplayer = 0x03,     // 00585458 in 005853C0 (FE.main_multiplayer_title)
    // Pages 04h..08h are the five mission lists. Each one is settled by three
    // independent facts that agree, not by a neighbouring string: the title
    // 00597870 assigns, the mission-group widget handle it copies into +110h,
    // and the group index it publishes to 00E194D8. See the Corrections section
    // of docs/MAIN_MENU_MISSION_DETAIL.md; the earlier USN/IJN spelling here was
    // inferred from adjacent literals and had both sides reversed.
    CampaignJapan = 0x04,   // 005978FE pushes FE.ijn_campaign (00CEFE5C);
                            // 00597957 publishes group 1; 00597961 copies
                            // +314h missions_JP_Group
    CampaignUs = 0x05,      // 00597987 pushes FE.usn_campaign (00CEFE4C);
                            // 005979E0 publishes group 2; 005979EA copies
                            // +310h missions_US_Group
    CampaignJapanDlc = 0x06,  // 00597A75 pushes FE.main_ijn_dlc_title
                              // (00CEFE20); 00597AD3 publishes group 3;
                              // 00597AD9 copies +31Ch missions_JP_DLC_Group
    CampaignUsDlc = 0x07,   // 00597AFC pushes FE.main_usn_dlc_title (00CEFE08);
                            // 00597B3C publishes group 4; 00597B42 copies
                            // +318h missions_US_DLC_Group
    Page08 = 0x08,          // 0058099A in 00580940. Retained spelling.
    TrainingGrounds = 0x08, // 00597A05 pushes FE.training_grounds (00CEFE38);
                            // 00597A55 publishes group 0; 00597A5B copies
                            // +320h training_Group
    MissionDetail = 0x09,   // 0058CAC3 in 0058C010 (globals.continue)
    TacticalLibrary = 0x0B, // 0058877C in 005886F0 (FE.main_tacticallibrary);
                            // 005629B6 in the leaderboard screen writes it too
    Objectives = 0x0C,      // 00594C51 in 00594BF0 (globals.obj_pri/sec/hid)
};

// True for the four pages 00598B60 treats as a mission list at 00598B7B..
bool is_campaign_mission_list_page(MainMenuPage page) noexcept;

// True for the two of those four that carry the United States campaign.
// 00598FB3 stores that predicate into the screen byte the decompiler renders as
// `+55Ch`; the routine runs on `this = screen + 8` (00598FBF recovers the
// screen with `LEA ESI,[EDI-8]`), so the real field is screen+564h, the same
// byte 00597870 and 0058C010 set to 1 on pages 5 and 7. It is a side flag, not
// a content flag; the earlier `is_downloadable_content_page` reading of this
// site is recorded under Corrections in docs/MAIN_MENU_MISSION_DETAIL.md.
bool is_us_campaign_page(MainMenuPage page) noexcept;

// True for the two of those four that are downloadable content. The evidence is
// screen+565h, which 00597870 sets to 1 on pages 6 and 7 (00597AEE, 00597B4A)
// and to 0 on pages 4 and 5 (00597976, 005979F0), and which 0058C010 reproduces
// from the group index at 0058C0E8 and 0058C103.
bool is_downloadable_content_page(MainMenuPage page) noexcept;

// ---------------------------------------------------------------------------
// The top-level menu, built by 00584AE0
// ---------------------------------------------------------------------------

// 00584AE0 opens the FE.main_menu layout, sets the page to TopLevel and then
// runs a seven-iteration loop at 00584BF0..00584C70. Each pass allocates a list
// entry through 00AAB4C0, labels it with the string at 00E087B8[i], stores i
// into entry+0D8h and writes `enable[i] == 0` into the byte at entry+77h. The
// enable table is the seven bytes at 00CEF77C, all 1 in this image, so no item
// is disabled on this build.
inline constexpr std::size_t kMainMenuItemCount = 7;

struct MainMenuItem {
    int index;                 // the value stored at entry+0D8h
    std::string_view label;    // 00E087B8[index]
    bool enabled;              // 00CEF77C[index] != 0
};

inline constexpr std::array<MainMenuItem, kMainMenuItemCount> kMainMenuItems{{
    {0, "FE.main_singleplayer", true},    // 00CEF764
    {1, "FE.main_multiplayer", true},     // 00CEF750
    {2, "FE.main_tacticallibrary", true}, // 00CEF738
    {3, "FE.main_options", true},         // 00CEF728
    {4, "globals.live", true},            // 00CEF718
    {5, "FE_pc.main_marketplace", true},  // 00CEF700
    {6, "FE_pc.main_quit", true},         // 00CEF6F0
}};

// The eighth pointer in the 00E087B8 table. The loop stops at seven, so this
// entry is never used as a menu item; it is the help line for item 0.
inline constexpr std::string_view kMainMenuSinglePlayerHelp =
    "FE.main_singleplayer_help"; // 00CEF6D4

// ---------------------------------------------------------------------------
// Opening the tactical library, 005885D0 and 005886C0
// ---------------------------------------------------------------------------

// Both routines push interface 0Bh through 004CC460 on the manager and then
// write a mode into the tactical-library screen at 00E198AC+70h. They are the
// only two sites in the packet that reach another screen's fields directly.
struct TacticalLibraryRequest {
    int mode;                     // screen+94h: 5 from 005885D0, 4 from 005886C0
    int selector;                 // screen+98h, 63h at both sites
    bool has_selection;           // 005885D0 only: it fills +9Ch/+A0h first
    std::uint32_t selection_a;    // screen+9Ch, from 005806A0 then 005C27E0
    std::uint32_t selection_b;    // screen+A0h, from a second 005806A0
};

// 005885D0. __thiscall, ECX is the main-menu screen, no stack arguments, RET.
// It resolves two values off the screen, stores them into the library screen
// with the flag byte at +A4h cleared, sets mode 5 and then pushes 0Bh.
TacticalLibraryRequest open_tactical_library_005885d0(
    std::uint32_t selection_a, std::uint32_t selection_b) noexcept;

// 005886C0. Pushes 0Bh first and only then writes mode 4 and selector 63h, so
// the ordering is the reverse of 005885D0. No selection is written, which
// leaves +9Ch/+A0h holding whatever the previous request left there.
TacticalLibraryRequest open_tactical_library_005886c0() noexcept;

// ---------------------------------------------------------------------------
// 005884A0, the title music the shell starts
// ---------------------------------------------------------------------------

// The shell loads ECX with *(00E198AC)+58h at 004E4271 and calls 005884A0 at
// 004E4274, but the body never reads ECX: the only `this` it uses is the
// manager at 00E198AC. So this is a method on the +58h screen class whose body
// ignores its own object, and the effect is entirely on the manager. It is not,
// as the entry doc reads it, a call into the screen at +58h.
//
// It plays the title music. The path is the manager's +40h native string
// (sound/music/titlescreen.fsb) and the stream lands in the manager's +50h
// handle, which frontend_managers marks provisional; this routine is the writer
// that resolves it.
inline constexpr std::uint32_t kStartTitleMusicAddress = 0x005884A0u;
inline constexpr std::size_t kMusicStreamObjectSize = 0x54; // 005884D5

// 00584A30 builds the literal below and passes it to 004F8BA0, which compares
// it against the active clip and returns a byte. 005884A0 gives up when that
// byte is non-zero, so the title music does not start over the intro movie.
inline constexpr std::string_view kIntroMovieClip = "movies/midwaytheme.bik";

// The manager fields 005884A0 touches, projected out of MainMenuManager.
struct MainMenuMusicState {
    // 00E198AC+50h. Null until this routine fills it, and the routine's own
    // early-out, so the music is started at most once per manager lifetime.
    void* title_stream{nullptr};
};

// One method per native call site in 005884A0, in body order.
struct TitleMusicHost {
    virtual ~TitleMusicHost() = default;
    // 00584A30: is `clip` the movie that is playing right now?
    virtual bool movie_clip_active(std::string_view clip) = 0;
    // 005884D5 operator new(54h) then 00588528 the constructor 00A877D0, which
    // resolves `path` through BSP_VFS_ResolveExistingName. Returns null when
    // the allocation fails; the native code does not check.
    virtual void* create_music_stream(std::string_view path) = 0;
    // 0058854A: the native string at 00E19504 copied into a temporary. Written
    // by 0058BE63/0058BE6F and 0059A16B, none of them in this packet, so what
    // selects the track is unresolved.
    virtual std::string_view selected_track_name() = 0;
    // 0058855C, 00A867B0, which reaches FMOD createStream. RET 4.
    virtual void open_stream(void* stream, std::string_view track) = 0;
    // 0058858C reads the float at 00F889A8, written by BSP_Application_Initialize
    // at 0073DB76 and by 00685C80, so it is the configured music volume.
    virtual float music_volume() = 0;
    virtual void set_stream_volume(void* stream, float volume) = 0; // 00A864F0
    virtual void play_stream(void* stream) = 0;                     // 00A85C20
};

// Reconstruction of 005884A0. Native ECX is the +58h screen and is unused, no
// stack arguments, RET, no return value.
//
// Faithfulness note: when create_music_stream returns null the native code
// stores the null into +50h and still calls open_stream, set_stream_volume and
// play_stream on it. That path is preserved here rather than guarded, so a host
// sees exactly the calls the binary makes; deciding what a null stream means is
// the host's.
void start_title_music_005884a0(MainMenuMusicState& state, std::string_view title_music_path,
    TitleMusicHost& host);

} // namespace bsp
