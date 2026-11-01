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

// Source counted-fill and completed-prefix cleanup for AB5400. Native ECX=destination, EDX=count,
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
// Keep actual frame/home/context and containing record backing retained.
// On escape retain partial/current and remaining outstanding obligations
// and providers. Already-returned destructor calls are history, not
// disposition proof; never rescan their pointees or replay those calls.
//
// Containing-backing retention lasts until an explicitly specified genuine
// enclosing allocation owner's deallocation is reached. That physical free
// ends trivial state C++ lifetimes; it is not logical texture/name retirement.
// After free retain numeric history and genuinely independent external
// resource/provider debt. No raw-pointee rescan or destructor replay is valid.
// The caller must supply actual allocation custody and independent debt
// lifetimes; this creates no owner, discharge, Origin or NoThrow guarantee.
//
// Private Source controls/spills are disjoint from operand/record and
// provider-write extents. Declared frame/incoming operand homes remain
// live: genuine callback writes are observed at reached epochs; source
// home0 is reloaded for each copy. Distinct active invocations require
// distinct retained frame/home backing. No count/progress grants custody.
//
// On a Source C++ escape, capture CURRENT frame.begin_18 FIRST, then
// CURRENT frame.progress_14 ONCE. Destroy that captured interval in
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
void repeat_construct_native_gui_icon_states_storage_00ab5400(
    void* actual_destination, std::uint32_t count_edx,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateRepeatStorageFrame& frame,
    NativeGuiIconStorageContext& context);
} // namespace bsp
