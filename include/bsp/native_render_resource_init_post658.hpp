#pragma once
#include "bsp/native_render_resource_init_passthrough_dust.hpp"

namespace bsp {
struct NativeRenderResourceInitPost658Names {
    const char* effect_00d5e2cc;      // Passtrough_OldFilm.mshd
    const char* letterbox_00d5e2bc;   // cLetterboxRatio
    const char* noise_offset_00d5e2ac;// cNoiseOffset
    const char* noise_scale_00d5e2a0; // cNoiseScale
    const char* flicker_00d5e294;     // cFlicker
    const char* shake_00d5e28c;       // cShake
};

// Independent persistent post20 block for ONLY [B120B9,B124A1). Its completed
// raw B4E470 owner already has its canonical companion; never add another view.
// All names/context/predecessors and service parameter storage remain alive.
// Each captured current+34 owner must also outlive its borrowed parameter data,
// even if callbacks later replace +34. Diagnostics add no retain or credit.
// Existing explicit quiescence is required before child reset/destruction.
struct NativeRenderResourceInitPost658State final {
    enum class Phase { fresh, preparing, running, awaiting_b124a1_continuation,
        post65c_running, awaiting_later_continuation, failed };
    NativeRenderResourceInitPost658State() = default;
    NativeRenderResourceInitPost658State(const NativeRenderResourceInitPost658State&) = delete;
    NativeRenderResourceInitPost658State& operator=(const NativeRenderResourceInitPost658State&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitPassthroughDustState* previous{};
    const void* post65c_identity{}; // One-use successor; no ownership credit.
    const NativeRenderResourceInitPost658Names* names_identity{};
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{},ebx_bits{};
    void* raw_post{};
    void* returned_post{};
    bool field658_published{};
    NativePostEffect20ConstructionBlock post;
    std::array<NativeString,9> names; // Effect then eight parameter headers.
    std::array<bool,9> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,8> parameter_sources{};
    std::array<void*,8> parameter_materials{},captured_parameter_owners{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    void* captured_texture{};
    void* captured_texture_post{};
    void* captured_texture_material{};
    NativeFrameTargetOwnerStorage* captured_frame{};
    void* captured_frame_post{};
};

// NORMAL-PATH FRAGMENT with exact one-use B120B9 predecessor and SAME original
// continuation context/entry/argument identities. Inherited EBX=service194 and
// EBP=serviceF4 are USED at the native parameter sites, before later replacements
// with service228/service648. Capture current67C BEFORE current658 for texture3;
// build eight borrowed records with the exact current+34/receiver order; capture
// current1D4 BEFORE current658 for frame assignment. Stop BEFORE B124A1 PUSH20.
// No argument-cell read, duplicate companion, invented field/default, caller
// rollback/free/retry, full init/lifetime, FH3/SEH/private-stack alias or ABI claim.
void continue_native_render_resource_init_00b120b9_fragment(
    NativeRenderResourceInitPassthroughDustState&, NativeRenderResourceInitContinuationContext&,
    const NativeRenderResourceInitPost658Names&, NativeRenderResourceInitPost658State&);
} // namespace bsp
