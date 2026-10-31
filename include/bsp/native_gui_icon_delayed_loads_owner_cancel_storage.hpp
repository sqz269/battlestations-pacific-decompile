#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon delayed owner cancellation requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Ordinary caller-retained homes, no initializers/owner/phase/permission.
// Caller stages the full owner argument at its Native PUSH epoch. Saved index
// and preserve HOME remain separate from the independently captured registers.
struct NativeGuiIconDelayedLoadsOwnerCancelStorageFrame {
    std::uint32_t owner_argument_home;
    std::uint32_t saved_index_home;
    std::uint32_t preserve_home;
};
static_assert(sizeof(NativeGuiIconDelayedLoadsOwnerCancelStorageFrame) == 12);

// Whole normal AB50E0: actual16B owner, only CURRENT DATA+4/count+8 consulted.
// Initial count read precedes saved-index0 even for empty/negative return.
// Each iteration reads DATA then CURRENT owner HOME then the selected+8 word.
// MatchedFFFFFFFF takes its real no-removal common-compare path. Assignment
// captures one CURRENT data base for source/destination, never calls54/zeros/
// constructs. Self skips only string calls; late source+8 is still copied.
// Exact41DD40 uses actual preserve HOME1 and SAME canonical feature reference;
// CURRENT sourceLEN gates real BF, including CURRENT destination count0.
// Fresh tail uses CURRENT count THEN data. Raw41DD20 returns its header buffer
// without clearing fields; decrement CURRENT count only after it returns,
// then reload saved index WITHOUT recomputing the independent displacement.
//
// Caller proves genuine containing owner, individually live12B entries/raw
// descriptor/buffers/subobjects/preimages, actual publication/provenance and
// retained custody across callbacks/incomplete attempts. No T[n] family or
// typed pointer arithmetic is inferred. This full Frame/HOMEs and active Source
// arguments/providers must be disjoint from all Native write extents. Reentry
// needs distinct retained homes; no early owner/value/feature snapshot.
// Borrow SAME actual raw pool/new-handler/deletion graph and authenticated
// live0109EEA4 cell. Inherit exact resize/BF extents/DF0/reached CPU and lifetime
// domains; Original private aliases/addresses/fault order/EFLAGS/ABI/FH3/SEH
// and runtime identity remain unproved. No vtable/cap/profile/overflow guard,
// fake owner, default, RAII, noexcept, cleanup, compensation, retry or issuer.
// Existing bool/typed/byvalue-owner service and scalar flag route stay separate.
void cancel_native_gui_icon_delayed_loads_owner_storage_00ab50e0(
    void* actual_delayed_owner16, NativeGuiIconDelayedLoadsOwnerCancelStorageFrame&,
    NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
