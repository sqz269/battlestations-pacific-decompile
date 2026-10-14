// Segment 8 large leftovers (cc13-left8b). See include/bsp/cc13_left8b_seg8.hpp
// and docs/CC13_LEFT8B.md. Offsets are the image's; comments give each
// function's address, original ABI and the simplifications made.

#include "bsp/cc13_left8b_seg8.hpp"

#include <cmath>
#include <cstring>

#include "bsp/band_0085_cc13.hpp"

namespace bsp::cc13_left8b {

namespace {

constexpr std::uint32_t kEnumTables = 0x00e1867c;          // [00E1867C], ECX of 0048E960
constexpr std::uint32_t kGame = 0x00e188a8;                // [00E188A8], the game object
constexpr std::uint32_t kTroopsEnabled = 0x00e081c0;       // byte
constexpr std::uint32_t kUpX = 0x00f8758c;                 // float3 global up vector
constexpr std::uint32_t kUpY = 0x00f87590;
constexpr std::uint32_t kUpZ = 0x00f87594;
constexpr std::uint32_t kAxis1 = 0x00f87580;               // float3 traffic axis
constexpr std::uint32_t kAxis2 = 0x00f87598;               // float3 traffic axis
constexpr float kOne = 1.0f;                               // 00D7A24C
constexpr float kRunnerRadiusStart = 2.0f;                 // 00CE3958
constexpr double kHalf = 0.5;                              // 00D7A280 (qword)
constexpr double kPhaseDivisor = 369097.0;                 // 00CE6428 (qword) = 5A1C9h
constexpr double kPhaseRange = 60.0;                       // 00CE3D68 (qword)
constexpr double kLaneLengthPad = 2.0;                     // 00D7A308 (qword)
constexpr float kLaneQuarter = 0.25f;                      // 00CE3868
constexpr double kLaneTailPad = 42.0;                      // 00CE6840 (qword)
constexpr double kLaneSpacingScale = 3.0;                  // 00D7A2B0 (qword)
constexpr double kFive = 5.0;                              // 00D7A370 (qword)
constexpr double kBuoyGapGrow = 0.10000000149011612;       // 00D7A3A0 (qword, 0.1f widened)
constexpr float kBuoyFieldFive = 5.0f;                     // 00CE3850
constexpr float kSceneBestStart = 1.0e6f;                  // 00CE6A04
constexpr float kSceneBestInner = 1.0e10f;                 // 00CE4970
constexpr double kPad = 2.0;                               // 00D7A308 (qword)
constexpr float kNegativeZero = -0.0f;                     // 00D7A208
constexpr float kPi = 3.14159274f;                         // 00D7A264 (00CE684C holds -kPi)
constexpr float kTen = 10.0f;                              // 00CE38B8 (00CE6848 holds -10)
constexpr double kOneD = 1.0;                              // 00D7A210 (qword)
// Formation wobble: angle = index * c + time * k (qwords; k are float values widened).
constexpr double kWobble1 = 42.45;                         // 00CE6820
constexpr double kWobbleT1 = 0.27000001072883606;          // 00CE6828
constexpr double kWobble2 = 97.76;                         // 00CE6810
constexpr double kWobbleT2 = 0.3199999928474426;           // 00CE6818
constexpr double kWobble3 = 54.23;                         // 00CE6800
constexpr double kWobbleT3 = 0.28999999165534973;          // 00CE6808
constexpr double kWobble4 = 83.87;                         // 00CE67F0
constexpr double kWobbleT4 = 0.3400000035762787;           // 00CE67F8
constexpr double kJitterStep = 0.30000001192092896;        // 00CE3DC8
constexpr double kJitterBias = 0.15000000596046448;        // 00CE6618
// Debris tables (std::vector objects: proxy, first, last, end) and globals.
constexpr std::uint32_t kDebrisSpawnCount = 0x00e186e8;    // dword counter
constexpr std::uint32_t kDebrisSpread = 0x00e18744;        // float
constexpr std::uint32_t kDebrisSpreadSmall = 0x00e18748;   // float
constexpr std::uint32_t kDebrisCountHi = 0x00e1874c;       // int
constexpr std::uint32_t kDebrisCountLo = 0x00e18750;       // int
constexpr std::uint32_t kDebrisCountHiSmall = 0x00e18754;  // int
constexpr std::uint32_t kDebrisCountLoSmall = 0x00e18758;  // int
constexpr std::uint32_t kDebrisClasses = 0x00e1875c;       // vector<class*>
constexpr std::uint32_t kDebrisWeightsSmall = 0x00e1876c;  // vector<float>
constexpr std::uint32_t kDebrisLimitsSmall = 0x00e1878c;   // vector<int>
constexpr std::uint32_t kDebrisWeights = 0x00e1879c;       // vector<float>
constexpr std::uint32_t kDebrisLimits = 0x00e187ac;        // vector<int>

constexpr std::uint32_t kPathRuntimeVtable = 0x00ce68d4;
constexpr std::uint32_t kSceneTrafficVtable = 0x00ce68e4;
constexpr std::uint32_t kUnitPathVtable = 0x00ce6a08;
constexpr std::uint32_t kLandingLaneVtable = 0x00ce6a68;
constexpr std::uint32_t kBuoyFieldVtable = 0x00ce6860;
constexpr std::uint32_t kSoldierTypeAnimsVtable = 0x00ce691c;
constexpr std::uint32_t kShipIdleGroupVtable = 0x00ce69d4;

// A native string {length +0, data +4}; its inlined destructor returns data to
// the sized pool (00419CC0, then 00BD1510(ECX = pool, data, length + 1, 1)).
struct NativeString {
    std::uint32_t length{0};
    char* data{nullptr};
};

void release_native_string(Left8bHost& host, NativeString& s) {
    if (s.data != nullptr) host.string_pool_return_00bd1510(host.string_pool_00419cc0(), s.data, s.length + 1, 1);
}

// The inlined construction from a C string as these bodies emit it: zero the
// header, strlen, 0041DD40(length, 1), then memcpy(data, text, length + 1)
// when data is set. The memcpy is the CRT 00BF7680.
void native_string_from_text(Left8bHost& host, NativeString& s, const char* text) {
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
}

// The checked vector size of a {proxy, first +4, last +8, end +0Ch} vector of
// `stride`-byte elements: 0 when first is null.
std::uint32_t vector_size(Object vector, std::uint32_t stride) {
    Object first = at<Object>(vector, 0x4);
    if (first == nullptr) return 0;
    return static_cast<std::uint32_t>(at<Object>(vector, 0x8) - first) / stride;
}

}  // namespace

// 004A4B70 __thiscall(runtime, parent, const record* source), RET 8, EAX =
// runtime. The scene-traffic path-runtime base; derived by 004A50D0, 004A5470
// and (through it) 004A66B0. Steps, in image order:
//   00496920(runtime, parent); 0049FA70(runtime+20h, source) copies the 4Ch
//   traffic record (+20h is its path object); vtable 00CE68D4; list +6Ch head
//   from 00485320, list +78h head from 004870F0 (sizes 0); 0049F9B0 builds the
//   second 4Ch record at +8Ch; the five vectors at +E0h, +F0h, +100h, +110h,
//   +120h get first/last/end = 0; +84h = 0.0f, +88h = 0.
//   entity = 007ACA30(path); +D8h = [entity+3Ch] when it is set and its
//   vtable +5Ch(44h) (is-kind-of) is true, else 0.
//   The first key of the map at +54h (head [+58h]; key at node+0Ch; the image
//   calls 00BF6713 when the map is empty) is looked up by value in the
//   "LandVehicleclasses" enumeration (0048E960 with ECX = [00E1867C], then
//   008F0D80); +DCh = (no name), i.e. the group is made of soldiers.
//   +130h = 1.0f, +134h = 2.0f.
//   For each map node: soldiers -> the "SoldierTypes" name of the key becomes
//   a native string, 004B1400 loads the soldier class, 00499210 pushes it on
//   +F0h, the string is released; vehicles -> 00964790(key, 1) and 0047B3C0
//   give the visual class pushed on +E0h (00499190), and +134h becomes the
//   maximum of itself and the class's +A0h, +A4h, +ACh (each a strict
//   class > current test, as FCOMIP/JBE); both -> 0045AFD0 pushes the mapped
//   float (node+10h) on +100h; 0047D090 advances.
//   004809F0 normalises +100h. Then for the map<string, float> at +60h
//   (head [+64h]): 00450540 appends the key (node+0Ch) to +110h and the value
//   (node+14h) is pushed on +120h through the inlined push_back (store when
//   capacity remains, else 0045A120(vector, &result, vector, last, &value)
//   after the first <= last check); 00443880 advances. 004809F0 normalises +120h.
//   When [+34h] > 1: +134h += ([+34h] - 1) * [+3Ch] * 0.5 (x87 FILD/FMUL float/
//   FMUL qword/FADD float, one float store). +138h = +13Ch = +140h = +134h.
//   When byte path+18h is 0, 007AF150(path) derives the knots. +144h =
//   ([+28h] < [+24h]) signed; +148h = byte [+2Ch] ? path+28h : path+1Ch; when
//   [+30h] > 0, +38h = that length / [+30h] (FIDIV).
// Simplifications: the iterator checks that compare the container with itself
// or a node with the head inside the loop cannot fail and are not modelled.
// The x87 expressions are evaluated in double, not 80-bit extended, before the
// float store (last-bit differences only).
Object path_runtime_construct_004a4b70(Left8bHost& host, Object runtime, Object parent, Object record) {
    cc13_left8::list_node_construct_00496920(host, runtime, parent);
    cc13_left8::traffic_record_copy_0049fa70(host, runtime + 0x20, record);
    at<std::uint32_t>(runtime, 0x0) = kPathRuntimeVtable;
    at<Object>(runtime, 0x70) = host.list_buy_head_node_00485320(runtime + 0x6c);
    at<std::uint32_t>(runtime, 0x74) = 0;
    at<Object>(runtime, 0x7c) = host.list_buy_head_node_004870f0(runtime + 0x78);
    at<std::uint32_t>(runtime, 0x80) = 0;
    cc13_left8::traffic_record_construct_0049f9b0(host, runtime + 0x8c);
    for (std::size_t vector = 0xe0; vector <= 0x120; vector += 0x10) {
        at<std::uint32_t>(runtime, vector + 0x4) = 0;
        at<std::uint32_t>(runtime, vector + 0x8) = 0;
        at<std::uint32_t>(runtime, vector + 0xc) = 0;
    }
    Object entity = host.path_object_get_entity_007aca30(at<Object>(runtime, 0x20));
    Object attached = at<Object>(entity, 0x3c);
    at<float>(runtime, 0x84) = 0.0f;
    at<std::uint32_t>(runtime, 0x88) = 0;
    if (attached != nullptr && host.vcall_is_kind_of_5c(attached, 0x44)) {
        at<Object>(runtime, 0xd8) = attached;
    } else {
        at<Object>(runtime, 0xd8) = nullptr;
    }

    Object kinds = runtime + 0x54;
    Object first = at<Object>(at<Object>(kinds, 0x4), 0x0);
    if (first == at<Object>(kinds, 0x4)) host.invalid_parameter_00bf6713();
    const std::uint32_t first_key = at<std::uint32_t>(first, 0xc);
    Object tables = obj(host.global_u32(kEnumTables));
    const char* vehicle = host.enum_find_name_by_value_008f0d80(
        host.enum_table_read_word_0048e960(tables, "LandVehicleclasses"), first_key);
    at<float>(runtime, 0x130) = kOne;
    at<bool>(runtime, 0xdc) = vehicle == nullptr;
    at<float>(runtime, 0x134) = kRunnerRadiusStart;

    Object iterator[2] = {kinds, at<Object>(at<Object>(kinds, 0x4), 0x0)};
    while (iterator[1] != at<Object>(kinds, 0x4)) {
        Object node = iterator[1];
        if (at<bool>(runtime, 0xdc)) {
            const char* text = host.enum_find_name_by_value_008f0d80(
                host.enum_table_read_word_0048e960(obj(host.global_u32(kEnumTables)), "SoldierTypes"),
                at<std::uint32_t>(node, 0xc));
            NativeString name;
            native_string_from_text(host, name, text);
            Object soldier_class = cc13_left8::soldier_class_get_or_load_004b1400(host, &name);
            host.pointer_vector_push_back_00499210(runtime + 0xf0, &soldier_class);
            release_native_string(host, name);
        } else {
            Object vehicle_class = host.vehicle_class_get_or_create_00964790(at<std::uint32_t>(node, 0xc), true);
            Object visual = host.vehicle_visual_class_get_or_create_0047b3c0(vehicle_class);
            host.pointer_vector_push_back_00499190(runtime + 0xe0, &visual);
            for (std::size_t field : {std::size_t{0xa0}, std::size_t{0xa4}, std::size_t{0xac}}) {
                const float candidate = at<float>(vehicle_class, field);
                const float current = at<float>(runtime, 0x134);
                at<float>(runtime, 0x134) = candidate > current ? candidate : current;
            }
        }
        host.float_vector_push_back_0045afd0(runtime + 0x100, &at<float>(node, 0x10));
        host.tree_iterator_increment_0047d090(reinterpret_cast<Object>(iterator));
    }
    host.weights_normalize_004809f0(runtime + 0x100);

    Object costs = runtime + 0x60;
    Object weights = runtime + 0x120;
    Object cost_iterator[2] = {costs, at<Object>(at<Object>(costs, 0x4), 0x0)};
    while (cost_iterator[1] != at<Object>(costs, 0x4)) {
        Object node = cost_iterator[1];
        host.native_string_vector_append_00450540(runtime + 0x110, node + 0xc);
        const float* value = &at<float>(node, 0x14);
        Object first_weight = at<Object>(weights, 0x4);
        if (first_weight != nullptr && vector_size(weights, 4) < static_cast<std::uint32_t>(
                                                                   at<Object>(weights, 0xc) - first_weight) / 4) {
            at<float>(at<Object>(weights, 0x8), 0x0) = *value;
            at<Object>(weights, 0x8) += 4;
        } else {
            Object last = at<Object>(weights, 0x8);
            if (first_weight > last) host.invalid_parameter_00bf6713();
            Object result[2] = {nullptr, nullptr};
            host.float_vector_insert_0045a120(weights, reinterpret_cast<Object>(result), weights, last, value);
        }
        host.string_float_map_iterator_increment_00443880(reinterpret_cast<Object>(cost_iterator));
    }
    host.weights_normalize_004809f0(weights);

    const std::int32_t members = at<std::int32_t>(runtime, 0x34);
    if (members > 1) {
        at<float>(runtime, 0x134) = static_cast<float>(
            static_cast<double>(members - 1) * at<float>(runtime, 0x3c) * kHalf + at<float>(runtime, 0x134));
    }
    const float radius = at<float>(runtime, 0x134);
    at<float>(runtime, 0x140) = radius;
    at<float>(runtime, 0x13c) = radius;
    at<float>(runtime, 0x138) = radius;
    Object path = at<Object>(runtime, 0x20);
    if (at<std::uint8_t>(path, 0x18) == 0) host.camera_path_derive_knots_007af150(path);
    at<bool>(runtime, 0x144) = at<std::int32_t>(runtime, 0x28) < at<std::int32_t>(runtime, 0x24);
    const float length = at<std::uint8_t>(runtime, 0x2c) != 0 ? at<float>(at<Object>(runtime, 0x20), 0x28)
                                                              : at<float>(at<Object>(runtime, 0x20), 0x1c);
    at<float>(runtime, 0x148) = length;
    const std::int32_t count = at<std::int32_t>(runtime, 0x30);
    if (count > 0) at<float>(runtime, 0x38) = static_cast<float>(static_cast<double>(length) / count);
    return runtime;
}


// 004A50D0 __thiscall(runtime, parent, const record* source), RET 8, EAX =
// runtime. The scene traffic runtime (vtable 00CE68E4) that 004A5620 creates
// for each committed item. 004A4B70(runtime, parent, source); +14Ch = source.
// The path points from min(+24h, +28h) to max(+24h, +28h) through 007AF800
// (path, &point, index) give the box {min +150h, max +15Ch}: both corners take
// the first point, 00427D10 includes the rest. The box is then grown by the
// half extents (+138h, +13Ch, +140h): min -= them, max += them (float stores).
// +168h = 0. With an attached entity (+D8h) its child list ([entity+48h],
// next at child+44h) is walked: for each child whose vtable +5Ch(1Ch) is true,
// the child's world matrix is refreshed when byte child+C8h is 0 (00414DB0 on
// the parent [child+3Ch] when set, then 00413920(child+74h, &temp, parent+CCh)
// or child+74h itself copied to child+CCh..+108h, +C8h = 1, +10Ch = 0), and the
// smallest squared distance from the child's world position (+FCh..+104h) to
// every path point (007AF800 over [path+8]..[path+0Ch]) is taken, starting
// from 1.0e10f; each squared distance is stored as a float before the strict
// compare. When it is below the running best (starting 1.0e6f), +168h becomes
// that child. Finally +16Ch = 0, 00487470(runtime), +16Ch = 1,
// 00487470(runtime), +16Ch = 3.
// Simplifications: the squared distance is formed in double rather than x87
// extended before the float store (dx*dx + dy*dy first, then + dz*dz, as the
// image orders it).
Object scene_traffic_runtime_construct_004a50d0(Left8bHost& host, Object runtime, Object parent, Object record) {
    path_runtime_construct_004a4b70(host, runtime, parent, record);
    at<std::uint32_t>(runtime, 0x0) = kSceneTrafficVtable;
    at<Object>(runtime, 0x14c) = record;
    const std::int32_t a = at<std::int32_t>(runtime, 0x24);
    const std::int32_t b = at<std::int32_t>(runtime, 0x28);
    std::int32_t index = a < b ? a : b;
    const std::int32_t last = a > b ? a : b;
    float point[3];
    const float* p = host.scene_path_transform_point_to_world_007af800(at<Object>(runtime, 0x20), point, index);
    float* box = &at<float>(runtime, 0x150);
    at<float>(runtime, 0x15c) = p[0];
    at<float>(runtime, 0x160) = p[1];
    at<float>(runtime, 0x164) = p[2];
    box[0] = at<float>(runtime, 0x15c);
    box[1] = at<float>(runtime, 0x160);
    box[2] = at<float>(runtime, 0x164);
    for (++index; index <= last; ++index) {
        host.aabb_include_point_00427d10(
            box, host.scene_path_transform_point_to_world_007af800(at<Object>(runtime, 0x20), point, index));
    }
    float best = kSceneBestStart;
    box[0] = box[0] - at<float>(runtime, 0x138);
    box[1] = box[1] - at<float>(runtime, 0x13c);
    box[2] = box[2] - at<float>(runtime, 0x140);
    at<float>(runtime, 0x15c) = at<float>(runtime, 0x138) + at<float>(runtime, 0x15c);
    at<float>(runtime, 0x160) = at<float>(runtime, 0x13c) + at<float>(runtime, 0x160);
    at<float>(runtime, 0x164) = at<float>(runtime, 0x140) + at<float>(runtime, 0x164);
    at<Object>(runtime, 0x168) = nullptr;
    Object entity = at<Object>(runtime, 0xd8);
    if (entity != nullptr) {
        for (Object child = at<Object>(entity, 0x48); child != nullptr; child = at<Object>(child, 0x44)) {
            if (!host.vcall_is_kind_of_5c(child, 0x1c)) continue;
            float nearest = kSceneBestInner;
            for (std::int32_t knot = 0;; ++knot) {
                Object path = at<Object>(runtime, 0x20);
                if (knot >= static_cast<std::int32_t>(vector_size(path + 0x4, 4))) break;
                if (at<std::uint8_t>(child, 0xc8) == 0) {
                    Object owner = at<Object>(child, 0x3c);
                    if (owner != nullptr) host.entity_pose_refresh_world_00414db0(owner);
                    float product[16];
                    const float* world = at<Object>(child, 0x3c) != nullptr
                                             ? host.matrix_multiply_00413920(child + 0x74, product,
                                                                             &at<float>(at<Object>(child, 0x3c), 0xcc))
                                             : &at<float>(child, 0x74);
                    for (std::size_t i = 0; i < 16; ++i) at<float>(child, 0xcc + 4 * i) = world[i];
                    at<std::uint8_t>(child, 0xc8) = 1;
                    at<std::uint8_t>(child, 0x10c) = 0;
                }
                const float* q = host.scene_path_transform_point_to_world_007af800(at<Object>(runtime, 0x20), point,
                                                                                   knot);
                const float dx = q[0] - at<float>(child, 0xfc);
                const float dy = q[1] - at<float>(child, 0x100);
                const float dz = q[2] - at<float>(child, 0x104);
                const float distance = static_cast<float>(
                    (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) + static_cast<double>(dz) * dz);
                if (nearest > distance) nearest = distance;
            }
            if (best > nearest) {
                best = nearest;
                at<Object>(runtime, 0x168) = child;
            }
        }
    }
    at<std::uint32_t>(runtime, 0x16c) = 0;
    host.traffic_runtime_pass_00487470(runtime);
    at<std::uint32_t>(runtime, 0x16c) = 1;
    host.traffic_runtime_pass_00487470(runtime);
    at<std::uint32_t>(runtime, 0x16c) = 3;
    return runtime;
}

// 004A5470 __thiscall(runtime, parent, unit, const record* source), RET 0Ch,
// EAX = runtime. The unit-attached path runtime (vtable 00CE6A08), created by
// 004A6830 for units that are not of kind 0Ch. 004A4B70(runtime, parent,
// source); +14Ch = unit. The path points from min(+24h, +28h) to max through
// 007AF740 (path, &point, index) give a stack box {min, max} (00427D10 includes
// the rest). The four ground corners at the box's minimum height are stored:
// +150h (min.x, min.y, min.z), +15Ch (min.x, min.y, max.z), +168h (max.x,
// min.y, max.z), +174h (max.x, min.y, min.z); max.y is never stored.
// +84h = float(float((word[unit+174h] * 7B913h + 67933h) mod 5A1C9h unsigned)
// / 369097.0) * 60.0): a deterministic per-unit phase in [0, 60).
Object unit_path_runtime_construct_004a5470(Left8bHost& host, Object runtime, Object parent, Object unit,
                                            Object record) {
    path_runtime_construct_004a4b70(host, runtime, parent, record);
    at<std::uint32_t>(runtime, 0x0) = kUnitPathVtable;
    at<Object>(runtime, 0x14c) = unit;
    const std::int32_t a = at<std::int32_t>(runtime, 0x24);
    const std::int32_t b = at<std::int32_t>(runtime, 0x28);
    std::int32_t index = a < b ? a : b;
    const std::int32_t last = a > b ? a : b;
    float point[3];
    const float* p = host.scene_path_point_to_world_cached_007af740(at<Object>(runtime, 0x20), point, index);
    float box[6] = {p[0], p[1], p[2], p[0], p[1], p[2]};
    for (++index; index <= last; ++index) {
        host.aabb_include_point_00427d10(
            box, host.scene_path_point_to_world_cached_007af740(at<Object>(runtime, 0x20), point, index));
    }
    const float corners[12] = {box[0], box[1], box[2], box[0], box[1], box[5],
                               box[3], box[1], box[5], box[3], box[1], box[2]};
    for (std::size_t i = 0; i < 12; ++i) at<float>(runtime, 0x150 + 4 * i) = corners[i];
    const std::uint32_t hash =
        (static_cast<std::uint32_t>(at<std::uint16_t>(unit, 0x174)) * 0x7b913u + 0x67933u) % 0x5a1c9u;
    const float phase = static_cast<float>(static_cast<double>(static_cast<std::int32_t>(hash)) / kPhaseDivisor);
    at<float>(runtime, 0x84) = static_cast<float>(phase * kPhaseRange);
    return runtime;
}

// 004A66B0 __thiscall(lane, parent, unit, const record* source), RET 0Ch, EAX
// = lane. The landing lane (vtable 00CE6A68, 1B4h bytes), created by 004A6830
// for units of kind 0Ch. 004A5470(lane, parent, unit, source); byte +180h = 0;
// +D8h = 0. L = 007B05E0(path, knot count - 1) (count from [path+8]..[path+0Ch],
// 0 when unset, so the index is then -1), stored as float at +18Ch. s = [+38h]
// (the member spacing). +30h = _ftol2(L / s * 0.5) (00BF7420 truncates);
// +188h = 0.25f; +184h = float(L + 2.0); +84h = 0.0f; +190h = float(+184h +
// 42.0); +194h = float(s * 3.0); +198h = float([+3Ch] * 5.0).
// The division order L / s is from the bytes (DE FA, FDIVP ST(2): ST(2) =
// ST(2) / ST(0) with ST(2) = L, ST(0) = s). x87 intermediates are evaluated
// in double.
Object landing_lane_construct_004a66b0(Left8bHost& host, Object lane, Object parent, Object unit, Object record) {
    unit_path_runtime_construct_004a5470(host, lane, parent, unit, record);
    Object path = at<Object>(lane, 0x20);
    at<std::uint32_t>(lane, 0x0) = kLandingLaneVtable;
    at<std::uint8_t>(lane, 0x180) = 0;
    at<Object>(lane, 0xd8) = nullptr;
    const std::uint32_t knots = vector_size(path + 0x4, 4);
    const float length = host.path_knot_distance_007b05e0(path, knots - 1);
    at<float>(lane, 0x18c) = length;
    const float spacing = at<float>(lane, 0x38);
    const double members = static_cast<double>(length) / spacing * kHalf;
    at<std::int32_t>(lane, 0x30) = static_cast<std::int32_t>(members);
    at<float>(lane, 0x188) = kLaneQuarter;
    const float padded = static_cast<float>(static_cast<double>(length) + kLaneLengthPad);
    at<float>(lane, 0x184) = padded;
    at<float>(lane, 0x84) = 0.0f;
    at<float>(lane, 0x190) = static_cast<float>(static_cast<double>(padded) + kLaneTailPad);
    at<float>(lane, 0x194) = static_cast<float>(static_cast<double>(spacing) * kLaneSpacingScale);
    at<float>(lane, 0x198) = static_cast<float>(static_cast<double>(at<float>(lane, 0x3c)) * kFive);
    return lane;
}

// 004A5850 __thiscall(field, parent, const float box[6], const vector<NativeString>*
// class_names, const vector<float>* weights, float gap, float deviation), RET
// 18h, EAX = field. The buoy field (vtable 00CE6860, 84h bytes) built by
// TempAddFloatsam 004A5F30. 00496920(field, parent); the vectors at +20h,
// +30h and +48h get first/last/end = 0; the box is copied to +58h..+6Ch and
// its max.x (+64h) and max.z (+6Ch) grow by float(gap * 0.1f-as-double).
// 00495C80(+20h, name count, 0) sizes the class vector; each class_names[i]
// (8-byte strings; the image calls 00BF6713 on an out-of-range index) gets
// 00475710 (ECX = &name) stored in +20h[i]. 004951A0(+30h, weights) copies
// the weights and 004809F0 normalises them. +40h = gap, +44h = deviation.
// nx = _ftol2((box.max.x - box.min.x) / gap + 1), nz = _ftol2((box.max.z -
// box.min.z) / gap + 1), from the ORIGINAL box argument (not the grown copy).
// 004A48D0(+48h, nx, empty vector) sizes the rows; each row i < nx gets
// 00495AE0(row, nz, 0). +70h = +78h = float(deviation + 5.0), +74h = 5.0f,
// +7Ch = float(gap + gap), +80h = 0.
// x87 order checked against the bytes: DE FA (FDIVP ST(2): dx / gap), DC C2
// (FADD ST(2), ST0: + 1), DE F1 (FDIVRP ST(1): dz / gap), DE C1 (+ 1). The
// image's bounds checks before each element access are kept.
Object buoy_field_construct_004a5850(Left8bHost& host, Object field, Object parent, const float* box,
                                     Object class_names, Object weights, float gap, float deviation) {
    cc13_left8::list_node_construct_00496920(host, field, parent);
    at<std::uint32_t>(field, 0x0) = kBuoyFieldVtable;
    for (std::size_t vector : {std::size_t{0x20}, std::size_t{0x30}, std::size_t{0x48}}) {
        at<std::uint32_t>(field, vector + 0x4) = 0;
        at<std::uint32_t>(field, vector + 0x8) = 0;
        at<std::uint32_t>(field, vector + 0xc) = 0;
    }
    for (std::size_t i = 0; i < 6; ++i) at<float>(field, 0x58 + 4 * i) = box[i];
    const float grow = static_cast<float>(static_cast<double>(gap) * kBuoyGapGrow);
    at<float>(field, 0x64) = at<float>(field, 0x64) + grow;
    at<float>(field, 0x6c) = grow + at<float>(field, 0x6c);
    Object classes = field + 0x20;
    host.pointer_vector_resize_00495c80(classes, vector_size(class_names, 8), 0);
    for (std::uint32_t i = 0; i < host.native_string_vector_size_00427ac0(class_names); ++i) {
        if (at<Object>(class_names, 0x4) == nullptr || i >= vector_size(class_names, 8)) {
            host.invalid_parameter_00bf6713();
        }
        Object name = at<Object>(class_names, 0x4) + 8 * i;
        if (at<Object>(classes, 0x4) == nullptr || i >= vector_size(classes, 4)) host.invalid_parameter_00bf6713();
        Object* slot = reinterpret_cast<Object*>(at<Object>(classes, 0x4) + 4 * i);
        *slot = host.buoy_class_get_or_create_00475710(name);
    }
    host.float_vector_assign_004951a0(field + 0x30, weights);
    host.weights_normalize_004809f0(field + 0x30);
    at<float>(field, 0x40) = gap;
    at<float>(field, 0x44) = deviation;
    const double dx = static_cast<double>(box[3]) - box[0];
    const std::int32_t nx = static_cast<std::int32_t>(dx / gap + 1.0);
    const double dz = static_cast<double>(box[5]) - box[2];
    const std::int32_t nz = static_cast<std::int32_t>(dz / gap + 1.0);
    Object rows = field + 0x48;
    const std::uint32_t empty_row[4] = {0, 0, 0, 0};
    host.vector_vector_resize_004a48d0(rows, static_cast<std::uint32_t>(nx), empty_row);
    for (std::int32_t i = 0; i < nx; ++i) {
        if (at<Object>(rows, 0x4) == nullptr || static_cast<std::uint32_t>(i) >= vector_size(rows, 0x10)) {
            host.invalid_parameter_00bf6713();
        }
        host.dword_vector_resize_00495ae0(at<Object>(rows, 0x4) + 0x10 * i, static_cast<std::uint32_t>(nz), 0);
    }
    const float edge = static_cast<float>(static_cast<double>(deviation) + kFive);
    at<float>(field, 0x70) = edge;
    at<float>(field, 0x74) = kBuoyFieldFive;
    at<float>(field, 0x78) = edge;
    at<float>(field, 0x7c) = static_cast<float>(static_cast<double>(gap) + gap);
    at<std::uint32_t>(field, 0x80) = 0;
    return field;
}

// 004A5F30 __thiscall(manager, LuaObject table by value), RET 14h:
// TempAddFloatsam (008C5B80 passes the LuaObject of arg 0). top =
// table.topBox; the box min = 00888760(top.minCorner), max =
// 00888760(top.maxCorner) (each through a temporary that is destructed);
// top = table.buoyClassNames (00B67690 assign). For i = 1.. while top[i]
// (00B67720) is not nil: its string (00B662B0) becomes a native string that
// 00450540 appends to the local name vector, then is released. top =
// table.buoyTypeWeights; for i = 1.. while not nil: float(00B66270) pushed on
// the local weight vector (inlined push_back; 0045A120 when full). gap =
// float(table.buoyGap), deviation = float(table.buoyDev). operator new(84h)
// -> 004A5850(memory, [manager+8], &box, &names, &weights, gap, deviation)
// (0 when the allocation fails). When table.tempID is not nil, its string
// (constructed by 0041E870) keys 0049CB60(manager+34h) and the slot takes the
// field. Teardown: the element and top Lua objects are destructed, the weight
// buffer is freed (00BF65AC), the names are destroyed by 00432050 (ECX =
// first, EDX = last, stack: &vector, then the stale dword in the frame slot
// the string/allocation shared; it is an unused allocator argument) and freed,
// and the by-value table is destructed.
// Simplifications: the frame slot passed as 00432050's second stack argument is
// modelled as the value it holds (the tempID string length when the tempID
// branch ran, else the 84h allocation pointer).
void temp_add_floatsam_004a5f30(Left8bHost& host, Object manager, LuaObject* table) {
    std::uint32_t names[4] = {0, 0, 0, 0};    // vector<NativeString> {proxy, first, last, end}
    std::uint32_t weights[4] = {0, 0, 0, 0};  // vector<float>
    Object name_vector = reinterpret_cast<Object>(names);
    Object weight_vector = reinterpret_cast<Object>(weights);
    LuaObject top{};
    LuaObject temp{};
    host.lua_get_by_name_00b67800(table, &top, "topBox");
    float box[6];
    float corner[3];
    host.lua_read_vector3_00888760(corner, host.lua_get_by_name_00b67800(&top, &temp, "minCorner"));
    box[0] = corner[0];
    box[1] = corner[1];
    box[2] = corner[2];
    host.lua_object_destruct_00b67700(&temp);
    host.lua_read_vector3_00888760(corner, host.lua_get_by_name_00b67800(&top, &temp, "maxCorner"));
    box[3] = corner[0];
    box[4] = corner[1];
    box[5] = corner[2];
    host.lua_object_destruct_00b67700(&temp);
    host.lua_object_assign_00b67690(&top, host.lua_get_by_name_00b67800(table, &temp, "buoyClassNames"));
    host.lua_object_destruct_00b67700(&temp);

    LuaObject element{};
    std::int32_t index = 1;
    host.lua_get_by_index_00b67720(&top, &element, 1);
    while (!host.lua_is_nil_00b65fb0(&element)) {
        NativeString name;
        native_string_from_text(host, name, host.lua_get_string_00b662b0(&element));
        host.native_string_vector_append_00450540(name_vector, &name);
        release_native_string(host, name);
        ++index;
        host.lua_object_assign_00b67690(&element, host.lua_get_by_index_00b67720(&top, &temp, index));
        host.lua_object_destruct_00b67700(&temp);
    }

    host.lua_object_assign_00b67690(&top, host.lua_get_by_name_00b67800(table, &temp, "buoyTypeWeights"));
    host.lua_object_destruct_00b67700(&temp);
    index = 1;
    host.lua_object_assign_00b67690(&element, host.lua_get_by_index_00b67720(&top, &temp, index));
    host.lua_object_destruct_00b67700(&temp);
    while (!host.lua_is_nil_00b65fb0(&element)) {
        const float value = static_cast<float>(host.lua_get_number_00b66270(&element));
        Object first = at<Object>(weight_vector, 0x4);
        Object last = at<Object>(weight_vector, 0x8);
        if (first != nullptr && static_cast<std::uint32_t>(last - first) / 4 <
                                    static_cast<std::uint32_t>(at<Object>(weight_vector, 0xc) - first) / 4) {
            at<float>(last, 0x0) = value;
            at<Object>(weight_vector, 0x8) = last + 4;
        } else {
            if (first > last) host.invalid_parameter_00bf6713();
            Object result[2] = {nullptr, nullptr};
            host.float_vector_insert_0045a120(weight_vector, reinterpret_cast<Object>(result), weight_vector, last,
                                              &value);
        }
        ++index;
        host.lua_object_assign_00b67690(&element, host.lua_get_by_index_00b67720(&top, &temp, index));
        host.lua_object_destruct_00b67700(&temp);
    }

    const float gap = static_cast<float>(host.lua_get_number_00b66270(host.lua_get_by_name_00b67800(table, &temp,
                                                                                                    "buoyGap")));
    host.lua_object_destruct_00b67700(&temp);
    const float deviation = static_cast<float>(
        host.lua_get_number_00b66270(host.lua_get_by_name_00b67800(table, &temp, "buoyDev")));
    host.lua_object_destruct_00b67700(&temp);
    Object memory = host.operator_new_00bf681b(0x84);
    std::uint32_t shared_slot = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(memory));
    Object field = nullptr;
    if (memory != nullptr) {
        field = buoy_field_construct_004a5850(host, memory, at<Object>(manager, 0x8), box, name_vector,
                                              weight_vector, gap, deviation);
    }
    const bool has_id = !host.lua_is_nil_00b65fb0(host.lua_get_by_name_00b67800(table, &temp, "tempID"));
    host.lua_object_destruct_00b67700(&temp);
    if (has_id) {
        NativeString id;
        host.native_string_assign_0041e870(&id, host.lua_get_string_00b662b0(
                                                    host.lua_get_by_name_00b67800(table, &temp, "tempID")));
        *host.temp_map_subscript_0049cb60(manager + 0x34, &id) = field;
        shared_slot = id.length;
        release_native_string(host, id);
        host.lua_object_destruct_00b67700(&temp);
    }
    host.lua_object_destruct_00b67700(&element);
    host.lua_object_destruct_00b67700(&top);
    if (at<Object>(weight_vector, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(weight_vector, 0x4));
    weights[1] = weights[2] = weights[3] = 0;
    if (at<Object>(name_vector, 0x4) != nullptr) {
        host.element_range_destroy_00432050(at<Object>(name_vector, 0x4), at<Object>(name_vector, 0x8), name_vector,
                                            obj(shared_slot));
        host.crt_free_00bf65ac(at<Object>(name_vector, 0x4));
    }
    names[1] = names[2] = names[3] = 0;
    host.lua_object_destruct_00b67700(table);
}

// 004A5C60 __thiscall(manager, key, const float box[6] {min, max}), RET 8.
// Called from the unit-parts damage step (00935350). groups =
// 004A5750(manager+1Ch, &key); the first group whose [+4] is 3 is taken (none:
// return). For each member slot i < [group+24h]: the bit i of the vector<bool>
// at group+80h (begin iterator {group+80h, [group+88h], 0}, the image checks
// [+88h] <= [+8Ch], 0048D3B0 advances by i, then the checked bit-range test)
// is read; when it is clear: the unit [group+20h] gets its world matrix
// refreshed when byte unit+C8h is 0 (00414DB0 on [unit+3Ch] when set; then
// 00413920(unit+74h, &temp, parent+CCh) or unit+74h, copied by 004134F0 into
// unit+CCh; +C8h = 1, +10Ch = 0); the slot point (class = [unit+538h], the
// 40h-byte entries at [class+6F0h], point at entry+30h, checked index) is
// transformed by 004142E0 with unit+CCh; when 0085BF20(box min, box max,
// point, point) reports an overlap, the bit is set through 0048F930 (checked)
// and the member [group+74h][i] (checked) when set gets vtable +114h(0),
// 00926D90(member, 2) (kill) and the slot is cleared.
// Simplifications: the null-container checks on the stack iterator (its
// container is group+80h) cannot fail and are not modelled.
void unit_traffic_kill_in_box_004a5c60(Left8bHost& host, Object manager, std::uint32_t key, const float* box) {
    Object groups = host.traffic_registry_groups_004a5750(manager + 0x1c, &key);
    const std::int32_t count = static_cast<std::int32_t>(vector_size(groups, 4));
    Object group = nullptr;
    for (std::int32_t i = 0;; ++i) {
        if (i >= count) return;
        if (at<Object>(groups, 0x4) == nullptr || static_cast<std::uint32_t>(i) >= vector_size(groups, 4)) {
            host.invalid_parameter_00bf6713();
        }
        Object candidate = at<Object>(at<Object>(groups, 0x4), 4 * i);
        if (at<std::int32_t>(candidate, 0x4) == 3) {
            group = candidate;
            break;
        }
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(group, 0x24); ++i) {
        Object bits = group + 0x80;
        if (at<Object>(bits, 0x8) > at<Object>(bits, 0xc)) host.invalid_parameter_00bf6713();
        BitRef it{bits, reinterpret_cast<std::uint32_t*>(at<Object>(bits, 0x8)), 0};
        host.bit_iterator_advance_0048d3b0(&it, i);
        if (it.container == nullptr || it.word == nullptr) host.invalid_parameter_00bf6713();
        Object first_word = at<Object>(it.container, 0x8);
        if (first_word > at<Object>(it.container, 0xc)) host.invalid_parameter_00bf6713();
        const std::uint32_t position =
            static_cast<std::uint32_t>((reinterpret_cast<Object>(it.word) - first_word) / 4) * 32 + it.bit;
        if (position >= at<std::uint32_t>(it.container, 0x0)) host.invalid_parameter_00bf6713();
        if ((*it.word & (1u << (it.bit & 31))) != 0) continue;

        Object unit = at<Object>(group, 0x20);
        if (at<std::uint8_t>(unit, 0xc8) == 0) {
            if (at<Object>(unit, 0x3c) != nullptr) host.entity_pose_refresh_world_00414db0(at<Object>(unit, 0x3c));
            float product[16];
            const float* world = at<Object>(unit, 0x3c) != nullptr
                                     ? host.matrix_multiply_00413920(unit + 0x74, product,
                                                                     &at<float>(at<Object>(unit, 0x3c), 0xcc))
                                     : &at<float>(unit, 0x74);
            host.matrix_copy_004134f0(unit + 0xcc, world);
            at<std::uint8_t>(unit, 0xc8) = 1;
            at<std::uint8_t>(unit, 0x10c) = 0;
        }
        Object unit_class = at<Object>(at<Object>(group, 0x20), 0x538);
        Object slots = unit_class + 0x6ec;
        if (at<Object>(slots, 0x4) == nullptr || static_cast<std::uint32_t>(i) >= vector_size(slots, 0x40)) {
            host.invalid_parameter_00bf6713();
        }
        float point[3];
        host.vector3_transform_affine_point_004142e0(&at<float>(at<Object>(slots, 0x4), 0x40 * i + 0x30), point,
                                                     &at<float>(unit, 0xcc));
        if (!cc13_band0085::aabb_overlaps_0085bf20(box, box + 3, point, point)) continue;
        BitRef ref{};
        BitRef* r = host.bit_vector_at_0048f930(bits, &ref, static_cast<std::uint32_t>(i));
        if (r->container == nullptr || r->word == nullptr) host.invalid_parameter_00bf6713();
        Object ref_first = at<Object>(r->container, 0x8);
        if (ref_first > at<Object>(r->container, 0xc)) host.invalid_parameter_00bf6713();
        if (static_cast<std::uint32_t>((reinterpret_cast<Object>(r->word) - ref_first) / 4) * 32 + r->bit >=
            at<std::uint32_t>(r->container, 0x0)) {
            host.invalid_parameter_00bf6713();
        }
        *r->word |= 1u << (r->bit & 31);
        Object members = group + 0x70;
        if (at<Object>(members, 0x4) == nullptr || static_cast<std::uint32_t>(i) >= vector_size(members, 4)) {
            host.invalid_parameter_00bf6713();
        }
        Object* slot = reinterpret_cast<Object*>(at<Object>(members, 0x4) + 4 * i);
        if (*slot != nullptr) {
            host.vcall_set_group_114(*slot, nullptr);
            host.mission_entity_kill_00926d90(*slot, 2);
            *slot = nullptr;
        }
    }
}

// 004A6830 __thiscall(manager, unit), RET 4: creates the traffic attached to a
// unit (called from BSP_UnitInstance_SEntityInit). class = [unit+538h]. For
// each 50h-byte entry of the vector at class+6E0h (checked index): the 4Ch
// traffic record part is copied to a stack record (0049FA70); the entry's
// dword +4Ch indexes the unit's path vector at unit+117Ch (checked) and the
// record's path (+0) becomes that object + 170h (0 when the slot is null).
// When record end (+8) > start (+4) the record runs [0, count - 1] the other
// way round: start = 0, end = count - 1; otherwise end = 0, start = count - 1
// (count from [path+8]..[path+0Ch]; the image reads [path+8] without a null
// check). Units of kind 0Ch (vtable +5Ch(0Ch)) get operator new(1B4h) ->
// 004A66B0(memory, [manager+8], unit, &record) (a landing lane), others
// operator new(180h) -> 004A5470(...); 0 when the allocation fails. The object
// is appended to 004A5750(manager+1Ch, &unit) through the inlined push_back
// (00496110 when full, after the first <= last check), and the stack record is
// destructed (0049F910).
// After the loop, when the 1Ch-byte vector at class+700h is not empty: operator
// new(98h) -> 004A3610(memory, [manager+8], unit) (the ship idle actors group;
// 0 on failure; reconstructed below), appended through 00499410 to
// 004A5750(manager+1Ch, &unit).
// Simplifications: the loop re-reads the entry count each pass as the image
// does; the duplicate bounds check is kept once.
void unit_traffic_create_004a6830(Left8bHost& host, Object manager, Object unit) {
    Object unit_class = at<Object>(unit, 0x538);
    const std::uint32_t key = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(unit));
    for (std::uint32_t i = 0;; ++i) {
        Object entries = unit_class + 0x6dc;
        if (at<Object>(entries, 0x4) == nullptr || i >= vector_size(entries, 0x50)) break;
        Object entry = at<Object>(entries, 0x4) + 0x50 * i;
        alignas(4) std::uint8_t record[0x4c];
        Object local = record;
        cc13_left8::traffic_record_copy_0049fa70(host, local, entry);
        const std::uint32_t path_index = at<std::uint32_t>(entry, 0x4c);
        Object paths = unit + 0x1178;
        if (at<Object>(paths, 0x4) == nullptr || path_index >= vector_size(paths, 4)) {
            host.invalid_parameter_00bf6713();
        }
        Object path_owner = at<Object>(at<Object>(paths, 0x4), 4 * path_index);
        Object path = path_owner != nullptr ? path_owner + 0x170 : nullptr;
        at<Object>(local, 0x0) = path;
        const std::int32_t count = static_cast<std::int32_t>(vector_size(path + 0x4, 4));
        if (at<std::int32_t>(local, 0x8) > at<std::int32_t>(local, 0x4)) {
            at<std::int32_t>(local, 0x4) = 0;
            at<std::int32_t>(local, 0x8) = count - 1;
        } else {
            at<std::int32_t>(local, 0x8) = 0;
            at<std::int32_t>(local, 0x4) = count - 1;
        }
        Object created = nullptr;
        if (host.vcall_is_kind_of_5c(unit, 0xc)) {
            Object memory = host.operator_new_00bf681b(0x1b4);
            if (memory != nullptr) {
                created = landing_lane_construct_004a66b0(host, memory, at<Object>(manager, 0x8), unit, local);
            }
        } else {
            Object memory = host.operator_new_00bf681b(0x180);
            if (memory != nullptr) {
                created = unit_path_runtime_construct_004a5470(host, memory, at<Object>(manager, 0x8), unit, local);
            }
        }
        Object groups = host.traffic_registry_groups_004a5750(manager + 0x1c, &key);
        Object first = at<Object>(groups, 0x4);
        if (first != nullptr && vector_size(groups, 4) < static_cast<std::uint32_t>(at<Object>(groups, 0xc) - first) / 4) {
            at<Object>(at<Object>(groups, 0x8), 0x0) = created;
            at<Object>(groups, 0x8) += 4;
        } else {
            Object last = at<Object>(groups, 0x8);
            if (first > last) host.invalid_parameter_00bf6713();
            Object result[2] = {nullptr, nullptr};
            host.pointer_vector_insert_00496110(groups, reinterpret_cast<Object>(result), groups, last, &created);
        }
        cc13_left8::traffic_record_destruct_0049f910(host, local);
    }
    Object idle = unit_class + 0x6fc;
    if (at<Object>(idle, 0x4) != nullptr && vector_size(idle, 0x1c) != 0) {
        Object memory = host.operator_new_00bf681b(0x98);
        Object group = memory != nullptr
                           ? ship_idle_group_construct_004a3610(host, memory, at<Object>(manager, 0x8), unit)
                           : nullptr;
        host.pointer_vector_push_back_00499410(host.traffic_registry_groups_004a5750(manager + 0x1c, &key), &group);
    }
}

namespace {
// The four animation-name readers of 0049FC80 share this inlined shape: when
// the table is not nil, a key and a value Lua object are constructed
// (00B65F50 each), 00B67080 starts the iteration, and until 00B66420 reports
// the end each value's string (00B662B0) becomes a native string appended by
// 00450540 to `names`, then released; 00B67190 advances. The value, then the
// key, are destructed.
void read_animation_names(Left8bHost& host, LuaObject* table, Object names) {
    if (host.lua_is_nil_00b65fb0(table)) return;
    LuaObject key{};
    LuaObject value{};
    host.lua_object_construct_00b65f50(&key);
    host.lua_object_construct_00b65f50(&value);
    host.lua_iterate_first_00b67080(table, &key, &value);
    while (!host.lua_iterate_done_00b66420(table, &key)) {
        NativeString name;
        native_string_from_text(host, name, host.lua_get_string_00b662b0(&value));
        host.native_string_vector_append_00450540(names, &name);
        release_native_string(host, name);
        host.lua_iterate_next_00b67190(table, &key, &value);
    }
    host.lua_object_destruct_00b67700(&value);
    host.lua_object_destruct_00b67700(&key);
}
}  // namespace

// 0049FC80 __thiscall(type, class, LuaObject* row), RET 8, EAX = type. The
// soldier type with animation names (vtable 00CE691C), one row of
// LandingTroopClass[n].Classes. 0048DC40(type, class, row) builds the base;
// the four native-string vectors at +3Ch (run), +4Ch (fire), +5Ch (die) and
// +6Ch (idle) get first/last/end = 0, and the first three are cleared by
// 004954F0(vector, &result, vector, first, vector, last) after the image's
// first <= last checks (twice each, as emitted). Then row.AnimRun,
// row.AnimFire, row.AnimDie and row.AnimIdle (the first read straight into
// the table object, the others through a temporary assigned by 00B67690 and
// destructed) are read into their vectors; byte +38h = 1; the table object is
// destructed.
Object soldier_type_with_anims_construct_0049fc80(Left8bHost& host, Object type, Object soldier_class,
                                                  LuaObject* row) {
    host.soldier_type_construct_0048dc40(type, soldier_class, row);
    at<std::uint32_t>(type, 0x0) = kSoldierTypeAnimsVtable;
    for (std::size_t vector : {std::size_t{0x3c}, std::size_t{0x4c}, std::size_t{0x5c}, std::size_t{0x6c}}) {
        at<std::uint32_t>(type, vector + 0x4) = 0;
        at<std::uint32_t>(type, vector + 0x8) = 0;
        at<std::uint32_t>(type, vector + 0xc) = 0;
    }
    for (std::size_t offset : {std::size_t{0x3c}, std::size_t{0x4c}, std::size_t{0x5c}}) {
        Object vector = type + offset;
        Object last = at<Object>(vector, 0x8);
        if (at<Object>(vector, 0x4) > last) host.invalid_parameter_00bf6713();
        Object first = at<Object>(vector, 0x4);
        if (first > at<Object>(vector, 0x8)) host.invalid_parameter_00bf6713();
        std::uint32_t result[2] = {0, 0};
        host.string_vector_erase_range_004954f0(vector, reinterpret_cast<Object>(result), vector, first, vector,
                                                last);
    }
    LuaObject table{};
    LuaObject temp{};
    host.lua_get_by_name_00b67800(row, &table, "AnimRun");
    read_animation_names(host, &table, type + 0x3c);
    const struct {
        const char* name;
        std::size_t vector;
    } rest[3] = {{"AnimFire", 0x4c}, {"AnimDie", 0x5c}, {"AnimIdle", 0x6c}};
    for (const auto& entry : rest) {
        host.lua_object_assign_00b67690(&table, host.lua_get_by_name_00b67800(row, &temp, entry.name));
        host.lua_object_destruct_00b67700(&temp);
        read_animation_names(host, &table, type + entry.vector);
    }
    at<std::uint8_t>(type, 0x38) = 1;
    host.lua_object_destruct_00b67700(&table);
    return type;
}

// 004A2FC0 __thiscall(troop, float dt), RET 4: the landing-troop group tick
// (vtable 00CE66E0 slot 1). Nothing happens while byte [00E081C0] is 0.
// visible = 004B48E0([00E188A8], troop+9Ch); 004A0D20(troop); 0048AF00(troop);
// 0048A6D0(troop, dt); 0048B120(troop, dt). The three member lists (heads at
// +B8h, +C4h, +D0h; node: entity +0Ch, class +8, path time +14h, path +18h,
// state +34h) are walked; any node clears `empty`.
//   Not visible: an entity whose byte +5Ch is set gets vtable +118h().
//   Visible: a stack matrix starts as identity. State 5 copies the entity's
//   local matrix (+74h..+B0h). Otherwise 007B03C0(path, t, &row3, &row2, 1)
//   samples the path; row1 is the global up vector [00F8758C..00F87594],
//   except when the troop has a carrier entity (+DCh) and the state is 0:
//   then the carrier pose is refreshed when its byte +C8h is 0 (00414DB0),
//   its vtable +24Ch(&temp, 1) point (w = 1) is transformed by 00B62D10 with
//   carrier+CCh, and only row1.y becomes float(float(1 / w) * y). 0085DAD0
//   orthonormalises the matrix and row3.y = 0087FA20(landscape [+54h], row3.x,
//   row3.z) - [landscape+A8h].
//   Both: row3 (w = 1) is projected through 00B62D10 with troop+58h, the
//   screen point is {x, y, z} * float(1 / w); when 0048A010(entity, &screen)
//   accepts it, row3.y is re-sampled from the landscape as above, a class
//   without byte +38h takes the landscape normal 0087FB90 as row1 and the
//   matrix is orthonormalised again; the matrix becomes the entity's local
//   matrix, bytes +C8h and +10Ch are cleared, 0042ED50 invalidates every
//   child of [entity+48h] (next +44h), and vtable +D8h() is called.
// When no node was seen and byte +20h is set, 00491E70(manager
// [[00E188A8]+21D0h], ship, 1) removes the group, where ship = ([[+D8h]] or,
// when 0, [[+D8h]+8]) + 1E4h (0 when both are 0).
// The x87 orders are from the bytes (DE F1: 1 / w; DE CA and D8 C9 for the
// products). Products are evaluated in double before each float store.
// Simplifications: the node == head checks inside the walk cannot fail.
void landing_troop_tick_004a2fc0(Left8bHost& host, Object troop, float dt) {
    if (host.global_u8(kTroopsEnabled) == 0) return;
    Object game = obj(host.global_u32(kGame));
    const bool visible = cc13_left8::game_is_aabb_in_view_004b48e0(host, game, &at<float>(troop, 0x9c));
    host.landing_troop_update_004a0d20(troop);
    host.landing_troop_spawn_missing_members_0048af00(troop);
    host.landing_troop_tick_fake_effects_0048a6d0(troop, dt);
    host.landing_troop_tick_members_0048b120(troop, dt);
    bool empty = true;
    for (std::size_t list = 0xb8; list <= 0xd0; list += 0xc) {
        for (Object node = at<Object>(at<Object>(troop, list), 0x0); node != at<Object>(troop, list);
             node = at<Object>(node, 0x0)) {
            empty = false;
            Object entity = at<Object>(node, 0xc);
            if (!visible) {
                if (entity != nullptr && at<std::uint8_t>(entity, 0x5c) != 0) host.vcall_118(entity);
                continue;
            }
            float m[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
            if (at<std::int32_t>(node, 0x34) == 5) {
                for (std::size_t i = 0; i < 16; ++i) m[i] = at<float>(entity, 0x74 + 4 * i);
            } else {
                host.camera_path_sample_world_007b03c0(at<Object>(node, 0x18), at<float>(node, 0x14), &m[12], &m[8],
                                                       1);
                Object carrier = at<Object>(troop, 0xdc);
                if (carrier != nullptr && at<std::int32_t>(node, 0x34) == 0) {
                    if (at<std::uint8_t>(carrier, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(carrier);
                    float temp[3];
                    const float* c = host.vcall_24c(at<Object>(troop, 0xdc), temp, 1);
                    const float point[4] = {c[0], c[1], c[2], kOne};
                    float out[4];
                    const float* t = host.vector4_transform_00b62d10(out, point, &at<float>(carrier, 0xcc));
                    const float y = t[1];
                    const float w = t[3];
                    const float inverse = static_cast<float>(1.0 / static_cast<double>(w));
                    m[5] = static_cast<float>(static_cast<double>(inverse) * y);
                } else {
                    m[4] = host.global_f32(kUpX);
                    m[5] = host.global_f32(kUpY);
                    m[6] = host.global_f32(kUpZ);
                }
                host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(m));
                Object landscape = at<Object>(troop, 0x54);
                m[13] = static_cast<float>(static_cast<double>(host.landscape_forward_local_height_0087fa20(
                                               landscape, m[12], m[14])) -
                                           at<float>(landscape, 0xa8));
            }
            const float point[4] = {m[12], m[13], m[14], kOne};
            float out[4];
            const float* p = host.vector4_transform_00b62d10(out, point, &at<float>(troop, 0x58));
            const float projected[4] = {p[0], p[1], p[2], p[3]};
            const float inverse = static_cast<float>(1.0 / static_cast<double>(projected[3]));
            const float screen[3] = {static_cast<float>(static_cast<double>(projected[0]) * inverse),
                                     static_cast<float>(static_cast<double>(projected[1]) * inverse),
                                     static_cast<float>(static_cast<double>(inverse) * projected[2])};
            if (!host.soldier_entity_update_clip_visibility_0048a010(at<Object>(node, 0xc), screen)) continue;
            Object landscape = at<Object>(troop, 0x54);
            m[13] = static_cast<float>(
                static_cast<double>(host.landscape_forward_local_height_0087fa20(landscape, m[12], m[14])) -
                at<float>(landscape, 0xa8));
            if (at<std::uint8_t>(at<Object>(node, 0x8), 0x38) == 0) {
                float normal[3];
                const float* n =
                    host.landscape_forward_local_normal_0087fb90(at<Object>(troop, 0x54), normal, m[12], m[14]);
                m[4] = n[0];
                m[5] = n[1];
                m[6] = n[2];
                host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(m));
            }
            entity = at<Object>(node, 0xc);
            for (std::size_t i = 0; i < 16; ++i) at<float>(entity, 0x74 + 4 * i) = m[i];
            entity = at<Object>(node, 0xc);
            Object child = at<Object>(entity, 0x48);
            at<std::uint8_t>(entity, 0xc8) = 0;
            at<std::uint8_t>(entity, 0x10c) = 0;
            for (; child != nullptr; child = at<Object>(child, 0x44)) host.scene_node_invalidate_subtree_pose_0042ed50(child);
            host.vcall_d8(at<Object>(node, 0xc));
        }
    }
    if (!empty || at<std::uint8_t>(troop, 0x20) == 0) return;
    Object owner = at<Object>(troop, 0xd8);
    Object ship = at<Object>(owner, 0x0) != nullptr ? at<Object>(owner, 0x0) : at<Object>(owner, 0x8);
    const std::uint32_t key = ship != nullptr ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(ship + 0x1e4)) : 0;
    Object manager = at<Object>(obj(host.global_u32(kGame)), 0x21d0);
    cc13_left8::landing_traffic_remove_ship_00491e70(host, manager, key, true);
}

// 0049E560 __fastcall(ECX = traffic), RET: rebuilds the scene-traffic layer
// cells (tail-called by 0049F260 when [+0Ch] is set). 004814C0(traffic) resets
// the entity layers. With g = [00E188A8], A = g+711Ch, B = g+7128h (float3),
// axis1 = [00F87580..88], axis2 = [00F87598..A0] and k = -0.0f [00D7A208]:
//   +3Ch = A; d = float(A.z - B.z... namely [g+7124h] - [g+7130h]); +48h = A -
//   float(d * axis2) per component; +54h = B; e = float([g+7128h] - [g+711Ch]);
//   +60h = A + float(e * axis1). The boxes +6Ch/+78h = {A - 2, +60h + 2},
//   +84h/+90h = {+48h - 2, A + 2}, +9Ch/+A8h = {+48h - 2, B + 2}, +B4h/+C0h =
//   {B - 2, +60h + 2} (2.0 is the qword 00D7A308). The layer origins +CCh =
//   A, +D8h = +48h, +E4h = B, +F0h = +60h with directions +FCh = axis1, +108h =
//   axis2, +114h = k - axis1, +120h = k - axis2, and the spare rows +12Ch =
//   axis2, +138h = k - axis1, +144h = k - axis2, +150h = axis1. Extents: +15Ch
//   = +164h = d, +160h = +168h = e.
//   +38h = _ftol2(min(+15Ch, +160h) / [+30h] / [+34h]) (FCOMIP min; FDIV float,
//   FIDIV int).
// For each layer l = 0..3: count = _ftol2(extent[l] / [+30h]) at +17Ch + 4l;
// scale = float(extent[l] / count) at +16Ch + 4l; byte +20Ch + l = 0;
// 00495AE0(+18Ch + 10h*l, count, 0); 00495BA0(+1CCh + 10h*l, [+38h], box by
// value); 0049DF50(+210h + 14h*l, [+38h], 0). Then for j < [+38h]:
//   t1 = float(scale * float(count*(j+1)/n - 1)), t0 = float(scale *
//   float(count*j/n)) (integer divisions by n = [+38h]); p = origin + t*dir
//   (each product and sum stored as float); cell j of the box vector gets
//   min(p0, p1) - 2.0 at +0 and max(p0, p1) + 2.0 at +0Ch (checked index,
//   18h-byte cells; the image recomputes p0 and p1 for the max with the same
//   values); bit j of the layer's vector<bool> is cleared (begin iterator,
//   0048D3B0 advance, checked range test).
// Simplifications: the 18h-byte by-value fill box handed to 00495BA0 is
// uninitialised stack in the image (every cell it could fill is overwritten in
// the loop); it is passed as zeros here. The NaN behaviour of the min/max
// selects follows the FCOMIP/JBE forms. Arithmetic is float-rounded at every
// image store; sums and products are formed in double before each store.
void traffic_layers_rebuild_0049e560(Left8bHost& host, Object traffic) {
    host.scene_entity_layers_reset_all_004814c0(traffic);
    Object game = obj(host.global_u32(kGame));
    const auto f = [&](std::size_t offset) { return at<float>(obj(host.global_u32(kGame)), offset); };
    const auto round = [](double value) { return static_cast<float>(value); };
    at<float>(traffic, 0x3c) = at<float>(game, 0x711c);
    at<float>(traffic, 0x40) = at<float>(game, 0x7120);
    at<float>(traffic, 0x44) = at<float>(game, 0x7124);
    const float d = round(static_cast<double>(f(0x7124)) - f(0x7130));
    const float axis1[3] = {host.global_f32(kAxis1), host.global_f32(kAxis1 + 4), host.global_f32(kAxis1 + 8)};
    const float axis2[3] = {host.global_f32(kAxis2), host.global_f32(kAxis2 + 4), host.global_f32(kAxis2 + 8)};
    for (std::size_t i = 0; i < 3; ++i) {
        const float scaled = round(static_cast<double>(d) * axis2[i]);
        at<float>(traffic, 0x48 + 4 * i) = round(static_cast<double>(at<float>(traffic, 0x3c + 4 * i)) - scaled);
    }
    at<float>(traffic, 0x54) = f(0x7128);
    at<float>(traffic, 0x58) = f(0x712c);
    at<float>(traffic, 0x5c) = f(0x7130);
    const float e = round(static_cast<double>(f(0x7128)) - f(0x711c));
    for (std::size_t i = 0; i < 3; ++i) {
        const float scaled = round(static_cast<double>(e) * axis1[i]);
        at<float>(traffic, 0x60 + 4 * i) = round(static_cast<double>(at<float>(traffic, 0x3c + 4 * i)) + scaled);
    }
    // The four padded boxes: {lower source - 2, upper source + 2}.
    const std::size_t boxes[4][3] = {{0x6c, 0x3c, 0x60}, {0x84, 0x48, 0x3c}, {0x9c, 0x48, 0x54}, {0xb4, 0x54, 0x60}};
    for (const auto& box : boxes) {
        for (std::size_t i = 0; i < 3; ++i) {
            at<float>(traffic, box[0] + 4 * i) = round(static_cast<double>(at<float>(traffic, box[1] + 4 * i)) - kPad);
        }
        for (std::size_t i = 0; i < 3; ++i) {
            at<float>(traffic, box[0] + 0xc + 4 * i) =
                round(static_cast<double>(at<float>(traffic, box[2] + 4 * i)) + kPad);
        }
    }
    const float k = kNegativeZero;
    for (std::size_t i = 0; i < 3; ++i) {
        at<float>(traffic, 0xcc + 4 * i) = at<float>(traffic, 0x3c + 4 * i);
        at<float>(traffic, 0xfc + 4 * i) = axis1[i];
        at<float>(traffic, 0x12c + 4 * i) = axis2[i];
        at<float>(traffic, 0xd8 + 4 * i) = at<float>(traffic, 0x48 + 4 * i);
        at<float>(traffic, 0x108 + 4 * i) = axis2[i];
        at<float>(traffic, 0x138 + 4 * i) = k - axis1[i];
        at<float>(traffic, 0xe4 + 4 * i) = at<float>(traffic, 0x54 + 4 * i);
        at<float>(traffic, 0x114 + 4 * i) = k - axis1[i];
        at<float>(traffic, 0x144 + 4 * i) = k - axis2[i];
        at<float>(traffic, 0xf0 + 4 * i) = at<float>(traffic, 0x60 + 4 * i);
        at<float>(traffic, 0x120 + 4 * i) = k - axis2[i];
        at<float>(traffic, 0x150 + 4 * i) = axis1[i];
    }
    const float depth = round(static_cast<double>(f(0x7124)) - f(0x7130));
    at<float>(traffic, 0x164) = depth;
    at<float>(traffic, 0x15c) = depth;
    const float width = round(static_cast<double>(f(0x7128)) - f(0x711c));
    at<float>(traffic, 0x168) = width;
    at<float>(traffic, 0x160) = width;
    const float a = at<float>(traffic, 0x15c);
    const float b = at<float>(traffic, 0x160);
    const float smaller = b > a ? a : b;
    at<std::int32_t>(traffic, 0x38) = static_cast<std::int32_t>(
        static_cast<double>(smaller) / at<float>(traffic, 0x30) / at<std::int32_t>(traffic, 0x34));

    for (std::size_t layer = 0; layer < 4; ++layer) {
        const float extent = at<float>(traffic, 0x15c + 4 * layer);
        const std::int32_t count = static_cast<std::int32_t>(static_cast<double>(extent) / at<float>(traffic, 0x30));
        at<std::int32_t>(traffic, 0x17c + 4 * layer) = count;
        at<float>(traffic, 0x16c + 4 * layer) = round(static_cast<double>(extent) / count);
        at<std::uint8_t>(traffic, 0x20c + layer) = 0;
        host.dword_vector_resize_00495ae0(traffic + 0x18c + 0x10 * layer,
                                          static_cast<std::uint32_t>(at<std::int32_t>(traffic, 0x17c + 4 * layer)), 0);
        const float fill[6] = {0, 0, 0, 0, 0, 0};
        Object cells = traffic + 0x1cc + 0x10 * layer;
        host.box_vector_resize_00495ba0(cells, static_cast<std::uint32_t>(at<std::int32_t>(traffic, 0x38)), fill);
        Object bits = traffic + 0x210 + 0x14 * layer;
        host.bit_vector_resize_0049df50(bits, static_cast<std::uint32_t>(at<std::int32_t>(traffic, 0x38)), 0);
        const float* origin = &at<float>(traffic, 0xcc + 0xc * layer);
        const float* dir = origin + 0xc;
        for (std::int32_t j = 0; j < at<std::int32_t>(traffic, 0x38); ++j) {
            const std::int32_t n = at<std::int32_t>(traffic, 0x38);
            const std::int32_t cells_in_layer = at<std::int32_t>(traffic, 0x17c + 4 * layer);
            const float scale = at<float>(traffic, 0x16c + 4 * layer);
            const float a1 = static_cast<float>(cells_in_layer * (j + 1) / n - 1);
            const float a0 = static_cast<float>(cells_in_layer * j / n);
            const float t1 = round(static_cast<double>(scale) * a1);
            const float t0 = round(static_cast<double>(scale) * a0);
            float p1[3];
            float p0[3];
            for (std::size_t i = 0; i < 3; ++i) {
                p1[i] = round(static_cast<double>(origin[i]) + round(static_cast<double>(t1) * dir[i]));
                p0[i] = round(static_cast<double>(origin[i]) + round(static_cast<double>(t0) * dir[i]));
            }
            float low[3];
            float high[3];
            for (std::size_t i = 0; i < 3; ++i) {
                low[i] = p1[i] > p0[i] ? p0[i] : p1[i];
                high[i] = p0[i] > p1[i] ? p0[i] : p1[i];
            }
            if (at<Object>(cells, 0x4) == nullptr || static_cast<std::uint32_t>(j) >= vector_size(cells, 0x18)) {
                host.invalid_parameter_00bf6713();
            }
            for (std::size_t i = 0; i < 3; ++i) {
                at<float>(at<Object>(cells, 0x4), 0x18 * j + 4 * i) = round(static_cast<double>(low[i]) - kPad);
            }
            if (at<Object>(cells, 0x4) == nullptr || static_cast<std::uint32_t>(j) >= vector_size(cells, 0x18)) {
                host.invalid_parameter_00bf6713();
            }
            for (std::size_t i = 0; i < 3; ++i) {
                at<float>(at<Object>(cells, 0x4), 0x18 * j + 0xc + 4 * i) = round(static_cast<double>(high[i]) + kPad);
            }
            if (at<Object>(bits, 0x8) > at<Object>(bits, 0xc)) host.invalid_parameter_00bf6713();
            BitRef it{bits, reinterpret_cast<std::uint32_t*>(at<Object>(bits, 0x8)), 0};
            host.bit_iterator_advance_0048d3b0(&it, j);
            if (it.container == nullptr || it.word == nullptr) host.invalid_parameter_00bf6713();
            Object first_word = at<Object>(it.container, 0x8);
            if (first_word > at<Object>(it.container, 0xc)) host.invalid_parameter_00bf6713();
            if (static_cast<std::uint32_t>((reinterpret_cast<Object>(it.word) - first_word) / 4) * 32 + it.bit >=
                at<std::uint32_t>(it.container, 0x0)) {
                host.invalid_parameter_00bf6713();
            }
            *it.word &= ~(1u << (it.bit & 31));
        }
    }
}

namespace {
// The inlined push_back of a 4-byte element on a {proxy, first, last, end}
// vector: store when capacity remains, else the image's checked first <= last
// test and the out-of-line insert at last.
template <class T, class Insert>
void push_back4(Left8bHost& host, Object vector, const T& value, Insert insert) {
    Object first = at<Object>(vector, 0x4);
    Object last = at<Object>(vector, 0x8);
    if (first != nullptr && static_cast<std::uint32_t>(last - first) / 4 <
                                static_cast<std::uint32_t>(at<Object>(vector, 0xc) - first) / 4) {
        at<T>(last, 0x0) = value;
        at<Object>(vector, 0x8) = last + 4;
        return;
    }
    if (first > last) host.invalid_parameter_00bf6713();
    Object result[2] = {nullptr, nullptr};
    insert(vector, reinterpret_cast<Object>(result), vector, last, &value);
}

// A begin() iterator of the vector<bool> {count +0, proxy +4, first +8, last
// +0Ch} advanced by `index` through 0048D3B0, with the image's checks before
// the advance (first <= last) and after it (container and word set, first <=
// last, bit position < count).
BitRef checked_bit(Left8bHost& host, Object bits, std::int32_t index) {
    if (at<Object>(bits, 0x8) > at<Object>(bits, 0xc)) host.invalid_parameter_00bf6713();
    BitRef it{bits, reinterpret_cast<std::uint32_t*>(at<Object>(bits, 0x8)), 0};
    host.bit_iterator_advance_0048d3b0(&it, index);
    if (it.container == nullptr || it.word == nullptr) host.invalid_parameter_00bf6713();
    Object first_word = at<Object>(it.container, 0x8);
    if (first_word > at<Object>(it.container, 0xc)) host.invalid_parameter_00bf6713();
    if (static_cast<std::uint32_t>((reinterpret_cast<Object>(it.word) - first_word) / 4) * 32 + it.bit >=
        at<std::uint32_t>(it.container, 0x0)) {
        host.invalid_parameter_00bf6713();
    }
    return it;
}

void check_index(Left8bHost& host, Object vector, std::uint32_t index, std::uint32_t stride) {
    if (at<Object>(vector, 0x4) == nullptr || index >= vector_size(vector, stride)) host.invalid_parameter_00bf6713();
}
}  // namespace

// 004A3610 __thiscall(group, parent, unit), RET 8, EAX = group (frame aligned
// to 8 through EBP). The ship idle actors group (vtable 00CE69D4, 98h bytes),
// created by 004A6830 when the unit class has idle-actor slots (class+700h,
// 1Ch-byte entries {map<uint32, float> kinds +0, map<string, float> anims
// +0Ch}). 00496920(group, parent); the containers +28h (vector<bool>), +40h,
// +50h, +60h (native strings), +70h and +80h (vector<bool>) are zeroed; +4 = 3
// (the group kind 004A5C60 looks for); +20h = unit; n = the slot count at +24h;
// 0049DF50(+28h, n, 0); byte +3Ch = 1; 00495770(+40h, n, 0); 004958C0(+50h,
// n, 0); 0049E050(+60h, n, empty string); 00495A20(+70h, n, 0); 0049DF50(+80h,
// n, 0); byte +94h = 0. For each slot i (entry checked twice, as emitted):
//   For each kind (key at node+0Ch, weight node+10h): soldier = no
//   LandVehicleclasses name for the key; the flag is appended to a local
//   vector<bool> L (end iterator built from L's first word and advanced by
//   its count, then 0049E150(L, &result, where, flag)); the visual class
//   (0 for soldiers, else 0047B3C0(00964790(key, 1))) and the soldier class
//   (004B1400 of the SoldierTypes name for soldiers, else 0) are pushed on
//   local vectors V1 / V2 (inlined, 00495830 / 00495980 when full), the
//   weight on V3 (0045A120 when full); 0047D090 advances. 004809F0(V3).
//   For each anim entry: 00450540 appends the name to V4, the weight goes on
//   V5 (0045A120 when full); 00443880 advances. 004809F0(V5).
//   H = {word[unit+174h], 0} (00441F30 insert at begin, then the inlined
//   push_back). pick = 00480A90(00484660(H), &V3); soldier = L[pick]. Bit i
//   of +28h is set or cleared to it; byte +3Ch &= it; +40h[i] = soldier ? 0 :
//   V1[pick]; +50h[i] = soldier ? V2[pick] : 0. H[1] = 1; pick2 =
//   00480A90(00484660(H), &V5); +60h[i] = V4[pick2] (inlined assign when the
//   strings differ: 0041DD40(dst, length, 1), memcpy of length bytes when
//   non-empty). The locals are freed (H, V5, V4 with each string returned to
//   the pool, V3, V2, V1, L's words).
// The picks are deterministic hashes of the unit id; the shared RNG 00BD2F10
// is not used.
// Simplifications: the loop-internal node == head checks and container
// self-checks cannot fail and are not modelled; the bool argument of 0049E150
// is the whole stack dword in the image (only its low byte is the flag).
Object ship_idle_group_construct_004a3610(Left8bHost& host, Object group, Object parent, Object unit) {
    cc13_left8::list_node_construct_00496920(host, group, parent);
    at<std::uint32_t>(group, 0x0) = kShipIdleGroupVtable;
    at<std::uint32_t>(group, 0x28) = 0;
    for (std::size_t offset : {std::size_t{0x30}, std::size_t{0x34}, std::size_t{0x38}, std::size_t{0x44},
                               std::size_t{0x48}, std::size_t{0x4c}, std::size_t{0x54}, std::size_t{0x58},
                               std::size_t{0x5c}, std::size_t{0x64}, std::size_t{0x68}, std::size_t{0x6c},
                               std::size_t{0x74}, std::size_t{0x78}, std::size_t{0x7c}, std::size_t{0x80},
                               std::size_t{0x88}, std::size_t{0x8c}, std::size_t{0x90}}) {
        at<std::uint32_t>(group, offset) = 0;
    }
    at<std::uint32_t>(group, 0x4) = 3;
    at<Object>(group, 0x20) = unit;
    Object slots = at<Object>(unit, 0x538) + 0x6fc;
    const std::uint32_t n = vector_size(slots, 0x1c);
    at<std::uint32_t>(group, 0x24) = n;
    host.bit_vector_resize_0049df50(group + 0x28, n, 0);
    at<std::uint8_t>(group, 0x3c) = 1;
    host.dword_vector_resize_00495770(group + 0x40, at<std::uint32_t>(group, 0x24), 0);
    host.dword_vector_resize_004958c0(group + 0x50, at<std::uint32_t>(group, 0x24), 0);
    const std::uint32_t empty_string[2] = {0, 0};
    host.native_string_vector_resize_0049e050(group + 0x60, at<std::uint32_t>(group, 0x24), empty_string);
    host.dword_vector_resize_00495a20(group + 0x70, at<std::uint32_t>(group, 0x24), 0);
    host.bit_vector_resize_0049df50(group + 0x80, at<std::uint32_t>(group, 0x24), 0);
    at<std::uint8_t>(group, 0x94) = 0;
    const std::int32_t zero = 0;

    for (std::int32_t i = 0; i < at<std::int32_t>(group, 0x24); ++i) {
        Object entries = at<Object>(unit, 0x538) + 0x6fc;
        check_index(host, entries, static_cast<std::uint32_t>(i), 0x1c);
        Object entry = at<Object>(entries, 0x4) + 0x1c * i;
        entries = at<Object>(unit, 0x538) + 0x6fc;
        check_index(host, entries, static_cast<std::uint32_t>(i), 0x1c);
        Object anims = at<Object>(entries, 0x4) + 0x1c * i + 0xc;

        std::uint32_t bits_l[5] = {0, 0, 0, 0, 0};  // vector<bool> {count, proxy, first, last, end}
        std::uint32_t visuals[4] = {0, 0, 0, 0};
        std::uint32_t soldiers[4] = {0, 0, 0, 0};
        std::uint32_t kind_weights[4] = {0, 0, 0, 0};
        std::uint32_t names[4] = {0, 0, 0, 0};
        std::uint32_t anim_weights[4] = {0, 0, 0, 0};
        Object l = reinterpret_cast<Object>(bits_l);
        Object v1 = reinterpret_cast<Object>(visuals);
        Object v2 = reinterpret_cast<Object>(soldiers);
        Object v3 = reinterpret_cast<Object>(kind_weights);
        Object v4 = reinterpret_cast<Object>(names);
        Object v5 = reinterpret_cast<Object>(anim_weights);

        Object kinds[2] = {entry, at<Object>(at<Object>(entry, 0x4), 0x0)};
        while (kinds[1] != at<Object>(entry, 0x4)) {
            Object node = kinds[1];
            const char* vehicle = host.enum_find_name_by_value_008f0d80(
                host.enum_table_read_word_0048e960(obj(host.global_u32(kEnumTables)), "LandVehicleclasses"),
                at<std::uint32_t>(node, 0xc));
            const bool soldier = vehicle == nullptr;
            Object l_first = at<Object>(l, 0x8);
            if (l_first > at<Object>(l, 0xc)) host.invalid_parameter_00bf6713();
            BitRef where{l, reinterpret_cast<std::uint32_t*>(l_first), 0};
            if (at<std::uint32_t>(l, 0x0) != 0) {
                host.bit_iterator_advance_0048d3b0(&where, static_cast<std::int32_t>(at<std::uint32_t>(l, 0x0)));
            }
            std::uint32_t inserted[3] = {0, 0, 0};
            host.bit_vector_insert_0049e150(l, reinterpret_cast<Object>(inserted), where, soldier);
            Object visual = nullptr;
            if (!soldier) {
                visual = host.vehicle_visual_class_get_or_create_0047b3c0(
                    host.vehicle_class_get_or_create_00964790(at<std::uint32_t>(node, 0xc), true));
            }
            push_back4(host, v1, visual, [&](Object v, Object r, Object c, Object w, const Object* x) {
                host.pointer_vector_insert_00495830(v, r, c, w, x);
            });
            Object soldier_class = nullptr;
            NativeString name;
            bool name_built = false;
            if (soldier) {
                const char* text = host.enum_find_name_by_value_008f0d80(
                    host.enum_table_read_word_0048e960(obj(host.global_u32(kEnumTables)), "SoldierTypes"),
                    at<std::uint32_t>(node, 0xc));
                native_string_from_text(host, name, text);
                name_built = true;
                soldier_class = cc13_left8::soldier_class_get_or_load_004b1400(host, &name);
            }
            push_back4(host, v2, soldier_class, [&](Object v, Object r, Object c, Object w, const Object* x) {
                host.pointer_vector_insert_00495980(v, r, c, w, x);
            });
            if (name_built) release_native_string(host, name);
            push_back4(host, v3, at<float>(node, 0x10), [&](Object v, Object r, Object c, Object w, const float* x) {
                host.float_vector_insert_0045a120(v, r, c, w, x);
            });
            host.tree_iterator_increment_0047d090(reinterpret_cast<Object>(kinds));
        }
        host.weights_normalize_004809f0(v3);

        Object anim_iterator[2] = {anims, at<Object>(at<Object>(anims, 0x4), 0x0)};
        while (anim_iterator[1] != at<Object>(anims, 0x4)) {
            Object node = anim_iterator[1];
            host.native_string_vector_append_00450540(v4, node + 0xc);
            push_back4(host, v5, at<float>(node, 0x14), [&](Object v, Object r, Object c, Object w, const float* x) {
                host.float_vector_insert_0045a120(v, r, c, w, x);
            });
            host.string_float_map_iterator_increment_00443880(reinterpret_cast<Object>(anim_iterator));
        }
        host.weights_normalize_004809f0(v5);

        std::uint32_t hash_input[4] = {0, 0, 0, 0};
        Object h = reinterpret_cast<Object>(hash_input);
        const std::int32_t id = at<std::uint16_t>(unit, 0x174);
        std::uint32_t result[2] = {0, 0};
        host.int_vector_insert_00441f30(h, reinterpret_cast<Object>(result), h, nullptr, &id);
        push_back4(host, h, zero, [&](Object v, Object r, Object c, Object w, const std::int32_t* x) {
            host.int_vector_insert_00441f30(v, r, c, w, x);
        });
        const std::uint32_t pick = host.weighted_pick_00480a90(host.int_vector_hash_00484660(h), v3);
        const BitRef chosen = checked_bit(host, l, static_cast<std::int32_t>(pick));
        const bool soldier = (*chosen.word & (1u << (chosen.bit & 31))) != 0;
        const BitRef slot_bit = checked_bit(host, group + 0x28, i);
        if (soldier) {
            *slot_bit.word |= 1u << (slot_bit.bit & 31);
        } else {
            *slot_bit.word &= ~(1u << (slot_bit.bit & 31));
        }
        at<std::uint8_t>(group, 0x3c) &= static_cast<std::uint8_t>(soldier);
        Object visual = nullptr;
        if (!soldier) {
            check_index(host, v1, pick, 4);
            visual = at<Object>(at<Object>(v1, 0x4), 4 * pick);
        }
        check_index(host, group + 0x40, static_cast<std::uint32_t>(i), 4);
        at<Object>(at<Object>(group, 0x44), 4 * i) = visual;
        Object soldier_class = nullptr;
        if (soldier) {
            check_index(host, v2, pick, 4);
            soldier_class = at<Object>(at<Object>(v2, 0x4), 4 * pick);
        }
        check_index(host, group + 0x50, static_cast<std::uint32_t>(i), 4);
        at<Object>(at<Object>(group, 0x54), 4 * i) = soldier_class;
        if (at<Object>(h, 0x4) == nullptr || vector_size(h, 4) <= 1) host.invalid_parameter_00bf6713();
        at<std::int32_t>(at<Object>(h, 0x4), 0x4) = 1;
        const std::uint32_t pick2 = host.weighted_pick_00480a90(host.int_vector_hash_00484660(h), v5);
        check_index(host, v4, pick2, 8);
        Object source = at<Object>(v4, 0x4) + 8 * pick2;
        check_index(host, group + 0x60, static_cast<std::uint32_t>(i), 8);
        Object target = at<Object>(group, 0x64) + 8 * i;
        if (target != source) {
            host.native_string_resize_0041dd40(target, at<std::uint32_t>(source, 0x0), 1);
            if (at<std::uint32_t>(source, 0x0) != 0) {
                std::memcpy(at<char*>(target, 0x4), at<char*>(source, 0x4), at<std::uint32_t>(target, 0x0));
            }
        }

        if (at<Object>(h, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(h, 0x4));
        if (at<Object>(v5, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v5, 0x4));
        if (at<Object>(v4, 0x4) != nullptr) {
            for (Object s = at<Object>(v4, 0x4); s != at<Object>(v4, 0x8); s += 8) {
                if (at<char*>(s, 0x4) != nullptr) {
                    host.string_pool_return_00bd1510(host.string_pool_00419cc0(), at<char*>(s, 0x4),
                                                     at<std::uint32_t>(s, 0x0) + 1, 1);
                }
            }
            host.crt_free_00bf65ac(at<Object>(v4, 0x4));
        }
        if (at<Object>(v3, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v3, 0x4));
        if (at<Object>(v2, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v2, 0x4));
        if (at<Object>(v1, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v1, 0x4));
        bits_l[0] = 0;
        if (at<Object>(l, 0x8) != nullptr) host.crt_free_00bf65ac(at<Object>(l, 0x8));
    }
    return group;
}

// 0049A760 __fastcall(ECX = spawner), RET: spawns the debris pieces of a
// plane once (caller BSP_Plane_HandleStateMessageKinds 007D0B80; the name is
// a hypothesis). Returns at once when byte +49h is set; otherwise sets it and
// adds 1 to the dword [00E186E8], then returns when the entity [+44h] is 0.
// The entity pose is refreshed when its byte +C8h is 0 (00414DB0); its vtable
// +34h(&position) gives the spawn origin and 0047B880(entity) selects the
// table set (true: weights 00E1876C, limits [00E18790], spread [00E18748],
// count range [00E18758]..[00E18754]; false: 00E1879C, [00E187B0],
// [00E18744], [00E18750]..[00E1874C]). A local per-class counter vector (the
// class list is the vector at 00E18760) is sized by 00492210 and zeroed.
// count = _ftol2(00BD2F10(0, float(lo), float(hi + 1))); the 54h-byte record
// vector at +20h grows by count (00495D40). For each new record:
//   +50h = 00480A90(seed = the address spawner + 0Dh, weights), clamped to
//   the class range, then advanced (wrapping) while the class counter has
//   reached its limit; the counter is incremented; +4 = the class, +0 = 0,
//   byte +4Ch = 1.
//   r = 00BD2F10(0, 0, spread); d = 00439950 random unit direction; v =
//   {r*d.x, float(d.y*r) * 0.1, r*d.z}; +8 = (v.z*row2 + v.y*row1) + v.x*row0
//   + row3 of the entity world matrix (+CCh).
//   +20h..+28h = 00BD2F10(-pi, pi) x3; +44h = 00BD2F10(class+28h, +2Ch);
//   three 00BD2F10(-10, 10) draws give signs s1..s3 (-1, 0 or 1); +2Ch =
//   draw(class+48h, +4Ch) * s3, +30h = draw * s2, +34h = draw * s1;
//   +38h..+40h = draw(class+50h, +54h) x3; +48h = draw(class+40h, +44h);
//   dist = draw(class+30h, +34h), yaw = draw(class+38h, +3Ch), angle =
//   draw(-pi, pi); w = sin(angle)*row0 + cos(angle)*row1; dir = sin(yaw)*w +
//   cos(yaw)*row2; +14h = origin + dist*dir.
// The counter vector is freed (00BF65AC).
// RNG: draws from the shared generator 00BD2F10 (stream 0) 18 times per
// record plus once for the count, and 00439950 draws a direction; the order
// above is the image's.
// Simplifications: sin/cos come from the CRT in double and are rounded to
// float, where the image uses FSIN/FCOS on the float input (last-bit
// differences at most); products and sums are float-rounded at each image
// store. The 54h-byte fill value passed by value to 00495D40 is uninitialised
// stack in the image and is not modelled (every field of each new record is
// written). The counter-limit search loops forever when every class is full,
// as the image does.
void plane_debris_spawn_0049a760(Left8bHost& host, Object spawner) {
    if (at<std::uint8_t>(spawner, 0x49) != 0) return;
    at<std::uint8_t>(spawner, 0x49) = 1;
    host.global_add_i32(kDebrisSpawnCount, 1);
    Object entity = at<Object>(spawner, 0x44);
    if (entity == nullptr) return;
    if (at<std::uint8_t>(entity, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float* m = &at<float>(entity, 0xcc);
    float origin[3];
    host.vcall_34(entity, origin);
    const bool small = host.unit_uses_small_turn_roll_limit_0047b880(entity);
    Object classes = host.image_address(kDebrisClasses);
    const std::int32_t types = static_cast<std::int32_t>(vector_size(classes, 4));
    std::uint32_t counts_storage[4] = {0, 0, 0, 0};
    Object counts = reinterpret_cast<Object>(counts_storage);
    host.dword_vector_resize_00492210(counts, static_cast<std::uint32_t>(types), 0);
    for (std::int32_t k = 0; k < types; ++k) {
        check_index(host, counts, static_cast<std::uint32_t>(k), 4);
        at<std::int32_t>(at<Object>(counts, 0x4), 4 * k) = 0;
    }
    const std::int32_t lo = static_cast<std::int32_t>(host.global_u32(small ? kDebrisCountLoSmall : kDebrisCountLo));
    const std::int32_t hi = static_cast<std::int32_t>(host.global_u32(small ? kDebrisCountHiSmall : kDebrisCountHi));
    const std::int32_t added = static_cast<std::int32_t>(
        host.random_uniform_00bd2f10(0, static_cast<float>(lo), static_cast<float>(hi + 1)));
    Object records = spawner + 0x20;
    const std::int32_t existing = static_cast<std::int32_t>(vector_size(records, 0x54));
    const std::int32_t total = added + existing;
    host.record54_vector_resize_00495d40(records, static_cast<std::uint32_t>(total));
    const std::uint32_t seed = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(spawner + 0xd));
    Object weights = host.image_address(small ? kDebrisWeightsSmall : kDebrisWeights);
    Object limits = host.image_address(small ? kDebrisLimitsSmall : kDebrisLimits);
    for (std::int32_t k = existing; k < total; ++k) {
        check_index(host, records, static_cast<std::uint32_t>(k), 0x54);
        Object record = at<Object>(records, 0x4) + 0x54 * k;
        std::int32_t pick = static_cast<std::int32_t>(host.weighted_pick_00480a90(seed, weights));
        at<std::int32_t>(record, 0x50) = pick;
        if (pick < 0) {
            pick = 0;
        } else if (pick > types - 1) {
            pick = types - 1;
        }
        at<std::int32_t>(record, 0x50) = pick;
        for (;;) {
            const std::uint32_t index = at<std::uint32_t>(record, 0x50);
            check_index(host, counts, index, 4);
            check_index(host, limits, index, 4);
            if (at<std::int32_t>(at<Object>(counts, 0x4), 4 * index) <
                at<std::int32_t>(at<Object>(limits, 0x4), 4 * index)) {
                break;
            }
            at<std::int32_t>(record, 0x50) += 1;
            if (at<std::int32_t>(record, 0x50) >= types) at<std::int32_t>(record, 0x50) = 0;
        }
        const std::uint32_t chosen = at<std::uint32_t>(record, 0x50);
        check_index(host, counts, chosen, 4);
        at<std::int32_t>(at<Object>(counts, 0x4), 4 * chosen) += 1;
        check_index(host, classes, chosen, 4);
        at<Object>(record, 0x4) = at<Object>(at<Object>(classes, 0x4), 4 * chosen);
        at<std::uint32_t>(record, 0x0) = 0;
        at<std::uint8_t>(record, 0x4c) = 1;

        const float spread = host.global_f32(small ? kDebrisSpreadSmall : kDebrisSpread);
        const float r = host.random_uniform_00bd2f10(0, 0.0f, spread);
        float direction[3];
        const float* d = host.random_unit_direction_00439950(direction);
        const auto mul = [](float a, float b) { return static_cast<float>(static_cast<double>(a) * b); };
        const auto add = [](float a, float b) { return static_cast<float>(static_cast<double>(a) + b); };
        at<float>(record, 0x8) = mul(r, d[0]);
        at<float>(record, 0xc) = mul(d[1], r);
        at<float>(record, 0x10) = mul(r, d[2]);
        at<float>(record, 0xc) = static_cast<float>(static_cast<double>(at<float>(record, 0xc)) * kBuoyGapGrow);
        const float vx = at<float>(record, 0x8);
        const float vy = at<float>(record, 0xc);
        const float vz = at<float>(record, 0x10);
        const float a[3] = {mul(m[0], vx), mul(m[1], vx), mul(m[2], vx)};
        const float b[3] = {mul(m[4], vy), mul(m[5], vy), mul(m[6], vy)};
        const float c[3] = {mul(vz, m[8]), mul(m[9], vz), mul(vz, m[10])};
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x8 + 4 * i) = add(add(c[i], b[i]), a[i]);
        for (std::size_t i = 0; i < 3; ++i) {
            at<float>(record, 0x8 + 4 * i) = add(m[12 + i], at<float>(record, 0x8 + 4 * i));
        }
        float spin[3];
        for (float& value : spin) value = host.random_uniform_00bd2f10(0, -kPi, kPi);
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x20 + 4 * i) = spin[i];
        Object debris_class = at<Object>(record, 0x4);
        at<float>(record, 0x44) =
            host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x28), at<float>(debris_class, 0x2c));
        const float r1 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const float r2 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const float r3 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const std::int32_t s1 = 0.0f > r1 ? -1 : (r1 > 0.0f ? 1 : 0);
        const std::int32_t s2 = 0.0f > r2 ? -1 : (r2 > 0.0f ? 1 : 0);
        const std::int32_t s3 = 0.0f > r3 ? -1 : (r3 > 0.0f ? 1 : 0);
        float turn[3];
        const std::int32_t signs[3] = {s3, s2, s1};
        for (std::size_t i = 0; i < 3; ++i) {
            debris_class = at<Object>(record, 0x4);
            turn[i] = static_cast<float>(
                static_cast<double>(host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x48),
                                                                 at<float>(debris_class, 0x4c))) *
                signs[i]);
        }
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x2c + 4 * i) = turn[i];
        float scale[3];
        for (float& value : scale) {
            debris_class = at<Object>(record, 0x4);
            value = host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x50), at<float>(debris_class, 0x54));
        }
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x38 + 4 * i) = scale[i];
        debris_class = at<Object>(record, 0x4);
        at<float>(record, 0x48) =
            host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x40), at<float>(debris_class, 0x44));
        debris_class = at<Object>(record, 0x4);
        const float dist =
            host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x30), at<float>(debris_class, 0x34));
        debris_class = at<Object>(record, 0x4);
        const float yaw = host.random_uniform_00bd2f10(0, at<float>(debris_class, 0x38), at<float>(debris_class, 0x3c));
        const float angle = host.random_uniform_00bd2f10(0, -kPi, kPi);
        const float cos_angle = static_cast<float>(std::cos(static_cast<double>(angle)));
        const float u[3] = {mul(cos_angle, m[4]), mul(m[5], cos_angle), mul(cos_angle, m[6])};
        const float sin_angle = static_cast<float>(std::sin(static_cast<double>(angle)));
        const float v[3] = {mul(sin_angle, m[0]), mul(m[1], sin_angle), mul(sin_angle, m[2])};
        const float w[3] = {add(v[0], u[0]), add(v[1], u[1]), add(v[2], u[2])};
        const float cos_yaw = static_cast<float>(std::cos(static_cast<double>(yaw)));
        const float p[3] = {mul(m[8], cos_yaw), mul(m[9], cos_yaw), mul(cos_yaw, m[10])};
        const float sin_yaw = static_cast<float>(std::sin(static_cast<double>(yaw)));
        const float q[3] = {mul(sin_yaw, w[0]), mul(w[1], sin_yaw), mul(sin_yaw, w[2])};
        const float dir[3] = {add(q[0], p[0]), add(q[1], p[1]), add(q[2], p[2])};
        const float offset[3] = {mul(dir[0], dist), mul(dir[1], dist), mul(dist, dir[2])};
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x14 + 4 * i) = add(origin[i], offset[i]);
    }
    if (at<Object>(counts, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(counts, 0x4));
}

// 00497BB0 __thiscall(lane, float time, bool force_visible), RET 8 (EBP frame
// aligned to 8): the landing-lane formation update (caller 0049A410). With
// s = [+184h] and the progress t = max(0, (time - s) / ([+190h] - s)) (FCOMIP;
// a NaN gives 0): spacing = [+38h]*(1-t) + [+194h]*t, lateral = [+198h]*t +
// [+3Ch]*(1-t), amplitude = t*[+48h]*[+44h]*[+130h] (each one float store).
// One member index counter and the iterators of the skip list +78h and the
// member list +6Ch run across all rows. For each row r < [+30h]:
//   M = identity; 007B04C0(path, time, &pos, 0, 0); off = lateral + the half
//   extents +138h..+140h; visible = force_visible or 004B48E0(game, {pos -
//   off, pos + off}); when visible, 007B03C0(path, time, &M.row3, &M.row2,
//   0), M.row1 = the up vector [00F8758C..94], 0085DAD0(M).
//   For each column k < [+34h]: when the skip-list node holds the current
//   index, the node advances and the column is skipped. Otherwise, when
//   visible: M2 = M; with [+34h] > 1, f = float((k - ([+34h] - 1.0) * 0.5) *
//   lateral) and M2.row3 += f * M.row0; when the landscape [+D8h] is set and
//   time > s, the position wobbles: a_j = float(index * c_j + time * k_j)
//   (qword constants 42.45/0.27f, 97.76/0.32f, 54.23/0.29f, 83.87/0.34f),
//   row3.x += sin(a1) * amplitude * sin(a2), row3.z += sin(a3) * amplitude *
//   sin(a4), row3.y = 0087FA20(landscape, x, z) - [landscape+A8h], and for
//   vehicles (byte +DCh clear) row1 = 0087FB90 normal and 0085DAD0(M2);
//   soldiers (byte +DCh set) are offset by (index % 2) * 0.3f - 0.15f in x and
//   z. Then the next member is taken from +6Ch, or, at the list end, created:
//   class = 00480A90(index, +100h) picks from the soldier (+F0h) or vehicle
//   (+E0h) class vector, member = class vtable +8(); vtable +98h(
//   [pathentity+3Ch], [pathentity+30h], &identity) (007ACA30 twice);
//   appended to +6Ch (00485920, 0048FFB0, links); vtable +114h(lane), +A0h();
//   soldiers with animation names (+110h) start one picked by 00480A90(index,
//   +120h) through 004B0F80(member, name, 1, 0); the member iterator is the
//   list end again. Visible members get the M2 local matrix (a new member
//   first copies it, then, like an existing member with byte +5Ch clear,
//   calls vtable +11Ch() and copies it again; an existing member with +5Ch
//   set copies it with 004134F0), clear bytes +C8h/+10Ch, invalidate their
//   children (0042ED50) and call vtable +D8h(); hidden members with byte +5Ch
//   set call vtable +118h(). The index advances with k.
//   time = float(time - spacing).
// The x87 orders are from the bytes (DE EA, DE F9, DE E1, D8 CA, DE CA, DE E5,
// DE E9, D8 C9, D8 C1). sin is the CRT's in double rounded to float where the
// image uses FSIN on the float angle. No RNG: the picks hash the index.
// Simplifications: the self-checks of the list iterators cannot fail and are
// not modelled; products/sums are evaluated in double and rounded where the
// image stores a float.
void landing_lane_update_formation_00497bb0(Left8bHost& host, Object lane, float time, bool force_visible) {
    const float start = at<float>(lane, 0x184);
    float t = static_cast<float>((static_cast<double>(time) - start) /
                                 (static_cast<double>(at<float>(lane, 0x190)) - start));
    t = t > 0.0f ? t : 0.0f;
    const double inverse = 1.0 - t;
    const float spacing = static_cast<float>(static_cast<double>(at<float>(lane, 0x194)) * t +
                                             static_cast<double>(at<float>(lane, 0x38)) * inverse);
    const float lateral = static_cast<float>(static_cast<double>(at<float>(lane, 0x198)) * t +
                                             static_cast<double>(at<float>(lane, 0x3c)) * inverse);
    const float amplitude = static_cast<float>(static_cast<double>(t) * at<float>(lane, 0x48) *
                                               at<float>(lane, 0x44) * at<float>(lane, 0x130));
    Object skip = at<Object>(at<Object>(lane, 0x7c), 0x0);
    Object member_node = at<Object>(at<Object>(lane, 0x70), 0x0);
    std::int32_t index = 0;
    const auto mul = [](float a, float b) { return static_cast<float>(static_cast<double>(a) * b); };
    const auto add = [](float a, float b) { return static_cast<float>(static_cast<double>(a) + b); };
    for (std::int32_t row = 0; row < at<std::int32_t>(lane, 0x30); ++row) {
        float m[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
        float pos[3];
        host.camera_path_sample_point_007b04c0(at<Object>(lane, 0x20), time, pos, 0, 0);
        const float off[3] = {add(lateral, at<float>(lane, 0x138)), add(at<float>(lane, 0x13c), lateral),
                              add(lateral, at<float>(lane, 0x140))};
        const float box[6] = {static_cast<float>(static_cast<double>(pos[0]) - off[0]),
                              static_cast<float>(static_cast<double>(pos[1]) - off[1]),
                              static_cast<float>(static_cast<double>(pos[2]) - off[2]),
                              add(off[0], pos[0]), add(off[1], pos[1]), add(off[2], pos[2])};
        bool visible = true;
        if (!force_visible) {
            visible = cc13_left8::game_is_aabb_in_view_004b48e0(host, obj(host.global_u32(kGame)), box);
        }
        if (visible) {
            host.camera_path_sample_world_007b03c0(at<Object>(lane, 0x20), time, &m[12], &m[8], 0);
            m[4] = host.global_f32(kUpX);
            m[5] = host.global_f32(kUpY);
            m[6] = host.global_f32(kUpZ);
            host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(m));
        }
        for (std::int32_t k = 0; k < at<std::int32_t>(lane, 0x34); ++k, ++index) {
            if (skip != at<Object>(lane, 0x7c) && at<std::int32_t>(skip, 0x8) == index) {
                skip = at<Object>(skip, 0x0);
                continue;
            }
            float m2[16];
            if (visible) {
                for (std::size_t i = 0; i < 16; ++i) m2[i] = m[i];
                const std::int32_t columns = at<std::int32_t>(lane, 0x34);
                if (columns > 1) {
                    const float f = static_cast<float>(
                        (static_cast<double>(k) - (static_cast<double>(columns) - kOneD) * kHalf) * lateral);
                    const float d[3] = {mul(f, m[0]), mul(m[1], f), mul(f, m[2])};
                    m2[12] = add(m[12], d[0]);
                    m2[13] = add(d[1], m[13]);
                    m2[14] = add(d[2], m[14]);
                }
                Object landscape = at<Object>(lane, 0xd8);
                if (landscape != nullptr && time > at<float>(lane, 0x184)) {
                    const double id = static_cast<double>(index);
                    const auto angle = [&](double c, double k_time) {
                        return static_cast<float>(id * c + static_cast<double>(time) * k_time);
                    };
                    const float s1 = static_cast<float>(std::sin(static_cast<double>(angle(kWobble1, kWobbleT1))));
                    const float s2 = static_cast<float>(std::sin(static_cast<double>(angle(kWobble2, kWobbleT2))));
                    m2[12] = static_cast<float>(static_cast<double>(s1) * amplitude * s2 + m2[12]);
                    const float s3 = static_cast<float>(std::sin(static_cast<double>(angle(kWobble3, kWobbleT3))));
                    const float s4 = static_cast<float>(std::sin(static_cast<double>(angle(kWobble4, kWobbleT4))));
                    m2[14] = static_cast<float>(static_cast<double>(s3) * amplitude * s4 + m2[14]);
                    m2[13] = static_cast<float>(
                        static_cast<double>(host.landscape_forward_local_height_0087fa20(landscape, m2[12], m2[14])) -
                        at<float>(landscape, 0xa8));
                    if (at<std::uint8_t>(lane, 0xdc) == 0) {
                        float normal[3];
                        const float* n = host.landscape_forward_local_normal_0087fb90(at<Object>(lane, 0xd8), normal,
                                                                                    m2[12], m2[14]);
                        m2[4] = n[0];
                        m2[5] = n[1];
                        m2[6] = n[2];
                        host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(m2));
                    }
                }
                if (at<std::uint8_t>(lane, 0xdc) != 0) {
                    const double jitter = static_cast<double>(index % 2) * kJitterStep - kJitterBias;
                    m2[12] = static_cast<float>(static_cast<double>(m2[12]) + jitter);
                    m2[14] = static_cast<float>(jitter + m2[14]);
                }
            }
            Object member = nullptr;
            bool created = false;
            if (member_node == at<Object>(lane, 0x70)) {
                Object class_vector = at<std::uint8_t>(lane, 0xdc) != 0 ? lane + 0xf0 : lane + 0xe0;
                const std::uint32_t pick =
                    host.weighted_pick_00480a90(static_cast<std::uint32_t>(index), lane + 0x100);
                check_index(host, class_vector, pick, 4);
                member = host.vcall_create_instance_08(at<Object>(at<Object>(class_vector, 0x4), 4 * pick));
                const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
                Object node_owner = at<Object>(host.path_object_get_entity_007aca30(at<Object>(lane, 0x20)), 0x30);
                Object owner = at<Object>(host.path_object_get_entity_007aca30(at<Object>(lane, 0x20)), 0x3c);
                host.vcall_attach_98(member, owner, node_owner,
                                     reinterpret_cast<Object>(const_cast<float*>(identity)));
                Object head = at<Object>(lane, 0x70);
                Object node = host.list_buy_node_00485920(lane + 0x6c, head, at<Object>(head, 0x4), &member);
                host.list_incsize_0048ffb0(lane + 0x6c, 1);
                at<Object>(head, 0x4) = node;
                at<Object>(at<Object>(node, 0x4), 0x0) = node;
                host.vcall_set_group_114(member, lane);
                host.vcall_a0(member);
                if (at<std::uint8_t>(lane, 0xdc) != 0 && at<Object>(lane, 0x114) != nullptr &&
                    vector_size(lane + 0x110, 8) != 0) {
                    const std::uint32_t anim =
                        host.weighted_pick_00480a90(static_cast<std::uint32_t>(index), lane + 0x120);
                    check_index(host, lane + 0x110, anim, 8);
                    cc13_left8::soldier_start_animation_004b0f80(host, member, at<Object>(lane, 0x114) + 8 * anim,
                                                                 true, false);
                }
                member_node = at<Object>(lane, 0x70);
                created = true;
            } else {
                member = at<Object>(member_node, 0x8);
                member_node = at<Object>(member_node, 0x0);
            }
            if (!visible) {
                if (at<std::uint8_t>(member, 0x5c) != 0) host.vcall_118(member);
                continue;
            }
            const auto store = [&]() {
                for (std::size_t i = 0; i < 16; ++i) at<float>(member, 0x74 + 4 * i) = m2[i];
            };
            if (created) store();
            if (!created && at<std::uint8_t>(member, 0x5c) != 0) {
                host.matrix_copy_004134f0(member + 0x74, m2);
            } else if (at<std::uint8_t>(member, 0x5c) == 0) {
                host.vcall_11c(member);
                store();
            }
            Object child = at<Object>(member, 0x48);
            at<std::uint8_t>(member, 0xc8) = 0;
            at<std::uint8_t>(member, 0x10c) = 0;
            for (; child != nullptr; child = at<Object>(child, 0x44)) host.scene_node_invalidate_subtree_pose_0042ed50(child);
            host.vcall_d8(member);
        }
        time = static_cast<float>(static_cast<double>(time) - spacing);
    }
}

}  // namespace bsp::cc13_left8b
