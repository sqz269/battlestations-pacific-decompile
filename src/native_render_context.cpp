#include "bsp/native_render_context.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <array>

namespace bsp {
namespace {
constexpr std::uint32_t reference_table = 0x00ceb130;
constexpr std::uint32_t context_table = 0x00d5e5c4;

class ContextBaseCleanup final {
public:
    explicit ContextBaseCleanup(NativeRenderContextStorage& storage) noexcept : storage_(storage) {}
    ~ContextBaseCleanup() { storage_.native_vtable_00 = reference_table; }
private:
    NativeRenderContextStorage& storage_;
};

void release_then_clear(void*& slot, NativeRenderActualOwners& owners) {
    void* const captured = slot;
    if (captured) {
        release_native_render_actual_owner(owners, captured);
        slot = nullptr;
    }
}
} // namespace

struct NativeRenderActualOwnerRegistry::Impl final {
    struct LegacyEntry final {
        void* identity;
        RenderCommandReference* canonical;
        LegacyEntry* previous{};
        LegacyEntry* next{};
        LegacyEntry* bucket_previous{};
        LegacyEntry* bucket_next{};
        LegacyEntry(void* key, RenderCommandReference* value) noexcept
            : identity(key), canonical(value) {}
    };
    mutable std::mutex mutex;
    LegacyEntry* legacy_head{};
    ExactRecord* exact_head{};
    static constexpr std::size_t bucket_count = 4096;
    std::array<LegacyEntry*, bucket_count> legacy_buckets{};
    std::array<ExactRecord*, bucket_count> exact_buckets{};
    std::size_t retained_count{};

    static std::size_t bucket(void* identity) noexcept {
        // Opaque address bits are an internal metadata index only. Multiplying
        // before taking high bits also distributes aligned pool allocation keys.
        const auto word = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(identity) >> 2);
        return static_cast<std::uint32_t>(word * 2654435761u) >> 20;
    }
    LegacyEntry* legacy(void* identity) const noexcept {
        for (auto* entry = legacy_buckets[bucket(identity)]; entry; entry = entry->bucket_next)
            if (entry->identity == identity) return entry;
        return nullptr;
    }
    ExactRecord* indexed_exact(void* identity) const noexcept {
        for (auto* record = exact_buckets[bucket(identity)]; record; record = record->bucket_next_) {
            // Independent genuine end facts remove only this old key from
            // active indexing, even if poisoned. The node/caller/debt stay retained.
            if (record->normal_native_dead_.load(std::memory_order_acquire) ||
                record->unborn_abandoned_.load(std::memory_order_acquire) ||
                record->partial_cpp_ended_.load(std::memory_order_acquire)) continue;
            if (record->actual_key_ == identity && identity) return record;
        }
        return nullptr;
    }
    bool key_present(void* identity) const noexcept {
        return legacy(identity) != nullptr || indexed_exact(identity) != nullptr;
    }
    bool contains(const ExactRecord& sought) const noexcept {
        for (auto* record = exact_head; record; record = record->next_)
            if (record == &sought) return true;
        return false;
    }
    void insert(LegacyEntry& entry) noexcept {
        entry.next = legacy_head;
        if (legacy_head) legacy_head->previous = &entry;
        legacy_head = &entry;
        auto& head = legacy_buckets[bucket(entry.identity)];
        entry.bucket_next = head;
        if (head) head->bucket_previous = &entry;
        head = &entry;
        ++retained_count;
    }
    void detach(LegacyEntry& entry) noexcept {
        if (entry.previous) entry.previous->next = entry.next;
        else legacy_head = entry.next;
        if (entry.next) entry.next->previous = entry.previous;
        if (entry.bucket_previous) entry.bucket_previous->bucket_next = entry.bucket_next;
        else legacy_buckets[bucket(entry.identity)] = entry.bucket_next;
        if (entry.bucket_next) entry.bucket_next->bucket_previous = entry.bucket_previous;
        --retained_count;
    }
    void index_exact(ExactRecord& record) noexcept {
        auto& head = exact_buckets[bucket(record.actual_key_)];
        record.bucket_next_ = head;
        if (head) head->bucket_previous_ = &record;
        head = &record;
    }
    void detach_exact(ExactRecord& record) noexcept {
        if (record.actual_key_) {
            if (record.bucket_previous_) record.bucket_previous_->bucket_next_ = record.bucket_next_;
            else exact_buckets[bucket(record.actual_key_)] = record.bucket_next_;
            if (record.bucket_next_) record.bucket_next_->bucket_previous_ = record.bucket_previous_;
            record.bucket_previous_ = nullptr;
            record.bucket_next_ = nullptr;
        }
        if (record.previous_) record.previous_->next_ = record.next_;
        else exact_head = record.next_;
        if (record.next_) record.next_->previous_ = record.previous_;
        record.previous_ = nullptr;
        record.next_ = nullptr;
        record.retained_member_ = false;
        --retained_count;
    }
};

NativeRenderActualOwnerRegistry::BindingAdmission::BindingAdmission(
    NativeRenderActualOwnerRegistry& registry, void* identity,
    RenderCommandReference** entry) noexcept
    : registry_(&registry), identity_(identity), entry_(entry) {}

NativeRenderActualOwnerRegistry::BindingAdmission::BindingAdmission(
    BindingAdmission&& other) noexcept
    : registry_(other.registry_), identity_(other.identity_), entry_(other.entry_) {
    other.registry_ = nullptr;
    other.identity_ = nullptr;
    other.entry_ = nullptr;
}

NativeRenderActualOwnerRegistry::BindingAdmission&
NativeRenderActualOwnerRegistry::BindingAdmission::operator=(BindingAdmission&& other) noexcept {
    if (this != &other) {
        cancel();
        registry_ = other.registry_;
        identity_ = other.identity_;
        entry_ = other.entry_;
        other.registry_ = nullptr;
        other.identity_ = nullptr;
        other.entry_ = nullptr;
    }
    return *this;
}

NativeRenderActualOwnerRegistry::BindingAdmission::~BindingAdmission() noexcept { cancel(); }

void NativeRenderActualOwnerRegistry::BindingAdmission::cancel() noexcept {
    if (!registry_) return;
    std::unique_ptr<Impl::LegacyEntry> detached;
    {
        auto& impl = *registry_->impl_;
        const std::lock_guard<std::mutex> lock(impl.mutex);
        auto* const found = impl.legacy(identity_);
        if (!found || &found->canonical != entry_ || found->canonical != nullptr)
            std::terminate();
        impl.detach(*found);
        detached.reset(found);
        registry_ = nullptr;
        identity_ = nullptr;
        entry_ = nullptr;
    }
    // Metadata deallocation occurs only after the mutex has been released.
}

NativeRenderActualOwnerRegistry::NativeRenderActualOwnerRegistry()
    : impl_(std::make_unique<Impl>()) {}

NativeRenderActualOwnerRegistry::~NativeRenderActualOwnerRegistry() {
    if (!empty()) std::terminate();
}

void NativeRenderActualOwnerRegistry::bind(void* identity,
    RenderCommandReference& reference) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % alignof(std::uint32_t) != 0)
        throw std::invalid_argument("native owner registry requires an aligned actual identity");
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    if (&reference.reference_count != actual ||
        actual->load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument(
            "native owner registry companion must borrow the live actual +04 atomic");
    // Preallocate before the metadata lock. No Native field or count changes.
    auto entry = std::make_unique<Impl::LegacyEntry>(identity, &reference);
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        if (!impl_->key_present(identity)) {
            impl_->insert(*entry);
            entry.release();
            return;
        }
    }
    throw std::logic_error("native owner identity already has a canonical companion");
}

NativeRenderActualOwnerRegistry::BindingAdmission
NativeRenderActualOwnerRegistry::prepare_binding(void* identity) {
    if (!identity || reinterpret_cast<std::uintptr_t>(identity) % alignof(std::uint32_t) != 0)
        throw std::invalid_argument("native owner registry requires an aligned actual identity");
    auto entry = std::make_unique<Impl::LegacyEntry>(identity, nullptr);
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        if (!impl_->key_present(identity)) {
            auto* const retained = entry.release();
            impl_->insert(*retained);
            return BindingAdmission(*this, identity, &retained->canonical);
        }
    }
    throw std::logic_error("native owner identity already has a registry entry");
}

void NativeRenderActualOwnerRegistry::publish(BindingAdmission&& admission,
    RenderCommandReference& reference) {
    if (admission.registry_ != this) std::terminate();
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    auto* const found = impl_->legacy(admission.identity_);
    if (!found || &found->canonical != admission.entry_ || found->canonical != nullptr)
        std::terminate();
    // Preserve the legacy capability validation before its actual +04 read.
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(admission.identity_) + 4));
    if (&reference.reference_count != actual ||
        actual->load(std::memory_order_relaxed) <= 0)
        std::terminate();
    *admission.entry_ = &reference;
    admission.registry_ = nullptr;
    admission.identity_ = nullptr;
    admission.entry_ = nullptr;
}

void NativeRenderActualOwnerRegistry::unbind(void* identity,
    RenderCommandReference& reference) noexcept {
    std::unique_ptr<Impl::LegacyEntry> detached;
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        auto* const found = impl_->legacy(identity);
        if (!found || found->canonical != &reference) std::terminate();
        impl_->detach(*found);
        detached.reset(found);
    }
}

RenderCommandReference* NativeRenderActualOwnerRegistry::find(void* identity) {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    if (auto* const found = impl_->legacy(identity)) return found->canonical;
    auto* const exact = impl_->indexed_exact(identity);
    if (!exact || exact->poisoned_.load(std::memory_order_acquire) ||
        exact->availability_.load(std::memory_order_acquire) != ExactRecord::Availability::live)
        return nullptr;
    return exact->canonical_;
}

RenderCommandReference& NativeRenderActualOwnerRegistry::resolve_actual(void* identity) {
    auto* const found = find(identity);
    if (!found)
        throw std::logic_error("actual render owner has no canonical companion");
    return *found;
}

std::size_t NativeRenderActualOwnerRegistry::size() const {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->retained_count;
}

bool NativeRenderActualOwnerRegistry::empty() const {
    return size() == 0;
}

std::mutex& NativeRenderActualOwnerRegistry::exact_metadata_mutex() noexcept {
    return impl_->mutex;
}

void NativeRenderActualOwnerRegistry::attach_exact_record(ExactRecord& record) {
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    attach_exact_record_locked(record);
}

void NativeRenderActualOwnerRegistry::attach_exact_record_locked(ExactRecord& record) noexcept {
    // Private owning callers hold exact_metadata_mutex() and have already
    // reserved all storage. Contract errors do not become recovery authority.
    if (record.registry_ != this || !record.owning_identity_ || record.retained_member_ ||
        record.retired_ || record.actual_key_ || impl_->contains(record))
        std::terminate();
    record.next_ = impl_->exact_head;
    if (impl_->exact_head) impl_->exact_head->previous_ = &record;
    impl_->exact_head = &record;
    record.retained_member_ = true;
    ++impl_->retained_count;
}

void NativeRenderActualOwnerRegistry::prepare_exact_record(
    ExactRecord& record, void* captured_actual_key) {
    if (!captured_actual_key || reinterpret_cast<std::uintptr_t>(captured_actual_key) %
            alignof(std::uint32_t) != 0)
        throw std::invalid_argument("native exact owner requires a captured aligned actual identity");
    bool invalid_record;
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        invalid_record = record.registry_ != this || !record.retained_member_ || !impl_->contains(record) ||
            record.retired_ || record.actual_key_ ||
            record.poisoned_.load(std::memory_order_acquire) ||
            record.normal_native_dead_.load(std::memory_order_acquire) ||
            record.unborn_abandoned_.load(std::memory_order_acquire) ||
            record.partial_cpp_ended_.load(std::memory_order_acquire);
        if (!invalid_record && !impl_->key_present(captured_actual_key)) {
            record.actual_key_ = captured_actual_key;
            impl_->index_exact(record);
            return;
        }
    }
    if (invalid_record)
        throw std::logic_error("native exact owner preparation requires its fresh retained record");
    throw std::logic_error("native owner identity already has a registry entry");
}

void NativeRenderActualOwnerRegistry::publish_exact_record(
    ExactRecord& record, RenderCommandReference& canonical) noexcept {
    // Genuine owner-only ctor return, canonical construction and one-shot
    // preparation are preconditions established before entering this seam.
    record.canonical_ = &canonical;
    record.availability_.store(ExactRecord::Availability::live, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::poison_exact_record(ExactRecord& record) noexcept {
    record.poisoned_.store(true, std::memory_order_release);
    record.availability_.store(ExactRecord::Availability::unavailable, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::deactivate_exact_record_from_native_end(
    ExactRecord& record) noexcept {
    // Invoked only after real normal whole-payload cleanup and the last late
    // flags read. Does not inspect Native storage, enter a lock or call a provider.
    record.availability_.store(ExactRecord::Availability::unavailable, std::memory_order_release);
    record.normal_native_dead_.store(true, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::withhold_exact_record_for_live_storage_debt(
    ExactRecord& record) noexcept {
    // Legitimate flags0 after the actual family payload and its late flags read.
    // Material110h/Section60h carrier and physical slot stay owed, without Source failure.
    record.availability_.store(ExactRecord::Availability::unavailable, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::deactivate_exact_record_from_partial_cpp_end(
    ExactRecord& record) noexcept {
    // Private owning continuation proved actual C++ prefix birth, ctor escape,
    // no whole Native return/publication/borrow, selected SAME raw operand and
    // actual last-prefix-use plus trivial prefix end. No Native access here.
    // Caller metadata and the separately live pool index survive the pool call.
    record.availability_.store(ExactRecord::Availability::unavailable, std::memory_order_release);
    record.partial_cpp_ended_.store(true, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::abandon_exact_unborn_record(ExactRecord& record) noexcept {
    // Private authentic raw-slot unwind, NOT normal Native destruction or a
    // physical-return receipt. BEFORE ANY Owner/Node/payload C++ birth and
    // native ctor entry; no publication. Actual family pool-index access stays
    // with the real return provider. Partial/failed constructor is NOT enough.
    record.availability_.store(ExactRecord::Availability::unavailable, std::memory_order_release);
    record.unborn_abandoned_.store(true, std::memory_order_release);
}

void NativeRenderActualOwnerRegistry::retire_exact_record(ExactRecord& record) {
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        if (record.registry_ == this && record.retained_member_ && impl_->contains(record) &&
            !record.retired_ && (record.normal_native_dead_.load(std::memory_order_acquire) ||
                record.unborn_abandoned_.load(std::memory_order_acquire))) {
            record.retired_ = true;
            return;
        }
    }
    throw std::logic_error("native exact owner retirement requires that authentically ended retained record");
}

void NativeRenderActualOwnerRegistry::cancel_exact_record_without_payload(ExactRecord& record) {
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        if (record.registry_ == this && record.retained_member_ && impl_->contains(record) &&
            !record.actual_key_ && !record.canonical_ && !record.retired_ &&
            record.availability_.load(std::memory_order_acquire) == ExactRecord::Availability::unavailable &&
            !record.normal_native_dead_.load(std::memory_order_acquire) &&
            !record.unborn_abandoned_.load(std::memory_order_acquire) &&
            !record.partial_cpp_ended_.load(std::memory_order_acquire) &&
            record.outer_calls_.load(std::memory_order_acquire) == 0 &&
            record.borrows_.load(std::memory_order_acquire) == 0) {
            // Only the genuine owner knows allocator-never-entered or actual
            // normal-NULL. A missing key alone does not prove either fact.
            impl_->detach_exact(record);
            return;
        }
    }
    throw std::logic_error("native exact owner cancellation requires that quiescent unpublished payload-free record");
}

void NativeRenderActualOwnerRegistry::close_exact_record_at_quiescence(ExactRecord& record) {
    {
        const std::lock_guard<std::mutex> lock(impl_->mutex);
        if (record.registry_ == this && record.retained_member_ && impl_->contains(record) &&
            record.retired_ && (record.normal_native_dead_.load(std::memory_order_acquire) ||
                record.unborn_abandoned_.load(std::memory_order_acquire)) &&
            record.outer_calls_.load(std::memory_order_acquire) == 0 &&
            record.borrows_.load(std::memory_order_acquire) == 0) {
            // The genuine owner additionally proves physical disposition, all
            // links and outer Source fences; counters do not infer those facts.
            impl_->detach_exact(record);
            return;
        }
    }
    throw std::logic_error("native exact owner close requires that retired quiescent retained record");
}


void NativeRenderActualOwnerRegistry::bind_callback(void* context, void* identity,
    RenderCommandReference& reference) {
    if (!context) throw std::invalid_argument("native owner registry context is null");
    static_cast<NativeRenderActualOwnerRegistry*>(context)->bind(identity, reference);
}

void NativeRenderActualOwnerRegistry::unbind_callback(void* context, void* identity,
    RenderCommandReference& reference) noexcept {
    if (!context) std::terminate();
    static_cast<NativeRenderActualOwnerRegistry*>(context)->unbind(identity, reference);
}

RenderCommandReference* NativeRenderActualOwnerRegistry::find_callback(
    void* context, void* identity) {
    if (!context) throw std::invalid_argument("native owner registry context is null");
    return static_cast<NativeRenderActualOwnerRegistry*>(context)->find(identity);
}

void dispatch_native_render_actual_owner_zero(
    NativeRenderActualOwners& owners, void* identity) {
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    auto& reference = owners.resolve_actual(identity);
    if (&reference.reference_count != actual)
        throw std::invalid_argument("native owner companion must borrow the same actual +04 atomic");
    reference.release_zero_references();
}

void release_native_render_actual_owner(NativeRenderActualOwners& owners, void* identity) {
    NativeRenderActualOwnerReleaseObservation observation;
    release_native_render_actual_owner_observed(owners, identity, observation);
}

void release_native_render_actual_owner_observed(NativeRenderActualOwners& owners, void* identity,
    NativeRenderActualOwnerReleaseObservation& observation) {
    const NativeRenderObservedZeroAccess::Invocation invocation(observation.cpp_zero_frame);
    if (observation.entered)
        throw std::logic_error("actual owner release observation is one-shot");
    observation.identity = identity;
    observation.entered = true;
    auto* const actual = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    observation.previous_count = actual->fetch_sub(1, std::memory_order_seq_cst);
    observation.result_bits = static_cast<std::uint32_t>(observation.previous_count) - 1u;
    observation.decremented = true;
    if (observation.previous_count == 1) {
        observation.zero_dispatch_entered = true;
        NativeRenderObservedZeroAccess::dispatch(owners, identity, observation.cpp_zero_frame);
        // Only external retained metadata is touched after the terminal.
        observation.zero_dispatch_returned = true;
    }
    observation.returned = true;
}

NativeRenderContextStorage* initialize_native_render_context_00b1edc0_fragment(void* raw) noexcept {
    auto* context = ::new (raw) NativeRenderContextStorage;
    context->native_vtable_00 = reference_table;
    context->references_04.store(1, std::memory_order_relaxed);
    context->native_vtable_00 = context_table;
    context->camera_08 = nullptr;
    context->second_owner_0c = nullptr;
    context->borrowed_command_10 = nullptr;
    context->target_14 = nullptr;
    return context;
}

void destroy_native_render_context_00b1d120(
    NativeRenderContextStorage& context, NativeRenderActualOwners& owners) {
    context.native_vtable_00 = context_table;
    const ContextBaseCleanup base(context);
    release_then_clear(context.camera_08, owners);
    release_then_clear(context.second_owner_0c, owners);
    release_then_clear(context.target_14, owners);
}

NativeRenderContextStorage* delete_native_render_context_00b1d570(
    NativeRenderContextStorage* context, NativeRenderActualOwners& owners, std::uint32_t flags) {
    destroy_native_render_context_00b1d120(*context, owners);
    if (flags & 1) singleton_lifetime_free(context);
    return context;
}

NativeRenderContextReference::NativeRenderContextReference(NativeRenderContextStorage& storage,
    NativeRenderActualOwners& owners, const volatile std::uint32_t* table,
    NativeRenderContextCompanionDisposal disposal)
    : RenderCommandReference(storage.references_04), storage_(storage), owners_(owners),
      vtable_00d5e5c4_(table), disposal_(disposal) {
    if (!disposal.retire || storage.references_04.load(std::memory_order_relaxed) <= 0)
        throw std::invalid_argument("context reference requires live actual storage and explicit companion retirement");
    require_current_profile();
}

NativeRenderContextReference::~NativeRenderContextReference() {
    if (phase_ != Phase::retired) std::terminate();
}

void NativeRenderContextReference::require_current_profile() const noexcept {
    if (storage_.native_vtable_00 != context_table || !vtable_00d5e5c4_ ||
        vtable_00d5e5c4_[0] != 0x00bd30e0 || vtable_00d5e5c4_[1] != 0x00b1d570)
        std::terminate();
}

void NativeRenderContextReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    require_current_profile();
    phase_ = Phase::destroying;
    const auto disposal = disposal_;
    delete_native_render_context_00b1d570(&storage_, owners_, 1);
    phase_ = Phase::retired;
    disposal.retire(disposal.context, *this);
}

} // namespace bsp
