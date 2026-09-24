#pragma once
#include "bsp/native_render_resource_init_continuation.hpp"

namespace bsp {

// Retained attempt storage for ONLY [B11599,B118AF) inside B107F0. These
// records borrow actual owners and parameter addresses; they add no resource
// credits and never free/unbind/rollback retained previous construction blocks.
// Keep this state, its predecessor, original entry/argument cells and the SAME
// continuation context alive and immovable through all later stages.
struct NativeRenderResourceInitPostState final {
    enum class Phase { fresh, running, awaiting_b118af_continuation, failed };
    NativeRenderResourceInitPostState() = default;
    NativeRenderResourceInitPostState(const NativeRenderResourceInitPostState&) = delete;
    NativeRenderResourceInitPostState& operator=(const NativeRenderResourceInitPostState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitContinuationState* previous{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::array<NativeString,7> names;
    std::array<bool,7> name_constructed{},name_return_started{},name_returned{};
    // Captured diagnostics are borrowed, possibly stale after provider calls.
    // In particular, returned name storage is not dereferenceable ownership.
    std::array<const void*,7> parameter_sources{};
    std::array<void*,7> parameter_materials{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
};

// NORMAL-PATH FRAGMENT [B11599,B118AF). Claims the published B11599 predecessor
// once; binds +70 textures0/1, seven borrowed parameters and output color0.
// Original ECX service / three stacked DWORDs eventually RET0C in B107F0;
// this new C++ interface executes no native return and does not read those args.
// The sixth registration uses the retained EBP address, not a copied value.
// Stop BEFORE the next PUSH20 allocation stage. Preserve all acquired state on
// failure; no full initialization/destruction, native FH3/SEH, private stack
// alias, arbitrary profile, app wiring or binary ABI admission.
void continue_native_render_resource_init_00b11599_fragment(
    NativeRenderResourceInitContinuationState&,
    NativeRenderResourceInitContinuationContext&,
    NativeRenderResourceInitPostState&);

} // namespace bsp
