// 009BEE30's fly-to arm.  See include/bsp/plane_follow_law.hpp for the
// addresses, the contract with 009BFEE0 and what is provisional.
#include "bsp/plane_follow_law.hpp"

#include <cmath>

#include "bsp/dive_bomb_task.hpp"

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

}  // namespace bsp
