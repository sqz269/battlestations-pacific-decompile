#include "bsp/ship_motion.hpp"

#include <cmath>
#include <cstring>

#include "bsp/camera_position_modes.hpp" // camera_asin_clamped_0042cf10, the 0042CF10 body
#include "bsp/unit_motion.hpp"           // unit_step_towards_0042ac60

// docs/SHIP_MOTION.md. Every expression is the listing's expression in the listing's
// order. x87 register values are carried as double and narrowed with an explicit cast
// wherever the listing has an `FSTP float ptr`, which is where the hardware rounds under
// the MSVC default control word. Absolute values use the listing's form rather than
// fabsf, because -0.0f - 0.0f is -0.0f where fabsf gives +0.0f.

namespace bsp {
namespace {

// 0092EA09 and 0092D46C: the SSE absolute value, 00D7A208 minus the value.
inline float listing_abs(float x) noexcept {
    return (x <= 0.0f) ? (-0.0f - x) : x;
}

// 0092EA97 and 0092D4FD: the same thing done with a bit mask on a float already in
// memory. Written out because it and the subtract form differ on a signalling NaN.
inline float mask_abs(float x) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &x, sizeof(bits));
    bits &= 0x7FFFFFFFu;
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

// One row of the basis dotted with the angular velocity, in the listing's grouping:
// (row[1]*w.y + row[0]*w.x) + row[2]*w.z, with a single narrowing at the store.
inline float basis_row_dot(const float (&row)[3], const OceanVec3& w) noexcept {
    const double a = static_cast<double>(row[1]) * static_cast<double>(w.y);
    const double b = static_cast<double>(row[0]) * static_cast<double>(w.x);
    const double c = static_cast<double>(row[2]) * static_cast<double>(w.z);
    return static_cast<float>((a + b) + c);
}

} // namespace

// ---------------------------------------------------------------------------
// 0092E8C0
// ---------------------------------------------------------------------------

float ship_righting_rate_0092e9e1(float rate, float angle, float dt) noexcept {
    // 0092E9F2..0092EA1C: the gate. |angle| must clear the float threshold strictly.
    const float magnitude = listing_abs(angle);
    if (!(magnitude > kShipMotionRightingThreshold)) {
        return rate; // 0092EA1C JBE -> 0092EA73, the value is discarded unchanged
    }

    // 0092EA1E..0092EA3D: the sign, as an int in a stack slot.
    int sign = 0;
    if (0.0f > angle) {
        sign = -1; // 0092EA26
    } else if (angle > 0.0f) {
        sign = 1; // 0092EA33 with the JA at 0092EA3B
    }

    // 0092EA49..0092EA57: FILD of that int, times the double at 00D19628, minus the
    // angle, and the difference is narrowed to float before it is used.
    const float error =
        static_cast<float>(static_cast<double>(sign) * kShipMotionRighting - static_cast<double>(angle));

    // 0092EA5B..0092EA6D: rate - error * (dt * 10.0), one narrowing at the end.
    const double gain = static_cast<double>(dt) * kShipMotionRightingGain;
    return static_cast<float>(static_cast<double>(rate) - static_cast<double>(error) * gain);
}

ShipSteeringStep ship_apply_steering_0092e8c0(const ShipSteeringInputs& in) noexcept {
    ShipSteeringStep out{};

    // 0092E8C3..0092E8EF: the rudder slews toward the command at dt*0.5 per step. The
    // 0.5 is the double at 00D7A280 and the product is narrowed before the helper.
    const float rudder_step = static_cast<float>(static_cast<double>(in.dt) * kShipMotionHalf);
    out.smoothed_rudder = unit_step_towards_0042ac60(in.smoothed_rudder, in.to_turn, rudder_step);

    // 0092E96F..0092E9D3: the angular velocity decomposed onto the three rows. The
    // listing computes row 1 first, then row 2, then row 0.
    const float rate1 = basis_row_dot(in.basis.row1, in.angular_velocity);
    const float rate2 = basis_row_dot(in.basis.row2, in.angular_velocity);
    float rate0 = basis_row_dot(in.basis.row0, in.angular_velocity);

    // 0092E9D7..0092EA73: the righting term, only for units answering IsKindOf(0Eh).
    if (in.righting_active) {
        const float angle = camera_asin_clamped_0042cf10(in.basis.row2[1]); // 0092E9E9
        rate0 = ship_righting_rate_0092e9e1(rate0, angle, in.dt);
    }

    // 0092EA75..0092EACF: the row-1 component slews toward the commanded yaw rate with
    // a step of 2*dt. The gap is taken from the float difference by bit mask.
    const float max_step = static_cast<float>(static_cast<double>(in.dt) + static_cast<double>(in.dt));
    const float gap = mask_abs(static_cast<float>(static_cast<double>(rate1) -
                                                  static_cast<double>(in.yaw_rate_target)));
    float new_rate1 = rate1;
    if (max_step > gap) {
        new_rate1 = in.yaw_rate_target; // 0092EAAF, the step covers the gap
    } else if (in.yaw_rate_target > rate1) {
        new_rate1 = static_cast<float>(static_cast<double>(rate1) + static_cast<double>(max_step));
    } else {
        new_rate1 = static_cast<float>(static_cast<double>(rate1) - static_cast<double>(max_step));
    }

    out.rate_row0 = rate0;
    out.rate_row1 = new_rate1;
    out.rate_row2 = rate2;

    // 0092EAD3..0092EB8B: recomposition. Each product is stored as a float, then the
    // row-1 and row-2 contributions are summed and the row-0 one is added last.
    float p0[3];
    float p1[3];
    float p2[3];
    for (int i = 0; i < 3; ++i) {
        p0[i] = static_cast<float>(static_cast<double>(rate0) * static_cast<double>(in.basis.row0[i]));
        p2[i] = static_cast<float>(static_cast<double>(rate2) * static_cast<double>(in.basis.row2[i]));
        p1[i] = static_cast<float>(static_cast<double>(new_rate1) * static_cast<double>(in.basis.row1[i]));
    }
    float sum[3];
    for (int i = 0; i < 3; ++i) {
        sum[i] = static_cast<float>(static_cast<double>(p1[i]) + static_cast<double>(p2[i]));
    }
    out.angular_velocity.x = static_cast<float>(static_cast<double>(sum[0]) + static_cast<double>(p0[0]));
    out.angular_velocity.y = static_cast<float>(static_cast<double>(sum[1]) + static_cast<double>(p0[1]));
    out.angular_velocity.z = static_cast<float>(static_cast<double>(sum[2]) + static_cast<double>(p0[2]));
    return out;
}

// ---------------------------------------------------------------------------
// 00825F20's own rules
// ---------------------------------------------------------------------------

OceanVec3 ship_keel_point_00826866(const ShipKeelPointInputs& in) noexcept {
    // 00826866..0082688F: minus half the hull length along the forward row.
    const float along = static_cast<float>(static_cast<double>(in.hull_length) * kShipMotionHalfNegative);
    float stern[3];
    for (int i = 0; i < 3; ++i) {
        stern[i] = static_cast<float>(static_cast<double>(along) * static_cast<double>(in.row_forward[i]));
    }

    // 00826893..008268B9: added to the world translation component by component.
    float base[3];
    for (int i = 0; i < 3; ++i) {
        base[i] = static_cast<float>(static_cast<double>(stern[i]) + static_cast<double>(in.translation[i]));
    }

    // 00826910..00826972: minus half the hull height along the up row, added again.
    const float down = static_cast<float>(static_cast<double>(in.hull_height) * kShipMotionHalfNegative);
    OceanVec3 out{};
    float keel[3];
    for (int i = 0; i < 3; ++i) {
        const float drop =
            static_cast<float>(static_cast<double>(down) * static_cast<double>(in.row_up[i]));
        keel[i] = static_cast<float>(static_cast<double>(drop) + static_cast<double>(base[i]));
    }
    out.x = keel[0];
    out.y = keel[1];
    out.z = keel[2];
    return out;
}

ShipThrottleGate ship_throttle_gate_00826994(const ShipThrottleGateInputs& in) noexcept {
    ShipThrottleGate gate{};

    // 0082698A..008269A9. The sampled height is halved and stored as a float before the
    // comparison, and the comparison is against the keel point's y.
    const float half_height = static_cast<float>(static_cast<double>(in.wave_height) * kShipMotionHalf);
    if (in.keel_y <= half_height) {
        gate.command_applies = true;
        gate.throttle = in.throttle; // 008269A9
    } else {
        gate.command_applies = false; // 008269A5 XOR BL,BL
        gate.throttle = 0.0f;         // 008269A2
    }

    // 008269B1..008269F0: the submarine scale. Class id 8 at 008269B1, the deck
    // reference -3.0f at 00CE3D50, and the comparison proceeds only when the reference
    // is strictly above the pose base.
    if (in.class_id == kUnitForceSubmarineClassId && kUnitForceDeckDepth > in.pose_base_y) {
        gate.throttle = static_cast<float>(static_cast<double>(in.submerged_scale) *
                                           static_cast<double>(gate.throttle));
    }

    // 008269F4..00826A00: an out-of-action hull makes no way. This does not clear the
    // gate, so the steering call still runs.
    if (in.out_of_action_5d) {
        gate.throttle = 0.0f;
    }
    return gate;
}

float ship_target_speed_00826a3a(float max_speed, float gameplay_scale, float throttle,
                                 float engine_gate) noexcept {
    // 00826A3A..00826A4F: the first product is stored as a float on its own.
    const float scaled = static_cast<float>(static_cast<double>(max_speed) *
                                            static_cast<double>(gameplay_scale));
    // 00826A53..00826A5F: the remaining two multiplies share one narrowing.
    return static_cast<float>((static_cast<double>(scaled) * static_cast<double>(throttle)) *
                              static_cast<double>(engine_gate));
}

float ship_engine_gate_00826754(bool engine_jam, float thrust_mod) noexcept {
    // 00826754..0082676E: AL is set only when the jam byte is clear and the modifier
    // compares unequal to the zeroed XMM1, then converted to a float at 00826792.
    if (engine_jam) {
        return 0.0f;
    }
    return (thrust_mod != 0.0f) ? 1.0f : 0.0f;
}

ShipBoostStep ship_boost_step_00826a6f(const ShipBoostInputs& in) noexcept {
    ShipBoostStep out{};
    out.reserve = in.reserve;
    out.target_speed = in.target_speed;
    out.kind_mirror = 0;

    if (!in.trait_0e) {
        return out; // 00826A7C, the whole block is skipped
    }
    out.applied = true;
    out.kind_mirror = in.order_kind; // 00826A90, unit+118Ch = unit+988h

    if (in.order_kind != 0) {
        // 00826A98: a standing boost order with an empty reserve changes nothing.
        if (!(in.reserve > 0.0f)) {
            return out;
        }
        // 00826AAB..00826AB6: the commanded speed is replaced, not scaled.
        out.target_speed = static_cast<float>(static_cast<double>(in.reference_speed) *
                                              static_cast<double>(in.boost_speed_scale));
        // 00826ABA..00826B04.
        out.reserve = static_cast<float>(static_cast<double>(in.reserve) - static_cast<double>(in.dt));
        return out;
    }

    // 00826AC9..00826AE2: refill. One narrowing after the multiply, divide and add.
    const double refilled = (static_cast<double>(in.dt) * static_cast<double>(in.boost_capacity)) /
                                static_cast<double>(in.boost_refill_time) +
                            static_cast<double>(in.reserve);
    float value = static_cast<float>(refilled);
    // 00826AF0..00826B04: clamped at the capacity. The JC keeps the value only when it
    // is strictly below, so an unordered compare clamps.
    if (!(value < in.boost_capacity)) {
        value = in.boost_capacity;
    }
    out.reserve = value;
    return out;
}

// ---------------------------------------------------------------------------
// The tick
// ---------------------------------------------------------------------------

ShipMotionStepResult ship_motion_step_00825f20(ShipMotionState& state,
                                               const ShipMotionClass& cls,
                                               ShipMotionHost& host, float dt) {
    ShipMotionStepResult result{};
    result.dt_raw = dt;

    // 00826121: the ring tick, with the raw delta. It is what writes state.throttle and
    // state.to_turn, so the host runs it before anything reads them.
    host.tick_order_ring(dt);

    // 00826126..00826144: a positive per-unit time scale rescales everything after the
    // ring tick. Zero and negative values leave the delta alone.
    float scaled = dt;
    if (state.motion_time_scale > 0.0f) {
        scaled = static_cast<float>(static_cast<double>(state.motion_time_scale) *
                                    static_cast<double>(dt));
    }
    result.dt_scaled = scaled;

    // 00826866..00826985: the keel sample point and the wave height under it.
    ShipKeelPointInputs keel{};
    for (int i = 0; i < 3; ++i) {
        keel.translation[i] = state.position[i];
        keel.row_up[i] = state.pose_row1[i];
        keel.row_forward[i] = state.pose_row2[i];
    }
    keel.hull_length = cls.hull_length;
    keel.hull_height = cls.hull_height;
    result.keel_point = ship_keel_point_00826866(keel);
    result.wave_height = host.ocean_height(result.keel_point.x, result.keel_point.z);

    // 00826994..00826A00.
    ShipThrottleGateInputs gate_in{};
    gate_in.keel_y = result.keel_point.y;
    gate_in.wave_height = result.wave_height;
    gate_in.throttle = state.throttle;
    gate_in.out_of_action_5d = state.out_of_action_5d;
    gate_in.class_id = state.class_id;
    gate_in.pose_base_y = state.position[1];
    gate_in.submerged_scale = 1.0f; // settings+4B4h reaches the gate through this field
    result.gate = ship_throttle_gate_00826994(gate_in);

    // 00826754..0082676E, hoisted far above the command tail in the listing.
    result.engine_gate = ship_engine_gate_00826754(state.engine_jam, state.thrust_mod);

    // 00826A06..00826A5F.
    const float scale = host.gameplay_scale();
    result.target_speed =
        ship_target_speed_00826a3a(state.max_speed, scale, result.gate.throttle, result.engine_gate);

    // 00826A6D: the force model, before either command call. For a surface ship this is
    // 00937440, which adds a rudder torque and then runs the hydrodynamic callback.
    result.force_model_torque = host.run_force_model(scaled);

    // 00826A6F..00826B04.
    ShipBoostInputs boost{};
    boost.trait_0e = host.unit_trait_0e();
    boost.order_kind = state.order_kind;
    boost.reserve = state.boost_reserve;
    // 00826AAB is reached only past the trait probe, the non-zero kind and the positive
    // reserve, so the host call is made under the same three conditions.
    boost.reference_speed =
        (boost.trait_0e && state.order_kind != 0 && state.boost_reserve > 0.0f)
            ? host.reference_speed()
            : 0.0f;
    boost.boost_speed_scale = cls.boost_speed_scale;
    boost.boost_capacity = cls.boost_capacity;
    boost.boost_refill_time = cls.boost_refill_time;
    boost.dt = scaled;
    boost.target_speed = result.target_speed;
    result.boost = ship_boost_step_00826a6f(boost);
    if (result.boost.applied) {
        state.boost_reserve = result.boost.reserve;
        state.order_kind_mirror = result.boost.kind_mirror;
        result.target_speed = result.boost.target_speed;
    }

    // 00826B0A..00826B29: the throttle half, suppressed when the keel point cleared the
    // water.
    if (result.gate.command_applies) {
        UnitAxialSpeedInputs axial{};
        axial.velocity = host.body_linear_velocity(); // 0092D30E
        const ShipBodyBasis speed_basis = host.body_basis(); // 0092D316
        axial.axis.x = speed_basis.row2[0];
        axial.axis.y = speed_basis.row2[1];
        axial.axis.z = speed_basis.row2[2];
        axial.commanded_speed = result.target_speed;
        // 0092D444 selects between 00825EC0's value and class+504h; the reconstruction
        // in bsp/unit_forces.hpp names the first drive_accel and the second brake_accel,
        // which the class data contradicts. See the Corrections section of
        // docs/SHIP_MOTION.md; the mapping below is by offset, not by name.
        axial.drive_accel = host.forward_acceleration(); // 00825EC0, class+508h boosted
        axial.brake_accel = cls.max_accel;               // class+504h
        axial.dt = scaled;
        result.axial = unit_approach_axial_speed_0092d300(axial);
        host.body_set_linear_velocity(result.axial.velocity);
        result.speed_applied = true;

        // 00826B2E..00826B54: the rudder half, skipped by a jammed rudder.
        if (!state.steering_jam) {
            ShipSteeringInputs steer{};
            steer.smoothed_rudder = state.smoothed_rudder;
            steer.to_turn = state.to_turn;
            steer.dt = scaled;
            steer.basis = host.body_basis(); // 0092E8F7, a second call in the listing
            steer.angular_velocity = host.body_angular_velocity(); // 0092E942
            // 0092E947 reads controller+80h *after* the slew, so the value handed to
            // 00811890 is the slewed one. The slew is the same pure step the routine
            // itself repeats at 0092E8EF; it is recomputed here rather than passed in so
            // ship_apply_steering_0092e8c0 stays a whole reconstruction of its routine.
            const float slewed_rudder = unit_step_towards_0042ac60(
                state.smoothed_rudder, state.to_turn,
                static_cast<float>(static_cast<double>(scaled) * kShipMotionHalf));
            steer.yaw_rate_target = host.yaw_rate_target(slewed_rudder);
            steer.righting_active = boost.trait_0e;
            result.steering = ship_apply_steering_0092e8c0(steer);
            state.smoothed_rudder = result.steering.smoothed_rudder;
            host.body_set_angular_velocity(result.steering.angular_velocity);
            result.steering_applied = true;
        }
    }

    // 00826B59..00826B84: both run whatever the gate said.
    host.controller_step(scaled);
    host.unit_post_motion(scaled);
    return result;
}

// ---------------------------------------------------------------------------
// The stand-in integrator
// ---------------------------------------------------------------------------

void ship_integrate_stand_in(ShipMotionState& state, float dt) noexcept {
    state.position[0] += state.linear_velocity.x * dt;
    state.position[1] += state.linear_velocity.y * dt;
    state.position[2] += state.linear_velocity.z * dt;

    // Rotate the three rows by the body angular velocity, w x row, then re-orthonormalise
    // so repeated steps do not drift. The game does none of this: it hands the velocities
    // to the physics library and reads the pose back.
    const float wx = state.angular_velocity.x * dt;
    const float wy = state.angular_velocity.y * dt;
    const float wz = state.angular_velocity.z * dt;

    float* rows[3] = {state.pose_row0, state.pose_row1, state.pose_row2};
    for (float* row : rows) {
        const float x = row[0];
        const float y = row[1];
        const float z = row[2];
        row[0] = x + (wy * z - wz * y);
        row[1] = y + (wz * x - wx * z);
        row[2] = z + (wx * y - wy * x);
    }

    auto normalise = [](float* v) {
        const float n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (n > 0.0f) {
            v[0] /= n;
            v[1] /= n;
            v[2] /= n;
        }
    };
    auto cross = [](const float* a, const float* b, float* out) {
        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
    };
    normalise(state.pose_row1);
    cross(state.pose_row1, state.pose_row2, state.pose_row0);
    normalise(state.pose_row0);
    cross(state.pose_row0, state.pose_row1, state.pose_row2);
    normalise(state.pose_row2);
}

} // namespace bsp
