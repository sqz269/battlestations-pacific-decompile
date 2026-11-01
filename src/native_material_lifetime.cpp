#include "bsp/native_material_lifetime.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include <atomic>
#include <array>
#include <cstring>
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Ordinary Material lifetime requires the qualified MSVC Win32 providers.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Counter=std::atomic<std::int32_t>;
enum Entry : Word { initial=1, clone=2, cached=3, callable=4 };
constexpr std::size_t texture_sites=std::tuple_size_v<decltype(NativeMaterialStorage::textures_10)>;
constexpr std::size_t constructor_sites=texture_sites+1; // effect and actual texture assigns
constexpr std::size_t payload_sites=texture_sites+2; // plus conditional retained source drop
static_assert(sizeof(void*)==4 && sizeof(Counter)==4);
static_assert(sizeof(NativeMaterialStorage)==0x110 && alignof(NativeMaterialStorage)==4);
static_assert(std::is_trivially_destructible_v<NativeMaterialStorage>);
std::uint8_t late_flags(const void* home) noexcept {
    std::uint8_t value;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::memcpy(&value,home,1);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return value;
}
}
struct NativeMaterialDomain::Impl {
    NativeMaterialDestructionAccess& access;
    NativeRenderActualOwnerRegistry& owners;
    const volatile Word* const profile;
    NativeMeshRetentionState& retention;
    std::mutex mutex;
    NativeMaterialFrame* first{};
    std::atomic<unsigned> retained{0};
    std::atomic<bool> failed{false},closing{false};
    Impl(NativeMaterialDestructionAccess& a,NativeRenderActualOwnerRegistry& o,
        const volatile Word* p,NativeMeshRetentionState& r):access(a),owners(o),profile(p),retention(r){}
};
struct NativeMaterialFrame::Impl {
    NativeMaterialDomain& domain;
    NativeMaterialFrame* next{};
    NativeMaterialStorage* actual{};
    Counter* count_home{};
    NativeMaterialReference* reference{};
    using RefCell=std::aligned_storage_t<sizeof(NativeMaterialReference),alignof(NativeMaterialReference)>;
    RefCell reference_cell;
    bool attached{},counted{},reference_live{};
    std::atomic<unsigned> outer_calls{0};
    std::atomic<Word> entry{0};
    std::atomic<bool> failed{false},closed{false},producer_returned{false};
    std::atomic<bool> allocation_entered{false},allocation_returned{false};
    std::atomic<bool> constructor_entered{false},prefix_born{false},count_born{false};
    std::atomic<bool> native_ctor_returned{false},ctor_escaped{false},published{false};
    std::atomic<bool> partial_cpp_ended{false},selected_consumed{false};
    std::atomic<bool> selected_return_entered{false},selected_return_normal_same_slot{false};
    void* selected_operand{};
    NativeMaterialSlotPool* selected_pool{};
    std::atomic<bool> effect_acquire_entered{false},effect_acquire_returned{false};
    std::atomic<bool> effect_release_entered{false},effect_release_returned{false},effect_credit_consumed{false};
    void* acquired_effect{}; // opaque owned-return observation; never adopt/read
    void* release_effect{};
    NativeMaterialFactoryAcquired* factory_receiver{}; // stable retained external C++ borrow
    NativeMaterialEffectCacheContext* factory_context{};
    std::atomic<bool> creator_outstanding{false},creator_release_entered{false},zero_armed{false};
    std::atomic<bool> scalar_entered{false},payload_returned{false},flags_read{false};
    std::atomic<bool> typed_ended{false},live_storage_debt{false};
    std::atomic<bool> pool_return_entered{false},pool_return_normal{false};
    std::atomic<bool> scalar_body_completed{false},scalar_source_returned{false},retired{false};
    std::uint8_t captured_flags{};
    Word virtual_flags_home{};
    std::array<NativeRenderActualOwnerReleaseObservation,constructor_sites> constructor_children;
    std::array<NativeRenderActualOwnerReleaseObservation,payload_sites> payload_children;
    NativeRenderActualOwnerReleaseObservation factory_effect;
    explicit Impl(NativeMaterialDomain& d):domain(d){}
};
class NativeMaterialFrame::ChildScope final {
public:
    // Receiver ownership is private CPP metadata, not a Native mode/token:
    // 0=no heads,1=the unique producer's ctor/factory,2=the unique scalar's payload.
    explicit ChildScope(NativeMaterialFrame& f,Word owned) noexcept
        :frame_(f),uncaught_(std::uncaught_exceptions()),owned_(owned){
        frame_.begin_child_scope();}
    ~ChildScope() noexcept{
        // This scope belongs to the genuine one-shot operation after its
        // admission guard. No concurrent generic guard walks mutable heads.
        if(std::uncaught_exceptions()>uncaught_||frame_.requires_process_retention()){
            frame_.record_source_failure();frame_.fail_children_owned(owned_);
        }
        frame_.end_child_scope();
    }
    ChildScope(const ChildScope&)=delete;
    ChildScope& operator=(const ChildScope&)=delete;
private:
    NativeMaterialFrame& frame_;
    const int uncaught_;
    const Word owned_;
};

NativeMaterialDomain::NativeMaterialDomain(NativeMaterialDestructionAccess& access,
    NativeRenderActualOwnerRegistry& owners,const volatile Word* profile,
    NativeMeshRetentionState& retention):impl_(std::make_unique<Impl>(access,owners,profile,retention)) {
    require_owned(&access.retained_owners==static_cast<NativeRenderActualOwners*>(&owners) && profile,
        "Material domain requires SAME actual registry/pool/access and current profile binding");
}
NativeMaterialDomain::~NativeMaterialDomain() {
    // Actual owner must retain this graph on any unclosed/failed debt. No
    // destructor reconstructs cleanup, resets Native counts, or terminates.
    auto* frame=impl_->first;
    while(frame){auto* next=frame->impl_->next;delete frame;frame=next;}
}
void NativeMaterialDomain::require_owned(bool value,const char* message) const {
    if(!value){NativeMaterialFrame::record_domain_failure(*const_cast<NativeMaterialDomain*>(this));
        throw std::logic_error(message);}
}
NativeMaterialFrame::NativeMaterialFrame(NativeMaterialDomain& domain)
    :impl_(std::make_unique<Impl>(domain)),exact_(domain.impl_->owners,this){}
NativeMaterialFrame::~NativeMaterialFrame(){
    if(impl_->reference_live)std::destroy_at(impl_->reference); // private ordinary metadata branch
}
void NativeMaterialFrame::record_domain_failure(NativeMaterialDomain& d) noexcept {
    d.impl_->failed.store(true,std::memory_order_release);d.impl_->retention.fail();
}
void NativeMaterialFrame::record_source_failure() noexcept {
    impl_->failed.store(true,std::memory_order_release);
    record_domain_failure(impl_->domain);
    if(impl_->attached)impl_->domain.impl_->owners.poison_exact_record(exact_);
    // Metadata-only and idempotent. The operation's own scope propagates its
    // caller heads at the real fence; saved-C++ guards may run on another thread.
}
void NativeMaterialFrame::require_owned(bool value,const char* message) const {
    if(!value){const_cast<NativeMaterialFrame*>(this)->record_source_failure();throw std::logic_error(message);}
}
void NativeMaterialFrame::require_healthy() const {
    require_owned(!impl_->failed.load(std::memory_order_acquire) &&
        !impl_->domain.impl_->failed.load(std::memory_order_acquire) && !impl_->closed.load() &&
        !impl_->domain.impl_->retention.requires_process_retention(),
        "Material retains failed/closed Source custody");
}
bool NativeMaterialDomain::requires_process_retention() const noexcept{return impl_->failed.load(std::memory_order_acquire)||impl_->retention.requires_process_retention();}
bool NativeMaterialDomain::has_retained_records() const noexcept{return impl_->retained.load(std::memory_order_acquire)!=0;}
bool NativeMaterialFrame::requires_process_retention() const noexcept{return impl_->failed.load(std::memory_order_acquire)||impl_->domain.requires_process_retention();}
NativeMaterialFrame& NativeMaterialDomain::prepare(){
    try{
        require_owned(!impl_->closing.load()&&!impl_->failed.load(),"Material prepare after close/failure");
        auto pending=std::unique_ptr<NativeMaterialFrame>(new NativeMaterialFrame(*this));
        auto* frame=pending.get();bool linked=false;
        {std::lock_guard<std::mutex> lock(impl_->mutex);
            if(!impl_->closing.load()){frame->impl_->next=impl_->first;impl_->first=frame;
                (void)pending.release();linked=true;}}
        require_owned(linked,"Material prepare crossed actual owning close fence");
        frame->prepare_after_link();return *frame;
    }catch(...){NativeMaterialFrame::record_domain_failure(*this);throw;}
}
void NativeMaterialFrame::prepare_after_link(){
    try{
        auto& d=*impl_->domain.impl_;d.owners.attach_exact_record(exact_);impl_->attached=true;
        d.retention.retained_.fetch_add(1,std::memory_order_acq_rel);
        d.retained.fetch_add(1,std::memory_order_acq_rel);impl_->counted=true;
    }catch(...){record_source_failure();throw;}
}
void NativeMaterialFrame::begin_outer_call(){
    require_healthy();require_owned(!impl_->domain.impl_->closing.load(),"Material operation after owning close fence");
    impl_->outer_calls.fetch_add(1,std::memory_order_acq_rel);exact_.outer_calls_.fetch_add(1,std::memory_order_acq_rel);
}
void NativeMaterialFrame::end_outer_call() noexcept {
    impl_->outer_calls.fetch_sub(1,std::memory_order_release);exact_.outer_calls_.fetch_sub(1,std::memory_order_release);
}
void NativeMaterialFrame::begin_producer(Word kind){
    require_healthy();Word expected=0;
    require_owned(impl_->entry.compare_exchange_strong(expected,kind),"Material producer cannot replay/adopt a born slot");
}
void NativeMaterialFrame::begin_child_scope() noexcept {
    for(auto& o:impl_->constructor_children)o.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
    for(auto& o:impl_->payload_children)o.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
    impl_->factory_effect.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
}
void NativeMaterialFrame::end_child_scope() noexcept {
    for(auto& o:impl_->constructor_children)o.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
    for(auto& o:impl_->payload_children)o.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
    impl_->factory_effect.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
}
void NativeMaterialFrame::fail_children_owned(Word owned) noexcept {
    // Only the admitted operation's actual last-use/unwind scope may traverse
    // these same-operation caller heads. Child failure methods mark metadata;
    // they do not recursively traverse another operation's live receiver heads.
    if(owned==1){
        for(auto& o:impl_->constructor_children)NativeRenderObservedZeroAccess::fail_frame(o.cpp_zero_frame);
        NativeRenderObservedZeroAccess::fail_frame(impl_->factory_effect.cpp_zero_frame);
    }else if(owned==2){
        for(auto& o:impl_->payload_children)NativeRenderObservedZeroAccess::fail_frame(o.cpp_zero_frame);
    }
}
void NativeMaterialFrame::release_child(NativeRenderActualOwners& owners,void* actual,std::size_t site,bool payload){
    require_owned(&owners==&impl_->domain.impl_->access.retained_owners&&site<(payload?payload_sites:constructor_sites),
        "Material child release requires its exact real field/site and SAME owner domain");
    auto& observation=payload?impl_->payload_children[site]:impl_->constructor_children[site];
    release_native_render_actual_owner_observed(owners,actual,observation);
}
void NativeMaterialFrame::release_factory_effect(NativeRenderActualOwners& owners,void* actual){
    require_owned(&owners==&impl_->domain.impl_->access.retained_owners,"Material factory effect domain mismatch");
    release_native_render_actual_owner_observed(owners,actual,impl_->factory_effect);
}
void NativeMaterialFrame::native_allocation_entered() noexcept {
    if(impl_->allocation_entered.exchange(true))record_source_failure();
}
void NativeMaterialFrame::native_allocation_returned(void* actual){
    require_owned(impl_->allocation_entered.load()&&!impl_->allocation_returned.exchange(true),
        "Material allocation observation cannot be fabricated/replayed");
    impl_->actual=static_cast<NativeMaterialStorage*>(actual);
    if(actual)impl_->domain.impl_->owners.prepare_exact_record(exact_,actual);
}
void NativeMaterialFrame::native_constructor_entered(void* actual){
    require_owned(impl_->allocation_returned.load()&&actual&&actual==impl_->actual&&
        !impl_->constructor_entered.exchange(true),"Material ctor requires exact real allocation return");
}
void NativeMaterialFrame::cpp_prefix_born(NativeMaterialStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->constructor_entered.load()||impl_->prefix_born.exchange(true))
        record_source_failure();
}
void NativeMaterialFrame::native_count1_born(NativeMaterialStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->prefix_born.load()||impl_->count_born.exchange(true)){
        record_source_failure();return;}
    impl_->count_home=&actual.references_04; // address only, after existing store1
}
void NativeMaterialFrame::native_constructor_returned(NativeMaterialStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->count_born.load()||impl_->native_ctor_returned.exchange(true)){
        record_source_failure();return;}
    impl_->creator_outstanding.store(true,std::memory_order_release);
}
void NativeMaterialFrame::source_constructor_escaped() noexcept {
    impl_->ctor_escaped.store(true,std::memory_order_release);record_source_failure();
}
bool NativeMaterialFrame::may_select_constructor_raw_return() const noexcept {
    return impl_->constructor_entered.load()&&impl_->ctor_escaped.load(std::memory_order_acquire)&&!impl_->native_ctor_returned.load()&&
        !impl_->selected_consumed.load();
}
void NativeMaterialFrame::selected_constructor_raw_return(void* selected,NativeMaterialSlotPool& pool){
    // This is an actual selected action, even when the late CURRENT operand no
    // longer equals our original allocation. Never substitute A for that HOME.
    if(!may_select_constructor_raw_return()||impl_->selected_consumed.exchange(true)){
        record_source_failure();throw std::logic_error("Material selected raw-return replay/wrong continuation");}
    impl_->selected_operand=selected;impl_->selected_pool=&pool;
    const bool same=selected==impl_->actual && &pool==&impl_->domain.impl_->access.material_slots;
    const bool can_end=same&&impl_->prefix_born.load()&&!impl_->native_ctor_returned.load()&&
        !impl_->published.load()&&exact_.borrows_.load()==0;
    if(can_end){
        // The real shared ctor has already escaped through its final prefix
        // use. No canonical or prefix borrow escaped; this is a NEW actual
        // trivial C++ end, not normal Native destruction/unborn abandonment.
        impl_->actual->~NativeMaterialStorage();
        impl_->partial_cpp_ended.store(true,std::memory_order_release);
        impl_->domain.impl_->owners.deactivate_exact_record_from_partial_cpp_end(exact_);
    }
    // A mismatch issues no B lifetime/ownership fact, no A end/physical fact.
    // The inherited actual selected operand/provider row remains explicit.
    impl_->selected_return_entered.store(true,std::memory_order_release);
    try{
        pool.return_slot_00b17a80(selected);
        if(same)impl_->selected_return_normal_same_slot.store(true,std::memory_order_release);
    }catch(...){record_source_failure();throw;}
    // Failed graph and exact membership are deliberately retained. This new
    // partial fact does NOT authorize retire/close/coldcancel or child refunds.
}
void NativeMaterialFrame::effect_acquisition_entered() noexcept {impl_->effect_acquire_entered.store(true,std::memory_order_release);}
void NativeMaterialFrame::effect_acquisition_returned(void* effect) noexcept {
    impl_->acquired_effect=effect;impl_->effect_acquire_returned.store(true,std::memory_order_release);
}
void NativeMaterialFrame::effect_release_entered(void* effect) noexcept {
    impl_->release_effect=effect;impl_->effect_release_entered.store(true,std::memory_order_release);
}
void NativeMaterialFrame::effect_release_returned() noexcept {
    impl_->effect_release_returned.store(true,std::memory_order_release);
    if(impl_->effect_acquire_returned.load()&&impl_->release_effect==impl_->acquired_effect)
        impl_->effect_credit_consumed.store(true,std::memory_order_release);
    else record_source_failure(); // actual other operand is not an A-credit receipt
}
void NativeMaterialFrame::publish_after_native_constructor(){
    require_healthy();require_owned(impl_->native_ctor_returned.load()&&!impl_->published.load(),
        "Material canonical publication needs real complete constructor return");
    auto& d=*impl_->domain.impl_;
    impl_->reference=::new(&impl_->reference_cell)NativeMaterialReference(*impl_->actual,d.access,d.profile,*this);
    impl_->reference_live=true;impl_->reference->bind_ordinary_material(*this);
    d.owners.publish_exact_record(exact_,*impl_->reference);impl_->published.store(true,std::memory_order_release);
}
void NativeMaterialFrame::producer_returned(NativeMaterialStorage* actual){
    require_healthy();require_owned(impl_->allocation_returned.load()&&actual==impl_->actual,
        "Material producer return must match actual allocator/constructor");
    const bool factory=impl_->entry.load()==Entry::cached||impl_->entry.load()==Entry::callable;
    require_owned(!factory||(impl_->effect_acquire_returned.load()&&impl_->effect_release_returned.load()&&
        impl_->effect_credit_consumed.load()),
        "Material factory return lacks its real effect-release completion");
    if(actual)publish_after_native_constructor();
    else require_owned(!impl_->constructor_entered.load()&&!impl_->prefix_born.load(),"NULL Material cannot claim birth");
    require_healthy();impl_->producer_returned.store(true,std::memory_order_release);
}
NativeMaterialStorage* NativeMaterialFrame::create_initial(void* effect){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();
    try{
        begin_producer(Entry::initial);const ChildScope child_scope(*this,1);native_allocation_entered();
        void* actual=allocate_native_material_slot_00b18780(impl_->domain.impl_->access.material_slots);
        native_allocation_returned(actual);
        if(actual){
            try{(void)NativeMaterialStorageAccess::initialize(actual,effect,impl_->domain.impl_->access.retained_owners,*this);}
            catch(...){if(may_select_constructor_raw_return())selected_constructor_raw_return(actual,impl_->domain.impl_->access.material_slots);throw;}
        }
        producer_returned(static_cast<NativeMaterialStorage*>(actual));end_outer_call();return static_cast<NativeMaterialStorage*>(actual);
    }catch(...){record_source_failure();end_outer_call();throw;}
}
NativeMaterialStorage* NativeMaterialFrame::create_clone(const NativeMaterialStorage& source){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();
    try{
        begin_producer(Entry::clone);const ChildScope child_scope(*this,1);native_allocation_entered();
        void* actual=allocate_native_material_slot_00b18780(impl_->domain.impl_->access.material_slots);
        native_allocation_returned(actual);
        if(actual){
            try{(void)NativeMaterialStorageAccess::clone(actual,source,impl_->domain.impl_->access.retained_owners,*this);}
            catch(...){if(may_select_constructor_raw_return())selected_constructor_raw_return(actual,impl_->domain.impl_->access.material_slots);throw;}
        }
        producer_returned(static_cast<NativeMaterialStorage*>(actual));end_outer_call();return static_cast<NativeMaterialStorage*>(actual);
    }catch(...){record_source_failure();end_outer_call();throw;}
}
NativeMaterialStorage* NativeMaterialFrame::create_cached(const void* name,NativeMaterialEffectCacheContext& cache,
    NativeMaterialFactoryAcquired& acquired){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();
    try{
        begin_producer(Entry::cached);const ChildScope child_scope(*this,1);
        // This is a real persistent C++ receiver/context loan, not a public
        // phase/Acquired-derived Native permit. Never inspect it after end.
        impl_->factory_receiver=&acquired;impl_->factory_context=&cache;
        auto& d=*impl_->domain.impl_;
        auto* actual=NativeMaterialFactoryAccess::cached(name,d.access.material_slots,d.access.retained_owners,cache,acquired,*this);
        producer_returned(actual);end_outer_call();return actual;
    }catch(...){record_source_failure();end_outer_call();throw;}
}
NativeMaterialStorage* NativeMaterialFrame::create_callable(NativeString& name,void* const volatile& renderer){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();
    try{
        begin_producer(Entry::callable);const ChildScope child_scope(*this,1);auto& d=*impl_->domain.impl_;
        auto* actual=NativeMaterialFactoryAccess::callable(name,renderer,d.access.material_slots,d.access.retained_owners,*this);
        producer_returned(actual);end_outer_call();return actual;
    }catch(...){record_source_failure();end_outer_call();throw;}
}
void NativeMaterialFrame::require_same_canonical(RenderCommandReference& reference) const {
    require_healthy();require_owned(!impl_->domain.impl_->closing.load()&&impl_->published.load()&&
        impl_->producer_returned.load()&&impl_->reference==&reference&&reference.ordinary_material_==this&&
        !impl_->scalar_entered.load()&&!impl_->typed_ended.load()&&!impl_->partial_cpp_ended.load()&&
        static_cast<void*>(&reference.reference_count)==impl_->count_home,
        "Material requires SAME private genuine live canonical/count address");
}
NativeMaterialReference& NativeMaterialFrame::canonical(){
    require_owned(impl_->reference!=nullptr,"Material canonical unavailable");
    require_same_canonical(*impl_->reference);return *impl_->reference;
}
void NativeMaterialFrame::begin_cpp_count_use(RenderCommandReference& reference){
    require_same_canonical(reference);exact_.borrows_.fetch_add(1,std::memory_order_acq_rel);
    try{require_same_canonical(reference);}catch(...){record_source_failure();end_cpp_count_use();throw;}
}
void NativeMaterialFrame::end_cpp_count_use() noexcept {exact_.borrows_.fetch_sub(1,std::memory_order_release);}
void NativeMaterialFrame::hold_observed_zero(NativeRenderObservedZeroFrame& caller,RenderCommandReference& reference){
    begin_outer_call();try{
        require_same_canonical(reference);bool expected=false;
        require_owned(observed_zero_hold_.claimed_.compare_exchange_strong(expected,true),"Material zero hold cannot replay");
        observed_zero_hold_.material_=this;NativeRenderObservedZeroAccess::attach_hold(caller,observed_zero_hold_);
    }catch(...){record_source_failure();end_outer_call();throw;}
}
void NativeMaterialFrame::release_observed_zero_hold(bool failed) noexcept {if(failed)record_source_failure();end_outer_call();}
void NativeMaterialFrame::invoke_observed_zero(void* actual,RenderCommandReference& reference){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();try{
        require_same_canonical(reference);require_owned(actual==impl_->actual&&!impl_->zero_armed.exchange(true),
            "Material zero action/actual mismatch");
        const ChildScope child_scope(*this,0);
        reference.release_zero_references();require_healthy();end_outer_call();
    }catch(...){impl_->zero_armed.store(false);record_source_failure();end_outer_call();throw;}
}
void NativeMaterialFrame::ordinary_virtual_zero(RenderCommandReference& reference){
    require_same_canonical(reference);require_owned(impl_->zero_armed.exchange(false),"Material has no real observed-zero action");
    impl_->reference->validate_ordinary_target();impl_->virtual_flags_home=1;
    (void)scalar(&impl_->virtual_flags_home);
}
NativeMaterialStorage* NativeMaterialFrame::scalar(const void* flags_home){
    require_healthy();require_owned(flags_home&&impl_->producer_returned.load()&&impl_->published.load()&&
        !impl_->scalar_entered.exchange(true),"Material scalar needs a genuine one-shot direct/zero action");
    const ChildScope child_scope(*this,2);
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();auto* const original=impl_->actual;
    try{
        NativeMaterialStorageAccess::destroy(*original,impl_->domain.impl_->access,*this);
        impl_->payload_returned.store(true,std::memory_order_release);
        impl_->creator_outstanding.store(false,std::memory_order_release);
        const auto flags=late_flags(flags_home);impl_->captured_flags=flags;
        impl_->flags_read.store(true,std::memory_order_release);
        if(flags&1u){
            original->~NativeMaterialStorage();impl_->typed_ended.store(true,std::memory_order_release);
            impl_->domain.impl_->owners.deactivate_exact_record_from_native_end(exact_);
            impl_->pool_return_entered.store(true,std::memory_order_release);
            impl_->domain.impl_->access.material_slots.return_slot_00b17a80(original);
            impl_->pool_return_normal.store(true,std::memory_order_release);
        }else{
            // Keep the inherited flags0 C++ carrier lifetime. This is known
            // debt, NOT failure, normal-dead, unborn or partial C++ end.
            impl_->live_storage_debt.store(true,std::memory_order_release);
            impl_->domain.impl_->owners.withhold_exact_record_for_live_storage_debt(exact_);
        }
        impl_->scalar_body_completed.store(true,std::memory_order_release);
        if(flags&1u){impl_->domain.impl_->owners.retire_exact_record(exact_);impl_->retired.store(true,std::memory_order_release);}
        require_healthy();impl_->scalar_source_returned.store(true,std::memory_order_release);
        end_outer_call();return original;
    }catch(...){record_source_failure();end_outer_call();throw;}
}
void NativeMaterialFrame::release_creator(long (__stdcall* decrement)(volatile long*)){
    require_healthy();require_owned(decrement&&impl_->producer_returned.load()&&impl_->published.load()&&
        impl_->creator_outstanding.load()&&!impl_->scalar_entered.load()&&!impl_->creator_release_entered.exchange(true),
        "Material creator action requires its actual unconsumed constructor credit/import");
    const ChildScope child_scope(*this,0);
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();try{
        const long result=decrement(reinterpret_cast<volatile long*>(impl_->count_home));
        impl_->creator_outstanding.store(false,std::memory_order_release);
        if(result==0)NativeRenderObservedZeroAccess::dispatch(impl_->domain.impl_->owners,impl_->actual,terminal_zero_frame_);
        require_healthy();end_outer_call();
    }catch(...){record_source_failure();end_outer_call();throw;}
}
void NativeMaterialFrame::preflight_close() const {
    require_healthy();auto* caller=observed_zero_hold_.frame_;
    const unsigned held=caller?1u:0u;
    // All physical and active-caller facts precede the FIRST hold unlink.
    // The real owner separately proves whole caller/Native/worker quiescence.
    require_owned(terminal_zero_frame_.active_invocations_.load()==0&&(!caller||caller->active_invocations_.load()==0)&&
        impl_->outer_calls.load()==held&&exact_.outer_calls_.load()==held&&exact_.borrows_.load()==0,
        "Material close crossed an actual active operation/count/caller fence");
    for(auto& o:impl_->constructor_children)require_owned(o.cpp_zero_frame.active_invocations_.load()==0,"Material ctor child caller active");
    for(auto& o:impl_->payload_children)require_owned(o.cpp_zero_frame.active_invocations_.load()==0,"Material payload child caller active");
    require_owned(impl_->factory_effect.cpp_zero_frame.active_invocations_.load()==0,"Material factory effect caller active");
    const bool cold=impl_->entry.load()==0&&!impl_->allocation_entered.load()&&!impl_->prefix_born.load();
    const bool normal_null=impl_->producer_returned.load()&&impl_->allocation_returned.load()&&!impl_->actual&&
        !impl_->constructor_entered.load()&&!impl_->prefix_born.load();
    const bool complete=impl_->scalar_source_returned.load()&&impl_->payload_returned.load()&&impl_->typed_ended.load()&&
        impl_->pool_return_normal.load()&&impl_->retired.load()&&!impl_->creator_outstanding.load()&&!impl_->live_storage_debt.load();
    require_owned(cold||normal_null||complete,"Material close retains flags0/partial/unknown/physical/source debt");
}
void NativeMaterialFrame::close_after_host_quiescence(){
    preflight_close();
    auto* caller=observed_zero_hold_.frame_;
    const bool cold=impl_->entry.load()==0;
    const bool normal_null=impl_->producer_returned.load()&&!impl_->actual;
    try{
        for(auto& o:impl_->constructor_children)require_owned(NativeRenderObservedZeroAccess::end_frame(o.cpp_zero_frame,false),"Material ctor child did not quiesce");
        for(auto& o:impl_->payload_children)require_owned(NativeRenderObservedZeroAccess::end_frame(o.cpp_zero_frame,false),"Material payload child did not quiesce");
        require_owned(NativeRenderObservedZeroAccess::end_frame(impl_->factory_effect.cpp_zero_frame,false),"Material factory effect did not quiesce");
        if(caller)require_owned(NativeRenderObservedZeroAccess::end_frame(*caller,false),"Material caller invocation active");
        if(caller!=&terminal_zero_frame_)require_owned(NativeRenderObservedZeroAccess::end_frame(terminal_zero_frame_,false),
            "Material terminal invocation active");
        require_healthy();require_owned(impl_->outer_calls.load()==0&&exact_.outer_calls_.load()==0&&exact_.borrows_.load()==0,
            "Material close still has actual CPP users");
        if(cold||normal_null)impl_->domain.impl_->owners.cancel_exact_record_without_payload(exact_);
        else impl_->domain.impl_->owners.close_exact_record_at_quiescence(exact_);
        impl_->attached=false;impl_->closed.store(true,std::memory_order_release);
        impl_->factory_receiver=nullptr;impl_->factory_context=nullptr;
        if(impl_->counted){impl_->domain.impl_->retention.retained_.fetch_sub(1,std::memory_order_release);
            impl_->domain.impl_->retained.fetch_sub(1,std::memory_order_release);impl_->counted=false;}
    }catch(...){record_source_failure();throw;}
}
void NativeMaterialDomain::close_after_host_quiescence(){
    try{
        require_owned(!impl_->failed.load(),"Failed Material graph must remain retained");
        {std::lock_guard<std::mutex> lock(impl_->mutex);impl_->closing.store(true,std::memory_order_release);}
        // Domain-wide first pass: no earlier Frame may unlink before a later
        // flags0/physical/active/child/caller obligation has been checked.
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next)
            if(!frame->impl_->closed.load())frame->preflight_close();
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next)if(!frame->impl_->closed.load())frame->close_after_host_quiescence();
        require_owned(impl_->retained.load()==0,"Material domain still retains actual obligations");
    }catch(...){NativeMaterialFrame::record_domain_failure(*this);throw;}
}

NativeMaterialStorage* create_native_material_initial_00b18900(NativeMaterialFrame& f,void* effect){return f.create_initial(effect);}
NativeMaterialStorage* create_native_material_clone_00b18b60(NativeMaterialFrame& f,const NativeMaterialStorage& source){return f.create_clone(source);}
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(NativeMaterialFrame& f,NativeString& name,
    NativeMaterialEffectCacheContext& c,NativeMaterialFactoryAcquired& a){return f.create_cached(&name,c,a);}
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(NativeMaterialFrame& f,const void* name,
    NativeMaterialEffectCacheContext& c,NativeMaterialFactoryAcquired& a){return f.create_cached(name,c,a);}
NativeMaterialStorage* create_native_material_for_effect_00535320(NativeMaterialFrame& f,NativeString& name,void* const volatile& renderer){return f.create_callable(name,renderer);}
void release_native_material_creator(NativeMaterialFrame& f,long (__stdcall* decrement)(volatile long*)){f.release_creator(decrement);}
NativeMaterialStorage* delete_native_material_scalar_00b194b0(NativeMaterialFrame& f,const void* flags_home){return f.scalar(flags_home);}
} // namespace bsp
