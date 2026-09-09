#include "bsp/shader_source.hpp"
#include <cstddef>
#include <utility>

namespace bsp {
ShaderSourceStatus parse_pixel_usage_00b61280(const std::string& disassembly,
    std::vector<std::uint32_t>& texcoord, std::vector<std::uint32_t>& color) {
    if (texcoord.empty()) return ShaderSourceStatus::invalid_packing;
    if (texcoord[0] == 500) return ShaderSourceStatus::complete;
    const std::string text(disassembly.c_str());
    auto tex = texcoord, colors = color;
    auto parse = [&](const char* token, std::size_t index_offset,
                     std::vector<std::uint32_t>& masks) {
        std::size_t scan = 0;
        while ((scan = text.find(token, scan)) != std::string::npos) {
            const auto index_pos = scan + index_offset;
            const auto body = index_pos + 2;
            if (body > text.size()) return false;
            const auto end = text.find('\n', body);
            if (end == std::string::npos) return false;
            const char digit = text[index_pos];
            const auto index = static_cast<std::size_t>(digit >= '0' && digit <= '9' ? digit - '0' : 0);
            if (index >= masks.size()) return false;
            std::uint32_t mask = 0;
            const char* components = "xyzw";
            for (unsigned bit = 0; bit < 4; ++bit) {
                const auto found = text.find(components[bit], body);
                if (found != std::string::npos && found < end) mask |= 1u << bit;
            }
            masks[index] |= mask ? mask : 15;
            scan = end;
        }
        return true;
    };
    if (!parse("dcl_texcoord", 12, tex) || !parse("dcl_color", 9, colors))
        return ShaderSourceStatus::invalid_packing;
    texcoord = std::move(tex); color = std::move(colors);
    return ShaderSourceStatus::complete;
}

ShaderSourceStatus append_selected_interpolators_00b36800(
    const std::vector<ShaderField>& base, const std::vector<ShaderField>& effect,
    const std::vector<std::uint32_t>* texcoord_usage,
    const std::vector<std::uint32_t>* color_usage, std::vector<ShaderField>& output) {
    std::vector<ShaderField> selected{{"ScreenSpacePos", ShaderScalarType::floating,
        4, ShaderSemantic::position, 0, 0}};
    std::uint64_t texcoord_offset = 0, color_offset = 0;
    for (const auto* descriptor : {&base, &effect}) {
        for (const auto& field : *descriptor) {
            auto copy = field;
            if (!texcoord_usage && !color_usage) {
                copy.component_mask = field.component_count >= 32 ? 0xffffffffu
                    : (1u << field.component_count) - 1;
                selected.push_back(std::move(copy));
                continue;
            }
            const auto* usage = field.semantic == ShaderSemantic::texcoord ? texcoord_usage
                : field.semantic == ShaderSemantic::color ? color_usage : nullptr;
            if (!usage) continue;
            auto& offset = field.semantic == ShaderSemantic::texcoord ? texcoord_offset : color_offset;
            const auto end = offset + field.component_count;
            if (end > 0xffffffffu || end > static_cast<std::uint64_t>(usage->size()) * 4)
                return ShaderSourceStatus::invalid_packing;
            copy.component_count = 0; copy.component_mask = 0;
            for (std::uint32_t component = field.component_count; component != 0; --component) {
                const auto slot = offset + component - 1;
                if ((*usage)[static_cast<std::size_t>(slot / 4)] & (1u << (slot % 4))) {
                    ++copy.component_count;
                    copy.component_mask |= 1u << ((component - 1) & 31);
                }
            }
            offset = end;
            if (copy.component_count) selected.push_back(std::move(copy));
        }
    }
    output.insert(output.end(), selected.begin(), selected.end());
    return ShaderSourceStatus::complete;
}

void append_vertex_system_fields_00b35be0(std::vector<ShaderField>& output) {
    const ShaderField fields[] = {
#include "shader_vertex_system_fields.inc"
    };
    output.insert(output.end(), std::begin(fields), std::end(fields));
}
void append_pixel_system_fields_00b372d0(std::vector<ShaderField>& output) {
    const ShaderField fields[] = {
#include "shader_pixel_system_fields.inc"
    };
    output.insert(output.end(), std::begin(fields), std::end(fields));
}
void append_vertex_inputs_00b35930(const std::vector<ShaderField>& base,
    const std::vector<ShaderField>& effect, std::vector<ShaderField>& output) {
    for (const auto* fields : {&base, &effect}) {
        for (const auto& field : *fields) {
            output.push_back(field); output.back().component_mask = 0;
        }
    }
}

ShaderProfiles select_shader_profiles_00b43b00(std::uint32_t version,
    const std::optional<std::string>& vertex_override,
    const std::optional<std::string>& pixel_override) {
    // Native source strings are copied using strlen, not the Lua byte length.
    return {vertex_override ? std::string(vertex_override->c_str()) : version < 3 ? "vs_2_a" : "vs_3_0",
        pixel_override ? std::string(pixel_override->c_str()) : version < 3 ? "ps_2_b" : "ps_3_0"};
}

std::vector<ShaderSystemConstant> make_system_constant_registry_00b5bf70() {
    return {
#include "shader_system_registry.inc"
    };
}

void append_zero_shader_fields_00b357d0(const std::string& instance,
    const std::vector<ShaderField>& fields, std::string& output) {
    for (const auto& field : fields) {
        output += "\t\t"; output += instance.c_str(); output += '.';
        output += field.name.c_str(); output += "=0;\n";
    }
}

void append_vertex_input_decode_00b35820(const std::vector<ShaderField>& fields,
    bool enabled, std::uint32_t field_limit, std::string& output) {
    if (!enabled) return;
    static constexpr const char* swizzles[] = {"", "x", "xy", "xyz", "xyzw"};
    for (std::size_t i = 0; i < fields.size() && i < field_limit; ++i) {
        const auto& field = fields[i];
        const char* swizzle = field.component_count <= 4 ? swizzles[field.component_count] : "";
        const auto index = std::to_string(i);
        output += "\tIN."; output += field.name.c_str(); output += "=IN.";
        output += field.name.c_str(); output += " * cVtxElemScale[";
        output += index; output += "]."; output += swizzle;
        output += " + cVtxElemOffset["; output += index; output += "].";
        output += swizzle; output += ";\n";
    }
}

namespace {
#include "shader_vertex_literals.inc"
#include "shader_shadow_literals.inc"
#include "shader_pixel_literals.inc"
std::string signed_decimal(std::uint32_t value) {
    // Native variadic %i interprets the full DWORD as signed, even though
    // dimension selection above one uses unsigned comparisons.
    return std::to_string(value <= 0x7fffffffu ? static_cast<std::int64_t>(value)
        : static_cast<std::int64_t>(value) - 0x100000000ll);
}
}

void append_shadow_helper_00b38230(bool projected_sampling, std::string& output) {
    output += shadow_begin; output += '\n';
    if (projected_sampling) {
        output += shadow_projected_position; output += '\n';
        output += shadow_projected_return; output += '\n';
    } else {
        output += shadow_filtered_body; output += '\n';
    }
    output += "}\n";
}

ShaderSourceStatus generate_pixel_source_00b39880(ShaderPixelProgram& program, std::string& output) {
    if (program.color_outputs < 1 || program.color_outputs > 4) return ShaderSourceStatus::invalid_packing;
    std::string source;
    auto line = [&](const std::string& text) { source += text; source += '\n'; };
    const char* mode = nullptr;
    switch (program.effect.render_mode) {
    case 0: mode = "NORMAL"; break; case 1: mode = "REFLECTION"; break;
    case 3: mode = "UNDERWATER"; break; case 4: mode = "REFRACTION"; break;
    case 6: mode = "DRAW_SHADOW"; break; case 7: mode = "MAP"; break;
    default: break;
    }
    if (mode) line(std::string("#define RM_") + mode + " 1");
    append_system_constant_header_00b38ff0(program.constants, true, program.register_limit, source);
    source += '\n'; source += program.base.header.c_str(); source += '\n';
    source += program.effect.header.c_str(); source += '\n';
    auto layout = program.interpolators;
    ShaderInterpolatorOptions io;
    io.include_fog = !program.zero_fog; io.allow_vpos = true; io.zero_fog = program.zero_fog;
    io.base_descriptor_vpos = program.base.vpos; io.effect_descriptor_vpos = program.effect.vpos;
    auto status = append_interpolator_struct_00b36e30(layout, io, source);
    if (status != ShaderSourceStatus::complete) return status;
    ShaderStructOptions so; so.first_field = 1; so.allow_vpos = true;
    so.base_descriptor_vpos = program.base.vpos; so.effect_descriptor_vpos = program.effect.vpos;
    status = append_shader_struct_00b38b50("sPixelIn", program.inputs, so, source);
    if (status != ShaderSourceStatus::complete) return status;
    so = {};
    status = append_shader_struct_00b38b50("sSysValues", program.system_values, so, source);
    if (status != ShaderSourceStatus::complete) return status;
    append_pixel_samplers_00b37ef0(program.base.samplers, program.effect.samplers, source);
    if (program.effect.shadow_helpers) {
        append_shadow_helper_00b38230(program.projected_shadow, source);
        append_map_shadow_helper_00b382b0(program.projected_shadow, source);
    }
    line(pixel_ambient_fog_helpers); line(pixel_srgb_helpers);
    line("\nvoid ShaderCode(sPixelIn IN, inout sSysValues SYS)");
    line("{"); line(program.base.pixel_code); line("}");
    const auto count = std::to_string(program.color_outputs);
    line("\nvoid EffectCode(sPixelIn IN, inout sSysValues SYS, out float4 FinalColor[" + count
        + (program.depth_output ? "], out float Depth)" : "])"));
    line("{"); line(program.effect.pixel_code); line("}");
    status = append_interpolator_unpack_00b37000(program.unpack_fields, layout, io, source);
    if (status != ShaderSourceStatus::complete) return status;
    source += "\nvoid main(sInterpolators INT";
    for (std::uint32_t i = 0; i < program.color_outputs; ++i) {
        const auto index = std::to_string(i);
        source += ", out float4 Color" + index + " : COLOR" + index;
    }
    if (program.depth_output) source += ", out float Depth : DEPTH";
    line(")"); source += "{\n\n";
    for (std::uint32_t i = 0; i < program.color_outputs; ++i) {
        if (i) source += ' ';
        source += "Color" + std::to_string(i) + "=0;";
    }
    if (program.depth_output) source += " Depth=0;";
    source += '\n';
    line("\t\tsPixelIn\t\tIN = UnpackInterpolators(INT);\n\t\tsSysValues\t\tSYS;\n\t\tfloat4\t\t\tFinalColors[" + count + "];");
    append_zero_shader_fields_00b357d0("SYS", program.system_values, source);
    line(program.depth_output ? "\n\t\t\tShaderCode(IN,SYS);\n\t\t\tEffectCode(IN,SYS,FinalColors,Depth);"
        : "\n\t\t\tShaderCode(IN,SYS);\n\t\t\tEffectCode(IN,SYS,FinalColors);");
    const auto render_mode = program.effect.render_mode;
    if (render_mode == 3 || render_mode == 9 || render_mode == 11)
        line("\nFinalColors[0].xyz = min(pow(FinalColors[0].xyz,0.25)*23,120*FinalColors[0].xyz);\n");
    if ((render_mode == 0 || render_mode == 12 || render_mode == 8 || render_mode == 10)
        && !program.base.suppress_time_transform)
        line("\nif(cElapsedTime[1]>0.5)\nFinalColors[0].xyz = min(pow(FinalColors[0].xyz,0.25)*23,120*FinalColors[0].xyz);\n");
    const bool fog = layout.fog.field != 0xff && !program.zero_fog;
    line("\nColor0=FinalColors[0];\n");
    if (fog) line("Color0.rgb=lerp(cFogDirColor,Color0.rgb,INT.Fog);\n");
    if (program.effect.alpha_override)
        line(program.visibility_alpha ? "Color0.a=SYS.DiffuseColor.a * saturate(cVisibility);\n"
            : "Color0.a=SYS.DiffuseColor.a;\n");
    if (!fog && program.base.premultiply_alpha) line("Color0.rgb *= Color0.a;");
    for (std::uint32_t i = 1; i < program.color_outputs; ++i) {
        const auto index = std::to_string(i);
        line("\nColor" + index + "=FinalColors[" + index + "];");
    }
    line("\n}");
    output = std::move(source); program.interpolators = std::move(layout);
    return ShaderSourceStatus::complete;
}

void append_map_shadow_helper_00b382b0(bool projected_sampling, std::string& output) {
    output += map_shadow_begin; output += '\n';
    if (projected_sampling) {
        output += shadow_projected_position; output += '\n';
        output += map_shadow_projected_return; output += '\n';
    } else {
        output += map_shadow_filtered_body; output += '\n';
    }
    output += "}\n";
}

ShaderSourceStatus generate_vertex_source_00b39110(ShaderVertexProgram& program,
    std::string& output) {
    std::string source;
    const char* mode = nullptr;
    switch (program.effect.render_mode) {
    case 0: mode = "NORMAL"; break;
    case 1: mode = "REFLECTION"; break;
    case 3: mode = "UNDERWATER"; break;
    case 4: mode = "REFRACTION"; break;
    case 6: mode = "DRAW_SHADOW"; break;
    case 7: mode = "MAP"; break;
    default: break;
    }
    if (mode) { source += "#define RM_"; source += mode; source += " 1\n"; }
    append_system_constant_header_00b38ff0(program.constants, true, program.register_limit, source);
    source += '\n'; source += program.base.header.c_str(); source += '\n';
    source += program.effect.header.c_str(); source += '\n';
    ShaderStructOptions options; options.include_semantics = true;
    auto status = append_shader_struct_00b38b50("sVertexIn", program.inputs, options, source);
    if (status != ShaderSourceStatus::complete) return status;
    options.include_semantics = false;
    status = append_shader_struct_00b38b50("sSysValues", program.system_values, options, source);
    if (status != ShaderSourceStatus::complete) return status;
    status = append_shader_struct_00b38b50("sVertexOut", program.outputs, options, source);
    if (status != ShaderSourceStatus::complete) return status;
    auto layout = program.interpolators;
    ShaderInterpolatorOptions interpolator_options;
    interpolator_options.include_position = true; interpolator_options.include_fog = true;
    status = append_interpolator_struct_00b36e30(layout, interpolator_options, source);
    if (status != ShaderSourceStatus::complete) return status;
    append_vertex_samplers_00b38080(program.base.samplers, program.effect.samplers, source);
    if (program.effect.shadow_helper) { source += vertex_shadow_helper; source += '\n'; }
    source += vertex_ambient_fog_helpers; source += '\n';
    source += "\nvoid ShaderCode(sVertexIn IN, inout sSysValues SYS, inout sVertexOut OUT)\n{\n";
    source += program.base.vertex_code; source += "\n}\n";
    source += "\nvoid EffectCode(inout sSysValues SYS, inout sVertexOut OUT)\n{\n";
    source += program.effect.vertex_code; source += "\n}\n";
    status = append_interpolator_pack_00b35540(program.packing_fields, layout, source);
    if (status != ShaderSourceStatus::complete) return status;
    source += "\nsInterpolators main(sVertexIn IN)\n{\n\t\tsSysValues\t\tSYS;\n\t\tsVertexOut\t\tOUT;\n\n";
    append_zero_shader_fields_00b357d0("SYS", program.system_values, source);
    append_zero_shader_fields_00b357d0("OUT", program.outputs, source);
    append_vertex_input_decode_00b35820(program.inputs, program.base.decode_inputs,
        program.base.decode_field_limit, source);
    source += "\n\t\tShaderCode(IN,SYS,OUT);\n\t\tEffectCode(SYS,OUT);\n\t\t\n\t\tOUT.ScreenSpacePos = SYS.ScreenSpacePos;\n\n";
    source += "\t\treturn PackInterpolators(OUT);\n}\n";
    output = std::move(source);
    program.interpolators = std::move(layout);
    return ShaderSourceStatus::complete;
}

namespace {
void append_stage_samplers(const std::vector<ShaderSamplerDeclaration>& base,
    const std::vector<ShaderSamplerDeclaration>& effect, bool vertex_stage, std::string& output) {
    std::uint32_t slot = 0;
    for (const auto* descriptor : {&base, &effect}) {
        for (const auto& sampler : *descriptor) {
            if (sampler.vertex_stage != vertex_stage) continue;
            const char* type = nullptr;
            switch (sampler.dimension) {
            case 1: type = "sampler1D"; break;
            case 2: type = "sampler2D"; break;
            case 3: type = "samplerCUBE"; break;
            case 4: type = "sampler3D"; break;
            default: break;
            }
            if (type) {
                output += type; output += '\t'; output += sampler.name.c_str();
                output += "\t\t: register(s"; output += signed_decimal(slot); output += ");\n";
            }
            ++slot;
        }
    }
}
}

void append_vertex_samplers_00b38080(const std::vector<ShaderSamplerDeclaration>& base,
    const std::vector<ShaderSamplerDeclaration>& effect, std::string& output) {
    append_stage_samplers(base, effect, true, output);
}

void append_pixel_samplers_00b37ef0(const std::vector<ShaderSamplerDeclaration>& base,
    const std::vector<ShaderSamplerDeclaration>& effect, std::string& output) {
    append_stage_samplers(base, effect, false, output);
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
