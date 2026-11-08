// bsp_game.exe milestone 2m: the mission script's navigator, formation, skill,
// repair and role bindings, run through their recovered bodies.
//
// See include/bsp/game_hosts_script_orders.hpp for the address list and for what
// this file supplies rather than recovers.

#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/options_seg56_cc13.hpp"
#include "bsp/game_hosts_hud.hpp"
#include "bsp/game_sound_dialog_runtime.hpp"
#include "bsp/engine_seg56_cc13.hpp"
#include "bsp/oct18_tail_a.hpp"  // 00450750: active dialog keys
#include "bsp/band_0071_cc13.hpp"
#include "bsp/frontend_screen_sets.hpp"   // chain 648: GameInputContextManagerBinding
#include "bsp/game_hosts_frontend.hpp"    // goal 5 chain 652: the GUI manager's pages
#include "bsp/gui_layout_loader.hpp"      // goal 5 chain 652: GuiLayoutPage, 00AA7E00
#include <cstring>
#include <stdexcept>
#include <string>
namespace bsp::game { void harness_note_mission_end() noexcept; }   // src/game_hosts_mission.cpp
#include "bsp/game_hosts_lua.hpp"  // kReconLoadForceRefreshCc13Bound (cc13-reconload)
#include "bsp/strafe_task.hpp"
#include "bsp/depth_charge_task_cc13.hpp"
#include "bsp/rocket_task_cc13.hpp"   // kRocketTaskBound, packet cc9-rocket1
#include "bsp/game_hosts_commands.hpp"

#include "bsp/air_operations.hpp"
#include "bsp/air_ops_launch_flag_cc13.hpp"  // cc13-tier1, 006C5050 flag arm
#include "bsp/base_launch_axes_cc13.hpp"  // cc13-launchaxes, 006C5294 owner rows
#include "bsp/dive_bomb_task.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/game_hosts_world.hpp"
#include "bsp/scoring_bodies.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_fixed_step.hpp"
#include "bsp/in_mission_subsystem_tick.hpp"  // kFixedSimulationStepSeconds / Float
#include "bsp/command_execution.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/wire_script_orders_host_switches.hpp"
#include "bsp/cc13_seg52.hpp"
#include "bsp/cc13_seg48.hpp"
#include "bsp/cc13_left55_seg56b.hpp"   // goal 4 chain 365: 008AD540 / 008ACD10 bodies
#include "bsp/cc13_sweepB.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/game_hosts_message_manager.hpp"
#include "bsp/voice_playback.hpp"                // VoiceClipRecord, for 004483F0's record
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/game_hosts_mission_frame.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/gameplay_seg38_cc13.hpp"
#include "bsp/attack_commands.hpp"
#include <vector>
#include "bsp/attack_target_classify.hpp"
#include "bsp/ordnance_kinds.hpp"
#include "bsp/pilot_order_bindings.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/script_orders_ver_cc13.hpp"
#include "bsp/script_natives_fix5_cc13.hpp"   // packet cc13-fix5
#include "bsp/plane_retreat_task.hpp"  // packet cc9_plane_retreat_task
#include "bsp/kamikaze_task_host.hpp"   // packet cc9_lua52_kamikaze_wiring
#include "bsp/camera_decomposition.hpp"  // packet cc9_lua_camera_state
#include "bsp/mission_camera.hpp"        // packet cc9_lua_camera_state
#include "bsp/mission_lua_host.hpp"
#include "bsp/cc13_powerup.hpp"  // packet cc13-powerup
// Packet cc8_ship_follow: 00779D50's transcription and the ship-base kind.
#include "bsp/ship_ai_states.hpp"
#include "bsp/game_hosts_ship_ai.hpp"  // packet cc9_usn02_deruyter_fire
#include "bsp/game_hosts_ai.hpp"       // packet cc9_l65_chooser_surface_set
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_missionlua_cc13.hpp"
#include "bsp/weapon_director.hpp"     // packet g44_scan_row_write (0071D560)
#include "bsp/unit_gunnery_pass.hpp"
#include "bsp/recon_sensor_pass.hpp"  // packet cc9_recon_level_table
#include "bsp/mission_ver_cc13.hpp"   // packet cc9_l59_sq_avg_health
#include "bsp/racked_round_entity_cc13.hpp"  // packet cc13-rackround
#include "bsp/cc13_seg40.hpp"                // goal 5 chain 643: the narrative object
#include "bsp/cc13_seg48U.hpp"
#include "bsp/cc15_g24c.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_physical_file_date.hpp"  // 00435C40
#include "bsp/native_string.hpp"
#include "bsp/native_lua_class_setters.hpp"
#include "bsp/game_hosts_global_subsystems.hpp"
#include "bsp/weather_config.hpp"
#include "bsp/band_0040_cc13.hpp"
#include "bsp/seg3_4_cc13.hpp"
// Goal 5 chain 655: the segment-56 unit queries.
#include "bsp/band_0080_cc13.hpp"                 // 008457E0
#include "bsp/cc13_dyn1.hpp"                      // 00C336C0
#include "bsp/cc13_seg33a.hpp"                    // 006E6270
#include "bsp/shipyard_production.hpp"            // the shipyard registry
#include "bsp/left55_seg56a_host_unreached.hpp"   // 008A0F10
#include "bsp/left55_seg56c_host_unreached.hpp"   // 008BEBE0 (goal 5 chain 660)
#include "bsp/cc13_left55_seg56a.hpp"             // 0089E850 / 008A54C0
#include "bsp/unit_weapon_host_unreached.hpp"     // 0089A8B0 / 0089C590 / 0089C8F0
#include "bsp/left55_seg56b_host_unreached.hpp"   // 008ABAC0 / 008ABF90
#include "bsp/plane_squadron_entity.hpp"          // 007ECF70
#include "bsp/seg56_host_unreached.hpp"           // 008953F0 / 008950D0 / 00890BB0
#include "bsp/unit_instance_layout.hpp"           // kUnitRoleTableEntries
#include <algorithm>
#include <new>
#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace bsp::game {
namespace {

// Untracked, borrowed views: the selected Lua helpers read owner.state_04 and
// object.index_08 only. Positive slots remain stable while the helpers push keys.
struct ScriptOrdersLuaStackView {
    bsp::NativeLuaStateStorage owner{};
    bsp::NativeLuaObjectStorage object{};

    ScriptOrdersLuaStackView(lua_State* state, int slot) noexcept {
        owner.state_04 = state;
        object.owner_00 = &owner;
        object.kind_04 = 2;
        object.index_08 = slot;
    }
};

// Own the native string header consumed by the string setters. The native
// helpers borrow it; this adapter returns its buffer to the same allocator.
struct ScriptOrdersNativeText {
    bsp::NativeString text;

    explicit ScriptOrdersNativeText(const std::string& value) {
        text.resize_0041dd40(bsp::crt_string_storage(),
            static_cast<std::uint32_t>(value.size()), false);
        if (!value.empty()) std::memcpy(text.data(), value.data(), value.size());
    }
    ~ScriptOrdersNativeText() { text.release_to(bsp::crt_string_storage()); }
};

// The eight rows src/lua_binding_navigator.cpp reconstructs, with the binding
// table row address 006b8610 registers each one under. The three names that
// share 008a2f20's body are listed with their own rows because the table does.
struct ScriptOrderBinding {
    const char* name;
    std::uint32_t row_address;
};

// Packet cc9_bsm01_state_natives (docs/LUA_BINDING_MISSION.md, "BSM01's state
// natives"): three natives BSM01's scripted sinkings call. Each switch OFF leaves its
// native an unimplemented record.
//  * DisablePhysics 00891380: 0080E490 (unit+1018h, the force controller), then
//    byte +14h = BL (0089149E, MOV [EAX+14h],BL); 009329C9 then parks the body every step.
//  * AddMatrixInterpolator 008ADE00: argument 0 an entity, 1 and 2 two Vector3
//    tables, 3 a number, into 00905080 on the world (008ADF92), which the world
//    pass 00904600 applies through the entity's slot 88h.
//  * ExplodeToParts 0088E1B0: 0080E490, then 00935C70 detaches every part whose
//    health is above 0.
// ON since the BSM01 / USN01 / USN04 / USN02 pairs: identical (no call reached on
// any of them), so all three are bound but unexercised.
// Packet cc9_scripted_order_natives (docs/SHIP_AI_OPEN_ITEMS.md section 49).
// True binds three scripted orders; false leaves each one as it was.
//  * NavigatorEnable 008A7060: [unit+740h]+11h = argument 1, the enabled byte
//    of the ship AI controller's tick sub-node, so 008759C4 skips 009F50E0
//    (GameShipAiHost::set_navigator_enabled_0011). When false, it is an
//    unimplemented record.
//  * UnitHoldFire 008A6AC0: unit vtable[114h], then 0071BED0 (stance 0) with
//    no null check (008A6BE4). A squadron's +348h block takes the 0084D910 /
//    0084D930 answers for stance 0. A plane's vtable[114h] is 0047F180
//    (XOR EAX,EAX), so the image dereferences null at 0071BED6: counted, and
//    nothing is done. A ship's director stays a record, because its 5Ah
//    message consumer is the gunnery host's. When false, it is an
//    unimplemented record.
//  * EntityTurnToEntity 008A0A10, the arms after the squadron arm. For kind 6,
//    008A0DE9..008A0E68 sets translation = entity+FCh, then vtable[88h]
//    (006E00A0) and the physics body pose 00C56CF0. Otherwise, vtable[88h]
//    alone (a plane: 007C9540). When false, it is the old record at 008a0d1c.
// ON (2026-09-29, section 49.3): JM05 moved as predicted (Event2Pt stays at its
// spawn, 3000 steps skipped) and LOMP10 3000/9000 moved (PT 01 and 02 posed).
// IJN01 is gameplay identical and BSM01 identical. No death row moved.
inline constexpr bool kScriptedOrderNativesBound = true;
// Packet cc9_scripted_order_natives part 2 (section 50). True binds these two
// natives; false leaves each one an unimplemented record.
//  * SetShipMaxSpeed 00890A10: arg0 through 00888AA0 (no kind check), arg1 a
//    number, then 00890B51 FSTP [ESI+9C0h] (GameUnitsHost::set_unit_max_speed_09c0).
//  * PilotLand 008A47B0: 0077D600(entity, land 00E08FA0, target from arg1, 1)
//    at 008A4907. The planes' bots take it through 0099A3DD, with the site
//    from the command target (GameUnitsHost::land_at_site_0099a3dd, after the
//    delivery, as run_pilot_move_to_range does).
// kScriptedOrderNatives2Bound gates SetShipMaxSpeed. ON (section 50.2): BSM01
// moved as predicted (Whitney 494 -> 315 m, Tautog 187 -> 150 m, no death).
// kPilotLandNativeBound stays OFF (section 50.2). IJN01's target
// FindEntity("Airfield 02") answers nil in this host, which matches names
// case-sensitively, while the image's 009251F0 matches case-insensitively
// (the unit is "AirField 02"). So the IJN01 pair could not exercise the land
// install. Re-paired with FindEntity fixed (section 55.1): the tasks install
// at 68.20 s but retire at 68.30 s because the gunnery host's 0071EBF0
// refresh zeroes command_target_plus_one for units with no authored command
// row. ON (section 56.1) on top of kCommandTargetKeepUnauthoredBound: the four
// IJN01 tasks survive and fly toward AirField 02; death rows identical.
inline constexpr bool kScriptedOrderNatives2Bound = true;
// Packet cc13-ver12 (docs/CC13_VER_12.md section 1). True binds UnitFreeAttack
// 008A67E0 and UnitFreeFire 008A6950, the release half of UnitHoldFire: the
// same vt[114h] director route with the stance literal of 0071BF70 (2) or
// 0071BF20 (1) in place of 0071BED0's 0. False leaves both unimplemented
// records, as before, and every hold stays permanent.
inline constexpr bool kLuaUnitFreeFireBound = true;  // ON (cc9-lua71, 2026-10-24): faithful; unreached on USN01/USN10/USN04/USN03/PRCP01/BSM04 pairs (identical); see WINS_TABLE
// Packet cc9_l66_putto: PutTo 008AA131-008AA14D and PutFormationTo 008AA3CA turn a
// Lua degree value into vtable[11Ch]'s radians: FMUL [00CE3D28] (pi as a float,
// 400921FB60000000), FDIV [00CE3D20] (180.0), FSTP float. LABELLED: modelled with
// the main thread's 24-bit x87 precision, one rounding after each operation.
float put_to_heading_radians(double degrees) {
    const float scaled = static_cast<float>(degrees * 3.1415927410125732);
    return static_cast<float>(static_cast<double>(scaled) / 180.0);
}
// Packet cc9_l57_think_delay (docs/CC13_VER_8.md V3): 00929460 stores delay - step
// into +1E0h at 009294C7..009294D7 for each walked entity on the timed branch, and
// only then 009294E5 calls the think; a SetWait (008982C9) inside that think writes
// the new delay, first decremented on the next pass, and an entity registered this
// pass sits on the pending list and is not decremented. False: every armed entity
// loses one step after the walk, re-armed and new delays included.
inline constexpr bool kThinkDelayInWalkBound = true;   // ON: SQUADRON_LAND_TASK 5gv.1
// Packet cc13-ver12 (docs/CC13_VER_12.md section 2). True binds NavigatorStop
// 008A7400: 0077D600(entity, stop 00E08F88, zeroed descriptor, 1) at 008A7569,
// through entity_issue_command like the other navigator orders. False leaves
// it an unimplemented record, as before.
inline constexpr bool kLuaNavigatorStopBound = true;  // ON: SQUADRON_LAND_TASK 5hb
// Packet cc9_l65_chooser_surface_set (SQUADRON_LAND_TASK 5hy). 007EEC9F calls the
// thunk 00922DC0 (DL = 1) into 00922C80; a target past the ship, airfield,
// shipyard and 1Ch arms (an MLandFort: hangars, bunkers) reaches 008DDF90, the
// local slot's objective set, then the tail, whose allow_far arm accepts an
// MLandFort with FakedType 1Bh (the authored default; no row of this
// installation's Lua sets FakedType). True: that branch is answered as
// game_hosts_ship_ai.cpp's target_is_surface_00922dc0 answers it (slot 0's set,
// then the tail with allow_far). False: the branch answers "not surface", so
// every ordnance class fails and a squadron ordered onto a hangar gets 007F1688's
// moveto instead of divebomb.
inline constexpr bool kChooserSurfaceSetBranchBound = true;   // ON: SQUADRON_LAND_TASK 5hy.1
// Packet cc9_l65_turn_member (SQUADRON_LAND_TASK 5hz). 008A0A10 takes its squadron
// arm (008A0D5D vt[5Ch](18h)) only for a squadron argument. True: a member plane
// that is not the squadron's fused leader takes the plane arm (008A0E6F, its own
// vtable[88h] 007C9540) instead of re-posing the whole squadron. False: any
// member re-poses every member, as before. LABELLED: the leader plane and the
// squadron are one host unit, so a script passing the leader still turns all.
inline constexpr bool kTurnToEntityMemberPlaneArmBound = true;
// Packet cc9_l65_moveto_member_launch (SQUADRON_LAND_TASK 5ia). 007F1688-007F16AD
// issues the null-class moveto to the squadron (0071ECF0), whose 007ECF80 fan-out
// reaches every +3D0h plane, on the deck or flying. True: the standing per-member
// launch order (kAutoAttackMemberOnLaunchBound) also carries that moveto, so a
// deck member gets it when it is airborne. False: only the members flying at the
// serve get it (5hl L3), and later members fly no command.
inline constexpr bool kAutoAttackMoveToMemberLaunchBound = true;   // ON: SQUADRON_LAND_TASK 5ia.1
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 7). True binds NavigatorCruise
// 008A75C0: NavigatorStop's twin, 0077D600(entity, cruise 00E08F70, zeroed
// descriptor, 1) at 008A7729 (bsp::navigator_cruise_order_008a75c0), through
// entity_issue_command. False leaves it an unimplemented record, as before.
inline constexpr bool kLuaNavigatorCruiseBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 9). True binds PutFormationTo
// 008AA260 (unit, pos[, shape[, heading]]), single player: with shape 1..3 and the
// unit in a group, 0070EFD0(shape) on [unit+284h] (a record here: no host reshape
// by shape), then vtable[118h](pos) and, with exactly four arguments,
// vtable[11Ch](heading * pi / 180) - PutTo's placement (008193A0, the squadron's
// member placement) and its heading record. False: unimplemented, as before.
inline constexpr bool kLuaPutFormationToBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
inline constexpr bool kPilotLandNativeBound = true;
constexpr bool kDisablePhysicsBound = true;
constexpr bool kAddMatrixInterpolatorBound = true;
constexpr bool kExplodeToPartsBound = true;

constexpr ScriptOrderBinding kScriptOrderBindings[] = {
    {"SetDayTime", 0x008b1cc0u},
    {"GetDayTime", 0x008b1ec0u},
    {"DisablePhysics", 0x00891380u},
    {"AddMatrixInterpolator", 0x008ade00u},
    {"ExplodeToParts", 0x0088e1b0u},
    {"NavigatorAttackMove", 0x008a30d0u},
    // Packet cc7_pilot_order_bindings. The scripts' most-used order by an order
    // of magnitude - 1125 lines across 197 shipped files - and the one USN01
    // actually calls (`native PilotSetTarget argc=2 phase=luaStageInit`).
    {"PilotSetTarget", 0x008a4c90u},
    // Packet cc9_pilot_moveto_task: the escorts' order in usn_19_coralus.lua
    // (luaBombersSpawnedLex, difficulty 1-2). Routed only with
    // kPilotMoveToTaskBound; otherwise it stays a named record.
    {"PilotMoveToRange", 0x008a4590u},
    // Packet cc9_pilot_move_to: prcpjm08.lua:573 and :722. Handled only with
    // kPilotMoveToBound.
    {"PilotMoveTo", 0x008a4150u},
    // Packet cc9_usn02_deruyter_fire: luaSetScriptTarget's ship arm
    // (commandhelpers.lua:2941). Handled only with kScriptFireTargetBound.
    {"SetFireTarget", 0x0089a8b0u},
    // Packet cc9_pilot_move_on_path: jm06.lua:118 and :2392. Handled only with
    // kPilotMoveOnPathBound.
    {"PilotMoveOnPath", 0x008a3e70u},
    // Packet cc9_pilot_moveto_task part 1b: the same callback's turn and
    // stance. Handled only with kMissionTurnAndStanceBound.
    {"EntityTurnToEntity", 0x008a0a10u},
    {"UnitSetFireStance", 0x008a6490u},
    // Packet cc9_scripted_order_natives: handled only with
    // kScriptedOrderNativesBound.
    {"UnitHoldFire", 0x008a6ac0u},
    {"NavigatorEnable", 0x008a7060u},
    // Packet cc13-ver12: handled only with kLuaUnitFreeFireBound.
    {"UnitFreeAttack", 0x008a67e0u},
    {"UnitFreeFire", 0x008a6950u},
    // Packet cc13-ver12: handled only with kLuaNavigatorStopBound.
    {"NavigatorStop", 0x008a7400u},
    // Packet cc13-fix5: handled only with kLuaNavigatorCruiseBound.
    {"NavigatorCruise", 0x008a75c0u},
    // Packet cc13-fix5: handled only with kLuaPutFormationToBound.
    {"PutFormationTo", 0x008aa260u},
    // Packet cc9_scripted_order_natives part 2: SetShipMaxSpeed is handled only
    // with kScriptedOrderNatives2Bound, PilotLand only with kPilotLandNativeBound.
    {"SetShipMaxSpeed", 0x00890a10u},
    {"PilotLand", 0x008a47b0u},
    // Packet cc9_plane_retreat_task: handled only with bsp::kPlaneRetreatTaskBound.
    {"PilotRetreat", 0x008a4300u},
    // Packet cc8_navigator_path. The navigator sibling cc_lua_navigator left
    // out, with its two per-unit companions: 98, 18 and 18 calls on USN04, and
    // the eight script sites are the Lexington and the Town being told to circle
    // their authored patrol paths. 008a3600 is NOT a 0077d600 issue like the
    // four rows above it - it routes a 5Bh session message that the receiving
    // unit's weapon director turns into the moveonpath command.
    {"NavigatorMoveOnPath", 0x008a3600u},
    {"NavigatorSetAvoidLandCollision", 0x008a3b10u},
    {"NavigatorSetTorpedoEvasion", 0x008a3cd0u},
    // Packet cc9_navigator_ship_avoidance (docs/SHIP_AI_OPEN_ITEMS.md section 90).
    {"NavigatorSetAvoidShipCollision", 0x008a3970u},
    {"NavigatorMoveToRange", 0x008a2f20u},
    {"NavigatorMoveToPos", 0x008a2bc0u},
    {"NavigatorDirectMoveToRange", 0x008a2d70u},
    {"JoinFormation", 0x00899d10u},
    {"SetSkillLevel", 0x00895250u},
    {"RepairEnable", 0x008ad330u},
    // Packet cc9_ship_torpedo_mask_read: usn_2_java.lua 349/354 and 701/706
    // re-enable the Japanese destroyers' torpedo director after pass B cleared
    // it. Handled only with kShipDirectorEnablesBound.
    {"TorpedoEnable", 0x0089c8f0u},
    // Packet cc13-fix15: usn_09_leyte.lua 9-11 switch the Fortress1-3 artillery
    // off at load. Handled only with kLuaArtilleryEnableBound.
    {"ArtilleryEnable", 0x0089c590u},
    // Packet cc13-fix17: usn_03_battle_of_santa_cruz.lua:593 (Puffin, every
    // think). Handled only with kLuaForceFireDepthChargeBound.
    {"ForceFireDepthCharge", 0x0089b9d0u},
    // Packet cc13-fix17: jm10.lua:1862, jm12.lua:1783 and LOMP 08_lightning_strike
    // .lua:392. Handled only with kLuaSetSubmarineAirSupplyBound.
    {"SetSubmarineAirSupply", 0x00893a60u},
    // Packet l68_travel_speed: bsm_09_battle_of_the_coral_sea.lua (the
    // LexKillerGang, KMH(200)), BSM04, BSM10, LOMP03/08/11, ESMP01/02, USN11 and
    // JM12. Handled only with kLuaSquadronSetTravelSpeedBound.
    {"SquadronSetTravelSpeed", 0x008a0630u},
    // Packet cc9_ship_set_torpedo_stock: bsm_01_stationed_at_pearl.lua:1790 and
    // the COTP-IJN spawn tables call it. Handled only with
    // kShipSetTorpedoStockBound.
    {"ShipSetTorpedoStock", 0x0089eee0u},
    // Packet p17_reload_torpedoes: commandhelpers.lua 3531/3595/14801, jm03:1486,
    // jm07, jm08, jm11. Handled only with kReloadTorpedoesBound.
    {"ReloadTorpedoes", 0x008c3ce0u},
    {"SetRoleAvailable", 0x008ab850u},
    // Goal 5 chain 655: the two role tables SetRoleAvailable and the role
    // bookkeeping write, through fill_nine_integers.
    {"GetRoleAvailable", 0x008abac0u},
    {"GetRoleOwner", 0x008abf90u},
    // Packet cc_lua_binding_audit: the delayed-call scheduler and the four state
    // queries usn_2_java's objective checker reads. src/lua_binding_mission.cpp
    // and, for SetThink, src/lua_binding_core.cpp.
    {"CreateScript", 0x00898750u},
    {"SetThink", 0x00897fb0u},
    {"SetWait", 0x00898150u},
    {"ClearThink", 0x00898490u},
    {"DeleteScript", 0x00898ac0u},
    {"GetHpPercentage", 0x0088e9d0u},
    {"GetPosition", 0x008a7b00u},
    // Packet cc9_fill_path_points: bsm_01_stationed_at_pearl.lua:1571/1833,
    // usn_2_java.lua:867 (luaShowPath, never called) and five commandhelpers
    // helpers. Handled only with kFillPathPointsBound.
    {"FillPathPoints", 0x0089a190u},
    // Packet cc9_bsm01_think_natives: bsm_01_stationed_at_pearl.lua 1849..1858,
    // the PT boats and rescue craft onto their path points. Handled only with
    // kPutToBound.
    {"PutTo", 0x008a9f90u},
    // Packet l68_putrelto: handled only with kPutRelToBound.
    {"PutRelTo", 0x008aa560u},
    // Packet cc9_get_squadron_planes: bsm_04_vengance_at_luzon.lua:1717 and
    // 1720 (luaIntroMovieA). Handled only with kGetSquadronPlanesBound.
    {"GetSquadronPlanes", 0x0089cc50u},
    // Packet cc9_navigator_force_torpedo: jm06.lua:2582 (the two submarine
    // shots after the convoy movie) and competitive12/14. Handled only with
    // kNavigatorForceTorpedoBound.
    {"NavigatorForceTorpedo", 0x008a7200u},
    // Packet cc9_submarine_depth_level: 06_crucial_cargo.lua:700/713/755 and
    // eleven other shipped scripts. Handled only with
    // kSubmarineDepthLevelBound.
    {"GetSubmarineDepthLevel", 0x00894100u},
    // Packet cc9_recon_level_table: 06_crucial_cargo.lua:530 every second in
    // phase 1. Handled only with kReconLevelTableBound.
    {"ForceRecon", 0x008aadf0u},
    {"GetMeasure", 0x0088d8e0u},
    {"GameTime", 0x008a9320u},
    {"random", 0x0088c160u},
    // Packet cc8_usn04_strike_class. The body has been in src/lua_binding_core.cpp
    // since docs/LUA_BINDING_CORE.md; only this row was missing, so the binding
    // reported UNIMPLEMENTED and pushed nothing. A binding that pushes nothing is
    // not a neutral value to a Lua script: `Mission.Difficulty = GetDifficulty()`
    // left the field nil, and usn_19_coralus.lua gates its whole Japanese carrier
    // strike on `Mission.Difficulty == 0` / `== 1` / `== 2` (`:1382`, `:1454`,
    // `:1526`), so every one of the six striker launches was unreachable.
    // docs/USN04_STRIKE_CLASS.md.
    {"GetDifficulty", 0x008ae030u},
    // Packet cc8_lua_binding_routing_audit. Five more bodies that already exist
    // in src/lua_binding_core.cpp and had no row. Unlike GetDifficulty, none of
    // these changes a script branch: every one returns 0, so it pushes nothing
    // and the native pushes nothing either, which makes nil and no-value the
    // same thing to the caller. They are routed so the host's own records stop
    // reporting UNIMPLEMENTED for work that is reconstructed - a record that lies
    // in that direction is what hid the GetDifficulty gap.
    // `PrepareClass` 008C8F70 is deliberately NOT here although its body exists:
    // its host reader `resolve_global_integer` is a stub, so routing it would
    // claim work it does not do. docs/LUA_BINDING_ROUTING_AUDIT.md.
    {"SETLOG", 0x0088c620u},
    {"EnableMessages", 0x008cfe40u},
    {"LoadMessageMap", 0x008c61c0u},
    {"Music_Control_SetLevel", 0x008c4d10u},
    {"MusicFade", 0x008c7190u},
    {"Scoring_SetFinalScoringFunctionName", 0x008b8640u},
    // Packet cc_mission_blackout: the fade whose completion callback is the only
    // route from the intro movie to `luaIn`. src/mission_blackout.cpp.
    {"Blackout", 0x008d1340u},
    // Packet cc_lua_find_entity: usn_2_java.lua:54 is `this.Party = SetParty(this,
    // PARTY_ALLIED)`, so the mission's own party number is this binding's return
    // value and nothing else in the shipped scripts assigns it. The body is
    // already reconstructed in src/lua_binding_core.cpp.
    {"SetParty", 0x008a8930u},
    // Packet cc9_mission_end: the dialog registry at [game+21E4h]+1Ch.
    // StartDialog 008B0540 inserts through 00451A90 (008B0688), KillDialog
    // 008B0940 erases through 004514A0 (008B0A5F), and GetActDialogIDs 008CB730
    // collects the keys through 00450750 (008CB834) into a new array table
    // 1..n, pushed even when empty. luaClearDialogs' callback does
    // `pairs(GetActDialogIDs())`, so the old nil raised every pass.
    {"StartDialog", 0x008b0540u},
    {"KillDialog", 0x008b0940u},
    {"GetActDialogIDs", 0x008cb730u},
    // Packet cc9_lua_countdown: the countdown at [game+21E8h]. Handled only with
    // kLuaCountdownBound.
    {"Countdown", 0x008b16e0u},
    {"CountdownCancel", 0x008b19a0u},
    {"CountdownTimeLeft", 0x008b1b40u},
    // Chain 648: EnableInput's context 1Eh level on the application's input
    // manager. Handled only once bind_input_context_manager has run.
    {"EnableInput", 0x008afbd0u},
    // Goal 5 chain 652: the GUI manager's page list. Handled only once
    // bind_gui_frontend has run.
    {"IsGUIActive", 0x008ca010u},
    // Packet cc9_lua_capture_percentage: JM05.lua:5178-5216. Handled only with
    // kLuaCapturePercentageBound.
    {"GetCapturePercentage", 0x0089b840u},
    // Packet cc13-fix13: JM05 (PRCPIJN/jm05.lua:2779, :1262/:1263/:1283). Handled
    // only with kLuaGetCommandBuildingLevelBound / kLuaSetCommandBuildingOwnerPlayerBound.
    {"GetCommandBuildingLevel", 0x0089b6d0u},
    {"SetCommandBuildingOwnerPlayer", 0x008a8d90u},
    // Packet cc9_lua_mission_narrative: handled only with kLuaMissionNarrativeBound.
    {"MissionNarrative", 0x008b0c10u},
    {"MissionNarrativeEnqueue", 0x008b0c10u},
    // Goal 5 chain 643: the 007349C0 and 00734E50 posts.
    {"MissionNarrativeUrgent", 0x008b0e10u},
    {"MissionNarrativeOverride", 0x008b1010u},
    {"MissionNarrativeClear", 0x008b15b0u},
    {"MissionNarrativeSize", 0x008b0ac0u},
    {"EndScene", 0x008b01b0u},
    // Packet cc9_lua47_display_scores: handled only with kLuaDisplayScoresBound.
    {"DisplayScores", 0x008c20d0u},
    {"HideScoreDisplay", 0x008c24b0u},
    // Packet cc9_lua_camera_state: handled only with kLuaCameraStateBound.
    {"GetCameraState", 0x008bf6a0u},
    {"GetRotation", 0x008a7e60u},
    // Packet cc13-fix14b: handled only with kLuaGetShipMaxSpeedBound /
    // kLuaGetDirectionBound.
    {"GetShipMaxSpeed", 0x00890bb0u},
    {"GetDirection", 0x008a7cb0u},
    // Goal 5 chain 678: three live numeric arguments, one vector table.
    {"DoVector3", 0x00899300u},
    // Goal 5 chain 655: the skill index SetSkillLevel / SetCrewLevel write.
    {"GetSkillLevel", 0x008953f0u},
    {"GetCrewLevel", 0x008950d0u},
    // Goal 5 chain 655: the air-operations block's in-air plane limit (+58h) and a
    // slot's state (+4Ch slot +2Ch).
    {"SetAirBasePlaneLimit", 0x00897720u},
    {"GetAirBasePlaneLimit", 0x008978d0u},
    {"GetAirBaseSlotStatus", 0x00896220u},
    // Goal 5 chain 655: the shipyard's stock records (+774h).
    {"RemoveShipyardStock", 0x00896ee0u},
    // Goal 5 chain 655: EntityTurnToEntity's twin with a point for a target.
    {"EntityTurnToPosition", 0x008a0f10u},
    // Goal 5 chain 655: NavigatorStop's pilot twin (jm12.lua 1724 / 1858).
    {"PilotStop", 0x008a7780u},
    // Packet cc13-fix16: handled only with its own switch (kLuaPlaneForceReleaseBound,
    // kLuaSquadronForceReleaseBound, kLuaPilotBombBound, kLuaPlaneBombFallingTimeBound,
    // kLuaPlaneReloadBombPlatformsBound, kLuaGetPlaneIsReloadingBound).
    {"PlaneForceRelease", 0x0089eba0u},
    {"SquadronForceRelease", 0x0089e850u},
    {"PilotBomb", 0x008a4f00u},
    {"PlaneBombFallingTime", 0x008a54c0u},
    {"PlaneReloadBombPlatforms", 0x0089f080u},
    {"GetPlaneIsReloading", 0x008bebe0u},
    // Goal 5 chain 660: scripts/global/commandhelpers.lua 5507 / 5581.
    {"PilotGunFire", 0x008a50d0u},
    {"PilotTorpedo", 0x008a5310u},
    // Packet cc13-fix21: usn_11_protecting_iowa.lua 1296/1533 (and its PRCPUS copy),
    // prcp_11_patrol.lua 1302/1539. Handled only with kLuaGetSquadTorpedoesBound.
    {"GetSquadTorpedoes", 0x008bef80u},
    // Packet cc13-fix21: jm10.lua:1764 (luaJM10SaveMissionData, both copies).
    // Handled only with kLuaGetSubmarineAirSupplyBound.
    {"GetSubmarineAirSupply", 0x008938e0u},
    // Packet cc13-powerup: handled only with bsp::cc13_powerup::kPowerupGrantBound.
    {"AddPowerup", 0x008ee410u},
    {"PreparePowerup", 0x008ea610u},
    {"GetAvailablePowerups", 0x008eb350u},
};

// The id the first script entity takes. The created scene instances number from 1
// (milestone 2l), so the script entities start past any plausible instance count
// and a `Ptr` from one family is never mistaken for the other.
constexpr std::uint32_t kScriptEntityIdBase = 100000u;

// The records are handed to Lua as light userdata, so the storage must not move
// under a live `Ptr`; a deque keeps them in place. With kScriptEntityPoolUnboundedBound
// off, creation stops at this count; the native has no such bound, which is why the
// cap is reported rather than silent.
constexpr std::size_t kScriptEntityCapacity = 512;

// The adapter for the bodies of docs/LUA_BINDING_CORE.md that this host runs:
// SetThink's, 008980E8 / 0088A330, and, since packet cc8_usn04_strike_class,
// GetDifficulty's. Every other method belongs to a binding not dispatched here
// and records itself if it ever is; the two difficulty readers below do not,
// because GetDifficulty reaches them on every call and each has a source.
GameSoundDialogRuntime*& bound_music_dialog() noexcept {
    // This slot is first accessed only in the ON application binding. The
    // application retires it before destroying SoundServices.
    static GameSoundDialogRuntime* runtime = nullptr;
    return runtime;
}
GameSoundDialogRuntime& music_dialog() {
    auto* const runtime = bound_music_dialog();
    if (!runtime) throw std::logic_error("Mission music requires the application's dialog runtime");
    return *runtime;
}
class ScriptMusicFadeAdapter final : public bsp::cc13_seg56::MusicFadeHost {
public:
    void music_set_target_level(float level) override {
        music_dialog().set_music_target_level(level);
    }
};

// Chain 648: the application's input context manager (GameInputContextManagerBinding,
// the same native owner and configuration the action poll uses).
bsp::GameInputContextManagerBinding*& bound_input_context_manager() noexcept {
    static bsp::GameInputContextManagerBinding* manager = nullptr;
    return manager;
}

// The InterfaceHost of EnableInput 008AFBD0, whose one host call (bsp.py show
// 008afbd0) is 00A933F0 on 004BEC00's input manager with context 1Eh. The
// manager's table at owner+10h (count +14h) holds slot 1Eh once the input
// configuration has loaded: 00698A10's action parse ends by configuring action
// 128h in context 1Eh, and 00A93C80 grows the table to it. 00A933F0 itself
// writes base + index*4 unchecked, so a smaller table is refused here rather
// than written past. Every other InterfaceHost site belongs to another binding.
class ScriptInputContextBinding final : public bsp::cc13_seg56::InterfaceHost {
public:
    explicit ScriptInputContextBinding(bsp::GameInputContextManagerBinding& manager)
        : manager_(manager) {}

    void input_set_context_level(int context, int level) override {
        void* const owner = manager_.current_owner();   // 004BEC00
        const auto* const raw = static_cast<const std::uint8_t*>(owner);
        std::int32_t count = 0;
        std::memcpy(&count, raw + 0x14, sizeof count);
        if (context < 0 || context >= count) {
            throw std::logic_error("EnableInput: the input context table has no slot "
                                   + std::to_string(context));
        }
        manager_.set_level(owner, static_cast<std::uint32_t>(context),
                           static_cast<std::uint32_t>(level));   // 00A933F0
    }

    void music_set_target_level(float) override { unreached("00A77860"); }
    float map_cursor_x_554() override { unreached("map+554h"); }
    float map_cursor_z_558() override { unreached("map+558h"); }
    std::int32_t map_zoom_2c() override { unreached("map+2Ch"); }
    void map_force_cursor_pos(const bsp::cc13_seg56::Vec3&) override { unreached("0059AD20"); }
    void map_release_cursor_0059ad50() override { unreached("0059AD50"); }
    void map_set_node_visible_by_name(const std::string&, bool) override { unreached("005A4020"); }
    void map_set_unit_highlight(void*, bool) override { unreached("005AAAD0"); }
    void music_set_channel_byte_4e(int, bool) override { unreached("00A77840"); }
    void engine_movie_screen_black_bars(bool) override { unreached("005CB0F0"); }
    void movie_screen_black_bars(bool) override { unreached("005CBC30"); }
    bool input_record_enabled(int) override { unreached("input record +28h"); }
    float input_record_value(int) override { unreached("input record +24h"); }
    void options_save_input_modifiers() override { unreached("005F0B90"); }
    bool interface_present() override { unreached("00E198C4"); }
    bool limbo_screen_present() override { unreached("00E198C4+E8h"); }
    bool limbo_screen_active_byte4() override { unreached("limbo+4h"); }
    void* gui_find_widget(bsp::cc13_seg56::LuaArgs&, int) override { unreached("008C9F80"); }
    bool gui_widget_vcall_38(void*) override { unreached("widget vtable+38h"); }
    std::string gui_widget_raw_text(void*) override { unreached("widget+F4h"); }
    void marker_add_gui_highlight(void*, bool, bool) override { unreached("006DAE20"); }
    void marker_remove_gui_highlight(void*) override { unreached("006DC5A0"); }
    void gui_load_page_and_show(const std::string&) override { unreached("00AA5840"); }
    void objectives_toggle_disabled_byte(bool) override { unreached("00E198C4+60h"); }
    void hud_counters_hide_unit_hp(int) override { unreached("0052B570"); }
    void hud_counters_hide_score(int, int) override { unreached("0052B900"); }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("ScriptInputContextBinding: EnableInput does not reach ")
                               + site);
    }

    bsp::GameInputContextManagerBinding& manager_;
};

// Goal 5 chain 652: the GUI manager 004C12B0 whose page list IsGUIActive
// searches. The application's GameFrontendHost owns the one host manager and
// its registry; bound in game_hosts.cpp beside the input manager.
GameFrontendHost*& bound_gui_frontend() noexcept {
    static GameFrontendHost* frontend = nullptr;
    return frontend;
}

// The services of IsGUIActive 008CA010 (bsp.py show 008ca010): 008C9F80 twice
// over argument 0 (00B662B0, then 008C8C20's dotted lookup), the widget's
// vtable +38h, and 00B66450 for the answer. 008C8C20 resolves the first
// segment with 00AA3140 on 004C12B0's page list and each further segment with
// 00AA7E00 (src/engine_seg56_cc13.cpp gui_find_widget_by_dotted_path).
// The base +38h is 00A9E0D0: node +4Ch null -> false, else node +ACh > 0.0.
// 00AA8530 (vtable +34h) writes that factor as 1.0 or 0.0 at once; this host's
// projection of +34h is GuiLayoutWidget::visible (GameFrontendHost::
// set_widget_visible, and commit_page_visibility for a screen's pages, which
// 004f83b0 drives from the screen's applied byte), so +38h reads it back.
// A page root is the page's own widget (the GuiScreen 00AA3140 returns).
// Every other InterfaceHost site belongs to another binding.
class ScriptGuiQueryBinding final : public bsp::cc13_seg56::InterfaceHost,
                                    public bsp::cc13_seg56::GuiTreeHost,
                                    public bsp::cc13_seg56::LuaResults {
public:
    ScriptGuiQueryBinding(GameFrontendHost& frontend, std::function<void(bool)> push)
        : frontend_(frontend), push_(std::move(push)) {}

    // --- 008C9F80 / 008C8C20 ---
    void* gui_find_widget(bsp::cc13_seg56::LuaArgs& args, int index) override {
        return bsp::cc13_seg56::lua_gui_find_widget_from_string_arg(args, index, *this);
    }
    void* find_page(const std::string& name) override {
        GuiLayoutPage* page = frontend_.find_page_00aa3140(name);   // 00AA3140
        return page != nullptr ? page->root.get() : nullptr;
    }
    void* find_child(void* widget, const std::string& name) override {
        // 00AA7E00 over the current widget's direct children.
        return widget != nullptr
            ? find_child_by_name_00aa7e00(*static_cast<GuiLayoutWidget*>(widget), name)
            : nullptr;
    }
    bool gui_widget_vcall_38(void* widget) override {
        return static_cast<const GuiLayoutWidget*>(widget)->visible;   // 00A9E0D0
    }
    // --- 00B66450 ---
    void push_boolean(bool value) override { push_(value); }

    void music_set_target_level(float) override { unreached("00A77860"); }
    float map_cursor_x_554() override { unreached("map+554h"); }
    float map_cursor_z_558() override { unreached("map+558h"); }
    std::int32_t map_zoom_2c() override { unreached("map+2Ch"); }
    void map_force_cursor_pos(const bsp::cc13_seg56::Vec3&) override { unreached("0059AD20"); }
    void map_release_cursor_0059ad50() override { unreached("0059AD50"); }
    void map_set_node_visible_by_name(const std::string&, bool) override { unreached("005A4020"); }
    void map_set_unit_highlight(void*, bool) override { unreached("005AAAD0"); }
    void music_set_channel_byte_4e(int, bool) override { unreached("00A77840"); }
    void engine_movie_screen_black_bars(bool) override { unreached("005CB0F0"); }
    void movie_screen_black_bars(bool) override { unreached("005CBC30"); }
    void input_set_context_level(int, int) override { unreached("00A933F0"); }
    bool input_record_enabled(int) override { unreached("input record +28h"); }
    float input_record_value(int) override { unreached("input record +24h"); }
    void options_save_input_modifiers() override { unreached("005F0B90"); }
    bool interface_present() override { unreached("00E198C4"); }
    bool limbo_screen_present() override { unreached("00E198C4+E8h"); }
    bool limbo_screen_active_byte4() override { unreached("limbo+4h"); }
    std::string gui_widget_raw_text(void*) override { unreached("widget+F4h"); }
    void marker_add_gui_highlight(void*, bool, bool) override { unreached("006DAE20"); }
    void marker_remove_gui_highlight(void*) override { unreached("006DC5A0"); }
    void gui_load_page_and_show(const std::string&) override { unreached("00AA5840"); }
    void objectives_toggle_disabled_byte(bool) override { unreached("00E198C4+60h"); }
    void hud_counters_hide_unit_hp(int) override { unreached("0052B570"); }
    void hud_counters_hide_score(int, int) override { unreached("0052B900"); }
    void push_number(int) override { unreached("00B664B0"); }
    void push_nil() override { unreached("00B66430"); }
    void push_string(const std::string&) override { unreached("00B66710"); }
    void push_float(float) override { unreached("00B66480"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unreached("integer fields");
    }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unreached("vector3"); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unreached("vector3 fields");
    }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("ScriptGuiQueryBinding: IsGUIActive does not reach ")
                               + site);
    }

    GameFrontendHost& frontend_;
    std::function<void(bool)> push_;
};

// Goal 5 chain 663. StartDialog 008B0540, KillDialog 008B0940 and GetActDialogIDs
// 008CB730 on their bodies lua_start_dialog / lua_kill_dialog / lua_get_act_dialog_ids
// (src/engine_seg56_cc13.cpp). Their services are the dialog panel owner
// [[00E188A8]+21E4h], which this host keeps as its active-dialog map and parsed
// entries (the rows these bodies replace kept them the same way):
//  * 00451A90 dialog_start_from_lua, after argument 0 (008B0660);
//  * [00E188A8]+1FE4h, 0 in single player (game_session_mode), so the session
//    message E1 (0044C970) is not reached;
//  * 004514A0 dialog_kill;
//  * 00450750 dialog_active_ids, then 00B672F0 per id into the one result table.
// Every other ContentHost / PresentationHost site belongs to another binding.
// Goal 5 chain 714. bsp.py show 00450750 --asm: ECX is the dialog owner,
// [ECX+20h] is the sentinel, [sentinel] is begin, node+0Ch is an 8-byte
// NativeString key; RET 4 consumes the output vector (EDX is not an input).
// Borrow the existing registry's iterator and key bytes, preserving its
// case-insensitive map order. Only the fields the body reads are projected;
// 00448C10 advances the real map iterator, not a fabricated native tree.
// These are source-level bindings, not original-ABI replacements.
template<class DialogMap>
class ScriptDialogIdCollection final : public bsp::oct18_tail_a::DialogServices {
    struct Key {
        std::uint32_t size;
        const char* data;
    };
    struct Node {
        Node* first{};
        void* unused_parent{};
        void* unused_right{};
        Key key{};
    };
    struct MapView {
        std::uint32_t unused{};
        Node* head{};
    };
    struct Owner {
        std::uint8_t unused[0x1c]{};
        MapView map;
    };
    static_assert(sizeof(void*) == 4);
    static_assert(sizeof(Key) == 8);
    static_assert(offsetof(Node, key) == 0x0c);
    static_assert(offsetof(Owner, map) + offsetof(MapView, head) == 0x20);
public:
    explicit ScriptDialogIdCollection(const DialogMap& dialogs)
        : current_(dialogs.begin()), end_(dialogs.end()) {
        owner_.map.head = &sentinel_;
        sentinel_.first = current_node();
        refresh_key();
        ids_.reserve(dialogs.size());
    }
    std::vector<std::string> collect() {
        bsp::oct18_tail_a::collect_active_dialog_ids_00450750(&owner_, &ids_, *this);
        return std::move(ids_);
    }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("00450750: invalid active-dialog iterator");
    }
    void append_native_string_00450540(void* output, const void* key) override {
        if (output != &ids_ || key != &node_.key || current_ == end_)
            throw std::logic_error("00450540: invalid active-dialog key");
        // 00450540 appends a copy; the borrowed key remains owned by the map.
        ids_.emplace_back(node_.key.data, node_.key.size);
    }
    void increment_tree_iterator_00448c10(bsp::oct18_tail_a::DialogIterator& it) override {
        if (it.owner != &owner_.map || it.node != &node_ || current_ == end_)
            throw std::logic_error("00448C10: invalid active-dialog iterator");
        ++current_;
        refresh_key();
        it.node = current_node();
    }
private:
    Node* current_node() { return current_ == end_ ? &sentinel_ : &node_; }
    void refresh_key() {
        if (current_ != end_) {
            node_.key = {static_cast<std::uint32_t>(current_->first.size()),
                         current_->first.data()};
        }
    }
    typename DialogMap::const_iterator current_, end_;
    Owner owner_;
    Node sentinel_, node_;
    std::vector<std::string> ids_;
};

class ScriptDialogBinding final : public bsp::cc13_seg56::DialogStartHost,
                                  public bsp::cc13_seg56::ContentHost,
                                  public bsp::cc13_seg56::PresentationHost,
                                  public bsp::cc13_seg56::LuaStringListResults {
public:
    struct Calls {
        std::function<void(const std::string&, int)> start;           // 00451A90
        std::function<std::int32_t()> session_mode;                     // game+1FE4h
        std::function<void(const std::string&)> kill;                  // 004514A0
        std::function<std::vector<std::string>()> active_ids;          // 00450750
        std::function<void(const std::vector<std::string>&)> push_ids; // 00B672F0 table
    };
    explicit ScriptDialogBinding(Calls calls) : calls_(std::move(calls)) {}

    // --- DialogStartHost ---
    void dialog_start_from_lua(const std::string& name, bsp::cc13_seg56::LuaArgs&,
                               int index) override {
        calls_.start(name, index);
    }
    std::int32_t game_session_mode_1fe4() override { return calls_.session_mode(); }
    void session_send_dialog_e1(const std::string&, bsp::cc13_seg56::LuaArgs&, int) override {
        unreached("0044C970 (session message E1)");
    }
    // --- ContentHost ---
    void dialog_kill(const std::string& name) override { calls_.kill(name); }
    int narrative_size() override { unreached("00733260"); }
    void narrative_clear() override { unreached("00734FA0"); }
    void localization_register_table_name(const std::string&) override { unreached("00AA0D30"); }
    void localization_reload_tables(int) override { unreached("00AA06D0"); }
    void atmosphere_replace_sky_box(const std::string&) override { unreached("00BBDF60"); }
    void rain_set_drop_num_percent(float) override { unreached("008003B0"); }
    void rain_settings_store_70(float) override { unreached("rain settings +70h"); }
    void time_of_day_set(int, int, int, int) override { unreached("004439B0"); }
    void time_of_day_split(int&, int&, int&) override { unreached("00443A30"); }
    void light_store_float(std::size_t, float) override { unreached("game+5FCh"); }
    void scene_ensure_default_lighting() override { unreached("004C9EC0"); }
    std::int32_t vehicle_class_remap(int) override { unreached("00437F50 +2010h"); }
    void narrative_send_to_party(int, const std::string&) override { unreached("00734B10"); }
    void narrative_send_to_player(int, const std::string&) override { unreached("00734BF0"); }
    void dialog_break(const std::string&) override { unreached("00451B90"); }
    void point_effect_stop_children(void*) override { unreached("00867B10"); }
    void point_effect_set_byte9(void*) override { unreached("effect+9"); }
    void effect_manager_stop_matching(void*) override { unreached("004D1100"); }
    // --- PresentationHost ---
    std::vector<std::string> dialog_active_ids() override { return calls_.active_ids(); }
    float voice_current_level_d4() override { unreached("voice +D4h"); }
    void voice_begin_sound_fade(float, float, const std::string&) override {
        unreached("005B9760");
    }
    float global_config_default_fade_e0() override { unreached("00432650 +E0h"); }
    void set_skip_movie_name(const std::string&) override { unreached("game+21E0h +19Ch"); }
    void* lua_table_entity_ptr(bsp::cc13_seg56::LuaArgs&, int) override { unreached("00888D20"); }
    void entity_set_force_3d_visible_2f0(void*, bool) override { unreached("entity+2F0h"); }
    void listener_add(const std::string&, const std::string&, bsp::cc13_seg56::LuaArgs&,
                      int) override {
        unreached("00980C10");
    }
    void listener_remove(const std::string&, const std::string&) override {
        unreached("00980D30");
    }
    bool listener_active(const std::string&, const std::string&) override {
        unreached("00980E00");
    }
    std::int32_t game_loading_depth_644() override { unreached("game+644h"); }
    void please_wait_screen_hide() override { unreached("0060D3E0"); }
    void game_set_cinematic_mode(int, int, int) override { unreached("004CD0F0"); }
    // --- LuaStringListResults ---
    void push_string_array(const std::vector<std::string>& values) override {
        calls_.push_ids(values);
    }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("ScriptDialogBinding: the dialog rows do not reach ")
                               + site);
    }
    Calls calls_;
};

// Goal 5 chain 663. DisplayScores 008C20D0 and HideScoreDisplay 008C24B0 on their bodies
// lua_display_scores / lua_hide_score_display (src/engine_seg56_cc13.cpp). The bodies
// read the arguments (00B66290 / 00B662B0) and call the HUD counters on
// [[00E198C4]+C8h]: 0052B780 for six arguments (008C22DA), 0052B620 for four
// (008C241A), 0052B900 to hide (008C25E5). Goal 5 chain 667: the dispatch's calls
// run those senders on the HUD host's counters screen. Every other HudScoreHost /
// InterfaceHost site belongs to another binding.
class ScoreDisplayBinding final : public bsp::cc13_seg56::HudScoreHost,
                                  public bsp::cc13_seg56::InterfaceHost {
public:
    struct Calls {
        // 0052B620 (pair false) / 0052B780 (pair true, with e and f).
        std::function<void(int, int, const std::string&, const std::string&, bool, int, int)>
            show;
        std::function<void(int, int)> hide;   // 0052B900
    };
    explicit ScoreDisplayBinding(Calls calls) : calls_(std::move(calls)) {}

    // --- HudScoreHost ---
    void display_score(int a, int b, const std::string& c, const std::string& d) override {
        calls_.show(a, b, c, d, false, 0, 0);
    }
    void display_score_pair(int a, int b, const std::string& c, const std::string& d, int e,
                            int f) override {
        calls_.show(a, b, c, d, true, e, f);
    }
    std::vector<void*> entity_list_0088a900(bsp::cc13_seg56::LuaArgs&, int) override {
        unreached("0088A900");
    }
    void display_unit_hp(int, const std::vector<void*>&, const std::string&) override {
        unreached("0052B400");
    }
    // --- InterfaceHost ---
    void hud_counters_hide_score(int first, int second) override { calls_.hide(first, second); }
    void music_set_target_level(float) override { unreached("00A77860"); }
    float map_cursor_x_554() override { unreached("map+554h"); }
    float map_cursor_z_558() override { unreached("map+558h"); }
    std::int32_t map_zoom_2c() override { unreached("map+2Ch"); }
    void map_force_cursor_pos(const bsp::cc13_seg56::Vec3&) override { unreached("0059AD20"); }
    void map_release_cursor_0059ad50() override { unreached("0059AD50"); }
    void map_set_node_visible_by_name(const std::string&, bool) override { unreached("005A4020"); }
    void map_set_unit_highlight(void*, bool) override { unreached("005AAAD0"); }
    void music_set_channel_byte_4e(int, bool) override { unreached("00A77840"); }
    void engine_movie_screen_black_bars(bool) override { unreached("005CB0F0"); }
    void movie_screen_black_bars(bool) override { unreached("005CBC30"); }
    void input_set_context_level(int, int) override { unreached("00A933F0"); }
    bool input_record_enabled(int) override { unreached("input record +28h"); }
    float input_record_value(int) override { unreached("input record +24h"); }
    void options_save_input_modifiers() override { unreached("005F0B90"); }
    bool interface_present() override { unreached("00E198C4"); }
    bool limbo_screen_present() override { unreached("00E198C4+E8h"); }
    bool limbo_screen_active_byte4() override { unreached("limbo+4h"); }
    void* gui_find_widget(bsp::cc13_seg56::LuaArgs&, int) override { unreached("008C9F80"); }
    bool gui_widget_vcall_38(void*) override { unreached("widget vtable+38h"); }
    std::string gui_widget_raw_text(void*) override { unreached("widget+F4h"); }
    void marker_add_gui_highlight(void*, bool, bool) override { unreached("006DAE20"); }
    void marker_remove_gui_highlight(void*) override { unreached("006DC5A0"); }
    void gui_load_page_and_show(const std::string&) override { unreached("00AA5840"); }
    void objectives_toggle_disabled_byte(bool) override { unreached("00E198C4+60h"); }
    void hud_counters_hide_unit_hp(int) override { unreached("0052B570"); }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("ScoreDisplayBinding: the score rows do not reach ")
                               + site);
    }
    Calls calls_;
};

// Goal 5 chain 663. GetCameraState 008BF6A0 on its body lua_get_camera_state_008bf6a0
// (src/engine_seg56_cc13.cpp) over the camera node the caller chose (the world matrix at
// [game+19FCh]+F0h after 00B6DB70, bsp.py ghidra disasm 008bf6a0: 008BF799 / 008BF7BC):
// +30h of it is the translation row (008BF7BC LEA EDX,[ESI+30h] into 0088BA30), 0042D2E0
// gives the basis angles in radians, and Zoom is [00F889B4] / node+1C4h. The table is
// built by `push`. Every other CameraStateHost site belongs to another binding.
class CameraStateBinding final : public bsp::cc13_seg56::CameraStateHost,
                                 public bsp::cc13_seg56::CameraStateResults {
public:
    CameraStateBinding(const bsp::CameraMatrix& world, float fov,
                       std::function<void(const bsp::cc13_seg56::CameraStateResult&)> push)
        : world_(world), fov_(fov), push_(std::move(push)) {}

    bsp::cc13_seg56::Vec3 game_camera_vector_30() override {
        return {world_[12], world_[13], world_[14]};
    }
    bsp::cc13_seg56::Vec3 game_camera_basis_angles() override {
        bsp::CameraMatrix basis = world_;
        bsp::cc13_seg56::Vec3 a;
        bsp::extract_camera_matrix_angles_0042d2e0(basis, a.x, a.y, a.z);
        return a;
    }
    float game_camera_fov_1c4() override { return fov_; }
    float base_fov_00f889b4() override { return bsp::kFovDivisor00f889b4; }
    void push_camera_state(const bsp::cc13_seg56::CameraStateResult& state) override {
        push_(state);
    }

    bsp::cc13_seg56::Vec3 game_camera_world_position() override { unreached("+120h..+128h"); }
    bsp::cc13_seg56::Vec3 game_camera_direction_110() override { unreached("+110h..+118h"); }
    bsp::cc13_seg56::Vec3 entity_world_position(void*) override { unreached("entity +FCh"); }
    std::array<float, 2> camera_direction_angle_delta(const bsp::cc13_seg56::Vec3&,
                                                      const bsp::cc13_seg56::Vec3&) override {
        unreached("00794750");
    }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("CameraStateBinding: GetCameraState does not reach ")
                               + site);
    }
    bsp::CameraMatrix world_;
    float fov_;
    std::function<void(const bsp::cc13_seg56::CameraStateResult&)> push_;
};

class SetThinkCoreHost final : public bsp::LuaBindingCoreHost {
public:
    explicit SetThinkCoreHost(GameScriptOrdersHost& owner) : owner_(owner) {}

    void entity_set_think_script_name(void* entity, const std::string& name) override {
        owner_.entity_set_think_script_name_0088a330(entity, name);
    }

    // game+1FE4h. Not a placeholder: this process already asserts a campaign
    // session in two other places, `non_campaign_session()` returning false and
    // `inputs.non_campaign_session = false`, both in src/game_hosts_mission.cpp,
    // and 006CDC70 gates its sub-updates on the same word. Zero is what those say.
    int game_non_campaign_flag() override { return 0; }
    // game+6ACh, the effective difficulty, read at 008AE12A. Packet
    // cc9_difficulty: `MissionStart::set_effective_difficulty` now stores what
    // 0058BF37 / 0058BF58 would, so this reads it back. With the switch off it
    // is the old constructor zero, which sent every mission down its
    // difficulty-0 arms (usn_19_coralus.lua :70, :2582; usn_1_marshall.lua :180).
    int game_effective_difficulty() override {
        return kSkillLevelBound ? game_effective_difficulty_6ac() : 0;
    }
    void log_prepare_class(int) override {}
    bool resolve_global_integer(const std::string&, int&) override { return false; }
    void vehicle_class_mark_party_required(int, int) override {}
    void vehicle_class_get_or_create(int, bool) override {}
    void music_director_set_level(int level) override {
        // 00A788A0 calls the actual published manager's level body; OFF keeps the no-op.
        music_dialog().set_music_level(level);
    }
    void session_send_music_level(int) override {}
    // SetParty's virtuals and local route. Chain 298 binds the kind query to
    // the created unit's class chain; OFF retains the previous direct arm.
    bool entity_vcall_5c(void* entity, int selector) override {
        // 008A8A83 queries IsKindOf(2). The created unit's recovered
        // class chain supplies this virtual; a script entity is kind 3
        // under kind 1 and continues through the direct +2Ch arm.
        return static_cast<bsp::LuaBindingNavigatorHost&>(owner_)
            .entity_is_kind_of(entity, selector);
    }
    void entity_vcall_2c(void* entity, int party, std::uint32_t field_58) override {
        static_cast<void>(field_58);
        if constexpr (kSceneRaceAndScriptIdentityBound) {
            // Packet cc9_scene_race_and_script_identity: the script entity's
            // slot is 00928F50; 008A8ADF pushes the entity's own +58h, so the
            // race is handed back unchanged.
            if (owner_.set_script_entity_party_00928f50(entity, party)) return;
        }
        if constexpr (kSetPartyUnitBound) {
            // Packet cc9_lua46_set_party_unit: a unit's slot, 00951F30 -> 00928F50.
            if (owner_.set_unit_party_00928f50(entity, party)) return;
        }
        owner_.record_unimplemented("LuaBindingCore::entity_set_party_vtable_2c",
            "008a8ae3");
    }
    void session_route_party_message(void* entity, int party) override {
        const auto* messages = commands_session_messages();
        const int copy_party = messages != nullptr
            ? static_cast<int>(messages->queue_vehicle_party_7f(
                static_cast<std::uint32_t>(party))) : party;
        // 008A8AC4 routes flags 7 through the same local loopback used by
        // formation orders. entity_object_id is this host's existing
        // unit-index+1 handle. Capture the long-lived owner, not this
        // temporary SetThinkCoreHost. The setter reads the current race
        // and mirrors Lua at delivery (0095ABE0's 7Fh arm).
        auto* owner = &owner_;
        const auto id = static_cast<bsp::LuaCommandTargetSource&>(owner_)
            .entity_object_id(entity);
        if (id != 0 && commands_post_loopback_callback_0076e520(
                static_cast<std::size_t>(id) - 1u, [owner, entity, copy_party] {
                    owner->set_unit_party_00928f50(entity, copy_party);
                })) return;
        // As in formation delivery, a standalone host without a live
        // commands queue retains its existing immediate setter.
        owner_.set_unit_party_00928f50(entity, copy_party);
    }
    void scoring_set_real_play_time_running(bool) override {}
    void scoring_set_final_scoring_function_name(const std::string& name) override {
        // 0090BFC0 stores the binding's name on its scoring owner; OFF keeps the no-op.
        owner_.set_final_scoring_function_name(name);
    }
    void message_map_load(const std::string& name, int side) override {
        // 008C62F6..008C6308: 00706760 on [game+21DCh] with the two Lua
        // arguments; OFF keeps the no-op.
        owner_.load_mission_message_map(name, side);
    }
    void call_0088b6d0_0076a9f0_00765590(const std::string&, int) override {}
    void set_entity_message_suppression(void*, bool) override {}
    void set_global_message_suppression(bool) override {}
    void message_system_drain_queue() override {
        // 0096BF70 (008D000A): ON calls game_warning_clear_waiting_queue, which
        // runs warning_manager_clear_waiting_0096bf70 over the mission frame's
        // pending warning queue; OFF keeps the no-op.
        game_warning_clear_waiting_queue();
    }

private:
    GameScriptOrdersHost& owner_;
};

// Goal 4 chain 331: the UnloadMessageMap binding 008C6490 through its body
// bsp::cc13_seg56::lua_unload_message_map. The body reads two arguments and the
// session word; every other accessor of the two seg56 interfaces belongs to a
// different binding and is refused rather than answered.
[[noreturn]] void unload_message_map_unreached(const char* what) {
    throw std::logic_error(std::string("UnloadMessageMap 008C6490: unreached ") + what);
}

class UnloadMessageMapArgs final : public bsp::cc13_seg56::LuaArgs {
public:
    explicit UnloadMessageMapArgs(bsp::LuaBindingArgumentReader& core) : core_(core) {}
    int count() override { return core_.count(); }
    int get_integer(int index) override { return core_.get_integer(index); }
    double get_number(int index) override { return core_.get_number(index); }
    bool get_boolean(int index) override { return core_.get_boolean(index); }
    std::string get_string(int index) override { return core_.get_string(index); }
    bool is_string(int index) override { return core_.is_string(index); }
    bool is_nil(int index) override { return core_.is_nil(index); }
    bool is_entity_table(int index) override { return core_.is_entity_table(index); }
    void* entity_at(int index) override { return core_.entity_at(index); }
    bool is_boolean(int) override { unload_message_map_unreached("00B66000"); }
    bool is_number(int) override { unload_message_map_unreached("00B66050"); }
    bool is_integer(int) override { unload_message_map_unreached("00B66A60"); }
    bool is_table(int) override { unload_message_map_unreached("00B661B0"); }
    bsp::cc13_seg56::Vec3 read_vector3(int) override { unload_message_map_unreached("00888760"); }
    bool is_vector3_table(int) override { unload_message_map_unreached("0088B840"); }
    void* to_userdata(int) override { unload_message_map_unreached("00B662D0"); }
    bool boolean_or(int, bool) override { unload_message_map_unreached("00B662F0"); }
    double table_field_element(int, const char*, int) override {
        unload_message_map_unreached("00B67800 element");
    }
    double table_field_number_or(int, const char*, double) override {
        unload_message_map_unreached("00B67800 number");
    }

private:
    bsp::LuaBindingArgumentReader& core_;
};

class UnloadMessageMapGlue final : public bsp::cc13_seg56::GlueHost {
public:
    explicit UnloadMessageMapGlue(GameScriptOrdersHost& owner) : owner_(owner) {}
    // 008C65D8: 00706FC0 on [game+21DCh].
    void message_map_unload(const std::string& name, int index) override {
        owner_.unload_mission_message_map(name, index);
    }
    // game+1FE4h: the campaign session SetThinkCoreHost::game_non_campaign_flag
    // answers for LoadMessageMap's same test (008C635B); the session arm is not taken.
    std::int32_t game_session_mode_1fe4() override { return 0; }
    void session_manage_messages(const std::string&, int, int) override {
        unload_message_map_unreached("0088B6D0 MANAGE_MESSAGES (session arm)");
    }
    bsp::cc13_seg56::Vec3 game_camera_world_position() override {
        unload_message_map_unreached("camera position");
    }
    bsp::cc13_seg56::Vec3 entity_world_position(void*) override {
        unload_message_map_unreached("entity position");
    }
    bsp::cc13_seg56::Vec3 entity_world_to_local(void*, const bsp::cc13_seg56::Vec3&) override {
        unload_message_map_unreached("entity inverse");
    }
    void input_collect_action_binding_texts(int, std::vector<std::string>&,
        std::vector<std::string>&) override {
        unload_message_map_unreached("006A33F0");
    }
    void forced_input_set_insert(int) override { unload_message_map_unreached("0069FA40"); }
    void forced_input_set_erase(int) override { unload_message_map_unreached("008CA7B0"); }
    std::int32_t vehicle_class_slot(int) override { unload_message_map_unreached("00437F50"); }
    std::int32_t vehicle_class_cache_size() override { unload_message_map_unreached("00437F50 size"); }
    void vehicle_class_resize_cache(int) override { unload_message_map_unreached("00437BD0"); }
    void vehicle_class_release_cached(int) override { unload_message_map_unreached("class release"); }
    void log_text(const std::string&) override { unload_message_map_unreached("004254B0"); }

private:
    GameScriptOrdersHost& owner_;
};

// Goal 4 chain 365: FailureRepairEnable (008AD540) and AddFire (008ACD10) through
// their bodies in src/cc13_left55_seg56b.cpp. Both read only the core argument
// accessors (008AD540: 00888AA0 argument 0 at 008AD63F, 00B66250 argument 1 at
// 008AD67B; 008ACD10: 00888AA0 at 008ACE12, 00B66290 twice at 008ACE41 / 008ACE6F
// and 00B66270 at 008ACE9E); every other accessor is refused.
[[noreturn]] void repair_binding_unreached(const char* what) {
    throw std::logic_error(std::string("FailureRepairEnable/AddFire: unreached ") + what);
}

class RepairBindingArgs : public bsp::cc13_seg56::LuaArgs {
public:
    explicit RepairBindingArgs(bsp::LuaBindingArgumentReader& core) : core_(core) {}
    int count() override { return core_.count(); }
    int get_integer(int index) override { return core_.get_integer(index); }
    double get_number(int index) override { return core_.get_number(index); }
    bool get_boolean(int index) override { return core_.get_boolean(index); }
    std::string get_string(int index) override { return core_.get_string(index); }
    bool is_string(int index) override { return core_.is_string(index); }
    bool is_nil(int index) override { return core_.is_nil(index); }
    bool is_entity_table(int index) override { return core_.is_entity_table(index); }
    void* entity_at(int index) override { return core_.entity_at(index); }
    bool is_boolean(int) override { repair_binding_unreached("00B66000"); }
    bool is_number(int) override { repair_binding_unreached("00B66050"); }
    bool is_integer(int) override { repair_binding_unreached("00B66A60"); }
    bool is_table(int) override { repair_binding_unreached("00B661B0"); }
    bsp::cc13_seg56::Vec3 read_vector3(int) override { repair_binding_unreached("00888760"); }
    bool is_vector3_table(int) override { repair_binding_unreached("0088B840"); }
    void* to_userdata(int) override { repair_binding_unreached("00B662D0"); }
    bool boolean_or(int, bool) override { repair_binding_unreached("00B662F0"); }
    double table_field_element(int, const char*, int) override {
        repair_binding_unreached("00B67800 element");
    }
    double table_field_number_or(int, const char*, double) override {
        repair_binding_unreached("00B67800 number");
    }

private:
    bsp::LuaBindingArgumentReader& core_;
};

// 008B1CC0 uses 00B66290 for all three integers. Its float32 spill precedes
// truncation; the generic host argument_integer's direct double cast differs
// at boundaries such as 1.99999999. Borrow the real argument slots and use the
// existing complete conversion, including the host's 0109EEA4 projection.
class ScriptDayTimeArgs final : public RepairBindingArgs {
public:
    ScriptDayTimeArgs(bsp::LuaBindingArgumentReader& core, lua_State* state)
        : RepairBindingArgs(core), state_(state),
          sse2_conversion_(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE) {}
    int get_integer(int index) override {
        if (state_ == nullptr) return 0;
        ScriptOrdersLuaStackView value(state_, index + 1);
        return bsp::native_lua_integer_00b66290(value.object, sse2_conversion_);
    }
private:
    lua_State* state_;
    bool sse2_conversion_;
};

// bsp.py show 008B1CC0 --asm, 008B1E5E..008B1E6F: both bindings use the
// weather owner at [game+21C4h], not the atmosphere's separate time field.
// Its +4h is the Globals.lua StartDayTimeDefault cell the loader published.
// 00443A30 reads only +4h; the rest of WeatherConfig keeps
// its semantic C++ layout. No native vtable or map is dispatched here.
class ScriptDayTimeBinding final : public bsp::cc13_seg56::TimeOfDayHost,
                                   public bsp::cc13_seg56::LuaResults {
public:
    ScriptDayTimeBinding(bsp::WeatherConfig& weather, lua_State* state)
        : weather_(weather), state_(state) {}

    void time_of_day_set(int hours, int minutes, int seconds, int ms) override {
        weather_.start_day_time_04 = bsp::cc13_band0040::time_of_day_hours_004439b0(
            hours, minutes, seconds, ms);
    }
    void time_of_day_split(int& hours, int& minutes, int& seconds) override {
        static_assert(offsetof(bsp::WeatherConfig, start_day_time_04) == 4);
        bsp::seg3_4_cc13::time_of_day_split_clock_00443a30(
            &weather_, &hours, &minutes, &seconds, nullptr);
    }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>& fields) override {
        if (state_ == nullptr) return;
        // 008B1FC6 onward: one new result table and 00B67460 for hours,
        // minutes, seconds, in that order, on this dispatch's actual Lua VM.
        ScriptOrdersLuaStackView frame(state_, 0);
        bsp::NativeLuaObjectStorage result{};
        bsp::native_lua_new_table_00b67930(frame.object, result);
        for (const auto& field : fields) {
            ScriptOrdersNativeText key(field.first);
            bsp::native_lua_set_integer_00b67460(result, key.text, field.second);
        }
    }
    void push_number(int) override { repair_binding_unreached("day-time integer result"); }
    void push_float(float) override { repair_binding_unreached("day-time float result"); }
    void push_boolean(bool) override { repair_binding_unreached("day-time boolean result"); }
    void push_nil() override { repair_binding_unreached("day-time nil result"); }
    void push_string(const std::string&) override { repair_binding_unreached("day-time string result"); }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override {
        repair_binding_unreached("day-time vector result");
    }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        repair_binding_unreached("day-time vector fields result");
    }

private:
    bsp::WeatherConfig& weather_;
    lua_State* state_;
};

// 008A8720's three callees and its sole image read, bound to the current Lua
// entity owner. The body chooses the direct or local-message path, not this
// adapter. Narrow interfaces avoid supplying unrelated segment-56 services.
class SetRaceBinding final : public bsp::seg56_options_cc13::EntityIntImageAccess,
                             public bsp::seg56_options_cc13::EntityRaceHost {
public:
    using ClassQuery = std::function<bool(void*, int)>;
    using PartyRead = std::function<int(void*)>;
    using Apply = std::function<void(void*, int, int)>;
    using Route = std::function<void(void*, const bsp::seg56_options_cc13::IntValueMessage&)>;
    SetRaceBinding(ClassQuery query, PartyRead party, Apply apply, Route route)
        : query_(std::move(query)), party_(std::move(party)),
          apply_(std::move(apply)), route_(std::move(route)) {}

    std::int32_t read_i32(void* entity, std::uint32_t offset) override {
        if (offset != 0x54u) throw std::logic_error("SetRace: unexpected field read");
        return party_(entity);
    }
    bool entity_is_class_vt5c(void* entity, std::int32_t kind) override {
        return query_(entity, kind);
    }
    void entity_set_race_vt2c(void* entity, std::int32_t party,
        std::int32_t race, std::int32_t*) override {
        apply_(entity, party, race);
    }
    void route_int_message_0077c2a0(void* entity,
        const bsp::seg56_options_cc13::IntValueMessage& message,
        int flags, int out) override {
        if (message.type != 0x80 || message.vtable != 0x00d032fcu
            || message.dword_04 != 1 || message.word_18 != 0
            || message.byte_1a != 0 || flags != 7 || out != 0)
            throw std::logic_error("SetRace: unexpected route descriptor");
        route_(entity, message);
    }
private:
    ClassQuery query_;
    PartyRead party_;
    Apply apply_;
    Route route_;
};

// Goal 4 chain 525. ImageAccess is a typed view of the existing unit owner,
// not a second image of entity+127Ch. Resolve the actual Lua entity identity
// at the body's read point and borrow the air value 00855250/00893A60 maintain.
// bsp.py show 008938E0: argument 0 -> 00888AA0 -> FLD [entity+127Ch] ->
// 00B66480; the existing valid-submarine admission remains at the call site.
// Only that read and PushFloat belong to this binding; refuse all other
// operations of the shared segment-56 interfaces.
class SubmarineAirBinding final : public bsp::seg56_options_cc13::ImageAccess,
                                  public bsp::cc13_seg56::LuaResults {
public:
    SubmarineAirBinding(GameUnitsHost& units,
        std::function<std::size_t(void*)> index_of, std::function<void(float)> push)
        : units_(units), index_of_(std::move(index_of)), push_(std::move(push)) {}

    float read_f32(void* entity, std::uint32_t offset) override {
        if (offset != 0x127cu || entity == nullptr
            || !units_.submarine_air_008938e0(index_of_(entity), air_))
            unsupported();
        return air_;
    }
    void push_float(float value) override { push_(value); }
    float air() const noexcept { return air_; }

    std::uint8_t read_u8(void*, std::uint32_t) override { unsupported(); }
    std::int32_t read_i32(void*, std::uint32_t) override { unsupported(); }
    void* read_ptr(void*, std::uint32_t) override { unsupported(); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unsupported(); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unsupported(); }
    void write_f32(void*, std::uint32_t, float) override { unsupported(); }
    std::uint16_t global_u16(std::uint32_t) override { unsupported(); }
    void* global_ptr(std::uint32_t) override { unsupported(); }
    void write_global_u8(std::uint32_t, std::uint8_t) override { unsupported(); }
    void write_global_f32(std::uint32_t, float) override { unsupported(); }
    std::int32_t global_i32(std::uint32_t) override { unsupported(); }
    void push_number(int) override { unsupported(); }
    void push_boolean(bool) override { unsupported(); }
    void push_nil() override { unsupported(); }
    void push_string(const std::string&) override { unsupported(); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override { unsupported(); }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unsupported(); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override { unsupported(); }

private:
    [[noreturn]] static void unsupported() {
        throw std::logic_error("GetSubmarineAirSupply 008938E0: unbound accessor");
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    std::function<void(float)> push_;
    float air_{};
};

// ---------------------------------------------------------------------------
// Goal 5 chain 655: the segment-56 unit queries GetRoleAvailable 008ABAC0,
// GetRoleOwner 008ABF90, GetSkillLevel 008953F0, GetCrewLevel 008950D0 and
// GetShipMaxSpeed 00890BB0 run their bodies (src/cc13_left55_seg56b.cpp,
// src/options_seg56_cc13.cpp) over the units host's slot state. Each adapter
// binds only the operations its body reaches and refuses the rest, so a path
// nobody read fails at once instead of answering an invented value. Argument 0
// is the body's own 00888AA0 (entity_at): the table's `ID`, the units-host index
// plus one. The dispatch checks the slot before it calls a body, because the
// image reads whatever object it is handed and faults on a null one.
[[noreturn]] void unit_query_unreached(const char* what) {
    throw std::logic_error(std::string("segment-56 unit query: unreached ") + what);
}

// Goal 5 chain 663. The field stores of SetSubmarineAirSupply 00893A60 (FSTP float
// [entity+127Ch], 00893BA1) and SquadronSetTravelSpeed 008A0630 (008A07E2 +388h =
// float 00CE3800 0.5, 008A07F0 +38Fh the flag, 008A07FA +3A0h the speed, 008A0802
// +3ACh = 1), from their bodies lua_set_submarine_air_supply_00893a60 (src/options_seg56_cc13.cpp)
// and lua_squadron_set_travel_speed_008a0630 (src/cc13_left55_seg56a.cpp). ImageAccess is a
// typed view of the units host's slot state, not a second image: +127Ch is the
// submarine's air (set_submarine_air_00893a60, which stores nothing for a unit with no
// seeded dive state), and the squadron's four fields are the ones
// set_squadron_travel_speed_008a0630 keeps (its +388h is the 0.5 s countdown held as an
// expiry, so any other +388h value is refused rather than mis-stored). The squadron
// stores are committed together after the body returns. Every other access is refused.
class UnitFieldStoreBinding final : public bsp::seg56_options_cc13::ImageAccess {
public:
    UnitFieldStoreBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of)
        : units_(units), index_of_(std::move(index_of)) {}

    void write_f32(void* entity, std::uint32_t offset, float value) override {
        if (offset == 0x127Cu) {
            stored_ = units_.set_submarine_air_00893a60(index(entity), value);
            return;
        }
        if (offset == 0x388u && value == bsp::cc13_left55::seg56a::kTravelSpeedField388) {
            bind_squadron(entity);
            seen_ |= kSeen388;
            return;
        }
        if (offset == 0x3A0u) {
            bind_squadron(entity);
            speed_ = value;
            seen_ |= kSeen3A0;
            return;
        }
        unit_query_unreached("a float store other than +127Ch, +388h = 0.5 or +3A0h");
    }
    void write_u8(void* entity, std::uint32_t offset, std::uint8_t value) override {
        if (offset == 0x38Fu) {
            bind_squadron(entity);
            force_ = value != 0;
            seen_ |= kSeen38F;
            return;
        }
        if (offset == 0x3ACu && value == 1u) {
            bind_squadron(entity);
            seen_ |= kSeen3AC;
            return;
        }
        unit_query_unreached("a byte store other than +38Fh or +3ACh = 1");
    }
    // SquadronSetTravelSpeed's four stores, as one units-host write. False when the
    // body did not make all four or the slot has no squadron.
    bool commit_squadron_travel_speed() {
        if (seen_ != (kSeen388 | kSeen38F | kSeen3A0 | kSeen3AC)) return false;
        stored_ = units_.set_squadron_travel_speed_008a0630(index(squadron_), speed_, force_);
        return stored_;
    }
    bool stored() const noexcept { return stored_; }
    float speed() const noexcept { return speed_; }
    bool force() const noexcept { return force_; }

    std::int32_t read_i32(void*, std::uint32_t) override { unit_query_unreached("read_i32"); }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void* read_ptr(void*, std::uint32_t) override { unit_query_unreached("read_ptr"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

private:
    static constexpr unsigned kSeen388 = 1u, kSeen38F = 2u, kSeen3A0 = 4u, kSeen3AC = 8u;
    std::size_t index(void* entity) const {
        const std::size_t i = entity != nullptr ? index_of_(entity) : ~static_cast<std::size_t>(0);
        if (i >= units_.count()) unit_query_unreached("a store on no units-host slot");
        return i;
    }
    void bind_squadron(void* entity) {
        if (squadron_ != nullptr && squadron_ != entity) {
            unit_query_unreached("squadron stores on two entities");
        }
        squadron_ = entity;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    void* squadron_{nullptr};
    unsigned seen_{0};
    float speed_{0.0f};
    bool force_{false};
    bool stored_{false};
};

// 006BCD20's block selection (bsp.py ghidra disasm 006bcd20): a null entity answers
// null; 006BCD35 vtable[5Ch](9) gives owner+1188h, 006BCD4C vtable[5Ch](45h)
// owner+72Ch, anything else null. `owner_class` says whether one of the two tests
// held (the image's block is embedded in that owner); the deck is the block the
// scene loaded for it, which the deck registry binds to the table's `ID` (the
// units-host index plus one). Null when the scene loaded no deck for the owner.
bsp::AirOpsDeck* unit_air_ops_deck_006bcd20(const GameUnitsHost& units, std::size_t index,
    bool& owner_class) {
    owner_class = false;
    if (index >= units.count()) return nullptr;
    std::uint32_t class_id = 0;
    if (units.unit_is_kind_of(index, static_cast<int>(bsp::kAirOpsClassIdMothership))) {
        class_id = bsp::kAirOpsClassIdMothership;
    } else if (units.unit_is_kind_of(index, static_cast<int>(bsp::kAirOpsClassIdAirfield))) {
        class_id = bsp::kAirOpsClassIdAirfield;
    }
    owner_class = bsp::air_ops_block_offset_006bcd20(class_id) != 0;
    if (!owner_class) return nullptr;
    return bsp::air_ops_decks().find_mutable_by_entity_id(static_cast<int>(index + 1u));
}

// The squadron whose entity the slot is. This process fuses the squadron's slot with
// its wing-0 plane (PlaneSquadronHostRecord::squadron_unit), so an argument naming that
// slot is the squadron, as GetEntitySpeed, SetCrewLevel and EntityTurnToEntity read it.
const bsp::PlaneSquadronHostRecord* fused_squadron_record(std::size_t index) {
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == index) return &r;
    }
    return nullptr;
}

// The squadron's +3D0h array as the image holds it: 007F3970 compacts it at a death, so
// it is the live planes in order, +3CCh of them. The record keeps a wing that never
// became a unit (or left) as kPlaneSquadronNoUnit to keep the spawn stamps aligned, so
// those are skipped here, as live_count() and flight_leader() skip them.
std::vector<std::size_t> squadron_live_members(const bsp::PlaneSquadronHostRecord& squadron) {
    std::vector<std::size_t> live;
    for (const std::size_t unit : squadron.member_units) {
        if (unit != bsp::kPlaneSquadronNoUnit) live.push_back(unit);
    }
    return live;
}

// The two pushes 008953F0 / 008950D0 (00B664B0 PushNumber) and 00890BB0 (00B66480
// PushFloat) make, onto the dispatch's own Lua frame.
class UnitQueryResults final : public bsp::cc13_seg56::LuaResults {
public:
    UnitQueryResults(std::function<void(int)> push_number, std::function<void(float)> push_float)
        : push_number_(std::move(push_number)), push_float_(std::move(push_float)) {}
    void push_number(int value) override { push_number_(value); ++pushed_; }
    void push_float(float value) override { push_float_(value); ++pushed_; }
    void push_boolean(bool) override { unit_query_unreached("00B66450"); }
    void push_nil() override { unit_query_unreached("00B66430"); }
    void push_string(const std::string&) override { unit_query_unreached("00B66710"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unit_query_unreached("00B67930 integer fields");
    }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unit_query_unreached("0088BA30"); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unit_query_unreached("00B67930 vector fields");
    }
    int pushed() const noexcept { return pushed_; }

private:
    std::function<void(int)> push_number_;
    std::function<void(float)> push_float_;
    int pushed_{0};
};

// 008953F0 / 008950D0 / 00890BB0 through Seg56Host. Slot 12Ch (bsp.py show 006d1ec0:
// MOV EAX,[ECX+390h] / RET) is the unit's +390h skill index on every ship and plane
// vtable that carries 006D1EC0; the squadron's slot (00D088EC, bsp.py show 007ecf70:
// MOV ECX,[ECX+3D0h] / JMP [vtable+12Ch]) forwards to the wing in member slot 0.
// The host keeps +390h as the slot's pilot_skill_index (GameUnitsHost::skill_level,
// the index SetSkillLevel / SetCrewLevel write through 006D1EB0), and its squadron is
// the registry record whose fused slot the argument is, as GetEntitySpeed and
// SetCrewLevel treat it. 006E6270 reads the session word, the 0 this host reports
// for single player (GameScriptOrdersHost::game_session_mode). 0080FC30 is the
// units host's reference speed, unit+9C0h (the class MaxSpeed SetShipMaxSpeed moves)
// times the power-up scale (GameUnitsHost::throttle_ceiling_inputs).
class UnitQuerySeg56Host final : public bsp::seg56_options_cc13::Seg56HostUnreached,
                                 public bsp::cc13_seg33a::Seg33aSessionModeHost {
public:
    UnitQuerySeg56Host(const GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        std::function<std::int32_t()> session_mode)
        : units_(units), index_of_(std::move(index_of)), session_mode_(std::move(session_mode)) {}

    std::int32_t entity_skill_vt12c(void* entity) override {
        const std::size_t index = slot_of(entity);
        const bsp::PlaneSquadronHostRecord* squadron = fused_squadron_record(index);
        if (squadron == nullptr) return units_.skill_level(index);   // 006D1EC0
        // 007ECF70 over the squadron's +3D0h array (its live planes, squadron_live_members).
        bsp::PlaneSquadronEntity view;
        const std::vector<std::size_t> live = squadron_live_members(*squadron);
        view.live_count = static_cast<int>(live.size());
        for (std::size_t i = 0; i < live.size() && i < bsp::kPlaneSquadronMaxWings; ++i) {
            view.members[i] = reinterpret_cast<void*>(static_cast<std::uintptr_t>(live[i] + 1u));
        }
        const auto leader_skill_vt12c = [&](void* leader) {
            // The image calls through a null slot 0 (no test at 007ECF70); the dispatch
            // refuses a squadron with no live plane first.
            if (leader == nullptr) unit_query_unreached("007ECF70 with an empty member slot 0");
            return units_.skill_level(slot_of(leader));               // 006D1EC0
        };
        return bsp::plane_squadron_leader_slot12c_007ecf70(view, leader_skill_vt12c);
    }
    std::int32_t crew_level_from_skill_006e6270(std::int32_t skill) override {
        return bsp::cc13_seg33a::crew_level_from_skill_006e6270(*this, skill);
    }
    std::int32_t session_mode_00e188a8_1fe4() override { return session_mode_(); }
    double unit_reference_speed_0080fc30(void* unit) override {
        return units_.throttle_ceiling_inputs(slot_of(unit)).reference_speed;
    }

    // 006BCD20(entity, DL) for SetAirBasePlaneLimit 00897720 / GetAirBasePlaneLimit
    // 008978D0 (both pass DL = 0, XOR DL,DL at 00897839 / 008979E6), through
    // unit_air_ops_deck_006bcd20. With DL set (006BCD5C) the block's +7Ch owner and
    // the owner's +5Dh are the deck's owner_present / owner_blocked.
    void* air_ops_block_006bcd20(void* entity, bool require_active) override {
        bool owner_class = false;
        bsp::AirOpsDeck* deck = unit_air_ops_deck_006bcd20(units_, slot_of(entity), owner_class);
        if (!owner_class) return nullptr;
        if (deck == nullptr) unit_query_unreached("a carrier or airfield with no loaded deck");
        if (!bsp::air_ops_block_is_usable_006bcd20(require_active, deck->owner_present,
                deck->owner_blocked)) {
            return nullptr;
        }
        return deck;
    }
    // 006BA6C0: MOV EAX,[ECX+58h] / RET. 006BA6D0: MOV EAX,[ESP+4] / MOV [ECX+58h],EAX /
    // RET 4 (bsp.py ghidra disasm). block+58h is the deck's max_in_air_planes. Neither
    // accessor tests the block: the image faults on a null one, which the dispatch's
    // pre-check keeps from happening.
    std::int32_t air_ops_block_plane_limit_006ba6c0(void* block) override {
        return deck_of(block).max_in_air_planes;
    }
    void air_ops_block_set_plane_limit_006ba6d0(void* block, std::int32_t limit) override {
        deck_of(block).max_in_air_planes = limit;
    }

    // 008457E0 for RemoveShipyardStock 00896EE0 (0089705F, ECX the entity 00888AA0
    // answered, then the class and count integers). Its records are the shipyard's
    // +774h stock vector (stride 20h; class [rec+4h]+70h, count +8h), which this
    // process keeps as the registry's ShipyardState::stock for the shipyard the scene
    // attached (bound to the table's `ID`). The body runs over a {class, count} view
    // of those records, and the counts it leaves are written back, so its first-match
    // clamp is the one rule applied to the host's own stock.
    void shipyard_consume_stock_008457e0(void* shipyard, std::int32_t class_id,
        std::int32_t count) override {
        bsp::ShipyardState* state = bsp::shipyards().find_mutable_by_entity_id(
            static_cast<int>(slot_of(shipyard) + 1u));
        if (state == nullptr) unit_query_unreached("008457E0 on an entity with no shipyard");
        std::vector<bsp::cc13_band0080::ShipyardStockRecord> view(state->stock.size());
        for (std::size_t i = 0; i < view.size(); ++i) {
            view[i].class_id = static_cast<std::int32_t>(state->stock[i].class_id);
            view[i].count = state->stock[i].count;
        }
        bsp::cc13_band0080::shipyard_consume_stock_008457e0(view.data(), view.size(),
            class_id, count);
        for (std::size_t i = 0; i < view.size(); ++i) state->stock[i].count = view[i].count;
    }

private:
    static bsp::AirOpsDeck& deck_of(void* block) {
        if (block == nullptr) unit_query_unreached("006BA6C0 / 006BA6D0 on a null block");
        return *static_cast<bsp::AirOpsDeck*>(block);
    }
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    const GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    std::function<std::int32_t()> session_mode_;
};

// GetAirBaseSlotStatus 00896220's two image reads (bsp.py show 00896220 --asm: 00896369
// SUB ESI,1 / 00896378 IMUL ESI,ESI,58h / 0089637B MOV EDI,[EDI+4Ch] / 0089637E MOV
// ESI,[ESI+EDI+2Ch], then 00B664B0): the block's slot array at +4Ch, stride 58h, and a
// slot's state at +2Ch. The array is the deck's `slots` vector. This view answers +4Ch
// with the base of an address range of 58h bytes per slot, sized to that vector and
// never read, and +2Ch at slot k of the range with slots[k].state (the AirOpsSlotState
// values 006C0510 and the launch rule test). The dispatch has checked the 1-based index
// against the deck, so every address the body forms lies in the range.
class AirSlotStateView final : public bsp::seg56_options_cc13::ImageAccess {
public:
    void* read_ptr(void* block, std::uint32_t offset) override {
        if (offset != 0x4Cu || block == nullptr) unit_query_unreached("a pointer other than block+4Ch");
        deck_ = static_cast<const bsp::AirOpsDeck*>(block);
        range_.assign(deck_->slots.size() * kSlotStride, 0u);
        return range_.data();
    }
    std::int32_t read_i32(void* slot, std::uint32_t offset) override {
        if (offset != 0x2Cu || deck_ == nullptr) unit_query_unreached("a dword other than slot+2Ch");
        const std::ptrdiff_t at = static_cast<const std::uint8_t*>(slot) - range_.data();
        if (at < 0 || at % static_cast<std::ptrdiff_t>(kSlotStride) != 0
            || static_cast<std::size_t>(at) / kSlotStride >= deck_->slots.size()) {
            unit_query_unreached("a slot outside the deck's slot array");
        }
        return static_cast<std::int32_t>(deck_->slots[static_cast<std::size_t>(at) / kSlotStride].state);
    }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

private:
    static constexpr std::size_t kSlotStride = 0x58;
    const bsp::AirOpsDeck* deck_{nullptr};
    std::vector<std::uint8_t> range_;
};

// 008ABAC0 / 008ABF90 over fill_nine_integers (bsp.py show 008abac0 --asm: 00888AA0 on
// argument 0 at 008ABBC1, 00B67930 at 008ABBE1, then 008ABBF6..008ABC0E nine 00B665D0
// stores of keys 1..9 from [entity+188h+4i], and 00B66400 at 008ABC14; 008ABF90 is the
// same body over +1ACh). +188h is the role permission word 00927D20 writes per role
// (the host's role_permission_0188, which SetRoleAvailable reaches through
// set_role_availability_00927d20) and +1ACh the role's current holder slot
// (current_roles_01ac). The table is built on the dispatch's real Lua stack by the
// native LuaObject bodies 00B67930 / 00B665D0; 00B66400 is the frame's growth.
class RoleTableBinding final : public bsp::cc13_left55::seg56b::Left55Seg56bHostUnreached,
                               public bsp::seg56_options_cc13::ImageAccess,
                               public bsp::seg56_options_cc13::LuaFrame {
public:
    RoleTableBinding(lua_State* state, const GameUnitsHost& units,
        std::function<std::size_t(void*)> index_of)
        : stack_(state, 0), units_(units), index_of_(std::move(index_of)),
          original_top_(lua_gettop(state)) {}

    // --- Left55Seg56bHost: the two LuaObject calls ---
    bsp::cc13_left55::seg56b::LuaRef frame_new_table_00b67930(
        bsp::seg56_options_cc13::LuaFrame&) override {
        bsp::native_lua_new_table_00b67930(stack_.object, table_);
        return &table_;
    }
    void table_set_integer_at_index_00b665d0(bsp::cc13_left55::seg56b::LuaRef table, int key,
        int value) override {
        bsp::native_lua_set_integer_at_00b665d0(
            *static_cast<bsp::NativeLuaObjectStorage*>(table), key, value);
    }

    // --- ImageAccess: the nine words at +188h or +1ACh ---
    std::int32_t read_i32(void* entity, std::uint32_t offset) override {
        constexpr std::uint32_t kWords = static_cast<std::uint32_t>(bsp::kUnitRoleTableEntries);
        const bool permission = offset >= 0x188u && offset < 0x188u + 4u * kWords;
        const bool holder = offset >= 0x1ACu && offset < 0x1ACu + 4u * kWords;
        if ((!permission && !holder) || (offset & 3u) != 0u) {
            unit_query_unreached("a unit read outside +188h..+1CFh");
        }
        const std::size_t index = index_of_(entity);
        const std::int32_t role =
            static_cast<std::int32_t>((offset - (permission ? 0x188u : 0x1ACu)) / 4u);
        std::int32_t value = 0;
        const bool read = permission ? units_.unit_role_permission(index, role, value)
                                     : units_.unit_current_role_slot(index, role, value);
        if (!read) unit_query_unreached("an entity without a units-host slot");
        return value;
    }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void* read_ptr(void*, std::uint32_t) override { unit_query_unreached("read_ptr"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

    // --- LuaFrame: 00B66400, and no direct pushes ---
    int result_count() override { return lua_gettop(stack_.owner.state_04) - original_top_; }
    void push_number(int) override { unit_query_unreached("00B664B0"); }
    void push_boolean(bool) override { unit_query_unreached("00B66450"); }
    void push_nil() override { unit_query_unreached("00B66430"); }
    void push_float(float) override { unit_query_unreached("00B66480"); }
    void push_string(const std::string&) override { unit_query_unreached("00B66710"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unit_query_unreached("00B67930 integer fields");
    }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unit_query_unreached("0088BA30"); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unit_query_unreached("00B67930 vector fields");
    }

private:
    // The table object borrows stack_.owner, so both live as long as the binding.
    ScriptOrdersLuaStackView stack_;
    bsp::NativeLuaObjectStorage table_{};
    const GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    int original_top_;
};

// What 008A0F10 EntityTurnToPosition reads from its frame (bsp.py show 008a0f10 --asm):
// 008A101F 00888AA0 on argument 0, 008A1059 00888760 (a Vector3 table) on argument 1,
// 008A10C7 00B663F0 the count and, only when it is 3, 008A10F1 00B66250 on argument 2.
class TurnToPositionArgs final : public bsp::cc13_seg56::LuaArgs {
public:
    using Vector3 = std::function<bool(int, float (&)[3])>;
    TurnToPositionArgs(bsp::LuaBindingArgumentReader& core, Vector3 vector3)
        : core_(core), vector3_(std::move(vector3)) {}
    void* entity_at(int index) override { return core_.entity_at(index); }
    int count() override { return core_.count(); }
    bool get_boolean(int index) override { return core_.get_boolean(index); }
    bsp::cc13_seg56::Vec3 read_vector3(int index) override {
        float p[3] = {};
        if (!vector3_(index, p)) unit_query_unreached("00888760 on a value that is no Vector3");
        return bsp::cc13_seg56::Vec3{p[0], p[1], p[2]};
    }
    int get_integer(int) override { unit_query_unreached("00B66290"); }
    double get_number(int) override { unit_query_unreached("00B66270"); }
    std::string get_string(int) override { unit_query_unreached("00B662B0"); }
    bool is_string(int) override { unit_query_unreached("00B660A0"); }
    bool is_nil(int) override { unit_query_unreached("00B65FB0"); }
    bool is_entity_table(int) override { unit_query_unreached("008889C0"); }
    bool is_boolean(int) override { unit_query_unreached("00B66000"); }
    bool is_number(int) override { unit_query_unreached("00B66050"); }
    bool is_integer(int) override { unit_query_unreached("00B66A60"); }
    bool is_table(int) override { unit_query_unreached("00B661B0"); }
    bool is_vector3_table(int) override { unit_query_unreached("0088B840"); }
    void* to_userdata(int) override { unit_query_unreached("00B662D0"); }
    bool boolean_or(int, bool) override { unit_query_unreached("00B662F0"); }
    double table_field_element(int, const char*, int) override {
        unit_query_unreached("00B67800 element");
    }
    double table_field_number_or(int, const char*, double) override {
        unit_query_unreached("00B67800 number");
    }

private:
    bsp::LuaBindingArgumentReader& core_;
    Vector3 vector3_;
};

// 008A0F10's services and image reads over the units host (lua_entity_turn_to_position,
// src/cc13_left55_seg56a.cpp). The single-player arms only: [[00E188A8]+1FE4h] is the
// session word (CMP at 008A11B7, zero JZ 008A121E past the session-message arm), the
// 0 GameScriptOrdersHost::game_session_mode reports. A squadron (vtable[5Ch](18h) at
// 008A1227, the slot fused with its wing 0) re-poses each member at the member's own
// +FCh through vtable[88h] (008A1294); a ship (5Ch(6)) takes vtable[88h] at 008A1301,
// then 00C336C0 and 00C56CF0 on [0080E490()+2Ch], the physics body; anything else takes
// vtable[88h] alone (008A1378). vtable[88h] is 006E00A0 on a ship (the units host's
// set_local_matrix_006e00a0) and 007C9540 on a plane (set_unit_world_basis_007c9540),
// the two setters EntityTurnToEntity already uses.
class TurnToPositionBinding final : public bsp::cc13_left55::seg56a::Left55Seg56aHostUnreached,
                                    public bsp::seg56_options_cc13::ImageAccess {
public:
    TurnToPositionBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        std::function<std::int32_t()> session_mode)
        : units_(units), index_of_(std::move(index_of)), session_mode_(std::move(session_mode)) {}

    // --- ImageAccess ---
    // [00E188A8]: the binding reads only +1FE4h through it, so the address is a stand-in
    // the session word answers for.
    void* global_ptr(std::uint32_t address) override {
        if (address != bsp::cc13_left55::seg56a::kGameGlobal) {
            unit_query_unreached("a global other than [00E188A8]");
        }
        return &game_;
    }
    std::int32_t read_i32(void* base, std::uint32_t offset) override {
        if (base == &game_ && offset == bsp::cc13_left55::seg56a::kSessionModeOffset) {
            return session_mode_();
        }
        if (offset == 0x3CCu) return squadron_of(base).live_count();   // 008A122D / 008A129C
        unit_query_unreached("a dword other than game+1FE4h or squadron+3CCh");
    }
    // +C8h, the pose-valid byte 00414DB0 sets. LABELLED, as GetDirection and GetPosition
    // take it: the units host's poses are never stale, so the refresh is not reached.
    std::uint8_t read_u8(void* entity, std::uint32_t offset) override {
        if (offset != 0xC8u) unit_query_unreached("a byte other than +C8h");
        static_cast<void>(slot_of(entity));
        return 1;
    }
    // +FCh..+104h, the world position (GameUnitsHost::unit_position_00fc).
    float read_f32(void* entity, std::uint32_t offset) override {
        float p[3] = {};
        units_.unit_position_00fc(slot_of(entity), p[0], p[1], p[2]);
        if (offset == 0xFCu) return p[0];
        if (offset == 0x100u) return p[1];
        if (offset == 0x104u) return p[2];
        unit_query_unreached("a float other than +FCh..+104h");
    }
    // +3D0h + 4i, the squadron's member slots, and [parts+2Ch], the physics body.
    void* read_ptr(void* base, std::uint32_t offset) override {
        if (base == &parts_ && offset == 0x2Cu) return &body_;
        if (offset >= 0x3D0u && offset < 0x3D0u + 4u * bsp::kPlaneSquadronMaxWings
            && (offset & 3u) == 0u) {
            const std::vector<std::size_t> live = squadron_live_members(squadron_of(base));
            const std::size_t slot = (offset - 0x3D0u) / 4u;
            if (slot >= live.size()) return nullptr;
            return reinterpret_cast<void*>(static_cast<std::uintptr_t>(live[slot] + 1u));
        }
        unit_query_unreached("a pointer other than +3D0h's slots or [parts+2Ch]");
    }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

    // --- Left55Seg56aHost ---
    bool entity_vcall_5c(void* entity, std::int32_t class_id) override {
        const std::size_t index = slot_of(entity);
        if (class_id == 0x18) return fused_squadron_record(index) != nullptr;
        return units_.unit_is_kind_of(index, class_id);
    }
    void entity_vcall_88(void* entity, const float* matrix) override {
        const std::size_t index = slot_of(entity);
        bool posed = false;
        if (units_.unit_is_kind_of(index, 6)) {
            posed = units_.set_local_matrix_006e00a0(index, matrix);            // 006E00A0
        } else if (units_.unit_is_kind_of(index, 0x0F)) {
            posed = units_.set_unit_world_basis_007c9540(index, &matrix[0], &matrix[4],
                &matrix[8]);                                                  // 007C9540
        } else {
            unit_query_unreached("vtable[88h] on a class this host does not pose");
        }
        if (posed) ++posed_;
    }
    void matrix_orthonormalize_basis_rows_0085dc80(float* matrix) override {
        bsp::orthonormalize_pose_matrix_0085dc80(matrix);
    }
    void dyn_copy_matrix43_00c336c0(float out[12], const float* matrix) override {
        bsp::cc13_dyn1::transform_from_4x4_00c336c0(out, matrix);
    }
    void* unit_parts_object_0080e490(void* unit) override {
        static_cast<void>(slot_of(unit));
        return &parts_;
    }
    // SUBSTITUTION, labelled, as EntityTurnToEntity's ship arm takes it: the ship's
    // physics body is the units host's motion state, which vtable[88h] (006E00A0) has
    // just set from the same matrix (008A1301), so the 4x3 00C336C0 built is that pose
    // and nothing further is stored.
    void dyn_body_set_transform_00c56cf0(void* body, const float* transform) override {
        if (body != &body_) unit_query_unreached("00C56CF0 on a body other than [parts+2Ch]");
        static_cast<void>(transform);
        ++body_poses_;
    }

    unsigned posed() const noexcept { return posed_; }
    unsigned body_poses() const noexcept { return body_poses_; }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    const bsp::PlaneSquadronHostRecord& squadron_of(void* entity) const {
        const bsp::PlaneSquadronHostRecord* squadron = fused_squadron_record(slot_of(entity));
        if (squadron == nullptr) unit_query_unreached("a squadron read on a non-squadron");
        return *squadron;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    std::function<std::int32_t()> session_mode_;
    // Stand-in addresses for the three objects the binding only hands back: the game
    // object, the parts object and its physics body.
    unsigned char game_{};
    unsigned char parts_{};
    unsigned char body_{};
    unsigned posed_{0};
    unsigned body_poses_{0};
};

// 008A7780 PilotStop through lua_pilot_stop (issue_fixed_command, src/cc13_left55_seg56a.cpp):
// 008A787E 00888AA0 on argument 0, the zero descriptor with the vector at 00F87574
// (008A7896 / 008A78A4 MOVSS), then 0077D600(entity, stop 00E08F88, &descriptor, 1) at
// 008A78E9; nothing pushed. 0077D600 is the host's entity_issue_command, the same issue
// NavigatorStop 008A7400 makes. 00F87574..7C is the BSS zero vector, read as zero, as
// run_navigator_stop takes it.
class FixedCommandBinding final : public bsp::cc13_left55::seg56a::Left55Seg56aHostUnreached,
                                  public bsp::seg56_options_cc13::ImageAccess {
public:
    using Issue = std::function<void(void*, std::uint32_t, const bsp::SceneCommandTarget&, int)>;
    explicit FixedCommandBinding(Issue issue) : issue_(std::move(issue)) {}

    void entity_issue_command_0077d600(void* entity, void* command,
        const bsp::SceneCommandTarget& target, std::int32_t flags) override {
        issue_(entity, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(command)),
            target, flags);
        ++issued_;
    }
    std::int32_t global_i32(std::uint32_t address) override {
        if (address != bsp::cc13_left55::seg56a::kZeroVectorGlobal
            && address != bsp::cc13_left55::seg56a::kZeroVectorGlobal + 4u
            && address != bsp::cc13_left55::seg56a::kZeroVectorGlobal + 8u) {
            unit_query_unreached("a global other than the zero vector 00F87574");
        }
        return 0;
    }
    std::int32_t read_i32(void*, std::uint32_t) override { unit_query_unreached("read_i32"); }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void* read_ptr(void*, std::uint32_t) override { unit_query_unreached("read_ptr"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    unsigned issued() const noexcept { return issued_; }

private:
    Issue issue_;
    unsigned issued_{0};
};

// Goal 4 chain 460. These callbacks borrow the current script orders owner;
// neither adapter creates another narrative queue or another Lua call frame.
class NarrativeQueueBinding final : public bsp::cc13_seg56::NarrativeContentHost {
public:
    NarrativeQueueBinding(std::function<int()> size, std::function<void()> clear)
        : size_(std::move(size)), clear_(std::move(clear)) {}
    int narrative_size() override { return size_(); }
    void narrative_clear() override { clear_(); }
private:
    std::function<int()> size_;
    std::function<void()> clear_;
};

class NarrativePostBinding final : public bsp::cc13_seg56::NarrativePostHost {
public:
    using Post = std::function<void(std::uint32_t, const std::string&, const std::string&, int)>;
    explicit NarrativePostBinding(Post post) : post_(std::move(post)) {}
    void narrative_post(std::uint32_t entry_point, const std::string& text,
        const std::string& callback, bsp::cc13_seg56::LuaArgs&, int extras_from) override {
        // 008B0DAC (MissionNarrative and its Enqueue alias) calls 00734870,
        // 008B0FAC (Urgent) 007349C0 and 008B11AC (Override) 00734E50.
        post_(entry_point, text, callback, extras_from);
    }
private:
    Post post_;
};

// Goal 5 chain 643. The services the narrative object [game+21E8h] reaches
// through its own bodies: 00735030 / 00735080 / 00733C80 / 00733DC0 (the
// MissionNarrativeObjectHost hooks, cc13_seg40.hpp) and the three list writers
// 00734870 / 007349C0 / 00734E50 (cc15_g24c::NarrativeListHost). Native
// strings go through the process's actual string pool (0041DD40, and 00419CC0
// with 00BD1510 for the returns). 00733430, 007342B0, 007341B0, 00734CE0 and
// 00734D20 are the VC9 std::list members (library rows, bound here) as their
// listings show them: operator new(20h) 00BF681B is malloc with bad_alloc on
// failure, and every node goes back through _free 00BF65AC.
//
// An entry's +14h (node +1Ch) is the Lua variant vector the binding builds
// with 006EDF00 from the extra arguments. This host keeps those arguments as
// Lua registry references instead (the substitution its narrative queue
// already made), held per list node by the owner; the native word stays 0, so
// 00733DC0's vector arm never runs, and `released` fires for every node
// 00734CE0 / 00734D20 frees so the owner drops that node's references.
class NarrativeObjectServices final : public bsp::MissionNarrativeObjectHost,
                                      public bsp::cc15_g24c::NarrativeListHost {
public:
    using Released = std::function<void(const void* node)>;
    explicit NarrativeObjectServices(Released released) : released_(std::move(released)) {}

    // ---- MissionNarrativeObjectHost ----
    void crt_free_00bf6989(void* block) override { std::free(block); }
    void crt_free_00bf65ac(void* block) override { std::free(block); }
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t keep) override {
        bsp::resize_native_string_header_0041dd40(string, strings(), length, keep != 0);
    }
    void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t size) override {
        std::memcpy(destination, source, size);
    }
    void* sized_pool_get_00419cc0() override { return &strings(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
                                          std::int32_t) override {
        if (pool != &strings()) throw std::logic_error("narrative: 00BD1510 on another pool");
        strings().release(static_cast<char*>(data), size);
    }
    void lua_variant_vector_resize_006b88e0(void*, std::int32_t) override {
        throw std::logic_error("narrative: a native Lua variant vector reached 006B88E0");
    }
    void narrative_list_clear_00734ce0(void* list) override {
        clear(static_cast<std::uint8_t*>(list));
    }
    // 00733430: operator new(20h), next = prev = the node itself.
    void* narrative_list_buy_head_node_00733430(void*) override {
        std::uint8_t* const node = operator_new_00bf681b(0x20);
        store(node, 0x00, node);
        store(node, 0x04, node);
        return node;
    }

    // ---- cc15_g24c::NarrativeListHost ----
    // 007342B0: operator new(20h), +0h next, +4h prev, then 00733C80 copies the
    // entry into +8h; its EH state 1 (Catch_All 0073432B) frees the node and
    // rethrows when the copy throws.
    std::uint8_t* narrative_list_buy_node_007342b0(std::uint8_t*, std::uint8_t* next,
                                                   std::uint8_t* prev,
                                                   const std::uint8_t* entry) override {
        std::uint8_t* const node = operator_new_00bf681b(0x20);
        store(node, 0x00, next);
        store(node, 0x04, prev);
        try {
            bsp::cc13_seg48U_narrative_entry_copy_construct_00733c80(node + 0x08, entry, *this);
        } catch (...) {
            std::free(node);
            throw;
        }
        return node;
    }
    // 007341B0: length_error("list<T> too long") past 0AAAAAAAh, else +8h += count.
    void narrative_list_incsize_007341b0(std::uint8_t* list, std::uint32_t count) override {
        const std::uint32_t size = load<std::uint32_t>(list, 0x08);
        if (0x0AAAAAAAu - size < count) throw std::length_error("list<T> too long");
        store(list, 0x08, size + count);
    }
    void narrative_entry_destruct_00733dc0(std::uint8_t* entry) override {
        bsp::cc13_seg40_narrative_entry_destruct_00733dc0(entry, *this);
    }
    void narrative_list_clear_00734ce0(std::uint8_t* list) override { clear(list); }
    bool native_string_equals_insensitive_00435c40(const std::uint8_t* string,
                                                   const void* other) override {
        return bsp::equal_native_string_headers_00435c40(string, other);
    }
    void native_string_resize_0041dd40(std::uint8_t* string, std::uint32_t length,
                                       std::int32_t flag) override {
        bsp::resize_native_string_header_0041dd40(string, strings(), length, flag != 0);
    }
    void crt_invalid_parameter_noinfo_00bf6713() override {
        throw std::logic_error("narrative: checked list iterator failure (00BF6713)");
    }

    // 00734D20, __thiscall(ECX = list; out iterator, container, where), RET 0Ch
    // (listing 00734D83),
    // as 00735100 calls it on the front node (container = the list itself):
    // unlink, 00733DC0 on +8h, _free, size -1.
    void erase_00734d20(std::uint8_t* list, std::uint8_t* where) {
        if (where == load<std::uint8_t*>(list, 0x04)) crt_invalid_parameter_noinfo_00bf6713();
        std::uint8_t* const next = load<std::uint8_t*>(where, 0x00);
        std::uint8_t* const prev = load<std::uint8_t*>(where, 0x04);
        store(prev, 0x00, next);
        store(next, 0x04, prev);
        bsp::cc13_seg40_narrative_entry_destruct_00733dc0(where + 0x08, *this);
        if (released_) released_(where);
        std::free(where);
        store(list, 0x08, load<std::uint32_t>(list, 0x08) - 1u);
    }

    template <typename T>
    static T load(const std::uint8_t* object, std::uint32_t offset) {
        T value;
        std::memcpy(&value, object + offset, sizeof value);
        return value;
    }
    template <typename T>
    static void store(std::uint8_t* object, std::uint32_t offset, T value) {
        std::memcpy(object + offset, &value, sizeof value);
    }

private:
    static bsp::NativeStringStorage& strings() { return game_native_string_process().strings(); }
    static std::uint8_t* operator_new_00bf681b(std::size_t size) {
        void* const block = std::malloc(size);
        if (block == nullptr) throw std::bad_alloc();
        return static_cast<std::uint8_t*>(block);
    }
    // 00734CE0 (listing 00734CE0..00734D1D): head self-linked and +8h = 0
    // first, then each old node: 00733DC0 (ECX = node+8h), _free, until the
    // re-read head.
    void clear(std::uint8_t* list) {
        std::uint8_t* const head = load<std::uint8_t*>(list, 0x04);
        std::uint8_t* node = load<std::uint8_t*>(head, 0x00);
        store(head, 0x00, head);
        store(load<std::uint8_t*>(list, 0x04), 0x04, load<std::uint8_t*>(list, 0x04));
        store<std::uint32_t>(list, 0x08, 0u);
        while (node != load<std::uint8_t*>(list, 0x04)) {
            std::uint8_t* const next = load<std::uint8_t*>(node, 0x00);
            bsp::cc13_seg40_narrative_entry_destruct_00733dc0(node + 0x08, *this);
            if (released_) released_(node);
            std::free(node);
            node = next;
        }
    }
    Released released_;
};

// 008AD540's two callees. The class test's answer is discarded by the binding
// (no TEST after 008AD65F); the task store goes to the gunnery host, which holds
// the repair task at unit+A20h. This host's entity handle is the entity's id
// (index_of), so the task address the body forms (LEA ECX, [entity+A20h] at
// 008AD699) minus A20h is that handle again.
class FailureRepairEnableAdapter final
    : public bsp::cc13_left55::seg56b::Left55FailureRepairEnableHost {
public:
    FailureRepairEnableAdapter(std::function<bool(void*, int)> is_kind_of,
                               std::function<void(void*, bool)> set_enabled)
        : is_kind_of_(std::move(is_kind_of)), set_enabled_(std::move(set_enabled)) {}
    bool entity_vcall_5c(void* entity, int class_id) override {
        return is_kind_of_(entity, class_id);
    }
    void repair_task_set_failure_repair_enabled_00939fe0(void* task, bool enabled) override {
        void* const entity = reinterpret_cast<void*>(
            reinterpret_cast<std::uintptr_t>(task) - static_cast<std::uintptr_t>(0xA20u));
        set_enabled_(entity, enabled);
    }

private:
    std::function<bool(void*, int)> is_kind_of_;
    std::function<void(void*, bool)> set_enabled_;
};

constexpr ScriptOrderBinding kFailureRepairEnableBinding{"FailureRepairEnable", 0x008ad540u};
constexpr ScriptOrderBinding kAddFireBinding{"AddFire", 0x008acd10u};

// Not a kScriptOrderBindings row so that the switch can withhold it: OFF leaves
// UnloadMessageMap the unimplemented record it was.
constexpr ScriptOrderBinding kUnloadMessageMapBinding{"UnloadMessageMap", 0x008c6490u};
constexpr ScriptOrderBinding kSetRaceBinding{"SetRace", 0x008a8720u};

const ScriptOrderBinding* find_binding(const char* name) noexcept {
    if (name == nullptr) return nullptr;
    for (const ScriptOrderBinding& row : kScriptOrderBindings) {
        if (std::strcmp(row.name, name) == 0) return &row;
    }
    if (std::strcmp(kSetRaceBinding.name, name) == 0) return &kSetRaceBinding;
    if (std::strcmp(kUnloadMessageMapBinding.name, name) == 0) return &kUnloadMessageMapBinding;
    // Goal 4 chain 365: offered only ON; OFF leaves both unimplemented records.
    if (std::strcmp(kFailureRepairEnableBinding.name, name) == 0) {
        return &kFailureRepairEnableBinding;
    }
    if (std::strcmp(kAddFireBinding.name, name) == 0) return &kAddFireBinding;
    return nullptr;
}

// bsp::LuaCommandTargetSource and bsp::LuaBindingNavigatorHost index arguments
// the way the native's call frame does: index 0 is Lua stack slot 1.
int stack_slot(int index) noexcept { return index + 1; }

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

// The two command objects the four navigator bindings name. Both constants come
// from include/bsp/unit_commanded_speed.hpp and lua_binding_navigator.hpp.
const char* command_name_of(std::uint32_t object) noexcept {
    if (object == bsp::kCommandObjectAttackMove) return "attackmove";
    if (object == bsp::kCommandObjectMoveTo) return "moveto";
    // Packet cc8_navigator_path. 00E08F80, the third object a navigator row
    // names; the constant is command_execution.hpp's, not a new one.
    if (object == bsp::kCommandMoveOnPath) return "moveonpath";
    // Packet cc13-ver12: NavigatorStop's 00E08F88 (only issued with
    // kLuaNavigatorStopBound).
    if (object == bsp::kCommandStop) return "stop";
    return "";
}

// game+6ACh. One word for the process, as the image has one game object.
std::int32_t g_effective_difficulty_6ac = 0;

}  // namespace

std::int32_t game_effective_difficulty_6ac() noexcept { return g_effective_difficulty_6ac; }
void set_game_effective_difficulty_6ac(std::int32_t value) noexcept {
    g_effective_difficulty_6ac = value;
}

namespace {
GameScriptOrdersHost* g_live_script_orders = nullptr;

// DIAGNOSTIC, env-gated (BSP_LUA_CALLBACK_TRACE=1, cc9-lua33, docs/
// SQUADRON_LAND_TASK.md 5cn): a Lua call hook on the mission state that
// records the first mission-clock time of every Lua function named lua* by its
// caller, and of every unnamed Lua function (a callback the host or a C
// binding calls, keyed by source:line), with a call count. Reported once by
// report(). Prints nothing when unset; no Lua behaviour changes.
struct LuaCallbackTrace {
    std::vector<std::pair<std::string, float>> firsts;
    std::map<std::string, unsigned long long> counts;
    const float* clock = nullptr;
    bool installed = false;
};
LuaCallbackTrace g_lua_callback_trace;

bool lua_callback_trace_enabled() {
    static const bool on = [] {
        char* v = nullptr;
        std::size_t n = 0;
        const bool set = _dupenv_s(&v, &n, "BSP_LUA_CALLBACK_TRACE") == 0 && v != nullptr;
        std::free(v);
        return set;
    }();
    return on;
}

void lua_callback_trace_hook(lua_State* state, lua_Debug* ar) {
    if (ar->event != LUA_HOOKCALL) return;
    if (lua_getinfo(state, "nS", ar) == 0) return;
    if (ar->what == nullptr || std::strcmp(ar->what, "Lua") != 0) return;
    std::string key;
    if (ar->name != nullptr) {
        if (std::strncmp(ar->name, "lua", 3) != 0) return;
        key = ar->name;
    } else {
        key = std::string("?") + ar->short_src + ":" + std::to_string(ar->linedefined);
    }
    unsigned long long& count = g_lua_callback_trace.counts[key];
    if (count++ == 0) {
        g_lua_callback_trace.firsts.emplace_back(
            key, g_lua_callback_trace.clock != nullptr ? *g_lua_callback_trace.clock : -1.0f);
    }
}

// Packet cc13-fix15. 0071C225..0071C231, the sub-kind 3 arm of 0071C1E0:
// director+220h "artilleryEnabled" = (msg+24h != 0). A name with no entry takes
// the constructor's four 1s (007202FD) first. Returns whether the byte moved.
bool record_director_artillery_enable_0071c231(const std::string& name, bool enabled) {
    SceneDirectorEnables enables;
    if (const SceneDirectorEnables* e = scene_director_enables_find(name)) enables = *e;
    const bool changed = enables.artillery != enabled;
    // 0071C1E0 calls apply_command_message_0071c1e0; OFF keeps the direct artillery store.
    if constexpr (kWireScriptOrdersHostBound) {
        bsp::gameplay_seg38_cc13::DirectorFlags flags;
        flags.enables_220[0] = enables.artillery;
        bsp::gameplay_seg38_cc13::apply_command_message_0071c1e0(flags, 3, enabled ? 1 : 0);
        enables.artillery = flags.enables_220[0];
    } else {
    enables.artillery = enabled;
    }
    scene_director_enables_set(name, enables);
    return changed;
}

// Goal 5 chain 660: SetFireTarget 0089A8B0, ArtilleryEnable 0089C590 and TorpedoEnable
// 0089C8F0 run their bodies (src/unit_weapons.cpp) over this adapter, which binds only
// the services those three reach and refuses the rest (UnitWeaponHostUnreached).
// Entities are the dispatch's identities (00888AA0, the table's `ID`: the units-host
// index plus one). The director handle vtable[114h] answers is the unit's own handle:
// the host keeps each director's state by its unit (the ship AI host's fire target, the
// scene director enables by name), and a null answer is the image's no-op arm, as a
// plane's 0047F180 is. 0071DFD0 / 0071E0D0 are __thiscall on that director and route
// the 5Ah message to [director+34h] (0071E026 MOV ECX,[ESI+34h] / 0071E035 CALL
// 0077C2A0, ESI = ECX at 0071DFE9), the unit that owns it; the reconstructed builders
// take no `this`, so the adapter routes to the director it last answered.
class UnitWeaponBinding final : public bsp::UnitWeaponHostUnreached {
public:
    struct Calls {
        std::function<void*(int)> entity_at;                       // 00888AA0
        std::function<bool(int)> get_boolean;                      // 00B66250
        std::function<bool(int)> is_nil;                           // the nil test
        std::function<bool(int)> is_entity_table;                  // 008889C0
        std::function<void(int, float*)> vector3;                  // 00888760
        std::function<bool(void*)> has_director;                   // vtable[114h] non-null
        std::function<void(void*, void*)> set_fire_target;         // 5Eh -> 00836240
        std::function<void(void*, const float*)> set_fire_target_position;
        std::function<void(void*, const bsp::WeaponEnableMessage&)> route_enable;  // 0077C2A0
    };
    explicit UnitWeaponBinding(Calls calls) : calls_(std::move(calls)) {}

    bsp::NativeHandle entity_from_lua_argument(int index) override {
        return reinterpret_cast<bsp::NativeHandle>(calls_.entity_at(index));
    }
    bool lua_argument_bool(int index) override { return calls_.get_boolean(index); }
    bool lua_argument_is_nil(int index) override { return calls_.is_nil(index); }
    bool lua_argument_is_entity(int index) override { return calls_.is_entity_table(index); }
    void lua_argument_vector3(int index, float out[3]) override { calls_.vector3(index, out); }
    // 00B66400: none of the three pushes a value.
    int lua_result_count() override { return 0; }

    bsp::NativeHandle weapon_director(bsp::NativeHandle unit) override {
        director_ = calls_.has_director(entity(unit)) ? unit : 0;
        return director_;
    }
    void director_clear_fire_target(bsp::NativeHandle director) override {
        calls_.set_fire_target(owner(director), nullptr);
    }
    void director_set_fire_target_entity(bsp::NativeHandle director,
                                         bsp::NativeHandle target) override {
        calls_.set_fire_target(owner(director), entity(target));
    }
    void director_set_fire_target_position(bsp::NativeHandle director,
                                           const float position[3]) override {
        calls_.set_fire_target_position(owner(director), position);
    }
    void route_weapon_enable(const bsp::WeaponEnableMessage& message) override {
        if (director_ == 0) unit_query_unreached("0077C2A0 with no director answered");
        calls_.route_enable(owner(director_), message);
    }

private:
    static void* entity(bsp::NativeHandle handle) { return reinterpret_cast<void*>(handle); }
    void* owner(bsp::NativeHandle director) const {
        if (director == 0 || director != director_) {
            unit_query_unreached("a director other than the one vtable[114h] answered");
        }
        return entity(director);
    }
    Calls calls_;
    bsp::NativeHandle director_{0};
};

// GetDirection 008A7CB0 (lua_get_direction): 008A7DD4 CMP byte [ESI+0C8h],0 / JNE past
// 008A7DE7 00414DB0, then 008A7DF9 0088BA30 with EDX = ESI+0CCh+20h, the world matrix's
// forward row +ECh..+F4h. SUBSTITUTION, labelled (packet cc13-fix14b): the row is
// GameUnitsHost::unit_pose's forward row for the entity's slot; the refresh is the host's
// 00414DB0 behind its +C8h test (the host's poses are never stale, so it is not reached).
class DirectionQueryBinding final : public bsp::cc13_seg56::QueryHost {
public:
    DirectionQueryBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        std::function<void(void*)> refresh_if_stale)
        : units_(units), index_of_(std::move(index_of)),
          refresh_if_stale_(std::move(refresh_if_stale)) {}

    bsp::cc13_seg56::Vec3 entity_world_row_ec(void* entity) override {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("+ECh on an entity without a slot");
        refresh_if_stale_(entity);                                       // 008A7DD4..008A7DE7
        float right[3], up[3], forward[3], position[3];
        if (!units_.unit_pose(index, right, up, forward, position)) {
            unit_query_unreached("+ECh on an entity without a units-host pose");
        }
        return {forward[0], forward[1], forward[2]};                     // 008A7DF9
    }
    bool world_ground_height_at(const bsp::cc13_seg56::Vec3&, float&) override {
        unit_query_unreached("00903860");
    }
    bool spatial_query_segment(const bsp::cc13_seg56::Vec3&, const bsp::cc13_seg56::Vec3&,
        bool&, float&) override {
        unit_query_unreached("0098ADD0");
    }
    float entity_vcall_38_speed(void*) override { unit_query_unreached("vtable[38h]"); }
    bsp::cc13_seg56::Vec3 entity_local_to_world(void*, const bsp::cc13_seg56::Vec3&) override {
        unit_query_unreached("004142E0");
    }
    void entity_bounding_box(void*, bsp::cc13_seg56::Vec3&, bsp::cc13_seg56::Vec3&) override {
        unit_query_unreached("+360h box");
    }

private:
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    std::function<void(void*)> refresh_if_stale_;
};

// DoVector3's original number reader (00B66270 -> lua_tonumber) also converts
// numeric strings. Keep this policy local: the shared host get_number reader
// intentionally accepts only LUA_TNUMBER for its existing callers.
class Vector3NumberArgs final : public bsp::LuaBindingArgumentReader {
public:
    Vector3NumberArgs(bsp::LuaBindingArgumentReader& core, lua_State* state)
        : core_(core), state_(state) {}
    int count() override { return core_.count(); }
    int get_integer(int index) override { return core_.get_integer(index); }
    double get_number(int index) override {
        return state_ != nullptr ? lua_tonumber(state_, index + 1) : 0.0;
    }
    bool get_boolean(int index) override { return core_.get_boolean(index); }
    std::string get_string(int index) override { return core_.get_string(index); }
    bool is_string(int index) override { return core_.is_string(index); }
    bool is_nil(int index) override { return core_.is_nil(index); }
    bool is_entity_table(int index) override { return core_.is_entity_table(index); }
    void* entity_at(int index) override { return core_.entity_at(index); }
private:
    bsp::LuaBindingArgumentReader& core_;
    lua_State* state_;
};

// GetDirection / DoVector3's one push, 0088BA30's {x, y, z} table on the dispatch's frame.
class DirectionResults final : public bsp::cc13_seg56::LuaResults {
public:
    explicit DirectionResults(std::function<void(const bsp::cc13_seg56::Vec3&)> push)
        : push_(std::move(push)) {}
    void push_vector3(const bsp::cc13_seg56::Vec3& value) override { push_(value); }
    void push_number(int) override { unit_query_unreached("00B664B0"); }
    void push_float(float) override { unit_query_unreached("00B66480"); }
    void push_boolean(bool) override { unit_query_unreached("00B66450"); }
    void push_nil() override { unit_query_unreached("00B66430"); }
    void push_string(const std::string&) override { unit_query_unreached("00B66710"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unit_query_unreached("00B67930 integer fields");
    }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unit_query_unreached("00B67930 vector fields");
    }

private:
    std::function<void(const bsp::cc13_seg56::Vec3&)> push_;
};

// GetCommandBuildingLevel 0089B6D0 (lua_get_command_building_level): 0089B7E6 MOV
// ESI,[ESI+770h], 0089B7F1 00B664B0. SUBSTITUTION, labelled (packet cc13-fix13): +770h is
// the ship-AI host's capture record (as 006F2780 and 006F38E0 keep it); before the
// records exist, the units host's scene Level (the constructor's value, 0 for a unit that
// is not a CommandBuilding).
class CommandBuildingLevelView final : public bsp::seg56_options_cc13::ImageAccess,
                                       public bsp::cc13_seg56::LuaResults {
public:
    CommandBuildingLevelView(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        std::function<void(int)> push_integer)
        : units_(units), index_of_(std::move(index_of)), push_integer_(std::move(push_integer)) {}

    std::int32_t read_i32(void* entity, std::uint32_t offset) override {
        if (offset != 0x770u) unit_query_unreached("a dword other than +770h");
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("+770h on an entity without a slot");
        std::int32_t level = 0;
        GameShipAiHost* ai = units_.ship_ai();
        if (ai == nullptr || !ai->command_building_level_0770(index, level)) {
            level = units_.command_building_level_0770(index);
        }
        return level;
    }
    void push_number(int value) override { push_integer_(value); }   // 00B664B0

    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void* read_ptr(void*, std::uint32_t) override { unit_query_unreached("read_ptr"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }
    void push_float(float) override { unit_query_unreached("00B66480"); }
    void push_boolean(bool) override { unit_query_unreached("00B66450"); }
    void push_nil() override { unit_query_unreached("00B66430"); }
    void push_string(const std::string&) override { unit_query_unreached("00B66710"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unit_query_unreached("00B67930 integer fields");
    }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unit_query_unreached("0088BA30"); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unit_query_unreached("00B67930 vector fields");
    }

private:
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    std::function<void(int)> push_integer_;
};

// SetCommandBuildingOwnerPlayer 008A8D90 (lua_set_command_building_owner_player):
// 008A8EE1 vtable[5Ch](1Ch), then message D3h routed by 008A8F2A 0077C2A0(&message, 7, 0)
// with ECX the entity. SUBSTITUTIONS, labelled (packet cc13-fix13): the CommandBuilding
// test is the ship-AI host's capture-record lookup, and the message is applied at the call
// through the ship-AI host's D3h (command_building_set_owner_player_006f4d10).
class CommandBuildingOwnerBinding final
    : public bsp::cc13_left55::seg56b::Left55Seg56bHostUnreached {
public:
    CommandBuildingOwnerBinding(GameShipAiHost& ai, std::size_t units,
        std::function<std::size_t(void*)> index_of)
        : ai_(ai), units_(units), index_of_(std::move(index_of)) {}

    bool entity_vcall_5c(void* entity, int class_id) override {
        if (class_id != 0x1C) unit_query_unreached("vtable[5Ch] with a class other than 1Ch");
        std::int32_t level = 0;
        return ai_.command_building_level_0770(slot_of(entity), level);
    }
    void route_message_0077c2a0(void* target,
        const bsp::cc13_left55::seg56b::CommandBuildingOwnerMessage& message, int flags,
        int out) override {
        if (flags != bsp::cc13_left55::seg56b::kRouteFlags || out != 0) {
            unit_query_unreached("a D3h route other than (7, 0)");
        }
        if (ai_.command_building_set_owner_player_006f4d10(slot_of(target), message.player)) {
            ++applied_;
        }
    }
    unsigned applied() const noexcept { return applied_; }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    GameShipAiHost& ai_;
    std::size_t units_;
    std::function<std::size_t(void*)> index_of_;
    unsigned applied_{0};
};
}  // namespace

// ArtilleryEnable 0089C590 / TorpedoEnable 0089C8F0 over UnitWeaponBinding. vtable[114h]
// is null for a plane (0047F180) and for an argument this host cannot name; otherwise the
// unit's director. The 5Ah message is applied where 0071C1E0 applies it, by the unit's
// name: sub-kind 3 at director+220h (0071C231), sub-kind 5 at director+222h (0071C25B).
// SUBSTITUTION, labelled (packet cc13-fix15): stored at the call, not delivered through
// 0077C2A0.
int GameScriptOrdersHost::run_weapon_enable_binding(const char* name) {
    const bool artillery = std::strcmp(name, "ArtilleryEnable") == 0;
    UnitWeaponBinding binding(UnitWeaponBinding::Calls{
        [this](int i) { return entity_at(i); },
        [this](int i) { return get_boolean(i); },
        [this](int i) { return is_nil(i); },
        [this](int i) { return is_entity_table(i); },
        [](int, float*) { unit_query_unreached("00888760"); },
        [this](void* unit) {
            const std::size_t index = index_of(unit);
            const bool plane = index < units_.count() && units_.unit_is_kind_of(index, 0x0f);
            return unit != nullptr && !plane && !name_of(unit).empty();
        },
        [](void*, void*) { unit_query_unreached("00835860"); },
        [](void*, const float*) { unit_query_unreached("00835860 position"); },
        [this](void* unit, const bsp::WeaponEnableMessage& message) {
            const std::string unit_name = name_of(unit);
            if (message.sub_kind == bsp::kWeaponEnableSubKindTorpedo) {
                const bool changed = scene_director_enables_set_torpedo(unit_name, message.enabled);
                ++(message.enabled ? scene_director_torpedo_writes().lua_enables
                                   : scene_director_torpedo_writes().lua_disables);
                log_.notef("  TorpedoEnable: %s director+222h=%d changed=%d (0089C8F0 -> 0071E0D0 "
                    "-> 0071C25B)", unit_name.c_str(), message.enabled ? 1 : 0, changed ? 1 : 0);
            } else if (message.sub_kind == bsp::kWeaponEnableSubKindArtillery) {
                const bool changed =
                    record_director_artillery_enable_0071c231(unit_name, message.enabled);
                log_.notef("  ArtilleryEnable: %s director+220h=%d changed=%d (0089C590 -> "
                    "0071DFD0 -> 0071C231)", unit_name.c_str(), message.enabled ? 1 : 0,
                    changed ? 1 : 0);
            } else {
                unit_query_unreached("a 5Ah sub-kind other than 3 or 5");
            }
        }});
    return artillery ? bsp::lua_artillery_enable_0089c590(binding)
                     : bsp::lua_torpedo_enable_0089c8f0(binding);
}

GameSoundDialogRuntime* GameScriptOrdersHost::bound_music_dialog_runtime() noexcept {
    return bound_music_dialog();
}
void GameScriptOrdersHost::bind_music_dialog(GameSoundDialogRuntime* runtime) noexcept {
    bound_music_dialog() = runtime;
}
void GameScriptOrdersHost::bind_input_context_manager(
    bsp::GameInputContextManagerBinding* manager) noexcept {
    bound_input_context_manager() = manager;
}
void GameScriptOrdersHost::bind_gui_frontend(GameFrontendHost* frontend) noexcept {
    bound_gui_frontend() = frontend;
}

GameScriptOrdersHost::GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units)
    : log_(log), units_(units) {
    // Goal 5 chain 643. BSP_Game_ConstructWorld 004DE610 builds [game+21E8h]:
    // operator new(58h), 00735030 (004DFA75), then 00733030 (004DFA8D). This
    // host owns the narrative for the run, so the object lives as long as it.
    NarrativeObjectServices services([this](const void* node) { release_narrative_node_refs(node); });
    bsp::cc13_seg48U_mission_narrative_construct_00735030(narrative_object_, services);
    bsp::cc13_seg48U_mission_narrative_reset_00733030(narrative_object_);
    g_live_script_orders = this;
}

GameScriptOrdersHost::~GameScriptOrdersHost() {
    if (g_live_script_orders == this) g_live_script_orders = nullptr;
    // 00735420's 00735080. The queued nodes' references are dropped without
    // luaL_unref, as the host always left the shown entry's: the machine state
    // may already be closed.
    NarrativeObjectServices services([this](const void* node) { narrative_node_refs_.erase(node); });
    bsp::cc13_seg40_mission_narrative_destruct_00735080(narrative_object_, services);
}

void GameScriptOrdersHost::set_final_scoring_function_name(const std::string& name) {
    scoring_name_owner_->set_final_scoring_function_name(name);
}

void GameScriptOrdersHost::load_mission_message_map(const std::string& name, int side) {
    // Goal 4 chain 331. 008C62F6 MOV ECX,[00E188A8]; 008C62FC MOV ECX,[ECX+21DCh];
    // 008C6302/008C6307 push the integer then &name; 008C6308 CALL 00706760. No null
    // test: [game+21DCh] is built at 004DFA39 inside construct_world, which the
    // load walk runs before luaStageInit (where the missions call LoadMessageMap,
    // e.g. bsm_01_stationed_at_pearl.lua:39 LoadMessageMap("bsmdlg",1)). The
    // host builds it only under kMessageManagerCc15Bound, so a missing manager is
    // counted instead of dereferenced.
    if (message_manager_ == nullptr || !message_manager_->constructed()) {
        ++message_maps_without_manager_;
        log_.notef("LoadMessageMap(\"%s\", %d): no MessageManager [game+21DCh] (00706760 "
            "not run, goal 4 chain 331)", name.c_str(), side);
        return;
    }
    const std::size_t before = message_manager_->record_count();
    message_manager_->load_message_map_00706760(name, side);
    ++message_maps_loaded_;
    log_.notef("LoadMessageMap(\"%s\", %d): 00706760 records %zu -> %zu (goal 4 chain 331)",
        name.c_str(), side, before, message_manager_->record_count());
}

void GameScriptOrdersHost::unload_mission_message_map(const std::string& name, int side) {
    // Goal 4 chain 331. 008C65C6 MOV ECX,[00E188A8]; 008C65CC MOV ECX,[ECX+21DCh];
    // 008C65D2/008C65D7 push the integer then &name; 008C65D8 CALL 00706FC0. No
    // shipped script calls UnloadMessageMap; the native registers it for every
    // mission state all the same. A missing manager is counted, as for the load.
    if (message_manager_ == nullptr || !message_manager_->constructed()) {
        ++message_maps_without_manager_;
        log_.notef("UnloadMessageMap(\"%s\", %d): no MessageManager [game+21DCh] (00706FC0 "
            "not run, goal 4 chain 331)", name.c_str(), side);
        return;
    }
    message_manager_->unload_message_map_00706fc0(name, side);
    ++message_maps_unloaded_;
    log_.notef("UnloadMessageMap(\"%s\", %d): 00706FC0 (goal 4 chain 331)", name.c_str(), side);
}

std::size_t script_orders_drain_loopback_0076c600() {
    return g_live_script_orders != nullptr
        ? g_live_script_orders->drain_deferred_orders_0076c600() : 0;
}

std::size_t GameScriptOrdersHost::drain_deferred_orders_0076c600() {
    // 0076C600 over the orders posted since the last pump, in post order.
    if (deferred_orders_.empty()) return 0;
    std::vector<DeferredOrder> batch;
    batch.swap(deferred_orders_);
    // Packet cc9_after_row9_continuation. The drain is the delivery, not a
    // script call: the pump can run inside a Blackout callback (GenerateObject's
    // 00874D00 with CL = 1), and what a delivery issues in turn (the squadron
    // fan-out's wingman orders) is the receiver's, so it applies here rather
    // than being queued again. Nor does the delivery belong to the binding row
    // being run, if any.
    const std::string outer_poster = after_row9_poster_;
    GameScriptOrderRow* const outer_row = row_;
    after_row9_poster_.clear();
    row_ = nullptr;
    for (const DeferredOrder& o : batch) {
        apply_issued_order(o.index, o.command_object, o.target, o.flags, o.source,
            o.target_name);
        ++deferred_applied_;
        log_.notef("  after-row-9 queue: %s's order for unit %zu applied at the step's pump "
            "(0076C600)%s", o.poster.c_str(), o.index,
            o.after_apply ? ", then its delivery continuation" : "");
        if (o.after_apply) {
            ++deferred_continuations_;
            delivery_source_ = o.source;
            o.after_apply();
            delivery_source_.clear();
        }
    }
    after_row9_poster_ = outer_poster;
    row_ = outer_row;
    log_.implemented("Session::drain_loopback_queue_0076c600", "00778542");
    return batch.size();
}

void GameScriptOrdersHost::after_order_delivery(std::function<void()> fn) {
    if (last_issue_deferred_ && !deferred_orders_.empty()) {
        DeferredOrder& o = deferred_orders_.back();
        if (o.after_apply) {
            std::function<void()> first = std::move(o.after_apply);
            o.after_apply = [first = std::move(first), fn = std::move(fn)]() {
                first();
                fn();
            };
        } else {
            o.after_apply = std::move(fn);
        }
        return;
    }
    // Packet cc9_set_command_queue_delay: with the loopback queue bound, a
    // direct order's MT_COMMAND waits for the session pump's drain, and the
    // receiver side runs after its delivery there (false when it has been
    // delivered already, or while the switch is off).
    if (commands_after_last_issue_delivery(fn)) return;
    fn();
}

bool GameScriptOrdersHost::handles(const char* binding_name) noexcept {
    const ScriptOrderBinding* binding = find_binding(binding_name);
    if (binding == nullptr) return false;
    if (std::strcmp(binding->name, "MusicFade") == 0) return true;
    if (std::strcmp(binding->name, "EnableInput") == 0) {
        return bound_input_context_manager() != nullptr;
    }
    if (std::strcmp(binding->name, "IsGUIActive") == 0) {
        return bound_gui_frontend() != nullptr;
    }
    // A switch that is off leaves its native an unimplemented record, not a
    // concrete binding that does nothing.
    if (std::strcmp(binding->name, "PilotMoveToRange") == 0) return bsp::kPilotMoveToTaskBound;
    if (std::strcmp(binding->name, "PilotMoveTo") == 0) {
        return kPilotMoveToBound && bsp::kPilotMoveToTaskBound;
    }
    if (std::strcmp(binding->name, "PilotMoveOnPath") == 0) return kPilotMoveOnPathBound;
    if (std::strcmp(binding->name, "AddPowerup") == 0 ||
        std::strcmp(binding->name, "PreparePowerup") == 0 ||
        std::strcmp(binding->name, "GetAvailablePowerups") == 0) {
        return bsp::cc13_powerup::kPowerupGrantBound;
    }
    if (std::strcmp(binding->name, "SetFireTarget") == 0) return kScriptFireTargetBound;
    if (std::strcmp(binding->name, "TorpedoEnable") == 0) return kShipDirectorEnablesBound;
    if (std::strcmp(binding->name, "ArtilleryEnable") == 0) return kLuaArtilleryEnableBound;
    if (std::strcmp(binding->name, "ForceFireDepthCharge") == 0) {
        return kLuaForceFireDepthChargeBound;
    }
    if (std::strcmp(binding->name, "SetSubmarineAirSupply") == 0) {
        return kLuaSetSubmarineAirSupplyBound;
    }
    if (std::strcmp(binding->name, "SquadronSetTravelSpeed") == 0) {
        return kLuaSquadronSetTravelSpeedBound;
    }
    if (std::strcmp(binding->name, "ShipSetTorpedoStock") == 0) return kShipSetTorpedoStockBound;
    if (std::strcmp(binding->name, "ReloadTorpedoes") == 0) return kReloadTorpedoesBound;
    if (std::strcmp(binding->name, "FillPathPoints") == 0) return kFillPathPointsBound;
    if (std::strcmp(binding->name, "PutTo") == 0) return kPutToBound;
    if (std::strcmp(binding->name, "PutRelTo") == 0) return kPutRelToBound;
    if (std::strcmp(binding->name, "GetSquadronPlanes") == 0) return kGetSquadronPlanesBound;
    if (std::strcmp(binding->name, "GetSubmarineDepthLevel") == 0) {
        return kSubmarineDepthLevelBound;
    }
    if (std::strcmp(binding->name, "ForceRecon") == 0) return kReconLevelTableBound;
    if (std::strcmp(binding->name, "DisablePhysics") == 0) return kDisablePhysicsBound;
    if (std::strcmp(binding->name, "AddMatrixInterpolator") == 0) {
        return kAddMatrixInterpolatorBound;
    }
    if (std::strcmp(binding->name, "ExplodeToParts") == 0) return kExplodeToPartsBound;
    if (std::strcmp(binding->name, "NavigatorForceTorpedo") == 0) {
        return kNavigatorForceTorpedoBound;
    }
    if (std::strcmp(binding->name, "EntityTurnToEntity") == 0 ||
        std::strcmp(binding->name, "UnitSetFireStance") == 0) {
        return bsp::kMissionTurnAndStanceBound;
    }
    if (std::strcmp(binding->name, "UnitHoldFire") == 0 ||
        std::strcmp(binding->name, "NavigatorEnable") == 0) {
        return kScriptedOrderNativesBound;
    }
    if (std::strcmp(binding->name, "UnitFreeAttack") == 0 ||
        std::strcmp(binding->name, "UnitFreeFire") == 0) {
        return kLuaUnitFreeFireBound;
    }
    if (std::strcmp(binding->name, "NavigatorStop") == 0) return kLuaNavigatorStopBound;
    if (std::strcmp(binding->name, "NavigatorCruise") == 0) return kLuaNavigatorCruiseBound;
    if (std::strcmp(binding->name, "PutFormationTo") == 0) return kLuaPutFormationToBound;
    if (std::strcmp(binding->name, "SetShipMaxSpeed") == 0) return kScriptedOrderNatives2Bound;
    if (std::strcmp(binding->name, "PilotLand") == 0) return kPilotLandNativeBound;
    if (std::strcmp(binding->name, "PilotRetreat") == 0) return bsp::kPlaneRetreatTaskBound;
    if (std::strcmp(binding->name, "Countdown") == 0 ||
        std::strcmp(binding->name, "CountdownCancel") == 0 ||
        std::strcmp(binding->name, "CountdownTimeLeft") == 0) {
        return kLuaCountdownBound;
    }
    if (std::strcmp(binding->name, "MissionNarrative") == 0 ||
        std::strcmp(binding->name, "MissionNarrativeEnqueue") == 0 ||
        std::strcmp(binding->name, "MissionNarrativeUrgent") == 0 ||
        std::strcmp(binding->name, "MissionNarrativeOverride") == 0 ||
        std::strcmp(binding->name, "MissionNarrativeClear") == 0 ||
        std::strcmp(binding->name, "MissionNarrativeSize") == 0 ||
        std::strcmp(binding->name, "EndScene") == 0) {
        return kLuaMissionNarrativeBound;
    }
    if (std::strcmp(binding->name, "GetCameraState") == 0 ||
        std::strcmp(binding->name, "GetRotation") == 0) {
        return kLuaCameraStateBound;
    }
    if (std::strcmp(binding->name, "GetCapturePercentage") == 0) {
        return kLuaCapturePercentageBound;
    }
    if (std::strcmp(binding->name, "GetCommandBuildingLevel") == 0) {
        return kLuaGetCommandBuildingLevelBound;
    }
    if (std::strcmp(binding->name, "SetCommandBuildingOwnerPlayer") == 0) {
        return kLuaSetCommandBuildingOwnerPlayerBound;
    }
    if (std::strcmp(binding->name, "GetShipMaxSpeed") == 0) return kLuaGetShipMaxSpeedBound;
    if (std::strcmp(binding->name, "GetDirection") == 0) return kLuaGetDirectionBound;
    if (std::strcmp(binding->name, "PlaneForceRelease") == 0) return kLuaPlaneForceReleaseBound;
    if (std::strcmp(binding->name, "SquadronForceRelease") == 0) {
        return kLuaSquadronForceReleaseBound;
    }
    if (std::strcmp(binding->name, "PilotBomb") == 0) return kLuaPilotBombBound;
    if (std::strcmp(binding->name, "PlaneBombFallingTime") == 0) {
        return kLuaPlaneBombFallingTimeBound;
    }
    if (std::strcmp(binding->name, "PlaneReloadBombPlatforms") == 0) {
        return kLuaPlaneReloadBombPlatformsBound;
    }
    if (std::strcmp(binding->name, "GetPlaneIsReloading") == 0) {
        return kLuaGetPlaneIsReloadingBound;
    }
    if (std::strcmp(binding->name, "GetSquadTorpedoes") == 0) return kLuaGetSquadTorpedoesBound;
    if (std::strcmp(binding->name, "GetSubmarineAirSupply") == 0) {
        return kLuaGetSubmarineAirSupplyBound;
    }
    if (std::strcmp(binding->name, "DisplayScores") == 0 ||
        std::strcmp(binding->name, "HideScoreDisplay") == 0) {
        return kLuaDisplayScoresBound;
    }
    return true;
}

// ---------------------------------------------------------------------------
// The argument reads
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::record_unimplemented(const char* method,
    const char* address) {
    log_.unimplemented(method, address);
}

void GameScriptOrdersHost::register_scene_marker(int id, const std::string& name,
    const float world_position[3], int class_id) {
    if (id <= 0) return;
    SceneMarker marker;
    marker.id = id;
    marker.name = name;
    marker.class_id = class_id;
    marker.position[0] = world_position[0];
    marker.position[1] = world_position[1];
    marker.position[2] = world_position[2];
    markers_.push_back(marker);
    // Packet cc9_prcp03_phase_progress: the ship AI's goal needs the same
    // position when a command names this marker.
    units_.register_scene_marker_position(id, world_position);
}

// Packet cc9_building_pad_model. 006F5CC0's third pass, run once on the first
// call, which is after the scene load as the image's InitAll pass C is. The
// registry list 1Dh is taken as the LandingPoint markers in registration order
// (SUBSTITUTION, labelled: the list's append order, 00928860's registrar, was
// not read; order only breaks exact distance ties in 006F2E60 / 006F3AF0).
bsp::BuildingPadModel* GameScriptOrdersHost::building_pads() {
    if (!kBuildingPadModelBound) return nullptr;
    bsp::BuildingPadModel& building_pads_ = bsp::building_pad_model();
    if (building_pads_built_) return &building_pads_;
    building_pads_built_ = true;
    building_pads_.clear();
    std::vector<bsp::BuildingPadModel::Pad> points;
    for (const SceneMarker& marker : markers_) {
        if (marker.class_id != 0x1D) continue;  // LandingPoint, docs/ENTITY_CLASS_IDS.md
        bsp::BuildingPadModel::Pad pad;
        pad.marker_id = marker.id;
        for (int i = 0; i < 3; ++i) pad.position[i] = marker.position[i];
        // 006AC260: the forward row +0ECh / +0F4h (frame[8], frame[10]) over the
        // length of (x, 0, z); zero when that length is not positive.
        float frame[16]{};
        if (units_.scene_marker_frame(static_cast<std::uint32_t>(marker.id), frame)) {
            // 006AC260 reads entity+EC/+F4 after refreshing its world pose.
            // register_scene_marker_frame retains the actual authored world
            // matrix; this view needs no synthetic entity or dirty flag.
            struct MarkerFrameView final : bsp::EntityForwardXzView {
                explicit MarkerFrameView(const float* frame) : frame(frame) {}
                const float* current_world_cc() override { return frame; }
                float vector3_length_00419440(const float xyz[3]) override {
                    return bsp::length_00419440({xyz[0], xyz[1], xyz[2]});
                }
                const float* frame;
            } view(frame);
            float facing[3];
            bsp::cc13_sweepB_entity_forward_xz_006ac260(view, facing);
            pad.facing_x = facing[0];
            pad.facing_z = facing[2];
        }
        points.push_back(pad);
    }
    for (std::size_t unit = 0; unit < units_.count(); ++unit) {
        if (!units_.unit_is_kind_of(unit, 0x1C)) continue;  // MCommandBuilding
        float position[3]{};
        units_.unit_position_00fc(unit, position[0], position[1], position[2]);
        building_pads_.adopt_landing_pads_006f5cc0(static_cast<int>(unit), position,
            units_.command_building_landing_point_range_07cc(unit), points);
    }
    return &building_pads_;
}

// Packet cc8_airops_launch_tick. 006C5050 fills a scene property bag - `Type`
// (the class's +70h), `WingCount` (slot+8h), `Skill`, `Party`, `OwnerPlayer` and
// `HomeBase` off the owner at block+7Ch, `State`, `Equipment` (slot+10h) - and
// hands it to 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen. What that makes is
// a squadron container of 1089 bytes that then holds `WingCount` planes.
//
// LABELLED SUBSTITUTION, and the largest one in this packet. This process has no
// squadron container: `create_plane_squadron_004f0ad0` allocates an instance and
// places it, and nothing in it spawns or flies the wing. So what is created here
// is ONE unit of the slot's own class - a plane - standing for the squadron, and
// the deck reports the authored `WingCount` as its live plane count so the
// committed-planes arithmetic of 006BD3F0 and 006BF230 stays the authored one.
// The Lua `squadron` key therefore names a flyable aircraft rather than a group,
// which is what lets `PilotSetTarget` install a task at all. The wingmen are not
// created. docs/AIROPS_LAUNCH_TICK.md.
std::uint32_t GameScriptOrdersHost::create_air_ops_squadron_006c5050(
    std::uint32_t vehicle_class, std::int32_t wing_count, std::int32_t equipment,
    const std::string& home_base, std::string& created_name,
    std::int32_t& wing_count_out, std::int32_t bag_state, float ceiling_210) {
    created_name.clear();
    wing_count_out = 0;
    if (vehicle_class == 0u) return 0u;
    // A process guard, and RE-JUSTIFIED by packet cc8_airops_deck_brake. It was
    // written as a stand-in for the deck's readiness brake, block+38h.
    // CORRECTED (docs/SQUADRON_LAND_TASK.md 5ax, 5bj): a campaign launch does set
    // that brake. 006C5050 stores each flag-0 squadron at block+38h
    // (006C5314-006C5348), and 006C5B70 releases it once every member is flying
    // or on the ground in state 4. Both are bound behind kBaseLaunchBrakeBound.
    // So this limit is only a safety net, not a stand-in for any native rule.
    //
    // What bounds a campaign launch is the brake together with the mission
    // script's own gate -
    // `stloPlaneNum < 2` for each American carrier and `< 4` for each Japanese
    // one - and that gate works only because the tick keeps slot+28h filled. Over
    // USN04's six decks it admits about two dozen squadrons, so the number below
    // is at the bound rather than under it and has never fired. It stays as a
    // safety net against a mission whose gates this process does not satisfy,
    // raised so that it cannot shadow the script's own pacing.
    // docs/USN04_STRIKE_CLASS.md.
    // Packet cc9_lua56_squadron_ceiling (kAirOpsSquadronCeilingRaisedBound): the
    // 64 counted every squadron ever made, dead or alive, and on USN21 (six decks,
    // a strike every 400 frames) it stopped all launches at 192 aircraft while the
    // decks still held stock (`squadron=0` from about 860 s). The image has no such
    // count. True: the net is 4096, far above any mission's pacing. False: 64.
    constexpr bool kAirOpsSquadronCeilingRaisedBound = true;   // ON: SQUADRON_LAND_TASK 5gr.1; ESMP08 needs ~72 squadrons (cc9-physics6)
    constexpr std::size_t kSquadronCreateLimit = kAirOpsSquadronCeilingRaisedBound ? 4096 : 64;
    // Metres above the carrier's own origin. The native launches the plane off
    // the deck through the taxi and catapult paths; none of that is reconstructed
    // here, so the squadron starts airborne over its home base. contract.
    constexpr float kAirOpsSquadronLaunchAltitude = 150.0f;
    if (squadrons_.size() >= kSquadronCreateLimit) {
        if (!squadron_limit_logged_) {
            squadron_limit_logged_ = true;
            log_.notef("air ops squadron: the %zu-squadron ceiling of this process is "
                "reached; further launches leave slot+28h zero. It is a safety net, not "
                "a native rule: the deck's readiness brake (block+38h, 006C5050 / "
                "006C5B70) and the mission script's own stloPlaneNum gate are what pace "
                "a launch",
                kSquadronCreateLimit);
        }
        return 0u;
    }

    // kAirOpsOwnerByInstanceBound: the live instance, as 006C5050's block+7Ch is.
    const std::size_t owner = bsp::air_ops_owner_row(units_.count(), home_base,
        [&](std::size_t i) -> const std::string* {
            const GameUnitRow* row = units_.unit_row(i);
            return row != nullptr ? &row->name : nullptr;
        },
        [&](std::size_t i) { return units_.gunnery() != nullptr && units_.gunnery()->unit_dead(i); });
    if (owner >= units_.count()) {
        log_.notef("air ops squadron: `HomeBase` \"%s\" names no created instance, so "
            "006C5050's bag has no owner to read `Party` off and nothing is created",
            home_base.c_str());
        return 0u;
    }
    const GameUnitRow* owner_row = units_.unit_row(owner);
    if (owner_row == nullptr) return 0u;

    GameSceneEntityRecord record;
    char suffix[32];
    std::snprintf(suffix, sizeof(suffix), "_sqn%02zu", squadrons_.size() + 1);
    record.name = home_base + suffix;
    record.class_name = "PlaneSquadronGen";
    record.class_id = 0x18;  // the creator row 004F0AD0 sits on
    record.type_id = static_cast<int>(vehicle_class);
    // `Party` is the owner's +54h, which for this process is the owner unit's own
    // party rather than the deck's unfilled field.
    record.party = owner_row->party;
    // Packet cc9_plane_scene_equipment: 006C5232..006C523E push `Equipment` into
    // the bag only when positive; otherwise the key is absent (0).
    record.bag_equipment = equipment > 0 ? equipment : 0;
    record.created = true;
    // The world 4x4 create_units reads: rows 0..2 the body axes, row 3 the
    // position. The native places the squadron through 004F03C0's deferral with
    // the carrier's own frame; this puts it over the deck, airborne, because the
    // plane rows create_units seeds (+900h = 7, +908h = 3600) describe a plane in
    // free flight and a plane on the sea surface is not that.
    record.world[0] = 1.0f;
    record.world[5] = 1.0f;
    record.world[10] = 1.0f;
    record.world[12] = owner_row->position[0];
    record.world[13] = owner_row->position[1] + kAirOpsSquadronLaunchAltitude;
    record.world[14] = owner_row->position[2];
    if constexpr (kBaseLaunchChainBound) {
        // Packet cc9_base_launch_inside_start (docs/SQUADRON_LAND_TASK.md 5ax).
        // 006C5050 copies the owner's world matrix (owner+CCh, 16 dwords) into
        // the bag and adds the random launch height (tuning+210h) to its y only
        // with the airborne flag (006C5050's param_2), which this host's one
        // launch path, 006CC733, pushes as 0. So the squadron is made at the
        // base's own origin, with `State` 1; the axes stay the identity
        // (SUBSTITUTION, labelled: the owner's rotation rows are not copied).
        // That holds only with kBaseLaunchAxesCc13Bound OFF; ON, the block
        // below copies the owner's rows 0..2 (cc13-launchaxes).
        // CORRECTED (cc13-tier1): the flag arm REPLACES y (Ceiling + the draw;
        // nothing is added to the owner's height), and 006CC733 is not the only
        // path: LaunchAirBaseSlot(base, slot, true) reaches 006C7490(slot, 1)
        // through 006CA8FA. That arm is the block below.
        record.world[13] = owner_row->position[1];
        // cc13-launchaxes, kBaseLaunchAxesCc13Bound (include/bsp/
        // base_launch_axes_cc13.hpp). 006C527A-006C5294 REP MOVSD 10h dwords from
        // owner+CCh: the right, up and forward rows land in the bag matrix rows
        // 0..2 unchanged (no transform, no orthonormalisation), on both flag
        // arms; the flag arm below replaces only y. ABI: none (host record).
        // Uncertain: unit_pose reads the host's slot world, which the motion sync
        // refreshes; the image's 00414DB0 refresh (owner+C8h clear, 006C526A) has
        // no separate counterpart here.
        if constexpr (kBaseLaunchAxesCc13Bound) {
            float right[3], up[3], forward[3], translation[3];
            if (units_.unit_pose(owner, right, up, forward, translation)) {
                for (int lane = 0; lane < 3; ++lane) {
                    record.world[lane] = right[lane];
                    record.world[4 + lane] = up[lane];
                    record.world[8 + lane] = forward[lane];
                }
            }
        } else {
            // DIAGNOSTIC (cc9-physics29): the OFF census, whether the ON arm would
            // replace the identity axes (the owner's rows differ from identity).
            float right[3], up[3], forward[3], translation[3];
            if (units_.unit_pose(owner, right, up, forward, translation)) {
                double dev = 0.0;
                for (int lane = 0; lane < 3; ++lane) {
                    dev = std::max(dev, std::fabs(static_cast<double>(right[lane]) - (lane == 0)));
                    dev = std::max(dev, std::fabs(static_cast<double>(up[lane]) - (lane == 1)));
                    dev = std::max(dev, std::fabs(static_cast<double>(forward[lane]) - (lane == 2)));
                }
                log_.notef("air ops launch axes census: squadron=%s owner=%s identity=%d "
                    "max_dev=%.4f (kBaseLaunchAxesCc13Bound OFF, cc9-physics29 diagnostic)",
                    record.name.c_str(), home_base.c_str(), dev == 0.0 ? 1 : 0, dev);
            }
        }
    }
    // cc13-tier1, kAirOpsLaunchFlagAirborneBound (include/bsp/
    // air_ops_launch_flag_cc13.hpp): 006C5050's flag arm, reached from
    // LaunchAirBaseSlot(base, slot, true) through 006CA8FA. 006C5298-006C52C5
    // REPLACE the copied y with Dynamics/Ceiling (tuning+210h) + 00BD2F10(1,
    // 100.0, 300.0); x and z stay the owner's. The draw is taken here, after the
    // bag and the owner copy and before 004F0AD0 (create_units below), as in the
    // image. `VelocitySI` (class+18Ch TravelSpeed, 006C51EC) is the seed
    // create_units already gives every plane, so it has no field here.
    const bool airborne_launch = kAirOpsLaunchFlagAirborneBound && bag_state == 7;
    if (airborne_launch) {
        float draw = 0.0f;
        bool drew = false;
        if constexpr (kAirOpsLaunchFlagAltitudeDrawBound) {
            if (GameGunneryHost* gunnery = units_.gunnery()) {
                draw = gunnery->death_mode_draw_00bd2f10(1, owner,
                    kAirOpsLaunchAltitudeDrawLow, kAirOpsLaunchAltitudeDrawHigh);  // 006C52BA
                drew = true;
            } else {
                log_.notef("air ops squadron: WARNING kAirOpsLaunchFlagAltitudeDrawBound is ON "
                    "but there is no gunnery host, so the 006C52BA 00BD2F10 draw for \"%s\" "
                    "is NOT taken and y falls back to the Ceiling alone (packet cc13-tier1)",
                    home_base.c_str());
            }
        }
        // 006C52BF FADD dword [tuning+210h] under 24-bit precision, 006C52C5 FSTP
        // dword: one float rounding, the float sum. With the draw switch off (or
        // no gunnery host) y is the Ceiling alone, with NO random offset
        // (SUBSTITUTION, labelled; the image has no constant to stand in).
        record.world[13] = drew ? draw + ceiling_210 : ceiling_210;
        log_.notef("air ops squadron: airborne launch for \"%s\" (006C5050 flag arm, State 7): "
            "y = %s = %.2f, x/z the owner's (006C5298-006C52C5, packet cc13-tier1)",
            home_base.c_str(),
            drew ? "Ceiling + 00BD2F10(1, 100, 300)" : "Ceiling, 00BD2F10 draw NOT taken",
            static_cast<double>(record.world[13]));
    }

    // 006C5050 fills `WingCount` from slot+8h and hands the bag to 004F0AD0,
    // whose slot-39 attach 007F4580 then makes that many planes. This is the
    // second of the two creation seams and it runs the same reconstructed loop
    // as the authored scene row, so a launched squadron has the same shape as an
    // authored one: the squadron record IS wing 0 (the fused flight leader, the
    // substitution docs/PLANE_SQUADRON_HOST.md labels) and wings 1..n-1 are
    // created beside it in the same batch.
    bsp::PlaneSquadronSpawnRequest request;
    request.squadron_name = record.name;
    request.type_class_id = static_cast<std::int32_t>(vehicle_class);
    request.wing_count_present = wing_count > 0;
    request.wing_count_raw = wing_count;
    const bsp::PlaneSquadronSpawnPlan plan =
        bsp::plane_squadron_plan_members_007f4580(request);

    const std::size_t before = units_.count();
    std::vector<GameSceneEntityRecord> batch;
    batch.push_back(record);
    std::vector<GameSceneEntityRecord> wing_records;
    for (std::size_t wing = 1; wing < plan.members.size(); ++wing) {
        GameSceneEntityRecord wing_record = record;
        wing_record.name = plan.members[wing].name;
        // 007F4811 builds the plane through the vehicle class's vtable +28h,
        // which for this `Type` is 007CFD20 BSP_PlaneUnitInstance_Construct: a
        // plane, not a second PlaneSquadronGen.
        wing_record.class_name = "PlaneUnitInstance";
        wing_record.class_id = -1;
        // Packet cc9_wing_construction: held for the squadron's pass A.
        if constexpr (kWingConstructionInPassABound) {
            wing_records.push_back(std::move(wing_record));
        } else {
            batch.push_back(std::move(wing_record));
        }
    }
    // The registry record is built BEFORE create_units, not after it. That
    // placement was made when create_units constructed a fresh
    // GameAiCoordinatorHost on every batch and ran build_squadrons at its tail,
    // so a record added afterwards missed that census. On USN04 (one scene row,
    // four launches) the census then stepped 1 over 3, 4 over 6, 5 over 9, 6
    // over 12, 7 over 15: each launch grouped the PREVIOUS one.
    //
    // That is no longer how the coordinator lives. Since packet
    // cc8_ship_follow, create_units builds it once, at the initial scene pass,
    // guarded by `host.ai == nullptr` (src/game_hosts_units.cpp), and
    // build_squadrons runs only inside that one create_00a32350. So an air-ops
    // launch no longer reaches the census, before or after create_units, and
    // this placement is kept only because it is harmless.
    // docs/GUNNERY_HOST_LIFETIME.md section 8 audits this path.
    //
    // Nothing here needs the units to exist yet: `before` is already captured
    // and the batch order IS the wing order, so +3D0h[wing] = before + wing is
    // known in advance. The count guard the old placement used moves below,
    // where create_units has said how many it actually made.
    bsp::PlaneSquadronHostRecord& squadron =
        bsp::plane_squadron_registry().add(record.name);
    squadron.wing_count = plan.wing_count;       // +3C8h
    squadron.behaviour = plan.behaviour;         // +364h
    squadron.type_class_id = static_cast<std::int32_t>(vehicle_class);
    squadron.party = record.party;
    squadron.from_air_ops_launch = true;
    // The bag's `HomeBase` (006C5050, the owner at block+7Ch) and `State`: 1,
    // since 006CC733 pushes the airborne flag as 0 (006C5050: State =
    // flag ? 7 : 1). Read by the squadron's pass C behind kBaseLaunchChainBound.
    // (The flag-1 arm, from 006CA8FA, is the 7 below.)
    squadron.bag_home_base = home_base;
    // kAirOpsLaunchFlagAirborneBound: 7 on the flag arm, so 007F4C36 SETLE gives
    // flag 0, 007F1C74 leaves +408h clear and 007F4DB0 runs 007F2920 (the
    // host's pass C place_squadron_members_007f2920) instead of the inside start.
    squadron.bag_state = airborne_launch ? 7 : 1;
    squadron.squadron_unit = before;
    squadron.member_names.clear();
    squadron.member_units.clear();
    squadron.member_spawn_index.clear();
    const std::size_t registered = kWingConstructionInPassABound
        ? plan.members.size() : batch.size();
    for (std::size_t wing = 0; wing < plan.members.size() && wing < registered; ++wing) {
        squadron.member_names.push_back(wing == 0 ? record.name
                                                  : plan.members[wing].name);
        // +3D0h[wing]. Packet cc9_wing_construction: a wing plane's index is the
        // one the pass A hook gives it, so the slot waits empty until then.
        const bool held = kWingConstructionInPassABound && wing > 0;
        squadron.member_units.push_back(held ? bsp::kPlaneSquadronNoUnit : before + wing);
        squadron.member_spawn_index.push_back(plan.members[wing].spawn_index);
    }

    units_.create_units(batch);
    if (units_.count() <= before) {
        // The record was registered on the expectation that create_units would
        // make the slots. It made none, so empty it rather than leave a
        // squadron naming units that do not exist.
        squadron.member_names.clear();
        squadron.member_units.clear();
        squadron.member_spawn_index.clear();
        squadron.squadron_unit = bsp::kPlaneSquadronNoUnit;
        log_.notef("air ops squadron: create_units made no instance for class %u, so "
            "slot+28h stays zero", vehicle_class);
        return 0u;
    }
    // A partial batch is the case the old `unit >= units_.count()` guard
    // covered: drop any slot whose plane was not actually made, so live_count()
    // and the flight leader stay honest.
    for (std::size_t wing = 0; wing < squadron.member_units.size(); ++wing) {
        if (squadron.member_units[wing] >= units_.count()) {
            squadron.member_units[wing] = bsp::kPlaneSquadronNoUnit;
        }
    }
    if constexpr (kWingConstructionInPassABound) {
        if (!wing_records.empty()) units_.stage_squadron_wing(before, std::move(wing_records));
    }
    AirOpsSquadron made;
    made.unit_index = before;
    made.entity_id = static_cast<std::uint32_t>(made.unit_index + 1);
    made.wing_count = plan.plane_count > 0 ? plan.plane_count : 1;
    made.name = record.name;
    squadrons_.push_back(made);
    created_name = made.name;
    wing_count_out = made.wing_count;
    const GameUnitRow* made_row = units_.unit_row(made.unit_index);
    log_.notef("air ops squadron: %s class=%u wing=%d equipment=%d home=%s -> unit %zu "
        "id %u at (%.1f %.1f %.1f) class_row=%s (006c5050 bag -> 004f0ad0)",
        made.name.c_str(), vehicle_class, made.wing_count, equipment, home_base.c_str(),
        made.unit_index, made.entity_id, record.world[12], record.world[13],
        record.world[14],
        (made_row != nullptr && made_row->class_row_found) ? "found" : "absent");
    log_.implemented("AirOps::build_squadron", "006c5050");
    return made.entity_id;
}

// Packet cc9_lua52_kamikaze_wiring. 006FED18-006FEED3: the bag of the released
// kamikaze (Type = KamikazePlaneClass's class, WingCount 1, Skill, Race, Party,
// OwnerPlayer, State 7, VelocitySI, Vel3D, Invincible 0.0, FireStance 2,
// AutoAttackTarget, DummyObjectID) into 004F0AD0 at the round's matrix, the
// creator create_air_ops_squadron_006c5050 already reaches. SUBSTITUTIONS,
// labelled: VelocitySI/Vel3D, Invincible, FireStance and DummyObjectID have no
// reader in create_units (the plane starts at the units host's seeded speed);
// the name is the carrier's plus a counter, where 004F0AD0 takes
// [desc+DCh]+13Ch. The AutoAttackTarget is the caller's.
std::uint32_t GameScriptOrdersHost::create_dummy_kamikaze_squadron_006fec00(
    const bsp::DummyKamikazeSpawn& spawn, std::string& created_name, const char* suffix_format) {
    created_name.clear();
    if (spawn.plane_class <= 0) return 0u;
    const GameUnitRow* carrier = units_.unit_row(spawn.carrier);
    GameSceneEntityRecord record;
    char suffix[32];
    std::snprintf(suffix, sizeof(suffix), suffix_format, spawn.dummy_id);
    record.name = (carrier != nullptr ? carrier->name : std::string("DummyKamikaze")) + suffix;
    record.class_name = "PlaneSquadronGen";
    record.class_id = 0x18;
    record.type_id = spawn.plane_class;
    record.party = spawn.party;
    record.race = spawn.race;
    record.owner_player = spawn.owner_player;
    record.bag_skill = spawn.skill;
    record.bag_skill_source = "Skill";
    record.resource_usage_304 = spawn.resource_usage;
    // The bag's Equipment when the caller's bag has one (MCatapult::Fire
    // 006ECA20); the kamikaze bag has none and leaves the record's -1.
    record.bag_equipment = spawn.equipment;
    record.created = true;
    for (int i = 0; i < 16; ++i) record.world[i] = spawn.world[static_cast<std::size_t>(i)];

    bsp::PlaneSquadronSpawnRequest request;
    request.squadron_name = record.name;
    request.type_class_id = spawn.plane_class;
    request.wing_count_present = true;
    request.wing_count_raw = 1;                       // 006FED63 PUSH 1, WingCount
    // The bag's Behaviour when the caller's bag has one (MCatapult::Fire
    // 006ECA2F, 1); 007F47C7 stores it at +364h. The kamikaze bag has none,
    // so -1 leaves the key absent and the constructor's -1 seed stands.
    request.behaviour_present = spawn.behaviour >= 0;
    request.behaviour = spawn.behaviour;
    const bsp::PlaneSquadronSpawnPlan plan = bsp::plane_squadron_plan_members_007f4580(request);

    const std::size_t before = units_.count();
    bsp::PlaneSquadronHostRecord& squadron = bsp::plane_squadron_registry().add(record.name);
    squadron.wing_count = plan.wing_count;
    squadron.behaviour = plan.behaviour;
    squadron.type_class_id = spawn.plane_class;
    squadron.party = record.party;
    squadron.from_air_ops_launch = true;               // a mission-time creation
    squadron.bag_state = 7;                            // 006FEDC7 PUSH 7, State
    squadron.squadron_unit = before;
    squadron.member_names.clear();
    squadron.member_units.clear();
    squadron.member_spawn_index.clear();
    if (!plan.members.empty()) {
        squadron.member_names.push_back(record.name);
        squadron.member_units.push_back(before);
        squadron.member_spawn_index.push_back(plan.members[0].spawn_index);
    }
    std::vector<GameSceneEntityRecord> batch;
    batch.push_back(record);
    units_.create_units(batch);
    if (units_.count() <= before) {
        squadron.member_names.clear();
        squadron.member_units.clear();
        squadron.member_spawn_index.clear();
        squadron.squadron_unit = bsp::kPlaneSquadronNoUnit;
        log_.notef("dummy kamikaze spawn: create_units made no instance for class %d",
                   spawn.plane_class);
        return 0u;
    }
    AirOpsSquadron made;
    made.unit_index = before;
    made.entity_id = static_cast<std::uint32_t>(made.unit_index + 1);
    made.wing_count = 1;
    made.name = record.name;
    squadrons_.push_back(made);
    created_name = made.name;
    const GameUnitRow* target = spawn.target_plus_one > 0
        ? units_.unit_row(spawn.target_plus_one - 1) : nullptr;
    log_.notef("dummy kamikaze spawn: %s class=%d party=%d skill=%d -> unit %zu at "
        "(%.1f %.1f %.1f) v=(%.1f %.1f %.1f) |v|=%.1f target=%s dummy=%u "
        "(006FEC00 -> 004F0AD0, packet cc9_lua52_kamikaze_wiring)",
        made.name.c_str(), spawn.plane_class, spawn.party, spawn.skill, made.unit_index,
        static_cast<double>(record.world[12]), static_cast<double>(record.world[13]),
        static_cast<double>(record.world[14]), static_cast<double>(spawn.velocity[0]),
        static_cast<double>(spawn.velocity[1]), static_cast<double>(spawn.velocity[2]),
        static_cast<double>(spawn.speed), target != nullptr ? target->name.c_str() : "-",
        spawn.dummy_id);
    log_.implemented("DummyKamikazePlane::spawn_kamikaze", "006fec00");
    return made.entity_id;
}

// 006CDC70 BSP_AirOps_Update, once per fixed step over every deck this process
// built. POSITION IS A DEVIATION, and a deliberate one: the executable reaches
// 006CDC70 from the owning unit's own motion pass, 00758270
// BSP_MotherShipUnit_UpdateMotion for a carrier and 006D2510
// BSP_AirField_TickAdvance for an airfield, and the natural home here is
// GameUnitsHost::motion_step_00825f20. That file is leased to another worker for
// the length of this packet, so the walk runs from this host's own per-frame
// pass instead, which the mission frame drives on the same fixed step. The order
// within the frame therefore differs from the native's; nothing in the tick
// reads anything the motion pass writes, so the difference is one of position
// and not of result. docs/AIROPS_LAUNCH_TICK.md.
void GameScriptOrdersHost::run_air_ops_update_006cdc70(float step) {
    if (!(step > 0.0f)) return;
    // The squadron table's +3D0h array holds names until create_units has made
    // the units; this is the per-frame pass that keeps it current for a mission
    // that never calls PilotSetTarget. It returns at once unless the unit count
    // has moved.
    resolve_plane_squadron_members();
    // A squadron whose unit is gone is what 007F1B70 hands 006C65B0. This process
    // has no destruction path that calls it, so the reader's own zero is the
    // signal and the release runs here, once, for each slot still holding it.
    bsp::AirOpsDeckRegistry& registry = bsp::air_ops_decks();
    for (std::size_t deck_index = 0; deck_index < registry.size(); ++deck_index) {
        bsp::AirOpsDeck* deck = registry.mutable_at(deck_index);
        if (deck == nullptr) continue;
        for (const bsp::AirOpsSlot& slot : deck->slots) {
            const std::uint32_t squadron = slot.launched_squadron;
            if (squadron == 0u) continue;
            if (air_ops_squadron_plane_count(squadron) > 0) continue;
            air_ops_released_ += bsp::air_ops_release_squadron_slot_006c65b0(*deck, squadron);
            break;  // the walk above is invalidated by the release
        }
    }
    const bsp::AirOpsDeckTickResult tick = bsp::air_ops_update_decks_006cdc70(step);
    // Packet cc9_player_air_ops_launch: 006C64B0, sub-update 4 of 006CDC70.
    if constexpr (bsp::kAirOpsPlayerLaunchBound) run_air_ops_player_launch_queue(step);
    // 006CDCDC: the landing queue 006CD240, after 006C0DA0 in 006CDC70's order
    // (006C77E0, 006C64B0 and 006C6540 between them are not run here). Packet
    // cc9_landing_sequencer; returns at once while kLandingSequencerBound is off.
    units_.run_landing_queue_006cd240(step);
    air_ops_ticks_ += tick.slots;
    air_ops_refills_ += tick.became_ready;
    air_ops_tracking_ = tick.tracking;
    if (tick.became_ready > 0 && air_ops_refill_logs_ < 8) {
        ++air_ops_refill_logs_;
        log_.notef("air ops tick 006c0510: %zu slot(s) left state 3/4 for state 5 this "
            "step; %zu slot(s) still hold a squadron, %llu released so far",
            tick.became_ready, tick.tracking, air_ops_released_);
    }
    log_.implemented("AirOps::update", "006cdc70");
    log_.implemented("AirOps::update_slots", "006c0da0");
    log_.implemented("AirOps::slot_tick", "006c0510");
}

std::uint32_t GameScriptOrdersHost::create_unit_from_scene_record_0046db4b(
    const GameSceneEntityRecord& record) {
    // 0046DB4B `CALL EAX` with EAX = [descriptor+4h] is the instantiate-pass
    // creator, the same descriptor[1] the .scn reader uses, so a script-spawned
    // object is created exactly as a load-time one is. This host reaches that
    // creator the way the scene pass does, through the units host.
    GameSceneEntityRecord copy = record;
    copy.created = true;
    copy.skipped_because.clear();
    const std::size_t before = units_.count();
    std::vector<GameSceneEntityRecord> batch;
    batch.push_back(copy);

    // The third creation seam. A held-back PlaneSquadronGen row reaches 004F0AD0
    // here rather than at scene load, and its slot-39 attach 007F4580 spawns the
    // same wing; the property bag did not survive the hold-back, so `WingCount`
    // was carried on the pool entry when the row was held.
    //
    // UNVALIDATED BY A RUN: neither USN01 nor USN04 reaches a GenerateObject call
    // in its frame budget, so this seam is reconstructed to the same rule as the
    // other two and measured by neither. docs/PLANE_SQUADRON_HOST.md.
    bsp::PlaneSquadronSpawnPlan plan;
    std::vector<GameSceneEntityRecord> held_wing;  // packet cc9_wing_construction
    const bool is_squadron = copy.class_id == 0x18;
    if (is_squadron) {
        bsp::PlaneSquadronSpawnRequest request;
        request.squadron_name = copy.name;
        request.type_class_id = copy.type_id;
        if (const SceneSpawnPoolEntry* held =
                scene_spawn_pool().find(copy.name)) {
            request.wing_count_present = held->wing_count_present;
            request.wing_count_raw = held->wing_count_raw;
        }
        plan = bsp::plane_squadron_plan_members_007f4580(request);
        for (std::size_t wing = 1; wing < plan.members.size(); ++wing) {
            GameSceneEntityRecord wing_record = copy;
            wing_record.name = plan.members[wing].name;
            wing_record.class_name = "PlaneUnitInstance";
            wing_record.class_id = -1;
            // Packet cc9_wing_construction: held for the squadron's pass A.
            if constexpr (kWingConstructionInPassABound) {
                held_wing.push_back(std::move(wing_record));
            } else {
                batch.push_back(std::move(wing_record));
            }
        }
    }

    units_.create_units(batch);
    if (units_.count() <= before) return 0u;
    const std::size_t index = before;
    if (is_squadron && !plan.members.empty()) {
        bsp::PlaneSquadronHostRecord& squadron =
            bsp::plane_squadron_registry().add(copy.name);
        squadron.wing_count = plan.wing_count;
        squadron.behaviour = plan.behaviour;
        squadron.type_class_id = copy.type_id;
        squadron.party = copy.party;
        squadron.squadron_unit = index;
        squadron.member_names.clear();
        squadron.member_units.clear();
        squadron.member_spawn_index.clear();
        for (std::size_t wing = 0; wing < plan.members.size(); ++wing) {
            const std::size_t unit = index + wing;
            // Packet cc9_wing_construction: a held wing plane's slot waits for
            // the index the pass A hook gives it.
            const bool held = kWingConstructionInPassABound && wing > 0;
            if (!held && unit >= units_.count()) break;
            squadron.member_names.push_back(wing == 0 ? copy.name
                                                      : plan.members[wing].name);
            squadron.member_units.push_back(held ? bsp::kPlaneSquadronNoUnit : unit);
            squadron.member_spawn_index.push_back(plan.members[wing].spawn_index);
        }
        if constexpr (kWingConstructionInPassABound) {
            if (!held_wing.empty()) units_.stage_squadron_wing(index, std::move(held_wing));
        }
        log_.notef("GenerateObject squadron %s: WingCount=%d -> %d member plane(s) "
            "(007F4580 mode 1 on the held-back row)", copy.name.c_str(),
            plan.wing_count, plan.plane_count);
    }
    return static_cast<std::uint32_t>(index + 1);
}

std::int32_t GameScriptOrdersHost::air_ops_squadron_plane_count(
    std::uint32_t squadron) const noexcept {
    for (const AirOpsSquadron& made : squadrons_) {
        if (made.entity_id != squadron || squadron == 0u) continue;
        // entity+3CCh is the squadron's live plane count, and since packet
        // cc8_plane_squadron_host the squadron has real members: the count is the
        // number of entries in its +3D0h array whose unit is still active, which
        // is what 007F3970's compaction leaves behind. The authored wing is no
        // longer reported in its place.
        const bsp::PlaneSquadronHostRecord* record =
            bsp::plane_squadron_registry().find(made.name);
        if (record == nullptr) {
            const GameUnitRow* row = units_.unit_row(made.unit_index);
            if (row == nullptr || !row->active) return 0;
            return made.wing_count;
        }
        std::int32_t live = 0;
        for (std::size_t member : record->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            const GameUnitRow* row = units_.unit_row(member);
            if constexpr (kBaseLaunchChainBound) {
                // Packet cc9_base_launch_task. A member Inside its base is a
                // disabled entity (+5Ch clear, 007BC550) but still in +3D0h:
                // only 007F3970's compaction at death takes it out. So the count
                // is the members not out of action (+5Dh), destroyed (+5Eh) or
                // removed (+5Fh), not the enabled ones.
                bsp::SceneNodeFlags flags;
                if (row == nullptr) continue;
                if (!units_.unit_scene_node_flags(member, flags)) {
                    if (row->active) ++live;
                } else if (!flags.torn_down && !flags.destroyed && !flags.removed) {
                    ++live;
                }
            } else {
                if (row != nullptr && row->active) ++live;
            }
        }
        return live;
    }
    return 0;
}

const GameScriptOrdersHost::SceneMarker* GameScriptOrdersHost::marker_for_id(
    int id) const noexcept {
    for (const SceneMarker& marker : markers_) {
        if (marker.id == id) return &marker;
    }
    return nullptr;
}

void* GameScriptOrdersHost::entity_from_argument(int index) {
    // 00888aa0 BSP_ObjectHandle_FromLuaTable. The native validates the table and
    // converts its `Ptr` to the entity object; this process has no such object,
    // so the resolve is the `ID` field 00928a00 seeds turned into the created
    // instance of that index. Milestone 2l established that identity.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return nullptr;
    if (lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "ID");
    // 00928BA5 seeds `ID` as the key text; lua_tonumber converts it.
    const int type = lua_type(state_, -1);
    const bool number = type == LUA_TNUMBER || type == LUA_TSTRING;
    const int id = number ? static_cast<int>(lua_tonumber(state_, -1)) : 0;
    lua_settop(state_, argument_count_);
    if (!number || id <= 0) return nullptr;
    const std::size_t unit = static_cast<std::size_t>(id - 1);
    // Packet cc_lua_find_entity: the native's 00888AA0 validates the table and
    // converts its `Ptr`, and every entity that reached virtual slot 39 has one,
    // unit or not. A scene marker is therefore as resolvable here as a unit is.
    if (unit >= units_.count() && marker_for_id(id) == nullptr) return nullptr;
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(id));
}

std::size_t GameScriptOrdersHost::index_of(void* entity) const noexcept {
    const std::uintptr_t id = reinterpret_cast<std::uintptr_t>(entity);
    if (id == 0) return static_cast<std::size_t>(-1);
    return static_cast<std::size_t>(id - 1);
}

std::string GameScriptOrdersHost::name_of(void* entity) const {
    const std::size_t index = index_of(entity);
    const GameUnitRow* row = (index < units_.count()) ? units_.unit_row(index) : nullptr;
    if (row != nullptr) return row->name;
    const SceneMarker* marker
        = marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(entity)));
    return (marker != nullptr) ? marker->name : std::string();
}

bool GameScriptOrdersHost::argument_id_field_is_nil(int index) {
    // 00b67910 fetching "ID" at 0088a83b, then 00b65fb0 at 0088a84a.
    if (state_ == nullptr) return true;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return true;
    // 00B67910 calls native_lua_get_by_literal_00b67910; OFF keeps lua_getfield.
    // The borrowed owner/view lives only for this lookup. The real positive
    // argument slot and VM come from dispatch; get-by-name produces the
    // tracked result, which is released before the host restores its stack.
    // NativeLuaStateStorage has 50 tracked slots (index 0 is unused here).
    if (lua_gettop(state_) < 49) {
        ScriptOrdersLuaStackView table(state_, slot);
        bsp::NativeLuaObjectStorage result{};
        bsp::native_lua_get_by_literal_00b67910(table.object, result, "ID");
        bsp::native_lua_release_00b67050(result, 0);
        log_.implemented("LuaObject::GetByLiteral", "00b67910");
    } else {
        lua_getfield(state_, slot, "ID");
    }
    const bool nil = lua_type(state_, -1) == LUA_TNIL;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaCommandTarget::argument_id_field", "0088a83b");
    return nil;
}

void* GameScriptOrdersHost::argument_entity(int index) {
    void* entity = entity_from_argument(index);
    log_.implemented("LuaBinding::argument_entity", "00888aa0");
    return entity;
}

bool GameScriptOrdersHost::argument_vector3(int index, float out[3]) {
    // 00888760 at 0088a8a1 reads the value as a Vector3. The shipped scripts
    // pass an entity table to every one of these rows, so this arm is reached
    // only for a target that carries no `ID`.
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return false;
    static const char* const kKeys[3] = {"x", "y", "z"};
    bool complete = true;
    for (int lane = 0; lane < 3; ++lane) {
        lua_getfield(state_, slot, kKeys[lane]);
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            out[lane] = static_cast<float>(lua_tonumber(state_, -1));
        } else {
            complete = false;
        }
        lua_settop(state_, argument_count_);
    }
    log_.implemented("LuaCommandTarget::argument_vector3", "00888760");
    return complete;
}

std::uint16_t GameScriptOrdersHost::entity_object_id(void* entity) {
    // entity+174h, the handle-table ordinal. The two tables at 00f89a0c and
    // 00f89a60 are not built here, so the executable's own entity number stands
    // in for it, which is the same substitution GameCommandUnit already makes.
    log_.unimplemented("LuaBinding::entity_object_id", "0088a88e");
    const std::size_t index = index_of(entity);
    const int id = static_cast<int>(reinterpret_cast<std::uintptr_t>(entity));
    if (index >= units_.count() && marker_for_id(id) == nullptr) return 0;
    return static_cast<std::uint16_t>(index + 1);
}

// 008A4C90 `PilotSetTarget(unit, target [, attackType])`, the order the shipped
// scripts use more than any other - 1125 lines across 197 files - and the only
// `Pilot*` row USN01 calls. docs/PILOT_ORDER_BINDINGS.md has the recovered body.
//
// Goal 5 chain 663: the native runs its body pilot_set_target_008a4c90 over
// GameScriptOrdersHost::PilotOrderBinding (beside run_pilot_set_target); the
// command class is choose_attack_class_007eec50's 007EEC50 answer.
//
// The argument path, which was the open question: the
// native's `00888AA0` reads the Lua table's `Ptr` field and not its `ID`, and
// this host seeds `Ptr` with the entity id (src/game_hosts_lua.cpp) and
// represents an entity as that id cast to a pointer - so the native's own field
// is the correct one to read here, and `entity_from_argument`'s `ID`-as-number
// path is the odd one out.
// 0099A170 BSP_Bot_InstallCommandTask's host, satisfied from what this process
// holds. src/attack_commands.cpp has the routine; this only supplies its reads.
//
// Entities are the id cast to a pointer throughout this host, so a `unit` or
// `target` token here is that same id and `index_of` turns it back.
//
// Two methods answer a fixed refusal rather than a value, and both are on arms
// the chooser does not reach for an aircraft attacking a ship:
// `resolve_return_to_base` needs 007F16D0, which is not reconstructed, and
// `land_group_available` needs the 009F3F50 group gate. They return 0/false so
// the `returntobase` and `land` arms decline rather than inventing an order.
class ScriptOrderAttackCommandHost final : public bsp::AttackCommandHost {
public:
    ScriptOrderAttackCommandHost(GameUnitsHost& units, GameHostLog& log,
        std::uint32_t chosen, std::uint32_t target_id)
        : units_(units), log_(log), chosen_(chosen), target_id_(target_id) {}

    std::uint32_t unit_weapon_director(std::uint32_t unit) override {
        // 0099A18B unit->vtable[114h](). This host has no separate director
        // object; a unit that exists owns one, and the id stands for it.
        return index_for(unit) < units_.count() ? unit : 0u;
    }
    std::uint32_t director_current_command(std::uint32_t director) override {
        // 0071BE40, which GameUnitsHost already answers.
        return units_.director_current_command_0071be40(index_for(director));
    }
    std::uint32_t director_current_params(std::uint32_t director) override {
        return director;  // the params record is reached through the same unit
    }
    std::uint32_t resolve_target(std::uint32_t) override {
        return target_id_;  // 00521EA0's answer for the command just issued
    }
    bool entity_is_kind(std::uint32_t entity, int kind) override {
        return units_.unit_is_kind_of(index_for(entity), kind);
    }
    bool target_still_attackable(std::uint32_t target) override {
        // 009229F0 with kind 6, read from the listing at 0099A2F7 `MOV EDX,6`.
        // Not a temporal test despite the interface name: docs/ATTACK_CAPABILITY_INPUTS.md
        // shows 33 instructions, no +5Dh read, a class test with a LandFort
        // FakedType fallback. FakedType defaults to 1Bh, which is what a fort
        // that authors none carries, and the fallback is unreached for anything
        // that is not a fort.
        const std::size_t i = index_for(target);
        return bsp::entity_kind_or_faked_009229f0(
            i < units_.count(), units_.unit_is_kind_of(i, 0x06),
            units_.unit_is_kind_of(i, 0x1b), 0x1b, 0x06);
    }
    std::uint32_t choose_attack_command(std::uint32_t, std::uint32_t, bool, bool) override {
        return chosen_;  // 007EEC50's answer, computed by the caller
    }
    std::uint32_t resolve_return_to_base(std::uint32_t) override {
        ++refusals_;
        return 0u;  // 007F16D0 unreconstructed
    }
    std::uint32_t create_bot_task(std::uint32_t unit, std::uint32_t command,
                                  std::uint32_t target) override {
        // The thirteen BotTask_Make* factories build a native task object this
        // process does not have. What 0099A170 needs back is a non-zero handle
        // it can install, so the host records the triple and hands out an index.
        tasks_.push_back(BotTask{unit, command, target});
        return static_cast<std::uint32_t>(tasks_.size());
    }
    void install_bot_task(std::uint32_t unit, std::uint32_t task) override {
        installed_unit_ = unit;
        installed_task_ = task;
    }
    bool land_group_available(std::uint32_t) override {
        ++refusals_;
        return false;  // the 009F3F50 group gate is not built
    }

    struct BotTask {
        std::uint32_t unit{0};
        std::uint32_t command{0};
        std::uint32_t target{0};
    };
    const std::vector<BotTask>& tasks() const noexcept { return tasks_; }
    std::uint32_t installed_task() const noexcept { return installed_task_; }
    unsigned refusals() const noexcept { return refusals_; }

private:
    std::size_t index_for(std::uint32_t token) const noexcept {
        return token > 0u ? static_cast<std::size_t>(token - 1u)
                          : ~static_cast<std::size_t>(0);
    }
    GameUnitsHost& units_;
    GameHostLog& log_;
    std::uint32_t chosen_{0};
    std::uint32_t target_id_{0};
    std::vector<BotTask> tasks_;
    std::uint32_t installed_unit_{0};
    std::uint32_t installed_task_{0};
    unsigned refusals_{0};
};

void GameScriptOrdersHost::resolve_plane_squadron_members() {
    const std::size_t count = units_.count();
    if (count == squadron_resolved_units_ && count != 0) return;
    squadron_resolved_units_ = count;
    bsp::game::resolve_plane_squadron_members(units_, &log_);
}

std::size_t resolve_plane_squadron_members(const GameUnitsHost& units,
                                           GameHostLog* log,
                                           bool only_unresolved) {
    const std::size_t count = units.count();
    std::size_t resolved = 0;
    std::size_t missing = 0;
    for (bsp::PlaneSquadronHostRecord& record : bsp::plane_squadron_registry().records()) {
        // Leave a record that already has an answer alone when asked to, so the
        // wipe below can never un-fill the air-ops route's inline slots.
        if (only_unresolved && record.live_count() != 0) continue;
        record.member_units.assign(record.member_names.size(), bsp::kPlaneSquadronNoUnit);
        for (std::size_t slot = 0; slot < record.member_names.size(); ++slot) {
            for (std::size_t unit = 0; unit < count; ++unit) {
                const GameUnitRow* const unit_row = units.unit_row(unit);
                if (unit_row == nullptr || unit_row->name != record.member_names[slot]) {
                    continue;
                }
                record.member_units[slot] = unit;
                break;
            }
            if (record.member_units[slot] == bsp::kPlaneSquadronNoUnit) {
                ++missing;
            } else {
                ++resolved;
            }
        }
        // 007EDA91 reads the leader from slot 0; the squadron entity itself is
        // the fused wing 0, so the two are the same unit here.
        if (record.squadron_unit == bsp::kPlaneSquadronNoUnit) {
            record.squadron_unit = record.flight_leader();
        }
    }
    if (log != nullptr && (resolved != 0 || missing != 0)) {
        log->notef("plane squadron members: %zu of %zu wing record(s) resolved to units "
            "over %zu squadron(s) (007F4B55's +3D0h array, by name)%s",
            resolved, resolved + missing, bsp::plane_squadron_registry().size(),
            missing != 0 ? " - a wing with no unit stays out of +3CCh" : "");
    }
    return resolved;
}

std::uint32_t GameScriptOrdersHost::choose_attack_class_007eec50(std::size_t unit_index,
    std::size_t target_index, bool prefer_ordnance, bool allow_guns, const char* label) {
    // docs/ATTACK_CAPABILITY_INPUTS.md: eight of 007EEC50's eleven feasibility
    // inputs are answerable from an authored class id alone, and this host
    // already holds the machinery - unit_is_kind_of walks the 88 compiled
    // vt[5Ch] bodies, which is the same question the native asks. Report them so
    // the remaining gap is a measured list rather than an assertion.
    const int self_class = units_.unit_class_id(unit_index);
    const int target_class = units_.unit_class_id(target_index);
    // docs/ATTACK_CAPABILITY_INPUTS.md Part 2: the two classifiers behind
    // target_is_air and target_is_surface. Both need the target's live +5Dh
    // byte, which this host does model - unit_flag_005d, and the same byte
    // unit_alive_and_visible reads - so they are answerable here rather than
    // refused, which is what that document's contract assumed a host could not
    // do. The surface walk still reports kUnreadSetBranch if it reaches
    // 008DDF90, which no ship or aircraft target does.
    bsp::EntityTargetFacts tf;
    tf.present = target_index < units_.count();
    if (tf.present) {
        tf.not_engageable = units_.unit_flag_005d(target_index);
        tf.is_plane = units_.unit_is_kind_of(target_index, 0x0f);
        tf.is_plane_squadron = units_.unit_is_kind_of(target_index, 0x18);
        tf.is_ship_family = units_.unit_is_kind_of(target_index, 0x06);
        tf.is_submarine = units_.unit_is_kind_of(target_index, 0x08);
        tf.is_airfield = units_.unit_is_kind_of(target_index, 0x45);
        tf.is_shipyard = units_.unit_is_kind_of(target_index, 0x46);
        tf.is_command_building = units_.unit_is_kind_of(target_index, 0x1c);
        tf.is_dummy_target = units_.unit_is_kind_of(target_index, 0x35);
        tf.is_land_fort = units_.unit_is_kind_of(target_index, 0x1b);
        float r[3], u[3], f[3], t[3];
        if (units_.unit_pose(target_index, r, u, f, t)) tf.world_y = t[1];
    }
    const bool target_air = bsp::entity_mission_seg60_argon::
        entity_is_airborne_target_00922b10(tf) != 0;
    const bsp::SurfaceTargetAnswer target_surface =
        bsp::entity_is_surface_target_00922c80(tf);
    log_.notef("  %s classify: target_is_air=%d target_is_surface=%s "
        "(+5Dh=%d y=%.1f)", label, target_air ? 1 : 0,
        target_surface == bsp::SurfaceTargetAnswer::kYes ? "yes"
            : (target_surface == bsp::SurfaceTargetAnswer::kNo ? "no" : "UNREAD-SET-BRANCH"),
        tf.not_engageable ? 1 : 0, static_cast<double>(tf.world_y));
    log_.notef("  %s caps: self_class=%d level_bomber=%d kamikaze_capable=%d "
        "dogfight_excluded=%d | target_class=%d structure=%d bomb_excluded=%d "
        "submarine=%d ship_family=%d | torpedo first-rack/class inputs bound below",
        label, self_class,
        units_.unit_is_kind_of(unit_index, 0x10) ? 1 : 0,
        units_.unit_is_kind_of(unit_index, 0x17) ? 1 : 0,
        units_.unit_is_kind_of(unit_index, 0x16) ? 1 : 0,
        target_class,
        units_.unit_is_kind_of(target_index, 0x1c) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x0e) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x08) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x06) ? 1 : 0);
    // All eleven of 007EEC50's feasibility inputs, assembled from what this host
    // holds. docs/ATTACK_CAPABILITY_INPUTS.md and docs/ORDNANCE_KIND_IDENTITY.md
    // carry the evidence for each; nothing here is guessed.
    bsp::AttackFeasibilityInputs in;
    in.target_present = tf.present && !tf.not_engageable;
    in.unit_side = units_.unit_side_0054(unit_index);
    in.target_side = tf.present ? units_.unit_side_0054(target_index) : 0;
    in.target_is_structure = units_.unit_is_kind_of(target_index, 0x1c);
    in.target_is_bomb_excluded = units_.unit_is_kind_of(target_index, 0x0e);
    in.target_is_submarine = units_.unit_is_kind_of(target_index, 0x08);
    in.target_is_kamikaze_ship = units_.unit_is_kind_of(target_index, 0x06);
    in.target_is_strafe_fallback = units_.unit_is_kind_of(target_index, 0x41);
    in.self_is_level_bomber = units_.unit_is_kind_of(unit_index, 0x10);
    in.self_is_kamikaze_capable = units_.unit_is_kind_of(unit_index, 0x17);
    in.self_is_dogfight_excluded = units_.unit_is_kind_of(unit_index, 0x16);
    in.target_is_air = target_air;
    in.target_is_surface = target_surface == bsp::SurfaceTargetAnswer::kYes;
    if (target_surface == bsp::SurfaceTargetAnswer::kUnreadSetBranch) {
        // 00922D3F..00922D5B: 008DDF90 over the local slot's set (slot 0 here,
        // LABELLED as in game_hosts_ship_ai.cpp); 008DDFA9 looks up an
        // IsKindOf(5) entity as itself. Otherwise the tail with DL = 1.
        bool in_set = false;
        if (units_.unit_is_kind_of(target_index, 0x05)) {
            const std::vector<std::size_t> set =
                bsp::game::game_objective_sets().units_in_slot(0);
            in_set = std::find(set.begin(), set.end(), target_index) != set.end();
        }
        const bool tail = bsp::entity_surface_target_tail_00922c80(tf, true);
        if constexpr (kChooserSurfaceSetBranchBound) {
            in.target_is_surface = in_set || tail;
        }
        log_.notef("  %s surface set branch: in_set=%d tail=%d land_fort=%d -> %d "
            "(00922C80 -> 008DDF90, packet cc9_l65_chooser_surface_set%s)", label,
            in_set ? 1 : 0, tail ? 1 : 0, tf.is_land_fort ? 1 : 0,
            in.target_is_surface ? 1 : 0, kChooserSurfaceSetBranchBound ? "" : ", OFF");
    }
    // [unit+3D0h] != 0, the squadron's slot-0 plane. This host models a single
    // unit rather than a squadron holding an array, and the "self" queries above
    // already read the ordered unit AS that slot-0 plane - which is the native's
    // own arrangement, since 007ED830 loops +3CCh over +3D0h and every self
    // query runs on element 0. So the occupancy question reduces to whether the
    // ordered unit is a plane. That follows from the identification the self
    // queries already make; it is not a further assumption.
    in.unit_has_weapon_controller = units_.unit_is_kind_of(unit_index, 0x0f);
    {
        // Goal 5 chain 862. The existing chooser uses current rack state and
        // gunnery liveness for +3CCh membership. Keep that projection, including
        // the ordered plane first and its surviving siblings in registry order.
        // bsp.py show 007ED830/007ED970 --asm: each body calls the member's
        // controller and stops on the first hit; no native-shaped copy is made.
        struct TargetOrdnance final : bsp::Cc13SquadronTargetOrdnanceHost {
            GameUnitsHost& units;
            std::vector<std::size_t> members;
            explicit TargetOrdnance(GameUnitsHost& u, std::size_t ordered)
                : units(u), members{ordered} {}
            std::int32_t squadron_member_count(void*) override {
                return static_cast<std::int32_t>(members.size());
            }
            void* squadron_member(void*, std::int32_t i) override {
                return &members[static_cast<std::size_t>(i)];
            }
            std::size_t unit(void* member, std::uint32_t loadout) const {
                // This host owns only the active loadout, as did the replaced
                // unit_ordnance/007B9480 loop. Zero selects that same state.
                if (loadout != 0) throw std::logic_error("unbound chooser loadout");
                return *static_cast<const std::size_t*>(member);
            }
            bsp::OrdnanceKindSet kinds(void* member, std::uint32_t loadout) const {
                return {units.unit_ordnance(unit(member, loadout))};
            }
            bool weapon_controller_has_level_bomb_ordnance_007b9500(
                void* member, std::uint32_t loadout) override {
                return bsp::ordnance_has_paratrooper_31h(kinds(member, loadout));
            }
            bool weapon_controller_has_general_bomb_ordnance_007b9320(
                void* member, std::uint32_t loadout) override {
                return bsp::ordnance_has_general_bomb_2ah(kinds(member, loadout));
            }
            bool weapon_controller_has_drop_kamikaze_ordnance_007b93e0(
                void* member, std::uint32_t loadout) override {
                return bsp::ordnance_has_drop_kamikaze_2fh(kinds(member, loadout));
            }
            bool weapon_controller_has_torpedo_ordnance_007b93f0(
                void* member, std::uint32_t loadout) override {
                return bsp::ordnance_has_torpedo_2bh(kinds(member, loadout));
            }
            bool weapon_controller_has_depth_charge_ordnance_007b94f0(
                void* member, std::uint32_t loadout) override {
                return kinds(member, loadout).contains(0x2c);
            }
            bool weapon_controller_has_rocket_ordnance_007b9480(
                void* member, std::uint32_t loadout) override {
                return units.unit_has_rocket_ordnance_007b9480(unit(member, loadout));
            }
        } ordnance(units_, unit_index);
        const bsp::PlaneSquadronHostRecord* sq =
            bsp::plane_squadron_registry().find_by_member_unit(unit_index);
        auto* const gun = units_.gunnery();
        if (sq != nullptr) {
            for (const std::size_t m : sq->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit || m >= units_.count() || m == unit_index) continue;
                if (gun != nullptr && gun->unit_dead(m)) continue;
                ordnance.members.push_back(m);
            }
        }
        in.has_level_bomb_ordnance = bsp::cc13_seg48_unit_has_level_bomb_ordnance_007ed830(&ordnance, 0, ordnance);
        in.has_general_bomb_ordnance = bsp::cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(&ordnance, 0, ordnance);
        in.has_drop_kamikaze_ordnance = bsp::cc13_seg48_unit_has_drop_kamikaze_ordnance_007ed880(&ordnance, 0, ordnance);
        in.has_torpedo_ordnance = bsp::cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(&ordnance, 0, ordnance);
        if constexpr (bsp::kDepthChargeTaskBound)
            in.has_depth_charge_ordnance = bsp::cc13_seg48_unit_has_depth_charge_ordnance_007eda10(&ordnance, 0, ordnance);
        if constexpr (bsp::kRocketTaskBound)
            in.has_rocket_ordnance = bsp::cc13_seg48_unit_has_rocket_ordnance_007ed970(&ordnance, 0, ordnance);
    }
    // Packet cc9_kamikaze_ship_blocked: 007EEB64..007EEB8A. 00827F70 on the
    // target's class, then 00604A50 on the slot-0 plane (the ordered unit, as
    // the self queries above take it): kind 17h with PilotFires clear.
    if (tf.present && in.self_is_kamikaze_capable && in.target_is_kamikaze_ship) {
        GameShipAiHost* ship_ai = units_.ship_ai();
        const bool big = kShipAiBigLandingShipBound && ship_ai != nullptr &&
            ship_ai->unit_big_landing_ship_0808(target_index);
        const bool small_target = units_.unit_is_kind_of(target_index, 0x0e) ||
            (units_.unit_is_kind_of(target_index, 0x0c) && !big);
        if (small_target) {
            ++kamikaze_small_targets_;
            // 007EEB74 calls 00604A50 on the same slot-0 plane. Goal 5 chain
            // 865: its canonical body consumes the retained kind-17h answer
            // and the live PilotFires byte (00604A58 / 00604A60).
            const bool uncommitted_kamikaze =
                bsp::cc13_sweepB_entity_kind17_ready_00604a50(
                    in.self_is_kamikaze_capable,
                    static_cast<std::uint8_t>(units_.plane_pilot_fires_0c24(unit_index)));
            const bool blocked = !uncommitted_kamikaze;
            if (blocked) ++kamikaze_blocked_;
            log_.notef("  %s kamikaze small-ship test: target class %d small, "
                "00604A50=%d, blocked=%d bound=%d (007EEB74, packet cc9_kamikaze_ship_blocked)",
                label, target_class, uncommitted_kamikaze ? 1 : 0, blocked ? 1 : 0,
                kKamikazeShipBlockedBound ? 1 : 0);
            if (kKamikazeShipBlockedBound) in.kamikaze_ship_blocked = blocked;
        }
    }
    // Packet cc9_strafe_arm: the two guns inputs of the strafe and dogfight arms,
    // read on the slot-0 plane as the self queries above are. 007EEB08/007EEBB7
    // `CMP byte [plane+C24h],0` (PilotFires); 007EEB2C/007EEBC2 0047B850, which
    // answers vt[5Ch](10h) || vt[5Ch](16h). OFF: both stay false and 007EEC50's
    // guns answer is always 0 (docs/SQUADRON_LAND_TASK.md 5bw.1).
    if constexpr (bsp::kAttackChoiceGunsFedBound) {
        in.guns_available = units_.plane_pilot_fires_0c24(unit_index);
        in.guns_suppressed = in.self_is_level_bomber || in.self_is_dogfight_excluded;
        log_.notef("  %s guns feed: pilot_fires=%d suppressed=%d "
            "(007EEB08/007EEB2C, packet cc9_strafe_arm)",
            label, in.guns_available ? 1 : 0, in.guns_suppressed ? 1 : 0);
    }
    if (in.has_torpedo_ordnance) {
        // 007EEA5B / 007B9230: the CURRENT leader's first +974h rack, not the
        // first torpedo found among the squadron's other members or guns.
        std::size_t leader = unit_index;
        const auto* const squadron = bsp::plane_squadron_registry().find_by_member_unit(unit_index);
        if (squadron != nullptr) {
            leader = squadron->flight_leader();
        }
        const int bullet_id = units_.plane_first_rack_bullet_id(leader);
        const bsp::WeaponClassDescriptor* descriptor = nullptr;
        if (bullet_id >= 0) {
            auto* const gunnery = units_.gunnery();
            if (gunnery == nullptr || (descriptor = gunnery->weapon_class_descriptor(bullet_id)) == nullptr)
                throw std::logic_error("loaded first rack has no retained projectile descriptor");
        }
        // 00828EC0 reads the actual target class: kind 0Eh, or kind 0Ch with
        // +808h clear, then +500h >= 00D099AC. The existing class readers own
        // the boolean and converted speed, so do not substitute unit velocity.
        const bool landing_ship = tf.present && units_.unit_is_kind_of(target_index, 0x0c);
        const auto* const ship_ai = units_.ship_ai();
        if (landing_ship && ship_ai == nullptr)
            throw std::logic_error("landing-ship torpedo gate has no class flag owner");
        bsp::bind_attack_torpedo_inputs(in, descriptor,
            tf.present && units_.unit_is_kind_of(target_index, 0x0e), landing_ship,
            landing_ship && ship_ai->unit_big_landing_ship_0808(target_index),
            tf.present ? units_.unit_class_max_speed_0500(target_index) : 0.0f);
    }
    const std::uint32_t chosen =
        bsp::attack_command_choose(in, prefer_ordnance, allow_guns);
    log_.notef("  %s choose: 007EEC50 -> %08lx  (weapon_controller=%d "
        "ordnance lb=%d gb=%d dk=%d torp=%d, sides %d/%d)",
        label, static_cast<unsigned long>(chosen),
        in.unit_has_weapon_controller ? 1 : 0,
        in.has_level_bomb_ordnance ? 1 : 0, in.has_general_bomb_ordnance ? 1 : 0,
        in.has_drop_kamikaze_ordnance ? 1 : 0, in.has_torpedo_ordnance ? 1 : 0,
        in.unit_side, in.target_side);
    if constexpr (bsp::kRocketTaskBound) {
        if (in.has_rocket_ordnance) {
            log_.notef("  %s rocket feed: 007ED970 has_rocket_ordnance=1 surface=%d "
                "(packet cc9-rocket1)", label, in.target_is_surface ? 1 : 0);
        }
    }
    return chosen;
}

// Packet cc9_ai_squadron_settarget_intake (docs/SQUADRON_LAND_TASK.md 5ck). The
// squadron's vtable[160h] 007F1940 (slot 00D08920 of 00D087C0), reached from
// the MT_COMMAND delivery as every entity's intake is. Its arm for `settarget`
// (00E08EF8) and `attackmove` (00E08F78), 007F1AD6-007F1B24, read from the disk
// bytes: 00521EA0 resolves the message descriptor's object, then 007EEC50
// (ECX = the squadron, target, 1, 1). This arm skips the +3D0h test the other
// arms make (007F1AE4). With a non-null class and +21h = 1 (00A13B60 pushes 1
// at 00A14A4A) it clears (0071D880) and issues the class with the same
// descriptor (0071ECF0); a null class issues nothing and leaves the squadron's
// command as it was. The +5Dh player-control early-out (007F194A) is not
// modelled: an AI group's squadron is not player-controlled.
// SUBSTITUTIONS, labelled: the chooser's self queries run on the slot-0 plane
// (as PilotSetTarget's do); the squadron's director is stood in for by each
// member plane's, so the class is issued to every member and each member's
// 0099A170 install runs at that delivery, one bot tick early (22.7), as the
// PilotSetTarget fan-out does.
std::uint32_t GameScriptOrdersHost::squadron_intake_007f1940(std::size_t leader,
    const std::vector<std::size_t>& members, std::size_t target_index) {
    ++squadron_intake_calls_;
    // 007F1B14-007F1B24 PUSH 1 / PUSH 1 / 00521EA0 / PUSH EAX / 007EEC50.
    const std::uint32_t chosen = choose_attack_class_007eec50(leader, target_index,
        true, true, "SquadronIntake 007F1940");
    if (chosen == 0u || target_index >= units_.count()) {
        ++squadron_intake_declined_;   // 007F1B2D JE 007F1B5D: nothing issued
        return 0u;
    }
    issue_command_to_members(members, chosen, target_index, "ai:close_attack 007F1940");
    return chosen;
}

void GameScriptOrdersHost::issue_command_to_members(const std::vector<std::size_t>& members,
    std::uint32_t chosen, std::size_t target_index, const char* source) {
    // 00A14A08-00A14A5D: the kind-1 descriptor naming the target, its +174h id.
    bsp::SceneCommandTarget target{};
    target.kind = 1;
    target.object_id = static_cast<std::uint16_t>(target_index + 1u);
    target.object = reinterpret_cast<void*>(static_cast<std::uintptr_t>(target_index + 1u));
    const std::uint32_t target_token = static_cast<std::uint32_t>(target_index + 1u);
    const std::string saved_source = delivery_source_;
    delivery_source_ = source;
    for (const std::size_t member : members) {
        if (member >= units_.count()) continue;
        void* const handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(member + 1u));
        entity_issue_command(handle, chosen, target, 1);
        ++squadron_intake_member_orders_;
        std::function<void()> install = [this, member, chosen, target_token]() {
            ScriptOrderAttackCommandHost bots(units_, log_, chosen, target_token);
            const std::uint32_t task = bsp::bot_install_command_task_0099a170(
                static_cast<std::uint32_t>(member + 1u), bots);
            if (task != 0u) {
                units_.store_unit_attack_command_class(member, chosen);
                ++squadron_intake_tasks_;
            }
        };
        if (!commands_after_last_issue_delivery(install)) install();
    }
    delivery_source_ = saved_source;
}

// Packet p15_free_attack_tick: the bot side of an override 0071D980 sent or
// cleared. 0099A170 reads the member's current command (0071BE40, the override
// in mode 2); `command` and `target_index` are what the caller knows that read
// answers, as the PilotSetTarget fan-out passes them.
std::uint32_t GameScriptOrdersHost::install_command_task_0099a170(std::size_t member,
    std::uint32_t command, std::size_t target_index) {
    if (member >= units_.count()) return 0u;
    const std::uint32_t target_token = target_index < units_.count()
        ? static_cast<std::uint32_t>(target_index + 1u) : 0u;
    ScriptOrderAttackCommandHost bots(units_, log_, command, target_token);
    const std::uint32_t task = bsp::bot_install_command_task_0099a170(
        static_cast<std::uint32_t>(member + 1u), bots);
    if (task != 0u) units_.store_unit_attack_command_class(member, command);
    return task;
}

bool script_orders_squadron_permissions_3c3d(const bsp::PlaneSquadronHostRecord& squadron,
    bool& allow_fire, bool& allow_move) {
    allow_fire = false;
    allow_move = false;
    return g_live_script_orders != nullptr
        && g_live_script_orders->squadron_permissions_3c3d(squadron, allow_fire, allow_move);
}

std::uint32_t script_orders_choose_attack_class_007eec50(std::size_t leader,
    std::size_t target_index, bool prefer_ordnance, bool allow_guns) {
    return g_live_script_orders != nullptr
        ? g_live_script_orders->choose_attack_class_for_free_attack(leader, target_index,
              prefer_ordnance, allow_guns)
        : 0u;
}

std::uint32_t script_orders_install_command_task_0099a170(std::size_t member,
    std::uint32_t command, std::size_t target_index) {
    return g_live_script_orders != nullptr
        ? g_live_script_orders->install_command_task_0099a170(member, command, target_index)
        : 0u;
}

std::uint32_t script_orders_squadron_intake_007f1940(std::size_t leader,
    const std::vector<std::size_t>& members, std::size_t target_index) {
    return g_live_script_orders != nullptr
        ? g_live_script_orders->squadron_intake_007f1940(leader, members, target_index)
        : 0u;
}

// ---------------------------------------------------------------------------
// Packet cc9_player_air_ops_launch (bsp::kAirOpsPlayerLaunchBound). The Support
// Manager's targeted launch, 00673A10 (00674765-006748D7, reached from 00675C40 at
// 0067A642), for the harness's `launch` line. docs/AIROPS_LAUNCH_TICK.md, "The player's launch".
// ---------------------------------------------------------------------------
namespace {
// class+134h DefaultEquipment, which 006C0F00 copies into slot+10h when the
// class changes. The mission Lua host, which holds the class table, sets it.
int (*g_class_default_arm_reader)(int vehicle_class) = nullptr;
// kPlayerLaunchScreenCursorBound retired: the cursor key's state-5 path (0067A565) is not the targeted launch (docs/SHIP_AI_OPEN_ITEMS.md 212).

// Goal 5 chain 636. The support manager's slot order (00673A10 at 006748C0 /
// 0067491B, 00675C40 at 0067A5D1 / 0067A945): 00656280 builds 82h {slot,
// order, target+174h, game+18ECh} and 0077C2A0(msg, 0, 0) routes it to the
// block's owner; the local copy reaches 006CD6C0's 82h arm 006CD160, which
// hands 006CCDA0 the copy's slot, order, target and player. The values below
// are the copy's. +174h is this process's unit index + 1 (entity_object_id);
// LABELLED: a target whose id the copy's 12 bits cannot carry keeps the posted
// one, as queue_gun_aim_79 keeps its target id. LABELLED, as before: the copy
// is delivered in this step, where 0076E520 queues it for the session's
// receive drain. A standalone host without a live commands queue keeps the
// posted values, as the 89h serve does.
struct SlotOrderDelivery {
    bool delivered{false};
    int slot{-1};
    std::uint32_t order{0};
    std::uint32_t target_plus_one{0};
    std::uint32_t player{0};
};
SlotOrderDelivery deliver_air_ops_slot_order_82(int slot, std::uint32_t order,
    std::uint32_t target_plus_one) {
    SlotOrderDelivery out;
    out.slot = slot;
    out.order = order;
    out.target_plus_one = target_plus_one;
    const bsp::HostSessionMessages* const messages = commands_session_messages();
    if (messages == nullptr) {
        out.delivered = true;
        return out;
    }
    const bsp::HostSessionMessages::AirbaseSlotOrderCopy copy =
        messages->queue_airbase_slot_order_82(static_cast<std::uint32_t>(slot), order,
            static_cast<std::uint16_t>(target_plus_one));
    if (!copy.to_unit) return out;
    out.delivered = true;
    out.slot = static_cast<int>(copy.slot);
    out.order = copy.order;
    if (target_plus_one <= 0xfffu) out.target_plus_one = copy.target_id;
    out.player = copy.player;
    return out;
}
}  // namespace

// Packet p20_squadron_helm (docs/OCEAN_WAVE_FIELD.md section 79). The player's
// moveto 005F9B20 orders [00E188D8] through 0077D600 with 00E08F68, the class
// kPilotOrderClassMoveTo; on a squadron the command lands on the squadron's
// director and each member's bot (0099A4C0 on the change, then 0099A170) builds
// a kind-7 moveto task on a position descriptor. The host's member command rows
// and the install at delivery are the standing fan-out substitution, as in the
// air-ops order's moveto above (006CCDA0 kMoveToSelf). LABELLED: no 00645060
// select or 0064B870 / 00780214 role transfer runs; the harness's order stands
// for a player who selected the squadron.
int GameScriptOrdersHost::player_plane_moveto(std::size_t unit_index, float x, float z) {
    if (unit_index >= units_.count()) return -1;
    const bsp::PlaneSquadronHostRecord* record =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (record == nullptr) return -1;
    float lx = 0.0f, ly = 0.0f, lz = 0.0f;
    units_.unit_position_00fc(unit_index, lx, ly, lz);
    bsp::SceneCommandTarget here;
    here.kind = 0;
    here.position_valid = 1;
    here.position[0] = x;
    here.position[1] = ly;
    here.position[2] = z;
    const std::string saved_source = delivery_source_;
    delivery_source_ = "player:helm moveto 005F9B20";
    int planes = 0;
    for (const std::size_t m : record->member_units) {
        if (m == bsp::kPlaneSquadronNoUnit || m >= units_.count()) continue;
        if (!units_.unit_alive_and_visible(m)) continue;
        void* const handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(m + 1u));
        entity_issue_command(handle, bsp::kPilotOrderClassMoveTo, here, 1);
        ++planes;
        after_order_delivery([this, m, here]() {
            ScriptOrderAttackCommandHost bot_host(units_, log_, bsp::kPilotOrderClassMoveTo, 0u);
            const std::uint32_t task = bsp::bot_install_command_task_0099a170(
                static_cast<std::uint32_t>(m + 1u), bot_host);
            if (task == 0u) return;
            units_.store_unit_attack_command_class(m, bsp::kPilotOrderClassMoveTo);
            units_.store_unit_moveto_range(m, 0.0f);
            units_.store_unit_moveto_target(m, ~static_cast<std::size_t>(0));
            units_.store_unit_moveto_point(m, here.position);
        });
    }
    delivery_source_ = saved_source;
    return planes;
}

std::size_t harness_target_row(const GameUnitsHost& units, const std::string& name) {
    std::size_t first = units.count();
    for (std::size_t k = 0; k < units.count(); ++k) {
        const GameUnitRow* row = units.unit_row(k);
        if (row == nullptr || row->name != name) continue;
        if (first == units.count()) first = k;
        if constexpr (!kHarnessTargetPrefersLiveBound) break;
        if (units.unit_alive_and_visible(k)) return k;
    }
    return first;
}

void script_orders_set_class_default_arm_reader(int (*reader)(int vehicle_class)) {
    g_class_default_arm_reader = reader;
}

PlayerAirOpsLaunchResult GameScriptOrdersHost::player_air_ops_launch(const std::string& base,
    int slot_number, int vehicle_class, int count, const std::string& target, int loadout) {
    PlayerAirOpsLaunchResult out;
    ++player_launch_requests_;
    auto refuse = [&](const char* why) {
        out.reason = why;
        log_.notef("player air ops launch REFUSED: base=\"%s\" slot=%d class=%d count=%d "
            "target=\"%s\": %s (packet cc9_player_air_ops_launch)", base.c_str(), slot_number,
            vehicle_class, count, target.c_str(), why);
        return out;
    };
    if constexpr (!bsp::kAirOpsPlayerLaunchBound) return refuse("kAirOpsPlayerLaunchBound is off");
    bsp::AirOpsDeckRegistry& registry = bsp::air_ops_decks();
    bsp::AirOpsDeck* deck = nullptr;
    for (std::size_t i = 0; i < registry.size(); ++i) {
        if (registry.name_at(i) == base) { deck = registry.mutable_at(i); break; }
    }
    if (deck == nullptr) return refuse("no air-ops deck for that base");
    const std::size_t target_index = harness_target_row(units_, target);
    // 006CCDD3: order 3 with no target answers 0 and does nothing.
    if (target_index >= units_.count()) return refuse("no unit by the target name");
    int slot = slot_number - 1;
    if (slot_number <= 0) {
        // 00673A10 at 00674765: the first slot in state 1 or 5, the same walk as
        // 006C7210 without its growth (none: slots.size(), refused below as at 006747E7).
        slot = bsp::air_ops_pick_launch_slot_006c7210(*deck);
    }
    if (slot < 0 || static_cast<std::size_t>(slot) >= deck->slots.size())
        return refuse("no such slot");
    const bsp::AirOpsSlotState state = deck->slots[static_cast<std::size_t>(slot)].state;
    // 00673A10 at 00674785-006747C2: the targeted launch takes only a slot in state 1 or 5.
    if (state != bsp::AirOpsSlotState::kCooldown && state != bsp::AirOpsSlotState::kReady)
        return refuse("the slot is not in state 1 or 5");
    std::vector<std::int32_t> live(deck->slots.size(), 0);
    for (std::size_t i = 0; i < deck->slots.size(); ++i)
        live[i] = air_ops_squadron_plane_count(deck->slots[i].launched_squadron);
    const int arm = g_class_default_arm_reader != nullptr
        ? g_class_default_arm_reader(vehicle_class) : 0;
    // Packet cc9_player_launch_group: 00674801..0067482B hands 006C0F00
    // min(stock, screen+2B4h), and screen+2B4h is the constant 3 (0066EC0A). The
    // stock clamp is 006C0F00's own; this applies the screen's.
    // Packet cc13-fix7 D1: 0067480E 006BF310(class) is 006BF230's total; a total
    // of 0 or less leaves at 0067481B (JLE 00674B46) before 006C0F00 writes the
    // slot. Reachable once 006CA640 consumes the stock (kAirOpsQueueConsumeStockBound).
    if constexpr (bsp::kAirOpsQueueConsumeStockBound) {
        if (bsp::air_ops_stock_available_006bf230(*deck,
                static_cast<std::uint32_t>(vehicle_class), live.data()).total <= 0)
            return refuse("no stock of the class (0067480E 006BF310, 0067481B)");
    }
    int screen_count = count;
    if constexpr (bsp::kAirOpsPlayerLaunchGroupBound) {
        if (screen_count > bsp::kSupportManagerLaunchGroup) {
            log_.notef("player air ops launch: count %d clamped to the screen's %d "
                "(screen+2B4h, 0066EC0A / 0067A592, packet cc9_player_launch_group)",
                screen_count, bsp::kSupportManagerLaunchGroup);
            screen_count = bsp::kSupportManagerLaunchGroup;
        }
    }
    // 00674848 -> 006C4780 -> 006C0F00.
    out.count = bsp::air_ops_fill_slot_006c0f00(*deck, slot,
        static_cast<std::uint32_t>(vehicle_class), screen_count, arm, live.data());
    if (out.count <= 0) return refuse("006C0F00 left the slot empty (no stock or plane room)");
    // 0067484D-0067485D: 006BA4B0(slot, screen+334h) stores the loadout ring's choice
    // at slot+10h, which 006C7490 hands the squadron as its Equipment. The screen's
    // value defaults to the class's +134h (0065C9A0), what 006C0F00 stored above, so
    // only a line that names a loadout changes anything. LABELLED harness input: the
    // ring (00665F30 / 00662020) is not run; the line stands for the player's pick,
    // and its index is not checked against the class's Equipments count.
    if (loadout >= 0) {
        deck->slots[static_cast<std::size_t>(slot)].class_field_134 = loadout;
        log_.notef("player air ops launch: base=\"%s\" slot=%d loadout %d (screen+334h -> "
            "slot+10h, 0067485D 006BA4B0; harness input)", base.c_str(), slot + 1, loadout);
    }
    // 006748C0 00656280: 82h {slot, order 3, the target's +174h, game+18ECh};
    // 006748D7 routes it, and 006CD6C0 case 1 -> 006CD160 -> 006CCDA0(apply 1)
    // on the local copy (HostSessionMessages::queue_airbase_slot_order_82).
    const SlotOrderDelivery delivery = deliver_air_ops_slot_order_82(slot, 3,
        static_cast<std::uint32_t>(target_index + 1u));
    if (!delivery.delivered) return refuse("the 82h copy did not reach the block's session-kind switch");
    if (delivery.slot < 0 || static_cast<std::size_t>(delivery.slot) >= deck->slots.size())
        return refuse("the 82h copy names no slot of this block");
    const int answer = bsp::air_ops_slot_command_006ccda0(*deck, delivery.slot,
        static_cast<int>(delivery.order), delivery.target_plus_one,
        static_cast<int>(delivery.player), true);
    if (answer != 1) return refuse("006CCDA0 refused the attack order");
    out.accepted = true;
    out.slot = delivery.slot;
    ++player_launch_accepted_;
    log_.notef("player air ops launch: base=\"%s\" slot=%d class=%d count=%d->%d target=\"%s\" "
        "queued (0067A5A7 006C0F00, 0067A5E6 82h -> 006CCDA0 order 3 -> 006CA640, packet "
        "cc9_player_air_ops_launch)", base.c_str(), slot + 1, vehicle_class, count, out.count,
        target.c_str());
    return out;
}

// Packet cc9_air_ops_held_slot_orders. 006CCDA0's state-3/4 arms for the player's
// order to a held slot; the command is issued on the squadron (0077D600 with ECX =
// slot+28h). SUBSTITUTIONS, labelled:
//  - the 82h message takes its local copy (deliver_air_ops_slot_order_82) and is
//    delivered in the same step;
//  - 006BC5E0's "holding at the moveto point" is the flight leader's stored command
//    class being moveto; its distance test (class+268h x [00CEC160]) is not read,
//    which only matters for a moveto whose point is not the squadron's own;
//  - the squadron's command reaches its members the host's way: attack through the
//    squadron intake 007F1940, moveto issued on each member (0077D600) at the
//    leader's position and its kind-7 task (0099A170) installed after delivery,
//    land through 0099A3DD at the deck owner;
//  - 0071BED0's hold fire on the recall is not carried.
PlayerAirOpsOrderResult GameScriptOrdersHost::player_air_ops_order(const std::string& base,
    int slot_number, int order, const std::string& target) {
    PlayerAirOpsOrderResult out;
    auto refuse = [&](const char* why) {
        out.reason = why;
        log_.notef("player air ops order REFUSED: base=\"%s\" slot=%d order=%d target=\"%s\": %s "
            "(packet cc9_air_ops_held_slot_orders)", base.c_str(), slot_number, order,
            target.c_str(), why);
        return out;
    };
    if constexpr (!bsp::kAirOpsHeldSlotOrdersBound) return refuse("kAirOpsHeldSlotOrdersBound is off");
    bsp::AirOpsDeckRegistry& registry = bsp::air_ops_decks();
    bsp::AirOpsDeck* deck = nullptr;
    for (std::size_t i = 0; i < registry.size(); ++i) {
        if (registry.name_at(i) == base) { deck = registry.mutable_at(i); break; }
    }
    if (deck == nullptr) return refuse("no air-ops deck for that base");
    const int slot = slot_number - 1;
    if (slot < 0 || static_cast<std::size_t>(slot) >= deck->slots.size()) return refuse("no such slot");
    bsp::AirOpsSlot& s = deck->slots[static_cast<std::size_t>(slot)];
    std::size_t target_index = units_.count();
    if (!target.empty()) {
        target_index = harness_target_row(units_, target);
        if (target_index >= units_.count()) return refuse("no unit by the target name");
        // LABELLED (cc9-lua46): the screen offers only targets that pass the marker
        // gate 0043F080 (006431A8), as the launch line's harness gate; a file can then
        // list fallback targets for one slot and the first live one is taken.
        if (order == 3 && !units_.unit_alive_and_visible(target_index))
            return refuse("the target fails 0043F080 (dead or hidden); the screen does not offer it");
    }
    const bsp::PlaneSquadronHostRecord* record = nullptr;
    for (const AirOpsSquadron& made : squadrons_) {
        if (made.entity_id == s.launched_squadron && s.launched_squadron != 0u) {
            record = bsp::plane_squadron_registry().find(made.name);
            break;
        }
    }
    if constexpr (kAirOpsSceneSquadronOrdersBound) {
        if (record == nullptr && s.launched_squadron != 0u
            && static_cast<std::size_t>(s.launched_squadron - 1u) < units_.count()) {
            const GameUnitRow* row = units_.unit_row(s.launched_squadron - 1u);
            if (row != nullptr) record = bsp::plane_squadron_registry().find(row->name);
            if (record != nullptr) {
                log_.notef("player air ops order: base=\"%s\" slot=%d holds scene squadron %s "
                    "(slot+28h = %u, 006C56D0; packet cc9_lua47_scene_squadron_orders)",
                    base.c_str(), slot_number, row->name.c_str(), s.launched_squadron);
            }
        }
    } else if (record == nullptr && s.launched_squadron != 0u
               && static_cast<std::size_t>(s.launched_squadron - 1u) < units_.count()) {
        // Packet cc9-sweep2b reach: the order the ON arm would take, logged only.
        const GameUnitRow* row = units_.unit_row(s.launched_squadron - 1u);
        if (row != nullptr && bsp::plane_squadron_registry().find(row->name) != nullptr) {
            log_.notef("player air ops order reach (kAirOpsSceneSquadronOrdersBound OFF): "
                "base=\"%s\" slot=%d holds scene squadron %s, refused", base.c_str(),
                slot_number, row->name.c_str());
        }
    }
    if (record == nullptr) return refuse("the slot holds no squadron this host built");
    const std::size_t leader = record->flight_leader();
    // 007EE5C0: members not landed in state 5 or 2 (or 1 while disabled).
    std::int32_t unstowed = 0;
    std::vector<std::size_t> members;
    for (const std::size_t m : record->member_units) {
        if (m == bsp::kPlaneSquadronNoUnit) continue;
        int st = 0;
        bool landed = false, enabled = false;
        unsigned int cls = 0;
        if (!units_.plane_order_view(m, st, landed, enabled, cls)) continue;
        members.push_back(m);
        const bool stowed = landed && (st == 5 || st == 2 || (!enabled && st == 1));
        if (!stowed) ++unstowed;
    }
    bool holding = false;
    {
        int st = 0;
        bool landed = false, enabled = false;
        unsigned int cls = 0;
        if (leader < units_.count() && units_.plane_order_view(leader, st, landed, enabled, cls))
            holding = cls == bsp::kPilotOrderClassMoveTo;
    }
    const std::uint32_t posted_target_plus_one = target_index < units_.count()
        ? static_cast<std::uint32_t>(target_index + 1u) : 0u;
    // Goal 5 chain 636: the order travels as 82h's local copy into 006CD160,
    // which passes 006CCDA0 the copy's slot, order and target (the held-slot
    // arms here read no player).
    const SlotOrderDelivery delivery = deliver_air_ops_slot_order_82(slot,
        static_cast<std::uint32_t>(order), posted_target_plus_one);
    if (!delivery.delivered) return refuse("the 82h copy did not reach the block's session-kind switch");
    if (delivery.slot != slot) return refuse("the 82h copy names another slot");
    const std::uint32_t target_plus_one = delivery.target_plus_one;
    const bsp::AirOpsHeldSlotOrder r = bsp::air_ops_held_slot_order_006ccda0(*deck, slot,
        static_cast<int>(delivery.order), target_plus_one, holding, unstowed, true);
    out.answer = r.answer;
    out.state_after = static_cast<int>(s.state);
    if (r.answer == -1) return refuse("the slot is not in state 3 or 4 (the launch entry serves 1 and 5)");
    if (r.answer != 1) {
        return refuse(r.answer == 2 ? "006CCDA0 refused the order in this slot state"
                                    : "006CCDA0 refused the order (no class, or no target)");
    }
    if (r.action == bsp::AirOpsHeldSlotAction::kAttack && leader < units_.count()) {
        if (script_orders_squadron_intake_007f1940(leader, members, target_index) != 0u)
            out.planes = static_cast<int>(members.size());
    } else if (r.action == bsp::AirOpsHeldSlotAction::kMoveToSelf && leader < units_.count()) {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units_.unit_position_00fc(leader, x, y, z);                   // 00427EB0
        // 00468560's position descriptor (kind 0) at the squadron's own position.
        // cc9-lua46: the moveto is issued on each member first (0077D600), so
        // 0099A170 reads it as the member's current command (0071BE40); before, the
        // install read the member's old command and a squadron whose torpedo
        // target was dead took no task (ESMP08 i1: 8 of 23 order-1s, 0 planes).
        bsp::SceneCommandTarget here;
        here.kind = 0;
        here.position_valid = 1;
        here.position[0] = x;
        here.position[1] = y;
        here.position[2] = z;
        const std::string saved_source = delivery_source_;
        delivery_source_ = "player:air ops order 006CCDA0";
        // `planes` counts the members the moveto is issued to: the install runs
        // after the order's delivery, usually after this entry returns.
        for (const std::size_t m : members) {
            void* const handle = reinterpret_cast<void*>(static_cast<std::uintptr_t>(m + 1u));
            entity_issue_command(handle, bsp::kPilotOrderClassMoveTo, here, 1);
            ++out.planes;
            after_order_delivery([this, m, here]() {
                ScriptOrderAttackCommandHost bot_host(units_, log_, bsp::kPilotOrderClassMoveTo, 0u);
                const std::uint32_t task = bsp::bot_install_command_task_0099a170(
                    static_cast<std::uint32_t>(m + 1u), bot_host);
                if (task == 0u) return;
                units_.store_unit_attack_command_class(m, bsp::kPilotOrderClassMoveTo);
                units_.store_unit_moveto_range(m, 0.0f);
                units_.store_unit_moveto_target(m, ~static_cast<std::size_t>(0));
                units_.store_unit_moveto_point(m, here.position);
            });
        }
        delivery_source_ = saved_source;
    } else if (r.action == bsp::AirOpsHeldSlotAction::kRecallLand) {
        std::size_t owner = units_.count();
        for (std::size_t k = 0; k < units_.count(); ++k) {
            const GameUnitRow* row = units_.unit_row(k);
            if (row != nullptr && row->name == base) { owner = k; break; }
        }
        if (owner < units_.count()) {
            for (const std::size_t m : members) {
                if (units_.land_at_site_0099a3dd(m, owner) != 0u) ++out.planes;
            }
        }
    }
    out.accepted = true;
    out.state_after = static_cast<int>(s.state);
    log_.notef("player air ops order: base=\"%s\" slot=%d order=%d target=\"%s\" squadron=%s "
        "holding=%d unstowed=%d -> state %d, %d plane(s) (0067A5E6 82h -> 006CCDA0, packet "
        "cc9_air_ops_held_slot_orders)", base.c_str(), slot_number, order, target.c_str(),
        record->name.c_str(), holding ? 1 : 0, unstowed, out.state_after, out.planes);
    return out;
}

PlayerAirOpsOrderResult script_orders_player_air_ops_order(const std::string& base,
    int slot_number, int order, const std::string& target) {
    if (g_live_script_orders == nullptr) {
        PlayerAirOpsOrderResult out;
        out.reason = "no live script-orders host";
        return out;
    }
    return g_live_script_orders->player_air_ops_order(base, slot_number, order, target);
}

void GameScriptOrdersHost::queue_auto_attack_target_007f15f0(std::uint32_t squadron_entity,
                                                             std::uint32_t target_plus_one) {
    PendingAutoAttack p;
    p.squadron_entity = squadron_entity;
    p.target_plus_one = target_plus_one;
    pending_auto_attacks_.push_back(p);
}

// 006CDC70's 006C64B0 for every deck, then the AutoAttackTarget the squadrons
// launched from an ordered slot carry. 007F4BA0 reads the bag key at 007F4EC0 and
// calls 007F15F0(00521E30(id)): a plane target is replaced by its squadron
// (+9D4h), 007EEC50(target, 1, 1) chooses the class and 0071ECF0 issues it, a
// null class issuing moveto instead. SUBSTITUTIONS, labelled: the host serves it
// through the squadron intake 007F1940 (the same 007EEC50 call; a declined class
// issues nothing rather than moveto) at the first step the squadron's members
// exist, where the image runs it in the squadron's own pass-C init.
void GameScriptOrdersHost::run_air_ops_player_launch_queue(float step) {
    static_cast<void>(step);
    bsp::AirOpsDeckRegistry& registry = bsp::air_ops_decks();
    for (std::size_t d = 0; d < registry.size(); ++d) {
        bsp::AirOpsDeck* deck = registry.mutable_at(d);
        if (deck == nullptr) continue;
        int slot = bsp::air_ops_queued_slot_wait_006c64b0(*deck);
        if (slot < 0) continue;
        // 006C6502: 006BC8E0 routes 89h through 0076E520's local copy, and
        // 006CD6C0 case 8 runs 006C7490(copy slot, 0). A standalone host
        // without a live commands queue keeps the served slot.
        if (const bsp::HostSessionMessages* messages = commands_session_messages()) {
            const std::uint32_t copy =
                messages->queue_airbase_do_launch_slot_89(static_cast<std::uint32_t>(slot));
            if (copy >= deck->slots.size()) {
                // LABELLED: the image's receiver indexes the copy's slot unchecked.
                log_.unimplemented("AirOps::do_launch_slot_copy_out_of_range", "006c7490");
                continue;
            }
            slot = static_cast<int>(copy);
        }
        bsp::air_ops_launch_start_006c7490(*deck, slot);
        ++player_launch_started_;
        log_.notef("air ops queued launch: base=\"%s\" slot=%d squadron=%u target=%u "
            "(006C64B0 -> 89h -> 006C7490, packet cc9_player_air_ops_launch)",
            registry.name_at(d).c_str(), slot + 1,
            deck->slots[static_cast<std::size_t>(slot)].launched_squadron,
            deck->slots[static_cast<std::size_t>(slot)].order_target_4c);
    }
    for (std::size_t i = 0; i < pending_auto_attacks_.size();) {
        PendingAutoAttack& p = pending_auto_attacks_[i];
        const bsp::PlaneSquadronHostRecord* record = nullptr;
        for (const AirOpsSquadron& made : squadrons_) {
            if (made.entity_id == p.squadron_entity) {
                record = bsp::plane_squadron_registry().find(made.name);
                break;
            }
        }
        std::vector<std::size_t> members;
        bool slot_waiting = false;
        if (record != nullptr) {
            for (const std::size_t m : record->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit) slot_waiting = true;
                if (m != bsp::kPlaneSquadronNoUnit && m < units_.count() &&
                    units_.unit_alive_and_visible(m)) {
                    members.push_back(m);
                }
            }
        }
        // Packet cc9_player_launch_group (kAutoAttackAllMembersBound): the image
        // reads the key in pass-C init 007F4BA0, after 007F4580 has made every
        // wing, so the order waits while a registered slot still awaits its
        // pass-A plane (kWingConstructionInPassABound). Same 600-step cap.
        if constexpr (bsp::kAutoAttackAllMembersBound) {
            if (slot_waiting && p.waited_steps + 1 < 600) {
                ++p.waited_steps;
                ++i;
                continue;
            }
        }
        if (members.empty() && ++p.waited_steps < 600) { ++i; continue; }
        std::size_t target = p.target_plus_one - 1u;
        if (target < units_.count()) {
            // 007F15F7-007F1606: a plane target stands for its squadron; the host
            // fuses the squadron into its flight leader.
            if (const bsp::PlaneSquadronHostRecord* t =
                    bsp::plane_squadron_registry().find_by_member_unit(target)) {
                const std::size_t lead = t->flight_leader();
                if (lead != bsp::kPlaneSquadronNoUnit && lead < units_.count()) target = lead;
            }
        }
        std::uint32_t chosen = 0u;
        if (!members.empty() && target < units_.count())
            chosen = squadron_intake_007f1940(members.front(), members, target);
        if (chosen != 0u) ++auto_attack_issued_; else ++auto_attack_declined_;
        const GameUnitRow* trow = target < units_.count() ? units_.unit_row(target) : nullptr;
        if constexpr (bsp::kAutoAttackNullClassMoveToBound) {
            // 007F1688-007F16AD: no class, so moveto (00E08F68) on the same descriptor.
            if (chosen == 0u && !members.empty() && target < units_.count()) {
                issue_command_to_members(members, 0x00E08F68u, target,
                                         "ai:auto_attack moveto 007F1688");
                ++auto_attack_null_class_movetos_;
                log_.notef("air ops AutoAttackTarget: squadron %u members=%zu no class -> moveto "
                    "\"%s\" (007F1688 -> 0071ECF0, packet cc9_l60_auto_attack_moveto)",
                    p.squadron_entity, members.size(), trow != nullptr ? trow->name.c_str() : "-");
            }
        }
        log_.notef("air ops AutoAttackTarget: squadron %u members=%zu target=\"%s\" class=%08X "
            "(007F4EC0 -> 007F15F0 -> 007EEC50, packet cc9_player_air_ops_launch)",
            p.squadron_entity, members.size(), trow != nullptr ? trow->name.c_str() : "-",
            chosen);
        if constexpr (bsp::kAutoAttackMemberOnLaunchBound) {
            const bool null_moveto = kAutoAttackMoveToMemberLaunchBound
                && bsp::kAutoAttackNullClassMoveToBound && chosen == 0u
                && !members.empty() && target < units_.count();
            if ((chosen != 0u || null_moveto) && record != nullptr) {
                SquadronStandingAttack s;
                s.squadron_entity = p.squadron_entity;
                s.target = p.target_plus_one - 1u;
                s.served = members;
                s.moveto = chosen == 0u;
                standing_attacks_.push_back(std::move(s));
            }
        }
        pending_auto_attacks_.erase(pending_auto_attacks_.begin() + static_cast<std::ptrdiff_t>(i));
    }
    if constexpr (bsp::kAutoAttackMemberOnLaunchBound) run_standing_attacks();
}

// Packet cc9_auto_attack_member_launch (bsp::kAutoAttackMemberOnLaunchBound).
// In the image 007F15F0 issues the class to the squadron (0071ECF0), and the
// squadron's +128h 007ECF80 applies it to every plane in +3D0h, whether it is
// flying or still on the deck. This host's intake issues per member, and a
// member Inside its base is not alive-and-visible, so a deck member is given the
// order on the first step it is airborne. SUBSTITUTION, labelled: the timing
// (at launch, not at the serve) and the per-member 007EEC50 choice (the image
// chooses once, on the squadron). A standing order ends when every registered
// member has been served or has left the squadron, when the target is gone,
// or after 6000 steps.
void GameScriptOrdersHost::run_standing_attacks() {
    for (std::size_t i = 0; i < standing_attacks_.size();) {
        SquadronStandingAttack& s = standing_attacks_[i];
        const bsp::PlaneSquadronHostRecord* record = nullptr;
        for (const AirOpsSquadron& made : squadrons_) {
            if (made.entity_id == s.squadron_entity) {
                record = bsp::plane_squadron_registry().find(made.name);
                break;
            }
        }
        std::size_t target = s.target;
        bool target_live = target < units_.count() && units_.unit_alive_and_visible(target);
        if (target < units_.count()) {
            // 007F15F7-007F1606, as at the serve: a plane target stands for its
            // squadron, which this host fuses into its flight leader.
            if (const bsp::PlaneSquadronHostRecord* t =
                    bsp::plane_squadron_registry().find_by_member_unit(target)) {
                const std::size_t lead = t->flight_leader();
                if (lead != bsp::kPlaneSquadronNoUnit && lead < units_.count()) {
                    target = lead;
                    target_live = units_.unit_alive_and_visible(lead);
                }
            }
        }
        bool open = false;
        if (record != nullptr && target_live) {
            for (const std::size_t m : record->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit) { open = true; continue; }
                if (std::find(s.served.begin(), s.served.end(), m) != s.served.end()) continue;
                open = true;
                if (m >= units_.count() || !units_.unit_alive_and_visible(m)) continue;
                const std::vector<std::size_t> one{m};
                std::uint32_t chosen = 0u;
                if (s.moveto) {
                    // 007F1688's moveto reaches this member through the same
                    // squadron fan-out (007ECF80) as a chosen class.
                    issue_command_to_members(one, 0x00E08F68u, target,
                                             "ai:auto_attack moveto 007F1688 member");
                    chosen = 0x00E08F68u;
                } else {
                    chosen = squadron_intake_007f1940(m, one, target);
                }
                s.served.push_back(m);
                if (chosen != 0u) ++standing_attack_member_orders_;
                const GameUnitRow* mrow = units_.unit_row(m);
                const GameUnitRow* trow = units_.unit_row(target);
                log_.notef("air ops AutoAttackTarget member at launch: squadron %u member=\"%s\" "
                    "target=\"%s\" class=%08X (007F15F0 -> 0071ECF0 -> 007ECF80 stand-in, packet "
                    "cc9_auto_attack_member_launch)", s.squadron_entity,
                    mrow != nullptr ? mrow->name.c_str() : "-",
                    trow != nullptr ? trow->name.c_str() : "-", chosen);
            }
        }
        if (!open || ++s.age_steps >= 6000) {
            standing_attacks_.erase(standing_attacks_.begin() + static_cast<std::ptrdiff_t>(i));
        } else {
            ++i;
        }
    }
}

PlayerAirOpsLaunchResult script_orders_player_air_ops_launch(const std::string& base,
    int slot_number, int vehicle_class, int count, const std::string& target, int loadout) {
    if (g_live_script_orders == nullptr) {
        PlayerAirOpsLaunchResult out;
        out.reason = "no live script-orders host";
        return out;
    }
    return g_live_script_orders->player_air_ops_launch(base, slot_number, vehicle_class,
                                                       count, target, loadout);
}

// The receiver's side of a Pilot* attack order, shared by PilotSetTarget and
// PilotBomb (packet cc13-fix16, a pure extraction from run_pilot_set_target:
// with `label` "PilotSetTarget" and its own counter the log lines and counts
// are unchanged). `tasks` counts the 0099A170 installs.
void GameScriptOrdersHost::install_pilot_attack_task_after_delivery(
    std::size_t leader_index, std::string leader_name, std::uint32_t chosen,
    bsp::SceneCommandTarget target, std::size_t target_index, const char* label,
    unsigned long long& tasks) {
    // Packet cc9_after_row9_continuation (docs/SENTITY_INIT_ATTACH_ORDER.md
    // 22.7). What follows is the receiver's side: 0099A170 reads the command
    // the director holds, so it runs once the order is delivered. A direct
    // order is delivered at once; a queued one at the drain. SUBSTITUTION,
    // labelled: the image installs in the bot's tick 0099ACD0 behind +7Ch,
    // one bot tick after the delivery; this host installs at the delivery.
    unsigned long long* const tasks_counter = &tasks;
    after_order_delivery([this, leader_index, leader_name, chosen, target,
                          target_index, label, tasks_counter]() {
    // 0099A170: the command the director now holds becomes a bot task. The
    // routine is src/attack_commands.cpp's; this only feeds it.
    const std::uint32_t unit_token =
        static_cast<std::uint32_t>(leader_index + 1u);
    const std::uint32_t target_token = target_index < units_.count()
        ? static_cast<std::uint32_t>(target_index + 1u) : 0u;
    ScriptOrderAttackCommandHost bot_host(units_, log_, chosen, target_token);
    const std::uint32_t task =
        bsp::bot_install_command_task_0099a170(unit_token, bot_host);
    // The class the task was built from, kept on the unit so a per-class
    // arm can ask whether its own task exists rather than guessing from
    // ordnance. docs/DIVE_BOMB_TASK.md, "The class gate".
    if (task != 0u) units_.store_unit_attack_command_class(leader_index, chosen);
    if (task != 0u) ++*tasks_counter;
    log_.notef("  %s task: 0099A170 -> %u (unit=%s command=%08lx "
        "target_token=%u refusals=%u)", label, task, leader_name.c_str(),
        static_cast<unsigned long>(chosen), target_token, bot_host.refusals());

    // 007ECF80, the squadron's own vtable slot +128h: a squadron applies what
    // it is given to every live member of +3D0h under +3CCh, in array order.
    // docs/PLANE_SQUADRON_ENTITY.md section 4 established that shape for the
    // skill-level setter, and the order path is the same shape applied to an
    // order - a SUBSTITUTION for the unread order arms of the +164h
    // dispatcher 007F0030, not a reconstruction of them. What makes it
    // necessary rather than optional is 007EEF30: it walks +3D0h, and a
    // squadron whose wingmen carry no task has nothing to issue a release
    // order to.
    //
    // The ordered unit is the fused flight leader and already has its task
    // from the block above, so the fan-out starts at slot 1.
    bsp::PlaneSquadronHostRecord* squadron =
        bsp::plane_squadron_registry().find_by_member_unit(leader_index);
    std::size_t fanned = 0;
    if (squadron != nullptr) {
        // entity_issue_command writes the outcome onto the binding row being
        // run, which named the leader. Keep the leader's outcome and let the
        // wingmen's show up in the summary counters and the fan-out line
        // below. At a drain no row is being run.
        GameScriptOrderRow* const leader_row = row_;
        GameScriptOrderRow saved;
        if (leader_row != nullptr) saved = *leader_row;
        for (std::size_t slot = 0; slot < squadron->member_units.size(); ++slot) {
            const std::size_t member = squadron->member_units[slot];
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            if (member == leader_index) continue;
            void* const member_handle =
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(member + 1u));
            entity_issue_command(member_handle, chosen, target, 1);
            // Packet cc9_set_command_queue_delay: with the loopback queue
            // bound, this wingman's order is delivered next in the drain,
            // after the leader's delivery returns, so its 0099A170 reads the
            // director then. Otherwise it runs here, as before.
            std::function<void()> install = [this, member, chosen, target_token,
                                             tasks_counter]() {
                ScriptOrderAttackCommandHost member_bots(units_, log_, chosen,
                                                         target_token);
                const std::uint32_t member_task = bsp::bot_install_command_task_0099a170(
                    static_cast<std::uint32_t>(member + 1u), member_bots);
                if (member_task != 0u) {
                    units_.store_unit_attack_command_class(member, chosen);
                    ++*tasks_counter;
                }
            };
            if (commands_after_last_issue_delivery(install)) continue;
            const auto before = *tasks_counter;
            install();
            if (*tasks_counter != before) ++fanned;
        }
        if (leader_row != nullptr) {
            leader_row->command = saved.command;
            leader_row->target = saved.target;
            leader_row->issued = saved.issued;
            leader_row->reached_director = saved.reached_director;
            leader_row->blocked = saved.blocked;
        }
        log_.notef("  %s fan-out 007ECF80: squadron=%s +3CCh=%d "
            "-> %zu wingman task(s) beside the leader %s", label,
            squadron->name.c_str(), squadron->live_count(), fanned,
            leader_name.c_str());
    }
    });
}

// Goal 5 chain 663. PilotSetTarget 008A4C90, PilotMoveTo 008A4150, PilotMoveToRange
// 008A4590 and PilotLand 008A47B0 run their bodies (src/pilot_order_bindings.cpp) over
// this adapter, one service per native call site the bodies name:
//  * 00888AA0 unit_from_argument: the table's `Ptr` (00888D20's read, this host seeds
//    `Ptr` with the entity's id). SUBSTITUTION, labelled, kept from the rows these
//    bodies replace: a table whose `Ptr` is not a light userdata falls back to its
//    `ID` field (entity_from_argument).
//  * 0088A810 command_target_from_argument: bsp::lua_read_command_target.
//  * 00B663F0 / 00B66290 / 00B66270: the frame's argument count, integer and number.
//  * the +5Ch/+5Dh/+60h/+5Eh quartet (liveness): the units host's persistent cells, the
//    same four 0043F080 reads GameUnitsHost::unit_alive_and_visible makes; unavailable
//    state fails the gate. SUBSTITUTION, labelled, as packet
//    cc9_squadron_observer_liveness reads it: a squadron's fused slot stands for the
//    squadron entity, which 007F3970 kills (00926D90) only when +3CCh reaches 0, so while
//    the registry record keeps a live plane the slot answers the squadron's live
//    quartet after its own plane died.
//  * 00521EA0 resolve_target_object: GameUnitsHost::resolve_command_target_00521ea0, the
//    one-based handle this host's entities are. The +4h store caches the identity the
//    descriptor already holds, so it is not repeated.
//  * 007EEC50 choose_attack_command: choose_attack_class_007eec50.
//  * 0077D600 issue_command: the row's own issue, which is entity_issue_command and then
//    the receiver's side the image runs behind the delivery (0099A170's install, the
//    007ECF80 squadron fan-out).
//  * +C8h / 00414DB0: GameUnitsHost::unit_pose_valid_00c8 and entity_pose_refresh_00414db0.
// PilotRetreat 008A4300's three services (+54h, +FCh, 004C7730) are not bound: its row
// keeps run_pilot_retreat, because the 004C7730 zone corners are only reachable inside
// GameUnitsHost::retreat_point_008a4300.
class GameScriptOrdersHost::PilotOrderBinding final : public bsp::PilotOrderHost {
public:
    using Issue = std::function<void(void*, std::uint32_t, const bsp::SceneCommandTarget&, int)>;
    PilotOrderBinding(GameScriptOrdersHost& host, const char* label, Issue issue)
        : host_(host), label_(label), issue_(std::move(issue)) {}

    int argument_count() override { return host_.argument_count_; }
    void* unit_from_argument(int index) override {
        void* unit = host_.argument_ptr_field(index);
        if (unit == nullptr) unit = host_.entity_from_argument(index);
        return unit;
    }
    bsp::SceneCommandTarget command_target_from_argument(int index) override {
        return bsp::lua_read_command_target(host_, index);
    }
    int argument_as_int(int index) override { return host_.argument_integer(index); }
    float argument_as_float(int index) override { return host_.argument_number(index); }

    bsp::EntityLivenessBytes liveness(void* entity) override {
        bsp::EntityLivenessBytes bytes;   // present clear: the quartet fails
        const std::size_t index = host_.index_of(entity);
        bsp::SceneNodeFlags node;
        bool pending = false;
        if (index >= host_.units_.count() || !host_.units_.unit_scene_node_flags(index, node)
            || !host_.units_.unit_pending_destroy_0060(index, pending)) {
            return bytes;
        }
        bytes.present = node.active;          // +5Ch
        bytes.out_of_action = node.torn_down; // +5Dh
        bytes.flag_60 = pending;              // +60h
        bytes.flag_5e = node.destroyed;       // +5Eh
        if (bytes.flag_5e || bytes.flag_60) {
            const bsp::PlaneSquadronHostRecord* squadron = fused_squadron_record(index);
            if (squadron != nullptr && squadron->live_count() > 0) {
                bytes = bsp::EntityLivenessBytes{true, false, false, false};
                ++squadron_holds_;
            }
        }
        return bytes;
    }
    void* resolve_target_object(bsp::SceneCommandTarget& target) override {
        const std::uint32_t handle = host_.units_.resolve_command_target_00521ea0(target);
        resolved_ = reinterpret_cast<void*>(static_cast<std::uintptr_t>(handle));
        return resolved_;
    }
    void* choose_attack_command(void* unit, void* target, bool prefer_ordnance,
                                bool allow_guns) override {
        const std::uint32_t chosen = host_.choose_attack_class_007eec50(host_.index_of(unit),
            host_.index_of(target), prefer_ordnance, allow_guns, label_);
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(chosen));
    }
    void issue_command(void* unit, std::uint32_t command_class,
                       const bsp::SceneCommandTarget& target, int flags) override {
        issued_ = true;
        issue_(unit, command_class, target, flags);
    }
    bool world_pose_valid(void* entity) override {
        const std::size_t index = host_.index_of(entity);
        return index >= host_.units_.count() || host_.units_.unit_pose_valid_00c8(index);
    }
    void refresh_world_pose(void* entity) override { host_.entity_pose_refresh_00414db0(entity); }
    int entity_side(void*) override { unreached("+54h (PilotRetreat)"); }
    const float* entity_world_position(void*) override { unreached("+FCh (PilotRetreat)"); }
    bsp::PilotRetreatZoneCorners retreat_zone(const float*, int) override {
        unreached("004C7730 (PilotRetreat)");
    }

    void* resolved() const noexcept { return resolved_; }
    bool issued() const noexcept { return issued_; }
    unsigned squadron_holds() const noexcept { return squadron_holds_; }

private:
    [[noreturn]] static void unreached(const char* what) {
        throw std::logic_error(std::string("PilotOrderBinding: unreached ") + what);
    }
    GameScriptOrdersHost& host_;
    const char* label_;
    Issue issue_;
    void* resolved_{nullptr};
    bool issued_{false};
    unsigned squadron_holds_{0};
};

// 008A4C90 on its body: the unit quartet, argument 1 (0088A810), the attack type
// (008A4E0B) and flags (008A4E54), 00521EA0, 007EEC50, the target quartet, the repeated
// 007EEC50 when the first answered null, then 0077D600(unit, class, &target, 1) at
// 008A4EAC. The issue is followed by the receiver's 0099A170 install after delivery.
int GameScriptOrdersHost::run_pilot_set_target(GameScriptOrderRow& row) {
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);

    // The descriptor and attack type the body reads, for the log line only.
    const bsp::SceneCommandTarget target = bsp::lua_read_command_target(*this, 1);
    const int attack_type = bsp::pilot_set_target_attack_type_008a4e0b(
        argument_count_, argument_integer(2));
    const bsp::PilotAttackSelectorFlags flags =
        bsp::pilot_attack_selector_flags_008a4e54(attack_type);

    std::uint32_t chosen = 0;
    PilotOrderBinding binding(*this, "PilotSetTarget",
        [this, &chosen](void* entity, std::uint32_t cls, const bsp::SceneCommandTarget& issued,
                        int issue_flags) {
            chosen = cls;
            const std::size_t unit_index = index_of(entity);
            entity_issue_command(entity, cls, issued, issue_flags);   // 0077D600
            ++pilot_set_target_issued_;
            install_pilot_attack_task_after_delivery(unit_index, name_of(entity), cls, issued,
                index_of(issued.object), "PilotSetTarget", pilot_set_target_tasks_);
        });
    const bsp::PilotSetTargetOutcome outcome = bsp::pilot_set_target_008a4c90(binding);
    const char* outcome_text = "ISSUED";
    switch (outcome) {
    case bsp::PilotSetTargetOutcome::kUnitNotLive: outcome_text = "not issued (unit not live)"; break;
    case bsp::PilotSetTargetOutcome::kTargetNotLive:
        outcome_text = "not issued (target not live)";
        break;
    case bsp::PilotSetTargetOutcome::kNoCommandClass:
        outcome_text = "not issued (007EEC50 declined)";
        break;
    case bsp::PilotSetTargetOutcome::kIssued: break;
    }
    log_.notef("  PilotSetTarget: unit=%s target_object_id=%u target_valid=%d "
        "pos=(%.1f %.1f %.1f) attack_type=%d prefer_ordnance=%d allow_guns=%d "
        "class=%08lx squadron_holds=%u -> %s (008A4C90 body, goal 5 chain 663)",
        row.unit.empty() ? "(unresolved)" : row.unit.c_str(),
        static_cast<unsigned>(target.object_id),
        target.position_valid ? 1 : 0,
        static_cast<double>(target.position[0]),
        static_cast<double>(target.position[1]),
        static_cast<double>(target.position[2]),
        attack_type, flags.prefer_ordnance ? 1 : 0, flags.allow_guns ? 1 : 0,
        static_cast<unsigned long>(chosen), binding.squadron_holds(), outcome_text);
    ++pilot_set_target_calls_;
    if (!row.unit.empty()) ++pilot_set_target_unit_resolved_;
    if (target.object_id != 0 || target.position_valid) ++pilot_set_target_target_resolved_;
    // 00B66400 calls mission_binding_result_count; OFF keeps the zero-result return.
    if constexpr (kWireScriptOrdersHostBound) {
        return state_ != nullptr ? bsp::mission_binding_result_count(lua_gettop(state_),
            {bsp::kBindingFirstArgumentSlot, argument_count_}) : 0;
    } else {
    return 0;  // 00B66400: the binding pushes nothing.
    }
}

// 008A4590 `PilotMoveToRange(unit, target [, range])`, packet cc9_pilot_moveto_task.
// The native issues command 00E08F68 (moveto) through 0077D600 with the target
// descriptor, whose +14h holds the range when exactly three arguments are
// passed (008A46DC-008A4708, pilot_move_to_range_008a46dc). 0099A170's moveto
// arm (0099A223-0099A236) takes it without a target precondition and builds
// the kind-7 task 009C3BE0 -> 009C3000. The squadron fan-out is the same
// 007ECF80 route PilotSetTarget takes, so every member gets the order and its
// own task. docs/PILOT_MOVETO_TASK.md.
// Goal 5 chain 663: PilotMoveTo 008A4150 and PilotMoveToRange 008A4590 run their bodies
// pilot_move_to_008a4150 / pilot_move_to_range_008a4590 over PilotOrderBinding. The
// bodies read both arguments (and the range at 008A46DC), issue at 008A42A7 / 008A472A,
// and PilotMoveToRange then tests +C8h (008A472F). The issue below is this host's
// 0077D600 and the receiver's side behind it: the 007ECF80 fan-out and each order's
// 0099A170 install after its delivery.
int GameScriptOrdersHost::run_pilot_move_to_range(GameScriptOrderRow& row) {
    if constexpr (!bsp::kPilotMoveToTaskBound) {
        return 0;
    }
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    // The log's task count is the installs made before this binding returns.
    // The continuation counts into the member, never into this frame: with
    // kSetCommandQueueDelayBound a direct order's continuation runs at the
    // session drain, after the return (packet b2-pilotmoveto-fix; the old
    // pointer to a local here incremented a dead stack slot and corrupted
    // JM02's GenerateObject record, 0xC0000374).
    const unsigned long long tasks_before = pilot_move_to_tasks_;
    std::uint32_t logged_token = 0;
    float logged_range = 0.0f;
    const auto issue = [this, &logged_token, &logged_range](void* entity, std::uint32_t cls,
                                                           const bsp::SceneCommandTarget& target,
                                                           int flags) {
        const std::size_t unit_index = index_of(entity);
        const std::size_t target_index = target.object != nullptr
            ? index_of(target.object)
            : (target.object_id > 0 ? static_cast<std::size_t>(target.object_id - 1)
                                    : ~static_cast<std::size_t>(0));
        const std::uint32_t target_token = target_index < units_.count()
            ? static_cast<std::uint32_t>(target_index + 1u) : 0u;
        logged_token = target_token;
        logged_range = target.trailing;
        // Packet cc9_pilot_move_to: a target that is no unit but a scene marker
        // (a NavPoint) steers the task to the marker's authored position, which is
        // what 009BEBA0 reads off [approach+44h] in the image.
        bool marker_goal = false;
        float marker_world[3] = {0.0f, 0.0f, 0.0f};
        if (kPilotMoveToBound && target_token == 0u) {
            const int marker_id = target.object != nullptr
                ? static_cast<int>(reinterpret_cast<std::uintptr_t>(target.object))
                : static_cast<int>(target.object_id);
            if (const SceneMarker* marker = marker_for_id(marker_id)) {
                marker_goal = true;
                marker_world[0] = marker->position[0];
                marker_world[1] = marker->position[1];
                marker_world[2] = marker->position[2];
                ++pilot_marker_goals_;
            }
        }
        auto order_one = [&](std::size_t index, void* handle) {
            entity_issue_command(handle, cls, target, flags);
            // Packet cc9_after_row9_continuation: the install follows the delivery
            // (a queued order's at the drain), as in run_pilot_set_target.
            after_order_delivery([this, index, cls, target, target_index, target_token,
                                  marker_goal, marker_world]() {
                ScriptOrderAttackCommandHost bot_host(units_, log_, cls, target_token);
                const std::uint32_t task = bsp::bot_install_command_task_0099a170(
                    static_cast<std::uint32_t>(index + 1u), bot_host);
                if (task != 0u) {
                    units_.store_unit_attack_command_class(index, cls);
                    units_.store_unit_moveto_range(index, target.trailing);
                    units_.store_unit_moveto_target(index, target_index);
                    if (marker_goal) units_.store_unit_moveto_point(index, marker_world);
                    ++pilot_move_to_tasks_;
                }
            });
        };
        // The bodies issue without a null test; 0077D600 on no units-host slot does
        // nothing here (entity_issue_command's own test), as before.
        if (entity == nullptr || unit_index >= units_.count()) return;
        order_one(unit_index, entity);
        bsp::PlaneSquadronHostRecord* squadron =
            bsp::plane_squadron_registry().find_by_member_unit(unit_index);
        if (squadron != nullptr) {
            for (const std::size_t member : squadron->member_units) {
                if (member == bsp::kPlaneSquadronNoUnit || member == unit_index) continue;
                order_one(member,
                          reinterpret_cast<void*>(static_cast<std::uintptr_t>(member + 1u)));
            }
        }
    };
    PilotOrderBinding binding(*this, pilot_move_to_plain_ ? "PilotMoveTo" : "PilotMoveToRange",
                              issue);
    if (pilot_move_to_plain_) {
        bsp::pilot_move_to_008a4150(binding);         // never reads argument 2: +14h is 0
    } else {
        bsp::pilot_move_to_range_008a4590(binding);
    }
    log_.notef("  %s: unit=%s target_token=%u range=%.1f -> %zu moveto task(s) "
        "(%s body -> 0077D600 00E08F68 -> 0099A170 -> 009C3BE0)",
        pilot_move_to_plain_ ? "PilotMoveTo" : "PilotMoveToRange",
        row.unit.empty() ? "(unresolved)" : row.unit.c_str(),
        static_cast<unsigned>(logged_token), static_cast<double>(logged_range),
        static_cast<std::size_t>(pilot_move_to_tasks_ - tasks_before),
        pilot_move_to_plain_ ? "008A4150" : "008A4590");
    return 0;  // the binding pushes nothing
}

// 008A0A10 `EntityTurnToEntity(entity, target [, keep_pitch])`, packet
// cc9_pilot_moveto_task part 1b. The single-player arm ([00E188A8]+1FE4h is 0,
// so the session-message arm through 007EDEB0 is not taken):
//  - d = target+FCh - entity+FCh (008A0B6A-008A0BD8); d.y is zeroed unless a
//    third argument is present and true (008A0C0A-008A0C5F);
//  - rows (1,0,0), (0,1,0), d and position 0 at esp+34h, then 0085DC80
//    (008A0C65-008A0CE3);
//  - a squadron (vtable[5Ch](18h)) re-poses each of its +3CCh members, at most
//    five, at the member's own +FCh through member vtable[88h] (008A0D6D-
//    008A0DE2), which is 007C9540 for a plane.
// Only the squadron arm is bound. The entity's +FCh is the squadron's spawn
// matrix, which 007F4580 gives every member, so the leader's position stands in
// for it. A plane or ship argument is left a record: those arms are unread.
int GameScriptOrdersHost::run_entity_turn_to_entity(GameScriptOrderRow& row) {
    if constexpr (!bsp::kMissionTurnAndStanceBound) {
        return 0;
    }
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    void* target = entity_from_argument(1);
    if (target == nullptr) target = argument_ptr_field(1);
    const std::size_t target_index = index_of(target);
    bsp::PlaneSquadronHostRecord* squadron = row.unit_index < units_.count()
        ? bsp::plane_squadron_registry().find_by_member_unit(row.unit_index) : nullptr;
    if constexpr (kTurnToEntityMemberPlaneArmBound) {
        // 008A0D5D: the squadron arm is the argument's own vt[5Ch](18h). A
        // member plane other than the fused leader answers 0Fh, not 18h, and
        // takes 008A0E6F's vtable[88h] arm on itself alone.
        if (squadron != nullptr && squadron->squadron_unit != row.unit_index) {
            squadron = nullptr;
        }
    }
    const bool other_arm = squadron == nullptr && row.unit_index < units_.count()
        && target_index < units_.count();
    if (target_index >= units_.count() || (squadron == nullptr &&
        (!kScriptedOrderNativesBound || !other_arm))) {
        log_.unimplemented("MissionLuaNative::EntityTurnToEntity non-squadron arm", "008a0d1c");
        return 0;
    }
    float ex = 0.0f, ey = 0.0f, ez = 0.0f, tx = 0.0f, ty = 0.0f, tz = 0.0f;
    units_.unit_position_00fc(row.unit_index, ex, ey, ez);
    units_.unit_position_00fc(target_index, tx, ty, tz);
    const bool keep_pitch = argument_count_ == 3 && argument_boolean(2);
    float m[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f, 0.0f,
                   tx - ex, keep_pitch ? ty - ey : 0.0f, tz - ez, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f};
    bsp::orthonormalize_pose_matrix_0085dc80(m);
    if (squadron == nullptr) {
        // Packet cc9_scripted_order_natives. 008A0DE9: vtable[5Ch](6), the ship
        // family. Both arms refresh +FCh (00414DB0 when +C8h is clear), put it
        // in the translation row (esp+64h..6Ch = m[12..14]) and call
        // vtable[88h] with the matrix. The ship arm then builds a transform
        // (00C336C0) and poses the physics body [0080E490()+2Ch] through
        // 00C56CF0. SUBSTITUTION, labelled: this host's ship pose and physics
        // state are one motion state, so 006E00A0's store covers both; the
        // body's velocity is not touched, as the image leaves it.
        m[12] = ex;
        m[13] = ey;
        m[14] = ez;
        const bool ship = units_.unit_is_kind_of(row.unit_index, 6);
        bool posed = false;
        if (ship) {
            posed = units_.set_local_matrix_006e00a0(row.unit_index, m);   // vtable[88h]
            if (posed) ++turn_ship_arm_posed_;
            log_.implemented("EntityTurnToEntity::ship_body_pose_00c56cf0 (the motion state)",
                             "00c56cf0");
        } else if (units_.unit_is_kind_of(row.unit_index, 0x0f)) {
            posed = units_.set_unit_world_basis_007c9540(row.unit_index, &m[0], &m[4],
                                                         &m[8]);
        } else {
            log_.unimplemented("MissionLuaNative::EntityTurnToEntity vtable[88h] arm",
                               "008a0eb8");
        }
        log_.notef("  EntityTurnToEntity: %s %s -> %s: posed=%d forward (%.3f %.3f %.3f) "
            "(008A0A10 %s, packet cc9_scripted_order_natives)",
            ship ? "ship" : "entity", row.unit.c_str(), name_of(target).c_str(), posed ? 1 : 0,
            static_cast<double>(m[8]), static_cast<double>(m[9]), static_cast<double>(m[10]),
            ship ? "008A0DE9 -> 006E00A0 + 00C56CF0" : "008A0E6F -> vtable[88h]");
        return 0;
    }
    std::size_t posed = 0;
    std::size_t seat = 0;
    for (const std::size_t member : squadron->member_units) {
        if (seat++ > 4) break;                       // 008A0D80: at most five
        if (member == bsp::kPlaneSquadronNoUnit) continue;
        if (units_.set_unit_world_basis_007c9540(member, &m[0], &m[4], &m[8])) ++posed;
    }
    log_.notef("  EntityTurnToEntity: squadron %s (argument %s) -> %s: %zu member(s) re-posed, "
        "forward (%.3f %.3f %.3f) (008A0A10 -> 0085DC80 -> vtable[88h] 007C9540)",
        squadron->name.c_str(), row.unit.c_str(), name_of(target).c_str(), posed,
        static_cast<double>(m[8]), static_cast<double>(m[9]), static_cast<double>(m[10]));
    return 0;
}

// 008A6490 `UnitSetFireStance(unit, stance)`, packet cc9_pilot_moveto_task part
// 1b, the squadron arm. vtable[114h] of a squadron is 007ECFD0, which hands back
// the +348h command block 007F4FE3-007F5009 builds with 0084D810 (vtable
// 00D0BD98). 0071BE80 asks that block's own predicates:
//   +24h 0084D910: allowFire for stance 1 or 2;
//   +28h 0084D930: allowMove for stance 0, 2 or 3;
// then stores them through 0071DA50/0071DAD0 (+3Ch, +3Dh). The block's
// defaults (0084D862-0084D8A2): Behaviour (+364h) 0 frees both; any other
// Behaviour frees fire only for a fighter class (vtable[18h](13h)), move only
// otherwise. A plane's vtable[114h] is 0047F180 (XOR EAX,EAX), so a plane
// argument is the image's own no-op. A ship argument is left a record.
namespace {
// Chain 257. Evidence: `bsp.py show 0071BED0 --asm` (0071BEDB/0071BEE8
// push 0) and 0071BF70 --asm (0071BF7B/0071BF88 push 2) call slots 24h,
// 28h, 40h, 44h in that order, querying both permissions before either send.
// 007ECFD0 returns the real squadron's +348h command block, whose slots
// 24h/28h are 0084D910/0084D930; a ship uses 0071D560/0071D580. In particular,
// stance 0 allows squadron movement but forbids ship movement. The two output
// bytes below are committed by the caller to its existing live permission row.
class ScriptDirectorStanceHost final : public bsp::WeaponDirectorStanceHost {
public:
    explicit ScriptDirectorStanceHost(bool squadron) : squadron_(squadron) {}

    bool director_stance_allows_fire(bsp::FireStance stance) override {
        return squadron_
            ? bsp::cc13_seg52::squadron_block_allows_category24_0084d910(
                  static_cast<std::int32_t>(stance)) != 0
            : bsp::director_stance_allows_fire_0071d560(stance);
    }
    bool director_stance_allows_move(bsp::FireStance stance) override {
        return squadron_
            ? bsp::cc13_seg52::squadron_block_allows_category28_0084d930(
                  static_cast<std::int32_t>(stance)) != 0
            : bsp::director_stance_allows_move_0071d580(stance);
    }
    void send_allow_fire(bool allowed) override { allow_fire = allowed; }
    void send_allow_move(bool allowed) override { allow_move = allowed; }

    bool allow_fire{};
    bool allow_move{};

private:
    bool squadron_;
};

GameScriptOrdersHost::SquadronPermissions squadron_permissions_0084d810(
    const bsp::PlaneSquadronHostRecord& squadron, bool fighter) noexcept {
    GameScriptOrdersHost::SquadronPermissions p;
    if (squadron.behaviour == 0) {
        p.allow_fire = true;
        p.allow_move = true;
    } else if (fighter) {
        p.allow_fire = true;
    } else {
        p.allow_move = true;
    }
    return p;
}

// Packet g44_scan_row_write (kDirectorFirePermissionClearBound): the 5Ah stance
// message reaches 00836210 through 0071C1E0 sub-kind 0, which drops director+238h
// when the stance forbids fire (0071D560 false). The ship arms call this after
// 0071BE80's host. Returns true when a fire target was dropped.
bool director_fire_permission_00836210(GameShipAiHost* ship_ai, std::size_t unit,
                                       int stance) {
    if constexpr (kDirectorFirePermissionClearBound) {
        if (ship_ai == nullptr) return false;
        return ship_ai->store_fire_permission_00836210(unit,
            bsp::director_stance_allows_fire_0071d560(static_cast<bsp::FireStance>(stance)));
    } else {
        static_cast<void>(ship_ai);
        static_cast<void>(unit);
        static_cast<void>(stance);
        return false;
    }
}
}  // namespace

// Packet p15_free_attack_tick: the bytes run_unit_set_fire_stance keeps, with the
// same fighter test (the leader's IsKindOf(13h)) for 0084D810's defaults.
bool GameScriptOrdersHost::squadron_permissions_3c3d(
    const bsp::PlaneSquadronHostRecord& squadron, bool& allow_fire, bool& allow_move) {
    const std::size_t leader = squadron.member_units.empty()
        ? bsp::kPlaneSquadronNoUnit : squadron.member_units.front();
    const bool fighter = leader != bsp::kPlaneSquadronNoUnit &&
        units_.unit_is_kind_of(leader, 0x13);
    const auto found = squadron_permissions_.find(squadron.name);
    const SquadronPermissions p = found != squadron_permissions_.end()
        ? found->second : squadron_permissions_0084d810(squadron, fighter);
    allow_fire = p.allow_fire;
    allow_move = p.allow_move;
    return true;
}

int GameScriptOrdersHost::run_unit_set_fire_stance(GameScriptOrderRow& row) {
    if constexpr (!bsp::kMissionTurnAndStanceBound) {
        return 0;
    }
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    const int stance = argument_integer(1);
    bsp::PlaneSquadronHostRecord* squadron = row.unit_index < units_.count()
        ? bsp::plane_squadron_registry().find_by_member_unit(row.unit_index) : nullptr;
    if (squadron == nullptr) {
        if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 0x0f)) {
            log_.implemented("Plane::weapon_director_0047f180 (null, 0071BE80 returns)",
                             "0047f180");
        } else if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 6)
                   && units_.gunnery() != nullptr) {
            // Packet cc9_ship_fire_stance (routed from gunnery13): the ship's
            // director predicates 0071D560 / 0071D580, stored by the gunnery host.
            // The director fire-target release when a stance forbids fire (00836210
            // at 0083622B) is kDirectorFirePermissionClearBound.
            {
                ScriptDirectorStanceHost director(false);
                bsp::set_fire_stance_0071be80(director, static_cast<bsp::FireStance>(stance));
                units_.gunnery()->store_director_stance_permissions(row.unit_index, stance,
                    director.allow_fire, director.allow_move);
            }
            director_fire_permission_00836210(units_.ship_ai(), row.unit_index, stance);
            log_.implemented("MissionLuaNative::UnitSetFireStance ship arm (0071BE80)",
                             "0071be80");
        } else {
            log_.unimplemented("MissionLuaNative::UnitSetFireStance non-squadron arm",
                               "0071be80");
        }
        return 0;
    }
    const std::size_t leader = squadron->member_units.empty()
        ? bsp::kPlaneSquadronNoUnit : squadron->member_units.front();
    const bool fighter = leader != bsp::kPlaneSquadronNoUnit &&
        units_.unit_is_kind_of(leader, 0x13);
    auto found = squadron_permissions_.find(squadron->name);
    const SquadronPermissions before = found != squadron_permissions_.end()
        ? found->second : squadron_permissions_0084d810(*squadron, fighter);
    SquadronPermissions after;
    {
        ScriptDirectorStanceHost director(true);
        bsp::set_fire_stance_0071be80(director, static_cast<bsp::FireStance>(stance));
        after.allow_fire = director.allow_fire;
        after.allow_move = director.allow_move;
    }
    squadron_permissions_[squadron->name] = after;                  // 0071DA50/0071DAD0
    log_.notef("  UnitSetFireStance: squadron %s stance=%d allowFire %d->%d allowMove %d->%d "
        "(behaviour=%d fighter=%d; 008A6490 -> 007ECFD0 +348h -> 0071BE80)",
        squadron->name.c_str(), stance, before.allow_fire ? 1 : 0, after.allow_fire ? 1 : 0,
        before.allow_move ? 1 : 0, after.allow_move ? 1 : 0,
        static_cast<int>(squadron->behaviour), fighter ? 1 : 0);
    return 0;
}

// 008A6AC0 `UnitHoldFire(unit)`, packet cc9_scripted_order_natives. Argument 0
// through 00888AA0, then vtable[114h] and 0071BED0 on the answer (008A6BD6-
// 008A6BE4, no null check). 0071BED0 is 0071BE80 with the literal stance 0
// (0071BEDB / 0071BEEA). No result is pushed.
int GameScriptOrdersHost::run_unit_hold_fire(GameScriptOrderRow& row) {
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    bsp::PlaneSquadronHostRecord* squadron = row.unit_index < units_.count()
        ? bsp::plane_squadron_registry().find_by_member_unit(row.unit_index) : nullptr;
    if (squadron != nullptr) {
        // 007ECFD0 +348h: 0084D910 (stance 1 or 2) and 0084D930 (0, 2 or 3).
        {
            ScriptDirectorStanceHost director(true);
            bsp::hold_fire_0071bed0(director);
            squadron_permissions_[squadron->name] = SquadronPermissions{
                director.allow_fire, director.allow_move};
        }
        log_.notef("  UnitHoldFire: squadron %s allowFire 0 allowMove 1 "
            "(008A6AC0 -> 007ECFD0 +348h -> 0071BED0)", squadron->name.c_str());
    } else if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 0x0f)) {
        // 0047F180 answers null, and 0071BED6 (MOV EAX,[ESI]) reads through it.
        // This host cannot reproduce the fault, so it records it and does nothing.
        ++hold_fire_null_director_;
        log_.notef("  UnitHoldFire: plane %s: vtable[114h] 0047F180 is null; the image "
            "faults at 0071BED6 (recorded, nothing done)", row.unit.c_str());
    } else if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 6)
               && units_.gunnery() != nullptr) {
        // Packet cc9_ship_fire_stance (routed from gunnery13): 0071BED0 is
        // 0071BE80 with stance 0 on the ship's director.
        {
            ScriptDirectorStanceHost director(false);
            bsp::hold_fire_0071bed0(director);
            units_.gunnery()->store_director_stance_permissions(row.unit_index, 0,
                director.allow_fire, director.allow_move);
        }
        director_fire_permission_00836210(units_.ship_ai(), row.unit_index, 0);
        log_.implemented("MissionLuaNative::UnitHoldFire ship arm (0071BED0)", "0071bed0");
    } else {
        log_.unimplemented("MissionLuaNative::UnitHoldFire director arm (0071DA50 5Ah)",
                           "0071bed0");
    }
    return 0;
}

// 008A67E0 `UnitFreeAttack(unit)` and 008A6950 `UnitFreeFire(unit)`, packet
// cc13-ver12 (docs/CC13_VER_12.md section 1). Each has UnitHoldFire's shape:
// argument 0 through 00888AA0, vt[114h], then the wrapper with its literal -
// 0071BF70 PUSH 2 (0071BF7B) and 0071BF20 PUSH 1 (0071BF2B) - into the same
// vt[24h]/vt[28h] predicates and 5Ah sub-kinds 0/1 0071BE80 sends. No result
// is pushed. So the stance overwrites the row UnitHoldFire set, which is what
// lets a held unit fire again. unit_weapons.cpp's lua_unit_free_fire_008a6950
// is the same body over UnitWeaponHost, an interface this process does not
// implement; this routes through the director store UnitHoldFire uses.
int GameScriptOrdersHost::run_unit_free_fire(GameScriptOrderRow& row,
                                             bsp::ScriptFireOrder order) {
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    const int stance = bsp::script_fire_order_stance_0071be80(order);
    const char* native = order == bsp::ScriptFireOrder::kFreeAttack ? "UnitFreeAttack"
                                                                    : "UnitFreeFire";
    bsp::PlaneSquadronHostRecord* squadron = row.unit_index < units_.count()
        ? bsp::plane_squadron_registry().find_by_member_unit(row.unit_index) : nullptr;
    if (squadron != nullptr) {
        // 007ECFD0 +348h: 0084D910 (stance 1 or 2) and 0084D930 (0, 2 or 3).
        SquadronPermissions after;
        {
            ScriptDirectorStanceHost director(true);
            if (order == bsp::ScriptFireOrder::kFreeAttack) {
                bsp::free_attack_0071bf70(director);
            } else {
                bsp::free_fire_0071bf20(director);
            }
            after.allow_fire = director.allow_fire;
            after.allow_move = director.allow_move;
        }
        squadron_permissions_[squadron->name] = after;
        log_.notef("  %s: squadron %s allowFire %d allowMove %d (+348h stance %d)", native,
            squadron->name.c_str(), after.allow_fire ? 1 : 0, after.allow_move ? 1 : 0,
            stance);
    } else if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 0x0f)) {
        // As UnitHoldFire: 0047F180 answers null and the wrapper reads through it.
        log_.notef("  %s: plane %s: vtable[114h] 0047F180 is null; the image faults in "
            "the wrapper (recorded, nothing done)", native, row.unit.c_str());
    } else if (row.unit_index < units_.count() && units_.unit_is_kind_of(row.unit_index, 6)
               && units_.gunnery() != nullptr) {
        {
            ScriptDirectorStanceHost director(false);
            if (order == bsp::ScriptFireOrder::kFreeAttack) {
                bsp::free_attack_0071bf70(director);
            } else {
                bsp::free_fire_0071bf20(director);
            }
            units_.gunnery()->store_director_stance_permissions(row.unit_index, stance,
                director.allow_fire, director.allow_move);
        }
        director_fire_permission_00836210(units_.ship_ai(), row.unit_index, stance);
        log_.implemented(order == bsp::ScriptFireOrder::kFreeAttack
            ? "MissionLuaNative::UnitFreeAttack ship arm (0071BF70)"
            : "MissionLuaNative::UnitFreeFire ship arm (0071BF20)",
            order == bsp::ScriptFireOrder::kFreeAttack ? "0071bf70" : "0071bf20");
    } else {
        log_.unimplemented(order == bsp::ScriptFireOrder::kFreeAttack
            ? "MissionLuaNative::UnitFreeAttack director arm (0071DA50 5Ah)"
            : "MissionLuaNative::UnitFreeFire director arm (0071DA50 5Ah)",
            order == bsp::ScriptFireOrder::kFreeAttack ? "0071bf70" : "0071bf20");
    }
    return 0;
}

// 008A7400 `NavigatorStop(unit)`, packet cc13-ver12 (docs/CC13_VER_12.md
// section 2). Argument 0 through 00888AA0 (008A74FE), a descriptor that is zero
// apart from the vector copied from 00F87574 (the zero vector), and
// 0077D600(entity, stop 00E08F88, &descriptor, 1) at 008A7569. Nothing is
// pushed. The image makes no null test; this skips a null entity, as the
// navigator move bodies do.
int GameScriptOrdersHost::run_navigator_stop(GameScriptOrderRow& row) {
    void* entity = argument_entity(0);
    row.unit_index = index_of(entity);
    row.unit = name_of(entity);
    static const float kZeroVector00f87574[3] = {0.0f, 0.0f, 0.0f};
    const bsp::ScriptOrderIssue order = bsp::navigator_stop_order_008a7400(kZeroVector00f87574);
    if (entity != nullptr) {
        entity_issue_command(entity, order.command, order.target, order.flags);
    }
    return 0;
}

// 008A47B0 `PilotLand(unit, site)`, packet cc9_scripted_order_natives part 2.
// Argument 0 through 00888AA0, argument 1 through ReadCommandTarget, then
// 0077D600(entity, land 00E08FA0, target, 1) at 008A4907. The receiving bots'
// 0099A170 land arm 0099A3DD takes the site from the command target; the units
// host installs at the delivery (the standing substitution of
// docs/SENTITY_INIT_ATTACH_ORDER.md 22.7) and fans a squadron out itself.
// Goal 5 chain 663: the body pilot_land_008a47b0 over PilotOrderBinding reads both
// arguments and issues; the issue below is 0077D600 and the 0099A3DD intake after it.
int GameScriptOrdersHost::run_pilot_land(GameScriptOrderRow& row) {
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    ++pilot_land_calls_;
    const auto issue = [this](void* entity, std::uint32_t cls,
                              const bsp::SceneCommandTarget& target, int flags) {
        const std::size_t leader = index_of(entity);
        if (entity == nullptr || leader >= units_.count()) {
            log_.notef("  PilotLand: unit unresolved; nothing issued (008A47B0)");
            return;
        }
        const std::size_t site_index = target.object != nullptr
            ? index_of(target.object)
            : (target.object_id > 0 ? static_cast<std::size_t>(target.object_id - 1)
                                    : ~static_cast<std::size_t>(0));
        entity_issue_command(entity, cls, target, flags);   // 008A4907
        const std::string name = name_of(entity);
        after_order_delivery([this, leader, site_index, name]() {
            if (site_index >= units_.count()) {
                log_.notef("  PilotLand: %s: the target is no unit; no task", name.c_str());
                return;
            }
            pilot_land_tasks_ += units_.land_at_site_0099a3dd(leader, site_index);
        });
        log_.notef("  PilotLand: %s -> %s (target object=%d id=%d position=%d; 008A47B0 body "
            "-> 0077D600 00E08FA0 -> 0099A3DD)", name.c_str(),
            site_index < units_.count() ? units_.unit_row(site_index)->name.c_str()
                                        : "(no unit)",
            target.object != nullptr ? 1 : 0, static_cast<int>(target.object_id),
            static_cast<int>(target.position_valid));
    };
    PilotOrderBinding binding(*this, "PilotLand", issue);
    bsp::pilot_land_008a47b0(binding);
    return 0;  // the binding pushes nothing
}

// 008A4300 `PilotRetreat(unit)`, packet cc9_plane_retreat_task (the body is
// bsp::pilot_retreat_008a4300). Argument 0 through 00888AA0, the unit's side
// (+54h), 004C7730 over world+7134h, the corner mean 008A443E as the hand-built
// descriptor's position, then 0077D600(entity, retreat 00E08F90, desc, 1) at
// 008A4532. The bots' arm 0099A45B installs at the delivery (the standing
// substitution PilotLand states). LABELLED: with no zone the image reads a null
// record; here the position stays (0, 0, 0) and the order is still issued.
int GameScriptOrdersHost::run_pilot_retreat(GameScriptOrderRow& row) {
    resolve_plane_squadron_members();
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    ++pilot_retreat_calls_;
    if (unit == nullptr || row.unit_index >= units_.count()) {
        log_.notef("  PilotRetreat: unit unresolved; nothing issued (008A4300)");
        return 0;
    }
    bsp::SceneCommandTarget target;
    target.kind = 0;            // 008A4471
    target.position_valid = 1;  // 008A445E
    target.object_id = 0;       // 008A4475
    target.object = nullptr;    // 008A446D
    target.trailing = 0.0f;     // 008A447E
    const bool zone = units_.retreat_point_008a4300(row.unit_index, target.position);
    entity_issue_command(unit, bsp::kPilotOrderClassRetreat, target,
                         bsp::kPilotOrderIssueFlags);                     // 008A4532
    const std::size_t leader = row.unit_index;
    after_order_delivery([this, leader]() {
        pilot_retreat_tasks_ += units_.retreat_0099a45b(leader, "PilotRetreat 008A4300");
    });
    log_.notef("  PilotRetreat: %s -> (%.1f, %.1f, %.1f) zone=%d (008A4300 -> 0077D600 "
        "00E08F90 -> 0099A45B, packet cc9_plane_retreat_task)", row.unit.c_str(),
        static_cast<double>(target.position[0]), static_cast<double>(target.position[1]),
        static_cast<double>(target.position[2]), zone ? 1 : 0);
    return 0;  // the binding pushes nothing
}

// 008A7060 `NavigatorEnable(unit, enable)`, packet cc9_scripted_order_natives.
// Argument 0 through 00888AA0 with no kind check, then [unit+740h] and
// MOV [EAX+11h],AL with argument 1 as a boolean. No result is pushed.
int GameScriptOrdersHost::run_navigator_enable(GameScriptOrderRow& row) {
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);
    const bool enable = argument_boolean(1);
    GameShipAiHost* ship_ai = units_.ship_ai();
    const bool stored = ship_ai != nullptr && row.unit_index < units_.count()
        && units_.unit_is_kind_of(row.unit_index, 6);
    if (stored) ship_ai->set_navigator_enabled_0011(row.unit_index, enable);
    log_.notef("  NavigatorEnable: %s enable=%d stored=%d (008A7060 [unit+740h]+11h, "
        "008759C4 gate)", row.unit.c_str(), enable ? 1 : 0, stored ? 1 : 0);
    if (!stored) {
        log_.unimplemented("MissionLuaNative::NavigatorEnable non-ship", "008a7060");
    }
    return 0;
}

int GameScriptOrdersHost::argument_integer(int index) {
    if (state_ == nullptr) return 0;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return 0;
    log_.implemented("LuaBinding::argument_integer", "00b66290");
    return static_cast<int>(lua_tonumber(state_, slot));
}

bool GameScriptOrdersHost::argument_boolean(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return false;
    log_.implemented("LuaBinding::argument_boolean", "00b66250");
    return lua_toboolean(state_, slot) != 0;
}

void* GameScriptOrdersHost::argument_ptr_field(int index) {
    // 00888d20 BSP_LuaTable_GetPtrField: the table's `Ptr` without 00888aa0's
    // entity validation. This process seeds `Ptr` with the entity's own id, so
    // the same identity comes back.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "Ptr");
    void* value = (lua_type(state_, -1) == LUA_TLIGHTUSERDATA)
        ? lua_touserdata(state_, -1) : nullptr;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaBinding::argument_ptr_field", "00888d20");
    return value;
}

// ---------------------------------------------------------------------------
// The effects
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::entity_issue_command(void* entity,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags) {
    last_issue_deferred_ = false;
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return;
    if (!logged_path_) {
        logged_path_ = true;
        log_.notef("the mission script's bindings issue through the same path an "
            "authored scene command takes: the binding hands 0077d600 a command object "
            "and the descriptor 0088a810 built, and 0077d600 builds MT_COMMAND and "
            "routes it through 00816e30's arms. This note used to end `so the command "
            "stops one hop short of a weapon director slot`, which was true before "
            "packet `entity_command_arms` projected those arms and is not true now - "
            "the run's own `reached_director` counter is the authority, and it matches "
            "`issued` (docs/ENTITY_COMMAND_ARMS.md)");
    }
    const char* name = command_name_of(command_object);
    std::string target_name;
    if (target.object != nullptr) {
        target_name = name_of(target.object);
    } else if (target.position_valid != 0) {
        target_name = "(position)";
    }
    std::string source("script:");
    source += (row_ != nullptr) ? row_->binding : std::string("navigator");
    // Packet cc9_after_row9_continuation: an order a delivery continuation
    // issues (the fan-out's wingman orders) keeps the queued order's source.
    if (row_ == nullptr && !delivery_source_.empty()) source = delivery_source_;
    if (kAfterRow9OrderQueueBound && !after_row9_poster_.empty()) {
        // Posted after row 9 (a Blackout callback): 0077D600 routes the
        // MT_COMMAND into the loopback queue, applied by the next step's pump.
        DeferredOrder o;
        o.index = index;
        o.command_object = command_object;
        o.target = target;
        o.flags = flags;
        o.source = source;
        o.target_name = target_name;
        o.poster = "blackout:" + after_row9_poster_;
        bool counted = false;
        for (auto& entry : deferred_by_poster_) {
            if (entry.first == o.poster) { ++entry.second; counted = true; break; }
        }
        if (!counted) deferred_by_poster_.emplace_back(o.poster, 1ull);
        deferred_orders_.push_back(std::move(o));
        last_issue_deferred_ = true;
        if (row_ != nullptr) {
            row_->command = name;
            row_->target = target_name;
        }
        return;
    }
    apply_issued_order(index, command_object, target, flags, source, target_name);
}

void GameScriptOrdersHost::apply_issued_order(std::size_t index, std::uint32_t command_object,
    const bsp::SceneCommandTarget& target, int flags, const std::string& source,
    const std::string& target_name) {
    const char* name = command_name_of(command_object);
    const GameCommandRow* issued = units_.issue_script_command(index, command_object,
        target, flags, source, target_name);
    if (row_ != nullptr) {
        row_->command = name;
        row_->target = target_name;
        row_->issued = issued != nullptr && issued->issued;
        row_->reached_director = issued != nullptr && issued->projected_arm;
        if (issued != nullptr) row_->blocked = issued->blocked;
    }
    if (issued != nullptr && issued->issued) ++summary_.issued;
    if (issued != nullptr && issued->projected_arm) ++summary_.reached_director;
    if (command_object == bsp::kCommandObjectAttackMove) ++summary_.attack_moves;
    if (command_object == bsp::kCommandObjectMoveTo) ++summary_.move_tos;
    bool seen = false;
    for (std::size_t existing : ordered_units_) {
        if (existing == index) { seen = true; break; }
    }
    if (!seen) {
        ordered_units_.push_back(index);
        ++summary_.units_ordered;
    }
}

// ---------------------------------------------------------------------------
// Packet cc8_navigator_path: 008a3600 and its two companions
// ---------------------------------------------------------------------------

// 00B663F0 and 00B66270. Both already exist on this host for the nine rows of
// packet cc_lua_binding_audit; these two names are the navigator host's spelling
// of the same two reads, so they forward rather than duplicate the stack logic.
int GameScriptOrdersHost::argument_count() { return count(); }

float GameScriptOrdersHost::argument_number(int index) {
    return static_cast<float>(get_number(index));
}

// *(*(entity+538h)+500h) at 008a3724, which is exactly
// GameUnitsHost::unit_class_max_speed_0500.
float GameScriptOrdersHost::entity_class_max_speed(void* entity) {
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return 0.0f;
    return units_.unit_class_max_speed_0500(index);
}

// 0075B430(5Bh) at 008a387f + 0077c2a0 at 008a38d0.
//
// This host does not carry session messages, so the message is not transported;
// what is reproduced is the receiver's net effect, which packet cc8_navigator_path
// read whole at 00721a9d-00721ae6: 00720fa0 re-resolves the path entity from the
// uint16 at msg+20h, 00465080 builds a descriptor from it, and the director's
// vtable[60h] queues 00E08F80 `moveonpath` against that descriptor.
//
// Two differences from the executable, both holes and neither a proof:
//
//  1. The ENTRY POINT. The executable reaches the director through its own
//     vtable[60h] (008358d0, the queue). This host has one command-issue seam,
//     the projected 0077d600 / 00816e30 path, and uses it. 00816e30's moveonpath
//     arm at 00816f7d exists (docs/ENTITY_COMMAND_ARMS.md), so the destination
//     slot is the same one; the route to it is not.
//  2. The FOLLOW MODE has nowhere to land. 0071c1b0 (body 0071c1b0-0071c1dd,
//     read whole) walks [director+54h + i*1Ch] for the first i < 10 whose dword
//     is zero and writes msg+24h to [[director+1a0h + i*4] + 8h] and msg+28h to
//     +0Ch - a pair, not the single value docs/DIRECTOR_UPDATE_ARMS.md records.
//     Both land on the slot's PATH OBJECT, which src/game_hosts_commands.cpp's
//     command_count already says "this process does not build". So the pair is
//     recorded and reported, not stored.
void GameScriptOrdersHost::session_route_path_order_message(void* entity,
    const bsp::NavigatorPathOrder& order) {
    if (!logged_path_order_) {
        logged_path_order_ = true;
        log_.notef("luaMW_NavigatorMoveOnPath 008a3600 routes a 5Bh session message "
            "rather than calling 0077d600 like the other four navigator rows. The "
            "receiver 00721a40's 5Bh arm at 00721ab6 queues 00E08F80 `moveonpath` "
            "against a descriptor built from the path entity, which is what this host "
            "issues; 0071c1b0's follow-mode pair needs the slot's path object that "
            "0071f600 would build, and this process does not build it");
    }
    // 00465080 BSP_CommandTarget_FromEntity over the entity 00720fa0 resolved.
    // The descriptor the receiver builds carries the PATH entity, not the unit.
    void* path_entity = path_entity_for_order_;
    bsp::SceneCommandTarget target{};
    if (path_entity != nullptr) {
        target.kind = 1;
        target.position_valid = 0;
        target.object = path_entity;
        target.object_id = order.path_object_id;
    }
    entity_issue_command(entity, bsp::kCommandMoveOnPath, target,
        bsp::kNavigatorPathOrderRouteFlags);
    ++summary_.path_orders;
    // Packet cc8_ship_moveonpath fills difference 2 above. 0071C1B0's pair now
    // has a slot object to land on, and 0071F600's build (0071F6A5-0071F885)
    // turns the authored Path entity's points into the cursor 009E59C0 reads.
    // The order is the executable's: the queue of step 3, then 0071C1B0 at
    // 00721ADB, and the build only when the command is begun, which is why the
    // build reads the pair rather than being handed it.
    // Packet cc9_after_row9_continuation: the receiver makes the pair and the
    // build on delivery (00721ADB), so a queued order carries them to the drain.
    const auto follow_mode = order.follow_mode;
    const auto path_parameter = order.path_parameter;
    after_order_delivery([this, entity, path_entity, follow_mode, path_parameter]() {
    const std::size_t index = index_of(entity);
    if (index < units_.count()) {
        units_.commands().set_path_follow_pair_0071c1b0(index, follow_mode,
                                                        path_parameter);
        // The marker registry first, not name_of(). A Path entity is always a
        // scene marker, and marker ids are handed out from the unit count as it
        // stood at scene load (21 on USN04); by the time the script issues these
        // orders SpawnNew has taken the count past that, so name_of()'s first
        // branch resolves marker id 22 to unit index 21 and answers a plane's
        // name. `local/mop_after2_usn04.log` printed exactly that:
        // `no authored points for path "D3A Val #1.1"`.
        const SceneMarker* marker
            = marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(path_entity)));
        const std::string path_name = marker != nullptr ? marker->name
                                                        : name_of(path_entity);
        const bsp::game::ScenePathEntry* authored
            = bsp::game::scene_path_registry().find(path_name);
        float ux = 0.0f, uy = 0.0f, uz = 0.0f;
        units_.unit_position_00fc(index, ux, uy, uz);
        static const std::vector<std::array<float, 3>> kNoPoints;
        const bool built = units_.commands().begin_path_command_0071f600(index, path_name,
            authored != nullptr ? authored->points_world : kNoPoints, ux, uz);
        if (!built && !logged_path_points_missing_) {
            logged_path_points_missing_ = true;
            log_.notef("moveonpath path build 0071f600 found no authored points for "
                "path \"%s\": 007ADC30 then answers `no legs` on the first state step "
                "and the ship holds its heading", path_name.c_str());
        }
        // Packet cc13-moveonpath1 (docs/CC13_MOVEONPATH_TASK.md): on a plane, the
        // pilot bot's 0099A170 now reads `moveonpath` as the current command and its
        // arm at 0099A24A builds the kind-6 task (009BDBB0). The units host
        // constructs and ticks it (run_moveonpath_task_arm_009bd990).
        if constexpr (bsp::kMoveOnPathTaskBound) {
            if (units_.unit_is_kind_of(index, 0x0F)) {   // kUnitKindQueryPlane
                ScriptOrderAttackCommandHost bot_host(units_, log_, bsp::kCommandMoveOnPath, 0u);
                if (bsp::bot_install_command_task_0099a170(
                        static_cast<std::uint32_t>(index + 1u), bot_host) != 0u) {
                    units_.store_unit_attack_command_class(index, bsp::kCommandMoveOnPath);
                }
            }
        }
    }
    });
}

// Packet cc9_shipyard_create_unit (GUNNERY 136 (D), docs/SQUADRON_LAND_TASK.md 5ek).
// 00844FC0 issues `moveonpath` itself (008454B4..008454C9) and makes no 0071C1B0
// call, so the slot's follow pair is whatever the issue leaves.
// SUBSTITUTION (labelled): the path build 0071F600, which the director runs when it
// begins the command, is made on delivery, as the navigator route above does.
bool GameScriptOrdersHost::issue_shipyard_moveonpath_008454b4(std::size_t unit_index,
    const std::string& path_name) {
    if (unit_index >= units_.count() || path_name.empty()) return false;
    const SceneMarker* marker = nullptr;
    for (const SceneMarker& m : markers_) {
        if (m.name == path_name) {
            marker = &m;
            break;
        }
    }
    if (marker == nullptr) return false;
    void* const entity = reinterpret_cast<void*>(static_cast<std::uintptr_t>(unit_index + 1));
    void* const path_entity = reinterpret_cast<void*>(static_cast<std::uintptr_t>(marker->id));
    bsp::SceneCommandTarget target{};
    target.kind = 1;
    target.position_valid = 0;
    target.object = path_entity;
    // +174h, entity_object_id's stand-in. The image's +174h is a 16-bit handle;
    // this host's marker ids are not: load-time markers (50000 + n) fit, but a
    // GenerateObject marker (90000 + n) would wrap modulo 65536 and could name
    // another marker in scene_marker_position. Such an id is posted as 0 (no
    // object id) instead of wrapped. Only the generated LandConvoy markers are
    // above 0xFFFF, so no authored Path takes that arm (SHIP_AI_OPEN_ITEMS 295.4).
    if (marker->id > 0 && marker->id <= 0xFFFF) {
        target.object_id = static_cast<std::uint16_t>(marker->id);
    } else {
        target.object_id = 0;
        log_.notef("shipyard moveonpath 008454b4: marker \"%s\" id %d does not fit the "
            "16-bit +174h, posted as object id 0 rather than wrapped", marker->name.c_str(),
            marker->id);
    }
    entity_issue_command(entity, bsp::kCommandMoveOnPath, target, 1);   // 008454B4 PUSH 1
    const std::string name = marker->name;
    after_order_delivery([this, unit_index, name]() {
        if (unit_index >= units_.count()) return;
        const bsp::game::ScenePathEntry* authored = bsp::game::scene_path_registry().find(name);
        float ux = 0.0f, uy = 0.0f, uz = 0.0f;
        units_.unit_position_00fc(unit_index, ux, uy, uz);
        static const std::vector<std::array<float, 3>> kNoPoints;
        units_.commands().begin_path_command_0071f600(unit_index, name,
            authored != nullptr ? authored->points_world : kNoPoints, ux, uz);
    });
    return true;
}

// The pair at *(entity+73Ch)+24h and +28h, 008a3901 and 008a3912. The same
// store luaMW_SetShipSpeed 00890d30 makes, which this host already owns.
void GameScriptOrdersHost::entity_store_commanded_speed(void* entity, float speed) {
    // 008A3E70 (PilotMoveOnPath) has no 00890E6F store.
    if (pilot_path_no_speed_) return;
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return;
    units_.store_commanded_speed_00890e6f(index, speed);
    ++summary_.commanded_speed_stores;
}

// *(entity+738h), read at 008a3c60 and 008a3df6. This host holds one GameDirector
// per unit slot and keys it by unit index, so the unit handle identifies the
// director; nothing here dereferences +738h.
void* GameScriptOrdersHost::entity_weapon_director(void* entity) {
    const std::size_t index = index_of(entity);
    return (index < units_.count()) ? entity : nullptr;
}

// 00835a40 (selector 9) and 00835940 (selector 7), both building 0075B430(5Ah)
// with vtable 00CFD9C4 and routing through 0077c2a0 with flags 7.
//
// The send side is complete: both senders were read whole and the two selectors
// are the only thing that differs between them. The RECEIVE side is the hole.
// 00721a40's 5Ah arm at 00721a93 hands the message to director->vtable[38h] =
// 00835640 over 0071c1e0, and neither is projected in this process. The block
// the value would land in does exist - bsp::ShipAiAvoidanceRequest's enable_3f4
// (blk+3ECh) - and its two setters 009dabb0 (torpedo) and 009dabd0 (land) are
// the ones include/bsp/ship_ai_avoidance_request.hpp marks Unused, i.e. with no
// caller found. So this is a store with a named consumer that nothing calls yet.
//
// Packet cc9_navigator_avoidance (docs/SHIP_AI_OPEN_ITEMS.md section 88). True: the
// message is delivered to the receiver this host already has, 00835640's arms
// (GameUnitsHost::apply_director_avoidance_message_00835640): sub-kind 7 stores
// director+240h (torpedo evasion), 9 stores +242h (land collision), which the
// bound ship AI readers 009DA231 / 009DA6FB.. / 009EFCBD consult. The director
// constructor 008366D0 seeds all three bytes to 1 (EBX = 1 at 008366F4, stored at
// 00836724..00836730), so only a `false` moves anything. Delivered at once
// (SUBSTITUTION, labelled: the 0077C2A0 route is a loopback here, as for the
// other 5Ah senders). False: counted only, as before. ON by section 88.2: only
// USN02 moves (its four torpedo-evasion disables); every other row identical.
inline constexpr bool kNavigatorAvoidanceDeliveryBound = true;

// Packet cc9_navigator_ship_avoidance (docs/SHIP_AI_OPEN_ITEMS.md section 90).
// 008A3970 NavigatorSetAvoidShipCollision sends selector 8 through 008359C0
// (00835A0A), which 00835640 stores into director+241h (00835668). The bound
// readers of that byte are 009EC787 (009EC770, the clearance category gate),
// 009EF37B (009EF350, the traffic pass) and 009F106C (009F0EA0, the neighbour
// refresh); each reads GameDirectorAvoidance::ship. The constructor seeds it to
// 1 (0083672A), so only a `false` changes anything. True: delivered at once, the
// same loopback SUBSTITUTION as the two setters above. False: counted only (the
// row is routed here in both builds, so the binding no longer reports as an
// unimplemented native). ON by section 90.2: USN16's six Taffy carriers take the
// `false`; 009DD010 stops running and Kitkun Bay's traffic writes drop 255 -> 0,
// gameplay identical (their OFF traffic terms were all zero). Every other row
// makes no call.
inline constexpr bool kNavigatorShipAvoidanceDeliveryBound = true;

void GameScriptOrdersHost::session_route_avoidance_message(void* director,
    int selector, bool enabled) {
    if (selector == bsp::kNavigatorAvoidanceSelectorShipCollision) {
        ++summary_.ship_avoidance_orders;
        if (!enabled) ++summary_.ship_avoidance_disables;
        const std::size_t ship_index = index_of(director);
        if (avoidance_logged_ < 60) {
            ++avoidance_logged_;
            const GameUnitRow* row =
                ship_index < units_.count() ? units_.unit_row(ship_index) : nullptr;
            log_.notef("navigator avoidance order: unit=%s ship=%d bound=%d (008359C0 -> "
                "00835640 +241h)", row != nullptr ? row->name.c_str() : "?", enabled ? 1 : 0,
                kNavigatorShipAvoidanceDeliveryBound ? 1 : 0);
        }
        if constexpr (kNavigatorShipAvoidanceDeliveryBound) {
            bsp::DirectorCommandMessage message;
            // Wire 008359C0 calls director_command_message_0071da50; OFF keeps the literal header.
            if constexpr (kWireScriptOrdersHostBound) {
                message = bsp::director_command_message_0071da50(
                    bsp::DirectorCommandSubKind::ShipCollisionAvoidance, enabled);
            } else {
            message.base_kind = 0x5a;
            message.vtable = 0x00cfd9c4u;
            message.sub_kind = selector;
            message.value = enabled ? 1u : 0u;
            }
            if (ship_index < units_.count()
                && units_.apply_director_avoidance_message_00835640(ship_index, message)) {
                ++summary_.ship_avoidance_delivered;
            }
        } else {
            record_unimplemented("Navigator::avoidance_receiver_ship", "00835640");
        }
        return;
    }
    const bool land = selector == bsp::kNavigatorAvoidanceSelectorLandCollision;
    if (land) {
        ++summary_.land_avoidance_orders;
    } else {
        ++summary_.torpedo_evasion_orders;
    }
    if (!enabled) ++summary_.avoidance_disables;
    const std::size_t index = index_of(director);
    if (avoidance_logged_ < 60) {
        ++avoidance_logged_;
        const GameUnitRow* row = index < units_.count() ? units_.unit_row(index) : nullptr;
        log_.notef("navigator avoidance order: unit=%s %s=%d bound=%d (00835940 / 00835A40 "
            "-> 00835640)", row != nullptr ? row->name.c_str() : "?",
            land ? "land" : "torpedo", enabled ? 1 : 0,
            kNavigatorAvoidanceDeliveryBound ? 1 : 0);
    }
    if constexpr (kNavigatorAvoidanceDeliveryBound) {
        bsp::DirectorCommandMessage message;
        // Wire 00835940 calls director_command_message_0071da50; OFF keeps the literal header.
        if constexpr (kWireScriptOrdersHostBound) {
            message = bsp::director_command_message_0071da50(
                static_cast<bsp::DirectorCommandSubKind>(selector), enabled);
        } else {
        message.base_kind = 0x5a;
        message.vtable = 0x00cfd9c4u;
        message.sub_kind = selector;
        message.value = enabled ? 1u : 0u;
        }
        if (index < units_.count()
            && units_.apply_director_avoidance_message_00835640(index, message)) {
            ++summary_.avoidance_delivered;
        }
    } else {
        record_unimplemented(land ? "Navigator::avoidance_receiver_land"
            : "Navigator::avoidance_receiver_torpedo", "0071c1e0");
    }
}

// Packet cc9_parts_land_avoidance (docs/SHIP_AI_OPEN_ITEMS.md section 167, from
// 163 item 3). 0092BD00 (0092BD00-0092BD24, read whole), __thiscall(controller)
// with ECX = 0080E490(unit) = [unit+1018h], on the disable side of 008A3B10 only:
// the hull body [ctl+2Ch] (no null test), its first shape 00C31DC0, then for
// every shape (next at +208h) 00C48020(0Dh), which stores the mask shape+30h =
// 0Dh and clears the body's manifolds (00C43AA0). The hull build gave that mask
// 0Dh | the class bit (009394A9..009395E2), so the store drops the class bit,
// the same effect as 0092BD70(0) (GameUnitsHost::set_hull_class_bit_0092bd70,
// packet cc9_avoid_zone_draft_bodies): the draft bodies that pull hulls through
// the class bit stop catching this hull. True: that accessor with alse.
// SUBSTITUTIONS, labelled: a sunk hull whose terrain bit 8 was cleared
// (00C47F60 at 0082643B) gets bit 8 back from the 0Dh store in the image, not
// here; the manifold clear is not modelled (the next contact phase rebuilds
// them). False: a record, as before.
inline constexpr bool kNavigatorPartsLandAvoidanceBound = true;   // ON: SHIP_AI 167.1

void GameScriptOrdersHost::unit_parts_land_avoidance_disabled(void* entity) {
    const std::size_t index = index_of(entity);
    if (!kNavigatorPartsLandAvoidanceBound || index >= units_.count()) {
        record_unimplemented("Navigator::parts_land_avoidance_disabled", "0092bd00");
        return;
    }
    units_.set_hull_class_bit_0092bd70(index, false);
    ++parts_land_avoidance_disables_;
    const GameUnitRow* row = units_.unit_row(index);
    log_.notef("navigator land avoidance off: unit=%s hull mask -> 0Dh, class bit dropped at "
        "%.2f s (008A3C79 -> 0092BD00 -> 00C48020, packet cc9_parts_land_avoidance)",
        row != nullptr ? row->name.c_str() : "?", static_cast<double>(mission_clock_));
    log_.implemented("Navigator::parts_land_avoidance_disabled", "0092bd00");
}

bool GameScriptOrdersHost::entity_command_is_available(void* entity,
    const char* command_name, void* target) {
    // Packet cc8_ship_follow read 008162B0 and the routine its follow arm defers
    // to, 00779D50, whole. The old note here said answering true would invent the
    // predicate; it is now transcribed instead, and this is the script path's
    // copy of the same answer the AI path already gives.
    // docs/SHIP_UNIT_GROUP_FOLLOW.md section 8.
    if (!logged_predicate_) {
        logged_predicate_ = true;
        log_.notef("JoinFormation's first question is answered now: 0077C8D0 asks the "
            "follower's vtable 16Ch (008162B0 for MDestroyer), whose follow arm is entirely "
            "008162BF's call to 00779D50 - a live ship may follow a live ship of its own "
            "side. The +188h OwnerPlayer arm is skipped, this process has no producer for "
            "it (docs/SHIP_UNIT_GROUP_FOLLOW.md section 8)");
    }
    const std::size_t follower_index = index_of(entity);
    const std::size_t leader_index = index_of(target);
    const bool token_is_follow = command_name != nullptr
        && std::strcmp(command_name, "follow") == 0;   // 00779D68, 00CFB52C
    if (follower_index >= units_.count() || leader_index >= units_.count()) {
        ++summary_.formations_refused;
        log_.implemented("Formation::command_is_available", "0077c8fe");
        return false;                                   // 00779D76
    }
    bsp::EntityFollowFacts facts;
    facts.follower_flag_005d = units_.unit_flag_005d(follower_index);
    facts.target_present = true;
    facts.target_flag_005d = units_.unit_flag_005d(leader_index);
    facts.target_kind_02 = units_.unit_is_kind_of(leader_index, 0x02);
    facts.follower_kind_06 =
        units_.unit_is_kind_of(follower_index, bsp::kUnitGunneryKindShipBase);
    facts.follower_kind_08 = units_.unit_is_kind_of(follower_index, 0x08);
    facts.target_kind_06 = units_.unit_is_kind_of(leader_index, bsp::kUnitGunneryKindShipBase);
    facts.target_kind_08 = units_.unit_is_kind_of(leader_index, 0x08);
    facts.follower_party_0054 = units_.unit_side_0054(follower_index);
    facts.target_party_0054 = units_.unit_side_0054(leader_index);
    const std::int32_t follower_group = units_.unit_formation_group_0284(follower_index);
    facts.same_entity_or_group_00779820 =
        (follower_index == leader_index)
        || (follower_group >= 0
            && follower_group == units_.unit_formation_group_0284(leader_index));
    facts.owner_player_known = false;                   // +188h has no producer here
    const bool available = bsp::entity_may_follow_target_00779d50(facts, token_is_follow);
    if (!available) ++summary_.formations_refused;
    log_.implemented("Formation::command_is_available", "0077c8fe");
    return available;
}

int GameScriptOrdersHost::entity_route_slot(void* entity) {
    // 0077C904 reads entity+1ACh, the first CURRENT role assignment,
    // not the +188h permission or the party. Borrow the table whose
    // constructor/assignment receivers already retain that exact field.
    std::int32_t slot = -1;
    if (!units_.unit_current_role_slot(index_of(entity), 0, slot)) return -1;
    log_.implemented("Formation::entity_route_slot", "0077c904");
    return slot; // entity_join_formation keeps the native unsigned <8 gate.
}

void GameScriptOrdersHost::slot_counter_increment(int slot) {
    // Phase5 attaches the SAME result/scoring owner before any mission
    // script can issue JoinFormation; no detached counter is introduced.
    if (scoring_name_owner_ == nullptr)
        throw std::logic_error("formation-use scoring requires the mission scoring owner");
    scoring_name_owner_->increment_formation_use(slot);
}

// Packet cc9_formation_join_loopback (docs/SHIP_AI_OPEN_ITEMS.md section 92).
// 0077C964 routes the 76h join through 0077C2A0 with flags 7, which on a local
// session appends it to the loopback vector (0077C44D -> 0076E520,
// docs/SESSION_MESSAGE_DISPATCH.md) behind the scene's authored MT_COMMANDs
// (0077D600, flags 1). The pump's drain 0076C600 delivers them in post order: the
// authored `cruise` clears the slots and becomes current, then 0077F940 -> 00720CD0
// clears again and makes `follow` current, which is what 009F5DE0 compares. This
// host joined at the call, before the authored commands arrived, so the cruise
// (flags 1, 0081733E clear-all) replaced the follow and 009F5DEB released every
// scripted follower at t=0.05. True: the join is posted into the commands host's
// loopback vector and runs at its turn. False: joined at the call, as before.
// ON by section 92.5: the t=0.05 leaves go to 0 in all eight reach rows (exit 3),
// both controls exit 1, and JM08's StartInvasion fires (first HQ latch 538.85 s).
inline constexpr bool kFormationJoinLoopbackBound = true;

void GameScriptOrdersHost::session_route_formation_message(void* follower,
    std::uint16_t leader_object_id) {
    // 0077C964 routes a type-76h message whose only payload is the leader's
    // 16-bit id; 0077FE80's arm for that type is four instructions - resolve the
    // id and call 0077F940 - and in this process the route is local, so the join
    // runs here. The wire hop is what is not modelled, not the merge.
    // docs/SHIP_UNIT_GROUP_FOLLOW.md section 3.
    const std::size_t follower_index = index_of(follower);
    // entity_object_id above stands the unit index + 1 in for entity+174h, so the
    // receiver's 00521E30 lookup inverts to the same substitution.
    const std::size_t leader_index = static_cast<std::size_t>(leader_object_id) - 1u;
    if (follower_index >= units_.count() || leader_object_id == 0
        || leader_index >= units_.count()) {
        log_.unimplemented("Formation::route_join_message", "0077c964");
        return;
    }
    // Goal 4 chain 260: 0077C964 -> 0076E520 writes the 76h (0075C7C0) and
    // the factory rebuilds it (0075A2E0, 0075C810 read); 0077FE80's arm
    // resolves the copy's +20h, so the delivery joins the copy's leader.
    const std::size_t copy_leader =
        bsp::game::commands_formation_member_copy_0076e520(0x76, leader_index);
    if (copy_leader >= units_.count()) {
        log_.unimplemented("Formation::route_join_message", "0077c964");
        return;
    }
    if constexpr (kFormationJoinLoopbackBound) {
        ++summary_.formation_joins_posted;
        const bool posted = bsp::game::commands_post_loopback_callback_0076e520(
            follower_index, [this, follower_index, copy_leader] {
                ++summary_.formation_joins_delivered;
                if (units_.formation_join_0077f940(follower_index, copy_leader)) {
                    ++summary_.formations_joined;
                }
            });
        if (posted) {
            log_.implemented("Formation::route_join_message", "0077c964");
            return;
        }
    }
    if (units_.formation_join_0077f940(follower_index, copy_leader)) {
        ++summary_.formations_joined;
    }
    log_.implemented("Formation::route_join_message", "0077c964");
}

void GameScriptOrdersHost::entity_set_skill_level(void* entity, int level) {
    if (row_ != nullptr) row_->skill_level = level;
    ++summary_.skills;
    // entity->vtable[128h] at 0089539a. Packet cc9_difficulty: for a plane that
    // is 006D1EB0 -> 007B8AE0, the pilot bot's index (bot+34h), and 009565A0,
    // unit+390h; a ship reaches 009565A0 through 0080DF90. The host keeps both
    // as the slot's one index. Its readers: the dive-bomb approach's capture
    // (009F9D22). The weapon director's and child objects' re-skill that
    // 009565A0 also performs is not modelled (the gun bots read units.skill_level() live, so 00727E70's bot+34h fan-out is in effect; GUNNERY_OPEN_ITEMS 212).
    const std::size_t index = index_of(entity);
    if (!kSkillLevelBound || index >= units_.count()) {
        log_.unimplemented("UnitInstance::set_skill_level", "0089539a");
        return;
    }
    units_.set_skill_level_007b8ae0(index, level);
    log_.notef("SetSkillLevel unit=%zu level=%d", index, level);
    log_.implemented("UnitInstance::set_skill_level", "0089539a");
}

bool GameScriptOrdersHost::entity_is_kind_of(void* entity, int class_id) {
    // entity->vtable[5Ch] at 008ad44f, the IsKindOf the corpus uses everywhere.
    // This process runs it over the recovered class chain of the created
    // instance, the same one 0068aca0's 20h arm and the minimap walk use.
    const std::size_t index = index_of(entity);
    log_.implemented("UnitInstance::is_kind_of", "008ad44f");
    if (index >= units_.count()) return false;
    return units_.unit_is_kind_of(index, class_id);
}

void GameScriptOrdersHost::entity_set_repair_enabled_field(void* entity, bool enabled) {
    static_cast<void>(entity);
    // The byte at entity+378h, 008ad4e8. No recovered field of a created
    // instance covers it, so the write is recorded.
    log_.unimplemented("UnitInstance::set_repair_enabled_field", "008ad4e8");
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
}

void GameScriptOrdersHost::session_route_repair_enable_message(void* entity, bool enabled) {
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
    // Packet cc9_repair_enable_route (GUNNERY_OPEN_ITEMS 82): 008AD48B builds message
    // 9Fh with the flag at +1Ch and 0077C2A0 (008AD4CD, flags 7) routes it to the
    // entity; 00821E80's 9Fh arm (008220D7) calls 00939FD0 on the repair task at
    // unit+A20h. The gunnery host holds the task.
    const std::size_t index = index_of(entity);
    GameGunneryHost* gunnery = units_.gunnery();
    {
        // Goal 4 chain 260: 0076E520's copy of the 9Fh (00760A50 construct,
        // 00760AE0 write, factory case 9Fh 00760A50, 00760B30 read); 00821E80's
        // 9Fh arm hands the copy's +1Ch to 00939FD0. UNCERTAIN, as for 79h:
        // delivered here at the route, where the image delivers at the drain.
        const bsp::HostSessionMessages* messages = bsp::game::commands_session_messages();
        const bool copy_enabled =
            messages != nullptr ? messages->queue_repair_body_9f(enabled) : enabled;
        if (gunnery != nullptr && index < units_.count()) {
            gunnery->set_hull_repair_enabled_00939fd0(index, copy_enabled);
        }
    }
    if constexpr (kHullRepairEnableRouteBound) {
        log_.implemented("Session::route_repair_enable_message", "008ad4cd");
    } else {
        log_.unimplemented("Session::route_repair_enable_message", "008ad4cd");
    }
}

int GameScriptOrdersHost::game_session_mode() {
    // [00e188a8]+1fe4h, the field every other host in this executable reports as
    // the single-player value.
    log_.implemented("Game::session_mode", "008ab9d1");
    return 0;
}

// Goal 5 chain 655. The dispatch has checked that argument 0 names a units-host slot.
int GameScriptOrdersHost::run_role_table_query(std::uint32_t address) {
    if (state_ == nullptr) return 0;
    RepairBindingArgs args(*this);
    RoleTableBinding binding(state_, units_, [this](void* entity) { return index_of(entity); });
    namespace seg56b = bsp::cc13_left55::seg56b;
    return address == 0x008abf90u
        ? seg56b::lua_get_role_owner(args, binding, binding, binding)       // 008ABF90
        : seg56b::lua_get_role_available(args, binding, binding, binding);  // 008ABAC0
}

// Goal 5 chain 655. The dispatch has checked the unit and the point.
int GameScriptOrdersHost::run_turn_to_position() {
    TurnToPositionArgs args(*this,
        [this](int index, float (&out)[3]) { return argument_vector3(index, out); });
    TurnToPositionBinding binding(units_, [this](void* entity) { return index_of(entity); },
        [this] { return static_cast<std::int32_t>(game_session_mode()); });
    const int results = bsp::cc13_left55::seg56a::lua_entity_turn_to_position(args, binding,
        binding);
    turn_to_position_posed_ += binding.posed();
    return state_ != nullptr ? results : 0;
}

int GameScriptOrdersHost::run_seg56_unit_query(std::uint32_t address) {
    RepairBindingArgs args(*this);
    UnitQueryResults results([this](int value) { push_number(value); },
        [this](float value) { push_number_float_00b66480(value); });
    UnitQuerySeg56Host host(units_, [this](void* entity) { return index_of(entity); },
        [this] { return static_cast<std::int32_t>(game_session_mode()); });
    namespace seg56 = bsp::seg56_options_cc13;
    int count = 0;
    switch (address) {
    case 0x008953f0u: count = seg56::lua_get_skill_level(args, results, host); break;
    case 0x008950d0u: count = seg56::lua_get_crew_level(args, results, host); break;
    case 0x00890bb0u: count = seg56::lua_get_ship_max_speed(args, results, host); break;
    case 0x00897720u: count = seg56::lua_set_air_base_plane_limit(args, host); break;
    case 0x008978d0u: count = seg56::lua_get_air_base_plane_limit(args, results, host); break;
    case 0x00896ee0u: count = seg56::lua_remove_shipyard_stock(args, host); break;
    case 0x00896220u: {
        AirSlotStateView slots;
        count = seg56::lua_get_air_base_slot_status(args, results, slots, host);
        break;
    }
    default: unit_query_unreached("a row with no segment-56 unit query body");
    }
    return state_ != nullptr ? count : 0;
}

int GameScriptOrdersHost::game_effective_game_mode() {
    // 004bca50 at 008ab9de. game+614h and game+61Ch resolve a single-player
    // campaign load to 8, which is the value the scene contents pass already
    // runs its gate on.
    log_.implemented("Game::effective_game_mode", "004bca50");
    return 8;
}

void GameScriptOrdersHost::role_owner_set_role_available(void* owner, int role, int value) {
    static_cast<void>(owner);
    // owner->vtable[148h] = 0077F360 -> 00927D20 (packet
    // cc9_player_role_bookkeeping): the units host writes the permission words
    // when kPlayerRoleBookkeepingBound is on, and ignores the call otherwise.
    if (row_ != nullptr && row_->unit_index < units_.count()) {
        units_.set_role_availability_00927d20(row_->unit_index,
            static_cast<std::uint32_t>(role), value);
    }
    log_.unimplemented("RoleOwner::set_role_available", "008aba51");
    if (row_ != nullptr) {
        row_->role = role;
        row_->role_value = value;
    }
    ++summary_.roles;
}

void GameScriptOrdersHost::session_route_role_message(void* owner, int role, int value) {
    static_cast<void>(owner);
    static_cast<void>(role);
    static_cast<void>(value);
    log_.unimplemented("Session::route_role_message", "008aba2f");
}

void GameScriptOrdersHost::game_assign_party_player_slots(int value) {
    static_cast<void>(value);
    log_.unimplemented("Game::assign_party_player_slots", "008aba60");
}

// ---------------------------------------------------------------------------
// The dispatch
// ---------------------------------------------------------------------------

int GameScriptOrdersHost::dispatch(lua_State* state, const char* binding_name,
    int argument_count) {
    const ScriptOrderBinding* binding = find_binding(binding_name);
    if (binding == nullptr) return 0;
    // CreateScript runs the named global inside its own body, and that global
    // calls further bindings, so a dispatch can re-enter this function. The three
    // per-call fields are saved and restored rather than cleared at the end.
    // Packet cc13-dispguard: the restore is a guard's destructor, so a C++
    // exception out of a native also restores the outer values. The row lives
    // in rows_in_flight_ (a std::list, so its address is stable), not in this
    // frame. A Lua error is a longjmp (Lua 5.1 is built as C), and under /EHsc
    // the destructor is not guaranteed to run for it. When it is skipped,
    // row_, state_ and argument_count_ all stay at the inner dispatch's
    // values, and its node is not erased, so rows_in_flight_ grows by one per
    // escaped error. row_ then points at that live heap row, never at a dead
    // stack frame, and the growth is bounded by the error count: harmless.
    struct DispatchRestore {
        GameScriptOrdersHost& host;
        lua_State* const outer_state;
        const int outer_argument_count;
        GameScriptOrderRow* const outer_row;
        const std::list<GameScriptOrderRow>::iterator in_flight;
        ~DispatchRestore() {
            host.row_ = outer_row;
            host.state_ = outer_state;
            host.argument_count_ = outer_argument_count;
            host.rows_in_flight_.erase(in_flight);
        }
    };
    const DispatchRestore restore{*this, state_, argument_count_, row_,
        rows_in_flight_.emplace(rows_in_flight_.end())};
    state_ = state;
    // Packet cc13-dispguard: the machine is the main thread only. lua_pushthread
    // answers 1 for it (no allocation, no GC step); a native run on a coroutine
    // leaves the kept pointer as it was, so a collected coroutine is never kept.
    // Evidence that natives run on the main thread: src/ has no lua_newthread
    // or lua_resume, and none of this installation's 621 .lua files mentions
    // coroutine or newthread. A later script that does use coroutines makes
    // the readers see the last main-thread state instead of the coroutine:
    // an intended safety change, but a behavioural one.
    if (state == nullptr) {
        machine_state_ = state;
    } else {
        const bool main_thread = lua_pushthread(state) == 1;
        lua_pop(state, 1);
        if (main_thread) machine_state_ = state;
    }
    argument_count_ = argument_count;
    if (!g_lua_callback_trace.installed && lua_callback_trace_enabled()) {
        g_lua_callback_trace.installed = true;
        g_lua_callback_trace.clock = &mission_clock_;
        lua_sethook(state, lua_callback_trace_hook, LUA_MASKCALL, 0);
    }
    // Packet cc9_recon_level_table: bring the unit tables' `reconlevel` up to
    // the last recon pass before the native reads anything.
    if (kReconLevelTableBound) sync_recon_level_tables_0077b0c0();

    GameScriptOrderRow& row = *restore.in_flight;
    row.binding = binding->name;
    row.row_address = binding->row_address;
    row_ = &row;

    char address[16];
    format_address(binding->row_address, address);
    char method[96];
    std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding->name);
    log_.implemented(method, address);

    // DoVector3 and the day-time bindings read numbers only. A table converts
    // to zero without reading ID (lua_getfield would invoke __index).
    void* subject = (std::strcmp(binding->name, "DoVector3") == 0
        || std::strcmp(binding->name, "SetDayTime") == 0
        || std::strcmp(binding->name, "GetDayTime") == 0)
        ? nullptr : entity_from_argument(0);
    if (subject == nullptr && std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        subject = argument_ptr_field(0);
    }
    row.unit_index = index_of(subject);
    row.unit = name_of(subject);

    int results = 0;
    if (std::strcmp(binding->name, "SetDayTime") == 0
        || std::strcmp(binding->name, "GetDayTime") == 0) {
        bsp::WeatherConfig* const weather = global_subsystems_ != nullptr
            ? global_subsystems_->weather() : nullptr;
        if (weather == nullptr) {
            log_.unimplemented("LuaNative::day-time without game+21C4h weather owner", address);
        } else {
            ScriptDayTimeArgs args(*this, state_);
            ScriptDayTimeBinding clock(*weather, state_);
            results = std::strcmp(binding->name, "SetDayTime") == 0
                ? bsp::cc13_seg56::lua_set_day_time_008b1cc0(args, clock)
                : bsp::cc13_seg56::lua_get_day_time_008b1ec0(clock, clock);
            if (state_ == nullptr) results = 0;
        }
    } else if (std::strcmp(binding->name, "PilotSetTarget") == 0) {
        results = run_pilot_set_target(row);
    } else if (std::strcmp(binding->name, "PlaneForceRelease") == 0 ||
               std::strcmp(binding->name, "SquadronForceRelease") == 0 ||
               std::strcmp(binding->name, "PilotBomb") == 0 ||
               std::strcmp(binding->name, "PlaneBombFallingTime") == 0 ||
               std::strcmp(binding->name, "PlaneReloadBombPlatforms") == 0 ||
               std::strcmp(binding->name, "GetPlaneIsReloading") == 0) {
        // Packet cc13-fix16. handles() refuses each row while its switch is off.
        results = run_bomb_release_native(row, binding->name);
    } else if (std::strcmp(binding->name, "PilotGunFire") == 0 ||
               std::strcmp(binding->name, "PilotTorpedo") == 0) {
        results = run_pilot_gun_fire_or_torpedo(binding->name);
    } else if (std::strcmp(binding->name, "PilotMoveTo") == 0) {
        // 008A4150: the PilotMoveToRange issue without the range argument.
        ++pilot_move_to_calls_;
        const bool outer_plain = pilot_move_to_plain_;
        pilot_move_to_plain_ = true;
        results = run_pilot_move_to_range(row);
        pilot_move_to_plain_ = outer_plain;
    } else if (std::strcmp(binding->name, "PilotMoveToRange") == 0) {
        results = run_pilot_move_to_range(row);
    } else if (std::strcmp(binding->name, "EntityTurnToEntity") == 0) {
        results = run_entity_turn_to_entity(row);
    } else if (std::strcmp(binding->name, "UnitSetFireStance") == 0) {
        results = run_unit_set_fire_stance(row);
    } else if (std::strcmp(binding->name, "UnitHoldFire") == 0) {
        results = run_unit_hold_fire(row);
    } else if (std::strcmp(binding->name, "UnitFreeAttack") == 0) {
        results = run_unit_free_fire(row, bsp::ScriptFireOrder::kFreeAttack);
    } else if (std::strcmp(binding->name, "UnitFreeFire") == 0) {
        results = run_unit_free_fire(row, bsp::ScriptFireOrder::kFreeFire);
    } else if (std::strcmp(binding->name, "NavigatorStop") == 0) {
        results = run_navigator_stop(row);
    } else if (std::strcmp(binding->name, "NavigatorCruise") == 0) {
        // 008A75C0, packet cc13-fix5 (docs/CC13_FIX_5.md section 7): as
        // run_navigator_stop with the cruise command; nothing is pushed.
        void* entity = argument_entity(0);
        row.unit_index = index_of(entity);
        row.unit = name_of(entity);
        static const float kZeroVector00f87574[3] = {0.0f, 0.0f, 0.0f};
        const bsp::ScriptOrderIssue order =
            bsp::navigator_cruise_order_008a75c0(kZeroVector00f87574);
        if (entity != nullptr) {
            entity_issue_command(entity, order.command, order.target, order.flags);
        }
        results = 0;
    } else if (std::strcmp(binding->name, "NavigatorEnable") == 0) {
        results = run_navigator_enable(row);
    } else if (std::strcmp(binding->name, "SetShipMaxSpeed") == 0) {
        // 00890A10: argument 0 through 00888AA0, argument 1 through 00B66270,
        // then 00890B51 FSTP [ESI+9C0h]. No result is pushed.
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        const float value = static_cast<float>(argument_number(1));
        const bool stored = entity != nullptr && index < units_.count()
            && units_.set_unit_max_speed_09c0(index, value);
        log_.notef("  SetShipMaxSpeed: %s unit+9C0h = %.4f stored=%d (00890A10 -> 00890B51)",
            index < units_.count() ? name_of(entity).c_str() : "?",
            static_cast<double>(value), stored ? 1 : 0);
        results = 0;
    } else if (std::strcmp(binding->name, "PilotLand") == 0) {
        results = run_pilot_land(row);
    } else if (std::strcmp(binding->name, "PilotRetreat") == 0) {
        results = run_pilot_retreat(row);
    } else if (std::strcmp(binding->name, "NavigatorAttackMove") == 0) {
        results = bsp::lua_binding_navigator_attack_move(*this, *this);
    } else if (std::strcmp(binding->name, "SetFireTarget") == 0) {
        // 0089A8B0: argument 0 through 00888AA0; argument 1 nil -> null, an
        // entity table -> that entity, a Vector3 -> a dummy entity (not modelled);
        // then entity->vtable[114h]()->00835860(target, 1), whose 5Eh message
        // 00836240 stores. No result is pushed. Goal 5 chain 660: the body
        // lua_set_fire_target_0089a8b0 over UnitWeaponBinding. A plane's vtable[114h]
        // is 0047F180 (null), so a plane is the image's no-op (0089A8E2 test); without
        // the ship AI host there is no director store.
        GameShipAiHost* ai = units_.ship_ai();
        const auto set_target = [this, ai, &row](void* unit, void* target) {
            const std::size_t target_index = index_of(target);
            const std::size_t plus_one =
                (target != nullptr && target_index < units_.count()) ? target_index + 1 : 0;
            ai->store_fire_target_00836240(index_of(unit), plus_one, true);
            row.target = name_of(target);
        };
        UnitWeaponBinding weapons(UnitWeaponBinding::Calls{
            [this](int i) { return entity_at(i); },
            [this](int i) { return get_boolean(i); },
            [this](int i) { return is_nil(i); },
            [this](int i) { return is_entity_table(i); },
            [this](int i, float* out) { static_cast<void>(argument_vector3(i, out)); },
            [this, ai](void* unit) {
                const std::size_t index = index_of(unit);
                return ai != nullptr && index < units_.count()
                    && !units_.unit_is_kind_of(index, 0x0f);
            },
            set_target,
            [this, set_target](void* unit, const float*) {
                // The image's dummy entity at the position is not modelled; the
                // target is cleared, as this row did before it ran the body.
                log_.unimplemented("SetFireTarget(position): the dummy target entity",
                    "0089a8b0");
                set_target(unit, nullptr);
            },
            [](void*, const bsp::WeaponEnableMessage&) { unit_query_unreached("0071DFD0"); }});
        results = bsp::lua_set_fire_target_0089a8b0(weapons);
    } else if (std::strcmp(binding->name, "PilotMoveOnPath") == 0) {
        // 008A3E70: the NavigatorMoveOnPath message without the speed half.
        ++pilot_move_on_path_calls_;
        bsp::NavigatorPathOrder order{};
        void* outer_path_entity = path_entity_for_order_;
        const bool outer_no_speed = pilot_path_no_speed_;
        path_entity_for_order_ = entity_from_argument(1);
        pilot_path_no_speed_ = true;
        results = bsp::lua_binding_navigator_move_on_path(*this, *this, order);
        pilot_path_no_speed_ = outer_no_speed;
        path_entity_for_order_ = outer_path_entity;
        row.command = "moveonpath";
        row.target = name_of(entity_from_argument(1));
        row.path_follow_mode = order.follow_mode;
        row.path_parameter = order.path_parameter;
        row.path_object_id = order.path_object_id;
    } else if (std::strcmp(binding->name, "NavigatorMoveOnPath") == 0) {
        bsp::NavigatorPathOrder order{};
        void* outer_path_entity = path_entity_for_order_;
        path_entity_for_order_ = entity_from_argument(1);
        results = bsp::lua_binding_navigator_move_on_path(*this, *this, order);
        path_entity_for_order_ = outer_path_entity;
        row.command = "moveonpath";
        row.target = name_of(entity_from_argument(1));
        row.path_follow_mode = order.follow_mode;
        row.path_parameter = order.path_parameter;
        row.path_object_id = order.path_object_id;
    } else if (std::strcmp(binding->name, "NavigatorSetAvoidLandCollision") == 0) {
        results = bsp::lua_binding_navigator_set_avoid_land_collision(*this);
    } else if (std::strcmp(binding->name, "NavigatorSetTorpedoEvasion") == 0) {
        results = bsp::lua_binding_navigator_set_torpedo_evasion(*this);
    } else if (std::strcmp(binding->name, "NavigatorSetAvoidShipCollision") == 0) {
        results = bsp::lua_binding_navigator_set_avoid_ship_collision(*this);
    } else if (std::strcmp(binding->name, "JoinFormation") == 0) {
        void* leader = entity_from_argument(1);
        row.formation_leader = name_of(leader);
        ++summary_.formations_requested;
        results = bsp::lua_binding_join_formation(*this);
    } else if (std::strcmp(binding->name, "SetSkillLevel") == 0) {
        results = bsp::lua_binding_set_skill_level(*this);
    } else if (std::strcmp(binding->name, "TorpedoEnable") == 0) {
        // 0089C8F0: the unit's weapon director (vtable[114h]) gates the send;
        // 0071E0D0 builds session message 5Ah with sub-kind 5 and the flag,
        // and 0071C1E0 stores the flag at director+222h (0071C25B). A plane's
        // vtable[114h] is 0047F180 (null), so a plane is the image's no-op.
        // SUBSTITUTION: stored at the call, not delivered through 0077C2A0.
        // Goal 5 chain 660: the body lua_torpedo_enable_0089c8f0 over UnitWeaponBinding.
        results = run_weapon_enable_binding(binding->name);
    } else if (std::strcmp(binding->name, "ArtilleryEnable") == 0) {
        // Packet cc13-fix15. 0089C590: argument 0 through 00888AA0, the
        // director through vtable[114h] (0089C6A8), argument 1 through 00B66250;
        // with a director, 0071DFD0 routes message 5Ah sub-kind 3, which
        // 0071C1E0 stores at director+220h (0071C231). A plane's vtable[114h]
        // (0047F180) is null, so a plane is the image's no-op. SUBSTITUTION:
        // stored at the call, not delivered through 0077C2A0. Goal 5 chain 660: the
        // body lua_artillery_enable_0089c590 over UnitWeaponBinding.
        results = run_weapon_enable_binding(binding->name);
    } else if (std::strcmp(binding->name, "NavigatorForceTorpedo") == 0) {
        // 008A7200: argument 0 through 00888AA0; with more than one argument,
        // argument 1 through BSP_LuaObject_GetBoolean (008A7288..); then the
        // device walk. No result is pushed.
        void* entity = entity_from_argument(0);
        const bool first_only = argument_count_ > 1 && argument_boolean(1);
        const std::size_t index = index_of(entity);
        GameGunneryHost* gunnery = units_.gunnery();
        if (entity != nullptr && index < units_.count() && gunnery != nullptr) {
            gunnery->force_torpedo_fire_008a7200(index, first_only);
        }
        results = 0;
    } else if (std::strcmp(binding->name, "DisablePhysics") == 0) {
        // 00891380: 00888AA0 resolves argument 0; 0080E490 reads unit+1018h and
        // 0089149E stores 1 at +14h with no null test (a unit with no controller
        // faults in the image; the host drops it).
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        if (entity != nullptr && index < units_.count()) {
            const bool done = units_.disable_physics_00891380(index);
            log_.notef("  DisablePhysics: %s controller+14h=1%s (00891380)",
                units_.unit_row(index) != nullptr ? units_.unit_row(index)->name.c_str() : "?",
                done ? "" : " refused: no controller");
        }
    } else if (std::strcmp(binding->name, "AddMatrixInterpolator") == 0) {
        // 008ADE00: argument 3 as a number, arguments 2 and 1
        // through 00888760 (Vector3 tables), argument 0 through 00888AA0, then
        // 00905080(world, entity, translation, rotation, duration) at 008ADF92.
        // LABELLED: queued for the world's next pass, which reads the base
        // matrix and the clock when it registers the record.
        void* entity = entity_from_argument(0);
        float translation[3] = {0.0f, 0.0f, 0.0f};
        float rotation[3] = {0.0f, 0.0f, 0.0f};
        argument_vector3(1, translation);
        argument_vector3(2, rotation);
        const float duration = argument_number(3);
        const std::size_t index = index_of(entity);
        if (entity != nullptr && index < units_.count()) {
            queue_add_matrix_interpolator_008ade00(index, translation, rotation, duration);
            log_.notef("  AddMatrixInterpolator: %s t=(%.2f,%.2f,%.2f) r=(%.4f,%.4f,%.4f) "
                "duration=%.2f (008ADE00 -> 00905080, queued)",
                units_.unit_row(index) != nullptr ? units_.unit_row(index)->name.c_str() : "?",
                static_cast<double>(translation[0]), static_cast<double>(translation[1]),
                static_cast<double>(translation[2]), static_cast<double>(rotation[0]),
                static_cast<double>(rotation[1]), static_cast<double>(rotation[2]),
                static_cast<double>(duration));
        }
    } else if (std::strcmp(binding->name, "ExplodeToParts") == 0) {
        // 0088E1B0: argument 0 through 00888AA0, 0080E490, then 00935C70 on the
        // controller. LABELLED: queued for the gunnery host's next fixed step,
        // which holds the 20 segment healths.
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        if (entity != nullptr && index < units_.count()) {
            queue_explode_to_parts_0088e1b0(index);
            log_.notef("  ExplodeToParts: %s (0088E1B0 -> 00935C70, queued)",
                units_.unit_row(index) != nullptr ? units_.unit_row(index)->name.c_str() : "?");
        }
    } else if (std::strcmp(binding->name, "ForceRecon") == 0) {
        // 008AADF0: no argument read; 00807A50, then the result count. The
        // pass it forces publishes its level changes into `reconlevel` at
        // once (the image writes them inside the pass).
        ++force_recon_calls_;
        GameGunneryHost* gunnery = units_.gunnery();
        if (gunnery != nullptr) gunnery->force_recon_refresh_00807a50();
        sync_recon_level_tables_0077b0c0();
        results = 0;
    } else if (std::strcmp(binding->name, "GetSubmarineDepthLevel") == 0) {
        // 00894100: argument 0 through 00888AA0 with no kind check, EDI =
        // unit+1268h (0089421E). If +1281h, +5Dh or 008522C0 is set, the class
        // is asked: +510h or +514h above 0 keeps the level, else 0
        // (0089425A). One integer result (0089425C -> 00B664B0).
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        ++depth_level_calls_;
        std::int32_t level = 0;
        if (entity != nullptr && index < units_.count()) {
            const GameUnitRow* unit_row = units_.unit_row(index);
            if (unit_row != nullptr && unit_row->submarine_depth_seeded) {
                level = unit_row->submarine_depth_level;
            }
            const GameGunneryHost* gunnery = units_.gunnery();
            const bool need_air = false;        // +1281h, LABELLED
            bool catapult = false;              // 008522C0
            // 0089422E: 008522C0 on the boat, over its catapult devices and
            // the ship's catapult launch record (GameGunneryHost's adapter).
            catapult = gunnery != nullptr
                && gunnery->submarine_catapult_wants_surface(index);
            const bool dead = gunnery != nullptr && gunnery->unit_dead(index);
            const bool kamikaze = false;        // class+510h/+514h, LABELLED
            if ((need_air || dead || catapult) && !kamikaze && level != 0) {
                level = 0;
                ++depth_level_forced_;
            }
        }
        depth_level_last_ = static_cast<int>(level);
        push_number(static_cast<int>(level));
        results = 1;
    } else if (std::strcmp(binding->name, "GetSquadronPlanes") == 0) {
        // 0089CC50: argument 0 through BSP_ObjectHandle_FromLuaTable (no kind
        // check), a new table (00B67930), then for i = 1..[sq+3CCh] the member
        // [sq+3D0h + 4(i-1)]'s u16 +174h as a string at index i (0089CD75-
        // 0089CE00); one result. SUBSTITUTION, labelled: the squadron is the
        // registry record holding the argument's unit, and a plane's +174h is
        // its unit index plus one, the `thisTable` key (kMissionLuaEntityKeyFormat).
        resolve_plane_squadron_members();
        void* entity = argument_ptr_field(0);
        if (entity == nullptr) entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        ++squadron_planes_calls_;
        // 00B67930 calls native_lua_new_table_00b67930; OFF keeps lua_createtable.
        {
            // 0089CD70 creates an untracked result on this dispatch's real VM.
            // The temporary header borrows that stack value; no destructor pops it.
            ScriptOrdersLuaStackView frame(state_, 0);
            bsp::NativeLuaObjectStorage result{};
            bsp::native_lua_new_table_00b67930(frame.object, result);
            log_.implemented("LuaObject::NewTable", "00b67930");
        }
        // Packet cc13-fix5: with kGetSquadronPlanesDepartedLeadBound, a dead lead
        // still names its squadron (find_by_member_or_departed_unit).
        const bsp::PlaneSquadronHostRecord* record =
            index >= units_.count() ? nullptr
            : kGetSquadronPlanesDepartedLeadBound
                ? bsp::plane_squadron_registry().find_by_member_or_departed_unit(index)
                : bsp::plane_squadron_registry().find_by_member_unit(index);
        int slot = 0;
        if (record != nullptr) {
            for (const std::size_t member : record->member_units) {
                if (member == bsp::kPlaneSquadronNoUnit || member >= units_.count()) continue;
                if (slot >= 5) break;   // 0089CD98: past five the image faults
                ++slot;
                char key[16];
                std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
                              static_cast<int>(member + 1));
                lua_pushstring(state_, key);
                lua_rawseti(state_, -2, slot);
            }
        } else {
            ++squadron_planes_unresolved_;
        }
        squadron_planes_entries_ += static_cast<unsigned long long>(slot);
        results = 1;
    } else if (std::strcmp(binding->name, "PutTo") == 0) {
        // 008A9F90: argument 0 through 00888AA0, 007788B0 when it answers
        // IsKindOf(2) (result unused, 008AA0BF), argument 1 through 00888760 as
        // a Vector3, and with three arguments argument 2 * pi / 180. Single
        // player: vtable[118h](pos), then vtable[11Ch](heading) when given.
        void* entity = entity_from_argument(0);
        float pos[3] = {0.0f, 0.0f, 0.0f};
        const bool have_pos = argument_vector3(1, pos);
        const std::size_t index = index_of(entity);
        ++put_to_calls_;
        bool placed = false;
        if (entity != nullptr && have_pos && index < units_.count()) {
            placed = units_.place_at_world_position_008193a0(index, pos);
            if (placed) ++put_to_placed_;
            // 007F2E8F: a squadron's vtable[118h] then places its members.
            if (kSquadronPutToMembersBound && placed) {
                units_.place_squadron_members_007f2920(index);
            }
        }
        if (argument_count_ >= 3) {
            if constexpr (kPutToFormationBound) {
                // 008AA10E-008AA14D: only with exactly three arguments, heading =
                // float(arg * 00CE3D28 / 00CE3D20); then vtable[11Ch] (008AA207).
                if (argument_count_ == 3 && entity != nullptr && index < units_.count()) {
                    units_.set_heading_008196b0(index, put_to_heading_radians(get_number(2)));
                }
            } else {
                log_.unimplemented("PutTo::set_heading_vtable_011c", "008196b0");
            }
        }
        log_.notef("  PutTo: %s pos=(%.1f, %.1f, %.1f) placed=%d (008A9F90 -> 008193A0)",
            index < units_.count() ? name_of(entity).c_str() : "?",
            static_cast<double>(pos[0]), static_cast<double>(pos[1]),
            static_cast<double>(pos[2]), placed ? 1 : 0);
        results = 0;
    } else if (std::strcmp(binding->name, "PutRelTo") == 0) {
        // 008AA560, packet l68_putrelto: arguments 0 and 1 through 00888AA0,
        // argument 2 through 00888760 as a Vector3. vtable[114h] (0080E150, the
        // weapon director) and 007788B0 (when IsKindOf(2)) run with their results
        // unused; vtable[120h] (00819840) takes argument 1's rotation and the
        // transformed point (put_rel_to_00819840). With exactly four arguments
        // (008AA7CF) argument 3 * 00CE3D28 / 00CE3D20 goes to vtable[11Ch]. Then
        // vtable[0D8h] (no host counterpart) and, for an occupant owner (00778890,
        // the group's leader), every other member through vtable[118h](&00F87574).
        // No result.
        void* entity = entity_from_argument(0);
        void* ref = entity_from_argument(1);
        float rel[3] = {0.0f, 0.0f, 0.0f};
        const bool have_rel = argument_vector3(2, rel);
        const std::size_t index = index_of(entity);
        const std::size_t ref_index = index_of(ref);
        bool placed = false;
        if (entity != nullptr && ref != nullptr && have_rel && index < units_.count() &&
            ref_index < units_.count()) {
            placed = units_.put_rel_to_00819840(index, ref_index, rel);
            if (argument_count_ == 4) {
                units_.set_heading_008196b0(index, put_to_heading_radians(get_number(3)));
            }
            const std::int32_t group = units_.unit_formation_group_0284(index);
            if (group >= 0 && units_.formation_leader_0014(group) == index) {
                units_.relocate_formation_members_00819672(index);   // 008AA851
            }
        }
        float pos[3] = {0.0f, 0.0f, 0.0f};
        if (index < units_.count()) units_.unit_position_00fc(index, pos[0], pos[1], pos[2]);
        log_.notef("  PutRelTo: %s rel to %s (%.1f, %.1f, %.1f) placed=%d at (%.1f, %.1f, %.1f) "
            "(008AA560 -> 00819840, packet l68_putrelto)",
            index < units_.count() ? name_of(entity).c_str() : "?",
            ref_index < units_.count() ? name_of(ref).c_str() : "?",
            static_cast<double>(rel[0]), static_cast<double>(rel[1]), static_cast<double>(rel[2]),
            placed ? 1 : 0, static_cast<double>(pos[0]), static_cast<double>(pos[1]),
            static_cast<double>(pos[2]));
        results = 0;
    } else if (std::strcmp(binding->name, "PutFormationTo") == 0) {
        // 008AA260, packet cc13-fix5 (docs/CC13_FIX_5.md section 9): argument 0
        // through 00888D20 (the Ptr field), argument 1 as a Vector3; with exactly
        // four arguments (008AA3CA) argument 3 * 00CE3D28 (pi as a float) /
        // 00CE3D20 (180.0); with three or more (008AA41F) argument 2 as the shape,
        // else -1. Single player ([game+1FE4h] == 0): 008AA4E0 0070EFD0(shape) on
        // [unit+284h] when shape - 1 < 3 and the unit is grouped, then
        // vtable[118h](pos) and vtable[11Ch](heading). No result.
        void* entity = argument_ptr_field(0);
        if (entity == nullptr) entity = entity_from_argument(0);
        float pos[3] = {0.0f, 0.0f, 0.0f};
        const bool have_pos = argument_vector3(1, pos);
        const int shape = argument_count_ >= 3 ? argument_integer(2) : -1;
        const std::size_t index = index_of(entity);
        bool placed = false;
        if (entity != nullptr && index < units_.count()) {
            if (static_cast<unsigned>(shape - 1) < 3u && units_.unit_formation_group_0284(index) >= 0) {
                if constexpr (kPutToFormationBound) {
                    units_.reshape_formation_0070efd0(index, shape);   // 008AA4E0
                } else {
                    log_.unimplemented("PutFormationTo::apply_formation_shape", "0070efd0");
                }
            }
            if (have_pos) {
                placed = units_.place_at_world_position_008193a0(index, pos);
                // 007F2E8F: a squadron's vtable[118h] then places its members.
                if (kSquadronPutToMembersBound && placed) {
                    units_.place_squadron_members_007f2920(index);
                }
            }
        }
        if (argument_count_ == 4) {
            if constexpr (kPutToFormationBound) {
                // 008AA3CA: argument 3 * 00CE3D28 / 00CE3D20, then vtable[11Ch].
                if (entity != nullptr && index < units_.count()) {
                    units_.set_heading_008196b0(index, put_to_heading_radians(get_number(3)));
                }
            } else {
                log_.unimplemented("PutFormationTo::set_heading_vtable_011c", "008196b0");
            }
        }
        log_.notef("  PutFormationTo: %s pos=(%.1f, %.1f, %.1f) shape=%d placed=%d (008AA260, "
            "packet cc13-fix5)", index < units_.count() ? name_of(entity).c_str() : "?",
            static_cast<double>(pos[0]), static_cast<double>(pos[1]),
            static_cast<double>(pos[2]), shape, placed ? 1 : 0);
        results = 0;
    } else if (std::strcmp(binding->name, "GetSquadTorpedoes") == 0) {
        // Packet cc13-fix21, PARTIAL. 008BEF80: 008BF0E9 00B67930 NewTable is
        // unconditional (every path, nil or non-squadron argument included, answers
        // a table; a null squadron faults only once a list element qualifies), and
        // the walk of [game+19CCh] fills it with the 00927B40 self objects of the
        // live torpedoes whose +3BCh is one of the squadron's planes. LABELLED: the
        // host's payload is an ordnance set with no Lua objects, so the table is
        // always empty and the squadron stays armed where the image's callers Kill
        // those torpedoes. The flag (argument 1, read only when there are two
        // arguments, 008BF0A7) does not matter to an empty answer.
        if constexpr (kLuaGetSquadTorpedoesBound) {
            if constexpr (bsp::kRackedRoundEntityCc13Bound) {
                // Packet cc13-rackround (include/bsp/racked_round_entity_cc13.hpp).
                // 008BF0A7: the flag only with exactly two arguments. A racked round's
                // (e+310h)->vtable[38h] (008561F0, entity+458h "launched") is 0, so
                // only the flag admits it. LABELLED: in-flight torpedoes are gunnery
                // projectile rows with no Lua object and are not listed; the
                // squadron's planes are its registry members (the image reads the
                // +3D0h slots, i <= 4); a dead member lists nothing (its rounds die
                // with it, 00926D90's child loop).
                const bool include_all = argument_count_ == 2 && argument_boolean(1);
                std::size_t planes = 0;
                std::size_t listed = 0;
                if (state_ != nullptr) {
                    lua_newtable(state_);
                    const int table = lua_gettop(state_);
                    const std::size_t squadron = index_of(argument_entity(0));
                    const bsp::PlaneSquadronHostRecord* record = nullptr;
                    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
                        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == squadron) {
                            record = &r;
                            break;
                        }
                    }
                    const GameGunneryHost* gunnery = units_.gunnery();
                    if (record != nullptr && include_all) {
                        std::vector<int> racks;
                        for (const std::size_t m : record->member_units) {
                            if (m == bsp::kPlaneSquadronNoUnit || m >= units_.count()) continue;
                            if (gunnery != nullptr && gunnery->unit_dead(m)) continue;
                            ++planes;
                            units_.racked_torpedo_rounds(m, racks);
                            for (const int rack : racks) {
                                bsp::RackedRoundProxy& p = bsp::racked_round_proxy(state_, m, rack);
                                // A rack listed again after a kill holds a new round
                                // (a rearm attached one): a new entity, a new table.
                                lua_pushlightuserdata(state_, &p);
                                lua_rawget(state_, LUA_REGISTRYINDEX);
                                if (p.dead || !lua_istable(state_, -1)) {
                                    lua_pop(state_, 1);
                                    p.dead = false;
                                    lua_newtable(state_);
                                    lua_pushlightuserdata(state_, &p);
                                    lua_setfield(state_, -2, "Ptr");
                                    lua_pushlightuserdata(state_, &p);
                                    lua_pushvalue(state_, -2);
                                    lua_rawset(state_, LUA_REGISTRYINDEX);
                                }
                                // 00B666C0 at keys 1, 2, ...
                                lua_rawseti(state_, table, static_cast<int>(++listed));
                            }
                        }
                    }
                    results = 1;
                }
                log_.notef("  GetSquadTorpedoes: %s flag=%d -> %zu racked rounds on %zu planes "
                    "(008BEF80, packet cc13-rackround)", row.unit.c_str(), include_all ? 1 : 0,
                    listed, planes);
            } else {
            if (state_ != nullptr) {
                lua_newtable(state_);
                results = 1;
            }
            log_.notef("  GetSquadTorpedoes: %s -> {} (008BEF80, PARTIAL: payload "
                "torpedoes have no Lua objects, packet cc13-fix21)", row.unit.c_str());
            }
        }
    } else if (std::strcmp(binding->name, "GetSubmarineAirSupply") == 0) {
        // Packet cc13-fix21. 008938E0: 008939DF 00888AA0 on argument 0 (no null and
        // no class test), 008939F6 FLD float [unit+127Ch], 00B66480 PushFloat; one
        // result. unit+127Ch is the field 00893A60 stores (00893BA1) and the air
        // model 00855250 steps, the host's sub_air.air. LABELLED: an entity with no
        // seeded submarine air state pushes nothing (the image reads +127Ch of any
        // entity and faults on null).
        if constexpr (kLuaGetSubmarineAirSupplyBound) {
            float air = 0.0f;
            if (subject != nullptr && row.unit_index < units_.count()
                && units_.submarine_air_008938e0(row.unit_index, air)) {
                {
                    // Reuse the segment-56 core-argument forwarding adapter.
                    // The body asks only entity_at(0), so none of its extended
                    // repair-specific refusal paths can run here.
                    RepairBindingArgs args(*this);
                    SubmarineAirBinding view(units_,
                        [this](void* entity) { return index_of(entity); },
                        [this](float value) { push_number_float_00b66480(value); });
                    const int count = bsp::seg56_options_cc13::
                        lua_get_submarine_air_supply_008938e0(args, view, view);
                    results = state_ != nullptr ? count : 0;
                    air = view.air();
                }
                log_.notef("  GetSubmarineAirSupply: %s air=%.6f (008938E0 <- +127Ch)",
                    row.unit.c_str(), static_cast<double>(air));
            } else {
                log_.unimplemented(
                    "LuaNative::GetSubmarineAirSupply(entity without submarine air state)",
                    "008938e0");
            }
        }
    } else if (std::strcmp(binding->name, "FillPathPoints") == 0) {
        // 0089A190: argument 0 through 00888AA0 and 007AC9D0 (the path
        // interface), 00B67930's new table, then for i < 00415870's count
        // 00B67720(i + 1) and 0088BA30 of 007AF800(i) (0089A2E7..0089A32D);
        // the table is the one result. The path is found by name, as
        // NavigatorMoveOnPath finds it, marker registry first.
        // LABELLED: an argument that is no retained path answers an empty
        // table; the image would dereference whatever 007AC9D0 returned.
        void* path_entity = entity_from_argument(0);
        const SceneMarker* marker = path_entity == nullptr ? nullptr
            : marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(path_entity)));
        const std::string path_name = marker != nullptr ? marker->name
            : (path_entity != nullptr ? name_of(path_entity) : std::string());
        const bsp::game::ScenePathEntry* authored = path_name.empty() ? nullptr
            : bsp::game::scene_path_registry().find(path_name);
        const std::size_t count = authored != nullptr ? authored->points_world.size() : 0;
        ++fill_path_points_calls_;
        if (count == 0) ++fill_path_points_empty_;
        log_.notef("  FillPathPoints(%s) points=%zu (0089A190 -> 007AF800)",
            path_name.empty() ? "?" : path_name.c_str(), count);
        if (state_ != nullptr) {
            // 00B67930 calls native_lua_new_table_00b67930; OFF keeps preallocated lua_createtable.
            {
                // 0089A2B7: fresh table on the call frame's VM, before its points.
                ScriptOrdersLuaStackView frame(state_, 0);
                bsp::NativeLuaObjectStorage result{};
                bsp::native_lua_new_table_00b67930(frame.object, result);
                log_.implemented("LuaObject::NewTable", "00b67930");
            }
            for (std::size_t i = 0; i < count; ++i) {
                push_vector3_table_0088ba30(authored->points_world[i].data());
                lua_rawseti(state_, -2, static_cast<int>(i + 1));
            }
            results = 1;
        } else {
            results = 0;
        }
    } else if (std::strcmp(binding->name, "ShipSetTorpedoStock") == 0) {
        // 0089EEE0: 008F2260 resolves argument 0, 00B66290 reads argument 1 as
        // an integer, and 0081F8B0 runs on the unit with no class test and no
        // director (0089EFD0..0089F01D). A null entity reaches 0081F8B0 with
        // ECX = 0 in the image; the host drops it. LABELLED: queued for the
        // gunnery host's next fixed step.
        const int stock = argument_integer(1);
        if (subject != nullptr && row.unit_index < units_.count()) {
            queue_ship_set_torpedo_stock_0089eee0(row.unit_index, stock);
            log_.notef("  ShipSetTorpedoStock: %s stock=%d (0089EEE0, queued for 0081F8B0)",
                row.unit.c_str(), stock);
        }
    } else if (std::strcmp(binding->name, "ForceFireDepthCharge") == 0) {
        // Packet cc13-fix17. 0089B9D0: argument 0 through 00888AA0 and straight
        // into the child walk (no null test; a null entity faults in the image,
        // the host drops it). No result. LABELLED: queued for the gunnery host's
        // next fixed step.
        if (subject != nullptr && row.unit_index < units_.count()) {
            queue_force_fire_depth_charge_0089b9d0(row.unit_index);
            log_.notef("  ForceFireDepthCharge: %s (0089B9D0 -> vtable[1F0h], queued)",
                row.unit.c_str());
        }
    } else if (std::strcmp(binding->name, "SetSubmarineAirSupply") == 0) {
        // Packet cc13-fix17. 00893A60: argument 0 through 00888AA0, argument 1
        // through 00B66270 rounded to float (00893B95), stored at unit+127Ch
        // (00893BA1) with no class, null or range test. No result. LABELLED: a
        // unit with no seeded submarine air state stores nothing; a null entity
        // (the image faults) is dropped before the body.
        // Goal 5 chain 663: the body lua_set_submarine_air_supply_00893a60 over
        // UnitFieldStoreBinding.
        if (subject != nullptr && row.unit_index < units_.count()) {
            RepairBindingArgs args(*this);
            UnitFieldStoreBinding image(units_, [this](void* e) { return index_of(e); });
            results = bsp::seg56_options_cc13::lua_set_submarine_air_supply_00893a60(args, image);
            log_.notef("  SetSubmarineAirSupply: %s air=%.3f stored=%d (00893A60 body -> +127Ch)",
                row.unit.c_str(), static_cast<double>(argument_number(1)),
                image.stored() ? 1 : 0);
        }
    } else if (std::strcmp(binding->name, "SquadronSetTravelSpeed") == 0) {
        // Packet l68_travel_speed. 008A0630: argument 0 through 00888AA0, argument 1
        // through 00B66270 rounded to float (008A076C), argument 2 through 00B66250
        // only when exactly three arguments are passed (008A07BA), else false. No
        // class or null test before the four stores; a unit with no squadron slot
        // stores nothing here, and a null entity (the image faults) is dropped
        // before the body. No result.
        // Goal 5 chain 663: the body lua_squadron_set_travel_speed_008a0630 over
        // UnitFieldStoreBinding, its four stores committed as one units-host write.
        if (subject != nullptr && row.unit_index < units_.count()) {
            RepairBindingArgs args(*this);
            UnitFieldStoreBinding image(units_, [this](void* e) { return index_of(e); });
            results = bsp::cc13_left55::seg56a::lua_squadron_set_travel_speed_008a0630(args, image);
            const bool stored = image.commit_squadron_travel_speed();
            log_.notef("  SquadronSetTravelSpeed: %s speed=%.3f force=%d stored=%d "
                "(008A0630 body -> +388h/+38Fh/+3A0h/+3ACh)", row.unit.c_str(),
                static_cast<double>(image.speed()), image.force() ? 1 : 0, stored ? 1 : 0);
        }
    } else if (std::strcmp(binding->name, "ReloadTorpedoes") == 0) {
        // 008C3CE0: 008C3DE0 00888AA0 resolves argument 0 (a second argument is
        // never read); the device loop 008C3E00-008C3E47 runs in the gunnery host.
        // LABELLED: queued for its next fixed step. A null entity would fault at
        // 008C3DF7 in the image; the host drops it.
        if (subject != nullptr && row.unit_index < units_.count()) {
            queue_reload_torpedoes_008c3ce0(row.unit_index);
            log_.notef("  ReloadTorpedoes: %s (008C3CE0, queued for 0072CF00 on every tube)",
                row.unit.c_str());
        }
    } else if (std::strcmp(binding->name, "RepairEnable") == 0) {
        bsp::RepairEnableArm arm = bsp::RepairEnableArm::kLocalFieldWrite;
        results = bsp::lua_binding_repair_enable(*this, arm);
    } else if (std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        bsp::SetRoleAvailableArm arm = bsp::SetRoleAvailableArm::kDirectCall;
        results = bsp::lua_binding_set_role_available(*this, arm);
    } else if (std::strcmp(binding->name, "CreateScript") == 0) {
        results = bsp::lua_binding_create_script(*this, *this);
    } else if (std::strcmp(binding->name, "SetThink") == 0) {
        // 00897FB0's body is bsp::lua_binding_set_think (docs/LUA_BINDING_CORE.md);
        // this file supplies its one callee rather than restating the binding.
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_set_think(*this, core);
    } else if (std::strcmp(binding->name, "SetParty") == 0) {
        // 008A8930's own body, over the same adapter SetThink uses. The two
        // entity virtuals it dispatches through have no resolved concrete
        // vtable (docs/LUA_BINDING_CORE.md), so the adapter records both and
        // takes the no-session-message arm; the number the binding pushes back
        // is the argument, which is what usn_2_java.lua:54 stores in
        // Mission.Party and what recon[Mission.Party] is then indexed with.
        SetThinkCoreHost core(*this);
        if (luainit_depth_ > 0) ++luainit_setparty_calls_;   // cc13-reconload, log-only
        results = bsp::lua_binding_set_party(*this, *this, core);
    } else if (std::strcmp(binding->name, "SetRace") == 0) {
        // bsp.py show 008A8720: read integer arg1 before resolving arg0;
        // kind 2 uses route flags 7, others pass the current +54h to +2Ch.
        // Preserve the host's existing partial 00923B80/00928F50 owner:
        // fields and Lua mirror are concrete; native child propagation and
        // observer notification remain outside that owner, as for SetParty.
        auto party = [this](void* entity) {
            if (const GameScriptEntity* script = script_entity(entity)) return script->party;
            const GameUnitRow* row = units_.unit_row(index_of(entity));
            if (row == nullptr) throw std::logic_error("SetRace: no entity owner");
            return row->party;
        };
        auto apply = [this](void* entity, int current_party, int race) {
            if (GameScriptEntity* script = script_entity(entity)) {
                script->party = current_party;
                script->race = race;
                write_script_identity_fields(*script, false);
            } else {
                const std::size_t index = index_of(entity);
                if (index >= units_.count()) throw std::logic_error("SetRace: no unit owner");
                units_.set_unit_race(index, race);
                if (party_mirror_ != nullptr) party_mirror_(party_mirror_context_);
            }
            log_.implemented("SetRace::set_party_race_lua_mirror", "00928f50");
        };
        SetRaceBinding host(
            [this](void* entity, int kind) { return entity_is_kind_of(entity, kind); },
            party, apply,
            [this, party, apply](void* entity,
                const bsp::seg56_options_cc13::IntValueMessage& posted) {
                const auto* messages = commands_session_messages();
                // A real unit route borrows the session codec already used
                // by SetParty. Its four-bit result, not the Lua integer,
                // reaches the receiver at the commands pump (0095ABE0/80h).
                if (messages == nullptr)
                    throw std::logic_error("SetRace: no local session codec");
                const bsp::HostSessionMessages::VehicleRaceFields fields{
                    posted.type, posted.word_18, posted.byte_1a,
                    static_cast<std::uint32_t>(posted.value)};
                const int copied_race = static_cast<int>(
                    messages->queue_vehicle_race_80(fields));
                const auto id = entity_object_id(entity);
                if (id == 0 || !commands_post_loopback_callback_0076e520(
                    static_cast<std::size_t>(id) - 1u,
                    [entity, copied_race, party, apply] {
                        apply(entity, party(entity), copied_race);
                    }))
                    throw std::logic_error("SetRace: no local unit delivery queue");
            });
        RepairBindingArgs args(*this);
        results = bsp::seg56_options_cc13::lua_set_race_008a8720(args, host, host);
    } else if (std::strcmp(binding->name, "SetWait") == 0) {
        results = bsp::lua_binding_set_wait(*this, *this);
    } else if (std::strcmp(binding->name, "ClearThink") == 0) {
        results = bsp::lua_binding_clear_think(*this, *this);
    } else if (std::strcmp(binding->name, "DeleteScript") == 0) {
        results = bsp::lua_binding_delete_script(*this, *this, nullptr);
    } else if (std::strcmp(binding->name, "GetHpPercentage") == 0) {
        results = bsp::lua_binding_get_hp_percentage(*this, *this);
    } else if (std::strcmp(binding->name, "DoVector3") == 0) {
        // 00899300 takes only the Lua call frame, so there is no unit/class
        // precondition. The reconstructed body performs all three reads and
        // float stores; its result is pushed onto this same VM, not a record.
        DirectionResults pushed([this](const bsp::cc13_seg56::Vec3& v) {
            const float xyz[3] = {v.x, v.y, v.z};
            push_vector3_table_0088ba30(xyz);
        });
        Vector3NumberArgs args(*this, state_);
        const int count = bsp::cc13_seg56::lua_do_vector3_00899300(args, pushed);
        results = state_ != nullptr ? count : 0;
    } else if (std::strcmp(binding->name, "GetPosition") == 0) {
        results = bsp::lua_binding_get_position(*this, *this);
    } else if (std::strcmp(binding->name, "GetMeasure") == 0) {
        results = bsp::lua_binding_get_measure(*this);
    } else if (std::strcmp(binding->name, "GameTime") == 0) {
        results = bsp::lua_binding_game_time(*this);
    } else if (std::strcmp(binding->name, "random") == 0) {
        results = bsp::lua_binding_random(*this, *this, *this);
    } else if (std::strcmp(binding->name, "GetDifficulty") == 0) {
        // 008AE10E reads 00E188A8, 008AE113 compares game+1FE4h against a zeroed
        // EBP and 008AE121's JZ takes the campaign arm, which pushes game+6ACh;
        // the other arm pushes the literal 2. Both are in
        // bsp::lua_binding_get_difficulty already.
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_get_difficulty(*this, core);
    } else if (std::strcmp(binding->name, "SETLOG") == 0) {
        // Packet cc8_lua_binding_routing_audit. These five push nothing, so none
        // of them changes a script branch; they are routed for the accuracy of
        // the host's own records. docs/LUA_BINDING_ROUTING_AUDIT.md.
        results = bsp::lua_binding_setlog();
    } else if (std::strcmp(binding->name, "EnableMessages") == 0) {
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_enable_messages(*this, core);
    } else if (std::strcmp(binding->name, "LoadMessageMap") == 0) {
        SetThinkCoreHost core(*this);
        // Packet cc9_dialog_sequencer: the map the message records resolve in.
        message_map_name_ = get_string(0);
        message_map_index_ = get_integer(1);
        results = bsp::lua_binding_load_message_map(*this, core);
    } else if (std::strcmp(binding->name, "UnloadMessageMap") == 0) {
        // Goal 4 chain 331: 008C6490's body; find_binding offers the row only ON.
        UnloadMessageMapArgs args(*this);
        UnloadMessageMapGlue glue(*this);
        results = bsp::cc13_seg56::lua_unload_message_map(args, glue);
    } else if (std::strcmp(binding->name, "FailureRepairEnable") == 0) {
        // Goal 4 chain 365: 008AD540's body; find_binding offers the row only ON.
        RepairBindingArgs args(*this);
        GameGunneryHost* const gunnery = units_.gunnery();
        FailureRepairEnableAdapter adapter(
            [this](void* entity, int class_id) { return entity_is_kind_of(entity, class_id); },
            [this, gunnery](void* entity, bool enabled) {
                const std::size_t index = index_of(entity);
                if (gunnery != nullptr && index < units_.count()) {
                    gunnery->set_failure_repair_enabled_00939fe0(index, enabled);
                } else {
                    // The image stores the byte at [entity+A20h+46h] of whatever the
                    // argument names; this host holds a repair task for units only.
                    log_.notef("  FailureRepairEnable: no units-host repair task for the "
                        "argument (008AD540 -> 00939FE0)");
                }
            });
        results = bsp::cc13_left55::seg56b::lua_failure_repair_enable(args, adapter);
    } else if (std::strcmp(binding->name, "AddFire") == 0) {
        // Goal 4 chain 365: 008ACD10's body reads its four arguments and stores
        // nothing (the image's AddFire is inert); no result.
        RepairBindingArgs args(*this);
        results = bsp::cc13_left55::seg56b::lua_add_fire(args);
    } else if (std::strcmp(binding->name, "Music_Control_SetLevel") == 0) {
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_music_control_set_level(*this, core);
    } else if (std::strcmp(binding->name, "MusicFade") == 0) {
        // 008C7190 calls its existing Lua body and 00A77860; OFF keeps fallback dispatch.
        ScriptMusicFadeAdapter music;
        results = bsp::cc13_seg56::lua_music_fade(*this, music);
    } else if (std::strcmp(binding->name, "Scoring_SetFinalScoringFunctionName") == 0) {
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_scoring_set_final_scoring_function_name(*this, core);
    } else if (kMissionEndBound && (std::strcmp(binding->name, "StartDialog") == 0 ||
                                    std::strcmp(binding->name, "KillDialog") == 0 ||
                                    std::strcmp(binding->name, "GetActDialogIDs") == 0)) {
        // Goal 5 chain 663: the three dialog rows on their bodies over ScriptDialogBinding.
        ScriptDialogBinding dialogs(ScriptDialogBinding::Calls{
            [this](const std::string& id, int index) {
                // 00451A90. The id is registered until KillDialog; the dialog table
                // (argument `index`) is what the panel sequence plays (render and
                // audio side, not modelled), so nothing here plays it to its end.
                if (!id.empty()) active_dialogs_[id] = mission_clock_;
                ++dialog_starts_;
                if (kDialogSequencerBound && !id.empty() && state_ != nullptr &&
                    lua_type(state_, stack_slot(index)) == LUA_TTABLE) {
                    // 00451920 looks the id up or inserts it, and 004507D0 rebuilds
                    // the entry from the table.
                    DialogEntry entry;
                    dialog_parse_entry(stack_slot(index), entry);
                    dialog_entries_[id] = entry;
                    log_.notef("  dialog entry \"%s\" commands=%zu priority=%.1f "
                        "default_pause=%.2f", id.c_str(), entry.commands.size(),
                        static_cast<double>(entry.priority),
                        static_cast<double>(entry.default_pause));
                }
                log_.notef("  StartDialog(\"%s\") active=%zu (008B0540 body -> 00451A90)",
                    id.c_str(), active_dialogs_.size());
            },
            [this]() { return static_cast<std::int32_t>(game_session_mode()); },
            [this](const std::string& id) {
                // 004514A0.
                active_dialogs_.erase(id);
                if (kDialogSequencerBound) {
                    dialog_entries_.erase(id);
                    if (_stricmp(dialog_current_.c_str(), id.c_str()) == 0) {
                        dialog_current_.clear();
                    }
                }
                ++dialog_kills_;
                log_.notef("  KillDialog(\"%s\") active=%zu (008B0940 body -> 004514A0)",
                    id.c_str(), active_dialogs_.size());
            },
            [this]() {
                ++dialog_queries_;
                ScriptDialogIdCollection<decltype(active_dialogs_)> collection(active_dialogs_);
                return collection.collect();
            },
            [this](const std::vector<std::string>& ids) {
                // 008CB807 BSP_LuaObject_NewTable, then 00B672F0(i + 1, id) per id.
                if (state_ == nullptr) return;
                lua_createtable(state_, static_cast<int>(ids.size()), 0);
                int n = 0;
                for (const std::string& id : ids) {
                    ScriptOrdersLuaStackView table(state_, lua_gettop(state_));
                    ScriptOrdersNativeText value(id);
                    bsp::native_lua_set_lstring_at_00b672f0(table.object, ++n, value.text);
                }
            }});
        RepairBindingArgs args(*this);
        if (std::strcmp(binding->name, "StartDialog") == 0) {
            results = bsp::cc13_seg56::lua_start_dialog(args, dialogs);
        } else if (std::strcmp(binding->name, "KillDialog") == 0) {
            results = bsp::cc13_seg56::lua_kill_dialog(args, dialogs);
        } else {
            const int pushed = bsp::cc13_seg56::lua_get_act_dialog_ids(dialogs, dialogs);
            results = state_ != nullptr ? pushed : 0;
        }
    } else if (std::strcmp(binding->name, "EnableInput") == 0) {
        // 008AFBD0: argument 0 the boolean, context 1Eh at level 0 (enable) or
        // 5 (disable) through 004BEC00 / 00A933F0; nothing is pushed.
        RepairBindingArgs args(*this);
        ScriptInputContextBinding input(*bound_input_context_manager());
        results = bsp::cc13_seg56::lua_enable_input(args, input);
    } else if (std::strcmp(binding->name, "IsGUIActive") == 0) {
        // 008CA010: 008C9F80 on argument 0; when it finds a widget, a second
        // 008C9F80 and that widget's +38h; 00B66450 of the answer (false when
        // nothing was found); one result.
        RepairBindingArgs args(*this);
        ScriptGuiQueryBinding gui(*bound_gui_frontend(), [this](bool value) {
            push_boolean(value);
        });
        results = bsp::cc13_seg56::lua_is_gui_active(args, gui, gui);
    } else if (std::strcmp(binding->name, "Countdown") == 0) {
        // 008B17B2-008B17C3: 00887120(-1, 4) reads the frame into 14h-byte
        // variants: the text +8h (argument 0), +1Ch (1), +30h (2), and the name
        // +44h (3) only when there are more than three (008B17D2); arguments 4..n
        // go to a new vector (008B1846-008B185E).
        const std::string text = get_string(0);
        const float level = static_cast<float>(get_number(1));
        const float seconds = static_cast<float>(get_number(2));
        const std::string callback = count() > 3 ? get_string(3) : std::string();
        std::vector<int> arguments;
        for (int index = 4; index < count(); ++index) {
            lua_pushvalue(state_, stack_slot(index));
            arguments.push_back(luaL_ref(state_, LUA_REGISTRYINDEX));
        }
        // 008B18B8: 0052B9B0 on [[00E198C4]+C8h] with the text (argument 0)
        // and the seconds (argument 2), before 00733FF0 (goal 5 chain 662: the
        // HUD's counters screen); 00733FF0's first call 005BCA70 on
        // [[00E198C4]+58h] is next.
        if (narrative_hud_ == nullptr || !narrative_hud_->show_mission_countdown(text, seconds))
            log_.unimplemented("HudCountdown::show_text", "0052b9b0");
        countdown_start_00733ff0(text, level, seconds, callback, std::move(arguments));
        ++countdown_starts_;
        log_.notef("  Countdown(\"%s\", %.0f, %.2f, \"%s\") at %.2f s, %zu more argument(s)",
            text.c_str(), static_cast<double>(level), static_cast<double>(seconds),
            countdown_.callback_4c.c_str(), static_cast<double>(mission_clock_),
            countdown_.argument_refs_54.size());
        results = 0;  // 008B1990, nothing pushed
    } else if (std::strcmp(binding->name, "CountdownCancel") == 0) {
        // 008B19A0: two 0052AB90 HUD calls (008B1A92, 008B1AA2; goal 5 chain
        // 662, the HUD's counters screen), then 007340A0(&left) and 00B66480(left).
        if (narrative_hud_ == nullptr || !narrative_hud_->cancel_mission_countdown())
            log_.unimplemented("HudCountdown::clear_text", "0052ab90");
        const float left = countdown_time_left();
        countdown_stop_007340a0();
        ++countdown_cancels_;
        push_number_float_00b66480(left);
        results = state_ != nullptr ? 1 : 0;
    } else if (std::strcmp(binding->name, "MissionNarrative") == 0 ||
               std::strcmp(binding->name, "MissionNarrativeEnqueue") == 0 ||
               std::strcmp(binding->name, "MissionNarrativeUrgent") == 0 ||
               std::strcmp(binding->name, "MissionNarrativeOverride") == 0) {
        // Evidence: bsp.py show 008B0C10 / 008B0E10 / 008B1010: the same frame
        // (variant 0 the text, variant 1 the callback, variants 2..n the
        // extras), then 00734870 (008B0DAC), 007349C0 (008B0FAC) or 00734E50
        // (008B11AC) on [game+21E8h]. The body selects the arguments and the
        // literal result count; the post runs the entry point's own body on
        // this owner's narrative object, consumed by run_narrative_update_00735100.
        const std::uint32_t entry_point =
            std::strcmp(binding->name, "MissionNarrativeUrgent") == 0
                ? bsp::cc13_seg56::kNarrativePostUrgent
                : std::strcmp(binding->name, "MissionNarrativeOverride") == 0
                      ? bsp::cc13_seg56::kNarrativePostOverride
                      : bsp::cc13_seg56::kNarrativePost;
        RepairBindingArgs args(*this);
        NarrativePostBinding host([&](std::uint32_t point, const std::string& text,
            const std::string& callback, int extras_from) {
            std::vector<int> refs;
            if (extras_from >= 0) {
                for (int index = extras_from; index < count(); ++index) {
                    lua_pushvalue(state_, stack_slot(index));
                    refs.push_back(luaL_ref(state_, LUA_REGISTRYINDEX));
                }
            }
            const std::size_t extras = refs.size();
            narrative_post(point, text, callback, std::move(refs));
            log_.notef("  %s(\"%s\", \"%s\") at %.2f s, %zu more argument(s), queue %u "
                "(%s -> %08X)", binding->name, text.c_str(), callback.c_str(),
                static_cast<double>(mission_clock_), extras,
                static_cast<unsigned>(narrative_queue_size()),
                binding->row_address == 0x008b0e10u   ? "008B0E10"
                : binding->row_address == 0x008b1010u ? "008B1010"
                                                      : "008B0C10",
                static_cast<unsigned>(point));
            ++narrative_enqueues_;
        });
        results = bsp::cc13_seg56::lua_mission_narrative_post(entry_point, args, host);
    } else if (std::strcmp(binding->name, "DisplayScores") == 0 ||
               std::strcmp(binding->name, "HideScoreDisplay") == 0) {
        // Goal 5 chain 663: 008C20D0 / 008C24B0 on their bodies over ScoreDisplayBinding.
        // DisplayScores: six arguments -> 0052B780 (008C22DA), four -> 0052B620
        // (008C241A), any other count posts nothing; HideScoreDisplay: 0052B900
        // (008C25E5). Both on [[00E198C4]+C8h], the HUD score message 23h through
        // 00772330. Neither pushes a result. Goal 5 chain 667: the senders run on
        // the HUD host's counters screen (GameHudHost::display_score / hide_score).
        ScoreDisplayBinding scores(ScoreDisplayBinding::Calls{
            [this](int id, int flag, const std::string& c, const std::string& d, bool pair,
                   int e, int f) {
                if (narrative_hud_ == nullptr || !narrative_hud_->display_score(
                        static_cast<std::uint32_t>(id), static_cast<std::uint32_t>(flag), c, d, pair,
                        static_cast<std::uint32_t>(e), static_cast<std::uint32_t>(f)))
                    log_.unimplemented("HudCounters::display_score", pair ? "0052b780" : "0052b620");
            },
            [this](int id, int second) {
                if (narrative_hud_ == nullptr || !narrative_hud_->hide_score(
                        static_cast<std::uint32_t>(id), static_cast<std::uint32_t>(second)))
                    log_.unimplemented("HudCounters::hide_score", "0052b900");
            }});
        RepairBindingArgs args(*this);
        results = std::strcmp(binding->name, "DisplayScores") == 0
            ? bsp::cc13_seg56::lua_display_scores(args, scores)
            : bsp::cc13_seg56::lua_hide_score_display(args, scores);
    } else if (std::strcmp(binding->name, "MissionNarrativeClear") == 0) {
        // 008B15B0 calls 00734FA0 once; its callback releases the same
        // queued/showing registry references as the previous host arm.
        NarrativeQueueBinding host([&] {
            return bsp::band_0071_cc13::mission_narrative_size_00733260(
                static_cast<std::int32_t>(narrative_queue_size()), narrative_active_14_);
        }, [&] {
            narrative_clear_00734fa0();
            ++narrative_clears_;
        });
        results = bsp::cc13_seg56::lua_mission_narrative_clear(host);
    } else if (std::strcmp(binding->name, "MissionNarrativeSize") == 0) {
        // 008B0AC0 -> 00733260 reads the retained queue count and the
        // current showing byte, then uses the existing 00B664B0 writer.
        NarrativeQueueBinding host([&] {
            return bsp::band_0071_cc13::mission_narrative_size_00733260(
                static_cast<std::int32_t>(narrative_queue_size()), narrative_active_14_);
        }, [&] {
            narrative_clear_00734fa0();
            ++narrative_clears_;
        });
        results = bsp::cc13_seg56::lua_mission_narrative_size(*this, host);
    } else if (std::strcmp(binding->name, "EndScene") == 0) {
        // 008B01B0. RECORD, labelled: for a failed single-player mission it raises
        // the restart prompt (00531B00) and returns; otherwise it reaches
        // BSP_Game_EndScene 004D7970 (docs/MISSION_END.md). This host ends no
        // scene: it records the first call and the run continues to its frame
        // budget, so a run reports `summary mission scene end`.
        ++end_scene_calls_;
        if (end_scene_first_at_ < 0.0f) end_scene_first_at_ = mission_clock_;
        log_.notef("  EndScene at %.2f s (008B01B0, RECORDED: no prompt and no unload)",
            static_cast<double>(mission_clock_));
        log_.unimplemented("Game::end_scene_prompt_or_unload", "008b01b0");
        results = 0;
    } else if (std::strcmp(binding->name, "GetCameraState") == 0) {
        results = run_get_camera_state_008bf6a0();
    } else if (std::strcmp(binding->name, "GetRotation") == 0) {
        // 008A7E60: argument 0 through 00888AA0 (no null test in the image),
        // the world refresh when +C8h is clear, 0042D2E0 on the unit's world
        // basis (unit+CCh rows), the degree conversion (as 008BF6A0's), and
        // 0088BA30's {x, y, z}: one result. LABELLED: an argument that is not a
        // unit pushes nothing (the image reads whatever object it is handed).
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        float right[3], up[3], forward[3], position[3];
        if (entity != nullptr && index < units_.count() &&
            units_.unit_pose(index, right, up, forward, position)) {
            const bsp::CameraMatrix world = {right[0], right[1], right[2], 0.0f,
                up[0], up[1], up[2], 0.0f, forward[0], forward[1], forward[2], 0.0f,
                position[0], position[1], position[2], 1.0f};
            float degrees[3];
            basis_degrees_0042d2e0(world, degrees);
            push_vector3_table_0088ba30(degrees);
            ++get_rotation_calls_;
            results = state_ != nullptr ? 1 : 0;
        } else {
            ++get_rotation_unresolved_;
            results = 0;
        }
    } else if (std::strcmp(binding->name, "AddPowerup") == 0 ||
               std::strcmp(binding->name, "PreparePowerup") == 0 ||
               std::strcmp(binding->name, "GetAvailablePowerups") == 0) {
        // Packet cc13-powerup: 008EE410, 008EA610 and 008EB350 (src/cc13_powerup.cpp).
        // handles() refuses the rows while kPowerupGrantBound is off. Without a
        // loaded registry (the global_subsystems step failed) there is no manager
        // to run them against, and the row stays an unimplemented record.
        bsp::cc13_powerup::PowerupRuntime& powerups = bsp::cc13_powerup::powerup_runtime();
        if (state_ == nullptr || powerups.host == nullptr) {
            log_.unimplemented(method, address);
            results = 0;
        } else if (std::strcmp(binding->name, "AddPowerup") == 0) {
            results = bsp::cc13_powerup::lua_add_powerup_008ee410(powerups, state_);
        } else if (std::strcmp(binding->name, "PreparePowerup") == 0) {
            results = bsp::cc13_powerup::lua_prepare_powerup_008ea610(powerups, state_);
        } else {
            results = bsp::cc13_powerup::lua_get_available_powerups_008eb350(powerups, state_);
        }
    } else if (std::strcmp(binding->name, "GetCapturePercentage") == 0) {
        // 0089B840: 00888AA0 on argument 0, 006F1F90 on that entity with no class
        // test, then 00B66480 on the result as it is; one result. 006F1F90
        // (006F1F90-006F1FB2): +7A4h == 0 -> 0.0, else FLD [+7A8h] / FIDIV [+7A4h] /
        // FSTP; the binding then applies FABS at 0089B966 before the push, so the
        // pushed fraction is never negative (kLuaCapturePercentageFabsBound; the
        // switch off pushes the signed fraction).
        // SUBSTITUTION, labelled: an entity that is not one of the ship-AI host's
        // capture buildings answers 0.0; the image reads those two offsets of
        // whatever object it is handed.
        ++capture_percentage_calls_;
        void* entity = entity_from_argument(0);
        const std::size_t index = index_of(entity);
        GameShipAiHost* ai = units_.ship_ai();
        float fraction = 0.0f;
        if (entity == nullptr || index >= units_.count() || ai == nullptr ||
            !ai->command_building_capture_fraction_006f1f90(index, fraction)) {
            ++capture_percentage_unresolved_;
            fraction = 0.0f;
        }
        if (kLuaCapturePercentageFabsBound) fraction = std::fabs(fraction);   // 0089B966
        push_number_float_00b66480(fraction);
        results = state_ != nullptr ? 1 : 0;
    } else if (std::strcmp(binding->name, "GetCommandBuildingLevel") == 0) {
        // Packet cc13-fix13. 0089B6D0 (__fastcall, ECX the Lua state, RET): 0089B7CF
        // 00888AA0 on argument 0, 0089B7E6 MOV ESI,[ESI+770h] with no null or class
        // test, 0089B7F1 00B664B0 PushInteger; one result.
        // SUBSTITUTIONS (labelled): the level is the ship-AI host's capture record
        // (+770h as 006F2780 and 006F38E0 keep it); before the records exist, the
        // units host's scene Level (the constructor's value, 0 for a unit that is
        // not a CommandBuilding). An entity with no units-host slot answers 0, where
        // the image would read +770h of a null or foreign object.
        ++command_building_level_calls_;
        void* entity = entity_at(0);   // the body's own 00888AA0 read
        const std::size_t index = index_of(entity);
        if (entity == nullptr || index >= units_.count()) {
            ++command_building_level_unresolved_;
            if (state_ != nullptr) lua_pushinteger(state_, 0);
            results = state_ != nullptr ? 1 : 0;
        } else {
            // Goal 5 chain 660: the body lua_get_command_building_level
            // (src/options_seg56_cc13.cpp) over CommandBuildingLevelView.
            RepairBindingArgs args(*this);
            CommandBuildingLevelView view(units_, [this](void* e) { return index_of(e); },
                [this](int value) {
                    if (state_ != nullptr) lua_pushinteger(state_, static_cast<lua_Integer>(value));
                });
            const int count = bsp::seg56_options_cc13::lua_get_command_building_level(args,
                view, view);
            results = state_ != nullptr ? count : 0;
        }
    } else if (std::strcmp(binding->name, "SetCommandBuildingOwnerPlayer") == 0) {
        // Packet cc13-fix13. 008A8D90 (__fastcall, ECX the Lua state, RET): 00888AA0 on
        // argument 0, 008A8EC1 00B66290 argument 1 as an integer, 008A8EE1
        // vtable[5Ch](1Ch); when true, message D3h (vtable 00CFADBC, +4h = 1, word
        // +18h = 0, byte +1Ah = 0, +1Ch = player, byte +20h = 1) routed by 008A8F2A
        // 0077C2A0(&message, 7, 0) with ECX the entity. Returns no value.
        // SUBSTITUTIONS (labelled): the message is applied at the call through the
        // ship-AI host's D3h (see command_building_set_owner_player_006f4d10); a
        // CommandBuilding test is the host's capture-record lookup; an entity with
        // no units-host slot does nothing, where the image's vtable call would fault.
        ++command_building_owner_calls_;
        void* entity = entity_at(0);   // the body's own 00888AA0 read
        const std::size_t index = index_of(entity);
        GameShipAiHost* ai = units_.ship_ai();
        results = 0;
        if (entity != nullptr && index < units_.count() && ai != nullptr) {
            // Goal 5 chain 660: the body lua_set_command_building_owner_player
            // (src/cc13_left55_seg56b.cpp) over CommandBuildingOwnerBinding.
            RepairBindingArgs args(*this);
            CommandBuildingOwnerBinding owner(*ai, units_.count(),
                [this](void* e) { return index_of(e); });
            results = bsp::cc13_left55::seg56b::lua_set_command_building_owner_player(args,
                owner);
            command_building_owner_applied_ += owner.applied();
        }
    } else if (std::strcmp(binding->name, "GetRoleAvailable") == 0 ||
               std::strcmp(binding->name, "GetRoleOwner") == 0) {
        // Goal 5 chain 655. 008ABAC0 GetRoleAvailable / 008ABF90 GetRoleOwner, their
        // body fill_nine_integers (src/cc13_left55_seg56b.cpp) over RoleTableBinding:
        // {[1..9] = entity+188h} / {[1..9] = entity+1ACh}, one result. Single-player
        // callers: commandhelpers.lua 19250 / 19264 read GetRoleAvailable(unit)[1], and
        // jm05.lua 3021 GetRoleOwner(unit); the multiplayer scripts make the other
        // calls of the 97 in the installed scripts. LABELLED: an
        // argument with no units-host slot pushes nothing and is counted, where the
        // image reads +188h of whatever object 00888AA0 returned.
        ++role_table_calls_;
        if (state_ != nullptr && index_of(entity_at(0)) < units_.count()) {
            results = run_role_table_query(binding->row_address);
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented(binding->row_address == 0x008abf90u
                    ? "LuaNative::GetRoleOwner(entity without a unit slot)"
                    : "LuaNative::GetRoleAvailable(entity without a unit slot)",
                address);
        }
    } else if (std::strcmp(binding->name, "GetSkillLevel") == 0 ||
               std::strcmp(binding->name, "GetCrewLevel") == 0) {
        // Goal 5 chain 655. 008953F0 GetSkillLevel pushes entity->vtable[12Ch]();
        // 008950D0 GetCrewLevel pushes 006E6270 of it, the crew level SetCrewLevel's
        // 006E6210 maps back. Both through UnitQuerySeg56Host (006D1EC0 on a unit,
        // 007ECF70 on a squadron). Callers: commandhelpers.lua 3928..3942 and 4335
        // GetCrewLevel(SelectedUnit) / (unit), jm05.lua 4978 / 4989 GetSkillLevel(unit)
        // against SKILL_ELITE / SKILL_SPNORMAL. LABELLED: an argument with no units-host
        // slot, or a squadron with no live plane, pushes nothing and is counted (the image
        // calls through whatever 00888AA0 or +3D0h holds).
        ++skill_query_calls_;
        resolve_plane_squadron_members();
        const std::size_t unit = index_of(entity_at(0));
        const bsp::PlaneSquadronHostRecord* squadron =
            unit < units_.count() ? fused_squadron_record(unit) : nullptr;
        // A squadron with no live plane has a null +3D0h, which 007ECF70 calls through.
        if (state_ != nullptr && unit < units_.count()
            && (squadron == nullptr || squadron->live_count() > 0)) {
            results = run_seg56_unit_query(binding->row_address);
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented(binding->row_address == 0x008950d0u
                    ? "LuaNative::GetCrewLevel(entity without a unit slot)"
                    : "LuaNative::GetSkillLevel(entity without a unit slot)",
                address);
        }
    } else if (std::strcmp(binding->name, "SetAirBasePlaneLimit") == 0 ||
               std::strcmp(binding->name, "GetAirBasePlaneLimit") == 0) {
        // Goal 5 chain 655. 00897720 SetAirBasePlaneLimit(base, limit): 006BCD20(base, 0),
        // integer argument 1 (00B66290 at 0089785E), 006BA6D0 at 00897878; no result.
        // 008978D0 GetAirBasePlaneLimit(base): 006BCD20(base, 0), 006BA6C0 at 008979F1,
        // 00B664B0 PushNumber; one result. Bodies lua_set_air_base_plane_limit /
        // lua_get_air_base_plane_limit (src/options_seg56_cc13.cpp) over
        // UnitQuerySeg56Host: block+58h is the deck's max_in_air_planes, which the slot
        // tick 006C0510, 006BD460's slot capacity, 006C0F00's fill and 006C5950's return
        // read (src/air_operations.cpp). Callers: usn_04_defend_
        // guadalcanal.lua 1658 (the Japanese carrier to 15), usn_10_battle_of_capeengano
        // .lua 1447 / 1486 / 2529 (both copies), commandhelpers.lua 15237 (the getter).
        // LABELLED: an argument that is not a carrier or airfield with a loaded deck
        // runs nothing and is counted; the image hands 006BA6C0 / 006BA6D0 the null
        // block 006BCD20 answers and faults.
        ++air_base_limit_calls_;
        bool owner_class = false;
        const std::size_t base = index_of(entity_at(0));
        if (state_ != nullptr && base < units_.count()
            && unit_air_ops_deck_006bcd20(units_, base, owner_class) != nullptr) {
            results = run_seg56_unit_query(binding->row_address);
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented(binding->row_address == 0x00897720u
                    ? "LuaNative::SetAirBasePlaneLimit(entity without an air-operations deck)"
                    : "LuaNative::GetAirBasePlaneLimit(entity without an air-operations deck)",
                address);
        }
    } else if (std::strcmp(binding->name, "GetAirBaseSlotStatus") == 0) {
        // Goal 5 chain 655. 00896220 GetAirBaseSlotStatus(base, slot): 00888AA0 on argument
        // 0 (00896322), 006BCD20(base, 0) (XOR DL,DL at 00896339), 00B66290 on argument 1
        // (0089635E), then [[block+4Ch] + (slot - 1) * 58h + 2Ch] pushed by 00B664B0; one
        // result. Body lua_get_air_base_slot_status over AirSlotStateView and
        // UnitQuerySeg56Host's 006BCD20. Callers: jm07.lua 1189 / 1195, jm11.lua 1006,
        // jm12.lua 867 (== 5, kReady) and the PRCPIJN copies, commandhelpers.lua 18905.
        // LABELLED: a base with no loaded deck, or a slot outside 1..its slot count, pushes
        // nothing and is counted; the image reads past the array (or faults on a null
        // block).
        ++air_base_slot_calls_;
        bool owner_class = false;
        const std::size_t base = index_of(entity_at(0));
        const bsp::AirOpsDeck* deck = base < units_.count()
            ? unit_air_ops_deck_006bcd20(units_, base, owner_class) : nullptr;
        const int slot = deck != nullptr ? get_integer(1) : 0;
        if (state_ != nullptr && deck != nullptr && slot >= 1
            && static_cast<std::size_t>(slot) <= deck->slots.size()) {
            results = run_seg56_unit_query(binding->row_address);
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented("LuaNative::GetAirBaseSlotStatus(no deck or no such slot)",
                address);
        }
    } else if (std::strcmp(binding->name, "EntityTurnToPosition") == 0) {
        // Goal 5 chain 655. 008A0F10 EntityTurnToPosition(entity, point [, keep_pitch]):
        // d = point - entity+FCh, d.y zeroed unless a third argument is true, rows
        // (1,0,0), (0,1,0), d through 0085DC80, then the single-player arms of
        // TurnToPositionBinding. Callers: usn_01_battle_of_eastern_solomons.lua 1265 /
        // 1339 / 1420, usn_05_guadalcanal_1st_battle.lua 1542 / 1561 (and the PRCPUS
        // copies), usn_06_guadalcanal_2nd_battle.lua 2294, prcp_06_2ndbattle.lua 2383.
        // LABELLED: an argument that is not a ship, a plane or a squadron, or a point that
        // is no Vector3 (jm09.lua 1984 passes only the point), runs nothing and is
        // counted; the image would call vtable[88h] of whatever it was handed.
        ++turn_to_position_calls_;
        resolve_plane_squadron_members();
        const std::size_t unit = index_of(entity_at(0));
        float point[3] = {};
        const bool poseable = unit < units_.count() && (fused_squadron_record(unit) != nullptr
            || units_.unit_is_kind_of(unit, 6) || units_.unit_is_kind_of(unit, 0x0F));
        if (state_ != nullptr && poseable && argument_vector3(1, point)) {
            results = run_turn_to_position();
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented("LuaNative::EntityTurnToPosition(no poseable unit or no point)",
                address);
        }
    } else if (std::strcmp(binding->name, "PilotStop") == 0) {
        // Goal 5 chain 655. 008A7780 PilotStop(unit) through lua_pilot_stop over
        // FixedCommandBinding: 0077D600(entity, stop 00E08F88, zero target, 1), the issue
        // NavigatorStop makes. Callers: jm12.lua 1724 / 1858 (both copies), prcpjm12.lua
        // 766. LABELLED: an argument with no units-host slot issues nothing and is counted
        // (the image calls 0077D600 with whatever 00888AA0 returned).
        ++pilot_stop_calls_;
        resolve_plane_squadron_members();
        if (index_of(entity_at(0)) < units_.count()) {
            RepairBindingArgs args(*this);
            FixedCommandBinding command([this](void* entity, std::uint32_t command_object,
                const bsp::SceneCommandTarget& target, int flags) {
                entity_issue_command(entity, command_object, target, flags);
            });
            results = bsp::cc13_left55::seg56a::lua_pilot_stop(args, command, command);
            pilot_stop_issued_ += command.issued();
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented("LuaNative::PilotStop(entity without a unit slot)", address);
        }
    } else if (std::strcmp(binding->name, "RemoveShipyardStock") == 0) {
        // Goal 5 chain 655. 00896EE0 RemoveShipyardStock(shipyard, class, count): 00888AA0
        // on argument 0 (00896FE3), 00B66290 class and count, then 008457E0(ECX =
        // entity, class, count) at 0089705F and 00B66400; no result, no session route.
        // Body lua_remove_shipyard_stock (src/options_seg56_cc13.cpp) over
        // UnitQuerySeg56Host's 008457E0. Callers: usn_04_defend_guadalcanal.lua 3265..3270
        // (class 27 from both USN shipyards), jm08.lua 1638..1668 (classes 77 and 174,
        // both copies). LABELLED: an argument with no shipyard state runs nothing and is
        // counted; the image would walk +774h of whatever entity it was handed.
        ++shipyard_stock_calls_;
        const std::size_t yard = index_of(entity_at(0));
        if (state_ != nullptr && yard < units_.count()
            && bsp::shipyards().find_mutable_by_entity_id(static_cast<int>(yard + 1u)) != nullptr) {
            results = run_seg56_unit_query(binding->row_address);
        } else {
            ++unit_query_unresolved_;
            log_.unimplemented("LuaNative::RemoveShipyardStock(entity without a shipyard)",
                address);
        }
    } else if (std::strcmp(binding->name, "GetShipMaxSpeed") == 0) {
        // Packet cc13-fix14b. 00890BB0 (__fastcall, ECX the Lua state, RET): 00890CAF
        // 00888AA0 on argument 0, no null or class test, 00890CC8 0080FC30 with ECX the
        // entity (ST0), 00890CD2 FSTP dword, 00890CD5 00B66480 PushFloat; one result.
        // 0080FC30 is float([unit+9C0h] * scale), the scale 1.0f (00D7A24C) unless the
        // powerups byte 00E0C978 and [[00F88C30]+B8h] are both set (0080FC31..0080FC4A).
        // The scale comes from the units host's throttle_ceiling_inputs, which
        // reads the power-up store under kPowerUpSubsystemCc13Bound (packet
        // cc13-powerup-bind). SUBSTITUTION (labelled): unit+9C0h is the
        // units host's slot value (00822C20's class MaxSpeed, moved by SetShipMaxSpeed
        // 00890B51). An entity with no units-host slot (a scene marker, or none)
        // pushes nothing and records UNIMPLEMENTED, where the image reads +9C0h of
        // whatever object it is handed (and faults on a null one).
        // Goal 5 chain 655: the push is the body lua_get_ship_max_speed
        // (src/options_seg56_cc13.cpp) over UnitQuerySeg56Host's 0080FC30.
        // handles() already refuses the row while the switch is off.
        if constexpr (kLuaGetShipMaxSpeedBound) {
            ++ship_max_speed_calls_;
            void* entity = entity_at(0);
            const std::size_t index = index_of(entity);
            if (entity != nullptr && index < units_.count()) {
                results = run_seg56_unit_query(binding->row_address);
            } else {
                ++ship_max_speed_unresolved_;
                log_.unimplemented("LuaNative::GetShipMaxSpeed(entity without a unit slot)",
                    "00890bb0");
                results = 0;
            }
        }
    } else if (std::strcmp(binding->name, "GetDirection") == 0) {
        // Packet cc13-fix14b. 008A7CB0 (__fastcall, ECX the Lua state, RET): 008A7DAF
        // 00888AA0 on argument 0, 008A7DCF 00B67930 the new table, 008A7DD4 CMP byte
        // [ESI+0C8h],0 / JNE past 008A7DE7 00414DB0 (the world refresh), then 008A7DF9
        // 0088BA30 with EDX = ESI+0CCh+20h, the world matrix's forward row +ECh..+F4h;
        // one result. No null or class test.
        // SUBSTITUTIONS (labelled): the row is GameUnitsHost::unit_pose's forward row
        // for the entity's units-host slot (the host's poses are never stale, so the
        // refresh is not reached, as for GetPosition). A scene marker (no basis in
        // this host) or an entity with no slot pushes nothing and records
        // UNIMPLEMENTED, where the image reads +ECh of whatever object it is handed.
        if constexpr (kLuaGetDirectionBound) {
            ++direction_calls_;
            void* entity = entity_at(0);   // the body's own 00888AA0 read
            const std::size_t index = index_of(entity);
            float right[3], up[3], forward[3], position[3];
            if (entity != nullptr && index < units_.count() &&
                units_.unit_pose(index, right, up, forward, position)) {
                // Goal 5 chain 660: the body lua_get_direction (src/engine_seg56_cc13.cpp)
                // over DirectionQueryBinding; its push is 0088BA30's {x, y, z} table.
                RepairBindingArgs args(*this);
                DirectionQueryBinding query(units_, [this](void* e) { return index_of(e); },
                    [this](void* e) {
                        if (entity_pose_stale_008a7c24(e)) entity_pose_refresh_00414db0(e);
                    });
                DirectionResults pushed([this](const bsp::cc13_seg56::Vec3& v) {
                    const float xyz[3] = {v.x, v.y, v.z};
                    push_vector3_table_0088ba30(xyz);
                });
                const int count = bsp::cc13_seg56::lua_get_direction(args, pushed, query);
                results = state_ != nullptr ? count : 0;
            } else {
                ++direction_unresolved_;
                log_.unimplemented("LuaNative::GetDirection(entity without a unit slot)",
                    "008a7cb0");
                results = 0;
            }
        }
    } else if (std::strcmp(binding->name, "CountdownTimeLeft") == 0) {
        // 008B1B40 computes the time left without testing +3Ch.
        ++countdown_time_left_reads_;
        push_number_float_00b66480(countdown_time_left());
        results = state_ != nullptr ? 1 : 0;
    } else if (std::strcmp(binding->name, "Blackout") == 0) {
        // 008D142F..008D1612 reads the frame; the marshalling stays here and the
        // decode, the arm and the immediate step are src/mission_blackout.cpp.
        bsp::MissionBlackoutLuaCall call;
        call.argument_count = count();
        call.enable = get_boolean(0);
        if (call.argument_count > 1) call.callback = get_string(1);
        if (call.argument_count > 2) {
            call.duration_is_boolean
                = lua_type(state_, stack_slot(2)) == LUA_TBOOLEAN;  // 00B66000
            if (call.duration_is_boolean) {
                call.duration_boolean = get_boolean(2);
            } else {
                call.duration_number = static_cast<float>(get_number(2));
            }
        }
        if (call.argument_count > 3) {
            call.level_number = static_cast<float>(get_number(3));
        }
        if (call.argument_count <= 2
            || (call.duration_is_boolean && !call.duration_boolean)) {
            // The one path that consumes *(00432650() + E0h), which this packet
            // did not read. Reported rather than silently taken as zero.
            blackout_configured_duration_used_ = true;
        }
        bsp::mission_blackout_binding_008d1340(call, blackout_configured_duration_,
            blackout_, *this);
        ++blackout_summary_.arms;
        blackout_summary_.level = blackout_.level;
        blackout_summary_.remaining = blackout_.remaining;
        log_.notef("  Blackout(%s, \"%s\") target=%.3f remaining=%.4f",
            call.enable ? "true" : "false", call.callback.c_str(),
            static_cast<double>(blackout_.target),
            static_cast<double>(blackout_.remaining));
        results = 0;  // 008D163E, nothing pushed
    } else {
        // 008a2f20, 008a2bc0 and 008a2d70 are one body with one command object.
        results = bsp::lua_binding_navigator_move_to(*this, *this);
    }

    ++summary_.calls;
    rows_.push_back(row);
    // `restore` puts row_, state_ and argument_count_ back here, as before.
    return results;
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: the argument reader
// ---------------------------------------------------------------------------

int GameScriptOrdersHost::count() { return argument_count_; }

int GameScriptOrdersHost::get_integer(int index) { return argument_integer(index); }

double GameScriptOrdersHost::get_number(int index) {
    if (state_ == nullptr) return 0.0;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return 0.0;
    if (lua_type(state_, slot) != LUA_TNUMBER) return 0.0;
    return static_cast<double>(lua_tonumber(state_, slot));
}

bool GameScriptOrdersHost::get_boolean(int index) { return argument_boolean(index); }

std::string GameScriptOrdersHost::get_string(int index) {
    if (state_ == nullptr) return std::string();
    const int slot = stack_slot(index);
    if (slot > argument_count_) return std::string();
    if (lua_type(state_, slot) != LUA_TSTRING) return std::string();
    const char* text = lua_tolstring(state_, slot, nullptr);
    return text != nullptr ? std::string(text) : std::string();
}

bool GameScriptOrdersHost::is_string(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    return slot <= argument_count_ && lua_type(state_, slot) == LUA_TSTRING;
}

bool GameScriptOrdersHost::is_nil(int index) {
    if (state_ == nullptr) return true;
    const int slot = stack_slot(index);
    return slot > argument_count_ || lua_type(state_, slot) == LUA_TNIL;
}

bool GameScriptOrdersHost::is_entity_table(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return false;
    lua_getfield(state_, slot, "Ptr");
    const bool has_ptr = lua_type(state_, -1) == LUA_TLIGHTUSERDATA;
    lua_settop(state_, argument_count_);
    if (has_ptr) return true;
    lua_getfield(state_, slot, "ID");
    const int id_type = lua_type(state_, -1);
    const bool has_id = id_type == LUA_TNUMBER || id_type == LUA_TSTRING;
    lua_settop(state_, argument_count_);
    return has_id;
}

void* GameScriptOrdersHost::entity_at(int index) {
    // 00888AA0 reads `Ptr`, the light userdata 00928A00 seeded. A script entity
    // this process created carries its own record pointer there; a created scene
    // instance carries the `ID`-derived pointer game_hosts_lua.cpp seeds, which
    // entity_from_argument already resolves.
    if (state_ != nullptr) {
        const int slot = stack_slot(index);
        if (slot <= argument_count_ && lua_type(state_, slot) == LUA_TTABLE) {
            lua_getfield(state_, slot, "Ptr");
            void* raw = (lua_type(state_, -1) == LUA_TLIGHTUSERDATA)
                ? lua_touserdata(state_, -1) : nullptr;
            lua_settop(state_, argument_count_);
            if (raw != nullptr && script_entity(raw) != nullptr) return raw;
        }
    }
    return entity_from_argument(index);
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: the script entities
// ---------------------------------------------------------------------------

GameScriptEntity* GameScriptOrdersHost::script_entity(void* handle) noexcept {
    for (GameScriptEntity& entity : script_entities_) {
        if (static_cast<void*>(&entity) == handle) return &entity;
    }
    return nullptr;
}

const GameScriptEntity* GameScriptOrdersHost::script_entity(void* handle) const noexcept {
    for (const GameScriptEntity& entity : script_entities_) {
        if (static_cast<const void*>(&entity) == handle) return &entity;
    }
    return nullptr;
}

bool GameScriptOrdersHost::build_script_self_table(const GameScriptEntity& entity) {
    // 00928A00's three seeded fields, the same shape game_hosts_lua.cpp builds for
    // the created scene instances: `ID` as a value, `Dead` false, `Ptr` as light
    // userdata. Here `Ptr` is the record's own address, which entity_at resolves.
    if (state_ == nullptr) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
        static_cast<int>(entity.id));
    lua_createtable(state_, 0, 3);
    // Corrected by packet cc_lua_find_entity: 00928BA5 seeds `ID` with the key
    // text through 00B67630's lua_pushlstring, and the shipped helpers index
    // thisTable with it.
    // 00B67630 calls native_lua_set_string_00b67630; OFF keeps the direct ID field write.
    if constexpr (kWireScriptOrdersHostBound) {
        ScriptOrdersLuaStackView table(state_, lua_gettop(state_));
        ScriptOrdersNativeText value(key);
        bsp::native_lua_set_string_00b67630(table.object, "ID", value.text);
    } else {
    lua_pushstring(state_, key);
    lua_setfield(state_, -2, "ID");
    }
    lua_pushboolean(state_, entity.dead ? 1 : 0);
    lua_setfield(state_, -2, "Dead");
    lua_pushlightuserdata(state_, const_cast<GameScriptEntity*>(&entity));
    lua_setfield(state_, -2, "Ptr");
    lua_setfield(state_, -2, key);
    lua_settop(state_, lua_gettop(state_) - 1);
    return true;
}

bool GameScriptOrdersHost::push_script_self_table(const GameScriptEntity& entity) {
    if (state_ == nullptr) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
        static_cast<int>(entity.id));
    lua_getfield(state_, -1, key);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 2);
        return false;
    }
    // Leave the slot on the stack and drop the table it came from.
    lua_insert(state_, -2);
    lua_settop(state_, lua_gettop(state_) - 1);
    return true;
}

void GameScriptOrdersHost::call_script_global(const GameScriptEntity& entity,
    const std::string& name, int stack_first, int stack_last) {
    // 009290A0 -> 00887E50 -> 00887750. The self-key block pushes
    // thisTable[entity+178h] as argument 1 (00887750 nargs starts at 1 only on this
    // path, docs/MISSION_NAMED_CALL_ARGS.md); a non-zero stack_first then appends
    // the caller's own Lua stack slots stack_first..stack_last.
    if (state_ == nullptr || name.empty()) return;
    const int base = lua_gettop(state_);
    // The native passes errfunc 0 (docs/MISSION_NAMED_CALL_ARGS.md) and keeps only
    // the message. This process installs `debug.traceback` as the handler purely
    // so a failure names the shipped-script call chain in the log; the traceback
    // is the executable's diagnostic, not a change to what the mission sees,
    // because a failed call still returns nothing either way.
    lua_getfield(state_, LUA_GLOBALSINDEX, "debug");
    int errfunc = 0;
    if (lua_istable(state_, -1)) {
        lua_getfield(state_, -1, "traceback");
        lua_remove(state_, -2);
        if (lua_isfunction(state_, -1)) {
            errfunc = lua_gettop(state_);
        } else {
            lua_settop(state_, base);
        }
    } else {
        lua_settop(state_, base);
    }
    lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(state_, -1)) {
        lua_settop(state_, base);
        return;
    }
    int pushed = 0;
    if (push_script_self_table(entity)) {
        ++pushed;
    } else {
        lua_createtable(state_, 0, 0);
        ++pushed;
    }
    if (stack_first != 0) {
        const int last = (stack_last < 0) ? base : stack_last;
        for (int slot = stack_first; slot <= last; ++slot) {
            lua_pushvalue(state_, slot);
            ++pushed;
        }
    }
    if (lua_pcall(state_, pushed, 0, errfunc) != 0) {
        ++timers_.call_failures;
        const char* message = lua_tolstring(state_, -1, nullptr);
        if (timers_.first_error.empty() && message != nullptr) {
            timers_.first_error = message;
        }
        log_.notef("  script call %s failed: %s", name.c_str(),
            message != nullptr ? message : "(no message)");
    }
    lua_settop(state_, base);
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: bsp::LuaBindingMissionHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::push_number(int value) {
    if (state_ == nullptr) return;
    // 00B664B0 calls native_lua_push_integer_00b664b0; OFF keeps lua_pushnumber.
    if constexpr (kWireScriptOrdersHostBound) {
        ScriptOrdersLuaStackView frame(state_, 1);
        bsp::native_lua_push_integer_00b664b0(frame.object, value);
    } else {
    lua_pushnumber(state_, static_cast<lua_Number>(value));
    }
}

void GameScriptOrdersHost::push_boolean(bool value) {
    if (state_ == nullptr) return;
    lua_pushboolean(state_, value ? 1 : 0);
}

void GameScriptOrdersHost::push_nil() {
    if (state_ == nullptr) return;
    lua_pushnil(state_);
}

void GameScriptOrdersHost::push_number_float_00b66480(float value) {
    if (state_ == nullptr) return;
    lua_pushnumber(state_, static_cast<lua_Number>(value));
}

float GameScriptOrdersHost::random_uniform_00bd2f10(float minimum, float maximum) {
    // 00BD2F10 draws from the thread's random state object (00BD2ED0). This
    // process has no such object, so the draw is this generator's: a 32-bit
    // linear congruential sequence with a fixed seed, which makes a headless run
    // reproducible. The distribution and the half-open bounds are the native's.
    ++random_draws_;
    random_state_ = random_state_ * 1664525u + 1013904223u;
    const float unit = static_cast<float>(random_state_ >> 8) / 16777216.0f;
    return minimum + (maximum - minimum) * unit;
}

bool GameScriptOrdersHost::unit_health_gate_5d_00923be4(void* entity) {
    // 00923BE4, [entity+5Dh]. Packet cc9_get_hp_percentage: bound, the unit's
    // death as the gunnery host records it (the destroy funnel sets +5Dh).
    // LABELLED: a scene marker has no death record and reads clear.
    if (!kUnitHealthFractionBound) {
        static_cast<void>(entity);
        return false;
    }
    const std::size_t index = index_of(entity);
    const GameGunneryHost* gunnery = units_.gunnery();
    if (index >= units_.count() || gunnery == nullptr) return false;
    return gunnery->unit_dead(index);
}

float GameScriptOrdersHost::unit_health_vtable_110_00923bf6(void* entity) {
    // The virtual at the entity's vtable +110h (00923BF1..00923BF7).
    if (!kUnitHealthFractionBound) {
        static_cast<void>(entity);
        log_.unimplemented("UnitInstance::health_vtable_110", "00923bf6");
        return 0.0f;
    }
    // Packet cc9_get_hp_percentage. Every unit vtable (00CF90B0, 00CFA778,
    // 00CFB738, 00CFC3D0, 00CFFA30, 00D01630, 00D09678, 00D0BF80, 00D0C648)
    // holds 00876260: FLD [+370h], FDIV [+36Ch], FSTP float. The gunnery host
    // keeps both fields per unit (health, max_health).
    ++health_reads_;
    const std::size_t index = index_of(entity);
    const GameGunneryHost* gunnery = units_.gunnery();
    if (index >= units_.count() || gunnery == nullptr) {
        // 0042BB50, the base entity's slot: FLD1.
        ++health_reads_marker_;
        log_.implemented("UnitInstance::health_vtable_110_base_0042bb50", "0042bb50");
        return 1.0f;
    }
    const std::vector<GameGunneryUnitRow>& rows = gunnery->unit_rows();
    if (index >= rows.size()) return 1.0f;
    if constexpr (kSquadronAverageHealthBound) {
        // 007EDC30, the squadron's vtable[110h]: 00923BE0 (dead -> 0, else
        // health / max health) over the current members, over +3C8h.
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != index) continue;
            std::vector<float> member_health;
            for (const std::size_t m : r.member_units) {
                if (m == bsp::kPlaneSquadronNoUnit || m >= rows.size()) continue;
                member_health.push_back(gunnery->unit_dead(m) ? 0.0f
                    : static_cast<float>(static_cast<double>(rows[m].health) /
                                         static_cast<double>(rows[m].max_health)));
            }
            ++health_reads_squadron_;
            log_.implemented("PlaneSquadron::average_health_007edc30", "007edc30");
            return bsp::squadron_average_health_007edc30(member_health.data(),
                static_cast<int>(member_health.size()), r.wing_count);
        }
    }
    if (gunnery->unit_dead(index)) ++health_reads_dead_;
    log_.implemented("UnitInstance::health_vtable_110_00876260", "00876260");
    return static_cast<float>(static_cast<double>(rows[index].health) /
                              static_cast<double>(rows[index].max_health));
}

void GameScriptOrdersHost::unit_health_cache_store_00923c16(void* entity, float value) {
    static_cast<void>(entity);
    static_cast<void>(value);
}

bool GameScriptOrdersHost::entity_pose_stale_008a7c24(void* entity) {
    // This process's instances publish their position every motion tick, so the
    // pose is never stale here and the refresh at 008A7C37 is not reached.
    static_cast<void>(entity);
    return false;
}

void GameScriptOrdersHost::entity_pose_refresh_00414db0(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("EntityPose::refresh_world", "00414db0");
}

bool GameScriptOrdersHost::entity_pose_translation_008a7c3c(void* entity, float out[3]) {
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    const std::size_t index = index_of(entity);
    const GameUnitRow* row = (index < units_.count()) ? units_.unit_row(index) : nullptr;
    if (row != nullptr) {
        out[0] = row->position[0];
        out[1] = row->position[1];
        out[2] = row->position[2];
        return true;
    }
    // Packet cc_lua_find_entity: 008A7C3C reads entity+0FCh, the world matrix
    // translation row, for whatever entity the argument resolved to. A scene
    // marker's is the composed `localframe` the scene file authored.
    const SceneMarker* marker
        = marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(entity)));
    if (marker == nullptr) return false;
    out[0] = marker->position[0];
    out[1] = marker->position[1];
    out[2] = marker->position[2];
    return true;
}

void GameScriptOrdersHost::push_vector3_table_0088ba30(const float xyz[3]) {
    if (state_ == nullptr) return;
    lua_createtable(state_, 0, 3);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[0]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyX);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[1]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyY);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[2]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyZ);
}

bool GameScriptOrdersHost::measure_is_imperial_0088d9bd() {
    // 00F88988. Nothing in this process writes it, so it keeps its zero and the
    // binding takes the metric arm, which is the executable's own state and not a
    // substitute for one.
    return measure_imperial_;
}

void GameScriptOrdersHost::push_global_path_value_00b672b0(const char* dotted_path) {
    // Corrected by packet cc_lua_find_entity. 00B672B0 does not walk the path:
    // its two callees are 00A672F0 `lua_checkstack(L, 1)` and 00A67A10
    // `lua_pushlstring(L, data, length)` over the NativeString the caller built
    // at 0088D9D8 / 0088DA0D, with the empty literal at 0108FF2C standing in for
    // a null data pointer (00B672CD). GetMeasure therefore answers the text key
    // "globals.kilometer" or "globals.mile" itself, which is what usn_2_java's
    // own luaMetric (line 923) compares against and what the "#Mission.Measure#"
    // substitution in its score line expects.
    if (state_ == nullptr || dotted_path == nullptr) return;
    lua_pushstring(state_, dotted_path);
}

float GameScriptOrdersHost::game_clock_seconds_008a93fe() { return mission_clock_; }

void* GameScriptOrdersHost::script_entity_create_00898841() {
    // The 0x1E4-byte allocation and 00928630's construct. This process keeps only
    // the fields the five script bindings and 00929460 read; every other byte of
    // the native block has no reader here.
    if (!kScriptEntityPoolUnboundedBound && script_entities_.size() >= kScriptEntityCapacity) {
        // 00898841 has no bound; this process does, because the records are handed
        // to Lua as light userdata and must not move.
        log_.notef("script entity cap %zu reached; CreateScript answers no entity "
            "from here on, which the native never does", kScriptEntityCapacity);
        return nullptr;
    }
    GameScriptEntity entity;
    entity.id = kScriptEntityIdBase + static_cast<std::uint32_t>(script_entities_.size());
    script_entities_.push_back(entity);
    ++timers_.scripts_created;
    return static_cast<void*>(&script_entities_.back());
}

void GameScriptOrdersHost::entity_set_think_script_name_0088a330(void* entity,
    const std::string& name) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        // A created scene instance, not a script entity: this process has no
        // think slot on those, so the registration is a record.
        log_.unimplemented("Entity::set_think_script_name", "0088a330");
        return;
    }
    // 0088A35B/0088A363 free and null the old name first; 0088A373 strdups the new
    // one; 0088A34B appends to the pending list only on the null-to-name edge.
    const bool was_null = script->think_name.empty();
    script->think_name = name;
    if (was_null && !name.empty()) {
        bsp::register_pending_think_entity_0088a240(think_pending_, script->id);
        ++timers_.think_registrations;
    }
    if (name.empty()) script->delay_armed = false;  // 0088A37F
}

void GameScriptOrdersHost::script_entity_vcall_98_0089892c(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("ScriptEntity::place_vtable_98", "0089892c");
}

void GameScriptOrdersHost::script_entity_call_00927610_00898932(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("ScriptEntity::call_00927610", "00898932");
}

int GameScriptOrdersHost::lua_stack_top_00b65eb0() {
    return state_ != nullptr ? lua_gettop(state_) : 0;
}

void GameScriptOrdersHost::entity_call_named_009290a0(void* entity,
    const std::string& name, int stack_first, int stack_last) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) return;
    script->created_for = name;
    build_script_self_table(*script);
    // Packet cc13-reconload, log-only: the luaInit window for the SetParty count.
    const bool luainit = name == "luaInit";
    if (luainit) ++luainit_depth_;
    call_script_global(*script, name, stack_first, stack_last);
    if (luainit) --luainit_depth_;
}

void GameScriptOrdersHost::note_recon_load_entries(std::size_t entries) noexcept {
    recon_load_entries_ = entries;
    recon_load_seen_ = true;
}

bool GameScriptOrdersHost::push_self_table_slot_008989f6(void* entity) {
    const GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) return false;
    return push_script_self_table(*script);
}

void GameScriptOrdersHost::entity_arm_think_delay_008982c9(void* entity, float seconds) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("Entity::arm_think_delay", "008982c9");
        return;
    }
    script->delay_seconds = seconds;
    script->delay_armed = true;
    ++script->delay_generation;
    ++timers_.waits_armed;
}

void GameScriptOrdersHost::entity_clear_think_name_008985a6(void* entity) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("Entity::clear_think_name", "008985a6");
        return;
    }
    script->think_name.clear();
    script->delay_armed = false;
    ++script->delay_generation;
    ++timers_.clears;
}

bool GameScriptOrdersHost::entity_flag_5e_00898bd9(void* entity) {
    const GameScriptEntity* script = script_entity(entity);
    return script != nullptr ? script->blocked_5e : false;
}

void GameScriptOrdersHost::entity_kill_00926d90(void* entity, int cause) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("MissionEntity::kill", "00926d90");
        return;
    }
    static_cast<void>(cause);
    // 00926D90 sets +5Fh and queues the entity for the on-killed dispatch at
    // 009273A0, which is where 00929800 sets thisTable[key].Dead and erases the
    // entity from both think lists (00929AA7, 00929AB2). The queue and the
    // dispatch are not run here; the three observable effects are.
    script->dead = true;
    script->blocked_5e = true;
    if (in_think_walk_) {
        // The kill reaches here from inside a think function, so the walk is
        // iterating the live list. The native unlinks a node under a cursor that
        // already captured its successor; this reconstruction walks a vector, so
        // the erase is held until the walk returns and the end state is the same.
        think_erase_after_walk_.push_back(script->id);
    } else {
        bsp::erase_think_entity_00928300(think_live_, script->id);
        bsp::erase_think_entity_00928300(think_pending_, script->id);
    }
    build_script_self_table(*script);
    ++timers_.deletes;
}

std::size_t GameScriptOrdersHost::kill_live_script_entities(int cause) {
    std::size_t killed = 0;
    for (GameScriptEntity& script : script_entities_) {
        if (script.dead) continue;
        entity_kill_00926d90(&script, cause);
        ++killed;
    }
    return killed;
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: bsp::EntityThinkHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::run_entity_think_00929150(std::uint32_t entity) {
    for (GameScriptEntity& script : script_entities_) {
        if (script.id != entity) continue;
        ++script.thinks;
        // 00929194: args = 0 and stack_first = 0, so the think function receives
        // only the self table the key resolves.
        call_script_global(script, script.think_name, 0, -1);
        return;
    }
}

void GameScriptOrdersHost::free_think_node_0092952c(const bsp::EntityThinkNode& node) {
    static_cast<void>(node);
}

bool GameScriptOrdersHost::gc_gate_predicate_0109cefc_vtable0c() {
    // Derived stats profile D685F4+0C selects BE2740, which returns AL=0.
    // Preserve that result; other publication and profile states remain unproved.
    return false;
}

void GameScriptOrdersHost::lua_run_string_006b8ad0(const char* chunk, int mode) {
    static_cast<void>(chunk);
    static_cast<void>(mode);
}

void GameScriptOrdersHost::splice_pending_into_live_00928380(bsp::EntityThinkList& live,
    const bsp::EntityThinkList& pending) {
    for (const bsp::EntityThinkNode& node : pending.nodes) live.nodes.push_back(node);
}

void GameScriptOrdersHost::clear_pending_00928330(bsp::EntityThinkList& pending) {
    pending.nodes.clear();
}

// ---------------------------------------------------------------------------
// Packet cc_mission_blackout: bsp::MissionBlackoutHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::blackout_icon_set_visible(bool visible) {
    // Widget vtable +34h at 005B99B7 and 005B9A3B. This process has no GUI page,
    // so the widget state is recorded and nothing is drawn.
    log_.implemented("BlackoutIcon::set_visible", "005b99b7");
    static_cast<void>(visible);
}

void GameScriptOrdersHost::blackout_icon_set_colour(const bsp::BlackoutFillColour& colour) {
    // Widget vtable +50h at 005B9A3B.
    log_.implemented("BlackoutIcon::set_colour", "005b9a3b");
    blackout_colour_ = colour;
}

void GameScriptOrdersHost::blackout_icon_get_colour(bsp::BlackoutFillColour& colour) {
    // Widget vtable +54h at 005B99C9. The native reads the widget's own colour;
    // this process answers with the last colour it was given, which starts at the
    // opaque black bsp::BlackoutFillColour defaults to.
    log_.implemented("BlackoutIcon::get_colour", "005b99c9");
    colour = blackout_colour_;
}

void GameScriptOrdersHost::mission_lua_call_named_00887e50(const std::string& name) {
    // 00887E50 at 005B9969 with self key 0, no argument record and no stack
    // range, so the call is `_G[name]()` with zero arguments
    // (docs/MISSION_NAMED_CALL_ARGS.md: nargs starts at 0 off the self-key path).
    log_.implemented("MissionLuaHost::call_named", "00887e50");
    ++blackout_summary_.callbacks;
    blackout_summary_.last_callback = name;
    lua_State* const machine = (state_ != nullptr) ? state_ : machine_state_;
    if (machine == nullptr || name.empty()) return;
    const int base = lua_gettop(machine);
    lua_getfield(machine, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(machine, -1)) {
        log_.notef("  blackout callback %s is not a global function", name.c_str());
        lua_settop(machine, base);
        return;
    }
    const std::string outer_poster = after_row9_poster_;
    if constexpr (kAfterRow9OrderQueueBound) after_row9_poster_ = name;
    const int call_status = lua_pcall(machine, 0, 0, 0);
    after_row9_poster_ = outer_poster;
    if (call_status != 0) {
        ++timers_.call_failures;
        const char* message = lua_tolstring(machine, -1, nullptr);
        if (timers_.first_error.empty() && message != nullptr) {
            timers_.first_error = message;
        }
        log_.notef("  blackout callback %s failed: %s", name.c_str(),
            message != nullptr ? message : "(no message)");
    } else {
        log_.notef("  blackout callback %s ran", name.c_str());
    }
    lua_settop(machine, base);
}

void* GameScriptOrdersHost::local_player_unit_00e188d8() {
    // 00E188D8, read at 005B9867 and 005B9893. This executable never publishes a
    // local player unit, so the interface request the completion arm would push
    // is not reached. It is a HUD transition, not a script gate.
    log_.unimplemented("Game::local_player_unit", "00e188d8");
    return nullptr;
}

bool GameScriptOrdersHost::interface_request_pending_005b66d0() {
    // 005B66D0's body: `*(manager+20h)` against 29h, 2Bh, 2Ch and 2Dh. This
    // process pushes no interface request, so no id is ever pending.
    log_.implemented("InGameInterfaceManager::request_pending", "005b66d0");
    return false;
}

void GameScriptOrdersHost::ingame_interface_store_1c_00644220(int value) {
    // 00644220's whole body is `[this+1Ch] = argument`, on `*(00E198C4 + 40h)`.
    log_.unimplemented("InGameInterface::store_1c", "00644220");
    static_cast<void>(value);
}

void GameScriptOrdersHost::push_interface_request_004cc460(int request_id, void* payload) {
    log_.unimplemented("FrontEndManager::push_interface_request", "004cc460");
    static_cast<void>(payload);
    if (request_id == bsp::kMissionBlackoutCompletionInterfaceRequest) {
        ++blackout_summary_.interface_requests;
    }
}

int GameScriptOrdersHost::game_session_kind_1fe4h() {
    // The same field game_session_mode() answers, `[00e188a8]+1fe4h`.
    return game_session_mode();
}

void GameScriptOrdersHost::session_broadcast_blackout_0076d310(float level, float duration) {
    // 0076D310 at 005B9C36, session message kind 2Bh. Offline, so unreachable.
    log_.unimplemented("Session::broadcast_blackout", "0076d310");
    static_cast<void>(level);
    static_cast<void>(duration);
}

void GameScriptOrdersHost::force_show_please_wait_screen_00e19698() {
    // 005B9A04..005B9A20 on the process-lifetime `_PleaseWait` screen. Only on
    // the `game+1FE4h != 0` arm, which this process never takes.
    log_.unimplemented("PleaseWaitScreen::force_show", "005b9a04");
}

void GameScriptOrdersHost::run_blackout_update(float step) {
    // 004C40F0 at 004C4290..004C429A: BSP_FrontEndScreen_Update(*(00E198C4+A4h),
    // game+21F0h) -> vtable 00CF0ED8 +20h = 005BC920 -> 005B9800 at 005BC9FC.
    // The native gates on the screen's visibility bytes +4h and +5h and on
    // `game+21F0h > 0` (005BC9B0); this process has neither a screen set nor a
    // separate scaled delta and steps with the frame delta it is given.
    if (machine_state_ == nullptr) return;
    if (!(step > 0.0f)) return;  // 005BC9B0, the COMISS against zero
    lua_State* const outer_state = state_;
    const int outer_argument_count = argument_count_;
    state_ = machine_state_;
    argument_count_ = 0;
    // 005BC920 runs 00735100 on [game+21E8h] (005BC9EF) before 005B9800.
    if constexpr (kLuaCountdownBound) run_countdown_update_00735100();
    if constexpr (kLuaMissionNarrativeBound) run_narrative_update_00735100(step);
    const bsp::MissionBlackoutStep record
        = bsp::mission_blackout_update_005b9800(blackout_, step, *this);
    ++blackout_summary_.updates;
    if (record.reached_target) ++blackout_summary_.completions;
    blackout_summary_.level = blackout_.level;
    blackout_summary_.remaining = blackout_.remaining;
    state_ = outer_state;
    argument_count_ = outer_argument_count;
}

void GameScriptOrdersHost::countdown_start_00733ff0(const std::string& text,
    float level, float seconds, const std::string& callback, std::vector<int> argument_refs) {
    // bsp.py show 00733FF0 --asm, 00734003..0073409D: call the empty minimap
    // begin, publish level/active/duration/current clock, copy the callback,
    // then destroy the OLD argument vector before adopting the caller's NEW
    // vector. These are the existing live countdown owner and Lua registry
    // references, not a second countdown or an unconsumed message snapshot.
    // New C++ ownership interface; the original entry is thiscall/RET14h.
    if (narrative_hud_ == nullptr || !narrative_hud_->begin_countdown_panel(text))
        log_.unimplemented("HudCountdown::begin", "005bca70");
    countdown_.level_40 = level;
    countdown_.active_3c = true;
    countdown_.duration_44 = seconds;
    countdown_.started_48 = mission_clock_;
    countdown_.callback_4c = callback;
    for (const int ref : countdown_.argument_refs_54) {
        if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
    }
    countdown_.argument_refs_54.clear();
    countdown_.argument_refs_54 = std::move(argument_refs);
}

float GameScriptOrdersHost::countdown_time_left() const noexcept {
    // 007340AC-007340D5 and 008B1B40: +44h - (clock - +48h), each FSUB on a
    // float operand, stored as a float, then 0 when it is below zero.
    float left;
    if constexpr (kPc24MissionLuaCc13Bound) {
        // 007340B5 FSUB (clock - +48h) and 007340B8 FSUBP each round at PC24.
        left = static_cast<float>(cc13x87::x87_sub24(
            countdown_.duration_44, cc13x87::x87_sub24(mission_clock_, countdown_.started_48)));
    } else {
        left = static_cast<float>(static_cast<double>(countdown_.duration_44)
            - (static_cast<double>(mission_clock_) - static_cast<double>(countdown_.started_48)));
    }
    return left < 0.0f ? 0.0f : left;
}

void GameScriptOrdersHost::countdown_stop_007340a0() {
    if (narrative_hud_ == nullptr || !narrative_hud_->clear_countdown_panel())
        log_.unimplemented("HudCountdown::end", "005bcab0");  // 007340E1
    countdown_.active_3c = false;                           // 007340EF
    countdown_.callback_4c.clear();                         // 007340F3-0073410D
    for (const int ref : countdown_.argument_refs_54) {     // 00734110-0073412E
        if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
    }
    countdown_.argument_refs_54.clear();
}

// Packet cc9_lua_mission_narrative. 00735100's narrative half (00735229-
// 007352E5), after the countdown, on [game+21E8h] with dt = game+21F0h:
//  - nothing showing (+14h clear) and an entry queued: 00733BB0 starts the head
//    (+14h = 1, +18h = 0, +28h = 0; 005B7390 sets +1Ch = 0.5 x DialogFadeTime,
//    +20h = characters x TempCharWait + TempWaitBase, +24h = DialogFadeTime from
//    Scripts/datatables/DialogGlobals.lua, read by 005BB130 at 005BB80B-005BB901;
//    the callback +2Ch and arguments +34h), and 00734D20 pops it;
//  - showing: state 0 raises the alpha +28h by dt / +1Ch to 1, then state 1;
//    state 1 counts +20h down by dt unless the text is "*", then state 2;
//    state 2 lowers the alpha by dt / +24h. The alpha is clamped to [0, 1]
//    (005B5D20 is the HUD). At alpha 0 in state 2, 00734140 ends it.
// SUBSTITUTIONS, labelled: the character count is the text widget's +ECh after
// localisation; this host has no text table, so it counts the argument's own
// characters (a key such as "missionglobals.obj_fail" counts its key). The
// HUD calls 005B7390's widget writes, 005B5D20 and 005B5D40 are records. The
// skip key 0E7h (00735123) is never pressed by the idle player. The step runs
// inside run_blackout_update's gate (step > 0).
// Packet cc9_lua_camera_state. 008BF6A0 and 008A7E60's angle tail: 0042D2E0
// (extract_camera_matrix_angles_0042d2e0) on the basis, then for each angle
// (008BF870-008BF8AF) f = float(a + 42 pi) [00D13268], f = float(fmod(f, 2 pi))
// [00CE3828], degrees = float(f / pi [00CE3D28] * 180 [00CE3D20]).
void GameScriptOrdersHost::basis_degrees_0042d2e0(const bsp::CameraMatrix& world,
                                                  float out[3]) {
    float a[3] = {0.0f, 0.0f, 0.0f};
    bsp::CameraMatrix basis = world;
    bsp::extract_camera_matrix_angles_0042d2e0(basis, a[0], a[1], a[2]);
    for (int i = 0; i < 3; ++i) {
        const float shifted = static_cast<float>(static_cast<double>(a[i]) + 131.9468994140625);
        const float wrapped = static_cast<float>(std::fmod(static_cast<double>(shifted),
                                                           6.2831854820251465));
        if constexpr (kPc24MissionLuaCc13Bound) {
            // 008BF8A3 FDIV qword pi and 008BF8A9 FMUL qword 180 each round at PC24.
            out[i] = static_cast<float>(cc13x87::x87_mul24(
                cc13x87::x87_div24(wrapped, 3.1415927410125732), 180.0));
        } else {
            out[i] = static_cast<float>(static_cast<double>(wrapped) / 3.1415927410125732 * 180.0);
        }
    }
}

// 008BF6A0 GetCameraState(): a new table with `Position` (0088BA30 of the
// camera node's world translation, after 00B6DB70 when +5Ch bit 2 is clear),
// `Rotation` (the basis angles in degrees, above) and `Zoom` = [00F889B4] /
// node+1C4h (the fov). The node is [game+19FCh], the Operator camera the
// host's mission camera publishes (bsp::mission_camera_publication, packet
// cc9_mission_camera). SUBSTITUTION, labelled: when no pose has been
// published (a controlled unit with no ship camera, such as JM08's HQ), the
// controlled unit's own world matrix stands in, with Zoom 1.0 (its fov is taken as
// [00F889B4] itself); with no controlled unit either, the identity at the origin.
// Goal 5 chain 663: the position, the angles in degrees and the zoom are the body
// lua_get_camera_state_008bf6a0's (CameraStateBinding); this function picks the node.
int GameScriptOrdersHost::run_get_camera_state_008bf6a0() {
    if (state_ == nullptr) return 0;
    bsp::CameraMatrix world = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float fov = bsp::kFovDivisor00f889b4;
    const char* source = "identity";
    bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
    if (node.ready) {
        if ((node.state.transform.valid_flags & 2u) == 0) {
            bsp::refresh_camera_world_00b6db70(node.state.transform);
        }
        world = node.state.transform.world;
        fov = node.state.projection.fov;
        source = "camera";
        ++camera_state_from_camera_;
    } else if (units_.controlled_bound() && units_.controlled_index() < units_.count()) {
        float right[3], up[3], forward[3], position[3];
        if (units_.unit_pose(units_.controlled_index(), right, up, forward, position)) {
            world = {right[0], right[1], right[2], 0.0f, up[0], up[1], up[2], 0.0f,
                     forward[0], forward[1], forward[2], 0.0f,
                     position[0], position[1], position[2], 1.0f};
            source = "controlled unit (stand-in)";
        }
        ++camera_state_stand_ins_;
    } else {
        ++camera_state_stand_ins_;
    }
    ++camera_state_calls_;
    CameraStateBinding camera(world, fov,
        [this, source](const bsp::cc13_seg56::CameraStateResult& state) {
            const float position[3] = {state.position.x, state.position.y, state.position.z};
            const float degrees[3] = {state.rotation.x, state.rotation.y, state.rotation.z};
            lua_newtable(state_);
            push_vector3_table_0088ba30(position);
            lua_setfield(state_, -2, "Position");                 // 00CE68B4
            push_vector3_table_0088ba30(degrees);
            lua_setfield(state_, -2, "Rotation");                 // 00D0D734
            lua_pushnumber(state_, static_cast<lua_Number>(state.zoom));
            lua_setfield(state_, -2, "Zoom");                     // 00D14988
            if (camera_state_calls_ <= 8) {
                log_.notef("  GetCameraState at %.2f s: %s position=(%.1f, %.1f, %.1f) "
                    "rotation=(%.1f, %.1f, %.1f) zoom=%.3f (008BF6A0 body, packet "
                    "cc9_lua_camera_state)", static_cast<double>(mission_clock_), source,
                    static_cast<double>(position[0]), static_cast<double>(position[1]),
                    static_cast<double>(position[2]), static_cast<double>(degrees[0]),
                    static_cast<double>(degrees[1]), static_cast<double>(degrees[2]),
                    static_cast<double>(state.zoom));
            }
        });
    return bsp::cc13_seg56::lua_get_camera_state_008bf6a0(camera, camera);
}

void GameScriptOrdersHost::run_narrative_update_00735100(float step) {
    if (!narrative_active_14_) {
        // 00735258 JZ 007353A2 (+14h clear), 007353A2..007353E3: with +0Ch
        // non-zero, the front node (checked against the head, 007353AD) goes
        // to 00733BB0 as (+0Ch text, +14h callback, +1Ch arguments) at
        // 007353C7, before the erase; +1Ch is then zeroed (007353CE) and
        // 00734D20 erases the front (007353E3). This host copies the two
        // strings out first and starts the entry below, after the erase.
        if (narrative_queue_size() == 0) return;
        NarrativeObjectServices services([this](const void* node) { release_narrative_node_refs(node); });
        std::uint8_t* const list = narrative_object_ + 0x04;
        std::uint8_t* const head = NarrativeObjectServices::load<std::uint8_t*>(narrative_object_, 0x08);
        std::uint8_t* const front = NarrativeObjectServices::load<std::uint8_t*>(head, 0x00);
        if (front == head) services.crt_invalid_parameter_noinfo_00bf6713();
        const auto native_text = [](const std::uint8_t* string) {
            const auto length = NarrativeObjectServices::load<std::uint32_t>(string, 0x00);
            const char* const data = NarrativeObjectServices::load<const char*>(string, 0x04);
            return data != nullptr ? std::string(data, length) : std::string();
        };
        const std::string head_text = native_text(front + 0x0C);
        const std::string head_callback = native_text(front + 0x14);
        std::vector<int> head_refs;
        if (const auto it = narrative_node_refs_.find(front); it != narrative_node_refs_.end()) {
            head_refs = std::move(it->second);
            narrative_node_refs_.erase(it);
        }
        NarrativeObjectServices::store<std::uint32_t>(front, 0x1C, 0u);    // 007353CE
        services.erase_00734d20(list, front);                             // 007353E3
        float fade = 0.5f, char_wait = 0.04f, wait_base = 1.0f;     // DialogGlobals.lua
        if (machine_state_ != nullptr) {
            const int top = lua_gettop(machine_state_);
            lua_getfield(machine_state_, LUA_GLOBALSINDEX, "DialogFadeTime");
            if (lua_type(machine_state_, -1) == LUA_TNUMBER)
                fade = static_cast<float>(lua_tonumber(machine_state_, -1));
            lua_getfield(machine_state_, LUA_GLOBALSINDEX, "TempCharWait");
            if (lua_type(machine_state_, -1) == LUA_TNUMBER)
                char_wait = static_cast<float>(lua_tonumber(machine_state_, -1));
            lua_getfield(machine_state_, LUA_GLOBALSINDEX, "TempWaitBase");
            if (lua_type(machine_state_, -1) == LUA_TNUMBER)
                wait_base = static_cast<float>(lua_tonumber(machine_state_, -1));
            lua_settop(machine_state_, top);
        }
        // 00733BB0 and 005B7390 (005B7434-005B7466).
        narrative_active_14_ = true;
        narrative_fade_in_1c_ = static_cast<float>(static_cast<double>(fade) * 0.5);
        if constexpr (kPc24MissionLuaCc13Bound) {
            // 005B744E FILD, 005B7454 FMUL and 005B7457 FADD each round at PC24.
            narrative_display_20_ = static_cast<float>(cc13x87::x87_add24(
                cc13x87::x87_mul24(static_cast<double>(head_text.size()), char_wait),
                wait_base));
        } else {
            narrative_display_20_ = static_cast<float>(
                static_cast<double>(head_text.size()) * char_wait + wait_base);
        }
        narrative_fade_out_24_ = fade;
        narrative_state_18_ = 0;
        narrative_alpha_28_ = 0.0f;
        narrative_text_ = head_text;
        narrative_callback_2c_ = head_callback;
        for (const int ref : narrative_refs_34_) {
            if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
        }
        narrative_refs_34_ = std::move(head_refs);
        ++narrative_shown_;
        log_.notef("  narrative shows \"%s\" at %.2f s: fade-in %.3f display %.3f fade-out %.3f "
            "callback \"%s\" (00733BB0 / 005B7390)", narrative_text_.c_str(),
            static_cast<double>(mission_clock_), static_cast<double>(narrative_fade_in_1c_),
            static_cast<double>(narrative_display_20_),
            static_cast<double>(narrative_fade_out_24_), narrative_callback_2c_.c_str());
        if (narrative_hud_ != nullptr && narrative_hud_->show_mission_narrative_text(narrative_text_)) {
            // Source delivery is available; the native sizing/timing
            // outputs of 005B7390 still need its measured text context.
            log_.unimplemented("HudNarrative::show_text_layout", "005b7390");
        } else {
            log_.unimplemented("HudNarrative::show_text", "005b7390");
        }
        return;   // 007353EB: MOV AL,1 and return after the pop
    }
    float top = 1.0f;   // 00D7A24C
    if (narrative_state_18_ == 0) {
        if constexpr (kPc24MissionLuaCc13Bound) {
            // 0073530B FDIVRP (1 / +1Ch), 0073530D FMUL dt, 00735311 FADD +28h, each
            // at PC24.
            narrative_alpha_28_ = static_cast<float>(cc13x87::x87_add24(
                cc13x87::x87_mul24(cc13x87::x87_div24(1.0, narrative_fade_in_1c_), step),
                narrative_alpha_28_));
        } else {
            narrative_alpha_28_ = static_cast<float>(
                (1.0 / static_cast<double>(narrative_fade_in_1c_)) * step + narrative_alpha_28_);
        }
        if (1.0f <= narrative_alpha_28_) narrative_state_18_ = 1;
    } else if (narrative_state_18_ == 1) {
        if (narrative_text_ != "*") {
            narrative_display_20_ = static_cast<float>(
                static_cast<double>(narrative_display_20_) - step);
        }
        if (narrative_display_20_ <= 0.0f) narrative_state_18_ = 2;
        narrative_alpha_28_ = top;
    } else if (narrative_state_18_ == 2) {
        if constexpr (kPc24MissionLuaCc13Bound) {
            // 0073528D FDIVRP (1 / +24h), 0073528F FMUL dt, 00735293 FSUBP from +28h,
            // each at PC24.
            narrative_alpha_28_ = static_cast<float>(cc13x87::x87_sub24(
                narrative_alpha_28_,
                cc13x87::x87_mul24(cc13x87::x87_div24(1.0, narrative_fade_out_24_), step)));
        } else {
            narrative_alpha_28_ = static_cast<float>(narrative_alpha_28_ -
                (1.0 / static_cast<double>(narrative_fade_out_24_)) * step);
        }
    }
    if (0.0f <= narrative_alpha_28_) {
        if (top < narrative_alpha_28_) narrative_alpha_28_ = top;
    } else {
        narrative_alpha_28_ = 0.0f;
    }
    if (narrative_hud_ == nullptr || !narrative_hud_->query_mission_narrative_text_colour())
        log_.unimplemented("HudNarrative::query_text", "005b5d20");
    if (0.0f < narrative_alpha_28_) return;
    if (narrative_state_18_ == 2) narrative_finish_00734140();
}

// 00734140: 005B5D40 (the HUD), then, when the callback name +2Ch is not empty,
// 00887E50(self 0, &+2Ch, +34h, 0, -1) on [game+1A08h] with +34h taken and
// zeroed; +14h is cleared on both arms (00734199, 0073419F). The name +2Ch is
// not cleared here; it is overwritten by the next start.
void GameScriptOrdersHost::narrative_finish_00734140() {
    if (narrative_hud_ == nullptr || !narrative_hud_->hide_mission_narrative_text())
        log_.unimplemented("HudNarrative::hide", "005b5d40");
    narrative_active_14_ = false;
    std::vector<int> refs;
    refs.swap(narrative_refs_34_);
    if (narrative_callback_2c_.empty() || machine_state_ == nullptr) {
        for (const int ref : refs) {
            if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
        }
        return;
    }
    const std::string name = narrative_callback_2c_;
    ++narrative_callbacks_;
    narrative_last_callback_ = name;
    log_.implemented("MissionLuaHost::call_named", "00887e50");
    lua_State* const machine = machine_state_;
    const int base = lua_gettop(machine);
    lua_getfield(machine, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(machine, -1)) {
        log_.notef("  narrative callback %s is not a global function", name.c_str());
    } else {
        for (const int ref : refs) lua_rawgeti(machine, LUA_REGISTRYINDEX, ref);
        lua_State* const outer_state = state_;
        const std::string outer_poster = after_row9_poster_;
        if constexpr (kAfterRow9OrderQueueBound) after_row9_poster_ = name;
        state_ = machine;
        const int call_status = lua_pcall(machine, static_cast<int>(refs.size()), 0, 0);
        state_ = outer_state;
        after_row9_poster_ = outer_poster;
        if (call_status != 0) {
            ++timers_.call_failures;
            const char* message = lua_tolstring(machine, -1, nullptr);
            if (timers_.first_error.empty() && message != nullptr) timers_.first_error = message;
            log_.notef("  narrative callback %s failed: %s", name.c_str(),
                message != nullptr ? message : "(no message)");
        } else {
            log_.notef("  narrative callback %s ran at %.2f s", name.c_str(),
                static_cast<double>(mission_clock_));
        }
    }
    lua_settop(machine, base);
    for (const int ref : refs) luaL_unref(machine, LUA_REGISTRYINDEX, ref);
}

// 00734FA0: 00734CE0 empties the list, 005B5D40 (the HUD), +14h = 0 and the
// shown entry's arguments +34h are released. The name +2Ch stays (it never
// fires: nothing is showing).
void GameScriptOrdersHost::narrative_clear_00734fa0() {
    // 00734FA7: 00734CE0 on the narrative object's list +4h; every freed node's
    // argument references go with it.
    NarrativeObjectServices services([this](const void* node) { release_narrative_node_refs(node); });
    services.narrative_list_clear_00734ce0(narrative_object_ + 0x04);
    if (narrative_hud_ == nullptr || !narrative_hud_->hide_mission_narrative_text())
        log_.unimplemented("HudNarrative::hide", "005b5d40");
    narrative_active_14_ = false;
    for (const int ref : narrative_refs_34_) {
        if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
    }
    narrative_refs_34_.clear();
}

// Goal 5 chain 643. The post bindings pass the text (variant 0's native string)
// and a native copy of the callback by pointer, and the extras vector or null,
// to the entry point's body on [game+21E8h]. The two temporaries here are
// actual native strings in the process pool, released after the body has copied
// them (00733C80) into its node. The body's value is 0 (see
// NarrativeObjectServices); the extras' references attach to the node the body
// wrote: back() for 00734870 and 00734E50, front() for 007349C0 when it grew
// the list. 007349C0 inserts nothing when the front already holds this text and
// callback (007349FC / 00734A1B), and those references are released.
void GameScriptOrdersHost::narrative_post(std::uint32_t entry_point, const std::string& text,
                                          const std::string& callback,
                                          std::vector<int> argument_refs) {
    bsp::NativeStringStorage& strings = game_native_string_process().strings();
    struct NativeTemporaries {
        bsp::NativeStringStorage& strings;
        bsp::NativeString text;
        bsp::NativeString callback;
        ~NativeTemporaries() {
            callback.release_to(strings);
            text.release_to(strings);
        }
    } temporaries{strings, {}, {}};
    temporaries.text.assign_0041e870(strings, text.c_str());
    temporaries.callback.assign_0041e870(strings, callback.c_str());

    NarrativeObjectServices services([this](const void* node) { release_narrative_node_refs(node); });
    const std::uint32_t before = narrative_queue_size();
    if (entry_point == bsp::cc13_seg56::kNarrativePost) {
        bsp::cc15_g24c::narrative_list_append_00734870(narrative_object_, &temporaries.text,
            &temporaries.callback, 0u, services);
    } else if (entry_point == bsp::cc13_seg56::kNarrativePostUrgent) {
        bsp::cc15_g24c::narrative_list_push_front_unless_same_007349c0(narrative_object_,
            &temporaries.text, &temporaries.callback, 0u, services);
    } else if (entry_point == bsp::cc13_seg56::kNarrativePostOverride) {
        bsp::cc15_g24c::narrative_list_replace_all_00734e50(narrative_object_, &temporaries.text,
            &temporaries.callback, 0u, services);
    } else {
        throw std::logic_error("MissionNarrative post: unknown queue entry point");
    }

    std::uint8_t* const head = NarrativeObjectServices::load<std::uint8_t*>(narrative_object_, 0x08);
    const void* node = nullptr;
    if (entry_point != bsp::cc13_seg56::kNarrativePostUrgent) {
        node = NarrativeObjectServices::load<std::uint8_t*>(head, 0x04);        // back()
    } else if (narrative_queue_size() != before) {
        node = NarrativeObjectServices::load<std::uint8_t*>(head, 0x00);        // front()
    }
    if (node == nullptr) {
        for (const int ref : argument_refs) {
            if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
        }
        return;
    }
    if (!argument_refs.empty()) narrative_node_refs_[node] = std::move(argument_refs);
}

std::uint32_t GameScriptOrdersHost::narrative_queue_size() const noexcept {
    return NarrativeObjectServices::load<std::uint32_t>(narrative_object_, 0x0C);
}

void GameScriptOrdersHost::release_narrative_node_refs(const void* node) {
    const auto it = narrative_node_refs_.find(node);
    if (it == narrative_node_refs_.end()) return;
    for (const int ref : it->second) {
        if (machine_state_ != nullptr) luaL_unref(machine_state_, LUA_REGISTRYINDEX, ref);
    }
    narrative_node_refs_.erase(it);
}

void GameScriptOrdersHost::run_countdown_update_00735100() {
    if (!countdown_.active_3c) return;  // 0073514E
    // 00735161-0073517D: the remaining time, rounded to a float.
    float left;
    if constexpr (kPc24MissionLuaCc13Bound) {
        // 00735174 FSUB (clock - +48h) and 00735177 FSUBP each round at PC24.
        left = static_cast<float>(cc13x87::x87_sub24(
            countdown_.duration_44, cc13x87::x87_sub24(mission_clock_, countdown_.started_48)));
    } else {
        left = static_cast<float>(static_cast<double>(countdown_.duration_44)
            - (static_cast<double>(mission_clock_) - static_cast<double>(countdown_.started_48)));
    }
    // 00735181-00735194: expire when 0 > left, or when left <= [00D7A218] = 0.0f.
    if (left > 0.0f) {
        // 00735196-007351B3: 005BCA80(left / +44h, left) on [[00E198C4]+58h]:
        // FST the left into the second argument, FDIVRP (left / duration, one
        // PC24 rounding) FSTP float into the first (goal 5 chain 662).
        const float fraction = static_cast<float>(cc13x87::x87_div24(left, countdown_.duration_44));
        if (narrative_hud_ == nullptr || !narrative_hud_->set_countdown_panel(fraction, left))
            log_.unimplemented("HudCountdown::update", "005bca80");
        return;
    }
    countdown_.active_3c = false;                          // 007351C6
    if (narrative_hud_ == nullptr || !narrative_hud_->clear_countdown_panel())
        log_.unimplemented("HudCountdown::end", "005bcab0");   // 007351D7
    ++countdown_expiries_;
    if (countdown_.callback_4c.empty()) return;            // 007351E1
    // 007351E5-0073520A: copy the name, assign "" to +4Ch, take +54h and zero it.
    const std::string name = countdown_.callback_4c;
    countdown_.callback_4c.clear();
    std::vector<int> refs;
    refs.swap(countdown_.argument_refs_54);
    // 0073521F: 00887E50(self 0, &name, arguments, 0, -1) on [game+1A08h], then
    // 00733D50 on a local (contract: unread); the references are released here.
    ++countdown_callbacks_;
    countdown_last_callback_ = name;
    log_.implemented("MissionLuaHost::call_named", "00887e50");
    lua_State* const machine = machine_state_;
    if (machine == nullptr) return;
    const int base = lua_gettop(machine);
    lua_getfield(machine, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(machine, -1)) {
        log_.notef("  countdown callback %s is not a global function", name.c_str());
    } else {
        for (const int ref : refs) lua_rawgeti(machine, LUA_REGISTRYINDEX, ref);
        // The same HUD step as the blackout callback, so orders it posts are
        // after row 9 as well.
        lua_State* const outer_state = state_;
        const std::string outer_poster = after_row9_poster_;
        if constexpr (kAfterRow9OrderQueueBound) after_row9_poster_ = name;
        state_ = machine;
        const int call_status = lua_pcall(machine, static_cast<int>(refs.size()), 0, 0);
        state_ = outer_state;
        after_row9_poster_ = outer_poster;
        if (call_status != 0) {
            ++timers_.call_failures;
            const char* message = lua_tolstring(machine, -1, nullptr);
            if (timers_.first_error.empty() && message != nullptr) {
                timers_.first_error = message;
            }
            log_.notef("  countdown callback %s failed: %s", name.c_str(),
                message != nullptr ? message : "(no message)");
        } else {
            log_.notef("  countdown callback %s ran at %.2f s", name.c_str(),
                static_cast<double>(mission_clock_));
        }
    }
    lua_settop(machine, base);
    for (const int ref : refs) luaL_unref(machine, LUA_REGISTRYINDEX, ref);
}

// Packet cc9_entity_dead, docs/ENTITY_DEAD_FLAG.md. In the image a damage
// death runs 00958A30 -> vtable[70h](1) (0077D1A0 -> 00926C80: +60h set, +70h =
// 1, queued on the destroy list 00F899A8). The next frame's 009273A0 flush
// dispatches vtable[74h] 00926390 (+5Dh = +60h = 1, then JMP vtable[7Ch]); the
// unit override (ship 006FD8B0, generic 0077D270) ends in 00929B60 ->
// 00929800, which sets the self table's `Dead` to true (00929896) and
// `KillReason` to the cause's name from 00E0CF04 (cause 1 is "harm"). Here the
// flush is taken to run before the frame's script thinks; that order inside
// the frame is an assumption, and a one-frame difference is the bound.
// Not modelled: 00929800's think-message branch (+1D8h; a unit has no think
// here), 0077D270's `LastBanto`, 00927F60's scoring, and the planes that hit
// the water, whose destroy path was not read.
void GameScriptOrdersHost::publish_unit_deaths_00929800() {
    if (!kEntityDeadBound || machine_state_ == nullptr) return;
    const std::vector<std::pair<std::size_t, float>> deaths = units_.destroyed_units();
    if (deaths.empty()) return;
    if (dead_published_.size() < units_.count()) dead_published_.resize(units_.count(), false);
    lua_State* const L = machine_state_;
    std::vector<bool> sunk(units_.count(), false);
    for (const auto& death : deaths) {
        if (death.first < sunk.size()) sunk[death.first] = true;
    }
    for (const auto& death : deaths) {
        const std::size_t index = death.first;
        if (index >= dead_published_.size() || dead_published_[index]) continue;
        if constexpr (kSquadronDeadOnLastMemberBound) {
            // Packet cc9_squadron_dead (docs/ENTITY_DEAD_FLAG.md section 8). The
            // squadron entity is killed only by 007F3970 when its LAST plane
            // leaves +3D0h (007F3A16 +3CCh == 0, then 007F3A2A 00926D90 Kill with
            // ECX still the squadron: 007ED260 never writes ECX), so its self
            // table's `Dead` waits for that. Here the squadron is fused with its
            // wing-0 plane, so a fused leader's death is held while another
            // member still flies.
            bool held = false;
            for (const bsp::PlaneSquadronHostRecord& record :
                 bsp::plane_squadron_registry().records()) {
                if (record.squadron_unit != index) continue;
                for (const std::size_t m : record.member_units) {
                    if (m != bsp::kPlaneSquadronNoUnit && m != index && m < sunk.size() &&
                        !sunk[m]) {
                        held = true;
                        break;
                    }
                }
                break;
            }
            if (held) continue;
        }
        dead_published_[index] = true;
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
        if (!lua_istable(L, -1)) {
            lua_settop(L, lua_gettop(L) - 1);
            continue;
        }
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
            static_cast<int>(index + 1));
        lua_getfield(L, -1, key);
        // Packet cc9_mission_end: only the unit's own slot. Its `Ptr` is the
        // unit id as light userdata (game_hosts_lua.cpp's 00928A00 seed); a
        // marker slot that shared the key before the marker ids moved had
        // its own, and c8883c235 wrote `Dead` into two of those.
        bool own_slot = false;
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "Ptr");
            void* const ptr = lua_touserdata(L, -1);
            own_slot = ptr == reinterpret_cast<void*>(static_cast<std::uintptr_t>(index + 1));
            // Packet cc9-ships58: 00928C80 has nulled this table's Ptr already.
            if constexpr (kEntityDeadAfterPtrNullBound) {
                if (ptr == nullptr) own_slot = true;
            }
            lua_settop(L, lua_gettop(L) - 1);
        }
        if (own_slot) {
            const GameUnitRow* const row = units_.unit_row(index);
            const std::string unit_name = row != nullptr ? row->name : std::string();
            // 00E0CF04[+70h]: none, harm, soft, sell, exitzone, landed, editor
            // (00CE43EC, 00D0E35C, 00D0E354, 00D19080, 00D19074, 00D0E34C,
            // 00D1906C). Packet cc9_plane_retreat_task: the cause is the one the
            // host's kill passed (4 for the retreat leave-map kill), 1 otherwise.
            static const char* const kKillReasons[] = {
                "none", "harm", "soft", "sell", "exitzone", "landed", "editor"};
            const int cause = units_.kill_cause_00926d90(index);
            const char* const reason = (cause >= 0 && cause < 7) ? kKillReasons[cause] : "harm";
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "Dead");
            lua_pushstring(L, reason);
            lua_setfield(L, -2, "KillReason");
            log_.notef("entity dead: unit=%zu \"%s\" died=%.2f s published=%.2f s Dead=true "
                "KillReason=%s (00929800)", index, unit_name.c_str(),
                static_cast<double>(death.second), static_cast<double>(units_.mission_clock()),
                reason);
        }
        lua_settop(L, lua_gettop(L) - 2);
    }
    log_.implemented("MissionEntity::on_killed_lua_00929800", "00929800");
}

// Packet cc9_landing_unload_latch, docs/SHIP_AI_OPEN_ITEMS.md 106. In the image the
// landing ship's own update 0074AF50 writes `LandingStarted` = true on its Lua self
// object (BSP_MissionEntity_LuaSelfObject 00927B40, BSP_LuaObject_SetBoolean 00B673A0
// at 0074B274..0074B2C1) and then 0074AD90 writes `LandingFinished` = true, in the
// frame the ramp is found fully down. Here the ship AI queues the unload on the pad
// model (only with kLandingShipUnloadBound) and this pass, which runs where the
// death flags are published, writes both; the one-frame order bound of
// publish_unit_deaths_00929800 applies.
void GameScriptOrdersHost::publish_landing_unloads_0074ad90() {
    bsp::BuildingPadModel* const pads = building_pads();
    if (pads == nullptr || machine_state_ == nullptr) return;
    const std::vector<int> unloads = pads->take_unloads();
    if (unloads.empty()) return;
    lua_State* const L = machine_state_;
    for (const int ship : unloads) {
        if (ship < 0) continue;
        lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
        if (!lua_istable(L, -1)) {
            lua_settop(L, lua_gettop(L) - 1);
            continue;
        }
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat, ship + 1);
        lua_getfield(L, -1, key);
        bool own_slot = false;
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "Ptr");
            own_slot = lua_touserdata(L, -1)
                == reinterpret_cast<void*>(static_cast<std::uintptr_t>(ship + 1));
            lua_settop(L, lua_gettop(L) - 1);
        }
        if (own_slot) {
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "LandingStarted");    // 0074B2B1
            lua_pushboolean(L, 1);
            lua_setfield(L, -2, "LandingFinished");   // 0074AD90
            const GameUnitRow* const row = units_.unit_row(static_cast<std::size_t>(ship));
            log_.notef("landing ship unload published: unit=%d \"%s\" at t=%.2f "
                "LandingStarted=true LandingFinished=true (0074B2B1, 0074AD90)", ship,
                row != nullptr ? row->name.c_str() : "", static_cast<double>(units_.mission_clock()));
        }
        lua_settop(L, lua_gettop(L) - 2);
    }
    log_.implemented("LandingShip::publish_landing_lua_0074ad90", "0074ad90");
}

bool GameScriptOrdersHost::DialogKeyLess::operator()(const std::string& a,
    const std::string& b) const noexcept {
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) {
        const int ca = std::tolower(static_cast<unsigned char>(a[i]));
        const int cb = std::tolower(static_cast<unsigned char>(b[i]));
        if (ca != cb) return ca < cb;
    }
    return a.size() < b.size();
}

namespace {
std::string lua_field_text(lua_State* L, int index, const char* field) {
    lua_getfield(L, index, field);
    std::string out;
    const int type = lua_type(L, -1);
    if (type == LUA_TSTRING || type == LUA_TNUMBER) out = lua_tostring(L, -1);
    else if (type == LUA_TBOOLEAN) out = lua_toboolean(L, -1) ? "true" : "false";
    else if (type == LUA_TNIL) out = "nil";
    else out = lua_typename(L, type);
    lua_settop(L, lua_gettop(L) - 1);
    return out;
}
}  // namespace

void GameScriptOrdersHost::observe_mission_end() {
    if (!kMissionEndBound || machine_state_ == nullptr) return;
    if (mission_end_.seen && mission_end_.end_mission_at >= 0.0f) return;
    lua_State* const L = machine_state_;
    lua_getfield(L, LUA_GLOBALSINDEX, "Mission");
    if (!lua_istable(L, -1)) {
        lua_settop(L, lua_gettop(L) - 1);
        return;
    }
    const int mission = lua_gettop(L);
    lua_getfield(L, mission, "EndMission");
    const bool end_flag = lua_toboolean(L, -1) != 0;
    lua_settop(L, mission);
    if (end_flag && mission_end_.end_mission_at < 0.0f) mission_end_.end_mission_at = mission_clock_;
    if (mission_end_.seen) {
        lua_settop(L, mission - 1);
        return;
    }
    // The image's end chain: every commandhelpers.lua end function writes
    // Mission.MissionStatus (true for the completed ones, false for the failed
    // ones; no other script writes it), so a boolean there means the first
    // luaMissionCompletedNew / luaMissionFailedNew has run, whether or not the
    // mission sets its own EndMission. Diagnostic only: a table read.
    lua_getfield(L, mission, "MissionStatus");
    const bool status_set = lua_type(L, -1) == LUA_TBOOLEAN;
    lua_settop(L, mission);
    if (!end_flag && !status_set) {
        lua_settop(L, mission - 1);
        return;
    }
    mission_end_.seen = true;
    bsp::game::harness_note_mission_end();   // harness only: --stop-after-end
    mission_end_.trigger = status_set ? "MissionStatus" : "EndMission";
    mission_end_.at_seconds = mission_clock_;   // the frame whose thinks set it
    const std::string status = lua_field_text(L, mission, "MissionStatus");
    mission_end_.status = status == "false" ? "failed" : status == "true" ? "completed" : status;
    // commandhelpers.lua defines luaMissionFailedNew twice; the later one (:10360)
    // is the one that runs, and its luaInitMissionEnd (:13643) fills
    // Mission.MissionEndParams. The earlier one's MissionFailParams is the fallback.
    lua_getfield(L, mission, "MissionEndParams");
    if (!lua_istable(L, -1)) {
        lua_settop(L, mission);
        lua_getfield(L, mission, "MissionFailParams");
    }
    if (lua_istable(L, -1)) {
        const int params = lua_gettop(L);
        mission_end_.fail_text = lua_field_text(L, params, "Text");
        lua_getfield(L, params, "Ent");
        if (lua_istable(L, -1)) {
            const std::string id = lua_field_text(L, lua_gettop(L), "ID");
            const int n = std::atoi(id.c_str());
            mission_end_.fail_entity = n > 0 ? name_of(reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(n))) : id;
        }
    }
    lua_settop(L, mission);
    lua_getfield(L, mission, "Objectives");
    if (lua_istable(L, -1)) {
        const int objectives = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, objectives) != 0) {
            if (lua_type(L, -2) == LUA_TSTRING && lua_istable(L, -1)) {
                const std::string level = lua_tostring(L, -2);
                const int list = lua_gettop(L);
                lua_pushnil(L);
                while (lua_next(L, list) != 0) {
                    if (lua_type(L, -2) == LUA_TNUMBER && lua_istable(L, -1)) {
                        const int num = static_cast<int>(lua_tonumber(L, -2));
                        const int obj = lua_gettop(L);
                        mission_end_.objectives.push_back(level + ":" + std::to_string(num)
                            + " Active=" + lua_field_text(L, obj, "Active")
                            + " Success=" + lua_field_text(L, obj, "Success"));
                    }
                    lua_settop(L, lua_gettop(L) - 1);   // the value; the key stays
                }
            }
            lua_settop(L, lua_gettop(L) - 1);
        }
    }
    lua_settop(L, mission - 1);
    log_.notef("mission end: %s at %.2f s status=%s text=\"%s\" entity=\"%s\"",
        mission_end_.trigger.c_str(),
        static_cast<double>(mission_end_.at_seconds), mission_end_.status.c_str(),
        mission_end_.fail_text.c_str(), mission_end_.fail_entity.c_str());
}

void GameScriptOrdersHost::sync_recon_level_tables_0077b0c0() {
    // Packet cc9_recon_level_table. Two image steps, applied at a sync point:
    //  1. 0077FAD0 at 0077FD9C..0077FDF1: unless the holder at +C0h has kind 3
    //     (a host unit has none), `thisTable[id].reconlevel = {}` through
    //     00927B40 and 00B67580.
    //  2. 00805AF0 at 00805B98..00805BD8: when a record's effective level
    //     (+8h when the force byte +10h is set, else +4h) differs from the one
    //     it had, [+2Ch]->slot0(party [+30h], old, new), which is 0077B0C0:
    //     unless [00E188A8]+1FE4h is 2, `reconlevel[party] = new` through
    //     00B67800 and 00B665D0 (two lua_pushnumber, lua_settable).
    // A record starts at level 0, so a party that never saw the unit has no
    // entry. An own unit is refreshed by 008065B0 with +1 each pass and so
    // reads 2 from the first pass (own records carry 2 in the triples).
    if (!kReconLevelTableBound) return;
    lua_State* const L = state_ != nullptr ? state_ : machine_state_;
    if (L == nullptr) return;
    const GameGunneryHost* gunnery = units_.gunnery();
    if (gunnery == nullptr) return;
    const bsp::ReconSensorPassState& pass = gunnery->recon_sensor_pass_state();
    const std::size_t count = units_.count();
    if (recon_table_made_.size() == count && recon_sync_generation_ == pass.passes) return;
    ++recon_syncs_;
    recon_sync_generation_ = pass.passes;
    if (recon_table_made_.size() < count) recon_table_made_.resize(count, false);
    const int top = lua_gettop(L);
    lua_getfield(L, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(L, -1)) {
        lua_settop(L, top);
        return;
    }
    const int self = lua_gettop(L);
    std::vector<int> sides;
    for (std::size_t u = 0; u < count; ++u) {
        const int s = units_.unit_side_0054(u);
        if (!pass.side_covered(s)) continue;
        if (std::find(sides.begin(), sides.end(), s) == sides.end()) sides.push_back(s);
    }
    for (std::size_t u = 0; u < count; ++u) {
        char key[16];
        std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
            static_cast<int>(u + 1));
        lua_getfield(L, self, key);
        if (!lua_istable(L, -1)) {
            lua_settop(L, self);
            continue;
        }
        const int unit_table = lua_gettop(L);
        if (!recon_table_made_[u]) {
            lua_getfield(L, unit_table, "reconlevel");
            const bool have = lua_istable(L, -1);
            lua_settop(L, unit_table);
            if (!have) {
                lua_createtable(L, 0, 0);
                lua_setfield(L, unit_table, "reconlevel");
                ++recon_tables_created_;
            }
            recon_table_made_[u] = true;
        }
        const bool dead = gunnery->unit_dead(u);
        const int own_side = units_.unit_side_0054(u);
        for (const int s : sides) {
            int level = 0;
            if (s == own_side) {
                level = dead ? 0 : 2;
            } else {
                const bsp::ReconDetectionLevel lv = pass.level(s, u);
                level = lv == bsp::ReconDetectionLevel::identified ? 2
                    : lv == bsp::ReconDetectionLevel::blip ? 1 : 0;
            }
            int& published = recon_levels_published_[std::make_pair(u, s)];
            if (published == level) continue;
            published = level;
            lua_getfield(L, unit_table, "reconlevel");
            if (lua_istable(L, -1)) {
                // 00B665D0 calls native_lua_set_integer_at_00b665d0; OFF keeps the Lua writes.
                if constexpr (kWireScriptOrdersHostBound) {
                    ScriptOrdersLuaStackView table(L, lua_gettop(L));
                    bsp::native_lua_set_integer_at_00b665d0(table.object, s, level);
                } else {
                lua_pushnumber(L, static_cast<lua_Number>(s));
                lua_pushnumber(L, static_cast<lua_Number>(level));
                lua_settable(L, -3);
                }
                ++recon_level_writes_;
            }
            lua_settop(L, unit_table);
        }
        lua_settop(L, self);
    }
    lua_settop(L, top);
}

void GameScriptOrdersHost::run_script_think_pass(float step) {
    state_ = machine_state_;
    argument_count_ = 0;
    if (kReconLevelTableBound) sync_recon_level_tables_0077b0c0();
    mission_clock_ += step;
    if (kDialogSequencerBound) dialog_tick(step);
    // Packet cc9_script_entity_pool, diagnostic only: BSP_ORIGIN_DIAG=1 logs, every
    // 30 s of mission time, the nearest live unit of each party to (0,0,0), the
    // point JM08's CheckInvasion polls with a 300 m radius (prcpjm08.lua line 758).
    static const bool origin_diag = [] {
        char* text = nullptr;
        std::size_t bytes = 0;
        if (_dupenv_s(&text, &bytes, "BSP_ORIGIN_DIAG") != 0) return false;
        const bool on = text != nullptr && text[0] != '0';
        std::free(text);
        return on;
    }();
    if (origin_diag && mission_clock_ >= origin_diag_next_) {
        origin_diag_next_ = mission_clock_ + 30.0f;
        for (int party = 0; party < 4; ++party) {
            float best = -1.0f;
            std::string best_name;
            for (std::size_t i = 0; i < units_.count(); ++i) {
                const GameUnitRow* row = units_.unit_row(i);
                if (row == nullptr || row->party != party || !units_.unit_alive_and_visible(i)) continue;
                float x = 0.0f, y = 0.0f, z = 0.0f;
                units_.unit_position_00fc(i, x, y, z);
                const float d = std::sqrt(x * x + z * z);
                if (best < 0.0f || d < best) { best = d; best_name = row->name; }
            }
            if (best >= 0.0f) {
                log_.notef("origin diag t=%.1f party=%d nearest=%s dist=%.1f",
                    mission_clock_, party, best_name.c_str(), best);
            }
        }
        if (state_ != nullptr) {
            static const char kChunk[] =
                "local o={x=0,y=0,z=0} local n=0 "
                "local s=luaGetShipsAroundCoordinate(o,300,PARTY_ALLIED,\"own\") "
                "if s then for k,v in pairs(s) do n=n+1 end end "
                "local c,best,bn=0,-1,\"-\" "
                "local r=recon and recon[PARTY_ALLIED] and recon[PARTY_ALLIED].own "
                "if r then for k,t in pairs(r) do for k2,u in pairs(t) do c=c+1 "
                "if (k==\"destroyer\" or k==\"cargo\" or k==\"landingship\" or k==\"cruiser\" or k==\"battleship\") and not u.Dead then "
                "local d=luaGetDistance(u,o) if best<0 or d<best then best=d bn=k..\":\"..tostring(u.Name) end end end end end "
                "return tostring(PARTY_ALLIED)..\" around=\"..n..\" own_entries=\"..c..\" nearest_ship=\"..bn..\" dist=\"..tostring(best)";
            const int top = lua_gettop(state_);
            if (luaL_loadbuffer(state_, kChunk, sizeof(kChunk) - 1, "origin_diag") == 0 &&
                lua_pcall(state_, 0, 1, 0) == 0) {
                const char* text = lua_tostring(state_, -1);
                log_.notef("origin diag lua t=%.1f party_allied=%s", mission_clock_,
                    text != nullptr ? text : "?");
            } else {
                const char* text = lua_tostring(state_, -1);
                log_.notef("origin diag lua error: %s", text != nullptr ? text : "?");
            }
            lua_settop(state_, top);
        }
    }
    std::vector<bsp::EntityThinkFields> fields;
    fields.reserve(script_entities_.size());
    for (const GameScriptEntity& script : script_entities_) {
        bsp::EntityThinkFields row;
        row.entity = script.id;
        row.initialised = script.initialised;
        row.blocked_5d = script.blocked_5d;
        row.blocked_5e = script.blocked_5e;
        row.blocked_60 = script.blocked_60;
        row.has_think_name = !script.think_name.empty();
        row.delay_armed = script.delay_armed;
        row.delay_seconds = script.delay_seconds;
        fields.push_back(row);
    }
    // kThinkDelayInWalkBound: the live nodes the walk will take on the timed
    // branch, with their delay and arm generation before the walk.
    struct WalkedDelay {
        std::uint32_t id;
        float before;
        std::uint32_t generation;
    };
    std::vector<WalkedDelay> walked;
    if constexpr (kThinkDelayInWalkBound) {
        for (const bsp::EntityThinkNode& node : think_live_.nodes) {
            for (std::size_t i = 0; i < fields.size(); ++i) {
                if (fields[i].entity != node.entity) continue;
                const bsp::EntityThinkBranch b = bsp::entity_think_branch(fields[i], step, 0.0f);
                if (b == bsp::EntityThinkBranch::TimedFire || b == bsp::EntityThinkBranch::TimedWait) {
                    walked.push_back({node.entity, fields[i].delay_seconds,
                                      script_entities_[i].delay_generation});
                }
                break;
            }
        }
    }
    in_think_walk_ = true;
    const bsp::EntityThinkRunSummary run = bsp::run_entity_think_list_00929460(step,
        think_countdown_, think_live_, think_pending_, fields, *this);
    in_think_walk_ = false;
    for (std::uint32_t id : think_erase_after_walk_) {
        bsp::erase_think_entity_00928300(think_live_, id);
        bsp::erase_think_entity_00928300(think_pending_, id);
    }
    think_erase_after_walk_.clear();
    // 009294C7-009294D7 updates +1E0h in place and never re-arms the flag; the
    // rule's own helpers own both, so the entity records follow them here.
    if constexpr (kThinkDelayInWalkBound) {
        // 009294C7..009294D7 wrote before - step ahead of the think; a think that
        // re-armed (SetWait) or cleared the delay overwrote it, so keep that value.
        for (const WalkedDelay& w : walked) {
            for (GameScriptEntity& script : script_entities_) {
                if (script.id != w.id) continue;
                if (script.delay_generation == w.generation) {
                    script.delay_seconds = bsp::entity_think_delay_after_step(w.before, step);
                } else {
                    ++timers_.delays_rearmed_in_walk;
                }
                break;
            }
        }
    } else {
        for (GameScriptEntity& script : script_entities_) {
            if (!script.delay_armed) continue;
            if (script.delay_seconds <= 0.0f) continue;
            script.delay_seconds = bsp::entity_think_delay_after_step(script.delay_seconds,
                step);
        }
    }
    ++timers_.passes;
    timers_.timed_fires += run.thinks_run;
    state_ = nullptr;
}

// Packet cc9_scene_race_and_script_identity. A CreateScript entity is class 3
// (00898892 MOV [ESI+C4h],EBX with EBX 3); its pass C slot +A4h (00D111DC) is
// 009295B0, which with +C0h null (009295D2..009295DC) calls 00928100 at 009297E4.
// 00928100 needs +178h, the self object 00928A00 (pass A, slot +9Ch 00D111D4)
// builds at 00928A36, then writes Race = +58h and Party = +54h as numbers
// (0092814B, 00928162): the base 00925CE0's -1 and 2, since no spawn descriptor
// reaches 00927050's kind-1 arm. Name only when +154h is set (00928179): nothing
// on CreateScript's path writes +154h/+158h, and it never calls the name setter
// slot +0Ch (004313D0), so there is no Name. Type is 00E0CD80[3] = SCRIPTENTITY.
// SUBSTITUTION: run on the first mission frame after the creation, not at the
// next fixed step's InitAll row 12.
void GameScriptOrdersHost::write_script_identity_fields(const GameScriptEntity& script,
    bool type) {
    lua_State* const state = state_ != nullptr ? state_ : machine_state_;
    if (state == nullptr) return;
    const int base = lua_gettop(state);
    lua_getfield(state, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
        static_cast<int>(script.id));
    if (lua_istable(state, -1)) {
        lua_getfield(state, -1, key);
        if (lua_istable(state, -1)) {
            lua_pushnumber(state, static_cast<lua_Number>(script.race));
            lua_setfield(state, -2, "Race");
            lua_pushnumber(state, static_cast<lua_Number>(script.party));
            lua_setfield(state, -2, "Party");
            if (type) {
                lua_pushstring(state, "SCRIPTENTITY");
                lua_setfield(state, -2, "Type");
                ++script_identity_writes_;
            }
        }
    }
    lua_settop(state, base);
}

bool GameScriptOrdersHost::set_script_entity_party_00928f50(void* entity, int party) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) return false;
    script->party = party;                     // 00923B92
    write_script_identity_fields(*script, false);  // 00928FD9, 00929046
    ++script_party_sets_;
    log_.implemented("ScriptEntity::set_party_race_lua_mirror", "00928f50");
    return true;
}

bool GameScriptOrdersHost::set_unit_party_00928f50(void* entity, int party) {
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return false;
    const GameUnitRow* row = units_.unit_row(index);
    const int was = row != nullptr ? row->party : -1;
    units_.set_unit_side_0054(index, party);   // 00923B92 +54h
    // 00928FD9 / 00929046: the Lua Race and Party, re-read off the entity.
    const std::size_t mirrored = party_mirror_ != nullptr
        ? party_mirror_(party_mirror_context_) : 0u;
    log_.notef("SetParty 008a8930: unit=%s party %d -> %d (vtable+2Ch 00951F30 -> 00928F50, "
        "Lua mirror on %zu slot(s), packet cc9_lua46_set_party_unit)",
        row != nullptr ? row->name.c_str() : "?", was, party, mirrored);
    log_.implemented("Unit::set_party_race_lua_mirror", "00928f50");
    return true;
}

void GameScriptOrdersHost::mirror_script_identity_00928100() {
    for (GameScriptEntity& script : script_entities_) {
        if (!script.identity_pending) continue;
        script.identity_pending = false;
        write_script_identity_fields(script, true);
        log_.implemented("ScriptEntity::InitAll pass C mirror_identity", "00928100");
    }
}

void GameScriptOrdersHost::run_script_timers(float step) {
    // Packet cc9_deck_tick_in_step: with the switch on, the fixed step runs the
    // deck tick at its owner's motion instead (bsp/game_hosts_fixed_step.hpp).
    if (!kDeckTickInFixedStepBound) run_air_ops_update_006cdc70(step);
    if (machine_state_ == nullptr) return;
    observe_mission_end();
    publish_unit_deaths_00929800();
    publish_landing_unloads_0074ad90();
    if (bsp::BuildingPadModel* pads = building_pads()) {
        // Packet cc9_landing_modes_3_4: the pad's observer handle (+1E4h) drops a
        // destroyed occupant. SUBSTITUTION, labelled: the handle's notification
        // slot (vtable 00CE75CC) was not read.
        for (const auto& death : units_.destroyed_units()) {
            pads->forget_unit(static_cast<int>(death.first));
        }
    }
    if constexpr (kSceneRaceAndScriptIdentityBound) {
        mirror_script_identity_00928100();
    }
    if (script_entities_.empty()) {
        run_blackout_update(step);
        return;
    }
    if constexpr (kScriptThinkOnFixedStep) {
        // 00875BB0's rule (src/in_mission_subsystem_tick.cpp): add the frame delta,
        // then one 0.05f step while the step does not exceed the accumulator.
        const float fixed = static_cast<float>(bsp::kFixedSimulationStepSeconds);
        think_step_accumulator_ += step;
        while (fixed <= think_step_accumulator_) {
            run_script_think_pass(bsp::kFixedSimulationStepFloat);  // 00875E64
            think_step_accumulator_ -= fixed;
        }
    } else {
        run_script_think_pass(step);
    }
    // Packet cc_mission_blackout. GGame::OnMove step 18 runs 004C40A0, which
    // drives the fixed-step fan-out and so the think walk, and only then 004C40F0,
    // which steps the fade. Keeping that order here means a `luaDelay` that
    // expires on this frame arms its blackout before the same frame steps it,
    // exactly as the native does.
    run_blackout_update(step);
}

// Packet cc13-fix16: the squadron a squadron native's argument names, found as
// GetSquadronPlanes finds it (the registry record holding the argument's unit).
const bsp::PlaneSquadronHostRecord* GameScriptOrdersHost::squadron_record_for_argument(
    std::size_t index) {
    if (index >= units_.count()) return nullptr;
    return kGetSquadronPlanesDepartedLeadBound
        ? bsp::plane_squadron_registry().find_by_member_or_departed_unit(index)
        : bsp::plane_squadron_registry().find_by_member_unit(index);
}

namespace {

// 007BCC80 BSP_Weapon_DropFallTime (RET 4, __thiscall(unit, float height)) as the
// main thread runs it, x87 precision control 24-bit (docs/CC13_FPU_PRECISION.md):
// every x87 result below is rounded to float in the image's order. Read from the
// listing 007BCC80-007BCCEB:
//   007BCC86 COMISS 0, height / JC: height above 0 OR unordered continues, so a NaN
//            height takes the arithmetic, not the 0 (FLDZ 007BCC8D).
//   007BCC9F vtable[34h] (007BBB70, the world velocity); 007BCCA4 FSUB [00E08E54]
//            (3.0f, read-only: three literal readers, no writer), FSTP float.
//   007BCCB0-007BCCC2 FLD h, FADD ST0,ST0, FMUL qword [00CF9058] (9.81),
//            FLD ST1, FMULP ST2 (vy*vy), FADDP, FSTP float.
//   007BCCCA 00BF7030 sqrt, FSTP float; 007BCCD7 FADD float vy; 007BCCDA FDIV
//            qword [00CF9058]; 007BCCE0 FSTP float.
// The shared weapon_fall_time_007bcc80 (dive_bomb_task.cpp) rounds the
// discriminant once from double and returns 0 for a NaN height; this native keeps
// the image's 24-bit order and NaN arm. UNCERTAINTY: an x87 FMUL by a qword under
// 24-bit control rounds the exact product to float once; float(double * double)
// rounds twice, which can differ by one ulp on a tie.
float plane_bomb_fall_time_24bit_007bcc80(float height, float velocity_y) {
    if (height <= 0.0f) return 0.0f;   // ordered 0 >= height only
    const double gravity = bsp::dive_bomb_constant::kGravity;   // 00CF9058
    const float vy = velocity_y - bsp::dive_bomb_constant::kFallTimeVerticalBias;
    const float two_h = height + height;
    const float two_hg = static_cast<float>(static_cast<double>(two_h) * gravity);
    const float vy2 = vy * vy;
    const float disc = vy2 + two_hg;
    const float root = static_cast<float>(std::sqrt(static_cast<double>(disc)));
    const float sum = root + vy;
    return static_cast<float>(static_cast<double>(sum) / gravity);
}

// ---------------------------------------------------------------------------
// Goal 5 chain 660: the packet cc13-fix16 natives run their bodies
// (src/options_seg56_cc13.cpp, src/cc13_left55_seg56a.cpp, src/cc13_left55_seg56c.cpp,
// src/air_operations.cpp) over the units host's slot state. As for the chain 655
// unit queries, each adapter binds only what its body reaches and refuses the rest;
// the dispatch keeps its slot checks, because the image reads whatever object it is
// handed. Entities are the dispatch's identities: the table's `ID` (units-host index
// plus one) that 00888AA0 (entity_at) resolves.

// The image reads of PlaneForceRelease 0089EBA0 and SquadronForceRelease 0089E850.
// [plane + 9C0h + 8 * word[00F876B8]] is the record 007CDC70 published for the step
// slot, 007B9140(plane, 0): 1 for kind 17h (007B914A, MPlaneKamikaze), else some rack
// holds a round. SUBSTITUTION, labelled, as the units host takes it at 009A85EF and
// 009FA3F4 (game_hosts_units_motion.cpp): the host keeps no per-slot records and
// computes the answer live, not one step late. This view therefore holds that one
// record as its slot 0 and answers the slot word [00F876B8] (FixedStepClock::
// previous_buffer) with it, so the body's +9C0h + 8 * slot lands on the record; the
// word is an address into the view, not a game value the body passes on. The squadron
// reads +3CCh / +3D0h are the dispatch's record as TurnToPositionBinding reads them:
// its live members in order (squadron_live_members), each the member's index plus one.
class BombReleaseView : public bsp::seg56_options_cc13::ImageAccess {
public:
    BombReleaseView(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        void* argument, const bsp::PlaneSquadronHostRecord* squadron)
        : units_(units), index_of_(std::move(index_of)), argument_(argument),
          squadron_(squadron) {}

    std::uint16_t global_u16(std::uint32_t address) override {
        if (address != bsp::cc13_left55::seg56a::kLocalPlayerSlotGlobal) {
            unit_query_unreached("a word global other than [00F876B8]");
        }
        return 0;
    }
    std::uint8_t read_u8(void* plane, std::uint32_t offset) override {
        if (offset != 0x9C0u) unit_query_unreached("a byte other than the step record +9C0h");
        const std::size_t index = slot_of(plane);
        return (units_.unit_is_kind_of(index, 0x17)
                || units_.plane_holds_rack_round_007b9140(index)) ? 1 : 0;
    }
    std::int32_t read_i32(void* squadron, std::uint32_t offset) override {
        if (offset != 0x3CCu) unit_query_unreached("a dword other than squadron+3CCh");
        return squadron_of(squadron).live_count();                       // 0089E96B
    }
    void* read_ptr(void* squadron, std::uint32_t offset) override {
        if (offset < 0x3D0u || offset >= 0x3D0u + 4u * bsp::kPlaneSquadronMaxWings
            || (offset & 3u) != 0u) {
            unit_query_unreached("a pointer other than squadron+3D0h's slots");
        }
        const std::vector<std::size_t> live = squadron_live_members(squadron_of(squadron));
        const std::size_t slot = (offset - 0x3D0u) / 4u;
        if (slot >= live.size()) return nullptr;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(live[slot] + 1u));
    }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

    // 007BBBA0 on a plane: the units host's release request (packet cc13-fix16).
    void request_release(void* plane) {
        if (units_.request_ordnance_release_007bbba0(slot_of(plane))) ++requests_;
    }
    unsigned requests() const noexcept { return requests_; }

protected:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    const bsp::PlaneSquadronHostRecord& squadron_of(void* entity) const {
        if (squadron_ == nullptr || entity != argument_) {
            unit_query_unreached("a squadron read on an entity other than the resolved argument");
        }
        return *squadron_;
    }

private:
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    void* argument_;
    const bsp::PlaneSquadronHostRecord* squadron_;
    unsigned requests_{0};
};

// 0089EBA0 PlaneForceRelease: lua_plane_force_release over BombReleaseView.
class PlaneForceReleaseBinding final : public bsp::seg56_options_cc13::Seg56HostUnreached,
                                       public BombReleaseView {
public:
    using BombReleaseView::BombReleaseView;
    void unit_request_ordnance_release_007bbba0(void* plane) override { request_release(plane); }
};

// 0089E850 SquadronForceRelease: lua_squadron_force_release over BombReleaseView.
class SquadronForceReleaseBinding final
    : public bsp::cc13_left55::seg56a::Left55Seg56aHostUnreached, public BombReleaseView {
public:
    using BombReleaseView::BombReleaseView;
    void unit_request_ordnance_release_007bbba0(void* plane) override { request_release(plane); }
};

// The pushes PlaneBombFallingTime (00B66480 PushFloat at 008A55FC) and
// GetPlaneIsReloading (00B66450 PushBoolean) make onto the dispatch's own frame.
class BombReleaseResults final : public bsp::cc13_seg56::LuaResults {
public:
    BombReleaseResults(std::function<void(float)> push_float, std::function<void(bool)> push_boolean)
        : push_float_(std::move(push_float)), push_boolean_(std::move(push_boolean)) {}
    void push_float(float value) override { push_float_(value); }
    void push_boolean(bool value) override { push_boolean_(value); }
    void push_number(int) override { unit_query_unreached("00B664B0"); }
    void push_nil() override { unit_query_unreached("00B66430"); }
    void push_string(const std::string&) override { unit_query_unreached("00B66710"); }
    void push_integer_fields(const std::vector<std::pair<std::string, int>>&) override {
        unit_query_unreached("00B67930 integer fields");
    }
    void push_vector3(const bsp::cc13_seg56::Vec3&) override { unit_query_unreached("0088BA30"); }
    void push_vector3_fields(
        const std::vector<std::pair<std::string, bsp::cc13_seg56::Vec3>>&) override {
        unit_query_unreached("00B67930 vector fields");
    }

private:
    std::function<void(float)> push_float_;
    std::function<void(bool)> push_boolean_;
};

// 008A54C0 PlaneBombFallingTime: lua_plane_bomb_falling_time. +C8h is the pose-valid
// byte (008A55D6), answered from entity_pose_stale_008a7c24 as GetDirection takes it, and
// 00414DB0 is the host's refresh; +100h is the world translation y (unit_pose). 007BCC80's
// vtable[34h] (007BBB70, 007BCC9F) is the world velocity unit+AC8h that unit_linear_velocity
// answers, and the arithmetic is plane_bomb_fall_time_24bit_007bcc80 (24-bit order).
class BombFallTimeBinding final : public bsp::cc13_left55::seg56a::Left55Seg56aHostUnreached,
                                  public bsp::seg56_options_cc13::ImageAccess {
public:
    using PoseStale = std::function<bool(void*)>;
    using PoseRefresh = std::function<void(void*)>;
    BombFallTimeBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        PoseStale stale, PoseRefresh refresh)
        : units_(units), index_of_(std::move(index_of)), stale_(std::move(stale)),
          refresh_(std::move(refresh)) {}

    // --- ImageAccess ---
    std::uint8_t read_u8(void* plane, std::uint32_t offset) override {
        if (offset != 0xC8u) unit_query_unreached("a byte other than +C8h");
        static_cast<void>(slot_of(plane));
        return stale_(plane) ? 0 : 1;
    }
    float read_f32(void* plane, std::uint32_t offset) override {
        if (offset != 0x100u) unit_query_unreached("a float other than +100h");
        float right[3], up[3], forward[3], position[3];
        if (!units_.unit_pose(slot_of(plane), right, up, forward, position)) {
            unit_query_unreached("+100h on a unit without a pose");
        }
        return position[1];
    }
    std::int32_t read_i32(void*, std::uint32_t) override { unit_query_unreached("read_i32"); }
    void* read_ptr(void*, std::uint32_t) override { unit_query_unreached("read_ptr"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

    // --- Left55Seg56aHost ---
    void entity_pose_refresh_world_00414db0(void* entity) override { refresh_(entity); }
    float weapon_drop_fall_time_007bcc80(void* plane, float height) override {
        float velocity[3];
        if (!units_.unit_linear_velocity(slot_of(plane), velocity)) {
            unit_query_unreached("007BCC80 on a unit without a velocity");
        }
        return plane_bomb_fall_time_24bit_007bcc80(height, velocity[1]);
    }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    PoseStale stale_;
    PoseRefresh refresh_;
};

// 008BEBE0 GetPlaneIsReloading: lua_get_plane_is_reloading. vtable[5Ch](0Fh) is the
// units host's kind test; [entity+3D0h] (008BED0E) is the squadron's slot 0, the record's
// flight_leader() as the dispatch resolved it (null when that wing is gone); 007B92E0 is
// plane_any_rack_empty_007b92e0. SUBSTITUTION, labelled, as before: a fused squadron
// slot is its wing-0 plane, so it answers the 0Fh test as that plane.
class PlaneIsReloadingBinding final
    : public bsp::cc13_left55::seg56c::Left55Seg56cHostUnreached,
      public bsp::seg56_options_cc13::ImageAccess {
public:
    PlaneIsReloadingBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        void* argument, const bsp::PlaneSquadronHostRecord* squadron)
        : units_(units), index_of_(std::move(index_of)), argument_(argument),
          squadron_(squadron) {}

    // --- Left55Seg56cHost ---
    bool entity_vcall_5c(void* entity, int class_id) override {
        if (class_id != 0x0F) unit_query_unreached("vtable[5Ch] with a class other than 0Fh");
        return units_.unit_is_kind_of(slot_of(entity), class_id);
    }
    bool weapon_controller_any_slot_answers_007b92e0(void* plane) override {
        return units_.plane_any_rack_empty_007b92e0(slot_of(plane));
    }

    // --- ImageAccess ---
    void* read_ptr(void* entity, std::uint32_t offset) override {
        if (offset != 0x3D0u) unit_query_unreached("a pointer other than +3D0h");
        if (squadron_ == nullptr || entity != argument_) {
            unit_query_unreached("+3D0h on an entity other than the resolved squadron");
        }
        const std::size_t leader = squadron_->flight_leader();
        if (leader == bsp::kPlaneSquadronNoUnit) return nullptr;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(leader + 1u));
    }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    std::int32_t read_i32(void*, std::uint32_t) override { unit_query_unreached("read_i32"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("global_ptr"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("global_i32"); }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    void* argument_;
    const bsp::PlaneSquadronHostRecord* squadron_;
};

// 0089F080 PlaneReloadBombPlatforms -> 007ED650 at 0089F198: plane_reload_bomb_platforms_
// 0089f080 walks the squadron's +3D0h planes (+3CCh of them, no slot cap) and calls
// 007C1D80 on each (007ED667), which is the units host's plane_reload_bomb_platforms_
// 007c1d80. The planes are the record's live members, each its index plus one.
class ReloadBombPlatformsBinding final : public bsp::AirOperationsHost {
public:
    explicit ReloadBombPlatformsBinding(GameUnitsHost& units) : units_(units) {}

    void reload_plane_bomb_platforms(std::uint32_t plane) override {
        const std::size_t index = static_cast<std::size_t>(plane) - 1u;
        if (plane == 0u || index >= units_.count()) {
            unit_query_unreached("007C1D80 on a plane without a units-host slot");
        }
        const int changed = units_.plane_reload_bomb_platforms_007c1d80(index);
        if (changed > 0) racks_ += static_cast<std::size_t>(changed);
    }
    std::size_t racks() const noexcept { return racks_; }

    void release_squadron_slot(std::uint32_t, std::uint32_t) override {
        unit_query_unreached("006C65B0");
    }
    void kill_plane(std::uint32_t, std::int32_t) override { unit_query_unreached("00926D90"); }
    std::uint32_t build_stock_message(std::uint32_t, std::int32_t) override {
        unit_query_unreached("the stock-change message");
    }
    void send_message_to_peers(std::uint32_t, std::uint32_t) override {
        unit_query_unreached("0077C7B0");
    }
    void notify_stock_changed(std::uint32_t, std::uint32_t, std::int32_t) override {
        unit_query_unreached("00984EB0");
    }
    void route_command_message(std::int32_t, std::int32_t, float, bool) override {
        unit_query_unreached("0075B430 / 0077C2A0");
    }
    void run_air_ops_update(std::uint32_t) override { unit_query_unreached("006CDC70"); }
    std::uint32_t create_plane_squadron(std::uint32_t, const char*, const float*,
        const bsp::CatapultSpawnProperties&) override {
        unit_query_unreached("004F0AD0");
    }
    void bind_launched_squadron(std::uint32_t, std::uint32_t) override {
        unit_query_unreached("00694A60");
    }
    std::uint32_t attach_spawn_record(std::uint32_t, const bsp::CatapultSpawnProperties&) override {
        unit_query_unreached("00922E20");
    }
    void set_barrel_reload_timer(std::uint32_t, float) override {
        unit_query_unreached("BSP_Gun_SetBarrelReloadTimer");
    }

private:
    GameUnitsHost& units_;
    std::size_t racks_{0};
};

// 008A4F00 PilotBomb: lua_pilot_bomb. 007ED880(unit, 0) and the argument's vtable[5Ch](10h)
// are answered as the inline arm answered them (packet cc13-fix16 / cc13-fix21): the
// ordered unit's own ordnance OR its live squadron members' carries kind 2Fh, and a fused
// squadron slot (007EFB00, vtable 00D087C0+5Ch, never 10h) is the squadron. 0077D600 is the
// host's entity_issue_command, followed by the receiver's 0099A170 install.
class PilotBombBinding final : public bsp::seg56_options_cc13::Seg56HostUnreached {
public:
    using DropKamikaze = std::function<bool(void*)>;
    using IsSquadron = std::function<bool(void*)>;
    using Issue = std::function<void(void*, std::uint32_t, const bsp::SceneCommandTarget&, int)>;
    PilotBombBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        DropKamikaze drop_kamikaze, IsSquadron is_squadron, Issue issue)
        : units_(units), index_of_(std::move(index_of)), drop_kamikaze_(std::move(drop_kamikaze)),
          is_squadron_(std::move(is_squadron)), issue_(std::move(issue)) {}

    bool unit_has_drop_kamikaze_ordnance_007ed880(void* unit, std::int32_t loadout) override {
        if (loadout != 0) unit_query_unreached("007ED880 with a loadout other than 0");
        static_cast<void>(slot_of(unit));
        return drop_kamikaze_(unit);
    }
    bool entity_is_class_vt5c(void* entity, std::int32_t class_id) override {
        if (class_id != 0x10) unit_query_unreached("vtable[5Ch] with a class other than 10h");
        const std::size_t index = slot_of(entity);
        return !is_squadron_(entity) && units_.unit_is_kind_of(index, class_id);
    }
    void entity_issue_command_0077d600(void* entity, std::uint32_t command_object,
        const bsp::SceneCommandTarget& target, std::int32_t flags) override {
        issue_(entity, command_object, target, flags);
    }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    DropKamikaze drop_kamikaze_;
    IsSquadron is_squadron_;
    Issue issue_;
};

// PilotGunFire 008A50D0 (lua_pilot_gun_fire) and PilotTorpedo 008A5310 (lua_pilot_torpedo).
// GunFire's class tests run on the target object (008A5268): vtable[5Ch](18h) is a fused
// squadron slot (fused_squadron_record, as TurnToPositionBinding answers it) and (0Fh) the
// units host's kind test; neither gives dogfight 00E08F58. Otherwise [entity+3D0h] (the
// ordered squadron's member slot 0, its first live member plus one) answers (17h): kamikaze
// 00E08F50, else strafe 00E08F40 (docs/ATTACK_COMMANDS.md). 0077D600 is the host's issue.
// The command target's reader 0088A810 gives kind 1 with a non-null object for every
// entity target, so the handle-table read object_from_handle_inline is not reached.
class PilotAttackBinding final : public bsp::seg56_options_cc13::Seg56HostUnreached,
                                 public bsp::seg56_options_cc13::ImageAccess {
public:
    using Issue = std::function<void(void*, std::uint32_t, const bsp::SceneCommandTarget&, int)>;
    PilotAttackBinding(GameUnitsHost& units, std::function<std::size_t(void*)> index_of,
        Issue issue)
        : units_(units), index_of_(std::move(index_of)), issue_(std::move(issue)) {}

    // --- Seg56Host ---
    bool entity_is_class_vt5c(void* entity, std::int32_t class_id) override {
        const std::size_t index = slot_of(entity);
        if (class_id == 0x18) return fused_squadron_record(index) != nullptr;
        if (class_id == 0x0F || class_id == 0x17) return units_.unit_is_kind_of(index, class_id);
        unit_query_unreached("vtable[5Ch] with a class other than 18h, 0Fh or 17h");
    }
    void entity_issue_command_0077d600(void* entity, std::uint32_t command_object,
        const bsp::SceneCommandTarget& target, std::int32_t flags) override {
        issue_(entity, command_object, target, flags);
    }

    // --- ImageAccess: the ordered squadron's +3D0h slot 0 ---
    void* read_ptr(void* squadron, std::uint32_t offset) override {
        if (offset != 0x3D0u) unit_query_unreached("a pointer other than +3D0h");
        const bsp::PlaneSquadronHostRecord* record = fused_squadron_record(slot_of(squadron));
        if (record == nullptr) unit_query_unreached("+3D0h on an entity that is not a squadron");
        const std::vector<std::size_t> live = squadron_live_members(*record);
        if (live.empty()) return nullptr;
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(live.front() + 1u));
    }
    std::uint8_t read_u8(void*, std::uint32_t) override { unit_query_unreached("read_u8"); }
    float read_f32(void*, std::uint32_t) override { unit_query_unreached("read_f32"); }
    std::int32_t read_i32(void*, std::uint32_t) override { unit_query_unreached("read_i32"); }
    void write_u8(void*, std::uint32_t, std::uint8_t) override { unit_query_unreached("write_u8"); }
    void write_i32(void*, std::uint32_t, std::int32_t) override { unit_query_unreached("write_i32"); }
    void write_f32(void*, std::uint32_t, float) override { unit_query_unreached("write_f32"); }
    std::uint16_t global_u16(std::uint32_t) override { unit_query_unreached("global_u16"); }
    void* global_ptr(std::uint32_t) override { unit_query_unreached("the handle tables"); }
    void write_global_u8(std::uint32_t, std::uint8_t) override {
        unit_query_unreached("write_global_u8");
    }
    void write_global_f32(std::uint32_t, float) override {
        unit_query_unreached("write_global_f32");
    }
    std::int32_t global_i32(std::uint32_t) override { unit_query_unreached("the handle tables"); }

private:
    std::size_t slot_of(void* entity) const {
        const std::size_t index = index_of_(entity);
        if (index >= units_.count()) unit_query_unreached("an entity without a units-host slot");
        return index;
    }
    GameUnitsHost& units_;
    std::function<std::size_t(void*)> index_of_;
    Issue issue_;
};

}  // namespace

// Packet cc13-fix16 (docs/CC13_FIX16.md): PlaneForceRelease 0089EBA0,
// SquadronForceRelease 0089E850, PilotBomb 008A4F00, PlaneBombFallingTime
// 008A54C0, PlaneReloadBombPlatforms 0089F080 and GetPlaneIsReloading 008BEBE0.
// Each is __fastcall(ECX = the Lua state), RET, and reads argument 0 through
// 00888AA0 with no null test; the per-native evidence is on its switch in
// bsp/game_hosts_script_orders.hpp. An argument this host cannot resolve to a
// units-host slot (nil, a scene marker, a plane where a squadron is expected)
// pushes nothing and records UNIMPLEMENTED, where the image reads the fields of
// whatever object it was handed (and faults on a null one).
int GameScriptOrdersHost::run_bomb_release_native(GameScriptOrderRow& row, const char* name) {
    static_cast<void>(row);
    if (std::strcmp(name, "PlaneForceRelease") == 0) {
        if constexpr (kLuaPlaneForceReleaseBound) {
            ++plane_force_release_calls_;
            void* entity = entity_at(0);   // the body's own 00888AA0 read
            const std::size_t index = index_of(entity);
            if (entity == nullptr || index >= units_.count()) {
                ++plane_force_release_unresolved_;
                log_.unimplemented("LuaNative::PlaneForceRelease(entity without a unit slot)",
                    "0089eba0");
                return 0;
            }
            // Goal 5 chain 660: the body lua_plane_force_release (src/options_seg56_cc13.cpp)
            // over BombReleaseView. 0089ECBD: the step record byte +9C0h, 007B9140(plane, 0)
            // as 007CDC70 published it. SUBSTITUTION, labelled: taken live, not one step
            // late; the flag-0 arm equals plane_holds_rack_round_007b9140's flag-1 answer
            // in single player ([00E17BF2] = 0 at 005E2FB2), and 007B914A's kind-17h
            // arm (MPlaneKamikaze answers 1) is applied in the view.
            RepairBindingArgs args(*this);
            PlaneForceReleaseBinding binding(units_,
                [this](void* e) { return index_of(e); }, entity, nullptr);
            bsp::seg56_options_cc13::lua_plane_force_release(args, binding, binding);
            plane_force_release_requests_ += binding.requests();   // 0089ECC9
        }
        return 0;   // 0089ECD2 00B66400: nothing pushed
    }
    if (std::strcmp(name, "SquadronForceRelease") == 0) {
        if constexpr (kLuaSquadronForceReleaseBound) {
            ++squadron_force_release_calls_;
            resolve_plane_squadron_members();
            void* entity = argument_ptr_field(0);
            if (entity == nullptr) entity = entity_from_argument(0);
            const bsp::PlaneSquadronHostRecord* record =
                squadron_record_for_argument(index_of(entity));
            if (record == nullptr) {
                ++squadron_force_release_unresolved_;
                log_.unimplemented(
                    "LuaNative::SquadronForceRelease(entity without a squadron record)",
                    "0089e850");
                return 0;
            }
            // Goal 5 chain 660: the body lua_squadron_force_release (src/cc13_left55_
            // seg56a.cpp) over BombReleaseView. 0089E967-0089E9B1: each +3CCh member in
            // +3D0h order. SUBSTITUTION, labelled: the record's live member_units stand
            // for the compacted array (its kPlaneSquadronNoUnit holes skipped, as
            // GetSquadronPlanes skips them), and the record is the one this argument
            // resolves to; the byte is taken live, as for PlaneForceRelease.
            RepairBindingArgs args(*this);
            SquadronForceReleaseBinding binding(units_,
                [this](void* e) { return index_of(e); }, entity_at(0), record);
            bsp::cc13_left55::seg56a::lua_squadron_force_release(args, binding, binding);
            squadron_force_release_requests_ += binding.requests();   // 0089E9A0
        }
        return 0;   // 0089E9B7 00B66400: nothing pushed
    }
    if (std::strcmp(name, "PilotBomb") == 0) {
        if constexpr (kLuaPilotBombBound) {
            ++pilot_bomb_calls_;
            resolve_plane_squadron_members();
            void* unit = argument_ptr_field(0);
            if (unit == nullptr) unit = entity_from_argument(0);
            const std::size_t unit_index = index_of(unit);
            if (unit == nullptr || unit_index >= units_.count()) {
                ++pilot_bomb_unresolved_;
                log_.unimplemented("LuaNative::PilotBomb(entity without a unit slot)",
                    "008a4f00");
                return 0;
            }
            // Goal 5 chain 660: the body lua_pilot_bomb (src/options_seg56_cc13.cpp) reads
            // argument 0 (00888AA0), the target (0088A810 through this host's
            // LuaCommandTargetSource) and chooses the class; PilotBombBinding answers its
            // three calls.
            // 008A504C 007ED880(unit, 0): any of the +3CCh members at +3D0h carries
            // kind 2Fh (007B93E0). SUBSTITUTION, labelled, as choose_attack_class_
            // 007eec50 answers it: the ordered unit's own ordnance OR its live
            // squadron members' (gunnery liveness stands for +3CCh membership).
            const auto drop_kamikaze = [this](void* entity) {
                const std::size_t index = index_of(entity);
                bool drop = bsp::ordnance_has_drop_kamikaze_2fh(
                    bsp::OrdnanceKindSet{units_.unit_ordnance(index)});
                if (const bsp::PlaneSquadronHostRecord* sq =
                        bsp::plane_squadron_registry().find_by_member_unit(index)) {
                    auto* const gun = units_.gunnery();
                    for (const std::size_t m : sq->member_units) {
                        if (m == bsp::kPlaneSquadronNoUnit || m >= units_.count()) continue;
                        if (gun != nullptr && gun->unit_dead(m)) continue;
                        drop |= bsp::ordnance_has_drop_kamikaze_2fh(
                            bsp::OrdnanceKindSet{units_.unit_ordnance(m)});
                    }
                }
                return drop;
            };
            // 008A5053-008A5070: dropkamikaze 00E08F30, else vtable[5Ch](10h) gives
            // levelbomb 00E08F28 and its refusal divebomb 00E08F20 (the ordered
            // unit's class answers, as for 007EEC50's self_is_level_bomber).
            // Packet cc13-fix21: 008A505C-008A5065 `mov eax,[esi]; push 10h; mov
            // ecx,esi; call [eax+5Ch]` ask the ARGUMENT. A squadron's slot is
            // 007EFB00 (vtable 00D087C0+5Ch), which answers only 18h, 2, 1, 0 and
            // [this+C4h] (= 18h, 007F2DEC), so never 10h: a squadron argument gets
            // divebomb. SUBSTITUTION, labelled: a fused squadron slot (a registry
            // record's squadron_unit) is taken as the squadron, so its wing-0
            // plane named on its own cannot be told apart from it here; any other
            // slot answers with its own class (the plane arm).
            const auto is_squadron = [this](void* entity) {
                const std::size_t index = index_of(entity);
                for (const bsp::PlaneSquadronHostRecord& r :
                     bsp::plane_squadron_registry().records()) {
                    if (r.squadron_unit != bsp::kPlaneSquadronNoUnit
                        && r.squadron_unit == index) {
                        return true;
                    }
                }
                return false;
            };
            // 008A507F 0077D600(unit, class, &target, 1), with no test of the class
            // or the target: the order is issued whatever 007EEC50 would choose.
            const auto issue = [this](void* entity, std::uint32_t chosen,
                                      const bsp::SceneCommandTarget& target, int flags) {
                issue_pilot_attack_order(entity, chosen, target, flags, "PilotBomb",
                    pilot_bomb_issued_, pilot_bomb_tasks_);
                log_.notef("  PilotBomb: unit=%s target_object_id=%u class=%08lx "
                    "drop_kamikaze=%d (008A4F00, packet cc13-fix16)", name_of(entity).c_str(),
                    static_cast<unsigned>(target.object_id), static_cast<unsigned long>(chosen),
                    chosen == bsp::seg56_options_cc13::kCommandObjectDropKamikaze ? 1 : 0);
            };
            RepairBindingArgs args(*this);
            PilotBombBinding binding(units_, [this](void* e) { return index_of(e); },
                drop_kamikaze, is_squadron, issue);
            bsp::seg56_options_cc13::lua_pilot_bomb(args, *this, binding);
        }
        return 0;   // 008A5088 00B66400: nothing pushed
    }
    if (std::strcmp(name, "PlaneBombFallingTime") == 0) {
        if constexpr (kLuaPlaneBombFallingTimeBound) {
            ++bomb_falling_time_calls_;
            void* entity = entity_at(0);
            const std::size_t index = index_of(entity);
            float right[3], up[3], forward[3], position[3];
            float velocity[3];
            if (entity == nullptr || index >= units_.count()
                || !units_.unit_pose(index, right, up, forward, position)
                || !units_.unit_linear_velocity(index, velocity)) {
                ++bomb_falling_time_unresolved_;
                log_.unimplemented(
                    "LuaNative::PlaneBombFallingTime(entity without a unit slot)", "008a54c0");
                return 0;
            }
            // Goal 5 chain 660: the body lua_plane_bomb_falling_time (src/cc13_left55_
            // seg56a.cpp) over BombFallTimeBinding. 008A55D6 +C8h / 00414DB0, never stale
            // in this host (as GetDirection); 008A55E6 [plane+100h], the world translation
            // y; 007BCC95's vtable[34h] is the world velocity unit+AC8h, which
            // unit_linear_velocity answers.
            RepairBindingArgs args(*this);
            BombReleaseResults results([this](float value) { push_number_float_00b66480(value); },
                [this](bool value) { push_boolean(value); });
            BombFallTimeBinding binding(units_, [this](void* e) { return index_of(e); },
                [this](void* e) { return entity_pose_stale_008a7c24(e); },
                [this](void* e) { entity_pose_refresh_00414db0(e); });
            const int pushed = bsp::cc13_left55::seg56a::lua_plane_bomb_falling_time(args,
                results, binding, binding);
            return state_ != nullptr ? pushed : 0;
        }
        return 0;
    }
    if (std::strcmp(name, "PlaneReloadBombPlatforms") == 0) {
        if constexpr (kLuaPlaneReloadBombPlatformsBound) {
            ++reload_bomb_platforms_calls_;
            resolve_plane_squadron_members();
            void* entity = argument_ptr_field(0);
            if (entity == nullptr) entity = entity_from_argument(0);
            const bsp::PlaneSquadronHostRecord* record =
                squadron_record_for_argument(index_of(entity));
            if (record == nullptr) {
                ++reload_bomb_platforms_unresolved_;
                log_.unimplemented(
                    "LuaNative::PlaneReloadBombPlatforms(entity without a squadron record)",
                    "0089f080");
                return 0;
            }
            // Goal 5 chain 660: 007ED650 is plane_reload_bomb_platforms_0089f080
            // (src/air_operations.cpp) over ReloadBombPlatformsBinding: each +3CCh member
            // at +3D0h (no slot cap), 007C1D80 on it. The planes are the record's live
            // member_units (holes skipped), each its index plus one. The closing byte
            // squadron+3ECh = 1 (007ED67C) lands in a local and is not kept: a disp32
            // scan finds no byte reader of +3ECh in the squadron code (its other
            // writers are 007ED0D7, 007ED23E, 007ED6C9, 007F1D5D, 007F3A1D,
            // 007F4B6E), so no host state stands for it.
            std::vector<std::uint32_t> planes;
            for (const std::size_t m : record->member_units) {
                if (m == bsp::kPlaneSquadronNoUnit || m >= units_.count()) continue;
                planes.push_back(static_cast<std::uint32_t>(m + 1u));
            }
            ReloadBombPlatformsBinding binding(units_);
            bool flag_3ec = false;
            bsp::plane_reload_bomb_platforms_0089f080(binding, planes.data(),
                static_cast<std::int32_t>(planes.size()), &flag_3ec);
            reload_bomb_platforms_racks_ += binding.racks();
        }
        return 0;   // 0089F1A1 00B66400: nothing pushed
    }
    if (std::strcmp(name, "GetPlaneIsReloading") == 0) {
        if constexpr (kLuaGetPlaneIsReloadingBound) {
            ++plane_is_reloading_calls_;
            resolve_plane_squadron_members();
            void* entity = entity_at(0);
            const std::size_t index = index_of(entity);
            // 008BED04 vtable[5Ch](0Fh): a plane answers for itself; otherwise the
            // squadron's +3D0h slot 0 when non-null (008BED0E-008BED16).
            // SUBSTITUTION, labelled: a fused squadron slot is its wing-0 plane,
            // so it answers as that plane; once that plane is gone the record's
            // flight_leader() is not reached (the slot still answers 0Fh). The dispatch
            // refuses an argument that is neither, where the image reads +3D0h of
            // whatever object it was handed.
            const bsp::PlaneSquadronHostRecord* record = nullptr;
            if (entity != nullptr && index < units_.count()
                && !units_.unit_is_kind_of(index, 0x0F)) {
                record = squadron_record_for_argument(index);
                if (record == nullptr) entity = nullptr;
            }
            if (entity == nullptr || index >= units_.count()) {
                ++plane_is_reloading_unresolved_;
                log_.unimplemented(
                    "LuaNative::GetPlaneIsReloading(entity without a plane or squadron)",
                    "008bebe0");
                return 0;
            }
            // Goal 5 chain 660: the body lua_get_plane_is_reloading (src/cc13_left55_
            // seg56c.cpp) over PlaneIsReloadingBinding. 008BECFF the result byte starts 0
            // (a null +3D0h pushes false).
            RepairBindingArgs args(*this);
            bool reloading = false;
            BombReleaseResults results([](float) { unit_query_unreached("00B66480"); },
                [this, &reloading](bool value) {
                    reloading = value;
                    push_boolean(value);
                });
            PlaneIsReloadingBinding binding(units_, [this](void* e) { return index_of(e); },
                entity, record);
            const int pushed = bsp::cc13_left55::seg56c::lua_get_plane_is_reloading(args,
                results, binding, binding);
            if (reloading) ++plane_is_reloading_true_;
            return state_ != nullptr ? pushed : 0;
        }
        return 0;
    }
    return 0;
}

void GameScriptOrdersHost::issue_pilot_attack_order(void* entity, std::uint32_t chosen,
    const bsp::SceneCommandTarget& target, int flags, const char* label, std::size_t& issued,
    unsigned long long& tasks) {
    const std::size_t index = index_of(entity);
    const std::size_t target_index = target.object != nullptr
        ? index_of(target.object)
        : (target.object_id > 0 ? static_cast<std::size_t>(target.object_id - 1)
                                : ~static_cast<std::size_t>(0));
    entity_issue_command(entity, chosen, target, flags);   // 0077D600
    ++issued;
    install_pilot_attack_task_after_delivery(index, name_of(entity), chosen, target,
        target_index, label, tasks);
}

// Goal 5 chain 660. PilotGunFire 008A50D0 / PilotTorpedo 008A5310, __fastcall(ECX = the Lua
// state), RET: 00888AA0 on argument 0, 0088A810 on argument 1, then 0077D600(unit, class,
// &target, 1) at 008A52B6 / 008A5467; nothing pushed. Single-player callers:
// scripts/global/commandhelpers.lua 5507 PilotGunFire(spawned, plane) (the interceptors
// spawned against a far plane) and 5581 PilotTorpedo(this.Squad, enemyToAttack). The bodies
// are src/options_seg56_cc13.cpp's; as for PilotBomb, the order is followed by the
// receiver's 0099A170 install. LABELLED: an argument with no units-host slot, and a
// GunFire target this host cannot resolve to a unit (a position: the image's class tests
// then run on a null object, 008A5268), push nothing and are counted.
int GameScriptOrdersHost::run_pilot_gun_fire_or_torpedo(const char* name) {
    const bool gun_fire = std::strcmp(name, "PilotGunFire") == 0;
    ++(gun_fire ? pilot_gun_fire_calls_ : pilot_torpedo_calls_);
    resolve_plane_squadron_members();
    void* unit = entity_at(0);
    bool resolved = unit != nullptr && index_of(unit) < units_.count();
    if (resolved && gun_fire) {
        const bsp::SceneCommandTarget target = bsp::lua_read_command_target(*this, 1);
        const std::size_t target_index = index_of(target.object);
        const bool aircraft_target = target.kind != 0 && target_index < units_.count()
            && (fused_squadron_record(target_index) != nullptr
                || units_.unit_is_kind_of(target_index, 0x0F));
        // A surface target reads the ordered squadron's lead plane (008A527A).
        const bsp::PlaneSquadronHostRecord* squadron = fused_squadron_record(index_of(unit));
        resolved = target.kind != 0 && target_index < units_.count()
            && (aircraft_target
                || (squadron != nullptr && !squadron_live_members(*squadron).empty()));
    }
    if (!resolved) {
        ++(gun_fire ? pilot_gun_fire_unresolved_ : pilot_torpedo_unresolved_);
        log_.unimplemented(gun_fire ? "LuaNative::PilotGunFire(unresolved unit or target)"
                                    : "LuaNative::PilotTorpedo(entity without a unit slot)",
            gun_fire ? "008a50d0" : "008a5310");
        return 0;
    }
    PilotAttackBinding binding(units_, [this](void* e) { return index_of(e); },
        [this, gun_fire](void* entity, std::uint32_t chosen,
                         const bsp::SceneCommandTarget& target, int flags) {
            issue_pilot_attack_order(entity, chosen, target, flags,
                gun_fire ? "PilotGunFire" : "PilotTorpedo",
                gun_fire ? pilot_gun_fire_issued_ : pilot_torpedo_issued_,
                gun_fire ? pilot_gun_fire_tasks_ : pilot_torpedo_tasks_);
            log_.notef("  %s: unit=%s target_object_id=%u class=%08lx (%s, goal 5 chain 660)",
                gun_fire ? "PilotGunFire" : "PilotTorpedo", name_of(entity).c_str(),
                static_cast<unsigned>(target.object_id), static_cast<unsigned long>(chosen),
                gun_fire ? "008A50D0" : "008A5310");
        });
    RepairBindingArgs args(*this);
    return gun_fire ? bsp::seg56_options_cc13::lua_pilot_gun_fire(args, *this, binding, binding)
                    : bsp::seg56_options_cc13::lua_pilot_torpedo(args, *this, binding);
}

void GameScriptOrdersHost::report() {
    if (g_lua_callback_trace.installed) {
        for (const auto& first : g_lua_callback_trace.firsts) {
            log_.notef("lua callback trace first t=%.2f %s calls=%llu",
                static_cast<double>(first.second), first.first.c_str(),
                g_lua_callback_trace.counts[first.first]);
        }
        g_lua_callback_trace.clock = nullptr;
    }
    {
        std::string per;
        for (const auto& entry : deferred_by_poster_) {
            if (!per.empty()) per += ' ';
            per += entry.first + '=' + std::to_string(entry.second);
        }
        log_.notef("summary mission script navigator avoidance land=%zu torpedo=%zu disables=%zu "
        "delivered=%zu bound=%d (008A3B10 / 008A3CD0 -> 00835640, packet cc9_navigator_avoidance)",
        summary_.land_avoidance_orders, summary_.torpedo_evasion_orders,
        summary_.avoidance_disables, summary_.avoidance_delivered,
        kNavigatorAvoidanceDeliveryBound ? 1 : 0);
    log_.notef("summary mission script formation join loopback posted=%zu delivered=%zu "
        "joined=%zu bound=%d (0077C964 -> 0076E520 -> 0076C600 -> 0077F940, packet "
        "cc9_formation_join_loopback)", summary_.formation_joins_posted,
        summary_.formation_joins_delivered, summary_.formations_joined,
        kFormationJoinLoopbackBound ? 1 : 0);
    log_.notef("summary mission script navigator ship avoidance orders=%zu disables=%zu "
        "delivered=%zu bound=%d (008A3970 -> 008359C0 -> 00835640 +241h, packet "
        "cc9_navigator_ship_avoidance)", summary_.ship_avoidance_orders,
        summary_.ship_avoidance_disables, summary_.ship_avoidance_delivered,
        kNavigatorShipAvoidanceDeliveryBound ? 1 : 0);
    log_.notef("summary mission script after-row-9 order queue (packet "
            "cc9_after_row9_order_queue, 0076C600): bound=%d applied=%llu pending=%zu "
            "continuations=%llu deferred_by_poster: %s", kAfterRow9OrderQueueBound ? 1 : 0,
            deferred_applied_, deferred_orders_.size(), deferred_continuations_, per.empty() ? "(none)" : per.c_str());
    }
    log_.notef("summary mission script unit health reads bound=%d reads=%llu dead=%llu "
        "markers=%llu (00923BE0 -> 00876260, packet cc9_get_hp_percentage)",
        kUnitHealthFractionBound ? 1 : 0, health_reads_, health_reads_dead_,
        health_reads_marker_);
    if (health_reads_squadron_ != 0) {
        log_.notef("summary mission script squadron health reads bound=%d reads=%llu "
            "(007EDC30, packet cc9_l59_sq_avg_health)", kSquadronAverageHealthBound ? 1 : 0,
            health_reads_squadron_);
    }
    log_.notef("summary mission script put to bound=%d calls=%llu placed=%llu (008A9F90 -> "
        "008193A0, packet cc9_bsm01_think_natives)", kPutToBound ? 1 : 0, put_to_calls_,
        put_to_placed_);
    log_.notef("summary mission script squadron planes bound=%d calls=%llu entries=%llu "
        "unresolved=%llu (0089CC50, packet cc9_get_squadron_planes)",
        kGetSquadronPlanesBound ? 1 : 0, squadron_planes_calls_, squadron_planes_entries_,
        squadron_planes_unresolved_);
    log_.notef("summary mission script squadron intake bound=%d calls=%llu declined=%llu "
        "member_orders=%llu tasks=%llu (007F1940 settarget arm, packet "
        "cc9_ai_squadron_settarget_intake)", kAiSquadronSetTargetIntakeBound ? 1 : 0,
        squadron_intake_calls_, squadron_intake_declined_, squadron_intake_member_orders_,
        squadron_intake_tasks_);
    log_.notef("summary mission script kamikaze small-ship test bound=%d small_targets=%llu "
        "blocked=%llu (007EEB74, packet cc9_kamikaze_ship_blocked)",
        kKamikazeShipBlockedBound ? 1 : 0, kamikaze_small_targets_, kamikaze_blocked_);
    log_.notef("summary mission script pilot move on path bound=%d calls=%llu (008A3E70, "
        "packet cc9_pilot_move_on_path)", kPilotMoveOnPathBound ? 1 : 0,
        pilot_move_on_path_calls_);
    log_.notef("summary mission script pilot move to bound=%d calls=%llu marker_goals=%llu "
        "(008A4150 / 009BEBA0, packet cc9_pilot_move_to)", kPilotMoveToBound ? 1 : 0,
        pilot_move_to_calls_, pilot_marker_goals_);
    log_.notef("summary mission script recon level table bound=%d syncs=%llu tables=%llu "
        "writes=%llu force_recon=%llu (0077FAD0 / 0077B0C0 / 008AADF0, packet "
        "cc9_recon_level_table)", kReconLevelTableBound ? 1 : 0, recon_syncs_,
        recon_tables_created_, recon_level_writes_, force_recon_calls_);
    log_.notef("summary mission script submarine depth level bound=%d calls=%llu "
        "forced=%llu last=%d (00894100, packet cc9_submarine_depth_level)",
        kSubmarineDepthLevelBound ? 1 : 0, depth_level_calls_, depth_level_forced_,
        depth_level_last_);
    log_.notef("summary mission script fill path points bound=%d calls=%llu empty=%llu "
        "(0089A190, packet cc9_fill_path_points)", kFillPathPointsBound ? 1 : 0,
        fill_path_points_calls_, fill_path_points_empty_);
    if (kMissionEndBound) {
        // Packet cc9_mission_end. docs/MISSION_END.md: the image's end of a failed
        // single-player mission is EndScene 008B01B0 raising the restart prompt,
        // reached through the narrative callback this host does not fire.
        log_.notef("summary mission dialogs starts=%llu kills=%llu queries=%llu active_at_end=%zu",
            dialog_starts_, dialog_kills_, dialog_queries_, active_dialogs_.size());
        log_.notef("summary mission dialog sequencer bound=%d messages=%llu callbacks=%llu "
            "pauses=%llu finished=%llu missing_voice=%llu entries_at_end=%zu map=%s/%d "
            "(004527F0 on 005BBF10's tail timers, packet cc9_dialog_sequencer)",
            kDialogSequencerBound ? 1 : 0, dialog_messages_, dialog_callbacks_, dialog_pauses_,
            dialog_finished_, dialog_missing_voice_, dialog_entries_.size(),
            message_map_name_.c_str(), message_map_index_);
        log_.notef("summary message maps loaded=%llu unloaded=%llu without_manager=%llu "
            "dialog records resolved=%llu (008C6308 00706760, 008C65D8 00706FC0, "
            "00448416 00705E00, goal 4 chain 331)", message_maps_loaded_,
            message_maps_unloaded_, message_maps_without_manager_, dialog_records_resolved_);
        if (mission_end_.seen) {
            // Packet cc9_lua_mission_narrative: with the narrative bound the
            // callback fires, so the line says whether EndScene was reached.
            char end_scene[96];
            if (!kLuaMissionNarrativeBound) {
                std::snprintf(end_scene, sizeof(end_scene),
                    "not reached (the narrative callback is render-side)");
            } else if (end_scene_calls_ != 0) {
                std::snprintf(end_scene, sizeof(end_scene), "reached at %.2f s (recorded)",
                    static_cast<double>(end_scene_first_at_));
            } else {
                std::snprintf(end_scene, sizeof(end_scene), "not reached (narrative callbacks=%zu)",
                    narrative_callbacks_);
            }
            char end_flag[48];
            if (mission_end_.end_mission_at >= 0.0f) {
                std::snprintf(end_flag, sizeof(end_flag), "at %.2f s",
                    static_cast<double>(mission_end_.end_mission_at));
            } else {
                std::snprintf(end_flag, sizeof(end_flag), "never true");
            }
            log_.notef("summary mission end: %s at %.2f s (Mission.%s) text=\"%s\" "
                "entity=\"%s\" objectives=%zu; EndScene 008B01B0 %s; Mission.EndMission %s",
                mission_end_.status.c_str(),
                static_cast<double>(mission_end_.at_seconds), mission_end_.trigger.c_str(),
                mission_end_.fail_text.c_str(),
                mission_end_.fail_entity.c_str(), mission_end_.objectives.size(), end_scene,
                end_flag);
            for (const std::string& objective : mission_end_.objectives) {
                log_.notef("  objective %s", objective.c_str());
            }
        } else {
            log_.notef("summary mission end: none (Mission.EndMission never true)");
        }
    }
    // Packet cc8_airops_launch_tick.
    log_.notef("summary air ops tick slot_ticks=%llu refills_3_4_to_5=%llu "
        "releases_006c65b0=%llu slots_holding_a_squadron=%zu squadrons_created=%zu",
        air_ops_ticks_, air_ops_refills_, air_ops_released_, air_ops_tracking_,
        squadrons_.size());
    for (const AirOpsSquadron& made : squadrons_) {
        const GameUnitRow* row = made.unit_index < units_.count()
            ? units_.unit_row(made.unit_index) : nullptr;
        log_.notef("  squadron %-24s id=%u unit=%zu wing=%d active=%d pos=(%.0f %.0f %.0f)",
            made.name.c_str(), made.entity_id, made.unit_index, made.wing_count,
            (row != nullptr && row->active) ? 1 : 0,
            row != nullptr ? static_cast<double>(row->position[0]) : 0.0,
            row != nullptr ? static_cast<double>(row->position[1]) : 0.0,
            row != nullptr ? static_cast<double>(row->position[2]) : 0.0);
    }
    if (timers_.scripts_created != 0) {
        log_.notef("mission script timers (packet cc_lua_binding_audit): the delayed-call "
            "scheduler luaDelay -> CreateScript(\"luaDoTimeTable\") -> SetThink/SetWait, "
            "re-entered by the reconstructed think walk 00929460 once per mission frame");
        for (const GameScriptEntity& script : script_entities_) {
            log_.notef("  script entity %u created_for=%-20s think=%-16s armed=%d "
                "delay=%.2f thinks=%llu dead=%d", script.id,
                script.created_for.empty() ? "(none)" : script.created_for.c_str(),
                script.think_name.empty() ? "(none)" : script.think_name.c_str(),
                script.delay_armed ? 1 : 0,
                static_cast<double>(script.delay_seconds), script.thinks,
                script.dead ? 1 : 0);
        }
        log_.notef("summary mission script timers created=%zu think_registrations=%zu "
            "waits=%zu clears=%zu deletes=%zu passes=%zu fires=%llu failures=%llu %s",
            timers_.scripts_created, timers_.think_registrations, timers_.waits_armed,
            timers_.clears, timers_.deletes, timers_.passes, timers_.timed_fires,
            timers_.call_failures,
            timers_.first_error.empty() ? "" : timers_.first_error.c_str());
        if (bsp::BuildingPadModel* pads = building_pads()) {
            std::string per_building;
            for (std::size_t unit = 0; unit < units_.count(); ++unit) {
                const std::vector<int>& vec = pads->pads_of(static_cast<int>(unit));
                if (!units_.unit_is_kind_of(unit, 0x1C)) continue;
                const GameUnitRow* row = units_.unit_row(unit);
                char item[160];
                std::snprintf(item, sizeof(item), " %s=%zu",
                    row != nullptr ? row->name.c_str() : "?", vec.size());
                per_building += item;
            }
            log_.notef("summary building pads bound=1 buildings=%zu pads=%zu%s "
                "(006F5CC0 pass 3, packet cc9_building_pad_model)",
                pads->building_count(), pads->pad_count(), per_building.c_str());
        }
        log_.notef("summary mission script entity identity bound=%d written=%zu "
            "party_sets=%zu (00928100: Race +58h, Party +54h, Type SCRIPTENTITY; "
            "00928F50 on SetParty; packet cc9_scene_race_and_script_identity)",
            kSceneRaceAndScriptIdentityBound ? 1 : 0, script_identity_writes_,
            script_party_sets_);
    }
    // Packet cc13-reconload, log-only: K is the entry count of the scene load's
    // recon publication (00807A50), which precedes luaStageInit, so it is what
    // luaInit sees ON and what it would see OFF (the dry run); S counts SetParty
    // (008A8930) calls while a script entity runs `luaInit` (009290A0).
    // bound= is the switch itself; load_seen=0 means the scene-load block did
    // not run (hud, world_host or script_orders null), so K=0 there is no count.
    log_.notef("summary recon load refresh: bound=%d luainit_recon_entries=%zu "
        "luainit_setparty_calls=%zu load_seen=%d",
        kReconLoadForceRefreshCc13Bound ? 1 : 0, recon_load_entries_,
        luainit_setparty_calls_, recon_load_seen_ ? 1 : 0);
    if (blackout_summary_.arms != 0 || blackout_summary_.updates != 0) {
        log_.notef("summary mission blackout (packet cc_mission_blackout, 008D1340 / "
            "005B9BA0 / 005B9800): arms=%zu updates=%zu completions=%zu callbacks=%zu "
            "interface_requests=%zu last_callback=%s level=%.3f remaining=%.3f "
            "configured_default_used=%d",
            blackout_summary_.arms, blackout_summary_.updates,
            blackout_summary_.completions, blackout_summary_.callbacks,
            blackout_summary_.interface_requests,
            blackout_summary_.last_callback.empty()
                ? "(none)" : blackout_summary_.last_callback.c_str(),
            static_cast<double>(blackout_summary_.level),
            static_cast<double>(blackout_summary_.remaining),
            blackout_configured_duration_used_ ? 1 : 0);
    }
    if (countdown_starts_ != 0 || countdown_cancels_ != 0 || countdown_time_left_reads_ != 0) {
        log_.notef("summary mission countdown bound=%d starts=%zu cancels=%zu time_left_reads=%zu "
            "expiries=%zu callbacks=%zu last_callback=%s active_at_end=%d (008B16E0 / "
            "008B19A0 / 008B1B40 / 00735100, packet cc9_lua_countdown)",
            kLuaCountdownBound ? 1 : 0, countdown_starts_, countdown_cancels_,
            countdown_time_left_reads_, countdown_expiries_, countdown_callbacks_,
            countdown_last_callback_.empty() ? "(none)" : countdown_last_callback_.c_str(),
            countdown_.active_3c ? 1 : 0);
    }
    if (kLuaMissionNarrativeBound || narrative_enqueues_ != 0 || end_scene_calls_ != 0) {
        log_.notef("summary mission narrative bound=%d enqueues=%zu clears=%zu shown=%zu "
            "callbacks=%zu last_callback=%s queued_at_end=%zu showing_at_end=%d (008B0C10 / "
            "00735100 / 00734140, packet cc9_lua_mission_narrative)",
            kLuaMissionNarrativeBound ? 1 : 0, narrative_enqueues_, narrative_clears_,
            narrative_shown_, narrative_callbacks_,
            narrative_last_callback_.empty() ? "(none)" : narrative_last_callback_.c_str(),
            static_cast<std::size_t>(narrative_queue_size()), narrative_active_14_ ? 1 : 0);
        log_.notef("summary mission scene end calls=%zu first_at=%.2f (008B01B0, recorded; "
            "the run continues, packet cc9_lua_mission_narrative)", end_scene_calls_,
            static_cast<double>(end_scene_first_at_));
    }
    if (kLuaCameraStateBound || camera_state_calls_ != 0 || get_rotation_calls_ != 0) {
        log_.notef("summary mission camera state bound=%d calls=%zu from_camera=%zu stand_ins=%zu "
            "get_rotation=%zu unresolved=%zu (008BF6A0 / 008A7E60, packet cc9_lua_camera_state)",
            kLuaCameraStateBound ? 1 : 0, camera_state_calls_, camera_state_from_camera_,
            camera_state_stand_ins_, get_rotation_calls_, get_rotation_unresolved_);
    }
    if (capture_percentage_calls_ != 0) {
        log_.notef("summary mission script capture percentage bound=%d calls=%zu "
            "unresolved=%zu (0089B840 -> 006F1F90, packet cc9_lua_capture_percentage)",
            kLuaCapturePercentageBound ? 1 : 0, capture_percentage_calls_,
            capture_percentage_unresolved_);
    }
    if (command_building_level_calls_ != 0 || command_building_owner_calls_ != 0) {
        log_.notef("summary mission script command building level bound=%d calls=%zu "
            "unresolved=%zu owner_player bound=%d calls=%zu applied=%zu (0089B6D0 / "
            "008A8D90, packet cc13-fix13)", kLuaGetCommandBuildingLevelBound ? 1 : 0,
            command_building_level_calls_, command_building_level_unresolved_,
            kLuaSetCommandBuildingOwnerPlayerBound ? 1 : 0, command_building_owner_calls_,
            command_building_owner_applied_);
    }
    if (ship_max_speed_calls_ != 0 || direction_calls_ != 0) {
        log_.notef("summary mission script ship max speed bound=%d calls=%zu unresolved=%zu "
            "direction bound=%d calls=%zu unresolved=%zu (00890BB0 / 008A7CB0, packet "
            "cc13-fix14b)", kLuaGetShipMaxSpeedBound ? 1 : 0, ship_max_speed_calls_,
            ship_max_speed_unresolved_, kLuaGetDirectionBound ? 1 : 0, direction_calls_,
            direction_unresolved_);
    }
    if (role_table_calls_ != 0 || skill_query_calls_ != 0 || air_base_limit_calls_ != 0
        || shipyard_stock_calls_ != 0 || turn_to_position_calls_ != 0
        || pilot_stop_calls_ != 0 || air_base_slot_calls_ != 0) {
        log_.notef("summary mission script unit queries role tables=%zu skill/crew=%zu "
            "air base plane limit=%zu slot status=%zu shipyard stock=%zu turn to position=%zu "
            "(posed %zu) pilot stop=%zu (issued %zu) unresolved=%zu (008ABAC0 / 008ABF90 / "
            "008953F0 / 008950D0 / 00897720 / 008978D0 / 00896220 / 00896EE0 / 008A0F10 / "
            "008A7780, goal 5 chain 655)", role_table_calls_, skill_query_calls_,
            air_base_limit_calls_, air_base_slot_calls_, shipyard_stock_calls_,
            turn_to_position_calls_, turn_to_position_posed_, pilot_stop_calls_,
            pilot_stop_issued_, unit_query_unresolved_);
    }
    if (plane_force_release_calls_ != 0 || squadron_force_release_calls_ != 0 ||
        pilot_bomb_calls_ != 0 || bomb_falling_time_calls_ != 0 ||
        reload_bomb_platforms_calls_ != 0 || plane_is_reloading_calls_ != 0) {
        log_.notef("summary mission script bomb release natives (packet cc13-fix16): "
            "PlaneForceRelease 0089EBA0 calls=%zu requests=%zu unresolved=%zu | "
            "SquadronForceRelease 0089E850 calls=%zu requests=%zu unresolved=%zu | "
            "PilotBomb 008A4F00 calls=%zu issued=%zu tasks=%llu unresolved=%zu | "
            "PlaneBombFallingTime 008A54C0 calls=%zu unresolved=%zu | "
            "PlaneReloadBombPlatforms 0089F080 calls=%zu racks=%zu unresolved=%zu | "
            "GetPlaneIsReloading 008BEBE0 calls=%zu true=%zu unresolved=%zu",
            plane_force_release_calls_, plane_force_release_requests_,
            plane_force_release_unresolved_, squadron_force_release_calls_,
            squadron_force_release_requests_, squadron_force_release_unresolved_,
            pilot_bomb_calls_, pilot_bomb_issued_, pilot_bomb_tasks_, pilot_bomb_unresolved_,
            bomb_falling_time_calls_, bomb_falling_time_unresolved_,
            reload_bomb_platforms_calls_, reload_bomb_platforms_racks_,
            reload_bomb_platforms_unresolved_, plane_is_reloading_calls_,
            plane_is_reloading_true_, plane_is_reloading_unresolved_);
    }
    if (pilot_gun_fire_calls_ != 0 || pilot_torpedo_calls_ != 0) {
        log_.notef("summary mission script pilot attack natives (goal 5 chain 660): "
            "PilotGunFire 008A50D0 calls=%zu issued=%zu tasks=%llu unresolved=%zu | "
            "PilotTorpedo 008A5310 calls=%zu issued=%zu tasks=%llu unresolved=%zu",
            pilot_gun_fire_calls_, pilot_gun_fire_issued_, pilot_gun_fire_tasks_,
            pilot_gun_fire_unresolved_, pilot_torpedo_calls_, pilot_torpedo_issued_,
            pilot_torpedo_tasks_, pilot_torpedo_unresolved_);
    }
    if (rows_.empty()) return;
    log_.notef("the mission script's own orders, run through the eight reconstructed "
        "binding bodies (docs/LUA_BINDING_NAVIGATOR.md):");
    log_.notef("  %-26s %-16s %-10s %-16s %6s %6s", "binding", "unit", "command",
        "target", "issued", "slot");
    for (const GameScriptOrderRow& row : rows_) {
        if (row.command.empty()) continue;
        log_.notef("  %-26s %-16s %-10s %-16s %6d %6d", row.binding.c_str(),
            row.unit.c_str(), row.command.c_str(), row.target.c_str(),
            row.issued ? 1 : 0, row.reached_director ? 1 : 0);
    }
    log_.notef("summary mission script bindings calls=%zu attackmove=%zu moveto=%zu "
        "issued=%zu reached_director=%zu units=%zu formations=%zu/%zu skills=%zu "
        "repairs=%zu roles=%zu", summary_.calls, summary_.attack_moves,
        summary_.move_tos, summary_.issued, summary_.reached_director,
        summary_.units_ordered, summary_.formations_requested - summary_.formations_refused,
        summary_.formations_requested, summary_.skills, summary_.repairs, summary_.roles);
}

}  // namespace bsp::game

namespace bsp::game {

// ---------------------------------------------------------------------------
// Packet cc9_dialog_sequencer (docs/SQUADRON_LAND_TASK.md 5cs)
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::dialog_parse_entry(int table_slot, DialogEntry& entry) {
    // 004507D0: priority (+4h), requestTime (+8h, default [00F876A4]),
    // defaultPause (+Ch, default owner+30h, DialogDefaultPauseTime = 1.0 in this
    // installation's dialogglobals.lua), then `sequence`, one 0044BE50 command
    // per element. suppressInterruptMsg, nextUnit, breakRequested, lastPanel and
    // panelStates are read too and carry no timing; they are not kept.
    lua_State* L = state_;
    const int top = lua_gettop(L);
    auto number = [L, table_slot](const char* key, float fallback) {
        lua_getfield(L, table_slot, key);
        const float v = lua_type(L, -1) == LUA_TNUMBER
            ? static_cast<float>(lua_tonumber(L, -1)) : fallback;
        lua_settop(L, lua_gettop(L) - 1);
        return v;
    };
    entry.priority = number("priority", 0.0f);
    entry.request_time = number("requestTime", mission_clock_);
    entry.default_pause = number("defaultPause", 1.0f);
    lua_getfield(L, table_slot, "sequence");
    const int seq = lua_gettop(L);
    if (lua_type(L, seq) == LUA_TTABLE) {
        for (int i = 1;; ++i) {
            lua_rawgeti(L, seq, i);
            if (lua_type(L, -1) != LUA_TTABLE) break;
            const int el = lua_gettop(L);
            // 0044BE79..0044BF99: `type`, default "msg", compared without case.
            lua_getfield(L, el, "type");
            std::string type = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : "msg";
            lua_settop(L, el);
            DialogCommand cmd;
            auto text_of = [L, el](const char* key) {
                lua_getfield(L, el, key);
                std::string t = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : "";
                lua_settop(L, el);
                return t;
            };
            if (_stricmp(type.c_str(), "msg") == 0) {
                cmd.kind = 0;
                cmd.text = text_of("message");                 // 0044A440
            } else if (_stricmp(type.c_str(), "setpanel") == 0) {
                cmd.kind = 1;
            } else if (_stricmp(type.c_str(), "hidepanel") == 0) {
                cmd.kind = 2;
            } else if (_stricmp(type.c_str(), "pause") == 0) {
                cmd.kind = 3;
                lua_getfield(L, el, "time");                   // 0044A570, 00CE37A8
                cmd.value = lua_type(L, -1) == LUA_TNUMBER
                    ? static_cast<float>(lua_tonumber(L, -1)) : 0.0f;
                lua_settop(L, el);
            } else if (_stricmp(type.c_str(), "callback") == 0) {
                cmd.kind = 4;
                cmd.text = text_of("callback");                // 0044A5B0
            } else {
                lua_settop(L, seq);
                continue;   // 0044BFB7: no command object
            }
            entry.commands.push_back(cmd);
            lua_settop(L, seq);
        }
    }
    lua_settop(L, top);
}

std::pair<float, float> GameScriptOrdersHost::dialog_message_timing(const std::string& message) {
    // 00705E00 resolves the message record in the loaded message map, and
    // 005BBF10 holds a row until its clip's slot polls complete (key end 0) or
    // until the last key's end. LABELLED: the clip's playback length is the
    // streamed FSB4 header's sample count over its default frequency, read from
    // sound/messages/authentic/streamed_dialogs/<voice>.fsb (voice_dir authentic,
    // docs/APP_INIT_LOCALE.md); this process plays no voice.
    const auto cached = dialog_message_cache_.find(message);
    if (cached != dialog_message_cache_.end()) return cached->second;
    std::pair<float, float> timing{0.0f, 0.0f};
    std::string voice;
    bool from_record = false;
    // 004483F0 (src/panel_sequence.cpp request_panel_voice_004483f0):
    // 00448416 CALL 00705E00 on [game+21DCh] with the command's name and 0,
    // over the records 00706760 loaded for LoadMessageMap and 007065E0 for
    // Messages.lua. 005B94D0 (begin_panel_voice_record_005b94d0) then holds
    // the row for keys.back().end_14 when the record has keys. A miss is
    // 00705E00's own "<name>" record (kMessageRecordMissCc15Bound). The
    // voice is the record's +0Ch, 00704590's "voice" with its "tutorials"
    // language insert. LABELLED: a record with no keys takes 005B94D0's
    // text/voice-poll arm, which this host does not model (hold 0, as the
    // map-file read below gives for no subtitles). OFF, or with no manager,
    // keeps the host's own read of the last-loaded map file.
    if (message_manager_ != nullptr && message_manager_->constructed()) {
        const bsp::VoiceClipRecord* record = message_manager_->resolve_record_00705e00(message);
        if (record != nullptr) {
            if (!record->timed_keys_34.empty()) {
                timing.second = record->timed_keys_34.back().end_14;
            }
            const NativeString& name = record->auxiliary_0c;
            if (name.data() != nullptr && name.length() != 0) {
                voice.assign(name.data(), name.length());
            }
            from_record = true;
            ++dialog_records_resolved_;
        }
    }
    if (!from_record && state_ != nullptr && !message_map_name_.empty()) {
        std::string text;
        const std::string path = "scripts/datatables/messagemaps/" + message_map_name_ + ".lua";
        FILE* f = nullptr;
        if (fopen_s(&f, path.c_str(), "rb") == 0 && f != nullptr) {
            char buffer[4096];
            std::size_t n = 0;
            while ((n = std::fread(buffer, 1, sizeof(buffer), f)) > 0) text.append(buffer, n);
            std::fclose(f);
        }
        static const char kChunk[] =
            "local src, idx, msg = ... "
            "local f = loadstring(src) if not f then return nil end "
            "local env = {} setfenv(f, env) f() "
            "local m = env.Messages and env.Messages[idx] and env.Messages[idx][msg] "
            "if not m then return nil end "
            "local last = 0 "
            "if type(m.subtitles) == 'table' then for _, k in ipairs(m.subtitles) do "
            "if type(k[3]) == 'number' and k[3] > last then last = k[3] end end end "
            "return m.voice, last";
        const int top = lua_gettop(state_);
        if (!text.empty() &&
            luaL_loadbuffer(state_, kChunk, sizeof(kChunk) - 1, "dialog_message") == 0) {
            lua_pushlstring(state_, text.data(), text.size());
            lua_pushnumber(state_, static_cast<lua_Number>(message_map_index_));
            lua_pushstring(state_, message.c_str());
            if (lua_pcall(state_, 3, 2, 0) == 0) {
                if (lua_type(state_, -2) == LUA_TSTRING) voice = lua_tostring(state_, -2);
                if (lua_type(state_, -1) == LUA_TNUMBER) {
                    timing.second = static_cast<float>(lua_tonumber(state_, -1));
                }
            }
        }
        lua_settop(state_, top);
    }
    if (!voice.empty()) {
        const std::string path = "sound/messages/authentic/streamed_dialogs/" + voice + ".fsb";
        FILE* f = nullptr;
        if (fopen_s(&f, path.c_str(), "rb") == 0 && f != nullptr) {
            unsigned char head[0x68] = {};
            if (std::fread(head, 1, sizeof(head), f) == sizeof(head) &&
                std::memcmp(head, "FSB4", 4) == 0) {
                auto u32 = [&head](std::size_t at) {
                    return static_cast<std::uint32_t>(head[at]) |
                           (static_cast<std::uint32_t>(head[at + 1]) << 8) |
                           (static_cast<std::uint32_t>(head[at + 2]) << 16) |
                           (static_cast<std::uint32_t>(head[at + 3]) << 24);
                };
                const std::uint32_t samples = u32(0x50);   // first sample header +20h
                const std::uint32_t frequency = u32(0x64); // +34h, default frequency
                if (frequency != 0) timing.first = static_cast<float>(samples) / frequency;
            }
            std::fclose(f);
        }
    }
    if (timing.first <= 0.0f) ++dialog_missing_voice_;
    log_.notef("  dialog message %s voice=%s clip=%.3f key_end=%.3f", message.c_str(),
        voice.empty() ? "-" : voice.c_str(), static_cast<double>(timing.first),
        static_cast<double>(timing.second));
    dialog_message_cache_[message] = timing;
    return timing;
}

void GameScriptOrdersHost::dialog_step_00452740(const std::string& name) {
    // 00452360 first: setpanel / hidepanel / callback in order from the cursor.
    // A callback may start or kill dialogs, so the entry is looked up again
    // after each one (LABELLED: the image retains its entry pointer across the
    // callbacks, docs/PANEL_SEQUENCE.md).
    for (;;) {
        auto it = dialog_entries_.find(name);
        if (it == dialog_entries_.end()) return;
        DialogEntry& entry = it->second;
        if (entry.cursor >= entry.commands.size()) break;
        const DialogCommand cmd = entry.commands[entry.cursor];
        if (cmd.kind != 1 && cmd.kind != 2 && cmd.kind != 4) break;
        ++entry.cursor;
        if (cmd.kind == 4) {
            // 00E17BFA = 1, 00887E50 with no argument, 00E17BFA = 0.
            ++dialog_callbacks_;
            log_.notef("  dialog callback %s t=%.2f", cmd.text.c_str(),
                static_cast<double>(mission_clock_));
            mission_lua_call_named_00887e50(cmd.text);
        }
    }
    // Then the command at the cursor: a message starts its row (005B94D0 with
    // the entry's +Ch as the delay), a pause sets +88h to time - entry +Ch.
    auto found = dialog_entries_.find(name);
    if (found == dialog_entries_.end()) return;
    DialogEntry& entry = found->second;
    if (entry.cursor >= entry.commands.size()) return;
    const DialogCommand cmd = entry.commands[entry.cursor];
    if (cmd.kind == 0) {
        ++entry.cursor;
        ++dialog_messages_;
        const std::pair<float, float> timing = dialog_message_timing(cmd.text);
        dialog_row_selected_ = true;
        dialog_voice_left_ = timing.first;
        dialog_hold_8c_ = timing.second;
        dialog_delay_88_ = entry.default_pause;
        log_.notef("  dialog message %s starts t=%.2f clip=%.3f", cmd.text.c_str(),
            static_cast<double>(mission_clock_), static_cast<double>(timing.first));
    } else if (cmd.kind == 3) {
        ++entry.cursor;
        ++dialog_pauses_;
        dialog_delay_88_ = cmd.value - entry.default_pause;
    }
}

void GameScriptOrdersHost::dialog_advance_004527f0() {
    // State 1: an exhausted current entry is erased (00451020) and leaves the
    // active set. LABELLED: state 2's row reset on a name change is GUI-only and
    // folded into this call.
    if (!dialog_current_.empty()) {
        auto it = dialog_entries_.find(dialog_current_);
        if (it == dialog_entries_.end()) {
            dialog_current_.clear();
        } else if (it->second.cursor >= it->second.commands.size()) {
            log_.notef("  dialog \"%s\" finished t=%.2f", dialog_current_.c_str(),
                static_cast<double>(mission_clock_));
            active_dialogs_.erase(dialog_current_);
            dialog_entries_.erase(it);
            dialog_current_.clear();
            ++dialog_finished_;
        }
    }
    // 0044C390: keep the current one, else the larger priority, then the
    // smaller request time.
    if (dialog_current_.empty()) {
        const DialogEntry* best = nullptr;
        for (const auto& e : dialog_entries_) {
            if (best == nullptr || e.second.priority > best->priority ||
                (e.second.priority == best->priority &&
                 e.second.request_time < best->request_time)) {
                best = &e.second;
                dialog_current_ = e.first;
            }
        }
        if (best == nullptr) return;
    }
    dialog_step_00452740(dialog_current_);
}

void GameScriptOrdersHost::dialog_tick(float step) {
    // 005BBF10 steps 3 and 7. LABELLED: the image runs it in the frame update
    // with the frame delta; this process runs it once per script think step.
    if (dialog_row_selected_) {
        dialog_voice_left_ -= step;
        dialog_hold_8c_ -= step;
        if (dialog_voice_left_ > 0.0f || dialog_hold_8c_ > 0.0f) return;
        dialog_row_selected_ = false;   // selection FFFFFFFF
    }
    if (dialog_entries_.empty() && dialog_current_.empty()) return;
    dialog_delay_88_ -= step;
    if (dialog_delay_88_ > 0.0f) return;
    dialog_delay_88_ = 0.0f;
    dialog_advance_004527f0();
}

}  // namespace bsp::game
