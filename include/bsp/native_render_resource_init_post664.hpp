#pragma once
#include "bsp/native_render_resource_init_post65c.hpp"

namespace bsp {
// Original native literal views, retained alongside the original continuation's
// cSceneColorSampleOffset view. These are borrowed bytes, not new globals.
struct NativeRenderResourceInitPost664Names {
    const char* effect_00d5e240; // fakemotionblur.mshd
    const char* camera_velocity_00d5e230; // cCameraVelocity
    const char* camera_speed_00d5e220; // cCameraSpeed
    const char* min_speed_00d5e214; // cMinSpeed
    const char* max_blur_00d5e208; // cMaxBlur
    const char* divider_00d5e1fc; // cDivider
};

// Independent immovable post20 attempt for ONLY [B1297A,B12C9A). Existing
// B4E470 provides the canonical completed-owner companion. Keep this block,
// every predecessor, the SAME original context, literal views and parameter
// backing storage alive through dependent stages/callbacks. Parameter captures
// add no owner credit. Existing explicit external quiescence governs reset.
struct NativeRenderResourceInitPost664State final {
    enum class Phase { fresh, preparing, running, awaiting_b12c9a_continuation,
        tail_running, normal_work_complete, failed };
    NativeRenderResourceInitPost664State() = default;
    NativeRenderResourceInitPost664State(const NativeRenderResourceInitPost664State&) = delete;
    NativeRenderResourceInitPost664State& operator=(const NativeRenderResourceInitPost664State&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitPost65cState* previous{};
    const NativeRenderResourceInitPost664Names* names_identity{};
    const void* tail_identity{}; // One-use terminal normal-work fragment; retain all dependents.
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{},ebx_bits{};
    void* raw_post{};
    void* returned_post{};
    bool field664_published{};
    NativePostEffect20ConstructionBlock post;
    std::array<NativeString,7> names; // Effect, then six parameter headers.
    std::array<bool,7> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,6> parameter_sources{};
    std::array<void*,6> parameter_materials{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    NativeFrameTargetOwnerStorage* captured_frame{};
    void* captured_frame_post{};
};

// NORMAL-PATH FRAGMENT: consume the exact B1297A predecessor once with the SAME
// original continuation context, entry and argument identities. Construct and
// publish actual+664, register six borrowed records in their individual capture
// orders, and assign captured current frame1D4 to captured current664. Preserve
// EBX=100 before the construction branch, EBP=-1 before mask test/publication,
// EDI=raw allocation, full DWORD mask100, and final EBX=service4. No argument
// reads, texture/default insertion, duplicate companion, caller rollback/free,
// full initializer/lifetime, native FH3/SEH/private-stack alias or ABI claim.
void continue_native_render_resource_init_00b1297a_fragment(
    NativeRenderResourceInitPost65cState&, NativeRenderResourceInitContinuationContext&,
    const NativeRenderResourceInitPost664Names&, NativeRenderResourceInitPost664State&);
} // namespace bsp
