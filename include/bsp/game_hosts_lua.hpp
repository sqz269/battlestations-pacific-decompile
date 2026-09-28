#pragma once
// bsp_game.exe milestone 2f: the mission Lua machine as a process binding.
//
// Addresses: 00884be0 (BSP_MissionLuaHost_Initialize, called at 004dd627 from
// BSP_Game_OnInitOnce), 006b8740 / 006b8610 / 006b89f0 / 006b8ad0 (the machine
// construction, the binding registration and the two chunk runners), 00b6a303
// with callback 00b69e00 (the DoFile global the binding table does not carry),
// 00886900 with 00886370 (the global script folders, called at 004dc72f from
// BSP_Game_ConstructGlobalSubsystems), 005e2f00 (the LobbySettings table,
// called at 004e02d0 from BSP_Game_LoadMissionScene), 00885110 / 00885fb0 /
// 008860b0 (the script file, its content variants and the mission chunk),
// 00887750 / 00887b30 / 00887e50 with 0045f440 and 0045f520 (the named-call
// rule and the two entry-point wrappers) and 00b66200 (the defined check).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::MissionLuaHostServices, satisfied either by the repository's
// stock Lua 5.1.1 over the mounted virtual file system or by the explicit
// unimplemented policy in GameHostLog. The 560 bindings of the table at
// 00e0b7b8 are installed as real Lua globals whose bodies are host records: a
// script that calls one gets a logged call and a nil result, never invented
// game behaviour.
//
// Evidence: docs/MISSION_LUA_MACHINE.md, docs/MISSION_LUA_HOST.md,
// docs/GAME_EXECUTABLE.md.

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>

#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/air_operations.hpp"
#include "bsp/game_hosts_fixed_step.hpp"
#include "bsp/lua_spawn_new.hpp"
#include "bsp/mission_load_hosts.hpp"
#include "bsp/mission_lua_host.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"
#include "bsp/game_tuning_singleton.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/vehicle_class_lua_load.hpp"

struct lua_State;

namespace bsp {
class VfsLocaleRuntime;
struct ShipAiPathSearchTurnRamp;
struct ShipCameraSettings;
struct ShipClassCameraInputs;
}

namespace bsp::game {

// Packet cc9_wing_construction_lua (docs/WING_CONSTRUCTION_LUA.md, section 15.4 of
// docs/SENTITY_INIT_ATTACH_ORDER.md), committed OFF. Effective only together
// with the units host's kWingConstructionInPassABound (docs/CONSTRUCT_WORLD.md
// section 30): pass A marks the nodes appended during the squadron's 007F4580
// hook as its wing (wing_member, squadron_id = the leader, class_index = the
// squadron's), and the pass A wing append, the squadron's recorded wing range
// and the dedup's wing deferral are retired.
// ON since the joint flip (docs/CONSTRUCT_WORLD.md 30.7), by the lead's ruling
// on 30.6's pairs.
inline constexpr bool kWingConstructionLuaBound = true;
// Packet cc9_load_time_squadron_hooks (docs/SENTITY_INIT_ATTACH_ORDER.md 19).
// The scene read's InitAll (0046EB4B..0046ED0F, CL=0) is the same 00925F20
// walk, so a squadron the scene read constructs runs its vtable 00D087C0
// slots +9Ch (007F4580, pass A) and +A4h (007F4BA0, pass C). When set, the
// load walk marks a scene squadron's node a squadron, so passes A and C call
// the units host's two hooks for it as for a route squadron.
// ON since the pairs (docs/SENTITY_INIT_ATTACH_ORDER.md 19.5).
inline constexpr bool kLoadTimeSquadronHooksBound = true;
// Packet cc9_load_wing_squadron_id (docs/SENTITY_INIT_ATTACH_ORDER.md 20).
// 007F4580 stores the squadron in every plane it constructs (007F4B49 MOV
// [EBX+9D4h],ESI), and the plane's pass C 007C9770 reads it at 007C97E8 to
// write thisTable[plane].SquadronID. A scene squadron's wing is built by
// create_units, so its planes reach the load walk as plain nodes. When set,
// the load walk marks members 1.. of each scene squadron record as that
// squadron's wing, as the mission-time pass A marks a hook-built wing.
// ON since the pairs (docs/SENTITY_INIT_ATTACH_ORDER.md 20.4).
inline constexpr bool kLoadWingSquadronIdBound = true;

// Packet cc9_generated_entity_party (docs/SENTITY_INIT_ATTACH_ORDER.md 23),
// committed OFF with predictions. A GenerateObject'd entity (00944FD0 ->
// 0046D930/0046DC10) is built from its held-back scene record, whose kind-1
// bag carries `Party` and `Race` (every group derives from `Common`,
// section 17.1); pass A's attach 00928A00 reads them through 00927050
// (00928A1E) into +54h/+58h, and the Lua table gets them through the 00928F50
// mirror. While true, the GenerateObject route hands the record's party and
// race to its pending node, and pass A writes thisTable[key].Party/.Race as it
// does for a load-time node. While false only load-time nodes get them.
inline constexpr bool kGeneratedEntityPartyBound = true;  // ON: identity pairs (section 23.4)

// Packet cc9_objectives_completed (docs/MISSION_OBJECTIVES.md section 8),
// committed OFF with predictions. 008BD340 Objectives_Completed and 008BD900
// Objectives_Failed decode (party, slot, name, text, quiet) as Objectives_Add
// does its first three, and for each selected slot run 008E20D0 / 008E2200 on
// the set: the matching objective's +1Ch becomes 1 / 2 after 008DFE50 drops its
// units. Nothing native ends the mission or scores from +1Ch; the scripts keep
// their own Mission.Objectives state. While true the two rows update
// bsp::game::game_objective_sets(); while false they stay records.
// Packet cc9_generated_wing_party (docs/SENTITY_INIT_ATTACH_ORDER.md 23.5),
// committed OFF with predictions. 007F4580 hands every wing plane it builds
// the squadron's spawn descriptor: 007F48F4 [squadron+C0h] -> 00922DE0 ->
// 007F491A MOV [plane+C0h],EAX. The plane's own pass A (00928A00 -> 00927050,
// 00928A1E) then reads the squadron's bag, so each wing plane carries the
// squadron's Party and Race. While true, a generated squadron's wing nodes take
// the squadron node's party and race and pass A writes them as it does for the
// squadron; while false they carry none.
inline constexpr bool kGeneratedWingPartyBound = true;  // ON: identity pairs (section 23.6)

inline constexpr bool kObjectiveStatusBound = true;  // ON: identity pairs (docs/MISSION_OBJECTIVES.md 8)

// Packet cc9_get_property_class_readers (docs/MISSION_LUA_GETPROPERTY.md 9.6).
// 0088BF80 calls the entity's reader at vtable+138h, and the readers chain by
// class: 00927AD0 answers `unitcommand` for every entity, 00779BB0 adds
// `reconlevel` for every unit. True: GetProperty answers those two keys for a
// units-host slot. `unitcommand` pushes the director's current command name
// (0071BE40, then the command's vtable+4) or `nocommand`; `reconlevel` pushes a
// new table with number keys 0..2 from the recon pass. The class keys the
// host cannot source (ammoType, state, TargetIsHome, TorpedoStock, owner, the
// LandConvoy keys) stay unserved. False: both keys return no value, as before.
inline constexpr bool kGetPropertyClassReadersBound = true;  // ON: lead ruling (docs/MISSION_LUA_GETPROPERTY.md 9.13)

// Packet cc9_lua_kill (docs/LUA_BINDING_MISSION.md, "Kill, 008AC5C0"). The Lua
// native Kill(entity [, hard]) resolves argument 0 (00888AA0) and takes cause 1,
// or 2 when a second argument reads true (008AC6DF..008AC71B). A squadron (18h,
// 008AC729) kills its members through 007ED380, a LandConvoy (1Ah, 008AC740) its
// vector through 00742210, any other entity itself through 00926D90 (008AC756).
// True: the host kills the units-host slot (a squadron's live members) through the
// gunnery host's death funnel. False: the native stays an unimplemented record.
inline constexpr bool kLuaKillBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md, Kill verdict)

// Packet cc9_lua_listeners (docs/LUA_BINDING_MISSION.md, the listener sections).
// AddListener 008C6760 keys a subscription by (channel, id) through 00980C10;
// RemoveListener 008C6990 erases it; IsListenerActive 008C6BB0 looks it up. The
// `kill` channel's subscription (vtable 00D1B68C) loads callback, entity,
// lastAttacker and lastAttackerPlayerIndex (00972520) and matches a death when
// every set is empty or holds the value (0096ACE0, 00979140). True: the host keeps
// the registry and fires `kill` callbacks for units-host deaths. False: all three
// natives stay unimplemented records.
inline constexpr bool kLuaListenersBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_recon_listeners. The `recon` channel (00980E50, from 0077B0C0 on a
// recon level change) boxes (unit, old, new, party) and a subscription (vtable
// 00D1B67C, loader 00972450) matches when its entity, oldLevel, newLevel and party
// sets are each empty or hold the value (00968470); each callback is called with
// (unit, old, new, party). True (with kLuaListenersBound): the host fires `recon`
// listeners on its recon pass's level changes. False: `recon` entries never fire.
inline constexpr bool kLuaReconListenersBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_recon_level_step_check (docs/LUA_BINDING_MISSION.md, "Why LOMP06's
// seaplane listener was silent"). 008073C0 resets every record at each pass
// (00807490 -> 00805BE0), which notifies old -> 0 through 0077B0C0, and 00805AF0
// then notifies 0 -> new; a forced record keeps its effective level (+8h) and does
// not cycle. True: each pass fires both transitions for every non-forced record
// with a level, and a unit's own party reads 2 each pass. False: only net changes
// fire, and the own party steps 0 -> 1 -> 2.
inline constexpr bool kReconListenerResetCycleBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_forced_recon_level (docs/LUA_BINDING_MISSION.md, "SetForcedReconLevel,
// 008AA8F0"). True: SetForcedReconLevel(entity, level, party) forces the recon record
// of each resolved unit (a squadron's fused slot: its live members) for that party
// through bsp::set_forced_recon_level_00805cf0, and the next recon pass publishes it.
// False: the native stays an unimplemented record.
inline constexpr bool kForcedReconLevelBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_add_damage (docs/USN02_PHASES.md section 3; docs/LUA_BINDING_MISSION.md,
// "AddDamage, 0088E000"). AddDamage(entity, amount) resolves argument 0 (00888AA0), reads
// argument 1 as a number (0088E0DE) and calls entity->vtable[1ACh](amount) at 0088E15B,
// the unit's routed damage entry 0095DA00 -> 0087D730 -> 00879070. True: the host calls
// GameGunneryHost::apply_script_damage_0095da00 for the resolved slot. False: record.
inline constexpr bool kLuaAddDamageBound = true;  // ON: phase-2 pair held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_hit_listeners (docs/LUA_BINDING_MISSION.md, "Firing hit"). The `hit`
// channel (dispatcher 00988510, producer 0077CE60) evaluates subscriptions (vtable
// 00D1B740, loader 009725B0) whose keys are target, targetDevice, attacker,
// attackType, attackerPlayerIndex, damageCaused, fireCaused and leakCaused. True: the
// host drains GameGunneryHost::take_hit_events() once per frame and fires matching
// `hit` entries. False: `hit` entries never fire (the queue is still drained).
inline constexpr bool kLuaHitListenersBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_aa_enable (docs/LUA_BINDING_MISSION.md, "AAEnable, 0089C740").
// AAEnable(entity, flag): director = entity->vtable[114h](); when it exists,
// 0071E050 sends session message 5Ah sub-kind 4 with the flag, which the director
// stores at +221h (0071C246), the AA enable the gunnery stance (008624C0) and the
// ship AI (009F2E0F) read. True: the unit's scene director entry takes the flag
// (bsp::game::scene_director_enables_set). False: record.
inline constexpr bool kLuaAAEnableBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_set_ship_speed (docs/LUA_BINDING_MISSION.md, "SetShipSpeed, 00890D30").
// SetShipSpeed(entity, speed) stores max(speed, 0) at [entity+73Ch]+24h and the
// mission clock at +28h (00890E6F), the commanded-speed pair the cruise path reads
// (docs/UNIT_COMMANDED_SPEED.md). True: GameUnitsHost::store_commanded_speed_00890e6f
// for the resolved slot. False: record.
inline constexpr bool kLuaSetShipSpeedBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_unit_get_attack_target (docs/LUA_BINDING_MISSION.md, "UnitGetAttackTarget,
// 008A6DE0"). UnitGetAttackTarget(entity) takes the director (entity vtable[114h]) and asks
// its slot 48h with 2, a constant capability test (008364B0 base, 00836790 ship: true for 0
// and 2; 0084D8F0 squadron block: true for 0 and 1). True: the director's fire target
// director+238h (slot 2Ch, 008364E0). False: the current command (0071BE40) when its
// category (vtable[0Ch]) is 1 or 2, resolved through 0071EB60 and 00521EA0. A result whose
// +5Dh removed byte is clear is pushed as thisTable[id], anything else as nil.
// True: route the row to run_unit_get_attack_target_008a6de0. False: unimplemented (nil).
inline constexpr bool kLuaUnitGetAttackTargetBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_squadron_set_speed (docs/LUA_BINDING_MISSION.md, "SquadronSetSpeed,
// 0089F780"). SquadronSetSpeed(squadron, speed) calls member->vtable[3Ch](speed) for
// each of the +3CCh members at +3D0h (0089F8CA..0089F8FF); on a plane that is
// 0074E1E0 -> 007D9E80, the controller's forward-speed set. True: route the row to
// run_squadron_set_speed_0089f780. False: unimplemented.
inline constexpr bool kLuaSquadronSetSpeedBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_is_class_changed (docs/LUA_BINDING_MISSION.md, "IsClassChanged, 008CC4B0").
// IsClassChanged(id) pushes the boolean [registry+2010h+id*4] != id (008CC5CB..008CC5DA),
// the inverse class-index map 00592640 and 00506550 reset to the identity and then
// remap one pair in. True: route the row to run_is_class_changed_008cc4b0. False:
// unimplemented (nil, which every caller in this installation reads as false).
inline constexpr bool kLuaIsClassChangedBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_set_submarine_depth_level (docs/LUA_BINDING_MISSION.md,
// "SetSubmarineDepthLevel, 00893F40"). SetSubmarineDepthLevel(entity, level) reads the
// level as an integer, drops a request for 1 to 0 when periscopeState +122Ch is 2
// (broken) or the periscope node +1214h is null, then calls 008528B0. True: route the
// row to run_set_submarine_depth_level_00893f40. False: unimplemented.
inline constexpr bool kLuaSetSubmarineDepthLevelBound = true;  // ON: pairs held (docs/LUA_BINDING_MISSION.md)

// Packet cc9_set_air_base_slot_count (docs/LUA_BINDING_MISSION.md, "SetAirBaseSlotCount,
// 008963E0"). SetAirBaseSlotCount(entity, n) resizes the air-ops block's 58h slot array
// (+4Ch, count +50h) to exactly n through 006C7E20: new slots are default records, a
// shrink destroys from the tail. True: route the row to
// run_set_air_base_slot_count_008963e0. False: unimplemented.
inline constexpr bool kLuaSetAirBaseSlotCountBound = true;  // ON: pairs held, one recorded miss (docs/LUA_BINDING_MISSION.md)

// Packet cc9_hit_listener_filters (docs/LUA_BINDING_MISSION.md, "The unmodelled `hit`
// filters, bound"). 00988510 hands the channel eight parameters: target, targetDevice
// (the hit record's own entity when it is a live child other than the victim), attacker,
// attackType, attackerPlayerIndex ([src+1Ch]), and the record's floats +48h
// damageCaused, +4Ch fireCaused, +50h leakCaused. True: targetDevice, fireCaused and
// leakCaused are matched against what this process's hits carry (no device entity, no
// fire, no leak: 0.0); attackerPlayerIndex stays unmodelled. False: an entry naming any
// of the four is counted unmodelled and never fires.
inline constexpr bool kLuaHitFilterFieldsBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_hit_rate_limit (docs/LUA_BINDING_MISSION.md, "The hit-callback rate
// limit"). 00988510 keys a map at this+168h (009882F0 / 00499030) by (victim,
// attacking unit). Before the channel lookup it evaluates only when the stored time
// is at or before the clock DAT_00F876A4, then stores clock + 2.0 (00CE3958), or
// clock + 1e-4 (00CE3C68) for ordnance kinds 8..0Fh, 12h and 13h. True: the host's
// hit dispatcher applies it, the kind being GameGunneryHitEvent::ordnance_kind.
// False: every hit is evaluated.
inline constexpr bool kLuaHitRateLimitBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_device_reload_enabled (docs/LUA_BINDING_MISSION.md, "SetDeviceReloadEnabled,
// 008C1350"). SetDeviceReloadEnabled(flag) stores lua_toboolean(argument 0) in the global
// byte 00E17BF2 (008C144F -> 008C1458). The mission load's lobby sync writes the same byte
// (005E2FB2 / 005E3017, bsp::LobbySettingsModeFlags::reload_payload_on) and so does
// 0076FE6C. Every image reader pairs it with the squadron's ReloadEnabled byte +369h.
// True: route the row to run_set_device_reload_enabled_008c1350, and
// lua_device_reload_enabled_00e17bf2() reports the byte to the plane-task feeds in
// src/game_hosts_units.cpp, which also take the squadron +369h as its default 1.
// False: unimplemented, and the accessor reports false as before.
inline constexpr bool kLuaDeviceReloadEnabledBound = true;  // ON: identity pairs, feeds wired (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_formation_query (docs/LUA_BINDING_MISSION.md, "IsInFormation 008996A0 and
// LeaveFormation 00899EB0"). IsInFormation(unit) pushes unit+284h != 0 (008996A0).
// LeaveFormation(unit) calls 0077C980(unit, 0), which sends session message 77h with a null
// target when unit+284h is set; 0077FE80's arm 3 delivers it to 0077BD70(unit, null), the
// leave the host models as GameUnitsHost::leave_group_on_destroy_0077bd70. True: route both
// rows to run_is_in_formation_008996a0 / run_leave_formation_00899eb0. False: unimplemented.
inline constexpr bool kLuaFormationQueryBound = true;  // ON: mechanism matched, spread miss recorded (docs/LUA_BINDING_MISSION.md)

// The process-wide 00E17BF2. It is reset from the lobby flags when a mission's settings
// are published and written by SetDeviceReloadEnabled. It answers false while
// kLuaDeviceReloadEnabledBound is false.
bool lua_device_reload_enabled_00e17bf2() noexcept;

// Packet cc9_submarine_air (docs/SUBMARINE_MODEL.md section 13).
// SetUnlimitedAirSupply(entity, flag) stores lua_toboolean(argument 1) at unit+1280h
// (00893C00). Routed together with the air model: under kSubmarineAirBound
// (bsp/game_hosts_units.hpp) the row reaches GameUnitsHost::set_unlimited_air_00893c00;
// otherwise it stays unimplemented.

class GameHostLog;
class GameVfsHost;
class GameScriptOrdersHost;
class GameUnitsHost;

// One binding the running scripts actually reached, with the row address the
// table at 00e0b7b8 carries for it.
struct GameMissionNativeCall {
    std::string name;
    std::uint32_t address{0};
    unsigned long long calls{0};
    int last_argument_count{0};
    // Milestone 2l: the created instances this binding was called on, by the
    // `ID` field of the entity table in its first argument. It is how the run
    // answers how many of the mission's ships its own script addresses, and it
    // is read off the call rather than chosen: every binding whose argument 1
    // is an entity table contributes, and no list of "order" bindings is
    // hand-picked.
    std::vector<int> entity_subjects;
};

// Milestone 2i. One row of the installed `VehicleClass` global, read out of the
// live Lua state the recovered global-script step 00886900 loaded
// `Scripts/datatables/autoload/vehicleclasses.lua` into. Only the keys the
// motion path of 00825f20 reads are taken; the full descriptor reader
// 00831840 / 00960230 is bsp/ship_class_fields.hpp's and needs a descriptor
// object this process does not build.
struct GameVehicleClassRow {
    bool found{false};
    int index{-1};
    std::string name;   // "Name"
    std::string type;   // "Type", the literal 00964790's string chain compares
    float max_speed{0.0f};                   // class+500h
    float max_accel{0.0f};                   // class+504h
    float retardation{0.0f};                 // class+508h
    float max_rot_angle{0.0f};               // class+4F8h
    float max_rot_angle_change_ratio{0.0f};  // class+4FCh
    float length{0.0f};                      // class+A0h, written by 00960230
    float width{0.0f};                       // class+A4h, Width at00960368
    // class+A8h, the `Height` key 00960230 writes beside `Length`. Milestone 2r
    // reads it because 00826866 places the keel sample point at half the hull
    // height below the pose and 00937C90's hull body needs neither, so the two
    // keys are the same reader's pair. A missing key stores 0.
    float height{0.0f};                      // class+A8h
    // class+B0h, the `Mass` key 00960230 writes at 0096043A with the default
    // 1.0f. 00937C90 puts it in the body descriptor's +04h at 009399F7 and
    // 00937CF1 compares it against 100.0 to choose the physics material.
    float mass{0.0f};
    // The plane rate and acceleration keys 007D1F70 reads into the plane class
    // descriptor. src/plane_class_fields.cpp carries the store address for each
    // one; the key spellings here are that reader's, not guesses. They are zero
    // on a ship row, which is correct - only a plane row carries them.
    //
    // CAVEAT that has to travel with the numbers: vehicleclasses.lua is the one
    // file in this installation's scripts/datatables that carries a local
    // modification date, so these are this installation's plane rates and not
    // provably retail. The tuning in planeglobals.lua is a separate question
    // (docs/PLANE_CONTROL_RATE_LAW.md).
    float roll_spd{0.0f};             // desc+1A8h, 007D2530
    float pitch_spd{0.0f};            // desc+1ACh, 007D2569
    float yaw_spd{0.0f};              // desc+1B0h, 007D25A2
    float yaw_roll_ratio{0.0f};       // desc+1B4h
    float slide_ratio{0.0f};          // desc+1B8h
    float roll_accel{0.0f};           // desc+1BCh
    float pitch_accel{0.0f};          // desc+1C0h, 007D2731
    float yaw_accel{0.0f};            // desc+1C4h
    float negative_pitch_ratio{0.0f}; // desc+1D8h
    float plane_stall_spd{0.0f};      // desc+184h, 007D2351
    float turn_roll_spd{0.0f};        // desc+1C8h, 007D25DB
    float turn_roll{0.0f};            // desc+25Ch, 007D289B - the bank normaliser
    // The three aerodynamic keys 007D1F70 also reads, in the spellings
    // src/plane_class_fields.cpp:279-292 records against their writers.
    // 007DBD3A pairs XDrag with the body lateral velocity ctl+3Ch and
    // 007DBD50 pairs YDrag with ctl+40h, so these two ARE the coupling that
    // turns a plane's velocity onto its nose; 007C6340 seeds the spawn
    // airspeed from TravelSpeed and 009F9D30 divides MaxSpd by
    // Pilot/Torpedo/ReferenceSpeed for the run profile's distance scale.
    float x_drag{0.0f};               // desc+174h XDrag, 007D2189
    float y_drag{0.0f};               // desc+170h YDrag, 007D2150
    float max_spd{0.0f};              // desc+188h MaxSpd, 007D238A
    float travel_speed{0.0f};         // desc+18Ch TravelSpeed, 007D23C3
    // Packet cc9_submarine_dive: the MSubmarine keys 00854230 reads, with its
    // NumberOr defaults (src/vehicle_class_lua_load.cpp). SwimDepth1 is read into
    // the PeriscopeDepth slot only when PeriscopeDepth left it negative.
    float sub_periscope_depth{-1.0f};   // class+810h PeriscopeDepth / SwimDepth1
    float sub_swim_depth2{-1.0f};       // class+814h SwimDepth2
    float sub_swim_depth3{-1.0f};       // class+818h SwimDepth3
    float sub_up_down_accel{0.25f};     // class+824h UpDownAccel
    float sub_up_down_stop_time{5.0f};  // class+82Ch UpDownStopTime
    float sub_up_speed{1.2f};           // class+830h UpSpeed
    float sub_down_speed{1.2f};         // class+834h DownSpeed
    float sub_air_run_out_time{120.0f};  // class+838h AirRunOutTime (packet cc9_submarine_air)
    float sub_air_reload_time{5.0f};     // class+83Ch AirReloadTime
    // The four the thrust and drag accelerations are built from. 007C4990 makes
    // the drag coefficient desc+50Ch out of two of them, Accel / MaxSpd^2, which
    // is what puts a plane's equilibrium airspeed exactly on MaxSpd.
    float accel{0.0f};                // desc+164h Accel, 007D20C6
    float glide_rate{0.0f};           // desc+208h GlideRate, 007D2B10
    float drag_pitch_ratio{0.0f};     // desc+1D4h DragPitchRatio, 007D2829
    float air_brake_drag{0.0f};       // desc+1DCh AirBrakeDrag, 007D226D
    // desc+1F0h DropAngle, the gain AND the cap of 009FB800's dive arm. Its
    // climb twin desc+1ECh has no key in any shipped row, which is why an AI
    // plane dives toward a lower commanded altitude but never climbs toward a
    // higher one through that routine (docs/PLANE_FLIGHT.md, "009FB800").
    float drop_angle{0.0f};
    // desc+194h SwimHeight, 007D2413. One of the two terms of the free-flight
    // arm's water line; the other, desc+508h, is derived at 007C4D03.
    float swim_height{0.0f};
    // desc+198h MinWaterSpd. 007CB7F0 (007CB81A) leaves a live AI aircraft of a
    // class with a non-zero value in free flight when it touches the water.
    // Packet cc9_water_surface_law.
    float min_water_spd{0.0f};
    // desc+268h TurnCircleRadius. The dive-bomb approach constructor 009C3EA0
    // multiplies it twice, at 009C3F86 into approach+B4h and at 009C3FB5 into
    // approach+B8h/+BCh. Packet cc8_dive_race.
    float turn_circle_radius{0.0f};
};

// Actual selected class+570 bits and the existing producer's provenance.
// A successful depth read initializes every field; false leaves output intact.
// Native 0083B5E0/00837DE0 and the ship-leaf tails are projected only for this
// scalar. This is not the full settings singleton or a descriptor replacement.
struct GameShipDepthInput {
    std::uint32_t class_reference_0570;
    std::uint32_t settings_block_offset; // 80h for session zero, F0h otherwise
    std::uint32_t scalar_source;         // selected kShipLeafTuningSources slot
    const char* class_key;               // static producer key, e.g. Destroyer
};

// Same reader on an explicitly borrowed live interpreter. The conversion mode
// is the existing 0109EEA4 projection required by native_lua_integer_00b66290.
// Used by the host below and focused installed-data verification. Restores the
// Lua stack and leaves output unchanged on failure; does not execute scripts.
bool read_ship_depth_input_lua(lua_State&, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipDepthInput&, std::string& error);

// Selected class+560h..56Ch and +570h, using the existing native leaf source
// table. Partial settings projection: no whole singleton or descriptor is made.
// Surface leaves replicate Lua element2; Submarine retains elements2..5.
struct GameShipNavigationInput {
    ShipLeafTuning tuning;
    std::uint32_t settings_block_offset; // 80h for session zero, F0h otherwise
    const char* class_key;
};

bool read_ship_navigation_input_lua(lua_State&, int type_id, std::int32_t session_mode,
    const bool& crt_sse2_conversion, GameShipNavigationInput&, std::string& error);

// Settings+1F4h..208h: moveMin, moveMax, shipMin, shipMax, travelMin, travelMax.
// Bare native Number conversion; no fallback or bounds adjustment. Both new
// borrowed readers restore the stack and preserve output on protected errors.
// They read the current interpreter without running scripts.
bool read_ship_layer_timing_input_lua(lua_State&, std::array<float, 6>&,
    std::string& error);

// One of the four names 004dfb70 invokes.
struct GameMissionEntryPointRun {
    std::string name;
    bool defined{false};   // 00b66200 through 0045f440 / 0045f520
    bool dispatched{false};
    int pcall_status{0};
    std::string error;     // recovered with errfunc 0; the native discards it
};

struct GameMissionLuaSummary {
    bool machine_started{false};
    bool self_table_created{false};    // thisTable, 004e0305
    std::size_t entity_returns{0};     // entity-returning bindings that pushed nil
    std::size_t libraries_opened{0};
    std::size_t bindings_registered{0};
    bool platform_chunk_ran{false};
    bool fundamentals_ran{false};
    bool dofile_installed{false};
    std::size_t global_folder_scripts{0};
    std::size_t global_folder_errors{0};
    bool lobby_settings_published{false};
    std::string mission_script_path;   // what 008860b0 built
    std::size_t mission_chunks_run{0}; // base plus content variants
    bool mission_chunk_ok{false};
    std::string mission_chunk_error;
    std::size_t dofile_calls{0};
    std::vector<std::string> dofile_paths;
    std::vector<GameMissionEntryPointRun> entry_points;
    // Milestone 2l: the names the mission script handed `CreateScript`, and
    // what running each one did. usn_2_java.lua's `luaStageInit` creates one,
    // `luaInit`, and that function is where the mission issues its own orders.
    std::vector<std::string> created_scripts;
    std::vector<GameMissionEntryPointRun> created_script_runs;
    std::size_t self_table_entities{0};   // thisTable slots 00928a00 would build
    std::size_t party_mirrors{0};         // slots 00928f50's mirror wrote `Party` on
    unsigned long long entity_resolves{0};  // 0089903c's resolved arm
    unsigned long long native_calls{0};
    // The three objective bindings. docs/MISSION_OBJECTIVES.md.
    unsigned long long objective_binding_calls{0};
    unsigned long long objective_units_touched{0};
    // 0088BF80 GetProperty. docs/MISSION_LUA_GETPROPERTY.md. `served` counts the
    // calls whose key one of the reconstructed readers answered; `unserved` the
    // keys that reach the arm the native leaves empty.
    unsigned long long get_property_calls{0};
    unsigned long long get_property_served{0};
    unsigned long long get_property_unserved{0};
    unsigned long long get_property_slots_rows{0};
    // Packet cc9_get_property_class_readers: the two class-chain keys, counted
    // whatever the switch (asked), and what the bound readers pushed.
    unsigned long long get_property_unitcommand_asked{0};
    unsigned long long get_property_unitcommand_named{0};
    unsigned long long get_property_unitcommand_nocommand{0};
    unsigned long long get_property_unitcommand_unnamed{0};
    unsigned long long get_property_reconlevel_asked{0};
    unsigned long long get_property_reconlevel_tables{0};
    // Packet cc9_lua_kill: Kill calls, units killed, calls with no units-host
    // slot, calls on an already dead unit, squadron calls.
    unsigned long long kill_calls{0};
    unsigned long long kill_units{0};
    unsigned long long kill_unresolved{0};
    unsigned long long kill_already_dead{0};
    unsigned long long kill_squadrons{0};
    // Packet cc9_lua_listeners.
    unsigned long long listener_adds{0};
    unsigned long long listener_removes{0};
    unsigned long long listener_queries{0};
    unsigned long long listener_kill_deaths{0};
    unsigned long long listener_kill_fires{0};
    unsigned long long listener_attacker_filtered{0};
    unsigned long long listener_recon_changes{0};
    unsigned long long listener_recon_fires{0};
    unsigned long long listener_hit_events{0};
    unsigned long long listener_hit_fires{0};
    unsigned long long listener_hit_unmodelled{0};
    // Packet cc9_set_invincible_native.
    unsigned long long invincible_calls{0};
    unsigned long long invincible_units{0};
    unsigned long long invincible_unresolved{0};
    unsigned long long forced_recon_calls{0};
    unsigned long long forced_recon_units{0};
    unsigned long long forced_recon_unresolved{0};
    unsigned long long add_damage_calls{0};
    unsigned long long add_damage_units{0};
    unsigned long long add_damage_unresolved{0};
    unsigned long long aa_enable_calls{0};
    unsigned long long aa_enable_disables{0};
    unsigned long long aa_enable_unresolved{0};
    unsigned long long ship_speed_calls{0};
    unsigned long long ship_speed_units{0};
    unsigned long long ship_speed_unresolved{0};
    unsigned long long attack_target_calls{0};
    unsigned long long attack_target_fire_arm{0};
    unsigned long long attack_target_command_arm{0};
    unsigned long long attack_target_pushed{0};
    unsigned long long attack_target_nil{0};
    unsigned long long attack_target_unresolved{0};
    unsigned long long squadron_speed_calls{0};
    unsigned long long squadron_speed_planes{0};
    unsigned long long squadron_speed_unresolved{0};
    unsigned long long class_changed_calls{0};
    unsigned long long class_changed_true{0};
    unsigned long long sub_depth_calls{0};
    unsigned long long sub_depth_stored{0};
    unsigned long long sub_depth_unresolved{0};
    unsigned long long slot_count_calls{0};
    unsigned long long slot_count_resized{0};
    unsigned long long slot_count_unresolved{0};
    unsigned long long device_reload_calls{0};
    unsigned long long device_reload_true{0};
    unsigned long long in_formation_calls{0};
    unsigned long long in_formation_true{0};
    unsigned long long leave_formation_calls{0};
    unsigned long long leave_formation_left{0};
    unsigned long long unlimited_air_calls{0};
    unsigned long long unlimited_air_stored{0};
    unsigned long long listener_hit_throttled{0};   // packet cc9_hit_rate_limit
    unsigned long long listener_hit_throttle_passed{0};
    // 00895D20 and 0089E3C0. docs/AIROPS_LAUNCH_GATES.md.
    unsigned long long air_ops_ready_calls{0};
    unsigned long long air_ops_ready_true{0};
    unsigned long long air_ops_launch_calls{0};
    unsigned long long air_ops_launch_started{0};
    unsigned long long air_ops_launch_queued{0};
    // 006C5050 through the factory seam. docs/AIROPS_LAUNCH_TICK.md.
    unsigned long long air_ops_squadrons_created{0};
    unsigned long long air_ops_squadron_key_pushes{0};
    // 00944FD0. docs/LUA_GENERATE_OBJECT_HOST.md.
    unsigned long long generate_object_calls{0};
    unsigned long long generate_object_created{0};
    unsigned long long generate_object_repeat{0};
    unsigned long long generate_object_unknown{0};
    // Packet cc9_generated_entity_party.
    unsigned long long generated_party_nodes{0};
    unsigned long long generated_party_writes{0};
    // 0094C480 / 00949750 / 0094C490. docs/LUA_SPAWN_NEW_HOST.md.
    unsigned long long spawn_new_calls{0};       // tables the binding accepted
    unsigned long long spawn_new_rejected{0};    // argument 1 was not a table
    unsigned long long spawn_new_queued{0};      // records linked at 00949530
    unsigned long long spawn_new_attempts{0};    // 0094C490 passes that took one
    unsigned long long spawn_new_fulfilled{0};   // records that reached +C0h = 1
    unsigned long long spawn_new_requeued{0};    // 009478B0 pushes
    unsigned long long spawn_new_units{0};       // entities appended at +CCh
    unsigned long long wing_member_tables{0};    // cc9_mission_end, 00928A00 per wing plane
    // Packet cc9_sentity_init_all: 00925F20 calls, the ones that found work,
    // the nodes the walks visited, the wing nodes squadron attaches appended,
    // and the 00926BE0 pushes.
    unsigned long long init_all_calls{0};
    unsigned long long init_all_nonempty{0};
    unsigned long long init_all_entities{0};
    unsigned long long init_all_wing_appended{0};
    // Packet cc9_wing_construction_lua: nodes pass A marked as the squadron's
    // wing because the units host appended them during its 007F4580 hook.
    unsigned long long init_all_wing_marked{0};
    // Packet cc9_init_attach_order: pass B 009292B0 binds and pass C
    // SquadronID stores that found a slot.
    unsigned long long init_all_class_bound{0};
    unsigned long long init_all_squadron_ids{0};
    // Packet cc9_init_pass_e_property_bag: pass E releases taken as exact.
    unsigned long long init_all_holders_released{0};
    // Packet cc9_init_identity_gaps: load-time 009292B0 binds and 00928100 mirrors.
    unsigned long long load_class_bound{0};
    unsigned long long load_identity_mirrored{0};
    // Packet cc9_pending_list_dedup: pushes skipped (pending / attached),
    // squadron upgrades, wing nodes deferred to pass A, wing appends skipped,
    // and pending nodes the load attach dropped.
    unsigned long long dedup_skipped_pending{0};
    unsigned long long dedup_skipped_attached{0};
    unsigned long long dedup_squadron_upgrades{0};
    unsigned long long dedup_wing_deferred{0};
    unsigned long long dedup_wing_append_skipped{0};
    unsigned long long dedup_load_dropped{0};
    // Packet cc9_load_time_init_all: load-time pushes, and 00928100 mirrors
    // the walk's pass C made.
    unsigned long long load_init_all_pushes{0};
    // Packet cc9_load_time_squadron_hooks: load nodes marked squadrons.
    unsigned long long load_squadron_nodes{0};
    // Packet cc9_load_wing_squadron_id: load wing planes marked.
    unsigned long long load_wing_marked{0};
    // Packet cc9_pending_list_dedup step 3: route and load-walk calls that found
    // create_units's node (retired plain pushes, squadron annotations), and the
    // ones that found none and pushed.
    unsigned long long route_pushes_retired{0};
    unsigned long long route_squadron_annotations{0};
    unsigned long long route_fallback_pushes{0};
    unsigned long long init_all_identity_mirrored{0};
    unsigned long long init_all_pushes{0};
    unsigned long long spawn_new_callbacks{0};   // named globals actually called
    unsigned long long spawn_new_callback_missing{0};
    std::vector<GameMissionNativeCall> natives; // distinct, in first-call order
    std::string first_error;
    std::string first_error_phase;
};

// The mission Lua machine for one run. It is built the way 00884be0 builds it
// and it outlives the load, because 004dd627 constructs it once per process.
// Packet cc8_airops_launch_tick: this host is also what 006C5050's creator seam
// resolves to, because the squadron has to reach the mission script's own table
// and that table is this host's.
class GameMissionLuaHost final : public bsp::MissionLuaHostServices,
                                 public bsp::AirOpsSquadronFactory,
                                 public bsp::SpawnQueueDrain,
                                 public GameEntityInitAllRunner {
public:
    GameMissionLuaHost(GameHostLog& log, GameVfsHost& vfs);
    ~GameMissionLuaHost() override;
    GameMissionLuaHost(const GameMissionLuaHost&) = delete;
    GameMissionLuaHost& operator=(const GameMissionLuaHost&) = delete;

    // 00884be0 at 004dd627, then the fundamentals chunk whose bytes the host
    // owns (00884770) and the DoFile global from the owner layer at 00b6a303.
    bool start_machine_00884be0();
    // 00886900 at 004dc72f: Scripts/global/ then Scripts/datatables/autoload/.
    std::size_t run_global_script_folders_00886900();
    // 005e2f00 at 004e02d0.
    void publish_lobby_settings_005e2f00();
    // 004e0305: the load creates the `thisTable` self table and clears `recon`
    // on the same pass. docs/MISSION_LUA_SELF_TABLE.md.
    void create_self_table_004e0305();
    // 008860b0: "Scripts/missions/" + name + ".lua" through 00885fb0 with the
    // content variants enabled.
    bool run_mission_script_008860b0(const std::string& script_name);
    // 0045f520 (forced) and 0045f440 (thread safe).
    bool call_entry_point(const std::string& name, bool threadsafe);

    // Milestone 2i: `VehicleClass[index]`, from the table the autoload folder
    // of 00886900 already ran. `index` is the id the scene's
    // `Type = E ShipClasses : <symbol>` resolved to, which is the same number
    // the installed table indexes its rows by.
    GameVehicleClassRow read_vehicle_class_row(int index);

    // Milestone 2m. One integer field of `VehicleClass[index]`, optionally one
    // level down, for the two reads 0095c640 makes that the row above does not
    // carry: `LandingShip` (default 0) and `Catapult.LaunchedClass` (default
    // -1). The same plain table lookup against the live interpreter.
    int read_vehicle_class_integer(int index, const char* key, const char* nested_key,
        int fallback);

    // The float twin of the reader above, for the `VehicleClass[index]` fields
    // 00960230 BSP_VehicleClass_ReadLuaFields stores as floats rather than
    // integers. The first caller is `ReconModifier`: 0096239A pushes the key at
    // 00D1AAF4, 009623A9 fetches the field, 009623AE FLD1 supplies the 1.0f
    // default, and 009623CF FMUL ST0,ST0 squares it before 009623D9 stores it
    // at class+B8h. Squaring is the CALLER's job here, because the field table
    // in src/vehicle_class_fields.cpp records the Lua value, not the square.
    // src/vehicle_class_fields.cpp lists the other float fields at 009623A9's
    // sibling offsets; this reader serves all of them.
    float read_vehicle_class_number(int index, const char* key, float fallback);

    // `VehicleClass[index][key][nested_key]` as a number; `fallback` when any
    // level is absent or the leaf is not a number. Packet cc9_buoyancy_elements
    // reads `Hull.WaterLineRatio` (descriptor+71Ch, stored at 00832D9A) with it,
    // the float twin of read_vehicle_class_integer's nested form.
    float read_vehicle_class_nested_number(int index, const char* key,
        const char* nested_key, float fallback);

    // `Bullets[index][key]` from the live Lua state - the bullet class table that
    // Scripts/datatables/autoload/bulletclasses.lua publishes. Used for the
    // fields the flattened per-platform BSPGun table does not carry, notably
    // "FlyTime" and "WaterTravelSpeed". docs/TORPEDO_CATEGORY_ADMISSION.md.
    float read_bullet_class_number(int index, const char* key, float fallback);

    // `Bullets[index][key]` as a string; "" when absent. Used for `Type`.
    std::string read_bullet_class_string(int index, const char* key);

    // `DeviceClass[index][key]` as a string; "" when absent or not a string.
    // Used for the device row's `Mesh` model path. docs/GUN_BARREL_COUNT.md.
    std::string read_device_class_string(int index, const char* key);

    // `VehicleClass[index][key]` as a string; "" when absent. Used for the ship
    // row's `Mesh`, whose "slot" point groups 0095F500 turns into the platform
    // frames. docs/SHIP_PLATFORM_ATTACHMENT.md.
    std::string read_vehicle_class_string(int index, const char* key);

    // The whole of a mounted resource through the VFS the scripts are read
    // from (mode 2, the script read mode); false when it does not open.
    bool read_resource_file(const std::string& path, std::vector<std::uint8_t>& bytes);

    // Milestone 2j. The head of the gameplay settings loader 0083b5e0: it
    // formats `Scripts\datatables\ShipGlobals.lua` (the literal at 00d0b67c)
    // into a path at 0083b6c3, runs it through the Lua state owner's own runner
    // 00b69d40 at 0083b6e6, and takes the `ShipGlobals` global (00d0b670)
    // through 00b67980 / 00b67800 at 0083b721 / 0083b73d. The executable has one
    // Lua state, the mission machine's, so it runs the file through the
    // recovered file runner 00885110 on that state and records 00b69d40.
    // Returns true when the global is a table afterwards.
    bool load_ship_globals_0083b6e6();

    // 007E2A20 BSP_GameTuning_LoadFromPlaneGlobals, driven by the reconstruction
    // in bsp/game_tuning_singleton.hpp over the live interpreter. The native
    // runs Scripts/datatables/PlaneGlobals.lua in a PRIVATE Lua state and copies
    // 423 key paths into a 6D0h-byte singleton, whose first 312 bytes it then
    // mirrors into the globals at 00F872F0 with one REP MOVSD at 007EAAE1 - the
    // rotation factors 007DA710 reads are three of them
    // (docs/PLANE_CONTROL_RATE_LAW.md). This process has one Lua state, so the
    // script runs on the mission state the same way ShipGlobals does.
    //
    // Returns true when the `PlaneGlobals` global is a table afterwards. The
    // block is left at its defaults when it is not; the native has no such
    // fallback, because operator new hands it raw storage and it would read
    // whatever was there.
    bool load_plane_globals_007e2a20();
    // The loaded block. All zeroes until load_plane_globals_007e2a20 succeeds.
    const GameTuningBlock& plane_globals() const noexcept { return plane_globals_; }
    bool plane_globals_loaded() const noexcept { return plane_globals_loaded_; }

    // Stored settings+4 projection: loader store 0083BCD5, mutable mission
    // binding store 008D0852. False means no producer has established it and
    // leaves output unchanged. This is not a per-frame ShipGlobals lookup.
    bool read_avoid_all_ship_collision(bool& value) const noexcept;
    void set_avoid_all_ship_collision_008d0852(bool value);
    // Stored +194,+1D4,+1D8,+214,+218 snapshot from the represented load.
    bool read_avoidance_tuning(std::array<float, 5>& values) const noexcept;
    // Packet cc9_ship_neighbour_list: the ShipAvoidance block settings+190h..+1D8h,
    // nineteen floats in offset order ((offset - 190h) / 4), read with the same
    // getters and loader defaults 0083B7AD..0083BCB8 use. False until loaded.
    bool read_ship_avoidance_block(std::array<float, 19>& values) const noexcept;
    // Packet cc9_plane_death_modes. A global two-number table such as
    // planepartclasses.lua's ExplosionExplosionDelay = {0.6, 1.8}, which
    // 004A9BD0 reads into [00E18710]/[00E1870C]. False when the global is
    // absent or not a table of two numbers.
    bool read_global_number_pair(const char* name, float& first, float& second);
    // Packet cc9_bot_scheduler_writers: 0091B2E0's two reads of the Scoring
    // table (Scripts\datatables\Scoring.lua, run when `Scoring` is absent):
    // InGameScoreUpdateTimeInterval (default 1.0, 0091BC0E) and
    // ReCalcTimeInterval (default 5.0 = 00CE3850). A missing key keeps its
    // default. False when the table could not be reached.
    bool read_scoring_intervals_0091b2e0(float& update_interval, float& recalc_interval);
    // Packet cc9_screen_29h: Globals["Difficulty"]["LockRadiusMultipliers"],
    // which 0087D7B0 appends at 0087DC85..0087DCFD into the global config's
    // vector at +3Ch (first +40h), one float per difficulty index from Lua
    // index 1 while Difficulty's HPMultipliers has that index. Runs
    // globals.lua when Globals is absent. False when nothing was read.
    bool read_lock_radius_multipliers_0087dc85(std::vector<float>& out);
    // 0087D7B0's Difficulty walk: 1/HPMultipliers[i] (0087DB61, config+1Ch) and
    // 1/PlayerCheatMultipliers[i] (0087DD33, config+4Ch). False when either table
    // is missing. No caller in this file; the gunnery host reads it.
    bool read_difficulty_multipliers_0087d7b0(std::vector<float>& hp_inverse,
        std::vector<float>& cheat_inverse);
    // Packet cc9_hit_accuracy: the four WeaponHitAccuracy sub-objects at
    // settings+240h/+298h/+2F0h/+348h as 0083C795..0083C919 fills them. False
    // until ShipGlobals ran; the caller then keeps the 00836EF0 defaults.
    bool read_weapon_hit_accuracy(bsp::WeaponHitAccuracyProfile (&out)[4]) const noexcept;
    void load_weapon_hit_accuracy_0083c795();

    // 0083ce56..0083d10d of 0083b5e0, driven by the reconstruction in
    // bsp/unit_rudder_curve.hpp over the live `ShipGlobals["Navigator"]` table.
    // `found` is false when the machine or either table is missing, and the
    // curve settings are then left untouched.
    bool read_turn_multipliers_0083ce56(UnitRudderCurveSettings& out);

    // Milestone 2p. The seven AutoThrust keys of the same loader that
    // 009ec7c0 BSP_UnitBot_ComputeThrottleCeiling consumes, read off
    // `ShipGlobals["Navigator"]["AutoThrust"]` (docs/GAMEPLAY_SETTINGS.md rows
    // +6CCh, +6D0h, +6D4h, +6E0h, +6E4h, +6E8h and +6ECh, written by
    // 0083cc2c..0083ce3c). 0083cc2c itself is not projected; only its reads
    // run. False leaves the output untouched.
    bool read_auto_thrust_0083cc2c(ShipAiAutoThrustSettings& out);

    // Packet cc9_unit_instance_step11. The EngineSoundSmoothRate of the four
    // engine-sound records 0083B5E0 fills at settings+5BCh + i*24h + 8h
    // (008405D5), off `ShipGlobals["Sounds"][name]` for name = Ship, TBoat,
    // Submarine, Plane (jump table 00842954). An absent key keeps the 0.2f
    // default 00B66330 is handed (00CE54A0). False when `Sounds` or a record
    // table is missing; `out` then keeps what it held.
    bool read_engine_sound_smooth_rates_0083b5e0(float (&out)[4]);

    // Packet cc9_mission_camera. ShipGlobals["ShipCamera"], the four keys the
    // ShipCaptain camera reads from 00424C40()+450h..+45Ch: ZoomOffset,
    // LengthMult, MinCameraAngle, MaxCameraAngle. False when the table or a
    // key is missing; `out` then keeps what it held.
    bool read_ship_camera_settings_0083b5e0(ShipCameraSettings& out);
    // VehicleClass[type_id]'s camera keys as 00831840 reads them at
    // 00831E0D..00831FEC (CaptainCameraHeight, CameraDistanceFront,
    // CameraDistanceSide, CameraDistanceVertical, CameraMinHeight) and its
    // Length (class+A0h), each with its presence bit for the fallbacks
    // ship_class_camera_00831e0d applies. False when the row is missing.
    bool read_ship_class_camera_00831e0d(int type_id, ShipClassCameraInputs& out);
    // Globals["FOVs"]["Ship"], the number 0087D7B0 reads at 0087EBE2..0087EC0A
    // (globals.lua is run first when `Globals` is not yet a table). False when
    // absent.
    bool read_global_fov_ship_0087d7b0(double& degrees);
    // ShipGlobals["PipeSightParams"] pipesight_enabled (settings+44h) and
    // zoom_rate (settings+7Ch), the two keys screen 45h's FOV block reads.
    bool read_pipe_sight_params_0083b5e0(bool& enabled, float& zoom_rate);

    // Read the recovered0083D492..0083D575 fragment on this actual Lua state.
    // Parent lookup errors return false with text; caller must reject the load.
    // Successful reads include the native per-field non-number fallbacks.
    bool read_path_turn_ramp(ShipAiPathSearchTurnRamp& out, std::string& error);

    // VehicleClass[type_id].Type selects one of the eight existing ship leaves;
    // HeavyCruiser/BigLandingShip use native exact-Boolean-or-false semantics.
    // Reads ShipGlobals.AvoidZoneDepthsSingle/Multi[class_key][1] through the
    // native Lua wrappers. The bare final GetInteger preserves numeric-string,
    // missing-value and nonnumeric conversion behavior; it adds no depth default.
    // Unsupported/non-ship types and failed parent lookups return false with an
    // error and preserve output. Call before AI construction with actual session
    // mode; an unbound class scalar must not be replaced with a fabricated zero.
    bool read_ship_depth_input(int type_id, std::int32_t session_mode,
        GameShipDepthInput& out, std::string& error);

    // Full selected ShipLeafTuning, sharing the scalar reader's Type/variant
    // mapping and protected parser. docs/GAME_SHIP_LAYER_INPUT.md.
    bool read_ship_navigation_input(int type_id, std::int32_t session_mode,
        GameShipNavigationInput& out, std::string& error);
    bool read_ship_layer_timing_input(std::array<float, 6>& out, std::string& error);

    // Milestone 2k. The two reads 0087d7b0 makes into the global config object
    // 00432650 hands out: `Globals["Minimap"]["MinimapRange"]` into +6Ch and
    // `["VisibilityRange"]` into +70h (docs/HUD_MINIMAP.md). 0087d7b0 itself is
    // not reconstructed and its other seventy reads are not performed; this runs
    // `scripts/datatables/globals.lua` on the mission machine through the
    // recovered file runner 00885110 and takes those two numbers out of the
    // installed data. False leaves both outputs untouched.
    bool read_minimap_globals_0087d7b0(float& minimap_range, float& visibility_range);

    bool started() const noexcept;
    const GameMissionLuaSummary& summary() const noexcept;
    // Logs the distinct bindings the scripts reached, highest count first.
    void report_natives(std::size_t limit);

    // --- bsp::MissionLuaHostServices -------------------------------------
    void create_state() override;
    void set_panic_function(std::uint32_t function) override;
    void set_gc_pause(int what, int pause) override;
    void open_standard_library(const bsp::LuaStandardLibrary& library) override;
    void register_global_function(const bsp::MissionLuaBinding& binding) override;

    bool open_script(const std::string& path) override;
    int script_size() override;
    void read_script(char* buffer, int size) override;
    void close_script() override;
    std::vector<std::string> script_variant_names(const std::string& path) override;

    int lua_gettop() override;
    void lua_settop(int index) override;
    int luaL_loadbuffer(const char* buffer, int size, const char* chunk_name) override;
    int lua_pcall(int nargs, int nresults, int errfunc_index) override;
    std::string lua_tolstring_at_top() override;
    int collect_results(int count, int mode) override;

    void lua_getglobal(const char* name) override;
    void lua_pushstring(const char* text) override;
    void lua_gettable(int index) override;
    void lua_remove(int index) override;
    void lua_pushvalue(int index) override;
    void push_argument(const bsp::MissionLuaArgument& argument) override;
    bool global_is_defined(const char* name) override;
    int game_lifecycle_state() override;
    void adjust_call_stack_marker(int delta) override;
    void adjust_reentrancy_depth(int delta) override;
    bool on_frame_job_thread() override;
    void queue_named_call_for_main_thread(const std::string& name) override;

    // Milestone 2l: the per-entity Lua tables 00928a00 builds. One slot of
    // `thisTable` per created scene instance, keyed by the decimal of the u16
    // at entity+174h, carrying the `ID`, `Dead` and `Ptr` fields that routine
    // seeds (docs/MISSION_LUA_SELF_TABLE.md). 00928a00 itself and its caller
    // 0077e830 are records: what the executable supplies is the slot, so the
    // entity-returning bindings can take the arm at 0089903c that pushes
    // thisTable[key] instead of the nil arm. Returns how many slots it made.
    // `class_index` is the `Type = E ShipClasses : <symbol>` id the enum
    // library resolved, which is the row index of the installed `VehicleClass`
    // global. docs/MISSION_LUA_SELF_TABLE.md records that `Class` is added to
    // the slot later by a per-kind setter through 00b675d0 and not by 00928a00;
    // that the value is the `VehicleClass` row is established by the shipped
    // scripts, which read `.Class.Type` against the literal set the rows' own
    // `Type` keys carry ("Cruiser", "Destroyer", "Fighter", ...) and also read
    // `.Class.Length`, `.Class.Name`, `.Class.Height` and `.Class.Width`, all
    // top-level keys of the same rows. The setter itself stays a record.
    // Packet cc_lua_find_entity: `findable` separates the two questions the
    // milestone 2l code ran together. Having a `thisTable` slot is decided by
    // entity virtual slot 39, which BSP_SEntity_InitAll 00925F20 calls on every
    // pending entity at 0092604E with no class filter; being answerable by
    // `FindEntity` is decided by 0088B1B0, which walks 14 of the 97 world
    // buckets (docs/LUA_BINDING_ENTITY_LOOKUP.md). `MovieCamPos` and
    // `MovieCamLookat` get a slot and are not findable, so the name index and
    // the slot table are built from different sets.
    struct SceneEntity {
        std::string name;
        int id{0};
        int class_index{-1};
        bool findable{true};
        // Packet cc8_ship_drive. 00928A00 does not seed these: 00928F50
        // BSP_MissionEntity_SetPartyRaceLuaMirror does, on the object 00927B40
        // answers with, through 00B67460 with the field names `Race` (00928FD9)
        // and `Party` (00929046), and it reads them off the entity rather than
        // off its own arguments (00928FC7 and 00929034 both load from ESI). The
        // shipped `commandhelpers.lua` indexes `recon[targetUnit.Party]` at 330,
        // 494 and 518, so a slot without `Party` makes every one of those raise.
        // A negative value means the caller does not know it and the mirror does
        // not run, which is the state of every marker.
        int party{-1};
        int race{-1};
        // Packet cc9_init_identity_gaps. A marker's scene class id (+C4h, set by
        // its constructor, e.g. 0047B6C8 for Path), -1 for a unit, and the
        // authored `Party` its pass A copies to +54h (00927050's kind-1 arm),
        // -1 when the bag has none. Read only under
        // kSceneLoadThisTableIdentityBound.
        int marker_class_id{-1};
        int marker_authored_party{-1};
    };
    std::size_t attach_scene_entities_00928a00(const std::vector<SceneEntity>& entities);
    // Packet cc9_load_time_init_all: every load-time instance pushed (the
    // stand-in for each constructor's 00928760 push), then the scene read's
    // one InitAll (0046EB4B). Returns the entities the walk attached.
    std::size_t run_scene_load_init_all_0046eb4b(const std::vector<SceneEntity>& entities);
    void write_party_race_fields(int entity_id, int party, int race);
    // Packet cc9_init_identity_gaps: 00928100 on a slot that already exists.
    bool mirror_identity_00928100(int entity_id, int party, const std::string& name,
        const char* type_name);

    // Packet cc_lua_find_entity: the `recon` shell, through the already
    // reconstructed bsp::install_recon_values_00803a40. 004E0305 sets the global
    // to nil on the mission-load pass, and on that path the native's route back
    // to a table is 00806B10, whose 006B8190 / 00803750 / 008037D0 descent
    // recreates whatever is nil before 00805D90 fills the nineteen category maps
    // (docs/RECON_SLOT_LISTS.md section 4). 00806B10 is driven by the recon slot
    // lists, which this process does not build, so it stays a record: what the
    // executable supplies is the empty shell 00803A40 builds, and every category
    // map is empty because no unit was ever detected. Without it the shipped
    // `luaGetOwnUnits` (commandhelpers.lua:12343) raises on the first index of
    // `recon`, which is what ended the run's mission-complete path.
    void install_recon_tables_00803a40();

    // Packet cc_lua_find_entity: where the mission's own script ended up, read
    // off its `Mission` table at the end of the run. Nothing in the executable
    // writes that table; it is the shipped script's own state, which is why it
    // is the honest measure of how far the run carried the mission.
    void report_mission_script_state();

    // Milestone 2l: the script objects the mission's own stage init created.
    // The `CreateScript` binding body 00898750 is a record, so the trampoline
    // keeps the name it was handed and nothing else. Running each one is the
    // executable's stand-in for the script manager: 00898750 registers the
    // object and the fixed step's script rows 00888230 (00875e55) and 00929460
    // (00875e64) are what would call it, and none of the three is
    // reconstructed. Each named global is called once with one fresh table,
    // which is the `this` a script function takes. Returns how many ran.
    std::size_t run_created_scripts();

    // Milestone 2m: the host that runs the eight reconstructed binding bodies.
    // Attached once the created instances exist, because every one of the eight
    // addresses an entity. A row the host does not handle keeps milestone 2l's
    // record.
    void attach_script_orders(GameScriptOrdersHost* orders) noexcept;
    GameScriptOrdersHost* script_orders() const noexcept;

    // Packet cc8_ship_drive. 00928F50 BSP_MissionEntity_SetPartyRaceLuaMirror,
    // the other writer of a thisTable slot: `Party` at 00929046 and `Race` at
    // 00928FD9, both through 00B67460. Run once the orders host is attached,
    // because that is where this host can reach a unit's party. Answers how many
    // slots it wrote.
    std::size_t mirror_party_race_00928f50();

    // Called by the binding trampolines; public so the C callbacks can reach it.
    // `handled` says the row ran its reconstructed body rather than standing in
    // for a native one, which is what separates a concrete record from the
    // unimplemented policy.
    void note_native_call(std::size_t row, int argument_count, bool handled = false);

    // The three objective bindings' own line. docs/MISSION_OBJECTIVES.md.
    void note_objective_binding(const char* binding, const std::string& objective,
        unsigned int slot_mask, int units_touched);

    // 0088BF80 GetProperty. The native resolves argument 0 to an entity, reads
    // argument 1 as the key, and calls the entity's own reader at vtable+138h;
    // it pushes nothing of its own and returns that reader's result count.
    // docs/MISSION_LUA_GETPROPERTY.md.
    int run_get_property_0088bf80(lua_State* state, int argument_count);
    // 00927AD0 `unitcommand` and 00779BB0 `reconlevel` under
    // kGetPropertyClassReadersBound: pushed count, or -1 when the key is
    // neither or the switch is off. docs/MISSION_LUA_GETPROPERTY.md 9.6.
    int run_get_property_class_readers(lua_State* state, const char* key);
    // 008AC5C0 Kill under kLuaKillBound. docs/LUA_BINDING_MISSION.md.
    int run_kill_008ac5c0(lua_State* state, int argument_count);
    // Packet cc9_lua_listeners, under kLuaListenersBound.
    int run_add_listener_008c6760(lua_State* state, int argument_count);
    int run_remove_listener_008c6990(lua_State* state, int argument_count);
    int run_is_listener_active_008c6bb0(lua_State* state, int argument_count);
    void dispatch_kill_listeners_009813a0();
    void dispatch_recon_listeners_00980e50();
    void dispatch_hit_listeners_00988510();
    // Packet cc9_set_invincible_native. 00897A50 SetInvincible, always bound: the
    // gunnery host's setter stores unit+150h and every reader of it is gated on
    // kUnitInvincibilityFloorBound (src/game_hosts_gunnery.cpp), so this is inert
    // until that switch is on. docs/LUA_BINDING_MISSION.md.
    int run_set_invincible_00897a50(lua_State* state, int argument_count);
    // Packet cc9_forced_recon_level, under kForcedReconLevelBound.
    int run_set_forced_recon_level_008aa8f0(lua_State* state, int argument_count);
    // Packet cc9_lua_add_damage, under kLuaAddDamageBound.
    int run_add_damage_0088e000(lua_State* state, int argument_count);
    // Packet cc9_lua_aa_enable, under kLuaAAEnableBound.
    int run_aa_enable_0089c740(lua_State* state, int argument_count);
    // Packet cc9_lua_set_ship_speed, under kLuaSetShipSpeedBound.
    int run_set_ship_speed_00890d30(lua_State* state, int argument_count);
    // Packet cc9_unit_get_attack_target, under kLuaUnitGetAttackTargetBound.
    int run_unit_get_attack_target_008a6de0(lua_State* state, int argument_count);
    // Packet cc9_squadron_set_speed, under kLuaSquadronSetSpeedBound.
    int run_squadron_set_speed_0089f780(lua_State* state, int argument_count);
    // Packet cc9_is_class_changed, under kLuaIsClassChangedBound.
    int run_is_class_changed_008cc4b0(lua_State* state, int argument_count);
    // Packet cc9_set_submarine_depth_level, under kLuaSetSubmarineDepthLevelBound.
    int run_set_submarine_depth_level_00893f40(lua_State* state, int argument_count);
    // Packet cc9_set_air_base_slot_count, under kLuaSetAirBaseSlotCountBound.
    int run_set_air_base_slot_count_008963e0(lua_State* state, int argument_count);
    // Packet cc9_device_reload_enabled, under kLuaDeviceReloadEnabledBound.
    int run_set_device_reload_enabled_008c1350(lua_State* state, int argument_count);
    // Packet cc9_lua_formation_query, under kLuaFormationQueryBound.
    int run_is_in_formation_008996a0(lua_State* state, int argument_count);
    int run_leave_formation_00899eb0(lua_State* state, int argument_count);
    // Packet cc9_submarine_air, under kSubmarineAirBound.
    int run_set_unlimited_air_00893c00(lua_State* state, int argument_count);

    // 00895D20 IsReadyToSendPlanes and 0089E3C0 LaunchSquadron, the two gates
    // between the carrier deck and the mission script's launch line.
    // docs/AIROPS_LAUNCH_GATES.md.
    // 00851CB0's lookup: the globals, then `StationaryClass`, then the row by
    // the type's own text. A name that answers there is a stationary prop and
    // has no `VehicleClass` row by construction.
    // docs/SCENE_STATIONARY_UNITS.md.
    bool stationary_class_exists(const std::string& name);

    int run_is_ready_to_send_planes_00895d20(lua_State* state, int argument_count);
    int run_launch_squadron_0089e3c0(lua_State* state, int argument_count);
    // 00944FD0 GenerateObject. Instantiates one of the entities the scene pass
    // held back at 0046D3C5, by name, and pushes its `thisTable` slot the way the
    // entity-returning tail 0089903C does. docs/LUA_GENERATE_OBJECT_HOST.md.
    int run_generate_object_00944fd0(lua_State* state, int argument_count);

    // Packet cc8_spawn_new_route. 0094C480 SpawnNew is a three-instruction thunk
    // into 00949750 on the manager at *(00F89B3C): it parses ONE Lua table,
    // queues a DCh-byte request and creates nothing. Returns no results, which
    // is what the native does and what every one of the fourteen call sites in
    // this installation's usn_19_coralus.lua expects - not one of them uses the
    // return value. docs/LUA_SPAWN_NEW_HOST.md.
    int run_spawn_new_00949750(lua_State* state, int argument_count);
    // The consumer, 0094C490, reached in GGame::OnMove step 20 through the
    // `CALL 0094C490; RET 4` thunk at 0094C8F0. One request per
    // `SpawnAttemptDelay`; a request that cannot be placed goes back on the
    // queue (009478B0) instead of being dropped. The step is the mission
    // frame's, because the native's own clock is the world time at DAT_00F876A4.
    void run_spawn_queue_0094c490(float step_seconds) override;
    void report_spawn_queue();

    // --- bsp::AirOpsSquadronFactory, packet cc8_airops_launch_tick ----------
    // 006C5050's seam. The unit is made by the script-orders host, which owns the
    // units host; what this adds is the `thisTable` slot, without which the
    // squadron the deck hands back is an id no binding can resolve.
    std::uint32_t create_squadron(const bsp::AirOpsSquadronRequest& request) override;
    // One more slot in the table 00928A00 filled at load, for a unit that did not
    // exist then. The native's own 00925F20 walk reaches every entity as it is
    // created, so a mid-mission unit gets its slot the same way.
    // Packet cc9_mission_end: the `thisTable` slot of every other unit the creator
    // just made (the squadron's wing members), keyed by unit id.
    void attach_wing_member_tables(std::size_t units_before, std::uint32_t leader_entity,
        int class_index);
    // `seed_class` false: only the three fields 00928A00 itself seeds (packet
    // cc9_init_attach_order, where pass B's 009292B0 writes `Class`).
    bool attach_created_entity_00928a00(int entity_id, const std::string& name,
        int class_index, bool seed_class = true, bool findable = true);
    // Packet cc9_init_attach_order: 009292B0 BSP_Unit_BindLuaClass over the
    // entity's existing `thisTable` slot (ClassID, Name, Class), and the plane
    // pass C store 007C97E3..007C9805 (SquadronID). False when there is no slot.
    bool bind_lua_class_009292b0(int entity_id, int class_index, const std::string& name);
    bool set_plane_squadron_id_007c97e3(int plane_id, int squadron_id);

    // --- Packet cc9_sentity_init_all: the pending list 00F899D0 --------------
    // 00926BE0, the push_back the base constructor makes at 00928760 for every
    // entity. `units_before` marks a plane squadron: its pass A (007F4580)
    // constructs the wing, whose planes join the tail of the list then. This
    // process has already made the wing units when the squadron is pushed, so
    // they are appended from [units_before, count) at that point instead.
    void push_pending_entity_00926be0(int entity_id, const std::string& name,
        int class_index);
    void push_pending_squadron_00926be0(int entity_id, const std::string& name,
        int class_index, std::size_t units_before);
    // Packet cc9_pending_list_dedup step 3: the routes' calls. Under
    // kRoutePushesRetiredBound a plain one pushes nothing when the node is
    // pending, and a squadron one annotates the pending node.
    void route_push_entity(int entity_id, const std::string& name, int class_index);
    void route_push_squadron(int entity_id, const std::string& name, int class_index,
        std::size_t units_before);
    // GameEntityInitAllRunner: 00925F20 over that list (bsp/lua_binding_mission_2.hpp).
    void run_sentity_init_all_00925f20(bool flag, std::uint32_t call_site) override;
    // True once a pass A of this process gave the entity its `thisTable` slot.
    bool init_all_attached(int entity_id) const;
    void note_created_script(std::string name);
    void note_binding_subject(std::size_t row, int entity_id);
    // A failed named call is replayed once with errfunc 0 purely to recover the
    // message for the log. That replay is the executable's, not the game's, so
    // its binding calls are not counted twice.
    void set_error_replay(bool active) noexcept;
    bool error_replay() const noexcept;
    // One line per binding the scripts called on an entity table, and the count
    // of distinct created instances the mission's own script addressed.
    void report_entity_subjects();
    // The resolved arm of the entity tail, for `FindEntity` only: 00925a90's
    // own lookup is a record, so the executable walks the instances the
    // instantiate pass created and, on a hit, pushes that entity's thisTable
    // slot. Every other entity-returning row resolves its subject from game
    // state this process does not own and keeps the recovered nil arm.
    bool push_resolved_entity(lua_State* state, const char* binding_name, int argument_count);
    void note_entity_return();
    // Packet cc8_spawn_new_route, second pass. An entity-returning row that
    // answers from push_resolved_entity is decided AFTER `handled`, so without
    // this it stayed UNIMPLEMENTED in the summary while resolving every call.
    // GameHostLog's record is sticky on first insert, so note_native_call must
    // not record a status for these rows and this must record exactly one.
    void note_entity_status(const bsp::MissionLuaBinding& binding, bool resolved);
    // Milestone 2m. The globals walk 004d3167 performs: 00b67980 opens the
    // table, 00b67080 / 00b67190 iterate it and 00b66200 is
    // `lua_type(value) == LUA_TFUNCTION`. One entry per key, in the order the
    // interpreter yields them.
    std::vector<bsp::LuaGlobalEntry> lua_global_entries();
    int run_dofile(const std::string& path);
    void note_error(const std::string& message);
    void set_phase(std::string phase);

private:
    // Packet cc8_spawn_new_route, the two halves of one drain pass.
    // 0094A140 -> 00949300 -> 009483D0: build the frame, create every member,
    // set record+C0h. 0094C777: the completion walk over record+CCh.
    void fulfil_spawn_request_009483d0(bsp::SpawnNewRequest& request);
    void complete_spawn_request_0094c777(const bsp::SpawnNewRequest& request);
    // globalConfig+2DCh, read once from Globals["SpawnAttemptDelay"].
    float spawn_attempt_delay_0087f800();

    struct OpenScript {
        std::string path;
        std::vector<std::uint8_t> bytes;
        bool open{false};
    };

    // 007E2A20's 6D0h-byte block, as far as kGameTuningKeys names it. Value
    // initialised rather than left raw, which is a deliberate divergence:
    // operator new at 0042E7A2 hands the native raw storage and it writes only
    // the keys, so a key the data file omits reads whatever was there. Zero is
    // the honest stand-in and plane_globals_loaded_ says whether any of it is
    // real.
    GameTuningBlock plane_globals_{};
    bool plane_globals_loaded_{false};

    GameHostLog& log_;
    GameVfsHost& vfs_;
    // Declared before the adapter that borrows it: the adapter keeps a
    // reference to this list for its whole life.
    std::vector<std::string> content_suffixes_; // manager +48h/+4Ch, empty here
    std::unique_ptr<bsp::VfsLocaleRuntime> resources_;
    lua_State* state_{nullptr};
    OpenScript script_;
    std::string phase_;
    // Packet cc9_stage_init_chunk_errors: the last luaL_loadbuffer / lua_pcall
    // status and chunk name, so lua_tolstring_at_top reports an error only after
    // a failed load or call. The units host reads its query chunks' results
    // through the same accessor, and those numbers were logged as errors.
    int last_status_{0};
    std::string last_chunk_;
    int call_stack_marker_{0};  // game+1A18h
    int reentrancy_depth_{0};   // 00f87900
    std::map<std::string, std::size_t> native_index_;
    // Milestone 2l: the created instances by name, with the id their thisTable
    // slot is keyed by. This is the executable's stand-in for 00925a90.
    std::map<std::string, int> scene_entity_ids_;
    GameScriptOrdersHost* script_orders_{nullptr};
    // Packet cc9_run_extra_fixed_step: the fixed-step host's 00874D00.
    GameExtraFixedStepRunner* extra_fixed_step_{nullptr};
    // Packet cc9_squadron_pass_hooks_calls: the units host, writable, for its
    // two squadron entries (the orders host hands out a const view only).
    GameUnitsHost* units_hooks_{nullptr};
    // Packet cc9_lua_listeners: the (channel, id) registry of 00980C10.
    struct ListenerEntry {
        std::string channel;
        std::string id;
        std::string callback;                 // subscription+4h
        std::vector<int> entity_ids;          // the `entity` set (+0Ch)
        bool attacker_filters_set{false};     // +1Ch or +2Ch not empty
        // `recon` (00972450): oldLevel +1Ch, newLevel +2Ch, party +3Ch.
        std::vector<int> old_levels;
        std::vector<int> new_levels;
        std::vector<int> parties;
        // `hit` (009725B0): target +0Ch, attacker +2Ch, attackType +3Ch (strings),
        // damageCaused +5Ch as a [min, max] pair; the other keys only as non-empty flags.
        std::vector<int> attacker_ids;
        std::vector<std::string> attack_types;
        std::vector<float> damage_range;
        bool hit_filters_unmodelled{false};
        // Packet cc9_hit_listener_filters, under kLuaHitFilterFieldsBound.
        bool hit_device_filter{false};      // targetDevice (+1Ch), non-empty
        std::vector<float> fire_range;      // fireCaused (+68h)
        std::vector<float> leak_range;      // leakCaused (+74h)
    };
    std::vector<ListenerEntry> listeners_;
    // Packet cc9_hit_rate_limit: the map at 00988510's this+168h, (victim,
    // attacking unit) -> the clock at which the next hit on the pair is evaluated.
    std::map<std::pair<std::size_t, std::size_t>, float> hit_rate_limit_;
    std::vector<bool> listener_death_seen_;
    // Packet cc9_lua_recon_listeners: the last level per unit and party, and the
    // recon pass generation they were taken at.
    std::vector<int> recon_listener_levels_;
    unsigned long long recon_listener_generation_{0};
public:
    void attach_units_hooks(GameUnitsHost* units) noexcept { units_hooks_ = units; }
    void attach_extra_fixed_step(GameExtraFixedStepRunner* runner) noexcept {
        extra_fixed_step_ = runner;
    }
private:
    // Packet cc9_sentity_init_all. A node of 00F899D0: the entity and what pass
    // A hands 00928A00. A deque, because pass A appends while the walk holds
    // pointers to earlier nodes.
    struct PendingEntity {
        int entity_id{0};
        std::string name;
        int class_index{-1};
        bool squadron{false};
        bool wing_member{false};
        // A wing plane's squadron: the id plane+9D4h's +174h would give
        // (007F4B49 stores the squadron there in its pass A). This process
        // keys the squadron's slot by its leader unit's id.
        int squadron_id{0};
        // Packet cc9_load_time_init_all: a load-time instance, with what the
        // load attach used to take from its SceneEntity.
        bool load_scene{false};
        bool findable{true};
        int marker_class_id{-1};
        int marker_authored_party{-1};
        int party{-1};
        int race{-1};
        // Packet cc9_generated_entity_party: a GenerateObject'd node that
        // carries its record's party and race.
        bool generated_party{false};
        std::size_t units_before{0};
        // The unit count right after this squadron's creator returned: its wing
        // is [units_before, units_end). A later creation before InitAll (the
        // next SpawnNew member) must not be taken for this squadron's wing.
        std::size_t units_end{0};
    };
    friend class GameMissionLuaInitAllBinding;
    std::deque<PendingEntity> pending_entities_;  // 00F899D0, count 00F899D4
    // Packet cc9_pending_list_dedup: the node of `entity_id`, or nullptr.
    PendingEntity* find_pending(int entity_id);
    // Ids attached by the load-time attach (attach_scene_entities_00928a00).
    std::set<int> load_attached_;
    std::set<int> init_all_attached_;
    bool init_active_00f899a5_{false};
    bool error_replay_{false};
    bool avoid_all_ship_collision_{};
    bool avoid_all_ship_collision_loaded_{};
    std::array<float, 5> avoidance_tuning_{};
    bool avoidance_tuning_loaded_{};
    std::array<float, 19> ship_avoidance_block_{};
    bool ship_avoidance_block_loaded_{};
    bsp::WeaponHitAccuracyProfile weapon_hit_accuracy_[4]{};
    bool weapon_hit_accuracy_loaded_{};
    // Packet cc8_spawn_new_route. DAT_00F876A4, the world clock the drain
    // compares against manager+0Ch, accumulated from the mission frame's step
    // because this process has no world clock object of its own.
    float spawn_world_clock_{0.0f};
    float spawn_attempt_delay_{0.0f};
    bool spawn_attempt_delay_read_{false};
    unsigned spawn_requeue_logged_{0};
    GameMissionLuaSummary summary_;
};

}  // namespace bsp::game
