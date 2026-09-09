#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace bsp {
struct ShaderProfiles { std::string vertex, pixel; };
// Descriptor reader00b43b00 profile-selection fragment: unsigned version<3
// defaults to vs_2_a/ps_2_b, otherwise vs_3_0/ps_3_0. Optional values represent
// fields that passed the native string-type check; absent/non-string use defaults.
ShaderProfiles select_shader_profiles_00b43b00(std::uint32_t version,
    const std::optional<std::string>& vertex_override = {},
    const std::optional<std::string>& pixel_override = {});
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

// Thiscall RET8: instance name and field list. Emits zero assignments for all
// fields, independent of their type/count/mask; native %s name termination.
void append_zero_shader_fields_00b357d0(const std::string& instance,
    const std::vector<ShaderField>&, std::string& output);
// ECX builder, RET. Base descriptor +1Fh enables decoding; +20h bounds the
// prefix of input fields. Swizzle helper00b34e90 uses count alone (1..4),
// returning empty for other counts, even if that produces invalid HLSL.
void append_vertex_input_decode_00b35820(const std::vector<ShaderField>&,
    bool enabled, std::uint32_t field_limit, std::string& output);

struct ShaderSamplerDeclaration {
    std::string name; // Native string at +4h/+8h.
    bool vertex_stage{}; // +Ch: nonzero vertex, zero pixel (not disabled).
    std::uint32_t dimension{}; // +10h: 1=1D, 2=2D, 3=CUBE, 4=3D.
};
// ECX builder, RET. Walks base then effect; disabled records consume no slot,
// enabled unknown dimensions consume a slot but emit nothing. No deduplication.
void append_vertex_samplers_00b38080(const std::vector<ShaderSamplerDeclaration>& base,
    const std::vector<ShaderSamplerDeclaration>& effect, std::string& output);
// Same algorithm for records whose +Ch is zero; its slot counter is independent.
void append_pixel_samplers_00b37ef0(const std::vector<ShaderSamplerDeclaration>& base,
    const std::vector<ShaderSamplerDeclaration>& effect, std::string& output);
// ECX builder, RET; +AAh selects projected sampling instead of the four-tap
// comparison path. Emits original HLSL literals plus helper-added linefeeds.
void append_shadow_helper_00b38230(bool projected_sampling, std::string& output);
void append_map_shadow_helper_00b382b0(bool projected_sampling, std::string& output);

// Projection of the native 20h system constant record. Native declaration
// order is float<first>x<second>; register advance is second*array_count.
struct ShaderSystemConstant {
    std::string name; // +14h/+18h native string, getter00b5b820.
    std::uint32_t second_dimension{1}; // +8, getter00b5b840.
    std::uint32_t first_dimension{1};  // +C, getter00b5b850.
    std::uint32_t array_count{1};      // +10, getter00b5b860.
    std::uint32_t semantic_id{};      // +1Ch; native constant source selector.
};
// Typed ordered registry projection from constructor00b5bf70; does not recreate
// singleton registration, intrusive storage or renderer value population.
std::vector<ShaderSystemConstant> make_system_constant_registry_00b5bf70();
inline constexpr std::uint32_t system_constant_annotation_limit = 77; // Disk/saved00e13078.
// Thiscall RET8 and RET4 respectively. New explicit records replace native
// singleton00b5b890. The header register limit is native global00e13078.
void append_system_constant_00b38c60(const ShaderSystemConstant&,
    std::int32_t register_index, std::string& output);
void append_system_constant_header_00b38ff0(const std::vector<ShaderSystemConstant>&,
    bool explicit_registers, std::uint32_t register_limit, std::string& output);

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

struct ShaderVertexDescriptor {
    std::string header, vertex_code; // Native +E8h, +F0h strings.
    std::vector<ShaderSamplerDeclaration> samplers;
    std::int32_t render_mode{}; // Effect +10Ch.
    bool shadow_helper{}; // Effect +15h.
    bool decode_inputs{}; // Base +1Fh.
    std::uint32_t decode_field_limit{}; // Base +20h.
};
struct ShaderVertexProgram {
    ShaderVertexDescriptor base, effect;
    std::vector<ShaderField> inputs, system_values, outputs, packing_fields;
    // Four distinct native lists: +4h, +10h, +1Ch, +28h respectively.
    std::vector<ShaderSystemConstant> constants;
    std::uint32_t register_limit{};
    ShaderInterpolatorLayout interpolators;
};
// Full source composition at00b39110, ECX builder, RET. Replaces output and
// updates interpolator metadata on success; typed validation failure leaves
// both unchanged. Descriptor selection/registry population/compilation separate.
ShaderSourceStatus generate_vertex_source_00b39110(ShaderVertexProgram&, std::string& output);

struct ShaderPixelDescriptor {
    std::string header, pixel_code;
    std::vector<ShaderSamplerDeclaration> samplers;
    std::int32_t render_mode{};
    bool shadow_helpers{}, vpos{}, alpha_override{}; // Effect +15h, either +30h, effect +31h.
    bool suppress_time_transform{}, premultiply_alpha{}; // Base +1Dh/+32h.
};
struct ShaderPixelProgram {
    ShaderPixelDescriptor base, effect;
    std::vector<ShaderField> inputs, unpack_fields, system_values;
    std::vector<ShaderSystemConstant> constants;
    std::uint32_t register_limit{}, color_outputs{1};
    bool depth_output{}, zero_fog{}, visibility_alpha{}, projected_shadow{};
    ShaderInterpolatorLayout interpolators;
};
// 00b39880 thiscall RET8, two distinct input-list arguments. Explicit typed
// state replaces builder/registry. Invalid output counts outside1..4 return
// invalid_packing rather than emitting native malformed source.
ShaderSourceStatus generate_pixel_source_00b39880(ShaderPixelProgram&, std::string& output);
}
