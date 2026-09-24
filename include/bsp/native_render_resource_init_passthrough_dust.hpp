#pragma once
#include "bsp/native_render_resource_init_distortion.hpp"
#include "bsp/native_post_effect_dust_initializer.hpp"

namespace bsp {

// Same actual application domains, with the DISTINCT existing post24 context
// required by dust. Do not reinterpret the post20 context or create substitutes.
// Original D5E430 scene-color name is borrowed from continuation.parameter_names[0].
struct NativeRenderResourceInitPassthroughDustContext {
    NativePostEffectDustContext& dust;
    const char* effect_00d5e314;       // Passtrough.mshd (original spelling)
    const char* displacement_00d5e2f8; // cDisplacementSampleOffset
    const char* aa_offsets_00d5e2e4;   // cAASampleOffsets
};

// Persistent immovable attempt storage for ONLY [B11EF4,B120B9). The raw post20
// and dust's raw post24 blocks already supply their canonical companions; never
// register a generic duplicate. All child blocks, contexts, borrowed service
// parameter bytes and predecessor states survive dependent stages/callbacks.
// Their existing explicit external-quiescence contracts govern reset/destruction.
struct NativeRenderResourceInitPassthroughDustState final {
    enum class Phase { fresh, preparing, running, awaiting_b120b9_continuation, failed };
    NativeRenderResourceInitPassthroughDustState() = default;
    NativeRenderResourceInitPassthroughDustState(const NativeRenderResourceInitPassthroughDustState&) = delete;
    NativeRenderResourceInitPassthroughDustState& operator=(const NativeRenderResourceInitPassthroughDustState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitDistortionState* previous{};
    const NativeRenderResourceInitPassthroughDustContext* context_identity{};
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{};
    std::uint32_t ebx_bits{0xffffffffu}; // Becomes service+194 at B11FF4.
    void* raw_post{};
    void* returned_post{};
    bool field650_published{};
    NativePostEffect20ConstructionBlock post;
    // Effect header, then three parameter headers. Normally returned headers
    // deliberately stay stale; diagnostics never authorize a repeated return.
    std::array<NativeString,4> names;
    std::array<bool,4> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,3> parameter_sources{};
    std::array<void*,3> parameter_materials{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    NativeFrameTargetOwnerStorage* captured_frame{};
    void* captured_post{};
    void* captured_dust_receiver{};
    NativePostEffectDustBlock dust;
};

// NORMAL-PATH FRAGMENT: exact one-use B11EF4 predecessor and SAME original
// context/entry/argument cells. Construct/publish actual+650, register three raw
// borrowed parameters, capture current+1D4 BEFORE current+650 frame assignment,
// then load current+34 for the complete existing dust initializer. The actual
// +34 receiver must still satisfy its original CCh producer/provider contract.
// Stop BEFORE next post allocation B120B9. EBP becomes service+F4, EBX becomes
// service+194, EDI becomesFFFFFFFF, ESP14 remains raw+650. No argument cell read.
// No extra registry/count, invented initialization, caller rollback/free/retry,
// full init/service lifetime, native FH3/SEH, private-stack alias or ABI claim.
void continue_native_render_resource_init_00b11ef4_fragment(
    NativeRenderResourceInitDistortionState&, NativeRenderResourceInitContinuationContext&,
    NativeRenderResourceInitPassthroughDustContext&, NativeRenderResourceInitPassthroughDustState&);

} // namespace bsp
