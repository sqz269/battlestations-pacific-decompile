#include "bsp/material_point_light_constants.hpp"
#include "bsp/compiled_material.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include <cstdint>
#include <cstring>
#include <limits>

namespace bsp {
const std::vector<GeneratedModelPointLightLinks*>& borrowed_point_lights_00b6dc50(
    const GeneratedModelLifetime& model) noexcept {
    return model.point_lights_164;
}

bool write_material_point_light_constants_00b43160_fragment(
    const CompiledMaterialPass& pass,
    const std::vector<GeneratedModelPointLightLinks*>& actual_model_point_lights,
    std::vector<float>& vertex_words, std::string& error) {
    error.clear();
    const auto count_register = pass.vb.registers[45];
    if (count_register == 0xff) return true;
    const auto source_count = actual_model_point_lights.size();
    if (source_count > static_cast<std::size_t>(
            (std::numeric_limits<std::int32_t>::max)())) {
        error = "Material point-light count exceeds the valid native signed array count";
        return false;
    }
    const auto count = source_count < 4 ? source_count : std::size_t{4};
    const auto count_offset = static_cast<std::size_t>(count_register) * 4;
    if (count_offset > vertex_words.size()
        || vertex_words.size() - count_offset < 4) {
        error = "Material point-light count register exceeds vertex constant storage";
        return false;
    }
    // Native00B431AF/B4/B9/BB order. Count0..4 converts exactly under all
    // ordinary FP rounding modes; corrupt negative DWORD counts are excluded.
    vertex_words[count_offset + 1] = 0.0f;
    vertex_words[count_offset + 2] = 0.0f;
    vertex_words[count_offset] = static_cast<float>(count);
    vertex_words[count_offset + 3] = 0.0f;

    // Native reloads the VS metadata AFTER the count writes at00B431C0/C3.
    const auto data_register = pass.vb.registers[46];
    if (data_register == 0xff || count == 0) return true;
    const auto data_offset = static_cast<std::size_t>(data_register) * 4;
    if (data_offset > vertex_words.size()
        || vertex_words.size() - data_offset < count * 8) {
        error = "Material point-light data registers exceed vertex constant storage";
        return false;
    }
    for (std::size_t index = 0; index < count; ++index) {
        // Keep the original ordered borrowed pointers; do not snapshot lights
        // or route through building-instance packing (different layout/count).
        const auto* light = actual_model_point_lights[index];
        if (!light) {
            error = "Material point-light list contains a null selected owner";
            return false;
        }
        const auto output_offset = data_offset + index * 8;
        std::uint32_t position[4];
        std::memcpy(position, light->values.position_radius.data(), sizeof(position));
        for (std::size_t lane = 0; lane < 4; ++lane) {
            std::memcpy(&vertex_words[output_offset + lane], &position[lane],
                sizeof(std::uint32_t));
        }
        for (std::size_t lane = 0; lane < 4; ++lane) {
            std::uint32_t word;
            std::memcpy(&word, &light->values.color[lane], sizeof(word));
            std::memcpy(&vertex_words[output_offset + 4 + lane], &word, sizeof(word));
        }
    }
    return true;
}
}
