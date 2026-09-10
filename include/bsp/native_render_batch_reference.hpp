#pragma once
#include "bsp/native_render_batch_lifetime.hpp"

namespace bsp {

class NativeRenderBatchReference;
enum class NativeRenderBatchRetirement { returned_to_pool, unreturned_dead_slot };
struct NativeRenderBatchCompanionDisposal {
    void* context;
    // Pure host association retirement, exactly once after native destruction.
    // On failure the actual dead slot was NOT appended; do not free, append,
    // retain or modify it here. This callback may dispose the companion; no
    // native/companion access follows. It must neither allocate nor throw.
    void (*retire)(void*, NativeRenderBatchReference&, NativeRenderBatchRetirement) noexcept;
};

// Standalone canonical companion for one acquired actual18h batch lifetime.
// B55680 can throw while obtaining/growing its pool AFTER batch destruction;
// it cannot use RenderCommandReference's nonthrowing terminal interface.
// The supplied table view is the SAME current D5E5AC view used by batches.
// This constructor neither initializes nor retains the actual +04 atomic.
class NativeRenderBatchReference final {
public:
    NativeRenderBatchReference(NativeRenderBatchStorage&, NativeRenderBatchLifetime& batches,
        const volatile std::uint32_t* same_actual_vtable_00d5e5ac,
        NativeRenderBatchCompanionDisposal);
    ~NativeRenderBatchReference();
    NativeRenderBatchReference(const NativeRenderBatchReference&) = delete;
    NativeRenderBatchReference& operator=(const NativeRenderBatchReference&) = delete;

    std::atomic<std::int32_t>& reference_count;
    NativeRenderBatchStorage& storage() noexcept { return storage_; }
    // Count has ALREADY reached zero. Validate current table and slots0/4,
    // delegate real native destruction/pool return, retire the host association.
    // A lazy-pool/append failure retires as unreturned_dead_slot and rethrows;
    // it does not retry, scalar-free the dead slot or undo native state.
    void release_zero_00b55680();

private:
    enum class Phase { bound, destroying, retired };
    NativeRenderBatchStorage& storage_;
    NativeRenderBatchLifetime& batches_;
    const volatile std::uint32_t* table_;
    NativeRenderBatchCompanionDisposal disposal_;
    Phase phase_{Phase::bound};
    void require_current_profile() const;
};

// Pure canonical association: no retain, release, allocation or other lifetime
// behavior in lookup. It may reject an unbound identity; no no-op fallback.
// Actual backing, batch domain and companion must survive until terminal call.
class NativeRenderBatchReferences {
public:
    virtual ~NativeRenderBatchReferences() = default;
    virtual NativeRenderBatchReference& resolve_actual_batch(void* actual_batch) = 0;
};

// Capture/decrement actual+04 first; resolve only at zero and validate that the
// concrete companion borrows exactly this storage/count. No access follows its
// terminal callback, which may destroy the companion and return native storage.
void release_native_render_batch_reference(NativeRenderBatchReferences&,
    NativeRenderBatchStorage&);

// Native valid-domain preconditions: live acquired batch and canonical profile,
// valid nonnegative entry/free-list headers and spans, same singleton/pool/lock
// domain, no concurrent profile mutation or slot reuse before host retirement.
// Successful pool append cannot throw afterward. Destruction of the batch's
// borrowed entry storage itself cannot throw in this domain; later pool/lock
// lookup and free-list allocation can. A live companion is never implicitly
// destroyed; caller-prepared host storage survives any outstanding reference.
} // namespace bsp
