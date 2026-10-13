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
    virtual void binoculars_update_view_0051f330(std::uint32_t arg) = 0;
    virtual void screen2e_set_view_centre_005454b0(float x, float y) = 0;
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
void binoculars_update_view_recentre_0067c0e0(Raw s, std::uint32_t arg, HudScreenHost& host);

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
inline constexpr std::array<std::uint32_t, 4> kObserverRefReleased{
    0x0064B590, 0x0065AFA0, 0x0065B040, 0x0065B090};
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

void rewards_release_sound_handles_0061ec00(Raw s, RewardsScreenHost& host);
void rewards_widget_release_0061d770(Raw listener, void* widget, RewardsScreenHost& host);
void rewards_widget_press_0062b3b0(Raw listener, void* widget, RewardsScreenHost& host);
void rewards_exit_00621720(Raw s, RewardsScreenHost& host);
void rewards_continue_0062b110(Raw s, RewardsScreenHost& host);

} // namespace bsp::ingame_gui_cc13

#endif
