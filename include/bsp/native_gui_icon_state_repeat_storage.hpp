#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiIconStorageContext;

// Caller-retained ordinary backing for the five reached Native DWORD homes.
// Default-initialize this storage without ()/{}; no home is seeded here. These
// are separate Source objects, not a physical EBP/private-FH3 stack replica.
struct NativeGuiIconStateRepeatStorageFrame {
    std::uint32_t progress_14;
    std::uint32_t begin_18;
    std::uint32_t slot_1c;
    std::uint32_t placement_20;
    std::uint32_t state_04;
};
static_assert(sizeof(NativeGuiIconStateRepeatStorageFrame) == 20);

// NORMAL counted-fill support for AB5400. Native ECX=destination, EDX=count,
// four actual incoming DWORD homes and RET10h become this ordinary Source API.
// The count is the full unsigned DWORD captured once; the destination advances
// with DWORD wrap. Only CURRENT homes[0] is read, once per reached nonnull copy;
// homes[1..3] retain their genuine preimages and are not interpreted or seeded.
// No home read or object birth occurs on a null destination iteration.
//
// Each reached nonnull slot must be real aligned 40h storage in which an
// individual NativeGuiIconStateStorage may begin its lifetime without field
// initialization. That does not require or establish a T[n] array lifetime.
// The existing AB3A70 copy is called directly with the SAME context. Its actual
// qualified domain is inherited: distinct valid native records, real raw string
// pool/backing and canonical live texture+04 objects in the SAME registry.
// No promotion to exact BF0/mutable-preserve-home/overlap/Original behavior is
// made; this entry adds no validation, domain filter, owner or provider.
//
// Retain the actual frame, four-home array, context, current/partial/completed
// states, all payload/storage/custody and their genuine subobjects across every
// entered escape. Frame/Source parameter/control/spill backing is disjoint from
// borrowed records and all reached provider-write extents; the live source home
// may be changed by genuine callbacks and is reread on the next copy. Distinct
// reentrant invocations require distinct retained frame/home backing. No count
// limit, progress-based birth/ownership receipt or automatic disposition exists.
//
// The 13 Native catch-landing rows AB545B..AB5477 are HELD, not reconstructed.
// Arbitrary Source escape remains UNKNOWN and propagates without prefix cleanup,
// rollback/retry, Native-origin permission, noexcept or a new terminal policy.
// State/progress are passive operands, never EH/cleanup/retirement authority.
// Native private stack/register/FLAGS/fault/SEH/FH3/hardware-FP equivalence and
// vector/growth/caller/production/activation are unproved. No new body credit.
void repeat_construct_native_gui_icon_states_storage_00ab5400(
    void* actual_destination, std::uint32_t count_edx,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateRepeatStorageFrame& frame,
    NativeGuiIconStorageContext& context);
} // namespace bsp
