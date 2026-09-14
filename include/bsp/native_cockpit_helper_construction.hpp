#pragma once
#include "bsp/native_cockpit_construction_block.hpp"

namespace bsp {

using NativeCockpitInterlockedDecrement = long (__stdcall*)(volatile long*);

// Borrow the actual callable CE2220 IAT cell and the actual two-word D5E5F8
// table. Read their CURRENT values at the final local-viewport release, not at
// admission. The table is a source address binding, not a replacement profile.
// Both bindings stay fixed and live through the attempt; callable/table contents
// are current volatile reads. The captured native viewport must survive its call.
struct NativeCockpitViewportReleaseContext {
    NativeCockpitInterlockedDecrement const volatile& decrement_iat_00ce2220;
    const volatile std::uint32_t* const actual_two_word_table_00d5e5f8;
};

// Complete B3C800..B3C952 normal sequence in the admitted, nonnull concrete
// camera/viewport domain, plus its five-state C++ exception projection. Original
// ECX=actual 24h helper, no stacked args, EAX=same, RET. This explicit host API
// is not original ABI/FH3/SEH equivalence. See NATIVE_COCKPIT_HELPER_CONSTRUCTION.md.
//
// Caller owns aligned unused actual helper storage and persistent block/provider
// lifetimes. Preserve +10/+14/+1C/+20 preimages. Validate before consuming the
// capability or writing helper fields; move the capability before callbacks.
// Read near/far cells with x87 FLD32/FSTP32 at their native sites. The caller
// separately binds helper companions only after success. Failure does not free
// the helper allocation or erase surviving camera/viewport ownership. A settled
// block requires explicit native retirement and host-quiescent reset.
void* construct_native_cockpit_helper_00b3c800(void* actual_helper,
    std::size_t helper_bytes, const volatile std::uint32_t& near_00d7a2f0,
    const volatile std::uint32_t& far_00ce38b8,
    const NativeCockpitViewportReleaseContext&,
    NativeCockpitConstructionBlock::Admission&&);

} // namespace bsp
