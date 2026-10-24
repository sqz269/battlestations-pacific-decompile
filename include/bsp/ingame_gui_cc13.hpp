// In-game GUI screens of segments 26 and 27, reconstruction pass (packet cc13-fe2).
//
// Ranges 0061D5C0..00654A70 and 00654A90..00680D30: the HUD scene-init, markers,
// spectator, root, ship-status, submarine, periscope, support-manager, tactical
// sheet, cursor, follow, binoculars and unit-library screens. docs/CC13_FE_2.md
// carries the evidence and the list of functions left to cc9.
//
// Model: every routine works on the original object as raw bytes (Win32 layout,
// offsets as in the image) and reaches the engine through HudScreenServices.
// New C++ interfaces, not drop-in binary replacements; nothing here was run in
// bsp_game.

#ifndef BSP_INGAME_GUI_CC13_HPP
#define BSP_INGAME_GUI_CC13_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

#include "bsp/main_menu_command_bar.hpp"

namespace bsp::ingame_gui_cc13 {

static_assert(sizeof(void*) == 4, "the in-game GUI objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

// ---------------------------------------------------------------------------
// Engine services the screens call. Each method names the image routine.
// ---------------------------------------------------------------------------
struct HudScreenServices {
    virtual ~HudScreenServices() = default;
    // Front-end screen base (src/frontend_state_machine.cpp).
    virtual void front_end_screen_construct_004f7180(Raw screen) = 0;
    virtual void front_end_screen_destruct_004f71a0(Raw screen) = 0;
    virtual void front_end_screen_register_004f71d0(Raw screen) = 0;
    virtual void front_end_screen_commit_visibility_004f83b0(Raw screen) = 0;  // ECX = screen
    // The screen's own vtable slot +14h (the layout/bind virtual), ECX = screen.
    virtual void screen_virtual_14(Raw screen) = 0;
    // GUI manager: 004C12B0 then 00AA5840(name, 1, 0) / 00AA31F0(page).
    virtual void* gui_load_page_00aa5840(std::string_view name) = 0;
    virtual void gui_remove_and_destroy_page_00aa31f0(void* page) = 0;
    // 00AA7E00, ECX = parent widget, (name, 1).
    virtual void* gui_find_child_00aa7e00(void* parent, std::string_view name) = 0;
    // 00AA8000-family widget virtuals.
    virtual void widget_set_visible_34(void* widget, bool visible) = 0;     // vtable +34h
    virtual void widget_set_focus_60(void* widget, bool focus) = 0;         // vtable +60h
    virtual void widget_set_colour_50(void* widget, const float rgba[4]) = 0;  // vtable +50h
    virtual void widget_get_colour_54(void* widget, float rgba[4]) = 0;        // vtable +54h
    virtual void widget_set_float_4c(void* widget, float value) = 0;        // vtable +4Ch
    virtual void object_delete_04(void* object) = 0;                        // vtable +4h(1)
    // 00A9AC40, ECX = listbox, (listener).
    virtual void listbox_set_listener_00a9ac40(void* listbox, void* listener) = 0;
    // 004D6790, ECX = vector of page slots, (slot address).
    virtual void push_page_slot_004d6790(void* vector, void** slot) = 0;
    // Observer pair: 00694A60 register, 006952A0 unregister, 00694AF0 is-registered.
    // Each takes the observing object and the observed one.
    virtual void observer_register_00694a60(void* observer, void* observed) = 0;
    virtual void observer_unregister_006952a0(void* observer, void* observed) = 0;
    virtual bool observer_is_registered_00694af0(void* observer, void* observed) = 0;
    // 004D92B0, ECX = *00E188A8: action pressed this frame or auto-repeating.
    virtual bool action_edge_or_repeat_004d92b0(int action) = 0;
    // *(*00E188A8 + 1FE4h): the session mode (0 single player).
    virtual std::int32_t session_mode() = 0;
};

// ---------------------------------------------------------------------------
// Trivial vtable slots (constant returns, empty bodies, adjustor thunks)
// ---------------------------------------------------------------------------
enum class TrivialSlotKind : std::uint8_t {
    ReturnInt,      // MOV EAX,imm; RET
    ReturnByte,     // MOV AL,imm; RET
    Empty,          // RET n
    Adjustor,       // SUB ECX,n; JMP target (this-adjusting thunk)
    TailJump,       // JMP target (the whole body is the base routine)
};

struct TrivialSlot {
    std::uint32_t address;
    TrivialSlotKind kind;
    std::int32_t value;       // returned value, or the ECX adjustment for Adjustor
    std::uint16_t ret_bytes;  // stack bytes released by RET (Empty/Return kinds)
    std::uint32_t target;     // Adjustor / TailJump destination
    std::string_view role;
};

// Bodies read from the disk image (disasm-raw); see docs/CC13_FE_2.md.
inline constexpr std::array<TrivialSlot, 45> kTrivialSlots{{
    {0x00626710, TrivialSlotKind::ReturnInt, 0x04, 0, 0, "rewards screen id"},
    {0x00626750, TrivialSlotKind::Adjustor, 8, 0, 0x00627800, "rewards secondary-base deleting destructor"},
    {0x00636DC0, TrivialSlotKind::ReturnInt, 0x5A, 0, 0, "scene-init screen id"},
    {0x00636DD0, TrivialSlotKind::ReturnByte, 1, 0, 0, "scene-init occludes lower levels (slot +8h)"},
    {0x00636DE0, TrivialSlotKind::ReturnByte, 1, 0, 0, "scene-init self-managed (slot +4h)"},
    {0x00636DF0, TrivialSlotKind::Adjustor, 8, 0, 0x00636F10, "scene-init secondary-base deleting destructor"},
    {0x006375F0, TrivialSlotKind::Empty, 0, 4, 0, "markers listener slot +4h"},
    {0x006405E0, TrivialSlotKind::ReturnInt, 0x4D, 0, 0, "markers screen id"},
    {0x006405F0, TrivialSlotKind::Adjustor, 8, 0, 0x00640D50, "markers secondary-base deleting destructor"},
    {0x00643DC0, TrivialSlotKind::ReturnInt, 0x43, 0, 0, "spectator screen id"},
    {0x00643DD0, TrivialSlotKind::ReturnByte, 1, 0, 0, "spectator self-managed"},
    {0x00643DE0, TrivialSlotKind::Empty, 0, 0, 0, "spectator exit"},
    {0x00643DF0, TrivialSlotKind::Empty, 0, 4, 0, "spectator update(float)"},
    {0x00647FE0, TrivialSlotKind::ReturnInt, 0x44, 0, 0, "HUD root screen id"},
    {0x00647FF0, TrivialSlotKind::Adjustor, 8, 0, 0x00648040, "HUD root secondary-base deleting destructor"},
    {0x0064B6F0, TrivialSlotKind::Adjustor, 0x24, 0, 0x0064B760, "ShipCaptain camera base +24h destructor"},
    {0x0064B700, TrivialSlotKind::Adjustor, 0x10, 0, 0x0064B760, "ShipCaptain camera base +10h destructor"},
    {0x0064B710, TrivialSlotKind::Adjustor, 0x170, 0, 0x0064B760, "ShipCaptain camera base +170h destructor"},
    {0x0064B860, TrivialSlotKind::ReturnInt, 0x45, 0, 0, "ship-status screen id"},
    {0x00650160, TrivialSlotKind::ReturnInt, 0x48, 0, 0, "submarine screen id"},
    {0x00650190, TrivialSlotKind::Empty, 0, 0, 0, "submarine enter"},
    {0x00650880, TrivialSlotKind::Adjustor, 0x10, 0, 0x006508F0, "submarine camera base +10h destructor"},
    {0x00650890, TrivialSlotKind::Adjustor, 0x170, 0, 0x006508F0, "submarine camera base +170h destructor"},
    {0x006508A0, TrivialSlotKind::Adjustor, 0x24, 0, 0x006508F0, "submarine camera base +24h destructor"},
    {0x00650AC0, TrivialSlotKind::ReturnInt, 0x47, 0, 0, "periscope screen id"},
    {0x00650AD0, TrivialSlotKind::Adjustor, 8, 0, 0x00650AE0, "periscope secondary-base deleting destructor"},
    {0x006730F0, TrivialSlotKind::ReturnInt, 0x4E, 0, 0, "support-manager screen id"},
    {0x0067AF30, TrivialSlotKind::Empty, 0, 8, 0, "tactical sheet listbox selection changed"},
    {0x0067AF60, TrivialSlotKind::ReturnInt, 0x54, 0, 0, "tactical sheet screen id"},
    {0x0067AF70, TrivialSlotKind::Adjustor, 8, 0, 0x0067B0F0, "tactical sheet secondary-base deleting destructor"},
    {0x0067B320, TrivialSlotKind::ReturnInt, 0x28, 0, 0, "cursor screen id"},
    {0x0067B330, TrivialSlotKind::Empty, 0, 0, 0, "cursor exit"},
    {0x0067B3F0, TrivialSlotKind::TailJump, 0, 0, 0x004F71D0, "common overlay (27h) register = base register"},
    {0x0067B400, TrivialSlotKind::Empty, 0, 0, 0, "common overlay enter"},
    {0x0067B410, TrivialSlotKind::Empty, 0, 0, 0, "common overlay exit"},
    {0x0067BE50, TrivialSlotKind::ReturnInt, 0x49, 0, 0, "follow screen id"},
    {0x0067BE60, TrivialSlotKind::Empty, 0, 4, 0, "follow collect children"},
    {0x0067BEA0, TrivialSlotKind::Empty, 0, 0, 0, "follow bind layout (slot +14h)"},
    {0x0067BEB0, TrivialSlotKind::Empty, 0, 0, 0, "follow enter"},
    {0x0067C860, TrivialSlotKind::ReturnInt, 0x5C, 0, 0, "press-start screen id"},
    {0x0067D9F0, TrivialSlotKind::Empty, 0, 8, 0, "unit library listbox selection changed"},
    {0x0067DA00, TrivialSlotKind::Empty, 0, 4, 0, "unit library widget listener slot +0h"},
    {0x0067F050, TrivialSlotKind::ReturnInt, 0x05, 0, 0, "unit library screen id"},
    {0x0067F060, TrivialSlotKind::Adjustor, 8, 0, 0x00680760, "unit library secondary-base deleting destructor"},
    {0x0067F070, TrivialSlotKind::Adjustor, 0x64, 0, 0x00680760, "unit library +64h base deleting destructor"},
}};

const TrivialSlot* find_trivial_slot(std::uint32_t address) noexcept;

// ---------------------------------------------------------------------------
// Collect-children slots (+24h): out->push_back(&this->page) for each page.
// ---------------------------------------------------------------------------
struct CollectChildrenSlot {
    std::uint32_t address;
    std::array<std::uint16_t, 4> page_offsets;
    std::uint8_t count;
};

inline constexpr std::array<CollectChildrenSlot, 11> kCollectChildrenSlots{{
    {0x00626760, {0x20}, 1},                    // rewards
    {0x00637470, {0x1C}, 1},                    // scene init
    {0x00640600, {0x8C}, 1},                    // markers
    {0x00644060, {0x08}, 1},                    // spectator
    {0x00648000, {0x28, 0x2C, 0xF8}, 3},        // HUD root
    {0x0064DA00, {0x34, 0x38, 0x3C, 0x40}, 4},  // ship status
    {0x00651350, {0x08}, 1},                    // submarine
    {0x00651360, {0x6C}, 1},                    // periscope
    {0x00673100, {0x78, 0x16C}, 2},             // support manager
    {0x0067B2A0, {0x0C}, 1},                    // tactical sheet
    {0x0067BE20, {0x08}, 1},                    // cursor
}};

// __thiscall(ECX = screen, vector* out), RET 4.
void collect_children(const CollectChildrenSlot& slot, Raw screen, void* out, HudScreenServices& host);

// ---------------------------------------------------------------------------
// More engine services used by individual screens.
// ---------------------------------------------------------------------------
struct HudScreenHost : HudScreenServices {
    // A no-argument virtual of a screen or object: (*obj)[slot](), ECX = obj.
    virtual void object_virtual(Raw object, std::uint16_t slot) = 0;
    // *(global): the object a global pointer cell holds.
    virtual Raw global_object(std::uint32_t global) = 0;
    // *(*00E198C4 + off): a field of the in-game interface.
    virtual Raw interface_object(std::uint16_t off) = 0;
    // 006881F0, ECX = *00E198B4 (the multiplayer menu): pop the interface request.
    virtual void multi_menu_pop_request_006881f0() = 0;
    // 0041DD40(0, 0) then a zero-length copy from 00CE3A0C: assign "" to the
    // native string (count, data) at that address.
    virtual void native_string_assign_empty(Raw string) = 0;
    // 00695870, ECX = a callback-owner sub-object (non-virtual destructor).
    virtual void callback_owner_destroy_00695870(Raw owner) = 0;
    // 004BEC00 then 00A92C40(float), ECX = the input manager.
    virtual void input_manager_update_00a92c40(float seconds) = 0;
    // 006833F0, ECX = the scroller block.
    virtual void scroller_recapture_006833f0(Raw scroller) = 0;
    // *(*00E188A8 + 64Ch): the game clock in seconds (float).
    virtual float game_clock_064c() = 0;
    // 004C1E90(0Bh) then 00427190, ECX = that owner.
    virtual void auto_test_step_0b() = 0;
    // 00549260(unit, camera) and 005470C0 (the weapon-group HUD binding).
    virtual void weapon_group_bind_unit_00549260(void* unit, void* camera) = 0;
    virtual void weapon_group_release_005470c0() = 0;  // ECX = *(*00E198C4 + 50h)
    // 0077C470 BSP_UnitInstance_SendRoleTransfer, ECX = unit, (mask, take): a
    // cc9 gameplay routine (docs/SCRIPTED_HELM.md); the exits below release
    // mask 1FEh with take 0.
    virtual void unit_send_role_transfer_0077c470(void* unit, std::uint32_t mask, int take) = 0;
    // 0051E8E0, ECX = *(*00E198C4 + 4Ch): binoculars reset.
    virtual void binoculars_reset_0051e8e0() = 0;
    // 0051F330(arg), the binoculars view update, and 005454B0(x, y, z) on the
    // screen 2Eh object (*(*00E198C4 + 4Ch)), whose +38h supplies z.
    virtual void binoculars_update_view_0051f330(float seconds) = 0;  // ECX = *(*00E198C4 + 4Ch)
    // ECX = *(*00E198C4 + 50h); z is the binoculars zoom (*(*00E198C4 + 4Ch) + 38h)
    // or the caller's own +5Ch.
    virtual void screen2e_set_view_centre_005454b0(float x, float y, float z) = 0;
    virtual void text_set_ellipsized_00abb000(void* text, const char* source, float width, bool flag) = 0;
    virtual void game_set_view_fov_004dc940(float fov, int flag) = 0;      // ECX = *00E188A8
    virtual void submarine_camera_select_mode_0042cb70(void* camera, int mode) = 0;
    virtual void screen_update_20(Raw screen, float seconds) = 0;          // vtable +20h
    virtual void camera_virtual_dc(void* camera, float value) = 0;         // vtable +DCh
    virtual void unit_view_ensure_submarine_camera_00651370(Raw screen) = 0;
    // 00452B80(enabled), 006506E0 (periscope detach; reconstructed below but
    // reached through the host when another routine calls it).
    virtual void lens_flare_set_enabled_00452b80(bool enabled) = 0;
    virtual void periscope_detach_unit_006506e0(Raw screen) = 0;
    // 00926D90(2), ECX = the mission entity.
    virtual void mission_entity_kill_00926d90(void* entity) = 0;
    // 0068A9C0, ECX = *00E198C4: returns AL.
    virtual bool interface_apply_pending_level_change_0068a9c0() = 0;
    // 00B0CFC0(&a, &b, &c) and 00B0D6A0(a, b, c, x, y, z), ECX = *00F8D39C
    // (the render service); all six are floats passed by value.
    virtual void render_get_transition_00b0cfc0(float& a, float& b, float& c) = 0;
    virtual void render_set_transition_00b0d6a0(float a, float b, float c, float x, float y, float z) = 0;
    // 0042E740: game tuning; floats +D8h/+DCh/+E0h are the default transition.
    virtual float game_tuning_float(std::uint16_t off) = 0;
    // 004BCA80, ECX = *00E198C4.
    virtual void controlled_listener_publish_004bca80() = 0;
    // *00F876A4: the real-time clock (float seconds).
    virtual float real_clock_00f876a4() = 0;
    // 00AA8B80(index), ECX = widget: drop a timed entry.
    virtual void widget_delete_timed_entry_00aa8b80(void* widget, int index) = 0;
};

// ---------------------------------------------------------------------------
// Screen 54h, the tactical sheet (vtable 00CF6CEC, listener 00CF6CD8 at +8h).
// Page "TacticalSheetGui" at +0Ch.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kTacticalSheetVtable = 0x00CF6CEC;
inline constexpr std::uint32_t kTacticalSheetListenerVtable = 0x00CF6CD8;
inline constexpr std::uint32_t kListboxListenerBaseVtable = 0x00CEB0FC;
inline constexpr std::string_view kTacticalSheetPage = "TacticalSheetGui";
inline constexpr std::string_view kTacticalSheetListbox = "Main_Listbox";
inline constexpr int kActionTacticalSheetCloseA = 0x4A;
inline constexpr int kActionTacticalSheetCloseB = 0x4B;
inline constexpr std::uint32_t kTacticalSheetLinkedScreen = 0x00E1930C;
inline constexpr std::uint16_t kInterfaceResumeScreen = 0xD8;

Raw tactical_sheet_construct_0067af40(Raw s, HudScreenHost& host);
bool tactical_sheet_self_managed_0067af80(HudScreenHost& host);
void tactical_sheet_exit_0067afa0(HudScreenHost& host);
void tactical_sheet_close_0067afd0(Raw s, HudScreenHost& host);
void tactical_sheet_destruct_0067b040(Raw s, HudScreenHost& host);
void tactical_sheet_update_0067b0b0(Raw s, float seconds, HudScreenHost& host);
void tactical_sheet_listbox_activate_0067b0e0(Raw listener, HudScreenHost& host);
void tactical_sheet_register_0067b110(Raw s, HudScreenHost& host);
void tactical_sheet_enter_0067b1d0(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 28h, the cursor (vtable 00CF6D28). Page "CursorGui" at +8h, its
// "Cursor_Icon" at +0Ch.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kCursorScreenVtable = 0x00CF6D28;
inline constexpr std::string_view kCursorPage = "CursorGui";
inline constexpr std::string_view kCursorIcon = "Cursor_Icon";

Raw cursor_screen_construct_0067b300(Raw s, HudScreenHost& host);
void cursor_screen_destruct_0067b780(Raw s, HudScreenHost& host);
void cursor_screen_register_0067bcb0(Raw s, HudScreenHost& host);
void cursor_screen_enter_0067bd70(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 49h, the follow screen (vtable 00CF6D68): +8h is the followed unit.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kFollowScreenVtable = 0x00CF6D68;
Raw follow_screen_construct_0067be30(Raw s, HudScreenHost& host);
void follow_screen_register_0067be80(Raw s, HudScreenHost& host);
void follow_screen_exit_0067bec0(Raw s) noexcept;

// ---------------------------------------------------------------------------
// Screen 4Ah, the binoculars overlay. +1Ch view value, +24h unit, +2Ch camera.
// ---------------------------------------------------------------------------
inline constexpr std::string_view kBinocularsPage = "GUI_binoculars";
void binoculars_overlay_register_0067c430(Raw s, HudScreenHost& host);
void binoculars_overlay_enter_0067c050(Raw s, HudScreenHost& host);
void binoculars_overlay_released_0067c130(Raw s, void* object) noexcept;
void binoculars_overlay_exit_0067c0a0(Raw s, HudScreenHost& host);
void binoculars_update_view_recentre_0067c0e0(Raw s, float seconds, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 24h, the bomb/torpedo view (+4Ch bomb camera, +50h torpedo camera,
// +1Ch..+24h the saved render transition, +28h..+30h the last camera position,
// +40h..+48h a queued selection, +54h the enter time).
// ---------------------------------------------------------------------------
void bomb_view_released_0067b3a0(Raw s, void* object) noexcept;
void bomb_view_queue_selection_0067b3d0(Raw s, std::uint32_t a, std::uint32_t b) noexcept;
void bomb_view_exit_0067b450(Raw s, HudScreenHost& host);
void bomb_view_enter_0067baa0(Raw s, HudScreenHost& host);

// The common overlay's observer (+28h observed object).
void common_overlay_released_0067bc60(Raw s, void* object, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 43h, the spectator (vtable 00CF5998). Page "GUI_spectator" at +8h,
// "Score_Text" at +0Ch, "Spectator_Text" at +10h, +14h a byte.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kSpectatorScreenVtable = 0x00CF5998;
Raw spectator_construct_00643da0(Raw s, HudScreenHost& host);
void spectator_destruct_00643e00(Raw s, HudScreenHost& host);
void spectator_register_00643e80(Raw s, HudScreenHost& host);
void spectator_enter_00643f40(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 5Ah, the scene-init (loading) screen (vtable 00CF569C, callback owner
// 00CF5684 at +8h). Page "FE_sceneinit" at +1Ch, "Message_Text" at +20h and its
// colour at +24h..+30h.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kSceneInitVtable = 0x00CF569C;
inline constexpr std::uint32_t kSceneInitCallbackVtable = 0x00CF5684;
inline constexpr std::uint32_t kCallbackOwnerBaseVtable = 0x00CE3CD4;
inline constexpr std::uint32_t kSceneInitScreenGlobal = 0x00E19698;
inline constexpr double kSceneInitPulseScale = 0.75;  // 00CEC9D8
inline constexpr double kSceneInitPulseBase = 0.25;   // 00D7A348
void scene_init_exit_00636d80(HudScreenHost& host);
Raw scene_init_construct_00636d90(Raw s, HudScreenHost& host);
void scene_init_destruct_00636e00(Raw s, HudScreenHost& host);
void scene_init_update_00636e80(Raw s, float seconds, HudScreenHost& host);
void scene_init_register_00636f30(Raw s, HudScreenHost& host);
float scene_init_pulse_alpha(float clock) noexcept;

// ---------------------------------------------------------------------------
// Screens 48h (submarine, vtable 00CF61D4) and 47h (periscope, vtable 00CF639C).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kSubmarineScreenVtable = 0x00CF61D4;
inline constexpr std::uint32_t kPeriscopeScreenVtable = 0x00CF639C;
inline constexpr std::uint32_t kPeriscopeCallbackVtable = 0x00CF6384;
Raw submarine_construct_00650140(Raw s, HudScreenHost& host);
void submarine_register_00650170(Raw s, HudScreenHost& host);
void submarine_destruct_00650910(Raw s, HudScreenHost& host);
void periscope_released_006502d0(Raw s, void* object) noexcept;
void periscope_exit_00650810(Raw s, HudScreenHost& host);
Raw periscope_construct_00650b00(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 4Dh, the HUD markers, and its marker widget sets.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kHudMarkersVtable = 0x00CF5950;
inline constexpr std::uint32_t kHudMarkersCallbackVtable = 0x00CF5938;
inline constexpr std::uint32_t kHudNameMarkerVtable = 0x00CF5778;
inline constexpr std::uint32_t kHudBefogoMarkerVtable = 0x00CF577C;
inline constexpr std::uint32_t kHudColourIconSetVtable = 0x00CF575C;
inline constexpr std::string_view kHudNameMarkerText = "NameMarker_Text";
inline constexpr std::string_view kHudBefogoMarkerIcon = "BefogoMarker_Icon";
// 00637620, __thiscall(screen, x, y), RET 8.
void hud_markers_set_view_centre_00637620(Raw s, float x, float y) noexcept;
// 006374B0: hide the 24 widgets at +8h..+64h and the seven at +68h..+80h.
void hud_marker_entry_hide_all_006374b0(Raw entry, HudScreenHost& host);
// 00637540: hide +4h, +8h, +0Ch, +14h, +10h in that order.
void hud_colour_icon_set_hide_all_00637540(Raw set, HudScreenHost& host);
// 00638B20, __thiscall(marker, float out_a[3], float out_b[3]), RET 8: copy the
// two points +20h and +24h point at.
void hud_marker_endpoints_00638b20(Raw marker, float out_a[3], float out_b[3]) noexcept;
void hud_colour_icon_set_destruct_006391c0(Raw set, HudScreenHost& host);
void hud_name_marker_destruct_00639450(Raw marker, HudScreenHost& host);
void hud_befogo_marker_destruct_00639960(Raw marker, HudScreenHost& host);
// 0063E1C0 / 0063E950, __thiscall(marker, page), RET 4: find the template child
// in the page (00AA7E00) and keep a clone of it (00AAB4C0, ECX = child, (0)).
struct HudMarkerCloneHost {
    virtual ~HudMarkerCloneHost() = default;
    virtual void* clone_subtree_00aab4c0(void* widget, std::uint32_t flags) = 0;
};
Raw hud_name_marker_construct_0063e1c0(Raw marker, void* page, HudScreenHost& host, HudMarkerCloneHost& clone);
Raw hud_befogo_marker_construct_0063e950(Raw marker, void* page, HudScreenHost& host, HudMarkerCloneHost& clone);
Raw hud_markers_construct_00640c80(Raw s, HudScreenHost& host, void* tree_head);

// ---------------------------------------------------------------------------
// Observer references: an object whose +14h holds one observed pointer.
// ---------------------------------------------------------------------------
// 0064B340 / 00656DE0 / 00656E20 (void) and 0064B610 / 0064BB50 (return this):
// __thiscall(ref, observed), RET 4. Unregister the old, store, register the new.
inline constexpr std::array<std::uint32_t, 5> kObserverRefSetters{
    0x0064B340, 0x00656DE0, 0x00656E20, 0x0064B610, 0x0064BB50};
Raw observer_ref_set(Raw ref, void* observed, HudScreenHost& host);
// 0064B590 / 0065AFA0 / 0065B040 / 0065B090: __thiscall(ref, released), RET 4.
// released->vtable[+4h]() yields the object; if it is +14h, clear and unregister.
inline constexpr std::array<std::uint32_t, 5> kObserverRefReleased{
    0x0064B590, 0x0065AFA0, 0x0065AFF0, 0x0065B040, 0x0065B090};
struct ObserverReleaseHost {
    virtual ~ObserverReleaseHost() = default;
    virtual void* released_object_04(void* notifier) = 0;
};
void observer_ref_released(Raw ref, void* notifier, ObserverReleaseHost& query, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Screen 46h (ship view; +14h, +18h observed, +1Ch unit, +20h camera).
// ---------------------------------------------------------------------------
void ship_view_register_0064a360(Raw s, HudScreenHost& host);
void ship_view_enter_0064a450(Raw s, HudScreenHost& host);
void ship_view_released_0064a4c0(Raw s, void* object) noexcept;
void ship_view_exit_0064a490(Raw s, HudScreenHost& host);

// 0064A4F0: a widget quad fade (eight cleared dwords, +20h = 1).
void hud_widget_quad_fade_construct_0064a4f0(Raw fade) noexcept;
// 0064A540, __thiscall(fade, w0, w1, w2, w3), RET 10h: store and hide all four.
void hud_widget_quad_fade_bind_0064a540(Raw fade, void* w0, void* w1, void* w2, void* w3,
    HudScreenHost& host);

// ---------------------------------------------------------------------------
// Small helpers of the root, rewards, support-manager and camera objects.
// ---------------------------------------------------------------------------
// 0061EB80, __fastcall(profile): the +C0h vector of 1Ch records is non-empty.
bool profile_has_pending_awards_0061eb80(Raw profile) noexcept;
// 00626720, ECX = the widget listener at screen+0Ch: recapture its three
// scroller blocks +5Ch, +190h, +11Ch (screen +68h, +19Ch, +128h).
void rewards_recapture_scrollers_00626720(Raw s, HudScreenHost& host);
// 00645F10: HUD root register.
void hud_root_register_00645f10(Raw s, HudScreenHost& host);
// 006529F0 / 00652A00: support manager +5Ch == 1 / == 2.
bool support_manager_is_mode1_006529f0(Raw s) noexcept;
bool support_manager_is_mode2_00652a00(Raw s) noexcept;
// 00652A60: delete the clones at +7Ch..+88h and +ACh..+B8h (vtable +4h(1)).
void support_manager_delete_slot_clones_00652a60(Raw s, HudScreenHost& host);
// 00652AA0 / 00652AD0 / 00652B00 / 00652B30 / 00652B60: widget visibility pairs.
void support_manager_show_arrows_f8_00652aa0(Raw s, HudScreenHost& host);
void support_manager_hide_arrows_f8_00652ad0(Raw s, HudScreenHost& host);
void support_manager_show_arrows_100_00652b00(Raw s, HudScreenHost& host);
void support_manager_hide_arrows_100_00652b30(Raw s, HudScreenHost& host);
void powerup_manager_hide_info_00652b60(Raw s, HudScreenHost& host);
// 0064B6B0: ShipCaptain camera mode name, "PTBoat" when its unit (+3ACh) is of
// kind 0Eh (vtable +5Ch), else "ShipCaptain".
struct UnitKindHost {
    virtual ~UnitKindHost() = default;
    virtual bool unit_is_kind_5c(void* unit, int kind) = 0;
};
std::string_view ship_captain_mode_name_0064b6b0(Raw camera, UnitKindHost& kinds);
// 0064B6D0: vtable +130h, the unit at +3ACh unless detached (+400h).
void* ship_captain_unit_0064b6d0(Raw camera) noexcept;
// 00644240: drop timed entry 0 of +3Ch and reset its +4Ch float to 0.
void ship_screen_clear_timed_entry_00644240(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Native strings (count +0h, data +4h; 0041DD40 resize, 00419CC0/00BD1510
// pool release) and the records built from them.
// ---------------------------------------------------------------------------
struct NativeStringHost {
    virtual ~NativeStringHost() = default;
    // Release a string whose data is non-null: pool(data, count + 1).return.
    virtual void release(Raw string) = 0;
    // dst != src: resize dst to src's count (0041DD40(count, 1)), copy count bytes.
    virtual void assign(Raw dst, Raw src) = 0;
    // 00BF6AD6 _free.
    virtual void free_block(void* block) = 0;
};

void native_string_release_at(Raw object, std::size_t off, NativeStringHost& host);
// 0061F210 / 00645FC0 (identical): release +8h, then +0h.
inline constexpr std::array<std::uint32_t, 2> kStringPairDestroy{0x0061F210, 0x00645FC0};
void string_pair_destroy(Raw pair, NativeStringHost& host);
// 0061F2A0 / 00623370 release the string at +8h; 0065CBC0 the one at +4h.
inline constexpr std::array<std::uint32_t, 2> kStringReleaseAt8{0x0061F2A0, 0x00623370};
// 0061F2C0: a 60h scoreboard row's two strings, +10h then +8h.
void rewards_score_row_destroy_0061f2c0(Raw row, NativeStringHost& host);
// 0061F4A0, __thiscall(dst, src) -> dst: a 1Ch bonus record (3 strings + int).
Raw profile_bonus_record_assign_0061f4a0(Raw dst, Raw src, NativeStringHost& host);
// 0061F540 / 00622790: the 60h scoreboard row assign and copy constructor.
Raw rewards_score_row_assign_0061f540(Raw dst, Raw src, NativeStringHost& host);
Raw rewards_score_row_copy_construct_00622790(Raw dst, Raw src, NativeStringHost& host);
// 00622870: the 10h record (byte, int, string) copy constructor.
Raw rewards_record10_copy_construct_00622870(Raw dst, Raw src, NativeStringHost& host);
// 00622F90, __thiscall(dst, string*, {string, int}*), RET 8.
Raw string_pair_int_construct_00622f90(Raw dst, Raw first, Raw second_and_int, NativeStringHost& host);
// 00646230 / 00646E20: string pair assign / copy constructor.
Raw string_pair_assign_00646230(Raw dst, Raw src, NativeStringHost& host);
Raw string_pair_copy_construct_00646e20(Raw dst, Raw src, NativeStringHost& host);
// 006462A0 copy, 00646350 copy_backward, 00646F20 fill over 10h string pairs.
std::uint8_t* string_pair_copy_006462a0(std::uint8_t* first, std::uint8_t* last, std::uint8_t* out, NativeStringHost& host);
std::uint8_t* string_pair_copy_backward_00646350(std::uint8_t* first, std::uint8_t* last, std::uint8_t* out_end, NativeStringHost& host);
void string_pair_fill_00646f20(std::uint8_t* first, std::uint8_t* last, Raw value, NativeStringHost& host);
// 00646170, __fastcall(out, count, value*): fill_n over dwords.
void fill_n_dwords_00646170(std::uint32_t* out, std::int32_t count, const std::uint32_t* value) noexcept;
// 00625EC0 (60h rows, 0061F2C0) / 00647C90 (10h pairs, 00645FC0): tidy.
void vector_tidy_destroy(Raw vector, std::size_t element_size, void (*destroy)(Raw, NativeStringHost&),
    NativeStringHost& host);

// ---------------------------------------------------------------------------
// Screen 04h, the post-mission rewards (scoring) screen; widget listener at
// +0Ch (vtable 00CF5098). Complements src/ingame_gui.cpp (update/enter).
// ---------------------------------------------------------------------------
inline constexpr float kRewardsAutoScrollSpeed = 0.004f;  // 00CEC178
struct RewardsScreenHost : HudScreenHost {
    virtual void handle_release(void* handle) = 0;          // InterlockedDecrement(+4h), slot 0 at zero
    virtual void sound_stop_08(void* sound) = 0;            // vtable +8h(0)
    virtual std::int32_t widget_type_5c(void* widget) = 0;  // vtable +5Ch
    virtual void widget_set_state_88(void* widget, std::int32_t state, std::int32_t b, float c) = 0;
    virtual void widget_set_state_84(void* widget, std::int32_t state) = 0;
    virtual std::uint8_t widget_name_first_byte_f0(void* widget) = 0;  // *(+F0h ? +F0h : 00E196AC)
    virtual void scroller_set_auto_scroll_00683300(Raw block, std::int32_t direction, float speed, std::int32_t on) = 0;
    virtual void scroller_end_thumb_drag_00683320(Raw block) = 0;
    virtual void scroller_begin_thumb_drag_00683a40(Raw block) = 0;
    virtual void rewards_continue_0062b110(Raw screen) = 0;
    virtual bool rewards_selected_entry_flagged_0061d8b0(Raw screen) = 0;
    virtual void rewards_step_page_00622e30(Raw screen, std::int32_t direction) = 0;
    virtual void rewards_erase_first_rank_006258a0(Raw vector) = 0;
    virtual void rewards_fill_rank_page_0062a970(Raw screen) = 0;
    virtual void rewards_refresh_page_00621c60(Raw screen, std::int32_t page) = 0;
    virtual std::uint64_t selected_row_xuid_0061d710(Raw screen) = 0;
    virtual std::uint64_t active_xuid_00a3eb00() = 0;
    // 00A3EAC0(xuid) gives the user slot; then XShowGamerCardUI / XShowPlayerReviewUI.
    virtual void show_gamer_card_00a4d440(std::uint64_t xuid) = 0;
    virtual void show_player_review_00a4d470(std::uint64_t xuid) = 0;
    // The online service *00F8A2FC.
    virtual bool online_signed_in_198() = 0;
    virtual std::uint8_t online_byte_4d() = 0;
    virtual bool online_virtual_bool(std::uint16_t slot) = 0;
    virtual void online_virtual_arg(std::uint16_t slot, std::int32_t arg) = 0;
    virtual void online_virtual_void(std::uint16_t slot) = 0;
    virtual std::uint8_t& game_byte(std::uint16_t off) = 0;   // *(00E188A8)+off
    virtual std::int32_t game_int(std::uint16_t off) = 0;
    virtual std::uint8_t& global_byte(std::uint32_t address) = 0;
    virtual std::int32_t& global_int(std::uint32_t address) = 0;
    virtual void mission_player_records_reset_00916980() = 0;  // ECX = *(game+21A0h)
    virtual bool mission_menu_active_3c() = 0;                 // *(*00E198AC + 3Ch)
    virtual void mission_menu_publish_selection_00580940() = 0;  // ECX = *(*00E198AC + 58h)
    virtual void push_interface_request_004cc460(std::int32_t request, std::int32_t arg) = 0;
    virtual void session_set_mode_0076fad0(std::int32_t mode) = 0;  // ECX = game+1EF0h
    virtual void multi_menu_push_request_00688ad0(std::int32_t request) = 0;  // ECX = *00E198B4
    virtual bool xenon_has_selected_user_00a3e510() = 0;         // ECX = *00F8ABE8
    virtual std::int32_t xenon_selected_sign_in_state_00a3ead0() = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    virtual void rewards_play_test_speech_0061f9d0(void* file) = 0;  // ECX = *(*00E198AC + 60h)
    // 004FC150 (file-store factory), 00BE80B0(file) then 00BE7130.
    virtual void file_store_remove_00be7130(void* file) = 0;
    virtual void scroller_construct_00683610(Raw block) = 0;
    virtual void record10_destroy_range_006243c0(Raw vector) = 0;  // ECX begin, EDX end, (vector, screen)
    virtual void score_row_vector_tidy_00625a00(Raw vector) = 0;
};

// ---------------------------------------------------------------------------
// Exits and destructors that free containers.
// ---------------------------------------------------------------------------
struct ScreenTeardownHost : HudScreenHost {
    virtual void free_block(void* block) = 0;                    // 00BF65AC
    virtual void object_delete_00(void* object) = 0;             // vtable slot 0 (1)
    virtual void handle_release(void* handle) = 0;               // InterlockedDecrement(+4h), slot 0 at zero
    virtual void tree_erase_all_0063fb30(Raw tree) = 0;          // erase(begin, end) of the map at tree
    virtual void target_observer_destruct_0042bca0(Raw observer) = 0;
    virtual void string_pair_vector_tidy_00647ad0(Raw vector) = 0;
    virtual void render_set_sight_blur_00b0cf80(float a, float b) = 0;  // ECX = *00F8D39C
    virtual void widget_set_colour_00aa85b0(void* widget, const float rgba[4]) = 0;
};
void hud_markers_exit_0063e360(Raw s, ScreenTeardownHost& host);
void hud_markers_destruct_006404a0(Raw s, ScreenTeardownHost& host);
// 00640D50, markers scalar deleting destructor: __thiscall(s, flags), RET 4 -> EAX s.
Raw hud_markers_scalar_delete_00640d50(Raw s, std::uint8_t flags, ScreenTeardownHost& host);
// 006405F0, the kTrivialSlots Adjustor row as code: SUB ECX,8; JMP 00640D50.
// `secondary` is the +8h base sub-object; returns the full object (00640D50's EAX).
Raw hud_markers_scalar_delete_adjustor8_006405f0(Raw secondary, std::uint8_t flags,
                                                 ScreenTeardownHost& host);
void hud_root_destruct_00647db0(Raw s, ScreenTeardownHost& host, NativeStringHost& strings);
void ship_status_exit_0064d8d0(Raw s, ScreenTeardownHost& host);
void periscope_destruct_00650970(Raw s, ScreenTeardownHost& host);
void periscope_load_layout_00651230(Raw s, HudScreenHost& host);
void submarine_load_layout_00650b90(Raw s, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Batch 2 hosts and routines.
// ---------------------------------------------------------------------------
struct MarkerPool {
    std::uint32_t acquire;      // the routine
    std::uint16_t vector_off;   // the vector object (begin at +4h, end at +8h)
    std::uint16_t cursor_off;   // the hand-out cursor
    std::uint16_t alloc_size;   // operator new size of a fresh marker
    std::uint32_t constructor;  // __thiscall(marker, page)
    std::uint32_t push_back;    // ECX = the vector, (&marker)
};
inline constexpr MarkerPool kObjectiveMarkerPool{0x00640860, 0x5C, 0x80, 0x18, 0x0063DEC0, 0x00640250};
inline constexpr MarkerPool kCommandUnitMarkerPool{0x00640940, 0x6C, 0x84, 0x08, 0x0063E1C0, 0x006402C0};
struct MarkerPoolHost {
    virtual ~MarkerPoolHost() = default;
    // operator new(alloc_size) then the constructor with the page; null on failure.
    virtual void* make_marker(const MarkerPool& pool, void* page) = 0;
    virtual void push_marker(const MarkerPool& pool, Raw vector, void* marker) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
};
void* hud_markers_acquire_pooled_marker(const MarkerPool& pool, Raw s, MarkerPoolHost& host);

struct UnitIconHost {
    virtual ~UnitIconHost() = default;
    virtual bool unit_is_kind_5c(void* unit, int kind) = 0;
    virtual int plane_ordnance_icon_kind_007bca50(void* plane) = 0;
};
int hud_unit_icon_index_00644ae0(Raw unit, UnitIconHost& host);

struct UnitSelectHost {
    virtual ~UnitSelectHost() = default;
    virtual bool unit_selectable_00645060(void* unit, std::int32_t slot, int flag) = 0;
    virtual std::int32_t local_player_slot() = 0;          // game+18ECh
    virtual std::int32_t local_player_record_28() = 0;     // *(game+18CCh + 4*slot) + 28h
    virtual std::int32_t session_mode() = 0;               // game+1FE4h
};
bool unit_selectable_for_current_player_00645160(void* unit, int flag, UnitSelectHost& host);
bool unit_is_local_player_unit_00645180(Raw unit, UnitSelectHost& host);

struct ShipScreenWidgetHost : HudScreenHost {
    virtual void widget_resolved_position_00aa6750(void* widget, float out[3]) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float pos[3]) = 0;
    virtual void ship_screen_apply_unit_0064ae20(Raw screen) = 0;
};
void ship_screen_trigger_unit_ping_0064a8e0(Raw s, ShipScreenWidgetHost& host);
void ship_screen_set_unit_0064d590(Raw s, void* unit, ShipScreenWidgetHost& host);

struct PressStartStorageHost {
    virtual ~PressStartStorageHost() = default;
    virtual std::uint32_t storage_request_08() = 0;          // *(*0109CECC + 8)
    virtual void menu_prompts_dismiss_all_00530650() = 0;    // ECX = 00425D10()
    virtual void title_skip_0068d8a0() = 0;
};
void press_start_on_storage_complete_0067ca40(PressStartStorageHost& host);

Raw hud_colour_icon_set_construct_0063dec0(Raw set, void* page, HudScreenHost& host, HudMarkerCloneHost& clone);
void hud_unit_marker_widgets_destruct_00638f50(Raw w, HudScreenHost& host);
void text_owner_set_ellipsized_00646d30(Raw owner, const char* text, HudScreenHost& host);
void hud_root_set_interface_00646040(Raw s, std::uint32_t iface, void* observed, HudScreenHost& host);
void periscope_attach_unit_00650210(Raw s, HudScreenHost& host);
void periscope_detach_unit_006506e0(Raw s, HudScreenHost& host);
void periscope_enter_00650750(Raw s, HudScreenHost& host);
void unit_view_bind_unit_and_reset_00651760(Raw s, void* unit, HudScreenHost& host);

// ---------------------------------------------------------------------------
// Batch 3: observer-reference objects and support-manager records.
// ---------------------------------------------------------------------------
// 00653400 (vtable 00CF64AC) / 006569A0 (00CF6494).
void observer_ref_destruct(Raw ref, std::uint32_t vtable, HudScreenHost& host);
// 00658710 (00CF6494) / 006587C0 (00CF64AC).
Raw observer_ref_copy_construct(Raw ref, Raw src, std::uint32_t vtable, HudScreenHost& host);
void unit_slot_entry_destroy_0065bb00(Raw e, ScreenTeardownHost& host);
struct RecordAssignHost : HudScreenHost {
    virtual void legacy_string_assign_00408120(Raw dst, Raw src) = 0;   // ECX = dst, (src, 0, -1)
    virtual void legacy_string_range_destroy_004ce520(void* first, void* last) = 0;
    virtual void free_block(void* block) = 0;                           // 00BF65AC
    virtual void tree_erase_all_006641b0(Raw tree) = 0;
    virtual void tree_copy_0065ce20(Raw dst_tree, Raw src_tree) = 0;
};
Raw observer_string_record_assign_006626e0(Raw dst, Raw src, RecordAssignHost& host);
void string_vector_holder_destruct_006683d0(Raw h, RecordAssignHost& host);
Raw int_map_record_assign_00669350(Raw dst, Raw src, RecordAssignHost& host);
Raw int_string_int_record_copy_construct_00660540(Raw dst, Raw src, NativeStringHost& host);
Raw slot_record_assign_0065ca50(Raw dst, Raw src, HudScreenHost& host);
Raw slot_record_copy_construct_0065caf0(Raw dst, Raw src, HudScreenHost& host);
bool slot_record_is_out_00658530(Raw r) noexcept;
struct TrackedTargetHost : HudScreenHost {
    virtual bool unit_is_kind_5c(void* unit, int kind) = 0;
    virtual void* unit_target_140(void* unit) = 0;   // vtable +140h
};
void support_manager_set_tracked_unit_target_0065be80(Raw s, void* unit, TrackedTargetHost& host);
struct SupportEntriesHost {
    virtual ~SupportEntriesHost() = default;
    virtual std::int32_t game_effective_mode_004bca50() = 0;
    virtual std::int32_t game_counter_648() = 0;      // *(00E188A8)+648h
    virtual void support_refresh_0066fb00(Raw screen) = 0;
    virtual void support_rebuild_00668770(Raw screen) = 0;
    virtual void support_sync_cursor_0065a2b0(Raw screen) = 0;
};
bool support_manager_has_available_entries_0066ffa0(Raw s, SupportEntriesHost& host);

// ---------------------------------------------------------------------------
// Batch 4: unit library (05h, vtable 00CF6E50; 3D scene at +8h, listeners at
// +64h/+68h, page +6Ch), press start, text range fitting.
// ---------------------------------------------------------------------------
struct UnitLibraryHost : HudScreenHost {
    virtual void simple3d_scene_construct_0093cb10(Raw scene) = 0;
    virtual void simple3d_scene_destruct_0093cc80(Raw scene) = 0;
    virtual void free_block(void* block) = 0;
    virtual void handle_release(void* handle) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void gui_movie_stop_00aac870(void* movie) = 0;
    virtual void main_menu_start_title_music_005884a0() = 0;   // ECX = *(*00E198AC + 58h)
    virtual void profile_save_then_call_007fa1b0(int a, std::uint32_t continuation) = 0;  // ECX = game
};
Raw unit_library_construct_00680c20(Raw s, UnitLibraryHost& host);
void unit_library_release_model_parts_0067ed30(Raw s, UnitLibraryHost& host);
void unit_library_destruct_0067ef50(Raw s, UnitLibraryHost& host);
void unit_library_exit_0067ded0(Raw s, UnitLibraryHost& host);
void unit_library_reset_selection_0067e580(Raw s, UnitLibraryHost& host);
struct DeviceButtonHost {
    virtual ~DeviceButtonHost() = default;
    virtual bool button_held_20(void* device, int button) = 0;   // vtable +20h
};
bool press_start_device_button_held_0067c880(void* device, DeviceButtonHost& host);
struct InviteHost {
    virtual ~InviteHost() = default;
    virtual void* pending_invite_00e19880() = 0;
    virtual bool xenon_has_selected_user_00a3e510() = 0;       // ECX = *00F8ABE8
    virtual std::int32_t xenon_slot_for_invitee_00a3e470() = 0;
    virtual std::int32_t xenon_active_user_slot_00a3eac0() = 0;
};
bool game_sign_in_state_valid_0067c8f0(InviteHost& host);
struct TextRangeHost {
    virtual ~TextRangeHost() = default;
    // 00469840 (substring), 004261A0 (concat), 00ABAED0(&s, 0), releases.
    virtual void text_set_prefix_plus_substring(void* text, Raw prefix, Raw src, std::int32_t first,
        std::int32_t count) = 0;
    virtual void text_position_cursor_00ab9240(void* text, std::int32_t position) = 0;
    virtual std::int32_t text_line_count_110(void* text) = 0;   // +110h
};
void gui_text_set_source_range_0067d870(void* text, Raw prefix, Raw src, std::int32_t cursor_base,
    std::int32_t first, std::int32_t last, TextRangeHost& host);
void gui_text_fit_range_to_one_line_0067d940(void* text, Raw prefix, Raw src, std::int32_t cursor_base,
    TextRangeHost& host);

// ---------------------------------------------------------------------------
// Batch 5: layout binders.
// ---------------------------------------------------------------------------
struct LayoutHost : HudScreenHost {
    // 00871BA0(&out, name, 1): an effect handle by name (refcount owned by the caller).
    virtual void* effect_acquire_by_name_00871ba0(std::string_view name) = 0;
    // Store a refcounted handle: InterlockedIncrement the new one, decrement the
    // old one (slot 0 at zero), then drop the caller's temporary reference.
    virtual void handle_assign_acquired(Raw slot, void* handle) = 0;
    virtual void widget_resolved_position_00aa6750(void* widget, float out[3]) = 0;
    virtual void widget_set_resolved_position_00aa8240(void* widget, const float pos[3]) = 0;
    virtual const float* widget_vector_00aa67f0(void* widget) = 0;
    virtual void widget_set_scale_00aa86b0(void* widget, float x, float y) = 0;
    virtual void support_rebind_slots_0065d220(Raw screen) = 0;
};
void periscope_register_00650e00(Raw s, LayoutHost& host);
void hud_root_load_layout_006463e0(Raw s, LayoutHost& host);
void ship_status_load_layout_0064c0f0(Raw s, LayoutHost& host);
void support_manager_load_layout_0065daa0(Raw s, LayoutHost& host);

// ---------------------------------------------------------------------------
// Batch 6: support-manager fades and icon frames.
// ---------------------------------------------------------------------------
struct FadeHost : HudScreenHost {
    virtual void widget_set_alpha_recursive_00aa8610(void* widget, float alpha) = 0;
    virtual void widget_set_size_58(void* widget, const float size[2]) = 0;   // vtable +58h
    virtual void support_close_finish_00670020(Raw screen) = 0;
    virtual void support_layout_name_label_0065bbc0(Raw screen) = 0;
};
void support_manager_step_close_fade_006723f0(Raw s, float dt, FadeHost& host);
void support_manager_update_name_label_fade_0065bd90(Raw s, float dt, FadeHost& host);
struct IconFrameHost {
    virtual ~IconFrameHost() = default;
    virtual bool unit_has_kind_18(void* unit, int kind) = 0;      // vtable +18h
    virtual std::uint32_t unit_type_174(void* unit) = 0;          // vtable +174h, a type address
};
int hud_unit_icon_frame_00652ba0(Raw slot, IconFrameHost& host);
int support_manager_record_icon_frame_00658620(Raw s, Raw record, IconFrameHost& host);

// ---------------------------------------------------------------------------
// fe2b: the four large layout binders.
// ---------------------------------------------------------------------------
struct RewardsLayoutHost : LayoutHost {
    // *(cell): a char* the image keeps in .data (the page-group names).
    virtual const char* global_cstr(std::uint32_t cell) = 0;
    virtual const float* widget_vector_00aa6740(void* widget) = 0;
    virtual void scroller_bind_00683790(Raw block, void* move_group) = 0;          // ECX = block
    virtual void scroller_setup_00683380(Raw block, void* thumb, float track, void* up, void* down) = 0;
    virtual void widget_set_listener_00aa6bc0(void* widget, void* listener, int flag) = 0;  // ECX = widget
    // 00531030(&tmp, action): the key name of an action ("" at 00E196AA when empty).
    virtual std::string_view action_key_name_00531030(int action) = 0;
    // 004C5E60 widens the key; 00531130(ECX = text, &wide, listener, text).
    virtual void text_set_key_glyph_00531130(void* text, std::string_view key, void* listener) = 0;
    virtual void text_refresh_00ab80c0(void* text) = 0;
    virtual void text_set_localised_source_00abaed0(void* text, std::string_view source, bool flag) = 0;
};
void rewards_bind_layout_0061fbe0(Raw s, RewardsLayoutHost& host);
struct RewardsRegisterHost : HudScreenHost {
    // The checked vector of native strings at `vector` (a global): erase all / push_back.
    virtual void string_vector_clear_004954f0(std::uint32_t vector) = 0;
    virtual void string_vector_append_00450540(std::uint32_t vector, std::string_view value) = 0;
};
void rewards_register_00629d10(Raw s, RewardsRegisterHost& host);
struct ShipStatusRegisterHost : LayoutHost {
    virtual void* effect_acquire_00870cd0(int id) = 0;   // ECX = &out, EDX = id, (1)
    // The sound-sample cache *00F8BBE8 (SoundSampleCache_AcquireByName).
    virtual void* sound_sample_acquire_by_name(std::string_view name) = 0;
};
void ship_status_register_0064bb90(Raw s, ShipStatusRegisterHost& host);
struct UnitLibraryHdr;
struct UnitLibraryRegisterHost : HudScreenHost {
    virtual void simple3d_scene_init_0093ccc0(Raw scene) = 0;
    virtual void camera_set_render_mode_00b6fdf0(void* camera, int mode) = 0;
    virtual void camera_set_clear_flags_00b6fe10(void* camera, int flags) = 0;
    // 00B72110 on the scene, its +1Ch light list, first node +8h.
    virtual void* scene_first_directional_light(void* scene) = 0;
    virtual void light_set_base_diffuse_004b62e0(void* light, const float rgba[4]) = 0;
    virtual void scene_set_ambient_00b7af20(void* scene, const float rgba[4]) = 0;  // ECX = lighting owner +10h
    virtual void render_flag_hook_f8bbf0_04() = 0;          // (*00F8BBF0)[+4h](byte 00F88A04)
    virtual void render_resources_init_00b107f0() = 0;      // ECX = *00F8D39C, (00F88A0C, 00F88A0D, 00F889D8)
    virtual void scene_renderer_set_motion_blur_00b0ea20(const float params[8]) = 0;
    virtual void scene_renderer_set_hdr_00b0e9b0(const UnitLibraryHdr& params) = 0;
    virtual void light_attach_new_shadow_owner(void* light) = 0;   // 00A8FD30, 00B7BDF0, drop the temporary
    virtual void lua_run_script_00b69d40(std::string_view path) = 0;
};
void unit_library_register_0067f080(Raw s, UnitLibraryRegisterHost& host);

// ---------------------------------------------------------------------------
// fe2b: session-message classes in segments 26/27 (types 82h, 94h, A0h, A1h,
// A7h..A9h). Layout of the shared base: vtable +0h, +4h = 1, +18h word, +1Ah
// byte; the payload starts at +1Ch.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kSessionMessageRootVtable = 0x00CE4974;
enum class SessionFieldKind : std::uint8_t { Bool, Signed, Unsigned };
struct SessionField {
    std::uint8_t offset;
    SessionFieldKind kind;
    std::uint8_t bits;  // Signed / Unsigned
};
enum class SessionCtorShape : std::uint8_t {
    None,                 // no constructor in these segments
    Dword1C,              // (int) into +1Ch
    Clear1C,              // +1Ch byte 0
    Clear1CByte20,        // +1Ch byte 0, (byte) into +20h
    Clear1CDword20Byte24  // +1Ch byte 0, (int) into +20h, (byte) into +24h after the vtable store
};
struct SessionMessageClass {
    std::uint8_t type;
    std::array<std::uint8_t, 2> bases;  // accepted base types of IsType
    std::uint8_t base_count;
    std::uint32_t vtable;
    SessionCtorShape ctor;
    std::uint32_t construct, is_type, write, read, scalar_delete;  // 0 = not in these segments
    std::array<SessionField, 3> fields;
    std::uint8_t field_count;
};
inline constexpr std::array<SessionMessageClass, 7> kSessionMessageClasses{{
    {0xA0, {0x46, 0}, 1, 0x00CF5C38, SessionCtorShape::Dword1C, 0x0064A770, 0x0064A7A0, 0x0064A7C0, 0x0064A7E0, 0x0064ADE0,
     {{{0x1C, SessionFieldKind::Signed, 4}}}, 1},
    {0x94, {0x49, 0x46}, 2, 0x00CF5C4C, SessionCtorShape::Clear1C, 0x0064A820, 0x0064A850, 0, 0, 0x0064AE00, {}, 0},
    {0xA1, {0x49, 0x46}, 2, 0x00CF61FC, SessionCtorShape::Clear1CByte20, 0x00650310, 0x00650360, 0x00650390, 0x006503C0, 0x00650430,
     {{{0x1C, SessionFieldKind::Bool, 0}, {0x20, SessionFieldKind::Bool, 0}}}, 2},
    {0x82, {0x49, 0x46}, 2, 0, SessionCtorShape::None, 0, 0x006562F0, 0, 0x006563A0, 0x00656420, {}, 0},
    {0xA7, {0x49, 0x46}, 2, 0, SessionCtorShape::None, 0, 0x006564C0, 0x006564F0, 0x00656530, 0x00656770,
     {{{0x1C, SessionFieldKind::Bool, 0}, {0x20, SessionFieldKind::Unsigned, 2}, {0x24, SessionFieldKind::Bool, 0}}}, 3},
    {0xA8, {0x49, 0x46}, 2, 0x00CF64EC, SessionCtorShape::Clear1CDword20Byte24, 0x00656570, 0x006565C0, 0x006565F0, 0x00656630, 0x00656790,
     {{{0x1C, SessionFieldKind::Bool, 0}, {0x20, SessionFieldKind::Unsigned, 2}, {0x24, SessionFieldKind::Bool, 0}}}, 3},
    {0xA9, {0x49, 0x46}, 2, 0x00CF6500, SessionCtorShape::Clear1CDword20Byte24, 0x00656670, 0x006566C0, 0x006566F0, 0x00656730, 0x006567B0,
     {{{0x1C, SessionFieldKind::Bool, 0}, {0x20, SessionFieldKind::Unsigned, 2}, {0x24, SessionFieldKind::Bool, 0}}}, 3},
}};
struct SessionMessageHost {
    virtual ~SessionMessageHost() = default;
    virtual void construct_base_0075b430(Raw message, std::uint8_t type) = 0;
    virtual void write_header_0075b480(Raw message, void* cursor) = 0;
    virtual void read_header_0075b4c0(Raw message, void* cursor) = 0;
    // Bit cursor. Writes take ECX = the cursor passed to Write (00650390:
    // `mov ecx,edi`). Reads take ECX = that cursor + 4h, a sub-object of the
    // object passed to Read (006503C0: `add edi,4; mov ecx,edi`; 00605300 and
    // 0064A7E0: `lea ecx,[edi+4]`; 00656530 and 006563A0 the same for 00428D10
    // and 00428E30); the header read 0075B4C0 takes the unadjusted pointer.
    virtual void write_bool_004290b0(void* cursor, std::uint8_t value) = 0;
    virtual void write_signed_00429090(void* cursor, std::int32_t value, int bits) = 0;
    virtual void write_unsigned_00429070(void* cursor, std::uint32_t value, int bits) = 0;
    virtual void read_bool_00428d70(void* cursor, std::uint8_t& out) = 0;
    virtual void read_signed_00428d30(void* cursor, std::int32_t& out, int bits) = 0;
    virtual void read_unsigned_00428d10(void* cursor, std::uint32_t& out, int bits) = 0;
    virtual void read_word_bits_00428e30(void* cursor, std::uint16_t& out, int bits) = 0;
    virtual void free_block(void* block) = 0;
};
const SessionMessageClass* find_session_message_class(std::uint8_t type) noexcept;
// IsType, __stdcall(int type), RET 4 (no ECX use).
bool session_message_is_type(const SessionMessageClass& c, std::int32_t type) noexcept;
// Constructors: __thiscall(msg[, arg0[, arg1]]) -> EAX msg.
Raw session_message_construct(const SessionMessageClass& c, Raw m, std::uint32_t arg0, std::uint8_t arg1,
    SessionMessageHost& host);
// Write / Read: __thiscall(msg, cursor), RET 4.
void session_message_write(const SessionMessageClass& c, Raw m, void* cursor, SessionMessageHost& host);
void session_message_read(const SessionMessageClass& c, Raw m, void* cursor, SessionMessageHost& host);
void session_message130_read_006563a0(Raw m, void* cursor, SessionMessageHost& host);
// Scalar deleting destructors: __thiscall(msg, flags), RET 4 -> EAX msg.
Raw session_message_scalar_delete(Raw m, std::uint8_t flags, SessionMessageHost& host);

void rewards_release_sound_handles_0061ec00(Raw s, RewardsScreenHost& host);
bool rewards_selected_entry_flagged_0061d8b0(Raw s, RewardsScreenHost& host);
void rewards_play_and_remove_file_00622e00(void* file, RewardsScreenHost& host);
Raw rewards_construct_00626630(Raw s, RewardsScreenHost& host);
void rewards_destruct_00626770(Raw s, RewardsScreenHost& host, NativeStringHost& strings);
void rewards_widget_release_0061d770(Raw listener, void* widget, RewardsScreenHost& host);
void rewards_widget_press_0062b3b0(Raw listener, void* widget, RewardsScreenHost& host);
void rewards_exit_00621720(Raw s, RewardsScreenHost& host);
void rewards_continue_0062b110(Raw s, RewardsScreenHost& host);

// ---------------------------------------------------------------------------
// cc13-fe10 additions (docs/CC13_FE_10.md).
// ---------------------------------------------------------------------------
struct Fe10GuiHost : RewardsLayoutHost {
    // Static storage of the image at that address (a block, not a pointer cell).
    virtual Raw global_block(std::uint32_t address) = 0;
    // Sound: 00A83FD0, ECX = *00F8BBE8, (&out, const NativeString*): a sample
    // handle by name (one reference owned by the caller); ECX = *00F8BBD8:
    // 00A7ACF0(const NativeString*) class by name, 00A7B0A0(&name, 1) type by
    // name, 00A7E490(&out, owner, class, type) a tracked sound (one reference).
    virtual void* sound_sample_acquire_00a83fd0(Raw name) = 0;
    virtual void* sound_find_class_00a7acf0(std::string_view name) = 0;
    virtual void* sound_find_type_00a7b0a0(std::string_view name) = 0;
    virtual void* sound_create_tracked_00a7e490(void* owner, void* sound_class, void* sound_type) = 0;
    // The rewards routines this pass reaches by call: 0061D8B0 (reconstructed
    // as rewards_selected_entry_flagged_0061d8b0) and 0061D710, ECX = screen,
    // the selected entry's XUID (EDX:EAX).
    virtual bool rewards_selected_entry_flagged_0061d8b0(Raw screen) = 0;
    virtual std::uint64_t rewards_selected_entry_xuid_0061d710(Raw screen) = 0;
    // 00A3EB00, ECX = *00F8ABE8: the active profile's XUID.
    virtual std::uint64_t xenon_active_xuid_00a3eb00() = 0;
    // 0054B530, ECX = *00E1930C: the command bar from five triples.
    virtual void command_bar_rebuild_0054b530(const MainMenuCommandArgument (&arguments)[5]) = 0;
    // Lua objects are 14h-byte image objects the caller owns (storage passed
    // in): 00B65F50 construct, 00B67700 destruct, 00B67690 assign (ECX = dst),
    // 00B67980(&out), ECX = *00E188A8 + 1A0Ch: the globals table; 00B67800
    // (&out, name), ECX = table: a field; predicates 00B661B0 table, 00B66000
    // boolean, 00B65FB0 nil, 00B660A0 string; getters 00B66250 boolean,
    // 00B66290 integer, 00B662B0 string; 00B67080 / 00B67190 iterate first /
    // next (ECX = table, &key, &value), 00B66420 ended (ECX = table, &key).
    virtual void lua_construct_00b65f50(void* object) = 0;
    virtual void lua_destruct_00b67700(void* object) = 0;
    virtual void lua_assign_00b67690(void* dst, void* src) = 0;
    virtual void lua_globals_00b67980(void* out) = 0;
    virtual void lua_get_by_name_00b67800(void* table, void* out, const char* name) = 0;
    virtual bool lua_is_table_00b661b0(void* object) = 0;
    virtual bool lua_is_boolean_00b66000(void* object) = 0;
    virtual bool lua_is_nil_00b65fb0(void* object) = 0;
    virtual bool lua_is_string_00b660a0(void* object) = 0;
    virtual bool lua_get_boolean_00b66250(void* object) = 0;
    virtual std::int32_t lua_get_integer_00b66290(void* object) = 0;
    virtual const char* lua_get_string_00b662b0(void* object) = 0;
    virtual void lua_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    virtual bool lua_iterate_ended_00b66420(void* table, void* key) = 0;
    // Native strings: 0041E870 assign a C string, 00426060 copy-construct,
    // 004FB1F0 upper-case in place (ASCII), 0041E350 assign a C string (ECX =
    // dst), the pool release of the buffer.
    virtual void native_string_assign_cstr_0041e870(Raw dst, const char* text) = 0;
    virtual void native_string_copy_construct_00426060(Raw dst, Raw src) = 0;
    virtual void native_string_upper_ascii_004fb1f0(Raw string) = 0;
    virtual void native_string_assign_cstr_0041e350(Raw dst, const char* text) = 0;
    virtual void native_string_release(Raw string) = 0;
    // 0061C960(index), ECX = vector of 10h-byte records: checked operator[].
    virtual Raw record10_vector_at_0061c960(Raw vector, std::int32_t index) = 0;
    // 00ABAED0(const NativeString*, flag), ECX = text.
    virtual void text_set_localised_source_native_00abaed0(void* text, Raw source, bool flag) = 0;
    // 00ABBE50(const char*, 1), ECX = text.
    virtual void text_set_localised_cstr_00abbe50(void* text, const char* key) = 0;
    // 004FB150(id, widget), ECX = [*00E198AC+70h] (the tactical library's icon atlas).
    virtual void icon_atlas_apply_004fb150(std::int32_t id, void* widget) = 0;
    // 006278D0(const record*), ECX = vector of 10h-byte records: push_back.
    virtual void record10_vector_push_back_006278d0(Raw vector, Raw record) = 0;
    // 00574050, ECX = *00E188A8 + 650h: the rank setting (AL).
    virtual std::uint8_t session_filter_rank_setting_00574050() = 0;
    // Widget vtable +88h(state, 0, 1.0f).
    virtual void widget_virtual_88(void* widget, std::int32_t state, std::int32_t b, float c) = 0;
    // 0041DD40(dst, src.count, 1) then _memcpy of dst.count bytes when src.count != 0.
    virtual void native_string_resize_copy_0041dd40(Raw dst, Raw src) = 0;
    // 00AA7D00(x, y), ECX = widget: local x/y and bounds.
    virtual void widget_set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
    // Units and poses: 00414DB0, ECX = entity: refresh the world pose (when byte
    // +C8h is clear); unit vtable +5Ch(kind) kind test; 00852860, ECX = unit:
    // below a third of its altitude band; 004B4B00: the controlled vehicle;
    // 00927F30(role), ECX = unit: local player role; 00804350(range), ECX =
    // observer, EDX = target: sensors detect.
    virtual void entity_refresh_world_00414db0(void* entity) = 0;
    virtual bool unit_has_kind_5c(void* unit, std::int32_t kind) = 0;
    virtual bool entity_below_altitude_third_00852860(void* unit) = 0;
    virtual void* game_controlled_vehicle_004b4b00() = 0;
    virtual bool unit_is_local_player_role_00927f30(void* unit, std::int32_t role) = 0;
    virtual bool unit_sensors_detect_00804350(void* observer, void* target, std::int32_t range) = 0;
    // 0042B2F0, ECX = vec3: length (ST0).
    virtual double vector_length_0042b2f0(const float v[3]) = 0;
    // 0043A660(mode, flag), ECX = &world, EDX = &screen out.
    virtual void camera_project_0043a660(const float world[3], float out[3], std::int32_t mode, std::int32_t flag) = 0;
    // 00B6DB70, ECX = transform: refresh the world matrix (bit 2 of +5Ch clear).
    virtual void transform_refresh_world_00b6db70(void* transform) = 0;
    // _CIatan2 (00BF701A) on (ST1, ST0).
    virtual double crt_atan2_00bf701a(double st1, double st0) = 0;
    // Markers screen routines (ECX = screen): 0063ABD0(unit) already marked,
    // 006430C0(unit, a, b, c) add a unit marker; 00638E50 (ECX = &world, EDX =
    // &screen) project and clip -> AL; 005220C0(unit), ECX = [*00E198C4+CCh].
    virtual bool markers_contains_unit_0063abd0(Raw screen, void* unit) = 0;
    virtual void markers_add_unit_marker_006430c0(Raw screen, void* unit, std::int32_t a, std::int32_t b, std::int32_t c) = 0;
    virtual bool marker_project_and_clip_00638e50(const float world[3], float out[3]) = 0;
    virtual bool interface_cc_accepts_unit_005220c0(void* unit) = 0;
    // 0063FFC0(n, value), ECX = pointer vector: resize.
    virtual void pointer_vector_resize_0063ffc0(Raw vector, std::uint32_t n, void* value) = 0;
    // 005AAAF0(src), ECX = dst list: std::list<int> operator=; 00665D20(first,
    // last, tag), ECX = dst list: assign the checked-iterator range [begin, end)
    // of src (each iterator is the pair {container, node}).
    virtual void int_list_assign_005aaaf0(Raw dst, Raw src) = 0;
    virtual void int_list_assign_range_00665d20(Raw dst, Raw src) = 0;
    // 0067D5A0(text, count), ECX = wide string: assign count characters.
    virtual void wide_string_assign_n_0067d5a0(Raw dst, const std::uint16_t* text, std::uint32_t count) = 0;
    // 00638B90, ECX = &x, EDX = &y: clamp a marker position to the screen margins.
    virtual void marker_clamp_to_margins_00638b90(float& x, float& y) = 0;
    // Widget vtable +44h(float): rotation.
    virtual void widget_set_rotation_44(void* widget, float angle) = 0;
    // Widget vtable +58h(const vec2*): size.
    virtual void widget_set_size_58(void* widget, const float size[2]) = 0;
    // Entity vtable +18h -> its transform; transform vtable +4Ch(&box) ->
    // the local bounds {min xyz, max xyz}; 004142E0(&out, matrix), ECX = point:
    // affine transform (returns &out).
    virtual void* entity_transform_18(void* entity) = 0;
    virtual void transform_local_bounds_4c(void* transform, float box[6]) = 0;
    virtual const float* vector_transform_affine_004142e0(const float point[3], float out[3], const void* matrix) = 0;
    // Award table *00E19900: 006B9FD0 / 006B9F70(&out, const NativeString* key)
    // copy the record's +00h / +14h string (the out string is the caller's).
    virtual void award_table_text_006b9fd0(Raw out, Raw key) = 0;
    virtual void award_table_icon_006b9f70(Raw out, Raw key) = 0;
    // String-pair vector (10h elements): 00648A20 push_back(const pair*),
    // 00647CA0 erase(begin) (ECX = vector, (&out iterator, first iterator));
    // 00645FC0, ECX = pair: destroy both strings.
    virtual void string_pair_vector_push_back_00648a20(Raw vector, Raw pair) = 0;
    virtual void string_pair_vector_erase_front_00647ca0(Raw vector) = 0;
    virtual void string_pair_destroy_00645fc0(Raw pair) = 0;
    // 004CAD40(search, replacement, count), ECX = native string.
    virtual void string_replace_substrings_004cad40(Raw target, std::string_view search, std::string_view replacement,
                                                    std::uint32_t count) = 0;
    // 00AB44B0(state, const NativeString* name, const float uv[4]), ECX = widget.
    virtual void gui_set_state_texture_00ab44b0(void* widget, std::int32_t state, Raw name, const float uv[4]) = 0;
    // 00AA8B00(index), ECX = widget -> timed entry; 00AC2F20(a, b), ECX = entry.
    virtual Raw widget_timed_entry_00aa8b00(void* widget, std::int32_t index) = 0;
    virtual void ticker_set_range_00ac2f20(Raw entry, float a, float b) = 0;
    // 00436630(const wchar16*), ECX = out native string: narrow by low bytes.
    virtual void native_string_from_wide_00436630(Raw out, const std::uint16_t* text) = 0;
    // 00AA7DC0(const vec3*), ECX = widget: local position and bounds.
    virtual void widget_set_local_position_00aa7dc0(void* widget, const float xyz[3]) = 0;
    // 0065E870(widget, alpha), ECX = support-manager screen.
    virtual void support_set_large_icon_alpha_0065e870(Raw screen, void* widget, float alpha) = 0;
    // Support-manager exit helpers: 0054E610, ECX = [*00E198C4+A8h] (hints
    // dismiss); 0065ABB0 / 0065FAA0, ECX = screen (clear loadout icons, close
    // sound); 005BCA30(1), ECX = [*00E198C4+58h]; 0068AB80(0, 0), ECX =
    // *00E198C4; 00529060 then 00529A30 (tail jump), ECX = [*00E198C4+C8h].
    virtual void hints_dismiss_0054e610() = 0;
    virtual void support_clear_loadout_icons_0065abb0(Raw screen) = 0;
    virtual void support_play_close_sound_0065faa0(Raw screen) = 0;
    virtual void hud_panel_set_visible3_005bca30(std::int32_t value) = 0;
    virtual void interface_collapse_overlays_0068ab80(std::int32_t a, std::int32_t b) = 0;
    virtual void counters_restore_hidden_a5_00529060() = 0;
    virtual void counters_relayout_00529a30() = 0;
    // 00BF65AC: _free of a list node.
    virtual void free_00bf65ac(void* block) = 0;
    // Invite acceptance: 00A3E470, ECX = *00F8ABE8: the profile slot of the
    // invitee; 0067C970 rebind the primary input; 00A91620(2, slot), ECX =
    // *00F8BBF4: activate a device slot; device vtable +34h -> its slot;
    // 00571E50, ECX = record: session record construct; 00BF681B operator
    // new; *00F8A2FC vtable +6Ch(record): join the session.
    virtual std::int32_t xenon_find_slot_for_invitee_00a3e470() = 0;
    virtual void rebind_primary_input_0067c970() = 0;
    virtual void input_activate_device_slot_00a91620(std::int32_t kind, std::int32_t slot) = 0;
    virtual std::int32_t input_device_slot_34(void* device) = 0;
    virtual void session_record_construct_00571e50(Raw record) = 0;
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual void online_service_join_6c(Raw record) = 0;
    // Unit library (ECX = screen): 0067E9A0 toggle description, 0067E580 reset
    // selection; 004CC460(id, arg), ECX = *00E198AC: push an interface request.
    virtual void unit_library_toggle_description_0067e9a0(Raw screen) = 0;
    virtual void unit_library_reset_selection_0067e580(Raw screen) = 0;
    virtual void main_front_end_push_interface_004cc460(std::int32_t id, std::int32_t arg) = 0;
    virtual std::int32_t listbox_selected_index_00a9c920(void* listbox) = 0;
    virtual bool widget_is_visible_38(void* widget) = 0;
    // 004F8AC0, ECX = *00E18D48: stop the front-end movie; 00AAC870, ECX = GUI
    // movie widget: stop; 00A9C7C0(row), ECX = listbox: select; 0054A0C0, ECX =
    // *00E1930C: the help line.
    virtual void movie_player_stop_004f8ac0() = 0;
    virtual void gui_movie_stop_00aac870(void* movie) = 0;
    virtual void listbox_select_index_00a9c7c0(void* listbox, std::int32_t row) = 0;
    virtual void command_bar_set_help_line_0054a0c0(std::string_view text) = 0;
    virtual void widget_virtual_80(void* widget, std::int32_t value) = 0;
    // Camera binding: 00BF55BE operator new; 0064B650, ECX = block: the
    // ShipCaptain camera constructor (404h bytes, zeroed first); 00923870(0,
    // scene, const matrix*), ECX = camera; 00432E60(unit, 0), ECX = camera:
    // target and mode; 004BC410(camera), ECX = *00E188A8: the camera mover;
    // 0051E730(camera), ECX = [*00E198C4+4Ch]: binoculars bind.
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void* ship_captain_camera_construct_0064b650(void* block) = 0;
    virtual void camera_attach_00923870(void* camera, std::int32_t a, void* scene, const float matrix[16]) = 0;
    virtual void camera_mover_set_target_00432e60(void* camera, void* unit, std::int32_t mode) = 0;
    virtual void game_set_camera_mover_004bc410(void* camera) = 0;
    virtual void binoculars_bind_unit_0051e730(void* camera) = 0;
};

// Functions defined in Ghidra by this pass (no reference in the image):
// 0061F5E0 __thiscall(dst, const record* src), RET 4 -> dst: the rewards 10h
// record assign (byte +0, dword +4, native string +8 copied unless the same).
Raw rewards_record10_assign_0061f5e0(Raw dst, Raw src, Fe10GuiHost& host);
// 00637590 __thiscall(holder): (holder+4) vtable +34h(0).
void hud_widget_holder_hide_00637590(Raw holder, Fe10GuiHost& host);
// 006375A0 __thiscall(holder, float x, float y, const float* colour, const
// NativeString* key), RET 10h: 00AA7D00(x, y), +50h(colour), 00ABAED0(key, 0),
// +34h(1) on holder+4.
void hud_widget_holder_show_at_006375a0(Raw holder, float x, float y, const float* colour, Raw key, Fe10GuiHost& host);
// 0064A380 __thiscall(screen): weapon-group bind of +1Ch / +20h, then the
// observer pair (+8h on +1Ch) registered when it is not.
void hud_slot46_bind_weapon_group_unit_0064a380(Raw s, Fe10GuiHost& host);
// 0064A3C0 __thiscall(screen): weapon-group release, then role transfer 1FEh /
// take 0 on +1Ch when set (0077C470, cc9's routine, through the host).
void hud_slot46_release_weapon_group_0064a3c0(Raw s, Fe10GuiHost& host);

// 006434E0 __thiscall(ECX = markers screen, unit, b), RET 8: a squadron/group
// member's marker. Skipped when already marked; the unit's position (pose
// refreshed) has its y zeroed for a kind-8 unit below a third of its altitude
// band while the camera (*00E188A8+1ED4h) is above the water; then it must
// project on screen (00638E50), be detected by the screen's unit +24h within
// range 5 and pass 005220C0 before 006430C0(unit, 0, b, 0).
void markers_add_group_member_marker_006434e0(Raw s, void* unit, std::int32_t b, Fe10GuiHost& host);

// 006394B0 __thiscall(ECX = markers screen): the screen heading +2Ch. The
// camera transform *(*00E188A8+19FCh) (refreshed when bit 2 of +5Ch is clear)
// gives A = position (+120h..) + 10 * forward (+110h..) and B = A + (0, 1, 0);
// both are projected (0043A660(2, 0)) and, when the screen delta's squared
// length exceeds 0.001, +2Ch = atan2(ST1 = dx, ST0 = dy) - pi/2 (the double of
// the float pi/2). Read from the bytes (sc2_x87.py): FMUL ST1,ST0 is DC C9.
inline constexpr double kHeadingProbeDistance = 10.0;     // 00CE3DC0
inline constexpr double kHeadingProbeUp = 1.0;            // 00D7A210
inline constexpr double kHeadingMinLengthSq = 0.0010000000474974513;  // 00D7A318
inline constexpr double kHeadingQuarterTurn = 1.5707963705062866;     // 00CE3830
void markers_update_screen_heading_006394b0(Raw s, Fe10GuiHost& host);

// 00639810 __stdcall(unit) (ECX not read), RET 4 -> AL: the unit is inside the
// controlled vehicle's display range. d = |unit - *00E188D8| (0042B2F0); a
// vehicle or unit of kind 0Fh, a unit of kind 18h, or local roles 3 / 2
// compare d*d (float) against vehicle +49Ch; role 5 compares d against +44Ch,
// anything else against +490h (strictly greater wins).
bool hud_marker_is_within_display_range_00639810(void* unit, Fe10GuiHost& host);

// 00640620 __thiscall(ECX = markers screen): hide the pooled markers from the
// used counts on (+7Ch over +50h entries, +80h over +60h, +84h over +70h),
// delete the +A0h.. tail of the pointer vector +90h (vtable +0(1)), resize it
// to +A0h and zero the three counts.
void markers_reset_marker_pools_00640620(Raw s, Fe10GuiHost& host);

// 00668540 / 006697A0 __thiscall(dst, const record* src), RET 4 -> dst: the
// {int, std::list<int>, int} record assign (the first through list
// operator=, the second through the range assign unless the lists alias).
Raw int_list_int_record_assign_00668540(Raw dst, Raw src, Fe10GuiHost& host);
Raw int_list_int_record_assign_006697a0(Raw dst, Raw src, Fe10GuiHost& host);

// 0067D6B0 __thiscall(wide string, const wchar16* text), RET 4: assign the
// zero-terminated text (0067D5A0(text, length)).
void wide_string_assign_z_0067d6b0(Raw dst, const std::uint16_t* text, Fe10GuiHost& host);

// 00639230 __thiscall(ECX = colour icon set, float x, float y, float angle,
// const record* marker, void** unit_slot, bool offscreen, void* selected),
// RET 1Ch: hide the five icons (+4h, +8h, +0Ch, +14h, +10h), pick one by the
// marker kind +18h (0: +4h, 1: +8h, 3..5: +0Ch; none otherwise) or, for a unit
// whose team +54h is the local slot record's +28h, +10h (team 0) / +14h; show
// it rotated by angle - pi/2 at the clamped (00638B90) position with size
// (0.053124, 0.116667) for the selected unit, (0.035416, 0.077778) otherwise,
// (0.020833, 0.027778) offscreen; colour (1, 1, 1, 1) or, offscreen,
// alpha = *00F88A00 + 0.5; frame 2 offscreen, 1 otherwise.
inline constexpr float kColourIconSelectedSize[2] = {0.05312399938702583f, 0.1166669949889183f};   // 00CF5774/70
inline constexpr float kColourIconSize[2] = {0.035415999591350555f, 0.07777799665927887f};          // 00CF576C/68
inline constexpr float kColourIconOffscreenSize[2] = {0.020833000540733337f, 0.02777799963951111f}; // 00CF5764/60
inline constexpr std::uint32_t kMarkerPulseGlobal = 0x00F88A00;
// 006395E0 __thiscall(ECX unused, vec3 by value (unused), entity, float*
// min_x, float* min_y, float* max_x, float* max_y), RET 20h: the screen
// bounds of the entity's box. The outputs start at FLT_MAX / -FLT_MAX
// (00D7A248 / 00D7A244); the corners are built in the order (x0|x1, y0|y1,
// z0|z1) with z fastest, and only the first SEVEN are transformed (+F0h of the
// refreshed transform), projected (00638E50, result ignored) and folded in:
// the loop counter starts at 7 in the image.
void markers_project_part_bounds_006395e0(void* entity, float* min_x, float* min_y, float* max_x, float* max_y,
                                          Fe10GuiHost& host);

// 006567D0 __thiscall(ECX = support-manager screen): lay out the frame +1C8h
// around the widget +1C0h (shown): top-left = local position - anchor (+18h,
// +1Ch) * size, minus 0.6 * the frame's border (+10Ch, +114h), z - 1; size =
// widget size + 0.6 * (left + right) borders (+10Ch/+110h, +114h/+118h);
// then the widget +1B0h at (+1B4h - 0.035, +1B8h - 0.025, +1BCh). Traced
// from the bytes (sc2_x87.py); every constant is a double.
inline constexpr double kGuiScreenWidth = 960.0;  // 00CEC380
inline constexpr double kSupportFrameBorderScale = 0.6000000238418579;  // 00CEFF98
void support_manager_layout_widget_1c0_006567d0(Raw s, Fe10GuiHost& host);

// 0065BBC0 __thiscall(ECX = support-manager screen): the name label +170h.
// w = (float)(+114h / 960); when w is exactly 0 the text is re-applied from
// its wide source (+F0h, or the empty wide string 00E19834) to force a
// measure; when w >= the remembered +74h, the box +18Ch gets size (w + 0.21,
// +198h) and +74h = w; unless game+620h is set the label goes to
// (+19Ch + (((box width - +194h) - w) + 0.018) * 0.5 + 0.0035, +1A0h, +1A4h);
// then shown.
void support_manager_layout_name_label_0065bbc0(Raw s, Fe10GuiHost& host);

// 0065A2B0 __thiscall(ECX = support-manager screen): wrap the cursor index
// +308h into list A (+1ECh, size +1F4h; iterator +300h/+304h) and walk the
// iterator there; then wrap it into list B (+344h, size +34Ch; iterator
// +350h/+354h), show every B node's widget (+18h), walk to the index and
// give that node's widget frame 0 (+88h(0, 0, 1.0)) and z = +2D0h at its
// resolved x/y. The checked-iterator asserts are not modelled.
void support_manager_sync_cursor_iterators_0065a2b0(Raw s, Fe10GuiHost& host);

// 006619E0 __thiscall(ECX = support-manager screen): rotate the scroll rows.
// y = (n + 1 + +2E0h) * +2A8h for n = +2D8h; rows n..3 rotate the left icon
// (+ACh + 4i) by +2A4h + y and the right (+BCh + 4i) by -+2A4h - y, y += +2A8h.
// The large icon +B8h gets alpha 1 - +2E0h (0065E870); the selected radial
// icon (+DCh + 4 * +324h) too unless its alpha is exactly 1; when +2E0h is
// exactly 0 the bytes +2C1h, +2DCh, +2DDh clear, +FCh shows and that icon's
// alpha returns to 1.
void support_manager_layout_scroll_rows_006619e0(Raw s, Fe10GuiHost& host);

// 0065AC30 __thiscall(ECX = support-manager screen) -> count or -1: the stock
// of the selection for the unit of the cursor node (+304h, unit +8h). Kind
// 45h / 9 units search the list at +72Ch / +1188h (head +44h; node +8h id,
// +0Ch count) for id +2A0h; kind 46h units search the 20h-byte records of
// the vector +770h (+4h id, +8h count) for id +3A4h.
std::int32_t support_manager_selected_stock_count_0065ac30(Raw s, Fe10GuiHost& host);

// 00662470 __thiscall(ECX = support-manager screen): exit. Resets the label
// width +74h, hides [[*00E198C4+C4h]+64h], dismisses the hints, deletes the
// open sub-menus (+148h when +2C1h, +13Ch when +2C0h) and re-shows their
// buttons, collapses the scroll (+2E0h = 0, rows re-laid out, once more when
// +2DCh), hides the selected radial icon, frees the list +2E8h, clears the
// loadout icons, resets the box +18Ch to +194h, shows +190h, hides +170h,
// +CCh..+D8h, +1B0h and +78h; with a controlled unit whose slot record +19h
// and game+19C4h are clear the panel 005BCA30(1) returns; then overlays
// collapse, the close sound plays and the counters are restored and re-laid
// out (tail jump to 00529A30).
void support_manager_exit_00662470(Raw s, Fe10GuiHost& host);
// 00662690 __thiscall(ECX = support-manager screen, float dt), RET 4 (defined
// by this pass, unreferenced): when +2E0h is not exactly 0 it steps down by
// dt, clamps at 0, and the rows are re-laid out.
void support_manager_step_scroll_collapse_00662690(Raw s, float dt, Fe10GuiHost& host);

// 0067D6E0 __cdecl() -> AL: accept the pending invite *00E19880 (0 when
// none). The invitee's profile slot becomes the active input device slot
// (00A91620(2, slot), after 0067C970 unless the first device of
// [*00F8BBF4+B4h] already reports that slot); a session record (00571E50,
// byte +4 = FFh, wide string +8h = L"Session") carries a new 88h-byte info
// object (vtable 00CEED04: +4h/+8h from invite +14h/+18h, +0Ch..+18h from
// +40h..+4Ch, +1Ch..+3Fh from +1Ch..+3Fh) into the online service's join
// (vtable +6Ch); the invite is cleared and 1 returned.
inline constexpr std::uint32_t kPendingInviteGlobal = 0x00E19880;
inline constexpr std::uint32_t kSessionInfoVtable = 0x00CEED04;
inline constexpr std::uint16_t kSessionRecordName[8] = {u'S', u'e', u's', u's', u'i', u'o', u'n', 0};  // 00CF6DEC
bool game_accept_pending_invite_0067d6e0(Fe10GuiHost& host);

// 00680520 __thiscall(ECX = unit library screen+64h listener, widget, listbox),
// RET 8: listbox activation. Outside mode 1 (+E8h of the screen): rows 0/1
// toggle the description, row 2 pushes interface request (1, 0) on the main
// front end (tail jump to 004CC460). In mode 1: row 0 flips the visibility of
// the screen's +7Ch, row 1 resets the selection.
void unit_library_on_listbox_activate_00680520(Raw listener, void* widget, void* listbox, Fe10GuiHost& host);

// 0067E9A0 __thiscall(ECX = unit library screen): switch to the description
// view (+E8h = 1). The rows of the list [+6Ch]+64h hide their widget (+8h);
// +78h and +7Ch show, +70h hides; the listbox +74h gets focus, row 0, +80h(1)
// and the listener +64h; the front-end movie and the GUI movie +94h stop;
// [*00E198BC]+48h clears; +7Ch flips its visibility; command bar
// "FE.unitlib_toggledesc" A2h / back A3h (placement 2); empty help line.
void unit_library_toggle_description_0067e9a0(Raw s, Fe10GuiHost& host);

// 0067C1A0 __thiscall(ECX = torpedo-boat view screen, unit), RET 4: release
// the weapon group and, for the old unit +24h, its roles (0077C470(1FEh, 0),
// cc9's routine through the host) and its observer pair; +24h = unit. The
// ShipCaptain camera +2Ch is created on first use (404h zeroed bytes,
// identity matrix into 00923870(0, game+19CCh), pitch limits -1.553343 /
// 1.553343 at +3ECh / +3F0h, observer pair) or else the binoculars reset.
// When the camera's target changes, its offsets come from the unit
// ([+538h]+A0h * 0.75 into +38Ch..+398h, +384h/+388h zeroed, only when +3E8h
// is 0) and +A8h = unit +A8h + 5. Then the game camera mover, binoculars and
// weapon group bind it; the unit's observer pair is registered and +1Ch =
// unit +980h.
inline constexpr float kTorpedoBoatPitchLimit = 1.5533430576324463f;  // 00CEC400 (00CEC574 negative)
void torpedo_boat_view_bind_unit_camera_0067c1a0(Raw s, void* unit, Fe10GuiHost& host);

// 00648060 __thiscall(ECX = HUD root screen): show the front award of the
// queue +ACh (pairs {text, icon path}): the icon path gets "fe/" replaced by
// "gui/" (004CAD40, all occurrences), icon +5Ch state 0 texture with uv
// (0, 0, 1, 1), text +60h the localised text; the front is erased; both
// widgets alpha 1.0 and a timed entry 0 with range (0, 1) and +4h = 5.0.
inline constexpr float kAwardTickerSeconds = 5.0f;  // 00CE3850
void hud_root_advance_award_ticker_00648060(Raw s, Fe10GuiHost& host);
// 00648AB0 __thiscall(ECX = HUD root screen, NativeString key by value), RET 8:
// queue the award's text (006B9FD0) and icon (006B9F70) as a pair into +ACh;
// the key is freed by the callee.
void hud_root_queue_award_ticker_00648ab0(Raw s, Raw key, Fe10GuiHost& host);

void hud_colour_icon_set_show_for_kind_00639230(Raw set, float x, float y, float angle, Raw marker, void** unit_slot,
                                                bool offscreen, void* selected, Fe10GuiHost& host);

// 0061F9D0 __thiscall(ECX = rewards screen, const NativeString* sample), RET 4:
// the debrief speech. The sample handle is acquired by name into +10h
// (refcounted); the sound class "GUITestSpeech" is looked up once (guard bit 0
// of 00E196B4, cached in 00E196B0); a tracked sound of that class and type
// "Normal" on the owner [*00E198AC+60h]+10h is stored refcounted into
// [*00E198AC+60h]+14h.
inline constexpr std::uint32_t kTestSpeechGuard = 0x00E196B4;
inline constexpr std::uint32_t kTestSpeechClass = 0x00E196B0;
void rewards_play_test_speech_0061f9d0(Raw s, Raw sample, Fe10GuiHost& host);

// 00621870 __thiscall(ECX = screen+8h listener, listbox, widget), RET 8: when
// the widget is the score listbox (+D4h) on page 1 (+58h), rebuild the command
// bar: continue A2h at placement 0, navigate B3h when +1Dh is set, then for a
// flagged entry review A7h ("globals.playerreview", not for the local
// profile's own XUID) and gamercard A5h at placement 2.
void rewards_on_listbox_selection_changed_00621870(Raw listener, void* listbox, void* widget, Fe10GuiHost& host);

// 00622360 __thiscall(ECX = rewards screen, const NativeString* id, bool
// count_only), RET 8 -> count: walk the Lua table VehicleClass; every table
// entry with Unlock == true and a string UnlockID that contains id
// (both upper-cased, strstr) counts; unless count_only the vector +238h
// (10h-byte records) gets, at index size - count, the key (+4h) and the
// entry's Name (+8h).
std::int32_t rewards_collect_vehicle_unlocks_00622360(Raw s, Raw id, bool count_only, Fe10GuiHost& host);

// 0062A970 __thiscall(ECX = rewards screen): the rank / unlock pop-up of the
// group +25Ch (shown). "A_Text" visibility follows byte 00F88A30, "B_Text"
// reads "globals.continue" (00F88A30 set) or "FE_pc.init_enter". With the
// unlock vector +23Ch..+240h headed by a record whose byte +0 is set:
// "info_Text" = "FE.scoring_unlock_text", "rank_Icon" hidden, "rank_Text" =
// the record's name (+8h), "unit_Icon" shown with the atlas icon of id +4h,
// and the record is appended to +248h. Otherwise "FE.scoring_rank_text",
// "rank_Icon" frame max(rank setting - 1, 0) through +88h, "rank_Text" the key
// 00E08E00[frame] (FE.playerrank_01..), "unit_Icon" hidden.
inline constexpr std::uint32_t kPlayerRankKeys = 0x00E08E00;
void rewards_fill_rank_page_0062a970(Raw s, Fe10GuiHost& host);

// ---------------------------------------------------------------------------
// Packet cc13-trivialslots: one ordinary function per kTrivialSlots row, re-read from
// the disk bytes (local generator, rows checked against the image). `Raw` is the
// ECX object; stack arguments the body never reads are noted in the ABI and not
// passed. Adjustor and tail-jump rows appear only where the target is an existing
// declaration; the descriptive table above stays as documentation.
// ---------------------------------------------------------------------------
// 00626710 MOV EAX,4h; RET (B8 04 00 00 00 C3; next byte CC). BSP_RewardsScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 4, RET.
std::int32_t rewards_screen_get_screen_id_00626710(Raw) noexcept;
// 00636DC0 MOV EAX,5Ah; RET (B8 5A 00 00 00 C3; next byte CC).
// BSP_HudSceneInitScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 90, RET.
std::int32_t hud_scene_init_screen_get_screen_id_00636dc0(Raw) noexcept;
// 00636DD0 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudSceneInitScreen_OccludesLowerLevels.
// Original ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_scene_init_screen_occludes_lower_levels_00636dd0(Raw) noexcept;
// 00636DE0 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudSceneInitScreen_IsSelfManaged.
// Original ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_scene_init_screen_is_self_managed_00636de0(Raw) noexcept;
// 006375F0 RET 4 (C2 04 00; next byte CC). BSP_HudMarkersScreen_Listener04NoOp. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_markers_screen_listener04no_op_006375f0(Raw) noexcept;
// 006405E0 MOV EAX,4Dh; RET (B8 4D 00 00 00 C3; next byte CC).
// BSP_HudMarkersScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 77, RET.
std::int32_t hud_markers_screen_get_screen_id_006405e0(Raw) noexcept;
// 00643DC0 MOV EAX,43h; RET (B8 43 00 00 00 C3; next byte CC).
// BSP_HudSpectatorScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 67, RET.
std::int32_t hud_spectator_screen_get_screen_id_00643dc0(Raw) noexcept;
// 00643DD0 MOV AL,1; RET (B0 01 C3; next byte CC). BSP_HudSpectatorScreen_IsSelfManaged.
// Original ABI: __thiscall(ECX = this) -> AL = 1, RET.
bool hud_spectator_screen_is_self_managed_00643dd0(Raw) noexcept;
// 00643DE0 RET (C3; next byte CC). BSP_HudSpectatorScreen_Exit. Original ABI: __thiscall(ECX =
// this), RET.
void hud_spectator_screen_exit_00643de0(Raw) noexcept;
// 00643DF0 RET 4 (C2 04 00; next byte CC). BSP_HudSpectatorScreen_Update. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_spectator_screen_update_00643df0(Raw) noexcept;
// 00647FE0 MOV EAX,44h; RET (B8 44 00 00 00 C3; next byte CC).
// BSP_InGameHudRootScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 68, RET.
std::int32_t in_game_hud_root_screen_get_screen_id_00647fe0(Raw) noexcept;
// 0064B860 MOV EAX,45h; RET (B8 45 00 00 00 C3; next byte CC).
// BSP_HudShipStatusScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 69, RET.
std::int32_t hud_ship_status_screen_get_screen_id_0064b860(Raw) noexcept;
// 00650160 MOV EAX,48h; RET (B8 48 00 00 00 C3; next byte CC).
// BSP_HudSubmarineScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 72, RET.
std::int32_t hud_submarine_screen_get_screen_id_00650160(Raw) noexcept;
// 00650190 RET (C3; next byte CC). BSP_HudSubmarineScreen_Enter. Original ABI: __thiscall(ECX =
// this), RET.
void hud_submarine_screen_enter_00650190(Raw) noexcept;
// 00650AC0 MOV EAX,47h; RET (B8 47 00 00 00 C3; next byte CC).
// BSP_HudPeriscopeScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 71, RET.
std::int32_t hud_periscope_screen_get_screen_id_00650ac0(Raw) noexcept;
// 006730F0 MOV EAX,4Eh; RET (B8 4E 00 00 00 C3; next byte CC).
// BSP_HudSupportManagerScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 78,
// RET.
std::int32_t hud_support_manager_screen_get_screen_id_006730f0(Raw) noexcept;
// 0067AF30 RET 8 (C2 08 00; next byte CC).
// BSP_HudTacticalSheetScreen_OnListboxSelectionChangedNoOp. Original ABI: __thiscall(ECX =
// this, 2 stack dwords unread), RET 8.
void hud_tactical_sheet_screen_on_listbox_selection_changed_no_op_0067af30(Raw) noexcept;
// 0067AF60 MOV EAX,54h; RET (B8 54 00 00 00 C3; next byte CC).
// BSP_HudTacticalSheetScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 84,
// RET.
std::int32_t hud_tactical_sheet_screen_get_screen_id_0067af60(Raw) noexcept;
// 0067B320 MOV EAX,28h; RET (B8 28 00 00 00 C3; next byte CC). BSP_HudCursorScreen_GetScreenId.
// Original ABI: __thiscall(ECX = this) -> EAX = 40, RET.
std::int32_t hud_cursor_screen_get_screen_id_0067b320(Raw) noexcept;
// 0067B330 RET (C3; next byte CC). BSP_HudCursorScreen_Exit. Original ABI: __thiscall(ECX =
// this), RET.
void hud_cursor_screen_exit_0067b330(Raw) noexcept;
// 0067B3F0 JMP 004F71D0 (E9 DB BD E7 FF; next byte CC). BSP_HudCommonOverlayScreen_Register.
// Original ABI: __thiscall(ECX = this), the target's RET; tail call to 004F71D0
// (HudScreenServices::front_end_screen_register_004f71d0).
void hud_common_overlay_screen_register_0067b3f0(Raw self, HudScreenServices& host);
// 0067B400 RET (C3; next byte CC). BSP_HudCommonOverlayScreen_Enter. Original ABI:
// __thiscall(ECX = this), RET.
void hud_common_overlay_screen_enter_0067b400(Raw) noexcept;
// 0067B410 RET (C3; next byte CC). BSP_HudCommonOverlayScreen_Exit. Original ABI:
// __thiscall(ECX = this), RET.
void hud_common_overlay_screen_exit_0067b410(Raw) noexcept;
// 0067BE50 MOV EAX,49h; RET (B8 49 00 00 00 C3; next byte CC).
// BSP_HudFollowScreen49_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 73, RET.
std::int32_t hud_follow_screen49_get_screen_id_0067be50(Raw) noexcept;
// 0067BE60 RET 4 (C2 04 00; next byte CC). BSP_HudFollowScreen49_CollectChildren. Original ABI:
// __thiscall(ECX = this, 1 stack dword unread), RET 4.
void hud_follow_screen49_collect_children_0067be60(Raw) noexcept;
// 0067BEA0 RET (C3; next byte CC). BSP_HudFollowScreen49_BindLayout. Original ABI:
// __thiscall(ECX = this), RET.
void hud_follow_screen49_bind_layout_0067bea0(Raw) noexcept;
// 0067BEB0 RET (C3; next byte CC). BSP_HudFollowScreen49_Enter. Original ABI: __thiscall(ECX =
// this), RET.
void hud_follow_screen49_enter_0067beb0(Raw) noexcept;
// 0067C860 MOV EAX,5Ch; RET (B8 5C 00 00 00 C3; next byte CC).
// BSP_PressStartScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 92, RET.
std::int32_t press_start_screen_get_screen_id_0067c860(Raw) noexcept;
// 0067D9F0 RET 8 (C2 08 00; next byte CC). BSP_UnitLibraryScreen_OnListboxSelectionChangedNoOp.
// Original ABI: __thiscall(ECX = this, 2 stack dwords unread), RET 8.
void unit_library_screen_on_listbox_selection_changed_no_op_0067d9f0(Raw) noexcept;
// 0067DA00 RET 4 (C2 04 00; next byte CC). BSP_UnitLibraryWidgetListener_Current00NoOp.
// Original ABI: __thiscall(ECX = this, 1 stack dword unread), RET 4.
void unit_library_widget_listener_current00no_op_0067da00(Raw) noexcept;
// 0067F050 MOV EAX,5h; RET (B8 05 00 00 00 C3; next byte CC).
// BSP_UnitLibraryScreen_GetScreenId. Original ABI: __thiscall(ECX = this) -> EAX = 5, RET.
std::int32_t unit_library_screen_get_screen_id_0067f050(Raw) noexcept;

} // namespace bsp::ingame_gui_cc13

#endif
