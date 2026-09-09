#include "bsp/shader_source.hpp"
#include <cstddef>
#include <utility>

namespace bsp {
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
