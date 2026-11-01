#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStorageContext;

// Ordinary caller-retained backing for reached DWORD homes, without default
// member initializers. Default-initialize without ()/{}; no home is preseeded.
// These are genuine separate Source objects, not an EBP/savedESP/FH3 replica.
struct NativeGuiIconStateRangeConstructionStorageFrame {
    std::uint32_t end_14;
    std::uint32_t begin_18;
    std::uint32_t slot_1c;
    std::uint32_t placement_20;
    std::uint32_t state_04;
};
static_assert(sizeof(NativeGuiIconStateRangeConstructionStorageFrame) == 20);

// Source range construction and completed-prefix cleanup for AB4C50. Native ECX=source begin, EDX=source
// end, four stacked DWORD homes and RET10h become this ordinary Source API.
// Capture destination homes[0] first; save incoming end to end_14, capture source
// begin, save destination to begin_18, then full state0. Reload CURRENT end_14
// at EVERY loop head, including after callbacks. Destination is a separate
// captured DWORD register; never reload it from a callback-mutated homes[0].
//
// Each iteration saves both destination homes, stores only state LOWBYTE1,
// and skips birth/copy only for the actual null destination. A reached nonnull
// slot begins ONE genuine NativeGuiIconStateStorage lifetime by no-write
// default placement construction, then directly calls existing raw AB3A70.
// Source is the captured register and advances40h AFTER each actual homes[0]
// output write. Destination advances40h, state LOWBYTE0, homes[0]=destination,
// then source advances. All address arithmetic wraps as DWORD. Return is the
// final captured destination bits, not a fresh HOME read or ownership receipt.
// Other three incoming homes retain their actual unused preimages.
//
// Inherit the existing AB3A70 domain: distinct valid actual40B records, ordinary
// backing/subobjects, real raw string pool and SAME canonical live texture+04
// graph/context. Its bool-preserve resize/std::memcpy Source contract is NOT
// promoted to exact BF0/overlap/mutable-preserve-home/Original behavior. No new
// validator, cap, callback, owner, typed T[n] family or Native retain is added.
// Caller proves every reached individual slot/backing lifetime and retains
// containing backing/homes plus outstanding partial/remaining obligations
// and providers on escape. Already-returned destructor calls are history,
// not disposition proof; never rescan their pointees or replay those calls.
//
// Private Source parameter/control/spill storage is disjoint from borrowed
// operand/record and provider-write extents. Declared operand homes in frame
// and actual_stack_homes are separate from those private controls: callbacks
// may write them, including live end_14, and reached accesses stay current.
// Other provider writes/aliases retain genuine callee/storage contracts.
// Distinct active calls need distinct retained backing. Pure comparison /
// private register/stack/FLAGS/fault timing is not Original ABI proof.
//
// On a Source C++ escape, capture CURRENT frame.begin_18 FIRST, then
// CURRENT actual_stack_homes[0] ONCE. Destroy that captured interval in
// forward DWORD +40h order through real AB3280. Advance only after
// each whole destructor returns; bare-rethrow the original only after
// the full prefix returns. A destructor escape stops the loop and
// propagates; no retry, replacement cleanup or new terminal policy.
//
// The caller proves the actual captured interval consists of genuine
// born, completed owning states with valid current canonical texture
// and name/provider custody, including any callback-written endpoints.
// Passive progress/state/normal return alone grants no ownership. The
// entered partial copy is excluded solely by the published endpoint;
// do not fabricate a current-slot destructor or private completed count.
// Retain its backing/name/provider debt and all remaining graph on
// escape. No array family, automatic disposal or Native Origin follows.
//
// This ordinary catch implements the prefix schedule of the 13 Native
// landing rows, not FS/private FH3 or binary exception ABI. Placement
// action slot reads/401130 RET remain evidence, not a new partial dtor.
// Existing AB3280 name-unwind/secondary-terminate Source qualifications
// remain unchanged; they are not generalized Native runtime proof.
// Native register/FLAGS/fault/SEH/hardware-FP and caller/vector/actual
// production/runtime activation remain unproved. No new body credit.
std::uint32_t construct_native_gui_icon_state_range_storage_00ab4c50(
    const void* actual_source_begin_ecx, const void* actual_source_end_edx,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateRangeConstructionStorageFrame& frame,
    NativeGuiIconStorageContext& context);
} // namespace bsp
