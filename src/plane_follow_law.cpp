// 009BEE30's fly-to arm.  See include/bsp/plane_follow_law.hpp for the
// addresses, the contract with 009BFEE0 and what is provisional.
#include "bsp/plane_follow_law.hpp"

#include <cmath>

#include "bsp/camera_position_modes.hpp"      // camera_asin_clamped_0042cf10
#include "bsp/dive_bomb_task.hpp"
#include "bsp/native_scalar_float_by_ref.hpp"  // 00415510 / 00415550
// 00438B10 is already reconstructed there; `FLD [ESP+4] / FSUB [ESP+8]` at its
// own entry fixes the order as wrap(left - right), which is what every call
// below depends on.
#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 0042B2F0 BSP_Vector3_LengthFloatThreshold, as 009BFA42 and 009BFAA7 use it:
// the vector handed in has its Y zeroed (009BFA2B / 009BFA6A `MOVSS
// [ESP+58h],XMM0` after `XORPS XMM0,XMM0`), so both calls are horizontal.
// Below the strict double 1e-10 at 00CE3820 the callee returns +0, including
// for unordered input; above it, the genuine CRT sqrt at 00BF7030.
float horizontal_range_0042b2f0(float dx, float dz) noexcept {
    const float sum = dx * dx + dz * dz;
    if (!(static_cast<double>(sum) > 1e-10)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(sum)));
}

}  // namespace

float plane_follow_blended_altitude_009bfaac(float station_y, float steer_y,
                                             float distance_to_station,
                                             float distance_to_steer) noexcept {
    // 009BFAAC `FDIV [ESP+3Ch]`: the ratio is station range OVER steer range.
    // The image does not guard the divide; a zero steer range there yields a
    // non-finite t, which the comparison below then sends to the 1.0 arm for
    // +inf and to the `t` arm for NaN (JBE is taken when unordered).  This
    // host reproduces the ordered cases and takes the 1.0 arm for a zero
    // denominator, which is where +inf lands.
    float t;
    if (distance_to_steer == 0.0f) {
        t = 1.0f;
    } else {
        t = distance_to_station / distance_to_steer;
        // 009BFAC5 FLD1 / 009BFACB FCOMIP ST0,ST1 / JBE: t is kept when
        // t <= 1.0 and replaced by [00D7A24C] = 1.0f otherwise.  JBE is also
        // taken when unordered, so a NaN t is kept, as here.
        if (t > 1.0f) t = 1.0f;
    }
    // 009BFAE1-009BFB02: (steerY - stationY) * t + stationY.
    return (steer_y - station_y) * t + station_y;
}

PlaneFollowFlyToCommand plane_follow_flyto_command_009bee30(
    const PlaneFollowFlyToInputs& in) noexcept {
    PlaneFollowFlyToCommand out;

    // 009BFA1B-009BFA49.  The steer point's horizontal offset from the unit,
    // with Y zeroed before the length.
    out.distance_to_steer_point = horizontal_range_0042b2f0(
        in.steer_point[0] - in.unit_pos[0], in.steer_point[2] - in.unit_pos[2]);

    // 009BFA60-009BFAAC.  The station's horizontal offset, likewise.
    const float station_dx = in.station[0] - in.unit_pos[0];
    const float station_dz = in.station[2] - in.unit_pos[2];
    out.distance_to_station = horizontal_range_0042b2f0(station_dx, station_dz);

    // 009BFAAC-009BFB02.  station+34h is the station's Y.
    out.commanded_altitude = plane_follow_blended_altitude_009bfaac(
        in.station[1], in.steer_point[1], out.distance_to_station,
        out.distance_to_steer_point);

    // 009BFC0C-009BFC1E, the first argument of 009F9ED0.  pose+100h is the
    // unit's own world Y.
    out.altitude_error = out.commanded_altitude - in.unit_pos[1];

    // 009BFB0F-009BFB8F.  The direction to the station is re-derived inline
    // here rather than reused, with its own 1e-10 cutoff at 009BFB3D, and the
    // division is by that inline length.
    const float len_sq = station_dx * station_dx + station_dz * station_dz;
    float len = 0.0f;
    if (static_cast<double>(len_sq) > 1e-10) {
        len = static_cast<float>(std::sqrt(static_cast<double>(len_sq)));
    }
    float ux = 0.0f;
    float uz = 0.0f;
    if (len != 0.0f) {
        ux = station_dx / len;
        uz = station_dz / len;
    }

    // 009BFBBE-009BFBF0: the distance handed to 009F9ED0 is floored at
    // block+00h.  FCOMIP compares len against the block value and JBE (taken
    // when equal or unordered) selects the block value.
    out.command_distance = (len > in.min_command_dist) ? len : in.min_command_dist;

    // 009BFC26-009BFC3D.  How much the unit already points at its station.
    out.alignment_dot = in.unit_forward_x * ux + in.unit_forward_z * uz;

    // 009BFC58/009BFC63/009BFC7D: the leader's speed, floored at classDesc+188h.
    // This floored value is used ONLY as the factor of the catch-up speed.  The
    // ramp's y0 is a SECOND, separate call of the same virtual at 009BFCC3,
    // which is not floored -- so a leader flying slower than classDesc+188h
    // gives a member on its station the leader's true speed while still scaling
    // the catch-up end by the floor.  Reading 009BFC7D as if it fed both ends
    // is the easy error here.
    const float floored_leader_speed = (in.leader_speed > in.class_min_speed)
                                           ? in.leader_speed
                                           : in.class_min_speed;

    // 009BFC98-009BFCD1.  The five floats of 00419010 are assembled across TWO
    // stack windows with a zero-argument virtual call between them, which is
    // why a naive read of the `SUB ESP,0Ch` alone gets the arity wrong:
    //   009BFC98 SUB ESP,0Ch   -> [B+8]=len, [B+4]=block[4Ch]*leaderSpeed,
    //                             [B+0]=block[18h]
    //   009BFCC3 CALL [..+38h] -> the leader-speed virtual, RET 0
    //   009BFCC5 SUB ESP,8     -> [B-4]=that result, [B-8]=0.0 (FLDZ)
    //   009BFCD1 CALL 00419010, RET 14h, which cleans all five.
    // So the ramp is over the range to the station, from the leader's speed at
    // zero range to a catch-up speed at GoodPositionDist.
    out.distance_ramp_speed = dive_bomb_interpolate_clamped_00419010(
        0.0f, in.leader_speed, in.good_position_dist,
        in.catchup_speed_scale * floored_leader_speed, len);

    // 009BFCE6-009BFD0F.  The commanded speed is a second ramp, this one over
    // the alignment cosine, between the distance ramp above and the level
    // flight speed already scaled by 0.9 at 009BFC49.
    out.desired_speed_2b4 = dive_bomb_interpolate_clamped_00419010(
        in.align_ramp_lo, out.distance_ramp_speed, in.align_ramp_hi,
        in.level_flight_speed, out.alignment_dot);

    out.produced = true;
    return out;
}

namespace {

// The double constants 009BFEE0 reads (each at the width its FADD/FMUL takes).
constexpr double kHalfPi = 1.5707963705062866;        // [00CE3830]
constexpr double kPi = 3.1415927410125732;            // [00CE3D28]
constexpr double kTwoPi = 6.2831854820251465;         // [00CE3828]
constexpr double kThreeHalfPi = 4.7123889923095703;   // [00D20A20]
constexpr double kGuardShare = 0.05000000074505806;   // [00D7A270]
constexpr double kPullBack = 0.20000000298023224;     // [00CE3D10]
constexpr double kBandHalfHeight = 120.0;             // [00D1F3F8]
constexpr float kLengthFloor = 0.009999999776482582f; // [00D7A238]

// Every intermediate the listing keeps goes through an FSTP dword; this is
// that rounding.
inline float F(double v) noexcept { return static_cast<float>(v); }
inline float fsin(float v) noexcept { return F(std::sin(static_cast<double>(v))); }
inline float fcos(float v) noexcept { return F(std::cos(static_cast<double>(v))); }

// 009C0142-009C0164 and 009C0F86-009C0FAE both wrap with a SINGLE conditional
// add of 2*pi ([00CE3828] = 6.2831854820251465), not a loop; an input below
// -2*pi therefore stays negative in the image and does so here.
float wrap_once_to_two_pi_009c0150(float a) noexcept {
    if (!(0.0f <= a)) {
        return static_cast<float>(static_cast<double>(a) + kTwoPi);
    }
    return a;
}

// 00419440 BSP_Vector3f_Length, then 00419510's own rule: scale by 1/len when
// len > 0, else write the zero vector (00419522-0041953D).
void normalize_00419510(const float v[3], float out[3]) noexcept {
    const double len = std::sqrt(static_cast<double>(v[0]) * v[0] +
                                 static_cast<double>(v[1]) * v[1] +
                                 static_cast<double>(v[2]) * v[2]);
    if (!(len > 0.0)) {
        out[0] = out[1] = out[2] = 0.0f;
        return;
    }
    const float inv = static_cast<float>(1.0 / len);
    for (int i = 0; i < 3; ++i) out[i] = v[i] * inv;
}

// The tail's ordered select, 009C17C6-009C183B: below the floor the floor
// wins, above the ceiling the ceiling wins, otherwise the value passes.
float clamp_into_band_009c17c6(float v, float floor_v, float ceil_v) noexcept {
    if (v < floor_v) return floor_v;
    if (v > ceil_v) return ceil_v;
    return v;
}

// 0042CF10 as Phase A calls it: `PUSH` of a float, ST0 back, RET 4.
float asin_clamped(float v) noexcept { return camera_asin_clamped_0042cf10(v); }

// 0042BE90: |*ECX| (00D7A208 is the sign mask).
float fabs_0042be90(float v) noexcept { return v > 0.0f ? v : F(-0.0 - v); }

}  // namespace

PlaneFollowPhaseA plane_follow_phase_a_009c0251(
    const PlaneFollowPhaseAInputs& in) noexcept {
    const float A = in.heading_error;
    const float V = in.cross_track;
    const float AL = in.along_track;
    const float vl = in.leader_speed;
    const float iw = in.inv_turn_rate;
    const float r = in.turn_radius;
    // base-21h, written by the classifier at 009C01F0 / 009C021E.
    const bool sgn = V < 0.0f;
    // 009C0ECB-009C0EDF: (BL ? [00E0E2F9]=2 : [00E0E2FB]=4) | [00E0E2F8]=8,
    // BL being the V-sign byte on every path that reaches it.
    const int bl_turn_circle = sgn ? 10 : 12;
    // 009C0800: TEST BL,BL / JE 009C08F5 (BL=2) else 009C0814 (BL=4).
    const int bl_abeam_0800 = sgn ? 4 : 2;
    // 009C08E7: TEST BL,BL / JE 009C0814 (BL=4) else 009C08F5 (BL=2).
    const int bl_abeam_08e7 = sgn ? 2 : 4;

    PlaneFollowPhaseA out;
    // 009C086D / 009C0B82: behind the moving station at the end of the plan,
    // or more than 5% of that distance off the track -> turn circle; else lead
    // pursuit.  `time` is base+04h (quadrant 1) or base-10h (quadrant 2).
    auto lead_or_circle = [&](float p, float x24, float time) {
        out.time = time;
        if (0.0f > p) {
            out.bl = bl_turn_circle;
            return out;
        }
        const float e = fabs_0042be90(F(static_cast<double>(x24) + V));
        out.bl = (static_cast<double>(e) > static_cast<double>(p) * kGuardShare)
                     ? bl_turn_circle : 1;
        return out;
    };

    if (in.quadrant_bl == 1) {
        // 009C032D-009C0409: the first arc, turning through th onto the track.
        float th, x18, x24;
        if (!sgn) {
            th = F(A + kHalfPi);                                          // 009C033F
            x18 = F(-static_cast<double>(r) * fsin(th));
        } else {
            th = F(kHalfPi - A);                                          // 009C03A5
            x18 = F(static_cast<double>(fsin(th)) * r);
        }
        const float T = F(kHalfPi * iw + static_cast<double>(th) * iw);   // base-10h
        const float x1c = F(static_cast<double>(F(1.0 - fcos(th))) * r);
        x24 = sgn ? F(static_cast<double>(x18) + r) : F(static_cast<double>(x18) - r);
        float x28 = F(static_cast<double>(x1c) + r);                      // 009C0417
        const float t = F(static_cast<double>(x24) + V);                  // 009C0423
        float t4 = T;                                                     // base+04h
        if (!sgn ? (t < 0.0f) : (t > 0.0f)) {
            // 009C0463: the arc is cut short where it already meets the track.
            const float at = !sgn ? F(-0.0 - t) : t;
            const float half = F(static_cast<double>(at) * 0.5);
            float phi = asin_clamped(F(static_cast<double>(half) / r));
            phi = min_native_float_by_ref_00415510(&th, &phi);          // 009C0492
            x24 = F(-0.0 - V);                                            // 009C04AC
            x28 = F(x28 - static_cast<double>(r) * 2.0 * F(1.0 - fcos(phi)));
            t4 = F(T - static_cast<double>(phi) * 2.0 * iw);              // 009C04F6
        }
        // 009C0508-009C0522: along-track position at the end of the plan,
        // relative to the station that has moved on at the leader's speed.
        const float p = F(static_cast<double>(F(x28 - static_cast<double>(t4) * vl)) + AL);
        if (0.0f > p) return lead_or_circle(p, x24, t4);                  // 009C052E
        // 009C0534: ahead of it: plan a full reversal instead.
        float t2 = F(static_cast<double>(iw) * kPi + T);                  // 009C055C
        const float x1c_b = F(static_cast<double>(x1c) - r);              // base+1Ch
        const float x18_b = sgn ? F(static_cast<double>(x18) + r) : F(static_cast<double>(x18) - r);
        const float h = F(F(static_cast<double>(x18_b) + V) * 0.5);       // base-14h
        const bool too_far = !sgn ? (h >= r) : (-r >= h);                     // 009C0578 / 009C06C7
        if (!too_far) {
            // 009C057E-009C07B6: the second arc and its alternative.
            const float arg = (h < 0.0f)
                ? F(static_cast<double>(F(static_cast<double>(h) + r)) / r)       // 009C058D
                : F(-static_cast<double>(F(static_cast<double>(h) - r)) / r);     // 009C05BA
            const float psi1 = asin_clamped(arg);
            const float beta = !sgn ? F(-0.0 - A) : A;                    // 009C05F2 / 009C0728
            const float two_iw = F(2.0 * iw);                             // base+34h
            t2 = F(static_cast<double>(psi1) * two_iw + t2);
            const float x44 = !sgn
                ? F(static_cast<double>(F(1.0 - fcos(beta))) * -static_cast<double>(r))
                : F(static_cast<double>(F(1.0 - fcos(beta))) * r);
            const float x48 = F(static_cast<double>(fsin(beta)) * r);
            const float g = F(static_cast<double>(fabs_0042be90(
                F(F(static_cast<double>(x44) + V) * 0.5))) / r);
            const float psi2 = asin_clamped(g);
            const float t3 = F(static_cast<double>(psi2) * two_iw +
                               F(static_cast<double>(beta) * iw + static_cast<double>(iw) * kTwoPi));
            if (t3 <= t2) {
                // 009C0829-009C0861, 009C08DF: never reached by the sampled
                // geometries (docs/PLANE_FOLLOW_PHASE_A.md section 8);
                // transcribed from the listing.
                const float q = F(static_cast<double>(F(x48 - static_cast<double>(t3) * vl)) + AL);
                if (-q > p) return lead_or_circle(p, x24, t4);
                out.bl = bl_abeam_08e7;
                out.time = t3;
                return out;
            }
        }
        // 009C07D6-009C0814.
        const float q = F(static_cast<double>(F(x1c_b - static_cast<double>(t2) * vl)) + AL);
        if (-q > p) return lead_or_circle(p, x24, t4);                    // 009C07FC
        out.bl = bl_abeam_0800;
        out.time = t2;
        return out;
    }

    if (in.quadrant_bl == 2) {
        // 009C0909-009C09B9.
        float th, T, x24;
        if (!sgn) {
            th = A;
            T = F(static_cast<double>(iw) * A + static_cast<double>(iw) * kPi);
            x24 = F(static_cast<double>(r) * F(1.0 - fcos(th)) - 2.0 * r);
        } else {
            th = F(-0.0 - A);
            T = F(static_cast<double>(th) * iw + static_cast<double>(iw) * kPi);
            x24 = F(2.0 * r - static_cast<double>(r) * F(1.0 - fcos(th)));
        }
        float x28 = F(static_cast<double>(fsin(th)) * r + 2.0 * r);      // 009C09CD
        const float t = F(static_cast<double>(x24) + V);                  // 009C09D9
        if (!sgn ? (t < 0.0f) : (t > 0.0f)) {
            const float at = !sgn ? F(-0.0 - t) : t;                      // 009C0A7F
            const float phi = asin_clamped(F(static_cast<double>(F(static_cast<double>(at) * 0.5)) / r));
            x24 = F(-0.0 - V);
            x28 = F(x28 - static_cast<double>(F(1.0 - fcos(phi))) * (2.0 * r));
            T = F(T - (static_cast<double>(phi) + phi) * iw);
        }
        const float p = F(static_cast<double>(F(x28 - static_cast<double>(T) * vl)) + AL);  // 009C09FC
        if (0.0f > p) return lead_or_circle(p, x24, T);                   // 009C0A24
        const float gam = !sgn ? F(kHalfPi - A) : F(A + kHalfPi);         // 009C0A36 / 009C0AFF
        const float t5 = F(static_cast<double>(gam) * iw + static_cast<double>(iw) * kThreeHalfPi);
        const float x48 = F(static_cast<double>(F(static_cast<double>(F(1.0 - fcos(gam))) * r)) - r);
        const float q = F(static_cast<double>(F(x48 - static_cast<double>(t5) * vl)) + AL);  // 009C0B65
        if (-q > p) return lead_or_circle(p, x24, T);                     // 009C0B73
        out.bl = bl_abeam_08e7;                                           // 009C0B75
        out.time = t5;
        return out;
    }

    if (in.quadrant_bl == 4) {
        // 009C0BE3-009C0C6D.
        float gam, x34;
        if (!sgn) {
            gam = F(A - kHalfPi);
            x34 = F(static_cast<double>(F(fsin(gam) - 1.0)) * r);
        } else {
            gam = F(-(A + kHalfPi));
            x34 = F(static_cast<double>(F(1.0 - fsin(gam))) * r);
        }
        float t6 = F((static_cast<double>(gam) + kThreeHalfPi) * iw);     // base-10h
        float x38 = F((3.0 - F(1.0 - fcos(gam))) * r);                    // 009C0C81
        const float u = F(static_cast<double>(x34) + V);                  // 009C0C8D
        if (!sgn ? (u < 0.0f) : (u > 0.0f)) {
            const float phi = asin_clamped(                               // 009C0D15
                F(static_cast<double>(F(static_cast<double>(fabs_0042be90(u)) * 0.5)) / r));
            x38 = F(x38 - static_cast<double>(r) * 2.0 * F(1.0 - fcos(phi)));
            t6 = F(t6 - 2.0 * phi * iw);
        }
        const float p = F(static_cast<double>(F(x38 - static_cast<double>(t6) * vl)) + AL);  // 009C0CC2
        const float dl = !sgn ? F(kPi - A) : F(A + kPi);                  // 009C0CDC / 009C0DA0
        const float half_turn = F(kPi * iw);                              // base+24h (a double store)
        const float t7 = F(static_cast<double>(iw) * dl + kPi * iw);      // base+04h
        const float x48 = F(-static_cast<double>(r) * fsin(dl));          // 009C0DE0
        const float q = F(static_cast<double>(F(x48 - static_cast<double>(t7) * vl)) + AL);
        const float ap = (p > 0.0f) ? p : F(-0.0 - p);                    // 009C0E04 / 009C0E0C
        const float aq = (q > 0.0f) ? q : F(-0.0 - q);                    // 009C0E24 / 009C0E2C
        if (aq > ap) {                                                    // 009C0E42
            out.bl = bl_abeam_0800;
            out.time = t6;
            return out;
        }
        float t7v = t7;
        float hv = half_turn;
        out.bl = bl_turn_circle;
        out.time = min_native_float_by_ref_00415510(&t7v, &hv);         // 009C0EC2
        return out;
    }

    // Quadrant 3, 009C0E4E-009C0EC7: always the turn circle.
    const float gam = !sgn ? F(-(A + kHalfPi)) : F(A - kHalfPi);
    float t8 = F((kHalfPi + gam) * iw);                                   // base+04h
    float hv = F(static_cast<double>(iw) * kPi);                          // 009C0EB0
    out.bl = bl_turn_circle;
    out.time = min_native_float_by_ref_00415510(&t8, &hv);
    return out;
}

bool circle_tangent_points_004f4840(const float c[2], float r, const float q[2],
                                    float out0[2], float out1[2]) noexcept {
    const float dx = q[0] - c[0];                                         // 004F3826
    const float dz = q[1] - c[1];                                         // 004F3838
    const float d2 = static_cast<float>(
        static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    const float d = static_cast<double>(d2) > 1e-10                       // 00CE3820
        ? static_cast<float>(std::sqrt(static_cast<double>(d2))) : 0.0f;
    if (!(r < d)) return false;                                           // 004F3887
    const float ux = static_cast<float>(static_cast<double>(dx) / d);
    const float uz = static_cast<float>(static_cast<double>(dz) / d);
    const float p = static_cast<float>(static_cast<double>(r) * r / d);   // 004F38C1
    const float t = static_cast<float>(std::sqrt(static_cast<double>(
        static_cast<float>(static_cast<double>(d) * d -
                           static_cast<double>(r) * r))));                // 004F38D8
    const float h = static_cast<float>(static_cast<double>(t) * r / d);   // 004F38F2
    const float mx = static_cast<float>(
        c[0] + static_cast<double>(static_cast<float>(static_cast<double>(p) * ux)));
    const float mz = static_cast<float>(
        c[1] + static_cast<double>(static_cast<float>(static_cast<double>(p) * uz)));
    const float bx = static_cast<float>(-static_cast<double>(uz) * h);
    const float bz = static_cast<float>(static_cast<double>(ux) * h);
    out0[0] = static_cast<float>(static_cast<double>(mx) + bx);           // 004F4479
    out0[1] = static_cast<float>(static_cast<double>(mz) + bz);
    out1[0] = static_cast<float>(static_cast<double>(mx) - bx);           // 004F4498
    out1[1] = static_cast<float>(static_cast<double>(mz) - bz);
    return true;
}

PlaneFollowGeometry plane_follow_geometry_009bfee0(
    const PlaneFollowGeometryInputs& in, PlaneFollowRegime regime) noexcept {
    PlaneFollowGeometry out;
    out.regime = regime;

    // 009C0052-009C007A, then 009C0092-009C00BA.  Horizontal only; the Y
    // difference is not formed here.
    const float dx = in.own_pos[0] - in.station[0];
    const float dz = in.own_pos[2] - in.station[2];
    const float r = horizontal_range_0042b2f0(dx, dz);
    out.range_horizontal = r;

    // 009C00F7 then 009C0109-009C0128.  The lag time is ramped by the range to
    // the station; the reference heading is the leader's, lagged by it.
    out.lag_time = dive_bomb_interpolate_clamped_00419010(
        in.leader_heading_dist_1, in.leader_heading_time_1,
        in.leader_heading_dist_2, in.leader_heading_time_2, r);
    const float g = in.leader_turn_rate * out.lag_time;
    const float ref = wrapped_angle_subtract_00438b10(in.leader_heading, g);
    out.reference_heading = ref;

    // 009C0139-009C0164: the compass bearing station->aircraft.
    const float bearing_raw = static_cast<float>(
        std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    const float bearing = wrap_once_to_two_pi_009c0150(
        static_cast<float>(kHalfPi - bearing_raw));

    // 009C0176-009C01B2.  R * (sin, cos) of the bearing taken in the reference
    // frame: the cross-track and along-track offsets from the station.
    const float a0 = wrapped_angle_subtract_00438b10(bearing, ref);
    out.cross_track = r * static_cast<float>(std::sin(static_cast<double>(a0)));
    out.along_track = r * static_cast<float>(std::cos(static_cast<double>(a0)));

    // 009C01B6-009C01D3, then the quadrant classifier 009C01D3-009C024F.
    const float a = wrapped_angle_subtract_00438b10(in.own_heading, ref);
    out.heading_error = a;
    {
        const bool s = (out.cross_track >= 0.0f);
        const bool aa = (a >= 0.0f);
        const bool near_axis =
            (std::fabs(static_cast<double>(a)) <= kHalfPi);
        out.quadrant_bl = (s == aa) ? (near_axis ? 2 : 4) : (near_axis ? 1 : 3);
    }

    // Phase A, 009C0251-009C0EE0, then 009C0EE1-009C0F00.
    float lead_distance = 0.0f;
    float turn_radius = 0.0f;
    if (in.run_phase_a) {
        // 009C025E-009C02A9: the leader's horizontal speed.
        const float vl = horizontal_range_0042b2f0(in.leader_velocity[0], in.leader_velocity[2]);
        // 009C02AF-009C0323: the turn model.
        const float mul = in.own_large_turn_class ? in.large_plane_turn_mul
                                                  : in.small_plane_turn_mul;
        const float w = F(static_cast<double>(mul) * in.class_turn_rate_270);
        turn_radius = F(static_cast<double>(in.class_travel_speed_18c) / w);
        PlaneFollowPhaseAInputs pin;
        pin.heading_error = a;
        pin.cross_track = out.cross_track;
        pin.along_track = out.along_track;
        pin.quadrant_bl = out.quadrant_bl;
        pin.leader_speed = vl;
        pin.inv_turn_rate = F(1.0 / w);
        pin.turn_radius = turn_radius;
        const PlaneFollowPhaseA pa = plane_follow_phase_a_009c0251(pin);
        out.phase_a_bl = pa.bl;
        out.phase_a_time = pa.time;
        lead_distance = F(static_cast<double>(pa.time) * vl);             // 009C0F00
        out.lead_distance = lead_distance;
        out.turn_radius = turn_radius;
        // 009C1059 TEST [00E0E2FA]=1,BL; 009C1241 TEST [00E0E2F8]=8,BL.
        regime = (pa.bl & 1) ? PlaneFollowRegime::kLeadPursuit
               : (pa.bl & 8) ? PlaneFollowRegime::kOffsetPoint009c1328
                             : PlaneFollowRegime::kAbeam;
        out.regime = regime;
    }
    out.state_5c = -1.0f;                                                 // 009C0EF4

    // 009C0F0D-009C0F80: the leader's horizontal forward length, floored at
    // [00D7A238] = 0.01 so a vertical leader cannot degenerate the direction.
    float lh = horizontal_range_0042b2f0(in.leader_forward[0], in.leader_forward[2]);
    if (!(static_cast<double>(lh) >= kLengthFloor)) {
        lh = kLengthFloor;
    }

    // 009C0F86-009C104C.  U is the unit vector along the LAGGED leader heading
    // carrying the leader's own vertical slope.  The `LEA EAX,[EDI+0CCh]` at
    // 009C100F is dead: 00419510 is __fastcall(out = ECX, v = EDX) and returns
    // `out` in EAX (00419545 MOV EAX,EDI), so the copy that follows reads the
    // normalized local, not the leader's pose row 0.
    const float t_dir = wrap_once_to_two_pi_009c0150(
        static_cast<float>(kHalfPi - static_cast<double>(ref)));
    const float h_x = lh * static_cast<float>(std::cos(static_cast<double>(t_dir)));  // base+34h
    const float h_z = lh * static_cast<float>(std::sin(static_cast<double>(t_dir)));  // base+38h
    const float u_raw[3] = {h_x, in.leader_forward[1], h_z};
    float u[3];
    normalize_00419510(u_raw, u);

    // 009C10A9, the 3D range to the station.  0042B2F0 again, but this call is
    // handed a vector whose Y is NOT zeroed (009C108C-009C10A2).
    const float d3[3] = {
        in.own_pos[0] - in.station[0],
        in.own_pos[1] - in.station[1],
        in.own_pos[2] - in.station[2],
    };
    const float d = static_cast<float>(
        std::sqrt(static_cast<double>(d3[0]) * d3[0] +
                  static_cast<double>(d3[1]) * d3[1] +
                  static_cast<double>(d3[2]) * d3[2]));
    out.range_3d = d;
    // SUBSTITUTION, labelled, for the switch-off binding only: base-0Ch is
    // Phase A's; without it D stands in, the same slope-times-distance shape
    // the lead regime uses.
    const float base_0c = in.run_phase_a ? lead_distance : d;

    float p[3];
    bool copy_to_60 = true;
    switch (regime) {
        case PlaneFollowRegime::kLeadPursuit: {
            // 009C10D2-009C1105: the station pushed D ahead along the track.
            for (int i = 0; i < 3; ++i) p[i] = in.station[i] + d * u[i];
            // 009C111E-009C11EC: pulled 0.2 * D back toward the aircraft.
            // [00CE3D10] = 0.2, read as the double the FMUL takes.
            const float back[3] = {
                in.own_pos[0] - p[0], in.own_pos[1] - p[1], in.own_pos[2] - p[2]};
            float n[3];
            normalize_00419510(back, n);
            for (int i = 0; i < 3; ++i) {
                p[i] += static_cast<float>(kPullBack * static_cast<double>(d)) * n[i];
            }
            // 009C11EF-009C1239: and pushed FollowedPointDist further along.
            for (int i = 0; i < 3; ++i) p[i] += in.followed_point_dist * u[i];
            break;
        }
        case PlaneFollowRegime::kAbeam: {
            // 009C15C0-009C1661.  The horizontal perpendicular of the OWN
            // nose: pose row 2's (x, z) times 00419260's reciprocal length.
            float fx, fz;
            if (in.run_phase_a) {
                const double inv = 1.0 / std::sqrt(
                    static_cast<double>(in.own_forward[0]) * in.own_forward[0] +
                    static_cast<double>(in.own_forward[2]) * in.own_forward[2]);
                fx = F(in.own_forward[0] * inv);
                fz = F(in.own_forward[2] * inv);
            } else {
                // pose row 2 normalized is (sin h, cos h) for the slot +50h
                // heading h.
                fx = static_cast<float>(std::sin(static_cast<double>(in.own_heading)));
                fz = static_cast<float>(std::cos(static_cast<double>(in.own_heading)));
            }
            // 009C1646: BL&2 set -> (-fz, fx), the LEFT perpendicular; clear ->
            // (fz, -fx), the RIGHT one.  Which BL Phase A leaves depends on the
            // join it left through (009C0800 or 009C08E7, opposite polarity on
            // the V sign), so the side is Phase A's, not the V sign's.
            const bool left = in.run_phase_a ? ((out.phase_a_bl & 2) != 0)
                                             : (out.cross_track < 0.0f);
            const float dir_x = left ? -fz : fz;
            const float dir_z = left ? fx : -fx;
            // 009C1678-009C16AF.
            p[0] = in.own_pos[0] + in.followed_point_dist * dir_x;
            p[2] = in.own_pos[2] + in.followed_point_dist * dir_z;
            // 009C16B2: stationY + U.y * base-0Ch.
            p[1] = in.station[1] + u[1] * base_0c;
            break;
        }
        case PlaneFollowRegime::kOffsetPoint009c1328: {
            // 009C12E7-009C1336: station + base-0Ch * U.
            if (!in.run_phase_a) {
                // The switch-off binding, with D for base-0Ch.
                for (int i = 0; i < 3; ++i) p[i] = in.station[i] + d * u[i];
                break;
            }
            for (int i = 0; i < 3; ++i) p[i] = F(base_0c * u[i] + static_cast<double>(in.station[i]));
            // 009C124D-009C1262: the turn circle's radius, state+5Ch.
            out.state_5c = turn_radius;
            // 009C1273-009C12D9: the horizontal track direction (h / lh),
            // swapped, one component negated on the V sign, times r.
            const float ux = F(static_cast<double>(h_x) / lh);
            const float uz = F(static_cast<double>(h_z) / lh);
            const bool sgn = out.cross_track < 0.0f;                      // 009C12A7, base-21h
            const float nx = F(static_cast<double>(sgn ? F(-0.0 - uz) : uz) * turn_radius);
            const float nz = F(static_cast<double>(turn_radius) * (sgn ? ux : F(-0.0 - ux)));
            // 009C1343-009C134E: the circle centre beside that point.
            const float c[2] = {F(static_cast<double>(nx) + p[0]), F(static_cast<double>(nz) + p[2])};
            // 009C1365-009C13C2: the aircraft from the centre, floored at 0.01.
            const float ddx = in.own_pos[0] - c[0];
            const float ddz = in.own_pos[2] - c[1];
            float dist = horizontal_range_0042b2f0(ddx, ddz);
            if (!(static_cast<double>(dist) >= kLengthFloor)) dist = kLengthFloor;
            // 009C13C8-009C142C: inside radius + 1, pushed out to it.
            float q[2] = {in.own_pos[0], in.own_pos[2]};
            const double r1 = 1.0 + static_cast<double>(turn_radius);
            if (!(r1 <= dist)) {
                const float k = F(r1 / dist);
                q[0] = F(c[0] + static_cast<double>(F(static_cast<double>(ddx) * k)));
                q[1] = F(c[1] + static_cast<double>(F(static_cast<double>(k) * ddz)));
            }
            // 009C1432-009C14A9: the tangent; BL&2 (V < 0) takes out0.
            // When 004F4840 declines, the out slots keep what they held:
            // base+44h/48h = (base-0Ch * U.x, base-0Ch * U.y) and base+24h/28h
            // = (U.x, U.y).
            float t0[2] = {F(base_0c * u[0]), F(base_0c * u[1])};
            float t1[2] = {u[0], u[1]};
            circle_tangent_points_004f4840(c, turn_radius, q, t0, t1);
            const float* tp = sgn ? t0 : t1;
            out.state_60[0] = tp[0];
            out.state_60[1] = in.own_pos[1];                              // 009C15B1
            out.state_60[2] = tp[1];
            copy_to_60 = false;
            // 009C14B6-009C152B: the direction from q to the tangent point,
            // lengthened to FollowedPointDist when shorter.
            float vx = F(static_cast<double>(tp[0]) - q[0]);
            float vz = F(static_cast<double>(tp[1]) - q[1]);
            float len = horizontal_range_0042b2f0(vx, vz);
            if (!(in.followed_point_dist <= len)) {
                float floor01 = 0.1f;                                     // 009C14FE
                const float k = F(static_cast<double>(in.followed_point_dist) /
                                  max_native_float_by_ref_00415550(&floor01, &len));
                vx = F(static_cast<double>(vx) * k);
                vz = F(static_cast<double>(k) * vz);
            }
            // 009C1545-009C15B6: from the aircraft's own position; Y stays
            // the station's point.
            p[0] = F(static_cast<double>(in.own_pos[0]) + vx);
            p[2] = F(static_cast<double>(in.own_pos[2]) + vz);
            out.state_50[0] = c[0];
            out.state_50[1] = in.own_pos[1];
            out.state_50[2] = c[1];
            out.state_50_written = true;
            break;
        }
    }

    out.station_y = in.station[1];
    out.steer_point[0] = p[0];
    out.steer_point[1] = p[1];
    out.steer_point[2] = p[2];
    if (copy_to_60) {                                                     // 009C16C0
        for (int i = 0; i < 3; ++i) out.state_60[i] = p[i];
    }

    // 009C16D2-009C1846, the tail.  BOTH Y values into one leader-relative
    // band.  This is the `done` floor that keeps a spent wing member out of the
    // water while its leader flies, and it bounds whatever the substitutions
    // above did to the steer Y.
    if (in.band_inputs_available) {
        const float floor_candidate = in.leader_pos[1] + in.band_floor_offset;
        float floor_v =
            (floor_candidate <= in.state_88) ? floor_candidate : in.state_88;
        if (in.run_phase_a) {
            // 009C1734-009C175A: and never below leaderY - 120.
            const float lower = F(in.leader_pos[1] - kBandHalfHeight);
            if (!(lower <= floor_v)) floor_v = lower;
        }
        const float ceil_candidate = static_cast<float>(
            static_cast<double>(in.leader_pos[1]) + kBandHalfHeight);
        const float ceil_v = (in.band_ceiling_210 <= ceil_candidate)
                                 ? in.band_ceiling_210
                                 : ceil_candidate;
        out.station_y = clamp_into_band_009c17c6(out.station_y, floor_v, ceil_v);
        out.steer_point[1] =
            clamp_into_band_009c17c6(out.steer_point[1], floor_v, ceil_v);
        out.band_applied = true;
    }

    out.produced = true;
    return out;
}

}  // namespace bsp
