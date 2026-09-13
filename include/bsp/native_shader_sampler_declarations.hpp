#pragma once
#include "bsp/native_shader_constant_header.hpp"
#include "bsp/native_shader_sampler_owner.hpp"
#include <optional>

namespace bsp {
// Host continuation storage; no words are added to the actual B0h builder,
// 110h descriptors or 2Ch sampler records. The same B35110 child owns all
// formatting acquisitions. Its failed state stays available for diagnosis.
// Keep this frame, context, builder, current/replaceable descriptors, rows,
// sampler names, scratch and string domain alive while running/failed, and
// exclude their retirement externally. No native terminal interception or
// FH3 unwind is installed. A failed operation cannot be replayed.
struct NativeShaderSamplerDeclarationsOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, descriptor_offset{}, index{}, slot{};
    std::uint32_t completed_rows{}, emitted_lines{}, sampled_type{};
    std::uint8_t sampled_vertex_byte{};
    NativeMaterialProgramBuilderStorage* builder{};
    NativeShaderConstantHeaderContext* context{};
    NativeShaderDescriptorStorage* descriptor{};
    void* sampler_data{};
    NativeShaderSamplerStorage* sampler{};
    const char* name{};
    std::optional<NativeShaderConstantHeaderOperation> line;
    NativeShaderSamplerDeclarationsOperation() = default;
    ~NativeShaderSamplerDeclarationsOperation();
    NativeShaderSamplerDeclarationsOperation(const NativeShaderSamplerDeclarationsOperation&) = delete;
    NativeShaderSamplerDeclarationsOperation& operator=(const NativeShaderSamplerDeclarationsOperation&) = delete;
    // Only after the caller resolves every retained child acquisition. Also
    // acknowledges the failed B35110 child; this does not free native storage.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B37EF0/B38080: ECX actual B0h builder, no stack arguments, plain RET.
// Complete normal bodies: descriptor70 then mode_descriptor74, current C4/C8
// pointer lists, same slot counter across both. Pixel selects record BYTE0C
// zero; vertex selects nonzero. Selected unknown types consume a slot without
// text. Known 1/2/3/4 emit 1D/2D/CUBE/3D via existing B35110 into source4C.
// Descriptor/count/data reloads survive formatting callbacks. Actual valid
// pointers/extents are required; null descriptors/records are not empty lists.
// These explicit C++ interfaces are not original ABI replacements.
void append_native_pixel_sampler_declarations_00b37ef0(
    NativeMaterialProgramBuilderStorage&, NativeShaderConstantHeaderContext&,
    NativeShaderSamplerDeclarationsOperation&);
void append_native_vertex_sampler_declarations_00b38080(
    NativeMaterialProgramBuilderStorage&, NativeShaderConstantHeaderContext&,
    NativeShaderSamplerDeclarationsOperation&);
} // namespace bsp
