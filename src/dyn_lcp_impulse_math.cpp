#include "bsp/dyn_lcp_impulse_math.hpp"
#include "bsp/hull_terrain_contact.hpp"  // kDynWorkerPrecisionBound

#include <cmath>

// Reconstruction of the Dyn solver's per-constraint impulse math.
// docs/DYN_LCP_IMPULSE_MATH.md carries the addresses, the original ABI and the
// uncertainty. Semantic C++ for MSVC Win32, not a drop-in binary replacement.
//
// Everything is float32. The native code is x87 (FLD/FMUL/FADDP) with the control word
// at 24-bit precision (docs/X87_CONTROL_WORD.md) and stores each partial sum back to a
// float temporary (00C4264F FSTP float ptr / 00C42653 FLD float ptr), so a float-typed
// C++ expression rounds at the same points. The grouping below follows the listing.

namespace bsp {
namespace {

// -0.0f, the float at 00D7A208 that 00C4E6AC and friends subtract from to negate.
constexpr float kNegativeZero = -0.0f;

inline float abs_float(float v) noexcept { return std::fabs(v); }  // BSP_Math_AbsFloat, 00401170
inline float sqrt_float(float v) noexcept { return std::sqrt(v); } // FUN_004011D0, 004011DC

inline void add_scaled3(float dst[3], const float src[3], float scale) noexcept {
    dst[0] = dst[0] + src[0] * scale;
    dst[1] = dst[1] + src[1] * scale;
    dst[2] = dst[2] + src[2] * scale;
}

// p' = px*row0 + py*row1 + pz*row2 + position, the 3x4 at B+08h/+14h/+20h/+2Ch.
// Both bodies inside 00C4DF5F..00C4E092; body B's third row is 00C4E08C..00C4E092.
void transform_point(const DynSolverBodyInput& body, const float local[3],
                     float world[3]) noexcept {
    world[0] = local[1] * body.row1[0] + local[0] * body.row0[0] +
               local[2] * body.row2[0] + body.position[0];
    world[1] = body.row2[1] * local[2] + body.row0[1] * local[0] +
               body.row1[1] * local[1] + body.position[1];
    world[2] = local[2] * body.row2[2] + body.row0[2] * local[0] +
               body.row1[2] * local[1] + body.position[2];
}

// wA x rA and wB x rB, inside 00C4E4A6..00C4E5CD (00C4E4AC/00C4E4B2/00C4E4B6 is one term).
void cross(const float a[3], const float b[3], float out[3]) noexcept {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

// The twelve-term J . u the row builder subtracts, in the listing's group order
// (00C4E6BE angular A, 00C4E6E8 linear A, then linear B and angular B).
float jacobian_dot_velocity(const float j[12], const DynSolverBodyInput& a,
                            const DynSolverBodyInput& b) noexcept {
    float sum = dyn_row_group_dot(j[3], j[4], j[5], a.angular_velocity[0],
                                  a.angular_velocity[1], a.angular_velocity[2]);
    sum = sum + dyn_row_group_dot(j[0], j[1], j[2], a.linear_velocity[0],
                                  a.linear_velocity[1], a.linear_velocity[2]);
    sum = sum + dyn_row_group_dot(j[6], j[7], j[8], b.linear_velocity[0],
                                  b.linear_velocity[1], b.linear_velocity[2]);
    sum = sum + dyn_row_group_dot(j[9], j[10], j[11], b.angular_velocity[0],
                                  b.angular_velocity[1], b.angular_velocity[2]);
    return sum;
}

// 00C4DE40 for one point, associated as the listing is (kDynListingAssociationBound) and,
// with R = double, holding what the listing keeps on the x87 stack at a worker's 53 bits
// (kDynWorkerPrecisionBound). Every FSTP to a float is a sp() here; anything without one
// stayed on the stack. With R = float every operation rounds, as at 24 bits. The chains
// were read with local\p2_x87sym.py over the disk bytes; docs/GUNNERY_OPEN_ITEMS.md 183
// lists them. A single operation on two floats rounds the same at either precision, so
// those stay plain float arithmetic.
template <typename R>
void build_contact_rows_listing(const DynConstraintBuildInput& input,
                                const DynSolverWorldSettings& settings, float dt,
                                DynConstraintRow& normal_row,
                                DynConstraintRow& friction_row) noexcept {
    const auto sp = [](R v) noexcept { return static_cast<float>(v); };
    // (j1*v1 + j0*v0) + j2*v2, spilled once (00C4E6E0, 00C4E78E and their siblings).
    const auto dot3 = [&](const float* j, const float* v) noexcept {
        return sp((R(j[1]) * v[1] + R(j[0]) * v[0]) + R(j[2]) * v[2]);
    };
    const DynSolverBodyInput& a = input.body_a;
    const DynSolverBodyInput& b = input.body_b;
    const DynSolverContactPoint& point = input.point;

    // 00C4DFBA..00C4E0A8: ((row0*l0 + row1*l1) + row2*l2) + position, one spill each.
    const auto to_world = [&](const DynSolverBodyInput& body, const float* l,
                              float* w) noexcept {
        for (int k = 0; k < 3; ++k) {
            w[k] = sp(((R(body.row0[k]) * l[0] + R(body.row1[k]) * l[1]) +
                       R(body.row2[k]) * l[2]) + body.position[k]);
        }
    };
    float world_a[3];
    float world_b[3];
    to_world(a, point.local_point_a, world_a);
    to_world(b, point.local_point_b, world_b);
    const float ra[3] = {world_a[0] - a.position[0], world_a[1] - a.position[1],
                         world_a[2] - a.position[2]};
    const float rb[3] = {world_b[0] - b.position[0], world_b[1] - b.position[1],
                         world_b[2] - b.position[2]};

    normal_row.impulse = settings.warm_start_scale * point.normal_impulse;
    normal_row.impulse_bias = point.bias_impulse;
    normal_row.body_a = a.solver_index;
    normal_row.body_b = b.solver_index;
    friction_row.body_a = a.solver_index;
    friction_row.body_b = b.solver_index;

    // 00C4E1F7..00C4E31E: each angular entry is one a*b - c*d, stored once. The friction
    // row's -(ra x t) (00C4EB99..00C4EC55) is the same value: negation is exact.
    const auto jacobian = [&](const float* n, float* j) noexcept {
        j[0] = kNegativeZero - n[0];
        j[1] = kNegativeZero - n[1];
        j[2] = kNegativeZero - n[2];
        const float nax = kNegativeZero - ra[0];
        const float nay = kNegativeZero - ra[1];
        const float naz = kNegativeZero - ra[2];
        j[3] = sp(R(nay) * n[2] - R(naz) * n[1]);
        j[4] = sp(R(naz) * n[0] - R(n[2]) * nax);
        j[5] = sp(R(nax) * n[1] - R(nay) * n[0]);
        j[6] = n[0];
        j[7] = n[1];
        j[8] = n[2];
        j[9] = sp(R(rb[1]) * n[2] - R(rb[2]) * n[1]);
        j[10] = sp(R(rb[2]) * n[0] - R(n[2]) * rb[0]);
        j[11] = sp(R(n[1]) * rb[0] - R(rb[1]) * n[0]);
    };
    // 00C4E380..00C4E3DB, 00C4E448..00C4E499 (and 00C4ED72.., 00C4EE30..):
    // (I[3+c]*j1 + I[c]*j0) + I[6+c]*j2 for column c, one spill each.
    const auto mass_jacobian = [&](const float* j, float* m) noexcept {
        m[0] = a.inverse_mass * j[0];
        m[1] = j[1] * a.inverse_mass;
        m[2] = a.inverse_mass * j[2];
        m[6] = b.inverse_mass * j[6];
        m[7] = j[7] * b.inverse_mass;
        m[8] = b.inverse_mass * j[8];
        for (int c = 0; c < 3; ++c) {
            m[3 + c] = sp((R(a.inverse_inertia[3 + c]) * j[4] + R(a.inverse_inertia[c]) * j[3]) +
                          R(a.inverse_inertia[6 + c]) * j[5]);
            m[9 + c] = sp((R(b.inverse_inertia[3 + c]) * j[10] + R(b.inverse_inertia[c]) * j[9]) +
                          R(b.inverse_inertia[6 + c]) * j[11]);
        }
    };
    // 00C4E6BE..00C4E74F and 00C4EE84..00C4EEFB: the four group dots are spilled, their
    // sum stays on the stack.
    const auto jacobian_dot = [&](const float* j) noexcept {
        return ((R(dot3(j + 3, a.angular_velocity)) + dot3(j + 0, a.linear_velocity)) +
                dot3(j + 6, b.linear_velocity)) + dot3(j + 9, b.angular_velocity);
    };
    // 00C4E773..00C4E7FC and 00C4EF2D..00C4EF99: 1 / the stacked sum of spilled groups.
    const auto effective_mass = [&](const float* j, const float* m) noexcept {
        return sp(R(1) / (((R(dot3(j + 3, m + 3)) + dot3(j + 0, m + 0)) + dot3(j + 6, m + 6)) +
                          dot3(j + 9, m + 9)));
    };

    jacobian(point.normal, normal_row.jacobian);
    mass_jacobian(normal_row.jacobian, normal_row.mass_jacobian);

    // 00C4E4A6..00C4E63B: w x r one spill per component, then each add spilled.
    const auto cross_arm = [&](const float* w, const float* r, float* out) noexcept {
        out[0] = sp(R(r[2]) * w[1] - R(r[1]) * w[2]);
        out[1] = sp(R(r[0]) * w[2] - R(r[2]) * w[0]);
        out[2] = sp(R(r[1]) * w[0] - R(r[0]) * w[1]);
    };
    float wxra[3];
    float wxrb[3];
    cross_arm(a.angular_velocity, ra, wxra);
    cross_arm(b.angular_velocity, rb, wxrb);
    float relative[3];
    for (int k = 0; k < 3; ++k) {
        relative[k] = ((wxrb[k] + b.linear_velocity[k]) - a.linear_velocity[k]) - wxra[k];
    }
    const float* n = point.normal;
    // 00C4E655..00C4E671, then 00C4E675 FMUL restitution and 00C4E67C FADD the double 0.05.
    const float normal_velocity =
        sp((R(relative[0]) * n[0] + R(relative[1]) * n[1]) + R(relative[2]) * n[2]);
    const R bounce = R(normal_velocity) * input.restitution;
    float target = static_cast<float>(static_cast<double>(bounce) + 0.05);
    if (!(target < 0.0f)) target = 0.0f;  // 00C4E68A FLDZ / FCOMIP / JBE: zero unless below

    normal_row.rhs_velocity = sp(R(kNegativeZero - target) - jacobian_dot(normal_row.jacobian));
    float clamped;
    if (point.depth >= 0.0f) {
        clamped = point.depth;
        if (settings.max_correction_depth < point.depth) clamped = settings.max_correction_depth;
    } else {
        clamped = 0.0f;
    }
    normal_row.rhs_bias = sp((R(settings.position_correction) * clamped) / dt);  // 00C4E63F..4E651
    normal_row.effective_mass = effective_mass(normal_row.jacobian, normal_row.mass_jacobian);
    normal_row.friction_coefficient = input.friction;

    friction_row.impulse = 0.0f;
    friction_row.impulse_bias = 0.0f;
    friction_row.rhs_bias = 0.0f;
    friction_row.friction_coefficient = 0.0f;

    // 00C4E828..00C4E932: the perpendicular, as dyn_friction_direction builds it.
    const float an0 = abs_float(n[0]);
    const float an1 = abs_float(n[1]);
    float largest = an0;
    unsigned int axis = 0;
    if (an0 < an1) {
        largest = an1;
        axis = 1;
    }
    if (largest < abs_float(n[2])) axis = 2;
    const unsigned int bit = 1u << axis;
    const unsigned int other = bit & 3u;
    float perpendicular[3] = {n[0], n[1], n[2]};
    const float swapped = perpendicular[axis];
    perpendicular[axis] = perpendicular[other];
    perpendicular[other] = swapped;
    perpendicular[axis] = kNegativeZero - perpendicular[axis];
    perpendicular[(1u << (bit & 3u)) & 3u] = 0.0f;
    // 00C4E90F..00C4E92B: (p1*p1 + p0*p0) + p2*p2, spilled, then 004011D0.
    float length = sqrt_float(sp((R(perpendicular[1]) * perpendicular[1] +
                                  R(perpendicular[0]) * perpendicular[0]) +
                                 R(perpendicular[2]) * perpendicular[2]));
    const float px = perpendicular[0] / length;
    const float py = perpendicular[1] / length;
    const float pz = perpendicular[2] / length;
    // 00C4E96F..00C4E9C9, then the two projections 00C4E9D0..00C4E9FF and ..00C4EA49.
    const float qx = sp(R(py) * n[2] - R(pz) * n[1]);
    const float qy = sp(R(pz) * n[0] - R(n[2]) * px);
    const float qz = sp(R(n[1]) * px - R(n[0]) * py);
    const float along_q = sp((R(qy) * relative[1] + R(qx) * relative[0]) + R(qz) * relative[2]);
    const float along_p = sp((R(relative[0]) * px + R(relative[1]) * py) + R(pz) * relative[2]);
    const float tx = px * along_p + qx * along_q;  // each product and the sum spilled
    const float ty = py * along_p + qy * along_q;
    const float tz = pz * along_p + qz * along_q;
    const float length_sq = sp((R(tx) * tx + R(ty) * ty) + R(tz) * tz);  // 00C4EAC6
    float tangent[3];
    if (length_sq > kDynFrictionDirectionEpsilonSq) {
        length = sqrt_float(length_sq);
        tangent[0] = tx / length;
        tangent[1] = ty / length;
        tangent[2] = tz / length;
    } else {
        tangent[0] = px;
        tangent[1] = py;
        tangent[2] = pz;
    }
    jacobian(tangent, friction_row.jacobian);
    mass_jacobian(friction_row.jacobian, friction_row.mass_jacobian);
    friction_row.rhs_velocity = sp(-jacobian_dot(friction_row.jacobian));  // 00C4EEFB FCHS
    friction_row.effective_mass =
        effective_mass(friction_row.jacobian, friction_row.mass_jacobian);
}

}  // namespace

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

float dyn_row_group_dot(float j0, float j1, float j2, float v0, float v1,
                        float v2) noexcept {
    // 00C42638 FMULP gives j1*v1 first, 00C4263A..00C42641 folds in j0*v0, then
    // 00C42643..00C4264B folds in j2*v2.
    if constexpr (kDynWorkerPrecisionBound) {
        // The three products and two sums stay on the x87 stack until the FSTP at
        // 00C4264F (00C4267F, 00C426AA, 00C426DC; 00C42320, 00C4234F in 00C42230): one
        // rounding to float at a worker's 53 bits. The float products are exact in double.
        return static_cast<float>((static_cast<double>(j1) * v1 + static_cast<double>(j0) * v0) +
                                  static_cast<double>(j2) * v2);
    } else {
        return (j1 * v1 + j0 * v0) + j2 * v2;
    }
}

float dyn_row_residual(float rhs, const float jacobian[12], const float linear_a[3],
                       const float angular_a[3], const float linear_b[3],
                       const float angular_b[3]) noexcept {
    float acc = rhs - dyn_row_group_dot(jacobian[0], jacobian[1], jacobian[2],
                                        linear_a[0], linear_a[1], linear_a[2]);
    acc = acc - dyn_row_group_dot(jacobian[3], jacobian[4], jacobian[5], angular_a[0],
                                  angular_a[1], angular_a[2]);
    acc = acc - dyn_row_group_dot(jacobian[6], jacobian[7], jacobian[8], linear_b[0],
                                  linear_b[1], linear_b[2]);
    acc = acc - dyn_row_group_dot(jacobian[9], jacobian[10], jacobian[11], angular_b[0],
                                  angular_b[1], angular_b[2]);
    return acc;
}

void dyn_row_apply_impulse(const float mass_jacobian[12], float delta, float linear_a[3],
                           float angular_a[3], float linear_b[3],
                           float angular_b[3]) noexcept {
    add_scaled3(linear_a, mass_jacobian + 0, delta);
    add_scaled3(angular_a, mass_jacobian + 3, delta);
    add_scaled3(linear_b, mass_jacobian + 6, delta);
    add_scaled3(angular_b, mass_jacobian + 9, delta);
}

float dyn_effective_mass(const float jacobian[12],
                         const float mass_jacobian[12]) noexcept {
    // 00C4E6AC..00C4E7FA, in the listing's summation order. The native code divides
    // unguarded (FLD1 at 00C4E7F8, FDIVRP at 00C4E7FA).
    float sum = jacobian[5] * mass_jacobian[5];
    sum = sum + jacobian[3] * mass_jacobian[3];
    sum = sum + jacobian[4] * mass_jacobian[4];
    sum = sum + jacobian[2] * mass_jacobian[2];
    sum = sum + jacobian[0] * mass_jacobian[0];
    sum = sum + jacobian[1] * mass_jacobian[1];
    sum = sum + jacobian[8] * mass_jacobian[8];
    sum = sum + jacobian[6] * mass_jacobian[6];
    sum = sum + jacobian[7] * mass_jacobian[7];
    sum = sum + jacobian[11] * mass_jacobian[11];
    sum = sum + jacobian[9] * mass_jacobian[9];
    sum = sum + jacobian[10] * mass_jacobian[10];
    return 1.0f / sum;
}

float dyn_restitution_target(float normal_velocity, float restitution) noexcept {
    // 00C4E675 FMUL by manifold+04h, 00C4E67C FADD the 0.05 double at 00D7A270,
    // 00C4E68A..00C4E6A3 keeps the result only while it is strictly negative.
    float target = normal_velocity * restitution + kDynRestitutionVelocityThreshold;
    if (target >= 0.0f) target = 0.0f;  // 00C4E690 JBE takes the zero arm
    return target;
}

float dyn_penetration_bias(float depth, float max_correction_depth,
                           float position_correction, float dt) noexcept {
    // 00C4E0DC..00C4E11F: a negative depth contributes nothing and a deeper overlap is
    // capped, so the bias rows never push harder than max_correction_depth per substep.
    float clamped;
    if (depth >= 0.0f) {
        clamped = depth;
        if (max_correction_depth < depth) clamped = max_correction_depth;
    } else {
        clamped = 0.0f;
    }
    // 00C4E63F FLD world+18h, 00C4E646 FMUL the clamped depth, 00C4E64A FDIV dt.
    return (position_correction * clamped) / dt;
}

void dyn_contact_jacobian(const float normal[3], const float ra[3], const float rb[3],
                          float jacobian[12]) noexcept {
    // Body A takes the negative half: 00C4E1F7 writes -n and 00C4E232
    // writes -(ra x n) as products of the negated lever arm.
    jacobian[0] = kNegativeZero - normal[0];
    jacobian[1] = kNegativeZero - normal[1];
    jacobian[2] = kNegativeZero - normal[2];

    const float nax = kNegativeZero - ra[0];
    const float nay = kNegativeZero - ra[1];
    const float naz = kNegativeZero - ra[2];
    jacobian[3] = nay * normal[2] - naz * normal[1];
    jacobian[4] = naz * normal[0] - nax * normal[2];
    jacobian[5] = nax * normal[1] - normal[0] * nay;

    // 00C4E2A0 writes +n, 00C4E2CA..00C4E31E writes rb x n.
    jacobian[6] = normal[0];
    jacobian[7] = normal[1];
    jacobian[8] = normal[2];
    jacobian[9] = rb[1] * normal[2] - rb[2] * normal[1];
    jacobian[10] = rb[2] * normal[0] - rb[0] * normal[2];
    jacobian[11] = normal[1] * rb[0] - rb[1] * normal[0];
}

void dyn_mass_jacobian(const float jacobian[12], float inverse_mass_a,
                       const float inverse_inertia_a[9], float inverse_mass_b,
                       const float inverse_inertia_b[9],
                       float mass_jacobian[12]) noexcept {
    // 00C4E356..00C4E37C and 00C4E43E..00C4E444: the linear halves are a scalar multiply.
    mass_jacobian[0] = inverse_mass_a * jacobian[0];
    mass_jacobian[1] = jacobian[1] * inverse_mass_a;
    mass_jacobian[2] = inverse_mass_a * jacobian[2];
    mass_jacobian[6] = inverse_mass_b * jacobian[6];
    mass_jacobian[7] = jacobian[7] * inverse_mass_b;
    mass_jacobian[8] = inverse_mass_b * jacobian[8];

    // 00C4E380..00C4E3D6 and 00C4E448..00C4E48C: the world inverse inertia at M+60h is
    // applied as a matrix with columns (+60h,+64h,+68h), (+6Ch,+70h,+74h), (+78h,+7Ch,+80h).
    mass_jacobian[3] = inverse_inertia_a[6] * jacobian[5] +
                       jacobian[3] * inverse_inertia_a[0] +
                       inverse_inertia_a[3] * jacobian[4];
    mass_jacobian[4] = inverse_inertia_a[7] * jacobian[5] +
                       inverse_inertia_a[1] * jacobian[3] +
                       inverse_inertia_a[4] * jacobian[4];
    mass_jacobian[5] = inverse_inertia_a[8] * jacobian[5] +
                       inverse_inertia_a[2] * jacobian[3] +
                       inverse_inertia_a[5] * jacobian[4];

    mass_jacobian[9] = inverse_inertia_b[6] * jacobian[11] +
                       jacobian[9] * inverse_inertia_b[0] +
                       inverse_inertia_b[3] * jacobian[10];
    mass_jacobian[10] = inverse_inertia_b[7] * jacobian[11] +
                        inverse_inertia_b[1] * jacobian[9] +
                        inverse_inertia_b[4] * jacobian[10];
    mass_jacobian[11] = inverse_inertia_b[8] * jacobian[11] +
                        inverse_inertia_b[2] * jacobian[9] +
                        inverse_inertia_b[5] * jacobian[10];
}

bool dyn_friction_direction(const float normal[3], const float relative_velocity[3],
                            float tangent[3]) noexcept {
    // 00C4E828, 00C4E844 and 00C4E882: the dominant axis of |n|.
    const float an0 = abs_float(normal[0]);
    const float an1 = abs_float(normal[1]);
    float largest = an0;
    unsigned int axis = 0;
    if (an0 < an1) {
        largest = an1;
        axis = 1;
    }
    if (largest < abs_float(normal[2])) axis = 2;

    // 00C4E88C..00C4E932: swap the dominant component with the one at (1 << axis) & 3,
    // negate it, and zero the third. The result is perpendicular to n for every axis.
    const unsigned int bit = 1u << axis;
    const unsigned int other = bit & 3u;
    float perpendicular[3] = {normal[0], normal[1], normal[2]};
    const float swapped = perpendicular[axis];
    perpendicular[axis] = perpendicular[other];
    perpendicular[other] = swapped;
    perpendicular[axis] = kNegativeZero - perpendicular[axis];
    perpendicular[(1u << (bit & 3u)) & 3u] = 0.0f;

    // 00C4E936 sqrt, 00C4E959..00C4E96B the three divides.
    float length = sqrt_float(perpendicular[2] * perpendicular[2] +
                              perpendicular[0] * perpendicular[0] +
                              perpendicular[1] * perpendicular[1]);
    float px = perpendicular[0] / length;
    float py = perpendicular[1] / length;
    float pz = perpendicular[2] / length;

    // 00C4E96F..00C4E9A0: q = p x n, the second tangent.
    const float qx = normal[2] * py - normal[1] * pz;
    const float qy = normal[0] * pz - px * normal[2];
    const float qz = px * normal[1] - py * normal[0];

    // 00C4E9A4..00C4EAC6: project the relative velocity onto the (q, p) plane.
    float along_q = qz * relative_velocity[2] + relative_velocity[1] * qy +
                    relative_velocity[0] * qx;
    float along_p = pz * relative_velocity[2] + relative_velocity[1] * py +
                    px * relative_velocity[0];
    const float tx = along_p * px + along_q * qx;
    const float ty = along_p * py + along_q * qy;
    const float tz = along_p * pz + along_q * qz;
    const float length_sq = tz * tz + ty * ty + tx * tx;

    // 00C4EACA compares with the 1e-6f at 00D7A288; below it the perpendicular is kept.
    if (length_sq > kDynFrictionDirectionEpsilonSq) {
        length = sqrt_float(length_sq);
        tangent[0] = tx / length;
        tangent[1] = ty / length;
        tangent[2] = tz / length;
        return true;
    }
    tangent[0] = px;
    tangent[1] = py;
    tangent[2] = pz;
    return false;
}

float dyn_combine_friction(float friction_a, float friction_b) noexcept {
    // 00C44154..00C4418B. Either shape may veto the product with a negative value.
    if (friction_a < 0.0f) return kNegativeZero - friction_a;
    if (friction_b < 0.0f) return kNegativeZero - friction_b;
    return friction_b * friction_a;
}

float dyn_combine_restitution(float restitution_a, float restitution_b) noexcept {
    // 00C4419D FLD shape B, 00C441A4 FADD shape A, 00C441AF FMUL the 0.5 at 00D7A280.
    return (restitution_b + restitution_a) * 0.5f;
}

// ---------------------------------------------------------------------------
// The batch routines
// ---------------------------------------------------------------------------

void dyn_apply_warm_start_00c42ba0(DynConstraintBatch& batch) noexcept {
    for (std::int32_t i = 0; i < batch.normal_row_count; ++i) {
        const DynConstraintRow& row = batch.rows[i];
        DynSolverBodyVelocity& a = batch.velocities[row.body_a];
        DynSolverBodyVelocity& b = batch.velocities[row.body_b];
        // 00C42BA4..00C42D2C, the accumulated impulse into the velocity pair.
        dyn_row_apply_impulse(row.mass_jacobian, row.impulse, a.linear, a.angular,
                              b.linear, b.angular);
        // 00C42D31..00C42EBC, the accumulated bias impulse into the pseudo pair.
        dyn_row_apply_impulse(row.mass_jacobian, row.impulse_bias, a.linear_bias,
                              a.angular_bias, b.linear_bias, b.angular_bias);
    }
}

void dyn_solve_normal_rows_00c42530(DynConstraintBatch& batch) noexcept {
    for (std::int32_t i = 0; i < batch.normal_row_count; ++i) {
        DynConstraintRow& row = batch.rows[i];
        DynSolverBodyVelocity& a = batch.velocities[row.body_a];
        DynSolverBodyVelocity& b = batch.velocities[row.body_b];

        // 00C42618..00C426F5, the velocity row.
        float residual = dyn_row_residual(row.rhs_velocity, row.jacobian, a.linear,
                                          a.angular, b.linear, b.angular);
        float next = residual * row.effective_mass + row.impulse;
        if (next < 0.0f) next = 0.0f;  // 00C42708..00C42716, the unilateral clamp
        const float delta = next - row.impulse;
        row.impulse = next;
        dyn_row_apply_impulse(row.mass_jacobian, delta, a.linear, a.angular, b.linear,
                              b.angular);

        // 00C4298E..00C429D4, the bias row: same Jacobian, same effective mass, the
        // second accumulator pair and its own right-hand side and accumulator.
        residual = dyn_row_residual(row.rhs_bias, row.jacobian, a.linear_bias,
                                    a.angular_bias, b.linear_bias, b.angular_bias);
        next = residual * row.effective_mass + row.impulse_bias;
        if (next < 0.0f) next = 0.0f;
        const float delta_bias = next - row.impulse_bias;
        row.impulse_bias = next;
        dyn_row_apply_impulse(row.mass_jacobian, delta_bias, a.linear_bias, a.angular_bias,
                              b.linear_bias, b.angular_bias);
    }
}

void dyn_solve_friction_rows_00c42230(DynConstraintBatch& batch) noexcept {
    const std::int32_t base = batch.friction_row_base;
    for (std::int32_t k = 0; k < batch.normal_row_count; ++k) {
        DynConstraintRow& row = batch.rows[base + k];
        DynSolverBodyVelocity& a = batch.velocities[row.body_a];
        DynSolverBodyVelocity& b = batch.velocities[row.body_b];

        float residual = dyn_row_residual(row.rhs_velocity, row.jacobian, a.linear,
                                          a.angular, b.linear, b.angular);
        float next = residual * row.effective_mass + row.impulse;

        // 00C42373..00C423BB: the limit is the matching normal row's coefficient times
        // its accumulated impulse, so friction tracks the normal load inside one
        // iteration rather than one substep behind it.
        const DynConstraintRow& normal_row = batch.rows[k];
        const float limit = normal_row.friction_coefficient * normal_row.impulse;
        if (next < -limit) {
            next = -limit;
        } else if (limit < next) {
            next = limit;
        }

        const float delta = next - row.impulse;
        row.impulse = next;
        // 00C423F0..00C424D6: the first accumulator pair only. Friction never touches
        // the pseudo-velocity, so it cannot push bodies apart positionally.
        dyn_row_apply_impulse(row.mass_jacobian, delta, a.linear, a.angular, b.linear,
                              b.angular);
    }
}

void dyn_solve_group_00403720(DynConstraintBatch& batch,
                              std::int32_t iterations) noexcept {
    for (std::int32_t i = 0; i < iterations; ++i) {
        dyn_solve_normal_rows_00c42530(batch);
        dyn_solve_friction_rows_00c42230(batch);
    }
}

// The order used before kDynListingAssociationBound, kept as the OFF arm.
static void build_contact_rows_before(const DynConstraintBuildInput& input,
                                      const DynSolverWorldSettings& settings, float dt,
                                      DynConstraintRow& normal_row,
                                      DynConstraintRow& friction_row) noexcept {
    const DynSolverBodyInput& a = input.body_a;
    const DynSolverBodyInput& b = input.body_b;
    const DynSolverContactPoint& point = input.point;

    float world_a[3];
    float world_b[3];
    transform_point(a, point.local_point_a, world_a);
    transform_point(b, point.local_point_b, world_b);

    // 00C4E096..00C4E11C: the lever arms are the world contact point minus each body's
    // origin, which is the same subtraction the transform just added.
    const float ra[3] = {world_a[0] - a.position[0], world_a[1] - a.position[1],
                         world_a[2] - a.position[2]};
    const float rb[3] = {world_b[0] - b.position[0], world_b[1] - b.position[1],
                         world_b[2] - b.position[2]};

    // 00C4E0AF..00C4E0CE: the warm start. The stored normal impulse is attenuated by
    // world+20h, the stored bias impulse is not.
    normal_row.impulse = settings.warm_start_scale * point.normal_impulse;
    normal_row.impulse_bias = point.bias_impulse;

    // 00C4E11F/00C4E138 and 00C4EB2B/00C4EB57: both rows carry the same body pair.
    normal_row.body_a = a.solver_index;
    normal_row.body_b = b.solver_index;
    friction_row.body_a = a.solver_index;
    friction_row.body_b = b.solver_index;

    dyn_contact_jacobian(point.normal, ra, rb, normal_row.jacobian);
    dyn_mass_jacobian(normal_row.jacobian, a.inverse_mass, a.inverse_inertia,
                      b.inverse_mass, b.inverse_inertia, normal_row.mass_jacobian);

    // 00C4E4A6..00C4E5CD: (vB + wB x rB) - (vA + wA x rA).
    float wxra[3];
    float wxrb[3];
    cross(a.angular_velocity, ra, wxra);
    cross(b.angular_velocity, rb, wxrb);
    const float relative[3] = {
        (b.linear_velocity[0] + wxrb[0]) - a.linear_velocity[0] - wxra[0],
        (b.linear_velocity[1] + wxrb[1]) - a.linear_velocity[1] - wxra[1],
        (b.linear_velocity[2] + wxrb[2]) - a.linear_velocity[2] - wxra[2]};

    const float normal_velocity =
        relative[2] * point.normal[2] + relative[1] * point.normal[1] +
        relative[0] * point.normal[0];
    const float target = dyn_restitution_target(normal_velocity, input.restitution);

    // 00C4E6B2..00C4E763: the row's right-hand side already carries -(J . u), so the
    // iteration's accumulators start at zero and hold velocity *changes*.
    normal_row.rhs_velocity = (kNegativeZero - target) - jacobian_dot_velocity(
                                                             normal_row.jacobian, a, b);
    normal_row.rhs_bias = dyn_penetration_bias(point.depth, settings.max_correction_depth,
                                               settings.position_correction, dt);
    normal_row.effective_mass =
        dyn_effective_mass(normal_row.jacobian, normal_row.mass_jacobian);
    // 00C4EFA8..00C4EFB9, written last, after the friction row.
    normal_row.friction_coefficient = input.friction;

    // 00C4E807: friction rows are never warm started.
    friction_row.impulse = 0.0f;
    friction_row.impulse_bias = 0.0f;
    friction_row.rhs_bias = 0.0f;
    friction_row.friction_coefficient = 0.0f;

    float tangent[3];
    dyn_friction_direction(point.normal, relative, tangent);
    dyn_contact_jacobian(tangent, ra, rb, friction_row.jacobian);
    dyn_mass_jacobian(friction_row.jacobian, a.inverse_mass, a.inverse_inertia,
                      b.inverse_mass, b.inverse_inertia, friction_row.mass_jacobian);
    // 00C4EEBE..00C4EF7C: no restitution term on the friction row.
    friction_row.rhs_velocity =
        kNegativeZero - jacobian_dot_velocity(friction_row.jacobian, a, b);
    friction_row.effective_mass =
        dyn_effective_mass(friction_row.jacobian, friction_row.mass_jacobian);
}

void dyn_build_contact_rows_00c4de40(const DynConstraintBuildInput& input,
                                     const DynSolverWorldSettings& settings, float dt,
                                     DynConstraintRow& normal_row,
                                     DynConstraintRow& friction_row) noexcept {
    if constexpr (kDynWorkerPrecisionBound) {
        build_contact_rows_listing<double>(input, settings, dt, normal_row, friction_row);
    } else if constexpr (kDynListingAssociationBound) {
        build_contact_rows_listing<float>(input, settings, dt, normal_row, friction_row);
    } else {
        build_contact_rows_before(input, settings, dt, normal_row, friction_row);
    }
}

void dyn_write_back_velocities_00c37b50(const DynSolverBodyVelocity* velocities,
                                        std::int32_t velocity_count,
                                        DynSolverVelocityWriteBack* bodies) noexcept {
    // 00C37B51 sets the index to 1 and 00C37B56 tests it against context+0EAA0h, so the
    // written back.
    for (std::int32_t i = 1; i < velocity_count; ++i) {
        const DynSolverBodyVelocity& v = velocities[i];
        DynSolverVelocityWriteBack& out = bodies[i];
        add_scaled3(out.linear_velocity, v.linear, 1.0f);
        add_scaled3(out.angular_velocity, v.angular, 1.0f);
        add_scaled3(out.linear_bias, v.linear_bias, 1.0f);
        add_scaled3(out.angular_bias, v.angular_bias, 1.0f);
    }
}

void dyn_store_impulses_00c35020(const DynConstraintBatch& batch,
                                 DynSolverContactPoint* const* points_per_row,
                                 std::int32_t row_count) noexcept {
    const std::int32_t count =
        row_count < batch.normal_row_count ? row_count : batch.normal_row_count;
    for (std::int32_t i = 0; i < count; ++i) {
        DynSolverContactPoint* point = points_per_row[i];
        if (point == nullptr) continue;
        // 00C3506B writes point+24h from the impulse array, 00C35076 point+28h from the
        // bias array. Neither is scaled on the way out.
        point->normal_impulse = batch.rows[i].impulse;
        point->bias_impulse = batch.rows[i].impulse_bias;
    }
}

}  // namespace bsp
