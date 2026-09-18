// bsp_game.exe milestone 2i: the units the instantiate pass created, ticked.
//
// Milestone 2h created 32 `DestroyerGen` instances and reported that every unit
// pass of the frame and of the fixed step ticked 0 of them, because nothing
// walked them. This file is the other half of the fix: one state object per
// created unit, over which the reconstructions on main run.
//
// What runs here is recovered: 008255b0's twelve steps, 00813020's ring tick,
// 00825f20's gates and command chain with 0092d300, 0092e8c0 and 00937440,
// 0092be80's controller step and 004c0890's controlled-unit bind. What this
// file supplies, and labels, is listed in include/bsp/game_hosts_units.hpp.

#include "bsp/game_hosts_units.hpp"
#include "bsp/plane_flight.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/plane_advance_pose.hpp"
#include "bsp/plane_angular_velocity.hpp"
#include "bsp/plane_control_rate.hpp"
#include "bsp/pilot_plan_slots.hpp"
#include "bsp/plane_attitude_angles.hpp"
#include "bsp/plane_ai_control.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/torpedo_aim_tick.hpp"
#include "bsp/torpedo_approach_update.hpp"
#include "bsp/torpedo_first_release.hpp"
#include "bsp/torpedo_issue_timing.hpp"
#include "bsp/torpedo_release_orders.hpp"
#include "bsp/torpedo_task_arm.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/torpedo_release_spawn.hpp"
#include "bsp/bot_tasks.hpp"
#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/unit_hull_extents.hpp"

#include "bsp/camera_affine.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/cruise_speed_setting.hpp"
#include "bsp/dyn_world_settings.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/ship_hydro_forces.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/ship_hull_body.hpp"
#include "bsp/ship_motion.hpp"
#include "bsp/unit_controller.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/unit_instance.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/unit_kind_query.hpp"
#include "bsp/unit_world_registration.hpp"
#include "bsp/unit_generic_input_phase.hpp"
#include "bsp/tick_element_overrides.hpp"
#include "bsp/unit_order_record.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_construct.hpp"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

constexpr double kPi = 3.14159265358979323846;

// SUBSTITUTION, labelled: Pilot/Torpedo/ReferenceSpeed, the divisor of
// 009F9D30's run-profile speed ratio. The PilotBot tuning block is not
// reachable from this host, so the value is the authored one recorded against
// offset +440h in include/bsp/bot_tasks.hpp:266, KMH(300).
constexpr float kTorpedoReferenceSpeedAuthored = 83.333336f;


// Dispatch coverage only; direct_ship_body retains the existing partial
// 00825F20 reconstruction described in SHIP_MOTION.md.
enum class UnitMotionCoverage {
    unresolved,
    generic_guarded,
    direct_ship_body,
    ship_base_fragment,
};

struct UnitMotionDispatch {
    std::uint32_t creator{};
    std::uint32_t tick_vtable{};
    std::uint32_t entry{};
    UnitMotionCoverage coverage{UnitMotionCoverage::unresolved};

    bool runs_ship_base() const noexcept {
        return coverage == UnitMotionCoverage::direct_ship_body
            || coverage == UnitMotionCoverage::ship_base_fragment;
    }
};

// Actual leaf constructor stores at unit+310h, then that table's +8h slot.
// Select through the existing descriptor allocator identity, not MaxSpeed or
// the navigation-controller classifier. These are evidence addresses, not
// callable vtables in this process. See SHIP_MOTION_NONFINITE_ORIGIN.md.
constexpr UnitMotionDispatch kUnitMotionDispatches[] = {
    {0x006fe590, 0x00cfc38c, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x006fb430, 0x00cfb6f0, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x0074be00, 0x00cff9ec, 0x00749b20, UnitMotionCoverage::ship_base_fragment},
    {0x006eb290, 0x00cfa730, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x006dfef0, 0x00cf9068, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x008531a0, 0x00d0bf3c, 0x00855420, UnitMotionCoverage::ship_base_fragment},
    {0x00857e20, 0x00d0c604, 0x00825f20, UnitMotionCoverage::direct_ship_body},
    {0x00758d30, 0x00d015e8, 0x00758270, UnitMotionCoverage::ship_base_fragment},
    {0x008091d0, 0x00d0002c, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x0084ca50, 0x00d0ba3c, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x0074e540, 0x00d002c4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x007ddae0, 0x00d068dc, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x00956390, 0x00d19ce4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x009564e0, 0x00d19fbc, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x00956240, 0x00d1a294, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x007d7850, 0x00d065f4, 0x007ce040, UnitMotionCoverage::unresolved},
    {0x006d3110, 0x00cf8bc0, 0x006d2510, UnitMotionCoverage::unresolved},
    {0x00848380, 0x00d0b728, 0x00846320, UnitMotionCoverage::unresolved},
    {0x0074df10, 0x00cffd9c, 0x00953cc0, UnitMotionCoverage::generic_guarded},
    {0x00747000, 0x00cff3b4, 0x00953cc0, UnitMotionCoverage::generic_guarded},
    {0x006f5c10, 0x00cfafe0, 0x00953cc0, UnitMotionCoverage::generic_guarded},
};

UnitMotionDispatch unit_motion_dispatch(const bsp::VehicleClassDescriptorRow* descriptor) {
    if (descriptor != nullptr) {
        for (const UnitMotionDispatch& dispatch : kUnitMotionDispatches) {
            if (dispatch.creator == descriptor->allocate_instance) return dispatch;
        }
    }
    return {};
}

const char* unit_motion_coverage_name(UnitMotionCoverage coverage) {
    switch (coverage) {
    case UnitMotionCoverage::direct_ship_body: return "direct_ship_body";
    case UnitMotionCoverage::ship_base_fragment: return "ship_base_fragment";
    case UnitMotionCoverage::generic_guarded: return "generic_guarded";
    default: return "unresolved";
    }
}

float heading_degrees_of(const bsp::ShipMotionState& state) {
    return static_cast<float>(std::atan2(static_cast<double>(state.pose_row2[0]),
                                  static_cast<double>(state.pose_row2[2]))
        * 180.0 / kPi);
}

// `<x>,<z>`, the point form of `--order moveto:<target>`. Both halves must be
// numbers and nothing may follow them, so an entity name can never be mistaken
// for a point. An optional leading `@` is accepted for readability.
bool parse_order_position(const std::string& text, float& x, float& z) {
    std::size_t begin = 0;
    if (begin < text.size() && text[begin] == '@') ++begin;
    const std::size_t comma = text.find(',', begin);
    if (comma == std::string::npos || comma == begin) return false;
    const std::string first = text.substr(begin, comma - begin);
    const std::string second = text.substr(comma + 1);
    if (second.empty()) return false;
    char* end = nullptr;
    const double parsed_x = std::strtod(first.c_str(), &end);
    if (end == nullptr || *end != '\0') return false;
    const double parsed_z = std::strtod(second.c_str(), &end);
    if (end == nullptr || *end != '\0') return false;
    x = static_cast<float>(parsed_x);
    z = static_cast<float>(parsed_z);
    return true;
}

}  // namespace

// ---------------------------------------------------------------------------
// One created unit
// ---------------------------------------------------------------------------

// How many stand-in buoyancy elements a hull gets. The native count is
// (class+530h - class+52Ch) / 24h and is authored data this process does not
// have; eight is the probe's own --hydro-elements default, so the two sides of
// the comparison use the same number.
constexpr int kBuoyancyElementCount = 8;

struct GameUnitSlot;

// Persistent list/node storage for the existing mission world registry.
// These triples reproduce the fields consumed by00484540 and009F1877;
// the enclosing C++ owner is not a raw4BCh native world object.
struct GameUnitWorldNode {
    GameUnitWorldNode* previous{};
    GameUnitWorldNode* next{};
    GameUnitSlot* unit{};
};
struct GameUnitWorldList {
    std::uint32_t count{}; //004B7EC0 clears all three words
    GameUnitWorldNode* head{};
    GameUnitWorldNode* tail{};
};
static_assert(sizeof(GameUnitWorldNode) == 0x0c);
static_assert(offsetof(GameUnitWorldNode, unit) == 8);
static_assert(sizeof(GameUnitWorldList) == 0x0c);
struct GameUnitWorldLists {
    GameUnitWorldList entries[bsp::kWorldSlotCount];
    GameUnitWorldLists() = default;
    GameUnitWorldLists(const GameUnitWorldLists&) = delete;
    GameUnitWorldLists& operator=(const GameUnitWorldLists&) = delete;
    // Process resource cleanup. This does not claim native world teardown,
    // observer notification or entity detach callback ordering.
    ~GameUnitWorldLists() {
        for (GameUnitWorldList& list : entries) {
            while (list.head != nullptr) {
                GameUnitWorldNode* node = list.head;
                list.head = node->next;
                delete node;
            }
        }
    }
};

struct GameUnitSlot {
    GameUnitSlot() {
        // Partial projections of the two actual bases, not a whole unit ctor.
        bsp::initialize_unit_observed_prefix_00925cff(observer_prefix);
        bsp::initialize_unit_callback_prefix_00925d13(observer_prefix);
        bsp::publish_scene_observer_tables_00925d44(observer_prefix);
        //00928713..00928748: current assignments, not the +188h policy table.
        for (std::int32_t& role : current_roles_01ac) role = bsp::kUnitRoleTableFill;
    }

    GameUnitRow row;
    bsp::NativeUnitObserverPrefixStorage observer_prefix;
    bool observer_prefix_ready{false};
    UnitMotionDispatch motion_dispatch;
    GameUnitWorldLists* world_parent_0030{};
    std::size_t process_index{}; // metadata, not a native unit field
    // This unit owns the canonical current roles for every process consumer.
    // Actual 4Bh receive-side assignment and its side effects remain pending.
    std::int32_t current_roles_01ac[bsp::kUnitRoleTableEntries];

    // Only constructor-established cells used by 00953CC0 / 0095DC40.
    // The role and gate members of the pure routine's view are not owners.
    std::int32_t generic_notify_528{-9};       //0095CDC1
    std::int32_t generic_flag_634{0};          //0095CE0B
    float generic_timer_6f8{0.0f};             //0095CF50
    float generic_timer_6fc{0.0f};             //0095CF58
    bool generic_suppress_520{false};         //0095CDD7

    // Plane control state. 007CFD20 zeroes unit+900h (XOR EBX,EBX); the
    // free-flight arm needs 7, which 007C6340's fall-through sets at 007C6481
    // alongside unit+908h = 3600. All three inputs of select_motion_arm_007ce040
    // derive from this one field: the gate (*(unit+72Ch))->vtable[+38h] is
    // BSP_PlaneControlMode_IsFreeFlight, `+1D4h == 7`, and 310h+41Ch+1D4h = 900h.
    // docs/PLANE_FLIGHT_CORE_LAW.md, docs/PLANE_UNIT_TICK.md.
    std::int32_t plane_control_mode_900{0};
    float plane_airborne_908{0.0f};
    bool plane_airborne_frozen_9e0{false};
    // Free-flight motion. 007DB680 is an ACCUMULATOR pass, not an integrator:
    // it leaves four accumulators for 007D8470 to fold, and the caller applies
    // the result. The integration below is therefore the host's, not a
    // reconstruction of native code. docs/PLANE_FREE_FLIGHT_PHYSICS.md.
    float plane_world_velocity[3]{0.0f, 0.0f, 0.0f};
    float plane_lost_drag_timer_c3c{0.0f};
    bool plane_velocity_seeded{false};
    // The controller's body angular velocity, ctl+48h pitch, +4Ch yaw, +50h
    // roll. 007DA710 writes these three and 007D9C80 rotates them into world
    // for 0085E4D0 to turn the pose with; plane_angular_velocity.hpp records
    // the same offsets as kAngularBody, recovered from the other end.
    //
    // Nothing writes them yet. 007DA710 drives each toward a target built from
    // the latched control inputs at ctl+BB0h/BB4h/BB8h, and no path from a bot
    // task to those latches is reconstructed, so every target is zero, the law
    // holds the axis at zero, and no plane turns. That is the open link, not a
    // simplification: docs/PLANE_CONTROL_RATE_LAW.md and
    // docs/PLANE_BOT_CONTROL_WRITEBACK.md.
    float plane_body_angular[3]{0.0f, 0.0f, 0.0f};
    // unit+9E4h yaw, +9E8h pitch, +9ECh roll - the live pilot control block,
    // and unit+BB0h/+BB4h/+BB8h, the previous-step snapshot 007B9770 latches
    // from it. plane_flight.hpp owns the offsets and carries the correction
    // note for the axis order; plane_advance_pose.hpp had it swapped until
    // packet cc7_plane_control_targets.
    float plane_live_controls[3]{0.0f, 0.0f, 0.0f};
    // unit+9F0h and +9F4h. 007CFEB0 sets the throttle to 1.0f at construction
    // and 007CFEA4 zeroes the air brake.
    // unit+C64h pitch, +C68h bank, +C6Ch heading, all from 007C1900 over the
    // live pose. Slot state rather than locals because the native leaves the
    // heading and bank untouched when the forward axis is near vertical.
    float plane_pitch_angle_c64{0.0f};
    float plane_bank_angle_c68{0.0f};
    float plane_heading_c6c{0.0f};
    float plane_class_turn_roll_spd{0.0f};   // desc+1C8h TurnRollSpd
    float plane_class_turn_roll{0.0f};       // desc+25Ch TurnRoll
    // The plan's non-slot fields, reset by 0099B450 on every think.
    bsp::PilotPlanState plan_state;
    // The range to the commanded target the first time the yaw arm planned for
    // this unit, and the last. Two numbers, so the run can say whether an
    // ordered aircraft actually closed on what it was ordered at.
    float attack_range_first{-1.0f};
    float attack_range_last{-1.0f};
    // The absolute heading error at the first plan and the last. This is the
    // measurement that says whether the yaw law steers: the range can grow for
    // reasons the yaw arm does not control, but the heading error is exactly
    // what it is trying to shrink.
    float attack_hdg_err_first{-1.0f};
    float attack_hdg_err_last{-1.0f};
    float attack_pitch_last{0.0f};
    // The torpedo bot task (kind Eh) this ordered aircraft runs, when its
    // class carries torpedo ordnance (kind 2Bh). docs/TORPEDO_TASK_ARM.md.
    // The task object itself is src/bot_tasks.cpp's; what lives here is the
    // per-slot state the arm 009D4850 reads and writes.
    bool torpedo_task_installed{false};
    bsp::TorpedoState torpedo_state{bsp::TorpedoState::kNone};
    int torpedo_state_ticks[8]{0, 0, 0, 0, 0, 0, 0, 0};
    int torpedo_arm_ticks{0};
    int torpedo_releases{0};
    int torpedo_transitions{0};
    // task+424h, the manual-release budget the arm's passthrough spends.
    int torpedo_rounds_pending{0};
    // prepare+98h (task+7D8h), the countdown 009D49A0 raises.
    float torpedo_drop_timer{-1.0f};
    // The values 009D3420 produces. docs/TORPEDO_APPROACH_UPDATE.md reads that
    // routine; the reconstruction in torpedo_approach_update.hpp runs here and
    // these mirror its outputs so the arm and the census read one set.
    bool torpedo_aim_flag_529{false};    // approach+131h
    bool torpedo_attack_flag_52a{false};  // approach+132h
    float torpedo_engage_range_8c{0.0f};  // task+484h == approach+8Ch
    float torpedo_engage_limit_90{0.0f};  // task+488h == approach+90h
    int torpedo_blocked_by_engaged{0};
    int torpedo_blocked_by_arm{0};
    // unit+C58h, the queued release-order count BSP_PilotBot_Tick 0099ACD0
    // spends at 0099AF81 by offering it to each task's vtable +24h. Its raiser
    // was not found: contract: unread, so it stays at zero here.
    int torpedo_release_orders_c58{0};
    int torpedo_arm_offers{0};
    int torpedo_arm_blocked_no_order_0099af53{0};
    // unit+C25h, the byte 007C0EE2 raises before it calls the issuer, and the
    // issue path's own bookkeeping. docs/TORPEDO_RELEASE_ORDERS.md.
    bool torpedo_release_pending_c25{false};
    int torpedo_orders_issued{0};
    int torpedo_orders_issue_ticks{0};
    int torpedo_peak_release_orders_c58{0};
    // The 007CE9FD stage's own two fields. docs/TORPEDO_ISSUE_TIMING.md.
    // unit+C28h, the interval countdown; 007D5D20 leaves it at the
    // constructor's zero, so the first fixed step expires it at once.
    float torpedo_issue_interval_c28{0.0f};
    // unit+C20h, the pending release-request count. 007BBC00 raises it by one
    // at the tail of 007BBBA0 BSP_Unit_RequestOrdnanceRelease; 007CEA82 spends
    // one per issue. 007D625B seeds it to zero (EBX from 007D619A XOR).
    int torpedo_issue_requests_c20{0};
    int torpedo_issue_stage_ticks{0};
    int torpedo_issue_stage_guard_blocked{0};
    int torpedo_issue_stage_waiting{0};
    int torpedo_issue_stage_issues{0};
    int torpedo_issue_stage_cleanups{0};
    int torpedo_issue_first_issue_tick{-1};
    int torpedo_release_requests_007bbba0{0};
    // unit+DECh, the plane's actuator block. Packet cc8_torpedo_release_spawn;
    // docs/TORPEDO_RELEASE_SPAWN.md. Built at 007EABC0 by
    // BSP_Plane_ReadPropertyBag and stepped by 007DE3A0. Channel C is the
    // ordnance bay 007BBBA0 drives.
    //
    // SUBSTITUTION, labelled: the native writer of the channel's enabled byte
    // at +60h and of its rate at +6Ch is unread, so channel C is enabled here
    // with a nominal 1.0 per second travel. The request forces the value to
    // 1.0f on the same frame regardless, so the rate only governs how long the
    // bay takes to close again.
    bsp::PlaneActuatorBlock actuator_block_dec{};
    int torpedo_bay_requests_accepted{0};   // 007BBBA0 moved the channel
    int torpedo_bay_requests_refused{0};    // a guard rejected it
    int torpedo_drops_spawned{0};           // a round actually left the plane
    // ctl+370h, the pilot control block's attack mode. 0099B740 raises it to
    // kAttack on the flight leader's cruise-profile tick; 009D3F60 row 1 holds
    // a task in prepare while it is kHold.
    bsp::PilotAttackMode torpedo_attack_mode_370{bsp::PilotAttackMode::kHold};
    int torpedo_attack_mode_raised_tick{-1};
    // ctl+370h's whole history. docs/TORPEDO_ATTACK_MODE.md: only three sites
    // in the image write this field on the pilot control block, and neither
    // route to 0 is one a torpedo task runs.
    int torpedo_mode_ticks[3]{0, 0, 0};       // ticks spent at hold/attack/forced
    int torpedo_mode_changes{0};
    int torpedo_mode_hold_with_engaged{0};    // ticks at 0 while engaged
    int torpedo_mode_lowered_009a285e{0};     // the closetoship countdown
    int torpedo_mode_message_007f0068{0};     // the BCh message arm
    int torpedo_prepare_entries{0};
    int torpedo_prepare_first_tick{-1};
    int torpedo_blocked_no_order_first_tick{-1};
    // The closetoship task's +550h/+43Ch pair and the BCh message, neither of
    // which this host produces. Held so the bound rules are exercised the
    // moment a producer appears.
    bool torpedo_closetoship_task_installed{false};
    float torpedo_closetoship_timer_550{0.0f};
    bool torpedo_closetoship_latch_43c{false};
    bool torpedo_pending_mode_message_bc{false};
    unsigned char torpedo_mode_message_payload{0};
    bool torpedo_is_flight_lead{false};
    // The sector scan of 009D3420, once it has a sampler.
    int torpedo_scans_run{0};
    int torpedo_clear_sectors_last{-1};
    int torpedo_home_sector_last{-1};
    float torpedo_turn_offset_last{0.0f};
    // The issue gate 007EEF40 compares, as the host last evaluated it.
    float torpedo_issue_threshold_390{0.0f};
    float torpedo_armed_fraction_374{0.0f};
    bool torpedo_issue_gate_open{false};
    // The approach object embedded at task+3F8h.
    bsp::TorpedoApproachState torpedo_approach{};
    int torpedo_approach_ticks{0};
    int torpedo_approach_no_target_ticks{0};
    int torpedo_approach_replans{0};
    int torpedo_aim_ticks{0};
    // The first tick on which the engaged pair went non-zero, or -1.
    int torpedo_first_engaged_tick{-1};
    float torpedo_range_min{-1.0f};
    float torpedo_range_last{0.0f};
    float torpedo_aim_heading_last{0.0f};
    float torpedo_aim_throttle_last{0.0f};
    int torpedo_attackrun_altitude_commands{0};
    // 009D236E, the aim-complete byte state+2Ch, and the clause values on the
    // tick it first went true. docs/TORPEDO_AIM_TICK.md.
    bool torpedo_aim_complete_2c{false};
    int torpedo_aim_complete_tick{-1};
    int torpedo_aim_complete_clause{0};   // 1 = range, 2 = turn, 3 = both
    float torpedo_aim_f34_threshold{0.0f};
    float torpedo_aim_f14_range{0.0f};
    float torpedo_aim_f18_turn{0.0f};
    float torpedo_aim_f0c_time{0.0f};
    float torpedo_aim_ramp{0.0f};
    float torpedo_aim_floor{0.0f};
    // 009D0D90, the goaway enter: the break-off distance 009D3150 compares the
    // range against, and the alternating side byte. docs/TORPEDO_GOAWAY_RELEASE.md.
    float torpedo_goaway_distance_24{0.0f};
    float torpedo_goaway_side_2c{1.0f};
    int torpedo_goaway_enters{0};
    bool torpedo_goaway_done_last{false};
    float torpedo_goaway_range_peak{-1.0f};
    int torpedo_aim_run_time_updates{0};   // 009D19A4
    int torpedo_aim_release_arms{0};       // 009D2287
    float torpedo_aim_timer{0.0f};         // 009D2027, 009FA3A0(state+18h, dt)
    // The rest of the state+18h release timer. 009D2287 enables it with the
    // countdown at zero, 009FA3A0 counts it down and calls 007BBBA0 at
    // 009FA3D0. docs/TORPEDO_FIRST_RELEASE.md.
    bool torpedo_aim_timer_enabled_0c{false};
    int torpedo_aim_timer_fires{0};
    int torpedo_aim_timer_blocked_007bb110{0};
    int torpedo_aim_timer_first_fire_tick{-1};
    // plan+2C0h / +2CCh. docs/PILOT_TASK_HEADING_ARM.md: the arm writes the
    // bearing and mode 2, and a bot state's tick may overwrite the pair. The
    // torpedo aim tick does at 009D1D16 / 009D1D1E.
    float plan_heading_2c0{0.0f};
    int plan_heading_mode_2cc{0};
    bool plan_heading_2c0_written{false};
    float plane_live_throttle{1.0f};
    float plane_live_air_brake{0.0f};
    float plane_latched_controls[3]{0.0f, 0.0f, 0.0f};
    // The pilot bot's five plan slots (plan+274h, stride 0Ch) and the think
    // accumulator 0099ACD0 keeps at bot+70h. The tick gates on the accumulator
    // reaching 0.09 s and then passes the ACCUMULATED interval down, not the
    // frame delta - docs/PILOT_BOT_TICK_GATES.md.
    bsp::PilotPlanSlot plan_slots[5]{};
    float pilot_think_accumulator_70{0.0f};
    // unit+9FCh..+A10h, the pending command block, and the byte at unit+A14h
    // that 007B8C90 sets and 007BB920 clears.
    float pilot_command_block[5]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bool pilot_command_pending_a14{false};
    // The unit this one's current command names as its target, plus one, or 0
    // for none. The gunnery host resolves the command row's target token against
    // the unit table and pushes it here, because the plane's control path needs
    // the same answer the weapon director already has rather than a second
    // resolution of its own.
    std::size_t command_target_plus_one{0};
    // The plane row's rate and acceleration keys, read once at creation.
    bsp::PlaneControlClass plane_class;
    // desc+184h StallSpd, the divisor the control authority ramp uses. The
    // PlaneFreeFlightClass default stands in when the row does not carry it.
    float plane_stall_spd{17.5f};
    // desc+174h XDrag and desc+170h YDrag, the two body-frame damping
    // coefficients 007DBD3A and 007DBD50 multiply the body lateral and body
    // vertical velocity by. PlaneFreeFlightClass declares them 0.0f and this
    // host never filled them, which switched off the ONLY term that turns a
    // plane's velocity onto its nose. docs/TORPEDO_RUN_IN_VELOCITY.md.
    float plane_x_drag{0.0f};
    float plane_y_drag{0.0f};
    // desc+188h MaxSpd, the numerator of 009F9D30's run-profile speed ratio.
    float plane_max_spd{0.0f};
    // desc+18Ch TravelSpeed, the airspeed 007C6340 seeds a plane with.
    float plane_travel_speed{0.0f};
    // desc+164h Accel, desc+208h GlideRate, desc+1D4h DragPitchRatio and
    // desc+1DCh AirBrakeDrag: the four authored fields the thrust 007D9050 and
    // the drag 007D9140 are built from. docs/PLANE_POSE_THROTTLE_ALTITUDE.md.
    float plane_accel{0.0f};
    float plane_glide_rate{0.0f};
    float plane_drag_pitch_ratio{0.0f};
    float plane_air_brake_drag{0.0f};
    // desc+1F0h DropAngle, 009FB800's dive gain and cap.
    float plane_drop_angle{0.0f};
    // desc+1E4h and desc+1ECh, the two 007C4850 derives with the 007D98F0
    // climb-angle solver at 007C4BE9 and 007C4C14. desc+1ECh is the climb arm's
    // gain, and it is 0.6 * desc+1E4h, NOT zero: docs/PLANE_FLIGHT.md read it as
    // having no producer because it is derived rather than authored.
    float plane_climb_angle_1e4{0.0f};
    float plane_climb_angle_1ec{0.0f};
    // desc+194h SwimHeight, one of the two terms of the free-flight arm's water
    // line at 007CC4E8.
    float plane_swim_height{0.0f};
    int plane_water_contacts{0};
    // The altitude 009FBA50 was last commanded with, and the pitch 009FB800
    // answered, kept for the census only.
    float plane_commanded_altitude{-1.0f};
    float plane_commanded_pitch{0.0f};
    // unit+0BBCh and unit+0BB0h+10h, the latched throttle and air brake. The
    // latch 007B9783 copies the whole live block, not just the three stick
    // axes, and both of these are drag or thrust inputs.
    float plane_latched_throttle{1.0f};
    float plane_latched_air_brake{0.0f};
    // The union of this unit's guns' projectile descriptor answer sets, stored
    // by the gunnery host at load. docs/ORDNANCE_KIND_IDENTITY.md.
    std::uint64_t ordnance_mask{0};
    volatile float generic_input_63c{1.0f};   //0095CD9E

    // The pose the canonical projection borrows: +74h local, +C8h valid, +CCh
    // world, +10Ch derived-valid, with no parent because every entity of this
    // mission is top level (docs/GAME_EXECUTABLE.md milestone 2h, correction 8).
    bsp::CameraMatrix local{};
    bsp::CameraMatrix world{};
    std::uint8_t world_valid{1};
    std::uint8_t derived_valid{1};
    bsp::PoseRefreshView* parent{nullptr};
    std::unique_ptr<bsp::PoseRefreshView> pose;
    std::unique_ptr<bsp::UnitInstanceState> state;
    // Retained cells absent from UnitInstanceState. Its active/simulate cells
    // remain the only owners of+5C/+5D. Constructor00925CE0 stores BL=0 at
    //00925E11/+5E,00925E0B/+5F,00925E08/+60; later stores use this same slot.
    std::uint8_t scene_destroyed_005e{0};
    std::uint8_t scene_removed_005f{0};
    std::uint8_t scene_pending_destroy_0060{0};
    bool scene_flags_available{false}; // process provenance, not a native byte
    bsp::UnitClassBlock class_block{};

    // The motion half.
    bsp::ShipMotionState motion{};
    bsp::ShipMotionClass motion_class{};
    bsp::UnitHullExtents hull_extents{};
    float class_width_00a4{};
    bsp::ShipClassFields fields{};
    bsp::UnitOrderRing ring{};
    bsp::UnitOrderQueue queue{};
    bsp::UnitOrderRecordStorage scratch{};
    bsp::UnitControllerState controller{};

    // The hull's rigid body. `motion_state` is M = *(B+4h) and `body` is B, which
    // the unit's force-model controller holds at controller+2Ch. Milestone 2r
    // builds both through the tail of 00937C90 (docs/SHIP_HULL_BODY.md), so the
    // mass, the box inertia, the row-1 torque lock, both damping rates and both
    // speed clamps are the game's own; `hull_material` is the record 00937CF1
    // selects for this hull.
    bsp::DynMotionState motion_state{};
    bsp::DynBody body{};
    bsp::ShipPhysicsMaterial hull_material{bsp::ShipPhysicsMaterial::kShip};

    // Milestone 2s: what 009329C0 reads. The physics-material record is the row
    // settings+4E0h + material*38h that the hull body already selected, and the
    // element list is class+52Ch..+530h.
    //
    // The element list is a STAND-IN, not the game's own: its producer writes
    // class+528h..+534h and no function in the exported set does that, so
    // docs/SHIP_HYDRO_FORCES.md leaves the field roles as a hypothesis and its
    // follow-up `ship_buoyancy_element_producer` owns the question. The list
    // built here is the same one the probe builds under --hydro, from the class
    // row's own Length, Height and Mass, so the two sides are comparable.
    std::vector<bsp::ShipBuoyancyElement> buoyancy_elements;
    // unit+10FCh, the leak model's total accumulated water. 0074F930 writes it
    // and 009329C0 adds it to the hull mass at 00932B78; an undamaged hull
    // holds it at zero.
    float leak_water_mass_10fc{0.0f};

    // unit+C4h is the most-derived instance class selected by VehicleClass.Type.
    // -1 means its identity is unresolved; it is not a native class-id stamp.
    int class_id{bsp::kVehicleClassKindUnknown};
    // Milestone 2p: the two load latches the middle of 009F3F80 raises with the
    // inlined bodies of 009D4FB0 (unit+102Ch) and 009D4FE0 (unit+1034h). Their
    // consumers are not in this process; the fields exist so the raise is a
    // real store rather than a discarded call, and the report prints them.
    float turn_assist_load_102c{0.0f};
    float secondary_load_1034{0.0f};
    bool standing_order{false};
    float standing_throttle{0.0f};
    float standing_rudder{0.0f};
};

struct GameUnitsHost::Impl {
    Impl(GameHostLog& log_in, GameMissionLuaHost& lua_in)
        : log(log_in), lua(lua_in), commands(log_in) {}

    GameHostLog& log;
    GameMissionLuaHost& lua;
    GameObserverRuntime* observer_runtime{nullptr}; // application-owned, borrowed
    // Milestone 2l: the weapon director of every created unit, and the three
    // message hops between the authored `Command` token and its command slot.
    GameCommandsHost commands;
    std::vector<std::unique_ptr<GameUnitSlot>> slots;
    // A flat copy of the rows, rebuilt on demand so units() can hand the caller
    // one contiguous table without exposing the slots.
    mutable std::vector<GameUnitRow> rows;
    GameUnitsSummary summary{};
    // 00e188d8, the controlled unit. Held as an index because this process owns
    // no unit pointers; `bound` is the `!= 0` every reader of the global makes.
    bool controlled_bound{false};
    std::size_t controlled_index{0};
    // 00424c40 is a singleton, so the curve block is one table shared by every
    // ship in the mission, not a per-unit or per-class tuning. It is filled once
    // by load_gameplay_settings_0083b5e0; until then it carries the zeroes a
    // freshly allocated settings object has, which is why the load runs first.
    bsp::UnitRudderCurveSettings rudder_curve{};
    bool rudder_curve_loaded{false};
    // Milestone 2p: the seven AutoThrust keys of the same singleton, which
    // 009EC7C0 reads at +6CCh..+6ECh. Loaded beside the turn multipliers.
    bsp::ShipAiAutoThrustSettings auto_thrust{};
    bool auto_thrust_loaded{false};

    // The world fields 00c41550 and 00c5b1b0 read: gravity at world+04h..+0Ch,
    // the two sleep speed thresholds at world+3Ch/+40h and the countdown reload
    // at world+44h.
    //
    // Milestone 2r left these at zero on the reading that none of them had a
    // recovered producer. They do: 004DDB90 builds the world descriptor on its
    // own stack and 00C41AD0 copies it into the world, and packet
    // `dyn_world_settings` read both whole (docs/DYN_WORLD_SETTINGS.md). The
    // values are the game's own: gravity (0, -10, 0) from the double at
    // 00CE6848, both sleep speeds the zero 004DE1C7 / 004DE1CD store, so no
    // body ever sleeps.
    //
    // Gravity is switched on here and not earlier because it only balances once
    // 009329C0 runs: the buoyancy the element list produces at the hull's draft
    // is exactly mass * 10, which is what cancels 00C41550's gravity * dt. The
    // two belong to the same step and neither is correct alone.
    bsp::DynWorldStepConstants physics_world{
        bsp::dyn_world_step_constants(bsp::dyn_world_settings_game())};

    // The notes are logged once per kind, not once per unit tick.
    bool logged_ocean{false};
    bool logged_scale{false};
    bool logged_curve{false};
    bool logged_integrator{false};
    bool logged_cruise{false};
    bool logged_hydro{false};

    // Milestone 2s: the world registry's per-class unit lists, the 97 triples
    // at [[00E188A8]+19CCh] + 18h + id*0Ch that 004CB076's vector-constructor
    // iterator builds inside 004CB030 BSP_World_Construct. Each triple is
    // {count, head, tail};00484540 appends stable nodes holding direct unit
    // identities. The registry remains the same mission-owned object.
    // Only the ids a unit's +130h override joins are ever non-empty here.
    static constexpr int kWorldListCount = static_cast<int>(bsp::kWorldSlotCount);
    GameUnitWorldLists world_lists;

    // 00484540, __thiscall void(list, void* value), RET 4. The node is
    // {prev, next, value}; the empty branch at 00484586 writes the head and the
    // other at 00484572 chains from the tail, and both set the tail and
    // ADD dword ptr [ESI],1. Consumers can retain a node and reload its next
    // link after a callback, as the native candidate loop does.
    void world_list_push_back_00484540(GameUnitWorldLists& parent,
        std::uint32_t offset, GameUnitSlot& unit) {
        if (offset < bsp::kWorldSlotArrayOffset
            || (offset - bsp::kWorldSlotArrayOffset) % bsp::kWorldSlotStride != 0
            || (offset - bsp::kWorldSlotArrayOffset) / bsp::kWorldSlotStride
                >= bsp::kWorldSlotCount) {
            throw std::logic_error("invalid recovered world list offset");
        }
        GameUnitWorldList& list = parent.entries[
            (offset - bsp::kWorldSlotArrayOffset) / bsp::kWorldSlotStride];
        //00484546 allocates12;54..59 zero words,66 payload,6C previous;
        //75 old-tail next or86 head, then tail/next/count stores.
        auto* node = new GameUnitWorldNode{};
        node->unit = &unit;
        node->previous = list.tail;
        if (list.count != 0) list.tail->next = node;
        else list.head = node;
        list.tail = node;
        node->next = nullptr;
        ++list.count;
        ++summary.world_list_pushes;
        done("UnitList::push_back", 0x00484540u);
    }

    //009288F1 dispatches the actual leaf+130h registrar after placement.
    // All21 recovered creators first join parent list1 through00928560;
    // their remaining lists differ. Reload the actual parent at every push.
    void register_in_world_lists(GameUnitSlot& unit) {
        const auto* dispatch = bsp::unit_world_registration_for_creator(
            unit.motion_dispatch.creator);
        if (dispatch == nullptr) {
            ++summary.world_registration_unavailable;
            record_slot("UnitInstance::unresolved_world_registration", "unit/vtable+130h");
            return;
        }
        class Calls final : public bsp::UnitWorldRegistrationHost {
        public:
            explicit Calls(Impl& owner) : owner_(owner) {}
            void* parent_0030(void* value) override {
                return static_cast<GameUnitSlot*>(value)->world_parent_0030;
            }
            void push_back_00484540(void* parent, std::uint32_t offset,
                void* value) override {
                if (parent == nullptr) throw std::logic_error("unplaced unit registration");
                owner_.world_list_push_back_00484540(
                    *static_cast<GameUnitWorldLists*>(parent), offset,
                    *static_cast<GameUnitSlot*>(value));
            }
        private:
            Impl& owner_;
        } calls(*this);
        if (bsp::register_unit_world_lists_for_creator(calls,
                unit.motion_dispatch.creator, &unit)) {
            ++summary.world_registrations;
            done("GameEntity::register_in_parent_entity_list", 0x00928560u);
            // Keep the actual registrar in the method key: logging otherwise
            // groups all derived routines under the first routine's address.
            char name[72];
            std::snprintf(name, sizeof(name), "UnitInstance::world_registration_%08lx",
                static_cast<unsigned long>(dispatch->native_entry));
            done(name, dispatch->native_entry);
            log.notef("unit world registration: unit=%s creator=%08lx primary=%08lx entry=%08lx lists=%zu",
                unit.row.name.c_str(), static_cast<unsigned long>(dispatch->creator),
                static_cast<unsigned long>(dispatch->primary_vtable),
                static_cast<unsigned long>(dispatch->native_entry), dispatch->list_count);
        }
    }
    bool logged_gate{false};
    bool logged_precision{false};

    // Milestone 2n: the ship AI controller that publishes into each unit's own
    // 84-byte AI order slot, which the motion head at 00825f2c promotes.
    GameShipAiHost* ship_ai{nullptr};

    // Milestone 2t: the gun chain, one object per run. It owns the gunnery pass
    // at unit+6DCh of every created unit, the guns the authored `Platforms`
    // table produces, the projectiles in flight and the hit path behind them.
    std::unique_ptr<GameGunneryHost> gunnery;
    // docs/AI_COORDINATOR_TICK.md. 00A32350 BSP_AiController_Create is called
    // from BSP_Game_LoadMissionScene at 004E1838, and its 00A31730 constructor
    // registers the tick element into fixed-step group 0 at 00A31766.
    std::unique_ptr<GameAiCoordinatorHost> ai;

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void record_motion_phase(const char* phase, std::uint32_t entry) {
        // GameHostLog aggregates by method name, so retain each native entry
        // in that key as well as in the evidence-address column.
        char method[96];
        std::snprintf(method, sizeof(method), "UnitMotion::%s_%08lx", phase,
            static_cast<unsigned long>(entry));
        record(method, entry);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }
    // An indirect dispatch has no call-site address of its own; the slot is the
    // evidence, so the report carries `<vtable>+vtableNN` for it.
    // 007BBBA0 BSP_Unit_RequestOrdnanceRelease. Two call sites in this host
    // reach it - the torpedo task's manual passthrough and the aim state's
    // release timer - and both must raise unit+C20h, because 007BBC00 does so
    // on every path of that routine. docs/TORPEDO_ISSUE_TIMING.md,
    // docs/TORPEDO_FIRST_RELEASE.md.
    void release_ordnance_007bbba0(GameUnitSlot& slot) {
        // 007BBBA0 itself: the three guards and the writes onto channel C of the
        // block at unit+DECh. It is a bay-open command, not a spawn; nothing in
        // the block or its tick 007DE3A0 reads ordnance or makes a projectile.
        // docs/TORPEDO_RELEASE_SPAWN.md.
        //
        // The third guard, the per-slot byte at unit+9C3h[[00F876B8]*8], is the
        // caller's because it lives on the unit rather than the block. This host
        // has no such byte, so it is NOT applied and the refusal count below
        // therefore under-counts what the image would refuse.
        if (!slot.actuator_block_dec.channel_c.enabled) {
            slot.actuator_block_dec.channel_c.enabled = true;
            slot.actuator_block_dec.channel_c.rate = 1.0f;
        }
        const bool accepted =
            bsp::ordnance_release_request_007bbba0(slot.actuator_block_dec);
        done("Unit::request_ordnance_release_007bbba0", 0x007bbba0u);
        if (accepted) {
            ++slot.torpedo_bay_requests_accepted;
            // SUBSTITUTION, labelled. The native chain from here is
            // 007BBBA0 -> unit+C20h -> 007CE040's stage at 007CEA82 -> 007C0D90
            // -> BSP_PilotControl_IssueReleaseOrders 007EEF30 ->
            // BSP_Unit_SetQueuedReleaseOrders 007BCBE0 -> unit+C58h, which
            // BSP_PilotBot_Tick spends one of at 0099AFB6 after a bot task's
            // vtable[24h] answers. That task's release slot, and the bomb
            // platform's own release in vtable 00CFE308, are UNREAD, so the
            // spawn is not reproduced from its own site. What IS established is
            // that a bomb platform is a Gun subclass (00730B80 calls
            // BSP_Gun_Construct at 00730B88), so the round is handed to the gun
            // spawn 0072F830 the gunnery host already implements.
            if (gunnery != nullptr) {
                const std::size_t index = index_of_slot(slot);
                if (index < slots.size() && gunnery->release_ordnance_drop(index)) {
                    ++slot.torpedo_drops_spawned;
                }
            }
        } else {
            ++slot.torpedo_bay_requests_refused;
        }
        // The release instant, which is the sample the water-entry arithmetic
        // is built on. Printed for the first four releases only, like the
        // gunnery host's own drop line.
        if (slot.torpedo_releases < 4) {
            float vn[3];
            velocity_versus_nose(slot, vn);
            log.notef("release census: unit=%s alt=%.1f m |v|=%.2f m/s "
                "angle_to_nose=%.1f deg body_fwd_0092d730=%.2f m/s "
                "travel_spd=%.2f x_drag=%.2f y_drag=%.2f max_spd=%.2f",
                slot.row.name.c_str(),
                static_cast<double>(slot.motion.position[1]),
                static_cast<double>(vn[0]),
                static_cast<double>(vn[1]) * 180.0 / kPi,
                static_cast<double>(vn[2]),
                static_cast<double>(slot.plane_travel_speed),
                static_cast<double>(slot.plane_x_drag),
                static_cast<double>(slot.plane_y_drag),
                static_cast<double>(slot.plane_max_spd));
        }
        ++slot.torpedo_releases;
        ++slot.torpedo_release_requests_007bbba0;
        slot.torpedo_issue_requests_c20 =
            bsp::release_request_raise_007bbc00(slot.torpedo_issue_requests_c20);
    }

    // No slot carries its own index, and the release request is rare, so the
    // lookup is a scan rather than a new field on every slot.
    std::size_t index_of_slot(const GameUnitSlot& slot) const {
        for (std::size_t i = 0; i < slots.size(); ++i) {
            if (slots[i].get() == &slot) return i;
        }
        return slots.size();
    }

    void record_slot(const char* method, const char* text) { log.unimplemented(method, text); }

    bool is_controlled(const GameUnitSlot& slot) const {
        return controlled_bound && controlled_index < slots.size()
            && slots[controlled_index].get() == &slot;
    }

    // The +CCh pose block and the motion state are two views of the same rows;
    // the motion path writes them, the pose refresh reads them.
    static void publish_pose(GameUnitSlot& slot) {
        for (int i = 0; i < 3; ++i) {
            slot.world[static_cast<std::size_t>(i)] = slot.motion.pose_row0[i];
            slot.world[static_cast<std::size_t>(4 + i)] = slot.motion.pose_row1[i];
            slot.world[static_cast<std::size_t>(8 + i)] = slot.motion.pose_row2[i];
            slot.world[static_cast<std::size_t>(12 + i)] = slot.motion.position[i];
        }
        slot.world[3] = 0.0f;
        slot.world[7] = 0.0f;
        slot.world[11] = 0.0f;
        slot.world[15] = 1.0f;
        slot.local = slot.world;
    }

    void refresh_row(GameUnitSlot& slot);
    void issue_into_ring(GameUnitSlot& slot, float throttle, float rudder);

    // The heading 00835ac0 latches. The unit virtual at primary slot 50h is a
    // RET 0 getter with no reconstruction, so this is the executable's own
    // value: atan2 over pose row 2, the same convention the trajectory dump and
    // the run log print in degrees.
    // The three numbers docs/TORPEDO_RELEASE_GEOMETRY.md section 3 asked for
    // and did not take: |v|, the angle between v and the forward pose row, and
    // the body-axis speed 0092D730 itself computes (the dot of the body's
    // linear velocity from 00C31F40 with row 3 of the axis matrix 00C32000).
    // out[0] = magnitude, out[1] = angle in radians, out[2] = body-axis speed.
    static void velocity_versus_nose(const GameUnitSlot& slot, float out[3]) {
        const float v[3] = {slot.motion.linear_velocity.x,
            slot.motion.linear_velocity.y, slot.motion.linear_velocity.z};
        const float* const fwd = slot.motion.pose_row2;
        const float mag = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        const float fwd_len =
            std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] + fwd[2] * fwd[2]);
        const float dot = v[0] * fwd[0] + v[1] * fwd[1] + v[2] * fwd[2];
        out[0] = mag;
        out[2] = dot;   // 0092D730's own answer
        if (mag > 1e-6f && fwd_len > 1e-6f) {
            float c = dot / (mag * fwd_len);
            if (c > 1.0f) c = 1.0f;
            if (c < -1.0f) c = -1.0f;
            out[1] = static_cast<float>(std::acos(static_cast<double>(c)));
        } else {
            out[1] = 0.0f;
        }
    }

    static float pose_heading_radians(const GameUnitSlot& slot) {
        return static_cast<float>(std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
            static_cast<double>(slot.motion.pose_row2[2])));
    }
};

namespace {

// ---------------------------------------------------------------------------
// 0078cf20's two leaves, and the gameplay-modifier filter
// ---------------------------------------------------------------------------

// bsp::OceanHeightHost. The sampler itself is recovered and runs; what is not
// available is its receiver. 0078cf20 takes [[00e188a8]+19F0h] and hands
// [world+A8h] to both leaves, and that sub-object is the renderer/scene owner's
// (the foliage builder 00af0c50 takes its camera from the same pointer, and
// `construct_world` 004de610 is still a load record), so each leaf is a host
// record that answers the open-sea state docs/OCEAN_HEIGHT.md evidences: the
// wave field returns exactly 0.0f when field+F9h is set or field+24h is zero,
// and the coverage mask returns exactly 1.0f when no region covers the point.
class OceanFieldBinding final : public bsp::OceanHeightHost {
public:
    explicit OceanFieldBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}

    float wave_height_0078c890(float, float) override {
        if (!owner_.logged_ocean) {
            owner_.logged_ocean = true;
            owner_.log.notef("ocean sampler 0078cf20 runs, both of its leaves are records: "
                "its receiver is [[00e188a8]+19F0h] and both calls take [world+A8h], the "
                "renderer/scene owner's field object, so the wave field answers its own "
                "disabled value 0.0f and the coverage mask its own open-sea value 1.0f, and "
                "the product 0078cf64 is exactly 0.0f");
        }
        owner_.record("ShipMotion::ocean_wave_field", 0x0078c890u);
        return 0.0f;
    }
    float coverage_mask_00b9cf50(float, float) override {
        owner_.record("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
        return 1.0f;
    }

private:
    GameUnitsHost::Impl& owner_;
};

// bsp::GameplayModifierHost. 008e6430 walks the list at manager+80h+category*0Ch
// and this process registers no modifier record, so the walk visits nothing and
// the filter is never reached; the product is the routine's own 1.0f at
// 00d7a24c rather than a literal a host returns.
class NoGameplayModifiers final : public bsp::GameplayModifierHost {
public:
    bool entry_matches_008e4680(const bsp::GameplayModifierEntry&) override { return false; }
};

// ---------------------------------------------------------------------------
// bsp::UnitInstanceHost, one method per native call site of 008255b0
// ---------------------------------------------------------------------------

class UnitInstanceBinding final : public bsp::UnitInstanceHost {
public:
    UnitInstanceBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 00b6db70 on the scene node at +4A4h. 00928860, the placement that would
    // give a created unit a scene node, is a milestone 2h record, so the state
    // carries has_scene_node = false and this is unreachable; it is here because
    // the interface has no default.
    float refresh_scene_node_world_matrix() override {
        owner_.record("UnitInstance::refresh_scene_node", 0x00b6db70u);
        return 0.0f;
    }
    void release_submerged_effect(std::size_t) override {
        owner_.record("UnitInstance::release_submerged_effect", 0x00867b10u);
    }
    // The ocean object's own world Y, which step 2 of 008255b0 gates on. That is
    // the object's pose, not a sample of the sampler, and its producer is the
    // renderer/scene owner; a surface at y = 0 leaves the bubble block alone,
    // which is why it is a stated contract rather than a claim.
    bool ocean_exists() override { return true; }
    float ocean_world_y() override { return 0.0f; }
    float draw_bubble_interval() override {
        owner_.record("UnitInstance::draw_bubble_interval", 0x00bd2f10u);
        return 0.0f;
    }
    void spawn_bubble_effect() override {
        owner_.record("UnitInstance::spawn_bubble_effect", 0x008687c0u);
    }
    // 0092d730 on the controller at +1018h: the body-axis forward speed, which
    // is reconstructed, so this is the recovered value rather than a record.
    float controller_time_value() override {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        owner_.done("UnitInstance::controller_body_axis_speed", 0x0092d730u);
        return bsp::unit_forward_speed_0092d730(in);
    }
    // 00815aa0 is reconstructed as publish_unit_effect_intensity_00815aa0, but
    // it publishes into the effect groups at unit+FFCh, and a created unit has
    // none: 006fe590, the instance the descriptor's vtable +28h allocates, is a
    // milestone 2h record and nothing fills those groups.
    void sub_update_00815aa0(float) override {
        owner_.record("UnitInstance::publish_effect_intensity", 0x00815aa0u);
    }
    bsp::UnitAnchorPoint transform_bow_anchor() override {
        owner_.record("UnitInstance::transform_bow_anchor", 0x00413920u);
        return {};
    }
    bsp::UnitAnchorPoint transform_stern_anchor() override {
        owner_.record("UnitInstance::transform_stern_anchor", 0x00413920u);
        return {};
    }
    // The same 0078cf20 the motion tick's wave gate calls, run whole.
    float sample_ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("UnitInstance::ocean_height", 0x0078cf20u);
        return height;
    }
    void publish_bow_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_bow_anchor", 0x004842c0u);
    }
    void publish_stern_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_stern_anchor", 0x004842c0u);
    }
    // 0092be80 on the controller, whose whole recovered body is one RET
    // (docs/UNIT_CONTROLLER_UPDATE.md). Running it is the point: the negative
    // result is what the reconstruction records.
    void sub_update_0092be80(float scaled_delta) override {
        bsp::unit_controller_update_0092be80(slot_.controller, scaled_delta);
        owner_.done("UnitInstance::controller_update", 0x0092be80u);
    }
    void sub_update_0081c050() override {
        owner_.record("UnitInstance::sub_update_0081c050", 0x0081c050u);
    }
    bool is_controlled_unit() override { return owner_.is_controlled(slot_); }
    bool global_intensity_override() override { return false; }  // 00f87152
    void smooth_intensity_008227e0(float) override {
        owner_.record("UnitInstance::smooth_intensity", 0x008227e0u);
    }
    bool wake_enabled() override {
        owner_.record("UnitInstance::wake_setting", 0x00424c40u);
        return false;
    }
    void spawn_wake() override { owner_.record("UnitInstance::spawn_wake", 0x00935540u); }
    bool prop_wash_threshold_passed(float) override {
        owner_.record("UnitInstance::prop_wash_threshold", 0x00ce69d0u);
        return false;
    }
    void stop_prop_wash() override {
        owner_.record("UnitInstance::stop_prop_wash", 0x00b6da70u);
    }
    void update_attachment(std::size_t, const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::update_attachment", 0x008689c0u);
    }
    // The three timed sub-updates of step 11. docs/UNIT_TIMED_SUBUPDATES.md
    // analyses them; packet cc_unit_subupdates owns their reconstruction.
    void sub_update_008252c0(float) override {
        owner_.record("UnitInstance::update_engine_audio", 0x008252c0u);
    }
    void sub_update_00956600(float) override {
        owner_.record("UnitInstance::update_unit_timers", 0x00956600u);
    }
    void sub_update_00834e90(float) override {
        owner_.record("UnitInstance::update_propellers", 0x00834e90u);
    }
    void tick_part(std::size_t, float) override {
        owner_.record("UnitInstance::tick_part", 0x00815370u);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitRudderHost, the yaw-rate curve 00811890 reads
// ---------------------------------------------------------------------------

class UnitRudderBinding final : public bsp::UnitRudderHost {
public:
    UnitRudderBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 0082e890 calls 00424c40 four times on the low branch and three on the
    // high one and reads +438h..+44Ch off each returned pointer. The settings
    // object is the singleton the Lua-driven loader 0083b5e0 filled, so this
    // hands back the one shared block rather than a per-unit copy.
    const bsp::UnitRudderCurveSettings& settings_00424c40() override {
        owner_.done("ShipMotion::rudder_curve_settings", 0x00424c40u);
        return owner_.rudder_curve;
    }
    bool scale_manager_present() override { return false; }
    const bsp::ShipClassFields& ship_class() override { return slot_.fields; }
    bool gameplay_scale_enabled() override { return false; }
    bool scale_manager_enabled() override { return false; }
    float gameplay_scale_008e6430(int) override {
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }
    float turn_efficiency() override { return slot_.motion.turn_efficiency; }
    float forward_speed_0092d730() override { return forward_speed; }
    float steering_command() override { return slot_.motion.to_turn; }

    float forward_speed{0.0f};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitOrderRecordIssueHost, 00816a40's network half
// ---------------------------------------------------------------------------

class UnitOrderIssueBinding final : public bsp::UnitOrderRecordIssueHost {
public:
    explicit UnitOrderIssueBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    int session_mode() override { return 1; }  // game+1FE4h, single player
    bsp::UnitOrderMessageStorage construct_message_0075b430(int) override {
        owner_.record("UnitOrder::construct_message", 0x0075b430u);
        return bsp::UnitOrderMessageStorage{};
    }
    void send_message_0077c2a0(const bsp::UnitOrderMessageStorage&, std::uint32_t,
        std::uint32_t) override {
        owner_.record("UnitOrder::send_message", 0x0077c2a0u);
    }

private:
    GameUnitsHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::ShipHydroHost, one method per call site of 009329C0
// ---------------------------------------------------------------------------

// The hull's hydrodynamics, run where 00937440 makes its last call, 00937622.
// docs/SHIP_HYDRO_FORCES.md.
//
// Eleven of the twelve methods are calls. The exceptions and the qualifications:
//
//  * unit_category_8_vtable5c is the only record. Its callee is the unit's own
//    vtable slot 5Ch and no packet has read that body
//    (docs/SHIP_HYDRO_FORCES.md follow-up `unit_vtable_5c`), so the answer is
//    substituted from the recovered IsKindOf table and the slot is reported as
//    the evidence instead of a call site;
//  * the leak model at unit+10D4h has no leak points, because nothing in this
//    process damages a hull, so 0074F930's tick and 0074F2E0's heeling torque
//    both run over an empty entry list. They are calls, not records: the
//    reconstruction in bsp/unit_forces.hpp is what answers them;
//  * the disabled path's three effects (00C37E50, 00C37E20 and the OR of bit 2
//    into body+50h) are wired to the body but never reached, because
//    controller+14h is clear on a live hull.
class ShipHydroBinding final : public bsp::ShipHydroHost {
public:
    ShipHydroBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    bsp::OceanVec3 body_linear_velocity_00c31f40() override {
        owner_.done("ShipHydro::body_linear_velocity", 0x00c31f40u);
        return slot_.motion.linear_velocity;
    }
    bsp::OceanVec3 body_angular_velocity_00c31f20() override {
        owner_.done("ShipHydro::body_angular_velocity", 0x00c31f20u);
        return slot_.motion.angular_velocity;
    }
    // 00C32000 hands back body+8h and 00C33650 expands that 3x4 into the 4x4
    // whose rows are the body axes. The pose the motion state carries is the
    // same one the position phase 00C5B1B0 wrote into the body last step.
    bsp::ShipHydroTransform body_world_transform_00c33650() override {
        bsp::ShipHydroTransform m{};
        m.row0 = {slot_.motion.pose_row0[0], slot_.motion.pose_row0[1],
            slot_.motion.pose_row0[2]};
        m.row1 = {slot_.motion.pose_row1[0], slot_.motion.pose_row1[1],
            slot_.motion.pose_row1[2]};
        m.row2 = {slot_.motion.pose_row2[0], slot_.motion.pose_row2[1],
            slot_.motion.pose_row2[2]};
        m.position = {slot_.motion.position[0], slot_.motion.position[1],
            slot_.motion.position[2]};
        owner_.done("ShipHydro::body_world_transform", 0x00c33650u);
        return m;
    }
    // 00932A42 and 00932E2F, the unit's vtable slot 5Ch with the literal 8. The
    // compiled predicate belongs to the loaded instance class, as at the
    // hull-body call 00937CFD. docs/GAME_UNIT_KIND_BINDING.md.
    bool unit_category_8_vtable5c() override {
        const bsp::UnitKindClassBody* body = bsp::unit_kind_body_for_class(slot_.class_id);
        if (body != nullptr) {
            // GameHostLog groups by method name; keep distinct vtable owners
            // distinct so the first unit cannot label every later call.
            char method[64];
            std::snprintf(method, sizeof(method), "ShipHydro::unit_category_8_class_%02x",
                static_cast<unsigned int>(slot_.class_id));
            owner_.done(method, body->test_address);
        } else {
            owner_.record("ShipHydro::unit_category_8_unresolved_identity", 0x00932a42u);
        }
        return bsp::unit_is_kind_of(slot_.class_id, bsp::kUnitForceSubmarineClassId);
    }
    float water_height_0078cf20(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("ShipHydro::water_height", 0x0078cf20u);
        return height;
    }
    // 00933A01. An undamaged hull has no leak points, so the tick runs over a
    // count of zero and leaves the unit+10FCh it already read at zero.
    void leak_tick_0074f930(float dt) override {
        // Every scalar past `count` is read only inside the per-leak passes, so
        // with a count of zero the result is the zero the routine's own two
        // accumulators start at, whatever they are.
        leak_water_mass_10fc = bsp::unit_leak_tick_0074f930(nullptr, 0u, 0.0f, 0.0f,
            0.0f, 1.0f, false, dt).total_water;
        owner_.done("ShipHydro::leak_tick", 0x0074f930u);
    }
    // 00933A52, over the same empty list, so the heeling torque is the zero the
    // routine's own out vector starts at.
    bsp::OceanVec3 leak_heel_torque_0074f2e0() override {
        float rows[9] = {
            slot_.motion.pose_row0[0], slot_.motion.pose_row0[1], slot_.motion.pose_row0[2],
            slot_.motion.pose_row1[0], slot_.motion.pose_row1[1], slot_.motion.pose_row1[2],
            slot_.motion.pose_row2[0], slot_.motion.pose_row2[1], slot_.motion.pose_row2[2]};
        const bsp::OceanVec3 torque = bsp::unit_leak_torque_0074f2e0(nullptr, 0u, rows);
        owner_.done("ShipHydro::leak_heel_torque", 0x0074f2e0u);
        return torque;
    }
    // 00933B01 and 00933B38, the two flushes. They are the only reason the
    // velocity phase 00C41550 has anything to integrate.
    void add_force_00c35360(const bsp::OceanVec3& force) override {
        bsp::dyn_body_add_force_00c35360(slot_.body, force);
        ++owner_.summary.hydro_force_flushes;
        owner_.done("ShipHydro::add_force", 0x00c35360u);
    }
    void add_torque_00c35330(const bsp::OceanVec3& torque) override {
        bsp::dyn_body_add_torque_00c35330(slot_.body, torque);
        ++owner_.summary.hydro_torque_flushes;
        owner_.done("ShipHydro::add_torque", 0x00c35330u);
    }
    void set_linear_velocity_00c37e50(const bsp::OceanVec3& v) override {
        bsp::dyn_body_set_linear_velocity_00c37e50(slot_.body, v);
        slot_.motion.linear_velocity = v;
        owner_.done("ShipHydro::set_linear_velocity", 0x00c37e50u);
    }
    void set_angular_velocity_00c37e20(const bsp::OceanVec3& w) override {
        bsp::dyn_body_set_angular_velocity_00c37e20(slot_.body, w);
        slot_.motion.angular_velocity = w;
        owner_.done("ShipHydro::set_angular_velocity", 0x00c37e20u);
    }
    void set_body_no_gravity_flag_00932a16() override {
        slot_.body.flags |= bsp::kDynBodyFlagNoGravity;
        owner_.record("ShipHydro::set_body_no_gravity_flag", 0x00932a16u);
    }

    // unit+10FCh as the leak tick left it, for the next call's total mass.
    float leak_water_mass_10fc{0.0f};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::ShipMotionHost, one method per call site of 00825f20's motion path
// ---------------------------------------------------------------------------

class ShipMotionBinding final : public bsp::ShipMotionHost {
public:
    ShipMotionBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
        UnitRudderBinding& rudder)
        : owner_(owner), slot_(slot), rudder_(rudder) {}

    void tick_order_ring(float dt) override {
        bsp::tick_unit_order_ring_00813020(slot_.ring, dt, 1);
        slot_.motion.throttle = slot_.ring.current_param_a;
        slot_.motion.to_turn = slot_.ring.current_param_b;
        slot_.motion.order_kind = slot_.ring.current_kind;
        owner_.done("ShipMotion::order_ring_tick", 0x00813020u);
    }

    // 0078cf20 at 00826985, run whole: the wave field times the coverage mask,
    // formed at x87 precision and rounded once. Both leaves are records.
    float ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("ShipMotion::ocean_height", 0x0078cf20u);
        return height;
    }

    // 008e6430 at 00826a21, run whole over an empty category list. The
    // accumulator starts at the 1.0f at 00d7a24c and nothing multiplies into it,
    // so the result is the routine's own value.
    float gameplay_scale() override {
        if (!owner_.logged_scale) {
            owner_.logged_scale = true;
            owner_.log.notef("gameplay scale 008e6430 runs over an empty category list: no "
                "modifier record is registered in this process, so the product is the 1.0f "
                "the accumulator starts at (00d7a24c) and the filter 008e4680 is never "
                "reached");
        }
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }

    // controller->vtable[0](dt), which for a surface ship is 00937440.
    //
    // Milestone 2r said of this torque that "nothing applies it, because the
    // hydrodynamic tail 009329c0 and the rigid-body solver are external". Both
    // halves are now false. 00937440 applies it itself: `python tools/bsp.py
    // ghidra xrefs 00c35330` reports `From 00937613 in
    // BSP_UnitController_ApplyShipForces`, nine instructions before the call
    // into 009329C0, so the torque goes onto the body exactly as the hydro
    // totals do. It is a call the executable makes and it is made here, and it
    // moves nothing for two separate reasons: the vector it carries is the zero
    // vector, because the gain is multiplied by settings+588h and that field
    // has no recovered producer, and the inverse inertia is zero anyway,
    // because the collision AABB's producer 00C5C940 is unread.
    //
    // Then the tail. 00937622 is a CALL into 009329C0 with the same dt
    // (00937618 FLD [ESP+40h], 0093761C PUSH ECX, 0093761D MOV ECX,EDI,
    // 0093761F FSTP [ESP]), followed by the epilogue at 00937627, so the
    // hydrodynamics happen here, inside the motion tick at 00826A6D and ahead
    // of 0092D300's velocity rewrite, which is the native order. What they
    // stage reaches the hull through 00C35360 AddForce and 00C35330 AddTorque,
    // and the velocity phase 00C41550 at the end of the step integrates it.
    bsp::OceanVec3 run_force_model(float dt) override {
        bsp::UnitSteeringTorqueInputs in{};
        in.velocity = slot_.motion.linear_velocity;
        in.axis.x = slot_.motion.pose_row2[0];
        in.axis.y = slot_.motion.pose_row2[1];
        in.axis.z = slot_.motion.pose_row2[2];
        in.reference_speed = reference_speed();
        in.hull_mass = slot_.motion_class.hull_mass;
        in.steering = slot_.motion.to_turn;
        in.pose_row0_y = slot_.motion.pose_row0[1];
        in.pose_row2.x = slot_.motion.pose_row2[0];
        in.pose_row2.y = slot_.motion.pose_row2[1];
        in.pose_row2.z = slot_.motion.pose_row2[2];
        in.settings_rudder_torque = 0.0f;  // settings+588h, not recovered
        owner_.done("ShipMotion::force_model", 0x00937440u);
        const bsp::OceanVec3 torque = bsp::unit_steering_torque_00937440(in);
        // 00937449 CMP byte ptr [EAX+5Dh],0 / 0093744D JNZ 00937618: a set byte
        // skips the whole torque block and lands one instruction before the
        // hydrodynamic call, so the hydrodynamics run either way and only the
        // torque is gated. EAX is [controller+1Ch], the unit, and the byte is
        // the one bsp/unit_instance.hpp names `simulate`, held clear here.
        if (slot_.state == nullptr || !slot_.state->simulate) {
            bsp::dyn_body_add_torque_00c35330(slot_.body, torque);
            owner_.done("ShipMotion::add_rudder_torque", 0x00c35330u);
        }
        run_hydrodynamics_00937622(dt);
        return torque;
    }

    // 00937622, 00937440's last call. 009329C0 is slot 0 of the controller vtable
    // 00D19630 and 00937440 forwards its own dt into it.
    void run_hydrodynamics_00937622(float dt) {
        if (slot_.buoyancy_elements.empty()) return;
        ShipHydroBinding hydro_host(owner_, slot_);
        hydro_host.leak_water_mass_10fc = slot_.leak_water_mass_10fc;

        bsp::ShipHydroInputs in{};
        // controller+14h. Nothing in this process sets it, so the live path at
        // 00932A1F is the one taken and the routine never writes a velocity.
        in.disabled = false;
        in.material = slot_.hull_material;
        in.record = bsp::ship_physics_material_shipped(slot_.hull_material);
        in.class_mass = slot_.motion_class.hull_mass;      // class+B0h
        in.class_length = slot_.motion_class.hull_length;  // class+A0h
        // Raw class+A4 Width from00960368; a model-derived unit+9CC can
        // differ, so this class-field consumer retains the Lua value.
        in.class_width = slot_.class_width_00a4;
        in.leak_water_mass = slot_.leak_water_mass_10fc;
        // unit+5Dh, the byte docs/UNIT_INSTANCE.md names `simulate`. The list
        // filter requires it clear on a live ship, and clear is what opens the
        // planing branch at 00933639, not what shuts it.
        in.suppress_planing = slot_.state != nullptr && slot_.state->simulate != 0;
        in.elements = slot_.buoyancy_elements.data();
        in.element_count = static_cast<int>(slot_.buoyancy_elements.size());

        const bsp::ShipHydroResult result
            = bsp::ship_hydro_apply_forces_009329c0(in, dt, hydro_host);
        owner_.done("ShipMotion::hydrodynamics", 0x009329c0u);
        slot_.leak_water_mass_10fc = hydro_host.leak_water_mass_10fc;
        ++slot_.row.hydro_calls;
        ++owner_.summary.hydro_calls;
        owner_.summary.hydro_element_steps
            += static_cast<unsigned long long>(in.element_count);
        owner_.summary.hydro_submerged_steps
            += static_cast<unsigned long long>(result.submerged_elements);
        slot_.row.hydro_elements = in.element_count;
        slot_.row.hydro_submerged = result.submerged_elements;
        slot_.row.hydro_force[0] = result.force.x;
        slot_.row.hydro_force[1] = result.force.y;
        slot_.row.hydro_force[2] = result.force.z;
        slot_.row.hydro_torque[0] = result.torque.x;
        slot_.row.hydro_torque[1] = result.torque.y;
        slot_.row.hydro_torque[2] = result.torque.z;
        if (!owner_.logged_hydro) {
            owner_.logged_hydro = true;
            owner_.log.notef("hydrodynamics 009329c0 runs from 00937440's last call at "
                "00937622, over %d buoyancy elements of \"%s\" and physics material %d. "
                "The element list at class+52Ch is a STAND-IN: no function in the "
                "exported set writes class+528h..+534h, so the list is built from the "
                "class row's own Length %.1f, Height %.1f and Mass %.1f the way "
                "bsp_ship_motion_probe.exe --hydro builds it, and the buoyancy is solved "
                "so the hull displaces its own weight at its draft. The world's gravity "
                "is now the game's own (0, %.1f, 0) from 004DDB90; before this milestone "
                "it was zero, and the two only balance together. The first flush staged "
                "force (%.2f, %.2f, %.2f) and torque (%.2f, %.2f, %.2f) with %d of %d "
                "elements submerged",
                static_cast<int>(slot_.buoyancy_elements.size()), slot_.row.name.c_str(),
                static_cast<int>(slot_.hull_material),
                static_cast<double>(slot_.motion_class.hull_length),
                static_cast<double>(slot_.motion_class.hull_height),
                static_cast<double>(slot_.motion_class.hull_mass),
                static_cast<double>(owner_.physics_world.gravity.y),
                static_cast<double>(result.force.x), static_cast<double>(result.force.y),
                static_cast<double>(result.force.z),
                static_cast<double>(result.torque.x), static_cast<double>(result.torque.y),
                static_cast<double>(result.torque.z), result.submerged_elements,
                in.element_count);
        }
    }

    float reference_speed() override {
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled);
    }

    bsp::OceanVec3 body_linear_velocity() override { return slot_.motion.linear_velocity; }

    bsp::ShipBodyBasis body_basis() override {
        bsp::ShipBodyBasis basis{};
        for (int i = 0; i < 3; ++i) {
            basis.row0[i] = slot_.motion.pose_row0[i];
            basis.row1[i] = slot_.motion.pose_row1[i];
            basis.row2[i] = slot_.motion.pose_row2[i];
        }
        return basis;
    }

    float forward_acceleration() override {
        return bsp::unit_forward_acceleration_00825ec0(slot_.motion_class.retardation,
            slot_.motion.acceleration_boost, 0.0f);
    }

    void body_set_linear_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.linear_velocity = value;
    }

    bsp::OceanVec3 body_angular_velocity() override { return slot_.motion.angular_velocity; }

    float yaw_rate_target(float smoothed_rudder) override {
        rudder_.forward_speed = forward_speed();
        // 0092e950 then the FCHS at 0092e955.
        const float rate = -bsp::unit_yaw_rate_00811890(smoothed_rudder, rudder_);
        // 0092e966: settings+220h/+224h were not recovered; with a zero
        // propeller load the routine returns the rate unchanged whatever they
        // are, which is the state a created unit is in here.
        return bsp::unit_propeller_turn_assist_00825de0(rate, slot_.motion.propeller_load,
            0.0f, 0.0f);
    }

    // A destroyer's IsKindOf answers {0,1,2,4,5,6,7} plus its own class id, so
    // 0Eh is false; the same holds for every ship leaf this mission creates.
    bool unit_trait_0e() override { return false; }

    void body_set_angular_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.angular_velocity = value;
    }

    void controller_step(float dt) override {
        bsp::unit_controller_update_0092be80(slot_.controller, dt);
        owner_.done("ShipMotion::controller_step", 0x0092be80u);
    }
    void unit_post_motion(float) override {
        owner_.record("ShipMotion::unit_post_motion", 0x00826b84u);
    }

    float forward_speed() const {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        return bsp::unit_forward_speed_0092d730(in);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    UnitRudderBinding& rudder_;
};

// ---------------------------------------------------------------------------
// 004c0890, the controlled-unit bind
// ---------------------------------------------------------------------------

class ControlledUnitQueryBinding final : public bsp::ControlledUnitQuery {
public:
    explicit ControlledUnitQueryBinding(int class_id) : class_id_(class_id) {}
    bool unit_is_kind_of(int query) override {
        return bsp::unit_is_kind_of(class_id_, query);
    }
    bool has_driven_sub_unit() override { return false; }   // unit+3D0h
    bool sub_unit_is_kind_of(int) override { return false; }

private:
    int class_id_;
};

class SetControlledUnitBinding final : public bsp::SetControlledUnitHost {
public:
    explicit SetControlledUnitBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    void store_controlled_unit(bool unit_present) override {
        owner_.controlled_bound = unit_present;
        owner_.done("ControlledUnit::store_global", 0x004c0893u);
    }
    bool driven_listener_handle() override {
        owner_.record("ControlledUnit::driven_listener_handle", 0x004c08f7u);
        return false;
    }
    void publish_listener(bool) override {
        // 00b0d7b0 stores the handle at renderResources+1C0h and kicks +30h.
        // The render-resources singleton at 00f8d39c is the renderer owner's.
        owner_.record("ControlledUnit::publish_listener", 0x00b0d7b0u);
    }

private:
    GameUnitsHost::Impl& owner_;
};

}  // namespace

// ---------------------------------------------------------------------------
// Impl helpers
// ---------------------------------------------------------------------------

void GameUnitsHost::Impl::refresh_row(GameUnitSlot& slot) {
    GameUnitRow& row = slot.row;
    row.active = slot.state != nullptr && slot.state->active != 0;
    for (int i = 0; i < 3; ++i) row.position[i] = slot.motion.position[i];
    row.heading_degrees = heading_degrees_of(slot.motion);
    bsp::UnitBodyAxisSpeedInputs speed{};
    speed.velocity[0] = slot.motion.linear_velocity.x;
    speed.velocity[1] = slot.motion.linear_velocity.y;
    speed.velocity[2] = slot.motion.linear_velocity.z;
    speed.axis[0] = slot.motion.pose_row2[0];
    speed.axis[1] = slot.motion.pose_row2[1];
    speed.axis[2] = slot.motion.pose_row2[2];
    row.forward_speed = bsp::unit_forward_speed_0092d730(speed);
    row.throttle = slot.motion.throttle;
    row.ordered_rudder = slot.motion.to_turn;
    row.rudder = slot.motion.smoothed_rudder;
    row.yaw_rate = slot.motion.angular_velocity.y;
    // The component about the hull's own up axis, which is what 0092e8c0 slews.
    row.yaw_rate_up_axis = slot.motion.angular_velocity.x * slot.motion.pose_row1[0]
        + slot.motion.angular_velocity.y * slot.motion.pose_row1[1]
        + slot.motion.angular_velocity.z * slot.motion.pose_row1[2];
    const double dx = static_cast<double>(row.position[0]) - static_cast<double>(row.start[0]);
    const double dz = static_cast<double>(row.position[2]) - static_cast<double>(row.start[2]);
    row.distance = static_cast<float>(std::sqrt(dx * dx + dz * dz));
}

void GameUnitsHost::Impl::issue_into_ring(GameUnitSlot& slot, float throttle, float rudder) {
    // 00816a40 builds the 20h-byte record through 00815440, which clamps both
    // parameters into [-2,+2], and publishes it into the slot the write cursor
    // names. The queue and the ring are two projections of the same native
    // storage at unit+838h, so the published slot is copied into the ring the
    // tick reads; 0080dad0 clears +8h, which marks the slot authoritative.
    UnitOrderIssueBinding issue(*this);
    slot.queue.slot_index = slot.ring.write_cursor;
    bsp::issue_unit_order_record_00816a40(issue, slot.queue, slot.scratch, throttle, rudder, 0);
    const int index = slot.queue.slot_index;
    slot.ring.slot[index].param_a = slot.queue.slot[index].param_a;
    slot.ring.slot[index].param_b = slot.queue.slot[index].param_b;
    slot.ring.slot[index].kind = slot.queue.slot[index].kind;
    slot.ring.slot[index].predicted = slot.queue.slot_active[index];
    done("ShipMotion::issue_order_record", 0x00816a40u);
}

namespace {

// ---------------------------------------------------------------------------
// Milestone 2q: bsp::CruiseSpeedSettingHost, the five call sites of 00822C20's
// property-bag arm (0082356C..008235FB).
// ---------------------------------------------------------------------------
//
// 00822C20 is slot 0A0h of the game-unit vtable family and the executable
// reaches it where the native does, from BSP_SEntity_InitAll's slot call at
// 00926110, which in this process is the per-entity pass create_units makes
// over the records the instantiate pass left. The two finds are answered from
// the merged property bag the scene-contents host kept on the entity record
// (docs/CRUISE_SPEED_SETTING.md); the three setters are the reconstructions
// already on main.
class StartSpeedSeedBinding final : public bsp::CruiseSpeedSettingHost {
public:
    StartSpeedSeedBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
                          const GameSceneEntityRecord& entity)
        : owner_(owner), slot_(slot), entity_(entity) {}

    bool find_shipyard_launch_00823576(std::uint32_t, const char*) override {
        owner_.done("SceneStartSpeed::find_shipyard_launch", 0x00823576u);
        return entity_.shipyard_launch;
    }

    bsp::SceneStartSpeedProperty find_start_speed_00823590(std::uint32_t,
                                                           const char*) override {
        owner_.done("SceneStartSpeed::find_start_speed", 0x00823590u);
        bsp::SceneStartSpeedProperty record;
        record.present = entity_.start_speed_present;
        record.type = static_cast<bsp::ScenePropertyType>(entity_.start_speed_type);
        record.value_int = entity_.start_speed_int;
        record.value_float = entity_.start_speed_float;
        return record;
    }

    float unit_reference_speed_0080fc30(std::uint32_t) override {
        // 008235BA and 008235DC, the same callee at two sites. unit+9C0h is
        // the class `MaxSpeed` 00822C4F/00822C65 set earlier in this same
        // routine, which create_units copies out of the VehicleClass row.
        ++reference_calls;
        owner_.done("SceneStartSpeed::unit_reference_speed", 0x0080fc30u);
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled);
    }

    void set_order_ring_throttle_0080d9b0(std::uint32_t, float throttle) override {
        // 008235D5 with ECX = unit+838h. The reconstruction fills the pending
        // span and writes the live field, which is ring+148h.
        bsp::set_unit_order_ring_param_a_0080d9b0(slot_.ring, throttle);
        owner_.done("SceneStartSpeed::set_order_ring_throttle", 0x0080d9b0u);
    }

    void set_controller_axial_speed_0092d770(std::uint32_t, float speed) override {
        // 008235F7 with ECX = [unit+1018h]. The body axis is the pose's row 2,
        // the same forward vector 0092D730 projects onto.
        bsp::OceanVec3 axis{};
        axis.x = slot_.motion.pose_row2[0];
        axis.y = slot_.motion.pose_row2[1];
        axis.z = slot_.motion.pose_row2[2];
        slot_.motion.linear_velocity
            = bsp::unit_set_axial_speed_0092d770(slot_.motion.linear_velocity, axis, speed);
        slot_.motion_state.linear_velocity = slot_.motion.linear_velocity;
        owner_.done("SceneStartSpeed::set_controller_axial_speed", 0x0092d770u);
    }

    int reference_calls{0};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    const GameSceneEntityRecord& entity_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameUnitsHost
// ---------------------------------------------------------------------------

GameUnitsHost::GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua)
    : impl_(std::make_unique<Impl>(log, lua)) {}

GameUnitsHost::~GameUnitsHost() {
    Impl& host = *impl_;
    // Directors borrow units. Frame-level borrowers have already been released.
    host.gunnery.reset();
    if (host.observer_runtime == nullptr || host.slots.empty()) return;
    if (!host.observer_runtime->has_live_dispatch_owner()) {
        host.log.note("unit observer teardown requires the live application dispatch owner");
        std::terminate();
    }
    std::size_t destroyed = 0;
    for (const auto& slot : host.slots) {
        // Withdraw lookup before lifetime callbacks can re-enter this host.
        // Already borrowed aliases expire here; native array frees retain bytes.
        slot->observer_prefix_ready = false;
        // 0092589D precedes 009258AC: callback +10h, then observed +0h.
        // The slot and all other endpoint owners remain alive throughout.
        auto& lifetime = host.observer_runtime->lifetime();
        lifetime.destroy_callback_owner_00695870(slot->observer_prefix.callback_10);
        lifetime.destroy_observed_owner_00695760(slot->observer_prefix.observed_00);
        ++destroyed;
    }
    host.log.notef("unit observer teardown: units=%zu callback_then_observed=1 live_owner=1", destroyed);
}

void GameUnitsHost::bind_observer_runtime(GameObserverRuntime& runtime) {
    Impl& host = *impl_;
    if (!runtime.has_live_dispatch_owner())
        throw std::logic_error("unit observer binding requires a live dispatch owner");
    if (host.observer_runtime != nullptr && host.observer_runtime != &runtime)
        throw std::logic_error("unit observer runtime cannot change while the host lives");
    for (const auto& slot : host.slots) {
        if (!slot->observer_prefix_ready)
            throw std::logic_error("unit observer binding requires a known native creator");
    }
    host.observer_runtime = &runtime;
}

void GameUnitsHost::load_gameplay_settings_0083b5e0() {
    Impl& host = *impl_;
    // 007E2A20, the plane half of the same settings load. It is independent of
    // 0083B5E0 - a different singleton, its own two scripts, its own global -
    // and it runs before the rudder-curve guard because that guard is about
    // ShipGlobals and says nothing about whether the plane block is filled.
    // Idempotent: a second call re-runs the scripts and rewrites the block with
    // the same values.
    host.lua.load_plane_globals_007e2a20();
    if (host.rudder_curve_loaded) return;
    // 0083b5e0's head: run Scripts\datatables\ShipGlobals.lua and take the
    // `ShipGlobals` global. Then the fragment 0083ce56..0083d10d, which is what
    // fills +438h..+44Ch of the settings object 00424c40 hands out.
    const bool table = host.lua.load_ship_globals_0083b6e6();
    host.record("GameSettings::load_from_lua_globals", 0x0083b5e0u);
    if (!table) {
        host.log.notef("rudder curve: `ShipGlobals` did not load, so 00424c40()+438h..+44Ch "
            "keeps the zeroes a fresh settings object has and 0082e890 would divide "
            "MaxRotAngle by zero; the motion path is left with the curve unset");
        return;
    }
    // Milestone 2p: the AutoThrust half of the same loader, 0083cba8..0083ce3c,
    // read before the turn multipliers so an incomplete TurnMultipliers table
    // does not hide it. 009EC7C0 consumes seven of the eleven keys.
    if (host.lua.read_auto_thrust_0083cc2c(host.auto_thrust)) {
        host.auto_thrust_loaded = true;
        host.done("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust loaded from ShipGlobals[\"Navigator\"][\"AutoThrust\"]: "
            "hdg slow (%.4f, %.4f) thrust_min %.3f, hdg fast (%.4f, %.4f) thrust_min %.3f, "
            "danger mul %.3f; these are 00424c40()+6cch..+6ech, the block "
            "009ec7c0 BSP_UnitBot_ComputeThrottleCeiling interpolates over",
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_slow),
            static_cast<double>(host.auto_thrust.thrust_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_fast),
            static_cast<double>(host.auto_thrust.thrust_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_danger_mul));
    } else {
        host.record("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust: ShipGlobals[\"Navigator\"][\"AutoThrust\"] is absent or "
            "incomplete, so 00424c40()+6cch..+6ech keeps the zeroes a fresh settings object "
            "has and 009ec7c0's five interpolation stages all run on zero endpoints");
    }

    bsp::UnitRudderCurveSettings settings{};
    if (!host.lua.read_turn_multipliers_0083ce56(settings)) {
        host.log.notef("rudder curve: ShipGlobals[\"Navigator\"][\"TurnMultipliers\"] is "
            "incomplete, so the six fields stay unset");
        return;
    }
    host.rudder_curve = settings;
    host.rudder_curve_loaded = true;
    host.done("GameSettings::load_turn_multipliers", 0x0083ce56u);
    host.log.notef("rudder curve loaded from ShipGlobals[\"Navigator\"][\"TurnMultipliers\"]: "
        "min (%.3f, %.3f) med (%.3f, %.3f) max (%.3f, %.3f); index 1 is the throttle "
        "coordinate and index 2 the turn-circle multiplier 0082ecb0 divides MaxRotAngle by",
        static_cast<double>(settings.speed_0444), static_cast<double>(settings.value_0440),
        static_cast<double>(settings.speed_044c), static_cast<double>(settings.value_0448),
        static_cast<double>(settings.speed_043c), static_cast<double>(settings.value_0438));
}

void GameUnitsHost::create_units(const std::vector<GameSceneEntityRecord>& entities) {
    Impl& host = *impl_;
    for (const GameSceneEntityRecord& entity : entities) {
        if (!entity.created) continue;
        auto slot = std::make_unique<GameUnitSlot>();
        GameUnitRow& row = slot->row;
        row.name = entity.name;
        row.class_name = entity.class_name;
        row.type_symbol = entity.type_symbol;
        row.type_id = entity.type_id;
        row.party = entity.party;
        // Milestone 2l: the two strings 004f0520's last step handed 00469610.
        row.command = entity.command;
        row.command_target = entity.command_target;

        // The frame 0046cf40 composed for the gate is the instance's world 4x4;
        // rows 0..2 are the body axes and row 3 the position, which is the same
        // shape the motion path reads at unit+CCh..+108h.
        for (int i = 0; i < 3; ++i) {
            slot->motion.pose_row0[i] = entity.world[i];
            slot->motion.pose_row1[i] = entity.world[4 + i];
            slot->motion.pose_row2[i] = entity.world[8 + i];
            slot->motion.position[i] = entity.world[12 + i];
            row.start[i] = slot->motion.position[i];
            row.position[i] = slot->motion.position[i];
        }
        row.heading_degrees = heading_degrees_of(slot->motion);

        // The class descriptor. 00964790, the factory that would build one, is
        // a milestone 2h record; what this reads is the installed
        // `VehicleClass` global the recovered global-script step 00886900
        // loaded, indexed by the `Type = E ShipClasses : <symbol>` id the enum
        // library resolved.
        const GameVehicleClassRow lua_row = host.lua.read_vehicle_class_row(row.type_id);
        row.class_row_found = lua_row.found;
        row.class_row_name = lua_row.name;
        const bsp::VehicleClassDescriptorRow* kind = lua_row.found
            ? bsp::vehicle_class_kind_row(lua_row.type.c_str()) : nullptr;
        slot->motion_dispatch = unit_motion_dispatch(kind);
        if (slot->motion_dispatch.entry == 0x007ce040u) {
            // 007C6340's fall-through at 007C6481 sets unit+900h = 7 and
            // unit+908h = 3600 together, and one call satisfies both of the
            // free-flight arm's requirements. 007CFD20 leaves +900h at 0, which
            // selects no arm at all - which is why a freshly created plane is
            // correctly motionless until something seeds it.
            // docs/PLANE_FLIGHT_CORE_LAW.md.
            slot->plane_control_mode_900 = 7;
            slot->plane_airborne_908 = 3600.0f;
            // 007D1F70's rate and acceleration keys for this row. They are what
            // the control targets and the rate law are built from, and they are
            // per-class rather than global, so a Zero and a Dauntless turn at
            // different speeds. Zero on a row that does not carry them, which
            // is the right answer for a ship.
            slot->plane_class.roll_spd = lua_row.roll_spd;
            slot->plane_class.pitch_spd = lua_row.pitch_spd;
            slot->plane_class.yaw_spd = lua_row.yaw_spd;
            slot->plane_class.yaw_roll_ratio = lua_row.yaw_roll_ratio;
            slot->plane_class.slide_ratio = lua_row.slide_ratio;
            slot->plane_class.roll_accel = lua_row.roll_accel;
            slot->plane_class.pitch_accel = lua_row.pitch_accel;
            slot->plane_class.yaw_accel = lua_row.yaw_accel;
            slot->plane_class.negative_pitch_ratio = lua_row.negative_pitch_ratio;
            slot->plane_class_turn_roll_spd = lua_row.turn_roll_spd;
            slot->plane_class_turn_roll = lua_row.turn_roll;
            if (lua_row.plane_stall_spd > 0.0f) {
                // desc+184h. PlaneFreeFlightClass keeps 17.5f as its fallback;
                // an authored row wins.
                slot->plane_stall_spd = lua_row.plane_stall_spd;
            }
            // desc+174h / desc+170h. Zero on a ship row, which is what a ship
            // should read; 7.0 on this installation's TBD Devastator row.
            slot->plane_x_drag = lua_row.x_drag;
            slot->plane_y_drag = lua_row.y_drag;
            slot->plane_max_spd = lua_row.max_spd;
            slot->plane_travel_speed = lua_row.travel_speed;
            slot->plane_accel = lua_row.accel;
            slot->plane_glide_rate = lua_row.glide_rate;
            slot->plane_drag_pitch_ratio = lua_row.drag_pitch_ratio;
            slot->plane_air_brake_drag = lua_row.air_brake_drag;
            slot->plane_drop_angle = lua_row.drop_angle;
            slot->plane_swim_height = lua_row.swim_height;
            // 007C4BC5-007C4C14. The probe speed of the first call is
            // tuning+24Ch LevelFlight times desc+184h StallSpd, which is the
            // same product the lift term caps its q at, and 007C4C0E scales the
            // answer by the double 0.6 at 00CEFF98 into desc+1ECh.
            {
                bsp::PlaneClimbAngleInputs cai;
                cai.accel = slot->plane_accel;
                cai.max_spd = slot->plane_max_spd;
                float level_flight = 1.8f;      // tuning+24Ch
                float accel_cheat_mul = 1.5f;   // tuning+31Ch
                if (host.lua.plane_globals_loaded()) {
                    const bsp::GameTuningBlock& g = host.lua.plane_globals();
                    level_flight = g.dynamics_spd_multipliers_level_flight;
                    accel_cheat_mul = g.dynamics_accel_cheat_mul;
                }
                cai.accel_cheat_mul = accel_cheat_mul;
                cai.probe_speed = level_flight * slot->plane_stall_spd;
                slot->plane_climb_angle_1e4 =
                    bsp::max_sustainable_climb_angle_007d98f0(cai);
                slot->plane_climb_angle_1ec = slot->plane_climb_angle_1e4 * 0.6f;
            }
            // The spawn airspeed. docs/PLANE_FLIGHT_CORE_LAW.md measures the
            // seed at 141.67 m/s, and the acceptance test in tests/math_tests.cpp
            // pins lift against gravity at exactly that value and at
            // 1.8 * StallSpd.
            //
            // It is an AIRSPEED, so it goes along the plane's own forward axis,
            // not along a world axis. That axis is pose_row2 - the frame
            // 0046cf40 composed from the authored placement, which the loop
            // above already copied in, and which 00521374's camera path reads
            // the same way (src/system_camera_axes.cpp:387 takes world[8..10]
            // as forward). An earlier revision seeded world +Z instead, on the
            // unchecked assumption that a placement carries no rotation; that
            // flew every plane in the same arbitrary direction.
            {
                const float* const fwd = slot->motion.pose_row2;
                const float len = std::sqrt(fwd[0] * fwd[0] + fwd[1] * fwd[1] +
                                            fwd[2] * fwd[2]);
                // The magnitude is desc+18Ch TravelSpeed, which
                // docs/PLANE_FLIGHT_CORE_LAW.md already named as the field
                // 007C6340 seeds from - it was just stood in for by one row's
                // value. 141.666672 is the TravelSpeed of four rows in this
                // installation's vehicleclasses.lua; the torpedo bombers
                // author 61.111111 (TBD Devastator and TBF Avenger alike), and
                // running them at 141.67 is what put an air-dropped torpedo
                // into the water above MaxWaterHitVel no matter how low it was
                // released. The constant stays as the fallback for a row that
                // carries no TravelSpeed, which is every ship row.
                // docs/TORPEDO_RUN_IN_VELOCITY.md.
                const float seed_speed = slot->plane_travel_speed > 0.0f
                    ? slot->plane_travel_speed : 141.666672f;
                if (len > 1e-6f) {
                    for (int i = 0; i < 3; ++i) {
                        slot->plane_world_velocity[i] = seed_speed * fwd[i] / len;
                    }
                } else {
                    // A degenerate authored frame keeps the old world-+Z seed
                    // rather than propagating a NaN through every lift term.
                    slot->plane_world_velocity[2] = seed_speed;
                }
            }
            // The same mirror as the integration step: the body velocity field
            // 0092D730 reads has to hold the seed too, or the first frames of a
            // plane's life answer 0.0 m/s. docs/TORPEDO_RELEASE_GEOMETRY.md.
            slot->motion.linear_velocity = bsp::OceanVec3{
                slot->plane_world_velocity[0], slot->plane_world_velocity[1],
                slot->plane_world_velocity[2]};
            slot->plane_velocity_seeded = true;
            host.log.notef("plane spawn: unit=%s heading=%.2f deg forward=(%.4f %.4f %.4f)"
                " seed=(%.2f %.2f %.2f)", row.name.c_str(),
                static_cast<double>(row.heading_degrees),
                static_cast<double>(slot->motion.pose_row2[0]),
                static_cast<double>(slot->motion.pose_row2[1]),
                static_cast<double>(slot->motion.pose_row2[2]),
                static_cast<double>(slot->plane_world_velocity[0]),
                static_cast<double>(slot->plane_world_velocity[1]),
                static_cast<double>(slot->plane_world_velocity[2]));
        }
        bsp::publish_game_entity_observer_tables_00928662(slot->observer_prefix);
        slot->observer_prefix_ready = bsp::publish_unit_leaf_observer_tables_for_creator(
            slot->observer_prefix, slot->motion_dispatch.creator);
        if (host.observer_runtime != nullptr) {
            if (!host.observer_runtime->has_live_dispatch_owner())
                throw std::logic_error("unit creation requires the live bound observer owner");
            if (!slot->observer_prefix_ready)
                throw std::logic_error("unit observer creator projection is unavailable");
        }
        host.log.notef("unit motion dispatch: unit=%s creator=%08lx tick_vtable=%08lx "
            "entry=%08lx coverage=%s", row.name.c_str(),
            static_cast<unsigned long>(slot->motion_dispatch.creator),
            static_cast<unsigned long>(slot->motion_dispatch.tick_vtable),
            static_cast<unsigned long>(slot->motion_dispatch.entry),
            unit_motion_coverage_name(slot->motion_dispatch.coverage));
        if (lua_row.found) {
            ++host.summary.class_rows;
            row.max_speed = lua_row.max_speed;
            row.max_rot_angle = lua_row.max_rot_angle;
            slot->motion.max_speed = lua_row.max_speed;   // 00822c20 seeds unit+9C0h
            slot->motion_class.max_accel = lua_row.max_accel;
            slot->motion_class.retardation = lua_row.retardation;
            slot->fields.max_speed = lua_row.max_speed;
            slot->fields.max_rot_angle = lua_row.max_rot_angle;
            slot->fields.max_rot_angle_change_ratio = lua_row.max_rot_angle_change_ratio;
            // 00964790 maps VehicleClass.Type to a descriptor whose +28h
            // allocator constructs the instance. Its constructor stamps +C4h:
            // e.g. 006FE590 -> 006FE460, store 7 at 006FE4B3. The recovered
            // descriptor kind and instance class id coincide for these leaves.
            if (kind != nullptr) slot->class_id = static_cast<int>(kind->kind);
        }
        if (slot->class_id == bsp::kVehicleClassKindUnknown) {
            host.log.notef("unit identity unresolved: unit=%s type_id=%d class_row_found=%d "
                "VehicleClass.Type=\"%s\"; class_id=-1, kind queries return false",
                row.name.c_str(), row.type_id, row.class_row_found ? 1 : 0,
                lua_row.type.c_str());
        }
        // Milestone 2r. class+A0h, class+A8h and class+B0h are the `Length`,
        // `Height` and `Mass` keys 00960230 writes into the descriptor, and the
        // installed `VehicleClass` row carries all three (DeRuyter row 20:
        // 171, 5, 7688). Milestone 2q left them at zero because the keel gate
        // passes either way on a flat sea; they are read now because
        // 00937C90's hull body needs the mass and 00937440 needs it squared,
        // and because 00826866 places the keel sample point half a hull length
        // astern and half a hull height below the pose. A missing key leaves
        // the zero the reader stores, except `Mass`, whose reader default is
        // the 1.0f at 0096043A.
        slot->motion_class.hull_length = lua_row.length;   // class+A0h
        slot->motion_class.hull_height = lua_row.height;   // class+A8h
        slot->class_width_00a4 = lua_row.width;            // class+A4h
        //00810FAF..00811073: this represented class has no model resource.
        // Copy its actual Lua Width/Length through the recovered no-model arm;
        // a missing class leaves the slot's prior extent fields untouched.
        const bsp::UnitHullExtentClassInputs extent_class{
            nullptr, lua_row.length, lua_row.width};
        bsp::produce_unit_hull_extents_00810faf(slot->hull_extents,
            lua_row.found ? &extent_class : nullptr);
        constexpr float kClassMassReaderDefault = 1.0f;  // 0096043A
        slot->motion_class.hull_mass
            = lua_row.mass > 0.0f ? lua_row.mass : kClassMassReaderDefault;
        slot->motion_class.boost_refill_time = 1.0f;
        slot->motion.thrust_mod = 1.0f;        // 00823714
        slot->motion.turn_efficiency = 1.0f;   // 0082371c
        slot->motion.class_id = slot->class_id;

        bsp::construct_unit_order_ring_00812d40(slot->ring);

        // Milestone 2q: 00926110, BSP_SEntity_InitAll's call of the entity's
        // vtable slot 0A0h, which for this class family is 00822C20. Only that
        // routine's property-bag arm 0082356C..008235FB is run here, and only
        // its kind-1 branch: 0046d5b0 built the holder over a cloned scene
        // property bag, so 00823537's tag is 1 and 0082353D takes the arm.
        // The seed has to happen here, before issue_authored_commands latches
        // the scene's queued `Cruise` at 00835E17, because the latch captures
        // the ring's live throttle (docs/CRUISE_SPEED_SETTING.md).
        if (slot->motion_dispatch.runs_ship_base()) {
            StartSpeedSeedBinding seed_host(host, *slot, entity);
            const bsp::SceneStartSpeedSeed seed = bsp::run_start_speed_arm_0082356c(
                seed_host, static_cast<std::uint32_t>(host.slots.size()) + 1u,
                static_cast<std::uint32_t>(host.slots.size()) + 1u,
                bsp::kSceneEntityBagRefKindPropertyBag);
            host.done("SceneStartSpeed::init_slot_00a0", 0x00926110u);
            row.start_speed_authored = seed.authored;
            row.start_speed = seed.start_speed;
            row.start_speed_ratio = seed.ring_throttle;
            row.start_speed_axial = seed.axial_speed;
            if (seed.authored) {
                ++host.summary.start_speed_seeds;
                // ring.current_param_a and motion.throttle are two projections
                // of the one native field unit+980h, which 0080D9E8 has just
                // written; the ring tick copies it every step, so this only
                // keeps the two in step before the first tick runs.
                slot->motion.throttle = slot->ring.current_param_a;
            }
        }

        // Milestone 2r: the hull body the game builds, replacing milestone 2h's
        // freshly constructed one and the 1.0e30f speed clamps that stood in
        // for M+18h / M+1Ch. 00939E2A calls 00937C90 last in the controller
        // constructor 00939CB0 and its tail 009399C0..00939C05 is the writer:
        // the descriptor default 009391C2, the mass at 009399F7, the zeroed
        // inertia diagonal, the angular damping 1.0f at 00939A2F, the row-1
        // torque lock when the mass is under 100.0 (00939A57), then
        // 00C5D580 CreateBody and 00C37E70 with the box inertia at 00939C05.
        // The linear damping stays the descriptor's own 0.0f (00939295); it is
        // not a gap, and docs/SHIP_HULL_BODY.md says so.
        //
        // The collision AABB is the one input this cannot supply: its producer
        // is the shape attach 00C5C940 behind 00937D3F..009399BF, which no
        // packet has read, so the span is zero and the box inertia with it,
        // which 00C37E70 turns into a zero inverse inertia. Nothing here
        // applies a torque, so that decides nothing this run measures; it is
        // the same default the probe takes.
        if (slot->motion_dispatch.runs_ship_base()) {
            bsp::ShipHullBodyInputs hull{};
            hull.mass = slot->motion_class.hull_mass;             // 009399F7
            // 00937CFD calls virtual slot 5Ch with category 8 (MSubmarine);
            // the accepting branch picks the third physics record at 00937D09.
            hull.unit_category_8 = bsp::unit_is_kind_of(
                slot->class_id, bsp::kUnitForceSubmarineClassId);
            for (int i = 0; i < 3; ++i) {
                hull.row0[i] = slot->motion.pose_row0[i];
                hull.row1[i] = slot->motion.pose_row1[i];
                hull.row2[i] = slot->motion.pose_row2[i];
                hull.position[i] = slot->motion.position[i];
            }
            bsp::ship_hull_body_create_00937c90(hull, slot->body, slot->motion_state);
            slot->hull_material = bsp::ship_hull_material_00937cf1(hull.unit_category_8,
                hull.mass);
            slot->body.motion = &slot->motion_state;
        }

        // Milestone 2s: the buoyancy element list at class+52Ch..+530h, which
        // 009329C0 walks. This is a STAND-IN and the doc says so: the producer
        // writes class+528h..+534h and no function in the exported set does,
        // so docs/SHIP_HYDRO_FORCES.md carries the four field roles as a
        // hypothesis reconciled between its only two readers and leaves the
        // writer to packet `ship_buoyancy_element_producer`.
        //
        // The list built here is the one bsp_ship_motion_probe.exe --hydro
        // builds, so the two sides of the comparison are the same list: the
        // elements are spread evenly along the class `Length` in the hull's own
        // Y = 0 plane, the draft is half the class `Height`, and the shared
        // coefficient is solved so that a hull floating at that draft displaces
        // exactly its own weight against the world gravity of 10. A class row
        // that carries no Length or Height produces no list at all, and the
        // hydrodynamic step is then skipped rather than run on invented data.
        if (slot->motion_dispatch.runs_ship_base()) {
            const float length = slot->motion_class.hull_length;
            const float height = slot->motion_class.hull_height;
            const float mass = slot->motion_class.hull_mass;
            if (length > 0.0f && height > 0.0f && mass > 0.0f) {
                const float draft = height * 0.5f;
                // ship_hydro_buoyancy_00932e44 at depth == draft, with the
                // surface mix of 0.5 the 00CE3800 float supplies.
                const float shape
                    = bsp::kShipHydroBuoyancyShapeMix * (draft / height)
                        + (1.0f - bsp::kShipHydroBuoyancyShapeMix);
                const float count = static_cast<float>(kBuoyancyElementCount);
                const float per_element = (mass * bsp::kShipHydroDragGravity) / count;
                const float coefficient = per_element / (draft * shape);
                slot->buoyancy_elements.reserve(kBuoyancyElementCount);
                for (int i = 0; i < kBuoyancyElementCount; ++i) {
                    bsp::ShipBuoyancyElement element{};
                    element.coefficient = coefficient;
                    element.level_base = 0.0f;
                    element.level_draft = draft;
                    element.level_top = height;
                    element.position.x = 0.0f;
                    element.position.y = 0.0f;
                    element.position.z = length
                        * ((static_cast<float>(i) + 0.5f) / count - 0.5f);
                    slot->buoyancy_elements.push_back(element);
                }
            }
        }

        slot->parent = nullptr;
        slot->pose = std::make_unique<bsp::PoseRefreshView>(
            bsp::PoseRefreshView{bsp::PoseRefreshParentSlot(slot->parent), slot->local,
                slot->world_valid, slot->world, slot->derived_valid});
        slot->state = std::make_unique<bsp::UnitInstanceState>(
            bsp::UnitInstanceState{*slot->pose});
        slot->state->class_id = slot->class_id;
        // The represented kind1 scene path reaches00923840 through each
        // supported creator's primary+A0 initializer.00923855 writes+5C=1;
        // this is initialization, not the constructor's initial zero.
        //00925E14 clears+5D; type3 deadMeat delivery is not represented here.
        slot->state->active = 1;
        slot->state->simulate = 0;
        slot->scene_flags_available = slot->motion_dispatch.creator != 0;
        row.active = slot->state->active != 0;
        slot->state->has_scene_node = false;  // +4A4h, 00928860 is a 2h record
        slot->state->part_count = 0;     // +A18h, the instance has no parts here
        // The actual descriptor creator selects the native+130h override;
        //009288F1 is the recovered dispatch site, not a missing caller.
        slot->process_index = host.slots.size();
        //00925906 stores the actual world/list owner at unit+30h. This
        // process already owns that registry; full placement/locking and
        // hierarchy attachment remain separate unreconstructed runtime work.
        slot->world_parent_0030 = &host.world_lists;
        host.register_in_world_lists(*slot);
        Impl::publish_pose(*slot);
        host.slots.push_back(std::move(slot));
    }
    host.summary.units = host.slots.size();
    // Milestone 2l: the entities 0046aab0's target lookup would find, and the
    // owners its records name. entity+174h is the executable's own id space,
    // because the two handle tables at 00f89a0c / 00f89a60 are not built here.
    std::vector<GameCommandUnit> command_units;
    command_units.reserve(host.slots.size());
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameCommandUnit unit;
        unit.index = index;
        unit.name = host.slots[index]->row.name;
        unit.object_id = static_cast<std::uint16_t>(index + 1);
        for (int lane = 0; lane < 3; ++lane) {
            unit.position[lane] = host.slots[index]->motion.position[lane];
        }
        command_units.push_back(unit);
    }
    host.commands.register_units(std::move(command_units));
    host.log.notef("world units: %zu created instance(s) carried into the frame, %zu with a "
        "VehicleClass row out of the installed table", host.summary.units,
        host.summary.class_rows);
    // Milestone 2t: the unit-side gunnery pass. 00810DD0's creation block puts a
    // 558h-byte object at unit+6DCh and attaches it to the unit's own tick
    // element unit+310h through its vtable +4h (00864BD0), which is why it runs
    // on the fixed step beside the motion pass. The guns themselves come from
    // the authored `VehicleClass[id].Platforms` table, because this process
    // builds no model hierarchy; include/bsp/game_hosts_gunnery.hpp says so.
    host.gunnery = std::make_unique<GameGunneryHost>(host.log, *this, host.lua);
    host.gunnery->set_ship_ai(host.ship_ai);
    host.gunnery->attach_00864bd0();
    host.ai = std::make_unique<GameAiCoordinatorHost>(host.log, *this);
    host.ai->create_00a32350();
}

void GameUnitsHost::issue_authored_commands() {
    Impl& host = *impl_;
    if (!host.logged_cruise) {
        host.logged_cruise = true;
        host.log.notef("milestone 2l: the authored `Command` token is no longer a stand-in "
            "order. docs/CRUISE_COMMAND.md recovered `cruise` as a latch that captures the "
            "ring's ordered pair (unit+980h / unit+984h) and the heading when it becomes "
            "the unit's current command and re-applies what it captured, so a ship whose "
            "ring is still zero holds zero. Milestone 2i wrote throttle 1 / rudder 0 into "
            "the ring for the same token, which is what the latch would produce for a ship "
            "already running at full throttle and is wrong for a ship at rest");
    }
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        // The token every DestroyerGen of this scene authors. An entity that
        // authored none is skipped, exactly as 004f0520's last step skips the
        // queue call when the property is absent.
        if (slot.row.command.empty()) continue;
        const float heading = host.pose_heading_radians(slot);
        const GameCommandRow* row = host.commands.issue(index, slot.row.command,
            slot.row.command_target, slot.ring, heading);
        if (row == nullptr) continue;
        slot.row.command_current = row->current;
        slot.row.command_latched = row->latched;
        slot.row.latch_is_heading = row->fields.is_heading;
        slot.row.latch_steer = row->fields.steer_or_heading;
        slot.row.latch_thrust = row->fields.thrust;
        // Nothing on this path writes an order ring, so no standing order is
        // armed: the ring stays at whatever 00812d40 constructed it with.
        slot.standing_order = false;
        ++host.summary.cruise_orders;
    }
}

bool GameUnitsHost::issue_player_command(const std::string& token,
    const std::string& target_token, const std::string& unit_name) {
    Impl& host = *impl_;
    std::size_t index = host.controlled_index;
    if (!unit_name.empty()) {
        index = host.slots.size();
        for (std::size_t candidate = 0; candidate < host.slots.size(); ++candidate) {
            if (host.slots[candidate]->row.name == unit_name) { index = candidate; break; }
        }
        if (index >= host.slots.size()) {
            host.log.notef("--order-unit \"%s\" names no created instance; the command was "
                "not issued", unit_name.c_str());
            return false;
        }
    } else if (!host.controlled_bound) {
        return false;
    }
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    const float heading = host.pose_heading_radians(slot);
    // Milestone 2s: a fixed point instead of a named entity. `--order
    // moveto:<x>,<z>` is the same order `NavigatorMoveToPos` issues from a
    // mission script: 0088A810's Vector3 branch builds a descriptor whose
    // position_valid byte is set, whose object is null and whose three floats
    // are the point, and 008A2BC0 hands that and the fixed command object to
    // 0077D600. A named entity has no comma in it on any scene of this game, so
    // a target token that parses as two numbers is the point form.
    //
    // Milestone 2r's section 6 is why this exists: `moveto:<unit>` names a ship
    // whose position the goal vector re-reads every frame, and a goal that
    // outruns the chaser cannot be reached, so the arrival latch could never be
    // exercised whatever the AI did.
    float point[3] = {0.0f, 0.0f, 0.0f};
    if (parse_order_position(target_token, point[0], point[2])) {
        bsp::SceneCommandTarget target{};
        target.kind = 0;              // 0088A8A6 clears the word
        target.position_valid = 1;    // 0088A8A9
        target.object = nullptr;      // 0088A8BE
        target.object_id = 0;
        target.position[0] = point[0];
        target.position[1] = point[1];
        target.position[2] = point[2];
        target.trailing = 0.0f;       // 0088A8C7
        char label[64];
        std::snprintf(label, sizeof(label), "(%.1f, %.1f)",
            static_cast<double>(point[0]), static_cast<double>(point[2]));
        const GameCommandRow* placed = host.commands.issue_command_object(index,
            bsp::kCommandObjectMoveTo, target, bsp::kNavigatorIssueFlags,
            std::string("player:--order"), std::string(label), slot.ring, heading);
        if (placed == nullptr) return false;
        slot.row.command = placed->command.empty() ? token : placed->command;
        slot.row.command_target = label;
        slot.row.command_current = placed->current;
        slot.row.command_latched = placed->latched;
        slot.row.latch_is_heading = placed->fields.is_heading;
        slot.row.latch_steer = placed->fields.steer_or_heading;
        slot.row.latch_thrust = placed->fields.thrust;
        ++host.summary.player_orders;
        const double dx = static_cast<double>(point[0]) - slot.motion.position[0];
        const double dz = static_cast<double>(point[2]) - slot.motion.position[2];
        host.log.notef("player command issued to \"%s\": token=\"%s\" resolved=\"%s\" a "
            "FIXED POINT %s through the command builder 0088a810 takes for "
            "NavigatorMoveToPos, not a named entity. The hull is at (%.1f, %.1f), so the "
            "point is %.2f m ahead. current=%d latched=%d",
            slot.row.name.c_str(), token.c_str(), placed->command.c_str(), label,
            static_cast<double>(slot.motion.position[0]),
            static_cast<double>(slot.motion.position[2]),
            std::sqrt(dx * dx + dz * dz), placed->current ? 1 : 0,
            placed->latched ? 1 : 0);
        if (!placed->blocked.empty()) host.log.notef("  %s", placed->blocked.c_str());
        return true;
    }
    const GameCommandRow* row = host.commands.issue(index, token,
        target_token, slot.ring, heading);
    if (row == nullptr) return false;
    slot.row.command = row->command.empty() ? token : row->command;
    slot.row.command_target = target_token;
    slot.row.command_current = row->current;
    slot.row.command_latched = row->latched;
    slot.row.latch_is_heading = row->fields.is_heading;
    slot.row.latch_steer = row->fields.steer_or_heading;
    slot.row.latch_thrust = row->fields.thrust;
    ++host.summary.player_orders;
    host.log.notef("player command issued to \"%s\": token=\"%s\" resolved=\"%s\" "
        "outcome=%s current=%d latched=%d (%s %.3f, thrust %.3f)", slot.row.name.c_str(),
        token.c_str(), row->command.c_str(), row->resolve_outcome.c_str(),
        row->current ? 1 : 0, row->latched ? 1 : 0,
        row->fields.is_heading ? "heading" : "rudder",
        static_cast<double>(row->fields.steer_or_heading),
        static_cast<double>(row->fields.thrust));
    if (!row->blocked.empty()) host.log.notef("  %s", row->blocked.c_str());
    return true;
}

const GameCommandRow* GameUnitsHost::issue_script_command(std::size_t unit_index,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags,
    const std::string& source, const std::string& target_name) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return nullptr;
    GameUnitSlot& slot = *host.slots[unit_index];
    const float heading = host.pose_heading_radians(slot);
    const GameCommandRow* row = host.commands.issue_command_object(unit_index,
        command_object, target, flags, source, target_name, slot.ring, heading);
    // The unit's own row keeps the token the scene authored: a scripted order is
    // a second command on the same director, not a replacement for the first,
    // and GameScriptOrdersHost reports it in its own table.
    return row;
}

void GameUnitsHost::store_commanded_speed_00890e6f(std::size_t unit_index, float speed) {
    Impl& host = *impl_;
    host.commands.store_commanded_speed_00890e6f(unit_index, speed,
        host.summary.simulated_seconds);
}

bsp::CruiseSpeedSetting GameUnitsHost::commanded_speed(std::size_t unit_index) const {
    return impl_->commands.commanded_speed(unit_index);
}

float GameUnitsHost::mission_clock() const noexcept {
    return impl_->summary.simulated_seconds;
}

void GameUnitsHost::run_director_steps_00836920() {
    Impl& host = *impl_;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        if (!slot.state->active) continue;
        const float heading = host.pose_heading_radians(slot);
        const GameDirectorStepOutcome outcome = host.commands.director_step_00836920(
            index, host.is_controlled(slot), host.summary.simulated_seconds, slot.ring,
            heading);
        if (outcome.reissued == bsp::DirectorDefaultCommand::None) continue;
        // The idle tail's own choice becomes the unit's standing command when
        // the slot push took it; 0071be40 is what answers that.
        slot.row.command_current = slot.row.command_current
            || host.commands.holds_cruise(index);
    }
}

const GameCommandsHost& GameUnitsHost::commands() const noexcept { return impl_->commands; }

void GameUnitsHost::set_controlled_unit_004c0890(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    host.controlled_index = index;
    ControlledUnitQueryBinding query(slot.class_id);
    SetControlledUnitBinding binding(host);
    const bsp::ControlledUnitGlobals globals
        = bsp::set_controlled_unit_004c0890(true, query, binding);
    host.done("ControlledUnit::set_controlled_unit", 0x004c0890u);
    for (std::unique_ptr<GameUnitSlot>& other : host.slots) other->row.controlled = false;
    slot.row.controlled = true;
    host.summary.controlled_bound = globals.unit_present;
    host.summary.controlled_index = index;
    host.summary.controlled_name = slot.row.name;
    host.log.notef("controlled unit: 00e188d8 = \"%s\" (%s %s, party %d); 00e188dc %s; "
        "class_id=%d IsKindOf(0Fh)=%d IsKindOf(18h)=%d; "
        "listener handle/publication adapters remain unimplemented",
        slot.row.name.c_str(), slot.row.class_name.c_str(), slot.row.type_symbol.c_str(),
        slot.row.party, globals.listener_present ? "published a handle" : "was cleared",
        slot.class_id, query.unit_is_kind_of(0x0f) ? 1 : 0,
        query.unit_is_kind_of(0x18) ? 1 : 0);
}

void GameUnitsHost::issue_player_order(float throttle, float rudder) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[host.controlled_index];
    slot.standing_order = true;
    slot.standing_throttle = throttle;
    slot.standing_rudder = rudder;
    host.issue_into_ring(slot, throttle, rudder);
    ++host.summary.player_orders;
    host.log.notef("player order issued to \"%s\": throttle=%.3f rudder=%.3f through "
        "00816a40, clamped into [-2,+2] by 00815440", slot.row.name.c_str(),
        static_cast<double>(throttle), static_cast<double>(rudder));
}

void GameUnitsHost::update_entity_008255b0(std::size_t index, float scaled_delta) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    UnitInstanceBinding binding(host, slot);
    bsp::update_unit_instance_008255b0(*slot.state, slot.class_block, binding, scaled_delta);
    ++slot.row.instance_updates;
    ++host.summary.instance_updates;
    host.done("World::update_entity", 0x008255b0u);
}

void GameUnitsHost::motion_step_00825f20(float step_seconds) {
    Impl& host = *impl_;
    if (host.slots.empty()) return;
    if (!host.logged_precision) {
        // The second half of docs/X87_CONTROL_WORD.md's read, taken where that
        // document says it matters: the fixed step that runs the reconstruction
        // of 00825f20, after the Direct3D 9 device exists. The first half is in
        // game_main before Direct3D is created. Neither read changes anything.
        host.logged_precision = true;
        const unsigned long field = x87_precision_field();
        host.log.notef("x87 precision at the fixed simulation step: %s (_controlfp_s & "
            "_MCW_PC = 0x%08lx). The shipped executable runs 00825f20's own float "
            "expressions on the x87 stack at this precision; this reconstruction computes "
            "them with SSE2 float32 under /fp:strict, which rounds every intermediate to "
            "24 bits of mantissa as well",
            x87_precision_name(field), field);
    }
    ++host.summary.motion_steps;
    host.summary.simulated_seconds += step_seconds;
    // Milestone 2m: the weapon director's own step, before the motion pass that
    // reads what the step decided. 00836920's caller is the unit update's
    // director block, which this process does not reach, so the position is the
    // executable's decision and is recorded as one.
    run_director_steps_00836920();
    // Milestone 2n: the ship AI controller 009f50e0 for every created unit, and
    // the weapon director's automatic target think 009f5da0 beside it. Neither
    // has a caller in the call graph, so the position is the executable's
    // decision and is recorded as one; it runs before the motion pass because
    // the motion's own head at 00825f2c consumes the order slot 009f4d10
    // published on this step.
    if (host.ship_ai != nullptr) {
        host.ship_ai->controller_step(step_seconds);
        host.ship_ai->log_sample(host.summary.motion_steps, 10);
    }
    // Milestone 2t: the gunnery pass, the gun bots' aim ticks, 0072D130's 0ADh
    // send and the projectiles. 00875B90 runs a tick element's sub-nodes in
    // wave 2 of the fixed step at 0.05 s (docs/FIXED_STEP_JOB_WAVES.md), and
    // the pass and the gun bots are both sub-nodes of a tick element, so they
    // run on this step. Their position relative to the motion pass is the
    // executable's decision and is recorded as one: the guns read the pose the
    // previous step left, which is what a sub-node of unit+310h does.
    // 00A32D50, slot +8h of vtable 00D23168: the composition pass every step and
    // the party think on its own 3 to 5 s clock. It runs before the gunnery pass
    // because an order issued this step is what the gun chain then acts on.
    if (host.ai != nullptr) host.ai->fixed_step(step_seconds);
    if (host.gunnery != nullptr) {
        // 007DE3A0 BSP_PlaneActuatorBlock_Step, slot 3 of vtable 00D0862C on the
        // block at unit+DECh. It runs on the plane's fixed step, before the
        // gunnery pass, and its only effect is to move the three channels and
        // lower the aggregate flag at +11h once all three come to rest.
        // docs/TORPEDO_RELEASE_SPAWN.md.
        for (std::unique_ptr<GameUnitSlot>& unit_slot : host.slots) {
            if (unit_slot == nullptr) continue;
            bsp::plane_actuator_block_step_007de3a0(unit_slot->actuator_block_dec,
                step_seconds, static_cast<unsigned>(host.summary.motion_steps));
        }
        host.gunnery->fixed_step(step_seconds);
        host.gunnery->log_sample(host.summary.motion_steps, 100);
    }
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        if (!slot.state->active) continue;
        // The native receiver is the class's unit+310h tick node. AirField's
        // 8E4h allocation, for example, has entry 006D2510 and cannot contain
        // the ship-only +9C0h/+1018h fields. Resolve before any ship operation.
        if (!slot.motion_dispatch.runs_ship_base()) {
            if (slot.motion_dispatch.entry == 0x00953cc0u) {
                // These are the canonical current assignments. Role4==8
                // proves the input predicate false for EVERY local-slot value.
                // Flag634==0 skips the unresolved +61h read; role0==8 skips
                // participant availability. Recheck on every call.
                if (slot.current_roles_01ac[0] == bsp::kUnitRoleTableFill
                    && slot.current_roles_01ac[4] == bsp::kUnitRoleTableFill
                    && slot.generic_flag_634 == 0) {
                    class GenericTickCalls final : public bsp::UnitTickAdvanceHost {
                    public:
                        explicit GenericTickCalls(GameUnitSlot& unit) : unit_(unit) {}
                        void unit_virtual_5c_is_kind_of(int id) override {
                            (void)bsp::unit_is_kind_of(unit_.class_id, id);
                        }
                        void unit_virtual_1f0_advance(float) override {
                            bsp::unit_generic_input_unassigned_0095dd71(
                                unit_.generic_input_63c);
                        }
                        bool unit_role_still_available_00927f10(int) override {
                            throw std::logic_error("generic tick role guard violated");
                        }
                        // All three proven generic leaf tables use006D1F20:
                        // RET4, no input read and no store. New C++ ABI here.
                        void unit_virtual_1d8_advance(float) override {}
                    private:
                        GameUnitSlot& unit_;
                    } calls(slot);
                    bsp::UnitTickAdvanceState view;
                    view.notify_code_528h = slot.generic_notify_528;
                    view.flag_634h = slot.generic_flag_634;
                    view.timer_6f8h = slot.generic_timer_6f8;
                    view.timer_6fch = slot.generic_timer_6fc;
                    view.role_1ach = slot.current_roles_01ac[0];
                    view.suppress_1d8_520h = slot.generic_suppress_520;
                    // view.gate_byte_61h is not a native value and is not read:
                    // the verified zero flag short-circuits that expression.
                    bsp::unit_tick_advance_sim_00953cc0(view, step_seconds, calls);
                    slot.generic_flag_634 = view.flag_634h;
                    slot.generic_timer_6f8 = view.timer_6f8h;
                    slot.generic_timer_6fc = view.timer_6fch;
                    slot.generic_suppress_520 = view.suppress_1d8_520h;
                    ++host.summary.generic_tick_calls;
                    host.done("UnitMotion::generic_constructor_state_tick", 0x00953cc0u);
                    host.done("UnitMotion::generic_unassigned_input", 0x0095dc40u);
                    continue;
                }
                ++host.summary.generic_tick_unavailable;
            }
            if (slot.motion_dispatch.entry == 0x007ce040u) {
                // The plane fixed step. docs/PLANE_FLIGHT_CORE_LAW.md proves the
                // arm selection is reachable: seed unit+900h to 7 as 007C6481
                // does and the free-flight arm runs. The arms themselves are not
                // wired here - this binds the sequence and reports which arm the
                // native would take, so the next step has a harness and the
                // selection is measurable before any motion is claimed.
                class PlaneBinding final : public bsp::PlaneFlightHost {
                public:
                    PlaneBinding(GameUnitsHost::Impl& owner, GameUnitSlot& unit)
                        : owner_(owner), unit_(unit) {}
                    bool unit_game_object_tick_00953cc0(float step) override {
                        // 00953CC0 is the first call of 007CE040, so this is
                        // where the step the whole fixed step runs on is known.
                        // The 007CE9FD stage needs it and gets no argument of
                        // its own in the reconstructed sequence.
                        fixed_step_seconds_ = step;
                        return unit_.generic_suppress_520;
                    }
                    void class_input_poll_0095dc40(float) override {}
                    void out_of_action_countdown_007c6c30(float) override {}
                    bool free_flight_gate_00d06130_38() override {
                        // 0074E210 BSP_PlaneControlMode_IsFreeFlight: +1D4h == 7.
                        return unit_.plane_control_mode_900 == 7;
                    }
                    void accumulate_airborne_time(float step) override {
                        unit_.plane_airborne_908 += step;   // 007CEC4E
                    }
                    void free_flight_007cc2f0(float step) override {
                        ++owner_.summary.plane_arm_free_flight;
                        // 007CE040 calls 007BB920 at 007CE865 and the latch
                        // 007B9770 at 007CE96F, in that address order with the
                        // motion arm between, so the think and commit run first
                        // and control_step_007da710's latch runs last.
                        refresh_attitude_007c1900();
                        pilot_think_and_commit(step);
                        // The class field the law actually depends on. StallSpd
                        // is the only authored one; everything else is a
                        // PlaneGlobals default the mirror fills at load.
                        bsp::PlaneFreeFlightClass cls;
                        // desc+184h, desc+174h, desc+170h - the three class
                        // fields 007DB760, 007DBD3A and 007DBD50 read. Leaving
                        // x_drag and y_drag at their 0.0f declaration multiplied
                        // the whole body-damping block by zero, so a plane's
                        // velocity kept whatever direction it was seeded with
                        // while advance_pose_0085e4d0 turned the nose away from
                        // it: at the torpedo release instant 0092D730 read
                        // -6.8 m/s against a 141 m/s velocity, the two vectors
                        // almost perpendicular. docs/TORPEDO_RUN_IN_VELOCITY.md.
                        if (unit_.plane_stall_spd > 0.0f) {
                            cls.stall_spd = unit_.plane_stall_spd;
                        }
                        cls.x_drag = unit_.plane_x_drag;
                        cls.y_drag = unit_.plane_y_drag;
                        bsp::PlaneFreeFlightTuning tuning;
                        // Every field of PlaneFreeFlightTuning is a Dynamics/*
                        // row inside the 00F872F0 mirror, and the mirror is now
                        // filled from the installation's PlaneGlobals.lua by the
                        // recovered 007E2A20. Its struct defaults stay as the
                        // fallback for a run where the data file did not load;
                        // when it did, the authored numbers win.
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            tuning.ceiling = g.dynamics_ceiling;
                            tuning.ceiling_force = g.dynamics_ceiling_force;
                            tuning.drag_func_power = g.dynamics_drag_func_power;
                            tuning.drag_range_min = g.dynamics_spd_multipliers_drag_range_min;
                            tuning.drag_range_max = g.dynamics_spd_multipliers_drag_range_max;
                            tuning.level_flight = g.dynamics_spd_multipliers_level_flight;
                            tuning.lost_drag_time = g.dynamics_dead_meat_lost_drag_time;
                            tuning.extra_gravity_mul = g.dynamics_dead_meat_extra_gravity_mul;
                            tuning.accel_cheat_mul = g.dynamics_accel_cheat_mul;
                        }
                        bsp::PlaneFreeFlightState state;
                        for (int i = 0; i < 3; ++i) {
                            state.world_velocity[i] = unit_.plane_world_velocity[i];
                        }
                        // ctl+0B0h, row-major, body = M * world. The pose rows
                        // ARE that matrix: row i is body axis i expressed in
                        // world, so dot(row_i, world_vec) is the body component.
                        // The frame comes from the authored placement through
                        // 0046cf40; nothing here invents a rotation.
                        const float* const rows[3] = {unit_.motion.pose_row0,
                            unit_.motion.pose_row1, unit_.motion.pose_row2};
                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                state.world_to_body[r * 3 + c] = rows[r][c];
                            }
                            // ctl+3Ch..44h, which 007D9C39 rebuilds from the
                            // world velocity through this same matrix each step.
                            state.body_velocity[r] =
                                rows[r][0] * unit_.plane_world_velocity[0] +
                                rows[r][1] * unit_.plane_world_velocity[1] +
                                rows[r][2] * unit_.plane_world_velocity[2];
                        }
                        // ctl+44h, the body forward component. The carrier term
                        // 007D99C0 adds to it is zero off a deck. Under the
                        // identity frame this is the world +Z the previous
                        // revision took, so the acceptance test is unmoved.
                        state.forward_speed = state.body_velocity[2];
                        state.world_altitude = unit_.motion.position[1];
                        state.lost_drag_timer = unit_.plane_lost_drag_timer_c3c;
                        state.pitch = unit_.plane_pitch_angle_c64;
                        // --- 007DB744-007DB80A, thrust; 007D9050 ---
                        // The gate at 007DB76C is unit+0BBCh > 0.01f. The rule is
                        // a = desc+164h Accel * throttle, times tuning+330h
                        // TurboMultiplier when ctl+4h and desc+604h when ctl+5h
                        // (neither modelled, so neither is applied), times the
                        // fall cheat when the pitch is negative, clamped to
                        // [0, 100]. The call site then scales by unit+0CC8h and
                        // either 008E6430(6, unit) or 1.0f; both are unmodelled
                        // and taken as 1.0. Labelled partial on those three.
                        if (unit_.plane_latched_throttle > 0.01f) {
                            float a = unit_.plane_accel * unit_.plane_latched_throttle;
                            if (state.pitch < 0.0f) {
                                float fall_mul = 2.6f;      // tuning+324h
                                float range1 = 0.174533f;   // tuning+328h DEG(10)
                                float range2 = 1.047198f;   // tuning+32Ch DEG(60)
                                if (owner_.lua.plane_globals_loaded()) {
                                    const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                                    fall_mul = g.dynamics_accel_cheat_fall_mul;
                                    range1 = g.dynamics_accel_cheat_fall_pitch_range_1;
                                    range2 = g.dynamics_accel_cheat_fall_pitch_range_2;
                                }
                                const float u = bsp::clamped_interpolate_00419010(
                                    range1, 0.0f, range2, 1.5707964f, -state.pitch);
                                a *= 1.0f + (fall_mul - 1.0f)
                                    * static_cast<float>(std::sin(static_cast<double>(u)));
                            }
                            if (a < 0.0f) a = 0.0f;
                            if (a > 100.0f) a = 100.0f;
                            state.thrust_accel = a;
                        }
                        // --- 007DBA32-007DBC76, drag; 007D9140 ---
                        // 007C4990-007C499C derives the coefficient desc+50Ch as
                        // Accel / MaxSpd^2 (FDIVP ST2,ST0 then FDIVP over
                        // desc+188h twice), which is exactly what makes the
                        // equilibrium airspeed MaxSpd at full throttle.
                        if (unit_.plane_max_spd > 0.0f) {
                            float max_drag_spd_mul = 0.1f;   // tuning+310h
                            float min_drag_spd_mul = 0.1f;   // tuning+314h
                            float max_drag_pitch = 1.0f;     // tuning+318h
                            if (owner_.lua.plane_globals_loaded()) {
                                const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                                max_drag_spd_mul = g.dynamics_max_drag_spd_mul;
                                min_drag_spd_mul = g.dynamics_min_drag_spd_mul;
                                max_drag_pitch = g.dynamics_max_drag_pitch;
                            }
                            const float coefficient =
                                unit_.plane_accel
                                / (unit_.plane_max_spd * unit_.plane_max_spd);
                            const float world_speed = std::sqrt(
                                unit_.plane_world_velocity[0] * unit_.plane_world_velocity[0]
                                + unit_.plane_world_velocity[1] * unit_.plane_world_velocity[1]
                                + unit_.plane_world_velocity[2] * unit_.plane_world_velocity[2]);
                            // The speed floor, whose two endpoints are both
                            // MaxSpd * 0.1 in this installation's PlaneGlobals.
                            const float floor_speed = bsp::clamped_interpolate_00419010(
                                0.0f, unit_.plane_max_spd * min_drag_spd_mul,
                                max_drag_pitch, unit_.plane_max_spd * max_drag_spd_mul,
                                state.pitch);
                            float v = world_speed > floor_speed ? world_speed : floor_speed;
                            float closed = 1.0f - unit_.plane_latched_throttle;
                            if (closed < 0.0f) closed = 0.0f;
                            if (closed > 1.0f) closed = 1.0f;
                            const float elevator = std::fabs(unit_.plane_latched_controls[1]);
                            const float k = closed * closed * unit_.plane_glide_rate + 1.0f
                                + unit_.plane_drag_pitch_ratio * elevator;
                            const float brake =
                                unit_.plane_air_brake_drag * unit_.plane_latched_air_brake + 1.0f;
                            // 007D926C-007D929B multiplies in -sgn(v), so the
                            // value opposes forward motion.
                            const float sign = v > 0.0f ? 1.0f : (v < 0.0f ? -1.0f : 0.0f);
                            float d = -sign * v * v * k * coefficient * brake;
                            // 007DBB0E / 007DBB23, the two pitch ramps the call
                            // site multiplies in. Both endpoints of the second
                            // are 1.0 for a healthy aircraft, so r2 is 1.0 until
                            // the DeadMeat timer runs.
                            const float r1 = bsp::clamped_interpolate_00419010(
                                0.0f, 1.0f, tuning.lost_drag_time, 0.0f,
                                unit_.plane_lost_drag_timer_c3c);
                            const float r2 = bsp::clamped_interpolate_00419010(
                                -0.3f, r1, 0.1f, 1.0f, state.pitch);
                            state.drag_accel = d * r2;
                        }
                        state.airborne_time = unit_.plane_airborne_908;
                        const bsp::PlaneDynAccumulators acc =
                            bsp::accumulate_free_flight_007db680(state, cls, tuning, step);
                        const bsp::PlaneBodyAcceleration body =
                            bsp::fold_world_into_body_007d8470(acc, state.world_to_body);
                        // 007D8470 returns a BODY-frame acceleration - its own
                        // name says so, and free_flight_world_up_acceleration in
                        // the same header rotates the result back "through the
                        // transpose of ctl+0B0h" to get a world quantity. This
                        // loop used to add it straight to a world velocity.
                        //
                        // That was invisible for the whole history of this
                        // reconstruction, because nothing ever rotated a plane
                        // and the two frames agree at the identity. The first
                        // run in which a bot actually turned showed it at once:
                        // the planes accelerated to 636 m/s and flew off. A
                        // frame error that only a working control law can
                        // expose is worth the note.
                        //
                        // world = M^T * body, with M's rows the pose rows.
                        float world_accel[3] = {0.0f, 0.0f, 0.0f};
                        for (int c = 0; c < 3; ++c) {
                            for (int r = 0; r < 3; ++r) {
                                world_accel[c] += rows[r][c] * body.total[r];
                            }
                        }
                        for (int i = 0; i < 3; ++i) {
                            unit_.plane_world_velocity[i] += world_accel[i] * step;
                            unit_.motion.position[i] += unit_.plane_world_velocity[i] * step;
                        }
                        // 0092D730 takes the body's linear velocity from
                        // 00C31F40 and dots it with the third row of the body
                        // axis matrix. It does not care what moved the body, so
                        // the ONE body velocity field has to carry a plane's
                        // motion as well as a ship's. Only the ship hydro path
                        // wrote it (set_linear_velocity_00c37e50), so
                        // unit_forward_speed_0092d730 answered 0.0 for every
                        // flying plane, and an air-dropped torpedo inherited no
                        // velocity at all. docs/TORPEDO_RELEASE_GEOMETRY.md.
                        unit_.motion.linear_velocity = bsp::OceanVec3{
                            unit_.plane_world_velocity[0],
                            unit_.plane_world_velocity[1],
                            unit_.plane_world_velocity[2]};
                        // The 3D step length. The seed no longer lies along
                        // world +Z, so one component would understate it.
                        const float* const wv = unit_.plane_world_velocity;
                        owner_.summary.plane_distance_moved +=
                            std::sqrt(wv[0] * wv[0] + wv[1] * wv[1] + wv[2] * wv[2]) * step;
                        control_step_007da710(step, state.forward_speed);
                        advance_pose_0085e4d0(step);
                        // 007C6500 BSP_PlaneTickElement_AdvancePose, tick-element
                        // slot +4h of all nine plane vtables, is where the native
                        // publishes a plane's pose, and it is a DIFFERENT element
                        // from the fixed step 007CE040 this arm stands in for.
                        // Its tail at 007C658C picks 007D8230 or 007D9F60 on
                        // unit+210h; 007D8230 copies the 64-byte committed matrix
                        // out of unit+674h (007D8252 LEA ESI,[EAX+674h]), advances
                        // its translation row by step * (unit+810h..818h +
                        // ctl+18h..20h), and writes it back through
                        // 007D8293 LEA ECX,[EBX+74h] / 007D831C CALL 004134F0
                        // BSP_Matrix_Copy4x4X87. unit+74h is the PUBLISHED pose
                        // every other system reads.
                        //
                        // This host has no +4h element - GameFixedStepHost's five
                        // 68h groups at 00F876C0 are empty, docs/PLANE_UNIT_TICK.md
                        // - and publish_pose had exactly two call sites, unit
                        // creation and the ship body path. So every consumer of
                        // unit_pose saw each aircraft frozen at its spawn
                        // placement for the whole life of this reconstruction:
                        // the gunnery host spawned an air-dropped torpedo at
                        // "700 m" in a log whose own census read -14556 m at the
                        // same instant. docs/PLANE_POSE_THROTTLE_ALTITUDE.md.
                        //
                        // The interpolation the native applies is a fraction of
                        // one frame's motion; this host has no sub-frame time, so
                        // the copy is taken at the committed pose, which is
                        // 007D8230 with its step at zero.
                        GameUnitsHost::Impl::publish_pose(unit_);
                        // 007CC523-007CC562, the free-flight arm own water
                        // test, and the answer to why a plane here could fly to
                        // -400 m. The arm samples the sea under the aircraft and
                        // hands a contact to 007CB7F0:
                        //
                        //   h = 0078CF20(unit+0FCh, unit+104h)
                        //   if (unit+100h < desc+194h + h + desc+508h) 007CB7F0()
                        //
                        // 007C4CF8-007C4D03 derives desc+508h as
                        // `-[EDI+4] - desc+194h`, so the two SwimHeight terms
                        // cancel and the real line is `h - [EDI+4]`, the
                        // aircraft's lowest point touching the water.
                        // SUBSTITUTION, labelled: [EDI+4] is the model bound
                        // this host does not carry, so it stands at zero and the
                        // test is the ORIGIN crossing the sea rather than the
                        // belly. For a torpedo bomber that is a metre or two.
                        //
                        // 007CB7F0's tail at 007CB92C is what matters: when
                        // unit+900h is 7, 4 or 5 it calls BSP_Plane_SetFlightState(6),
                        // and 0074E210's free-flight gate is unit+900h == 7, so
                        // the aircraft leaves this arm for the water surface law
                        // 007DCDD0. docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md.
                        if (unit_.plane_control_mode_900 == 7) {
                            OceanFieldBinding sea(owner_);
                            const float water = bsp::ocean_water_height_0078cf20(
                                unit_.motion.position[0], unit_.motion.position[2], sea);
                            const float line = water + unit_.plane_swim_height
                                + (0.0f - unit_.plane_swim_height);
                            if (unit_.motion.position[1] < line) {
                                ++unit_.plane_water_contacts;
                                // 007CB92C: 7, 4 and 5 all go to 6. The
                                // "powerlost" effect, the 0090F6C0(unit, 3)
                                // damage call and the 0C3h session message are
                                // contracts, unread here.
                                unit_.plane_control_mode_900 = 6;
                                owner_.record("Plane::water_contact_007cb7f0", 0x007cb7f0u);
                                if (unit_.plane_water_contacts == 1) {
                                    owner_.log.notef("plane water contact: unit=%s "
                                        "alt=%.2f water=%.2f |v|=%.2f state 7 -> 6 "
                                        "(007CB7F0 tail 007CB92C); the free-flight "
                                        "gate 0074E210 is now false and the water "
                                        "surface law 007DCDD0 is a contract",
                                        unit_.row.name.c_str(),
                                        static_cast<double>(unit_.motion.position[1]),
                                        static_cast<double>(water),
                                        static_cast<double>(std::sqrt(
                                            unit_.plane_world_velocity[0]
                                                * unit_.plane_world_velocity[0]
                                            + unit_.plane_world_velocity[1]
                                                * unit_.plane_world_velocity[1]
                                            + unit_.plane_world_velocity[2]
                                                * unit_.plane_world_velocity[2])));
                                }
                            }
                        }
                        // The release-order issue used to run here. It does not
                        // belong to the free-flight arm: 007CEA8D sits at
                        // 007CE9FD, past the latch 007CE96F and past the arm
                        // dispatch, and its own mode test at 007CEA33 accepts
                        // unit+900h of 4, 5, 6 or 7. It now runs from
                        // latch_control_input_007b9770, which is the sequence
                        // position of 007CE96F. docs/TORPEDO_ISSUE_TIMING.md.
                        owner_.done("PlaneMotion::free_flight_007cc2f0", 0x007cc2f0u);
                    }
                    // 007D9C80 then 0085E4D0, the two steps that turn a plane.
                    // The controller's body angular velocity goes to world
                    // through the live pose - 0042D0D0 is a row-vector product,
                    // so with the pose rows being the body axes in world this
                    // is w[0]*row0 + w[1]*row1 + w[2]*row2 - and 0085E4D0 then
                    // rotates the pose about that world axis.
                    //
                    // It does nothing at all today, and that is the honest
                    // state rather than a hedge: plane_body_angular is never
                    // written, so `w` is the zero vector, 0085E4D0 takes its
                    // 0085E871 exit without writing a pose, and the counter
                    // below stays at zero to say so. The link this is waiting
                    // on is a path from an installed bot task to the latched
                    // control inputs at ctl+BB0h/BB4h/BB8h; 007DA710 turns
                    // those into this angular velocity and is reconstructed
                    // (bsp::plane_control_axis_step_007da710), the path to them
                    // is not.
                    // 0099ACD0's think gate and the stage of its tick that this
                    // host can support, then 007BB920's commit.
                    //
                    // What is here: the accumulator at bot+70h and its 0.09 s
                    // threshold (gate 6), 0099B450's seed, 0099BC00's slew and
                    // clamp, 007B8C90's command block with its pending byte, and
                    // 007BB920 -> 007BB6E0's quantisation into the live block.
                    //
                    // What is NOT here, named rather than glossed:
                    //
                    // * **The planner.** 0099D300 fills the slots' `desired`
                    //   fields, and nothing does that here, so every slot keeps
                    //   `desired == current` from the seed, every slew returns
                    //   the live value bit-exactly, and the quantised result is
                    //   what was already there. The whole stage is a faithful
                    //   no-op until a planner exists.
                    // * **0099BF30's band repair**, which runs between the slew
                    //   and the command block and is the LAST WRITER of all five
                    //   command floats. Its body has since been read
                    //   (docs/PILOT_COMMAND_BAND_REPAIR.md) and on a freshly
                    //   constructed plan it writes NOTHING: the three band
                    //   tables are constructed empty and the throttle ceiling
                    //   1.0f, and both guards return early. So the pass-through
                    //   here is well-founded rather than a placeholder - with
                    //   the caveat that 0099B450 does not reset either field and
                    //   no writer of them was found, by a scan blind to SIB and
                    //   block copies.
                    // * **Eleven of the twelve gates** in
                    //   docs/PILOT_BOT_TICK_GATES.md. This host has no bot
                    //   object, no task vector and no per-slot state to gate on,
                    //   so only the think interval is modelled. A plane here
                    //   thinks unconditionally; the native's would also need a
                    //   live task.
                    // ------------------------------------------------
                    // 009D4850, the torpedo task's per-tick arm (task vtable
                    // slot +64h). docs/TORPEDO_TASK_ARM.md carries the rules.
                    //
                    // This host has no bot object and no task vector, so the
                    // slot below stands in for the single torpedo task an
                    // ordered aircraft would carry. Every native call site the
                    // arm reaches is a method on this host; the ones whose
                    // bodies this packet did not read are recorded through the
                    // unimplemented-host mechanism.
                    // ------------------------------------------------
                    // 007C0D90 -> 007EEF30 -> 007BCBE0, the release-order
                    // budget. docs/TORPEDO_RELEASE_ORDERS.md. This host has one
                    // aircraft per slot rather than a shared pilot control
                    // block, so the block's unit array at ctl+3D0h is modelled
                    // as the ordered aircraft of the same flight, in slot order,
                    // and the first of them is the leader.
                    // ------------------------------------------------
                    struct TorpedoReleaseOrderBinding final
                        : bsp::TorpedoReleaseOrderHost {
                        TorpedoReleaseOrderBinding(GameUnitsHost::Impl& owner,
                                                   GameUnitSlot& slot)
                            : owner_(owner), slot_(slot) {}

                        bool unit_carries_droppable_device_007c0d90() override {
                            // 007C0DB8-007C0E61: a device of class 25h holding
                            // ordnance 2Ah whose descriptor answers 2Ch, 2Bh or
                            // 33h. 2Bh is Torpedo and its descriptor answers 2Ah
                            // too, so the slot's ordnance mask decides it here.
                            // The six vtable slots are contract: unread.
                            owner_.log.unimplemented(
                                "Plane::droppable_device_walk", "007c0d90");
                            const bsp::OrdnanceKindSet set{slot_.ordnance_mask};
                            return bsp::ordnance_has_torpedo_2bh(set);
                        }
                        void set_release_pending_c25(bool value) override {
                            slot_.torpedo_release_pending_c25 = value;  // 007C0EE2
                        }
                        void pre_issue_hook_007ee7f0() override {
                            owner_.log.unimplemented(
                                "PilotControl::pre_issue_hook", "007ee7f0");
                        }
                        bsp::ReleaseOrderIssueInputs read_issue_inputs() override {
                            // 007EEF40 gates on ctl+390h > ctl+374h.
                            // docs/TORPEDO_RELEASE_ORDERS.md section (5).
                            bsp::ReleaseOrderIssueInputs in;
                            const int count = controlled_unit_count();
                            in.controlled_count_3cc = count;
                            in.force_flag_378 = false;   // ctl+378h

                            // ctl+374h: 007EE7F0's armed fraction, through the
                            // reconstructed rule rather than a constant. The
                            // per-unit round count is 007C1F60, which sums
                            // 006E3500 over the aircraft's class 25h devices
                            // holding ordnance 2Ah. This host has no device
                            // model, so a torpedo-armed aircraft stands in with
                            // one round until it releases: a SUBSTITUTION for
                            // 007C1F60, not a reading of it.
                            std::vector<bool> enabled;
                            std::vector<int> rounds;
                            std::vector<bool> is_caller;
                            for (int i = 0; i < count; ++i) {
                                const GameUnitSlot* const u = controlled(i);
                                enabled.push_back(u != nullptr);
                                int n = 0;
                                if (u != nullptr) {
                                    const bsp::OrdnanceKindSet set{u->ordnance_mask};
                                    if (bsp::ordnance_has_torpedo_2bh(set)) {
                                        n = 1 - u->torpedo_releases;
                                        if (n < 0) n = 0;
                                    }
                                }
                                rounds.push_back(n);
                                is_caller.push_back(u == &slot_);
                            }
                            // std::vector<bool> is a bit proxy, so copy out.
                            std::vector<unsigned char> enabled_bytes(
                                enabled.begin(), enabled.end());
                            std::vector<unsigned char> caller_bytes(
                                is_caller.begin(), is_caller.end());
                            static_assert(sizeof(bool) == sizeof(unsigned char),
                                          "bool and unsigned char must share a size");
                            bsp::FlightArmedFractionInputs frac;
                            frac.controlled_count_3cc = count;
                            frac.unit_enabled_5c =
                                reinterpret_cast<const bool*>(enabled_bytes.data());
                            frac.unit_rounds_007c1f60 = rounds.data();
                            frac.unit_is_caller =
                                reinterpret_cast<const bool*>(caller_bytes.data());
                            in.authorise_threshold_374 =
                                bsp::flight_armed_fraction_007ee7f0(frac);

                            // ctl+390h: 0079CD36 seeds it with
                            // *(float*)(unit->+538h + A0h) * 0.95. The class
                            // descriptor field is unread - offset +A0h is shared
                            // by too many object types for a byte scan to name it
                            // - so the descriptor value stands in at 1.0 while
                            // the 0.95 scale is the native's own. SUBSTITUTION,
                            // with the address.
                            owner_.log.unimplemented(
                                "PlaneClass::issue_threshold_a0", "0079cd36");
                            in.authorise_value_390 = static_cast<float>(
                                1.0 * bsp::kIssueThresholdScale_00ceffb0);

                            slot_.torpedo_issue_threshold_390 =
                                in.authorise_value_390;
                            slot_.torpedo_armed_fraction_374 =
                                in.authorise_threshold_374;
                            return in;
                        }
                        int controlled_unit_count() override {
                            int n = 0;
                            for (const auto& s : owner_.slots) {
                                if (is_flight_member(*s)) ++n;
                            }
                            return n;
                        }
                        // ctl+3D0h / ctl+3CCh, the squadron's own unit array.
                        // 007F2C60 BSP_PlaneSquadronTickableEntity_Construct
                        // builds the block (ctl+34Ch at 007F2CDB, the attack
                        // mode ctl+370h at 007F2DC1) from
                        // 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen, which
                        // runs at launch, not on an order. Membership by
                        // installed task was therefore wrong: it made the
                        // flight empty until the order arrived, and an empty
                        // array skips the whole loop at 007EEF54. This host has
                        // no squadron object, so carrying torpedo ordnance
                        // stands in for it. SUBSTITUTION.
                        static bool is_flight_member(const GameUnitSlot& s) {
                            const bsp::OrdnanceKindSet set{s.ordnance_mask};
                            return bsp::ordnance_has_torpedo_2bh(set);
                        }
                        bool unit_lacks_follow_target_007b8ad0(int index) override {
                            GameUnitSlot* const u = controlled(index);
                            // 007B8AD0 tests unit+9D8h. An ordered aircraft has
                            // a command target, which is what stands in for it.
                            return u == nullptr || u->command_target_plus_one == 0;
                        }
                        bsp::ReleaseOrderSetInputs read_set_inputs(
                            int index, int requested) override {
                            bsp::ReleaseOrderSetInputs in;
                            in.requested_count = requested;
                            // unit+5Ch, the scene-node enabled byte that
                            // BSP_SceneNode_Enable owns. Every live slot here is
                            // registered in the world lists.
                            GameUnitSlot* const u = controlled(index);
                            in.scene_node_enabled_5c = u != nullptr;
                            // 007B9140: kind 17h on the unit, else a device
                            // holding 2Ah. The torpedo descriptor answers 2Ah.
                            bool holds_2ah = false;
                            if (u != nullptr) {
                                const bsp::OrdnanceKindSet set{u->ordnance_mask};
                                holds_2ah = bsp::ordnance_has_torpedo_2bh(set);
                            }
                            const bool device_table[1] = {holds_2ah};
                            bsp::CanDropOrdnanceInputs can;
                            can.unit_is_kind_17h = false;
                            can.device_count_994 = 1;
                            can.device_holds_2ah = device_table;
                            in.can_drop_ordnance =
                                bsp::unit_can_drop_ordnance_007b9140(can);
                            return in;
                        }
                        void write_release_order_count(int index, int count) override {
                            GameUnitSlot* const u = controlled(index);
                            if (u == nullptr) return;
                            u->torpedo_release_orders_c58 = count;   // 007BCBFD
                            if (count > u->torpedo_peak_release_orders_c58) {
                                u->torpedo_peak_release_orders_c58 = count;
                            }
                        }

                       private:
                        GameUnitSlot* controlled(int index) const {
                            int n = 0;
                            for (const auto& s : owner_.slots) {
                                if (!is_flight_member(*s)) continue;
                                if (n == index) return s.get();
                                ++n;
                            }
                            return nullptr;
                        }
                        GameUnitsHost::Impl& owner_;
                        GameUnitSlot& slot_;
                    };

                    // ------------------------------------------------
                    // 009D3420, the approach update the arm calls at 009D486F
                    // before the transition rule. docs/TORPEDO_APPROACH_UPDATE.md.
                    // The approach's target (approach+CCh) is the ordered
                    // aircraft's command target, and its target point is that
                    // unit's world position: the approach vtable slot 0 of the
                    // torpedo class was not read, so the point is the target's
                    // own position here rather than a lead.
                    // ------------------------------------------------
                    struct TorpedoApproachBinding final : bsp::TorpedoApproachHost {
                        TorpedoApproachBinding(GameUnitsHost::Impl& owner,
                                               GameUnitSlot& slot)
                            : owner_(owner), slot_(slot) {}

                        void tick_approach_subobject_009fada0(float) override {
                            // approach+B4h, shared by all ten approach classes.
                            owner_.log.unimplemented(
                                "BotApproach::subobject_tick", "009fada0");
                        }
                        bool unit_has_torpedo_ordnance_007b93f0() override {
                            // 009D34C5. The kind 2Bh test 0099A170 already made
                            // when it built the task; the loadout does not shrink
                            // in this host, so it holds for the whole run.
                            const bsp::OrdnanceKindSet set{slot_.ordnance_mask};
                            return bsp::ordnance_has_torpedo_2bh(set);
                        }
                        void unit_world_xz(float out_xz[2]) override {
                            out_xz[0] = slot_.motion.position[0];
                            out_xz[1] = slot_.motion.position[2];
                        }
                        bool target_world_xz(float out_xz[2]) override {
                            const GameUnitSlot* const t = target();
                            if (t == nullptr) return false;
                            out_xz[0] = t->motion.position[0];
                            out_xz[1] = t->motion.position[2];
                            return true;
                        }
                        bool approach_target_point(float out_point[3]) override {
                            // 009D3517, approach->vtable[0]. The torpedo class's
                            // slot 0 body is a contract; the ordered target's
                            // world position stands in for it.
                            const GameUnitSlot* const t = target();
                            if (t == nullptr) return false;
                            out_point[0] = t->motion.position[0];
                            out_point[1] = t->motion.position[1];
                            out_point[2] = t->motion.position[2];
                            return true;
                        }
                        float terrain_height_0041bc20(float x, float z) override {
                            // 0041BC20 bilinearly samples the AVOID-ZONE LAYER
                            // at ctl+34Ch (00417FA0 maps world to cell, 0041BAE0
                            // fetches four clamped cells), not the terrain
                            // heightfield: the layer comes from
                            // BSP_AvoidZoneRegistry_SelectLayerBySlope at
                            // 007F1DCA and 0041BC20 appears in no vtable.
                            // docs/TORPEDO_RUN_IN_PATH.md. USN01 is open water,
                            // where the layer has nothing to report, so the sea
                            // surface stands in for it here.
                            owner_.log.unimplemented(
                                "AvoidZoneLayer::sample_0041bc20", "0041bc20");
                            OceanFieldBinding sea(owner_);
                            return bsp::ocean_water_height_0078cf20(x, z, sea);
                        }
                        bool segment_blocked_00903bc0(const float from[3],
                                                      const float to[3]) override {
                            // 00903BC0 with ECX = [00E188A8]+19CCh: both
                            // endpoints must clear the ground that 00903860
                            // reports (the max over the objects at that
                            // manager's +34Ch, each answering its +3D0h
                            // sub-object's vtable +28h), and then no object in
                            // the same list may intersect the segment through
                            // vtable +3Ch. The occluder sweep is contract:
                            // unread; the two ground tests are modelled.
                            owner_.log.unimplemented(
                                "World::segment_occluders_00903bc0", "00903bc0");
                            OceanFieldBinding sea(owner_);
                            const float ga =
                                bsp::ocean_water_height_0078cf20(from[0], from[2], sea);
                            if (from[1] < ga) return true;   // 009D390E
                            const float gb =
                                bsp::ocean_water_height_0078cf20(to[0], to[2], sea);
                            return to[1] < gb;               // 009D3946
                        }
                        bool target_reachable_007df360(const float[3]) override {
                            owner_.log.unimplemented(
                                "BotApproach::target_reachable", "007df360");
                            return false;
                        }
                        float unit_speed_vtable38() override {
                            // 009D3D01, unit->vtable[38h]. The plane's speed,
                            // taken from the live linear velocity.
                            const float vx = slot_.motion.linear_velocity.x;
                            const float vy = slot_.motion.linear_velocity.y;
                            const float vz = slot_.motion.linear_velocity.z;
                            return static_cast<float>(std::sqrt(
                                static_cast<double>(vx) * vx +
                                static_cast<double>(vy) * vy +
                                static_cast<double>(vz) * vz));
                        }
                        bsp::TorpedoApproachControl read_control_block() override {
                            // unit+9D4h. This host has no pilot control block,
                            // so the fields come from the values 009D4A70 wrote
                            // into it: Pilot/Torpedo/CruisingAlt at +394h and
                            // the speed ceiling [00CE4C04] at +39Ch.
                            bsp::TorpedoApproachControl ctl;
                            ctl.second_altitude_398 =
                                bsp::kPilotTorpedoCruisingAltDefault;
                            ctl.speed_ceiling_39c =
                                bsp::kPilotTorpedoSpeedCeiling_00ce4c04;
                            ctl.profile_dirty_3ad = 1;
                            ctl.always_engage_369 = 0;
                            // ctl+370h, now live: 0099B740 raises it to 1 on
                            // the flight leader's cruise-profile tick.
                            // docs/TORPEDO_RELEASE_ORDERS.md.
                            ctl.attack_mode_370 =
                                static_cast<int>(slot_.torpedo_attack_mode_370);
                            // ctl+34Ch, the avoid-zone layer 007F1DCA selects.
                            // It is now backed, so the sector scan runs.
                            ctl.has_terrain_34c = true;
                            ctl.has_designated_target_3d0 = target() != nullptr;
                            ctl.designated_target_is_self = false;
                            return ctl;
                        }

                       private:
                        const GameUnitSlot* target() const {
                            if (slot_.command_target_plus_one == 0) return nullptr;
                            const std::size_t i = slot_.command_target_plus_one - 1;
                            if (i >= owner_.slots.size()) return nullptr;
                            return owner_.slots[i].get();
                        }
                        GameUnitsHost::Impl& owner_;
                        GameUnitSlot& slot_;
                    };

                    struct TorpedoArmBinding final : bsp::TorpedoTaskHost {
                        TorpedoArmBinding(GameUnitsHost::Impl& owner,
                                          GameUnitSlot& slot)
                            : owner_(owner), slot_(slot) {}

                        // --- BotTaskStateHost, the shared half ---
                        void register_state_name(void*, const char*, void*) override {}
                        void exit_state(void*) override {}
                        void enter_state(void*) override {}
                        void tick_state(void*, float) override {}
                        void set_desired_speed(void*, float) override {}
                        void update_approach(void*, float) override {}
                        void refresh_move_to_ranges(void*, float, float, float) override {
                            // 009BDE80 at 009D48CF. contract: unread body.
                            record("BotStateMoveTo::refresh_ranges", "009bde80");
                        }
                        bool unit_has_no_follow_target(const void*) override {
                            // 007B8AD0: unit->+9D8h == 0. This host has no
                            // follow target, so the answer is always yes.
                            return true;
                        }
                        bool should_break_off(void*) override {
                            // task->vtable[1Ch] == 009D4C10. contract: unread.
                            record("BotTaskTorpedo::should_break_off", "009d4c10");
                            return false;
                        }
                        bool manual_release_requested(void*) override {
                            // (unit+72Ch)->vtable[38h] at 009D4923.
                            record("Unit::device_requests_release", "009d4923");
                            return false;
                        }
                        void request_ordnance_release(void*) override {
                            // 007BBBA0 at 009D4956/009D26F8/009D2938/009D29CB.
                            // The device at unit+DECh and the projectile spawn
                            // behind it are this packet's contract.
                            // 007BBC00 `ADD dword [ECX+0xC20], EBX` with
                            // EBX = 1 from 007BBBAB, past every early exit, so
                            // the counter rises whether or not the device
                            // accepts the drop. docs/TORPEDO_ISSUE_TIMING.md.
                            owner_.release_ordnance_007bbba0(slot_);
                        }
                        float unit_altitude(const void*) override {
                            return slot_.motion.position[1];
                        }
                        float random_between(float low, float) override { return low; }
                        float sample_heading_offset(void*, int) override { return 0.0f; }
                        float add_wrapped_angle(float base, float delta) override {
                            // 00438AA0: add and wrap into [0, 2pi).
                            float v = base + delta;
                            const float two_pi = 6.2831855f;
                            while (v >= two_pi) v -= two_pi;
                            while (v < 0.0f) v += two_pi;
                            return v;
                        }
                        void command_altitude_and_throttle(void*, float base,
                                                           float range_low,
                                                           float range_high,
                                                           float) override {
                            // 009FBA50 then 009FB800, the chain step 4 of the
                            // attackrun tick 009D07B0 runs. 009FBA50 biases the
                            // base by span * scale * class+518h only when
                            // span = max(range_high - range_low, 0) is positive,
                            // clamps against Dynamics/Ceiling - 50, and hands
                            // 009FB800 the CLAMPED altitude as its first
                            // argument and the UNCLAMPED one as its second.
                            // 009FB800 then writes cmd+2BCh and cmd+2D0h = 2.
                            // docs/PLANE_FLIGHT.md and
                            // docs/TORPEDO_RUN_IN_DESCENT.md.
                            bsp::PlaneCruiseAltitudeInputs cin;
                            cin.base_altitude = base;
                            cin.range_low = range_low;
                            cin.range_high = range_high;
                            // The squadron altitude limit squadron+394h and the
                            // per-class gain class+518h are unmodelled; with
                            // span at zero the gain is unreachable anyway.
                            cin.has_squadron = false;
                            if (owner_.lua.plane_globals_loaded()) {
                                cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                            }
                            const bsp::PlaneCruiseAltitudeResult c =
                                bsp::cruise_altitude_command_009fba50(cin);
                            bsp::PlanePitchCommandInputs pin;
                            pin.desired_altitude = c.clamped_altitude;
                            pin.reference = c.unclamped_altitude;   // 009FBB03
                            pin.unit_world_y = slot_.motion.position[1];
                            pin.ceiling = cin.ceiling;
                            if (owner_.lua.plane_globals_loaded()) {
                                const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                                pin.climb_dist = g.pilot_general_climb_dist;
                                pin.drop_dist = g.pilot_general_drop_dist;
                            }
                            // class+1ECh DOES have a producer, and
                            // docs/PLANE_FLIGHT.md's "no producer, so the climb
                            // arm commands nothing" is refuted: it is DERIVED
                            // rather than authored, at 007C4C08-007C4C14, as
                            // 0.6 times desc+1E4h, which 007C4BE9 fills with the
                            // 007D98F0 climb-angle solver. A .text scan for a
                            // store into a plane descriptor finds it only inside
                            // 007C4850, which is also where desc+50Ch comes
                            // from. docs/PLANE_ALTITUDE_HOLD_AND_SURFACE.md.
                            pin.class_climb_angle = slot_.plane_climb_angle_1ec;
                            pin.class_drop_angle = slot_.plane_drop_angle;
                            const float demand = bsp::pitch_command_009fb800(pin);
                            slot_.plane_commanded_altitude = c.clamped_altitude;
                            slot_.plane_commanded_pitch = demand;
                            // cmd+2BCh and cmd+2D0h = 2. The mode is recorded
                            // but not acted on: 0099DCE0's mode-2 arm holds
                            // unit+C84h, which this host does not model, so the
                            // demand reaches the planner as a plain target.
                            slot_.plan_state.pitch_target_2bc = demand;
                            record("BotApproach::command_altitude", "009fba50");
                        }
                        void write_command_word(void*, int, int) override {}
                        void write_command_byte(void*, int, unsigned char) override {}
                        void write_command_float(void*, int, float) override {}
                        void set_arm_request(void*, bool) override {}
                        void set_weapon_selector(void*, int) override {}
                        void consume_round(void*) override {}

                        // --- TorpedoTaskHost, the torpedo half ---
                        void update_torpedo_approach_009d3420(void*, float dt) override {
                            // 009D3420-009D3E3F, 2592 bytes, the producer of
                            // every state-machine input.
                            // docs/TORPEDO_APPROACH_UPDATE.md.
                            record("BotApproachTorpedo::update", "009d3420");
                            TorpedoApproachBinding approach(owner_, slot_);
                            const bsp::TorpedoApproachUpdateResult r =
                                bsp::torpedo_approach_update_009d3420(
                                    approach, slot_.torpedo_approach, dt);
                            ++slot_.torpedo_approach_ticks;
                            if (r.early_out_no_target) {
                                ++slot_.torpedo_approach_no_target_ticks;
                            }
                            if (r.replanned) ++slot_.torpedo_approach_replans;
                            if (r.scan_ran) {
                                ++slot_.torpedo_scans_run;
                                slot_.torpedo_clear_sectors_last =
                                    slot_.torpedo_approach.sector_clear_count_58;
                                slot_.torpedo_turn_offset_last =
                                    slot_.torpedo_approach.turn_offset_5c;
                                slot_.torpedo_home_sector_last =
                                    slot_.torpedo_approach.home_sector_64;
                            }
                            // task+529h/+52Ah and task+484h/+488h are the same
                            // storage as approach+131h/+132h and +8Ch/+90h.
                            slot_.torpedo_aim_flag_529 = r.in_range_latch;
                            slot_.torpedo_attack_flag_52a = r.has_ordnance;
                            slot_.torpedo_engage_range_8c =
                                slot_.torpedo_approach.engage_range_8c;
                            slot_.torpedo_engage_limit_90 = r.range;
                            slot_.torpedo_range_last = r.range;
                            if (r.range > 0.0f &&
                                (slot_.torpedo_range_min < 0.0f ||
                                 r.range < slot_.torpedo_range_min)) {
                                slot_.torpedo_range_min = r.range;
                            }
                            if (slot_.torpedo_first_engaged_tick < 0 &&
                                slot_.torpedo_engage_range_8c > 0.0f &&
                                slot_.torpedo_engage_limit_90 > 0.0f) {
                                slot_.torpedo_first_engaged_tick =
                                    slot_.torpedo_approach_ticks;
                            }
                        }
                        bsp::TorpedoTransitionInputs read_transition_inputs(void*) override {
                            bsp::TorpedoTransitionInputs in;
                            in.current = slot_.torpedo_state;
                            in.engaged.aim_flag_529 = slot_.torpedo_aim_flag_529;
                            in.engaged.has_engage_target_4c4 =
                                slot_.command_target_plus_one != 0;
                            in.engaged.pilot_control_mode_370 =
                                static_cast<int>(slot_.torpedo_attack_mode_370);
                            in.engaged.unit_has_no_follow_target = true;
                            in.engaged.engage_range_484 = slot_.torpedo_engage_range_8c;
                            // 009D324F FMUL double [00D05AC8] = 2.2,
                            // read this packet. The previous binding
                            // carried 1.0 and refused a range the
                            // native admits.
                            in.engaged.engage_range_scale =
                                bsp::kTorpedoEngageRangeScale_00d05ac8;
                            in.engaged.engage_range_limit_488 =
                                slot_.torpedo_engage_limit_90;
                            in.entry.pilot_control_mode_370 =
                                static_cast<int>(slot_.torpedo_attack_mode_370);
                            in.entry.attack_flag_52a = slot_.torpedo_attack_flag_52a;
                            in.entry.control_flag_369 = false;
                            in.entry.global_e17bf2 = false;
                            in.entry.aim_flag_529 = slot_.torpedo_aim_flag_529;
                            in.unit_has_no_follow_target = true;
                            in.should_break_off = false;
                            // 009D31B0: MOV EAX,[ECX+4]; CMP byte [EAX+132h],0;
                            // JNZ 009D31BF; XOR AL,AL; RET / MOV AL,[ECX+2Ch].
                            // So the hold is the aim-complete byte the tick
                            // writes at 009D236E, but only while the approach
                            // still reports ordnance.
                            in.aim_hold_009d31b0 =
                                slot_.torpedo_approach.has_ordnance_132 &&
                                slot_.torpedo_aim_complete_2c;
                            // 009D3150, now computed. ctl+369h is 0 in this
                            // host (read_control_block), so the 0.4 scaling and
                            // the ordnance refusal are both skipped and the test
                            // is the bare range comparison at 009D3195.
                            bsp::TorpedoGoAwayCompleteInputs gc;
                            gc.range_90 = slot_.torpedo_approach.range_90;
                            gc.break_off_distance_24 =
                                slot_.torpedo_goaway_distance_24;
                            gc.has_ordnance_132 =
                                slot_.torpedo_approach.has_ordnance_132;
                            gc.control_flag_369 = false;
                            gc.global_e17bf2 = false;
                            in.goaway_done_009d3150 =
                                bsp::torpedo_goaway_complete_009d3150(gc);
                            slot_.torpedo_goaway_done_last =
                                in.goaway_done_009d3150;
                            if (slot_.torpedo_state == bsp::TorpedoState::kGoAway &&
                                slot_.torpedo_approach.range_90 >
                                    slot_.torpedo_goaway_range_peak) {
                                slot_.torpedo_goaway_range_peak =
                                    slot_.torpedo_approach.range_90;
                            }
                            return in;
                        }
                        void set_state(void*, bsp::TorpedoState next) override {
                            // 009D0D90, the goaway state's vtable slot +4h. The
                            // registrar never writes goaway+24h, so this enter
                            // is its only producer and 009D3150 reads whatever
                            // it leaves.
                            if (next == bsp::TorpedoState::kGoAway &&
                                slot_.torpedo_state != bsp::TorpedoState::kGoAway) {
                                bsp::TorpedoGoAwayEnterInputs gin;
                                gin.safe_distance_438 = safe_distance_438();
                                // 007B5BE0's target extent needs approach+CCh's
                                // entity bounds, which this host does not model.
                                gin.has_extent_target = false;
                                gin.target_extent = 0.0f;
                                // 00BD2F10 UniformFloatRange(1.0, 1.15). This
                                // host's torpedo binding takes the low end of
                                // every draw, as random_between already does.
                                gin.distance_jitter =
                                    bsp::torpedo_goaway::kDistanceJitterLo;
                                gin.side_bit = (slot_.torpedo_goaway_enters & 1) != 0;
                                const bsp::TorpedoGoAwayState g =
                                    bsp::torpedo_goaway_enter_009d0d90(gin);
                                slot_.torpedo_goaway_distance_24 =
                                    g.break_off_distance_24;
                                slot_.torpedo_goaway_side_2c = g.break_off_side_2c;
                                ++slot_.torpedo_goaway_enters;
                                slot_.torpedo_goaway_range_peak = -1.0f;
                            }
                            slot_.torpedo_state = next;
                            ++slot_.torpedo_transitions;
                        }
                        float safe_distance_438() const {
                            // 0042E740()+438h, Pilot/Torpedo/SafeDist.
                            if (owner_.lua.plane_globals_loaded()) {
                                return owner_.lua.plane_globals()
                                    .pilot_torpedo_safe_dist;
                            }
                            return 0.0f;
                        }
                        float time_to_target_009d1500(void*) override {
                            // 009D2A44-009D2A52 recomputes the same metric
                            // inline: planar distance over the reference speed.
                            record("BotApproachTorpedo::time_to_target", "009d1500");
                            return 0.0f;
                        }
                        float bearing_error_to_target(void*, void*) override {
                            return (slot_.attack_hdg_err_last < 0.0f)
                                ? 0.0f : slot_.attack_hdg_err_last;
                        }
                        float unit_bank_c68(const void*) override {
                            return std::fabs(slot_.plane_bank_angle_c68);
                        }
                        void approach_committed_hook_009d1360(void*) override {
                            record("BotApproachTorpedo::committed_hook", "009d1360");
                        }
                        void follow_base_tick_009c1fd0(void*, float) override {
                            record("BotStateFollow::tick", "009c1fd0");
                        }
                        void steer_toward_target_009f9e40(void*) override {
                            record("BotApproach::steer_to_point", "009f9e40");
                        }
                        void clear_approach_1c_field40(void*) override {}
                        float read_drop_timer(void*) override {
                            return slot_.torpedo_drop_timer;
                        }
                        void write_drop_timer(void*, float value) override {
                            slot_.torpedo_drop_timer = value;
                        }
                        bool pilot_control_has_target(void*) override {
                            return slot_.command_target_plus_one != 0;
                        }
                        int rounds_pending(void*) override {
                            return slot_.torpedo_rounds_pending;
                        }
                        void spend_pending_round(void*) override {
                            if (slot_.torpedo_rounds_pending > 0) {
                                --slot_.torpedo_rounds_pending;
                            }
                        }
                        void set_plan_step_scratch(void*, int) override {}
                        void commit_plan_step_result(void*) override {}
                        float approach_speed_for_arm(void*) override {
                            // 009D4886/009D4890 over approach+134h, +7Ch, +80h,
                            // all of them 009D3420's outputs.
                            return bsp::torpedo_arm_speed_009d4886(0.0f, 0.0f, 0.0f);
                        }
                        float approach_move_to_range_for_arm(void*) override {
                            return bsp::torpedo_arm_move_to_range_009d48ac(0.0f, 0.0f);
                        }
                        void* move_to_state(void* task) override { return task; }

                      private:
                        void record(const char* method, const char* address) {
                            owner_.log.unimplemented(method, address);
                        }
                        GameUnitsHost::Impl& owner_;
                        GameUnitSlot& slot_;
                    };

                    // 0099A170 builds a kind Eh task only for a unit whose
                    // class carries torpedo ordnance (kind 2Bh, 007ED8D0 ->
                    // 007B93F0) under a PilotSetTarget-class order.
                    void run_torpedo_task_arm_009d4850(float dt) {
                        const bsp::OrdnanceKindSet set{unit_.ordnance_mask};
                        if (unit_.command_target_plus_one == 0) return;
                        if (!bsp::ordnance_has_torpedo_2bh(set)) return;
                        if (!unit_.torpedo_task_installed) {
                            unit_.torpedo_task_installed = true;
                            // 009D3050 leaves +310h on the moveto/follow pair
                            // 009D2DA0 registered; 009D24E0 leaves +98h at -1.
                            unit_.torpedo_state = bsp::TorpedoState::kMoveTo;
                            unit_.torpedo_drop_timer = -1.0f;
                            // 009D4A70, the task's +54h cruise profile, is the
                            // one producer of the engage distance the engaged
                            // predicate 009D3210 compares the range against:
                            // task+484h = max(task+484h, tuning+434h *
                            // task+41Ch). docs/TORPEDO_APPROACH_UPDATE.md.
                            // task+41Ch, the speed ratio, was not read; 1.0
                            // leaves the clamp at the tuning row itself.
                            bsp::TorpedoApproachState& ap = unit_.torpedo_approach;
                            ap.engage_range_8c = bsp::torpedo_engage_range_009d4ac4(
                                ap.engage_range_8c,
                                bsp::kPilotTorpedoAttackDistDefault, 1.0f);
                            ap.scan_radius_seed_88 = ap.engage_range_8c;
                            // 009D4AE7 parks the cached plan position so the
                            // first 009D3420 tick replans, and 009D4AF5 flips a
                            // positive replan timer negative for the same reason.
                            ap.plan_target_x_ac = bsp::kTorpedoPlanPositionReset_00cf87d0;
                            ap.plan_target_z_b0 = bsp::kTorpedoPlanPositionReset_00cf87d0;
                            ap.forward_gap_68 = 0;
                            ap.backward_gap_6c = 0;
                            ap.turn_offset_5c = 0.0f;
                            ap.home_sector_64 = 0;
                            // 009D0579-009D05C1, the approach reset: +128h = 1,
                            // +12Ch = -Random(0, 1), +130h/+131h = 0, +134h = 0.
                            ap.replan_period_128 = 1.0f;
                            ap.replan_timer_12c = 0.0f;
                            ap.elapsed_134 = 0.0f;
                            ap.in_range_latch_131 = false;
                            // 009D0484-009D0497 seeds the two run speeds inside
                            // BSP_BotApproachTorpedo_Reset, and they are SPEEDS:
                            //   009D046C  mov  eax, [esi+14h]   the run profile
                            //   009D047D  fld  [esi+24h]        the scale
                            //   009D0491  fstp [esi+7Ch]        record[+4]*scale
                            //   009D0497  fstp [esi+80h]        record[+8]*scale
                            // 009D05ED and 009D0625 then jitter the pair.
                            //
                            // These two lines used to assign
                            // kPilotTorpedoCruisingAltDefault, which is
                            // Pilot/Torpedo/CruisingAlt = 500 and an ALTITUDE in
                            // metres, not a speed. 009D1500 divides the range by
                            // whichever slot the 15 s switch picks, and the aim
                            // tick's clause 2 compares that time against the
                            // steering delta in radians, so the oversized slot
                            // broke every run off at 1268 m: inverting
                            // 009D1500's third arm on the before-run gives
                            // (1239.16-500)/600+1 = 2.2319 and
                            // (1263.31-500)/600+1 = 2.2722, exactly the F0C the
                            // census reported. docs/TORPEDO_STEERING_DELTA.md.
                            //
                            // SUBSTITUTION, and an unevidenced one. Neither
                            // approach+14h nor +24h is written anywhere in
                            // 009D0380-009D066F, so this host models neither and
                            // has no value with any evidence behind it.
                            //
                            // MEASURED, do not repeat: slot.motion.max_speed
                            // (VehicleClass+500h) was tried here and is 0.0f on
                            // an aircraft. The class row exists - the run reports
                            // all 77 units carrying one - but planes take no
                            // speed from it, and 009D1500's `speed == 0` guard
                            // then returns F0C = 0.0000 on every tick. That made
                            // clause 2 (|delta| > F0C) fire on the first aim tick
                            // of every run and pushed goaway entries from 96 to
                            // 208 per aircraft. local/usn01_after.log.
                            //
                            // PRODUCER NOW READ, packet cc8_torpedo_run_profile.
                            // These slots are RELEASE DISTANCES in metres, not
                            // speeds. 009F9CFF-009F9D22 points approach+14h at
                            // &PilotBotConfig.levels[[[unit+DF4h]+34h]] and
                            // 009D0484-009D0497 takes record+4h
                            // TorpReleaseDistNear into +7Ch and record+8h
                            // TorpReleaseDistFar into +80h, both times
                            // approach+24h = max(1.0, desc.MaxSpd /
                            // Pilot/Torpedo/ReferenceSpeed) from 009F9D30.
                            // 009D1500 therefore returns a range ratio, not a
                            // time, which is why clause 2 compares it against
                            // radians. docs/TORPEDO_RUN_PROFILE.md.
                            //
                            // The placeholder still stands, but for a different
                            // reason than before: the value wanted is a distance
                            // in metres and 500 is a plausible one, so it is no
                            // longer known to be the wrong KIND of quantity, only
                            // the wrong SOURCE. Binding the real pair needs the
                            // PilotBot registry, which this units host cannot
                            // reach, and desc.MaxSpd, which the Lua row does not
                            // load. Both are the contract in the doc.
                            // SUBSTITUTION, labelled, replacing an older one.
                            // The registry is still out of reach, but the values
                            // the registry would carry are authored in the
                            // installed scripts/datatables/robots.lua and can be
                            // named exactly instead of stood in for by an
                            // altitude default. The `SPNormal` row:
                            //   TorpReleaseDistNear 450  -> approach+7Ch
                            //   TorpReleaseDistFar  650  -> approach+80h
                            //   TorpReleaseAlt       12  -> scales approach+78h
                            // The row's own Hungarian comment on TorpReleaseAlt
                            // reads "ilyen magasrol dobja a torpedot", the height
                            // it drops the torpedo from, so it is metres of
                            // release altitude and not a scale factor on one.
                            //
                            // WHICH row is the substitution: the difficulty index
                            // at [[unit+DF4h]+34h] is unmodelled, so SPNormal is
                            // picked and named. SPVeteran authors 5/800/1200 and
                            // the MP rows 10/800/1200, so the altitude this host
                            // commands is within a factor of about two of any of
                            // them and the distances within a factor of two.
                            // docs/TORPEDO_RELEASE_GEOMETRY.md.
                            owner_.log.unimplemented(
                                "TorpedoApproach::run_profile_record_14h", "009d0484");
                            // approach+24h. 009F9D30 is
                            // bot_task_speed_ratio(desc+188h MaxSpd,
                            // Pilot/Torpedo/ReferenceSpeed) with a floor of
                            // 1.0, and desc+188h is now loaded, so the third
                            // argument is no longer a bare placeholder. The
                            // divisor is the PilotBot tuning float at +440h,
                            // authored KMH(300) = 83.333336 m/s
                            // (include/bsp/bot_tasks.hpp:266); the registry
                            // itself is still unreachable from this host, so
                            // the DIVISOR remains a labelled substitution while
                            // the numerator is real. For this installation's
                            // torpedo bombers MaxSpd is 69.444443 (TBD) and
                            // 72.222221 (TBF), both below the reference, so the
                            // floor wins and the ratio is 1.0 either way.
                            const float ratio_24h =
                                unit_.plane_max_spd > 0.0f
                                    ? bsp::bot_task_speed_ratio(
                                          unit_.plane_max_spd,
                                          kTorpedoReferenceSpeedAuthored)
                                    : 1.0f;
                            const bsp::TorpedoRunSpeeds seeded =
                                bsp::torpedo_seed_run_speeds_009d0484(
                                    kTorpReleaseDistNearSPNormal,
                                    kTorpReleaseDistFarSPNormal, ratio_24h);
                            ap.speed_early_80 = seeded.speed_early_80;
                            ap.speed_late_7c = seeded.speed_late_7c;
                            // 009D046A scales approach+78h by the row's
                            // TorpReleaseAlt. approach+74h stays where
                            // 009D3489 puts it, from the control block's
                            // second altitude under the ceiling of 100 at
                            // 00D7A220, so the floor the aim tick reads is
                            // alt_floor_74 + alt_margin_78 and this is the
                            // margin half of it.
                            ap.alt_margin_78 = kTorpReleaseAltSPNormal;
                            // ctl+3D0h[0], the flight leader: the only task
                            // whose 0099B740 raises the shared attack mode.
                            bool lead_taken = false;
                            for (const auto& s : owner_.slots) {
                                if (s->torpedo_is_flight_lead) lead_taken = true;
                            }
                            unit_.torpedo_is_flight_lead = !lead_taken;
                            owner_.log.notef("torpedo task 009D4E30 kind Eh installed "
                                "for an ordered aircraft; arm 009D4850 runs on the "
                                "0.09 s pilot think; 009D4A70 sets the engage "
                                "distance task+484h=%.1f (Pilot/Torpedo/AttackDist); "
                                "flight_lead=%d",
                                static_cast<double>(ap.engage_range_8c),
                                unit_.torpedo_is_flight_lead ? 1 : 0);
                        }
                        // 009D4A70's tail 0099B740, the cruise profile that
                        // also sets the engage distance.
                        run_attack_mode_tick_0099b740();
                        {
                            const int m = static_cast<int>(
                                unit_.torpedo_attack_mode_370);
                            if (m >= 0 && m <= 2) ++unit_.torpedo_mode_ticks[m];
                            // 009D3F69 and 009D40CB both send an engaged task
                            // to prepare only while the mode is 0, so a tick at
                            // 0 with the task engaged is a prepare window.
                            if (m == 0) ++unit_.torpedo_mode_hold_with_engaged;
                        }
                        // The two native routes back to 0, both bound and both
                        // inert on this mission. 009A285E needs a closetoship
                        // task, which no ordered aircraft here has; 007F0068
                        // needs message BCh, which nothing sends.
                        run_attack_mode_lowering_paths(dt);
                        TorpedoArmBinding binding(owner_, unit_);
                        bsp::TorpedoTaskContext ctx;
                        ctx.task = &unit_;
                        ctx.approach = &unit_;
                        ctx.unit = &unit_;
                        ctx.command_block = &unit_;
                        ctx.pilot_control_block = &unit_;
                        ctx.state = &unit_;
                        ctx.prepare_state = &unit_;
                        ctx.current = unit_.torpedo_state;
                        const bsp::TorpedoState before_state = ctx.current;
                        const bsp::TorpedoArmTickResult r =
                            bsp::torpedo_task_arm_009d4850(binding, ctx, dt);
                        unit_.torpedo_state = ctx.current;
                        ++unit_.torpedo_arm_ticks;
                        const int bucket = torpedo_state_bucket(ctx.current);
                        if (bucket >= 0) ++unit_.torpedo_state_ticks[bucket];
                        if (ctx.current == bsp::TorpedoState::kPrepare &&
                            before_state != bsp::TorpedoState::kPrepare) {
                            ++unit_.torpedo_prepare_entries;
                            if (unit_.torpedo_prepare_first_tick < 0) {
                                unit_.torpedo_prepare_first_tick =
                                    unit_.torpedo_arm_ticks;
                            }
                        }
                        // 009D48F6, the current state's own tick. The aim state
                        // runs 009D15F0, whose heading is the bearing to the
                        // target plus the turn offset the approach update's
                        // sector scan chose. docs/TORPEDO_APPROACH_UPDATE.md.
                        if (ctx.current == bsp::TorpedoState::kAim) {
                            run_torpedo_aim_tick_009d15f0(dt);
                        } else if (ctx.current == bsp::TorpedoState::kAttackRun) {
                            // Step 4 of the attackrun tick 009D07B0, the half
                            // that commands the altitude: "altitude
                            // approach->+78h + approach->+74h through 009FBA50"
                            // (docs/BOT_TASK_STATES.md, "The torpedo run").
                            // This is the ONLY descent command in the whole
                            // torpedo chain - the aim tick's own plan+2BCh is a
                            // nose-up floor (docs/PLANE_POSE_THROTTLE_ALTITUDE.md
                            // section 4) - and this host ran no state tick but
                            // aim, so no aircraft was ever told to come down.
                            //
                            // PARTIAL, labelled: only step 4's altitude. Steps
                            // 1, 2 and 3 keep a per-state countdown, period and
                            // lateral offset at state+18h/+1Ch/+20h that this
                            // host does not carry, and step 4's throttle half
                            // and step 5's four command bits are not run. The
                            // heading those steps would write is the same raw
                            // bearing the yaw arm already uses here.
                            const bsp::TorpedoApproachState& ap =
                                unit_.torpedo_approach;
                            binding.command_altitude_and_throttle(
                                nullptr, ap.alt_margin_78 + ap.alt_floor_74,
                                0.0f, 0.0f, 0.0f);
                            ++unit_.torpedo_attackrun_altitude_commands;
                            if ((unit_.torpedo_attackrun_altitude_commands % 50) == 1) {
                                owner_.log.notef("  torpedo %-12s descent census "
                                    "n=%d base=%.2f (74h=%.2f 78h=%.2f) "
                                    "commanded=%.2f live_alt=%.1f pitch_demand=%.4f "
                                    "pitch=%.4f drop_angle=%.4f climb_1ec=%.4f",
                                    unit_.row.name.c_str(),
                                    unit_.torpedo_attackrun_altitude_commands,
                                    static_cast<double>(ap.alt_margin_78 + ap.alt_floor_74),
                                    static_cast<double>(ap.alt_floor_74),
                                    static_cast<double>(ap.alt_margin_78),
                                    static_cast<double>(unit_.plane_commanded_altitude),
                                    static_cast<double>(unit_.motion.position[1]),
                                    static_cast<double>(unit_.plane_commanded_pitch),
                                    static_cast<double>(unit_.plane_pitch_angle_c64),
                                    static_cast<double>(unit_.plane_drop_angle),
                                    static_cast<double>(unit_.plane_climb_angle_1ec));
                            }
                        }
                        // The flag is deliberately NOT cleared when the task
                        // leaves aim. plan+2C0h is a persistent plan field, and
                        // the state that follows aim writes it too: the goaway
                        // tick 009D0F10 stores it at 009D110C and 009D11A5 with
                        // mode 2 at 009D1112/009D11AB. That tick is not
                        // reconstructed, so the plan keeps the aim tick's last
                        // heading through goaway. Clearing the flag would fall
                        // back to the raw target bearing, which is the one
                        // heading the native never flies here.
                        // 0099AF53-0099AFAF, the ordnance-arming loop of
                        // BSP_PilotBot_Tick: it spends one queued release order
                        // per tick by offering it to each task's vtable +24h,
                        // and the torpedo task takes it by arming prepare+98h.
                        // docs/TORPEDO_TASK_ARM.md section (4).
                        if (unit_.torpedo_release_orders_c58 <= 0) {
                            ++unit_.torpedo_arm_blocked_no_order_0099af53;
                            if (unit_.torpedo_blocked_no_order_first_tick < 0) {
                                unit_.torpedo_blocked_no_order_first_tick =
                                    unit_.torpedo_arm_ticks;
                            }
                            owner_.log.unimplemented(
                                "PilotBot::queue_release_order", "0099af53");
                        } else {
                            ++unit_.torpedo_arm_offers;
                            bsp::TorpedoArmInputs arm;
                            arm.attack_flag_52a = unit_.torpedo_attack_flag_52a;
                            arm.current = unit_.torpedo_state;
                            // 009D49C2 MOVSS [00CE3D34], the countdown value.
                            arm.arm_countdown_00ce3d34 = 4.0f;
                            arm.engaged.aim_flag_529 = unit_.torpedo_aim_flag_529;
                            arm.engaged.has_engage_target_4c4 =
                                unit_.command_target_plus_one != 0;
                            arm.engaged.pilot_control_mode_370 =
                                static_cast<int>(unit_.torpedo_attack_mode_370);
                            arm.engaged.unit_has_no_follow_target = true;
                            arm.engaged.engage_range_484 =
                                unit_.torpedo_engage_range_8c;
                            arm.engaged.engage_range_scale =
                                bsp::kTorpedoEngageRangeScale_00d05ac8;
                            arm.engaged.engage_range_limit_488 =
                                unit_.torpedo_engage_limit_90;
                            const bsp::TorpedoArmResult ar =
                                bsp::torpedo_arm_drop_009d49a0(arm);
                            if (ar.armed) {
                                unit_.torpedo_drop_timer = ar.prepare_timer;
                            }
                            if (ar.consumed) --unit_.torpedo_release_orders_c58;
                        }
                        // The two gates that stop the drop in this host, both
                        // fed by the unread 009D3420.
                        bsp::TorpedoEngagedInputs eng =
                            binding.read_transition_inputs(&unit_).engaged;
                        if (!bsp::torpedo_engaged_009d3210(eng)) {
                            ++unit_.torpedo_blocked_by_engaged;
                        } else if (!unit_.torpedo_attack_flag_52a) {
                            ++unit_.torpedo_blocked_by_arm;
                        }
                        (void)r;
                    }

                    // 007C0D90, the plane fixed step's release-order issue.
                    // It assigns rather than accumulates, so a slot that is
                    // already at 999 simply stays there.
                    // 007CE9FD-007CEB31, the stage that owns 007CEA8D, the one
                    // call site of 007C0D90 in the image. It is not gated on a
                    // task: it runs on every fixed step of any plane whose
                    // control mode unit+900h is 4, 5, 6 or 7, and decides
                    // through the countdown unit+C28h and the pending
                    // release-request count unit+C20h whether the issue path
                    // runs this step. docs/TORPEDO_ISSUE_TIMING.md.
                    void run_release_issue_stage_007ce9fd(float dt) {
                        bsp::PlaneReleaseIssueStageInputs in;
                        // 007CEA02. The host has no app-state singleton, so the
                        // stage is never suppressed by a pause.
                        owner_.log.unimplemented("App::state_1fe4", "007cea02");
                        in.app_state_1fe4 = 0;
                        // 007CEA0F: the same unit+9E0h the airborne
                        // accumulator is gated on, which the plane binding
                        // already reads.
                        in.blocked_9e0 = unit_.plane_airborne_frozen_9e0;
                        // 007CEA1C and 007CEA29. contract: unread.
                        owner_.log.unimplemented("Plane::issue_block_c3a", "007cea1c");
                        owner_.log.unimplemented("Unit::issue_block_5d", "007cea29");
                        in.blocked_c3a = false;
                        in.blocked_5d = false;
                        in.control_mode_900 = unit_.plane_control_mode_900;
                        in.interval_timer_c28 = unit_.torpedo_issue_interval_c28;
                        in.step_seconds = dt;
                        in.issue_requests_c20 = unit_.torpedo_issue_requests_c20;
                        in.release_pending_c25 = unit_.torpedo_release_pending_c25;
                        // 007CEB00: the element's device walk. contract: unread,
                        // so no device ever holds the cleanup off.
                        owner_.log.unimplemented("Plane::device_busy_1fc", "007ceb00");
                        in.any_device_busy_1fc = false;
                        // 007CEAB3 BSP_Random_UniformFloatRange(1, 0.9, 1.1).
                        // The host's deterministic draw takes the low end, the
                        // same convention random_between already uses here.
                        in.interval_draw = bsp::kIssueIntervalLow_00ce3860;
                        // 007CEAB8 FMUL [class+1F4h]. SUBSTITUTION: the class
                        // descriptor field is unread, so the scale is 1.
                        owner_.log.unimplemented("PlaneClass::issue_interval_1f4",
                                                 "007ceab8");
                        in.class_interval_scale_1f4 = 1.0f;

                        const bsp::PlaneReleaseIssueStageResult r =
                            bsp::plane_release_issue_stage_007ce9fd(in);
                        unit_.torpedo_issue_interval_c28 = r.next_interval_timer_c28;
                        unit_.torpedo_issue_requests_c20 = r.next_issue_requests_c20;
                        if (r.clear_release_pending_c25) {
                            unit_.torpedo_release_pending_c25 = false;
                        }
                        ++unit_.torpedo_issue_stage_ticks;
                        if (!r.guards_passed) {
                            ++unit_.torpedo_issue_stage_guard_blocked;
                            return;
                        }
                        switch (r.arm) {
                            case bsp::PlaneReleaseIssueStageArm::kIssue:
                                ++unit_.torpedo_issue_stage_issues;
                                if (unit_.torpedo_issue_first_issue_tick < 0) {
                                    unit_.torpedo_issue_first_issue_tick =
                                        unit_.torpedo_arm_offers;
                                }
                                break;
                            case bsp::PlaneReleaseIssueStageArm::kCleanup:
                            case bsp::PlaneReleaseIssueStageArm::kCleanupBlocked:
                                ++unit_.torpedo_issue_stage_cleanups;
                                break;
                            default:
                                ++unit_.torpedo_issue_stage_waiting;
                                break;
                        }
                        if (!r.call_issue_007c0d90) return;
                        run_release_order_issue_007c0d90();
                    }

                    void run_release_order_issue_007c0d90() {
                        TorpedoReleaseOrderBinding binding(owner_, unit_);
                        const bsp::ReleaseOrderIssueResult r =
                            bsp::torpedo_issue_release_orders_007c0d90(binding);
                        unit_.torpedo_issue_gate_open = r.gate_passed;
                        if (!r.walk_found_device) return;
                        ++unit_.torpedo_orders_issue_ticks;
                        unit_.torpedo_orders_issued += r.units_raised;
                    }

                    // 0099B740, the tail of the torpedo task's +54h cruise
                    // profile 009D4A70. It is the one producer of attack mode 1
                    // in the image, and only the flight leader's task runs it to
                    // completion. docs/TORPEDO_RELEASE_ORDERS.md.
                    // 009A2810 and 007F0068, the only two writers of 0.
                    // Both are driven from outside the torpedo task, so both
                    // are bound here and report whether they ever fired.
                    void run_attack_mode_lowering_paths(float dt) {
                        // 009A2B00, the closetoship task's vtable +64h arm, is
                        // the only caller of 009A2810. An ordered torpedo
                        // bomber has a kind Eh task and no closetoship task, so
                        // the countdown never runs. contract: unread producer.
                        if (unit_.torpedo_closetoship_task_installed) {
                            bsp::CloseToShipModeCountdown cin;
                            cin.timer_550 = unit_.torpedo_closetoship_timer_550;
                            cin.latch_43c = unit_.torpedo_closetoship_latch_43c;
                            cin.mode_370 =
                                static_cast<int>(unit_.torpedo_attack_mode_370);
                            cin.dt = dt;
                            const bsp::CloseToShipModeResult r =
                                bsp::closetoship_attack_mode_countdown_009a2810(cin);
                            unit_.torpedo_closetoship_timer_550 = r.timer_550;
                            unit_.torpedo_closetoship_latch_43c = r.latch_43c;
                            if (r.lower_to_hold) {
                                unit_.torpedo_attack_mode_370 =
                                    bsp::PilotAttackMode::kHold;
                                ++unit_.torpedo_mode_lowered_009a285e;
                                ++unit_.torpedo_mode_changes;
                            }
                        }
                        // 007F0030's message arm for id BCh. Nothing in this
                        // host raises that message; when a producer is found,
                        // deliver it here. contract: unread producer.
                        if (unit_.torpedo_pending_mode_message_bc) {
                            unit_.torpedo_pending_mode_message_bc = false;
                            const int m =
                                bsp::pilot_control_attack_mode_from_message_007f0068(
                                    unit_.torpedo_mode_message_payload);
                            unit_.torpedo_attack_mode_370 =
                                static_cast<bsp::PilotAttackMode>(m);
                            ++unit_.torpedo_mode_message_007f0068;
                            ++unit_.torpedo_mode_changes;
                        }
                    }

                    void run_attack_mode_tick_0099b740() {
                        bsp::PilotAttackModeInputs in;
                        in.has_control_block_2fc = true;
                        in.has_unit_2f4 = true;
                        in.unit_is_flight_lead = unit_.torpedo_is_flight_lead;
                        // The torpedo task inherits the base vtable +38h,
                        // 0099B710, which is MOV AL,1 / RET.
                        in.task_authorises_38h = true;
                        const bsp::PilotAttackMode next =
                            bsp::pilot_attack_mode_0099b740(
                                unit_.torpedo_attack_mode_370, in);
                        if (next != unit_.torpedo_attack_mode_370) {
                            unit_.torpedo_attack_mode_370 = next;
                            if (unit_.torpedo_attack_mode_raised_tick < 0) {
                                unit_.torpedo_attack_mode_raised_tick =
                                    unit_.torpedo_arm_ticks;
                            }
                        }
                        // The mode lives on the pilot control block, which the
                        // whole flight shares, so the leader's value is what
                        // every aircraft of the flight reads.
                        if (unit_.torpedo_is_flight_lead) {
                            for (const auto& s : owner_.slots) {
                                if (!s->torpedo_task_installed) continue;
                                s->torpedo_attack_mode_370 =
                                    unit_.torpedo_attack_mode_370;
                            }
                        }
                    }

                    // 009D15F0, the aim state's tick (state vtable 00D212FC
                    // slot +Ch), reconstructed whole in src/torpedo_aim_tick.cpp.
                    // The byte it writes at 009D236E is the gate 009D31B0 reads
                    // and the transition rule 009D4030 turns into goaway.
                    struct AimTickBinding final : bsp::TorpedoAimTickHost {
                        AimTickBinding(GameUnitsHost::Impl& o, GameUnitSlot& s)
                            : owner_(o), s_(s) {}
                        float time_to_target_009d1500() override {
                            return bsp::torpedo_time_to_target_009d1500(
                                s_.torpedo_approach);
                        }
                        float unit_heading_vtable50() override {
                            // 009D1679 CALL EDX with EDX = [[approach+4]+50h].
                            // Slot 50h on an aircraft is 0074E260
                            // BSP_PlaneUnitInstance_GetHeading, FLD [ECX+0C6Ch]
                            // / RET, not the ship override 006DFD60 FLD
                            // [ECX+1050h]. 00D05F20 is the vtable
                            // BSP_PlaneUnitInstance_Construct installs at
                            // 007CFD78, and 0074E260 is its slot 50h.
                            // docs/TORPEDO_STEERING_DELTA.md.
                            return heading_;
                        }
                        // approach+CCh's target entity is not modelled, so the
                        // aspect slot F=2Ch keeps the 009D16AA zero, which is
                        // exactly the no-target arm of the native branch.
                        float target_heading_vtable50() override { return 0.0f; }
                        bool target_is_kind_vtable5c(int) override { return false; }
                        bool unit_is_kind_vtable5c(int) override { return false; }
                        bool has_target_cc() override { return false; }
                        void refresh_world_pose_00414db0(bool) override {}
                        float ground_height_00903860() override { return 0.0f; }
                        void update_run_time_009d1360() override {
                            ++s_.torpedo_aim_run_time_updates;
                        }
                        bsp::TorpedoAimSectorProbe sector_probe_009d1a94(
                            float, float, float) override {
                            return bsp::TorpedoAimSectorProbe{};
                        }
                        void accumulate_timer_009fa3a0(float d) override {
                            // 009FA3A0 SUBTRACTS the step and, on a strictly
                            // negative countdown, calls 007BBBA0 at 009FA3D0
                            // behind 007BB110 alone. It does not accumulate:
                            // the earlier `+= d` had the sign backwards and
                            // dropped the release entirely, which is why this
                            // host never reached a drop.
                            // docs/TORPEDO_FIRST_RELEASE.md.
                            bsp::ReleaseTimerInputs in;
                            in.enabled_0c = s_.torpedo_aim_timer_enabled_0c;
                            in.countdown_10 = s_.torpedo_aim_timer;
                            in.step_seconds = d;
                            // 007BB110 reads the device at unit+DECh. This host
                            // has no device model, so an aircraft that still
                            // carries a round stands in for it. SUBSTITUTION.
                            owner_.log.unimplemented("Unit::can_release_007bb110",
                                                     "007bb110");
                            const bsp::OrdnanceKindSet set{s_.ordnance_mask};
                            in.unit_can_release_007bb110 =
                                bsp::ordnance_has_torpedo_2bh(set) &&
                                s_.torpedo_releases < 1;
                            // 009FA3EA draws between timer+4h and timer+8h,
                            // both unread on this object. contract.
                            owner_.log.unimplemented("ReleaseTimer::reseed_bounds",
                                                     "009fa3ea");
                            in.reseed_draw = 0.0f;
                            owner_.log.unimplemented("Unit::slot_byte_9c0", "009fa3fb");
                            in.slot_byte_9c0_clear = false;
                            const bsp::ReleaseTimerResult r =
                                bsp::release_timer_tick_009fa3a0(in);
                            s_.torpedo_aim_timer = r.next_countdown_10;
                            if (r.blocked_by_predicate) {
                                ++s_.torpedo_aim_timer_blocked_007bb110;
                            }
                            if (!r.request_release_007bbba0) return;
                            ++s_.torpedo_aim_timer_fires;
                            if (s_.torpedo_aim_timer_first_fire_tick < 0) {
                                s_.torpedo_aim_timer_first_fire_tick =
                                    s_.torpedo_aim_ticks;
                            }
                            owner_.release_ordnance_007bbba0(s_);
                        }
                        void arm_release_timer_009d2287() override {
                            // 009D2279-009D2287: refuses to re-arm, and leaves
                            // the countdown at zero so the drop lands on the
                            // next tick.
                            const bsp::ReleaseTimerArmResult a =
                                bsp::release_timer_arm_009d2287(
                                    s_.torpedo_aim_timer_enabled_0c);
                            if (!a.armed) return;
                            ++s_.torpedo_aim_release_arms;
                            s_.torpedo_aim_timer = a.countdown_10;
                            s_.torpedo_aim_timer_enabled_0c = a.enabled_0c;
                        }
                        GameUnitsHost::Impl& owner_;
                        GameUnitSlot& s_;
                        float heading_{0.0f};
                    };

                    void run_torpedo_aim_tick_009d15f0(float dt) {
                        const bsp::TorpedoApproachState& ap = unit_.torpedo_approach;
                        AimTickBinding binding(owner_, unit_);
                        // unit+C6Ch, which 007C1900 writes and the pilot planner
                        // subtracts at 0099DEB8. It used to be
                        // pose_heading_radians, a bare atan2 over pose row 2,
                        // which is the producer of the SHIP field unit+1050h.
                        // Both reduce to atan2(fx, fz) on a well-formed pose, so
                        // this does not move the delta; what it restores is
                        // 007C1900's latch, which leaves the previous heading in
                        // place when the forward axis is near vertical.
                        binding.heading_ = unit_.plane_heading_c6c;
                        bsp::TorpedoAimTickState in;
                        in.range_90 = ap.range_90;
                        in.bearing_94 = ap.bearing_94;
                        in.turn_offset_5c = ap.turn_offset_5c;
                        in.alt_floor_74 = ap.alt_floor_74;
                        in.alt_margin_78 = ap.alt_margin_78;
                        in.over_land_a9 = ap.over_land_a9 != 0;
                        in.sector_probe_enabled_aa = ap.no_clear_sector_aa != 0;
                        in.has_ordnance_132 = ap.has_ordnance_132;
                        in.elapsed_134 = ap.elapsed_134;
                        in.speed_late_7c = ap.speed_late_7c;
                        in.speed_early_80 = ap.speed_early_80;
                        // approach+84h, the aspect scale 009D1FD0 loads, is not
                        // in the approach struct yet; it only shapes the
                        // aim-solution byte 009D2021, never the gate.
                        in.aspect_scale_84 = 1.0f;
                        in.fall_lead_a0 = ap.fall_lead_a0;
                        in.unit_altitude = unit_.motion.position[1];
                        // unit+C64h is the PITCH angle, written at 007C1966
                        // FSTP [ESI+0C64h], and unit+C68h is the BANK angle,
                        // written at 007C1A94 (docs/PLANE_ATTITUDE_ANGLES.md
                        // sections 1 and 3). The contract's field names say bank
                        // for both and this binding used to feed
                        // plane_latched_controls[0] - the latched YAW control
                        // axis, a number in [-1, 1] - where the native reads an
                        // attitude angle in radians, and a literal zero where it
                        // reads the bank. Three consumers depend on it: the bank
                        // cap's pitch fold at 009D1BDB, the pull-up denominator
                        // at 009D1DE6 and the release gate at 009D2165.
                        // docs/PLANE_POSE_THROTTLE_ALTITUDE.md.
                        in.unit_bank_c64 = unit_.plane_pitch_angle_c64;
                        in.unit_bank_rate_c68 = unit_.plane_bank_angle_c68;
                        in.pose_dirty_c8 = false;
                        // approach+8h, the aircraft description object, is not
                        // modelled. These five feed the turn-radius escape, the
                        // throttle and the pitch, never the aim-complete test,
                        // so the contract carries zeros and says so.
                        in.turn_radius_268 = 0.0f;
                        in.turn_radius_26c = 0.0f;
                        in.cruise_speed_25c = 1.0f;
                        in.alt_fold_a4 = 1.0f;
                        in.pitch_scale_188 = 1.0f;
                        in.pitch_div_1ac = 1.0f;
                        in.state_flag_24 = false;
                        const bsp::TorpedoAimTickResult r =
                            bsp::torpedo_aim_tick_full_009d15f0(binding, in, dt);
                        ++unit_.torpedo_aim_ticks;
                        // Packet cc8_torpedo_steering_delta's per-tick census.
                        // Every heading in one row so the convention question is
                        // answerable from a run rather than from arithmetic:
                        // the aim tick's own heading (slot 50h, unit+C6Ch), the
                        // ship-convention hull heading it used to be given, the
                        // bearing the delta is measured from, the delta itself
                        // and the cone it is tested against at 009D2209.
                        if ((unit_.torpedo_aim_ticks % 50) == 1) {
                            owner_.log.notef("  torpedo %-12s aim census tick=%d "
                                "cmd_2C0=%.4f yaw_C6C=%.4f hull_1050=%.4f "
                                "bearing_94=%.4f delta_F10=%.4f |delta|_F18=%.4f "
                                "F0C=%.4f F14=%.1f cone_open=%d "
                                "yaw_desired=%.4f yaw_current=%.4f range_90=%.1f "
                                "speed_80=%.1f class_max_speed=%.1f stall=%.1f",
                                unit_.row.name.c_str(), unit_.torpedo_aim_ticks,
                                static_cast<double>(r.commanded_heading_2c0),
                                static_cast<double>(unit_.plane_heading_c6c),
                                static_cast<double>(
                                    owner_.pose_heading_radians(unit_)),
                                static_cast<double>(ap.bearing_94),
                                static_cast<double>(
                                    bsp::wrapped_angle_subtract_00438b10(
                                        r.commanded_heading_2c0,
                                        unit_.plane_heading_c6c)),
                                static_cast<double>(r.turn_magnitude_f18),
                                static_cast<double>(r.time_to_target_f0c),
                                static_cast<double>(r.range_f14),
                                r.gate_cone ? 1 : 0,
                                static_cast<double>(
                                    unit_.plan_slots[bsp::kPilotSlotYaw].desired),
                                static_cast<double>(
                                    unit_.plan_slots[bsp::kPilotSlotYaw].current),
                                static_cast<double>(ap.range_90),
                                // The two candidates for the unmodelled run
                                // profile record at approach+14h, printed so the
                                // substitution question is settled from a run
                                // rather than argued. 009D0484-009D0497.
                                static_cast<double>(ap.speed_early_80),
                                static_cast<double>(unit_.motion.max_speed),
                                static_cast<double>(unit_.plane_stall_spd));
                            // Packet cc8_torpedo_run_in_velocity: the velocity
                            // frame, sampled on the same cadence. |v| is the
                            // world-velocity magnitude, angle is its angle to
                            // pose row 2 in degrees, and body_fwd is what
                            // 0092D730 answers. With XDrag/YDrag wired the
                            // angle collapses to near zero and body_fwd tracks
                            // |v|; with them zero the angle sat near 90 deg.
                            float vn[3];
                            GameUnitsHost::Impl::velocity_versus_nose(unit_, vn);
                            owner_.log.notef("  torpedo %-12s velocity census "
                                "tick=%d |v|=%.2f angle=%.1f deg body_fwd=%.2f "
                                "alt=%.1f travel_spd=%.2f x_drag=%.2f "
                                "max_spd=%.2f pitch=%.4f bank=%.4f "
                                "bank_cap_2c8=%.4f pitch_target_2bc=%.4f "
                                "throttle=%.2f accel=%.2f",
                                unit_.row.name.c_str(), unit_.torpedo_aim_ticks,
                                static_cast<double>(vn[0]),
                                static_cast<double>(vn[1]) * 180.0 / kPi,
                                static_cast<double>(vn[2]),
                                static_cast<double>(unit_.motion.position[1]),
                                static_cast<double>(unit_.plane_travel_speed),
                                static_cast<double>(unit_.plane_x_drag),
                                static_cast<double>(unit_.plane_max_spd),
                                static_cast<double>(unit_.plane_pitch_angle_c64),
                                static_cast<double>(unit_.plane_bank_angle_c68),
                                static_cast<double>(unit_.plan_state.bank_limit_2c8),
                                static_cast<double>(unit_.plan_state.pitch_target_2bc),
                                static_cast<double>(unit_.plane_live_throttle),
                                static_cast<double>(unit_.plane_accel));
                        }
                        unit_.torpedo_aim_heading_last = r.commanded_heading_2c0;
                        // 009D1D16 / 009D1D1E write plan+2C0h and plan+2CCh.
                        // docs/PILOT_TASK_HEADING_ARM.md: that pair is the plan
                        // the yaw arm 0099D300 reads, so publishing it here is
                        // what lets the aircraft fly the run-in the tick plans.
                        unit_.plan_heading_2c0 = r.commanded_heading_2c0;
                        unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                        unit_.plan_heading_2c0_written = true;
                        unit_.torpedo_aim_throttle_last = r.commanded_throttle_2c8;
                        // 009D1D2E writes plan+2C8h, and plan+2C8h is NOT a
                        // throttle: docs/PILOT_PLANNER_PITCH_ROLL.md section (2)
                        // shows 0099E27B clamping the bank target plan+2C4h into
                        // +-plan+2C8h when it is below pi, and its note 6 names
                        // 0099B55E's 20.0f reset - deliberately above pi so the
                        // clamp is inert - with task arms opting in. This is one
                        // of the seven task-side writers that doc left open. The
                        // arithmetic agrees: the product's own base is desc+25Ch
                        // TurnRoll, an authored maximum BANK ANGLE in radians
                        // (1.047198 on this installation's TBD Devastator), and
                        // the ceiling at 00CE3814 is 1.2 rad, not a throttle's
                        // 1.2 of full. The field keeps its old name here because
                        // renaming it belongs to the aim tick's owner.
                        unit_.plan_state.bank_limit_2c8 = r.commanded_throttle_2c8;
                        // 009D1EDD writes plan+2BCh, the PITCH target
                        // (docs/PILOT_PLANNER_PITCH_ROLL.md note 6 and section
                        // 2a), and 009D1EE5 writes the mode plan+2D0h = 1 that
                        // 0099E3D1 gates the whole pitch law on. The value is
                        // clamp(-f34 / den, 0.05625, 0.872665) and f34 is the
                        // aircraft's height ABOVE the altitude floor, so the
                        // quotient is negative whenever the aircraft is high and
                        // the clamp floors it at 0.05625 rad. It is a nose-up
                        // floor and a pull-up, not a descent command: nothing in
                        // the aim tick brings a torpedo bomber down.
                        unit_.plan_state.pitch_target_2bc = r.commanded_altitude_2bc;
                        unit_.torpedo_approach.aim_solution_130 = r.aim_solution_130;
                        if (r.aim_complete_2c && !unit_.torpedo_aim_complete_2c) {
                            unit_.torpedo_aim_complete_tick = unit_.torpedo_aim_ticks;
                            unit_.torpedo_aim_complete_clause =
                                (r.clause_range ? 1 : 0) | (r.clause_turn ? 2 : 0);
                            unit_.torpedo_aim_f34_threshold = r.release_threshold_f34;
                            unit_.torpedo_aim_f14_range = r.range_f14;
                            unit_.torpedo_aim_f18_turn = r.turn_magnitude_f18;
                            unit_.torpedo_aim_f0c_time = r.time_to_target_f0c;
                            unit_.torpedo_aim_ramp = r.ramp;
                            unit_.torpedo_aim_floor = r.altitude_floor_f28;
                        }
                        // 009D236E only ever sets the byte; nothing in the tick
                        // clears it, so the host latches it the same way.
                        if (r.aim_complete_2c) unit_.torpedo_aim_complete_2c = true;
                    }

                    static int torpedo_state_bucket(bsp::TorpedoState s) {
                        switch (s) {
                            case bsp::TorpedoState::kMoveTo: return 0;
                            case bsp::TorpedoState::kFollow: return 1;
                            case bsp::TorpedoState::kDone: return 2;
                            case bsp::TorpedoState::kAttackRun: return 3;
                            case bsp::TorpedoState::kGoAway: return 4;
                            case bsp::TorpedoState::kAim: return 5;
                            case bsp::TorpedoState::kPrepare: return 6;
                            default: return -1;
                        }
                    }

                    void pilot_think_and_commit(float step) {
                        // Gate 6, 0099AD0F..0099AD29. The accumulator absorbs
                        // the frame delta and the tick fires when it reaches
                        // 0.09 s; the value passed downstream is the accumulated
                        // interval, not `step`.
                        unit_.pilot_think_accumulator_70 += step;
                        if (unit_.pilot_think_accumulator_70 >= bsp::kPilotThinkInterval) {
                            const float elapsed = unit_.pilot_think_accumulator_70;
                            unit_.pilot_think_accumulator_70 = 0.0f;   // 0099AD75

                            // 0099B450, seeded from the live block in the plan's
                            // own axis order.
                            float live[5];
                            live[bsp::kPilotSlotYaw] = unit_.plane_live_controls[0];
                            live[bsp::kPilotSlotPitch] = unit_.plane_live_controls[1];
                            live[bsp::kPilotSlotRoll] = unit_.plane_live_controls[2];
                            live[bsp::kPilotSlotThrottle] = unit_.plane_live_throttle;
                            live[bsp::kPilotSlotAirBrake] = unit_.plane_live_air_brake;
                            // The full plan reset, not just the slots: plan+2BCh
                            // is zeroed every think, which is what keeps the
                            // pitch floor from ratcheting.
                            bsp::pilot_reset_plan_0099b450(unit_.plan_state,
                                unit_.plan_slots, live);

                            // 0099D300's yaw arm. It writes `desired` only when
                            // the unit has a commanded target; every other slot
                            // keeps desired == current from the seed, so the
                            // other four axes stay bit-exact no-ops.
                            // 009998A0's slow path runs the task arm before
                            // the planner: 0099B740, task->vtable[64h](dt),
                            // 009FC7C0, 009FD0E0, 009A17D0, then 0099D300.
                            // Only the arm is modelled here; the three
                            // sub-object updates are contracts.
                            run_torpedo_task_arm_009d4850(elapsed);

                            if (plan_yaw_0099d300()) {
                                ++owner_.summary.pilot_yaw_plans;
                            }

                            // 0099BEE0 -> 0099BC00.
                            bsp::pilot_evaluate_plan_slots_0099bc00(
                                unit_.plan_slots, unit_.pilot_command_block,
                                bsp::kPilotSlewRate, elapsed);
                            // 0099BF30 would run here.
                            // 007B8C90: the block is published and the byte set.
                            unit_.pilot_command_pending_a14 = true;
                            ++owner_.summary.pilot_thinks;
                        }

                        // 007BB920, gated on unit+A14h. The two overrides it
                        // applies - air brake full when the flight state is not
                        // one of {7,6,4,5}, throttle full under a vtable test -
                        // are not modelled: this host's plane is always in state
                        // 7 for the first, and the second needs 00C24h and a
                        // vtable slot it does not have.
                        if (!unit_.pilot_command_pending_a14) {
                            return;
                        }
                        // 007BB6E0, one axis at a time, into the live block.
                        unit_.plane_live_controls[0] = bsp::pilot_quantize_control_axis_007bb6e0(
                            unit_.pilot_command_block[bsp::kPilotCmdYaw]);
                        unit_.plane_live_controls[1] = bsp::pilot_quantize_control_axis_007bb6e0(
                            unit_.pilot_command_block[bsp::kPilotCmdPitch]);
                        unit_.plane_live_controls[2] = bsp::pilot_quantize_control_axis_007bb6e0(
                            unit_.pilot_command_block[bsp::kPilotCmdRoll]);
                        unit_.plane_live_throttle = bsp::pilot_quantize_control_axis_007bb6e0(
                            unit_.pilot_command_block[bsp::kPilotCmdThrottle]);
                        unit_.plane_live_air_brake = bsp::pilot_quantize_control_axis_007bb6e0(
                            unit_.pilot_command_block[bsp::kPilotCmdAirBrake]);
                        unit_.pilot_command_pending_a14 = false;   // 007BB990
                        ++owner_.summary.pilot_commits;
                        owner_.record("Plane::commit_pilot_command", 0x007bb920u);
                    }

                    // 007C1900's three attitude angles, from the live pose.
                    // They feed both the control targets (sin(bank), cos(pitch))
                    // and the planner's yaw arm (cos(bank), |bank|, heading), so
                    // they are computed once per step and cached on the slot.
                    void refresh_attitude_007c1900() {
                        bsp::AdvanceMatrix pose{};
                        const float* const rows[3] = {unit_.motion.pose_row0,
                            unit_.motion.pose_row1, unit_.motion.pose_row2};
                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                pose.m[r * 4 + c] = rows[r][c];
                            }
                            pose.m[r * 4 + 3] = 0.0f;
                        }
                        pose.m[15] = 1.0f;
                        bsp::NativeAdvanceMatrixOps ops;
                        const bsp::PlaneAttitudeAngles a =
                            bsp::plane_attitude_angles_007c1900(pose, ops);
                        unit_.plane_pitch_angle_c64 = a.pitch;
                        // The native leaves the previous heading and bank in
                        // place when the forward axis is too near vertical, and
                        // so does this - which is why they are slot state rather
                        // than locals.
                        if (a.heading_and_bank_written) {
                            unit_.plane_heading_c6c = a.heading;
                            unit_.plane_bank_angle_c68 = a.bank;
                        }
                    }

                    // 009AC190's bearing into plan+2C0h, then 0099D300's yaw arm
                    // into the yaw slot's `desired`.
                    //
                    // The bearing is to the commanded target's position. The
                    // native latches that position once at task construction and
                    // never re-reads the entity; this host re-reads it each
                    // think, which is a DIVERGENCE and is flagged rather than
                    // hidden - against a stationary or slow target the two agree,
                    // and against a manoeuvring one the native's bot would aim at
                    // where the target was when the order was given.
                    //
                    // Everything downstream of the slot is the recovered
                    // reconstruction in plane_ai_control.hpp: the base numerator
                    // 0099DE8A, the bank fade 0099DFFB, and the arm 0099E81A.
                    // The turn numerator stays 0 - its producers at
                    // 0099E6D2/E6E8/E729 are in the pitch arm, which is unread -
                    // so the turn term is off and the plane holds heading with
                    // what the law calls its rudder.
                    bool plan_yaw_0099d300() {
                        if (unit_.command_target_plus_one == 0) return false;
                        const std::size_t target_index =
                            unit_.command_target_plus_one - 1;
                        if (target_index >= owner_.slots.size()) return false;

                        const float* const target_pos =
                            owner_.slots[target_index]->motion.position;
                        // 009AC40B writes the bearing into plan+2C0h with mode
                        // 2 at 009AC41B, and a bot state's tick may overwrite
                        // the same pair - the torpedo aim tick does at 009D1D16.
                        // 0099DEB8 reads plan+2C0h, never the raw bearing, so
                        // the state's heading wins whenever it has written one.
                        const float arm_heading =
                            bsp::plane_bearing_to_target_009ac190(
                                unit_.motion.position, target_pos);
                        const float desired_heading =
                            (unit_.plan_heading_2c0_written &&
                             unit_.plan_heading_mode_2cc == 2)
                                ? unit_.plan_heading_2c0
                                : arm_heading;
                        {
                            const double dx = target_pos[0] - unit_.motion.position[0];
                            const double dy = target_pos[1] - unit_.motion.position[1];
                            const double dz = target_pos[2] - unit_.motion.position[2];
                            const float range =
                                static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
                            if (unit_.attack_range_first < 0.0f) {
                                unit_.attack_range_first = range;
                            }
                            unit_.attack_range_last = range;
                        }

                        bsp::PilotBotHeadingTerm term;
                        term.heading_error = bsp::wrapped_angle_subtract_00438b10(
                            desired_heading, unit_.plane_heading_c6c);
                        // 1 / max(unit+340h * 0.4, 1.0). The time scale is 0 in
                        // this host, so the divisor is 1.
                        term.speed_scale = 1.0f;
                        bsp::PilotBotTuning tuning;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            term.deadband = g.pilot_general_soft_hdg_zone;
                            term.rate_scale = g.pilot_general_soft_hdg_limit;
                            term.keep = g.pilot_general_soft_hdg_mul;
                            tuning.blend_x0 = g.pilot_general_yaw_turn_roll_range_1;
                            tuning.blend_x1 = g.pilot_general_yaw_turn_roll_range_2;
                            tuning.rate = g.pilot_general_yaw_ctrl_set_time_mul;
                        }
                        term.rate_a = unit_.plane_class_turn_roll_spd;  // class+1C8h
                        term.rate_b = unit_.plane_class.pitch_spd;  // class+1ACh

                        {
                            const float e = std::fabs(term.heading_error);
                            if (unit_.attack_hdg_err_first < 0.0f) {
                                unit_.attack_hdg_err_first = e;
                            }
                            unit_.attack_hdg_err_last = e;
                            unit_.attack_pitch_last = unit_.plane_pitch_angle_c64;
                        }

                        bsp::PilotBotFrame frame;
                        frame.pitch = unit_.plane_pitch_angle_c64;
                        frame.sin_bank = std::sin(unit_.plane_bank_angle_c68);
                        frame.cos_bank = std::cos(unit_.plane_bank_angle_c68);
                        frame.cos_pitch = std::cos(unit_.plane_pitch_angle_c64);
                        frame.abs_bank = std::fabs(unit_.plane_bank_angle_c68);

                        bsp::PilotBotYawScratch scratch;
                        scratch.base_num = bsp::yaw_base_numerator_0099de8a(term);
                        scratch.base_gain =
                            bsp::yaw_base_gain_0099dffb(tuning, frame.abs_bank);
                        scratch.turn_num = 0.0f;

                        const float desired = bsp::plan_yaw_0099e81a(
                            frame, tuning, scratch, unit_.plane_class.yaw_spd);
                        unit_.plan_slots[bsp::kPilotSlotYaw].desired = desired;
                        unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;  // 0099EA46

                        // 0099DE93-0099E39D, the roll arm. Without it the plane
                        // banks unopposed under 007DA710's own yaw-roll coupling
                        // and never rolls level again - a measured run held two
                        // aircraft at 59 degrees of bank for two and a half
                        // minutes, which is a state the game's own bot would
                        // never leave them in.
                        bsp::PilotBotRollInputs rin;
                        rin.heading_error = term.heading_error;
                        rin.bank = unit_.plane_bank_angle_c68;
                        rin.dt_scale = 1.0f;
                        rin.turn_scale_2e8 = unit_.plan_state.turn_scale_2e8;
                        rin.bank_limit_2c8 = unit_.plan_state.bank_limit_2c8;
                        // 0047B880 is read (docs/PILOT_BANK_COMMAND_INPUTS.md):
                        // !(vtable[5Ch](10h) || vtable[5Ch](16h)), and 0099D0A0
                        // tests the SAME disjunction inline at 0099D1C2-0099D1D9
                        // on the same unit to cap its rate multiplier at 1.0f.
                        // One bit and its complement, so only this one is set
                        // and pilot_plan_roll_0099e2ba derives the
                        // TurnRollLimitSmall/Large choice from it; that is why
                        // the rin.small_turn_roll_limit assignment that used to
                        // sit here is gone. Slot 5Ch on a plane is
                        // BSP_PlaneInstance_IsKindOf (00D05F20+5Ch -> 0074E400),
                        // the same class test bsp::unit_is_kind_of models, and
                        // src/unit_kind_query.cpp names the two literals:
                        // 10h MPlaneBomber, 16h MLargeReconPlane.
                        rin.scale.caps_rate_at_one =
                            bsp::unit_is_kind_of(unit_.class_id, 0x10) ||
                            bsp::unit_is_kind_of(unit_.class_id, 0x16);
                        rin.roll_spd = unit_.plane_class.roll_spd;
                        rin.roll_accel = unit_.plane_class.roll_accel;
                        rin.scale.pitch_angle = unit_.plane_pitch_angle_c64;
                        rin.scale.pitch_command =
                            unit_.plan_slots[bsp::kPilotSlotPitch].active != 0
                                ? unit_.plan_slots[bsp::kPilotSlotPitch].desired
                                : unit_.plan_slots[bsp::kPilotSlotPitch].current;
                        rin.scale.roll_spd = unit_.plane_class.roll_spd;
                        rin.scale.pitch_spd = unit_.plane_class.pitch_spd;
                        rin.scale.yaw_spd = unit_.plane_class.yaw_spd;
                        rin.scale.slide_ratio = unit_.plane_class.slide_ratio;
                        rin.scale.roll_accel = unit_.plane_class.roll_accel;
                        rin.scale.turn_roll_spd = unit_.plane_class_turn_roll_spd;
                        rin.scale.turn_roll = unit_.plane_class_turn_roll;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            rin.turn_roll_limit_small = g.pilot_general_turn_roll_limit_small;
                            rin.turn_roll_limit_large = g.pilot_general_turn_roll_limit_large;
                            rin.turn_roll_pitch_limit_pitch_1 =
                                g.pilot_general_turn_roll_pitch_limit_pitch_1;
                            rin.turn_roll_pitch_limit_pitch_2 =
                                g.pilot_general_turn_roll_pitch_limit_pitch_2;
                            rin.turn_roll_pitch_limit_roll_1 =
                                g.pilot_general_turn_roll_pitch_limit_roll_1;
                            rin.turn_roll_pitch_limit_roll_2 =
                                g.pilot_general_turn_roll_pitch_limit_roll_2;
                            rin.pitch_turn_hdg_range_1 = g.pilot_general_pitch_turn_hdg_range_1;
                            rin.pitch_turn_hdg_range_2 = g.pilot_general_pitch_turn_hdg_range_2;
                            rin.soft_roll_ctrl = g.pilot_general_soft_roll_ctrl;
                            rin.soft_roll_mul = g.pilot_general_soft_roll_mul;
                            rin.soft_roll_offset = g.derived_580;
                            rin.waggle_limit = g.pilot_general_waggle_limit;
                            rin.scale.hdg_diff_calc_limit_1 =
                                g.pilot_general_hdg_diff_calc_limit_1;
                            rin.scale.hdg_diff_calc_limit_2 =
                                g.pilot_general_hdg_diff_calc_limit_2;
                            rin.scale.hdg_diff_calc_min_pitch =
                                g.pilot_general_hdg_diff_calc_min_pitch;
                            rin.scale.hdg_diff_calc_min_roll =
                                g.pilot_general_hdg_diff_calc_min_roll;
                        }
                        // The pitch error the bank limit is scheduled on.
                        rin.pitch_error = bsp::wrapped_angle_subtract_00438b10(
                            unit_.plane_pitch_angle_c64, unit_.plan_state.pitch_target_2bc);
                        const bsp::PilotBotRollResult roll =
                            bsp::pilot_plan_roll_0099e2ba(rin);
                        unit_.plan_state.bank_target_2c4 = roll.bank_target;
                        unit_.plan_slots[bsp::kPilotSlotRoll].desired = roll.desired;
                        unit_.plan_slots[bsp::kPilotSlotRoll].active = 1;  // 0099E3AE

                        // 0099E490-0099E739, the pitch arm. Without it a planned
                        // bot follows whatever plan+2BCh was last set to, which
                        // is downward: a measured run with only the yaw arm
                        // wired turned correctly toward its target and reached
                        // a 63-degree dive doing it. The floor inside
                        // pilot_pitch_demand_0099e490 is the whole of what stops
                        // that, and it is an ATTITUDE floor - nothing in this
                        // chain reads an altitude.
                        bsp::PilotBotPitchInputs pin;
                        pin.bank = unit_.plane_bank_angle_c68;
                        pin.pitch = unit_.plane_pitch_angle_c64;
                        // Mode 2 holds unit+C84h; this host does not model that
                        // arm, so the measured angle is the live pitch, which is
                        // what the native's own `else` branch at 0099DD54 uses.
                        pin.held_pitch = unit_.plane_pitch_angle_c64;
                        pin.pitch_target = unit_.plan_state.pitch_target_2bc;
                        pin.heading_error = term.heading_error;
                        pin.control_authority = control_authority(
                            unit_.plane_world_velocity[0] * unit_.motion.pose_row2[0] +
                            unit_.plane_world_velocity[1] * unit_.motion.pose_row2[1] +
                            unit_.plane_world_velocity[2] * unit_.motion.pose_row2[2]);
                        pin.dt_scale = 1.0f;
                        pin.turn_roll = unit_.plane_class_turn_roll;
                        pin.pitch_spd = unit_.plane_class.pitch_spd;
                        pin.yaw_spd = unit_.plane_class.yaw_spd;
                        pin.slide_ratio = unit_.plane_class.slide_ratio;
                        pin.negative_pitch_ratio = unit_.plane_class.negative_pitch_ratio;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            pin.pitch_turn_max_pitch = g.pilot_general_pitch_turn_max_pitch;
                            pin.pitch_turn_hdg_range_1 = g.pilot_general_pitch_turn_hdg_range_1;
                            pin.pitch_turn_hdg_range_2 = g.pilot_general_pitch_turn_hdg_range_2;
                            pin.pitch_ctrl_set_time_mul = g.pilot_general_pitch_ctrl_set_time_mul;
                        }
                        const bsp::PilotBotPitchResult pitch =
                            bsp::pilot_pitch_demand_0099e490(pin);
                        unit_.plan_state.pitch_target_2bc = pitch.floored_target;
                        unit_.plan_slots[bsp::kPilotSlotPitch].desired =
                            bsp::plan_pitch_0099e68d(pitch.demand);
                        unit_.plan_slots[bsp::kPilotSlotPitch].active = 1;  // 0099E741
                        owner_.record("PilotBot::plan_controls", 0x0099d300u);
                        return true;
                    }

                    void advance_pose_0085e4d0(float step) {
                        bsp::AdvanceMatrix live{};
                        const float* const rows[3] = {unit_.motion.pose_row0,
                            unit_.motion.pose_row1, unit_.motion.pose_row2};
                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                live.m[r * 4 + c] = rows[r][c];
                            }
                            live.m[r * 4 + 3] = 0.0f;
                            live.m[12 + r] = unit_.motion.position[r];
                        }
                        live.m[15] = 1.0f;

                        bsp::ControllerVelocities vel;
                        for (int i = 0; i < 3; ++i) {
                            vel.angular_body[i] = unit_.plane_body_angular[i];
                        }
                        bsp::body_to_world_007d9c80(vel, live);

                        // 00F87574, the eye the look-at is built around. It is
                        // the zero vector - twelve zero bytes in the
                        // uninitialised part of .data, and three docs already
                        // name it so - which puts the look-at's forward exactly
                        // on the normalised axis. It stays a parameter because
                        // nothing proves no one writes it.
                        static const float eye_00f87574[3] = {0.0f, 0.0f, 0.0f};
                        bsp::NativeAdvanceMatrixOps ops;
                        bsp::AdvanceMatrix rotated{};
                        if (!bsp::rotate_about_axis_0085e4d0(rotated, live,
                                vel.angular_world, step, eye_00f87574, ops)) {
                            return;
                        }
                        const float before = heading_of(rows[2]);
                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                const_cast<float*>(rows[r])[c] = rotated.m[r * 4 + c];
                            }
                        }
                        ++owner_.summary.plane_pose_rotations;
                        // atan2's branch cut, wrapped out. Without this a plane
                        // that turns past +-pi books a spurious 2*pi: a probe
                        // that injected a known 0.2 rad/s about body Y read
                        // 219 rad where 100 was owed, and the 119 was twenty
                        // planes crossing the cut roughly once each.
                        double delta = static_cast<double>(heading_of(rows[2]) - before);
                        while (delta > kPi) {
                            delta -= 2.0 * kPi;
                        }
                        while (delta < -kPi) {
                            delta += 2.0 * kPi;
                        }
                        owner_.summary.plane_heading_change += std::fabs(delta);
                    }

                    static float heading_of(const float forward[3]) {
                        return static_cast<float>(std::atan2(
                            static_cast<double>(forward[0]),
                            static_cast<double>(forward[2])));
                    }

                    // 007D9A70 BSP_PlaneFlight_ControlAuthority, the scalar that
                    // scales all three rotation accelerations.
                    // docs/PLANE_CONTROL_AUTHORITY.md has the derivation.
                    //
                    // Two of its inputs are not modelled and are passed as the
                    // values they would take rather than approximated: the
                    // damage scale 007C0F40 returns 1.0 for a plane with no
                    // parts on unit+974h, which is every plane here, and
                    // (ctl+10h)->+0C0h has no identified owner so its term is
                    // zero. Both are named in the comment so a later packet can
                    // find them; neither is a guess dressed as a value.
                    float control_authority(float forward_speed) const {
                        const float a = bsp::clamped_interpolate_00419010(
                            3.0f, 0.0f, 6.0f, 0.25f, unit_.plane_airborne_908);
                        const float stall = unit_.plane_stall_spd > 0.0f
                            ? unit_.plane_stall_spd : 17.5f;
                        const float s = forward_speed / stall;
                        float range_min = 1.1f;
                        float range_max = 1.7f;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            range_min = g.dynamics_spd_multipliers_control_range_min;
                            range_max = g.dynamics_spd_multipliers_control_range_max;
                        }
                        const float r = bsp::clamped_interpolate_00419010(
                            range_min, 0.0f, range_max, 1.0f, s);
                        const float b = r;   // + (ctl+10h)->+0C0h * 0.6, unmodelled
                        // 007D9B10..007D9B6E, the literal three-way pick.
                        float t = 0.0f;
                        if (a > b) {
                            t = a;
                        } else {
                            t = (b <= 1.0f) ? b : 1.0f;
                        }
                        return t * t;   // x 007C0F40, which is 1.0 here
                    }

                    // 007B9770 BSP_Plane_LatchControlInput, then 007DA710's
                    // target build and rate law. This is the whole control half
                    // of a plane's step, and it runs after the motion arm so the
                    // forward speed it reads is this step's.
                    //
                    // docs/PLANE_BOT_CONTROL_WRITEBACK.md establishes that the
                    // live block is written by the bot chain through
                    // 007B8C90 -> 007BB920 -> 007BB6E0; none of that is built
                    // here, so plane_live_controls stays zero and every target
                    // is zero. The rate law then holds each axis at zero, which
                    // is the correct behaviour for a plane with a centred stick
                    // and is why nothing turns yet.
                    void control_step_007da710(float step, float forward_speed) {
                        // 007B9783 / 007B979C / 007B97A8: the previous-step
                        // snapshot, +9E4h -> +BB0h and so on.
                        for (int i = 0; i < 3; ++i) {
                            unit_.plane_latched_controls[i] = unit_.plane_live_controls[i];
                        }
                        // The same snapshot for the two axes the drag and thrust
                        // read: unit+0BBCh is the latched throttle (007DB76C
                        // gates the whole thrust term on it exceeding 0.01f) and
                        // unit+0BB0h+10h the latched air brake (007D91E0 pairs it
                        // with AirBrakeDrag).
                        unit_.plane_latched_throttle = unit_.plane_live_throttle;
                        unit_.plane_latched_air_brake = unit_.plane_live_air_brake;

                        bsp::PlaneControlUnitState state;
                        state.latched_yaw = unit_.plane_latched_controls[0];
                        state.latched_pitch = unit_.plane_latched_controls[1];
                        state.latched_roll = unit_.plane_latched_controls[2];
                        state.flight_state_900 = unit_.plane_control_mode_900;
                        // 007DC841 zeroes ctl+FCh every step, so free flight is
                        // mode 0 and the roll target is not zeroed.
                        state.controller_mode_fc = 0;

                        const float m = control_authority(forward_speed);
                        // Free flight takes 007DA380's mode-0 arm, which writes
                        // the same scalar to both outputs and sets the flag to 1.
                        const bsp::PlaneControlTargets targets =
                            bsp::plane_control_targets_007da710(
                                unit_.plane_class, state, m, m, true);

                        bsp::PlaneRotationFactors factors;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            factors.a = g.dynamics_rotation_factors_a;
                            factors.b = g.dynamics_rotation_factors_b;
                            factors.c = g.dynamics_rotation_factors_c;
                        }
                        for (int axis = 0; axis < 3; ++axis) {
                            bsp::PlaneControlAxisState in;
                            in.current = unit_.plane_body_angular[axis];
                            in.target = targets.target[axis];
                            in.accel = targets.accel[axis];
                            unit_.plane_body_angular[axis] =
                                bsp::plane_control_axis_step_007da710(factors, in, true, step);
                        }
                        owner_.record("PlaneFlight::control_rate_law", 0x007da710u);
                    }

                    void ground_roll_007cbfa0(float) override {
                        ++owner_.summary.plane_arm_ground_roll;
                    }
                    void surface_007cba50(float) override {
                        ++owner_.summary.plane_arm_surface;
                    }
                    // 007CECBA on unit+674h. Not a pose *commit* despite the
                    // name this interface inherited from a doc annotation that
                    // turned out to be wrong: 0085DC80 is
                    // BSP_Matrix_OrthonormalizeBasisRows, a general Gram-Schmidt
                    // with 54 callers that takes no step, no velocity and no
                    // control axis. docs/PLANE_POSE_COMMIT.md.
                    //
                    // On an already-orthonormal basis it is a no-op, and the
                    // host's pose is orthonormal because it comes from the
                    // authored placement and nothing rotates it yet. So this
                    // changes nothing today and is wired for faithfulness, not
                    // effect - it is the drift tidy-up that will matter once
                    // something actually turns a plane.
                    //
                    // No zero-length guard: the native has none, and a zero or
                    // NaN forward silently collapses the basis there too. The
                    // counter below records it instead of hiding it.
                    void commit_step_pose_0085dc80() override {
                        bsp::PoseBasis in;
                        for (int i = 0; i < 3; ++i) {
                            in.row0[i] = unit_.motion.pose_row0[i];
                            in.row1[i] = unit_.motion.pose_row1[i];
                            in.row2[i] = unit_.motion.pose_row2[i];
                        }
                        const bsp::PoseOrthonormalizeResult r =
                            bsp::orthonormalize_basis_rows_0085dc80(in);
                        for (int i = 0; i < 3; ++i) {
                            unit_.motion.pose_row0[i] = r.basis.row0[i];
                            unit_.motion.pose_row1[i] = r.basis.row1[i];
                            unit_.motion.pose_row2[i] = r.basis.row2[i];
                        }
                        if (r.branch == bsp::PoseOrthonormalizeBranch::RightReference)
                            ++owner_.summary.plane_pose_right_reference;
                        if (!(r.forward_length > 0.0f))
                            ++owner_.summary.plane_pose_collapsed;
                        owner_.done("PlaneMotion::commit_step_pose_0085dc80", 0x0085dc80u);
                    }
                    void latch_control_input_007b9770() override {
                        // 007CE96F, and the release-order issue stage begins at
                        // 007CE9FD, 0x8E bytes later, with no branch between
                        // that could skip it: 007CE99D's JE lands on 007CE9FD
                        // itself. So the stage runs on every fixed step of
                        // every arm, which is what this position models.
                        run_release_issue_stage_007ce9fd(fixed_step_seconds_);
                    }
                    bool airborne_time_frozen() override {
                        return unit_.plane_airborne_frozen_9e0;
                    }
                    int ground_water_mode() override {
                        return unit_.plane_control_mode_900;
                    }
                    int surface_mode() override {
                        // The surface arm's documented unit+5F0h is the same
                        // field: 007CEC39's LEA proves unit+72Ch is embedded.
                        return unit_.plane_control_mode_900;
                    }
                private:
                    GameUnitsHost::Impl& owner_;
                    GameUnitSlot& unit_;
                    // 00953CC0's argument, held for the 007CE9FD stage.
                    float fixed_step_seconds_{0.0f};
                } plane_calls(host, slot);
                const bsp::PlaneMotionArm arm =
                    bsp::run_plane_fixed_step_007ce040(plane_calls, step_seconds);
                if (arm == bsp::PlaneMotionArm::None) ++host.summary.plane_arm_none;
                ++host.summary.plane_steps;
                host.done("UnitMotion::plane_fixed_step_007ce040", 0x007ce040u);
                continue;
            }
            if (slot.motion_dispatch.entry != 0) {
                host.record_motion_phase("unreconstructed_phase", slot.motion_dispatch.entry);
            } else {
                host.record_slot("UnitMotion::unresolved_dispatch", "unit+310h/vtable+8h");
            }
            continue;
        }
        // 00825f2c..00825f7c, the head of BSP_UnitInstance_UpdateShipMotion:
        // the promotion of the slot the AI controller just published, before
        // anything else the routine does.
        if (host.ship_ai != nullptr) host.ship_ai->promote_order_00825f2c(index);
        // The order under the write cursor is refilled every step so a standing
        // order keeps standing: the game does that from the HUD every frame the
        // key is held, and the ring's own forward copy at 00813186 would
        // otherwise only mark later slots predicted.
        if (slot.standing_order) {
            host.issue_into_ring(slot, slot.standing_throttle, slot.standing_rudder);
        }
        // Milestone 2n: 009e1170's AI arm is no longer run from here. It is the
        // `cruise` state's vtable +0Ch, and 009f5186 calls it on a re-plan tick
        // of the controller above, which is what milestone 2l's own correction
        // from docs/SHIP_AI_STATES.md says. When no controller is attached the
        // step does not run at all rather than running on the wrong schedule.
        UnitRudderBinding rudder(host, slot);
        ShipMotionBinding motion(host, slot, rudder);
        const float before[3] = {slot.motion.position[0], slot.motion.position[1],
            slot.motion.position[2]};
        const bsp::ShipMotionStepResult result
            = bsp::ship_motion_step_00825f20(slot.motion, slot.motion_class, motion,
                step_seconds);
        // 00749B2C, 0085542F and 0075827B pass the unchanged tick receiver and
        // float argument to 00825F20 before any branch. Only that base-call
        // fragment runs here; each native override's following work is open.
        if (slot.motion_dispatch.coverage == UnitMotionCoverage::ship_base_fragment) {
            host.record_motion_phase("unreconstructed_override_remainder",
                slot.motion_dispatch.entry);
        }
        // The Dyn library's own two integration phases, in the order 00c5bb30
        // runs them. The motion tick has just written both velocities onto the
        // body through 00c37e50 / 00c37e20, so the velocity phase 00c41550 sees
        // no force, no gravity and no damping and only rebuilds the world
        // inverse inertia, and the position phase 00c5b1b0 is what turns the two
        // velocities into a pose. docs/RIGID_BODY_INTEGRATION.md.
        if (!host.logged_integrator) {
            host.logged_integrator = true;
            host.log.notef("rigid body: 00c41550 then 00c5b1b0, one substep of the whole "
                "%.4f s game step. Milestone 2r builds the hull body through the tail of "
                "00937c90, so \"%s\" carries mass %.1f (inverse %.8f), angular damping "
                "%.1f, linear damping %.1f, both speed clamps at %.1f, torque-lock=%d and "
                "physics material %d; the box inertia is zero because the collision AABB's "
                "producer 00c5c940 is unread. The linear damping of 0.0f is the game's own "
                "descriptor default (00939295), not a gap: the only routine that resists a "
                "hull's lateral velocity is 009329c0, the hydrodynamic tail of 00937440",
                static_cast<double>(step_seconds), slot.row.name.c_str(),
                static_cast<double>(slot.motion_class.hull_mass),
                static_cast<double>(slot.motion_state.inverse_mass),
                static_cast<double>(slot.motion_state.angular_damping),
                static_cast<double>(slot.motion_state.linear_damping),
                static_cast<double>(slot.motion_state.max_linear_speed),
                slot.motion_state.lock_torque_to_row1 ? 1 : 0,
                static_cast<int>(slot.hull_material));
        }
        host.record("ShipMotion::rigid_body_substep_schedule", 0x00c5bb30u);
        slot.motion_state.linear_velocity = slot.motion.linear_velocity;
        slot.motion_state.angular_velocity = slot.motion.angular_velocity;
        slot.body.motion = &slot.motion_state;
        for (int i = 0; i < 3; ++i) {
            slot.body.row0[i] = slot.motion.pose_row0[i];
            slot.body.row1[i] = slot.motion.pose_row1[i];
            slot.body.row2[i] = slot.motion.pose_row2[i];
            slot.body.position[i] = slot.motion.position[i];
        }
        bsp::dyn_body_integrate_velocity_00c41550(slot.body, host.physics_world,
            step_seconds);
        host.done("ShipMotion::rigid_body_velocity_phase", 0x00c41550u);
        bsp::dyn_body_integrate_position_00c5b1b0(slot.body, host.physics_world,
            step_seconds);
        host.done("ShipMotion::rigid_body_position_phase", 0x00c5b1b0u);
        slot.motion.linear_velocity = slot.motion_state.linear_velocity;
        slot.motion.angular_velocity = slot.motion_state.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            slot.motion.pose_row0[i] = slot.body.row0[i];
            slot.motion.pose_row1[i] = slot.body.row1[i];
            slot.motion.pose_row2[i] = slot.body.row2[i];
            slot.motion.position[i] = slot.body.position[i];
        }
        Impl::publish_pose(slot);
        const double dx = static_cast<double>(slot.motion.position[0]) - before[0];
        const double dy = static_cast<double>(slot.motion.position[1]) - before[1];
        const double dz = static_cast<double>(slot.motion.position[2]) - before[2];
        slot.row.path_length += static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
        // Milestone 2s: the drift angle and the trajectory speed, measured from
        // the step's own displacement rather than from the velocity, so they
        // report where the hull went and not what it was told. The drift is the
        // angle in the horizontal plane between that displacement and the
        // hull's forward axis (pose row 2), which is the quantity milestone 2r's
        // section 2 table reports for both the executable and the probe.
        {
            const double planar = std::sqrt(dx * dx + dz * dz);
            slot.row.trajectory_speed = (step_seconds > 0.0f)
                ? static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz)
                    / static_cast<double>(step_seconds))
                : 0.0f;
            if (planar > 1.0e-6) {
                const double fx = slot.motion.pose_row2[0];
                const double fz = slot.motion.pose_row2[2];
                const double forward = std::sqrt(fx * fx + fz * fz);
                if (forward > 1.0e-6) {
                    double cosine = (dx * fx + dz * fz) / (planar * forward);
                    if (cosine > 1.0) cosine = 1.0;
                    if (cosine < -1.0) cosine = -1.0;
                    slot.row.drift_degrees
                        = static_cast<float>(std::acos(cosine) * 180.0 / 3.14159265358979323846);
                    if (slot.row.drift_degrees > slot.row.peak_drift_degrees) {
                        slot.row.peak_drift_degrees = slot.row.drift_degrees;
                    }
                }
            }
        }
        slot.row.command_applied = result.gate.command_applies;
        if (!host.logged_gate) {
            host.logged_gate = true;
            // 00826994: the command is suppressed when the keel sample point has
            // risen above half the local wave height. With the flat-sea stand-in
            // that is a test of the authored hull height against zero, so a
            // reader can tell a closed gate from a missing class row.
            host.log.notef("ship motion gate on \"%s\": keel=(%.2f, %.2f, %.2f) wave=%.2f "
                "applies=%d throttle=%.3f target_speed=%.3f engine_gate=%.1f",
                slot.row.name.c_str(), static_cast<double>(result.keel_point.x),
                static_cast<double>(result.keel_point.y),
                static_cast<double>(result.keel_point.z),
                static_cast<double>(result.wave_height),
                result.gate.command_applies ? 1 : 0,
                static_cast<double>(result.gate.throttle),
                static_cast<double>(result.target_speed),
                static_cast<double>(result.engine_gate));
        }
        ++slot.row.motion_ticks;
        ++host.summary.motion_ticks;
        host.done("World::unit_motion_tick", 0x00825f20u);
        host.refresh_row(slot);
    }
    host.summary.total_path_length = 0.0f;
    for (const std::unique_ptr<GameUnitSlot>& slot : host.slots) {
        host.summary.total_path_length += slot->row.path_length;
    }
    if (host.controlled_bound && host.controlled_index < host.slots.size()) {
        host.summary.controlled_distance = host.slots[host.controlled_index]->row.distance;
    }
}

// ---------------------------------------------------------------------------
// Milestone 2n: what the ship AI controller reads off a unit
// ---------------------------------------------------------------------------

void GameUnitsHost::set_ship_ai(GameShipAiHost* ai) noexcept {
    impl_->ship_ai = ai;
    if (impl_->gunnery != nullptr) impl_->gunnery->set_ship_ai(ai);
}

GameGunneryHost* GameUnitsHost::gunnery() noexcept { return impl_->gunnery.get(); }
const GameGunneryHost* GameUnitsHost::gunnery() const noexcept {
    return impl_->gunnery.get();
}

std::uint32_t GameUnitsHost::director_current_command_0071be40(std::size_t index) const {
    return impl_->commands.current_command_0071be40(index);
}

bool GameUnitsHost::director_avoidance(std::size_t index, GameDirectorAvoidance& out) const {
    return impl_->commands.director_avoidance(index, out);
}

bool GameUnitsHost::apply_director_avoidance_message_00835640(std::size_t index,
    const bsp::DirectorCommandMessage& message) {
    return impl_->commands.apply_director_avoidance_message_00835640(index, message);
}

std::size_t GameUnitsHost::report_command_event_00984300(std::size_t index,
    std::uint32_t command_object, const char* status) {
    return impl_->commands.report_command_event_00984300(index, command_object, status);
}

GameCommandCompletion GameUnitsHost::end_command_0071e430(std::size_t index,
    std::uint32_t command_object, bool terminal) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return GameCommandCompletion{};
    GameUnitSlot& slot = *host.slots[index];
    return host.commands.end_command_0071e430(index, command_object, terminal,
        unit_player_controlled_0184(index), slot.ring, Impl::pose_heading_radians(slot));
}

bool GameUnitsHost::unit_player_controlled_0184(std::size_t index) const {
    const Impl& host = *impl_;
    return host.controlled_bound && host.controlled_index == index;
}

bool GameUnitsHost::unit_current_role_slot(std::size_t index, std::int32_t role_index,
    std::int32_t& out) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size() || role_index < 0
        || role_index >= bsp::kUnitRoleTableEntries) return false;
    out = host.slots[index]->current_roles_01ac[role_index];
    return true;
}

void GameUnitsHost::store_unit_command_target(std::size_t index,
                                              std::size_t target_plus_one) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->command_target_plus_one = target_plus_one;
}

void GameUnitsHost::store_unit_ordnance(std::size_t index, std::uint64_t mask) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->ordnance_mask = mask;
}

std::uint64_t GameUnitsHost::unit_ordnance(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return 0;
    return impl_->slots[index]->ordnance_mask;
}

bool GameUnitsHost::unit_flag_005d(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->state->simulate != 0;
}

bool GameUnitsHost::unit_flag_0061(std::size_t index) const {
    // docs/UNIT_AUTOPILOT_PAIR.md scanned .text for a writer of unit+61h at the
    // direct displacement and at the seven shifted unit bases and found none, so
    // nothing in this process or in the recovered load can set it. The byte is
    // clear, which is what lets the ship AI controller run at all.
    static_cast<void>(index);
    return false;
}

float GameUnitsHost::unit_forward_speed_0092d730(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const GameUnitSlot& slot = *host.slots[index];
    bsp::UnitBodyAxisSpeedInputs axis{};
    axis.velocity[0] = slot.motion.linear_velocity.x;
    axis.velocity[1] = slot.motion.linear_velocity.y;
    axis.velocity[2] = slot.motion.linear_velocity.z;
    axis.axis[0] = slot.motion.pose_row2[0];
    axis.axis[1] = slot.motion.pose_row2[1];
    axis.axis[2] = slot.motion.pose_row2[2];
    return bsp::unit_forward_speed_0092d730(axis);
}

float GameUnitsHost::unit_retardation_0508(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->motion_class.retardation;
}

float GameUnitsHost::unit_heading_radians(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return Impl::pose_heading_radians(*host.slots[index]);
}

float GameUnitsHost::unit_current_yaw_rate_00811940(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    UnitRudderBinding rudder(host, *host.slots[index]);
    return bsp::unit_current_yaw_rate_00811940(rudder);
}

bool GameUnitsHost::run_cruise_state_step_009e1170(std::size_t index,
    bsp::ShipAiControlBlock& blk, bsp::ShipAiSetterHost& setters) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    bsp::CruiseOrderedValues ordered{};
    return host.commands.cruise_step(index, host.is_controlled(slot),
        unit_forward_speed_0092d730(index),
        bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled),
        ordered, &blk, &setters);
}

bool GameUnitsHost::run_cruise_state_step_009e1170(std::size_t index,
    bsp::ShipAiControlBlock& blk, bsp::ShipAiSetterHost& setters,
    bsp::ShipAiAvoidanceRequest& request,
    const bsp::ShipAiCruiseAvoidanceInputs& avoidance_inputs) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    bsp::CruiseOrderedValues ordered{};
    return host.commands.cruise_step(index, host.is_controlled(slot),
        unit_forward_speed_0092d730(index),
        bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled),
        ordered, &blk, &setters, &request, &avoidance_inputs);
}

// ---------------------------------------------------------------------------
// Milestone 2o: the ring's write slot, which the tail of 009f3f80 reads and
// then writes back through 0080e170 / 0080e190.
// ---------------------------------------------------------------------------

float GameUnitsHost::unit_ring_write_slot_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_throttle(ring);
}

float GameUnitsHost::unit_ring_write_slot_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_rudder(ring);
}

void GameUnitsHost::unit_ring_set_write_slot_throttle_0080e170(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_throttle_0080e170(host.slots[index]->ring, value);
}

void GameUnitsHost::unit_ring_set_write_slot_rudder_0080e190(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_rudder_0080e190(host.slots[index]->ring, value);
}

float GameUnitsHost::unit_ring_current_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_a;
}

float GameUnitsHost::unit_ring_current_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_b;
}

float GameUnitsHost::unit_yaw_authority_0524(std::size_t index, bool& derived) const {
    derived = false;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::ShipClassFields& fields = host.slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    derived = out.derived;
    return out.yaw_authority_0524;
}

bool GameUnitsHost::enable_ai_drive(const std::string& unit_name, float throttle,
    float rudder) {
    Impl& host = *impl_;
    if (host.ship_ai == nullptr) return false;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        if (host.slots[index]->row.name != unit_name) continue;
        host.ship_ai->set_ai_drive(index, throttle, rudder);
        return true;
    }
    host.log.notef("--ai-drive \"%s\" names no created instance; nothing was driven",
        unit_name.c_str());
    return false;
}

bool GameUnitsHost::unit_pose_valid_00c8(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->world_valid != 0;
}

void GameUnitsHost::unit_position_00fc(std::size_t index, float& x, float& y,
    float& z) const {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    const bsp::ShipMotionState& motion = host.slots[index]->motion;
    x = motion.position[0];
    y = motion.position[1];
    z = motion.position[2];
}

std::size_t GameUnitsHost::world_list_size(int class_id) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return 0;
    return impl_->world_lists.entries[class_id].count;
}

std::size_t GameUnitsHost::world_list_entry(int class_id, std::size_t position) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return impl_->slots.size();
    const GameUnitWorldList& list = impl_->world_lists.entries[class_id];
    if (position >= list.count) return impl_->slots.size();
    const GameUnitWorldNode* node = list.head;
    while (position-- != 0) node = node->next;
    return node->unit->process_index;
}

const void* GameUnitsHost::world_list_head(int class_id) const noexcept {
    if (class_id < 0 || class_id >= Impl::kWorldListCount) return nullptr;
    return impl_->world_lists.entries[class_id].head;
}

const void* GameUnitsHost::world_list_node_unit(const void* node) noexcept {
    return static_cast<const GameUnitWorldNode*>(node)->unit;
}

const void* GameUnitsHost::world_list_node_next(const void* node) noexcept {
    return static_cast<const GameUnitWorldNode*>(node)->next;
}

std::size_t GameUnitsHost::count() const noexcept { return impl_->slots.size(); }

bool GameUnitsHost::unit_active(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->state->active != 0;
}

const void* GameUnitsHost::unit_identity(std::size_t index) const noexcept {
    return index < impl_->slots.size() ? impl_->slots[index].get() : nullptr;
}

std::optional<bsp::NativeUnitObserverAlias> GameUnitsHost::observer_alias(
    const void* identity) noexcept {
    if (impl_->observer_runtime == nullptr ||
        !impl_->observer_runtime->has_live_dispatch_owner()) return std::nullopt;
    for (const auto& slot : impl_->slots) {
        if (slot.get() == identity && slot->observer_prefix_ready)
            return bsp::NativeUnitObserverAlias{slot.get(), slot->observer_prefix};
    }
    return std::nullopt;
}

std::optional<bsp::NativeSceneLifecycleView> GameUnitsHost::scene_lifecycle_view(
    const void* identity) noexcept {
    auto alias = observer_alias(identity);
    if (!alias) return std::nullopt;
    for (const auto& slot : impl_->slots) {
        if (slot.get() != identity) continue;
        if (!slot->scene_flags_available || slot->state == nullptr) return std::nullopt;
        return bsp::NativeSceneLifecycleView{*alias, slot->state->active,
            slot->state->simulate, slot->scene_destroyed_005e,
            slot->scene_removed_005f, slot->scene_pending_destroy_0060};
    }
    return std::nullopt;
}

const void* GameUnitsHost::unit_identity_from_observer(
    const bsp::NativeObserverOwnerStorage* endpoint) const noexcept {
    if (impl_->observer_runtime == nullptr ||
        !impl_->observer_runtime->has_live_dispatch_owner()) return nullptr;
    for (const auto& slot : impl_->slots) {
        if (slot->observer_prefix_ready &&
            (endpoint == &slot->observer_prefix.observed_00 ||
                endpoint == &slot->observer_prefix.callback_10)) return slot.get();
    }
    return nullptr;
}

bool GameUnitsHost::unit_scene_node_flags(std::size_t index,
    bsp::SceneNodeFlags& out) const noexcept {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    if (!slot.scene_flags_available || slot.state == nullptr) return false;
    out = {slot.state->active != 0, slot.state->simulate != 0,
        slot.scene_destroyed_005e != 0, slot.scene_removed_005f != 0};
    return true;
}

bool GameUnitsHost::read_scene_node_flags(const void* identity,
    bsp::SceneNodeFlags& out) const noexcept {
    // Compare identities before dereferencing: pending queues must hand over
    // the same stable slot as the world registry, not an index-shaped token.
    for (std::size_t index = 0; index < impl_->slots.size(); ++index) {
        if (impl_->slots[index].get() == identity) return unit_scene_node_flags(index, out);
    }
    return false;
}

bool GameUnitsHost::store_scene_node_flags(const void* identity,
    const bsp::SceneNodeFlags& flags) noexcept {
    for (const auto& owned : impl_->slots) {
        if (owned.get() != identity) continue;
        GameUnitSlot& slot = *owned;
        if (!slot.scene_flags_available || slot.state == nullptr) return false;
        slot.state->active = flags.active;
        slot.state->simulate = flags.torn_down;
        slot.scene_destroyed_005e = flags.destroyed;
        slot.scene_removed_005f = flags.removed;
        slot.row.active = flags.active;
        return true;
    }
    return false;
}

bool GameUnitsHost::unit_pending_destroy_0060(std::size_t index, bool& out) const noexcept {
    if (index >= impl_->slots.size()) return false;
    const GameUnitSlot& slot = *impl_->slots[index];
    if (!slot.scene_flags_available) return false;
    out = slot.scene_pending_destroy_0060 != 0;
    return true;
}

bool GameUnitsHost::store_pending_destroy_0060(const void* identity, bool pending) noexcept {
    for (const auto& owned : impl_->slots) {
        if (owned.get() != identity) continue;
        if (!owned->scene_flags_available) return false;
        owned->scene_pending_destroy_0060 = pending;
        return true;
    }
    return false;
}

bool GameUnitsHost::unit_is_kind_of(std::size_t index, int class_id) const {
    if (index >= impl_->slots.size()) return false;
    return bsp::unit_is_kind_of(impl_->slots[index]->class_id, class_id);
}

int GameUnitsHost::unit_class_id(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return -1;
    return impl_->slots[index]->class_id;
}

bool GameUnitsHost::unit_alive_and_visible(std::size_t index) const {
    bsp::SceneNodeFlags flags;
    bool pending_destroy;
    if (!unit_scene_node_flags(index, flags) ||
        !unit_pending_destroy_0060(index, pending_destroy)) return false;
    // 0043f080: [+5Ch] != 0 && [+5Dh] == 0 && [+60h] == 0 && [+5Eh] == 0.
    return flags.active && !flags.torn_down && !pending_destroy && !flags.destroyed;
}

bool GameUnitsHost::unit_pose(std::size_t index, float right[3], float up[3],
    float forward[3], float translation[3]) const {
    if (index >= impl_->slots.size()) return false;
    const bsp::CameraMatrix& world = impl_->slots[index]->world;
    for (std::size_t lane = 0; lane < 3; ++lane) {
        right[lane] = world[lane];
        up[lane] = world[4 + lane];
        forward[lane] = world[8 + lane];
        translation[lane] = world[12 + lane];
    }
    return true;
}

// ---------------------------------------------------------------------------
// Milestone 2p: what the brain pre-pass and the drive's middle read off a unit
// ---------------------------------------------------------------------------

bool GameUnitsHost::world_bounds_box_00e188a8(float& min_x, float& max_x, float& min_z,
    float& max_z) const {
    min_x = 0.0f;
    max_x = 0.0f;
    min_z = 0.0f;
    max_z = 0.0f;
    // The world object at [00E188A8] is never built here, so there is no box.
    // Reporting its absence is what lets 0071C4F0's own rule run at its call
    // site without four comparisons against zeroes.
    return false;
}

bool GameUnitsHost::active_command_descriptor_0071eb60(std::size_t index,
    bsp::SceneCommandTarget& out, int& mode) const {
    return impl_->commands.active_command_descriptor_0071eb60(index, out, mode);
}

std::uint32_t GameUnitsHost::resolve_command_target_00521ea0(
    const bsp::SceneCommandTarget& target) const {
    return impl_->commands.resolve_command_target_00521ea0(target);
}

void GameUnitsHost::transform_by_unit_matrix_004142e0(std::size_t index, float in_x,
    float in_y, float in_z, float& out_x, float& out_y, float& out_z) const {
    out_x = in_x;
    out_y = in_y;
    out_z = in_z;
    if (index >= impl_->slots.size()) return;
    const std::array<float, 3> source{in_x, in_y, in_z};
    std::array<float, 3> result{};
    bsp::transform_point_004142e0(source, impl_->slots[index]->world, result);
    out_x = result[0];
    out_y = result[1];
    out_z = result[2];
}

int GameUnitsHost::unit_side_0054(std::size_t index) const {
    if (index >= impl_->slots.size()) return -1;
    // unit+54h is the party the scene record authored, which milestone 2h's
    // party_class_marks pass already carries on the row.
    return impl_->slots[index]->row.party;
}

float GameUnitsHost::director_target_hold_0040(std::size_t index) const {
    return impl_->commands.director_target_hold_0040(index);
}

int GameUnitsHost::director_leading_slot_categories_0071df83(std::size_t index, int* out,
    int max_out) const {
    return impl_->commands.director_leading_slot_categories_0071df83(index, out, max_out);
}

std::uint32_t GameUnitsHost::director_slot_command(std::size_t index, int slot_index) const {
    return impl_->commands.director_slot_command(index, slot_index);
}

const char* GameUnitsHost::command_name_of(std::uint32_t command_object) const {
    return impl_->commands.command_name_of(command_object);
}

const bsp::ShipAiAutoThrustSettings& GameUnitsHost::auto_thrust_settings(bool& loaded) const {
    loaded = impl_->auto_thrust_loaded;
    return impl_->auto_thrust;
}

bsp::ShipAiThrottleCeilingInputs GameUnitsHost::throttle_ceiling_inputs(
    std::size_t index) const {
    bsp::ShipAiThrottleCeilingInputs in{};
    if (index >= impl_->slots.size()) return in;
    const GameUnitSlot& slot = *impl_->slots[index];
    // 009EC97B, |unit+980h|: the order ring's published throttle. The absolute
    // value is taken inside the reconstruction, so the raw field goes in.
    in.live_throttle = slot.row.throttle;
    // 009EC99C / 009EC9A4, *(unit+73Ch)+28h and +24h: the commanded-speed pair
    // milestone 2m put on the navigator parameter block.
    const bsp::CruiseSpeedSetting speed = impl_->commands.commanded_speed(index);
    in.commanded_speed = speed.speed;
    in.commanded_speed_enabled = speed.enable >= 0.0f;
    // 009EC9AB, blk+3C4h: the cached reference speed, the same 0080FC30 value
    // 009E12BD divides by.
    in.reference_speed = bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
        bsp::kUnitReferenceSpeedUnscaled);
    return in;
}

float GameUnitsHost::unit_half_width_09cc(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // Historical API name:00811010/00811062 produce FULL width.
    return impl_->slots[index]->hull_extents.width_09cc;
}

float GameUnitsHost::unit_class_max_speed_0500(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_speed;
}

float GameUnitsHost::unit_class_max_rot_angle_04f8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_rot_angle;
}

float GameUnitsHost::unit_class_max_accel_0504(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.max_accel;
}

float GameUnitsHost::unit_class_turn_radius_0520(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // class+520h, the field 00828F20 derives once per class at 00828F66 from
    // MaxSpeed and MaxRotAngle. 0082E970 is its second reader, beside 0082E850.
    const bsp::ShipClassFields& fields = impl_->slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    return out.derived ? out.turn_radius_0520 : 0.0f;
}

float GameUnitsHost::unit_class_turn_circle_radius_0082e960(std::size_t index,
    float throttle) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    GameUnitSlot& slot = *host.slots[index];
    bsp::ShipAiNavBlockClassInputs in{};
    in.max_rot_angle_04f8 = slot.fields.max_rot_angle;
    in.max_speed_0500 = slot.fields.max_speed;
    in.turn_radius_0520 = unit_class_turn_radius_0520(index);
    UnitRudderBinding rudder(host, slot);
    const float radius = bsp::ship_class_turn_circle_radius_0082e960(in, throttle, rudder);
    host.done("ShipAiNavBlock::class_turn_circle_0082e960", 0x0082e960u);
    return radius;
}

float GameUnitsHost::unit_hull_length_09c8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->hull_extents.length_09c8;
}

float GameUnitsHost::unit_hull_mass_00b0(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.hull_mass;
}

int GameUnitsHost::unit_hull_material(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0;
    return static_cast<int>(impl_->slots[index]->hull_material);
}

float GameUnitsHost::unit_hull_linear_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.linear_damping;
}

float GameUnitsHost::unit_hull_angular_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.angular_damping;
}

void GameUnitsHost::raise_turn_assist_load_102c(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    // 009D4FB0 inlined: a compare and a store, so the raise is the whole body.
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.turn_assist_load_102c) slot.turn_assist_load_102c = value;
}

void GameUnitsHost::raise_secondary_load_1034(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.secondary_load_1034) slot.secondary_load_1034 = value;
}

float GameUnitsHost::turn_assist_load_102c(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->turn_assist_load_102c;
}

float GameUnitsHost::secondary_load_1034(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->secondary_load_1034;
}

void GameUnitsHost::unit_class_extents(std::size_t index, float& forward, float& right,
    float& up) const {
    forward = 0.0f;
    right = 0.0f;
    up = 0.0f;
    if (index >= impl_->slots.size()) return;
    const bsp::ShipMotionClass& motion_class = impl_->slots[index]->motion_class;
    forward = motion_class.hull_length;  // class+A0h
    up = motion_class.hull_height;       // class+A8h
    // class+A4h has no recovered Lua key, so it stays zero rather than being
    // guessed from either of the other two.
}

const GameUnitRow* GameUnitsHost::unit_row(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return nullptr;
    GameUnitSlot& slot = *impl_->slots[index];
    // Lifecycle providers can write the borrowed byte without going through
    // store_scene_node_flags. Refresh this diagnostic snapshot at its reader.
    slot.row.active = slot.state != nullptr && slot.state->active != 0;
    return &slot.row;
}

bool GameUnitsHost::controlled_bound() const noexcept { return impl_->controlled_bound; }

std::size_t GameUnitsHost::controlled_index() const noexcept { return impl_->controlled_index; }

const std::vector<GameUnitRow>& GameUnitsHost::units() const noexcept {
    // The rows live inside the slots; a flat copy is rebuilt on demand so the
    // caller sees one contiguous table.
    impl_->rows.clear();
    impl_->rows.reserve(impl_->slots.size());
    for (const std::unique_ptr<GameUnitSlot>& slot : impl_->slots) {
        slot->row.active = slot->state != nullptr && slot->state->active != 0;
        impl_->rows.push_back(slot->row);
    }
    return impl_->rows;
}

const GameUnitsSummary& GameUnitsHost::summary() const noexcept { return impl_->summary; }

void GameUnitsHost::log_controlled_trajectory(unsigned long long mission_frame) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    const GameUnitRow& row = host.slots[host.controlled_index]->row;
    host.log.notef("  controlled unit frame %-4llu t=%7.2f  x=%10.2f z=%10.2f  "
        "heading=%8.3f  fwd=%7.3f  throttle=%6.3f rudder=%6.3f  yaw=%8.5f",
        mission_frame, static_cast<double>(host.summary.simulated_seconds),
        static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
        static_cast<double>(row.heading_degrees), static_cast<double>(row.forward_speed),
        static_cast<double>(row.throttle), static_cast<double>(row.rudder),
        static_cast<double>(row.yaw_rate));
}

void GameUnitsHost::report() {
    Impl& host = *impl_;
    if (host.slots.empty()) return;
    if (host.gunnery != nullptr) host.gunnery->report();
    if (host.ai != nullptr) host.ai->report();
    host.log.notef("unit motion: %llu motion step(s) of %llu unit tick(s) over %.2f s of "
        "simulated time, %llu instance update(s) of 008255b0",
        host.summary.motion_steps, host.summary.motion_ticks,
        static_cast<double>(host.summary.simulated_seconds), host.summary.instance_updates);
    // Milestone 2k added the ordered pair each unit is running under, which is
    // unit+980h / unit+984h as the ring published them. Milestone 2l adds the
    // authored command and what its latch captured, and the two columns now
    // disagree on purpose: `cruise` writes the director's +243h / +244h / +248h
    // and never the ring, so a ship under an authored command alone shows a
    // zero ordered pair and does not move.
    host.log.notef("  %-20s %-12s %5s %5s %8s %8s %-8s %7s %9s %9s %8s %5s", "unit", "type",
        "party", "class", "throttle", "rudder", "command", "latched", "x", "z", "moved",
        "gate");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        char latched[16];
        if (row.command_latched) {
            std::snprintf(latched, sizeof(latched), "%s%.2f",
                row.latch_is_heading ? "h" : "r",
                static_cast<double>(row.latch_steer));
        } else {
            std::snprintf(latched, sizeof(latched), "-");
        }
        host.log.notef("  %-20s %-12s %5d %5d %8.3f %8.3f %-8s %7s %9.1f %9.1f %8.2f "
            "%5d%s", row.name.c_str(), row.type_symbol.c_str(), row.party, owned->class_id,
            static_cast<double>(row.throttle), static_cast<double>(row.ordered_rudder),
            row.command.empty() ? "-" : row.command.c_str(), latched,
            static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
            static_cast<double>(row.distance), row.command_applied ? 1 : 0,
            row.controlled ? "  <- controlled" : "");
    }
    // Milestone 2q: what 00822C20's property-bag arm seeded at creation.
    // `ratio` is the float32 008235CA stored into ring+148h and `axial` the
    // float32 008235EC handed to 0092D770; `expected` is ratio times the class
    // reference speed times the seconds simulated, the straight-run distance a
    // cruise ship that never changes throttle should cover.
    host.log.notef("  %-20s %10s %10s %8s %9s %10s %10s %10s", "unit", "StartSpeed",
        "reference", "ratio", "axial", "expected", "moved", "delta");
    std::size_t seeded = 0;
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        if (!row.start_speed_authored || row.start_speed == 0.0f) continue;
        ++seeded;
        const double expected = static_cast<double>(row.start_speed_ratio)
            * static_cast<double>(row.max_speed)
            * static_cast<double>(host.summary.simulated_seconds);
        host.log.notef("  %-20s %10.4f %10.4f %8.4f %9.4f %10.2f %10.2f %10.2f",
            row.name.c_str(), static_cast<double>(row.start_speed),
            static_cast<double>(row.max_speed),
            static_cast<double>(row.start_speed_ratio),
            static_cast<double>(row.start_speed_axial), expected,
            static_cast<double>(row.distance),
            static_cast<double>(row.distance) - expected);
    }
    host.log.notef("summary mission start speed seeds=%zu non_zero=%zu key=%s",
        host.summary.start_speed_seeds, seeded, bsp::kSceneUnitStartSpeedKey);
    // Milestone 2s: what the hydrodynamic callback did. `drift` is the angle
    // between the last step's displacement and the hull's bow, `peak` the worst
    // over the whole run, and `traj` the speed along that displacement against
    // `fwd`, the axial speed 0092D300 commands. A hull with no drag holds every
    // metre per second it acquires sideways, so traj runs far above fwd and the
    // drift runs toward ninety degrees; with the drag they converge.
    host.log.notef("  %-20s %5s %4s %5s %9s %9s %9s %10s %10s", "unit", "elem", "sub",
        "mat", "drift", "peak", "fwd", "traj", "force_y");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        if (row.hydro_calls == 0) continue;
        host.log.notef("  %-20s %5d %4d %5d %9.4f %9.4f %9.4f %10.4f %10.1f",
            row.name.c_str(), row.hydro_elements, row.hydro_submerged,
            static_cast<int>(owned->hull_material),
            static_cast<double>(row.drift_degrees),
            static_cast<double>(row.peak_drift_degrees),
            static_cast<double>(row.forward_speed),
            static_cast<double>(row.trajectory_speed),
            static_cast<double>(row.hydro_force[1]));
    }
    host.summary.world_class_6_list = host.world_lists.entries[6].count;
    {
        // The ids a unit's +130h override joins, and how long each list is.
        char ids[512];
        int written = 0;
        for (int id = 0; id < Impl::kWorldListCount; ++id) {
            if (host.world_lists.entries[id].count == 0) continue;
            written += std::snprintf(ids + written,
                sizeof(ids) - static_cast<std::size_t>(written), "%s%d=%zu",
                written == 0 ? "" : " ", id,
                static_cast<std::size_t>(host.world_lists.entries[id].count));
            if (written >= static_cast<int>(sizeof(ids)) - 1) break;
        }
        host.log.notef("summary mission world lists registrations=%llu pushes=%llu "
            "lists{%s} (actual leaf+130h through 00484540; id 6 is the one 009f1877 walks for "
            "ship AI neighbour candidates, and nothing walks it yet)",
            host.summary.world_registrations, host.summary.world_list_pushes, ids);
    }
    std::size_t nodes = 0;
    std::size_t invalid_lists = 0;
    for (const GameUnitWorldList& list : host.world_lists.entries) {
        const GameUnitWorldNode* previous = nullptr;
        const GameUnitWorldNode* node = list.head;
        std::uint32_t visited = 0;
        bool valid = true;
        for (; node != nullptr && visited < list.count; ++visited) {
            if (node->previous != previous || node->unit == nullptr
                || node->unit->world_parent_0030 != &host.world_lists) valid = false;
            previous = node;
            node = node->next;
        }
        nodes += visited;
        if (!valid || node != nullptr || visited != list.count || previous != list.tail)
            ++invalid_lists;
    }
    host.log.notef("world registration owner audit: unavailable=%llu nodes=%zu invalid_lists=%zu ships=%u planes=%u",
        host.summary.world_registration_unavailable, nodes, invalid_lists,
        host.world_lists.entries[6].count, host.world_lists.entries[15].count);
    host.log.notef("summary mission hydrodynamics calls=%llu element_steps=%llu "
        "submerged_steps=%llu add_force=%llu add_torque=%llu gravity_y=%.1f "
        "elements_per_hull=%d (009329c0 from 00937440 at 00937622, its last call)",
        host.summary.hydro_calls, host.summary.hydro_element_steps,
        host.summary.hydro_submerged_steps, host.summary.hydro_force_flushes,
        host.summary.hydro_torque_flushes,
        static_cast<double>(host.physics_world.gravity.y), kBuoyancyElementCount);
    std::size_t generic_units = 0;
    std::size_t generic_input_one = 0;
    for (const auto& slot : host.slots) {
        if (slot->motion_dispatch.entry != 0x00953cc0u) continue;
        ++generic_units;
        if (slot->generic_input_63c == 1.0f) ++generic_input_one;
    }
    host.log.notef("summary mission generic tick: calls=%llu unavailable=%llu "
        "units=%zu input_one=%zu (00953cc0; current roles0/4=8, flag634=0)",
        host.summary.generic_tick_calls, host.summary.generic_tick_unavailable,
        generic_units, generic_input_one);
    host.log.notef("summary mission plane step: steps=%llu free_flight=%llu "
        "ground_roll=%llu surface=%llu none=%llu (007ce040 arm selection)",
        host.summary.plane_steps, host.summary.plane_arm_free_flight,
        host.summary.plane_arm_ground_roll, host.summary.plane_arm_surface,
        host.summary.plane_arm_none);
    host.log.notef("summary mission plane motion: distance_moved=%.2f m "
        "pose_right_reference=%llu pose_collapsed=%llu "
        "pose_rotations=%llu heading_change=%.3f rad thinks=%llu commits=%llu yaw_plans=%llu",
        host.summary.plane_distance_moved,
        host.summary.plane_pose_right_reference,
        host.summary.plane_pose_collapsed,
        host.summary.plane_pose_rotations,
        host.summary.plane_heading_change,
        host.summary.pilot_thinks,
        host.summary.pilot_commits,
        host.summary.pilot_yaw_plans);
    {
        // What an ordered aircraft actually did about the order. The range is
        // measured at the first think that planned for it and at the last, so a
        // plane that flew straight past shows a small closure and one that
        // turned in shows a large one.
        std::size_t ordered = 0;
        double first_total = 0.0;
        double last_total = 0.0;
        double worst_closure = 0.0;
        for (const auto& slot : host.slots) {
            if (slot->attack_range_first < 0.0f) continue;
            ++ordered;
            first_total += slot->attack_range_first;
            last_total += slot->attack_range_last;
            const double closed = static_cast<double>(slot->attack_range_first) -
                                  static_cast<double>(slot->attack_range_last);
            if (ordered == 1 || closed < worst_closure) worst_closure = closed;
        }
        if (ordered > 0) {
            double err_first = 0.0;
            double err_last = 0.0;
            double pitch_last = 0.0;
            for (const auto& slot : host.slots) {
                if (slot->attack_range_first < 0.0f) continue;
                err_first += slot->attack_hdg_err_first;
                err_last += slot->attack_hdg_err_last;
                pitch_last += slot->attack_pitch_last;
            }
            const double n = static_cast<double>(ordered);
            // Per aircraft, because the mean hides the case that matters: a
            // plane pointing at its target and still losing ground because the
            // target is faster than it is.
            for (const auto& slot : host.slots) {
                if (slot->attack_range_first < 0.0f) continue;
                host.log.notef("  ordered %-12s range %8.1f -> %8.1f m  closed %8.1f m  "
                    "heading error %.3f -> %.3f rad",
                    slot->row.name.c_str(),
                    static_cast<double>(slot->attack_range_first),
                    static_cast<double>(slot->attack_range_last),
                    static_cast<double>(slot->attack_range_first - slot->attack_range_last),
                    static_cast<double>(slot->attack_hdg_err_first),
                    static_cast<double>(slot->attack_hdg_err_last));
            }
            host.log.notef("summary mission pilot attack: ordered=%zu "
                "range_first_mean=%.1f m range_last_mean=%.1f m closed_mean=%.1f m "
                "worst_closed=%.1f m | heading_error_first_mean=%.3f rad "
                "heading_error_last_mean=%.3f rad final_pitch_mean=%.3f rad",
                ordered, first_total / n, last_total / n,
                (first_total - last_total) / n, worst_closure,
                err_first / n, err_last / n, pitch_last / n);
        } else {
            host.log.notef("summary mission pilot attack: no unit was ever ordered "
                "at a target the yaw arm could plan for");
        }


        // The torpedo task census: per ordered aircraft the states its arm
        // 009D4850 entered with tick counts, and the release requests
        // 007BBBA0 it issued. docs/TORPEDO_TASK_ARM.md.
        {
            static const char* const kTorpedoStateNames[8] = {
                "moveto", "follow", "done", "attackrun",
                "goaway", "aim", "prepare", "?"};
            std::size_t tasked = 0;
            int releases_total = 0;
            int blocked_engaged = 0;
            int blocked_arm = 0;
            for (const auto& slot : host.slots) {
                if (!slot->torpedo_task_installed) continue;
                ++tasked;
                releases_total += slot->torpedo_releases;
                blocked_engaged += slot->torpedo_blocked_by_engaged;
                blocked_arm += slot->torpedo_blocked_by_arm;
                char states[192];
                int used = 0;
                states[0] = '\0';
                for (int k = 0; k < 7; ++k) {
                    if (slot->torpedo_state_ticks[k] == 0) continue;
                    const int n = std::snprintf(states + used,
                        sizeof(states) - static_cast<std::size_t>(used),
                        "%s%s=%d", used == 0 ? "" : " ",
                        kTorpedoStateNames[k], slot->torpedo_state_ticks[k]);
                    if (n <= 0) break;
                    used += n;
                    if (static_cast<std::size_t>(used) >= sizeof(states)) break;
                }
                if (used == 0) std::snprintf(states, sizeof(states), "(none)");
                host.log.notef("  torpedo %-12s arm_ticks=%d transitions=%d "
                    "states[%s] releases=%d",
                    slot->row.name.c_str(), slot->torpedo_arm_ticks,
                    slot->torpedo_transitions, states, slot->torpedo_releases);
                // The engaged pair over time: approach+8Ch (the engage
                // distance 009D4A70 sets) against approach+90h (the range
                // 009D3420 writes every tick it has a target), and the first
                // tick on which both went non-zero.
                if (slot->torpedo_first_engaged_tick >= 0) {
                    host.log.notef("  torpedo %-12s approach 009D3420: "
                        "ticks=%d no_target=%d replans=%d aim_ticks=%d | "
                        "engage 8Ch=%.1f range 90h first non-zero at tick %d, "
                        "min=%.1f last=%.1f",
                        slot->row.name.c_str(), slot->torpedo_approach_ticks,
                        slot->torpedo_approach_no_target_ticks,
                        slot->torpedo_approach_replans, slot->torpedo_aim_ticks,
                        static_cast<double>(slot->torpedo_engage_range_8c),
                        slot->torpedo_first_engaged_tick,
                        static_cast<double>(slot->torpedo_range_min),
                        static_cast<double>(slot->torpedo_range_last));
                    host.log.notef("  torpedo %-12s scan 009D37AE: runs=%d "
                        "clear_sectors=%d of 36 home_sector=%d turn_5c=%.4f rad",
                        slot->row.name.c_str(), slot->torpedo_scans_run,
                        slot->torpedo_clear_sectors_last,
                        slot->torpedo_home_sector_last,
                        static_cast<double>(slot->torpedo_turn_offset_last));
                    host.log.notef("  torpedo %-12s orders: lead=%d "
                        "issue_ticks=%d raised=%d peak_C58h=%d left=%d "
                        "arm_offers=%d blocked_0099af53=%d attack_mode_370=%d "
                        "raised_at_tick=%d drop_timer_98=%.1f",
                        slot->row.name.c_str(),
                        slot->torpedo_is_flight_lead ? 1 : 0,
                        slot->torpedo_orders_issue_ticks,
                        slot->torpedo_orders_issued,
                        slot->torpedo_peak_release_orders_c58,
                        slot->torpedo_release_orders_c58,
                        slot->torpedo_arm_offers,
                        slot->torpedo_arm_blocked_no_order_0099af53,
                        static_cast<int>(slot->torpedo_attack_mode_370),
                        slot->torpedo_attack_mode_raised_tick,
                        static_cast<double>(slot->torpedo_drop_timer));
                    host.log.notef("  torpedo %-12s issue stage 007CE9FD: "
                        "stage_ticks=%d guard_blocked=%d waiting=%d issues=%d "
                        "cleanups=%d first_issue_at_arm_tick=%d "
                        "requests_007BBBA0=%d C20h_left=%d C28h=%.3f",
                        slot->row.name.c_str(),
                        slot->torpedo_issue_stage_ticks,
                        slot->torpedo_issue_stage_guard_blocked,
                        slot->torpedo_issue_stage_waiting,
                        slot->torpedo_issue_stage_issues,
                        slot->torpedo_issue_stage_cleanups,
                        slot->torpedo_issue_first_issue_tick,
                        slot->torpedo_release_requests_007bbba0,
                        slot->torpedo_issue_requests_c20,
                        static_cast<double>(slot->torpedo_issue_interval_c28));
                    host.log.notef("  torpedo %-12s issue gate 007EEF40: "
                        "ctl+390h=%.4f ctl+374h=%.4f open=%d",
                        slot->row.name.c_str(),
                        static_cast<double>(slot->torpedo_issue_threshold_390),
                        static_cast<double>(slot->torpedo_armed_fraction_374),
                        slot->torpedo_issue_gate_open ? 1 : 0);
                    // 009D22FF-009D236E, the aim-complete test. clause 1 is
                    // 009D235A (F=34h > F=14h - ramp), clause 2 is 009D2368
                    // (F=18h > F=0Ch). docs/TORPEDO_AIM_TICK.md.
                    host.log.notef("  torpedo %-12s aim tick 009D15F0: "
                        "aim_complete_2Ch=%d first_true_at_aim_tick=%d "
                        "clause=%s | F34=%.2f F14=%.2f ramp=%.2f F18=%.4f "
                        "F0C=%.4f floor=%.1f | run_time_009D1360=%d "
                        "release_arm_009D2287=%d timer_009FA3A0=%.2f "
                        "timer_on=%d timer_fires=%d timer_blocked_007BB110=%d "
                        "first_fire_at_aim_tick=%d",
                        slot->row.name.c_str(),
                        slot->torpedo_aim_complete_2c ? 1 : 0,
                        slot->torpedo_aim_complete_tick,
                        slot->torpedo_aim_complete_clause == 3 ? "both"
                            : slot->torpedo_aim_complete_clause == 2 ? "turn"
                            : slot->torpedo_aim_complete_clause == 1 ? "range"
                            : "none",
                        static_cast<double>(slot->torpedo_aim_f34_threshold),
                        static_cast<double>(slot->torpedo_aim_f14_range),
                        static_cast<double>(slot->torpedo_aim_ramp),
                        static_cast<double>(slot->torpedo_aim_f18_turn),
                        static_cast<double>(slot->torpedo_aim_f0c_time),
                        static_cast<double>(slot->torpedo_aim_floor),
                        slot->torpedo_aim_run_time_updates,
                        slot->torpedo_aim_release_arms,
                        static_cast<double>(slot->torpedo_aim_timer),
                        slot->torpedo_aim_timer_enabled_0c ? 1 : 0,
                        slot->torpedo_aim_timer_fires,
                        slot->torpedo_aim_timer_blocked_007bb110,
                        slot->torpedo_aim_timer_first_fire_tick);
                    // ctl+370h. docs/TORPEDO_ATTACK_MODE.md: 009D3F69 and
                    // 009D40CB send an engaged task to prepare only at 0.
                    host.log.notef("  torpedo %-12s attack mode ctl+370h: "
                        "now=%d raised_at_tick=%d changes=%d | ticks "
                        "hold=%d attack=%d forced=%d | prepare_entries=%d "
                        "lowered_009A285E=%d message_007F0068=%d",
                        slot->row.name.c_str(),
                        static_cast<int>(slot->torpedo_attack_mode_370),
                        slot->torpedo_attack_mode_raised_tick,
                        slot->torpedo_mode_changes,
                        slot->torpedo_mode_ticks[0],
                        slot->torpedo_mode_ticks[1],
                        slot->torpedo_mode_ticks[2],
                        slot->torpedo_prepare_entries,
                        slot->torpedo_mode_lowered_009a285e,
                        slot->torpedo_mode_message_007f0068);
                    // Whether the one tick the mode is still 0 is the same tick
                    // the release-order queue unit+C58h is empty, which is what
                    // would decide the drop. 0099AF53 is the empty-queue arm.
                    host.log.notef("  torpedo %-12s prepare window: "
                        "first_prepare_at_arm_tick=%d "
                        "first_blocked_no_order_at_arm_tick=%d coincide=%s",
                        slot->row.name.c_str(),
                        slot->torpedo_prepare_first_tick,
                        slot->torpedo_blocked_no_order_first_tick,
                        (slot->torpedo_prepare_first_tick >= 0 &&
                         slot->torpedo_prepare_first_tick ==
                             slot->torpedo_blocked_no_order_first_tick)
                            ? "yes" : "no");
                    // 009D0D90 / 009D3150, the goaway break-off.
                    host.log.notef("  torpedo %-12s goaway 009D0D90/009D3150: "
                        "enters=%d break_off_24h=%.1f side_2Ch=%+.0f "
                        "range_peak_in_goaway=%.1f done_last=%d "
                        "(SafeDist 0042E740+438h=%.1f)",
                        slot->row.name.c_str(),
                        slot->torpedo_goaway_enters,
                        static_cast<double>(slot->torpedo_goaway_distance_24),
                        static_cast<double>(slot->torpedo_goaway_side_2c),
                        static_cast<double>(slot->torpedo_goaway_range_peak),
                        slot->torpedo_goaway_done_last ? 1 : 0,
                        static_cast<double>(
                            host.lua.plane_globals_loaded()
                                ? host.lua.plane_globals().pilot_torpedo_safe_dist
                                : 0.0f));
                } else {
                    host.log.notef("  torpedo %-12s approach 009D3420: "
                        "ticks=%d no_target=%d replans=%d aim_ticks=%d | "
                        "engage pair stays zero: 8Ch=%.1f 90h=%.1f",
                        slot->row.name.c_str(), slot->torpedo_approach_ticks,
                        slot->torpedo_approach_no_target_ticks,
                        slot->torpedo_approach_replans, slot->torpedo_aim_ticks,
                        static_cast<double>(slot->torpedo_engage_range_8c),
                        static_cast<double>(slot->torpedo_engage_limit_90));
                }
            }
            if (tasked > 0) {
                host.log.notef("summary mission torpedo task: aircraft=%zu "
                    "releases=%d blocked_engaged_009d3210=%d blocked_arm_009d49a0=%d",
                    tasked, releases_total, blocked_engaged, blocked_arm);
                if (releases_total == 0) {
                    // With 009D3420 reconstructed the engaged pair is live, so
                    // the gate is no longer the approach update itself. Name
                    // whichever of the two clauses of 009D3210 still refuses.
                    float worst_range = -1.0f;
                    float engage = 0.0f;
                    bool any_target = false;
                    for (const auto& slot : host.slots) {
                        if (!slot->torpedo_task_installed) continue;
                        engage = slot->torpedo_engage_range_8c;
                        if (slot->torpedo_range_min >= 0.0f &&
                            (worst_range < 0.0f ||
                             slot->torpedo_range_min < worst_range)) {
                            worst_range = slot->torpedo_range_min;
                        }
                        if (slot->torpedo_approach_no_target_ticks <
                            slot->torpedo_approach_ticks) {
                            any_target = true;
                        }
                    }
                    if (!any_target) {
                        host.log.notef("summary mission torpedo task: no release. "
                            "009D3420 took its 009D3506 early-out on every tick: "
                            "the approach has no target at approach+CCh "
                            "(task+4C4h), so 009D3210 returns 0 at 009D3223");
                    } else {
                        int arm_blocked = 0;
                        int arm_offers = 0;
                        int orders = 0;
                        for (const auto& s : host.slots) {
                            if (!s->torpedo_task_installed) continue;
                            arm_blocked += s->torpedo_arm_blocked_no_order_0099af53;
                            arm_offers += s->torpedo_arm_offers;
                            orders += s->torpedo_release_orders_c58;
                        }
                        if (arm_offers == 0) {
                            host.log.notef("summary mission torpedo task: no "
                                "release. 009D3420 and 009D3210 now pass and the "
                                "rule reaches prepare. The gate is unit+C58h at "
                                "0099AF53 in BSP_PilotBot_Tick: the queued "
                                "release-order count is %d on every one of %d "
                                "ticks, so the arming loop at 0099AF81 never "
                                "offers the task its vtable +24h (009D49A0), "
                                "prepare+98h stays at %.1f and 009D2720 never "
                                "reaches 007BBBA0. The raiser of unit+C58h is "
                                "the contract this packet leaves open",
                                orders, arm_blocked,
                                static_cast<double>(-1.0f));
                        } else if (worst_range >= 0.0f && worst_range >= engage) {
                            // 009D3210 admits 2.2 times the engage distance, so
                            // the rule leaves moveto, but the in-range latch
                            // itself needs the range INSIDE it.
                            host.log.notef("summary mission torpedo task: no "
                                "release. 009D3420, 009D3210, ctl+370h and "
                                "unit+C58h all pass and the rule reaches "
                                "attackrun. The gate is the in-range latch "
                                "approach+131h (task+529h) at 009D3774: it "
                                "closes only below approach+8Ch, and the closest "
                                "any aircraft came was %.1f against an engage "
                                "distance of %.1f. Without the latch 009D4030 "
                                "step 10 never promotes attackrun to aim, and "
                                "009D49A0 only arms prepare+98h in prepare, so "
                                "the %d offers at 0099AF9B spend nothing",
                                static_cast<double>(worst_range),
                                static_cast<double>(engage), arm_offers);
                        } else {
                            int aim_total = 0;
                            for (const auto& s : host.slots) {
                                if (!s->torpedo_task_installed) continue;
                                aim_total += s->torpedo_aim_ticks;
                            }
                            int aim_done = 0;
                            for (const auto& s2 : host.slots) {
                                if (!s2->torpedo_task_installed) continue;
                                if (s2->torpedo_aim_complete_2c) ++aim_done;
                            }
                            if (aim_done == 0) {
                                host.log.notef("summary mission torpedo task: no "
                                    "release. The range reached the engage "
                                    "distance (closest %.1f against 8Ch=%.1f), "
                                    "%d offers were made at 0099AF9B and the "
                                    "rule reached aim for %d ticks, but no "
                                    "aircraft ever set the aim-complete byte "
                                    "state+2Ch at 009D236E",
                                    static_cast<double>(worst_range),
                                    static_cast<double>(engage), arm_offers,
                                    aim_total);
                            } else {
                                int goaway_done = 0;
                                float peak = -1.0f;
                                float breakoff = 0.0f;
                                for (const auto& s3 : host.slots) {
                                    if (!s3->torpedo_task_installed) continue;
                                    if (s3->torpedo_goaway_done_last) ++goaway_done;
                                    if (s3->torpedo_goaway_range_peak > peak) {
                                        peak = s3->torpedo_goaway_range_peak;
                                    }
                                    breakoff = s3->torpedo_goaway_distance_24;
                                }
                                int hold_ticks = 0;
                                int prep = 0;
                                for (const auto& s4 : host.slots) {
                                    if (!s4->torpedo_task_installed) continue;
                                    hold_ticks += s4->torpedo_mode_ticks[0];
                                    prep += s4->torpedo_prepare_entries;
                                }
                                host.log.notef("summary mission torpedo task: no "
                                    "release. 009D15F0 writes the aim-complete "
                                    "byte (%d of %zu) and 009D3150 is computed "
                                    "from goaway+24h=%.1f (true for %d, furthest "
                                    "%.1f m). The gate is ctl+370h, the attack "
                                    "mode: 009D3F69 and 009D40CB send an engaged "
                                    "task to prepare only while it is 0, and "
                                    "009D49A0 arms prepare+98h only in prepare. "
                                    "0099B774 raised it to 1 and it stayed there "
                                    "for %d of %d arm ticks; prepare was entered "
                                    "%d times. Neither route back to 0 is one a "
                                    "torpedo task runs: 009A285E needs the "
                                    "closetoship task's countdown after a Lua "
                                    "PilotStopCloseToShip, and 007F0068 needs "
                                    "the BCh message on the control block. "
                                    "closest approach %.1f against 8Ch=%.1f, %d "
                                    "offers, aim ran %d ticks",
                                    aim_done, tasked,
                                    static_cast<double>(breakoff), goaway_done,
                                    static_cast<double>(peak),
                                    arm_offers - hold_ticks, arm_offers, prep,
                                    static_cast<double>(worst_range),
                                    static_cast<double>(engage), arm_offers,
                                    aim_total);
                            }
                        }
                    }
                }
            } else {
                host.log.notef("summary mission torpedo task: no ordered aircraft "
                    "carries torpedo ordnance (kind 2Bh), so 0099A170 builds no "
                    "kind Eh task");
            }
        }
    }
    host.commands.report();
}

}  // namespace bsp::game
