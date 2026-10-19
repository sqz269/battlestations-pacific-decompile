#pragma once

#include "bsp/native_render_resource_init_tail.hpp"

namespace bsp {

// Borrow the original providers and byte strings. Every stage uses the SAME
// continuation context, service and three argument cells; no shadow domain is
// created. All referents survive the operation and native child retirement.
struct NativeRenderResourceInitializerContext {
    NativeRenderResourceInitContinuationContext& continuation;
    NativeRenderResourceInitDistortionContext& distortion;
    NativeRenderResourceInitPassthroughDustContext& passthrough_dust;
    const NativeRenderResourceInitDofNames& dof_names;
    const NativeRenderResourceInitPost658Names& post658_names;
    const NativeRenderResourceInitPost65cNames& post65c_names;
    const NativeRenderResourceInitPost664Names& post664_names;
    const NativeRenderResourceInitTailNames& tail_names;
};

// Persistent, immovable operation storage. These are the existing acquisition
// frames, not additional native resource references. Keep the entire operation
// alive through dependent work and genuine native child retirement. A failure
// is retained without caller rollback, reset, replay or fabricated EH cleanup.
class NativeRenderResourceInitialization final {
public:
    enum class Phase { fresh, running, already_initialized_returned,
        normal_work_complete, failed };
    NativeRenderResourceInitialization() = default;
    NativeRenderResourceInitialization(const NativeRenderResourceInitialization&) = delete;
    NativeRenderResourceInitialization& operator=(const NativeRenderResourceInitialization&) = delete;
    Phase phase() const noexcept { return phase_; }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::running || phase_ == Phase::failed;
    }
    const NativeRenderResourceInitEntryState& entry() const noexcept { return entry_; }
    const NativeRenderResourceInitTailState& tail() const noexcept { return tail_; }
    // Entry address of the last entered stage, not the precise failed CALL.
    // Individual retained frames carry their own current native-site evidence.
    std::uint32_t active_stage() const noexcept { return active_stage_; }
    // Borrow the SAME scene companion publication that the retained distortion
    // producer fills. Its lifetime context must be bound before initialization;
    // this view neither constructs a scene nor supplies a default cleanup field.
    NativeGuiSceneOwner* const volatile& distortion_scene_owner_3c() const noexcept {
        return distortion_.distortion.scene_owner_3c();
    }
    // Metadata closure only, after genuine native service/child retirement and
    // all external parameter/viewport/scene borrows have ended. Each existing
    // child block enforces its own terminal facts. No release, free, retry or
    // reset of this one-shot operation is performed. Failed attempts are retained.
    void close_children_after_host_quiescence() noexcept;

private:
    friend void initialize_native_render_resources_00b107f0(void*,
        const volatile NativeRenderResourceInitArguments&,
        NativeRenderResourceInitializerContext&, NativeRenderResourceInitialization&);
    Phase phase_{Phase::fresh};
    std::uint32_t active_stage_{};
    bool children_closed_{};
    NativeRenderResourceInitEntryState entry_;
    NativeRenderResourceInitContinuationState continuation_;
    NativeRenderResourceInitPostState post_;
    NativeRenderResourceInitDofState dof_;
    NativeRenderResourceInitSecondBloomState second_bloom_;
    NativeRenderResourceInitDistortionState distortion_;
    NativeRenderResourceInitPassthroughDustState passthrough_dust_;
    NativeRenderResourceInitPost658State post658_;
    NativeRenderResourceInitPost65cState post65c_;
    NativeRenderResourceInitPost664State post664_;
    NativeRenderResourceInitTailState tail_;
};

// Compose the established normal fragments [B107F0,B13010), or return via the
// already-initialized entry branch. Normal completion performs the original
// resource work, including +70's producer. It does not prove that partial-child
// destructor profiles, native FH3/SEH, saved registers, private-stack aliasing
// or RET0C are reproduced. This is a new C++ interface, not a binary replacement
// or permission to destroy a service with unresolved child lifetime contracts.
void initialize_native_render_resources_00b107f0(void* actual_service,
    const volatile NativeRenderResourceInitArguments&,
    NativeRenderResourceInitializerContext&, NativeRenderResourceInitialization&);

} // namespace bsp
