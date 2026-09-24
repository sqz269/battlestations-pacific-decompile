#pragma once

#include "bsp/native_frame_target_owner.hpp"
#include "bsp/native_render_service_texture_lifetime.hpp"

namespace bsp {

// Original B107F0 stack words, in increasing address order. Callers disagree
// about words zero/two; retain all three without assigning guessed meanings.
// This storage models the native argument cells and must outlive continuation.
struct NativeRenderResourceInitArguments {
    std::uint32_t word_00;
    std::uint32_t word_04;
    std::uint32_t word_08;
};
static_assert(sizeof(NativeRenderResourceInitArguments) == 12);

struct NativeRenderResourceInitEntryContext {
    NativeFrameTargetOwnerContext& frame_targets;
    // Renderer publication is borrowed through frame_targets' surface context.
    // These are original current profiles/import cells, not host vtables.
    const volatile std::uint32_t* actual_frame_profile_00d5e600;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    NativeRenderServiceTextureDecrement const volatile& decrement_iat_00ce2220;
};

// Caller-owned continuation record, not an owner, success token or machine
// stack. It does not free the newly published frame. Never reset/replay a used
// record; keep the service, argument cells and contexts alive across continuation.
struct NativeRenderResourceInitEntryState final {
    enum class Phase { fresh, running, already_initialized_returned,
        awaiting_b109bc_continuation, failed };
    NativeRenderResourceInitEntryState() = default;
    NativeRenderResourceInitEntryState(const NativeRenderResourceInitEntryState&) = delete;
    NativeRenderResourceInitEntryState& operator=(const NativeRenderResourceInitEntryState&) = delete;
    Phase phase{Phase::fresh};
    void* service{};
    const volatile NativeRenderResourceInitArguments* argument_cells{};
    std::uint32_t native_site{};
    int unwind_state{-1};
    // EDI and ESP+20 identities at the frontier. The old frame can be freed;
    // never dereference it merely because its address is retained here.
    void* captured_old_frame{};
    void* frame_allocation{};
    std::uint32_t dimensions_esp18[2]{};
    std::uint32_t aligned_height_esp24{};
    std::uint32_t aligned_width_eax{};
    std::uint32_t height_remainder_ecx{};
    std::uint32_t width_remainder_edx{};
    std::uint32_t scalar_low_xmm[4]{};
};

// Complete 24-byte native B21F10 body: ECX renderer, stack output, EAX output,
// RET4. New C++ interface; both DWORD inputs are captured before either store,
// including when output overlaps the presentation block at renderer+1A28.
void* copy_native_renderer_dimensions_00b21f10(const void* actual_renderer,
    void* actual_output) noexcept;

// B107F0 ENTRY FRAGMENT: [B107F0,B109BC), plus the taken existing-init tail
// [B13009,B13029). Native ECX service, three stack words, RET0C. Fresh entry
// stops BEFORE B109BC, retaining dimensions and registers above; it never
// performs the full function return, initializes +70, or admits destruction.
// Existing-init entry completes its original dirty/invalidate branch. No
// native FH3/SEH, stack aliasing, arbitrary profile or binary ABI claim.
void begin_native_render_resource_init_00b107f0_fragment(void* actual_service,
    const volatile NativeRenderResourceInitArguments&,
    NativeRenderResourceInitEntryContext&, NativeRenderResourceInitEntryState&);

} // namespace bsp
