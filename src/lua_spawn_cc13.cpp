// Packet cc13-spawn1. See include/bsp/lua_spawn_cc13.hpp for the routine contracts and
// docs/CC13_LUA_SPAWN.md for the evidence. Every name is a hypothesis, not a recovered symbol.
// Nothing here invents a global or stubs an unresolved call: the three world calls are
// SpawnPlacementHost methods the caller supplies.

#include "bsp/lua_spawn_cc13.hpp"

#include <array>
#include <cctype>
#include <cmath>
#include <cstring>

#include "bsp/gun_bot_remainder.hpp"   // segment_crossing_004f3730

namespace bsp {
namespace {

float to_float(double value) noexcept { return static_cast<float>(value); }

// 00419440 BSP_Vector3f_Length over a float3 already stored.
float length3(const float v[3]) noexcept {
    return to_float(std::sqrt(static_cast<double>(v[0]) * v[0] +
                              static_cast<double>(v[1]) * v[1] +
                              static_cast<double>(v[2]) * v[2]));
}

// The box test 009426D0 (00942894..009428D8) and 00942CB0 (00942F6F..00942FB7,
// 009431B9..009431FC) inline: strict comparisons on x and z only.
bool outside_box_xz(const SpawnWorldBox& box, float x, float z) noexcept {
    return x < box.min_x || x > box.max_x || z > box.max_z || z < box.min_z;
}

void copy3(float out[3], const float in[3]) noexcept {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

}  // namespace

SpawnNewExcludeRadius spawn_direct_exclude_block_00942cb0() noexcept {
    SpawnNewExcludeRadius block;
    block.present = true;
    block.own_horizontal = kSpawnOwnPartyRadius;     // 100^2 = 10000, 00CE3D64 at +14h
    block.enemy_horizontal = kSpawnOtherPartyRadius; // 2000^2 = 4000000, 00D19914 at +18h
    block.own_vertical = 100.0f;                     // 00CE3D08 at +8h
    block.enemy_vertical = 500.0f;                   // 00CE397C at +Ch
    return block;
}

SpawnRingPoints spawn_ring_points_00941fa0(const float origin[3], float near_distance,
                                           float far_distance, float height,
                                           float angle) noexcept {
    SpawnRingPoints out;
    out.angle = angle;
    // 00941FCD FSIN / 00941FD7 FCOS of the stored float, each stored as a float; 00941FE1
    // FCHS before the store, so the x factor is -sin.
    const float minus_sin = -to_float(std::sin(static_cast<double>(angle)));
    const float cosine = to_float(std::cos(static_cast<double>(angle)));
    // 00942001 / 0094200F: the two products are stored before the adds.
    const float near_x = near_distance * minus_sin;
    const float near_z = cosine * near_distance;
    out.near_point[0] = origin[0] + near_x;
    out.near_point[1] = height;   // 0094207F overwrites the origin.y + 0.0 of 00942026
    out.near_point[2] = origin[2] + near_z;
    const float far_x = far_distance * minus_sin;
    const float far_z = cosine * far_distance;
    out.far_point[0] = origin[0] + far_x;
    out.far_point[1] = height;    // 00942084
    out.far_point[2] = origin[2] + far_z;
    return out;
}

SpawnRingPoints spawn_ring_draw_00941fa0(SpawnPlacementHost& host, const float origin[3],
                                         float near_distance, float far_distance,
                                         float height, float angle_low,
                                         float angle_high) {
    // 00941FB6 MOV ECX,1 / 00941FC0 CALL 00BD2F10: stream 1 in the image.
    const float angle = host.uniform_00bd2f10(kSpawnRingDrawKey, angle_low, angle_high);
    return spawn_ring_points_00941fa0(origin, near_distance, far_distance, height, angle);
}

bool spawn_ray_sphere_00941aa0(const float start[3], const float dir[3],
                               const float center[3], float radius, float& t0,
                               float& t1) noexcept {
    // cc13-fixture2 (fixture-found): every named value is an FSTP to float and the sums between
    // the stores stay on the x87 stack (double here, the 53-bit control word): the projection
    // sums x, y, then z (00941B18..00941B33) and the squared miss sums x, y, then z
    // (00941B9A..00941BB6).
    using D = double;
    const float d0 = center[0] - start[0], d1 = center[1] - start[1], d2 = center[2] - start[2];
    const float proj = to_float((D(dir[0]) * d0 + D(dir[1]) * d1) + D(dir[2]) * d2);
    const float dx = (start[0] + to_float(D(proj) * dir[0])) - center[0];
    const float dy = (start[1] + to_float(D(dir[1]) * proj)) - center[1];
    const float dz = (start[2] + to_float(D(proj) * dir[2])) - center[2];
    const float miss2 = to_float((D(dx) * dx + D(dy) * dy) + D(dz) * dz);
    const float radius2 = to_float(D(radius) * radius);
    // 00941BCC FCOMI miss2, radius2 / JB: a smaller miss intersects, and so does an unordered
    // (NaN) compare.
    if (miss2 >= radius2) return false;
    const float half = to_float(std::sqrt(static_cast<double>(radius2 - miss2)));
    t0 = proj - half;
    t1 = half + proj;
    return true;
}

void spawn_mask_clear_00941c20(const float start[3], const float dir[3], float length,
                               float center_copy[3], float radius,
                               unsigned char mask[kSpawnLineSamples]) noexcept {
    center_copy[1] = start[1];   // 00941C2A
    // DEVIATION: with near == far the image divides 0 by 0 below, 00BF7420 turns the NaN into
    // 80000000h and the memset writes far outside the mask. The reconstruction clears nothing.
    if (!(length > 0.0f)) return;
    float t0 = 0.0f;
    float t1 = 0.0f;
    if (!spawn_ray_sphere_00941aa0(start, dir, center_copy, radius, t0, t1)) return;
    if (0.0f > t0) t0 = 0.0f;        // 00941C51
    if (t0 > length) t0 = length;    // 00941C6C
    if (0.0f > t1) t1 = 0.0f;        // 00941C76
    if (t1 > length) t1 = length;    // 00941C87
    // 00941C91 FDIVR, 00941C98 FMUL 255.0, stored, 00BF85B0 floor, stored, 00BF7420 ftol.
    const float lo_scaled = to_float(static_cast<double>(t0) / length * kSpawnMaskScale);
    const int lo = static_cast<int>(to_float(std::floor(static_cast<double>(lo_scaled))));
    const float hi_scaled = to_float(static_cast<double>(t1) / length * kSpawnMaskScale);
    // 00941CE7..00941CFD: a truncating FISTP of the floored value.
    const int hi = static_cast<int>(to_float(std::floor(static_cast<double>(hi_scaled))));
    if (hi < lo) return;
    std::memset(mask + lo, 0, static_cast<std::size_t>(hi - lo + 1));   // 00941D14
}

SpawnLineResult spawn_line_search_009426d0(const float start[3], const float end[3],
                                           std::int32_t party, const SpawnWorldBox& box,
                                           const std::vector<SpawnPlacementEntity>& entities,
                                           SpawnPlacementHost& host) {
    SpawnLineResult out;
    unsigned char mask[kSpawnLineSamples];
    std::memset(mask, 1, sizeof(mask));   // 009426FC

    const float delta[3] = {end[0] - start[0], end[1] - start[1], end[2] - start[2]};
    const float raw_length = length3(delta);                 // 0094275A 00419440
    const float inverse = raw_length > 0.0f ? to_float(1.0 / raw_length) : 0.0f;
    const float dir[3] = {inverse * delta[0], delta[1] * inverse, inverse * delta[2]};
    // 009427D8..009427F4: the squared length again, then the guarded 00BF7030.
    const float squared = to_float(static_cast<double>(delta[0]) * delta[0] +
                                   static_cast<double>(delta[1]) * delta[1] +
                                   static_cast<double>(delta[2]) * delta[2]);
    const float length = static_cast<double>(squared) > kSpawnLineLengthEpsilon
                             ? to_float(std::sqrt(static_cast<double>(squared)))
                             : 0.0f;
    const float step = to_float(length * kSpawnLineSampleStep);   // 0094282D
    const float sample_step[3] = {step * dir[0], dir[1] * step, dir[2] * step};

    float sample[3] = {start[0], start[1], start[2]};
    for (int i = 0; i < kSpawnLineSamples; ++i) {
        if (outside_box_xz(box, sample[0], sample[2])) {
            mask[i] = 0;   // 009429C0
            ++out.blocked_by_box;
        } else if (kSpawnShallowStartBelow > start[1]) {
            // 009428DE COMISS 10, start.y: the terrain walk runs only for a start below 10.
            const double stretch = step;   // 009428F3 FST double
            float t = 0.0f;
            if (stretch > t) {
                for (;;) {
                    const float point[3] = {dir[0] * t + sample[0], dir[1] * t + sample[1],
                                            t * dir[2] + sample[2]};
                    float ground = 0.0f;
                    if (host.ground_height_00903860(point, ground) &&
                        ground > kSpawnGroundBlocksAbove) {
                        mask[i] = 0;   // 00942A51
                        ++out.blocked_by_ground;
                        break;
                    }
                    if (mask[i] == 0) break;   // 00942981
                    t = to_float(t + kSpawnTerrainWalkStep);
                    if (!(stretch > t)) break;   // 009429A5 FCOMIP / JBE
                }
            }
        }
        sample[0] += sample_step[0];   // 009429CA..009429F3
        sample[1] += sample_step[1];
        sample[2] += sample_step[2];
    }

    // 009429FD..00942BCA: [[GGame+19CCh]+58h], each entity's world translation +FCh..+104h.
    for (const SpawnPlacementEntity& entity : entities) {
        const float dy = start[1] - entity.position[1];
        float radius = 0.0f;
        if (entity.party == party) {   // 00942B1A CMP [ESI+54h],EAX
            if (std::fabs(dy) > kSpawnOwnPartyVertical) continue;
            radius = kSpawnOwnPartyRadius;
        } else {
            if (std::fabs(dy) > kSpawnOtherPartyVertical) continue;
            radius = kSpawnOtherPartyRadius;
        }
        float center[3] = {entity.position[0], entity.position[1], entity.position[2]};
        spawn_mask_clear_00941c20(start, dir, length, center, radius, mask);
    }

    int index = 0;
    while (index < kSpawnLineSamples && mask[index] == 0) ++index;   // 00942BD0..00942C02
    out.index = index;
    out.ok = index < kSpawnLineSamples;   // 00942C17..00942C25
    // 00942C28..00942C9E: out = start + (i/256) * (L * dir).
    const float fraction = to_float(index * kSpawnLineSampleStep);
    const float span[3] = {dir[0] * length, dir[1] * length, dir[2] * length};
    out.position[0] = start[0] + fraction * span[0];
    out.position[1] = fraction * span[1] + start[1];
    out.position[2] = start[2] + fraction * span[2];
    return out;
}

SpawnEdgeCrossing spawn_edge_crossing_004bc120(const SpawnWorldBox& box, const float a[3],
                                               const float b[3]) noexcept {
    SpawnEdgeCrossing out;
    const std::array<float, 2> a0 = {a[0], a[2]};
    const std::array<float, 2> a1 = {b[0], b[2]};
    struct Edge {
        std::array<float, 2> b0;
        std::array<float, 2> b1;
        float edge_a[3];
        float edge_b[3];
    };
    // The four 004F3730 calls (004BC1EE, 004BC2CA, and through 004B6720 004BC34E,
    // 004BC3B5) with the endpoints each success branch stores.
    const Edge edges[4] = {
        {{box.min_x, box.max_z}, {box.max_x, box.max_z},
         {box.min_x, box.min_y, box.max_z}, {box.max_x, box.min_y, box.max_z}},
        {{box.max_x, box.max_z}, {box.max_x, box.min_z},
         {box.max_x, box.min_y, box.max_z}, {box.max_x, box.max_y, box.min_z}},
        {{box.max_x, box.min_z}, {box.min_x, box.min_z},
         {box.max_x, box.max_y, box.min_z}, {box.min_x, box.min_y, box.min_z}},
        {{box.min_x, box.min_z}, {box.min_x, box.max_z},
         {box.min_x, box.min_y, box.min_z}, {box.min_x, box.min_y, box.max_z}},
    };
    for (int i = 0; i < 4; ++i) {
        const SegmentCrossingXZ hit = segment_crossing_004f3730(a0, a1, edges[i].b0, edges[i].b1);
        if (!hit.crossed) continue;
        out.crossed = true;
        out.cross_written = true;
        out.edge = i;
        out.cross[0] = hit.point[0];
        out.cross[1] = 0.0f;   // 004BC21B / 004BC2E9 / 004B6720
        out.cross[2] = hit.point[1];
        copy3(out.edge_a, edges[i].edge_a);
        copy3(out.edge_b, edges[i].edge_b);
        return out;
    }
    // SUBSTITUTION: with no crossing the image's cross is 004B6720's unwritten locals.
    return out;
}

bool spawn_point_legal_00941d30(const float point[3], std::int32_t party, bool aircraft,
                                const std::vector<SpawnPlacementEntity>& entities,
                                SpawnPlacementHost& host) {
    const bool outside = host.point_outside_map_0071c4f0(point);   // 00941D41
    if (outside) return false;
    if (!aircraft) {
        // 00941D51..00941E3x: class 0, so the probe arm (00903860 at 00941E22; the
        // class arm 0082ADA0 / 0041B680 is never taken from 00942CB0). The height local is seeded once
        // with -1000 (00D7A240) and only read when 00903860 answers true.
        for (const auto& probe : kSpawnTerrainProbes) {
            const float at[3] = {probe[0] + point[0], probe[1] + point[1], probe[2] + point[2]};
            float ground = kSpawnProbeGroundDefault;
            if (host.ground_height_00903860(at, ground) && ground > kSpawnGroundBlocksAbove) {
                return false;
            }
        }
    }
    return spawn_member_placement_legal_00941d30(point, party, spawn_direct_exclude_block_00942cb0(),
                                                 entities, false);
}

SpawnSearchResult spawn_find_position_00944c44(const SpawnPlacementRequest& request,
                                               const SpawnWorldBox& box,
                                               const std::vector<SpawnPlacementEntity>& entities,
                                               SpawnPlacementHost& host) {
    SpawnSearchResult out;
    out.ring = spawn_ring_draw_00941fa0(host, request.origin, request.near_distance,
                                        request.far_distance, request.height,
                                        request.angle_low, request.angle_high);   // 00944C44
    SpawnLineResult line = spawn_line_search_009426d0(out.ring.near_point, out.ring.far_point,
                                                      request.party, box, entities, host);   // 00944C64
    copy3(out.position, line.position);
    if (line.ok) {
        out.ok = true;
        out.stage = 1;
        return out;
    }
    const SpawnEdgeCrossing edge = spawn_edge_crossing_004bc120(box, out.ring.near_point,
                                                                out.ring.far_point);   // 00944CC4
    if (!edge.crossed) return out;   // 00944CCE JZ 00944F20, nil
    line = spawn_line_search_009426d0(edge.cross, edge.edge_a, request.party, box, entities, host);   // 00944CEF
    copy3(out.position, line.position);
    if (line.ok) {
        out.ok = true;
        out.stage = 2;
        return out;
    }
    line = spawn_line_search_009426d0(edge.cross, edge.edge_b, request.party, box, entities, host);   // 00944D36
    copy3(out.position, line.position);
    if (line.ok) {
        out.ok = true;
        out.stage = 3;
    }
    return out;   // 00944D60 JZ 00944F20 when not ok
}

SpawnDirectResult spawn_place_directly_00942cb0(const SpawnPlacementRequest& request,
                                                const SpawnWorldBox& box,
                                                const std::vector<SpawnPlacementEntity>& entities,
                                                SpawnPlacementHost& host) {
    SpawnDirectResult out;
    const std::int32_t party = request.party;
    const bool aircraft = request.aircraft;
    SpawnRingPoints ring;
    float delta[3] = {0.0f, 0.0f, 0.0f};

    // 00942CE0..00942F69: eight bearings, each a fresh draw.
    for (int attempt = 0; attempt < kSpawnDirectAttempts; ++attempt) {
        out.attempts = attempt + 1;
        ring = spawn_ring_draw_00941fa0(host, request.origin, request.near_distance,
                                        request.far_distance, request.height,
                                        request.angle_low, request.angle_high);   // 00942D25
        delta[0] = ring.far_point[0] - ring.near_point[0];
        delta[1] = ring.far_point[1] - ring.near_point[1];
        delta[2] = ring.far_point[2] - ring.near_point[2];
        for (float t = 0.0f; 1.0f > t; t = to_float(t + kSpawnDirectSweepStep)) {
            const float point[3] = {delta[0] * t + ring.near_point[0],
                                    delta[1] * t + ring.near_point[1],
                                    t * delta[2] + ring.near_point[2]};
            if (spawn_point_legal_00941d30(point, party, aircraft, entities, host)) {   // 00942E36
                out.stage = 1;
                copy3(out.position, point);
                return out;
            }
        }
        if (!aircraft) continue;   // 00942E5F TEST BL,BL
        for (float t = 0.0f; 1.0f > t; t = to_float(t + kSpawnDirectSweepStep)) {
            const float lifted_y = delta[1] * t + ring.near_point[1];
            const float point[3] = {delta[0] * t + ring.near_point[0],
                                    to_float(lifted_y + kSpawnDirectAircraftLift),   // 00942F2E
                                    t * delta[2] + ring.near_point[2]};
            if (spawn_point_legal_00941d30(point, party, aircraft, entities, host)) {   // 00942F38
                out.stage = 2;
                copy3(out.position, point);
                return out;
            }
        }
    }

    // 00942F6F..00943161: the target the border cut is taken toward.
    float target[3];
    if (outside_box_xz(box, ring.near_point[0], ring.near_point[2])) {
        copy3(target, request.origin);   // 00943094
    } else {
        const float length = length3(delta);   // 00942FF9
        const float inverse = length > 0.0f ? to_float(1.0 / length) : 0.0f;
        const float unit[3] = {inverse * delta[0], delta[1] * inverse, inverse * delta[2]};
        for (int i = 0; i < 3; ++i) {
            target[i] = to_float(unit[i] * kSpawnDirectFallbackReach) + ring.near_point[i];
        }
    }
    SpawnEdgeCrossing cut = spawn_edge_crossing_004bc120(box, ring.near_point, target);   // 00943191
    const float half_width = box.max_x;   // 009431A1: [GGame+7128h] alone
    if (!cut.crossed && outside_box_xz(box, request.origin[0], request.origin[2]) &&
        host.point_outside_map_0071c4f0(ring.near_point)) {   // 0094320F
        const float high = to_float(static_cast<double>(half_width) - 1.0);
        const float low = to_float(1.0 - static_cast<double>(half_width));
        auto clamp = [low, high](float v) {
            if (low > v) return low;    // 0094324D / 00943298
            if (v > high) return high;  // 00943261 / 009432B0
            return v;
        };
        target[0] = clamp(target[0]);
        target[2] = clamp(target[2]);
        cut = spawn_edge_crossing_004bc120(box, ring.near_point, target);   // 009432F1, unchecked
    }
    if (!cut.cross_written) {
        // SUBSTITUTION: the image walks on from 004B6720's unwritten locals.
        out.undefined_border = true;
        out.stage = 5;
        out.position[1] = kSpawnGiveUpHeight;
        return out;
    }

    // 009432FA..0094343A: the border point at the ring height, snapped onto its edge, and the
    // two walkers' first steps.
    float border[3] = {cut.cross[0], request.height, cut.cross[2]};   // 00943313
    const double w = half_width;
    float steps[2][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
    const float step = kSpawnBorderWalkStep;
    if (!(static_cast<double>(border[0]) + 1.0 < w)) {            // 0094331D
        border[0] = to_float(w - 1.0);
        steps[0][2] = -step;
        steps[1][2] = step;
    } else if (!(static_cast<double>(border[2]) + 1.0 < w)) {     // 00943365
        border[2] = to_float(w - 1.0);
        steps[0][0] = -step;
        steps[1][0] = step;
    } else if (!(-w < static_cast<double>(border[0]) - 1.0)) {    // 009433BA
        border[0] = to_float(1.0 - w);
        steps[0][2] = step;
        steps[1][2] = -step;
    } else if (!(-w < static_cast<double>(border[2]) - 1.0)) {    // 009433FE
        border[2] = to_float(1.0 - w);
        steps[0][0] = step;
        steps[1][0] = -step;
    } else {
        // SUBSTITUTION: 00943438 keeps whatever the step slots held (the lifted sweep's last
        // point, or never-written stack); a crossing off the [-W, W] square lands here.
        out.undefined_border = true;
        out.stage = 5;
        out.position[1] = kSpawnGiveUpHeight;
        return out;
    }
    if (spawn_point_legal_00941d30(border, party, aircraft, entities, host)) {   // 009434AB
        out.stage = 3;
        copy3(out.position, border);
        return out;
    }

    // 009434E5..009437A4: two walkers with three corner turns each.
    float walker[2][3] = {{border[0], border[1], border[2]}, {border[0], border[1], border[2]}};
    int turns[2] = {kSpawnBorderWalkTurns, kSpawnBorderWalkTurns};
    const float lo = to_float(-w);
    while (turns[0] >= 0 || turns[1] >= 0) {   // 00943541..0094354D
        for (int i = 0; i < 2; ++i) {
            if (turns[i] < 0) continue;   // 0094355B
            float* p = walker[i];
            float* s = steps[i];
            p[0] += s[0];
            p[1] += s[1];
            p[2] += s[2];
            // 0094359E..009436F5: the multiplier is +1 or -1 by walker and edge.
            if (p[0] > half_width) {
                p[0] = half_width;
                const float m = i == 0 ? 1.0f : -1.0f;
                s[0] = 0.0f, s[1] = 0.0f, s[2] = to_float(kSpawnBorderTurnStep * m);
                --turns[i];
            } else if (p[2] > half_width) {
                p[2] = half_width;
                const float m = i == 0 ? -1.0f : 1.0f;
                s[0] = to_float(kSpawnBorderTurnStep * m), s[1] = 0.0f, s[2] = 0.0f;
                --turns[i];
            } else if (lo > p[0]) {
                p[0] = lo;
                const float m = i == 0 ? -1.0f : 1.0f;
                s[0] = 0.0f, s[1] = 0.0f, s[2] = to_float(kSpawnBorderTurnStep * m);
                --turns[i];
            } else if (lo > p[2]) {
                p[2] = lo;
                const float m = i == 0 ? 1.0f : -1.0f;
                s[0] = to_float(kSpawnBorderTurnStep * m), s[1] = 0.0f, s[2] = 0.0f;
                --turns[i];
            }
            if (spawn_point_legal_00941d30(p, party, aircraft, entities, host)) {   // 00943779
                out.stage = 4;
                out.walker = i;
                copy3(out.position, p);
                return out;
            }
        }
    }
    out.stage = 5;   // 009437A9: (0, 10, 0)
    out.position[1] = kSpawnGiveUpHeight;
    return out;
}

float spawn_heading_00944db8(const float origin[3], const float position[3]) noexcept {
    const float dz = position[2] - origin[2];   // 00944D98..00944DA0
    const float dx = position[0] - origin[0];   // 00944DA4..00944DAC
    const float bearing = to_float(std::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    return to_float(bearing + kSpawnQuarterTurn);   // 00944DC7
}

float spawn_relative_yaw_00944a2c(float row_z_x, float row_z_z) noexcept {
    const float bearing =
        to_float(std::atan2(static_cast<double>(row_z_z), static_cast<double>(row_z_x)));
    return to_float(bearing - kSpawnQuarterTurn);   // 00944A39
}

bool spawn_class_is_aircraft_00944ead(const std::string& class_name) noexcept {
    const char* literal = kSpawnAircraftClassName;
    const std::size_t size = std::strlen(literal);
    if (class_name.size() != size) return false;
    for (std::size_t i = 0; i < size; ++i) {
        if (std::tolower(static_cast<unsigned char>(class_name[i])) !=
            std::tolower(static_cast<unsigned char>(literal[i]))) {
            return false;
        }
    }
    return true;
}

SpawnResolvedPosition lua_spawn_resolve_position(const SpawnPlacementRequest& request,
                                                 const SpawnWorldBox& box,
                                                 const std::vector<SpawnPlacementEntity>& entities,
                                                 SpawnPlacementHost& host) {
    SpawnResolvedPosition out;
    out.direct = request.direct;
    if (request.direct) {   // 00944BF8 JNZ 00944EA5
        const SpawnDirectResult placed = spawn_place_directly_00942cb0(request, box, entities, host);
        out.ok = true;
        out.stage = placed.stage;
        copy3(out.position, placed.position);
    } else {
        const SpawnSearchResult found = spawn_find_position_00944c44(request, box, entities, host);
        if (!found.ok) return out;   // 00944F20
        out.ok = true;
        out.stage = found.stage;
        copy3(out.position, found.position);
    }
    out.heading = spawn_heading_00944db8(request.origin, out.position);
    return out;
}

}  // namespace bsp
