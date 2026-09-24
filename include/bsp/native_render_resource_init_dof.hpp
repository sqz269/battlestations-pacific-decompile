#pragma once
#include "bsp/native_render_resource_init_post.hpp"

namespace bsp {

// Original immutable byte strings; other parameter names come from the SAME
// retained continuation context. This adds no global or string-pool domain.
struct NativeRenderResourceInitDofNames {
    const char* effect_00d5e348;       // HDRFinalPass_DOF.mshd
    const char* focal_plane_00d5e338;  // cFocalPlaneData
    const char* focal_plane2_00d5e324; // cFocalPlaneData2
};

// Persistent, immovable caller storage for ONLY [B118AF,B11D7E). The actual
// post20 block supplies its canonical completed-owner companion. Never register
// a second pass/owner view or blanket-register its directly owned children.
// All predecessors, the SAME context, service and original argument cells must
// survive every dependent stage. Explicit child quiescence is still required
// before destroying/resetting this state, including failed preparation.
struct NativeRenderResourceInitDofState final {
    enum class Phase { fresh, preparing, running, awaiting_b11d7e_continuation,
        second_bloom_running, awaiting_later_continuation, failed };
    NativeRenderResourceInitDofState() = default;
    NativeRenderResourceInitDofState(const NativeRenderResourceInitDofState&) = delete;
    NativeRenderResourceInitDofState& operator=(const NativeRenderResourceInitDofState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitPostState* previous{};
    const void* second_bloom_identity{}; // One-use claim; no ownership credit.
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{};
    void* raw_post{};
    void* returned_post{};
    bool field74_published{};
    NativePostEffect20ConstructionBlock post;
    // Effect header first, then ten parameter headers in native order.
    std::array<NativeString,11> names;
    std::array<bool,11> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,10> parameter_sources{};
    std::array<void*,10> parameter_materials{};
    // Borrowed diagnostic captures can be stale after providers return.
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
};

// NORMAL-PATH FRAGMENT [B118AF,B11D7E). Consume the published B118AF stage once,
// construct/publish actual service+74, configure two textures, ten borrowed
// parameters and color0. Seventh parameter uses retained EBP=service+A0.
// B18AC0 sites forward count1 as four words/matrix0 to the existing B17E10.
// The containing B107F0 ABI is ECX service/three stack DWORDs/eventual RET0C;
// this explicit C++ API executes no native return or argument-cell reads.
// Stop BEFORE B11D7E PUSH40. On failure retain acquisitions and diagnostics;
// no caller rollback/retry/free or full-init/teardown/FH3/SEH/stack-alias claim.
void continue_native_render_resource_init_00b118af_fragment(
    NativeRenderResourceInitPostState&, NativeRenderResourceInitContinuationContext&,
    const NativeRenderResourceInitDofNames&, NativeRenderResourceInitDofState&);

} // namespace bsp
