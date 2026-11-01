#pragma once
#include "bsp/native_gui_icon_native_size_storage.hpp"

namespace bsp {
// No-DMI ordinary retained homes. The first12B correspond to AB24B0's private
// difference/output-width/output-height homes. The next DWORD is the distinct
// outgoing UV-pointer HOME, overwritten by the genuine AB17B0 body; its own
// two private DWORDs are separate nested backing. This is Source object layout,
// not an Original stack frame, owner, Native lifetime or success receipt.
struct NativeGuiIconSizeComparisonScratch {
    std::uint32_t difference_home;
    std::uint32_t native_width_home;
    std::uint32_t native_height_home;
    volatile std::uint32_t uv_pointer_home;
    NativeGuiIconNativeSizeScratch extent_scratch;
};
static_assert(sizeof(NativeGuiIconSizeComparisonScratch) == 24);

// Whole AB24B0..AB2530 normal schedule:129B/40ops, ECXactual40Bstate, no incoming
// stack argument, full EAX0or1, plain RET on BOTH paths (not stale RET4 wording).
// This six-borrow ordinary Source API changes that ABI. It captures the state
// and CURRENT texture00, stages actual state+0C into the real outgoing HOME,
// and calls the complete raw AB17B0 API with actual private output8B and SAME
// callbacks/constants. No semantic state/UV/size or injected dimensions/bool.
//
// Only AFTER the helper returns does MOVSS read CURRENT minimumD5C3B8; COMISS
// and JA retain their ordered/unordered behavior. CURRENT state34 minus output
// width is rounded BEFORE loading double epsilonD7A268 once. Epsilon stays live
// through first FCOMIP's false/unordered arm and CURRENT state38/height/temp/
// sign-mask operations. Second FCOMIP is followed by FSTP BEFORE JA. Widthtrue
// pops epsilon separately; minimumtrue never pushes or pops epsilon. No C++ FP
// comparison/arithmetic, callback or Source metadata service runs inside that
// live interval. Ambient x87/MXCSR control/status are not reset or restored.
//
// Minimum/epsilon are the SAME authenticated live original-VA readonly cells,
// with the existing permanent owner lifetime; no constexpr copy, bank, default
// or setter is provided. All AB17B0 captured-target/canonical+04/current-table/
// raw backing/provider qualifications remain in force. Actual producer/caller
// must supply these borrows; existing semantic Game bindings are not raw wiring.
//
// Caller retains genuine state/subobjects/texture/registry/providers and every
// reached home through callbacks/escape, with separate stable Scratch per active
// invocation. Scratch is disjoint from actual payloads and OTHER provider write
// extents; the helper's declared output/HOME/extent_scratch writes are intentional
// distinct subobjects. Own Source argument/control/carrier storage is separate.
// No partial cleanup, replay/reset, freshness guard, noexcept, Native exception-
// origin or disposition policy is added. Original private-stack/alias/register/
// FLAGS/fault/SEH/FH3/unmaskedFP, compiler/Core and production activation remain
// unproved. Normal0/1 is only this Source result, not Native cleanup authority.
std::uint32_t gui_icon_state_differs_from_native_size_storage_00ab24b0(
    void* actual_40b_state, NativeGuiIconSizeComparisonScratch& scratch,
    const GuiTextureCallbacks& callbacks,
    const GuiIconStateTextureConstants& extent_constants,
    const volatile float& actual_minimum_width_00d5c3b8,
    const volatile double& actual_epsilon_00d7a268);
} // namespace bsp
