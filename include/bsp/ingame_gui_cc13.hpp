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

} // namespace bsp::ingame_gui_cc13

#endif
