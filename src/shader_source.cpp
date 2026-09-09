#include "bsp/shader_source.hpp"
#include <cstddef>
#include <utility>

namespace bsp {
namespace {
std::string signed_decimal(std::uint32_t value) {
    // Native variadic %i interprets the full DWORD as signed, even though
    // dimension selection above one uses unsigned comparisons.
    return std::to_string(value <= 0x7fffffffu ? static_cast<std::int64_t>(value)
        : static_cast<std::int64_t>(value) - 0x100000000ll);
}
}

void append_system_constant_00b38c60(const ShaderSystemConstant& constant,
    std::int32_t register_index, std::string& output) {
    std::string result("float");
    if (constant.second_dimension > 1) {
        result += signed_decimal(constant.first_dimension);
        result += 'x'; result += signed_decimal(constant.second_dimension);
    } else if (constant.first_dimension > 1) {
        result += signed_decimal(constant.first_dimension);
    }
    result += ' '; result += constant.name.c_str();
    if (constant.array_count > 1) {
        result += '['; result += signed_decimal(constant.array_count); result += ']';
    }
    if (register_index >= 0) {
        result += " : register(c"; result += std::to_string(register_index); result += ')';
    }
    result += ";\n";
    output += result;
}

void append_system_constant_header_00b38ff0(const std::vector<ShaderSystemConstant>& constants,
    bool explicit_registers, std::uint32_t register_limit, std::string& output) {
    std::uint32_t cursor = 0;
    for (const auto& constant : constants) {
        // A cursor with its sign bit set is passed by native but the emitter
        // suppresses its annotation; only the starting cursor is limit-checked.
        const auto index = explicit_registers && cursor < register_limit && cursor <= 0x7fffffffu
            ? static_cast<std::int32_t>(cursor) : -1;
        append_system_constant_00b38c60(constant, index, output);
        cursor += constant.second_dimension * constant.array_count; // DWORD wrap.
    }
}

ShaderSourceStatus append_interpolator_pack_00b35540(const std::vector<ShaderField>& fields,
    const ShaderInterpolatorLayout& layout, std::string& output) {
    for (const auto* components : {&layout.texcoords, &layout.colors}) {
        for (const auto& entry : *components) {
            if (entry.field >= fields.size() || entry.component >= 4)
                return ShaderSourceStatus::invalid_packing;
        }
    }
    if (layout.fog.field != 0xff && layout.fog.field >= fields.size())
        return ShaderSourceStatus::invalid_packing;
    std::string result("\nsInterpolators PackInterpolators(sVertexOut OUT)\n{\n"
        "\t\tsInterpolators INT;\n\n\t\tINT.Position = OUT.ScreenSpacePos;\n");
    auto emit = [&](const std::vector<ShaderPackedComponent>& components, const char* target) {
        static constexpr char channels[] = "xyzw";
        for (std::size_t i = 0; i < components.size(); ++i) {
            const auto& entry = components[i];
            result += "\t\tINT."; result += target; result += std::to_string(i / 4);
            result += '.'; result += channels[i % 4]; result += " = OUT.";
            result += fields[entry.field].name.c_str(); result += '.';
            result += channels[entry.component]; result += ";\n";
        }
    };
    emit(layout.texcoords, "TexCoord"); emit(layout.colors, "Color");
    if (layout.fog.field != 0xff) {
        result += "\t\tINT.Fog = OUT."; result += fields[layout.fog.field].name.c_str();
        result += ";\n";
    }
    result += "\t\treturn INT;\n}\n";
    output += result;
    return ShaderSourceStatus::complete;
}

void append_interpolator_mapping_00b34aa0(const std::vector<ShaderField>& fields,
    ShaderInterpolatorLayout& layout) {
    for (std::size_t i = 1; i < fields.size(); ++i) {
        const auto& field = fields[i];
        auto* target = field.semantic == ShaderSemantic::texcoord ? &layout.texcoords
            : field.semantic == ShaderSemantic::color ? &layout.colors : nullptr;
        if (target) {
            for (std::uint8_t component = 0; component < 4; ++component) {
                if (field.component_mask & (1u << component))
                    target->push_back({static_cast<std::uint8_t>(i), component});
            }
        } else if (field.semantic == ShaderSemantic::fog) {
            layout.fog = {static_cast<std::uint8_t>(i), 0};
        }
    }
}

ShaderSourceStatus append_interpolator_struct_00b36e30(ShaderInterpolatorLayout& layout,
    const ShaderInterpolatorOptions& options, std::string& output) {
    // Native signed (count+3)/4; restrict the new interface to non-overflow counts.
    if (layout.texcoords.size() > 0x7ffffffcu || layout.colors.size() > 0x7ffffffcu)
        return ShaderSourceStatus::invalid_packing;
    std::string result("\nstruct sInterpolators\n{\n");
    if (options.include_position) result += "\tfloat4 Position\t: POSITION0;\n";
    auto emit = [&](std::size_t count, const char* name, const char* semantic) {
        const auto registers = static_cast<std::uint32_t>((count + 3) / 4);
        const auto last = static_cast<std::uint32_t>(count % 4 ? count % 4 : 4);
        for (std::uint32_t i = 0; i < registers; ++i) {
            result += "\tfloat" + std::to_string(i + 1 == registers ? last : 4);
            result += ' '; result += name; result += std::to_string(i);
            result += "\t: "; result += semantic; result += std::to_string(i);
            result += ";\n";
        }
    };
    emit(layout.texcoords.size(), "TexCoord", "TEXCOORD");
    emit(layout.colors.size(), "Color", "COLOR");
    if (layout.fog.field != 0xff && options.include_fog) result += "\tfloat  Fog\t: FOG;\n";
    if (options.allow_vpos && (options.base_descriptor_vpos || options.effect_descriptor_vpos))
        result += "\tfloat2  vPos\t: VPOS;\n";
    result += "};\n\n";
    output += result;
    layout.texcoord_registers = static_cast<std::uint32_t>((layout.texcoords.size() + 3) / 4);
    layout.texcoord_last_width = static_cast<std::uint32_t>(layout.texcoords.size() % 4 ? layout.texcoords.size() % 4 : 4);
    layout.color_registers = static_cast<std::uint32_t>((layout.colors.size() + 3) / 4);
    layout.color_last_width = static_cast<std::uint32_t>(layout.colors.size() % 4 ? layout.colors.size() % 4 : 4);
    return ShaderSourceStatus::complete;
}

ShaderSourceStatus append_interpolator_unpack_00b37000(const std::vector<ShaderField>& fields,
    const ShaderInterpolatorLayout& layout, const ShaderInterpolatorOptions& options,
    std::string& output) {
    for (const auto* components : {&layout.texcoords, &layout.colors}) {
        for (const auto& entry : *components) {
            if (entry.field >= fields.size() || entry.component >= 4)
                return ShaderSourceStatus::invalid_packing;
        }
    }
    if (layout.fog.field != 0xff && layout.fog.field >= fields.size())
        return ShaderSourceStatus::invalid_packing;
    std::string result("\nsPixelIn UnpackInterpolators(sInterpolators INT)\n{\n\t\tsPixelIn PixelIn;\n\n");
    auto emit = [&](const std::vector<ShaderPackedComponent>& components, const char* source) {
        static constexpr char channels[] = "xyzw";
        for (std::size_t i = 0; i < components.size(); ++i) {
            const auto& entry = components[i];
            result += "\t\tPixelIn."; result += fields[entry.field].name.c_str();
            result += '.'; result += channels[entry.component]; result += " = INT.";
            result += source; result += std::to_string(i / 4); result += '.';
            result += channels[i % 4]; result += ";\n";
        }
    };
    emit(layout.texcoords, "TexCoord"); emit(layout.colors, "Color");
    if (layout.fog.field != 0xff) {
        result += "\t\tPixelIn."; result += fields[layout.fog.field].name.c_str();
        result += options.zero_fog ? " = 0;\n" : " = INT.Fog;\n";
    }
    if (options.base_descriptor_vpos || options.effect_descriptor_vpos)
        result += "\t\tPixelIn.vPos = INT.vPos;\n";
    result += "\t\treturn PixelIn;\n}\n";
    output += result;
    return ShaderSourceStatus::complete;
}

ShaderSourceStatus format_shader_field_00b385b0(const ShaderField& field,
    bool include_semantic, std::string& output) {
    const char* type = nullptr;
    switch (field.scalar_type) {
    case ShaderScalarType::floating: type = "float"; break;
    case ShaderScalarType::integer: type = "int"; break;
    default: return ShaderSourceStatus::unsupported_scalar_type;
    }
    static constexpr const char* semantics[] = {
        "POSITION", "COLOR", "TEXCOORD", "NORMAL", "BINORMAL", "TANGENT",
        "BLENDINDICES", "BLENDWEIGHT", "FOG", "INDEX", "VPOS"
    };
    const auto semantic_index = static_cast<std::uint32_t>(field.semantic);
    if (include_semantic && semantic_index >= sizeof(semantics) / sizeof(semantics[0])) {
        return ShaderSourceStatus::unsupported_semantic;
    }
    std::string result(type);
    if (field.component_count > 1) result += std::to_string(field.component_count);
    result += "\t\t";
    result += field.name;
    if (include_semantic) {
        result += "\t\t : ";
        result += semantics[semantic_index];
        result += std::to_string(field.semantic_index);
    }
    // Native 00b389d2..00b38a0c appends literal00ce5698, elided by pseudocode.
    result += ';';
    output = std::move(result);
    return ShaderSourceStatus::complete;
}

ShaderSourceStatus append_shader_struct_00b38b50(const std::string& name,
    const std::vector<ShaderField>& fields, const ShaderStructOptions& options,
    std::string& output) {
    // Build separately so an unsupported field cannot leave a partial struct.
    std::string result("\nstruct ");
    result += name.c_str(); // Native formatted %s append.
    result += "\n{\n";
    for (std::size_t i = options.first_field; i < fields.size(); ++i) {
        std::string declaration;
        const auto status = format_shader_field_00b385b0(fields[i],
            options.include_semantics, declaration);
        if (status != ShaderSourceStatus::complete) return status;
        result += '\t';
        result += declaration.c_str();
        result += '\n'; // 00b35110 adds a newline after its formatted text.
    }
    if (options.allow_vpos
        && (options.base_descriptor_vpos || options.effect_descriptor_vpos)) {
        result += "\tfloat2 vPos;\n";
    }
    result += "};\n\n"; // Literal already has LF; 00b34f20 adds another.
    output += result;
    return ShaderSourceStatus::complete;
}
}
