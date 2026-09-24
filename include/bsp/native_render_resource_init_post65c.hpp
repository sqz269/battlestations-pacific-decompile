#pragma once
#include "bsp/native_render_resource_init_post658.hpp"

namespace bsp {
// The other eight literal views remain borrowed from the original continuation
// and retained predecessor contexts. These are original bytes, not new globals.
struct NativeRenderResourceInitPost65cNames {
    const char* effect_00d5e26c; // Passtrough_OldFilm_Wave.mshd
    const char* horizontal_00d5e260; // cHParams
    const char* vertical_00d5e254; // cVParams
};

// Independent immovable post20 attempt for ONLY [B124A1,B1297A). Existing
// B4E470 construction provides the actual canonical completed-owner companion;
// no generic duplicate. Keep all providers, names, predecessor identities and
// parameter backing storage alive through dependent stages/callbacks. Each
// captured current+34 owner must outlive its own borrowed parameter record,
// even if later callbacks replace the parent field. Captures add no credit.
// Existing explicit external quiescence governs child reset/destruction.
struct NativeRenderResourceInitPost65cState final {
    enum class Phase { fresh, preparing, running, awaiting_b1297a_continuation, failed };
    NativeRenderResourceInitPost65cState() = default;
    NativeRenderResourceInitPost65cState(const NativeRenderResourceInitPost65cState&) = delete;
    NativeRenderResourceInitPost65cState& operator=(const NativeRenderResourceInitPost65cState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitPost658State* previous{};
    const NativeRenderResourceInitPost65cNames* names_identity{};
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{},ebx_bits{};
    void* raw_post{};
    void* returned_post{};
    bool field65c_published{};
    NativePostEffect20ConstructionBlock post;
    std::array<NativeString,11> names; // Effect, then ten parameter headers.
    std::array<bool,11> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,10> parameter_sources{};
    std::array<void*,10> parameter_materials{},captured_parameter_owners{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    void* captured_texture{};
    void* captured_texture_post{};
    void* captured_texture_material{};
    NativeFrameTargetOwnerStorage* captured_frame{};
    void* captured_frame_post{};
};

// NORMAL-PATH FRAGMENT: consume the exact B124A1 predecessor once with the SAME
// original continuation context, entry and argument identities. Use inherited
// EBX=service228 and EBP=service648 at their real sites; they stay unchanged.
// Construct/publish actual+65C, bind current67C texture3, register ten borrowed
// records with individual receiver/source order, then assign captured current
// frame1D4 to captured current65C. Stop BEFORE B1297A PUSH20. No argument read.
// No duplicate companion, extra count/primitive/default, caller rollback/free,
// full initializer/lifetime, native FH3/SEH/private-stack alias or ABI claim.
void continue_native_render_resource_init_00b124a1_fragment(
    NativeRenderResourceInitPost658State&, NativeRenderResourceInitContinuationContext&,
    const NativeRenderResourceInitPost65cNames&, NativeRenderResourceInitPost65cState&);
} // namespace bsp
