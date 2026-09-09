#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
enum class ShaderScalarType : std::uint32_t { floating = 0, integer = 1 };
enum class ShaderSemantic : std::uint32_t {
    position = 0, color, texcoord, normal, binormal, tangent,
    blendindices, blendweight, fog, index, vpos
};

// New projection of the native 1Ch record, not its string/storage ABI.
struct ShaderField {
    std::string name; // Native +0h length, +4h data.
    ShaderScalarType scalar_type{ShaderScalarType::floating}; // +8h.
    std::uint32_t component_count{1}; // +Ch; unsigned decimal suffix only if >1.
    ShaderSemantic semantic{ShaderSemantic::position}; // +14h.
    std::uint32_t semantic_index{}; // +18h; unsigned decimal, including zero.
    std::uint32_t component_mask{0xf}; // +10h; packing consumes low four bits.
};

enum class ShaderSourceStatus { complete, unsupported_scalar_type, unsupported_semantic, invalid_packing };

struct ShaderPackedComponent {
    std::uint8_t field{}, component{};
};
struct ShaderInterpolatorLayout {
    std::vector<ShaderPackedComponent> texcoords, colors; // Builder +54h, +60h.
    ShaderPackedComponent fog{0xff, 0}; // +6Ch; field FF means absent.
    std::uint32_t texcoord_registers{}, texcoord_last_width{4}; // +7Ch, +80h.
    std::uint32_t color_registers{}, color_last_width{4}; // +84h, +88h.
};
struct ShaderInterpolatorOptions {
    bool include_position{}, include_fog{}, allow_vpos{};
    bool base_descriptor_vpos{}, effect_descriptor_vpos{};
    bool zero_fog{}; // Builder +98h, consumed by unpacking only.
};
// Native thiscall RET4: appends without clearing; skips field zero, ignores
// component_count, truncates field indices to a byte; last FOG field wins.
void append_interpolator_mapping_00b34aa0(const std::vector<ShaderField>&,
    ShaderInterpolatorLayout&);
// Native thiscall RET0Ch / RET4. New typed APIs append source. Invalid counts
// or consumed mapping references leave source/metadata unchanged. No allocator ABI.
ShaderSourceStatus append_interpolator_struct_00b36e30(ShaderInterpolatorLayout&,
    const ShaderInterpolatorOptions&, std::string& output);
ShaderSourceStatus append_interpolator_unpack_00b37000(const std::vector<ShaderField>&,
    const ShaderInterpolatorLayout&, const ShaderInterpolatorOptions&, std::string& output);
// Native ECX builder, RET; fields are builder+28h. Always writes Position from
// OUT.ScreenSpacePos, then mapped TEXCOORD/COLOR and optional Fog. No vPos write.
ShaderSourceStatus append_interpolator_pack_00b35540(const std::vector<ShaderField>&,
    const ShaderInterpolatorLayout&, std::string& output);

struct ShaderStructOptions {
    std::uint32_t first_field{};
    bool include_semantics{};
    bool allow_vpos{};
    bool base_descriptor_vpos{};   // Native descriptor at builder+70h, byte+30h.
    bool effect_descriptor_vpos{}; // Native descriptor at builder+74h, byte+30h.
};

// 00b385b0: ECX field, output-string pointer and semantics flag on stack,
// RET8; EAX output pointer. This new API replaces output on success.
// 00b38b50: ECX builder, start/name/list/semantics/allow-vPos stack args,
// RET14h. This new API appends to output on success.
//
// Explicit unsupported-enum statuses are new interface behavior; no native
// fallback HLSL tokens are invented. On status failure output is unchanged.
// Component counts are formatted as native (0/1 omit suffix); this is not an
// HLSL validator. Names are not escaped. Struct emission applies native %s
// termination at embedded NULs, whereas field formatting retains name length.
// Allocation exceptions follow standard C++ string/vector behavior.
// Evidence: docs/SHADER_SOURCE_GENERATION.md and its JSON report. Neither
// helper generates a complete shader, applies analysis names, or compiles HLSL.
ShaderSourceStatus format_shader_field_00b385b0(const ShaderField&,
    bool include_semantic, std::string& output);
ShaderSourceStatus append_shader_struct_00b38b50(const std::string& name,
    const std::vector<ShaderField>& fields, const ShaderStructOptions&,
    std::string& output);
}
