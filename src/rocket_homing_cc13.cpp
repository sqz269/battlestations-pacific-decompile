// Packet cc13-homing1: the rocket air step 0080A000 and its homing branch.
// docs/CC13_ROCKET_HOMING.md. Reconstructed from the listing; build-tested only.
//
// Precision: x87 intermediates are carried in double and every FSTP dword is a
// float store, as in src/rocket_task_cc13.cpp. The main thread runs the FPU at
// 24 bits (docs/CC13_FPU_PRECISION.md), so the last bit can differ.
#include "bsp/rocket_homing_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include "bsp/camera_affine.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/pose_derived.hpp"
#include "bsp/rocket_task_cc13.hpp"

namespace bsp {

namespace {

constexpr std::size_t kRight = 0;     // +74h
constexpr std::size_t kUp = 4;        // +84h
constexpr std::size_t kForward = 8;   // +94h (0042D5E0 writes matrix+20h)

// 00415690 BSP_Math_ClampInPlace: the low bound first, and it wins outright.
float clamp_low_first(float value, float low, float high) noexcept {
    if (low > value) return low;
    if (value > high) return high;
    return value;
}

// 00415550 BSP_Math_MaxFloatByRef: `b < a ? a : b`, so an unordered pair gives b.
float max_by_ref(float a, float b) noexcept {
    return b < a ? a : b;
}

std::array<float, 3> transform_point(const std::array<float, 3>& p, const CameraMatrix& m) {
    std::array<float, 3> out{};
    transform_point_004142e0(p, m, out);
    return out;
}

}  // namespace

void vector3_divide_in_place_pc24_0042ae80(std::array<float, 3>& v, float s) noexcept {
    v[0] = static_cast<float>(cc13x87::x87_div24(v[0], s));   // 0042AE8A FDIVP
    v[1] = static_cast<float>(cc13x87::x87_div24(v[1], s));   // 0042AE93 FDIV
    v[2] = static_cast<float>(cc13x87::x87_div24(v[2], s));   // 0042AE98 FDIVR
}

void vector3_divide_in_place_0042ae80(std::array<float, 3>& v, float s) noexcept {
    if constexpr (kVector3DivideInPlacePc24Cc13Bound) {
        vector3_divide_in_place_pc24_0042ae80(v, s);
    } else {
        v[0] = static_cast<float>(static_cast<double>(v[0]) / s);   // 0042AE8A FDIVP
        v[1] = static_cast<float>(static_cast<double>(v[1]) / s);   // 0042AE93 FDIV
        v[2] = static_cast<float>(static_cast<double>(v[2]) / s);   // 0042AE98 FDIVR
    }
}

void rocket_set_homing_target_0080aa20(RocketHomingBlock& block, std::size_t target_plus_one,
                                       bool target_is_kind_5,
                                       const std::array<float, 3>& point, float angle) noexcept {
    if (target_plus_one != 0 && target_is_kind_5) {
        block.target_plus_one = target_plus_one;   // observer re-registered on change
        block.body_point = point;
        block.angle = angle;
        return;
    }
    block.target_plus_one = 0;
    block.body_point = {0.0f, 0.0f, 0.0f};          // [00F87574..00F8757C]
    block.angle = 0.0f;
}

bool rocket_ignition_step_0080a024(RocketFlight& r, const RocketClassParams& c, float dt) noexcept {
    float pre = 0.0f;      // [ESP+24h]
    float credit = 0.0f;   // [ESP+18h]
    bool lit_now = false;
    const float timer = r.ignition_timer_490;
    if (r.ignited_494) {
        // 0080A02E..0080A063: credit = min(timer, dt), the timer on a tie.
        credit = timer > dt ? dt : timer;
    } else {
        // 0080A06E..0080A0C7.
        const float remaining = static_cast<float>(static_cast<double>(c.ignition_delay_e0) - timer);
        pre = remaining > dt ? dt : remaining;                      // 0080A091 JA
        const float post = static_cast<float>(static_cast<double>(dt) - pre);
        credit = timer > post ? post : timer;                       // 0080A0BD JBE
        // 0080A0D0..0080A0E6: a non-zero (or unordered) credit lights the motor,
        // and so does a zero pre-ignition slice. Then record+310h->vtable[3Ch].
        if (credit != 0.0f || credit != credit || pre == 0.0f) {
            r.ignited_494 = true;
            lit_now = true;
        }
    }
    // 0080A100..0080A148.
    const float accumulated = static_cast<float>(static_cast<double>(pre) + timer);
    r.ignition_timer_490 = static_cast<float>(static_cast<double>(accumulated) - credit);
    const double g = rocket_homing::kGravity;
    r.velocity[1] = static_cast<float>(static_cast<double>(r.velocity[1]) - g * pre);
    r.velocity[1] = static_cast<float>(static_cast<double>(r.velocity[1]) - g * -static_cast<double>(credit));
    return lit_now;
}

float rocket_thrust_step_0080a15b(RocketFlight& r, const RocketClassParams& c, float dt) noexcept {
    // 0080A16B..0080A198: the axial speed along the world forward row +ECh..+F4h
    // (+CCh + 20h). The host's pose doubles as the world matrix (labelled).
    const CameraMatrix& w = r.pose;
    const float axial = static_cast<float>(
        static_cast<double>(w[kForward + 1]) * r.velocity[1]
        + static_cast<double>(w[kForward + 0]) * r.velocity[0]
        + static_cast<double>(w[kForward + 2]) * r.velocity[2]);
    const float vmax = c.vmax_d8;                                    // [ESP+0Ch]
    if (!(vmax > axial)) return axial;                              // 0080A1B0 JBE
    float stepped = static_cast<float>(static_cast<double>(c.accel_dc) * dt + axial);
    if (stepped > vmax) stepped = vmax;                              // 0080A1D1 JBE
    const float delta = static_cast<float>(static_cast<double>(stepped) - axial);
    const float dx = static_cast<float>(static_cast<double>(delta) * w[kForward + 0]);
    const float dy = static_cast<float>(static_cast<double>(w[kForward + 1]) * delta);
    const float dz = static_cast<float>(static_cast<double>(delta) * w[kForward + 2]);
    r.velocity[0] = static_cast<float>(static_cast<double>(dx) + r.velocity[0]);
    r.velocity[1] = static_cast<float>(static_cast<double>(r.velocity[1]) + dy);
    r.velocity[2] = static_cast<float>(static_cast<double>(dz) + r.velocity[2]);
    return stepped;                                                  // 0080A1FF [ESP+24h]
}

RocketHomingExit rocket_homing_step_0080a25c(RocketFlight& r, const RocketClassParams& c,
                                             const RocketHomingTargetView* target,
                                             float axial_speed, float* flight_time_out,
                                             std::array<float, 2>* steer_out) noexcept {
    if (target == nullptr || r.homing.target_plus_one == 0) return RocketHomingExit::kNoTarget;
    if (!(r.homing.angle > rocket_homing::kAngleFloor)) return RocketHomingExit::kAngleNotPositive;

    // +110h, the derived affine inverse of the world matrix (00414E10 ->
    // 00B63D50), recomputed here because the host has no pose cache.
    CameraMatrix inverse{};
    derive_pose_affine_inverse_00b63d50(inverse, r.pose);

    // 0080A28F..0080A2DA: the body point in world, then in the rocket's frame.
    const std::array<float, 3> world_point = transform_point(r.homing.body_point, target->world);
    const std::array<float, 3> local = transform_point(world_point, inverse);
    if (!(static_cast<double>(local[2]) > rocket_homing::kMinForwardRange)) {
        return RocketHomingExit::kBodyPointNear;                     // 0080A311 JBE
    }

    // 0080A317..0080A322: 00809E10 on the rocket itself. Its v0 is
    // [record+3Ch]->vtable[38h] when +3Ch is set, else |+318h| (0042B2F0). A
    // released rocket is read as having no +3Ch (provisional, as the task doc).
    const float launch_speed = force_event_vector_length_0042b2f0(r.velocity);
    const float t = rocket_flight_time_00809e10(local[2], launch_speed, c.vmax_d8, c.accel_dc);
    if (flight_time_out != nullptr) *flight_time_out = t;

    // 0080A326..0080A381: the lead point, world_point + target velocity * t.
    const std::array<float, 3>& v = target->velocity;
    const float lx = static_cast<float>(static_cast<double>(v[0]) * t);
    const float ly = static_cast<float>(static_cast<double>(v[1]) * t);
    const float lz = static_cast<float>(static_cast<double>(t) * v[2]);
    const std::array<float, 3> lead{
        static_cast<float>(static_cast<double>(lx) + world_point[0]),
        static_cast<float>(static_cast<double>(world_point[1]) + ly),
        static_cast<float>(static_cast<double>(world_point[2]) + lz)};

    // 0080A385 00414D10, then 0080A3B4 the same 50 m test.
    std::array<float, 3> steer = transform_point(lead, inverse);
    if (!(static_cast<double>(steer[2]) > rocket_homing::kMinForwardRange)) {
        return RocketHomingExit::kLeadPointNear;
    }

    // 0080A3BA..0080A3DD: divide by 0.5 x max(z, VMax). The forward term below is
    // the literal 1, so for z >= VMax the lateral terms are 2x/z and 2y/z: the
    // rocket aims at twice the bearing tangent.
    const float larger = max_by_ref(steer[2], c.vmax_d8);
    const float divisor = static_cast<float>(static_cast<double>(larger) * rocket_homing::kDivisorHalf);
    vector3_divide_in_place_0042ae80(steer, divisor);

    // 0080A3E2..0080A437: clamp x then y to [-0.05 HA, 0.05 HA].
    const float cap = static_cast<float>(static_cast<double>(rocket_homing::kStepCapScale) * r.homing.angle);
    const float low = rocket_homing::kNegativeZero - cap;            // SUBSS
    steer[0] = clamp_low_first(steer[0], low, cap);
    steer[1] = clamp_low_first(steer[1], low, cap);
    if (steer_out != nullptr) *steer_out = {steer[0], steer[1]};

    // 0080A43C..0080A4FD: forward + x' right, then + y' up; float stores.
    CameraMatrix& m = r.pose;
    std::array<float, 3> aim{m[kForward + 0], m[kForward + 1], m[kForward + 2]};
    {
        const float a0 = static_cast<float>(static_cast<double>(steer[0]) * m[kRight + 0]);
        const float a1 = static_cast<float>(static_cast<double>(m[kRight + 1]) * steer[0]);
        const float a2 = static_cast<float>(static_cast<double>(steer[0]) * m[kRight + 2]);
        aim[0] = static_cast<float>(static_cast<double>(a0) + aim[0]);
        aim[1] = static_cast<float>(static_cast<double>(aim[1]) + a1);
        aim[2] = static_cast<float>(static_cast<double>(aim[2]) + a2);
    }
    {
        const float b0 = static_cast<float>(static_cast<double>(steer[1]) * m[kUp + 0]);
        const float b1 = static_cast<float>(static_cast<double>(m[kUp + 1]) * steer[1]);
        const float b2 = static_cast<float>(static_cast<double>(steer[1]) * m[kUp + 2]);
        aim[0] = static_cast<float>(static_cast<double>(b0) + aim[0]);
        aim[1] = static_cast<float>(static_cast<double>(aim[1]) + b1);
        aim[2] = static_cast<float>(static_cast<double>(aim[2]) + b2);
    }

    // 0080A501 0042D5E0: forward row = aim, then 0085DC80 rebuilds the basis.
    // 0080A508 0042ED50 invalidates the subtree pose (no host cache).
    m[kForward + 0] = aim[0];
    m[kForward + 1] = aim[1];
    m[kForward + 2] = aim[2];
    orthonormalize_pose_matrix_0085dc80(m.data());

    // 0080A50D..0080A553: velocity = world rotation x (0, 0, axial speed), 0042D0D0
    // with normalize 0. The zero terms are kept so a non-finite row propagates.
    for (int i = 0; i < 3; ++i) {
        r.velocity[i] = static_cast<float>(0.0 * m[kRight + i] + 0.0 * m[kUp + i]
                                           + static_cast<double>(axial_speed) * m[kForward + i]);
    }
    return RocketHomingExit::kSteered;
}

RocketAirStepResult rocket_air_step_0080a000(RocketFlight& r, const RocketClassParams& c,
                                             const RocketHomingTargetView* target,
                                             float dt) noexcept {
    RocketAirStepResult out{};
    out.ignited_this_step = rocket_ignition_step_0080a024(r, c, dt);
    if (!r.ignited_494) return out;                                  // 0080A155 JZ
    out.axial_speed = rocket_thrust_step_0080a15b(r, c, dt);
    out.homing = rocket_homing_step_0080a25c(r, c, target, out.axial_speed, &out.flight_time,
                                             &out.steer);
    return out;
}

}  // namespace bsp
