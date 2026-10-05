#include "bsp/avoid_zone_draft_bodies.hpp"

#include <cstdlib>

#include "bsp/avoid_zone_polygon_partition.hpp"

namespace bsp {

std::uint32_t hull_class_bit_009394fb(int kind, bool class_byte_808) noexcept {
    switch (kind) {                                       // 009394FB ADD EAX,-7; CMP EAX,7
    case 0x07: return 0x40;                               // 0093951C
    case 0x08: return 0x2000;                             // 00939570
    case 0x09: return 0x20;                               // 00939515
    case 0x0A: return class_byte_808 ? 0x1000 : 0x800;    // 0093952A..00939549
    case 0x0B: return 0x400;                              // 00939523
    case 0x0C: return class_byte_808 ? 0x200 : 0x100;     // 0093954D..0093956C
    case 0x0D: return 0x10;                               // 0093950E
    case 0x0E: return 0x80;                               // 00939577
    default: return 0;                                    // 009394F7 XOR EDI,EDI
    }
}

std::vector<AvoidZoneDraftBody> avoid_zone_draft_bodies_00424ddf(const AvoidZoneTable& table,
    const AvoidZoneDraftGroupZones& zones, const AvoidZoneDraftDepthInputs& depths,
    const CameraAxesCrtAccess& crt, AvoidZoneDraftBuildCensus& census) {
    static const AvoidZoneDynHullMemory memory{
        nullptr, [](void*, std::size_t bytes) -> void* { return std::malloc(bytes); },
        [](void*, void* p) { std::free(p); }};
    std::vector<AvoidZoneDraftBody> bodies;
    // 00424DFB..00425420, then 00425459 per pair and 00423C50(layer, mask).
    for (const AvoidZoneDraftLayer& pair : avoid_zone_draft_layers_00424dfb(depths)) {
        ++census.pairs;
        const std::int32_t layer = avoid_zone_draft_key_00425459(pair);
        // 00423C7C..00423C88: only the exact group; a fallback group is left alone.
        const std::int32_t group = table.groups.empty()
            ? -1 : avoid_zone_draft_exact_group_00423c50(table, layer);
        if (group < 0) { ++census.groups_missing; continue; }
        const AvoidZoneClearanceGroupView view = zones(static_cast<std::size_t>(group));
        for (std::uint32_t z = 0; z < view.count; ++z) {
            ++census.zones;
            // 00423CA8..00423CF2: fewer than three corners skips the zone.
            const std::vector<AvoidZoneDraftPoint> points =
                avoid_zone_draft_points_00423cb6(*view.zones[z]);
            if (points.empty()) { ++census.zones_short; continue; }
            // 00423D08: 004F6F20 with [00CE3984].
            const AvoidZonePolygonPartition partition = avoid_zone_polygon_partition_004f6f20(
                points, avoid_zone_draft_partition_parameter(), crt);
            for (std::size_t p = 0; p < partition.pieces.size(); ++p) {
                ++census.pieces;
                AvoidZoneDraftHullInput hull_input;
                if (!avoid_zone_draft_extrude_00423d81(points, partition.pieces[p], hull_input)) {
                    ++census.pieces_short;
                    continue;
                }
                // 0042414E 00C5DF30, 0042415F 00C32D20 < 4 destroys it and goes on.
                auto handle = std::shared_ptr<AvoidZoneDynHullHandle>(new AvoidZoneDynHullHandle{},
                    [](AvoidZoneDynHullHandle* h) {
                        avoid_zone_dyn_hull_destroy_00c37450(*h, memory);
                        delete h;
                    });
                avoid_zone_dyn_hull_construct_00c5df30(*handle, hull_input.local_points.data(),
                    static_cast<std::uint32_t>(hull_input.local_points.size()), memory);
                if (avoid_zone_dyn_hull_vertex_count_00c32d20(*handle) < 4) {
                    ++census.hulls_rejected;
                    continue;
                }
                // 00424204..004243E5: the body at (meanX, 0, meanZ), identity rows.
                AvoidZoneDraftBody body;
                body.layer = layer;
                body.group = pair.class_mask;
                body.zone = z;
                body.piece = p;
                const float frame[12] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                         hull_input.center.x, 0.0f, hull_input.center.z};
                for (int k = 0; k < 12; ++k) body.frame[k] = frame[k];
                body.hull = std::move(handle);
                bodies.push_back(std::move(body));
                ++census.bodies;
            }
        }
    }
    return bodies;
}

}  // namespace bsp
