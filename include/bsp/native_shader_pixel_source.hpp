#pragma once
#include "bsp/native_shader_field_initialization.hpp"
#include "bsp/native_shader_interpolator_source.hpp"
#include "bsp/native_shader_sampler_declarations.hpp"
#include "bsp/native_shader_shadow_source.hpp"
#include <array>

namespace bsp {
// Persistent host metadata, never added to actual B0h/110h/1Ch owners. Keep
// this frame, both input lists, all replaceable descriptors/fields, context,
// scratch and string domain alive and exclude retirement while running/failed.
// Failed work cannot be replayed. No outer FH3 unwind or implicit rollback.
struct NativeShaderPixelSourceOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    struct Temporary {
        NativeString header;
        bool entered{}, returned{}, live{}, captured{};
        char* captured_data{};
        std::uint32_t captured_length{};
    };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{}, sampled_count{}, initial_length{};
    std::uint8_t sampled_depth{}, sampled_alpha{};
    NativeMaterialProgramBuilderStorage* builder{};
    const NativeShaderDescriptorArray* declaration_fields{};
    const NativeShaderDescriptorArray* unpack_fields{};
    NativeShaderFieldInitializationContext* context{};
    char* initial_data{};
    // 0: reusable literal/name; 1: suffix; 2: number; 3: prefix;
    // 4: prefix+number; 5: final concatenation. Entered-but-not-returned
    // headers retain evidence only: existing constructor/concat failure
    // cleanup determines whether they still own any acquisition.
    std::array<Temporary, 6> temporary;
    std::unique_ptr<NativeShaderStructDeclarationOperation> line;
    std::unique_ptr<NativeShaderConstantHeaderOperation> constants;
    std::unique_ptr<NativeShaderInterpolatorSourceOperation> interpolators;
    std::unique_ptr<NativeShaderSamplerDeclarationsOperation> samplers;
    std::unique_ptr<NativeShaderShadowSourceOperation> shadow;
    std::unique_ptr<NativeShaderFieldInitializationOperation> initialization;
    NativeShaderPixelSourceOperation() = default;
    ~NativeShaderPixelSourceOperation();
    NativeShaderPixelSourceOperation(const NativeShaderPixelSourceOperation&) = delete;
    NativeShaderPixelSourceOperation& operator=(const NativeShaderPixelSourceOperation&) = delete;
    // Only AFTER explicit cleanup of each retained acquisition and child.
    // Does not free actual strings, complete native work, or permit replay.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// B39880 ECX actual B0h builder, stacked actual0Ch declaration/unpack lists,
// RET8. Complete normal body: rebuilds SAME source4C using unchanged actual
// helpers. A zero-length old source leaves its data word untouched. Unknown
// MRT count skips main signature/initialization, then continues emission.
// Descriptor/list/count rereads and captured cleanup operands follow original
// instructions; no semantic projection, shader compilation or owner creation.
// Existing field context supplies both distinct native empty-string cells.
// Explicit C++ interface; not an original ABI/FH3 replacement.
void build_native_shader_pixel_source_00b39880(NativeMaterialProgramBuilderStorage&,
    const NativeShaderDescriptorArray& declaration_fields,
    const NativeShaderDescriptorArray& unpack_fields,
    NativeShaderFieldInitializationContext&, NativeShaderPixelSourceOperation&);
} // namespace bsp
