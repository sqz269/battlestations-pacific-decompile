// bsp_ship_motion_probe - drive one reconstructed hull with one reconstructed order.
//
// The chain the probe runs, all of it reconstructed code:
//
//   vehicleclasses.lua (installed)  -> the class descriptor floats the motion reads
//   00815440 / 00816A40 / 0080DAD0  -> a throttle-and-rudder order into the ring
//   00813020                        -> the ring tick, which writes unit+980h / +984h
//   00825F20                        -> the motion tick: the gate, the target speed,
//                                      0092D300 (speed) and 0092E8C0 (steering)
//   00C41550 then 00C5B1B0          -> position and attitude, the Dyn library's own two
//                                      integration phases for one substep
//
// The four stand-ins the first version of this probe carried are gone: 0078CF20 (the
// ocean sampler), 008E6430 (the gameplay-modifier product), the rudder curve settings at
// +438h..+44Ch, and the integrator itself are all reconstructed now. The hand-built hull
// body is gone too: the body now comes from 00937C90's tail through
// bsp/ship_hull_body.hpp, and the world constants from 004DDB90 through
// bsp/dyn_world_settings.hpp, which settles the substep size (0.05f, one substep of the
// whole game step) and the substep budget (1).
//
// The probe runs the same trajectory twice so the difference the real body makes is
// measured rather than asserted: once with the old hand-built stand-in (no mass, no
// inertia, no damping, no gravity, clamps at 1e30) and once with the class-built body in
// the game's own world.
//
// What is still open, and is printed in the header so no reader mistakes it for recovered
// behaviour: the force path into the body (009329C0's hydrodynamics are not reconstructed,
// so nothing pushes force here and nothing cancels gravity), and the hull's collision AABB,
// whose producer is the shape attach 00C5C940 this packet does not read. The AABB decides
// the inertia, so the probe takes it as an argument and defaults it to zero.
//
// docs/SHIP_MOTION.md, docs/RIGID_BODY_INTEGRATION.md, docs/SHIP_HULL_BODY.md,
// docs/DYN_WORLD_SETTINGS.md, docs/UNIT_RUDDER_CURVE.md, docs/OCEAN_HEIGHT.md,
// docs/CONTROLLED_UNIT.md.

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "bsp/cruise_command.hpp"
#include "bsp/ship_ai_navigation.hpp"
#include "bsp/ship_ai_states.hpp"
#include "bsp/unit_autopilot_pair.hpp"
#include "bsp/vector_helpers.hpp"
#include "bsp/unit_commanded_speed.hpp"
#include "bsp/dyn_physics_substep.hpp"
#include "bsp/dyn_world_settings.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/ship_hull_body.hpp"
#include "bsp/ship_motion.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_order_record.hpp"
#include "bsp/unit_orders.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/unit_state_message.hpp"

namespace {

const char* const kDefaultLuaPath =
    "I:/SteamLibrary/steamapps/common/Battlestations Pacific/scripts/datatables/autoload/"
    "vehicleclasses.lua";

// -------------------------------------------------------------------------
// The installed class table
// -------------------------------------------------------------------------

struct LuaVehicleClass {
    int index{0};
    std::string comment;
    std::string name;
    std::string type;
    bool have_max_speed{false};
    float max_speed{0.0f};
    float max_accel{0.0f};
    float retardation{0.0f};
    float max_rot_angle{0.0f};
    float max_rot_angle_change_ratio{0.0f};
    // class+B0h, +A0h, +A8h. docs/VEHICLE_CLASS_FIELDS.md; `Mass` defaults to 1.0f and a
    // missing `Length` or `Height` stores 0.
    float mass{1.0f};
    float length{0.0f};
    float height{0.0f};
};

// The table is written one key per line. A top-level key of a VehicleClass block is
// indented with exactly one tab; anything nested carries more. That rule, and the
// `VehicleClass[N] =` block header, are the whole grammar this needs.
bool line_is_top_level_key(const std::string& line, std::string& key, std::string& value) {
    if (line.size() < 4 || line[0] != '\t' || line[1] == '\t' || line[1] != '[') {
        return false;
    }
    const std::size_t open = line.find("[\"", 1);
    if (open != 1) {
        return false;
    }
    const std::size_t close = line.find("\"]", open + 2);
    if (close == std::string::npos) {
        return false;
    }
    key = line.substr(open + 2, close - open - 2);
    const std::size_t eq = line.find('=', close);
    if (eq == std::string::npos) {
        return false;
    }
    std::size_t begin = line.find_first_not_of(" \t", eq + 1);
    if (begin == std::string::npos) {
        return false;
    }
    std::size_t end = line.find_last_not_of(" \t\r,");
    if (end == std::string::npos || end < begin) {
        return false;
    }
    value = line.substr(begin, end - begin + 1);
    return true;
}

std::string strip_quotes(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

bool load_vehicle_classes(const std::string& path, std::vector<LuaVehicleClass>& out,
                          std::string& error) {
    std::ifstream in(path);
    if (!in) {
        error = "cannot open " + path;
        return false;
    }
    std::string line;
    bool in_block = false;
    LuaVehicleClass current{};
    while (std::getline(in, line)) {
        if (line.compare(0, 13, "VehicleClass[") == 0) {
            if (in_block) {
                out.push_back(current);
            }
            current = LuaVehicleClass{};
            in_block = true;
            current.index = std::atoi(line.c_str() + 13);
            const std::size_t dash = line.find("--");
            if (dash != std::string::npos) {
                std::size_t b = line.find_first_not_of(" \t", dash + 2);
                if (b != std::string::npos) {
                    current.comment = line.substr(b);
                }
            }
            continue;
        }
        if (!in_block) {
            continue;
        }
        std::string key;
        std::string value;
        if (!line_is_top_level_key(line, key, value)) {
            continue;
        }
        if (key == "MaxSpeed") {
            current.max_speed = static_cast<float>(std::atof(value.c_str()));
            current.have_max_speed = true;
        } else if (key == "MaxAccel") {
            current.max_accel = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "Retardation") {
            current.retardation = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "MaxRotAngle") {
            current.max_rot_angle = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "MaxRotAngleChangeRatio") {
            current.max_rot_angle_change_ratio = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "Mass") {
            current.mass = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "Length") {
            current.length = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "Height") {
            current.height = static_cast<float>(std::atof(value.c_str()));
        } else if (key == "Name") {
            current.name = strip_quotes(value);
        } else if (key == "Type") {
            current.type = strip_quotes(value);
        }
    }
    if (in_block) {
        out.push_back(current);
    }
    return true;
}

// -------------------------------------------------------------------------
// Hosts
// -------------------------------------------------------------------------

// The rudder curve settings at +438h..+44Ch are the gameplay settings singleton's, filled
// by the Lua settings loader 0083B5E0 from ShipGlobals["Navigator"]["TurnMultipliers"].
// The installed shipglobals.lua authors them as {0.0, 0.4}, {0.5, 1.5}, {1.0, 2.0}.
// docs/UNIT_RUDDER_CURVE.md. This replaces the identity-curve stand-in the probe used.
bsp::UnitRudderCurveSettings shipped_curve() {
    return bsp::unit_rudder_curve_settings(bsp::kShippedTurnMultipliers);
}

struct ProbeRudderHost final : bsp::UnitRudderHost {
    bsp::ShipClassFields fields{};
    bsp::UnitRudderCurveSettings settings = shipped_curve();
    float forward_speed{0.0f};
    float efficiency{1.0f};
    float steering{0.0f};

    const bsp::UnitRudderCurveSettings& settings_00424c40() override { return settings; }
    bool scale_manager_present() override { return false; }
    const bsp::ShipClassFields& ship_class() override { return fields; }
    bool gameplay_scale_enabled() override { return false; }
    bool scale_manager_enabled() override { return false; }
    // 008E6430 with an empty modifier list. The product starts at the 1.0f at 00D7A24C
    // and a mission that registers nothing leaves it there, which is also the value
    // 0080FC30 substitutes when its two gates fail.
    float gameplay_scale_008e6430(int) override {
        return bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
    }
    struct NoModifiers final : bsp::GameplayModifierHost {
        bool entry_matches_008e4680(const bsp::GameplayModifierEntry&) override { return false; }
    } modifiers{};
    float turn_efficiency() override { return efficiency; }
    float forward_speed_0092d730() override { return forward_speed; }
    float steering_command() override { return steering; }
};

struct ProbeIssueHost final : bsp::UnitOrderRecordIssueHost {
    int mode{1};
    int session_mode() override { return mode; }
    bsp::UnitOrderMessageStorage construct_message_0075b430(int) override {
        return bsp::UnitOrderMessageStorage{};
    }
    void send_message_0077c2a0(const bsp::UnitOrderMessageStorage&, std::uint32_t,
                               std::uint32_t) override {}
};

struct ProbeMotionHost final : bsp::ShipMotionHost {
    bsp::ShipMotionState* state{nullptr};
    bsp::ShipMotionClass cls{};
    bsp::UnitOrderRing* ring{nullptr};
    ProbeRudderHost* rudder{nullptr};
    int session_mode{1};
    float last_yaw_rate{0.0f};
    float last_force_model_dt{0.0f};

    void tick_order_ring(float dt) override {
        bsp::tick_unit_order_ring_00813020(*ring, dt, session_mode);
        state->throttle = ring->current_param_a;
        state->to_turn = ring->current_param_b;
        state->order_kind = ring->current_kind;
    }

    // 0078CF20, reconstructed: the wave field times the coverage mask. A flat sea is the
    // wave field disabled or its amplitude zero, which makes the product exactly 0.0f.
    // docs/OCEAN_HEIGHT.md.
    bsp::FlatSeaOceanHost sea{};
    float ocean_height(float x, float z) override {
        return bsp::ocean_water_height_0078cf20(x, z, sea);
    }

    // 008E6430, reconstructed: the product over the category-4 modifier list. Empty here,
    // so it is the 1.0f at 00D7A24C, which is also what 00826A06's clear globals give.
    float gameplay_scale() override {
        return bsp::gameplay_modifier_product_008e6430(nullptr, 0, no_modifiers);
    }
    struct NoModifiers final : bsp::GameplayModifierHost {
        bool entry_matches_008e4680(const bsp::GameplayModifierEntry&) override { return false; }
    } no_modifiers{};

    // 00826A6D dispatches the controller's force-model slot; 00937440 for a surface
    // ship. Its torque is computed so the probe can report it, but nothing applies it:
    // the hydrodynamic tail 009329C0 and the rigid-body solver are both external.
    bsp::OceanVec3 run_force_model(float dt) override {
        last_force_model_dt = dt;
        bsp::UnitSteeringTorqueInputs in{};
        in.velocity = state->linear_velocity;
        in.axis.x = state->pose_row2[0];
        in.axis.y = state->pose_row2[1];
        in.axis.z = state->pose_row2[2];
        in.reference_speed = reference_speed();
        in.hull_mass = cls.hull_mass;
        in.steering = state->to_turn;
        in.pose_row0_y = state->pose_row0[1];
        in.pose_row2.x = state->pose_row2[0];
        in.pose_row2.y = state->pose_row2[1];
        in.pose_row2.z = state->pose_row2[2];
        in.settings_rudder_torque = 0.0f; // settings+588h, not recovered
        return bsp::unit_steering_torque_00937440(in);
    }

    float reference_speed() override {
        return bsp::unit_reference_speed_0080fc30(state->max_speed,
                                                  bsp::kUnitReferenceSpeedUnscaled);
    }

    bsp::OceanVec3 body_linear_velocity() override { return state->linear_velocity; }

    bsp::ShipBodyBasis body_basis() override {
        bsp::ShipBodyBasis b{};
        for (int i = 0; i < 3; ++i) {
            b.row0[i] = state->pose_row0[i];
            b.row1[i] = state->pose_row1[i];
            b.row2[i] = state->pose_row2[i];
        }
        return b;
    }

    float forward_acceleration() override {
        // 00825EC0: class+508h scaled by unit+1038h against settings+228h. The boost is
        // zero here, so the settings field never enters.
        return bsp::unit_forward_acceleration_00825ec0(cls.retardation,
                                                       state->acceleration_boost, 0.0f);
    }

    void body_set_linear_velocity(const bsp::OceanVec3& v) override {
        state->linear_velocity = v;
    }

    bsp::OceanVec3 body_angular_velocity() override { return state->angular_velocity; }

    float yaw_rate_target(float smoothed_rudder) override {
        rudder->forward_speed = forward_speed();
        rudder->efficiency = state->turn_efficiency;
        rudder->steering = state->to_turn;
        // 0092E950 then the FCHS at 0092E955.
        const float rate = -bsp::unit_yaw_rate_00811890(smoothed_rudder, *rudder);
        // 0092E966: the propeller assist. settings+220h/+224h were not recovered; with a
        // zero propeller load the routine returns the rate unchanged whatever they are.
        last_yaw_rate = bsp::unit_propeller_turn_assist_00825de0(rate, state->propeller_load,
                                                                 0.0f, 0.0f);
        return last_yaw_rate;
    }

    // A destroyer's IsKindOf answers {0,1,2,4,5,6,7} plus its class id, so 0Eh is false.
    bool unit_trait_0e() override { return false; }

    void body_set_angular_velocity(const bsp::OceanVec3& w) override {
        state->angular_velocity = w;
    }

    // 0092BE80 is the controller step, owned by another packet; the unit virtual at
    // +1ECh was never read. Neither is modelled here.
    void controller_step(float) override {}
    void unit_post_motion(float) override {}

    float forward_speed() const {
        // 0092D730's rule, already reconstructed: the body axis flattened, dotted with
        // the linear velocity.
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = state->linear_velocity.x;
        in.velocity[1] = state->linear_velocity.y;
        in.velocity[2] = state->linear_velocity.z;
        in.axis[0] = state->pose_row2[0];
        in.axis[1] = state->pose_row2[1];
        in.axis[2] = state->pose_row2[2];
        return bsp::unit_forward_speed_0092d730(in);
    }
};

// The probe's stand-in for unit->vtable[50h], the heading getter the MT_SHIP_SYNC
// builder quantizes over +-pi (docs/UNIT_STATE_MESSAGE.md) and the one 00835E46
// hands to the cruise latch. The native body is not reconstructed; this is the
// same forward-axis angle the probe already prints.
float heading_radians(const bsp::ShipMotionState& state) {
    return static_cast<float>(std::atan2(static_cast<double>(state.pose_row2[0]),
                                         static_cast<double>(state.pose_row2[2])));
}

float heading_degrees(const bsp::ShipMotionState& state) {
    return static_cast<float>(static_cast<double>(heading_radians(state)) * 180.0 /
                              3.14159265358979323846);
}

// -------------------------------------------------------------------------
// --moveto: the reconstructed ship AI chain
// -------------------------------------------------------------------------
// The host boundaries the chain needs here. Every method that returns a value
// the probe cannot supply returns an explicit stand-in and is named in the run
// report; none of them feeds the trajectory, only the published slot.

struct MoveToSetterHost final : bsp::ShipAiSetterHost {
    void on_steering_mode_change_009da4e0() override {}    // 009DA4E0 unread
    void after_heading_stored_00605070(float) override {}  // 00605070 unread
};

struct MoveToDirectHost final : bsp::ShipAiDirectControlHost {
    float speed{0.0f};
    float heading{0.0f};
    void prologue_0080e000(float) override {}              // 0080E000 unread
    bool controller_belongs_to_another_007788b0() override { return false; }
    float unit_body_axis_speed_0092d730() override { return speed; }
    // Stand-ins: this probe has no value for the class field at +508h of
    // [unit+538h] or for unit+9C8h. They only scale blk+32Ch on a stopped ship.
    float ship_class_field_0508() override { return 1.0f; }
    float unit_field_09c8() override { return 0.0f; }
    float unit_heading_vtable_0050() override { return heading; }
    float unit_current_yaw_rate_00811940() override { return 0.0f; }
};

struct MoveToHeadingHost final : bsp::UnitHeadingTargetHost {
    float heading{0.0f};
    float speed{0.0f};
    float heading_virtual_0050() override { return heading; }
    float forward_speed_0092d730() override { return speed; }
};

// The navigation arm's two host boundaries, 009EE6AC and 009EE8C7. The probe
// steers at a single point rather than a path, so the remaining path length is
// the distance to that point, which is what 009D9E50 returns for a one-leg path.
struct MoveToNavHost final : bsp::ShipAiNavHost {
    float path_length{0.0f};
    float heading{0.0f};
    float remaining_path_length_009d9e50() override { return path_length; }
    float unit_heading_vtable_0050() override { return heading; }
};

struct MoveToPublishHost final : bsp::ShipAiPublishHost {
    int index{0};
    MoveToHeadingHost heading_host{};
    int order_slot_index_0b40() override { return index; }
    void set_heading_target_00811960(bsp::UnitHeadingTargetState& state,
                                    float desired_heading) override {
        bsp::unit_set_heading_target_00811960(state, desired_heading, heading_host);
    }
    void tail_009f0100(float) override {}   // 009F0100 unread
    void tail_009ef350() override {}        // 009EF350 unread
    void tail_009ef910(float) override {}   // 009EF910 unread
};

// -------------------------------------------------------------------------
// One trajectory
// -------------------------------------------------------------------------

// The body and the world one run integrates with.
struct HullBodySetup {
    const char* label{""};
    bsp::DynMotionState motion{};
    bsp::DynBody body{};
    bsp::DynWorldStepConstants world{};
    // The two fields 00C5C540's accumulator loop reads, world+00h and world+34h. Both
    // setups take the shipped values from 004DDB90, because the stand-in below stands in
    // for the BODY, not for the world's step size; giving it a zero substep would make
    // the schedule take no substep at all and the comparison meaningless.
    bsp::DynWorldSettings settings{};
};

// 00C5C540's schedule driven over the one hull body this probe owns.
//
// The probe sails in open water, so the rest of 00C5BB30 is inert: the collision pass
// finds no manifolds, 00C4B610 forms no groups, the solver dispatch is skipped for a
// zero group count, world+24h holds no contact listener and 00C4B550 sleeps nothing.
// What is left of the substep is exactly the two integration phases, which is what
// dyn_body_substep runs. The counters record what the schedule actually did rather than
// assuming it.
struct ProbeSimulateHost final : bsp::DynSimulateHost {
    bsp::DynBody* body{nullptr};
    const bsp::DynWorldStepConstants* world{nullptr};
    bsp::DynPreviousTransform previous{};
    int substeps{0};
    int removal_flushes{0};
    int counter_resets{0};

    void reset_profiler_counter_tree_00c321b0() override { counter_resets += 1; }
    void push_profiler_scope(const bsp::DynProfilerScopeSlot&) override {}
    void pop_profiler_scope(const bsp::DynProfilerScopeSlot&) override {}
    void flush_pending_body_removals_00c4d980() override { removal_flushes += 1; }

    bsp::DynRegisteredBody first_registered_body() override {
        bsp::DynRegisteredBody entry{};
        entry.body = body;
        entry.previous = &previous;
        return entry;
    }
    // One body, so the next step is the sentinel at world+208h.
    bsp::DynRegisteredBody next_registered_body(bsp::DynBody*) override {
        return bsp::DynRegisteredBody{};
    }

    void run_substep_00c5bb30(float dt) override {
        substeps += 1;
        bsp::dyn_body_substep(*body, *world, dt);
    }
};

// The hand-built stand-in every earlier version of this probe carried: no mass, no
// inertia, no damping, no gravity, and two clamps that never fire. Kept so the change the
// real body makes can be measured instead of asserted.
HullBodySetup stand_in_body() {
    HullBodySetup setup{};
    setup.label = "hand-built stand-in";
    setup.motion.max_linear_speed = 1.0e30f;   // M+18h, a clamp that never fires
    setup.motion.max_angular_speed = 1.0e30f;  // M+1Ch, likewise
    setup.body.motion = &setup.motion;
    setup.settings = bsp::dyn_world_settings_game();
    return setup;  // world: gravity and both sleep thresholds zero
}

// The body 00937C90's tail creates, in the world 004DDB90 constructs.
// `extent` is the hull's collision AABB span, whose producer (the shape attach 00C5C940)
// this packet does not read; a zero span gives a zero inertia, which 00C37E70 turns into
// a zero inverse inertia, i.e. no angular response to torque.
HullBodySetup class_body(const LuaVehicleClass& c, const bsp::OceanVec3& extent) {
    HullBodySetup setup{};
    setup.label = "class-built hull body";

    bsp::ShipHullBodyInputs in{};
    in.mass = c.mass;  // class+B0h
    // 00937CFD asks the unit whether it is category 8; a destroyer answers no, so the
    // material is Ship or TBoat by mass alone.
    in.unit_category_8 = false;
    in.aabb_max = extent;  // aabb_min stays zero, so the span is `extent`
    bsp::ship_hull_body_create_00937c90(in, setup.body, setup.motion);
    setup.body.motion = &setup.motion;

    setup.settings = bsp::dyn_world_settings_game();
    setup.world = bsp::dyn_world_step_constants(setup.settings);
    return setup;
}

struct TrajectoryInputs {
    const LuaVehicleClass* cls{nullptr};
    int steps{0};
    float dt{0.0f};
    float throttle{0.0f};
    float rudder{0.0f};
    // --cruise: run the unit under the reconstructed `Cruise` order instead of
    // a hand order re-issued every step. docs/CRUISE_COMMAND.md.
    bool cruise{false};
    int cruise_frame{20};
    // --commanded-speed: what luaMW_SetShipSpeed (00890D30) or
    // luaMW_NavigatorMoveOnPath (008A3600) would have written into the navigator
    // parameter block at *(unit+73Ch) +24h/+28h before the latch frame.
    // docs/UNIT_COMMANDED_SPEED.md.
    bool have_commanded_speed{false};
    float commanded_speed{0.0f};
    // --moveto X,Z: drive the unit through the reconstructed ship AI chain of
    // docs/SHIP_AI_STATES.md instead of a standing hand order. The two
    // stand-ins the run needs are named in the report it prints.
    bool moveto{false};
    float moveto_x{0.0f};
    float moveto_z{0.0f};
};

struct TrajectoryResult {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float heading{0.0f};
    float final_speed{0.0f};
    float peak_speed{0.0f};
    float peak_yaw{0.0f};
    float reference{0.0f};
    // Filled only under --cruise: what 00835AC0 latched at the latch step.
    bool cruise{false};
    bsp::CruiseAutopilotFields cruise_fields{};
    bsp::CruiseSteerMode cruise_mode{bsp::CruiseSteerMode::Rudder};
    // The commanded-speed pair as the cruise rule saw it, and the throttle the
    // override produced at the latch frame.
    bsp::CruiseSpeedSetting commanded{};
    float commanded_throttle{0.0f};
    // What 00C5C540's schedule did over the whole run: how many times it was called
    // (world+2Ch) and how many substeps of 00C5BB30 that came to.
    int schedule_calls{0};
    int substeps{0};
    // Filled only under --moveto.
    bool moveto{false};
    float moveto_first_distance{0.0f};
    float moveto_final_distance{0.0f};
    float moveto_min_distance{0.0f};
    int moveto_inside_step{-1};
    float moveto_published_heading{0.0f};
    float moveto_published_distance_40{0.0f};
    float moveto_published_distance_48{0.0f};
    int moveto_promotions{0};
    float moveto_bearing{0.0f};   // blk+324h after the navigation arm
    int moveto_turn_leads{0};     // steps on which the 009EE964 gate opened
    float moveto_arrival_time{-1.0f};
};

TrajectoryResult run_trajectory(const TrajectoryInputs& in, HullBodySetup& setup,
                                bool verbose) {
    const LuaVehicleClass& chosen = *in.cls;

    // --- the unit state -------------------------------------------------------
    bsp::ShipMotionState state{};
    state.max_speed = chosen.max_speed;  // 00822C20 seeds unit+9C0h from class+500h
    state.thrust_mod = 1.0f;             // 00823714
    state.turn_efficiency = 1.0f;        // 0082371C
    state.class_id = 7;                  // MDestroyer, docs/UNIT_INSTANCE_UPDATE.md
    state.position[1] = 0.0f;

    bsp::ShipMotionClass cls{};
    cls.max_accel = chosen.max_accel;
    cls.retardation = chosen.retardation;
    // class+B0h, +A0h and +A8h, now read from the installed table. The mass is read only
    // by 00937440, which this probe does not reach; the length and the height place the
    // keel sample point the throttle gate tests against the water height.
    cls.hull_mass = chosen.mass;
    cls.hull_length = chosen.length;
    cls.hull_height = chosen.height;
    cls.boost_refill_time = 1.0f;

    bsp::ShipClassFields fields{};
    fields.max_speed = chosen.max_speed;
    fields.max_rot_angle = chosen.max_rot_angle;
    fields.max_rot_angle_change_ratio = chosen.max_rot_angle_change_ratio;

    ProbeRudderHost rudder_host{};
    rudder_host.fields = fields;

    bsp::UnitOrderRing ring{};
    bsp::construct_unit_order_ring_00812d40(ring);

    ProbeMotionHost host{};
    host.state = &state;
    host.cls = cls;
    host.ring = &ring;
    host.rudder = &rudder_host;

    // --- the order ------------------------------------------------------------
    // 00816A40 builds the 20h-byte record through 00815440 (which clamps both
    // parameters into [-2,+2]) and publishes it into the slot the write cursor names.
    bsp::UnitOrderQueue queue{};
    queue.slot_index = ring.write_cursor;
    ProbeIssueHost issue{};
    bsp::UnitOrderRecordStorage scratch{};
    bsp::issue_unit_order_record_00816a40(issue, queue, scratch, in.throttle, in.rudder, 0);

    // The queue and the ring are two projections of the same native storage at unit+838h
    // (docs/UNIT_ORDER_RECORD.md and docs/UNIT_STATE_MESSAGE.md). Copy the slot the
    // publish wrote into the ring the tick reads. slot+08h is `active` in one projection
    // and `predicted` in the other, and 0080DAD0 clears it, marking the slot
    // authoritative.
    const int slot = queue.slot_index;
    ring.slot[slot].param_a = queue.slot[slot].param_a;
    ring.slot[slot].param_b = queue.slot[slot].param_b;
    ring.slot[slot].kind = queue.slot[slot].kind;
    ring.slot[slot].predicted = queue.slot_active[slot];

    bsp::DynMotionState& motion = setup.motion;
    bsp::DynBody& body = setup.body;
    body.motion = &motion;

    // The world's own per-step state: the accumulator at world+48h, which 00C5C540
    // leaves at zero on every path, and the call counter at world+2Ch.
    ProbeSimulateHost sim_host{};
    sim_host.body = &body;
    sim_host.world = &setup.world;
    float sim_accumulator = 0.0f;
    std::int32_t sim_step_counter = 0;

    // --- the run --------------------------------------------------------------
    TrajectoryResult result{};
    result.reference = host.reference_speed();
    if (verbose) {
        std::printf("  reference speed 0080FC30 = %.6f\n\n",
                    static_cast<double>(result.reference));
        std::printf("   step        t         x         y         z    heading   fwd_spd"
                    "   throttle    rudder   yaw_rate\n");
    }

    // --cruise state. The speed setting at *(unit+73Ch) +24h / +28h defaults to
    // disabled (-1.0f, the value 0081F278 constructs it with and 009E13F8 restores).
    // Under --commanded-speed the run stores the pair the two producers store,
    // 00890E6F..00890E97 and 008A38D5..008A3912: the clamped speed at +24h and the
    // mission clock DAT_00F876A4 at +28h, taken here as the run's own clock at the
    // latch frame. docs/UNIT_COMMANDED_SPEED.md.
    bsp::CruiseAutopilotFields cruise_fields{};
    bsp::CruiseSpeedSetting cruise_speed_setting{};

    float t = 0.0f;
    // --moveto state, carried across steps the way blk and the unit's two order
    // slots are. docs/SHIP_AI_STATES.md, docs/UNIT_AUTOPILOT_PAIR.md.
    bsp::ShipAiControlBlock blk{};
    bsp::ShipAiNavState nav{};
    // The two navigation fields the block is constructed with are not recovered:
    // blk+3C8h (the look-ahead ceiling 009ED769 seeds blk+340h from) and blk+3D0h
    // (the heading window the turn lead tolerates). The run uses 500.0f and a
    // tenth of a radian so the arm's two gates are both exercised, and the
    // report names them as run inputs, not as recovered values.
    // The slot pair lives on the unit, so the run uses the one inside
    // ShipMotionState: the AI publishes into it here and 00825F20's own head
    // promotes it inside ship_motion_step_00825f20.
    bsp::UnitAiOrderPromotion& promotion = state.ai_order;
    MoveToSetterHost setter_host{};
    MoveToNavHost nav_host{};
    MoveToDirectHost direct_host{};
    MoveToPublishHost publish_host{};
    result.moveto = in.moveto;

    for (int step = 0; step <= in.steps; ++step) {
        if (verbose && step % 10 == 0) {
            std::printf("%7d %8.2f %9.2f %9.2f %9.2f %10.3f %9.4f %10.4f %9.4f %10.5f\n",
                        step, static_cast<double>(t), static_cast<double>(state.position[0]),
                        static_cast<double>(state.position[1]),
                        static_cast<double>(state.position[2]),
                        static_cast<double>(heading_degrees(state)),
                        static_cast<double>(host.forward_speed()),
                        static_cast<double>(state.throttle),
                        static_cast<double>(state.smoothed_rudder),
                        static_cast<double>(state.angular_velocity.y));
        }
        if (step == in.steps) {
            break;
        }

        // Re-fill the slot under the write cursor so the order keeps standing: the game
        // does this from the HUD every frame the key is held, and the ring's own forward
        // copy at 00813186 would otherwise only ever mark slots predicted.
        //
        // Under --cruise the values are not the hand order after the latch step: the
        // authored `Cruise` captures the standing order once (00835E17..00835E58, the
        // rule in cruise_command_begin_00835e17) and then re-applies it every step
        // (009E1265..009E13B1, cruise_ordered_values_009e1170). Only the rudder arm of
        // that rule reaches the ring here; the heading arm goes through 009E0040, the
        // ship AI's own heading controller, which this packet does not reconstruct, and
        // a ship already on its latched heading is steered with a zero rudder either
        // way. docs/CRUISE_COMMAND.md says so and lists the limit.
        float order_a = in.throttle;
        float order_b = in.rudder;
        if (in.cruise && step >= in.cruise_frame) {
            if (step == in.cruise_frame) {
                cruise_fields = bsp::cruise_command_begin_00835e17(ring, heading_radians(state));
                result.cruise = true;
                result.cruise_fields = cruise_fields;
                if (in.have_commanded_speed) {
                    cruise_speed_setting = bsp::navigator_commanded_speed_store_00890e6f(
                        in.commanded_speed, t);
                }
                result.commanded = cruise_speed_setting;
            }
            const bsp::CruiseOrderedValues ordered = bsp::cruise_ordered_values_009e1170(
                cruise_fields, cruise_speed_setting, result.reference, host.forward_speed());
            result.cruise_mode = ordered.mode;
            if (step == in.cruise_frame) {
                result.commanded_throttle = ordered.throttle;
            }
            order_a = ordered.throttle;
            order_b = (ordered.mode == bsp::CruiseSteerMode::Rudder) ? ordered.steer_or_heading
                                                                    : 0.0f;
        }
        if (in.moveto) {
            // 1. The goal. A `movetopos` state reads it from brain+0B2Ch and
            //    brain+0B34h (009E57D0, 009E57B4); here it is the argument.
            const float dx = in.moveto_x - state.position[0];
            const float dz = in.moveto_z - state.position[2];
            const std::array<float, 2> delta{dx, dz};
            const float distance = bsp::length_2d_00414c60(delta);
            if (step == 0) {
                result.moveto_first_distance = distance;
                result.moveto_min_distance = distance;
            } else if (distance < result.moveto_min_distance) {
                result.moveto_min_distance = distance;
            }
            if (result.moveto_inside_step < 0 && distance < bsp::kShipAiLookAheadBonus) {
                result.moveto_inside_step = step;
                result.moveto_arrival_time = t;
            }
            if (step == 0) {
                // Run inputs, not recovered values: see the note by `nav`.
                nav.look_ahead_max_3c8 = 500.0f;
                nav.look_ahead_340 = 500.0f;
                nav.turn_window_3d0 = 0.1f;
                nav.hull_axis_19c = std::array<float, 2>{state.pose_row2[0], state.pose_row2[2]};
            } else {
                nav.hull_axis_19c = std::array<float, 2>{state.pose_row2[0], state.pose_row2[2]};
            }
            // 2. The AI setters and the direct-control arm, reconstructed. The
            //    throttle goes in through 009DBF90; the heading setter is what
            //    puts blk+1C4h in Heading, which is the mode the direct-control
            //    arm needs to leave blk+324h alone for the navigation arm.
            bsp::ship_ai_set_desired_throttle_009dbf90(blk, in.throttle);
            bsp::ship_ai_set_desired_heading_009e0040(blk, 0.0f, setter_host);
            direct_host.speed = host.forward_speed();
            direct_host.heading = heading_radians(state);
            bsp::ship_ai_direct_control_arm_009ed6b0(blk, in.dt, direct_host);
            // 3. The navigation arm, 009EE671..009EEAA2, reconstructed here.
            //    Its gate is blk+1C4h == Navigate, and no site writing that mode
            //    was read in the image, so the probe sets it directly and says
            //    so. docs/SHIP_AI_NAVIGATION_ARM.md.
            //    It is what turns a point and the unit's pose into the bearing
            //    at blk+324h, the distance at blk+32Ch and the path length at
            //    blk+330h - the exact trio 009F4D10 publishes. The probe drives
            //    it with a one-leg path, so `more_path` is false and the next
            //    leg degenerates to the goal itself.
            blk.mode = bsp::ShipAiSteeringMode::Navigate;
            nav_host.path_length = distance;
            nav_host.heading = heading_radians(state);
            bsp::ShipAiNavWaypoint waypoint{};
            waypoint.x = in.moveto_x;
            waypoint.z = in.moveto_z;
            waypoint.next_x = in.moveto_x;
            waypoint.next_z = in.moveto_z;
            waypoint.more_path = false;
            waypoint.steer_enabled = true;
            waypoint.side = bsp::ShipAiNavTurnSide::Unconstrained;
            bsp::ShipAiNavPose nav_pose{};
            nav_pose.x = state.position[0];
            nav_pose.z = state.position[2];
            const bsp::ShipAiNavResult navigated =
                bsp::ship_ai_navigation_arm_009ee671(blk, nav, waypoint, nav_pose, nav_host);
            result.moveto_bearing = navigated.heading_target;
            if (navigated.turn_lead_applied) {
                ++result.moveto_turn_leads;
            }
            // 4. 009F4D10 publishes into the unit's order slot; 00811960 limits
            //    the heading to a quarter turn about the unit's own heading.
            publish_host.index = promotion.index;
            publish_host.heading_host.heading = heading_radians(state);
            publish_host.heading_host.speed = host.forward_speed();
            const bsp::ShipAiPublishResult published = bsp::ship_ai_publish_order_009f4d10(
                blk.heading_target_324, blk.distance_32c, blk.distance_330, in.dt,
                publish_host);
            promotion.slots[promotion.index] = published.slot;
            result.moveto_published_heading = published.slot.heading_44;
            result.moveto_published_distance_40 = published.slot.distance_40;
            result.moveto_published_distance_48 = published.slot.distance_48;
            // 5. 00825F2C promotes it. That happens inside
            //    ship_motion_step_00825f20 below, at the head of the tick,
            //    exactly where 00825F2C sits in 00825F20.

            // 6. STAND-IN, not recovered, and now known not to exist in this
            //    shape: packet cc_ai_order_hop scanned every computation of a
            //    slot address in .text and found no reader of +40h, +44h or
            //    +48h on the unit's own motion path, so no native hop turns the
            //    published triple into the order ring. The probe keeps a
            //    proportional law over the +/-pi/4 window 00811960 clamps to so
            //    that --moveto still produces a trajectory. A positive rudder
            //    lowers the heading in this probe, hence the sign.
            //    docs/UNIT_AI_ORDER_SLOT_READER.md.
            const float error = bsp::wrapped_angle_subtract_00438b10(
                published.slot.heading_44, heading_radians(state));
            float steer = error / 0.785398185253143310546875f;
            if (steer > 1.0f) {
                steer = 1.0f;
            } else if (steer < -1.0f) {
                steer = -1.0f;
            }
            order_a = in.throttle;
            order_b = -steer;
        }
        queue.slot_index = ring.write_cursor;
        bsp::issue_unit_order_record_00816a40(issue, queue, scratch, order_a, order_b, 0);
        const int w = ring.write_cursor;
        ring.slot[w].param_a = queue.slot[w].param_a;
        ring.slot[w].param_b = queue.slot[w].param_b;
        ring.slot[w].kind = queue.slot[w].kind;
        ring.slot[w].predicted = queue.slot_active[w];

        const bsp::ShipMotionStepResult motion_step =
            bsp::ship_motion_step_00825f20(state, cls, host, in.dt);
        if (motion_step.ai_order_promoted) {
            ++result.moveto_promotions;
        }

        // The game's own integrator, and now the game's own schedule around it: the
        // whole of 00C5C540, which resets the profiler counters, flushes the pending
        // removals, publishes every body's previous transform into M+84h and then runs
        // 00C5BB30 as many times as its accumulator and budget allow. With world+00h =
        // 0.05f and world+34h = 1 that is one substep of the whole game step, so the
        // trajectory is unchanged against the direct dyn_body_substep call this replaced;
        // the schedule is here so the probe exercises the rule rather than its result.
        // docs/DYN_PHYSICS_SUBSTEP.md, docs/DYN_WORLD_SETTINGS.md.
        motion.linear_velocity = state.linear_velocity;
        motion.angular_velocity = state.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            body.row0[i] = state.pose_row0[i];
            body.row1[i] = state.pose_row1[i];
            body.row2[i] = state.pose_row2[i];
            body.position[i] = state.position[i];
        }
        bsp::dyn_physics_world_simulate_00c5c540(setup.settings, sim_accumulator,
                                                 sim_step_counter, in.dt, sim_host);
        state.linear_velocity = motion.linear_velocity;
        state.angular_velocity = motion.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            state.pose_row0[i] = body.row0[i];
            state.pose_row1[i] = body.row1[i];
            state.pose_row2[i] = body.row2[i];
            state.position[i] = body.position[i];
        }
        t += in.dt;

        const float speed = host.forward_speed();
        if (speed > result.peak_speed) {
            result.peak_speed = speed;
        }
        const float yaw = std::fabs(state.angular_velocity.y);
        if (yaw > result.peak_yaw) {
            result.peak_yaw = yaw;
        }
    }

    if (in.moveto) {
        const std::array<float, 2> final_delta{in.moveto_x - state.position[0],
                                               in.moveto_z - state.position[2]};
        result.moveto_final_distance = bsp::length_2d_00414c60(final_delta);
    }
    result.x = state.position[0];
    result.y = state.position[1];
    result.z = state.position[2];
    result.heading = heading_degrees(state);
    result.final_speed = host.forward_speed();
    result.schedule_calls = sim_step_counter;
    result.substeps = sim_host.substeps;
    return result;
}

} // namespace

int main(int argc, char** argv) {
    std::string lua_path = kDefaultLuaPath;
    int want_index = -1;
    std::string want_type = "Destroyer";
    int steps = 400;
    float dt = bsp::kUnitStateMessageTickSeconds; // 0.05f, 00D0DE84
    float throttle = 1.0f;
    float rudder = 1.0f;
    // --cruise: after --cruise-frame steps of the hand order, latch it the way
    // 00835E17 does and run the rest under 009E1170's rule. The default 20 is the
    // frame count docs/GAME_EXECUTABLE.md milestone 2j names for the stretch the
    // executable runs under the authored `Cruise` before the injected order.
    bool cruise = false;
    int cruise_frame = 20;
    bool moveto = false;
    float moveto_x = 0.0f;
    float moveto_z = 0.0f;
    // --commanded-speed: the metres per second a Lua `SetShipSpeed` or
    // `NavigatorMoveOnPath` order would have put on the navigator parameter block,
    // which 009E1265's arm turns into a throttle of speed / reference.
    bool have_commanded_speed = false;
    float commanded_speed = 0.0f;
    // The hull's collision AABB span. Its producer is the shape attach 00C5C940, which
    // this packet does not read, so it defaults to zero rather than to an invented box.
    bsp::OceanVec3 hull_extent{};

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool has_next = (i + 1) < argc;
        if (arg == "--lua" && has_next) {
            lua_path = argv[++i];
        } else if (arg == "--class" && has_next) {
            want_index = std::atoi(argv[++i]);
        } else if (arg == "--type" && has_next) {
            want_type = argv[++i];
        } else if (arg == "--steps" && has_next) {
            steps = std::atoi(argv[++i]);
        } else if (arg == "--dt" && has_next) {
            dt = static_cast<float>(std::atof(argv[++i]));
        } else if (arg == "--throttle" && has_next) {
            throttle = static_cast<float>(std::atof(argv[++i]));
        } else if (arg == "--rudder" && has_next) {
            rudder = static_cast<float>(std::atof(argv[++i]));
        } else if (arg == "--moveto" && has_next) {
            const std::string value = argv[++i];
            const std::string::size_type comma = value.find(',');
            if (comma == std::string::npos) {
                std::printf("ship motion probe: --moveto wants X,Z\n");
                return 2;
            }
            moveto_x = static_cast<float>(std::atof(value.substr(0, comma).c_str()));
            moveto_z = static_cast<float>(std::atof(value.substr(comma + 1).c_str()));
            moveto = true;
        } else if (arg == "--cruise") {
            cruise = true;
        } else if (arg == "--cruise-frame" && has_next) {
            cruise_frame = std::atoi(argv[++i]);
        } else if (arg == "--commanded-speed" && has_next) {
            commanded_speed = static_cast<float>(std::atof(argv[++i]));
            have_commanded_speed = true;
        } else if (arg == "--hull-extent" && (i + 3) < argc) {
            hull_extent.x = static_cast<float>(std::atof(argv[++i]));
            hull_extent.y = static_cast<float>(std::atof(argv[++i]));
            hull_extent.z = static_cast<float>(std::atof(argv[++i]));
        } else {
            std::printf("usage: %s [--lua <vehicleclasses.lua>] [--class N] [--type T]\n"
                        "          [--steps N] [--dt S] [--throttle X] [--rudder X]\n"
                        "          [--moveto X,Z]\n"
                        "          [--cruise] [--cruise-frame N]"
                        " [--commanded-speed M_PER_S]\n"
                        "          [--hull-extent DX DY DZ]\n",
                        argv[0]);
            return 2;
        }
    }

    std::vector<LuaVehicleClass> classes;
    std::string error;
    if (!load_vehicle_classes(lua_path, classes, error)) {
        std::printf("ship motion probe: %s\n", error.c_str());
        return 1;
    }

    const LuaVehicleClass* chosen = nullptr;
    for (const LuaVehicleClass& c : classes) {
        if (want_index >= 0) {
            if (c.index == want_index) {
                chosen = &c;
                break;
            }
            continue;
        }
        if (c.type == want_type && c.have_max_speed && c.max_speed > 0.0f &&
            c.max_rot_angle > 0.0f) {
            chosen = &c;
            break;
        }
    }
    if (chosen == nullptr) {
        std::printf("ship motion probe: no class matched (%d classes read from %s)\n",
                    static_cast<int>(classes.size()), lua_path.c_str());
        return 1;
    }

    std::printf("ship motion probe\n");
    std::printf("  class        VehicleClass[%d] %s (%s)\n", chosen->index,
                chosen->name.empty() ? chosen->comment.c_str() : chosen->name.c_str(),
                chosen->type.c_str());
    std::printf("  MaxSpeed     %.6f   (class+500h)\n", static_cast<double>(chosen->max_speed));
    std::printf("  MaxAccel     %.6f   (class+504h)\n", static_cast<double>(chosen->max_accel));
    std::printf("  Retardation  %.6f   (class+508h)\n",
                static_cast<double>(chosen->retardation));
    std::printf("  MaxRotAngle  %.6f rad/s (class+4F8h)\n",
                static_cast<double>(chosen->max_rot_angle));
    std::printf("  Mass         %.4f      (class+B0h), Length %.2f (class+A0h),"
                " Height %.2f (class+A8h)\n",
                static_cast<double>(chosen->mass), static_cast<double>(chosen->length),
                static_cast<double>(chosen->height));
    std::printf("  step         %.4f s, %d steps\n", static_cast<double>(dt), steps);
    std::printf("  order        throttle %.3f, rudder %.3f, through 00816A40\n",
                static_cast<double>(throttle), static_cast<double>(rudder));
    if (cruise) {
        std::printf("  cruise       on: the hand order stands for %d steps, then the latch"
                    " 00835E17\n"
                    "               captures it into cruiseIsHeading /"
                    " cruiseSteerOrHeading /\n"
                    "               cruiseThrust and 009E1170 re-applies it every step."
                    " The commanded\n"
                    "               speed at *(unit+73Ch)+24h/+28h is %s.\n",
                    cruise_frame,
                    have_commanded_speed ? "set at that frame" : "left at -1.0f (inactive)");
    }
    if (have_commanded_speed) {
        const bsp::CruiseSpeedSetting stored =
            bsp::navigator_commanded_speed_store_00890e6f(commanded_speed, 0.0f);
        std::printf("  cmd speed    %.4f m/s requested -> +24h %.4f, +28h the mission clock\n"
                    "               (00890E6F..00890E97 in luaMW_SetShipSpeed, the same store"
                    " as\n"
                    "               008A38D5..008A3912 in luaMW_NavigatorMoveOnPath)\n",
                    static_cast<double>(commanded_speed),
                    static_cast<double>(stored.speed));
    }
    std::printf("  inputs       ocean 0078CF20 reconstructed, flat sea (wave field off -> 0.0)\n"
                "               gameplay scale 008E6430 reconstructed, empty list -> 1.0\n"
                "               rudder curve +438h..+44Ch from shipglobals.lua"
                " {0,0.4} {0.5,1.5} {1,2}\n"
                "               integration 00C41550 + 00C5B1B0, the game's own routines\n"
                "               hull body 00937C90 tail, world 004DDB90: world+00h is"
                " %.4f s and\n"
                "               the budget at world+34h is 1, so 00C5C540 takes exactly one\n"
                "               substep of the whole game step\n"
                "  left open    the force path into the body (009329C0's hydrodynamics are\n"
                "               not reconstructed, so nothing pushes force and nothing\n"
                "               cancels gravity), and the hull collision AABB, whose\n"
                "               producer 00C5C940 is unread; --hull-extent supplies it\n\n",
                static_cast<double>(bsp::kDynWorldFixedSubstep));

    // --- the two runs ---------------------------------------------------------
    const bsp::DynWorldSettings game_world = bsp::dyn_world_settings_game();
    TrajectoryInputs run_in{};
    run_in.cls = chosen;
    run_in.steps = steps;
    run_in.dt = dt;
    run_in.throttle = throttle;
    run_in.rudder = rudder;
    run_in.cruise = cruise;
    run_in.cruise_frame = cruise_frame;
    run_in.have_commanded_speed = have_commanded_speed;
    run_in.commanded_speed = commanded_speed;
    run_in.moveto = moveto;
    run_in.moveto_x = moveto_x;
    run_in.moveto_z = moveto_z;

    HullBodySetup stand_in = stand_in_body();
    HullBodySetup real = class_body(*chosen, hull_extent);

    std::printf("  body A       %s\n", stand_in.label);
    std::printf("  body B       %s: mass %.1f, 1/mass %.8f, inverse inertia"
                " (%.8f %.8f %.8f),\n"
                "               angular damping %.3f, clamps %.1f / %.1f,"
                " row-1 torque lock %s\n",
                real.label, static_cast<double>(chosen->mass),
                static_cast<double>(real.motion.inverse_mass),
                static_cast<double>(real.motion.inverse_inertia_body.x),
                static_cast<double>(real.motion.inverse_inertia_body.y),
                static_cast<double>(real.motion.inverse_inertia_body.z),
                static_cast<double>(real.motion.angular_damping),
                static_cast<double>(real.motion.max_linear_speed),
                static_cast<double>(real.motion.max_angular_speed),
                real.motion.lock_torque_to_row1 ? "on" : "off");
    std::printf("  world B      substep %.4f s, budget %d, gravity (%.1f %.1f %.1f),"
                " sleep %.1f / %.1f / %d\n\n",
                static_cast<double>(game_world.fixed_substep), game_world.substep_budget,
                static_cast<double>(game_world.gravity.x),
                static_cast<double>(game_world.gravity.y),
                static_cast<double>(game_world.gravity.z),
                static_cast<double>(game_world.sleep_linear_speed),
                static_cast<double>(game_world.sleep_angular_speed),
                game_world.sleep_countdown_reload);

    std::printf("A: %s\n", stand_in.label);
    const TrajectoryResult a = run_trajectory(run_in, stand_in, true);
    std::printf("\nB: %s\n", real.label);
    const TrajectoryResult b = run_trajectory(run_in, real, true);

    std::printf("\n  final state        A (%s)        B (%s)\n", stand_in.label, real.label);
    std::printf("  x                %12.4f %22.4f\n", static_cast<double>(a.x),
                static_cast<double>(b.x));
    std::printf("  y                %12.4f %22.4f\n", static_cast<double>(a.y),
                static_cast<double>(b.y));
    std::printf("  z                %12.4f %22.4f\n", static_cast<double>(a.z),
                static_cast<double>(b.z));
    std::printf("  heading (deg)    %12.4f %22.4f\n", static_cast<double>(a.heading),
                static_cast<double>(b.heading));
    std::printf("  final fwd speed  %12.4f %22.4f\n", static_cast<double>(a.final_speed),
                static_cast<double>(b.final_speed));
    std::printf("  peak fwd speed   %12.4f %22.4f\n", static_cast<double>(a.peak_speed),
                static_cast<double>(b.peak_speed));
    std::printf("  peak yaw (rad/s) %12.5f %22.5f\n", static_cast<double>(a.peak_yaw),
                static_cast<double>(b.peak_yaw));
    std::printf("  00C5C540 calls   %12d %22d\n", a.schedule_calls, b.schedule_calls);
    std::printf("  00C5BB30 substeps%12d %22d\n", a.substeps, b.substeps);
    std::printf("  B's y falls because 00C41550 adds the world's gravity and nothing here\n"
                "  cancels it: 009329C0's buoyancy is not reconstructed.\n");

    if (b.moveto) {
        std::printf("\n  --moveto %.1f,%.1f through the reconstructed ship AI chain\n"
                    "    009DBF90 / 009E0040 set blk+1D0h and blk+1D8h; 009ED6B0's\n"
                    "    direct-control arm copies the held heading into blk+324h;\n"
                    "    009EE671..009EEAA2, its navigation arm, writes the bearing\n"
                    "    into blk+324h, the distance into blk+32Ch and the path\n"
                    "    length into blk+330h; 009F4D10 publishes that trio through\n"
                    "    00811960 into the unit's order slot; 00825F2C promotes it.\n"
                    "    distance to the goal  start %.2f, minimum %.2f, final %.2f\n"
                    "    first step inside %.1f (00CF0DD8): %s\n"
                    "    last published slot   +44h %.6f rad, +40h %.4f, +48h %.4f\n"
                    "    slot promotions       %d of %d steps\n"
                    "    nav arm bearing       %.6f rad, turn leads applied %d\n",
                    static_cast<double>(run_in.moveto_x), static_cast<double>(run_in.moveto_z),
                    static_cast<double>(b.moveto_first_distance),
                    static_cast<double>(b.moveto_min_distance),
                    static_cast<double>(b.moveto_final_distance),
                    static_cast<double>(bsp::kShipAiLookAheadBonus),
                    (b.moveto_inside_step >= 0) ? "yes" : "no",
                    static_cast<double>(b.moveto_published_heading),
                    static_cast<double>(b.moveto_published_distance_40),
                    static_cast<double>(b.moveto_published_distance_48),
                    b.moveto_promotions, run_in.steps,
                    static_cast<double>(b.moveto_bearing), b.moveto_turn_leads);
        if (b.moveto_inside_step >= 0) {
            std::printf("    reached that range at step %d, %.2f s\n", b.moveto_inside_step,
                        static_cast<double>(b.moveto_arrival_time));
        }
        std::printf("    one stand-in carries this run: the slot-to-rudder hop.\n"
                    "    Packet cc_ai_order_hop scanned every slot-address\n"
                    "    computation in .text and found no reader of slot +40h,\n"
                    "    +44h or +48h on the unit's own motion path, so the\n"
                    "    trajectory below is evidence about the published slot and\n"
                    "    about the reconstructed motion, not about the shipped\n"
                    "    steering law. blk+3C8h and blk+3D0h are run inputs.\n");
    }

    if (b.cruise) {
        static const char* const kModeName[] = {"rudder", "heading", "straight"};
        std::printf("\n  cruise latch at step %d (00835AC0 through 00835E17)\n"
                    "    cruiseIsHeading       %s   (|ordered rudder| < 0.01)\n"
                    "    cruiseSteerOrHeading  %.6f  (%s)\n"
                    "    cruiseThrust          %.6f  (the ring's +148h at the latch)\n"
                    "    steering arm 009E1170 takes: %s\n",
                    run_in.cruise_frame, b.cruise_fields.is_heading ? "true " : "false",
                    static_cast<double>(b.cruise_fields.steer_or_heading),
                    b.cruise_fields.is_heading ? "radians of heading" : "the ordered rudder",
                    static_cast<double>(b.cruise_fields.thrust),
                    kModeName[static_cast<int>(b.cruise_mode)]);
        const bool active = bsp::navigator_commanded_speed_active(b.commanded);
        std::printf("    commanded speed       +24h %.6f, +28h %.6f -> %s\n"
                    "    throttle at the latch frame %.6f  (%s)\n",
                    static_cast<double>(b.commanded.speed),
                    static_cast<double>(b.commanded.enable),
                    active ? "active, 009E12BD overrides cruiseThrust"
                           : "inactive, 009E12AC keeps cruiseThrust",
                    static_cast<double>(b.commanded_throttle),
                    active ? "+24h / the reference speed 0080FC30"
                           : "cruiseThrust straight from the latch");
    }

    const TrajectoryResult& result = b;
    const float reference = result.reference;
    const float peak_speed = result.peak_speed;
    const float peak_yaw = result.peak_yaw;
    const float final_speed = result.final_speed;

    // --- the two checks a differential run would make --------------------------
    const float peak_ratio = (reference > 0.0f) ? (peak_speed / reference) : 0.0f;
    const float rate_at_full = chosen->max_rot_angle;

    std::printf("\n  peak forward speed  %.4f, class reference %.4f, ratio %.4f\n",
                static_cast<double>(peak_speed), static_cast<double>(reference),
                static_cast<double>(peak_ratio));
    std::printf("  final forward speed %.4f (a sustained turn bleeds speed: 0092D300 rewrites\n"
                "                            only the axial component, so the lateral one stays)\n",
                static_cast<double>(final_speed));
    std::printf("  speed approaches the class maximum: %s\n",
                (peak_ratio > 0.99f && peak_ratio < 1.01f) ? "yes" : "no");
    std::printf("  peak yaw rate %.5f rad/s, class MaxRotAngle %.5f rad/s, ratio %.4f\n",
                static_cast<double>(peak_yaw), static_cast<double>(rate_at_full),
                static_cast<double>((rate_at_full > 0.0f) ? peak_yaw / rate_at_full : 0.0f));
    // With the shipped curve the denominator at full throttle is 2.0, so the expected
    // steady yaw rate at a hard-over rudder and full speed is MaxRotAngle / 2, not
    // MaxRotAngle. 0082ECB0 is (MaxRotAngle / denominator) * speedRatio * rudder *
    // efficiency, and the steering slew at 0092EAAD reaches that target within 2 dt.
    const float expected_yaw =
        rate_at_full /
        bsp::kShippedTurnMultipliers.max_speed.turn_circle_multiplier;
    std::printf("  expected yaw rate with the shipped curve %.5f rad/s"
                " (MaxRotAngle / %.2f)\n",
                static_cast<double>(expected_yaw),
                static_cast<double>(
                    bsp::kShippedTurnMultipliers.max_speed.turn_circle_multiplier));
    std::printf("  heading turns at the curve's rate: %s\n",
                (expected_yaw > 0.0f && peak_yaw > 0.99f * expected_yaw &&
                 peak_yaw < 1.01f * expected_yaw)
                    ? "yes"
                    : "no");
    // The real body answers why not. 00937C90 gives the hull an angular damping rate of
    // 1.0f (00939A2F) and each phase of the substep multiplies the angular velocity by
    // 1 - rate*dt, so the steering target set at 0092EB8F is scaled by (1 - dt)^2 inside
    // the same step. docs/SHIP_HULL_BODY.md.
    const float damp_once = 1.0f - bsp::kShipHullAngularDamping * dt;
    const float damped = damp_once * damp_once;
    std::printf("  the hull's angular damping (rate %.1f, twice per substep) scales that\n"
                "  target by %.4f, giving %.5f rad/s: measured %.5f\n",
                static_cast<double>(bsp::kShipHullAngularDamping),
                static_cast<double>(damped), static_cast<double>(expected_yaw * damped),
                static_cast<double>(peak_yaw));
    return 0;
}
