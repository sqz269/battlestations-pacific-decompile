#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/ingame_interface.hpp"

// The GUI page each of the 42 in-mission HUD screens loads, the named widgets it
// binds, and the enter routines of the three screens the most interface ids
// raise. See docs/HUD_SCREEN_PAGES.md.
//
// The screen class has ten virtual slots (base vtable 00CEAE54, constructor
// 004F7180). Slot +10h registers and all 42 screens override it; slot +14h loads
// the layout and is a no-op (004F7590) in the base. The page name is a literal
// copied into a NativeString and passed to BSP_GuiManager_LoadPage (00AA5840),
// from either of those two slots; named widgets come from the child lookup
// 00AA7E00 on the page that was just loaded.
//
// The slot table itself, the manager layout and the interface-id screen sets
// live in bsp/ingame_interface.hpp; nothing here duplicates them.
namespace bsp {

// ---------------------------------------------------------------------------
// The pages
// ---------------------------------------------------------------------------

// One page name the binary passes to 00AA5840, with the installed file it
// resolves to. The name is matched case-insensitively: the binary writes
// "GUI_markers", the shipped file is interface/gui_markers.lua.
struct HudScreenPage {
    const char* name;           // the literal in .rdata
    const char* installed_file; // path under the game install, verified present
    std::uint16_t widget_keys;  // Name_Class keys the installed file defines
};

inline constexpr std::size_t kHudScreenPageCount = 47;
extern const HudScreenPage kHudScreenPages[kHudScreenPageCount];

// Case-insensitive lookup by the literal the binary uses. Null when the name is
// not one of the 47 pages these screens load.
const HudScreenPage* hud_screen_page(const char* name) noexcept;

// ---------------------------------------------------------------------------
// The widget bindings
// ---------------------------------------------------------------------------

// Which virtual performs a binding. Register and Layout are the only two the
// page loads happen in; a widget can also be looked up from the enter and update
// virtuals, which is why the binding carries the slot rather than assuming one.
enum class HudScreenVirtual : std::uint8_t {
    Id = 0x00,
    Register = 0x10,
    Layout = 0x14,
    Enter = 0x18,
    Exit = 0x1C,
    Update = 0x20,
};

// One name passed to the child lookup 00AA7E00 by a HUD screen. Every name in
// the table is a key defined by one of that screen's own pages; the check is
// recorded in reports/hud_screen_pages.json.
struct HudScreenWidgetBinding {
    const char* name;
    HudScreenVirtual bound_in;
};

// The page and widget layout of one HUD screen, keyed the same way as
// kInGameHudScreens in bsp/ingame_interface.hpp.
struct HudScreenLayout {
    std::uint8_t registry_slot;        // vtable +00h return value
    std::uint16_t offset;              // byte offset inside the manager
    std::uint32_t register_virtual;    // vtable +10h, always overridden
    std::uint32_t layout_virtual;      // vtable +14h, 004F7590 when not overridden
    const char* const* pages;          // in load order
    std::size_t page_count;
    const HudScreenWidgetBinding* widgets;
    std::size_t widget_count;
};

extern const HudScreenLayout kHudScreenLayouts[kInGameHudScreenCount];

// Lookup by the registry slot the screen's virtual +00h returns. Null for a slot
// the manager does not own.
const HudScreenLayout* hud_screen_layout_for_slot(int registry_slot) noexcept;

// True when ApplyPendingInterface's arm for `interface_id` puts `registry_slot`
// in the level-1 screen list. Derived from in_game_interface_screen_set so the
// two can never disagree; ids outside 20h..35h take the default arm, which
// clears the level and therefore raises nothing.
bool hud_screen_slot_raised_by_interface(int registry_slot, int interface_id) noexcept;

// The base's no-op layout virtual, the value kHudScreenLayouts carries for a
// screen that does not override +14h.
inline constexpr std::uint32_t kFrontEndScreenLayoutNoOp = 0x004F7590u;

// ---------------------------------------------------------------------------
// The three central screens
// ---------------------------------------------------------------------------

// 00E188D8, the controlled unit. Written only by BSP_Game_SetControlledUnit
// (004C0880); 00644230 is the whole accessor, `mov eax,[00E188D8]; ret`, so the
// HUD screens hold no unit pointer of their own and re-read the global.
inline constexpr std::uint32_t kControlledUnitGlobal = 0x00E188D8u;
inline constexpr std::uint32_t kHudRootControlledUnitAccessor = 0x00644230u;

// Integration boundary for the enter routines below. One method per native call
// site; no default implementations, and nothing here stands in for game
// behaviour that was not recovered.
struct HudScreenHost {
    virtual ~HudScreenHost() = default;
    // Widget virtual +4Ch with a float argument. The three call sites in
    // 006488D0 push 0.0f through `fldz` / `fstp [esp]`.
    virtual void widget_apply_float(std::uint32_t widget, float value) = 0;
    // Widget virtual +34h(1) -- the show call the minimap enter makes three times.
    virtual void widget_set_shown(std::uint32_t widget, bool shown) = 0;
    // 004BCA50 BSP_Game_GetEffectiveGameMode, ECX = the game at 00E188A8.
    virtual int game_effective_mode() = 0;
    // 00648290: clears the screen's two unit vectors and refills them from the
    // world unit list at [00E188A8 + 1974h]. Analyzed, not reconstructed.
    virtual void hud_root_rebuild_unit_lists() = 0;
    // The screen's own layout virtual +14h. 005BD550 re-runs it on every enter.
    virtual void screen_load_layout() = 0;
    // [00E188A8 + 61Fh] or [00E188A8 + 620h]: while either is set the HUD root
    // update returns without doing anything.
    virtual bool game_hud_suppressed() = 0;
};

// Slot 44h, the HUD root at manager +40h. Fields the enter and update touch.
struct HudRootScreenState {
    std::uint32_t widget_3c{0};        // +3Ch
    std::uint32_t widget_5c{0};        // +5Ch
    std::uint32_t widget_60{0};        // +60h
    int effective_game_mode{0};        // +24h, written by the enter
    std::int32_t update_countdown{0};  // +F4h, the update cadence counter
};

// Slot 4Dh, the markers screen at manager +48h.
struct HudMarkersScreenState {
    std::uint32_t page{0}; // +8Ch, the GUI_markers page the register virtual loads
    float field_88{0.0f};  // +88h
    float field_44{0.0f};  // +44h
    float field_48{0.0f};  // +48h
};

// Slot 35h, the minimap at manager +58h. Only the three widgets the enter shows.
struct HudMinimapScreenState {
    std::uint32_t widget_1c{0};  // +1Ch
    std::uint32_t widget_48{0};  // +48h
    std::uint32_t widget_f0{0};  // +F0h
};

// The two float constants the markers enter stores, 00CEB698 and 00CE3800.
inline constexpr float kHudMarkersEnterField88 = 0.023f;
inline constexpr float kHudMarkersEnterHalf = 0.5f;
// The reload value of +F4h at 00649899: the HUD root body runs every other call.
inline constexpr std::int32_t kHudRootUpdateInterval = 2;

// Enter virtual +18h of slot 44h, 006488D0, __thiscall(this), RET.
// Ghidra has no function here; body 006488D0..00648923.
void hud_root_screen_enter(HudRootScreenState& state, HudScreenHost& host);

// Enter virtual +18h of slot 4Dh, 00639480, __thiscall(this), RET. Three
// constant stores and nothing else. Ghidra has no function here;
// body 00639480..006394A2.
void hud_markers_screen_enter(HudMarkersScreenState& state) noexcept;

// Enter virtual +18h of slot 35h, 005BD550, __thiscall(this), RET,
// body 005BD550..005BD582.
void hud_minimap_screen_enter(const HudMinimapScreenState& state, HudScreenHost& host);

// The prologue of the slot 44h update virtual +20h, 00649860,
// __thiscall(this, float), RET 4, body 00649860..0064A24C. Returns true when the
// body runs. The rest of 00649860 is analyzed only; see docs/HUD_SCREEN_PAGES.md.
bool hud_root_screen_update_should_run(HudRootScreenState& state, HudScreenHost& host);

} // namespace bsp
