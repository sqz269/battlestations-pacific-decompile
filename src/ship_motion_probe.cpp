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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

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

float heading_degrees(const bsp::ShipMotionState& state) {
    return static_cast<float>(std::atan2(static_cast<double>(state.pose_row2[0]),
                                         static_cast<double>(state.pose_row2[2])) *
                              180.0 / 3.14159265358979323846);
}

// -------------------------------------------------------------------------
// One trajectory
// -------------------------------------------------------------------------

// The body and the world one run integrates with.
struct HullBodySetup {
    const char* label{""};
    bsp::DynMotionState motion{};
    bsp::DynBody body{};
    bsp::DynWorldStepConstants world{};
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

    setup.world = bsp::dyn_world_step_constants(bsp::dyn_world_settings_game());
    return setup;
}

struct TrajectoryInputs {
    const LuaVehicleClass* cls{nullptr};
    int steps{0};
    float dt{0.0f};
    float throttle{0.0f};
    float rudder{0.0f};
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

    // --- the run --------------------------------------------------------------
    TrajectoryResult result{};
    result.reference = host.reference_speed();
    if (verbose) {
        std::printf("  reference speed 0080FC30 = %.6f\n\n",
                    static_cast<double>(result.reference));
        std::printf("   step        t         x         y         z    heading   fwd_spd"
                    "   throttle    rudder   yaw_rate\n");
    }

    float t = 0.0f;
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
        queue.slot_index = ring.write_cursor;
        bsp::issue_unit_order_record_00816a40(issue, queue, scratch, in.throttle, in.rudder, 0);
        const int w = ring.write_cursor;
        ring.slot[w].param_a = queue.slot[w].param_a;
        ring.slot[w].param_b = queue.slot[w].param_b;
        ring.slot[w].kind = queue.slot[w].kind;
        ring.slot[w].predicted = queue.slot_active[w];

        bsp::ship_motion_step_00825f20(state, cls, host, in.dt);

        // The game's own integrator, not an Euler stand-in: 00C41550 then 00C5B1B0, the
        // two phases of one Dyn substep. 00C5C540's schedule takes exactly one substep of
        // the whole game step, because world+00h is the game step itself (00CE7638) and
        // the budget at world+34h is 1. docs/DYN_WORLD_SETTINGS.md.
        motion.linear_velocity = state.linear_velocity;
        motion.angular_velocity = state.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            body.row0[i] = state.pose_row0[i];
            body.row1[i] = state.pose_row1[i];
            body.row2[i] = state.pose_row2[i];
            body.position[i] = state.position[i];
        }
        bsp::dyn_body_substep(body, setup.world, in.dt);
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

    result.x = state.position[0];
    result.y = state.position[1];
    result.z = state.position[2];
    result.heading = heading_degrees(state);
    result.final_speed = host.forward_speed();
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
        } else if (arg == "--hull-extent" && (i + 3) < argc) {
            hull_extent.x = static_cast<float>(std::atof(argv[++i]));
            hull_extent.y = static_cast<float>(std::atof(argv[++i]));
            hull_extent.z = static_cast<float>(std::atof(argv[++i]));
        } else {
            std::printf("usage: %s [--lua <vehicleclasses.lua>] [--class N] [--type T]\n"
                        "          [--steps N] [--dt S] [--throttle X] [--rudder X]\n"
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
    std::printf("  B's y falls because 00C41550 adds the world's gravity and nothing here\n"
                "  cancels it: 009329C0's buoyancy is not reconstructed.\n");

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
