#pragma once
#include "bsp/native_gui_text_default_storage.hpp"
#include "bsp/native_gui_text_retirement.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_context.hpp"
#include <cstddef>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
class NativeGuiTextReference;

struct NativeGuiTextReferenceDisposal {
    // Explicit companion-only retirement, not a native Text destructor. Supply
    // the actual extent of nonnull context backing; nullptr requires zero bytes.
    void* context;
    std::size_t context_bytes;
    void (*retire)(void*, NativeGuiTextReference&) noexcept;
};
struct NativeGuiTextReferenceFrame {
    // Persistent separate callframe. Initial zero_flags bits are irrelevant:
    // observed-zero dispatch writes literal DWORD1 at the reached scalar call.
    volatile std::uint32_t zero_flags{};
    NativeGuiTextScalarRetirementAcquired scalar;
};
struct NativeGuiTextReferenceContext {
    NativeRenderActualOwnerRegistry& owners; // SAME118 actual owner registry
    game::GameNativeReadOnlyData& data; // existing initialized Original-VA bands
    NativeGuiTextDefaultStorageContext& construction_context;
    NativeGuiTextDefaultStorageFrame& construction_frame;
    const NativeGuiTextDefaultStorageAcquired& construction;
    NativeGuiTextRetirementContext& retirement;
};

// Canonical metadata for the SAME completed actual93 Text and its +04 atomic.
// No native owner, replacement count, retain, fake vtable or typed GUI overlay.
// Construct only with bind_native_gui_text_reference and caller-provided storage;
// its preflight runs BEFORE placement vptr/base/member stores and registry.bind.
class NativeGuiTextReference final : public RenderCommandReference,
    private NativeRefCountedDeleteCalls {
public:
    enum class Phase { bound, zero_dispatch, retiring, retired, failed };
    ~NativeGuiTextReference() override;
    NativeGuiTextReference(const NativeGuiTextReference&) = delete;
    NativeGuiTextReference& operator=(const NativeGuiTextReference&) = delete;
    NativeGuiTextReference(NativeGuiTextReference&&) = delete;
    NativeGuiTextReference& operator=(NativeGuiTextReference&&) = delete;
    Phase phase() const noexcept { return phase_; }
    // Diagnostic identity only after retirement/failure; never permission to
    // rediscover fields, read a dead count, retry, or operate on a recycled slot.
    void* storage() const noexcept { return identity_; }

    // Metadata-only borrow of this SAME completed93/canonical122 service graph.
    // This grants no new native lifetime/count credit. Field operations still
    // require bound phase, live backing and the caller's serialized lifetime.
    const NativeGuiTextReferenceContext& storage_context() const noexcept { return context_; }

    // Actual already-zero entry. Reuses wholeBD30E0/current04/real118; does not
    // decrement again. Source noexcept policy terminates on any entered error.
    void release_zero_references() noexcept override;
    // Genuine direct current04: no count load/gate/decrement. Caller flags are
    // live stable DWORD backing, separate from this/frame/known provider graph;
    // only real118 reads their CURRENT LOW BYTE, after its destructor returns.
    // Escaping failure keeps this binding and every independent graph/frame;
    // no unbind/disposal/rollback/retry or automatic native cleanup follows.
    void* delete_current_scalar_00ab8ee0(const volatile std::uint32_t& caller_flags);

private:
    friend NativeGuiTextReference& bind_native_gui_text_reference(void*, void*,
        NativeGuiTextReferenceContext&, NativeGuiTextReferenceFrame&,
        NativeGuiTextReferenceDisposal);
    NativeGuiTextReference(void*, NativeGuiTextReferenceContext&,
        NativeGuiTextReferenceFrame&, NativeGuiTextReferenceDisposal,
        const volatile std::uint32_t*, std::atomic<std::int32_t>&);
    void delete_vslot04(void*, std::uint32_t captured_profile, std::uint32_t flags) override;
    void require_slot(std::uint32_t captured_profile, unsigned slot, std::uint32_t target) const;
    struct Completion {
        void* identity;
        NativeRenderActualOwnerRegistry* owners;
        NativeGuiTextReferenceDisposal disposal;
    };
    Completion completion() const noexcept;
    void finish_success(Completion) noexcept;
    void* const identity_;
    NativeGuiTextReferenceContext& context_;
    NativeGuiTextReferenceFrame& frame_;
    const NativeGuiTextReferenceDisposal disposal_;
    const volatile std::uint32_t* const profile_;
    Phase phase_{Phase::bound};
};

// companion_storage is aligned sizeof(NativeGuiTextReference) caller storage.
// Completed genuine producer provenance, SAME actual pool/slab/trailer and the
// WHOLE independently retained creator/context/profile/registry/backing graph
// remain caller obligations. Editable completion diagnostics are necessary
// checks, never proof of actual ownership. Keep distinct prepared recursive
// frames BEFORE callbacks; current children/timed providers remain required.
// Known-region checks do not establish opaque/transitive extent disjointness.
// All graph regions are disjoint from Text, this companion and writable frame;
// required canonical provider aliases may share their genuine identities.
// Registry bind failure leaves no constructed companion and performs no native
// cleanup/count operation; actual raw creator and graph stay with the caller.
//
// Serialize Native lifetime and exclude slot reuse/new binding through the
// post-scalar unbind. A successful118 may already return the slot; subsequent
// completion touches metadata only, then calls saved disposal LAST (may destroy
// this companion). Direct raw118 bypass while bound is excluded. Forced scalar
// or ancestor retirement is not prevented by a positive count/held creator.
// A failed direct call retains its registry entry only as failed diagnostics:
// no further Native count/virtual access, retry/reset/assignment or raw cleanup.
// Metadata destructor requires success-retired and never cleans a failed graph.
// Source checks/zero-noexcept policy are not Native FH3/SEH/register ABI or
// arbitrary failure equivalence. No factory/copy/activation/production binding.
NativeGuiTextReference& bind_native_gui_text_reference(void* companion_storage,
    void* actual_text, NativeGuiTextReferenceContext&, NativeGuiTextReferenceFrame&,
    NativeGuiTextReferenceDisposal);
} // namespace bsp
