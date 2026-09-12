#include "bsp/avoid_zone_owner.hpp"

#include "bsp/geometry_helpers.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/pose_refresh.hpp"

#include <new>
#include <utility>

namespace bsp {
namespace {
using Point2 = std::array<float, 2>;
float subtract(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fsub b
        fstp result
    }
    return result;
}
float add(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
}
float multiply(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fmul b
        fstp result
    }
    return result;
}
float divide(float a, float b) noexcept {
    float result;
    __asm {
        fld a
        fdiv b
        fstp result
    }
    return result;
}
float squared_length(float x, float z) noexcept {
    float result;
    __asm {
        fld z
        fld x
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
float plane_distance(const Point2& p, const std::array<float, 3>& plane) noexcept {
    const auto* xy = p.data();
    const auto* abc = plane.data();
    float result;
    __asm {
        mov eax, xy
        mov ecx, abc
        fld dword ptr [eax+4]
        fmul dword ptr [ecx+4]
        fld dword ptr [eax]
        fmul dword ptr [ecx]
        faddp st(1), st(0)
        fadd dword ptr [ecx+8]
        fstp result
    }
    return result;
}
std::uint32_t count(const AvoidZoneNativeStorage& zone) noexcept {
    return static_cast<std::uint32_t>(zone.corners.count);
}
std::uint32_t scene_count(const AvoidZoneScenePointSlots& slots) noexcept {
    if (!slots.begin) return 0;
    const auto distance = reinterpret_cast<std::uintptr_t>(slots.end) -
        reinterpret_cast<std::uintptr_t>(slots.begin);
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(distance) >> 2);
}
ShipAiPathLateralRecord* make_record(const AvoidZoneAllocationAccess& alloc,
    float x, float z) {
    void* memory = alloc.allocate_record_00bf681b(alloc.context, 0x24);
    if (!memory) return nullptr; // Native branch retained; real new throws.
    auto* result = ::new (memory) ShipAiPathLateralRecord;
    result->x = x;
    result->z = z;
    return result;
}
void append(AvoidZoneNativeStorage& zone, ShipAiPathLateralRecord* record,
    const AvoidZoneAllocationAccess& alloc) {
    if (count(zone) == zone.capacity) {
        const auto capacity = zone.capacity * 2u + 2u;
        if (capacity > zone.capacity) {
            zone.capacity = capacity; // Store before allocation, including throw.
            const auto bytes = capacity > 0x3fffffffu ? 0xffffffffu : capacity * 4u;
            auto** replacement = static_cast<ShipAiPathLateralRecord**>(
                alloc.allocate_array_00bf55be(alloc.context, bytes));
            if (zone.corners.records) {
                for (std::uint32_t i = 0; i < count(zone); ++i)
                    replacement[i] = zone.corners.records[i];
                alloc.free_array_00bf6989(alloc.context, zone.corners.records);
            }
            zone.corners.records = replacement;
        }
    }
    zone.corners.records[count(zone)] = record;
    zone.corners.count = static_cast<std::int32_t>(count(zone) + 1u);
}
void include(AvoidZoneNativeStorage& zone, const Point2& point) noexcept {
    std::array<float, 4> bounds{zone.min_x, zone.min_z, zone.max_x, zone.max_z};
    include_point_00415010(bounds, point.data());
    zone.min_x = bounds[0]; zone.min_z = bounds[1];
    zone.max_x = bounds[2]; zone.max_z = bounds[3];
}
struct TemporaryPoints {
    const AvoidZoneAllocationAccess& alloc;
    Point2* points{};
    std::uint32_t count{};
    std::uint32_t capacity{};

    explicit TemporaryPoints(const AvoidZoneAllocationAccess& access) : alloc(access) {}
    ~TemporaryPoints() {
        if (points) alloc.free_array_00bf6989(alloc.context, points);
    }
    void reserve_initial(std::uint32_t n) {
        if (n) {
            capacity = n;
            points = static_cast<Point2*>(alloc.allocate_array_00bf55be(alloc.context, n * 8u));
        }
    }
    void append(const Point2& point) {
        if (count == capacity) {
            capacity = capacity * 2u + 2u; // Native temporary growth is unchecked.
            auto* replacement = static_cast<Point2*>(
                alloc.allocate_array_00bf55be(alloc.context, capacity * 8u));
            for (std::uint32_t i = 0; i < count; ++i) replacement[i] = points[i];
            if (points) alloc.free_array_00bf6989(alloc.context, points);
            points = replacement;
        }
        points[count] = point;
        ++count;
    }
};
Point2 intersection(const Point2& previous, const Point2& current,
    float previous_distance, float current_distance, bool entering) noexcept {
    const float dx = subtract(current[0], previous[0]);
    const float dz = subtract(current[1], previous[1]);
    // Native subtract/divide remain extended until the ratio's binary32 spill.
    float ratio;
    __asm {
        fld previous_distance
        cmp entering, 0
        je leaving
        fchs
        fld current_distance
        fsub previous_distance
        jmp denominator_ready
    leaving:
        fld previous_distance
        fsub current_distance
    denominator_ready:
        fdivp st(1), st(0)
        fstp ratio
    }
    const float product_x = multiply(ratio, dx);
    const float product_z = multiply(ratio, dz);
    return {add(previous[0], product_x), add(previous[1], product_z)};
}
} // namespace

AvoidZoneScenePoint& avoid_zone_scene_world_point_007af800(
    AvoidZoneScenePathAccess& scene, AvoidZoneScenePoint& output, std::uint32_t index) {
    auto& pose = scene.scene_pose();
    if (pose.world_valid_c8 == 0) refresh_pose_00414db0(pose);
    const auto& slots = scene.point_slots();
    if (!slots.begin || scene_count(slots) <= index) scene.invalid_parameter_00bf6713();
    const auto& source = *scene.point_slots().begin[index];
    const std::array<float, 4> local{source[0], source[1], source[2], 1.0f};
    std::array<float, 4> world;
    transform_native_vector4_00b62d10(local.data(), world.data(), &pose.world_cc);
    const float reciprocal_w = divide(1.0f, world[3]);
    output[0] = multiply(reciprocal_w, world[0]);
    output[1] = multiply(world[1], reciprocal_w);
    output[2] = multiply(reciprocal_w, world[2]);
    return output;
}

void avoid_zone_clear_records_004167c0(
    AvoidZoneNativeStorage& zone, const AvoidZoneAllocationAccess& alloc) noexcept {
    for (std::uint32_t i = 0; i < count(zone); ++i)
        alloc.free_record_00bf65ac(alloc.context, zone.corners.records[i]);
    zone.corners.count = 0;
}

void avoid_zone_clip_world_bounds_0041a540(AvoidZoneNativeStorage& zone,
    const AvoidZoneScenePoint& minimum, const AvoidZoneScenePoint& maximum,
    const AvoidZoneAllocationAccess& alloc) {
    if (zone.min_x >= maximum[0] || minimum[0] >= zone.max_x ||
        zone.min_z >= maximum[2] || minimum[2] >= zone.max_z) {
        avoid_zone_clear_records_004167c0(zone, alloc);
        return;
    }
    const std::array<std::array<float, 3>, 4> planes{{
        {1.0f, 0.0f, static_cast<float>(-(static_cast<double>(minimum[0]) - 100.0))},
        {-1.0f, 0.0f, static_cast<float>(static_cast<double>(maximum[0]) + 100.0)},
        {0.0f, 1.0f, static_cast<float>(-(static_cast<double>(minimum[2]) - 100.0))},
        {0.0f, -1.0f, static_cast<float>(static_cast<double>(maximum[2]) + 100.0)}}};
    TemporaryPoints first(alloc);
    first.reserve_initial(count(zone));
    first.count = count(zone);
    TemporaryPoints second(alloc);
    second.reserve_initial(count(zone));
    for (std::uint32_t i = 0; i < count(zone); ++i)
        first.points[i] = {zone.corners.records[i]->x, zone.corners.records[i]->z};
    avoid_zone_clear_records_004167c0(zone, alloc);
    auto* input = &first;
    auto* output = &second;
    for (const auto& plane : planes) {
        // Intentionally precedes the count check, matching0041A722.
        Point2 previous = input->points[input->count - 1u];
        output->count = 0;
        for (std::uint32_t i = 0; i < input->count; ++i) {
            const Point2 current = input->points[i];
            const float current_distance = plane_distance(current, plane);
            const float previous_distance = plane_distance(previous, plane);
            if (current_distance > 0.0f) {
                if (previous_distance < 0.0f)
                    output->append(intersection(previous, current, previous_distance, current_distance, true));
                output->append(current);
            } else if (previous_distance > 0.0f) {
                output->append(intersection(previous, current, previous_distance, current_distance, false));
            }
            previous = current;
        }
        if (output->count < 3u) return;
        std::swap(input, output);
    }
    for (std::uint32_t i = 0; i < input->count; ++i) {
        auto* record = make_record(alloc, 0.0f, 0.0f);
        append(zone, record, alloc);
        record->x = input->points[i][0];
        record->z = input->points[i][1];
    }
}

AvoidZoneNativeStorage& avoid_zone_construct_from_scene_path_0041ccd0(
    AvoidZoneNativeStorage& zone, AvoidZoneScenePathAccess& scene, std::uint32_t layer,
    const AvoidZoneScenePoint& minimum, const AvoidZoneScenePoint& maximum,
    const AvoidZoneAllocationAccess& alloc, const CameraAxesCrtAccess& crt) {
    zone.corners.records = nullptr;
    zone.corners.count = 0;
    zone.capacity = 0;
    zone.layer = layer;
    zone.associated_entity = nullptr;
    try {
        if (scene_count(scene.point_slots()) != 0) {
            AvoidZoneScenePoint world;
            avoid_zone_scene_world_point_007af800(scene, world, 0);
            Point2 previous{world[0], world[2]};
            zone.min_x = zone.max_x = previous[0];
            zone.min_z = zone.max_z = previous[1];
            append(zone, make_record(alloc, previous[0], previous[1]), alloc);
            const auto source_count = static_cast<std::int32_t>(scene_count(scene.point_slots()));
            for (std::int32_t i = 1; i < source_count; ++i) {
                avoid_zone_scene_world_point_007af800(scene, world, static_cast<std::uint32_t>(i));
                const Point2 point{world[0], world[2]};
                const auto dx = subtract(point[0], previous[0]);
                const auto dz = subtract(point[1], previous[1]);
                if (static_cast<double>(squared_length(dx, dz)) > 25.0) {
                    append(zone, make_record(alloc, point[0], point[1]), alloc);
                    include(zone, point);
                    previous = point;
                }
            }
        }
        avoid_zone_clip_world_bounds_0041a540(zone, minimum, maximum, alloc);
        if (count(zone) != 0) {
            (void)ship_ai_lateral_records_derive_0041a200(zone.corners, true, crt);
            if (scene.call_00923810(1)) {
                void* parent = scene.call_00923810(1);
                if (scene.call_parent_vtable_5c(parent, 0x44) != 0)
                    zone.associated_entity = scene.call_00923810(1);
            }
        }
    } catch (...) {
        // Native constructor unwind00C5E190 calls00412DC0, not004167C0.
        if (zone.corners.records) {
            alloc.free_array_00bf6989(alloc.context, zone.corners.records);
            zone.corners.records = nullptr; //00412DD2, missing from stored flow.
        }
        throw;
    }
    return zone;
}

void avoid_zone_release_owned_storage(
    AvoidZoneNativeStorage& zone, const AvoidZoneAllocationAccess& alloc) noexcept {
    avoid_zone_clear_records_004167c0(zone, alloc);
    if (zone.corners.records) alloc.free_array_00bf6989(alloc.context, zone.corners.records);
    zone.corners.records = nullptr;
    zone.capacity = 0;
}
} // namespace bsp
