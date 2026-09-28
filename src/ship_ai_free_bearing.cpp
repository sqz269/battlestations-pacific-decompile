#include "bsp/ship_ai_free_bearing.hpp"

// Evidence: docs/SHIP_AI_OPEN_ITEMS.md section 14 (packet cc9_free_bearing_query).
// Target MSVC Win32. The arithmetic keeps the image's float stores where the
// listing stores; x87 intermediate precision between stores is not modelled.

#include <cmath>

#include "bsp/avoid_zone_arc.hpp"
#include "bsp/geometry_helpers.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"

namespace bsp {
namespace {

// 00CE38B8, the shortest query range and the shortest free leg.
constexpr float kFreeBearingMinRange = 10.0f;
// 00D7A264 = pi, the "no candidate" deviation both sides start from.
constexpr float kFreeBearingNoDeviation = 3.1415927410125732f;
// 00D1F6F8, 75 degrees: the first round's (monotone walks) acceptance.
constexpr float kFreeBearingFirstRoundLimit = 1.3089970350265503f;
// 00CEDCD0 (double) = pi/4: the second round's (non-monotone walks) acceptance.
constexpr double kFreeBearingSecondRoundLimit = 0.7853981852531433;
// 00CF87C0 (double) = 1.25: along-track distance per metre of lateral give.
constexpr double kFreeBearingLateralSlope = 1.25;
// 00D7A23C: the smallest net lateral slope that turns the bearing.
constexpr float kFreeBearingMinSlope = 0.001f;

float dist2(const std::array<float, 2>& a, const std::array<float, 2>& b) {
    const float dx = a[0] - b[0];
    const float dz = a[1] - b[1];
    return dx * dx + dz * dz;
}

// One corner candidate (009DC4CD..009DC5B1 and its three twins): walk the
// outline from the hit, pull the corner back one unit along the query
// direction, and if the leg to it crosses the outline again walk that crossing
// the other way round (always monotone). Answers false when the first walk
// does not (null segment).
bool corner_candidate(const AvoidZoneSelectedSegment* hit_segment,
                      const std::array<float, 2>& origin,
                      const std::array<float, 2>& direction, float range, bool forward,
                      bool monotone, ShipAiFreeBearingHost& host, AvoidZoneSelectedSegment* head,
                      std::array<float, 2>& corner) {
    std::array<float, 2> walked{};
    if (!avoid_zone_outline_walk_00416270(hit_segment, origin, range, forward, monotone, walked,
                                          host.crt())) {
        return false;
    }
    // 009DC4DA..009DC510: toward = corner - direction.
    std::array<float, 2> toward{walked[0] - direction[0], walked[1] - direction[1]};
    std::array<float, 2> crossing = walked;
    AvoidZoneSelectedSegment* again =
        avoid_zone_selected_segments_hit_004158e0(head, origin, toward, crossing);
    if (again != nullptr) {
        // 009DC519..009DC548: the second walk, the other way, monotone.
        std::array<float, 2> second{};
        avoid_zone_outline_walk_00416270(again, origin, range, !forward, true, second,
                                         host.crt());
        toward = second;
    }
    corner = toward;
    return true;
}

}  // namespace

bool avoid_zone_outline_walk_00416270(const AvoidZoneSelectedSegment* segment,
                                      const std::array<float, 2>& origin, float radius,
                                      bool forward, bool monotone, std::array<float, 2>& out,
                                      const CameraAxesCrtAccess& crt) {
    if (segment == nullptr) return false;  // 00416298 / 004164FB
    const float radius2 = radius * radius; // 00416273..00416286
    float previous = 0.0f;                 // [ESP+44h], 0 before the first segment
    for (const AvoidZoneSelectedSegment* s = segment;;) {
        // Forward: near = start, far = end; backward the other way round.
        const std::array<float, 2> near_end = forward ? s->start : s->end;
        const std::array<float, 2> far_end = forward ? s->end : s->start;
        const float far2 = dist2(far_end, origin);
        // The outline normal n = (e.z - s.z, -(e.x - s.x)), taken at the near end.
        const float nx = s->end[1] - s->start[1];
        const float nz = -(s->end[0] - s->start[0]);
        const float facing = (origin[0] - near_end[0]) * nx + (origin[1] - near_end[1]) * nz;
        if (facing < 0.0f) {  // 0041636A / 004165CD
            out = near_end;
            return true;
        }
        if (radius2 < far2) {  // 00416378 / 004165DF: the far end leaves the circle
            std::array<std::array<float, 2>, 2> points{{far_end, far_end}};
            const int count =
                avoid_zone_circle_segment_004f3ba0(origin, near_end, radius, far_end, points, crt);
            if (count >= 2) {
                // 004163EE..0041646A: the crossing nearer the far end.
                out = dist2(points[1], far_end) <= dist2(points[0], far_end) ? points[1]
                                                                            : points[0];
            } else if (count == 1) {
                out = points[0];
            } else {
                out = far_end;
            }
            return true;
        }
        const AvoidZoneSelectedSegment* next = forward ? s->next : s->previous;
        if (next == nullptr) {  // 004164CF: the chain ends at this far end
            out = far_end;
            return true;
        }
        if (!monotone && previous > far2) {  // 00416389 / 004165F0
            out = near_end;
            return true;
        }
        previous = far2;
        s = next;
    }
}

bool ship_ai_free_bearing_009dc2e0(bool enabled, ShipAiSectorFreeBearingQuery& query,
                                   std::int32_t layer_key, ShipAiFreeBearingHost& host,
                                   ShipAiFreeBearingOutcome* outcome) {
    ShipAiFreeBearingOutcome local;
    ShipAiFreeBearingOutcome& trace = outcome != nullptr ? *outcome : local;
    if (!enabled) return false;                               // 009DC2F4
    if (kFreeBearingMinRange > query.range) return false;     // 009DC313
    // 009DC319..009DC3B2: refresh the searcher for a square of half side
    // sqrt(max(width_a, width_b)^2 + range^2) about the origin.
    const float width = query.width_a > query.width_b ? query.width_a : query.width_b;
    const float half = static_cast<float>(
        std::sqrt(static_cast<double>(width * width + query.range * query.range)));
    ShipAiAvoidZoneQuery refresh;
    refresh.x = query.origin_x;
    refresh.z = query.origin_z;
    refresh.half_width = half;
    refresh.half_height = half;
    refresh.layer_key = layer_key;
    host.refresh_query_009d7050(refresh);
    trace.refreshed = true;
    AvoidZoneSelectedSegment* head = host.selected_head();
    if (head == nullptr) {                                    // 009DC3BD
        trace.list_empty = true;
        return false;
    }

    const std::array<float, 2> origin{query.origin_x, query.origin_z};
    std::array<float, 2> direction{query.direction_x, query.direction_z};
    const float range = query.range;
    bool answer = false;
    // 009DC3F6..009DC453: the leg along the query direction.
    std::array<float, 2> leg_end{range * direction[0] + origin[0],
                                 range * direction[1] + origin[1]};
    std::array<float, 2> clear_end = leg_end;  // [ESP+30h]: what the clearance pass measures to
    std::array<float, 2> hit = leg_end;
    AvoidZoneSelectedSegment* hit_segment =
        avoid_zone_selected_segments_hit_004158e0(head, origin, leg_end, hit);
    if (hit_segment != nullptr) {
        trace.ahead_hit = true;
        clear_end = {hit[0] - direction[0], hit[1] - direction[1]};  // 009DC468..009DC48B
        const float base = heading_angle_00414eb0(direction);          // 009DC48F
        // Side 1 walks the outline forward, side 2 backward.
        float dev1 = kFreeBearingNoDeviation, dev2 = kFreeBearingNoDeviation;
        float angle1 = 0.0f, angle2 = 0.0f;
        bool side1 = false;
        bool chosen = false;
        std::array<float, 2> corner{};
        auto deviation = [&](const std::array<float, 2>& c, float& angle) {
            angle = heading_angle_00414eb0({c[0] - origin[0], c[1] - origin[1]});
            const float d = wrapped_angle_subtract_00438b10(angle, base);
            return d < 0.0f ? -d : d;
        };
        // Round one: monotone walks, 75 degrees (009DC4AA..009DC6B5).
        if (corner_candidate(hit_segment, origin, direction, range, true, true, host, head,
                             corner)) {
            dev1 = deviation(corner, angle1);
            side1 = kFreeBearingFirstRoundLimit > dev1;
        }
        if (corner_candidate(hit_segment, origin, direction, range, false, true, host, head,
                             corner)) {
            dev2 = deviation(corner, angle2);
            if (kFreeBearingFirstRoundLimit > dev2) chosen = true;
        }
        if (!chosen && side1) chosen = true;                   // 009DC6BB
        if (!chosen) {
            // Round two: non-monotone walks, pi/4 (009DC6C6..009DC8D2).
            if (corner_candidate(hit_segment, origin, direction, range, true, false, host,
                                 head, corner)) {
                dev1 = deviation(corner, angle1);
                side1 = kFreeBearingSecondRoundLimit > static_cast<double>(dev1);
            }
            if (corner_candidate(hit_segment, origin, direction, range, false, false, host,
                                 head, corner)) {
                dev2 = deviation(corner, angle2);
                if (kFreeBearingSecondRoundLimit > static_cast<double>(dev2)) chosen = true;
            }
            if (!chosen && side1) chosen = true;
        }
        if (chosen) {
            // 009DC8D8..009DC9B1: the smaller deviation wins, side 1 on a tie.
            answer = true;
            float bearing = angle2;
            trace.corner_side = 2;
            if (side1 && dev1 <= dev2) {
                bearing = angle1;
                trace.corner_side = 1;
            }
            query.bearing = bearing;
            direction = heading_to_direction_006bc0c0(query.bearing);
            leg_end = {range * direction[0] + origin[0], range * direction[1] + origin[1]};
            clear_end = leg_end;
            std::array<float, 2> again = leg_end;
            if (avoid_zone_selected_segments_hit_004158e0(head, origin, leg_end, again) !=
                nullptr) {
                clear_end = {again[0] - direction[0], again[1] - direction[1]};
            }
        }
    }

    // 009DC9B5: the clearance pass, only when a width reaches 1.
    const float width_a = query.width_a;
    if (!(1.0f <= width_a) && !(1.0f <= query.width_b)) return answer;
    trace.lateral_run = true;
    const float leg = static_cast<float>(std::sqrt(static_cast<double>(
        dist2(origin, clear_end))));                                   // 009DC9F3, 00414C60
    if (kFreeBearingMinRange > leg) {                                  // 009DCA08
        trace.short_leg = true;
        return false;
    }
    // 009DCA17..009DCDDE: every selected segment, in the frame of the (possibly
    // turned) direction: along = d . r, lateral = (-d.z, d.x) . r.
    const float perp_x = -0.0f - direction[1];
    float give_neg = 0.0f;  // [ESP+7Ch], the largest (>= 0) shift toward width_b's side
    float give_pos = 0.0f;  // [ESP+40h], the smallest (<= 0) shift toward width_a's side
    for (const AvoidZoneSelectedSegment* s = head; s != nullptr;) {
        const float rsx = s->start[0] - origin[0], rsz = s->start[1] - origin[1];
        const float rex = s->end[0] - origin[0], rez = s->end[1] - origin[1];
        float lat_s = rsx * perp_x + rsz * direction[0];
        float along_s = direction[0] * rsx + direction[1] * rsz;
        float lat_e = rez * direction[0] + rex * perp_x;
        float along_e = rez * direction[1] + rex * direction[0];
        const float lat_max = lat_s <= lat_e ? lat_e : lat_s;
        const float lat_min = lat_s <= lat_e ? lat_s : lat_e;
        const float along_max = along_s <= along_e ? along_e : along_s;
        const float along_min = along_s <= along_e ? along_s : along_e;
        const bool outside = along_max <= 0.0f || leg <= along_min ||
                             lat_max <= -query.width_b || width_a <= lat_min;
        if (!outside) {
            // Clip to 0 <= along <= leg (009DCBC0..009DCC72).
            if (0.0f > along_s) {
                lat_s = (lat_e - lat_s) * (-along_s / (along_e - along_s)) + lat_s;
                along_s = 0.0f;
            } else if (0.0f > along_e) {
                lat_e = (lat_s - lat_e) * (-along_e / (along_s - along_e)) + lat_e;
                along_e = 0.0f;
            }
            if (along_s > leg) {
                lat_s = (lat_e - lat_s) * ((along_s - leg) / (along_s - along_e)) + lat_s;
                along_s = leg;
            } else if (along_e > leg) {
                lat_e = lat_e + ((along_e - leg) / (along_e - along_s)) * (lat_s - lat_e);
                along_e = leg;
            }
            const float mn = lat_s <= lat_e ? lat_s : lat_e;  // 00415510
            const float mx = lat_s <= lat_e ? lat_e : lat_s;  // 00415550
            if (!(0.0f >= mx) && !(-mn > mx)) {
                // 009DCCB4..009DCD3A: mostly on the positive side; the end point.
                const float reach = static_cast<float>(
                    static_cast<double>(along_e) / kFreeBearingLateralSlope);
                const float give = query.width_a < reach ? query.width_a : reach;
                const float shift = static_cast<float>(static_cast<double>(lat_e) - give);
                const float span = along_e > 1.0f ? along_e : 1.0f;
                const float slope = shift / span;
                if (give_pos > slope) give_pos = slope;
            } else {
                // 009DCD3E..009DCDB2: mostly on the negative side; the start point.
                const float reach = static_cast<float>(
                    static_cast<double>(along_s) / kFreeBearingLateralSlope);
                const float give = query.width_b < reach ? query.width_b : reach;
                const float shift = give + lat_s;
                const float span = along_s > 1.0f ? along_s : 1.0f;
                const float slope = shift / span;
                if (slope > give_neg) give_neg = slope;
            }
            // 009DCDB8..009DCDDA: a segment reaching past +-1 on the other side
            // is taken a second time before the walk moves on. The second pass
            // computes the same slopes, so it changes nothing and is not repeated.
        }
        // 009DCE73: the run's next edge unless the run closes, else the next run.
        s = (s->closes_run == 0 && s->next != nullptr) ? s->next : s->next_run;
    }
    const float net = give_neg + give_pos;  // 009DCDE8
    const float magnitude = net < 0.0f ? -net : net;
    if (magnitude > kFreeBearingMinSlope) {  // 009DCE10
        const float heading = heading_angle_00414eb0(direction);
        query.bearing = heading;
        const float turn = static_cast<float>(std::atan(static_cast<double>(net)));  // 00BF8490
        query.bearing = wrapped_angle_subtract_00438b10(heading, turn);
        answer = true;
        trace.lateral_turn = true;
    }
    return answer;
}

}  // namespace bsp
