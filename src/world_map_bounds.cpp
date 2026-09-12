#include "bsp/world_map_bounds.hpp"

#include "bsp/scene_property_bag.hpp"

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
} // namespace bsp
