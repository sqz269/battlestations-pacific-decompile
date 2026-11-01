#include "bsp/native_post_effect_parent_lifetime.hpp"
#include "bsp/native_post_effect_construction.hpp"
#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_bright_pass_initializer.hpp"
#include "bsp/native_post_effect_dust_initializer.hpp"
#include "bsp/native_post_effect_ordinary_lifetime.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/native_material_lifetime.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/native_mesh_section_lifetime.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/native_render_observed_zero.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <exception>
#include <optional>
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

// Impl owns this nonmoving record BEFORE any fallible Section reservation.
// Frames/Calls belong to the selected Section Domain. Once their live loans
// end, the saved addresses are opaque history only, never late dereferences.
struct NativePostEffectParentFrame::SectionConstruction final {
    NativeMeshSectionLifetimeSourceScope* source_live{};
    const NativeMeshSectionLifetimeSourceScope* source_identity{};
    NativePostEffectParentFrame* source_next{};
    NativeMeshSectionLifetimeFrame* frame{};
    NativeMeshSectionLifetimeCall* create{};
    NativeMeshSectionLifetimeCall* material{};
    NativeMeshSectionLifetimeCall* rebuild{};
    NativeMeshSectionLifetimeCall* creator{};
    NativeMeshSectionStorage* constructor_return_value{};
    NativeMeshSectionReference* canonical{};
    bool source_ended{}, source_failed{}, retirement_pending{}, retired{};
    bool whole_constructor_returned{}, producer_returned{}, normal_null{};
    bool material_returned{}, rebuild_returned{}, creator_returned{};
    bool caller_end_entered{}, caller_ended{}, block_attached{}, borrowed{};
    SectionConstruction() = default;
    SectionConstruction(const SectionConstruction&) = delete;
    SectionConstruction& operator=(const SectionConstruction&) = delete;
    SectionConstruction(SectionConstruction&&) = delete;
    SectionConstruction& operator=(SectionConstruction&&) = delete;
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
    std::atomic<bool> constructor_scope_active{false}, payload_scope_active{false};
    std::atomic<bool> material_setter_entered{false}, material_setter_returned{false};
    std::atomic<bool> normal_null_recorded{false};
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
    std::unique_ptr<SectionConstruction> section;
    std::unique_ptr<InitializerConstruction> initializer;
    Impl(NativePostEffectParentDomain& d, NativePostEffect20ConstructionBlock& b)
        : domain(d), kind(Kind::parent20), block20(&b) {}
    Impl(NativePostEffectParentDomain& d, NativePostEffectConstructionBlock& b)
        : domain(d), kind(Kind::parent24), block24(&b) {}
    ~Impl(); // InitializerConstruction must be complete at destruction.
};

// Only the admitted producer owns this setter receiver's mutable heads.
// Optional emplacement in the wrapper happens AFTER its producer CAS and the
// object lives through the wrapper's complete publication/health/catch fence.
class NativePostEffectParentFrame::MaterialConstructorScope final {
public:
    explicit MaterialConstructorScope(NativePostEffectParentFrame& frame)
        :frame_(frame),material_(frame.material_frame_),uncaught_(std::uncaught_exceptions()) {
        frame_.impl_->constructor_outer.store(true,std::memory_order_release);
        try {
            if(material_)material_->begin_outer_call(); // C++ use only, no Native retain
            frame_.material_parameter_release_.cpp_zero_frame.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
            frame_.impl_->constructor_scope_active.store(true,std::memory_order_release);
        }catch(...){frame_.impl_->constructor_outer.store(false,std::memory_order_release);frame_.record_source_failure();throw;}
    }
    ~MaterialConstructorScope() noexcept {
        if(std::uncaught_exceptions()>uncaught_||frame_.requires_process_retention()){
            frame_.record_source_failure();
            NativeRenderObservedZeroAccess::fail_frame(frame_.material_parameter_release_.cpp_zero_frame);
        }
        frame_.impl_->constructor_scope_active.store(false,std::memory_order_release);
        frame_.impl_->constructor_outer.store(false,std::memory_order_release);
        frame_.material_parameter_release_.cpp_zero_frame.active_invocations_.fetch_sub(1,std::memory_order_release);
        if(material_)material_->end_outer_call();
    }
    MaterialConstructorScope(const MaterialConstructorScope&)=delete;
    MaterialConstructorScope& operator=(const MaterialConstructorScope&)=delete;
private:
    NativePostEffectParentFrame& frame_;
    NativeMaterialFrame* const material_;
    const int uncaught_;
};

// Retained before initializer admission. The automatic Caller guard owns only
// its real invocation; this record and all observer targets outlive that guard.
struct NativePostEffectParentFrame::InitializerConstruction final {
    NativeBrightPassInitializationBlock* bright_live{};
    const NativeBrightPassInitializationBlock* bright_identity{};
    NativePostEffectDustBlock* dust_live{};
    const NativePostEffectDustBlock* dust_identity{};
    NativeMeshSectionLifetimeSourceScope* source_live{};
    const NativeMeshSectionLifetimeSourceScope* source_identity{};
    NativePostEffectParentFrame* source_next{};
    bool owns_observer{}, source_ended{}, source_failed{};
    bool admitted{}, outer_entered{}, finished{}, source_returned{}, detached{};
    bool constructor_call_entered{}, constructor_call_returned{}, constructor_call_escaped{};
    bool raw_free_entered{}, raw_free_returned{}, raw_free_same{}, prefix_ended{};
    int uncaught{};
    void* constructor_return_value{};
    void* raw_free_operand{};
    std::optional<NativeRenderObservedZeroAccess::Invocation> terminal_invocation;
    std::optional<MaterialConstructorScope> material_scope;
    InitializerConstruction(NativeBrightPassInitializationBlock&, NativeMeshSectionLifetimeSourceScope&);
    InitializerConstruction(NativePostEffectDustBlock&, NativeMeshSectionLifetimeSourceScope&);
    InitializerConstruction(const InitializerConstruction&) = delete;
    InitializerConstruction& operator=(const InitializerConstruction&) = delete;
    InitializerConstruction(InitializerConstruction&&) = delete;
    InitializerConstruction& operator=(InitializerConstruction&&) = delete;
};
NativePostEffectParentFrame::Impl::~Impl() = default;

NativePostEffectParentFrame::InitializerConstruction::InitializerConstruction(
    NativeBrightPassInitializationBlock& block, NativeMeshSectionLifetimeSourceScope& source)
    : bright_live(&block), bright_identity(&block), source_live(&source), source_identity(&source) {
    auto* frame=block.post_effect().parent_frame_;
    if(!frame)throw std::logic_error("Bright initializer lost its retained Parent attachment");
    frame->require_owned(frame->impl_->kind==Kind::parent20&&frame->impl_->block20==&block.post_effect()&&
        !source.ended_&&!source.failed_,"Bright initializer requires SAME cold Parent and live external Source");
    if(frame->impl_->section){
        auto& s=*frame->impl_->section;
        frame->require_owned(s.source_live==&source&&s.source_identity==&source&&!s.source_ended,
            "Bright initializer must reuse its SAME Section Source observer");
    }else{
        source_next=source.parents_;source.parents_=frame;owns_observer=true;
    }
}
NativePostEffectParentFrame::InitializerConstruction::InitializerConstruction(
    NativePostEffectDustBlock& block, NativeMeshSectionLifetimeSourceScope& source)
    : dust_live(&block), dust_identity(&block), source_live(&source), source_identity(&source) {
    auto* frame=block.child().parent_frame_;
    if(!frame)throw std::logic_error("Dust initializer lost its retained Parent attachment");
    frame->require_owned(frame->impl_->kind==Kind::parent24&&frame->impl_->block24==&block.child()&&
        !source.ended_&&!source.failed_,"Dust initializer requires SAME cold Parent and live external Source");
    if(frame->impl_->section){
        auto& s=*frame->impl_->section;
        frame->require_owned(s.source_live==&source&&s.source_identity==&source&&!s.source_ended,
            "Dust initializer must reuse its SAME Section Source observer");
    }else{
        source_next=source.parents_;source.parents_=frame;owns_observer=true;
    }
}
// Only the scalar CAS winner owns payload heads. It does not pin/substitute
// factory A for the actual CURRENT target B; Common holds B's own real Frame.
class NativePostEffectParentFrame::MaterialPayloadScope final {
public:
    explicit MaterialPayloadScope(NativePostEffectParentFrame& frame) noexcept
        :frame_(frame),uncaught_(std::uncaught_exceptions()) {
        frame_.terminal_zero_frame_.active_invocations_.fetch_add(1,std::memory_order_acq_rel);
        frame_.impl_->payload_scope_active.store(true,std::memory_order_release);
    }
    ~MaterialPayloadScope() noexcept {
        if(std::uncaught_exceptions()>uncaught_||frame_.requires_process_retention()){
            frame_.record_source_failure();NativeRenderObservedZeroAccess::fail_frame(frame_.terminal_zero_frame_);
        }
        frame_.impl_->payload_scope_active.store(false,std::memory_order_release);
        frame_.terminal_zero_frame_.active_invocations_.fetch_sub(1,std::memory_order_release);
    }
    MaterialPayloadScope(const MaterialPayloadScope&)=delete;
    MaterialPayloadScope& operator=(const MaterialPayloadScope&)=delete;
private:
    NativePostEffectParentFrame& frame_;
    const int uncaught_;
};

NativePostEffectParentDomain::NativePostEffectParentDomain(
    NativePostEffect20ConstructionContext& c20, NativePostEffectConstructionContext& c24,
    const NativePostEffectOrdinaryLifetimeBinding& binding,
    NativeRenderActualOwnerRegistry& owners, NativeMeshRetentionState& retention)
    : impl_(std::make_unique<Impl>(c20,c24,binding,owners,retention)) {
    require_owned(&c20.destruction.actual_owners == static_cast<NativeRenderActualOwners*>(&owners) &&
        &c24.destruction.actual_owners == static_cast<NativeRenderActualOwners*>(&owners) &&
        c20.destruction.ordinary_models == &binding.models &&
        c24.destruction.ordinary_models == &binding.models,
        "Parent domain requires the SAME actual registry and immutable ordinary Model binding");
    if(binding.materials){
        binding.materials->require_parent_binding(c20.materials,owners,
            c20.destruction.actual_material_profile_00d5e520,retention);
        binding.materials->require_parent_binding(c24.materials,owners,
            c24.destruction.actual_material_profile_00d5e520,retention);
    }
    if(binding.sections){
        binding.sections->require_owner_binding(c20.sections,owners,
            c20.section_bounds_w_00ce4970,retention);
        binding.sections->require_owner_binding(c24.sections,owners,
            c24.section_bounds_w_00ce4970,retention);
        require_owned(&c20.section_bounds_w_00ce4970 == &c20.mesh_constants.maximum_00ce4970 &&
            &c24.section_bounds_w_00ce4970 == &c24.mesh_constants.maximum_00ce4970,
            "Parent Section bounds must be the SAME live Mesh maximum reference");
    }
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
        impl_->domain.impl_->failed.load(std::memory_order_acquire) || impl_->closed.load() ||
        impl_->domain.impl_->retention.requires_process_retention()) {
        const_cast<NativePostEffectParentFrame*>(this)->record_source_failure();
        throw std::logic_error("Parent record retains failed/closed Source custody");
    }
}
bool NativePostEffectParentDomain::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire) || impl_->retention.requires_process_retention();
}
bool NativePostEffectParentDomain::has_retained_records() const noexcept {
    return impl_->retained.load(std::memory_order_acquire) != 0;
}
bool NativePostEffectParentFrame::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire) || impl_->domain.requires_process_retention();
}

NativePostEffectParentFrame& NativePostEffectParentDomain::prepare20(
    NativePostEffect20ConstructionBlock& block) {
    try {
        require_owned(!impl_->binding.sections,"Selected Parent Sections require the actual external Source scope");
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
        require_owned(!impl_->binding.sections,"Selected Parent Sections require the actual external Source scope");
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
NativePostEffectParentFrame& NativePostEffectParentDomain::prepare20(
    NativePostEffect20ConstructionBlock& block, NativeMeshSectionLifetimeSourceScope& source) {
    if(!impl_->binding.sections)return prepare20(block);
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
        frame->prepare_section_caller(source);
        return *frame;
    } catch (...) { NativePostEffectParentFrame::record_domain_failure(*this); throw; }
}
NativePostEffectParentFrame& NativePostEffectParentDomain::prepare24(
    NativePostEffectConstructionBlock& block, NativeMeshSectionLifetimeSourceScope& source) {
    if(!impl_->binding.sections)return prepare24(block);
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
        frame->prepare_section_caller(source);
        return *frame;
    } catch (...) { NativePostEffectParentFrame::record_domain_failure(*this); throw; }
}
NativePostEffectParentFrame& NativePostEffectParentDomain::prepare_initializer(
    NativeBrightPassInitializationBlock& caller, NativePostEffect20ConstructionContext& context,
    NativeMeshSectionLifetimeSourceScope& source) {
    NativePostEffectParentFrame* frame=nullptr;
    try {
        require_owned(&context==&impl_->c20&&caller.context_ && &caller.context_->post_effects==&context&&
            caller.phase_==NativeBrightPassInitializationBlock::Phase::preparing&&!caller.parent_frame_,
            "Bright initializer requires its exact Domain context and cold Caller");
        frame=&prepare20(caller.post_effect_,source);
        caller.parent_frame_=frame; // retained loan survives a later host allocation failure
        require_owned(!frame->impl_->initializer,"Bright initializer association cannot replay");
        frame->impl_->initializer=std::make_unique<NativePostEffectParentFrame::InitializerConstruction>(caller,source);
        return *frame;
    }catch(...){if(frame)frame->record_source_failure();else NativePostEffectParentFrame::record_domain_failure(*this);throw;}
}
NativePostEffectParentFrame& NativePostEffectParentDomain::prepare_initializer(
    NativePostEffectDustBlock& caller, NativePostEffectConstructionContext& context,
    NativeMeshSectionLifetimeSourceScope& source) {
    NativePostEffectParentFrame* frame=nullptr;
    try {
        require_owned(&context==&impl_->c24&&caller.context_ && &caller.context_->construction==&context&&
            caller.phase_==NativePostEffectDustBlock::Phase::preparing&&!caller.parent_frame_,
            "Dust initializer requires its exact Domain context and cold Caller");
        frame=&prepare24(caller.child_,source);
        caller.parent_frame_=frame;
        require_owned(!frame->impl_->initializer,"Dust initializer association cannot replay");
        frame->impl_->initializer=std::make_unique<NativePostEffectParentFrame::InitializerConstruction>(caller,source);
        return *frame;
    }catch(...){if(frame)frame->record_source_failure();else NativePostEffectParentFrame::record_domain_failure(*this);throw;}
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
        if(d.binding.materials){
            if(impl_->block20){
                require_owned(impl_->block20->material_frame_&&impl_->block20->material_factory_,"Parent20 lost its cold Material/factory loan");
                prepare_material_caller(*impl_->block20->material_frame_,*impl_->block20->material_factory_,d.c20.material_effects);
            }else{
                require_owned(impl_->block24->material_frame_&&impl_->block24->material_factory_,"Parent24 lost its cold Material/factory loan");
                prepare_material_caller(*impl_->block24->material_frame_,*impl_->block24->material_factory_,d.c24.material_effects);
            }
        }
    } catch (...) { record_source_failure(); throw; }
}
void NativePostEffectParentFrame::begin_initializer_call(NativeBrightPassInitializationBlock& caller) {
    require_healthy();auto* i=impl_->initializer.get();
    require_owned(i&&i->bright_live==&caller&&i->bright_identity==&caller&&!i->dust_identity&&
        impl_->kind==Kind::parent20&&impl_->block20==&caller.post_effect()&&
        caller.phase()==NativeBrightPassInitializationBlock::Phase::executing&&
        i->source_live&&!i->source_ended&&!i->source_failed&&!i->source_live->ended_&&!i->source_live->failed_&&
        !i->admitted&&!i->finished&&!i->detached,
        "Bright admission requires the SAME prepared Caller/Frame and real live Source");
    i->admitted=true;i->uncaught=std::uncaught_exceptions();
    try {
        require_owned(!impl_->producer_entered.exchange(true),"Initializer Parent producer cannot replay");
        i->terminal_invocation.emplace(terminal_zero_frame_);
        begin_outer_call();i->outer_entered=true;
        i->material_scope.emplace(*this);
    }catch(...){
        record_source_failure();
        if(i->outer_entered){i->outer_entered=false;end_outer_call();}
        i->material_scope.reset();i->terminal_invocation.reset();i->finished=true;throw;
    }
}
void NativePostEffectParentFrame::begin_initializer_call(NativePostEffectDustBlock& caller) {
    require_healthy();auto* i=impl_->initializer.get();
    require_owned(i&&i->dust_live==&caller&&i->dust_identity==&caller&&!i->bright_identity&&
        impl_->kind==Kind::parent24&&impl_->block24==&caller.child()&&
        caller.phase()==NativePostEffectDustBlock::Phase::executing&&
        i->source_live&&!i->source_ended&&!i->source_failed&&!i->source_live->ended_&&!i->source_live->failed_&&
        !i->admitted&&!i->finished&&!i->detached,
        "Dust admission requires the SAME prepared Caller/Frame and real live Source");
    i->admitted=true;i->uncaught=std::uncaught_exceptions();
    try {
        require_owned(!impl_->producer_entered.exchange(true),"Initializer Parent producer cannot replay");
        i->terminal_invocation.emplace(terminal_zero_frame_);
        begin_outer_call();i->outer_entered=true;
        i->material_scope.emplace(*this);
    }catch(...){
        record_source_failure();
        if(i->outer_entered){i->outer_entered=false;end_outer_call();}
        i->material_scope.reset();i->terminal_invocation.reset();i->finished=true;throw;
    }
}
void* NativePostEffectParentFrame::allocate_initializer_parent() {
    require_healthy();auto* i=impl_->initializer.get();
    require_owned(i&&i->admitted&&i->outer_entered&&!i->finished&&i->material_scope&&
        !i->constructor_call_entered&&!impl_->allocator_entered.load(),
        "Initializer supplier needs its actual admitted whole Caller scope");
    // allocate_parent saves actual/returned BEFORE fallible exact preparation.
    // If it throws afterwards, the Caller's state0 is still genuinely unreached.
    void* actual=allocate_parent();
    if(!actual)impl_->normal_null_recorded.store(true,std::memory_order_release);
    return actual; // preserve a real normal-NULL/control path; no early reset
}
void NativePostEffectParentFrame::initializer_constructor_entered() {
    require_healthy();auto* i=impl_->initializer.get();
    require_owned(i&&i->admitted&&i->outer_entered&&!i->finished&&i->material_scope&&
        impl_->allocator_returned.load()&&impl_->actual&&!impl_->constructor_entered.load()&&
        !i->constructor_call_entered&&!impl_->native_constructor_returned.load()&&!impl_->published.load(),
        "Initializer constructor entry requires its exact genuine supplier return");
    // Last metadata stores before the actual raw call; all fallible admission
    // is above. Its adjacent escape catch must enclose ONLY that call.
    i->constructor_call_entered=true;
    impl_->constructor_entered.store(true,std::memory_order_release);
}
void NativePostEffectParentFrame::initializer_constructor_returned(void* returned) {
    auto* i=impl_->initializer.get();
    const bool valid=i&&i->admitted&&i->outer_entered&&!i->finished&&i->constructor_call_entered&&
        !i->constructor_call_returned&&!i->constructor_call_escaped;
    // Save the actual WHOLE C++ return before ANY fallible post-return check.
    if(i&&!i->constructor_call_returned){i->constructor_return_value=returned;i->constructor_call_returned=true;}
    require_owned(valid,"Initializer constructor return has no SAME admitted call");
    require_owned(i->constructor_return_value==impl_->actual,"Initializer constructor returned another Parent allocation");
    source_constructor_returned();
}
void NativePostEffectParentFrame::initializer_constructor_escaped() noexcept {
    auto* i=impl_->initializer.get();
    if(!i||!i->admitted||!i->outer_entered||i->finished||!i->constructor_call_entered||
        i->constructor_call_returned||i->constructor_call_escaped){record_source_failure();return;}
    i->constructor_call_escaped=true;
    // This is a Source raw-call escape. native_constructor_returned separately
    // preserves completed Native body/publication debt on a later host failure.
    record_source_failure();
}
void NativePostEffectParentFrame::before_initializer_raw_free(void* captured) noexcept {
    auto* i=impl_->initializer.get();
    const bool first=i&&!i->raw_free_entered;
    if(first){i->raw_free_operand=captured;i->raw_free_entered=true;}
    record_source_failure(); // actual failure continuation, no fallible health probe
    if(!first||!i->admitted||!i->outer_entered||i->finished||!i->material_scope||
        !impl_->attached||!impl_->allocator_returned.load()||!captured||captured!=impl_->actual||
        impl_->native_constructor_returned.load()||impl_->published.load()||impl_->owner_live||
        impl_->reference_live||exact_.borrows_.load(std::memory_order_acquire)!=0||
        (i->constructor_call_entered&&!i->constructor_call_escaped)||i->constructor_call_returned||
        impl_->constructor_entered.load()!=i->constructor_call_entered)return;
    i->raw_free_same=true;
    if(!impl_->atomic_born.load()){
        impl_->domain.impl_->owners.abandon_exact_parent_record_before_cpp_birth(exact_);
    }else{
        if(!i->constructor_call_escaped||!impl_->count_home||i->prefix_ended)return;
        // Only actual born +4, after reached name/constructor unwind's last
        // prefix use and BEFORE the SAME free. No Native count load or store.
        std::destroy_at(impl_->count_home);i->prefix_ended=true;
        impl_->domain.impl_->owners.deactivate_exact_record_from_partial_cpp_end(exact_);
    }
    // The caller ALWAYS executes its original free, including mismatch. This
    // hook never substitutes/suppresses/replays it or records normal death.
}
void NativePostEffectParentFrame::initializer_raw_free_returned() noexcept {
    auto* i=impl_->initializer.get();
    if(!i||!i->admitted||!i->outer_entered||i->finished||!i->raw_free_entered||
        i->raw_free_returned){record_source_failure();return;}
    i->raw_free_returned=true; // actual void free returned; no physical close credit
}
void NativePostEffectParentFrame::initializer_source_returned() {
    require_healthy();auto* i=impl_->initializer.get();
    const bool normal_null=impl_->allocator_returned.load()&&!impl_->actual&&
        impl_->normal_null_recorded.load()&&!impl_->constructor_entered.load();
    require_owned(i&&i->admitted&&i->outer_entered&&!i->finished&&i->material_scope&&
        !i->source_returned&&!i->raw_free_entered&&
        (normal_null||(i->constructor_call_returned&&i->constructor_return_value==impl_->actual&&
            impl_->source_constructor_returned.load())),
        "Initializer Source return requires its real complete tail/health or normal NULL");
    i->source_returned=true;
}
void NativePostEffectParentFrame::finish_initializer_call() noexcept {
    auto* i=impl_->initializer.get();
    if(!i||!i->admitted||i->finished){record_source_failure();return;}
    if(!i->source_returned||std::uncaught_exceptions()>i->uncaught)record_source_failure();
    i->finished=true;
    if(i->outer_entered){i->outer_entered=false;end_outer_call();}
    i->material_scope.reset(); // after outer withdrawal, exactly as the existing wrapper
    i->terminal_invocation.reset();
}
void NativePostEffectParentFrame::detach_initializer_caller(NativeBrightPassInitializationBlock& caller) {
    require_owned(!requires_process_retention(),"Failed initializer Caller loan remains retained");
    auto* i=impl_->initializer.get();
    require_owned(i&&i->bright_live==&caller&&i->bright_identity==&caller&&!i->dust_identity&&
        !i->detached&&impl_->closed.load()&&!impl_->attached&&!impl_->block_linked&&
        !i->outer_entered&&!i->material_scope&&!i->terminal_invocation&&(!i->admitted||i->finished)&&
        i->source_ended&&!i->source_live&&!i->source_next&&!i->source_failed&&
        caller.post_effect().phase()==NativePostEffect20ConstructionBlock::Phase::idle,
        "Bright Caller loan ends only after SAME Parent logical close and real Source end");
    i->bright_live=nullptr;i->detached=true; // opaque identity/history remains
}
void NativePostEffectParentFrame::detach_initializer_caller(NativePostEffectDustBlock& caller) {
    require_owned(!requires_process_retention(),"Failed initializer Caller loan remains retained");
    auto* i=impl_->initializer.get();
    require_owned(i&&i->dust_live==&caller&&i->dust_identity==&caller&&!i->bright_identity&&
        !i->detached&&impl_->closed.load()&&!impl_->attached&&!impl_->block_linked&&
        !i->outer_entered&&!i->material_scope&&!i->terminal_invocation&&(!i->admitted||i->finished)&&
        i->source_ended&&!i->source_live&&!i->source_next&&!i->source_failed&&
        caller.child().phase()==NativePostEffectConstructionBlock::Phase::idle,
        "Dust Caller loan ends only after SAME Parent logical close and real Source end");
    i->dust_live=nullptr;i->detached=true;
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
    require_owned(impl_->producer_entered.load()&&impl_->constructor_scope_active.load(),"Parent allocation requires its admitted whole constructor scope");
    if(impl_->domain.impl_->binding.sections)require_prepared_section_calls();
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
    if(impl_->domain.impl_->binding.materials || impl_->domain.impl_->binding.sections){
        // Genuine NULL is not a cancellation receipt. The cold Material loan
        // remains live through this admitted constructor's final catch fence.
        impl_->normal_null_recorded.store(true,std::memory_order_release);return;
    }
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
    begin_outer_call();
    std::optional<MaterialConstructorScope> material_scope;
    try {
        require_owned(!impl_->producer_entered.exchange(true),"Parent allocation/constructor cannot replay");
        material_scope.emplace(*this); // admitted winner only, BEFORE allocation
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect20_00b4e470(actual,0x20u,name,vertices,optional,*impl_->block20);
            source_constructor_returned();
        }
        end_outer_call(); return actual;
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
}
void* NativePostEffectParentFrame::construct20(const NativePostEffect20ArgumentView& arguments) {
    require_owned(impl_->kind == Kind::parent20,"Parent20 producer family mismatch");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call();
    std::optional<MaterialConstructorScope> material_scope;
    try {
        require_owned(!impl_->producer_entered.exchange(true),"Parent allocation/constructor cannot replay");
        material_scope.emplace(*this); // admitted winner only, BEFORE allocation
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect20_00b4e470(actual,0x20u,arguments,*impl_->block20);
            source_constructor_returned();
        }
        end_outer_call(); return actual;
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
}
void* NativePostEffectParentFrame::construct24(NativeString& name, Word primitives, const void* optional) {
    require_owned(impl_->kind == Kind::parent24,"Parent24 producer family mismatch");
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_outer_call();
    std::optional<MaterialConstructorScope> material_scope;
    try {
        require_owned(!impl_->producer_entered.exchange(true),"Parent allocation/constructor cannot replay");
        material_scope.emplace(*this); // admitted winner only, BEFORE allocation
        void* actual = allocate_parent();
        if (!actual) cancel_normal_null_allocation();
        else {
            impl_->constructor_entered.store(true,std::memory_order_release);
            (void)construct_native_post_effect_00b4e840(actual,0x24u,name,primitives,optional,*impl_->block24);
            source_constructor_returned();
        }
        end_outer_call(); return actual;
    } catch (...) { record_source_failure(); end_outer_call(); throw; }
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
    std::optional<MaterialPayloadScope> material_scope;
    const Word original = reinterpret_cast<Word>(impl_->actual);
    try {
        material_scope.emplace(*this); // scalar CAS winner, lives through catch
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
void NativePostEffectParentFrame::prepare_section_caller(NativeMeshSectionLifetimeSourceScope& source) {
    require_healthy();auto& d=*impl_->domain.impl_;
    require_owned(d.binding.sections&&!impl_->section&&impl_->attached&&impl_->counted&&impl_->block_linked&&
        !impl_->producer_entered.load()&&!impl_->allocator_entered.load()&&!source.ended_&&!source.failed_,
        "Parent Section association requires retained cold attachment and SAME live Source scope");
    if(impl_->block20)require_owned(impl_->block20->parent_frame_==this&&impl_->block20->context_==&d.c20&&
        impl_->block20->ordinary_lifetime_==&d.binding&&!impl_->block20->section_frame_,
        "Parent20 Section association lost its exact cold Block");
    else require_owned(impl_->block24->parent_frame_==this&&impl_->block24->context_==&d.c24&&
        impl_->block24->ordinary_lifetime_==&d.binding&&!impl_->block24->section_frame_,
        "Parent24 Section association lost its exact cold Block");
    // Store first: a later reservation failure must not discard acquired loans.
    impl_->section=std::make_unique<SectionConstruction>();
    auto& s=*impl_->section;
    s.source_live=&source;s.source_identity=&source;s.block_attached=true;
    s.source_next=source.parents_;source.parents_=this;
    s.frame=&d.binding.sections->prepare();
    s.frame->bind_parent_caller(*this);s.retirement_pending=true;
    s.create=&s.frame->prepare_call(source);
    s.material=&s.frame->prepare_call(source);
    s.rebuild=&s.frame->prepare_call(source);
    s.creator=&s.frame->prepare_call(source);
}
void NativePostEffectParentFrame::require_prepared_section_calls() const {
    require_healthy();auto& d=*impl_->domain.impl_;
    const auto* s=impl_->section.get();
    require_owned(d.binding.sections&&s&&s->source_live&&s->source_identity==s->source_live&&
        !s->source_ended&&!s->source_failed&&!s->source_live->ended_&&!s->source_live->failed_&&
        s->frame&&s->create&&s->material&&s->rebuild&&s->creator&&s->retirement_pending&&
        s->block_attached&&!s->caller_end_entered&&!s->whole_constructor_returned&&!s->producer_returned,
        "Selected Parent allocation needs all four SAME unused Section callers before effects");
    s->frame->require_prepared_call(*s->create);
    s->frame->require_prepared_call(*s->material);
    s->frame->require_prepared_call(*s->rebuild);
    s->frame->require_prepared_call(*s->creator);
}
void NativePostEffectParentFrame::require_section_constructor_operation(void* actual) const {
    require_prepared_section_calls();auto& d=*impl_->domain.impl_;
    require_owned(impl_->attached&&impl_->counted&&impl_->block_linked&&
        impl_->producer_entered.load()&&impl_->constructor_outer.load()&&
        impl_->constructor_scope_active.load(std::memory_order_acquire)&&
        impl_->allocator_entered.load()&&impl_->allocator_returned.load()&&impl_->constructor_entered.load()&&
        actual&&actual==impl_->actual&&!impl_->published.load()&&!impl_->native_constructor_returned.load()&&
        !impl_->source_constructor_returned.load()&&!impl_->scalar_entered.load(),
        "Selected Parent Section entry requires its admitted whole constructor and exact allocation");
    if(impl_->block20)require_owned(impl_->block20->parent_frame_==this&&impl_->block20->context_==&d.c20&&
        impl_->block20->ordinary_lifetime_==&d.binding,"Selected Parent20 lost its exact Section caller Block");
    else require_owned(impl_->block24->parent_frame_==this&&impl_->block24->context_==&d.c24&&
        impl_->block24->ordinary_lifetime_==&d.binding,"Selected Parent24 lost its exact Section caller Block");
    // No later count1/profile value or invented Native read authenticates entry.
}
void NativePostEffectParentFrame::require_section_producer_operation(
    NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionLifetimeCall& call) const {
    require_section_constructor_operation(impl_->actual);
    require_owned(impl_->section->frame==&frame&&impl_->section->create==&call,
        "Parent Section producer must use its exact reserved create site");
}
NativeMeshSectionStorage* NativePostEffectParentFrame::construct_section() {
    require_section_constructor_operation(impl_->actual);auto& s=*impl_->section;
    auto* actual=s.frame->produce_for_parent(*s.create,*this);
    s.normal_null=actual==nullptr;s.producer_returned=true;
    return actual;
}
void NativePostEffectParentFrame::capture_section_constructor_return(
    NativeMeshSectionLifetimeFrame& frame,NativeMeshSectionStorage* returned) noexcept {
    auto* s=impl_->section.get();
    if(!s||s->frame!=&frame||!s->retirement_pending||s->whole_constructor_returned||
        !s->block_attached||!impl_->block_linked||!impl_->constructor_scope_active.load(std::memory_order_acquire)||
        !impl_->constructor_outer.load()||!impl_->constructor_entered.load()||!impl_->actual){
        record_source_failure();return;
    }
    // Pointer-form observation only. In particular, do not form *returned before
    // the Section producer validates its own whole return and publishes it.
    s->constructor_return_value=returned;s->whole_constructor_returned=true;
    if(impl_->block20)impl_->block20->acquired_.section_created=returned;
    else impl_->block24->acquired_.section_created=returned;
}
NativeMeshSectionReference& NativePostEffectParentFrame::publish_section_borrow() {
    require_healthy();auto& s=*impl_->section;
    require_owned(s.producer_returned&&!s.normal_null&&s.whole_constructor_returned&&
        s.constructor_return_value&&s.block_attached&&impl_->block_linked&&!s.borrowed&&!s.canonical,
        "Parent Section borrow needs the genuine returned/published child, once only");
    auto& reference=s.frame->canonical();s.canonical=&reference;s.borrowed=true;
    if(impl_->block20){
        auto& b=*impl_->block20;b.section_frame_=s.frame;
        b.keys_[NativePostEffect20ConstructionBlock::section]=s.constructor_return_value;
        b.references_[NativePostEffect20ConstructionBlock::section]=&reference;
        b.registered_[NativePostEffect20ConstructionBlock::section]=false;
        b.retired_[NativePostEffect20ConstructionBlock::section]=false;
        b.acquired_.section_reference=&reference;
    }else{
        auto& b=*impl_->block24;b.section_frame_=s.frame;
        b.keys_[NativePostEffectConstructionBlock::section]=s.constructor_return_value;
        b.references_[NativePostEffectConstructionBlock::section]=&reference;
        b.registered_[NativePostEffectConstructionBlock::section]=false;
        b.retired_[NativePostEffectConstructionBlock::section]=false;
        b.acquired_.section_reference=&reference;
    }
    return reference; // SAME Frame canonical; no optional companion/bind/retain
}
void NativePostEffectParentFrame::set_section_material(void* incoming) {
    require_healthy();auto& s=*impl_->section;
    require_owned(s.borrowed&&s.block_attached&&impl_->constructor_scope_active.load()&&
        impl_->constructor_outer.load()&&!impl_->native_constructor_returned.load(),
        "Section material site requires the actual whole Parent constructor");
    set_native_mesh_section_material_00b864c0(*s.frame,*s.material,incoming);
    s.material_returned=true;
}
void NativePostEffectParentFrame::rebuild_section_layout(void* mesh) {
    require_healthy();auto& s=*impl_->section;
    require_owned(s.borrowed&&s.block_attached&&impl_->constructor_scope_active.load()&&
        impl_->constructor_outer.load()&&!impl_->native_constructor_returned.load(),
        "Section layout site requires the actual whole Parent constructor");
    auto& d=*impl_->domain.impl_;
    auto& layouts=impl_->block20?d.c20.layouts:d.c24.layouts;
    rebuild_native_mesh_section_vertex_layout_00b865a0(*s.frame,*s.rebuild,mesh,layouts);
    s.rebuild_returned=true;
}
void NativePostEffectParentFrame::release_section_creator(long (__stdcall* captured)(volatile long*)) {
    require_healthy();auto& s=*impl_->section;
    require_owned(s.borrowed&&s.block_attached&&impl_->constructor_scope_active.load()&&
        impl_->constructor_outer.load()&&!impl_->native_constructor_returned.load(),
        "Section creator site requires its actual local A and saved import epoch");
    release_native_mesh_section_creator(*s.frame,*s.creator,captured);
    s.creator_returned=true;
}
NativePostEffectParentFrame* NativePostEffectParentFrame::section_source_scope_ended(
    const NativeMeshSectionLifetimeSourceScope& source,bool failed) noexcept {
    auto* s=impl_->section.get();auto* i=impl_->initializer.get();
    NativePostEffectParentFrame* next=nullptr;
    if(s){
        next=s->source_next;
        if(s->source_identity!=&source||s->source_live!=&source||s->source_ended){
            record_source_failure();return next;
        }
        s->source_next=nullptr;s->source_live=nullptr;s->source_ended=true;s->source_failed=failed;
        if(i){
            if(i->source_identity!=&source||i->source_live!=&source||i->source_ended||i->owns_observer){
                record_source_failure();return next;
            }
            i->source_live=nullptr;i->source_ended=true;i->source_failed=failed;
        }
    }else if(i&&i->owns_observer){
        next=i->source_next;
        if(i->source_identity!=&source||i->source_live!=&source||i->source_ended){
            record_source_failure();return next;
        }
        i->source_next=nullptr;i->source_live=nullptr;i->source_ended=true;
        i->source_failed=failed;i->owns_observer=false;
    }else {record_source_failure();return nullptr;}
    if(failed)record_source_failure();
    // Only the actual Source destructor calls this AFTER ALL Call withdrawals.
    // Every target remains owned through its ENTIRE traversal; no flag pins it.
    finish_closed_section_custody();return next;
}
void NativePostEffectParentFrame::section_native_retired(NativeMeshSectionLifetimeFrame& frame,
    NativeMeshSectionReference& reference) noexcept {
    auto* s=impl_->section.get();
    if(!s||s->frame!=&frame||s->canonical!=&reference||!s->whole_constructor_returned||
        !s->retirement_pending||s->retired){record_source_failure();return;}
    s->retired=true;s->retirement_pending=false;
    if(s->block_attached){
        if(!impl_->block_linked){record_source_failure();return;}
        if(impl_->block20)impl_->block20->section_source_retired(*this,frame,reference);
        else impl_->block24->section_source_retired(*this,frame,reference);
        s->borrowed=false;
    }
    // After logical detach ONLY this stable Parent-owned metadata is touched.
    // Retirement precedes scalar health/Source return and is no success fence.
    finish_closed_section_custody();
}
void NativePostEffectParentFrame::preflight_section_caller_end() const {
    if(!impl_->domain.impl_->binding.sections){
        require_owned(!impl_->section,"Null Section binding cannot adopt a caller later");return;
    }
    require_healthy();const auto* s=impl_->section.get();
    require_owned(s&&s->source_ended&&!s->source_live&&!s->source_next&&!s->source_failed,
        "Parent Section callers require the SAME actual post-withdrawal Source end");
    if(s->caller_ended)return; // remaining addresses are opaque history only
    require_owned(!s->caller_end_entered&&s->frame&&s->create&&s->material&&s->rebuild&&s->creator&&
        !impl_->constructor_outer.load()&&!impl_->constructor_scope_active.load(),
        "Parent Section caller end cannot replay/cross its constructor fence");
    const bool skipped=(!impl_->producer_entered.load()&&!impl_->allocator_entered.load())||
        (impl_->normal_null_recorded.load()&&impl_->allocator_returned.load()&&!impl_->actual);
    if(skipped)require_owned(!s->producer_returned&&!s->whole_constructor_returned&&!s->canonical&&
        !s->borrowed&&!s->material_returned&&!s->rebuild_returned&&!s->creator_returned,
        "Unused Section callers require genuine never-entered/Parent-NULL history");
    else require_owned(s->producer_returned&&!s->normal_null&&s->whole_constructor_returned&&s->canonical&&
        s->material_returned&&s->rebuild_returned&&s->creator_returned,
        "Parent Section sites retain unknown/partial/normal-tail return custody");
    // These are ONLY our four actual top-level Calls, never Frame.calls,
    // terminal/foreign/ancestor heads or the Section Domain final seal.
    s->frame->preflight_caller_end(*s->create);
    s->frame->preflight_caller_end(*s->material);
    s->frame->preflight_caller_end(*s->rebuild);
    s->frame->preflight_caller_end(*s->creator);
}
void NativePostEffectParentFrame::end_section_caller_after_host_quiescence() {
    preflight_section_caller_end();if(!impl_->section||impl_->section->caller_ended)return;
    auto& s=*impl_->section;s.caller_end_entered=true;
    s.frame->end_caller_after_host_quiescence(*s.create);
    s.frame->end_caller_after_host_quiescence(*s.material);
    s.frame->end_caller_after_host_quiescence(*s.rebuild);
    s.frame->end_caller_after_host_quiescence(*s.creator);
    if(s.retirement_pending&&s.frame->detach_unborn_parent_caller(*this))s.retirement_pending=false;
    // End the Block's loan before Material/factory reset. No Native death or
    // fake retired flag is issued by clearing borrowed C++ cells.
    if(impl_->block20){
        auto& b=*impl_->block20;
        b.references_[NativePostEffect20ConstructionBlock::section]=nullptr;
        b.acquired_.section_reference=nullptr;b.section_frame_=nullptr;
    }else{
        auto& b=*impl_->block24;
        b.references_[NativePostEffectConstructionBlock::section]=nullptr;
        b.acquired_.section_reference=nullptr;b.section_frame_=nullptr;
    }
    s.borrowed=false;s.block_attached=false;s.caller_ended=true;
}
bool NativePostEffectParentFrame::section_caller_ended() const noexcept {
    return impl_->section&&impl_->section->caller_ended&&!requires_process_retention();
}
bool NativePostEffectParentFrame::section_custody_pending() const noexcept {
    const auto* s=impl_->section.get();
    return s&&(s->source_live||!s->source_ended||s->retirement_pending||!s->caller_ended);
}
void NativePostEffectParentFrame::finish_closed_section_custody() noexcept {
    if(impl_->closed.load(std::memory_order_acquire)&&impl_->counted&&
        !section_custody_pending()&&!requires_process_retention()){
        impl_->domain.impl_->retention.retained_.fetch_sub(1,std::memory_order_release);
        impl_->domain.impl_->retained.fetch_sub(1,std::memory_order_release);impl_->counted=false;
    }
}

void NativePostEffectParentFrame::prepare_material_caller(NativeMaterialFrame& material,
    NativeMaterialFactoryAcquired& acquired,NativeMaterialEffectCacheContext& cache) {
    require_healthy();auto& d=*impl_->domain.impl_;
    require_owned(d.binding.materials&&!material_frame_&&impl_->attached&&impl_->counted&&impl_->block_linked&&
        !impl_->producer_entered.load()&&!impl_->allocator_entered.load(),
        "Parent Material association requires retained cold Parent attachment before effects");
    if(impl_->block20){
        auto& block=*impl_->block20;
        require_owned(block.parent_frame_==this&&block.ordinary_lifetime_==&d.binding&&block.context_==&d.c20&&
            block.material_frame_==&material&&block.material_factory_ && &*block.material_factory_==&acquired&&
            &cache==&d.c20.material_effects,"Parent20 Material loan must preserve its exact stable Block/factory/cache");
    }else{
        auto& block=*impl_->block24;
        require_owned(block.parent_frame_==this&&block.ordinary_lifetime_==&d.binding&&block.context_==&d.c24&&
            block.material_frame_==&material&&block.material_factory_ && &*block.material_factory_==&acquired&&
            &cache==&d.c24.material_effects,"Parent24 Material loan must preserve its exact stable Block/factory/cache");
    }
    material.bind_parent_caller(*d.binding.materials,*this,acquired,cache);
    material_frame_=&material; // immutable association; actual outer owner owns the loan
}
void NativePostEffectParentFrame::require_material_constructor_operation(void* actual) const {
    require_healthy();auto& d=*impl_->domain.impl_;
    require_owned(d.binding.materials&&material_frame_&&impl_->attached&&impl_->counted&&impl_->block_linked&&
        impl_->constructor_scope_active.load(std::memory_order_acquire)&&impl_->constructor_outer.load()&&
        impl_->producer_entered.load()&&impl_->allocator_entered.load()&&impl_->allocator_returned.load()&&
        impl_->constructor_entered.load()&&actual&&actual==impl_->actual&&!impl_->published.load()&&
        !impl_->native_constructor_returned.load()&&!impl_->source_constructor_returned.load()&&!impl_->scalar_entered.load(),
        "Selected Parent constructor needs its admitted actual allocation/whole Source scope before Native effects");
    if(impl_->block20)require_owned(impl_->block20->parent_frame_==this&&impl_->block20->context_==&d.c20&&
        impl_->block20->ordinary_lifetime_==&d.binding&&impl_->block20->material_frame_==material_frame_,
        "Selected Parent20 constructor lost its exact Block attachment");
    else require_owned(impl_->block24->parent_frame_==this&&impl_->block24->context_==&d.c24&&
        impl_->block24->ordinary_lifetime_==&d.binding&&impl_->block24->material_frame_==material_frame_,
        "Selected Parent24 constructor lost its exact Block attachment");
    // No later atomic/count1 birth prerequisite and no Native payload read.
}
void NativePostEffectParentFrame::material_field_published(NativeMaterialStorage* stored) noexcept {
    // The free constructor already authenticated its real raw allocation before
    // effects. This hook follows its actual +14 store, carrying THAT operand.
    if(!material_frame_||!impl_->attached||!impl_->block_linked||!impl_->actual||
        !impl_->allocator_returned.load()||!impl_->constructor_entered.load()||
        !impl_->constructor_scope_active.load(std::memory_order_acquire)||!impl_->constructor_outer.load()||
        (impl_->block20?impl_->block20->parent_frame_!=this:impl_->block24->parent_frame_!=this)){
        record_source_failure();return;
    }
    material_frame_->parent_field_published(*this,impl_->actual,stored);
    // Metadata-only genuine creator handoff, zero Native count operations.
}
void NativePostEffectParentFrame::set_material_parameter_owner(NativeMaterialStorage& material,
    void* incoming,std::uint8_t retain_byte,NativeRenderActualOwners& owners) {
    require_healthy();
    require_owned(material_frame_&&impl_->constructor_scope_active.load(std::memory_order_acquire)&&
        impl_->constructor_outer.load()&&impl_->constructor_entered.load()&&!impl_->native_constructor_returned.load()&&
        incoming==impl_->actual && &owners==&impl_->domain.impl_->owners&&
        &material==&material_frame_->canonical().storage()&&!impl_->material_setter_entered.exchange(true),
        "Parent setter requires the actual admitted constructor/local factory Material and persistent receiver");
    NativeMaterialStorageAccess::set_parameter_owner(material,incoming,retain_byte,owners,material_parameter_release_);
    require_healthy();impl_->material_setter_returned.store(true,std::memory_order_release);
}
void NativePostEffectParentFrame::require_material_payload_operation() const {
    require_healthy();
    require_owned(material_frame_&&impl_->actual&&impl_->published.load()&&impl_->source_constructor_returned.load()&&
        impl_->scalar_entered.load()&&impl_->payload_scope_active.load(std::memory_order_acquire)&&
        !impl_->payload_returned.load()&&!impl_->typed_ended.load(),
        "Parent Material zero continuation requires its genuine scalar CAS winner's live payload");
    // This authentic private scope is issued only by the admitted scalar;
    // generic active-invocation counts/public phases do not issue the action.
}
void NativePostEffectParentFrame::invoke_material_observed_zero(RenderCommandReference& reference,void* actual) {
    require_material_payload_operation();
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    try{
        // The payload provider already decremented once, checked CURRENT
        // profile/+4/type/storage and resolved THIS actual target exactly once.
        // No factory-A equality check: actual CURRENT B owns its own hold.
        NativeRenderObservedZeroAccess::invoke(reference,actual,terminal_zero_frame_);
        require_healthy();
    }catch(...){record_source_failure();throw;} // metadata only; scalar owner fails heads at its fence
}
void NativePostEffectParentFrame::preflight_material_caller_close() const {
    require_healthy();
    require_owned(!impl_->constructor_scope_active.load()&&!impl_->payload_scope_active.load()&&
        material_parameter_release_.cpp_zero_frame.active_invocations_.load()==0&&
        terminal_zero_frame_.active_invocations_.load()==0,
        "Parent Material callers still own active constructor/scalar/receiver scopes");
    require_owned(!impl_->material_setter_entered.load()||impl_->material_setter_returned.load(),
        "Parent Material setter return/health remains unknown");
    const auto& setter=material_parameter_release_;
    require_owned(!setter.entered||(setter.decremented&&setter.returned&&
        (!setter.zero_dispatch_entered||setter.zero_dispatch_returned)),
        "Parent parameter old-source decrement/zero/return remains unknown");
    if(impl_->domain.impl_->binding.materials){
        require_owned(material_frame_!=nullptr,"Parent lost its selected Material caller association");
        if(impl_->block20)require_owned(impl_->block20->material_frame_==material_frame_,"Parent20 Material association changed");
        else require_owned(impl_->block24->material_frame_==material_frame_,"Parent24 Material association changed");
        material_frame_->preflight_parent_caller_close(*this);
    }else require_owned(!material_frame_,"Null Material binding cannot adopt a later caller association");
}
void NativePostEffectParentFrame::close_material_caller_after_host_quiescence() {
    preflight_material_caller_close();
    require_owned(NativeRenderObservedZeroAccess::end_frame(material_parameter_release_.cpp_zero_frame,false),
        "Parent setter caller did not quiesce");
    if(material_frame_)material_frame_->close_parent_caller_after_host_quiescence(*this);
    require_healthy();
}
bool NativePostEffectParentFrame::material_caller_closed() const noexcept {
    return material_frame_&&!requires_process_retention()&&material_frame_->parent_caller_closed(*this);
}
void NativePostEffectParentFrame::preflight_close() const {
    try {
    require_healthy();auto* caller=observed_zero_hold_.frame_;const unsigned retained_hold=caller?1u:0u;
    if(const auto* i=impl_->initializer.get()){
        require_owned(i->source_ended&&!i->source_live&&!i->source_next&&!i->source_failed&&!i->owns_observer&&
            !i->outer_entered&&!i->material_scope&&!i->terminal_invocation&&
            (!i->admitted||(i->finished&&i->source_returned)),
            "Initializer close requires full automatic tail AND real external Source end");
    }
    require_owned(!impl_->constructor_outer.load()&&!impl_->constructor_scope_active.load()&&!impl_->payload_scope_active.load()&&
        terminal_zero_frame_.active_invocations_.load(std::memory_order_acquire)==0&&
        material_parameter_release_.cpp_zero_frame.active_invocations_.load(std::memory_order_acquire)==0&&
        (!caller||caller->active_invocations_.load(std::memory_order_acquire)==0)&&
        impl_->outer_calls.load()==retained_hold&&exact_.outer_calls_.load()==retained_hold&&exact_.borrows_.load()==0,
        "Parent close crossed an active operation/count use/whole caller fence");
    const bool never_entered=!impl_->producer_entered.load()&&!impl_->allocator_entered.load()&&
        !impl_->constructor_entered.load()&&!impl_->atomic_born.load()&&!impl_->published.load();
    const bool normal_null=impl_->normal_null_recorded.load()&&impl_->allocator_returned.load()&&!impl_->actual&&
        !impl_->constructor_entered.load()&&!impl_->atomic_born.load()&&!impl_->published.load();
    const bool cancelled=impl_->cold_cancelled.load()&&!impl_->actual&&!impl_->constructor_entered.load()&&
        !impl_->atomic_born.load()&&!impl_->published.load()&&
        (never_entered||(impl_->allocator_entered.load()&&impl_->allocator_returned.load()));
    require_owned(never_entered||normal_null||cancelled||
        (impl_->scalar_source_returned.load()&&impl_->payload_returned.load()&&impl_->typed_ended.load()&&
         impl_->free_returned.load()&&impl_->retired.load()&&!impl_->creator_outstanding.load()),
        "Parent close requires actual complete Source/physical facts, not flags0/partial/unknown");
    preflight_section_caller_end();
    preflight_material_caller_close();
    // This is the separately frozen Block4+Root4 non-draining normal/cold
    // readiness seam. Its checks precede EVERY receiver/child/cancel/reset.
    if(impl_->block20){
        require_owned(impl_->block20->phase_!=NativePostEffect20ConstructionBlock::Phase::idle||cancelled,
            "Idle Parent20 Block readiness requires authentic prior cold cancellation");
        impl_->block20->preflight_close_after_host_quiescence();
    }else{
        require_owned(impl_->block24->phase_!=NativePostEffectConstructionBlock::Phase::idle||cancelled,
            "Idle Parent24 Block readiness requires authentic prior cold cancellation");
        impl_->block24->preflight_close_after_host_quiescence();
    }
    }catch(...){const_cast<NativePostEffectParentFrame*>(this)->record_source_failure();throw;}
}
void NativePostEffectParentDomain::preflight_close() const {
    try {
    require_owned(!impl_->failed.load()&&!impl_->retention.requires_process_retention(),
        "Failed SAME Parent graph must remain retained before any drain");
    NativePostEffectParentFrame* first;
    {std::lock_guard<std::mutex> lock(impl_->mutex);first=impl_->first;}
    // Stable Domain-owned list; actual owner independently proves no incoming
    // workers/Native/callers/preparations. Throwing checks run OUTSIDE the lock.
    for(auto* frame=first;frame;frame=frame->impl_->next)
        if(!frame->impl_->closed.load())frame->preflight_close();
    }catch(...){NativePostEffectParentFrame::record_domain_failure(*const_cast<NativePostEffectParentDomain*>(this));throw;}
}

void NativePostEffectParentFrame::close_after_host_quiescence() {
    preflight_close();
    auto* caller=observed_zero_hold_.frame_;
    const bool never_entered=!impl_->producer_entered.load()&&!impl_->allocator_entered.load()&&
        !impl_->constructor_entered.load()&&!impl_->atomic_born.load()&&!impl_->published.load();
    const bool selected_material=impl_->domain.impl_->binding.materials!=nullptr;
    const bool selected_section=impl_->domain.impl_->binding.sections!=nullptr;
    const bool deferred_cold=selected_material||selected_section||impl_->initializer!=nullptr;
    try {
        if(never_entered&&!deferred_cold){
            // Protected old null-binding cold cancellation route.
            if(impl_->block20)impl_->block20->cancel_ordinary_preparation();
            else impl_->block24->cancel_ordinary_preparation();
            impl_->cold_cancelled.store(true,std::memory_order_release);
        }
        // Full Domain first pass has validated EVERY family's receivers before
        // this first unlink. End only the four Section callers before reset.
        if(selected_section)end_section_caller_after_host_quiescence();
        if(caller)require_owned(NativeRenderObservedZeroAccess::end_frame(*caller,false),
            "Parent caller still has an active invocation");
        if(caller!=&terminal_zero_frame_)require_owned(NativeRenderObservedZeroAccess::end_frame(terminal_zero_frame_,false),
            "Parent terminal caller still has an active invocation");
        require_owned(NativeRenderObservedZeroAccess::end_frame(material_parameter_release_.cpp_zero_frame,false),
            "Parent parameter setter caller still has an active invocation");
        require_healthy();
        require_owned(impl_->outer_calls.load()==0&&exact_.outer_calls_.load()==0&&exact_.borrows_.load()==0,
            "Parent close retains an actual C++ caller/count user");
        close_material_caller_after_host_quiescence();
        if(deferred_cold&&(never_entered||impl_->normal_null_recorded.load())){
            // The genuine cold Material frame/observations have NOW closed;
            // only this actual post-scope fence may destroy its factory loan.
            if(impl_->block20)impl_->block20->cancel_ordinary_preparation();
            else impl_->block24->cancel_ordinary_preparation();
            impl_->cold_cancelled.store(true,std::memory_order_release);
        }
        if(impl_->cold_cancelled.load())impl_->domain.impl_->owners.cancel_exact_record_without_payload(exact_);
        else {
            if(impl_->block20)impl_->block20->close_ordinary_after_host_quiescence();
            else impl_->block24->close_ordinary_after_host_quiescence();
            impl_->domain.impl_->owners.close_exact_record_at_quiescence(exact_);
        }
        impl_->attached=false;
        if(impl_->block20)impl_->block20->parent_frame_=nullptr;
        else impl_->block24->parent_frame_=nullptr;
        impl_->block_linked=false;impl_->closed.store(true,std::memory_order_release);
        if(impl_->counted&&!section_custody_pending()){
            impl_->domain.impl_->retention.retained_.fetch_sub(1,std::memory_order_release);
            impl_->domain.impl_->retained.fetch_sub(1,std::memory_order_release);impl_->counted=false;
        }
    }catch(...){record_source_failure();throw;}
}
void NativePostEffectParentDomain::close_after_host_quiescence() {
    try {
        require_owned(!impl_->failed.load()&&!impl_->retention.requires_process_retention(),
            "Failed SAME Parent graph must remain retained");
        {std::lock_guard<std::mutex> lock(impl_->mutex);impl_->closing.store(true,std::memory_order_release);}
        // Full Domain FIRST pass includes Material/setter/payload and actual
        // non-Material Block normal/cold readiness before ANY earlier drain.
        preflight_close();
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next)
            if(!frame->impl_->closed.load())frame->close_after_host_quiescence();
        unsigned expected_pending=0;
        for(auto* frame=impl_->first;frame;frame=frame->impl_->next){
            require_owned(frame->impl_->closed.load(),"Parent graph retains an unclosed record");
            if(frame->impl_->counted){
                require_owned(frame->section_custody_pending(),"Parent graph retains unexplained metadata custody");
                ++expected_pending;
            }
        }
        // Healthy pending retirement is expected retained metadata, not a
        // failure or a collection fence. The real owner keeps this Domain and
        // ALL targets through callback/Source/catch/worker/final fences.
        require_owned(impl_->retained.load()==expected_pending,"Parent graph retained metadata accounting changed");
    }catch(...){NativePostEffectParentFrame::record_domain_failure(*this);throw;}
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
