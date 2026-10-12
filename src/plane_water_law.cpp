#include "bsp/plane_water_law.hpp"

#include "bsp/unit_rudder.hpp"

#include <cmath>

// Packet cc9_plane_water_law. docs/WATER_SURFACE_LAW.md section 10 carries the read;
// the comments below name the listing site of each term. Float arithmetic throughout:
// the plane laws run on the main thread at the 24-bit precision control.

namespace bsp {
namespace {

constexpr double kHalfPi = 1.5707963705062866;   // 00CE3830, double of float pi/2
constexpr double kTwoPi = 6.2831854820251465;    // 00CE3828
constexpr double kGravity = 9.8100004196167;     // 00CF9058
constexpr float kKickLimit = 1000.0f;            // 00CE3804

// 00415620 BSP_Math_ClampFloatByRef(value, low, high): low when low > value, then
// high when value > high.
float clamp_by_ref_00415620(float value, float low, float high) {
    if (low > value) return low;
    if (value > high) return high;
    return value;
}

// 00415550 BSP_Math_MaxFloatByRef(a, b): b when a <= b (or unordered), else a.
float max_by_ref_00415550(float a, float b) { return a > b ? a : b; }

// 00415510 BSP_Math_MinFloatByRef(a, b): b when b <= a (or unordered), else a.
float min_by_ref_00415510(float a, float b) { return a < b ? a : b; }

// 00414C60 BSP_Vector2f_LengthWithCutoff: the float sum against double 1e-10.
float length_2d_00414c60(float x, float y) {
    const float s = x * x + y * y;
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

// 00419260 BSP_Vector2f_ReciprocalLength: zero for a zero length.
float reciprocal_length_2d_00419260(float x, float y) {
    const float len = static_cast<float>(std::sqrt(static_cast<double>(x * x + y * y)));
    if (len == 0.0f) return 0.0f;
    return 1.0f / len;
}

// 00419440 BSP_Vector3f_Length: (y*y + x*x) + z*z, no threshold.
float length_3d_00419440(const float v[3]) {
    const float s = (v[1] * v[1] + v[0] * v[0]) + v[2] * v[2];
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

// 0042B2F0 BSP_Vector3_LengthFloatThreshold: zero at or below double 1e-10.
float length_3d_0042b2f0(const float v[3]) {
    const float s = (v[1] * v[1] + v[0] * v[0]) + v[2] * v[2];
    if (!(static_cast<double>(s) > 1e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

// 0042B260 BSP_Geometry_NormalizeVectorWithFloor: divide by float(1e-5) when the
// squared length is at or below double 1e-10 (or unordered).
void normalize_0042b260(float v[3]) {
    const float s = (v[0] * v[0] + v[1] * v[1]) + v[2] * v[2];
    const float d = static_cast<double>(s) > 1e-10
        ? static_cast<float>(std::sqrt(static_cast<double>(s)))
        : static_cast<float>(1e-5);
    v[0] = v[0] / d;
    v[1] = v[1] / d;
    v[2] = v[2] / d;
}

// 0042D0D0 with normalise false: out = v * M over the three basis rows.
void transform_direction(float out[3], const PlaneWaterPose& pose, const float v[3]) {
    for (int c = 0; c < 3; ++c) {
        out[c] = v[0] * pose.rows[0][c] + v[1] * pose.rows[1][c] + v[2] * pose.rows[2][c];
    }
}

// 004F9B30 BSP_Vector3f_Cross(out, a = EDX, b = stack).
void cross_004f9b30(float out[3], const float a[3], const float b[3]) {
    const float x = a[1] * b[2] - a[2] * b[1];
    const float y = b[0] * a[2] - a[0] * b[2];
    const float z = b[1] * a[0] - a[1] * b[0];
    out[0] = x;
    out[1] = y;
    out[2] = z;
}

// The heading the law compares: pi/2 - atan2(y, x), wrapped into [0, 2pi) once
// (007DD0D8-007DD103 and its three copies).
float compass_heading(float y, float x) {
    const float a = static_cast<float>(std::atan2(static_cast<double>(y),
                                                  static_cast<double>(x)));
    float h = static_cast<float>(kHalfPi - static_cast<double>(a));
    if (0.0f > h) h = static_cast<float>(static_cast<double>(h) + kTwoPi);
    return h;
}

// |x|^e through FYL2X / F2XM1 / FSCALE, zero for x == 0 (007DD4C6-007DD514).
float pow_abs(float x, float e) {
    if (x == 0.0f) return 0.0f;
    if (0.0f > x) x = -0.0f - x;   // 00D7A208 is -0.0f
    return static_cast<float>(std::pow(static_cast<double>(x), static_cast<double>(e)));
}

}  // namespace

void water_surface_normal_0078d3d0(float out[3], float x, float z,
                                   const PlaneWaterHeightSampler& height) {
    // 0078D3F3 / 0078D41E: x +- 1.0 as double, stored float for the call.
    const float xp = static_cast<float>(static_cast<double>(x) + 1.0);
    const float xm = static_cast<float>(static_cast<double>(x) - 1.0);
    const float zp = static_cast<float>(static_cast<double>(z) + 1.0);
    const float zm = static_cast<float>(static_cast<double>(z) - 1.0);
    const float dhx = height(xp, z) - height(xm, z);   // 0078D434 FSUBR, float 0078D440
    const float dhz = height(x, zp) - height(x, zm);   // 0078D49D, float 0078D4B2
    const float a[3] = {1.0f, dhx, 0.0f};              // [ESP+10h..18h]
    const float b[3] = {0.0f, dhz, 1.0f};              // [ESP+1Ch..24h]
    float n[3];
    cross_004f9b30(n, b, a);                           // 0078D4C0, EDX = b
    const float len = length_3d_00419440(n);           // 0078D4E9
    const float inv = len > 0.0f ? 1.0f / len : 0.0f;
    out[0] = inv * n[0];
    out[1] = n[1] * inv;
    out[2] = inv * n[2];
}

PlaneWaterLawSample water_law_sample_007dcdd0(const PlaneWaterLawClass& cls,
                                              const PlaneWaterPose& pose,
                                              const PlaneWaterHeightSampler& height) {
    PlaneWaterLawSample out;
    float n[3];
    float d0 = 0.0f;
    if (cls.min_water_spd_198 == 0.0f) {
        // 007DCE04-007DCEDA: the origin and the body points (8, 0, 0), (-8, 0, 0)
        // through 004142E0 (v * M + translation).
        const float* const p0 = pose.position;
        float p1[3];
        float p2[3];
        for (int c = 0; c < 3; ++c) {
            p1[c] = 8.0f * pose.rows[0][c] + pose.position[c];    // 00CE3918
            p2[c] = -8.0f * pose.rows[0][c] + pose.position[c];   // 00CE3CC8
        }
        d0 = p0[1] - height(p0[0], p0[2]);   // 007DCF0A, float [ESP+1Ch]
        const float d1 = p1[1] - height(p1[0], p1[2]);
        const float d2 = p2[1] - height(p2[0], p2[2]);
        // 007DCF81-007DD00B: c = clamp((d_tip - d0) * 4.0, 0, 1). 007DCF89 is the DC
        // form (ST2 = ST2 * ST0).
        float c1 = (d1 - d0) * 4.0f;
        c1 = 0.0f > c1 ? 0.0f : (c1 > 1.0f ? 1.0f : c1);
        float c2 = (d2 - d0) * 4.0f;
        c2 = 0.0f > c2 ? 0.0f : (c2 > 1.0f ? 1.0f : c2);
        water_surface_normal_0078d3d0(n, p0[0], p0[2], height);   // 007DD034
        const float* const tips[2] = {p1, p2};
        const float weights[2] = {c1, c2};
        for (int k = 0; k < 2; ++k) {
            // 007DD044 / 007DD1C0: COMISS against 0.0f (00D7A218).
            if (!(weights[k] > 0.0f)) continue;
            const float dx = tips[k][0] - p0[0];
            const float dz = tips[k][2] - p0[2];
            const float r = reciprocal_length_2d_00419260(dx, dz);   // 007DD0A4
            const float ux = r * dx;
            const float uz = r * dz;
            const float horizontal = length_2d_00414c60(n[0], n[2]);   // 007DD0C7
            const float to_tip = compass_heading(uz, ux);             // 007DD0D8
            const float of_normal = compass_heading(n[2], n[0]);      // 007DD10F
            const float delta = wrapped_angle_subtract_00438b10(to_tip, of_normal);  // 007DD14C
            float cosine = static_cast<float>(std::cos(static_cast<double>(delta)));
            if (0.0f > cosine) cosine = 0.0f;
            const float w = cosine * horizontal * weights[k];   // 007DD17C-007DD188
            if (w > 0.0f) {
                n[0] = n[0] - ux * w;   // 007DD198-007DD1A4
                n[2] = n[2] - w * uz;   // 007DD1A8-007DD1B2
            }
        }
    } else {
        // 007DD349-007DD3F7: the origin alone.
        water_surface_normal_0078d3d0(n, pose.position[0], pose.position[2], height);
        d0 = pose.position[1] - height(pose.position[0], pose.position[2]);
    }
    // 007DD401 FCHS: depth = -(d0 - SwimHeight).
    out.depth = -(d0 - cls.swim_height_194);
    const float len = length_3d_00419440(n);   // 007DD40B
    const float inv = len > 0.0f ? 1.0f / len : 0.0f;
    out.normal[0] = inv * n[0];
    out.normal[1] = n[1] * inv;
    out.normal[2] = inv * n[2];
    return out;
}

PlaneWaterBandTerms water_band_007dc205(const PlaneWaterBandInputs& in,
                                        const PlaneWaterLawClass& cls,
                                        const PlaneWaterLawTuning& tuning) {
    PlaneWaterBandTerms out;
    // 007DC208-007DC243: the DeadMeat fade, 1 - C3Ch * 0.5, floored at 0.
    float fade = static_cast<float>(1.0 - static_cast<double>(in.lost_drag_timer_c3c) * 0.5);
    if (0.0f > fade) fade = 0.0f;
    float lift;          // [ESP+1Ch]
    float descent = 0.0f;  // [ESP+14h]
    float m;             // ST0 at 007DC34A
    bool sinking_check;
    if (in.depth > 0.0f) {
        // 007DC25B-007DC2F4: (depth + 1)^LiftDepthRatio clamped to [1, LiftMax].
        const float x = 1.0f + in.depth;
        lift = clamp_by_ref_00415620(pow_abs(x, tuning.lift_depth_ratio), 1.0f, tuning.lift_max);
        m = 1.0f;
        sinking_check = true;
    } else {
        // 007DC2F6-007DC348: max(0, depth / (desc+508h * LiftBeginDiveMul) + 1)^2.
        const float q = cls.derived_508 * tuning.lift_begin_dive_mul;
        const float t = in.depth / q + 1.0f;
        m = max_by_ref_00415550(0.0f, t);
        m = m * m;
        lift = m;
        sinking_check = m > 0.0f;
    }
    if (sinking_check && 0.0f > in.world_vy_acc) {
        // 007DC34A-007DC390: min(m * vy^2 * DecelSpeed, 6.0) while sinking.
        const float vy = in.world_vy_acc;
        descent = min_by_ref_00415510(vy * vy * tuning.decel_speed * m, 6.0f);   // 00CE6630
    }
    // 007DC3A1-007DC3FA: the accumulator fades both terms out over ten units.
    const float a = static_cast<float>(static_cast<double>(in.accumulator_90c) / 10.0);  // 00CE3DC0
    const float s = 1.0f - clamp_by_ref_00415620(a, 0.0f, 1.0f);
    const float sf = s * fade;
    lift = lift * sf;
    descent = sf * descent;
    const float g = static_cast<float>(static_cast<double>(tuning.accel_cheat_mul) * kGravity);
    if (lift > 0.0f) {
        const float b = g * lift;   // 007DC40A-007DC41E
        out.world_gravity[0] = in.normal[0] * b;
        out.world_gravity[1] = in.normal[1] * b;
        out.world_gravity[2] = b * in.normal[2];
    }
    if (descent > 0.0f) {
        const float e = g * descent;   // 007DC470-007DC486
        out.world_drag[0] = e * in.normal[0];
        out.world_drag[1] = in.normal[1] * e;
        out.world_drag[2] = e * in.normal[2];
    }
    // 007DBA32-007DBA99 (the drag block's [ESP+18h] and [ESP+4Ch..54h]): the world
    // speed, zero at or below double 1e-10, and the velocity divided by it.
    const float* const v = in.world_velocity;
    const float v2 = (v[0] * v[0] + v[1] * v[1]) + v[2] * v[2];
    float speed = static_cast<double>(v2) > 1e-10
        ? static_cast<float>(std::sqrt(static_cast<double>(v2))) : 0.0f;
    if (!(static_cast<double>(speed) > 0.01)) return out;   // 007DC4C2, double 00CE6638
    const float dir_x = v[0] / speed;   // speed > 0.001 (00D7A23C) holds here
    const float dir_z = v[2] / speed;
    float depth = in.depth;   // [ESP+24h]
    if (cls.min_water_spd_198 != 0.0f) {
        depth = static_cast<float>(static_cast<double>(in.depth) + 0.5);   // 007DC4EE
    }
    float h = cls.derived_508;
    if (depth > 0.0f) h = max_by_ref_00415550(2.0f, cls.derived_508);   // 00CE3958
    // 007DC533-007DC569: r = clamp((h + depth) / h, 0, 1.8) (00CF4848).
    float r = clamp_by_ref_00415620((h + depth) / h, 0.0f, 1.8f);
    if (!(r > 0.0f)) return out;
    speed = max_by_ref_00415550(speed, tuning.min_drag_spd);   // 007DC586
    if (1.0f > r) r = 1.0f - (1.0f - r) * (1.0f - r);         // 007DC59E-007DC5AE
    const float f = r * speed * speed * cls.water_decel_1a0;   // 007DC5B2-007DC5D1
    out.world_resist_34[0] = dir_x * f;
    out.world_resist_34[1] = static_cast<float>(0.0 * static_cast<double>(f));  // 00D7A258
    out.world_resist_34[2] = f * dir_z;
    // 007DC617-007DC689: the side drag against the body lateral velocity.
    const float vx = in.body_vx;
    const int sign = 0.0f > vx ? -1 : (vx > 0.0f ? 1 : 0);
    float side = vx * (static_cast<float>(-sign) * r) * vx;
    side = side * tuning.side_drag_ratio;
    const float low = -0.0f - tuning.max_side_drag;
    if (side < low) side = low;   // 00415690, both tests strict
    if (side > tuning.max_side_drag) side = tuning.max_side_drag;
    out.body_damping_x = side;
    return out;
}

void add_water_band(PlaneDynAccumulators& acc, const PlaneWaterBandTerms& terms,
                    const float world_to_body[9]) {
    for (int i = 0; i < 3; ++i) {
        acc.world_gravity[i] += terms.world_gravity[i];
        acc.world_drag[i] += terms.world_drag[i];
    }
    acc.body_damping[0] += terms.body_damping_x;
    // 007D84C2-007D84FF, fold 3: dyn+34h through ctl+0B0h into dyn+40h.
    for (int r = 0; r < 3; ++r) {
        acc.body_resist_40[r] += world_to_body[r * 3 + 0] * terms.world_resist_34[0]
            + world_to_body[r * 3 + 1] * terms.world_resist_34[1]
            + world_to_body[r * 3 + 2] * terms.world_resist_34[2];
    }
}

void water_angular_kick_007dafd0(float angular_world[3], const PlaneWaterPose& pose,
                                 const float body_point[3], const float world_dir[3],
                                 float accel, float step, float limit,
                                 const float fallback_axis_674[3]) {
    float pw[3];
    transform_direction(pw, pose, body_point);   // 007DAFFB
    float axis[3];
    cross_004f9b30(axis, world_dir, pw);         // 007DB00D, EDX = dir
    const float s = (axis[0] * axis[0] + axis[1] * axis[1]) + axis[2] * axis[2];
    const float len = static_cast<double>(s) > 1e-10   // 007DB046, double 00CE3820
        ? static_cast<float>(std::sqrt(static_cast<double>(s))) : 0.0f;
    if (0.001 > static_cast<double>(len)) {      // 007DB078, double 00CF3F30
        for (int i = 0; i < 3; ++i) axis[i] = fallback_axis_674[i];
    }
    normalize_0042b260(axis);                    // 007DB0B4
    for (int i = 0; i < 3; ++i) {
        const float c = accel * axis[i];
        const float w = angular_world[i];
        if (0.0f > c) {
            if (limit * c < w) angular_world[i] = w + step * c;
        } else if (w < limit * c) {
            angular_world[i] = step * c + w;
        }
    }
}

void water_law_tail_007dd493(float angular_world[3], const PlaneWaterTailInputs& in,
                             const PlaneWaterPose& pose, const PlaneWaterLawClass& cls,
                             const PlaneWaterLawTuning& tuning) {
    // 007DD4B4-007DD56C: the depth term.
    float f;
    if (in.depth > 0.0f) {
        f = pow_abs(static_cast<float>(static_cast<double>(in.depth) + 1.0),
                    tuning.lift_rotate_depth_ratio);
    } else {
        const float q = cls.derived_508 * tuning.lift_begin_dive_mul;
        float t = static_cast<float>(static_cast<double>(in.depth / q) + 1.0);
        if (0.0f > t) t = 0.0f;
        f = t * t;
    }
    // 007DD570-007DD5C6: the body vector (0, cos WaterPitch, sin WaterPitch).
    const float wp = cls.water_pitch_204;
    const float body_axis[3] = {0.0f,
        static_cast<float>(std::cos(static_cast<double>(wp))),
        static_cast<float>(std::sin(static_cast<double>(wp)))};
    float world_axis[3];
    transform_direction(world_axis, pose, body_axis);
    // 007DD5CB-007DD65C: |2 - dot|^LiftRotateAngleRatio (double 00D7A308 = 2.0).
    const float dot = (world_axis[1] * in.normal[1] + world_axis[0] * in.normal[0])
        + world_axis[2] * in.normal[2];
    const float s = static_cast<float>(2.0 - static_cast<double>(dot));
    f = pow_abs(s, tuning.lift_rotate_angle_ratio) * f;
    if (!(tuning.lift_rotate_max > f)) f = tuning.lift_rotate_max;   // 007DD676 JA
    water_angular_kick_007dafd0(angular_world, pose, body_axis, in.normal,
                                cls.water_rot_accel_180 * f, in.step, kKickLimit,
                                in.fallback_axis_674);   // 007DD6BF
    if (in.nose_down_94) {
        // 007DD6D1-007DD83A: the nose (0, 0, 1) toward world down (0, -1, 0).
        const float blend = clamped_interpolate_00419010(0.0f, 3.0f, 83.33333587646484f,
                                                         16.0f, in.horizontal_speed);
        const float pitch = in.pitch_c64 > 0.0f ? in.pitch_c64 : -0.0f - in.pitch_c64;
        float level = 1.0f - pitch;
        if (0.0f > level) level = 0.0f;
        const float roll = in.roll_c68 > 0.0f ? in.roll_c68 : -0.0f - in.roll_c68;
        float bank = roll + roll;
        if (bank > 1.0f) bank = 1.0f;
        const float nose[3] = {0.0f, 0.0f, 1.0f};
        const float down[3] = {0.0f, -1.0f, 0.0f};   // 00D7A260
        water_angular_kick_007dafd0(angular_world, pose, nose, down, bank * level * blend,
                                    in.step, kKickLimit, in.fallback_axis_674);
    }
    // 007DD83F-007DD991: below StallSpd the angular velocity decays by 1 - q^2 per
    // second, never past zero.
    const float q = clamped_interpolate_00419010(0.0f, 0.0f, cls.stall_spd_184, 1.0f,
                                                 in.forward_speed_44);
    if (1.0f > q && in.step > 0.0f) {
        const float k = -(1.0f - q * q) * in.step;
        for (int i = 0; i < 3; ++i) {
            float d = angular_world[i] * k;
            const float m = -angular_world[i];
            if (0.0f > d) {
                if (m > d) d = m;
            } else if (d > m) {
                d = m;
            }
            angular_world[i] = d + angular_world[i];
        }
    }
}

PlaneWaterRateFactors water_rate_factors_007da3d0(const PlaneWaterRateInputs& in) {
    PlaneWaterRateFactors out;
    out.out_a = clamped_interpolate_00419010(in.max_ctrl_angle, 1.0f, in.min_ctrl_angle, 0.0f,
                                             in.up_y);   // 007DA3F0-007DA42B
    const float v = in.speed_6c;
    if (v < in.normal_yaw_control_spd) {
        float y1 = 1.0f;
        bool done = false;
        if (!(in.throttle_9f0 <= 0.0f)) {
            y1 = in.max_yaw_control;
            if (in.max_yaw_control_spd < v) {
                const float x0 = static_cast<float>(
                    static_cast<double>(in.max_yaw_control_spd) * 1.25);   // 00CF87C0
                out.out_b = clamped_interpolate_00419010(x0, in.max_yaw_control,
                                                         in.normal_yaw_control_spd, 1.0f, v);
                done = true;
            }
        }
        if (!done) {
            const float x1 = static_cast<float>(
                static_cast<double>(in.max_yaw_control_spd) * 0.800000011920929);  // 00CE3D40
            out.out_b = clamped_interpolate_00419010(0.0f, 0.0f, x1, y1, v);
        }
    }
    out.out_a = out.out_a * in.authority;          // 007DA51F
    out.out_b = out.out_b * in.yaw_control_factor;
    return out;
}

PlaneWaterLiftOff water_lift_off_007cbc36(float depth, float derived_508, bool hull_check_770,
                                          bool any_hull_point_under) {
    PlaneWaterLiftOff out;
    const float height = -depth;   // 007CBC44 FCHS, float [ESP+68h]
    if (static_cast<double>(derived_508) + 1.5 > static_cast<double>(height)) {
        out.stay_on_water = true;   // 007CBC60
        return out;
    }
    out.stay_on_water = hull_check_770 && any_hull_point_under;   // 007CBE70
    out.hull_clear_call = hull_check_770 && !any_hull_point_under;
    return out;
}

}  // namespace bsp
