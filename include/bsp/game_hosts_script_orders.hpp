#pragma once
// bsp_game.exe milestone 2m: the mission script's own orders, run through the
// bindings' recovered bodies instead of being counted.
//
// Addresses: 008a30d0 (NavigatorAttackMove) and 008a2f20 (NavigatorMoveToRange,
// with 008a2bc0 NavigatorMoveToPos and 008a2d70 NavigatorDirectMoveToRange as
// the same body), 0088a810 (the Lua value to SceneCommandTarget reader) with
// 00888aa0 / 00888760 / 00b67910 / 00b65fb0, 00899d10 (JoinFormation) through
// 0077c8d0 with the availability predicate at vtable 16Ch and the type-76h
// message, 00895250 (SetSkillLevel) with vtable 128h, 008ad330 (RepairEnable)
// with vtable 5Ch and the type-9Fh message, and 008ab850 (SetRoleAvailable)
// with 00888d20, 004bca50, vtable 148h, the type-4Ch message and 004c3840.
//
// Milestone 2l ran usn_2_java's own order function and watched it address 21 of
// the 32 created instances through ten bindings that were all host records, so
// no order reached a ship. Packet cc_lua_navigator then read eight of those ten
// rows (docs/LUA_BINDING_NAVIGATOR.md, include/bsp/lua_binding_navigator.hpp,
// src/lua_binding_navigator.cpp). This file is the executable's host for that
// reconstruction: nothing in it is a reconstruction of native code, every method
// is one call site of bsp::LuaBindingNavigatorHost or
// bsp::LuaCommandTargetSource, and each is satisfied either by state this
// process owns or by the explicit unimplemented policy in GameHostLog.
//
// What this file supplies rather than recovers, each labelled at its site:
//   - the entity behind an argument. The native resolves a Lua entity table
//     through 00888aa0 to the native object; this process has no such object and
//     resolves the table's `ID` field, the one 00928a00 seeds, to the created
//     instance of that index. That is the same identity milestone 2l reports.
//   - the object id. entity+174h is the handle-table ordinal; the two tables at
//     00f89a0c / 00f89a60 are not built here, so the executable numbers its own
//     entities, exactly as GameCommandUnit already does.
//
// What stays a record, and therefore stops an order:
//   - vtable 16Ch, the command-availability predicate 008162b0 that 0077c8d0
//     asks before it does anything at all. Its body was not read by the packet
//     that reconstructed the binding, so the host answers the neutral false and
//     JoinFormation has no effect.
//   - vtable 128h (skill), vtable 148h (role) and the three session messages.
//   - 00816e30's own arms 00816f7c..00817330, which is where an issued
//     `attackmove` or `moveto` stops: docs/CRUISE_COMMAND.md reads them in
//     pseudocode and projects none of them, so the command never reaches a
//     weapon director slot. That is packet `entity_command_arms`.
//
// Evidence: docs/LUA_BINDING_NAVIGATOR.md, docs/UNIT_COMMANDED_SPEED.md,
// docs/CRUISE_COMMAND.md, docs/SCENE_COMMAND_TYPES.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "bsp/building_pads.hpp"
#include "bsp/entity_think_dispatch.hpp"
#include "bsp/lua_binding_mission.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_blackout.hpp"

struct lua_State;

namespace bsp::game {

class GameUnitsHost;
class GameHostLog;

// The plane-squadron member write-back, as a free function so that a caller
// which holds no scripts-orders host can drive it. It fills the registry's
// +3D0h array by name from the units host, which is the only thing the body
// ever needed: `bsp::plane_squadron_registry()` is process-wide and
// `count()` / `unit_row()` are the units host's own public API. Same shape as
// `game_objective_sets()` - a producer and a reader in different hosts with
// neither owning the other.
//
// It exists as a free function because the consumer that needs it earliest,
// GameAiCoordinatorHost::Impl::build_squadrons, runs from `create_units`'s
// tail, while the member's own call sites are both in the mission loop: a
// scene-row squadron's array was therefore still empty when the AI asked, and
// USN04 built 7 AI squadrons over 15 member planes where the mission has 5.
// Adding a scripts-orders pointer to the units host instead would need a
// wiring line in the Codex-owned src/game_hosts.cpp.
//
// `only_unresolved` decides what happens to a record that already has its
// array. The body WIPES with assign(NoUnit) before re-resolving, so it can
// un-fill as well as fill: a caller that runs while a record holds a correct
// answer from another route would destroy it if a lookup missed. The air-ops
// launch path fills +3D0h inline from its batch position, so:
//
//  - false, the mission loop's long-standing behaviour, re-resolves everything
//    every time. Safe there because it runs after the launch path has finished
//    and the names always match: the launch pushes `plan.members[wing].name`
//    into BOTH the unit record and member_names, from the same expression.
//  - true, which is what a call from `create_units` passes, touches only a
//    record whose array is still empty. `create_units` runs in the MIDDLE of
//    the air-ops launch, before the record at all, so a wipe there could only
//    ever hit an older squadron; skipping the filled ones removes the question
//    rather than relying on the name argument holding forever.
//
// Returns the number of slots that resolved.
std::size_t resolve_plane_squadron_members(const GameUnitsHost& units,
                                           GameHostLog* log,
                                           bool only_unresolved = false);

// Packet cc9_difficulty, docs/GAME_DIFFICULTY.md. The switch for the game
// difficulty and the per-unit skill level. True: 0058BF37/0058BF58 store the
// effective difficulty, GetDifficulty (008AE12A) reads it back, and
// SetSkillLevel (0089539A -> 007B8AE0 / 009565A0) sets the units-host slot's
// skill index, which the dive-bomb approach captures at 009F9D22. False: the
// old behaviour, difficulty 0 everywhere and every skill call recorded only.
inline constexpr bool kSkillLevelBound = true;

// game+6ACh, the effective difficulty, one process-wide word as in the image
// (`*(00E188A8)+6ACh`). The mission host's MissionStart store writes it and
// the script host's GetDifficulty reads it. Zero until the first store, the
// value the game object's constructor leaves.
std::int32_t game_effective_difficulty_6ac() noexcept;
void set_game_effective_difficulty_6ac(std::int32_t value) noexcept;

// Packet cc9_entity_dead, docs/ENTITY_DEAD_FLAG.md. True: a unit whose damage
// death the gunnery host has recorded gets `thisTable[id].Dead = true` and
// `KillReason = "harm"` before the next script think, as the destroy-list
// flush does in the image (009273A0 -> vtable[74h] 00926390 -> vtable[7Ch] ->
// 00929B60 -> 00929800). False: the old behaviour, `Dead` stays false.
inline constexpr bool kEntityDeadBound = true;

// Packet cc9_mission_end, docs/MISSION_END.md. True: the dialog registry that
// StartDialog / KillDialog / GetActDialogIDs share (the case-insensitive map at
// [game+21E4h]+1Ch), and the census row for the script's mission end. False:
// the three bindings stay host records and no row is written.
inline constexpr bool kMissionEndBound = true;
// Packet cc9_dialog_sequencer (docs/SQUADRON_LAND_TASK.md 5cs). True: StartDialog's
// table is parsed into the panel sequence entry 004507D0 builds (commands from
// 0044BE50: msg, setpanel, hidepanel, pause, callback), and the entry is played
// by 004527F0 / 00452740 / 00452360 on 005BBF10's tail timers: a message holds
// for its streamed voice clip, then the entry's defaultPause (+88); a callback
// runs through 00887E50; an exhausted entry is erased (it leaves
// GetActDialogIDs). False: StartDialog only registers the id.
// ON (2026-10-05): controls identical; BSM04 plays INTRO and ZEKES (5cs.1).
inline constexpr bool kDialogSequencerBound = true;

// Packet cc9_kamikaze_ship_blocked (docs/SHIP_AI_OPEN_ITEMS.md section 42). In
// 007EE8F0's kamikaze arm, a ship target (007EEB64) whose class 00827F70 calls
// small (007EEB74: TorpedoBoat 0Eh, or LandingShip 0Ch without BigLandingShip)
// rejects unless 00604A50([unit+3D0h]) holds: kind 17h and PilotFires
// ([plane+C24h]) clear (007EEB83..007EEB8A). True: PilotSetTarget sets
// AttackFeasibilityInputs::kamikaze_ship_blocked from that. False: it stays
// false, as before. The census counts run in both states.
// ON (2026-09-29, section 42): USN19 gameplay identical with zero reach; the
// PilotFires stand-in is replaced by GameUnitsHost::plane_pilot_fires_0c24.
inline constexpr bool kKamikazeShipBlockedBound = true;

// Packet cc9_fill_path_points (docs/LUA_BINDING_MISSION.md, "FillPathPoints").
// True: the Lua native FillPathPoints (0089A190) answers a new table whose
// entry i + 1 is {x, y, z}, point i of the path entity's point list carried
// through 007AF800 into the world (0088BA30 writes the three fields). The
// points are the scene path registry's, which applies the same 007AF800 step
// to the authored PathPoints/Point%02i/Pos at scene load. False: the native
// stays an unimplemented record and answers nothing (the script sees nil).
inline constexpr bool kFillPathPointsBound = true;

// Packet cc9_bsm01_think_natives (docs/LUA_BINDING_MISSION.md,
// "Scoring_GetPlayerShotDown and PutTo"). True: Scoring_GetPlayerShotDown
// (008BC9B0) answers slot argument 0's (default 0) count of enemy aircraft
// kills credited to that player slot's own kill tree (record+B4h, level-1 key
// ENEMY, level-3 keys 7..0Ch; docs/SCORING_BODIES.md section 6). False: the
// native stays an unimplemented record and answers nothing.
// LABELLED SUBSTITUTION: this process keeps no scoring record. The count is
// taken from the gunnery host's death rows: a sunk unit of the plane family
// (IsKindOf(0Fh)) whose killer is the controlled unit (player slot 0; no
// other unit carries a player slot in this process) and whose side is ENEMY
// to the killer's (00803510). Other slots answer 0.
inline constexpr bool kScoringPlayerShotDownBound = true;

// Packet cc9_bsm01_think_natives. True: PutTo (008A9F90) places a unit through
// its vtable slot +118h (008193A0, GameUnitsHost::place_at_world_position_008193a0)
// in the single-player arm ([00E188A8]+1FE4h == 0, 008AA164..008AA16F).
// LABELLED: the optional third argument, degrees turned to radians
// (008AA136..008AA14D) and handed to vtable slot +11Ch (008196B0, no Ghidra
// function, unread), is recorded and not applied. False: the native stays an
// unimplemented record.
inline constexpr bool kPutToBound = true;

// Packet cc9_get_hp_percentage (docs/LUA_BINDING_MISSION.md, "GetHpPercentage's
// health slot"). True: 00923BE0's two host reads answer from the gunnery host.
// The +5Dh gate is the unit's death (GameGunneryHost::unit_dead), and
// vtable[110h] is 00876260 on all nine unit vtables, [+370h] / [+36Ch] (health
// over max health) float-stored; a scene marker (no unit) takes the base
// entity's 0042BB50, FLD1. False: the gate is clear and the slot answers 0, so
// every unit reads 0% to the scripts.
inline constexpr bool kUnitHealthFractionBound = true;

// Packet cc9_submarine_depth_level (docs/SUBMARINE_MODEL.md section 11).
// True: the Lua native GetSubmarineDepthLevel (00894100) pushes the unit's
// seeded depthLevel (+1268h, GameUnitRow::submarine_depth_level), forced to
// 0 when the death flag (+5Dh, the gunnery host's death) is set and the class
// is not kamikaze. LABELLED: needAir (+1281h) and 008522C0 read false (no air
// or catapult model), the kamikaze test reads false, and a unit that is not
// a submarine answers 0. False: the native stays an unimplemented record and
// pushes nothing, which 06_crucial_cargo.lua:713 compares as nil.
// ON by the pairs at eedf5dc79: LOMP06 Narwhal answers 1 on all six calls and
// the failures move from :713 to :531 (reconlevel), gameplay identical;
// USN02 and USN04 identical (docs/SUBMARINE_MODEL.md section 11).
inline constexpr bool kSubmarineDepthLevelBound = true;

// Packet cc9_recon_level_table (docs/RECON_SENSOR_PASS_BINDING.md, "The
// reconlevel table"). True: every unit's Lua table gets `reconlevel = {}`
// (0077FAD0 at 0077FD9C..0077FDF1), and each change of a unit's published
// detection level for a party writes `reconlevel[party] = level`, 0/1/2
// (00805BA9..00805BD8 calling the +1E4h sub-object's slot 0, 0077B0C0,
// which stores through 00B67800 / 00B665D0). The Lua native ForceRecon
// (008AADF0) runs 00807A50. LABELLED: the tables are made and the changes
// written when the host next syncs (a native call or a think pass after a
// recon pass), not inside the pass; forced levels (SetForcedReconLevel) are
// not modelled; 0077B0C0's other stores (+2F8h, +2FCh, +300h) and its
// 00980E50 notify are not modelled. False: no table, ForceRecon stays an
// unimplemented record.
// ON by the pairs at 72c01329b: LOMP06's six :531 failures went to 0 and the
// once-a-second ForceRecon moved gameplay (Narwhal survives); USN02 and USN04
// identical (docs/RECON_SENSOR_PASS_BINDING.md).
inline constexpr bool kReconLevelTableBound = true;

// Packet cc9_pilot_move_to (docs/GAME_SHIP_NAVIGATION_BINDING.md, "PilotMoveTo").
// True: the Lua native PilotMoveTo (008A4150) is served by the PilotMoveToRange
// body with the descriptor's +14h range held at 0: 008A4150 issues the same
// 0077D600(entity, 00E08F68, &target, 1) as 008A4590 but reads no third
// argument, so +14h keeps 0088A8C7's 0, and it has no pose-refresh tail. False:
// the native stays an unimplemented record.
// ON by the pairs at 4df48ee59: JM08's four calls served, Movie Mavis to
// MoviePoint (marker_goals=1), the Wildcat wave to the flagship; USN01, USN02
// and USN04 identical.
inline constexpr bool kPilotMoveToBound = true;

// Packet cc9_pilot_move_on_path (docs/GAME_SHIP_NAVIGATION_BINDING.md,
// "PilotMoveOnPath"). True: the Lua native PilotMoveOnPath (008A3E70) is served
// by the NavigatorMoveOnPath body without its speed half. 008A3E70 reads the
// entity (00888AA0), the optional follow mode (argument 2, default 1) and
// start/parameter (argument 3, default 5), then the path (argument 1, 0088A810,
// resolved through the 00F89A54 / 00F89AA8 handle tables), and routes the same
// 5Bh MT_GAMEUNIT_MOVEONPATH message (path id +174h at +20h, the pair at +24h /
// +28h) through 0077C2A0; it has no argument 4 and no 00890E6F speed store.
// False: the native stays an unimplemented record.
// ON by the pairs at 4df48ee59: JM06 (1 call) and BSM01 (2 calls) served with
// gameplay identical; USN01, USN02 and USN04 identical.
inline constexpr bool kPilotMoveOnPathBound = true;

// Packet cc9_navigator_force_torpedo (docs/UNIT_WEAPON_DEVICES.md,
// "NavigatorForceTorpedo"). True: the Lua native NavigatorForceTorpedo
// (008A7200) fires the unit's torpedo guns through 00730160, all of them or
// only the first when argument 1 is true (GameGunneryHost::
// force_torpedo_fire_008a7200). False: the native stays an unimplemented record.
inline constexpr bool kNavigatorForceTorpedoBound = false;

// Packet cc9_frame_delta_jitter, docs/GAME_EXECUTABLE.md. True: the script think
// walk 00929460 runs once per 0.05f fixed step, as the image's fan-out row 8 does
// (00875E64 inside 00875BB0), while the Blackout fade still steps once per frame
// with the frame delta (004C429A). The step count mirrors 00875BB0's accumulator
// rule. With a lockstep 0.05 s frame this is one pass per frame, which is the
// old behaviour exactly. False: one think pass per frame with the frame delta.
inline constexpr bool kScriptThinkOnFixedStep = true;

// Packet cc9_script_entity_pool, docs/SHIP_AI_OPEN_ITEMS.md section 74. True: CreateScript
// (00898841, one operator_new per call) has no count bound, as in the image. False: creation
// stops at kScriptEntityCapacity (512) and CreateScript answers no entity, which ends every
// luaDelay chain of a long mission (JM08 at mission frame 2001).
inline constexpr bool kScriptEntityPoolUnboundedBound = true;

// Packet cc9_building_pad_model, docs/SHIP_AI_OPEN_ITEMS.md section 75. True: the
// first building_pads() call builds every CommandBuilding's landing-pad vector as
// 006F5CC0's third pass does at InitAll pass C (bsp/building_pads.hpp) and the end
// summary reports it. Nothing reads the pads yet (retarget modes 3/4 and the land
// step are separate switches). False: no pad vector exists.
inline constexpr bool kBuildingPadModelBound = true;

class GameHostLog;
class GameUnitsHost;
struct GameSceneEntityRecord;

// One call of one binding, as the report prints it.
struct GameScriptOrderRow {
    std::string binding;             // the Lua global's name
    std::uint32_t row_address{0};    // the binding table row 006b8610 registers
    std::size_t unit_index{0};
    std::string unit;                // the created instance the call addressed
    std::string command;             // `attackmove` / `moveto`, "" otherwise
    std::string target;              // the named entity, or "(position)"
    bool issued{false};              // 0077d600 built and routed MT_COMMAND
    bool reached_director{false};    // 00816e30's movement fall-through was taken
    std::string blocked;             // why it stopped, when it did
    int skill_level{-1};             // 00895250's argument 1
    int repair{-1};                  // 008ad330's argument 1
    std::string formation_leader;    // 00899d10's argument 1
    int role{-1};                    // 008ab850's argument 1
    int role_value{-1};              // 008ab850's argument 2
    // Packet cc8_navigator_path: 008a3600's 5Bh message, as the native built it.
    int path_follow_mode{-1};        // msg+24h, Lua argument 2, default 1
    int path_parameter{-1};          // msg+28h, Lua argument 3, default 5
    int path_object_id{-1};          // msg+20h, the path entity's uint16 +174h
};

struct GameScriptOrdersSummary {
    std::size_t calls{0};              // binding calls this host took
    std::size_t attack_moves{0};
    std::size_t move_tos{0};
    std::size_t issued{0};             // commands 0077d600 routed
    std::size_t reached_director{0};   // of those, past 00816e30's arm test
    std::size_t formations_requested{0};
    std::size_t formations_refused{0}; // vtable 16Ch answered false
    // Packet cc8_ship_follow: joins the script path actually made, once
    // 00779D50 was transcribed and 0077FE80's type-76h arm bound.
    std::size_t formations_joined{0};
    // Packet cc8_navigator_path.
    std::size_t path_orders{0};            // 008a3600's 5Bh message
    std::size_t commanded_speed_stores{0}; // 008a3901 / 008a3912
    std::size_t land_avoidance_orders{0};  // 008a3b10, 5Ah selector 9
    std::size_t torpedo_evasion_orders{0}; // 008a3cd0, 5Ah selector 7
    std::size_t avoidance_disables{0};     // either setter with false
    std::size_t avoidance_delivered{0};    // 00835640 applied (cc9_navigator_avoidance)
    // Packet cc9_navigator_ship_avoidance: 008a3970, 5Ah selector 8. Kept apart
    // from the three counters above so their summary line reads as before.
    std::size_t ship_avoidance_orders{0};
    std::size_t ship_avoidance_disables{0};
    std::size_t ship_avoidance_delivered{0};
    // Packet cc9_formation_join_loopback: 76h joins posted to / delivered by the
    // commands host's loopback drain.
    std::size_t formation_joins_posted{0};
    std::size_t formation_joins_delivered{0};
    std::size_t skills{0};
    std::size_t repairs{0};
    std::size_t roles{0};
    std::size_t units_ordered{0};      // distinct instances a command reached
};

// Packet cc_lua_binding_audit. This process's stand-in for the 0x1E4-byte script
// entity CreateScript allocates at 00898841. It carries only the fields the five
// script bindings and the think walk 00929460 read; every other byte of the native
// allocation is untouched here and nothing depends on its layout.
struct GameScriptEntity {
    std::uint32_t id{0};          // entity+174h, the u16 the self-table key formats
    std::string created_for;      // the global CreateScript called
    std::string think_name;       // +1D8h, null when empty
    bool delay_armed{false};      // +1DCh
    float delay_seconds{0.0f};    // +1E0h
    bool initialised{true};       // +5Ch, set by the construct at 0089886F
    bool blocked_5d{false};       // +5Dh
    bool blocked_5e{false};       // +5Eh, the byte DeleteScript tests at 00898BD9
    bool blocked_60{false};       // +60h
    bool dead{false};             // thisTable[key].Dead, set by 00929800 on the kill
    unsigned long long thinks{0};
    // Packet cc9_scene_race_and_script_identity: pushed by 00928630 and not yet
    // through an InitAll's pass C (00928100).
    bool identity_pending{true};
    int party{2};   // +54h: 00925CE0's 2 (00925E1D), then SetParty's 00923B80
    int race{-1};   // +58h: 00925CE0's -1 (00925E24); SetParty passes it back unchanged
};

// Packet cc_mission_blackout: what the fade at `*(00E198C4 + A4h) + C0h` did
// across a run. `callbacks` is the count of 005B9969 calls, the only route from
// the native fade back into the mission script.
struct GameBlackoutSummary {
    std::size_t arms{0};              // 005B9BA0 entries
    std::size_t callbacks{0};         // 005B9969 calls
    std::size_t updates{0};           // 005B9800 entries
    std::size_t completions{0};       // the 005B9842 arm
    std::size_t interface_requests{0};  // 004CC460 with id 20h
    std::string last_callback;
    float level{0.0f};
    float remaining{0.0f};
};

struct GameScriptTimerSummary {
    std::size_t scripts_created{0};
    std::size_t think_registrations{0};
    std::size_t waits_armed{0};
    std::size_t clears{0};
    std::size_t deletes{0};
    std::size_t passes{0};
    unsigned long long timed_fires{0};
    unsigned long long untimed_fires{0};
    unsigned long long call_failures{0};
    std::string first_error;
};

// Packet cc9_after_row9_order_queue (docs/SENTITY_INIT_ATTACH_ORDER.md 22),
// committed OFF with predictions. A local order the image routes after
// fixed-step fan-out row 9 reaches its receiver in the NEXT step's session
// pump (0076C600 at 00778542). On an idle run the one such poster is the Lua a
// Blackout callback runs (005B9800 is screen 33h's update, inside 0068C1F0).
// While true, the MT_COMMAND issues (0077D600) that callback's bindings make
// are queued and applied, in post order, by the next
// GameFixedStepHost::pump_session_00778450. GenerateObject and
// SetSelectedUnit are not session orders and stay direct. The receiver side a
// binding does after issuing (the bot task install, the squadron fan-out, the
// path pair) runs on delivery through DeferredOrder::after_apply (packet
// cc9_after_row9_continuation, section 22.8). ON by the verdict of section
// 22.9: USN01 applied 7 with 6 continuations and identical gameplay; USN04 and
// USN02 identical with no census.
inline constexpr bool kAfterRow9OrderQueueBound = true;
// The pump's loopback drain: applies the queued orders of the one live host.
// Returns the number applied.
std::size_t script_orders_drain_loopback_0076c600();

// Packet cc9_ai_squadron_settarget_intake (docs/SQUADRON_LAND_TASK.md 5ck). True:
// an AI `settarget` or `attackmove` for a squadron (00A13B60's order, 00A14A6E)
// is served as the squadron's intake 007F1940 serves it: 007F1AD6-007F1B24 run
// 007EEC50(target, 1, 1) with ECX = the squadron, and a non-null class is issued
// in place of the order (007F1B2F-, flags 1, so 0071D880 clears first); a null
// class issues nothing. Each member's bot then installs that class's task
// (0099A4C0 -> 0099A170). False: the host fans the order itself out to the
// member planes, whose 0099A170 has no `settarget` arm.
inline constexpr bool kAiSquadronSetTargetIntakeBound = true;

// Packet cc9_get_squadron_planes (docs/SQUADRON_LAND_TASK.md 5cn). True: the
// native GetSquadronPlanes 0089CC50 returns a new table whose entries 1..n are
// the squadron's member planes' entity ids as strings (0089CD75-0089CE00: the
// +3D0h array up to +3CCh, each u16 +174h through 004260B0, stored by
// 00B672F0 at index i), the `thisTable` keys. Past five entries the image
// reads +174h through a null pointer (0089CD98 JA -> XOR EAX,EAX); the host
// stops at five, labelled. False: an unimplemented record that returns nothing, so
// bsm_04_vengance_at_luzon.lua:1718 indexes nil and luaStartMission fails on
// every think.
inline constexpr bool kGetSquadronPlanesBound = true;   // ON: SQUADRON_LAND_TASK 5cn.1
// The intake above for the one live host. `members` are the squadron's member
// planes, slot 0 first; `leader` is the squadron's slot-0 plane, on which the
// chooser's self queries run. Returns the class issued, 0 when 007EEC50
// declined (nothing issued) or no host is live.
std::uint32_t script_orders_squadron_intake_007f1940(std::size_t leader,
    const std::vector<std::size_t>& members, std::size_t target_index);

// The host the reconstructed binding bodies run over. Owned for the whole run
// because the rows are per run and the units it addresses are the created scene
// instances.
class GameScriptOrdersHost final : public bsp::LuaBindingNavigatorHost,
                                   public bsp::LuaCommandTargetSource,
                                   public bsp::LuaBindingArgumentReader,
                                   public bsp::LuaBindingResultWriter,
                                   public bsp::LuaBindingMissionHost,
                                   public bsp::EntityThinkHost,
                                   public bsp::MissionBlackoutHost {
public:
    GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units);
    ~GameScriptOrdersHost();
    // Packet cc9_after_row9_order_queue: apply the deferred orders.
    std::size_t drain_deferred_orders_0076c600();
    // Packet cc9_ai_squadron_settarget_intake: see
    // script_orders_squadron_intake_007f1940.
    std::uint32_t squadron_intake_007f1940(std::size_t leader,
        const std::vector<std::size_t>& members, std::size_t target_index);

    // Packet cc8_ship_moveonpath: `GetSelectedUnit` 008AB070 reads the global
    // 00E188D8, which 004C0893 stores in BSP_Game_SetControlledUnit 004C0890.
    // This host is the only object the Lua host holds that reaches the units.
    const GameUnitsHost& units() const noexcept { return units_; }

    // The eight navigator rows src/lua_binding_navigator.cpp reconstructs plus the
    // nine rows src/lua_binding_mission.cpp reconstructs. A row this answers false
    // for keeps milestone 2l's record.
    static bool handles(const char* binding_name) noexcept;

    // Runs the named binding's recovered body over `state`'s call frame and
    // returns its Lua result count.
    int dispatch(lua_State* state, const char* binding_name, int argument_count);

    // Packet cc_lua_binding_audit: one call of 00929460 with this process's script
    // entities, driven once per mission frame by the caller that owns the step.
    // The machine is the one the first dispatch arrived on; before any binding has
    // been dispatched the pass is a no-op, exactly as the native walk is with an
    // empty list.
    void run_script_timers(float step);
    // Packet cc9_entity_dead. 00929800's two Lua writes for every unit whose
    // death is new since the last call; see the .cpp.
    void publish_unit_deaths_00929800();
    // Packet cc9_landing_unload_latch: the unloads the ship AI queued on the pad
    // model get `LandingStarted` (0074B274..0074B2C1) then `LandingFinished` (0074AD90).
    void publish_landing_unloads_0074ad90();
    // Packet cc9_mission_end: stamps the frame `Mission.EndMission` first reads
    // true, with the fail/complete status and the objectives at that moment.
    void observe_mission_end();
    const GameScriptTimerSummary& timers() const noexcept { return timers_; }
    const GameBlackoutSummary& blackout() const noexcept { return blackout_summary_; }

    // 008980E8 / 0088A330, the single callee of SetThink's reconstructed body
    // (bsp::lua_binding_set_think). Public so the small bsp::LuaBindingCoreHost
    // adapter in the .cpp can reach it without this file growing a second copy of
    // the binding. 0088A34B appends to the pending list only on the null-to-name
    // transition, which bsp::register_pending_think_entity_0088a240 carries.
    void entity_set_think_script_name_0088a330(void* entity, const std::string& name);

    // Packet cc_lua_find_entity: the small bsp::LuaBindingCoreHost adapter in the
    // .cpp reports the SetParty virtuals through this rather than growing its own
    // reference to the log.
    void record_unimplemented(const char* method, const char* address);

    // Packet cc_lua_find_entity: a scene entity that is not a unit but that
    // BSP_SEntity_InitAll 00925F20 still hands to entity virtual slot 39 at
    // 0092604E, so it carries a `thisTable` slot and answers `FindEntity`. The
    // pose is the entity's own world matrix translation at +0FCh, which for a
    // `FixedInstance` marker class is the `localframe` the scene authored and
    // which nothing in the mission moves; that is the value `GetPosition`
    // 008A7B00 reads at 008A7C3C. `id` is this process's entity number, the
    // same substitution for the u16 at +174h that the unit rows already make.
    // Packet cc9_building_pad_model: `class_id` is the marker's scene class
    // (GameSceneMarkerSeed::class_id); LandingPoint 1Dh markers are the pads
    // building_pads() adopts. -1 when the caller does not pass it.
    void register_scene_marker(int id, const std::string& name,
        const float world_position[3], int class_id = -1);

    // Packet cc9_building_pad_model. Null while kBuildingPadModelBound is off.
    // The first call fills bsp::building_pad_model() (cleared first).
    bsp::BuildingPadModel* building_pads();

    // Packet cc8_airops_launch_tick. The unit side of 006C5050: the launch start
    // 006C7490 calls it at 006C74C6 and stores what it returns in slot+28h. The
    // native fills a scene property bag and hands it to 004F0AD0, the same
    // creator a `PlaneSquadronGen` row uses, which is how the squadron becomes an
    // ordinary created unit and reaches the script's table. This host owns the
    // units host, so the creation lands here. `wing_count_out` carries the count
    // the deck's plane-count reader then reports for the new squadron.
    // Returns the entity id (unit index plus one), or 0 when nothing was made.
    std::uint32_t create_air_ops_squadron_006c5050(std::uint32_t vehicle_class,
        std::int32_t wing_count, std::int32_t equipment, const std::string& home_base,
        std::string& created_name, std::int32_t& wing_count_out);

    // Packet cc8_lua_generate_object. 0046DB4B runs the descriptor's own
    // instantiate-pass creator on an authored record; this host owns the units
    // host, so the creation lands here as it does for the squadron. The record is
    // the one the scene pass held back, with its world frame already overridden
    // by any position or yaw the script passed. Returns the entity id, or 0.
    std::uint32_t create_unit_from_scene_record_0046db4b(
        const GameSceneEntityRecord& record);

    // 007F4B55's array, filled in. The scene pass queues one entity record per
    // wing and create_units turns them into units afterwards, so the squadron
    // table's +3D0h holds names until something resolves them to unit indices.
    // This does that, by name, and is idempotent: it re-runs only when the unit
    // count has moved, which is what the air-ops launch seam does when it
    // appends. Called from the order path rather than from create_units, because
    // the mission frame owns that call site.
    void resolve_plane_squadron_members();

    // The squadron's live plane count, entity+3CCh, for the tick 006C0510 and for
    // 006BD3F0. A squadron whose unit is gone reports zero.
    std::int32_t air_ops_squadron_plane_count(std::uint32_t squadron) const noexcept;
    std::size_t air_ops_squadrons_created() const noexcept { return squadrons_.size(); }
    // 006CDC70's walk, driven from run_script_timers. See the .cpp for why it is
    // not in the unit motion pass, where the executable has it.
    void run_air_ops_update_006cdc70(float step);
    unsigned long long air_ops_slot_ticks() const noexcept { return air_ops_ticks_; }
    unsigned long long air_ops_slot_refills() const noexcept { return air_ops_refills_; }
    unsigned long long air_ops_slot_releases() const noexcept { return air_ops_released_; }
    std::size_t air_ops_slots_tracking() const noexcept { return air_ops_tracking_; }

    void report();
    const GameScriptOrdersSummary& summary() const noexcept { return summary_; }
    const std::vector<GameScriptOrderRow>& rows() const noexcept { return rows_; }

private:
    // --- bsp::LuaCommandTargetSource, the four reads inside 0088a810 --------
    bool argument_id_field_is_nil(int index) override;
    void* argument_entity(int index) override;
    bool argument_vector3(int index, float out[3]) override;
    std::uint16_t entity_object_id(void* entity) override;

    // --- bsp::LuaBindingNavigatorHost --------------------------------------
    int argument_integer(int index) override;
    bool argument_boolean(int index) override;
    void* argument_ptr_field(int index) override;
    void entity_issue_command(void* entity, std::uint32_t command_object,
        const bsp::SceneCommandTarget& target, int flags) override;
    bool entity_command_is_available(void* entity, const char* command_name,
        void* target) override;
    int entity_route_slot(void* entity) override;
    void slot_counter_increment(int slot) override;
    void session_route_formation_message(void* follower,
        std::uint16_t leader_object_id) override;
    void entity_set_skill_level(void* entity, int level) override;
    bool entity_is_kind_of(void* entity, int class_id) override;
    void entity_set_repair_enabled_field(void* entity, bool enabled) override;
    void session_route_repair_enable_message(void* entity, bool enabled) override;
    int game_session_mode() override;
    int game_effective_game_mode() override;
    void role_owner_set_role_available(void* owner, int role, int value) override;
    void session_route_role_message(void* owner, int role, int value) override;
    void game_assign_party_player_slots(int value) override;
    // Packet cc8_navigator_path: 008a3600, 008a3b10 and 008a3cd0.
    int argument_count() override;
    float argument_number(int index) override;
    float entity_class_max_speed(void* entity) override;
    void session_route_path_order_message(void* entity,
        const bsp::NavigatorPathOrder& order) override;
    void entity_store_commanded_speed(void* entity, float speed) override;
    void* entity_weapon_director(void* entity) override;
    void session_route_avoidance_message(void* director, int selector,
        bool enabled) override;
    void unit_parts_land_avoidance_disabled(void* entity) override;

    // --- bsp::LuaBindingArgumentReader, the reads the nine rows of packet
    // cc_lua_binding_audit make through 00B677E0 --------------------------------
    int count() override;
    int get_integer(int index) override;
    double get_number(int index) override;
    bool get_boolean(int index) override;
    std::string get_string(int index) override;
    bool is_string(int index) override;
    bool is_nil(int index) override;
    bool is_entity_table(int index) override;
    void* entity_at(int index) override;

    // --- bsp::LuaBindingResultWriter, the three pushes of 00B664B0/50/30 --------
    void push_number(int value) override;
    void push_boolean(bool value) override;
    void push_nil() override;

    // --- bsp::LuaBindingMissionHost, one method per native call site ------------
    void push_number_float_00b66480(float value) override;
    float random_uniform_00bd2f10(float minimum, float maximum) override;
    bool unit_health_gate_5d_00923be4(void* entity) override;
    float unit_health_vtable_110_00923bf6(void* entity) override;
    void unit_health_cache_store_00923c16(void* entity, float value) override;
    bool entity_pose_stale_008a7c24(void* entity) override;
    void entity_pose_refresh_00414db0(void* entity) override;
    bool entity_pose_translation_008a7c3c(void* entity, float out[3]) override;
    void push_vector3_table_0088ba30(const float xyz[3]) override;
    bool measure_is_imperial_0088d9bd() override;
    void push_global_path_value_00b672b0(const char* dotted_path) override;
    float game_clock_seconds_008a93fe() override;
    void* script_entity_create_00898841() override;
    void script_entity_vcall_98_0089892c(void* entity) override;
    void script_entity_call_00927610_00898932(void* entity) override;
    int lua_stack_top_00b65eb0() override;
    void entity_call_named_009290a0(void* entity, const std::string& name,
        int stack_first, int stack_last) override;
    bool push_self_table_slot_008989f6(void* entity) override;
    void entity_arm_think_delay_008982c9(void* entity, float seconds) override;
    void entity_clear_think_name_008985a6(void* entity) override;
    bool entity_flag_5e_00898bd9(void* entity) override;
    void entity_kill_00926d90(void* entity, int cause) override;

    // --- bsp::MissionBlackoutHost, one method per native call site -------------
    void blackout_icon_set_visible(bool visible) override;
    void blackout_icon_set_colour(const bsp::BlackoutFillColour& colour) override;
    void blackout_icon_get_colour(bsp::BlackoutFillColour& colour) override;
    void mission_lua_call_named_00887e50(const std::string& name) override;
    void* local_player_unit_00e188d8() override;
    bool interface_request_pending_005b66d0() override;
    void ingame_interface_store_1c_00644220(int value) override;
    void push_interface_request_004cc460(int request_id, void* payload) override;
    int game_session_kind_1fe4h() override;
    void session_broadcast_blackout_0076d310(float level, float duration) override;
    void force_show_please_wait_screen_00e19698() override;

    // The 004C40F0 step at 004C429A: one 005B9800 pass with the frame delta.
    void run_blackout_update(float step);
    // One 00929460 walk with its delay updates, the body run_script_timers repeats.
    void run_script_think_pass(float step);

    // --- bsp::EntityThinkHost, the walk 00929460 makes over those entities ------
    void run_entity_think_00929150(std::uint32_t entity) override;
    void free_think_node_0092952c(const bsp::EntityThinkNode& node) override;
    bool gc_gate_predicate_0109cefc_vtable0c() override;
    void lua_run_string_006b8ad0(const char* chunk, int mode) override;
    void splice_pending_into_live_00928380(bsp::EntityThinkList& live,
        const bsp::EntityThinkList& pending) override;
    void clear_pending_00928330(bsp::EntityThinkList& pending) override;

    // The `ID` field 00928a00 seeds, turned into a created instance. Null when
    // the argument is not one of this process's entity tables.
    void* entity_from_argument(int index);
    // 008A4C90 PilotSetTarget. Resolves and reports; see the definition for why
    // it does not yet issue.
    int run_pilot_set_target(GameScriptOrderRow& row);
    int run_pilot_move_to_range(GameScriptOrderRow& row);
    // Packet cc9_pilot_move_to: set while PilotMoveTo runs the shared body.
    bool pilot_move_to_plain_{false};
    unsigned long long pilot_move_to_calls_{0};
    unsigned long long pilot_marker_goals_{0};
    // Packet cc9_pilot_move_on_path: set while PilotMoveOnPath runs the body.
    bool pilot_path_no_speed_{false};
    unsigned long long pilot_move_on_path_calls_{0};
    int run_entity_turn_to_entity(GameScriptOrderRow& row);
    int run_unit_set_fire_stance(GameScriptOrderRow& row);
    // Packet cc9_scripted_order_natives (kScriptedOrderNativesBound).
    int run_unit_hold_fire(GameScriptOrderRow& row);
    int run_navigator_enable(GameScriptOrderRow& row);
    int run_pilot_land(GameScriptOrderRow& row);
    unsigned long long pilot_land_calls_{0};
    std::size_t pilot_land_tasks_{0};
    unsigned long long hold_fire_null_director_{0};   // 0071BED6 on a null ECX
    unsigned long long turn_ship_arm_posed_{0};       // 008A0DE9 arm
public:
    // The +3Ch allowFire / +3Dh allowMove bytes of a squadron's +348h command
    // block (0084D810), keyed by squadron name. This host builds no such block,
    // so the bytes live here; absent means the constructor's defaults
    // (0084D862-0084D8A2).
    struct SquadronPermissions { bool allow_fire{false}; bool allow_move{false}; };
    // Packet cc9_lua_kill_script_entity: whether a `Ptr` is one of this host's
    // script entities (a CreateScript record), for Kill's 00926D90 route.
    bool is_script_entity(void* handle) const noexcept { return script_entity(handle) != nullptr; }
    void kill_script_entity(void* handle, int cause) { entity_kill_00926d90(handle, cause); }
private:
    std::map<std::string, SquadronPermissions> squadron_permissions_;
    std::size_t index_of(void* entity) const noexcept;
    std::size_t avoidance_logged_{0};
    std::string name_of(void* entity) const;

    // The script entities CreateScript made. Not a native structure: it is this
    // process's stand-in for the 0x1E4-byte allocation at 00898841, carrying only
    // the fields 00929460 and the five script bindings read.
    GameScriptEntity* script_entity(void* handle) noexcept;
    const GameScriptEntity* script_entity(void* handle) const noexcept;
    bool build_script_self_table(const GameScriptEntity& entity);
    bool push_script_self_table(const GameScriptEntity& entity);
    void call_script_global(const GameScriptEntity& entity, const std::string& name,
        int stack_first, int stack_last);

    // Packet cc_lua_find_entity.
    struct SceneMarker {
        int id{0};
        std::string name;
        float position[3]{0.0f, 0.0f, 0.0f};
        int class_id{-1};  // packet cc9_building_pad_model
    };
    const SceneMarker* marker_for_id(int id) const noexcept;

    // Packet cc8_airops_launch_tick: what this process made for a slot+28h.
    struct AirOpsSquadron {
        std::uint32_t entity_id{0};
        std::size_t unit_index{0};
        std::int32_t wing_count{0};
        std::string name;
    };
    std::vector<AirOpsSquadron> squadrons_;
    // The unit count the squadron table was last resolved against.
    std::size_t squadron_resolved_units_{0};
    bool squadron_limit_logged_{false};
    unsigned long long air_ops_ticks_{0};
    unsigned long long air_ops_refills_{0};
    unsigned long long air_ops_released_{0};
    std::size_t air_ops_tracking_{0};
    std::size_t air_ops_refill_logs_{0};

    GameHostLog& log_;
    GameUnitsHost& units_;
    std::vector<bool> dead_published_;   // per unit index, 00929800 has run
    // Packet cc9_mission_end. [game+21E4h]+1Ch: the active dialogs, keyed by id,
    // compared without case (docs/PANEL_SEQUENCE.md). The value is the start time.
    struct DialogKeyLess {
        bool operator()(const std::string& a, const std::string& b) const noexcept;
    };
    std::map<std::string, float, DialogKeyLess> active_dialogs_;
    struct MissionEndRecord {
        bool seen{false};
        float at_seconds{0.0f};
        std::string status;          // Mission.MissionStatus: failed / completed / unset
        std::string fail_text;       // Mission.MissionEndParams.Text
        std::string fail_entity;     // Mission.MissionEndParams.Ent
        std::vector<std::string> objectives;  // level:num=Active/Success
    } mission_end_;
    unsigned long long dialog_starts_{0};
    unsigned long long dialog_kills_{0};
    unsigned long long dialog_queries_{0};
    // Packet cc9_dialog_sequencer.
    struct DialogCommand {
        int kind{0};          // 0 msg, 1 setpanel, 2 hidepanel, 3 pause, 4 callback
        std::string text;     // msg: message; callback: function name
        float value{0.0f};    // pause: time (+8h)
    };
    struct DialogEntry {
        float priority{0.0f};       // +4h
        float request_time{0.0f};   // +8h, default the mission clock 00F876A4
        float default_pause{1.0f};  // +Ch, default owner+30h (DialogDefaultPauseTime)
        std::vector<DialogCommand> commands;   // +14h..+1Ch
        std::size_t cursor{0};
    };
    std::map<std::string, DialogEntry, DialogKeyLess> dialog_entries_;
    std::string dialog_current_;          // owner+28h
    bool dialog_row_selected_{false};     // voice manager +84h != -1
    float dialog_voice_left_{0.0f};       // the playing clip, polled at step 3
    float dialog_hold_8c_{0.0f};
    float dialog_delay_88_{0.0f};
    std::string message_map_name_;        // LoadMessageMap(name, index)
    int message_map_index_{0};
    std::map<std::string, std::pair<float, float>> dialog_message_cache_;
    unsigned long long dialog_messages_{0};
    unsigned long long dialog_callbacks_{0};
    unsigned long long dialog_pauses_{0};
    unsigned long long dialog_finished_{0};
    unsigned long long dialog_missing_voice_{0};
    void dialog_parse_entry(int table_slot, DialogEntry& entry);
    void dialog_tick(float step);
    void dialog_advance_004527f0();
    void dialog_step_00452740(const std::string& name);
    std::pair<float, float> dialog_message_timing(const std::string& message);
    // Packet cc9_fill_path_points.
    unsigned long long fill_path_points_calls_{0};
    unsigned long long fill_path_points_empty_{0};
    // Packet cc9_bsm01_think_natives.
    unsigned long long shot_down_calls_{0};
    int shot_down_last_{0};
    unsigned long long put_to_calls_{0};
    // Packet cc9_submarine_depth_level.
    // Packet cc9_recon_level_table.
    void sync_recon_level_tables_0077b0c0();
    std::map<std::pair<std::size_t, int>, int> recon_levels_published_;
    std::vector<bool> recon_table_made_;
    unsigned long long recon_sync_generation_{~0ull};
    unsigned long long recon_tables_created_{0};
    unsigned long long recon_level_writes_{0};
    unsigned long long recon_syncs_{0};
    unsigned long long force_recon_calls_{0};
    unsigned long long depth_level_calls_{0};
    unsigned long long depth_level_forced_{0};
    int depth_level_last_{-1};
    unsigned long long put_to_placed_{0};
    // Packet cc9_get_hp_percentage.
    unsigned long long health_reads_{0};
    unsigned long long health_reads_dead_{0};
    unsigned long long health_reads_marker_{0};
    // 008A4C90's tally. `calls` counts what the scripts asked for; the two
    // `resolved` counters say whether the Lua argument path actually reached a
    // unit and a target, which was the open question the wiring settles.
    unsigned long long pilot_set_target_calls_{0};
    unsigned long long pilot_set_target_unit_resolved_{0};
    unsigned long long pilot_set_target_target_resolved_{0};
    unsigned long long pilot_set_target_issued_{0};
    unsigned long long pilot_set_target_tasks_{0};
    unsigned long long kamikaze_small_targets_{0};   // packet cc9_kamikaze_ship_blocked
    unsigned long long kamikaze_blocked_{0};
    // 007EEC50's eleven feasibility inputs assembled for `unit_index` against
    // `target_index`, then the choice; `label` prefixes the census lines.
    std::uint32_t choose_attack_class_007eec50(std::size_t unit_index,
        std::size_t target_index, bool prefer_ordnance, bool allow_guns,
        const char* label);
    // Packet cc9_ai_squadron_settarget_intake census.
    unsigned long long squadron_intake_calls_{0};
    unsigned long long squadron_intake_declined_{0};
    unsigned long long squadron_intake_member_orders_{0};
    unsigned long long squadron_intake_tasks_{0};
    // Packet cc9_get_squadron_planes census.
    unsigned long long squadron_planes_calls_{0};
    unsigned long long squadron_planes_entries_{0};
    unsigned long long squadron_planes_unresolved_{0};
    std::vector<SceneMarker> markers_;
    lua_State* state_{nullptr};
    // The mission machine, kept past a dispatch so the per-frame timer pass can
    // call the think globals on it. The native reaches the same machine through
    // *(*(00E188A8)+1A08h)+4h; this process has no such chain and keeps the
    // pointer the trampoline handed it.
    lua_State* machine_state_{nullptr};
    int argument_count_{0};
    GameScriptOrderRow* row_{nullptr};
    GameScriptOrdersSummary summary_{};
    std::vector<GameScriptOrderRow> rows_;
    std::vector<std::size_t> ordered_units_;
    bool logged_path_{false};
    bool logged_predicate_{false};
    // Packet cc8_navigator_path. 008a3600's path argument, kept for the span of
    // one dispatch so the 5Bh arm can build the receiver's descriptor from the
    // same entity 00720fa0 would have re-resolved out of the message.
    bool logged_path_order_{false};
    bool logged_path_points_missing_{false};
    void* path_entity_for_order_{nullptr};

    // Packet cc_lua_binding_audit.
    // A deque: Lua holds the records as light userdata, and push_back keeps them in place.
    std::deque<GameScriptEntity> script_entities_;
    bsp::EntityThinkList think_live_{};
    bsp::EntityThinkList think_pending_{};
    float think_countdown_{0.0f};   // 00F89A04, zero at process start
    bool building_pads_built_{false};
    float origin_diag_next_{0.0f};  // BSP_ORIGIN_DIAG, packet cc9_script_entity_pool
    float think_step_accumulator_{0.0f};  // mirror of 00875BB0's accumulator (00F876AC)
    // A DeleteScript from inside a think function would erase from the live list
    // while the walk is iterating it. The native's cursor captured its successor
    // first; this reconstruction holds the erase until the walk returns.
    bool in_think_walk_{false};
    std::vector<std::uint32_t> think_erase_after_walk_{};
    float mission_clock_{0.0f};     // 00F876A4, advanced by the caller's step
    bool measure_imperial_{false};  // 00F88988, untouched by this process
    // The stream 00BD2ED0 hands 00BD2F10. This process has no thread-local random
    // state object, so the sequence is this generator's and not the game's; a run
    // is reproducible, which is what a headless comparison needs.
    std::uint32_t random_state_{0x13579BDFu};
    unsigned long long random_draws_{0};
    GameScriptTimerSummary timers_{};
    std::size_t script_identity_writes_{0};  // packet cc9_scene_race_and_script_identity
    std::size_t script_party_sets_{0};
    void mirror_script_identity_00928100();
    void write_script_identity_fields(const GameScriptEntity& script, bool type);

public:
    // SetParty's vtable +2Ch on a CreateScript entity (00D11164 = 00928F50):
    // 00923B80 stores +54h = party (00923B92), then 00928F50 mirrors Race and
    // Party. Answers false when the entity is not a script entity.
    bool set_script_entity_party_00928f50(void* entity, int party);

private:

    // Packet cc_mission_blackout. The five fields at `*(00E198C4 + A4h) + C0h`,
    // and the widget colour the +54h getter would answer with. 005BA7B0 leaves
    // +C0h, +C4h and +C8h unwritten; this process starts them at zero, which is
    // the state a first `Blackout` overwrites anyway.
    bsp::MissionBlackoutFade blackout_{};
    bsp::BlackoutFillColour blackout_colour_{};
    GameBlackoutSummary blackout_summary_{};
    // Packet cc9_after_row9_order_queue: the callback being run, and the queue.
    std::string after_row9_poster_;
    struct DeferredOrder {
        std::size_t index{0};
        std::uint32_t command_object{0};
        bsp::SceneCommandTarget target{};
        int flags{0};
        std::string source;
        std::string target_name;
        std::string poster;
        // Packet cc9_after_row9_continuation (docs/SENTITY_INIT_ATTACH_ORDER.md
        // 22.7): what the host does on the order's delivery, run right after the
        // drain applies it. The image's receiver side (the bot's task install in
        // 0099ACD0 behind +7Ch, the squadron fan-out 007ECF80, the path pair
        // 0071C1B0 at 00721ADB) follows the delivery, so a deferred order carries
        // it with it.
        std::function<void()> after_apply;
    };
    std::vector<DeferredOrder> deferred_orders_;
    std::vector<std::pair<std::string, unsigned long long>> deferred_by_poster_;
    unsigned long long deferred_applied_{0};
    unsigned long long deferred_continuations_{0};
    // True when the last entity_issue_command queued its order.
    bool last_issue_deferred_{false};
    // The queued order's source while its continuation runs at the drain.
    std::string delivery_source_;
    // Runs `fn` now when the last order was applied directly, or attaches it to
    // that queued order so the drain runs it on delivery.
    void after_order_delivery(std::function<void()> fn);
    void apply_issued_order(std::size_t index, std::uint32_t command_object,
        const bsp::SceneCommandTarget& target, int flags, const std::string& source,
        const std::string& target_name);
    // *(float*)(00432650() + E0h), the configured default duration. That field
    // was not read by this packet; every `Blackout` in usn_2_java and
    // commandhelpers.lua passes an explicit numeric argument 3, so the default is
    // never consumed. The host logs it if a call ever reaches it.
    float blackout_configured_duration_{0.0f};
    bool blackout_configured_duration_used_{false};
};

}  // namespace bsp::game
