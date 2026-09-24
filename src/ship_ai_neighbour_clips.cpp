// Packet cc9_neighbour_clips. Semantic reconstructions; see
// bsp/ship_ai_neighbour_clips.hpp and docs/SHIP_NEIGHBOUR_AVOIDANCE.md section 4.
#include "bsp/ship_ai_neighbour_clips.hpp"

#include "bsp/avoid_zone_arc.hpp"   // avoid_zone_circle_segment_004f3ba0
#include "bsp/native_scalar_float_by_ref.hpp" // max_native_float_by_ref_00415550
#include "bsp/unit_rudder.hpp"      // wrapped_angle_add_00438aa0 / _subtract_00438b10
#include "bsp/ship_ai_throttle_ring.hpp" // heading_to_direction_006bc0c0

#include <cmath>

namespace bsp {
namespace {

constexpr double kHalfPi = 1.5707963705062866;    // 00CE3830
constexpr double kFullTurn = 6.2831854820251465;  // 00CE3828
constexpr float kNegZero = -0.0f;                 // 00D7A208
constexpr float kPi = 3.14159274f;                // 00D7A264
constexpr float kStillSpeed = 1.0f;               // 00D7A24C
constexpr float kStraightError = 0.0872664675f;   // 00CEDF5C, 5 degrees
constexpr double kBehindMargin = 200.0;           // 00CE4D70
constexpr double kLengthEpsilon = 1.0e-10;        // 00CE3820
constexpr float kLeaderHold = 0.5f;               // 00CE3800
constexpr float kPassInterval = 0.9f;             // 00CE3860

// The four corners in the order 009DD010 and 009DD540 store them, closed by a
// copy of the first: (+beam, +fwd), (+beam, -fwd), (-beam, -fwd), (-beam, +fwd).
std::array<std::array<float, 2>, 5> box_polygon(const ShipAiObstacleNode& n) noexcept {
    const float hb = n.avoid_half_beam;
    const float hl = n.avoid_half_length;
    const float bx = n.avoid_box_x + hb * n.corner_beam_x;
    const float bz = n.avoid_box_z + hb * n.corner_beam_z;
    const float mx = n.avoid_box_x - hb * n.corner_beam_x;
    const float mz = n.avoid_box_z - hb * n.corner_beam_z;
    const float fx = hl * n.corner_forward_x;
    const float fz = hl * n.corner_forward_z;
    std::array<std::array<float, 2>, 5> p{};
    p[0] = {bx + fx, bz + fz};
    p[1] = {bx - fx, bz - fz};
    p[2] = {mx - fx, mz - fz};
    p[3] = {mx + fx, mz + fz};
    p[4] = p[0];
    return p;
}

float square_length(float x, float z) noexcept { return z * z + x * x; }

} // namespace

std::array<float, 2> ship_ai_obstacle_closest_point_009d8a30(
    const ShipAiObstacleNode& n, const std::array<float, 2>& point) noexcept {
    if (n.no_pose_68) return {n.avoid_box_x, n.avoid_box_z};
    const float dx = point[0] - n.avoid_box_x;
    const float dz = point[1] - n.avoid_box_z;
    float u = -n.avoid_half_beam;
    const float t = dx * n.corner_beam_x + dz * n.corner_beam_z;
    if (u <= t) u = (n.avoid_half_beam < t) ? n.avoid_half_beam : t;
    float v = -n.avoid_half_length;
    const float s = dz * n.corner_forward_z + n.corner_forward_x * dx;
    if (v <= s) v = (n.avoid_half_length < s) ? n.avoid_half_length : s;
    return {n.avoid_box_x + u * n.corner_beam_x + v * n.corner_forward_x,
            n.avoid_box_z + u * n.corner_beam_z + v * n.corner_forward_z};
}

std::array<float, 2> ship_ai_obstacle_support_point_009d8860(
    const ShipAiObstacleNode& n, const std::array<float, 2>& dir) noexcept {
    if (n.no_pose_68) return {n.avoid_box_x, n.avoid_box_z};
    const float along_forward = n.corner_forward_z * dir[1] + n.corner_forward_x * dir[0];
    const float along_beam = dir[1] * n.corner_beam_z + dir[0] * n.corner_beam_x;
    const float hb = n.avoid_half_beam;
    const float fx = n.avoid_half_length * n.corner_forward_x;
    const float fz = n.avoid_half_length * n.corner_forward_z;
    float bx, bz;
    if (along_beam < 0.0f) {
        bx = n.avoid_box_x - hb * n.corner_beam_x;
        bz = n.avoid_box_z - hb * n.corner_beam_z;
    } else {
        bx = n.avoid_box_x + hb * n.corner_beam_x;
        bz = n.avoid_box_z + hb * n.corner_beam_z;
    }
    if (0.0f <= along_forward) return {bx + fx, bz + fz};
    return {bx - fx, bz - fz};
}

bool ship_ai_obstacle_clip_arc_009dd010(const ShipAiObstacleNode& n,
                                        const std::array<float, 2>& centre, float radius,
                                        float from_bearing, float& io_to_bearing,
                                        const CameraAxesCrtAccess& crt) {
    if (n.no_pose_68 || n.no_arc_69) return false;               // 009DD013, 009DD01D
    const float r2 = radius * radius;                            // 009DD030
    const auto p = box_polygon(n);
    const std::array<float, 2> nearest = ship_ai_obstacle_closest_point_009d8a30(n, centre);
    if (square_length(nearest[0] - centre[0], nearest[1] - centre[1]) > r2) return false; // 009DD29A
    // 009DD2A9..009DD376: corners 0, 2, 3, then 0 again.
    const float d0 = square_length(p[0][0] - centre[0], p[0][1] - centre[1]);
    const float d2 = square_length(p[2][0] - centre[0], p[2][1] - centre[1]);
    const float d3 = square_length(p[3][0] - centre[0], p[3][1] - centre[1]);
    if (r2 > d0 && r2 > d2 && r2 > d3 && r2 > d0) return false;
    bool narrowed = false;
    for (int edge = 0; edge < 4; ++edge) {
        std::array<std::array<float, 2>, 2> hits{};
        const int count = avoid_zone_circle_segment_004f3ba0(centre, p[edge], radius,
                                                            p[edge + 1], hits, crt);
        if (count == 0) continue;
        const float delta = wrapped_angle_subtract_00438b10(io_to_bearing, from_bearing);
        for (int i = 0; i < count; ++i) {
            const float dx = hits[i][0] - centre[0];
            const float dz = hits[i][1] - centre[1];
            const float angle = static_cast<float>(std::atan2(static_cast<double>(dz),
                                                              static_cast<double>(dx)));
            float bearing = static_cast<float>(kHalfPi - static_cast<double>(angle));
            if (0.0f > bearing) bearing = static_cast<float>(bearing + kFullTurn);
            const float rel = wrapped_angle_subtract_00438b10(bearing, from_bearing);
            if (delta > 0.0f) {
                if (rel > 0.0f && delta > rel) {                // 009DD483, 009DD497
                    narrowed = true;
                    io_to_bearing = wrapped_angle_add_00438aa0(from_bearing, rel);
                }
            } else if (0.0f > rel && rel > delta) {             // 009DD4BE, 009DD4D3
                narrowed = true;
                io_to_bearing = wrapped_angle_add_00438aa0(from_bearing, rel);
            }
        }
    }
    return narrowed;
}

bool ship_ai_obstacle_segment_ray_009d8210(const ShipAiObstacleNode& n,
                                           const std::array<float, 2>& a,
                                           const std::array<float, 2>& b,
                                           const std::array<float, 2>& o,
                                           const std::array<float, 2>& d,
                                           float& io_range) noexcept {
    if (n.no_pose_68 || n.no_arc_69) return false;               // 009D8213, 009D821D
    const float ax = a[0] - o[0], az = a[1] - o[1];
    const float bx = b[0] - o[0], bz = b[1] - o[1];
    const float along_a = d[1] * az + d[0] * ax;
    const float along_b = d[1] * bz + d[0] * bx;
    // COMISS/FCOMI take the continuing branch on an unordered compare.
    if (!(0.0f < along_a) && !std::isnan(along_a) && 0.0f > along_b) return false; // 009D82AD, 009D82F5
    const float range = io_range;
    const bool a_short = along_a < range || std::isnan(along_a) || std::isnan(range);
    const bool b_short = along_b < range || std::isnan(along_b) || std::isnan(range);
    if (!a_short && !b_short) return false;                      // 009D830E, 009D8316
    const float side_a = az * -d[0] + ax * d[1];                 // perpendicular (dz, -dx)
    const float side_b = bz * -d[0] + bx * d[1];
    if (!(0.0f > side_b * side_a)) return false;                 // 009D83AC
    const float t = std::fabs(side_a) / (std::fabs(side_b) + std::fabs(side_a));
    const float px = t * (b[0] - a[0]) + a[0];
    const float pz = a[1] + t * (b[1] - a[1]);
    const float hit = d[1] * (pz - o[1]) + (px - o[0]) * d[0];
    if (!(hit > 0.0f) || !(range > hit)) return false;           // 009D848D, 009D849B
    io_range = hit;                                              // 009D84AB
    return true;
}

bool ship_ai_obstacle_clip_ray_009dd540(const ShipAiObstacleNode& n, bool owner_gone_5e,
                                        const std::array<float, 2>& o,
                                        const std::array<float, 2>& d,
                                        float& io_range) noexcept {
    if (n.owner == nullptr || owner_gone_5e || n.no_pose_68) return false;
    auto dot_from_origin = [&](const std::array<float, 2>& q,
                               const std::array<float, 2>& axis) noexcept {
        return axis[1] * (q[1] - o[1]) + axis[0] * (q[0] - o[0]);
    };
    // 009DD578: the box lies wholly behind the origin.
    const auto s1 = ship_ai_obstacle_support_point_009d8860(n, d);
    if (0.0f >= dot_from_origin(s1, d)) return false;            // 009DD5DD JAE
    // 009DD623: its nearest point along the ray is at or past the range.
    const auto s2 = ship_ai_obstacle_support_point_009d8860(n, {kNegZero - d[0], kNegZero - d[1]});
    if (dot_from_origin(s2, d) >= io_range) return false;        // 009DD66F JAE
    // 009DD693 / 009DD710: wholly on one side of the ray line.
    const std::array<float, 2> perp{d[1], kNegZero - d[0]};
    const auto s3 = ship_ai_obstacle_support_point_009d8860(n, perp);
    if (0.0f >= dot_from_origin(s3, perp)) return false;         // 009DD6D9 JAE
    const std::array<float, 2> perp_neg{kNegZero - d[1], d[0]};
    const auto s4 = ship_ai_obstacle_support_point_009d8860(n, perp_neg);
    if (!(0.0f < dot_from_origin(s4, perp_neg)) &&
        !std::isnan(dot_from_origin(s4, perp_neg))) return false; // 009DD75A JB
    const auto p = box_polygon(n);
    bool hit = false;
    for (int edge = 0; edge < 4; ++edge) {
        hit = ship_ai_obstacle_segment_ray_009d8210(n, p[edge], p[edge + 1], o, d, io_range) || hit;
    }
    return hit;
}

bool ship_ai_neighbour_pass_gate_009d8010(ShipAiObstacleNode& n, ShipAiNeighbourPassState& pass,
                                          bool owner_gone_5e, bool unit_present,
                                          bool unit_leads_owner_group, float dt) noexcept {
    if (n.owner == nullptr || owner_gone_5e) return false;       // 009D8016, 009D801C
    pass.timer_7c = pass.timer_7c - dt;                          // 009D8030..009D803B
    if (n.no_pose_68) {
        n.pass_side_88 = 0;                                      // 009D803F
    } else if (unit_present && unit_leads_owner_group) {         // 009D8050..009D806E
        const float hold = kLeaderHold;
        pass.timer_7c = max_native_float_by_ref_00415550(&pass.timer_7c, &hold); // 009D8084
        n.pass_side_88 = 0;                                      // 009D808B
    }
    return !n.no_pose_68 && 0.0f > pass.timer_7c;                // 009D8096..009D80A5
}

ShipAiOrderTailPassResult ship_ai_order_tail_neighbour_pass_009f0100(
    const ShipAiOrderTailPassInputs& in, float dt, ShipAiOrderTailPassHost& host,
    const CameraAxesCrtAccess&) {
    ShipAiOrderTailPassResult result;
    auto clear = [&](int i) {
        ShipAiObstacleNode& n = host.node_608(i);
        n.pass_side_88 = 0;
        host.pass_state(i).flag_75 = false;
    };
    auto gate = [&](int i, float step) {
        return ship_ai_neighbour_pass_gate_009d8010(host.node_608(i), host.pass_state(i),
                                                    host.owner_gone_5e(i), in.unit_present,
                                                    host.unit_leads_owner_group(i), step);
    };
    // 009F0120..009F01B6.
    if (in.mode_35c == 0 && kStillSpeed > std::fabs(in.body_speed)) {
        result.clear_arm = true;
        for (int i = 0; i < host.count_604(); ++i) {
            if (gate(i, dt)) clear(i);                           // 009F019F, 009F01A5
        }
        return result;
    }
    // 009F01BB..009F0201: every node's timer moves; any due one opens the body.
    for (int i = 0; i < host.count_604(); ++i) {
        if (gate(i, dt)) result.any_due = true;
    }
    if (!result.any_due) return result;

    const std::array<float, 2> pose = in.pose_184;
    float heading = in.heading;
    std::array<float, 2> forward = in.forward_1ac;
    if (in.mode_35c == 2) {                                      // 009F0260
        heading = wrapped_angle_add_00438aa0(heading, kPi);
        forward = {kNegZero - forward[0], kNegZero - forward[1]};
    }
    const float error = wrapped_angle_subtract_00438b10(in.heading_target_324, heading);
    float target_bearing = static_cast<float>(kHalfPi - static_cast<double>(in.heading_target_324));
    if (0.0f > target_bearing) target_bearing = static_cast<float>(target_bearing + kFullTurn);
    const float c = static_cast<float>(std::cos(static_cast<double>(target_bearing)));
    const float s = static_cast<float>(std::sin(static_cast<double>(target_bearing)));
    const float neg_c = kNegZero - c;                            // [esp+60h]
    // A ([esp+44h]) is the reference `along` is measured from; B ([esp+34h]) the
    // point the far branch measures to. `side_flag` is BL.
    std::array<float, 2> ref_a{}, ref_b{};
    bool side_flag = true;
    const float r = in.turn_radius_3cc;
    if (!(std::fabs(error) > kStraightError)) {                  // 009F0301 JBE
        ref_a = pose;
        ref_b = {pose[0] + r * (kNegZero - s), pose[1] + r * c};
    } else {
        result.turning = true;
        bool starboard;
        bool offset_sc;   // true: (s, -c); false: (-s, c)
        if (in.mode_35c == 2) {
            if (error > 0.0f) { starboard = true; offset_sc = false; side_flag = false; }
            else { starboard = false; offset_sc = true; side_flag = true; }
        } else {
            if (0.0f > error) { starboard = true; offset_sc = true; side_flag = true; }
            else { starboard = false; offset_sc = false; side_flag = false; }
        }
        ref_b = starboard ? in.starboard_194 : in.port_18c;
        const float ox = offset_sc ? r * s : r * (kNegZero - s);
        const float oz = offset_sc ? r * neg_c : r * (kNegZero - neg_c);
        ref_a = {ox + ref_b[0], oz + ref_b[1]};
    }

    const int count = host.count_604();
    for (int i = 0; i < count; ++i) {
        if (!gate(i, 0.0f)) continue;                            // 009F0650, FLDZ
        ShipAiObstacleNode& n = host.node_608(i);
        ++result.processed;
        const float rx = n.avoid_box_x - ref_a[0];
        const float rz = n.avoid_box_z - ref_a[1];
        const float along = rz * s + rx * c;
        const double scale = static_cast<double>(in.hull_scale_3e4);
        const double limit = result.turning ? -scale : -(scale + kBehindMargin);
        int post = 0;
        bool cleared = false;
        if (!(static_cast<double>(along) < limit)) {             // 009F06CE JB
            if (along > in.remaining_32c + in.corner_reach_add_1c8) {
                cleared = true;                                  // 009F06EC
            } else {
                const float lateral = rz * neg_c + rx * s;       // 009F06FF..009F070E
                if (std::fabs(lateral) > n.avoid_half_beam + in.line_check_1cc) {
                    cleared = true;                              // 009F073C
                } else {
                    float threshold = 0.0f;
                    if (n.pass_side_88 == 1) threshold = kNegZero - in.own_width_9cc;
                    else if (n.pass_side_88 == 2) threshold = in.own_width_9cc;
                    post = (lateral >= threshold) ? 1 : 2;       // 009F079E JB
                }
            }
        } else {
            const auto sup = ship_ai_obstacle_support_point_009d8860(n, forward);
            const float qx = sup[0] - pose[0];
            const float qz = sup[1] - pose[1];
            const float ahead = qz * forward[1] + forward[0] * qx;
            if (!(static_cast<double>(ahead) > -(scale + kBehindMargin))) {
                cleared = true;                                  // 009F084A JBE
            } else {
                const float vx = ref_b[0] - n.avoid_box_x;
                const float vz = ref_b[1] - n.avoid_box_z;
                const float d2 = vx * vx + vz * vz;
                float dist = (static_cast<double>(d2) > kLengthEpsilon)
                                 ? static_cast<float>(std::sqrt(static_cast<double>(d2))) : 0.0f;
                const bool minus = side_flag ? (n.pass_side_88 == 2) : (n.pass_side_88 == 1);
                dist = minus ? dist - in.own_width_9cc : in.own_width_9cc + dist;
                if (r < dist) {                                  // 009F0908 JB, 009F0A33
                    const auto sup3 = ship_ai_obstacle_support_point_009d8860(n, {vx, vz});
                    const float wx = ref_b[0] - sup3[0];
                    const float wz = ref_b[1] - sup3[1];
                    const float k = r + in.line_check_1cc;
                    if (wx * wx + wz * wz > k * k) cleared = true;   // 009F0AB1 JA
                    else post = side_flag ? 1 : 2;                   // 009F0AB7..009F0ABF
                } else {
                    const auto sup2 = ship_ai_obstacle_support_point_009d8860(
                        n, {kNegZero - vx, kNegZero - vz});
                    const float wx = ref_b[0] - sup2[0];
                    const float wz = ref_b[1] - sup2[1];
                    bool inner = false;
                    if (r > in.line_check_1cc) {                 // 009F0994
                        const float k = r - in.line_check_1cc;
                        inner = k * k > wx * wx + wz * wz;       // 009F09BD
                    }
                    if (inner) cleared = true;
                    else post = side_flag ? 2 : 1;               // 009F0A1E..009F0A27
                }
            }
        }
        if (cleared) {
            clear(i);
            ++result.clears;
        } else if (post != 0 && in.unit_present && n.owner != nullptr) {
            host.post_pass_side_009d8c60(i, post);               // 009D8CAA..009D8CC4
            ++result.posts;
        }
        host.pass_state(i).timer_7c = kPassInterval;             // 009F09E0
    }
    return result;
}

// ---------------------------------------------------------------------------
// Packet cc9_pass_side_message.
// ---------------------------------------------------------------------------

namespace {
constexpr double kPassCornerFar2 = 225.0;          // 00CF8EB0
constexpr float kPassBearingOffset = 0.0872664675f;  // 00CEDF5C, 5 degrees
constexpr float kCrossingEpsilon = 0.001f;          // 00D7A23C
constexpr double kCrossingLengthScale = 2.5;        // 00CE3DE0
constexpr float kCrossingRangeCap = 400.0f;         // 00CFD710
constexpr double kCrossingWidthScale = 4.0;         // 00D7A328
constexpr double kCrossingLengthDivisor = 1.8000000476837158; // 00D049A8
constexpr float kCrossingLengthFloor = 100.0f;      // 00CE3D08
constexpr double kCrossingHalf = 0.5;               // 00D7A280
constexpr float kTrafficOpenSide = 3.14f;           // 00CF0AA8
constexpr double kTrafficWindowNudge = 0.01;        // 00D7A358
constexpr float kTrafficNoDistance = 1.0e8f;        // 00D21A90
constexpr float kTrafficHold = 3.0f;                // 00CE3854

// (+/-beam, +/-forward) corner of the avoid box.
std::array<float, 2> box_corner(const ShipAiObstacleNode& n, bool plus_beam,
                                bool plus_forward) noexcept {
    const float bx = n.avoid_half_beam * n.corner_beam_x;
    const float bz = n.avoid_half_beam * n.corner_beam_z;
    const float fx = n.avoid_half_length * n.corner_forward_x;
    const float fz = n.avoid_half_length * n.corner_forward_z;
    const float x = plus_beam ? n.avoid_box_x + bx : n.avoid_box_x - bx;
    const float z = plus_beam ? n.avoid_box_z + bz : n.avoid_box_z - bz;
    return plus_forward ? std::array<float, 2>{x + fx, z + fz}
                        : std::array<float, 2>{x - fx, z - fz};
}

float bearing_of(float dx, float dz) noexcept {
    const float angle = static_cast<float>(std::atan2(static_cast<double>(dz),
                                                      static_cast<double>(dx)));
    float bearing = static_cast<float>(kHalfPi - static_cast<double>(angle));
    if (0.0f > bearing) bearing = static_cast<float>(bearing + kFullTurn);
    return bearing;
}
} // namespace

std::array<float, 2> ship_ai_obstacle_pass_corner_009d7af0(
    const ShipAiObstacleNode& n, const std::array<float, 2>& point) noexcept {
    if (n.no_pose_68) return {n.avoid_box_x, n.avoid_box_z};   // 009D7AF7
    const float dx = point[0] - n.avoid_box_x;
    const float dz = point[1] - n.avoid_box_z;
    const float b = dz * n.corner_beam_z + dx * n.corner_beam_x;      // local beam
    const float f = dz * n.corner_forward_z + n.corner_forward_x * dx; // local forward
    const float hb = n.avoid_half_beam;
    const float hl = n.avoid_half_length;
    if (n.pass_side_88 == 1) {
        if (b < 0.0f) {
            if (f < 0.0f) return box_corner(n, !(-f <= hl), false);
            return box_corner(n, false, -b <= hb);
        }
        if (0.0f <= f) return f <= hl ? box_corner(n, true, true) : box_corner(n, false, true);
        return box_corner(n, true, hb < b);
    }
    if (b < 0.0f) {
        if (0.0f <= f) return box_corner(n, !(f <= hl), true);
        return box_corner(n, false, hb < -b);
    }
    if (f < 0.0f) return box_corner(n, !(hl < -f), false);
    return box_corner(n, true, b <= hb);
}

void ship_ai_neighbour_pass_bearing_009dceb0(const ShipAiObstacleNode& n,
                                             ShipAiNeighbourPassState& pass,
                                             const std::array<float, 2>& pose,
                                             const std::array<float, 2>& dir) {
    pass.flag_75 = false;                                        // 009DCEBC
    pass.flag_74 = false;                                        // 009DCEBF
    const auto s = ship_ai_obstacle_support_point_009d8860(n, dir);
    const float ahead = dir[1] * (s[1] - pose[1]) + dir[0] * (s[0] - pose[0]);
    if (0.0f > ahead) return;                                    // 009DCF13 JA
    const int side = n.pass_side_88;                             // 009DCF1A
    const auto corner = ship_ai_obstacle_pass_corner_009d7af0(n, pose);
    const float dx = corner[0] - pose[0];
    const float dz = corner[1] - pose[1];
    const float d2 = dx * dx + dz * dz;
    pass.distance2_6c = d2;                                      // 009DCF71 FST
    if (!(static_cast<double>(d2) > kPassCornerFar2)) return;    // 009DCF80 JBE
    const float bearing = bearing_of(dx, dz);
    pass.bearing_70 = bearing;                                   // 009DCFC7
    pass.bearing_70 = side == 1
        ? wrapped_angle_subtract_00438b10(bearing, kPassBearingOffset)   // 009DCFD1
        : wrapped_angle_add_00438aa0(bearing, kPassBearingOffset);       // 009DCFE9
    pass.flag_74 = true;
    pass.flag_75 = true;
}

void ship_ai_predict_track_crossing_009d8ce0(
    ShipAiObstacleNode& n1, ShipAiNeighbourPassState& p1, int side,
    const ShipAiObstacleNode* n2, bool n2_owner_torn_down_5d,
    const ShipAiOrderSlotView& o_slot, const ShipAiOrderSlotView& m_slot,
    float o_length_9c8, float m_length_9c8, float m_width_9cc) {
    // 009D8CEC..009D8D1F, then 009D9118.
    if (n2 == nullptr || side == 0 || side == n2->pass_side_88 || n2->pass_side_88 == 0 ||
        n2_owner_torn_down_5d) {
        p1.negotiated_8c = 0;
    } else {
        // n1 sees the other ship ("o"), n2 sees this one ("m"); each near-box
        // centre is where the observer places the observed hull.
        const auto od = heading_to_direction_006bc0c0(o_slot.heading_44);  // 009D8D63
        const auto md = heading_to_direction_006bc0c0(m_slot.heading_44);  // 009D8DCE
        const float ox = n1.near_box_x, oz = n1.near_box_z;
        const float mx = n2->near_box_x, mz = n2->near_box_z;
        const float nx = od[1];                 // the other track's normal (oz, -ox)
        const float nz = kNegZero - od[0];
        const float across = (mz - oz) * nz + (mx - ox) * nx;          // 009D8E44
        const float next = ((md[1] + mz) - oz) * nz + ((md[0] + mx) - ox) * nx;
        const float denominator = across - next;                         // 009D8EB2
        if (std::fabs(denominator) > kCrossingEpsilon) {                 // 009D8ED6
            const float t = across / denominator;                        // 009D8EE3
            const float cx = t * md[0] + mx;
            const float cz = t * md[1] + mz;
            const float along_o = (cz - oz) * od[1] + (cx - ox) * od[0];  // 009D8F5C
            const float along_m = md[1] * (cz - mz) + (cx - mx) * md[0];  // 009D8F7C
            float cap = kCrossingRangeCap;
            float scaled = static_cast<float>(m_length_9c8 * kCrossingLengthScale);
            const float range_m = min_native_float_by_ref_00415510(&scaled, &cap);
            cap = kCrossingRangeCap;
            scaled = static_cast<float>(o_length_9c8 * kCrossingLengthScale);
            const float range_o = min_native_float_by_ref_00415510(&scaled, &cap);
            const float width = static_cast<float>(m_width_9cc * kCrossingWidthScale);
            int verdict = 0;
            bool close = width > std::fabs(across);                      // 009D8FEC JA
            if (!close) {
                const float lim_o = min_native_float_by_ref_00415510(&range_m, &o_slot.distance_40);
                if (lim_o > along_o) {                                   // 009D900D
                    const float lim_m = min_native_float_by_ref_00415510(&range_o, &m_slot.distance_40);
                    close = lim_m > along_m;                             // 009D9032
                }
            }
            if (close) {                                                 // 009D9038
                const float o_margin = static_cast<float>(o_length_9c8 / kCrossingLengthDivisor);
                const float floor = kCrossingLengthFloor;
                const float m_margin = max_native_float_by_ref_00415550(&floor, &m_length_9c8);
                bool decide = false;
                if (m_margin + along_m > m_slot.remaining_48) {          // 009D906D
                    verdict = 1;
                } else if (along_o > 0.0f) {                             // 009D908A
                    if (o_margin + along_o > o_slot.remaining_48) verdict = 1;   // 009D909B
                    else decide = true;
                } else if (along_m > 0.0f) {                             // 009D90D9
                    decide = true;
                }
                if (decide) {                                            // 009D90DE
                    float along = along_o;
                    if (p1.negotiated_8c == 2) {
                        along = static_cast<float>(along - o_margin * kCrossingHalf);
                    }
                    verdict = (along_m > along) ? 2 : 1;                 // 009D9102
                }
            }
            if (verdict != p1.negotiated_8c) p1.negotiated_8c = verdict; // 009D90B9
            if (p1.negotiated_8c == 2) side = n2->pass_side_88;          // 009D90C7
        }
    }
    n1.pass_side_88 = side;                                          // 009D912F
    p1.flag_75 = true;                                               // 009D9135
}

ShipAiTrafficPassResult ship_ai_traffic_pass_009ef350(
    const ShipAiTrafficPassInputs& in, float& heading_target_324, float& clearance_33c,
    float& hold_354, ShipAiTrafficPassHost& host) {
    ShipAiTrafficPassResult result;
    const int count = host.count_604();
    if (count == 0) return result;                                   // 009EF359
    const std::array<float, 2> dir = (in.mode_35c == 2)
        ? std::array<float, 2>{kNegZero - in.forward_1ac[0], kNegZero - in.forward_1ac[1]}
        : in.forward_1ac;
    int best1 = -1, best2 = -1;
    float d1 = 0.0f, d2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
    for (int i = 0; i < count; ++i) {
        ShipAiObstacleNode& n = host.node_608(i);
        ShipAiNeighbourPassState& p = host.pass_state(i);
        p.flag_75 = false;
        p.flag_74 = false;
        if (n.pass_side_88 == 0 || n.owner == nullptr || host.owner_gone_5e(i)) continue;
        const int party = host.owner_party_54(i);
        if (party < 0 || party > 2 || !in.party_accepted[party]) continue;
        ship_ai_neighbour_pass_bearing_009dceb0(n, p, in.pose_184, dir);
        if (!p.flag_75 || n.no_arc_69) continue;
        p.flag_74 = true;
        if (n.pass_side_88 == 1) {
            if (result.side1 == 0 || p.distance2_6c < d1) {
                d1 = p.distance2_6c; best1 = i; a1 = p.bearing_70;
            }
            ++result.side1;
        } else {
            if (result.side2 == 0 || p.distance2_6c < d2) {
                d2 = p.distance2_6c; a2 = p.bearing_70; best2 = i;
            }
            ++result.side2;
        }
    }
    if (result.side2 <= 0 && result.side1 <= 0) return result;
    if (best2 >= 0) host.pass_state(best2).flag_74 = false;
    if (best1 >= 0) host.pass_state(best1).flag_74 = false;
    float heading = host.unit_heading_vtable50();
    if (in.mode_35c == 2) heading = wrapped_angle_add_00438aa0(heading, kPi);
    if (result.side2 == 0) a2 = wrapped_angle_subtract_00438b10(heading, kTrafficOpenSide);
    else if (result.side1 == 0) a1 = wrapped_angle_add_00438aa0(heading, kTrafficOpenSide);
    float lo = wrapped_angle_subtract_00438b10(a2, heading);
    float hi = wrapped_angle_subtract_00438b10(a1, heading);
    const int both = (result.side2 == 0 || result.side1 == 0) ? 1 : 2;
    int rounds = result.side1 - both + result.side2;                 // 009EF61D..009EF621
    while (rounds > 0 && hi > lo) {                                  // 009EF640
        int i1 = -1, i2 = -1;
        float m1 = kTrafficNoDistance, m2 = kTrafficNoDistance;
        for (int i = 0; i < count; ++i) {
            const ShipAiObstacleNode& n = host.node_608(i);
            const ShipAiNeighbourPassState& p = host.pass_state(i);
            if (!p.flag_74 || !p.flag_75 || n.no_arc_69) continue;
            if (n.pass_side_88 == 1) {
                if (m1 > p.distance2_6c) { m1 = p.distance2_6c; i1 = i; }
            } else if (n.pass_side_88 == 2) {
                if (m2 > p.distance2_6c) { m2 = p.distance2_6c; i2 = i; }
            }
        }
        if (!(m1 > m2)) {                                            // 009EF6FC JBE
            if (i1 >= 0) {
                ShipAiNeighbourPassState& p = host.pass_state(i1);
                p.flag_74 = false;
                const float a = wrapped_angle_subtract_00438b10(p.bearing_70, heading);
                if (hi > a) { d1 = p.distance2_6c; hi = a; }         // 009EF79C
            }
        } else if (i2 >= 0) {
            ShipAiNeighbourPassState& p = host.pass_state(i2);
            p.flag_74 = false;
            const float a = wrapped_angle_subtract_00438b10(p.bearing_70, heading);
            if (a > lo) { lo = a; d2 = p.distance2_6c; }             // 009EF73A
        }
        --rounds;
    }
    if (lo > hi) {                                                   // 009EF7DB
        if (d1 > d2) hi = static_cast<float>(lo + kTrafficWindowNudge);   // 009EF7FF
        else lo = static_cast<float>(hi - kTrafficWindowNudge);           // 009EF7EF
    }
    float nearest;
    if (result.side2 == 0) nearest = d1;
    else if (result.side1 == 0) nearest = d2;
    else nearest = (d1 > d2) ? d2 : d1;                              // 009EF84B
    clearance_33c = static_cast<float>(std::sqrt(static_cast<double>(nearest)));  // 009EF876
    const float error = wrapped_angle_subtract_00438b10(heading_target_324, heading);
    float clamped = error;
    if (lo > error) clamped = lo;                                    // 009EF8A1
    else if (error > hi) clamped = hi;                               // 009EF8B5
    const float before = heading_target_324;
    heading_target_324 = wrapped_angle_add_00438aa0(heading, clamped);   // 009EF8EE
    if (kTrafficHold > hold_354) hold_354 = kTrafficHold;            // 009EF8F4
    result.wrote = true;
    result.turn = wrapped_angle_subtract_00438b10(heading_target_324, before);
    return result;
}

} // namespace bsp
