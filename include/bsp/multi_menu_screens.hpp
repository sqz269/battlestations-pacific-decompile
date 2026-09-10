#pragma once
// The six screen objects the multiplayer menu manager at 00E198B4 builds into
// +40h..+54h, and the online refresh its Activate override runs.
// Addresses: 005E6D90, 005EA8C0, 005E3290, 00574240, 005D2F90, 005CFDA0,
//            00689310, 00689510.
// Every name below is a hypothesis, not a recovered symbol. Evidence and the
// call-by-call recovery are in docs/MULTI_MENU_SCREENS.md. Nothing here is
// binary compatible with the original: the vtables, the pooled strings, the
// checked-iterator containers and the MSVC list nodes are not reproduced.
//
// The manager object itself, its base and the interface-id constants are in
// bsp/frontend_managers.hpp; the screen base and its 95-slot registry are in
// bsp/frontend_states.hpp. Neither is redeclared here.
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/frontend_managers.hpp"
#include "bsp/frontend_states.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Interface ids these screens serve
// ---------------------------------------------------------------------------

// Read back from the 00E08CD8 name table (bsp/frontend_managers.hpp describes
// the table itself). Only the ids reached by this packet are added; 0Fh and 10h
// are already declared as kInterfaceMultiMainMenu and kInterfaceMultiModeSelector.
inline constexpr int kInterfaceMultiGameLobby = 0x14;      // "INTF_MULTIGAMELOBBY" 00CF7540
inline constexpr int kInterfaceMultiSessionBrowser = 0x1A; // "INTF_MULTISESSIONBROWSER" 00CF74BC
inline constexpr int kInterfaceMultiError = 0x1B;          // "INTF_MULTIERROR" 00CF74AC

// ---------------------------------------------------------------------------
// The six screens
// ---------------------------------------------------------------------------

// The screen registry index each class returns from primary vtable slot +00h,
// which BSP_FrontEndScreen_Register (004F71D0) uses as the index into the
// 95-slot array at 00E18B60. It is a different namespace from the interface id:
// the map at 00687330 translates one into the other.
enum class MultiMenuScreenId : int {
    MainMenu = 0x10,       // 005E6E00
    ModeSelector = 0x11,   // 005EA930
    Chat = 0x15,           // 005CFE40
    GameLobby = 0x16,      // 005E10A0
    SessionBrowser = 0x1C, // 00573BB0
    Error = 0x1D,          // 005D2F40
};

// One row per member of MultiMenuManager::screens, in the order Init (00689540)
// allocates them into +40h..+54h. `interface_id` is what 00687330 maps to
// `screen_id`; the chat overlay is no map target, so it has none.
struct MultiMenuScreenDescriptor {
    std::uint32_t constructor_address;
    std::size_t size_bytes;              // the operator new size in 00689540
    MultiMenuScreenId screen_id;
    int interface_id;                    // -1 when 00687330 never selects it
    std::uint32_t primary_vtable;        // stored at +00h
    std::uint32_t secondary_vtable;      // stored at +08h, 0 when absent
    std::uint32_t tertiary_vtable;       // stored at +0Ch, 0 when absent
    std::size_t manager_offset;          // slot in the manager, +40h..+54h
    std::string_view name;
};

inline constexpr std::size_t kMultiMenuScreenCount = 6;

inline constexpr std::array<MultiMenuScreenDescriptor, kMultiMenuScreenCount> kMultiMenuScreens{{
    {0x005E6D90u, 0x0F8u, MultiMenuScreenId::MainMenu, kInterfaceMultiMainMenu,
        0x00CF2708u, 0x00CF26F4u, 0x00CF26D4u, 0x40u, "multi main menu"},
    {0x005EA8C0u, 0x100u, MultiMenuScreenId::ModeSelector, kInterfaceMultiModeSelector,
        0x00CF2B44u, 0x00CF2B30u, 0x00CF2B10u, 0x44u, "multi mode selector"},
    {0x005E3290u, 0x300u, MultiMenuScreenId::GameLobby, kInterfaceMultiGameLobby,
        0x00CF23E4u, 0x00CF23D0u, 0x00CF23B0u, 0x48u, "multi game lobby"},
    {0x00574240u, 0x2D0u, MultiMenuScreenId::SessionBrowser, kInterfaceMultiSessionBrowser,
        0x00CEF134u, 0x00CEF120u, 0x00CEF100u, 0x4Cu, "multi session browser"},
    {0x005D2F90u, 0x05Cu, MultiMenuScreenId::Error, kInterfaceMultiError,
        0x00CF1BF4u, 0x00CF1BE0u, 0x00CF1BC0u, 0x50u, "multi error dialog"},
    {0x005CFDA0u, 0x090u, MultiMenuScreenId::Chat, -1,
        0x00CF1998u, 0x00CF19C0u, 0u, 0x54u, "multi chat overlay"},
}};

// Lookup helpers over the table above. Both return nullptr when nothing matches.
const MultiMenuScreenDescriptor* multi_menu_screen_by_id(MultiMenuScreenId id) noexcept;
const MultiMenuScreenDescriptor* multi_menu_screen_for_interface(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// GUI pages the register virtuals bind
// ---------------------------------------------------------------------------

// Each name is the literal handed to BSP_GuiManager_GetOrCreate (004C12B0) by
// the screen's virtual +10h; the handle 00AA5840 returns is stored at
// `handle_offset` of the screen. The two screens whose +10h does not call the
// GUI manager (005EA720 and 005EA7C0) reach their page from virtual +14h and
// have no row here.
struct MultiMenuGuiPage {
    MultiMenuScreenId screen_id;
    std::string_view page;
    std::size_t handle_offset;
    std::uint32_t register_address;
};

inline constexpr std::array<MultiMenuGuiPage, 4> kMultiMenuGuiPages{{
    {MultiMenuScreenId::GameLobby, "FE_lobby", 0x14u, 0x005E1820u},
    {MultiMenuScreenId::GameLobby, "FE_lobby_settings", 0x18u, 0x005E1820u},
    {MultiMenuScreenId::Error, "_MultiError", 0x10u, 0x005D2440u},
    {MultiMenuScreenId::Chat, "_Chat", 0x3Cu, 0x005CF260u},
}};

// Layout labels stored next to the main-menu screen's vtable at 00CF2730 and
// 00CF2748. They are adjacent .rdata literals, not a call argument this packet
// followed, so which of the two a run selects is unresolved.
inline constexpr std::string_view kMultiMainMenuPlayerTitle = "FE.multi_player_title";
inline constexpr std::string_view kMultiMainMenuRankedTitle = "FE.multi_ranked_title";

// ---------------------------------------------------------------------------
// Recovered field layout
// ---------------------------------------------------------------------------

// 00683610, the block every screen but the chat overlay embeds. It is
// __thiscall(this) with no callees: it clears a dword at +00h and +20h and +28h,
// zeroes eight floats (+0Ch..+1Ch, +24h, +2Ch, +34h..+3Ch, +54h) and clears the
// bytes at +48h and +58h. docs/FRONTEND_SCREEN_ANIMATION.md established that this
// is not an animation record but the constructor of the vertical scroll
// controller (BSP_GuiScroller_*, 00683300..00683a8f); bsp::FrontEndScreenScroller in
// include/bsp/frontend_screen_animation.hpp is the reconstructed layout and
// supersedes this placeholder. The main menu embeds one at +6Ch and the mode
// selector at +3Ch (LEA ECX,[ESI+3Ch] at 005ea8f8); the lobby and the session
// browser embed three consecutively.
struct MultiMenuScreenAnimation {
    std::uint32_t mode{0};        // +00h
    std::array<float, 5> a{};     // +0Ch..+1Ch
    std::uint32_t flags_20{0};    // +20h
    float b{0.0f};                // +24h
    std::uint32_t flags_28{0};    // +28h
    float c{0.0f};                // +2Ch
    std::array<float, 3> d{};     // +34h..+3Ch
    bool enabled_48{false};       // +48h
    float e{0.0f};                // +54h
    bool enabled_58{false};       // +58h
};

// 005E6D90, F8h. Constructor: base 004F7180, the two intermediate vtables
// 00CEB0FC/00CEB110 at +08h/+0Ch, then its own three, then one animation block
// at +6Ch. Every field below is written by its register virtual 005EA720.
struct MultiMainMenuScreen {
    FrontEndScreen base{};
    MultiMenuScreenAnimation animation{};  // +6Ch
    std::uint32_t field_e0{0};             // +E0h, cleared
    std::uint32_t field_e4{0};             // +E4h, cleared
    std::uint32_t field_ec{0};             // +ECh = *(00AA6750(scratch) + 4)
    std::uint32_t field_f0{0};             // +F0h, cleared
    std::int32_t selection_f4{-1};         // +F4h = -1
};

// 005EA8C0, 100h. Same constructor shape; its register virtual 005EA7C0 only
// runs virtual +14h and writes these two.
struct MultiModeSelectorScreen {
    FrontEndScreen base{};
    MultiMenuScreenAnimation animation{}; // +3Ch, not +6Ch (docs/FRONTEND_SCREEN_ANIMATION.md)
    std::uint32_t field_10{0};            // +10h, cleared
    std::int32_t selection_f4{-1};        // +F4h = -1
};

// 005E3290, 300h. Three animation blocks, then the constructor clears three
// vector triples and two bytes, publishes DAT_00E19594 and walks eight entries.
struct MultiGameLobbyScreen {
    FrontEndScreen base{};
    std::uint32_t field_1c{0};              // +1Ch, cleared
    std::uint32_t lobby_page{0};            // +14h, "FE_lobby"
    std::uint32_t lobby_settings_page{0};   // +18h, "FE_lobby_settings"
    std::array<MultiMenuScreenAnimation, 3> animation{};
    std::vector<void*> rows_1f8{};          // +1F8h..+200h
    std::vector<void*> rows_234{};          // +234h..+23Ch
    std::vector<void*> rows_244{};          // +244h..+24Ch
    bool flag_270{false};                   // +270h
    bool flag_271{false};                   // +271h
    std::vector<void*> rows_2e4{};          // +2E4h..+2ECh
    std::int32_t selection_2f0{-1};         // +2F0h = -1
    std::int32_t selection_2f4{-1};         // +2F4h = -1
};

// 00574240, 2D0h. Three animation blocks, then a std::list sentinel allocated
// by 0056A430 and stored at +27Ch, whose three links are pointed back at itself
// and whose byte +11h is set; 0056A090 clears the list and the sentinel is
// relinked a second time.
struct MultiSessionBrowserScreen {
    FrontEndScreen base{};
    std::array<MultiMenuScreenAnimation, 3> animation{};
    std::uint32_t field_270{0};      // +270h = DAT_00CEECEC
    std::int32_t field_274{-1};      // +274h = -1
    std::vector<void*> sessions{};   // the list whose sentinel lives at +27Ch
    std::uint32_t count_280{0};      // +280h, cleared twice
    std::uint32_t field_284{0};      // +284h
    bool flag_288{false};            // +288h
    std::uint32_t field_28c{0};      // +28Ch
    std::vector<void*> rows_294{};   // +294h..+29Ch
    std::vector<void*> rows_2a4{};   // +2A4h..+2ACh
    std::uint32_t field_2b0{0};      // +2B0h
};

// 005D2F90, 5Ch. Two 10h-byte elements constructed at +30h by the EH vector
// iterator, then "globals.yes" and "globals.no" pushed through 00450540.
struct MultiErrorScreen {
    FrontEndScreen base{};
    std::uint32_t page{0};                 // +10h, "_MultiError"
    std::array<std::uint32_t, 4> button_a{}; // +30h, 10h bytes, ctor 004324A0
    std::array<std::uint32_t, 4> button_b{}; // +40h, 10h bytes, ctor 004324A0
    std::uint32_t field_54{0};             // +54h, cleared by 005D2440
};
inline constexpr std::string_view kMultiErrorConfirmLabel = "globals.yes";
inline constexpr std::string_view kMultiErrorCancelLabel = "globals.no";

// 005CFDA0, 90h. The only one of the six with two vtables rather than three,
// and the only one whose constructor chains a second base (00A97260) after
// 004F7180. Three subobjects follow, each with a cleared dword behind it.
struct MultiChatScreen {
    FrontEndScreen base{};
    std::uint32_t page{0};       // +3Ch, "_Chat"
    std::uint32_t part_6c{0};    // +6Ch = 005CEE20()
    std::uint32_t field_70{0};   // +70h
    std::uint32_t part_78{0};    // +78h = 005CEE40()
    std::uint32_t field_7c{0};   // +7Ch
    std::uint32_t part_84{0};    // +84h = 005CEE40()
    std::uint32_t field_88{0};   // +88h
    bool flag_8c{false};         // +8Ch
    bool flag_8d{false};         // +8Dh
};

// ---------------------------------------------------------------------------
// The online refresh, 00689310 and 00689510
// ---------------------------------------------------------------------------

// The record 006883B0 copies element by element. Its traced extent is 3Ch: a
// std::string at +04h and the scalars at +20h, +24h, +28h..+2Bh, +2Ch, +30h,
// +34h and +38h..+3Bh. Nothing in this packet reads any of the scalars, so they
// keep their offsets as names.
struct MultiMenuOnlineRecord {
    std::string name;                        // +04h, MSVC std::string
    std::uint32_t field_20{0};
    std::uint32_t field_24{0};
    std::array<std::uint8_t, 4> flags_28{};
    std::uint32_t field_2c{0};
    std::uint8_t field_30{0};
    std::uint32_t field_34{0};
    std::array<std::uint8_t, 4> flags_38{};
};

// The scratch record 00689310 builds at ESP+40h before the scan. Only these
// four writes happen, so the rest of the record starts value-initialised.
inline constexpr std::uint8_t kMultiMenuScratchFlag3A = 1; // byte +3Ah = 1

// 00E08701, the byte 00689310 sets at 00689466. 0057B530, in the session
// browser's own segment, tests it and clears it before rebuilding its list from
// the client's collection at virtual +94h, which is what makes it a dirty flag
// rather than a mode. 005E6F18 and 005E71CD, both inside the main-menu screen
// class, are the other two writers and were not read here.
struct MultiMenuOnlineRefreshState {
    bool player_list_dirty{false};
};

// One method per native call site of 00689510 and 00689310. There are no
// default implementations: nothing here stands in for unrecovered game
// behaviour. The online client is the object at 00F8A2FC; 00689510 dereferences
// it without a null check, so a host must always have one.
struct MultiMenuOnlineHost {
    virtual ~MultiMenuOnlineHost() = default;

    // 00684700, the base activate, run before the gate.
    virtual void activate_front_end_manager_base() = 0;

    // Online client virtual +68h. Its bool in AL is the only gate on the
    // refresh; 00689510 returns without it.
    virtual bool online_refresh_available() = 0;

    // Online client virtual +70h(out, query, 1). Returns a record by value into
    // a caller buffer; `query` is a default-constructed record and the trailing
    // argument is the literal 1. The result supplies the name every list entry
    // is compared against.
    virtual MultiMenuOnlineRecord online_local_record(
        const MultiMenuOnlineRecord& query, int selector) = 0;

    // Online client virtual +7Ch(out). Fills an MSVC std::list of the record
    // above; 00689310 owns it and frees the head proxy itself at 00689486.
    virtual std::vector<MultiMenuOnlineRecord> online_player_list() = 0;

    // Online client virtual +1A0h(0), the last call before the list is torn
    // down. Its single argument is the literal 0.
    virtual void online_notify_refreshed(int reason) = 0;
};

inline constexpr int kMultiMenuOnlineLocalSelector = 1; // +70h third argument
inline constexpr int kMultiMenuOnlineNotifyReason = 0;  // +1A0h argument

// 00689310, __thiscall(this) reached by the tail jump at 0068952C, RET, no
// stack arguments. ECX carries the manager but the body never reads it, so no
// manager parameter is modelled. Scans the client's player list for the entry
// whose name equals the local record's, sets the dirty byte and notifies the
// client. The original copies each match into a stack record it then destroys;
// the match is returned here so a caller can see what the scan selected, and
// that return has no counterpart in the original.
std::optional<MultiMenuOnlineRecord> refresh_online_player_list_00689310(
    MultiMenuOnlineRefreshState& state, MultiMenuOnlineHost& host);

// 00689510 BSP_MultiMenu_Activate, __thiscall(this), RET, the manager's virtual
// +8h. Base activate, then the refresh when the client allows it. Returns
// whether the refresh ran.
bool activate_multi_menu_00689510(
    MultiMenuOnlineRefreshState& state, MultiMenuOnlineHost& host);
}
