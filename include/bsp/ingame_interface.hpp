#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/frontend_managers.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/frontend_states.hpp"
#include "bsp/simulation_gate.hpp"

// Packet in_mission_interface_manager. The fifth instance of the front-end
// manager base (00684E10), the one that lives at 00E198C4 for the duration of a
// mission and owns the whole in-session HUD.
//
// Addresses reconstructed here: 0068A990 (constructor), 0068CC70 (Init, the
// vtable +4h override), 0068ACA0 (ApplyPendingInterface, the vtable +10h
// override, listing only), 0068BC60 (scalar deleting destructor), 0068B630 (the
// destructor body), 0068AB80 (the level-2/level-3 collapse), 00689FA0 / 00689FC0
// / 00689FE0 (three canned level-1 input-context lists), and the creation site
// at 004E0452..004E0480 inside BSP_Game_LoadMissionScene (004DFB70).
//
// Evidence and uncertainties: docs/IN_MISSION_INTERFACE_MANAGER.md. Every name
// here is a hypothesis; no symbol was recovered from the image.
namespace bsp {

// ---------------------------------------------------------------------------
// In-session interface ids, 20h..35h
// ---------------------------------------------------------------------------

// Read back from the 00E08CD8 pointer table, which holds 36h entries, not the
// 45 that `kInterfaceNameCount` in frontend_managers.hpp currently records; ids
// 2Dh..35h below are the nine the shorter count drops. Because of that,
// `front_end_interface_name` returns nullptr for those nine today. Ids 00h..1Fh
// belong to the three front-end managers; 00683E90 splits the two ranges at
// `kFirstInGameInterface` (20h).
inline constexpr std::size_t kInterfaceNameTableEntries = 0x36; // 00E08CD8..00E08DAF

inline constexpr int kInterfaceScene3d = 0x20;          // INTF_SCENE3D
inline constexpr int kInterfaceMap = 0x21;              // INTF_MAP
inline constexpr int kInterfacePlane = 0x22;            // INTF_PLANE
inline constexpr int kInterfacePlaneBomber = 0x23;      // INTF_PLANEBOMBER
inline constexpr int kInterfacePlaneSpawn = 0x24;       // INTF_PLANESPAWN
inline constexpr int kInterfaceCaptain = 0x25;          // INTF_CAPTAIN
inline constexpr int kInterfaceBombView = 0x26;         // INTF_BOMBVIEW
inline constexpr int kInterfaceTBoatHelmsman = 0x27;    // INTF_TBOATHELMSMAN
inline constexpr int kInterfaceSubmarine = 0x28;        // INTF_SUBMARINE
// 29h is kInterfaceFreeCamera in simulation_gate.hpp.
inline constexpr int kInterfaceIdleCamera = 0x2A;       // INTF_IDLECAMERA
inline constexpr int kInterfaceMovieCamera = 0x2B;      // INTF_MOVIECAMERA
// 2Ch is kMovieCameraNewInterface in frontend_managers.hpp, the id the lock
// 00E19894 lets through untouched.
// 2Dh is kInterfaceEngineMovie in simulation_gate.hpp; the table entry reads
// INTF_ENGINEMOVIECAMERA, so that constant is named one word short.
inline constexpr int kInterfaceAirfield = 0x2E;         // INTF_AIRFIELD
inline constexpr int kInterfaceCommandBuilding = 0x2F;  // INTF_COMMANDBUILDING
inline constexpr int kInterfaceShipyardStareDumb = 0x30; // INTF_SHIPYARD_STAREDUMB
// 31h and 32h are kInterfaceShipyard and kInterfaceAirbase in simulation_gate.hpp.
inline constexpr int kInterfaceLaunchLanding = 0x33;    // INTF_LAUNCHLANDING
inline constexpr int kInterfaceLimbo = 0x34;            // INTF_LIMBO
inline constexpr int kInterfaceSupportManager = 0x35;   // INTF_SUPPORTMANAGER
inline constexpr int kLastInGameInterface = kInterfaceSupportManager;

// Names for 20h..35h, indexed by `id - kFirstInGameInterface`. Supplied here
// only because the shared table stops at 2Ch; ids below 20h stay with
// `front_end_interface_name`. Returns nullptr outside 20h..35h.
const char* in_game_interface_name(int interface_id) noexcept;

// 0068A140 is already reconstructed as `bsp::is_base_interface_0068a140`.

// ---------------------------------------------------------------------------
// The object, 108h bytes at 00E198C4
// ---------------------------------------------------------------------------

inline constexpr std::size_t kInGameInterfaceManagerSizeBytes = 0x108; // 004E0452
inline constexpr std::uint32_t kInGameInterfaceManagerVtable = 0x00CF7A60u;
// "interface/Textures/game.ats", loaded by Init and released by the destructor.
// The installed tree carries the compression variants game_dxt1.ats,
// game_dxt5_1.ats and game_dxt5_2.ats, so the literal is a stem.
inline constexpr const char* kInGameInterfaceAtlas = "interface/Textures/game.ats";

// One HUD screen owned by the manager. `offset` is its slot in the object,
// `registry_slot` the id its virtual +0h returns, which is the id the level-1
// screen-set lists below use and the index into FrontEndScreenTable (00E18B60).
struct InGameHudScreenSlot {
    std::uint16_t offset;        // byte offset inside the manager
    std::uint8_t registry_slot;  // vtable +0h return value
    std::uint32_t constructor;   // 0 when Init inlines the constructor
    std::uint32_t vtable;
    std::uint16_t size_bytes;    // the operator new argument
    std::uint8_t init_order;     // construction index inside 0068CC70
};

inline constexpr std::size_t kInGameHudScreenCount = 42;
extern const InGameHudScreenSlot kInGameHudScreens[kInGameHudScreenCount];

// Offset -> index into kInGameHudScreens, or kInGameHudScreenCount when the
// offset is not a screen slot. D0h is the one hole in the 40h..E8h run: neither
// the constructor nor Init writes it and the destructor never collects it.
inline constexpr std::uint16_t kInGameHudScreenGapOffset = 0xD0;
std::size_t in_game_hud_screen_index_at(std::uint16_t offset) noexcept;
// registry slot -> index, or kInGameHudScreenCount.
std::size_t in_game_hud_screen_index_for_slot(int registry_slot) noexcept;

// The manager layout. The first 40h bytes are the shared base; everything from
// 40h up is this class. Screen pointers are modelled as registry slots so the
// reconstruction stays free of native pointers.
struct InGameInterfaceManager {
    FrontEndManagerBase base{}; // +00h..+3Fh, 00684E10

    // +40h..+E8h. Index matches kInGameHudScreens; a slot is "constructed" once
    // Init has run. D0h is not represented because nothing writes it.
    bool screen_constructed[kInGameHudScreenCount]{};

    // +ECh, zeroed by the constructor (param_1[3Bh]) and never written again by
    // anything traced. Purpose unknown.
    std::uint32_t field_ec{0};

    // +100h, the ambient sound resource of the unit currently driving the HUD.
    // ApplyPendingInterface copies it from `unit->[538h] + 10Ch` with the
    // refcounted assign 004E7BB0 and compares it before restarting the sound.
    // Modelled as an opaque handle id; 0 is "none".
    std::uint32_t ambient_sound_source{0};

    // +104h, the playing instance of that sound. Released with virtual +8h(0)
    // followed by 0054D510.
    std::uint32_t ambient_sound_instance{0};

    // +FDh, a byte the constructor clears and nothing traced here reads.
    bool field_fd{false};
};

// ---------------------------------------------------------------------------
// The interface-id to screen-set map, 0068ACA0
// ---------------------------------------------------------------------------

// The level-1 screen ids an interface requests, and the level-1 input contexts
// it installs. Both lists are the native varargs lists in argument order, which
// is the reverse of the push order in the listing. An empty screen list clears
// level 1; a null input list means the arm does not touch the input contexts at
// all, which is not the same as clearing them.
struct InGameInterfaceScreenSet {
    const int* screen_ids{nullptr};
    std::size_t screen_count{0};
    const int* input_contexts{nullptr};
    std::size_t input_context_count{0};
    // False for 2Eh, 30h, 31h, 32h and 35h, which jump straight to the tail at
    // 0068B23A and leave the level untouched. That is different from the empty
    // list 21h and every out-of-range id send through 004F8530(0), which clears
    // level 1.
    bool sets_screen_set{false};
    bool sets_input_contexts{false};
};

// Lookup for 20h..35h. Ids outside that range take the same default arm as 21h:
// an empty screen list, `sets_screen_set == true`, no input-context call.
InGameInterfaceScreenSet in_game_interface_screen_set(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// The unit-type refinement, 0068AE0B
// ---------------------------------------------------------------------------

// Type codes handed to the payload's virtual +5Ch, in the order the listing
// tests them. The predicate is a kind-of test on the unit that is becoming the
// player's; the first match decides the interface.
inline constexpr int kUnitTypeTorpedoBoat = 0x0E;
inline constexpr int kUnitTypeSubmarine = 0x08;
inline constexpr int kUnitTypeShip = 0x06;
inline constexpr int kUnitTypePlane = 0x0F;
inline constexpr int kUnitTypeAirfield = 0x45;
inline constexpr int kUnitTypeCommandBuilding = 0x1C;
inline constexpr int kUnitTypeShipyard = 0x46;
inline constexpr int kUnitTypeCarrierGroupHost = 0x18;

// What the host must answer for the SCENE3D refinement.
struct InGameInterfaceUnitQuery {
    virtual ~InGameInterfaceUnitQuery() = default;
    // payload->vtable[5Ch](type_code) at 0068AE1C and its repeats.
    virtual bool unit_is_kind_of(int type_code) = 0;
    // 007BB9A0(unit) at 0068AE84. True selects INTF_PLANE, false INTF_PLANESPAWN.
    virtual bool plane_is_in_flight() = 0;
    // unit->[3D0h] at 0068AF18: the child unit a carrier-group host delegates to.
    // Returning false ends the chain.
    virtual bool has_delegate_unit() = 0;
};

// The result of the 20h arm. `redispatch_with_delegate` is the 18h case, which
// re-enters ApplyPendingInterface with 20h and the delegate unit rather than
// choosing an interface.
struct InGameSceneInterfaceChoice {
    int interface_id{0};
    bool chosen{false};
    bool redispatch_with_delegate{false};
};

// 0068AE0B. `has_unit` false is the null-payload arm: single player picks the
// bare scene set, multiplayer redispatches as INTF_IDLECAMERA (0068AFAB).
InGameSceneInterfaceChoice choose_scene_interface_0068ae0b(bool has_unit, bool multiplayer,
    InGameInterfaceUnitQuery& query) noexcept;

// ---------------------------------------------------------------------------
// 0068AB80, the level-2 and level-3 collapse
// ---------------------------------------------------------------------------

struct InGameInterfaceCollapseState {
    // The level-3 screen vector 00E18D18 through its head/tail pair
    // 00E18D1C/00E18D20; the arm runs only when it is non-empty.
    bool level3_non_empty{false};
    // [00E198C4 + BCh] + 30h, a flag on the camera screen (registry slot 4Eh).
    bool camera_screen_flag_30{false};
    // [this + BCh] + 0Ah, set to 1 when the flag above is set.
    bool camera_screen_flag_0a{false};
    // [this + 54h] + 4h: the requested byte of the registry slot 4Ch screen.
    // Level 2 is only collapsed while that screen is requested.
    bool screen_4c_wanted{false};
    bool level2_cleared{false};
    bool level3_cleared{false};
    bool extra_hook_ran{false}; // 0051E8E0, taken when `run_extra_hook` is set
};

// 0068AB80, __thiscall(this, char clear_level2, char run_extra_hook), RET 8.
// ApplyPendingInterface calls it with clear_level2 set only for the four camera
// interfaces 29h, 2Bh, 2Ch and 2Dh, and run_extra_hook always 1.
void collapse_in_game_overlays_0068ab80(InGameInterfaceCollapseState& state,
    bool clear_level2, bool run_extra_hook) noexcept;

// The predicate ApplyPendingInterface computes at 0068AD20 for that argument.
bool in_game_interface_is_camera_mode(int interface_id) noexcept;

// The predicate at 0068B255: which interfaces suppress the unit ambient sound.
// The camera modes plus INTF_LIMBO.
bool in_game_interface_suppresses_ambience(int interface_id) noexcept;

// ---------------------------------------------------------------------------
// The whole override, 0068ACA0
// ---------------------------------------------------------------------------

// One method per native call site that this reconstruction does not own, in the
// style of `bsp::run_application_frame`.
struct InGameInterfaceHost {
    virtual ~InGameInterfaceHost() = default;

    // 00684600, the base ApplyPendingInterface. False means the lock rejected
    // the request and the override returns immediately.
    virtual bool apply_base_request(int interface_id, bool has_payload) = 0;

    // payload->[5Dh] at 0068ACE8: the unit is gone, so the HUD goes to limbo.
    virtual bool payload_unit_is_dead() = 0;
    // [this + E8h]->00565FB0(payload->[70h]) at 0068ACF8, the limbo screen.
    virtual void limbo_screen_take_unit() = 0;

    // [this + 40h]->00646040(id, payload) at 0068AD07. Runs for every id, before
    // the switch, on the registry slot 44h screen.
    virtual void hud_root_screen_set_interface(int interface_id, bool has_payload) = 0;

    // game+1ED4h at 0068AD11, then 0042A930. The mission-tree overlay tick.
    virtual bool mission_overlay_present() = 0;
    virtual void tick_mission_overlay() = 0;

    // 004F8530 and 004D8A50, the level-1 setters.
    virtual void set_level1_screen_set(const int* ids, std::size_t count) = 0;
    virtual void set_level1_input_contexts(const int* ids, std::size_t count) = 0;

    // The per-arm screen hand-off calls, keyed by the manager offset they load
    // ECX from, so the mapping to the listing stays literal.
    virtual void screen_receive_unit(std::uint16_t manager_offset) = 0;

    // 00689FE0 / 00689FA0 / 00689FC0 do the level-1 input contexts for 27h, 25h
    // and 28h; they are already covered by set_level1_input_contexts, so the
    // host only needs the extra work those arms do. 00689FE0 is called through
    // this for INTF_TBOATHELMSMAN, and so on.

    // 00644230 on the registry slot 44h screen and the write-back at 0068B199.
    virtual int hud_root_screen_query() = 0;
    virtual void bomb_view_screen_bind(int value) = 0;
    virtual void bomb_view_screen_store(int value) = 0;

    // The ambient-sound tail, 0068B282..0068B38B.
    virtual std::uint32_t unit_ambient_sound_source() = 0;
    virtual void stop_ambient_sound() = 0;
    // Returns the new instance handle stored at +104h, 0 when nothing started.
    virtual std::uint32_t start_ambient_sound(std::uint32_t source) = 0;

    // 0068AB80.
    virtual void collapse_overlays(bool clear_level2, bool run_extra_hook) = 0;

    // The 20h arm's unit query.
    virtual InGameInterfaceUnitQuery& unit_query() = 0;
    virtual bool is_multiplayer() = 0;
    // Re-entry through the object's own virtual +10h.
    virtual void redispatch(int interface_id, bool has_payload) = 0;
};

// 0068ACA0, __thiscall(this, int id, void* payload) -> bool in AL, RET 8. The
// listing has no Ghidra function; the body runs 0068ACA0..0068B38F and the jump
// table for ids 20h..35h sits immediately after it at 0068B390.
bool apply_in_game_interface_0068aca0(InGameInterfaceManager& manager, InGameInterfaceHost& host,
    int interface_id, bool has_payload) noexcept;

}  // namespace bsp
