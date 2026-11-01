#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
// Whole AB5650..AB568A,59B/28ops, one actual AB3C10 call and no local EH.
// Three incoming address-DWORD homes become a genuine caller-retained array:
// [0]=source begin, [1]=source end, [2]=destination end. Capture [2], then [1],
// then [0] ONCE before any assignment. Later callback writes do not retarget
// these captures. Keep the real array/backing/providers through entered escape.
//
// Compute the returned address DWORD before copying: wrapped(end-begin), actual
// SAR6 then SHL6, and wrapped destinationEnd-minus-that-result. Empty still
// computes this value but touches neither assignment HOME nor any provider.
// Nonempty captures destinationEnd-sourceEnd once; predecrement source by40h,
// assign to currentSource+fixedDelta, compare captured begin and repeat. There
// is no ordering/count/alignment/null guard, cap, snapshot or bulk-copy shortcut.
// Legal overlap follows that actual backward per-record schedule and AB3C10's
// reached alias/load/store contract, not an inferred memmove/T[n] guarantee.
//
// The callee is the published genuine raw AB3C10, never GuiIconState/std::string
// or copy-construction. Borrow the SAME ordinary live assignment preserve DWORD,
// raw pool and canonical0109EEA4 feature reference for every reached call. This
// wrapper never reads/seeds that HOME; AB3C10 stages full1 only on its nonself
// name path. The HOME may be reused after each normal call; distinct active
// nested invocations require distinct HOME/control backing. No new Frame/type
// birth, reset/freshness capability, texture ref traffic or custody is supplied.
//
// Caller proves real already-live40B records/8B name subobjects/string/texture
// backing and all inherited pool/feature/DF0/SSE2/module/extent lifetimes. Actual
// incoming homes, ordinary preserve HOME and Source controls/spills are disjoint
// from record/payload/provider-write and active argument/return/provider extents.
// UNKNOWN escapes propagate without cleanup/refund/retry/rollback or replay;
// retain current/partial/completed/remaining graph, HOME and incomplete callee
// header/pool/allocation debt. No recoverable-pointer/progress/ownership or
// Native EH/Origin/cleanup/disposition authority is minted. Return is numeric
// address data, not an owner, birth or normal-retirement receipt.
//
// Ordinary four-argument Source ABI is not Native three-stack-argument/plainRET
// EAX/register/private-stack/FLAGS/fault/FH3/SEH/unmaskedFP identity. Existing high
// STL name/tag and all prior credit stay unchanged; add zero Original/vendor/
// helper/CG/duplicate-body credit. Actual AB59E0/vector/caller/activation remain
// separate. No noexcept, new terminal policy or default provider exists.
std::uint32_t assign_native_gui_icon_state_range_backward_storage_00ab5650(
    const volatile std::uint32_t (&actual_argument_homes)[3],
    std::uint32_t& actual_assignment_preserve_home, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
