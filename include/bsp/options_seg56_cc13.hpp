// Packet cc13-seg56: script bindings of segment 56 (0088B410..008D3E00) that had neither a tag
// nor a reconstruction record. docs/CC13_SEG56.md has the per-function table.
//
// Every binding here opens with the same dead prologue (a `luaMW_<name> failed:` string built
// and freed, the one-time `luakod` guard), wraps the Lua state (00B66C00) and opens a call frame
// (00B679B0); argument i is 00B677E0(frame, i). That shell is not repeated below; the functions
// start at the first argument read. Every binding is `int __fastcall(lua_State* in ECX)` with a
// plain RET, and returns 00B66400, the number of values pushed on the frame. Where the frame is
// only pushed to by the binding itself that count is a literal here.
//
// Entities are what 00888AA0 resolves (LuaBindingArgumentReader::entity_at). None of these
// bindings tests the resolved pointer unless its comment says so; the reconstruction passes a
// null through exactly as the native does.
//
// Field reads and writes go through ImageAccess with the image offset, and each call the native
// makes is one named host method (address in the name). Field names are hypotheses from the Lua
// binding name and are not recovered identifiers.
#pragma once

#include <cstdint>
#include <string>

#include "bsp/engine_seg56_cc13.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp::seg56_options_cc13 {

using cc13_seg56::LuaArgs;
using cc13_seg56::LuaResults;

// The frame as the bindings that hand it to a callee see it: 00B66400 reports how many values
// the frame holds, which is the binding's return value.
struct LuaFrame : LuaResults {
    virtual int result_count() = 0; // 00B66400
};

// Raw image reads and writes at `base + offset`, and of absolute globals.
struct ImageAccess {
    virtual ~ImageAccess() = default;
    virtual std::uint8_t read_u8(void* base, std::uint32_t offset) = 0;
    virtual std::int32_t read_i32(void* base, std::uint32_t offset) = 0;
    virtual float read_f32(void* base, std::uint32_t offset) = 0;
    virtual void* read_ptr(void* base, std::uint32_t offset) = 0;
    virtual void write_u8(void* base, std::uint32_t offset, std::uint8_t value) = 0;
    virtual void write_i32(void* base, std::uint32_t offset, std::int32_t value) = 0;
    virtual void write_f32(void* base, std::uint32_t offset, float value) = 0;
    virtual std::uint16_t global_u16(std::uint32_t address) = 0;
    virtual void* global_ptr(std::uint32_t address) = 0;
    virtual void write_global_u8(std::uint32_t address, std::uint8_t value) = 0;
};

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;          // the game object
inline constexpr std::uint32_t kInGameInterfaceGlobal = 0x00E198C4u; // the in-game interface
inline constexpr std::uint32_t kSessionModeOffset = 0x1FE4u;       // game+1FE4h, 1 = host

// ---------------------------------------------------------------------------
// Session messages these bindings build
// ---------------------------------------------------------------------------

// 0088C84B..0088C883 (Bann) and 0088C9DB..0088CA13 (Permit): 0075B430(type D7h) on a stack
// message, then +4h = 1, vtable 00D035E0, byte +18h = the flag, then
// 0076A9F0(game+1EF0h, &message, 0); the base vtable 00CE4974 is restored afterwards.
struct SupportManagerBanMessage {
    static constexpr std::uint8_t kType = 0xD7;
    static constexpr std::uint32_t kVtable = 0x00D035E0u;
    std::int32_t dword_04{1};
    bool banned{false}; // +18h
};

// 0088FA02..0088FA42 (Activate) and 0088FC06..0088FC49 (Deactivate): 0075B430(type BFh), then
// word +18h = 0, +4h = 1, byte +1Ah = 0, vtable 00D02EC0, +1Ch = integer argument 1,
// byte +20h = 1 (Activate) / 0 (Deactivate); then 0077C2A0(entity, &message, 7, 0).
struct SpawnpointMessage {
    static constexpr std::uint8_t kType = 0xBF;
    static constexpr std::uint32_t kVtable = 0x00D02EC0u;
    std::int32_t dword_04{1};
    std::uint16_t word_18{0};
    std::uint8_t byte_1a{0};
    std::int32_t argument_1{0}; // +1Ch
    bool active{false};         // +20h
};
inline constexpr int kSpawnpointRouteFlags = 7; // PUSH 7 at 0088FA31 / 0088FC38

// ---------------------------------------------------------------------------
// The host: one method per call site kind
// ---------------------------------------------------------------------------
struct Seg56Host {
    virtual ~Seg56Host() = default;

    // --- support manager (in-game interface panel [[00E198C4]+BCh]) ---
    // 00A38DA0, __fastcall(CL = banned): stores 00F8AB6A, calls 00652A50/00652A40 and, when
    // banning, collapses the interface level and rebuilds the per-party support lists.
    virtual void support_manager_set_banned_00a38da0(bool banned) = 0;
    // 0075B430 + field stores + 0076A9F0(game+1EF0h, &message, 0), as described above.
    virtual void session_send_0076a9f0(const SupportManagerBanMessage& message) = 0;
    // 0068C0B0, __thiscall(interface, int), RET 4.
    virtual void interface_toggle_tactical_overlay_0068c0b0(void* interface_object, int argument) = 0;
    // 0065BBA0, __thiscall(panel, entity): 0065B1F0 removes the entity from the list at +1F8h.
    virtual void panel_list_remove_0065bba0(void* panel, void* entity) = 0;
    // 00665EF0, __thiscall(panel, entity): appends the entity to the list at +1F8h.
    virtual void panel_list_push_back_00665ef0(void* panel, void* entity) = 0;
    // 006529E0, __thiscall: MOV AL,[ECX+4] / RET.
    virtual bool panel_visible_006529e0(void* panel) = 0;
    // 006529D0, __thiscall(panel, int), RET 4: [ECX+2B4h] = int.
    virtual void panel_set_dword_2b4_006529d0(void* panel, std::int32_t value) = 0;

    // --- entity state ---
    // vtable+19Ch, __thiscall(entity, NativeString* name, LuaObject* frame).
    virtual void entity_clear_failure_vt19c(void* entity, const std::string& name, LuaFrame& frame) = 0;
    // 0093A390, __thiscall on entity+A20h.
    virtual void failure_block_reset_0093a390(void* failure_block) = 0;
    // vtable+1B4h, __thiscall(entity, int).
    virtual void entity_set_damaged_gfx_level_vt1b4(void* entity, std::int32_t level) = 0;
    // 0084CE60, __thiscall(site, byte), RET 4: +340h = byte.
    virtual void support_site_set_active_0084ce60(void* entity, bool active) = 0;
    // 0077C2A0, __thiscall(entity, message*, flags, out), RET 0Ch; 0075B430 builds the message
    // before it (the struct's fields) and the base vtable is restored after.
    virtual void route_message_0077c2a0(void* entity, const SpawnpointMessage& message, int flags, int out) = 0;
    // 008761E0, __thiscall(unit, int), RET 4.
    virtual void unit_set_numbering_008761e0(void* unit, std::int32_t number) = 0;
    // 0070EFD0, __thiscall(group, shape), RET 4.
    virtual void unit_group_apply_formation_shape_0070efd0(void* group, std::int32_t shape) = 0;
    // 0077A080(&message, group) builds FORMATION_SET from the group, 0077C880 on [group+14h]
    // sends it, then the message's two buffers (+28h, +2Ch) are freed (00BF6989 twice).
    virtual void unit_group_broadcast_formation_set_0077a080_0077c880(void* group) = 0;

    // --- planes and ships ---
    // 007B8CE0, __thiscall(plane, mode, flag), RET 8. The plane may be null (slot index > 4).
    virtual void plane_set_gear_mode_007b8ce0(void* plane, std::int32_t mode, std::int32_t flag) = 0;
    // vtable+38h of the sub-object at plane+72Ch, __thiscall, returns AL.
    virtual bool plane_controller_vt38(void* controller) = 0;
    // 0080FC30, __thiscall(unit) -> ST0.
    virtual double unit_reference_speed_0080fc30(void* unit) = 0;
    // vtable+38h on the entity -> ST0 (the current speed).
    virtual double entity_speed_vt38(void* entity) = 0;
    // 005F9470, __thiscall(panel, bool), RET 4: [ECX+8Ch] = bool.
    virtual void panel_set_byte_8c_005f9470(void* panel, bool value) = 0;
    // 0080E490, __thiscall: MOV EAX,[ECX+1018h].
    virtual void* unit_parts_object_0080e490(void* unit) = 0;
    // 0092BEA0 / 0092BE90, __thiscall: MOV EAX,[ECX+370h] / [ECX+374h].
    virtual std::int32_t parts_dword_370_0092bea0(void* parts) = 0;
    virtual std::int32_t parts_dword_374_0092be90(void* parts) = 0;
    // 00814520, __thiscall(unit), RET 0.
    virtual void unit_on_breakup_00814520(void* unit) = 0;
    // vtable+5Ch, __thiscall(entity, class id) -> AL: the class test.
    virtual bool entity_is_class_vt5c(void* entity, std::int32_t class_id) = 0;
    // 00953920, __thiscall(ship) -> int.
    virtual std::int32_t ship_count_catapulted_planes_00953920(void* ship) = 0;
    // 009539A0, __thiscall(ship, int), RET 4.
    virtual void ship_set_catapult_stock_009539a0(void* ship, std::int32_t stock) = 0;
    // 0077D600, __thiscall(entity, command object, target*, flags), RET 0Ch.
    virtual void entity_issue_command_0077d600(void* entity, std::uint32_t command_object,
        const SceneCommandTarget& target, std::int32_t flags) = 0;
    // 006E6210 / 006E6270, __fastcall(ECX = value) -> EAX: crew level to skill and back; both
    // read the session mode game+1FE4h.
    virtual std::int32_t skill_from_crew_level_006e6210(std::int32_t crew_level) = 0;
    virtual std::int32_t crew_level_from_skill_006e6270(std::int32_t skill) = 0;
    // vtable+128h (set skill, __thiscall(entity, int)) and +12Ch (get skill).
    virtual void entity_set_skill_vt128(void* entity, std::int32_t skill) = 0;
    virtual std::int32_t entity_skill_vt12c(void* entity) = 0;

    // --- air operations and shipyards ---
    // 006BCD20, __fastcall(ECX = entity, DL = require_active) -> block or null.
    virtual void* air_ops_block_006bcd20(void* entity, bool require_active) = 0;
    // 006BF5C0, __thiscall(block) -> int.
    virtual std::int32_t air_ops_block_status_006bf5c0(void* block) = 0;
    // 006BA4E0, __thiscall(slot): [ECX+2Ch] = 6.
    virtual void air_ops_slot_set_ready_006ba4e0(void* slot) = 0;
    // 006BA6C0, __thiscall(block): MOV EAX,[ECX+58h].
    virtual std::int32_t air_ops_block_plane_limit_006ba6c0(void* block) = 0;
    // 008457E0, __thiscall(shipyard, class id, count), RET 8.
    virtual void shipyard_consume_stock_008457e0(void* shipyard, std::int32_t class_id, std::int32_t count) = 0;

    // --- formations, squadrons, selection ---
    // 00925F20, __fastcall(CL).
    virtual void sentity_init_all_00925f20(bool flag) = 0;
    // 00778890 / 007788B0, __thiscall(unit) -> AL.
    virtual bool unit_is_formation_leader_00778890(void* unit) = 0;
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 007F1C00, __thiscall(squadron, base), RET 4.
    virtual void squadron_set_home_air_base_007f1c00(void* squadron, void* base) = 0;
    // 007BBBA0, __thiscall(plane).
    virtual void unit_request_ordnance_release_007bbba0(void* plane) = 0;
    // 007ED690, __thiscall(squadron, int), RET 4; `this` is the global [00E188D8].
    virtual void squadron_reequip_all_007ed690(void* squadron, std::int32_t ammo_type) = 0;
    // 007ED590, __thiscall(squadron).
    virtual void squadron_reset_travel_speed_007ed590(void* squadron) = 0;
    // 00645160, __fastcall(ECX = unit, DL = flag) -> AL.
    virtual bool unit_is_selectable_00645160(void* unit, bool flag) = 0;
    // 004CD0F0, __thiscall(game, bool, int, int), RET 0Ch.
    virtual void game_set_cinematic_mode_004cd0f0(void* game, bool on, std::int32_t second, std::int32_t third) = 0;
};

// ---------------------------------------------------------------------------
// Support manager and interface glue (0088C750..0088D2B0)
// ---------------------------------------------------------------------------

// BannSupportmanager 0088C750 / PermitSupportmanager 0088C8E0: 00A38DA0(1 / 0); when the
// session mode game+1FE4h is 1, the D7h message with the same flag. Returns 0 values.
int lua_bann_support_manager(ImageAccess& image, Seg56Host& host);
int lua_permit_support_manager(ImageAccess& image, Seg56Host& host);
// ShowHideSupportManager 0088CA70: 0068C0B0([00E198C4], 0). Reads no argument.
int lua_show_hide_support_manager(ImageAccess& image, Seg56Host& host);
// SupportManagerEnable 0088CBB0: entity arg0, boolean arg1; true removes the entity from the
// support panel's list (0065BBA0), false appends it (00665EF0). The panel is read after both
// arguments.
int lua_support_manager_enable(LuaArgs& args, ImageAccess& image, Seg56Host& host);
// IsSMVisible 0088CD70: pushes 006529E0(panel).
int lua_is_sm_visible(LuaResults& results, ImageAccess& image, Seg56Host& host);
// UnlockAllMission 0088D000: opens the frame and returns; the body is empty.
int lua_unlock_all_mission();
// SetBPCsodaSquadMereteHack 0088D2B0: 006529D0(panel, integer arg0).
int lua_set_bpc_soda_squad_merete_hack(LuaArgs& args, ImageAccess& image, Seg56Host& host);

// ---------------------------------------------------------------------------
// Failures, damage visuals, spawnpoints, numbering, formations (0088F3C0..008901C0)
// ---------------------------------------------------------------------------

// ClearFailure 0088F3C0: entity arg0, string arg1 copied into a native string, then
// vtable+19Ch(&name, &frame). Returns the frame's count, since the callee gets the frame.
int lua_clear_failure(LuaArgs& args, LuaFrame& frame, Seg56Host& host);
// ClearAllShipFailure 0088F5A0: 0093A390 on entity+A20h.
int lua_clear_all_ship_failure(LuaArgs& args, Seg56Host& host);
// SetDamagedGFXLevel 0088F710: vtable+1B4h(integer arg1).
int lua_set_damaged_gfx_level(LuaArgs& args, Seg56Host& host);
// GetDamagedGFXLevel 0088FCC0: pushes the dword at entity+358h.
int lua_get_damaged_gfx_level(LuaArgs& args, LuaResults& results, ImageAccess& image);
// ActivateSpawnpoint 0088F8B0 / DeactivateSpawnpoint 0088FAB0: with exactly one argument
// 0084CE60(entity, 1 / 0); otherwise integer arg1 goes into the BFh message routed through
// 0077C2A0, and 0084CE60 is not called.
int lua_activate_spawnpoint(LuaArgs& args, Seg56Host& host);
int lua_deactivate_spawnpoint(LuaArgs& args, Seg56Host& host);
// SetNumbering 0088FE30: 008761E0(entity, integer arg1).
int lua_set_numbering(LuaArgs& args, Seg56Host& host);
// SetFormationShape 0088FFD0: group = entity+284h; when non-null, 0070EFD0(group, integer
// arg1) then the FORMATION_SET broadcast. A null group skips the argument read too.
int lua_set_formation_shape(LuaArgs& args, ImageAccess& image, Seg56Host& host);
// GetFormationShape 008901C0: pushes group+500h, or -1 without a group.
int lua_get_formation_shape(LuaArgs& args, LuaResults& results, ImageAccess& image);

// ---------------------------------------------------------------------------
// Planes, ships, submarines (00890340..00896590)
// ---------------------------------------------------------------------------

// SetPlaneGears 00890340 / ForcePlaneGears 00890840: for every slot below the squadron's
// count +3CCh (re-read each pass) 007B8CE0(slot <= 4 ? [+3D0h + 4*slot] : null, integer arg1,
// 0 / 1).
int lua_set_plane_gears(LuaArgs& args, ImageAccess& image, Seg56Host& host);
int lua_force_plane_gears(LuaArgs& args, ImageAccess& image, Seg56Host& host);
// GetPlaneSurelyLanded 00890510: byte +904h set and state +900h 2 or 1.
int lua_get_plane_surely_landed(LuaArgs& args, LuaResults& results, ImageAccess& image);
// GetPlaneLandingAborted 008906A0: byte +904h clear and state +900h 2 or 4 gives true;
// otherwise the answer of the +72Ch controller's vtable+38h.
int lua_get_plane_landing_aborted(LuaArgs& args, LuaResults& results, ImageAccess& image, Seg56Host& host);
// GetShipMaxSpeed 00890BB0: pushes (float)0080FC30(entity).
int lua_get_ship_max_speed(LuaArgs& args, LuaResults& results, Seg56Host& host);
// GetShipSpeed 00890EF0: pushes (float)vtable+38h(entity).
int lua_get_ship_speed(LuaArgs& args, LuaResults& results, Seg56Host& host);
// SetForceSellableUnits 00891070: 005F9470([[00E198C4]+64h], boolean arg0).
int lua_set_force_sellable_units(LuaArgs& args, ImageAccess& image, Seg56Host& host);
// BreakShip 008914F0: 00814520(entity) when the parts object's +370h and then (fetched again)
// +374h are both non-zero.
int lua_break_ship(LuaArgs& args, Seg56Host& host);
// GetNumCatapulted 00892AA0: 0 for a null entity or one failing the class-6 test, else 00953920.
int lua_get_num_catapulted(LuaArgs& args, LuaResults& results, Seg56Host& host);
// SetCatapultStock 00892C30: for a non-null entity passing the class-5 test, 009539A0(integer
// arg1); the argument is read only on that path.
int lua_set_catapult_stock(LuaArgs& args, Seg56Host& host);
// SetPump 00893240: reads entity arg0 and boolean arg1 and does nothing with them.
int lua_set_pump(LuaArgs& args);
// GetSubmarineAirSupply 008938E0 / SetSubmarineAirSupply 00893A60: float entity+127Ch.
int lua_get_submarine_air_supply(LuaArgs& args, LuaResults& results, ImageAccess& image);
int lua_set_submarine_air_supply(LuaArgs& args, ImageAccess& image);
// ForceSubmarinePeriscope 00893DA0: byte entity+1235h = boolean arg1.
int lua_force_submarine_periscope(LuaArgs& args, ImageAccess& image);
// SubmarineAttack 00894440 / PilotTorpedo 008A5310: 0077D600(entity, command object,
// 0088A810(arg1), 1) with attackmove 00E08F78 / 00E08F18.
inline constexpr std::uint32_t kCommandObjectAttackMoveSeg56 = 0x00E08F78u; // 0089458C
inline constexpr std::uint32_t kCommandObjectPilotTorpedo = 0x00E08F18u;    // 008A5460
int lua_submarine_attack(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host);
int lua_pilot_torpedo(LuaArgs& args, LuaCommandTargetSource& targets, Seg56Host& host);
// SetCrewLevel 00894F20: vtable+128h(006E6210(integer arg1)).
int lua_set_crew_level(LuaArgs& args, Seg56Host& host);
// GetCrewLevel 008950D0: pushes 006E6270(vtable+12Ch()).
int lua_get_crew_level(LuaArgs& args, LuaResults& results, Seg56Host& host);
// GetSkillLevel 008953F0: pushes vtable+12Ch().
int lua_get_skill_level(LuaArgs& args, LuaResults& results, Seg56Host& host);
// GetAirBaseStatus 00895BA0: pushes 006BF5C0(006BCD20(entity, 0)).
int lua_get_air_base_status(LuaArgs& args, LuaResults& results, Seg56Host& host);
// GetAirBaseSlotStatus 00896220: pushes slot(+4Ch, 58h-byte records, 1-based arg1)+2Ch.
int lua_get_air_base_slot_status(LuaArgs& args, LuaResults& results, ImageAccess& image, Seg56Host& host);
// SetAirBaseSlotReady 00896590: 006BA4E0 on that slot.
int lua_set_air_base_slot_ready(LuaArgs& args, ImageAccess& image, Seg56Host& host);

// ---------------------------------------------------------------------------
// Shipyards, units, squadrons, navigator flags (00896EE0..008AFD50)
// ---------------------------------------------------------------------------

int lua_remove_shipyard_stock(LuaArgs& args, Seg56Host& host);                                 // 00896EE0
int lua_get_air_base_plane_limit(LuaArgs& args, LuaResults& results, Seg56Host& host);         // 008978D0
int lua_inferior_is_disabled(LuaArgs& args, LuaResults& results, ImageAccess& image);          // 00897E40
int lua_clear_wait(LuaArgs& args, ImageAccess& image);                                         // 00898320
int lua_init_all(Seg56Host& host);                                                             // 00898610
int lua_is_formation_leader(LuaArgs& args, LuaResults& results, Seg56Host& host);              // 00899810
int lua_is_formation_follower(LuaArgs& args, LuaResults& results, Seg56Host& host);            // 00899980
int lua_set_beach_fight(LuaArgs& args);                                                        // 0089ADB0
int lua_set_capture_value(LuaArgs& args, ImageAccess& image);                                  // 0089B0C0
int lua_get_command_building_level(LuaArgs& args, LuaResults& results, ImageAccess& image);    // 0089B6D0
int lua_get_squadron_landed(LuaArgs& args, LuaResults& results, ImageAccess& image);           // 0089CE60
int lua_plane_set_target_roll(LuaArgs& args, ImageAccess& image);                              // 0089D690
int lua_plane_set_target_pitch(LuaArgs& args, ImageAccess& image);                             // 0089D830
int lua_squadron_set_home_base(LuaArgs& args, Seg56Host& host);                                // 0089E220
int lua_squadron_set_old_style_bombing(LuaArgs& args, ImageAccess& image);                     // 0089EA00
int lua_plane_force_release(LuaArgs& args, ImageAccess& image, Seg56Host& host);               // 0089EBA0
int lua_plane_change_ammo_type(LuaArgs& args, ImageAccess& image, Seg56Host& host);            // 0089F1F0
int lua_squadron_set_ai_retreat_enabled(LuaArgs& args, ImageAccess& image);                    // 0089FCB0
int lua_squadron_enable_vehicle_avoidance(LuaArgs& args, ImageAccess& image);                  // 008A0000
int lua_squadron_enable_gunfire_avoidance(LuaArgs& args, ImageAccess& image);                  // 008A01B0
int lua_squadron_enable_player_takeoff(LuaArgs& args, ImageAccess& image);                     // 008A0360
int lua_squadron_disable_player_takeoff(LuaArgs& args, ImageAccess& image);                    // 008A04C0
int lua_squadron_restore_travel_speed(LuaArgs& args, Seg56Host& host);                         // 008A1A00
int lua_squadron_set_drop_all_bomb_percent(LuaArgs& args, ImageAccess& image);                 // 008A1F40
int lua_navigator_land_attacker_stop(LuaArgs& args, ImageAccess& image);                       // 008A2A50
int lua_navigator_allow_max_depth(LuaArgs& args, ImageAccess& image);                          // 008A32C0
int lua_navigator_force_move_close_to_target(LuaArgs& args, ImageAccess& image);               // 008A3460
int lua_is_unit_selectable(LuaArgs& args, LuaResults& results, Seg56Host& host);               // 008AB580
int lua_set_repair_effectivity(LuaArgs& args, ImageAccess& image);                             // 008AD8D0
int lua_pause(LuaArgs& args, ImageAccess& image, Seg56Host& host);                             // 008AFD50

} // namespace bsp::seg56_options_cc13
