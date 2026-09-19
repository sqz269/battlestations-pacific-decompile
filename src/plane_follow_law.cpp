// 009BEE30's fly-to arm.  See include/bsp/plane_follow_law.hpp for the
// addresses, the contract with 009BFEE0 and what is provisional.
#include "bsp/plane_follow_law.hpp"

#include <cmath>

#include "bsp/dive_bomb_task.hpp"
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

// 009C0142-009C0164 and 009C0F86-009C0FAE both wrap with a SINGLE conditional
// add of 2*pi ([00CE3828] = 6.2831854820251465), not a loop; an input below
// -2*pi therefore stays negative in the image and does so here.
float wrap_once_to_two_pi_009c0150(float a) noexcept {
    if (!(0.0f <= a)) {
        return static_cast<float>(static_cast<double>(a) + 6.2831854820251465);
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

}  // namespace

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
        static_cast<float>(1.5707963705062866 - bearing_raw));

    // 009C0176-009C01B2.  R * (sin, cos) of the bearing taken in the reference
    // frame: the cross-track and along-track offsets from the station.
    const float a0 = wrapped_angle_subtract_00438b10(bearing, ref);
    out.cross_track = r * static_cast<float>(std::sin(static_cast<double>(a0)));
    out.along_track = r * static_cast<float>(std::cos(static_cast<double>(a0)));

    // 009C01B6-009C01D3, then the quadrant classifier 009C01D3-009C024F.
    // Recorded because it is Phase A's input, not because it selects a regime:
    // Phase A overwrites BL before the dispatch reads it.
    const float a = wrapped_angle_subtract_00438b10(in.own_heading, ref);
    out.heading_error = a;
    {
        const bool s = (out.cross_track >= 0.0f);
        const bool aa = (a >= 0.0f);
        const bool near_axis =
            (std::fabs(static_cast<double>(a)) <= 1.5707963705062866);
        out.quadrant_bl = (s == aa) ? (near_axis ? 2 : 4) : (near_axis ? 1 : 3);
    }

    // 009C0F0D-009C0F80: the leader's horizontal forward length, floored at
    // [00D7A238] = 0.01 so a vertical leader cannot degenerate the direction.
    float lh = horizontal_range_0042b2f0(in.leader_forward[0], in.leader_forward[2]);
    if (!(static_cast<double>(lh) >= 0.009999999776482582)) {
        lh = 0.009999999776482582f;
    }

    // 009C0F86-009C104C.  U is the unit vector along the LAGGED leader heading
    // carrying the leader's own vertical slope.  The `LEA EAX,[EDI+0CCh]` at
    // 009C100F is dead: 00419510 is __fastcall(out = ECX, v = EDX) and returns
    // `out` in EAX (00419545 MOV EAX,EDI), so the copy that follows reads the
    // normalized local, not the leader's pose row 0.
    const float t_dir = wrap_once_to_two_pi_009c0150(
        static_cast<float>(1.5707963705062866 - static_cast<double>(ref)));
    const float u_raw[3] = {
        lh * static_cast<float>(std::cos(static_cast<double>(t_dir))),
        in.leader_forward[1],
        lh * static_cast<float>(std::sin(static_cast<double>(t_dir))),
    };
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

    float p[3];
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
                p[i] += static_cast<float>(0.20000000298023224 *
                                           static_cast<double>(d)) * n[i];
            }
            // 009C11EF-009C1239: and pushed FollowedPointDist further along.
            for (int i = 0; i < 3; ++i) p[i] += in.followed_point_dist * u[i];
            break;
        }
        case PlaneFollowRegime::kAbeam: {
            // 009C15C0-009C1661.  The horizontal perpendicular of the OWN
            // nose.  pose row 2 normalized is (sin h, cos h) for the slot +50h
            // heading h, which is what makes deriving it from `own_heading`
            // the same vector the listing builds from pose +ECh/+F4h.
            const float ux =
                static_cast<float>(std::sin(static_cast<double>(in.own_heading)));
            const float uz =
                static_cast<float>(std::cos(static_cast<double>(in.own_heading)));
            // 009C1646: BL&2 set -> (-uz, ux), the LEFT perpendicular; clear ->
            // (uz, -ux), the RIGHT one.  Phase A sets that bit from the saved
            // sign of V (009C08E7 `TEST BL,BL` on base-21h), so the side is the
            // side the aircraft is already on.
            const bool left = (out.cross_track < 0.0f);
            const float dir_x = left ? -uz : uz;
            const float dir_z = left ? ux : -ux;
            // 009C167B-009C16AF.
            p[0] = in.own_pos[0] + in.followed_point_dist * dir_x;
            p[2] = in.own_pos[2] + in.followed_point_dist * dir_z;
            // 009C16B2: stationY + U.y * base-0Ch.  SUBSTITUTION, labelled:
            // base-0Ch is Phase A's (009C0EE1-009C0F00 scales it by base-8h and
            // nothing writes it again before here), so D stands in for it -
            // the same slope-times-distance shape the lead regime uses.
            p[1] = in.station[1] + u[1] * d;
            break;
        }
        case PlaneFollowRegime::kOffsetPoint009c1328: {
            // 009C12DD-009C1336: station + base-0Ch * U, with the same
            // substitution for base-0Ch.  The perpendicular this block also
            // builds (009C124D-009C12D9) feeds computation past 009C1365 that
            // is not part of the steer point and is not bound.
            for (int i = 0; i < 3; ++i) p[i] = in.station[i] + d * u[i];
            break;
        }
    }

    out.station_y = in.station[1];
    out.steer_point[0] = p[0];
    out.steer_point[1] = p[1];
    out.steer_point[2] = p[2];

    // 009C16D2-009C1846, the tail.  BOTH Y values into one leader-relative
    // band.  This is the `done` floor that keeps a spent wing member out of the
    // water while its leader flies, and it bounds whatever the substitutions
    // above did to the steer Y.
    if (in.band_inputs_available) {
        const float floor_candidate = in.leader_pos[1] + in.band_floor_offset;
        const float floor_v =
            (floor_candidate < in.state_88) ? floor_candidate : in.state_88;
        const float ceil_candidate = static_cast<float>(
            static_cast<double>(in.leader_pos[1]) + 120.0);
        const float ceil_v = (in.band_ceiling_210 < ceil_candidate)
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
