// Packet cc9_neighbour_clips. Semantic reconstructions; see
// bsp/ship_ai_neighbour_clips.hpp and docs/SHIP_NEIGHBOUR_AVOIDANCE.md section 4.
#include "bsp/ship_ai_neighbour_clips.hpp"

#include "bsp/avoid_zone_arc.hpp"   // avoid_zone_circle_segment_004f3ba0
#include "bsp/native_scalar_float_by_ref.hpp" // max_native_float_by_ref_00415550
#include "bsp/unit_rudder.hpp"      // wrapped_angle_add_00438aa0 / _subtract_00438b10

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

} // namespace bsp
