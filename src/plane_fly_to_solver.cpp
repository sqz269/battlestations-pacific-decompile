#include "bsp/plane_fly_to_solver.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 00CE3830 and 00CE3828: the float pi/2 and the float 2*pi, both stored in the
// image as DOUBLES promoted from those floats. Every use here is an `FSUBR
// qword` or `FADD qword` against an x87 register holding a float, so the
// promoted values are the ones the native arithmetic sees.
constexpr double kHalfPi_00ce3830 = 1.5707963705062866;
constexpr double kTwoPi_00ce3828 = 6.2831854820251465;

// 009FDEAC-009FDEC4, 009FDB79-009FDB93 and 009FDBCA-009FDBE4 are the same three
// instructions: convert the math-convention angle to a compass heading and pull
// it into [0, 2pi). The test is `0 > h`, so an exact zero is left alone.
float wrap_to_two_pi(float h) noexcept {
    if (0.0f > h) {
        return static_cast<float>(static_cast<double>(h) + kTwoPi_00ce3828);
    }
    return h;
}

float bearing_of(float x, float z) noexcept {
    // _CIatan2 takes y in ST(1) and x in ST(0); all three sites push z first,
    // so every one of them is atan2(z, x). 009FDB6C, 009FDBBD, 009FDE97.
    const float a = static_cast<float>(std::atan2(static_cast<double>(z),
                                                  static_cast<double>(x)));
    return wrap_to_two_pi(
        static_cast<float>(kHalfPi_00ce3830 - static_cast<double>(a)));
}

// 009FD837 / 009FDA0C / 009FDADB / 009FDE11 all guard the square root the same
// way: below 1e-10 the length is zero rather than sqrt of a denormal.
float guarded_sqrt(float v2) noexcept {
    if (static_cast<double>(v2) > fly_to_solver::kEpsilon) {
        return static_cast<float>(std::sqrt(static_cast<double>(v2)));
    }
    return 0.0f;
}

}  // namespace

float fly_to_bearing_of_009fde8f(float x, float z) noexcept {
    return bearing_of(x, z);
}

bool fly_to_avoidance_009fd749(const float lead_point[3],
                               const FlyToObstacle* obstacles,
                               std::size_t count, float out_steer[2]) noexcept {
    // 009FD74D seeds the accumulator from the zero vector at 00F87574 and the
    // best-extent tracker from -1.0 (00D7A260).
    float ax = 0.0f;
    float az = 0.0f;
    float best = -1.0f;

    for (std::size_t i = 0; i < count; ++i) {
        const FlyToObstacle& o = obstacles[i];
        const float dx = lead_point[0] - o.position[0];
        const float dz = lead_point[2] - o.position[2];
        // 009FD829 FLDZ / FMUL ST0: the y term is an explicit zero, so this is
        // a horizontal range and an obstacle directly below still counts.
        const float dist2 = dx * dx + 0.0f + dz * dz;
        const float dist = guarded_sqrt(dist2);

        // 009FD8B4 and 009FD8D1, both strict-ish: `dist <= 1` and `r <= dist`
        // drop the obstacle.
        if (!(dist > 1.0f)) continue;
        const float r = static_cast<float>(
            static_cast<double>(o.extent_max) +
            static_cast<double>(fly_to_solver::kObstacleRadiusPad));
        if (!(r > dist)) continue;

        const float nx = dx / dist;
        const float nz = dz / dist;
        const float t = dist / r;
        // 009FD921: 1.0 out to 0.7 of the padded radius, then down to 0 at the
        // rim. The arguments are (x0, y0, x1, y1, x), so y FALLS as x rises.
        const float w = clamped_interpolate_00419010(
            fly_to_solver::kFalloffNear, 1.0f, 1.0f, 0.0f, t);
        const float s = o.extent_sum;
        // 009FD930-009FD984: the unit vector is scaled by the falloff and then
        // by the obstacle's own size, in that order.
        const float cx = nx * w * s;
        const float cz = nz * w * s;
        if (s > best) best = s;
        ax += cx;
        az += cz;
    }

    out_steer[0] = 0.0f;
    out_steer[1] = 0.0f;
    if (!(best > 0.0f)) return false;

    // 009FD9EC-009FDA7D. The length is taken over all three components but only
    // x and z are divided, which is harmless because the y contribution is the
    // zero the loop always writes.
    const float len = guarded_sqrt(ax * ax + 0.0f + az * az);
    const float m = (len > best) ? len : best;
    const float k = static_cast<float>(static_cast<double>(m) /
                                       static_cast<double>(fly_to_solver::kLeadSeconds));
    out_steer[0] = ax / k;
    out_steer[1] = az / k;
    return true;
}

FlyToSolverResult fly_to_point_heading_009fd570(const FlyToSolverInputs& in,
                                                float side) noexcept {
    FlyToSolverResult out;
    out.side = side;

    // 009FD5A8-009FD61F: lead = unit position + 3.0 * unit->vtable[34h]().
    const float lead[3] = {
        in.unit_position[0] +
            static_cast<float>(static_cast<double>(in.unit_lead_vector[0]) *
                               static_cast<double>(fly_to_solver::kLeadSeconds)),
        in.unit_position[1] +
            static_cast<float>(static_cast<double>(in.unit_lead_vector[1]) *
                               static_cast<double>(fly_to_solver::kLeadSeconds)),
        in.unit_position[2] +
            static_cast<float>(static_cast<double>(in.unit_lead_vector[2]) *
                               static_cast<double>(fly_to_solver::kLeadSeconds)),
    };

    float avoid[2] = {0.0f, 0.0f};
    out.avoidance_ran = fly_to_avoidance_009fd749(lead, in.obstacles,
                                                  in.obstacle_count, avoid);

    // 009FDA87-009FDB35: the direct leg, from the POINT to the lead position.
    // The sign is that way round in the image, so the bare bearing points AWAY
    // from the point and the blend below is what turns it back in.
    const float dx = lead[0] - in.point[0];
    const float dz = lead[2] - in.point[2];
    const float dist = guarded_sqrt(dx * dx + 0.0f + dz * dz);
    const float inv = (dist > 0.0f) ? (1.0f / dist) : 0.0f;
    const float nx = dx * inv;
    const float nz = dz * inv;

    float steer_x = nx;
    float steer_z = nz;

    // 009FDB39-009FDB59. e is how far outside the standoff ring the unit is.
    const float e = in.range - in.standoff;
    if (static_cast<double>(e) > static_cast<double>(fly_to_solver::kBlendLow)) {
        out.blend_ran = true;
        const float x = e * in.offset_scale;
        const float h = bearing_of(nx, nz);

        if (out.avoidance_ran) {
            // 009FDBB5-009FDC48. The avoidance bearing decides which way round
            // the point to go, and the latch only ever flips one way per call.
            const float h2 = bearing_of(avoid[0], avoid[1]);
            const float delta = wrapped_angle_subtract_00438b10(h, h2);
            if (static_cast<double>(delta) >
                    static_cast<double>(fly_to_solver::kSideSwitch) &&
                out.side > 0.0f) {
                out.side = -1.0f;
                out.side_written = true;
            } else if (static_cast<double>(delta) <
                           -static_cast<double>(fly_to_solver::kSideSwitch) &&
                       out.side < 0.0f) {
                out.side = 1.0f;
                out.side_written = true;
            }
        }

        // 009FDC57-009FDC97: the offset ramps 0 -> pi as the unit goes from
        // 200 m inside the ring to 300 m outside it, so a unit far outside is
        // commanded straight AT the point and one deep inside straight away
        // from it. `side` picks the hand of the turn in between.
        const float offset = clamped_interpolate_00419010(
            fly_to_solver::kBlendLow, 0.0f, fly_to_solver::kBlendHigh,
            fly_to_solver::kOffsetHigh, x);
        const float hc = wrapped_angle_add_00438aa0(h, offset * out.side);

        // 009FDC9C-009FDCD6: back to a math angle, then cos/sin. That is the
        // exact inverse of bearing_of, so (cos g, sin g) == (sin hc, cos hc);
        // the literal form is kept because the wrap is what the image stores.
        const float g = wrap_to_two_pi(static_cast<float>(
            kHalfPi_00ce3830 - static_cast<double>(hc)));
        const float cg = static_cast<float>(std::cos(static_cast<double>(g)));
        const float sg = static_cast<float>(std::sin(static_cast<double>(g)));

        // 009FDCE5-009FDD2D: the direct term's weight, 1 at 200 m inside the
        // ring up to 4 at 300 m outside. It only matters against the avoidance
        // and world-edge terms, which are not rescaled.
        const float weight = clamped_interpolate_00419010(
            fly_to_solver::kBlendLow, fly_to_solver::kWeightLow,
            fly_to_solver::kBlendHigh, fly_to_solver::kWeightHigh, x);
        steer_x = weight * cg;
        steer_z = weight * sg;
    }

    // 009FDD39-009FDD54.
    steer_x += avoid[0];
    steer_z += avoid[1];

    // 009FDD62-009FDE8B: only within 500 m of a world edge, and then pulling
    // toward the map centre with a gain of the overrun over 60.
    if (in.world_edge.near_edge) {
        const float q = static_cast<float>(
            static_cast<double>(in.world_edge.penetration) /
            static_cast<double>(fly_to_solver::kEdgeGainDivisor));
        const float ux = in.world_edge.centre[0] - lead[0];
        const float uz = in.world_edge.centre[1] - lead[2];
        const float ulen = guarded_sqrt(ux * ux + 0.0f + uz * uz);
        const float uinv = (ulen > 0.0f) ? (1.0f / ulen) : 0.0f;
        steer_x += (ux * uinv) * q;
        steer_z += (uz * uinv) * q;
    }

    out.steer[0] = steer_x;
    out.steer[1] = steer_z;
    out.heading = bearing_of(steer_x, steer_z);
    return out;
}

}  // namespace bsp
