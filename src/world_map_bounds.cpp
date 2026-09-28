#include "bsp/world_map_bounds.hpp"

#include "bsp/scene_property_bag.hpp"

#include "bsp/gamepad_force_events.hpp"

#include <cmath>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
float spill(float value) noexcept {
    float result;
    __asm { fld value
            fstp result }
    return result;
}
float half_size(float value, bool negative) noexcept {
    const double half = 0.5; // 00D7A280: 00 00 00 00 00 00 E0 3F.
    float result;
    __asm {
        fld value
        cmp negative, 0
        je positive
        fchs
    positive:
        fld half
        fmulp st(1), st(0)
        fstp result
    }
    return result;
}
bool greater(float left, float right) noexcept {
    unsigned char result;
    __asm {
        fld right
        fld left
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
const SceneProperty* property(const ScenePropertyBlock& bag, const char* key) {
    for (const auto& item : bag.values) {
        if (_stricmp(item.key.c_str(), key) == 0) return &item;
    }
    return nullptr;
}
const ScenePropertyBlock* sub_bag(const ScenePropertyBlock& bag, const char* key) {
    for (const auto& item : bag.blocks) {
        if (_stricmp(item.first.c_str(), key) == 0) return &item.second;
    }
    return nullptr;
}
ScenePropertyValue decode(const ScenePropertyBlock& bag, const char* key,
    ScenePropertyType expected, bool allow_integer = false) {
    const auto* item = property(bag, key);
    ScenePropertyValue value;
    ScenePropertyType type;
    SceneReferenceKind kind = SceneReferenceKind::Any;
    if (!item || !scene_property_type_for_letter(item->type_letter, type, kind) ||
        (type != expected && !(allow_integer && type == ScenePropertyType::Int)) ||
        !scene_decode_property_value(type, kind, item->values, value)) {
        throw std::invalid_argument(std::string("Invalid required map property: ") + key);
    }
    return value;
}
float border_size(const ScenePropertyBlock& bag, const char* key) {
    const auto value = decode(bag, key, ScenePropertyType::Float, true);
    if (value.type != ScenePropertyType::Int) return value.float_value;
    // 004E6C16 / 004E6C3A: CVTSI2SS, not an x87 integer conversion.
    const auto integer = value.int_value;
    float result;
    __asm { cvtsi2ss xmm0, integer
            movss result, xmm0 }
    return result;
}
} // namespace

std::array<float, 3> WorldMapBounds::clip_minimum() const noexcept {
    return {north_west[0], north_west[1], south_east[2]};
}
std::array<float, 3> WorldMapBounds::clip_maximum() const noexcept {
    return {south_east[0], south_east[1], north_west[2]};
}

bool read_world_map_settings_004e6c00(const ScenePropertyBlock& map,
    WorldMapSettings& output) {
    const float x = border_size(map, "BorderSizeX");
    const float y = border_size(map, "BorderSizeY");
    const auto* multi = sub_bag(map, "MultiPlayMapSizes");
    if (!multi) return false; // 004E6C58..65, before any global data store.
    static const char* north_west[] = {"IslandCapture1v1_nw", "IslandCapture2v2_nw",
        "IslandCapture3v3_nw", "IslandCapture4v4_nw", "Duel_nw", "Escort_nw",
        "Siege_nw", "Competitive_nw"};
    static const char* south_east[] = {"IslandCapture1v1_se", "IslandCapture2v2_se",
        "IslandCapture3v3_se", "IslandCapture4v4_se", "Duel_se", "Escort_se",
        "Siege_se", "Competitive_se"};
    WorldMapSettings result;
    for (std::size_t i = 0; i < result.multiplayer.size(); ++i) {
        const auto nw = decode(*multi, north_west[i], ScenePropertyType::Vector3);
        const auto se = decode(*multi, south_east[i], ScenePropertyType::Vector3);
        std::memcpy(result.multiplayer[i].north_west.data(), nw.vector, sizeof(nw.vector));
        std::memcpy(result.multiplayer[i].south_east.data(), se.vector, sizeof(se.vector));
    }
    result.border_size_x = x;
    result.border_size_y = y;
    output = result;
    return true;
}

WorldMapBounds select_world_map_bounds_004d5ede(const WorldMapSettings& settings,
    std::int32_t mode, std::uint8_t forced, std::int32_t session) noexcept {
    if ((forced != 0 || session != 0 || mode == 9 || mode == 8) &&
        static_cast<std::uint32_t>(mode) <= 7) {
        const auto& source = settings.multiplayer[static_cast<std::size_t>(mode)];
        WorldMapBounds result;
        for (std::size_t i = 0; i < 3; ++i) result.north_west[i] = spill(source.north_west[i]);
        for (std::size_t i = 0; i < 3; ++i) result.south_east[i] = spill(source.south_east[i]);
        return result;
    }
    // Native default arm computes NW then SE, with binary32 stores between.
    WorldMapBounds result;
    result.north_west = {spill(half_size(settings.border_size_x, true)), 0.0f,
                        spill(half_size(settings.border_size_y, false))};
    result.south_east = {spill(half_size(settings.border_size_x, false)), 0.0f,
                        spill(half_size(settings.border_size_y, true))};
    return result;
}

bool point_outside_world_map_0071c4f0(const WorldMapBounds& box,
    const std::array<float, 3>& point) noexcept {
    const float x = spill(point[0]);
    if (greater(box.north_west[0], x) || greater(x, box.south_east[0])) return true;
    const float z = spill(point[2]);
    return greater(z, box.north_west[2]) || greater(box.south_east[2], z);
}
// Packet cc9_get_closest_border_zone. docs/WORLD_MAP_BOUNDS.md, "Border zones".
namespace {
constexpr float kBorderMargin = 50.0f;        // 00CE3938, double 50.0
constexpr float kBorderEndExtension = 1000.0f;  // 00CE47A0, double 1000.0
constexpr double kBorderDepth = 950.0;        // 00CE7640, double 950.0
constexpr float kBorderPositive = 1.0f;       // 00D7A24C
constexpr float kBorderNegative = -1.0f;      // 00D7A260
constexpr double kBorderThirds = 3.0;         // 00D7A2B0
constexpr double kBorderMatchEpsilon = 1e-10; // 00CE3820
constexpr float kBorderSeedDistance = 1e10f;  // 00CE4970

// 004B6C40: edges 0 and 2 run along x (SE.x - NW.x, +7128h - +711Ch), edges 1
// and 3 along z (NW.z - SE.z, +7124h - +7130h). Returned on the x87 stack.
double border_edge_length_004b6c40(const WorldMapBounds& bounds, int edge) noexcept {
    if (edge != 0 && edge != 2)
        return static_cast<double>(bounds.north_west[2]) - bounds.south_east[2];
    return static_cast<double>(bounds.south_east[0]) - bounds.north_west[0];
}
float to_float(double value) noexcept { return static_cast<float>(value); }
} // namespace

BorderZoneSet build_border_zones_004d5bd0(const WorldMapBounds& bounds) {
    BorderZoneSet zones;
    // 004D61C4..004D6240: three records per edge, +0 = 004B6C40(edge) / 3.0,
    // +4 = 2.
    for (int edge = 0; edge < 4; ++edge) {
        for (int i = 0; i < 3; ++i) {
            BorderZoneRecord record;
            record.length = to_float(border_edge_length_004b6c40(bounds, edge) / kBorderThirds);
            record.side = 2;
            zones.edges[edge].push_back(record);
        }
    }
    // 004C71C0: the layout.
    const float nw_x = bounds.north_west[0], nw_z = bounds.north_west[2];
    for (int edge = 0; edge < 4; ++edge) {
        auto& list = zones.edges[edge];
        const int last = static_cast<int>(list.size()) - 1;
        float offset = 0.0f;
        for (int i = 0; i <= last; ++i) {
            BorderZoneRecord& r = list[static_cast<std::size_t>(i)];
            r.edge = edge;                  // 004C7243
            r.index = i;                    // 004C7246
            r.offset = offset;              // 004C7249
            if (i == last)                  // 004C7253..004C7275
                r.length = to_float(border_edge_length_004b6c40(bounds, edge) - offset);
            const double len = r.length;
            if (edge == 0 || edge == 2) {
                // 004C72BC..004C7305, x along the edge.
                const double start = static_cast<double>(nw_x) + offset;
                r.a[0] = to_float(start + kBorderMargin);
                r.b[0] = r.a[0];
                r.c[0] = to_float(start + len - kBorderMargin);
                r.d[0] = r.c[0];
                if (i == 0) r.a[0] = to_float(static_cast<double>(r.a[0]) - kBorderEndExtension);
                if (i == last) r.d[0] = to_float(static_cast<double>(r.d[0]) + kBorderEndExtension);
                // 004C7333..004C7362, z outside the edge.
                const float z0 = edge == 0 ? bounds.north_west[2] : bounds.south_east[2];
                const float dir = edge == 0 ? kBorderPositive : kBorderNegative;
                r.a[2] = to_float(static_cast<double>(z0) + kBorderDepth * dir);
                r.b[2] = to_float(static_cast<double>(z0) + static_cast<double>(dir) * kBorderMargin);
                r.c[2] = r.b[2];
                r.d[2] = r.a[2];
            } else {
                // 004C736D.., x outside the edge.
                const float x0 = edge == 1 ? bounds.south_east[0] : nw_x;
                const float dir = edge == 1 ? kBorderPositive : kBorderNegative;
                r.a[0] = to_float(static_cast<double>(x0) + kBorderDepth * dir);
                r.b[0] = to_float(static_cast<double>(x0) + static_cast<double>(dir) * kBorderMargin);
                r.c[0] = r.b[0];
                r.d[0] = r.a[0];
                // z along the edge, from NW.z downward.
                const double start = static_cast<double>(nw_z) - offset;
                r.a[2] = to_float(start - kBorderMargin);
                r.b[2] = r.a[2];
                r.c[2] = to_float(start - len + kBorderMargin);
                r.d[2] = r.c[2];
                if (i == 0) r.a[2] = to_float(static_cast<double>(r.a[2]) + kBorderEndExtension);
                if (i == last) r.d[2] = to_float(static_cast<double>(r.d[2]) - kBorderEndExtension);
            }
            offset = to_float(static_cast<double>(offset) + len);
        }
    }
    // 004E7142..004E7209: 004C7150(edge, index, side), twelve constant calls.
    static constexpr std::int32_t kSides[4][3] = {{0, 1, 0}, {1, 0, 1}, {0, 1, 0}, {1, 0, 1}};
    for (int edge = 0; edge < 4; ++edge)
        for (int i = 0; i < 3; ++i)
            zones.edges[edge][static_cast<std::size_t>(i)].side = kSides[edge][i];
    return zones;
}

BorderZoneHit closest_border_zone_004c7730(const BorderZoneSet& zones,
    const std::array<float, 3>& position, std::int32_t side) {
    for (;;) {
        BorderZoneHit best;
        float best_distance = kBorderSeedDistance;
        for (int edge = 0; edge < 4; ++edge) {
            for (const BorderZoneRecord& r : zones.edges[edge]) {
                if (side >= 0 && r.side != side) continue;
                float x, z, dir_x, dir_z;
                if (edge == 0 || edge == 2) {
                    // +1Ch / +28h clamp x, +24h is z.
                    z = r.b[2];
                    const float lo = r.c[0] < r.b[0] ? r.c[0] : r.b[0];
                    const float hi = r.c[0] < r.b[0] ? r.b[0] : r.c[0];
                    x = position[0] < lo ? lo : (position[0] <= hi ? position[0] : hi);
                    dir_x = 0.0f;
                    dir_z = edge == 0 ? kBorderPositive : kBorderNegative;
                } else {
                    // +1Ch is x, +24h / +30h clamp z.
                    x = r.b[0];
                    const float lo = r.c[2] < r.b[2] ? r.c[2] : r.b[2];
                    const float hi = r.c[2] < r.b[2] ? r.b[2] : r.c[2];
                    z = position[2] < lo ? lo : (position[2] <= hi ? position[2] : hi);
                    dir_x = edge == 3 ? kBorderNegative : kBorderPositive;
                    dir_z = 0.0f;
                }
                const double dx = static_cast<double>(position[0]) - x;
                const double dz = static_cast<double>(position[2]) - z;
                const double d2 = dx * dx + dz * dz;
                const float distance = d2 <= kBorderMatchEpsilon ? 0.0f
                    : static_cast<float>(std::sqrt(d2));
                if (distance < best_distance) {
                    best_distance = distance;
                    best.zone = &r;
                    best.point = {x, position[1], z};
                    best.direction = {dir_x, 0.0f, dir_z};
                }
            }
        }
        if (best.zone != nullptr || side == 0 || side == 1 || side < 0) return best;
        side = -1;
    }
}

std::array<float, 3> get_closest_border_zone_008aecd0(const WorldMapBounds& bounds,
    const BorderZoneSet& zones, const std::array<float, 3>& position, float offset,
    bool& found) {
    const BorderZoneHit hit = closest_border_zone_004c7730(zones, position, -1);
    found = hit.zone != nullptr;
    if (!found) return position;
    const std::array<float, 3>& point = hit.point;
    // 008AEE94..008AEEB5.
    const float dx = to_float(static_cast<double>(point[0]) - position[0]);
    const float dz = to_float(static_cast<double>(point[2]) - position[2]);
    std::array<float, 3> add{};
    bool along = false;
    // 008AEEA2 COMISS offset, 0.0 / JBE; 008AEECA 0071C4F0 on the position.
    if (offset > 0.0f && !point_outside_world_map_0071c4f0(bounds, position)) {
        // 008AEED7..008AEEFB: ((dx*dx + 0*0) + dz*dz) as a float, then 1.0 < it.
        const float square = to_float((static_cast<double>(dx) * dx + 0.0) + static_cast<double>(dz) * dz);
        if (1.0f < square) {
            // 008AEF22 0042B2F0 on (dx, 0, dz); 008AEF2B..008AEF55.
            const float length = force_event_vector_length_0042b2f0({dx, 0.0f, dz});
            const float scale = to_float(static_cast<double>(offset) / length);
            add = {to_float(static_cast<double>(scale) * dx), to_float(0.0 * scale),
                to_float(static_cast<double>(scale) * dz)};
            along = true;
        }
    }
    if (!along) {
        // 008AEF5B..008AEF77: the outward direction times the offset.
        add = {to_float(static_cast<double>(hit.direction[0]) * offset),
            to_float(static_cast<double>(hit.direction[1]) * offset),
            to_float(static_cast<double>(offset) * hit.direction[2])};
    }
    // 008AEF83..008AEFA8.
    return {to_float(static_cast<double>(point[0]) + add[0]),
        to_float(static_cast<double>(add[1]) + point[1]),
        to_float(static_cast<double>(add[2]) + point[2])};
}

} // namespace bsp
