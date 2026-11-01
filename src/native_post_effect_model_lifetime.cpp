#include "bsp/native_render_observed_zero.hpp"
#include "bsp/native_post_effect_model_lifetime.hpp"
#include "bsp/native_post_effect_construction.hpp"
#include "bsp/native_post_effect20_construction.hpp"
#include "bsp/native_model_owner.hpp"
#include "bsp/native_model_pool.hpp"
#include "bsp/native_model_storage_geometry.hpp"
#include "bsp/native_mesh_canonical.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include <array>
#include <atomic>
#include <exception>
#include <limits>
#include <list>
#include <mutex>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Ordinary post-effect Model storage requires qualified MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
enum class ModelState : std::uint8_t { cold, allocated, live, ending, dead, closed };
enum class HomeState : std::uint8_t { ready, active, unknown };
}

struct NativePostEffectModelLifetimeContext::Impl {
    NativePostEffectModelLifetimeContext& self;
    NativeModelEnvironment& environment;
    NativeNodeBaseDestructionContext& common;
    NativeRenderActualOwnerRegistry& owners;
    NativePostEffectModelProfiles& profiles;
    const NativeNodeRawConstants& constants;
    NativeMeshCanonicalDomain& meshes;
    std::atomic<bool> failed{false};
    std::atomic<Word> active{0};

    Impl(NativePostEffectModelLifetimeContext& s, NativeModelEnvironment& e,
        NativeNodeBaseDestructionContext& n, NativeRenderActualOwnerRegistry& o,
        NativePostEffectModelProfiles& p, const NativeNodeRawConstants& k,
        NativeMeshCanonicalDomain& m)
        : self(s), environment(e), common(n), owners(o), profiles(p), constants(k), meshes(m) {
        require(e.actual_names == nullptr && e.nodes.uses_raw_name_pool() &&
            &e.nodes.require_raw_name_pool() == &n.strings &&
            &e.retained_owners == static_cast<NativeRenderActualOwners*>(&o) &&
            &n.scenes.owners == &o && &n.trees.owners == static_cast<NativeRenderActualOwners*>(&o) &&
            &n.scenes.decrement_00ce2220 == &n.trees.decrement_00ce2220 &&
            &k.negative_bound_00ce4adc == &e.constants.minimum_00ce4adc &&
            &k.positive_bound_00ce4970 == &e.constants.maximum_00ce4970,
            "ordinary post-effect Model requires the actual shared provider identities");
    }

    NativePostEffectModelFrame& lookup(void* actual, NativePostEffectModelFrame* origin = nullptr) {
        // An already-entered Native cleanup may still use its own live Node
        // after failure poisoning; it does not reacquire a public canonical.
        if (origin && origin->actual_key() == actual && origin->live_for_native_callback())
            return *origin;
        auto* base = owners.find(actual);
        auto* reference = dynamic_cast<NativeModelReference*>(base);
        auto* frame = reference ? NativePostEffectModelFrame::from_reference(*reference) : nullptr;
        require(frame && frame->belongs_to(self) && frame->actual_key() == actual &&
            frame->canonical() == reference && frame->live_for_native_callback(),
            "reached node requires its genuine ordinary Model frame; foreign/base producer is unadmitted");
        return *frame;
    }

    struct Operation final : NativeNodeTreeRetirementDispatch,
        NativeNodeTreeRetirementFrames, NativeNodeSceneChildDispatch,
        NativeSceneRegistrationDerivedTypes {
        Impl& shared;
        NativePostEffectModelFrame& origin;
        NativeSceneRegistrationGateContext gates;
        NativeNodeSceneAttachmentContext scenes;
        NativeNodeTreeRetirementContext trees;
        NativeNodeBaseDestructionContext node;
        NativeModelStorageContext storage;
        NativeNodeTreeRetirementFrame tree_frame{};
        NativeNodeTreeRetirementAcquired tree{}, unlink{};
        NativeNodeSceneAttachmentFrame scene_frame{};
        NativeNodeSceneAttachmentAcquired scene{};
        bool returned{};
        Operation(Impl& s, NativePostEffectModelFrame& f)
            : shared(s), origin(f),
              gates{s.common.scenes.gates.registry, s.common.scenes.gates.light_types,
                  s.common.scenes.gates.camera_types,
                  s.common.scenes.gates.actual_node_profile_00d62c88,
                  s.common.scenes.gates.actual_camera_profile_00d62cf0,
                  s.common.scenes.gates.actual_light_profile_00d62f58,
                  s.common.scenes.gates.actual_directional_profile_00d62fb0, this},
              scenes{gates,s.owners,s.common.scenes.increment_00ce221c,
                  s.common.scenes.decrement_00ce2220,*this},
              trees{s.environment.nodes.point_lights,s.owners,
                  s.common.trees.decrement_00ce2220,*this,*this},
              node{scenes,trees,s.environment.nodes.require_raw_name_pool(),s.common.world},
              storage{node,s.environment.pool_01090054,s.constants,
                  s.environment.constants.negative_zero_00d7a208} {}
        const volatile Word* resolve_profile(Word profile) const override {
            return shared.profiles.resolve_profile(profile);
        }
        std::uint8_t invoke_type_0c(Word target, void* actual, Word token) override {
            try {
                (void)shared.lookup(actual,&origin);
                require(target == 0x006ef860,"ordinary Model type requires genuine current 6EF860");
                return static_cast<std::uint8_t>(shared.environment.types.is_type_006ef860(token));
            } catch (...) { origin.record_source_failure(); throw; }
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            try { shared.lookup(actual,&origin).invoke_current_scene(target,requested,recursion); }
            catch (...) { origin.record_source_failure(); throw; }
        }
        void invoke_virtual18(Word target, void* actual) override {
            try { shared.lookup(actual,&origin).invoke_current18(target); }
            catch (...) { origin.record_source_failure(); throw; }
        }
        void invoke_virtual0(Word target, void* actual, RenderCommandReference& canonical) override {
            try {
                require(target == 0x00bd30e0,"ordinary Model tree requires genuine current BD30E0");
                auto* reference = dynamic_cast<NativeModelReference*>(&canonical);
                auto* selected = reference ? NativePostEffectModelFrame::from_reference(*reference) : nullptr;
                require(selected && selected->belongs_to(shared.self) &&
                    selected->actual_key() == actual && selected->canonical() == reference &&
                    selected->live_for_native_callback(),
                    "ordinary Model tree lost SAME selected canonical");
                // The real B6F310 site already performed CE2220, observed zero,
                // resolved this canonical and verified its actual+04 alias.
                selected->hold_observed_zero(origin.terminal_zero_frame_,*reference);
                selected->invoke_observed_zero(actual,*reference);
            } catch (...) { origin.record_source_failure(); throw; }
        }
        NativeNodeTreeRetirementInvocation recursive_unregister(void* actual) override {
            try { return shared.lookup(actual,&origin).loan_recursive_home(actual); }
            catch (...) { origin.record_source_failure(); throw; }
        }
    private:
        void native_recursive_returned(void* actual,
            const NativeNodeTreeRetirementInvocation& invocation) noexcept override {
            NativePostEffectModelFrame::recursive_home_returned(actual,invocation);
        }
    };
};

struct NativePostEffectModelFrame::Impl {
    NativePostEffectModelFrame& self;
    NativePostEffectModelLifetimeContext& context;
    NativePostEffectModelLifetimeContext::Impl& shared;
    const void* const producer;
    void (*notice)(const void*,NativePostEffectModelFrame&) noexcept{};
    // Reserved CPP companion storage only, never a view over Native words.
    std::aligned_storage_t<sizeof(NativeModelOwner),alignof(NativeModelOwner)> owner_storage;
    std::aligned_storage_t<sizeof(NativeModelReference),alignof(NativeModelReference)> reference_storage;
    NativeModelOwner* owner{};
    NativeModelReference* reference{};
    std::unique_ptr<NativePostEffectModelLifetimeContext::Impl::Operation> terminal_operation;
    NativeModelStorageLifetimeFrame terminal_frame{};
    NativeModelStorageLifetimeAcquired terminal{};
    NativeModelRawHostLifetime host{};
    NativeModelRawLifetimeBinding binding;
    std::array<volatile Word,4> homes{};
    NativeModelStorageGeometryFrame geometry_frame;
    NativeModelStorageGeometryAcquired geometry{};
    std::mutex operations_mutex;
    std::list<std::unique_ptr<NativePostEffectModelLifetimeContext::Impl::Operation>> operations;
    NativeNodeTreeRetirementFrame recursive_home{};
    NativeNodeTreeRetirementAcquired recursive_acquired{};
    std::atomic<HomeState> home_state{HomeState::ready};
    std::uint64_t home_invocation{};
    std::atomic<ModelState> state{ModelState::cold};
    std::atomic<bool> failed{false}, typed_ended{false}, zero_armed{false}, physical_debt{false};
    NativeModelPool* receiver{};
    void* allocation{};
    void* node_key{};
    void* return_target{};
    bool attached{}, retained{}, constructor_outer{}, wrapper_attempted{}, allocator_started{};
    bool allocation_returned{}, owner_birth_entered{}, owner_returned{}, native_constructor_entered{};
    bool constructor_returned{}, parent_published{}, prefix_tail_ended{};
    bool return_started{}, return_normal{}, scalar_started{}, scalar_kernel_entered{}, scalar_pool_started{};
    bool scalar_pool_normal{}, scalar_body_normal{}, normal_dead{}, last_flags{}, retired{}, closed{};
    bool zero_consumed{}, zero_source_returned{};
    std::uint8_t low_flags{};

    Impl(NativePostEffectModelFrame& f, NativePostEffectModelLifetimeContext& c, const void* p)
        : self(f), context(c), shared(*c.impl_), producer(p),
          terminal_operation(std::make_unique<NativePostEffectModelLifetimeContext::Impl::Operation>(shared,f)),
          binding{terminal_operation->storage,terminal_frame,terminal,host},
          geometry_frame(homes[0],homes[1],homes[2],homes[3]) {}
    auto& operation() {
        auto call = std::make_unique<NativePostEffectModelLifetimeContext::Impl::Operation>(shared,self);
        std::lock_guard<std::mutex> lock(operations_mutex);
        operations.push_back(std::move(call));
        return *operations.back(); // stable until explicit whole-Frame close
    }
};

NativePostEffectModelLifetimeContext::NativePostEffectModelLifetimeContext(
    NativeModelEnvironment& env, NativeNodeBaseDestructionContext& common,
    NativeRenderActualOwnerRegistry& owners, NativePostEffectModelProfiles& profiles,
    const NativeNodeRawConstants& constants, NativeMeshCanonicalDomain& meshes)
    : impl_(std::make_unique<Impl>(*this,env,common,owners,profiles,constants,meshes)) {}
NativePostEffectModelLifetimeContext::~NativePostEffectModelLifetimeContext() noexcept = default;
bool NativePostEffectModelLifetimeContext::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire) || impl_->active.load(std::memory_order_acquire) != 0;
}
void NativePostEffectModelLifetimeContext::release_captured_node(void* actual,
    NativeNodeDestructionRuntime& legacy) {
    impl_->active.fetch_add(1,std::memory_order_acq_rel);
    try {
        auto* reference = dynamic_cast<NativeModelReference*>(impl_->owners.find(actual));
        if (reference && NativePostEffectModelFrame::from_reference(*reference)) {
            impl_->lookup(actual).unlink_current_model(actual);
            impl_->active.fetch_sub(1,std::memory_order_release);
            return;
        }
        auto* binding = legacy.attachments.find_actual_node(
            static_cast<Word>(reinterpret_cast<std::uintptr_t>(actual)));
        require(binding != nullptr,"captured post-effect node has no real legacy or ordinary canonical");
        unlink_and_release_render_model_00b6dfa0(*binding);
        impl_->active.fetch_sub(1,std::memory_order_release);
    } catch (...) {
        NativePostEffectModelFrame::fail_context(*this);
        impl_->active.fetch_sub(1,std::memory_order_release);
        throw;
    }
}

NativePostEffectModelFrame::NativePostEffectModelFrame(
    NativePostEffectModelLifetimeContext& context, const void* producer)
    : exact_(context.impl_->owners,this), impl_(std::make_unique<Impl>(*this,context,producer)) {
    require(producer && &context.impl_->meshes.registry_ == &context.impl_->owners,
        "ordinary Model requires its exact producer and SAME central registry");
    context.impl_->owners.attach_exact_record(exact_);
    impl_->attached = true;
    context.impl_->meshes.retention_.retained_.fetch_add(1,std::memory_order_acq_rel);
    impl_->retained = true;
}
NativePostEffectModelFrame::~NativePostEffectModelFrame() noexcept = default;
void NativePostEffectModelFrame::fail_context(NativePostEffectModelLifetimeContext& c) noexcept {
    c.impl_->failed.store(true,std::memory_order_release);
    c.impl_->meshes.retention_.fail();
}
void NativePostEffectModelFrame::record_source_failure() noexcept {
    impl_->failed.store(true,std::memory_order_release);
    impl_->shared.owners.poison_exact_record(exact_);
    fail_context(impl_->context);
}
bool NativePostEffectModelFrame::requires_process_retention() const noexcept {
    return impl_->failed.load(std::memory_order_acquire) ||
        exact_.outer_calls_.load(std::memory_order_acquire) != 0 ||
        exact_.borrows_.load(std::memory_order_acquire) != 0 ||
        impl_->physical_debt.load(std::memory_order_acquire);
}
void NativePostEffectModelFrame::require_mesh_domain(NativeMeshCanonicalDomain& meshes) {
    require(&meshes == &impl_->shared.meshes,"ordinary Block must borrow SAME Mesh retention domain");
}
void NativePostEffectModelFrame::bind_block(NativePostEffectConstructionBlock& block) {
    require(impl_->producer == &block && !impl_->notice,"Model producer backlink cannot retarget");
    impl_->notice = [](const void* p, NativePostEffectModelFrame& f) noexcept {
        const_cast<NativePostEffectConstructionBlock*>(static_cast<const NativePostEffectConstructionBlock*>(p))->model_source_retired(f);
    };
}
void NativePostEffectModelFrame::bind_block(NativePostEffect20ConstructionBlock& block) {
    require(impl_->producer == &block && !impl_->notice,"Model producer backlink cannot retarget");
    impl_->notice = [](const void* p, NativePostEffectModelFrame& f) noexcept {
        const_cast<NativePostEffect20ConstructionBlock*>(static_cast<const NativePostEffect20ConstructionBlock*>(p))->model_source_retired(f);
    };
}
void NativePostEffectModelFrame::begin_call_pin() {
    require(!impl_->closed,"ordinary Model frame is already closed");
    exact_.outer_calls_.fetch_add(1,std::memory_order_acq_rel);
    impl_->shared.active.fetch_add(1,std::memory_order_acq_rel);
}
void NativePostEffectModelFrame::end_call_pin() noexcept {
    impl_->shared.active.fetch_sub(1,std::memory_order_release);
    exact_.outer_calls_.fetch_sub(1,std::memory_order_release);
}
void NativePostEffectModelFrame::require_source_healthy() {
    require(!impl_->failed.load(std::memory_order_acquire),
        "ordinary Model retains a prior Source/Native escape");
}
void NativePostEffectModelFrame::begin_constructor() {
    require(!impl_->constructor_outer && !impl_->wrapper_attempted && impl_->notice,
        "ordinary Model constructor requires one prepared exact producer");
    begin_call_pin();
    impl_->constructor_outer = true;
}
void NativePostEffectModelFrame::finish_constructor() noexcept {
    if (impl_->constructor_outer) {
        impl_->constructor_outer = false;
        end_call_pin();
    }
}
void NativePostEffectModelFrame::wrapper_entered() {
    require_source_healthy();
    require(impl_->constructor_outer && !impl_->wrapper_attempted && !impl_->allocation,
        "ordinary Model allocation cannot adopt or replay a slot");
    impl_->wrapper_attempted = true;
}
void NativePostEffectModelFrame::allocator_entered(NativeModelPool& receiver) {
    require(impl_->wrapper_attempted && !impl_->allocator_started &&
        &receiver == &impl_->shared.environment.pool_01090054,
        "ordinary Model static allocator requires its captured SAME receiver");
    impl_->receiver = &receiver;
    impl_->allocator_started = true;
}
void NativePostEffectModelFrame::allocator_returned(NativeModelPool& receiver, void* raw) noexcept {
    impl_->receiver = &receiver;
    impl_->allocation = raw;
    impl_->allocation_returned = true;
    impl_->physical_debt.store(raw != nullptr,std::memory_order_release);
    impl_->state.store(ModelState::allocated,std::memory_order_release);
}
void NativePostEffectModelFrame::allocator_escaped() noexcept { record_source_failure(); }
void NativePostEffectModelFrame::return_entered(NativeModelPool& receiver, void* raw) {
    require(impl_->allocation_returned &&
        &receiver == impl_->receiver && !impl_->return_started && !impl_->reference &&
        !impl_->owner,
        "constructor failure return requires actual receiver and CPP prefix disposition");
    impl_->return_target = raw; // B4E470 cleanup reloads CURRENT argument HOME
    impl_->return_started = true;
}
void NativePostEffectModelFrame::return_returned() noexcept {
    impl_->return_normal = true;
    if (impl_->return_target == impl_->allocation)
        impl_->physical_debt.store(false,std::memory_order_release);
}
void NativePostEffectModelFrame::return_escaped() noexcept { record_source_failure(); }
void* NativePostEffectModelFrame::actual_for_owner() {
    require(impl_->allocation_returned && impl_->allocation && !impl_->owner_returned &&
        impl_->receiver == &impl_->shared.environment.pool_01090054,
        "ordinary Model owner requires its genuine normal actual allocation");
    return impl_->allocation;
}
NativeModelEnvironment& NativePostEffectModelFrame::environment_for_owner() noexcept {
    return impl_->shared.environment;
}
NativeModelRawLifetimeBinding& NativePostEffectModelFrame::lifetime_for_owner() noexcept {
    return impl_->binding;
}
void NativePostEffectModelFrame::owner_cpp_returned(NativeModelOwner& owner) noexcept {
    impl_->node_key = &owner.storage.node;
    impl_->owner_returned = true;
}
void NativePostEffectModelFrame::native_constructor_entered(NativeModelOwner&) noexcept {
    impl_->native_constructor_entered = true;
}
void NativePostEffectModelFrame::native_constructor_returned(NativeModelOwner&) noexcept {
    // IMMEDIATE owning B75030 normal continuation, before any caller lowering,
    // canonical construction/publication or Source health fence can escape.
    impl_->constructor_returned = true;
}
NativeModelOwner& NativePostEffectModelFrame::prepare_owner() {
    require_source_healthy();
    require(!impl_->owner && impl_->allocation_returned && impl_->allocation,
        "ordinary Model owner cannot be prepared twice");
    impl_->shared.owners.prepare_exact_record(exact_,impl_->allocation);
    impl_->owner_birth_entered = true;
    impl_->owner = ::new (&impl_->owner_storage) NativeModelOwner(*this);
    return *impl_->owner;
}
NativeModelOwner* NativePostEffectModelFrame::owner() noexcept {
    return impl_->owner;
}
NativeModelReference* NativePostEffectModelFrame::canonical() noexcept {
    return impl_->reference;
}
NativeModelReference& NativePostEffectModelFrame::publish_constructed_owner() noexcept {
    // Called only by the real constructor's normal-return continuation. Every
    // operand/storage slot was prepared before birth; no Native value sample.
    auto& reference = *(impl_->reference = ::new (&impl_->reference_storage) NativeModelReference(*this));
    reference.bind_ordinary_model(*this); // private Base metadata before publication
    impl_->state.store(ModelState::live,std::memory_order_release);
    impl_->shared.owners.publish_exact_record(exact_,reference);
    return reference;
}
void NativePostEffectModelFrame::parent_field_published() noexcept { impl_->parent_published = true; }
void NativePostEffectModelFrame::tail_ended_after_prefix(NativeModelOwner& owner) noexcept {
    owner.raw_facts_.typed_end_entered = true;
    owner.storage.model.~NativeModelTailStorage();
    owner.phase = NativeModelOwner::Phase::dead;
    impl_->prefix_tail_ended = true;
    // Only the tail completion is issued here. The escaped prefix callee's
    // own effects remain qualified; do not claim whole Node typed-end,
    // normal B750C0 death or a last-flags/physical-return receipt.
}
void NativePostEffectModelFrame::owner_cpp_ending(NativeModelOwner& owner) noexcept {
    if (owner.phase == NativeModelOwner::Phase::prepared) {
        owner.raw_facts_.typed_end_entered = true;
        owner.storage.model.~NativeModelTailStorage();
        owner.storage.node.~NativeNodeStorage();
        owner.phase = NativeModelOwner::Phase::dead;
        impl_->typed_ended.store(true,std::memory_order_release);
        impl_->host.typed_storage_ended = true;
    } else if (owner.phase != NativeModelOwner::Phase::dead) {
        // Same pre-existing ModelOwner explicit-disposition policy. Ordinary
        // callers retain the enclosing graph, never unwind this live owner.
        std::terminate();
    }
}
bool NativePostEffectModelFrame::abandon_before_constructor_return() {
    if (impl_->constructor_returned || impl_->reference) {
        record_source_failure();
        return false; // born object/creator debt survives a later Source failure
    }
    if (impl_->owner) {
        require(impl_->owner->phase == NativeModelOwner::Phase::prepared ||
            impl_->owner->phase == NativeModelOwner::Phase::dead,
            "partial Model constructor has unresolved actual cleanup");
        impl_->owner->~NativeModelOwner();
        impl_->owner = nullptr;
    }
    if (impl_->allocation_returned && impl_->allocation && !impl_->owner_birth_entered &&
        !impl_->native_constructor_entered && !impl_->reference) {
        // Genuine selected state4 cleanup, never entered ANY Model/Node type
        // birth or B75030; no prior payload borrow exists. Model184 remains a
        // live pool-index cell for the real CURRENT-target return below.
        impl_->shared.owners.abandon_exact_unborn_record(exact_);
    }
    return true; // selected Original state4 action; entered-exception stays poisoned
}
void NativePostEffectModelFrame::end_typed_storage() noexcept {
    auto& owner = *impl_->owner;
    owner.raw_facts_.typed_end_entered = true;
    owner.storage.node.~NativeNodeStorage();
    owner.storage.model.~NativeModelTailStorage();
    owner.phase = NativeModelOwner::Phase::dead;
    impl_->typed_ended.store(true,std::memory_order_release);
    impl_->host.typed_storage_ended = true;
    // Existing normal/exception typed end epoch; no logical forget/unbind.
}
void NativePostEffectModelFrame::end_from_scalar(void* context) noexcept {
    static_cast<NativePostEffectModelFrame*>(context)->end_typed_storage();
}
NativePostEffectModelFrame* NativePostEffectModelFrame::from_reference(NativeModelReference& r) noexcept {
    return r.ordinary_post_effect_;
}
bool NativePostEffectModelFrame::belongs_to(const NativePostEffectModelLifetimeContext& c) const noexcept {
    return &impl_->context == &c;
}
void* NativePostEffectModelFrame::actual_key() const noexcept { return impl_->node_key; }
bool NativePostEffectModelFrame::live_for_native_callback() const noexcept {
    return impl_->constructor_returned && !impl_->typed_ended.load(std::memory_order_acquire);
}
void NativePostEffectModelFrame::require_cpp_count_access(NativeModelReference& reference) {
    require(impl_->state.load(std::memory_order_acquire) == ModelState::live &&
        !impl_->failed.load(std::memory_order_acquire) &&
        !impl_->typed_ended.load(std::memory_order_acquire),
        "saved ordinary Model canonical has no live CPP count permission");
    require(impl_->reference && &*impl_->reference == &reference &&
        reference.ordinary_post_effect_ == this && impl_->constructor_returned &&
        static_cast<void*>(&reference.reference_count) == static_cast<std::byte*>(impl_->node_key) + 4,
        "saved ordinary Model canonical lost SAME born count identity");
}
void NativePostEffectModelFrame::begin_cpp_count_use(NativeModelReference& reference) {
    require_cpp_count_access(reference);
    exact_.borrows_.fetch_add(1,std::memory_order_acq_rel);
    impl_->shared.active.fetch_add(1,std::memory_order_acq_rel);
    try { require_cpp_count_access(reference); }
    catch (...) { end_cpp_count_use(); throw; }
}
void NativePostEffectModelFrame::end_cpp_count_use() noexcept {
    impl_->shared.active.fetch_sub(1,std::memory_order_release);
    exact_.borrows_.fetch_sub(1,std::memory_order_release);
}
void NativePostEffectModelFrame::hold_observed_zero(
    NativeRenderObservedZeroFrame& caller, NativeModelReference& reference) {
    begin_call_pin();
    try {
        require_source_healthy();
        require_cpp_count_access(reference); // metadata/identity only, no count sample
        bool expected = false;
        require(observed_zero_hold_.claimed_.compare_exchange_strong(expected,true,
            std::memory_order_acq_rel),"ordinary Model zero hold cannot be claimed twice");
        observed_zero_hold_.model_ = this;
        NativeRenderObservedZeroAccess::attach_hold(caller,observed_zero_hold_);
    } catch (...) {
        record_source_failure();
        end_call_pin();
        throw;
    }
}
void NativePostEffectModelFrame::release_observed_zero_hold(bool source_failed) noexcept {
    if (source_failed) record_source_failure();
    end_call_pin();
}
void NativePostEffectModelFrame::invoke_observed_zero(void* actual, NativeModelReference& reference) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    const NativeRenderObservedZeroAccess::Invocation payload_invocation(impl_->terminal_frame.observed_zero_frame);
    const NativeRenderObservedZeroAccess::Invocation selected_invocation(impl_->terminal.geometry_unwind.observed_zero_frame);
    begin_call_pin();
    try {
        require_source_healthy();
        require(actual == impl_->node_key && canonical() == &reference &&
            reference.ordinary_post_effect_ == this && live_for_native_callback(),
            "ordinary Model observed-zero site requires SAME real born canonical");
        bool expected = false;
        require(!impl_->zero_consumed && impl_->zero_armed.compare_exchange_strong(expected,true,
            std::memory_order_acq_rel),"ordinary Model zero action cannot replay");
        reference.release_zero_references();
        require_source_healthy();
    } catch (...) {
        impl_->zero_armed.store(false,std::memory_order_release);
        record_source_failure();
        end_call_pin();
        throw;
    }
    end_call_pin();
}
bool NativePostEffectModelFrame::consume_zero_action(NativeModelReference& reference) {
    if (canonical() != &reference || reference.ordinary_post_effect_ != this ||
        impl_->zero_consumed || !impl_->zero_armed.exchange(false,std::memory_order_acq_rel)) return false;
    impl_->zero_consumed = true;
    return true;
}
void NativePostEffectModelFrame::complete_zero_action() noexcept { impl_->zero_source_returned = true; }
void NativePostEffectModelFrame::scalar_admit(NativeModelOwner& owner,
    const volatile Word& flags) {
    require(impl_->zero_consumed && impl_->constructor_returned && !impl_->scalar_started &&
        impl_->owner && &*impl_->owner == &owner &&
        &flags == &impl_->terminal_frame.scalar_flags_argument &&
        owner.ordinary_post_effect_ == this && owner.raw_lifetime == &impl_->binding &&
        owner.raw_facts_.returned && owner.raw_facts_.node == &owner.storage.node &&
        owner.raw_facts_.tail == &owner.storage.model &&
        owner.raw_facts_.count == &owner.storage.node.references_04 &&
        owner.raw_facts_.binding == &impl_->binding &&
        owner.raw_facts_.environment == &impl_->shared.environment &&
        impl_->receiver == &impl_->shared.environment.pool_01090054,
        "ordinary Model scalar lacks its actual allocation/birth/zero/retained HOME binding");
    require_source_healthy();
    owner.raw_facts_.destruction_entered = true;
    impl_->scalar_started = true;
    impl_->state.store(ModelState::ending,std::memory_order_release);
}
NativePostEffectModelFrame::ScalarOperands NativePostEffectModelFrame::scalar_operands() {
    require(impl_->scalar_started && !impl_->scalar_kernel_entered && impl_->zero_consumed && impl_->owner &&
        impl_->reference && !impl_->typed_ended.load(std::memory_order_acquire),
        "ordinary Model owning kernel requires its admitted once-only scalar");
    impl_->scalar_kernel_entered = true; // private once-only entry, never public diagnostics
    return {impl_->node_key,impl_->terminal_frame.scalar_flags_argument,
        impl_->terminal_frame,impl_->terminal_operation->storage,impl_->terminal,
        {this,&NativePostEffectModelFrame::end_from_scalar}};
}
void NativePostEffectModelFrame::normal_payload_and_last_flags(
    const volatile Word&, std::uint8_t low) noexcept {
    // Owning kernel calls only after whole payload NORMAL and its sole late
    // byte load. Typed-end alone, including the exception arm, never arrives.
    impl_->normal_dead = true;
    impl_->last_flags = true;
    impl_->low_flags = low;
    impl_->state.store(ModelState::dead,std::memory_order_release);
    impl_->shared.owners.deactivate_exact_record_from_native_end(exact_);
    if ((low & 1u) == 0) fail_context(impl_->context); // real physical debt, no recovery
}
void NativePostEffectModelFrame::scalar_pool_entered() noexcept { impl_->scalar_pool_started = true; }
void NativePostEffectModelFrame::scalar_pool_returned() noexcept {
    impl_->scalar_pool_normal = true;
    impl_->physical_debt.store(false,std::memory_order_release);
}
void NativePostEffectModelFrame::scalar_body_returned() noexcept { impl_->scalar_body_normal = true; }
void NativePostEffectModelFrame::scalar_escaped() noexcept { record_source_failure(); }
void NativePostEffectModelFrame::retire_after_scalar() {
    require(impl_->scalar_body_normal && impl_->normal_dead && impl_->last_flags &&
        impl_->typed_ended.load(std::memory_order_acquire),
        "ordinary Model retirement needs genuine whole body/last-borrow facts");
    impl_->shared.owners.retire_exact_record(exact_);
    impl_->retired = true;
    impl_->reference->phase_ = NativeModelReference::Phase::retired;
    impl_->notice(impl_->producer,*this); // metadata only; never resets this record
}
void NativePostEffectModelFrame::delete_scalar_from_reference(
    NativeModelReference& reference, const volatile Word& flags) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    const NativeRenderObservedZeroAccess::Invocation payload_invocation(impl_->terminal_frame.observed_zero_frame);
    const NativeRenderObservedZeroAccess::Invocation selected_invocation(impl_->terminal.geometry_unwind.observed_zero_frame);
    require(canonical() == &reference && reference.phase_ == NativeModelReference::Phase::bound,
        "ordinary Model scalar requires SAME bound canonical");
    scalar_admit(*impl_->owner,flags);
    reference.phase_ = NativeModelReference::Phase::destroying;
    try {
        (void)delete_native_model_storage_00b75290(*this);
        retire_after_scalar();
        require_source_healthy();
    } catch (...) {
        reference.phase_ = NativeModelReference::Phase::failed;
        record_source_failure();
        throw;
    }
}
volatile Word* NativePostEffectModelFrame::geometry_homes() noexcept { return impl_->homes.data(); }
void NativePostEffectModelFrame::set_geometry_on_captured_model(void* actual) {
    const NativeRenderObservedZeroAccess::Invocation invocation(impl_->geometry_frame.observed_zero_frame_);
    begin_call_pin();
    try {
        auto& target = impl_->shared.lookup(actual);
        target.begin_call_pin();
        try {
            target.canonical()->set_raw_geometry_00b75170(impl_->geometry_frame,impl_->geometry);
            target.require_source_healthy();
        } catch (...) { target.record_source_failure(); target.end_call_pin(); throw; }
        target.end_call_pin();
        require_source_healthy();
    } catch (...) { record_source_failure(); end_call_pin(); throw; }
    end_call_pin();
}
void NativePostEffectModelFrame::unlink_current_model(void* actual) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_call_pin();
    try {
        require_source_healthy();
        require(actual == impl_->node_key && live_for_native_callback(),
            "ordinary Model unlink requires its private still-live actual extent");
        auto& call = impl_->operation(); // fallible BEFORE actual B6DFA0 entry
        unlink_native_node_tree_00b6dfa0(actual,call.trees,call.unlink);
        call.returned = true;
        require_source_healthy();
    } catch (...) { record_source_failure(); end_call_pin(); throw; }
    end_call_pin();
}
void NativePostEffectModelFrame::invoke_current18(Word target) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_call_pin();
    try {
        require(target == 0x00b6f310 && live_for_native_callback(),
            "ordinary Model current18 requires its genuine B6F310/live extent");
        auto& call = impl_->operation(); // Native caller effects are retained on failure
        release_native_node_tree_00b6f310(impl_->node_key,call.tree_frame,call.trees,call.tree);
        call.returned = true;
        require_source_healthy();
    } catch (...) { record_source_failure(); end_call_pin(); throw; }
    end_call_pin();
}
void NativePostEffectModelFrame::invoke_current_scene(Word target, Word scene, Word recursion) {
    const NativeRenderObservedZeroAccess::Invocation invocation(terminal_zero_frame_);
    begin_call_pin();
    try {
        require(live_for_native_callback() && (target == 0x00b6ed80 || target == 0x00b6ee10),
            "ordinary Model current50/54 requires its real live Node extent");
        auto& call = impl_->operation();
        call.scene_frame.scene_argument = scene;
        call.scene_frame.recursion_argument = recursion;
        if (target == 0x00b6ed80)
            set_native_node_scene_storage_00b6ed80(impl_->node_key,call.scene_frame,call.scenes,call.scene);
        else
            remove_native_node_scene_storage_00b6ee10(impl_->node_key,call.scene_frame,call.scenes,call.scene);
        call.returned = true;
        require_source_healthy();
    } catch (...) { record_source_failure(); end_call_pin(); throw; }
    end_call_pin();
}
NativeNodeTreeRetirementInvocation NativePostEffectModelFrame::loan_recursive_home(void* actual) {
    try {
        require(actual == impl_->node_key && live_for_native_callback() &&
            !impl_->failed.load(std::memory_order_acquire),
            "recursive unregister requires SAME genuine Model Node174 carrier");
        HomeState expected = HomeState::ready;
        require(impl_->home_state.compare_exchange_strong(expected,HomeState::active,
            std::memory_order_acq_rel),"recursive Model HOME is active or escaped; it cannot reset/retry");
        require(impl_->home_invocation != std::numeric_limits<std::uint64_t>::max(),
            "Model CPP invocation identity exhausted");
        ++impl_->home_invocation;
        impl_->recursive_home.node_argument = 0;
        impl_->recursive_acquired = {};
        NativeNodeTreeRetirementInvocation result{impl_->recursive_home,impl_->recursive_acquired};
        result.model_home_owner_ = this;
        result.model_home_invocation_ = impl_->home_invocation;
        return result;
    } catch (...) {
        impl_->home_state.store(HomeState::unknown,std::memory_order_release);
        record_source_failure();
        throw;
    }
}
void NativePostEffectModelFrame::recursive_home_returned(void* actual,
    const NativeNodeTreeRetirementInvocation& invocation) noexcept {
    auto* frame = invocation.model_home_owner_;
    if (!frame) return; // the unchanged default/legacy supplier issued no Model loan
    auto& f = *frame->impl_;
    if (actual != f.node_key || &invocation.frame != &f.recursive_home ||
        &invocation.acquired != &f.recursive_acquired ||
        invocation.model_home_invocation_ != f.home_invocation ||
        f.home_state.load(std::memory_order_acquire) != HomeState::active) {
        f.home_state.store(HomeState::unknown,std::memory_order_release);
        frame->record_source_failure();
        return;
    }
    // Only the private owning B6D850 callsite can make this notice, AFTER the
    // actual callee returned. The original erase/walk retains no HOME address.
    f.home_state.store(HomeState::ready,std::memory_order_release);
}
void NativePostEffectModelFrame::cancel_without_native_payload() {
    require(!impl_->allocator_started || (impl_->allocation_returned && !impl_->allocation),
        "entered/unknown Model allocation cannot be cold-cancelled");
    require(!impl_->owner_returned && !impl_->owner && !impl_->reference &&
        !impl_->node_key && !impl_->constructor_outer &&
        exact_.outer_calls_.load(std::memory_order_acquire) == 0 &&
        exact_.borrows_.load(std::memory_order_acquire) == 0,
        "Model cold cancellation requires no typed/key/call/borrow");
    impl_->shared.owners.cancel_exact_record_without_payload(exact_);
    impl_->attached = false;
    if (impl_->retained) {
        impl_->shared.meshes.retention_.retained_.fetch_sub(1,std::memory_order_release);
        impl_->retained = false;
    }
    impl_->closed = true;
    impl_->state.store(ModelState::closed,std::memory_order_release);
}
void NativePostEffectModelFrame::close_at_quiescence() {
    // Entry already requires genuine graph/Block quiescence through ALL outer
    // calls and saved canonical users. Reject every independent active/failure
    // obligation BEFORE unlinking any retained caller metadata.
    try {
        require_source_healthy();
        require(!impl_->closed && impl_->retired && impl_->scalar_body_normal &&
            impl_->zero_source_returned && impl_->normal_dead && impl_->last_flags &&
            impl_->scalar_pool_normal && !impl_->physical_debt.load(std::memory_order_acquire) &&
            !impl_->constructor_outer &&
            impl_->home_state.load(std::memory_order_acquire) == HomeState::ready &&
            exact_.borrows_.load(std::memory_order_acquire) == 0,
            "ordinary Model hold drain requires genuine full normal disposition and source fences");
        bool operations_returned=true;
        {
            std::lock_guard<std::mutex> lock(impl_->operations_mutex);
            for (const auto& call : impl_->operations)
                if (!call->returned) { operations_returned=false; break; }
        }
        require(operations_returned,"entered/escaped Model invocation must remain retained");
        const auto active=[](const NativeRenderObservedZeroFrame& frame) noexcept {
            return frame.active_invocations_.load(std::memory_order_acquire)!=0u;
        };
        require((!observed_zero_hold_.frame_ || !active(*observed_zero_hold_.frame_)) &&
            !active(terminal_zero_frame_) && !active(impl_->terminal_frame.observed_zero_frame) &&
            !active(impl_->terminal.geometry_unwind.observed_zero_frame) &&
            !active(impl_->geometry_frame.observed_zero_frame_),
            "ordinary Model cannot unlink an actually active caller frame");
    } catch (...) { record_source_failure(); throw; }
    // Only the retained-hold contribution to outer_calls/context.active is
    // discharged here, at the caller's real independent quiescence fence.
    if (observed_zero_hold_.frame_)
        require(NativeRenderObservedZeroAccess::end_frame(*observed_zero_hold_.frame_,false),
            "ordinary Model remains inside its actual zero caller");
    require(NativeRenderObservedZeroAccess::end_frame(terminal_zero_frame_,false) &&
        NativeRenderObservedZeroAccess::end_frame(impl_->terminal_frame.observed_zero_frame,false) &&
        NativeRenderObservedZeroAccess::end_frame(impl_->terminal.geometry_unwind.observed_zero_frame,false) &&
        NativeRenderObservedZeroAccess::end_frame(impl_->geometry_frame.observed_zero_frame_,false),
        "ordinary Model caller metadata still has an active invocation");

    require(!impl_->closed && impl_->retired && impl_->scalar_body_normal &&
        impl_->zero_source_returned && impl_->normal_dead && impl_->last_flags &&
        impl_->scalar_pool_normal && !impl_->physical_debt.load(std::memory_order_acquire) &&
        !impl_->constructor_outer &&
        impl_->home_state.load(std::memory_order_acquire) == HomeState::ready &&
        exact_.outer_calls_.load(std::memory_order_acquire) == 0 &&
        exact_.borrows_.load(std::memory_order_acquire) == 0,
        "Model close requires actual full disposition and completed CPP/producer fences");
    // Actual graph quiescence (including future saved canonical users) is the
    // caller's proof, not inferred from these zero counters or registry size.
    impl_->shared.owners.close_exact_record_at_quiescence(exact_);
    impl_->attached = false;
    impl_->notice = nullptr;
    impl_->reference->~NativeModelReference();
    impl_->reference = nullptr;
    impl_->owner->~NativeModelOwner(); // complete CPP destructor while Frame/Context live
    impl_->owner = nullptr;
    impl_->operations.clear();
    if (impl_->retained) {
        impl_->shared.meshes.retention_.retained_.fetch_sub(1,std::memory_order_release);
        impl_->retained = false;
    }
    impl_->closed = true;
    impl_->state.store(ModelState::closed,std::memory_order_release);
}
} // namespace bsp
