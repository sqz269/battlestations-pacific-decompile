// Packet cc13-ver3: reference models of what cc9's ship motion host substitutes or omits.
// docs/CC13_VER_3.md. Read from the disk bytes (Capstone); every intermediate is rounded
// to float, the main thread's 24-bit x87 precision.

#include "bsp/ship_motion_ver_cc13.hpp"

namespace bsp {

std::uint32_t ship_motion_word_4f8_00810010(const void* owner) noexcept {
    return *reinterpret_cast<const std::uint32_t*>(static_cast<const char*>(owner) + 0x4f8);
}
void ship_motion_write_controls_00815590(const float* controls, void* writer, ShipMotionFloatArchiveCalls& calls) {
    calls.write_float_slot0c(writer, "thrust", controls[0]);
    calls.write_float_slot0c(writer, "toTurn", controls[1]);
}
namespace {

// The listing's absolute value: a compare against 0, then -0.0f (00D7A208) minus the
// value on the non-positive side.
inline float listing_abs(float x) noexcept {
    return (x > 0.0f) ? x : (-0.0f - x);
}

}  // namespace

float ship_submerged_throttle_008269b1(int class_id, float pose_base_y, float throttle,
                                       float depth_speed_mul) noexcept {
    // 008269B1 CMP [unit+C4h],8; 008269D0..008269DF COMISS -3.0f against unit+100h, JBE
    // skips, so an unordered compare skips too.
    if (class_id == 8 && -3.0f > pose_base_y) {
        return depth_speed_mul * throttle;  // 008269E6..008269F0
    }
    return throttle;
}

TorpedoBoatTurboClass torpedo_boat_turbo_class_00857f40(const TorpedoBoatTurboRow& row) noexcept {
    // 00857F77, 00857FB8, 00857FF9: each default is pushed into the reader 00B66330, which
    // returns it when the key is missing; the result is stored with FSTP float.
    TorpedoBoatTurboClass out{};
    if (row.has_time) out.turbo_time_808 = row.time;                                  // 00857F90
    if (row.has_strength) out.turbo_strength_80c = row.strength;                      // 00857FD1
    if (row.has_recharging_time) out.turbo_recharging_time_810 = row.recharging_time; // 00858012
    return out;
}

OceanVec3 tboat_rudder_torque_00937440(const TorpedoBoatRudderTorqueInputs& in) noexcept {
    // 00937472..0093748A: (vy*fy + vx*fx) + vz*fz, one FSTP float.
    const float along = (in.velocity.y * in.row2[1] + in.velocity.x * in.row2[0]) +
                        in.velocity.z * in.row2[2];
    // 0093748E..009374A7: the float is widened, divided by 0080FC30 (FDIVR m64), stored.
    const float ratio = listing_abs(along / in.reference_speed);  // 009374AB..009374C2
    const float heel = listing_abs(in.pose_row0_y);              // 00937542..00937567
    // 0093757F..0093759D: (1 - heel) by DE E1, times ratio, to_turn, settings+588h.
    float g = 1.0f - heel;
    g = g * ratio;
    g = g * in.to_turn;
    g = g * in.torque_mul;
    // 009375A1..009375BD: g times the pose's forward row, each stored.
    const float p0 = g * in.row2[0];
    const float p1 = g * in.row2[1];
    const float p2 = g * in.row2[2];
    // 009375C1..009375C7: mass squared, stored. 009375CB..009375EB: p_i * m2, stored.
    const float m2 = in.hull_mass * in.hull_mass;
    const float t0 = p0 * m2;
    const float t1 = p1 * m2;
    const float t2 = p2 * m2;
    // 009375EF..0093760F: each over the double 10000.0 at 00CE4BD8 (DC F9, D8 F1, D8 7C).
    OceanVec3 out{};
    out.x = static_cast<float>(static_cast<double>(t0) / 10000.0);
    out.y = static_cast<float>(static_cast<double>(t1) / 10000.0);
    out.z = static_cast<float>(static_cast<double>(t2) / 10000.0);
    return out;
}

float ship_righting_rate_0092e9e1_cc13(float rate, float angle, float dt) noexcept {
    // 0092E9F2..0092EA1C: |angle| by the listing's form, COMISS against 00D05AA8, JBE
    // skips (equality and unordered too).
    if (!(listing_abs(angle) > kShipRightingThreshold00D05AA8)) {
        return rate;
    }
    // 0092EA1E..0092EA3D: the sign as an int.
    int sign = 0;
    if (0.0f > angle) {
        sign = -1;
    } else if (angle > 0.0f) {
        sign = 1;
    }
    // 0092EA45..0092EA57: FILD, FMUL m64 00D19628, DE E2 (angle subtracted), FSTP float.
    const float target = static_cast<float>(static_cast<double>(sign) * kShipRightingAngle00D19628);
    const float error = target - angle;
    // 0092EA5B..0092EA6D: dt * 10.0 (FMUL m64 00CE3DC0), DE C9, DE E9 (rate minus it).
    const float gain = dt * 10.0f;
    const float step = error * gain;
    return rate - step;
}

}  // namespace bsp
