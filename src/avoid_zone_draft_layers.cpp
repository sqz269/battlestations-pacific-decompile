#include "bsp/avoid_zone_draft_layers.hpp"
#include "bsp/vehicle_class_lua_load.hpp"

#include <cstring>
#include <stdexcept>
#include <utility>
#include <xmmintrin.h>

namespace bsp {
namespace {
std::int32_t read_depth(const std::byte* record, AvoidZoneDepthSlot slot) noexcept {
    std::int32_t value;
    std::memcpy(&value, record + static_cast<int>(slot), sizeof(value));
    return value;
}

float convert_depth(std::int32_t value) noexcept {
    return _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), value));
}

// Preserve the native single x87 operation followed by FSTP binary32 at each
// summation, division and centering store, including the caller's x87 mode.
float add_stored(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
}
float subtract_stored(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fsub b
        fstp result
    }
    return result;
}
float divide_stored(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fdiv b
        fstp result
    }
    return result;
}
float count_as_float(std::int32_t count) noexcept {
    float result;
    __asm {
        fild count
        fstp result
    }
    return result;
}
} // namespace

AvoidZoneDraftDepthInputs avoid_zone_draft_depth_inputs(
    const GameplayTuningSettings& settings, std::int32_t session_mode) noexcept {
    const std::byte* record = settings.gap_080h +
        (ship_tuning_block_offset(session_mode) - kAvoidZoneDepthsSingleOffset);
    return {
        read_depth(record, AvoidZoneDepthSlot::kBattleShip),
        read_depth(record, AvoidZoneDepthSlot::kMotherShip),
        read_depth(record, AvoidZoneDepthSlot::kDestroyer),
        read_depth(record, AvoidZoneDepthSlot::kTBoat),
        read_depth(record, AvoidZoneDepthSlot::kLargeLandingShip),
        read_depth(record, AvoidZoneDepthSlot::kCargoShip),
        read_depth(record, AvoidZoneDepthSlot::kLightCruiser),
        read_depth(record, AvoidZoneDepthSlot::kHeavyCruiser),
        read_depth(record, AvoidZoneDepthSlot::kSubmarine)};
}

void avoid_zone_draft_pair_append_004223b0(
    std::vector<AvoidZoneDraftLayer>& pairs, const AvoidZoneDraftLayer& value) {
    pairs.push_back(value);
}

std::vector<AvoidZoneDraftLayer> avoid_zone_draft_layers_00424dfb(
    const AvoidZoneDraftDepthInputs& input) {
    const std::int32_t values[] = {
        input.battle_ship, input.mother_ship, input.destroyer, input.tboat,
        input.large_landing_ship, input.cargo_ship, input.light_cruiser,
        input.heavy_cruiser, input.submarine};
    constexpr std::uint32_t masks[] = {
        0x10, 0x20, 0x40, 0x80, 0x200, 0x400, 0x800, 0x1000, 0x2000};
    std::vector<AvoidZoneDraftLayer> result;
    for (std::size_t i = 0; i < 9; ++i) {
        const float key = convert_depth(values[i]);
        std::size_t found = 0;
        while (found < result.size() && result[found].key != key) ++found;
        if (found == result.size()) {
            avoid_zone_draft_pair_append_004223b0(result, {key, masks[i]});
        } else {
            result[found].class_mask |= masks[i];
        }
    }
    return result;
}

std::int32_t avoid_zone_draft_key_00425459(const AvoidZoneDraftLayer& pair) noexcept {
    return _mm_cvtt_ss2si(_mm_set_ss(pair.key));
}

std::int32_t avoid_zone_draft_exact_group_00423c50(
    const AvoidZoneTable& table, std::int32_t requested_key) {
    if (table.groups.empty()) throw std::invalid_argument("native draft manager has no groups");
    const std::int32_t index = avoid_zone_group_for_layer_004120d0(table, requested_key);
    return table.groups[static_cast<std::size_t>(index)].layer_key == requested_key ? index : -1;
}

std::vector<AvoidZoneDraftPoint> avoid_zone_draft_points_00423cb6(
    const AvoidZoneNativeStorage& zone) {
    const std::uint32_t count = static_cast<std::uint32_t>(zone.corners.count);
    if (count < 3) return {};
    if (count > 0x1fffffff || zone.corners.records == nullptr)
        throw std::invalid_argument("invalid native draft zone corner array");
    std::vector<AvoidZoneDraftPoint> points;
    points.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto* corner = zone.corners.records[i];
        if (corner == nullptr) throw std::invalid_argument("null native draft zone corner");
        points.push_back({corner->x, corner->z});
    }
    return points;
}

float avoid_zone_draft_partition_parameter() noexcept {
    float value;
    std::memcpy(&value, &kAvoidZoneDraftPartitionParameterBits, sizeof(value));
    return value;
}

bool avoid_zone_draft_extrude_00423d81(
    const std::vector<AvoidZoneDraftPoint>& points,
    const std::vector<std::uint32_t>& indices, AvoidZoneDraftHullInput& output) {
    if (indices.size() < 3) return false;
    // The native twelve-byte point vector has maximum size 15555555h;
    // 00423DA6 doubles the number of indices before its resize call.
    if (indices.size() > 0x0aaaaaaa)
        throw std::length_error("native draft hull point vector too large");
    AvoidZoneDraftHullInput result{{0.0f, 0.0f, 0.0f}, {}};
    result.local_points.reserve(indices.size() * 2);
    for (const std::uint32_t index : indices) {
        if (index >= points.size()) throw std::out_of_range("native draft partition index");
        const auto& point = points[index];
        result.local_points.push_back({point[0], -500.0f, point[1]});
        result.local_points.push_back({point[0], 500.0f, point[1]});
        result.center.x = add_stored(point[0], result.center.x);
        result.center.z = add_stored(point[1], result.center.z);
    }
    const float count = count_as_float(static_cast<std::int32_t>(indices.size()));
    result.center.x = divide_stored(result.center.x, count);
    result.center.z = divide_stored(result.center.z, count);
    for (auto& point : result.local_points) {
        point.x = subtract_stored(point.x, result.center.x);
        // 004240E8 subtracts the double +0 at 00D7A258 from the exact +/-500.
        point.z = subtract_stored(point.z, result.center.z);
    }
    output = std::move(result);
    return true;
}

AvoidZoneDraftBodyInputs avoid_zone_draft_body_inputs_00424204(
    const AvoidZoneDraftHullInput& hull, std::uint32_t mask, const void* retained_hull) {
    if (retained_hull == nullptr) throw std::invalid_argument("native retained draft hull is null");
    AvoidZoneDraftBodyInputs result{};
    result.body = dyn_body_descriptor_default_009391c2();
    result.body.position[0] = hull.center.x;
    result.body.position[1] = 0.0f;
    result.body.position[2] = hull.center.z;
    result.body.flags = 1;
    result.body.shape_count = 1;
    result.shape.class_mask_08 = mask;
    result.shape.kind_10 = 4;
    result.shape.retained_hull_14 = retained_hull;
    result.shape.transform_18[0] = 1.0f;
    result.shape.transform_18[4] = 1.0f;
    result.shape.transform_18[8] = 1.0f;
    return result;
}
} // namespace bsp
