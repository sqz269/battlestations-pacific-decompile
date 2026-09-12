#include "bsp/scene_contents_hosts.hpp"

#include <cmath>
#include <cstring>

// Reconstruction of the four scene-contents steps. Evidence, offset tables and
// the labelled gaps: docs/SCENE_CONTENTS_HOSTS.md.
namespace bsp {
namespace {

// The image compares native strings case-insensitively with 00BF7FBF (_stricmp)
// and treats an absent buffer as the empty string. 0046E271..0046E28D and
// 0046E493..0046E4BA both spell that rule out by hand, so it lives here once.
bool native_string_equals_insensitive(const char* left, const char* right) noexcept {
    const char* a = left != nullptr ? left : "";
    const char* b = right != nullptr ? right : "";
    for (;; ++a, ++b) {
        const unsigned char ca = static_cast<unsigned char>(*a);
        const unsigned char cb = static_cast<unsigned char>(*b);
        const int la = ca >= 'A' && ca <= 'Z' ? ca - 'A' + 'a' : ca;
        const int lb = cb >= 'A' && cb <= 'Z' ? cb - 'A' + 'a' : cb;
        if (la != lb) {
            return false;
        }
        if (la == 0) {
            return true;
        }
    }
}

bool read_u32(const std::uint8_t* data, std::size_t size, std::size_t& at, std::uint32_t& out) {
    if (at + 4 > size) {
        return false;
    }
    out = static_cast<std::uint32_t>(data[at]) | static_cast<std::uint32_t>(data[at + 1]) << 8 |
        static_cast<std::uint32_t>(data[at + 2]) << 16 | static_cast<std::uint32_t>(data[at + 3]) << 24;
    at += 4;
    return true;
}

bool read_f32(const std::uint8_t* data, std::size_t size, std::size_t& at, float& out) {
    std::uint32_t bits = 0;
    if (!read_u32(data, size, at, bits)) {
        return false;
    }
    std::memcpy(&out, &bits, sizeof out);
    return true;
}

bool read_length_prefixed(
    const std::uint8_t* data, std::size_t size, std::size_t& at, std::string& out) {
    std::uint32_t length = 0;
    if (!read_u32(data, size, at, length)) {
        return false;
    }
    if (length > size - at) {
        return false;
    }
    out.assign(reinterpret_cast<const char*>(data + at), length);
    at += length;
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// Step 1, 004D0EE0. Coverage: complete.

void preload_scene_record_effects_004d0ee0(SceneEffectPreloadHost& host) {
    host.clear_effect_handles(); // 004D0F05, 004CB160(record+D50h, 0)
    // 004D0F0C tests the count before the first iteration and 004D0F6D re-reads
    // it at the bottom, so a host that shrinks the list mid-walk stops early.
    for (int index = 0; index < host.record_effect_name_count(); ++index) {
        const SceneContentsHost::EffectHandle handle =
            host.acquire_effect_by_name(host.record_effect_name(index)); // 004D0F24
        host.push_effect_handle(handle); // 004D0F34
        host.release_temporary(handle); // 004D0F47..004D0F62
    }
}

// ---------------------------------------------------------------------------
// Step 2, the .nav file.

TerrainGridNavFile parse_terrain_grid_nav(const std::uint8_t* data, std::size_t size) {
    TerrainGridNavFile file;
    if (data == nullptr) {
        file.error = "no data";
        return file;
    }
    std::size_t at = 0;
    if (!read_length_prefixed(data, size, at, file.class_name)) { // 004248D9, vtable +60h
        file.error = "truncated root name";
        return file;
    }
    std::uint32_t count = 0;
    if (!read_u32(data, size, at, count)) { // 004248E7, vtable +38h
        file.error = "truncated layer count";
        return file;
    }
    // 004248EB is a signed test, so a negative count loads nothing at all.
    if (static_cast<std::int32_t>(count) <= 0) {
        file.bytes_consumed = at;
        file.ok = true;
        return file;
    }
    file.layers.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        TerrainGridLayerRecord layer;
        std::uint32_t dimension = 0;
        const bool header_ok = read_length_prefixed(data, size, at, layer.class_name) && // 00423A0F
            read_f32(data, size, at, layer.half_extent) && // 00423A22, +44h
            read_u32(data, size, at, dimension) && // 00423A30, +38h
            read_f32(data, size, at, layer.cell_size) && // 00423A3E, +44h
            read_f32(data, size, at, layer.file_scalar) && // 00423A4C
            read_f32(data, size, at, layer.slope_limit); // 00423A5A
        if (!header_ok) {
            file.error = "truncated layer header";
            return file;
        }
        layer.dimension = static_cast<int>(dimension);
        // 00423A5F..00423A6D: the grid is resized to n*n, and 00423A9D reads
        // exactly that many bytes. The native IMUL is signed and unchecked.
        const std::size_t cells = static_cast<std::size_t>(layer.dimension) *
            static_cast<std::size_t>(layer.dimension);
        if (layer.dimension < 0 || cells > size - at) {
            file.error = "truncated grid";
            return file;
        }
        layer.grid.assign(data + at, data + at + cells);
        at += cells;
        file.layers.push_back(std::move(layer));
    }
    file.bytes_consumed = at;
    file.ok = true;
    return file;
}

int select_terrain_grid_layer(
    const std::vector<TerrainGridLayerRecord>& layers, float value, bool value_is_angle) noexcept {
    if (layers.empty()) {
        return -1;
    }
    // 0041DF58..0041DF67: tan is applied only when the flag is set.
    const float wanted = value_is_angle ? static_cast<float>(std::tan(value)) : value;
    // 0041DF88 primes EBX with the first element's data before the walk, so an
    // unmatched query hands back the first layer, not null.
    int best = 0;
    float best_limit = 0.0f; // 0041DF70, XORPS zeroes the running maximum
    for (std::size_t i = 0; i < layers.size(); ++i) {
        const float limit = layers[i].slope_limit;
        if (limit > best_limit && wanted > limit) { // 0041DFB9 then 0041DFC3
            best_limit = limit;
            best = static_cast<int>(i);
        }
    }
    return best;
}

void load_avoid_zones_004248a0(SceneAvoidZoneHost& host) {
    host.ensure_registry(); // 004D5251, 004C17D0
    host.begin_load(); // 004248C1, 0041DED0 on the registry
    host.read_string(); // 004248D9: the root name, read and dropped
    const int count = host.read_int(); // 004248E7
    for (int i = 0; i < count; ++i) { // 004248EB is signed, so <= 0 loads nothing
        TerrainGridLayerRecord layer;
        layer.class_name = host.read_string(); // 00423A0F, also dropped natively
        layer.half_extent = host.read_float(); // 00423A22
        layer.dimension = host.read_int(); // 00423A30
        layer.cell_size = host.read_float(); // 00423A3E
        layer.file_scalar = host.read_float(); // 00423A4C
        layer.slope_limit = host.read_float(); // 00423A5A
        const std::size_t cells = layer.dimension > 0
            ? static_cast<std::size_t>(layer.dimension) * static_cast<std::size_t>(layer.dimension)
            : 0u;
        layer.grid.assign(cells, 0u);
        if (cells != 0) {
            host.read_bytes(layer.grid.data(), cells); // 00423A9D, vtable +24h
        }
        host.append_layer(layer); // 0042494F, push into the list at registry+8h
    }
    host.end_load(); // 00424975, 0041E000
}

// ---------------------------------------------------------------------------
// Step 3, 004BA870.

float scene_cloud_weight_total(const std::array<float, 3>& weights) noexcept {
    // 004BA8A3..004BA8CE: FLD w0 / FADD qword 0.0 / FSTP float, then the same
    // load-add-store for w1 and w2. Each partial is rounded back to float, so
    // the parentheses below are the native order, not a simplification.
    float total = static_cast<float>(static_cast<double>(weights[0]) + 0.0); // 00D7A258 is 0.0
    total = total + weights[1];
    total = total + weights[2];
    return total;
}

bool scene_cloud_candidate_rejected(
    const std::array<float, 3>& candidate,
    const std::array<float, 3>& placed,
    float placed_separation,
    float candidate_separation) noexcept {
    const float dx = candidate[0] - placed[0]; // 004BA9F4
    const float dy = candidate[1] - placed[1]; // 004BA9FF
    const float dz = candidate[2] - placed[2]; // 004BAA0A
    const float squared = dx * dx + dy * dy + dz * dz; // 004BAA1D..004BAA2D
    // 004BAA31..004BAA59: the sqrt is skipped and the distance forced to 0 when
    // the squared distance is at or below the epsilon, which makes coincident
    // points a rejection rather than a division-by-zero.
    const float distance = static_cast<double>(squared) > kSceneCloudDistanceEpsilon
        ? static_cast<float>(std::sqrt(static_cast<double>(squared)))
        : 0.0f;
    return placed_separation + candidate_separation > distance; // 004BAA6E, JA
}

std::array<float, 16> scene_cloud_placement_matrix(const std::array<float, 3>& point) noexcept {
    // 004BAB17..004BAB9C, row-major with 1.0f (00D7A24C) on the diagonal.
    return {{1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             point[0], point[1], point[2], 1.0f}};
}

void scatter_scene_clouds_004ba870(SceneCloudScatterHost& host) {
    if (host.cloud_gate() < 1) {
        return; // 004BA879, the only early exit
    }
    const int count = host.cloud_count(); // record+CA0h, ESI+18h
    const std::array<float, 3> weights = host.cloud_kind_weights();
    const std::array<float, 3> box_min = host.cloud_box_min();
    const std::array<float, 3> box_max = host.cloud_box_max();
    const float total = scene_cloud_weight_total(weights);

    // 004BA8D7 and 004BA8F6 allocate count*12 and count*4 bytes up front. Only
    // the second is freed (004BABE0); the point array leaks, which this
    // projection does not reproduce.
    std::vector<std::array<float, 3>> points;
    std::vector<float> separations;
    points.reserve(count > 0 ? static_cast<std::size_t>(count) : 0u);
    separations.reserve(count > 0 ? static_cast<std::size_t>(count) : 0u);

    for (int placed = 0; placed < count; ++placed) {
        const float roll = host.random_range(0.0f, total); // 004BA929
        const SceneCloudKindChoice choice = select_scene_cloud_kind(weights, roll);
        const float separation = kSceneCloudKinds[choice.index].min_separation; // 00E081DC + kind*18h

        std::array<float, 3> candidate{};
        int attempts = 0;
        for (;;) {
            candidate[0] = host.random_range(box_min[0], box_max[0]); // 004BA984
            candidate[1] = host.random_range(box_min[1], box_max[1]); // 004BA9A2
            candidate[2] = host.random_range(box_min[2], box_max[2]); // 004BA9C0
            bool rejected = false;
            for (int i = 0; i < placed; ++i) { // 004BA9CB..004BAA7C
                if (scene_cloud_candidate_rejected(
                        candidate, points[static_cast<std::size_t>(i)],
                        separations[static_cast<std::size_t>(i)], separation)) {
                    rejected = true;
                    break;
                }
            }
            if (!rejected) {
                break; // 004BAA88, every placed point cleared
            }
            ++attempts;
            if (attempts >= kSceneCloudMaxPlacementAttempts) {
                break; // 004BAA91, the last candidate is kept as it stands
            }
        }

        points.push_back(candidate); // 004BAAAE..004BAACC
        separations.push_back(separation); // 004BAAD7

        // 004BAADC..004BAAF7: one more draw in [-pi, +pi] whose value is popped
        // unused at 004BAB02. It is kept because it advances the stream.
        (void)host.random_range(kSceneCloudYawLow, kSceneCloudYawHigh);

        const SceneContentsHost::EntityHandle entity =
            host.create_cloud_entity(kSceneCloudKinds[choice.index].class_name); // 004BAB12
        host.place_entity(entity, scene_cloud_placement_matrix(candidate)); // vtable +88h
        host.activate_entity(entity); // vtable +D8h
    }
}

// ---------------------------------------------------------------------------
// Step 4, the weather pass.

WeatherDescriptorUse weather_sub_scene_use(
    const WeatherSubScene& row, const char* override_name) noexcept {
    if (row.descriptor.empty()) {
        return WeatherDescriptorUse::Skipped; // 0046E4DD, no tokenizer is built
    }
    // 0046E530..0046E766. An absent ID or an absent override name both behave
    // as the empty string, so the empty-ID row is what a null override selects.
    const bool selected = native_string_equals_insensitive(
        override_name, row.id.c_str());
    return selected ? WeatherDescriptorUse::AppliedToReaderBag
                    : WeatherDescriptorUse::ParsedAndDiscarded;
}

int select_weather_entry(
    const std::vector<WeatherEntry>& entries, const char* scene_path) noexcept {
    int matched = -1;
    // 0046E1E0..0046E6D0: the loop runs to the end of the table whatever it
    // finds, so the last matching entry is the one whose sub-scenes applied.
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (native_string_equals_insensitive(entries[i].scene_file.c_str(), scene_path)) {
            matched = static_cast<int>(i); // 0046E2CD
        }
    }
    return matched;
}

void run_weather_descriptor_pass_0046df00(
    SceneWeatherPassHost& host, const char* scene_path, const char* override_name) {
    host.open_lua_state(kWeatherLuaLibraryMask); // 0046E0B6 then 0046E0CC
    host.run_script(kWeatherLuaPath); // 0046E119
    const std::vector<WeatherEntry> entries = host.read_weathers_table(); // 0046E145..

    for (const WeatherEntry& entry : entries) {
        if (!native_string_equals_insensitive(entry.scene_file.c_str(), scene_path)) {
            continue; // 0046E2CD, on to the next Weathers row
        }
        for (const WeatherSubScene& row : entry.sub_scenes) {
            const WeatherDescriptorUse use = weather_sub_scene_use(row, override_name);
            if (use == WeatherDescriptorUse::Skipped) {
                continue;
            }
            host.parse_descriptor(row.descriptor, use); // 008D9CF0, 008F5A00, 008D9C30
            if (use != WeatherDescriptorUse::AppliedToReaderBag) {
                continue; // 0046E563: the bag is built and destroyed at once
            }
            // 0046E783..0046E9CC. Each key is skipped when the Lua value is nil
            // and again when 008F2260 does not find it in the bag.
            if (row.has_static_shadow_texture) {
                host.set_bag_string(kWeatherShadowTextureKey, row.static_shadow_texture);
            }
            if (row.has_shot_offset_x) {
                host.set_bag_float(kWeatherShotOffsetXKey, row.shot_offset_x);
            }
            if (row.has_shot_offset_z) {
                host.set_bag_float(kWeatherShotOffsetZKey, row.shot_offset_z);
            }
            if (row.has_shot_size) {
                host.set_bag_float(kWeatherShotSizeKey, row.shot_size);
            }
        }
    }

    host.close_lua_state(); // 0046E6F9, 00B669A0
}

} // namespace bsp
