#include "bsp/native_mesh_section_lifetime.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <exception>
#include <mutex>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Ordinary Section lifetime requires the qualified MSVC Win32 providers.
#endif

namespace bsp {
namespace {
using Word=std::uint32_t;
using Counter=std::atomic<std::int32_t>;
constexpr std::size_t child_sites=8; // +38,+5C,+20,four actual +3C slots,+50
static_assert(sizeof(void*)==4 && sizeof(Counter)==4);
static_assert(sizeof(NativeMeshSectionStorage)==0x60 && alignof(NativeMeshSectionStorage)==4);
static_assert(std::is_trivially_destructible_v<NativeMeshSectionStorage>);
static_assert(NativeMeshSectionPool::slot_bytes==0x64 && NativeMeshSectionPool::slot_slab_index_offset==0x60);
std::uint8_t late_flags(const void* home) noexcept {
    std::uint8_t value;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    std::memcpy(&value,home,1);
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return value;
}
}

struct NativeMeshSectionLifetimeDomain::Impl {
    NativeMeshSectionEnvironment& environment;
    NativeRenderActualOwnerRegistry& owners;
    const volatile Word& bounds;
    NativeMeshRetentionState& retention;
    std::mutex mutex;
    NativeMeshSectionLifetimeFrame* first{};
    std::atomic<unsigned> retained{0};
    std::atomic<bool> failed{false},closing{false};
    Impl(NativeMeshSectionEnvironment& e,NativeRenderActualOwnerRegistry& o,
        const volatile Word& b,NativeMeshRetentionState& r):environment(e),owners(o),bounds(b),retention(r){}
};
struct NativeMeshSectionLifetimeCall::Impl {
    NativeMeshSectionLifetimeFrame& frame;
    NativeMeshSectionLifetimeCall* next{};
    NativeMeshSectionLifetimeCall* source_next{};
    NativeMeshSectionLifetimeSourceScope* source{};
    const bool terminal;
    std::atomic<Word> operation{0};
    std::atomic<bool> source_active{false},leaf_entered{false},returned{false},failed{false};
    std::array<NativeRenderActualOwnerReleaseObservation,child_sites> children;
    NativeRenderActualOwnerReleaseObservation reference_drop;
    NativeRenderObservedZeroFrame direct_zero;
    Impl(NativeMeshSectionLifetimeFrame& f,bool t):frame(f),terminal(t){}
};
struct NativeMeshSectionLifetimeFrame::Impl {
    NativeMeshSectionLifetimeDomain& domain;
    NativeMeshSectionLifetimeFrame* next{};
    std::mutex calls_mutex;
    Call* calls{};
    Call* terminal{};
    NativeMeshSectionStorage* actual{};
    Counter* count_home{};
    NativeMeshSectionReference* reference{};
    using RefCell=std::aligned_storage_t<sizeof(NativeMeshSectionReference),alignof(NativeMeshSectionReference)>;
    RefCell reference_cell;
    bool attached{},counted{},reference_live{};
    std::atomic<unsigned> outer_calls{0};
    std::atomic<Word> producer{0};
    std::atomic<bool> failed{false},closed{false},producer_returned{false};
    std::atomic<bool> allocation_entered{false},allocation_returned{false};
    std::atomic<bool> constructor_entered{false},prefix_born{false},count_born{false};
    std::atomic<bool> native_ctor_returned{false},ctor_return_captured{false},ctor_escaped{false},published{false};
    NativeMeshSectionStorage* constructor_return_value{};
    std::atomic<bool> partial_cpp_ended{false},selected_consumed{false};
    std::atomic<bool> selected_return_entered{false},selected_return_normal_same_slot{false};
    void* selected_operand{};
    NativeMeshSectionPool* selected_pool{};
    std::atomic<bool> creator_outstanding{false},creator_release_entered{false},creator_release_returned{false};
    std::atomic<bool> creator_zero_entered{false},creator_zero_returned{false},zero_armed{false};
    long creator_result{};
    std::atomic<bool> scalar_entered{false},payload_active{false},payload_returned{false},flags_read{false};
    std::atomic<bool> typed_ended{false},live_storage_debt{false};
    std::atomic<bool> pool_return_entered{false},pool_return_normal{false};
    std::atomic<bool> scalar_body_completed{false},scalar_source_returned{false},retired{false};
    std::uint8_t captured_flags{};
    Word virtual_flags_home{};
    explicit Impl(NativeMeshSectionLifetimeDomain& d):domain(d){}
};

NativeMeshSectionLifetimeSourceScope::NativeMeshSectionLifetimeSourceScope() noexcept
    :uncaught_(std::uncaught_exceptions()){}
void NativeMeshSectionLifetimeSourceScope::fail() noexcept {
    failed_=true;
    // Metadata only. No generic saved-C++ guard traverses mutable zero heads.
    for(auto* call=first_;call;call=call->impl_->source_next)call->impl_->frame.record_source_failure();
}
NativeMeshSectionLifetimeSourceScope::~NativeMeshSectionLifetimeSourceScope() noexcept {
    ended_=true;
    const bool escaped=failed_||std::uncaught_exceptions()>uncaught_;
    // This is the actual Source owner's last-use fence, and this list contains
    // only Calls attached to THAT invocation before their own Native effects.
    for(auto* call=first_;call;call=call->impl_->source_next){
        auto& frame=call->impl_->frame;
        if(escaped||frame.requires_process_retention()){
            frame.record_source_failure();frame.fail_call_owned(*call);
        }
        if(call->impl_->source_active.exchange(false,std::memory_order_acq_rel)){
            frame.end_call_activity(*call);frame.end_outer_call();
        }else frame.record_source_failure();
    }
}
NativeMeshSectionLifetimeCall::NativeMeshSectionLifetimeCall(NativeMeshSectionLifetimeFrame& frame,bool terminal)
    :impl_(std::make_unique<Impl>(frame,terminal)){}
NativeMeshSectionLifetimeCall::~NativeMeshSectionLifetimeCall()=default;
bool NativeMeshSectionLifetimeCall::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire)||impl_->frame.requires_process_retention();
}

class NativeMeshSectionLifetimeFrame::OperationScope final {
public:
    OperationScope(NativeMeshSectionLifetimeFrame& frame,Call& call):frame_(frame),call_(call),uncaught_(std::uncaught_exceptions()){
        frame_.begin_outer_call();frame_.begin_call_activity(call_);
        call_.impl_->leaf_entered.store(true,std::memory_order_release);
    }
    ~OperationScope() noexcept {
        if(std::uncaught_exceptions()>uncaught_||frame_.requires_process_retention()){
            frame_.record_source_failure();frame_.fail_call_owned(call_);
        }
        frame_.end_call_activity(call_);frame_.end_outer_call();
    }
    OperationScope(const OperationScope&)=delete;
    OperationScope& operator=(const OperationScope&)=delete;
private:
    NativeMeshSectionLifetimeFrame& frame_;
    Call& call_;
    const int uncaught_;
};
class NativeMeshSectionLifetimeFrame::PayloadScope final {
public:
    explicit PayloadScope(NativeMeshSectionLifetimeFrame& frame) noexcept:frame_(frame){
        frame_.impl_->payload_active.store(true,std::memory_order_release);
    }
    ~PayloadScope() noexcept {frame_.impl_->payload_active.store(false,std::memory_order_release);}
    PayloadScope(const PayloadScope&)=delete;
    PayloadScope& operator=(const PayloadScope&)=delete;
private:
    NativeMeshSectionLifetimeFrame& frame_;
};

NativeMeshSectionLifetimeDomain::NativeMeshSectionLifetimeDomain(NativeMeshSectionEnvironment& environment,
    NativeRenderActualOwnerRegistry& owners,const volatile Word& bounds,NativeMeshRetentionState& retention)
    :impl_(std::make_unique<Impl>(environment,owners,bounds,retention)){
    require_owned(&environment.retained_owners==static_cast<NativeRenderActualOwners*>(&owners)&&
        environment.vtable_00d63194,"Section domain requires SAME actual environment/registry/profile providers");
}
NativeMeshSectionLifetimeDomain::~NativeMeshSectionLifetimeDomain(){
    // The real owner must retain this entire graph on live/failed debt. No
    // destructor invents Native cleanup, refunds a count, or terminates.
    auto* frame=impl_->first;
    while(frame){auto* next=frame->impl_->next;delete frame;frame=next;}
}
void NativeMeshSectionLifetimeDomain::require_owned(bool value,const char* message) const {
    if(!value){NativeMeshSectionLifetimeFrame::record_domain_failure(*const_cast<NativeMeshSectionLifetimeDomain*>(this));
        throw std::logic_error(message);}
}
NativeMeshSectionLifetimeFrame::NativeMeshSectionLifetimeFrame(NativeMeshSectionLifetimeDomain& domain)
    :impl_(std::make_unique<Impl>(domain)),exact_(domain.impl_->owners,this){}
NativeMeshSectionLifetimeFrame::~NativeMeshSectionLifetimeFrame(){
    if(impl_->reference_live)std::destroy_at(impl_->reference); // private ordinary metadata branch
    auto* call=impl_->calls;
    while(call){auto* next=call->impl_->next;delete call;call=next;}
}
void NativeMeshSectionLifetimeFrame::record_domain_failure(NativeMeshSectionLifetimeDomain& domain) noexcept {
    domain.impl_->failed.store(true,std::memory_order_release);domain.impl_->retention.fail();
}
void NativeMeshSectionLifetimeFrame::record_source_failure() noexcept {
    impl_->failed.store(true,std::memory_order_release);record_domain_failure(impl_->domain);
    if(impl_->attached)impl_->domain.impl_->owners.poison_exact_record(exact_);
    // Metadata ONLY. The actual operation's own scope propagates its own heads.
}
void NativeMeshSectionLifetimeFrame::require_owned(bool value,const char* message) const {
    if(!value){const_cast<NativeMeshSectionLifetimeFrame*>(this)->record_source_failure();throw std::logic_error(message);}
}
void NativeMeshSectionLifetimeFrame::require_healthy() const {
    require_owned(!impl_->failed.load(std::memory_order_acquire)&&!impl_->domain.impl_->failed.load(std::memory_order_acquire)&&
        !impl_->closed.load(std::memory_order_acquire)&&!impl_->domain.impl_->retention.requires_process_retention(),
        "Section retains failed/closed Source custody");
}
bool NativeMeshSectionLifetimeDomain::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire)||impl_->retention.requires_process_retention();
}
bool NativeMeshSectionLifetimeDomain::has_retained_records() const noexcept{return impl_->retained.load(std::memory_order_acquire)!=0;}
bool NativeMeshSectionLifetimeFrame::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire)||impl_->domain.requires_process_retention();
}
NativeMeshSectionLifetimeFrame& NativeMeshSectionLifetimeDomain::prepare(){
    try{
        require_owned(!impl_->closing.load()&&!impl_->failed.load(),"Section prepare after close/failure");
        auto pending=std::unique_ptr<NativeMeshSectionLifetimeFrame>(new NativeMeshSectionLifetimeFrame(*this));
        auto* frame=pending.get();bool linked=false;
        {std::lock_guard<std::mutex> lock(impl_->mutex);
            if(!impl_->closing.load()){frame->impl_->next=impl_->first;impl_->first=frame;(void)pending.release();linked=true;}}
        require_owned(linked,"Section prepare crossed actual owning close fence");
        frame->prepare_after_link();return *frame;
    }catch(...){NativeMeshSectionLifetimeFrame::record_domain_failure(*this);throw;}
}
void NativeMeshSectionLifetimeFrame::prepare_after_link(){
    try{
        auto pending=std::unique_ptr<Call>(new Call(*this,true));
        impl_->terminal=pending.get();impl_->calls=pending.release();
        auto& domain=*impl_->domain.impl_;domain.owners.attach_exact_record(exact_);impl_->attached=true;
        domain.retention.retained_.fetch_add(1,std::memory_order_acq_rel);
        domain.retained.fetch_add(1,std::memory_order_acq_rel);impl_->counted=true;
    }catch(...){record_source_failure();throw;}
}
void NativeMeshSectionLifetimeFrame::begin_outer_call(){
    require_healthy();require_owned(!impl_->domain.impl_->closing.load(),"Section operation after owning close fence");
    impl_->outer_calls.fetch_add(1,std::memory_order_acq_rel);exact_.outer_calls_.fetch_add(1,std::memory_order_acq_rel);
}
void NativeMeshSectionLifetimeFrame::end_outer_call() noexcept {
    impl_->outer_calls.fetch_sub(1,std::memory_order_release);exact_.outer_calls_.fetch_sub(1,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::begin_call_activity(Call& call) noexcept {
    for(auto& observation:call.impl_->children)observation.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
    call.impl_->reference_drop.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
    call.impl_->direct_zero.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
}
void NativeMeshSectionLifetimeFrame::end_call_activity(Call& call) noexcept {
    for(auto& observation:call.impl_->children)observation.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
    call.impl_->reference_drop.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
    call.impl_->direct_zero.active_invocations_.fetch_sub(1,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::fail_call_owned(Call& call) noexcept {
    call.impl_->failed.store(true,std::memory_order_release);
    for(auto& observation:call.impl_->children)NativeRenderObservedZeroAccess::fail_frame(observation.cpp_zero_frame);
    NativeRenderObservedZeroAccess::fail_frame(call.impl_->reference_drop.cpp_zero_frame);
    NativeRenderObservedZeroAccess::fail_frame(call.impl_->direct_zero);
}
NativeMeshSectionLifetimeFrame::Call& NativeMeshSectionLifetimeFrame::prepare_call(SourceScope& source){
    require_healthy();require_owned(!source.ended_&&!source.failed_,"Section Call requires actual live healthy outer Source scope");
    try{
        auto pending=std::unique_ptr<Call>(new Call(*this,false));auto* call=pending.get();
        {std::lock_guard<std::mutex> lock(impl_->calls_mutex);
            require_owned(!impl_->domain.impl_->closing.load(),"Section Call reservation crossed owning close fence");
            call->impl_->next=impl_->calls;impl_->calls=call;(void)pending.release();}
        begin_outer_call();begin_call_activity(*call);
        call->impl_->source=&source;call->impl_->source_active.store(true,std::memory_order_release);
        call->impl_->source_next=source.first_;source.first_=call;
        return *call;
    }catch(...){record_source_failure();throw;}
}
void NativeMeshSectionLifetimeFrame::begin_operation(Call& call,Word operation){
    require_healthy();
    require_owned(&call.impl_->frame==this && (call.impl_->terminal?operation==static_cast<Word>(Operation::scalar):
        call.impl_->source_active.load(std::memory_order_acquire)),"Section needs its own fresh pre-effect caller record");
    Word expected=0;
    require_owned(call.impl_->operation.compare_exchange_strong(expected,operation),"Section caller observation cannot reset/replay");
    if(operation==static_cast<Word>(Operation::create)||operation==static_cast<Word>(Operation::copy)){
        expected=0;require_owned(impl_->producer.compare_exchange_strong(expected,operation),"Section producer cannot adopt/replay a born slot");
    }else{
        require_owned(impl_->reference!=nullptr,"Section operation has no genuine canonical");
        switch(static_cast<Operation>(operation)){
        case Operation::material: case Operation::stream: case Operation::layout:
        case Operation::generator: case Operation::append: case Operation::indexed:
        case Operation::clear_streams: case Operation::clear_resources: case Operation::rebuild:
            require_live_payload_borrow();break;
        default: require_same_canonical(*impl_->reference);break;
        }
    }
}
void NativeMeshSectionLifetimeFrame::release_child(Call& call,NativeRenderActualOwners& owners,void* actual,std::size_t site){
    require_owned(&call.impl_->frame==this && &owners==&impl_->domain.impl_->environment.retained_owners&&site<child_sites,
        "Section child release requires its authentic call/site and SAME actual registry");
    release_native_render_actual_owner_observed(owners,actual,call.impl_->children[site]);
}
void NativeMeshSectionLifetimeFrame::native_allocation_entered() noexcept {
    if(impl_->allocation_entered.exchange(true))record_source_failure();
}
void NativeMeshSectionLifetimeFrame::native_allocation_returned(void* actual){
    require_owned(impl_->allocation_entered.load()&&!impl_->allocation_returned.exchange(true),"Section allocator return cannot be fabricated/replayed");
    impl_->actual=static_cast<NativeMeshSectionStorage*>(actual);
    if(actual)impl_->domain.impl_->owners.prepare_exact_record(exact_,actual);
}
void NativeMeshSectionLifetimeFrame::native_constructor_entered(void* actual){
    require_owned(impl_->allocation_returned.load()&&actual&&actual==impl_->actual&&!impl_->constructor_entered.exchange(true),
        "Section constructor needs its own genuine allocation return");
}
void NativeMeshSectionLifetimeFrame::cpp_prefix_born(NativeMeshSectionStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->constructor_entered.load()||impl_->prefix_born.exchange(true))record_source_failure();
}
void NativeMeshSectionLifetimeFrame::native_count1_born(NativeMeshSectionStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->prefix_born.load()||impl_->count_born.exchange(true)){record_source_failure();return;}
    impl_->count_home=&actual.references_04; // address only, AFTER the original actual store1
}
void NativeMeshSectionLifetimeFrame::native_constructor_returned(NativeMeshSectionStorage& actual) noexcept {
    if(&actual!=impl_->actual||!impl_->count_born.load()||impl_->native_ctor_returned.exchange(true)){record_source_failure();return;}
    impl_->creator_outstanding.store(true,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::source_constructor_escaped() noexcept {
    impl_->ctor_escaped.store(true,std::memory_order_release);record_source_failure();
}
bool NativeMeshSectionLifetimeFrame::may_select_constructor_raw_return() const noexcept {
    return impl_->constructor_entered.load()&&impl_->ctor_escaped.load(std::memory_order_acquire)&&
        !impl_->native_ctor_returned.load()&&!impl_->selected_consumed.load();
}
void NativeMeshSectionLifetimeFrame::selected_constructor_raw_return(void* selected,NativeMeshSectionPool& pool){
    // ONLY the real caller's existing selected copy-exception continuation may
    // enter. No failed producer invents a raw return or full resource cleanup.
    if(!may_select_constructor_raw_return()||impl_->selected_consumed.exchange(true)){
        record_source_failure();throw std::logic_error("Section selected raw-return replay/wrong continuation");}
    impl_->selected_operand=selected;impl_->selected_pool=&pool;
    const bool same=selected==impl_->actual && &pool==&impl_->domain.impl_->environment.pool_010901d4;
    const bool can_end=same&&impl_->prefix_born.load()&&!impl_->published.load()&&exact_.borrows_.load()==0;
    if(can_end){
        impl_->actual->~NativeMeshSectionStorage();
        impl_->partial_cpp_ended.store(true,std::memory_order_release);
        impl_->domain.impl_->owners.deactivate_exact_record_from_partial_cpp_end(exact_);
    }
    impl_->selected_return_entered.store(true,std::memory_order_release);
    try{
        return_native_mesh_section_slot_00b85b20(pool,selected);
        if(same)impl_->selected_return_normal_same_slot.store(true,std::memory_order_release);
    }catch(...){record_source_failure();throw;}
    // Failed graph and exact membership remain retained. Partial is no normal
    // Native death, retire/close/cold cancellation, or child-credit refund.
}
void NativeMeshSectionLifetimeFrame::publish_after_native_constructor(){
    require_healthy();require_owned(impl_->native_ctor_returned.load()&&impl_->ctor_return_captured.load()&&
        impl_->constructor_return_value==impl_->actual&&!impl_->published.load(),"Section canonical needs captured complete constructor return");
    auto& domain=*impl_->domain.impl_;
    impl_->reference=::new(&impl_->reference_cell)NativeMeshSectionReference(*impl_->actual,domain.environment,*this);
    impl_->reference_live=true;impl_->reference->bind_ordinary_section(*this);
    impl_->reference->validate_ordinary_target();
    domain.owners.publish_exact_record(exact_,*impl_->reference);impl_->published.store(true,std::memory_order_release);
}
NativeMeshSectionStorage* NativeMeshSectionLifetimeFrame::produce(Call& call,const NativeMeshSectionStorage* source){
    begin_operation(call,static_cast<Word>(source?Operation::copy:Operation::create));
    const OperationScope operation(*this,call);
    native_allocation_entered();
    void* const raw=allocate_native_mesh_section_slot_00b85ee0(impl_->domain.impl_->environment.pool_010901d4);
    native_allocation_returned(raw);
    if(raw){
        native_constructor_entered(raw);
        NativeMeshSectionStorage* returned;
        try{
            if(source)returned=NativeMeshSectionLifetimeAccess::copy(raw,*source,impl_->domain.impl_->environment.retained_owners,
                impl_->domain.impl_->bounds,*this,call);
            else returned=NativeMeshSectionLifetimeAccess::construct(raw,impl_->domain.impl_->bounds,*this,call);
        }catch(...){source_constructor_escaped();throw;}
        // Capture the actual WHOLE constructor return before ANY fallible
        // canonical/profile/registry host publication. No catch loses its debt.
        impl_->constructor_return_value=returned;impl_->ctor_return_captured.store(true,std::memory_order_release);
        require_owned(returned==impl_->actual,"Section whole constructor returned another slot");
        publish_after_native_constructor();
    }else require_owned(!impl_->constructor_entered.load()&&!impl_->prefix_born.load(),"NULL Section cannot issue Native birth");
    require_healthy();impl_->producer_returned.store(true,std::memory_order_release);
    call.impl_->returned.store(true,std::memory_order_release);return impl_->actual;
}
void NativeMeshSectionLifetimeFrame::require_same_canonical(RenderCommandReference& reference) const {
    require_healthy();require_owned(impl_->actual&&impl_->reference&&impl_->published.load()&&impl_->native_ctor_returned.load()&&
        impl_->producer_returned.load()&&impl_->reference==&reference&&reference.ordinary_section_==this&&
        !impl_->scalar_entered.load()&&!impl_->typed_ended.load()&&!impl_->partial_cpp_ended.load()&&
        static_cast<void*>(&reference.reference_count)==impl_->count_home,
        "Section requires SAME privately born live canonical/count address");
}
void NativeMeshSectionLifetimeFrame::require_live_payload_borrow() const {
    require_healthy();
    // An actual callback can own an ongoing Native borrow while Section+4 is
    // already zero. Do not fabricate/require a separate Section count credit,
    // sample that count, or turn scalar_entered into a NoReentry restriction.
    // The caller owns that real borrow and its full SourceScope independently.
    // Only field/resource mutators use this guard; count/zero/scalar actions
    // retain their stricter private canonical/action guard above.
    require_owned(impl_->actual&&impl_->reference&&impl_->prefix_born.load()&&impl_->count_born.load()&&
        impl_->native_ctor_returned.load()&&impl_->ctor_return_captured.load()&&impl_->published.load()&&
        impl_->producer_returned.load()&&impl_->reference->ordinary_section_==this&&
        static_cast<void*>(&impl_->reference->reference_count)==impl_->count_home&&
        (!impl_->scalar_entered.load()||impl_->payload_active.load(std::memory_order_acquire))&&
        !impl_->payload_returned.load()&&!impl_->typed_ended.load()&&!impl_->partial_cpp_ended.load()&&
        !impl_->pool_return_entered.load()&&!impl_->pool_return_normal.load()&&!impl_->live_storage_debt.load(),
        "Section mutation requires SAME genuinely born still-live payload and actual caller Native borrow");
}
NativeMeshSectionReference& NativeMeshSectionLifetimeFrame::canonical(){
    require_owned(impl_->reference!=nullptr,"Section canonical unavailable");require_same_canonical(*impl_->reference);return *impl_->reference;
}
void NativeMeshSectionLifetimeFrame::begin_cpp_count_use(RenderCommandReference& reference){
    require_same_canonical(reference);exact_.borrows_.fetch_add(1,std::memory_order_acq_rel);
    try{require_same_canonical(reference);}catch(...){record_source_failure();end_cpp_count_use();throw;}
}
void NativeMeshSectionLifetimeFrame::end_cpp_count_use() noexcept {exact_.borrows_.fetch_sub(1,std::memory_order_release);}
void NativeMeshSectionLifetimeFrame::hold_observed_zero(NativeRenderObservedZeroFrame& caller,RenderCommandReference& reference){
    begin_outer_call();try{
        require_same_canonical(reference);bool expected=false;
        require_owned(observed_zero_hold_.claimed_.compare_exchange_strong(expected,true),"Section zero hold cannot replay");
        observed_zero_hold_.section_=this;NativeRenderObservedZeroAccess::attach_hold(caller,observed_zero_hold_);
    }catch(...){record_source_failure();end_outer_call();throw;}
}
void NativeMeshSectionLifetimeFrame::release_observed_zero_hold(bool failed) noexcept {if(failed)record_source_failure();end_outer_call();}
void NativeMeshSectionLifetimeFrame::invoke_observed_zero(void* actual,RenderCommandReference& reference){
    const NativeRenderObservedZeroAccess::Invocation active(terminal_zero_frame_);
    begin_outer_call();try{
        require_same_canonical(reference);require_owned(actual==impl_->actual&&!impl_->zero_armed.exchange(true),"Section actual-zero target/action mismatch");
        reference.release_zero_references();require_healthy();end_outer_call();
    }catch(...){impl_->zero_armed.store(false);record_source_failure();end_outer_call();throw;}
}
void NativeMeshSectionLifetimeFrame::ordinary_virtual_zero(RenderCommandReference& reference){
    require_same_canonical(reference);require_owned(impl_->zero_armed.exchange(false),"Section has no authentic armed actual-zero action");
    impl_->reference->validate_ordinary_target();impl_->virtual_flags_home=1;
    (void)scalar(*impl_->terminal,&impl_->virtual_flags_home);
}
void NativeMeshSectionLifetimeFrame::mutate(Call& call,Word kind,void* operand,std::int32_t index,NativeMeshSectionLayoutServices* services){
    begin_operation(call,kind);const OperationScope operation(*this,call);
    NativeMeshSectionLifetimeAccess::mutate(*impl_->actual,impl_->domain.impl_->environment.retained_owners,
        kind,operand,index,services,*this,call);
    require_healthy();call.impl_->returned.store(true,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::release_creator(Call& call,long (__stdcall* decrement)(volatile long*)){
    begin_operation(call,static_cast<Word>(Operation::creator));const OperationScope operation(*this,call);
    require_owned(decrement&&impl_->creator_outstanding.load()&&!impl_->creator_release_entered.exchange(true),
        "Section creator drop needs actual unconsumed genuine constructor credit/captured helper");
    const NativeRenderObservedZeroAccess::CppCountUse count_use(*impl_->reference);
    const long result=decrement(reinterpret_cast<volatile long*>(impl_->count_home));
    impl_->creator_result=result;impl_->creator_release_returned.store(true,std::memory_order_release);
    impl_->creator_outstanding.store(false,std::memory_order_release);
    if(result==0){
        impl_->creator_zero_entered.store(true,std::memory_order_release);
        // SAME already-owned canonical: no second resolve/decrement/count read.
        NativeRenderObservedZeroAccess::invoke(*impl_->reference,impl_->actual,call.impl_->direct_zero);
        impl_->creator_zero_returned.store(true,std::memory_order_release);
    }
    require_healthy();call.impl_->returned.store(true,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::release_reference(Call& call){
    begin_operation(call,static_cast<Word>(Operation::reference));const OperationScope operation(*this,call);
    // The actual caller separately owns THIS real retain (e.g. CURRENT
    // existing Section acquire). It is not a second constructor-credit claim.
    // This authentic helper supplies the sole decrement and zero-only resolve;
    // its external per-invocation receiver outlives the complete caller tail.
    release_native_render_actual_owner_observed(impl_->domain.impl_->environment.retained_owners,
        impl_->actual,call.impl_->reference_drop);
    require_healthy();call.impl_->returned.store(true,std::memory_order_release);
}
void NativeMeshSectionLifetimeFrame::payload(Call& call){
    begin_operation(call,static_cast<Word>(Operation::direct_payload));
    require_owned(!impl_->scalar_entered.exchange(true),"Section payload cannot replay");
    const OperationScope operation(*this,call);
    {
        const PayloadScope payload_scope(*this);
        NativeMeshSectionLifetimeAccess::destroy(*impl_->actual,impl_->domain.impl_->environment.retained_owners,*this,call);
    }
    impl_->payload_returned.store(true,std::memory_order_release);
    impl_->live_storage_debt.store(true,std::memory_order_release);
    impl_->domain.impl_->owners.withhold_exact_record_for_live_storage_debt(exact_);
    require_healthy();call.impl_->returned.store(true,std::memory_order_release);
}
NativeMeshSectionStorage* NativeMeshSectionLifetimeFrame::scalar(Call& call,const void* flags_home){
    begin_operation(call,static_cast<Word>(Operation::scalar));
    require_owned(flags_home&&!impl_->scalar_entered.exchange(true),"Section scalar needs genuine one-shot action/live flags HOME");
    const OperationScope operation(*this,call);auto* const original=impl_->actual;
    {
        const PayloadScope payload_scope(*this);
        NativeMeshSectionLifetimeAccess::destroy(*original,impl_->domain.impl_->environment.retained_owners,*this,call);
    }
    impl_->payload_returned.store(true,std::memory_order_release);impl_->creator_outstanding.store(false,std::memory_order_release);
    // Whole payload/base cleanup FIRST, then the sole late low BYTE read.
    const auto flags=late_flags(flags_home);impl_->captured_flags=flags;impl_->flags_read.store(true,std::memory_order_release);
    if(flags&1u){
        original->~NativeMeshSectionStorage();impl_->typed_ended.store(true,std::memory_order_release);
        impl_->domain.impl_->owners.deactivate_exact_record_from_native_end(exact_);
        impl_->pool_return_entered.store(true,std::memory_order_release);
        impl_->domain.impl_->environment.pool_010901d4.return_slot_00b859b0(original);
        impl_->pool_return_normal.store(true,std::memory_order_release);
    }else{
        impl_->live_storage_debt.store(true,std::memory_order_release);
        impl_->domain.impl_->owners.withhold_exact_record_for_live_storage_debt(exact_);
    }
    // Pool+60 is a separate live slab ID. From return onward ONLY captured
    // identities/external metadata are touched; no payload/profile/count read.
    impl_->scalar_body_completed.store(true,std::memory_order_release);
    if(flags&1u){impl_->domain.impl_->owners.retire_exact_record(exact_);impl_->retired.store(true,std::memory_order_release);}
    require_healthy();impl_->scalar_source_returned.store(true,std::memory_order_release);
    call.impl_->returned.store(true,std::memory_order_release);return original;
}
void NativeMeshSectionLifetimeFrame::preflight_close() const {
    require_healthy();auto* caller=observed_zero_hold_.frame_;const unsigned held=caller?1u:0u;
    require_owned(impl_->attached&&impl_->counted&&exact_.registry_==&impl_->domain.impl_->owners&&
        exact_.owning_identity_==this&&exact_.retained_member_&&!exact_.poisoned_.load()&&
        !exact_.partial_cpp_ended_.load()&&!exact_.unborn_abandoned_.load(),
        "Section close requires SAME nonfailed normal/cold exact membership");
    require_owned(!impl_->payload_active.load()&&terminal_zero_frame_.active_invocations_.load()==0&&(!caller||caller->active_invocations_.load()==0)&&
        impl_->outer_calls.load()==held&&exact_.outer_calls_.load()==held&&exact_.borrows_.load()==0,
        "Section close crossed active operation/count/whole-caller fence");
    for(auto* call=impl_->calls;call;call=call->impl_->next){
        require_owned(!call->impl_->source_active.load()&&!call->impl_->failed.load()&&
            call->impl_->direct_zero.active_invocations_.load()==0,"Section Source/creator caller still active/failed");
        require_owned(call->impl_->operation.load()==0||call->impl_->returned.load(),"Section invocation return/health remains unknown");
        const auto& drop=call->impl_->reference_drop;
        require_owned(drop.cpp_zero_frame.active_invocations_.load()==0&&(!drop.entered||
            (drop.decremented&&drop.returned&&(!drop.zero_dispatch_entered||drop.zero_dispatch_returned))),
            "Section retained-credit decrement/zero/whole-caller return remains unknown");
        for(auto& observation:call->impl_->children){
            require_owned(observation.cpp_zero_frame.active_invocations_.load()==0,"Section child caller still active");
            require_owned(!observation.entered||(observation.decremented&&observation.returned&&
                (!observation.zero_dispatch_entered||observation.zero_dispatch_returned)),"Section child decrement/zero/return remains unknown");
        }
    }
    require_owned(!impl_->creator_release_entered.load()||impl_->creator_release_returned.load(),"Section creator decrement return unknown");
    require_owned(!impl_->creator_zero_entered.load()||impl_->creator_zero_returned.load(),"Section creator terminal return unknown");
    const bool cold=impl_->producer.load()==0&&!impl_->allocation_entered.load()&&!impl_->prefix_born.load();
    const bool normal_null=impl_->producer_returned.load()&&impl_->allocation_returned.load()&&!impl_->actual&&
        !impl_->constructor_entered.load()&&!impl_->prefix_born.load();
    const bool complete=impl_->scalar_source_returned.load()&&impl_->payload_returned.load()&&impl_->typed_ended.load()&&
        impl_->pool_return_normal.load()&&impl_->retired.load()&&!impl_->creator_outstanding.load()&&!impl_->live_storage_debt.load();
    require_owned(cold||normal_null||complete,"Section close retains live/flags0/partial/unknown/physical/Source debt");
    require_owned(exact_.availability_.load()==NativeRenderActualOwnerRegistry::ExactRecord::Availability::unavailable&&
        ((complete&&exact_.retired_&&exact_.normal_native_dead_.load())||
        ((cold||normal_null)&&!exact_.actual_key_&&!exact_.canonical_&&!exact_.retired_&&!exact_.normal_native_dead_.load())),
        "Section exact physical/end/retirement or genuine never-born fact is incomplete");
}
void NativeMeshSectionLifetimeFrame::close_after_host_quiescence(){
    preflight_close();auto* caller=observed_zero_hold_.frame_;
    const bool cold=impl_->producer.load()==0;const bool normal_null=impl_->producer_returned.load()&&!impl_->actual;
    try{
        for(auto* call=impl_->calls;call;call=call->impl_->next){
            require_owned(NativeRenderObservedZeroAccess::end_frame(call->impl_->reference_drop.cpp_zero_frame,false),
                "Section retained-credit caller did not quiesce");
            for(auto& observation:call->impl_->children)require_owned(NativeRenderObservedZeroAccess::end_frame(observation.cpp_zero_frame,false),
                "Section child caller did not quiesce");
            require_owned(NativeRenderObservedZeroAccess::end_frame(call->impl_->direct_zero,false),"Section creator caller did not quiesce");
        }
        if(caller)require_owned(NativeRenderObservedZeroAccess::end_frame(*caller,false),"Section outer zero caller still active");
        if(caller!=&terminal_zero_frame_)require_owned(NativeRenderObservedZeroAccess::end_frame(terminal_zero_frame_,false),"Section terminal caller still active");
        require_healthy();require_owned(impl_->outer_calls.load()==0&&exact_.outer_calls_.load()==0&&exact_.borrows_.load()==0,
            "Section still has actual C++ users after caller drain");
        if(cold||normal_null)impl_->domain.impl_->owners.cancel_exact_record_without_payload(exact_);
        else impl_->domain.impl_->owners.close_exact_record_at_quiescence(exact_);
        impl_->attached=false;impl_->closed.store(true,std::memory_order_release);
        if(impl_->counted){impl_->domain.impl_->retention.retained_.fetch_sub(1,std::memory_order_release);
            impl_->domain.impl_->retained.fetch_sub(1,std::memory_order_release);impl_->counted=false;}
    }catch(...){record_source_failure();throw;}
}
void NativeMeshSectionLifetimeDomain::close_after_host_quiescence(){
    try{
        require_owned(!impl_->failed.load(),"Failed Section graph must remain retained");
        {std::lock_guard<std::mutex> lock(impl_->mutex);impl_->closing.store(true,std::memory_order_release);}
        // Domain-wide first pass BEFORE the first child/caller hold unlink.
        // Actual cross-domain owner quiescence/preflight remains independent.
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next)if(!frame->impl_->closed.load())frame->preflight_close();
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next)if(!frame->impl_->closed.load())frame->close_after_host_quiescence();
        require_owned(impl_->retained.load()==0,"Section domain retains actual obligations");
    }catch(...){NativeMeshSectionLifetimeFrame::record_domain_failure(*this);throw;}
}

NativeMeshSectionStorage* create_native_mesh_section_00533fa0(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call){return frame.produce(call,nullptr);}
NativeMeshSectionStorage* create_native_mesh_section_clone_00b85ef0(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,const NativeMeshSectionStorage& source){return frame.produce(call,&source);}
void release_native_mesh_section_creator(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,long (__stdcall* decrement)(volatile long*)){frame.release_creator(call,decrement);}
void release_native_mesh_section_reference(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call){frame.release_reference(call);}
NativeMeshSectionStorage* delete_native_mesh_section_00b86690(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,const void* flags){return frame.scalar(call,flags);}
void destroy_native_mesh_section_00b86420(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call){frame.payload(call);}
void set_native_mesh_section_material_00b864c0(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,void* material){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::material),material,0,nullptr);}
void set_native_mesh_section_vertex_stream_00b86500(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,std::int32_t index,void* stream){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::stream),stream,index,nullptr);}
void set_native_mesh_section_vertex_layout_00b86650(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,void* layout){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::layout),layout,0,nullptr);}
void set_native_mesh_section_instance_generator_binding_00b417e0(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,void* generator){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::generator),generator,0,nullptr);}
void append_native_mesh_section_vertex_stream_00b85b80(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,void* stream){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::append),stream,0,nullptr);}
void set_native_mesh_section_indexed_00b85600(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,std::uint8_t indexed){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::indexed),nullptr,indexed,nullptr);}
void clear_native_mesh_section_vertex_streams_00b86550(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::clear_streams),nullptr,0,nullptr);}
void clear_native_mesh_section_resources_00b86390(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::clear_resources),nullptr,0,nullptr);}
void rebuild_native_mesh_section_vertex_layout_00b865a0(NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeFrame::Call& call,void* mesh,NativeMeshSectionLayoutServices& services){frame.mutate(call,static_cast<Word>(NativeMeshSectionLifetimeFrame::Operation::rebuild),mesh,0,&services);}
} // namespace bsp
