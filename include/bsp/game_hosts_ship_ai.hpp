#pragma once
// bsp_game.exe milestone 2n: the ship AI controller per unit, and the weapon
// director's own automatic target selector.
//
// Addresses: 009f50e0 (the AI controller's frame sequence, sixteen steps),
// 009f3dd0 (which state the director's current command asks for), 009f3d00
// (the state the command object selects) with 00779aa0, the nine state objects
// of the family at 00d21598 and their vtable slots +0ch (the step), +24h (the
// command object) and +28h (the re-plan interval), 009ed6b0 (the direct-control
// arm that turns the desired values into a heading target and two distances),
// 009f4d10 with 00811960 (the publish into the unit's 84-byte AI order slot at
// unit+0aech - 84*[unit+0b40h]), 00825f2c..00825f7c with 00811d10 (the motion
// head's promotion of that slot) and 009f5da0 with 009f5610, 009f5d30, 009f5b70,
// 009f52f0 and 00835860 (the director's once-a-second automatic target think).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::ShipAiControllerHost, bsp::ShipAiSyncHost,
// bsp::ShipAiSetterHost, bsp::ShipAiDirectControlHost, bsp::ShipAiPublishHost or
// bsp::BotFireTargetHost, satisfied either by a reconstruction already on main
// or by the explicit unimplemented policy in GameHostLog.
//
// Runtime ownership remains a C++ representation. Ship navigation now exists
// only for the actual ship-family descriptor kinds: their virtual+210 reaches
//00810DD0/009F3F20. The recovered009F50E0 sequence runs before the represented
// motion pass. Its complete009E0270 pre-step borrows actual depth, extents,
// cached pose and persistent byte/profile storage. Generic command/director
// updates remain separate; individual adapters record unresolved state.
// One existing runtime boundary is:
//   - the party list 009f5d30 scans. Its source is the recon slot 008053c0
//     returns for the owner's party and the intrusive list at slot+0de8h, which
//     nothing in this process fills, so the executable hands the recovered scan
//     the created instances of the opposing party and records 008053c0. This is
//     the same substitution milestone 2i makes for walk 0 of 004c3cb0.
//
// Evidence: docs/SHIP_AI_STATES.md, docs/UNIT_AUTOPILOT_PAIR.md,
// docs/BOT_FIRE_TARGET.md, docs/CRUISE_COMMAND.md, docs/UNIT_RUDDER_CURVE.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/bot_fire_target.hpp"
#include "bsp/ship_ai_states.hpp"
#include "bsp/unit_autopilot_pair.hpp"

namespace bsp { class SessionParticipantPools; }

namespace bsp::game {

// Packet cc9_prcp03_phase_progress (docs/GAME_SHIP_NAVIGATION_BINDING.md,
// "A command aimed at a scene marker"). 00521EA0 resolves a command's uint16
// object id through the two handle tables 00F89A54 / 00F89AA8, which hold every
// entity, a NavPoint included; 009E2FB0 then latches it and 009DBCC0 places the
// goal at the object's matrix times the zero offset, the marker's position.
// This host's 00521EA0 answers units only, so a `moveto` aimed at a NavPoint
// (PRCP03 Aylwin -> CarrierPoint, USN01 Convoy1 -> ConvoyGoTo) latched nothing
// and the goal fell to the zero triple: the world origin. True: an
// unresolved object id that names a scene marker supplies the marker's authored
// world position as the goal. LABELLED: the marker is handed over as a
// position, not a latched object, so raw_target_0b20 stays 0 (the image would
// hold the NavPoint there; 009F1491 filters it out as not IsKindOf(2)). The
// weapon director's arrival test (00836A6C) still does not resolve markers.
// ON by the pairs at 4df48ee59: PRCP03 (Aylwin to CarrierPoint, 2710 m short),
// USN01 (Convoy1 to ConvoyGoTo) and USN02 (DeRuyter to DRGoTo) move; USN04
// identical.
inline constexpr bool kShipAiMarkerTargetBound = true;

// Packet cc9_usn02_deruyter_fire (docs/USN02_DERUYTER_FIRE.md). The Lua native
// SetFireTarget (0089A8B0), which luaSetScriptTarget calls for every ship, takes
// the unit's director (vtable[114h]) and calls 00835860(target, force = 1). The
// kind 5Eh message's receiver 00836240 stores director+238h = target and
// +23Ch = force, and the automatic selector 009F5DA0 skips its own choice at
// 009F5EFD..009F5F21 while a target is held and +23Ch is set. True: the native
// is bound, the lock is held, and the selector keeps the scripted target.
// LABELLED: a held target that dies is released (the observer pair 00836240
// registers), and a Vector3 argument (a dummy target entity) is not modelled.
// False: the native stays an unimplemented record and nothing locks.
// ON by the pairs at 6c936d2d0: USN02 reaches phase 2 (DeRuyter below 15%);
// USN01, USN04 and JM06 identical.
inline constexpr bool kScriptFireTargetBound = true;

// Packet cc9_weapon_director_fire_target (docs/GUNNERY_OPEN_ITEMS.md section 14). True:
// every 00835860 the host reproduces goes through 00836240's gate and store
// (store_fire_target_00836240): the AutoTarget tick's call unforced (009F5F1E PUSH 0),
// SetCommand's forced (00835924 PUSH 1), BeginCurrentCommand's with its own force
// (00835E07). The commands host queues its two (take_fire_target_requests) and this
// host applies them at the session pump, as the routed 5Eh message arrives. LABELLED:
// the entity-command arm (00816E30) and the order appliers stay records; the arm is
// not reached on the reference runs. False: the AutoTarget tick writes the target
// directly and the command paths store nothing, as before.
// ON by the pairs of 2026-09-28 (docs/GUNNERY_OPEN_ITEMS.md section 15).
inline constexpr bool kWeaponDirectorFireTargetBound = true;

// Packet cc9_torpedo_threat_first_node (docs/GUNNERY_OPEN_ITEMS.md sections 22-23).
// 00814420 (ship vtable[1D4h], body 00814420-00814492) walks the world torpedo
// list [world+220h] (count +21Ch) but never advances its node: 00814450 reads
// ESI = [EBX+8] on every pass and only the count at [ESP+10h] moves (00814390
// advances at 00814402 MOV EDI,[EDI+4]). 00484540 appends at the tail, so the
// head is the oldest registered torpedo. The answer is the list count when that
// torpedo is live ([+310h] vtable[38h]), not the target's own (+4F8h) and
// threatening (target->vtable[1D0h] 008173E0), else 0. False: every foreign
// threatening torpedo counted once, the host's stand-in.
// ON by the pairs of 2026-09-28 (docs/GUNNERY_OPEN_ITEMS.md section 24).
inline constexpr bool kForeignTorpedoThreatHeadBound = true;

// Packet cc9_torpedo_threat_first_node, section 17's last rank-2 term. 009F5E59
// 0071D6D0(director, attackmove 00E08F78, 00465080(current target)) when the
// director's target is locked: true keeps the target and skips the scan and the
// retained-score reset (009F5E62). Routed to the commands host's concrete body.
// False: the stand-in answers false and the scan runs.
// ON by the pairs of 2026-09-28 (docs/GUNNERY_OPEN_ITEMS.md section 24).
inline constexpr bool kAutoTargetCommandAcceptBound = true;

// Packet cc9_big_landing_ship (docs/SHIP_AI_OPEN_ITEMS.md section 37). 00827F70
// (a class method): TorpedoBoat 0Eh is small; LandingShip 0Ch is small only
// when the byte at class+808h, BigLandingShip, is 0 (00827F95). The byte is
// VehicleClass[type].BigLandingShip, which the LandingShip leaf reads with
// exact-Boolean-or-false at 0074C630 and which also picks the leaf's tuning
// pair ("BigLandingShip true": scalar source 20h). True: the four callers
// this lane owns read it - the neighbour admission 009F0D82, the approach mode
// latch 009F1F76, the AI bullet accuracy group 009FE2D4.. and the capture
// weight 00A0360B. False: every landing ship is small, as before.
// ON by the pairs of 2026-09-29 (section 37): gameplay identical on JM08, IJN01
// and JM06; the admission reads it 1550 / 4588 times, the AI sites 0 / 200.
inline constexpr bool kShipAiBigLandingShipBound = true;

// Packet cc9_standoff_target_kind (docs/SHIP_AI_OPEN_ITEMS.md section 39). The
// standoff choice 009E6E80 asks the target [brain+0B20h] vtable[5Ch](8) at
// 009E6F01 and vtable[5Ch](1Ch) at 009E6F3A (mode 4) and 009E701C (mode 2), and
// 009E6F11 calls 00827F70 on [brain+0AACh], the unit's own class. True: the
// kind queries answer the target's kinds, 00827F70 answers as the mode latch
// does, and mode 2 reads the CommandBuilding's CaptureRange [target+7A0h]
// (006F2780, FIMUL at 009E706F, FILD at 009E7087), and mode 4 its LandingRange
// [target+7C4h] (006F285F, FILD at 009E6F4E; GameUnitsHost::
// command_building_landing_range_07c4, main a19a551ba). False: every kind
// answers false, as before.
// ON by the pairs of 2026-09-29 (section 39): gameplay identical on USN01 and
// JM16, zero reach on both. The mode-4 arm, deferred then, is bound since
// section 41 with the same zero reach.
inline constexpr bool kShipAiStandoffTargetKindBound = true;

class GameHostLog;
class GameUnitsHost;
class GameSceneContentsHost;
class GameMissionLuaHost;
class GameGunneryHost;

// 009F3D00's own dispatch, read from the image at 009F3D04..009F3DA0. Each row
// is one `CMP EAX,<command>` and the `LEA EDI,[ESI+<ai offset>]` it takes; the
// brain offset is the ai offset minus 58h, which is what docs/SHIP_AI_STATES.md
// tabulates. The two attack rows share one arm and are not in this table.
struct ShipAiCommandStateRow {
    std::uint32_t command_object;  // the singleton 009F3DD0 read from the director
    std::uint32_t ai_offset;       // LEA ESI+<this>
    const char* state;             // the literal 009F39C0 registered
    std::uint32_t compare_site;    // the CMP that selects it
};

// The rows in the order 009F3D00 tests them.
inline constexpr ShipAiCommandStateRow kShipAiCommandStates[] = {
    {0x00e08f68u, 0x0C5Cu, "movetopos",  0x009f3d04u},
    {0x00e08fa0u, 0x0C38u, "land",       0x009f3d1au},
    {0x00e08f88u, 0x0BD8u, "stop",       0x009f3d29u},
    {0x00e08f80u, 0x0C64u, "moveonpath", 0x009f3d38u},
    {0x00e08f60u, 0x0BE4u, "follow",     0x009f3d47u},
    {0x00e08f70u, 0x0BC8u, "cruise",     0x009f3d64u},
};
inline constexpr int kShipAiCommandStateCount =
    static_cast<int>(sizeof(kShipAiCommandStates) / sizeof(kShipAiCommandStates[0]));

// The two command objects that share the attack arm at 009F3D56..009F3D96.
inline constexpr std::uint32_t kShipAiArtilleryCommandObject = 0x00e08f10u;
inline constexpr std::uint32_t kShipAiAttackMoveCommandObject = 0x00e08f78u;
// 009F3DA0: every command object no row matches falls to the `stop` state.
inline constexpr std::uint32_t kShipAiDefaultStateAiOffset = 0x0BD8u;

// One unit's AI controller as this process holds it, for the run log and the
// report. Every value is what a recovered routine left behind.
struct GameShipAiRow {
    std::size_t unit_index{0};
    std::string unit;
    // 009F3DD0 / 009F3D00
    std::uint32_t command_object{0};
    std::string state;              // the state the command selected
    std::uint32_t state_step{0};    // that state's vtable +0Ch
    bool state_step_concrete{false};
    unsigned long long state_changes{0};
    // 009F50E0's own counters
    unsigned long long controller_steps{0};  // bodies that passed all three gates
    unsigned long long gated{0};             // steps one of the gates stopped
    unsigned long long replans{0};
    float replan_interval{0.0f};             // vtable +28h times 0.05f
    // the control block blk = brain+8h
    int steering_mode{0};
    float desired_throttle{0.0f};
    float desired_rudder{0.0f};
    float desired_heading{0.0f};
    int latched_direction{0};
    float heading_target{0.0f};     // blk+324h
    float distance_32c{0.0f};
    float distance_330{0.0f};
    bool distance_finite{true};
    // the 84-byte order slot the publish fills and the motion head promotes
    unsigned long long publishes{0};
    unsigned long long promotions{0};
    int slot_index{0};
    float slot_heading_44{0.0f};
    float slot_distance_40{0.0f};
    float slot_distance_48{0.0f};
    bool slot_valid{false};
    // Milestone 2o: the hop at the tail of 009F3F80, chain slot 16.
    unsigned long long ring_hops{0};        // bodies of 009F3F80 that reached 009F4B99
    unsigned long long ring_gated_3f5{0};   // steps the 009F3FF2 gate on blk+3F5h stopped
    unsigned long long ring_writes{0};      // 0080E170 / 0080E190 pairs written
    unsigned long long rudder_law_calls{0}; // 009F44F7, the 009F44E4 gate open
    unsigned long long rudder_deadbands{0}; // the 009F4BC6 arm that zeroes blk+1D4h
    unsigned long long live_pair_changes{0}; // steps on which 00813020 moved +148h/+14Ch
    float ring_slot_throttle{0.0f};  // what 0080E170 last received
    float ring_slot_rudder{0.0f};    // what 0080E190 last received
    float ring_live_throttle{0.0f};  // ring+148h as the motion read it
    float ring_live_rudder{0.0f};    // ring+14Ch
    float heading_error{0.0f};       // 00438B10(blk+324h, heading) at 009F40BB
    bool ai_driven{false};           // --ai-drive named this unit
    // Milestone 2o, second pass: the real state steps of packet
    // ship_ai_state_steps, once they were on main.
    unsigned long long state_step_real{0};   // 009E14C0 / 009E5770 / 009E8820 bodies run
    unsigned long long goal_sets{0};         // 009DE050 calls
    unsigned long long goal_replans{0};      // calls that re-planned the path
    unsigned long long substate_steps{0};    // attackmove's vtable +0Ch records
    float goal_x{0.0f};                      // blk+1DCh after the last 009DE050
    float goal_z{0.0f};                      // blk+1E0h
    bool goal_final_leg{false};              // blk+1E4h
    bool avoidance_enabled{false};           // blk+3ECh, what `stop` asks for
    int avoidance_side{-1};                  // blk+3F0h
    // Milestone 2p: the brain pre-pass 009F1420 and what follows from it.
    unsigned long long goal_refreshes{0};    // 009F1420 bodies that rewrote the triple
    unsigned long long goal_prepasses{0};    // 009F1420 bodies run at all
    float brain_goal_x{0.0f};                // brain+0B2Ch after the last one
    float brain_goal_y{0.0f};                // brain+0B30h
    float brain_goal_z{0.0f};                // brain+0B34h
    std::uint32_t brain_target{0};           // brain+0B20h
    std::string brain_target_name;           // the created instance it resolves to
    std::string command_descriptor;          // what 0071EB60 answered with
    unsigned long long path_plan_refreshes{0};  // 009ED3E0 call sites
    unsigned long long corridor_group_widths{0};  // 009ED3E0 head: widths from the group
    unsigned long long path_plan_seeds{0};   // 009E3780 calls that rebuilt the graph
    unsigned long long path_plan_accepts{0}; // 009E3780 calls that answered true
    int path_plan_state{0};                  // plan+1Ch after the last request
    int path_plan_nodes{0};                  // plan+34h
    // Milestone 2q: 009EC680's own ticks through the 009ED4E4 arm, the swap at
    // 009ED5BD and what 009E3C00 handed back.
    unsigned long long path_search_ticks{0};
    unsigned long long path_plan_swaps{0};
    unsigned long long path_points{0};       // 009E3C00 calls that produced a point
    unsigned long long path_corner_arms{0};  // follower exits through CornerTangent
    unsigned long long nav_output_blocks{0}; // 009EE671 bodies
    unsigned long long nav_bearings{0};      // 009EE813, the bearing was taken
    float path_point_x{0.0f};                // record+8h
    float path_point_z{0.0f};                // record+0Ch
    float nav_distance_32c{0.0f};            // blk+32Ch after the last block
    float nav_heading_324{0.0f};             // blk+324h
    // Milestone 2q: the completion round trip this unit's state step started.
    unsigned long long command_events{0};    // 009E595C, 00984300
    unsigned long long command_endings{0};   // 009E5997 / 009E88C1, 0071E430
    unsigned long long command_completions{0};  // of those, 00720850 advanced
    // Milestone 2q: 009EEAAB..009EF226, the navigation arm's tail.
    unsigned long long arm_tails{0};         // bodies
    unsigned long long arm_tail_latched{0};  // left blk+35Ch non-zero
    unsigned long long arm_tail_stops{0};    // took the 009EEFD3 stop request
    // Packet cc9_ship_ai_neighbour_count: 009EEB2A..009EEEEC, the traffic
    // setback walk over world list 6.
    unsigned long long traffic_scans{0};     // walks the four-way gate opened
    unsigned long long traffic_steps{0};     // 009EED62 set-back steps taken
    float traffic_setback_max{0.0f};         // the largest setback of a tail
    unsigned long long arrival_latches{0};   // 009EF034 set blk+2FEh
    int latched_direction_35c{0};            // blk+35Ch after the last tail
    unsigned long long approach_frames{0};   // 009F1BC0 bodies
    float approach_point_x{0.0f};            // nested+1228h
    float approach_point_z{0.0f};            // nested+1230h
    float approach_goal_range{0.0f};         // nested+11E0h
    // Packet cc8_ship_ai_approach_curves: nested+11E4h after 009E6E80, the
    // standoff range the 119-step curve scan chose, first and last, with the
    // number of 009E6E80 bodies that produced them.
    unsigned long long standoff_choices{0};  // 009E6E80 bodies
    float standoff_range_first{0.0f};        // nested+11E4h after the first
    float standoff_range_last{0.0f};         // nested+11E4h after the last
    // Packet cc9_torpedo_standoff: 009F2AC9..009F2E9B's torpedo gate and
    // clearance, and 009E72F3's cap.
    unsigned long long torpedo_standoff_frames{0};   // the block ran
    unsigned long long torpedo_standoff_enabled{0};  // it left +12BAh set
    unsigned long long torpedo_standoff_cap_gates{0}; // 009E731B with ready barrels > 0
    // Packet cc9_torpedo_gate_bytes: frames 009F1BC0 filled the gate bytes, and
    // how many left each set (+12B8h AA, +12B9h artillery, +12BBh depth charge).
    unsigned long long query_gate_frames{0};
    unsigned long long query_gate_aa{0};
    unsigned long long query_gate_artillery{0};
    unsigned long long query_gate_depth_charge{0};
    float torpedo_standoff_clearance_min{-1.0f};     // the smallest +12B4h left with +12BAh set
    float torpedo_standoff_clearance_last{-1.0f};
    // Packet cc8_ship_ai_firepower_inputs: how much of each 60-sample curve
    // 0095F080 actually filled, counted at the last refill.
    int curve_own_nonzero{0};                // nested+12C0h samples > 0
    int curve_target_nonzero{0};             // nested+13B0h samples > 0
    float unit_max_weapon_range{0.0f};       // unit+494h as the host answers it
    // Packet cc8_ship_ai_approach_slot_tune: what 009E76D0 settled on, and how
    // often the heading 009E5E90 published actually moved.
    int ring_winner_first{-1};               // nested+11E8h after the first scan
    int ring_winner_last{-1};                // nested+11E8h after the last
    // Packet cc8_ship_ai_committed_slot. nested+11E8h is the slot the unit's
    // own heading falls in, rewritten every frame-state pass at 009F28F1; the
    // ring scan's winner is the register 009E79CA..009E7C19 settles on and is
    // reported separately.
    unsigned long long committed_slot_changes{0};  // 009F28F1 wrote a new value
    int ring_scan_winner_first{-1};          // 009E7C19 after the first scan
    int ring_scan_winner_last{-1};           // 009E7C19 after the last
    float ring_total_best{0.0f};             // 009E7BE0's sum for the winner
    float ring_total_worst{0.0f};            // the same sum for the lowest slot
    float ring_word_raw_18{0.0f};            // slot 0's +18h, 009E5DA0's output
    float ring_word_normalized_2c{0.0f};     // +2Ch, 009E81E4
    float ring_word_penalty_30{0.0f};        // +30h, 009E784B's reject arm
    float ring_word_bearing_34{0.0f};        // +34h, 009E74D0
    float ring_word_evade_38{0.0f};          // +38h, 009E74D0
    float ring_word_avoid_3c{0.0f};          // +3Ch, 009E9190
    // Packet cc9_ship_traffic: the traffic records at nested+14A0h that 009E9190
    // inserts (009E935D), advances (009E950F) and erases (009E9588).
    unsigned long long traffic_inserts{0};
    unsigned long long traffic_erases{0};
    unsigned long long traffic_refreshes{0};   // 009E6240 bodies past the countdown
    int traffic_max_records{0};
    float traffic_weight_max{0.0f};            // record+120h after a refresh
    unsigned long long avoid_active_passes{0}; // 009E9190 passes with strength > 0
    float avoid_strength_max{0.0f};
    std::string traffic_first_entity;          // the first unit ever inserted
    unsigned long long ring_scan_winner_changes{0};
    // Packet cc9_target_release: 009F3240 steps that took the hold arm because
    // the held target is torn down (+5Dh).
    unsigned long long target_retired_holds{0};
    // Packet cc9_ship_formation_speed: 009F4DA0's two outputs over the run.
    unsigned long long formation_ceiling_steps{0};
    unsigned long long formation_limit_344_below_1{0};
    float formation_limit_344_min{1.0e9f};
    float formation_limit_348_min{1.0e9f};
    float formation_limit_348_max{-1.0e9f};
    std::string formation_role;    // "leader", "follower" or "none" at the last step
    // Packet cc9_station_keeping: the arm 009EDA28 and 009F4DA0's +3ADh arm.
    unsigned long long station_arm_runs{0};
    // Packet cc9_free_bearing_query.
    unsigned long long rudder_gate_open{0};     // drive ticks with blk+33Ch < 0 (009F4511)
    unsigned long long free_bearing_queries{0}; // 009DC2E0 calls from section 7
    unsigned long long free_bearing_accepts{0}; // blk+324h = query+1Ch
    float free_bearing_max_turn{0.0f};
    // Packet cc9_avoid_zone_escape.
    unsigned long long layer_selections{0};     // 009ECA20 bodies
    unsigned long long zone_inside_steps{0};    // blk+160h set after 009ECA20
    unsigned long long zone_escape_turns{0};    // 009DE8CD applied a turn
    float zone_escape_max_turn{0.0f};           // |turn|, radians
    float zone_escape_first_s{-1.0f};
    // Packet cc9_ship_neighbour_list (docs/SHIP_NEIGHBOUR_AVOIDANCE.md).
    unsigned long long neighbour_walks{0};       // 009F1856 ran (B50h due)
    unsigned long long neighbour_candidates{0};  // 009F0D20 calls
    unsigned long long neighbour_admitted{0};    // calls that appended a node
    unsigned long long neighbour_expired{0};     // nodes 009F0EA0 destroyed
    unsigned long long neighbour_node_steps{0};  // nodes both boxes refreshed
    unsigned long long neighbour_collapsed{0};   // node-steps ending with +68h set
    std::int32_t neighbour_max{0};               // largest blk+604h
    unsigned long long neighbour_list_steps{0};  // frame refreshes with a node
    unsigned long long sector_node_blocks{0};    // 009EB660 scans a node blocked
    unsigned long long separation_turns{0};      // 009DEBB9 applied a turn
    float separation_max_turn{0.0f};             // |turn|, radians
    unsigned long long traffic_passes{0};        // 009EF350 over a non-empty list
    // Packet cc9_neighbour_clips.
    unsigned long long neighbour_pass_runs{0};   // 009F0100 bodies that processed a node
    unsigned long long neighbour_pass_posts{0};  // pass-side messages 009D8C60 would route
    unsigned long long clearance_node_hits{0};   // 009EF910 sweeps a node blocked
    unsigned long long danger_steps{0};          // middle runs ending with blk+0A84h > 0
    float danger_max{0.0f};                      // largest blk+0A84h
    // Packet cc9_pass_side_message.
    unsigned long long pass_side_delivered{0};   // 009D8CE0 calls from delivered messages
    unsigned long long pass_side_changes{0};     // deliveries that changed node+88h
    unsigned long long pass_side_negotiated{0};  // deliveries ending with node+8Ch != 0
    unsigned long long traffic_writes{0};        // 009EF350 wrote blk+324h/+33Ch/+354h
    float traffic_max_turn{0.0f};                // |blk+324h change|, radians
    std::uint32_t travel_layer_min{0xFFFFFFFFu};// nav+30Ch range
    std::uint32_t travel_layer_max{0};
    // Packet cc9_ship_torpedo_response.
    unsigned long long torpedo_scans{0};        // 009F163F walks
    unsigned long long torpedo_admits{0};       // 009F0AD0 calls
    unsigned long long torpedo_tracks_built{0}; // 009EACA0 constructions
    std::size_t torpedo_tracks_max{0};
    unsigned long long torpedo_gate_open{0};    // 009DA1D0 true over a track
    unsigned long long torpedo_vector_steps{0}; // 009E04E0 left a non-zero vector
    unsigned long long torpedo_overrides{0};    // 009DE932 replaced blk+324h
    float torpedo_override_max_turn{0.0f};      // |wrap(new - old)|, radians
    float torpedo_first_override_s{-1.0f};
    unsigned long long station_requests{0};
    float station_throttle_min{1.0e9f};
    float station_throttle_max{-1.0e9f};
    unsigned long long station_limit_steps{0};  // 009F4F8C arm entered
    float station_limit_min{1.0e9f};
    float station_limit_max{-1.0e9f};
    unsigned long long heading_changes{0};   // nested+120Ch differed from before
    // Packet cc8_ship_ai_approach_slot_scorers: the three gates 009E7FC0 passes
    // before it scores a slot, as the host last answered them.
    bool  gate_flag_0b28{false};             // 009E80B0
    float gate_reference_127c{0.0f};         // 009E80BD
    float gate_lookahead_0494{0.0f};         // 009E80CD
    unsigned long long gate_flag_stops{0};   // returns at 009E80B0
    unsigned long long gate_range_stops{0};  // returns at 009E80DF
    // Packet cc8_ship_ai_goal_vector_visibility: which arm of the narrowing at
    // 009F14D2..009F1522 left brain+0B28h set.
    unsigned long long goal_timer_expiries{0};   // 009F14B6 took the long way
    unsigned long long goal_visible_true{0};     // brain+0B28h set after the pass
    unsigned long long goal_visible_recon{0};    // 009F1504 wrote nonzero
    unsigned long long goal_visible_surface{0};  // 009F1522 reopened the gate
    unsigned long long controller_updates{0};  // 0071F290 bodies
    bool controller_update_session_gate{false};
    unsigned long long path_picks{0};        // 009EE580 bodies that passed the gate
    unsigned long long path_publishes{0};    // 009EE66C, 00815F30
    unsigned long long station_keeping{0};   // 009EDA28 bodies the gate let in
    unsigned long long sector_refreshes{0};  // 009EF230 bodies
    // Milestone 2p: the projected middle of 009F3F80.
    unsigned long long middle_runs{0};       // 009F40CA..009F4B98 bodies
    float danger_a84{0.0f};                  // blk+0A84h after the last one
    float throttle_limit_344{1.0f};          // blk+344h
    float turn_assist_load_102c{0.0f};       // unit+102Ch
    // Milestone 2p: the attackmove sub-state the selector settled on.
    std::uint32_t substate{0};               // state+1508h, by its own offset
    unsigned long long substate_concrete{0}; // sub-state steps that ran a body
    // Milestone 2p: 0071DF70's two rules, measured.
    float director_hold_0040{0.0f};
    unsigned long long target_gate_tests{0};  // ticks that reached 0071DF70
    int slot0_category{-1};
    std::string slot0_command;
    // 009F5DA0, the automatic target selector
    unsigned long long target_thinks{0};   // ticks whose countdown was spent
    unsigned long long target_scans{0};
    unsigned long long fire_target_sets{0};
    unsigned long long attackmove_issues{0};
    std::string fire_target;               // the entity 00835860 last received
    float fire_target_score{0.0f};
    std::string target_blocked;            // the gate that stopped the think
    // Milestone 2r: what 009E4330 wrote into this unit's navigation block, and
    // the hull body 00937C90's tail built for it.
    float nav_turn_circle_3c8{0.0f};       // blk+3C8h, 0082E960(class, 1.0f)
    float nav_turn_circle_3cc{0.0f};       // blk+3CCh, 0082E960(class, 0.9f)
    float nav_yaw_floor_3d0{0.0f};         // blk+3D0h
    float nav_stop_radius_3d4{0.0f};       // blk+3D4h
    float nav_start_radius_3d8{0.0f};      // blk+3D8h
    float nav_hull_length_9c8{0.0f};       // unit+9C8h
    float hull_mass_00b0{0.0f};            // class+B0h
    int hull_material{0};                  // what 00937CF1 picked
    unsigned long long clearance_refreshes{0};  // 009EF910 bodies
    float clearance_37c{0.0f};             // blk+37Ch after the last one
    unsigned long long throttle_profiles{0};    // 009E04E0 bodies
    unsigned long long sector_scans{0};    // 009EB660 bodies
    unsigned long long sector_marks{0};    // scans that marked the sector blocked
    unsigned long long ring_scans{0};      // 009E76D0 bodies
    unsigned long long ring_scan_bearings{0};   // 009E5E90 commits
    int ring_scan_winner{-1};              // the slot 009E76D0 chose
    float approach_heading_120c{0.0f};     // nested+120Ch
    float approach_throttle_1210{0.0f};    // nested+1210h
    unsigned long long firepower_ratings{0};    // 0095EB40 calls
};

struct GameShipAiSummary {
    std::size_t units{0};             // controllers built
    // Packet cc9_torpedo_threat_first_node, counted in both builds.
    unsigned long long threat_head_calls{0};    // 00814420 answers asked for
    unsigned long long threat_head_differs{0};  // head answer != per-torpedo count
    unsigned long long accept_calls{0};         // 009F5E59 0071D6D0 asked
    unsigned long long accept_true{0};          // of those, the concrete body accepts
    // Packet cc9_prcp03_phase_progress: goals taken from a scene marker.
    unsigned long long marker_goal_resolves{0};
    std::size_t ai_owned{0};          // units whose +184h is clear
    unsigned long long steps{0};      // 009F50E0 bodies run
    unsigned long long gated{0};
    unsigned long long replans{0};
    unsigned long long state_steps_concrete{0};
    unsigned long long state_steps_recorded{0};
    unsigned long long publishes{0};
    unsigned long long promotions{0};
    // Milestone 2o.
    unsigned long long ring_hops{0};
    unsigned long long ring_gated_3f5{0};
    unsigned long long ring_writes{0};
    unsigned long long rudder_law_calls{0};
    unsigned long long rudder_deadbands{0};
    unsigned long long live_pair_changes{0};
    // Milestone 2o, second pass.
    unsigned long long state_steps_real{0};
    unsigned long long goal_sets{0};
    unsigned long long goal_replans{0};
    unsigned long long substate_steps{0};
    unsigned long long navigate_mode_steps{0};  // controller steps that ended in Navigate
    std::size_t units_with_goal{0};
    // Milestone 2p.
    unsigned long long goal_prepasses{0};
    unsigned long long goal_refreshes{0};
    std::size_t units_with_nonzero_goal{0};
    std::size_t units_with_brain_target{0};
    unsigned long long path_plan_refreshes{0};
    unsigned long long corridor_group_widths{0};
    float corridor_width_max{0.0f};
    unsigned long long warning_timer_expiries{0};
    unsigned long long warning_torpedo_reports{0};
    unsigned long long warning_other_reports{0};
    unsigned long long path_plan_seeds{0};
    unsigned long long path_plan_accepts{0};
    unsigned long long approach_frames{0};
    unsigned long long controller_updates{0};
    unsigned long long path_picks{0};
    unsigned long long path_publishes{0};
    unsigned long long station_keeping{0};
    unsigned long long sector_refreshes{0};
    unsigned long long middle_runs{0};
    unsigned long long substate_concrete{0};
    // Milestone 2q.
    unsigned long long path_search_ticks{0};
    unsigned long long path_plan_swaps{0};
    unsigned long long path_points{0};
    unsigned long long path_corner_arms{0};
    unsigned long long nav_output_blocks{0};
    unsigned long long nav_bearings{0};
    std::size_t units_with_path_point{0};
    unsigned long long command_events{0};
    unsigned long long command_event_callbacks{0};
    unsigned long long command_endings{0};
    unsigned long long command_completions{0};
    unsigned long long arm_tails{0};
    unsigned long long arm_tail_latched{0};
    unsigned long long arm_tail_stops{0};
    unsigned long long arrival_latches{0};
    // Packet cc9_ship_ai_neighbour_count.
    unsigned long long traffic_count_reads{0};   // 009EEB8B reads
    unsigned long long traffic_count_sum{0};     // their values, summed
    std::size_t traffic_count_max{0};
    unsigned long long traffic_kind_passes{0};   // entities past 009EEBCC and 009EEBDA
    unsigned long long traffic_scans{0};
    unsigned long long traffic_steps{0};
    // Packet cc9_ship_ai_ring_scan_probe.
    unsigned long long ring_probe_spaces{0};
    unsigned long long ring_probe_moved_starts{0};
    unsigned long long ring_probe_casts{0};
    unsigned long long ring_probe_hits{0};
    // Packet cc9_approach_enter_reseed: approach member enters (009F3220),
    // attackmove state enters and exits (009E86C0 / 009E86E0), and the stream-1
    // draws the re-seeds took. The first three count on both sides.
    unsigned long long approach_member_enters{0};
    unsigned long long attackmove_state_enters{0};
    unsigned long long attackmove_state_exits{0};
    unsigned long long approach_reseeds{0};
    unsigned long long approach_reseed_draws{0};
    // Packet cc9_follow_station_point: the zone sets resolved and pushes run
    // (ON only), pushes that moved the point, and leader yaw-rate reads that
    // were non-zero (both sides).
    unsigned long long follow_zone_sets{0};
    unsigned long long follow_pushes{0};
    unsigned long long follow_pushes_moved{0};
    unsigned long long follow_leader_turning{0};
    // Packet cc9_ai_command_avoid_zone_point: the AI command's 00417B10 at
    // 00A020F0, asked through avoid_zone_offset_point_00a020f0 (ON only): the
    // queries answered with a zone set, and those whose point moved.
    unsigned long long ai_command_zone_points{0};
    // Packet cc9_engage_kamikaze_gate: 009E85CD reads, and reads whose class
    // carries a positive KamikazeDamage or KamikazeBlastDamage (both sides).
    unsigned long long engage_kamikaze_reads{0};
    unsigned long long engage_kamikaze_classes{0};
    // Packet cc9_engage_kamikaze_gate: the engage member state+14C0h, its enters
    // and steps (both sides) and the steps that left the run latch set (ON only).
    unsigned long long engage_member_enters{0};
    unsigned long long engage_member_steps{0};
    unsigned long long engage_member_run_steps{0};
    // Packet cc9_approach_sight_test: 009E7FC0's two sight tests. Target tests
    // run ON only (they write the pass cache); point tests run on both sides.
    unsigned long long sight_target_tests{0};
    unsigned long long sight_target_hidden{0};
    unsigned long long sight_point_tests{0};
    unsigned long long sight_point_hidden{0};
    // Packet cc9_approach_mode_latch: 009F1BC0's latch computed on every
    // frame-state pass (both sides), by target class and by the mode it chose.
    unsigned long long latch_frames{0};
    unsigned long long latch_no_target{0};
    unsigned long long latch_ship_target{0};
    unsigned long long latch_sub_target{0};      // ship target of kind 8, unit not kind 8
    unsigned long long latch_building_target{0}; // kind 1Ch and not kind 6
    unsigned long long latch_building_same_side{0};
    unsigned long long latch_building_lander{0};
    unsigned long long latch_other_target{0};
    // Packet cc9_standoff_target_kind (either state): 009E6E80's kind queries
    // that the target answers true, and the building arms they open.
    unsigned long long standoff_kind_calls{0};
    unsigned long long standoff_kind_08{0};
    unsigned long long standoff_kind_1c_mode2{0};
    unsigned long long standoff_kind_1c_mode4{0};
    unsigned long long standoff_small_class{0};    // 00827F70 true at 009E6F11
    unsigned long long latch_modes[5]{};
    unsigned long long latch_clamps{0};          // mode-1 arm lowered +11F0h
    unsigned long long latch_resets{0};          // 009F20DE..009F20ED
    unsigned long long latch_retarget_reachable{0}; // no ship target: 009F2124 follows
    unsigned long long latch_retarget_entries{0};   // ... and nested+11D6h was clear
    // Packet cc9_landing_modes_3_4 (ON only): mode-3 runs with no free or held
    // pad, runs that set a pad approach point, pad approach lines recast, runs
    // inside the reach, landings begun (0074A990), and mode-4 standoff points.
    unsigned long long landing_mode3_no_pad{0};
    unsigned long long landing_mode3_points{0};
    unsigned long long landing_pad_line_casts{0};
    unsigned long long landing_mode3_in_reach{0};
    unsigned long long landing_begins{0};
    // Packet cc9_startlanding_94h: 0074A4C0's outcomes.
    unsigned long long landing_requests_94h{0};
    unsigned long long landing_requests_begun{0};
    unsigned long long landing_requests_held{0};     // -8, ship+1200h already set
    unsigned long long landing_requests_no_site{0};  // 006F2C30 found none
    unsigned long long landing_requests_no_pad{0};   // -4, 006F2A50 found none
    unsigned long long landing_mode4_points{0};
    // Packet cc9_landing_craft_launch: 008206F0's outcomes, its crafts, and the
    // perimeter candidates the depth and clearance probes rejected.
    unsigned long long craft_launch_calls{0};
    unsigned long long craft_launch_launched{0};   // calls that made a craft
    unsigned long long craft_launch_cooling{0};    // -3, +1124h above 0.0
    unsigned long long craft_launch_no_site{0};
    unsigned long long craft_launch_no_pad{0};     // -4
    unsigned long long craft_launch_refused{0};    // -8, every candidate rejected
    unsigned long long craft_launch_other{0};      // -1 / -2
    unsigned long long crafts_created{0};
    unsigned long long craft_create_failed{0};
    unsigned long long craft_depth_rejects{0};
    unsigned long long craft_clearance_rejects{0};
    // Packet cc9_land_step_host: the land state's enter (009E18D0) and step
    // (009E1950) runs, steps that held a pad, steps in the final arm, and pad
    // re-picks that assigned a new pad.
    unsigned long long land_enters{0};
    unsigned long long land_steps{0};
    unsigned long long land_steps_with_pad{0};
    unsigned long long land_steps_final{0};
    unsigned long long land_pad_assigns{0};
    // Packet cc9_approach_retarget_ring (ON only): arm runs past 009F2124, runs
    // whose goal lay in a zone, runs that left the point off the goal, and the
    // per-slot Landscape queries, hits and reach failures.
    unsigned long long retarget_off_zone_frames{0};  // OFF: arm frames, goal in a zone
    // Packet cc9_ship_ai_backoff_countdown: 009F3F80 entries with the astern
    // latch blk+380h at or above zero, and (ON) the entries that expired it.
    unsigned long long backoff_held_steps{0};
    unsigned long long backoff_expiries{0};
    // 009ED6B0 entries that found the escape byte blk+36Ch still set (009ED788).
    unsigned long long escape_36c_stale_steps{0};
    // 009ED6B0 entries that found the turn side blk+304h non-zero (009ED78F).
    unsigned long long nav_side_304_set_on_entry{0};
    // Station-arm steps whose 009DE5B0 separation turn ran on a non-zero side.
    unsigned long long station_separation_sided{0};
    // Packet cc9_approach_no_ship_hold (kShipAiApproachNoShipHoldBound).
    unsigned long long hold_arm_runs{0};       // 009F2124 head passed: the goal stored
    unsigned long long hold_frames{0};         // modes 0/2 no-ship frames held
    unsigned long long hold_frames_differ{0};  // held point differs from the goal copy
    unsigned long long retarget_runs{0};
    unsigned long long retarget_zone_runs{0};
    unsigned long long retarget_moved_runs{0};
    unsigned long long retarget_landscape_queries{0};
    unsigned long long retarget_landscape_hits{0};
    unsigned long long retarget_out_of_reach{0};
    unsigned long long ai_command_zone_points_moved{0};
    // Packet cc9_plane_row_autotarget: AutoTarget ticks reaching a plane or
    // squadron row (both sides) and the thinks they ran (OFF only).
    unsigned long long plane_row_autotarget_ticks{0};
    unsigned long long plane_row_autotarget_thinks{0};
    // Packet cc9_autotarget_follower_gate: AutoTarget thinks by a formation
    // follower (both sides) and the leaves 009F5DEB ran (ON only).
    unsigned long long autotarget_follower_thinks{0};
    // Packet cc9_clearance_path_fade: 009F0000's gate, counted on both sides.
    unsigned long long path_fade_tests{0};
    unsigned long long path_fade_leader{0};
    unsigned long long path_fade_moveonpath{0};
    unsigned long long path_fade_applied{0};
    unsigned long long clearance_heading_error_large{0};
    // Packet cc9_arm_final_area_key: 0070E450 at the arm final's 009DEEE9 / 009DEFD3.
    unsigned long long arm_final_area_keys{0};
    // Packet cc9_close_member_class_trait: 009F347E / 009F35E3 asks, and trait holds.
    unsigned long long approach_trait_tests{0};
    // Packet cc9_clearance_outcome_wiring: blk+370h as 009F3F80 sees it, per value,
    // and the obstacle routine's load raises (all three sites of each).
    unsigned long long clearance_outcome_frames[4]{};
    unsigned long long obstacle_turn_assist_raises{0};
    unsigned long long obstacle_secondary_raises{0};
    // Packet cc9_heading_wrap_census: the heading stores 00605070 wraps in place
    // (009DFF81, 009E00FA, the setter 009DFFB0, the approach 009F3360), and how many
    // of them lie outside (-pi, pi].
    unsigned long long heading_wrap_stores{0};
    unsigned long long heading_wrap_out_of_range{0};
    float heading_wrap_max_abs{0.0f};
    unsigned long long approach_troop_landers{0};
    unsigned long long arm_final_area_key_differs{0};
    // Packet cc9_surface_set_branch: 00922D54's 008DDF90 asks, counted both ways.
    unsigned long long surface_set_queries{0};
    unsigned long long surface_set_nonempty{0};
    unsigned long long surface_set_vehicles{0};
    unsigned long long surface_set_hits{0};
    // Packet cc9_engage_gate_avoid_zone: 009E8658's asks, the destinations in a
    // zone, and those the range test would have passed (counted both ways).
    unsigned long long engage_zone_asks{0};
    unsigned long long engage_zone_hits{0};
    unsigned long long engage_zone_closes{0};
    unsigned long long autotarget_follower_leaves{0};
    // Packet cc9_free_bearing_query: 009DC2E0 calls by site (both sides) and
    // what the bound query did (ON only).
    unsigned long long free_bearing_calls_scan{0};
    unsigned long long free_bearing_calls_arm{0};
    unsigned long long free_bearing_unready{0};
    unsigned long long free_bearing_empty{0};
    unsigned long long free_bearing_refills{0};
    unsigned long long free_bearing_ahead_hits{0};
    unsigned long long free_bearing_corner_fwd{0};
    unsigned long long free_bearing_corner_back{0};
    unsigned long long free_bearing_lateral_turns{0};
    unsigned long long free_bearing_short_legs{0};
    unsigned long long free_bearing_answers{0};
    std::size_t units_accepting_new_target{0};
    unsigned long long thinks{0};
    unsigned long long scans{0};
    unsigned long long fire_target_sets{0};
    unsigned long long attackmove_issues{0};
    std::size_t units_with_fire_target{0};
    std::size_t states_cruise{0};
    std::size_t states_stop{0};
    std::size_t states_attackmove{0};
    std::size_t states_movetopos{0};
    std::size_t states_other{0};
    // Milestone 2r.
    std::size_t nav_blocks{0};                  //009E4330 bodies, actual ship classes only
    unsigned long long clearance_refreshes{0};  // 009EF910
    unsigned long long throttle_profiles{0};    // 009E04E0
    unsigned long long sector_scans{0};         // 009EB660
    unsigned long long sector_marks{0};
    unsigned long long ring_scans{0};           // 009E76D0
    unsigned long long ring_scan_bearings{0};   // 009E5E90
    unsigned long long firepower_ratings{0};    // 0095EB40
    unsigned long long standoff_choices{0};     // 009E6E80
    unsigned long long approach_curve_refreshes{0}; // 0095F080 at 009F2F11 / 009F2FB1
    // Packet cc9_target_curve_refill: [owner+0B20h]'s kind per approach frame
    // (vehicle 5, plane 0Fh, squadron 18h, structure 1Ch, other, none), and the
    // bound arm's outcomes at 009F2F26..009F2FD3.
    unsigned long long target_curve_kinds[6]{};
    unsigned long long target_curve_emptied{0};    // 009F2FC6
    unsigned long long target_curve_kind_skips{0}; // 009F2F44
    unsigned long long target_curve_mode_skips{0}; // 009F2F7F
    unsigned long long target_curve_refills{0};    // 009F2FB1
    // Packet cc9_own_curve_refill_gate: due own refills the bound gates refuse.
    unsigned long long own_curve_mode_skips{0};    // 009F2EE2
    unsigned long long own_curve_flag_skips{0};    // 009F2EEB
    // Packet cc9_approach_leader_answers: 00778890 asks at 009E6C45 / 009E6FA0 / 009E70A7.
    unsigned long long approach_leader_asks{0};
    // Packet cc9_attackmove_group_hand_back (009E8852..009E889C), both ways.
    unsigned long long attack_hand_back_asks{0};
    unsigned long long attack_hand_back_grouped{0};
    unsigned long long attack_hand_back_mixed{0};
    // Packet cc9_approach_landing_sweep (009F33E2..009F35F5), both ways: sweep
    // entries, leaders, candidates timed, those slow enough (ratio < 0.4),
    // those in LandingRange, the 94h messages, and the bound deliveries.
    unsigned long long landing_sweep_entries{0};
    unsigned long long landing_sweep_leaders{0};
    unsigned long long landing_sweep_candidates{0};
    unsigned long long landing_sweep_slow{0};
    unsigned long long landing_sweep_in_range{0};
    unsigned long long landing_sweep_messages{0};
    unsigned long long landing_sweep_begun{0};
    unsigned long long landing_sweep_launched{0};
    // Packet cc9_approach_target_layer_push (009F1E36..009F1F07), both ways.
    unsigned long long target_layer_ship_passes{0}; // a ship target with a bot
    unsigned long long target_layer_below{0};       // its layer < own class+570h
    unsigned long long target_layer_moved{0};       // 00417B10 moves the goal > 1
    int target_layer_min{2147483647};               // lowest target position layer
    int target_layer_own_max{-2147483647 - 1};      // highest own class+570h
    // Packet cc9_torpedo_standoff: the exits of 009F2AC9..009F2E9B in the order
    // of bsp::ShipAiTorpedoStandoffExit, and 009E72F3's gate.
    unsigned long long torpedo_standoff_frames{0};
    unsigned long long torpedo_standoff_exits[9]{};
    unsigned long long torpedo_standoff_cap_tests{0};  // 009E731B reached (+12BAh, +12B4h > 0)
    unsigned long long torpedo_standoff_cap_gates{0};  // and 0080DF40 answered > 0
    // Packet cc9_torpedo_gate_bytes: frames and set counts of the four gate
    // bytes 009F1BC0 fills (+12BAh counted after the standoff block).
    unsigned long long query_gate_frames{0};
    unsigned long long query_gate_aa{0};
    unsigned long long query_gate_artillery{0};
    unsigned long long query_gate_torpedo{0};
    unsigned long long query_gate_depth_charge{0};
    unsigned long long firepower_gate_stops{0}; // 0095EB6E, b[0] >= unit+494h
    unsigned long long firepower_mounts{0};     // gun list nodes 0095EB40 walked
    unsigned long long path_follower_points{0}; // 009E3C00 through the full follower
    unsigned long long path_follower_corners{0};
    unsigned long long path_follower_advances{0};
};

// One ship AI controller per created unit, for the whole run.
class GameShipAiHost {
public:
    GameShipAiHost(GameHostLog& log, GameUnitsHost& units);
    ~GameShipAiHost();
    GameShipAiHost(const GameShipAiHost&) = delete;
    GameShipAiHost& operator=(const GameShipAiHost&) = delete;

    // Borrow the actual session participant owner; it must outlive this host.
    void bind_session_participants(const bsp::SessionParticipantPools& owner) noexcept;
    // The bound owner, or null (packet cc9_usn02_sameside_torpedoes: 00927F10 for gunnery).
    const bsp::SessionParticipantPools* session_participants() const noexcept;

    // Packet cc8_ship_ai_firepower_inputs. Borrow the gunnery host so the
    // firepower rating 0095EB40 can read the tables 00956C20 built: the max
    // weapon range at unit+494h that gates its whole body (0095EB62), the
    // per-category counts at unit+394h and ranges at unit+430h, and the
    // category gun lists at unit+398h. GameGunneryHost::set_ship_ai calls this,
    // so the two hosts are wired wherever that already is. It must outlive this
    // host.
    void bind_gunnery(GameGunneryHost* gunnery) noexcept;

    // One controller per created instance, in creation order. Called once,
    // after the instantiate pass and after the authored commands were issued.
    void register_units(GameMissionLuaHost& lua, std::int32_t session_mode);
    void load_avoid_zone_geometry(const GameSceneContentsHost&, GameMissionLuaHost&,
        std::int32_t mode, std::uint8_t forced, std::int32_t session);

    // 009F50E0 for every registered unit, once per fixed simulation step, and
    // 009F5DA0 beside it. Run before the motion pass, because the motion's head
    // at 00825F2C consumes the slot 009F4D10 published.
    void controller_step(float seconds);

    // 00825F2C..00825F7C, the motion head's own promotion of the slot the
    // controller published. Returns true when a valid order was promoted.
    bool promote_order_00825f2c(std::size_t unit_index);

    // Packet cc9_command_building_capture_bind: the gunnery host's kill funnel asks
    // this first. True for a CommandBuilding while kCommandBuildingCaptureBound is
    // on: 006F3270 neutralizes an owned building at health 0 (or returns for a
    // neutral one) and the building does not die (vtable[1A8h] 006F1F80 is RET).
    // False otherwise, and the funnel kills as before.
    bool command_building_health_zero_006f3270(std::size_t unit_index);

    // Packet cc9_big_landing_ship: class+808h for the unit's class, from its
    // VehicleClass row at load (the depth reader's tuning-pair choice). False
    // for a unit that is not a loaded LandingShip. Read-only.
    bool unit_big_landing_ship_0808(std::size_t unit_index) const;

    // Packet cc9_scripted_order_natives: 008A7060 NavigatorEnable stores its
    // boolean at [unit+740h]+11h, the enabled byte of the ship AI controller's
    // tick sub-node (0072BBD0 sets it to 1). 008759C4 skips the controller's
    // +0Ch tick (009F50E0) while it is 0. Kept per unit index, so a call made
    // before the controller is registered still applies.
    void set_navigator_enabled_0011(std::size_t unit_index, bool enabled);

    // --ai-drive <name>=<throttle>,<rudder>, milestone 2o. A LABELLED
    // DIAGNOSTIC STAND-IN, not a reconstruction: eight of the nine state steps
    // have no body, so on a re-plan tick of a unit named here the executable
    // calls the two recovered setters 009DBF90 and 009DFFB0 on that unit's own
    // control block in place of the state step's decision. Everything after
    // that point - 009ED6B0, 009F4D10, 009F4DA0, 009F3F80's hop, 00813020 and
    // 00825F20 - is the game's own recovered path.
    void set_ai_drive(std::size_t unit_index, float throttle, float rudder);

    const std::vector<GameShipAiRow>& rows() const noexcept;
    // Packet cc9_usn02_deruyter_fire: 00836240 on the unit's director, with
    // target_plus_one 0 for a null target.
    void store_fire_target_00836240(std::size_t unit, std::size_t target_plus_one,
                                    bool force);
    const GameShipAiSummary& summary() const noexcept;

    // Packet cc9_ai_command_avoid_zone_point. 00A02020's ship arm for the AI
    // host: 00A020BE 0082ADA0(ECX = [unit+538h], 0) = 004120D0(manager,
    // [class+560h]), then 00A020F0 00417B10(ECX = that group, &out, &in {x, z},
    // margin, 1). Returns false, leaving `out` alone, when the avoid-zone runtime
    // is not ready, the unit has no ship controller or no group answers the
    // layer; the caller then keeps the requested point.
    bool avoid_zone_offset_point_00a020f0(std::size_t unit, const float in_xz[2],
                                          float margin, float out_xz[2]);

    // Packet cc9_startlanding_94h (docs/SHIP_AI_OPEN_ITEMS.md section 97). 94h
    // MT_SHIP_STARTLANDING on an MLandingShip: 00821F61 -> vt+238h = 0074A4C0
    // (body 0074A4C0-0074A59A). -8 when ship+1200h is set; 006F2C30(&ship+FCh,
    // ship+54h, 2) for the site (err - 8 when none; this host answers -9); the
    // nearest free pad 006F2A50(site)(ship) (-4 when none); then ship+1200h =
    // pad and 0A5h -> 0074B570 -> 0074A990, delivered at the call (LABELLED, as
    // the mode-3 path does). Answers 1 when a landing began.
    int landing_ship_request_landing_0074a4c0(std::size_t unit);

    // Packet cc9_landing_craft_launch (docs/SHIP_AI_OPEN_ITEMS.md section 100).
    // 94h on the troop transport (vtable 00CFA778): vt+238h = 008206F0, body
    // 008206F0-00821E79, __fastcall(ship). Guards -1 (no craft class +78Ch), -2
    // (LandingShipAmount 0), -3 (+1124h above 0.0); no site answers -9 as the
    // 0074A4C0 host does. Rings 1..4 of the grown hull rectangle, three perimeter
    // points near the first free pad, the depth and ship clearance probes, then
    // one craft per free pad (created through GameUnitsHost::create_units; its
    // InitAll's 0074A990 and `land` delivered at the call, LABELLED). Answers
    // the number launched, else -4 (pads ran out) or -8.
    int transport_launch_craft_008206f0(std::size_t unit);
    // ship+1124h, the launch cooldown: written by 008206F0 at 00821DC9 and by
    // 95h at 00821F85 only; 0.0 until then (constructor 0081F2B4).
    float transport_cooldown_1124(std::size_t unit) const;

    // The per-unit table and the one-line summaries the milestone reports.
    void report();
    // One sampled line per unit every `interval` controller steps.
    void log_sample(unsigned long long step_index, unsigned long long interval);

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
