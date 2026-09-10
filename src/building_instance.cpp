#include "bsp/building_instance.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>

namespace bsp {
bool write_building_instance_data_00b55780(const CameraMatrix& refreshed_world,
    const std::vector<BuildingInstancePointLight>& ordered_point_lights,
    float visibility, float material_diffuse_alpha,
    BuildingInstanceData& output, std::string& error) {
    static_assert(sizeof(BuildingInstanceData) == building_instance_stride,
        "Building instance declaration requires nine32-bit float4 records");
    if (ordered_point_lights.size()
        > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)())) {
        error = "Building instance light list exceeds the native nonnegative count domain";
        return false;
    }
    BuildingInstanceData record;
    //00b5579a..00b55806: twelve sequential native FLD/FSTP pairs.
    for (std::size_t row = 0; row < 3; ++row)
        for (std::size_t column = 0; column < 4; ++column)
            record[row * 4 + column] = refreshed_world[column * 4 + row];

    const auto light_count = (std::min)(ordered_point_lights.size(), std::size_t{3});
    // Native code writes each position and color pair, including its original
    // color alpha, before performing the final three scalar overwrites.
    for (std::size_t light = 0; light < 3; ++light) {
        auto* position = record.data() + 12 + light * 4;
        auto* color = record.data() + 24 + light * 4;
        if (light < light_count) {
            std::memcpy(position, ordered_point_lights[light].position_radius.data(), 16);
            std::memcpy(color, ordered_point_lights[light].color.data(), 16);
        } else {
            std::fill_n(position, 4, 0.0f);
            std::fill_n(color, 4, 0.0f);
        }
    }
    record[27] = visibility;
    record[31] = static_cast<float>(light_count);
    record[35] = material_diffuse_alpha;
    output = record;
    error.clear();
    return true;
}
}
