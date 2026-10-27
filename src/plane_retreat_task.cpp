// The plane `retreat` bot task (kind 9), packet cc9_plane_retreat_task.
// See include/bsp/plane_retreat_task.hpp and docs/SQUADRON_LAND_TASK.md 5ea.
// SUBSTITUTION, labelled: the x87 chains are taken in double and rounded to
// float at the listing's FSTP stores, as src/world_map_bounds.cpp does.
#include "bsp/plane_retreat_task.hpp"

#include <cmath>

#include "bsp/cc13_x87_model.hpp"
#include "bsp/dive_bomb_task.hpp"   // the per-op float 00419010 (kRetreatX87FaithfulCc13Bound)
#include "bsp/pc24_planetasks_cc13.hpp"
#include "bsp/plane_task_reach_sw3.hpp"

namespace bsp {
namespace {

constexpr double kHalfPi = 1.5707963705062866;   // 00CE3830, a float-rounded double
constexpr double kTwoPi = 6.2831854820251465;    // 00CE3828

float to_float(double v) noexcept { return static_cast<float>(v); }

// Every kRetreatX87FaithfulCc13Bound site below is a template on kFaithful (the
// switch's value) so that the sw3 reach counters can evaluate the other arm.

// 00419260 BSP_Vector2f_ReciprocalLength: the squares and their sum stored
// as floats (00419278, 00419286, 0041928E), then sqrt and the reciprocal.
template <bool kFaithful>
float reciprocal_length_00419260(float x, float z) noexcept {
    const float xx = to_float(static_cast<double>(x) * x);
    const float zz = to_float(static_cast<double>(z) * z);
    const float sum = to_float(static_cast<double>(zz) + xx);
    const float root = to_float(std::sqrt(static_cast<double>(sum)));
    // 004192B3-004192C2: a root equal to 0 returns 0 (FUCOMIP, LAHF/TEST
    // AH,44h/JP), not 1/0. Only behind kRetreatX87FaithfulCc13Bound.
    if constexpr (kFaithful) {
        return (root == 0.0f) ? 0.0f : to_float(1.0 / root);
    } else {
        return to_float(1.0 / root);
    }
}

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x): the lerp, then the
// result clamped to [min(y0, y1), max(y0, y1)]; y0 when x0 == x1.
// At PC24 (00419033-0041905F) the two FSUBs, the FDIVP, the FSUBP, the FMULP and
// the FADD each round to float. kRetreatX87FaithfulCc13Bound takes the per-op
// float form (dive_bomb_interpolate_clamped_00419010, same clamp). OFF is the
// double chain with one rounding, as before.
template <bool kFaithful>
float interpolate_clamped_00419010(float x0, float y0, float x1, float y1, float x) noexcept {
    if constexpr (kFaithful) {
        return dive_bomb_interpolate_clamped_00419010(x0, y0, x1, y1, x);
    } else {
        if (x1 == x0) return y0;
        const float v = to_float(((static_cast<double>(x) - x0) / (static_cast<double>(x1) - x0)) *
                                 (static_cast<double>(y1) - y0) + y0);
        const float hi = (y1 < y0) ? y0 : y1;
        const float lo = (y0 < y1) ? y0 : y1;
        if (v < lo) return lo;
        return (v <= hi) ? v : hi;
    }
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

// 009C8D40-009C8F36, __fastcall(ECX = retreat approach). The pose refresh
// (009C8D4B-009C8D59) is the caller's. 009C8D70 004C7730(unit+FCh, side
// [unit+54h], 0, 0) gives the zone at +3Ch. The edge +40h..+4Ch is copied from
// the zone's +1Ch/+24h/+28h/+30h. Then, each stored as float:
// - corner sums (((0 + a) + b) + c) + d over +10h/+18h, +1Ch/+24h,
//   +28h/+30h, +34h/+3Ch (009C8DB2-009C8E32);
// - quarter = sum * 0.25 [00D7A348];
// - mid = (+40h + +48h) * 0.5 and 0.5 * (+4Ch + +44h) [00D7A280];
// - to-centre = quarter - mid, and 00419260 r;
// - +58h/+5Ch = r * to-centre;
// - R = class+268h; the enter point +50h/+54h = R * dir * 3.0 [00D7A2B0] + mid.
// The listing's `FMUL ST1` at 009C8E40/8E6E/8EFB is the DC form (FMUL
// ST(1),ST0): the FXCH/FSTP that follows stores the product.
// DEFECT ROW (cc13-D2): the OFF arm runs these chains in double and rounds once.
// Because every image step is stored, that differs at PC24 and PC53 alike. The
// ON arm (kRetreatX87FaithfulCc13Bound) rounds at each store. DOCUMENTED: the
// null-zone return is a host guard (the image dereferences 004C7730's result).
namespace {
template <bool kFaithful>
bool retreat_refresh_zone_impl(RetreatApproach& approach, const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side, float turn_circle_radius) {
    const BorderZoneHit hit = closest_border_zone_004c7730(zones, position, side);
    if (hit.zone == nullptr) return false;
    const BorderZoneRecord& z = *hit.zone;
    approach.zone_3c = hit.zone;
    approach.edge_40[0] = z.b[0];   // +1Ch
    approach.edge_40[1] = z.b[2];   // +24h
    approach.edge_40[2] = z.c[0];   // +28h
    approach.edge_40[3] = z.c[2];   // +30h
    if constexpr (kFaithful) {
        float sx = 0.0f + z.a[0];                  // 009C8DB2-009C8DB8
        float sz = 0.0f + z.a[2];                  // 009C8DBC-009C8DC0
        sx = z.b[0] + sx;  sz = z.b[2] + sz;       // 009C8DD2-009C8DE6
        sx = z.c[0] + sx;  sz = z.c[2] + sz;       // 009C8DF8-009C8E0C
        sx = z.d[0] + sx;  sz = z.d[2] + sz;       // 009C8E1E-009C8E32
        const float quarter = static_cast<float>(kRetreatQuarter);
        const float half = static_cast<float>(kRetreatHalf);
        const float qx = sx * quarter;             // 009C8E40-009C8E44
        const float qz = quarter * sz;             // 009C8E48-009C8E4C
        const float ex = approach.edge_40[0] + approach.edge_40[2];  // 009C8E50-009C8E56
        const float ez = approach.edge_40[3] + approach.edge_40[1];  // 009C8E5A-009C8E60
        const float mid_x = ex * half;             // 009C8E6E-009C8E72
        const float mid_z = half * ez;             // 009C8E76-009C8E7A
        const float to_x = qx - mid_x;             // 009C8E7E-009C8E86
        const float to_z = qz - mid_z;             // 009C8E8A-009C8E96
        const float r = reciprocal_length_00419260<kFaithful>(to_x, to_z);   // 009C8E9A
        approach.direction_58[0] = r * to_x;       // 009C8EAF-009C8EB5, +58h
        approach.direction_58[1] = r * to_z;       // 009C8EB9-009C8EBD, +5Ch
        const float rx = turn_circle_radius * approach.direction_58[0];  // 009C8EE0-009C8EE6
        const float rz = turn_circle_radius * approach.direction_58[1];  // 009C8EEA-009C8EED
        const float three = static_cast<float>(kRetreatEnterPointScale);
        const float px = rx * three;               // 009C8EFB-009C8EFF
        const float pz = three * rz;               // 009C8F03-009C8F07
        approach.enter_point_50[0] = px + mid_x;   // 009C8F0B-009C8F27, +50h
        approach.enter_point_50[1] = pz + mid_z;   // 009C8F17-009C8F2E, +54h
    } else {
        const double mid_x = (static_cast<double>(approach.edge_40[0]) + approach.edge_40[2]) * kRetreatHalf;
        const double mid_z = kRetreatHalf * (static_cast<double>(approach.edge_40[3]) + approach.edge_40[1]);
        const float to_x = to_float(((((0.0 + z.a[0]) + z.b[0]) + z.c[0]) + z.d[0]) * kRetreatQuarter - mid_x);
        const float to_z = to_float(kRetreatQuarter * ((((0.0 + z.a[2]) + z.b[2]) + z.c[2]) + z.d[2]) - mid_z);
        const float r = reciprocal_length_00419260<kFaithful>(to_x, to_z);
        approach.direction_58[0] = to_float(static_cast<double>(r) * to_x);
        approach.direction_58[1] = to_float(static_cast<double>(r) * to_z);
        approach.enter_point_50[0] = to_float(static_cast<double>(turn_circle_radius) *
            approach.direction_58[0] * kRetreatEnterPointScale + mid_x);
        approach.enter_point_50[1] = to_float(kRetreatEnterPointScale * turn_circle_radius *
            approach.direction_58[1] + mid_z);
    }
    return true;
}
}  // namespace

bool retreat_refresh_zone_009c8d40(RetreatApproach& approach, const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side, float turn_circle_radius) {
    RetreatApproach other = approach;
    const bool ok = retreat_refresh_zone_impl<kRetreatX87FaithfulCc13Bound>(
        approach, zones, position, side, turn_circle_radius);
    {   // sw3 reach: the other arm on a copy.
        PlaneTaskReachShadow shadow;
        retreat_refresh_zone_impl<!kRetreatX87FaithfulCc13Bound>(
            other, zones, position, side, turn_circle_radius);
    }
    if (ok) {
        bool differs = false;
        for (int i = 0; i < 2; ++i) {
            differs = differs ||
                plane_task_reach_bits_differ(approach.direction_58[i], other.direction_58[i]) ||
                plane_task_reach_bits_differ(approach.enter_point_50[i], other.enter_point_50[i]);
        }
        plane_task_reach_count(g_plane_task_reach_sw3.retreat_zone, differs);
    }
    return ok;
}

// 00489C40-00489C90, cdecl-style RET 4 (point*) -> EAX = node+8h. A wrapper:
// 004C74C0(point, &iterator), then _invalid_parameter_noinfo (00BF6713) for a
// null list or the end node, then node+8h. 004C74C0 is folded in here. It was
// re-checked (cc13-D2) against its listing reconstruction
// world_border_zone_at_point_004c74c0 (src/pool_pool15_cc13.cpp).
// - Overshoots: d0 = p.z - NW.z, d2 = SE.z - p.z, d1 = p.x - SE.x,
//   d3 = NW.x - p.x.
// - Edge: the first of 0, 2, 1, 3 that is > 0 and above its two neighbours.
// - Walk: remaining = p.x - NW.x (edges 0/2) or NW.z - p.z. Subtract each
//   record's length and stop when 0 > remaining or at the last node.
// Each step is one float subtraction, so precision does not matter.
// DOCUMENTED DEVIATION: nullptr where the image raises the invalid-parameter
// path (empty list). It is also nullptr where the image takes its garbage edge
// (a point inside the map; 004C75B6 loads the point pointer as the edge). The
// callers ask only for a point outside the map (0071C4F0 first).
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
        float lo;
        float hi;
        if constexpr (kPc24PlaneTasksCc13Bound) {
            // pc24w1-sweep3-4: 009C8FC7-009C901D, m + m (FADD ST0,ST1) first, then
            // min + 2m (FADD) and max - 2m (FSUBR), each rounded at PC24 and stored.
            const double twice = cc13x87::x87_add24(m, m);
            lo = to_float(cc13x87::x87_add24((x1 < x2) ? x1 : x2, twice));
            hi = to_float(cc13x87::x87_sub24((x2 < x1) ? x1 : x2, twice));
        } else {
            lo = to_float(static_cast<double>((x1 < x2) ? x1 : x2) + m + m);
            hi = to_float(static_cast<double>((x2 < x1) ? x1 : x2) - (static_cast<double>(m) + m));
        }
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

// The countdown head of 009C9FB0 (009C9FB4-009C9FD9, task +560h/+55Ch) and of
// 009CA130 (009CA135-009CA158, approach +38h/+34h). FCOMI dt,c then JC: dt < c
// (and, ON, an unordered compare) decrements and stops; otherwise c += period -
// dt and the rule runs.
namespace {
template <bool kFaithful>
bool retreat_countdown_step_impl(float& countdown, float period, float dt) noexcept {
    const bool decrement = kFaithful ? !(dt >= countdown) : dt < countdown;
    if (decrement) {
        countdown = to_float(static_cast<double>(countdown) - dt);
        return false;
    }
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // pc24w1-sweep3-4: 009C9FCA FSUBR (period - dt), 009C9FD7 FADDP (+ countdown),
        // each rounded at PC24; 009CA151/009CA156 are the same.
        countdown = to_float(cc13x87::x87_add24(cc13x87::x87_sub24(period, dt), countdown));
    } else {
        countdown = to_float((static_cast<double>(period) - dt) + countdown);
    }
    return true;
}
}  // namespace

bool retreat_countdown_step(float& countdown, float period, float dt) noexcept {
    float other = countdown;
    const bool runs = retreat_countdown_step_impl<kRetreatX87FaithfulCc13Bound>(countdown, period, dt);
    const bool other_runs =
        retreat_countdown_step_impl<!kRetreatX87FaithfulCc13Bound>(other, period, dt);
    plane_task_reach_count(g_plane_task_reach_sw3.retreat_countdown, runs != other_runs,
                           plane_task_reach_bits_differ(countdown, other));   // sw3 reach
    return runs;
}

float retreat_moveto_timer_009c92f0(float turn_circle_radius, float travel_speed) noexcept {
    return to_float(static_cast<double>(turn_circle_radius) / travel_speed);
}

// 009C9FB0-009CA08F, __thiscall(ECX = task, float dt), RET 4. The rule after
// the countdown head: follow (+4C4h) -> moveto (+478h) for a 007B8AD0 leader;
// moveto -> leave (+4ACh) on +458h, else enter zone (+494h) unless the timer
// (+478h+18h) is not below 0 and [+478h+4]+61h is clear; enter zone -> leave
// when 009C9EA0. Each change is 009C9F70 / 007B6EE0 (the caller's enter).
// DOCUMENTED DEVIATION (cc13-D2): the countdown head is retreat_countdown_step,
// which the units host runs first (the native is one function), and the
// 009C9EA0 test is an input the caller computes before the rule (it is pure).
// DEFECT (OFF-switched): 009CA048 COMISS 0,timer / JA enters the zone only when
// 0 > timer, so a NaN timer stays; the OFF arm's `>= 0` test leaves on a NaN.
namespace {
template <bool kFaithful>
RetreatState retreat_arm_impl(const RetreatArmInputs& in) noexcept {
    switch (in.state) {
    case RetreatState::kFollow:
        return in.flight_leader ? RetreatState::kMoveTo : RetreatState::kFollow;
    case RetreatState::kMoveTo:
        if (in.leave_now_60) return RetreatState::kLeave;
        // 009CA02C: stay while the timer is >= 0 and +61h is clear.
        {
            bool stay;
            if constexpr (kFaithful) {
                stay = !(0.0f > in.moveto_timer) && !in.lined_up_61;
            } else {
                stay = (0.0f < in.moveto_timer || in.moveto_timer == 0.0f) && !in.lined_up_61;
            }
            if (stay) return RetreatState::kMoveTo;
        }
        return RetreatState::kEnterZone;
    case RetreatState::kEnterZone:
        return in.inside_enter_zone ? RetreatState::kLeave : RetreatState::kEnterZone;
    default:
        return in.state;
    }
}
}  // namespace

RetreatState retreat_arm_009c9fb0(const RetreatArmInputs& in) noexcept {
    const RetreatState state = retreat_arm_impl<kRetreatX87FaithfulCc13Bound>(in);
    plane_task_reach_count(g_plane_task_reach_sw3.retreat_arm,
        state != retreat_arm_impl<!kRetreatX87FaithfulCc13Bound>(in));   // sw3 reach
    return state;
}

namespace {
template <bool kFaithful>
bool retreat_inside_enter_zone_impl(const RetreatApproach& approach, float x, float z,
    float turn_circle_radius) noexcept {
    const float dx = to_float(static_cast<double>(x) - approach.enter_point_50[0]);
    const float dz = to_float(static_cast<double>(z) - approach.enter_point_50[1]);
    // 009C9EE8-009C9EF8: dx*dx, dz*dz and their sum (FADDP) with no store between,
    // so at PC24 (kRetreatX87FaithfulCc13Bound) each rounds to float; 009C9F06
    // FCOMI tests the stored float sum against 1e-10 [00CE3820], JBE (also on
    // NaN) to 0. 009C9F3C FCOMIP R,d / JBE: a NaN distance answers false.
    float d;
    if constexpr (kFaithful) {
        const float xx = dx * dx;
        const float zz = dz * dz;
        const float sq = zz + xx;
        d = !(static_cast<double>(sq) > kRetreatTinySquare) ? 0.0f : std::sqrt(sq);
    } else {
        const double sq = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
        d = sq <= kRetreatTinySquare ? 0.0f : to_float(std::sqrt(sq));
    }
    return d < turn_circle_radius;
}
}  // namespace

bool retreat_inside_enter_zone_009c9ea0(const RetreatApproach& approach, float x, float z,
    float turn_circle_radius) noexcept {
    const bool inside = retreat_inside_enter_zone_impl<kRetreatX87FaithfulCc13Bound>(
        approach, x, z, turn_circle_radius);
    plane_task_reach_count(g_plane_task_reach_sw3.retreat_inside,
        inside != retreat_inside_enter_zone_impl<!kRetreatX87FaithfulCc13Bound>(
            approach, x, z, turn_circle_radius));   // sw3 reach
    return inside;
}

namespace {
template <bool kFaithful>
RetreatMoveToSteer retreat_moveto_steer_impl(const RetreatMoveToInputs& in) noexcept {
    RetreatMoveToSteer out;
    const float m = in.margin_30;
    const float max_z = in.bounds.north_west[2];   // +7124h
    const float min_z = in.bounds.south_east[2];   // +7130h
    const float max_x = in.bounds.south_east[0];   // +7128h
    const float min_x = in.bounds.north_west[0];   // +711Ch
    // 009C93BD, 009C93F6, 009C942F, 009C9468.
    const float w1 = interpolate_clamped_00419010<kFaithful>(max_z, 1.0f, to_float(static_cast<double>(max_z) - m), 0.0f, in.z);
    const float w2 = interpolate_clamped_00419010<kFaithful>(min_z, 1.0f, to_float(static_cast<double>(min_z) + m), 0.0f, in.z);
    const float w3 = interpolate_clamped_00419010<kFaithful>(max_x, 1.0f, to_float(static_cast<double>(max_x) - m), 0.0f, in.x);
    const float w4 = interpolate_clamped_00419010<kFaithful>(min_x, 1.0f, to_float(static_cast<double>(min_x) + m), 0.0f, in.x);
    // 009C94D4-009C95B0: the push, rows (0,-1), (0,1), (-1,0), (1,0).
    float push_x = to_float(static_cast<double>(to_float(0.0 - w3)) + w4);
    float push_z = to_float(static_cast<double>(to_float(0.0 - w1)) + w2);
    // 009C95B4-009C95D2: (((0 + w1) + w2) + w3) + w4.
    float w = to_float(static_cast<double>(to_float(static_cast<double>(
        to_float(0.0 + w1)) + w2)) + w3);
    w = to_float(static_cast<double>(w4) + w);
    if (0.0f < w) {
        const float r = reciprocal_length_00419260<kFaithful>(push_x, push_z);
        push_x = to_float(static_cast<double>(r) * push_x);
        push_z = to_float(static_cast<double>(r) * push_z);
        if (1.0f <= w) w = 1.0f;
    }
    out.weight = w;
    // 009C9672-009C96E8: the planar distance to the target.
    const float dx = to_float(static_cast<double>(in.target_x) - in.x);
    const float dz = to_float(static_cast<double>(in.target_z) - in.z);
    // 009C96AD-009C96BD: dx*dx, dz*dz and their sum (FADDP) with no store between,
    // so at PC24 (kRetreatX87FaithfulCc13Bound) each rounds to float; the stored
    // float sum is what FCOMI tests against 1e-10 [00CE3820] at 009C96CB.
    if constexpr (kFaithful) {
        const float xx = dx * dx;
        const float zz = dz * dz;
        const float sq = zz + xx;
        out.distance = !(static_cast<double>(sq) > kRetreatTinySquare) ? 0.0f
                                                                        : std::sqrt(sq);
    } else {
        const double sq = static_cast<double>(dx) * dx + static_cast<double>(dz) * dz;
        out.distance = sq <= kRetreatTinySquare ? 0.0f : to_float(std::sqrt(sq));
    }
    out.timer_runs = in.turn_circle_radius > out.distance;   // 009C96FB
    // 009C9716-009C978B.
    const float r = reciprocal_length_00419260<kFaithful>(dx, dz);
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
    out.desired_speed_2b4 = interpolate_clamped_00419010<kFaithful>(0.0f, in.squadron_speed_3a0, 1.0f,
                                                         in.max_spd_188, w);
    return out;
}
}  // namespace

RetreatMoveToSteer retreat_moveto_steer_009c9310(const RetreatMoveToInputs& in) noexcept {
    const RetreatMoveToSteer out = retreat_moveto_steer_impl<kRetreatX87FaithfulCc13Bound>(in);
    RetreatMoveToSteer other;
    {   // sw3 reach: the other arm.
        PlaneTaskReachShadow shadow;
        other = retreat_moveto_steer_impl<!kRetreatX87FaithfulCc13Bound>(in);
    }
    plane_task_reach_count(g_plane_task_reach_sw3.retreat_steer,
        plane_task_reach_bits_differ(out.weight, other.weight) ||
        plane_task_reach_bits_differ(out.heading_2c0, other.heading_2c0) ||
        plane_task_reach_bits_differ(out.desired_speed_2b4, other.desired_speed_2b4) ||
        plane_task_reach_bits_differ(out.distance, other.distance) ||
        out.timer_runs != other.timer_runs);
    return out;
}

// 009F9D90-009F9E32, __thiscall(ECX = approach, point*), RET 4. dz = p.z -
// unit+104h and dx = p.x - unit+FCh, each stored as float. Then atan2(dz, dx)
// (00BF701A, ST1/ST0), pi/2 [00CE3830] - a and, when 0 > h, + 2pi [00CE3828],
// each stored as float. That is heading_from_vector exactly; the image result
// does not depend on PC.
// DOCUMENTED DEVIATION (cc13-D2): the stores plan+2C0h = heading and
// plan+2CCh = 2 (009F9E1C/009F9E24) are made by the units host from the
// return value.
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
