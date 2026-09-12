// The avoid-zone manager's geometry. Packet cc_ai_avoid_zones, worker
// agent/cc-ai-avoid-zones. Evidence, original ABI and uncertainty per routine:
// docs/AVOID_ZONE_GEOMETRY.md. Names are hypotheses, not recovered symbols.

#include "bsp/avoid_zone_geometry.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "bsp/gun_bot_remainder.hpp"  // segment_crossing_004f3730
#include "bsp/unit_rudder.hpp"        // wrapped_angle_subtract_00438b10

namespace bsp {
namespace {

// 00CE3830 and 00CE3828 as the listing loads them: doubles holding the
// float-rounded pi/2 and 2*pi. Every bearing rounds the atan2 result to float
// first (004225C4 FSTP float), subtracts in double and rounds again.
constexpr double kHalfPiDouble = 1.5707963705062866;
constexpr double kTwoPiDouble = 6.2831854820251465;

float length_of(float x, float y) noexcept {
    return static_cast<float>(std::sqrt(static_cast<double>(x) * static_cast<double>(x)
                                        + static_cast<double>(y) * static_cast<double>(y)));
}

// 00419260 BSP_Vector2f_ReciprocalLength returns 1/|v|, or zero for a zero
// vector (its FUCOMIP/JP reciprocal-or-zero selection). The reconstruction in
// src/native_vector2_math.cpp needs the CRT access block, so this file uses the
// plain rule and records the divergence in the doc.
std::array<float, 2> normalized(float x, float y) noexcept {
    const float len = length_of(x, y);
    if (len == 0.0f) return {{0.0f, 0.0f}};
    const float inverse = 1.0f / len;
    return {{inverse * x, inverse * y}};
}

// The bearing conversion without the one-unit gate: 0041A37C and 0041A3B3 call
// atan2 on a unit vector directly, where that gate would reject every input.
float bearing_of_vector(float x, float y) noexcept {
    const float raw = static_cast<float>(std::atan2(static_cast<double>(y),
                                                    static_cast<double>(x)));
    float bearing = static_cast<float>(kHalfPiDouble - static_cast<double>(raw));
    if (bearing < 0.0f) {
        bearing = static_cast<float>(static_cast<double>(bearing) + kTwoPiDouble);
    }
    return bearing;
}

std::int32_t next_corner(std::int32_t index, std::int32_t count) noexcept {
    // 00422558..00422576: p + 1 unless p is the last corner, then the first.
    if (count <= 0) return 0;
    return (index + 1 >= count) ? 0 : index + 1;
}

std::int32_t previous_corner(std::int32_t index, std::int32_t count) noexcept {
    // 00422A16..00422A2A: p - 1 unless p is the first corner, then the last.
    if (count <= 0) return 0;
    return (index <= 0) ? count - 1 : index - 1;
}

// 00417EF0 BSP_AvoidZoneManager_SegmentHitPoint, the two pull-back sites at
// 00422BAB and 0042315C. That routine belongs to packet cc_ai_path_follower;
// this is the two-step body it is (004120D0 then 004179D0), inlined here so the
// tangent walk does not need a host for its own manager.
void pull_back_to_zone_boundary(const AvoidZoneTable& table,
                                std::int32_t layer,
                                const std::array<float, 2>& toward,
                                std::array<float, 2> from,
                                std::array<float, 2>& point) noexcept {
    const std::int32_t group = avoid_zone_group_for_layer_004120d0(table, layer);
    if (group < 0) return;
    const AvoidZoneGroupHit hit =
        avoid_zone_group_segment_hit_004179d0(table.groups[static_cast<std::size_t>(group)],
                                              toward, from);
    if (hit.hit) point = hit.point;
}

} // namespace

// ---------------------------------------------------------------------------
// Bearings
// ---------------------------------------------------------------------------

float avoid_zone_bearing(const std::array<float, 2>& from,
                         const std::array<float, 2>& to) noexcept {
    const float dx = to[0] - from[0];
    const float dy = to[1] - from[1];
    // 004225B3 FLD1 / FCOMIP: strictly greater than one square unit.
    if (!(dx * dx + dy * dy > kAvoidZoneBearingGateSq)) return 0.0f;

    // 004225BF CALL 00BF701A with ST0 = dx and ST1 = dy, so atan2(dy, dx);
    // 004225C4 rounds it to float before the subtraction, and 004225DC then
    // 004225E0 apply one 2*pi correction, not a loop.
    return bearing_of_vector(dx, dy);
}

// ---------------------------------------------------------------------------
// 0085C910, the box-versus-segment reject
// ---------------------------------------------------------------------------

bool avoid_zone_box_meets_segment_0085c910(const std::array<float, 2>& box_min,
                                           const std::array<float, 2>& box_max,
                                           const std::array<float, 2>& b0,
                                           const std::array<float, 2>& b1) noexcept {
    const float half = kAvoidZoneBoxHalf;

    // 0085C91C..0085C93A: the box half extent, the segment half vector and the
    // centre difference, each scaled by 0.5.
    const float segment_x = (b1[0] - b0[0]) * half;
    const float extent_x = (box_max[0] - box_min[0]) * half;
    const float delta_x = ((b0[0] + b1[0]) - (box_max[0] + box_min[0])) * half;
    if (!(std::fabs(delta_x) <= std::fabs(segment_x) + extent_x)) return false;

    const float segment_y = (b1[1] - b0[1]) * half;
    const float extent_y = (box_max[1] - box_min[1]) * half;
    const float delta_y = ((b0[1] + b1[1]) - (box_max[1] + box_min[1])) * half;
    if (extent_y + std::fabs(segment_y) < std::fabs(delta_y)) return false;

    // 0085CA8A: the segment's own perpendicular axis.
    const float cross = std::fabs(segment_x * delta_y - segment_y * delta_x);
    return cross <= std::fabs(segment_x) * extent_y + extent_x * std::fabs(segment_y);
}

// ---------------------------------------------------------------------------
// 00416B50, point in zone
// ---------------------------------------------------------------------------

bool avoid_zone_contains_point_00416b50(const AvoidZonePolygon& zone,
                                        const std::array<float, 2>& point) noexcept {
    // 00416B58..00416B85: half open, min inclusive and max exclusive.
    if (!(zone.bounds_min[0] <= point[0] && point[0] < zone.bounds_max[0]
          && zone.bounds_min[1] <= point[1] && point[1] < zone.bounds_max[1])) {
        return false;
    }
    const std::size_t count = zone.corners.size();
    if (count == 0) return false;

    std::int32_t crossings = 0;
    std::array<float, 2> previous = zone.corners[count - 1].position;
    for (std::size_t i = 0; i < count; ++i) {
        const std::array<float, 2> current = zone.corners[i].position;
        // 00416BC0: the scanline straddles the edge, each end tested once.
        const bool upward = previous[1] <= point[1] && point[1] < current[1];
        const bool downward = current[1] < point[1] && point[1] <= previous[1];
        if (upward || downward) {
            float x = previous[0];
            if (current[1] - previous[1] != 0.0f) {
                x = previous[0]
                    + ((current[0] - previous[0]) * (point[1] - previous[1]))
                          / (current[1] - previous[1]);
            }
            if (x < point[0]) ++crossings;
        }
        previous = current;
    }
    // 00416CA6 AND 80000001h and the sign fixup: an odd crossing count.
    return (crossings & 1) == 1;
}

// ---------------------------------------------------------------------------
// 00416DD0 and 004179D0, the segment tests
// ---------------------------------------------------------------------------

bool avoid_zone_segment_hit_00416dd0(const AvoidZonePolygon& zone,
                                     const std::array<float, 2>& toward,
                                     std::array<float, 2>& running,
                                     std::int32_t& edge_index) noexcept {
    // 00416DE9: the AABB reject, through the adjustor thunk 004F2B00.
    if (!avoid_zone_box_meets_segment_0085c910(zone.bounds_min, zone.bounds_max,
                                               toward, running)) {
        return false;
    }
    const std::int32_t count = static_cast<std::int32_t>(zone.corners.size());
    if (count == 0) return false;

    bool hit = false;
    std::int32_t index = -1;  // 00416E30 MOV dword ptr [ESP+10h], -1
    std::array<float, 2> previous = zone.corners[static_cast<std::size_t>(count - 1)].position;
    for (std::int32_t i = 0; i < count; ++i) {
        const std::array<float, 2> current = zone.corners[static_cast<std::size_t>(i)].position;
        const float dx = toward[0] - running[0];
        const float dy = toward[1] - running[1];
        // 00416E92: stop once the running point has reached `toward`.
        if (dx * dx + dy * dy < kAvoidZoneSegmentEndEpsilonSq) break;

        // 00416EA5: ECX = toward, EDX = the running point, then the edge.
        const SegmentCrossingXZ crossing =
            segment_crossing_004f3730(toward, running, previous, current);
        if (crossing.crossed) {
            edge_index = index;
            running = crossing.point;
            hit = true;
        }
        ++index;
        previous = current;
    }
    // 00416F11: a hit on the wrap edge reports the last corner.
    if (hit && edge_index < 0) edge_index += count;
    return hit;
}

AvoidZoneGroupHit avoid_zone_group_segment_hit_004179d0(
    const AvoidZoneLayerGroup& group,
    const std::array<float, 2>& toward,
    const std::array<float, 2>& from) noexcept {
    AvoidZoneGroupHit result;
    // 004179D4..004179E6: the running point starts at *from, once, and is then
    // shared by every zone in the group.
    std::array<float, 2> running = from;
    for (std::size_t i = 0; i < group.zones.size(); ++i) {
        std::int32_t edge = result.edge_index;
        if (avoid_zone_segment_hit_00416dd0(group.zones[i], toward, running, edge)) {
            result.hit = true;
            result.zone_index = static_cast<std::int32_t>(i);  // 00417A1F
            result.edge_index = edge;
        }
    }
    result.point = running;
    return result;
}

// ---------------------------------------------------------------------------
// The table
// ---------------------------------------------------------------------------

std::int32_t avoid_zone_group_for_layer_004120d0(const AvoidZoneTable& table,
                                                 std::int32_t layer) noexcept {
    const std::int32_t count = static_cast<std::int32_t>(table.groups.size());
    if (count == 0) return -1;  // the native reads slot 0 regardless
    std::int32_t fallback = 0;
    for (std::int32_t i = 0; i < count; ++i) {
        const std::int32_t key = table.groups[static_cast<std::size_t>(i)].layer_key;
        if (key == layer) return i;
        if (key < layer) fallback = i;
    }
    return fallback;
}

std::int32_t avoid_zone_group_for_layer_00412120(const AvoidZoneTable& table,
                                                 std::int32_t layer) noexcept {
    const std::int32_t count = static_cast<std::int32_t>(table.groups.size());
    if (count < 1) return -1;  // 00412128 returns slot 0 without checking
    std::int32_t found = 0;
    for (std::int32_t i = 0; i < count; ++i) {
        const std::int32_t key = table.groups[static_cast<std::size_t>(i)].layer_key;
        if (key <= layer) {
            found = i;
            if (key == layer) break;
        }
    }
    return found;
}

std::int32_t avoid_zone_first_containing_004178f0(const AvoidZoneLayerGroup& group,
                                                  const std::array<float, 2>& point) noexcept {
    for (std::size_t i = 0; i < group.zones.size(); ++i) {
        if (avoid_zone_contains_point_00416b50(group.zones[i], point)) {
            return static_cast<std::int32_t>(i);
        }
    }
    return -1;
}

std::int32_t avoid_zone_group_find_or_create_00417ca0(AvoidZoneTable& table,
                                                      std::int32_t layer) noexcept {
    const std::int32_t count = static_cast<std::int32_t>(table.groups.size());
    for (std::int32_t i = 0; i < count; ++i) {
        if (table.groups[static_cast<std::size_t>(i)].layer_key == layer) return i;
    }
    // 00417CC7 operator new(14h), 00417CD5 the vtable, 00417CE4 key = -1, then
    // 00417CFA overwrites it with the requested key.
    AvoidZoneLayerGroup created;
    created.layer_key = layer;

    // 00417D05..00417D1B: the first slot whose key is above the new one.
    std::int32_t position = 0;
    while (position < count && !(layer < table.groups[static_cast<std::size_t>(position)].layer_key)) {
        ++position;
    }
    table.groups.insert(table.groups.begin() + position, created);
    return position;
}

void avoid_zone_table_clear_0041f600(AvoidZoneTable& table) noexcept {
    // 0041F603 count = 0 and the 14h-slot destructor loop. The tail call
    // 0041F62F 0041E960 is not projected.
    table.groups.clear();
}

// ---------------------------------------------------------------------------
// 00422500, the tangent walk
// ---------------------------------------------------------------------------

std::array<float, 2> avoid_zone_corner_offset_direction(const AvoidZonePolygon& zone,
                                                        std::int32_t corner,
                                                        bool forward) noexcept {
    const std::int32_t count = static_cast<std::int32_t>(zone.corners.size());
    if (count == 0) return {{0.0f, 0.0f}};

    const std::array<float, 2> here = zone.corners[static_cast<std::size_t>(corner)].position;
    const std::array<float, 2> after =
        zone.corners[static_cast<std::size_t>(next_corner(corner, count))].position;
    const std::array<float, 2> before =
        zone.corners[static_cast<std::size_t>(previous_corner(corner, count))].position;

    // 004229D6 / 00422F56: the two unit vectors from the corner to each
    // neighbour. Their sum bisects the corner and points out of the polygon.
    const std::array<float, 2> to_after = normalized(here[0] - after[0], here[1] - after[1]);
    const std::array<float, 2> to_before = normalized(here[0] - before[0], here[1] - before[1]);
    const float sum_x = to_after[0] + to_before[0];
    const float sum_y = to_after[1] + to_before[1];

    // 00422A80 / 00423012: opposite edges leave no bisector, so take the
    // perpendicular of the vector to one neighbour instead. The two passes
    // pick different neighbours and opposite rotations.
    if (sum_x * sum_x + sum_y * sum_y <= kAvoidZoneBisectorDegenerateSq) {
        if (forward) {
            const float dx = before[0] - here[0];
            const float dy = before[1] - here[1];
            return normalized(dy, -dx);
        }
        const float dx = after[0] - here[0];
        const float dy = after[1] - here[1];
        return normalized(-dy, dx);
    }
    return normalized(sum_x, sum_y);
}

AvoidZoneTangentCorners avoid_zone_tangent_corners_00422500(
    const AvoidZoneTable& table,
    const AvoidZonePolygon& zone,
    const std::array<float, 2>& far_point,
    std::int32_t edge_index,
    std::int32_t near_corner_hint,
    std::int32_t far_side_hint,
    float margin) noexcept {
    AvoidZoneTangentCorners out;

    const std::int32_t count = static_cast<std::int32_t>(zone.corners.size());
    // The native walks an empty corner array off the end of the allocation;
    // 0041D1E0 never registers a zone with no corners, so this guard is a
    // projection choice, not native behaviour.
    if (count == 0 || edge_index < 0 || edge_index >= count) return out;

    // 00422515 COMISS: the margin floor.
    if (margin < kAvoidZoneMarginFloor) margin = kAvoidZoneMarginFloor;

    const std::int32_t start = edge_index;
    std::int32_t flag = 0;

    // -- the forward pass, 0042257A..00422BDA -------------------------------
    {
        const std::int32_t after_start = next_corner(start, count);
        const std::array<float, 2> hit_corner =
            zone.corners[static_cast<std::size_t>(start)].position;
        const float to_far = avoid_zone_bearing(hit_corner, far_point);
        const float along_edge = avoid_zone_bearing(
            hit_corner, zone.corners[static_cast<std::size_t>(after_start)].position);
        float angle = wrapped_angle_subtract_00438b10(to_far, along_edge);
        // 004226C1: 0.1 > angle leaves the flag set.
        bool crossed = angle < kAvoidZoneSideTolerance;

        std::int32_t stop = -1;
        if (near_corner_hint >= 0) {
            stop = near_corner_hint - start;
            if (stop < 0) stop += count;
        }

        if (far_side_hint >= 0) {
            std::int32_t steps = 0;
            std::int32_t here = after_start;
            while (true) {
                ++steps;
                if (stop > 0 && stop < steps) break;
                const std::int32_t after = next_corner(here, count);
                angle -= zone.corners[static_cast<std::size_t>(here)].turn_angle;

                if (angle < 0.0f) {
                    crossed = true;
                } else if (crossed) {
                    // 004227B6 and 00422848: the exact angle at this corner.
                    const std::array<float, 2> position =
                        zone.corners[static_cast<std::size_t>(here)].position;
                    const float edge_bearing = avoid_zone_bearing(
                        position, zone.corners[static_cast<std::size_t>(after)].position);
                    const float point_bearing = avoid_zone_bearing(position, far_point);
                    if (wrapped_angle_subtract_00438b10(edge_bearing, point_bearing) < 0.0f) {
                        const std::array<float, 2> direction =
                            avoid_zone_corner_offset_direction(zone, here, true);
                        const std::array<float, 2> near_point = {
                            {position[0] + direction[0], position[1] + direction[1]}};
                        out.forward_point = {{position[0] + margin * direction[0],
                                              position[1] + margin * direction[1]}};
                        // 00422B85: only a margin above ten units is pulled back.
                        if (margin > kAvoidZonePullbackMargin) {
                            pull_back_to_zone_boundary(table, zone.layer, near_point,
                                                       out.forward_point, out.forward_point);
                        }
                        std::int32_t index = start + steps;
                        if (index >= count) index -= count;
                        out.forward_index = index;
                        out.has_forward = true;
                        flag = 1;
                        break;
                    }
                }

                here = after;
                if (steps > count) break;
            }
        }
    }

    // -- the backward pass, 00422BDF..0042318D ------------------------------
    {
        const std::int32_t after_start = next_corner(start, count);
        const std::array<float, 2> pivot =
            zone.corners[static_cast<std::size_t>(after_start)].position;
        const float to_far = avoid_zone_bearing(pivot, far_point);
        const float along_edge = avoid_zone_bearing(
            pivot, zone.corners[static_cast<std::size_t>(start)].position);
        float angle = wrapped_angle_subtract_00438b10(to_far, along_edge);
        // 00422CC2: angle > -0.1 leaves the flag set.
        bool crossed = angle > -kAvoidZoneSideTolerance;

        std::int32_t stop = -1;
        if (near_corner_hint >= 0) {
            stop = start - near_corner_hint;
            if (stop < 0) stop += count;
        }

        if (far_side_hint <= 0) {
            std::int32_t steps = 0;
            std::int32_t here = start;
            while (true) {
                ++steps;
                if (stop > 0 && stop < steps) break;
                const std::int32_t before = previous_corner(here, count);
                angle += zone.corners[static_cast<std::size_t>(here)].turn_angle;

                if (angle > 0.0f) {
                    crossed = true;
                } else if (crossed) {
                    const std::array<float, 2> position =
                        zone.corners[static_cast<std::size_t>(here)].position;
                    const float edge_bearing = avoid_zone_bearing(
                        position, zone.corners[static_cast<std::size_t>(before)].position);
                    const float point_bearing = avoid_zone_bearing(position, far_point);
                    if (wrapped_angle_subtract_00438b10(edge_bearing, point_bearing) > 0.0f) {
                        const std::array<float, 2> direction =
                            avoid_zone_corner_offset_direction(zone, here, false);
                        const std::array<float, 2> near_point = {
                            {position[0] + direction[0], position[1] + direction[1]}};
                        out.backward_point = {{position[0] + margin * direction[0],
                                               position[1] + margin * direction[1]}};
                        if (margin > kAvoidZonePullbackMargin) {
                            pull_back_to_zone_boundary(table, zone.layer, near_point,
                                                       out.backward_point, out.backward_point);
                        }
                        std::int32_t index = start - steps;
                        if (index < 0) index += count;
                        out.backward_index = index;
                        out.has_backward = true;
                        // 00423186 SUB EAX, 1.
                        out.side_code = flag - 1;
                        return out;
                    }
                }

                here = before;
                if (steps > count) break;
            }
        }
    }

    out.side_code = flag;
    return out;
}

// ---------------------------------------------------------------------------
// The producer
// ---------------------------------------------------------------------------

void avoid_zone_rebuild_corner_data(AvoidZonePolygon& zone) noexcept {
    const std::int32_t count = static_cast<std::int32_t>(zone.corners.size());
    if (count == 0) return;

    for (int attempt = 0; attempt < 2; ++attempt) {
        float turn_sum = 0.0f;
        for (std::int32_t i = 0; i < count; ++i) {
            AvoidZoneCorner& corner = zone.corners[static_cast<std::size_t>(i)];
            const std::array<float, 2> before =
                zone.corners[static_cast<std::size_t>(previous_corner(i, count))].position;
            const std::array<float, 2> after =
                zone.corners[static_cast<std::size_t>(next_corner(i, count))].position;

            const std::array<float, 2> incoming =
                normalized(corner.position[0] - before[0], corner.position[1] - before[1]);
            const float out_x = after[0] - corner.position[0];
            const float out_y = after[1] - corner.position[1];

            // 0041A310: below 1e-10 the edge has no length and 0041A339 stores
            // zero; the native then divides by that zero. The projection keeps
            // the direction at zero instead, which is the one divergence here.
            const float length_squared = out_x * out_x + out_y * out_y;
            corner.edge_length = (static_cast<double>(length_squared) > kAvoidZoneEdgeLengthEpsilonSq)
                                     ? length_of(out_x, out_y)
                                     : 0.0f;
            const std::array<float, 2> outgoing =
                (corner.edge_length == 0.0f)
                    ? std::array<float, 2>{{0.0f, 0.0f}}
                    : std::array<float, 2>{{out_x / corner.edge_length, out_y / corner.edge_length}};
            corner.edge_direction = outgoing;

            // 0041A37C and 0041A3B3: the bearing of each unit vector, taken
            // without the one-unit gate the tangent walk uses, then
            // 0041A3F0 subtract(outgoing, incoming) into +1Ch.
            const float incoming_bearing = bearing_of_vector(incoming[0], incoming[1]);
            const float outgoing_bearing = bearing_of_vector(outgoing[0], outgoing[1]);
            corner.turn_angle =
                wrapped_angle_subtract_00438b10(outgoing_bearing, incoming_bearing);

            // 0041A3FC..0041A41A: the corner normal, the bisector rotated.
            corner.normal = normalized(incoming[1] + outgoing[1],
                                       -(incoming[0] + outgoing[0]));
            turn_sum += corner.turn_angle;
        }

        // 0041A47E: a polygon that turns the wrong way is reversed once and
        // measured again, with the reverse disabled on the second pass.
        if (!(turn_sum > kAvoidZoneWindingTurnLimit)) return;
        for (std::int32_t i = 0, j = count - 1; i < j; ++i, --j) {
            const AvoidZoneCorner swap = zone.corners[static_cast<std::size_t>(i)];
            zone.corners[static_cast<std::size_t>(i)] = zone.corners[static_cast<std::size_t>(j)];
            zone.corners[static_cast<std::size_t>(j)] = swap;
        }
    }
}

AvoidZonePolygon avoid_zone_from_path_points(const std::vector<std::array<float, 3>>& path_points,
                                             std::int32_t layer) {
    AvoidZonePolygon zone;
    zone.layer = layer;
    if (path_points.empty()) return zone;

    // 0041CD32: point 0 is always kept, and it seeds both bounds.
    const std::array<float, 2> first = {{path_points[0][0], path_points[0][2]}};
    AvoidZoneCorner corner;
    corner.position = first;
    zone.corners.push_back(corner);
    zone.bounds_min = first;
    zone.bounds_max = first;

    std::array<float, 2> last_kept = first;
    for (std::size_t i = 1; i < path_points.size(); ++i) {
        const std::array<float, 2> point = {{path_points[i][0], path_points[i][2]}};
        const float dx = point[0] - last_kept[0];
        const float dy = point[1] - last_kept[1];
        // 0041CEDB: strictly farther than five units from the last kept point.
        if (!(static_cast<double>(dx * dx + dy * dy) > kAvoidZoneMinCornerSpacingSq)) continue;

        AvoidZoneCorner added;
        added.position = point;
        zone.corners.push_back(added);
        // 0041CFB9 00415010 BSP_Geometry_IncludePoint on the zone's +14h box.
        if (point[0] < zone.bounds_min[0]) zone.bounds_min[0] = point[0];
        if (point[1] < zone.bounds_min[1]) zone.bounds_min[1] = point[1];
        if (point[0] > zone.bounds_max[0]) zone.bounds_max[0] = point[0];
        if (point[1] > zone.bounds_max[1]) zone.bounds_max[1] = point[1];
        last_kept = point;
    }

    // 0041D063: the per-corner pass, after the world-box step this projection
    // leaves out.
    avoid_zone_rebuild_corner_data(zone);
    return zone;
}

// Preserve the original CRT conversion contract of 00424DAE's format string.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
std::int32_t avoid_zone_layer_from_entity_name(const std::string& name) noexcept {
    // 00424D7A: the gate is a nine-character prefix compare.
    if (std::strncmp(name.c_str(), "AvoidZone", 9) != 0) return -1;
    // 00424D8E: the layer slot is zero before the format runs.
    std::int32_t layer = kAvoidZoneDefaultLayer;
    if (std::strncmp(name.c_str(), "AvoidZoneG", 10) == 0) {
        int parsed_layer = 0;
        int parsed_index = 0;
        // 00424DAE: _sscanf(name, "AvoidZoneG %*s %d #%03d", &layer, &index).
        // The index is read and never used; the skipped token is the word the
        // shipped scenes all write as "all".
        if (std::sscanf(name.c_str(), "AvoidZoneG %*s %d #%03d", &parsed_layer, &parsed_index) >= 1) {
            layer = parsed_layer;
        }
    }
    return layer;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

void avoid_zone_rebuild_from_scene_00424d00(AvoidZoneSceneHost& host, AvoidZoneTable& table) {
    // 00424D22 0041F600.
    avoid_zone_table_clear_0041f600(table);

    const std::int32_t entities = host.world_entity_count();  // 00424D32, the list head
    for (std::int32_t i = 0; i < entities; ++i) {
        const std::int32_t layer = avoid_zone_layer_from_entity_name(host.world_entity_name(i));
        if (layer < 0) continue;

        // 00424DBE: the group exists before the entity is examined, so a named
        // entity that fails the path gate still creates its layer.
        const std::int32_t group = avoid_zone_group_find_or_create_00417ca0(table, layer);

        // 0041D201 and 0041D206, inside 0041D1E0.
        if (!host.world_entity_is_zone_path(i)) continue;
        AvoidZonePolygon zone = avoid_zone_from_path_points(host.world_entity_path_points(i), layer);
        // 0041D24B: a zone with no corners is freed instead of registered.
        if (zone.corners.empty()) continue;
        table.groups[static_cast<std::size_t>(group)].zones.push_back(zone);
    }

    // 00424DDA 00417CA0(0).
    avoid_zone_group_find_or_create_00417ca0(table, kAvoidZoneDefaultLayer);
}

} // namespace bsp
