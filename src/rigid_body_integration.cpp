// The Dyn rigid-body integrator. See docs/RIGID_BODY_INTEGRATION.md for the addresses,
// the original ABI, the evidence and the uncertainty. Every expression below is
// transcribed from the listing in the listing's order; the intermediates are float32
// because the two integration phases are SSE-free scalar x87 code whose every temporary
// is an FSTP to a float slot or a decompiler float local, with no double constant in
// either body apart from the two image doubles the header names.

#include "bsp/rigid_body_integration.hpp"

#include <cmath>

namespace bsp {
namespace {

// 004011D0, the image's float square root (it forwards to the CRT helper 00BF7030 and
// rounds the result to float at 004011E1). Both phases call it for every magnitude.
float sqrt_004011d0(float value) noexcept { return std::sqrt(value); }

// The clamped damping factor both phases build: 1 - rate*dt, floored at zero. The listing
// stores it to a float slot and compares with 0.0f using the "<= 0" direction.
float damping_factor(float one_minus) noexcept { return (one_minus <= 0.0f) ? 0.0f : one_minus; }

}  // namespace

void dyn_body_set_mass_00c37f40(DynBody& body, float mass) noexcept {
    // 00C37F44 loads M through B+4h; 00C37F47..00C37F53 is FLD1 / FDIVRP / FSTP float.
    body.motion->inverse_mass = static_cast<float>(1.0f / mass);
}

float dyn_body_mass_00c31fc0(const DynBody& body) noexcept {
    // 00C31FC1 tests B+50h bit 0 and returns FLDZ for a static body; otherwise
    // 00C31FCE..00C31FD8 is 1.0f / M+50h rounded through a float slot.
    if ((body.flags & kDynBodyFlagStatic) != 0u) {
        return 0.0f;
    }
    return static_cast<float>(1.0f / body.motion->inverse_mass);
}

void dyn_body_set_inertia_00c37e70(DynBody& body, const OceanVec3& inertia) noexcept {
    // 00C37E70: each component is compared with zero through UCOMISS/LAHF/TEST AH,44h.
    // Equality is the only case that takes the JNP at 00C37E8C, and it stores the FLDZ
    // zero; every other case (including a NaN, which is unordered) takes the reciprocal
    // through FDIVR ST(0),ST(1). The written fields are M+54h, M+58h and M+5Ch.
    DynMotionState& m = *body.motion;
    m.inverse_inertia_body.x = (inertia.x == 0.0f) ? 0.0f : static_cast<float>(1.0f / inertia.x);
    m.inverse_inertia_body.y = (inertia.y == 0.0f) ? 0.0f : static_cast<float>(1.0f / inertia.y);
    m.inverse_inertia_body.z = (inertia.z == 0.0f) ? 0.0f : static_cast<float>(1.0f / inertia.z);
}

void dyn_body_set_linear_damping_00c37e00(DynBody& body, float damping) noexcept {
    body.motion->linear_damping = damping;  // 00C37E09, MOVSS to M+B8h, no flag change
}

void dyn_body_set_angular_damping_00c37de0(DynBody& body, float damping) noexcept {
    body.motion->angular_damping = damping;  // 00C37DE9, MOVSS to M+BCh, no flag change
}

void dyn_body_set_linear_velocity_00c37e50(DynBody& body, const OceanVec3& v) noexcept {
    body.motion->linear_velocity = v;          // 00C37E5A..00C37E65
    body.flags &= ~kDynBodyWakeMask;           // 00C37E68
}

void dyn_body_set_angular_velocity_00c37e20(DynBody& body, const OceanVec3& w) noexcept {
    body.motion->angular_velocity = w;         // 00C37E2A..00C37E39
    body.flags &= ~kDynBodyWakeMask;           // 00C37E3C
}

void dyn_body_add_force_00c35360(DynBody& body, const OceanVec3& f) noexcept {
    DynMotionState& m = *body.motion;
    // 00C35363..00C35380: the accumulator is loaded first for x, the argument first for y
    // and z. The sums are float because each is stored back with FSTP m32.
    m.force.x = static_cast<float>(m.force.x + f.x);
    m.force.y = static_cast<float>(f.y + m.force.y);
    m.force.z = static_cast<float>(f.z + m.force.z);
    body.flags &= ~kDynBodyWakeMask;           // 00C35383
}

void dyn_body_add_torque_00c35330(DynBody& body, const OceanVec3& t) noexcept {
    DynMotionState& m = *body.motion;
    m.torque.x = static_cast<float>(m.torque.x + t.x);  // 00C35333..00C3533F
    m.torque.y = static_cast<float>(t.y + m.torque.y);  // 00C35341..00C35347
    m.torque.z = static_cast<float>(t.z + m.torque.z);  // 00C3534A..00C35350
    body.flags &= ~kDynBodyWakeMask;                    // 00C35353
}

void dyn_body_integrate_velocity_00c41550(DynBody& body, const DynWorldStepConstants& world,
                                          float dt) noexcept {
    // 00C4155E: the loop body runs only when B+50h bit 4 is clear.
    if ((body.flags & kDynBodyFlagNoIntegrate) != 0u) {
        return;
    }
    DynMotionState& m = *body.motion;

    // Force to linear velocity. M+50h is the inverse mass, so the product is an
    // acceleration times dt. The scale is formed once and reused for the three axes.
    const float k = m.inverse_mass * dt;
    m.linear_velocity.x = m.linear_velocity.x + k * m.force.x;
    m.linear_velocity.y = m.linear_velocity.y + m.force.y * k;
    m.linear_velocity.z = m.linear_velocity.z + k * m.force.z;

    // Gravity, skipped when B+50h bit 2 is set. The y and z components are read into
    // temporaries before the x store, which is why the order below looks shuffled.
    if ((body.flags & kDynBodyFlagNoGravity) == 0u) {
        const float gy = world.gravity.y;
        const float gz = world.gravity.z;
        m.linear_velocity.x = m.linear_velocity.x + world.gravity.x * dt;
        m.linear_velocity.y = m.linear_velocity.y + gy * dt;
        m.linear_velocity.z = m.linear_velocity.z + gz * dt;
    }

    // Linear damping.
    float d = damping_factor(1.0f - m.linear_damping * dt);
    m.linear_velocity.x = d * m.linear_velocity.x;
    m.linear_velocity.y = m.linear_velocity.y * d;
    m.linear_velocity.z = d * m.linear_velocity.z;

    // The world-space inverse inertia, rebuilt from the body rows every substep:
    // W[i][j] = sum over k of Ik * row_k[i] * row_k[j], with the row terms accumulated in
    // the listing's order (row 2 first for the first column block, row 0 first afterwards).
    const float i0 = m.inverse_inertia_body.x;
    const float i1 = m.inverse_inertia_body.y;
    const float i2 = m.inverse_inertia_body.z;

    const float a0x = i0 * body.row0[0];
    const float a0y = body.row0[1] * i0;
    const float a0z = i0 * body.row0[2];
    const float a1x = i1 * body.row1[0];
    const float a1y = body.row1[1] * i1;
    const float a1z = i1 * body.row1[2];
    const float a2x = i2 * body.row2[0];
    const float a2y = body.row2[1] * i2;
    const float a2z = i2 * body.row2[2];

    float* const w = m.inverse_inertia_world;
    w[0] = body.row2[0] * a2x + body.row0[0] * a0x + a1x * body.row1[0];
    w[1] = a2y * body.row2[0] + a0y * body.row0[0] + a1y * body.row1[0];
    w[2] = a2z * body.row2[0] + a0z * body.row0[0] + a1z * body.row1[0];
    w[3] = body.row2[1] * a2x + body.row0[1] * a0x + body.row1[1] * a1x;
    w[4] = a2y * body.row2[1] + a0y * body.row0[1] + a1y * body.row1[1];
    w[5] = a2z * body.row2[1] + a0z * body.row0[1] + a1z * body.row1[1];
    w[6] = body.row2[2] * a2x + body.row0[2] * a0x + body.row1[2] * a1x;
    w[7] = a2y * body.row2[2] + a0y * body.row0[2] + a1y * body.row1[2];
    w[8] = a2z * body.row2[2] + a0z * body.row0[2] + a1z * body.row1[2];

    // Torque to angular velocity through that tensor.
    const float tx = m.torque.x;
    const float ty = m.torque.y;
    const float tz = m.torque.z;
    m.angular_velocity.x = (w[6] * tz + w[0] * tx + w[3] * ty) * dt + m.angular_velocity.x;
    m.angular_velocity.y = m.angular_velocity.y + (w[7] * tz + w[4] * ty + w[1] * tx) * dt;
    m.angular_velocity.z = m.angular_velocity.z + (w[8] * tz + w[5] * ty + w[2] * tx) * dt;

    // Angular damping.
    d = damping_factor(1.0f - m.angular_damping * dt);
    m.angular_velocity.x = d * m.angular_velocity.x;
    m.angular_velocity.y = d * m.angular_velocity.y;
    m.angular_velocity.z = m.angular_velocity.z * d;

    // M+B4h. The rebuild multiplies the row-0 and row-2 terms by a literal 0.0f, leaving
    // only the row-1 contribution. It runs AFTER the angular velocity update, so the lock
    // takes effect on the NEXT substep, not this one. That ordering is the listing's.
    if (m.lock_torque_to_row1) {
        const float z0x = body.row0[0] * 0.0f;
        const float z2x = body.row2[0] * 0.0f;
        w[0] = z0x + a1x * body.row1[0] + z2x;
        w[1] = z0x + a1y * body.row1[0] + z2x;
        w[2] = z0x + a1z * body.row1[0] + z2x;
        const float z0y = body.row0[1] * 0.0f;
        const float z2y = body.row2[1] * 0.0f;
        w[3] = z0y + body.row1[1] * a1x + z2y;
        w[4] = z0y + a1y * body.row1[1] + z2y;
        w[5] = z0y + a1z * body.row1[1] + z2y;
        const float z0z = body.row0[2] * 0.0f;
        const float z2z = body.row2[2] * 0.0f;
        w[6] = body.row1[2] * a1x + z0z + z2z;
        w[7] = a1y * body.row1[2] + z0z + z2z;
        w[8] = z0z + a1z * body.row1[2] + z2z;
    }
}

void dyn_body_integrate_position_00c5b1b0(DynBody& body, const DynWorldStepConstants& world,
                                          float dt) noexcept {
    // 00C5B1D4: same bit-4 gate as the velocity phase.
    if ((body.flags & kDynBodyFlagNoIntegrate) != 0u) {
        return;
    }
    DynMotionState& m = *body.motion;

    // Position from the velocity plus its bias. The y and z bias components are read into
    // temporaries before the x store; the listing's operand order is kept.
    const float by = m.linear_bias.y;
    const float vy = m.linear_velocity.y;
    const float bz = m.linear_bias.z;
    const float vz = m.linear_velocity.z;
    body.position[0] = body.position[0] + (m.linear_velocity.x + m.linear_bias.x) * dt;
    body.position[1] = body.position[1] + (by + vy) * dt;
    body.position[2] = body.position[2] + (bz + vz) * dt;

    // The rotation axis and angle. The angular velocity is scaled up by 100 before the
    // magnitude test and the angle divides it back out, so the effective threshold on the
    // unscaled magnitude is 1e-7 per second.
    float ax = (m.angular_bias.x + m.angular_velocity.x) * kDynAngularScale;
    float ay = (m.angular_bias.y + m.angular_velocity.y) * kDynAngularScale;
    float az = kDynAngularScale * (m.angular_bias.z + m.angular_velocity.z);
    float mag = sqrt_004011d0(az * az + ay * ay + ax * ax);
    if (mag > kDynAngularEpsilon) {
        ax = ax / mag;
        ay = ay / mag;
        az = az / mag;
        const float angle = (mag * dt) / kDynAngularScale;

        // Rodrigues on row 2, then on row 1, each with its own FSIN/FCOS pair.
        {
            const float dot = ay * body.row2[1] + ax * body.row2[0] + az * body.row2[2];
            const float px = body.row2[0] - dot * ax;
            const float py = body.row2[1] - ay * dot;
            const float pz = body.row2[2] - dot * az;
            const float s = std::sin(angle);
            const float c = std::cos(angle);
            body.row2[0] = c * px + s * (pz * ay - py * az) + dot * ax;
            body.row2[1] = py * c + (px * az - ax * pz) * s + ay * dot;
            body.row2[2] = c * pz + s * (py * ax - ay * px) + dot * az;
        }
        {
            const float dot = ay * body.row1[1] + ax * body.row1[0] + az * body.row1[2];
            const float px = body.row1[0] - dot * ax;
            const float py = body.row1[1] - ay * dot;
            const float pz = body.row1[2] - dot * az;
            const float s = std::sin(angle);
            const float c = std::cos(angle);
            body.row1[0] = c * px + s * (pz * ay - py * az) + dot * ax;
            body.row1[1] = py * c + (px * az - ax * pz) * s + ay * dot;
            body.row1[2] = c * pz + s * (py * ax - ay * px) + dot * az;
        }

        // Gram-Schmidt rebuild: normalise row 2, row 0 = row1 x row2 normalised,
        // row 1 = row2 x row0. The triple is therefore right-handed by construction.
        {
            const float y = body.row2[1];
            const float x = body.row2[0];
            const float z = body.row2[2];
            const float n = sqrt_004011d0(z * z + x * x + y * y);
            body.row2[0] = x / n;
            body.row2[1] = y / n;
            body.row2[2] = z / n;
        }
        body.row0[0] = body.row1[1] * body.row2[2] - body.row1[2] * body.row2[1];
        body.row0[1] = body.row2[0] * body.row1[2] - body.row2[2] * body.row1[0];
        body.row0[2] = body.row1[0] * body.row2[1] - body.row1[1] * body.row2[0];
        {
            const float x = body.row0[0];
            const float n = sqrt_004011d0(x * x + body.row0[1] * body.row0[1] +
                                          body.row0[2] * body.row0[2]);
            body.row0[0] = x / n;
            body.row0[1] = body.row0[1] / n;
            body.row0[2] = body.row0[2] / n;
        }
        body.row1[0] = body.row0[2] * body.row2[1] - body.row2[2] * body.row0[1];
        body.row1[1] = body.row0[0] * body.row2[2] - body.row2[0] * body.row0[2];
        body.row1[2] = body.row0[1] * body.row2[0] - body.row2[1] * body.row0[0];
    }

    // Damping again, angular first this time, then linear.
    float d = damping_factor(1.0f - dt * m.angular_damping);
    m.angular_velocity.x = d * m.angular_velocity.x;
    m.angular_velocity.y = m.angular_velocity.y * d;
    m.angular_velocity.z = d * m.angular_velocity.z;

    d = damping_factor(1.0f - m.linear_damping * dt);
    const float lx = d * m.linear_velocity.x;
    m.linear_velocity.x = lx;
    const float ly = m.linear_velocity.y * d;
    m.linear_velocity.y = ly;
    const float lz = m.linear_velocity.z * d;
    m.linear_velocity.z = lz;

    // The two speed clamps. Both compare squared magnitudes against the squared limit.
    const float wx = m.angular_velocity.x;
    const float wy = m.angular_velocity.y;
    const float wz = m.angular_velocity.z;
    const float linear_sq = lz * lz + lx * lx + ly * ly;
    const float linear_limit = m.max_linear_speed;
    if (linear_limit * linear_limit < linear_sq) {
        const float n = sqrt_004011d0(linear_sq);
        m.linear_velocity.x = linear_limit * (m.linear_velocity.x / n);
        m.linear_velocity.y = (m.linear_velocity.y / n) * linear_limit;
        m.linear_velocity.z = linear_limit * (m.linear_velocity.z / n);
    }
    const float angular_limit = m.max_angular_speed;
    if (angular_limit * angular_limit < wz * wz + wx * wx + wy * wy) {
        const float n = sqrt_004011d0(m.angular_velocity.z * m.angular_velocity.z +
                                      m.angular_velocity.x * m.angular_velocity.x +
                                      m.angular_velocity.y * m.angular_velocity.y);
        m.angular_velocity.x = angular_limit * (m.angular_velocity.x / n);
        m.angular_velocity.y = (m.angular_velocity.y / n) * angular_limit;
        m.angular_velocity.z = angular_limit * (m.angular_velocity.z / n);
    }

    // The sleep countdown. Above either world threshold the body is woken and the counter
    // reloaded; below both it counts down and, once negative, the velocities are scaled by
    // 0.9 every substep and bit 1 is set.
    const float sleep_w = world.sleep_angular_speed;
    const float sleep_v = world.sleep_linear_speed;
    const bool awake =
        (sleep_w * sleep_w < m.angular_velocity.z * m.angular_velocity.z +
                                 m.angular_velocity.x * m.angular_velocity.x +
                                 m.angular_velocity.y * m.angular_velocity.y) ||
        (sleep_v * sleep_v < m.linear_velocity.z * m.linear_velocity.z +
                                 m.linear_velocity.y * m.linear_velocity.y +
                                 m.linear_velocity.x * m.linear_velocity.x);
    if (awake) {
        body.flags &= ~kDynBodyWakeMask;
        body.sleep_countdown = world.sleep_countdown_reload;
    } else {
        body.sleep_countdown = body.sleep_countdown - 1;
        if (body.sleep_countdown < 0) {
            m.linear_velocity.x = m.linear_velocity.x * kDynSleepVelocityScale;
            m.linear_velocity.y = m.linear_velocity.y * kDynSleepVelocityScale;
            m.linear_velocity.z = m.linear_velocity.z * kDynSleepVelocityScale;
            m.angular_velocity.x = m.angular_velocity.x * kDynSleepVelocityScale;
            m.angular_velocity.y = m.angular_velocity.y * kDynSleepVelocityScale;
            m.angular_velocity.z = kDynSleepVelocityScale * m.angular_velocity.z;
            body.flags |= kDynBodyFlagAsleep;
        }
    }

    // The accumulators are cleared at the end of EVERY substep, which is why a force the
    // game side pushes once per 0.05 s game step is only integrated by the first substep.
    m.force = OceanVec3{};
    m.torque = OceanVec3{};
    m.linear_bias = OceanVec3{};
    m.angular_bias = OceanVec3{};
}

void dyn_body_substep(DynBody& body, const DynWorldStepConstants& world, float dt) noexcept {
    // 00C5BB30 runs 00C41550 first, then the collision and constraint phases this packet
    // does not reconstruct, then 00C5B1B0 under the "UpdatePosition" profiler scope.
    dyn_body_integrate_velocity_00c41550(body, world, dt);
    dyn_body_integrate_position_00c5b1b0(body, world, dt);
}

DynWorldSubstepPlan dyn_world_substep_plan_00c5c540(float fixed_substep, float budget,
                                                    float& accumulator, float dt) noexcept {
    // 00C5C68B..00C5C6C6. `budget` is world+34h, a float the routine decrements through an
    // integer cast; the loop stops when it reaches zero, leaving the accumulator cleared.
    DynWorldSubstepPlan plan{};
    plan.substep_dt = fixed_substep;

    float remaining_budget = budget;
    const float previous = accumulator;
    accumulator = dt + previous;
    float step = fixed_substep;
    if (step < dt + previous) {
        do {
            if (remaining_budget == 0.0f) {
                accumulator = 0.0f;
                return plan;
            }
            plan.full_substeps = plan.full_substeps + 1;
            step = fixed_substep;
            remaining_budget = static_cast<float>(static_cast<int>(remaining_budget) - 1);
            accumulator = accumulator - step;
        } while (fixed_substep < accumulator);
    }
    if (remaining_budget != 0.0f && kDynRemainderSubstepMin < static_cast<double>(accumulator)) {
        plan.remainder_substep = true;
        plan.remainder_dt = accumulator;
    }
    accumulator = 0.0f;
    return plan;
}

}  // namespace bsp
