#include "bsp/native_post_effect_parent_lifetime.hpp"
#include "bsp/native_post_effect_construction.hpp"
#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_post_effect_ordinary_lifetime.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstring>
#include <mutex>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Ordinary parent lifetime requires the existing MSVC Win32 providers.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Counter = std::atomic<std::int32_t>;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4 && sizeof(Counter) == 4);
static_assert(alignof(Counter) == 4 && std::is_trivially_destructible_v<Counter>);
enum class Kind { parent20, parent24 };
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
std::uint8_t late_byte(const void* actual_home) noexcept {
    std::uint8_t result;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::memcpy(&result, actual_home, 1);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return result;
}
} // namespace

struct NativePostEffectParentDomain::Impl {
    NativePostEffect20ConstructionContext& c20;
    NativePostEffectConstructionContext& c24;
    const NativePostEffectOrdinaryLifetimeBinding& binding;
    NativeRenderActualOwnerRegistry& owners;
    NativeMeshRetentionState& retention;
    std::mutex mutex;
    NativePostEffectParentFrame* first{};
    std::atomic<unsigned> retained{0};
    std::atomic<bool> failed{false}, closing{false};
    Impl(NativePostEffect20ConstructionContext& a, NativePostEffectConstructionContext& b,
        const NativePostEffectOrdinaryLifetimeBinding& c, NativeRenderActualOwnerRegistry& d,
        NativeMeshRetentionState& e) : c20(a), c24(b), binding(c), owners(d), retention(e) {}
};

struct NativePostEffectParentFrame::Impl {
    NativePostEffectParentDomain& domain;
    const Kind kind;
    NativePostEffect20ConstructionBlock* block20{};
    NativePostEffectConstructionBlock* block24{};
    NativePostEffectParentFrame* next{};
    void* actual{};
    Counter* count_home{};
    RenderCommandReference* reference{};
    NativePostEffect20Owner* owner20{};
    NativePostEffectOwner* owner24{};
    using Owner20Cell = std::aligned_storage_t<sizeof(NativePostEffect20Owner), alignof(NativePostEffect20Owner)>;
    using Owner24Cell = std::aligned_storage_t<sizeof(NativePostEffectOwner), alignof(NativePostEffectOwner)>;
    using Ref20Cell = std::aligned_storage_t<sizeof(NativePostEffect20Reference), alignof(NativePostEffect20Reference)>;
    using Ref24Cell = std::aligned_storage_t<sizeof(NativePostEffectReference), alignof(NativePostEffectReference)>;
    Owner20Cell owner20_cell;
    Owner24Cell owner24_cell;
    Ref20Cell ref20_cell;
    Ref24Cell ref24_cell;
    bool owner_live{}, reference_live{}, attached{}, counted{}, block_linked{};
    std::atomic<unsigned> outer_calls{0};
    std::atomic<bool> failed{false}, closed{false}, producer_entered{false};
    std::atomic<bool> constructor_outer{false}, constructor_entered{false};
    std::atomic<bool> allocator_entered{false}, allocator_returned{false};
    std::atomic<bool> atomic_born{false}, native_constructor_returned{false};
    std::atomic<bool> source_constructor_returned{false}, published{false};
    std::atomic<bool> creator_outstanding{false}, creator_release_entered{false};
    std::atomic<bool> zero_armed{false}, scalar_entered{false}, typed_ended{false};
    std::atomic<bool> flags_read{false}, free_entered{false}, free_returned{false};
    std::atomic<bool> payload_returned{false}, scalar_body_completed{false}, scalar_source_returned{false};
    std::atomic<bool> retired{false}, cold_cancelled{false};
    std::uint8_t captured_flags{};
    Word virtual_flags_home{};
    Impl(NativePostEffectParentDomain& d, NativePostEffect20ConstructionBlock& b)
        : domain(d), kind(Kind::parent20), block20(&b) {}
    Impl(NativePostEffectParentDomain& d, NativePostEffectConstructionBlock& b)
        : domain(d), kind(Kind::parent24), block24(&b) {}
};

NativePostEffectParentDomain::NativePostEffectParentDomain(
    NativePostEffect20ConstructionContext& c20, NativePostEffectConstructionContext& c24,
    const NativePostEffectOrdinaryLifetimeBinding& binding,
    NativeRenderActualOwnerRegistry& owners, NativeMeshRetentionState& retention)
    : impl_(std::make_unique<Impl>(c20,c24,binding,owners,retention)) {
    require(&c20.destruction.actual_owners == static_cast<NativeRenderActualOwners*>(&owners) &&
        &c24.destruction.actual_owners == static_cast<NativeRenderActualOwners*>(&owners) &&
        c20.destruction.ordinary_models == &binding.models &&
        c24.destruction.ordinary_models == &binding.models,
        "Parent domain requires the SAME actual registry and immutable ordinary Model binding");
}
NativePostEffectParentDomain::~NativePostEffectParentDomain() {
    // Real owning callers must deny destruction until the genuine close fence.
    // This destructor invents neither Native cleanup, a refund, nor termination.
    auto* frame = impl_->first;
    while (frame) { auto* next = frame->impl_->next; delete frame; frame = next; }
}
void NativePostEffectParentDomain::require_owned(bool value, const char* message) const {
    if (!value) {
        NativePostEffectParentFrame::record_domain_failure(
            *const_cast<NativePostEffectParentDomain*>(this));
        throw std::logic_error(message);
    }
}
NativePostEffectParentFrame::NativePostEffectParentFrame(NativePostEffectParentDomain& d,
    NativePostEffect20ConstructionBlock& b)
    : impl_(std::make_unique<Impl>(d,b)), exact_(d.impl_->owners,this) {}
NativePostEffectParentFrame::NativePostEffectParentFrame(NativePostEffectParentDomain& d,
    NativePostEffectConstructionBlock& b)
    : impl_(std::make_unique<Impl>(d,b)), exact_(d.impl_->owners,this) {}
NativePostEffectParentFrame::~NativePostEffectParentFrame() {
    // These companions are separate C++ metadata, never aliases of the20/24h
    // Native fields. No Native pointer/count/profile is followed here.
    if (impl_->reference_live) {
        if (impl_->kind == Kind::parent20)
            std::destroy_at(static_cast<NativePostEffect20Reference*>(impl_->reference));
        else std::destroy_at(static_cast<NativePostEffectReference*>(impl_->reference));
    }
    if (impl_->owner_live) {
        if (impl_->kind == Kind::parent20) std::destroy_at(impl_->owner20);
        else std::destroy_at(impl_->owner24);
    }
}
void NativePostEffectParentFrame::record_domain_failure(NativePostEffectParentDomain& d) noexcept {
    d.impl_->failed.store(true,std::memory_order_release);
    d.impl_->retention.fail();
}
void NativePostEffectParentFrame::record_source_failure() noexcept {
    impl_->failed.store(true,std::memory_order_release);
    record_domain_failure(impl_->domain);
    if (impl_->attached) impl_->domain.impl_->owners.poison_exact_record(exact_);
}
void NativePostEffectParentFrame::require_owned(bool value, const char* message) const {
    if (!value) {
        const_cast<NativePostEffectParentFrame*>(this)->record_source_failure();
        throw std::logic_error(message);
    }
}
void NativePostEffectParentFrame::require_healthy() const {
    if (impl_->failed.load(std::memory_order_acquire) ||
        impl_->domain.impl_->failed.load(std::memory_order_acquire) || impl_->closed.load()) {
        const_cast<NativePostEffectParentFrame*>(this)->record_source_failure();
        throw std::logic_error("Parent record retains failed/closed Source custody");
    }
}
bool NativePostEffectParentDomain::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire);
}
bool NativePostEffectParentDomain::has_retained_records() const noexcept {
    return impl_->retained.load(std::memory_order_acquire) != 0;
}
bool NativePostEffectParentFrame::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire);
}

NativePostEffectParentFrame& NativePostEffectParentDomain::prepare20(
    NativePostEffect20ConstructionBlock& block) {
    try {
        require_owned(!impl_->closing.load() && !impl_->failed.load(),"Parent domain cannot prepare after close/failure");
        if (block.ordinary_lifetime_ != &impl_->binding) block.prepare(impl_->c20,impl_->binding);
        require_owned(block.context_ == &impl_->c20 && block.phase_ == NativePostEffect20ConstructionBlock::Phase::prepared && !block.parent_frame_,"Parent20 requires its exact cold Block");
        auto pending = std::unique_ptr<NativePostEffectParentFrame>(new NativePostEffectParentFrame(*this,block));
        auto* frame = pending.get();
        bool linked = false;
        { std::lock_guard<std::mutex> lock(impl_->mutex);
          if (!impl_->closing.load()) {
              frame->impl_->next = impl_->first; impl_->first = frame;
              (void)pending.release(); linked = true;
          } }
        require_owned(linked,"Parent prepare crossed its real close fence"); // diagnostics outside mutex
        frame->prepare_after_link();
        return *frame;
    } catch (...) { NativePostEffectParentFrame::record_domain_failure(*this); throw; }
}
NativePostEffectParentFrame& NativePostEffectParentDomain::prepare24(
    NativePostEffectConstructionBlock& block) {
    try {
        require_owned(!impl_->closing.load() && !impl_->failed.load(),"Parent domain cannot prepare after close/failure");
        if (block.ordinary_lifetime_ != &impl_->binding) block.prepare(impl_->c24,impl_->binding);
        require_owned(block.context_ == &impl_->c24 && block.phase_ == NativePostEffectConstructionBlock::Phase::prepared && !block.parent_frame_,"Parent24 requires its exact cold Block");
        auto pending = std::unique_ptr<NativePostEffectParentFrame>(new NativePostEffectParentFrame(*this,block));
        auto* frame = pending.get();
        bool linked = false;
        { std::lock_guard<std::mutex> lock(impl_->mutex);
          if (!impl_->closing.load()) {
              frame->impl_->next = impl_->first; impl_->first = frame;
              (void)pending.release(); linked = true;
          } }
        require_owned(linked,"Parent prepare crossed its real close fence"); // diagnostics outside mutex
        frame->prepare_after_link();
        return *frame;
    } catch (...) { NativePostEffectParentFrame::record_domain_failure(*this); throw; }
}
void NativePostEffectParentFrame::prepare_after_link() {
    try {
        auto& d = *impl_->domain.impl_;
        // Root's shared header supplies only private metadata friendship.
        require_owned(&d.binding.meshes.retention_ == &d.retention,
            "Parent and child Mesh records require the SAME retained graph receiver");
        d.owners.attach_exact_record(exact_);
        impl_->attached = true;
        d.retention.retained_.fetch_add(1,std::memory_order_acq_rel);
        d.retained.fetch_add(1,std::memory_order_acq_rel);
        impl_->counted = true;
        if (impl_->block20) impl_->block20->parent_frame_ = this;
        else impl_->block24->parent_frame_ = this;
        impl_->block_linked = true;
    } catch (...) { record_source_failure(); throw; }
}
void NativePostEffectParentFrame::begin_outer_call() {
    require_healthy();
    require_owned(!impl_->domain.impl_->closing.load(),"Parent operation crossed its actual graph-close fence");
    exact_.outer_calls_.fetch_add(1,std::memory_order_acq_rel);
    impl_->outer_calls.fetch_add(1,std::memory_order_acq_rel);
}
void NativePostEffectParentFrame::end_outer_call() noexcept {
    impl_->outer_calls.fetch_sub(1,std::memory_order_release);
    exact_.outer_calls_.fetch_sub(1,std::memory_order_release);
}
void* NativePostEffectParentFrame::allocate_parent() {
    require_healthy();
    require_owned(!impl_->producer_entered.exchange(true),"Parent allocation/constructor cannot replay");
    impl_->allocator_entered.store(true,std::memory_order_release);
    const std::size_t bytes = impl_->kind == Kind::parent20 ? 0x20u : 0x24u;
    void* actual = singleton_lifetime_allocate(
        SingletonAllocationRequest{SingletonAllocationKind::object,bytes,bytes});
    impl_->actual = actual;
    impl_->allocator_returned.store(true,std::memory_order_release);
    if (actual) impl_->domain.impl_->owners.prepare_exact_record(exact_,actual);
    require_healthy();
    return actual;
}
void NativePostEffectParentFrame::cancel_normal_null_allocation() {
    require_owned(impl_->allocator_returned.load() && !impl_->actual && !impl_->constructor_entered.load() &&
        !impl_->atomic_born.load() && !impl_->published.load(),"Parent cold cancel requires actual normal NULL");
    // Real child cold cancellation; no Native parent/header/count is touched.
    if (impl_->block20) impl_->block20->cancel_ordinary_preparation();
    else impl_->block24->cancel_ordinary_preparation();
    // Exact record/Block link stay retained through the real outer return.
    // Registry cold detach requires zero outer calls and occurs only at close.
    impl_->cold_cancelled.store(true,std::memory_order_release);
}
void NativePostEffectParentFrame::native_atomic_born(void* same_actual) noexcept {
    if (!impl_->allocator_returned.load() || !impl_->constructor_entered.load() ||
        !same_actual || same_actual != impl_->actual || impl_->atomic_born.exchange(true)) {
        record_source_failure(); return;
    }
    // Existing exact-size malloc supplies opaque byte backing. Only +4 has a
    // C++ typed payload birth here, performed by the existing ctor, not us.
    impl_->count_home = std::launder(reinterpret_cast<Counter*>(
        reinterpret_cast<Word>(same_actual) + 4u));
}
void NativePostEffectParentFrame::native_constructor_returned(void* same_actual) noexcept {
    if (same_actual != impl_->actual || !impl_->atomic_born.load() ||
        impl_->native_constructor_returned.exchange(true)) { record_source_failure(); return; }
    impl_->creator_outstanding.store(true,std::memory_order_release);
}
bool NativePostEffectParentFrame::native_constructor_has_returned() const noexcept {
    return impl_->native_constructor_returned.load(std::memory_order_acquire);
}
void NativePostEffectParentFrame::publish_after_native_constructor() {
    require_healthy();
    require_owned(native_constructor_has_returned() && !impl_->published.load(),"Parent publication requires its actual normal body return");
    if (impl_->kind == Kind::parent20) {
        impl_->owner20 = ::new(&impl_->owner20_cell) NativePostEffect20Owner(
            impl_->actual,impl_->domain.impl_->c20.destruction,*this);
        impl_->owner_live = true;
        auto* ref = ::new(&impl_->ref20_cell) NativePostEffect20Reference(*impl_->owner20,*this);
        impl_->reference = ref;
    } else {
        impl_->owner24 = ::new(&impl_->owner24_cell) NativePostEffectOwner(
            impl_->actual,impl_->domain.impl_->c24.destruction,*this);
        impl_->owner_live = true;
        auto* ref = ::new(&impl_->ref24_cell) NativePostEffectReference(*impl_->owner24,*this);
        impl_->reference = ref;
    }
    impl_->reference_live = true;
    impl_->reference->bind_ordinary_parent(*this);
    impl_->domain.impl_->owners.publish_exact_record(exact_,*impl_->reference);
    impl_->published.store(true,std::memory_order_release);
}
RenderCommandReference& NativePostEffectParentFrame::canonical_after_publication() {
    require_healthy();
    require_owned(impl_->published.load() && impl_->reference,"Parent lost its genuine published canonical");
    return *impl_->reference;
}
NativePostEffect20Owner& NativePostEffectParentFrame::owner20_after_publication() {
    (void)canonical_after_publication(); require_owned(impl_->owner20 != nullptr,"Parent20 owner family mismatch"); return *impl_->owner20;
}
NativePostEffectOwner& NativePostEffectParentFrame::owner24_after_publication() {
    (void)canonical_after_publication(); require_owned(impl_->owner24 != nullptr,"Parent24 owner family mismatch"); return *impl_->owner24;
}
void NativePostEffectParentFrame::source_constructor_returned() {
    require_healthy(); require_owned(native_constructor_has_returned() && impl_->published.load(),"Parent Source return lacks real Native return/publication");
    impl_->source_constructor_returned.store(true,std::memory_order_release);
}

void* NativePostEffectParentFrame::construct20(NativeString& name, Word vertices, const void* optional) {
    require_owned(impl_->kind == Kind::parent20,"Parent20 producer family mismatch");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call(); impl_->constructor_outer.store(true,std::memory_order_release);
    try {
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect20_00b4e470(actual,0x20u,name,vertices,optional,*impl_->block20);
            source_constructor_returned();
        }
        impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); return actual;
    } catch (...) { record_source_failure(); impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); throw; }
}
void* NativePostEffectParentFrame::construct20(const NativePostEffect20ArgumentView& arguments) {
    require_owned(impl_->kind == Kind::parent20,"Parent20 producer family mismatch");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call(); impl_->constructor_outer.store(true,std::memory_order_release);
    try {
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect20_00b4e470(actual,0x20u,arguments,*impl_->block20);
            source_constructor_returned();
        }
        impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); return actual;
    } catch (...) { record_source_failure(); impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); throw; }
}
void* NativePostEffectParentFrame::construct24(NativeString& name, Word primitives, const void* optional) {
    require_owned(impl_->kind == Kind::parent24,"Parent24 producer family mismatch");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call(); impl_->constructor_outer.store(true,std::memory_order_release);
    try {
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect_00b4e840(actual,0x24u,name,primitives,optional,*impl_->block24);
            source_constructor_returned();
        }
        impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); return actual;
    } catch (...) { record_source_failure(); impl_->constructor_outer.store(false,std::memory_order_release); end_outer_call(); throw; }
}
void NativePostEffectParentFrame::require_same_canonical(RenderCommandReference& reference) const {
    require_healthy();
    require_owned(!impl_->domain.impl_->closing.load() &&
        impl_->published.load() && impl_->source_constructor_returned.load() &&
        impl_->reference == &reference && reference.ordinary_parent_ == this &&
        !impl_->typed_ended.load() && !impl_->scalar_entered.load() &&
        static_cast<void*>(&reference.reference_count) == impl_->count_home,
        "Parent count access requires SAME born published live canonical");
}
void NativePostEffectParentFrame::begin_cpp_count_use(RenderCommandReference& reference) {
    require_same_canonical(reference);
    exact_.borrows_.fetch_add(1,std::memory_order_acq_rel);
    try { require_same_canonical(reference); }
    catch (...) { end_cpp_count_use(); record_source_failure(); throw; }
}
void NativePostEffectParentFrame::end_cpp_count_use() noexcept { exact_.borrows_.fetch_sub(1,std::memory_order_release); }
void NativePostEffectParentFrame::hold_observed_zero(NativeRenderObservedZeroFrame& caller,
    RenderCommandReference& reference) {
    begin_outer_call();
    try {
        require_same_canonical(reference);
        bool expected = false;
        require_owned(observed_zero_hold_.claimed_.compare_exchange_strong(expected,true,std::memory_order_acq_rel),
            "Parent observed-zero hold cannot replay");
        observed_zero_hold_.parent_ = this;
        NativeRenderObservedZeroAccess::attach_hold(caller,observed_zero_hold_);
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
}
void NativePostEffectParentFrame::release_observed_zero_hold(bool source_failed) noexcept {
    if (source_failed) record_source_failure();
    end_outer_call();
}
void NativePostEffectParentFrame::invoke_observed_zero(void* actual, RenderCommandReference& reference) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call();
    try {
        require_same_canonical(reference);
        require_owned(actual == impl_->actual && !impl_->zero_armed.exchange(true),"Parent observed-zero actual/action mismatch");
        reference.release_zero_references();
        require_healthy(); end_outer_call();
    } catch (...) { impl_->zero_armed.store(false); record_source_failure(); end_outer_call(); throw; }
}
void NativePostEffectParentFrame::ordinary_virtual_zero(RenderCommandReference& reference) {
    require_same_canonical(reference);
    require_owned(impl_->zero_armed.exchange(false),"Parent virtual zero lacks its real observed-zero action");
    if (impl_->kind == Kind::parent20)
        static_cast<NativePostEffect20Reference&>(reference).validate_parent_target();
    else static_cast<NativePostEffectReference&>(reference).validate_parent_target();
    // The genuine deleting virtual supplies1 at this argument epoch only.
    impl_->virtual_flags_home = 1;
    (void)scalar(&impl_->virtual_flags_home);
}
void NativePostEffectParentFrame::end_cpp_parent_payload() noexcept {
    // Other Native parent members are opaque raw byte/assembly operands, not a
    // cast whole typed struct. End ONLY the actual constructor-born +4 atomic.
    std::destroy_at(impl_->count_home);
    impl_->typed_ended.store(true,std::memory_order_release);
    impl_->domain.impl_->owners.deactivate_exact_record_from_native_end(exact_);
}
void NativePostEffectParentFrame::notify_block_retired() noexcept {
    if (impl_->block20) impl_->block20->parent_source_retired(*this,*impl_->reference);
    else impl_->block24->parent_source_retired(*this,*impl_->reference);
}
void* NativePostEffectParentFrame::scalar(const void* flags_home) {
    require_healthy(); require_owned(flags_home && impl_->source_constructor_returned.load() &&
        impl_->published.load() && !impl_->scalar_entered.exchange(true),"Parent scalar requires its genuine one-shot caller action");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call();
    const Word original = reinterpret_cast<Word>(impl_->actual);
    try {
        // Direct action entry is not a successful creator/death disposition.
        // Keep any outstanding creator debt on a partial payload escape.
        if (impl_->kind == Kind::parent20) destroy_native_post_effect_owner_00b4e1f0(*impl_->owner20);
        else destroy_native_post_effect_owner_00b4e2f0(*impl_->owner24);
        impl_->payload_returned.store(true,std::memory_order_release);
        // The valid direct action has now actually ended the whole payload;
        // a real creator decrement, when used, was recorded separately.
        impl_->creator_outstanding.store(false,std::memory_order_release);
        const std::uint8_t flags = late_byte(flags_home);
        impl_->captured_flags = flags; impl_->flags_read.store(true,std::memory_order_release);
        end_cpp_parent_payload(); // no Native store, before actual conditional free
        if (flags & 1u) {
            impl_->free_entered.store(true,std::memory_order_release);
            singleton_lifetime_free(reinterpret_cast<void*>(original));
            impl_->free_returned.store(true,std::memory_order_release);
        }
        impl_->scalar_body_completed.store(true,std::memory_order_release);
        impl_->domain.impl_->owners.retire_exact_record(exact_);
        impl_->retired.store(true,std::memory_order_release);
        notify_block_retired();
        require_healthy();
        impl_->scalar_source_returned.store(true,std::memory_order_release);
        end_outer_call(); return reinterpret_cast<void*>(original);
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
}
void NativePostEffectParentFrame::release_creator(long (__stdcall* decrement)(volatile long*)) {
    require_healthy(); require_owned(decrement && impl_->source_constructor_returned.load() &&
        !impl_->scalar_entered.load() && !impl_->typed_ended.load() && !impl_->retired.load() &&
        impl_->creator_outstanding.load() && !impl_->creator_release_entered.exchange(true),
        "Parent creator release requires its real unconsumed constructor credit/import");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call();
    try {
        const long result = decrement(reinterpret_cast<volatile long*>(impl_->count_home));
        impl_->creator_outstanding.store(false,std::memory_order_release);
        if (result == 0) NativeRenderObservedZeroAccess::dispatch(
            impl_->domain.impl_->owners,impl_->actual,terminal_zero_frame_);
        require_healthy(); end_outer_call();
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
}
void NativePostEffectParentFrame::close_after_host_quiescence() {
    require_healthy();
    auto* caller = observed_zero_hold_.frame_;
    const unsigned retained_hold = caller ? 1u : 0u;
    // A concrete owner must first establish actual whole-graph/worker/last-use
    // quiescence. These checks reject misuse; counters do not issue that fact.
    // Validate BOTH receiver frames and all non-hold facts before ANY unlink.
    require_owned(!impl_->constructor_outer.load() &&
        terminal_zero_frame_.active_invocations_.load(std::memory_order_acquire)==0 &&
        (!caller || caller->active_invocations_.load(std::memory_order_acquire)==0) &&
        impl_->outer_calls.load()==retained_hold && exact_.outer_calls_.load()==retained_hold &&
        exact_.borrows_.load()==0,
        "Parent close crossed an active operation/count use/caller invocation");
    const bool never_entered = !impl_->producer_entered.load() && !impl_->allocator_entered.load() &&
        !impl_->constructor_entered.load() && !impl_->atomic_born.load() && !impl_->published.load();
    require_owned(never_entered || impl_->cold_cancelled.load() ||
        (impl_->scalar_source_returned.load() && impl_->payload_returned.load() &&
         impl_->typed_ended.load() && impl_->free_returned.load() && !impl_->creator_outstanding.load()),
        "Parent close requires actual complete caller/physical facts, not flags0/partial/unknown");
    try {
        if (never_entered) {
            // Authentic cold owner fence: no allocator or constructor entered.
            if (impl_->block20) impl_->block20->cancel_ordinary_preparation();
            else impl_->block24->cancel_ordinary_preparation();
            impl_->cold_cancelled.store(true,std::memory_order_release);
        }
        if (caller)
            require_owned(NativeRenderObservedZeroAccess::end_frame(*caller,false),
                "Parent caller still has an active invocation");
        if (caller != &terminal_zero_frame_)
            require_owned(NativeRenderObservedZeroAccess::end_frame(terminal_zero_frame_,false),
                "Parent terminal caller still has an active invocation");
        require_healthy();
        require_owned(impl_->outer_calls.load()==0 && exact_.outer_calls_.load()==0 && exact_.borrows_.load()==0,
            "Parent close retains an actual CPP caller/count user");
        if (impl_->cold_cancelled.load()) {
            impl_->domain.impl_->owners.cancel_exact_record_without_payload(exact_);
        } else {
            if (impl_->block20) impl_->block20->close_ordinary_after_host_quiescence();
            else impl_->block24->close_ordinary_after_host_quiescence();
            impl_->domain.impl_->owners.close_exact_record_at_quiescence(exact_);
        }
        impl_->attached = false;
        if (impl_->block20) impl_->block20->parent_frame_ = nullptr;
        else impl_->block24->parent_frame_ = nullptr;
        impl_->block_linked = false;
        impl_->closed.store(true,std::memory_order_release);
        if (impl_->counted) {
            impl_->domain.impl_->retention.retained_.fetch_sub(1,std::memory_order_release);
            impl_->domain.impl_->retained.fetch_sub(1,std::memory_order_release);
            impl_->counted = false;
        }
    } catch (...) { record_source_failure(); throw; }
}
void NativePostEffectParentDomain::close_after_host_quiescence() {
    require_owned(!impl_->failed.load(),"Failed parent graph must remain retained");
    { std::lock_guard<std::mutex> lock(impl_->mutex); impl_->closing.store(true,std::memory_order_release); }
    for (auto* frame=impl_->first; frame; frame=frame->impl_->next)
        if (!frame->impl_->closed.load()) frame->close_after_host_quiescence();
    require_owned(impl_->retained.load()==0,"Parent graph retains unresolved records");
}

void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame& frame,
    NativeString& name, Word vertices, const void* optional) { return frame.construct20(name,vertices,optional); }
void* create_native_post_effect_parent20_00b4e470(NativePostEffectParentFrame& frame,
    const NativePostEffect20ArgumentView& arguments) { return frame.construct20(arguments); }
void* create_native_post_effect_parent24_00b4e840(NativePostEffectParentFrame& frame,
    NativeString& name, Word primitives, const void* optional) { return frame.construct24(name,primitives,optional); }
void release_native_post_effect_parent_creator(NativePostEffectParentFrame& frame,
    long (__stdcall* decrement)(volatile long*)) { frame.release_creator(decrement); }
void* delete_native_post_effect_parent20_scalar_00b4e430(NativePostEffectParentFrame& frame,
    const void* flags_home) { frame.require_owned(frame.impl_->kind == Kind::parent20,"Parent20 scalar family mismatch"); return frame.scalar(flags_home); }
void* delete_native_post_effect_parent24_scalar_00b4e450(NativePostEffectParentFrame& frame,
    const void* flags_home) { frame.require_owned(frame.impl_->kind == Kind::parent24,"Parent24 scalar family mismatch"); return frame.scalar(flags_home); }
} // namespace bsp
