#pragma once
#include "bsp/native_render_context.hpp"
#include "bsp/native_render_zero_frame.hpp"
#include "bsp/native_node_tree_retirement.hpp"
#include "bsp/native_model_storage_lifetime.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
class NativeModelPool;
class NativeModelOwner;
class NativeModelReference;
class NativeRenderObservedZeroAccess;
class NativePostEffectModelScalarIssuer;
class NativeMeshCanonicalDomain;
class NativePostEffectConstructionBlock;
class NativePostEffect20ConstructionBlock;
struct NativeModelEnvironment;
struct NativeNodeRawConstants;
struct NativeNodeBaseDestructionContext;
struct NativeModelRawLifetimeBinding;
struct NativeModelStorageView;
class NativeString;
struct NativePostEffect20ArgumentView;
class NativeModelRawFactsAccess;

// Actual mapped profile lookup supplied by the retained Mission receiver.
// No initialization, Native mutation, fallback target or numeric call.
class NativePostEffectModelProfiles {
public:
    virtual ~NativePostEffectModelProfiles() = default;
    virtual const volatile std::uint32_t* resolve_profile(std::uint32_t) const = 0;
};

// Nonmoving metadata over the renderer's actual providers. The entire Context
// outlives every Block/Frame and their complete CPP destructors/outer fences.
class NativePostEffectModelLifetimeContext final {
public:
    NativePostEffectModelLifetimeContext(NativeModelEnvironment&,
        NativeNodeBaseDestructionContext& actual_startup_node,
        NativeRenderActualOwnerRegistry&, NativePostEffectModelProfiles&,
        const NativeNodeRawConstants&, NativeMeshCanonicalDomain&);
    ~NativePostEffectModelLifetimeContext() noexcept;
    NativePostEffectModelLifetimeContext(const NativePostEffectModelLifetimeContext&) = delete;
    NativePostEffectModelLifetimeContext& operator=(const NativePostEffectModelLifetimeContext&) = delete;
    bool requires_process_retention() const noexcept;
    // Exact captured parent field; genuine Native field credit is a caller
    // precondition. Legacy camera transport keeps its existing policy.
    void release_captured_node(void* actual, class NativeNodeDestructionRuntime& legacy);
private:
    friend class NativePostEffectModelFrame;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Reserved before Native allocation; no incoming raw key or public receipt.
// Default destruction performs no Native cleanup or implicit pool return.
// Entered/unknown/flags0 records require actual enclosing-owner retention.
class NativePostEffectModelFrame final {
public:
    NativePostEffectModelFrame(NativePostEffectModelLifetimeContext&,
        const void* exact_stable_producer_identity);
    ~NativePostEffectModelFrame() noexcept;
    NativePostEffectModelFrame(const NativePostEffectModelFrame&) = delete;
    NativePostEffectModelFrame& operator=(const NativePostEffectModelFrame&) = delete;
    NativePostEffectModelFrame(NativePostEffectModelFrame&&) = delete;
    NativePostEffectModelFrame& operator=(NativePostEffectModelFrame&&) = delete;
private:
    friend class NativeModelOwner;
    friend class NativeModelReference;
    friend class NativePostEffectConstructionBlock;
    friend class NativePostEffect20ConstructionBlock;
    friend class NativePostEffectModelLifetimeContext;
    friend class NativePostEffectModelScalarIssuer;
    friend class NativeRenderObservedZeroAccess;
    friend class NativeModelRawFactsAccess;
    friend void* construct_native_post_effect_00b4e840(void*, std::size_t,
        NativeString&, std::uint32_t, const void*, NativePostEffectConstructionBlock&);
    friend void* construct_native_post_effect20_00b4e470(void*, std::size_t,
        NativeString&, std::uint32_t, const void*, NativePostEffect20ConstructionBlock&);
    friend void* construct_native_post_effect20_00b4e470(void*, std::size_t,
        const NativePostEffect20ArgumentView&, NativePostEffect20ConstructionBlock&);
    friend void* allocate_native_model_slot_00b74eb0(NativePostEffectModelFrame&);
    friend void return_native_model_slot_00b748c0(void*, NativePostEffectModelFrame&);
    struct Impl;
    NativeRenderActualOwnerRegistry::ExactRecord exact_;
    std::unique_ptr<Impl> impl_;
    NativeRenderObservedZeroHold observed_zero_hold_;
    NativeRenderObservedZeroFrame terminal_zero_frame_;

    void bind_block(NativePostEffectConstructionBlock&);
    void bind_block(NativePostEffect20ConstructionBlock&);
    void require_mesh_domain(NativeMeshCanonicalDomain&);
    void begin_constructor();
    void finish_constructor() noexcept;
    void record_source_failure() noexcept;
    void cancel_without_native_payload();
    void close_at_quiescence();
    bool requires_process_retention() const noexcept;
    void wrapper_entered();
    void allocator_entered(NativeModelPool&);
    void allocator_returned(NativeModelPool&, void*) noexcept;
    void allocator_escaped() noexcept;
    void return_entered(NativeModelPool&, void*);
    void return_returned() noexcept;
    void return_escaped() noexcept;
    NativeModelOwner& prepare_owner();
    NativeModelReference& publish_constructed_owner() noexcept;
    NativeModelOwner* owner() noexcept;
    NativeModelReference* canonical() noexcept;
    void parent_field_published() noexcept;
    bool abandon_before_constructor_return();
    volatile std::uint32_t* geometry_homes() noexcept;
    void set_geometry_on_captured_model(void*);
    void unlink_current_model(void*);
    void invoke_current18(std::uint32_t);
    void invoke_current_scene(std::uint32_t, std::uint32_t, std::uint32_t);

    void* actual_for_owner();
    NativeModelEnvironment& environment_for_owner() noexcept;
    NativeModelRawLifetimeBinding& lifetime_for_owner() noexcept;
    void owner_cpp_returned(NativeModelOwner&) noexcept;
    void native_constructor_entered(NativeModelOwner&) noexcept;
    void native_constructor_returned(NativeModelOwner&) noexcept;
    void owner_cpp_ending(NativeModelOwner&) noexcept;
    void tail_ended_after_prefix(NativeModelOwner&) noexcept;
    void end_typed_storage() noexcept;
    static void end_from_scalar(void*) noexcept;
    struct ScalarOperands {
        void* actual;
        const volatile std::uint32_t& flags;
        NativeModelStorageLifetimeFrame& frame;
        NativeModelStorageContext& context;
        NativeModelStorageLifetimeAcquired& acquired;
        NativeModelStorageHostCompletion completion;
    };
    ScalarOperands scalar_operands();
    void scalar_admit(NativeModelOwner&, const volatile std::uint32_t&);
    void normal_payload_and_last_flags(const volatile std::uint32_t&, std::uint8_t) noexcept;
    void scalar_pool_entered() noexcept;
    void scalar_pool_returned() noexcept;
    void scalar_body_returned() noexcept;
    void scalar_escaped() noexcept;
    void retire_after_scalar();
    void delete_scalar_from_reference(NativeModelReference&, const volatile std::uint32_t&);
    void require_cpp_count_access(NativeModelReference&);
    void begin_cpp_count_use(NativeModelReference&);
    void end_cpp_count_use() noexcept;
    void hold_observed_zero(NativeRenderObservedZeroFrame&, NativeModelReference&);
    void release_observed_zero_hold(bool source_failed) noexcept;
    void invoke_observed_zero(void*, NativeModelReference&);
    bool consume_zero_action(NativeModelReference&);
    void complete_zero_action() noexcept;
    NativeNodeTreeRetirementInvocation loan_recursive_home(void*);
    static void recursive_home_returned(void*, const NativeNodeTreeRetirementInvocation&) noexcept;
    static NativePostEffectModelFrame* from_reference(NativeModelReference&) noexcept;
    bool belongs_to(const NativePostEffectModelLifetimeContext&) const noexcept;
    void* actual_key() const noexcept;
    bool live_for_native_callback() const noexcept;
    static void fail_context(NativePostEffectModelLifetimeContext&) noexcept;
    void begin_call_pin();
    void end_call_pin() noexcept;
    void require_source_healthy();
};
}
