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
#include "bsp/plane_death_modes.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/plane_advance_pose.hpp"
#include "bsp/plane_angular_velocity.hpp"
#include "bsp/plane_control_rate.hpp"
#include "bsp/pilot_plan_slots.hpp"
#include "bsp/dogfight_task.hpp"
#include "bsp/near_field_probe.hpp"
#include "bsp/plane_attitude_angles.hpp"
#include "bsp/plane_ai_control.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/dive_bomb_aimdive_tail.hpp"
#include "bsp/dive_bomb_task.hpp"
#include "bsp/dive_bomb_goaway_turn.hpp"
#include "bsp/move_to_glide.hpp"
#include "bsp/torpedo_aim_tick.hpp"
#include "bsp/torpedo_goaway_tick.hpp"
#include "bsp/plane_fly_to_solver.hpp"
#include "bsp/torpedo_approach_update.hpp"
#include "bsp/torpedo_first_release.hpp"
#include "bsp/torpedo_issue_timing.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/plane_follow_law.hpp"
#include "bsp/plane_formation.hpp"
#include "bsp/torpedo_release_orders.hpp"
#include "bsp/torpedo_task_arm.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/game_hosts_script_orders.hpp"
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
#include "bsp/ocean_wave_field.hpp"
#include "bsp/ship_hydro_forces.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_ai_wake_trail.hpp"
#include "bsp/ship_ai_path_corridor.hpp"  // ShipAiUnitGroupMember, the 34h record
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
#include "bsp/approach_target_ref.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/world_ocean.hpp"
#include "bsp/world_construct.hpp"
#include "bsp/plane_class_fields.hpp"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
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

// The stationary prop, which has no vehicle-class descriptor and so no creator
// to key on. 004F0FB0 takes it at 004F0FFE when the scene property `Stationary`
// is set: 00748C40 allocates 1ACh bytes and 00748A40 constructs them, storing
// 00CFF678 at this+0 and 00CFF65C at this+10h (00748A64 and 00748A6A). Those are
// the same two slots the keyed rows in src/native_unit_observer_endpoint.cpp
// take, which 00745940 shows for LandFort at 0074597D and 00745983 against its
// row {00747000, 00CFF3F8, 00CFF3E0}. The prop is 1ACh bytes with no slot at
// 310h, so unlike a fort it carries no tick vtable and takes no motion dispatch.
//
// This lives here rather than beside the keyed rows because that file and its
// header are Codex-lineage and we do not edit them.
// docs/SCENE_STATIONARY_UNITS.md.
void publish_stationary_prop_observer_tables_00748a40(
    bsp::NativeUnitObserverPrefixStorage& unit) noexcept {
    unit.observed_00.native_vtable_00 = 0x00cff678u;
    unit.callback_10.native_vtable_00 = 0x00cff65cu;
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
    // The unit took 004F0FB0's stationary arm: no vehicle-class descriptor, so
    // no creator to key the observer tables on, and no motion dispatch either
    // because the 1ACh prop has no slot at 310h.
    // docs/SCENE_STATIONARY_UNITS.md.
    bool stationary_prop{false};
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
    // The dive-bomb bot task (kind 8) this ordered aircraft runs, when its
    // class carries general bomb ordnance (kind 2Ah, 007ED7E0 -> 007B9320) and
    // the command arm at 007EE9C5 chose class 00E08F20. docs/DIVE_BOMB_TASK.md.
    // The class 007EEC50 chose, stored by the PilotSetTarget path. 0 means no
    // attack order, which is what every aircraft in IJN01 and USN01 has.
    unsigned int attack_command_class{0};
    bool dive_bomb_task_installed{false};
    bsp::DiveBombState dive_bomb_state{bsp::DiveBombState::kNone};
    // Packet cc9_dogfight_task: the dogfight task (kind 2), installed when
    // 007EEC50 chose class 00E08F58. docs/DOGFIGHT_TASK.md.
    bool dogfight_task_installed{false};
    bsp::DogfightState dogfight_state{bsp::DogfightState::kNone};
    int df_state_ticks[bsp::kDogfightStateCount]{};
    int df_transitions{0};
    int df_within_attack_dist_ticks{0};
    float df_min_target_range{-1.0f};
    // Packet cc9_dogfight_engaged: the approach object's dogfight fields
    // (task+3F8h + N) and the engaged states'. docs/DOGFIGHT_ENGAGED.md.
    bsp::DogfightPilotRow df_row{};         // approach+14h, SPNormal substitute
    bool df_target_squadron{false};         // approach+CCh != 0 (task+4C4h)
    std::size_t df_target_plus_one{0};      // approach+B4h
    bool df_latch_d0{false};                // approach+D0h (task+4C8h)
    float df_distance_d4{9999.0f};          // approach+D4h
    float df_horizontal_d8{0.0f};           // approach+D8h
    float df_reselect_timer_e4{0.0f};       // approach+E4h
    float df_local_ec[3]{0.0f, 0.0f, 0.0f}; // approach+ECh..F4h
    float df_tan_f8[2]{0.0f, 0.0f};         // approach+F8h/FCh
    float df_aim[3]{0.0f, 0.0f, 0.0f};      // approach+48h (substitute: target origin)
    bsp::DogfightAimState df_aim_state{};   // task+67Ch + 18h..25h
    float df_maneuver_range_34{0.0f};       // task+6D8h
    float df_avoid_timer{0.0f};             // task+720h / +748h
    float df_avoid_sign{1.0f};
    int df_latch_sets{0};
    int df_latched_ticks{0};
    int df_reselects{0};
    int df_target_changes{0};
    int df_gun_window_ticks{0};
    bsp::DogfightGunState df_gun{};         // task+314h, packet cc9_dogfight_gun
    // unit+C50h (packet cc9_plane_gunfire): the enemy-aircraft list +50h, the
    // refresh clock +7Ch and the finder clock +84h (U(0,P)+P at midpoints),
    // and the finder's cached choice +B0h.
    std::vector<std::size_t> nb_enemy_50;
    float nb_clock_7c{4.5f};
    float nb_clock_84{3.0f};
    std::size_t nb_found_plus_one{0};
    int nb_refreshes{0};
    int nb_scans{0};
    // Packet cc9_near_field_probe census.
    int nf_calls{0};
    // Packet cc9_plane_gun_pass.
    float df_dc{0.0f};                 // approach+DCh
    float df_e0{1.0f};                 // approach+E0h
    int df_early_edges{0};
    int df_early_asks{0};
    bool plane_gun_fire_bc9{false};    // unit+BC9h, the latched gunFire
    int pg_trigger_ticks{0};
    int pg_trigger_rises{0};
    int df_head_on_ticks{0};
    int pc_air_brake_overrides{0};
    // Packet cc9_dogfight_moveto.
    float df_station_world[3]{0.0f, 0.0f, 0.0f};   // follow state +30h..38h
    bool df_station_valid{false};
    float df_leader_heading{0.0f};                 // followed unit vtable[50h]
    bool plane_pilot_fires_c24{true};              // unit+C24h PilotFires
    int df_moveto_speed_commands{0};
    float df_moveto_speed_min{1.0e9f};
    float df_moveto_speed_max{0.0f};
    float df_head_on_throttle_min{1.0f};
    int nf_hits{0};
    int nf_attackrun_weaves{0};
    float nf_attackrun_max_offset{0.0f};
    int nf_flyover_slot_writes{0};
    int nf_goaway_hits{0};
    float nf_flyover_slot_min{0.0f};
    float nf_flyover_slot_max{0.0f};
    int dive_bomb_state_ticks[10]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dive_bomb_arm_ticks{0};
    int dive_bomb_transitions{0};
    int dive_bomb_releases{0};
    int dive_bomb_rounds_pending{0};   // task+424h
    int dive_bomb_rounds_remaining{0};  // 007C1DB0(unit)
    // The approach fields 009C7A80 and the seed 009C3EA0 produce, mirrored so
    // the arm, the two release rules and the census read one set.
    float db_dive_alt_a8{0.0f};      // approach+A8h, the release floor
    // Packet cc9_difficulty. pilot_skill_index is unit+390h and the pilot bot's
    // bot+34h (0095CCCC's default 1; 009565A0 / 007B8AE0 set it). The approach
    // captures it once, at 009F9D22, as its PilotBot row: db_skill_row_14 is
    // approach+14h held as the row index rather than the row's address.
    int pilot_skill_index{1};
    int db_skill_row_14{1};
    float db_begin_alt_ac{0.0f};     // approach+ACh = ctl+398h, BeginAltRange/1
    float db_alt_span_b0{0.0f};      // approach+B0h = BeginAltRange/2 - /1
    float db_attack_dist_b4{0.0f};   // approach+B4h, the moveto speed argument
    float db_in_range_b8{0.0f};      // approach+B8h == task+4B0h
    float db_planar_bc{0.0f};        // approach+BCh, the planar range
    // 009C8B14-009C8B3B: what 009C8A90's range arm actually measures, which is
    // NOT approach+BCh. The far point is the approach's own aim point, fetched
    // through vtable slot 0 (00D20E10 -> 009C40A0, which copies +4Ch/+50h/+54h),
    // the near point is the unit, and 0042B2F0 takes the full three-vector
    // length. docs/BOMBER_AFTER_TASK.md 10.10, docs/DIVE_BOMB_APPROACH.md 7.
    // Packet cc8_hull_aim_point. The target-reference sub-object every attack
    // approach embeds (approach+30h dive bomb, approach+B4h torpedo), bound in
    // include/bsp/approach_target_ref.hpp. One per ordered target: the body
    // frame offset is drawn once, because slot +104h is 0042BB20 on every class
    // that samples a hull and so never refuses the point it already has.
    bsp::ApproachTargetRefState hull_aim_ref{};
    std::size_t hull_aim_target_plus_one{0};
    std::uint32_t hull_aim_seed{0};
    // Packet cc9_hull_axis: the target the in-range print last fired for, so
    // it fires once per (attacker, target). Diagnostic only, behind the switch.
    std::size_t hull_aim_range_printed_plus_one{0};
    float db_aim_point_3d{0.0f};
    float db_bearing_c0{0.0f};       // approach+C0h
    float db_aim_point_height_50{0.0f};   // approach+50h
    float db_release_range_d4{0.0f};  // approach+D4h
    float db_lead_high_5c{0.0f};     // (approach+14h)->+5Ch
    float db_gain_high_60{0.0f};     // (approach+14h)->+60h
    bool db_in_range_d0{false};      // approach+D0h == task+4C8h
    bool db_has_bomb_d1{false};      // approach+D1h == task+4C9h
    // squadron+370h, the attack mode. Its faithful owner is the squadron
    // (`plane+9D4h`), not the unit: 0099B740 writes it through `bot+2FCh` and
    // 009C83F8 reads it through `task+404h`, which is the same object. This
    // host keeps a per-slot copy that the flight leader broadcasts to its
    // squadron's members every think, so every member holds the value the
    // leader wrote. Labelled: the copy is a hole in OWNERSHIP only, the value
    // is the leader's. Seeded kForced because the scene's attack order reaches
    // 008A4C41 -> 007ED430(2); the leader's first think then sets it to 1.
    bsp::PilotAttackMode db_attack_mode_370{bsp::PilotAttackMode::kForced};
    // sqn+3D0h[0] == this unit, the test at 0099B757. Only the leader's think
    // writes the mode.
    bool db_is_flight_lead{false};
    bool db_flight_lead_resolved{false};
    int db_mode_ticks[3]{0, 0, 0};
    int db_mode_changes{0};
    int db_mode_first_set_tick{-1};
    // Cached inputs of the two predicates, sampled where they are computed so
    // the exit log can print what actually decided the edge.
    bool db_dbg_engaged{false};
    bool db_dbg_break_off{false};
    float db_dbg_break_off_distance{0.0f};
    float db_dbg_break_off_threshold{0.0f};
    // Where a spent bomber ends up, and how low it gets afterwards.
    int db_spent_exit_logs{0};
    int db_post_done_ticks{0};
    float db_post_done_min_alt{-1.0f};
    int db_approach_returns{0};
    int db_approach_return_tick{-1};
    // The per-state fields the two aim states and flyabove carry.
    float db_aim_rearm_1c{0.0f};     // aimdive/aimglide state+1Ch
    float db_glide_travel_20{0.0f};  // aimglide state+20h
    bool db_aim_alive_19{false};     // aimdive state+19h
    bool db_aim_pull_out_18{false};  // aimdive state+18h
    bool db_flyabove_ready_19{false};
    bool db_flyabove_can_dive_18{false};
    bool db_flyabove_leave_1a{false};
    float db_turn_roll_18{0.0f};     // turndown state+18h, 009C7800's output
    // The attackrun state's own fields, 009C4220.
    float db_attackrun_timer_1c{0.0f};
    float db_attackrun_period_18{1.0f};
    float db_attackrun_offset_20{0.0f};
    int db_attackrun_ticks{0};
    int db_attackrun_rerolls{0};
    int db_latch_closed_tick{-1};
    float db_attackrun_heading_last{0.0f};
    float db_attackrun_throttle_last{0.0f};
    float db_attackrun_alt_last{0.0f};
    float db_range_at_second[16]{};
    int db_range_samples{0};
    bool db_turndown_latch_1c{false};  // turndown state+1Ch, 009C465D
    int db_turndown_ticks{0};
    int db_turndown_roll_writes{0};
    int db_turndown_pitch_writes{0};
    int db_turndown_latched_tick{-1};
    float db_turndown_bank_last{0.0f};
    float db_turndown_roll_last{0.0f};
    float db_turndown_pitch_last{0.0f};
    // The two pose angles 009C7EA0 reads, sampled over the turndown: the gate
    // out of it needs pose+C64h under the -1.3 at 00D1F98C, or under -1.0 with
    // |pose+C68h| past the 135 degrees at 00D20E80.
    float db_turndown_pose_c64_last{0.0f};
    float db_turndown_pose_c64_min{0.0f};
    // Census.
    float db_dive_entry_alt{-1.0f};
    float db_dive_entry_pitch{0.0f};
    float db_release_alt{-1.0f};
    float db_release_speed{-1.0f};
    float db_release_range{-1.0f};
    float db_aim_error_last{0.0f};
    // 009C6493's height and the span 009C65FD draws from it, the one pair all
    // three flyabove flags key on.
    float db_flyabove_height{0.0f};
    float db_flyabove_span{0.0f};
    // 009C6385-009C63E6: the range and the bearing to the aim point predicted
    // three seconds ahead, which is what the fly-above's flags and its heading
    // arm all read. Packet cc8_dive_heading.
    float db_flyabove_lead_range{0.0f};
    float db_flyabove_lead_bearing{0.0f};
    // The aimdive trace. Endpoint values cannot tell a dive that never pointed
    // at the target from one that pointed and was too slow, so the aim states
    // sample their own geometry the way the run-in samples its range.
    // 009C62B0's and 009C5180's heading arms.
    int db_flyabove_tick_ticks{0};
    int db_aimglide_tick_ticks{0};
    // 009C18C0 and 009C1FD0, the two approach states the arm's virtual tick
    // dispatch at 009C884C reaches and this host never ran.
    // docs/DIVE_BOMB_APPROACH.md.
    int db_moveto_tick_ticks{0};
    int db_follow_tick_ticks{0};
    // 009C7240 and 009C7270, the dive-bomb done/prepare state, packet
    // cc8_done_state. `plan_mode_26c` is the value 009C1FE2 stamps into the
    // pilot command block at the head of every follow tick. Nothing in this
    // host reads it, and docs/BOMBER_AFTER_TASK.md section 6c says why nothing
    // in the IMAGE acts on it for a flight leader either: the only reader,
    // 0099D300's gate A, needs three further conditions that a leader's follow
    // tick never produces. It is carried to be measured, not to be acted on.
    int db_done_entries{0};
    int db_done_tick_ticks{0};
    int db_done_placed_ticks{0};
    float db_done_entry_alt{0.0f};
    float db_done_last_alt{0.0f};
    float db_done_last_hdg{0.0f};
    int plan_mode_26c{0};
    // 009FBA50's terms, sampled through the run-in.
    float db_cruise_span[8]{};
    float db_cruise_gain[8]{};
    float db_cruise_scale[8]{};
    float db_cruise_base[8]{};
    float db_cruise_clamped[8]{};
    int db_cruise_samples{0};
    int db_goaway_tick_ticks{0};
    float db_goaway_pitch_last{0.0f};
    // Packet cc8_dive_goaway: the climb census. `pitch_last` on its own cannot
    // tell a state that never commanded a climb from one whose command was
    // eaten later, so the altitude the aircraft actually reaches goes beside it,
    // and so do the two operands of 009C7F00's second arm - the ceiling
    // min(ctl+398h, approach+ACh + approach+50h) and the deficit ceiling - Y,
    // which 009C4B36 is about to be shown to feed to the climb curve as well.
    float db_goaway_pitch_max{0.0f};
    float db_goaway_alt_first{-1.0f};
    float db_goaway_alt_last{0.0f};
    float db_goaway_alt_max{-1.0f};
    float db_goaway_ceiling_last{0.0f};
    float db_goaway_deficit_last{0.0f};
    int db_goaway_complete_ticks{0};
    // The goaway state's OWN +20h. 009C7F00's completion rule reads the goaway
    // state, not the aimglide's, and conflating them made goaway finish on its
    // first tick.
    float db_goaway_travel_20{0.0f};
    // Packet cc9_goaway_turn: the goaway state's own +18h..+2Ch, persisting
    // across entries as the native object does (the enter 009C4950 rewrites
    // +18h/+20h/+24h only). docs/DIVE_BOMB_GOAWAY_TURN.md.
    bsp::DiveBombGoAwayTurnState db_goaway_turn{};
    int db_goaway_enters{0};
    int db_goaway_flag0_ticks{0};       // ticks that reached 009C4CBA
    int db_goaway_countdown_ticks{0};   // 009C4A86 ran
    int db_goaway_rerolls{0};           // 009C4CF1 arm ran
    int db_goaway_first_reroll_tick{-1};
    int db_goaway_bank_ticks{0};        // 009C4DAF arm
    int db_goaway_heading_ticks{0};     // 009C4E05 arm
    int db_goaway_first_heading_tick{-1};
    int db_goaway_side_writes{0};       // 009FD570 changed +18h
    float db_goaway_bank_min{0.0f};
    float db_goaway_bank_max{0.0f};
    float db_goaway_heading_last{0.0f};
    float db_goaway_heading_err_max{0.0f};  // |heading - own heading|, heading arm
    int db_flyabove_heading_writes{0};
    float db_flyabove_heading_last{0.0f};
    // Packet cc8_dive_flyover: flyabove+1Ch, the roll-in LATCH of 009C6919, and
    // the bank arm around it. The latch is per-state - 009C629E clears it on the
    // fly-over's enter - and once set it both skips the bank evaluation
    // (009C6865) and suppresses the heading write (009C6DDA).
    // Packet cc8_dive_flyover: the aimglide's pull-out latch, state+18h =
    // whole-object +76Ch. 009C57FF sets it, 009C4F0C clears it on the enter,
    // and 009C86B2/009C86BF read it to send the state to goaway - which is the
    // only way an aircraft that aborted its dive gets a second attack run.
    bool db_aimglide_pull_out_76c{false};
    float db_glide_bearing_abs_18{0.0f};  // [ESP+18h], the arm's input
    float db_glide_bearing_max{0.0f};     // the census arm A needs either way
    int db_aimglide_pull_outs{0};
    bool db_flyabove_bank_latch_1c{false};
    float db_flyabove_dead_band_t{0.0f};
    float db_flyabove_along_track{0.0f};
    float db_flyabove_cross_track{0.0f};
    int db_flyabove_bl_ticks{0};        // ticks with BL set at 009C67BF
    int db_flyabove_latched_ticks{0};   // ticks the latch suppressed the heading
    int db_flyabove_latch_tick{-1};     // the arm tick the latch first closed on
    // Packet cc8_dive_entry. Where the dive-entry altitude comes from is a
    // question about the altitude at each HAND-OVER, and no existing census
    // carries it: db_dive_entry_alt is the aimdive entry only, and the geo
    // trace prints one slot and starts at the turndown. One record per state
    // change, for every dive bomber.
    struct DbTransition {
        int tick{0};
        signed char from{-1};
        signed char to{-1};
        float alt{0.0f};
        float range{0.0f};
        float cmd_alt{0.0f};
        // Packet cc8_dive_heading: the three flyabove numbers that decide the
        // race, latched at the hand-over so the arm that fired can be named
        // instead of inferred. `span` is 009C65FD's, `f18` is 009C680E's
        // can-dive and `f19` is 009C67B0's roll-in permission.
        float span{0.0f};
        float b_height{0.0f};
        signed char f18{-1};
        signed char f19{-1};
    };
    static constexpr int kDbTransitions = 12;
    DbTransition db_transitions[kDbTransitions]{};
    int db_transition_count{0};
    // 009C6E10-009C6F91, the flyabove's altitude arm.
    int db_fa_alt_calls{0};
    int db_fa_level_ticks{0};
    float db_fa_limit_c{0.0f};
    float db_fa_band{0.0f};
    float db_fa_target_last{0.0f};
    float db_fa_ref_last{0.0f};
    float db_fa_err_last{0.0f};
    float db_fa_err_first{0.0f};
    float db_fa_pitch_last{0.0f};
    // The geometry when the turndown starts, which is what decides where the
    // dive begins.
    float db_turndown_entry_range{-1.0f};
    float db_turndown_entry_bearing{0.0f};
    float db_aimdive_entry_range{-1.0f};
    float db_aimdive_entry_bearing{0.0f};
    float db_aimdive_min_range{-1.0f};
    float db_aim_trace_range[12]{};
    float db_aim_trace_error[12]{};
    float db_aim_trace_bank[12]{};
    float db_aim_trace_bearing[12]{};
    int db_aim_trace_samples{0};
    int db_aim_state_ticks{0};
    // 009C58D0's steering census.
    int db_aimdive_steer_ticks{0};
    float db_aimdive_pitch_last{0.0f};
    float db_aimdive_roll_last{0.0f};
    float db_aimdive_bearing_last{0.0f};
    float db_aim_heading_last{0.0f};   // 009C4F80's result, the aim heading
    // The closest the aim error came to the 25.0 m gate at 00CE3880 while an aim
    // state owned the tick, with the geometry at that sample. -1 means never.
    float db_aim_error_abs_min{-1.0f};
    float db_aim_error_min_range{-1.0f};
    float db_aim_error_min_alt{-1.0f};
    // Packet cc8_dive_geometry: a per-tick window over the split-S and the
    // start of the dive, to settle which manoeuvre the host flies against the
    // one 009C44F0 commands. Additive; nothing here feeds a command.
    static constexpr int kDbGeoSamples = 140;
    struct DbGeoSample {
        int tick;
        int state;
        float pitch_c64;
        float bank_c68;
        float heading_c6c;
        float altitude;
        float range;
        float bearing_err;
        float aim_heading;
        float roll_input;
        float roll_cmd;
        float pitch_cmd;
        int pitch_mode_2d0;
        int heading_mode_2cc;
    };
    DbGeoSample db_geo[kDbGeoSamples]{};
    int db_geo_samples{0};
    int db_blocked_no_latch{0};      // ticks with approach+D0h clear
    int db_blocked_no_bomb{0};       // ticks with approach+D1h clear
    // Packet cc8_dive_release, item 1: which of 009C8634-009C8682's three arms
    // ends the aimdive, with the three flags as the transition read them (that
    // is, BEFORE the same arm's tick can rewrite them). Additive census only.
    int db_aimdive_entries{0};
    int db_aimdive_exits{0};
    int db_aimdive_exit_alive{0};    // 009C8645 taken: +19h was 0
    int db_aimdive_exit_nobomb{0};   // 009C8668 taken: approach+D1h was 0
    int db_aimdive_exit_pullout{0};  // 009C8671 not taken: +18h was set
    int db_aimdive_exit_other{0};
    int db_aimdive_run_ticks{0};     // ticks in the current aimdive run
    int db_aimdive_first_run{-1};    // ticks the first aimdive run lasted
    int db_aimdive_last_run{-1};
    // approach+D8h/+DCh/+E0h and the two quantities the aimdive tick takes from
    // them: the second sqrt at 009C5A40 ([ESP+5Ch]) and the bearing 009C5AF1
    // leaves in [ESP+18h].
    float db_run_in_origin[3]{};
    float db_impact_fall_time{0.0f};
    float db_impact_planar_5c{-1.0f};
    float db_impact_bearing_18{0.0f};
    // Packet cc8_dive_glide. The aimglide tick builds a THIRD planar vector
    // from the same point, `impactPoint - unit` at 009C5207-009C522E, and its
    // magnitude at 009C52C7-009C5303 is the [ESP+14h] the 120 m gate and the
    // lead read. The aimdive never needs it, which is why no slot carried it.
    float db_impact_throw_14{0.0f};
    // The aimglide release census, packet cc8_dive_glide: how far down the
    // 009C5689-009C5755 chain each call got, and the lead when it was computed.
    int db_glide_calls{0};
    int db_glide_gate_reached[7]{};
    float db_glide_lead_last{0.0f};
    float db_glide_ratio_last{0.0f};  // aimglide [ESP+24h], cc9_dive_throttle
    float db_glide_pitch_last{0.0f};  // aimglide cmd+2BCh, cc9_aimglide_pitch
    float df_moveto_cmd_alt_last{0.0f};  // cc9_dive_modes, generic moveto
    int db_glide_yaw_ticks{0};        // aimglide yaw arm ticks, cc9_aimglide_pitch
    float db_glide_lead_min{0.0f};      // the most negative lead seen
    float db_glide_bearing_min{-1.0f};
    int db_glide_releases{0};
    int db_glide_rounds{0};
    // Packet cc8_dive_glide: the bomb side of the release, which until now had
    // no fields of its own. The summary's `bombs_spawned` column was printing
    // torpedo_drops_spawned - a different field with a different producer.
    int db_bay_requests_accepted{0};
    int db_bay_requests_refused{0};
    int db_bombs_spawned{0};
    // The 009C5B01-009C5B48 abort's own operands the first time it fires.
    int db_abort_fires{0};
    int db_abort_first_tick{-1};
    float db_abort_d4{0.0f};
    float db_abort_h14{0.0f};
    float db_abort_range{0.0f};
    float db_abort_pitch{0.0f};
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
    // cc8_torpedo_retire item 1. Every input of 009D4C10's binding, latched on
    // the FIRST tick it returns true, so the arm that retires the bomber is read
    // from a run instead of inferred. Additive: nothing here feeds behaviour.
    int torpedo_breakoff_first_tick{-1};      // slot arm tick, or -1
    int torpedo_breakoff_true_ticks{0};
    int torpedo_breakoff_arm{0};              // 2 = target arm, 5 = range arm
    int torpedo_breakoff_state{0};
    int torpedo_breakoff_target_plus_one{0};
    int torpedo_breakoff_approach_ticks{0};
    int torpedo_breakoff_no_target_ticks{0};
    bool torpedo_breakoff_has_target{false};
    bool torpedo_breakoff_target_marked{false};
    bool torpedo_breakoff_in_attack_state{false};
    bool torpedo_breakoff_attack_flag_52a{false};
    bool torpedo_breakoff_has_ordnance_132{false};
    float torpedo_breakoff_range_90{0.0f};    // what the binding fed as distance
    float torpedo_breakoff_limit_90{0.0f};    // task+488h's twin, the image's cell
    float torpedo_breakoff_prev_range{0.0f};  // range_90 one transition tick back
    float torpedo_breakoff_safe_dist{0.0f};   // safe_distance_438() on that tick
    float torpedo_prev_range_90{0.0f};
    // cc8_torpedo_retire item 5: the crossing angle at ATTACKRUN ENTRY, to be
    // set against the 5 to 12 degrees the closest-approach census measures at
    // the end of the swim. The target's number is the hull POSE heading,
    // atan2(pose_row2.x, pose_row2.z), which is the same quantity the gunnery
    // census differences the round's track against. It is NOT the ship-ai
    // step heading: docs/TORPEDO_AFTER_THE_DROP.md section 13 measures that
    // control-block field 0.7 to 0.9 rad away from the hull's pose.
    float torpedo_attackrun_yaw_c6c{0.0f};
    float torpedo_attackrun_own_pose{0.0f};
    float torpedo_attackrun_target_pose{0.0f};
    float torpedo_attackrun_crossing{-1.0f};   // -1 = attackrun never entered
    int torpedo_attackrun_entries{0};
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
    // 009D0F10's own state object, the rest of task+6D8h. The five fields above
    // stay because the report and 009D3150 read them; this carries +18h, +1Ch,
    // +20h, +28h, +30h and +34h, which only the tick uses.
    bsp::TorpedoGoAwayRuntime torpedo_goaway_runtime{};
    int torpedo_goaway_ticks{0};
    int torpedo_goaway_arm_ticks[5]{0, 0, 0, 0, 0};  // index 1..4 = the four arms
    int torpedo_goaway_heading_ticks{0};             // arms that publish +2C0h
    float torpedo_goaway_alt_cmd_last{0.0f};
    float torpedo_goaway_alt_min{1e9f};
    float torpedo_goaway_alt_max{-1e9f};
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
    // desc+188h MaxSpd, the numerator of 009F9D30's run-profile speed ratio,
    // and the scale on the torpedo aim tick's pitch denominator at 009D1E64.
    float plane_max_spd{0.0f};
    // desc+1ACh PitchSpd (DEG(30) on this installation's TBD). 007DA8EB uses it
    // as the pitch rate; 009D1E39 divides the nose-down angle by it to shallow
    // the aim tick's dive command as the dive steepens.
    float plane_pitch_spd{0.0f};
    // desc+18Ch TravelSpeed, the airspeed 007C6340 seeds a plane with.
    float plane_travel_speed{0.0f};
    float plane_bomb_delay_1f4{1.0f};   // class+1F4h BombDelay
    // desc+164h Accel, desc+208h GlideRate, desc+1D4h DragPitchRatio and
    // desc+1DCh AirBrakeDrag: the four authored fields the thrust 007D9050 and
    // the drag 007D9140 are built from. docs/PLANE_POSE_THROTTLE_ALTITUDE.md.
    float plane_accel{0.0f};
    float plane_glide_rate{0.0f};
    float plane_drag_pitch_ratio{0.0f};
    float plane_air_brake_drag{0.0f};
    // desc+1F0h DropAngle, 009FB800's dive gain and cap.
    float plane_drop_angle{0.0f};
    // plan+2B4h and plan+2D8h. 009C1850 writes the first at 009C189A and raises
    // the second at 009C18A7, and 0099D300's throttle arms read both. They live
    // on the slot rather than in PilotPlanState because that header belongs to
    // another packet. docs/PILOT_THROTTLE_CUT_RAISER.md.
    float plane_desired_speed_2b4{0.0f};
    int plane_air_brake_mode_2d8{0};
    // plan+2B0h, a byte. Its only reader in the plane-AI range is 0099D924
    // `CMP byte [ESI+2B0h],0` in 0099D300's speed-demand arm: ZERO lets
    // 0099D92D-0099D970 raise the speed-error divisor plan+2B8h to
    // InterpolateClamped(-TrgSpeedCorrMinPitch, TrgSpeedCorrSpeedMul, 0, 1,
    // pitch) (singleton+5E8h/5ECh), non-zero skips that raise. The host's
    // throttle rule substitutes 1.0 for plan+2B8h, so nothing here reads it
    // yet; it is carried so the image's store sequence is complete.
    // docs/PLANE_FOLLOW_SPEED.md.
    std::uint8_t plane_trg_speed_corr_off_2b0{0};
    float plane_throttle_last{1.0f};
    float plane_think_dt{0.0f};   // 0099D300's argument, packet cc9_throttle_slot
    int plane_speed_commands{0};
    // desc+1E4h and desc+1ECh, the two 007C4850 derives with the 007D98F0
    // climb-angle solver at 007C4BE9 and 007C4C14. desc+1ECh is the climb arm's
    // gain, and it is 0.6 * desc+1E4h, NOT zero: docs/PLANE_FLIGHT.md read it as
    // having no producer because it is derived rather than authored.
    float plane_climb_angle_1e4{0.0f};
    float plane_climb_angle_1ec{0.0f};
    // desc+268h TurnCircleRadius, the scale both of the dive-bomb approach's
    // attack-distance draws multiply (009C3F86 and 009C3FB5, each dominated by
    // its own `MOV EBP,[ESI+8]`). Packet cc8_dive_race.
    float plane_turn_circle_radius{0.0f};
    // desc+194h SwimHeight, one of the two terms of the free-flight arm's water
    // line at 007CC4E8.
    float plane_swim_height{0.0f};
    int plane_water_contacts{0};
    // Packet cc9_plane_death_modes (docs/PLANE_DEATH_MODES.md): the death mode
    // 007CA8A0 chose, unit+C10h the explosion timer 007BBFA0 stored, the flags
    // 007D0B80 set (unit+C39h power lost, +C36h spinning, +C3Ah message seen),
    // and whether the aircraft has been killed and left the world.
    bsp::PlaneDeathMode plane_death_mode{bsp::PlaneDeathMode::None};
    float plane_death_timer_c10{-1.0f};
    bool plane_death_c39{false};
    bool plane_death_c36{false};
    bool plane_death_c3a{false};
    bool plane_death_removed{false};
    float plane_death_seconds{-1.0f};
    // The altitude 009FBA50 was last commanded with, and the pitch 009FB800
    // answered, kept for the census only.
    float plane_commanded_altitude{-1.0f};
    float plane_commanded_pitch{0.0f};
    float plane_dive_probe_timer{0.0f};
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
    // The wake trail at unit+0BD0h, appended by 00825F20's tail (00826CEE) and
    // read by 0070D290 for every formation follower's station. Packet
    // cc8_ship_follow, docs/SHIP_UNIT_GROUP_FOLLOW.md sections 5b and 5c.
    bsp::ShipAiWakeTrail wake{};
    // unit+284h, the unit group pointer 0070EF38 writes before any test, as an
    // index into Impl::formation_groups. -1 is a null pointer.
    std::int32_t formation_group{-1};
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

// ---------------------------------------------------------------------------
// Packet cc8_hull_aim_point: the aim point both attack tasks steer to.
// ---------------------------------------------------------------------------
// Until this packet both tasks aimed at the target's ORIGIN. The image does
// not: 009FADA0 stores target_world_matrix(target+CCh) x body_frame_offset to
// the sub-object's +1Ch, and for a ship that offset is a point drawn inside the
// authored hull box by 00816650 (docs/HULL_AIM_POINT.md).
//
// A monotonic counter gives each newly ordered (attacker, target) pair its own
// draw, the way each approach sub-object gets its own in the image. It is
// deterministic because the order in which targets are assigned is.
std::uint32_t g_hull_aim_pick_counter = 0;

// 00816650 is reached only through the nine unit vtables that carry it; the
// plane, land-vehicle and land-fort classes carry 0042D810, the origin.
//
// CORRECTED after the first USN04 pair measured nothing. This tested
// `class_id == kVehicleClassIsShipKind` (6), but that constant is one of
// 00964790's KIND TESTS (include/bsp/vehicle_class.hpp:176), not the leaf
// class_id this host stores on the slot. The run's own rows settle it:
// York-class02 kind=10, Lexington-class01 kind=9, Fletcher-class01 kind=7 -
// never 6 - so the predicate was false for every ship and the feed was inert.
//
// The ship family is the one src/game_hosts_ship_ai.cpp:73 already enumerates
// as has_ship_navigation_class. That function is in another translation unit's
// anonymous namespace, so the switch is restated rather than called; keep the
// two in step. It is the right family on the evidence side too: planes, land
// vehicles and land forts are exactly the families carrying 0042D810.
bool hull_aim_target_samples_hull(const GameUnitSlot& target) {
    switch (static_cast<bsp::VehicleClassKind>(target.class_id)) {
    case bsp::VehicleClassKind::Destroyer:
    case bsp::VehicleClassKind::Submarine:
    case bsp::VehicleClassKind::MotherShip:
    case bsp::VehicleClassKind::Cruiser:
    case bsp::VehicleClassKind::Cargo:
    case bsp::VehicleClassKind::LandingShip:
    case bsp::VehicleClassKind::BattleShip:
    case bsp::VehicleClassKind::TorpedoBoat:
        return true;
    default:
        return false;
    }
}

// THE NAMED SWITCH, and it is OFF. With the offset live, USN04 dive-bomb drops
// fell from 23 to 8 (local\hullaim_after2_usn04.log against
// local\hullaim_before_usn04.log, same base ec14870c3, identical parameters).
// The reading in docs/HULL_AIM_POINT.md predicts the bombs SCATTER along the
// hull, not that two thirds of them stop being released, so that is a missed
// prediction and it is not understood yet. Until it is, this host keeps the
// target's origin, which is the configuration measured to be line-identical to
// the before on all 43 census lines.
//
// Turning this to `true` re-arms the binding; docs/HANDOFF_HULL_AIM_POINT.md
// section (f) lists the three candidates to check first. Nothing below this
// line is disabled - the pick still runs and the state still advances, so a
// successor can print the drawn offset per aircraft without re-arming the feed.
//
// Packet cc9_hull_axis (docs/HULL_AIM_AXIS.md) measured the first two and both
// are clean: one draw per aircraft, and the offset lies on row 2, which is the
// ship's course. Three more sites read the hull point in the image (009C6342,
// 009C59CD, 009C5278) that this host feeds the origin, but feeding them does
// not restore the releases. The loss scales with the horizontal offset (1% and
// 10% of it: 23 releases, 50%: 16, 100%: 8) and not with its height, so it
// stays OFF. With it on, `hull_aim draw` and `hull_aim inrange` lines print.
constexpr bool kHullAimOffsetEnabled = false;
// Packet cc9_hull_turndown: the per-tick turndown/aimdive/aimglide trace in
// update_dive_bomb_approach. Diagnostic only; off in the default build.
constexpr bool kHullAimTrace = false;
// Packet cc9_aimdive_response: the aimdive tick's yaw, throttle and air-brake
// tail 009C5DB8-009C6080 (include/bsp/dive_bomb_aimdive_tail.hpp), read whole.
// OFF: bound, USN04 releases fell 23 -> 4 with the hull switch off
// (local/boff_usn04.log). The host's aim error saturates the pitch command, so
// the tail holds MinPowerCtrl 0.2 and MaxBrakeCtrl 0.5 through the dive and
// the aircraft arrives slow and shallow. docs/AIMDIVE_RESPONSE.md section 4.
// Re-measured with the throttle fix (cc9_dive_throttle, docs/DIVE_THROTTLE.md 4): the
// swing settles and the tail lifts the brake, but releases stay 0 because the
// aimglide pitch target 009C5522-009C55DF is unbound. Still OFF.
constexpr bool kAimDiveTailBound = false;
// Packet cc9_dive_flight_response: class+164h Accel as the image's reader leaves
// it (007D20F3-007D2127). docs/DIVE_FLIGHT_RESPONSE.md. OFF, measured: main 32 -> 24
// releases (local/S1fr_9000.log); the throttle-fix pair lacks its control.
constexpr bool kPlaneAccelCheatScaleBound = false;
// Packet cc9_dive_throttle: goaway's throttle and air-brake commands on both
// sides of the nose-down split, 009C4C0C-009C4CA7 and 009C4CBA-009C4CE1.
// docs/DIVE_THROTTLE.md section 1.
constexpr bool kGoawayThrottleBound = true;
// Packet cc9_dive_throttle: the aimglide tick's throttle and air brake
// 009C55E5-009C5679 and its cmd+2D8h = 0 at 009C567F, which this host lacked
// (so aimglide ran in the per-think speed mode). docs/DIVE_THROTTLE.md 2.
// ON with kGoawayThrottleBound, measured (local/G0_9000.log vs C0): the drop
// rows are unchanged, and only the post-release glide and climb-out paths move.
constexpr bool kAimGlideThrottleBound = true;
// Packet cc9_aimglide_pitch: the aimglide pitch target 009C5484-009C55DF
// (cmd+2BCh, cmd+2D0h = 2). docs/AIMGLIDE_PITCH.md section 1.
// OFF, measured with the throttle fix and the tail (U3, local/U3_9000.log):
// releases 16 against the control's 29 (8 against 17 at 4500). With the fix off
// it is unmeasured, so it lands off with the flip.
constexpr bool kAimGlidePitchBound = false;
// Packet cc9_aimglide_pitch: the aimglide yaw arm 009C53E7-009C542A, taken when
// the planar miss is under 140 m; the host ran only the heading arm.
// OFF with kAimGlidePitchBound, for the same measurement.
constexpr bool kAimGlideYawBound = false;
// Packet cc9_flyover_speed: the flyabove desired-speed arm 009C6F97-009C6FFB,
// and approach+50h fed with the aim point's height. docs/FLYOVER_SPEED.md.
// ON: USN04 moves only through the approach+50h feed on the two Yorktown
// squadrons (local/v1 = local/v4, the height-only run); the speed arm itself
// moves no census line there.
constexpr bool kFlyoverSpeedBound = true;

// Packet cc9_hull_axis, diagnostic only. The drawn body-frame offset, the
// world point, the target origin and heading, and the world point resolved
// back onto the target's row 2 (along) and row 0 (across). Row 2 is the axis
// 004142E0 multiplies body z by and the forward axis 00826866 and 0092D300 use.
void hull_aim_print(GameHostLog& log, const char* why, const GameUnitSlot& shooter,
                    const GameUnitSlot& target, std::size_t target_plus_one,
                    const std::array<float, 3>& w) {
    const bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    const float* m = target.world.data();
    const double d[3] = {static_cast<double>(w[0]) - m[12],
                         static_cast<double>(w[1]) - m[13],
                         static_cast<double>(w[2]) - m[14]};
    const double r2n = std::sqrt(static_cast<double>(m[8]) * m[8] +
                                 static_cast<double>(m[9]) * m[9] +
                                 static_cast<double>(m[10]) * m[10]);
    const double r0n = std::sqrt(static_cast<double>(m[0]) * m[0] +
                                 static_cast<double>(m[1]) * m[1] +
                                 static_cast<double>(m[2]) * m[2]);
    const double along = (d[0] * m[8] + d[1] * m[9] + d[2] * m[10]) / (r2n > 0 ? r2n : 1);
    const double across = (d[0] * m[0] + d[1] * m[1] + d[2] * m[2]) / (r0n > 0 ? r0n : 1);
    const double vx = target.motion.linear_velocity.x;
    const double vz = target.motion.linear_velocity.z;
    log.notef("hull_aim %s shooter=%s target=%s(#%u) seed=%u body=(%.2f %.2f %.2f)"
              " L=%.1f W=%.1f world=(%.1f %.1f %.1f) origin=(%.1f %.1f %.1f)"
              " pos=(%.1f %.1f %.1f) row2=(%.3f %.3f %.3f) row0=(%.3f %.3f %.3f)"
              " along=%.2f across=%.2f vel_heading=%.3f",
              why, shooter.row.name.c_str(), target.row.name.c_str(),
              static_cast<unsigned>(target_plus_one), shooter.hull_aim_seed,
              st.body_offset_28[0], st.body_offset_28[1], st.body_offset_28[2],
              target.motion_class.hull_length, target.class_width_00a4,
              w[0], w[1], w[2], m[12], m[13], m[14],
              target.motion.position[0], target.motion.position[1],
              target.motion.position[2], m[8], m[9], m[10], m[0], m[1], m[2],
              along, across, std::atan2(vx, vz));
}

// The world aim point for `shooter` against `target`. Returns false when the
// target supplies no hull, in which case the caller keeps the origin it had.
bool hull_aim_world_point(GameUnitSlot& shooter, const GameUnitSlot& target,
                          std::size_t target_plus_one, float out[3],
                          GameHostLog* log = nullptr) {
    if (shooter.hull_aim_target_plus_one != target_plus_one) {
        // A new ordered target means a new sub-object: 009FB200.
        shooter.hull_aim_ref =
            bsp::approach_target_ref_construct_009fb200(0.0f);
        shooter.hull_aim_target_plus_one = target_plus_one;
        shooter.hull_aim_seed = ++g_hull_aim_pick_counter;
    }
    bsp::ApproachTargetRefState& st = shooter.hull_aim_ref;
    const bool samples = hull_aim_target_samples_hull(target);
    if (st.dirty_41) {
        // 009FAEB0-009FAEB8, the dirty byte the constructor set at 009FB272.
        bsp::LeadAimHullExtents hull;
        hull.length = target.motion_class.hull_length;  // class+A0h `Length`
        hull.width = target.class_width_00a4;           // class+A4h `Width`
        hull.height = target.motion_class.hull_height;  // class+A8h `Height`
        const std::array<float, 4> unit =
            bsp::approach_target_ref_unit_draws_substitute(shooter.hull_aim_seed);
        const bsp::ShipLeadRandomDraws draws =
            bsp::approach_target_ref_draws_from_unit(unit, st.spread_48);
        bsp::approach_target_ref_pick_009fa260(st, hull, draws, samples);
        if (kHullAimOffsetEnabled && samples && log != nullptr) {
            // Packet cc9_hull_axis: one line per draw, so a re-draw shows up
            // as a second line for the same (attacker, target).
            std::array<float, 3> w{};
            bsp::transform_point_004142e0(st.body_offset_28, target.world, w);
            hull_aim_print(*log, "draw", shooter, target, target_plus_one, w);
        }
    }
    if (!samples || !kHullAimOffsetEnabled) return false;
    // 009FAED0-009FAF00.
    bsp::approach_target_ref_store_world_point_009faeea(st, target.world);
    out[0] = st.world_point_1c[0];
    out[1] = st.world_point_1c[1];
    out[2] = st.world_point_1c[2];
    return true;
}

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
    // Packet cc8_ship_follow: the 508h-byte unit groups 0070DB20 allocates. A
    // unit points at one through GameUnitSlot::formation_group (unit+284h).
    struct FormationGroup {
        std::size_t leader{0};                               // group+14h
        std::vector<bsp::ShipAiUnitGroupMember> members;     // group+18h, 34h each
        std::int32_t column{0};                              // group+500h, the
                                                             // pattern index the
                                                             // constructor leaves 0
        std::int32_t type_04fc{6};                           // group+4FCh, 6 for a
                                                             // ship leader (0070D82F)
    };
    std::vector<FormationGroup> formation_groups;
    unsigned long long formation_joins{0};
    unsigned long long formation_creates{0};
    unsigned long long formation_rejoins{0};
    unsigned long long formation_clamped{0};
    unsigned long long formation_columns_unmeasurable{0};
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

    // ---- the dive-bomb task's host side. docs/DIVE_BOMB_TASK.md ----
    // docs/GAME_TUNING_SINGLETON.md rows +4C0h..+4D8h.
    static constexpr float kPilotDiveBombCruisingAlt = 1300.0f;
    static constexpr float kPilotDiveBombAttackDist = 1100.0f;
    static constexpr float kPilotDiveBombBeginAltRange1 = 1000.0f;
    static constexpr float kPilotDiveBombBeginAltRange2 = 1200.0f;
    // scripts/datatables/robots.lua, the SPNormal row, the fields
    // include/bsp/robot_config.hpp names from the same Lua keys.
    // docs/GAME_TUNING_SINGLETON.md +4D8h, `Pilot/DiveBomb/ReferenceSpeed`,
    // authored KMH(280) and read by 009C3EA0 - the divisor of task+41Ch's
    // max(1.0, MaxSpd / ReferenceSpeed). Logged, not yet bound.
    static constexpr float kPilotDiveBombReferenceSpeed = 280.0f / 3.6f;
    static constexpr float kDiveBombReleaseAlt1 = 350.0f;   // row+38h
    static constexpr float kDiveBombReleaseAlt2 = 450.0f;   // row+3Ch
    // These four are (approach+14h)->+5Ch, +60h, +64h and +68h. 009F9D1E sets
    // approach+14h to `00F8A30C + index * 248h + 0Ch`, so the record is a
    // 0x248-stride robots row viewed 0xCh in: ->+5Ch is row+68h, ->+60h is
    // row+6Ch, ->+64h is row+70h and ->+68h is row+74h. include/bsp/
    // robot_config.hpp names those four dive_bomb_aim_prec_dist_068,
    // _mul_06c, _pull_plus_070 and _pull_minus_074, and this installation's
    // scripts/datatables/robots.lua SPNormal row authors them. So the first
    // two are no longer an assumption and the last two are recovered.
    static constexpr float kDiveBombAimPrecDist = 70.0f;    // row+68h, ->+5Ch
    static constexpr float kDiveBombAimPrecMul = 0.3f;      // row+6Ch, ->+60h
    // "tavolsagtol fuggoen mennyire huzza a pitch-t, ha nem pontos a celzas" -
    // how much it pulls the pitch when the aim is off - and its push twin.
    static constexpr float kDiveBombAimPrecPullPlus = 0.018f;   // row+70h, ->+64h
    static constexpr float kDiveBombAimPrecPullMinus = 0.025f;  // row+74h, ->+68h
    // row+4Ch, ->+40h. "ha nem leboritott manoverrel bombaz, csak siman
    // rarepulve, akkor a fenti ReleaseAlt erteket ennyivel megszorozva
    // hasznalja" - bombing without the wingover, it uses ReleaseAlt times this.
    static constexpr float kDiveBombNewReleaseMul = 0.6f;       // row+4Ch, ->+40h

    // Packet cc9_difficulty. The six PilotBot rows the approach can capture,
    // the dive-bomb fields only, named as include/bsp/robot_config.hpp's
    // PilotBotParameters names them. SUBSTITUTION, labelled: the rows are
    // PilotBotConfig.levels[0..5] (00F8A30C + 0Ch), which 00901610 fills from
    // robots.lua. This process cannot build the context 004DC6A0 hands that
    // reader (src/game_hosts_mission_frame.cpp records the step), so the values
    // are this installation's scripts/datatables/robots.lua PilotBot blocks,
    // copied by line: Stun :1259-1273, SPNormal :568-582, SPVeteran :707-721,
    // MPNormal :845-859, MPVeteran :983-997, Elite :1121-1135. The level index
    // is 00901610's (register_robot_config_009013d0's table): Stun 0,
    // SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5.
    struct PilotDiveBombRow {
        float release_alt_1_044;
        float release_alt_2_048;
        float new_release_mul_04c;
        float max_power_ctrl_050;
        float min_power_ctrl_054;
        float max_brake_ctrl_058;
        float min_brake_ctrl_05c;
        float aim_pitch_ratio_060;
        float aim_prec_dist_068;
        float aim_prec_mul_06c;
        float aim_prec_pull_plus_070;
        float aim_prec_pull_minus_074;
    };
    static constexpr PilotDiveBombRow kPilotDiveBombRows[6] = {
        {350.0f, 450.0f, 0.6f, 0.6f, 0.1f, 0.5f, 0.0f, 3.0f, 150.0f, 0.75f, 0.03f, 0.06f},   // Stun
        {350.0f, 450.0f, 0.6f, 0.7f, 0.2f, 0.5f, 0.0f, 3.0f, 70.0f, 0.3f, 0.018f, 0.025f},   // SPNormal
        {250.0f, 300.0f, 0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},        // SPVeteran
        {250.0f, 300.0f, 0.6f, 0.85f, 0.5f, 0.25f, 0.0f, 3.0f, 50.0f, 0.24f, 0.014f, 0.018f}, // MPNormal
        {250.0f, 300.0f, 0.6f, 1.0f, 0.8f, 0.1f, 0.0f, 3.0f, 20.0f, 0.20f, 0.012f, 0.016f},  // MPVeteran
        {250.0f, 300.0f, 0.6f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},        // Elite
    };
    static_assert(kPilotDiveBombRows[1].release_alt_1_044 == kDiveBombReleaseAlt1
                  && kPilotDiveBombRows[1].aim_prec_dist_068 == kDiveBombAimPrecDist
                  && kPilotDiveBombRows[1].aim_prec_mul_06c == kDiveBombAimPrecMul
                  && kPilotDiveBombRows[1].aim_prec_pull_plus_070 == kDiveBombAimPrecPullPlus
                  && kPilotDiveBombRows[1].aim_prec_pull_minus_074 == kDiveBombAimPrecPullMinus
                  && kPilotDiveBombRows[1].new_release_mul_04c == kDiveBombNewReleaseMul,
                  "row 1 is the SPNormal row the host used before the binding");
    // The captured row. Switch off: SPNormal, the old behaviour. An index
    // outside 0..5 would read past the native's array; the host takes row 1.
    static const PilotDiveBombRow& dive_bomb_row(const GameUnitSlot& slot) noexcept {
        const int i = slot.db_skill_row_14;
        if (!kSkillLevelBound || i < 0 || i > 5) return kPilotDiveBombRows[1];
        return kPilotDiveBombRows[i];
    }

    // 007C1DB0: the device list at unit+48h, summing 006E3500 over every device
    // whose vtable[+5Ch] answers 25h. The gunnery host owns that list; the
    // count here is the aircraft's bomb platforms, one round each, minus what
    // it has already dropped.
    int dive_bomb_rounds_remaining(GameUnitSlot& slot) {
        const int dropped = slot.torpedo_drops_spawned;
        const int carried = slot.dive_bomb_task_installed
            ? slot.dive_bomb_rounds_remaining + dropped
            : kDiveBombCarriedRoundsSubstitute;
        const int left = carried - dropped;
        return left > 0 ? left : 0;
    }
    // SUBSTITUTION, labelled: 006E3500's per-device round count is unread, so
    // the host gives a bomb-carrying aircraft one salvo's worth. The aimglide
    // release at 009C5777 caps its loop against this number, so it decides how
    // many bombs a single glide drop puts out.
    static constexpr int kDiveBombCarriedRoundsSubstitute = 2;

    // 009C7A80's outputs, the producer of every state-machine input. The body
    // is 009C7A80-009C7E9E and its tail 009C7C5B-009C7E9E is read for its
    // outputs only, so this is a PARTIAL binding of the rules that were read.
    void update_dive_bomb_approach(GameUnitSlot& slot, float dt,
                                   bool diving = false) {
        (void)dt;
        // 009C7AFE: approach+D1h = BSP_WeaponController_HasGeneralBombOrdnance.
        const bsp::OrdnanceKindSet set{slot.ordnance_mask};
        slot.db_has_bomb_d1 = bsp::ordnance_has_general_bomb_2ah(set) &&
                              dive_bomb_rounds_remaining(slot) > 0;
        // 009C7A96: approach+ACh = ctl+398h, which the cruise profile 009C8920
        // holds at Pilot/DiveBomb/BeginAltRange/1.
        slot.db_begin_alt_ac = kPilotDiveBombBeginAltRange1;
        // 009C7A9E-009C7AB4, reconstructed. ctl+39Ch is the 9999.0f at
        // 00CE4C04 the same profile writes, so the min leaves +A8h alone below
        // about 9523 and this is a ceiling, not a decay, at these altitudes.
        slot.db_dive_alt_a8 = bsp::dive_bomb_decay_dive_altitude_009c7a94(
            slot.db_dive_alt_a8, bsp::dive_bomb_constant::kCruisingAltitudeThird);
        // 009C8A5E, the cruise profile's clamp on task+4B0h == approach+B8h.
        {
            const float wanted = kPilotDiveBombAttackDist * 1.0f;
            if (wanted > slot.db_in_range_b8) slot.db_in_range_b8 = wanted;
        }
        if (slot.command_target_plus_one == 0) {
            slot.db_in_range_d0 = false;   // 009C7B0A
            return;
        }
        const std::size_t ti = slot.command_target_plus_one - 1;
        if (ti >= slots.size()) { slot.db_in_range_d0 = false; return; }
        // Packet cc8_hull_aim_point. This was the target's ORIGIN; 009FADA0 on
        // approach+30h stores the hull point instead. On a target whose vtable
        // slot +100h is 0042D810 the helper leaves the origin in place, which
        // is what that class does.
        float tp_hull[3] = {slots[ti]->motion.position[0],
                            slots[ti]->motion.position[1],
                            slots[ti]->motion.position[2]};
        hull_aim_world_point(slot, *slots[ti], ti + 1, tp_hull, &log);
        const float* const tp = tp_hull;
        // Packet cc9_flyover_speed: approach+50h is sub+20h, the aim point's
        // height that 009FADA0 stores at 009FAEF5 every tick. The enter-time
        // zero near 009C4F00 stands for the ticks before the first update.
        if (kFlyoverSpeedBound) slot.db_aim_point_height_50 = tp[1];
        // 009C7B4F-009C7B80: the planar distance, components 0 and 2 only.
        const double dx = static_cast<double>(tp[0]) -
                          static_cast<double>(slot.motion.position[0]);
        const double dz = static_cast<double>(tp[2]) -
                          static_cast<double>(slot.motion.position[2]);
        const double d2 = dx * dx + dz * dz;
        slot.db_planar_bc = (d2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
            ? 0.0f : static_cast<float>(std::sqrt(d2));
        // Packet cc9_hull_axis: once per (attacker, target), when the range to
        // the aim point first falls inside approach+B8h, the attackrun ->
        // fly-over hand-over range. Diagnostic only, behind the switch.
        if (kHullAimOffsetEnabled && slot.db_planar_bc < slot.db_in_range_b8 &&
            slot.hull_aim_range_printed_plus_one != ti + 1) {
            slot.hull_aim_range_printed_plus_one = ti + 1;
            hull_aim_print(log, "inrange", slot, *slots[ti], ti + 1,
                           {tp[0], tp[1], tp[2]});
            log.notef("hull_aim inrange shooter=%s planar_to_aim=%.1f "
                      "planar_to_origin=%.1f shooter_pos=(%.1f %.1f %.1f)",
                      slot.row.name.c_str(), static_cast<double>(slot.db_planar_bc),
                      std::sqrt(std::pow(static_cast<double>(slots[ti]->motion.position[0]) -
                                             slot.motion.position[0], 2) +
                                std::pow(static_cast<double>(slots[ti]->motion.position[2]) -
                                             slot.motion.position[2], 2)),
                      slot.motion.position[0], slot.motion.position[1],
                      slot.motion.position[2]);
        }
        // 009C8B14-009C8B3B, the break-off's own range. The image subtracts all
        // THREE components of `aimPoint - unitPosition` and takes 0042B2F0's
        // length; approach+BCh is planar and measured to the target entity, so
        // feeding it made the break-off fire late and low. The aim point this
        // host carries is the commanded target's position (the same labelled
        // substitution +BCh and +C0h already run on), so the x and z are the
        // same two differences and only the vertical term is new.
        {
            const double dy = static_cast<double>(tp[1]) -
                              static_cast<double>(slot.motion.position[1]);
            const double d3 = d2 + dy * dy;
            slot.db_aim_point_3d =
                (d3 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(d3));
        }
        // 009C7B8A-009C7BB0: pi/2 - atan2, wrapped into [0, 2pi).
        {
            float b = static_cast<float>(bsp::dive_bomb_constant::kHalfPi -
                                         std::atan2(dz, dx));
            if (b < 0.0f) {
                b += static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_bearing_c0 = b;
        }
        // 009C7BFB-009C7C31, reconstructed.
        bsp::DiveBombRangeLatchInputs lin;
        lin.latched = slot.db_in_range_d0;
        lin.planar_distance = slot.db_planar_bc;
        lin.in_range_distance = slot.db_in_range_b8;
        lin.control_flag_369 = false;
        lin.global_e17bf2 = false;
        // 009C7C31-009C7CFE, the spent-member arm. A bomber with no bombs left
        // that is not its flight leader has its latch ANDed with "the leader is
        // within approach+B8h of this aircraft's aim point" - a second,
        // independent mechanism taking a spent squadron out of its attack, and
        // it bites in exactly the situation docs/BOMBER_AFTER_TASK.md section 10
        // is about. `[sqn+3D0h]` is member_units[0]: 0099B757 compares the unit
        // against that same dword to decide the flight lead, and 009C7C7C reads
        // it for a position at +FCh/+104h.
        lin.has_bomb_ordnance_d1 = slot.db_has_bomb_d1;
        {
            auto* const sqn = bsp::plane_squadron_registry().find_by_member_unit(
                slot.process_index);
            if (sqn != nullptr) {
                for (const std::size_t member : sqn->member_units) {
                    if (member == bsp::kPlaneSquadronNoUnit) continue;
                    lin.is_flight_leader = (member == slot.process_index);
                    if (!lin.is_flight_leader && member < slots.size()) {
                        const float* const lp = slots[member]->motion.position;
                        // SUBSTITUTION, labelled: the aim point is this host's
                        // commanded-target position, the same stand-in +BCh and
                        // +C0h run on, so this is |target.xz - leader.xz|.
                        const double lx = static_cast<double>(tp[0]) -
                                          static_cast<double>(lp[0]);
                        const double lz = static_cast<double>(tp[2]) -
                                          static_cast<double>(lp[2]);
                        // 00414C60 BSP_Vector2f_LengthWithCutoff carries the
                        // same 00CE3820 cutoff the other two sqrt guards use.
                        const double l2 = lx * lx + lz * lz;
                        lin.leader_to_aim_point =
                            (l2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                                ? 0.0f : static_cast<float>(std::sqrt(l2));
                        lin.leader_known = true;
                    }
                    break;
                }
            }
        }
        slot.db_in_range_d0 = bsp::dive_bomb_in_range_latch_009c7c31(lin);
        // 009C7D04-009C7E33, packet cc8_dive_release item 2. approach+D8h/+DCh/
        // +E0h is rewritten HERE, every tick, not latched: see the header note on
        // dive_bomb_impact_point_009c7d71. Before the aircraft is diving and
        // before the range latch it is the raw unit position (009C7D27); from
        // then on it is the predicted bomb impact point (009C7D71). The aim
        // point this host has is the commanded target's position, the same
        // substitution +BCh/+C0h above already carry.
        if (!diving && !slot.db_in_range_d0) {
            slot.db_run_in_origin[0] = slot.motion.position[0];
            slot.db_run_in_origin[1] = tp[1];
            slot.db_run_in_origin[2] = slot.motion.position[2];
            slot.db_impact_fall_time = 0.0f;
        } else {
            bsp::DiveBombImpactPointInputs ip;
            ip.unit_position[0] = slot.motion.position[0];
            ip.unit_position[1] = slot.motion.position[1];
            ip.unit_position[2] = slot.motion.position[2];
            ip.unit_velocity[0] = slot.motion.linear_velocity.x;
            ip.unit_velocity[1] = slot.motion.linear_velocity.y;
            ip.unit_velocity[2] = slot.motion.linear_velocity.z;
            ip.aim_point_y = tp[1];
            const bsp::DiveBombImpactPoint r =
                bsp::dive_bomb_impact_point_009c7d71(ip);
            slot.db_run_in_origin[0] = r.point[0];
            slot.db_run_in_origin[1] = r.point[1];
            slot.db_run_in_origin[2] = r.point[2];
            slot.db_impact_fall_time = r.fall_time;
        }
        // 009C5950/009C5960 then 009C5A40 and 009C5AF1: the aim error's range
        // and bearing are taken from that point, not from the aircraft.
        {
            const double idx = static_cast<double>(tp[0]) -
                               static_cast<double>(slot.db_run_in_origin[0]);
            const double idz = static_cast<double>(tp[2]) -
                               static_cast<double>(slot.db_run_in_origin[2]);
            const double id2 = idx * idx + idz * idz;
            slot.db_impact_planar_5c =
                (id2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(id2));
            float ib = static_cast<float>(bsp::dive_bomb_constant::kHalfPi -
                                          std::atan2(idz, idx));
            if (ib < 0.0f) {
                ib += static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_impact_bearing_18 = ib;
        }
        // 009C5207-009C522E then 009C52C7-009C5303: the aimglide's throw, the
        // planar distance from the aircraft to that same predicted impact
        // point. Built with the same epsilon the other two sqrt guards use
        // (00CE3820, 1e-10), which 009C52DB compares the squared sum against.
        {
            const double wx = static_cast<double>(slot.db_run_in_origin[0]) -
                              static_cast<double>(slot.motion.position[0]);
            const double wz = static_cast<double>(slot.db_run_in_origin[2]) -
                              static_cast<double>(slot.motion.position[2]);
            const double w2 = wx * wx + wz * wz;
            slot.db_impact_throw_14 =
                (w2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(w2));
        }
        // Packet cc9_hull_turndown, diagnostic only: one line per tick of the
        // turndown, aimdive and aimglide, every quantity relative to the fed
        // aim point `tp` so the offset itself cancels between builds. The
        // commands are the ones the state ticks wrote on the previous tick.
        if (kHullAimTrace &&
            (slot.dive_bomb_state == bsp::DiveBombState::kTurnDown ||
             slot.dive_bomb_state == bsp::DiveBombState::kAimDive ||
             slot.dive_bomb_state == bsp::DiveBombState::kAimGlide)) {
            log.notef("hull_trace %s t=%d st=%x rel=(%.2f %.2f %.2f) "
                      "vel=(%.2f %.2f %.2f) hdg=%.4f pitch=%.4f bank=%.4f "
                      "cmd_pitch=%.4f bank_tgt=%.4f hdg_tgt=%.4f "
                      "ccip_rel=(%.2f %.2f) rng=%.2f latch=%d err=%.2f "
                      "ccip_d=%.2f brg_c0=%.4f brg_18=%.4f abort=%d rel_n=%d "
                      "yaw=%.3f thr=%.3f brk=%.3f roll=%.3f aimhdg=%.4f live_roll=%.3f",
                      slot.row.name.c_str(), slot.dive_bomb_arm_ticks,
                      static_cast<unsigned>(slot.dive_bomb_state),
                      slot.motion.position[0] - tp[0], slot.motion.position[1] - tp[1],
                      slot.motion.position[2] - tp[2],
                      slot.plane_world_velocity[0], slot.plane_world_velocity[1],
                      slot.plane_world_velocity[2], slot.plane_heading_c6c,
                      slot.plane_pitch_angle_c64, slot.plane_bank_angle_c68,
                      slot.plane_commanded_pitch, slot.plan_state.bank_target_2c4,
                      slot.plan_heading_2c0,
                      slot.db_run_in_origin[0] - tp[0], slot.db_run_in_origin[2] - tp[2],
                      slot.db_planar_bc, slot.db_in_range_d0 ? 1 : 0,
                      slot.db_aim_error_last, slot.db_impact_planar_5c,
                      slot.db_bearing_c0, slot.db_impact_bearing_18,
                      slot.db_abort_fires, slot.dive_bomb_releases,
                      slot.plan_slots[bsp::kPilotSlotYaw].desired,
                      slot.plan_slots[bsp::kPilotSlotThrottle].desired,
                      slot.plan_slots[bsp::kPilotSlotAirBrake].desired,
                      static_cast<double>(slot.db_aimdive_roll_last),
                      static_cast<double>(slot.db_aim_heading_last),
                      static_cast<double>(slot.plane_live_controls[2]));
        }
    }

    bsp::DiveBombTransitionInputs dive_bomb_transition_inputs(GameUnitSlot& slot) {
        bsp::DiveBombTransitionInputs in;
        in.current = slot.dive_bomb_state;
        in.engaged.in_range_latch_4c8 = slot.db_in_range_d0;
        // squadron+370h, the attack mode 009C83F8 reads through task+404h.
        // The constant 2 makes `engaged` true for as long as a target is
        // latched, which makes BOTH of 009C83E0's return-to-approach edges
        // (009C8419-009C845E in the attacking half, 009C873E-009C8783 in the
        // other) unreachable. The real producer is the flight leader's 0099B740
        // tick, which SETS it to 1 every think; `slot.db_attack_mode_370` now
        // carries that value and is correct (docs/BOMBER_AFTER_TASK.md 10.6).
        //
        // NOT WIRED, and the measurement says why. Feeding the real mode here
        // was measured on USN04, same binary apart from this one line
        // (docs/BOMBER_AFTER_TASK.md 10.9): it does take the leader out of the
        // sea -- `movieval` loses its `plane water contact` and its 303 done
        // ticks, and the approach edge fires for the first time -- but it also
        // takes `movieval`'s releases from 2 to 0 and the mission's dive-bomber
        // water contacts from 1 to 7. With `engaged` collapsed to the latch,
        // entry into the attack waits for `d < approach+B8h` = 1100 m, and this
        // host's `moveto` approach state does not fly an attack profile, so the
        // bombers arrive low and exit aimdive to `goaway` at ~240 m instead of
        // `aimglide` at ~600 m. The gate is faithful; the approach state behind
        // it is not yet. Re-wire this when moveto/follow are real, not before.
        //
        // STILL NOT WIRED after packet cc8_dive_approach, and the new
        // measurement says the blocker has MOVED. `moveto` is now real: the arm
        // dispatches 009C18C0 for kMoveTo and it commands the glide from
        // BeginAltRange/1 above the target down to that altitude at
        // approach+B4h. Runs A2 (unwired) and B (this line reading
        // db_attack_mode_370), same binary apart from this line:
        //
        //   dive entry altitude   651 / 626 / 677 m  ->  1044 / 1040 / 1045 m
        //   aim error 009C5C9B    -24.9 / -33.1 / -18.0 m -> +12.9 / -3.7 / +17.3 m
        //   mission water contacts             1    ->  0
        //   total dive-bomb releases           5    ->  0
        //
        // So the approach defect 10.9 diagnosed is FIXED: the bombers now enter
        // the dive at the authored BeginAltRange/1 instead of 200-400 m below
        // it, all three aim errors move inside the 25 m gate, and nobody
        // ditches. What the wiring costs is the five releases, and they are not
        // lost to this change: A2's releases all came from the AIMDIVE gate
        // reached from a dive entry 350 m too low, and with the faithful entry
        // every bomber pulls out into AIMGLIDE instead -- where the release gate
        // passes ZERO times in both runs (9 and 11 aimglide rows, `passed=0`
        // and `releases=0` on every one, `rearm` blocking 343 of 344 calls).
        // The aimglide re-arm timer is frozen in this tree: only the aimdive
        // input builder counts it down (line ~1544, 009C58E9); the aimglide
        // builder below reads it and never decrements it. Packet
        // cc8_dive_entry reports fixing exactly that on agent/cc8-dive-entry,
        // which is not in this tree.
        //
        // Re-wire this once that fix is merged and B is re-run; the experiment
        // is one line and one window. docs/DIVE_BOMB_APPROACH.md section 13.
        //
        // RE-TAKEN on the merged base dd5364d6d, where approach+B8h is the
        // image's draw 2080.0 instead of main's old 1100.0, and the verdict
        // CHANGED SIGN on the criterion that mattered. Runs A2' and B', same
        // binary apart from this line:
        //
        //   total dive-bomb releases          19  ->  20   (up)
        //   movieval / #1.1 releases        2 / 2  ->  2 / 2
        //   mission water contacts             0  ->  2
        //   movieval approach_returns          -  ->  0, done_ticks=609
        //
        // The releases held and rose, but movieval NEVER leaves done, and on
        // the OLD base with R = 1100 it left done entirely (no done ticks at
        // all). The reason is this line's own gate: engaged = latch || (mode ==
        // 2 && target), the mode is 1, so engaged IS the latch, and the latch
        // disengages only past approach+B8h + 100. movieval ends at
        // approach+BCh = 494.9 m against B8h = 2080.0, so the latch can never
        // clear and 009C8483's `CMP EDI,EBX / JZ ret` parks it. Raising B8h
        // from 1100 to 2080 widened the hysteresis past the whole engagement,
        // which is what removes the benefit this feed had at R = 1100.
        //
        // So the two ditchers are downstream of that: D3A Val #1.1|.-2 had
        // ALREADY released both bombs and descends out of done (alt 274.5 ->
        // 31.3 unwired, 270.6 -> 0.0 wired - the same descent, 31 m of margin
        // in one and none in the other), and #3.1|.-2 dives with an aim error
        // of -135 m and flies in. Neither is a moveto defect; both are the
        // done-state descent and the dive aim, reached more often because the
        // latch holds.
        // FED, packet cc8_follow_enter run D, under the integrator's explicit
        // grant of 2026-09-19 (this line is the integrator's pin, not a
        // worker's). It was pinned at 2 on two earlier measurements in which
        // wiring the real squadron attack mode was a net regression
        // (docs/BOMBER_AFTER_TASK.md 10.9 and docs/DIVE_BOMB_APPROACH.md's B'
        // run): the leader sets the mode to 1 every think via
        // 0099B740 -> 007ED3F0, so `engaged` collapses to the in-range latch at
        // R = approach+B8h = 2080 m.
        //
        // Both verdicts were taken when EVERY aircraft answered 007B8AD0 as a
        // leader and flew its own moveto, with no follow state to fall back to.
        // This packet removed that premise, so the verdict was RE-TAKEN as run
        // D. It regressed again, for a third reason, and the pin STAYS. USN04,
        // 4800 mission frames, placement ON in every run:
        //
        //   run                     follow law  releases  mutual kills  deaths
        //   A  before                        0        35             4      14
        //   B  predicate fed                 0        35             0      10
        //   D  B + this line fed            32        26             0       6
        //
        // D is the first run in this chain in which the follow LAW executes at
        // all, and it holds B's gains (the four mutual torpedo kills stay gone,
        // deaths fall further). But nine releases are lost, and the state that
        // loses them is FLYABOVE. Six dive-bomber wing members end the run like
        //
        //   divebomb movieval|.-2 arm_ticks=1812 transitions=1
        //       states[follow=1546 flyabove=266] releases=0 rounds_left=2
        //
        // - one transition, all of it: they hold formation for 1100-1550 ticks,
        // leave follow into flyabove, and the run ends before the approach
        // sequence (aimdive -> turndown -> attackrun) can complete. They never
        // reach `done` either, so criterion (c) becomes untestable rather than
        // passing. The aircraft are not lost to a bad command; they are lost to
        // holding station until there is no mission left to fly.
        //
        // What that means is that this pin is NOT only propping up the missing
        // follow state, as this packet expected. Something downstream of follow
        // is too slow - most likely the flyabove arm, or the moment the member's
        // own latch is allowed to set. Feeding this line honestly needs that
        // read first; it is not blocked on the follow entry any more.
        // PACKET cc8_follow_attack. The pin STAYS, but for a different reason
        // than the two paragraphs above give, and both of those are now wrong.
        //
        // (1) "They are lost to holding station until there is no mission left
        // to fly" is FALSE. Run B has no `plane water contact` line; run D has
        // four, and two are those wing members, at alt=-1.34 and -1.66 with the
        // last flyabove altitude sample at err=-209.4 against target=210.0.
        // Of the six: two drowned, two are in a fly-over/go-away limit cycle
        // (transitions=6 with goaway=1071, and transitions=32), and only the
        // #5.1 pair is the mission window - and that pair released 0 in run B.
        //
        // (2) The pin is NOT the faithful value. docs/TORPEDO_ATTACK_MODE.md's
        // writer census gives mode 2 exactly two producers image-wide, 008A4C41
        // inside the Lua binding 008A4B10 (callsite_census 007ED430 = total 1)
        // and pilot-control message BCh; an AI-ordered bomber reaches neither,
        // so 0099B740 leaves it at 1. What the pin actually buys is `attackrun`,
        // which 009C8310 can only return when `engaged` is true with the latch
        // CLEAR - impossible at mode 1, where 009C83F8 collapses `engaged` to
        // the latch.
        //
        // (3) The blocker is THIS HOST'S placement substitution, not anything
        // downstream of follow: see the note on
        // place_wing_member_on_station_007f23a0 below.
        //
        // (4) And the pinned baseline is not safe either. Run E1 (pinned, USN04
        // at 9000 mission frames) reproduces run B and then the `done` state
        // flies everything into the sea - every dive-bomb done row descends
        // without stopping (alt 278.5 -> 0.1, 274.5 -> 0.0) and the log carries
        // SIXTEEN water contacts against run D's four. 4800 frames merely ended
        // the mission first. Dive-bomb releases pinned at 9000 are 30.
        //
        // So the pin is held here only because the change it blocks has not
        // been measured yet, not because it is right. Run E2 measures it; see
        // docs/FOLLOWER_ATTACK_HANDOVER.md sections 8 and 9.
        in.engaged.control_mode_370 = 2;
        in.engaged.has_latched_target_440 = slot.command_target_plus_one != 0;
        in.entry.control_mode_370 = in.engaged.control_mode_370;
        in.entry.has_bomb_ordnance_4c9 = slot.db_has_bomb_d1;
        in.entry.control_flag_369 = false;
        in.entry.global_e17bf2 = false;
        in.entry.in_range_latch_4c8 = slot.db_in_range_d0;
        // FED, packet cc8_follow_enter, replacing a hardcoded `true`. The field
        // name follows the ledger's misreading; 007B8AD0 is the flight-leader
        // test (see unit_is_flight_leader_007b8ad0). 009C8419-009C845E and
        // 009C873E-009C8783 are the two `!engaged` edges and both make the same
        // choice the constructor 009C7777 made: AL != 0 -> +4F0h moveto,
        // AL == 0 -> +52Ch follow. This is what lets the host enter follow at
        // all; with the hardcode every aircraft took moveto and the follow tick
        // never ran.
        in.unit_lacks_follow_target =
            unit_is_flight_leader_007b8ad0(slot.process_index);
        {
            bsp::DiveBombBreakOffInputs b;
            b.base_0099c230 = true;
            b.has_latched_target = slot.command_target_plus_one != 0;
            b.has_bomb_ordnance_4c9 = slot.db_has_bomb_d1;
            // CORRECTED, packet cc8_dive_approach. This was db_planar_bc, which
            // is planar and measured to the target entity; 009C8B14-009C8B3B
            // measures the 3-D `aimPoint - unitPosition`. Both errors pushed the
            // same way, so the break-off used to fire later and lower than the
            // image's. docs/BOMBER_AFTER_TASK.md 10.10.
            b.distance_to_target = slot.db_aim_point_3d;
            b.speed_ratio_41c = 1.0f;
            in.should_break_off = bsp::dive_bomb_should_break_off_009c8a90(b);
            // 009C8B40-009C8B51: the range arm is `[tuning+4C8h] * [task+41Ch]`
            // against the 3-D range. NOTE the id-space collision: that +4C8h is
            // Pilot/DiveBomb/SafeDist on the tuning singleton 0042E740, a
            // different object from the task's +4C8h in-range latch above.
            slot.db_dbg_break_off = in.should_break_off;
            slot.db_dbg_break_off_distance = b.distance_to_target;
            slot.db_dbg_break_off_threshold = b.safe_distance * b.speed_ratio_41c;
        }
        slot.db_dbg_engaged = bsp::dive_bomb_engaged_009c83f8(in.engaged);
        // 009C62B0 is defined and its two decisive flags are recovered, so
        // the geometry stand-in is gone. docs/DIVE_BOMB_TASK.md.
        //
        // All three flyabove flags key on ONE height: B at 009C6493, the
        // aircraft's Y less out[1] of the approach's vtable[0] (009C40A0), and
        // the same frame slot reaches 009C67C7's can-dive test. This host's aim
        // point is the commanded target's own position, so out[1] is its Y.
        {
            const float* const own_p = slot.motion.position;
            float target_p[3] = {own_p[0], own_p[1], own_p[2]};
            float target_v[3] = {0.0f, 0.0f, 0.0f};
            if (slot.command_target_plus_one != 0) {
                const std::size_t ti = slot.command_target_plus_one - 1;
                if (ti < slots.size()) {
                    const GameUnitSlot& tgt = *slots[ti];
                    target_p[0] = tgt.motion.position[0];
                    target_p[1] = tgt.motion.position[1];
                    target_p[2] = tgt.motion.position[2];
                    // 009C6342: the fly-over lead point starts from approach->vtable[0],
                    // the fed aim point (the origin while kHullAimOffsetEnabled is
                    // false). Packet cc9_hull_turndown.
                    hull_aim_world_point(slot, tgt, ti + 1, target_p);
                    // 009FA2E0 reaches vtable[34h] on the object at
                    // approach+44h, else approach+48h; a plane slot carries its
                    // world velocity separately from the rigid body.
                    if (tgt.plane_velocity_seeded) {
                        target_v[0] = tgt.plane_world_velocity[0];
                        target_v[2] = tgt.plane_world_velocity[2];
                    } else {
                        target_v[0] = tgt.motion.linear_velocity.x;
                        target_v[2] = tgt.motion.linear_velocity.z;
                    }
                }
            }
            const float height_above = own_p[1] - target_p[1];
            slot.db_flyabove_height = height_above;
            // 009C62D1-009C63E6, BOUND (packet cc8_dive_heading): the fly-over
            // does NOT measure its range and its bearing to the aim point. It
            // measures them to where the aim point will be relative to the
            // aircraft in THREE SECONDS.
            //
            //   009C62CF  vtable[34h] on [[ESI+4]+4]        the own velocity
            //   009C62E8  009FA2E0 on approach+30h          the target velocity
            //   009C62ED/009C6305  (dx,dz) = v_own - v_tgt
            //   009C6320/009C6328  the qword 3.0 at 00D7A2B0
            //   009C6336/009C633E  3*dx, 3*dz
            //   009C6342  vtable[0] on the approach          the aim point
            //   009C6346/009C6351  aim - 3*(dx,dz)
            //   009C635D/009C636B  less the entity pose +FCh/+104h
            //   009C6385-009C63A6  R = sqrt(x*x + z*z), the 1e-10 floor at
            //                      00CE3820 collapsing it to 0
            //   009C63BF-009C63E6  bearing = wrap(pi/2 - atan2(z,x)), the +2pi
            //                      at 00CE3828
            //
            // Integrating, in 3 s the aircraft moves 3*v_own and the aim point
            // moves 3*v_tgt, so `aim - pos - 3*(v_own - v_tgt)` is exactly the
            // predicted separation. vtable[34h] is the velocity getter, the
            // same slot 009C7D71 multiplies by 007BCC80's fall time to build
            // the predicted impact point. R and the bearing move together,
            // which is why both are bound here rather than one of them.
            const float rel_vx =
                slot.plane_world_velocity[0] - target_v[0];
            const float rel_vz =
                slot.plane_world_velocity[2] - target_v[2];
            const float lead_x = target_p[0] - 3.0f * rel_vx - own_p[0];
            const float lead_z = target_p[2] - 3.0f * rel_vz - own_p[2];
            const double lead_d2 = static_cast<double>(lead_x) * lead_x +
                                   static_cast<double>(lead_z) * lead_z;
            const float lead_range =
                (lead_d2 <= bsp::dive_bomb_constant::kDistanceEpsilonSq)
                    ? 0.0f : static_cast<float>(std::sqrt(lead_d2));
            float lead_bearing = static_cast<float>(
                bsp::dive_bomb_constant::kHalfPi -
                std::atan2(static_cast<double>(lead_z),
                           static_cast<double>(lead_x)));
            if (lead_bearing < 0.0f) {
                lead_bearing +=
                    static_cast<float>(bsp::dive_bomb_constant::kTwoPi);
            }
            slot.db_flyabove_lead_range = lead_range;
            slot.db_flyabove_lead_bearing = lead_bearing;
            const float lead_bearing_error =
                bsp::wrapped_angle_subtract_00438b10(lead_bearing,
                                                     slot.plane_heading_c6c);
            // CORRECTION, packet cc8_dive_heading, edited under the
            // integrator's hunk arbitration of 2026-09-19: 009C65DB's FSUBP
            // takes the PLANAR RANGE off the stack, not the height, so the
            // span is `max(R - S, 0)`, and `R` is the three-second lead range
            // built above, not `db_planar_bc` and not the height.
            const bsp::DiveBombFlyAboveSpan span =
                bsp::dive_bomb_flyabove_span_009c65fd(height_above, lead_range);
            slot.db_flyabove_span = span.span;
            // +18h at 009C680E: dive once higher above the aim point than
            // approach+D4h, which is now the real 675.0 m.
            in.flyabove_can_dive_790 = bsp::dive_bomb_flyabove_can_dive_009c680e(
                height_above, slot.db_release_range_d4);
            slot.db_flyabove_can_dive_18 = in.flyabove_can_dive_790;
            // +19h at 009C67B0, BOUND. The first arm is the 1.6 rad bearing
            // test at 00CE3D48; the second is `span <= 0` (009C67A9 with
            // 009C67AE the byte 72, JC). RETRACTED by packet cc8_dive_heading:
            // "with the 0.7/200.0 pair that is height <= 666.7 m, the same gate
            // approach+D4h's 675.0 m expresses" was an artefact of feeding the
            // span the height. The second arm is `R <= 0.7 * max(B,100) + 200`,
            // a range-to-go test against a glide slope, and it has nothing to
            // do with +D4h.
            in.flyabove_ready_791 = bsp::dive_bomb_flyabove_roll_in_009c67b0(
                lead_bearing_error, span.span);
            slot.db_flyabove_ready_19 = in.flyabove_ready_791;
            // +1Ah at 009C66E3, BOUND: leave when the folded bearing error
            // beats a tolerance opening from 20 degrees at span 0 to pi at
            // approach+B4h * 0.8 - S. 009C66E7 clears +19h on the same edge,
            // which the transition rule already models by taking `ready` first.
            in.flyabove_leave_792 = bsp::dive_bomb_flyabove_leave_009c66e3(
                lead_bearing_error, span, slot.db_attack_dist_b4);
            // Packet cc9_dive_modes, diagnostic only: the fly-over's roll-in
            // inputs per tick, to tell 009C67B0's two arms apart.
            if (kHullAimTrace && slot.dive_bomb_state == bsp::DiveBombState::kFlyAbove) {
                const double own_v = std::sqrt(
                    static_cast<double>(slot.plane_world_velocity[0]) * slot.plane_world_velocity[0] +
                    static_cast<double>(slot.plane_world_velocity[2]) * slot.plane_world_velocity[2]);
                log.notef("fa_trace %s t=%d h=%.1f rng=%.1f lead_R=%.1f lead_brg=%.4f hdg=%.4f "
                          "brg_err=%.4f span=%.1f thr=%.1f v_planar=%.2f tv=(%.2f %.2f) "
                          "can_dive=%d ready=%d leave=%d",
                          slot.row.name.c_str(), slot.dive_bomb_arm_ticks,
                          static_cast<double>(height_above), static_cast<double>(slot.db_planar_bc),
                          static_cast<double>(lead_range), static_cast<double>(lead_bearing),
                          static_cast<double>(slot.plane_heading_c6c),
                          static_cast<double>(lead_bearing_error),
                          static_cast<double>(span.span), static_cast<double>(span.threshold),
                          own_v, static_cast<double>(target_v[0]), static_cast<double>(target_v[2]),
                          in.flyabove_can_dive_790 ? 1 : 0, in.flyabove_ready_791 ? 1 : 0,
                          in.flyabove_leave_792 ? 1 : 0);
            }
        }
        in.flyabove_turn_side_798 = 0;
        in.aimdive_alive_74d = slot.db_aim_alive_19;
        in.aimdive_pull_out_74c = slot.db_aim_pull_out_18;
        // 009C86B2/009C86BF. Packet cc8_dive_flyover, the ONE line the
        // integrator granted in this function under the hunk arbitration of
        // 2026-09-19: the aimglide state's +76Ch is now kept (009C57FF sets it,
        // 009C4F0C clears it) instead of being hardcoded false, which made the
        // aimglide terminal for any aircraft still holding a bomb.
        in.aimglide_pull_out_76c = slot.db_aimglide_pull_out_76c;
        // 009C7850: !HasGeneralBombOrdnance.
        in.aimglide_out_of_bombs = !slot.db_has_bomb_d1;
        // 009C7EA0, reconstructed.
        // 009C7EA0 reads pose+C64h first and pose+C68h second. The slot's
        // own comment makes +C64h the pitch and +C68h the bank, so the pair
        // goes in offset order, not axis-name order.
        in.turndown_complete = bsp::dive_bomb_turndown_complete_009c7ea0(
            slot.plane_pitch_angle_c64, slot.plane_bank_angle_c68);
        // 009C7F00 is PARTIAL; its first rule is d = state+20h * 0.9 against
        // the planar range, and with the travel accumulator at 0 that answers
        // as soon as the aircraft has opened at all.
        // CORRECTION: this read db_glide_travel_20, the AIMGLIDE state's +20h.
        // 009C7F00 is the goaway state's own completion rule and reads the
        // goaway state's +20h; the two are different objects. Once the aimglide
        // seed was recovered as max(arg, 5.0) the conflation made goaway finish
        // on its very first tick, which is the 1-tick goaway in every run so
        // far. The goaway's own accumulator starts at zero here, so the rule
        // stays the labelled PARTIAL it was - but it is no longer fed a value
        // that belongs to another state.
        {
            bsp::DiveBombGoAwayCompleteInputs g;
            g.planar_distance_bc = slot.db_planar_bc;
            g.travel_20 = slot.db_goaway_travel_20;
            g.altitude = slot.motion.position[1];
            // ctl+398h is what approach+ACh is refreshed from every tick, so
            // this host has one value for both.
            g.cruise_altitude_398 = slot.db_begin_alt_ac;
            g.begin_altitude_ac = slot.db_begin_alt_ac;
            g.aim_point_height_50 = slot.db_aim_point_height_50;
            g.has_bomb_ordnance_d1 = slot.db_has_bomb_d1;
            g.control_flag_369 = false;
            g.global_e17bf2 = false;
            in.goaway_complete = bsp::dive_bomb_goaway_complete_009c7f00(g);
            if (in.goaway_complete) {
                ++slot.db_goaway_complete_ticks;
            }
        }
        in.unit_bank_c68 = slot.plane_bank_angle_c68;
        in.bank_high_00ce398c = 0.0f;
        in.bank_low_00d1fbc0 = 0.0f;
        in.random_turn_side = 1;
        return in;
    }

    bsp::DiveBombAimDiveReleaseInputs dive_bomb_aimdive_inputs(GameUnitSlot& slot,
                                                               float dt) {
        // 009C58E9 counts state+1Ch down by dt.
        if (slot.db_aim_rearm_1c >= 0.0f) slot.db_aim_rearm_1c -= dt;
        bsp::DiveBombAimDiveReleaseInputs in;
        in.altitude = slot.motion.position[1];
        in.dive_altitude_a8 = slot.db_dive_alt_a8;
        in.rearm_timer_1c = slot.db_aim_rearm_1c;
        in.rearm_draw = bsp::dive_bomb_constant::kAimDiveRearmLow;
        // 009C59BA-009C5C9B, reconstructed.
        bsp::DiveBombAimErrorInputs e;
        float target_y = slot.motion.position[1];
        if (slot.command_target_plus_one != 0) {
            const std::size_t ti = slot.command_target_plus_one - 1;
            if (ti < slots.size()) {
                // 009C59CD: the aimdive height [ESP+14h] is unit+100h less
                // approach->vtable[0].out[1], the fed aim point. Packet
                // cc9_hull_turndown.
                float hp[3] = {slots[ti]->motion.position[0],
                               slots[ti]->motion.position[1],
                               slots[ti]->motion.position[2]};
                hull_aim_world_point(slot, *slots[ti], ti + 1, hp);
                target_y = hp[1];
            }
        }
        e.height_above_target = slot.motion.position[1] - target_y;
        e.dive_altitude_a8 = slot.db_dive_alt_a8;
        e.begin_altitude_ac = slot.db_begin_alt_ac;
        e.aim_point_height_50 = slot.db_aim_point_height_50;
        e.lead_at_high_5c = slot.db_lead_high_5c;
        e.gain_at_high_60 = slot.db_gain_high_60;
        // 009C5AF1 is the SAME shape as the roll input at 009C5AA3: the
        // 009C4F80 heading is arg0 at [ESP] and the bearing to the target is
        // arg1 at [ESP+4], both at the base frame 0x58 (walked forward over the
        // whole body with local/x87_walk.py, no depth conflict in this range).
        // This stood as (bearing - pose+C6Ch) here, which is off by pi for the
        // whole dive because the turndown ends inverted: cos(pi) = -1 turned
        // `along_track` into MINUS the range, which is what pinned the pitch
        // command at -1.0 for all 344 aimdive ticks of local/usn04_geo2.log.
        // The image's bearing here is drawn from the LATCHED approach+D8h/+E0h
        // point rather than the aircraft; that remains a labelled SUBSTITUTION,
        // unchanged by this fix.
        bsp::DiveBombAimHeadingInputs ahin;
        ahin.pitch_c64 = slot.plane_pitch_angle_c64;
        ahin.bank_c68 = slot.plane_bank_angle_c68;
        ahin.heading_c6c = slot.plane_heading_c6c;
        ahin.body_up_x = slot.motion.pose_row1[0];
        ahin.body_up_z = slot.motion.pose_row1[2];
        // CORRECTED, packet cc8_dive_release item 2. The aim error's two
        // geometric inputs are 009C5AF1's bearing ([ESP+18h]) and the second
        // sqrt at 009C5A40 ([ESP+5Ch]), and BOTH are measured from
        // approach+D8h/+E0h, not from the aircraft: 009C593E makes EDI the
        // approach for 009C5950/009C5960 and only 009C5966 `MOV EDI,[EBP+4]`
        // rebases it to the unit, for 009C598C/009C5999 - so the first sqrt
        // ([ESP+1Ch]) and 009C5AA3's bearing ([ESP+24h]) are the live pair and
        // this one is not. And approach+D8h is the predicted impact point the
        // approach update rewrites every tick, so this is a CCIP solution:
        // passing the live range here made the error track the range exactly.
        e.bearing_error = bsp::wrapped_angle_subtract_00438b10(
            bsp::dive_bomb_aim_heading_009c4f80(ahin), slot.db_impact_bearing_18);
        e.planar_distance = slot.db_impact_planar_5c >= 0.0f
                                ? slot.db_impact_planar_5c : slot.db_planar_bc;
        const bsp::DiveBombAimError err = bsp::dive_bomb_aim_error_009c5c9b(e);
        slot.db_aim_error_last = err.error;
        // The release gate 00CE3880 tests the magnitude, so the census keeps the
        // closest approach to it rather than only the last sample: the last one
        // is taken after the aircraft has overflown and says nothing about the
        // dive. Sampled only while the aim states own the tick.
        if (slot.dive_bomb_state == bsp::DiveBombState::kAimDive ||
            slot.dive_bomb_state == bsp::DiveBombState::kAimGlide) {
            const float mag = err.error < 0.0f ? -err.error : err.error;
            if (slot.db_aim_error_abs_min < 0.0f || mag < slot.db_aim_error_abs_min) {
                slot.db_aim_error_abs_min = mag;
                slot.db_aim_error_min_range = slot.db_planar_bc;
                slot.db_aim_error_min_alt = slot.motion.position[1];
            }
            if (slot.db_aimdive_min_range < 0.0f ||
                slot.db_planar_bc < slot.db_aimdive_min_range) {
                slot.db_aimdive_min_range = slot.db_planar_bc;
            }
            const float bearing_error = bsp::wrapped_angle_subtract_00438b10(
                slot.db_bearing_c0, slot.plane_heading_c6c);
            if (slot.db_aimdive_entry_range < 0.0f) {
                slot.db_aimdive_entry_range = slot.db_planar_bc;
                slot.db_aimdive_entry_bearing = bearing_error;
            }
            ++slot.db_aim_state_ticks;
            // One sample every 30 aim ticks, about 2.7 s.
            if (slot.db_aim_trace_samples < 12 &&
                slot.db_aim_state_ticks >= (slot.db_aim_trace_samples + 1) * 30) {
                const int i = slot.db_aim_trace_samples;
                slot.db_aim_trace_range[i] = slot.db_planar_bc;
                slot.db_aim_trace_error[i] = err.error;
                slot.db_aim_trace_bank[i] = slot.plane_bank_angle_c68;
                slot.db_aim_trace_bearing[i] = bearing_error;
                ++slot.db_aim_trace_samples;
            }
        }
        // 009C60C1 reloads [ESP+5Ch], the same slot 009C5C9B writes - but the
        // 009C5BDB gate jumps over 009C5C9B, and the last writer before it is
        // the second sqrt at 009C5A4D/009C5A58. So a dive shallower than 30
        // degrees nose-down brings the LATCHED PLANAR DISTANCE to the 25 m
        // window, not the aim error. Same condition as the steer arm below;
        // the image evaluates it once because it is one function.
        in.aim_error =
            (slot.plane_pitch_angle_c64 >
             bsp::dive_bomb_constant::kAimDiveSteepGateAngle)
                ? (slot.db_impact_planar_5c >= 0.0f ? slot.db_impact_planar_5c
                                                    : slot.db_planar_bc)
                : err.error;
        // 009C5B01-009C5B48, the dive abort: clearing +19h is what sends the
        // state to aimglide on the next transition.
        bsp::DiveBombDiveAbortInputs ab;
        ab.release_range_d4 = slot.db_release_range_d4;
        ab.aim_point_height_50 = slot.db_aim_point_height_50;
        // CORRECTED: [ESP+14h] is the height above the aim point, the same
        // quantity `e.height_above_target` above carries, not a second copy of
        // the planar range. With the range here, 009C5B3E reduced to
        // `0.3*range + 150 > range` and aborted the dive at any range under
        // 214 m - which is where usn04_geo3.log lost it, at 204.1 m.
        ab.height_above_target_14 = e.height_above_target;
        ab.aim_point_distance = slot.db_planar_bc;
        ab.unit_attitude_c64 = slot.plane_pitch_angle_c64;
        if (bsp::dive_bomb_dive_abort_009c5b43(ab)) {
            if (slot.db_abort_first_tick < 0) {
                slot.db_abort_first_tick = slot.dive_bomb_arm_ticks;
                slot.db_abort_d4 = ab.release_range_d4;
                slot.db_abort_h14 = ab.height_above_target_14;
                slot.db_abort_range = ab.aim_point_distance;
                slot.db_abort_pitch = ab.unit_attitude_c64;
            }
            ++slot.db_abort_fires;
            slot.db_aim_alive_19 = false;
            slot.db_aim_pull_out_18 = false;
        }
        return in;
    }

    bsp::DiveBombAimGlideReleaseInputs dive_bomb_aimglide_inputs(GameUnitSlot& slot,
                                                                 float dt) {
        bsp::DiveBombAimGlideReleaseInputs in;
        // 009C5188-009C51A5, packet cc8_dive_entry. The aimglide tick counts
        // state+1Ch down by its OWN dt at the top of every tick, exactly as
        // 009C58E9 does for the aimdive, and 009C519B (byte 72, JC) against the
        // 0.0f at 00D7A218 keeps it from running below zero:
        //   009c5188 MOVSS XMM0,[ESI+1Ch] / 009c518d COMISS XMM0,[00D7A218]
        //   009c519b JC 009c51a8 / 009c519d FLD [ESP+28h] / 009c51a1 FSUB dt
        //   009c51a5 FSTP [ESI+1Ch]
        // This host decremented the timer only in dive_bomb_aimdive_inputs,
        // which runs only while the state is kAimDive, so in the glide the
        // timer sat at the 0.0 its enter writes and gate 1 at 009C5689
        // (`state+1Ch < 0`) refused every call: local/entry_before.log has
        // `blocked[rearm=629 bearing=0 ceiling=1 ...] passed=0` out of 630 for
        // D3A Val #1.1, where docs/DIVE_BOMB_TASK.md's table from the glide
        // packet's own tree reads `rearm=0 bearing=523 ceiling=107`.
        if (slot.db_aim_rearm_1c >= 0.0f) slot.db_aim_rearm_1c -= dt;
        // Packet cc8_dive_glide: every slot behind 009C5689-009C5755 is now
        // traced to its producer and bound from the geometry this host already
        // keeps. The `four frame slots not traced` note is withdrawn.
        //
        // [ESP+18h], 009C5357-009C53CA. CORRECTED: this was |pitch|, and the
        // slot is not an attitude at all - it is the horizontal angle between
        // the bearing to the aim point (approach+C0h's own construction, taken
        // from the AIRCRAFT here, unlike the aimdive's, which 009C5AF1 takes
        // from the impact point) and 009C4F80's aim heading, then abs'd. The
        // wrapped subtract is even and so is the 009C5708 cosine, so the
        // argument order is immaterial to both consumers.
        in.rearm_timer_1c = slot.db_aim_rearm_1c;
        {
            bsp::DiveBombAimHeadingInputs ahin;
            ahin.pitch_c64 = slot.plane_pitch_angle_c64;
            ahin.bank_c68 = slot.plane_bank_angle_c68;
            ahin.heading_c6c = slot.plane_heading_c6c;
            ahin.body_up_x = slot.motion.pose_row1[0];
            ahin.body_up_z = slot.motion.pose_row1[2];
            const float be = bsp::wrapped_angle_subtract_00438b10(
                bsp::dive_bomb_aim_heading_009c4f80(ahin), slot.db_bearing_c0);
            in.bearing_error_18 = be < 0.0f ? -be : be;
            if (slot.db_glide_bearing_min < 0.0f ||
                in.bearing_error_18 < slot.db_glide_bearing_min) {
                slot.db_glide_bearing_min = in.bearing_error_18;
            }
            // Packet cc8_dive_flyover: the same [ESP+18h] the pull-out arm at
            // 009C57CA reads, kept so the tick's tail can test it.
            slot.db_glide_bearing_abs_18 = in.bearing_error_18;
        }
        // Both RECOVERED this packet; see include/bsp/dive_bomb_task.hpp. The
        // height is measured against the aim point, as 009C5281 builds it, and
        // the ceiling is row+4Ch times the drawn release altitude - 0.6 * 350.0
        // = 210.0, so with the 50.0 margin the glide release opens below 260 m.
        {
            float target_y = slot.motion.position[1];
            if (slot.command_target_plus_one != 0) {
                const std::size_t ti = slot.command_target_plus_one - 1;
                if (ti < slots.size()) {
                    // 009C5278: the aimglide height is taken to approach->vtable[0],
                    // the fed aim point (the origin while kHullAimOffsetEnabled is
                    // false). Packet cc9_hull_turndown.
                    float hp[3] = {slots[ti]->motion.position[0],
                                   slots[ti]->motion.position[1],
                                   slots[ti]->motion.position[2]};
                    hull_aim_world_point(slot, *slots[ti], ti + 1, hp);
                    target_y = hp[1];
                }
            }
            in.height_above_aim_point = slot.motion.position[1] - target_y;
        }
        in.glide_release_ceiling =
            dive_bomb_row(slot).new_release_mul_04c * slot.db_dive_alt_a8;
        // CORRECTED: both of these were slot.db_planar_bc, one quantity standing
        // in for two different ones, which made 009C56C2's gate `0 < 120` and
        // 009C5704's lead `range * (1 - cos)`, never negative - so the salvo at
        // 009C5777 could not fire at any altitude, angle or travel. [ESP+14h]
        // is the throw to the predicted impact point and [ESP+10h] is the range
        // to the aim point; see the header for the three-vector walk.
        in.lateral_a = slot.db_impact_throw_14;
        in.lateral_b = slot.db_planar_bc;
        in.travel_accumulator_20 = slot.db_glide_travel_20;
        // The cap is EBP, the literal 2 at 009C53DD - a proof, not the
        // kDiveBombCarriedRoundsSubstitute the two happened to share.
        in.rounds_cap = bsp::dive_bomb_constant::kGlideSalvoCap;
        in.rearm_draw = bsp::dive_bomb_constant::kAimGlideRearmLow;
        // RECOVERED, packet cc8_dive_glide - this was a labelled 0.0f, and a
        // zero here freezes the release window at the 009C4F50 seed. A scan of
        // 009C3E00-009CA000 for `[reg+0A4h]` returns exactly one writer among
        // six touches, 009C3F16 in the constructor FUN_009C3EA0:
        //   009C3EF4 MOV ECX,[ESI+8]        ; the plane class descriptor
        //   009C3EF7 FLD  [ECX+188h]        ; MaxSpd, already named in this repo
        //   009C3F00 FMUL qword [00CEFFB0]  ; 0.95
        //   009C3F16 FSTP [ESI+0A4h]
        // so approach+A4h is 0.95 * MaxSpd, set once and never rewritten. The
        // three following instructions settle approach+A8h in the same breath:
        // 009C3F1C/009C3F23 push (approach+14h)->+3Ch and ->+38h into
        // BSP_Random_UniformFloatRange and 009C3F2E stores the draw, which is
        // the authored uniform(350, 450) this host pins at 350.
        in.drift_rate_a4 = static_cast<float>(
            bsp::dive_bomb_constant::kApproachDriftScaleA4) * slot.plane_max_spd;
        return in;
    }

    // Packet cc8_dive_glide. The dive-bomb twin of release_ordnance_007bbba0
    // above, kept separate rather than sharing it: that one spawns through
    // release_ordnance_drop, which selects torpedo rows and clears kind 2Bh, and
    // a bomb needs the kind 2Ah selection instead. The bay-open command itself
    // is the same 007BBBA0 the image calls at 009C60F1 and at 009C5777.
    //
    // The point handed to the spawn is slot.db_run_in_origin, which is
    // approach+D8h/+DCh/+E0h and which 009C7A80 has already rewritten this tick
    // through 009C7D71 - the predicted impact point of a bomb released now. So
    // it is the prediction to score the round against, not a latched origin.
    void release_bomb_007bbba0(GameUnitSlot& slot, int rounds) {
        if (!slot.actuator_block_dec.channel_c.enabled) {
            slot.actuator_block_dec.channel_c.enabled = true;
            slot.actuator_block_dec.channel_c.rate = 1.0f;
        }
        // The bay request is raised ONCE per release even when the salvo is two
        // rounds, and that is not a shortcut. 007BBBA0 is a latch: 007BBBB2
        // leaves when channel C is already at the top, so the image's own
        // 009C5771-009C5784 loop gets one acceptance and one refusal from a
        // two-round salvo. The bay is a bay; what the image counts per round is
        // approach+2Ch, which the loop decrements whether or not 007BBBA0
        // accepted - the call's result is never tested at 009C5777.
        const bool accepted =
            bsp::ordnance_release_request_007bbba0(slot.actuator_block_dec);
        done("Unit::request_ordnance_release_007bbba0", 0x007bbba0u);
        if (accepted) {
            ++slot.db_bay_requests_accepted;
        } else {
            ++slot.db_bay_requests_refused;
        }
        // SUBSTITUTION, labelled, and the reason the spawn does not hang off
        // `accepted`: the image's chain from the bay to a round is 007BBBA0 ->
        // unit+C20h -> 007CE040 -> 007C0D90 -> 007EEF30 -> 007BCBE0 ->
        // unit+C58h, and the bot task's release slot at the far end is UNREAD.
        // Tying the spawn to the latch would drop the second round of every
        // salvo, which is a property of the bay, not of the ordnance. One round
        // per counted round is the smallest model that keeps the count the
        // image keeps.
        if (gunnery == nullptr) return;
        const std::size_t index = index_of_slot(slot);
        if (index >= slots.size()) return;
        for (int i = 0; i < rounds; ++i) {
            // Packet cc8_dive_aim item 2, edited under the integrator's hunk
            // arbitration of 2026-09-19: db_impact_fall_time is 009C7D71's tf
            // as of THIS tick, so carrying it here is the one sample that is
            // provably a release-tick one.
            if (!gunnery->release_bomb_drop(index, slot.db_run_in_origin,
                                            slot.db_impact_fall_time)) break;
            ++slot.db_bombs_spawned;
            // NO decrement of dive_bomb_rounds_remaining here. The task's own
            // `spend_round` already does it, and a second one took the stock
            // from 2 to 0 on the first bomb: db_has_bomb_d1 went false, the
            // aimdive ended 15 ticks early and `movieval` dropped from
            // releases=2 to releases=1. Measured in local\bomb_spawn.log before
            // this line came out.
        }
    }

    // `rounds` is 1 for the aimdive, whose 009C60F1 raises one request per gate,
    // and r.rounds_released for the aimglide, whose 009C5771-009C5784 loop calls
    // 007BBBA0 once per round of the salvo.
    void note_dive_bomb_release(GameUnitSlot& slot, int rounds = 1) {
        ++slot.dive_bomb_releases;
        // Packet cc8_dive_glide: the release was only ever COUNTED here.
        release_bomb_007bbba0(slot, rounds);
        if (slot.db_release_alt < 0.0f) {
            slot.db_release_alt = slot.motion.position[1];
            slot.db_release_speed = slot.plane_travel_speed;
            slot.db_release_range = slot.db_planar_bc;
        }
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

    // 009BFD70, the first half of the follow tick 009C1FD0. The plane wing's
    // formation, which is NOT the ship unit group: see docs/PLANE_FORMATION.md
    // and the header note on why 0077C8D0 cannot reach a plane.
    //
    // WHAT IS BOUND: 009BFD70 takes the squadron from `[[task+4]+0Ch]`, refuses
    // when the unit IS the flight leader (009BFD92 `CMP EAX,[ECX+4]` / JZ
    // 009BFEB1), reads the unit's own `plane+9D0h` and asks 007F23A0 for the
    // station (009BFDBE-009BFDCC). 007ED260 and 007F23A0 are reconstructed in
    // src/plane_formation.cpp from the listing.
    //
    // WHAT IS NOT, stated as a hole rather than dressed as a proof: 009BFEE0
    // (1795 instructions, 009BFEE0-009C1846, 55 calls) is the law that FLIES a
    // member to its station - the throttle, the heading and the GoodPosition /
    // WaitForHdg gates of the whole `Pilot/Follow/*` tuning block. It is not
    // reconstructed. What runs here instead PLACES the member on its station.
    // The geometry is the image's; the path by which a member reaches it is not.
    //
    // The switch below exists so that the BEFORE half of this packet's
    // measurement is the same binary as the AFTER apart from the placement: the
    // station and the wing-geometry report are produced either way. It ships
    // true.
    // Packet `cc8_follow_regimes` replaces the placement with the law itself.
    // 009BFEE0's steer-point producer is now read far enough to fly a member:
    // the LEAD-PURSUIT regime (009C107B-009C123C) end to end, the abeam regime
    // (009C15C0-009C16CF), the 009C1328 regime and the tail's altitude band,
    // all in src/plane_follow_law.cpp against 009BEE30's already-bound fly-to
    // arm.  TWO SUBSTITUTIONS remain and they are named, not hidden:
    //
    //   * the REGIME SELECTOR is an output of the unread Phase A
    //     (009C0251-009C0EE0), because BL is rewritten there - see the header.
    //     This host always takes lead pursuit, whose D -> 0 limit is
    //     `station + FollowedPointDist * U`, i.e. steering along the leader's
    //     lagged track at the station, which is formation flight.
    //   * the GOOD-POSITION gate selects 009BEE30's hold arm
    //     (009BEE56-009BF9E5, ~700 instructions, unread), so this host runs the
    //     fly-to arm unconditionally.  At d_station = 0 that arm's own ramps
    //     return the leader's speed and stationY, so this is the fly-to law's
    //     limit standing in for the hold law, not an invented hold law.
    //
    // MEASURED 2026-09-19, and the result is a NULL that must not be read as a
    // success.  With placement false and the law true, USN04 gave mean wing
    // pairwise separation 2.5 m against the placement's 213.3 m - which looks
    // like a formation holding tightly and is nothing of the kind.  The law
    // never ran: `follow law` appears ZERO times in local/after_follow_law.log,
    // and the last `plane formation geometry` line is still the spawn clump
    // (pairwise 0.00 / 3.46 / 3.46).  The reason is three comments below at the
    // `once` gate: this host NEVER ENTERS the follow state - every plane of
    // every USN04 squadron reports `states[attackrun=...]` with
    // `prepare_entries=0` - so 009C1FD0's tick, where the law is wired, does
    // not execute, while the placement that DID run was the member's first
    // step.  Turning placement off therefore removed the only thing that put a
    // member on its station and replaced it with nothing.
    //
    // So the two flags are NOT a pair to alternate yet.  Placement stays true
    // because it is the only station-keeping this host actually reaches; the
    // law stays true because it is correct where it is wired and costs nothing
    // until 009C1FD0 becomes reachable.  The blocker for a real before/after is
    // no longer the reading of 009BFEE0 - it is that nothing in this host
    // enters BotStateFollow.  See docs/HANDOFF_PLANE_FOLLOW_REGIMES.md.
    static constexpr bool kPlaneFormationPlacementEnabled = true;
    static constexpr bool kPlaneFollowLawEnabled = true;
    // The fly-to arm's last two plan stores, 009BFD15 plan+2B0h = 0 and
    // 009BFD1C plan+2D8h = 1, after the desired speed at 009BFD0F. Without
    // +2D8h = 1 an airborne plane (flight state 7) never enters 0099D300's
    // speed-demand arm, so the follow law's desired speed was never read.
    // Packet cc9_follow_speed, docs/PLANE_FOLLOW_SPEED.md.
    static constexpr bool kPlaneFollowFlyToSpeedStores = true;
    // The fly-to arm's pitch through 009F9ED0 (packet cc9_follow_pitch) rather
    // than the 009FB800 substitute, which this binding fed the commanded
    // altitude as its dimensionless `reference` and so ran bang-bang at the
    // +0.698 / -1.047 caps. docs/PLANE_FOLLOW_PITCH.md.
    static constexpr bool kPlaneFollowFlyToPitch = true;
    // The pitch mode 009FB800 writes (cmd+2D0h = 2) at the three 009FBA50 seams
    // that left it out. Packet cc9_pitch_callers, docs/PITCH_COMMAND_CALLERS.md.
    static constexpr bool kPitchCommandCallersBound = true;
    // The dogfight task's skeleton (packet cc9_dogfight_task): install, the
    // 009AAFA0 unengaged arm (moveto leader / follow wing), the generic follow
    // tick, and a labelled moveto stand-in. docs/DOGFIGHT_TASK.md.
    static constexpr bool kDogfightTaskBound = true;
    // Packet cc9_dogfight_engaged: 009AAC70's selection and latch, all of
    // 009AAFA0, the aim state, and labelled maneuver/avoid stand-ins.
    // docs/DOGFIGHT_ENGAGED.md.
    static constexpr bool kDogfightEngagedBound = true;
    // Packet cc9_dogfight_gun: the task gun controller 009FC7C0 (fire decision
    // and burst clock, census only: the gunFire consumer is not established),
    // the head-on and maneuver throttle 007B4ED0. docs/DOGFIGHT_GUN.md.
    // Packet cc9_plane_gunfire: 0099B450's per-think re-seed of plan+2B4h,
    // +2B0h and +2D8h. docs/PLANE_GUNFIRE.md.
    static constexpr bool kPilotPlanReseedBound = true;
    static constexpr bool kDogfightGunBound = true;
    // Packet cc9_plane_gunfire: the unit+C50h enemy-aircraft list (007E11D0,
    // 3 s refresh) and its finder 007E2090/007DEEC0 as the gun's +74h target.
    static constexpr bool kPlaneFinderBound = true;
    // Packet cc9_near_field_probe: 007F0280 at the dive-bomb attack run
    // (009C42B8) and the fly-over speed slot (009C6B3A). docs/NEAR_FIELD_PROBE.md.
    static constexpr bool kNearFieldProbeBound = true;
    // Packet cc9_plane_gun_pass: 009AAA80's early maneuver -> aim edge, and the
    // latched gunFire (unit+BC9h) the plane's fixed step hands to every gun's
    // SetTriggerHeld (007CE9A0-007CE9F4). docs/PLANE_GUN_PASS.md.
    static constexpr bool kDogfightEarlyEdgeBound = true;
    static constexpr bool kPlaneGunfireBound = true;
    // Packet cc9_plane_flight_natives. docs/PLANE_FLIGHT_NATIVES.md.
    // The aim tick's head-on test reads vtable[34h] = 007BBB70, unit+AC8h, the
    // world linear velocity, for both aircraft (not the forward rows).
    static constexpr bool kDogfightHeadOnVelocityBound = true;
    // 007CE9FD's release-issue predicates: session mode, +C3Ah, +5Dh, BombDelay.
    static constexpr bool kPlaneFixedStepPredicatesBound = true;
    // Packet cc9_plane_death_modes: 007CA8A0's death mode, 007CAF10's dead-step
    // terms, the kill that takes the aircraft out of the world, and the release
    // refusal of a dead aircraft (007CEA1C). docs/PLANE_DEATH_MODES.md.
    static constexpr bool kPlaneDeathModesBound = true;
    // 007DA710 (free-flight arm, read) and 007BB920's air-brake override.
    static constexpr bool kPlaneControlRateLawBound = true;
    static constexpr bool kPlaneCommitCommandBound = true;
    // Packet cc9_dogfight_moveto: the dogfight moveto's speed slot 009C1BC0
    // (009BECD0 / 007EF2C0 / 009BE3E0), and PilotFires (unit+C24h, 007CD930)
    // loaded from the authored Platforms[].PilotFires. docs/DOGFIGHT_MOVETO.md.
    static constexpr bool kDogfightMovetoSpeedBound = true;
    // Packet cc9_dive_modes: the dogfight moveto runs the generic 009C18C0
    // glide with its fixed ranges (500, 100, 1000) instead of the stand-in.
    // OFF, measured (docs/DIVE_MODES.md 5): with the throttle fix the leader holds
    // 1475 m / 82.2 m/s (was 1547 m / 34 m/s), but on main alone (local/G_9000.log)
    // it moves 46 dive-bomb rows through the fighters, not yet explained.
    static constexpr bool kDogfightMovetoGenericBound = false;
    static constexpr bool kPilotFiresBound = true;
    // Packet cc9_throttle_slot: a per-think trace of the throttle slot for the
    // Yorktown flight (dogfight) and D3A Val #1.1 (dive-bomb). Diagnostic, off.
    static constexpr bool kThrottleSlotTrace = false;
    // Packet cc9_throttle_slot: 0099D300's speed-hold multiplier is dt in speed
    // mode (not the slot's pending distance). docs/PILOT_THROTTLE_SLOT.md.
    // OFF, measured: USN04 bomb drops 29 -> 0 (local/S0_9000.log vs
    // local/S1_9000.log). The speed hold now works in flyabove and turndown and
    // cuts the throttle to 0.001; aimdive (kAimDiveTailBound off) and goaway
    // (009C4C0C-009C4CA7 unmodelled) write no throttle in this host, so the Val
    // dives unpowered at 53 m/s instead of 134. Binding the aimdive tail too
    // (local/S1A_9000.log) still drops nothing. Bind those writers first.
    static constexpr bool kPilotThrottleSlotBound = false;
    // 007B4ED0 wiring (the head-on arm and the maneuver tail's full throttle).
    // OFF, measured: run F1 drowned Yorktown-class01_sqn02 and its .-2 at |v| 51
    // after one maneuver tick left the throttle slot active in mode 0. The image
    // re-seeds the command block every think (0099B4E8); this host does not, so
    // the write outlives the state. docs/DOGFIGHT_GUN.md section 6.
    // Packet cc9_throttle_slot corrects that cause: the stall was the speed-hold
    // multiplier (kPilotThrottleSlotBound). With both on (local/S1T_9000.log) the
    // pair no longer stalls but still reaches the water at |v| 100-106, in aim,
    // chasing Vals that the same switch sends gliding into the sea. Still OFF.
    static constexpr bool kDogfightThrottleBound = false;
    // Packet cc9_aimglide_pitch: approach+A8h is Uniform(row+38h, row+3Ch) drawn
    // by 00BD2F10 on stream ECX = 1 in the approach constructor 009C3EA0
    // (009C3F09-009C3F2E), once per dive-bomb task. This host pinned it to the
    // low end. SUBSTITUTION, labelled: the image's stream is the one process-wide
    // generator the gunnery host models, but that generator is private to
    // src/game_hosts_gunnery.cpp, so this draws from a copy of its algorithm and
    // seed (one sequence across these draws). With BSP_GUNNERY_RNG_STREAMS=1 the
    // draw is keyed per unit name instead, as the gunnery option keys its draws.
    // OFF with kAimGlidePitchBound (docs/AIMGLIDE_PITCH.md section 5).
    static constexpr bool kReleaseAltitudeDrawBound = false;
    std::uint32_t db_release_rng{0x9E3779B9u};
    std::map<std::uint64_t, std::uint32_t> db_release_rng_by_key;
    static bool release_rng_streams_enabled() {
        static const bool on = [] {
            char* text = nullptr;
            std::size_t bytes = 0;
            bool value = false;
            if (_dupenv_s(&text, &bytes, "BSP_GUNNERY_RNG_STREAMS") == 0 && text != nullptr)
                value = text[0] == '1';
            std::free(text);
            return value;
        }();
        return on;
    }
    float release_altitude_draw_00bd2f10(const std::string& unit_name, float low, float high) {
        std::uint32_t* state = &db_release_rng;
        if (release_rng_streams_enabled()) {
            std::uint64_t key = 0xcbf29ce484222325ull;   // FNV-1a of the name
            for (const char ch : unit_name) {
                key = (key ^ static_cast<unsigned char>(ch)) * 0x100000001b3ull;
            }
            auto it = db_release_rng_by_key.find(key);
            if (it == db_release_rng_by_key.end()) {
                std::uint64_t z = key + 0x9E3779B97F4A7C15ull;
                z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
                z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
                z ^= z >> 31;
                it = db_release_rng_by_key.emplace(key, static_cast<std::uint32_t>(z)).first;
            }
            state = &it->second;
        }
        *state = *state * 1664525u + 1013904223u;
        const float unit = static_cast<float>((*state >> 8) & 0xFFFFFFu)
            / static_cast<float>(0x1000000u);
        return low + (high - low) * unit;
    }
    // Diagnostic period for the `follow trace` row below; 0 compiles it out.
    // Packet cc9_follow_speed ran it at 25 (docs/PLANE_FOLLOW_SPEED.md section 5).
    static constexpr int kFollowTraceEvery = 0;
    // 007B8AD0, four instructions at 007b8ad0-007b8adb, bytes
    // `33 c0 39 81 d8 09 00 00 0f 94 c0 c3`:
    //   XOR EAX,EAX / CMP [ECX+9D8h],EAX / SETE AL / RET
    // `__fastcall(ECX = unit) -> bool`, i.e. `return unit->+9D8h == 0`.
    //
    // CORRECTED, packet cc8_follow_enter. The ledger name
    // `BSP_Unit_LacksFollowTarget` and docs/BOT_TASK_STATES.md read `+9D8h` as
    // a follow-target pointer, with `contract: unread` on the writer. It is not
    // a pointer. Every writer stores an INDEX - a census of both store forms
    // (`89 ?? d8 09 00 00` -> seven sites, `c7 ?? d8 09 00 00` -> two, and
    // those two are in FUN_00A414D0/FUN_00A415B0, far outside unit code):
    //   007CFE72  BSP_PlaneUnitInstance_Construct stamps -1; 007CFD69 is
    //             `OR EDI,0FFFFFFFFh` and EDI is written nowhere between that
    //             and the store (whole-listing filter, and EDI is non-volatile
    //             across the intervening calls)
    //   007F4B43  the spawn attach stamps squadron+3CCh, the count before the
    //             append; 007CDF7C and 007ED220 are two further append helpers
    //             carrying the same +9D4h=squadron / +9D8h=index pair
    //   007ED292  BSP_PlaneSquadron_AssignFormationIndices rewrites it to the
    //             live walk index, and 007F4BFA runs that in the spawn tail, so
    //             it equals the array position from the first assignment on
    // 007ED610 BSP_PlaneSquadron_PromoteFlightLeader rotates the promoted
    // member to the FRONT of +3D0h and only then re-indexes, which is what
    // makes slot 0 mean "leader". So the predicate asks "am I my squadron's
    // flight leader": the leader takes moveto (009C777E -> +4F0h,
    // 009D30BE -> +544h) and every wing member takes follow (+52Ch, +580h).
    //
    // SUBSTITUTION, labelled: a unit with no squadron record answers TRUE, the
    // leader answer, which is this host's previous hardcode. The image's own
    // no-squadron state is the constructor's -1, which is NOT slot 0 and would
    // take follow - but "this host's registry has no record" is not evidence of
    // "the image would have left -1 here". The registry holds the planes spawned
    // through PlaneSquadronGen/007F4580 only, and USN04 registers one squadron
    // of three; every other aircraft in the mission reaches a task by a route
    // this host does not model as a squadron, so answering false for them would
    // put aircraft whose membership is simply unknown into a follow state that
    // 009BFD70 then declines to produce a station for - commanded nothing, on no
    // evidence. Keeping the leader answer confines this packet's change to the
    // aircraft whose slot the host actually knows, which is what makes the
    // before/after separable.
    bool unit_is_flight_leader_007b8ad0(std::size_t process_index) const {
        const bsp::PlaneSquadronHostRecord* const sqn =
            bsp::plane_squadron_registry().find_by_member_unit(process_index);
        if (sqn == nullptr) return true;
        for (const std::size_t member : sqn->member_units) {
            if (member == bsp::kPlaneSquadronNoUnit) continue;
            return member == process_index;   // +3D0h[0], the flight leader
        }
        return true;   // a record with no live member: the same unknown case
    }

    // `apply_position` false computes the station and hands it to the caller
    // WITHOUT teleporting the member onto it. Packet cc8_follow_attack: the
    // teleport writes `motion.position` only, while `plane_world_velocity` goes
    // on integrating from its own accelerations in the plane step
    // (`plane_world_velocity[i] += world_accel[i] * step`), so a member held
    // in follow by placement carries a velocity that never had to match the
    // motion the teleport displayed. The fly-over inherits that velocity at the
    // hand-over. docs/FOLLOWER_ATTACK_HANDOVER.md section 7.
    bool place_wing_member_on_station_007f23a0(
        GameUnitSlot& unit, bool once,
        bsp::PlaneFormationStation* station_out = nullptr,
        const GameUnitSlot** leader_out = nullptr,
        bool apply_position = true) {
        bsp::PlaneSquadronHostRecord* const squadron =
            bsp::plane_squadron_registry().find_by_member_unit(unit.process_index);
        if (squadron == nullptr) return false;
        // +3D0h in array order, skipping slots whose plane never became a unit,
        // which is live_count()'s own rule.
        std::vector<std::size_t> wing;
        for (const std::size_t member : squadron->member_units) {
            if (member != bsp::kPlaneSquadronNoUnit) wing.push_back(member);
        }
        if (wing.size() < 2u) return false;
        // 007EDA91 reads slot 0 whatever the count, and 009BFD92 refuses when
        // the leader is the unit itself.
        if (wing.front() == unit.process_index) return false;

        // 007ED260. `tools/callsite_census.py` gives it nine call sites where
        // `ghidra callers` gives four, and the one that decides the schedule is
        // 007F4BFA inside the spawn tail 007F4580, just past the 007F4B43
        // attach: the image assigns the indices AT SPAWN, and again on every
        // promote (007ED645), leave (007F3A11) and follow entry (009BEDE4).
        // This host runs the same rule once per squadron at the member's first
        // step, because at 007F4580 time its members are still plans with no
        // unit and no pose to transform against.
        if (!squadron->formation_indices_assigned
            || squadron->member_formation_index.size() != wing.size()) {
            squadron->member_formation_index.assign(wing.size(), 0);
            std::vector<std::int32_t> stamps(wing.size(), 0);
            bsp::plane_formation_assign_indices_007ed260(
                squadron->member_formation_index.data(), stamps.data(),
                static_cast<int>(wing.size()));
            squadron->formation_indices_assigned = true;
            std::string indices;
            for (std::size_t i = 0; i < wing.size(); ++i) {
                if (i != 0) indices += " ";
                indices += std::to_string(squadron->member_formation_index[i]);
            }
            log.notef("plane formation: squadron %s assigned 007ED260 indices [%s] over "
                "%zu live members; shape +3E4h=%d morale +3E8h=%.3f",
                squadron->name.c_str(), indices.c_str(), wing.size(),
                static_cast<int>(squadron->formation_shape_3e4),
                static_cast<double>(squadron->morale_3e8));
        }
        std::size_t seat = wing.size();
        for (std::size_t i = 0; i < wing.size(); ++i) {
            if (wing[i] == unit.process_index) { seat = i; break; }
        }
        if (seat >= squadron->member_formation_index.size()) return false;

        const std::size_t leader_index = wing.front();
        if (leader_index >= slots.size() || slots[leader_index] == nullptr) return false;
        const GameUnitSlot& leader = *slots[leader_index];

        // The triple is the authored one or nothing: without the installation's
        // own `Pilot/Follow/*` block there is no spacing to place anyone at, and
        // inventing one is exactly what this packet must not do.
        if (!lua.plane_globals_loaded()) return false;
        const bsp::GameTuningBlock& tuning = lua.plane_globals();

        bsp::PlaneFormationStationInputs in;
        in.formation_index = squadron->member_formation_index[seat];
        in.shape = squadron->formation_shape_3e4;
        in.morale = squadron->morale_3e8;
        // 007F23F0 `84 c0 75 6c` and 007F2403 `84 c0 75 59`, both JNZ into the
        // bomber arm: the LEADER's class chain decides for the whole wing.
        // `unit_is_kind_of` is the same 0074E400 model slot 5Ch uses.
        const bool bomber = bsp::plane_formation_uses_bomber_displacement(
            bsp::unit_is_kind_of(leader.class_id, 0x10),
            bsp::unit_is_kind_of(leader.class_id, 0x14));
        in.displacement[0] = bomber ? tuning.pilot_follow_bomber_displacement
                                    : tuning.pilot_follow_small_plane_displacement;
        in.displacement[1] = bomber ? tuning.pilot_follow_bomber_displacement_2
                                    : tuning.pilot_follow_small_plane_displacement_2;
        in.displacement[2] = bomber ? tuning.pilot_follow_bomber_displacement_3
                                    : tuning.pilot_follow_small_plane_displacement_3;
        in.symmetrical_position = tuning.pilot_follow_symmetrical_position;
        in.symmetrical_altitude = tuning.pilot_follow_symmetrical_altitude;
        // `[[squadron+35Ch]+A4h]`: squadron+35Ch is the plane class descriptor
        // and class+A4h is the Lua `Width` this host already carries.
        in.leader_class_width = leader.class_width_00a4;
        // The small-plane arm transforms by the leader's whole world matrix,
        // leader+0CCh (007F2456); the bomber arm rebuilds a yaw-only frame from
        // `leader->vtable[50h]()` with the same translation (007F24AB,
        // 007F24E4-007F2508). This host has the leader's published pose, which
        // IS leader+0CCh; for a bomber leader the roll and pitch rows are
        // therefore carried where the image would have flattened them, and that
        // difference is labelled, not hidden. On USN04 the leaders are
        // TorpedoBombers (11h), which take the small-plane arm, so it does not
        // arise there.
        in.leader_frame = leader.world;

        const bsp::PlaneFormationStation station =
            bsp::plane_formation_station_007f23a0(in);
        if (!station.produced) return false;
        // Handed out before the placement gate below, so the follow law can use
        // the station whether or not the placement that used to stand in for it
        // is enabled.
        if (station_out != nullptr) *station_out = station;
        if (leader_out != nullptr) *leader_out = &leader;

        // Reporting, from the first non-leader seat only so one squadron gives
        // one line. This runs whether or not the placement below is enabled,
        // which is what makes a before/after on the same binary possible.
        if (seat == 1u) {
            const std::int32_t tick = squadron->formation_report_ticks++;
            if (tick == 0 || (tick % 400) == 0) {
                std::string pairs;
                for (std::size_t a = 0; a < wing.size(); ++a) {
                    for (std::size_t b = a + 1u; b < wing.size(); ++b) {
                        if (wing[a] >= slots.size() || wing[b] >= slots.size()) continue;
                        const GameUnitSlot& ua = *slots[wing[a]];
                        const GameUnitSlot& ub = *slots[wing[b]];
                        float d2 = 0.0f;
                        for (int i = 0; i < 3; ++i) {
                            const float dv = ua.motion.position[i] - ub.motion.position[i];
                            d2 += dv * dv;
                        }
                        if (!pairs.empty()) pairs += " ";
                        pairs += std::to_string(a) + "-" + std::to_string(b) + "=";
                        pairs += std::to_string(static_cast<double>(std::sqrt(d2)));
                    }
                }
                log.notef("plane formation geometry: squadron %s tick=%d wing=%zu "
                    "pairwise=[%s] seat1 index=%d local=(%.1f %.1f %.1f) disp=(%.1f %.1f %.1f) "
                    "bomber_triple=%d",
                    squadron->name.c_str(), static_cast<int>(tick), wing.size(),
                    pairs.c_str(), in.formation_index,
                    static_cast<double>(station.local[0]),
                    static_cast<double>(station.local[1]),
                    static_cast<double>(station.local[2]),
                    static_cast<double>(in.displacement[0]),
                    static_cast<double>(in.displacement[1]),
                    static_cast<double>(in.displacement[2]),
                    bomber ? 1 : 0);
            }
        }

        // The placement that stands in for 009BFEE0. `false` here is the BEFORE
        // half of this packet's measurement and is not a shipped configuration.
        if (!kPlaneFormationPlacementEnabled) return false;
        // Packet cc8_follow_attack: the station and the leader are already in
        // the out-params above, so the follow law still gets its geometry.
        if (!apply_position) return false;
        if (once) {
            // The image separates the wing inside the follow state and holds it
            // there. This host never enters that state - every plane of every
            // USN04 squadron reports `states[attackrun=...]` and
            // `prepare_entries=0` - so the only moment left is the member's
            // first step. Applied once, the member then flies its own run from
            // its own station, which is the geometry the run-in starts from.
            if (squadron->member_station_applied.size() != wing.size()) {
                squadron->member_station_applied.assign(wing.size(), 0u);
            }
            if (squadron->member_station_applied[seat] != 0u) return false;
            squadron->member_station_applied[seat] = 1u;
        }
        for (int i = 0; i < 3; ++i) unit.motion.position[i] = station.world[i];
        publish_pose(unit);
        return true;
    }

    void refresh_row(GameUnitSlot& slot);
    void issue_into_ring(GameUnitSlot& slot, float throttle, float rudder);

    // The heading 00835ac0 latches. The unit virtual at primary slot 50h is a
    // RET 0 getter with no reconstruction, so this is the executable's own
    // value: atan2 over pose row 2, the same convention the trajectory dump and
    // the run log print in degrees.
    // 007C47F0(approach+8h): tuning+24Ch Dynamics/SpdMultipliers/LevelFlight
    // times classDesc+184h StallSpd. Both halves are real here: the tuning row
    // comes from the PlaneGlobals mirror and the stall speed from the unit's own
    // vehicle-class row, which src/game_hosts_lua.cpp loads and the free-flight
    // arm already reads at 007DB760. docs/BOT_SPEED_CLASS_ROWS.md.
    // Packet cc9_dogfight_moveto: VehicleClass[id].BSPPilotFires = 1 when any
    // platform with at least one gun authors PilotFires = true, else 0.
    bool pilot_fires_flattened{false};
    void ensure_pilot_fires_flattened() {
        if (pilot_fires_flattened) return;
        pilot_fires_flattened = true;
        static const char chunk[] =
            "if type(VehicleClass) == 'table' then\n"
            "  for id, row in pairs(VehicleClass) do\n"
            "    if type(row) == 'table' then\n"
            "      local pf = 0\n"
            "      if type(row.Platforms) == 'table' then\n"
            "        for k = 1, 64 do\n"
            "          local p = row.Platforms[k]\n"
            "          if type(p) == 'table' and type(p.Gun) == 'table'\n"
            "             and type(p.Gun[1]) == 'number' and p.PilotFires == true then\n"
            "            pf = 1\n"
            "          end\n"
            "        end\n"
            "      end\n"
            "      row.BSPPilotFires = pf\n"
            "    end\n"
            "  end\n"
            "end\n";
        if (lua.luaL_loadbuffer(chunk, static_cast<int>(sizeof(chunk) - 1), "bsp_pilot_fires") != 0) {
            log.note("pilot fires: flatten chunk did not compile; PilotFires stays true");
            lua.lua_settop(-2);
            return;
        }
        if (lua.lua_pcall(0, 0, 0) != 0) {
            log.note("pilot fires: flatten chunk failed; PilotFires stays true");
            lua.lua_settop(-2);
            return;
        }
        done("Plane::compute_pilot_fires_007cd930", 0x007cd930u);
    }

    float bot_desired_speed_007c47f0(const GameUnitSlot& slot) const {
        float level_flight = 1.8f;                    // tuning+24Ch
        if (lua.plane_globals_loaded()) {
            level_flight = lua.plane_globals().dynamics_spd_multipliers_level_flight;
        }
        const float stall = slot.plane_stall_spd > 0.0f ? slot.plane_stall_spd : 17.5f;
        return level_flight * stall;
    }

    // Hoisted out of the dive-bomb arm class, packet cc9_follow_package, so
    // that the torpedo seam (follow_base_tick_009c1fd0, 009D2731) CAN call it:
    // 009D2731 reaches 009C1FD0 and so 009BEE30 exactly as the dive-bomb
    // follow state does (docs/PLANE_FOLLOW_HOLD_ARM.md section 11). The torpedo
    // seam does not call it yet. Text move only; behaviour unchanged.
    // 009C2068 CALL 009BFEE0 then 009C2077 CALL 009BEE30, in
    // that order and with the station 009BFD70 produced, which
    // is the tick's own order (section 2 of
    // docs/PLANE_FOLLOW_LAW.md).
    void run_follow_law_009bfee0_009bee30(
        GameUnitSlot& unit,
        const bsp::PlaneFormationStation& station,
        const GameUnitSlot& leader) {
        if (!lua.plane_globals_loaded()) return;
        const bsp::GameTuningBlock& gt = lua.plane_globals();

        bsp::PlaneFollowGeometryInputs gin;
        for (int i = 0; i < 3; ++i) {
            gin.own_pos[i] = unit.motion.position[i];
            gin.station[i] = station.world[i];
        }
        gin.own_heading = unit.plane_heading_c6c;
        gin.leader_heading = leader.plane_heading_c6c;
        // CameraMatrix is the image's 16-float order, so row 2
        // at leader+0ECh is the forward basis and row 3 at
        // leader+0FCh the translation - the same two rows
        // 009C0F0D and 009C16EB read.
        for (int i = 0; i < 3; ++i) {
            gin.leader_forward[i] = leader.motion.pose_row2[i];
            gin.leader_pos[i] = leader.motion.position[i];
        }
        // SUBSTITUTION, labelled: 007D7DA0(leader+0AB0h) at
        // 009C0109 is the leader's turn rate and its body is
        // unread.  Zero makes `ref` the leader's instantaneous
        // heading, which is this law's own zero-turn-rate limit;
        // it costs the lag only while the leader is turning.
        gin.leader_turn_rate = 0.0f;
        gin.followed_point_dist = gt.pilot_follow_followed_point_dist;
        gin.leader_heading_time_1 =
            gt.pilot_follow_leader_heading_spd_time_1;
        gin.leader_heading_time_2 =
            gt.pilot_follow_leader_heading_spd_time_2;
        gin.leader_heading_dist_1 =
            gt.pilot_follow_leader_heading_spd_dist_1;
        gin.leader_heading_dist_2 =
            gt.pilot_follow_leader_heading_spd_dist_2;
        // 009C16E8 reads block+04h = singleton+384h, which this
        // host already carries as Pilot/Follow/LeaderFollowAlt,
        // and 009C1789 reads singleton+210h = Dynamics/Ceiling.
        gin.band_floor_offset = gt.pilot_follow_leader_follow_alt;
        gin.band_ceiling_210 = gt.dynamics_ceiling;
        // SUBSTITUTION, labelled: state+88h is the floor's other
        // half and this host has no field for it.  A huge value
        // makes the min pick `leaderY + LeaderFollowAlt`, which
        // is the half that is named; it can only RAISE the floor
        // relative to the image, never lower it.
        gin.state_88 = 1.0e30f;
        gin.band_inputs_available = true;

        const bsp::PlaneFollowGeometry geo =
            bsp::plane_follow_geometry_009bfee0(
                gin, bsp::PlaneFollowRegime::kLeadPursuit);
        record("BotStateFollow::steer_point", 0x009bfee0u);

        bsp::PlaneFollowFlyToInputs fin;
        for (int i = 0; i < 3; ++i) {
            fin.station[i] = station.world[i];
            fin.steer_point[i] = geo.steer_point[i];
            fin.unit_pos[i] = unit.motion.position[i];
        }
        // The tail rewrote the station's Y (009C17F1), and that
        // rewritten value is what 009BEE30 blends from.
        fin.station[1] = geo.station_y;
        fin.unit_forward_x = static_cast<float>(
            std::sin(static_cast<double>(unit.plane_heading_c6c)));
        fin.unit_forward_z = static_cast<float>(
            std::cos(static_cast<double>(unit.plane_heading_c6c)));
        // 009BFC58 `CALL [[state+2Ch]]+38h`, the leader-speed
        // virtual: the leader's own travel speed.
        fin.leader_speed = leader.plane_travel_speed;
        // SUBSTITUTIONS, labelled: 007C47F0
        // BSP_PlaneClass_LevelFlightSpeed (009BFC41, then
        // `FMUL double [00D7A390]` = 0.9) and the floor at
        // classDesc+188h (009BFC7D) are not carried by this
        // host.  §5.4 records that the alignment ramp can only
        // reach 0.39 of the way toward the first of them, and
        // the second only scales the catch-up end.
        fin.level_flight_speed = unit.plane_max_spd * 0.9f;
        fin.class_min_speed = unit.plane_stall_spd;
        fin.good_position_dist = gt.pilot_follow_good_position_dist;
        fin.align_ramp_lo = gt.pilot_follow_max_follow_spd_target_dir;
        fin.align_ramp_hi = gt.pilot_follow_min_follow_spd_target_dir;
        fin.catchup_speed_scale =
            gt.dynamics_spd_multipliers_turbo_multiplier;
        fin.min_command_dist = gt.pilot_follow_followed_point_dist;
        const bsp::PlaneFollowFlyToCommand cmd =
            bsp::plane_follow_flyto_command_009bee30(fin);
        if (!cmd.produced) return;

        // 009BF9EA-009BF9F0: the pilot is steered at the steer
        // point.  Same mode-2 pair the moveto and attackrun
        // ticks write; 009F9E40's body is unread, so the bearing
        // is this host's own heading_command_009f9e40.
        unit.plan_heading_2c0 = bsp::heading_command_009f9e40(
            geo.steer_point[0], geo.steer_point[2],
            unit.motion.position[0], unit.motion.position[2]);
        unit.plan_heading_2c0_written = true;
        unit.plan_heading_mode_2cc = 2;
        record("BotStateFollow::steer_to_point", 0x009f9e40u);

        // 009BFC0C CALL 009F9ED0(cmdAlt - ownY, dist).  That
        // body is unread, so the commanded altitude is turned
        // into a pitch through the same 009FB800 every other
        // state uses, with the commanded altitude as its own
        // reference.  SUBSTITUTION, labelled.
        bsp::PlanePitchCommandInputs pin;
        pin.desired_altitude = cmd.commanded_altitude;
        pin.reference = cmd.commanded_altitude;
        pin.unit_world_y = unit.motion.position[1];
        pin.ceiling = gt.dynamics_ceiling;
        pin.climb_dist = gt.pilot_general_climb_dist;
        pin.drop_dist = gt.pilot_general_drop_dist;
        pin.class_climb_angle = unit.plane_climb_angle_1ec;
        pin.class_drop_angle = unit.plane_drop_angle;
        unit.plane_commanded_altitude = cmd.commanded_altitude;
        if constexpr (kPlaneFollowFlyToPitch) {
            // 009BFC03-009BFC21 CALL 009F9ED0(cmdAlt - ownY, max(dist,
            // FollowedPointDist)), read in packet cc9_follow_pitch: the
            // elevation angle of the steer altitude, capped at the class's
            // sustainable climb desc+1E4h, stored to plan+2BCh with the pitch
            // mode plan+2D0h = 2 (009F9F68 / 009F9F70). docs/PLANE_FOLLOW_PITCH.md.
            unit.plane_commanded_pitch = bsp::pitch_command_to_point_009f9ed0(
                cmd.altitude_error, cmd.command_distance, unit.plane_climb_angle_1e4);
            unit.plan_state.pitch_target_2bc = unit.plane_commanded_pitch;
            unit.plan_state.pitch_mode_2d0 = 2;
        } else {
            unit.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
            unit.plan_state.pitch_target_2bc = unit.plane_commanded_pitch;
        }

        // 009BFD0F-009BFD1C.
        unit.plane_desired_speed_2b4 = cmd.desired_speed_2b4;
        if constexpr (kPlaneFollowFlyToSpeedStores) {
            unit.plane_trg_speed_corr_off_2b0 = 0;   // 009BFD15, byte
            unit.plane_air_brake_mode_2d8 = 1;       // 009BFD1C, dword
        }
        record("BotStateFollow::command_step", 0x009bee30u);

        if ((unit.db_follow_tick_ticks % 400) == 1) {
            log.notef("  follow law %-12s n=%d R=%.1f D=%.1f "
                "ref=%.3f V=%.1f along=%.1f A=%.3f lag=%.2f "
                "steer=(%.1f %.1f %.1f) cmdalt=%.1f ownY=%.1f "
                "spd=%.2f band=%d",
                unit.row.name.c_str(), unit.db_follow_tick_ticks,
                static_cast<double>(geo.range_horizontal),
                static_cast<double>(geo.range_3d),
                static_cast<double>(geo.reference_heading),
                static_cast<double>(geo.cross_track),
                static_cast<double>(geo.along_track),
                static_cast<double>(geo.heading_error),
                static_cast<double>(geo.lag_time),
                static_cast<double>(geo.steer_point[0]),
                static_cast<double>(geo.steer_point[1]),
                static_cast<double>(geo.steer_point[2]),
                static_cast<double>(cmd.commanded_altitude),
                static_cast<double>(unit.motion.position[1]),
                static_cast<double>(cmd.desired_speed_2b4),
                geo.band_applied ? 1 : 0);
        }
        // DIAGNOSTIC, packet cc9_follow_speed: the throttle slot, the speed mode,
        // the pitch command and the measured speed, every kFollowTraceEvery ticks.
        if constexpr (kFollowTraceEvery > 0) {
        constexpr int kEvery = kFollowTraceEvery > 0 ? kFollowTraceEvery : 1;
        if ((unit.db_follow_tick_ticks % kEvery) == 1) {
            const bsp::PilotPlanSlot& th = unit.plan_slots[bsp::kPilotSlotThrottle];
            const float v2 = unit.plane_world_velocity[0] * unit.plane_world_velocity[0] +
                unit.plane_world_velocity[1] * unit.plane_world_velocity[1] +
                unit.plane_world_velocity[2] * unit.plane_world_velocity[2];
            log.notef("  follow trace %-12s n=%d ownY=%.1f cmdalt=%.1f pitch=%.3f "
                "v=%.2f want=%.2f mode2d8=%d thr=%.3f/%.3f act=%d state=%d",
                unit.row.name.c_str(), unit.db_follow_tick_ticks,
                static_cast<double>(unit.motion.position[1]),
                static_cast<double>(cmd.commanded_altitude),
                static_cast<double>(unit.plane_commanded_pitch),
                static_cast<double>(std::sqrt(v2)),
                static_cast<double>(unit.plane_desired_speed_2b4),
                unit.plane_air_brake_mode_2d8,
                static_cast<double>(th.current), static_cast<double>(th.desired),
                static_cast<int>(th.active), unit.plane_control_mode_900);
        }
        }
    }

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
// Packet cc9_ocean_waves, docs/OCEAN_WAVE_FIELD.md. True: both leaves run their
// reconstructed rules over the field state this process can establish: amplitude
// field+24h = 0.0f (00BA19C9, the only store), +B4h = 1/100 (00B9A4C0 over the
// 004CB420 default tile 100.0 at authored +30h), +F9h clear, no coverage regions
// (00BA0C00 / 00BA1370 build them from bitmaps this process does not load) and a
// labelled zero grid for the field+BCh spectrum. False: the records, as before.
constexpr bool kOceanWaveFieldBound = true;

class OceanFieldBinding final : public bsp::OceanHeightHost {
public:
    explicit OceanFieldBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}

    float wave_height_0078c890(float x, float z) override {
        if (!owner_.logged_ocean) {
            owner_.logged_ocean = true;
            owner_.log.notef("ocean sampler 0078cf20 runs over [world+A8h]'s field: amplitude "
                "+24h 0.0f (00ba19c9, its only store), tile 100.0 (004cb420 default), +F9h "
                "clear, no coverage regions, a labelled zero grid for the +BCh spectrum; "
                "0078c890 answers h * 0.0f and the product 0078cf64 is 0.0f");
        }
        if (!kOceanWaveFieldBound) {
            owner_.record("ShipMotion::ocean_wave_field", 0x0078c890u);
            return 0.0f;
        }
        bsp::OceanWaveFieldState field;
        field.flat_f9 = false;        // [[game+5FCh]+0C20h]: no scene record here
        field.inv_tile_b4 = static_cast<float>(1.0 / 100.0);   // FLD1; FDIVRP at 00B9A4D2
        field.amplitude_24 = 0.0f;    // 00BA19C9
        const bsp::OceanWaveGridView zero_grid{};  // LABELLED: the spectrum is not run
        owner_.done("ShipMotion::ocean_wave_field", 0x0078c890u);
        return bsp::ocean_wave_field_sample_0078c890(field, zero_grid, x, z);
    }
    float coverage_mask_00b9cf50(float x, float z) override {
        if (!kOceanWaveFieldBound) {
            owner_.record("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
            return 1.0f;
        }
        static const std::vector<bsp::OceanCoverageRegion> kNoRegions;
        owner_.done("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
        return bsp::ocean_coverage_mask_00b9cf50(kNoRegions, x, z);
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
        // 004F0FB0's stationary arm. The native reaches its own factory for
        // these: 00851CB0 fetches the globals, takes `StationaryClass` by name
        // and then the row by the type's own text, so the class lives in a
        // different table from `VehicleClass` and carries no `Type` literal for
        // 00964790's chain to match. A unit whose type resolves there is a
        // stationary prop, and it has no descriptor by construction rather than
        // by a gap in this process. docs/SCENE_STATIONARY_UNITS.md.
        if (kind == nullptr && host.lua.stationary_class_exists(row.type_symbol)) {
            slot->stationary_prop = true;
        }
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
            slot->plane_pitch_spd = lua_row.pitch_spd;
            slot->plane_travel_speed = lua_row.travel_speed;
            if constexpr (Impl::kPilotFiresBound) {
                // 007CD930 (from BSP_Plane_ReadPropertyBag): unit+C24h = any gun
                // whose platform ([class+94h][gun+38Ch]) has PilotFires set. The
                // authored table is VehicleClass[id].Platforms[k].PilotFires with
                // the platform's guns in .Gun; flattened once per class.
                host.ensure_pilot_fires_flattened();
                slot->plane_pilot_fires_c24 =
                    host.lua.read_vehicle_class_number(row.type_id, "BSPPilotFires", 1.0f) != 0.0f;
            }
            slot->plane_bomb_delay_1f4 = host.lua.read_vehicle_class_number(row.type_id, "BombDelay", 1.0f);   // class+1F4h BombDelay, 007D2318
            slot->plane_accel = lua_row.accel;
            // Packet cc9_dive_flight_response: the class reader scales Accel in
            // place (007D20F3-007D2127): when tuning+31Ch AccelCheatMul > 1.0,
            // class+164h = tuning+320h AccelCheatMulMul * AccelCheatMul * Accel,
            // before 007C4850 derives the drag coefficient +50Ch = +164h / MaxSpd^2
            // (007C4984-007C499C). This installation: 1.15 * 1.5 = 1.725. The host
            // kept the raw row value for thrust, drag and the climb angle.
            // docs/DIVE_FLIGHT_RESPONSE.md.
            if (kPlaneAccelCheatScaleBound && host.lua.plane_globals_loaded()) {
                const bsp::GameTuningBlock& g = host.lua.plane_globals();
                slot->plane_accel = bsp::plane_class_accel_007d20f3(
                    lua_row.accel, g.dynamics_accel_cheat_mul,
                    g.dynamics_accel_cheat_mul_mul).accel;
            }
            slot->plane_glide_rate = lua_row.glide_rate;
            slot->plane_drag_pitch_ratio = lua_row.drag_pitch_ratio;
            slot->plane_air_brake_drag = lua_row.air_brake_drag;
            slot->plane_drop_angle = lua_row.drop_angle;
            // Packet cc8_dive_race: desc+268h, read by the dive-bomb approach
            // constructor. The loader already parsed it (plane_class_fields.cpp
            // "TurnCircleRadius"); only the copy onto the slot was missing.
            slot->plane_turn_circle_radius = lua_row.turn_circle_radius;
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
        if (!slot->observer_prefix_ready && slot->stationary_prop) {
            // The prop's own pair, from its constructor rather than from a
            // creator row, because it has no descriptor to be keyed by.
            publish_stationary_prop_observer_tables_00748a40(slot->observer_prefix);
            slot->observer_prefix_ready = true;
        }
        if (host.observer_runtime != nullptr) {
            if (!host.observer_runtime->has_live_dispatch_owner())
                throw std::logic_error("unit creation requires the live bound observer owner");
            if (!slot->observer_prefix_ready) {
                // The message used to name neither the unit nor the creator,
                // which cost a whole survey a wrong cause: a zero creator and an
                // unmapped one are different failures and read identically.
                // Zero means no vehicle-class descriptor was found at all, which
                // is what a scene row typed from `StationaryTypes` produces: the
                // 220 rows of this installation's stationaryclasses.lua are
                // `StationaryClass`, not `VehicleClass`, and not one of them
                // carries the `Type` literal that 00964790's chain compares, so
                // the factory returns a null descriptor and there is nothing to
                // map. A non-zero creator means the class resolved but is absent
                // from the 22 rows in src/native_unit_observer_endpoint.cpp.
                // docs/SCENE_STATIONARY_REGISTRATION.md.
                char detail[192];
                std::snprintf(detail, sizeof(detail),
                    "unit observer creator projection is unavailable: unit=%s creator=%08lx (%s)",
                    row.name.c_str(),
                    static_cast<unsigned long>(slot->motion_dispatch.creator),
                    slot->motion_dispatch.creator == 0
                        ? "no vehicle-class descriptor; a StationaryClass row carries no Type"
                        : "resolved class with no observer table row");
                host.log.notef("%s", detail);
                throw std::logic_error(detail);
            }
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

        // Packet cc8_ship_station, edited under the integrator's hunk
        // arbitration of 2026-09-19. 00822C20's FIRST reconstructed step, before
        // the property-bag arm below: 00823508 calls 00818EA0 in straight-line
        // code, and 00818EA0's tail 00819367..00819381 fills the pose-history
        // ring at entity+0BD0h from the entity's cached world position
        // (&entity+0FCh) and the float its vtable slot +50h returns. That is what
        // gives a ship a trail before it has moved: forty samples 50 m apart
        // running 1950 m astern of the spawn heading. Without it a follower of a
        // leader that never moves measures its station along a ring of zeros and
        // steams for the world origin, which is what USN01 did
        // (docs/SHIP_UNIT_GROUP_FOLLOW.md section 5e).
        //
        // The heading is the same expression the motion tail uses for the append
        // at 00826CEE - an atan2 over world row 2, unit+1050h - because slot +50h
        // is that field's getter (docs/SHIP_AI_RUDDER_HOP.md).
        //
        // Not ship-gated: 00818EA0 is reached from the shared game-unit
        // SEntityInit, so every unit this host creates fills its ring.
        {
            const float spawn_heading = static_cast<float>(
                std::atan2(static_cast<double>(slot->motion.pose_row2[0]),
                           static_cast<double>(slot->motion.pose_row2[2])));
            bsp::ship_ai_wake_fill_00810020(slot->wake, slot->motion.position,
                spawn_heading);
            host.done("UnitPoseHistoryRing::fill_at_spawn", 0x00810020u);
        }

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
    // The plane-squadron member write-back, here because the slots now exist
    // and the AI host below reads the array as soon as it is constructed.
    // GameScriptOrdersHost owns this body but the free function needs no
    // scripts-orders host: it takes the process-wide registry and this host's
    // own count()/unit_row(). Without the call, a squadron that came from a
    // SCENE row still had an empty +3D0h array when build_squadrons asked, and
    // USN04 built 7 AI squadrons over 15 member planes where the mission has 5.
    // `only_unresolved` so the wipe inside it can never un-fill the inline
    // answer an air-ops launch already wrote; docs/PLANE_SQUADRON_HOST.md.
    bsp::game::resolve_plane_squadron_members(*this, &host.log, true);
    // Milestone 2t: the unit-side gunnery pass. 00810DD0's creation block puts a
    // 558h-byte object at unit+6DCh and attaches it to the unit's own tick
    // element unit+310h through its vtable +4h (00864BD0), which is why it runs
    // on the fixed step beside the motion pass. The guns themselves come from
    // the authored `VehicleClass[id].Platforms` table, because this process
    // builds no model hierarchy; include/bsp/game_hosts_gunnery.hpp says so.
    // Packet cc8_gunnery_host. This used to construct a FRESH gunnery host on
    // every create_units, which is every spawn batch: USN04 ran create_units 13
    // times in 4800 mission frames and 25 times in 9000 (four SpawnNew batches
    // at t=0, four air-ops launches at t=27..30 s, four more SpawnNew at
    // t=105 s, three single-unit creations at t=345 s and nine more air-ops
    // launches at t=366..381 s). Each one threw away the summary, the hit
    // records, the kill credits and every round, bomb and torpedo in flight.
    // The image creates the per-unit gun object once per unit - 00810DD0's
    // creation block puts the 558h-byte object at unit+6DCh and attaches it
    // through 00864BD0 - and has no per-batch refresh, so the host is the
    // mission's, not the batch's. Same class of artefact as the coordinator
    // below and the weapon directors cc8_ship_drive fixed in register_units.
    //
    // Packet cc8_gunnery_host measured the pair this guard is for, on one build
    // differing only by the guard (`if (true)` reproduces the old path exactly),
    // USN04, same parameters, 4800 mission frames: the summary stops being a
    // since-the-last-batch count and becomes the mission's. `first_hit` goes
    // 13.35 s -> 119.90 s, which is the SAME event relabelled - the last batch
    // built its host at mission 106.55 s, and 13.35 + 106.55 = 119.90.
    // `queued_hits` 151 -> 143 and `total_damage` 14042.2 -> 14607.3
    // (local/gh_off_usn04.log vs local/gh_on_usn04.log); `bomb_drops` 23,
    // `bomb_impacts` 20, `torpedo_drop drops` 12 and `deaths` 14 are unchanged,
    // because USN04 drops its first bomb after the last batch anyway and the
    // deaths that moved cancel.
    //
    // They do move, though, and that is the gameplay half of this. Ignoring the
    // clock relabel, the two runs are identical for 6274 census lines and
    // diverge at mission 224.80 s, where a torpedo that hits Fletcher-class02
    // unguarded swims past it guarded. Four sinkings flip: Lexington-class01
    // (the controlled carrier) took 6291 of 8000 and lived unguarded, takes the
    // full 8000 and SINKS at 230.26 s guarded; movieval dies; Fletcher-class02
    // and movieval|.-3 now survive. The cause is that build_guns ran
    // `state.health = state.max_health` over every unit on every batch, so a
    // unit was fully healed 12 times in this mission. USN01 makes one
    // create_units call and is bit-identical across the pair: 7140 census lines,
    // zero differences (local/gh_off_usn01.log vs local/gh_on_usn01.log).
    if (host.gunnery == nullptr) {
        host.gunnery = std::make_unique<GameGunneryHost>(host.log, *this, host.lua);
        host.gunnery->set_ship_ai(host.ship_ai);
        host.gunnery->attach_00864bd0();
    } else {
        host.gunnery->register_new_units_00864bd0();
    }
    // Packet cc8_ship_follow. This used to rebuild the coordinator on EVERY
    // spawn batch, which on USN04's twelve batches threw away its groups and its
    // counters eleven times: the AI summary reported available=0 refused=238
    // joins=0 while the units host, which survives, reported joins=17 for the
    // same run. The image creates one coordinator for the mission - 00A32350 is
    // a creation, not a per-batch refresh - and this is the same class of host
    // artefact as the director destruction cc8_ship_drive fixed in
    // register_units. New units need no registration here: the coordinator reads
    // the units host live and its next compose pass picks them up.
    // Packet cc8_ship_screen measured the pair this guard is for, on one build
    // differing only by the guard, USN04, same parameters: without it
    // `summary mission ai follow` reads requests=238 available=0 refused=238
    // joins=0 (local/coord_off_usn04.log) because the coordinator is rebuilt on
    // every spawn batch and the counters are the last batch's alone; with it,
    // requests=731 available=1 refused=730 joins=1 (local/screen_usn04.log), and
    // `total_path` goes 26985.10 -> 29455.68. The units host's own
    // `joins=25 clamped=9` is identical either way, which is what made the
    // artefact look harmless.
    if (host.ai == nullptr) {
        host.ai = std::make_unique<GameAiCoordinatorHost>(host.log, *this);
        host.ai->create_00a32350();
    }
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

void GameUnitsHost::set_skill_level_007b8ae0(std::size_t unit_index, int level) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return;
    host.slots[unit_index]->pilot_skill_index = level;
    // 007ECF80: a squadron forwards the call to every member's vtable[128h].
    // process_index is the slot index (see where slots are appended).
    const bsp::PlaneSquadronHostRecord* const sqn =
        bsp::plane_squadron_registry().find_by_member_unit(unit_index);
    if (sqn == nullptr) return;
    for (const std::size_t member : sqn->member_units) {
        if (member == bsp::kPlaneSquadronNoUnit || member >= host.slots.size()) continue;
        host.slots[member]->pilot_skill_index = level;
    }
}

int GameUnitsHost::skill_level(std::size_t unit_index) const {
    const Impl& host = *impl_;
    return unit_index < host.slots.size() ? host.slots[unit_index]->pilot_skill_index : 1;
}

std::vector<std::pair<std::size_t, float>> GameUnitsHost::destroyed_units() const {
    std::vector<std::pair<std::size_t, float>> out;
    const Impl& host = *impl_;
    if (host.gunnery == nullptr) return out;
    for (const GameGunneryUnitRow& row : host.gunnery->unit_rows()) {
        if (row.sunk) out.emplace_back(row.unit_index, row.sunk_seconds);
    }
    return out;
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
        // Packet cc8_ship_moveonpath, edited under the integrator's hunk
        // arbitration of 2026-09-19. 00836BF0-00836D66 is the `moveonpath` arm
        // of the same step, and it is the only place the path cursor moves:
        // 009E59C0 reads the cursor and never writes it.
        {
            float px = 0.0f, py = 0.0f, pz = 0.0f;
            unit_position_00fc(index, px, py, pz);
            // Packet cc8_ship_drive, edited under the integrator's hunk
            // arbitration of 2026-09-19. 00835C70's begin runs before the arm:
            // 0071F600 builds the path object for the command that is the queue
            // head, so a `moveonpath` that was queued under another command
            // begins on the step the queue advances to it. Where in a frame
            // 00835C70 runs is not established (00D09FD0/00D09FD4 are its only
            // references), so this is the same choice milestone 2m already made
            // for the `cruise` begin: immediately, on the director step.
            host.commands.begin_current_command_00835c70(index, px, pz);
            host.commands.advance_path_cursor_00836bf0(index, px, pz,
                unit_hull_length_09c8(index), unit_class_turn_radius_0520(index));
        }
        if (outcome.reissued == bsp::DirectorDefaultCommand::None) continue;
        // The idle tail's own choice becomes the unit's standing command when
        // the slot push took it; 0071be40 is what answers that.
        slot.row.command_current = slot.row.command_current
            || host.commands.holds_cruise(index);
    }
}

const GameCommandsHost& GameUnitsHost::commands() const noexcept { return impl_->commands; }

// Packet cc8_ship_moveonpath, edited under the integrator's hunk arbitration of
// 2026-09-19: the script-orders host reaches the command path only through this
// object, and the `moveonpath` path build writes to the director it owns.
GameCommandsHost& GameUnitsHost::commands() noexcept { return impl_->commands; }

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
                // Packet cc9_plane_death_modes (docs/PLANE_DEATH_MODES.md). The
                // death message chain as it reaches a plane: 00877B90 SetHealth
                // -> vtable[1B0h] 007CA8A0 chooses the mode on the draw ->
                // vtable[194h] 007BBFA0 stores unit+C10h and routes its message
                // -> 007D0B80 sets C39h/C36h, and its tail 007D12FC sets C3Ah and
                // calls vtable[70h](1), so the aircraft is dead (+5Dh) at once.
                // An immediate "explosion" also kills it at 007D0CFD. A killed
                // aircraft leaves the world, so this host stops stepping it.
                // SUBSTITUTIONS, labelled:
                // * the death is the gunnery host's (health <= 0 there), seen at
                //   the next plane step rather than inside SetHealth;
                // * the two draws come from a units-host generator, not from
                //   00BD2F10's stream 1 (selection) and stream 0 (delay): the
                //   gunnery host owns the shared stream and its file is leased;
                // * unit+DFCh (rammed by a unit) is clear, since this host has no
                //   ram hit, and unit+800h stays at 007CFDB4's -1, since its hits
                //   carry no hull segment, so neither spin is reachable;
                // * the 007D0B80 message is delivered in the same step.
                if constexpr (GameUnitsHost::Impl::kPlaneDeathModesBound) {
                    GameGunneryHost* const gun_host = host.gunnery.get();
                    if (slot.plane_death_mode == bsp::PlaneDeathMode::None
                        && !slot.plane_death_removed && gun_host != nullptr
                        && gun_host->unit_dead(index)) {
                        static std::uint32_t select_stream = 0x2545F491u;
                        static std::uint32_t delay_stream = 0x9E3779B9u;
                        const auto next = [](std::uint32_t& state, float low, float high) {
                            state = state * 1664525u + 1013904223u;
                            const float unit = static_cast<float>((state >> 8) & 0xFFFFFFu)
                                / static_cast<float>(0x1000000u);
                            return low + (high - low) * unit;
                        };
                        bsp::PlaneDeathModeInputs din;
                        din.health_370 = 0.0f;
                        din.draw = next(select_stream, 0.0f, 1.0f);   // 007CA914
                        din.rammed_dfc = false;
                        din.spin_side_800 = -1;
                        if (host.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = host.lua.plane_globals();
                            din.chance_explosion = g.death_mode_chances_explosion;
                            din.chance_explosion_delayed = g.death_mode_chances_explosion_delayed;
                            din.chance_spinning = g.death_mode_chances_spinning;
                            din.chance_powerloss = g.death_mode_chances_powerloss;
                        }
                        const bsp::PlaneDeathMode mode = bsp::plane_death_mode_007ca8a0(din);
                        // planepartclasses.lua ExplosionExplosionDelay, which
                        // 004A9BD0 stores at [00E18710] / [00E1870C].
                        static bool delay_read = false;
                        static float delay_pair[2] = {0.6f, 1.8f};
                        if (!delay_read) {
                            delay_read = true;
                            host.lua.read_global_number_pair("ExplosionExplosionDelay",
                                delay_pair[0], delay_pair[1]);
                        }
                        const float delayed = mode == bsp::PlaneDeathMode::ExplosionDelayed
                            ? next(delay_stream, delay_pair[0], delay_pair[1]) : 0.0f;
                        slot.plane_death_mode = mode;
                        slot.plane_death_timer_c10 =
                            bsp::plane_death_timer_c10_007bbfa0(mode, delayed);
                        const bsp::PlaneDeathFlags flags = bsp::plane_death_flags_007d0b80(
                            mode, slot.plane_death_timer_c10);
                        slot.plane_death_c39 = flags.powerlost_c39;
                        slot.plane_death_c36 = flags.spinning_c36;
                        slot.plane_death_c3a = true;
                        slot.plane_death_seconds = host.summary.simulated_seconds;
                        host.log.notef("plane death mode: unit=%s t=%.2f draw=%.4f mode=%s "
                            "c10=%.2f (007CA8A0 -> 007BBFA0 -> 007D0B80, packet "
                            "cc9_plane_death_modes)", slot.row.name.c_str(),
                            static_cast<double>(host.summary.simulated_seconds),
                            static_cast<double>(din.draw), bsp::plane_death_mode_name(mode),
                            static_cast<double>(slot.plane_death_timer_c10));
                        host.done("Plane::choose_death_mode_007ca8a0", 0x007ca8a0u);
                        if (flags.killed_now) {
                            slot.plane_death_removed = true;
                            host.log.notef("plane death explosion: unit=%s t=%.2f "
                                "dead_for=0.00 (007D0CFD Kill, packet cc9_plane_death_modes)",
                                slot.row.name.c_str(),
                                static_cast<double>(host.summary.simulated_seconds));
                            host.done("Death::entity_kill_00926d90", 0x007d0cfdu);
                        }
                    }
                    if (slot.plane_death_removed) continue;
                }
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
                        // Packet cc9_plane_death_modes: 007CAF10's death terms,
                        // called at 007CC322 before the controller step. The
                        // DeadMeat timer unit+C3Ch runs while the aircraft is
                        // dead, the explosion fires once it passes unit+C10h,
                        // and a power-lost aircraft has its throttle and air
                        // brake zeroed, live and latched, while the pilot keeps
                        // steering. SUBSTITUTION, labelled: the explosion budget
                        // [00E186E8] <= [00E1873C] MaxExplosionNum is taken as met.
                        if constexpr (GameUnitsHost::Impl::kPlaneDeathModesBound) {
                            if (unit_.plane_death_c3a) {
                                bsp::PlaneDeathStepInputs dsi;
                                dsi.dead_5d = true;
                                dsi.powerlost_c39 = unit_.plane_death_c39;
                                dsi.spinning_c36 = unit_.plane_death_c36;
                                dsi.free_flight_gate = true;
                                dsi.timer_c10 = unit_.plane_death_timer_c10;
                                dsi.dead_timer_c3c = unit_.plane_lost_drag_timer_c3c;
                                dsi.step = step;
                                const bsp::PlaneDeathStepResult dso =
                                    bsp::plane_death_step_007caf10(dsi);
                                unit_.plane_lost_drag_timer_c3c = dso.dead_timer_c3c;
                                if (dso.explode) {
                                    unit_.plane_death_timer_c10 = -1.0f;
                                    unit_.plane_death_removed = true;
                                    owner_.log.notef("plane death explosion: unit=%s "
                                        "t=%.2f dead_for=%.2f (007CAF10 -> 007D0CFD Kill, "
                                        "packet cc9_plane_death_modes)",
                                        unit_.row.name.c_str(),
                                        static_cast<double>(owner_.summary.simulated_seconds),
                                        static_cast<double>(dso.dead_timer_c3c));
                                }
                                if (dso.zero_throttle) {
                                    unit_.plane_live_throttle = 0.0f;
                                    unit_.plane_latched_throttle = 0.0f;
                                }
                                if (dso.zero_air_brake) {
                                    unit_.plane_live_air_brake = 0.0f;
                                    unit_.plane_latched_air_brake = 0.0f;
                                }
                                owner_.done("Plane::death_step_007caf10", 0x007caf10u);
                            }
                        }
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
                        // Packet cc8_plane_dive_instrumented. docs/PLANE_DIVE_RESPONSE.md
                        // section 3 shows the host's own terms capping any dive at about
                        // 129 m/s while two runs measured 141.5 to 141.9 at the water.
                        // This prints the four accumulator triples before the fold, the
                        // folded body total, and the three gravity candidates the doc
                        // named by address, once a second for a diving aircraft.
                        unit_.plane_dive_probe_timer += step;
                        if (unit_.torpedo_task_installed &&
                            unit_.plane_pitch_angle_c64 < -0.3f &&
                            unit_.plane_dive_probe_timer >= 1.0f) {
                            unit_.plane_dive_probe_timer = 0.0f;
                            const float* const wv = unit_.plane_world_velocity;
                            const float spd = std::sqrt(wv[0] * wv[0] + wv[1] * wv[1] +
                                                        wv[2] * wv[2]);
                            const float* const fwd = unit_.motion.pose_row2;
                            // the along-path share of each term, which is what the
                            // 1D model in local/dive_sim.py compares against
                            const float nx = spd > 1e-6f ? wv[0] / spd : 0.0f;
                            const float ny = spd > 1e-6f ? wv[1] / spd : 0.0f;
                            const float nz = spd > 1e-6f ? wv[2] / spd : 0.0f;
                            // 007D8470 returns a BODY acceleration; the arm rotates it
                            // back through the transpose of the pose rows. That rotation
                            // is gravity candidate 3.
                            const float* const probe_rows[3] = {unit_.motion.pose_row0,
                                unit_.motion.pose_row1, unit_.motion.pose_row2};
                            float wa[3] = {0.0f, 0.0f, 0.0f};
                            for (int c = 0; c < 3; ++c) {
                                for (int r = 0; r < 3; ++r) {
                                    wa[c] += probe_rows[r][c] * body.total[r];
                                }
                            }
                            const float along = wa[0] * nx + wa[1] * ny + wa[2] * nz;
                            owner_.log.notef(
                                "  dive probe %-12s alt=%.1f spd=%.2f pitch=%.4f "
                                "path=%.4f aoa=%.4f | along=%.3f thrust=%.3f drag=%.3f "
                                "| damp=(%.3f %.3f %.3f) wdrag=(%.3f %.3f %.3f) "
                                "lift=(%.3f %.3f %.3f) grav=(%.3f %.3f %.3f) "
                                "| body=(%.3f %.3f %.3f) world=(%.3f %.3f %.3f) "
                                "| fwd=(%.3f %.3f %.3f) cheat=%.2f",
                                unit_.row.name.c_str(),
                                static_cast<double>(unit_.motion.position[1]),
                                static_cast<double>(spd),
                                static_cast<double>(unit_.plane_pitch_angle_c64),
                                static_cast<double>(std::atan2(
                                    static_cast<double>(wv[1]),
                                    std::sqrt(static_cast<double>(wv[0]) * wv[0] +
                                              static_cast<double>(wv[2]) * wv[2]))),
                                static_cast<double>(std::acos(std::max(-1.0, std::min(1.0,
                                    static_cast<double>(nx * fwd[0] + ny * fwd[1] +
                                                        nz * fwd[2]))))),
                                static_cast<double>(along),
                                static_cast<double>(state.thrust_accel),
                                static_cast<double>(state.drag_accel),
                                static_cast<double>(acc.body_damping[0]),
                                static_cast<double>(acc.body_damping[1]),
                                static_cast<double>(acc.body_damping[2]),
                                static_cast<double>(acc.world_drag[0]),
                                static_cast<double>(acc.world_drag[1]),
                                static_cast<double>(acc.world_drag[2]),
                                static_cast<double>(acc.body_lift[0]),
                                static_cast<double>(acc.body_lift[1]),
                                static_cast<double>(acc.body_lift[2]),
                                static_cast<double>(acc.world_gravity[0]),
                                static_cast<double>(acc.world_gravity[1]),
                                static_cast<double>(acc.world_gravity[2]),
                                static_cast<double>(body.total[0]),
                                static_cast<double>(body.total[1]),
                                static_cast<double>(body.total[2]),
                                static_cast<double>(wa[0]),
                                static_cast<double>(wa[1]),
                                static_cast<double>(wa[2]),
                                static_cast<double>(fwd[0]),
                                static_cast<double>(fwd[1]),
                                static_cast<double>(fwd[2]),
                                static_cast<double>(tuning.accel_cheat_mul));
                        }
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
                        // The plane wing's formation station, applied once per
                        // member. The image's own site for this is the follow
                        // state's tick (009C1FD0 -> 009BFD70 -> 007F23A0) and
                        // this host binds that seam too, but no plane in this
                        // process ever enters that state, so the station would
                        // never be applied at all. docs/PLANE_FORMATION.md
                        // section 6 states this as a scheduling hole.
                        owner_.place_wing_member_on_station_007f23a0(unit_, true);
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
                            // Packet cc9_water_surface_law
                            // (docs/WATER_SURFACE_LAW.md). 007CB7F0's first test,
                            // from the listing: 007CB81A sets BL when desc+198h
                            // MinWaterSpd is zero, then 007CB82D-007CB838 return
                            // at 007CB9C2 with nothing done when 007BC5B0 is false
                            // and BL is clear. So a live AI aircraft of a class
                            // with a non-zero MinWaterSpd (this installation's
                            // Val: 22.22) stays in free flight under the surface.
                            // 007BC5B0 SUBSTITUTED: true when the gunnery host has
                            // the aircraft dead (health unit+150h <= 0); unit+61h
                            // has no writer and unit+AA0h is not carried (0); every
                            // aircraft is AI-held, since no human flies one here.
                            constexpr bool kPlaneWaterContactGateBound = true;
                            std::size_t self_index = owner_.slots.size();
                            for (std::size_t i = 0; i < owner_.slots.size(); ++i) {
                                if (owner_.slots[i].get() == &unit_) { self_index = i; break; }
                            }
                            GameGunneryHost* const gun_host = owner_.gunnery.get();
                            bool contact_ignored = false;
                            float min_water_spd = 0.0f;
                            bool dead = false;
                            if (unit_.motion.position[1] < line) {
                                static std::map<int, float> min_water_spd_by_type;
                                auto found = min_water_spd_by_type.find(unit_.row.type_id);
                                if (found == min_water_spd_by_type.end()) {
                                    found = min_water_spd_by_type.emplace(unit_.row.type_id,
                                        owner_.lua.read_vehicle_class_row(unit_.row.type_id)
                                            .min_water_spd).first;
                                }
                                min_water_spd = found->second;
                                dead = gun_host != nullptr
                                    && self_index < owner_.slots.size()
                                    && gun_host->unit_dead(self_index);
                            }
                            if (kPlaneWaterContactGateBound
                                && unit_.motion.position[1] < line) {
                                const bool gate_007bc5b0 = dead;
                                if (!gate_007bc5b0 && min_water_spd != 0.0f) {
                                    contact_ignored = true;
                                    static std::map<const void*, unsigned> ignored_by_unit;
                                    const unsigned ignored = ++ignored_by_unit[&unit_];
                                    owner_.record("Plane::water_contact_007cb7f0", 0x007cb7f0u);
                                    if (ignored == 1) {
                                        owner_.log.notef("plane water contact ignored: "
                                            "unit=%s alt=%.2f water=%.2f up_y=%.4f "
                                            "MinWaterSpd=%.3f; 007BC5B0 false, so 007CB7F0 "
                                            "returns at 007CB9C2 and the aircraft stays in "
                                            "free flight (packet cc9_water_surface_law)",
                                            unit_.row.name.c_str(),
                                            static_cast<double>(unit_.motion.position[1]),
                                            static_cast<double>(water),
                                            static_cast<double>(unit_.motion.pose_row1[1]),
                                            static_cast<double>(min_water_spd));
                                    }
                                }
                            }
                            if (!contact_ignored && unit_.motion.position[1] < line) {
                                ++unit_.plane_water_contacts;
                                // 007CB92C: 7, 4 and 5 all go to 6. The
                                // "powerlost" effect and the 0C3h session message
                                // are contracts; 0090F6C0(unit, 3) is the SA_OC
                                // scoring award, not damage.
                                unit_.plane_control_mode_900 = 6;
                                owner_.record("Plane::water_contact_007cb7f0", 0x007cb7f0u);
                                if (unit_.plane_water_contacts == 1) {
                                    owner_.log.notef("plane water contact: unit=%s "
                                        "alt=%.2f water=%.2f |v|=%.2f state 7 -> 6 "
                                        "(007CB7F0 tail 007CB92C); the free-flight "
                                        "gate 0074E210 is now false and the water "
                                        "surface law 007DCDD0 is a contract "
                                        "[type=%d MinWaterSpd=%.3f dead=%d up_y=%.4f]",
                                        unit_.row.name.c_str(),
                                        static_cast<double>(unit_.motion.position[1]),
                                        static_cast<double>(water),
                                        static_cast<double>(std::sqrt(
                                            unit_.plane_world_velocity[0]
                                                * unit_.plane_world_velocity[0]
                                            + unit_.plane_world_velocity[1]
                                                * unit_.plane_world_velocity[1]
                                            + unit_.plane_world_velocity[2]
                                                * unit_.plane_world_velocity[2])),
                                        unit_.row.type_id,
                                        static_cast<double>(min_water_spd), dead ? 1 : 0,
                                        static_cast<double>(unit_.motion.pose_row1[1]));
                                }
                            }
                        }
                        // Packet cc9_water_surface_law. 007CE313-007CE3AC, the plane
                        // tick's depth kill: in net modes other than 2 and with
                        // unit+61h clear, an altitude unit+100h below minus the
                        // limit calls BSP_MissionEntity_Kill(unit, 1) at 007CE3A7.
                        // The limit is 30.0f (00CE38C8) or the float +0Ch of
                        // (unit+360h)->+160h->+0Ch->vtable[48h](). SUBSTITUTION,
                        // labelled: that chain is not carried, so the image's own
                        // 30.0f fallback stands. SUBSTITUTION, labelled: the image
                        // runs this at the top of every tick in every state; here it
                        // runs after the free-flight step, for state 7 only, since a
                        // state-6 aircraft is frozen at the surface in this host.
                        {
                            constexpr bool kPlaneDepthKillBound = true;
                            constexpr float kPlaneDepthKillLimit = 30.0f;  // 00CE38C8
                            if (kPlaneDepthKillBound && unit_.plane_control_mode_900 == 7
                                && unit_.motion.position[1] < -kPlaneDepthKillLimit) {
                                GameGunneryHost* const gun_host = owner_.gunnery.get();
                                std::size_t self_index = owner_.slots.size();
                                for (std::size_t i = 0; i < owner_.slots.size(); ++i) {
                                    if (owner_.slots[i].get() == &unit_) { self_index = i; break; }
                                }
                                if (gun_host != nullptr && self_index < owner_.slots.size()
                                    && !gun_host->unit_dead(self_index)) {
                                    owner_.log.notef("plane depth kill: unit=%s alt=%.2f "
                                        "below -%.1f; BSP_MissionEntity_Kill(unit, 1) at "
                                        "007CE3A7 (packet cc9_water_surface_law)",
                                        unit_.row.name.c_str(),
                                        static_cast<double>(unit_.motion.position[1]),
                                        static_cast<double>(kPlaneDepthKillLimit));
                                    gun_host->kill_unit_00926d90(self_index, 1);
                                }
                                owner_.record("Plane::depth_kill_007ce3a7", 0x007ce3a7u);
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
                            // ctl+378h. 007F2D1E MOV byte [ESI+378h],1 seeds it
                            // SET, so a squadron that has not been through
                            // 007ED3C0 takes 007EEF6B's jump straight to the
                            // raise without consulting 007B8AD0. Nothing in this
                            // host calls 007ED3C0, whose caller is unlocated, so
                            // the flag stays set here; that divergence is
                            // labelled in docs/PLANE_SQUADRON_HOST.md section 6.
                            // A caller in no squadron keeps the old false.
                            const bsp::PlaneSquadronHostRecord* const sq378 = squadron();
                            in.force_flag_378 =
                                sq378 != nullptr && sq378->force_flag_378;

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
                            // ctl+3CCh. 007EE7F0 jumps to 007EE891 and stores
                            // zero into +374h when this is not positive, so a
                            // caller in no squadron answering 0 is the native's
                            // own arm and not a refusal invented here.
                            const bsp::PlaneSquadronHostRecord* const sq = squadron();
                            return sq != nullptr ? sq->live_count() : 0;
                        }
                        // ctl+3D0h / ctl+3CCh, the squadron's own unit array.
                        // 007F2C60 BSP_PlaneSquadronTickableEntity_Construct
                        // builds the block (ctl+34Ch at 007F2CDB, the attack
                        // mode ctl+370h at 007F2DC1) from
                        // 004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen, which
                        // runs at launch, not on an order.
                        //
                        // This used to stand in for the array with "carries
                        // torpedo ordnance", which counted every torpedo
                        // aircraft in the mission as one flight - five of them
                        // in USN01, all in different squadrons. Packet
                        // cc8_plane_squadron_host gave a PlaneSquadronGen row
                        // its real WingCount members, so the array is read from
                        // the squadron table now and the substitution is gone.
                        // 007C0EFA takes the receiver from plane+9D4h, which is
                        // exactly what find_by_member_unit answers.
                        const bsp::PlaneSquadronHostRecord* squadron() const {
                            return bsp::plane_squadron_registry().find_by_member_unit(
                                owner_.index_of_slot(slot_));
                        }
                        bool unit_lacks_follow_target_007b8ad0(int index) override {
                            GameUnitSlot* const u = controlled(index);
                            // CORRECTED, packet cc8_follow_enter. This used to
                            // answer `u->command_target_plus_one == 0` - "an
                            // ordered aircraft has a command target, which is
                            // what stands in for it". 007B8AD0 tests unit+9D8h,
                            // the squadron array slot, so the predicate is the
                            // flight-leader test and a command target has
                            // nothing to do with it: an ordered LEADER answered
                            // false here, which is the wrong way round.
                            if (u == nullptr) return true;
                            return owner_.unit_is_flight_leader_007b8ad0(
                                u->process_index);
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
                        // ctl+3D0h[index], in array order, skipping a wing whose
                        // record never became a unit so the index matches the
                        // count controlled_unit_count answers.
                        GameUnitSlot* controlled(int index) const {
                            const bsp::PlaneSquadronHostRecord* const sq = squadron();
                            if (sq == nullptr) return nullptr;
                            int n = 0;
                            for (std::size_t member : sq->member_units) {
                                if (member == bsp::kPlaneSquadronNoUnit) continue;
                                if (member >= owner_.slots.size()) continue;
                                if (n == index) return owner_.slots[member].get();
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
                            // when it built the task. The loadout DOES shrink now:
                            // GameGunneryHost::release_ordnance_drop clears the
                            // kind 2Bh bit from this mask on a drop, so this
                            // returns false for a spent bomber and approach+132h
                            // (== task+52Ah) goes false on the next approach
                            // update, which is what lets 009D4C10's range arm be
                            // reached. docs/TORPEDO_AFTER_THE_DROP.md section 12.4.
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
                            // 009D3517, approach->vtable[0]. Packet
                            // cc8_hull_aim_point: that getter returns the
                            // target-reference sub-object's +1Ch, which
                            // 009FADA0 fills at approach+B4h (B4h + 1Ch = D0h,
                            // the long-unfound approach+D0h writer). It is the
                            // hull point, not the target's origin, and there is
                            // no lead term in it.
                            const GameUnitSlot* const t = target();
                            if (t == nullptr) return false;
                            out_point[0] = t->motion.position[0];
                            out_point[1] = t->motion.position[1];
                            out_point[2] = t->motion.position[2];
                            hull_aim_world_point(slot_, *t,
                                                 slot_.command_target_plus_one,
                                                 out_point);
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
                            // 007B8AD0: unit->+9D8h == 0. FED, packet
                            // cc8_follow_enter: +9D8h is the squadron array
                            // slot, so this is the flight-leader test.
                            return owner_.unit_is_flight_leader_007b8ad0(
                                slot_.process_index);
                        }
                        bool should_break_off(void*) override {
                            // 009D4C10, now READ and bound. This override is the
                            // one the arm does not use: the transition rule takes
                            // should_break_off by value and it is computed where
                            // the target and the tuning are in scope, below.
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
                                                           float scale) override {
                            // 009FBA50 then 009FB800, the chain step 4 of the
                            // attackrun tick 009D07B0 runs. 009FBA50 biases the
                            // base by span * scale * class+518h only when
                            // span = max(range_high - range_low, 0) is positive,
                            // clamps against Dynamics/Ceiling - 50, and hands
                            // 009FB800 the CLAMPED altitude as its first
                            // argument and its OWN FOURTH ARGUMENT as its
                            // second. 009FB800 then writes cmd+2BCh and
                            // cmd+2D0h = 2. docs/TORPEDO_DESCENT_LAW.md.
                            bsp::PlaneCruiseAltitudeInputs cin;
                            cin.base_altitude = base;
                            cin.range_low = range_low;
                            cin.range_high = range_high;
                            cin.scale = scale;
                            // class+518h, derived at 007C4A3F/007C4A44 as
                            // tan(desc+1F0h DropAngle), so the bias term is
                            // horizontalDistance * scale * tan(DropAngle) and
                            // the commanded altitude IS a glide slope.
                            // docs/BOT_TASK_STATES.md line 564.
                            cin.class_gain = static_cast<float>(
                                std::tan(static_cast<double>(slot_.plane_drop_angle)));
                            // The squadron altitude limit squadron+394h, the
                            // ctl+394h leg of 009FBA9B, is unmodelled.
                            cin.has_squadron = false;
                            if (owner_.lua.plane_globals_loaded()) {
                                cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                            }
                            const bsp::PlaneCruiseAltitudeResult c =
                                bsp::cruise_altitude_command_009fba50(cin);
                            bsp::PlanePitchCommandInputs pin;
                            pin.desired_altitude = c.clamped_altitude;
                            pin.reference = c.pitch_reference;      // 009FBB06
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
                            // 009FBA50 ends in 009FB800, which writes cmd+2D0h = 2
                            // on every exit (009FB93F / 009FB95F / 009FBA41).
                            // Packet cc9_pitch_callers, docs/PITCH_COMMAND_CALLERS.md.
                            if constexpr (GameUnitsHost::Impl::kPitchCommandCallersBound) {
                                slot_.plan_state.pitch_mode_2d0 = 2;
                            }
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
                            // FED, packet cc8_follow_enter: 009D323A inside
                            // 009D3210, the leader-only gate on the self-engage
                            // range test (009D3241 JZ -> not engaged).
                            in.engaged.unit_has_no_follow_target =
                                owner_.unit_is_flight_leader_007b8ad0(
                                    slot_.process_index);
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
                            // FED, packet cc8_follow_enter: 009D4082 and
                            // 009D41E4, the two `!engaged` edges of 009D4030,
                            // each `LEA EDI,[ESI+544h]` / JNZ over
                            // `LEA EDI,[ESI+580h]` - moveto for the flight
                            // leader, follow for every wing member.
                            in.unit_has_no_follow_target =
                                owner_.unit_is_flight_leader_007b8ad0(
                                    slot_.process_index);
                            // 009D4C10, slot 1Ch of the TASK vtable 00D213C8,
                            // read whole from the listing 009D4C10-009D4C8E and
                            // re-derived jump sense by jump sense from the branch
                            // bytes. docs/TORPEDO_AFTER_THE_DROP.md sections 12
                            // and 13.1. Rule:
                            //   if (!0099C230(task))                 return false;
                            //   if (target == 0 || target->+5Dh)     return true;
                            //   if (ctl->+369h && [00E17BF2])        return false;
                            //   if (IsAttackState(cur) && task+52Ah) return false;
                            //   return range >= SafeDist * approach+24h;
                            // bot_task_should_break_off already carries the shape
                            // all five ordnance classes share, so this passes a
                            // torpedo class_extra rather than adding a sixth body.
                            //
                            // base_gate is a PROOF, not a stand-in. 0099C230
                            // returns false only for the unit the in-mission
                            // interface manager [00E198C4] is attached to -- the
                            // player's aircraft. That manager is null in this
                            // process, so the image takes its own first branch and
                            // returns true. Section 13.2.
                            //
                            // speed_ratio 1.0 is a FLOOR, not a match, and this is
                            // a hole rather than a proof: 009F9CE0 writes
                            // approach+24h as max(1.0, MaxSpd / ReferenceSpeed)
                            // (docs/BOT_TASKS.md:102) and the divisor is
                            // substituted here. The ratio is >= 1.0, so 1.0 gives
                            // the image's MINIMUM threshold of 700 m and retires a
                            // bomber EARLY, never late. The dive-bomb binding at
                            // :1144 carries the same stand-in for the same field.
                            {
                                const GameUnitSlot* tgt = nullptr;
                                if (slot_.command_target_plus_one != 0) {
                                    const std::size_t ti =
                                        slot_.command_target_plus_one - 1;
                                    if (ti < owner_.slots.size()) {
                                        tgt = owner_.slots[ti].get();
                                    }
                                }
                                // unit+5Dh. The polarity is settled from both
                                // ends: 00937449 CMP byte [EAX+5Dh],0 / JNZ shows
                                // a SET byte suppresses behaviour, and 00925E14
                                // CLEARS it at creation, so clear == live and
                                // engageable. `simulate` is this host's cell for
                                // that byte and it has TWO writers: the `= 0` at
                                // creation (:2686) and `= flags.torn_down` in
                                // store_scene_node_flags (:7318). So a live target
                                // gives false and this arm cannot retire a bomber
                                // spuriously, while a torn-down target DOES set it,
                                // so this is the right field rather than an inert
                                // hole. UNVERIFIED, and not chased here: who calls
                                // store_scene_node_flags, and whether a sunk ship
                                // reaches it with torn_down set. So the route is
                                // reachable in principle; that it fires when a
                                // target dies is NOT established. USN01 does not
                                // exercise it -- the two deaths are bombers, not
                                // the ordered targets. The field's name is the
                                // trap: read it as "marked", not "simulating".
                                const bool target_marked =
                                    tgt != nullptr && tgt->state != nullptr &&
                                    tgt->state->simulate != 0;
                                // ctl+369h is clear in this host
                                // (read_control_block), so the first refusal arm
                                // cannot fire and the ordnance arm is the live one.
                                const bool ctl_369 = false;
                                const bool global_e17bf2 = false;
                                const bool class_extra =
                                    !(ctl_369 && global_e17bf2) &&
                                    !(bsp::torpedo_in_attack_state_009d31d0(
                                          slot_.torpedo_state) &&
                                      slot_.torpedo_attack_flag_52a);
                                const float safe_dist = safe_distance_438();
                                in.should_break_off =
                                    bsp::bot_task_should_break_off(
                                        /*base_gate=*/true,
                                        /*has_target=*/tgt != nullptr,
                                        target_marked, class_extra,
                                        slot_.torpedo_approach.range_90,
                                        safe_dist,
                                        /*speed_ratio=*/1.0f);
                                // cc8_torpedo_retire item 1. Latch every input
                                // on the first tick the predicate is true. With
                                // base_gate true only two arms can return true,
                                // so the arm is decided by the target pair:
                                // arm 2 is `!has_target || target_marked`, arm 5
                                // is `SafeDist * ratio <= range`.
                                if (in.should_break_off) {
                                    ++slot_.torpedo_breakoff_true_ticks;
                                    if (slot_.torpedo_breakoff_first_tick < 0) {
                                        slot_.torpedo_breakoff_first_tick =
                                            slot_.torpedo_arm_ticks;
                                        slot_.torpedo_breakoff_arm =
                                            (tgt == nullptr || target_marked)
                                                ? 2 : 5;
                                        // The enum's values are vtable offsets,
                                        // so map to the census bucket the state
                                        // tick table already uses.
                                        switch (slot_.torpedo_state) {
                                            case bsp::TorpedoState::kMoveTo:
                                                slot_.torpedo_breakoff_state = 0;
                                                break;
                                            case bsp::TorpedoState::kFollow:
                                                slot_.torpedo_breakoff_state = 1;
                                                break;
                                            case bsp::TorpedoState::kDone:
                                                slot_.torpedo_breakoff_state = 2;
                                                break;
                                            case bsp::TorpedoState::kAttackRun:
                                                slot_.torpedo_breakoff_state = 3;
                                                break;
                                            case bsp::TorpedoState::kGoAway:
                                                slot_.torpedo_breakoff_state = 4;
                                                break;
                                            case bsp::TorpedoState::kAim:
                                                slot_.torpedo_breakoff_state = 5;
                                                break;
                                            case bsp::TorpedoState::kPrepare:
                                                slot_.torpedo_breakoff_state = 6;
                                                break;
                                            default:
                                                slot_.torpedo_breakoff_state = 7;
                                                break;
                                        }
                                        slot_.torpedo_breakoff_target_plus_one =
                                            static_cast<int>(
                                                slot_.command_target_plus_one);
                                        slot_.torpedo_breakoff_approach_ticks =
                                            slot_.torpedo_approach_ticks;
                                        slot_.torpedo_breakoff_no_target_ticks =
                                            slot_.torpedo_approach_no_target_ticks;
                                        slot_.torpedo_breakoff_has_target =
                                            tgt != nullptr;
                                        slot_.torpedo_breakoff_target_marked =
                                            target_marked;
                                        slot_.torpedo_breakoff_in_attack_state =
                                            bsp::torpedo_in_attack_state_009d31d0(
                                                slot_.torpedo_state);
                                        slot_.torpedo_breakoff_attack_flag_52a =
                                            slot_.torpedo_attack_flag_52a;
                                        slot_.torpedo_breakoff_has_ordnance_132 =
                                            slot_.torpedo_approach.has_ordnance_132;
                                        slot_.torpedo_breakoff_range_90 =
                                            slot_.torpedo_approach.range_90;
                                        slot_.torpedo_breakoff_limit_90 =
                                            slot_.torpedo_engage_limit_90;
                                        slot_.torpedo_breakoff_prev_range =
                                            slot_.torpedo_prev_range_90;
                                        slot_.torpedo_breakoff_safe_dist = safe_dist;
                                    }
                                }
                                slot_.torpedo_prev_range_90 =
                                    slot_.torpedo_approach.range_90;
                            }
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
                            // cc8_torpedo_retire item 5. Latch the run-in
                            // geometry on the FIRST tick of each attackrun, in
                            // the same convention the closest-approach census
                            // uses at the other end of the swim.
                            if (next == bsp::TorpedoState::kAttackRun &&
                                slot_.torpedo_state !=
                                    bsp::TorpedoState::kAttackRun) {
                                ++slot_.torpedo_attackrun_entries;
                                const GameUnitSlot* tgt = nullptr;
                                if (slot_.command_target_plus_one != 0) {
                                    const std::size_t ti =
                                        slot_.command_target_plus_one - 1;
                                    if (ti < owner_.slots.size()) {
                                        tgt = owner_.slots[ti].get();
                                    }
                                }
                                slot_.torpedo_attackrun_yaw_c6c =
                                    slot_.plane_heading_c6c;
                                slot_.torpedo_attackrun_own_pose =
                                    owner_.pose_heading_radians(slot_);
                                if (tgt != nullptr) {
                                    slot_.torpedo_attackrun_target_pose =
                                        owner_.pose_heading_radians(*tgt);
                                    slot_.torpedo_attackrun_crossing =
                                        std::fabs(
                                            bsp::wrapped_angle_subtract_00438b10(
                                                slot_.torpedo_attackrun_yaw_c6c,
                                                slot_
                                                    .torpedo_attackrun_target_pose));
                                }
                            }
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
                                // 009D0E3A-009D0F04, the rest of the same
                                // enter. It is what fills the climb altitude
                                // +1Ch and the `high` threshold +20h that the
                                // tick tests unit world y against, and without
                                // it the tick reads zeros.
                                bsp::TorpedoGoAwayEnterTailInputs gt;
                                gt.has_ordnance_132 =
                                    slot_.torpedo_approach.has_ordnance_132;
                                gt.alt_floor_74 = slot_.torpedo_approach.alt_floor_74;
                                gt.alt_margin_78 = slot_.torpedo_approach.alt_margin_78;
                                // CORRECTED by cc8_torpedo_retire, and the
                                // field's name is the trap: this is NOT a
                                // squadron object. 009D0E7C-009D0E7F is
                                //   mov ecx,[eax+0Ch]
                                //   fld dword ptr [ecx+394h]
                                // with eax = goaway+4h = the approach, so the
                                // base is approach+0Ch, and 009F9CE0 sets
                                // approach+0Ch to unit+9D4h - THE PILOT CONTROL
                                // BLOCK, the same block read_control_block()
                                // above already models. The confusion is with
                                // 009FBA9B's ceiling leg, which reads a
                                // different object's +394h.
                                //
                                // ctl+394h is the desired cruising altitude:
                                // step 4 of every task's +54h cruise profile
                                // writes `ctl->+394h = <cruising alt>` behind
                                // the not-overridden gate (docs/BOT_TASKS.md
                                // "The constructor shape" step 4 and the class
                                // table), and for this class that value is
                                // Pilot/Torpedo/CruisingAlt, tuning+430h.
                                //
                                // This matters because 009D0E42 sends a SPENT
                                // bomber down this leg and no other: with the
                                // ordnance byte clear it is the only producer of
                                // the climb altitude. Left false, the goaway ran
                                // with climb_1Ch=0/known=0, took its post-window
                                // LOW arm on every tick, commanded no altitude
                                // at all, and all five bombers flew level at 12
                                // m into the sea (local/retire_fix_usn01.log,
                                // before this line changed).
                                //
                                // UNCERTAINTY, stated rather than hidden: this
                                // host models no pilot control block, so it
                                // cannot observe the write. It assumes the
                                // cruise profile's write landed - exactly the
                                // assumption read_control_block() already makes
                                // for +398h and +39Ch, no weaker and no
                                // stronger. If the not-overridden gate refused
                                // the write, the image would read whatever the
                                // motion controller left there instead.
                                gt.has_squadron_cruising_alt_394 = true;
                                gt.squadron_cruising_alt_394 = cruising_alt_394();
                                // 00BD2F10 UniformFloatRange(50, 100) at
                                // 009D0E71; this host takes the low end of
                                // every draw.
                                gt.climb_jitter =
                                    bsp::torpedo_goaway_tick::kClimbJitterLo;
                                // UniformFloatRange(row+10h, row+14h) at
                                // 009D0EA3. row+10h and row+14h are
                                // TorpFlikFlakTime 1 and 2 in
                                // PilotBotParameters (docs/TORPEDO_RUN_PROFILE.md
                                // fixes row+0h as TorpReleaseAlt, so +10h is the
                                // fifth float). This host does not load the
                                // PilotBotConfig at all, so the draw is a HOLE,
                                // not a stand-in: it is left at zero and the
                                // manoeuvre window therefore opens on the first
                                // tick the aircraft is above +20h instead of
                                // after the authored delay.
                                owner_.log.unimplemented(
                                    "PilotBotParameters::TorpFlikFlakTime",
                                    "00997c68");
                                gt.window_delay_draw = 0.0f;
                                bsp::torpedo_goaway_enter_tail_009d0e3a(
                                    gt, slot_.torpedo_goaway_runtime);
                                slot_.torpedo_goaway_runtime
                                    .break_off_distance_24 = g.break_off_distance_24;
                                slot_.torpedo_goaway_runtime.side_2c =
                                    g.break_off_side_2c;
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
                        float cruising_alt_394() const {
                            // ctl+394h, which the +54h cruise profile 009D4A70
                            // fills from 0042E740()+430h,
                            // Pilot/Torpedo/CruisingAlt. The constant is the
                            // 500 of docs/GAME_TUNING_SINGLETON.md and is only
                            // the fallback; this installation's own value wins.
                            if (owner_.lua.plane_globals_loaded()) {
                                return owner_.lua.plane_globals()
                                    .pilot_torpedo_cruising_alt;
                            }
                            return bsp::kPilotTorpedoCruisingAltDefault;
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
                            // 009C1FD0 runs 009BFD70 (the station) and then
                            // 009BFEE0 (the law that flies to it). Only the
                            // first is bound; see
                            // Impl::place_wing_member_on_station_007f23a0 and
                            // docs/PLANE_FORMATION.md for what that leaves open.
                            if (owner_.place_wing_member_on_station_007f23a0(slot_, false)) {
                                owner_.log.implemented("BotStateFollow::station_point",
                                                       "007f23a0");
                            }
                            record("BotStateFollow::station_keeping", "009bfee0");
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

                    // ---- the dive-bomb task, kind 8. docs/DIVE_BOMB_TASK.md ----
                    struct DiveBombArmBinding final : bsp::DiveBombTaskHost {
                        DiveBombArmBinding(GameUnitsHost::Impl& owner,
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
                            // 009BDE80 at 009C8825. contract: unread body.
                        }
                        bool unit_has_no_follow_target(const void*) override {
                            // 007B8AD0 at 009C841F. FED, packet cc8_follow_enter.
                            return owner_.unit_is_flight_leader_007b8ad0(
                                slot_.process_index);
                        }
                        bool should_break_off(void* task) override {
                            // 009C8A90, reconstructed. The class extra is
                            // this->+4C9h == 0, so the dive bomber only breaks
                            // off on distance once its bombs are gone.
                            (void)task;
                            bsp::DiveBombBreakOffInputs in;
                            in.base_0099c230 = true;
                            in.has_latched_target = slot_.command_target_plus_one != 0;
                            in.has_bomb_ordnance_4c9 = slot_.db_has_bomb_d1;
                            // CORRECTED with the feed above: the 3-D range to
                            // the aim point, 009C8B14-009C8B3B.
                            in.distance_to_target = slot_.db_aim_point_3d;
                            in.speed_ratio_41c = 1.0f;
                            return bsp::dive_bomb_should_break_off_009c8a90(in);
                        }
                        bool manual_release_requested(void*) override {
                            // (unit+72Ch)->vtable[38h] at 009C887C.
                            return false;
                        }
                        void request_ordnance_release(void*) override {
                            // 007BBBA0 at 009C60F1, 009C5777 and 009C88C4. The
                            // same binding the torpedo task uses.
                            owner_.release_ordnance_007bbba0(slot_);
                        }
                        float unit_altitude(const void*) override {
                            return slot_.motion.position[1];
                        }
                        float random_between(float low, float) override { return low; }
                        float sample_heading_offset(void*, int) override { return 0.0f; }
                        float add_wrapped_angle(float base, float delta) override {
                            float v = base + delta;
                            const float two_pi = 6.2831855f;
                            while (v >= two_pi) v -= two_pi;
                            while (v < 0.0f) v += two_pi;
                            return v;
                        }
                        void command_altitude_and_throttle(void*, float, float, float,
                                                           float) override {}
                        void write_command_word(void*, int, int) override {}
                        void write_command_byte(void*, int, unsigned char) override {}
                        void write_command_float(void*, int, float) override {}
                        void set_arm_request(void*, bool) override {}
                        void set_weapon_selector(void*, int) override {}
                        void consume_round(void*) override {}

                        // --- DiveBombTaskHost ---
                        void update_dive_bomb_approach_009c7a80(void*, float dt,
                                                                bool diving)
                            override {
                            // The third argument was discarded; 009C7D09 reads
                            // it, so it now reaches the impact-point arm.
                            owner_.update_dive_bomb_approach(slot_, dt, diving);
                        }
                        bsp::DiveBombTransitionInputs read_transition_inputs(
                            void*) override {
                            return owner_.dive_bomb_transition_inputs(slot_);
                        }
                        void set_dive_bomb_state(void*, bsp::DiveBombState) override {
                            ++slot_.dive_bomb_transitions;
                        }
                        void write_turn_direction(void*, float roll) override {
                            slot_.db_turn_roll_18 = roll;  // 009C7800
                        }
                        bsp::DiveBombAimDiveReleaseInputs read_aimdive_inputs(
                            void* state, float dt) override {
                            (void)state;
                            return owner_.dive_bomb_aimdive_inputs(slot_, dt);
                        }
                        void apply_aimdive_result(
                            void*, const bsp::DiveBombAimDiveReleaseResult& r) override {
                            slot_.db_aim_rearm_1c = r.rearm_timer_1c;
                            slot_.db_aim_pull_out_18 = r.pull_out;
                            if (r.released) owner_.note_dive_bomb_release(slot_);
                        }
                        bsp::DiveBombAimGlideReleaseInputs read_aimglide_inputs(
                            void*, float dt) override {
                            return owner_.dive_bomb_aimglide_inputs(slot_, dt);
                        }
                        void apply_aimglide_result(
                            void*, const bsp::DiveBombAimGlideReleaseResult& r) override {
                            slot_.db_aim_rearm_1c = r.rearm_timer_1c;
                            slot_.db_glide_travel_20 = r.travel_accumulator_20;
                            // Packet cc8_dive_glide's census: which gate of
                            // 009C5689-009C5755 stopped this call, and the lead
                            // when the chain got far enough to compute one.
                            ++slot_.db_glide_calls;
                            if (r.gate_reached >= 0 && r.gate_reached <= 6) {
                                ++slot_.db_glide_gate_reached[r.gate_reached];
                            }
                            if (r.gate_reached >= 4) {
                                slot_.db_glide_lead_last = r.lead;
                                if (r.lead < slot_.db_glide_lead_min) {
                                    slot_.db_glide_lead_min = r.lead;
                                }
                            }
                            if (r.released) {
                                ++slot_.db_glide_releases;
                                slot_.db_glide_rounds += r.rounds_released;
                            }
                            if (r.released) {
                                owner_.note_dive_bomb_release(
                                    slot_, r.rounds_released);
                            }
                        }
                        int rounds_remaining_007c1db0(void*) override {
                            // 007C1DB0 walks the device list at unit+48h and
                            // sums 006E3500 over every device whose
                            // vtable[+5Ch] answers 25h. The gunnery host owns
                            // that list, so the count comes from there.
                            return owner_.dive_bomb_rounds_remaining(slot_);
                        }
                        float uniform_between(float low, float) override { return low; }
                        bool approach_has_bomb_ordnance(void*) override {
                            return slot_.db_has_bomb_d1;
                        }
                        bool approach_in_range_latch(void*) override {
                            return slot_.db_in_range_d0;
                        }
                        void spend_round(void*) override {
                            if (slot_.dive_bomb_rounds_remaining > 0) {
                                --slot_.dive_bomb_rounds_remaining;
                            }
                        }
                        int rounds_pending(void*) override {
                            return slot_.dive_bomb_rounds_pending;
                        }
                        void spend_pending_round(void*) override {
                            if (slot_.dive_bomb_rounds_pending > 0) {
                                --slot_.dive_bomb_rounds_pending;
                            }
                        }
                        void set_plan_step_scratch(void*, int) override {}
                        void commit_plan_step_result(void*) override {}
                        void write_drop_timer(void*, float) override {}
                        void* move_to_state(void* task) override { return task; }
                        float arm_move_to_range(void*) override {
                            return slot_.db_begin_alt_ac;   // task+4A4h
                        }
                        float arm_move_to_speed(void*) override {
                            return slot_.db_attack_dist_b4;  // task+4ACh
                        }

                        GameUnitsHost::Impl& owner_;
                        GameUnitSlot& slot_;
                    };

                    // 0099A170 builds a kind 8 task for an ordered aircraft
                    // whose class carries general bomb ordnance and is not
                    // IsKindOf(10h) (docs/ATTACK_COMMANDS.md, 007EE9C5).
                    // 0099993C. BSP_PilotBot_Update's slow path calls 0099B740
                    // once per think and BEFORE the task arm task->vtable[64h].
                    // 0099B740 (0099B740-0099B77A, read whole) is flight-leader
                    // only -- 0099B757 `CMP EAX,[ECX+3D0h]` -- and its abandon
                    // predicate [[bot]+38h] is 0099B710, `MOV AL,1 / RET`, for
                    // the dive bomb: the task vtable is 00D20E18 (proved by its
                    // +1Ch slot 00D20E34 = 009C8A90, the break-off predicate)
                    // and 00D20E50 = +38h holds 0099B710. So for a flight leader
                    // the predicate is always true and 0099B774 CALL 007ED3F0
                    // with PUSH 1 SETS squadron+370h to 1 on every think --
                    // 007ED3F0 is `MOV [ECX+370h],EAX / RET 4`, an assignment.
                    // Its sibling 007ED430 is the max/raise the Lua attack order
                    // uses at 008A4C41 to reach 2; nothing here calls that.
                    void run_dive_bomb_attack_mode_tick_0099b740() {
                        bsp::PilotAttackModeInputs in;
                        in.has_control_block_2fc = true;
                        in.has_unit_2f4 = true;
                        in.unit_is_flight_lead = unit_.db_is_flight_lead;
                        in.task_authorises_38h = true;
                        const bsp::PilotAttackMode next =
                            bsp::pilot_attack_mode_0099b740(
                                unit_.db_attack_mode_370, in);
                        if (next != unit_.db_attack_mode_370) {
                            unit_.db_attack_mode_370 = next;
                            ++unit_.db_mode_changes;
                            if (unit_.db_mode_first_set_tick < 0) {
                                unit_.db_mode_first_set_tick =
                                    unit_.dive_bomb_arm_ticks;
                            }
                        }
                        // +370h lives on the squadron, so the leader's value is
                        // what every member reads. Scoped through the registry
                        // to this unit's OWN squadron, not to every dive bomber
                        // in the mission.
                        if (!unit_.db_is_flight_lead) return;
                        auto* const sqn =
                            bsp::plane_squadron_registry().find_by_member_unit(
                                unit_.process_index);
                        if (sqn == nullptr) return;
                        for (const std::size_t member : sqn->member_units) {
                            if (member == bsp::kPlaneSquadronNoUnit) continue;
                            if (member >= owner_.slots.size()) continue;
                            owner_.slots[member]->db_attack_mode_370 =
                                unit_.db_attack_mode_370;
                        }
                    }

                    // 009AB1C0, the dogfight task's per-tick arm (primary vtable
                    // 00D1F9B0 slot +64h): 009AAC70 (approach update), 009AAFA0
                    // (transitions), then the state tick. Bound: install on class
                    // 00E08F58 or the `dogfight` token, 009AAC70's target
                    // selection and +4C8h latch, all of 009AAFA0, the generic
                    // follow tick, the aim state (009A75C0/009A76E0). Labelled
                    // stand-ins: moveto, maneuver, avoid_roll/avoid_turn.
                    // docs/DOGFIGHT_TASK.md, docs/DOGFIGHT_ENGAGED.md.
                    static bool df_slot_live(const GameUnitSlot& s) {
                        // 0043F080 / 009AACD1: +5Ch set, +5Dh/+5Eh/+60h clear.
                        return s.state != nullptr && s.state->active != 0 &&
                               s.state->simulate == 0 && s.scene_destroyed_005e == 0 &&
                               s.scene_pending_destroy_0060 == 0;
                    }
                    // The point in the unit's frame: 004142E0 with unit+110h, the
                    // inverse of the world matrix; rows 0/1/2 are right/up/forward.
                    void df_local(const float p[3], float out[3]) const {
                        const float* m = unit_.world.data();
                        const double d[3] = {static_cast<double>(p[0]) - m[12],
                                             static_cast<double>(p[1]) - m[13],
                                             static_cast<double>(p[2]) - m[14]};
                        for (int r = 0; r < 3; ++r) {
                            const float* row = m + 4 * r;
                            const double n = std::sqrt(static_cast<double>(row[0]) * row[0] +
                                                       static_cast<double>(row[1]) * row[1] +
                                                       static_cast<double>(row[2]) * row[2]);
                            out[r] = static_cast<float>(
                                (d[0] * row[0] + d[1] * row[1] + d[2] * row[2]) / (n > 0 ? n : 1));
                        }
                    }

                    // 009AAC70 and its selector 009AA630. The target squadron
                    // (approach+CCh) is the commanded target's squadron: the image
                    // sets it from the dogfight order; this host resolves the order
                    // to one unit, so its squadron stands in (SUBSTITUTION,
                    // labelled). Draws are fixed at their midpoints: the reselect
                    // timer 00BD2F10(1, 3) -> 2.0, the non-current factor
                    // 00BD2F10(0.8, 1) -> 0.9.
                    void df_approach_update_009aac70(float dt, float attack_dist) {
                        const bsp::PlaneSquadronHostRecord* tsq = nullptr;
                        if (unit_.command_target_plus_one != 0) {
                            tsq = bsp::plane_squadron_registry().find_by_member_unit(
                                unit_.command_target_plus_one - 1);
                        }
                        if (tsq == nullptr) {  // 009AAC76
                            unit_.df_latch_d0 = false;
                            unit_.df_distance_d4 = 9999.0f;
                            return;
                        }
                        unit_.df_target_squadron = true;
                        unit_.df_reselect_timer_e4 -= dt;
                        const GameUnitSlot* cur = nullptr;
                        if (unit_.df_target_plus_one != 0 &&
                            unit_.df_target_plus_one - 1 < owner_.slots.size()) {
                            cur = owner_.slots[unit_.df_target_plus_one - 1].get();
                        }
                        const bool live = cur != nullptr && df_slot_live(*cur);
                        if (bsp::dogfight_needs_reselect_009aac70(
                                live, unit_.df_reselect_timer_e4, unit_.df_distance_d4,
                                unit_.df_row.aim_shoot_distance)) {
                            df_select_009aa630(*tsq);
                        }
                        cur = nullptr;
                        if (unit_.df_target_plus_one != 0 &&
                            unit_.df_target_plus_one - 1 < owner_.slots.size()) {
                            cur = owner_.slots[unit_.df_target_plus_one - 1].get();
                        }
                        if (cur == nullptr) {
                            unit_.df_latch_d0 = false;
                            return;
                        }
                        // SUBSTITUTION, labelled: the aim point approach+48h is the
                        // 009FADA0 target reference's world point; for an aircraft
                        // target this host uses the target's origin.
                        for (int i = 0; i < 3; ++i) unit_.df_aim[i] = cur->motion.position[i];
                        const double dx = static_cast<double>(unit_.df_aim[0]) - unit_.motion.position[0];
                        const double dy = static_cast<double>(unit_.df_aim[1]) - unit_.motion.position[1];
                        const double dz = static_cast<double>(unit_.df_aim[2]) - unit_.motion.position[2];
                        unit_.df_distance_d4 = static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
                        unit_.df_horizontal_d8 = static_cast<float>(std::sqrt(dx * dx + dz * dz));
                        // approach+24h: max(1, MaxSpd / ReferenceSpeed). The 1.0
                        // floor stands in, as in the torpedo and dive-bomb
                        // bindings: the image's latch range is never shorter.
                        const bsp::DogfightLatch l = bsp::dogfight_latch_009aac70(
                            true, unit_.df_distance_d4, attack_dist, 1.0f, unit_.df_latch_d0,
                            1.0f);
                        if (l.latched && !unit_.df_latch_d0) ++unit_.df_latch_sets;
                        unit_.df_latch_d0 = l.latched;
                        df_local(unit_.df_aim, unit_.df_local_ec);
                        bsp::dogfight_off_axis_009aac70(unit_.df_local_ec, unit_.df_tan_f8);
                    }

                    void df_select_009aa630(const bsp::PlaneSquadronHostRecord& tsq) {
                        ++unit_.df_reselects;
                        unit_.df_reselect_timer_e4 = 2.0f;  // 00BD2F10(1.0, 3.0), midpoint
                        std::size_t members[5];
                        std::size_t n = 0;
                        for (const std::size_t m : tsq.member_units) {
                            if (n == 5 || m == bsp::kPlaneSquadronNoUnit) break;
                            members[n++] = m;
                        }
                        if (n == 0) return;
                        std::size_t best = bsp::kPlaneSquadronNoUnit;
                        if (n < 2) {  // 009AA66B, +3CCh < 2
                            best = members[0];
                        } else {
                            const bsp::PlaneSquadronHostRecord* own =
                                bsp::plane_squadron_registry().find_by_member_unit(
                                    unit_.process_index);
                            float best_score = 0.0f;
                            for (std::size_t i = 0; i < n; ++i) {
                                if (members[i] >= owner_.slots.size()) continue;
                                const GameUnitSlot& c = *owner_.slots[members[i]];
                                if (!df_slot_live(c)) continue;
                                bsp::DogfightCandidate cand;
                                df_local(c.motion.position, cand.local);
                                const double dx = static_cast<double>(c.motion.position[0]) - unit_.motion.position[0];
                                const double dy = static_cast<double>(c.motion.position[1]) - unit_.motion.position[1];
                                const double dz = static_cast<double>(c.motion.position[2]) - unit_.motion.position[2];
                                cand.distance = static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
                                cand.is_current_target = unit_.df_target_plus_one == members[i] + 1;
                                if (own != nullptr) {
                                    for (const std::size_t w : own->member_units) {
                                        if (w == bsp::kPlaneSquadronNoUnit) break;
                                        if (w == unit_.process_index || w >= owner_.slots.size()) continue;
                                        // 007BBC10 reads the wingmate's pilot
                                        // target; its dogfight target stands in.
                                        if (owner_.slots[w]->df_target_plus_one == members[i] + 1) {
                                            ++cand.wingmates_on_it;
                                        }
                                    }
                                }
                                const float s = bsp::dogfight_target_score_009aa630(
                                    cand, unit_.df_row.aim_shoot_distance, 0.9f);
                                if (best_score < s) {
                                    best_score = s;
                                    best = members[i];
                                }
                            }
                            if (best == bsp::kPlaneSquadronNoUnit) best = members[0];
                        }
                        if (unit_.df_target_plus_one != best + 1) {
                            ++unit_.df_target_changes;
                            owner_.log.notef("  dogfight %-12s target -> %s",
                                unit_.row.name.c_str(),
                                best < owner_.slots.size()
                                    ? owner_.slots[best]->row.name.c_str() : "?");
                        }
                        unit_.df_target_plus_one = best + 1;  // 009A7650
                        owner_.record("BotTaskDogfight::select_target", 0x009aa630u);
                    }

                    void df_steer(float heading, float pitch, float altitude) {
                        unit_.plan_heading_2c0 = heading;
                        unit_.plan_heading_2c0_written = true;
                        unit_.plan_heading_mode_2cc = 2;
                        unit_.plane_commanded_altitude = altitude;
                        unit_.plane_commanded_pitch = pitch;
                        unit_.plan_state.pitch_target_2bc = pitch;
                        unit_.plan_state.pitch_mode_2d0 = 2;
                    }

                    // 00999979: BSP_PilotBot_Update ticks the task gun controller
                    // 009FC7C0 (task+314h) after the task arm, while unit+C24h is
                    // set. Census only (packet cc9_dogfight_gun).
                    void run_dogfight_task_arm_009ab1c0(float dt) {
                        df_arm_body_009ab1c0(dt);
                        if constexpr (GameUnitsHost::Impl::kDogfightGunBound) {
                            // 00999962: the gun controller ticks only while unit+C24h.
                            if (unit_.dogfight_task_installed &&
                                (!GameUnitsHost::Impl::kPilotFiresBound ||
                                 unit_.plane_pilot_fires_c24)) {
                                df_gun_tick_009fc7c0(dt);
                            }
                        }
                    }

                    void df_gun_tick_009fc7c0(float dt) {
                        bsp::DogfightGunInputs gi;
                        gi.dt = dt;
                        gi.shoot_distance = unit_.df_row.aim_shoot_distance;
                        gi.search_range_30 = unit_.df_row.aim_shoot_distance + 650.0f;
                        const GameUnitSlot* tgt = nullptr;
                        if (unit_.df_target_plus_one != 0 &&
                            unit_.df_target_plus_one - 1 < owner_.slots.size()) {
                            tgt = owner_.slots[unit_.df_target_plus_one - 1].get();
                        }
                        // SUBSTITUTION, labelled: +74h comes from [unit+C50h]'s
                        // finder 007E2090 (unread); the task's own target stands in,
                        // unled (the target's vt[48h] prediction is not modelled).
                        if (tgt != nullptr && df_slot_live(*tgt)) {
                            gi.has_target = true;
                            float p[3];
                            for (int i = 0; i < 3; ++i) p[i] = tgt->motion.position[i];
                            df_local(p, gi.lead_local);
                        }
                        if constexpr (GameUnitsHost::Impl::kPlaneFinderBound) {
                            gi.has_target = false;
                            tgt = df_finder_007e2090(dt, gi);
                        }
                        const bool was_burst = unit_.df_gun.burst_4b;
                        bsp::dogfight_gun_tick_009fc7c0(unit_.df_gun, gi);
                        if constexpr (GameUnitsHost::Impl::kPlaneGunfireBound) {
                            // task+2E0h = plan+2DCh -> cmd+16h -> unit+9FAh (007BB8BF,
                            // forced 0 when unit+5Dh is set) -> +BC9h (007B97B4).
                            const bool f = unit_.df_gun.fire_48 && df_slot_live(unit_);
                            if (f && !unit_.plane_gun_fire_bc9) ++unit_.pg_trigger_rises;
                            if (f) ++unit_.pg_trigger_ticks;
                            unit_.plane_gun_fire_bc9 = f;
                        }
                        if (unit_.df_gun.burst_4b && !was_burst) {
                            const double d = std::sqrt(
                                static_cast<double>(gi.lead_local[0]) * gi.lead_local[0] +
                                static_cast<double>(gi.lead_local[1]) * gi.lead_local[1] +
                                static_cast<double>(gi.lead_local[2]) * gi.lead_local[2]);
                            owner_.log.notef("  fighter gun burst %-12s target=%s d=%.1f "
                                "lateral=%.2f state=%s",
                                unit_.row.name.c_str(),
                                tgt != nullptr ? tgt->row.name.c_str() : "-", d,
                                std::sqrt(static_cast<double>(gi.lead_local[0]) * gi.lead_local[0] +
                                          static_cast<double>(gi.lead_local[1]) * gi.lead_local[1]),
                                bsp::dogfight_state_name(unit_.dogfight_state));
                        }
                        owner_.record("BotTaskGun::tick", 0x009fc7c0u);
                    }

                    // 007E2010 (the C50h object's update) and 007E11D0 (its
                    // refresh), then 007E2090 with the gun's arguments from
                    // 009FC90E: cone max(+2Ch 0.4, 1.25 * +40h), +38h, +30h,
                    // +34h * 0.3, threshold +28h (-1.0), no squadron filter, no
                    // noise. The object's own tick cadence is not read; the
                    // think interval stands in. Labelled.
                    // plane+9D0h, the tie-break 007F0792/007F07E3/007F082F read:
                    // the member's formation index from its squadron record (0
                    // until 007ED260 has run, as in the image).
                    int nf_formation_index_9d0(std::size_t unit) const {
                        const bsp::PlaneSquadronHostRecord* sq =
                            bsp::plane_squadron_registry().find_by_member_unit(unit);
                        if (sq == nullptr || !sq->formation_indices_assigned) return 0;
                        for (std::size_t k = 0; k < sq->member_units.size(); ++k) {
                            if (sq->member_units[k] == unit &&
                                k < sq->member_formation_index.size()) {
                                return sq->member_formation_index[k];
                            }
                        }
                        return 0;
                    }

                    // 007F0280 in mode 1. SUBSTITUTION, labelled: the image walks
                    // [unit+C50h]'s +30h list (IsKindOf 6 or 0Fh within 1080 m,
                    // refreshed every 3 s by 007E11D0) and keeps vtable[5Ch](0Fh);
                    // this scans every live aircraft directly, without the 3 s lag.
                    // The box is at most 140 m, far inside the list's 1080 m.
                    bsp::NearFieldProbeResult nf_probe_007f0280(const float extents[3],
                                                                const float weights[3]) {
                        std::vector<bsp::NearFieldCandidate> cands;
                        const float reach = extents[0] + extents[1] + extents[2];
                        for (std::size_t j = 0; j < owner_.slots.size(); ++j) {
                            if (j == unit_.process_index) continue;   // 007F056F
                            const GameUnitSlot& o = *owner_.slots[j];
                            if (!df_slot_live(o)) continue;
                            if (!bsp::unit_is_kind_of(o.class_id, 0x0F)) continue;  // 007F03D8
                            const double dx = static_cast<double>(o.motion.position[0]) - unit_.motion.position[0];
                            const double dy = static_cast<double>(o.motion.position[1]) - unit_.motion.position[1];
                            const double dz = static_cast<double>(o.motion.position[2]) - unit_.motion.position[2];
                            if (dx * dx + dy * dy + dz * dz > static_cast<double>(reach) * reach) continue;
                            bsp::NearFieldCandidate c;
                            float pp[3] = {o.motion.position[0], o.motion.position[1], o.motion.position[2]};
                            df_local(pp, c.local);
                            c.formation_index_9d0 = nf_formation_index_9d0(j);
                            cands.push_back(c);
                        }
                        const bsp::NearFieldProbeResult r = bsp::near_field_probe_007f0280(
                            extents, weights, nf_formation_index_9d0(unit_.process_index),
                            cands.data(), cands.size());
                        ++unit_.nf_calls;
                        if (r.hit) ++unit_.nf_hits;
                        owner_.record("NearFieldProbe::probe", 0x007f0280u);
                        return r;
                    }

                    // 007E2090 proper: +84h <= +8Ch (2.0) returns the cached +B0h;
                    // otherwise subtract 2.0 and rescan +50h with these arguments.
                    // One cache and one clock for every caller (the gun, 009AAA80).
                    const GameUnitSlot* df_finder_scan_007e2090(
                            const bsp::PlaneFinderParams& fp, float threshold,
                            const bsp::PlaneSquadronHostRecord* squadron, float noise_mid) {
                        const GameUnitSlot* found = nullptr;
                        if (unit_.nb_found_plus_one != 0 &&
                            unit_.nb_found_plus_one - 1 < owner_.slots.size()) {
                            found = owner_.slots[unit_.nb_found_plus_one - 1].get();
                        }
                        if (!(unit_.nb_clock_84 > 2.0f)) return found;
                        unit_.nb_clock_84 -= 2.0f;
                        ++unit_.nb_scans;
                        float best = threshold;
                        std::size_t pick = bsp::kPlaneSquadronNoUnit;
                        for (const std::size_t e : unit_.nb_enemy_50) {
                            if (e >= owner_.slots.size()) continue;
                            const GameUnitSlot& o = *owner_.slots[e];
                            if (!df_slot_live(o)) continue;
                            if (squadron != nullptr &&   // candidate+9D4h == squadron
                                bsp::plane_squadron_registry().find_by_member_unit(e) != squadron) {
                                continue;
                            }
                            float loc[3];
                            float p[3] = {o.motion.position[0], o.motion.position[1],
                                          o.motion.position[2]};
                            df_local(p, loc);
                            const double hx = static_cast<double>(p[0]) - unit_.motion.position[0];
                            const double hz = static_cast<double>(p[2]) - unit_.motion.position[2];
                            float s = bsp::plane_finder_score_007deec0(
                                fp, loc, p[1] - unit_.motion.position[1],
                                static_cast<float>(std::sqrt(hx * hx + hz * hz)));
                            if (0.0f < s && noise_mid > 0.0f) s += noise_mid;  // U(0, noise)
                            if (best < s) {
                                best = s;
                                pick = e;
                            }
                        }
                        unit_.nb_found_plus_one = pick == bsp::kPlaneSquadronNoUnit ? 0 : pick + 1;
                        return pick == bsp::kPlaneSquadronNoUnit ? nullptr : owner_.slots[pick].get();
                    }

                    // 009AAA80, read from the listing (packet cc9_plane_gun_pass):
                    // approach+DCh > 0.0 and +E0h < 1.0, else 007B9740 and false;
                    // 007B96F0(+DCh, row+220h, task+344h, row+20Ch, +E0h, task+4C4h,
                    // 0.05 noise); a found target below the unit is taken; one above
                    // is taken when atan(dy / max(1, h)) < interp(007C4830 = 1.6 *
                    // StallSpd, 0, 007C47F0 = LevelFlight * StallSpd, 1.6, speed).
                    // Either way 009A7650 retargets and the task goes to aim.
                    bool df_early_edge_009aaa80() {
                        if (!(unit_.df_dc > 0.0f) || !(unit_.df_e0 < 1.0f)) return false;
                        ++unit_.df_early_asks;
                        const bsp::PlaneSquadronHostRecord* tsq = nullptr;
                        if (unit_.command_target_plus_one != 0) {
                            tsq = bsp::plane_squadron_registry().find_by_member_unit(
                                unit_.command_target_plus_one - 1);
                        }
                        bsp::PlaneFinderParams fp;
                        fp.cone_b8 = unit_.df_dc;
                        fp.inner_c0 = 0.03f;                               // row+220h, SPNormal
                        fp.range_bc = unit_.df_row.aim_shoot_distance + 650.0f;  // task+344h
                        fp.near_c4 = unit_.df_row.follow_dist;             // row+20Ch
                        // 00CE7638 = 0.05, the U(0, 0.05) draw at its midpoint.
                        const GameUnitSlot* f = df_finder_scan_007e2090(fp, unit_.df_e0, tsq, 0.025f);
                        if (f == nullptr) return false;
                        const double dx = static_cast<double>(f->motion.position[0]) - unit_.motion.position[0];
                        const double dy = static_cast<double>(f->motion.position[1]) - unit_.motion.position[1];
                        const double dz = static_cast<double>(f->motion.position[2]) - unit_.motion.position[2];
                        bool take = !(dy > 0.0);   // 009AABC9: dy <= 0 takes it at once
                        if (!take) {
                            const float stall = unit_.plane_stall_spd > 0.0f ? unit_.plane_stall_spd : 17.5f;
                            float lf = 1.8f, sr = 1.6f;
                            if (owner_.lua.plane_globals_loaded()) {
                                lf = owner_.lua.plane_globals().dynamics_spd_multipliers_level_flight;
                            }
                            const double v2 = static_cast<double>(unit_.plane_world_velocity[0]) * unit_.plane_world_velocity[0] +
                                static_cast<double>(unit_.plane_world_velocity[1]) * unit_.plane_world_velocity[1] +
                                static_cast<double>(unit_.plane_world_velocity[2]) * unit_.plane_world_velocity[2];
                            const float limit = bsp::dive_bomb_interpolate_clamped_00419010(
                                sr * stall, 0.0f, lf * stall, 1.6f, static_cast<float>(std::sqrt(v2)));
                            double h = std::sqrt(dx * dx + dz * dz);
                            if (h < 1.0) h = 1.0;
                            take = std::atan(dy / h) < limit;
                        }
                        if (!take) return false;
                        for (std::size_t j = 0; j < owner_.slots.size(); ++j) {
                            if (owner_.slots[j].get() == f) {
                                if (unit_.df_target_plus_one != j + 1) ++unit_.df_target_changes;
                                unit_.df_target_plus_one = j + 1;   // 009A7650
                                break;
                            }
                        }
                        ++unit_.df_early_edges;
                        return true;
                    }

                    const GameUnitSlot* df_finder_007e2090(float dt, bsp::DogfightGunInputs& gi) {
                        unit_.nb_clock_7c += dt;
                        unit_.nb_clock_84 += dt;
                        const bsp::PlaneNeighbourRadii r = bsp::plane_neighbour_radii_007e11d0(3.0f);
                        auto dist2 = [&](const GameUnitSlot& o) {
                            const double dx = static_cast<double>(o.motion.position[0]) - unit_.motion.position[0];
                            const double dy = static_cast<double>(o.motion.position[1]) - unit_.motion.position[1];
                            const double dz = static_cast<double>(o.motion.position[2]) - unit_.motion.position[2];
                            return dx * dx + dy * dy + dz * dz;
                        };
                        if (unit_.nb_clock_7c >= 3.0f) {  // 007E204D
                            ++unit_.nb_refreshes;
                            const double lim = static_cast<double>(r.enemy_plane) * r.enemy_plane;
                            auto& v = unit_.nb_enemy_50;
                            for (std::size_t k = 0; k < v.size();) {   // the drop pass
                                if (v[k] >= owner_.slots.size() || dist2(*owner_.slots[v[k]]) >= lim) {
                                    v[k] = v.back();
                                    v.pop_back();
                                } else {
                                    ++k;
                                }
                            }
                            for (std::size_t j = 0; j < owner_.slots.size(); ++j) {  // the add pass
                                if (j == unit_.process_index) continue;
                                const GameUnitSlot& o = *owner_.slots[j];
                                if (!df_slot_live(o)) continue;
                                if (!bsp::unit_is_kind_of(o.class_id, 0x0F)) continue;
                                if (o.row.party == unit_.row.party) continue;
                                if (!(dist2(o) < lim)) continue;
                                bool listed = false;
                                for (const std::size_t e : v) listed = listed || e == j;
                                if (!listed) v.push_back(j);
                            }
                            unit_.nb_clock_7c -= 3.0f;
                        }
                        const GameUnitSlot* found = nullptr;
                        if (unit_.nb_found_plus_one != 0 &&
                            unit_.nb_found_plus_one - 1 < owner_.slots.size()) {
                            found = owner_.slots[unit_.nb_found_plus_one - 1].get();
                        }
                        {
                            bsp::PlaneFinderParams fp;
                            float angle_strafe = 0.0f;
                            if (unit_.dogfight_state == bsp::DogfightState::kAim &&
                                owner_.lua.plane_globals_loaded()) {
                                angle_strafe = owner_.lua.plane_globals()
                                    .pilot_auto_strafe_angle_angle_strafe;
                            }
                            fp.cone_b8 = (0.4f > angle_strafe * 1.25f) ? 0.4f : angle_strafe * 1.25f;
                            fp.inner_c0 = gi.lateral_cap_38;
                            fp.range_bc = gi.search_range_30;
                            fp.near_c4 = static_cast<float>(gi.shoot_distance * 0.30000001192092896);
                            found = df_finder_scan_007e2090(fp, -1.0f, nullptr, 0.0f);  // +28h
                        }
                        // 009FC922: a live result becomes +74h.
                        if (found != nullptr && df_slot_live(*found)) {
                            gi.has_target = true;
                            float p[3] = {found->motion.position[0], found->motion.position[1],
                                          found->motion.position[2]};
                            df_local(p, gi.lead_local);
                            return found;
                        }
                        return nullptr;
                    }

                    // 009C1BC0 with 009BECD0, 007EF2C0 and 009BE3E0 (packet
                    // cc9_dogfight_moveto). `sep` is the moveto target's planar
                    // range, the argument 009C18C0 passes (009C198A-009C1999).
                    void df_moveto_speed_009c1bc0(float sep) {
                        float w1 = 3000.0f, w2 = 5000.0f;
                        float dont_wait = 0.87266463f, wait = 1.7453293f, good = 100.0f, nearby = 200.0f;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            w1 = g.pilot_general_wingmen_wait_dist_1;
                            w2 = g.pilot_general_wingmen_wait_dist_2;
                            dont_wait = g.pilot_follow_dont_wait_for_hdg_diff;
                            wait = g.pilot_follow_wait_for_hdg_diff;
                            good = g.pilot_follow_good_position_dist;
                            nearby = g.pilot_follow_nearby_dist;
                        }
                        const bsp::PlaneSquadronHostRecord* sq =
                            bsp::plane_squadron_registry().find_by_member_unit(unit_.process_index);
                        float wingmen = 1.0f;
                        if (sq != nullptr) {
                            std::vector<float> values;
                            for (const std::size_t m : sq->member_units) {
                                float v = -1.0f;   // 007BCC20: dead, leader, no pilot
                                if (m != bsp::kPlaneSquadronNoUnit && m < owner_.slots.size() &&
                                    m != unit_.process_index) {
                                    const GameUnitSlot& o = *owner_.slots[m];
                                    // 00999AE0 -> 009A9C60: the follow state's value
                                    // while the member's dogfight task is in follow.
                                    if (df_slot_live(o) && o.dogfight_task_installed &&
                                        o.dogfight_state == bsp::DogfightState::kFollow &&
                                        o.df_station_valid) {
                                        const float d[3] = {
                                            o.motion.position[0] - o.df_station_world[0],
                                            o.motion.position[1] - o.df_station_world[1],
                                            o.motion.position[2] - o.df_station_world[2]};
                                        // state+85h is not modelled: taken as clear.
                                        v = bsp::follow_wait_value_009be3e0(
                                            false, d, o.df_leader_heading, dont_wait, wait, good, nearby);
                                    }
                                }
                                values.push_back(v);
                            }
                            wingmen = bsp::squadron_wingmen_value_007ef2c0(
                                values.data(), static_cast<int>(values.size()),
                                sq->formation_shape_3e4);
                        }
                        const float speed = bsp::dogfight_moveto_speed_009becd0(
                            unit_.plane_max_spd, owner_.bot_desired_speed_007c47f0(unit_), sep,
                            w1, w2, sq != nullptr, wingmen);
                        unit_.plane_desired_speed_2b4 = speed;       // 009C1C07
                        unit_.plane_trg_speed_corr_off_2b0 = 0;      // 009C1C0D, byte
                        unit_.plane_air_brake_mode_2d8 = 1;          // 009C1C14, dword
                        ++unit_.df_moveto_speed_commands;
                        if (speed < unit_.df_moveto_speed_min) unit_.df_moveto_speed_min = speed;
                        if (speed > unit_.df_moveto_speed_max) unit_.df_moveto_speed_max = speed;
                        owner_.done("BotStateMoveTo::dogfight_speed_009c1bc0", 0x009c1bc0u);
                    }

                    void df_arm_body_009ab1c0(float dt) {
                        // Install trigger. The image builds the kind-2 task through
                        // 0099A170 from the class 007EEC50 chose (00E08F58). A
                        // scene-issued `dogfight` order reaches this host's
                        // director as the resolved command token and never sets
                        // attack_command_class. The token is the class by name:
                        // scene command type 13 is object 00E08F58, whose name
                        // getter 006F8790 returns the literal `dogfight`
                        // (docs/SCENE_COMMAND_TYPES.md), so a scene record naming
                        // `dogfight` queues that object (packet
                        // cc9_dogfight_engaged). Run G1 showed the class test
                        // alone never fires in USN04.
                        if (unit_.attack_command_class != bsp::kDogfightCommandClass &&
                            unit_.row.command != "dogfight") {
                            return;
                        }
                        const bool leader =
                            owner_.unit_is_flight_leader_007b8ad0(unit_.process_index);
                        const bsp::DogfightState was = unit_.dogfight_state;
                        if (!unit_.dogfight_task_installed) {
                            unit_.dogfight_task_installed = true;
                        }
                        float attack_dist = 2000.0f;
                        float ceiling = 3000.0f;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            attack_dist = g.pilot_dogfight_attack_dist;
                            ceiling = g.dynamics_ceiling;
                        }

                        if constexpr (GameUnitsHost::Impl::kDogfightEngagedBound) {
                            df_approach_update_009aac70(dt, attack_dist);
                            // squadron+370h. The dogfight task's vtable+38h
                            // (00D1F9E8) is 0099B710 (MOV AL,1), so the leader's
                            // 0099B740 sets the squadron's mode to 1 every think
                            // (007ED3F0 at 0099B774) and ENG is the latch alone.
                            bsp::DogfightTransitionInputs ti;
                            ti.current = unit_.dogfight_state;
                            ti.latch_4c8 = unit_.df_latch_d0;
                            ti.squadron_mode_370 = 1;
                            ti.target_squadron_4c4 = unit_.df_target_squadron;
                            ti.is_flight_leader = leader;
                            // 009AAA80: SUBSTITUTION, labelled. Its gate is shut
                            // after aim (the aim tick writes +DCh = 0), but the
                            // maneuver tick opens it (+DCh = 1.5, +E0h = 0.1 at
                            // 009A9270) and 007B96F0's search is unread, so false
                            // drops maneuver's early edge to aim.
                            ti.aaa80 = false;
                            if constexpr (GameUnitsHost::Impl::kDogfightEarlyEdgeBound) {
                                // 009AB047: only asked off aim, with ENG and a non-zero mode.
                                if (unit_.dogfight_state != bsp::DogfightState::kAim &&
                                    unit_.dogfight_state != bsp::DogfightState::kPrepare &&
                                    unit_.dogfight_state != bsp::DogfightState::kMoveTo &&
                                    unit_.dogfight_state != bsp::DogfightState::kFollow &&
                                    bsp::dogfight_engaged_009aafa0(unit_.df_latch_d0, 1,
                                                                   unit_.df_target_squadron)) {
                                    ti.aaa80 = df_early_edge_009aaa80();
                                }
                            }
                            ti.aim_too_close_6a0 = unit_.df_aim_state.too_close_24;
                            ti.aim_bored_98f0 =
                                unit_.df_aim_state.bored_18 > unit_.df_aim_state.bored_limit_1c;
                            ti.maneuver_on_target_9bd0 = bsp::dogfight_on_target_009a9bd0(
                                unit_.df_local_ec[2], unit_.df_tan_f8[0], unit_.df_tan_f8[1]);
                            ti.avoid_timer = unit_.df_avoid_timer;
                            // 009A9970: the weights 009A7F70/009A83B0 are only
                            // partly read; avoid_roll stands in. Labelled.
                            ti.avoid_pick_turn = false;
                            const bsp::DogfightTransition tr = bsp::dogfight_transition_009aafa0(ti);
                            unit_.dogfight_state = tr.next;
                            if (tr.maneuver_from_aim_8560) {
                                // 009A8560, draw 00BD2F10(0.3, 1.1) at its midpoint.
                                unit_.df_maneuver_range_34 = bsp::dogfight_maneuver_pursuit_range_009a8560(
                                    unit_.df_row.aim_shoot_distance, 0.7f);
                            }
                            if (unit_.dogfight_state != was) {
                                // 009A9D50: enter the new state.
                                if (unit_.dogfight_state == bsp::DogfightState::kAim) {
                                    // Draws 00BD2F10(0.8, 1.4) and (0.5, 0.8) at midpoints.
                                    unit_.df_aim_state = bsp::dogfight_aim_enter_009a75c0(
                                        unit_.df_row, 1.0f, 1.1f, 0.65f);
                                } else if (unit_.dogfight_state == bsp::DogfightState::kAvoidRoll ||
                                           unit_.dogfight_state == bsp::DogfightState::kAvoidTurn) {
                                    // Draw 00BD2F10(0.75, 1.5) at its midpoint.
                                    unit_.df_avoid_timer = bsp::dogfight_avoid_timer_009a7de0(
                                        unit_.df_row.avoid_time, 1.125f);
                                    unit_.df_avoid_sign =
                                        unit_.plane_bank_angle_c68 > 0.0f ? 1.0f : -1.0f;
                                }
                            }
                        } else {
                            // 009AAFA0 unengaged arm only.
                            unit_.dogfight_state = bsp::dogfight_unengaged_state_009aafa0(leader);
                        }
                        if (unit_.dogfight_state != was) {
                            ++unit_.df_transitions;
                            owner_.log.notef("  dogfight %-12s %s -> %s d=%.1f",
                                unit_.row.name.c_str(),
                                bsp::dogfight_state_name(was),
                                bsp::dogfight_state_name(unit_.dogfight_state),
                                static_cast<double>(unit_.df_distance_d4));
                        }
                        ++unit_.df_state_ticks[static_cast<int>(unit_.dogfight_state)];
                        if (unit_.df_latch_d0) ++unit_.df_latched_ticks;

                        if (unit_.dogfight_state == bsp::DogfightState::kFollow) {
                            // The generic follow state, tick 009C1FD0: station
                            // point, then the fly-to law, as the dive-bomb follow
                            // tick does.
                            bsp::PlaneFormationStation station;
                            const GameUnitSlot* fl = nullptr;
                            owner_.place_wing_member_on_station_007f23a0(
                                unit_, false, &station, &fl);
                            if (owner_.kPlaneFollowLawEnabled && station.produced &&
                                fl != nullptr) {
                                owner_.run_follow_law_009bfee0_009bee30(unit_, station, *fl);
                            }
                            unit_.df_station_valid = station.produced && fl != nullptr;
                            if (unit_.df_station_valid) {
                                for (int i = 0; i < 3; ++i) unit_.df_station_world[i] = station.world[i];
                                unit_.df_leader_heading = fl->plane_heading_c6c;
                            }
                            owner_.record("BotTaskDogfight::follow", 0x009c1fd0u);
                            return;
                        }

                        if (unit_.dogfight_state == bsp::DogfightState::kAim) {
                            // 009A76E0.
                            const GameUnitSlot& tgt = *owner_.slots[unit_.df_target_plus_one - 1];
                            const float* mo = unit_.world.data();
                            const float* mt = tgt.world.data();
                            bsp::DogfightAimInputs ai;
                            ai.distance = unit_.df_distance_d4;
                            ai.tan_len = static_cast<float>(std::sqrt(
                                static_cast<double>(unit_.df_tan_f8[0]) * unit_.df_tan_f8[0] +
                                static_cast<double>(unit_.df_tan_f8[1]) * unit_.df_tan_f8[1]));
                            ai.local_z = unit_.df_local_ec[2];
                            // vt[34h] on both units; the forward rows stand in.
                            ai.opposing = mo[8] * mt[8] + mo[9] * mt[9] + mo[10] * mt[10] < 0.0f;
                            if constexpr (GameUnitsHost::Impl::kDogfightHeadOnVelocityBound) {
                                // 007BBB70 copies unit+AC8h..AD0h, the world linear
                                // velocity (docs/PLANE_FOLLOW_HOLD_ARM.md: ctl+18h).
                                const float* vo = unit_.plane_world_velocity;
                                const float* vt = tgt.plane_world_velocity;
                                ai.opposing = vo[0] * vt[0] + vo[1] * vt[1] + vo[2] * vt[2] < 0.0f;
                            }
                            // (approach+1Ch)+48h: last tick's fire flag from 009FC7C0.
                            unit_.df_dc = 0.0f;    // 009A76E0: approach+DCh = 0
                            unit_.df_e0 = 1.0f;    // approach+E0h = 1.0
                            ai.gun_locked_48 = GameUnitsHost::Impl::kDogfightGunBound &&
                                               unit_.df_gun.fire_48;
                            {
                                const auto& v = tgt.motion.linear_velocity;
                                ai.target_speed = static_cast<float>(std::sqrt(
                                    static_cast<double>(v.x) * v.x + static_cast<double>(v.y) * v.y +
                                    static_cast<double>(v.z) * v.z));
                            }
                            ai.dt = dt;
                            const bsp::DogfightAimCommand c =
                                bsp::dogfight_aim_tick_009a76e0(unit_.df_aim_state, ai, unit_.df_row);
                            const float h = bsp::heading_command_009f9e40(
                                unit_.df_aim[0], unit_.df_aim[2],
                                unit_.motion.position[0], unit_.motion.position[2]);
                            const float d = unit_.df_horizontal_d8 > 1.0f ? unit_.df_horizontal_d8 : 1.0f;
                            const float p = bsp::pitch_command_to_point_009f9ed0(
                                unit_.df_aim[1] - unit_.motion.position[1], d,
                                unit_.plane_climb_angle_1e4);
                            df_steer(h, p, unit_.df_aim[1]);
                            if (!c.speed_from_target) {
                                ++unit_.df_head_on_ticks;
                                if (c.head_on_fraction < unit_.df_head_on_throttle_min) {
                                    unit_.df_head_on_throttle_min = c.head_on_fraction;
                                }
                            }
                            if (c.speed_from_target) {
                                unit_.plane_desired_speed_2b4 = c.desired_speed;
                                unit_.plane_air_brake_mode_2d8 = 1;
                            } else if constexpr (GameUnitsHost::Impl::kDogfightThrottleBound) {
                                // 007B4ED0: direct throttle, speed mode +2D8h = 0.
                                const bsp::DogfightThrottle th =
                                    bsp::dogfight_throttle_007b4ed0(c.head_on_fraction);
                                unit_.plan_slots[bsp::kPilotSlotThrottle].desired = th.throttle;
                                unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = th.air_brake;
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                                unit_.plane_air_brake_mode_2d8 = 0;
                            } else {
                                unit_.plane_desired_speed_2b4 = c.head_on_fraction * unit_.plane_max_spd;
                                unit_.plane_air_brake_mode_2d8 = 1;
                            }
                            if (unit_.df_distance_d4 < unit_.df_row.aim_shoot_distance &&
                                ai.tan_len < 0.25f) {
                                ++unit_.df_gun_window_ticks;
                            }
                            owner_.record("BotStateDogfightAim::tick", 0x009a76e0u);
                            return;
                        }

                        if (unit_.dogfight_state == bsp::DogfightState::kAvoidRoll ||
                            unit_.dogfight_state == bsp::DogfightState::kAvoidTurn) {
                            // STAND-IN, labelled: 009A7E80/009A80E0 call 009A7A50(dt)
                            // and are unread past it; the timer runs down here and
                            // the heading is held 90 degrees off the target bearing.
                            unit_.df_avoid_timer -= dt;
                            float h = bsp::heading_command_009f9e40(
                                unit_.df_aim[0], unit_.df_aim[2],
                                unit_.motion.position[0], unit_.motion.position[2]);
                            h += unit_.df_avoid_sign * 1.5707964f;
                            const float two_pi = 6.2831855f;
                            if (h < 0.0f) h += two_pi;
                            if (h >= two_pi) h -= two_pi;
                            const float p = bsp::pitch_command_to_point_009f9ed0(
                                0.0f, 1000.0f, unit_.plane_climb_angle_1e4);
                            df_steer(h, p, unit_.motion.position[1]);
                            owner_.record("BotStateDogfightAvoid::standin", 0x009a7e80u);
                            return;
                        }

                        if (unit_.dogfight_state == bsp::DogfightState::kManeuver ||
                            unit_.dogfight_state == bsp::DogfightState::kAttackRun ||
                            unit_.dogfight_state == bsp::DogfightState::kPrepare) {
                            // STAND-IN, labelled: 009A8B20's mode-0 arm (attackrun
                            // and prepare are unreachable while the mode is 1).
                            const bsp::DogfightSteer s = bsp::dogfight_maneuver_standin(
                                unit_.motion.position, unit_.df_aim, unit_.df_horizontal_d8,
                                unit_.df_row.aim_shoot_distance, ceiling,
                                unit_.plane_climb_angle_1e4);
                            df_steer(s.heading, s.pitch, unit_.df_aim[1]);
                            if constexpr (GameUnitsHost::Impl::kDogfightThrottleBound) {
                                if (unit_.dogfight_state == bsp::DogfightState::kManeuver) {
                                    // 009A9270: +278h = 1.0, +2A8h = 0, both active, +2D8h = 0.
                                    const bsp::DogfightThrottle th =
                                        bsp::dogfight_throttle_007b4ed0(1.0f);
                                    unit_.plan_slots[bsp::kPilotSlotThrottle].desired = th.throttle;
                                    unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                                    unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = th.air_brake;
                                    unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                                    unit_.plane_air_brake_mode_2d8 = 0;
                                }
                            }
                            if (unit_.dogfight_state == bsp::DogfightState::kManeuver) {
                                unit_.df_dc = 1.5f;   // 009A9270 tail: 00CE380C
                                unit_.df_e0 = 0.1f;   // 00D7A2F0
                            }
                            owner_.record("BotStateDogfightManeuver::standin", 0x009a8b20u);
                            return;
                        }

                        // moveto: STAND-IN for 009C18C0 with the dogfight cruise
                        // profile (009AAF30) and speed slot 009C1BC0, both unread.
                        if (unit_.command_target_plus_one == 0) return;
                        const std::size_t ti = unit_.command_target_plus_one - 1;
                        if (ti >= owner_.slots.size()) return;
                        bsp::DogfightMoveToInputs din;
                        for (int i = 0; i < 3; ++i) {
                            din.own_pos[i] = unit_.motion.position[i];
                            din.target_pos[i] = owner_.slots[ti]->motion.position[i];
                        }
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            din.cruising_alt = g.pilot_dogfight_cruising_alt;
                            din.min_distance = g.pilot_follow_followed_point_dist;
                        }
                        din.class_climb_angle_1e4 = unit_.plane_climb_angle_1e4;
                        const bsp::DogfightMoveToCommand c = bsp::dogfight_moveto_standin(din);
                        unit_.plan_heading_2c0 = c.heading;
                        unit_.plan_heading_2c0_written = true;
                        unit_.plan_heading_mode_2cc = 2;
                        if constexpr (GameUnitsHost::Impl::kDogfightMovetoGenericBound) {
                            // Packet cc9_dive_modes: the image runs the GENERIC
                            // move-to tick 009C18C0 here. The dogfight moveto is
                            // 009C2CA0 (vtable 00D20B24: +Ch 009C18C0, +1Ch
                            // 009C1BC0), built once at 009A955B with the ranges
                            // +30h = 500.0 (00CE397C), +34h = 100.0 (00CE3D08),
                            // +38h = 1000.0 (00CE3804); the dogfight arm 009AB1C0
                            // never calls 009BDE80, so they stay fixed. Steps 3-5
                            // as the dive-bomb tick runs them; the heading stays
                            // 009F9E40 toward the target, as above.
                            bsp::MoveToGlideInputs gin;
                            gin.near_range_30 = 500.0f;
                            gin.far_range_34 = 100.0f;
                            gin.speed_range_38 = 1000.0f;
                            gin.target_world_y = din.target_pos[1];
                            gin.unit_world_y = unit_.motion.position[1];
                            gin.planar_distance = c.horizontal_range;
                            const bsp::MoveToGlideCommand g = bsp::move_to_glide_009c18c0(gin);
                            bsp::PlaneCruiseAltitudeInputs cin;
                            cin.base_altitude = g.base;
                            cin.range_low = g.range_low;
                            cin.range_high = g.range_high;
                            cin.scale = g.scale;
                            cin.class_gain = static_cast<float>(
                                std::tan(static_cast<double>(unit_.plane_drop_angle)));
                            cin.has_squadron = false;
                            if (owner_.lua.plane_globals_loaded()) {
                                cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                            }
                            const bsp::PlaneCruiseAltitudeResult ca =
                                bsp::cruise_altitude_command_009fba50(cin);
                            bsp::PlanePitchCommandInputs pin;
                            pin.desired_altitude = ca.clamped_altitude;
                            pin.reference = ca.pitch_reference;
                            pin.unit_world_y = unit_.motion.position[1];
                            pin.ceiling = cin.ceiling;
                            if (owner_.lua.plane_globals_loaded()) {
                                const bsp::GameTuningBlock& gt = owner_.lua.plane_globals();
                                pin.climb_dist = gt.pilot_general_climb_dist;
                                pin.drop_dist = gt.pilot_general_drop_dist;
                            }
                            pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                            pin.class_drop_angle = unit_.plane_drop_angle;
                            unit_.plane_commanded_altitude = ca.clamped_altitude;
                            unit_.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
                            unit_.plan_state.pitch_target_2bc = unit_.plane_commanded_pitch;
                            unit_.plan_state.pitch_mode_2d0 = 2;
                            unit_.df_moveto_cmd_alt_last = ca.clamped_altitude;
                        } else {
                        unit_.plane_commanded_altitude = din.cruising_alt;
                        unit_.plane_commanded_pitch = c.pitch;
                        unit_.plan_state.pitch_target_2bc = c.pitch;
                        unit_.plan_state.pitch_mode_2d0 = 2;
                        }
                        if (unit_.df_min_target_range < 0.0f ||
                            c.horizontal_range < unit_.df_min_target_range) {
                            unit_.df_min_target_range = c.horizontal_range;
                        }
                        if (c.horizontal_range < attack_dist) {
                            ++unit_.df_within_attack_dist_ticks;
                        }
                        if constexpr (GameUnitsHost::Impl::kDogfightMovetoSpeedBound) {
                            df_moveto_speed_009c1bc0(c.horizontal_range);
                        }
                        owner_.record("BotTaskDogfight::moveto_standin", 0x009c18c0u);
                    }

                    void run_dive_bomb_task_arm_009c8790(float dt) {
                        // 0099A170 builds a task from the class 007EEC50
                        // chose, so the only correct test is that the class IS
                        // the divebomb one. An earlier revision gated on
                        // "has a commanded target and carries bomb ordnance",
                        // which installed the task on 27 IJN01 aircraft the
                        // image gives `attackmove`: their commanded target is
                        // their authored `moveto` row, not an attack order.
                        // docs/DIVE_BOMB_TASK.md, "Gate 1".
                        if (!bsp::dive_bomb_task_installed_for_class(
                                unit_.attack_command_class)) {
                            return;
                        }
                        if (!unit_.dive_bomb_task_installed) {
                            unit_.dive_bomb_task_installed = true;
                            // 009C7710 leaves +310h on the moveto/follow pair
                            // 007B8AD0 picks: 009C7777 CALL / 009C777C TEST AL,AL
                            // / 009C777E LEA ECX,[ESI+4F0h] / 009C7784 JNZ over
                            // 009C7786 LEA ECX,[ESI+52Ch]. FED, packet
                            // cc8_follow_enter: the predicate is the flight-leader
                            // test, so the leader constructs into moveto and every
                            // wing member constructs into FOLLOW.
                            unit_.dive_bomb_state =
                                owner_.unit_is_flight_leader_007b8ad0(
                                    unit_.process_index)
                                    ? bsp::DiveBombState::kMoveTo
                                    : bsp::DiveBombState::kFollow;
                            // 009C3EA0, the approach seed. approach+ACh is
                            // tuning+4CCh Pilot/DiveBomb/BeginAltRange/1 and
                            // approach+B0h is tuning+4D0h - tuning+4CCh.
                            unit_.db_begin_alt_ac = GameUnitsHost::Impl::kPilotDiveBombBeginAltRange1;
                            unit_.db_alt_span_b0 =
                                GameUnitsHost::Impl::kPilotDiveBombBeginAltRange2 - GameUnitsHost::Impl::kPilotDiveBombBeginAltRange1;
                            // PRODUCER NOW READ, packet cc8_approach_base_ctor.
                            // 009F9D22 points approach+14h at
                            // &PilotBotConfig.levels[[[unit+DF4h]+34h]], and
                            // 009C3F29 draws approach+A8h as
                            // Uniform(row+38h, row+3Ch), which
                            // include/bsp/robot_config.hpp names
                            // dive_bomb_release_alt_1_044 and _2_048.
                            //
                            // SUBSTITUTION, labelled, replacing the old 800.
                            // The PilotBot registry is out of this host's reach,
                            // exactly as the torpedo profile records above, so
                            // the values come from the installed
                            // scripts/datatables/robots.lua SPNormal row, which
                            // authors DiveBombReleaseAlt = { 350, 450 }. The
                            // row's own Hungarian comment says the release
                            // altitude is "valahol a ketto kozott", somewhere
                            // between the two, which is the uniform draw.
                            // The difficulty index is unmodelled, so SPNormal is
                            // picked and named, as the torpedo profile does.
                            // BOUND, packet cc9_difficulty: 009F9D22 captures
                            // the row once, from the pilot bot's index.
                            unit_.db_skill_row_14 = unit_.pilot_skill_index;
                            const GameUnitsHost::Impl::PilotDiveBombRow& db_row =
                                GameUnitsHost::Impl::dive_bomb_row(unit_);
                            unit_.db_dive_alt_a8 =
                                db_row.release_alt_1_044;   // Uniform low, 009C3F23
                            if constexpr (GameUnitsHost::Impl::kReleaseAltitudeDrawBound) {
                                // 009C3F1C/009C3F23 push row+3Ch then row+38h;
                                // 009C3F29 00BD2F10(low, high); 009C3F2E stores it.
                                unit_.db_dive_alt_a8 = owner_.release_altitude_draw_00bd2f10(
                                    unit_.row.name, db_row.release_alt_1_044,
                                    db_row.release_alt_2_048);
                            }
                            // The aimdive interpolation endpoints, the same row:
                            // dive_bomb_aim_prec_dist_068 and _mul_06c. The
                            // row's comments name them exactly: "tavolrol
                            // ennyivel melle celoz", from far away it aims this
                            // much beside the target, and "celzasi pontossag
                            // szorzo. minel kisebb, annal jobb", the aiming
                            // accuracy multiplier, smaller is better. So the
                            // 25-metre gate at 00CE3880 measures an authored
                            // miss, and this is where it comes from.
                            unit_.db_lead_high_5c = db_row.aim_prec_dist_068;
                            unit_.db_gain_high_60 = db_row.aim_prec_mul_06c;
                            // approach+B8h == task+4B0h. 009C3F1C seeds it from
                            // Random * (approach+8h)->+268h, unread here, and
                            // 009C8A5E clamps it every tick to
                            // max(itself, Pilot/DiveBomb/AttackDist * task+41Ch).
                            // The clamp alone is a floor, and that is what the
                            // host applies.
                            // BOUND, packet cc8_dive_race. 009C3EA0 draws the
                            // two separately off the class's turn circle, and
                            // the constructor listing settles which register
                            // carries it - `009C3F5D MOV EBP,[ESI+8]` is the
                            // only write to EBP between the 0042E740 tuning
                            // load and `009C3F86 FMUL [EBP+268h]`, and
                            // `009C3F8C` reloads the same pointer for
                            // `009C3FB5`:
                            //   +B4h = uniform(0.6, 0.8) * desc+268h   009C3F97
                            //   +B8h = +BCh = uniform(1.6, 1.8) * ...  009C3FE8/FF5
                            // (00CE3D30 0.6, 00CE74F8 0.8, 00D06BB4 1.6,
                            // 00CF4848 1.8, all `FLD float ptr`; the second
                            // draw is ONE call stored twice, `FST` then `FSTP`).
                            // Pinned at the low end of each draw, as this host
                            // pins every draw. 009C8A5E's floor
                            // max(itself, AttackDist * task+41Ch) = 1100 is then
                            // inert, which is why it was mistaken for the value.
                            {
                                const float r = unit_.plane_turn_circle_radius;
                                if (r > 0.0f) {
                                    unit_.db_in_range_b8 = 1.6f * r;
                                    unit_.db_attack_dist_b4 = 0.6f * r;
                                } else {
                                    unit_.db_in_range_b8 =
                                        GameUnitsHost::Impl::kPilotDiveBombAttackDist;
                                    unit_.db_attack_dist_b4 =
                                        GameUnitsHost::Impl::kPilotDiveBombAttackDist;
                                }
                            }
                            // approach+D4h RECOVERED. The approach constructor
                            // 009C3EA0 - the routine that writes the vtable
                            // 00D20C48 at 009C3EE2 and draws +A8h at 009C3F2E
                            // and +ACh at 009C3F3F - closes with
                            // 009C3FFB-009C4045:
                            //
                            //   S24 = (+ACh + +A8h) * 0.5      (00D7A280)
                            //   S20 = +A8h + 250.0             (00CF8850)
                            //   +D4h = max(S20, S24)           (009C4035 `76` JBE)
                            //
                            // ESI is `this` from 009C3EB8 and is never
                            // reassigned, so both reads are the approach's own
                            // fields. With the drawn 350.0 and the 1000.0
                            // begin-altitude that is 675.0 m, and 009C680E's
                            // can-dive test becomes a real height gate rather
                            // than "above the target".
                            unit_.db_release_range_d4 =
                                bsp::dive_bomb_dive_entry_height_009c4045(
                                    unit_.db_dive_alt_a8, unit_.db_begin_alt_ac);
                            // SUBSTITUTION, labelled: approach+50h and the two
                            // interpolation endpoints (approach+14h)->+5Ch/+60h
                            // have no producer read. Zero leaves the lead at 0
                            // and the gain at 1, so the aim error is the bare
                            // along-track miss the release gate compares
                            // against 25 m.
                            //
                            // +50h was searched for this packet and is NOT the
                            // store at 009C3E2E that a `+50h` census turns up:
                            // 009C3DAD `MOV EDI,ECX` makes EDI the approach and
                            // 009C3E11 `LEA ESI,[EDI+30h]` rebases ESI, so that
                            // store lands on approach+80h. No writer through
                            // the approach base exists in the dive-bomb range.
                            unit_.db_aim_point_height_50 = 0.0f;
                            // 007C1DB0 at the aimglide enter 009C4F00 latches
                            // the count the salvo caps against.
                            unit_.dive_bomb_rounds_remaining =
                                GameUnitsHost::Impl::kDiveBombCarriedRoundsSubstitute;
                            owner_.log.notef("dive-bomb task 009C8C70 kind 8 installed "
                                "for an ordered aircraft; arm 009C8790 runs on the pilot "
                                "think; approach+ACh=%.1f (BeginAltRange/1) "
                                "approach+A8h=%.1f (substituted) approach+B8h=%.1f "
                                "(Pilot/DiveBomb/AttackDist floor from 009C8A5E) "
                                "rounds=%d",
                                static_cast<double>(unit_.db_begin_alt_ac),
                                static_cast<double>(unit_.db_dive_alt_a8),
                                static_cast<double>(unit_.db_in_range_b8),
                                unit_.dive_bomb_rounds_remaining);
                        }
                        // 0099B757 `CMP EAX,[ECX+3D0h]` compares this unit
                        // against the dword at squadron+3D0h, which is element
                        // 0 of the member array: the flight leader. Corroborated
                        // at 009C7C5D, where the same dword is dereferenced as a
                        // unit and read for its position at +FCh/+104h. Only the
                        // leader's think writes the attack mode.
                        if (!unit_.db_flight_lead_resolved) {
                            auto* const sqn =
                                bsp::plane_squadron_registry().find_by_member_unit(
                                    unit_.process_index);
                            if (sqn != nullptr) {
                                for (const std::size_t member : sqn->member_units) {
                                    if (member == bsp::kPlaneSquadronNoUnit) continue;
                                    unit_.db_is_flight_lead =
                                        (member == unit_.process_index);
                                    break;
                                }
                                unit_.db_flight_lead_resolved = true;
                            }
                        }
                        // 0099993C: before the task arm, once per think.
                        run_dive_bomb_attack_mode_tick_0099b740();
                        DiveBombArmBinding binding(owner_, unit_);
                        bsp::DiveBombTaskContext ctx;
                        ctx.task = &unit_;
                        ctx.approach = &unit_;
                        ctx.unit = &unit_;
                        ctx.command_block = &unit_;
                        ctx.pilot_control_block = &unit_;
                        ctx.state = &unit_;
                        ctx.current = unit_.dive_bomb_state;
                        const bsp::DiveBombState before = ctx.current;
                        // Packet cc8_dive_release item 1: the three flags the
                        // 009C8634 arm reads, sampled before the arm so the
                        // same tick's 009C58D0 cannot overwrite them.
                        const bool pre_alive_19 = unit_.db_aim_alive_19;
                        const bool pre_pull_18 = unit_.db_aim_pull_out_18;
                        const bool pre_bomb_d1 = unit_.db_has_bomb_d1;
                        const bsp::DiveBombArmTickResult r =
                            bsp::dive_bomb_task_arm_009c8790(binding, ctx, dt);
                        (void)r;
                        unit_.dive_bomb_state = ctx.current;
                        // Packet cc8_dive_entry: the altitude at each hand-over.
                        if (ctx.current != before &&
                            unit_.db_transition_count <
                                GameUnitSlot::kDbTransitions) {
                            GameUnitSlot::DbTransition& t =
                                unit_.db_transitions[unit_.db_transition_count++];
                            t.tick = unit_.dive_bomb_arm_ticks;
                            t.from = static_cast<signed char>(
                                dive_bomb_state_bucket(before));
                            t.to = static_cast<signed char>(
                                dive_bomb_state_bucket(ctx.current));
                            t.alt = unit_.motion.position[1];
                            t.range = unit_.db_planar_bc;
                            t.cmd_alt = unit_.plane_commanded_altitude;
                            t.span = unit_.db_flyabove_span;
                            t.b_height = unit_.db_flyabove_height;
                            t.f18 = unit_.db_flyabove_can_dive_18 ? 1 : 0;
                            t.f19 = unit_.db_flyabove_ready_19 ? 1 : 0;
                        }
                        if (before == bsp::DiveBombState::kAimDive &&
                            ctx.current != bsp::DiveBombState::kAimDive) {
                            ++unit_.db_aimdive_exits;
                            if (!pre_alive_19) ++unit_.db_aimdive_exit_alive;
                            else if (!pre_bomb_d1) ++unit_.db_aimdive_exit_nobomb;
                            else if (pre_pull_18) ++unit_.db_aimdive_exit_pullout;
                            else ++unit_.db_aimdive_exit_other;
                            unit_.db_aimdive_last_run = unit_.db_aimdive_run_ticks;
                            if (unit_.db_aimdive_first_run < 0) {
                                unit_.db_aimdive_first_run = unit_.db_aimdive_run_ticks;
                            }
                            unit_.db_aimdive_run_ticks = 0;
                        }
                        // Packet cc8_attack_mode item 2. The edge out of the
                        // two aim states, with every input of the two
                        // predicates 009C83E0 consults, so the before/after can
                        // be read off the log rather than inferred.
                        if ((before == bsp::DiveBombState::kAimDive ||
                             before == bsp::DiveBombState::kAimGlide) &&
                            ctx.current != before &&
                            unit_.db_spent_exit_logs < 12) {
                            ++unit_.db_spent_exit_logs;
                            static const char* const kNames[10] = {
                                "moveto", "follow", "prepare", "done", "goaway",
                                "aimdive", "aimglide", "flyabove", "turndown",
                                "attackrun"};
                            const int bb = dive_bomb_state_bucket(before);
                            const int ba = dive_bomb_state_bucket(ctx.current);
                            owner_.log.notef(
                                "db aim exit %s: %s -> %s tick=%d alt=%.1f "
                                "lead=%d engaged=%d latch_4c8=%d mode_370=%d "
                                "tgt_440=%d bomb_4c9=%d breakoff=%d d=%.1f "
                                "thr=%.1f R_b8=%.1f",
                                unit_.row.name.c_str(),
                                bb >= 0 ? kNames[bb] : "?",
                                ba >= 0 ? kNames[ba] : "?",
                                unit_.dive_bomb_arm_ticks,
                                static_cast<double>(unit_.motion.position[1]),
                                unit_.db_is_flight_lead ? 1 : 0,
                                unit_.db_dbg_engaged ? 1 : 0,
                                unit_.db_in_range_d0 ? 1 : 0,
                                static_cast<int>(unit_.db_attack_mode_370),
                                unit_.command_target_plus_one != 0 ? 1 : 0,
                                pre_bomb_d1 ? 1 : 0,
                                unit_.db_dbg_break_off ? 1 : 0,
                                static_cast<double>(unit_.db_dbg_break_off_distance),
                                static_cast<double>(unit_.db_dbg_break_off_threshold),
                                static_cast<double>(unit_.db_in_range_b8));
                        }
                        // What a spent bomber does after it is put down: how
                        // long it sits in kDone, the lowest it gets there, and
                        // whether the approach edge ever fires for it.
                        if (ctx.current == bsp::DiveBombState::kDone) {
                            ++unit_.db_post_done_ticks;
                            const float alt = unit_.motion.position[1];
                            if (unit_.db_post_done_min_alt < 0.0f ||
                                alt < unit_.db_post_done_min_alt) {
                                unit_.db_post_done_min_alt = alt;
                            }
                        }
                        if ((ctx.current == bsp::DiveBombState::kMoveTo ||
                             ctx.current == bsp::DiveBombState::kFollow) &&
                            before != ctx.current) {
                            ++unit_.db_approach_returns;
                            if (unit_.db_approach_return_tick < 0) {
                                unit_.db_approach_return_tick =
                                    unit_.dive_bomb_arm_ticks;
                            }
                        }
                        if (ctx.current == bsp::DiveBombState::kAimDive) {
                            ++unit_.db_aimdive_run_ticks;
                        }
                        ++unit_.dive_bomb_arm_ticks;
                        {
                            const int m = static_cast<int>(unit_.db_attack_mode_370);
                            if (m >= 0 && m <= 2) ++unit_.db_mode_ticks[m];
                        }
                        const int b = dive_bomb_state_bucket(ctx.current);
                        if (b >= 0) ++unit_.dive_bomb_state_ticks[b];
                        if (!unit_.db_in_range_d0) ++unit_.db_blocked_no_latch;
                        if (!unit_.db_has_bomb_d1) ++unit_.db_blocked_no_bomb;
                        if (ctx.current == bsp::DiveBombState::kAimDive &&
                            before != bsp::DiveBombState::kAimDive) {
                            unit_.db_dive_entry_alt = unit_.motion.position[1];
                            unit_.db_dive_entry_pitch = unit_.plane_commanded_pitch;
                            // 009C5876/009C5885: the enter clears the re-arm
                            // timer and sets +18h from the bomb flag, and
                            // 009C588B clears unit+844h.
                            unit_.db_aim_rearm_1c = 0.0f;
                            unit_.db_aim_pull_out_18 = !unit_.db_has_bomb_d1;
                            unit_.db_aim_alive_19 = true;
                            ++unit_.db_aimdive_entries;
                        }
                        // 009C883D-009C884C. The image's arm has NO per-state
                        // chain: it ends with `MOV ECX,[ESI+310h] / MOV EAX,[ECX]
                        // / MOV EDX,[EAX+0Ch] / CALL EDX`, one virtual tick on
                        // whatever state the transition left. moveto task+4F0h
                        // and follow task+52Ch are states like any other and
                        // were the only two this host never dispatched, so a
                        // dive bomber in the approach flew whatever command the
                        // previous state had left behind. docs/DIVE_BOMB_APPROACH.md.
                        if (ctx.current == bsp::DiveBombState::kMoveTo) {
                            run_dive_bomb_move_to_tick_009c18c0();
                        }
                        if (ctx.current == bsp::DiveBombState::kFollow) {
                            run_dive_bomb_follow_tick_009c1fd0();
                        }
                        if (ctx.current == bsp::DiveBombState::kAttackRun) {
                            run_dive_bomb_attackrun_tick_009c4220(dt);
                        }
                        if (ctx.current == bsp::DiveBombState::kTurnDown) {
                            run_dive_bomb_turndown_tick_009c44f0();
                        }
                        if (ctx.current == bsp::DiveBombState::kAimDive) {
                            run_dive_bomb_aimdive_tick_009c58d0();
                        }
                        if (ctx.current == bsp::DiveBombState::kFlyAbove) {
                            if (before != bsp::DiveBombState::kFlyAbove) {
                                // 009C6270, the fly-over's enter, edited under
                                // the integrator's hunk arbitration of
                                // 2026-09-19. It clears +18h/+19h/+1Bh/+1Ch
                                // (009C6295/009C6292/009C628F/009C629E) and
                                // sets +1Ah from `approach+D1h == 0`. Only the
                                // +1Ch clear needs state here: the other three
                                // are recomputed every tick by the state feed.
                                unit_.db_flyabove_bank_latch_1c = false;
                                unit_.db_flyabove_latch_tick = -1;
                            }
                            run_dive_bomb_flyabove_tick_009c62b0();
                        }
                        if (ctx.current == bsp::DiveBombState::kAimGlide) {
                            run_dive_bomb_aimglide_tick_009c5180();
                        }
                        if (ctx.current == bsp::DiveBombState::kGoAway) {
                            // 009C4950, the goaway vtable 00D20CA0 slot +4h.
                            // Packet cc9_goaway_turn.
                            if (before != bsp::DiveBombState::kGoAway) {
                                run_dive_bomb_goaway_enter_009c4950();
                            }
                            run_dive_bomb_goaway_tick_009c4a40(dt);
                        }
                        // 009C7240 / 009C7270. kDone and kPrepare share one
                        // vtable, 00D20D28, installed twice in 009C73A0, so they
                        // share the enter and the tick. Dispatched here, beside
                        // the other states, because bsp::dive_bomb_task_arm's
                        // `tick_state` hook is an empty override in this host.
                        if (ctx.current == bsp::DiveBombState::kDone ||
                            ctx.current == bsp::DiveBombState::kPrepare) {
                            if (before != ctx.current) {
                                run_dive_bomb_done_prepare_enter_009c7240();
                            }
                            run_dive_bomb_done_prepare_tick_009c7270();
                        }
                        // Packet cc8_dive_geometry: sample every turndown and
                        // aimdive tick, after the state tick has written its
                        // commands, until the buffer fills. Read-only.
                        if ((ctx.current == bsp::DiveBombState::kTurnDown ||
                             ctx.current == bsp::DiveBombState::kAimDive) &&
                            unit_.db_geo_samples < GameUnitSlot::kDbGeoSamples) {
                            GameUnitSlot::DbGeoSample& s =
                                unit_.db_geo[unit_.db_geo_samples++];
                            s.tick = unit_.dive_bomb_arm_ticks;
                            s.state = dive_bomb_state_bucket(ctx.current);
                            s.pitch_c64 = unit_.plane_pitch_angle_c64;
                            s.bank_c68 = unit_.plane_bank_angle_c68;
                            s.heading_c6c = unit_.plane_heading_c6c;
                            s.altitude = unit_.motion.position[1];
                            s.range = unit_.db_planar_bc;
                            s.bearing_err =
                                bsp::wrapped_angle_subtract_00438b10(
                                    unit_.db_bearing_c0,
                                    unit_.plane_heading_c6c);
                            {
                                bsp::DiveBombAimHeadingInputs hin;
                                hin.pitch_c64 = unit_.plane_pitch_angle_c64;
                                hin.bank_c68 = unit_.plane_bank_angle_c68;
                                hin.heading_c6c = unit_.plane_heading_c6c;
                                hin.body_up_x = unit_.motion.pose_row1[0];
                                hin.body_up_z = unit_.motion.pose_row1[2];
                                s.aim_heading =
                                    bsp::dive_bomb_aim_heading_009c4f80(hin);
                                s.roll_input =
                                    bsp::wrapped_angle_subtract_00438b10(
                                        s.aim_heading, unit_.db_bearing_c0);
                            }
                            s.roll_cmd =
                                unit_.plan_slots[bsp::kPilotSlotRoll].desired;
                            s.pitch_cmd =
                                unit_.plan_slots[bsp::kPilotSlotPitch].desired;
                            s.pitch_mode_2d0 = unit_.plan_state.pitch_mode_2d0;
                            s.heading_mode_2cc = unit_.plan_heading_mode_2cc;
                        }
                        if (ctx.current == bsp::DiveBombState::kAimGlide &&
                            before != bsp::DiveBombState::kAimGlide) {
                            // 009C4F40-009C4F71, the aimglide enter's seed of
                            // state+20h: `FLD [ESP+4]`, compare against the 5.0
                            // at 00D7A370, and 009C4F4E `76` JBE takes the 5.0
                            // at 00CE3850 when the argument is the smaller - so
                            // the travel accumulator starts at max(arg, 5.0) and
                            // is never zero.
                            //
                            // CORRECTION, packet cc8_dive_glide. `the argument's
                            // own producer is 009C4F00's caller, unread` is
                            // withdrawn: 009C4F00 is __thiscall(state) with no
                            // stack argument at all - it ends `ADD ESP,8` /
                            // `RET` with no immediate, balancing its own 009C4F00
                            // `SUB ESP,8` - and it builds the quantity itself:
                            //   009C4F15 FLD [EAX+0A4h]     ; 0.95 * MaxSpd
                            //   009C4F22 CALL 007C1DB0      ; rounds remaining
                            //   009C4F27 SUB EAX,1
                            //   009C4F2E FILD / 009C4F32 FMUL qword 00CED0D8 (0.07)
                            //   009C4F38 FMUL [ESP+8]
                            // so the seed is max((rounds - 1) * 0.07 *
                            // approach+A4h, 5.0). For the two-round USN04 Vals
                            // that is 1 * 0.07 * 0.95 * 69.44 = 4.62, under the
                            // floor - which is why the flat 5.0 happened to be
                            // right here. It is a coincidence, not the rule.
                            //
                            // This matters because the lead gates at
                            // 009C5743/009C5751 are satisfiable only while the
                            // accumulator is positive; the 0.0 that stood here
                            // closed the glide release by itself.
                            {
                                // 009C4F0C/009C4F10, the two stores before the
                                // seed: the aimglide enter clears state+18h and
                                // state+1Ch of its OWN state object, exactly as
                                // the aimdive enter does at 009C5876. This host
                                // carries one db_aim_rearm_1c for both states,
                                // so without this clear an aimdive release's
                                // 0.2-0.5 s draw would still be running when the
                                // glide's 009C5689 gate first asks.
                                unit_.db_aim_rearm_1c = 0.0f;
                                unit_.db_aim_pull_out_18 = false;
                                // Packet cc8_dive_flyover. db_aim_pull_out_18
                                // above is the AIMDIVE's +18h, whole-object
                                // +74Ch; the aimglide's own +18h is +76Ch and
                                // the two are different objects, so the glide
                                // latch gets its own field. 009C4F0C clears it.
                                unit_.db_aimglide_pull_out_76c = false;
                                const int rounds =
                                    owner_.dive_bomb_rounds_remaining(unit_);
                                const float seed =
                                    static_cast<float>(rounds - 1) *
                                    static_cast<float>(
                                        bsp::dive_bomb_constant::kGlideSeedScale) *
                                    (static_cast<float>(
                                         bsp::dive_bomb_constant::
                                             kApproachDriftScaleA4) *
                                     unit_.plane_max_spd);
                                unit_.db_glide_travel_20 =
                                    seed > bsp::dive_bomb_constant::kGlideTravelSeed
                                        ? seed
                                        : bsp::dive_bomb_constant::kGlideTravelSeed;
                            }
                        }
                        // The run-in census: one range sample per second of
                        // mission time, and the tick the latch closes.
                        if (unit_.db_in_range_d0 && unit_.db_latch_closed_tick < 0) {
                            unit_.db_latch_closed_tick = unit_.dive_bomb_arm_ticks;
                        }
                        if (unit_.db_range_samples < 16 &&
                            unit_.dive_bomb_arm_ticks >=
                                (unit_.db_range_samples + 1) * 10) {
                            unit_.db_range_at_second[unit_.db_range_samples] =
                                unit_.db_planar_bc;
                            ++unit_.db_range_samples;
                        }
                    }

                    // 009C18C0, the move-to tick, vtable 00D20AEC slot +Ch.
                    // PROVED for this task: 009C7436 constructs task+4F0h with
                    // 009C2AC0, which writes [state] = 0xD20AEC at 009C2B32, and
                    // 00D20AF8 holds 009C18C0. Nothing later overwrites it - the
                    // task ctor 009C7710 rewrites only [task], [task+3F8h] and
                    // [task+4DCh]. The same ctor call passes the TASK'S TARGET as
                    // the state's +2Ch (009C8C70 -> 009C7710 arg2 -> 009C73A0
                    // arg2 -> EBP at 009C7434), so step 1's separation is the
                    // aircraft-to-target planar range this host already keeps as
                    // db_planar_bc.
                    //
                    // The state's three floats come from 009C8825's
                    // `009BDE80(task+4A4h - 100.0, task+4A4h, task+4ACh)` - the
                    // arm refreshes them every tick, so the values are the LIVE
                    // approach+ACh (BeginAltRange/1) and approach+B4h.
                    // docs/DIVE_BOMB_APPROACH.md.
                    void run_dive_bomb_move_to_tick_009c18c0() {
                        ++unit_.db_moveto_tick_ticks;
                        // 009C198A-009C1999: the +1Ch speed slot is called with
                        // the separation UNCONDITIONALLY, before either early
                        // return. 00D20AEC+1Ch is 009C1850, which writes
                        // cmd+2B4h from 009BECD0(ctl+3A0h, 007C47F0(), speed),
                        // clears cmd+2B0h and raises cmd+2D8h.
                        //
                        // SUBSTITUTION, labelled and inherited from
                        // docs/TORPEDO_MOVETO_TICK.md: 009BECD0 shapes that
                        // product against the distance and is still unread, so
                        // what stands in is the shaping, not the speed. The
                        // speed itself is 007C47F0's LevelFlight * StallSpd.
                        unit_.plane_desired_speed_2b4 =
                            owner_.bot_desired_speed_007c47f0(unit_);
                        unit_.plane_air_brake_mode_2d8 = 1;
                        ++unit_.plane_speed_commands;
                        owner_.record("BotStateMoveTo::set_desired_speed", 0x009c1850u);

                        // 009C19A1: `CMP byte [unit+0C25h],0` returns early with
                        // four command writes. A contract: this host has no
                        // +0C25h and its bot aircraft are never under it.
                        // 009C19E7: with no target the tick takes the 009C1B69
                        // arm instead. Both are the image's own guards.
                        if (unit_.command_target_plus_one == 0) return;
                        const std::size_t ti = unit_.command_target_plus_one - 1;
                        if (ti >= owner_.slots.size()) return;
                        const float* const tp = owner_.slots[ti]->motion.position;

                        bsp::MoveToGlideInputs gin;
                        // 009C8814's `FSUB double [00D7A220]`, 100.0.
                        gin.near_range_30 = unit_.db_begin_alt_ac -
                            bsp::move_to_glide_constant::kNearRangeDrop;
                        gin.far_range_34 = unit_.db_begin_alt_ac;   // task+4A4h
                        gin.speed_range_38 = unit_.db_attack_dist_b4;  // task+4ACh
                        gin.target_world_y = tp[1];
                        gin.unit_world_y = unit_.motion.position[1];
                        // 009C1950's own sqrt; db_planar_bc is the same
                        // quantity, built by 009C7B4F from the same two poses
                        // with the same 00CE3820 epsilon.
                        gin.planar_distance = unit_.db_planar_bc;
                        const bsp::MoveToGlideCommand g =
                            bsp::move_to_glide_009c18c0(gin);

                        bsp::PlaneCruiseAltitudeInputs cin;
                        cin.base_altitude = g.base;
                        cin.range_low = g.range_low;
                        cin.range_high = g.range_high;
                        cin.scale = g.scale;
                        // class+518h, derived at 007C4A44 as tan(DropAngle).
                        cin.class_gain = static_cast<float>(
                            std::tan(static_cast<double>(unit_.plane_drop_angle)));
                        cin.has_squadron = false;
                        if (owner_.lua.plane_globals_loaded()) {
                            cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                        }
                        const bsp::PlaneCruiseAltitudeResult c =
                            bsp::cruise_altitude_command_009fba50(cin);
                        bsp::PlanePitchCommandInputs pin;
                        pin.desired_altitude = c.clamped_altitude;
                        pin.reference = c.pitch_reference;  // 009FBB06, not the altitude
                        pin.unit_world_y = unit_.motion.position[1];
                        pin.ceiling = cin.ceiling;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& gt = owner_.lua.plane_globals();
                            pin.climb_dist = gt.pilot_general_climb_dist;
                            pin.drop_dist = gt.pilot_general_drop_dist;
                        }
                        pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                        pin.class_drop_angle = unit_.plane_drop_angle;
                        unit_.plane_commanded_altitude = c.clamped_altitude;
                        unit_.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
                        unit_.plan_state.pitch_target_2bc = unit_.plane_commanded_pitch;
                        // 009C1B17 CALL 009FBA50 -> 009FB800, which writes
                        // cmd+2D0h = 2. docs/PITCH_COMMAND_CALLERS.md.
                        if constexpr (GameUnitsHost::Impl::kPitchCommandCallersBound) {
                            unit_.plan_state.pitch_mode_2d0 = 2;
                        }
                        owner_.record("BotStateMoveTo::glide_slope", 0x009c18c0u);

                        // 009C1B1C-009C1B23: `LEA ECX,[ESP+1Ch]` is the target
                        // world position step 1 parked there, so 009F9E40
                        // BSP_PilotBot_CommandHeadingToPoint steers at the
                        // target. SUBSTITUTION, labelled: 009F9E40's body is
                        // unread, so the bearing this host writes is its own
                        // db_bearing_c0, the pi/2 - atan2 convention 009C7B8A
                        // was read for, through the same mode-2 pair the
                        // attackrun and flyabove ticks use.
                        unit_.plan_heading_2c0 = unit_.db_bearing_c0;
                        unit_.plan_heading_2c0_written = true;
                        unit_.plan_heading_mode_2cc = 2;
                        owner_.record("BotStateMoveTo::steer_to_point", 0x009f9e40u);
                        if ((unit_.db_moveto_tick_ticks % 200) == 1) {
                            owner_.log.notef("  db moveto %-12s n=%d range=%.1f "
                                "base=%.1f low=%.1f scale=%.3f gain=%.3f "
                                "commanded=%.1f live_alt=%.1f hdg=%.3f spd=%.2f",
                                unit_.row.name.c_str(), unit_.db_moveto_tick_ticks,
                                static_cast<double>(gin.planar_distance),
                                static_cast<double>(g.base),
                                static_cast<double>(g.range_low),
                                static_cast<double>(g.scale),
                                static_cast<double>(cin.class_gain),
                                static_cast<double>(c.clamped_altitude),
                                static_cast<double>(unit_.motion.position[1]),
                                static_cast<double>(unit_.db_bearing_c0),
                                static_cast<double>(unit_.plane_desired_speed_2b4));
                        }
                    }

                    // 009C1FD0, the follow tick, vtable 00D20AB8 slot +Ch.
                    // PROVED that it is NOT 009C18C0: 009C7451 constructs
                    // task+52Ch with 009C2980, which writes [state] = 0xD20AB8
                    // at 009C29B5, and 00D20AC4 holds 009C1FD0. The ledger note
                    // on 009C18C0 calling 00D20AEC+0Ch "shared by moveto and
                    // follow" is wrong, and so is the last paragraph of section
                    // 2 of docs/TORPEDO_MOVETO_TICK.md.
                    //
                    // WHAT IS MISSING: 009C1FEA CALL 009BFD70 (the station
                    // point, bound) then 009BFEE0 and 009BEE30, about 2900
                    // instructions of station-keeping law that are not read
                    // (docs/PLANE_FORMATION.md section 6). This host places the
                    // member on its station instead - the geometry is the
                    // image's, the path to it is not - exactly as the done and
                    // prepare states do, which is faithful in that they reach
                    // this same body through 009C7278.
                    //
                    // REACHABLE since packet cc8_follow_enter. 009C777E and
                    // 009C8419 choose moveto when 007B8AD0 is true, and that
                    // predicate is `unit+9D8h == 0`, the squadron array slot -
                    // the flight-leader test, not a follow target. It is now fed
                    // from the squadron registry, so every wing member of a
                    // multi-plane squadron constructs into follow and stays
                    // there until its squadron's attack mode reaches 2.
                    void run_dive_bomb_follow_tick_009c1fd0() {
                        ++unit_.db_follow_tick_ticks;
                        unit_.plan_mode_26c = 2;   // 009C1FE2
                        bsp::PlaneFormationStation station;
                        const GameUnitSlot* leader = nullptr;
                        // Packet cc8_follow_attack measured run E2 with
                        // `/*apply_position=*/false` here - the dive-bomb follow
                        // tick alone stops teleporting, so the follow LAW flies
                        // the member and its velocity stays consistent with its
                        // motion, while the torpedo seam keeps placement because
                        // the law is not wired into it yet. That is the ONE
                        // argument to change to re-take E2; nothing else moves.
                        // Left at `true` here because E2 had not been read when
                        // this packet closed. docs/FOLLOWER_ATTACK_HANDOVER.md
                        // sections 7 and 9.
                        if (owner_.place_wing_member_on_station_007f23a0(
                                unit_, false, &station, &leader)) {
                            owner_.log.implemented("BotStateFollow::station_point",
                                                   "007f23a0");
                        }
                        if (owner_.kPlaneFollowLawEnabled && station.produced
                            && leader != nullptr) {
                            owner_.run_follow_law_009bfee0_009bee30(unit_, station, *leader);
                        }
                        owner_.record("BotStateFollow::station_keeping", 0x009bfee0u);
                    }

                    // 009C4220, the attackrun tick, vtable 00D20C68 slot +Ch.
                    // The run-in. It commands a heading at the target with mode
                    // 2, the pair 0099D300's yaw arm reads instead of the raw
                    // bearing, so binding it is what lets the aircraft close its
                    // attack range.
                    void run_dive_bomb_attackrun_tick_009c4220(float dt) {
                        bsp::DiveBombAttackRunInputs in;
                        in.dt = dt;
                        in.reroll_timer_1c = unit_.db_attackrun_timer_1c;
                        in.reroll_period_18 = unit_.db_attackrun_period_18;
                        in.lateral_offset_20 = unit_.db_attackrun_offset_20;
                        in.target_bearing_c0 = unit_.db_bearing_c0;
                        in.planar_distance_bc = unit_.db_planar_bc;
                        in.altitude = unit_.motion.position[1];
                        in.begin_altitude_ac = unit_.db_begin_alt_ac;
                        in.aim_point_height_50 = unit_.db_aim_point_height_50;
                        in.attack_distance_b4 = unit_.db_attack_dist_b4;
                        // SUBSTITUTION, labelled: 007F0280 at 009C42B8 is a
                        // contract, so the run-in flies straight at the target
                        // rather than weaving. Its three float arguments are the
                        // 80, 60 and 120 the listing pushes at 009C4258,
                        // 009C4268 and 009C4287.
                        in.sampler_result = 0.0f;
                        in.sampler_ran = false;
                        if constexpr (GameUnitsHost::Impl::kNearFieldProbeBound) {
                            // 009C42B8 runs only on the re-roll arm (dt >= +1Ch).
                            if (!(in.dt < in.reroll_timer_1c)) {
                                const float ext[3] = {80.0f, 60.0f, 120.0f};
                                const float w[3] = {0.0f, 0.0f, 0.0f};
                                const bsp::NearFieldProbeResult pr = nf_probe_007f0280(ext, w);
                                in.sampler_result = bsp::near_field_attackrun_sampler_009c42bd(pr);
                                in.sampler_ran = true;
                                if (in.sampler_result != 0.0f) {
                                    ++unit_.nf_attackrun_weaves;
                                    const float off = in.sampler_result < 0.0f
                                        ? -in.sampler_result : in.sampler_result;
                                    if (off > unit_.nf_attackrun_max_offset) {
                                        unit_.nf_attackrun_max_offset = off;
                                    }
                                }
                            }
                        }
                        const bsp::DiveBombAttackRunResult r =
                            bsp::dive_bomb_attackrun_tick_009c4220(in);
                        ++unit_.db_attackrun_ticks;
                        if (r.rerolled) ++unit_.db_attackrun_rerolls;
                        unit_.db_attackrun_timer_1c = r.reroll_timer_1c;
                        unit_.db_attackrun_offset_20 = r.lateral_offset_20;
                        unit_.db_attackrun_heading_last = r.commanded_heading_2c0;
                        unit_.db_attackrun_throttle_last = r.descent_scale;
                        unit_.db_attackrun_alt_last = r.commanded_altitude_base;
                        // 009C42FF and 009C4305: cmd+2C0h with cmd+2CCh = 2.
                        unit_.plan_heading_2c0 = r.commanded_heading_2c0;
                        unit_.plan_heading_2c0_written = true;
                        unit_.plan_heading_mode_2cc = 2;
                        // 009C4413-009C4434: full throttle, no air brake.
                        unit_.plan_slots[bsp::kPilotSlotThrottle].desired = 1.0f;
                        unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                        unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = 0.0f;
                        unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                        // 009C4401's four arguments, all now read from the image
                        // rather than inferred. 009C43D2 SUB ESP,0x10 opens the
                        // window and they go in at:
                        //   [ESP]    009C43F3  approach+ACh + approach+50h
                        //   [ESP+4]  009C43E3  approach+B4h, the attack distance
                        //   [ESP+8]  009C43DB  approach+BCh, the LIVE planar range
                        //   [ESP+Ch] 009C43CD  the InterpolateClamped result
                        //
                        // CORRECTED. Both ranges used to be attack_distance_b4,
                        // so `span = max(high - low, 0)` was identically zero,
                        // the bias `span * scale * class+518h` vanished and the
                        // aircraft was commanded to the bare base from 11 km
                        // out - it descended at once instead of gliding down as
                        // it closed. The second range is 009C4311's read of
                        // approach+BCh, parked at 009C4317 in the tick's own dt
                        // slot ([ESP+44h], reused as scratch once dt is spent).
                        // So span is the distance STILL TO CLOSE: about 9900 m
                        // at 11 km, zero at the attack distance. That is the
                        // glide slope. The base is composed too, and it is the
                        // same +ACh + +50h sum 009C7F00's ceiling uses.
                        // Found alongside cc8-plane-squadron's zero-range-pair
                        // diagnosis on the torpedo side. docs/DIVE_BOMB_TASK.md.
                        bsp::PlaneCruiseAltitudeInputs cin;
                        cin.base_altitude =
                            unit_.db_begin_alt_ac + unit_.db_aim_point_height_50;
                        cin.range_low = in.attack_distance_b4;
                        cin.range_high = in.planar_distance_bc;
                        // CORRECTION, packet cc8_torpedo_descent_law, for the
                        // owner of src/dive_bomb_task.cpp: 009C43CD's
                        // InterpolateClamped result is NOT a throttle. 009C43D5
                        // FSTP [ESP+0xc] makes it 009FBA50's arg3, which
                        // 009FBB06 hands to 009FB800 as the reference - the same
                        // slot and the same constants (00D7A2F0 = 0.1,
                        // 00CE7804 = 0.4) as the torpedo attackrun's 009D0A63.
                        // The full throttle this state commands is the literal
                        // 1.0 at 009C4413 above. The field keeps its name here
                        // because renaming it reaches into another lease.
                        cin.scale = r.descent_scale;
                        cin.class_gain = static_cast<float>(
                            std::tan(static_cast<double>(unit_.plane_drop_angle)));
                        cin.has_squadron = false;
                        if (owner_.lua.plane_globals_loaded()) {
                            cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                        }
                        const bsp::PlaneCruiseAltitudeResult c =
                            bsp::cruise_altitude_command_009fba50(cin);
                        // Instrumentation, not a rule. The 009FBA50 fix came back
                        // byte-identical and two inferences about why have already
                        // been wrong - class_gain being zero, and a -List search
                        // that claimed no aircraft carries DropAngle - so these
                        // terms get measured rather than guessed at again.
                        if (unit_.db_cruise_samples < 8 &&
                            unit_.db_attackrun_ticks >=
                                (unit_.db_cruise_samples + 1) * 120) {
                            const int i = unit_.db_cruise_samples;
                            unit_.db_cruise_span[i] =
                                (cin.range_high > cin.range_low)
                                    ? cin.range_high - cin.range_low : 0.0f;
                            unit_.db_cruise_gain[i] = cin.class_gain;
                            unit_.db_cruise_scale[i] = cin.scale;
                            unit_.db_cruise_base[i] = cin.base_altitude;
                            unit_.db_cruise_clamped[i] = c.clamped_altitude;
                            ++unit_.db_cruise_samples;
                        }
                        bsp::PlanePitchCommandInputs pin;
                        pin.desired_altitude = c.clamped_altitude;
                        pin.reference = c.pitch_reference;  // 009FBB06, not the altitude
                        pin.unit_world_y = unit_.motion.position[1];
                        pin.ceiling = cin.ceiling;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            pin.climb_dist = g.pilot_general_climb_dist;
                            pin.drop_dist = g.pilot_general_drop_dist;
                        }
                        // Packet cc8_dive_race. Was a literal 0.0f, which is a
                        // host defect and cannot be a transcription: 009FB800
                        // takes only TWO stack arguments (009FB96B `RET 0x8`,
                        // the clamped altitude at [ESP+0Ch] and the reference at
                        // [ESP+10h]) and fetches both angles itself off the same
                        // pointer chain, `MOV ECX,[ESI]` / `MOV EDX,[ECX+4]` /
                        // `MOV EAX,[EDX+538h]`:
                        //   009FB88D  FLD float ptr [EAX + 0x1ec]   ; climb gain
                        //   009FB979  FLD float ptr [EDX + 0x1f0]   ; DropAngle
                        // No caller passes either, so the dive-bomb run-in reads
                        // exactly the desc+1ECh every other path reads. With the
                        // gain at zero 009FB918's `min(gain * t, limit)` is zero
                        // and the run-in could never climb to its command.
                        pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                        pin.class_drop_angle = unit_.plane_drop_angle;
                        unit_.plane_commanded_altitude = c.clamped_altitude;
                        unit_.plane_commanded_pitch = bsp::pitch_command_009fb800(pin);
                        unit_.plan_state.pitch_target_2bc = unit_.plane_commanded_pitch;
                        // 009FB800's chain writes cmd+2D0h = 2 beside +2BCh, so
                        // the run-in keeps the planner's pitch arm running. The
                        // reset at 0099B54E already leaves 2 there; this is the
                        // write made explicit next to the gate at 0099E3BF.
                        unit_.plan_state.pitch_mode_2d0 = 2;
                        // REMOVED: `plane_desired_speed_2b4 = descent_scale`.
                        // A command census over 009C4220-009C447D finds no write
                        // to cmd+2B4h at all, so the run-in issues no desired
                        // speed; the host was inventing one out of 009C43CD's
                        // result, which cc8_torpedo_descent_law has just shown is
                        // the descent scale and never was a throttle. The only
                        // throttle this state commands is the literal 1.0 at
                        // 009C4413, already modelled above.
                        unit_.plane_air_brake_mode_2d8 = 0;   // 009C4434
                    }

                    // 009C44F0, the turndown tick, vtable 00D20C84 slot +Ch.
                    // The arm reaches it through state->vtable[+Ch] at
                    // 009C884C. docs/DIVE_BOMB_TASK.md carries the body.
                    // 009C4A40's climb-out, the state that gets the aircraft out
                    // of its dive. Until this was dispatched the walk ended
                    // `aimdive` -> `goaway` one tick -> water contact at
                    // -1.12 m: the pull-out edge fired, the state changed and
                    // nothing happened. docs/DIVE_BOMB_TASK.md.
                    void run_dive_bomb_goaway_tick_009c4a40(float dt) {
                        bsp::DiveBombGoAwayInputs in;
                        in.altitude = unit_.motion.position[1];   // [EDI+100h]
                        // (approach+8h)->+1ECh, which this host already carries
                        // from the Lua row as 0.6 of the sustainable climb
                        // angle 007D98F0 returns - the real field, not a stand-in.
                        in.climb_angle_1ec = unit_.plane_climb_angle_1ec;
                        // Packet cc8_dive_goaway: curve A, 009C4B32-009C4B61,
                        // whose interpolant is the altitude DEFICIT against the
                        // ceiling min(ctl+398h, approach+ACh + approach+50h).
                        // Without it the tick modelled only the ground-avoidance
                        // curve, which is zero at and above 300 m, so an aircraft
                        // that left its dive at 82-174 m climbed to 300 m and
                        // then held level for the rest of the mission while
                        // 009C7F00 waited for 900 m. ctl+398h is what
                        // approach+ACh is refreshed from every tick here, so this
                        // host has one value for both, as 009C7F00's feed does.
                        in.cruise_altitude_398 = unit_.db_begin_alt_ac;
                        in.begin_altitude_ac = unit_.db_begin_alt_ac;
                        in.aim_point_height_50 = unit_.db_aim_point_height_50;
                        in.unit_pitch_c64 = unit_.plane_pitch_angle_c64;
                        const bsp::DiveBombGoAwayCommand r =
                            bsp::dive_bomb_goaway_climb_009c4b44(in);
                        ++unit_.db_goaway_tick_ticks;
                        unit_.db_goaway_pitch_last = r.pitch_target_2bc;
                        // Packet cc8_dive_goaway census. `pitch_max` is a
                        // running maximum over the whole state, `pitch_last` the
                        // final sample; quoting either one alone has misled this
                        // area before, so both are printed.
                        if (r.pitch_target_2bc > unit_.db_goaway_pitch_max) {
                            unit_.db_goaway_pitch_max = r.pitch_target_2bc;
                        }
                        const float goaway_y = unit_.motion.position[1];
                        if (unit_.db_goaway_alt_first < 0.0f) {
                            unit_.db_goaway_alt_first = goaway_y;
                        }
                        unit_.db_goaway_alt_last = goaway_y;
                        if (goaway_y > unit_.db_goaway_alt_max) {
                            unit_.db_goaway_alt_max = goaway_y;
                        }
                        // 009C4ACF-009C4B05, the ceiling, and 009C4B26's FSUB.
                        const float goaway_sum = unit_.db_begin_alt_ac +
                                                 unit_.db_aim_point_height_50;
                        const float goaway_ceiling =
                            (unit_.db_begin_alt_ac <= goaway_sum)
                                ? unit_.db_begin_alt_ac : goaway_sum;
                        unit_.db_goaway_ceiling_last = goaway_ceiling;
                        unit_.db_goaway_deficit_last = goaway_ceiling - goaway_y;
                        // 009C4BE0/009C4BE8: the planner's own pitch arm flies
                        // this, because mode 1 passes the gate at 0099E3BF.
                        unit_.plan_state.pitch_target_2bc = r.pitch_target_2bc;
                        unit_.plan_state.pitch_mode_2d0 = r.pitch_mode_2d0;
                        // Packet cc9_goaway_turn. 009C4A6D-009C4ACD, the timers.
                        // They run on every tick, on both sides of the nose-down
                        // split, and ahead of the climb in the image; the climb
                        // reads none of their fields, so running them here
                        // changes nothing about it.
                        bsp::DiveBombGoAwayTurnState& gt = unit_.db_goaway_turn;
                        {
                            bsp::DiveBombGoAwayTimerInputs tin;
                            tin.dt = dt;                                    // [ESP+18h]
                            tin.planar_distance_bc = unit_.db_planar_bc;    // 009C4A6D
                            // approach+C4h is seeded 3600.0 (00CFDEB0) at 009C3FD2
                            // and 009C8A74 and only ever advanced `+= dt` at
                            // 009C7A8C. This host does not carry the clock; its
                            // floor already answers the one test that reads it
                            // (009C4AA4, 1.0 > C4h), which is false.
                            tin.approach_clock_c4 = 3600.0f;
                            const bsp::DiveBombGoAwayTimerReport tr =
                                bsp::dive_bomb_goaway_timers_009c4a6d(tin, gt);
                            if (tr.counted_down) ++unit_.db_goaway_countdown_ticks;
                        }
                        // 009C4BD8/009C4BEF `JZ 009C4CBA`: the nose-down byte picks
                        // the side. `wrote_bank_heading` is now consumed, because
                        // the flag-0 side below writes its own lateral command.
                        if (r.wrote_bank_heading) {
                            // 009C4BFE/009C4C06: wings level through the servo, the
                            // mode-1 arm at 0099E26E, which rolls the aircraft
                            // upright out of the inverted dive. Then the throttle and
                            // air-brake shaping 009C4C0C-009C4CA7 (cc9_dive_throttle).
                            if (kGoawayThrottleBound) {
                                const bsp::DiveBombGoAwayThrottle gth =
                                    bsp::dive_bomb_goaway_throttle_009c4c0c(
                                        true, unit_.plane_pitch_angle_c64);
                                unit_.plan_slots[bsp::kPilotSlotThrottle].desired = gth.throttle_278;
                                unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = gth.air_brake_2a8;
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                            }
                            unit_.plan_state.bank_target_2c4 = r.bank_target_2c4;
                            unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                            unit_.plan_heading_2c0_written = false;
                            unit_.plane_air_brake_mode_2d8 = r.air_brake_mode_2d8;
                            return;
                        }
                        // 009C4CBA-009C4CE7: full throttle (+278h = 1.0, +27Ch = 1),
                        // no air brake (+2A8h = 0, +2ACh = 1), +2D8h = 0.
                        ++unit_.db_goaway_flag0_ticks;
                        if (kGoawayThrottleBound) {
                            const bsp::DiveBombGoAwayThrottle gth =
                                bsp::dive_bomb_goaway_throttle_009c4c0c(
                                    false, unit_.plane_pitch_angle_c64);
                            unit_.plan_slots[bsp::kPilotSlotThrottle].desired = gth.throttle_278;
                            unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = gth.air_brake_2a8;
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                        }
                        unit_.plane_air_brake_mode_2d8 = 0;  // 009C4CE7
                        {
                            // 009C4CF1-009C4D9A, the re-roll.
                            bsp::DiveBombGoAwayRerollInputs rin;
                            rin.altitude = unit_.motion.position[1];  // 009C4D54
                            // 00BD2F10 UniformFloatRange(3, 6) at 009C4D13 and
                            // (12, 15) at 009C4D33: the low end, as this host pins
                            // every draw (random_between).
                            rin.window_draw = bsp::dive_bomb_goaway_turn::kBankWindowLo;
                            rin.countdown_draw = bsp::dive_bomb_goaway_turn::kCountdownLo;
                            if (bsp::dive_bomb_goaway_reroll_009c4cf1(rin, gt)) {
                                ++unit_.db_goaway_rerolls;
                                if (unit_.db_goaway_first_reroll_tick < 0) {
                                    unit_.db_goaway_first_reroll_tick =
                                        unit_.dive_bomb_arm_ticks;
                                }
                            }
                        }
                        // 009C4DAD, the split. 009C47D0 is called only on the
                        // heading arm (009C4E09), so +1Ch and 009FD570's side
                        // write-back move only there.
                        bsp::DiveBombGoAwayTurnCommand tc =
                            bsp::dive_bomb_goaway_turn_split_009c4dad(gt, gt.heading_1c);
                        if (tc.bank_arm) {
                            // 009C4DF0/009C4DF6: the planner's mode-1 arm servos to
                            // the task's bank target.
                            unit_.plan_state.bank_target_2c4 = tc.bank_target_2c4;
                            unit_.plan_heading_mode_2cc = tc.heading_mode_2cc;
                            unit_.plan_heading_2c0_written = false;
                            if (unit_.db_goaway_bank_ticks == 0 ||
                                tc.bank_target_2c4 < unit_.db_goaway_bank_min) {
                                unit_.db_goaway_bank_min = tc.bank_target_2c4;
                            }
                            if (unit_.db_goaway_bank_ticks == 0 ||
                                tc.bank_target_2c4 > unit_.db_goaway_bank_max) {
                                unit_.db_goaway_bank_max = tc.bank_target_2c4;
                            }
                            ++unit_.db_goaway_bank_ticks;
                            return;
                        }
                        gt.heading_1c = goaway_turn_heading_009c47d0(gt);  // 009C493C
                        tc.heading_2c0 = gt.heading_1c;
                        // 009C4E17/009C4E1D: the heading and mode 2, which the
                        // planner's heading term reads at 0099DEB8.
                        unit_.plan_heading_2c0 = tc.heading_2c0;
                        unit_.plan_heading_2c0_written = true;
                        unit_.plan_heading_mode_2cc = tc.heading_mode_2cc;
                        if (unit_.db_goaway_first_heading_tick < 0) {
                            unit_.db_goaway_first_heading_tick = unit_.dive_bomb_arm_ticks;
                        }
                        ++unit_.db_goaway_heading_ticks;
                        unit_.db_goaway_heading_last = tc.heading_2c0;
                        {
                            const float err = std::fabs(bsp::wrapped_angle_subtract_00438b10(
                                tc.heading_2c0, unit_.plane_heading_c6c));
                            if (err > unit_.db_goaway_heading_err_max) {
                                unit_.db_goaway_heading_err_max = err;
                            }
                        }
                        // 009C4E27-009C4E58, NOT MODELLED, and not a speed command:
                        // 0042E740()+674h (`Pilot/AutoStrafeAngle/Angle_GoAway`) is
                        // stored to (approach+1Ch)+40h, then
                        // 009FABE0(approach+1Ch; +1Ch, 0099B630(cmd)) writes the
                        // normalised (cos, tan(pitch), sin) of the compass heading
                        // to (approach+1Ch)+68h..70h. 0099B630 returns cmd+2BCh
                        // while cmd+2D0h is set (it is, 009C4BE8). This host models
                        // no approach+1Ch object.
                    }

                    // 009C47D0, `void __thiscall(goaway state)`, no stack argument,
                    // plain RET; its only caller is 009C4E09. It is 009D0C10, the
                    // torpedo break-off geometry, with the goaway's own fields and
                    // literals: standoff +20h (not +24h), side +18h (not +2Ch),
                    // range approach+BCh (not +90h), 009FD570's arg6 FLD1 (not the
                    // 0.8 at 00CE74F8), 007F0280 mode 1 (not 0) and half-extents
                    // 80/50/100, and the store to +1Ch (not +18h). Every
                    // instruction after the 009FD570 call is the same, so
                    // torpedo_goaway_heading_009d0c10 is reused as it stands.
                    // docs/DIVE_BOMB_GOAWAY_TURN.md section 1.
                    float goaway_turn_heading_009c47d0(bsp::DiveBombGoAwayTurnState& gt) {
                        if (unit_.command_target_plus_one == 0) return gt.heading_1c;
                        const std::size_t ti = unit_.command_target_plus_one - 1;
                        if (ti >= owner_.slots.size()) return gt.heading_1c;
                        const GameUnitSlot& tgt = *owner_.slots[ti];
                        bsp::FlyToSolverInputs fin;
                        // arg1, approach->vtable[0](): the fed aim point, the same
                        // feed 009C6342's fly-over uses (the target's origin while
                        // kHullAimOffsetEnabled is false).
                        float p[3] = {tgt.motion.position[0], tgt.motion.position[1],
                                      tgt.motion.position[2]};
                        hull_aim_world_point(unit_, tgt, ti + 1, p);
                        for (int i = 0; i < 3; ++i) {
                            fin.point[i] = p[i];
                            fin.unit_position[i] = unit_.motion.position[i];
                            // unit->vtable[34h]: the same velocity HYPOTHESIS the
                            // torpedo break-off feed states.
                            fin.unit_lead_vector[i] = unit_.plane_world_velocity[i];
                        }
                        fin.standoff = gt.standoff_20;        // arg3, [ESI+20h]
                        fin.range = unit_.db_planar_bc;       // arg5, approach+BCh
                        fin.offset_scale = bsp::dive_bomb_goaway_turn::kFlyToOffsetScale;
                        // HOLE, stated: the solver's own obstacle cache (ECX is the
                        // goaway state; +0Ch..+14h zeroed at 009C74D0-009C74E8) is
                        // rebuilt from GGame+19CCh's unit list, which this host
                        // does not expose. Unlike the torpedo's open-water break
                        // off, this aircraft starts next to its target, so the
                        // target itself may be an obstacle the image steers round.
                        fin.obstacles = nullptr;
                        fin.obstacle_count = 0;
                        fin.world_edge.near_edge = false;
                        const bsp::FlyToSolverResult fr =
                            bsp::fly_to_point_heading_009fd570(fin, gt.side_18);
                        if (fr.side != gt.side_18) ++unit_.db_goaway_side_writes;
                        gt.side_18 = fr.side;  // arg4 IN AND OUT, 009FDC48
                        bsp::TorpedoGoAwayGeometryInputs geo;
                        geo.unit_heading_c6c = unit_.plane_heading_c6c;  // vtable[50h]
                        geo.break_off_bearing = fr.heading;
                        // 007F0280 at 009C4878, mode 1, half-extents 80/50/100
                        // (009C4819/009C482C/009C483A), weights zero: the rule
                        // forms -probe[0] * probe[1] * probe[2], and 009C487D-
                        // 009C4892 take out_a.x, out_b.y, out_b.z.
                        if constexpr (GameUnitsHost::Impl::kNearFieldProbeBound) {
                            const float ext[3] = {bsp::dive_bomb_goaway_turn::kProbeExtentX,
                                                  bsp::dive_bomb_goaway_turn::kProbeExtentY,
                                                  bsp::dive_bomb_goaway_turn::kProbeExtentZ};
                            const float w[3] = {0.0f, 0.0f, 0.0f};
                            const bsp::NearFieldProbeResult pr = nf_probe_007f0280(ext, w);
                            geo.probe[0] = pr.out_a[0];
                            geo.probe[1] = pr.out_b[1];
                            geo.probe[2] = pr.out_b[2];
                            if (pr.hit) ++unit_.nf_goaway_hits;
                        }
                        return bsp::torpedo_goaway_heading_009d0c10(geo);
                    }

                    // 009C4950, the goaway enter (vtable 00D20CA0 slot +4h). Ghidra
                    // has no function here; body 009C4950-009C4A3C, plain RET.
                    void run_dive_bomb_goaway_enter_009c4950() {
                        bsp::DiveBombGoAwayEnterInputs in;
                        in.attack_distance_b4 = unit_.db_attack_dist_b4;  // 009C4990
                        // 007B5BE0's target extent needs the target's +444h/+448h,
                        // which this host does not model; the torpedo enter
                        // 009D0D90 reports the same hole.
                        in.has_extent_target = false;
                        // 00BD2F10 UniformFloatRange(1.0, 1.25): the low end.
                        in.standoff_jitter = bsp::dive_bomb_goaway_turn::kStandoffJitterLo;
                        // [00F876B0] is the mission step counter, +1 per step. This
                        // host has none; the aircraft's own arm tick count also
                        // advances once per step, so its parity is the step's
                        // parity up to one per-aircraft constant. STAND-IN.
                        in.step_odd = (unit_.dive_bomb_arm_ticks & 1) != 0;
                        // ctl+369h and [00E17BF2]: false, as 009C7F00's feed has them.
                        in.control_flag_369 = false;
                        in.global_e17bf2 = false;
                        bsp::dive_bomb_goaway_enter_009c4950(in, unit_.db_goaway_turn);
                        // 009C7F00 completes against this same +20h, which the host
                        // had left at zero since the field was split from the
                        // aimglide's.
                        unit_.db_goaway_travel_20 = unit_.db_goaway_turn.standoff_20;
                        ++unit_.db_goaway_enters;
                    }

                    // 009C7240 and 009C7270, the dive-bomb done/prepare state.
                    // 00D20D28 is installed TWICE in 009C73A0 (009C747E and
                    // 009C74B8), so "done" and "prepare" are ONE class with one
                    // enter, one exit and one tick - the mirror of the torpedo's
                    // 00D21320. docs/BOMBER_AFTER_TASK.md section 1.
                    //
                    // The enter, body 009C7240-009C725B, __thiscall(state):
                    //   009c7240  MOVSS XMM0,[00D7A260]     ; -1.0f
                    //   009c7248  MOV   byte ptr [ECX+9Ch],0
                    //   009c724f  MOVSS [ECX+98h],XMM0
                    //   009c7257  JMP   009BED80            ; the Follow base enter
                    // The dive bomber does NOT override the formation shape: the
                    // base enter leaves squadron+3E4h = 1 at 009BEDDA, the only
                    // shape src/plane_formation.cpp reconstructs. The torpedo's
                    // own enter overrides it to 2 at 009D254E and re-assigns,
                    // which is why the torpedo's done state cannot be bound this
                    // way until 007F25BD (shape 2) is read.
                    void run_dive_bomb_done_prepare_enter_009c7240() {
                        ++unit_.db_done_entries;
                        unit_.db_done_entry_alt = unit_.motion.position[1];
                        // 009BEDDA then 009BEDE4. The shape is squadron state
                        // here; 007ED260 itself runs once per squadron inside
                        // the placement seam, which is the scheduling difference
                        // docs/PLANE_FORMATION.md section 6 already states.
                        if (bsp::PlaneSquadronHostRecord* const squadron =
                                bsp::plane_squadron_registry().find_by_member_unit(
                                    unit_.process_index)) {
                            squadron->formation_shape_3e4 = 1;
                        }
                        // HOLE, stated. 009BED80 also caches the Pilot/Follow
                        // tuning block (tuning+380h) into state+6Ch, seeds
                        // state+88h from *(*(state+6Ch)+8), writes state+84h,
                        // +85h, +8Ch, +90h and +94h, and registers the leader
                        // observer (006952A0/00694A60) into state+2Ch. This host
                        // models no follow-state object, so none of that is
                        // carried. 009C7240's own state+98h and +9Ch are the drop
                        // countdown and its flag, which only the TORPEDO tick
                        // 009D2720 reads; 009C7270 never looks at them.
                        owner_.record("BotStateDiveBombDone::enter", 0x009c7240u);
                    }

                    // The tick, read whole:
                    //   009c7270  FLD   float ptr [ESP+4]     ; dt
                    //   009c7274  PUSH  ECX
                    //   009c7275  FSTP  float ptr [ESP]
                    //   009c7278  CALL  009C1FD0
                    //   009c727d  RET   4
                    // so it is __thiscall(state, float dt) and forwards dt, and
                    // its body is 009C7270-009C727F, SIXTEEN bytes - not the 13
                    // docs/BOMBER_AFTER_TASK.md section 4 records. `dt` is not a
                    // parameter here because the only part of 009C1FD0 this host
                    // binds, the station placement, does not consume it; the
                    // consumers of dt are 009BFEE0 and 009BEE30, both unread.
                    // 009C1FD0's head writes
                    // [[state+4]+18h]+26Ch = 2 BEFORE the 009BFD70 gate, so that
                    // store runs for every aircraft in the state, leader
                    // included; a flight LEADER then takes 009BFD70's false
                    // return and 009C1FF1 JZ 009C234E ends the tick at the
                    // epilogue (POP EBP / POP EBX / ADD ESP,68h / RET 4, read),
                    // having commanded that one byte and nothing else.
                    void run_dive_bomb_done_prepare_tick_009c7270() {
                        ++unit_.db_done_tick_ticks;
                        // 009C1FE2. Carried, not acted on - see the field's note.
                        unit_.plan_mode_26c = 2;
                        unit_.db_done_last_alt = unit_.motion.position[1];
                        unit_.db_done_last_hdg = unit_.plane_heading_c6c;
                        // 009C1FEA-009C2077 for a wing MEMBER: 009BFD70 (the
                        // station, bound), then 009BFEE0 and 009BEE30, ~2900
                        // instructions that are not reconstructed. This host
                        // PLACES the member on its station instead; the geometry
                        // is the image's, the path to it is not.
                        // docs/PLANE_FORMATION.md section 6.
                        if (owner_.place_wing_member_on_station_007f23a0(unit_, false)) {
                            ++unit_.db_done_placed_ticks;
                            owner_.log.implemented("BotStateDiveBombDone::station_point",
                                                   "007f23a0");
                        }
                        owner_.record("BotStateDiveBombDone::station_keeping",
                                      0x009bfee0u);
                    }

                    // 009C5180's heading arm, the glide's counterpart to the
                    // flyabove's. PARTIAL for the same reason; the bank, the
                    // altitude and the direct-yaw sibling arm are read in
                    // include/bsp/dive_bomb_task.hpp and left unbound.
                    void run_dive_bomb_aimglide_tick_009c5180() {
                        // 009C57C4-009C57FF, the tick's TAIL: the pull-out
                        // latch. Packet cc8_dive_flyover, arm A only; the
                        // header says why arm B is not modelled. Sampled from
                        // the same [ESP+18h] the release gate reads, which the
                        // approach feed has already written this frame.
                        {
                            const float e = unit_.db_glide_bearing_abs_18;
                            if (e > unit_.db_glide_bearing_max) {
                                unit_.db_glide_bearing_max = e;
                            }
                            if (!unit_.db_aimglide_pull_out_76c &&
                                bsp::dive_bomb_aimglide_pull_out_009c57ff(e)) {
                                unit_.db_aimglide_pull_out_76c = true;
                                ++unit_.db_aimglide_pull_outs;
                            }
                        }
                        bsp::DiveBombAimGlideCommandInputs in;
                        // SUBSTITUTION, labelled: the bearing to the aim point
                        // in place of the frame slot 009C5435 reads.
                        in.heading_to_aim_point = unit_.db_bearing_c0;
                        const bsp::DiveBombAimGlideCommand r =
                            bsp::dive_bomb_aimglide_command_009c542c(in);
                        ++unit_.db_aimglide_tick_ticks;
                        bool glide_yaw_arm = false;
                        if (kAimGlideYawBound) {
                            // 009C539C: 00438B10(bearing [ESP+6Ch], aim heading
                            // [ESP+28h]), in the image's argument order.
                            bsp::DiveBombAimHeadingInputs ahin;
                            ahin.pitch_c64 = unit_.plane_pitch_angle_c64;
                            ahin.bank_c68 = unit_.plane_bank_angle_c68;
                            ahin.heading_c6c = unit_.plane_heading_c6c;
                            ahin.body_up_x = unit_.motion.pose_row1[0];
                            ahin.body_up_z = unit_.motion.pose_row1[2];
                            const float e = bsp::wrapped_angle_subtract_00438b10(
                                unit_.db_bearing_c0, bsp::dive_bomb_aim_heading_009c4f80(ahin));
                            // [ESP+1Ch], the planar miss aimPoint - impactPoint.
                            const bsp::DiveBombAimGlideSteer st =
                                bsp::dive_bomb_aimglide_steer_009c53d0(
                                    unit_.db_impact_planar_5c, e);
                            if (st.yaw_arm) {
                                glide_yaw_arm = true;
                                ++unit_.db_glide_yaw_ticks;
                                unit_.plan_state.bank_target_2c4 = 0.0f;   // 009C5400
                                unit_.plan_heading_mode_2cc = 1;           // 009C5408
                                unit_.plan_heading_2c0_written = false;
                                unit_.plan_slots[bsp::kPilotSlotYaw].desired = st.yaw_284;
                                unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;
                            }
                        }
                        if (r.wrote_heading && !glide_yaw_arm) {
                            unit_.plan_heading_2c0 = r.heading_2c0;
                            unit_.plan_heading_2c0_written = true;
                            unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                        }
                        if (kAimGlideThrottleBound) {
                            // [ESP+10h] is the planar distance to the fed aim
                            // point, the quantity 009C7A80 keeps as approach+BCh
                            // from the same point this tick; [ESP+14h] is the
                            // throw to the predicted impact point.
                            const GameUnitsHost::Impl::PilotDiveBombRow& grow =
                                GameUnitsHost::Impl::dive_bomb_row(unit_);
                            const bsp::DiveBombAimGlideThrottle g =
                                bsp::dive_bomb_aimglide_throttle_009c55e5(
                                    unit_.db_planar_bc, unit_.db_impact_throw_14,
                                    grow.max_power_ctrl_050, grow.min_power_ctrl_054);
                            unit_.db_glide_ratio_last = g.ratio_24;
                            unit_.plan_slots[bsp::kPilotSlotThrottle].desired = g.throttle_278;
                            unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = g.air_brake_2a8;
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                            unit_.plane_air_brake_mode_2d8 = 0;  // 009C567F
                        }
                        if (kAimGlidePitchBound) {
                            // 009C5484-009C55DF, the pitch target, mode 2.
                            const GameUnitsHost::Impl::PilotDiveBombRow& prow =
                                GameUnitsHost::Impl::dive_bomb_row(unit_);
                            bsp::DiveBombAimGlidePitchInputs pin;
                            pin.planar_to_aim_10 = unit_.db_planar_bc;
                            pin.planar_to_impact_14 = unit_.db_impact_throw_14;
                            // 009C534B: the same FDIV the throttle's ratio takes.
                            pin.ratio_24 = unit_.db_planar_bc / unit_.db_impact_throw_14;
                            // [ESP+20h] 009C5265-009C5281: unit+100h less the fed
                            // aim point's y, which db_aim_point_height_50 holds.
                            pin.height_above_aim_20 =
                                unit_.motion.position[1] - unit_.db_aim_point_height_50;
                            pin.release_ceiling_1c =
                                prow.new_release_mul_04c * unit_.db_dive_alt_a8;
                            pin.climb_angle_1ec = unit_.plane_climb_angle_1ec;
                            const bsp::DiveBombAimGlidePitch gp =
                                bsp::dive_bomb_aimglide_pitch_009c5484(pin);
                            unit_.db_glide_pitch_last = gp.pitch_target_2bc;
                            unit_.plan_state.pitch_target_2bc = gp.pitch_target_2bc;
                            unit_.plan_state.pitch_mode_2d0 =
                                bsp::dive_bomb_glide_pitch_constant::kPitchMode;
                        }
                    }

                    // 009C62B0's heading arm. PARTIAL: only the heading is
                    // bound, because it is the one the aim trace indicts and
                    // the only one whose value this host can justify. The bank
                    // target, the altitude and the desired speed are read in
                    // include/bsp/dive_bomb_task.hpp and left unbound; their
                    // values need frame slots this body cannot resolve.
                    void run_dive_bomb_flyabove_tick_009c62b0() {
                        ++unit_.db_flyabove_tick_ticks;
                        // The heading arm now runs AFTER the altitude arm,
                        // because the bank predicate BL at 009C650E compares
                        // approach+D4h against C, the commanded altitude
                        // 009C64C9 writes and the altitude arm returns as
                        // `limit_c`. In the image C is computed once at
                        // 009C64C9, ahead of both arms; here the altitude call
                        // is what produces it. Both arms are pure and write
                        // different cmd fields, so the order of the two writes
                        // (009C6DE7 the heading, 009C6F84 the pitch) is
                        // unchanged. Packet cc8_dive_flyover, edited under the
                        // integrator's hunk arbitration of 2026-09-19.
                        // 009C6E10-009C6F91, the altitude arm, BOUND (packet
                        // cc8_dive_entry). It sits on the same straight-line
                        // path as the heading arm above: 009C6DDA and 009C6DF9
                        // both jump forward into it, and nothing between
                        // 009C6DCD and 009C6E10 leaves the body.
                        {
                            bsp::DiveBombFlyAboveAltitudeInputs ain;
                            ain.height_above_aim_b = unit_.db_flyabove_height;
                            ain.begin_altitude_ac = unit_.db_begin_alt_ac;
                            ain.aim_point_height_50 = unit_.db_aim_point_height_50;
                            ain.alt_span_b0 = unit_.db_alt_span_b0;
                            ain.release_altitude_a8 = unit_.db_dive_alt_a8;
                            ain.new_release_mul_40 =
                                GameUnitsHost::Impl::dive_bomb_row(unit_).new_release_mul_04c;
                            // A at 009C63A6 is the planar distance to the aim
                            // point, the same quantity 009C7A80 keeps in
                            // approach+BCh; the flyabove recomputes it locally.
                            ain.planar_distance = unit_.db_planar_bc;
                            // SUBSTITUTION, labelled: approach+0Ch is unit+9D4h
                            // and nothing in this reconstruction fills it, so
                            // the arm takes 009C64B2's fall-back. The ordered
                            // cruise altitude ctl+398h is unreachable here.
                            ain.has_control_block_0c = false;
                            const bsp::DiveBombFlyAboveAltitudeCommand a =
                                bsp::dive_bomb_flyabove_altitude_009c6e10(ain);
                            ++unit_.db_fa_alt_calls;
                            unit_.db_fa_limit_c = a.limit_c;
                            unit_.db_fa_band = a.dead_band;
                            unit_.db_fa_target_last = a.target_altitude;
                            unit_.db_fa_ref_last = a.reference;
                            unit_.db_fa_err_last = a.height_error;
                            if (unit_.db_fa_alt_calls == 1) {
                                unit_.db_fa_err_first = a.height_error;
                            }
                            if (a.level_arm) {
                                // 009C6F89 / 009C6F91.
                                ++unit_.db_fa_level_ticks;
                                unit_.plan_state.pitch_target_2bc = 0.0f;
                                unit_.plan_state.pitch_mode_2d0 = a.pitch_mode_2d0;
                                unit_.plane_commanded_pitch = 0.0f;
                            } else {
                                bsp::PlanePitchCommandInputs pin;
                                pin.desired_altitude = a.target_altitude;
                                pin.reference = a.reference;
                                pin.unit_world_y = unit_.motion.position[1];
                                if (owner_.lua.plane_globals_loaded()) {
                                    const bsp::GameTuningBlock& g =
                                        owner_.lua.plane_globals();
                                    pin.ceiling = g.dynamics_ceiling;
                                    pin.climb_dist = g.pilot_general_climb_dist;
                                    pin.drop_dist = g.pilot_general_drop_dist;
                                }
                                // Packet cc8_dive_race, the same one-field defect
                                // as the run-in above; 009FB800 reads desc+1ECh
                                // itself at 009FB88D. Expected inert in USN04,
                                // because the flyabove's own target is the 210 m
                                // release clamp and every dive bomber here is
                                // above it, so 009FB87C takes the dive arm.
                                pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                                pin.class_drop_angle = unit_.plane_drop_angle;
                                unit_.plane_commanded_altitude = a.target_altitude;
                                unit_.plane_commanded_pitch =
                                    bsp::pitch_command_009fb800(pin);
                                unit_.plan_state.pitch_target_2bc =
                                    unit_.plane_commanded_pitch;
                                unit_.plan_state.pitch_mode_2d0 = 2;
                            }
                            unit_.db_fa_pitch_last = unit_.plane_commanded_pitch;
                        }
                        // Packet cc9_flyover_speed: 009C6F97-009C6FFB, the
                        // desired-speed arm, which follows the altitude arm in
                        // the image. docs/FLYOVER_SPEED.md.
                        if (kFlyoverSpeedBound) {
                            float min_speed = 0.0f;
                            if (owner_.lua.plane_globals_loaded()) {
                                const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                                // tuning+28Ch, derived at 007E413B-007E41DF.
                                const float m28c = bsp::tuning_min_control_multiplier_007e41df(
                                    g.dynamics_spd_multipliers_control_range_min,
                                    g.dynamics_spd_multipliers_control_range_max,
                                    g.dynamics_spd_multipliers_stall_range_max,
                                    g.dynamics_spd_multipliers_level_flight);
                                // 009C6FB0 CALL 007C4810 on approach+8h, the class.
                                min_speed = bsp::plane_min_control_speed_007c4810(
                                    m28c, unit_.plane_stall_spd);
                            }
                            // The frame slot entry-108: 009C6AD9 zeroes it every
                            // tick and only the 007F0280 avoidance arm
                            // (009C6B75, 009C6BB4) overwrites it. 007F0280 is
                            // unbound in this host, its outputs stay zero, and
                            // |0| > 0.05 fails, so the slot is the 0.0 the
                            // image leaves when no unit is in the near field.
                            float slot_entry_108 = 0.0f;
                            if constexpr (GameUnitsHost::Impl::kNearFieldProbeBound) {
                                const bool b18 = unit_.db_flyabove_can_dive_18;
                                float ext[3];
                                bsp::near_field_flyover_extents_009c6aa3(b18, ext);
                                const float w[3] = {30.0f, 0.0f, 0.0f};  // 00CE38C8
                                const bsp::NearFieldProbeResult pr = nf_probe_007f0280(ext, w);
                                slot_entry_108 = bsp::near_field_flyover_slot_009c6b75(pr, b18);
                                if (slot_entry_108 != 0.0f) {
                                    ++unit_.nf_flyover_slot_writes;
                                    if (slot_entry_108 < unit_.nf_flyover_slot_min) unit_.nf_flyover_slot_min = slot_entry_108;
                                    if (slot_entry_108 > unit_.nf_flyover_slot_max) unit_.nf_flyover_slot_max = slot_entry_108;
                                }
                            }
                            unit_.plane_desired_speed_2b4 =
                                bsp::dive_bomb_flyabove_desired_speed_009c6f97(
                                    static_cast<float>(
                                        bsp::dive_bomb_constant::kApproachDriftScaleA4) *
                                        unit_.plane_max_spd,   // approach+A4h, 009C3F16
                                    min_speed,
                                    slot_entry_108);
                            unit_.plane_trg_speed_corr_off_2b0 = 0;   // 009C6FEA
                            unit_.plane_air_brake_mode_2d8 = 1;       // 009C6FF1
                            ++unit_.plane_speed_commands;
                        }
                        // --------------------------------------------------
                        // 009C64EE-009C6DEF, the heading arm. Packet
                        // cc8_dive_flyover; the SUBSTITUTION packet
                        // cc8_dive_heading left here (`command = the bearing to
                        // the lead point`) is withdrawn. What replaces it is
                        // the image's own chain: the bank predicate BL, the
                        // roll-in latch flyabove+1Ch, the dead band on the
                        // bearing error and the slew limiter.
                        //
                        // STILL UNBOUND, and labelled: the avoidance increment
                        // 009C6D59 adds to A, which comes out of the unbound
                        // 007F0280. The 009C64EC query that can veto BL is NOT
                        // in that list any more - it is IsKindOf(RECON_PLANE),
                        // and these are dive bombers.
                        const float bearing_error =
                            bsp::wrapped_angle_subtract_00438b10(
                                unit_.db_flyabove_lead_bearing,
                                unit_.plane_heading_c6c);
                        // 009C642F-009C6453, the class's abs-fold.
                        const float bearing_error_abs =
                            (bearing_error > 0.0f)
                                ? bearing_error
                                : (bsp::dive_bomb_constant::kNegativeZero -
                                   bearing_error);
                        // C as 009C650E sees it is the UNCLAMPED altitude of
                        // 009C64C9, not the `limit_c` the altitude arm returns:
                        // the 210 m release clamp at 009C6580-009C6589 sits
                        // inside the 009C654A branch and runs AFTER the BL test
                        // at 009C6532. Reading limit_c here would compare
                        // approach+D4h (675 m) against 210 m and veto the bank
                        // arm on every tick of every mission.
                        const float unclamped_c = unit_.db_begin_alt_ac +
                                                  unit_.db_aim_point_height_50;
                        const bool bank_arm_bl =
                            bsp::dive_bomb_flyabove_bank_arm_009c6530(
                                /*is_recon_plane=*/false,
                                unit_.db_release_range_d4,
                                unclamped_c,
                                unit_.db_attack_dist_b4,
                                unit_.db_flyabove_lead_range,
                                unit_.db_flyabove_height);
                        if (bank_arm_bl) ++unit_.db_flyabove_bl_ticks;
                        bsp::DiveBombFlyAboveBankInputs bin;
                        bin.latched_1c = unit_.db_flyabove_bank_latch_1c;
                        bin.bank_arm_bl = bank_arm_bl;
                        bin.bearing_error_abs = bearing_error_abs;
                        bin.lead_range_r = unit_.db_flyabove_lead_range;
                        bin.turn_circle_radius = unit_.plane_turn_circle_radius;
                        bin.span_dead_band =
                            bsp::dive_bomb_flyabove_span_dead_band_009c6674(
                                unit_.db_flyabove_span);
                        const bsp::DiveBombFlyAboveBank bank =
                            bsp::dive_bomb_flyabove_bank_009c6857(bin);
                        if (bank.latched_1c &&
                            !unit_.db_flyabove_bank_latch_1c) {
                            unit_.db_flyabove_latch_tick =
                                unit_.dive_bomb_arm_ticks;
                        }
                        unit_.db_flyabove_bank_latch_1c = bank.latched_1c;
                        unit_.db_flyabove_dead_band_t = bank.dead_band_t;
                        unit_.db_flyabove_along_track = bank.along_track;
                        unit_.db_flyabove_cross_track = bank.cross_track;
                        // 009C688F / 009C68E1 clear +19h. READ and NOT applied:
                        // in the image +19h persists across ticks and this
                        // host recomputes it from 009C67B0's rule alone in the
                        // state feed, before the transition rule and before
                        // this tick, so a clear written here would be
                        // overwritten before anything could read it. Modelling
                        // it means giving +19h its own carried state, which is
                        // the 009C6A30/009C6826 pair as well, and that is a
                        // packet rather than a line. `bank.clear_roll_in_19` is
                        // carried for the census only.
                        bsp::DiveBombFlyAboveCommandInputs in;
                        // 009C6DCD / 009C6DDA: the latch suppresses the write.
                        in.suppress_heading_1c = bank.latched_1c;
                        // 009C6A37-009C6A9F then 009C6D6F-009C6DC8. A is
                        // AddWrappedAngle(C, deadband) when 009C6A46's JBE is
                        // not taken and the raw bearing otherwise; the slew
                        // then takes SubtractWrappedAngle(A, C) back off, so
                        // the delta the clamp sees is the dead-band output
                        // either way. L is pi/2: the 10-degree L of 009C6497
                        // needs flyabove+1Bh, which is 0 in this installation
                        // (kOldStyleBombing1b).
                        in.heading_to_aim_point =
                            bsp::dive_bomb_flyabove_slew_009c6d6f(
                                unit_.plane_heading_c6c,
                                bsp::wrapped_angle_add_00438aa0(
                                    unit_.plane_heading_c6c,
                                    bsp::dive_bomb_flyabove_dead_band_009c6a37(
                                        bearing_error, bank.dead_band_t)),
                                bsp::dive_bomb_flyabove_constant::
                                    kHeadingSlewLimit);
                        const bsp::DiveBombFlyAboveCommand r =
                            bsp::dive_bomb_flyabove_command_009c6dcd(in);
                        if (!r.wrote_heading) ++unit_.db_flyabove_latched_ticks;
                        if (r.wrote_heading) {
                            ++unit_.db_flyabove_heading_writes;
                            unit_.db_flyabove_heading_last = r.heading_2c0;
                            // 009C6DE7 and 009C6DEF. Mode 2 is the planner's own
                            // roll arm, the one 0099DE8A lets through and
                            // pilot_plan_roll_0099e2ba models.
                            unit_.plan_heading_2c0 = r.heading_2c0;
                            unit_.plan_heading_2c0_written = true;
                            unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                        }
                    }

                    // 009C58D0's steering, the part of the aimdive tick that
                    // aims. Until this was bound, `tick_state` was empty and
                    // 664 live aimdive ticks issued no command at all: the
                    // planner levelled the aircraft and it passed the target
                    // at 444 m, which is the whole of the 340 m miss the aim
                    // census reported. docs/DIVE_BOMB_TASK.md.
                    void run_dive_bomb_aimdive_tick_009c58d0() {
                        bsp::DiveBombAimDiveSteerInputs in;
                        // 009C5C9B's own result, computed for this same tick by
                        // dive_bomb_aimdive_inputs() a few lines earlier.
                        in.aim_error = unit_.db_aim_error_last;
                        // 009C5BCC/009C5BD4: the gate that decides whether the
                        // aim error is computed at all, and what [ESP+5Ch]
                        // still holds when it is not.
                        in.pitch_c64 = unit_.plane_pitch_angle_c64;
                        // [ESP+5Ch] is the second sqrt, the impact point to aim
                        // point range - not the aircraft's own.
                        in.planar_distance_slot_5c =
                            unit_.db_impact_planar_5c >= 0.0f
                                ? unit_.db_impact_planar_5c : unit_.db_planar_bc;
                        // 009C5935 calls 009C4F80 for the heading and
                        // 009C5AA3 subtracts the bearing FROM it - arg0 at
                        // [ESP] is the 009C4F80 result, arg1 at [ESP+4] is the
                        // bearing - so the interpolant's x is
                        // (aim heading - bearing), not (bearing - heading).
                        // Both the order and the source stood wrong here: the
                        // raw pose+C6Ch is off by pi for the whole of the dive,
                        // because 009C7EA0's second arm ends the turndown while
                        // the aircraft is still inverted.
                        bsp::DiveBombAimHeadingInputs hin;
                        hin.pitch_c64 = unit_.plane_pitch_angle_c64;
                        hin.bank_c68 = unit_.plane_bank_angle_c68;
                        hin.heading_c6c = unit_.plane_heading_c6c;
                        hin.body_up_x = unit_.motion.pose_row1[0];
                        hin.body_up_z = unit_.motion.pose_row1[2];
                        unit_.db_aim_heading_last =
                            bsp::dive_bomb_aim_heading_009c4f80(hin);
                        // 009C5D60's arm takes the bearing measured from the
                        // aircraft, 009C5D33's the one measured from the
                        // predicted impact point. Both from the same heading.
                        in.bearing_error = bsp::wrapped_angle_subtract_00438b10(
                            unit_.db_aim_heading_last, unit_.db_bearing_c0);
                        in.bearing_error_wide_18 =
                            bsp::wrapped_angle_subtract_00438b10(
                                unit_.db_aim_heading_last,
                                unit_.db_impact_bearing_18);
                        // 009C5919-009C592D folds pose+C68h into the slot the
                        // band test at 009C5D2C reads.
                        in.bank_c68 = unit_.plane_bank_angle_c68;
                        // (approach+14h)->+64h and ->+68h at 009C5CAB and
                        // 009C5CD0, RECOVERED: 009F9D1E makes approach+14h a
                        // 0x248-stride robots row viewed 0xCh in, so these are
                        // row+70h and row+74h, DiveBombAimPrecPullPlus and
                        // PullMinus. Same substitution class as the two the aim
                        // error already uses, and from the same row.
                        in.pitch_gain_positive_64 =
                            GameUnitsHost::Impl::dive_bomb_row(unit_).aim_prec_pull_plus_070;
                        in.pitch_gain_negative_68 =
                            GameUnitsHost::Impl::dive_bomb_row(unit_).aim_prec_pull_minus_074;
                        const bsp::DiveBombAimDiveSteerResult r =
                            bsp::dive_bomb_aimdive_steer_009c5c9f(in);
                        ++unit_.db_aimdive_steer_ticks;
                        unit_.db_aimdive_pitch_last = r.pitch_29c;
                        unit_.db_aimdive_roll_last = r.roll_290;
                        unit_.db_aimdive_bearing_last = in.bearing_error;
                        // 009C5CFA, 009C5D15, 009C5D1C.
                        unit_.plan_slots[bsp::kPilotSlotPitch].desired = r.pitch_29c;
                        unit_.plan_slots[bsp::kPilotSlotPitch].active = 1;
                        unit_.plan_state.pitch_mode_2d0 = 0;
                        // 009C5DA3, 009C5DAB, 009C5DB2. Mode 0 is neither the
                        // planner's 2 nor its 1, so its roll arm is skipped and
                        // this command is what reaches the stick.
                        unit_.plan_slots[bsp::kPilotSlotRoll].desired = r.roll_290;
                        unit_.plan_slots[bsp::kPilotSlotRoll].active = 1;
                        unit_.plan_heading_mode_2cc = 0;
                        unit_.plan_heading_2c0_written = false;
                        // 009C6080: cmd+2D8h = 0, the air-brake mode.
                        unit_.plane_air_brake_mode_2d8 = 0;
                        // Packet cc9_aimdive_response: 009C5DB8-009C6080, the
                        // yaw, throttle and air-brake commands that follow the
                        // roll. docs/AIMDIVE_RESPONSE.md.
                        if (kAimDiveTailBound &&
                            unit_.command_target_plus_one != 0 &&
                            unit_.command_target_plus_one - 1 < owner_.slots.size()) {
                            const std::size_t ti = unit_.command_target_plus_one - 1;
                            // 009C5DF1: approach->vtable[0], the fed aim point.
                            float ap[3] = {owner_.slots[ti]->motion.position[0],
                                           owner_.slots[ti]->motion.position[1],
                                           owner_.slots[ti]->motion.position[2]};
                            hull_aim_world_point(unit_, *owner_.slots[ti], ti + 1, ap);
                            // 009C5DE0 00B63D50 then 009C5E01 004142E0: the
                            // orthogonal scaled inverse of the pose at +CCh,
                            // i.e. (p - t) . row_j / |row_j|^2.
                            const bsp::CameraMatrix& m = unit_.world;
                            const double dx = static_cast<double>(ap[0]) - m[12];
                            const double dy = static_cast<double>(ap[1]) - m[13];
                            const double dz = static_cast<double>(ap[2]) - m[14];
                            const double r0 = static_cast<double>(m[0]) * m[0] +
                                static_cast<double>(m[1]) * m[1] + static_cast<double>(m[2]) * m[2];
                            const double r2 = static_cast<double>(m[8]) * m[8] +
                                static_cast<double>(m[9]) * m[9] + static_cast<double>(m[10]) * m[10];
                            bsp::DiveBombAimDiveTailInputs tin;
                            tin.aim_local_x = static_cast<float>(
                                (dx * m[0] + dy * m[1] + dz * m[2]) / r0);
                            tin.aim_local_z = static_cast<float>(
                                (dx * m[8] + dy * m[9] + dz * m[10]) / r2);
                            tin.pitch_c64 = unit_.plane_pitch_angle_c64;
                            tin.bank_c68 = unit_.plane_bank_angle_c68;
                            // unit->vtable[38h]: the same live-velocity length
                            // the torpedo seam's unit_speed_vtable38 returns.
                            const double vx = unit_.motion.linear_velocity.x;
                            const double vy = unit_.motion.linear_velocity.y;
                            const double vz = unit_.motion.linear_velocity.z;
                            tin.speed_vtable38 = static_cast<float>(
                                std::sqrt(vx * vx + vy * vy + vz * vz));
                            tin.pitch_spd_1ac = unit_.plane_pitch_spd;   // class+1ACh
                            // [ESP+14h], 009C59D6: unit+100h less the aim point y.
                            tin.height_above_aim_14 = unit_.motion.position[1] - ap[1];
                            tin.release_alt_a8 = unit_.db_dive_alt_a8;
                            tin.pitch_command = r.pitch_29c;
                            // This installation's robots.lua, PilotBot SPNormal
                            // row (the one kDiveBombAimPrecPullPlus comes from):
                            // DiveBombMaxPowerCtrl 0.7, MinPowerCtrl 0.2,
                            // MaxBrakeCtrl 0.5, MinBrakeCtrl 0.0, AimPitchRatio 3.0.
                            // Packet cc9_difficulty: the captured row's.
                            const GameUnitsHost::Impl::PilotDiveBombRow& trow =
                                GameUnitsHost::Impl::dive_bomb_row(unit_);
                            tin.max_power_44 = trow.max_power_ctrl_050;
                            tin.min_power_48 = trow.min_power_ctrl_054;
                            tin.max_brake_4c = trow.max_brake_ctrl_058;
                            tin.min_brake_50 = trow.min_brake_ctrl_05c;
                            tin.aim_pitch_ratio_54 = trow.aim_pitch_ratio_060;
                            const bsp::DiveBombAimDiveTail tail =
                                bsp::dive_bomb_aimdive_tail_009c5db8(tin);
                            // 009C5E5D, 009C5E3F, 009C5E63 (+2D4h = 0).
                            unit_.plan_slots[bsp::kPilotSlotYaw].desired = tail.yaw_284;
                            unit_.plan_slots[bsp::kPilotSlotYaw].active = 1;
                            // 009C605F/009C5F37 and 009C606A.
                            unit_.plan_slots[bsp::kPilotSlotThrottle].desired = tail.throttle_278;
                            unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                            // 009C6071, 009C6079.
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].desired = tail.air_brake_2a8;
                            unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                        }
                    }

                    void run_dive_bomb_turndown_tick_009c44f0() {
                        bsp::DiveBombTurnDownInputs in;
                        in.bank_c68 = unit_.plane_bank_angle_c68;
                        in.pitch_c64 = unit_.plane_pitch_angle_c64;
                        in.rolled_latch_1c = unit_.db_turndown_latch_1c;
                        in.roll_command_18 = unit_.db_turn_roll_18;
                        // 007C47F0(approach+8h): tuning+24Ch LevelFlight times
                        // classDesc+184h StallSpd. Both halves are now the real
                        // ones - the tuning row from the PlaneGlobals mirror and
                        // the stall speed from this unit's own vehicle-class row,
                        // the same desc+184h the free-flight arm reads at
                        // 007DB760 - so the substitution that stood here is
                        // retired. docs/BOT_SPEED_CLASS_ROWS.md.
                        in.desired_speed = owner_.bot_desired_speed_007c47f0(unit_);
                        const bsp::DiveBombTurnDownResult r =
                            bsp::dive_bomb_turndown_tick_009c44f0(in);
                        if (unit_.db_turndown_entry_range < 0.0f) {
                            unit_.db_turndown_entry_range = unit_.db_planar_bc;
                            unit_.db_turndown_entry_bearing =
                                bsp::wrapped_angle_subtract_00438b10(
                                    unit_.db_bearing_c0, unit_.plane_heading_c6c);
                        }
                        ++unit_.db_turndown_ticks;
                        unit_.db_turndown_bank_last = r.folded_bank;
                        unit_.db_turndown_pose_c64_last = unit_.plane_pitch_angle_c64;
                        if (unit_.db_turndown_ticks == 1 ||
                            unit_.plane_pitch_angle_c64 < unit_.db_turndown_pose_c64_min) {
                            unit_.db_turndown_pose_c64_min = unit_.plane_pitch_angle_c64;
                        }
                        if (r.latch_1c_set) {
                            if (!unit_.db_turndown_latch_1c) {
                                unit_.db_turndown_latched_tick =
                                    unit_.dive_bomb_arm_ticks;
                            }
                            unit_.db_turndown_latch_1c = true;
                        }
                        // 009C4512-009C4524, the desired-speed pair and the
                        // one-shot docs/PILOT_THROTTLE_CUT_RAISER.md names.
                        unit_.plane_desired_speed_2b4 = r.speed_2b4;
                        unit_.plane_air_brake_mode_2d8 = 1;
                        ++unit_.plane_speed_commands;
                        if (r.wrote_roll) {
                            ++unit_.db_turndown_roll_writes;
                            unit_.db_turndown_roll_last = r.roll_290;
                            unit_.plan_slots[bsp::kPilotSlotRoll].desired = r.roll_290;
                            unit_.plan_slots[bsp::kPilotSlotRoll].active = 1;
                            // 009C462F cmd+2CCh = 0. That is the value the
                            // planner's gate at 0099E275 lets through, so this
                            // is what keeps the roll command alive.
                            unit_.plan_heading_mode_2cc = 0;
                            unit_.plan_heading_2c0_written = false;
                        }
                        if (r.released_roll) {
                            // 009C4646 cmd+2C4h = pi and 009C464E cmd+2CCh = 1.
                            // Mode 1 PASSES the planner's compare at 0099E26E,
                            // so the planner's roll servo runs, and because
                            // 0099E25C was skipped it drives toward the pi the
                            // turndown just wrote: the hand-over rolls the
                            // aircraft the rest of the way to inverted.
                            unit_.plan_state.bank_target_2c4 =
                                bsp::dive_bomb_turndown_constant::kPi;
                            unit_.plan_heading_mode_2cc = 1;
                        }
                        if (r.wrote_pitch) {
                            ++unit_.db_turndown_pitch_writes;
                            unit_.db_turndown_pitch_last = r.pitch_29c;
                            unit_.plan_slots[bsp::kPilotSlotPitch].desired = r.pitch_29c;
                            unit_.plan_slots[bsp::kPilotSlotPitch].active = 1;
                            // 009C469C and 009C472A, `MOV [reg+2D0h],EBP` with
                            // EBP zeroed at 009C44FB: both pitch arms, before and
                            // after the latch, leave the pitch mode at 0. That is
                            // the value the planner's gate at 0099E3BF lets
                            // through, so this deflection is what reaches the
                            // elevator instead of the planner's own demand.
                            unit_.plan_state.pitch_mode_2d0 = 0;
                        }
                        if (r.wrote_altitude_hold) {
                            // 009C46AA: +2BCh = 0 with mode +2D0h = 2, which
                            // sends the think back through the planner's arm.
                            unit_.plan_state.pitch_target_2bc = 0.0f;
                            unit_.plan_state.pitch_mode_2d0 = 2;
                        }
                    }

                    static int dive_bomb_state_bucket(bsp::DiveBombState s) {
                        switch (s) {
                            case bsp::DiveBombState::kMoveTo: return 0;
                            case bsp::DiveBombState::kFollow: return 1;
                            case bsp::DiveBombState::kPrepare: return 2;
                            case bsp::DiveBombState::kDone: return 3;
                            case bsp::DiveBombState::kGoAway: return 4;
                            case bsp::DiveBombState::kAimDive: return 5;
                            case bsp::DiveBombState::kAimGlide: return 6;
                            case bsp::DiveBombState::kFlyAbove: return 7;
                            case bsp::DiveBombState::kTurnDown: return 8;
                            case bsp::DiveBombState::kAttackRun: return 9;
                            default: return -1;
                        }
                    }

                    // 0099A170 builds a kind Eh task only for a unit whose
                    // class carries torpedo ordnance (kind 2Bh, 007ED8D0 ->
                    // 007B93F0) under a PilotSetTarget-class order.
                    void run_torpedo_task_arm_009d4850(float dt) {
                        const bsp::OrdnanceKindSet set{unit_.ordnance_mask};
                        if (unit_.command_target_plus_one == 0) return;
                        if (!unit_.torpedo_task_installed) {
                            // cc8_torpedo_retire. The kind 2Bh test is an
                            // INSTALL condition, not a per-tick one, and until
                            // this line moved inside the install block it was
                            // both. 0099A170 makes the test once, when the
                            // attack order is issued, to pick WHICH task class
                            // to construct (docs/TORPEDO_RELEASE_ORDERS.md); the
                            // task it builds is not destroyed when the loadout
                            // empties. 009D4C10 proves that directly: its
                            // ordnance arm, `if (IsAttackState(cur) &&
                            // task+52Ah) return false`, only ever lets the range
                            // test through once task+52Ah is CLEAR, so the image
                            // arms this task after the drop -- an image that
                            // stopped arming a spent task could never reach the
                            // break-off at all.
                            //
                            // MEASURED, local/retire_arm_usn01.log: with the
                            // drop clearing the bit (e3cd4b997) this guard fired
                            // on the tick after each release and the arm stopped
                            // dead -- Mav1 arm_ticks=678 = attackrun 424 + aim
                            // 254, no goaway tick, no state tick after it, and
                            // the aircraft flew its last commanded descent into
                            // the water 166 ticks later. That, and not the
                            // break-off predicate, is the whole of the deaths
                            // 2 -> 5 regression: the same log shows 009D4C10
                            // true on exactly ONE tick per aircraft, arm tick 0
                            // in moveto at 4183 m, and false at the drop.
                            if (!bsp::ordnance_has_torpedo_2bh(set)) return;
                            unit_.torpedo_task_installed = true;
                            // 009D3050 leaves +310h on the moveto/follow pair
                            // 009D2DA0 registered; 009D24E0 leaves +98h at -1.
                            // FED, packet cc8_follow_enter. 009D30B7 CALL 007B8AD0
                            // / 009D30BC TEST AL,AL / 009D30BE LEA ECX,[ESI+544h]
                            // / 009D30C4 JNZ over 009D30C6 LEA ECX,[ESI+580h] -
                            // the same flight-leader choice the dive bomber makes.
                            unit_.torpedo_state =
                                owner_.unit_is_flight_leader_007b8ad0(
                                    unit_.process_index)
                                    ? bsp::TorpedoState::kMoveTo
                                    : bsp::TorpedoState::kFollow;
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
                            // 009D049D/009D04A0 seed approach+84h from the SAME
                            // robots row, one field further on (row+18h), as a
                            // bare FLD/FSTP with NO ratio.
                            //
                            // WHICH TWO ARE SCALED AND WHICH IS NOT, because
                            // mirroring the neighbours here would silently
                            // corrupt the gate: 009D0491 writes +7Ch as
                            // record[+4] * approach+24h and 009D0497 writes
                            // +80h as record[+8] * approach+24h - both SCALED -
                            // while 009D049D/009D04A0 is FLD then FSTP with
                            // nothing between. approach+84h is a unitless
                            // factor and multiplying it by a speed ratio would
                            // be caught by no compile and no census.
                            //
                            // It is also the only one the jitter leaves alone.
                            // The 00BD2F10 draws at 009D0581-009D0625 rewrite
                            // +7Ch (009D05ED) and +80h (009D0625); a census of
                            // the whole of 009D0380 finds [ESI+84h] written
                            // EXACTLY ONCE, at 009D04A0. So this is a straight
                            // read of the row value: no scale, no draw.
                            // It used to be forced to 1.0f at the aim-tick
                            // binding, which made both endpoints of the
                            // interpolation at 009D1FED equal and the release
                            // gate inert. docs/TORPEDO_RELEASE_GATE.md.
                            ap.aspect_scale_84 = kTorpReleaseDropCloserMulSPNormal;
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
                        } else if (ctx.current == bsp::TorpedoState::kGoAway) {
                            // 009D0F10. Until this packet the goaway state ran
                            // NO tick at all, which is why the five bombers
                            // held their post-release descent into the water:
                            // nothing commanded the climb-away.
                            run_goaway_tick_009d0f10(dt);
                        } else if (ctx.current == bsp::TorpedoState::kMoveTo ||
                                   ctx.current == bsp::TorpedoState::kFollow) {
                            run_move_to_tick_009c18c0();
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
                            //
                            // CORRECTED, packet cc8_torpedo_descent_law: the
                            // three zeros this used to pass were the whole of
                            // the plunge. 009D0951-009D0A92 computes all four
                            // arguments, and 009FBA50's range term is the glide
                            // slope that brings the aircraft down to the release
                            // altitude AT the release distance rather than
                            // straight away. docs/TORPEDO_DESCENT_LAW.md.
                            const bsp::TorpedoApproachState& ap =
                                unit_.torpedo_approach;
                            bsp::TorpedoAttackRunAltitudeInputs ain;
                            ain.alt_floor_74 = ap.alt_floor_74;
                            ain.alt_margin_78 = ap.alt_margin_78;
                            ain.speed_late_7c = ap.speed_late_7c;
                            ain.speed_early_80 = ap.speed_early_80;
                            ain.range_90 = ap.range_90;
                            ain.elapsed_134 = ap.elapsed_134;
                            ain.unit_world_y = unit_.motion.position[1];
                            ain.drop_angle = unit_.plane_drop_angle;
                            const bsp::TorpedoAttackRunAltitudeCommand acmd =
                                bsp::torpedo_attack_run_altitude_009d0951(ain);
                            binding.command_altitude_and_throttle(
                                nullptr, acmd.base, acmd.range_low,
                                acmd.range_high, acmd.scale);
                            ++unit_.torpedo_attackrun_altitude_commands;
                            if ((unit_.torpedo_attackrun_altitude_commands % 50) == 1) {
                                owner_.log.notef("  torpedo %-12s descent census "
                                    "n=%d base=%.2f (74h=%.2f 78h=%.2f) "
                                    "commanded=%.2f live_alt=%.1f pitch_demand=%.4f "
                                    "pitch=%.4f drop_angle=%.4f climb_1ec=%.4f "
                                    "| range=%.1f low=%.1f span=%.1f margin=%.1f "
                                    "denom=%.1f scale=%.4f gain=%.4f slope_deg=%.2f",
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
                                    static_cast<double>(unit_.plane_climb_angle_1ec),
                                    static_cast<double>(acmd.range_high),
                                    static_cast<double>(acmd.range_low),
                                    static_cast<double>(
                                        acmd.range_high - acmd.range_low > 0.0f
                                            ? acmd.range_high - acmd.range_low : 0.0f),
                                    static_cast<double>(acmd.margin),
                                    static_cast<double>(acmd.denom),
                                    static_cast<double>(acmd.scale),
                                    static_cast<double>(acmd.class_gain),
                                    static_cast<double>(
                                        std::atan(acmd.scale * acmd.class_gain) * 57.2957795));
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
                            // FED, packet cc8_follow_enter. 009D3210
                            // BSP_BotTaskTorpedo_IsEngaged calls 007B8AD0 at
                            // 009D323A and 009D3241 JZ returns 0 when it is
                            // false: a wing member CANNOT self-engage. Only the
                            // flight leader reaches the range test
                            // 009D3243-009D3259 (`[484h] * 2.2 > [488h]`,
                            // 00D05AC8 = 2.2). A member becomes engaged only
                            // when the squadron's mode 009D322B reaches 2.
                            arm.engaged.unit_has_no_follow_target =
                                owner_.unit_is_flight_leader_007b8ad0(
                                    unit_.process_index);
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
                        if constexpr (GameUnitsHost::Impl::kPlaneFixedStepPredicatesBound) {
                            // game+1FE4h is the session mode, 0 for none
                            // (docs/GAME_SESSION_POLLS.md); this process runs
                            // single player, so 0 is the value, not a stand-in.
                            owner_.done("App::state_1fe4", 0x007cea02u);
                        } else {
                            owner_.log.unimplemented("App::state_1fe4", "007cea02");
                        }
                        in.app_state_1fe4 = 0;
                        // 007CEA0F: the same unit+9E0h the airborne
                        // accumulator is gated on, which the plane binding
                        // already reads.
                        in.blocked_9e0 = unit_.plane_airborne_frozen_9e0;
                        // 007CEA1C and 007CEA29. contract: unread.
                        if constexpr (GameUnitsHost::Impl::kPlaneFixedStepPredicatesBound) {
                            // unit+C3Ah: its only run-time writer is the plane
                            // state-message arm 007D1314 (after unit+C39h, then
                            // vtable[70h](1)); the constructor zeroes it at
                            // 007D0136 and 007B84D0's setter has no caller. This
                            // host delivers no plane state message, so clear is
                            // exact here. unit+5Dh is the scene byte `simulate`.
                            // Packet cc9_plane_death_modes: the death message
                            // tail 007D1314 now runs, so C3Ah is set from death.
                            in.blocked_c3a = GameUnitsHost::Impl::kPlaneDeathModesBound
                                && unit_.plane_death_c3a;
                            in.blocked_5d = unit_.state != nullptr && unit_.state->simulate != 0;
                            owner_.done("Plane::issue_block_c3a", 0x007cea1cu);
                            owner_.done("Unit::issue_block_5d", 0x007cea29u);
                        } else {
                            owner_.log.unimplemented("Plane::issue_block_c3a", "007cea1c");
                            owner_.log.unimplemented("Unit::issue_block_5d", "007cea29");
                            in.blocked_c3a = false;
                            in.blocked_5d = false;
                        }
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
                        if constexpr (GameUnitsHost::Impl::kPlaneFixedStepPredicatesBound) {
                            // class+1F4h is BombDelay (007D2318; plane_class_fields).
                            in.class_interval_scale_1f4 = unit_.plane_bomb_delay_1f4;
                            owner_.done("PlaneClass::issue_interval_1f4", 0x007ceab8u);
                        } else {
                            owner_.log.unimplemented("PlaneClass::issue_interval_1f4",
                                                     "007ceab8");
                            in.class_interval_scale_1f4 = 1.0f;
                        }

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
                        // approach+CCh's target entity. This used to return the
                        // no-target arm on all three (0.0f / false / false),
                        // with a comment saying that was "exactly the no-target
                        // arm of the native branch". It is - but it made the
                        // aspect gate BLIND, and that was measured, not
                        // reasoned: src/torpedo_aim_tick.cpp:122 seeds
                        // `f2c = 0.0f` and only assigns it at :134 under
                        // `has_target_cc() && target_is_kind_vtable5c(...)`, so
                        // with those false the slot stays 0, |cos(0)| is 1.0,
                        // and the interpolation at 009D1FED returns its y1 -
                        // approach+84h - on every tick whatever the real
                        // aspect. Binding approach+84h to the authored 0.7 then
                        // moved every release from a flat 450 m to a flat 315 m
                        // instead of making it depend on aspect: USN01 fell
                        // 433-441 -> 300-305 m on all five rounds, including
                        // four whose real |cos| is below the 0.5 knee and must
                        // not have moved at all. docs/TORPEDO_RELEASE_GATE.md
                        // section 2.6.
                        //
                        // So the aspect is bound here from the ordered target.
                        // pose_heading_radians is exactly what
                        // GameUnitsHost::unit_heading_radians calls, and it is
                        // the same quantity the drop census takes for BOTH
                        // headings, so this aspect is commensurable with the
                        // `crossing` column (the check is recorded at
                        // src/game_hosts_gunnery.cpp:3270-3275).
                        const GameUnitSlot* aim_target() const {
                            if (s_.command_target_plus_one == 0) return nullptr;
                            const std::size_t i =
                                s_.command_target_plus_one - 1;
                            if (i >= owner_.slots.size()) return nullptr;
                            return owner_.slots[i].get();
                        }
                        float target_heading_vtable50() override {
                            // 009D1721, the target's vtable[50h] heading.
                            const GameUnitSlot* const t = aim_target();
                            return t != nullptr
                                ? GameUnitsHost::Impl::pose_heading_radians(*t)
                                : 0.0f;
                        }
                        // 009D1710/009D1714 probes the target with
                        // vtable[5Ch](6), and kind 6 is the SHIP BASE across
                        // this project - kKindKamikazeTargetShip
                        // (attack_commands.hpp:71), kKillCreditKindShipBase
                        // (kill_credit.hpp:30), kUnitGunneryKindShipBase
                        // (game_hosts_ai.cpp:792). So the image admits the
                        // aspect for a SHIP target and keeps the 009D16AA zero
                        // otherwise - an aircraft chasing an aircraft gets no
                        // aspect term.
                        //
                        // BOUND, packet cc9_torpedo_kind: the image's own probe
                        // with the image's own argument. bsp::unit_is_kind_of
                        // decodes all 88 compiled slot-5Ch bodies and answers
                        // from the object's own class id, so it is the routine
                        // the target's vtable reaches whatever class the target
                        // is (0074E400 is the plane body among them). This
                        // replaces `aim_target() != nullptr`, which gave a
                        // NON-ship target an aspect the image withholds. Every
                        // ordered target in USN01 and USN04 is a ship, and the
                        // confirming pair reproduced both missions to the digit:
                        // docs/TORPEDO_KIND_PROBE.md section 3.
                        bool target_is_kind_vtable5c(int query) override {
                            const GameUnitSlot* const t = aim_target();
                            return t != nullptr && bsp::unit_is_kind_of(t->class_id, query);
                        }
                        // Packet cc9_torpedo_kind: the own-unit probe,
                        // [[state+4]+4] = the aircraft, at 009D175F/009D176E
                        // (the 0.9 tighten of range and time to target) and
                        // 009D1E08/009D1E17 (the 2.5/1.5 pitch denominator),
                        // kinds 10h MPlaneBomber and 16h MLargeReconPlane. It
                        // was `return false`, so neither use ever fired. Only
                        // USN01's five H6K Mavis (LargeReconPlane) reach it;
                        // USN04's Kates are TorpedoBomber and are untouched.
                        // docs/TORPEDO_KIND_PROBE.md sections 6-8.
                        bool unit_is_kind_vtable5c(int query) override {
                            return bsp::unit_is_kind_of(s_.class_id, query);
                        }
                        bool has_target_cc() override {
                            return aim_target() != nullptr;
                        }
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

                    // 009C18C0's steps 2 and 5, the two halves that matter to a
                    // torpedo bomber's approach. docs/TORPEDO_MOVETO_TICK.md and
                    // docs/TORPEDO_AIM_ALT_AND_SAFE_DIST.md.
                    void run_move_to_tick_009c18c0() {
                        const bsp::TorpedoApproachState& ap = unit_.torpedo_approach;
                        // Step 1: the planar separation. approach+90h already
                        // carries it, which is what 009C196C's square root
                        // produces and holds in EBX to the 009FBA50 call.
                        const float distance = ap.range_90;
                        if (!(distance > 0.0f)) return;

                        // Step 2, 009C1850 BSP_BotStateMoveTo_SetDesiredSpeed.
                        // 009C189A writes cmd+2B4h, 009C18A0 clears the byte
                        // cmd+2B0h and 009C18A7 raises cmd+2D8h, with NO
                        // condition. The speed is 007C47F0's product, tuning+24Ch
                        // LevelFlight times this unit's own classDesc+184h
                        // StallSpd, which the dive-bomb turndown reads through the
                        // same call - so the earlier substitution here, the row's
                        // TravelSpeed, was the wrong FIELD and not merely a stand
                        // -in value. 009BECD0 then shapes that product against the
                        // distance and is still unread, so what remains
                        // substituted is the shaping, not the speed.
                        // docs/BOT_SPEED_CLASS_ROWS.md.
                        unit_.plane_desired_speed_2b4 =
                            owner_.bot_desired_speed_007c47f0(unit_);
                        unit_.plane_air_brake_mode_2d8 = 1;
                        ++unit_.plane_speed_commands;
                        owner_.record("BotStateMoveTo::set_desired_speed", 0x009c1850u);

                        // Step 5, the glide slope. 009C1B17 calls 009FBA50 with
                        // base = max(state+34h + targetY, state+30h), rangeLow =
                        // state+38h and rangeHigh = the distance, and the torpedo
                        // arm 009D48CF fills those three with the RELEASE
                        // ALTITUDE approach+74h + approach+78h in both altitude
                        // slots and the release distance approach+7Ch or +80h in
                        // the third, on the approach+134h >= 15.0 switch.
                        const float base = ap.alt_floor_74 + ap.alt_margin_78;
                        const float range_low = ap.elapsed_134 >= 15.0f
                            ? ap.speed_late_7c : ap.speed_early_80;
                        // 009C1AF3: Interp(0.05, 0.35, 0.4, 1.6, margin/denom),
                        // with margin = max(1400 - unit+100h, 50) and denom the
                        // distance less 1000, clamped into [50, 2000].
                        float margin = 1400.0f - unit_.motion.position[1];
                        if (margin < 50.0f) margin = 50.0f;
                        float denom = distance - 1000.0f;
                        if (denom < 50.0f) denom = 50.0f;
                        if (denom >= 2000.0f) denom = 2000.0f;
                        const float t = bsp::clamped_interpolate_00419010(
                            0.05f, 0.35f, 0.4f, 1.6f, margin / denom);
                        // class+518h, derived at 007C4A44 as tan(DropAngle), so
                        // the span term is horizontalDistance * tan(angle).
                        const float gain = static_cast<float>(
                            std::tan(static_cast<double>(unit_.plane_drop_angle)));
                        bsp::PlaneCruiseAltitudeInputs cin;
                        cin.base_altitude = base;
                        cin.range_low = range_low;
                        cin.range_high = distance;
                        cin.scale = t;
                        cin.class_gain = gain;
                        cin.has_squadron = false;
                        if (owner_.lua.plane_globals_loaded()) {
                            cin.ceiling = owner_.lua.plane_globals().dynamics_ceiling;
                        }
                        const bsp::PlaneCruiseAltitudeResult c =
                            bsp::cruise_altitude_command_009fba50(cin);
                        bsp::PlanePitchCommandInputs pin;
                        pin.desired_altitude = c.clamped_altitude;
                        pin.reference = c.pitch_reference;  // 009FBB06, not the altitude
                        pin.unit_world_y = unit_.motion.position[1];
                        pin.ceiling = cin.ceiling;
                        if (owner_.lua.plane_globals_loaded()) {
                            const bsp::GameTuningBlock& g = owner_.lua.plane_globals();
                            pin.climb_dist = g.pilot_general_climb_dist;
                            pin.drop_dist = g.pilot_general_drop_dist;
                        }
                        pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                        pin.class_drop_angle = unit_.plane_drop_angle;
                        const float demand = bsp::pitch_command_009fb800(pin);
                        unit_.plane_commanded_altitude = c.clamped_altitude;
                        unit_.plane_commanded_pitch = demand;
                        unit_.plan_state.pitch_target_2bc = demand;
                        // 009C1B17 CALL 009FBA50 -> 009FB800, which writes
                        // cmd+2D0h = 2. docs/PITCH_COMMAND_CALLERS.md.
                        if constexpr (GameUnitsHost::Impl::kPitchCommandCallersBound) {
                            unit_.plan_state.pitch_mode_2d0 = 2;
                        }
                        owner_.record("BotStateMoveTo::glide_slope", 0x009c18c0u);
                        if ((unit_.plane_speed_commands % 50) == 1) {
                            owner_.log.notef("  torpedo %-12s glide census n=%d "
                                "range=%.1f base=%.2f low=%.1f t=%.3f gain=%.3f "
                                "commanded=%.1f live_alt=%.1f pitch_demand=%.4f "
                                "desired_spd=%.2f (level_flight*stall) stall=%.2f "
                                "|v|=%.2f throttle=%.3f",
                                unit_.row.name.c_str(), unit_.plane_speed_commands,
                                static_cast<double>(distance),
                                static_cast<double>(base),
                                static_cast<double>(range_low),
                                static_cast<double>(t), static_cast<double>(gain),
                                static_cast<double>(c.clamped_altitude),
                                static_cast<double>(unit_.motion.position[1]),
                                static_cast<double>(demand),
                                static_cast<double>(unit_.plane_desired_speed_2b4),
                                static_cast<double>(unit_.plane_stall_spd),
                                static_cast<double>(std::sqrt(
                                    unit_.plane_world_velocity[0] * unit_.plane_world_velocity[0] +
                                    unit_.plane_world_velocity[1] * unit_.plane_world_velocity[1] +
                                    unit_.plane_world_velocity[2] * unit_.plane_world_velocity[2])),
                                static_cast<double>(unit_.plane_live_throttle));
                        }
                    }

                    // The same lookup TorpedoApproachBinding::target() does, on
                    // this class's own slot reference.
                    const GameUnitSlot* goaway_target() const {
                        if (unit_.command_target_plus_one == 0) return nullptr;
                        const std::size_t i = unit_.command_target_plus_one - 1;
                        if (i >= owner_.slots.size()) return nullptr;
                        return owner_.slots[i].get();
                    }

                    // 009D0C10 then 009D0F10, the goaway state's geometry
                    // update and its tick. docs/TORPEDO_FLY_TO_SOLVER.md and
                    // docs/TORPEDO_AFTER_THE_DROP.md section 3.1.
                    void run_goaway_tick_009d0f10(float dt) {
                        bsp::TorpedoGoAwayRuntime& g = unit_.torpedo_goaway_runtime;

                        // --- 009D0F46, the geometry update, which runs first.
                        bsp::FlyToSolverInputs fin;
                        // arg1, approach->vtable[0](): the target point. The
                        // torpedo class's slot 0 body is still a contract, and
                        // the ordered target's world position stands in for it
                        // exactly as approach_target_point already does.
                        bool have_point = false;
                        {
                            float p[3] = {0.0f, 0.0f, 0.0f};
                            if (const GameUnitSlot* const t = goaway_target()) {
                                p[0] = t->motion.position[0];
                                p[1] = t->motion.position[1];
                                p[2] = t->motion.position[2];
                                have_point = true;
                            }
                            fin.point[0] = p[0];
                            fin.point[1] = p[1];
                            fin.point[2] = p[2];
                        }
                        for (int i = 0; i < 3; ++i) {
                            fin.unit_position[i] = unit_.motion.position[i];
                            // unit->vtable[34h] is 007BBB70, which copies out
                            // unit+AC8h..AD0h (all nine vtables that carry
                            // 0074E260 at slot 50h carry 007BBB70 at 34h). That
                            // vector has NO literal-address writer in .text -
                            // the only two disp32 references, 007BBB74 and
                            // 007C1B85, are both reads - so its identity is a
                            // HYPOTHESIS: this host feeds its own world
                            // velocity, which makes the solver's 3.0 multiplier
                            // a three-second lead. If the vector is not the
                            // velocity the break-off BEARING is wrong; nothing
                            // else in the tick depends on it.
                            fin.unit_lead_vector[i] = unit_.plane_world_velocity[i];
                        }
                        fin.standoff = g.break_off_distance_24;
                        fin.range = unit_.torpedo_approach.range_90;
                        // arg6 at this call site is the literal 0.8 at 00CE74F8.
                        fin.offset_scale = 0.8f;
                        // The cached obstacle list is a walk of GGame+19CCh's
                        // unit list, which this host does not expose. Empty is
                        // the answer for an open-water placement with no other
                        // unit inside its own extent plus 100 m of the lead
                        // point, and it is reported rather than assumed.
                        fin.obstacles = nullptr;
                        fin.obstacle_count = 0;
                        // 00681F40 / 009FA510 over GGame+711Ch..7130h: the world
                        // bounds are unmodelled, and USN01's aircraft are in
                        // open ocean rather than against a map edge.
                        fin.world_edge.near_edge = false;

                        const bsp::FlyToSolverResult fr =
                            bsp::fly_to_point_heading_009fd570(fin, g.side_2c);
                        g.side_2c = fr.side;

                        bsp::TorpedoGoAwayGeometryInputs geo;
                        geo.unit_heading_c6c = unit_.plane_heading_c6c;
                        geo.break_off_bearing = fr.heading;
                        // 007F0280's three out-slots stay at the zeros the
                        // caller writes at 009D0C96-009D0CAA, so the 009D0D50
                        // nudge cannot fire. docs/TORPEDO_AFTER_THE_DROP.md 3.5.
                        const float heading_18 =
                            bsp::torpedo_goaway_heading_009d0c10(geo);

                        // --- 009D0F4E onward, the tick.
                        bsp::TorpedoGoAwayTickInputs in;
                        in.unit_altitude = unit_.motion.position[1];
                        in.range_90 = unit_.torpedo_approach.range_90;
                        in.elapsed_134 = unit_.torpedo_approach.elapsed_134;
                        in.heading_18 = have_point ? heading_18 : g.heading_18;
                        // UniformFloatRange(3, 6) at 009D0FD7, low end.
                        in.window_jitter_draw =
                            bsp::torpedo_goaway_tick::kWindowJitterLo;
                        // The same TorpFlikFlakTime hole the enter reports.
                        in.window_delay_draw = 0.0f;

                        const bsp::TorpedoGoAwayTickResult r =
                            bsp::torpedo_goaway_tick_009d0f10(g, in, dt);
                        ++unit_.torpedo_goaway_ticks;
                        if (r.arm >= 1 && r.arm <= 4) {
                            ++unit_.torpedo_goaway_arm_ticks[r.arm];
                        }

                        // 009FB800(altitude, 1.0). The altitude command is the
                        // whole point of this binding, so it goes through the
                        // same helper the attack run uses rather than a
                        // shortcut: 009FB800 writes cmd+2BCh and cmd+2D0h = 2,
                        // and cmd+2D0h non-zero is what lets 0099E3BF run the
                        // pitch arm at all.
                        if (r.commands_altitude && r.altitude_known) {
                            bsp::PlanePitchCommandInputs pin;
                            pin.desired_altitude = r.altitude;
                            pin.reference = bsp::torpedo_goaway_tick::kPitchReference;
                            pin.unit_world_y = unit_.motion.position[1];
                            if (owner_.lua.plane_globals_loaded()) {
                                const bsp::GameTuningBlock& gt =
                                    owner_.lua.plane_globals();
                                pin.ceiling = gt.dynamics_ceiling;
                                pin.climb_dist = gt.pilot_general_climb_dist;
                                pin.drop_dist = gt.pilot_general_drop_dist;
                            }
                            pin.class_climb_angle = unit_.plane_climb_angle_1ec;
                            pin.class_drop_angle = unit_.plane_drop_angle;
                            unit_.plane_commanded_altitude = r.altitude;
                            unit_.plane_commanded_pitch =
                                bsp::pitch_command_009fb800(pin);
                            unit_.plan_state.pitch_target_2bc =
                                unit_.plane_commanded_pitch;
                            unit_.plan_state.pitch_mode_2d0 = 2;
                            unit_.torpedo_goaway_alt_cmd_last = r.altitude;
                        } else if (r.commands_altitude) {
                            // The +1Ch hole: the ordnance byte was clear at the
                            // enter and there is no squadron block, so the image
                            // would command [[approach+0Ch]+394h] and this host
                            // has no value. Commanding nothing is the honest
                            // answer; commanding zero would be a dive.
                            owner_.log.unimplemented(
                                "BotStateTorpedoGoAway::climb_altitude_1c",
                                "009d0e7f");
                        }

                        // cmd+2C0h / cmd+2CCh, the pair 0099D300's yaw arm reads.
                        if (r.commands_heading) {
                            unit_.plan_heading_2c0 = r.heading_2c0;
                            unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                            unit_.plan_heading_2c0_written = true;
                            ++unit_.torpedo_goaway_heading_ticks;
                        } else {
                            // Mode 1 leaves the heading alone and lets the roll
                            // target through, which is exactly what arms 2 and 4
                            // want. docs/PILOT_PLANNER_PITCH_ROLL.md.
                            unit_.plan_heading_mode_2cc = r.heading_mode_2cc;
                        }
                        if (r.commands_roll) {
                            unit_.plan_state.bank_target_2c4 = r.roll_2c4;
                        }
                        const float y = unit_.motion.position[1];
                        if (y < unit_.torpedo_goaway_alt_min) {
                            unit_.torpedo_goaway_alt_min = y;
                        }
                        if (y > unit_.torpedo_goaway_alt_max) {
                            unit_.torpedo_goaway_alt_max = y;
                        }
                    }

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
                        // approach+84h, the aspect scale 009D1FD0 loads. It is
                        // now seeded from the robots row at the profile seed
                        // above, as 009D049D does.
                        //
                        // The comment this replaces said it "only shapes the
                        // aim-solution byte 009D2021, never the gate". That was
                        // WRONG and is retracted: 009D1FED's result is
                        // multiplied by the release distance at 009D1FF2 and
                        // CACHED in [ESP+28h] at 009D1FF6, and that one cached
                        // product is read TWICE - at 009D1FFE for the +200.0
                        // aim-solution comparison, and again at 009D2034 for
                        // the release chain's own range flag, with no slack.
                        // The frame is stable across the intervening PUSH ECX /
                        // CALL 009FA3A0 because that callee ends RET 4
                        // (009FA40A). So this field gates the RELEASE.
                        // docs/TORPEDO_RELEASE_GATE.md section 1.1.
                        in.aspect_scale_84 = ap.aspect_scale_84;
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
                        // CORRECTED, packet cc8_torpedo_release_timer: both of
                        // these were the stand-in 1.0f, and both are inputs to
                        // the aim tick's DIVE command.
                        //
                        // 009D1E64 FLD [EAX+188h] / 009D1E6A FMUL denom, then
                        // 009D1E7E takes pitchDen = max(range - 1200, that
                        // product). Inside 1200 m of range the first term is
                        // negative, so the product IS the denominator: about
                        // 208 upward with MaxSpd, 3.0 upward with 1.0. With 1.0
                        // the command saturates at -DEG(80) for any height
                        // above the release floor over about four metres, and
                        // the aircraft goes vertical inside 1200 m.
                        in.pitch_scale_188 = unit_.plane_max_spd;
                        // 009D1E39 FDIV [EDX+1ACh], the divisor of the
                        // nose-down damper denom = 3 + (-unit+C64h / PitchSpd)
                        // * sel, which is what shallows the command as the dive
                        // steepens. 009D1DFD JBE is taken when -unit+C64h <= 0,
                        // so it runs only while the nose is already down.
                        in.pitch_div_1ac = unit_.plane_pitch_spd;
                        in.state_flag_24 = false;
                        const int arms_before = unit_.torpedo_aim_release_arms;
                        const bsp::TorpedoAimTickResult r =
                            bsp::torpedo_aim_tick_full_009d15f0(binding, in, dt);
                        ++unit_.torpedo_aim_ticks;
                        // Packet cc9_torpedo_kind_land: one line per release, on
                        // the tick whose five-flag chain armed the timer at
                        // 009D2287 (the drop follows on the next tick). It sets
                        // the range the tick compared (approach+90h, scaled by
                        // the 0.9 tighten into F14) beside the live centre-to-
                        // centre distance, so a moving target's release range
                        // can be read off rather than modelled.
                        // docs/TORPEDO_KIND_PROBE.md section 8.
                        // The aim tick at which each flag of the chain last turned
                        // true, so the line can name the one that opened last.
                        // Kept per slot in a function-local table because the
                        // slot struct belongs to other packets; observation only.
                        static std::map<const GameUnitSlot*, std::array<int, 4>> opened;
                        {
                            auto it = opened.try_emplace(&unit_,
                                std::array<int, 4>{-1, -1, -1, -1}).first;
                            const bool now[4] = {r.gate_lead, r.gate_altitude,
                                                 r.gate_bank, r.gate_cone};
                            for (int g = 0; g < 4; ++g) {
                                if (!now[g]) it->second[static_cast<std::size_t>(g)] = -1;
                                else if (it->second[static_cast<std::size_t>(g)] < 0)
                                    it->second[static_cast<std::size_t>(g)] =
                                        unit_.torpedo_aim_ticks;
                            }
                        }
                        if (unit_.torpedo_aim_release_arms != arms_before) {
                            const std::array<int, 4>& op = opened[&unit_];
                            const GameUnitSlot* const t = binding.aim_target();
                            float centre = -1.0f;
                            float target_speed = 0.0f;
                            if (t != nullptr) {
                                const double dx = static_cast<double>(t->motion.position[0]) -
                                                  unit_.motion.position[0];
                                const double dz = static_cast<double>(t->motion.position[2]) -
                                                  unit_.motion.position[2];
                                centre = static_cast<float>(std::sqrt(dx * dx + dz * dz));
                                const double vx = t->motion.linear_velocity.x;
                                const double vz = t->motion.linear_velocity.z;
                                target_speed = static_cast<float>(std::sqrt(vx * vx + vz * vz));
                            }
                            owner_.log.notef("  torpedo %-12s timer arm 009D2287 aim_tick=%d "
                                "target=%s range_90=%.1f centre_dist=%.1f F14=%.1f F0C=%.4f "
                                "cmd_speed=%.1f fall_lead_a0=%.1f alt=%.1f target_speed=%.2f "
                                "gates lead=%d alt=%d bank=%d cone=%d "
                                "opened_at lead=%d alt=%d bank=%d cone=%d",
                                unit_.row.name.c_str(), unit_.torpedo_aim_ticks,
                                t != nullptr ? t->row.name.c_str() : "-",
                                static_cast<double>(ap.range_90), static_cast<double>(centre),
                                static_cast<double>(r.range_f14),
                                static_cast<double>(r.time_to_target_f0c),
                                static_cast<double>(bsp::torpedo_commanded_speed_009d3c99(
                                    ap.elapsed_134, ap.speed_late_7c, ap.speed_early_80)),
                                static_cast<double>(ap.fall_lead_a0),
                                static_cast<double>(unit_.motion.position[1]),
                                static_cast<double>(target_speed),
                                r.gate_lead ? 1 : 0, r.gate_altitude ? 1 : 0,
                                r.gate_bank ? 1 : 0, r.gate_cone ? 1 : 0,
                                op[0], op[1], op[2], op[3]);
                        }
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
                        // 0099E3D1 gates the whole pitch law on.
                        //
                        // CORRECTED, packet cc8_torpedo_release_timer. The value
                        // is clamp(-f34 / den, -1.3962634, 0.872665), i.e.
                        // [-DEG(80), +DEG(50)]: 00D21318 holds the FLOAT
                        // 0xBFB2B8C3 = -1.3962634, and the 0.05625 this comment
                        // used to cite is the DOUBLE at those same eight bytes,
                        // which neither of the two four-byte loads (009D1EA6
                        // FLD float ptr, 009D1EB0 MOVSS) reads. f34 is the
                        // aircraft's height ABOVE the altitude floor and
                        // 009D1E98 FCHS negates it, so this IS the descent
                        // command: the aim state's own dive, from the in-range
                        // latch down to the 25-to-40 metre release band at
                        // 009D20C4, easing to zero as the height above the
                        // floor closes. docs/TORPEDO_RELEASE_TIMER.md.
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
                            // Packet cc9_plane_gunfire: the rest of 0099B450's tail
                            // (0099B4E8-0099B580) for the fields this host keeps on
                            // the slot. +2B4h = [plan+2F4h]+190h (0099B503/0099B511),
                            // which 007D2406 stores as TravelSpeed * tuning+334h
                            // NewTravelSpeedMul; +2B0h = 0 (0099B53C, byte); +2D8h =
                            // 1 (0099B542, dword). Without it a state's speed write
                            // outlived the state (run F1, docs/DOGFIGHT_GUN.md 6).
                            if constexpr (GameUnitsHost::Impl::kPilotPlanReseedBound) {
                                float mul = 1.6f;
                                if (owner_.lua.plane_globals_loaded()) {
                                    mul = owner_.lua.plane_globals()
                                        .dynamics_spd_multipliers_new_travel_speed_mul;
                                }
                                unit_.plane_desired_speed_2b4 = unit_.plane_travel_speed * mul;
                                unit_.plane_trg_speed_corr_off_2b0 = 0;
                                unit_.plane_air_brake_mode_2d8 = 1;
                            }
                            // cmd+2CCh is ONE word in the image and two fields
                            // here: PilotPlanState::heading_mode_2cc, which the
                            // reset above sets to 1 for 0099B548, and
                            // plan_heading_mode_2cc, which the roll gate at
                            // 0099DE8A/0099E275 actually reads and every task
                            // tick writes. Without this copy the reset never
                            // reached the gate, so where the image re-arms the
                            // SERVO arm every think - holding whatever bank
                            // target cmd+2C4h carries - this host left the field
                            // wherever the last writer put it, and the planner's
                            // own mode-2 arm zeroes it at 0099E3B5. A state that
                            // wrote no roll mode then got neither arm and simply
                            // stopped banking. docs/DIVE_BOMB_TASK.md.
                            unit_.plan_heading_mode_2cc =
                                unit_.plan_state.heading_mode_2cc;

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
                            run_dive_bomb_task_arm_009c8790(elapsed);
                            if constexpr (GameUnitsHost::Impl::kDogfightTaskBound) {
                                run_dogfight_task_arm_009ab1c0(elapsed);
                            }

                            unit_.plane_think_dt = elapsed;   // 0099D300's argument
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
                        if constexpr (GameUnitsHost::Impl::kPlaneCommitCommandBound) {
                            // 007BB923: unit+61h has no writer (GameUnitsHost::
                            // unit_flag_0061), so the gate is open.
                            // 007BB932-007BB954: outside flight states 7/6/4/5 the
                            // air brake command is forced to 1.0 (00D7A24C).
                            const int m = unit_.plane_control_mode_900;
                            if (m != 7 && m != 6 && m != 4 && m != 5) {
                                unit_.pilot_command_block[bsp::kPilotCmdAirBrake] = 1.0f;
                                ++unit_.pc_air_brake_overrides;
                            }
                            // 007BB95C-007BB97A: IsKindOf(17h) with PilotFires
                            // (unit+C24h) clear forces throttle 1.0. PilotFires is
                            // authored per weapon group and not loaded here, so a
                            // kamikaze-capable plane's override stays a contract.
                            if (bsp::unit_is_kind_of(unit_.class_id, 0x17)) {
                                if constexpr (GameUnitsHost::Impl::kPilotFiresBound) {
                                    if (!unit_.plane_pilot_fires_c24) {   // 007BB969
                                        unit_.pilot_command_block[bsp::kPilotCmdThrottle] = 1.0f;
                                    }
                                } else {
                                    owner_.log.unimplemented("Plane::pilot_fires_c24", "007bb969");
                                }
                            }
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
                        if constexpr (GameUnitsHost::Impl::kPlaneCommitCommandBound) {
                            owner_.done("Plane::commit_pilot_command", 0x007bb920u);
                        } else {
                            owner_.record("Plane::commit_pilot_command", 0x007bb920u);
                        }
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
                        // The bank target write at 0099E25C is INSIDE the
                        // region the same jump skips: 0099E25C < 0099E264 <
                        // 0099E26E, so a task that reaches 0099E26E by
                        // 0099DE8D keeps its own cmd+2C4h as well as its mode.
                        // THE GATE, 0099DDBE-0099E275. 0099DDBE loads the mode
                        // word plan+2CCh the task wrote; 0099DE8A CMP ECX,2 and
                        // 0099DE8D JNZ send anything but 2 to 0099E26E, PAST the
                        // planner's own MOV [ESI+2CCh],1 at 0099E264; 0099E26E
                        // CMP [ESI+2CCh],1 and 0099E275 JNZ then skip the whole
                        // roll arm, so the task's plan+290h survives. Mode 2, a
                        // commanded heading, falls through and the planner
                        // writes the bank itself. The turndown 009C44F0 writes
                        // mode 0, which is exactly the value that passes.
                        // docs/DIVE_BOMB_TASK.md, "The roll-arm gate".
                        const int roll_mode = unit_.plan_heading_mode_2cc;
                        if (roll_mode == 2) {
                            // The mode-2 arm: the planner computes the target and
                            // writes it at 0099E25C, then runs its own law.
                            unit_.plan_state.bank_target_2c4 = roll.bank_target;
                            unit_.plan_slots[bsp::kPilotSlotRoll].desired = roll.desired;
                            unit_.plan_slots[bsp::kPilotSlotRoll].active = 1;  // 0099E3AE
                            unit_.plan_heading_mode_2cc = 0;  // 0099E3B5
                        } else if (roll_mode == 1) {
                            // The mode-1 arm, 0099E26E-0099E39D. The jump at
                            // 0099DE8D skipped both 0099E25C and 0099E264, so
                            // plan+2C4h still holds what the TASK wrote and this
                            // arm servos toward it rather than computing one.
                            // The dive-bomb turndown writes pi there at 009C4646.
                            bsp::PilotBotRollDemandInputs sin;
                            sin.bank_target_2c4 = unit_.plan_state.bank_target_2c4;
                            sin.bank = unit_.plane_bank_angle_c68;
                            // SUBSTITUTION, labelled and now one item shorter:
                            // [ESP+28h] and the EBX tuning block are contracts,
                            // so the scale is 1, the band is open and the rate
                            // is zero. The map's interpolant is no longer among
                            // them - it is the demand the first half returns,
                            // which is what the frame slot at 0099E36B holds.
                            sin.error_scale = 1.0f;
                            sin.band_40 = 0.0f;
                            sin.gain_44 = 1.0f;
                            sin.rate_48 = 0.0f;
                            const bsp::PilotBotRollDemandResult sr =
                                bsp::pilot_roll_bank_demand_0099e2ba(sin);
                            // 0099E344-0099E367's limit is a contract: desc+1A8h,
                            // desc+1BCh and EBX[0] have no producer read here.
                            unit_.plan_slots[bsp::kPilotSlotRoll].desired =
                                bsp::pilot_roll_rate_limited_0099e344(sr.demand, 1.0f);
                            unit_.plan_slots[bsp::kPilotSlotRoll].active = 1;  // 0099E3AE
                            unit_.plan_heading_mode_2cc = 0;  // 0099E3B5
                        }

                        // THE PITCH-MODE GATE, 0099E3BF-0099E3D1. `MOV ECX,
                        // [ESI+2D0h]` / `TEST ECX,ECX` / `JNE 0099E490`: the
                        // planner's pitch arm runs only when the pitch mode the
                        // task left behind is NON-zero. Mode 0 takes the branch
                        // 0099E3D7-0099E483, which touches nothing but the
                        // command's +2ECh stamp and then jumps to 0099E756,
                        // PAST the arm - so the task's own plan+29Ch/+2A0h
                        // survives the think.
                        //
                        // This is the pitch twin of the roll gate on +2CCh above,
                        // and it is what the dive-bomb turndown needs: 009C469C
                        // and 009C472A both write +2D0h = 0 (EBP, zeroed at
                        // 009C44FB) beside the pitch command, so the full nose-
                        // down deflection it asks for is meant to reach the
                        // elevator unaltered. Without the gate the arm below
                        // overwrote it every think and the dive never steepened.
                        // docs/DIVE_BOMB_TASK.md, "The pitch-arm gate".
                        //
                        // 0099E3D7-0099E483 itself is NOT modelled: its only
                        // writes are cmd+2ECh from 00E0E2F4/00E0E2F0, and this
                        // host keeps no +2ECh.
                        if (unit_.plan_state.pitch_mode_2d0 != 0) {
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
                        }   // end of the +2D0h != 0 arm, 0099E490-0099E739
                        // 0099D300's throttle arms. The demand arm is reachable
                        // in flight through 0099D8CD, which jumps past the
                        // flight-state test at 0099D8FD.
                        // docs/PILOT_THROTTLE_CUT_RAISER.md.
                        {
                            bsp::PilotBotThrottleInputs tin;
                            const bsp::PilotPlanSlot& th =
                                unit_.plan_slots[bsp::kPilotSlotThrottle];
                            tin.slot_current = th.current;
                            tin.slot_desired = th.desired;
                            tin.slot_active = th.active != 0;
                            tin.flight_state = unit_.plane_control_mode_900;
                            tin.air_brake_mode = unit_.plane_air_brake_mode_2d8;
                            tin.one_shot_threshold = unit_.plane_desired_speed_2b4;
                            tin.measured_speed = std::sqrt(
                                unit_.plane_world_velocity[0] * unit_.plane_world_velocity[0] +
                                unit_.plane_world_velocity[1] * unit_.plane_world_velocity[1] +
                                unit_.plane_world_velocity[2] * unit_.plane_world_velocity[2]);
                            // plan+2B8h. Its producer, 0099D756-0099D79A and
                            // 0099D970, is unread; 1.0 leaves the error in m/s,
                            // which is the unit the interpolation's +-6.9444
                            // endpoints are in. SUBSTITUTION, labelled.
                            tin.speed_scale = 1.0f;
                            tin.desired_speed = unit_.plane_desired_speed_2b4;
                            const float pend = th.active != 0
                                ? th.desired - th.current : 0.0f;
                            tin.pending = pend < 0.0f ? -pend : pend;
                            if constexpr (GameUnitsHost::Impl::kPilotThrottleSlotBound) {
                                // Packet cc9_throttle_slot. 0099DBBF multiplies the
                                // demand increment by the argument slot [frame+4].
                                // With +2D8h != 0, 0099D79F jumps to 0099D8C1 and
                                // that slot still holds 0099D4EE's dt / dtScale,
                                // dtScale = max(unit+340h (CheatTurbo) * 0.4, 1.0)
                                // (00CE65D0), which is 1.0 here. Only mode 0
                                // reaches 0099D7CB-0099D87E, where the slot becomes
                                // max(|throttle pending|, |air-brake pending|).
                                if (unit_.plane_air_brake_mode_2d8 != 0) {
                                    tin.pending = unit_.plane_think_dt;
                                } else {
                                    const bsp::PilotPlanSlot& ab =
                                        unit_.plan_slots[bsp::kPilotSlotAirBrake];
                                    float pb = ab.active != 0 ? ab.desired - ab.current : 0.0f;
                                    if (pb < 0.0f) pb = -pb;
                                    if (pb > tin.pending) tin.pending = pb;
                                }
                            }
                            // Both inputs of 0099DAC8's correction - unit+B1Ch
                            // and the vector at unit+AE0h - have no displacement
                            // writer anywhere in the image, so the term is taken
                            // as zero. SUBSTITUTION, labelled.
                            tin.error_correction = 0.0f;
                            tin.dead_band_skips = false;
                            const bsp::PilotBotThrottleResult tr =
                                bsp::pilot_plan_throttle_0099d300(tin);
                            if constexpr (GameUnitsHost::Impl::kThrottleSlotTrace) {
                                // Packet cc9_throttle_slot: one line per think for
                                // the Yorktown flight while its dogfight task runs.
                                if (unit_.dogfight_task_installed &&
                                    unit_.row.name.rfind("Yorktown-class01_sqn02", 0) == 0) {
                                    owner_.log.notef("throttle_trace %s st=%s mode=%d want=%.2f "
                                        "slot=(cur %.3f des %.3f act %d) pend=%.3f out=%.3f wrote=%d v=%.2f y=%.1f",
                                        unit_.row.name.c_str(),
                                        bsp::dogfight_state_name(unit_.dogfight_state),
                                        unit_.plane_air_brake_mode_2d8,
                                        static_cast<double>(unit_.plane_desired_speed_2b4),
                                        static_cast<double>(th.current), static_cast<double>(th.desired),
                                        th.active ? 1 : 0, static_cast<double>(tin.pending),
                                        static_cast<double>(tr.throttle_desired), tr.wrote_throttle ? 1 : 0,
                                        static_cast<double>(tin.measured_speed),
                                        static_cast<double>(unit_.motion.position[1]));
                                }
                            }
                            if constexpr (GameUnitsHost::Impl::kThrottleSlotTrace) {
                                if (unit_.dive_bomb_task_installed &&
                                    unit_.row.name == "D3A Val #1.1") {
                                    owner_.log.notef("val_trace st=%d mode=%d want=%.2f "
                                        "slot=(cur %.3f des %.3f act %d) pend=%.3f out=%.3f wrote=%d v=%.2f y=%.1f",
                                        static_cast<int>(unit_.dive_bomb_state),
                                        unit_.plane_air_brake_mode_2d8,
                                        static_cast<double>(unit_.plane_desired_speed_2b4),
                                        static_cast<double>(th.current), static_cast<double>(th.desired),
                                        th.active ? 1 : 0, static_cast<double>(tin.pending),
                                        static_cast<double>(tr.throttle_desired), tr.wrote_throttle ? 1 : 0,
                                        static_cast<double>(tin.measured_speed),
                                        static_cast<double>(unit_.motion.position[1]));
                                }
                            }
                            if (tr.wrote_throttle) {
                                unit_.plan_slots[bsp::kPilotSlotThrottle].desired =
                                    tr.throttle_desired;
                                unit_.plan_slots[bsp::kPilotSlotThrottle].active = 1;
                                unit_.plane_throttle_last = tr.throttle_desired;
                            }
                            if (tr.wrote_air_brake) {
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].desired =
                                    tr.air_brake_desired;
                                unit_.plan_slots[bsp::kPilotSlotAirBrake].active = 1;
                            }
                            if (tr.clears_air_brake_mode) {
                                unit_.plane_air_brake_mode_2d8 = 0;   // 0099DC6B
                            }
                        }
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
                        if constexpr (GameUnitsHost::Impl::kPlaneControlRateLawBound) {
                            // The free-flight arm (controller mode 0, flag 1) is
                            // read in full: docs/PLANE_CONTROL_RATE_LAW.md and
                            // docs/PLANE_CONTROL_TARGETS.md. Only the ground arm
                            // (007DA542) is unread, and this host never runs it.
                            owner_.done("PlaneFlight::control_rate_law", 0x007da710u);
                        } else {
                            owner_.record("PlaneFlight::control_rate_law", 0x007da710u);
                        }
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
        // 00826CEE, the motion TAIL: 00810190(unit+0BD0h, &unit+0FCh, heading,
        // yaw_rate). It runs after the pose is written, which is why it is here
        // and not inside the step. The heading is unit+1050h, what vtable slot
        // 50h returns, written at 00826C56 by an atan2 over world row 2 - the
        // same expression `heading_degrees_of` above already uses, in radians.
        // Packet cc8_ship_follow, docs/SHIP_UNIT_GROUP_FOLLOW.md section 5b.
        //
        // PROVISIONAL, and named as such: the yaw rate is 00826C75's, and this
        // packet did not re-read that site. `rate_row1` is the host's row-1
        // steering rate after the slew limiter, the nearest value it holds. The
        // trail's GEOMETRY does not depend on it - only 0070D290's `out[4]`,
        // which 009DF2D0 uses for a follower's speed blend, ever reads it back.
        {
            const float wake_heading = static_cast<float>(
                std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
                           static_cast<double>(slot.motion.pose_row2[2])));
            bsp::ship_ai_wake_append_00810190(slot.wake, slot.motion.position,
                wake_heading, result.steering.rate_row1);
            host.done("UnitWake::append_sample", 0x00810190u);
        }
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

void GameUnitsHost::store_unit_attack_command_class(std::size_t index,
                                                    unsigned int cls) noexcept {
    if (index >= impl_->slots.size()) return;
    impl_->slots[index]->attack_command_class = cls;
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

// ---- packet cc8_ship_follow: the unit group at entity+284h -----------------

std::int32_t GameUnitsHost::unit_formation_group_0284(std::size_t index) const noexcept {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return -1;
    return host.slots[index]->formation_group;
}

std::size_t GameUnitsHost::formation_leader_0014(std::int32_t group) const noexcept {
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return static_cast<std::size_t>(-1);
    }
    return host.formation_groups[static_cast<std::size_t>(group)].leader;
}

bool GameUnitsHost::unit_is_formation_follower_007788b0(std::size_t index) const noexcept {
    // 007788B0 whole: g = [unit+284h]; g && [g+14h] != unit.
    const std::int32_t group = unit_formation_group_0284(index);
    if (group < 0) return false;
    return formation_leader_0014(group) != index;
}

std::int32_t GameUnitsHost::formation_member_count(std::int32_t group) const noexcept {
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return 0;
    }
    return static_cast<std::int32_t>(
        host.formation_groups[static_cast<std::size_t>(group)].members.size());
}

std::size_t GameUnitsHost::formation_member_unit(std::int32_t group,
                                                std::int32_t slot) const noexcept {
    // Packet cc9_ship_formation_speed. The record's +00h is the one-based handle
    // the join stores (record.entity = unit + 1).
    const Impl& host = *impl_;
    if (group < 0 || static_cast<std::size_t>(group) >= host.formation_groups.size()) {
        return static_cast<std::size_t>(-1);
    }
    const auto& members = host.formation_groups[static_cast<std::size_t>(group)].members;
    if (slot < 0 || static_cast<std::size_t>(slot) >= members.size()) {
        return static_cast<std::size_t>(-1);
    }
    const std::uint32_t entity = members[static_cast<std::size_t>(slot)].entity;
    return entity == 0u ? static_cast<std::size_t>(-1) : static_cast<std::size_t>(entity - 1u);
}

GameUnitsHost::FormationStation GameUnitsHost::formation_station_0070d290(
    std::size_t unit, float across_scale, float along_scale) const noexcept {
    FormationStation out{};
    const Impl& host = *impl_;
    if (unit >= host.slots.size()) return out;
    const std::int32_t group_index = host.slots[unit]->formation_group;

    // 0070D2A1 resolves the member record through 0070D080 and 0070D2A6/0070D2AF
    // take the leader branch when the unit IS the leader or has no record.
    const bsp::ShipAiUnitGroupMember* record = nullptr;
    std::size_t leader = unit;
    if (group_index >= 0
        && static_cast<std::size_t>(group_index) < host.formation_groups.size()) {
        const Impl::FormationGroup& group =
            host.formation_groups[static_cast<std::size_t>(group_index)];
        leader = group.leader;
        if (leader != unit) {
            const std::uint32_t handle = static_cast<std::uint32_t>(unit + 1u);
            for (const bsp::ShipAiUnitGroupMember& member : group.members) {
                if (member.entity == handle) { record = &member; break; }
            }
        }
        if (record != nullptr) {
            const std::size_t column = static_cast<std::size_t>(group.column);
            if (column < bsp::kShipAiUnitGroupColumnCount) {
                out.across = record->lateral[column] * across_scale;   // 0070D2BD
                out.along = record->axial[column] * along_scale;       // 0070D2CC
            }
        }
    }

    if (record != nullptr && leader < host.slots.size()) {
        // 0070D2EE: 00811150(leader, along) -> the wake point and its direction,
        // then out[0] = pos.x - across * dir.z and out[1] = pos.z + across * dir.x
        // (0070D342, 0070D348), the left normal this packet also decomposes on.
        const bsp::ShipAiWakePoint base = bsp::ship_ai_wake_sample_at_distance_00810630(
            host.slots[leader]->wake, out.along);
        out.x = base.x - out.across * base.dir_z;
        out.z = base.z + out.across * base.dir_x;
        out.dir_x = base.dir_x;
        out.dir_z = base.dir_z;
        out.wake_yaw_rate = base.yaw_rate;
        out.wake_yaw_written = base.yaw_rate_written;
        out.valid = true;
        return out;
    }

    // 0070D362: the unit's own position and (cos, sin) of wrap_2pi(pi/2 - heading).
    out.is_leader_branch = true;
    const bsp::ShipMotionState& motion = host.slots[unit]->motion;
    out.x = motion.position[0];
    out.z = motion.position[2];
    const float heading = static_cast<float>(
        std::atan2(static_cast<double>(motion.pose_row2[0]),
                   static_cast<double>(motion.pose_row2[2])));
    float angle = 1.5707963705062866f - heading;                  // 00CE3830
    if (angle < 0.0f) angle += 6.2831854820251465f;               // 00CE3828
    out.dir_x = std::cos(angle);
    out.dir_z = std::sin(angle);
    out.across = 0.0f;                                            // 0070D3E1
    out.along = 0.0f;                                             // 0070D3E9
    out.wake_yaw_written = false;                                 // 0070D3F1 writes 0
    out.valid = true;
    return out;
}

bool GameUnitsHost::formation_join_0077f940(std::size_t follower, std::size_t leader) {
    Impl& host = *impl_;
    if (follower >= host.slots.size() || leader >= host.slots.size()) return false;
    if (follower == leader) return false;                 // 00779824, the identity arm

    // Packet cc8_ship_station: 0077F940's merge arm, read whole from the listing
    // (126 instructions, body 0077F940-0077FAC8, RET 4). USN01 reaches it - the
    // script orders Northampton, which already leads SaltLakeCity and Dunlap, to
    // join Enterprise - so the arm the previous packet left out is the one that
    // decides what those three ships do.
    //
    //   0077F96E  nothing happens at all when the ordered ship's group is the
    //             target's group: `iVar3 == 0 || iVar3 != iVar4`.
    //   0077F97x  `brought` is 1, or the ordered ship's own member count
    //             (group+4F8h) when it LEADS that group - it brings its whole
    //             formation with it.
    //   0077F9B4  the cap: (float)(target_count + brought) <= settings+420h,
    //             where target_count is 1 when the target has no group. On a
    //             refusal nothing at all happens - no create, no detach, no add.
    //   0077F9D9  0070DB20 creates a group around the target when it has none;
    //             0077F9E6 otherwise redirects the target to its group's leader.
    //   0077FA10  the list: MemberAt(i) over the ordered ship's group, the
    //             ordered ship itself at index 0 and every other member after it.
    //   0077FA40  each of those followers leaves through 0077BD70 with the
    //             ordered ship as the argument, and 0077FA51 the ordered ship
    //             itself leaves with a null one.
    //   0077FA81  0070EF30 adds each brought unit to the target group in that
    //             order, ECX = the group, so each one's column is measured
    //             against the NEW leader's wake at its own position.
    //
    // NOT modelled, and named: 0077BD70's own body (0070D0C0 SetLeader, 0070D8D0,
    // 0070E4C0 DetachMember) is unread, so a detach here empties the record and
    // clears entity+284h without promoting a new leader for the group left
    // behind; and the vtable[114h] / vtable[58h] follow-up at 0077FA8D is
    // unread, as the previous packet already recorded. The create/redirect is
    // also left where it already stood, after the detach rather than before it,
    // which can only differ when the two groups are the same - and that case
    // returns above.
    const std::int32_t ordered_group_before = host.slots[follower]->formation_group;
    const std::int32_t target_group_before = host.slots[leader]->formation_group;
    if (ordered_group_before >= 0 && ordered_group_before == target_group_before) {
        ++host.formation_rejoins;
        return false;                                      // 0077F96E
    }

    // 0077FA10's list, built BEFORE any detach. Only a unit that leads its group
    // brings anything; a follower ordered away brings only itself.
    std::vector<std::size_t> brought;
    brought.push_back(follower);
    if (ordered_group_before >= 0
        && host.formation_groups[static_cast<std::size_t>(ordered_group_before)].leader
               == follower) {
        for (const bsp::ShipAiUnitGroupMember& member :
             host.formation_groups[static_cast<std::size_t>(ordered_group_before)].members) {
            const std::size_t unit = static_cast<std::size_t>(member.entity) - 1u;
            if (unit != follower && unit < host.slots.size()) brought.push_back(unit);
        }
    }

    // 0077F9B4. FormationMaxCount is settings+420h; 24 is cited from
    // docs/SHIP_AI_FORMATION.md rather than re-read here.
    constexpr float kFormationMaxCount = 24.0f;
    const std::size_t target_count = (target_group_before >= 0)
        ? host.formation_groups[static_cast<std::size_t>(target_group_before)].members.size()
        : static_cast<std::size_t>(1);
    if (static_cast<float>(target_count + brought.size()) > kFormationMaxCount) {
        host.log.notef("formation join refused: ordered=%s target=%s brought=%zu "
            "target_count=%zu cap=%.0f (0077F9B4)",
            host.slots[follower]->row.name.c_str(), host.slots[leader]->row.name.c_str(),
            brought.size(), target_count, static_cast<double>(kFormationMaxCount));
        return false;
    }

    // 0077FA40 / 0077FA51: every brought unit leaves its old group first.
    for (const std::size_t unit : brought) {
        const std::int32_t old_group = host.slots[unit]->formation_group;
        if (old_group < 0) continue;
        std::vector<bsp::ShipAiUnitGroupMember>& members =
            host.formation_groups[static_cast<std::size_t>(old_group)].members;
        const std::uint32_t old_handle = static_cast<std::uint32_t>(unit + 1u);
        std::vector<bsp::ShipAiUnitGroupMember> kept;
        kept.reserve(members.size());
        for (const bsp::ShipAiUnitGroupMember& member : members) {
            if (member.entity != old_handle) kept.push_back(member);
        }
        members.swap(kept);
        host.slots[unit]->formation_group = -1;
    }

    std::int32_t group = host.slots[leader]->formation_group;
    if (group < 0) {
        // 0070DB20: the leader becomes member 0 with an all-zero relative
        // position, the count is 1, and shape stays 0 so column 0 is live.
        group = static_cast<std::int32_t>(host.formation_groups.size());
        host.formation_groups.emplace_back();
        Impl::FormationGroup& created = host.formation_groups.back();
        created.leader = leader;
        bsp::ShipAiUnitGroupMember record;
        record.entity = static_cast<std::uint32_t>(leader + 1u);
        created.members.push_back(record);
        host.slots[leader]->formation_group = group;       // leader+284h = group
        ++host.formation_creates;
    } else {
        // "otherwise redirects to other's leader": the group the leader already
        // belongs to is the one joined, whoever it is led by.
        group = host.slots[leader]->formation_group;
    }

    Impl::FormationGroup& target = host.formation_groups[static_cast<std::size_t>(group)];
    const std::uint32_t handle = static_cast<std::uint32_t>(follower + 1u);
    for (const bsp::ShipAiUnitGroupMember& member : target.members) {
        if (member.entity == handle) {
            // 0070EF30 with an entity that is already a member: only the observer
            // pair is added, no record is appended and the count does not move.
            ++host.formation_rejoins;
            return false;
        }
    }
    // 0070EF38 writes entity+284h before any test.
    host.slots[follower]->formation_group = group;
    bsp::ShipAiUnitGroupMember record;
    record.entity = handle;
    record.field_30 = 999u;                                // 0070EF85's record+30h

    // 0070ED30, the column-0 arm, run at the join index before the count moves.
    // The image expresses the member's position in the leader's frame, clamps
    // that to FollowerMaxDist, transforms it back to world and decomposes it
    // against the leader's wake. A rigid transform preserves length, so clamping
    // the world offset is the same clamp; the leader-frame round trip is not
    // modelled because column 0 never reads record+4h.
    //
    // FollowerMaxDist is settings+424h. 4000.0 is this installation's authored
    // value, from scripts/datatables/shipglobals.lua (mtime 2024-07-13, the
    // untouched bulk date, so not one of this install's modified tables). The
    // host parses the same key into GameplaySettings::formacio_follower_max_dist
    // but nothing reaches it from here, so the constant is named rather than
    // plumbed. It does not bind in either measured mission: USN01's widest
    // follower sits about 1460 m from the Enterprise.
    constexpr float kFormationFollowerMaxDist = 4000.0f;
    const float* const leader_pos = host.slots[leader]->motion.position;
    const float* const member_pos = host.slots[follower]->motion.position;
    float offset[3] = {member_pos[0] - leader_pos[0], member_pos[1] - leader_pos[1],
                       member_pos[2] - leader_pos[2]};
    const float offset_len2 = offset[0] * offset[0] + offset[1] * offset[1]
                            + offset[2] * offset[2];
    if (offset_len2 > kFormationFollowerMaxDist * kFormationFollowerMaxDist) {
        const float scale = kFormationFollowerMaxDist / std::sqrt(offset_len2);
        offset[0] *= scale;
        offset[1] *= scale;
        offset[2] *= scale;
        ++host.formation_clamped;
    }
    const float world[3] = {leader_pos[0] + offset[0], leader_pos[1] + offset[1],
                            leader_pos[2] + offset[2]};
    const bsp::ShipAiWakeDecomposition decomposition =
        bsp::ship_ai_wake_decompose_00811180(host.slots[leader]->wake, world);
    if (decomposition.valid) {
        record.lateral[0] = decomposition.across;           // record+10h
        record.axial[0] = decomposition.along;              // record+20h
    } else {
        // RETRACTED (packet cc8_ship_station): this used to say "the image
        // would decompose against its zeroed ring". The image's ring is never
        // zeroed - 00822C20 fills it with forty synthetic legs at the spawn
        // pose through 00818EA0 -> 00810020, which this host now runs at
        // creation. The counter should read 0 from here on; a non-zero value
        // means a unit reached a join without its ring filled.
        ++host.formation_columns_unmeasurable;
    }
    // Packet cc8_ship_station. One line per join - thirteen on USN01 - because
    // the first run with 00810020 bound moved `columns_unmeasurable` 10 -> 0 and
    // left every follower's path and station error bit-identical, which the
    // numbers below are there to explain. The head sample is the leader's ring
    // head, which the fill puts at its spawn pose.
    {
        const bsp::ShipAiWakeSample& head =
            host.slots[leader]->wake.samples[host.slots[leader]->wake.head];
        host.log.notef("formation column: follower=%s leader=%s group=%d "
            "member=(%.1f %.1f) leader_pos=(%.1f %.1f) clamped_point=(%.1f %.1f) "
            "wake{written=%u head=%d sample=(%.1f %.1f) seg=%.1f} "
            "across=%.2f along=%.2f valid=%d",
            host.slots[follower]->row.name.c_str(),
            host.slots[leader]->row.name.c_str(),
            group,
            static_cast<double>(member_pos[0]), static_cast<double>(member_pos[2]),
            static_cast<double>(leader_pos[0]), static_cast<double>(leader_pos[2]),
            static_cast<double>(world[0]), static_cast<double>(world[2]),
            host.slots[leader]->wake.written, host.slots[leader]->wake.head,
            static_cast<double>(head.x), static_cast<double>(head.z),
            static_cast<double>(head.segment),
            static_cast<double>(decomposition.across),
            static_cast<double>(decomposition.along),
            decomposition.valid ? 1 : 0);
    }
    target.members.push_back(record);
    ++host.formation_joins;
    // 007788B0 / 007788D0 are read by 00836920's idle re-issue, which lives in
    // the commands host and has no route to the group, so the pair is pushed.
    host.commands.set_unit_formation(follower, true, leader);
    // 0070DA00's speed ceiling over the new membership is NOT run: nothing reads
    // group+504h yet. Columns 1, 2 and 3 are not produced either - they are the
    // canned LINE / COLUMN / DIAMOND tables, which only a type-78h reshape
    // selects and which this packet's across convention cannot be trusted for.

    // 0077FA81's loop over the rest of the list. Each one has already been
    // detached above, so it leads nothing and brings only itself: the recursion
    // is one level deep and terminates. The leader taken is the target group's
    // own leader, which is 0077F9E6's redirect.
    const std::size_t target_leader =
        host.formation_groups[static_cast<std::size_t>(group)].leader;
    for (std::size_t i = 1; i < brought.size(); ++i) {
        formation_join_0077f940(brought[i], target_leader);
    }
    return true;
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

// Packet cc9_plane_gun_pass: unit+BC9h, the latched gunFire the plane's fixed
// step hands to each enabled gun's SetTriggerHeld (007CE9A0-007CE9F4).
bool GameUnitsHost::plane_gun_trigger_bc9(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->plane_gun_fire_bc9;
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
    // Packet cc8_ship_follow: the wake trail 0070D290 measures a formation
    // follower's station along. Nothing consumes it yet, so these counters are
    // the whole observable effect of binding 00810190.
    {
        std::size_t with_trail = 0;
        unsigned long long appends = 0;
        unsigned long long advances = 0;
        unsigned long long merges = 0;
        float longest = 0.0f;
        for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
            const bsp::ShipAiWakeTrail& trail = owned->wake;
            if (trail.appends == 0) continue;
            ++with_trail;
            appends += trail.appends;
            advances += trail.advances;
            merges += trail.merges;
            const float length = bsp::ship_ai_wake_trail_length(trail);
            if (length > longest) longest = length;
        }
        host.log.notef("summary unit wake ships=%llu appends=%llu advances=%llu merges=%llu "
            "longest_trail=%.2f m (00810190, 4 m gate, 50 m legs, 40 slots)",
            static_cast<unsigned long long>(with_trail), appends, advances, merges,
            static_cast<double>(longest));
    }
    // Packet cc8_ship_follow: the unit groups at entity+284h. Columns are NOT
    // produced yet (0070ED30 needs 00811180's across sign), so membership is all
    // this reports and no station may be computed from these records.
    host.log.notef("summary unit formation groups=%llu joins=%llu creates=%llu rejoins=%llu "
        "clamped=%llu columns_unmeasurable=%llu (0070DB20 create, 0070EF30 join, "
        "0070ED30 column 0)",
        static_cast<unsigned long long>(host.formation_groups.size()),
        host.formation_joins, host.formation_creates, host.formation_rejoins,
        host.formation_clamped, host.formation_columns_unmeasurable);
    for (std::size_t g = 0; g < host.formation_groups.size(); ++g) {
        const Impl::FormationGroup& group = host.formation_groups[g];
        host.log.notef("  formation %zu leader=%s count=%zu column=%d", g,
            (group.leader < host.slots.size()) ? host.slots[group.leader]->row.name.c_str()
                                               : "?",
            group.members.size(), group.column);
        for (const bsp::ShipAiUnitGroupMember& member : group.members) {
            const std::size_t index = static_cast<std::size_t>(member.entity) - 1u;
            host.log.notef("    %-18s across=%9.2f along=%9.2f",
                (index < host.slots.size()) ? host.slots[index]->row.name.c_str() : "?",
                static_cast<double>(member.lateral[0]),
                static_cast<double>(member.axial[0]));
        }
    }
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
        // The dive-bomb task, kind 8: every ordered aircraft the arm 009C8790
        // ran, its state occupancy, the geometry the in-range latch 009C7C31
        // keys on, and the releases 009C60F1 / 009C5777 issued.
        // docs/DIVE_BOMB_TASK.md.
        {
            static const char* const kDiveBombStateNames[10] = {
                "moveto", "follow", "prepare", "done", "goaway",
                "aimdive", "aimglide", "flyabove", "turndown", "attackrun"};
            std::size_t tasked = 0;
            int releases_total = 0;
            int spawned_total = 0;
            bool geo_trace_printed = false;
            for (const auto& slot : host.slots) {
                if (!slot->dive_bomb_task_installed) continue;
                ++tasked;
                releases_total += slot->dive_bomb_releases;
                spawned_total += slot->torpedo_drops_spawned;
                char states[192];
                int n = 0;
                states[0] = '\0';
                for (int i = 0; i < 10; ++i) {
                    if (slot->dive_bomb_state_ticks[i] == 0) continue;
                    n += std::snprintf(states + n,
                        (n < static_cast<int>(sizeof(states)))
                            ? sizeof(states) - static_cast<std::size_t>(n) : 0u,
                        "%s%s=%d", n > 0 ? " " : "", kDiveBombStateNames[i],
                        slot->dive_bomb_state_ticks[i]);
                    if (n >= static_cast<int>(sizeof(states))) break;
                }
                // CORRECTED, packet cc8_dive_glide: `bombs_spawned` was printing
                // slot->torpedo_drops_spawned, which is the TORPEDO drop's
                // counter and can only ever be zero for a dive bomber. It now
                // prints the bomb spawn's own field, with the bay requests
                // 007BBBA0 accepted and refused beside it.
                host.log.notef("  divebomb %-12s arm_ticks=%d transitions=%d "
                    "states[%s] releases=%d bombs_spawned=%d rounds_left=%d "
                    "| bay 007BBBA0: accepted=%d refused=%d (torpedo column "
                    "was %d)",
                    slot->row.name.c_str(), slot->dive_bomb_arm_ticks,
                    slot->dive_bomb_transitions, states,
                    slot->dive_bomb_releases, slot->db_bombs_spawned,
                    slot->dive_bomb_rounds_remaining,
                    slot->db_bay_requests_accepted,
                    slot->db_bay_requests_refused,
                    slot->torpedo_drops_spawned);
                // 009C7240 / 009C7270, packet cc8_done_state. `placed` counts
                // the ticks on which the placement stand-in moved the aircraft,
                // which is zero for a flight LEADER by 009BFD70's own refusal -
                // so the leader's row is the one that shows what the image's
                // done state commands on its own, and it is one inert byte.
                if (slot->db_done_entries > 0) {
                    host.log.notef("  divebomb %-12s done 009C7240/009C7270: "
                        "entries=%d ticks=%d placed=%d plan_mode_26c=%d "
                        "alt %.1f -> %.1f hdg=%.4f",
                        slot->row.name.c_str(), slot->db_done_entries,
                        slot->db_done_tick_ticks, slot->db_done_placed_ticks,
                        slot->plan_mode_26c,
                        static_cast<double>(slot->db_done_entry_alt),
                        static_cast<double>(slot->db_done_last_alt),
                        static_cast<double>(slot->db_done_last_hdg));
                }
                // The first gate check the packet asks for: approach+BCh
                // against approach+B8h in the latch at 009C7C31.
                if (slot->db_turndown_ticks > 0) {
                    host.log.notef("  divebomb %-12s turndown 009C44F0: ticks=%d "
                        "roll_writes=%d pitch_writes=%d latched_at_tick=%d "
                        "|bank|=%.4f rad roll=%.4f pitch=%.4f speed_2b4=%.1f "
                        "| 009C7EA0 window: pose_c64_last=%.4f pose_c64_min=%.4f "
                        "(needs < -1.3, or < -1.0 with |bank| > 2.356)",
                        slot->row.name.c_str(), slot->db_turndown_ticks,
                        slot->db_turndown_roll_writes,
                        slot->db_turndown_pitch_writes,
                        slot->db_turndown_latched_tick,
                        static_cast<double>(slot->db_turndown_bank_last),
                        static_cast<double>(slot->db_turndown_roll_last),
                        static_cast<double>(slot->db_turndown_pitch_last),
                        static_cast<double>(slot->plane_desired_speed_2b4),
                        static_cast<double>(slot->db_turndown_pose_c64_last),
                        static_cast<double>(slot->db_turndown_pose_c64_min));
                }
                // Packet cc8_dive_geometry: the per-tick window, first slot
                // only - the three USN04 bombers fly identical traces.
                if (slot->db_geo_samples > 0 && !geo_trace_printed) {
                    geo_trace_printed = true;
                    host.log.notef("  divebomb %-12s geo trace: state tick "
                        "pitch_c64 bank_c68 heading_c6c alt range bearing_err "
                        "aim_head roll_in roll_cmd pitch_cmd mode_2d0 mode_2cc",
                        slot->row.name.c_str());
                    for (int i = 0; i < slot->db_geo_samples; ++i) {
                        const GameUnitSlot::DbGeoSample& s = slot->db_geo[i];
                        host.log.notef("  geo %-9s %5d %8.4f %8.4f %8.4f "
                            "%8.1f %8.1f %8.4f %8.4f %8.4f %7.3f %7.3f %d %d",
                            kDiveBombStateNames[s.state], s.tick,
                            static_cast<double>(s.pitch_c64),
                            static_cast<double>(s.bank_c68),
                            static_cast<double>(s.heading_c6c),
                            static_cast<double>(s.altitude),
                            static_cast<double>(s.range),
                            static_cast<double>(s.bearing_err),
                            static_cast<double>(s.aim_heading),
                            static_cast<double>(s.roll_input),
                            static_cast<double>(s.roll_cmd),
                            static_cast<double>(s.pitch_cmd),
                            s.pitch_mode_2d0, s.heading_mode_2cc);
                    }
                }
                if (slot->db_attackrun_ticks > 0) {
                    char ranges[192];
                    int rn = 0;
                    ranges[0] = '\0';
                    for (int i = 0; i < slot->db_range_samples; ++i) {
                        rn += std::snprintf(ranges + rn,
                            (rn < static_cast<int>(sizeof(ranges)))
                                ? sizeof(ranges) - static_cast<std::size_t>(rn) : 0u,
                            "%s%.0f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_range_at_second[i]));
                        if (rn >= static_cast<int>(sizeof(ranges))) break;
                    }
                    host.log.notef("  divebomb %-12s attackrun 009C4220: ticks=%d "
                        "rerolls=%d latch_closed_tick=%d heading=%.4f rad "
                        "throttle=%.3f alt_base=%.1f m | climb_1ec=%.4f rad "
                        "b4=%.1f b8=%.1f | range per second: %s",
                        slot->row.name.c_str(), slot->db_attackrun_ticks,
                        slot->db_attackrun_rerolls, slot->db_latch_closed_tick,
                        static_cast<double>(slot->db_attackrun_heading_last),
                        static_cast<double>(slot->db_attackrun_throttle_last),
                        static_cast<double>(slot->db_attackrun_alt_last),
                        // Packet cc8_dive_race: the two substitutions under test,
                        // printed so neither is argued from a source constant.
                        static_cast<double>(slot->plane_climb_angle_1ec),
                        static_cast<double>(slot->db_attack_dist_b4),
                        static_cast<double>(slot->db_in_range_b8), ranges);
                }
                // Packet cc8_dive_glide: the aimglide release chain
                // 009C5689-009C5755, per aircraft. blocked[] counts the calls
                // that stopped at each gate in order; the lead window is
                // -4*travel - 5.0 < lead < -5.0, so with the 009C4F50 seed of
                // 5.0 and approach+A4h held at zero it is -25.0 .. -5.0 m.
                if (slot->db_glide_calls > 0) {
                    host.log.notef("  divebomb %-12s aimglide 009C5689-009C5755: "
                        "calls=%d blocked[rearm=%d bearing=%d ceiling=%d "
                        "lateral=%d lead_hi=%d lead_lo=%d] passed=%d | "
                        "releases=%d rounds=%d | lead last=%.2f m min=%.2f m "
                        "(window -%.1f..-%.1f m) | bearing err min=%.4f rad "
                        "max=%.4f rad pull_outs=%d "
                        "(gate %.4f) | throw=%.1f m range=%.1f m travel=%.2f",
                        slot->row.name.c_str(), slot->db_glide_calls,
                        slot->db_glide_gate_reached[0], slot->db_glide_gate_reached[1],
                        slot->db_glide_gate_reached[2], slot->db_glide_gate_reached[3],
                        slot->db_glide_gate_reached[4], slot->db_glide_gate_reached[5],
                        slot->db_glide_gate_reached[6],
                        slot->db_glide_releases, slot->db_glide_rounds,
                        static_cast<double>(slot->db_glide_lead_last),
                        static_cast<double>(slot->db_glide_lead_min),
                        4.0 * static_cast<double>(slot->db_glide_travel_20) + 5.0,
                        5.0,
                        static_cast<double>(slot->db_glide_bearing_min),
                        static_cast<double>(slot->db_glide_bearing_max),
                        slot->db_aimglide_pull_outs,
                        static_cast<double>(
                            bsp::dive_bomb_constant::kGlideDiveAngleLimit),
                        static_cast<double>(slot->db_impact_throw_14),
                        static_cast<double>(slot->db_planar_bc),
                        static_cast<double>(slot->db_glide_travel_20));
                }
                // Packet cc8_dive_goaway. 009C4A40's climb-out and 009C7F00's
                // completion, side by side: the state exists to put the aircraft
                // back at its ceiling, so the question is only whether the pitch
                // it commands moves the altitude toward the ceiling.
                if (slot->db_goaway_tick_ticks > 0) {
                    host.log.notef("  divebomb %-12s goaway 009C4A40: ticks=%d "
                        "pitch last=%.4f max=%.4f rad | alt first=%.1f last=%.1f "
                        "max=%.1f m | ceiling=%.1f m deficit=%.1f m | "
                        "009C7F00 complete_ticks=%d travel_20=%.2f",
                        slot->row.name.c_str(), slot->db_goaway_tick_ticks,
                        static_cast<double>(slot->db_goaway_pitch_last),
                        static_cast<double>(slot->db_goaway_pitch_max),
                        static_cast<double>(slot->db_goaway_alt_first),
                        static_cast<double>(slot->db_goaway_alt_last),
                        static_cast<double>(slot->db_goaway_alt_max),
                        static_cast<double>(slot->db_goaway_ceiling_last),
                        static_cast<double>(slot->db_goaway_deficit_last),
                        slot->db_goaway_complete_ticks,
                        static_cast<double>(slot->db_goaway_travel_20));
                    // Packet cc9_goaway_turn: the evasive turn's arms.
                    const bsp::DiveBombGoAwayTurnState& gt = slot->db_goaway_turn;
                    host.log.notef("  divebomb %-12s goaway turn 009C4DAD: "
                        "enters=%d flag0=%d countdown=%d rerolls=%d "
                        "first_reroll=%d bank=%d heading=%d first_heading=%d "
                        "side_writes=%d | bank min=%.3f max=%.3f rad | "
                        "heading last=%.4f err_max=%.4f rad | side=%.1f "
                        "standoff=%.1f countdown=%.2f clock=%.2f window=%.2f",
                        slot->row.name.c_str(), slot->db_goaway_enters,
                        slot->db_goaway_flag0_ticks,
                        slot->db_goaway_countdown_ticks, slot->db_goaway_rerolls,
                        slot->db_goaway_first_reroll_tick,
                        slot->db_goaway_bank_ticks, slot->db_goaway_heading_ticks,
                        slot->db_goaway_first_heading_tick,
                        slot->db_goaway_side_writes,
                        static_cast<double>(slot->db_goaway_bank_min),
                        static_cast<double>(slot->db_goaway_bank_max),
                        static_cast<double>(slot->db_goaway_heading_last),
                        static_cast<double>(slot->db_goaway_heading_err_max),
                        static_cast<double>(gt.side_18),
                        static_cast<double>(gt.standoff_20),
                        static_cast<double>(gt.countdown_24),
                        static_cast<double>(gt.clock_28),
                        static_cast<double>(gt.window_2c));
                }
                host.log.notef("  divebomb %-12s gate 009C7C31: "
                    "approach+BCh=%.1f m approach+B8h=%.1f m latch_D0h=%d "
                    "bomb_D1h=%d | ticks without latch=%d without bombs=%d",
                    slot->row.name.c_str(),
                    static_cast<double>(slot->db_planar_bc),
                    static_cast<double>(slot->db_in_range_b8),
                    slot->db_in_range_d0 ? 1 : 0,
                    slot->db_has_bomb_d1 ? 1 : 0,
                    slot->db_blocked_no_latch, slot->db_blocked_no_bomb);
                // Packet cc8_dive_entry: the altitude at every hand-over, for
                // every dive bomber, outside the dive-entry guard below so the
                // aircraft that never dive are in it too. `cmd` is
                // plane_commanded_altitude, which only the run-in writes in this
                // host - 009C6E10-009C6F7D, the flyabove's own altitude command,
                // is unbound - so a cmd that stops moving across the flyabove is
                // that gap measured rather than argued.
                if (slot->db_transition_count > 0) {
                    char tr[1200];  // cc8_dive_heading widened the per-entry text
                    int tn = 0;
                    tr[0] = '\0';
                    for (int i = 0; i < slot->db_transition_count; ++i) {
                        const GameUnitSlot::DbTransition& t =
                            slot->db_transitions[i];
                        tn += std::snprintf(tr + tn,
                            (tn < static_cast<int>(sizeof(tr)))
                                ? sizeof(tr) - static_cast<std::size_t>(tn) : 0u,
                            "%s%s>%s@%d alt=%.0f rng=%.0f cmd=%.0f "
                            "span=%.0f b=%.0f f18=%d f19=%d",
                            i > 0 ? " | " : "",
                            (t.from >= 0 && t.from < 10)
                                ? kDiveBombStateNames[t.from] : "?",
                            (t.to >= 0 && t.to < 10)
                                ? kDiveBombStateNames[t.to] : "?",
                            t.tick, static_cast<double>(t.alt),
                            static_cast<double>(t.range),
                            static_cast<double>(t.cmd_alt),
                            static_cast<double>(t.span),
                            static_cast<double>(t.b_height),
                            static_cast<int>(t.f18), static_cast<int>(t.f19));
                        if (tn >= static_cast<int>(sizeof(tr))) break;
                    }
                    host.log.notef("  divebomb %-12s hand-overs: %s",
                        slot->row.name.c_str(), tr);
                }
                if (slot->db_fa_alt_calls > 0) {
                    host.log.notef("  divebomb %-12s flyabove altitude "
                        "009C6E10: calls=%d level_arm=%d | C=%.1f band=%.1f "
                        "target=%.1f ref=%.3f | err first=%.1f last=%.1f | "
                        "pitch=%.3f rad",
                        slot->row.name.c_str(), slot->db_fa_alt_calls,
                        slot->db_fa_level_ticks,
                        static_cast<double>(slot->db_fa_limit_c),
                        static_cast<double>(slot->db_fa_band),
                        static_cast<double>(slot->db_fa_target_last),
                        static_cast<double>(slot->db_fa_ref_last),
                        static_cast<double>(slot->db_fa_err_first),
                        static_cast<double>(slot->db_fa_err_last),
                        static_cast<double>(slot->db_fa_pitch_last));
                }
                if (slot->db_dive_entry_alt >= 0.0f ||
                    slot->db_release_alt >= 0.0f) {
                    host.log.notef("  divebomb %-12s dive entry alt=%.1f m "
                        "pitch=%.3f rad | release alt=%.1f m speed=%.1f m/s "
                        "range=%.1f m | aim error 009C5C9B=%.2f m "
                        "(gate 00CE3880 = 25.0 m) closest=%.2f m at range=%.1f m "
                        "alt=%.1f m | 009C58D0 steer: ticks=%d pitch=%.3f "
                        "roll=%.3f bearing=%.4f rad | flyabove B=%.1f m "
                        "span=%.1f m",
                        slot->row.name.c_str(),
                        static_cast<double>(slot->db_dive_entry_alt),
                        static_cast<double>(slot->db_dive_entry_pitch),
                        static_cast<double>(slot->db_release_alt),
                        static_cast<double>(slot->db_release_speed),
                        static_cast<double>(slot->db_release_range),
                        static_cast<double>(slot->db_aim_error_last),
                        static_cast<double>(slot->db_aim_error_abs_min),
                        static_cast<double>(slot->db_aim_error_min_range),
                        static_cast<double>(slot->db_aim_error_min_alt),
                        slot->db_aimdive_steer_ticks,
                        static_cast<double>(slot->db_aimdive_pitch_last),
                        static_cast<double>(slot->db_aimdive_roll_last),
                        static_cast<double>(slot->db_aimdive_bearing_last),
                        static_cast<double>(slot->db_flyabove_height),
                        static_cast<double>(slot->db_flyabove_span));
                // Packet cc8_dive_release item 1: which 009C8634 arm ends the
                // aimdive, and the 009C5B01 abort's own operands when it first
                // fired. MaxSpd/ReferenceSpeed is item 4's quotient, logged in
                // a run taken anyway.
                host.log.notef("  divebomb %-12s aimdive: entries=%d exits=%d "
                    "by[alive_19=%d nobomb_d1=%d pullout_18=%d other=%d] "
                    "first_run=%d last_run=%d | abort 009C5B43: fires=%d "
                    "first_tick=%d d4=%.1f h14=%.1f range=%.1f pitch=%.3f | "
                    "speed: max=%.2f ref=%.2f quotient=%.3f | impact 009C7D71: "
                    "tf=%.2f s range=%.1f m (live %.1f m)",
                    slot->row.name.c_str(),
                    slot->db_aimdive_entries, slot->db_aimdive_exits,
                    slot->db_aimdive_exit_alive, slot->db_aimdive_exit_nobomb,
                    slot->db_aimdive_exit_pullout, slot->db_aimdive_exit_other,
                    slot->db_aimdive_first_run, slot->db_aimdive_last_run,
                    slot->db_abort_fires, slot->db_abort_first_tick,
                    static_cast<double>(slot->db_abort_d4),
                    static_cast<double>(slot->db_abort_h14),
                    static_cast<double>(slot->db_abort_range),
                    static_cast<double>(slot->db_abort_pitch),
                    // desc+188h MaxSpd, NOT motion.max_speed: that one is the
                    // ship row and is 0.00 for every aircraft, which is what the
                    // first run of this packet measured.
                    static_cast<double>(slot->plane_max_spd),
                    // GAME_TUNING_SINGLETON +4D8h Pilot/DiveBomb/ReferenceSpeed
                    // = KMH(280); 009C3EA0 reads it.
                    static_cast<double>(280.0f / 3.6f),
                    static_cast<double>(slot->plane_max_spd /
                                        (280.0f / 3.6f)),
                    static_cast<double>(slot->db_impact_fall_time),
                    static_cast<double>(slot->db_impact_planar_5c),
                    static_cast<double>(slot->db_planar_bc));
                // Packet cc8_attack_mode. squadron+370h per aircraft, and what
                // the spent bomber did afterwards. mode_ticks is indexed by the
                // PilotAttackMode value: [0]=kHold, [1]=kAttack (what 0099B740
                // sets), [2]=kForced (what the Lua attack order raises to).
                host.log.notef("  divebomb %-12s attack mode: lead=%d "
                    "mode_370=%d ticks[hold=%d attack=%d forced=%d] changes=%d "
                    "first_set_tick=%d | after: done_ticks=%d done_min_alt=%.1f "
                    "approach_returns=%d first_return_tick=%d",
                    slot->row.name.c_str(),
                    slot->db_is_flight_lead ? 1 : 0,
                    static_cast<int>(slot->db_attack_mode_370),
                    slot->db_mode_ticks[0], slot->db_mode_ticks[1],
                    slot->db_mode_ticks[2], slot->db_mode_changes,
                    slot->db_mode_first_set_tick,
                    slot->db_post_done_ticks,
                    static_cast<double>(slot->db_post_done_min_alt),
                    slot->db_approach_returns, slot->db_approach_return_tick);
                // 009FBA50's own terms, to settle why correcting its arguments
                // changed nothing observable.
                if (slot->db_cruise_samples > 0) {
                    char ct[320];
                    int cn = 0;
                    ct[0] = '\0';
                    for (int i = 0; i < slot->db_cruise_samples; ++i) {
                        cn += std::snprintf(ct + cn,
                            (cn < static_cast<int>(sizeof(ct)))
                                ? sizeof(ct) - static_cast<std::size_t>(cn) : 0u,
                            "%s%.0f/%.3f/%.3f/%.0f/%.0f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_cruise_span[i]),
                            static_cast<double>(slot->db_cruise_gain[i]),
                            static_cast<double>(slot->db_cruise_scale[i]),
                            static_cast<double>(slot->db_cruise_base[i]),
                            static_cast<double>(slot->db_cruise_clamped[i]));
                        if (cn >= static_cast<int>(sizeof(ct))) break;
                    }
                    host.log.notef("  divebomb %-12s 009FBA50 terms "
                        "span/gain/scale/base/clamped per 120 attackrun ticks: %s",
                        slot->row.name.c_str(), ct);
                }
                // The aim trace: what the endpoint numbers cannot say.
                if (slot->db_aim_state_ticks > 0) {
                    char trace[320];
                    int tn = 0;
                    trace[0] = '\0';
                    for (int i = 0; i < slot->db_aim_trace_samples; ++i) {
                        tn += std::snprintf(trace + tn,
                            (tn < static_cast<int>(sizeof(trace)))
                                ? sizeof(trace) - static_cast<std::size_t>(tn) : 0u,
                            "%s%.0f/%.0f/%.2f/%.2f", i > 0 ? " " : "",
                            static_cast<double>(slot->db_aim_trace_range[i]),
                            static_cast<double>(slot->db_aim_trace_error[i]),
                            static_cast<double>(slot->db_aim_trace_bank[i]),
                            static_cast<double>(slot->db_aim_trace_bearing[i]));
                        if (tn >= static_cast<int>(sizeof(trace))) break;
                    }
                    host.log.notef("  divebomb %-12s aim trace: 009C62B0 ticks=%d "
                        "heading writes=%d last=%.4f rad | turndown entry "
                        "range=%.1f m bearing=%.4f rad | aimdive entry "
                        "range=%.1f m bearing=%.4f rad | closest range=%.1f m | "
                        "ticks=%d | range/error/bank/bearing per 30 ticks: %s",
                        slot->row.name.c_str(),
                        slot->db_flyabove_tick_ticks,
                        slot->db_flyabove_heading_writes,
                        static_cast<double>(slot->db_flyabove_heading_last),
                        static_cast<double>(slot->db_turndown_entry_range),
                        static_cast<double>(slot->db_turndown_entry_bearing),
                        static_cast<double>(slot->db_aimdive_entry_range),
                        static_cast<double>(slot->db_aimdive_entry_bearing),
                        static_cast<double>(slot->db_aimdive_min_range),
                        slot->db_aim_state_ticks, trace);
                    // Packet cc8_dive_flyover: the bank arm's own census. `bl`
                    // is 009C67BF, `latch` the 009C6919 flyabove+1Ch, `sup` the
                    // ticks 009C6DDA skipped the heading write, `T` the dead
                    // band as it last reached 009C6A43, `along`/`cross` the two
                    // 009C68D4 / 009C6889 distances on the last bank tick.
                    host.log.notef("  divebomb %-12s flyabove bank: bl=%d "
                        "latch=%d@%d sup=%d T=%.4f rad along=%.1f m "
                        "cross=%.1f m turn_circle=%.1f m",
                        slot->row.name.c_str(),
                        slot->db_flyabove_bl_ticks,
                        slot->db_flyabove_bank_latch_1c ? 1 : 0,
                        slot->db_flyabove_latch_tick,
                        slot->db_flyabove_latched_ticks,
                        static_cast<double>(slot->db_flyabove_dead_band_t),
                        static_cast<double>(slot->db_flyabove_along_track),
                        static_cast<double>(slot->db_flyabove_cross_track),
                        static_cast<double>(slot->plane_turn_circle_radius));
                }
                }
            }
            {
                // Packet cc9_dogfight_task.
                std::size_t df_aircraft = 0;
                int df_moveto = 0, df_follow = 0, df_near = 0;
                for (const auto& slot : host.slots) {
                    if (!slot->dogfight_task_installed) continue;
                    ++df_aircraft;
                    df_moveto += slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kMoveTo)];
                    df_follow += slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kFollow)];
                    df_near += slot->df_within_attack_dist_ticks;
                    host.log.notef("  dogfight %-12s moveto=%d follow=%d transitions=%d "
                        "min_target_range=%.1f within_attack_dist_ticks=%d",
                        slot->row.name.c_str(),
                        slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kMoveTo)],
                        slot->df_state_ticks[static_cast<int>(bsp::DogfightState::kFollow)],
                        slot->df_transitions,
                        static_cast<double>(slot->df_min_target_range),
                        slot->df_within_attack_dist_ticks);
                    {
                        const auto& k = slot->df_state_ticks;
                        using S = bsp::DogfightState;
                        host.log.notef("  dogfight-engaged %-12s aim=%d maneuver=%d avoid=%d "
                            "attackrun=%d prepare=%d latch_sets=%d latched_ticks=%d "
                            "reselects=%d target_changes=%d gun_window_ticks=%d target=%s",
                            slot->row.name.c_str(), k[static_cast<int>(S::kAim)],
                            k[static_cast<int>(S::kManeuver)],
                            k[static_cast<int>(S::kAvoidRoll)] + k[static_cast<int>(S::kAvoidTurn)],
                            k[static_cast<int>(S::kAttackRun)], k[static_cast<int>(S::kPrepare)],
                            slot->df_latch_sets, slot->df_latched_ticks, slot->df_reselects,
                            slot->df_target_changes, slot->df_gun_window_ticks,
                            (slot->df_target_plus_one != 0 &&
                             slot->df_target_plus_one - 1 < host.slots.size())
                                ? host.slots[slot->df_target_plus_one - 1]->row.name.c_str()
                                : "-");
                    }
                }
                {
                    {
                        int calls = 0, hits = 0, weaves = 0, slots = 0;
                        for (const auto& slot : host.slots) {
                            calls += slot->nf_calls;
                            hits += slot->nf_hits;
                            weaves += slot->nf_attackrun_weaves;
                            slots += slot->nf_flyover_slot_writes;
                            if (slot->nf_hits == 0) continue;
                            host.log.notef("  near field %-12s calls=%d hits=%d attackrun_weaves=%d "
                                "max_offset=%.3f flyover_slot_writes=%d slot=[%.3f %.3f] goaway_hits=%d",
                                slot->row.name.c_str(), slot->nf_calls, slot->nf_hits,
                                slot->nf_attackrun_weaves,
                                static_cast<double>(slot->nf_attackrun_max_offset),
                                slot->nf_flyover_slot_writes,
                                static_cast<double>(slot->nf_flyover_slot_min),
                                static_cast<double>(slot->nf_flyover_slot_max),
                                slot->nf_goaway_hits);
                        }
                        host.log.notef("summary mission near field probe: calls=%d hits=%d "
                            "attackrun_weaves=%d flyover_slot_writes=%d", calls, hits, weaves, slots);
                    }
                    int bursts = 0, fire_ticks = 0;
                    for (const auto& slot : host.slots) {
                        if (!slot->dogfight_task_installed) continue;
                        bursts += slot->df_gun.bursts;
                        fire_ticks += slot->df_gun.fire_ticks;
                        host.log.notef("  fighter gun %-12s bursts=%d fire_ticks=%d "
                            "c50_refreshes=%d finder_scans=%d enemy_list=%zu trigger_ticks=%d "
                            "trigger_rises=%d early_edges=%d early_asks=%d head_on_ticks=%d "
                            "head_on_throttle_min=%.3f",
                            slot->row.name.c_str(), slot->df_gun.bursts, slot->df_gun.fire_ticks,
                            slot->nb_refreshes, slot->nb_scans, slot->nb_enemy_50.size(),
                            slot->pg_trigger_ticks, slot->pg_trigger_rises, slot->df_early_edges,
                            slot->df_early_asks, slot->df_head_on_ticks,
                            static_cast<double>(slot->df_head_on_throttle_min));
                        host.log.notef("  dogfight moveto %-12s speed_commands=%d speed=[%.1f %.1f] pilot_fires=%d",
                            slot->row.name.c_str(), slot->df_moveto_speed_commands,
                            static_cast<double>(slot->df_moveto_speed_commands ? slot->df_moveto_speed_min : 0.0f),
                            static_cast<double>(slot->df_moveto_speed_max),
                            slot->plane_pilot_fires_c24 ? 1 : 0);
                    }
                    if (df_aircraft > 0) {
                        host.log.notef("summary mission fighter gun: bursts=%d fire_ticks=%d "
                            "rounds=0 (the gunFire consumer is not established; "
                            "docs/DOGFIGHT_GUN.md)", bursts, fire_ticks);
                    }
                }
                if (df_aircraft > 0) {
                    host.log.notef("summary mission dogfight task: aircraft=%zu "
                        "moveto_ticks=%d follow_ticks=%d within_attack_dist_ticks=%d "
                        "(engaged: docs/DOGFIGHT_ENGAGED.md)",
                        df_aircraft, df_moveto, df_follow, df_near);
                }
            }
            if (tasked > 0) {
                host.log.notef("summary mission dive-bomb task: aircraft=%zu "
                    "releases=%d bombs_spawned=%d",
                    tasked, releases_total, spawned_total);
                if (releases_total == 0) {
                    host.log.notef("summary mission dive-bomb task: no release. "
                        "The torpedo's blocker, unit+C58h at 0099AF53, does not "
                        "apply here: 009C60F1 and 009C5777 are inside state "
                        "ticks the arm reaches through state->vtable[+Ch] at "
                        "009C884C, not through the arming loop. Read the walk "
                        "off the state ticks above: the in-range latch "
                        "approach+D0h at 009C7C31 gates entry to flyabove, "
                        "009C7EA0 gates turndown to aimdive, and the release "
                        "itself needs the aim error 009C5C9B inside the 25.0 m "
                        "at 00CE3880. The per-aircraft lines carry all three.");
                }
            }
        }

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
                    // cc8_torpedo_retire item 1. 009D4C10's inputs on the FIRST
                    // tick its binding returned true. arm 2 = the target pair
                    // (`target == 0 || target->+5Dh`), arm 5 = the range test
                    // (`SafeDist * ratio <= task+488h`). Nothing else can return
                    // true once base_gate is proved.
                    host.log.notef("  torpedo %-12s break-off 009D4C10 first true: "
                        "arm=%d at_arm_tick=%d true_ticks=%d state=%s "
                        "has_target=%d target_plus_one=%d target_marked=%d "
                        "in_attack=%d flag_52a=%d ordnance_132=%d "
                        "range_90=%.1f prev_range_90=%.1f limit_90_488h=%.1f "
                        "safe_dist=%.1f threshold=%.1f "
                        "approach_ticks=%d no_target_ticks=%d",
                        slot->row.name.c_str(),
                        slot->torpedo_breakoff_arm,
                        slot->torpedo_breakoff_first_tick,
                        slot->torpedo_breakoff_true_ticks,
                        kTorpedoStateNames[
                            static_cast<std::size_t>(
                                slot->torpedo_breakoff_state) & 7u],
                        slot->torpedo_breakoff_has_target ? 1 : 0,
                        slot->torpedo_breakoff_target_plus_one,
                        slot->torpedo_breakoff_target_marked ? 1 : 0,
                        slot->torpedo_breakoff_in_attack_state ? 1 : 0,
                        slot->torpedo_breakoff_attack_flag_52a ? 1 : 0,
                        slot->torpedo_breakoff_has_ordnance_132 ? 1 : 0,
                        static_cast<double>(slot->torpedo_breakoff_range_90),
                        static_cast<double>(slot->torpedo_breakoff_prev_range),
                        static_cast<double>(slot->torpedo_breakoff_limit_90),
                        static_cast<double>(slot->torpedo_breakoff_safe_dist),
                        static_cast<double>(slot->torpedo_breakoff_safe_dist),
                        slot->torpedo_breakoff_approach_ticks,
                        slot->torpedo_breakoff_no_target_ticks);
                    // cc8_torpedo_retire item 5. The run-in geometry at
                    // ATTACKRUN ENTRY, for comparison with the crossing angle
                    // the closest-approach census reports at the far end of the
                    // swim. Both target numbers are hull POSE headings, so the
                    // two are commensurable; the ship-ai step heading is a
                    // different field and is deliberately absent here.
                    host.log.notef("  torpedo %-12s run-in at attackrun entry: "
                        "entries=%d yaw_C6C=%.4f own_pose=%.4f "
                        "target_pose=%.4f crossing=%.4f rad (%.1f deg)",
                        slot->row.name.c_str(),
                        slot->torpedo_attackrun_entries,
                        static_cast<double>(slot->torpedo_attackrun_yaw_c6c),
                        static_cast<double>(slot->torpedo_attackrun_own_pose),
                        static_cast<double>(slot->torpedo_attackrun_target_pose),
                        static_cast<double>(slot->torpedo_attackrun_crossing),
                        static_cast<double>(slot->torpedo_attackrun_crossing) *
                            180.0 / kPi);
                    // cc8_torpedo_retire item 4, observation only: the speed
                    // ratio task+41Ch = approach+24h that 009F9CE0 writes as
                    // max(1.0f, classBlock->+188h / reference_speed). The
                    // divisor is the TORPEDO row and it is not a stand-in any
                    // more: 009D03A1-009D03B7 inside BSP_BotApproachTorpedo_Reset
                    // calls 0042E740 and pushes tuning+440h
                    // (Pilot/Torpedo/ReferenceSpeed, docs/BOT_TASKS.md:246) as
                    // 009F9CE0's third argument. The numerator is MaxSpd
                    // (src/plane_class_fields.cpp, .MaxSpd -> +188h), which this
                    // host already carries as plane_max_spd.
                    {
                        const float ref = host.lua.plane_globals_loaded()
                            ? host.lua.plane_globals().pilot_torpedo_reference_speed
                            : 0.0f;
                        const float ratio = (ref > 0.0f)
                            ? bsp::bot_task_speed_ratio(slot->plane_max_spd, ref)
                            : 1.0f;
                        host.log.notef("  torpedo %-12s speed ratio 009F9CE0: "
                            "max_spd_188h=%.2f reference_speed_440h=%.2f "
                            "ratio_41Ch=%.4f break_off_threshold=%.1f "
                            "(bound=1.0)",
                            slot->row.name.c_str(),
                            static_cast<double>(slot->plane_max_spd),
                            static_cast<double>(ref),
                            static_cast<double>(ratio),
                            static_cast<double>(
                                (host.lua.plane_globals_loaded()
                                     ? host.lua.plane_globals().pilot_torpedo_safe_dist
                                     : 0.0f) * ratio));
                    }
                    // 009D0F10, bound by packet cc8_flyto_solver_and_goaway.
                    // arms: 1 = window/below 20 m (climb to 1000 on +18h),
                    // 2 = window/above 20 m (roll), 3 = post-window/high
                    // (heading +18h), 4 = post-window/low (wings level).
                    host.log.notef("  torpedo %-12s goaway 009D0F10: ticks=%d "
                        "arms[win_low=%d win_high=%d post_high=%d post_low=%d] "
                        "heading_ticks=%d climb_1Ch=%.1f known=%d high_20h=%.1f "
                        "alt_cmd=%.1f alt_range=[%.1f,%.1f]",
                        slot->row.name.c_str(),
                        slot->torpedo_goaway_ticks,
                        slot->torpedo_goaway_arm_ticks[1],
                        slot->torpedo_goaway_arm_ticks[2],
                        slot->torpedo_goaway_arm_ticks[3],
                        slot->torpedo_goaway_arm_ticks[4],
                        slot->torpedo_goaway_heading_ticks,
                        static_cast<double>(
                            slot->torpedo_goaway_runtime.climb_altitude_1c),
                        slot->torpedo_goaway_runtime.climb_altitude_known ? 1 : 0,
                        static_cast<double>(
                            slot->torpedo_goaway_runtime.high_threshold_20),
                        static_cast<double>(slot->torpedo_goaway_alt_cmd_last),
                        static_cast<double>(slot->torpedo_goaway_ticks > 0
                            ? slot->torpedo_goaway_alt_min : 0.0f),
                        static_cast<double>(slot->torpedo_goaway_ticks > 0
                            ? slot->torpedo_goaway_alt_max : 0.0f));
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
