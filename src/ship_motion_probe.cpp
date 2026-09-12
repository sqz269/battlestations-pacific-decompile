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
// +438h..+44Ch, and the integrator itself are all reconstructed now.
//
// What is still open, and is printed in the header so no reader mistakes it for recovered
// behaviour: the hull body's mass, inertia and damping; the force path into that body
// (009329C0's hydrodynamics are not reconstructed, so nothing pushes force here); and the
// library's own substep size at world+00h, which decides how many substeps one 0.05 s
// game step takes.
//
// docs/SHIP_MOTION.md, docs/RIGID_BODY_INTEGRATION.md, docs/UNIT_RUDDER_CURVE.md,
// docs/OCEAN_HEIGHT.md, docs/CONTROLLED_UNIT.md.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "bsp/ship_class_fields.hpp"
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

} // namespace

int main(int argc, char** argv) {
    std::string lua_path = kDefaultLuaPath;
    int want_index = -1;
    std::string want_type = "Destroyer";
    int steps = 400;
    float dt = bsp::kUnitStateMessageTickSeconds; // 0.05f, 00D0DE84
    float throttle = 1.0f;
    float rudder = 1.0f;

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
        } else {
            std::printf("usage: %s [--lua <vehicleclasses.lua>] [--class N] [--type T]\n"
                        "          [--steps N] [--dt S] [--throttle X] [--rudder X]\n",
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
    std::printf("  step         %.4f s, %d steps\n", static_cast<double>(dt), steps);
    std::printf("  order        throttle %.3f, rudder %.3f, through 00816A40\n",
                static_cast<double>(throttle), static_cast<double>(rudder));
    std::printf("  inputs       ocean 0078CF20 reconstructed, flat sea (wave field off -> 0.0)\n"
                "               gameplay scale 008E6430 reconstructed, empty list -> 1.0\n"
                "               rudder curve +438h..+44Ch from shipglobals.lua"
                " {0,0.4} {0.5,1.5} {1,2}\n"
                "               integration 00C41550 + 00C5B1B0, the game's own routines\n"
                "  left open    the hull body's mass, inertia and damping; the force path\n"
                "               into the body; the library's substep size (world+00h), so\n"
                "               one substep of the full %.4f s game step is taken\n\n",
                static_cast<double>(dt));

    // --- the unit state -------------------------------------------------------
    bsp::ShipMotionState state{};
    state.max_speed = chosen->max_speed; // 00822C20 seeds unit+9C0h from class+500h
    state.thrust_mod = 1.0f;             // 00823714
    state.turn_efficiency = 1.0f;        // 0082371C
    state.class_id = 7;                  // MDestroyer, docs/UNIT_INSTANCE_UPDATE.md
    state.position[1] = 0.0f;

    bsp::ShipMotionClass cls{};
    cls.max_accel = chosen->max_accel;
    cls.retardation = chosen->retardation;
    cls.hull_mass = 0.0f;    // class+B0h, only 00937440 reads it
    cls.hull_length = 0.0f;  // class+A0h and +A8h place the keel sample point; with a flat
    cls.hull_height = 0.0f;  // sea at y=0 and an upright hull the gate passes either way
    cls.boost_refill_time = 1.0f;

    bsp::ShipClassFields fields{};
    fields.max_speed = chosen->max_speed;
    fields.max_rot_angle = chosen->max_rot_angle;
    fields.max_rot_angle_change_ratio = chosen->max_rot_angle_change_ratio;

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
    bsp::issue_unit_order_record_00816a40(issue, queue, scratch, throttle, rudder, 0);

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

    // --- the rigid body -------------------------------------------------------
    // The hull body's own mass, inertia and damping are not established (no ship-side
    // caller of 00C37F40, 00C37E70, 00C37E00 or 00C37DE0 was found), and the force path
    // into it is not reconstructed, so this body carries no force, no gravity and no
    // damping. Only the position and orientation update is exercised, which is exactly
    // the stand-in this probe used to supply by hand.
    bsp::DynMotionState motion{};
    motion.max_linear_speed = 1.0e30f;   // M+18h, unknown; a clamp that never fires
    motion.max_angular_speed = 1.0e30f;  // M+1Ch, likewise
    bsp::DynBody body{};
    body.motion = &motion;
    bsp::DynWorldStepConstants physics_world{};  // gravity and both sleep thresholds zero

    // --- the run --------------------------------------------------------------
    const float reference = host.reference_speed();
    std::printf("  reference speed 0080FC30 = %.6f\n\n", static_cast<double>(reference));
    std::printf("   step        t         x         z    heading   fwd_spd   throttle"
                "    rudder   yaw_rate\n");

    float peak_speed = 0.0f;
    float peak_yaw = 0.0f;
    float t = 0.0f;
    for (int step = 0; step <= steps; ++step) {
        if (step % 10 == 0) {
            std::printf("%7d %8.2f %9.2f %9.2f %10.3f %9.4f %10.4f %9.4f %10.5f\n", step,
                        static_cast<double>(t), static_cast<double>(state.position[0]),
                        static_cast<double>(state.position[2]),
                        static_cast<double>(heading_degrees(state)),
                        static_cast<double>(host.forward_speed()),
                        static_cast<double>(state.throttle),
                        static_cast<double>(state.smoothed_rudder),
                        static_cast<double>(state.angular_velocity.y));
        }
        if (step == steps) {
            break;
        }

        // Re-fill the slot under the write cursor so the order keeps standing: the game
        // does this from the HUD every frame the key is held, and the ring's own forward
        // copy at 00813186 would otherwise only ever mark slots predicted.
        queue.slot_index = ring.write_cursor;
        bsp::issue_unit_order_record_00816a40(issue, queue, scratch, throttle, rudder, 0);
        const int w = ring.write_cursor;
        ring.slot[w].param_a = queue.slot[w].param_a;
        ring.slot[w].param_b = queue.slot[w].param_b;
        ring.slot[w].kind = queue.slot[w].kind;
        ring.slot[w].predicted = queue.slot_active[w];

        bsp::ship_motion_step_00825f20(state, cls, host, dt);

        // The game's own integrator, not an Euler stand-in: 00C41550 then 00C5B1B0, the
        // two phases of one Dyn substep. The motion tick has just written both velocities
        // straight onto the body (00C37E50 / 00C37E20), so the position phase is what
        // turns them into a pose. The velocity phase runs first, as 00C5BB30 orders it;
        // with no force, no gravity and no damping it only rebuilds the inverse inertia.
        // docs/RIGID_BODY_INTEGRATION.md.
        motion.linear_velocity = state.linear_velocity;
        motion.angular_velocity = state.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            body.row0[i] = state.pose_row0[i];
            body.row1[i] = state.pose_row1[i];
            body.row2[i] = state.pose_row2[i];
            body.position[i] = state.position[i];
        }
        bsp::dyn_body_substep(body, physics_world, dt);
        state.linear_velocity = motion.linear_velocity;
        state.angular_velocity = motion.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            state.pose_row0[i] = body.row0[i];
            state.pose_row1[i] = body.row1[i];
            state.pose_row2[i] = body.row2[i];
            state.position[i] = body.position[i];
        }
        t += dt;

        const float speed = host.forward_speed();
        if (speed > peak_speed) {
            peak_speed = speed;
        }
        const float yaw = std::fabs(state.angular_velocity.y);
        if (yaw > peak_yaw) {
            peak_yaw = yaw;
        }
    }

    // --- the two checks a differential run would make --------------------------
    const float final_speed = host.forward_speed();
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
    return 0;
}
