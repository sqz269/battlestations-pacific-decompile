#pragma once
#include "bsp/native_render_resource_init_dof.hpp"

namespace bsp {

// Persistent immovable attempt storage for ONLY [B11D7E,B11E40). The independent
// bloom block must not be reused from service+28. All actual owners, same context,
// original argument cells and predecessor blocks remain alive through later
// stages. Explicit existing child quiescence is required before reset/destruction.
struct NativeRenderResourceInitSecondBloomState final {
    enum class Phase { fresh, preparing, running, awaiting_b11e40_continuation,
        distortion_running, awaiting_later_continuation, failed };
    NativeRenderResourceInitSecondBloomState() = default;
    NativeRenderResourceInitSecondBloomState(const NativeRenderResourceInitSecondBloomState&) = delete;
    NativeRenderResourceInitSecondBloomState& operator=(const NativeRenderResourceInitSecondBloomState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitDofState* previous{};
    const void* distortion_identity{}; // One-use successor; no native ownership.
    std::uint32_t native_site{};
    int native_state{-1};
    void* raw_frame{};
    NativeFrameTargetOwnerStorage* returned_frame{};
    void* raw_bloom{};
    void* returned_bloom{};
    bool frame_published{},bloom_published{};
    // Borrowed diagnostic captures; actual current publications remain authoritative.
    void* captured_color_holder{};
    NativeSurfaceOwnerStorage* captured_color{};
    void* captured_renderer{};
    void* captured_depth{};
    void* captured_input_pass{};
    NativeBloomInitializationArguments arguments{};
    NativeBloomInitializationBlock bloom;
};

// NORMAL-PATH FRAGMENT [B11D7E,B11E40). Claims the exact DOF frontier once,
// constructs/publishes actual frame+1CC and second bloom+2C, preserving current
// publication/retain/load order and source FLD1/FSTP. At the new frontier EBP
// means half aligned width, EDI half aligned height, ESP14 raw second bloom.
// Original ECX service/three DWORD arguments/eventual RET0C remain represented
// by the SAME entry/context chain; this fragment reads no argument cell and
// executes no native return. Stop BEFORE distortion allocation at B11E40.
// No extra registry/count, caller rollback/free, full teardown, native FH3/SEH,
// machine-stack alias, arbitrary profile or gameplay admission.
void continue_native_render_resource_init_00b11d7e_fragment(
    NativeRenderResourceInitDofState&, NativeRenderResourceInitContinuationContext&,
    NativeRenderResourceInitSecondBloomState&);

} // namespace bsp
