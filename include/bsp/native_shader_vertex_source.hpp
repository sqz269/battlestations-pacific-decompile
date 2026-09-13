#pragma once
#include "bsp/native_shader_field_initialization.hpp"
#include "bsp/native_shader_interpolator_source.hpp"
#include "bsp/native_shader_sampler_declarations.hpp"

namespace bsp {
// Persistent host continuation metadata, outside the actual B0h builder and
// its 8h string. Keep this frame, builder, descriptors, registry, scratch and
// string domain alive and exclude retirement during running/failed work.
// Child frames retain their own native acquisitions and failure boundaries.
// This is not recovered FH3 unwind; no replay, rollback or implicit cleanup.
struct NativeShaderVertexSourceOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    enum class Step { none, source_release, mode, headers, temporary_resize,
        temporary_copy, temporary_release, child };
    Phase phase{Phase::fresh};
    Step step{Step::none};
    std::uint32_t native_site{}, mode{}, captured_length{};
    NativeMaterialProgramBuilderStorage* builder{};
    NativeShaderFieldInitializationContext* context{};
    NativeString temporary;
    bool temporary_live{}, temporary_captured{};
    char* captured_data{};
    const char* base_header{};
    const char* mode_header{};
    std::unique_ptr<NativeShaderStructDeclarationOperation> string_child;
    std::unique_ptr<NativeShaderConstantHeaderOperation> constant_child;
    std::unique_ptr<NativeShaderInterpolatorSourceOperation> interpolator_child;
    std::unique_ptr<NativeShaderSamplerDeclarationsOperation> sampler_child;
    std::unique_ptr<NativeShaderFieldInitializationOperation> field_child;
    NativeShaderVertexSourceOperation() = default;
    ~NativeShaderVertexSourceOperation();
    NativeShaderVertexSourceOperation(const NativeShaderVertexSourceOperation&) = delete;
    NativeShaderVertexSourceOperation& operator=(const NativeShaderVertexSourceOperation&) = delete;
    // Only after explicit native acquisition cleanup and retirement of failed
    // children. Does not release buffers, clear native headers or resume work.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete normal B39110, ECX actualB0h builder, no stack arguments, RET.
// Clears builder4C only when its length is nonzero, then emits the complete
// vertex source through the unchanged actual native children. Descriptor70/74
// reads occur at their native stages; E8 headers use C strings and F0 bodies
// use actual counted strings. Fields04/10/1C/28 and maps54/60 stay distinct.
// Source generation alone: B3B3C0's compilation/consumer continuation is not
// entered. This host interface is not a drop-in x86 ABI replacement.
void generate_native_shader_vertex_source_00b39110(NativeMaterialProgramBuilderStorage&,
    NativeShaderFieldInitializationContext&, NativeShaderVertexSourceOperation&);
} // namespace bsp
