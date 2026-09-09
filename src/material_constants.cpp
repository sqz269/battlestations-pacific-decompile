#include "bsp/material_constants.hpp"
#include <cstring>

namespace bsp {
void write_system_matrix_00b404a0(float* destination, const float* source) {
    for (unsigned row = 0; row < 4; ++row) {
        for (unsigned column = 0; column < 4; ++column) {
            const float* input = source + column * 4 + row;
            float* output = destination + row * 4 + column;
            __asm {
                mov eax, input
                mov ecx, output
                fld dword ptr [eax]
                fstp dword ptr [ecx]
            }
        }
    }
}

namespace {
static_assert(sizeof(float) == sizeof(std::uint32_t));

std::size_t matrix_rows(std::int32_t index,
    const std::vector<VertexConstantShape>& shapes) {
    std::int32_t rows = 0;
    for (const auto& shape : shapes) {
        if (shape.start_register == index) rows = shape.row_count;
    }
    return rows >= 2 && rows <= 4 ? static_cast<std::size_t>(rows) : 0;
}

MaterialConstantPackStatus check_stage(const MaterialConstantParameter& parameter,
    std::int32_t index, std::size_t rows, const std::vector<float>& output) {
    if (index < 0 || (parameter.matrix && rows == 0)) {
        return MaterialConstantPackStatus::complete;
    }
    if (parameter.matrix && parameter.source_words.size() < 16) {
        return MaterialConstantPackStatus::source_too_short;
    }
    const auto register_index = static_cast<std::size_t>(index);
    // Check before multiplication, including on Win32 size_t.
    if (register_index > output.size() / 4) {
        return MaterialConstantPackStatus::destination_too_short;
    }
    const auto offset = register_index * 4;
    const auto count = parameter.matrix ? rows * 4 : parameter.source_words.size();
    if (count > output.size() - offset) {
        return MaterialConstantPackStatus::destination_too_short;
    }
    return MaterialConstantPackStatus::complete;
}

void write_stage(const MaterialConstantParameter& parameter, std::int32_t index,
    std::size_t rows, std::vector<float>& output) {
    if (index < 0 || (parameter.matrix && rows == 0)) return;
    const auto offset = static_cast<std::size_t>(index) * 4;
    if (!parameter.matrix) {
        // Avoid even pointer arithmetic on an empty buffer for a zero copy.
        if (!parameter.source_words.empty()) {
            std::memcpy(output.data() + offset, parameter.source_words.data(),
                parameter.source_words.size() * sizeof(std::uint32_t));
        }
        return;
    }
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            std::memcpy(&output[offset + row * 4 + column],
                &parameter.source_words[column * 4 + row], sizeof(float));
        }
    }
}
}

MaterialConstantPackStatus pack_material_parameter_constants_00b423c5(
    const std::vector<MaterialConstantParameter>& parameters, std::size_t selector,
    const std::vector<VertexConstantShape>& vertex_shapes,
    std::vector<float>& vertex_words, std::vector<float>& pixel_words) {
    if (&vertex_words == &pixel_words) {
        return MaterialConstantPackStatus::shared_output_buffer;
    }
    for (const auto& parameter : parameters) {
        if (selector >= parameter.vertex_registers.size()
            || selector >= parameter.pixel_registers.size()) {
            return MaterialConstantPackStatus::selector_out_of_range;
        }
        const auto vertex_index = parameter.vertex_registers[selector];
        const auto rows = parameter.matrix ? matrix_rows(vertex_index, vertex_shapes) : 0;
        auto status = check_stage(parameter, vertex_index, rows, vertex_words);
        if (status != MaterialConstantPackStatus::complete) return status;
        status = check_stage(parameter, parameter.pixel_registers[selector], 4, pixel_words);
        if (status != MaterialConstantPackStatus::complete) return status;
    }
    for (const auto& parameter : parameters) {
        const auto vertex_index = parameter.vertex_registers[selector];
        const auto rows = parameter.matrix ? matrix_rows(vertex_index, vertex_shapes) : 0;
        write_stage(parameter, vertex_index, rows, vertex_words);
        write_stage(parameter, parameter.pixel_registers[selector], 4, pixel_words);
    }
    return MaterialConstantPackStatus::complete;
}
}
