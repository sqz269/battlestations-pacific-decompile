#pragma once
#include "bsp/native_gui_frame_box_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include <cstdint>

namespace bsp {
// Required borrowed services, never a new owner/pool or a default terminal.
// actual_matching_pool is the SAME already-initialized canonical process pool
// that allocated this actual120h slot (F8C188, payload11C/trailer11C). Registry
// MUST be lifetime.owners, and lifetime.nodes must resolve the SAME canonical
// model/resource domain. The actual immutable D5D130 view has at least2 DWORDs.
struct NativeGuiFrameBoxIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiFrameBoxLifetimeContext& lifetime;
    NativeGuiWidgetPool<GuiFrameBoxPoolTraits>& actual_matching_pool;
    const volatile std::uint32_t* actual_profile_00d5d130;
};

enum class NativeGuiFrameBoxIdentityPhase : std::uint8_t {
    bound, destroying, retired, retired_after_failure, retired_after_copy_unwind
};
class NativeGuiFrameBoxCopyStorageOperation;

// Source orchestration ONLY, admitted separately AFTER successful complete raw
// FrameBox construction. Profile/count checks do NOT prove that completion,
// allocation/pool authority, live subobjects or the independent graph lifetime.
// Borrow the actual live atomic+04 through RenderCommandReference's borrowed
// constructor: no second live count, Native store, retain or protective credit.
// Registry insertion may allocate/throw; failed admission leaves the Native
// object caller-owned and unchanged. Any existing registry key is rejected.
//
// BEFORE admission/callbacks independently retain the complete SAME graph,
// candidate backing, actual strings/render/node/widget services and prepared
//125 context: one fresh119 with exact header=owner+F0 and distinct fresh111
// array for its constructed span. Preserve125's expected-range/callback and
//119's captured-range/current matching-SDK-free contracts. Do not admit failed
// copies/unfinished constructors or allocate/prepare metadata on zero. Known
// metadata/slot regions are checked before base/member construction; full
// backing/private111/scratch disjointness and legal C++ lifetimes remain caller
// proofs. No raw vector/name/trailer snapshot is made at admission.
//
// Companion/context/registry/pool and all prepared services stay stable/live
// until retirement finishes. External serialization prevents slot reuse,
// reentry and callback mutation/destruction of this metadata. Every explicit
// scalar deletion after admission must use THIS companion. The caller retains
// all failure obligations outside this scope; links/counts are not discovery.
class NativeGuiFrameBoxReference final : public RenderCommandReference {
public:
    NativeGuiFrameBoxReference(void* actual_frame_box,
        NativeGuiFrameBoxIdentityContext&);
    ~NativeGuiFrameBoxReference() override;
    NativeGuiFrameBoxReference(const NativeGuiFrameBoxReference&) = delete;
    NativeGuiFrameBoxReference& operator=(const NativeGuiFrameBoxReference&) = delete;
    NativeGuiFrameBoxReference(NativeGuiFrameBoxReference&&) = delete;
    NativeGuiFrameBoxReference& operator=(NativeGuiFrameBoxReference&&) = delete;

    // Already-observed zero: no decrement. Validate CURRENT slot00=BD30E0;
    // genuine BD30E0 rereads the profile, selects CURRENT04=AD29B0(flags1),
    // then invokes real125 with the SAME prepared context/matching pool.
    // No lazy preparation, fallback, count reset or read after scalar return.
    // Existing Source terminal is noexcept: ordinary validation/deletion escape
    // terminates. It does not prove Native FH3/SEH/IAT or propagation identity.
    // Registry lookup may fail BEFORE this callback with count already zero;
    // phase cannot certify that release's origin. Retain that whole graph and
    // metadata; do not retry release or use explicit deletion as recovery.
    void release_zero_references() noexcept override;

    // No +04 gate/decrement: explicit Native scalar deletion ignores the count.
    // Borrow a stable flags DWORD disjoint from owner/other provider scratch;
    //125 reads its LOWBYTE only after destructor success (caller-local is OK).
    // Once admitted, even a125 Source-preflight failure retires this binding:
    // no same-lifetime retry. Success/failure unbinds by metadata only, with no
    // raw read or cleanup/rollback. Returned pointer is identity, possibly freed.
    void* delete_scalar_00ad29b0(const volatile std::uint32_t& actual_flags_slot);

    // Diagnostics read ONLY companion metadata, including after slot return.
    NativeGuiFrameBoxIdentityPhase phase() const noexcept { return phase_; }
    bool retired() const noexcept;

private:
    friend class NativeGuiFrameBoxCopyStorageOperation;
    // Only whole184's field-ready parent may publish this prepared capability.
    // It borrows the SAME count; its original creator credit/slot/whole graph
    // must survive the active constructor. This is not permission to invoke125
    // on an unfinished copy, or a recovered completion/count test.
    NativeGuiFrameBoxReference(void*, NativeGuiFrameBoxIdentityContext&,
        NativeRenderActualOwnerRegistry::BindingAdmission&&);
    // Called only AFTER184's full applicable genuine constructor unwind returns.
    // Saved metadata only: no raw read, decrement, scalar/material/slot cleanup.
    void retire_after_copy_unwind() noexcept;
    void* const identity_;
    NativeGuiFrameBoxIdentityContext& context_;
    NativeGuiFrameBoxIdentityPhase phase_{NativeGuiFrameBoxIdentityPhase::bound};
    void retire(NativeGuiFrameBoxIdentityPhase) noexcept;
};

// Bound/destroying metadata destruction fails closed; retired metadata may end
// without dereferencing its former raw slot. These are NEW Source rules.
// Retired-after-failure does NOT discharge any independent failed graph or
// active125/119/111 obligation: those owners/frames must OUTLIVE this metadata
// as required by their contracts. Unbind never permits retry, count repair,
// graph cleanup or destruction of entered/failed119. No actual B18A40 wrapper,
// whole AD0D80, production host activation or Original binary ABI is added.
// The private184 route is the only construction-progress exception to public
// completed admission. Its new terminal phase preserves all old enum values
// and layout. Ending that retired companion, INCLUDING its borrowed base, reads
// metadata only; all partial graphs/material Box credits/actual atomic/slot and
// external child obligations still require their independently held lifetimes.
} // namespace bsp
