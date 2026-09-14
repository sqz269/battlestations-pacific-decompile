#pragma once
#include "bsp/native_material_pass_owner.hpp"
#include "bsp/native_material_texture_queries.hpp"
#include "bsp/native_render_batch_preparation_actual.hpp"
#include "bsp/native_renderer_vertex_binding.hpp"
#include "bsp/native_renderer_material_state_binding.hpp"
#include "bsp/native_renderer_indexed_draw.hpp"

namespace bsp {
struct NativeMaterialConstantBuildContext;
struct NativeMaterialConstantBuildFrame;

// Concrete, separately reconstructed B42350 dependency. There is deliberately
// no implementation or fallback here: an executable reaching the derived pass
// must link that complete raw provider and retain its caller-owned frame.
void build_native_material_constants_00b42350(void* actual_pass,
    void* actual_entry, void* actual_override, NativeMaterialConstantBuildContext&,
    NativeMaterialConstantBuildFrame&);

struct NativeMaterialPassExecutionContext {
    void* const volatile& actual_renderer_00f8d394;
    void* const volatile& actual_diagnostics_00f8d39c;
    void* volatile& actual_cached_effect_0108fbf4;
    const volatile std::uint32_t& actual_system_register_count_00e13078;
    void* actual_vertex_constants_0108ebf4;
    void* actual_pixel_constants_0108dbec;
    const volatile float& actual_negative_zero_00d7a208;
    const volatile float& actual_zero_00d7a218;
    const volatile float& actual_one_00d7a24c;
    NativeRendererSynchronizationGlobals& synchronization;
    NativeRendererVertexBindingContext& vertices;
    NativeRendererIndexBindingContext& indices;
    NativeRendererVertexLayoutBindingContext& layouts;
    NativeRendererTextureBindingContext& textures;
    NativeRendererIndexedDrawContext& indexed_draw;
    const NativeRendererMaterialStateBindingContext& material_states;
    NativeRenderBatchPreparationContext& queue;
    NativeStringRawPoolContext& diagnostic_strings;
    NativePostEffectFrameAtomic const volatile& actual_increment_00ce221c;
    NativeMaterialConstantBuildContext& constants;
    // Borrow current original token views, never callable code tables. The
    // renderer view spans +140h; logical vertex spans +28h; pass views +08h.
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_vertex_profile_00d61d6c;
    const volatile std::uint32_t* actual_base_pass_profile_00d62a80;
    const volatile std::uint32_t* actual_derived_pass_profile_00d61be8;
};

// This frame owns no native object. Its referenced constant-builder frame is
// acquired by the caller and stays alive on failure. A failed execution cannot
// be reused; no child acknowledgement, cleanup disarm or rollback is performed.
struct NativeMaterialPassExecutionFrame {
    enum class Phase { fresh, running, complete, failed };
    NativeMaterialConstantBuildFrame& constants;
    Phase phase = Phase::fresh;
    std::uint32_t stage = 0;
    explicit NativeMaterialPassExecutionFrame(NativeMaterialConstantBuildFrame& c)
        : constants(c) {}
    NativeMaterialPassExecutionFrame(const NativeMaterialPassExecutionFrame&) = delete;
    NativeMaterialPassExecutionFrame& operator=(const NativeMaterialPassExecutionFrame&) = delete;
};

// Exact original three-byte base virtual08, ECX unused, one unused stack word,
// RET4. Even null pass/entry are unexamined. New unused EDX preserves RET4.
void __fastcall execute_native_material_pass_base_00b5e5e0(
    void* actual_pass, void* unused_edx, void* actual_entry) noexcept;
// B1BFA0 exact MOV EAX,[ECX+30]; RET, returning borrowed actual context.
void* __fastcall get_native_material_plane_context_00b1bfa0(const void*) noexcept;

// Full B43410 (600 bytes): ECX pass, six original stack words, RET18h.
// The unused index-owner word is carried explicitly. The low BYTE of indexed
// selects the draw; VS/PS count arithmetic and counters wrap at32 bits.
void apply_native_material_pass_00b43410(void* actual_pass, void* actual_entry,
    void* actual_override, std::uint32_t indexed, void* actual_index_owner,
    std::uint32_t index_base, std::uint32_t vertex_base,
    NativeMaterialPassExecutionContext&, NativeMaterialPassExecutionFrame&);
// Full B44750 (948 bytes): actual pass/entry/override; native RET8. Geometry
// gates precede all binding and state work. Current renderer reloads and the
// captured plane renderer, stream, section, effect and camera remain distinct.
void bind_native_material_pass_geometry_00b44750(void* actual_pass,
    void* actual_entry, void* actual_override, NativeMaterialPassExecutionContext&,
    NativeMaterialPassExecutionFrame&);
// Complete derived virtual08 wrapper: forward the SAME entry and override0.
void execute_native_material_pass_00b454d0(void* actual_pass, void* actual_entry,
    NativeMaterialPassExecutionContext&, NativeMaterialPassExecutionFrame&);
// Read the CURRENT pass profile and +08 target. D62A80 -> B5E5E0;
// D61BE8 -> B454D0. A nonnull actual pass is required for this dispatch.
void execute_native_material_pass_current08(void* actual_pass, void* actual_entry,
    NativeMaterialPassExecutionContext&, NativeMaterialPassExecutionFrame&);

// All contexts borrow the SAME actual storage/owner/COM/global domains as the
// other raw renderer providers. Raw material+08, when nonzero, must contain its
// actual callable code address; native ECX/EDX/EAX/EBX/EBP/ESI/EDI inputs are
// restored by a register trampoline. Original image tokens are never relocated
// automatically. No new material callback, shader or owner is manufactured.
// New source ABI, C++ exception domain, no original private frame/SEH identity.
} // namespace bsp
