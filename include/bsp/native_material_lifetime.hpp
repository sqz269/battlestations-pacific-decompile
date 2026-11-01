#pragma once
#include "bsp/native_material_owner.hpp"
#include "bsp/native_render_zero_frame.hpp"
#include <memory>

namespace bsp {
class NativeMaterialFrame;
class NativeMeshRetentionState;
class NativeMaterialStorageIssuer;
class NativeMaterialFactoryIssuer;
class NativeMaterialStorageAccess;
class NativeMaterialFactoryAccess;
class NativeRenderObservedZeroAccess;
struct NativeMaterialFactoryAcquired;
struct NativeMaterialEffectCacheContext;

// Real existing F8D3AC provider/context contract, not proof by pointer/profile.
// The actual owner keeps Domain, providers and caller receivers alive through
// every Native/Source/health/catch fence and denies teardown on retained debt.
class NativeMaterialDomain final {
public:
    NativeMaterialDomain(NativeMaterialDestructionAccess&,
        NativeRenderActualOwnerRegistry&, const volatile std::uint32_t* actual_d5e520,
        NativeMeshRetentionState& same_graph);
    ~NativeMaterialDomain(); // C++ metadata only; never invented Native cleanup
    NativeMaterialDomain(const NativeMaterialDomain&) = delete;
    NativeMaterialDomain& operator=(const NativeMaterialDomain&) = delete;
    NativeMaterialFrame& prepare(); // stable Domain-owned cold Frame, pre-effects
    bool requires_process_retention() const noexcept; // failed/unknown only
    bool has_retained_records() const noexcept;      // expected live/debt separate
    void close_after_host_quiescence();
private:
    friend class NativeMaterialFrame;
    void require_owned(bool, const char*) const;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

class NativeMaterialFrame final {
public:
    ~NativeMaterialFrame();
    NativeMaterialFrame(const NativeMaterialFrame&) = delete;
    NativeMaterialFrame& operator=(const NativeMaterialFrame&) = delete;
    NativeMaterialFrame(NativeMaterialFrame&&) = delete;
    NativeMaterialFrame& operator=(NativeMaterialFrame&&) = delete;
    // A borrowed same-canonical getter, never Native retain/action authority.
    // Caller independently holds its real Native credit and full C++ last use.
    NativeMaterialReference& canonical();
    bool requires_process_retention() const noexcept;
    void close_after_host_quiescence();
private:
    friend class NativeMaterialDomain;
    friend class NativeMaterialReference;
    friend class NativeMaterialStorageIssuer;
    friend class NativeMaterialFactoryIssuer;
    friend class NativeMaterialStorageAccess;
    friend class NativeMaterialFactoryAccess;
    friend class NativeRenderObservedZeroAccess;
    friend NativeMaterialStorage* create_native_material_initial_00b18900(NativeMaterialFrame&,void*);
    friend NativeMaterialStorage* create_native_material_clone_00b18b60(
        NativeMaterialFrame&,const NativeMaterialStorage&);
    friend NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
        NativeMaterialFrame&,NativeString&,NativeMaterialEffectCacheContext&,NativeMaterialFactoryAcquired&);
    friend NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
        NativeMaterialFrame&,const void*,NativeMaterialEffectCacheContext&,NativeMaterialFactoryAcquired&);
    friend NativeMaterialStorage* create_native_material_for_effect_00535320(
        NativeMaterialFrame&,NativeString&,void* const volatile&);
    friend void release_native_material_creator(NativeMaterialFrame&,long (__stdcall*)(volatile long*));
    friend NativeMaterialStorage* delete_native_material_scalar_00b194b0(
        NativeMaterialFrame&,const void*);

    explicit NativeMaterialFrame(NativeMaterialDomain&);
    void prepare_after_link();
    void require_owned(bool,const char*) const;
    void require_healthy() const;
    static void record_domain_failure(NativeMaterialDomain&) noexcept;
    void record_source_failure() noexcept;
    void require_same_canonical(RenderCommandReference&) const;
    void begin_cpp_count_use(RenderCommandReference&);
    void end_cpp_count_use() noexcept;
    void hold_observed_zero(NativeRenderObservedZeroFrame&,RenderCommandReference&);
    void release_observed_zero_hold(bool) noexcept;
    void invoke_observed_zero(void*,RenderCommandReference&);
    void ordinary_virtual_zero(RenderCommandReference&);
    void begin_outer_call();
    void end_outer_call() noexcept;
    void begin_producer(std::uint32_t private_entry_kind);
    class ChildScope;
    void begin_child_scope() noexcept;
    void end_child_scope() noexcept;
    void fail_children_owned(std::uint32_t private_owned_set) noexcept;
    void release_child(NativeRenderActualOwners&,void*,std::size_t,bool payload);
    void release_factory_effect(NativeRenderActualOwners&,void*);
    void preflight_close() const;

    // Issued only from the actual shared constructor/factory continuations.
    // No public acquired/phase/profile/count can set these observations.
    void native_allocation_entered() noexcept;
    void native_allocation_returned(void*);
    void native_constructor_entered(void*);
    void cpp_prefix_born(NativeMaterialStorage&) noexcept;
    void native_count1_born(NativeMaterialStorage&) noexcept;
    void native_constructor_returned(NativeMaterialStorage&) noexcept;
    void source_constructor_escaped() noexcept;
    bool may_select_constructor_raw_return() const noexcept;
    void selected_constructor_raw_return(void*,NativeMaterialSlotPool&);
    void effect_acquisition_entered() noexcept;
    void effect_acquisition_returned(void*) noexcept;
    void effect_release_entered(void*) noexcept;
    void effect_release_returned() noexcept;
    void publish_after_native_constructor();
    void producer_returned(NativeMaterialStorage*);

    NativeMaterialStorage* create_initial(void*);
    NativeMaterialStorage* create_clone(const NativeMaterialStorage&);
    NativeMaterialStorage* create_cached(const void*,NativeMaterialEffectCacheContext&,
        NativeMaterialFactoryAcquired&);
    NativeMaterialStorage* create_callable(NativeString&,void* const volatile&);
    NativeMaterialStorage* scalar(const void* actual_flags_home);
    void release_creator(long (__stdcall*)(volatile long*));

    struct Impl;
    std::unique_ptr<Impl> impl_;
    NativeRenderActualOwnerRegistry::ExactRecord exact_;
    NativeRenderObservedZeroHold observed_zero_hold_;
    NativeRenderObservedZeroFrame terminal_zero_frame_;
};

// Genuine allocator+existing whole constructor routes. No Source input/mode or
// nullable-effect restriction is added. Clone's actual source Native credit,
// typed extent and C++ lifetime belong to the real caller through all reads.
NativeMaterialStorage* create_native_material_initial_00b18900(NativeMaterialFrame&,void* actual_effect);
NativeMaterialStorage* create_native_material_clone_00b18b60(
    NativeMaterialFrame&,const NativeMaterialStorage&);
// The real caller retains SAME nonmoving Acquired/cache children/context and
// all providers through the Frame's whole return/health/catch/close, or retains
// that complete graph on failure/unknown. A call-duration-only borrow is invalid.
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    NativeMaterialFrame&,NativeString&,NativeMaterialEffectCacheContext&,NativeMaterialFactoryAcquired&);
NativeMaterialStorage* create_native_material_from_effect_cache_00535320(
    NativeMaterialFrame&,const void* actual_live_name_header,NativeMaterialEffectCacheContext&,
    NativeMaterialFactoryAcquired&);
NativeMaterialStorage* create_native_material_for_effect_00535320(
    NativeMaterialFrame&,NativeString&,void* const volatile& actual_renderer_home);
void release_native_material_creator(NativeMaterialFrame&,long (__stdcall* captured_decrement)(volatile long*));
// Actual direct action and borrower quiescence are independent valid-input
// caller obligations. Constructor/diagnostic facts do not authorize deletion.
// Flags0 keeps the old C++ carrier alive and physical debt; no close/retry.
NativeMaterialStorage* delete_native_material_scalar_00b194b0(
    NativeMaterialFrame&,const void* actual_live_four_byte_flags_home);
} // namespace bsp
