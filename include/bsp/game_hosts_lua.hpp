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
#include <list>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>

#include "bsp/ai_target_weights.hpp"
#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/ship_catapult_host.hpp"
#include "bsp/world_map_bounds.hpp"
#include "bsp/air_operations.hpp"
#include "bsp/shipyard_production.hpp"
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
// Packet cc9_get_property_ammotype (docs/SQUADRON_LAND_TASK.md 5ci). True: the
// squadron's own GetProperty 007EF1C0 (PlaneSquadronGen vtable 00D087C0 +138h)
// answers "ammoType" (_stricmp 00BF7FBF at 007EF1E1, so "ammotype" too) with
// 007EDAD0's integer through GameUnitsHost::squadron_ammo_type_007edad0. Only a
// squadron serves the key. False: no value, so a script's `~= 0` test passes.
inline constexpr bool kGetPropertySquadronAmmoTypeBound = true;  // ON 2026-09-30: controls identical, USNRM01 moved (5ci)
// Packet p16_torpedo_stock_property (docs/OCEAN_WAVE_FIELD.md section 61). True:
// a ship's GetProperty answers "TorpedoStock" (_stricmp 00BF7FBF at 00815891,
// so any case) the way 00815870 does: s = unit+104Ch; s < 0 pushes s, else
// 00810E90 (the loaded torpedo tubes) + s, through 00B664B0. 00815870 sits at
// vtable+138h of the eight ship vtables and is the mother ship's 00758340's
// first call, so every IsKindOf(6) unit serves it; no other entity does.
// The value is GameGunneryHost::torpedo_stock_total_00815850. False: no value,
// so jm08.lua's `torp < 12` raises on nil and aborts its PT loop.
inline constexpr bool kGetPropertyTorpedoStockBound = true;  // ON 2026-10-17: pairs identical in gameplay (OCEAN_WAVE_FIELD 61.1)
// Same packet. True: a squadron's GetProperty answers "state" (00425850 at
// 007EF20E) as 007EF1C0 does: over the +3CCh planes at +3D0h it counts those
// whose control mode (plane+72Ch)->vtable[38h] answers true (A; 0074E210, the
// mode +1D4h == 7, which this host keeps as +900h) and the rest (B); it pushes
// 2 when B is 0 (an empty squadron included), else 1 when A > 0, else 0.
// commandhelpers.lua's luaAirstrikeManager_think attacks only on `state == 2`.
// False: no value (nil), so that test never passes.
inline constexpr bool kGetPropertySquadronStateBound = true;  // ON 2026-10-17: by mechanism, never asked (61.1)

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

// Packet cc9_recon_publication (docs/RECON_PUBLICATION.md). 00806B10, called by
// 008079B0 after each slot's rebuild when the slot's +25h byte is set, sets
// recon[party].enemy/own/neutral/unknown to nil, then refills them from triples
// 1, 0, 2 and 3 through 00805D90: nineteen category tables each, keyed by the
// decimal +174h id, valued thisTable[id] (00927BF0). True: the host publishes
// its recon triples after each recon pass. False: the maps stay the empty shell
// 00803A40 built, and every luaGetShipsAround* query answers empty.
inline constexpr bool kReconPublishBound = true;  // ON: pairs held (docs/RECON_PUBLICATION.md section 4)

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
// Packet cc9_l59_lua_items (docs/CC13_SEG56.md; docs/SQUADRON_LAND_TASK.md 5he). 008AA8F0
// forces the entity's OWN record first (00805CF0 at 008AAA76, record = entity + party *
// 34h + 1E8h), then each squadron +3D0h member (008AAAB8) or convoy element. True: the
// resolved slot is forced, then the squadron's members. False: the members only (a slot
// with no squadron record is forced itself either way).
inline constexpr bool kForcedReconOwnRecordBound = true;   // ON: SQUADRON_LAND_TASK 5he.1
// Packet cc9_l60_forced_recon_sync (cc13-confirm F; docs/SQUADRON_LAND_TASK.md 5hk).
// 00805CF0 reads the record's effective level (+8h when +10h is set, else +4h), sets
// +10h = 1 and +8h = level (00805D0A/00805D0E), unlinks the record when +28h is set
// (00805D11-00805D1F), and when the effective level changed calls the record's
// listener [+2Ch] vt[0](+30h, old, new) (00805D36-), which reaches the `recon` channel.
// True: SetForcedReconLevel fires the matching `recon` listeners at once with the old
// and new effective levels, and the pass dispatch publishes a forced record's level
// whatever the party's coverage. False: the change surfaces at the next pass, and an
// uncovered party's forced record publishes 0.
inline constexpr bool kForcedReconSyncNotifyBound = true;   // ON: SQUADRON_LAND_TASK 5hk.1

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

// Packet cc9_lua50_hit_second_pass (docs/SQUADRON_LAND_TASK.md 5fs, from
// docs/CC13_EVENT_CHANNELS.md section 5). 00988510: when no `hit` subscription
// matched (0098901A, +105h clear) and the attacker answers IsKindOf(0Fh), the
// attacker is replaced by attacker->vtable[140h]() (00989047..00989060) and the
// channel is evaluated again (009892E4). For a plane (vtable 00D05F20, slot 140h =
// 007B97E0, `MOV EAX,[ECX+9D4h]; RET`) that is its squadron. True: a hit whose
// shooter is a squadron member and that matched nothing is re-evaluated with the
// shooter's squadron. False: one pass, as before. ON (5fs): BSM04, USNRM01 and
// JM13 9000 frames identical (55 / 58 / 0 second passes, none matched).
inline constexpr bool kLuaHitSquadronSecondPassBound = true;

// Packet cc9_lua51_listener_args (docs/SQUADRON_LAND_TASK.md 5fu, from
// docs/CC13_EVENT_CHANNELS.md section 3). The image calls every matched callback
// through 00887E50 with the boxed values as Lua arguments: `kill` (009813A0,
// 009818E8..00981977) passes (victim entity, lastAttacker entity, lastAttackerPlayerIndex
// number); `hit` (00988510, 00988D87..00988F2C) passes (target, targetDevice, attacker
// entities, attackType string, attackerPlayerIndex number, damage, fire, leak numbers).
// True: the host pushes them. False: callbacks are called with no argument, as before.
// ON (5fu): ESMP08's win now ends on entity="Case" (luaUSNSunk's argument) instead of "".
inline constexpr bool kLuaListenerCallbackArgsBound = true;

// Packet cc9_lua51_listener_args. 009813A0 boxes the victim's last attacker (the
// handle copied from victim+2B0h at 009816CA; its id word is +2C8h, which 0077CC80
// fills) and +2D8h, and 0096ACE0 tests them against the subscription's lastAttacker
// (+1Ch) and lastAttackerPlayerIndex (+2Ch) sets. When nothing matched (+105h clear,
// 00981AA4) and the attacker answers IsKindOf(0Fh) (00981AC2), the attacker is
// replaced by vtable[140h]() (the plane's squadron, 007B97E0) and the channel is
// evaluated again (00981C31). True: the host matches both sets and runs that second
// pass. False: a subscription with either set non-empty is skipped (counted).
// ON (5fu): BSM01/BSM04/JM06/USN03/ESMP03/LOMP05 gameplay-identical; nothing the rows'
// attacker sets name lands a kill in them.
inline constexpr bool kLuaKillAttackerBound = true;

// Packet cc13-ver5 (docs/CC13_VER_5.md section 3). The `entityKilled` channel: every
// unit destroy runs vtable[70h] = 0077D1A0, whose tail calls 00986480 on [00F8A0C4]
// (0077D248..0077D253) with no session gate. 00986480 boxes the entity (word +174h)
// and the KillReason string 00E0CF04[entity+70h] (cause 1 = "harm" for a damage death),
// and calls every subscription whose `entity` set (+0Ch) and `killReason` set (+1Ch,
// 00968FF0) hold, through 00887E50. It skips an entity whose +3Ch scene parent exists
// and is not IsKindOf(44h), the Landscape (009864D2..009864F1). Ten scripts in this
// installation listen on it (three copies of JM01, for its Radio station and the
// Oklahoma's secondary objective; five sea-training and two air-training missions,
// whose stages end on it); the host never delivered it.
// True: AddListener keeps the two sets, and each host death fires the channel once,
// before `kill`, with (entity, KillReason) as arguments when kLuaListenerCallbackArgsBound;
// the reason is the cause the host's kill recorded, as the `Dead` publisher names it.
// SUBSTITUTIONS (labelled): the host's units carry no scene parent other than the
// Landscape, so the 44h gate always passes; the death is the host's destroyed-unit row,
// evaluated on the next mission frame as `kill` is, not inside the hit; a squadron
// slot's death waits for its last plane, as the `kill` dispatcher does.
// False: entityKilled subscriptions are stored and never fire, as before.
inline constexpr bool kLuaEntityKilledListenersBound = true;  // ON: lead pair (USN02/PRCP01/LOMP06 wins identical, 2026-10-13)

// Packet cc13-ver5 (docs/CC13_VER_5.md D2). In the image a victim's `Dead` is written
// before its `kill` channel fires: 00959450 calls 00878990 -> 0077D270 -> 00929B60 ->
// 00929800 (Dead, KillReason) at 0095945B, and 009813A0 (the kill channel) only at
// 00959507, in the same row-15 flush. The host evaluates `kill` in the world tick's spawn
// drain and publishes `Dead` afterwards in run_script_timers, so a kill callback that
// reads the victim's .Dead, or luaRemoveDeadsFromTable over a table holding it, without
// setting it itself sees it alive. 55 kill callbacks in this installation do (USN02's
// convoy hidden objective, ESMP01's luaJPCVSunk failure, USN03's Zuiho, LOMP05's
// Vanguard, ...); on the last victim of a set the objective or failure never triggers.
// True: before a victim's kill callbacks run, its self table gets Dead = true and
// KillReason (the host's recorded cause), as 00929800 does (the later publisher writes
// the same values again); and the `hit` channel is evaluated before `kill`, as the image
// fires it inside the hit (0077CE60 -> 00988510) ahead of the flush, so a hit callback
// on the lethal hit still sees the victim alive. False: as before.
inline constexpr bool kKillListenerDeadFirstBound = true;  // ON: lead pair (USN02/PRCP01/LOMP06 wins identical, 2026-10-13)

// Packet cc13-ver8 (docs/CC13_VER_8.md V1). The image reaches the `kill` channel 009813A0
// from two places only, and both require cause 1 ("harm"):
//  - 00959450 (planes through 007BCAA0, ships through 00824B60, 00747170, 0074DB70):
//    00959460 COMISS clock [00F876A4] against [00D7A24C] = 1.0f, JBE past the report;
//    00959475 CMP [EDI+70h],1 / JNE past it; 0095948A world+4ACh; 009594C4..009594FE the
//    controller veto; then 00959507 CALL 009813A0.
//  - 007F3B10 (the squadron): 007F3B19 CMP [ESI+70h],1 / JNE past it, world+4ACh, the
//    veto (+35Ch kind 17h with +36Ah); then 007F3B56 CALL 009813A0. No clock gate.
// The host fires `kill` for every destroyed row: planes that land (cause 5) or leave the
// map (cause 4), a squadron whose last plane landed or left, and Lua Kill(x, true)
// (cause 2, 008AC6DF..008AC71B) all run kill callbacks the image never runs.
// True: a death is dispatched only when its cause is 1 and, unless it is a squadron
// slot, its time is above 1.0 s. The cause is the units host's recorded cause, or the
// cause of this host's own Lua Kill (the gunnery funnel drops it); a fused squadron
// slot takes the cause of its last member to die, as 007F3A2A kills the squadron with
// the last plane's +70h. SUBSTITUTIONS (labelled): the controller veto is not modelled
// (its bytes +C41h / +100Ah / +36Ah are not held here); the death time is the gunnery
// host's death clock, not the flush's [00F876A4]; members that die on the same clock
// are ordered by unit index. False: every death is dispatched, as before.
inline constexpr bool kKillListenerHarmOnlyBound = true;  // ON: lead pair, six wins identical (2026-10-13)

// Packet cc13-fix3 (docs/CC13_VER_8.md V5). The `exitzone` channel 00982120,
// __thiscall(manager [00F8A0C4], entity, exited byte), RET 8. It replaces the entity with
// entity->vtable[140h]() (a plane's is 007B97E0, MOV EAX,[ECX+9D4h]: its squadron; a ship's
// is 0047F320, itself), boxes the entity's word +174h (00D1AF24) and the byte (00D1AF34),
// and calls every subscription (vtable 00D1B69C, loader 009726D0: callback, `entity` +0Ch,
// `exited` +1Ch, a boolean set) whose Matches 00968710 holds; each callback is called with
// (entity, exited) through 00884A90 and 00884A30. Producers: 007F31A0 (the squadron's
// plane-left-map arm, reached from 007EE410) fires (plane, 1) at 007F31E6 before its
// Kill(4), only when squadron+361h is clear and the game state ([00E188A8], 004BCA50) is
// not 9; 007C6C30 fires (plane, 0) at 007C6D65 (the leave warning); 0081BEB0 and 00826D70
// are the ship out-of-map countdown, not hosted. This installation has 36 subscriptions;
// the host never delivered one.
// True: AddListener keeps the two sets, and each host plane death of cause 4 ("exitzone",
// the units host's 007F31A0 Kill(4)) fires (squadron, true) once, before `entityKilled`
// and `kill`, as 007F31E6 runs ahead of 007F3418. note_exit_zone_00982120 is the hook for
// the ship producer (its entity is the ship) and for the plane warning.
// SUBSTITUTIONS (labelled): the event is taken from the host's destroyed-unit rows on the
// next mission frame, not inside 007F31A0; the +361h latch is not held by this host, so
// only the first batch of a squadron's exits (the deaths at the clock of its first
// cause-4 death: the arm's caller and the members its leader arm sends out) fires, and
// later exits of the same squadron count as latched; the state-9 gate is not modelled
// (the host never runs state 9 inside a mission); the warning (plane, 0) has no host
// producer yet, and no subscription in this installation watches a plane with
// exited = false. The HUD warning 00982120 builds for exited = 0 is not modelled.
// False: exitzone subscriptions are stored and never fire, as before.
inline constexpr bool kLuaExitZoneListenersBound = true;  // ON: lead pair, six wins identical (2026-10-13)

// Packet cc9_lua54_prcp01 (docs/SQUADRON_LAND_TASK.md 5gj): the four natives on the
// config at [game+21C4h]. 008B2320 SetSimplifiedReconMultiplier stores Lua argument 1
// at +74h (008B243C), 008B24A0 GetSimplifiedReconMultiplier pushes it (008B258A);
// 008B25F0 / 008B2770 do the same for SimplifiedSonarMultiplier at +78h (008B270C).
// Both default to 1.0f (00444D20). True: the host keeps the two values and answers
// the getters. False: the rows are unimplemented and the getters push nothing, so
// commandhelpers.lua's luaPutToRecon (line 16266, `dist * GetSimplifiedRecon...()`)
// raises an arithmetic-on-nil error. LABELLED: a set value is not forwarded to the
// gunnery host's recon detection (`simplified_recon_multiplier()` stays 1.0f) unless
// the gunnery host's kReconMultiplierForwardBound is ON (packet cc9_g40_recon_mult).
inline constexpr bool kSimplifiedReconNativesBound = true;   // ON: SQUADRON_LAND_TASK 5gj

// Packet cc9_lua55_genobj (docs/SQUADRON_LAND_TASK.md 5gl). GenerateObject 00944FD0 ->
// 0046D930 instantiates on every call; this host answers a repeat call with the earlier
// entity (a deviation against duplicated capital ships). PRCP01's luaEngineMovieInit
// generates the Junyo and Zuiho groups for the intro, phase 0 kills them (usn_03:599),
// and phases 3 and 4 generate them again (usn_03:1135, 1244), which here answered the
// dead entity. True: a repeat call whose earlier instance is Dead (its thisTable slot)
// makes a new entity; a live earlier instance keeps the deviation. False: as before.
inline constexpr bool kGenerateObjectRespawnsDeadBound = true;   // ON: SQUADRON_LAND_TASK 5gl
// Packet p16_lua_natives (docs/OCEAN_WAVE_FIELD.md section 65; cc13 verify5 R5). True:
// the deviation goes: every call instantiates, as 0046D930 does, live earlier instance
// or not (usn_03:1621-1636 makes Max Kate/Val pairs; ijn_23 FinalJP1..3 three each).
// False: kGenerateObjectRespawnsDeadBound's narrowed deviation.
inline constexpr bool kGenerateObjectEveryCallBound = true;   // ON: PRCP01 pair (OCEAN_WAVE_FIELD 66)
// Packet p16_lua_natives (cc13 verify5, census `:3689`). True: SpawnNew's placement
// walk 00941D30 refuses a member outside the map through 0071C4F0 on this host's
// own Map bounds (border_bounds_, loaded by set_world_border_zones), so the image's
// retry / requeue arms are reached. With no bounds loaded nothing is refused.
// False: no member is ever outside the map.
inline constexpr bool kSpawnPlacementMapBoundsBound = true;   // ON: JM06 pair (OCEAN_WAVE_FIELD 66)

// docs/CC13_AUDIT_LUA51_FLIPS.md item 1c. 00988510 boxes, and 0097B8C0 matches,
// the target as hitEntity->vtable[140h]() kept only when it answers IsKindOf(2)
// (00988530..0098859B): for a plane that is its squadron (007B97E0, plane+9D4h).
// True: a hit on a squadron member is reported with the squadron as target.
// False: the hit unit, as before. ON (5fz): BSM04's UNDERATTACK1 now starts at 73.95 s on the first
// hit to B-17|.-2 instead of waiting for wing 0; gameplay identical.
inline constexpr bool kLuaHitTargetSquadronBound = true;

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
// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 1). 0088EFF0
// SetFailure(entity, name, ...) calls entity->vtable[194h](name, frame) and returns
// nothing; 0088F1D0 GetFailure(entity, name) calls vtable[198h](name, frame, &out)
// and pushes out's low byte. On the eight ship vtables those are 008132C0 and
// 008135C0 (the jam bytes +9E4h / +9E5h, Explosion, Periscope); 00953DA0 /
// 00953E60 are the base pair (InferiorFailure, +720h). True: both rows run on the
// gunnery and units hosts. False: unimplemented, nil.
inline constexpr bool kLuaSetFailureBound = true;  // ON: n1 pairs (GUNNERY 197-199, cc9-gunnery38)
// Packet cc13-fix11 (docs/CC13_FIX11.md). 008AC7B0 SetDeadMeat(entity), no result:
// a plane (IsKindOf 0Fh) gets vtable[194h]("spinning", 0) at 008AC8FB; a ship
// (IsKindOf 6) takes one stream-1 draw U(-1, 1) at 008AC94B (00BD2F10, ECX = 1)
// into the sink pivot's x, then 008110F0 at 008AC96A (refused on +5Dh or
// +150h > 0, else vtable[70h](1)); anything else gets vtable[70h](1) at 008AC97A.
// True: run_set_dead_meat_008ac7b0 on the gunnery host's draw and death funnel
// (the plane arm is counted, not applied). False: the row stays UNIMPLEMENTED.
inline constexpr bool kLuaSetDeadMeatBound = true;  // ON: BSM01 pair, SQUADRON_LAND_TASK 5ih
// Packet cc13-fix12 (docs/CC13_FIX12.md). 008AB580 IsUnitSelectable(entity): 008AB67F
// 00888AA0, 008AB696 MOV DL,1 / CALL 00645160 (EDX = [[00E188A8]+18ECh], then
// 00645060(unit, team, 1)), 008AB6A4 PushBoolean; one result. True:
// run_is_unit_selectable_008ab580 on GameHudHost::unit_selectable_00645060. False: the
// row stays UNIMPLEMENTED and pushes nothing. Flip with kLuaGetEntitySpeedBound
// (usn_11 / prcp_11 CheckMontR* test both in one `and`).
inline constexpr bool kLuaIsUnitSelectableBound = false;  // OFF: committed unpaired
// Packet cc13-fix12. 008C0820 GetEntitySpeed(entity): 008C091F 00888AA0, 008C0936
// unit->vtable[38h]() with no null test, 008C093F FSTP float, 008C094F 00B66480;
// one result. Slot 38h: ships 0080E0F0 (0092D730 on unit+1018h, the signed body
// forward speed), planes 007B8E60 (FLD [unit+B1Ch]), PlaneSquadronGen 007ED470 (the
// leader +3D0h's slot 38h). True: run_get_entity_speed_008c0820. False: UNIMPLEMENTED,
// nothing pushed.
inline constexpr bool kLuaGetEntitySpeedBound = false;  // OFF: committed unpaired
// Packet cc13-fix18 (docs/CC13_FIX18.md section 2). 008AAB90 ClearForcedReconLevel(entity,
// party), no result: 008AAC90 00888AA0 resolves argument 0 (no null test), 008AACC1
// 00B66290 reads argument 1 as the party; 008AACE2 00803B50 on the record
// entity+1E8h+party*34h, then a squadron (vtable[5Ch](18h)) clears each +3D0h member's
// record (count +3CCh, 008AACF6-008AAD27)
// and a LandConvoy (1Ah) each element of the +394h vector (008AAD83-008AADE4). True:
// run_clear_forced_recon_level_008aab90 through bsp::clear_forced_recon_level_00803b50
// for the slot and its squadron members. False: the row stays UNIMPLEMENTED and the
// forced table keeps the pair.
inline constexpr bool kLuaClearForcedReconLevelBound = true;  // ON-unexercised, body checked (SQUADRON_LAND_TASK 5ii)
// Packet cc13-fix18 (docs/CC13_FIX18.md section 6). 00894F20 SetCrewLevel(entity, level),
// no result: 00895022 00888AA0 argument 0, 00895053 00B66290 argument 1, 0089506C
// 006E6210(ECX = level), 0089507C entity->vtable[128h](skill) with no null test. 006E6210
// with the session word [00E188A8]+1FE4h zero (single player) maps 0 -> 0, 1 -> 1,
// 2 -> 2, 3 -> 5 and anything else -> 1. True: run_set_crew_level_00894f20 writes the
// skill index of the unit (006D1EB0) or of each member of a squadron (007ECF80) through
// GameUnitsHost::set_unit_skill_index_006d1eb0. False: UNIMPLEMENTED.
inline constexpr bool kLuaSetCrewLevelBound = true;  // ON-unexercised, body checked (SQUADRON_LAND_TASK 5ii)
// Packet cc13-fix10 (docs/CC13_FIX10.md; cc9 grant 2026-10-17). 008C9730
// AddProximityTrigger(entity, name, callback, target, range) makes a 60h trigger
// (007505C0 / 0074FEE0: target descriptor +44h from 0088A810, +5Ch = range * range)
// and joins it to the entity's tick element sub-list (vtable[60h], 00876020, priority
// 2). Wave 2 of the fixed step (00875B90 -> 008759B0) runs 00750000: planar x/z test
// of the watched entity's world translation against the target, firing when
// +5Ch > d2; 0074FDD0 calls the Lua global callback(entity, name or nil) through
// 00887E50 and then retires the trigger (vtable[10h] 0071C4A0), which the same walk
// releases: one-shot. 008C5FD0 RemoveTrigger(entity, name) retires every trigger of
// that name (0074FBB0, _stricmp) and pushes whether any matched.
// True: both rows run here, and run_proximity_triggers_008759b0 walks the lists once
// per fixed step after the motion step (game_hosts_mission_frame.cpp).
// Packet cc13-fix19 (docs/CC13_FIX19.md) adds the target-gone observer under this
// switch: 0074FEE0 registers it (00694A60) for every entity target, so the image has
// it whenever such a trigger exists. The observer's slot +4h (0074FFE0, adjustor
// table 00D00568) is reached only by the kill list's pass K (009273A0 row 15,
// 009274DE..00927510 -> 00696330); slot +8h, the destroy-list notice (00925C90), is
// 0042B120, a bare RET 4. So a target that is Killed (00926D90) retires the trigger
// silently through vtable[10h] at row 15, after that step's wave-2 walk; a target
// destroyed by damage does not, until its wreck's KillDepth Kill.
// SUBSTITUTIONS (labelled at the code): host units only carry lists; unit_active()
// stands for wave 2's +5Eh/+BDh admission; 00887E50's queued arm is not modelled;
// the observer is fed from the host's Kill sites only (target_gone_00925c40); the
// line-of-sight trigger stays unbound, so RemoveTrigger answers false for its names.
// False: both rows stay UNIMPLEMENTED and push nothing, and nothing is walked.
// ON (cc9-physics18, OCEAN_WAVE_FIELD 69-70): PRCP05's airfield trigger fires and
// completes primary 1; USN11 / USN10 identical.
inline constexpr bool kLuaProximityTriggersBound = true;
// Packet p18_spawn_missing_class (cc9-physics18, docs/OCEAN_WAVE_FIELD.md 70).
// SpawnNew 00949750 builds each groupMembers entry through 00945C30 -> 00944210, whose
// 00964790 reads the member's class: VehicleClass[classIndex] (00B67720, a nil object
// when the row is absent), then its `Type` through 00B67800 -> lua_gettable 00A67C20
// -> luaV_gettable, which raises "attempt to index a nil value" on the nil row. That
// error unwinds the SpawnNew native into the script call's protected call, so the
// script function aborts and nothing is spawned (the members are read inside the
// native, before 009484xx creates any). This installation's vehicleclasses.lua
// (2026-05-09) has no row 240, which usn_09_Leyte.lua's shipyard PTs ask for.
// True: a member whose Type has no VehicleClass row raises that error. False: the
// member is created and the units host aborts the run at its observer projection.
// ON (OCEAN_WAVE_FIELD 71): PRCP05 raised once at 1061.96 s (US09Think fails at
// line 1093) and ran its 36000 frames; OFF stopped there at the observer projection.
inline constexpr bool kSpawnNewMissingClassRaisesBound = true;
// Packet cc13-fix19 (docs/CC13_FIX19.md). 008C9A10
// AddSquadOrdnanceDroppedTrigger(entity, name?, callback): 00750660 makes a 34h trigger
// (vtable 00D005C0, +11h = 1, +18h = 2), 00750790 stores the name and callback, and
// the entity's list (vtable[60h]) takes it through 0074FAB0 -> vtable[4h] 007504C0:
// 00876020, then +30h = element+28h (the entity). Wave 2 runs 007504F0 each fixed
// step: +30h null retires it silently; otherwise 007ED790(ECX = +30h, 0) asks every
// member (+3D0h, count +3CCh) 007B9140(member, 0) (kind 17h answers true; else a rack
// holding a kind-2Ah round), and when none can drop, 0074FDD0 calls the Lua
// callback(entity, name or nil) and retires the trigger: one-shot. No float math,
// no RNG. RemoveTrigger (0074FBB0) retires it by name like any Lua trigger.
// True: the row runs here, the trigger joins the unit's list that
// run_proximity_triggers_008759b0 walks, and RemoveTrigger is bound and matches it.
// SUBSTITUTIONS (labelled at the code): the entity must be a squadron of the
// host's registry; its live members (registry members the gunnery host does not
// count dead) stand for +3D0h/+3CCh; a squadron with none is not tested.
// False: the row stays UNIMPLEMENTED and pushes nothing.
// ON (cc9-physics18, OCEAN_WAVE_FIELD 70): USNRM01's Oklahoma_Killers trigger fires and
// luaOklahomaKillersRetreat runs (the mission ends 784.93 s, was 569.98 s); BSM04 identical.
inline constexpr bool kLuaSquadOrdnanceDroppedTriggerBound = true;
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
// Packet cc13-fix6 (docs/CC13_VER_13.md, D1). The result test at 008A6F58 is
// `CMP byte [EAX+5Dh],0 / JNE nil` on both arms; +5Dh is set at the end of the kill step,
// while +5Ch (unit_active) stays set through the whole wreck descent. True: a resolved
// target with GameUnitsHost::unit_flag_005d set answers nil. False: the unit_active test.
inline constexpr bool kLuaAttackTargetDeadNilBound = true;  // ON: SQUADRON_LAND_TASK 5hc

// Packet cc9_squadron_set_speed (docs/LUA_BINDING_MISSION.md, "SquadronSetSpeed,
// 0089F780"). SquadronSetSpeed(squadron, speed) calls member->vtable[3Ch](speed) for
// each of the +3CCh members at +3D0h (0089F8CA..0089F8FF); on a plane that is
// 0074E1E0 -> 007D9E80, the controller's forward-speed set. True: route the row to
// run_squadron_set_speed_0089f780. False: unimplemented.
// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 4). 0089F950
// SquadronForceGunnerMode(entity, flag) stores argument 1 (00B66250) in the byte
// entity+379h (0089FA9C), with no class test and no result. Its only reader is the
// HUD root update (0064A1E0 on the controlled kind-18h unit) and 00647080, which
// keep the player in the gunner interface (23h). True: the byte is kept per unit
// for squadron_force_gunner_mode_379. False: unimplemented, nothing stored.
inline constexpr bool kLuaSquadronForceGunnerModeBound = false;
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

// Packet cc9_lua_submarine_on_surface (docs/LUA_BINDING_MISSION.md,
// "GetSubmarineOnSurface, 008942C0"). GetSubmarineOnSurface(entity) resolves argument 0
// through 00888AA0 and pushes 00852820's answer: the world y at +100h above
// (+1204h + +1200h) / 3.0, with no class test. True: route the row to
// run_get_submarine_on_surface_008942c0. False: unimplemented (nil, read as false).
inline constexpr bool kLuaGetSubmarineOnSurfaceBound = true;  // ON: JM06 24000 pair (docs/LUA_BINDING_MISSION.md)
// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 2). 00899500
// TrulyDead(entity) reads argument 0's `Ptr` field (00B67800, key 00CFAD08) as
// userdata (00B662D0) and pushes `Ptr == null` (00B66450). Only the on-killed slot
// 00928C80 (vtable[80h], from the kill list's drain) nulls `Ptr`. True: the same
// read on the host's self tables. False: unimplemented, nil.
inline constexpr bool kLuaTrulyDeadBound = true;   // ON: JM06 / USN04 pairs (OCEAN_WAVE_FIELD 66)
// Packet p16_truly_dead (docs/OCEAN_WAVE_FIELD.md section 65; cc13 verify5 R3). True:
// where the units host drains a unit off the kill list (the KillDepth drain and the
// row-15 removal, both recording 00928C80), the Lua half of 00928C80 runs: the self
// table's `Ptr` becomes lightuserdata(0) (00928D04, 00B67530(self, "Ptr", null)), so
// TrulyDead answers true from then on. LABELLED: the last-position table
// (00928D49..00928E7E) is not written. False: `Ptr` is never nulled.
inline constexpr bool kLuaTrulyDeadOnKilledBound = true;   // ON: JM06 / USN04 pairs (OCEAN_WAVE_FIELD 66)

// Packet cc9_set_air_base_slot_count (docs/LUA_BINDING_MISSION.md, "SetAirBaseSlotCount,
// 008963E0"). SetAirBaseSlotCount(entity, n) resizes the air-ops block's 58h slot array
// (+4Ch, count +50h) to exactly n through 006C7E20: new slots are default records, a
// shrink destroys from the tail. True: route the row to
// run_set_air_base_slot_count_008963e0. False: unimplemented.
inline constexpr bool kLuaSetAirBaseSlotCountBound = true;  // ON: pairs held, one recorded miss (docs/LUA_BINDING_MISSION.md)

// Packet cc9_lua_add_air_base_stock (docs/SQUADRON_LAND_TASK.md 5dt). AddAirBaseStock /
// AddAirBasePlanes(entity, class, count), 00896A90: the block from 006BCD20 (filter 0,
// 00896BAE), the class from 00964790 over argument 1, 0095BA60 (the party preload), then
// 006CA770(class, argument 2) at 00896C54: add to the entry of that class or push
// {class, count, 5}. True: route the row to run_add_air_base_stock_00896a90 on the deck
// registry's stock list. False: unimplemented.
inline constexpr bool kLuaAddAirBaseStockBound = true;   // ON: SQUADRON_LAND_TASK 5dt.1

// Packet cc9_lua_add_shipyard_stock. 00896CC0 AddShipyardStock(entity, class, count
// [, names]): 00888AA0 on argument 0 (the object in EDI), arguments 1 and 2 as integers
// (00B66290; the class is an id, not resolved through 00964790), argument 3 as a string
// when there are four (00B662B0, else 00CE3A0C), then 0084ACB0(class, count, names) with
// ECX = the object (00896E81..00896E8A). Returns no value. True: route the row to
// bsp::shipyard_add_stock_0084acb0 on the bsp::shipyards() record. False: unimplemented.
inline constexpr bool kLuaAddShipyardStockBound = true;   // ON: SQUADRON_LAND_TASK 5ej.1

// Packet cc9_hit_listener_filters (docs/LUA_BINDING_MISSION.md, "The unmodelled `hit`
// filters, bound"). 00988510 hands the channel eight parameters: target, targetDevice
// (the hit record's own entity when it is a live child other than the victim), attacker,
// attackType, attackerPlayerIndex ([src+1Ch]), and the record's floats +48h
// damageCaused, +4Ch fireCaused, +50h leakCaused. True: targetDevice, fireCaused and
// leakCaused are matched against what this process's hits carry (no device entity, no
// fire, no leak: 0.0); attackerPlayerIndex stays unmodelled. False: an entry naming any
// of the four is counted unmodelled and never fires.
inline constexpr bool kLuaHitFilterFieldsBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_hit_attacker_player_index (docs/LUA_BINDING_MISSION.md, "attackerPlayerIndex,
// bound"). 009725B0 loads the entry's attackerPlayerIndex into +4Ch through 009722D0
// (a list of integers, 00971250); 00988510 passes [src+1Ch], the shot's stamped team
// (0072C0FB / 0072C14B / 006E5527), or -1 with no ordnance. True: the entry keeps the set
// and dispatch tests the event's GameGunneryHitEvent::attacker_player_index against it
// (set membership, as for the recon party set; the comparison itself is unread).
// False: an entry naming it is counted unmodelled and never fires.
inline constexpr bool kLuaHitAttackerPlayerIndexBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc9_get_closest_border_zone (docs/WORLD_MAP_BOUNDS.md, "Border zones";
// docs/LUA_BINDING_MISSION.md, "GetClosestBorderZone, bound"). 008AECD0
// GetClosestBorderZone(position[, offset]) answers a point beyond the nearest
// border zone of world+7134h (004C7730, any side), built from the map bounds by
// 004D5BD0 / 004C71C0 / 004C7150. True: the native runs on the zones the mission
// frame hands over (set_world_border_zones). False: unimplemented, neutral.
// Packet cc9_spawn_new_id_queries (docs/LUA_BINDING_MISSION.md, "SpawnNewIDIsRequested
// and SpawnNewIDRemove, bound"). 00946380 -> 00945850 answers whether the
// SpawnNew queue at *(00F89B3C) holds a record whose id (+B8h) matches argument
// 1 (length, then __stricmp), pushing one boolean; 00946390 -> 00945A20 removes
// every such record and pushes nothing. True: both run on the host's queue.
// False: unimplemented, neutral.
inline constexpr bool kLuaSpawnNewIdQueriesBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
inline constexpr bool kLuaClosestBorderZoneBound = true;  // ON: pairs (docs/LUA_BINDING_MISSION.md)
// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 3). 008AF020
// IsInBorderZone(entity | position): an entity handle (008889C0) gives the pose
// translation +FCh..+104h, anything else the 00888760 vector; then 0071C4F0 on the
// game singleton [00E188A8] and, only when the point is outside the map, the side
// (+4h) of 00489C40's zone record through 00B664B0. Inside the map nothing is
// pushed. True: run it on the zones set_world_border_zones keeps. False:
// unimplemented, nil (the inside answer).
inline constexpr bool kLuaIsInBorderZoneBound = true;   // ON: JM06 pair, 82 calls (OCEAN_WAVE_FIELD 66)

// Packet cc9_hit_rate_limit (docs/LUA_BINDING_MISSION.md, "The hit-callback rate
// limit"). 00988510 keys a map at this+168h (009882F0 / 00499030) by (victim,
// attacking unit). Before the channel lookup it evaluates only when the stored time
// is at or before the clock DAT_00F876A4, then stores clock + 2.0 (00CE3958), or
// clock + 1e-4 (00CE3C68) for ordnance kinds 8..0Fh, 12h and 13h. True: the host's
// hit dispatcher applies it, the kind being GameGunneryHitEvent::ordnance_kind.
// False: every hit is evaluated.
inline constexpr bool kLuaHitRateLimitBound = true;  // ON: identity pairs (docs/LUA_BINDING_MISSION.md)

// Packet cc13-atkkind. 00988510's attackType box is the C string 00E08E58[kind]
// (00988B02 MOV EAX,[ESP+80h]; 00988B09 MOV ECX,[EAX*4+00E08E58h]; 00988B10 PUSH ECX,
// re-boxed for the squadron pass at 009890DC), kind = [[shot+4]+8] (00988674..00988682),
// the projectile class's sub-type after 006E9968 rewrites 1 -> 2/3 and 4 -> 5/6/7.
// True: the hit listener's attackType set is matched against that string, the one the
// host already pushes as the callback's argument 4. False: against the bullet class's
// Lua `Type`, which over-matches "BULLET"/"ARTILLERY" and never matches MACHINEGUN,
// AAMACHINEGUN or LIGHT/MEDIUM/HEAVYARTILLERY. Not modelled either way: the two
// arms that force kind 11h (KAMIKAZE) - 0098862A, with BL = 1 at 00988639, when
// [[esi+538h]+510h] or +514h > 0.0 (00988604..00988625), and 00988658 when the
// attacker's vtable[5Ch](17h) is true (0098864D) - and 0098869E's kind 0 for a null
// shot, which is moot because every host hit carries a class.
// summary listener_hit_attack_type_subkind_differs counts candidate disagreements per
// listener evaluation (before the damage test, second pass included), not changed fires.
inline constexpr bool kHitListenerAttackTypeSubKindCc13Bound = false;
static_assert(!kHitListenerAttackTypeSubKindCc13Bound
                  || (kLuaListenersBound && kLuaHitListenersBound),
              "the attackType sub-kind test lives in the hit listener dispatch");

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
// Packet cc9_get_formation_leader (docs/LUA_BINDING_MISSION.md). 00899AF0
// GetFormationLeader(unit): argument 0 through 00888AA0 (00899BEF), then
// 007788D0 BSP_Unit_FormationLeader ([unit+284h] ? [group+14h] : 0, 00899C08);
// a null leader pushes nil (00899CB4 -> 00B66430), otherwise the leader's
// thisTable slot keyed by its +174h id (00899C15..00899C65). True: the row
// answers from the units host's group; false: the entity arm answers nil.
inline constexpr bool kLuaFormationLeaderBound = true;   // ON by its pairs (LUA_BINDING_MISSION)
// Packet cc9_get_last_catapulted (docs/LUA_BINDING_MISSION.md). 00892860
// GetLastCatapulted(ship): argument 0 through the object handle; an entity that
// does not answer vtable[5Ch](6) pushes nothing and returns 0 results; a ship
// asks 00953A60, which reads unit+630h (set to -1 at 0095CDDD and written only
// by 00957450 BSP_UnitInstance_AddLaunchedChild, on the launched-child message)
// and answers 0 for a negative index, pushed as nil, else the plane in launch
// slot +550h + 18h * index, pushed as its thisTable slot. This host builds no
// catapult launch, so the index stays -1. True: the row answers from that
// state; false: the row stays unimplemented.
inline constexpr bool kLuaLastCatapultedBound = true;  // ON by its pairs (LUA_BINDING_MISSION)

// Packet cc9_lua55_catapult_stock (docs/SQUADRON_LAND_TASK.md 5gm). GetCatapultStock
// 00892DE0: vtable[5Ch](5), then max(unit+638h, 0); 0 for any other entity. unit+638h
// is written at init from the class Catapult.LaunchStock (0095E5B0: [unit+538h]+C8h),
// decremented by a catapult launch (00957450) and set by SetCatapultStock (009539A0).
// True: answers the class LaunchStock (LABELLED: neither writer is built here, so the
// constructor value stands). False: the unimplemented row pushes nothing, which breaks
// commandhelpers' luaCheckpoint (`GetCatapultStock(unit) > 0`) for every ship.
inline constexpr bool kLuaGetCatapultStockBound = true;   // ON: SQUADRON_LAND_TASK 5gm
// Packet cc13-fix14b (docs/CC13_FIX14.md). 00892AA0 GetNumCatapulted(ship): argument
// 0 through 00888AA0; a null entity, or one that does not answer vtable[5Ch](6) (the
// ship family), pushes integer 0 (00892BD3 PUSH EBP); a ship pushes 00953920's count
// of its ten launch slots +550h..+628h (stride 18h) whose first dword is non-zero
// (00892BCB, 00892BD8 00B664B0 PushInteger). One result either way. True: the row
// answers from this host; no path here fills a launch slot (the catapult launch
// 006EC8E0 -> 00957450 is not built), so a ship's count is 0, LABELLED. False: an
// unimplemented record that pushes nothing (jm02's luaJM2CheckReconplanes compares
// the nil `num`).
inline constexpr bool kLuaGetNumCatapultedBound = true;  // ON with kShipCatapultLaunchBound (OCEAN_WAVE_FIELD 69)
inline constexpr bool kLuaFormationQueryBound = true;  // ON: mechanism matched, spread miss recorded (docs/LUA_BINDING_MISSION.md)

// The process-wide 00E17BF2. It is reset from the lobby flags when a mission's settings
// are published and written by SetDeviceReloadEnabled. It answers false while
// kLuaDeviceReloadEnabledBound is false.
bool lua_device_reload_enabled_00e17bf2() noexcept;

// Packet cc13-fix18 (docs/CC13_FIX18.md section 5). 008C14B0 SetDeviceReloadTimeMul(mul),
// no entity, no result: 008C15AF 00B66270 (lua_tonumber) on argument 0, 008C15B4 FSTP
// float [00E08178]. The mission load sets the float to 1.0 (004DFBDD-004DFBEB, [00D7A24C])
// and so does the session-mode setter (0076FE73). Its two readers are the rack reload
// countdowns 006E41C0 (006E41D5) and 006E44C0 (006E44D5): +490h -= float(mul * dt). True:
// route the row to run_set_device_reload_time_mul_008c14b0, and the units host's rack
// reload slice (006E57A4) scales its step by lua_device_reload_time_mul_00e08178(). False:
// UNIMPLEMENTED, and the accessor answers 1.0.
inline constexpr bool kLuaDeviceReloadTimeMulBound = true;  // ON: cc9-ships58, SHIP_AI_OPEN_ITEMS 286
// The process-wide float 00E08178; 1.0 while kLuaDeviceReloadTimeMulBound is false.
float lua_device_reload_time_mul_00e08178() noexcept;

// Packet cc9_add_untouchable_unit (docs/LUA_BINDING_MISSION.md). 008AC140
// AddUntouchableUnit(unit): argument 0 through 00888AA0 (008AC23B), then
// [unit]->vtable[140h]() (008AC255..008AC25D, the unit's AI object) and
// MOV byte [EAX+1D4h],1 (008AC263). No other write, no null test, no result
// (008AC269 00B66400). The byte is what the unit-AI untouchable gate 00862440
// reads. True: the row sets the flag below; false: the row stays unimplemented.
// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 7). 008A24E0
// SquadronSetReleaseAlt(squadron, altitude[, force]) writes the squadron's third
// block: +384h = 0.5, +38Eh = force (only with exactly three arguments), +39Ch =
// altitude, +3ABh = 1, +3ADh = 0. 009C8920 overwrites +39Ch with 9999 once the
// countdown and lock allow, and 009C7A80 reads it into the dive altitude. True:
// route the row to the units host's third block. False: unimplemented.
inline constexpr bool kLuaSquadronSetReleaseAltBound = true;  // ON: BSM09 pair (OCEAN_WAVE_FIELD 69)
// Packet cc13-fix4 (docs/CC13_VER_10.md section 4, divergence D2). 008A20E0
// SquadronLandAndKill(squadron): 007F1B70 frees the launch slot on every carrier
// and airfield (008A220F), then from the last plane down Kill(plane, 5 "landed")
// (008A223F) and 007F3970(plane, 1) (008A2249); the last removal kills the
// squadron (007F3A2A). True: the row runs squadron_land_and_kill_008a20e0
// (src/air_operations.cpp) over the deck registry and the gunnery host's kill.
// False: the row stays unimplemented (PRCP01 keeps the extra squadrons flying).
inline constexpr bool kLuaSquadronLandAndKillBound = false;
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 1). 008975B0
// RemoveAllAirBasePlanes(base): 00888AA0 on argument 0, 006BCD20 with DL = 0, then
// 006C8840 on the block (008976D1): every stock node through 006C7B10 (the stock
// drops to zero and idle slots holding a stocked class are emptied), then the
// list is emptied. No result. True: the row runs air_base_remove_all_stock_006c8840
// (src/script_natives_fix5_cc13.cpp) on the entity's deck. False: unimplemented,
// so the scene stock survives and a following AddAirBaseStock adds to it
// (prcpjm08.lua:908, the Hosho).
inline constexpr bool kLuaRemoveAllAirBasePlanesBound = true;  // ON: lead pair (JM08 identical, 2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 3). 0089AC30 StartLanding2(unit):
// 00888AA0 on argument 0, then vtable[238h]() and its answer pushed as an integer
// (the 94h receiver's call, without the message): 0074A4C0 on an MLandingShip
// (vtable 00CFFA30), 008206F0 on every other ship vtable. True: the row runs the
// ship AI host's two models and pushes the answer. False: unimplemented, no
// result (jm08.lua:653 then fails on tonumber(nil) < 0).
inline constexpr bool kLuaStartLanding2Bound = true;  // ON: lead pair (JM08 identical, 2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 4). 0089CFE0 GetSquadronPlane(squadron,
// i): 00888AA0 on argument 0, argument 1 as an integer; when 0 <= i < [sq+3CCh] it
// pushes thisTable[[sq+3D0h+4i]+174h] (i < 5 and the member non-null) or nil, and
// pushes nothing for any other i. True: the row answers from the squadron registry
// record (live members in order). False: unimplemented, no result.
inline constexpr bool kLuaGetSquadronPlaneBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 5). 0089A020 DisbandFormation(unit):
// 00888AA0 on argument 0, then 0077CA60 on it (0089A138): with [unit+284h] set it
// snapshots the group's members (+4F8h count, 0070D060), the leader (+14h) last and
// skipped in the walk, and routes 77h with a null target (0077BD70's leave) to each
// one still in a group. True: the row runs the units host's leave for every member
// of the argument's group, the leader last. False: unimplemented.
inline constexpr bool kLuaDisbandFormationBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 6). The LandConvoy natives take
// argument 0 through 00888D20 (the table's Ptr) and make one store on the convoy:
// 00895570 LandConvoyStart -> 00741120 (+3A8h = 0), 008956E0 LandConvoyStop ->
// 00741130 (+3A8h = 1), 00895850 LandConvoySetSpeed(convoy, speed) -> 00741140
// (+368h = argument 1 through 00B66270). No result. True: the row makes the store
// on the units host's bound convoy of that name, which 00743060's stopped gate and
// speed already read. False: unimplemented.
inline constexpr bool kLuaLandConvoyStartBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
inline constexpr bool kLuaLandConvoyStopBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
inline constexpr bool kLuaLandConvoySetSpeedBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
// Packet cc13-fix5 (docs/CC13_FIX_5.md section 8). 00896750 LaunchAirBaseSlot(base,
// slot[, now]): the block (006BCD20), slot = argument 1 - 1, now = argument 2 only
// with three arguments. A slot in state 2 does nothing. Otherwise: 83h {slot, now}
// routed (00896964), slot+34h = 1 (00896972), 00874D00(CL=1), 006CDC70(0) on the
// block, 00874D00(CL=1) again, and when slot+28h is set, thisTable[squadron+174h] is the
// one result. 83h's receiver 006CA8E0 launches at once (006C7490 flag 1) or queues
// (state 2, the launch list), and consumes the slot's stock either way (006C7D10).
// True: the row runs that sequence on the entity's deck. False: unimplemented.
inline constexpr bool kLuaLaunchAirBaseSlotBound = true;  // ON: lead pair; PRCP01 completes 720.25 s ON (2026-10-13)
inline constexpr bool kLuaAddUntouchableUnitBound = true;   // ON on identity (LUA_BINDING_MISSION)
// Packet p16_lua_natives (docs/OCEAN_WAVE_FIELD.md section 65; cc13 verify5 R6). True:
// RemoveUntouchableUnit 008AC2B0 clears the byte AddUntouchableUnit sets (00888AA0 at
// 008AC3AF, vtable[140h] at 008AC3D0, `MOV byte [EAX+1D4h],0` at 008AC3D6), and
// IsUnitUntouchable 008AC420 pushes vtable[140h]() != null && its +1D4h != 0 as a
// boolean. LABELLED: an entity with no units-host slot is skipped (Remove) or answers
// false (Is), as AddUntouchableUnit already does. False: both rows stay unimplemented
// records, so a removal never happens and IsUnitUntouchable reads nil.
inline constexpr bool kLuaUntouchableClearBound = true;  // ON: lua70 pair, verdict FLIP (cc9, 2026-10-19)
// The +1D4h byte of units-host index `index`'s AI object, as AddUntouchableUnit
// left it. False for every unit while kLuaAddUntouchableUnitBound is false.
// Inert: nothing in this process reads it until the gunnery lane binds 00862440.
bool lua_unit_untouchable_1d4(std::size_t index) noexcept;

// Packet cc9_submarine_air (docs/SUBMARINE_MODEL.md section 13).
// SetUnlimitedAirSupply(entity, flag) stores lua_toboolean(argument 1) at unit+1280h
// (00893C00). Routed together with the air model: under kSubmarineAirBound
// (bsp/game_hosts_units.hpp) the row reaches GameUnitsHost::set_unlimited_air_00893c00;
// otherwise it stays unimplemented.

class GameHostLog;
class GameVfsHost;
class GameScriptOrdersHost;
class GameUnitsHost;
class GameMissionResultHost;
struct SceneSpawnPoolEntry;
struct GameSceneEntityRecord;

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
    // class+55Ch, the `DeathEfx` handle 00831BE8 stores (00870CD0 resolves the id);
    // the id is kept, 0 when the key is absent (packet cc13-luawire, 0092A240).
    int death_efx_id{0};
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
    float turn_roll_leader{0.0f};     // desc+260h, 007D28D4 (packet cc9_land_begin_state)
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
    // Packet cc9_unit_class_lands_troops: the three keys [class+538h]->vtable[2Ch]
    // reads. 00833B7D..00833BB6 (inside the ship reader 00831840) stores
    // class+78Ch only when `LandingShip` is non-zero, 00964790 resolves it and the
    // descriptor's vtable[18h](0Ch) answers true (00963C80 answers 0Ch only on the
    // MLandingShip descriptor); landing_ship_resolves is that store's condition.
    // LandingShipAmount is class+790h (ShipClassFields::landing_ship_amount) and
    // Rocketer is the landing-ship reader 0074C630's class+809h.
    int landing_ship_id{0};             // `LandingShip`, IntegerOr 0
    bool landing_ship_resolves{false};  // class+78Ch != 0
    int landing_ship_amount{0};         // class+790h `LandingShipAmount`, IntegerOr 0
    bool landing_ship_is_rocketer{false};  // class+809h `Rocketer`, BooleanOr false
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
    std::size_t class_rows_marked{0};     // VehicleClass rows given ID/Got (009648ED)
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
    unsigned long long get_property_ammotype_asked{0};
    unsigned long long get_property_ammotype_served{0};
    unsigned long long get_property_ammotype_zero{0};
    // Packet p16_torpedo_stock_property.
    unsigned long long get_property_torpedo_stock_asked{0};
    unsigned long long get_property_torpedo_stock_served{0};
    unsigned long long get_property_torpedo_stock_not_ship{0};
    unsigned long long get_property_squadron_state_asked{0};
    unsigned long long get_property_squadron_state_served{0};
    unsigned long long get_property_squadron_state_two{0};
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
    unsigned long long listener_entity_killed_deaths{0};   // packet cc13-ver5
    unsigned long long listener_entity_killed_fires{0};
    unsigned long long listener_exit_zone_events{0};      // packet cc13-fix3
    unsigned long long listener_exit_zone_latched{0};
    unsigned long long listener_exit_zone_fires{0};
    unsigned long long proximity_trigger_adds{0};          // packet cc13-fix10
    unsigned long long proximity_trigger_unresolved{0};
    unsigned long long proximity_trigger_checks{0};
    unsigned long long proximity_trigger_fires{0};
    unsigned long long proximity_trigger_callback_missing{0};
    unsigned long long remove_trigger_calls{0};
    unsigned long long remove_trigger_matched{0};
    unsigned long long target_gone_notices{0};             // packet cc13-fix19
    unsigned long long target_gone_retired{0};
    unsigned long long ordnance_trigger_adds{0};
    unsigned long long ordnance_trigger_unresolved{0};
    unsigned long long ordnance_trigger_checks{0};
    unsigned long long ordnance_trigger_fires{0};
    unsigned long long listener_kill_dead_first{0};          // packet cc13-ver5
    unsigned long long listener_kill_not_harm{0};            // packet cc13-ver8
    unsigned long long listener_kill_grace{0};               // packet cc13-ver8
    unsigned long long listener_attacker_filtered{0};
    unsigned long long listener_recon_changes{0};
    unsigned long long listener_recon_fires{0};
    // Packet cc9_recon_publication.
    unsigned long long recon_publish_passes{0};     // recon pass generations seen
    unsigned long long recon_publish_slots{0};      // 00806B10 calls (a slot published)
    unsigned long long recon_publish_unchanged{0};  // slots skipped: content as last published
    unsigned long long recon_publish_entries{0};    // category entries written
    unsigned long long recon_publish_excluded{0};   // 00805DE6 class exclusions
    unsigned long long recon_publish_no_category{0};// category 13h
    unsigned long long listener_hit_events{0};
    unsigned long long listener_hit_fires{0};
    unsigned long long listener_hit_unmodelled{0};
    unsigned long long listener_hit_attack_type_subkind_differs{0};  // cc13-atkkind, log-only
    unsigned long long listener_hit_second_passes{0};      // kLuaHitSquadronSecondPassBound
    unsigned long long listener_hit_second_pass_fires{0};
    // Packet cc9_lua51_listener_args.
    unsigned long long listener_kill_with_attacker{0};     // deaths with +2C8h set
    unsigned long long listener_kill_attacker_matched{0};  // filtered entries that held
    unsigned long long listener_kill_second_passes{0};
    unsigned long long listener_kill_second_pass_fires{0};
    unsigned long long listener_args_pushed{0};            // callbacks called with arguments
    // Packet cc9_set_invincible_native.
    unsigned long long invincible_calls{0};
    unsigned long long invincible_units{0};
    unsigned long long invincible_unresolved{0};
    // Packet cc9_get_closest_border_zone.
    unsigned long long border_zone_calls{0};
    unsigned long long border_zone_missing{0};
    unsigned long long in_border_zone_calls{0};     // packet cc13_natives1, 008AF020
    unsigned long long in_border_zone_outside{0};
    unsigned long long in_border_zone_no_zone{0};
    unsigned long long forced_recon_calls{0};
    unsigned long long forced_recon_units{0};
    unsigned long long forced_recon_sync_notifies{0};   // kForcedReconSyncNotifyBound
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
    unsigned long long set_failure_calls{0};        // packet cc13_natives1, 0088EFF0
    unsigned long long set_failure_applied{0};      // an arm of 008132C0 matched
    unsigned long long set_failure_other{0};        // not a ship, or no arm matched
    unsigned long long set_failure_unresolved{0};
    unsigned long long get_failure_calls{0};        // 0088F1D0
    unsigned long long get_failure_true{0};
    unsigned long long dead_meat_calls{0};          // packet cc13-fix11, 008AC7B0
    unsigned long long dead_meat_sunk{0};           // ship arm, 008110F0 not refused
    unsigned long long dead_meat_sink_refused{0};   // ship arm, +5Dh or +150h > 0
    unsigned long long dead_meat_destroyed{0};      // else arm, vtable[70h](1)
    unsigned long long dead_meat_plane_unapplied{0};  // plane arm, not hosted
    unsigned long long dead_meat_unresolved{0};
    unsigned long long spawn_missing_class_raises{0};   // packet p18_spawn_missing_class
    unsigned long long unit_selectable_calls{0};    // packet cc13-fix12, 008AB580
    unsigned long long unit_selectable_true{0};
    unsigned long long unit_selectable_unresolved{0};  // no entity: pushed false
    unsigned long long unit_selectable_no_hud{0};      // no HUD host: nothing pushed
    unsigned long long entity_speed_calls{0};       // packet cc13-fix12, 008C0820
    unsigned long long entity_speed_squadron{0};    // 007ED470 -> leader 007B8E60
    unsigned long long entity_speed_ship{0};        // 0080E0F0 -> 0092D730
    unsigned long long entity_speed_plane{0};       // 007B8E60
    unsigned long long entity_speed_unresolved{0};  // nothing pushed
    unsigned long long entity_speed_unmodelled{0};  // other class: nothing pushed
    unsigned long long clear_forced_recon_calls{0};       // packet cc13-fix18, 008AAB90
    unsigned long long clear_forced_recon_records{0};     // 00803B50 calls made
    unsigned long long clear_forced_recon_was_forced{0};  // ... on a forced pair
    unsigned long long clear_forced_recon_unresolved{0};
    unsigned long long crew_level_calls{0};               // packet cc13-fix18, 00894F20
    unsigned long long crew_level_unresolved{0};
    unsigned long long reload_time_mul_calls{0};          // packet cc13-fix18, 008C14B0
    unsigned long long support_manager_bann_calls{0};     // packet cc13-smbind, 0088C750
    unsigned long long support_manager_permit_calls{0};   // packet cc13-smbind, 0088C8E0
    unsigned long long support_manager_enable_calls{0};   // packet cc13-smbind, 0088CBB0
    unsigned long long support_manager_enable_unresolved{0};
    unsigned long long attack_target_calls{0};
    unsigned long long attack_target_fire_arm{0};
    unsigned long long attack_target_command_arm{0};
    unsigned long long attack_target_pushed{0};
    unsigned long long attack_target_nil{0};
    unsigned long long attack_target_unresolved{0};
    unsigned long long attack_target_squadron_standin{0};   // packet cc13-atkfix
    unsigned long long squadron_speed_calls{0};
    unsigned long long squadron_speed_planes{0};
    unsigned long long squadron_speed_unresolved{0};
    unsigned long long force_gunner_calls{0};       // packet cc13_natives1, 0089F950
    unsigned long long force_gunner_set{0};
    unsigned long long force_gunner_unresolved{0};
    unsigned long long class_changed_calls{0};
    unsigned long long class_changed_true{0};
    unsigned long long sub_depth_calls{0};
    unsigned long long sub_depth_stored{0};
    unsigned long long sub_depth_unresolved{0};
    // Packet cc13-luawire (bsp/cc13_lua_natives_wire.hpp).
    unsigned long long has_fired_calls{0};
    unsigned long long has_fired_true{0};
    unsigned long long has_fired_unresolved{0};
    unsigned long long has_fired_no_director{0};
    unsigned long long water_load_calls{0};
    unsigned long long water_load_routed{0};
    unsigned long long water_load_applied{0};
    unsigned long long water_load_skipped{0};
    unsigned long long water_load_unresolved{0};
    unsigned long long break_ship_calls{0};
    unsigned long long break_ship_broken{0};
    unsigned long long break_ship_no_halves{0};
    unsigned long long break_ship_unresolved{0};
    unsigned long long break_ship_puff_draws{0};   // 0092A240's 00BD2F10 draws
    unsigned long long player_commands_calls{0};      // 008A6C30, packet cc13-luawire
    unsigned long long player_commands_stored{0};
    unsigned long long player_commands_unresolved{0};
    unsigned long long visibility_calls{0};           // 008A13D0, packet cc13-luawire 7b
    unsigned long long visibility_planes{0};
    unsigned long long visibility_null_members{0};    // guarded: the image crashes
    unsigned long long visibility_unresolved{0};
    unsigned long long avoidance_calls{0};            // 008A0000
    unsigned long long avoidance_unresolved{0};
    unsigned long long base_unsupported_calls{0};     // 0089FAF0
    unsigned long long base_unsupported_added{0};
    unsigned long long base_unsupported_unresolved{0};
    unsigned long long cheat_turbo_calls{0};          // 008A62A0
    unsigned long long cheat_turbo_planes{0};
    unsigned long long cheat_turbo_unresolved{0};
    unsigned long long sub_surface_calls{0};
    unsigned long long sub_surface_true{0};
    unsigned long long sub_surface_unseeded{0};
    unsigned long long sub_surface_unresolved{0};
    unsigned long long truly_dead_calls{0};         // packet cc13_natives1, 00899500
    unsigned long long truly_dead_true{0};
    unsigned long long slot_count_calls{0};
    unsigned long long slot_count_resized{0};
    unsigned long long slot_count_unresolved{0};
    unsigned long long stock_add_calls{0};
    unsigned long long stock_add_created{0};
    unsigned long long stock_add_unresolved{0};
    unsigned long long shipyard_stock_calls{0};       // packet cc9_lua_add_shipyard_stock
    unsigned long long shipyard_stock_created{0};
    unsigned long long shipyard_stock_unresolved{0};
    unsigned long long shipyard_units_requested{0};   // packet cc9_shipyard_create_unit
    unsigned long long shipyard_units_made{0};
    unsigned long long device_reload_calls{0};
    unsigned long long device_reload_true{0};
    unsigned long long in_formation_calls{0};
    unsigned long long in_formation_true{0};
    unsigned long long formation_leader_calls{0};   // packet cc9_get_formation_leader
    unsigned long long last_catapulted_calls{0};    // packet cc9_get_last_catapulted
    unsigned long long catapult_stock_calls{0};     // packet cc9_lua55_catapult_stock
    unsigned long long last_catapulted_nil{0};
    unsigned long long last_catapulted_not_ship{0};
    unsigned long long num_catapulted_calls{0};     // packet cc13-fix14b
    // Packet p17_catapult_adapter.
    unsigned long long use_catapult_calls{0};
    unsigned long long use_catapult_launches{0};
    unsigned long long use_catapult_no_catapult{0};
    unsigned long long use_catapult_no_stock{0};
    unsigned long long use_catapult_in_air{0};
    unsigned long long use_catapult_spawn_failed{0};
    unsigned long long set_catapult_stock_calls{0};
    unsigned long long last_catapulted_live{0};
    unsigned long long num_catapulted_not_ship{0};
    unsigned long long untouchable_calls{0};        // packet cc9_add_untouchable_unit
    unsigned long long untouchable_marked{0};
    unsigned long long untouchable_remove_calls{0};   // packet p16_lua_natives
    unsigned long long truly_dead_ptr_nulled{0};      // packet p16_truly_dead
    unsigned long long untouchable_cleared{0};
    unsigned long long untouchable_is_calls{0};
    unsigned long long untouchable_is_true{0};
    unsigned long long formation_leader_found{0};
    unsigned long long formation_leader_other{0};   // the leader is not the argument
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
    unsigned long long generate_object_respawned{0};   // packet cc9_lua55_genobj
    unsigned long long generate_object_every_call{0};  // packet p16_lua_natives
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
    // Packet cc9_lua49_checkpoint (kMissionCheckpointBound in the .cpp):
    // SaveCheckpoint 008ACB70 (save = true) and LoadCheckpoint 008ACA30 on
    // this state's BSP_Chk_Save. Returns the binding's result count.
    int mission_checkpoint_binding(lua_State* state, bool save);

    // Milestone 2i: `VehicleClass[index]`, from the table the autoload folder
    // of 00886900 already ran. `index` is the id the scene's
    // `Type = E ShipClasses : <symbol>` resolved to, which is the same number
    // the installed table indexes its rows by.
    GameVehicleClassRow read_vehicle_class_row(int index);
    // cc15-vcp1 (kVehicleClassPreloadCc15Bound): the scene database's
    // 0046EC77 resolve, which marks the row the way 00964790's miss path does.
    // `site` names the image caller this mark stands in for; the census
    // reports it.
    void mark_scene_vehicle_class_row_00964790(int class_index, const char* site) {
        mark_vehicle_class_row_00964790(class_index, site);
    }
    // cc15-vcp1: what 0095CA70 at `site` (004E0A9B or 0045F6C1) would nil,
    // the VehicleClass rows whose Got is not true. Later reads of those rows
    // through the read_vehicle_class_* readers are counted for the summary
    // line. cc15-vcp3: with `purge`, the rows are nil'd as 0095CA70 does.
    void census_vehicle_class_purge_0095ca70(const char* site, bool purge = false);

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
    // `VehicleClass[index][key]` as a boolean; `fallback` unless the field is a Lua
    // boolean (BSP_LuaReference_GetBooleanOrDefault, e.g. 00749210's CapturePost).
    bool read_vehicle_class_boolean(int index, const char* key, bool fallback);

    // `VehicleClass[index][key][nested_key]` as a number; `fallback` when any
    // level is absent or the leaf is not a number. Packet cc9_buoyancy_elements
    // reads `Hull.WaterLineRatio` (descriptor+71Ch, stored at 00832D9A) with it,
    // the float twin of read_vehicle_class_integer's nested form.
    float read_vehicle_class_nested_number(int index, const char* key,
        const char* nested_key, float fallback);

    // `VehicleClass[index].PartAnims[key]`, the three-element record 007D381A /
    // 007D3990 / 007D3B06 fold into class+5C8h / +5D8h / +5E8h. True when the
    // sub-table exists (the record's present flag, e.g. class+5E4h for Wings).
    // `first`/`second` are elements [1]/[2] (0.0f when not numbers); `third` is
    // element [3] and `has_third` says whether it was a number, in which case it
    // replaces |[1] - [2]| as the duration. Packet cc9_wing_fold (cc9-physics5).
    bool read_vehicle_class_part_anim(int index, const char* key, float& first,
        float& second, float& third, bool& has_third);

    // `Bullets[index][key]` from the live Lua state - the bullet class table that
    // Scripts/datatables/autoload/bulletclasses.lua publishes. Used for the
    // fields the flattened per-platform BSPGun table does not carry, notably
    // "FlyTime" and "WaterTravelSpeed". docs/TORPEDO_CATEGORY_ADMISSION.md.
    float read_bullet_class_number(int index, const char* key, float fallback);

    // `Bullets[index][key][nested_key]`: the Blast sub-table's numbers, which a
    // dotted key does not reach through read_bullet_class_number. Packet
    // cc9_lua53_kamikaze_impact (the plane crash class's blast, 007C0910).
    float read_bullet_class_nested_number(int index, const char* key,
        const char* nested_key, float fallback);

    // `Bullets[index][key]` as a string; "" when absent. Used for `Type`.
    std::string read_bullet_class_string(int index, const char* key);

    // `DeviceClass[index][key]` as a string; "" when absent or not a string.
    // Used for the device row's `Mesh` model path. docs/GUN_BARREL_COUNT.md.
    std::string read_device_class_string(int index, const char* key);

    // `DeviceClass[index][key]` as a number; `fallback` when absent. Packet
    // cc9_plane_level_bomb_task reads a bomb platform's `RepeatTime` (desc+E0h).
    float read_device_class_number(int index, const char* key, float fallback);
    // Packet s54_seat_lead: a boolean key (00B662F0 with the default), for
    // "AddsSpeed" = descriptor+95h (00732BE9..00732C0E).
    bool read_device_class_bool(int index, const char* key, bool fallback);

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
    // Packet cc9_get_closest_border_zone. The map bounds 004D5EDE selected (the
    // avoid-zone load's inputs); the host builds world+7134h's records from them.
    // SUBSTITUTION, labelled: the image builds them inside 004E6C00's Map read;
    // the host takes them at the avoid-zone load step, before any script runs.
    void set_world_border_zones(const bsp::WorldMapBounds& bounds);
    // Packet cc9_squadron_land_task: world+7134h for 007F16D0's retreat arm,
    // null until set_world_border_zones ran.
    const bsp::BorderZoneSet* world_border_zones() const noexcept {
        return border_zones_loaded_ ? &border_zones_ : nullptr;
    }
    // Packet cc9_plane_retreat_task: world+711Ch..+7130h, the box 0071C4F0,
    // 0059C9B0 and the retreat moveto tick 009C9310 read; null until
    // set_world_border_zones ran.
    const bsp::WorldMapBounds* world_border_bounds() const noexcept {
        return border_zones_loaded_ ? &border_bounds_ : nullptr;
    }
    // Packet cc13-fix3, under kLuaExitZoneListenersBound: a producer's 00982120 call.
    // `unit` is the units-host index of the entity after vtable[140h] (a ship is itself,
    // a plane is its squadron); the event is dispatched on the next mission frame. A no-op
    // while the switch is false.
    void note_exit_zone_00982120(std::size_t unit, bool exited);
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
    // Packet cc9_carrier_elevator: a number two tables deep, such as
    // ShipGlobals.MotherShip.ElevatorSpeed (shipglobals.lua, which 00B67800 runs;
    // the settings loader stores it at +4D8h, 0083FB4C). False when absent.
    bool read_global_nested_number(const char* table, const char* sub, const char* key,
        float& out);
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
    // Packet cc9_ships7_entry_points: ShipGlobals.SubAttack.SubmarineLostTime as
    // 0083B5E0 loads it into settings+4D4h (0083F779..0083F7A1), read live off
    // the loaded ShipGlobals table; 0 when absent, as lua_tonumber gives the image.
    float sub_attack_submarine_lost_time_04d4() const;

    // Packet cc9_unit_instance_step11. The EngineSoundSmoothRate of the four
    // engine-sound records 0083B5E0 fills at settings+5BCh + i*24h + 8h
    // (008405D5), off `ShipGlobals["Sounds"][name]` for name = Ship, TBoat,
    // Submarine, Plane (jump table 00842954). An absent key keeps the 0.2f
    // default 00B66330 is handed (00CE54A0). False when `Sounds` or a record
    // table is missing; `out` then keeps what it held.
    bool read_engine_sound_smooth_rates_0083b5e0(float (&out)[4]);
    // 0083FE53..0083FECD of 0083B5E0: ShipGlobals["Physics"], TorpedoForce (key 00D0A79C)
    // with the 1.0f default (FLD1 at 0083FE6C) and TorpedoForcePower (00D0A788) with the
    // 2.0f default ([00CE3958]), both through 00B66330, stored at settings+590h (0083FE81)
    // and +594h (0083FECD). A key that is not a number keeps its default. False when
    // ShipGlobals or ShipGlobals.Physics is not a table. This installation's
    // Scripts/datatables/shipglobals.lua (mtime 2024-07-13) sets TorpedoForce = -75 and
    // has no TorpedoForcePower. Inert: no caller yet (for cc9-gunnery9).
    bool read_physics_torpedo_force_0083b5e0(float& force, float& power);

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
    // Packet cc9_avoid_zone_draft_bodies (docs/GUNNERY_OPEN_ITEMS.md 109): the nine depths
    // 00424DDF..004253FF reads, ShipGlobals.AvoidZoneDepthsSingle/Multi[class][1] (00837DE0
    // selects) for BattleShip, MotherShip, Destroyer, TBoat, LargeLandingShip, CargoShip,
    // LightCruiser, HeavyCruiser, Submarine in that order, converted as 0083B5E0 stores
    // them (00B67720 index 1, then 00B66290). Leaves output unchanged on failure.
    bool read_avoid_zone_draft_depths(std::int32_t session_mode,
        std::array<std::int32_t, 9>& out, std::string& error);

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

    // Packet cc13-fix20: the mission result object that the Scoring_SetMissionCompleted
    // (008B8AD0) and PlayBinkMovie (0089A480) rows reach, when their switches
    // in game_hosts_mission_result.hpp are on. The frame host owns it.
    void attach_mission_result(GameMissionResultHost* result) noexcept { mission_result_ = result; }
    GameMissionResultHost* mission_result() const noexcept { return mission_result_; }

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
    // DIAGNOSTIC: logs `mission objective: <text>` for each Objectives_Add.
    void note_objective_text(const std::string& objective, const std::string& text);

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
    void dispatch_entity_killed_listeners_00986480();   // packet cc13-ver5
    void dispatch_exit_zone_listeners_00982120();       // packet cc13-fix3
    void dispatch_recon_listeners_00980e50();
    // 00980E50's per-change body: every matching `recon` callback with (unit, old, new, party).
    void fire_recon_listeners_00980e50(std::size_t unit, int old, int level, int party);
    void publish_recon_slot_tables_00806b10();
    void dispatch_hit_listeners_00988510();
    // Packet cc9_set_invincible_native. 00897A50 SetInvincible, always bound: the
    // gunnery host's setter stores unit+150h and every reader of it is gated on
    // kUnitInvincibilityFloorBound (src/game_hosts_gunnery.cpp), so this is inert
    // until that switch is on. docs/LUA_BINDING_MISSION.md.
    int run_set_invincible_00897a50(lua_State* state, int argument_count);
    int run_override_hp_008c1930(lua_State* state, int argument_count);
    // Packet cc9_forced_recon_level, under kForcedReconLevelBound.
    int run_set_forced_recon_level_008aa8f0(lua_State* state, int argument_count);
    // Packet cc9_lua_add_damage, under kLuaAddDamageBound.
    int run_add_damage_0088e000(lua_State* state, int argument_count);
    // Packet cc9_lua_aa_enable, under kLuaAAEnableBound.
    int run_aa_enable_0089c740(lua_State* state, int argument_count);
    // Packet cc9_lua_set_ship_speed, under kLuaSetShipSpeedBound.
    int run_set_ship_speed_00890d30(lua_State* state, int argument_count);
    // Packet cc13_natives1, under kLuaSetFailureBound.
    int run_set_failure_0088eff0(lua_State* state, int argument_count);
    int run_get_failure_0088f1d0(lua_State* state, int argument_count);
    // Packet cc13-fix11, under kLuaSetDeadMeatBound.
    int run_set_dead_meat_008ac7b0(lua_State* state, int argument_count);
    // Packet cc13-fix12, under kLuaIsUnitSelectableBound / kLuaGetEntitySpeedBound.
    int run_is_unit_selectable_008ab580(lua_State* state, int argument_count);
    int run_get_entity_speed_008c0820(lua_State* state, int argument_count);
    // Packet cc13-fix18, under kLuaClearForcedReconLevelBound / kLuaSetCrewLevelBound.
    int run_clear_forced_recon_level_008aab90(lua_State* state, int argument_count);
    int run_set_crew_level_00894f20(lua_State* state, int argument_count);
    int run_set_device_reload_time_mul_008c14b0(lua_State* state, int argument_count);
    // Packet cc13-smbind: BannSupportmanager / PermitSupportmanager / SupportManagerEnable.
    int run_support_manager_set_banned_0088c750_0088c8e0(bool banned);
    int run_support_manager_enable_0088cbb0(lua_State* state, int argument_count);
    // The 00F8AB6A ban byte, for the 00A25B90 SpawnGroup gate (00A25BAB) once that spawn
    // is bound; nothing calls it yet.
    std::uint8_t support_manager_banned_00f8ab6a() const noexcept {
        return support_manager_banned_00f8ab6a_;
    }
    // Packet cc13-fix10, under kLuaProximityTriggersBound.
    int run_add_proximity_trigger_008c9730(lua_State* state, int argument_count);
    int run_remove_trigger_008c5fd0(lua_State* state, int argument_count);
    // Wave 2's sub-list walk (00875B90 -> 008759B0) over the proximity triggers;
    // the mission frame calls it once per fixed step, after the motion step.
    void run_proximity_triggers_008759b0(float step);
    // Packet cc13-fix19, under kLuaSquadOrdnanceDroppedTriggerBound.
    int run_add_squad_ordnance_dropped_trigger_008c9a10(lua_State* state, int argument_count);
    // Packet cc13-fix19, under kLuaProximityTriggersBound: a Kill of unit `unit_index`
    // reached the kill list (00926D90); its pass-K observer notice (00925C40 inlined
    // at 009274DE) is queued for target_gone_drain_009273a0.
    void target_gone_00925c40(std::size_t unit_index);
    // Row 15 (009273A0) of the fixed-step fan-out: each queued unit's notice runs
    // 0074FFE0 on every trigger whose entity target it is.
    void target_gone_drain_009273a0();
    // Packet cc13-dead, under kLandConvoyDeadWired (include/bsp/land_convoy_dead_cc13.hpp):
    // 00929800 on the LandConvoy marker named `convoy`, reached through 0077D270 ->
    // 00929B60 when the convoy's destroy is flushed: `Dead` = true and `KillReason` =
    // 00E0CF04[cause] on its self table. False when no convoy marker of that name
    // has an own self table (00929800 returns when +178h is clear).
    bool publish_land_convoy_dead_00929800(const std::string& convoy, int cause);
    // Packet cc9_unit_get_attack_target, under kLuaUnitGetAttackTargetBound.
    int run_unit_get_attack_target_008a6de0(lua_State* state, int argument_count);
    // Packet cc9_squadron_set_speed, under kLuaSquadronSetSpeedBound.
    int run_squadron_set_speed_0089f780(lua_State* state, int argument_count);
    // Packet cc9_is_class_changed, under kLuaIsClassChangedBound.
    int run_is_class_changed_008cc4b0(lua_State* state, int argument_count);
    // Packet cc9_set_submarine_depth_level, under kLuaSetSubmarineDepthLevelBound.
    int run_set_submarine_depth_level_00893f40(lua_State* state, int argument_count);
    // Packet cc13-luawire, under kLuaHasFiredBound / kLuaAddWaterLoadBound /
    // kLuaBreakShipBound (bsp/cc13_lua_natives_wire.hpp).
    int run_has_fired_008be7e0(lua_State* state, int argument_count);
    int run_add_water_load_00893380(lua_State* state, int argument_count);
    int run_break_ship_008914f0(lua_State* state, int argument_count);
    int run_unit_set_player_commands_enabled_008a6c30(lua_State* state, int argument_count);
    // Part 7b, under kLuaSetVisibilityBound / kLuaSquadronEnableVehicleAvoidanceBound /
    // kLuaSquadronSetBaseUnsupportedBound / kLuaSetCheatTurboBound.
    int run_set_visibility_008a13d0(lua_State* state, int argument_count);
    int run_squadron_enable_vehicle_avoidance_008a0000(lua_State* state, int argument_count);
    int run_squadron_set_base_unsupported_0089faf0(lua_State* state, int argument_count);
    int run_set_cheat_turbo_008a62a0(lua_State* state, int argument_count);
    // Packet cc9_lua_submarine_on_surface, under kLuaGetSubmarineOnSurfaceBound.
    int run_get_submarine_on_surface_008942c0(lua_State* state, int argument_count);
    // Packet cc13_natives1, under kLuaTrulyDeadBound.
    int run_truly_dead_00899500(lua_State* state, int argument_count);
    // Packet p16_truly_dead: 00928C80's Lua half for units-host index `unit_index`.
    void on_entity_killed_00928c80(std::size_t unit_index);
    // Packet cc9_set_air_base_slot_count, under kLuaSetAirBaseSlotCountBound.
    int run_set_air_base_slot_count_008963e0(lua_State* state, int argument_count);
    // Packet cc9_lua_add_air_base_stock, under kLuaAddAirBaseStockBound.
    int run_add_air_base_stock_00896a90(lua_State* state, int argument_count);
    // Packet cc9_lua_add_shipyard_stock, under kLuaAddShipyardStockBound.
    int run_add_shipyard_stock_00896cc0(lua_State* state, int argument_count);
    // Packet cc9_shipyard_create_unit: the unit 00844FC0 builds, registered with
    // bsp::shipyard_set_create_unit. Answers the new unit's index or kShipyardNone.
    std::size_t create_shipyard_unit_00844fc0(const ::bsp::ShipyardBuildRequest& request);
    unsigned shipyard_units_serial_{0};
    // Packet cc9_device_reload_enabled, under kLuaDeviceReloadEnabledBound.
    int run_set_device_reload_enabled_008c1350(lua_State* state, int argument_count);
    // Packet cc13_natives1, under kLuaSquadronForceGunnerModeBound: the byte
    // entity+379h per units-host slot, and its reader for the HUD root's
    // controlled_unit_flag_379 (0064A1E0). False for an unset or unknown slot.
    int run_squadron_force_gunner_mode_0089f950(lua_State* state, int argument_count);
    bool squadron_force_gunner_mode_379(std::size_t unit_index) const;
    std::vector<std::uint8_t> force_gunner_379_;
    // Packet cc9_lua_formation_query, under kLuaFormationQueryBound.
    int run_is_in_formation_008996a0(lua_State* state, int argument_count);
    int run_get_formation_leader_00899af0(lua_State* state, int argument_count);
    int run_get_last_catapulted_00892860(lua_State* state, int argument_count);
    int run_get_num_catapulted_00892aa0(lua_State* state, int argument_count);
    // Packet p17_catapult_adapter, under bsp::kShipCatapultLaunchBound.
    int run_ship_use_catapult_00891d50(lua_State* state, int argument_count);
    int run_set_catapult_stock_00892c30(lua_State* state, int argument_count);
    // The ship's class Catapult block and its live-pruned record (nullptr when
    // `unit` is not a units-host slot).
    bsp::ShipCatapultRecord* catapult_record(std::size_t unit, bsp::ShipCatapultClass& cls);
    int run_get_catapult_stock_00892de0(lua_State* state, int argument_count);
    int run_add_untouchable_unit_008ac140(lua_State* state, int argument_count);
    int run_remove_untouchable_unit_008ac2b0(lua_State* state, int argument_count);
    int run_is_unit_untouchable_008ac420(lua_State* state, int argument_count);
    int run_leave_formation_00899eb0(lua_State* state, int argument_count);
    // Packet cc13_natives1, under kLuaSquadronSetReleaseAltBound.
    int run_squadron_set_release_alt_008a24e0(lua_State* state, int argument_count);
    // Packet cc13-fix4, under kLuaSquadronLandAndKillBound.
    int run_squadron_land_and_kill_008a20e0(lua_State* state, int argument_count);
    // Packet cc13-fix5, under kLuaRemoveAllAirBasePlanesBound.
    int run_remove_all_air_base_planes_008975b0(lua_State* state, int argument_count);
    // Packet cc13-fix5, under kLuaStartLanding2Bound.
    int run_start_landing2_0089ac30(lua_State* state, int argument_count);
    // Packet cc13-fix5, under kLuaGetSquadronPlaneBound.
    int run_get_squadron_plane_0089cfe0(lua_State* state, int argument_count);
    // Packet cc13-fix5, under kLuaDisbandFormationBound.
    int run_disband_formation_0089a020(lua_State* state, int argument_count);
    // Packet cc13-fix5: the LandConvoy natives (00895570, 008956E0, 00895850), each
    // under its own switch. `address` selects the store.
    int run_land_convoy_native(lua_State* state, int argument_count, std::uint32_t address);
    // Packet cc13-fix5, under kLuaLaunchAirBaseSlotBound.
    int run_launch_air_base_slot_00896750(lua_State* state, int argument_count);
    // Packet cc9_squadron_travel_alt, under kSquadronTravelAltBound.
    int run_squadron_set_travel_alt_0089f550(lua_State* state, int argument_count);
    // Packet cc9_squadron_attack_alt, under kSquadronAttackAltBound.
    int run_squadron_set_attack_alt_008a22b0(lua_State* state, int argument_count);
    // Packet cc9_get_closest_border_zone, under kLuaClosestBorderZoneBound.
    int run_get_closest_border_zone_008aecd0(lua_State* state, int argument_count);
    // Packet cc13_natives1, under kLuaIsInBorderZoneBound.
    int run_is_in_border_zone_008af020(lua_State* state, int argument_count);
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
    // Packet cc9_lua54_prcp01: 008B2320/008B24A0 and 008B25F0/008B2770.
    int run_simplified_multiplier(lua_State* state, int argument_count,
                                  std::uint32_t address);
    float simplified_recon_74_{1.0f};
    float simplified_sonar_78_{1.0f};
    // Read-only views of [game+21C4h]+74h / +78h for the gunnery host's recon pass
    // (packet cc9_g40_recon_mult).
    float simplified_recon_multiplier() const { return simplified_recon_74_; }
    float simplified_sonar_multiplier() const { return simplified_sonar_78_; }
    int run_launch_squadron_0089e3c0(lua_State* state, int argument_count);
    // 00944FD0 GenerateObject. Instantiates one of the entities the scene pass
    // held back at 0046D3C5, by name, and pushes its `thisTable` slot the way the
    // entity-returning tail 0089903C does. docs/LUA_GENERATE_OBJECT_HOST.md.
    int run_generate_object_00944fd0(lua_State* state, int argument_count);
    // Packet cc9_lua46_generate_land_convoy: a held-back LandConvoy (class 1Ah)
    // becomes a scene marker plus its 00743450 members, as a load-time convoy.
    int generate_land_convoy_00743450(lua_State* state, SceneSpawnPoolEntry& entry,
        const GameSceneEntityRecord& held, bool placed);
    // Marker ids for GenerateObject's convoys; the load-time markers count up from
    // 50000 (kSceneMarkerIdBase) and script entities start at 100000.
    static constexpr int kGeneratedSceneMarkerIdBase = 90000;
    int generated_scene_markers_{0};

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
    // Packet cc9_lua52_kamikaze_wiring: 006FEC00 over the released rounds.
    void run_dummy_kamikaze_rounds_006fec00(float step_seconds);

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
    // Packet cc9_spawn_new_shipyard. 008F8680's +14h arm: the world matrix at
    // entity+CCh of the entity numbered `entity_id` (a unit's pose, or a scene
    // marker's authored frame). False when the id names neither.
    bool spawn_ref_entity_frame_008f8680(std::int32_t entity_id, float frame[16]) const;
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
    GameMissionResultHost* mission_result_{nullptr};  // packet cc13-fix20
    // Packet cc13-smbind. The byte global 00F8AB6A that 00A38DA0 stores (1 banned, 0
    // permitted), and the panel list at ([00E198C4]+BCh)+1F8h that SupportManagerEnable
    // edits, held as entity ids (unit index + 1). Nothing in this process reads either.
    // The image resets 00F8AB6A to 0 in InitPartySlots (00A32F77, in 00A32DF0) at each
    // mission; this host keeps the byte and the list for the GameMissionLuaHost lifetime.
    std::uint8_t support_manager_banned_00f8ab6a_{0};
    std::vector<int> support_panel_disabled_list_1f8_;
    // Packet cc9_lua_listeners: the (channel, id) registry of 00980C10.
    struct ListenerEntry {
        std::string channel;
        std::string id;
        std::string callback;                 // subscription+4h
        std::vector<int> entity_ids;          // the `entity` set (+0Ch)
        bool attacker_filters_set{false};     // +1Ch or +2Ch not empty
        // `kill` under kLuaKillAttackerBound: lastAttacker (+1Ch, 009721C0) and
        // lastAttackerPlayerIndex (+2Ch, 009722D0). A non-empty set whose members
        // carry no entity id keeps last_attacker_set and matches nothing.
        bool last_attacker_set{false};
        std::vector<int> last_attacker_ids;
        std::vector<int> last_attacker_player_indices;
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
        // Packet cc9_hit_attacker_player_index: attackerPlayerIndex (+4Ch).
        std::vector<int> attacker_player_indices;
        // `entityKilled` (00972F00) under kLuaEntityKilledListenersBound: entity +0Ch
        // (009721C0, kept in entity_ids) and killReason +1Ch (00970FF0).
        std::vector<std::string> kill_reasons;
        // `exitzone` (009726D0) under kLuaExitZoneListenersBound: entity +0Ch (in
        // entity_ids) and exited +1Ch, a boolean set held as 0/1.
        std::vector<int> exited_values;
    };
    std::vector<ListenerEntry> listeners_;
    // Packet cc9_get_closest_border_zone: world+711Ch..+7130h and world+7134h.
    bool border_zones_loaded_{false};
    bsp::WorldMapBounds border_bounds_{};
    bsp::BorderZoneSet border_zones_{};
    // Packet cc9_hit_rate_limit: the map at 00988510's this+168h, (victim,
    // attacking unit) -> the clock at which the next hit on the pair is evaluated.
    std::map<std::pair<std::size_t, std::size_t>, float> hit_rate_limit_;
    std::vector<bool> listener_death_seen_;
    std::vector<bool> listener_entity_killed_seen_;   // packet cc13-ver5
    // Packet cc13-fix3: cause-4 deaths already taken, each squadron's first exit clock,
    // and the producer events waiting for the next frame (entity unit, exited).
    std::vector<bool> listener_exit_zone_seen_;
    std::map<std::size_t, float> exit_zone_first_clock_;
    std::vector<std::pair<std::size_t, bool>> exit_zone_pending_;
    // Packet cc13-fix10: one 0074FEE0 trigger, the fields 00750000, 0074FDD0 and
    // 0074FBB0 read, under the image offsets.
    struct ProximityTrigger {
        // Packet cc13-fix19: the record's vtable. 00D00580 = proximity (check
        // 00750000), 00D005C0 = squad ordnance dropped (check 007504F0); both
        // share the Lua trigger base fields below (+10h..+28h).
        enum class Kind : std::uint8_t { Proximity, SquadOrdnanceDropped };
        Kind kind{Kind::Proximity};
        std::string name_1c;
        bool named_24{false};
        std::string callback_28;
        std::uint8_t target_kind_44{0};        // descriptor +0: 1 = entity
        bool target_point_valid_45{false};     // descriptor +1
        int target_id{0};                      // the entity's ID (descriptor +4)
        float target_point[3]{0.0f, 0.0f, 0.0f};  // descriptor +8
        float radius_sq_5c{0.0f};
        bool expired_10{false};
        bool enabled_11{true};
    };
    // Each watched unit's sub-list, in 00876020's order (all priority 2: appended).
    std::map<std::size_t, std::list<ProximityTrigger>> proximity_triggers_;
    void check_proximity_trigger_00750000(std::size_t unit, ProximityTrigger& trigger);
    // Packet cc13-fix19.
    void check_squad_ordnance_trigger_007504f0(std::size_t unit, ProximityTrigger& trigger);
    void fire_lua_trigger_0074fdd0(std::size_t unit, ProximityTrigger& trigger);
    std::vector<std::size_t> target_gone_pending_;   // the kill list's notices, FIFO
    std::vector<bool> target_gone_notified_;         // +5Eh: pass K ran (00926D90 idempotence)
    // Packet cc13-fix19b: a squadron target whose registry record has held a member
    // (+3CCh was non-zero), so a later empty record reads as 007F3970's Kill.
    std::vector<bool> squadron_target_resolved_;
    // Packet cc13-fix19c: target_gone_00925c40 without its fused-squadron-slot filter.
    void target_gone_mark_(std::size_t unit_index);
    // Packet cc13-ver8: the cause this host's Lua Kill (008AC5C0) passed, per unit; the
    // gunnery funnel does not keep it.
    std::map<std::size_t, int> lua_kill_cause_;
    // Packet cc9_lua_recon_listeners: the last level per unit and party, and the
    // recon pass generation they were taken at.
    std::vector<int> recon_listener_levels_;
    unsigned long long recon_listener_generation_{0};
    // Packet cc9_recon_publication: the pass generation last published, and
    // per party the (relation, category, id) triples last written.
    unsigned long long recon_publish_generation_{0};
    std::array<std::vector<std::uint32_t>, 3> recon_published_;
    // squadron+354h per squadron unit, frozen the first time a member resolves.
    std::map<std::size_t, int> recon_squadron_category_;
public:
    void attach_units_hooks(GameUnitsHost* units) noexcept { units_hooks_ = units; }
    void attach_extra_fixed_step(GameExtraFixedStepRunner* runner) noexcept {
        extra_fixed_step_ = runner;
    }
    // Packet cc13-baystale: bsp::cc13_release_timer::bomb_rack_bay_doors's
    // per-class flags. The host is rebuilt at every mission load
    // (game_hosts_mission.cpp), so this cache lives exactly one mission.
    std::map<int, std::vector<bool>>& bomb_rack_bay_doors_cache() noexcept {
        return bomb_rack_bay_doors_cache_;
    }
private:
    std::map<int, std::vector<bool>> bomb_rack_bay_doors_cache_;
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
        // Packet cc9_carrier_launch_skill: an air-ops squadron's bag `Skill`,
        // which 006C5050 fills from owner->vtable[12Ch]() at 006C5101..006C5113
        // (the carrier's live unit+390h at launch). The squadron's pass B,
        // 007F1FE0, hands it to its own vtable[128h] (007F226E), which 007ECF80
        // fans out to the wing. -1 for any other node.
        int launch_skill{-1};
    };
    friend class GameMissionLuaInitAllBinding;
    std::deque<PendingEntity> pending_entities_;  // 00F899D0, count 00F899D4
    // Packet cc9_pending_list_dedup: the node of `entity_id`, or nullptr.
    PendingEntity* find_pending(int entity_id);
    // Ids attached by the load-time attach (attach_scene_entities_00928a00).
    std::set<int> load_attached_;
    std::set<int> init_all_attached_;
    // Packet s54_class_id (kVehicleClassRowIdBound): the class rows 00964790's
    // miss path has written `ID` / `Got` on, once each (its cache stands in).
    std::set<int> vehicle_class_rows_marked_;
    void mark_vehicle_class_row_00964790(int class_index, const char* site = nullptr);
    void mark_vehicle_class_device_bullets_00960230(int class_index);
    // cc15-vcp1: the rows the last census found unmarked, with the reads
    // of each that came after it.
    std::map<int, std::size_t> vehicle_class_would_nil_;
    // cc15-vcp2: the first marking site of each row (nullptr is a lazy mark).
    std::map<int, const char*> vehicle_class_mark_site_;
    std::size_t vehicle_class_censuses_{0};
    std::size_t vehicle_class_rows_would_nil_{0};
    std::size_t vehicle_class_later_reads_{0};
    void note_vehicle_class_read(int index);
    // cc15-vcp3, harness diagnostic: the purged rows, and the script reads of
    // those keys seen through an __index hook on VehicleClass. Also the Lua
    // errors the run logged (note_error).
    std::size_t vehicle_class_purged_{0};
    std::size_t vehicle_class_purge_failures_{0};
    std::size_t vehicle_class_script_nil_reads_{0};
    std::map<int, std::size_t> vehicle_class_script_nil_rows_;
    std::size_t lua_errors_total_{0};
    static int vehicle_class_nil_index_hook(lua_State* state);
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
    // Packet cc13-luawire (kLuaBreakShipBound): units whose controller halves
    // +370h / +374h a BreakShip's 00935D30 cleared.
    std::set<std::size_t> break_ship_cleared_;
    // Packet cc13-luawire: director+1CCh, the player-command mask 0071C1E0 stores from
    // UnitSetPlayerCommandsEnabled's message 5Ah, keyed by the director's units-host
    // slot. No host code reads it: the image's readers are the HUD (0071C060's callers)
    // and the network snapshot 00721280.
    std::map<std::size_t, std::int32_t> director_player_commands_1cc_;
    float spawn_attempt_delay_{0.0f};
    bool spawn_attempt_delay_read_{false};
    unsigned spawn_requeue_logged_{0};
    GameMissionLuaSummary summary_;
};

// Packet cc9_ai_plane_loadout_arm (docs/SQUADRON_LAND_TASK.md 5df). 00A08460's
// loadout arm reads 009552E0's loadout record, which this process keeps only as
// the authored table, and the AI coordinator holds no Lua host, so the live
// GameMissionLuaHost answers through these process-wide reads (null host: no
// list). VehicleClass[class_id].Equipments[equipment] as 00961F57 stores it:
// per entry Ammo (+8h), ReloadTime (+0Ch) and Platform (+4h), the device class
// DeviceClass[Platform] answering 25h when its Type is BombPlatform or
// MultiBombPlatform (00442C70 / 00442D00), and that class's first bullet
// (00731040) from Bullets[DeviceClass[...].Bullet[1].Bullet]. Cached per pair.
bool game_ai_plane_loadout(int class_id, int equipment,
                           std::vector<bsp::AiPlaneLoadoutEntryFacts>& out);
// The size of VehicleClass[class_id].Equipments (class +128h, read by 00951F10).
int game_ai_plane_equipment_count(int class_id);

}  // namespace bsp::game
