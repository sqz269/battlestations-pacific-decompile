#pragma once
// The lifecycle of the in-mission interface manager 00E198C4: its constructor
// 0068A990, its Init 0068CC70, the per-frame update 0068C1F0 that the in-mission
// frame calls on it, and the teardown 0068BC60 / 0068B630 the mission exit runs.
//
// docs/IN_MISSION_INTERFACE_RUNTIME.md carries the evidence. Everything here is a
// hypothesis reconstructed from the listing; no symbol was recovered from the
// image. The manager layout, the interface ids, the screen table and the
// level-1 map live in "bsp/ingame_interface.hpp" and are reused, not redeclared.
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/ingame_interface.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// 1. The constructor, 0068A990
// ---------------------------------------------------------------------------

// __thiscall(this) -> this in EAX, RET at 0068A9BE. After the base constructor
// 00684E10 and the vtable store it clears exactly five fields; every other
// pointer field is written unconditionally by Init, which is why they are not
// cleared here. The offsets are in listing order.
// In listing order, with the instruction that clears each. The vtable store
// 00CF7A60 sits at 0068A99A, before them.
inline constexpr std::uint16_t kInGameInterfaceConstructorClears[] = {
    0x100,  // 0068A9A0, the ambient sound source
    0x104,  // 0068A9A6, the ambient sound instance
    0xEC,   // 0068A9AC, param_1[3Bh]
    0x54,   // 0068A9B2, the registry-slot 4Ch screen pointer
    0xFD,   // 0068A9B5, a byte
};
inline constexpr std::size_t kInGameInterfaceConstructorClearCount = 5;

// True when the constructor clears the field at `offset`. The reconstruction
// exposes the rule rather than a memset so a reader can tell a cleared field
// from one Init overwrites.
bool in_game_interface_constructor_clears(std::uint16_t offset) noexcept;

// Applies 0068A990 to `manager`. The base is left to the caller: 00684E10 is
// reconstructed elsewhere and this routine only models the derived part.
void construct_in_game_interface_0068a990(InGameInterfaceManager& manager) noexcept;

// ---------------------------------------------------------------------------
// 2. Init, 0068CC70
// ---------------------------------------------------------------------------

// The 42 screens are constructed in the order of `kInGameHudScreens` (the table
// in bsp/ingame_interface.hpp), each as `operator new(size)` at 00BF681B, the
// constructor, the store into the manager, and then the screen's virtual +10h,
// which is BSP_FrontEndScreen_Register 004F71D0. A null allocation skips the
// constructor but is still stored and still registered, which is the native
// behaviour at every one of the 42 sites (for example 0068CD1A..0068CD31).
struct InMissionInterfaceInitHost {
    virtual ~InMissionInterfaceInitHost() = default;

    // 00AF0060 on the texture-atlas manager at 00F8C26C, 0068CCD1. The literal
    // is the stem kInGameInterfaceAtlas; the install carries the per-format
    // variants. The pooled string around it (0041DD40, 00BF7680, 00419CC0,
    // 00BD1510) is allocation bookkeeping and is not a host step.
    virtual void load_texture_atlas(std::string_view stem) = 0;

    // 0068CCFE, `00E198C4 = this`. Init publishes the global a second time; the
    // first write is the one the mission-scene load does at 004E0480.
    virtual void publish_manager_global() = 0;

    // 0068CD04, the base Init 00683A90. Its body is a bare RET in this build.
    virtual void base_init() = 0;

    // One allocation per screen. Forty-one of the forty-two go through
    // 00BF681B (`operator new`); the registry-slot 29h screen at +CCh is the
    // exception and is allocated at 0068D47F through 00BF55BE and then zeroed
    // with `memset(p, 0, F0h)` at 0068D48D before its constructor runs. Returns
    // false when the allocator returned null, which is the only case the native
    // code branches on.
    virtual bool allocate_screen(std::size_t index, std::size_t size_bytes) = 0;
    // The screen's own constructor, or the inline build for the six slots whose
    // table entry has no constructor address. Skipped when the allocation
    // returned null.
    virtual void construct_screen(std::size_t index, std::uint32_t constructor) = 0;
    // The screen's virtual +10h, BSP_FrontEndScreen_Register 004F71D0, which
    // asks the screen for its registry slot through virtual +00h and stores it
    // at 00E18B60 + slot*4.
    virtual void register_screen(std::size_t index) = 0;

    // 0068D73A, `this->004CC460(20h, nullptr)`: the first interface request of
    // the mission is INTF_SCENE3D with no payload.
    virtual void push_interface_request(int interface_id, bool has_payload) = 0;

    // 0068D743, `[this+40h]->00644220(0)`. The callee is a one-line setter,
    // `hudRoot->[1Ch] = argument`.
    virtual void hud_root_set_field_1c(std::uint32_t value) = 0;
};

// 0068CC70, __thiscall(this), RET at 0068D759, vtable 00CF7A60 slot +04h. The
// native body is wrapped in an SEH frame (handler 00C7E093) whose state index
// counts the constructed screens so a throw unwinds them; that unwind path is
// not modelled.
void init_in_game_interface_0068cc70(InGameInterfaceManager& manager,
    InMissionInterfaceInitHost& host);

// ---------------------------------------------------------------------------
// 3. The per-frame update, 0068C1F0
// ---------------------------------------------------------------------------

// The four game modes 004BCA50 returns that open the spectator block at
// 0068C203..0068C219. docs/GAME_SIMULATION_GATE.md reads the same set.
bool in_mission_spectator_mode(int effective_game_mode) noexcept;

// The five modes that let the spectate-next block at 0068C4F2..0068C50C run
// without a controlled unit: 8, 0, 1, 2 and 3.
bool spectate_next_mode_allows_empty_unit(int effective_game_mode) noexcept;

// Unit class ids passed to the unit's virtual +5Ch (IsKindOf) from this routine.
// kUnitTypePlane and kUnitTypeSubmarine already exist in bsp/ingame_interface.hpp.
inline constexpr int kUnitClassCommandBuilding = 0x1C; // 0068C2FD

// The team value that makes a unit spectatable by any slot, 0068C309.
inline constexpr int kSpectateAnyTeam = 9;

// The two pilot states that abandon the spectate walk, 0068C2D3/0068C2D8:
// `[unit->[9D4h] + 3D0h]->[900h]` in {4, 5}.
bool spectate_walk_pilot_state_aborts(int pilot_state) noexcept;

// The two level-3 screen ids that suppress the spectate-next block when the
// level-3 vector holds exactly one entry, 0068C4BC and 0068C4E2.
inline constexpr int kLevel3ScreenSuppressA = 0x39;
inline constexpr int kLevel3ScreenSuppressB = 0x3B;
bool level3_entry_suppresses_spectate(int screen_id) noexcept;

// The input actions 004C43C0 is asked about, in the order the body asks.
inline constexpr int kActionFreeCameraToggle = 0x5A;   // 0068C3F6, 0068C42F
inline constexpr int kActionMovieCamera = 0x89;        // 0068C5EA
inline constexpr int kActionMovieCameraNew = 0x8A;     // 0068C605
inline constexpr int kActionBackOutOverlay = 0xC1;     // 0068C647
inline constexpr int kActionCloseLevel2 = 0xD6;        // 0068C6C6
inline constexpr int kActionOverlayA = 0xFB;           // 0068C704
inline constexpr int kActionOverlayB = 0xFC;           // 0068C73B
inline constexpr int kActionOverlayC = 0x108;          // 0068C8BD
inline constexpr int kActionCollapseLevel3 = 0x4B;     // 0068CA32

// The virtual-key GetKeyState reads at 0068C675 when the global at 00F88A30 is
// clear. A4h is VK_RMENU; the arm runs while the key is **not** down.
inline constexpr int kBackOutOverlayModifierKey = 0xA4;

// The level-3 sets the overlay-C arm installs at 0068C92C..0068C967. Each list
// is in argument order, which is the reverse of the push order; the trailing
// zero of the native varargs list is not part of the count. The argument counts
// come from the cleanups `ADD ESP,0Ch` at 0068C93C, `ADD ESP,10h` at 0068C954
// and `ADD ESP,1Ch` at 0068C96C, not from the pushes.
inline constexpr int kOverlayCLevel3Screens[] = {0x4F, 0x50};
inline constexpr int kOverlayCLevel3ContextsNetworked[] = {0x14, 0x0E};
inline constexpr int kOverlayCLevel3ContextsLocal[] = {0x14, 0x04, 0x0B, 0x06, 0x0A};

// The ambient-volume curve at 0068CB0C..0068CB37: the camera height at
// `[game+19FCh]+124h` minus the double 2.0 at 00D7A308, times the double 0.25
// at 00D7A348, clamped to [0, 1.0f]; 00D7A24C holds the 1.0f. The native code
// computes the product on the x87 stack and stores it as a float before the
// clamp, so the clamp sees the rounded value.
inline constexpr float kAmbientVolumeHeightBias = 2.0f;
inline constexpr float kAmbientVolumeHeightScale = 0.25f;
inline constexpr float kAmbientVolumeMax = 1.0f;
float ambient_volume_from_camera_height_0068cb0c(float camera_height) noexcept;

// The unit height below which the controlled unit forces the "Underwater"
// environment, 00CF1430. The test at 0068CC28 is `height >= -4.0f` for the
// non-forcing side, so a NaN height does not force.
inline constexpr float kUnderwaterUnitHeight = -4.0f;

// 0068CBF3..0068CC3A. The byte ANDed with the above-water test is 1 unless the
// controlled unit 00E188D8 exists, is a submarine (IsKindOf(8)) and sits below
// kUnderwaterUnitHeight. It is a byte AND, not a short circuit: the water
// height is always computed.
bool controlled_unit_allows_air_0068cbf3(bool has_controlled_unit, bool unit_is_submarine,
    float unit_height) noexcept;

// The whole environment choice of the tail, 0068CB4A..0068CC48. "Cockpit" wins
// over the water test. The two names are the same literals
// bsp/simulation_gate.hpp lists; this overload takes the inputs this packet
// establishes rather than the "device flag" that header records.
struct InMissionAudioEnvironmentInputs {
    // [this+6Ch]+4h && 00604F30() == 1, 0068CB4A..0068CB5B.
    bool cockpit_screen_wanted{false};
    int cockpit_screen_mode{0};
    // [this+84h]+4h && [this+84h]+74h == 2, 0068CB5D..0068CB6D.
    bool second_cockpit_screen_wanted{false};
    int second_cockpit_screen_mode{0};
    // The camera at game+19FCh, +124h, and 0078CF20 under +120h.
    float camera_height{0.0f};
    float water_height{0.0f};
    // The three inputs of controlled_unit_allows_air_0068cbf3.
    bool has_controlled_unit{false};
    bool controlled_unit_is_submarine{false};
    float controlled_unit_height{0.0f};
};
inline constexpr int kCockpitScreenMode = 1;       // 0068CB58
inline constexpr int kSecondCockpitScreenMode = 2; // 0068CB69
std::string_view in_mission_audio_environment_0068cb4a(
    const InMissionAudioEnvironmentInputs& inputs) noexcept;

// The six places the body performs the same level-3 collapse (0068C569,
// 0068C7C0, 0068C9ED, 0068CA68, plus the copies inside 0068C0B0 and 0068B3F0).
// It runs only while the level-3 vector is non-empty.
struct Level3CollapseOutcome {
    bool ran{false};
    bool camera_flag_0a_set{false};   // [this+BCh]+0Ah = 1 when the global's +30h is set
    bool scene_request_pushed{false}; // the 0068AA40 rule accepted the unit
};

// 0068AA40, __thiscall(this, unit), the re-entry request every collapse ends
// with. It pushes INTF_SCENE3D for `unit` only when the unit differs from the
// pending payload, is non-null, has its byte +5Ch set and its bytes +5Dh, +60h
// and +5Eh clear, and the pending interface id is not one of the four camera
// modes. 0068C5A2..0068C5DF is the same rule inlined.
struct SceneRequestUnitFlags {
    bool flag_5c{false};
    bool flag_5d{false};
    bool flag_5e{false};
    bool flag_60{false};
};
bool scene_request_accepts_unit_0068aa40(bool unit_present, bool unit_is_pending_payload,
    const SceneRequestUnitFlags& flags, int pending_interface_id) noexcept;

// The mutable state the update reads and writes across its blocks. Everything
// else it touches belongs to a screen or a global and reaches the host.
struct InMissionInterfaceUpdateState {
    // 00E1AE80, the one-shot free-camera request the update consumes at
    // 0068C401. Set elsewhere; this routine only clears it.
    bool free_camera_requested{false};
    // The loop flag at ESP+0Fh: set when the spectate walk found a candidate,
    // read at 0068C3C4 to decide whether to drop the camera flag.
    bool spectate_candidate_found{false};
};

// One method per native call site the update reaches, in the order the listing
// reaches them. There are no default implementations: nothing here stands in
// for unrecovered game behaviour. `unit_index` addresses the walk at
// [[game+19CCh]+58h], which advances through node +4h and takes the unit from
// node +8h.
struct InMissionInterfaceUpdateHost {
    virtual ~InMissionInterfaceUpdateHost() = default;

    // -- globals and the game object ---------------------------------------
    virtual int effective_game_mode() = 0;          // 004BCA50, 0068C1FE
    virtual int local_player_slot() = 0;            // game+18ECh
    virtual bool local_slot_flag_19() = 0;          // game[18CCh + slot*4]+19h
    virtual int local_slot_unit_id() = 0;           // the same record's +28h
    virtual bool has_controlled_unit() = 0;         // 00E188D8 != 0
    virtual bool input_action_pressed(int action) = 0; // 004C43C0, ECX = game
    // 0068C66C..0068C683: true when the arm may proceed, which is when the
    // global at 00F88A30 is set or the right-ALT key is not down.
    virtual bool modifier_key_allows_back_out() = 0;
    // game+19C4h at 0068C93F. 0068C0B0 branches on the same byte for the same
    // pair of input-context lists; what it distinguishes is not established.
    virtual bool session_flag_19c4() = 0;
    virtual bool overlay_c_extra_gate() = 0;        // 00E0C978, 0068C908

    // -- the camera screen at +BCh (registry slot 4Eh) ----------------------
    virtual bool camera_screen_spectating() = 0;    // 006529E0
    virtual bool camera_screen_flag_09() = 0;       // 00652A20
    virtual bool camera_screen_flag_08() = 0;       // 00652A30
    virtual void camera_screen_set_flag_08(bool value) = 0; // 00652A50
    virtual bool camera_screen_flag_0a() = 0;       // the byte read at 0068C25B
    virtual void camera_screen_set_flag_0a(bool value) = 0; // the stores at 0068C581 and friends
    virtual bool camera_screen_flag_30() = 0;       // [00E198C4+BCh]+30h
    virtual bool camera_screen_flag_66() = 0;       // 006529C0
    virtual void camera_screen_set_flag_66(bool value) = 0; // 006529B0
    virtual void camera_screen_frame_step() = 0;    // 00673130, 0068C77F

    // -- the level-3 vector 00E18D18 ---------------------------------------
    virtual std::size_t level3_screen_count() = 0;  // (00E18D20 - 00E18D1C) >> 2
    virtual int level3_screen_id(std::size_t index) = 0; // *00E18D1C
    virtual void set_level3_screen_set(const int* ids, std::size_t count) = 0;   // 004F8670
    virtual void set_level3_input_contexts(const int* ids, std::size_t count) = 0; // 004D8B70

    // -- the spectate walk --------------------------------------------------
    virtual std::size_t spectate_unit_count() = 0;
    virtual bool unit_is_kind_of(std::size_t unit_index, int class_id) = 0; // virtual +5Ch
    virtual bool unit_has_pilot(std::size_t unit_index) = 0;   // unit+9D4h
    virtual int unit_owner_id(std::size_t unit_index) = 0;     // unit+54h
    virtual int unit_pilot_state(std::size_t unit_index) = 0;  // [pilot+3D0h]+900h
    virtual bool unit_dead(std::size_t unit_index) = 0;        // unit+5Dh
    virtual int unit_player_slot(std::size_t unit_index) = 0;  // 009FFD20
    virtual int unit_team(std::size_t unit_index) = 0;         // unit+188h
    virtual void pilot_detach(std::size_t unit_index) = 0;     // 007EE4E0 on unit+9D4h
    virtual void unit_bind_player(std::size_t unit_index, int player_slot) = 0; // 00927CC0
    // [00E198C4+40h]->00647300(target) at 0068C3AF, the HUD root's spectate
    // setter: it tests the unit through 00645060, hands it to 00645600, and
    // either pushes INTF_LIMBO (no controlled unit and no pending change) or
    // calls the root's 00647040.
    virtual void hud_root_set_spectated_unit(std::size_t unit_index, bool use_pilot) = 0;

    // -- the manager's own calls -------------------------------------------
    virtual void push_interface_request(int interface_id, bool has_payload) = 0; // 004CC460
    virtual void push_scene_request_for_unit() = 0; // 004CC460(20h, 004B4B00())
    virtual void toggle_tactical_overlay(std::uint32_t argument) = 0; // 0068C0B0
    virtual void exit_free_camera_overlay() = 0;    // 0068B3F0
    virtual void back_out_one_overlay_level() = 0;  // 0068B470
    virtual void collapse_overlays(bool clear_level2, bool run_extra_hook) = 0; // 0068AB80
    virtual void toggle_movie_camera() = 0;         // 0068A160
    virtual void toggle_new_movie_camera() = 0;     // 0068A1F0

    // -- 004B4B00, the unit a scene request would carry ---------------------
    // The call at 0068C59D plus the field reads of what it returned. 004B4B00
    // returns the controlled unit 00E188D8 when it is IsKindOf(5), that unit's
    // +3D0h when it is IsKindOf(18h), and null otherwise. Returns false for the
    // null case. Only the inlined copy of 0068AA40 at 0068C5A2 needs this; the
    // other collapse sites reach the same rule through `push_scene_request_for_unit`.
    virtual bool query_scene_unit(SceneRequestUnitFlags& flags, bool& is_pending_payload) = 0;

    // -- other screens ------------------------------------------------------
    virtual void hud_root_set_field_1c(std::uint32_t value) = 0; // 00644220, 0068C419
    virtual bool screen_2b_wanted() = 0;            // [this+90h]+4h
    virtual void screen_2b_set_mode(bool value) = 0; // 0053D290, 0068C442/0068C44B
    virtual bool screen_34_query() = 0;             // [this+A8h]->0054D3C0
    virtual bool screen_2a_wanted() = 0;            // [this+5Ch]+4h
    virtual void screen_2a_clear_wanted() = 0;      // the store at 0068C6E3
    virtual bool screen_3b_block_a() = 0;           // [this+64h]+81h
    virtual bool screen_3b_block_b() = 0;           // [this+64h]+82h
    virtual void screen_3b_idle() = 0;              // 005FB080, 0068C6BB
    virtual bool screen_45_block_a() = 0;           // [this+78h]+156h
    virtual bool screen_45_block_b() = 0;           // [this+78h]+157h
    virtual bool map_screen_wanted() = 0;           // [this+C0h]+4h
    virtual void map_screen_frame_step() = 0;       // 00612DA0, 0068C78A
    virtual bool map_screen_busy() = 0;             // 00611750 on [this+C0h]
    virtual std::uint32_t map_screen_overlay_argument() = 0; // [this+CCh]+4Ch
    virtual bool pause_screen_flag_05() = 0;        // [00E198C4+A8h]+5h
    virtual bool pause_screen_flag_08() = 0;        // [00E198C4+A8h]+8h
    virtual bool pause_screen_child_query() = 0;    // [[00E198C4+A8h]+20h]->vtable[38h]
    virtual bool support_request_pending() = 0;     // 008ED9C0 on 00F88C30

    // -- input device bytes -------------------------------------------------
    virtual void input_manager_update(float seconds) = 0;  // 004BEC00 then 00A92C40
    virtual bool device_a_flag_0b() = 0;   // [[input+4h]+2F3Ch]+0Bh
    virtual bool device_a_flag_10() = 0;   // [[input+4h]+2F3Ch]+10h
    virtual bool device_b_flag_0b() = 0;   // [[input+4h]+2F6Ch]+0Bh
    virtual bool device_b_flag_10() = 0;   // [[input+4h]+2F6Ch]+10h
    virtual std::uint32_t controlled_unit_overlay_argument() = 0; // 00927880

    // -- the ambience and environment tail ----------------------------------
    virtual bool has_ambient_sound_instance() = 0;  // this+104h
    virtual bool camera_present() = 0;              // game+19FCh
    virtual void refresh_camera_transform() = 0;    // 00B6DB70 when +5Ch bit 1 is clear
    virtual float camera_height() = 0;              // camera+124h
    virtual float camera_ground_x() = 0;            // camera+120h
    virtual void set_ambient_volume(float volume) = 0; // 00A79880 on this+104h
    virtual float water_height(float x, float y) = 0;  // 0078CF20 on game+19F0h
    virtual bool cockpit_screen_wanted() = 0;       // [this+6Ch]+4h
    virtual int cockpit_screen_mode() = 0;          // 00604F30
    virtual bool second_cockpit_screen_wanted() = 0; // [this+84h]+4h
    virtual int second_cockpit_screen_mode() = 0;   // [this+84h]+74h
    virtual bool controlled_unit_is_submarine() = 0; // IsKindOf(8), 0068CC04
    virtual void refresh_controlled_unit_pose() = 0; // 00414DB0 when +C8h is clear
    virtual float controlled_unit_height() = 0;     // controlled unit +100h
    virtual void set_audio_environment(std::string_view name) = 0; // 00A7B710
    virtual void tick_profile_hints(int slot) = 0;  // 004C1E90(slot) then 00427190
};

// The slot the tail passes to 004C1E90 and 00427190, 0068CC54.
inline constexpr int kProfileHintsSlot = 0x0C;

// 0068C1F0, __thiscall(this = 00E198C4), no stack arguments, RET at 0068CC68.
// The in-mission frame calls it as step 17 of GGame::OnMove from the
// interface-only arm of the simulation gate, at 004E5252, guarded by
// `00E198C4 != 0 && [00E198C4]+3Ch != 0` (docs/MISSION_STATE_FRAME.md).
void update_in_game_interface_0068c1f0(InGameInterfaceManager& manager,
    InMissionInterfaceUpdateState& state, InMissionInterfaceUpdateHost& host);

// ---------------------------------------------------------------------------
// 4. Teardown, 0068BC60 and 0068B630
// ---------------------------------------------------------------------------

struct InMissionInterfaceTeardownHost {
    virtual ~InMissionInterfaceTeardownHost() = default;

    virtual void restore_vtable(std::uint32_t vtable) = 0;     // 0068B651
    // The refcounted release of the ambient sound instance: virtual +8h(0) to
    // stop it, then InterlockedDecrement on instance+4h and virtual +00h when
    // the count reaches zero. The first release is at 0068B674..0068B6A2, the
    // second at 0068BBF9..0068BC0F, and the source at +100h at 0068BC23.
    virtual void stop_ambient_sound() = 0;
    virtual void release_ambient_instance() = 0;
    virtual void release_ambient_source() = 0;

    // The 42 screens are pushed through 00686C20, which de-duplicates, so the
    // vector is the distinct set. Both passes walk that vector.
    virtual std::size_t collect_screens() = 0;                 // 44 calls to 00686C20
    // Both passes skip a null entry (0068BAB2 and 0068BB29).
    virtual bool screen_present(std::size_t index) = 0;
    virtual bool screen_visible(std::size_t index) = 0;        // screen+5h, 0068BAC3
    virtual void screen_hide(std::size_t index) = 0;           // virtual +1Ch, 0068BACF
    virtual void screen_clear_flags(std::size_t index) = 0;    // +4h and +5h, 0068BAD3
    virtual void screen_commit_visibility(std::size_t index) = 0; // 004F83B0, 0068BAD9
    virtual void screen_delete(std::size_t index) = 0;         // virtual +0Ch(1), 0068BB34

    virtual void clear_manager_global() = 0;                   // 0068BB54
    virtual void unload_texture_atlas(std::string_view stem) = 0; // 00AEFA30, 0068BB96
    virtual void close_menu_command_screen() = 0;              // 00425D10(1) then 00530630
    virtual void free_screen_vector() = 0;                     // 00BF65AC, 0068BBD6
    virtual void destroy_base() = 0;                           // 00684FA0, 0068BC49
    virtual void free_manager_memory() = 0;                    // 00BF65AC, 0068BC70
};

// 0068B630, __thiscall(this), RET at 0068BC5F. `python tools/bsp.py ghidra flow
// 0068b630` reports a three-byte fall-through gap at 0068BBDB..0068BBDE after
// the `_free`, which is why the export shows the tail as unreachable. The gap
// was not repaired; this follows the real control flow.
void destruct_in_game_interface_0068b630(InGameInterfaceManager& manager,
    InMissionInterfaceTeardownHost& host);

// 0068BC60, the scalar deleting destructor, __thiscall(this, byte flags) -> this
// in EAX, RET 4 at 0068BC7B. Bit 0 of `flags` frees the object.
void delete_in_game_interface_0068bc60(InGameInterfaceManager& manager,
    InMissionInterfaceTeardownHost& host, unsigned int flags);
inline constexpr unsigned int kScalarDeletingDestructorFreeBit = 1u;

// ---------------------------------------------------------------------------
// 5. What the mission exit calls on the manager
// ---------------------------------------------------------------------------

// The three sites inside BSP_Game_TeardownSessionState 004DA780 that touch the
// manager, in listing order. The drain of request 10h reaches 004DA780 from
// 004E458A (docs/GAME_EXECUTABLE.md milestone 2g). Read-only for this packet:
// 004DA780 is not leased and is not reconstructed here.
struct MissionExitInterfaceStep {
    std::uint32_t call_site{0};
    const char* effect{nullptr};
    bool guarded_by_manager_present{false};
};
inline constexpr std::size_t kMissionExitInterfaceStepCount = 3;
const MissionExitInterfaceStep& mission_exit_interface_step(std::size_t index) noexcept;

}  // namespace bsp
