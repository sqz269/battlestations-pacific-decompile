// 009BEE30's HOLD arm, 009BEE56-009BF9E5.  See include/bsp/plane_follow_hold.hpp
// and docs/PLANE_FOLLOW_HOLD_ARM.md.  Reconstructed and build-tested only; not
// wired into a host and not fixture- or game-validated.
#include "bsp/plane_follow_hold.hpp"

#include <cmath>

#include "bsp/dive_bomb_task.hpp"   // dive_bomb_interpolate_clamped_00419010
#include "bsp/unit_rudder.hpp"      // wrapped_angle_subtract_00438b10

namespace bsp {
namespace {

// Image constants, all dword or float loads unless marked double.
constexpr float kLockStationDist2 = 180.0f;   // 009BEF3B float [00D049FC]
constexpr float kLockDegrees5 = 0.0872664675f;  // 009BF086 dword [00CEDF5C]
constexpr float kLockDegrees8 = 0.139626339f;   // 009BF095 dword [00D20A18]
constexpr float kLockSpeedDiff = 1.5f;        // 009BF0AB dword [00CE380C]
constexpr float kSightMinAhead = 1.0f;        // 009BF19A dword [00D7A24C]
constexpr float kSightCone = 0.15f;           // 009BF1E9/009BF203 [00CE7818]
constexpr double kSightPitchGain = 20.0;      // 009BF21F double [00CE3D88]
constexpr double kSightYawGain = 30.0;        // 009BF25F double [00CE7630]
constexpr float kSightYawLimit = 1.2f;        // 009BF257/009BF26F [00CE3814]/[00D05EA4]
constexpr float kBankGateFull = 0.2f;         // 009BF345 float [00CE54A0]
constexpr float kBankGateZero = 0.8f;         // 009BF355 float [00CE74F8]
constexpr double kVertScale = 0.6;            // 009BF44F double [00CEFF98]
constexpr double kVertBase = 0.4;             // 009BF455 double [00CE65D0]
constexpr float kBlendStart = 0.5f;           // 009BF524 dword [00CE3800]
constexpr float kSteerMinAhead = 80.0f;       // 009BF58A dword [00CE5444]
constexpr double kSteerGain = 8.0;            // 009BF5DD double [00CE3DB0]
constexpr float kRollDirectLimit = 0.75f;     // 009BF71E dword [00CEE07C]
constexpr double kFloorAbove = 8.333333969116211;  // 009BF912 double [00D20A10], KMH(30)
constexpr double kFloorBelow = 5.555555820465088;  // 009BF933 double [00D20A08], KMH(20)
// 00605070's constants, doubles: [00CE3828] 2pi, [00CE3D18] -pi, [00CE3D28] pi.
constexpr double kTwoPi = 6.2831854820251465;
constexpr double kPi = 3.1415927410125732;

float clamp_between(float v, float lo, float hi) noexcept {
    if (lo > v) return lo;
    return (v > hi) ? hi : v;
}

// 00605070, __thiscall(float* angle), RET: fmod by 2pi (CRT 00BF857A), then
// one +2pi if <= -pi or one -2pi if > pi, stored in place as a float.
float wrap_angle_in_place_00605070(float angle) noexcept {
    float v = static_cast<float>(std::fmod(static_cast<double>(angle), kTwoPi));
    if (!(-kPi < static_cast<double>(v))) {
        return static_cast<float>(static_cast<double>(v) + kTwoPi);
    }
    if (static_cast<double>(v) > kPi) {
        return static_cast<float>(static_cast<double>(v) - kTwoPi);
    }
    return v;
}

}  // namespace

float plane_follow_hold_shape_009bf7ca(float x) noexcept {
    const double d = x;
    if (x >= 10.0f || x <= -10.0f) return x;          // 009BF7DC / 009BF7E7
    if (x > 5.0f) return static_cast<float>(d - kVertScale * (10.0 - d));   // 009BF7EE
    if (x > -5.0f) return static_cast<float>(d * kVertBase);                // 009BF805 / 009BF814
    return static_cast<float>(d + kVertScale * (d + 10.0));                 // 009BF81E
}

PlaneFollowHoldCommand plane_follow_hold_command_009bee56(
    const PlaneFollowHoldInputs& in) noexcept {
    PlaneFollowHoldCommand out;
    const PlaneFollowHoldAttitude& o = in.own;
    const PlaneFollowHoldAttitude& l = in.leader;
    const PlaneFollowHoldGains& g = in.gains;

    // 009BEEF2-009BF0B6, the lock gate.
    if (in.leader_present && in.leader_published_520) {
        const float* s = in.station_local;
        const float d2 = static_cast<float>(
            (static_cast<double>(s[0]) * s[0] + static_cast<double>(s[1]) * s[1]) +
            static_cast<double>(s[2]) * s[2]);                          // 009BEF33
        if (kLockStationDist2 > d2) {
            const float dp = std::fabs(wrapped_angle_subtract_00438b10(l.pitch, o.pitch));
            const float db = std::fabs(wrapped_angle_subtract_00438b10(l.bank, o.bank));
            const float dh = std::fabs(wrapped_angle_subtract_00438b10(l.heading, o.heading));
            const float dv = std::fabs(static_cast<float>(
                static_cast<double>(l.speed) - static_cast<double>(o.speed)));
            if (kLockDegrees5 > dp && kLockDegrees8 > db && kLockDegrees5 > dh &&
                kLockSpeedDiff > dv) {
                out.locked = true;
                return out;
            }
        }
    }
    out.clears_plan_270 = true;   // 009BF0F3

    // 009BF0FD-009BF295, the sight correction.  Zero unless it fires.
    if (in.sight_active_84 && in.leader_present) {
        const float x = in.sight_local[0];
        const float y = in.sight_local[1];
        const float z = in.sight_local[2];
        if (z > kSightMinAhead) {
            const float xz = x / z;
            const float yz = y / z;
            if (kSightCone > std::fabs(xz) && kSightCone > std::fabs(yz)) {
                out.sight_pitch = clamp_between(
                    static_cast<float>(yz * kSightPitchGain), -1.0f, 1.0f);
                out.sight_yaw = clamp_between(
                    static_cast<float>(xz * kSightYawGain), -kSightYawLimit, kSightYawLimit);
            } else {
                out.clears_sight_84 = true;   // 009BF295
            }
        }
    }

    // 009BF2D1-009BF363.
    const float dh = wrapped_angle_subtract_00438b10(l.heading, o.heading);
    const float dturn = wrapped_angle_subtract_00438b10(l.turn_c70, o.turn_c70);
    const float lbank_abs = std::fabs(l.bank);
    const float f = dive_bomb_interpolate_clamped_00419010(
        kBankGateZero, 0.0f, kBankGateFull, 1.0f, lbank_abs);
    out.bank_gate = f;
    const double fdh = static_cast<double>(f) * dh;   // 009BF37D, kept as a double

    // 009BF381-009BF3C5, yaw.
    float yaw = static_cast<float>(
        static_cast<double>(static_cast<float>(l.rate_a8 - o.rate_a8)) * g.yf_yaw_v_rad_per_sec +
        fdh * g.yf_hdg_rad);
    yaw = static_cast<float>(static_cast<double>(yaw) +
        (static_cast<double>(g.yf_sidepos_meter) * in.station_local[0] +
         static_cast<double>(g.yf_sidedir) * in.leader_forward_local[0]));
    yaw = yaw + out.sight_yaw;

    // 009BF3EC-009BF476, pitch.
    const float dp = wrapped_angle_subtract_00438b10(l.pitch, o.pitch);
    const float fdp = static_cast<float>(static_cast<double>(f) * dp);   // 009BF406
    float pitch = static_cast<float>(
        static_cast<double>(static_cast<float>(static_cast<double>(l.rate_a4) * f - o.rate_a4)) *
            g.pf_pitch_v_rad_per_sec +
        static_cast<double>(g.pf_pitch_rad) * fdp);
    const float vert_scale = static_cast<float>(static_cast<double>(f) * kVertScale + kVertBase);
    pitch = static_cast<float>(static_cast<double>(pitch) +
        static_cast<double>(vert_scale) *
            (static_cast<double>(g.pf_vertpos_meter) * in.station_local[1] +
             static_cast<double>(g.pf_vertdir) * in.leader_forward_local[1]));
    pitch = pitch + out.sight_pitch;

    // 009BF49D-009BF50D, roll.
    const float db = wrapped_angle_subtract_00438b10(l.bank, o.bank);
    float roll = static_cast<float>(
        static_cast<double>(static_cast<float>(l.rate_a0 - o.rate_a0)) * g.rf_roll_v_rad_per_sec +
        static_cast<double>(g.rf_roll_rad) * db);
    roll = static_cast<float>(static_cast<double>(roll) +
        (static_cast<double>(f) * dturn * g.rf_hdg_v_rad_per_sec +
         fdh * g.rf_hdg_rad));

    // 009BF524-009BF6E5: a steeply banked leader blends yaw and pitch toward
    // the steer point's direction, fully at 0.8 rad.
    if (lbank_abs > kBlendStart) {
        double sx = in.steer_local[0];
        double sy = in.steer_local[1];
        double sz = (in.steer_local[2] > kSteerMinAhead) ? in.steer_local[2] : kSteerMinAhead;
        // 00419510 BSP_Vector3f_Normalize; z >= 80 keeps the length non-zero.
        const double len = std::sqrt(sx * sx + sy * sy + sz * sz);
        const float nx = static_cast<float>(sx / len);
        const float ny = static_cast<float>(sy / len);
        const float yaw_to = clamp_between(static_cast<float>(nx * kSteerGain), -1.0f, 1.0f);
        const float pitch_to = clamp_between(static_cast<float>(ny * kSteerGain), -1.0f, 1.0f);
        yaw = dive_bomb_interpolate_clamped_00419010(
            kBlendStart, yaw, kBankGateZero, yaw_to, lbank_abs);
        pitch = dive_bomb_interpolate_clamped_00419010(
            kBlendStart, pitch, kBankGateZero, pitch_to, lbank_abs);
    }
    out.yaw_284 = yaw;
    out.pitch_29c = pitch;

    // 009BF71E-009BF789.
    if (kRollDirectLimit > lbank_abs) {
        out.writes_roll_290 = true;
        out.roll_290 = roll;
    } else {
        out.writes_bank_target_2c4 = true;
        out.bank_target_2c4 = wrap_angle_in_place_00605070(l.bank - l.rate_a0);
    }

    // 009BF793-009BF9C8, power.
    const float dv = static_cast<float>(static_cast<double>(l.speed) - o.speed);
    out.power_spd_90 = plane_follow_hold_shape_009bf7ca(dv) * g.pwr_spd_meter_per_sec;
    out.power_back_94 = g.pwr_back_meter *
        plane_follow_hold_shape_009bf7ca(in.station_local[2]);
    const float p = out.power_back_94 + out.power_spd_90;
    const float lo_x = static_cast<float>(in.level_flight_speed - kFloorBelow);
    const float hi_x = static_cast<float>(in.level_flight_speed + kFloorAbove);
    out.power_floor = dive_bomb_interpolate_clamped_00419010(lo_x, 1.0f, hi_x, -1.0f, o.speed);
    const float u = (p > out.power_floor) ? p : out.power_floor;   // 009BF956
    out.throttle_278 = clamp_between(u, 0.0f, 1.0f);
    out.air_brake_2a8 = clamp_between(-u, 0.0f, 1.0f);
    return out;
}

}  // namespace bsp
