#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include "bsp/native_post_effect_construction.hpp"
#include "bsp/native_post_effect20_construction.hpp"
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace bsp {
void return_native_mesh_slot_00b72f70(NativeMeshStorageFrame&, void*);
// One retained receiver for this record's actual once-only scalar action.
// Its per-reached nodes are Host metadata, never a Native operand list/count.
class NativeMeshPayloadReleaseReceiver final {
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalAccess;
    struct Node final {
        explicit Node(void* captured) noexcept : captured_(captured) {}
        Node* next_{};
        void* const captured_;
        NativeRenderActualOwnerReleaseObservation observation_;
    };
    explicit NativeMeshPayloadReleaseReceiver(NativeMeshCanonicalRecord& record) noexcept
        : record_(record) {}
    ~NativeMeshPayloadReleaseReceiver() noexcept {
        // C++ metadata only. The actual owner must retain failed/unknown graphs
        // instead of ending this storage; no Native cleanup is supplied here.
        while (first_) { auto* next=first_->next_; delete first_; first_=next; }
    }
    NativeMeshPayloadReleaseReceiver(const NativeMeshPayloadReleaseReceiver&)=delete;
    NativeMeshPayloadReleaseReceiver& operator=(const NativeMeshPayloadReleaseReceiver&)=delete;
    NativeMeshCanonicalRecord& record_;
    Node* first_{};
    std::atomic<bool> entered_{false}, active_{false}, returned_{false}, failed_{false}, drained_{false};
};
class NativeMeshCanonicalRecord final {
    friend class NativeMeshCanonicalDomain;
    friend class NativeMeshCanonicalProducerFrame;
    friend class NativeMeshCanonicalAccess;
    friend class NativeMeshCanonicalBorrow;
    friend class NativeMeshReference;
    friend struct std::default_delete<NativeMeshCanonicalRecord>;
    enum class Phase : std::uint8_t { cold, allocated, constructed, live, dead, retired, closed };
    NativeMeshCanonicalDomain& domain_;
    const void* const producer_identity_;
    NativeRenderActualOwnerRegistry::ExactRecord exact_;
    NativeRenderObservedZeroHold observed_zero_hold_;
    NativeMeshStorageFrame lower_;
    NativeMeshPayloadReleaseReceiver payload_release_;
    NativeMeshCanonicalProducerFrame* producer_frame_{};
    NativePostEffectConstructionBlock* block24_{};
    NativePostEffect20ConstructionBlock* block20_{};
    NativeMeshCanonicalRecord* next_{};
    std::aligned_storage_t<sizeof(NativeMeshReference), alignof(NativeMeshReference)> reference_storage_;
    NativeMeshReference* canonical_{};
    NativeMeshStorage* payload_{}; // nulled at the actual private typed end
    std::atomic<Phase> phase_{Phase::cold};
    std::atomic<bool> invocation_{false};
    std::atomic<bool> poisoned_{false};
    std::atomic<bool> pending_custody_{false};
    std::atomic<bool> zero_action_{false};
    std::atomic<bool> scalar_entered_{false};
    std::atomic<unsigned> exception_state_{0};
    std::exception_ptr primary_;
    NativeRenderActualOwnerReleaseObservation creator_release_;
    bool captured_decrement_entered_{}, captured_decrement_returned_{};
    long captured_decrement_result_{};
    volatile std::uint32_t scalar_flags_; // written ONLY at genuine zero-call epoch
    bool creator_outstanding_{};
    bool source_scalar_returned_{};
    std::atomic<bool> canonical_zero_returned_{false};
    bool producer_link_closed_{};
    bool cold_cancelled_{};
    bool closed_{};

    NativeMeshCanonicalRecord(NativeMeshCanonicalDomain& d, const void* producer)
        : domain_(d), producer_identity_(producer), exact_(d.registry_, this), lower_(d.context_),
          payload_release_(*this) {
        lower_.canonical_record_ = this; // private, before any Native operation
    }
    ~NativeMeshCanonicalRecord() noexcept {
        if (canonical_) canonical_->~NativeMeshReference(); // raw mode: C++ metadata only
    }
    static std::mutex& registry_mutex(NativeMeshCanonicalDomain& d) noexcept {
        return d.registry_.exact_metadata_mutex();
    }
    void attach_locked() noexcept { domain_.registry_.attach_exact_record_locked(exact_); }
    void close_exact() { domain_.registry_.close_exact_record_at_quiescence(exact_); }
    bool physical_returned() const noexcept { return lower_.pool_returned_; }
    bool native_custody() const noexcept {
        return pending_custody_.load(std::memory_order_acquire);
    }
    void mark_failed_metadata() noexcept {
        poisoned_.store(true, std::memory_order_release);
        domain_.registry_.poison_exact_record(exact_);
        if (native_custody()) domain_.retention_.fail();
    }
    void failed() noexcept {
        mark_failed_metadata();
        auto current = std::current_exception();
        unsigned expected = 0;
        if (current && exception_state_.compare_exchange_strong(expected, 1, std::memory_order_acq_rel)) {
            primary_ = std::move(current);
            exception_state_.store(2, std::memory_order_release);
        }
    }
    void healthy() {
        if (poisoned_.load(std::memory_order_acquire)) {
            failed();
            throw std::logic_error("Mesh owning invocation was reentered or poisoned");
        }
    }
    class Call final {
    public:
        explicit Call(NativeMeshCanonicalRecord& r, bool producer_operation = true)
            : record_(r), producer_operation_(producer_operation) {
            if (producer_operation_ && record_.invocation_.exchange(true, std::memory_order_acq_rel)) {
                record_.failed();
                throw std::logic_error("Mesh producer invocation cannot reenter or replay");
            }
            record_.exact_.outer_calls_.fetch_add(1, std::memory_order_acq_rel);
        }
        ~Call() noexcept {
            record_.exact_.outer_calls_.fetch_sub(1, std::memory_order_release);
            if (producer_operation_) record_.invocation_.store(false, std::memory_order_release);
        }
        Call(const Call&) = delete;
        Call& operator=(const Call&) = delete;
    private:
        NativeMeshCanonicalRecord& record_;
        bool producer_operation_;
    };
    static NativeMeshCanonicalRecord& get(NativeMeshCanonicalProducerFrame& f) {
        if (!f.record_ || f.record_->producer_frame_ != &f ||
            f.record_->producer_identity_ != f.producer_)
            throw std::logic_error("Mesh producer lost its exact retained record");
        return *f.record_;
    }
};

void NativeMeshCanonicalAccess::begin_payload_receiver(NativeMeshCanonicalRecord& r) {
    auto& receiver=r.payload_release_;
    if (!r.scalar_entered_.load(std::memory_order_acquire) || !r.canonical_ ||
        r.lower_.canonical_record_!=&r || r.lower_.payload_receiver_ ||
        r.lower_.active_ || r.lower_.phase_!=NativeMeshStorageFrame::Phase::live ||
        receiver.entered_.exchange(true,std::memory_order_acq_rel)) {
        r.failed();
        throw std::logic_error("Mesh payload receiver requires its genuine admitted scalar operation");
    }
    r.lower_.payload_receiver_=&receiver;
    receiver.active_.store(true,std::memory_order_release);
}
NativeMeshPayloadReleaseReceiver& NativeMeshCanonicalAccess::payload_receiver(
    NativeMeshCanonicalRecord& r, NativeMeshStorageFrame& lower) {
    auto& receiver=r.payload_release_;
    if (&lower!=&r.lower_ || lower.canonical_record_!=&r ||
        lower.payload_receiver_!=&receiver || !receiver.active_.load(std::memory_order_acquire) ||
        !lower.active_ || !lower.scalar_entered_ || !lower.destructor_entered_ ||
        lower.phase_!=NativeMeshStorageFrame::Phase::destroying) {
        r.failed();
        throw std::logic_error("Mesh payload lost its SAME private record/lower receiver binding");
    }
    return receiver;
}
void NativeMeshCanonicalAccess::release_payload_child(NativeMeshPayloadReleaseReceiver& receiver,
    NativeRenderActualOwners& owners, void* captured) {
    auto& r=receiver.record_;
    (void)payload_receiver(r,r.lower_);
    if (&owners!=&r.domain_.registry_ || &owners!=&r.domain_.environment_.retained_owners ||
        receiver.drained_.load(std::memory_order_acquire) || r.poisoned_.load(std::memory_order_acquire)) {
        r.failed();
        throw std::logic_error("Mesh child release requires its actual same-registry operation");
    }
    // The kernel has ALREADY captured this one CURRENT operand. Reserve and
    // retain its own observation before the actual helper/decrement can enter.
    auto pending=std::make_unique<NativeMeshPayloadReleaseReceiver::Node>(captured);
    auto* node=pending.get();node->next_=receiver.first_;receiver.first_=node;(void)pending.release();
    release_native_render_actual_owner_observed(owners,captured,node->observation_);
}
void NativeMeshCanonicalAccess::end_payload_receiver(NativeMeshCanonicalRecord& r,
    bool source_failed) noexcept {
    auto& receiver=r.payload_release_;
    if (!receiver.active_.load(std::memory_order_acquire)) {
        r.failed(); // rejected/saved guards never walk a different operation's heads
        return;
    }
    const bool failed=source_failed || r.lower_.payload_receiver_!=&receiver ||
        r.poisoned_.load(std::memory_order_acquire) || r.domain_.retention_.requires_process_retention();
    if (failed) {
        receiver.failed_.store(true,std::memory_order_release);r.failed();r.domain_.retention_.fail();
        // Only the admitted scalar owner reaches this mutable head traversal.
        for(auto* node=receiver.first_;node;node=node->next_)
            NativeRenderObservedZeroAccess::fail_frame(node->observation_.cpp_zero_frame);
    } else receiver.returned_.store(true,std::memory_order_release);
    if(r.lower_.payload_receiver_==&receiver)r.lower_.payload_receiver_=nullptr;
    receiver.active_.store(false,std::memory_order_release);
}
void NativeMeshCanonicalAccess::preflight_payload_receiver(NativeMeshCanonicalRecord& r, bool complete, ReadinessRejection rejection) {
    auto& receiver=r.payload_release_;
    const auto reject=[&r,rejection](const char* message) {
        if (rejection == ReadinessRejection::sticky) {r.failed();r.domain_.retention_.fail();}throw std::logic_error(message);
    };
    if(receiver.active_.load(std::memory_order_acquire) || r.lower_.payload_receiver_ ||
        receiver.failed_.load(std::memory_order_acquire) || r.poisoned_.load(std::memory_order_acquire) ||
        r.domain_.retention_.requires_process_retention())
        reject("Mesh payload receiver still active or its SAME graph failed");
    if(complete ? (!receiver.entered_.load(std::memory_order_acquire)||!receiver.returned_.load(std::memory_order_acquire)) :
        (receiver.entered_.load(std::memory_order_acquire)||receiver.first_))
        reject("Mesh payload receiver lacks actual whole-return or genuine never-entered facts");
    for(auto* node=receiver.first_;node;node=node->next_) {
        const auto& observation=node->observation_;
        if(observation.cpp_zero_frame.active_invocations_.load(std::memory_order_acquire)!=0u ||
            !observation.entered || observation.identity!=node->captured_ ||
            !observation.decremented || !observation.returned ||
            (observation.zero_dispatch_entered&&!observation.zero_dispatch_returned))
            reject("Mesh reached child observation did not pass its real return/health/caller fence");
    }
}
void NativeMeshCanonicalAccess::drain_payload_receiver(NativeMeshCanonicalRecord& r) {
    preflight_payload_receiver(r,!r.cold_cancelled_);
    auto& receiver=r.payload_release_;
    if(receiver.drained_.load(std::memory_order_acquire))return;
    for(auto* node=receiver.first_;node;node=node->next_)
        if(!NativeRenderObservedZeroAccess::end_frame(node->observation_.cpp_zero_frame,false)) {
            r.failed();r.domain_.retention_.fail();
            throw std::logic_error("Mesh child caller still has an active real Source invocation");
        }
    receiver.drained_.store(true,std::memory_order_release);
}

struct NativeMeshCanonicalDomain::Impl final {
    NativeMeshCanonicalRecord* head{};
    bool closing{}; // actual owner supplies close/producer quiescence
    ~Impl() noexcept {
        while (head) {
            auto* const next = head->next_;
            delete head; // legal owner has already closed every exact record
            head = next;
        }
    }
};

bool NativeMeshRetentionState::requires_process_retention() const noexcept {
    return failed_.load(std::memory_order_acquire);
}
bool NativeMeshRetentionState::has_retained_records() const noexcept {
    return retained_.load(std::memory_order_acquire) != 0u;
}
NativeMeshCanonicalDomain::NativeMeshCanonicalDomain(NativeMeshEnvironment& e,
    NativeMeshConstants c, NativeRenderActualOwnerRegistry& registry,
    NativeMeshRetentionState& retention)
    : environment_(e), registry_(registry), retention_(retention), constants_(c), context_(e, c),
      impl_(std::make_unique<Impl>()) {
    if (&e.retained_owners != &registry)
        throw std::invalid_argument("Mesh canonical domain requires its SAME actual registry");
}
NativeMeshCanonicalDomain::~NativeMeshCanonicalDomain() noexcept = default;
bool NativeMeshCanonicalDomain::requires_process_retention() const noexcept {
    return retention_.requires_process_retention();
}
bool NativeMeshCanonicalDomain::has_retained_records() const noexcept {
    return retention_.has_retained_records();
}
NativeMeshCanonicalRecord& NativeMeshCanonicalDomain::reserve(const void* producer) {
    if (!producer) throw std::invalid_argument("Mesh producer identity is required before Native effects");
    // All allocation/constructor failure is outside the registry mutex.
    std::unique_ptr<NativeMeshCanonicalRecord> candidate(new NativeMeshCanonicalRecord(*this, producer));
    bool closing = false;
    {
        std::lock_guard<std::mutex> lock(NativeMeshCanonicalRecord::registry_mutex(*this));
        closing = impl_->closing;
        if (!closing) {
            candidate->attach_locked();
            candidate->next_ = impl_->head;
            impl_->head = candidate.get();
            retention_.retained_.fetch_add(1, std::memory_order_release);
        }
    }
    if (closing) throw std::logic_error("Mesh canonical domain already entered actual close");
    return *candidate.release();
}
void NativeMeshCanonicalDomain::close_at_quiescence() {
    // Actual owning caller proves no concurrent producer/borrow/callback entry.
    // No count/profile/index/payload read is used to manufacture that proof.
    if (retention_.requires_process_retention())
        throw std::logic_error("Mesh graph failed before its actual close fence");
    {
        std::lock_guard<std::mutex> lock(NativeMeshCanonicalRecord::registry_mutex(*this));
        impl_->closing = true;
    }
    for (auto* r = impl_->head; r; r = r->next_) {
        if (r->closed_) continue;
        if (!r->producer_link_closed_ || r->invocation_.load(std::memory_order_acquire) ||
            r->exact_.borrows_.load(std::memory_order_acquire) != 0u ||
            r->poisoned_.load(std::memory_order_acquire) ||
            (!r->cold_cancelled_ && (!r->canonical_zero_returned_.load(std::memory_order_acquire) || !r->physical_returned()))) {
            retention_.fail();
            throw std::logic_error("Mesh close cannot drain active or unresolved owning custody");
        }
        NativeMeshCanonicalAccess::preflight_observed_outer_at_quiescence(*r);
        NativeMeshCanonicalAccess::preflight_payload_receiver(*r,!r->cold_cancelled_);
    }
    // Validate ALL records and their real caller receivers before the first
    // end_frame can unlink any hold. A later unresolved record must not leave
    // earlier records partially drained. The actual owner still supplies the
    // independent Native/worker/whole-caller quiescence fence.
    for (auto* r = impl_->head; r; r = r->next_) {
        if (r->closed_) continue;
        // Real graph quiescence is already established by the owning caller.
        // Drain retained caller metadata BEFORE testing the holds' counters;
        // never infer that quiescence from those counters or a phase flag.
        NativeMeshCanonicalAccess::drain_payload_receiver(*r);
        NativeMeshCanonicalAccess::drain_observed_outer_at_quiescence(*r);
        if (!r->producer_link_closed_ || r->invocation_.load(std::memory_order_acquire) ||
            r->exact_.outer_calls_.load(std::memory_order_acquire) != 0u ||
            r->exact_.borrows_.load(std::memory_order_acquire) != 0u ||
            r->poisoned_.load(std::memory_order_acquire) ||
            (!r->cold_cancelled_ && (!r->canonical_zero_returned_.load(std::memory_order_acquire) || !r->physical_returned()))) {
            retention_.fail();
            throw std::logic_error("Mesh close retains unresolved actual Native/Source custody");
        }
    }
    for (auto* r = impl_->head; r; r = r->next_) {
        if (r->closed_) continue;
        try {
            if (!r->cold_cancelled_) r->close_exact();
            r->closed_ = true;
            r->phase_.store(NativeMeshCanonicalRecord::Phase::closed, std::memory_order_release);
            retention_.retained_.fetch_sub(1, std::memory_order_release);
        } catch (...) { r->failed(); throw; }
    }
    // Records remain C++-owned until this quiescent domain's normal destruction.
}
NativeMeshCanonicalProducerFrame::NativeMeshCanonicalProducerFrame(
    NativeMeshCanonicalDomain& d, const void* producer) : domain_(d), producer_(producer) {
    auto& record = d.reserve(producer);
    record.producer_frame_ = this;
    record_ = &record;
}
bool NativeMeshCanonicalProducerFrame::requires_process_retention() const noexcept {
    return record_ && (record_->poisoned_.load(std::memory_order_acquire) ||
        record_->pending_custody_.load(std::memory_order_acquire));
}
void NativeMeshCanonicalAccess::require_domain_identity(NativeMeshCanonicalDomain& d,
    NativeMeshEnvironment& environment, NativeRenderActualOwners& owners) {
    if (&d.environment_ != &environment || &d.registry_ != &owners)
        throw std::invalid_argument("GUI ordinary Mesh binding requires SAME captured domain");
}
void NativeMeshCanonicalAccess::require_domain_identity(NativeMeshCanonicalDomain& d,
    NativeMeshEnvironment& environment, NativeRenderActualOwners& owners, NativeMeshConstants c) {
    require_domain_identity(d, environment, owners);
    if (&d.constants_.minimum_00ce4adc != &c.minimum_00ce4adc ||
        &d.constants_.maximum_00ce4970 != &c.maximum_00ce4970 ||
        &d.constants_.lod_00d7a24c != &c.lod_00d7a24c)
        throw std::invalid_argument("Mesh source must borrow the SAME actual constant cells");
}
void NativeMeshCanonicalAccess::retain_source_graph(NativeMeshCanonicalDomain& d) noexcept {
    d.retention_.fail(); // Source failure transport only, no Native ownership fact
}
void NativeMeshCanonicalAccess::bind_block(NativeMeshCanonicalProducerFrame& f,
    NativePostEffectConstructionBlock& block) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    if (f.producer_ != &block || r.lower_.allocation_entered_ || r.block24_ || r.block20_)
        throw std::logic_error("Mesh Block backlink must match its cold actual producer");
    r.block24_ = &block;
}
void NativeMeshCanonicalAccess::bind_block(NativeMeshCanonicalProducerFrame& f,
    NativePostEffect20ConstructionBlock& block) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    if (f.producer_ != &block || r.lower_.allocation_entered_ || r.block24_ || r.block20_)
        throw std::logic_error("Mesh Block backlink must match its cold actual producer");
    r.block20_ = &block;
}
void* NativeMeshCanonicalAccess::allocate_explicit(NativeMeshCanonicalProducerFrame& f) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    try {
        r.pending_custody_.store(true, std::memory_order_release); // pending call, not allocation proof
        auto* const actual = allocate_native_mesh_storage_slot(r.lower_);
        if (!actual) r.pending_custody_.store(false, std::memory_order_release);
        if (actual) r.domain_.registry_.prepare_exact_record(r.exact_, actual);
        r.phase_.store(NativeMeshCanonicalRecord::Phase::allocated, std::memory_order_release);
        r.healthy();
        return actual;
    } catch (...) { r.failed(); throw; }
}
void* NativeMeshCanonicalAccess::allocate_static(NativeMeshCanonicalProducerFrame& f) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    try {
        r.pending_custody_.store(true, std::memory_order_release); // pending call, not allocation proof
        auto* const actual = allocate_native_mesh_slot_00b73b60(r.lower_);
        if (!actual) r.pending_custody_.store(false, std::memory_order_release);
        if (actual) r.domain_.registry_.prepare_exact_record(r.exact_, actual);
        r.phase_.store(NativeMeshCanonicalRecord::Phase::allocated, std::memory_order_release);
        r.healthy();
        return actual;
    } catch (...) { r.failed(); throw; }
}
NativeMeshStorage* NativeMeshCanonicalAccess::construct(NativeMeshCanonicalProducerFrame& f) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    try {
        auto* const payload = construct_native_mesh_storage_00b73d70(r.lower_);
        r.payload_ = payload;
        // Actual creator issuance was captured INSIDE the lower genuine callee-return
        // continuation before any lower/outer Source validation or health fence.
        r.phase_.store(NativeMeshCanonicalRecord::Phase::constructed, std::memory_order_release);
        r.healthy();
        return payload;
    } catch (...) { r.failed(); throw; }
}
NativeMeshReference& NativeMeshCanonicalAccess::publish(NativeMeshCanonicalProducerFrame& f) noexcept {
    // Private callers establish this exact normal nonnull constructor path and
    // reserved node before reaching here. No post-birth validation may fail.
    auto& r = *f.record_;
    r.canonical_ = ::new (static_cast<void*>(&r.reference_storage_))
        NativeMeshReference(*r.payload_, r.domain_.environment_, r);
    r.domain_.registry_.publish_exact_record(r.exact_, *r.canonical_);
    r.phase_.store(NativeMeshCanonicalRecord::Phase::live, std::memory_order_release);
    return *r.canonical_;
}
NativeMeshReference* NativeMeshCanonicalAccess::canonical(NativeMeshCanonicalProducerFrame& f) noexcept {
    return f.record_ ? f.record_->canonical_ : nullptr;
}
bool NativeMeshCanonicalAccess::source_retired(const NativeMeshCanonicalProducerFrame& f) noexcept {
    return f.record_ && f.record_->canonical_zero_returned_.load(std::memory_order_acquire);
}
void NativeMeshCanonicalAccess::release_creator(NativeMeshCanonicalProducerFrame& f) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    if (!r.creator_outstanding_ || !r.canonical_) {
        r.failed();
        throw std::logic_error("Mesh creator release requires its actual unconsumed constructor credit");
    }
    try {
        release_native_render_actual_owner_observed(r.domain_.registry_,
            reinterpret_cast<void*>(r.lower_.actual_), r.creator_release_);
        if (r.creator_release_.decremented) r.creator_outstanding_ = false;
        r.healthy();
    } catch (...) {
        if (r.creator_release_.decremented) r.creator_outstanding_ = false;
        r.failed();
        throw;
    }
}
void NativeMeshCanonicalAccess::release_creator(NativeMeshCanonicalProducerFrame& f,
    long (__stdcall* captured_decrement)(volatile long*)) {
    static_assert(sizeof(long) == 4);
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    const NativeRenderObservedZeroAccess::Invocation outer(r.creator_release_.cpp_zero_frame);
    if (!r.creator_outstanding_ || !r.canonical_ || r.captured_decrement_entered_) {
        r.failed();
        throw std::logic_error("Mesh creator import release requires its exact unconsumed action");
    }
    try {
        // Native caller captured this REAL import once before section release.
        // No fresh IAT read, second count sample, clamp, or std::atomic substitute.
        if (!captured_decrement) throw std::logic_error("missing current CE2220 decrement");
        r.captured_decrement_entered_ = true;
        const auto result = captured_decrement(reinterpret_cast<volatile long*>(r.lower_.count_));
        r.captured_decrement_result_ = result;
        r.captured_decrement_returned_ = true;
        r.creator_outstanding_ = false; // actual decrement returned, any result
        if (result == 0) {
            // Preserve the existing creator helper's two CURRENT profile reads
            // and table0/table1 epochs BEFORE its canonical lookup. The terminal
            // keeps its own existing current-profile validation afterward.
            const auto* const actual = reinterpret_cast<const volatile std::uint32_t*>(r.lower_.actual_);
            const auto* const table = r.domain_.environment_.vtable_00d62d60;
            if (*actual != 0x00d62d60u || !table || table[0] != 0x00bd30e0u)
                throw std::logic_error("unsupported current post-effect creator virtual zero");
            if (*actual != 0x00d62d60u || table[1] != 0x00b74280u)
                throw std::logic_error("unsupported current post-effect creator deleting slot");
            NativeRenderObservedZeroAccess::dispatch(r.domain_.registry_,
                reinterpret_cast<void*>(r.lower_.actual_), r.creator_release_.cpp_zero_frame);
        }
        r.healthy();
    } catch (...) { r.failed(); throw; }
}
void NativeMeshCanonicalAccess::transfer_creator_to_caller(NativeMeshCanonicalProducerFrame& f) noexcept {
    // Private caller is the exact normal creator-return continuation. This
    // transfers C++ custody only, adding no Native retain, count or scalar right.
    f.record_->creator_outstanding_ = false;
}
bool NativeMeshCanonicalAccess::return_unconstructed_static_slot(
    NativeMeshCanonicalProducerFrame& f, void* captured_slot) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    NativeMeshCanonicalRecord::Call call(r);
    auto& low = r.lower_;
    if (low.raw_unwind_selected_) {
        r.failed();
        throw std::logic_error("Mesh state3 selected raw return cannot retry");
    }
    low.raw_unwind_selected_ = true; // consume before getter/return/diagnostics
    if (!low.allocation_returned_ || !low.actual_ ||
        reinterpret_cast<std::uintptr_t>(captured_slot) != low.actual_) {
        r.failed();
        throw std::logic_error("Mesh raw return lost its actual captured state3 allocation");
    }
    if (low.constructor_entered_ || low.constructor_returned_ || low.cpp_payload_live_ || r.canonical_) {
        // In particular, a Source health fence AFTER genuine ctor return is not
        // an original raw-constructor escape. Never free that already-born type.
        r.failed();
        return false;
    }
    try {
        // Exact no-payload reservation end, NOT normal Native death or return.
        // BC index remains live for the genuine pool provider; no old payload
        // borrow or future ctor is permitted after this consumed unwind edge.
        r.domain_.registry_.abandon_exact_unborn_record(r.exact_);
        return_native_mesh_slot_00b72f70(low, captured_slot);
        const bool original_disposed = low.raw_unwind_pool_returned_ &&
            low.raw_unwind_receiver_ == low.receiver_;
        // This catch-path record remains poisoned/retained; no fake scalar or
        // canonical return fact, no count repair, no cleanup retry/collection.
        r.domain_.registry_.retire_exact_record(r.exact_);
        r.failed();
        return original_disposed;
    } catch (...) { r.failed(); throw; }
}
void NativeMeshCanonicalAccess::preflight_cancel_without_native_payload(NativeMeshCanonicalProducerFrame& f, ReadinessRejection rejection) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    preflight_payload_receiver(r,false, rejection);
    // Only genuine never-entered or actual normal-NULL history, no guessed count.
    if (r.lower_.actual_ || (r.lower_.allocation_entered_ && !r.lower_.allocation_returned_) ||
        r.canonical_ || r.invocation_.load(std::memory_order_acquire) ||
        r.exact_.outer_calls_.load(std::memory_order_acquire) != 0u ||
        r.exact_.borrows_.load(std::memory_order_acquire) != 0u) {
        if (rejection == ReadinessRejection::sticky) r.failed();
        throw std::logic_error("Mesh cold cancel cannot discard entered/unknown Native custody");
    }
}
void NativeMeshCanonicalAccess::cancel_without_native_payload(NativeMeshCanonicalProducerFrame& f) {
    preflight_cancel_without_native_payload(f);
    auto& r = NativeMeshCanonicalRecord::get(f);
    r.domain_.registry_.cancel_exact_record_without_payload(r.exact_);
    r.cold_cancelled_ = true;
    r.pending_custody_.store(false, std::memory_order_release);
    r.producer_link_closed_ = true;
    r.producer_frame_ = nullptr;
    r.block24_ = nullptr;
    r.block20_ = nullptr;
    f.record_ = nullptr;
}
void NativeMeshCanonicalAccess::preflight_producer_link_at_quiescence(NativeMeshCanonicalProducerFrame& f, ReadinessRejection rejection) {
    auto& r = NativeMeshCanonicalRecord::get(f);
    preflight_payload_receiver(r,true, rejection);
    if (!r.canonical_zero_returned_.load(std::memory_order_acquire) || !r.lower_.pool_returned_ ||
        r.creator_outstanding_ || r.poisoned_.load(std::memory_order_acquire) ||
        r.invocation_.load(std::memory_order_acquire) ||
        r.exact_.borrows_.load(std::memory_order_acquire) != 0u) {
        if (rejection == ReadinessRejection::sticky) r.failed();
        throw std::logic_error("Failed Mesh producer cannot pass a normal close fence");
    }
    // end_frame must not be used to discover a live caller or discharge an
    // unrelated active-use contribution. The one linked hold is metadata,
    // not a Native count sample or an independent quiescence proof.
    auto* const caller=r.observed_zero_hold_.frame_;
    const unsigned retained_hold=caller?1u:0u;
    if ((caller && caller->active_invocations_.load(std::memory_order_acquire)!=0u) ||
        r.exact_.outer_calls_.load(std::memory_order_acquire)!=retained_hold) {
        if (rejection == ReadinessRejection::sticky) r.failed();
        throw std::logic_error("Mesh preflight requires only its inactive retained zero hold");
    }
}
void NativeMeshCanonicalAccess::close_producer_link_at_quiescence(NativeMeshCanonicalProducerFrame& f) {
    preflight_producer_link_at_quiescence(f);
    auto& r = NativeMeshCanonicalRecord::get(f);
    drain_payload_receiver(r);
    drain_observed_outer_at_quiescence(r);
    if (!r.canonical_zero_returned_.load(std::memory_order_acquire) || !r.lower_.pool_returned_ || r.creator_outstanding_ ||
        r.invocation_.load(std::memory_order_acquire) ||
        r.exact_.outer_calls_.load(std::memory_order_acquire) != 0u ||
        r.exact_.borrows_.load(std::memory_order_acquire) != 0u ||
        r.poisoned_.load(std::memory_order_acquire)) {
        r.failed();
        throw std::logic_error("Mesh producer close requires its actual complete physical disposition");
    }
    // Caller has completed its outer uses/fences. No callback later uses Block.
    r.pending_custody_.store(false, std::memory_order_release);
    r.producer_link_closed_ = true;
    r.producer_frame_ = nullptr;
    r.block24_ = nullptr;
    r.block20_ = nullptr;
    f.record_ = nullptr;
}
void NativeMeshCanonicalAccess::record_source_failure(NativeMeshCanonicalProducerFrame& f) noexcept {
    if (f.record_) f.record_->failed();
}
void NativeMeshCanonicalAccess::poison(NativeMeshCanonicalRecord& r) noexcept { r.failed(); }
void NativeMeshCanonicalAccess::native_constructor_returned(NativeMeshCanonicalRecord& r,
    NativeMeshStorage* actual_result) noexcept {
    r.payload_ = actual_result; // actual real callee return, before Source fences
    r.creator_outstanding_ = actual_result != nullptr;
}
void NativeMeshCanonicalAccess::native_payload_ended(NativeMeshCanonicalRecord& r) noexcept {
    // Only low Frame's genuine dtor-normal/sole late flags/no-store typed end.
    r.payload_ = nullptr;
    r.domain_.registry_.deactivate_exact_record_from_native_end(r.exact_);
    r.phase_.store(NativeMeshCanonicalRecord::Phase::dead, std::memory_order_release);
}
void NativeMeshCanonicalAccess::require_cpp_count_access(NativeMeshReference& reference) {
    auto* const r = reference.raw_record_;
    if (!r) return;
    if (r->poisoned_.load(std::memory_order_acquire) ||
        r->exact_.normal_native_dead_.load(std::memory_order_acquire) ||
        r->exact_.availability_.load(std::memory_order_acquire) !=
            NativeRenderActualOwnerRegistry::ExactRecord::Availability::live) {
        r->failed();
        throw std::logic_error("Old Mesh canonical cannot access a replacement's actual +4");
    }
}
void NativeMeshCanonicalAccess::hold_observed_zero(NativeMeshReference& reference,
    NativeRenderObservedZeroFrame& outer) {
    require_cpp_count_access(reference); // private metadata only, AFTER real zero
    auto& r=*reference.raw_record_;
    auto& hold=r.observed_zero_hold_;
    if(hold.claimed_.exchange(true,std::memory_order_acq_rel)) {
        r.mark_failed_metadata();
        throw std::logic_error("same Mesh incarnation cannot replay its outer zero hold");
    }
    r.exact_.outer_calls_.fetch_add(1,std::memory_order_acq_rel);
    hold.mesh_=&r;
    NativeRenderObservedZeroAccess::attach_hold(outer,hold);
}
void NativeMeshCanonicalAccess::fail_observed_outer(NativeMeshCanonicalRecord& r) noexcept {
    r.mark_failed_metadata();
}
void NativeMeshCanonicalAccess::release_observed_outer(NativeMeshCanonicalRecord& r) noexcept {
    r.exact_.outer_calls_.fetch_sub(1,std::memory_order_release);
}
void NativeMeshCanonicalAccess::preflight_observed_outer_at_quiescence(NativeMeshCanonicalRecord& r) {
    auto* const caller=r.observed_zero_hold_.frame_;
    const unsigned retained_hold=caller?1u:0u;
    if ((caller && caller->active_invocations_.load(std::memory_order_acquire)!=0u) ||
        r.exact_.outer_calls_.load(std::memory_order_acquire)!=retained_hold) {
        r.domain_.retention_.fail();
        r.failed();
        throw std::logic_error("Mesh domain preflight requires only its inactive retained zero hold");
    }
}
void NativeMeshCanonicalAccess::drain_observed_outer_at_quiescence(NativeMeshCanonicalRecord& r) {
    if(auto* frame=r.observed_zero_hold_.frame_) {
        if(!NativeRenderObservedZeroAccess::end_frame(*frame,false))
            throw std::logic_error("Mesh caller frame still has an actual active source invocation");
    }
}
void NativeMeshCanonicalAccess::arm_observed_zero(NativeMeshReference& reference) noexcept {
    auto* const r = reference.raw_record_;
    if (r && r->zero_action_.exchange(true, std::memory_order_acq_rel)) r->failed();
}
void NativeMeshCanonicalAccess::release_zero(NativeMeshReference& reference) {
    auto& r = *reference.raw_record_;
    NativeMeshCanonicalRecord::Call call(r, false);
    if (!r.zero_action_.exchange(false, std::memory_order_acq_rel) ||
        r.scalar_entered_.exchange(true, std::memory_order_acq_rel)) {
        r.failed();
        throw std::logic_error("Raw Mesh zero requires its genuine once-only observed-zero action");
    }
    bool owns_receiver=false;
    try {
        require_cpp_count_access(reference); // metadata before ANY ended-payload access
        auto* const table = r.domain_.environment_.vtable_00d62d60;
        if (!table || reference.storage_.native_vtable_00 != 0x00d62d60u ||
            table[0] != 0x00bd30e0u || table[1] != 0x00b74280u)
            throw std::logic_error("Raw Mesh zero requires its current actual deleting profile");
        begin_payload_receiver(r);owns_receiver=true;
        r.scalar_flags_ = 1u; // real deleting-virtual argument epoch, not birth authority
        (void)delete_native_mesh_storage_00b74280(r.scalar_flags_, r.lower_);
        r.source_scalar_returned_ = true; // actual lower normal return AFTER health fence
        r.domain_.registry_.retire_exact_record(r.exact_); // may throw; no Native rollback
        r.phase_.store(NativeMeshCanonicalRecord::Phase::retired, std::memory_order_release);
        // Concrete notices only; no optional reset/record erase/self disposal.
        if (r.block24_) r.block24_->mesh_source_retired(*r.producer_frame_);
        if (r.block20_) r.block20_->mesh_source_retired(*r.producer_frame_);
        r.healthy();
        r.canonical_zero_returned_.store(true, std::memory_order_release);
        end_payload_receiver(r,false);owns_receiver=false;
        r.healthy();
    } catch (...) {
        r.failed();
        if(owns_receiver)end_payload_receiver(r,true);
        throw;
    }
}
NativeMeshStorage& NativeMeshCanonicalAccess::reject_unscoped_storage(NativeMeshReference& reference) {
    reference.raw_record_->failed();
    throw std::logic_error("Raw Mesh storage requires a whole-use canonical borrow");
}
NativeMeshCanonicalBorrow::NativeMeshCanonicalBorrow(NativeMeshReference& reference)
    : reference_(reference), record_(reference.raw_record_) {
    if (!record_) return; // exact old legacy caller path, no new Native read
    uncaught_on_entry_ = std::uncaught_exceptions(); // genuine chosen Source CRT query only
    NativeMeshCanonicalAccess::require_cpp_count_access(reference_);
    record_->exact_.borrows_.fetch_add(1, std::memory_order_acq_rel);
    try { NativeMeshCanonicalAccess::require_cpp_count_access(reference_); }
    catch (...) {
        record_->exact_.borrows_.fetch_sub(1, std::memory_order_release);
        throw;
    }
}
NativeMeshCanonicalBorrow::NativeMeshCanonicalBorrow(NativeMeshCanonicalBorrow&& other) noexcept
    : reference_(other.reference_), record_(other.record_), active_(other.active_),
      uncaught_on_entry_(other.uncaught_on_entry_) {
    other.record_ = nullptr;
    other.active_ = false;
}
NativeMeshCanonicalBorrow::~NativeMeshCanonicalBorrow() noexcept {
    if (!record_) return;
    if (std::uncaught_exceptions() > uncaught_on_entry_)
        record_->mark_failed_metadata(); // pure metadata; no exception_ptr allocation/callback
    record_->exact_.borrows_.fetch_sub(1, std::memory_order_release);
}
NativeMeshStorage& NativeMeshCanonicalAccess::scoped_storage(NativeMeshReference& reference,
    NativeMeshCanonicalBorrow& borrow) {
    if (&borrow.reference_ != &reference || borrow.record_ != reference.raw_record_)
        throw std::logic_error("Mesh whole-use borrow belongs to a different canonical");
    require_cpp_count_access(reference);
    return reference.storage_;
}
NativeMeshStorage& NativeMeshCanonicalBorrow::storage() {
    if (!active_) throw std::logic_error("Moved Mesh C++ pin has no remaining use");
    if (!record_) return reference_.storage_;
    return NativeMeshCanonicalAccess::scoped_storage(reference_, *this);
}
} // namespace bsp
