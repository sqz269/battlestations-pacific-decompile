#include "bsp/native_shader_shadow_source.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
#include "shader_shadow_literals.inc"
void line(NativeShaderShadowSourceOperation& a, const char* text, std::uint32_t site) {
    a.text = text;
    a.native_site = site;
    a.line.emplace(); // Replaces only a returned, complete child.
    append_native_shader_cstring_line_00b34f20(a.builder->source_4c, text, *a.context, *a.line);
    ++a.completed_lines;
}
void append(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderConstantHeaderContext& context, NativeShaderShadowSourceOperation& a, bool map) {
    using Phase = NativeShaderShadowSourceOperation::Phase;
    if (a.phase != Phase::fresh) throw std::logic_error("shadow source operation is one-shot");
    a.phase = Phase::running;
    a.function = map ? 0x00b382b0u : 0x00b38230u;
    a.builder = &builder;
    a.context = &context;
    try {
        line(a, map ? map_shadow_begin : shadow_begin, map ? 0x00b382beu : 0x00b3823eu);
        // B38243/B382C3: the intro's pooled callbacks run before this read.
        a.sampled_shadow_byte = *reinterpret_cast<const volatile std::uint8_t*>(
            reinterpret_cast<const char*>(&builder) + 0xaa);
        a.shadow_byte_captured = true;
        if (a.sampled_shadow_byte != 0) {
            line(a, shadow_projected_position, map ? 0x00b382d3u : 0x00b38253u);
            line(a, map ? map_shadow_projected_return : shadow_projected_return,
                map ? 0x00b382dfu : 0x00b3825fu);
            line(a, "}", map ? 0x00b382ebu : 0x00b3826bu);
        } else {
            line(a, map ? map_shadow_filtered_body : shadow_filtered_body,
                map ? 0x00b382f8u : 0x00b38278u);
            line(a, "}", map ? 0x00b38304u : 0x00b38284u);
        }
        a.phase = Phase::complete;
    } catch (...) { a.phase = Phase::failed; throw; }
}
} // namespace
NativeShaderShadowSourceOperation::~NativeShaderShadowSourceOperation() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
void NativeShaderShadowSourceOperation::acknowledge_diagnostic_cleanup() noexcept {
    if (phase == Phase::failed) {
        if (line) line->acknowledge_diagnostic_cleanup();
        phase = Phase::diagnostic_retired;
    }
}
void append_native_shadow_helper_00b38230(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderConstantHeaderContext& context, NativeShaderShadowSourceOperation& operation) {
    append(builder, context, operation, false);
}
void append_native_map_shadow_helper_00b382b0(NativeMaterialProgramBuilderStorage& builder,
    NativeShaderConstantHeaderContext& context, NativeShaderShadowSourceOperation& operation) {
    append(builder, context, operation, true);
}
} // namespace bsp
