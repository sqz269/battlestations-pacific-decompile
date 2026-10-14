// cc13_avoid1: avoid-zone navigation leftovers of segment 1. See
// include/bsp/cc13_avoid1.hpp and docs/CC13_AVOID1.md. Names are hypotheses.
#include "bsp/cc13_avoid1.hpp"

#include "bsp/avoid_zone_arc.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/geometry_helpers.hpp"
#include "bsp/unit_rudder.hpp"

#include <cstring>
#include <limits>

namespace bsp {
namespace {
float float_bits(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

constexpr float kWhite = 1.0f;                    // 00D7A24C
constexpr double kScreenScaleX = 666.0;           // 00CE3848
constexpr double kScreenScaleY = 500.0;           // 00CE3840
constexpr float kArrowHeight = 3.0f;              // 00CE3854
constexpr float kMarkerRadius = 5.0f;             // 00CE3850
constexpr double kSquaredLengthCutoff = 1e-10;    // 00CE3820
constexpr float kNegativeZero = -0.0f;            // 00D7A208, 80000000h
constexpr double kMinusOne = -1.0;                // 00D7A250
constexpr double kNegativeSideScale = -1.2000000476837158;  // 00CE38C0, a widened -1.2f
constexpr float kRegistryHeightFloor = 10.0f;     // 00CE38B8
constexpr std::uint32_t kGroupVtable = 0x00CE3858;
constexpr std::uint32_t kManagerVtable = 0x00CE38F4;
constexpr std::uint32_t kRegistryVtable = 0x00CE38E4;
constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;

// Native allocator and free calls go through the caller's real services.
void free_record(const AvoidZoneAllocationAccess& a, void* p) noexcept {
    a.free_record_00bf65ac(a.context, p);
}
void free_array(const AvoidZoneAllocationAccess& a, void* p) noexcept {
    a.free_array_00bf6989(a.context, p);
}
}  // namespace

// ---------------------------------------------------------------------------
// Debug drawing
// ---------------------------------------------------------------------------

void cc13_avoid_draw_selected_segments_004166e0(AvoidZoneSelectedSegment* const& head,
    const float rgba[4], Cc13AvoidDebugDrawHost& host) {
    for (const AvoidZoneSelectedSegment* s = head; s != nullptr;) {  // 004166E6 / 00416766
        const float start[3] = {s->start[0], 0.0f, s->start[1]};      // [ESP+24h], ECX
        const float end[3] = {s->end[0], 0.0f, s->end[1]};            // [ESP+18h], EDX
        host.line_0085fb30(start, end, rgba[0], rgba[1], rgba[2], rgba[3]);  // 0041674D
        // 00416752..00416763: next_run when closes_run or no next.
        s = (s->closes_run != 0 || s->next == nullptr) ? s->next_run : s->next;
    }
}

void cc13_avoid_draw_zone_screen_00416840(const AvoidZoneBoundaryZoneView& zone,
    const CameraMatrix& matrix, Cc13AvoidDebugDrawHost& host) {
    const auto& vertices = *zone.vertices;
    // 00416851: the last vertex is read before the empty test at 004168AA.
    const auto& last = vertices.back();
    std::array<float, 3> projected{};
    transform_point_004142e0({last[0], 0.0f, last[1]}, matrix, projected);  // 0041687D
    // FLD float, FDIV double, FSTP float; y negates z first (FCHS).
    float previous_x = static_cast<float>(static_cast<double>(projected[0]) / kScreenScaleX);
    float previous_y = static_cast<float>(-static_cast<double>(projected[2]) / kScreenScaleY);
    for (const auto& vertex : vertices) {
        transform_point_004142e0({vertex[0], 0.0f, vertex[1]}, matrix, projected);  // 004168DA
        const float x = static_cast<float>(static_cast<double>(projected[0]) / kScreenScaleX);
        const float y = static_cast<float>(-static_cast<double>(projected[2]) / kScreenScaleY);
        const float white[4] = {kWhite, kWhite, kWhite, kWhite};  // 00416910..0041692E
        host.screen_line_00860030(white, previous_x, previous_y, x, y);  // 00416967
        previous_x = x;
        previous_y = y;
    }
}

void cc13_avoid_draw_zone_arrows_004169a0(const AvoidZoneBoundaryZoneView& zone,
    const float rgba[4], Cc13AvoidDebugDrawHost& host) {
    // Function-local statics with guard bits 00E1763C / 00E17650 in the image;
    // their values are the constant 1.0f and 0.
    static const float kRed[4] = {kWhite, 0.0f, 0.0f, kWhite};       // 00E1762C
    static const float kYellow[4] = {kWhite, kWhite, 0.0f, kWhite};  // 00E17640
    const auto& vertices = *zone.vertices;
    std::array<float, 2> previous = vertices.back();  // 004169B5: before the empty test
    bool first = true;                                // 004169C7 BL
    for (const auto& vertex : vertices) {
        const float from[3] = {previous[0], kArrowHeight, previous[1]};  // ECX
        const float to[3] = {vertex[0], kArrowHeight, vertex[1]};        // EDX
        host.arrow_0085fb70(from, to, rgba, rgba);                       // 00416A54
        if (first) {
            host.wire_sphere_00860be0(to, kYellow, kMarkerRadius, kRed, 1);  // 00416B18
            first = false;
        }
        previous = vertex;
    }
}

void cc13_avoid_draw_group_arrows_004178b0(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const float rgba[4], Cc13AvoidDebugDrawHost& host) {
    for (const auto& zone : zones) cc13_avoid_draw_zone_arrows_004169a0(zone, rgba, host);  // 004178D3
}

void cc13_avoid_draw_group_screen_00417990(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const CameraMatrix& matrix, Cc13AvoidDebugDrawHost& host) {
    for (const auto& zone : zones) cc13_avoid_draw_zone_screen_00416840(zone, matrix, host);  // 004179B3
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool cc13_avoid_zone_arc_hit_0041ac60(const AvoidZoneBoundaryZoneView& zone,
    const std::array<float, 2>& centre, float radius, float start_bearing, float sweep,
    const CameraAxesCrtAccess& crt) {
    // 0041AC86..0041ACB2: each grown bound stored as a float.
    const float minimum_x = zone.bounds[0] - radius;
    const float minimum_z = zone.bounds[1] - radius;
    const float maximum_x = zone.bounds[2] + radius;
    const float maximum_z = zone.bounds[3] + radius;
    if (!(centre[0] >= minimum_x)) return false;  // 0041ACC8 JC, unordered too
    if (!(maximum_x > centre[0])) return false;   // 0041ACD6 JBE
    if (!(centre[1] >= minimum_z)) return false;  // 0041ACEF
    if (!(maximum_z > centre[1])) return false;   // 0041ACFD
    const auto& vertices = *zone.vertices;
    if (vertices.size() < 1) return false;        // 0041AD06 CMP count, 1 / JC
    std::array<float, 2> previous = vertices.back();
    for (const auto& vertex : vertices) {
        std::array<std::array<float, 2>, 2> points{};
        // 0041AD82: ECX centre, EDX previous, stack radius, &vertex, &points.
        const int count =
            avoid_zone_circle_segment_004f3ba0(centre, previous, radius, vertex, points, crt);
        for (int i = 0; i < count; ++i) {
            const std::array<float, 2> delta{points[i][0] - centre[0], points[i][1] - centre[1]};
            // 0041ADAC..0041ADDF is 00414EB0 inlined: atan2, pi/2 - a, +2pi below 0.
            const float bearing = heading_angle_00414eb0(delta);
            const float difference = wrapped_angle_subtract_00438b10(bearing, start_bearing);
            if (sweep > 0.0f && difference > 0.0f && sweep > difference) return true;  // 0041AE1D
            if (0.0f > sweep && 0.0f > difference && difference > sweep) return true;  // 0041AE33
        }
        previous = vertex;  // 0041AE49..0041AE63
    }
    return false;
}

bool cc13_avoid_group_arc_hit_0041b510(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& centre, float radius, float start_bearing, float sweep,
    const CameraAxesCrtAccess& crt) {
    for (const auto& zone : zones) {
        if (cc13_avoid_zone_arc_hit_0041ac60(zone, centre, radius, start_bearing, sweep, crt)) {
            return true;  // 0041B554
        }
    }
    return false;
}

float cc13_avoid_zone_distance_0041b120(AvoidZoneBoundaryHost& host,
    const AvoidZoneBoundaryZoneView& zone, const std::array<float, 2>& point, float slack) {
    float best = (std::numeric_limits<float>::max)();  // 00D7A248
    const float minimum_x = zone.bounds[0] - slack;
    const float minimum_z = zone.bounds[1] - slack;
    const float maximum_x = zone.bounds[2] + slack;
    const float maximum_z = slack + zone.bounds[3];    // 0041B17B FADD
    if (!(point[0] >= minimum_x) || !(maximum_x > point[0]) || !(point[1] >= minimum_z) ||
        !(maximum_z > point[1])) {
        return (std::numeric_limits<float>::max)();    // 0041B1D6
    }
    if (host.contains_point_00416b50(zone.native_identity, point)) return 0.0f;  // 0041B1C1
    const auto& vertices = *zone.vertices;
    std::array<float, 2> previous = vertices.back();   // 0041B1EF: before the empty test
    for (const auto& vertex : vertices) {
        const float distance =
            host.segment_distance_00419ab0(previous, vertex, point);  // 0041B23B
        if (best > distance) best = distance;                         // 0041B24C / JBE
        previous = vertex;
    }
    return best;
}

float cc13_avoid_group_distance_0041b680(AvoidZoneBoundaryHost& host,
    const std::vector<AvoidZoneBoundaryZoneView>& zones, const std::array<float, 2>& point,
    float slack) {
    float best = (std::numeric_limits<float>::max)();  // 0041B681
    for (const auto& zone : zones) {
        const float distance = cc13_avoid_zone_distance_0041b120(host, zone, point, slack);
        if (best > distance) best = distance;          // 0041B6C4 / JBE
    }
    return best;
}

void cc13_avoid_zone_tangent_0041b290(AvoidZoneBoundaryHost& host, Cc13AvoidCrtHost& crt,
    const AvoidZoneBoundaryZoneView& zone, Cc13AvoidZoneOptionalVector& out,
    const std::array<float, 2>& point, float slack) {
    const float minimum_x = zone.bounds[0] - slack;
    const float minimum_z = zone.bounds[1] - slack;
    const float maximum_x = zone.bounds[2] + slack;
    const float maximum_z = slack + zone.bounds[3];  // 0041B2DD FADD
    if (!(point[0] >= minimum_x) || !(maximum_x > point[0]) || !(point[1] >= minimum_z) ||
        !(maximum_z > point[1])) {
        out.valid = 0;                               // 0041B45D: x and z untouched
        return;
    }
    const bool inside = host.contains_point_00416b50(zone.native_identity, point);  // 0041B334
    std::array<float, 2> closest{};
    std::array<float, 4> edge{};
    std::uint8_t endpoint = 0;  // the image writes it into the dead point-argument slot
    host.call_00416f30(zone.native_identity, point, closest, edge, endpoint, 0.0f);  // 0041B353
    float nx = point[0] - closest[0];                // 0041B358
    float nz = point[1] - closest[1];
    // 0041B36D..0041B37D: products and sum on the x87 stack, one float store.
    const float squared = static_cast<float>(static_cast<double>(nx) * nx +
                                             static_cast<double>(nz) * nz);
    float length = 0.0f;                             // 0041B3A4
    if (static_cast<double>(squared) > kSquaredLengthCutoff) {  // 0041B38F JBE
        length = static_cast<float>(crt.crt_sqrt_00bf7030(squared));  // 0041B391
    }
    nx = nx / length;                                // 0041B3BC: no zero guard
    nz = nz / length;                                // 0041B3C8
    if (inside) {                                    // 0041B3CE
        nx = static_cast<float>(nx * kMinusOne);
        nz = static_cast<float>(nz * kMinusOne);
        length = static_cast<float>(length * kMinusOne);
    }
    if (length > slack) {                            // 0041B402 JBE: unordered keeps going
        out.valid = 0;
        return;
    }
    const float remaining = slack - length;          // 0041B414
    out.valid = 1;                                   // 0041B41F
    out.z = kNegativeZero - nx * remaining;          // 0041B42F..0041B43D
    out.x = remaining * nz;                          // 0041B442..0041B44E
}

void cc13_avoid_merge_tangent_0041b700(Cc13AvoidZoneOptionalVector& accumulator,
    const Cc13AvoidZoneOptionalVector& addend) noexcept {
    // The NEG/SBB/TEST 00E17628 pairs are the safe-bool test of the valid byte.
    if (accumulator.valid != 0) {
        if (addend.valid == 0) return;               // 0041B721
        const float ax = accumulator.x;
        const float az = accumulator.z;
        float bx = addend.x;
        float bz = addend.z;
        const float dot = static_cast<float>(static_cast<double>(ax) * bx +
                                             static_cast<double>(az) * bz);  // 0041B75F
        if (dot <= 0.0f) {                           // 0041B76B JC skips on 0 < dot or NaN
            bx = static_cast<float>(bx * kMinusOne);
            bz = static_cast<float>(bz * kMinusOne);
        }
        accumulator.valid = 1;                       // 0041B795
        accumulator.x = bx + ax;
        accumulator.z = bz + az;
        return;
    }
    if (addend.valid == 0) return;                   // 0041B7BF
    accumulator.x = addend.x;                        // 0041B7C1..0041B7CE
    accumulator.z = addend.z;
    accumulator.valid = addend.valid;
}

std::array<float, 2>& cc13_avoid_corner_offset_point_00423520(const AvoidZoneNativeStorage& zone,
    std::array<float, 2>& out, std::int32_t index, float distance,
    AvoidZoneClearanceAccess& clearance, const AvoidZoneAllocationAccess& allocation,
    const CameraAxesCrtAccess& crt) {
    // 0042352B..00423530: CDQ/IDIV, the 00417610 lookup inlined.
    ShipAiPathLateralRecord* record = ship_ai_lateral_record_at_00417610(zone.corners, index);
    avoid_zone_ensure_corner_clearance_00423190(zone, *record, clearance, allocation, crt);  // 00423534
    const float cached = record->clearance_scale;                                // 00423539
    const float scale = cached > distance ? distance : cached;                  // 00423548 JBE
    const float offset_x = scale * record->offset_dir_x;                        // 0042356C
    const float offset_z = scale * record->offset_dir_z;                        // 00423573
    out[0] = offset_x + record->x;                                              // 0042357E
    out[1] = record->z + offset_z;                                              // 00423586
    return out;
}

void cc13_avoid_simplify_ring_0041ea30(Cc13AvoidZonePathNative* path,
    const float& tolerance_at_30, Cc13AvoidZoneSimplifyHost& host,
    const AvoidZoneAllocationAccess& allocation) {
    using Point = std::array<float, 3>;
    if (path == nullptr || path->first == nullptr) return;  // 0041EA3A / 0041EA45
    // 0041EA4B..0041EA5F: (last - first) / 0Ch, signed.
    const auto n = static_cast<std::int32_t>(path->last - path->first);
    if (n < 10) return;                                      // 0041EA68
    // 0041EA70..0041EA7F: (n + 1) * 4 saturated to FFFFFFFFh on overflow.
    const std::uint64_t wide = static_cast<std::uint64_t>(static_cast<std::uint32_t>(n + 1)) * 4u;
    const std::uint32_t bytes =
        (wide >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(wide);  // SETO/NEG/OR
    auto* index = static_cast<std::int32_t*>(
        allocation.allocate_array_00bf55be(allocation.context, bytes));  // 0041EA82
    for (std::int32_t i = 0; i <= n; ++i) index[i] = i;      // 0041EA94
    // The checked operator[]: index below the signed count, compared unsigned.
    auto at = [&](std::int32_t i) -> const Point& {
        if (path->first == nullptr ||
            static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>(path->last - path->first)) {
            host.invalid_parameter_00bf6713();
        }
        return path->first[i];
    };
    auto wrap = [n](std::int32_t value) { return value >= n ? 0 : value; };  // SETGE/SUB/AND
    bool changed = false;                                    // [ESP+13h]
    std::int32_t removed = 0;                                // [ESP+20h]
    bool done = true;                                        // byte in the path-argument slot
    do {
        std::int32_t anchor = index[0];                      // [ESP+1Ch]
        std::int32_t value = index[1];                       // EDI
        done = true;                                         // 0041EACC
        std::int32_t kept = 0;                               // [ESP+2Ch]
        std::int32_t position = 1;                           // EBX
        const float tolerance = host.interpolate_clamped_00419010(
            10.0f, 3.0f, 30.0f, tolerance_at_30,
            static_cast<float>(n - removed));                // 0041EB10: [00CE38B8], [00CE3854], [00CE38C8]
        const Point& a0 = at(anchor);
        float ax = a0[0];                                    // [ESP+54h]
        float az = a0[2];                                    // [ESP+58h]
        const Point& b0 = at(wrap(value));
        float bx = b0[0];                                    // [ESP+44h]
        float bz = b0[2];                                    // [ESP+48h]
        if (value < n) {                                     // 0041EBBF
            std::int32_t* cursor = index + 1;                // [ESP+30h]
            for (;;) {
                value = index[position + 1];                 // 0041EBE2
                ++position;
                const Point& p = at(wrap(value));
                const float px = p[0];                       // [ESP+4Ch]
                const float pz = p[2];                       // [ESP+50h]
                bx = px;
                bz = pz;
                const float dx = ax - px;                    // 0041EC51
                const float dz = az - pz;                    // 0041EC5D
                // 0041EC61..0041EC8B, three float stores: (u, v) ~ (-dz, dx).
                const float sum = dz + dx;
                const float u = dx - sum;
                const float v = sum + u;
                const float squared = static_cast<float>(static_cast<double>(u) * u +
                                                         static_cast<double>(v) * v);
                float length = 0.0f;                         // 0041ECCA
                if (static_cast<double>(squared) > kSquaredLengthCutoff) {
                    length = static_cast<float>(host.crt_sqrt_00bf7030(squared));  // 0041ECB3
                }
                const float nx = u / length;                 // 0041ECE4: no zero guard
                const float nz = v / length;                 // 0041ECEC
                bool exceeded = false;
                for (std::int32_t k = anchor + 1; k < value; ++k) {  // 0041ECDF / 0041ED99
                    const Point& q = at(k);
                    const float qx = q[0];
                    const float qz = q[2];
                    const float ex = qx - px;                // 0041ED47
                    const float ez = qz - pz;                // 0041ED53
                    float distance = static_cast<float>(static_cast<double>(ez) * nz +
                                                        static_cast<double>(ex) * nx);
                    if (0.0f > distance) {                   // 0041ED75 JBE
                        distance = static_cast<float>(distance * kNegativeSideScale);
                    }
                    if (distance > tolerance) {              // 0041ED8F JA
                        exceeded = true;
                        break;
                    }
                }
                if (!exceeded) {
                    if (value < n) continue;                 // 0041EBD2 / 0041EBD8
                } else if (position > kept + 1) {            // 0041EDAF
                    value = index[position - 1];             // 0041EDB5: back off one
                    --position;
                    const Point& b = at(wrap(value));
                    bx = b[0];
                    bz = b[2];
                }
                // 0041EE16: drop the positions between the kept point and EBX.
                const std::int32_t next_kept = kept + 1;
                if (position > next_kept) {
                    removed += position - kept - 1;          // 0041EE2B
                    done = false;
                    changed = true;
                    std::int32_t* out = cursor;
                    for (std::int32_t i = position; i <= n; ++i) *out++ = index[i];  // 0041EE45
                }
                ax = bx;                                     // 0041EE58..0041EE77
                az = bz;
                ++cursor;
                position = next_kept;
                kept = next_kept;
                anchor = value;
                if (!(value < n)) break;                     // 0041EE81
            }
        }
    } while (n - removed > 10 && !done);                     // 0041EE8D / 0041EE97
    if (changed) {                                           // 0041EE9D
        std::int32_t erased = 0;                             // [ESP+74h]
        std::int32_t j = 0;                                  // EBX
        if (n > 0) {
            do {
                if (j + erased < index[j]) {                 // 0041EEC7
                    // 0041EECC..0041EF24: checked vector::erase(begin() + j).
                    if (path->first > path->last) host.invalid_parameter_00bf6713();
                    Point* it = path->first + j;
                    if (it > path->last || it < path->first) host.invalid_parameter_00bf6713();
                    Point* const last = path->last;
                    for (Point* source = it + 1; source != last; ++source) {
                        Point& target = *(source - 1);       // FLD/FSTP float per word
                        target[0] = (*source)[0];
                        target[1] = (*source)[1];
                        target[2] = (*source)[2];
                    }
                    path->last = last - 1;
                    ++erased;
                } else {
                    ++j;                                     // 0041EF2D
                }
            } while (j + erased < n);
        }
    }
    allocation.free_array_00bf6989(allocation.context, index);  // 0041EF41
}

// ---------------------------------------------------------------------------
// Groups, manager and registry lifetime
// ---------------------------------------------------------------------------

void cc13_avoid_group_clear_0041d0c0(Cc13AvoidZoneNativeGroup& group,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    for (AvoidZoneNativeStorage** slot = group.zones; slot != group.zones + group.count; ++slot) {
        AvoidZoneNativeStorage* zone = *slot;
        if (zone == nullptr) continue;               // 0041D0DA
        // 0041D0DC..0041D101: 004167C0 inlined, count re-read every pass.
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(zone->corners.count); ++i) {
            free_record(allocation, zone->corners.records[i]);
        }
        zone->corners.count = 0;
        if (zone->corners.records != nullptr) {      // 0041D108
            free_array(allocation, zone->corners.records);
            zone->corners.records = nullptr;
        }
        free_record(allocation, zone);               // 0041D11A
        *slot = nullptr;
    }
    group.count = 0;                                 // 0041D13A
}

void cc13_avoid_group_remove_owner_0041d150(Cc13AvoidZoneNativeGroup& group, void* owner,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    if (owner == nullptr) {                          // 0041D155
        cc13_avoid_group_clear_0041d0c0(group, allocation);
        return;
    }
    AvoidZoneNativeStorage** slot = group.zones;
    while (slot != group.zones + group.count) {
        AvoidZoneNativeStorage* zone = *slot;
        if (zone->associated_entity != owner) {      // 0041D172: read before the null test
            ++slot;
            continue;
        }
        if (zone != nullptr) {                       // 0041D177
            avoid_zone_clear_records_004167c0(*zone, allocation);  // 0041D17D
            if (zone->corners.records != nullptr) {
                free_array(allocation, zone->corners.records);
                zone->corners.records = nullptr;
            }
            free_record(allocation, zone);
            *slot = nullptr;
        }
        *slot = group.zones[group.count - 1];        // 0041D1AC: the last slot fills the hole
        --group.count;
    }
}

void cc13_avoid_group_destroy_0041de70(Cc13AvoidZoneNativeGroup& group,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    group.vtable = kGroupVtable;                     // 0041DE8D
    cc13_avoid_group_clear_0041d0c0(group, allocation);
    if (group.zones != nullptr) {                    // 0041DEA3
        free_array(allocation, group.zones);
        group.zones = nullptr;
    }
}

Cc13AvoidZoneHullListNode* cc13_avoid_hull_list_buy_node_0041c0f0(
    Cc13AvoidZoneHullListNode* next, Cc13AvoidZoneHullListNode* previous,
    const AvoidZoneDynHullHandle& value, const AvoidZoneAllocationAccess& allocation,
    const AvoidZoneDynHullMemory& hull_memory) {
    auto* node = static_cast<Cc13AvoidZoneHullListNode*>(
        allocation.allocate_record_00bf681b(allocation.context, 0x10));  // 0041C113
    if (node != nullptr) node->next = next;          // 0041C129
    // 0041C130 / 0041C13C test node+4 and node+8 against null, true for any node.
    node->previous = previous;
    try {
        avoid_zone_dyn_hull_copy_00c40f50(node->value, value, hull_memory);  // 0041C151
    } catch (...) {
        free_record(allocation, node);               // Catch_All 0041C16B, then rethrow
        throw;
    }
    return node;
}

Cc13AvoidZoneManagerNative& cc13_avoid_manager_construct_00421500(
    Cc13AvoidZoneManagerNative& manager, Cc13AvoidZoneManagerHost& host) {
    manager.vtable = kManagerVtable;                 // 00421521
    manager.vector_first = nullptr;                  // 0042152B..00421531
    manager.vector_last = nullptr;
    manager.vector_end = nullptr;
    manager.list_head = host.buy_head_node_0041bf50(manager);  // 0042153E
    manager.list_size = 0;
    manager.group_count = 0;                         // 0042154B
    for (void*& slot : manager.group_slots) slot = nullptr;    // 0042154E..0042158C
    manager.critical_section = host.create_critical_section_00bd1860();  // 0042158F
    return manager;
}

void cc13_avoid_manager_destroy_0041f980(Cc13AvoidZoneManagerNative& manager,
    Cc13AvoidZoneManagerHost& host, const AvoidZoneAllocationAccess& allocation) {
    manager.vtable = kManagerVtable;                 // 0041F99F
    host.clear_groups_0041f600(manager);             // 0041F9AD
    host.destroy_owned_section_0041cc80(&manager.critical_section);  // 0041F9B5
    host.clear_hull_list_0041e200(manager);          // 0041F9C4
    free_record(allocation, manager.list_head);      // 0041F9CD: unconditional
    manager.list_head = nullptr;
    if (manager.vector_first != nullptr) free_record(allocation, manager.vector_first);  // 0041F9E2
    manager.vector_first = nullptr;                  // 0041F9EE..0041F9F4
    manager.vector_last = nullptr;
    manager.vector_end = nullptr;
    host.store_manager_singleton_00e17624(nullptr);  // 0041F9F7
    manager.vtable = kSingletonBaseVtable;           // 0041F9FE
}

void cc13_avoid_draft_scratch_destroy_004235a0(Cc13AvoidZoneDraftScratch& scratch,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    auto release = [&allocation](void* (&v)[3]) {
        if (v[0] != nullptr) free_record(allocation, v[0]);
        v[0] = nullptr;
        v[1] = nullptr;
        v[2] = nullptr;
    };
    release(scratch.vector58);                       // 004235A5..004235C1
    Cc13AvoidZoneRawVector& outer = scratch.vectors44;
    if (outer.first != nullptr) {                    // 004235C7
        // 004235D4 0041F7B0: each 10h-byte element releases its own buffer.
        auto* element = static_cast<Cc13AvoidZoneRawVector*>(outer.first);
        auto* const last = static_cast<Cc13AvoidZoneRawVector*>(outer.last);
        for (; element != last; ++element) {
            if (element->first != nullptr) free_record(allocation, element->first);
            element->first = nullptr;
            element->last = nullptr;
            element->end = nullptr;
        }
        free_record(allocation, outer.first);        // 004235DD
    }
    outer.first = nullptr;                           // 004235E5..004235EB
    outer.last = nullptr;
    outer.end = nullptr;
    release(scratch.vector30);                       // 004235EE
    release(scratch.vector20);                       // 00423608
    release(scratch.vector10);                       // 00423621
}

Cc13AvoidZoneRegistryNative& cc13_avoid_registry_construct_00424730(
    Cc13AvoidZoneRegistryNative& registry, Cc13AvoidZoneRegistryHost& host) {
    registry.vtable = kRegistryVtable;               // 0042475D
    registry.head = host.buy_head_node_0041bfd0(registry);  // 00424763
    registry.size = 0;                               // 0042476B
    const float angles[3] = {
        float_bits(0x3E32B8C3u),  // [00CE3990] 0.17453292, ten degrees
        float_bits(0x3EB2B8C3u),  // [00CE398C] twenty degrees
        float_bits(0x3F9C61ABu),  // [00CE3988] seventy degrees
    };
    for (const float angle : angles) {
        void* block = host.allocate_layer_00bf681b(0x28);  // 00424775 / 004247D1 / 0042482D
        Cc13TerrainGridLayerNative* layer = nullptr;       // 0042479D: null block, null layer
        if (block != nullptr) {
            try {
                layer = host.construct_layer_00424680(block, angle);  // 00424796
            } catch (...) {
                host.free_layer_00bf65ac(block);     // Unwind 00C5E673/7E/89
                throw;
            }
        }
        Cc13TerrainGridLayerListNode* head = registry.head;
        Cc13TerrainGridLayerListNode* node =
            host.buy_node_0041c1f0(registry, head, head->previous, layer);  // 004247B7
        host.increment_size_00420140(registry, 1);   // 004247C2
        head->previous = node;                       // 004247C7
        node->previous->next = node;                 // 004247CA..004247CF
    }
    return registry;
}

void cc13_avoid_registry_destroy_0041f0d0(Cc13AvoidZoneRegistryNative& registry,
    Cc13AvoidZoneRegistryHost& host) {
    registry.vtable = kRegistryVtable;               // 0041F0ED
    host.clear_layers_0041ded0(registry);            // 0041F0FB
    host.destroy_layer_list_0041d710(registry);      // 0041F103
    host.store_registry_singleton_00e17620(nullptr); // 0041F10C
    registry.vtable = kSingletonBaseVtable;          // 0041F116
}

void cc13_avoid_registry_end_load_0041e000(Cc13AvoidZoneRegistryNative& registry,
    Cc13AvoidZoneRegistryHost& host) {
    registry.max_cell_height = kRegistryHeightFloor;  // 0041E013
    if (registry.size == 0) return;                   // 0041E018
    Cc13TerrainGridLayerListNode* first = registry.head->next;
    if (first == registry.head) host.invalid_parameter_00bf6713();  // 0041E028
    const Cc13TerrainGridLayerNative* layer = first->value;
    const std::int32_t n = layer->dimension;          // 0041E032, read once for the bounds
    for (std::int32_t i = 0; i < n; ++i) {
        for (std::int32_t j = 0; j < n; ++j) {
            // 0041E048..0041E06D: 0041BAE0 inlined, dimension re-read.
            const std::int32_t last = layer->dimension - 1;
            const std::int32_t ci = i < 0 ? 0 : (i > last ? last : i);
            const std::int32_t cj = j < 0 ? 0 : (j > last ? last : j);
            const auto index = static_cast<std::uint32_t>(layer->dimension * cj + ci);
            if (layer->grid_first == nullptr ||
                static_cast<std::uint32_t>(layer->grid_last - layer->grid_first) <= index) {
                host.invalid_parameter_00bf6713();   // 0041E081
            }
            // FILD of the byte, FMUL float, FSTP float.
            const float height = static_cast<float>(
                static_cast<double>(layer->grid_first[index]) * layer->file_scalar);
            const float current = registry.max_cell_height;
            registry.max_cell_height = current > height ? current : height;  // 0041E0AF JBE
        }
    }
}

}  // namespace bsp
