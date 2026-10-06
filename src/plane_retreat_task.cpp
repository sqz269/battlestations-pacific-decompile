// The plane `retreat` bot task (kind 9), packet cc9_plane_retreat_task.
// See include/bsp/plane_retreat_task.hpp and docs/SQUADRON_LAND_TASK.md 5ea.
// SUBSTITUTION, labelled: the x87 chains are taken in double and rounded to
// float at the listing's FSTP stores, as src/world_map_bounds.cpp does.
#include "bsp/plane_retreat_task.hpp"

#include <cmath>

namespace bsp {
namespace {

constexpr double kHalfPi = 1.5707963705062866;   // 00CE3830, a float-rounded double
constexpr double kTwoPi = 6.2831854820251465;    // 00CE3828

float to_float(double v) noexcept { return static_cast<float>(v); }

// 00419260 BSP_Vector2f_ReciprocalLength: the squares and their sum stored
// as floats (00419278, 00419286, 0041928E), then sqrt and the reciprocal.
float reciprocal_length_00419260(float x, float z) noexcept {
    const float xx = to_float(static_cast<double>(x) * x);
    const float zz = to_float(static_cast<double>(z) * z);
    const float sum = to_float(static_cast<double>(zz) + xx);
    const float root = to_float(std::sqrt(static_cast<double>(sum)));
    return to_float(1.0 / root);
}

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x): the lerp, then the
// result clamped to [min(y0, y1), max(y0, y1)]; y0 when x0 == x1.
float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) noexcept {
    if (x1 == x0) return y0;
    const float v = to_float(((static_cast<double>(x) - x0) / (static_cast<double>(x1) - x0)) *
                             (static_cast<double>(y1) - y0) + y0);
    const float hi = (y1 < y0) ? y0 : y1;
    const float lo = (y0 < y1) ? y0 : y1;
    if (v < lo) return lo;
    return (v <= hi) ? v : hi;
}

// pi/2 - atan2(z, x) as a float, plus 2 pi when negative (009C979C-009C97B8,
// 009C99AF-009C99C9, 009F9DEE-009F9E08).
float heading_from_vector(float x, float z) noexcept {
    const float a = to_float(std::atan2(static_cast<double>(z), static_cast<double>(x)));
    const float h = to_float(kHalfPi - a);
    if (0.0f > h) return to_float(static_cast<double>(h) + kTwoPi);
    return h;
}

}  // namespace

bool retreat_refresh_zone_009c8d40(RetreatApproach& approach, const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side, float turn_circle_radius) {
    const BorderZoneHit hit = closest_border_zone_004c7730(zones, position, side);
    if (hit.zone == nullptr) return false;
    const BorderZoneRecord& z = *hit.zone;
    approach.zone_3c = hit.zone;
    approach.edge_40[0] = z.b[0];   // +1Ch
    approach.edge_40[1] = z.b[2];   // +24h
    approach.edge_40[2] = z.c[0];   // +28h
    approach.edge_40[3] = z.c[2];   // +30h
    const double mid_x = (static_cast<double>(approach.edge_40[0]) + approach.edge_40[2]) * kRetreatHalf;
    const double mid_z = kRetreatHalf * (static_cast<double>(approach.edge_40[3]) + approach.edge_40[1]);
    const float to_x = to_float(((((0.0 + z.a[0]) + z.b[0]) + z.c[0]) + z.d[0]) * kRetreatQuarter - mid_x);
    const float to_z = to_float(kRetreatQuarter * ((((0.0 + z.a[2]) + z.b[2]) + z.c[2]) + z.d[2]) - mid_z);
    const float r = reciprocal_length_00419260(to_x, to_z);
    approach.direction_58[0] = to_float(static_cast<double>(r) * to_x);
    approach.direction_58[1] = to_float(static_cast<double>(r) * to_z);
    approach.enter_point_50[0] = to_float(static_cast<double>(turn_circle_radius) *
        approach.direction_58[0] * kRetreatEnterPointScale + mid_x);
    approach.enter_point_50[1] = to_float(kRetreatEnterPointScale * turn_circle_radius *
        approach.direction_58[1] + mid_z);
    return true;
}

const BorderZoneRecord* border_zone_at_point_00489c40(const WorldMapBounds& bounds,
    const BorderZoneSet& zones, const std::array<float, 3>& point) {
    const float north = to_float(static_cast<double>(point[2]) - bounds.north_west[2]);  // +7124h
    const float south = to_float(static_cast<double>(bounds.south_east[2]) - point[2]);  // +7130h
    const float east = to_float(static_cast<double>(point[0]) - bounds.south_east[0]);   // +7128h
    const float west = to_float(static_cast<double>(bounds.north_west[0]) - point[0]);   // +711Ch
    int edge = -1;
    if (0.0f < north && east < north && west < north) edge = 0;
    else if (0.0f < south && east < south && west < south) edge = 2;
    else if (0.0f < east && north < east && south < east) edge = 1;
    else if (0.0f < west && north < west && south < west) edge = 3;
    if (edge < 0) return nullptr;
    const auto& list = zones.edges[static_cast<std::size_t>(edge)];
    if (list.empty()) return nullptr;
    // 004C75A0.. / 004C7690..: edges 0 and 2 measure x - NW.x, 1 and 3 NW.z - z.
    float remaining = (edge == 0 || edge == 2)
        ? to_float(static_cast<double>(point[0]) - bounds.north_west[0])
        : to_float(static_cast<double>(bounds.north_west[2]) - point[2]);
    for (std::size_t i = 0; i < list.size(); ++i) {
        remaining = to_float(static_cast<double>(remaining) - list[i].length);
        if (remaining < 0.0f || i + 1 == list.size()) return &list[i];
    }
    return nullptr;
}

void retreat_refresh_target_009c8f40(RetreatApproach& approach, const WorldMapBounds& bounds,
    const BorderZoneSet& zones, float x, float z) {
    const float m = approach.margin_30;
    const float x1 = approach.edge_40[0], z1 = approach.edge_40[1];
    const float x2 = approach.edge_40[2], z2 = approach.edge_40[3];
    bool lined_up = false;
    if (x1 == x2) {
        // 009C8F7A-009C8FD1: an x-constant edge.
        const float lo = (z1 < z2) ? z1 : z2;
        const float hi = (z2 < z1) ? z1 : z2;
        approach.target_64[0] = x1;
        approach.target_64[1] = (lo <= z) ? ((z <= hi) ? z : hi) : lo;
        lined_up = to_float(static_cast<double>(lo) - m) < z &&
                   z < to_float(static_cast<double>(hi) + m);
    } else {
        // 009C9010-009C9110: a z-constant edge, inset by twice the margin.
        const float lo = to_float(static_cast<double>((x1 < x2) ? x1 : x2) + m + m);
        const float hi = to_float(static_cast<double>((x2 < x1) ? x1 : x2) - (static_cast<double>(m) + m));
        approach.target_64[1] = z1;
        approach.target_64[0] = (lo <= x) ? ((x <= hi) ? x : hi) : lo;
        lined_up = to_float(static_cast<double>(lo) - m) < x &&
                   x < to_float(static_cast<double>(hi) + m);
    }
    approach.lined_up_61 = lined_up;
    approach.leave_now_60 = false;
    const std::array<float, 3> p{x, 0.0f, z};
    if (point_outside_world_map_0071c4f0(bounds, p)) {
        const BorderZoneRecord* at = border_zone_at_point_00489c40(bounds, zones, p);
        if (at != nullptr && at == approach.zone_3c) approach.leave_now_60 = true;
    }
}

bool retreat_countdown_step(float& countdown, float period, float dt) noexcept {
    if (dt < countdown) {
        countdown = to_float(static_cast<double>(countdown) - dt);
        return false;
    }
    countdown = to_float((static_cast<double>(period) - dt) + countdown);
    return true;
}

float retreat_moveto_timer_009c92f0(float turn_circle_radius, float travel_speed) noexcept {
    return to_float(static_cast<double>(turn_circle_radius) / travel_speed);
}

RetreatState retreat_arm_009c9fb0(const RetreatArmInputs& in) noexcept {
    switch (in.state) {
    case RetreatState::kFollow:
        return in.flight_leader ? RetreatState::kMoveTo : RetreatState::kFollow;
    case RetreatState::kMoveTo:
        if (in.leave_now_60) return RetreatState::kLeave;
        // 009CA02C: stay while the timer is >= 0 and +61h is clear.
        if ((0.0f < in.moveto_timer || in.moveto_timer == 0.0f) && !in.lined_up_61)
            return RetreatState::kMoveTo;
        return RetreatState::kEnterZone;
    case RetreatState::kEnterZone:
        return in.inside_enter_zone ? RetreatState::kLeave : RetreatState::kEnterZone;
    default:
        return in.state;
    }
}

bool retreat_inside_enter_zone_009c9ea0(const RetreatApproach& approach, float x, float z,
    float turn_circle_radius) noexcept {
    const float dx = to_float(static_cast<double>(x) - approach.enter_point_50[0]);
    const float dz = to_float(static_cast<double>(z) - approach.enter_point_50[1]);
    const double sq = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
    const float d = sq <= kRetreatTinySquare ? 0.0f : to_float(std::sqrt(sq));
    return d < turn_circle_radius;
}

RetreatMoveToSteer retreat_moveto_steer_009c9310(const RetreatMoveToInputs& in) noexcept {
    RetreatMoveToSteer out;
    const float m = in.margin_30;
    const float max_z = in.bounds.north_west[2];   // +7124h
    const float min_z = in.bounds.south_east[2];   // +7130h
    const float max_x = in.bounds.south_east[0];   // +7128h
    const float min_x = in.bounds.north_west[0];   // +711Ch
    // 009C93BD, 009C93F6, 009C942F, 009C9468.
    const float w1 = interpolate_clamped_00419010(max_z, 1.0f, to_float(static_cast<double>(max_z) - m), 0.0f, in.z);
    const float w2 = interpolate_clamped_00419010(min_z, 1.0f, to_float(static_cast<double>(min_z) + m), 0.0f, in.z);
    const float w3 = interpolate_clamped_00419010(max_x, 1.0f, to_float(static_cast<double>(max_x) - m), 0.0f, in.x);
    const float w4 = interpolate_clamped_00419010(min_x, 1.0f, to_float(static_cast<double>(min_x) + m), 0.0f, in.x);
    // 009C94D4-009C95B0: the push, rows (0,-1), (0,1), (-1,0), (1,0).
    float push_x = to_float(static_cast<double>(to_float(0.0 - w3)) + w4);
    float push_z = to_float(static_cast<double>(to_float(0.0 - w1)) + w2);
    // 009C95B4-009C95D2: (((0 + w1) + w2) + w3) + w4.
    float w = to_float(static_cast<double>(to_float(static_cast<double>(
        to_float(0.0 + w1)) + w2)) + w3);
    w = to_float(static_cast<double>(w4) + w);
    if (0.0f < w) {
        const float r = reciprocal_length_00419260(push_x, push_z);
        push_x = to_float(static_cast<double>(r) * push_x);
        push_z = to_float(static_cast<double>(r) * push_z);
        if (1.0f <= w) w = 1.0f;
    }
    out.weight = w;
    // 009C9672-009C96E8: the planar distance to the target.
    const float dx = to_float(static_cast<double>(in.target_x) - in.x);
    const float dz = to_float(static_cast<double>(in.target_z) - in.z);
    const double sq = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
    out.distance = sq <= kRetreatTinySquare ? 0.0f : to_float(std::sqrt(sq));
    out.timer_runs = in.turn_circle_radius > out.distance;   // 009C96FB
    // 009C9716-009C978B.
    const float r = reciprocal_length_00419260(dx, dz);
    const float ux = to_float(static_cast<double>(r) * dx);
    const float uz = to_float(static_cast<double>(r) * dz);
    const float om = to_float(1.0 - w);
    const float ax = to_float(static_cast<double>(om) * ux);
    const float az = to_float(static_cast<double>(om) * uz);
    const float bx = to_float(static_cast<double>(push_x) * w);
    const float bz = to_float(static_cast<double>(w) * push_z);
    const float vz = to_float(static_cast<double>(bz) + az);
    const float vx = to_float(static_cast<double>(bx) + ax);
    out.heading_2c0 = heading_from_vector(vx, vz);
    // 009C97C0-009C97F6.
    out.desired_speed_2b4 = interpolate_clamped_00419010(0.0f, in.squadron_speed_3a0, 1.0f,
                                                         in.max_spd_188, w);
    return out;
}

float retreat_heading_to_point_009f9d90(float x, float z, const float point[2]) noexcept {
    const float dz = to_float(static_cast<double>(point[1]) - z);
    const float dx = to_float(static_cast<double>(point[0]) - x);
    return heading_from_vector(dx, dz);
}

float retreat_leave_heading_009c9990(const RetreatApproach& approach) noexcept {
    return heading_from_vector(approach.direction_58[0], approach.direction_58[1]);
}

bool point_beyond_map_margin_0059c9b0(const WorldMapBounds& bounds, float x, float z,
    float margin) noexcept {
    if (x < to_float(static_cast<double>(bounds.north_west[0]) - margin)) return true;
    if (x <= to_float(static_cast<double>(bounds.south_east[0]) + margin) &&
        z <= to_float(static_cast<double>(bounds.north_west[2]) + margin) &&
        to_float(static_cast<double>(bounds.south_east[2]) - margin) <= z)
        return false;
    return true;
}

}  // namespace bsp
