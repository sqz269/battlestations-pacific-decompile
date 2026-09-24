#pragma once
#include "bsp/native_group_pool.hpp"
#include "bsp/native_node_base_destruction.hpp"

namespace bsp {
struct NativeGroupStorageLifetimeFrame { NativeNodeBaseDestructionFrame node; };
struct NativeGroupStorageLifetimeAcquired {
    bool descriptor_prepared{}; // separate host lifetime preparation, no credit
    bool started{}, complete{}, exception_cleanup_started{}, array_cleanup_completed{};
    bool pool_returned{};
    std::int32_t native_eh_state{-1};
    std::uint32_t active_call_site{};
    NativeNodeBaseDestructionAcquired node;
};
struct NativeGroupStorageReleaseAcquired {
    bool started{}, complete{};
    std::uint32_t cleared_entries{}, active_call_site{};
    NativeNodeTreeRetirementAcquired node;
};

// Complete B8F680[102], ECX actual18Ch group, RET. StampD634F8; state0
// resize0/current begin/free; state-1 before rawB6F440. Direct destruction
// deliberately leaves the borrowed entries' A0 untouched. Native begin and
// capacity stay stale. Reuses the genuine pointer-only B7C1C0 cleanup via
// pinned resize80/reserve95/cleanup23 normalized equivalence; Group native
// code inlines this schedule and does NOT call that helper entry.
void destroy_native_group_storage_00b8f680(void* actual_group,
    NativeGroupStorageLifetimeFrame&, NativeNodeBaseDestructionContext&,
    NativeGroupStorageLifetimeAcquired&);
// B8F8C0[32], ECX actual18Ch, stacked flags; EAX original identity, RET4.
// No extra entry stamp/decrement. Read CURRENT flags lowbyte AFTER destructor;
// bit0 returns SAME slot via actual010902F4/B8ED40/current188. Direct scalar
// permits positive count and flags0; genuine-pool provenance is caller-owned.
// No raw payload/count access after return to the pool.
void* delete_native_group_storage_00b8f8c0(void* actual_group,
    const volatile std::uint32_t& flags, NativeGroupPool&,
    NativeGroupStorageLifetimeFrame&, NativeNodeBaseDestructionContext&,
    NativeGroupStorageLifetimeAcquired&);
// B8EEC0[69], ECX actual group/no args, tail B6F310. Test current count;
// capture current count/begin/back before conditional decrement; clear captured
// back nodeA0, reread current count. This loop precedes the shared late byte44
// gate, including repeat logical release. Tail can retire/free actual owner:
// only persistent diagnostics are touched afterward. Fresh release acquisition
// and initialized tree frame are distinct from destruction frame/acquisition.
void release_native_group_storage_00b8eec0(void* actual_group,
    NativeNodeTreeRetirementFrame&, NativeNodeTreeRetirementContext&,
    NativeGroupStorageReleaseAcquired&);
// B8E6B0[8], ECX actual group, RET. Only AND138,FFFFFFCF. No enclosing A0
// callback/child traversal/logical SceneAttachmentRuntime projection.
void __fastcall invalidate_native_group_world_00b8e6b0(void* actual_group) noexcept;

struct NativeGroupStorageIdentityContext {
    NativeNodeBaseDestructionContext& lifetime;
    NativeRenderActualOwnerRegistry& owners;
    NativeGroupPool& pool_010902f4;
    const volatile std::uint32_t* actual_profile_00d634f8;
};
// Reuse the existing genuine raw106B construct_native_group_00b8f5e0 overload
// in gui_page_root.hpp (actual name/current string pool/current constants).
// It creates NativeNodeStorage/atomic04 and a trivial actual20B typed tail.
// Admission requires that completed, still-live typed tail, before raw pointer
// helper access. Pure same-pool/duplicate/profile/count checks happen first;
// registry insertion can fail before any descriptor preparation. On successful
// insertion, end ONLY that trivial old tail view and establish a live12B
// SystemAmbientBacklinks at178, preserving all bytes. Never use the obsolete
// NativeGroupStorageView.group afterward. No logical GroupOwner is involved.
// This is separate source metadata/lifetime preparation, not native behavior.
// SAME18Ch pool membership uses current188, exact slot range/modulus and the
// current3180 free-index stack/31C0 count before atomic/registry admission.
class NativeGroupStorageReference final : public RenderCommandReference {
public:
    NativeGroupStorageReference(void* actual_group,
        NativeGroupStorageIdentityContext&, NativeGroupStorageLifetimeFrame&,
        NativeGroupStorageLifetimeAcquired&);
    ~NativeGroupStorageReference() override;
    NativeGroupStorageReference(const NativeGroupStorageReference&) = delete;
    NativeGroupStorageReference& operator=(const NativeGroupStorageReference&) = delete;
    // SAME actual+4 observed0/current0BD30E0 -> freshly current4B8F8C0/flags1.
    // Escaping source failures terminate at this noexcept canonical boundary.
    void release_zero_references() noexcept override;
    void* delete_scalar_00b8f8c0(const volatile std::uint32_t& flags);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGroupStorageIdentityContext& context_;
    NativeGroupStorageLifetimeFrame& frame_;
    NativeGroupStorageLifetimeAcquired& acquired_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
// Valid nonnegative accessible borrowed arrays use genuine singleton backing;
// current callback-modified nodes/tables remain valid. No SceneResource object
// view, copied count, extra retain, generated callback or unknown-profile fallback.
// SAME actual-owner registry/import domain, initialized persistent frames and
// fresh acquisitions are required. Reached child18/current0 bindings are genuine
// source providers with prepared recursive frames. All admitted destruction
// routes through the reference. Quiescence forbids reentry/slot reuse until
// retirement finishes. Canonical retirement is metadata-only, never proof of
// cleanup/return on failure; backing, frames, acquisitions and unresolved credits
// remain caller-owned for disposition. No postreturn reads or native FH3/SEH,
// private-stack/binary ABI/full application/game compatibility claim.
} // namespace bsp
