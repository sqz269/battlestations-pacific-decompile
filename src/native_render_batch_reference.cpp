#include "bsp/native_render_batch_reference.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
constexpr std::uint32_t pooled_batch_table = 0x00d5e5ac;
constexpr std::uint32_t reference_table = 0x00ceb130;
}

NativeRenderBatchReference::NativeRenderBatchReference(NativeRenderBatchStorage& storage,
    NativeRenderBatchLifetime& batches, const volatile std::uint32_t* table,
    NativeRenderBatchCompanionDisposal disposal)
    : reference_count(storage.references_04), storage_(storage), batches_(batches),
      table_(table), disposal_(disposal) {
    if (!disposal.retire || reference_count.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("batch companion requires live actual+04 and host retirement");
    require_current_profile();
}

NativeRenderBatchReference::~NativeRenderBatchReference() {
    if (phase_ != Phase::retired) std::terminate();
}

void NativeRenderBatchReference::require_current_profile() const {
    if (storage_.native_vtable_00 != pooled_batch_table || !table_ ||
        table_[0] != 0x00b55680 || table_[1] != 0x00b1c630)
        throw std::invalid_argument("unsupported current native batch terminal profile");
}

void NativeRenderBatchReference::release_zero_00b55680() {
    if (phase_ != Phase::bound || reference_count.load(std::memory_order_relaxed) != 0)
        throw std::logic_error("batch terminal requires a bound companion at actual count zero");
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    try {
        batches_.recycle_zero_reference_00b55680(storage_);
    } catch (...) {
        // In the valid domain CEB130 proves the entry destructor completed;
        // B55680 has not appended this dead slot. Retire only host association
        // and preserve the original pool/lock/allocation exception. A rejected
        // dependency profile before destruction leaves the association bound.
        phase_ = Phase::bound;
        if (storage_.native_vtable_00 == reference_table) {
            phase_ = Phase::retired;
            disposal.retire(disposal.context, *this,
                NativeRenderBatchRetirement::unreturned_dead_slot);
        }
        throw;
    }
    // Do not inspect storage after successful append: another acquisition may
    // begin its next native lifetime as soon as the caller's association
    // synchronization permits. The saved disposal owns this host lifetime.
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this, NativeRenderBatchRetirement::returned_to_pool);
}

void release_native_render_batch_reference(NativeRenderBatchReferences& references,
    NativeRenderBatchStorage& storage) {
    if (storage.references_04.fetch_sub(1, std::memory_order_seq_cst) == 1) {
        auto& reference = references.resolve_actual_batch(&storage);
        if (&reference.storage() != &storage || &reference.reference_count != &storage.references_04)
            throw std::invalid_argument("batch companion must borrow the captured actual storage/count");
        reference.release_zero_00b55680();
    }
}
} // namespace bsp
