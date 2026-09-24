#pragma once
#include "bsp/native_render_resource_init_second_bloom.hpp"
#include "bsp/native_distortion_initializer.hpp"
#include "bsp/native_distortion_lifetime.hpp"
#include "bsp/native_render_pass_companion.hpp"
#include <optional>

namespace bsp {

struct NativeRenderResourceInitDistortionContext {
    NativeDistortionInitializationContext& distortion;
    const NativeDistortionOwnerConstants& constants;
    NativeRenderPassCompanionContext& pass_companions;
    NativeRenderResourcesDepthContext& depth;
};

// Persistent, immovable attempt storage for ONLY [B11E40,B11EF4). The actual
// distortion block's scene publication must be the SAME reference used by
// pass_companions.distortion. Keep that lifetime context, this context/state,
// canonical companion and every predecessor alive through dependent stages and
// callbacks. Existing explicit quiescence is required before child reset or
// destruction. Neither a diagnostic pointer nor an engaged optional is a credit.
struct NativeRenderResourceInitDistortionState final {
    enum class Phase { fresh, preparing, running, awaiting_b11ef4_continuation, failed };
    NativeRenderResourceInitDistortionState() = default;
    NativeRenderResourceInitDistortionState(const NativeRenderResourceInitDistortionState&) = delete;
    NativeRenderResourceInitDistortionState& operator=(const NativeRenderResourceInitDistortionState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitSecondBloomState* previous{};
    const NativeRenderResourceInitDistortionContext* context_identity{};
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    void* raw_distortion{};
    void* returned_distortion{};
    bool distortion_published{},initializer_returned{},initializer_result{};
    // Captured false-path owner may differ from returned_distortion and may be
    // retired by terminal dispatch. Never dereference these diagnostic bits.
    void* captured_false_owner{};
    NativeRenderServiceTextureDecrement captured_decrement{};
    long decrement_result{};
    bool decrement_started{},decrement_returned{},terminal_started{},parent_cleared{};
    NativeDistortionInitializationBlock distortion;
    std::optional<NativeRenderPassReference> pass_reference;
    void* raw_frame{};
    NativeFrameTargetOwnerStorage* returned_frame{};
    bool frame_published{};
    std::uint32_t original_third_argument{},original_width{},original_height{};
    NativeRendererSurfaceFactoryAcquired depth;
};

// NORMAL-PATH FRAGMENT. Requires the exact one-use B11E40 frontier and SAME
// original continuation context/entry/three argument cells. Constructor result
// is captured for B4F560 after publication; false result releases captured
// CURRENT service+30 through its existing canonical registry entry only at zero.
// Then construct/publish depth frame+1C8 and compose full B0FC10. Stop B11EF4.
//
// ADMISSION: whenever pre-init/capability-failure/reentrant release is possible,
// the actual allocated26Ch +34/+38/+3C preimage ALREADY obeys the real distortion
// cleanup contract, including matching live scene publication for nonnull+3C.
// Neither B4F0C0 nor the failed capability prefix initializes these fields.
// Binding metadata before publication does NOT establish their validity. This
// fragment adds no zero stores, allocator substitution, host fallback or proof
// of arbitrary heap safety. Callbacks must preserve all surviving owner domains.
// No null-init fallback, blanket registry, extra count, automatic rollback/free,
// full init/teardown, native FH3/SEH, private-stack alias or native ABI claim.
void continue_native_render_resource_init_00b11e40_fragment(
    NativeRenderResourceInitSecondBloomState&, NativeRenderResourceInitContinuationContext&,
    NativeRenderResourceInitDistortionContext&, NativeRenderResourceInitDistortionState&);

} // namespace bsp
