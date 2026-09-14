#pragma once
#include "bsp/native_compiled_shader_reflection.hpp"
#include "bsp/native_d3d9_shader_construction_actual.hpp"
#include "bsp/native_material_descriptor_samplers.hpp"
#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_material_pass_shader_slots.hpp"
#include "bsp/native_pixel_shader_compilation.hpp"
#include "bsp/native_shader_compiler_lookups.hpp"
#include "bsp/native_shader_interpolators.hpp"
#include "bsp/native_shader_system_fields.hpp"
#include "bsp/native_shader_pixel_source.hpp"
#include "bsp/native_shader_vertex_source.hpp"

namespace bsp {
// These callbacks register host metadata for the SAME newly produced native
// owners in the application's existing NativeRenderActualOwners domain. They
// neither mutate raw storage nor retain, copy, delete or substitute an owner.
// Registration must finish without throwing before the next native call.
struct NativeMaterialProgramCompilerActualRegistration {
    void* context;
    void (*bind_pass)(void*, NativeMaterialPassStorage&) noexcept;
    void (*bind_reflection)(void*, NativeCompiledShaderStorage&) noexcept;
    void (*bind_shader)(void*, NativeD3d9ShaderStorage&) noexcept;
};
struct NativeMaterialProgramCompilerActualContext {
    NativeStringStorage& strings;
    const volatile std::uint8_t& load_variants_0108d6f0;
    const volatile std::uint8_t& source_mode_0108d6f1;
    NativeShaderBinaryCacheStorage* const volatile& actual_cache_0108d6ec;
    void* const volatile& actual_renderer_00f8d394;
    NativeMaterialPassConstructionAccess& pass_construction;
    NativeTextureCacheContext& texture_cache;
    NativeShaderFieldInitializationContext& source;
    NativeVertexShaderCompilationContext& vertex_compilation;
    NativePixelShaderCompilationContext& pixel_compilation;
    NativeCompiledShaderReflectionContext& reflection;
    NativeD3d9ShaderConstructionActualContext& shader_construction;
    NativeMaterialProgramCompilerActualRegistration registration;
    // Two native calls have distinct caller-supplied entry/residue evidence.
    // Both contexts still borrow the SAME owner, pool and renderer domains.
    NativeMaterialDescriptorSamplersContext& root_samplers;
    NativeMaterialDescriptorSamplersContext& mode_samplers;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_physical_stream_profile_00d691b0;
    const char* actual_empty_0108d6f2;
    const char* actual_stream_empty_0109db64;
};

// Persistent acquisition/continuation metadata, attached to the original
// compiler frame's tail_child BEFORE any native continuation effect. Every
// child remains alive on failure; no automatic acknowledgement or rollback.
// A normal null return leaves the native orphaned pass/metadata/COM references
// alive in their original domains. This frame never deletes those raw owners.
struct NativeMaterialProgramCompilerActualFrame final : NativeMaterialProgramChildFrame {
    enum class Phase { fresh, running, complete, failed };
    struct Temporary {
        NativeString name;
        char* captured_data{};
        std::uint32_t captured_length{};
        bool entered{}, live{}, captured{};
    };
    NativeMaterialProgramCompilerActualFrame() {} // Preserve private scratch preimages.
    ~NativeMaterialProgramCompilerActualFrame() override;
    NativeMaterialProgramCompilerActualFrame(const NativeMaterialProgramCompilerActualFrame&) = delete;
    NativeMaterialProgramCompilerActualFrame& operator=(const NativeMaterialProgramCompilerActualFrame&) = delete;
    Phase phase{Phase::fresh};
    NativeMaterialProgramCompilerFrame* parent{};
    NativeMaterialProgramCompilerActualContext* context{};
    NativeMaterialProgramCompilerResume resume{};
    std::uint32_t native_site{}, exception_state{};
    void* raw_pass{};
    NativeMaterialPassStorage* pass{};
    NativeCompiledShaderStorage* reflection_owners[2]{};
    IDirect3DPixelShader9* preliminary_pixel_orphan{};
    IDirect3DVertexShader9* vertex_shader{};
    IDirect3DPixelShader9* pixel_shader{};
    void* vertex_bytecode{};
    void* pixel_bytecode{};
    NativeD3d9ShaderStorage* shader_owners[2]{};
    bool pass_registered{}, reflection_registered[2]{}, shader_registered[2]{};
    bool vertex_com_released{}, pixel_com_released{}, vertex_owner_released{}, pixel_owner_released{};
    bool vertex_texture_unsupported{}, native_null_return{};
    std::uint32_t vertex_size, pixel_size, final_pixel_sentinel;
    std::array<std::uint32_t, 10> texcoord_masks;
    std::array<std::uint32_t, 2> color_masks;
    // Inline resize(0) can orphan pointer elements exactly as native; record
    // acquired array backing before copying/freeing/publishing it.
    void* array_allocation{};
    void* array_header{};
    std::uint32_t array_native_site{};
    std::array<Temporary, 12> names;
    std::unique_ptr<NativeMaterialCompilerPassConstructionFrame> pass_constructor;
    std::unique_ptr<NativeShaderSystemFieldsOperation> vertex_system, pixel_system;
    std::array<std::unique_ptr<NativeShaderInterpolatorOperation>, 5> interpolators;
    std::unique_ptr<NativeShaderPixelSourceOperation> preliminary_source, pixel_source;
    std::unique_ptr<NativeShaderVertexSourceOperation> vertex_source;
    std::unique_ptr<NativePixelShaderCompilationOperation> preliminary_compile, pixel_compile;
    std::unique_ptr<NativeVertexShaderCompilationOperation> vertex_compile;
    std::array<std::unique_ptr<NativeCompiledShaderReflectionOperation>, 2> reflections;
    std::array<std::unique_ptr<NativeShaderCompilerLookupOperation>, 8> lookups;
    std::array<std::unique_ptr<NativeMaterialDescriptorSamplersOperation>, 2> samplers;
    std::array<std::unique_ptr<NativeMaterialPassShaderSlotsOperation>, 18> slots;
};

// Full B3B3C0 continuation at the two exact prefix handoff addresses. Native
// B3B3C0 ECX builder, stack effect/root descriptor/mode descriptor, RET0Ch.
// It borrows the already initialized builder/name and never replays B35930.
// Source interfaces preserve recovered normal effects and retain host failures;
// original private stack aliases, incidental registers and FH3/SEH are separate.
class NativeMaterialProgramCompilerActual final : public NativeMaterialProgramCompilerTail {
public:
    explicit NativeMaterialProgramCompilerActual(NativeMaterialProgramCompilerActualContext& context)
        : context_(context) {}
    NativeMaterialPassStorage* continue_material_pass_00b3b3c0(
        NativeMaterialProgramCompilerFrame&, NativeMaterialProgramCompilerResume,
        NativeMaterialProgramChild&) override;
private:
    NativeMaterialProgramCompilerActualContext& context_;
};
} // namespace bsp
